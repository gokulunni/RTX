//
// Created by Boris Nguyen on 2020-11-23.
//

#ifndef ECE350_TIMER_H
#define ECE350_TIMER_H

#include <LPC17xx.h>

extern uint32_t timer_init ( uint8_t n_timer );  /* initialize timer n_timer */

extern uint32_t timer_init_100MHZ(uint8_t n_timer);
void start_timer1();
int get_timer1();
int end_timer1();

#endif //ECE350_TIMER_H
