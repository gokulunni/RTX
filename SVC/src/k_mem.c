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

int first_fit_mem_init(size_t blk_size);
void *first_fit_mem_alloc(size_t size);
void first_fit_mem_dealloc(void *ptr);
int first_fit_count_extfrag(size_t size);


/* 
   This symbol is defined in the scatter file, 
   refer to ARM Compiler v5.x Linker User Guide
*/  
extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;

typedef struct __node_t {
    int size;
    struct __note_t *next;
    struct __note_t *prev;
//TODO: magic?
} node_t;


node_t *free_mem_head;
int mem_alloc_algo;
size_t mem_blk_size;


int k_mem_init(size_t blk_size, int algo){
    if (blk_size <= 0) {
        return RTX_ERR;
    }
    
	unsigned int end_addr = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;
    
    mem_blk_size = blk_size;
    mem_alloc_algo = algo;
    
#ifdef DEBUG_0
	printf("k_mem_init: blk_size = %d, algo = %d\r\n", blk_size, algo);
	printf("k_mem_init: image ends at 0x%x\r\n", end_addr);
	printf("k_mem_init: IRAM1 ends at 0x%x\r\n", IRAM1_END);
#endif /* DEBUG_0 */
    
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_mem_init(end_addr, blk_size);
        default:
            return RTX_ERR;
    }
    
	return 0;
}

void *k_mem_alloc(size_t size) {
#ifdef DEBUG_0
	printf("k_mem_alloc: requested memory size = %d\r\n", size);
#endif /* DEBUG_0 */
    if (size <= 0) {
        return RTX_ERR;
    }
    
    // CHECK INIT HAS BEEN RUN
    
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_mem_alloc(size);
        default:
            return RTX_ERR;
    }
    
	return NULL;
}

void k_mem_dealloc(void *ptr) {
#ifdef DEBUG_0
	printf("k_mem_dealloc: freeing 0x%x\r\n", (U32) ptr);
#endif /* DEBUG_0 */
    
    
    // CHECK INIT HAS BEEN RUN
    // CHECK PTR is VALID
    
    switch (mem_alloc_algo) {
           case FIRST_FIT:
               return first_fit_mem_dealloc(ptr);
           default:
               return RTX_ERR;
       }
    
    
	return;
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

	return 0;
}

/*
*  First Fit Memory Allocation
*/


int first_fit_mem_init(unsigned int end_addr, size_t blk_size) {
    if (blk_size <= 0) {
        return RTX_ERR;
    }

    free_mem_head = end_addr + 4;
    free_mem_head->size = IRAM1_END - end_addr;
    free_mem_head->prev = NULL;
    free_mem_head->next = NULL;

    // TODO: do we need to clear data

    return RTX_OK;
}


void *first_fit_mem_alloc(size_t size) {
    if (size <= 0) {
        return RTX_ERR;
    }
    
    int mem_chunk_size = CEIL((size + sizeof(node_t)), mem_blk_size) * mem_blk_size;

    node_t *cur_node = free_mem_head;
    while (cur_node != NULL) {
        if (cur_node->size > mem_chunk_size) {
            node_t new_node = cur_node + mem_chunk_size;
            new_node->size = cur_node->size - mem_chunk_size;
            new_node->next = cur_node->next;
            new_node->prev = cur_node->prev;
            cur_node->size = mem_chunk_size;
            return cur_node;
        } else if (cur_node->size == mem_chunk_size) {
            node_t next_node = cur_node->next;
            node_t prev_node = cur_node->prev;
            prev_node->next = next_node;
            next_node->prev = prev_node
            return cur_node + sizeof(node_t);
        }
//        else if (cur_node->size < blk_size) {
//
//            //print out investigate this
//        }
    }
}


void first_fit_mem_dealloc(void *ptr) {
    node_t *deallocateNode = (ptr - sizeOf(deallocateNode));
    void *startingAddress= ptr - sizeOf(deallocateNode);
    void *endingAddress = ptr + deallocateNode->size;
    node_t *curNode = free_mem_head;

    int frontSet =FALSE;
    node_t *nodeBefore=NULL;

    //Iterate through unallocated data linked list
    while(curNode!=NULL){
        
        //if the end of current Node's address space is the starting address expand data
        //Cur node is starting address
        //increment sizeOf(node_t) to increment past it
        //Add the curNode->size +1 to get to end of size and to next data segment
        if(((void*) curNode + sizeOf(curNode) + curNode->size + 4 )== startingAddress ){
            curNode->next = deallocateNode->next;
            curNode->size= curNode->size+sizeOf(deallocateNode)+deallocateNode->size;
            //This line below is so we can keep track of the deallocated node as it now an extension of curNode
            //if we have Free|Dealocated|Free memory situation
            deallocateNode= curNode;
            frontSet = TRUE;
        }
        else if((void*) curNode == endingAddress+4){
            deallocateNode->next = curNode->next;
            deallocateNode->size = deallocateNode->size +sizeOf(curNode)+curNode->size;
        }
        
        //Find the previous free node in linked list before the deallocateNode
        if(curNode<startingAddress){
            nodeBefore = curNode;
        }
        
        curNode = curNode->next;
    }

    //Put the deallocated node in the correct location within the linked list of unallocated data
    //Assuming we have the case allocated|Deallocate|(allocated or Free)
    if(!frontSet){
        deallocateNode->prev = nodeBefore;
        if(nodeBefore->next < endingAddress){
            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 1");
        }
        else if(nodeBefore > startingAddress){
            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 2");
        }
        else if (nodeBefore+nodeBefore->size+sizeOfNode(nodeBefore)>startingAddress){
            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 3");
        }
        else if (nodeBefore+nodeBefore->size+sizeOfNode(nodeBefore)+4==startingAddress){
            print("We have encountered an issue in allocated|Deallocate|(allocated or free) Code error 4");
        }
        deallocateNode->next = nodeBefore->next;
        nodeBefore->next = deallocateNode;
    }

    return;
}


int first_fit_count_extfrag(size_t size) {
    int counter=0;
    node_t *curNode = free_mem_head;

    while(curNode != NULL){
        if (curNode->size <= size){ // TODO: do we need to take into account header?
            counter++;
        }
        curNode = curNode->next;
    }
    
    return counter;
}
