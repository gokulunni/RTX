//
// Created by Boris Nguyen on 2020-12-04.
//

#include "timeval.h"
#include "k_time.h"
#include "k_rtx.h"
#include "rtx.h"
#include "linked_list.h"

extern TCB *ready_queue_head;
extern int task_switch(TCB *p_tcb_old);

void ps_task(void) {
    /* Pop next non-rt task from queue */
    struct timeval_rt budget = (struct timeval_rt) {kernel_sys_info.server.b_n.sec, kernel_sys_info.server.b_n.usec};

    TCB *next_task = NULL;
    struct timeval_rt startTime;
    struct timeval_rt endTime;
    while (!is_empty(ready_queue_head) && (budget.sec != 0 && budget.user != 0)) {
        next_task = pop(&ready_queue_head);

        get_time(&startTime);
        TCB *p_tcb_old = gp_current_task;
        gp_current_task = next_task;
        task_switch(p_tcb_old);
        get_time(&endTime);

        sub(&endTime, endTime, startTime);
        sub(&budget, budget, endTime);
    }

    tsk_done_rt();
}
