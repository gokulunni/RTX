//
// Created by Boris Nguyen on 2020-10-14.
//

#ifndef ECE350_LINKED_LIST_H
#define ECE350_LINKED_LIST_H

#include "k_rtx.h"

typedef struct free_tid {
    int tid;
    struct free_tid *next;
} FREE_TID_T;

void push_tid(FREE_TID_T **free_tid_head, FREE_TID_T *new_tid);
FREE_TID_T *pop_tid(FREE_TID_T **free_tid_head);
void print_free_tids(FREE_TID_T *free_tid_head);
int tid_is_available(FREE_TID_T *free_tid_head, int tid);

TCB *pop(TCB **prio_queue_head);
TCB *pop_task_by_id(TCB **prio_queue_head, U8 tid);
void push(TCB **prio_queue_head, TCB *task);
int is_empty(TCB *prio_queue_head);
void print_prio_queue(TCB *prio_queue_head);

#endif //ECE350_LINKED_LIST_H
