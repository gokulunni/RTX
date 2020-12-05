//
// Created by Boris Nguyen on 2020-11-23.
//

#ifndef ECE350_K_TIME_H
#define ECE350_K_TIME_H

#include "k_rtx.h"
/* ----- Functions ------ */
int k_get_time(struct timeval_rt *tv);

extern RTX_SYS_INFO kernel_sys_info;

#endif //ECE350_K_TIME_H
