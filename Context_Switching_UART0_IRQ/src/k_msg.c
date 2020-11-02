/**
 * @file:   k_msg.c
 * @brief:  kernel message passing routines
 * @author: Yiqing Huang
 * @date:   2020/10/09
 */

#include "k_msg.h"
#include "k_task.h"
#include "k_mem.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

int k_mbx_create(size_t size) {
#ifdef DEBUG_0
    printf("k_mbx_create: size = %d\r\n", size);
#endif /* DEBUG_0 */
    return 0;
}

int k_send_msg(task_t receiver_tid, const void *buf) {
#ifdef DEBUG_0
    printf("k_send_msg: receiver_tid = %d, buf=0x%x\r\n", receiver_tid, buf);
#endif /* DEBUG_0 */
    return 0;
}

int k_recv_msg(task_t *sender_tid, void *buf, size_t len) {
    #ifdef DEBUG_0
        printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
    #endif /* DEBUG_0 */
    if ( !(len > 0)) {
        return RTX_ERR;
    }

    //trap into kernel- atomicity on / disable interrupts
    __disable_irq();
    TCB* curr_task = gp_current_task;

    if (!curr_task->mailbox)
    {
        __enable_irq();
        return RTX_ERR;
    }
    
    while(is_empty(curr_task->mailbox))
    {
        curr_task->state = BLK_MSG;
        k_tsk_yield();
    }

    buf = k_mem_alloc(len);
    void* ptr;
    //unsure how dequeueing will be implemented for cicurlar queue
    dequeue_msg(curr_task->mailbox, ptr);

    if (ptr == NULL)
    {
        __enable_irq();
        return RTX_ERR;
    }
    //check if len < size of the message - length field should be first 4 bytes of the message (in the header)
    int actual_len;
    memcpy(&actual_len, ptr, 4);
    if (len < actual_len)
    {
        __enable_irq();
        return RTX_ERR;
    }

    memcpy(buf, ptr, len);
    //*sender_tid = ; //??
    //^---SENDER TID NEEDS TO BE SAVED SOMEWHERE IN THE MESSAGE, can't change message header, put it right after header??

    //atomicity off / enable interrupts
    __enable_irq();

    return 0;
}

int k_mbx_ls(task_t *buf, int count) {
#ifdef DEBUG_0
    printf("k_mbx_ls: buf=0x%x, count=%d\r\n", buf, count);
#endif /* DEBUG_0 */
    return 0;
}
