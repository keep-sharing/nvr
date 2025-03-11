#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include "msstd.h"
#include "mssocket.h"
#include "msdefs.h"
#include "msg.h"

#define MAX_BUFF_CLI 256
#define MAX_TIME_LIMIT_SEC (15)

static int global_exit = 0;
static int global_exit_ex = 0;
unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
hook_print global_debug_hook = 0;

static void recv_sock_data(void* arg, struct packet_header* header, void * data)
{
    if (header->type == REQUEST_FLAG_CLI_EXIT)
        global_exit_ex = 1;
    else
    	printf("%s", (char*)data);
}

static int ms_cli_init()
{
	if (ms_sock_init(recv_sock_data, NULL, SOCKET_TYPE_CLI, SOCK_MSSERVER_PATH) != 0)
	{
		msdebug(DEBUG_ERR, "mscli: init sock fail.");
		return -1;
	}

	return 0;
}

static void ms_cli_uninit()
{
	ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_CLI_EXIT, NULL, 0, 0);
	ms_sock_uninit();
}

static void send_cli(void* data, int len)
{
	int res = ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_CLI_ENTRANCE, data, len, 0);
	if(res == -1)
		msprintf("send_cli failed.sock client state:%d", ms_client_socket_state());
}

static int check_cli_data(char cli[256])
{
	int len = strlen(cli);
	if (!cli || cli[0]==0)
		return -1;
	if (cli[len - 1] == '\n')
		cli[len - 1] = '\0';
	if (strcmp(cli, ".help")==0)
		snprintf(cli, 256, "cli show commands");

	return 0;
}

static int parse_cli_info(const char* cli, struct cli_dst_info* dst)
{
	char* mod = NULL;
	char* cmd = NULL;
	char* result = NULL;
	if (!cli || cli[0]==0) 
		return -1;
		
	while(*cli==' ')
		cli++; 	
	if (*cli==0)
		return -1;
	
	mod = (char*)(cli);
	result = strchr(mod, ' ');
	if (!result) 
		return -1;
	snprintf(dst->module, sizeof(dst->module)>(result-mod+1)?(result-mod+1):sizeof(dst->module), "%s", mod);
	while(*result==' ') 
		result++; 
	if (*result==0) 
		return -1;

	cmd = result;
	result = strchr(cmd, ' ');
	if (!result)
		return -1;
	while(*result==' ') 
		result++; 
	if (*result==0)
		return -1;
	
	char* cmd2 = result;
	result = strchr(cmd2, ' ');
	if (!result)
	{
		snprintf(dst->command, sizeof(dst->command), "%s", cmd);
	}
	else
	{
		snprintf(dst->command, sizeof(dst->command)>(result-cmd+1)?(result-cmd+1):sizeof(dst->command), "%s", cmd);
		while(*result==' ') 
			result++; 
		if (*result==0)
			return 0;
		snprintf(dst->argument, sizeof(dst->argument), result);
	}
	
	return 0;
}

static void signal_handler(int signal)
{
	global_exit = 1;
	global_exit_ex = 1;
	msprintf("remove mscli.signo: %d", signal);
}

static void set_core_signal()
{
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGPIPE, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGTSTP, signal_handler);
	signal(SIGKILL, signal_handler);
}


static void ms_save_pid()
{
	char cmd[32];
	pid_t pId = getpid();
	snprintf(cmd, sizeof(cmd), "echo %d > %s", pId, MS_CLI_PID);
	ms_system(cmd); 
}

int main(int argc, char **argv)
{
	char data[MAX_BUFF_CLI] = {0}, buff[MAX_BUFF_CLI] = {0};
	struct cli_dst_info dst;
	if (ms_cli_init() == -1)
		return 0;

	int nTimetamp = time(0);
	ms_save_pid();
	set_core_signal();
	printf("Enter \".help\" for instructions, 'q' to quit\n");
	printf("Build Time:%s, %s\n", __DATE__, __TIME__);

	if (argc > 1)
	{
	    int i = 0;
	    int len = 0;
	    for (i = 1; i < argc; i++)
	    {
	        data[len++] = ' ';
            strcpy(&data[len], argv[i]);
	        len += strlen(argv[i]);
	    }
        printf("data[%s]\n", data);
        memset(&dst, 0, sizeof(dst));
        if (parse_cli_info(data, &dst) != 0) 
        {
            printf("sorry man, invalid cli.\n");
            return 0;
        }
        send_cli(&dst, sizeof(dst));
        while(!global_exit_ex)
        {
			usleep(50*1000);
			if(time(0) - nTimetamp >= MAX_TIME_LIMIT_SEC)
			{
				//msdebug(DEBUG_ERR, "mscli time out!!!!!!!!!!!");
				break;
			}
		}
	}
	else
	{
        while(!global_exit)
        {
            printf("mscli>");
            memset(data, 0, sizeof(data));
            if (fgets(data, sizeof(data), stdin) == NULL)
            {
                usleep(20*1000);
                continue;
            }
            //printf("[david debug] data:%02x, %02x, %02x, %02x, %02x\n", data[0], data[1], data[2], data[3], data[4]);
            if(data[0] == 0x1b && data[1] == 0x5b){
                memcpy(data, buff, sizeof(buff));
            }
            else if(data[0] != 0x0a){
                memcpy(buff, data, sizeof(data));
            }
            if (data[0] == 'q')
                break;
            if (check_cli_data(data) != 0) 
                continue;
            memset(&dst, 0, sizeof(dst));
            if (parse_cli_info(data, &dst) != 0) 
            {
                printf("sorry man, invalid cli.\n");
                continue;
            }
            send_cli(&dst, sizeof(dst));
        }
	}
    
    printf("sorry man, exit cli.\n");
	ms_cli_uninit();
	return 0;
}
