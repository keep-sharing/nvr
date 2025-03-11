#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "msstd.h"
#include "js_parse.h"
#include "msjs.h"

//#define UPPER_KEY

typedef struct key_map_s{
    int         row;
    int         col;
    _KEY_E       enKey;
    char        name[MAX_STR_LEN];
}_KEY_MAP_S;

static _KEY_MAP_S g_button_map[] = {
    {1,     1,      _KEY_ESC,        "ESC"},
		
    {2,     5,      _KEY_1,          "1"},
    {2,     6,      _KEY_2,          "2"},
    {2,     7,      _KEY_3,          "3"},
	
    {3,     5,      _KEY_4,          "4"},
    {3,     6,      _KEY_5,          "5"},
    {3,     7,      _KEY_6,          "6"},

    {4,     5,      _KEY_7,          "7"},
    {4,     6,      _KEY_8,          "8"},
    {4,     7,      _KEY_9,          "9"},

    {5,     6,      _KEY_0,          "0"},

    {-1,   -1,      _KEY_NONE,       " "},
};

static int parse_events_params(char *data, EVENT_S *pstEvent)
{
    int Vrate = 0;
    int Hrate = 0;
    int row = 0;
    int col = 0;
    int i;

    pstEvent->enType = TYPE_JOYSTICK;
    if (strstr(data, "J-B"))
    {
        pstEvent->enKey = _KEY_J_BUTTON;
        if (strstr(data, "DOWN"))
        {
            strcpy(pstEvent->name, "ENTER");
            pstEvent->enAction = ACTION_PRESS;
        }
        else
        {
            strcpy(pstEvent->name, "STOP");
            pstEvent->enAction = ACTION_RELEASE;
        }
    }
    else if (strstr(data, "J-Z"))
    {
        pstEvent->enKey = _KEY_J_ZOOM;
        if (strstr(data, "WIDE"))
        {
            strcpy(pstEvent->name, "WIDE");
            pstEvent->enAction = ACTION_WIDE;
        }
        else if (strstr(data, "TELE"))
        {
            strcpy(pstEvent->name, "TELE");
            pstEvent->enAction = ACTION_TELE;
        }
        else
        {
            strcpy(pstEvent->name, "STOP");
            pstEvent->enAction = ACTION_STOP;
        }
    }
    else if (strstr(data, "J-"))
    {
        pstEvent->enKey = _KEY_J_MOVE;
        if (sscanf(data, "%*[^0-9]%d%*[^0-9]%d", &Hrate, &Vrate) != 2)
        {
            jserror("scanf failed !\n");    
            return -1;
        }
                
        if (strstr(data, "LL"))
        {
            strcpy(pstEvent->name, "Left");
            pstEvent->enAction = ACTION_LL;
        }
        else if (strstr(data, "RR"))
        {
            strcpy(pstEvent->name, "Right");
            pstEvent->enAction = ACTION_RR;
        }
        else if (strstr(data, "UU"))
        {
            strcpy(pstEvent->name, "Up");
            pstEvent->enAction = ACTION_UU;
        }
        else if (strstr(data, "DD"))
        {
            strcpy(pstEvent->name, "Down");
            pstEvent->enAction = ACTION_DD;
        }
        else if (strstr(data, "LU"))
        {
            strcpy(pstEvent->name, "UpLeft");
            pstEvent->enAction = ACTION_LU;
        }
        else if (strstr(data, "RU"))
        {
            strcpy(pstEvent->name, "UpRight");
            pstEvent->enAction = ACTION_RU;
        }
        else if (strstr(data, "LD"))
        {
            strcpy(pstEvent->name, "DownLeft");
            pstEvent->enAction = ACTION_LD;
        }
        else if (strstr(data, "RD"))
        {
            strcpy(pstEvent->name, "DownRight");
            pstEvent->enAction = ACTION_RD;
        }
        else
        {
            strcpy(pstEvent->name, "STOP");
            pstEvent->enAction = ACTION_STOP;
        }
		
        pstEvent->Hrate = Hrate;
        pstEvent->Vrate = Vrate;
    }
    else
    {
        pstEvent->enType = TYPE_BUTTON;
        
        if (sscanf(data, "%*[^0-9]%d%*[^0-9]%d", &row, &col) != 2)
        {
            jserror("scanf failed !\n");    
            pstEvent->enAction = ACTION_RELEASE;
            return -1;
        }
            
        for (i = 0; i < ARRAY_SIZE(g_button_map); i++)
        {
            if (g_button_map[i].col == col && g_button_map[i].row == row)
            {
                strcpy(pstEvent->name, g_button_map[i].name);
                pstEvent->enKey = g_button_map[i].enKey;
                pstEvent->enAction = ACTION_PRESS;
                return 0;
            }
        }
        pstEvent->enAction = ACTION_RELEASE;
    }
	
    return 0;
}

int js_parse_string(char *data, EVENT_S *pstEvent)
{
    int ret = 0;
	
    ret = parse_events_params(data, pstEvent);
    
    return ret;
}
