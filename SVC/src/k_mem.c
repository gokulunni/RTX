/** 
 * @brief:  k_mem.c kernel API implementations, this is only a skeleton.
 * @author: Yiqing Huang
 * @date:   2020/09/03
 */

#include "common.h"
#include "k_mem.h"
#include "uart_polling.h"
#ifdef DEBUG_0
#include "printf.h"
#endif  /* DEBUG_0 */
#define CEIL(x,y) ((x+y-1)/y)


/* 
   This symbol is defined in the scatter file, 
   refer to ARM Compiler v5.x Linker User Guide
*/  
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;

typedef struct node {
    int size;
    struct node *next;
    struct node *prev;
//TODO: magic?
} node_t;

typedef struct used_mem_node {
    int size;
} used_mem_node_t;

node_t *free_mem_head;
int mem_alloc_algo;
size_t mem_blk_size;


int first_fit_mem_init(unsigned int end_addr);
void *first_fit_mem_alloc(size_t size);
void first_fit_mem_dealloc(void *ptr);
int first_fit_count_extfrag(size_t size);


int k_mem_init(size_t blk_size, int algo){
    unsigned int end_addr;
		
    if (blk_size <= 0) {
        return RTX_ERR;
    }
    
    end_addr = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;
    
    mem_blk_size = blk_size;
    mem_alloc_algo = algo;
    
#ifdef DEBUG_0
	printf("k_mem_init: blk_size = %d, algo = %d\r\n", blk_size, algo);
	printf("k_mem_init: image ends at 0x%x\r\n", end_addr);
	printf("k_mem_init: IRAM1 ends at 0x%x\r\n", IRAM1_END);
#endif /* DEBUG_0 */
    
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_mem_init(end_addr);
        default:
            return RTX_ERR;
    }
}

void *k_mem_alloc(size_t size) {
#ifdef DEBUG_0
	printf("k_mem_alloc: requested memory size = %d\r\n", size);
#endif /* DEBUG_0 */
    if (size <= 0) {
        return NULL;
    }
    
    // CHECK INIT HAS BEEN RUN
		
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_mem_alloc(size);
        default:
            return NULL;
    }
}

void k_mem_dealloc(void *ptr) {
#ifdef DEBUG_0
	printf("k_mem_dealloc: freeing 0x%x\r\n", (U32) ptr);
#endif /* DEBUG_0 */
    
    
    // CHECK INIT HAS BEEN RUN
    // CHECK PTR is VALID
    
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            first_fit_mem_dealloc(ptr);
        default:
            return;
    }
}

int k_mem_count_extfrag(size_t size) {
#ifdef DEBUG_0
	printf("k_mem_extfrag: size = %d\r\n", size);
#endif /* DEBUG_0 */
    
    if (size <= 0) {
        return RTX_ERR;
    }
    
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_count_extfrag(size);
        default:
            return RTX_ERR;
    }
}

/*
*  First Fit Memory Allocation
*/


int first_fit_mem_init(unsigned int end_addr) {
    free_mem_head = (node_t *) (end_addr + 4);
    free_mem_head->size = IRAM1_END - end_addr - 4;
    free_mem_head->prev = NULL;
    free_mem_head->next = NULL;
		printf("Header size %d\r\n", sizeof(node_t));
    // TODO: do we need to clear data

    return RTX_OK;
}


void *first_fit_mem_alloc(size_t size) {
		node_t *cur_node;
		node_t *new_node;
		int mem_chunk_size;
	
    if (size <= 0) {
        return NULL;
    }
    
    mem_chunk_size = CEIL((size + sizeof(node_t)), mem_blk_size) * mem_blk_size;
    printf("mem chunk size %d\r\n", mem_chunk_size);
    cur_node = free_mem_head;
    while (cur_node != NULL) {
        if (cur_node->size > mem_chunk_size) {
            new_node = (node_t *) (cur_node + mem_chunk_size/sizeof(node_t));
            new_node->size = cur_node->size - mem_chunk_size;
            new_node->next = cur_node->next;
            new_node->prev = cur_node->prev;
            printf("new_node address 0x%x\r\n", new_node);
            printf("new_node size 0x%x\r\n", new_node->size);
            printf("new_node prev 0x%x\r\n", new_node->prev);
            printf("new_node next 0x%x\r\n", new_node->next);
            cur_node->size = mem_chunk_size;
            printf("cur_node address 0x%x\r\n", cur_node);
            printf("cur_node size 0x%x\r\n", cur_node->size);
            printf("cur_node prev 0x%x\r\n", cur_node->prev);
            printf("cur_node next 0x%x\r\n", cur_node->next);
            return cur_node + sizeof(node_t);
        } else if (cur_node->size == mem_chunk_size) {
            cur_node->prev->next = cur_node->next;
            cur_node->next->prev = cur_node->prev;
            return cur_node + sizeof(node_t);
        }
//        else if (cur_node->size < blk_size) {
//
//            //print out investigate this
//        }
    }
		
		return NULL;
}


void first_fit_mem_dealloc(void *ptr) {
//    node_t *deallocateNode = (ptr - sizeof(deallocateNode));
//    void *startingAddress= ptr - sizeof(deallocateNode);
//    void *endingAddress = ptr + deallocateNode->size;
//    node_t *curNode = free_mem_head;

//    int frontSet =FALSE;
//    node_t *nodeBefore=NULL;

//    //Iterate through unallocated data linked list
//    while(curNode!=NULL){
//        
//        //if the end of current Node's address space is the starting address expand data
//        //Cur node is starting address
//        //increment sizeOf(node_t) to increment past it
//        //Add the curNode->size +1 to get to end of size and to next data segment
//        if(((void*) curNode + sizeof(curNode) + curNode->size + 4 )== startingAddress ){
//            curNode->next = deallocateNode->next;
//            curNode->size= curNode->size + sizeof(deallocateNode) + deallocateNode->size;
//            //This line below is so we can keep track of the deallocated node as it now an extension of curNode
//            //if we have Free|Dealocated|Free memory situation
//            deallocateNode= curNode;
//            frontSet = TRUE;
//        }
//        else if ((void*) curNode == endingAddress+4){
//            deallocateNode->next = curNode->next;
//            deallocateNode->size = deallocateNode->size + sizeof(curNode)+curNode->size;
//        }
//        
//        //Find the previous free node in linked list before the deallocateNode
//        if (curNode < startingAddress){
//            nodeBefore = curNode;
//        }
//        
//        curNode = curNode->next;
//    }

//    //Put the deallocated node in the correct location within the linked list of unallocated data
//    //Assuming we have the case allocated|Deallocate|(allocated or Free)
//    if(!frontSet){
//        deallocateNode->prev = nodeBefore;
//        if(nodeBefore->next < endingAddress){
//            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 1");
//        }
//        else if(nodeBefore > startingAddress){
//            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 2");
//        }
//        else if (nodeBefore+nodeBefore->size+sizeOfNode(nodeBefore)>startingAddress){
//            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 3");
//        }
//        else if (nodeBefore+nodeBefore->size+sizeOfNode(nodeBefore)+4==startingAddress){
//            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 4");
//        }
//        deallocateNode->next = nodeBefore->next;
//        nodeBefore->next = deallocateNode;
//    }

    return;
}


int first_fit_count_extfrag(size_t size) {
    int counter=0;
    node_t *curNode = free_mem_head;

    while(curNode != NULL){
        if (curNode->size < size){ // TODO: do we need to take into account header?
            counter++;
        }
        curNode = curNode->next;
    }
    
    return counter;
}
