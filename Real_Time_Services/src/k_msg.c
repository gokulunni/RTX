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
extern TCB *gp_current_task;
extern TCB *ready_queue_head;
extern TCB g_tcbs[MAX_TASKS];
extern TCB kernal_task;
extern INT_LL_NODE_T *free_tid_head;
#ifdef DEBUG_MSG
#include "printf.h"
#endif /* ! DEBUG_MSG */

int k_mbx_create(size_t size) {
#ifdef DEBUG_MSG
    printf("k_mbx_create: size = %d\r\n", size);
    printf("creating mailbox for tid: %d\r\n",gp_current_task->tid );
#endif /* DEBUG_MSG */

    if (size<=0||size<MIN_MBX_SIZE){
				#ifdef DEBUG_MSG
            printf("size is smaller than MIN_MBX_SIZE: size = %d\r\n", size);
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }

    //Assume it is not the NULL task running
    if (gp_current_task->has_mailbox ){ //MailBox already Exists
				#ifdef DEBUG_MSG
            printf("mailbox is found when initializing mailbox");
        #endif /* DEBUG_MSG */
        return RTX_ERR;    
    }

    //Need to check if size is larger than available memory at run time
    //Maybe this check be put inside circular queue during init?
    //try to run k_mem_alloc (allocation will run here)
    void* mailbox_buffer=k_mem_alloc(size);

    if (!mailbox_buffer){
        #ifdef DEBUG_MSG
            printf("Insufficent size during mem alloc: size = %d\r\n", size);
        #endif /* DEBUG_MSG */
        return RTX_ERR;
    }
    
    //ENSURE THAT MAILBOX IS SET TO NULL DURING TASK CREATE
    //Can you even set this to null? since it's not a pointer
    //OTHERWISE THERE MIGHT BE GARBAGE DATA
    
    //Call circular buffer init and pass in buffer and size
    circular_buffer_init(&gp_current_task->mailbox, mailbox_buffer, size);
		
		gp_current_task->has_mailbox=TRUE;

    return RTX_OK;
}

int k_send_msg(task_t receiver_tid, const void *buf) {
#ifdef DEBUG_MSG
    printf("k_send_msg: receiver_tid = %d, buf=0x%x\r\n", receiver_tid, buf);
    printf("sending from tid: %d\r\n",gp_current_task->tid );
    printf("sending to mailbox from for tid: %d\r\n",receiver_tid );
#endif /* DEBUG_MSG */

    //Trap kernel
    //No interrupts
    //__disable_irq();

    if (!buf){
        #ifdef DEBUG_MSG
            printf("k_send_msg: buf is NULL\r\n");
        #endif /* DEBUG_MSG */
        //__enable_irq();
        return RTX_ERR;
    }

    //Check all tid's in task through g_tcbs, ensure one exists
    //Recycle code from lab 2 
		int recieved_tid_int = (int) receiver_tid;
    
		
		//Check if task is avaliable use is avalible
		if (tid_is_available(free_tid_head, receiver_tid) == 1) {
        #ifdef DEBUG_MSG
        printf("[ERROR] k_tsk_get: task ID does not exist or dormant\n\r");
        #endif /* DEBUG_MSG */
			//__enable_irq();
        return RTX_ERR;
    }
		TCB *task = &g_tcbs[recieved_tid_int];

    //DOUBLE CHECK WHAT STATE IT SHOULD BE IN (RECIVEING TASK)
    if(task->state == DORMANT){
        //Check that the status of the sending task is not dormant
        #ifdef DEBUG_MSG
            printf("k_send_msg: reciever_tid is not running\r\n");
        #endif /* DEBUG_MSG */
        //__enable_irq();
        return RTX_ERR;
    }
     
     //Change to variable
    if(!task->has_mailbox){
				 #ifdef DEBUG_MSG
            printf("k_send_msg: reciever_tid has no mailbox running\r\n");
        #endif /* DEBUG_MSG */
        //No mailbox for task
        //__enable_irq();
        return RTX_ERR;
    }
		
    U32 length = *((U32 *) buf);
		U32 type = *((U32 *) ((char *)buf + 4));
    //Try this U32 length = *((U32 *) msg); struct rtx_msg_hdr *ptr = (void *)buf;
    //Check example of casting 

    if (length < MIN_MSG_SIZE){
				 #ifdef DEBUG_MSG
            printf("k_send_msg: Length of mailbox is less tha MIN_MSG_SIZE\r\n");
        #endif /* DEBUG_MSG */
        //__enable_irq();
        return RTX_ERR;
    }

		//Commenting out for testing purposes
    if(is_circ_buf_full(&task->mailbox,length)){
			 #ifdef DEBUG_MSG
            printf("k_send_msg: circular buffer is full for recieving tid\r\n");
        #endif /* DEBUG_MSG */
        //__enable_irq();
        return RTX_ERR;
    }

    //check that this doesn't conflict with pre emption
    if(task->state ==BLK_MSG){
				#ifdef DEBUG_MSG
					printf("k_send_msg: unblocking mailbox after sending message to blocked TID");
				#endif /* DEBUG_MSG */
        task->state = READY;
        //Add state back to ready_queue
        push(&ready_queue_head, task);
    }
		
		if (task->prio = PRIO_RT)
		{
			//U32 length and type created for msg
			//how do i check mailbox's predefined length and type
			//peek_msg_type and peek_msg_length are predefined?
			//how do we predefine real-time task's mailboxes
		}

    if (enqueue_msg( &(task->mailbox), (void*)buf)==RTX_ERR){
        //Does enqueue_msg read the header from the buffer?
        //Assuming i can just do deep copy and have logic
        //Inside of enqueue calculate length
			 #ifdef DEBUG_MSG
            printf("k_send_msg: could not enqueue message\r\n");
        #endif /* DEBUG_MSG */
        //__enable_irq();
        return RTX_ERR;
    }
    TCB *sendingTask= gp_current_task;
    gp_current_task =&kernal_task;

    INT_LL_NODE_T* tid_node = k_mem_alloc(sizeof(INT_LL_NODE_T));

    tid_node->tid =sendingTask->tid;
    //Pass TID onto the linked list of the task
    push_tid((INT_LL_NODE_T **)(&(task->msg_sender_head)), tid_node);
    
    gp_current_task = sendingTask;
    //Turn back on interrupts
    //__enable_irq();
    //Switch properly at the end (call yeild?)
		
    //Commenting out for testing purposes.
		k_tsk_yield();
  
    return RTX_OK;
}

int k_recv_msg(task_t *sender_tid, void *buf, size_t len) {
	#ifdef DEBUG_MSG
        printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
  #endif /* DEBUG_MSG */
	
    if ( !(len > 0)) {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: invalid len entered");
			#endif /* DEBUG_MSG */
        return RTX_ERR;
    }
		
		if (buf == NULL) {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: can not pass in unallocated pointer for buf");
			#endif /* DEBUG_MSG */
			return RTX_ERR;
		}

    //trap into kernel- atomicity on / disable interrupts

   // __disable_irq();
    //TCB* curr_task = gp_current_task;
		void* ptr=buf;
		task_t *save_sender = sender_tid;

    if (!gp_current_task->has_mailbox)
    {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: current task does NOT have a mailbox");
			#endif /* DEBUG_MSG */
			//__enable_irq();
			return RTX_ERR;
    }
    
    while(is_circ_buf_empty( &(gp_current_task->mailbox)))
    {
				
        gp_current_task->state = BLK_MSG;
				#ifdef DEBUG_MSG
					printf("k_recv_msg: blocking task due to empty mailbox");
				#endif /* DEBUG_MSG */
				//__enable_irq();
        k_tsk_yield();
				//__disable_irq();
    }
		

    if (dequeue_msg( &(gp_current_task->mailbox), buf, len) == RTX_ERR)
    {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: error dequeueing message from mailbox");
			#endif /* DEBUG_MSG */
			//__enable_irq();
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
			printf("k_recv_msg: could not deallocate pointer to sender");
			#endif /* DEBUG_MSG */
		}
		
    //atomicity off / enable interrupts
		gp_current_task = prev_current_task;

    //__enable_irq();
		
		#ifdef DEBUG_MSG
        printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
    #endif /* DEBUG_MSG */


    return 0;
}

int k_recv_msg_nb(task_t *sender_tid, void *buf, size_t len) {
	#ifdef DEBUG_MSG
        printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
  #endif /* DEBUG_MSG */
	
    if ( !(len > 0)) {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: invalid len entered");
			#endif /* DEBUG_MSG */
        return RTX_ERR;
    }
		
		if (buf == NULL) {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: can not pass in unallocated pointer for buf");
			#endif /* DEBUG_MSG */
			return RTX_ERR;
		}

    if (!gp_current_task->has_mailbox)
    {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: current task does NOT have a mailbox");
			#endif /* DEBUG_MSG */
			//__enable_irq();
			return RTX_ERR;
    }
    
    if (is_circ_buf_empty( &(gp_current_task->mailbox))) {
				return RTX_ERR;
    }
		

    if (dequeue_msg( &(gp_current_task->mailbox), buf, len) == RTX_ERR)
    {
			#ifdef DEBUG_MSG
        printf("k_recv_msg: error dequeueing message from mailbox");
			#endif /* DEBUG_MSG */
			//__enable_irq();
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
			printf("k_recv_msg: could not deallocate pointer to sender");
			#endif /* DEBUG_MSG */
		}
		
    //atomicity off / enable interrupts
		gp_current_task = prev_current_task;
		
		#ifdef DEBUG_MSG
        printf("k_recv_msg: sender_tid  = 0x%x, buf=0x%x, len=%d\r\n", sender_tid, buf, len);
    #endif /* DEBUG_MSG */
    return 0;
}

int k_mbx_ls(task_t *buf, int count) {
	#ifdef DEBUG_MSG
    printf("k_mbx_ls: buf=0x%x, count=%d\r\n", buf, count);
	#endif /* DEBUG_MSG */
	
	int actual_count = 0;
	int buf_index = 0;
	
	if (buf == NULL) {
		#ifdef DEBUG_MSG
    printf("k_mbx_ls: can not pass in NULL task elements");
		#endif /* DEBUG_MSG */
		return RTX_ERR;
	}
	if (count < 0) {
		#ifdef DEBUG_MSG
    printf("k_mbx_ls: invalid count passed in");
		#endif /* DEBUG_MSG */
		return RTX_ERR;
	}
	
	for (int i = MAX_TASKS-1; i >= 0; i--)
	{
		if (actual_count == count)
			break;
		
		if (g_tcbs[i].state != DORMANT && g_tcbs[i].has_mailbox) {
			actual_count++;
			buf[buf_index++] = g_tcbs[i].tid;
		}
	}
	return actual_count;
}
