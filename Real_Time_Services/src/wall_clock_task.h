#ifndef WALL_CLOCK_TASK_H_
#define WALL_CLOCK_TASK_H_

#include "common.h";

struct time_t 
{
    U32 sec;
    U32 min;
    U32 hr;
};

void send_time();

#endif /* WALL_CLOCK_TASK_H */