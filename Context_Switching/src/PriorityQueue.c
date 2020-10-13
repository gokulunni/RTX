#include "PriorityQueue.h"
#include "k_rtx.h"
#ifdef DEBUG_PRIO_Q
#include "printf.h"
#endif /* ! DEBUG_PRIO_Q */

TCB *get_task_by_id(PriorityQueue *queue, U8 tid) {
    TCB *iterator = queue->head;

    while (iterator != NULL && iterator->tid != tid) {
        iterator = iterator->next;
    }

    return iterator;
}

TCB *pop(PriorityQueue *queue)
{
    TCB *popped = queue->head;
    if (queue->head) {
        queue->head = queue->head->next;
    } else {
        queue->head = NULL;
    }

    popped->next = NULL;
    return popped;
} 
  
void push(PriorityQueue *queue, TCB *task) {
    TCB *iterator = queue->head;
	
    if(queue->head == NULL) {
        queue->head = task;
        task->next = NULL;
    } else if (queue->head->prio > task->prio) {  /* Edge Case: The head of list is lower priority then new node*/
        // Insert New Node before head 
        task->next = queue->head;
        queue->head = task;
    } else {
        /* Find position to insert new node */
        while (iterator -> next != NULL && iterator->prio <= task->prio) {
            iterator = iterator->next;
        } 

        task->next = iterator->next;
        iterator->next = task;
    } 
}

int is_empty(PriorityQueue *queue) {
    return queue->head == NULL;
} 

void print_priority_queue(PriorityQueue *queue) {
    #ifdef DEBUG_PRIO_Q
    printf("******************************************************\r\n");
	printf("print_priority_queue\r\n");

    TCB *iterator = queue->head;
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

        // Should always be 1
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
