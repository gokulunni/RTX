/* @brief: HAL.c Hardware Abstraction Layer
 * @author: Yiqing Huang
 * @date: 2020/09/20
 * NOTE: This file contains embedded assembly. 
 *       The code borrowed some ideas from ARM RL-RTX source code
 */
 
 #include "k_rtx.h"
 //#include "k_task.h"
 
extern TCB *gp_current_task;
 
/* pop off exception stack frame from the stack */
__asm void __rte(void)
{
  PRESERVE8                  ; 8 bytes alignement of the stack
  MVN  LR, #:NOT:0xFFFFFFFD  ; set EXC_RETURN value, Thread mode, PSP
  CPSIE I                    ; enable interrupt
  BX   LR
}

/* Overview 
  1. Get SVC number and verify if 0
  2. Load R0-R3, R12 from exception stack frame to call desired kernel mode function. Save gp-regs
  3. restore gp-regs, store return value in RO (which is on exception stack frame) 
  4. Return appropriate EXC_RETURN val to set processor mode, depending on current running task
*/

/* NOTE: We're assuming the exception stack frame is pushed onto PSP for user and kernel threads*/
__asm void SVC_Handler (void) 
{
  PRESERVE8             ; 8 bytes alignement of the stack
  CPSID I               ; disable interrupt
	
  MRS  R0, PSP          ; Read PSP into R0
	CMP  R0, #0 				  ; Check if this is the first invocation
	BNE  normal_operation 
	MRS  R0, MSP          ;since PSP = 0x0, load MSP address
												
	
normal_operation
  MSR MSP, R0          ; Set MSP = PSP
	
  
  LDR  R1, [R0, #24]   ; Read Saved PC from SP (skip over 6 regs - R0-R3, R12, LR)
                       ; Loads R1 from a word 24 bytes above the address in R0 (the MSP)
                       ; Note that R0 now contains the the SP value after the
                       ; exception stack frame is pushed onto the stack.
             
  LDRH R1, [R1, #-2]   ; Load halfword because SVC number is encoded there
  BICS R1, R1, #0xFF00 ; Extract SVC Number and save it in R1.  
                       ; R1 <= R1 & ~(0xFF00), update flags
                       ; SVC number is 2 bytes above PC on stack
                   
  BNE  SVC_EXIT        ; if SVC Number !=0, exit
 
  LDM  R0, {R0-R3, R12}; Read R0-R3, R12 from stack (no writeback)
                       ; NOTE R0 contains the sp before this instruction

  PUSH {R4-R11, LR}    ; Save other registers
  BLX  R12             ; Call SVC C Function, 
                       ; R12 contains the corresponding 
                       ; C kernel functions entry point
                       ; R0-R3 contains the kernel function input parameters (See AAPCS)
  POP {R4-R11, LR}     ; Restore other registers
  MRS  R12, MSP        ; Read MSP
  STR  R0, [R12]       ; store C kernel function return value in R0
                       ; to R0 on the exception stack frame  
SVC_EXIT  

  LDR R3, =__cpp(&gp_current_task)    ; Load R3 with priv level of current task
	STR R2, [R3, #120]                      ; 
  CMP R2, #1                                 ; check if priv level is 1 or 0
  BEQ kernel_thread                          ; if 1, handler was invoked by kernel thread
  B user_thread                            ; if 0, handler was invoked by user thread

kernel_thread
  MVN  LR, #:NOT:0xFFFFFFFD  ; set EXC_RETURN value to Thread mode, PSP
	MOV R3, #0                 ; 
	MSR CONTROL, R3            ; set control bit[0] to 0 (priviledged)
  CPSIE I                    ; enable interrupt
  BX   LR

user_thread
  MVN  LR, #:NOT:0xFFFFFFFD  ; set EXC_RETURN value to Thread mode, PSP
	MOV R3, #1                 ; 
	MSR CONTROL, R3            ; set control bit[0] to 1 (unpriviledged)
  CPSIE I                    ; enable interrupt
  BX   LR
}