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
int schedule_incrementer = 0;
task_t task_property_id;
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

void property_verification_task_1(void)
{
	total_tests = 1;
	print_test_start(5);
	//uart1_put_string ("alloc_pointer_task: entering \n\r");
	tsk_create(&task_property_id, task14, LOW, 0x250);
	
	RTX_TASK_INFO buf;
	tsk_get(task_property_id, &buf);
	
	if (buf.prio == LOW && buf.state == NEW && buf.priv == 0 && buf.u_stack_size == 0x250)
		print_test_result(5, RTX_OK, "Property verification test");
	else
		print_test_result(5, RTX_ERR, "Property verification test");
	tsk_exit();
}

void scheduling_policy_task_1(void)
{
	total_tests = 1;
	print_test_start(4);
    //uart1_put_string ("alloc_pointer_task: entering \n\r");
    /* do something */
    /* terminating */
	schedule_incrementer++;
	tsk_exit();
}

void scheduling_policy_task_2(void)
{
    //uart1_put_string ("alloc_pointer_task: entering \n\r");
    /* do something */
    /* terminating */
	if (schedule_incrementer == 1)
		schedule_incrementer++;
	tsk_exit();
}

void scheduling_policy_task_3(void)
{
    //uart1_put_string ("alloc_pointer_task: entering \n\r");
    /* do something */
    /* terminating */
	if (schedule_incrementer == 2)
		print_test_result(4, RTX_OK, "Scheduling policy test");
	else
		print_test_result(4, RTX_ERR, "Scheduling policy test");
	tsk_exit();
}

void alloc_pointer_task(void)
{
	total_tests = 4;
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
	tsk_exit();
		
}

void invalid_tsk_create_task(void)
{
	task_t tid;
	
	/* Try invalid prio */
	for(int i = 5; i < 20; i++)
	{
		if(tsk_create(&tid, should_never_run_task, (U8)i, 0x200) == RTX_OK)
		{
			print_test_result(3, RTX_ERR, "Invalid tsk create arg test");
			tsk_exit();
		}
	}
	
	print_test_result(3, RTX_OK, "Invalid tsk create arg test");
	tsk_exit();
}
void should_never_run_task()
{
	printf("There's something wrong cus I shouldn't have run!!\n");
	tsk_exit();
}
void invalid_tsk_set_prio_task(void)
{
	task_t tid;
	tsk_create(&tid, task13, LOW, 0x200);
	
	/* Try invalid prio */
	for(int i = 5; i < 20; i++)
	{
		if(tsk_set_prio(tid, (U8)i) == RTX_OK)
		{
			print_test_result(4, RTX_ERR, "Invalid tsk set prio arg test");
			tsk_exit();
		}
	}
	
	/* try to change null prio task */
	if(tsk_set_prio(0, 2) == RTX_OK)
		print_test_result(4, RTX_ERR, "Invalid tsk set prio arg test");
	else
		print_test_result(4, RTX_OK, "Invalid tsk set prio arg test");
		
	print_final_results();
	
	  tsk_exit();
}
