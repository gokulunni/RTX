/**
 * @brief: main routine for echoing user input through UART0 by interrupt.
 *         uart1 is by polling. It is used as a debugging terminal.
 * @file:  main_irq.c
 * @author: Yiqing Huang
 * @date: 2020/10/04
 */

#include <LPC17xx.h>
#include "uart_irq.h"
#include "uart_polling.h"
#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

extern uint8_t g_send_char;

int main()
{
	LPC_UART_TypeDef *pUart;
	
	SystemInit();	
	__disable_irq();
	
	uart_irq_init(0); /* uart0 interrupt driven, for RTX console */
	uart_init(1);     /* uart1 polling, for debugging */
	
#ifdef DEBUG_0
	init_printf(NULL, putc);
#endif // DEBUG_0
	__enable_irq();

	uart1_put_string("COM1> Type a character at COM0 terminal\n\r");

	pUart = (LPC_UART_TypeDef *) LPC_UART0;
	
	while( 1 ) {
		
		if (g_send_char == 0) {
			/* Enable RBR, THRE is disabled */
			pUart->IER = IER_RLS | IER_RBR;
		} else if (g_send_char == 1) {
			/* Enable THRE, RBR left as enabled */
			pUart->IER = IER_THRE | IER_RLS | IER_RBR;
		}
     
	}
}

