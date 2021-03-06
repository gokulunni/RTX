//
// Created by Boris Nguyen on 2020-10-14.
//

#include "linked_list.h"
#include "k_rtx.h"
#ifdef DEBUG_PRIO_Q
#include "printf.h"
#endif /* ! DEBUG_PRIO_Q */


/**
 * Free TID Linked List
 */

void push_tid(FREE_TID_T **free_tid_head, FREE_TID_T *new_tid) {
    new_tid->next = *free_tid_head;
    *free_tid_head = new_tid;
}

FREE_TID_T *pop_tid(FREE_TID_T **free_tid_head) {
    if (*free_tid_head == NULL) {
        return NULL;
    }

    FREE_TID_T *popped = *free_tid_head;
    *free_tid_head = (*free_tid_head)->next;
    return popped;
}

void print_free_tids(FREE_TID_T *free_tid_head) {
#ifdef DEBUG_TID_LL
    printf("****************\n");
    printf("Free TID List\n");

    FREE_TID_T *temp = free_tid_head;
    while(temp != NULL) {
        printf("%d\n", temp->tid);
        temp = temp->next;
    }
    printf("****************\n");
#endif /* DEBUG_TID_LL */
}

int tid_is_available(FREE_TID_T *free_tid_head, int tid) {
    FREE_TID_T *temp = free_tid_head;
    while(temp != NULL) {
        if (temp->tid == tid) {
            return 1;
        }
        temp = temp->next;
    }

    return 0;
}

/**
 * PRIORITY QUEUE
 */

/**
 * @brief pop TCB at the head of priority queue
 * @return NULL if priority queue is empty
 * Otherwise, return popped TCB
 */
TCB *pop(TCB **prio_queue_head) {
    TCB *popped = *prio_queue_head;

    if (popped) {
        *prio_queue_head = (*prio_queue_head)->next;
        popped->next = NULL;
    } else {
        *prio_queue_head = NULL;
    }

    return popped;
}


/**
 * @brief pop TCB with given tid from priority queueu
 * @return NULL if TCB with requested tid is not found
 * Otherwise, return popped TCB
 */
TCB *pop_task_by_id(TCB **prio_queue_head, U8 tid) {
    TCB *iterator = *prio_queue_head;
    TCB *prev_node = NULL;

    if (*prio_queue_head == NULL) {
        return NULL;
    }

    // Iterate through priority queue until TCB with tid is found
    while (iterator != NULL && iterator->tid != tid) {
        prev_node = iterator;
        iterator = iterator->next;
    }

    // If TCB with tid is not found, return NULL
    if (iterator == NULL || iterator->tid != tid) {
        return NULL;
    }

    if (iterator == *prio_queue_head) {
        *prio_queue_head = (*prio_queue_head)->next;
    }

    if (prev_node) {
        prev_node->next = iterator->next;
    }

    iterator->next = NULL;

    return iterator;
}


/**
 * @brief Push task into the sorted priority queue
 */
void push(TCB **prio_queue_head, TCB *task) {
    if(*prio_queue_head == NULL) {
        // Priority queue is empty, insert new task as the head node
        *prio_queue_head = task;
        task->next = NULL;
    } else if ((*prio_queue_head)->prio > task->prio) {
        // Insert task before head node since head of the list is lower priority than the new task
        task->next = *prio_queue_head;
        *prio_queue_head = task;
    } else {
        TCB *iterator = *prio_queue_head;

        /* Find position to insert new node */
        while (iterator->next != NULL && iterator->next->prio <= task->prio) {
            iterator = iterator->next;
        }

        task->next = iterator->next;
        iterator->next = task;
    }
}


/**
 * @brief print priority queue when DEBUG_PRIO_Q
 */
void print_prio_queue(TCB *prio_queue_head) {
#ifdef DEBUG_PRIO_Q
    printf("******************************************************\r\n");
	printf("PRINT_PRIO_QUEUE\r\n");

    TCB *iterator = prio_queue_head;
    int counter = 0;

    while (iterator != NULL) {
        printf("Node %d: TID = %d\r\n", counter, iterator->tid);
        printf("Node %d: MSP = 0x%x\r\n", counter, iterator->msp);
        printf("Node %d: PSP = 0x%x\r\n", counter, iterator->psp);

        switch (iterator->prio) {
            case HIGH:
                printf("Node %d: PRIO = HIGH\r\n", counter);
                break;
            case MEDIUM:
                printf("Node %d: PRIO = MEDIUM\r\n", counter);
                break;
            case LOW:
                printf("Node %d: PRIO = LOW\r\n", counter);
                break;
            case LOWEST:
                printf("Node %d: PRIO = LOWEST\r\n", counter);
                break;
            case PRIO_NULL:
                printf("Node %d: PRIO = NULL\r\n", counter);
                break;
        }

        printf("Node %d: STATE = %d\r\n", counter, iterator->state);

        if (iterator->priv) {
            printf("Node %d: Privileged\r\n", counter);
        } else {
            printf("Node %d: Unprivileged\r\n", counter);
        }

        counter++;
        iterator = iterator->next;
    }

    printf("******************************************************\r\n");
#endif /* DEBUG_PRIO_Q */
}


/**
 * @brief checks if priority queue is empty or not
 * @return 1 if priority queue is not empty
 */
int is_empty(TCB *prio_queue_head) {
    return prio_queue_head == NULL;
}
