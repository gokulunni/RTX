/** 
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2020/09/21
 */
 
#ifndef K_RTX_H_
#define K_RTX_H_

#include "common.h"
#include "circular_buffer.h"
/*----- Definitations -----*/
#define NUM_TEST_PROCS 2

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 256B  */
#endif /* DEBUG_0 */

/*----- Types -----*/


/*
  TCB data structure definition to support two kernel tasks.
  You will need to add more fields to this structure.
*/

typedef struct tcb
{
    struct tcb *next; /* next tcb, not used in this example */
    U32 *msp;    /* msp of the task */
    U32 *msp_hi; /* The msp stack starting addr. (high addr.)*/
    U32 *psp;   /* psp of the task */
    U32 *psp_hi; /* The psp stack starting addr. (high addr.)*/
    CIRCULAR_BUFFER_T mailbox;
    void *msg_sender_head;
    U16 psp_size;
    U8  tid;     /* task id */
    U8  prio;    /* Execution priority */
    U8  state;   /* task state */  
    U8  priv;    /* = 0 unprivileged, =1 priviliged */
    U8  has_mailbox; /* 0 = no mailbox. 1 = has mailbox */
} TCB;

#endif // ! K_RTX_H_
