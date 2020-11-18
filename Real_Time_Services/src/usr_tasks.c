/**
 * @file:   usr_tasks.c
 * @brief:  Two user/unprivileged  tasks: task1 and task2
 * @author: Yiqing Huang
 * @date:   2020/10/20
 * NOTE: Each task is in an infinite loop. Processes never terminate.
 * IMPORTANT: This file will be replaced by another file in automated testing.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_tasks.h"
#include "printf.h"

/* #ifdef DEBUG_0
#include "printf.h"
#endif  DEBUG_0 */

/* the following arrays can also be dynamic allocated 
   They do not have to be global buffers. 
 */
U8 g_buf1[256];
U8 g_buf2[256];
task_t g_tasks[MAX_TASKS];
int passed = 0;
int total_tests = 0;
int tests_completed = 0;

/**
 * @brief: fill the tasks array with information 
 * @param: tasks, an array of RTX_TASK_INFO elements
 * @param: num_tasks, length of the tasks array 
 * NOTE: we do not use this in the starter code.
 *       during testing, if we want to initialize a user task at boot time
 *       we will write a similar function to provide task info. to kernel.
 */
 
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
 
 
int set_usr_task_info(RTX_TASK_INFO *tasks, int num_tasks) {
    for (int i = 0; i < num_tasks; i++ ) {
        tasks[i].u_stack_size = 0x0;
        tasks[i].prio = HIGH;
        tasks[i].priv = 1;
    }
    tasks[0].ptask = &task1;
    tasks[1].ptask = &task2;
    return 2;
}    

void getTimeTask(void)
{
		uart1_put_string ("getTimeTask: entering \n\r");
		struct timeval_rt *tv = mem_alloc(sizeof(tv));
		get_time(tv);
		uart1_put_char('0'+ tv->sec);
	
}

/**
 * @brief: a dummy task1
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
    mbx_create(256);
    tsk_ls(g_tasks, MAX_TASKS);
    mbx_ls(g_tasks, MAX_TASKS);
    /* terminating */
    tsk_exit();
}


void task2(void)
{
    size_t msg_hdr_size = sizeof(struct rtx_msg_hdr);
    U8  *buf = &g_buf1[0]; /* buffer is allocated by the caller */
    struct rtx_msg_hdr *ptr = (void *)buf;
    task_t sender_tid = 0;
    
    uart1_put_string ("sent_task: entering \n\r");
    
    //mbx_create(128); 
    ptr->length = msg_hdr_size + 1;
    ptr->type = KCD_REG;
    buf += msg_hdr_size;
    *buf = 'W';
	uart1_put_string ("sent_task: sending \n\r");
    send_msg(4, (void *)ptr);
    //recv_msg(&sender_tid, g_buf2, 128);
    
    /* do command processing */
    //buf = (void*) ((char*)g_buf2+8);
		//uart1_put_string ("it's bavu");
	
		//uart1_put_string(buf);
    /* Terminate if you are not a daemon task.
       For a deamon task, it should be in an infinite loop and never terminate.
    */
		uart1_put_string ("sent_task: exiting \n\r");
    tsk_exit();
}

/**
 * @brief: a dummy task2
 */
void send_task(void)
{
    size_t msg_hdr_size = sizeof(struct rtx_msg_hdr);
    U8  *buf = &g_buf1[0]; /* buffer is allocated by the caller */
    struct rtx_msg_hdr *ptr = (void *)buf;
    task_t sender_tid = 0;
    
    uart1_put_string ("sent_task: entering \n\r");
    
    //mbx_create(128); 
    ptr->length = msg_hdr_size + 2;
    ptr->type = KCD_REG;
    buf += msg_hdr_size;
    *buf = 'W';
	uart1_put_string ("sent_task: sending \n\r");
    send_msg(3, (void *)ptr);
    //recv_msg(&sender_tid, g_buf2, 128);
    
    /* do command processing */
    //buf = (void*) ((char*)g_buf2+8);
		//uart1_put_string ("it's bavu");
	
		//uart1_put_string(buf);
    /* Terminate if you are not a daemon task.
       For a deamon task, it should be in an infinite loop and never terminate.
    */
		print_test_result(4, RTX_OK, "Scheduling policy test");
		uart1_put_string ("sent_task: exiting \n\r");
		
		print_final_results();
    tsk_exit();
}

void recieve_task(void)
{
	total_tests = 1;
	print_test_start(1);
    size_t msg_hdr_size = sizeof(struct rtx_msg_hdr);
    U8  *buf = &g_buf1[0]; /* buffer is allocated by the caller */
    struct rtx_msg_hdr *ptr = (void *)buf;
    task_t sender_tid = 0;
    
    uart1_put_string ("recieve_task: entering \n\r");
    
    mbx_create(128); 
    //ptr->length = msg_hdr_size + 1;
    //ptr->type = KCD_REG;
    //buf += msg_hdr_size;
    //*buf = 'W';
    //send_msg(3, (void *)ptr);
		uart1_put_string ("recieve_task: recieving message \n\r");
    recv_msg(&sender_tid, g_buf2, 128);
    
    /* do command processing */
		ptr = (void *) g_buf2;
    buf = (void*) ((char*)g_buf2+8);
		uart1_put_string ("it's bavu\n\r");
	  printf("recieved from sender: %d\n\r",sender_tid);
		printf("length: %d\n\r",ptr->length);
		printf("type: %d\n\r",ptr->type);
		printf("recieved message: %c\n\r",*buf);
    /* Terminate if you are not a daemon task.
       For a deamon task, it should be in an infinite loop and never terminate.
    */
    tsk_exit();
}



