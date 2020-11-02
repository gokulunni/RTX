/* The KCD Task Template File */
#include "rtx.h"
#include "kcd_task.h"
int str_cmp(const char *str1, const char *str2)
{
    int s1;
    int s2;
    do {
        s1 = *str1++;
        s2 = *str2++;
        if (s1 == 0)
            break;
    } while (s1 == s2);
    return (s1 != s2) ? -1 : 0;
}
void push_cmd(REGISTERED_CMD_T **registered_cmd_head, REGISTERED_CMD_T *new_cmd)
{
	REGISTERED_CMD_T *existing_cmd = get_cmd(*registered_cmd_head, new_cmd -> cmd);
	if(existing_cmd != NULL)
	{
		existing_cmd -> handler_tid = new_cmd -> handler_tid;
	}
	else
	{
		REGISTERED_CMD_T *tmp = *registered_cmd_head;
		while(tmp -> next != NULL)
		{
			tmp = tmp -> next;
		}
		
		tmp -> next = new_cmd;
		new_cmd -> next = NULL;
	}		
}

REGISTERED_CMD_T *get_cmd(REGISTERED_CMD_T *registered_cmd_head, char *cmd)
{
	REGISTERED_CMD_T *tmp = registered_cmd_head;
	while(tmp != NULL)
	{
		if(str_cmp(tmp -> cmd, cmd) == 0)
		{
			return tmp;
		}
	}
	
	return NULL;
}

void kcd_task(void)
{
    mbx_create(128); //TODO: Determine whether this is an appropriate size
    task_t sender_tid;
    int command_specifier = 0;
    char current_command[64];
    int command_index = 0;

    while(1)
    {
        U8 temp_buffer[64];
        if(recv_msg(&sender_tid, &temp_buffer , 64) == 0)
        {
            //KCD_CMD
            if((U32)temp_buffer[31] == KCD_CMD)
            {
            }

            //KCD_REG
            if((U32)temp_buffer[31] == KCD_REG)
            {
                
            }

            //KEY_IN
            if((U32)temp_buffer[31] == KEY_IN)
            {
                if(command_specifier)
                {
                    current_command[command_index] = temp_buffer[32];   
                    command_index++;
                    if(current_command[command_index - 1] == '\n')
                    {
                        if(str_cmp(current_command, "LT") == 0)
                        {

                        }
                        else if(str_cmp(current_command, "MT") == 0)
                        {

                        }
                        else if(linked list contains command handler task)
                        {
                            //send to mailbox of appropriate task
                        }
                        else /* unregistered command */
                        {
                            size_t msg_hdr_size = sizeof(RTX_MSG_HDR);
                            U8 buf[msg_hdr_size + 28];
                            RTX_MSG_HDR *header = (void*)buf;
                            header->length = msg_hdr_size + 1;
                            header->type = DISPLAY;
														char *message = "Command cannot be processed.";
                            for(int i = 0; i < 28; i++)
														{
															buf[msg_hdr_size + i] = message[i];
														}
                            
                            //TO DO: Do we need to add sender_tid (TID_UART0_IRQ) with the message?
                            //TO DO: define TID_LCD
                            send_msg(TID_LCD, buf);
                        }
                        
                    }
                }
                else if(temp_buffer[32] == '%')
                    command_specifier = 1;
                else
                {
                    
                }
                
                
            }
        }
    }
}
