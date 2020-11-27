/* The LCD Display Task Template File */

#include "rtx.h"
#include "common.h"
#include "uart_irq.h"
#include "k_mem.h"
#include <LPC17xx.h>
#include "helpers.h"

char *TX_buffer;
extern uint8_t buffer_index;

void lcd_task(void)
{
    int ret_val = 0;
    task_t sender_tid;

    mbx_create(128);

    TX_buffer = (char *) mem_alloc(64);

    while(1) {
        U8 temp_buffer[40];
        ret_val = recv_msg(&sender_tid, &temp_buffer , 40);
        if(ret_val == RTX_OK) {
            if((U32)temp_buffer[4] == DISPLAY) {
                LPC_UART_TypeDef * pUart = (LPC_UART_TypeDef *) LPC_UART0;
                U32 length = (((RTX_MSG_HDR *)temp_buffer) -> length) - 8;

                while(buffer_index != 0){}; //wait until prev info is written
                pUart->THR = temp_buffer[8]; //prime the THR buffer with first char in msg
                //copy remaining contents of buffer to internal kernel buffer for UART
                mem_cpy(TX_buffer, temp_buffer + 9, length - 1);

                //Enable UART Transmit interrupt
                pUart->IER = IER_THRE | IER_RLS | IER_RBR;
            }
        }
    }
}
