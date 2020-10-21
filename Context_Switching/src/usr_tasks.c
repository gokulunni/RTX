/**
 * @file:   usr_tasks.c
 * @brief:  Two user/unprivileged  tasks: task1 and task2
 * @author: Yiqing Huang
 * @date:   2020/09/20
 * NOTE: Each task is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_tasks.h"
#include "printf.h"
#include <LPC17xx.h>

 /* Global vars */
int *pointer;
int tests_completed = 0;
int total_tests; /* set based on which suite is being run */
int passed = 0;


void print_test_result(int test_num, int result, char *test_name)
{
	if(result == RTX_OK)
	{
		passed++;
		printf("G04_test: test %d (%s) OK\n", test_num, test_name);
	}
	else
		printf("G04_test: test %d (%s) FAIL\n", test_num, test_name);
	
	tests_completed++;
}

void print_test_start(int suite_num)
{
	printf("G04_test - suite_%d: START\n", suite_num);  
}

void print_final_results()
{
	printf("%d/%d OK\n", passed, total_tests);
  printf("%d/%d FAIL\n", total_tests-passed, total_tests);
  printf("G04_test: END\n");
}

/**
 * @brief: a dummy task1 - starter code
 */
void task1(void)
{
    task_t tid;
    RTX_TASK_INFO task_info;
    
    uart1_put_string ("task1: entering \n\r");
    /* do something */
    tsk_create(&tid, &task2, LOW, 0x200);  /*create a user task */
    tsk_get(tid, &task_info);
    tsk_set_prio(tid, LOWEST);
    /* terminating */
    tsk_exit();
}

/**
 * @brief: a dummy task2
 */
void task2(void)
{
	total_tests = 1;
	print_test_start(1);
    //uart1_put_string ("task2: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}

/**
 * @brief: a dummy task3
 */
void task3(void)
{
    //uart1_put_string ("task3: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}


/**
 * @brief: a dummy task4
 */
void task4(void)
{
    //uart1_put_string ("task4: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}


/**
 * @brief: a dummy task5
 */
void task5(void)
{
    //uart1_put_string ("task5: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}


/**
 * @brief: a dummy task6
 */
void task6(void)
{
    //uart1_put_string ("task6: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}


/**
 * @brief: a dummy task7
 */
void task7(void)
{
    //uart1_put_string ("task7: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}


/**
 * @brief: a dummy task8
 */
void task8(void)
{
    //uart1_put_string ("task8: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}


/**
 * @brief: a dummy task9
 */
void task9(void)
{
    //uart1_put_string ("task9: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}

/**
 * @brief: a dummy task10
 */
void task10(void)
{
    //uart1_put_string ("task10: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}



/**
 * @brief: a dummy task11
 */
void task11(void)
{
    //uart1_put_string ("task11: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}



/**
 * @brief: a dummy task12
 */
void task12(void)
{
    //uart1_put_string ("task12: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}



/**
 * @brief: a dummy task13
 */
void task13(void)
{
    //uart1_put_string ("task13: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}



/**
 * @brief: a dummy task14
 */
void task14(void)
{
    //uart1_put_string ("task14: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}



/**
 * @brief: a dummy task15
 */
void task15(void)
{
		print_test_result(1, RTX_OK, "MAX_TASKS Test");
		print_final_results();
    
		//uart1_put_string ("task15: entering \n\r");
    /* do something */
    /* terminating */
    tsk_exit();
}


void alloc_pointer_task(void)
{
	total_tests = 2;
	print_test_start(2);
    //uart1_put_string ("alloc_pointer_task: entering \n\r");
    /* do something */
    /* terminating */
		pointer = mem_alloc(sizeof(int));
    tsk_exit();
}

/**
 * @brief: a dummy task3
 */
void dealloc_pointer_task(void)
{
    //uart1_put_string ("dealloc_pointer_task: entering \n\r");
    /* do something */
    /* terminating */
		if(mem_dealloc(pointer) == RTX_OK)
			print_test_result(1, RTX_ERR, "Memory ownership test");
		else
			print_test_result(1, RTX_OK, "Memory ownership test");
		
    tsk_exit();
}

void ctl_reg_verification_task(void)
{
	if(__get_CONTROL() == 3)
		print_test_result(2, RTX_OK, "Control Register test");
	else
		print_test_result(2, RTX_ERR, "Control Register test");
	
	print_final_results();
		
}


