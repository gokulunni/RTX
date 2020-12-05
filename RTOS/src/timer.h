//
// Created by Boris Nguyen on 2020-11-23.
//

#ifndef ECE350_TIMER_H
#define ECE350_TIMER_H

#include <LPC17xx.h>

extern uint32_t timer_init (uint8_t n_timer);  /* initialize timer n_timer */
extern uint32_t timer_init_100MHZ(uint8_t n_timer);

extern void start_timer1();
extern void get_timer1();
extern void end_timer1();

extern RTX_SYS_INFO kernel_sys_info;

#endif //ECE350_TIMER_H
