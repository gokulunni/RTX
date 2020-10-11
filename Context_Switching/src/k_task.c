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
#include "PriorityQueue.h"
#include "k_mem.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
TCB *gp_current_task = NULL;    /* always point to the current RUN process */

/* TCBs and Kernel stacks are statically allocated and is inside the OS image */
TCB g_tcbs[MAX_TASKS];
U32 g_k_stacks[MAX_TASKS][KERN_STACK_SIZE >> 2];
U32 total_num_tasks = 0;

PriorityQueue *ready_queue;

typedef struct free_tid {
    int tid;
    struct free_tid* next;
} FREE_TID_T;

FREE_TID_T *free_tid_head = NULL;

void push_tid(int tid) {
    FREE_TID_T new_tid = {tid, free_tid_head};

    free_tid_head = &new_tid;
    
    return;
}

int pop_tid() {
    if (free_tid_head == NULL) {
        return 0;
    }
    int tid = free_tid_head->tid;
    free_tid_head = free_tid_head->next;
    return tid;
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
int k_tsk_init(RTX_TASK_INFO *task_info, int num_tasks) 
{
    int i;
    U32 *sp;
    RTX_TASK_INFO *p_taskinfo = task_info;
  
    //Create queues
    TCB kernal_task;
    kernal_task.tid = MAX_TASKS + 1;
    gp_current_task = &kernal_task;
    ready_queue = k_mem_alloc(sizeof(PriorityQueue));

    TCB* null_task = &g_tcbs[0]; // TODO: check index
    null_task->tid = 0;
    gp_current_task = null_task;
    null_task->state = NEW;
    null_task->psp = k_mem_alloc(0x18); // TODO: double check with TA
    null_task->msp = sp;
    null_task->prio = PRIO_NULL;
    null_task->priv = 0;
    total_num_tasks++;

    /* Pretend an exception happened, by adding exception stack frame */
    /* initilize exception stack frame (i.e. initial context) for each task */
    for (i = 0; i < num_tasks; i++) { // TODO: check that num task less than max
        int j;
        TCB *p_tcb = &g_tcbs[i+1];
        p_tcb ->tid = i+1;
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
            gp_current_task = p_tcb; // TODO: who owns user stack
            p_tcb -> psp = k_mem_alloc(p_taskinfo->u_stack_size);
        } else {
            p_tcb->psp = p_tcb->msp;
            p_tcb->priv = 1;
        }

        //Add task to the priority queue for NEW tasks
        push(ready_queue, p_tcb);
				
        total_num_tasks++;
        p_taskinfo++;
    }

    for (; i < MAX_TASKS; i++) {
        push_tid(i+1);
    }

    gp_current_task = null_task;

    return RTX_OK;
}

/*@brief: scheduler, pick the tid of the next to run task
 *@return: TCB pointer of the next to run task
 *         NULL if error happens
 *POST: if gp_current_task was NULL, then it gets set to tcb[1].
 *      No other effect on other global variables.
 */

TCB *dummy_scheduler(void) {
	
	//This should never be false
	//except potentially if the null task is running
    if(!isEmpty(ready_queue)) {
        TCB *popped = pop(ready_queue);

				//Push current task back on ready queue
        if (gp_current_task) {
            push(ready_queue, gp_current_task);
        }

        gp_current_task = popped;
    }

    return gp_current_task;
}

/*@brief: switch out old tcb (p_tcb_old), run the new tcb (gp_current_task)
 *@param: p_tcb_old, the old tcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_tcb_old and gp_current_task are pointing to valid TCBs.
 */
int task_switch(TCB *p_tcb_old) 
{
    U8 state;
    
    state = gp_current_task->state;

    if (state == NEW) {
        if (gp_current_task != p_tcb_old && p_tcb_old->state != NEW) {
            p_tcb_old->state = READY;
            p_tcb_old->msp = (U32 *) __get_MSP();
        }
        gp_current_task->state = RUNNING;
        __set_MSP((U32) gp_current_task->msp);
        __rte();  /* pop exception stack frame from the stack for a new task */
    } 
    
    /* The following will only execute if the if block above is FALSE */

    if (gp_current_task != p_tcb_old) {
        if (state == READY){         
            p_tcb_old->state = READY; 
            p_tcb_old->msp = (U32 *) __get_MSP(); // save the old process's sp
            gp_current_task->state = RUNNING;
            __set_MSP((U32) gp_current_task->msp); //switch to the new proc's stack    
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
int k_tsk_yield(void)
{
    //Their starter code
    // TCB *p_tcb_old = NULL;
    
    // p_tcb_old = gp_current_task;
    // gp_current_task = dummy_scheduler();
    
    // if ( gp_current_task == NULL  ) {
    //     gp_current_task = p_tcb_old; // revert back to the old task
    //     return RTX_ERR;
    // }
    // if ( p_tcb_old == NULL ) {
    //     p_tcb_old = gp_current_task;
    // }
    // task_switch(p_tcb_old);
    // return RTX_OK;

    //Keep track of task that was running and name it p_tcb_old
    //Get the old task
    TCB *p_tcb_old = gp_current_task;
    //Pop the next task in queue

    gp_current_task = dummy_scheduler();

    #ifdef DEBUG_0
    printf("Yielding task with ID: %d \n",p_tcb_old->tid);
    #endif /* DEBUG_0 */


    //If nothing can replace it 
    if (gp_current_task == NULL){
        #ifdef DEBUG_0
        printf("Ready queue popped NULL task during yield()");
        #endif /* DEBUG_0 */
        gp_current_task=p_tcb_old;
        return RTX_ERR;
    }
    if (p_tcb_old == NULL){
        #ifdef DEBUG_0
        printf("gp_current_task was NULL while Yield() was called");
        #endif /* DEBUG_0 */
        p_tcb_old = gp_current_task;
    }
    task_switch(p_tcb_old);

    return RTX_OK;

}

int k_tsk_create(task_t *task, void (*task_entry)(void), U8 prio, U16 stack_size)
{
#ifdef DEBUG_0
    printf("k_tsk_create: entering...\n\r");
    printf("task = 0x%x, task_entry = 0x%x, prio=%d, stack_size = %d\n\r", task, task_entry, prio, stack_size);
#endif /* DEBUG_0 */

    if (total_num_tasks >= MAX_TASKS)
        return -1;

    if (free_tid_head != NULL) {
        int tid = pop_tid();
        TCB* new_task = &g_tcbs[tid];
        new_task->tid = tid;
        new_task->state = NEW;
        new_task->psp = k_mem_alloc(stack_size);
        new_task->prio = prio;

        U32* sp = g_k_stacks[total_num_tasks + 1] + (KERN_STACK_SIZE >> 2);
        *(--sp) = INITIAL_xPSR;
        *(--sp) = (U32)(task_entry);
        for (int j = 0; j < 6; j++) {
            *(--sp) = 0x0;
        }
        new_task->msp = sp;

        push(ready_queue, new_task);
        if(gp_current_task->prio > new_task->prio)  {
            //must run immediately
            k_tsk_yield();
        }
        task = &new_task->tid;

        total_num_tasks++;

        return RTX_OK;
    }

#ifdef DEBUG_0
    printf("k_tsk_create: No available tid\n\r");
#endif /* DEBUG_0 */

    return RTX_ERR;
}

void k_tsk_exit(void) 
{
#ifdef DEBUG_0
    printf("k_tsk_exit: entering...\n\r");
#endif /* DEBUG_0 */
    //Keep track of task that was running and name it p_tcb_old
    //Get current running task
    TCB *p_tcb_old = gp_current_task;
    if (p_tcb_old->prio != PRIO_NULL) {
        gp_current_task->state = DORMANT;
        k_mem_dealloc(gp_current_task->psp);
        push_tid(gp_current_task->tid);
        gp_current_task = get_task_by_id(ready_queue, 0);
        gp_current_task = dummy_scheduler();
    }

    return;
}

int k_tsk_set_prio(task_t task_id, U8 prio) 
{
#ifdef DEBUG_0
    printf("k_tsk_set_prio: entering...\n\r");
    printf("task_id = %d, prio = %d.\n\r", task_id, prio);
#endif /* DEBUG_0 */
    TCB* task = get_task_by_id(ready_queue, task_id);

    if (task != NULL) {
        // An unprivileged task may change the priority of any other unprivileged task (including itself).
        // A privileged task may change the priority of any other task (including itself).
        // The priority of the null task cannot be changed and remains at level PRIO_NULL.
        if (((gp_current_task->priv == 0 && task->priv == 0) || gp_current_task->priv == 1) && task->prio != PRIO_NULL) {
            task->prio = prio;

            //    The caller of this primitive never blocks, but could be preempted.
            //    If the value of prio is higher than the priority of the current running task,
            //    and the task identified by task id is in ready state, then the task identified by
            //    the task id preempts the current running task. Otherwise, the current running task
            //    continues its execution.
            if (task->state == READY && task->prio < gp_current_task->prio) {
                TCB *p_tcb_old = gp_current_task;
                push(ready_queue, p_tcb_old);
                gp_current_task = task;
                pop(ready_queue);

                task_switch(p_tcb_old);
            }
        }
    }

    return RTX_OK;    
}

int k_tsk_get(task_t task_id, RTX_TASK_INFO *buffer)
{
    TCB *task;

#ifdef DEBUG_0
    printf("k_tsk_get: entering...\n\r");
    printf("task_id = %d, buffer = 0x%x.\n\r", task_id, buffer);
#endif /* DEBUG_0 */    
    if (buffer == NULL) {
        return RTX_ERR;
    }

    // TODO: what is this buffer?
    // TODO: is it only for ready tasks?
    if (task_id == gp_current_task->tid) {
        task = gp_current_task;
    } else {
        task = get_task_by_id(ready_queue, task_id);
    }

    /* The code fills the buffer with some fake task information. 
       You should fill the buffer with correct information    */
    buffer->tid = task_id;
    buffer->prio = task->prio;
    buffer->state = task->state;
    buffer->priv = task->priv;
    buffer->ptask = (void *) task;
    buffer->k_sp = (U32) task->msp;
    buffer->k_stack_size = KERN_STACK_SIZE;
    buffer->u_sp = (U32) task->psp;
    buffer->u_stack_size = 0x200;

    return RTX_OK;     
}
