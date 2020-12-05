/**
 * @file:   main_svc.c
 * @brief:  main routine to start up the RTX and two initial tasks
 * @author: Yiqing Huang
 * @date:   2020/10/20
 * NOTE: standard C library is not allowed in the final kernel code.
 *       A tiny printf function for embedded application development
 *       taken from http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
 *       is configured to use UART0 to output when DEBUG_0 is defined.
 *       Check target option->C/C++ to see the DEBUG_0 definition.
 *       Note that init_printf(NULL, putc) must be called to initialize 
 *       the printf function.
 * IMPORTANT: This file will be replaced by another file in automted testing.
 */

#include <LPC17xx.h>
#include "rtx.h"
#include "priv_tasks.h"
#include "uart_polling.h"
#include "usr_tasks.h"
#include "timer.h"
#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#ifdef RAM_TARGET
#define IROM_BASE  0x10000000
#else
#define IROM_BASE  0x0
#endif

extern void lcd_task(void);
extern void kcd_task(void);
extern void wall_clock_task(void);
extern void null_task(void);
void benchmark_task(void);

int set_fixed_tasks(RTX_TASK_INFO *tasks, int num_tasks){

    if (num_tasks != 4) {
        return RTX_ERR;
    }

    tasks[0].ptask = &lcd_task;
    tasks[0].u_stack_size = 0x0;
    tasks[0].prio = HIGH;
    tasks[0].priv = 1;
    
    tasks[1].ptask = &kcd_task;
    tasks[1].u_stack_size = 0x100;
    tasks[1].prio = HIGH;
    tasks[1].priv = 0;

    tasks[2].ptask = &wall_clock_task;
    tasks[2].u_stack_size = 0x100;
    tasks[2].prio = HIGH;
    tasks[2].priv = 0;

    tasks[3].ptask = &null_task;
    tasks[3].u_stack_size = 0x100;
    tasks[3].prio = PRIO_NULL;
    tasks[3].priv = 0;

    return RTX_OK;
}

void benchmark_task_mem_alloc(void)
{
	struct timeval_rt time;
	
	//Fragment the heap
	void *pointers[50];
	for(int i = 0; i < 50; i++)
	{
		pointers[i] = mem_alloc((i*3)%14 + 4);
		if(pointers[i] == NULL)
		{
			tsk_exit();
		}
	}
	
	mem_dealloc(pointers[10]);
	mem_dealloc(pointers[20]);
	mem_dealloc(pointers[30]);
	mem_dealloc(pointers[40]);
	mem_dealloc(pointers[49]);
	
	start_timer1();
	void *tmp = mem_alloc(128);
	end_timer1();
	
	get_timer1(&time);
	mem_dealloc(tmp);
	#ifdef DEBUG_0
	printf("8 bytes - Seconds: %d, microseconds: %d\n", time.sec, time.usec);
	#endif
	tsk_exit();
}
int main() 
{      
    RTX_TASK_INFO task_info[5];    /* 6 tasks, only 2 are used in uncommented code */
   
    /* CMSIS system initialization */
    SystemInit();  /* initialize the system */
    __disable_irq();
    uart_init(1);  /* uart1 uses polling for output */
#ifdef DEBUG_0
    init_printf(NULL, putc);
#endif /* DEBUG_0 */
    __enable_irq();
#ifdef DEBUG_0
    printf("Dereferencing Null to get inital SP = 0x%x\r\n", *(U32 *)(IROM_BASE));
    printf("Derefrencing Reset vector to get intial PC = 0x%x\r\n", *(U32 *)(IROM_BASE + 4));
    printf("We are at privileged level, so we can access SP.\r\n"); 
    printf("Read MSP = 0x%x\r\n", __get_MSP());
    printf("Read PSP = 0x%x\r\n", __get_PSP());
#endif /*DEBUG_0*/    
    /* sets task information */
    set_fixed_tasks(task_info, 4);  /* kcd, lcd, null tasks */
		task_info[4].ptask = &benchmark_task_mem_alloc;
		task_info[4].u_stack_size = 0x000;
		task_info[4].prio = MEDIUM;
		task_info[4].priv = 1;
		
    /* start the RTX and built-in tasks */
    rtx_init(32, FIRST_FIT, task_info+4, 1);
    /* We should never reach here!!! */
    return RTX_ERR;  
}
