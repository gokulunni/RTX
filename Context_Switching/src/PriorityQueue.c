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
  
void push(PriorityQueue *queue, TCB *task)
{
    TCB *head = queue->head;

    if(head == NULL) {
        head = task;
        task->next = NULL;
    } else if (head->prio > task->prio) {  /* Edge Case: The head of list is lower priority then new node*/
        // Insert New Node before head 
        task->next = head;
        head = task;
    } else {
        /* Find position to insert new node */
        while (head->next != NULL && head->next->prio < task->prio) {
            head = head->next; 
        } 

        task->next = head->next;
        head->next = task;
    } 
}

int isEmpty(PriorityQueue *queue) {
    return queue->head == NULL;
} 
