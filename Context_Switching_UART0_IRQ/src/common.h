/* @brief: common defines and structs for both kernel and user 
 * @file: common.h 
 * @author: Yiqing Huang
 * @date: 2020/10/13
 * IMPORTANT: DO NOT MODIFY
 */

#ifndef _COMMON_H_
#define _COMMON_H_

/* ----- Types ----- */
typedef signed char     S8;
typedef unsigned char   U8;
typedef short           S16;
typedef unsigned short  U16;
typedef int             S32;
typedef unsigned int    U32;
typedef long long       S64;
typedef unsigned long long U64;
typedef unsigned char   BIT;
typedef unsigned int    BOOL;
typedef unsigned int    size_t;
typedef signed int      ssize_t;
typedef unsigned char   task_t;

/* ----- Definitions ----- */
#define TRUE 1
#define FALSE 0
#define NULL 0
#define RTX_ERR -1
#define RTX_OK 0

/* Memory Algorithms */
#define FIXED_POOL 0
#define FIRST_FIT  1    /* only requies to implement this one */
#define BEST_FIT   2
#define WORST_FIT  3

#define PID_NULL 0         /* pre-defined Task ID for null task */
#define TID_KCD  15        /* pre-defined Task ID for KCD task */
#define TID_DISPLAY 14     /* pre-defined Task ID for DISPLAY task */
#define TID_UART_IRQ 0xFF  /* reserved TID for UART IRQ handler which is not a task */
#define TID_TIMER_IRQ 0xFE /* reserved TID for Timer IRQ handler, which is not a task */
#define MAX_TASKS 16       /* maximum number of tasks in the system */
#define KERN_STACK_SIZE 0x200 /* task kernel stack size in bytes */

/* Task Priority. The bigger the number is, the lower the priority is*/
#define HIGH    0
#define MEDIUM  1
#define LOW     2
#define LOWEST  3
#define PRIO_NULL 4    /* hidden priority for null task */

/* task state macro */
#define DORMANT        0  /* terminated task state */
#define READY          1  /* A ready torun task that has been executed */
#define RUNNING        2  /* Executing */
#define BLK_MEM        3  /* blocked on requesting memory, not used in labs 1-3 */
#define BLK_MSG        4  /* blocked on receiving a message */
#define UART_INT       5  /* Interrupted by UART IRQ Handler */
#define NEW            15 /* A ready to run task that has never been executed */

/* message passing macros */
/* message types */
#define DEFAULT 0  /* a general purpose message */
#define KCD_REG 1  /* a command registration message */
#define KCD_CMD 2  /* a message that contains a command */
#define DISPLAY 3  /* a message that contains chars to be displayed to the RTX console */
#define KEY_IN  4  /* keyboard input from console */
/* If you want to define your own messge types, 
   use a value starting from 10.
   Use a separate header file for your self-defined message types.
   Do not add them into the common.h.
   This file should not be modified.
*/

/* message data sizes */
#define MIN_MSG_SIZE 1  /* minimum message data size in bytes */

/* mailbox sizes */
#define MIN_MBX_SIZE 1  /* minimum mailbox size in bytes */

/* Structures */

/* common data structures in both kernel and user spaces */
/* Task information structure */
typedef struct rtx_task_info {
    void   (*ptask)();   /* Task entry address                      */ 
    U32    k_sp;         /* The task current kernel stack pointer   */
    U32    k_stack_hi;   /* The kernel stack starting addr. (high addr.)*/
    U32    u_sp;         /* The task current user stack pointer     */
    U32    u_stack_hi;   /* The user stack starting addr. (high addr.) */
    U16    k_stack_size; /* The task kernel total stack space in bytes */
    U16    u_stack_size; /* The task user total stack space in bytes*/
    task_t tid;          /* Task ID                                 */
    U8     prio;         /* Execution priority                      */
    U8     state;        /* Task state                              */
    U8     priv;         /* = 0 unprivileged, =1 priviliged         */  
} RTX_TASK_INFO;

/* message buffer header struct */
typedef struct rtx_msg_hdr {
    U32 length;   /* length of the mssage including the message header size */
    U32 type;     /* type of the message */
} RTX_MSG_HDR;

#endif // _COMMON_H_
