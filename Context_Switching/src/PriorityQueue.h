#include "k_rtx.h"
  
typedef struct node { 
    TCB *tcb; 
    int priority; /* Lower values for higher priority */
    struct node* next; 
  
} Node; 

typedef struct priorityQueue {
    Node *head;

} PriorityQueue;

Node* createNode(TCB tcb);
Node* pop(PriorityQueue** queue);
void push(PriorityQueue** queue, Node* task);
int isEmpty(PriorityQueue** queue);
