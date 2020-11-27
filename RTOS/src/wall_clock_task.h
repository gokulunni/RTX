#ifndef WALL_CLOCK_TASK_H_
#define WALL_CLOCK_TASK_H_

#include "common.h"

struct time_t 
{
    U8 sec;
    U8 min;
    U8 hr;
};

void send_time();

#endif /* WALL_CLOCK_TASK_H */
