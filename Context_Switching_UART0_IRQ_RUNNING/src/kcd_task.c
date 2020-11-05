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

  //TODO: Do we simply ignore control keys (i.e. arrows and fn keys)
  U8 up_arrow[] = {0x1B, 0x41, '\n'};
  U8 down_arrow[] = {0x1B, 0x42, '\n'};
  U8 right_arrow[] = {0x1B, 0x43, '\n'};
  U8 left_arrow[] = {0x1B, 0x44, '\n'};
  
  while(1)
  { 
    char temp_buffer[11]; //ASSUMPTION: No cmd will be longer than 3 chars? (for KCD_REG)


    if(recv_msg(&sender_tid, temp_buffer , msg_hdr_size + 3) == 0)
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
        
        //TODO: Determine whether the msg for KEY_IN is always a single char, to simplifiy logic
          U32 msg_length = (((RTX_MSG_HDR *)temp_buffer) -> length) - msg_hdr_size;
          for(int i = 0; i < msg_length; i++)
          {
            current_command[command_index] = temp_buffer[msg_hdr_size + i]; //add typed char to current cmd string
            command_index++;
          }
          --command_index; //we want to point to last char in array

        if(command_specifier) //check if '%' was typed already
        {
          if(current_command[command_index] == '\n')
          {
              current_command[command_index] = '\0'; //null terminate string for comparison
              if(str_cmp(current_command, "LT") == 0)
              {														
                //1. echo command
                char *message = "LT"; 
                U8 buf[11];
                RTX_MSG_HDR *header = (void*)buf;
                header->length = msg_hdr_size + 3;
                header->type = DISPLAY;
                mem_cpy(buf + msg_hdr_size, message, 3);

                send_msg(TID_DISPLAY, buf);

                //2. Send list of tids to LCD task
                task_t tids[MAX_TASKS];
                int num_tasks = tsk_ls(tids, MAX_TASKS);
                U8 *display_buffer = (U8 *)mem_alloc(msg_hdr_size + num_tasks*sizeof(task_t));
                header = (void *)display_buffer;
                header -> length = msg_hdr_size + sizeof(task_t)*num_tasks;
                header -> type = DISPLAY;
                mem_cpy(display_buffer + msg_hdr_size, tids, num_tasks * sizeof(task_t));
                send_msg(TID_DISPLAY, display_buffer);
              }
              else if(str_cmp(current_command, "LM") == 0)
              {
                //1. echo command
                char *message = "LM"; 
                U8 buf[11];
                RTX_MSG_HDR *header = (void*)buf;
                header->length = msg_hdr_size + 3;
                header->type = DISPLAY;
                mem_cpy(buf + msg_hdr_size, message, 3);

                send_msg(TID_DISPLAY, buf);

                //2. Send list of tids to LCD task
                task_t tids[MAX_TASKS];
                int num_tasks = mbx_ls(tids, MAX_TASKS);
                U8 *display_buffer = (U8 *)mem_alloc(msg_hdr_size + num_tasks*sizeof(task_t));
                header = (void *)display_buffer;
                header -> length = msg_hdr_size + sizeof(task_t)*num_tasks;
                header -> type = DISPLAY;
                mem_cpy(display_buffer + msg_hdr_size, tids, num_tasks * sizeof(task_t));
                send_msg(TID_DISPLAY, display_buffer);
              }
              
              REGISTERED_CMD_T *cmd = get_cmd(registered_cmd_head, current_command);
              if(cmd != NULL) /* Registered command */
              {
                //1. Echo command
                size_t string_length = command_index + 1;
                U8 *buf = (U8*)mem_alloc(msg_hdr_size + string_length);
                RTX_MSG_HDR *header = (void*)buf;
                header->length = msg_hdr_size + string_length;
                header->type = DISPLAY;
                mem_cpy(buf + msg_hdr_size, current_command, string_length);

                send_msg(TID_DISPLAY, buf);
                
                //2. Send to mailbox of registered task
                header->type = KCD_CMD;
                send_msg((g_tcbs[cmd->handler_tid]).tid, buf);
								
								mem_dealloc(buf);
              }
              else /* unregistered command */
              {
                //Display error message in terminal
                char *message = "Command cannot be processed."; 
                size_t string_length = 29; //28 chars + new line
                U8 *buf = (U8 *)mem_alloc(msg_hdr_size + string_length);
                RTX_MSG_HDR *header = (void*)buf;
                header->length = msg_hdr_size + string_length;
                header->type = DISPLAY;
                mem_cpy(buf + msg_hdr_size, current_command, string_length);
                
                send_msg(TID_DISPLAY, buf);
								mem_dealloc(buf);
              }

              //TODO: Confirm that we can reuse buffer and don't need to reallocate
              //mem_dealloc(temp_buffer);
              //temp_buffer = (U8 *)mem_alloc(msg_hdr_size + 3);

              command_index = 0; //reset the buffer since 'ENTER' was pressed
              command_specifier = 0;
            }
            else //wait for next character, cmd is not finished
            {
              command_index++;
            }
          }
          else if(command_index == 0 && current_command[0] == '%') //comand specifier was typed
          {
            command_specifier = 1;
          }
          else //Not a command, simply echo keystroke
          {
            RTX_MSG_HDR *header = (RTX_MSG_HDR *)temp_buffer;
            temp_buffer[msg_hdr_size + 1] = '\0'; //null terminate the string 
            header -> length = msg_hdr_size + 2;
            header -> type = DISPLAY;
            send_msg(TID_DISPLAY, temp_buffer);
          }
					mem_dealloc(temp_buffer);
      }
    }
  }
}
