//
// Created by Boris Nguyen on 2020-10-29.
//

#include "common.h"
#include "circular_buffer.h"
#ifdef DEBUG_CIRC_BUFF
#include "printf.h"
#endif /* ! DEBUG_CIRC_BUFF */


CIRCULAR_BUFFER_T *circular_buffer_init(CIRCULAR_BUFFER_T *mailbox, void *ptr, size_t size) {
    mailbox->buffer_start = ptr;
    mailbox->buffer_end = ptr + size;
    mailbox->head = ptr;
    mailbox->tail = ptr;
}

int is_circ_buf_empty(CIRCULAR_BUFFER_T *mailbox) {
    return mailbox->head == mailbox->tail;
}

int is_circ_buf_full(CIRCULAR_BUFFER_T *mailbox, U32 length) {
    if (mailbox->tail < mailbox->head && mailbox->tail + length < mailbox->head) {
        return RTX_ERR;
    } else if (mailbox->tail > mailbox->head) {
        if (mailbox->tail + length <= mailbox->buffer_end) {
            return RTX_ERR;
        } else if (mailbox->buffer_start + length - (mailbox->buffer_end - mailbox->tail) < mailbox->head) {
            return RTX_ERR;
        }
    }

    return RTX_OK;
}


U32 peek_msg_len(CIRCULAR_BUFFER_T *mailbox) {
    U32 len = 0;
    U32 res = 0;
    void *iterator = mailbox->head;

    for (int i = 4; i > 0; i--) {
        res = (*iterator) << (i-1);
        iterator = iterator + 1;

        if (iterator > mailbox->buffer_end) {
            iterator = mailbox->buffer_start;
        }
    }

    return res;
}

U32 peek_msg_type(CIRCULAR_BUFFER_T *mailbox) {
    U32 type = 0;
    U32 res = 0;
    void *iterator = mailbox->head;
    // Iterate thru first 4 bits, then do the same like in peek_msg_len

    if (iterator + 4 <= mailbox->buffer_end) {
        iterator = iterator + 4;
    } else {
        iterator = mailbox->buffer_start + 4 - (mailbox->buffer_end - iterator);
    }

    for (int i = 4; i > 0; i--) {
        res = (*iterator) << (i-1);
        iterator = iterator + 1;

        if (iterator > mailbox->buffer_end) {
            iterator = mailbox->buffer_start;
        }
    }

    return res;
}

int dequeue_msg(CIRCULAR_BUFFER_T *mailbox, void *buf, size_t buf_len) {
    if (mailbox->tail == mailbox->head) {
        return RTX_ERR;
    }

    U32 length = peek_msg_len(mailbox);

    if (length <= 0) {
        return RTX_ERR;
    }

    if (buf_len < length) {
        return RTX_ERR;
    }

    for (int i = 0; i < length; i++) {
        *(buf + i) = *(mailbox->head);

        mailbox->head = mailbox->head + 1;

        if (mailbox->head > mailbox->buffer_end) {
            mailbox->head = mailbox->buffer_start;
        }
    }

    return RTX_OK;
}

int enqueue_msg(CIRCULAR_BUFFER_T *mailbox, void *msg) {
    U32 length = *((U32 *) msg);

    if (length <= 0) {
        return RTX_ERR;
    }

    for (int i = 0; i < length; i++) {
        *(mailbox->tail) = msg + i;

        mailbox->tail = mailbox->tail + 1;

        if (mailbox->tail > mailbox->buffer_end) {
            mailbox->tail = mailbox->buffer_start;
        }
    }

    return RTX_OK;
}
