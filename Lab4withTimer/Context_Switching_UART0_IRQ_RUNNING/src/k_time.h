/**
 * @file:   k_time.h
 * @brief:  kernel timer managment header file
 * @author: Yiqing Huang
 * @date:   2020/09/20
 */
 
#ifndef K_TIME_H_
#define K_TIME_H_

#include "k_rtx.h"
/* ----- Functions ------ */
int k_get_time(struct timeval_rt *tv);

#endif /* ! K_TIME_H_ */
