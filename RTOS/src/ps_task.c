#include "k_rtx.h"
#include "linked_list.h"

extern TCB *ready_queue_head;
extern int task_switch(TCB *p_tcb_old);

void ps_task(void)
{
	/* Pop next non-rt task from queue */
	TCB *next_task = NULL;
	if (!is_empty(ready_queue_head))
	{
				next_task = pop(&ready_queue_head);
		}

	/* Switch tasks */
	task_switch(next_task);

	/* TO DO: modify tsk_exit() so that non-rt tasks switch back to polling server task */
}
