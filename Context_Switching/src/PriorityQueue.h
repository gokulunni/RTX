#include "k_rtx.h"

typedef struct priorityQueue {
    TCB *head;
} PriorityQueue;

TCB* get_task_by_id(PriorityQueue** queue, U8 tid);
TCB* pop(PriorityQueue** queue);
void push(PriorityQueue** queue, TCB* task);
int isEmpty(PriorityQueue** queue);
