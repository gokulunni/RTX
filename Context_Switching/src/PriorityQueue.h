#include "k_rtx.h"

typedef struct priorityQueue {
    Node *head;
} PriorityQueue;

TCB* pop(PriorityQueue** queue);
void push(PriorityQueue** queue, TCB* task);
int isEmpty(PriorityQueue** queue);
