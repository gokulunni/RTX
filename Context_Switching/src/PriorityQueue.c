#include "PriorityQueue.h"
#include "k_rtx.h"

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
		TCB *Iterator = queue->head;
	
    if(queue->head == NULL) {
        queue->head = task;
        task->next = NULL;
    } else if (queue->head->prio > task->prio) {  /* Edge Case: The head of list is lower priority then new node*/
        // Insert New Node before head 
        task->next = queue->head;
        queue->head = task;
    } else {
        /* Find position to insert new node */
        while (Iterator->next != NULL && Iterator->next->prio > task->prio) {
            Iterator = Iterator->next;
        } 

        task->next = Iterator->next;
        Iterator->next = task;
    } 
}

int isEmpty(PriorityQueue *queue) {
    return queue->head == NULL;
} 
