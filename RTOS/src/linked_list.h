//
// Created by Boris Nguyen on 2020-11-23.
//

#ifndef ECE350_LINKED_LIST_H
#define ECE350_LINKED_LIST_H

#include "k_rtx.h"

typedef struct free_tid {
    int tid;
    struct free_tid *next;
} INT_LL_NODE_T;

void push_tid(INT_LL_NODE_T **free_tid_head, INT_LL_NODE_T *new_tid);
INT_LL_NODE_T *pop_tid(INT_LL_NODE_T **free_tid_head);
void print_free_tids(INT_LL_NODE_T *free_tid_head);
int tid_is_available(INT_LL_NODE_T *free_tid_head, int tid);

TCB *pop(TCB **prio_queue_head);
TCB *pop_task_by_id(TCB **prio_queue_head, U8 tid);
void push(TCB **prio_queue_head, TCB *task);

TCB *pop_edf_queue(TCB **edf_queue_head);
void push_edf_queue(TCB **edf_queue_head, TCB *task);
void push_rm_queue(TCB **rm_queue_head, TCB *task);
void push_default_queue(TCB **default_queue_head, TCB *task);

TCB *pop_timeout_queue(TCB **timeout_queue_head);
void push_timeout_queue(TCB **timeout_queue_head, TCB *task, struct timeval_rt timeout);
int update_timeout(TCB **timeout_queue_head, struct timeval_rt passed_time);

int is_empty(TCB *queue_head);
void print_queue(TCB *queue_head);

#endif //ECE350_LINKED_LIST_H
