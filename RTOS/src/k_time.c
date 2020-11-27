//
// Created by Boris Nguyen on 2020-11-23.
//

#include "k_time.h"
#include "common.h"
#include "uart_polling.h"
#ifdef DEBUG_TIME
#include "printf.h"
#endif /* ! DEBUG_TIME */
extern volatile uint32_t g_timer_count;
extern volatile uint32_t g_timer_seconds;

int k_get_time(struct timeval_rt *tv){
    if (tv == NULL) {
        #ifdef DEBUG_TIME
        printf("k_get_time: argument is null");
        #endif /* DEBUG_TIME */
        return RTX_ERR;
    }

    tv->sec = g_timer_seconds;
    tv->usec = g_timer_count;

    return RTX_OK;
}


