/**
 * @file:   k_msg.c
 * @brief:  kernel message passing routines
 * @author: Yiqing Huang
 * @date:   2020/10/09
 */

#include "k_msg.h"
#include "common.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

int k_mbx_create(size_t size) {
#ifdef DEBUG_0
    printf("k_mbx_create: size = %d\r\n", size);
#endif /* DEBUG_0 */

    if (size<=0||size<MIN_MBX_SIZE){
        return RTX_ERR;
    }

    //Assume it is not the NULL task running
    if (!gp_current_task->mailbox ){ //MailBox already Exists
        return RTX_ERR;    
    }

    //Need to check if size is larger than available memory at run time
    //Maybe this check be put inside circular queue during init?
    //try to run k_mem_alloc (allocation will run here)
    void* mailbox_buffer=k_mem_alloc(size);

    if (!mailbox_buffer){
        #ifdef DEBUG_0
            printf("Insufficent size during mem alloc: size = %d\r\n", size);
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }
    
    //ENSURE THAT MAILBOX IS SET TO NULL DURING TASK CREATE
    //OTHERWISE THERE MIGHT BE GARBAGE DATA

    //Call circular buffer init and pass in buffer and size
    gp_current_task->mailbox=circular_buffer_init(size,mailbox_buffer);


    return RTX_OK;
}

int k_send_msg(task_t receiver_tid, const void *buf) {
#ifdef DEBUG_0
    printf("k_send_msg: receiver_tid = %d, buf=0x%x\r\n", receiver_tid, buf);
#endif /* DEBUG_0 */

    //Trap kernel
    //No interrupts
    __disable_irq()

    if (!buf){
        return RTX_ERR;
    }

    //Check all tid's in task through g_tcbs, ensure one exists
    //Recycle code from lab 2 
    if (!tid_exists(receiver_tid)){
        __enable_irq();
        return RTX_ERR;
    }

    TCB task =get_task_by_tid(reciever_tid);
     
    if(!task->mailbox){
        //No mailbox for task
        __enable_irq();
        return RTX_ERR;
    }

    RTX_MSG_HDR header = (header) buf;

    if (header->length < MIN_MSG_SIZE){
        __enable_irq();
        return RTX_ERR;
    }

    if(is_full(task->mailbox),header->length)){
        __enable_irq();
        return RTX_ERR;
    }

    //check that this doesn't conflict with pre emption
    if(task->state ==BLK_MSG){
        task->state== READY;
        //Add state back to ready_queue

        if (!enqueue_msg(task->mailbox,buf)){
            //Does enqueue_msg read the header from the buffer?
            //Assuming i can just do deep copy and have logic
            //Inside of enqueue calculate length
            __enable_irq();
            return RTX_ERR;
        }
    }

    //Turn back on interrupts
    __enable_irq();
    //Switch properly at the end (call yeild?)
    k_tsk_yield();
  


    return RTX_OK;
}

int k_recv_msg(task_t *sender_tid, void *buf, size_t len) {
#ifdef DEBUG_0
    printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
#endif /* DEBUG_0 */
    return 0;
}

int k_mbx_ls(task_t *buf, int count) {
#ifdef DEBUG_0
    printf("k_mbx_ls: buf=0x%x, count=%d\r\n", buf, count);
#endif /* DEBUG_0 */
    return 0;
}
