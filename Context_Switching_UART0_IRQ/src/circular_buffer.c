//
// Created by Boris Nguyen on 2020-10-29.
//

#include "common.h"
#include "circular_buffer.h"
#ifdef DEBUG_CIRC_BUFF
#include "printf.h"
#endif /* ! DEBUG_CIRC_BUFF */


int circular_buffer_init(CIRCULAR_BUFFER_T *mailbox, void *ptr, size_t size) {
		if (size < 1) {
			return RTX_ERR;
		}
	
    mailbox->buffer_start = ptr;
    mailbox->buffer_end = (char *) ptr + size;
    mailbox->head = ptr;
    mailbox->tail = ptr;
	
		return RTX_OK;
}

int is_circ_buf_empty(CIRCULAR_BUFFER_T *mailbox) {
    return mailbox->head == mailbox->tail;
}

int is_circ_buf_full(CIRCULAR_BUFFER_T *mailbox, U32 length) {
    if (mailbox->tail < mailbox->head && (char *) mailbox->tail + length < mailbox->head) {
        return 0;
    } else if (mailbox->tail > mailbox->head) {
        if ((char *) mailbox->tail + length <= mailbox->buffer_end) {
            return 0;
        } else if ((char *) mailbox->buffer_start + length - ((char *) mailbox->buffer_end - (char *) mailbox->tail) < mailbox->head) {
            return 0;
        }
    }

    return 1;
}


U32 peek_msg_len(CIRCULAR_BUFFER_T *mailbox) {
    U32 res = 0;
    void *iterator = mailbox->head;

    for (int i = 4; i > 0; i--) {
        res = (U32) *((char *) iterator) << (i-1);
        iterator = (char *) iterator + 1;

        if (iterator > mailbox->buffer_end) {
            iterator = mailbox->buffer_start;
        }
    }

    return res;
}

U32 peek_msg_type(CIRCULAR_BUFFER_T *mailbox) {
    U32 res = 0;
    void *iterator = mailbox->head;
    // Iterate thru first 4 bits, then do the same like in peek_msg_len

    if ((void *)((char *) iterator + 4) <= mailbox->buffer_end) {
        iterator = (char *) iterator + 4;
    } else {
        iterator = (char *) mailbox->buffer_start + 4 - ((char *) mailbox->buffer_end - (char *) iterator);
    }

    for (int i = 4; i > 0; i--) {
        res = (U32) *((char *) iterator) << (i-1);
        iterator = (char *) iterator + 1;

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
        *((char *) buf + i) = *((char *) mailbox->head);

        mailbox->head = (char *) mailbox->head + 1;

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
        *((char *) mailbox->tail) = *((char *) msg + i);

        mailbox->tail = (char *) mailbox->tail + 1;

        if (mailbox->tail > mailbox->buffer_end) {
            mailbox->tail = mailbox->buffer_start;
        }
    }

    return RTX_OK;
}
