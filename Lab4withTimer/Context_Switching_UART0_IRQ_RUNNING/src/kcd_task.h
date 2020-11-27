#ifndef KCD_TASK_H_
#define KCD_TASK_H_
#include "rtx.h"

typedef struct registered_command {
	char cmd;
	task_t handler_tid;
	struct registered_command *next;
}REGISTERED_CMD_T;

void push_cmd(REGISTERED_CMD_T **registered_cmd_head, REGISTERED_CMD_T *new_cmd);
//REGISTERED_CMD_T *pop_cmd(REGISTERED_CMD_T **registered_cmd_head);
REGISTERED_CMD_T *get_cmd(REGISTERED_CMD_T *registered_cmd_head, char cmd);

int str_cmp(const char *str1, const char *str2);
void print_failed_cmd(void);


#endif /* KCD_TASK_H */