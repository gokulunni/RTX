/**
 * @file:   k_mem.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2020/09/28
 */

#include "k_mem.h"
#include "common.h"
#include "uart_polling.h"
#ifdef DEBUG_MEM
#include "printf.h"
#endif /* ! DEBUG_MEM */
#define CEIL(x,y) (((x)+(y)-1)/(y))

/* The final memory map of IRAM1 looks like the following. 
   Refer to k_task.c for initilization function details.
---------------------------------------------------------------------------

                    0x10008000+---------------------------+ High Address
                              |                           |
                              |  Free Memory Space        |
                              |                           |
                              |                           |
                              |                           |
                              |---------------------------|
                              |        Padding            |
&Image$$RW_IRAM1$$ZI$$Limit-->|---------------------------|
                              |                           |          
                              |       RTX  Image          |
                              |                           |
                    0x10000000+---------------------------+ Low Address

*/


int get_time(struct timeval_rt *tv);


extern unsigned int Image$$RW_IRAM1$$ZI$$Limit;
extern TCB *gp_current_task;

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

    //end_addr = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;
    
    return get_time(tv);
}



/*
*  First Fit Memory Allocation
*/


int get_time(struct timeval_rt *tv) {

    return tv;
}
