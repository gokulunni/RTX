#include "PriorityQueue.h"
#include "k_mem.h"

Node* createNode(TCB tcb) 
{ 
    Node* temp = (Node*)k_mem_alloc(sizeof(Node)); 
    temp->tcb = tcb; 
    temp->priority = tcb.prio; 
    temp->next = NULL; 
  
    return temp; 
} 

void pop(PriorityQueue** queue) 
{ 
    Node* temp = (*queue)->head; 
    (*queue)->head = (*queue)->head->next; 
    k_mem_dealloc(temp); 
} 
  
void push(PriorityQueue** queue, TCB tcb) 
{ 

    Node *head = (*queue) -> head;

    Node* temp = createNode(tcb); 

    if(head == NULL)
        head = temp;
  
    /* Edge Case: The head of list is lower 
        priority then new node*/
    else if (head->priority > tcb.prio) { 
  
        // Insert New Node before head 
        temp->next = head; 
        head = temp; 
    } 
    else { 
  
        /* Find position to insert new node */
        while (head->next != NULL && 
               head->next->priority < tcb.prio) { 
            head = head->next; 
        } 

        temp->next = head->next; 
        head->next = temp; 
    } 
} 

int isEmpty(Node** head) 
{ 
    return (*head) == NULL; 
} 
