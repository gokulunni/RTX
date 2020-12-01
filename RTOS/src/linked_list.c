//
// Created by Boris Nguyen on 2020-11-23.
//

#include "linked_list.h"
#include "k_rtx.h"
#include "timeval.h"
#ifdef DEBUG_QUEUE
#include "printf.h"
#endif /* ! DEBUG_QUEUE */

/**
 * Free TID Linked List
 */

void push_tid(INT_LL_NODE_T **free_tid_head, INT_LL_NODE_T *new_tid) {
    new_tid->next = *free_tid_head;
    *free_tid_head = new_tid;
}

INT_LL_NODE_T *pop_tid(INT_LL_NODE_T **free_tid_head) {
    if (*free_tid_head == NULL) {
        return NULL;
    }

    INT_LL_NODE_T *temp_prev = NULL;
    INT_LL_NODE_T *temp = *free_tid_head;
    while (temp->next != NULL) {
        temp_prev = temp;
        temp = temp->next;
    }

    if (temp_prev != NULL) {
        temp_prev->next = NULL;
    } else {
        *free_tid_head = NULL;
    }

    INT_LL_NODE_T *popped = temp;

    return popped;
}

void print_free_tids(INT_LL_NODE_T *free_tid_head) {
#ifdef DEBUG_TID_LL
    printf("****************\n");
    printf("Free TID List\n");

    INT_LL_NODE_T *temp = free_tid_head;
    while(temp != NULL) {
        printf("%d\n", temp->tid);
        temp = temp->next;
    }
    printf("****************\n");
#endif /* DEBUG_TID_LL */
}

int tid_is_available(INT_LL_NODE_T *free_tid_head, int tid) {
    INT_LL_NODE_T *temp = free_tid_head;
    while(temp != NULL) {
        if (temp->tid == tid) {
            return 1;
        }
        temp = temp->next;
    }

    return 0;
}

/**
 * PRIORITY QUEUE
 */

/**
 * @brief pop TCB at the head of priority queue
 * @return NULL if priority queue is empty
 * Otherwise, return popped TCB
 */
TCB *pop(TCB **prio_queue_head) {
    TCB *popped = *prio_queue_head;

    if (popped) {
        *prio_queue_head = (*prio_queue_head)->next;
        popped->next = NULL;
    } else {
        *prio_queue_head = NULL;
    }

    return popped;
}


/**
 * @brief pop TCB with given tid from priority queueu
 * @return NULL if TCB with requested tid is not found
 * Otherwise, return popped TCB
 */
TCB *pop_task_by_id(TCB **prio_queue_head, U8 tid) {
    TCB *iterator = *prio_queue_head;
    TCB *prev_node = NULL;

    if (*prio_queue_head == NULL) {
        return NULL;
    }

    // Iterate through priority queue until TCB with tid is found
    while (iterator != NULL && iterator->tid != tid) {
        prev_node = iterator;
        iterator = iterator->next;
    }

    // If TCB with tid is not found, return NULL
    if (iterator == NULL || iterator->tid != tid) {
        return NULL;
    }

    if (iterator == *prio_queue_head) {
        *prio_queue_head = (*prio_queue_head)->next;
    }

    if (prev_node) {
        prev_node->next = iterator->next;
    }

    iterator->next = NULL;

    return iterator;
}


/**
 * @brief Push task into the sorted priority queue
 */
void push(TCB **prio_queue_head, TCB *task) {
    if(*prio_queue_head == NULL) {
        // Priority queue is empty, insert new task as the head node
        *prio_queue_head = task;
        task->next = NULL;
    } else if ((*prio_queue_head)->prio > task->prio) {
        // Insert task before head node since head of the list is lower priority than the new task
        task->next = *prio_queue_head;
        *prio_queue_head = task;
    } else {
        TCB *iterator = *prio_queue_head;

        /* Find position to insert new node */
        while (iterator->next != NULL && iterator->next->prio <= task->prio) {
            iterator = iterator->next;
        }

        task->next = iterator->next;
        iterator->next = task;
    }
}

/**
 * @brief checks if priority queue is empty or not
 * @return 1 if priority queue is not empty
 */
int is_empty(TCB *queue_head) {
    return queue_head == NULL;
}

/**
 * EDF QUEUE
 */


TCB *pop_edf_queue(TCB **edf_queue_head) {
    TCB *popped = *edf_queue_head;

    if (popped) {
        *edf_queue_head = (*edf_queue_head)->next;
        popped->next = NULL;
    } else {
        *edf_queue_head = NULL;
    }

    return popped;
}

void push_edf_queue(TCB **edf_queue_head, TCB *task) {
    if (*edf_queue_head == NULL || is_greater((*edf_queue_head)->deadline, task->deadline)) {
        task->next = *edf_queue_head;
        *edf_queue_head = task;
    } else {
        TCB *iterator = *edf_queue_head;

        while (iterator->next != NULL && is_less_equal(iterator->next->deadline, task->deadline)) {
            iterator = iterator->next;
        }

        task->next = iterator->next;
        iterator->next = task;
    }
}

void push_rm_queue(TCB **rm_queue_head, TCB *task)	{
	if (*rm_queue_head == NULL || is_greater((*rm_queue_head)->p_n, task->p_n))	{
		task->next = *rm_queue_head;
		*rm_queue_head = task;
	} else {
		TCB *iterator = *rm_queue_head;
		
		while (iterator->next != NULL && is_less_equal(iterator->next->deadline, task->deadline)) {
			iterator = iterator->next;
		}
		
		task->next = iterator->next;
		iterator->next = task;
	}
}

void push_default_queue(TCB **default_queue_head, TCB *task)	{
	task->next = *default_queue_head;
	*default_queue_head = task;
}

/**
 * TIMEOUT QUEUE
 */

TCB *pop_timeout_queue(TCB **timeout_queue_head) {
    TCB *popped = *timeout_queue_head;

    if (popped) {
        *timeout_queue_head = (*timeout_queue_head)->next;
        popped->next = NULL;
    } else {
        *timeout_queue_head = NULL;
    }

    return popped;
}


void push_timeout_queue(TCB **timeout_queue_head, TCB *task, struct timeval_rt timeout) {
    struct timeval_rt timeout_sum = {0, 0};
    TCB *iterator = *timeout_queue_head;

    if (*timeout_queue_head == NULL) {
        task->timeout.sec = timeout.sec;
        task->timeout.usec = timeout.usec;
        *timeout_queue_head = task;
        task->next = NULL;
        return;
    } else if (is_greater((*timeout_queue_head)->timeout, task->deadline)) {
        task->timeout.sec = timeout.sec;
        task->timeout.usec = timeout.usec;
        task->next = *timeout_queue_head;
        sub(&(task->next->timeout), task->next->timeout, timeout);
        *timeout_queue_head = task;

        iterator = task->next;
        while (iterator != NULL) {
            sub(&(iterator->timeout), iterator->timeout, task->timeout);
            iterator = iterator->next;
        }
    } else {
        add(&timeout_sum, timeout_sum, iterator->timeout);
        while (iterator->next != NULL) {
            add(&timeout_sum, timeout_sum, iterator->next->timeout);
            if (is_greater(timeout_sum, timeout)) {
                break;
            }
            iterator = iterator->next;
        }

        sub(&(task->timeout), timeout, timeout_sum);
        task->next = iterator->next;
        iterator->next = task;
    }

    iterator = task->next;
    while (iterator != NULL) {
        sub(&(iterator->timeout), iterator->timeout, task->timeout);
        iterator = iterator->next;
    }
}


int update_timeout(TCB **timeout_queue_head, struct timeval_rt passed_time) {
    if (*timeout_queue_head == NULL) {
        return 0;
    }

    sub(&((*timeout_queue_head)->timeout), (*timeout_queue_head)->timeout, passed_time);
    return 1;
}

/**
 * PRINT TCB QUEUE
 */

void print_queue(TCB *queue_head) {
#ifdef DEBUG_QUEUE
    printf("******************************************************\r\n");
	printf("PRINT_PRIO_QUEUE\r\n");

    TCB *iterator = queue_head;
    int counter = 0;

    while (iterator != NULL) {
        printf("Node %d: TID = %d\r\n", counter, iterator->tid);
        printf("Node %d: MSP = 0x%x\r\n", counter, iterator->msp);
        printf("Node %d: MSP_HI = 0x%x\r\n", counter, iterator->msp_hi);
        printf("Node %d: PSP = 0x%x\r\n", counter, iterator->psp);
        printf("Node %d: PSP_HI = 0x%x\r\n", counter, iterator->psp_hi);
        printf("Node %d: PSP_SIZE = 0x%x\r\n", counter, iterator->psp_size);

        if (iterator->priv) {
            printf("Node %d: Privileged\r\n", counter);
        } else {
            printf("Node %d: Unprivileged\r\n", counter);
        }

        switch (iterator->prio) {
            case PRIO_RT:
                printf("Node %d: PRIO = PRIO_RT\r\n", counter);
                break;
            case HIGH:
                printf("Node %d: PRIO = HIGH\r\n", counter);
                break;
            case MEDIUM:
                printf("Node %d: PRIO = MEDIUM\r\n", counter);
                break;
            case LOW:
                printf("Node %d: PRIO = LOW\r\n", counter);
                break;
            case LOWEST:
                printf("Node %d: PRIO = LOWEST\r\n", counter);
                break;
            case PRIO_NULL:
                printf("Node %d: PRIO = NULL\r\n", counter);
                break;
        }

        printf("Node %d: STATE = %d\r\n", counter, iterator->state);

        switch (iterator->state) {
            case READY:
                printf("Node %d: STATE = READY\r\n", counter);
                break;
            case RUNNING:
                printf("Node %d: STATE = RUNNING\r\n", counter);
                break;
            case BLK_MEM:
                printf("Node %d: STATE = BLK_MEM\r\n", counter);
                break;
            case BLK_MSG:
                printf("Node %d: STATE = BLK_MSG\r\n", counter);
                break;
            case SUSPENDED:
                printf("Node %d: STATE = SUSPENDED\r\n", counter);
                break;
            case NEW:
                printf("Node %d: STATE = NEW\r\n", counter);
                break;
            case DORMANT:
                printf("Node %d: STATE = DORMANT\r\n", counter);
                break;
        }

        printf("Node %d: MAILBOX = %d\r\n", counter, iterator->has_mailbox);

        printf("Node %d: tv_cpu.sec = %d, tv_cpu.usec = %d\r\n", counter, iterator->tv_cpu.sec, iterator->tv_cpu.usec);
        printf("Node %d: tv_wall.sec = %d, tv_wall.usec = %d\r\n", counter, iterator->tv_wall.sec, iterator->tv_wall.usec);

        if (iterator->prio == PRIO_RT) {
            printf("Node %d: period.sec = %d, period.usec = %d\r\n", counter, iterator->p_n.sec, iterator->p_n.usec);
            printf("Node %d: deadline.sec = %d, deadline.usec = %d\r\n", counter, iterator->deadline.sec, iterator->deadline.usec);
            printf("Node %d: timeout.sec = %d, timeout.usec = %d\r\n", counter, msg_hdr, iterator->tv_wall.usec);
            printf("Node %d: msg type = %d, msg length = %d\r\n", counter, *((U32 *) ((char *) msg_hdr + 4)), *((U32 *) msg_hdr));
            printf("Node %d: num_msgs = %d\r\n", counter, iterator->num_msgs);
        }

        counter++;
        iterator = iterator->next;
    }

    printf("******************************************************\r\n");
#endif /* DEBUG_QUEUE */
}
