#include <stdio.h>	
#include <sys/types.h>	
#include <sys/stat.h>  
#include <fcntl.h>	
#include <unistd.h>  
#include <string.h>  
#include <linux/input.h>
#include <linux/joystick.h>
#include <math.h>
#include "msstd.h"
#include "mssocket.h"
#include "msg.h"
#include "js_parse.h"
#include "msjs.h"


#define NMLZ_RATIO	51.2
#define JS_DEV_NAME  "/dev/input/js0"  //only to handle js0
#define JS_ABS(a)	   (((a) < 0) ? -(a) : (a))

static pthread_t	g_msjs_pid = 0;
static int 			g_msjs_start = 0;

struct js_rate_s
{
	int         Hrate;
    int         Vrate;
	int         Zrate;
};

static int data_nmlz(int value)
{
	int ret = 0;
	double result;
	
	result = (value-512)/NMLZ_RATIO;
	if (JS_ABS(result) >= 1.0)
		ret = round(result);
	else
		ret = 0;
//	printf("==result=%f, ret=%d==\n", result, ret);
	return ret;
}



static void J_ZOOM_choose_enAction(EVENT_S *pstEvent, int Zrate)
{
	if (Zrate == 0)
	{
		pstEvent->enAction = ACTION_STOP;
	}
	else if (Zrate < 0)
	{
		pstEvent->enAction = ACTION_WIDE;
	}
	else
	{
		pstEvent->enAction = ACTION_TELE;
	}
	
	pstEvent->Zrate = JS_ABS(Zrate);
}

static void J_MOVE_choose_enAction(EVENT_S *pstEvent, int Hrate, int Vrate)
{
	if (Hrate==0 && Vrate==0)
	{
		pstEvent->enAction = ACTION_STOP;
	}
	else if (Vrate == 0)
	{
		if (Hrate < 0)
			pstEvent->enAction = ACTION_LL;
		else
			pstEvent->enAction = ACTION_RR;
	}
	else if (Hrate == 0)
	{
		if (Vrate < 0)
			pstEvent->enAction = ACTION_UU;
		else
			pstEvent->enAction = ACTION_DD;
	}
	else
	{
		if (Hrate < 0)
		{
			if (Vrate < 0)
				pstEvent->enAction = ACTION_LU;
			else
				pstEvent->enAction = ACTION_LD;
		}
		else
		{
			if (Vrate < 0)
				pstEvent->enAction = ACTION_RU;
			else
				pstEvent->enAction = ACTION_RD;
		}
	}
	
	pstEvent->Hrate = JS_ABS(Hrate);
	pstEvent->Vrate = JS_ABS(Vrate);
}

static void js_event_to_cmd(EVENT_S *pstEvent, char *cmd)
{
//	printf("==(%d, %d, %d)\n", pstEvent->enType, pstEvent->enKey, pstEvent->enAction);
	if (pstEvent->enType == TYPE_JOYSTICK)
	{
		if (pstEvent->enKey == _KEY_J_MOVE)
		{
			if (pstEvent->enAction == ACTION_LL)
			{
				sprintf(cmd, "IP-Controller:J-LL-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			else if (pstEvent->enAction == ACTION_RR)
			{
				sprintf(cmd, "IP-Controller:J-RR-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			//else if (pstEvent->enAction == ACTION_RR)
			//{
			//	sprintf(cmd, "IP-Controller:J-RR-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			//}
			else if (pstEvent->enAction == ACTION_UU)
			{
				sprintf(cmd, "IP-Controller:J-UU-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			else if (pstEvent->enAction == ACTION_DD)
			{
				sprintf(cmd, "IP-Controller:J-DD-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			else if (pstEvent->enAction == ACTION_LU)
			{
				sprintf(cmd, "IP-Controller:J-LU-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			else if (pstEvent->enAction == ACTION_RU)
			{
				sprintf(cmd, "IP-Controller:J-RU-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			else if (pstEvent->enAction == ACTION_LD)
			{
				sprintf(cmd, "IP-Controller:J-LD-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			else if (pstEvent->enAction == ACTION_RD)
			{
				sprintf(cmd, "IP-Controller:J-RD-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
			else
			{
				sprintf(cmd, "IP-Controller:J-ST-H%02d-V%02d:EOF", pstEvent->Hrate, pstEvent->Vrate);
			}
		}
		else if (pstEvent->enKey == _KEY_J_ZOOM)
		{
			if (pstEvent->enAction == ACTION_WIDE)
			{
				sprintf(cmd, "IP-Controller:J-Z-WIDE:EOF");
			}
			else if (pstEvent->enAction == ACTION_TELE)
			{
				sprintf(cmd, "IP-Controller:J-Z-TELE:EOF");
			}
			else
			{
				sprintf(cmd, "IP-Controller:J-Z-STOP:EOF");
			}
		}
		else if (pstEvent->enKey == _KEY_J_BUTTON)
		{
			if (pstEvent->enAction == ACTION_PRESS)
			{
				sprintf(cmd, "IP-Controller:J-B-DOWN:EOF");
			}
			else
			{
				sprintf(cmd, "IP-Controller:J-B-UP:EOF");
			}
		}
	}
	else if (pstEvent->enType == TYPE_BUTTON)
	{
		if (pstEvent->enAction == ACTION_PRESS)
		{
			if (pstEvent->enKey == _KEY_ESC)
			{
				sprintf(cmd, "IP-Controller:R01-C01:EOF");
			}
			else if (pstEvent->enKey == _KEY_0)
			{
				sprintf(cmd, "IP-Controller:R02-C05:EOF");
			}
			else if (pstEvent->enKey == _KEY_1)
			{
				sprintf(cmd, "IP-Controller:R02-C06:EOF");
			}
			else if (pstEvent->enKey == _KEY_2)
			{
				sprintf(cmd, "IP-Controller:R02-C07:EOF");
			}
			else if (pstEvent->enKey == _KEY_3)
			{
				sprintf(cmd, "IP-Controller:R03-C05:EOF");
			}
			else if (pstEvent->enKey == _KEY_4)
			{
				sprintf(cmd, "IP-Controller:R03-C06:EOF");
			}
			else if (pstEvent->enKey == _KEY_5)
			{
				sprintf(cmd, "IP-Controller:R03-C07:EOF");
			}
			else if (pstEvent->enKey == _KEY_6)
			{
				sprintf(cmd, "IP-Controller:R04-C05:EOF");
			}
			else if (pstEvent->enKey == _KEY_7)
			{
				sprintf(cmd, "IP-Controller:R04-C06:EOF");
			}
			else if (pstEvent->enKey == _KEY_8)
			{
				sprintf(cmd, "IP-Controller:R04-C07:EOF");
			}
			else if (pstEvent->enKey == _KEY_9)
			{
				sprintf(cmd, "IP-Controller:R05-C06:EOF");
			}
		}
		else
		{
			sprintf(cmd, "IP-Controller:R00-C00:EOF");
		}
	}
}

static int code_is_digital(int code)
{
	if (code < BTN_TRIGGER || code > BTN_BASE4)  //BTN_TRIGGER:KEY_0; BTN_BASE4:KEY_9
		return 0;
	return 1;
}

static void js_test_show(struct input_event *input_joy)
{
#if 0
	if (input_joy->type == EV_KEY || input_joy->type == EV_ABS)
	{
		printf("input_joy==(type=%d, code=%x, value=%d)\n", 
			input_joy->type, input_joy->code, input_joy->value);
	}
#endif
}

static int js_data_handle(struct input_event *input_joy, struct event_s *pstEvent, struct js_rate_s *pstRate)
{
	switch(input_joy->type)
	{  
		case EV_KEY:
		{
			switch(input_joy->code)  
			{  
				//KEY BUTTON
				default:	
					if (code_is_digital(input_joy->code))
					{
						pstEvent->enType = TYPE_BUTTON;
						pstEvent->enKey = _KEY_0 + (input_joy->code-BTN_TRIGGER);
					}
				break;
				
				//JOY BUTTON
				case BTN_BASE5:
				{
					pstEvent->enType = TYPE_JOYSTICK;
					pstEvent->enKey = _KEY_J_BUTTON;
				}
				break;
				case BTN_BASE6:
				{
					pstEvent->enType = TYPE_BUTTON;
					pstEvent->enKey = _KEY_ESC;
				}
				break;
			}  
			
			if (input_joy->value)
				pstEvent->enAction = ACTION_PRESS;
			else
				pstEvent->enAction = ACTION_RELEASE;
		}
		break;
		case EV_ABS:  
		{  
			pstEvent->enType = TYPE_JOYSTICK;
			switch(input_joy->code)  
			{  
				case ABS_X:  
				{  
					pstEvent->enKey = _KEY_J_MOVE;
					pstRate->Hrate= data_nmlz(input_joy->value);
				}
				break;
				case ABS_Y:
				{
					pstEvent->enKey = _KEY_J_MOVE;
					pstRate->Vrate = data_nmlz(input_joy->value);
				}
				break;
				case ABS_Z:
				{  
					pstEvent->enKey = _KEY_J_ZOOM;
					pstRate->Zrate = data_nmlz(input_joy->value);
				}
				break;
			}
		}
		break;	
	}

	return 0;
}

static int js_event_parse_action(struct event_s *pstEvent, struct js_rate_s *pstRate)
{
	static int Hrate_old = 0, Vrate_old = 0, Zrate_old = 0;
	static int enAction_old = ACTION_RELEASE;
	int b_send = 0;
	
	if (pstEvent->enType == TYPE_JOYSTICK)
	{
		if (pstEvent->enKey == _KEY_J_MOVE)
		{
			J_MOVE_choose_enAction(pstEvent, pstRate->Hrate, pstRate->Vrate);
			if (pstRate->Hrate != Hrate_old || pstRate->Vrate != Vrate_old)
			{
				Hrate_old = pstRate->Hrate;
				Vrate_old = pstRate->Vrate;
				b_send = 1;
			}
		}
		else if (pstEvent->enKey == _KEY_J_ZOOM)
		{
			J_ZOOM_choose_enAction(pstEvent, pstRate->Zrate);
			if (pstRate->Zrate!=Zrate_old && !(pstRate->Zrate&&Zrate_old))
			{
				Zrate_old = pstRate->Zrate;
				b_send = 1;
			}
		}
		else if (pstEvent->enKey == _KEY_J_BUTTON)
		{
			if (pstEvent->enAction != enAction_old)
			{
				enAction_old = pstEvent->enAction;
				b_send = 1;
			}
		}
	}
	else if (pstEvent->enType == TYPE_BUTTON)
	{
		if (pstEvent->enAction != enAction_old)
		{
			enAction_old = pstEvent->enAction;
			b_send = 1;
		}
	}

	return b_send;
}

static int js_event_fd_open(char *dev_name)
{
	int fd, fd_js;
	int i;
	char name[64];
	char buf[64];
	
	for (i = 0; i < 6; i++) 
	{
        sprintf(name, "/dev/input/event%d", i);
        if ((fd = open(name, O_RDONLY, 0)) >= 0) 
		{
			ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
            if(!strcmp(buf, dev_name))
            {
            	//printf("##################[debug] dev_name:%s#################\n", dev_name);
            	close(fd);
            	fd_js = open(name, O_RDWR);
				return fd_js;
			}
            close(fd);
        }
    }
	
	return -1;
}

static int js_get_dev_name(char *dev_name)
{
	int fd;
	int ret = -1;
	char buf[64];
	
	fd = open(JS_DEV_NAME, O_RDWR);
	if (fd < 0)
	{
		printf("==fd open error==\n");
		return -1;
	}
	
	ret = ioctl(fd, JSIOCGNAME(sizeof(buf)), buf);
	if (ret < 0)
	{
		printf("==iotcl error==\n");
		close(fd);
		return -1;
	}
	close(fd);
	
	strcpy(dev_name, buf);
	
	return 0;
}

static void *msjs_server(void *argv)
{
	int pid_start = *(int *)argv;
	struct input_event input_joy;  
	struct event_s	g_jsEvent;
	struct timeval timeout;
	struct js_rate_s g_jsRate;
	char cmd[64];
	fd_set readfds;
	int  fd = -1;
	int  retval = 0;
	char b_send = 0;
	char dev_name[64];
	
	while(pid_start)
	{
		if (!access(JS_DEV_NAME, F_OK))
		{
			if (fd < 0)
			{
				retval = js_get_dev_name(dev_name);
				if (retval < 0)
				{
					sleep(1);
					continue;
				}
				fd = js_event_fd_open(dev_name);
				if (fd < 0)
				{
					sleep(1);
					continue;
				}
			}
		}
		else
		{
			if (fd > 0)
			{
				js_core_init();
				close(fd);
				fd = -1;
			}
			sleep(1);
			continue;
		}
		
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		retval = select(fd+1, &readfds, NULL, NULL, &timeout);
		if(retval==0)
			continue;
		if(FD_ISSET(fd,&readfds))
		{	 
			read(fd, &input_joy,sizeof(struct input_event));
			js_test_show(&input_joy);
			js_data_handle(&input_joy, &g_jsEvent, &g_jsRate);
			b_send = js_event_parse_action(&g_jsEvent, &g_jsRate);
			if (b_send)
			{
				b_send = 0;
				js_event_to_cmd(&g_jsEvent, cmd);
				js_cmd_handle(cmd);
			}
		}
	}
	close(fd);	
	return 0;
}

int ms_js_init()
{
	ms_sock_init(NULL, NULL, SOCKET_TYPE_JS, SOCK_MSSERVER_PATH);
	
	js_core_init();
	
	if (g_msjs_pid == 0)
	{
		g_msjs_start = 1;
		ms_task_create_join(&g_msjs_pid, msjs_server, &g_msjs_start);
	}
	
	return 0;
}

int ms_js_uninit()
{
	g_msjs_start = 0;
	ms_task_join(&g_msjs_pid);
	g_msjs_pid = 0;
	
	return 0;
}

