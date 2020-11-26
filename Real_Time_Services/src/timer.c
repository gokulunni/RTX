/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "timer.h"
#include "wall_clock_task.h"
#include "k_rtx.h"

#define BIT(X) (1<<X)

volatile uint32_t g_timer_count = 0; /* increment every 1 us */
volatile uint32_t seconds = 0;
extern TCB *gp_current_task;

volatile U32 g_timer_count_wall = 0;
extern struct time_t time;
extern int wall_clock_enabled;

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

    pTimer->PR = 4999;

    /* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
    pTimer->MR0 = 1; //Setting Match register 0 to 1, so interrupt occurs when counter == MR0 == 1us

    /* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
       Interrupt on MR0: when MR0 mathches the value in the TC, 
                         generate an interrupt.
       Reset on MR0: Reset TC if MR0 mathches it.
    */
    pTimer->MCR = BIT(0) | BIT(1);

    g_timer_count = 0;
    seconds = 0;
		

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
    LDR R3, =__cpp(&gp_current_task)    ; Load R3 with address of pointer to current task
    LDR R3, [R3]                        ; Get address of current task
    MOV R2, #0                          ; clear R2
    LDRB R2, [R3, #45]                  ; read priv member (45 byte offset)
    CMP R2, #1                          ; check if priv level is 1 or 0
    BEQ kernel_thread                   ; if 1, handler was invoked by kernel thread
    B user_thread                       ; if 0, handler was invoked by user thread

kernel_thread
    MVN  LR, #:NOT:0xFFFFFFF9  ; set EXC_RETURN value to Thread mode, MSP
    MOV R3, #0                 ; 
    MSR CONTROL, R3            ; set control bit[0] to 0 (privileged)
    CPSIE I                    ; enable interrupt
    BX   LR

user_thread
    MVN  LR, #:NOT:0xFFFFFFFD  ; set EXC_RETURN value to Thread mode, PSP  
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
    LPC_TIM0->IR = BIT(0);  //Writing 1 to MR0 interrupt resets the interrupt
    
    g_timer_count++ ;

    if (g_timer_count==10000){
        seconds++;
        g_timer_count=0;
    }

    if(wall_clock_enabled)
    {
        g_timer_count_wall++;

        //Update wall clock time value
        if(g_timer_count_wall == 1000000)
        {
            (time.sec)++;
            if(time.sec == 60)
            {
                time.sec = 0;
                time.min++;

                if(time.min == 60)
                {
                    time.min = 0;
                    time.hr++;
                    if(time.hr == 24)
                    {
                        time.hr = 0;
                    }
                }
            }
            send_time(); //send to LCD for display
        }
    }
}


//timer 1

/* a free running counter set up, no interrupt fired */

uint32_t timer_init_100MHZ(uint8_t n_timer)
{
    LPC_TIM_TypeDef *pTimer;
    if (n_timer == 1) {
        pTimer = (LPC_TIM_TypeDef *) LPC_TIM1;
    } else { /* other timer not supported yet */
        return 1;
    }


    /* Step 1. Prescale Register PR setting
       CCLK = 100 MHZ, PCLK = CCLK
       Prescalar counter increments every PCLK tick: (1/100)* 10^(-6) s = 10 ns
       TC increments every (PR + 1) PCLK cycles
       TC increments every (MR0 + 1) * (PR + 1) PCLK cycles
    */
    pTimer->PR = 4000000000 - 1; /* increment timer counter every 4*10^9 PCLK ticks, which is 40 sec */

    /* Step 2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
    /* Effectively, using timer1 as a counter to measure time, there is no overflow in TC in 4K-5K years */
    pTimer->MR0 = 0xFFFFFFFF - 1;

    /* Step 3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
       Reset on MR0: Reset TC if MR0 mathches it. No interrupt triggered on match.
    */
    pTimer->MCR = BIT(1);

    /* Step 4: Enable the counter. See table 427 on pg494 of LPC17xx_UM. */
    pTimer->TCR = 1;

    return 0;
}

void start_timer1(){
	LPC_TIM_TypeDef *pTimer = LPC_TIM1;
	pTimer->TCR = 2;  // disable counter, reset counters
  pTimer->TCR = 1;  //enable counter

	//pTimer->TCR = 0;
	//int e_tc = pTimer->TC;
  //int e_pc = pTimer->PC;
}

int get_timer1(){
	LPC_TIM_TypeDef *pTimer = LPC_TIM1;
	pTimer->TCR = 0; //disable counter
  int e_tc = pTimer->TC;
  int e_pc = pTimer->PC;

}

int end_timer1(){
	LPC_TIM_TypeDef *pTimer = LPC_TIM1;
	pTimer->TCR = 0;
	int e_tc = pTimer->TC;
  int e_pc = pTimer->PC;
	g_timer_count+=e_pc/1000000;
	seconds+= g_timer_count/100;
	g_timer_count= g_timer_count%100;
	
}

