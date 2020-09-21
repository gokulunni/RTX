/** 
 * @brief:  mem.c
 * @author: Nguyen Dinh Nguyen
 * @date:   2020/09/16
 */

#include "common.h"
#include "mem.h"
#include "uart_polling.h"
#ifdef DEBUG_0
#include "printf.h"
#endif  /* DEBUG_0 */

typedef struct __node_t {
    int size;
    struct __note_t *next;
    struct __note_t *prev;
//TODO: magic?
} node_t;


node_t *free_mem_head;
int mem_alloc_algo;
int mem_blk_size;


int mem_init(size_t blk_size, int algo) {
    // CHECK SIZE BIGGER THAN MIN
    
    mem_blk_size = blk_size;
    mem_alloc_algo = algo;
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_init(blk_size)
        default:
            return RTX_ERR;
    }
    
}

// TODO: EXTEND HEAP

void *mem_alloc(size_t size) {
    // CHECK INIT HAS BEEN RUN
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            first_fit_alloc(blk_size)
            break;
        default:
            break;
    }
}

void mem_dealloc(void *ptr) {
    // CHECK INIT HAS BEEN RUN
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            first_fit_dealloc(blk_size)
            break;
        default:
            break;
    }
}

int mem_count_extfrag(size_t size) {
    // CHECK INIT HAS BEEN RUN
    switch (mem_alloc_algo) {
        case FIRST_FIT:
            first_fit_count_extfrag(blk_size)
            break;
        default:
            break;
    }
}

/*
 *  First Fit Memory Allocation
 */

int first_fit_mem_init(size_t blk_size) {
    // TODO: mem_size
    // CHECK SIZE BIGGER THAN MIN
    int mem_size = 0;
    
    free_mem_head = mmap(NULL, mem_size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)
    
    if (free_mem_head == MAP_FAILED) {
        // PRINT ERROR
        return RTX_ERR;
    }
    
    head->size = mem_size - sizeof(node_t);
    head->prev = NULL;
    head->next = NULL;

    return RTX_OK;
}

void *first_fit_alloc(size_t size) {
    
    if (size <= 0) {
        return RTX_ERR;
    }
    
    int block_size = size + sizeof(node_t);
    
    node_t *n = free_mem_head;
    while (n != NULL) {
        if (n->size > block_size) {
            
        } else if (n->size == block_size) {
            
            
            
            return n;
        } else if (n->size < blk_size) {
            //print out investigate this
        }
    }
    
    
//     while(linked list not null){
//        if( linkedList_Size>= size+ headerSize){
//
//            if (linkedList_Size> size&& linkedList_Size > blockSize){
//            //Split if size is bigger
//                return split()
//            }
//            else if linkedList_size<blockSize{
//                print out investigate this
//            }
//            else{
//                //Otherwise return address
//                return linedList.Address + sizeOf(node_t);
//            }
//
//        }
//
//    }
}

void first_fit_dealloc(void *ptr) {
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

    while(curNode!=NULL){
        if (curNode->size<=size){
            counter++;
        }
        curNode = curNode->next;
    }
    return counter;
}
