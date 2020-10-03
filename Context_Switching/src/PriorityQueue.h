#include "k_rtx.h"
  
typedef struct node { 
    TCB tcb; 
    int priority; /* Lower values for higher priority */
    struct node* next; 
  
} Node; 

typedef struct priorityQueue {
    Node *head;

} PriorityQueue;

Node* createNode(TCB tcb);
void pop(PriorityQueue** queue);
void push(PriorityQueue** queue, TCB tcb);
int isEmpty(Node** head);
