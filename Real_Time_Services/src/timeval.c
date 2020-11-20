#include "timeval.h"

void add(struct timeval_rt* dest, struct timeval_rt time1, struct timeval_rt time2)
{
    //TO DO: overflow check?
    dest->usec = (time1.usec + time2.usec) % 1000000;
    U32 carry = (time1.usec + time2.usec) / 1000000;
    dest->sec = carry + time1.sec + time2.sec;
}

void sub(struct timeval_rt* dest, struct timeval_rt time1, struct timeval_rt time2)
{
    if (is_less_equal(time1, time2)) {
        dest->usec = 0;
        dest->sec = 0;
    }
    else
    {
        if (time1.usec < time2.usec) {
        time1.sec--;
        time1.usec += 1000000;
        }
        dest->usec = (time1.usec - time2.usec);
        dest->sec = (time1.sec - time2.sec);
    }
}

int is_equal(struct timeval_rt time1, struct timeval_rt time2)
{
    if (time1.sec == time2.sec && time1.usec == time2.usec) {
        return 1;
    }
    return 0;
}

int is_greater(struct timeval_rt time1, struct timeval_rt time2)
{
    if (time1.sec > time2.sec   ||  (time1.sec == time2.sec && time1.usec > time2.usec)) {
        return 1;
    }
    return 0;
}

int is_less(struct timeval_rt time1, struct timeval_rt time2)
{
    if (time1.sec < time2.sec   ||  (time1.sec == time2.sec && time1.usec < time2.usec)) {
        return 1;
    }
    return 0;
}

int is_greater_equal(struct timeval_rt time1, struct timeval_rt time2)
{
    if (time1.sec >= time2.sec   ||  (time1.sec == time2.sec && time1.usec >= time2.usec)) {
        return 1;
    }
    return 0;
}

int is_less_equal(struct timeval_rt time1, struct timeval_rt time2)
{
    if (time1.sec <= time2.sec   ||  (time1.sec == time2.sec && time1.usec <= time2.usec)) {
        return 1;
    }
    return 0;
}