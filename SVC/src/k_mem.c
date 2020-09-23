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

void print_linked_list();
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
    free_mem_head->size = IRAM1_END - end_addr - 4 - sizeof(node_t);
    free_mem_head->prev = NULL;
    free_mem_head->next = NULL;

#ifdef DEBUG_0
    printf("Free memory block metadata node size 0x%x\r\n", sizeof(node_t));
    printf("Used memory block metadata node size 0x%x\r\n", sizeof(used_mem_node_t));
    printf("First fit linked list address 0x%x at init\r\n", free_mem_head);
    printf("First fit head node size 0x%x at init\r\n", free_mem_head->size);
    printf("First fit head node prev 0x%x at init\r\n", free_mem_head->prev);
    printf("First fit head node next 0x%x at init\r\n", free_mem_head->next);
#endif /* DEBUG_0 */

    return RTX_OK;
}


void *first_fit_mem_alloc(size_t size) {
    int mem_chunk_size;
    node_t *new_node;
    used_mem_node_t *ret_node;
    node_t *cur_node = free_mem_head;
	
    if (size <= 0) {
        return NULL;
    }

#ifdef DEBUG_0
    printf("Linked list of free memory blocks before allocation\r\n");
    print_linked_list();
#endif /* DEBUG_0 */
    
    mem_chunk_size = CEIL((size + sizeof(used_mem_node_t)), mem_blk_size) * mem_blk_size;
#ifdef DEBUG_0
    printf("Return memory block size: 0x%x\r\n", mem_chunk_size);
#endif /* DEBUG_0 */

    while (cur_node != NULL) {
        if (cur_node->size + sizeof(node_t) > mem_chunk_size) {
#ifdef DEBUG_0
            printf("Splitting a memory block size\r\n", cur_node->size + sizeof(node_t));
            printf("Current node address 0x%x before splitting\r\n", cur_node);
            printf("Current node size 0x%x before splitting\r\n", cur_node->size);
            printf("Current node prev 0x%x before splitting\r\n", cur_node->prev);
            printf("Current node next 0x%x before splitting\r\n", cur_node->next);
#endif /* DEBUG_0 */

            new_node = (node_t *) ((char *) cur_node + mem_chunk_size);
            new_node->size = cur_node->size - mem_chunk_size;

            if (free_mem_head == cur_node) {
                free_mem_head = new_node;
            }

            new_node->next = cur_node->next;
            new_node->prev = cur_node->prev;

#ifdef DEBUG_0
            printf("New node address 0x%x after splitting\r\n", new_node);
            printf("New node size 0x%x after splitting\r\n", new_node->size);
            printf("New node prev 0x%x after splitting\r\n", new_node->prev);
            printf("New node next 0x%x after splitting\r\n", new_node->next);
#endif /* DEBUG_0 */

            ret_node = (used_mem_node_t *) cur_node;
            ret_node->size = mem_chunk_size;

#ifdef DEBUG_0
            printf("Used node address 0x%x after splitting\r\n", ret_node);
            printf("New node size 0x%x after splitting\r\n", ret_node->size);
#endif /* DEBUG_0 */
            print_linked_list();
            return ret_node + 1;
        } else if (cur_node->size + sizeof(node_t) == mem_chunk_size) {
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

#ifdef DEBUG_0
            printf("Used node address 0x%x after splitting\r\n", ret_node);
            printf("New node size 0x%x after splitting\r\n", ret_node->size);
#endif /* DEBUG_0 */
            print_linked_list();
            return ret_node + 1;
        }
        cur_node = cur_node->next;
    }
		
    return NULL;
}


void first_fit_mem_dealloc(void *ptr) {
    node_t *cur_node = free_mem_head;
    node_t *new_node;
    used_mem_node_t *dealloc_ptr = (used_mem_node_t *) ptr - 1;

    if (free_mem_head == NULL) {
        new_node = (node_t *) dealloc_ptr;
        new_node->size = dealloc_ptr->size + sizeof(dealloc_ptr) - sizeof(new_node);
        new_node->prev = NULL;
        new_node->next = NULL;
        free_mem_head = new_node;
    }

    while (cur_node != NULL) {
        if ((void *)cur_node > (void *) dealloc_ptr) {
            new_node = (node_t *) dealloc_ptr;
            new_node->size = dealloc_ptr->size + sizeof(dealloc_ptr) - sizeof(new_node);
            new_node->prev = cur_node->prev;
            cur_node->prev = new_node;
            new_node->next = cur_node;
            if (new_node->prev != NULL) {
                new_node->prev->next = new_node;
            } else {
                free_mem_head = new_node;
            }
        }

        cur_node = cur_node->next;
    }

    if (new_node) {
        if (new_node->prev != NULL && new_node == (node_t *) ((char *) new_node->prev + sizeof(new_node->prev) + new_node->prev->size + 1)) {
            new_node->prev->size = new_node->prev->size + new_node->size + sizeof(node_t);
            new_node->prev->next = new_node->next;
            new_node = new_node->prev;
        }

        if (new_node->next != NULL && new_node->next == (node_t *) ((char*) new_node + sizeof(new_node) + new_node->size + 1)) {
            new_node->size = new_node->size + new_node->next->size + sizeof(node_t);
            new_node->next = new_node->next->next;
        }
    }
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

void print_linked_list() {
#ifdef DEBUG_0
    node_t *cur_node = free_mem_head;
    int index = 0;
    while (cur_node != NULL) {
        printf("Node{%d} address 0x%x\r\n", index, cur_node);
        printf("Node{%d} size 0x%x\r\n", index, cur_node->size);
        printf("Node{%d} prev 0x%x\r\n", index, cur_node->prev);
        printf("Node{%d} next 0x%x\r\n", index, cur_node->next);
        cur_node = cur_node->next;
        index++;
    }
#endif /* DEBUG_0 */
}
