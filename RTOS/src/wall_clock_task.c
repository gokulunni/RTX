#include <LPC17xx.h>
#include "common.h"
#include "rtx.h"
#include "wall_clock_task.h"
#include "k_mem.h"
#include "kcd_task.h"
#include "helpers.h"

struct time_t time; //Wall clock time
extern volatile U32 g_timer_count_wall;
int wall_clock_enabled = 0;

void send_time()
{
    char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    size_t msg_hdr_size = sizeof(struct rtx_msg_hdr);
    U8 buf[19]; //msg_hdr (8) display time length (8) + new line + carriage return + null termination
    struct rtx_msg_hdr *header = (void *)buf;
    header -> length = msg_hdr_size + 8 + 3;
    header -> type = DISPLAY;

    //HH:
    buf[msg_hdr_size] = digits[time.hr/10];
    buf[msg_hdr_size + 1] = digits[time.hr%10];
    buf[msg_hdr_size + 2] = ':';

    //MM:
    buf[msg_hdr_size + 3] = digits[time.min/10];
    buf[msg_hdr_size + 4] = digits[time.min%10];
    buf[msg_hdr_size + 5] = ':';
    
    //SS
    buf[msg_hdr_size + 6] = digits[time.sec/10];
    buf[msg_hdr_size + 7] = digits[time.sec%10];
    buf[msg_hdr_size + 8] = '\n';
    buf[msg_hdr_size + 9] = '\r';
    buf[msg_hdr_size + 10] = '\0';
		
		//use escape sequence to display on top left
		char *seq = "\033[H"; 
		size_t string_length = 7; //6 + null terminator
		U8 esc_buf[15]; //statically allocate since this is called in timer handler
		header = (void*)esc_buf;
		header->length = msg_hdr_size + string_length;
		header->type = DISPLAY;
		mem_cpy(esc_buf + msg_hdr_size, seq, string_length);
		
		if(k_send_msg(TID_DISPLAY, esc_buf) == RTX_ERR)
		{
				#ifdef DEBUG_WALL_CLOCK
        printf("wall clock task: Message sending to LCD failed\n");
        #endif
		}

    if(k_send_msg(TID_DISPLAY, buf) == RTX_ERR)
    {
        #ifdef DEBUG_WALL_CLOCK
        printf("wall clock task: Message sending to LCD failed\n");
        #endif
    }
}

void wall_clock_task(void) {
    U8 *reg_buf = mem_alloc(8 + 2);
    if(reg_buf == NULL)
    {
        #ifdef DEBUG_WALL_CLOCK
        printf("wall clock task: Allocation of buffer failed\n");
        #endif
        tsk_exit();
    }

    struct rtx_msg_hdr *header = (void *)reg_buf;
    task_t sender_tid = 0;

    if(mbx_create(128) == RTX_ERR)
    {
        #ifdef DEBUG_WALL_CLOCK
        printf("wall clock task: Mailbox creation failed\n");
        #endif
        tsk_exit();
    }
    size_t msg_hdr_size = sizeof(struct rtx_msg_hdr);
    header->length = msg_hdr_size + 1;
    header->type = KCD_REG;
    reg_buf[msg_hdr_size] = 'W';
    
    if (send_msg(TID_KCD, reg_buf) == RTX_ERR) {
        #ifdef DEBUG_WALL_CLOCK
        printf("wall clock task: Command registration failed\n");
        #endif
        tsk_exit();
    }

    mem_dealloc(reg_buf);

    LPC_TIM_TypeDef *pTimer;
    
    while(1)
    {
        char temp_buffer[19]; //Longest possible message length is 19

        if(recv_msg(&sender_tid, temp_buffer, 20) == RTX_OK)
        {
            if((U32)temp_buffer[4] == KCD_CMD)
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
								//%WS hh:mm:ss
                else if(temp_buffer[9] == 'S' && (U32)(temp_buffer[0]) == 19)
                {
                    wall_clock_enabled = 0;
                    time.hr = (temp_buffer[msg_hdr_size + 3] - '0')*10 + (temp_buffer[msg_hdr_size + 4] - '0');
                    time.min = (temp_buffer[msg_hdr_size + 6] - '0')*10 + (temp_buffer[msg_hdr_size + 7] - '0');
                    time.sec = (temp_buffer[msg_hdr_size + 9] - '0')*10 + (temp_buffer[msg_hdr_size + 10] - '0');
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
                else
                {
                    print_failed_cmd();
                }
            }
        }
    }
}