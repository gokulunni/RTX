/* The LCD Display Task Template File */

#include "rtx.h"
#include "uart_irq.h"
#include "k_mem.h"
#include <LPC17xx.h>

char *TX_buffer;
extern TCB kernal_task;
extern TCB *gp_current_task;

void lcd_task(void)
{
    mbx_create(128); //TODO: Determine whether this is an appropriate size
    task_t sender_tid;
    
    /* Allocate internal kernel buffer for use between uart and LCD */
    TCB *lcdtask= gp_current_task;
    gp_current_task = &kernal_task;
    TX_buffer = (char *)mem_alloc(64); //TODO: Might not need this much space
    gp_current_task = lcdtask;

    while(1)
    {
		U8 temp_buffer[40]; //TODO: is 32 bytes for msg big enough?
        //copy contents to internal kernel buffer so UART can transmit
        if(recv_msg(&sender_tid, &temp_buffer , 64) == 0)
        {
            
            if((U32)temp_buffer[4] == DISPLAY)
            {
                LPC_UART_TypeDef * pUart = (LPC_UART_TypeDef *) LPC_UART0;
                U32 length = (((RTX_MSG_HDR *)temp_buffer) -> length) - 8;

                pUart->THR = temp_buffer[8]; //prime the THR buffer with first char in msg
                //copy remaining contents of buffer to internal kernel buffer for UART
                mem_cpy(TX_buffer, temp_buffer + 9, length - 1);

                //Enable UART Transmit interrupt
				pUart->IER = IER_THRE | IER_RLS | IER_RBR;
            }            
        }
    }
}
