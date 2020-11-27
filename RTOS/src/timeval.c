//
// Created by Boris Nguyen on 2020-11-23.
//

#include "timeval.h"
#include "common.h"

void add(struct timeval_rt *dest, struct timeval_rt time1, struct timeval_rt time2) {
    struct timeval_rt temp = (struct timeval_rt) {0, 0};
    temp.usec = (time1.usec + time2.usec) % 1000000;
    U32 carry = (time1.usec + time2.usec) / 1000000;
    temp.sec = carry + time1.sec + time2.sec;
    dest->usec = temp.usec;
    dest->sec = temp.sec/
}

void sub(struct timeval_rt* dest, struct timeval_rt time1, struct timeval_rt time2) {
    if (is_less_equal(time1, time2)) {
        dest->usec = 0;
        dest->sec = 0;
    } else {
        struct timeval_rt temp = (struct timeval_rt) {temp1.sec, temp1.usec};
        if (time1.usec < time2.usec) {
            temp.sec--;
            temp.usec += 1000000;
        }
        dest->usec = (temp.usec - time2.usec);
        dest->sec = (temp.sec - time2.sec);
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