/**
 * @file:   usr_proc.h
 * @brief:  Two user processes header file
 * @author: Yiqing Huang
 * @date:   2020/08/07
 */
 
#ifndef USR_PROC_H_
#define USR_PROC_H_

#ifdef SIM_TARGET       /* using the simulator is slow */
#define DELAY 500000
#else
#define DELAY 50000000
#endif /* SIM_TARGET */

/* Utility Functions */
void set_test_procs(void);
void print_test_result(int test_num, int result, char *test_name);
void print_test_start(int suite_num);
void print_final_results();

/******************* SUITE 1 START *************************/
/* Max tasks test tasks */
void task1(void);
void task2(void);
void task3(void);
void task4(void);
void task5(void);
void task6(void);
void task7(void);
void task8(void);
void task9(void);
void task10(void);
void task11(void);
void task12(void);
void task13(void);
void task14(void);
void task15(void);
/******************* SUITE 1 END *************************/


/******************* SUITE 2 START *************************/
/* Memory ownership test tasks */
void alloc_pointer_task(void);
void dealloc_pointer_task(void);
/******************* SUITE 2 END *************************/


#endif /* USR_PROC_H_ */
