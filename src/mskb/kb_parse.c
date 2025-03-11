#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "msstd.h"
#include "kb_parse.h"
#include "mskb.h"

//#define UPPER_KEY

typedef struct key_map_s {
    int         row;
    int         col;
    _KEY_E       enKey;
    char        name[MAX_STR_LEN];
} _KEY_MAP_S;

static _KEY_MAP_S g_button_map[] = {
    {1,     1,      _KEY_ESC,        "ESC"},
    {1,     2,      _KEY_SET,        "SET"},
    {1,     3,      _KEY_ID,         "ID"},
    {1,     4,      _KEY_LOCK,       "LOCK"},
    {2,     1,      _KEY_T1,         "T1"},
    {2,     2,      _KEY_MENU,       "MENU"},
    {2,     3,      _KEY_MULT,       "MULT"},
    {2,     4,      _KEY_DEL,        "DEL"},
    {2,     5,      _KEY_1,          "1"},
    {2,     6,      _KEY_2,          "2"},
    {2,     7,      _KEY_3,          "3"},
    {2,     8,      _KEY_PRESET,     "PRESET"},
    {2,     9,      _KEY_FOCUS_ADD,  "FOCUS+"},
    {2,    10,      _KEY_FOCUS_SUB,  "FOCUS-"},
    {3,     1,      _KEY_T2,         "T2"},
    {3,     2,      _KEY_PREV,       "PREV"},
    {3,     3,      _KEY_SEQ,        "SEQ"},
    {3,     4,      _KEY_SNAP,       "SNAP"},
    {3,     5,      _KEY_4,          "4"},
    {3,     6,      _KEY_5,          "5"},
    {3,     7,      _KEY_6,          "6"},
    {3,     8,      _KEY_PATROL,     "PATROL"},
    {3,     9,      _KEY_ZOOM_ADD,   "ZOOM+"},
    {3,    10,      _KEY_ZOOM_SUB,   "ZOOM-"},
    {4,     1,      _KEY_A,          "A"},
    {4,     2,      _KEY_NEXT,       "NEXT"},
    {4,     3,      _KEY_REC,        "REC"},
    {4,     4,      _KEY_AUDIO,      "AUDIO"},
    {4,     5,      _KEY_7,          "7"},
    {4,     6,      _KEY_8,          "8"},
    {4,     7,      _KEY_9,          "9"},
    {4,     8,      _KEY_PATTERN,    "PATTERN"},
    {4,     9,      _KEY_IRIS_ADD,   "IRIS+"},
    {4,    10,      _KEY_IRIS_SUB,   "IRIS-"},
    {5,     1,      _KEY_AUX,        "AUX"},
    {5,     2,      _KEY_MON,        "MON"},
    {5,     3,      _KEY_WIN,        "WIN"},
    {5,     4,      _KEY_CAM,        "CAM"},
    {5,     5,      _KEY_DOT,        "."},
    {5,     6,      _KEY_0,          "0"},
    {5,     7,      _KEY_ENTER,      "OK"},
    {5,     8,      _KEY_CALL,       "CALL"},
    {5,     9,      _KEY_LIGHT,      "LIGHT"},
    {5,    10,      _KEY_WIPER,      "WIPER"},
    {-1,   -1,      _KEY_NONE,       " "},
};

static _KEY_MAP_S g_button_map_1[] = {
    {1,     1,      _KEY_ESC,        "ESC"},
    {1,     2,      _KEY_SET,        "SYS"},//SYS
    {1,     3,      _KEY_LOCK,       "LOCK"},
    {1,     4,      _KEY_DEL,        "DEL"},
    {1,     5,      _KEY_1,          "1"},
    {1,     6,      _KEY_2,          "2"},
    {1,     7,      _KEY_3,          "3"},
    {1,     8,      _KEY_PRESET,     "PRESET"},
    {1,     9,      _KEY_MENU,       "MENU"},//MENU  FOCUS+
    {1,    10,      _KEY_TOOLBAR,    "TOOLBAR"},//TOOLBAR  FOCUS-
    {2,     1,      _KEY_PREV,       "PREV"},
    {2,     2,      _KEY_NEXT,       "NEXT"},
    {2,     3,      _KEY_SEQ,        "SEQ"},
    {2,     4,      _KEY_ID,         "ID"},
    {2,     5,      _KEY_4,          "4"},
    {2,     6,      _KEY_5,          "5"},
    {2,     7,      _KEY_6,          "6"},
    {2,     8,      _KEY_PATROL,     "PATROL"},
    {2,     9,      _KEY_CAM,        "CAM"},//CAM  ZOOM+
    {2,    10,      _KEY_WIN,        "WIN"},//WIN  ZOOM-
    {3,     1,      _KEY_PLAY,       "PLAY"},
    {3,     2,      _KEY_PAUSE,      "PAUSE"},
    {3,     3,      _KEY_SNAP,       "SNAP"},
    {3,     4,      _KEY_R_CLICK,    "R_CLICK"},
    {3,     5,      _KEY_7,          "7"},
    {3,     6,      _KEY_8,          "8"},
    {3,     7,      _KEY_9,          "9"},
    {3,     8,      _KEY_PATTERN,    "PATTERN"},
    {3,     9,      _KEY_MON,        "MON"},//MON  IRIS+
    {3,    10,      _KEY_MULT,       "MULT"},//MULT  IRIS-
    {4,     1,      _KEY_STOP,       "STOP"},
    {4,     2,      _KEY_REC,        "REC"},
    {4,     3,      _KEY_AUDIO,      "AUDIO"},
    {4,     4,      _KEY_A,          "SHIFT"},//A
    {4,     5,      _KEY_DOT,        "."},
    {4,     6,      _KEY_0,          "0"},
    {4,     7,      _KEY_ENTER,      "OK"},
    {4,     8,      _KEY_CALL,       "CALL/STOP"},
    {4,     9,      _KEY_T1,         "T1"},//T1  AUTO
    {4,    10,      _KEY_T2,         "T2"},//T2  LIGHT
    {-1,   -1,      _KEY_NONE,       " "},
};

static int key_is_digital(_KEY_E enKey)
{
    if (enKey < _KEY_0 || enKey > _KEY_9) {
        return 0;
    } else {
        return 1;
    }

}

static char key_select_abc(_KEY_E enKey, int isUpper, int *index, int start)
{
    int num = enKey - _KEY_0;
    char input = 0;
#ifdef UPPER_KEY
    char abc[10][5] = { "", "", "abc", "def", "ghi", "jkl", \
                        "mno", "pqrs", "tuv", "wxyz"
                      };
#else
    char abc[10][10] = { "", "", "abcABC", "defDEF", "ghiGHI", "jklJKL", \
                         "mnoMNO", "pqrsPQRS", "tuvTUV", "wxyzWXYZ"
                       };
#endif
    if (*index >= strlen(abc[num]) || start == 1) {
        *index = 0;
    }
    input = abc[num][*index];
    (*index)++;

    return isUpper ? input - 'a' + 'A' : input;
}

static int parse_key_123_abc(_KEY_E enKey, KB_EDIT_S *pstEdit)
{
    U64 now_time = get_now_time_ms();
    char input = 0;
    int isUpper = 0;

    if (pstEdit->enEdit == EDIT_123) {
        pstEdit->str[pstEdit->count] = enKey - _KEY_0 + '0';
        pstEdit->count++;
    } else {
        isUpper = pstEdit->enEdit == EDIT_ABC ? 1 : 0;
        if (now_time - pstEdit->lastStrTime < 800 || pstEdit->lastStrTime == 0) {
            if (enKey == pstEdit->lastKey) {
                input = key_select_abc(enKey, isUpper, &pstEdit->index, 0);
                if (input) {
                    pstEdit->str[pstEdit->count - 1] = input;
                }
            } else {
                input = key_select_abc(enKey, isUpper, &pstEdit->index, 1);
                if (input) {
                    pstEdit->str[pstEdit->count] = input;
                    pstEdit->count++;
                }
            }
        } else {
            input = key_select_abc(enKey, isUpper, &pstEdit->index, 1);
            if (input) {
                pstEdit->str[pstEdit->count] = input;
                pstEdit->count++;
            }
        }
    }

    pstEdit->lastKey = enKey;
    pstEdit->lastStrTime = now_time;

    return 0;
}

static int parse_events_params(char *data, EVENT_S *pstEvent)
{
    _KEY_MAP_S *button_map;
    int num = 0;
    int intensity = 0;
    int Vrate = 0;
    int Hrate = 0;
    int row = 0;
    int col = 0;
    int i;

    pstEvent->enType = TYPE_JOYSTICK;
    if (strstr(data, "J-B")) {
        pstEvent->enKey = _KEY_J_BUTTON;
        if (strstr(data, "DOWN")) {
            strcpy(pstEvent->name, "ENTER");
            pstEvent->enAction = ACTION_PRESS;
        } else {
            strcpy(pstEvent->name, "STOP");
            pstEvent->enAction = ACTION_RELEASE;
        }
    } else if (strstr(data, "J-Z")) {
        pstEvent->enKey = _KEY_J_ZOOM;
        if (strstr(data, "WIDE")) {
            strcpy(pstEvent->name, "WIDE");
            pstEvent->enAction = ACTION_WIDE;
        } else if (strstr(data, "TELE")) {
            strcpy(pstEvent->name, "TELE");
            pstEvent->enAction = ACTION_TELE;
        } else {
            strcpy(pstEvent->name, "STOP");
            pstEvent->enAction = ACTION_STOP;
        }
    } else if (strstr(data, "J-")) {
        pstEvent->enKey = _KEY_J_MOVE;
        if (sscanf(data, "%*[^0-9]%d%*[^0-9]%d", &Hrate, &Vrate) != 2) {
            kberror("scanf failed !\n");
            return -1;
        }

        if (strstr(data, "LL")) {
            strcpy(pstEvent->name, "Left");
            pstEvent->enAction = ACTION_LL;
        } else if (strstr(data, "RR")) {
            strcpy(pstEvent->name, "Right");
            pstEvent->enAction = ACTION_RR;
        } else if (strstr(data, "UU")) {
            strcpy(pstEvent->name, "Up");
            pstEvent->enAction = ACTION_UU;
        } else if (strstr(data, "DD")) {
            strcpy(pstEvent->name, "Down");
            pstEvent->enAction = ACTION_DD;
        } else if (strstr(data, "LU")) {
            strcpy(pstEvent->name, "UpLeft");
            pstEvent->enAction = ACTION_LU;
        } else if (strstr(data, "RU")) {
            strcpy(pstEvent->name, "UpRight");
            pstEvent->enAction = ACTION_RU;
        } else if (strstr(data, "LD")) {
            strcpy(pstEvent->name, "DownLeft");
            pstEvent->enAction = ACTION_LD;
        } else if (strstr(data, "RD")) {
            strcpy(pstEvent->name, "DownRight");
            pstEvent->enAction = ACTION_RD;
        } else {
            strcpy(pstEvent->name, "STOP");
            pstEvent->enAction = ACTION_STOP;
        }

        pstEvent->Hrate = Hrate;
        pstEvent->Vrate = Vrate;
    } else if (strstr(data, "S-I")) { //·ÉËó ÄÚÈ¦
        pstEvent->enType = TYPE_SHUTTLE;
        pstEvent->enKey = _KEY_SHUTTLE_I;
        if (strstr(data, "CCW")) { //ÄæÊ±Õë
            strcpy(pstEvent->name, "S-I-CCW");
            pstEvent->enAction = ACTION_CCW;
        } else if (strstr(data, "CW")) {
            strcpy(pstEvent->name, "S-I-CW");
            pstEvent->enAction = ACTION_CW;
        }
    } else if (strstr(data, "S-O")) { //·ÉËó ÍâÈ¦
        pstEvent->enType = TYPE_SHUTTLE;
        pstEvent->enKey = _KEY_SHUTTLE_O;

        if (sscanf(data, "%*[^0-9]%d", &intensity) != 1) {
            kberror("scanf failed !\n");
            return -1;
        }

        if (strstr(data, "CCW")) { //ÄæÊ±Õë
            strcpy(pstEvent->name, "S-O-CCW");
            pstEvent->enAction = ACTION_CCW;
        } else if (strstr(data, "CW")) {
            strcpy(pstEvent->name, "S-O-CW");
            pstEvent->enAction = ACTION_CW;
        } else {
            strcpy(pstEvent->name, "STOP");
            pstEvent->enAction = ACTION_STOP;
        }
        pstEvent->Intensity = intensity;
    } else {
        pstEvent->enType = TYPE_BUTTON;

        if (sscanf(data, "%*[^0-9]%d%*[^0-9]%d", &row, &col) != 2) {
            kberror("scanf failed !\n");
            pstEvent->enAction = ACTION_RELEASE;
            return -1;
        }

        if (g_kb_version == 1) {
            button_map = g_button_map_1;
            num = ARRAY_SIZE(g_button_map_1);
        } else {
            button_map = g_button_map;
            num = ARRAY_SIZE(g_button_map);
        }

        for (i = 0; i < num; i++) {
            if (button_map[i].col == col && button_map[i].row == row) {
                strcpy(pstEvent->name, button_map[i].name);
                pstEvent->enKey = button_map[i].enKey;
                pstEvent->enAction = ACTION_PRESS;
                return 0;
            }
        }

        pstEvent->enAction = ACTION_RELEASE;
    }

    return 0;
}

int kb_parse_string(char *data, EVENT_S *pstEvent)
{
    if (!data) {
        return -1;
    }
    int ret = 0;

    if (strstr(data, "HeartbeatACK")) {
        pstEvent->enType = TYPE_HB_ACK;
    } else if (strstr(data, "Heartbeat")) {
        pstEvent->enType = TYPE_HB_REQ;
    } else if (strstr(data, "LocalSetup-ON")) {
        pstEvent->enType = TYPE_KB_SET;
    } else if (strstr(data, "LocalSetup-OFF")) {
        pstEvent->enType = TYPE_KB_UNLOCK;
    } else if (strstr(data, "Unlock")) {
        pstEvent->enType = TYPE_KB_UNLOCK;
    } else if (strstr(data, "Lock")) {
        pstEvent->enType = TYPE_KB_LOCK;
    } else if (strstr(data, "VERSIONS-E4")) {
        g_kb_version = 1;
    } else {
        ret = parse_events_params(data, pstEvent);
    }
    return ret;
}

int editor_insert_string(_KEY_E enKey, KB_EDIT_S *pstEdit)
{
    int strLimit = 32;
    if (!pstEdit->isON) {
        return 0;
    }

    if ((enKey == _KEY_A) && !pstEdit->isLock) {
#ifdef UPPER_KEY
        pstEdit->enEdit++;
        if (pstEdit->enEdit ==  EDIT_END) {
            pstEdit->enEdit = EDIT_123;
        }
#else
        pstEdit->enEdit = !pstEdit->enEdit;
#endif
    } else if (pstEdit->isLock) {
        strLimit = 16;
    }
    if (!key_is_digital(enKey) && enKey != _KEY_DEL && enKey != _KEY_DOT) {
        return -1;
    }

    if (enKey != _KEY_DEL && enKey != _KEY_DOT)

    {
        parse_key_123_abc(enKey, pstEdit);
    } else if (enKey == _KEY_DEL) {
        pstEdit->count = pstEdit->count > 0 ? pstEdit->count - 1 : 0;
    }
    //add by shea for input '.'
    else if (enKey == _KEY_DOT) {
        pstEdit->str[pstEdit->count] = '.';
        pstEdit->count++;
    }

    if (pstEdit->count > strLimit) {
        pstEdit->count = strLimit;
    }
    pstEdit->str[pstEdit->count] = '\0';

    return 0;

}

void editor_clear_string(KB_EDIT_S *pstEdit)
{
    pstEdit->count = 0;
    memset(pstEdit->str, '\0', sizeof(pstEdit->str));
}

