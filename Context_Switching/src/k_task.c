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
TCB *gp_current_task = NULL;    /* always point to the current RUN process */

/* TCBs and Kernel stacks are statically allocated and is inside the OS image */
TCB g_tcbs[MAX_TASKS];
U32 g_k_stacks[MAX_TASKS][KERN_STACK_SIZE >> 2];

// Kernal Fake Task used for mem alloc of kernel necessary data
TCB kernal_task;
TCB *null_task;

TCB *ready_queue_head = NULL;
FREE_TID_T *free_tid_head = NULL;

__asm void __set_SP_to_PSP() {
   MOV R3, #2                 ; priviledged, use PSP (01)
   MSR CONTROL, R3
}

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
/**
 * @biref: initialize all tasks in the system
 * @param: RTX_TASK_INFO *task_info, an array of initial tasks
 * @param: num_tasks, number of elements in the task_info array
 * @return: none
 * PRE: memory has been properly initialized
 */
int k_tsk_init(RTX_TASK_INFO *task_info, int num_tasks) {
	/* Default is MSP when calling tsk_init(), set to PSP */
	  //__set_PSP((U32) __get_MSP());
		//__set_CONTROL((U32)3);
	 
	  
    int i;
    U32 *sp;
    RTX_TASK_INFO *p_taskinfo = task_info;

    for (int p = 0; p < MAX_TASKS; p++) {
        g_tcbs[p] = NULL;
    }
  
    // Create fake kernal task with ID = MAX_TASKS+1
    kernal_task.tid = MAX_TASKS + 1;
    gp_current_task = &kernal_task;


    ready_queue_head = k_mem_alloc(sizeof(TCB));


    null_task = &g_tcbs[0]; // TODO: check index
    null_task->tid = PID_NULL;
    null_task->state = NEW;
    null_task->psp = k_mem_alloc(0x18); // TODO: double check with TA
    null_task->psp_size = 0x18;
    null_task->msp = sp; // TODO: assign a value to SP
    null_task->prio = PRIO_NULL;
    null_task->priv = 0;

    /* Pretend an exception happened, by adding exception stack frame */
    /* initilize exception stack frame (i.e. initial context) for each task */
    // TODO: if PRIO is NULL, skip that task
    for (i = 0; i < num_tasks; i++) { // TODO: check that num task less than max
        int j;
        TCB *p_tcb = &g_tcbs[i+1];

        // TODO: can we skip NULL_PRIO task?
        if (p_tcb->prio == PRIO_NULL) {
            continue;
        }

        p_tcb->tid = i+1;
        p_tcb->state = NEW;
        sp = g_k_stacks[i+1] + (KERN_STACK_SIZE >> 2) ; /* stacks grows down, so get the high addr. */
        *(--sp)  = INITIAL_xPSR;    /* task initial xPSR (program status register) */
        *(--sp)  = (U32)(p_taskinfo->ptask); /* PC contains the entry point of the task */
        for ( j = 0; j < 6; j++ ) { /*R0-R3, R12, LR */
            *(--sp) = 0x0;
        }
        p_tcb->msp = sp;

        if ( p_taskinfo->priv == 0 ) { /* unpriviledged task */ 
            /* allocate user stack, not implemented */
            p_tcb->priv = 0;
            //allocate user stack and point psp to it
            p_tcb->psp = k_mem_alloc(p_taskinfo->u_stack_size);
            p_tcb->psp_size = p_taskinfo->u_stack_size;
        } else {
            p_tcb->priv = 1;
            p_tcb->psp = p_tcb->msp; // TODO: double check
        }

        //Add task to the priority queue for NEW tasks
        push(&ready_queue, p_tcb);

        p_taskinfo++;
    }

    for (int q = MAX_TASKS - 1; q > i; q--) {
        FREE_TID_T *new_tid = k_mem_alloc(sizeof(FREE_TID_T));
        new_tid->tid = q;
        push_tid(&free_tid_head, new_tid);
    }

    print_free_tids(free_tid_head);
    print_prio_queue(ready_queue);

    gp_current_task = null_task;

    return RTX_OK;
}

/*@brief: scheduler, pick the tid of the next to run task
 *@return: TCB pointer of the next to run task
 *         NULL if error happens
 *POST: if gp_current_task was NULL, then it gets set to tcb[1]. // TODO: why?
 *      No other effect on other global variables.
 */

TCB *dummy_scheduler(void) {
	//This should never be false except if the NULL_PRIO task is running
    if(!is_empty(ready_queue_head)) {
        TCB *popped = pop(&ready_queue_head);

        // If there is a current task, push current task back on ready queue
        if (gp_current_task) {
            push(&ready_queue_head, gp_current_task);
        }

        gp_current_task = popped;
    }

    if (gp_current_task == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] dummy_scheduler: gp_current_task is NULL\n",p_tcb_old->tid);
        #endif /* DEBUG_0 */
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
        __set_PSP((U32)gp_current_task->psp); // TODO: not implemented
				
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
    print_prio_queue(ready_queue);
    task_switch(p_tcb_old);

    return RTX_OK;

}



// TODO: DONT MAKE ASSUMPTION MALLOC WORKED
// TODO: argv?
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
    }

    if (free_tid_head == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: no available TID\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    TCB *prev_current_task = gp_current_task;
    gp_current_task = &kernal_task;
    FREE_TID_T *popped_tid = pop_tid(&free_tid_head);

    if (popped_tid == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_create: no available TID\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

    int tid = popped_tid->tid;
    k_mem_dealloc(popped_tid);

    // TODO: double check that kernal task owns all user stacks even if task created inside another task
    TCB *new_task = &g_tcbs[tid];

    new_task->psp = k_mem_alloc(stack_size);
    new_task->psp_size = stack_size;
    gp_current_task = prev_current_task;

    new_task->next = NULL;
    new_task->tid = tid;
    new_task->state = NEW;
    new_task->prio = prio;
    new_task->priv = 0;

    U32 *sp = g_k_stacks[tid] + (KERN_STACK_SIZE >> 2);
    *(--sp) = INITIAL_xPSR;
    *(--sp) = (U32)(task_entry);
    for (int j = 0; j < 6; j++) {
        *(--sp) = 0x0;
    }
    new_task->msp = sp;

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
    printf("k_tsk_exit: entering...\n\r");
#endif /* DEBUG_0 */
    // A PRIO_NULL task cannot exit
    if (gp_current_task->prio != PRIO_NULL) {
        gp_current_task->state = DORMANT;

        // If its unpriviledged task, dealloc user stack
        if (gp_current_task->priv == 0) {
            k_mem_dealloc(gp_current_task->psp);
            gp_current_task->psp = NULL;
        }

        // Temporarily set gp_current_task to malloc FREE_TID_T
        TCB *p_tcb_old = gp_current_task;
        gp_current_task = &kernal_task;
        FREE_TID_T *new_tid = k_mem_alloc(sizeof(FREE_TID_T));
        new_tid->tid = p_tcb_old->tid;
        push_tid(&free_tid_head, new_tid);

        gp_current_task = NULL;
        gp_current_task = dummy_scheduler();
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
    }

    // The priority of the null task cannot be changed and remains at level PRIO_NULL.
    if (task_id == 0) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: cannot change prio of NULL task\n\r");
        #endif /* DEBUG_0 */
        return RTX_ERR;
    }

//    TCB *task = get_task_by_id(&ready_queue_head, task_id);
// TODO: can i change PRIO of dormant task?

    if (task_id >= MAX_TASKS) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_get: task ID outside of TID domain\n\r");
        #endif /* DEBUG_0 */
    }

    TCB *task = g_tcbs[task_id];

    if (task == NULL) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_set_prio: task with tid %d not found\n\r", task_id);
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

        // TODO: insert before or after

        //    The caller of this primitive never blocks, but could be preempted.
        //    If the value of prio is higher than the priority of the current running task,
        //    and the task identified by task id is in ready state, then the task identified by
        //    the task id preempts the current running task. Otherwise, the current running task
        //    continues its execution.

        // TODO: what about NEW state?
        if (task->state == READY && task->prio < gp_current_task->prio) {
            TCB *p_tcb_old = gp_current_task;
            push(&ready_queue_head, p_tcb_old);
            gp_current_task = task;
            task_switch(p_tcb_old);
        } else {
            push(&ready_queue_head, task);
        }
    }

    print_prio_queue(&ready_queue);

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

    // TODO: can we get dormant task? what if task has been overwritten?

    if (task_id >= MAX_TASKS) {
        #ifdef DEBUG_0
        printf("[ERROR] k_tsk_get: task ID outside of TID domain\n\r");
        #endif /* DEBUG_0 */
    }

    TCB *task = g_tcbs[task_id];

    if (task == NULL) {
        return RTX_ERR;
    }

    buffer->tid = task->tid;
    buffer->prio = task->prio;
    buffer->state = task->state;
    buffer->priv = task->priv;
    buffer->ptask = (void *) task; //TODO: double check if its right or not
    buffer->k_sp = (U32) task->msp; // TODO: confirm this is indeed kernal stack, should i use get_MSP()
    buffer->k_stack_size = KERN_STACK_SIZE;
    buffer->u_sp = (U32) task->psp; // TODO: confirm this is indeed user stack, should i use get_PSP(), what if its priviliged task?
    buffer->u_stack_size = task->psp_size; // TODO: what is the size of privileged task

    return RTX_OK;     
}
