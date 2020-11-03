//
// Created by Boris Nguyen on 2020-10-29.
//

#ifndef ECE350_CIRCULAR_BUFFER_H
#define ECE350_CIRCULAR_BUFFER_H

typedef struct circular_buffer {
    void *buffer_start; // DO NOT CHANGE AFTER INIT
    void *buffer_end;   // DO NOT CHANGE AFTER INIT
    void *head;
    void *tail;
} CIRCULAR_BUFFER_T;

CIRCULAR_BUFFER_T *circular_buffer_init(CIRCULAR_BUFFER_T *mailbox, void *ptr, size_t size)
int is_empty(CIRCULAR_BUFFER_T *mailbox);
int is_full(CIRCULAR_BUFFER_T *mailbox, U32 length);
U32 peek_msg_len(CIRCULAR_BUFFER_T *mailbox);
U32 peek_msg_type(CIRCULAR_BUFFER_T *mailbox);
int dequeue_msg(CIRCULAR_BUFFER_T *mailbox, void *buf, size_t buf_len);
int enqueue_msg(CIRCULAR_BUFFER_T *mailbox, void *msg);


TCB *pop(TCB **prio_queue_head);

#endif //ECE350_CIRCULAR_BUFFER_H
