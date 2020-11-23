/**
 * @file:   k_msg.c
 * @brief:  kernel message passing routines
 * @author: Yiqing Huang
 * @date:   2020/10/09
 */

#include "k_msg.h"
#include "k_task.h"
#include "common.h"
#include "k_mem.h"
#include "linked_list.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

extern TCB *gp_current_task;
extern TCB *ready_queue_head;
extern TCB g_tcbs[MAX_TASKS];
extern TCB kernal_task;
extern INT_LL_NODE_T *free_tid_head;

int k_mbx_create(size_t size) {
    #ifdef DEBUG_0
    printf("k_mbx_create: size = %d\r\n", size);
    #endif /* DEBUG_0 */

    if (size<=0||size<MIN_MBX_SIZE) {
        #ifdef DEBUG_MSG
        printf("k_mbx_create: size<=0||size<MIN_MBX_SIZE\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (gp_current_task->has_mailbox){ //MailBox already Exists
        #ifdef DEBUG_MSG
        printf("k_mbx_create: gp_current_task->has_mailbox");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    void *mailbox_buffer = k_mem_alloc(size);

    if (mailbox_buffer == NULL){
        #ifdef DEBUG_MSG
        printf("k_mbx_create: mailbox malloc failed\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    circular_buffer_init(&gp_current_task->mailbox, mailbox_buffer, size);
    gp_current_task->has_mailbox = TRUE;

    return RTX_OK;
}

int k_send_msg(task_t receiver_tid, const void *buf) {
    #ifdef DEBUG_0
    printf("k_send_msg: receiver_tid = %d, buf=0x%x\r\n", receiver_tid, buf);
    #endif /* DEBUG_0 */

    if (buf == NULL){
        #ifdef DEBUG_MSG
        printf("k_send_msg: buf is NULL\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (tid_is_available(free_tid_head, receiver_tid) == 1) {
        #ifdef DEBUG_MSG
        printf("[ERROR] k_tsk_get: task ID does not exist or dormant\n\r");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    TCB *task = &g_tcbs[(int) receiver_tid];

    if(task->state == DORMANT){
        #ifdef DEBUG_MSG
        printf("k_send_msg: task is dormant\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if(!task->has_mailbox){
        #ifdef DEBUG_MSG
        printf("k_send_msg: task has no mailbox\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    U32 length = *((U32 *) buf);
    U32 type = *((U32 *) ((char *) buf + 4));

    if (length < MIN_MSG_SIZE){
        #ifdef DEBUG_MSG
        printf("k_send_msg: Length of mailbox is less tha MIN_MSG_SIZE\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if(is_circ_buf_full(&task->mailbox,length)){
        #ifdef DEBUG_MSG
        printf("k_send_msg: circular buffer is full\r\n");
        #endif /* DEBUG_MSG */
        //__enable_irq();
        return RTX_ERR;
    }

    if (task->prio == PRIO_RT) {
        if (length != task->msg_hdr->length || type != task->msg_hdr->type) {
            #ifdef DEBUG_MSG
            printf("k_send_msg: wrong msg header for RT receiving task");
            #endif /* DEBUG_MSG */
            return RTX_ERR;
        }
    }

    if (enqueue_msg(&(task->mailbox), (void *) buf) == RTX_ERR){
        #ifdef DEBUG_MSG
        printf("k_send_msg: enqueue_msg failed\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    TCB *temp = gp_current_task;
    gp_current_task = &kernal_task;

    INT_LL_NODE_T *tid_node = k_mem_alloc(sizeof(INT_LL_NODE_T));
    tid_node->tid = sendingTask->tid;
    push_tid((INT_LL_NODE_T **) (&(task->msg_sender_head)), tid_node);

    gp_current_task = temp;

    if(task->state == BLK_MSG){
        #ifdef DEBUG_MSG
        printf("k_send_msg: unblocking mailbox after sending message to blocked TID");
        #endif /* DEBUG_MSG */
        task->state = READY;
        push(&ready_queue_head, task);

        if (task->prio > gp_current_task->prio){
            k_tsk_yield();
        }
    }

    return 0;
}

int k_recv_msg(task_t *sender_tid, void *buf, size_t len) {
    #ifdef DEBUG_0
    printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
    #endif /* DEBUG_0 */

    if (!(len > 0)) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: len <= 0\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (buf == NULL) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: buf is NULL\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (!gp_current_task->has_mailbox) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: current task does NOT have a mailbox\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    while (is_circ_buf_empty(&(gp_current_task->mailbox))) {
        gp_current_task->state = BLK_MSG;
        k_tsk_yield();
    }

    if (dequeue_msg(&(gp_current_task->mailbox), buf, len) == RTX_ERR) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: dequeue_msg failed");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    TCB *prev_current_task = gp_current_task;
    gp_current_task = &kernal_task;

    INT_LL_NODE_T* sender = pop_tid((INT_LL_NODE_T**) &(prev_current_task->msg_sender_head));
    if (sender_tid != NULL) {
        *sender_tid = sender->tid;
    }
    if (k_mem_dealloc(sender) == RTX_ERR) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: could not deallocate pointer to sender\r\n");
        #endif /* DEBUG_MSG */
    }

    gp_current_task = prev_current_task;

    return 0;
}

int k_recv_msg_nb(task_t *sender_tid, void *buf, size_t len) {
    #ifdef DEBUG_0
    printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
    #endif /* DEBUG_0 */

    if (!(len > 0)) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: len <= 0\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (buf == NULL) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: buf is NULL\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (!gp_current_task->has_mailbox) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: current task does NOT have a mailbox\r\n");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (is_circ_buf_empty(&(gp_current_task->mailbox))) {
        return RTX_ERR;
    }

    if (dequeue_msg(&(gp_current_task->mailbox), buf, len) == RTX_ERR) {
        #ifdef DEBUG_MSG
        printf("k_recv_msg: dequeue_msg failed");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    TCB *prev_current_task = gp_current_task;
    gp_current_task = &kernal_task;

    INT_LL_NODE_T* sender = pop_tid((INT_LL_NODE_T**) &(prev_current_task->msg_sender_head));
    if (sender_tid != NULL) {
        *sender_tid = sender->tid;
    }
    if (k_mem_dealloc(sender) == RTX_ERR) {
#ifdef DEBUG_MSG
        printf("k_recv_msg: could not deallocate pointer to sender\r\n");
#endif /* DEBUG_MSG */
    }

    gp_current_task = prev_current_task;

    return 0;
}

int k_mbx_ls(task_t *buf, int count) {
    #ifdef DEBUG_0
    printf("k_mbx_ls: buf=0x%x, count=%d\r\n", buf, count);
    #endif /* DEBUG_0 */

    int actual_count = 0;
    int buf_index = 0;

    if (buf == NULL) {
        #ifdef DEBUG_MSG
        printf("k_mbx_ls: buf is NULL");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    if (count < 0) {
        #ifdef DEBUG_MSG
        printf("k_mbx_ls: count < 0");
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    for (int i = MAX_TASKS-1; i >= 0; i--) {
        if (actual_count == count)
            break;

        if (g_tcbs[i].state != DORMANT && g_tcbs[i].has_mailbox) {
            actual_count++;
            buf[buf_index++] = g_tcbs[i].tid;
        }
    }
    return actual_count;
}
