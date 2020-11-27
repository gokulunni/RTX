/* The KCD Task Template File */
#include "rtx.h"
#include "kcd_task.h"
#include "k_mem.h"
#include "k_rtx.h"
#include "helpers.h"

REGISTERED_CMD_T *registered_cmd_head = NULL; //head of linked list of registered tasks
extern TCB g_tcbs[MAX_TASKS];

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

REGISTERED_CMD_T *get_cmd(REGISTERED_CMD_T *registered_cmd_head, char cmd)
{
	REGISTERED_CMD_T *tmp = registered_cmd_head;
	while(tmp != NULL)
	{
		if(tmp -> cmd == cmd)
		{
			return tmp;
		}
		tmp = tmp -> next;
	}
	
	return NULL;
}

void print_failed_cmd(void)
{
  //Display error message in terminal
	int msg_hdr_size = 8;
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

void kcd_task(void)
{
  mbx_create(128); //TODO: Determine whether this is an appropriate size
  task_t sender_tid;
  int command_specifier = 0;
  char current_command[64];
  int command_index = 0;
  size_t msg_hdr_size = sizeof(RTX_MSG_HDR);

  registered_cmd_head =(REGISTERED_CMD_T *)mem_alloc(sizeof(REGISTERED_CMD_T));
  registered_cmd_head -> handler_tid = TID_KCD;
  registered_cmd_head -> cmd = 'L';
	registered_cmd_head -> next = NULL;

  char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  
  while(1)
  { 
    char temp_buffer[13];

    if(recv_msg(&sender_tid, temp_buffer , msg_hdr_size + 5) == RTX_OK)
    {
      /* Check the message type */

      //KCD_REG
      if((U32)temp_buffer[4] == KCD_REG)
      {
        U32 cmd_length = ((U32)temp_buffer[0]) - msg_hdr_size;
        if(cmd_length != 1)
        {
          #ifdef DEBUG_KCD
          printf("Command cannot be registered - Incorrect Length.\n");
          #endif
        }
        char cmd = temp_buffer[msg_hdr_size];
        if(cmd == 'L')
        {
          #ifdef DEBUG_KCD
          printf("'L' is a reserved command - Registration Failed.\n");
          #endif
        }

        REGISTERED_CMD_T *new_cmd =(REGISTERED_CMD_T *)mem_alloc(sizeof(REGISTERED_CMD_T));
        new_cmd -> handler_tid = sender_tid;
        new_cmd -> cmd = cmd;
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
					
					if(!command_specifier)
					{
						print_failed_cmd();
					}
        }
        else
        {
          temp_buffer[msg_hdr_size + 1] = '\0'; //null terminate the string 
          header -> length = msg_hdr_size + 2;
          send_msg(TID_DISPLAY, temp_buffer);
        }
          
        if(command_specifier) //check if '%' was typed already
        {
					//If backspace was entered, go back a character
					if(current_command[command_index] == 0x7F)
					{
						command_index -= 2; 
					}
          if(current_command[command_index] == '\r')
          {
              current_command[command_index] = '\0'; //null terminate string for comparison
							REGISTERED_CMD_T *cmd = get_cmd(registered_cmd_head, current_command[1]); //get registered cmd
						
              if(str_cmp(current_command + 1, "LT") == 0)
              {														
                // Send list of tids to LCD task
                task_t tids[MAX_TASKS];
                int num_tasks = tsk_ls(tids, MAX_TASKS);
                char *display_buffer = (char *)mem_alloc(msg_hdr_size + 4*num_tasks + 1); //upper bound on size
                header = (void *)display_buffer;
                header -> length = msg_hdr_size;
                header -> type = DISPLAY;
								
                //Convert the tids to char for displaying, and display each in a new line
                for(int i = 0; i < num_tasks; i++)
                {
                  if(tids[i] > 9)
                  {
                    int first_digit = 1;
										int second_digit = tids[i] - 10;
                    display_buffer[(header -> length)++] = digits[first_digit];
                    display_buffer[(header -> length)++] = digits[second_digit];
                    display_buffer[(header -> length)++] = '\n';
                    display_buffer[(header -> length)++] = '\r';
                  }
                  else
                  {
                    display_buffer[(header -> length)++] = digits[tids[i]];
                    display_buffer[(header -> length)++] = '\n';
                    display_buffer[(header -> length)++] = '\r';
                  }
                }
                display_buffer[(header -> length)++] = '\0';
                send_msg(TID_DISPLAY, display_buffer);
                mem_dealloc(display_buffer);
              }
              else if(str_cmp(current_command + 1, "LM") == 0)
              {
                // Send list of tids to LCD task
                task_t tids[MAX_TASKS];
                int num_tasks = mbx_ls(tids, MAX_TASKS);
                char *display_buffer = (char *)mem_alloc(msg_hdr_size + 4*num_tasks + 1); //upper bound on size
                header = (void *)display_buffer;
                header -> length = msg_hdr_size;
                header -> type = DISPLAY;
                //Conver the tids to char for displaying, and display each in a new line
                for(int i = 0; i < num_tasks; i++)
                {
                  if(tids[i] > 9)
                  {
                    int first_digit = 1;
										int second_digit = tids[i] - 10;
                    display_buffer[(header -> length)++] = digits[first_digit];
                    display_buffer[(header -> length)++] = digits[second_digit];
                    display_buffer[(header -> length)++] = '\n';
                    display_buffer[(header -> length)++] = '\r';
                  }
                  else
                  {
                    display_buffer[(header -> length)++] = digits[tids[i]];
                    display_buffer[(header -> length)++] = '\n';
                    display_buffer[(header -> length)++] = '\r';
                  }
                }
                display_buffer[(header -> length)++] = '\0';
                send_msg(TID_DISPLAY, display_buffer);
                mem_dealloc(display_buffer);
              }
              else if(cmd != NULL) /* Registered command */
              {
                // Send to mailbox of registered task
								size_t string_length = (command_index + 1) - 2; //disregard '/r' and '%'
								U8 *buf = (U8 *)mem_alloc(msg_hdr_size + string_length);
								RTX_MSG_HDR *header = (void*)buf;
                header->length = msg_hdr_size + string_length;
                header->type = KCD_CMD;
                mem_cpy(buf + msg_hdr_size, current_command + 1, string_length);
                send_msg(cmd->handler_tid, buf);
              }
              else /* unregistered command */
              {
                print_failed_cmd();
              }

              //reset the command buffer since 'ENTER' was pressed
              command_index = 0; 
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
          //Else we simply do nothing since it is not a command and we already echoed char
					mem_dealloc(temp_buffer);
      }
    }
  }
}
