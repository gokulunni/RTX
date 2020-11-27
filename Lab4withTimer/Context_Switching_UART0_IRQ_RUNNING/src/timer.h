/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_
//timer0
extern uint32_t timer_init ( uint8_t n_timer );  /* initialize timer n_timer */

//timer1
#include <LPC17xx.h>

extern uint32_t timer_init_100MHZ(uint8_t n_timer);

extern void start_timer1();
extern void get_timer1();
extern void end_timer1();
#endif /* ! _TIMER_H_ */