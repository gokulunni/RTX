/**
 * @file:   k_task.h
 * @brief:  task management hearder file
 * @author: Yiqing Huang
 * @date:   2020/09/26
 * NOTE: Assuming there are only two user tasks in the system
 */

#ifndef K_TASK_H_
#define K_TASK_H_

#include "k_rtx.h"

/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */

/* ----- Functions ----- */

int k_tsk_init(RTX_TASK_INFO *task_info, int num_tasks);    /* initialize all tasks in the system */
TCB *dummy_scheduler(void);      /* pick the tid of the next to run task */
int k_tsk_yield(void);           /* kernel tsk_yield function */

int k_tsk_create(task_t *task, void (*task_entry)(void), U8 prio, U16 stack_size);
void k_tsk_exit(void);
int k_tsk_set_prio(task_t task_id, U8 prio);
int k_tsk_get(task_t task_id, RTX_TASK_INFO *buffer);

extern void __rte(void);               /* pop exception stack frame */

#endif /* ! K_TASK_H_ */
