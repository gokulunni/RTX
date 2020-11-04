/* The LCD Display Task Template File */

#include "rtx.h"
#include "uart_irq.h"
#include "k_mem.h"
#include <LPC17xx.h>

U8 UART_buffer[128];

void lcd_task(void)
{
    mbx_create(128); //TODO: Determine whether this is an appropriate size
    task_t sender_tid;

    while(1)
    {
		U8 temp_buffer[sizeof(RTX_MSG_HDR) + 32]; //TODO: is 32 bytes for msg big enough?
        //copy contents to internal kernel buffer so UART can transmit
        if(recv_msg(&sender_tid, &temp_buffer , 64) == 0)
        {
            
            if((U32)temp_buffer[4] == DISPLAY)
            {
                //copy contents of buffer to internal kernel buffer for UART
                mem_cpy(UART_buffer, &temp_buffer, (U32)temp_buffer);

                //Enable UART Transmit interrupt
                LPC_UART_TypeDef * pUart = (LPC_UART_TypeDef *) LPC_UART0;
                pUart->IER ^= IER_THRE; //toggling THR
    
            }            
        }
    }
}
