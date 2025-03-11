/* 
 * ***************************************************************
 * Filename:      	js_parse.h
 * Created at:    	2017.01.17
 * Description:   	milesight keyboard protocol parse.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#ifndef __JS_PARSE_H__
#define __JS_PARSE_H__

#include "js_util.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STR_LEN     (48)
#define MAX_KEY_COUNT   (16)

#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))

typedef struct js_edit_s{
    int           isON;
    int           index;
    int           count;
    char          str[MAX_STR_LEN];
}JS_EDIT_S;


typedef enum key_e{
//NVR
    _KEY_NONE =0,
    _KEY_ESC = 1,
    _KEY_SET,
    _KEY_ID,
    _KEY_LOCK,
    _KEY_T1,
    _KEY_T2,
    _KEY_MENU,
    _KEY_MULT,
    _KEY_DEL,
    _KEY_PREV,
    _KEY_NEXT,
    _KEY_SEQ,
    _KEY_SNAP,
    _KEY_REC,
    _KEY_AUDIO,
    _KEY_MON,
    _KEY_WIN,
    _KEY_CAM,
    _KEY_A,
    _KEY_AUX,
    _KEY_0,
    _KEY_1,
    _KEY_2,
    _KEY_3,
    _KEY_4,
    _KEY_5,
    _KEY_6,
    _KEY_7,
    _KEY_8,
    _KEY_9,
    _KEY_DOT,
    _KEY_ENTER,
//PTZ
    _KEY_PRESET,
    _KEY_PATROL,
    _KEY_PATTERN,
    _KEY_CALL,
    _KEY_FOCUS_ADD,
    _KEY_FOCUS_SUB,
    _KEY_ZOOM_ADD,
    _KEY_ZOOM_SUB,
    _KEY_IRIS_ADD,
    _KEY_IRIS_SUB,
    _KEY_LIGHT,
    _KEY_WIPER,
    _KEY_J_BUTTON,
    _KEY_J_ZOOM,
    _KEY_J_MOVE,
//other
    _KEY_LOGIN,
}_KEY_E;

typedef enum js_action_e{
    ACTION_RELEASE = 0,
    ACTION_PRESS,    
    ACTION_LL,
    ACTION_RR,
    ACTION_UU,
    ACTION_DD,
    ACTION_LU,
    ACTION_RU,
    ACTION_LD,
    ACTION_RD,    
    ACTION_WIDE,
    ACTION_TELE,    
    ACTION_STOP,
}JS_ACTION_E;

typedef enum js_type_e{
    TYPE_UNKOWN = 0,
    TYPE_BUTTON,
    TYPE_JOYSTICK,
    TYPE_NUM,
}JS_TYPE_E;

typedef struct event_s{
	char        name[MAX_STR_LEN];
    JS_TYPE_E   enType;
    _KEY_E      enKey;
    JS_ACTION_E enAction;
    int         Hrate;
    int         Vrate;
	int 		Zrate;
}EVENT_S;

int js_parse_string(char *data, EVENT_S *pstEvent);



#ifdef __cplusplus
}
#endif

#endif


