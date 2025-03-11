/*
 * ***************************************************************
 * Filename:        kb_parse.h
 * Created at:      2017.01.17
 * Description:     milesight keyboard protocol parse.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __KB_PARSE_H__
#define __KB_PARSE_H__

#include "kb_util.h"
#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STR_LEN     (48)
#define MAX_KEY_COUNT   (16)

#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))

#define LINE_DATA_LEN   (16)
#define LINE_DATA_POS   (10)
#define LINE_PW_LEN     (16)

typedef enum line_e {
    LINE_BEGIN = 1,
    LINE_TITLE = LINE_BEGIN,
    LINE_MODE,
    LINE_DATA,
    LINE_KEY,
    LINE_END,
} LINE_E;

typedef enum key_e {
//NVR
    _KEY_NONE = 0,
    _KEY_ESC = 1,
    _KEY_SET,
    _KEY_ID,
    _KEY_LOCK,
    _KEY_T1,
    _KEY_T2,
    _KEY_MENU,
    _KEY_MULT,
    _KEY_DEL,
    _KEY_PREV,//10
    _KEY_NEXT,
    _KEY_SEQ,
    _KEY_SNAP,
    _KEY_REC,
    _KEY_AUDIO,
    _KEY_MON,
    _KEY_WIN,
    _KEY_CAM,
    _KEY_A,
    _KEY_AUX,//20
    _KEY_0,
    _KEY_1,
    _KEY_2,
    _KEY_3,
    _KEY_4,
    _KEY_5,
    _KEY_6,
    _KEY_7,
    _KEY_8,
    _KEY_9,//30
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
    _KEY_ZOOM_SUB,//40
    _KEY_IRIS_ADD,
    _KEY_IRIS_SUB,
    _KEY_LIGHT,
    _KEY_WIPER,
    _KEY_J_BUTTON,
    _KEY_J_ZOOM,
    _KEY_J_MOVE,
//other
    _KEY_LOGIN,
//new version
    _KEY_TOOLBAR,
    _KEY_PLAY,//50
    _KEY_PAUSE,
    _KEY_STOP,
    _KEY_R_CLICK,
    _KEY_AUTO,
    _KEY_SHUTTLE_I,
    _KEY_SHUTTLE_O,
    _KEY_PTZ_STOP,// CALL/STOP  Virtual buttons     stop patrol or pattern
    _KEY_PTZ_DEL,//del  Virtual buttons      delete patrol or pattern
} _KEY_E;

typedef enum kb_action_e {
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
    ACTION_CCW,//anticlockwise
    ACTION_CW,//clockwise
} KB_ACTION_E;

typedef enum kb_type_e {
    TYPE_UNKOWN = 0,
    TYPE_HB_REQ,
    TYPE_HB_ACK,
    TYPE_KB_SET,
    TYPE_KB_LOCK,
    TYPE_KB_UNLOCK,
    TYPE_BUTTON,
    TYPE_JOYSTICK,
    TYPE_SHUTTLE,
    TYPE_NUM,
} KB_TYPE_E;

typedef struct event_s {
    char        name[MAX_STR_LEN];
    char        dsp[MAX_STR_LEN];
    KB_TYPE_E   enType;
    _KEY_E       enKey;
    KB_ACTION_E enAction;
    int         Hrate;
    int         Vrate;
    int         Intensity;
} EVENT_S;

typedef enum kb_edit_e {
    EDIT_123 = 0,
    EDIT_abc,
    EDIT_ABC,
    EDIT_END,
} KB_EDIT_E;

typedef struct kb_edit_s {
    int           isON;
    int           isLock;
    KB_EDIT_E     enEdit;
    int           index;
    U64           lastStrTime;
    _KEY_E         lastKey;
    int           count;
    char          str[MAX_STR_LEN];
} KB_EDIT_S;

int kb_parse_string(char *data, EVENT_S *pstEvent);

int editor_insert_string(_KEY_E enKey, KB_EDIT_S *pstEdit);

void editor_clear_string(KB_EDIT_S *pstEdit);


#ifdef __cplusplus
}
#endif

#endif


