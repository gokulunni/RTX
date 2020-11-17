/**
 * @file:   k_time.c
 * @brief:  timing routines
 * @author: Yiqing Huang
 * @date:   2020/09/28
 */

#include "common.h"
#include "uart_polling.h"
#ifdef DEBUG_MEM
#include "printf.h"
#endif /* ! DEBUG_MEM */



int get_time(struct timeval_rt *tv);


int k_get_time(struct timeval_rt *tv){
    U32 end_addr;

    #ifdef DEBUG_MEM
    printf("******************************************************\r\n");
    #endif /* DEBUG_MEM */

    if (tv == NULL) {
        #ifdef DEBUG_MEM
        printf("k_get_time: argument is null");
        #endif /* DEBUG_MEM */
        return RTX_ERR;
    }
    
    return get_time(tv);
}



/*
*  get time function
*/


int get_time(struct timeval_rt *tv) {

    return tv;
}
