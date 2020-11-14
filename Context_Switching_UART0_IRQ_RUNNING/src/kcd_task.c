/* The KCD Task Template File */
#include "rtx.h"
#include "kcd_task.h"
#include "k_mem.h"
#include "k_rtx.h"

REGISTERED_CMD_T *registered_cmd_head = NULL; //head of linked list of registered tasks
extern TCB g_tcbs[MAX_TASKS];

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
	if(registered_cmd_head == NULL)
	{
		*registered_cmd_head = new_cmd;
		return;
	}
	
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
  size_t msg_hdr_size = sizeof(RTX_MSG_HDR);

  //TODO: Do we simply ignore control keys? (i.e. arrows and fn keys)
  U8 up_arrow[] = {0x1B, 0x41, '\n'};
  U8 down_arrow[] = {0x1B, 0x42, '\n'};
  U8 right_arrow[] = {0x1B, 0x43, '\n'};
  U8 left_arrow[] = {0x1B, 0x44, '\n'};
  char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  
  while(1)
  { 
    char temp_buffer[13];

    if(recv_msg(&sender_tid, temp_buffer , msg_hdr_size + 5) == 0)
    {
      /* Check the message type */

      //KCD_REG
      if((U32)temp_buffer[4] == KCD_REG)
      {
        REGISTERED_CMD_T *new_cmd =(REGISTERED_CMD_T *)mem_alloc(sizeof(REGISTERED_CMD_T));
        new_cmd -> handler_tid = sender_tid;
        U32 string_length = ((U32)temp_buffer[0]) - msg_hdr_size;
        new_cmd -> cmd = (char *)mem_alloc(sizeof(string_length));
        mem_cpy(new_cmd -> cmd, temp_buffer + msg_hdr_size, string_length);
        push_cmd(&registered_cmd_head, new_cmd);
      }

      //KEY_IN
      else if((U32)temp_buffer[4] == KEY_IN)
      {
					current_command[command_index] = temp_buffer[msg_hdr_size]; //add typed char to current cmd string

          //ALWAYS echo the character
					//Replace carriage return with new line on output
          RTX_MSG_HDR *header = (RTX_MSG_HDR *)temp_buffer;
          header -> type = DISPLAY;
					if(current_command[command_index] == '\r')
          {
						temp_buffer[msg_hdr_size] = '\n';
            temp_buffer[msg_hdr_size + 1] = '\r';
            temp_buffer[msg_hdr_size + 2] = '\0'; //null terminate the string 
            header -> length = msg_hdr_size + 3;
            send_msg(TID_DISPLAY, temp_buffer);
          }
					else
          {
            temp_buffer[msg_hdr_size + 1] = '\0'; //null terminate the string 
            header -> length = msg_hdr_size + 2;
            send_msg(TID_DISPLAY, temp_buffer);
          }
          
        if(command_specifier) //check if '%' was typed already
        {
          if(current_command[command_index] == '\r')
          {
              current_command[command_index] = '\0'; //null terminate string for comparison
							REGISTERED_CMD_T *cmd = get_cmd(registered_cmd_head, current_command + 1); //get registered cmd
						
              if(str_cmp(current_command + 1, "LT") == 0)
              {														
                // Send list of tids to LCD task
                task_t tids[MAX_TASKS];
                int num_tasks = tsk_ls(tids, MAX_TASKS);
                char *display_buffer = (char *)mem_alloc(msg_hdr_size + 5);
                header = (void *)display_buffer;
                header -> length = msg_hdr_size + 4;
                header -> type = DISPLAY;
								
                //Convert the tids to char for displaying, and display each in a new line
                for(int i = 0; i < num_tasks; i++)
                {
                  if(tids[i] > 9)
                  {
                    header -> length = msg_hdr_size + 5;
                    int first_digit = 1;
										int second_digit = tids[i] - 10;
                    display_buffer[msg_hdr_size] = digits[first_digit];
                    display_buffer[msg_hdr_size + 1] = digits[second_digit];
                    display_buffer[msg_hdr_size + 2] = '\n';
                    display_buffer[msg_hdr_size + 3] = '\r';
                    display_buffer[msg_hdr_size + 4] = '\0';
                  }
                  else
                  {
                    display_buffer[msg_hdr_size] = digits[tids[i]];
                    display_buffer[msg_hdr_size + 1] = '\n';
                    display_buffer[msg_hdr_size + 2] = '\r';
                    display_buffer[msg_hdr_size + 3] = '\0';
                  }
									send_msg(TID_DISPLAY, display_buffer);
                }
                mem_dealloc(display_buffer);
              }
              else if(str_cmp(current_command + 1, "LM") == 0)
              {
                // Send list of tids to LCD task
                task_t tids[MAX_TASKS];
                int num_tasks = mbx_ls(tids, MAX_TASKS);
                char *display_buffer = (char *)mem_alloc(msg_hdr_size + 5);
                header = (void *)display_buffer;
                header -> length = msg_hdr_size + 4;
                header -> type = DISPLAY;
                //Conver the tids to char for displaying, and display each in a new line
                for(int i = 0; i < num_tasks; i++)
                {
                  if(tids[i] > 9)
                  {
                    header -> length = msg_hdr_size + 5;
                    int first_digit = 1;
										int second_digit = tids[i] - 10;
                    display_buffer[msg_hdr_size] = digits[first_digit];
                    display_buffer[msg_hdr_size + 1] = digits[second_digit];
                    display_buffer[msg_hdr_size + 2] = '\n';
                    display_buffer[msg_hdr_size + 3] = '\r';
                    display_buffer[msg_hdr_size + 4] = '\0';
                  }
                  else
                  {
                    display_buffer[msg_hdr_size] = digits[tids[i]];
                    display_buffer[msg_hdr_size + 1] = '\n';
                    display_buffer[msg_hdr_size + 2] = '\r';
                    display_buffer[msg_hdr_size + 3] = '\0';
                  }
									send_msg(TID_DISPLAY, display_buffer);
                }
                mem_dealloc(display_buffer);
              }
              else if(cmd != NULL) /* Registered command */
              {
                // Send to mailbox of registered task
                header->type = KCD_CMD;
                send_msg((g_tcbs[cmd->handler_tid]).tid, temp_buffer);
              }
              else /* unregistered command */
              {
                //Display error message in terminal
                char *message = "Command cannot be processed.\n\r"; 
                size_t string_length = 31; //28 chars + new line + carriage return + null terminator
                U8 *buf = (U8 *)mem_alloc(msg_hdr_size + string_length);
                RTX_MSG_HDR *header = (void*)buf;
                header->length = msg_hdr_size + string_length;
                header->type = DISPLAY;
                mem_cpy(buf + msg_hdr_size, message, string_length);
                
                send_msg(TID_DISPLAY, buf);
								mem_dealloc(buf);
              }

              //TODO: Confirm that we can reuse buffer and don't need to reallocate
              //mem_dealloc(temp_buffer);
              //temp_buffer = (U8 *)mem_alloc(msg_hdr_size + 3);

              command_index = 0; //reset the buffer since 'ENTER' was pressed
              command_specifier = 0;
            }
            else //wait for next character
            {
              command_index++;
            }
          }
          else if(command_index == 0 && current_command[0] == '%') //command specifier was typed
          {
            current_command[0] = '%';
            command_index++;
            command_specifier = 1;
          }
          //Else we simply do nothing since it is not a command
					mem_dealloc(temp_buffer);
      }
    }
  }
}
