//
// Created by Boris Nguyen on 2020-11-23.
//

#ifndef ECE350_TIMEVAL_H
#define ECE350_TIMEVAL_H

#include "common.h"

void add(struct timeval_rt* dest, struct timeval_rt time1, struct timeval_rt time2);
void sub(struct timeval_rt* dest,struct timeval_rt time1, struct timeval_rt time2);
int is_equal(struct timeval_rt time1, struct timeval_rt time2);
int is_greater(struct timeval_rt time1, struct timeval_rt time2);
int is_less(struct timeval_rt time1, struct timeval_rt time2);
int is_greater_equal(struct timeval_rt time1, struct timeval_rt time2);
int is_less_equal(struct timeval_rt time1, struct timeval_rt time2);

#endif //ECE350_TIMEVAL_H
