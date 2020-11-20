//
// Created by Boris Nguyen on 2020-11-19.
//

#ifndef ECE350_TIMEVAL_H
#define ECE350_TIMEVAL_H

#include "common.h"

void add(timeval_rt dest, timeval_rt time1, timeval_rt time2);
void sub(timeval_rt dest, timeval_rt time1, timeval_rt time2);
int is_equal(timeval_rt time1, timeval_rt time2);
int is_greater(timeval_rt time1, timeval_rt time2);
int is_less(timeval_rt time1, timeval_rt time2);
int is_greater_equal(timeval_rt time1, timeval_rt time2);
int is_less_equal(timeval_rt time1, timeval_rt time2);

#endif //ECE350_TIMEVAL_H
