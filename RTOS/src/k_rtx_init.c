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

int rtx_init_rt(RTX_SYS_INFO *sys_info, RTX_TASK_INFO *tasks, int num_tasks){
	
	
}

int get_sys_info(RTX_SYS_INFO *buffer){
	
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
