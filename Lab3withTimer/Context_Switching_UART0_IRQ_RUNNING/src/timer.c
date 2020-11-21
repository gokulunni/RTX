/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "timer.h"
#include "k_rtx.h"

#define BIT(X) (1<<X)

volatile uint32_t g_timer_count = 0; /* increment every 1 us */
uint32_t seconds;
extern TCB *gp_current_task;
/**
 * @brief: initialize timer. Only timer 0 is supported
 */
uint32_t timer_init(uint8_t n_timer) 
{
    LPC_TIM_TypeDef *pTimer;
    if (n_timer == 0) {
        /*
        Steps 1 & 2: system control configuration.
        Under CMSIS, system_LPC17xx.c does these two steps
        
        ----------------------------------------------------- 
        Step 1: Power control configuration.
                See table 46 pg63 in LPC17xx_UM
        -----------------------------------------------------
        Enable TIMER0 power, this is the default setting
        done in system_LPC17xx.c under CMSIS.
        Enclose the code for your refrence
        //LPC_SC->PCONP |= BIT(1);
    
        -----------------------------------------------------
        Step2: Select the clock source, 
               default PCLK=CCLK/4 , where CCLK = 100MHZ.
               See tables 40 & 42 on pg56-57 in LPC17xx_UM.
        -----------------------------------------------------
        Check the PLL0 configuration to see how XTAL=12.0MHZ 
        gets to CCLK=100MHZ in system_LPC17xx.c file.
        PCLK = CCLK,  setting in system_LPC17xx.c.    

        -----------------------------------------------------
        Step 3: Pin Ctrl Block configuration. 
                Optional, not used in this example
                See Table 82 on pg110 in LPC17xx_UM 
        -----------------------------------------------------
        */
        pTimer = (LPC_TIM_TypeDef *) LPC_TIM0;
    } else { /* other timer not supported yet */
        return 1;
    }

    /*
    -----------------------------------------------------
    Step 4: Interrupts configuration
    -----------------------------------------------------
    */

    /* Step 4.1: Prescale Register PR setting 
       CCLK = 100 MHZ, PCLK = CCLK = 100 MHZ
       2*(49 + 1)* 1/100 * 10^(-6) s = 10^(-6) s = 1 usec
       TC (Timer Counter) toggles b/w 0 and 1 every 50 PCLKs
       see MR setting below 
    */
    pTimer->PR = 49;  

    /* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
    pTimer->MR0 = 1;

    /* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
       Interrupt on MR0: when MR0 mathches the value in the TC, 
                         generate an interrupt.
       Reset on MR0: Reset TC if MR0 mathches it.
    */
    pTimer->MCR = BIT(0) | BIT(1);

    g_timer_count = 0;

    /* Step 4.4: CSMSIS enable timer0 IRQ */
    NVIC_EnableIRQ(TIMER0_IRQn);

    /* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
    pTimer->TCR = 1;

    return 0;
}

/**
 * @brief: use CMSIS ISR for TIMER0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_TIMER0_IRQHandler does the rest of irq handling
 */
__asm void TIMER0_IRQHandler(void)
{
    PRESERVE8
    IMPORT c_TIMER0_IRQHandler
    CPSID I ; disable interrupts
    PUSH{r4-r11, lr}
    BL c_TIMER0_IRQHandler
		
	POP{r4-r11, LR}
	MVN  LR, #:NOT:0xFFFFFFFD           ; set EXC_RETURN value to Thread mode, PSP
  LDR R3, =__cpp(&gp_current_task)    ; Load R3 with address of pointer to current task
	LDR R3, [R3]                        ; Get address of current task
	MOV R2, #0                          ; clear R2
	LDRB R2, [R3, #45]                  ; read priv member (45 byte offset)
  CMP R2, #1                          ; check if priv level is 1 or 0
  BEQ kernel_thread                   ; if 1, handler was invoked by kernel thread
  B user_thread                       ; if 0, handler was invoked by user thread

kernel_thread
	MOV R3, #0                 ; 
	MSR CONTROL, R3            ; set control bit[0] to 0 (privileged)
  CPSIE I                    ; enable interrupt
  BX   LR

user_thread
	MOV R3, #1                 ; 
	MSR CONTROL, R3            ; set control bit[0] to 1 (unpriviledged)
  CPSIE I                    ; enable interrupt
  BX   LR
} 
/**
 * @brief: c TIMER0 IRQ Handler
 */
void c_TIMER0_IRQHandler(void)
{
    /* ack interrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
    LPC_TIM0->IR = BIT(0);  
    
    g_timer_count++ ;
}

