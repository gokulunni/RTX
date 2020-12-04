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

RTX_SYS_INFO sys_info_copy;
int sys_info_set=0;

int k_rtx_init_rt(RTX_SYS_INFO *sys_info, RTX_TASK_INFO *tasks, int num_tasks){
		
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
		
		//save sys_info
		sys_info_copy.mem_blk_size = sys_info->mem_blk_size;
		sys_info_copy.mem_algo = sys_info->mem_algo;
		sys_info_copy.rtx_time_qtm = sys_info->rtx_time_qtm;
		sys_info_copy.server = sys_info->server;
		sys_info_copy.sched = sys_info->sched;
		sys_info_set=1;
		
    
    /* start the first task */
    return k_tsk_yield();
}


int k_get_sys_info(RTX_SYS_INFO *buffer){
	
		if (buffer!=NULL){
			return 0;
		}
		
		buffer->mem_blk_size =sys_info_copy.mem_blk_size;
		buffer->mem_algo=sys_info_copy.mem_algo;
		buffer->rtx_time_qtm=sys_info_copy.rtx_time_qtm;
		buffer->server=sys_info_copy.server;
		buffer->sched=sys_info_copy.sched;
	
		return sys_info_set;
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
