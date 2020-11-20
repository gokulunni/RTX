#include <LPC17xx.h>
#include "common.h"
#include "rtx.h"
#include "wall_clock_task.h"
#include "k_mem.h"

struct time_t time; //Wall clock time
extern volatile U32 g_timer_count_wall;
int wall_clock_enabled = 0;

void send_time()
{
    size_t msg_hdr_size = sizeof(struct rtx_msg_hdr);
    U8 buf[19]; //msg_hdr (8) display time length (8) + new line + carriage return + null termination
    struct rtx_msg_hdr *header = (void *)buf;
    header -> length = msg_hdr_size + 8 + 3;
    header -> type = DISPLAY;

    //HH:
    buf[msg_hdr_size] = time.hr/10;
    buf[msg_hdr_size + 1] = time.hr%10;
    buf[msg_hdr_size + 2] = ':';

    //MM:
    buf[msg_hdr_size + 3] = time.min/10;
    buf[msg_hdr_size + 4] = time.min%10;
    buf[msg_hdr_size + 5] = ':';
    
    //SS
    buf[msg_hdr_size + 6] = time.sec/10;
    buf[msg_hdr_size + 7] = time.sec%10;
    buf[msg_hdr_size + 8] = '\n';
    buf[msg_hdr_size + 9] = '\r';
    buf[msg_hdr_size + 10] = '\0';

    k_send_msg(TID_DISPLAY, buf);
}

void wall_clock_task(void)
{
    U8 *reg_buf = mem_alloc(8 + 2);
    struct rtx_msg_hdr *header = (void *)reg_buf;
    task_t sender_tid = 0;

    mbx_create(128); 
    size_t msg_hdr_size = sizeof(struct rtx_msg_hdr);
    header->length = msg_hdr_size + 1;
    header->type = KCD_REG;
    reg_buf[msg_hdr_size] = 'W';
    send_msg(TID_KCD, reg_buf);

    mem_dealloc(reg_buf);

    char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    LPC_TIM_TypeDef *pTimer;
    
    while(1)
    {
        char temp_buffer[19]; //Longest possible message length is 19

        if(recv_msg(&sender_tid, temp_buffer, 20))
        {
            if((U32)temp_buffer[4] == KEY_IN)
            {
                //%WR
                if(temp_buffer[9] == 'R')
                {
                    //Reset clock time
                    wall_clock_enabled = 0;
                    time.sec = 0;
                    time.min = 0;   
                    time.hr = 0;
                    g_timer_count_wall = 0;
                    wall_clock_enabled = 1;
                }
                //%WS
                else if(temp_buffer[9] == 'S')
                {
                    //Set current time to system time
                    struct timeval_rt system_time;
                    get_time(&system_time);
                    wall_clock_enabled = 0;
                    time.hr = system_time.sec/3600;
                    system_time.sec -= time.hr*3600;
                    time.min = system_time.sec/60;
                    system_time.sec -= system_time.sec;
                    g_timer_count_wall = system_time.usec;

                    //enable clock
                    wall_clock_enabled = 1; 
                }
                //%WT
                else if(temp_buffer[9] == 'T')
                {
                    wall_clock_enabled = 0;
                }
                //%WS hh:mm:ss
                else if(temp_buffer[9] == 'S' && (U32)(temp_buffer) == msg_hdr_size + 11)
                {
                    wall_clock_enabled = 0;
                    time.hr = (temp_buffer[msg_hdr_size + 3] - '0')*10 + (temp_buffer[msg_hdr_size + 4] - '0');
                    time.min = (temp_buffer[msg_hdr_size + 6] - '0')*10 + (temp_buffer[msg_hdr_size + 7] - '0');
                    time.sec = (temp_buffer[msg_hdr_size + 9] - '0')*10 + (temp_buffer[msg_hdr_size + 10] - '0');
                    g_timer_count_wall = 0;
                    wall_clock_enabled = 1;
                }
                else
                {
                    //Display error message in terminal
                    char *message = "Command cannot be processed.\n\r"; 
                    U32 string_length = 31; //28 chars + new line + carriage return + null terminator
                    U8 *buf = (U8 *)mem_alloc(msg_hdr_size + string_length);
                    RTX_MSG_HDR *header = (void*)buf;
                    header->length = msg_hdr_size + string_length;
                    header->type = DISPLAY;
                    mem_cpy(buf + msg_hdr_size, message, string_length);
                    send_msg(TID_DISPLAY, buf);
                    mem_dealloc(buf);
                }
            }
        }
    }
}