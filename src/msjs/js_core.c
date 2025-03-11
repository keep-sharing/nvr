#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include "msstd.h"
#include "mssocket.h"
#include "js_parse.h"
#include "msg.h"
#include "msjs.h"

// unsigned int global_debug_mask = 0;
// hook_print global_debug_hook = 0;

typedef struct st_timer
{
	U64	start;
	U64	end;
} ST_TIMER;

typedef enum js_mode_e{
    MODE_NVR = 0,
    MODE_PTZ,
}JS_MODE_E;

typedef struct key_func_s{
    int     count; // ���ϼ��İ�������
    int     input; //�ڼ���������Ҫ��������
    _KEY_E   enKey[MAX_KEY_COUNT];
    int     (*func)(int cmd, void *argv);
}KEY_FUNC_S;

typedef struct client_s{
    JS_MODE_E  enMode;
    int        keyCount;
    _KEY_E      enKey[MAX_KEY_COUNT];
	U64        lastKeyTime;
    EVENT_S    stEvent;
	JS_EDIT_S  stEditor;
	
}CLIENT_S;

static CLIENT_S g_stClient;
static ST_TIMER g_stTimer;
int g_jsdebug = 0;
//----------------------------------------------
// keyboard ctrl api
//----------------------------------------------
static int js_ctrl_is_ready(CLIENT_S *pstClient)
{
    if (pstClient->stEvent.enAction == ACTION_RELEASE)
    {
        return 0;
    }

    return 1;
}

static int key_is_digital(_KEY_E enKey)
{
	if (enKey < _KEY_0 || enKey > _KEY_9)
		return 0;
	
	return 1;
}

static int parse_key_123(_KEY_E enKey, JS_EDIT_S *pstEdit)
{
   pstEdit->str[pstEdit->count] = enKey - _KEY_0 + '0';
   pstEdit->count++;

   return 0;
}

static void editor_clear()
{
	memset(&g_stClient.stEditor, 0, sizeof(JS_EDIT_S));
	g_stTimer.start = g_stTimer.end;
}

static void js_ctrl_set_mode(CLIENT_S *pstClient, JS_MODE_E enMode)
{
    pstClient->enMode = enMode;
}

static int send_msg_to_nvr(int sendto, int type, void* data, int size)
{
    return ms_sock_send_msg(sendto, type, data, size, 0);
}

static void js_send_func_cmd(char *cmd)
{
    char tmp[100] = {0};
	
	snprintf(tmp, sizeof(tmp), "%s", cmd);
    send_msg_to_nvr(SOCKET_TYPE_CORE, REQUEST_FLAG_SENDTO_QT_CMD, tmp, sizeof(tmp));
}


static int func_jmove(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    static int isDone = 0;
    int isMove = 0;
    char tmp[64];
	
    isMove = pstClient->stEvent.enAction != ACTION_STOP ? 1 : 0;
    if (pstClient->enMode == MODE_NVR)
    {
        if (!isMove)
        {
            isDone = 0;
        }
        if (!isDone)
        {
            sprintf(tmp, "Dir_Nvr_%s_%d_%d", pstClient->stEvent.name, \
                    pstClient->stEvent.Hrate, pstClient->stEvent.Vrate);
			
            isDone = isMove;
	    	js_send_func_cmd(tmp); 
			
        }
        else
        {
            return cmd;
        }
    }
	else if (pstClient->enMode == MODE_PTZ)
	{
		if (!isMove)
        {
            isDone = 0;
        }
        if (!isDone)
        {
			sprintf(tmp, "Dir_Ptz_%s_%d_%d", pstClient->stEvent.name, \
        	pstClient->stEvent.Hrate, pstClient->stEvent.Vrate);
			js_send_func_cmd(tmp);	
		}

		else
        {
            return cmd;
        }
	}
	
    return cmd;
}


static int func_jzoom(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    static JS_ACTION_E enAction;
	
    if (pstClient->stEvent.enAction == ACTION_TELE)
    {
        js_send_func_cmd("PTZ_ZoomPlus");
    }
    else if (pstClient->stEvent.enAction == ACTION_WIDE)
    {
        js_send_func_cmd("PTZ_ZoomMinus");
    }
    else
    {
        if (enAction == ACTION_TELE)
        {
            js_send_func_cmd("PTZ_ZoomPlusStop");
        }
        else if (enAction == ACTION_WIDE)
        {
            js_send_func_cmd("PTZ_ZoomMinusStop");
        }

    }
    enAction = pstClient->stEvent.enAction;

    return cmd;
}

static int func_call_preset(JS_EDIT_S  *pstEditor)
{
	char tmp[MAX_STR_LEN];
	
	sprintf(tmp, "PTZ_Preset_Call_%d", atoi(pstEditor->str));
    js_send_func_cmd(tmp);
	
	return 0;
}

static int func_jbutton(int cmd, void *argv)
{
	CLIENT_S *pstClient = (CLIENT_S *)argv;
    U64 now_time = get_now_time_ms();
	static JS_MODE_E js_mode = MODE_NVR;
	
	if (!js_ctrl_is_ready(pstClient))
	{
		return 0;
	}
	
	printf("stEditor.isON=%d\n", pstClient->stEditor.isON);
	if (pstClient->stEditor.isON)
	{
		func_call_preset(&pstClient->stEditor);
		editor_clear();
		return 0;
	}
	
    if (now_time - pstClient->lastKeyTime > 500)
    {
		js_mode = pstClient->enMode;
        js_send_func_cmd("Enter");
		js_ctrl_set_mode(pstClient, MODE_PTZ);
    }
    else
    {
		js_ctrl_set_mode(pstClient, js_mode);
        js_send_func_cmd("FullScreen");
    }
	
    pstClient->lastKeyTime = now_time;

    return cmd;
}

static int func_esc(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!js_ctrl_is_ready(pstClient))
    {
        return cmd;
    }
	
    js_ctrl_set_mode(pstClient, MODE_NVR);
	
    return cmd;
}

static KEY_FUNC_S g_key_func[] = {
    {1, 1, {_KEY_J_MOVE}, func_jmove},
    {1, 1, {_KEY_J_ZOOM}, func_jzoom},
    {1, 1, {_KEY_J_BUTTON}, func_jbutton},
	{1, 1, {_KEY_ESC}, func_esc},
    {0, 0, {0}, NULL},
};

static int key_func_match(CLIENT_S *pstClient)
{
    int i, j;
    int ret = 0;
    int match_key_num = 0;
    int num = ARRAY_SIZE(g_key_func);
    int count = 0;
    
    count = pstClient->keyCount?pstClient->keyCount : 1;
    
    for (i = 0; i < num; i++)
    {
        for (j = 0; j < count;)
        {
            if (g_key_func[i].enKey[j] != pstClient->enKey[j])
            {
				break;
            }
			j++;
            match_key_num = ms_max(match_key_num, j);
            if (j == g_key_func[i].input || j == g_key_func[i].count)
            {
                if (g_key_func[i].func)
                {
                    ret = g_key_func[i].func(g_key_func[i].count - j, pstClient);
                }
                if (ret == 0)
                {
                    return ret;
                }
            }
        }
    }
    return match_key_num;
}

int js_cmd_handle(char *data)
{
//	printf("recv_msg_from_kb:[%d] %s\n", strlen(data), (char *)data);
	if (g_stClient.stEditor.isON)
	{
		g_stTimer.end = get_now_time_ms();
		if (g_stTimer.end - g_stTimer.start > 10000) //10s
		{
			printf("==time is longer than 10s, clear it==");
			editor_clear();
		}
	}
	
	js_parse_string(data, &g_stClient.stEvent);
	if (g_stClient.stEvent.enAction != ACTION_RELEASE)
	{
		if (key_is_digital(g_stClient.stEvent.enKey))
		{
			g_stClient.stEditor.isON = 1;
			g_stTimer.start = get_now_time_ms();
			parse_key_123(g_stClient.stEvent.enKey, &g_stClient.stEditor);
		}
		else if (g_stClient.stEditor.isON)
		{
			if (g_stClient.stEvent.enKey != _KEY_J_BUTTON)
			{
				editor_clear();
			}
		}
		
		g_stClient.enKey[g_stClient.keyCount] = g_stClient.stEvent.enKey;
		if (g_stClient.enKey[g_stClient.keyCount] == _KEY_ESC)
		{
			g_stClient.keyCount = 1;
			g_stClient.enKey[0] = _KEY_ESC;
		}
		else
		{
			g_stClient.keyCount++;
		}
		
		g_stClient.keyCount = key_func_match(&g_stClient);
	}
	else
	{
		key_func_match(&g_stClient);
	}
	
    return 0;
}

int js_core_init()
{
	memset(&g_stClient, 0, sizeof(CLIENT_S));
	g_stClient.enMode = MODE_NVR;
	
	return 0;
}
