//
// Created by Boris Nguyen on 2020-11-23.
//

#include "timeval.h"
#include "common.h"

void add(struct timeval_rt* dest, struct timeval_rt time1, struct timeval_rt time2) {
    dest->usec = (time1.usec + time2.usec) % 1000000;
    U32 carry = (time1.usec + time2.usec) / 1000000;
    dest->sec = carry + time1.sec + time2.sec;
}

void sub(struct timeval_rt* dest, struct timeval_rt time1, struct timeval_rt time2) {
    if (is_less_equal(time1, time2)) {
        dest->usec = 0;
        dest->sec = 0;
    } else {
        if (time1.usec < time2.usec) {
            time1.sec--;
            time1.usec += 1000000;
        }
        dest->usec = (time1.usec - time2.usec);
        dest->sec = (time1.sec - time2.sec);
    }
}

int is_equal(struct timeval_rt time1, struct timeval_rt time2) {
    if (time1.sec == time2.sec && time1.usec == time2.usec) {
        return TRUE;
    }
    return FALSE;
}

int is_greater(struct timeval_rt time1, struct timeval_rt time2) {
    if (time1.sec > time2.sec || (time1.sec == time2.sec && time1.usec > time2.usec)) {
        return TRUE;
    }
    return FALSE;
}

int is_less(struct timeval_rt time1, struct timeval_rt time2) {
    if (time1.sec < time2.sec || (time1.sec == time2.sec && time1.usec < time2.usec)) {
        return TRUE;
    }
    return FALSE;
}

int is_greater_equal(struct timeval_rt time1, struct timeval_rt time2) {
    if (time1.sec >= time2.sec || (time1.sec == time2.sec && time1.usec >= time2.usec)) {
        return TRUE;
    }
    return FALSE;
}

int is_less_equal(struct timeval_rt time1, struct timeval_rt time2) {
    if (time1.sec <= time2.sec || (time1.sec == time2.sec && time1.usec <= time2.usec)) {
        return TRUE;
    }
    return FALSE;
}