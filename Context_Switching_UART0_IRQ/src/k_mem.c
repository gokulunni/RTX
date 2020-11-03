/**
 * @file:   k_mem.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2020/09/28
 */

#include "k_mem.h"
#include "common.h"
#include "uart_polling.h"
#ifdef DEBUG_MEM
#include "printf.h"
#endif /* ! DEBUG_MEM */
#define CEIL(x,y) ((x+y-1)/y)

/* The final memory map of IRAM1 looks like the following. 
   Refer to k_task.c for initilization function details.
---------------------------------------------------------------------------

                    0x10008000+---------------------------+ High Address
                              |                           |
                              |  Free Memory Space        |
                              |                           |
                              |                           |
                              |                           |
                              |---------------------------|
                              |        Padding            |
&Image$$RW_IRAM1$$ZI$$Limit-->|---------------------------|
                              |                           |          
                              |       RTX  Image          |
                              |                           |
                    0x10000000+---------------------------+ Low Address

*/

typedef struct node {
    int size;
    struct node *next;
    struct node *prev;
} node_t;

typedef struct used_mem_node {
    U32 owner_tid;
    int size;
} used_mem_node_t;

node_t *free_mem_head;
int mem_alloc_algo;
size_t mem_blk_size;
int mem_init_status;

void print_linked_list(char *prefix);
int first_fit_mem_init(unsigned int end_addr);
void *first_fit_mem_alloc(size_t size);
int first_fit_mem_dealloc(void *ptr);
int first_fit_count_extfrag(size_t size);

extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
extern TCB *gp_current_task;

int k_mem_init(size_t blk_size, int algo){
    U32 end_addr;

#ifdef DEBUG_MEM
    printf("******************************************************\r\n");
#endif /* DEBUG_MEM */

    if (blk_size <= 0) {
#ifdef DEBUG_MEM
        printf("k_mem_init: blk_size is less than 1, blk_size = %d", blk_size);
#endif /* DEBUG_MEM */
        mem_init_status = RTX_ERR;
        return RTX_ERR;
    }

    end_addr = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;

    mem_blk_size = blk_size;
    mem_alloc_algo = algo;

#ifdef DEBUG_MEM
    printf("k_mem_init: blk_size = %d, algo = %d\r\n", blk_size, algo);
    printf("k_mem_init: image ends at 0x%x\r\n", end_addr);
    printf("k_mem_init: heap starts at 0x%x\r\n", end_addr + 4);
    printf("k_mem_init: IRAM1 ends at 0x%x\r\n", IRAM1_END);
#endif /* DEBUG_MEM */

    switch (mem_alloc_algo) {
        case FIRST_FIT:
            mem_init_status = first_fit_mem_init(end_addr + 4);
            return mem_init_status;
        default:
            mem_init_status = RTX_ERR;
            return RTX_ERR;
    }
}


void *k_mem_alloc(size_t size) {
#ifdef DEBUG_MEM
    printf("******************************************************\r\n");
	printf("k_mem_alloc: requested memory size = %d\r\n", size);
#endif /* DEBUG_MEM */

    if (size <= 0) {
#ifdef DEBUG_MEM
        printf("k_mem_alloc: size is less than 1, size = %d\r\n", size);
#endif /* DEBUG_MEM */
        return NULL;
    }

    if (mem_init_status != RTX_OK) {
#ifdef DEBUG_MEM
        printf("k_mem_alloc: mem_init_status != RTX_OK, mem_init_status = %d\r\n", mem_init_status);
#endif /* DEBUG_MEM */
        return NULL;
    }

    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_mem_alloc(size);
        default:
            return NULL;
    }
}

int k_mem_dealloc(void *ptr) {
#ifdef DEBUG_MEM
    printf("******************************************************\r\n");
	printf("k_mem_dealloc: freeing 0x%x\r\n", (U32) ptr);
#endif /* DEBUG_MEM */

    if (mem_init_status != RTX_OK) {
#ifdef DEBUG_MEM
        printf("k_mem_dealloc: mem_init_status != RTX_OK, mem_init_status = %d\r\n", mem_init_status);
#endif /* DEBUG_MEM */
        return RTX_ERR;
    }

    switch (mem_alloc_algo) {
        case FIRST_FIT:
            return first_fit_mem_dealloc(ptr);
        default:
            return RTX_ERR;
    }
}

int k_mem_count_extfrag(size_t size) {
#ifdef DEBUG_MEM
    printf("******************************************************\r\n");
	printf("k_mem_extfrag: size = %d\r\n", size);
#endif /* DEBUG_MEM */

    if (mem_init_status != RTX_OK) {
#ifdef DEBUG_MEM
        printf("k_mem_count_extfrag: mem_init_status != RTX_OK, mem_init_status = %d\r\n", mem_init_status);
#endif /* DEBUG_MEM */
        return RTX_ERR;
    }

    if (size <= 0) {
#ifdef DEBUG_MEM
        printf("k_mem_count_extfrag: size is less than 1, size = %d\r\n", size);
#endif /* DEBUG_MEM */
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


int first_fit_mem_init(unsigned int heap_addr) {
    free_mem_head = (node_t *) (heap_addr);
    free_mem_head->size = IRAM1_END - heap_addr - sizeof(node_t);
    free_mem_head->prev = NULL;
    free_mem_head->next = NULL;

#ifdef DEBUG_MEM
    printf("first_fit_mem_init: Free memory block metadata node size 0x%x\r\n", sizeof(node_t));
    printf("first_fit_mem_init: Used memory block metadata node size 0x%x\r\n", sizeof(used_mem_node_t));
    print_linked_list("first_fit_mem_init");
#endif /* DEBUG_MEM */

    return RTX_OK;
}


void *first_fit_mem_alloc(size_t size) {
    int mem_chunk_size;
    used_mem_node_t *ret_node;
    node_t *new_node;
    node_t *cur_node = free_mem_head;

    if ((int)size <= 0) {
#ifdef DEBUG_MEM
        printf("first_fit_mem_alloc: size is less than 1, size = %d\r\n", size);
#endif /* DEBUG_MEM */
        return NULL;
    }
    else if((int)size==NULL){
#ifdef DEBUG_MEM
        printf("first_fit_mem_alloc: size is null");
#endif /* DEBUG_MEM */
        return NULL;
    }

#ifdef DEBUG_MEM
    printf("first_fit_mem_alloc: Linked list before allocation\r\n");
    print_linked_list("first_fit_mem_alloc");
#endif /* DEBUG_MEM */

    mem_chunk_size = CEIL((size + sizeof(used_mem_node_t)), mem_blk_size) * mem_blk_size;
#ifdef DEBUG_MEM
    printf("first_fit_mem_alloc: Return memory block size with header: 0x%x\r\n", mem_chunk_size);
#endif /* DEBUG_MEM */

    if (cur_node == NULL) {
#ifdef DEBUG_MEM
        printf("first_fit_mem_alloc: Heap is full\r\n");
#endif /* DEBUG_MEM */
        return NULL;
    }

    while (cur_node != NULL) {
        if (cur_node->size >= mem_chunk_size) {
#ifdef DEBUG_MEM
            printf("first_fit_mem_alloc: Splitting a memory block of size 0x%x\r\n", cur_node->size + sizeof(node_t));
            printf("first_fit_mem_alloc: Current node address 0x%x before splitting\r\n", cur_node);
            printf("first_fit_mem_alloc: Current node size 0x%x before splitting\r\n", cur_node->size);
            printf("first_fit_mem_alloc: Current node prev 0x%x before splitting\r\n", cur_node->prev);
            printf("first_fit_mem_alloc: Current node next 0x%x before splitting\r\n", cur_node->next);
#endif /* DEBUG_MEM */

            new_node = (node_t *) ((char *) cur_node + mem_chunk_size);
            new_node->size = cur_node->size - mem_chunk_size;

            if (free_mem_head == cur_node) {
                free_mem_head = new_node;
            }

            new_node->next = cur_node->next;
            new_node->prev = cur_node->prev;

#ifdef DEBUG_MEM
            printf("first_fit_mem_alloc: New free node address 0x%x after splitting\r\n", new_node);
            printf("first_fit_mem_alloc: New free node size 0x%x after splitting\r\n", new_node->size);
            printf("first_fit_mem_alloc: New free node prev 0x%x after splitting\r\n", new_node->prev);
            printf("first_fit_mem_alloc: New free node next 0x%x after splitting\r\n", new_node->next);
#endif /* DEBUG_MEM */

            ret_node = (used_mem_node_t *) cur_node;
            ret_node->size = mem_chunk_size - sizeof(used_mem_node_t);
            if (gp_current_task) {
                ret_node->owner_tid = gp_current_task->tid;
            } else {
#ifdef DEBUG_MEM
                printf("first_fit_mem_alloc: No running task to alloc a memory block\r\n");
#endif /* DEBUG_MEM */
                return NULL;
            }

#ifdef DEBUG_MEM
            printf("first_fit_mem_alloc: New allocated node address 0x%x\r\n", ret_node);
            printf("first_fit_mem_alloc: New allocated node size 0x%x\r\n", ret_node->size);
#endif /* DEBUG_MEM */

            print_linked_list("first_fit_mem_alloc");

            return ret_node + 1;
        } else if (!(cur_node->size + sizeof(node_t) < mem_chunk_size)) {
            if (free_mem_head == cur_node) {
                free_mem_head = cur_node->next;
            }

            if (cur_node->next != NULL) {
                cur_node->next->prev = cur_node->prev;
            }

            if (cur_node->prev != NULL) {
                cur_node->prev->next = cur_node->next;
            }

            ret_node = (used_mem_node_t *) cur_node;
            ret_node->size = cur_node->size;
            if (gp_current_task) {
                ret_node->owner_tid = gp_current_task->tid;
            } else {
#ifdef DEBUG_MEM
                printf("first_fit_mem_alloc: No running task to alloc a memory block\r\n");
#endif /* DEBUG_MEM */
                return NULL;
            }

#ifdef DEBUG_MEM
            printf("first_fit_mem_alloc: New allocated node address 0x%x\r\n", ret_node);
            printf("first_fit_mem_alloc: New allocated node size 0x%x\r\n", ret_node->size);
#endif /* DEBUG_MEM */

            print_linked_list("first_fit_mem_alloc");

            return ret_node + 1;
        }
        cur_node = cur_node->next;
    }

#ifdef DEBUG_MEM
    printf("first_fit_mem_alloc: No memory block big enough for requested size\r\n");
#endif /* DEBUG_MEM */

    return NULL;
}


int first_fit_mem_dealloc(void *ptr) {
    node_t *new_node;
    node_t *cur_node = free_mem_head;
    used_mem_node_t *dealloc_ptr = (used_mem_node_t *) ptr - 1;

#ifdef DEBUG_MEM
    printf("first_fit_mem_dealloc: Dealloc node address 0x%x\r\n", dealloc_ptr);
    printf("first_fit_mem_dealloc: Dealloc node size 0x%x\r\n", dealloc_ptr->size);
#endif /* DEBUG_MEM */

    if (gp_current_task->tid != dealloc_ptr->owner_tid) {
#ifdef DEBUG_MEM
        printf("first_fit_mem_dealloc: task %d cannot dealloc task %d\r\n", gp_current_task->tid, dealloc_ptr->owner_tid);
#endif /* DEBUG_MEM */
        return RTX_ERR;
    }

    if (free_mem_head == NULL) {
#ifdef DEBUG_MEM
        printf("first_fit_mem_dealloc: Linked list is empty, inserting dealloc node as linked list head\r\n");
#endif /* DEBUG_MEM */

        new_node = (node_t *) dealloc_ptr;
        new_node->size = dealloc_ptr->size + sizeof(used_mem_node_t) - sizeof(node_t);
        new_node->prev = NULL;
        new_node->next = NULL;
        free_mem_head = new_node;

        print_linked_list("first_fit_mem_dealloc");
        return RTX_OK;
    } else {
        while (cur_node != NULL) {
            if ((void *)cur_node > (void *) dealloc_ptr) {
#ifdef DEBUG_MEM
                printf("first_fit_mem_dealloc: Inserting node before node at address 0x%x\r\n", cur_node);
#endif /* DEBUG_MEM */

                new_node = (node_t *) dealloc_ptr;
                new_node->size = dealloc_ptr->size + sizeof(used_mem_node_t) - sizeof(node_t);
                new_node->prev = cur_node->prev;
                cur_node->prev = new_node;
                new_node->next = cur_node;

                if (new_node->prev != NULL) {
                    new_node->prev->next = new_node;
                } else {
                    free_mem_head = new_node;
                }

                print_linked_list("first_fit_mem_dealloc");
                break;
            }

            if (cur_node->next == NULL) {
#ifdef DEBUG_MEM
                printf("first_fit_mem_dealloc: Inserting dealloc node at the end\r\n");
#endif /* DEBUG_MEM */

                new_node = (node_t *) dealloc_ptr;
                new_node->size = dealloc_ptr->size + sizeof(used_mem_node_t) - sizeof(node_t);
                new_node->prev = cur_node;
                new_node->next = NULL;
                cur_node->next = new_node;
            }

            cur_node = cur_node->next;
        }

        if (new_node) {
            if (new_node->prev != NULL && new_node == (node_t *) ((char *) new_node->prev + sizeof(node_t) + new_node->prev->size)) {
#ifdef DEBUG_MEM
                printf("first_fit_mem_dealloc: Coalescing with previous node\r\n");
#endif /* DEBUG_MEM */

                new_node->prev->size = new_node->prev->size + new_node->size + sizeof(node_t);
                new_node->prev->next = new_node->next;
                new_node->next->prev = new_node->prev;
                new_node = new_node->prev;

#ifdef DEBUG_MEM
                printf("first_fit_mem_dealloc: coalesced node address 0x%x\r\n", new_node);
                printf("first_fit_mem_dealloc: coalesced node size 0x%x\r\n", new_node->size);
                printf("first_fit_mem_dealloc: coalesced node prev 0x%x\r\n", new_node->prev);
                printf("first_fit_mem_dealloc: coalesced node next 0x%x\r\n", new_node->next);
#endif /* DEBUG_MEM */
            }

            if (new_node->next != NULL && new_node->next == (node_t *) ((char*) new_node + sizeof(node_t) + new_node->size)) {
#ifdef DEBUG_MEM
                printf("first_fit_mem_dealloc: Coalescing with next node\r\n");
#endif /* DEBUG_MEM */

                new_node->size = new_node->size + new_node->next->size + sizeof(node_t);
                new_node->next = new_node->next->next;

                if (new_node->next->next) {
                    new_node->next->next->prev = new_node;
                }

#ifdef DEBUG_MEM
                printf("first_fit_mem_dealloc: coalesced node address 0x%x\r\n", new_node);
                printf("first_fit_mem_dealloc: coalesced node size 0x%x\r\n", new_node->size);
                printf("first_fit_mem_dealloc: coalesced node prev 0x%x\r\n", new_node->prev);
                printf("first_fit_mem_dealloc: coalesced node next 0x%x\r\n", new_node->next);
#endif /* DEBUG_MEM */
            }
        }

        print_linked_list("first_fit_mem_dealloc");
        return RTX_OK;
    }
}


int first_fit_count_extfrag(size_t size) {

    if ((int)size <= 0) {
#ifdef DEBUG_MEM
        printf("first_fit_mem_alloc: size is less than 1, size = %d\r\n", size);
#endif /* DEBUG_MEM */
        return 0;
    }
    else if((int)size==NULL){
#ifdef DEBUG_MEM
        printf("first_fit_mem_alloc: size is null");
#endif /* DEBUG_MEM */
        return 0;
    }



    node_t *cur_node = free_mem_head;
    int counter = 0;

    while (cur_node != NULL) {
        if (cur_node->size + sizeof(node_t) < size){
            counter++;
        }
        cur_node = cur_node->next;
    }

#ifdef DEBUG_MEM
    printf("first_fit_count_extfrag: external fragmentation %d\r\n", counter);
#endif /* DEBUG_MEM */

    return counter;
}


void print_linked_list(char *prefix) {
#ifdef DEBUG_MEM
    node_t *cur_node = free_mem_head;
    int index = 0;

		printf("Printing linked list\r\n");
    while (cur_node != NULL) {
        printf("%s: Node{%d} address 0x%x\r\n", prefix, index, cur_node);
        printf("%s: Node{%d} size 0x%x\r\n", prefix, index, cur_node->size);
        printf("%s: Node{%d} prev 0x%x\r\n", prefix, index, cur_node->prev);
        printf("%s: Node{%d} next 0x%x\r\n", prefix, index, cur_node->next);
        cur_node = cur_node->next;
        index++;
    }
#endif /* DEBUG_MEM */
}


int mem_cpy(void *destination, void *source, size_t size) {
    if (destination == NULL || source == NULL) {
        return RTX_ERR;
    }

    if (size <= 0) {
        return RTX_ERR;
    }

    char* dest = destination;
    char* src = source;
    for (int i = 0; i < n; i++) {
        dest[i] = src[i];
    }

    return RTX_OK;
}