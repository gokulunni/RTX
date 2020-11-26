/**
 * @file:   k_time.c
 * @brief:  timing routines
 * @author: Yiqing Huang
 * @date:   2020/09/28
 */

#include "k_time.h"
#include "common.h"
#include "uart_polling.h"
#ifdef DEBUG_TIME
#include "printf.h"
#endif /* ! DEBUG_TIME */
extern volatile uint32_t g_timer_count;
extern volatile uint32_t seconds;


int get_time(struct timeval_rt *tv);


int k_get_time(struct timeval_rt *tv){
    U32 end_addr;

    #ifdef DEBUG_TIME
    printf("******************************************************\r\n");
    #endif /* DEBUG_TIME */

    if (tv == NULL) {
        #ifdef DEBUG_TIME
        printf("k_get_time: argument is null");
        #endif /* DEBUG_TIME */
        return RTX_ERR;
    }
		
		tv->sec=seconds;
		tv->usec=g_timer_count;
    
    return RTX_OK;
}


