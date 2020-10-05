#include "PriorityQueue.h"

TCB* pop(PriorityQueue** queue)
{ 
    Node* popped = (*queue)->head;
    if ((*queue)->head) {
        (*queue)->head = (*queue)->head->next;
    } else {
        (*queue)->head = NULL;
    }

    return popped;
} 
  
void push(PriorityQueue** queue, TCB* task)
{
    Node *head = (*queue) -> head;

    if(head == NULL) {
        head = temp;
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

int isEmpty(PriorityQueue** queue) {
    return (*queue)->head == NULL; 
} 
