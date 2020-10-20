/**
 * @file:   main_svc.c
 * @brief:  main routine to start up the RTX and two initial tasks
 * @author: Yiqing Huang
 * @date:   2020/09/24
 * NOTE: standard C library is not allowed in the final kernel code.
 *       A tiny printf function for embedded application development
 *       taken from http://www.sparetimelabs.com/tinyprintf/tinyprintf.php
 *       is configured to use UART0 to output when DEBUG_0 is defined.
 *       Check target option->C/C++ to see the DEBUG_0 definition.
 *       Note that init_printf(NULL, putc) must be called to initialize 
 *       the printf function.
 */

#include <LPC17xx.h>
#include "rtx.h"
#include "priv_tasks.h"
#include "uart_polling.h"
#include "usr_tasks.h"
#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

#ifdef RAM_TARGET
#define IROM_BASE  0x10000000
#else
#define IROM_BASE  0x0
#endif

int total_tests = 0;
int (*tests[1]) (void);
char *test_names[1];


int main_og() 
{    
    RTX_TASK_INFO task_info[2];    
    /* CMSIS system initialization */
    SystemInit();  /* initiaslize the system */
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
    //set_task_info(task_info, 2);
    /* start the RTX and built-in tasks */
    //rtx_init(32, FIRST_FIT, task_info, 2);  
		
		/*Run the tests */
		int passed = 0;
		printf("G04_test: START\n");  
		for(int i = 0; i < total_tests; i++)
		{
			if((*tests[i])())
			{
				printf("G04_test: test %d (%s) OK\n", i+1, test_names[i]);
				passed++;
			}
			else
				printf("G04_test: test %d (%s) FAIL\n", i+1, test_names[i]);
		}

		printf("%d/%d OK\n", passed, total_tests);
		printf("%d/%d FAIL\n", total_tests-passed, total_tests);
		printf("G04_test: END\n");
  
		
    /* We should never reach here!!! */
    return RTX_ERR;  
}
