/** 
 * @file:   k_rtx_init.c
 * @brief:  Kernel initialization C file
 * @auther: Yiqing Huang
 * @date:   2020/09/26
 */

#include "k_rtx_init.h"
#include "uart_polling.h"
#include "uart_irq.h"
#include "k_mem.h"
#include "k_task.h"
#include "timer.h"

RTX_SYS_INFO kernel_sys_info;

int k_rtx_init_rt(RTX_SYS_INFO *sys_info, RTX_TASK_INFO *tasks, int num_tasks){

    if (sys_info == NULL) {
        return RTX_ERR;
    }

    if (tasks == NULL) {
        return RTX_ERR;
    }

    if (num_tasks < 1 || num_tasks > MAX_TASKS) {
        return RTX_ERR;
    }

    if (sys_info->mem_algo != FIXED_POOL && sys_info->mem_algo != FIRST_FIT && sys_info->mem_algo != BEST_FIT && sys_info->mem_algo != WORST_FIT) {
        return RTX_ERR;
    }

    if (!(sys_info->mem_blk_size > 0)) {
        return RTX_ERR;
    }

    if (sys_info->sched != DEFAULT && sys_info->sched != RM_NPS && sys_info->sched != RM_PS && sys_info->sched != EDF) {
        return RTX_ERR;
    }

    if (sys_info->rtx_time_qtm % MIN_RTX_QTM != 0) {
        return RTX_ERR;
    }

    /* interrupts are already disabled when we enter here */
    if (uart_irq_init(0) != RTX_OK) {
        return RTX_ERR;
    }
    
    if (k_mem_init(sys_info->mem_blk_size, sys_info->mem_algo) != RTX_OK) {
        return RTX_ERR;
    }

    if (k_tsk_init(tasks, num_tasks) != RTX_OK ) {
        return RTX_ERR;
    }

    if (timer_init(0) != RTX_OK){
        return RTX_ERR;
    }

    if (timer_init_100MHZ(1) != RTX_OK) {
        return RTX_ERR;
    }

    kernel_sys_info.mem_blk_size = sys_info->mem_blk_size;
    kernel_sys_info.mem_algo = sys_info->mem_algo;
    kernel_sys_info.rtx_time_qtm = sys_info->rtx_time_qtm;
    kernel_sys_info.server = sys_info->server;
    kernel_sys_info.sched = sys_info->sched;

    /* start the first task */
    return k_tsk_yield();
}


int k_rtx_init(size_t blk_size, int algo, RTX_TASK_INFO *task_info, int num_tasks)
{
    /* interrupts are already disabled when we enter here */
    if (uart_irq_init(0) != RTX_OK) {
        return RTX_ERR;
    }
    
    if (k_mem_init(blk_size, algo) != RTX_OK) {
        return RTX_ERR;
    }

    if (k_tsk_init(task_info, num_tasks) != RTX_OK ) {
        return RTX_ERR;
    }

    if (timer_init(0) != RTX_OK){
        return RTX_ERR;
    }

    if (timer_init_100MHZ(1) != RTX_OK) {
        return RTX_ERR;
    }
    
    /* start the first task */
    return k_tsk_yield();
}

nt k_get_sys_info(RTX_SYS_INFO *buffer) {
    if (buffer == NULL) {
        return RTX_ERR;
    }

    buffer->mem_blk_size = kernel_sys_info.mem_blk_size;
    buffer->mem_algo = kernel_sys_info.mem_algo;
    buffer->rtx_time_qtm = kernel_sys_info.rtx_time_qtm;
    buffer->server = kernel_sys_info.server;
    buffer->sched = kernel_sys_info.sched;

    return sys_info_set;
}	}