/**
 * @file:   k_task.c  
 * @brief:  task management C file
 * @author: Yiqing Huang
 * @date:   2020/09/24
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user task and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       You need to understand the assumptions and the limitations of the code. 
 */

#include <LPC17xx.h>
#include "uart_polling.h"
#include "k_task.h"
#include "linked_list.h"
#include "k_mem.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
TCB *gp_current_task;    /* always point to the current RUN process */

/* TCBs and Kernel stacks are statically allocated and is inside the OS image */
TCB g_tcbs[MAX_TASKS];
U32 g_k_stacks[MAX_TASKS][KERN_STACK_SIZE >> 2];

// Kernal Fake Task used for mem alloc of kernel necessary data
TCB kernal_task;
TCB *k_null_tsk = NULL;

TCB *ready_queue_head = NULL;
INT_LL_NODE_T *free_tid_head = NULL;
volatile U32 g_switch_flag = 0;  /* whether to continue to run the process before the UART receive interrupt */
void *alloc_user_stack(size_t size);
int dealloc_user_stack(U32 *ptr, size_t size);

/*---------------------------------------------------------------------------
The memory map of the OS image may look like the following:

                    0x10008000+---------------------------+ High Address
                              |                           |
                              |                           |
                              |    Free memory space      |
                              | (Alloc user stacks here)  |
                              |                           |
                              |                           |
                              |                           |
&Image$$RW_IRAM1$$ZI$$Limit-->|---------------------------|-----+-----
                              |         ......            |     ^
                              |---------------------------|     |
                              |                           |     | 
                              |---------------------------|     |
                              |      KERN_STACK_SIZE      |     |                
             g_k_stacks[15]-->|---------------------------|     |
                              |                           |     |
                              |     other kernel stacks   |     |                              
                              |---------------------------|     |
                              |      KERN_STACK_SIZE      |  OS Image
              g_k_stacks[2]-->|---------------------------|     |
                              |      KERN_STACK_SIZE      |     |                      
              g_k_stacks[1]-->|---------------------------|     |
                              |      KERN_STACK_SIZE      |     |
              g_k_stacks[0]-->|---------------------------|     |
                              |   other  global vars      |     |
                              |---------------------------|     |
                              |        TCBs               |     |
                      g_tcbs->|---------------------------|     |
                              |        global vars        |     |
                              |---------------------------|     |
                              |                           |     |          
                              |                           |     |
                              |                           |     |
                              |                           |     V
                    0x10000000+---------------------------+ Low Address
    
---------------------------------------------------------------------------*/

void *alloc_user_stack(size_t size) {
    U32 *stack_low = k_mem_alloc(size);
    if (stack_low == NULL) {
        return NULL;
    }

    return (char *) stack_low + size;
}

int dealloc_user_stack(U32 *ptr, size_t size) {
    if (ptr == NULL) {
        return RTX_ERR;
    }

    return k_mem_dealloc((char *) ptr - size);
}

void null_task_func() {
    while (1) {}
}

/**
 * @biref: initialize all tasks in the system
 * @param: RTX_TASK_INFO *task_info, an array of initial tasks
 * @param: num_tasks, number of elements in the task_info array
 * @return: none
 * PRE: memory has been properly initialized
 */
int k_tsk_init(RTX_TASK_INFO *task_info, int num_tasks) {
	gp_current_task = NULL;
	/* Default is MSP when calling tsk_init(), set to PSP */
	  //__set_PSP((U32) __get_MSP());
		//__set_CONTROL((U32)3);

    if (num_tasks <= 0 || num_tasks > MAX_TASKS) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_init: invalid num_tasks\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    if (task_info == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_init: no initial kernel tasks to run\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    if (k_null_tsk != NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_init: init has been run before\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }
	  
    int i;
    U32 *sp;
    RTX_TASK_INFO *p_taskinfo = task_info;
  
    // Create fake kernal task with ID = MAX_TASKS+1
    kernal_task.tid = MAX_TASKS + 1;
    gp_current_task = &kernal_task;

    /* Pretend an exception happened, by adding exception stack frame */
    /* initilize exception stack frame (i.e. initial context) for each task */
    // TODO: if PRIO is NULL, skip that task
    for (i = 0; i < num_tasks; i++) { // TODO: check that num task less than max
        int j;
        TCB *p_tcb;
							
        if(p_taskinfo -> ptask == &kcd_task)
        {
            p_tcb = &g_tcbs[TID_KCD];
            p_tcb->tid = TID_KCD;
			i--;
        }
        else if(p_taskinfo -> ptask == &lcd_task)
        {
            p_tcb = &g_tcbs[TID_DISPLAY];
            p_tcb-> tid = TID_DISPLAY;
	        i--;
        }
        else if((i + 1) == TID_KCD || (i + 1) == TID_DISPLAY) 
        {
            continue;
        }
        else
        {
            p_tcb = &g_tcbs[i+1];
            p_tcb->tid = i+1;
        }
        
        p_tcb->state = NEW;
        p_tcb->has_mailbox=NULL;
        //Initialize mailbox to NULL values
        p_tcb->mailbox.buffer_start = NULL;
        p_tcb->mailbox.buffer_end = NULL;
        p_tcb->mailbox.head = NULL;
        p_tcb->mailbox.tail = NULL;
			
        //CHECK CREATE FUNCTION

        p_tcb->prio = p_taskinfo->prio;
        // TODO: can we skip NULL_PRIO task? can we ignore user provided NULL TASK and use our own
        if (p_tcb->prio == PRIO_NULL) {
            continue;
        }

        if (p_taskinfo->priv == 0) { /* unprivileged task */
            p_tcb->priv = 0;
            p_tcb->psp_hi = alloc_user_stack(p_taskinfo->u_stack_size);

            if (p_tcb->psp_hi == NULL) {
                #ifdef DEBUG_0
                printf("[ERROR] k_tsk_init: failed to allocate init's task user stack\n\r");
                #endif /* DEBUG_0 */
                return RTX_ERR;
            }

            sp = p_tcb->psp_hi;
            *(--sp) = INITIAL_xPSR;
            *(--sp) = (U32)(p_taskinfo->ptask);
            for (int j = 0; j < 6; j++) {
                *(--sp) = 0x0;
            }

            p_tcb->psp = sp;
            p_tcb->psp_size = p_taskinfo->u_stack_size;
            p_tcb->msp_hi = g_k_stacks[i+1] + (KERN_STACK_SIZE >> 2);
            p_tcb->msp = p_tcb->msp_hi;
        } else { /* privileged task */
            p_tcb->priv = 1;

            p_tcb->msp_hi = g_k_stacks[i+1] + (KERN_STACK_SIZE >> 2);
            sp = p_tcb->msp_hi; /* stacks grows down, so get the high addr. */
            *(--sp)  = INITIAL_xPSR;    									/* task initial xPSR (program status register) */
            *(--sp)  = (U32)(p_taskinfo->ptask); 					/* PC contains the entry point of the task */
            for ( j = 0; j < 6; j++ ) { 									/*R0-R3, R12, LR */
                *(--sp) = 0x0;
            }

            p_tcb->msp = sp;
            p_tcb->psp = p_tcb->msp;
            p_tcb->psp_hi = NULL;
            p_tcb->psp_size = 0;
        }

        //Add task to the priority queue for NEW tasks
        push(&ready_queue_head, p_tcb);

        p_taskinfo++;
    }

    for (int q = i + 1; q < MAX_TASKS; q++) {
			
        if(q == TID_KCD || q == TID_DISPLAY) {
            continue;
        }
				
        INT_LL_NODE_T *new_tid = k_mem_alloc(sizeof(INT_LL_NODE_T));
        if (new_tid == NULL) {
            #ifdef DEBUG_0
            printf("[ERROR] k_tsk_init: tid failed to allocate memory\n\r");
            #endif /* DEBUG_0 */
            return RTX_ERR;
        }
				
		new_tid->tid = q;
        push_tid(&free_tid_head, new_tid);
	}

    print_free_tids(free_tid_head);
    print_prio_queue(ready_queue_head);


    if (k_null_tsk == NULL) {
        k_null_tsk = &g_tcbs[0];
        k_null_tsk->tid = PID_NULL;
        k_null_tsk->state = NEW;

        k_null_tsk->psp_hi = alloc_user_stack(0x44);
        if (k_null_tsk->psp_hi == NULL) {
            #ifdef DEBUG_0
            printf("[ERROR] k_tsk_init: failed to allocate memory for null task's user stack\n\r");
            #endif /* DEBUG_0 */
            return RTX_ERR;
        }
        k_null_tsk->psp_size = 0x44;

        sp = k_null_tsk->psp_hi; /* stacks grows down, so get the high addr. */
        *(--sp) = INITIAL_xPSR;
        *(--sp) = (U32)(&null_task_func);
        for (int g = 0; g < 6; g++) { /*R0-R3, R12, LR */
            *(--sp) = 0x0;
        }
        k_null_tsk->psp = sp;

        k_null_tsk->msp_hi = g_k_stacks[0] + (KERN_STACK_SIZE >> 2);
        k_null_tsk->msp = k_null_tsk->msp_hi;

        k_null_tsk->prio = PRIO_NULL;
        k_null_tsk->priv = 0;
    }

    gp_current_task = k_null_tsk;

    return RTX_OK;
}

/*@brief: scheduler, pick the tid of the next to run task
 *@return: TCB pointer of the next to run task
 *         NULL if error happens
 *POST: if gp_current_task was NULL, then it gets set to tcb[1].
 *      No other effect on other global variables.
 */

TCB *dummy_scheduler(void) {
	//This should never be false except if the NULL_PRIO task is running
    if(!is_empty(ready_queue_head)) {
        TCB *popped = pop(&ready_queue_head);

        // If there is a current task, push current task back on ready queue
        if (gp_current_task && gp_current_task -> state != BLK_MSG) {
            push(&ready_queue_head, gp_current_task);
        }

        gp_current_task = popped;
    }

    if (gp_current_task == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] dummy_scheduler: gp_current_task is NULL\n");
        #endif /* DEBUG_0 */
        pop_task_by_id(&ready_queue_head, 0);
        gp_current_task = &g_tcbs[0];
    }

    return gp_current_task;
}

/*@brief: switch out old tcb (p_tcb_old), run the new tcb (gp_current_task)
 *@param: p_tcb_old, the old tcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_tcb_old and gp_current_task are pointing to valid TCBs.
 */
int task_switch(TCB *p_tcb_old) { // TODO: confirm both p_tcb_old and gp_current_task are valid before invoking
    U8 state;
    
    state = gp_current_task->state;

    if (state == NEW) {
        if (gp_current_task != p_tcb_old && p_tcb_old->state != NEW) {
            p_tcb_old->state = READY;
            p_tcb_old->msp = (U32 *) __get_MSP();
            p_tcb_old->psp = (U32 *) __get_PSP();
        }
        gp_current_task->state = RUNNING;
        __set_MSP((U32) gp_current_task->msp);
        __set_PSP((U32)gp_current_task->psp);
				
        __rte();  /* pop exception stack frame from the stack for a new task */
    } 
    
    /* The following will only execute if the if block above is FALSE */

    if (gp_current_task != p_tcb_old) {
        if (state == READY) {
            p_tcb_old->state = READY; 
            p_tcb_old->msp = (U32 *) __get_MSP(); // save the old process's sp
            p_tcb_old->psp = (U32 *) __get_PSP();
            gp_current_task->state = RUNNING;
            __set_MSP((U32) gp_current_task->msp); //switch to the new proc's stack 
            __set_PSP((U32)gp_current_task->psp);
        } else {
            gp_current_task = p_tcb_old; // revert back to the old proc on error
            return RTX_ERR;
        } 
    }
    return RTX_OK;
}


/**
 * @brief yield the processor. The caller becomes READY and the scheduler picks the next ready to run task.
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_task gets updated to next to run process
 */
int k_tsk_yield(void) {
    //Keep track of task that was running and name it p_tcb_old
    //Get the old task
    TCB *p_tcb_old = gp_current_task;

    // a prioritity with a smaller value equals a higher priority
    if (ready_queue_head != NULL && p_tcb_old != NULL && ready_queue_head->prio <= p_tcb_old->prio) {

        //Pop the next task in queue
        gp_current_task = dummy_scheduler();

        #ifdef DEBUG_0
        printf("k_tsk_yield: Yielding task with ID: %d \n",p_tcb_old->tid);
        #endif /* DEBUG_0 */

        if (gp_current_task == NULL){
            #ifdef DEBUG_0
            printf("[ERROR] k_tsk_yield: No next task available");
            #endif /* DEBUG_0 */
            gp_current_task=p_tcb_old;
            return RTX_ERR;
        }
        if (p_tcb_old == NULL){
            #ifdef DEBUG_0
            printf("[WARNING] k_tsk_yield: gp_current_task was NULL while Yield() was called");
            #endif /* DEBUG_0 */
            p_tcb_old = gp_current_task;
        }
        print_prio_queue(ready_queue_head);
        if (task_switch(p_tcb_old) == RTX_ERR) {
            #ifdef DEBUG_0
            printf("[WARNING] k_tsk_yield: could not switch task, same task resuming");
            #endif
        }

    } else {
        #ifdef DEBUG_0
        printf("k_tsk_yield: gp_current_task priority was higher than head TCB in ready_queue, no task switching occured");
        #endif /* DEBUG_0 */
    }

    return RTX_OK;
}


int k_tsk_create(task_t *task, void (*task_entry)(void), U8 prio, U16 stack_size) {
    #ifdef DEBUG_0
    printf("k_tsk_create: entering...\n\r");
    printf("task = 0x%x, task_entry = 0x%x, prio=%d, stack_size = %d\n\r", task, task_entry, prio, stack_size);
    #endif /* DEBUG_0 */

    if (prio == PRIO_NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: attempted to create NULL task\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    } else if (!(prio >= 0 && prio <= 4)) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: prio outside of task priority bounds\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }
		
    if (stack_size <= 0) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: invalid stack_size entered\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    if (free_tid_head == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: no available TID\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    TCB *prev_current_task = gp_current_task;
    gp_current_task = &kernal_task;

    INT_LL_NODE_T *popped_tid = pop_tid(&free_tid_head);
    if (popped_tid == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: no available TID\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    int tid = popped_tid->tid;
    if (k_mem_dealloc(popped_tid) == RTX_ERR) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: error in deallocating tid\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    TCB *new_task = &g_tcbs[tid];

    new_task->tid = tid;
    new_task->state = NEW;
    new_task->next = NULL;
    new_task->prio = prio;
    new_task->priv = 0;
    new_task->has_mailbox=NULL;
    //Initialize mailbox to NULL values
		new_task->mailbox.buffer_start = NULL;
		new_task->mailbox.buffer_end = NULL;
		new_task->mailbox.head = NULL;
		new_task->mailbox.tail = NULL;

    new_task->psp_size = stack_size;
    new_task->psp_hi = alloc_user_stack(stack_size);
    if (new_task->psp_hi == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: could not allocate stack for new task\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    gp_current_task = prev_current_task;

    U32* sp = new_task->psp_hi;
    *(--sp) = INITIAL_xPSR;
    *(--sp) = (U32)(task_entry);
    for (int j = 0; j < 6; j++) {
        *(--sp) = 0x0;
    }
    new_task->psp = sp;

    new_task->msp_hi = g_k_stacks[tid] + (KERN_STACK_SIZE >> 2);
    new_task->msp = new_task->msp_hi;

    push(&ready_queue_head, new_task);
    print_prio_queue(ready_queue_head);

    if(gp_current_task->prio > new_task->prio)  {
        //must run immediately
        k_tsk_yield();
    }
    *task = new_task->tid;

    return RTX_OK;
}


void k_tsk_exit(void) {
    #ifdef DEBUG_0
    printf("k_tsk_exit: exiting...\n\r");
    #endif /* DEBUG_0 */
    // A PRIO_NULL task cannot exit
    if (gp_current_task->prio != PRIO_NULL) {
        gp_current_task->state = DORMANT;

        TCB *prev_current_task = gp_current_task;
        gp_current_task = &kernal_task;

        // If its unpriviledged task, dealloc user stack
        if (prev_current_task->priv == 0) {
            if (dealloc_user_stack(prev_current_task->psp_hi, prev_current_task->psp_size) == RTX_ERR) {
                #ifdef DEBUG_0
                printf("[ERROR] k_tsk_exit: failed to deallocate user stack for task %d\n\r", prev_current_task->tid);
                #endif /* DEBUG_0 */
            }
            prev_current_task->psp = NULL;
        }

        INT_LL_NODE_T *new_tid = k_mem_alloc(sizeof(INT_LL_NODE_T));
        if (new_tid == NULL) {
            #ifdef DEBUG_0
            printf("[ERROR] k_tsk_exit: could not allocate memory for new_tid\n\r");
            #endif /* DEBUG_0 */
        }
        new_tid->tid = prev_current_task->tid;
        push_tid(&free_tid_head, new_tid);

        pop_task_by_id(&ready_queue_head, 0);
        gp_current_task = &g_tcbs[0];

        k_tsk_yield();
    }

    return;
}


int k_tsk_set_prio(task_t task_id, U8 prio) {
    #ifdef DEBUG_0
    printf("k_tsk_set_prio: entering...\n\r");
    printf("task_id = %d, prio = %d.\n\r", task_id, prio);
    #endif /* DEBUG_0 */

    if (prio == PRIO_NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: cannot set prio to PRIO_NULL\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    } else if (!(prio >= 0 && prio <= 4)) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: prio outside of task priority bounds\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    // The priority of the null task cannot be changed and remains at level PRIO_NULL.
    if (task_id == 0) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: cannot change prio of NULL task\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    if (task_id >= MAX_TASKS || task_id < 0) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_get: task ID outside of TID domain\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    //Ensure that we are only popping the task frmo the ready queue if it isn't already runing
    TCB *task;

    if (task_id != gp_current_task->tid){
        task = pop_task_by_id(&ready_queue_head, task_id);
    } else {
        task = gp_current_task;
    }

    if (task == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: task with tid %d not found\n\r", task_id);
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    if (task->state == DORMANT) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: task with tid %d is DORMANT\n\r", task_id);
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    // The priority of the null task cannot be changed and remains at level PRIO_NULL.
    if (task->prio == PRIO_NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: cannot change prio of NULL task\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    // An unprivileged task may change the priority of any other unprivileged task (including itself).
    // A privileged task may change the priority of any other task (including itself).
    if (((gp_current_task->priv == 0 && task->priv == 0) || gp_current_task->priv == 1)) {
        task->prio = prio;

        //    The caller of this primitive never blocks, but could be preempted.
        //    If the value of prio is higher than the priority of the current running task,
        //    and the task identified by task id is in ready state, then the task identified by
        //    the task id preempts the current running task. Otherwise, the current running task
        //    continues its execution.
        
        //changing priority for a task in ready Q
        if (task_id != gp_current_task->tid){
            //if priority for a task in ready Q is higher than running task
            if ((task->state == READY || task->state == NEW) && task->prio < gp_current_task->prio) {
                TCB *p_tcb_old = gp_current_task;
                push(&ready_queue_head, p_tcb_old);
                gp_current_task = task;
                if (task_switch(p_tcb_old) == RTX_ERR) {
                    #ifdef DEBUG_0
                    printf("[ERROR] k_tsk_set_prio: could not switch task, same task resuming");
                    #endif
                    return RTX_ERR;
                }
            }
            else{
                //Push the changed priority task back into ready Q
                //Condition if setting task to equal or lower priority than running task
                push(&ready_queue_head,task);
            }
        }
        //changing priority for current running task
        else if (gp_current_task->prio > ready_queue_head->prio){
            //Yielding the current running task only if ready_queue_head
            //has a higher priority than the currently running task
            k_tsk_yield();
        }
        //No changes made if running task prio is higher or equal to prio of ready Q head

    }

    print_prio_queue(ready_queue_head);

    return RTX_OK;    
}

int k_tsk_get(task_t task_id, RTX_TASK_INFO *buffer) {
    #ifdef DEBUG_0
    printf("k_tsk_get: entering...\n\r");
    printf("task_id = %d, buffer = 0x%x.\n\r", task_id, buffer);
    #endif /* DEBUG_0 */

    if (buffer == NULL) {
        return RTX_ERR;
    }

    // TODO: what if we try to access non-existent task, DORMANT
    if (task_id >= MAX_TASKS || task_id < 0) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_get: task ID outside of TID domain\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    if (tid_is_available(free_tid_head, task_id) == 1) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_get: task ID does not exist or dormant\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    TCB *task = &g_tcbs[task_id];

    if (task == NULL) {
        return RTX_ERR;
    }

    buffer->tid = task->tid;
    buffer->prio = task->prio;
    buffer->state = task->state;
    buffer->priv = task->priv;
    buffer->k_stack_size = KERN_STACK_SIZE;
    buffer->k_sp = __get_MSP();
    buffer->k_stack_hi = (U32) task->msp_hi;

    if (task->priv == 0) {
        buffer->u_stack_size = task->psp_size;
        buffer->u_stack_hi = (U32) task->psp_hi;
        buffer->u_sp = __get_PSP();
        buffer->ptask = (void (*)()) (task->psp_hi - 2);
    } else {
        buffer->u_stack_size = 0;
        buffer->u_stack_hi = NULL;
        buffer->u_sp = NULL;
        buffer->ptask = (void (*)()) (task->msp_hi - 2);
    }

    return RTX_OK;     
}

int k_tsk_ls(task_t *buf, int count){
    #ifdef DEBUG_0
    printf("k_tsk_ls: buf=0x%x, count=%d\r\n", buf, count);
#endif /* DEBUG_0 */
	
	int actual_count = 0;
	int buf_index = 0;
	
	for (int i = MAX_TASKS-1; i >= 0; i--)
	{
		if (g_tcbs[i].state != DORMANT) {
			actual_count++;
			buf[buf_index++] = g_tcbs[i].tid;
		}
		
		if (actual_count == count)
			break;
	}
    return actual_count;
}
