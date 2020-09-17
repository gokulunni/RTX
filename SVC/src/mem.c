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

typedef struct __note_t {
    int size;
    struct __note_t *next;
    struct __note_t *prev;
//TODO: magic?
} node_t;


note_t *free_mem_head;
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
    
}

int first_fit_count_extfrag(size_t size) {
    
}
