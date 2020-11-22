/* @brief: HAL.c Hardware Abstraction Layer
 * @author: Yiqing Huang
 * @date: 2020/09/20
 * NOTE: This file contains embedded assembly. 
 *       The code borrowed some ideas from ARM RL-RTX source code
 */

#include "k_rtx.h"

extern TCB *gp_current_task;

/* pop off exception stack frame from the stack */
__asm void __rte(void)
{
  PRESERVE8            ; 8 bytes alignement of the stack
  B SVC_EXIT           ; Exit SVC_Handler
}

/* NOTE: assuming MSP is used. Ideally, PSP should be used */
__asm void SVC_Handler (void) 
{
  PRESERVE8            ; 8 bytes alignement of the stack
  CPSID I              ; disable interrupt
  CMP LR, 0xFFFFFFF9   ; Check LR value to see if MSP or PSP was used
  BEQ SET_MSP_SP       ; if 1, handler was invoked by kernel thread
  B SET_PSP_SP         ; if 0, handler was invoked by user thread

SET_MSP_SP
  MRS  R0, MSP         ; Read MSP
  B NORMAL_OPERATION

SET_PSP_SP
  MRS  R0, PSP         ; Read PSP
  B NORMAL_OPERATION

NORMAL_OPERATION
  LDR  R1, [R0, #24]   ; Read Saved PC from SP
                       ; Loads R1 from a word 24 bytes above the address in R0
                       ; Note that R0 now contains the the SP value after the
                       ; exception stack frame is pushed onto the stack.
             
  LDRH R1, [R1, #-2]   ; Load halfword because SVC number is encoded there
  BICS R1, R1, #0xFF00 ; Extract SVC Number and save it in R1.  
                       ; R1 <= R1 & ~(0xFF00), update flags
                   
  BNE  SVC_EXIT        ; if SVC Number !=0, exit
 
  LDM  R0, {R0-R3, R12}; Read R0-R3, R12 from stack. 
                       ; NOTE R0 contains the sp before this instruction

  PUSH {R4-R11, LR}    ; Save other registers
  BLX  R12             ; Call SVC C Function, 
                       ; R12 contains the corresponding 
                       ; C kernel functions entry point
                       ; R0-R3 contains the kernel function input parameter (See AAPCS)
  POP {R4-R11, LR}     ; Restore other registers
  MRS  R12, MSP        ; Read MSP
  STR  R0, [R12]       ; store C kernel function return value in R0
                       ; to R0 on the exception stack frame  

SVC_EXIT
  LDR R3, =__cpp(&gp_current_task)    ; Load R3 with address of pointer to current task
  LDR R3, [R3]                        ; Get address of current task
  MOV R2, #0                          ; clear R2
  LDRB R2, [R3, #45]                  ; read priv member (45 byte offset)
  CMP R2, #1                          ; check if priv level is 1 or 0
  BEQ kernel_thread                   ; if 1, handler was invoked by kernel thread
  B user_thread                       ; if 0, handler was invoked by user thread

kernel_thread
  MVN LR, #:NOT:0xFFFFFFF9   ; set EXC_RETURN value to Thread mode, MSP
  MOV R3, #0                 ;
  MSR CONTROL, R3            ; set control bit[0] to 0 (privileged)
  CPSIE I                    ; enable interrupt
  BX   LR

user_thread
  MVN LR, #:NOT:0xFFFFFFFD   ; set EXC_RETURN value to Thread mode, PSP
  MOV R3, #1                 ;
  MSR CONTROL, R3            ; set control bit[0] to 1 (unpriviledged)
  CPSIE I                    ; enable interrupt
  BX   LR
}
