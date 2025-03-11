#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>


#include "msdb.h"
#include "msstd.h"
#include "mssocket.h"
#include "kb_socket.h"
#include "kb_parse.h"
#include "mskb.h"
#include "msg.h"


#define SQLITE_FILE_NAME        "/mnt/nand/app/msdb.db"

// unsigned int global_debug_mask = 0;
// hook_print global_debug_hook = 0;
struct device_info device = {0};
int mode = 0;
int g_kb_version;// 0: previous versions ;   1:new version

typedef enum kb_mode_e {
    MODE_NVR = 0,
    MODE_PTZ,
} KB_MODE_E;

typedef struct key_func_s {
    int     count; // ���ϼ��İ�������
    int     input; //�ڼ���������Ҫ��������
    _KEY_E   enKey[MAX_KEY_COUNT];
    int (*func)(int cmd, void *argv);
} KEY_FUNC_S;

typedef struct client_s {
    SOCKET     fd;
    KB_MODE_E  enMode;
    U64        lastKeyTime;
    int        keyCount;
    _KEY_E     enKey[MAX_KEY_COUNT];
    int        isLock;
    int        isLogin;
    KB_EDIT_S  stEditor;
    EVENT_S    stEvent;
    pthread_t  pthread;
    int        isRun;
    int        heartbeat;
    char       ipaddr[64];
    char       password[64];
    int        version;
    int        psw_modified;
} CLIENT_S;

typedef struct login_device_info {
    int idx;
    char ipaddr[64];
    char password[64];
} LOGIN_DEVICE_INFO;

typedef struct server_s {
    struct get_user_info    stUser;
    CLIENT_S                *pClientList[MAX_CLIENT];
} SERVER_S;

static SERVER_S g_stSever;
static CLIENT_S *g_pstClient = NULL;
static struct get_qt_cmd_result g_sQtStatus;
int g_kbdebug = 0;
int g_kb_heartbeat = 0;

LOGIN_DEVICE_INFO g_device_list[MAX_CLIENT];
struct get_network_keyboard_info g_db_device_list;

//----------------------------------------------
// keyboard ctrl api
//----------------------------------------------

static int kb_get_nvr_user_info()
{
    int count = 20;

    while (count--) {
        ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_USER_INFO, NULL, 0, 0);
        if (g_stSever.stUser.cnt != 0) {
            usleep(100 * 1000);
            return 0;
        }
        usleep(100 * 1000);
    }
    kberror("faild \n");
    return -1;
}

#if 1
static int is_online_device_list(char *ipaddr)
{
    int i = 0, n = 0;
    if (ipaddr[0] == '\0') {
        return -1;
    }
    int count = 20;

    while (count--) {
        ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_GET_NETWORK_KEYBOARD, NULL, 0, 0);
        if (g_db_device_list.keyboardCount != 0) {
            for (i = 0; i < g_db_device_list.keyboardCount; i++) {
                if (g_db_device_list.keyboards[i].ip_addr[0] != '\0' && !strcmp(g_db_device_list.keyboards[i].ip_addr, ipaddr)) {
                    if (g_db_device_list.keyboards[i].password[0] == '\0') {
                        continue;
                    }
                    if (kb_get_nvr_user_info() != -1) {
                        for (n = 0; n < g_stSever.stUser.cnt; n++) {
                            if (!strcmp(g_db_device_list.keyboards[i].password, g_stSever.stUser.user[n].passwd)) {
                                return 0;
                            }
                        }
                    }
                }
            }
            usleep(100 * 1000);
            return -1;
        }
        usleep(100 * 1000);
    }
    return -1;
}

static int insert_to_device_list(char *ipaddr, char *password)
{
    if (ipaddr[0] == '\0' || password[0] == '\0') {
        return -1;
    }
    if (!is_online_device_list(ipaddr)) {
        return -1;
    }
    struct req_network_keyboard keyboardData;
    memset(&keyboardData, 0 , sizeof(struct req_network_keyboard));
    snprintf(keyboardData.ip_addr, sizeof(keyboardData.ip_addr), "%s", ipaddr);
    snprintf(keyboardData.password, sizeof(keyboardData.password), "%s", password);
    ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_ADD_NETWORK_KEYBOARD, &keyboardData, sizeof(struct req_network_keyboard), 0);
    usleep(100 * 1000);
    return 0;
}

static int get_online_device_pw(CLIENT_S *pstClient)
{
    if (!pstClient->isLogin) {
        return -1;
    }
    int i;
    if (g_db_device_list.keyboardCount != 0) {
        for (i = 0; i < g_db_device_list.keyboardCount; i++) {
            if (g_db_device_list.keyboards[i].ip_addr[0] != '\0' && !strcmp(g_db_device_list.keyboards[i].ip_addr, pstClient->ipaddr)) {
                if (g_db_device_list.keyboards[i].password[0] == '\0') {
                    continue;
                }
                strcpy(pstClient->password, g_db_device_list.keyboards[i].password);
                return 0;
            }
        }
    }
    return -1;
}

#else
static int is_online_device_list(char *ipaddr)
{
    int i = 0, n = 0;
    if (ipaddr[0] == '\0') {
        return -1;
    }
    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_device_list[i].ipaddr[0] != '\0' && !strcmp(g_device_list[i].ipaddr, ipaddr)) {
            if (g_device_list[i].password[0] == '\0') {
                continue;
            }
            if (kb_get_nvr_user_info() != -1) {
                for (n = 0; n < g_stSever.stUser.cnt; n++) {
                    if (!strcmp(g_device_list[i].password, g_stSever.stUser.user[n].passwd)) {
                        return 0;
                    }
                }
            }
        }
    }

    return -1;
}

static int insert_to_device_list(char *ipaddr, char *password)
{
    int i = 0;
    if (ipaddr[0] == '\0' || password[0] == '\0') {
        return -1;
    }
    if (!is_online_device_list(ipaddr)) {
        return -1;
    }
    for (i = 0 ; i < MAX_CLIENT; i++) {
        if (g_device_list[i].ipaddr[0] == '\0' || !strcmp(g_device_list[i].ipaddr, g_pstClient->ipaddr)) {
            g_device_list[i].idx = i;
            snprintf(g_device_list[i].ipaddr, sizeof(g_device_list[i].ipaddr), "%s", ipaddr);
            snprintf(g_device_list[i].password, sizeof(g_device_list[i].password), "%s", password);
            //strcpy(g_device_list[i].password, password);
            return 0;
        }
    }

    return -1;
}
#endif

static int delete_to_device(char *ipaddr)
{
    if (ipaddr[0] == '\0') {
        return -1;
    }

    struct req_network_keyboard keyboardData;
    memset(&keyboardData, 0 , sizeof(struct req_network_keyboard));
    snprintf(keyboardData.ip_addr, sizeof(keyboardData.ip_addr), "%s", ipaddr);
    ms_sock_send_msg(SOCKET_TYPE_CORE, REQUEST_FLAG_DELETE_NETWORK_KEYBOARD, &keyboardData, sizeof(struct req_network_keyboard), 0);
    usleep(100 * 1000);
    return 0;
}

static int kb_ctrl_delete_client(SOCKET fd)
{
    int i;

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_stSever.pClientList[i] != NULL) {
            if (g_stSever.pClientList[i]->fd == fd) {
                if (g_stSever.pClientList[i]->isRun) {
                    g_stSever.pClientList[i]->isRun = 0;
                    ms_task_join(&g_stSever.pClientList[i]->pthread);
                }
                free(g_stSever.pClientList[i]);
                g_stSever.pClientList[i] = NULL;
                kbdebug("=[%d]==kb_ctrl_delete_client=%d==\n", i, fd);
                return 0;
            }
        }
    }
    return -1;
}

static void *kb_heartbeat_pthread(void *argv)
{
//    SERVER_S *pstServer = (SERVER_S *)argv;
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    int count = 0;
    //char cmd[] = "Command:Heartbeat:EOF";
    ms_task_set_name("kb_heartbeat_pthread");

    while (pstClient->isRun) {
        usleep(300000);
        count++;
        if (count == 10) {
            count = 0;
            if (pstClient->heartbeat < 5) {
                // kb_socket_send_msg(pstClient->fd, cmd, sizeof(cmd));
                //pstClient->heartbeat++;
            } else {
                //pstClient->isRun = 0;
            }
        }
    }

    kb_msg_to_display(pstClient->fd, "Disconnected", LINE_MODE, 3, 16);
    kb_socket_client_close(pstClient->fd);
    kb_ctrl_delete_client(pstClient->fd);

    return 0;
}

static CLIENT_S *kb_ctrl_get_client(SOCKET fd)
{
    int i;

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_stSever.pClientList[i] != NULL) {
            if (g_stSever.pClientList[i]->fd == fd) {
                g_pstClient = g_stSever.pClientList[i];
                return g_pstClient;
            }
        }
    }
    return NULL;
}

static int kb_ctrl_save_client(SOCKET fd, char *ipaddr)
{
    int i;

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_stSever.pClientList[i] == NULL) {
            g_stSever.pClientList[i] = malloc(sizeof(CLIENT_S));
            if (g_stSever.pClientList[i] == NULL) {
                kberror("no memory !\n");
                return -1;
            }
            g_pstClient = g_stSever.pClientList[i];
            memset(g_pstClient, 0, sizeof(CLIENT_S));
            g_stSever.pClientList[i]->fd = fd;
            if (ipaddr[0] != '\0') {
                snprintf(g_stSever.pClientList[i]->ipaddr, sizeof(g_stSever.pClientList[i]->ipaddr), "%s", ipaddr);
            }
            g_pstClient->isRun = 1;
            g_pstClient->version = g_kb_version;
            g_pstClient->psw_modified = 0;
            ms_task_create_join(&g_pstClient->pthread, kb_heartbeat_pthread, g_pstClient);
            kbdebug("=[%d]===kb_ctrl_save_client=%d==\n", i, fd);
            return i;
        }
    }
    return -1;
}

static void kb_ctrl_set_mode(CLIENT_S *pstClient, KB_MODE_E enMode)
{
    char tmp[32];

    if (pstClient->stEditor.isON) {
        return ;
    }
    if (enMode == MODE_NVR) {
        mode = 1;
    } else {
        mode = 2;
    }
    sprintf(tmp, "Mode: %s", enMode == MODE_NVR ? "NVR" : "PTZ");
    kb_msg_to_display(pstClient->fd, tmp, LINE_MODE, 1, 16);
    pstClient->enMode = enMode;
    pstClient->stEditor.isLock = 0;
}


static void kb_ctrl_123_abc(KB_EDIT_E enEdit, int isLock)
{
    if (!g_pstClient->stEditor.isON) {
        return ;
    }

    switch (enEdit) {
        case EDIT_123:
            kb_msg_to_display(g_pstClient->fd, "Mode: 123", LINE_MODE, 1, 16);
            mode = 0;
            break;
        case EDIT_abc:
            kb_msg_to_display(g_pstClient->fd, "Mode: abc", LINE_MODE, 1, 16);
            mode = 3;
            break;
        case EDIT_ABC:
            kb_msg_to_display(g_pstClient->fd, "Mode: ABC", LINE_MODE, 1, 16);
            break;
        default:
            return ;
    }

    g_pstClient->stEditor.enEdit = enEdit;
    g_pstClient->stEditor.isLock = isLock;
}

static void kb_ctrl_nvr_ptz(EVENT_S *pstEvent)
{
    if (mode != 2) {
        return ;
    }

    int key = pstEvent->enKey;

    if (key == _KEY_MENU) {
        pstEvent->enKey = _KEY_FOCUS_ADD;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "FOCUS+");
    } else if (key == _KEY_TOOLBAR) {
        pstEvent->enKey = _KEY_FOCUS_SUB;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "FOCUS-");
    } else if (key == _KEY_CAM) {
        pstEvent->enKey = _KEY_ZOOM_ADD;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "ZOOM+");
    } else if (key == _KEY_WIN) {
        pstEvent->enKey = _KEY_ZOOM_SUB;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "ZOOM-");
    } else if (key == _KEY_MON) {
        pstEvent->enKey = _KEY_IRIS_ADD;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "IRIS+");
    } else if (key == _KEY_MULT) {
        pstEvent->enKey = _KEY_IRIS_SUB;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "IRIS-");
    } else if (key == _KEY_T1) {
        pstEvent->enKey = _KEY_AUTO;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "AUTO");
    } else if (key == _KEY_T2) {
        pstEvent->enKey = _KEY_LIGHT;
        snprintf(pstEvent->name, sizeof(pstEvent->name), "LIGHT");
    }

}

static void kb_ctrl_clear_display(int fd, int line)
{
    kbdebug("## kb_ctrl_clear_display line:[%d]", line);
    kb_msg_to_display(fd, " ", line, 1, 16);
}

static void kb_ctrl_clear_string(int line)
{
    editor_clear_string(&g_pstClient->stEditor);
    kb_ctrl_clear_display(g_pstClient->fd, line);
}

static void kb_ctrl_clear_screen(int fd)
{
    int line;
    if (g_kb_version == 0) {
        for (line = LINE_BEGIN; line < LINE_END; line++) {
            kb_ctrl_clear_display(fd, line);
        }
    } else {
#if 1
        for (line = LINE_BEGIN; line < 16; line++) {
            kb_msg_line_clear(fd, line, 1, 40);
        }
#else
        char clear[] = "Command:DISPLAY-CLEAR:EOF";
        kb_socket_send_msg(fd, clear, sizeof(clear));
        usleep(500 * 1000);
#endif
    }
}

static void kb_ctrl_show_login_ui(CLIENT_S *pstClient, const char *password)
{
    int len = 0;
    char tmp[MAX_STR_LEN];
    char line1[LINE_PW_LEN];
    char line2[LINE_PW_LEN];
    len = strlen(password);

    if (!len) {
        kb_msg_to_display(pstClient->fd, "     Login", 1, 1, 16);
        kb_ctrl_123_abc(pstClient->stEditor.enEdit, pstClient->stEditor.isLock);
        kb_ctrl_clear_display(pstClient->fd, 3);
        kb_msg_to_display(pstClient->fd, "   <HOST PW>", 3, 1, 16);
        kb_ctrl_clear_display(pstClient->fd, 4);
    } else {
        memset(line1, '\0', sizeof(line1));
        memset(line2, '\0', sizeof(line2));
        memset(tmp, '\0', sizeof(tmp));
        strcpy(tmp, password);
        memset(tmp, '*', len - 1);
        strncpy(line1, tmp, 16);
        strncpy(line2, tmp + 16, 16);
        kb_msg_to_display(pstClient->fd, line1, 3, 1, 16);
        kb_msg_to_display(pstClient->fd, line2, 4, 1, 16);
    }
}

static void kb_ctrl_show_home_ui(CLIENT_S *pstClient)
{
    kb_ctrl_clear_screen(pstClient->fd);
    pstClient->isLogin = 1;
    memset(&pstClient->stEditor, 0, sizeof(pstClient->stEditor));
    kb_msg_to_display(pstClient->fd, "Network Keyboard", LINE_TITLE, 1, 16);
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    insert_to_device_list(pstClient->ipaddr, pstClient->password);
}

static void kb_ctrl_show_select_ui(int fd, const char *id)
{
    kb_msg_to_display(fd, id, LINE_DATA, 16 - LINE_DATA_LEN + 1, LINE_DATA_LEN);
}

static void kb_ctrl_data_display(CLIENT_S *pstClient)
{
    kbdebug("kb_ctrl_data_display isLogin:[%d] entype:[%d]", pstClient->isLogin, pstClient->stEvent.enType);

    if (pstClient->isLogin) {
        if (pstClient->stEditor.isON && strlen(pstClient->stEditor.str)) {
            kb_ctrl_show_select_ui(pstClient->fd, pstClient->stEditor.str);
        }
    } else if (pstClient->stEvent.enType != TYPE_JOYSTICK) {
        pstClient->stEditor.isON = 1;
        kb_ctrl_show_login_ui(pstClient, pstClient->stEditor.str);
    }
}

static void kb_ctrl_key_display(CLIENT_S *pstClient, char *keyName)
{
    strcpy(pstClient->stEvent.dsp, keyName);
    kb_msg_to_display(pstClient->fd, pstClient->stEvent.dsp, LINE_KEY, 1, 16);
}

static int kb_ctrl_is_ready(CLIENT_S *pstClient)
{
    if (!pstClient->isLogin) {
        return 0;
    }

    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        return 0;
    }

    return 1;
}

//--------------------------------------------
// comunicate api
//--------------------------------------------
static void recv_state_from_kb_callback(int fd, char *ipaddr, KB_STATE_E enState)
{
    printf("recv_state_from_kb_callback!!!, fd:[%d] ip:[%s] enstate:[%d] g_kb_version:[%d]\n", fd, ipaddr, enState,
           g_kb_version);
    if (enState == KB_STATE_CONNECTED) {
        if (kb_ctrl_save_client(fd, ipaddr) == -1) {
            kb_msg_to_display(fd, "Device Busy", LINE_MODE, 3, 16);
            kb_socket_client_close(fd);
        } else {
            CLIENT_S *pstClient = kb_ctrl_get_client(fd);
            if (pstClient == NULL) {
                kberror("no find client \n");
                return ;
            }
            if (!is_online_device_list(pstClient->ipaddr)) { //if IP is have
                kb_ctrl_show_home_ui(pstClient);
                if (pstClient->password[0] == '\0') {
                    get_online_device_pw(pstClient);
                }
            } else {
                pstClient->stEditor.isON = 1;
                pstClient->stEditor.enEdit = EDIT_abc;
                if (g_kb_version == 1) {
                    usleep(1200 * 1000);
                    kb_ctrl_clear_screen(pstClient->fd);
                }
                kb_ctrl_show_login_ui(pstClient, "");
            }
        }
    } else {
        if (!kb_ctrl_get_client(fd)) {
            return ;
        }
        kb_ctrl_clear_screen(fd);
        kb_msg_to_display(fd, "Disconnected", LINE_MODE, 3, 16);
        g_pstClient->isLogin = 0;
        kb_ctrl_delete_client(fd);
    }
}

static int send_msg_to_nvr(int sendto, int type, void *data, int size)
{
    return ms_sock_send_msg(sendto, type, data, size, 0);
}

static void dealQtCmdResult(void *data)
{
    memcpy(&g_sQtStatus, data, sizeof(g_sQtStatus));
}

static void nvr_psw_modified()
{
    kbdebug("[KB PSW MODIFIED] 333 nvr_psw_modified");
    int i, j;
    for (i = 0, j = 0; i < MAX_CLIENT; i++, j = 0) {
        if (g_stSever.pClientList[i] != NULL) {
            if (g_stSever.pClientList[i]->ipaddr[0] != '\0' && g_stSever.pClientList[i]->password[0] != '\0' &&
                g_stSever.pClientList[i]->isLogin) {
                while (j < g_stSever.stUser.cnt) {
                    if (!strcmp(g_stSever.pClientList[i]->password, g_stSever.stUser.user[j++].passwd)) {
                        break;
                    }

                    if (j == g_stSever.stUser.cnt) {
                        g_stSever.pClientList[i]->psw_modified = 1;
                        kbdebug("[KB PSW MODIFIED] 444 fd:[%d], ip:[%s], psw:[%s]", g_stSever.pClientList[i]->fd,
                                g_stSever.pClientList[i]->ipaddr, g_stSever.pClientList[i]->password);
                    }
                }
            }
        }
    }
    for (i = 0, j = 0; i < MAX_LEN_128; i++, j = 0) {
        if (g_db_device_list.keyboards[i].ip_addr[0] != '\0' && g_db_device_list.keyboards[i].password[0] != '\0') {
            while (j < g_stSever.stUser.cnt) {
                if (!strcmp(g_db_device_list.keyboards[i].password, g_stSever.stUser.user[j++].passwd)) {
                    break;
                }

                if (j == g_stSever.stUser.cnt) {
                    delete_to_device(g_db_device_list.keyboards[i].ip_addr);
                }
            }
        }
    }
}

static void recv_msg_from_nvr_callback(void *arg, struct packet_header *hdr, void *data)
{
    static int psw_modified = 0;
    struct get_user_info *pgetUser = NULL;
    struct get_network_keyboard_info *pgetKeyboard = NULL;
    switch (hdr->type) {
        case RESPONSE_FLAG_GET_USER_INFO:
            pgetUser = (struct get_user_info *)data;
            memcpy(&g_stSever.stUser, pgetUser, sizeof(struct get_user_info));
            kbdebug("[KB PSW MODIFIED] 222 psw_modified:[%d], g_stSever.stUser.cnt:[%d]", psw_modified, g_stSever.stUser.cnt);
            if (psw_modified) {
                nvr_psw_modified();
            }
            psw_modified = 0;
            break;
        case RESPONSE_FLAG_SENDTO_QT_CMD:
            break;
        case RESPONSE_FLAG_USER_UPDATE:
            if (!kb_get_nvr_user_info()) {
                psw_modified = 1;
            }
            kbdebug("[KB PSW MODIFIED] 111 psw_modified:[%d], g_stSever.stUser.cnt:[%d]", psw_modified, g_stSever.stUser.cnt);
            break;
        case RESPONSE_FLAG_GET_QT_CMD_RESULT:
            dealQtCmdResult(data);
            break;
        case RESPONSE_FLAG_GET_NETWORK_KEYBOARD:
            pgetKeyboard = (struct get_network_keyboard_info*)data;
            memcpy(&g_db_device_list, pgetKeyboard, sizeof(struct get_network_keyboard_info));
            break;
        default:
            break;
    }
    return;
}

static void kb_send_func_cmd(char *cmd)
{
    static int cnt = 0;
    char tmp[100] = {0};

    cnt++;
    snprintf(tmp, sizeof(tmp), "%s", cmd);
    kbdebug("@@@@@@@@ kb_send_func_cmd send to qt cnt:[%d], cmd:[%s]", cnt, tmp);
    send_msg_to_nvr(SOCKET_TYPE_CORE, REQUEST_FLAG_SENDTO_QT_CMD, tmp, sizeof(tmp));
}

//-----------------------------------------------------------
// key map to function api
//------------------------------------------------------------
static int func_up_wall_enter(int cmd, void *argv)
{
    //static int mon, win, cam;
    char tmp[32];
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }


    /*
        if (cmd)
        {
            if (pstClient->stEditor.isON == 0)
            {
                pstClient->stEditor.isON = 1;
                kb_ctrl_123_abc(EDIT_123, 1);
                sprintf(tmp, "%s ID:", pstClient->stEvent.name);
                kb_msg_to_display(pstClient->fd, tmp, LINE_DATA, 1, LINE_DATA_POS);
            }
        }

        else
        {
            switch(pstClient->stEvent.enKey)
            {
                case _KEY_MON:
                    mon = atoi(pstClient->stEditor.str);
                    break;
                case _KEY_WIN:
                    win = atoi(pstClient->stEditor.str);
                    break;
                case _KEY_ENTER:
                    cam = atoi(pstClient->stEditor.str);
                    break;
                default:
                    return -1;
            }

            printf("===%s=-id=%d===\n\n", pstClient->stEvent.name, mon);
            printf("===%s=-id=%d===\n\n", pstClient->stEvent.name, win);
            printf("===%s=-id=%d===\n\n", pstClient->stEvent.name, cam);
            // TODO: no yet
           // sprintf(tmp, "SelChannel_%d", atoi(g_pstClient->stEditor.str));
            //kb_send_func_cmd(tmp);
            kb_ctrl_clear_string(LINE_DATA);
            kb_ctrl_set_mode(pstClient, pstClient->enMode);
            pstClient->stEditor.isON = 0;
            kb_ctrl_key_display(pstClient, "OK");
        }
    */

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            sprintf(tmp, "%s ID:", pstClient->stEvent.name);
            kb_msg_to_display(pstClient->fd, tmp, LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        //sprintf(tmp, "SelWnd_%d", atoi(pstClient->stEditor.str)-1);
        //kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        kb_ctrl_set_mode(pstClient, pstClient->enMode);
        pstClient->stEditor.isON = 0;
        kb_ctrl_key_display(pstClient, "OK");
    }
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    return cmd;
}

static int func_win_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            sprintf(tmp, "%s ID:", pstClient->stEvent.name);
            //printf("pstClient->stEvent.name = %s\n",pstClient->stEvent.name);
            kb_msg_to_display(pstClient->fd, tmp, LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "SelWnd_%d", atoi(pstClient->stEditor.str) - 1);
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        kb_ctrl_set_mode(pstClient, pstClient->enMode);
        pstClient->stEditor.isON = 0;
        kb_ctrl_key_display(pstClient, "OK");
    }
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    return cmd;
}

static int func_cam_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];
    int chan = 0;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            sprintf(tmp, "%s ID:", pstClient->stEvent.name);
            kb_msg_to_display(pstClient->fd, tmp, LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        chan = atoi(pstClient->stEditor.str);
        sprintf(tmp, "SingleChannel_%d", chan == 0 ? chan : chan - 1);
        kb_send_func_cmd(tmp);
        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_key_display(pstClient, "OK");
    }

    kb_ctrl_set_mode(pstClient, MODE_NVR);

    return cmd;
}

static int func_set_preset_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "SET ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Preset_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");

    }
    return cmd;
}

static int func_del_preset_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "DEL ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Preset_Del_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");

    }
    return cmd;
}

static int func_call_preset_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_msg_to_display(pstClient->fd, "CALL ID:", LINE_DATA, 1, LINE_DATA_POS);
            kb_ctrl_123_abc(EDIT_123, 1);
        }
    } else {
        sprintf(tmp, "PTZ_Preset_Call_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");
    }
    return cmd;
}

static int func_call_patrol_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];
    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "CALL ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Patrol_Call_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");
    }

    return cmd;
}

static int func_stop_patrol_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "STOP ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Patrol_Stop_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");
    }
    return cmd;
}

static int func_del_patrol_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "DELETE ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Patrol_Delete_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");
    }
    return cmd;
}

static int func_call_pattern_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "CALL ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Pattern_Call_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");
    }
    return cmd;
}

static int func_stop_pattern_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "STOP ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Pattern_Stop_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");
    }
    return cmd;
}
static int func_del_pattern_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    char tmp[32];

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (cmd) {
        if (pstClient->stEditor.isON == 0) {
            pstClient->stEditor.isON = 1;
            kb_ctrl_123_abc(EDIT_123, 1);
            kb_msg_to_display(pstClient->fd, "DELETE ID:", LINE_DATA, 1, LINE_DATA_POS);
        }
    } else {
        sprintf(tmp, "PTZ_Pattern_Delete_%d", atoi(pstClient->stEditor.str));
        kb_send_func_cmd(tmp);

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
        kb_ctrl_key_display(pstClient, "OK");
    }
    return cmd;
}

static int func_esc(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (pstClient->stEditor.isON == 1) {
        return cmd;
    }

    if (!pstClient->stEditor.isON) {
        kb_send_func_cmd("Esc");
    }

    kb_ctrl_clear_string(LINE_DATA);
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    pstClient->isLock = 0;

    return cmd;
}

static int func_mult(int cmd, void *argv)
{
    static int mode = 0;
    char data[100];
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }

    switch (mode) {
        case 0:
            strcpy(data, "Wnd_1");
            mode++;
            break;
        case 1:
            strcpy(data, "Wnd_4");
            if (device.max_cameras == 4) {
                mode = 0;
                break;
            }
            mode++;
            break;
        case 2:
            strcpy(data, "Wnd_8");
            mode++;
            break;
        case 3:
            strcpy(data, "Wnd_8_1");
            mode++;
            break;
        case 4:
            strcpy(data, "Wnd_9");
            if (device.max_cameras == 9) {
                mode = 0;
                break;
            }
            mode++;
            break;
        case 5:
            strcpy(data, "Wnd_12");
            mode++;
            break;
        case 6:
            strcpy(data, "Wnd_12_1");
            mode++;
            break;
        case 7:
            strcpy(data, "Wnd_14");
            mode++;
            break;
        case 8:
            strcpy(data, "Wnd_16");
            if (device.max_cameras == 16) {
                mode = 0;
                break;
            }
            mode++;
            break;
        case 9:
            strcpy(data, "Wnd_25");
            mode++;
            break;
        case 10:
            strcpy(data, "Wnd_32");
            mode++;
            break;
        case 11:
            strcpy(data, "Wnd_32_2");
            mode = 0;
            break;
        default:
            return cmd;
    }

    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd(data);

    return cmd;
}


static int func_jmove(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    static int isDone = 0;
    int isMove = 0;
    char tmp[64];

    if (!pstClient->isLogin) {
        return cmd;
    }

    isMove = pstClient->stEvent.enAction != ACTION_STOP ? 1 : 0;

    if (g_pstClient->stEditor.isON == 1) {
        if (!isMove) {
            isDone = 0;
        }
        if (!isDone) {
            isDone = isMove;
            if (strcmp(g_sQtStatus.curPage, "LIVEVIEW")) {
                if (!strcmp(pstClient->stEvent.name, "Left")) {
                    kb_send_func_cmd("Key_105");
                } else if (!strcmp(pstClient->stEvent.name, "Right")) {
                    kb_send_func_cmd("Key_106");
                }
            } else {
                return cmd;
            }
        }

        else {
            return cmd;
        }
    }

    else if (pstClient->enMode == MODE_NVR) {
        if (!isMove) {
            isDone = 0;
        }
        if (!isDone) {
            sprintf(tmp, "Dir_Nvr_%s_%d_%d", pstClient->stEvent.name, \
                    pstClient->stEvent.Hrate, pstClient->stEvent.Vrate);
            isDone = isMove;
            if (!strcmp(g_sQtStatus.curPage, "PLAYBACK")) {
                if (!strcmp(pstClient->stEvent.name, "Left")) {
                    snprintf(tmp, sizeof(tmp), "Video_Backward");
                } else if (!strcmp(pstClient->stEvent.name, "Right")) {
                    snprintf(tmp, sizeof(tmp), "Video_Forward");
                } else if (!strcmp(pstClient->stEvent.name, "STOP")) {
                    snprintf(tmp, sizeof(tmp), "Video_Here");
                }

                kb_ctrl_key_display(pstClient, tmp);
                kb_send_func_cmd(tmp);
            } else if (strcmp(g_sQtStatus.curPage, "LIVEVIEW")) {
                if (!strcmp(pstClient->stEvent.name, "Up")) {
                    kb_send_func_cmd("Key_103");
                } else if (!strcmp(pstClient->stEvent.name, "Down")) {
                    kb_send_func_cmd("Key_108");
                } else if (!strcmp(pstClient->stEvent.name, "Left")) {
                    kb_send_func_cmd("Key_105");
                } else if (!strcmp(pstClient->stEvent.name, "Right")) {
                    kb_send_func_cmd("Key_106");
                }
            } else {
                kb_send_func_cmd(tmp);
            }
            kbdebug("curPage:[%s], tmp:[%s]", g_sQtStatus.curPage, tmp);
        } else {
            return cmd;
        }
    }

    else if (pstClient->enMode == MODE_PTZ) {
        if (!isMove) {
            isDone = 0;
        }
        if (!isDone) {
            sprintf(tmp, "Dir_Ptz_%s_%d_%d", pstClient->stEvent.name, \
                    pstClient->stEvent.Hrate, pstClient->stEvent.Vrate);
            if (strcmp(g_sQtStatus.curPage, "LIVEVIEW")) {
                return cmd;
            } else {
                kb_send_func_cmd(tmp);
            }
        } else {
            return cmd;
        }
    }


    return cmd;
}


static int func_jzoom(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    static KB_ACTION_E enAction;

    if (!pstClient->isLogin) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }

    if (pstClient->stEvent.enAction == ACTION_TELE) {
        kb_send_func_cmd("PTZ_ZoomPlus");
    } else if (pstClient->stEvent.enAction == ACTION_WIDE) {
        kb_send_func_cmd("PTZ_ZoomMinus");
    } else {
        if (enAction == ACTION_TELE) {
            kb_send_func_cmd("PTZ_ZoomPlusStop");
        } else if (enAction == ACTION_WIDE) {
            kb_send_func_cmd("PTZ_ZoomMinusStop");
        }
    }
    enAction = pstClient->stEvent.enAction;

    return cmd;
}

static int func_jbutton(int cmd, void *argv)
{
    U64 now_time = get_now_time_ms();
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }

    if (now_time - pstClient->lastKeyTime > 500) {
        kb_send_func_cmd("Enter");
    } else {
        kb_send_func_cmd("FullScreen");
    }

    pstClient->lastKeyTime = now_time;

    return cmd;
}

static int func_shuttle_i(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }

    kb_ctrl_set_mode(pstClient, MODE_NVR);
    if (pstClient->stEvent.enAction == ACTION_CCW) {
        kb_send_func_cmd("Dial_Insid_Sub");
        kbdebug("#########func_shuttle_i Dial_Insid_Sub");
    } else if (pstClient->stEvent.enAction == ACTION_CW) {
        kb_send_func_cmd("Dial_Insid_Add");
        kbdebug("#########func_shuttle_i Dial_Insid_Add");
    }

    return cmd;
}
//slow down, speed up, fast-forward, or rewind
static int func_shuttle_o(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    static int run = 0;
    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }

    kb_ctrl_set_mode(pstClient, MODE_NVR);

    if (pstClient->stEvent.enAction == ACTION_CCW) {
        if (!run) {
            kb_send_func_cmd("Dial_Out_Sub");
            kbdebug("#########func_shuttle_o Dial_Out_Sub");
            run = 1;
        }
    } else if (pstClient->stEvent.enAction == ACTION_CW) {
        if (!run) {
            kb_send_func_cmd("Dial_Out_Add");
            kbdebug("#########func_shuttle_o Dial_Out_Add");
            run = 1;
        }
    } else {
        run = 0;
    }


    return cmd;
}

static int func_a(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    //kb_ctrl_set_mode(pstClient, !pstClient->enMode);
    //printf("mode = %d\n",mode);
    if (mode == 0) {
        pstClient->stEditor.isON = 0;
        g_pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_NVR);
    } else if (mode == 1) {
        pstClient->stEditor.isON = 0;
        g_pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_PTZ);
    } else if (mode == 2) {
        g_pstClient->stEditor.isON = 1;
        kb_ctrl_123_abc(EDIT_abc, 1);
        g_pstClient->stEditor.enEdit = EDIT_abc;
        //pstClient->enMode = MODE_NVR;
        mode = 3;
    }

    else if (mode == 3) {
        g_pstClient->stEditor.isON = 1;
        kb_ctrl_123_abc(EDIT_123, 1);
        g_pstClient->stEditor.enEdit = EDIT_123;
        //pstClient->enMode = MODE_NVR;
        mode = 0;
    }
    return cmd;
}

static int func_audio(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Audio");

    return cmd;
}

static int func_menu(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Menu");

    return cmd;
}

static int func_seq(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Seq_Current");

    return cmd;
}

static int func_rec(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Rec");

    return cmd;
}

static int func_snap(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Snap");

    return cmd;
}

static int func_prev(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Screen_Prev");
    //kb_send_func_cmd(KEY_UP);

    return cmd;
}

static int func_next(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Screen_Next");
    //kb_send_func_cmd(KEY_DOWN);

    return cmd;
}

static int func_t1(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("TabPage_Next");

    return cmd;
}

static int func_t2(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("ChangeFocus_Next");

    return cmd;
}

static int func_del(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    kb_send_func_cmd("Key_14");

    return cmd;
}


void simulate_key(int kval)
{
    int fd_key = -1;
    int fd = 0;
    int i;
    char name[64];
    char buf[256] = { 0, };
    for (i = 0; i < 6; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((fd = open(name, O_RDONLY, 0)) >= 0) {
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
            if (!strcmp(buf, "dvr_keypad")) {
                fd_key = open(name, O_RDWR);
            }
            close(fd);
        }
    }

    struct input_event event;
    gettimeofday(&event.time, 0);
    //按下kval键
    event.type = EV_KEY;
    event.value = 1;
    event.code = kval;
    write(fd_key, &event, sizeof(event));
    //同步，也就是把它报告给系统

    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd_key, &event, sizeof(event));

    //按下kval键
    event.type = EV_KEY;
    event.value = 0;
    event.code = kval;
    write(fd_key, &event, sizeof(event));
    //同步，也就是把它报告给系统

    event.type = EV_SYN;
    event.value = 0;
    event.code = SYN_REPORT;
    write(fd_key, &event, sizeof(event));

    close(fd_key);
}


static int func_enter(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    int i = 0;
    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        return cmd;
    }

    if (pstClient->isLogin) {
        //enter = 1;
        for (i = 0; i < pstClient->stEditor.count; i++) {
            if (pstClient->stEditor.str[i] == '1') {
                //kb_send_func_cmd(key_1);
                simulate_key(KEY_1);
            } else if (pstClient->stEditor.str[i] == '2') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_2);
            } else if (pstClient->stEditor.str[i] == '3') {
                //kb_send_func_cmd(key_3);
                simulate_key(KEY_3);
            } else if (pstClient->stEditor.str[i] == '4') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_4);
            } else if (pstClient->stEditor.str[i] == '5') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_5);
            } else if (pstClient->stEditor.str[i] == '6') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_6);
            } else if (pstClient->stEditor.str[i] == '7') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_7);
            } else if (pstClient->stEditor.str[i] == '8') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_8);
            } else if (pstClient->stEditor.str[i] == '9') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_9);
            } else if (pstClient->stEditor.str[i] == '0') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_0);
            } else if (pstClient->stEditor.str[i] == '.') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_DOT);
            } else if (pstClient->stEditor.str[i] == 'a') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_A);
            } else if (pstClient->stEditor.str[i] == 'b') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_B);
            } else if (pstClient->stEditor.str[i] == 'c') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_C);
            } else if (pstClient->stEditor.str[i] == 'd') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_D);
            } else if (pstClient->stEditor.str[i] == 'e') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_E);
            } else if (pstClient->stEditor.str[i] == 'f') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_F);
            } else if (pstClient->stEditor.str[i] == 'g') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_G);
            } else if (pstClient->stEditor.str[i] == 'h') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_H);
            } else if (pstClient->stEditor.str[i] == 'i') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_I);
            } else if (pstClient->stEditor.str[i] == 'j') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_J);
            } else if (pstClient->stEditor.str[i] == 'k') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_K);
            } else if (pstClient->stEditor.str[i] == 'l') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_L);
            } else if (pstClient->stEditor.str[i] == 'm') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_M);
            } else if (pstClient->stEditor.str[i] == 'n') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_N);
            } else if (pstClient->stEditor.str[i] == 'o') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_O);
            } else if (pstClient->stEditor.str[i] == 'p') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_P);
            } else if (pstClient->stEditor.str[i] == 'q') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_Q);
            } else if (pstClient->stEditor.str[i] == 'r') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_R);
            } else if (pstClient->stEditor.str[i] == 's') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_S);
            } else if (pstClient->stEditor.str[i] == 't') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_T);
            } else if (pstClient->stEditor.str[i] == 'u') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_U);
            } else if (pstClient->stEditor.str[i] == 'v') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_V);
            } else if (pstClient->stEditor.str[i] == 'w') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_W);
            } else if (pstClient->stEditor.str[i] == 'x') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_X);
            } else if (pstClient->stEditor.str[i] == 'y') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_Y);
            } else if (pstClient->stEditor.str[i] == 'z') {
                //kb_send_func_cmd(key_2);
                simulate_key(KEY_Z);
            }

        }
        kb_ctrl_clear_string(LINE_DATA);
        simulate_key(KEY_ENTER);
        kb_send_func_cmd("GetStatusInfo");
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_NVR);
        return cmd;

    }


    //else if (pstClient->isLogin)
    //{
    //  kb_send_func_cmd("Enter");
    //enter = 0;
    //}

    else if (strlen(pstClient->stEditor.str)) {
        kbdebug("NVR Password :%s\n", pstClient->stEditor.str);
        if (kb_get_nvr_user_info() != -1) {
            for (i = 0; i < g_stSever.stUser.cnt; i++) {
                kbdebug("[%d]===name = %s , pw = %s \n", i, g_stSever.stUser.user[i].user, g_stSever.stUser.user[i].passwd);
                if (!strcmp(pstClient->stEditor.str, g_stSever.stUser.user[i].passwd)) {
                    strcpy(pstClient->password, g_stSever.stUser.user[i].passwd);
                    kb_ctrl_show_home_ui(pstClient);
                    return cmd;
                }
            }

        }
        kb_ctrl_clear_string(LINE_DATA);
        kb_ctrl_clear_string(LINE_KEY);
        kb_msg_to_display(pstClient->fd, "   Try Again", 3, 1, 16);
        sleep(1);
        kb_ctrl_show_login_ui(pstClient, "");
    }

    return cmd;
}


static int func_focus_add(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!pstClient->isLogin) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);
    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        kb_send_func_cmd("PTZ_FocusPlusStop");
    } else {
        kb_send_func_cmd("PTZ_FocusPlus");
    }

    return cmd;
}

static int func_focus_sub(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!pstClient->isLogin) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);
    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        kb_send_func_cmd("PTZ_FocusMinusStop");
    } else {
        kb_send_func_cmd("PTZ_FocusMinus");
    }

    return cmd;
}

static int func_zoom_add(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!pstClient->isLogin) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }

    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);
    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        kb_send_func_cmd("PTZ_ZoomPlusStop");
    } else {
        kb_send_func_cmd("PTZ_ZoomPlus");
    }

    return cmd;
}

static int func_zoom_sub(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!pstClient->isLogin) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);
    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        kb_send_func_cmd("PTZ_ZoomMinusStop");
    } else {
        kb_send_func_cmd("PTZ_ZoomMinus");
    }

    return cmd;
}

static int func_iris_add(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!pstClient->isLogin) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);
    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        kb_send_func_cmd("PTZ_IrisPlusStop");
    } else {
        kb_send_func_cmd("PTZ_IrisPlus");
    }

    return cmd;
}

static int func_iris_sub(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!pstClient->isLogin) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);
    if (pstClient->stEvent.enAction == ACTION_RELEASE) {
        kb_send_func_cmd("PTZ_IrisMinusStop");
    } else {
        kb_send_func_cmd("PTZ_IrisMinus");
    }

    return cmd;
}

static int func_light(int cmd, void *argv)
{
    static int isON = 0;
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }

    isON = !isON;
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);
    if (isON) {
        kb_send_func_cmd("PTZ_LightOn");
    } else {
        kb_send_func_cmd("PTZ_LightOff");
    }

    return cmd;
}

static int func_wiper(int cmd, void *argv)
{
    static int isON = 0;
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);

    isON = !isON;
    if (isON) {
        kb_send_func_cmd("PTZ_WiperOn");
    } else {
        kb_send_func_cmd("PTZ_WiperOff");
    }

    return cmd;
}

static int func_debug(int cmd, void *argv)
{
    static int count = 0;
    static U64 last_time = 0;
    U64 now_time = get_now_time_ms();

    if (now_time - last_time < 500) {
        count++;
    } else {
        count = 0;
    }

    if (count == 3) {
        count = 0;
        g_kbdebug = !g_kbdebug;
    }

    last_time = now_time;

    return cmd;
}
#if 0
static int func_switch_version(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (cmd) {
        if (g_kb_version == 1) {
            kb_msg_to_display(pstClient->fd, "SWITCH TO OLD", LINE_DATA, 1, LINE_DATA_LEN);
        } else {
            kb_msg_to_display(pstClient->fd, "SWITCH TO NEW", LINE_DATA, 1, LINE_DATA_LEN);
        }
    } else {
        g_kb_version = !g_kb_version;
        pstClient->version = g_kb_version;
        if (g_kb_version == 1) {
            kbdebug("kb is new version!");
        } else {
            kbdebug("kb is old version!");
        }

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_NVR);
        kb_ctrl_key_display(pstClient, "OK");
    }
    kbdebug("#########func_switch_version");
    return cmd;

}
#endif

static int func_print_heartbeat(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (cmd) {
        if (g_kb_heartbeat) {
            kb_msg_to_display(pstClient->fd, "hide heartbeat", LINE_DATA, 1, LINE_DATA_LEN);
        } else {
            kb_msg_to_display(pstClient->fd, "show heartbeat", LINE_DATA, 1, LINE_DATA_LEN);
        }
    } else {
        g_kb_heartbeat = !g_kb_heartbeat;
        if (g_kb_heartbeat) {
            kbdebug("print heartbeat start\n");
        } else {
            kbdebug("print heartbeat end\n");
        }

        kb_ctrl_clear_string(LINE_DATA);
        pstClient->stEditor.isON = 0;
        kb_ctrl_set_mode(pstClient, MODE_NVR);
        kb_ctrl_key_display(pstClient, "OK");
    }

    return cmd;
}


static int func_r_click(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("R_Click");

    return cmd;
}

static int func_auto(int cmd, void *argv)
{
    static int run;
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_PTZ);

    if (run) {
        kb_send_func_cmd("PTZ_AutoScanOff");
    } else {
        kb_send_func_cmd("PTZ_AutoScanOn");
    }

    run = !run;
    return cmd;
}

static int func_play(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Video_Play");
    return cmd;
}

static int func_pause(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Video_Pause");
    return cmd;
}

static int func_play_stop(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }

    if (g_pstClient->stEditor.isON == 1) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Video_Stop");

    return cmd;
}

static int func_ptz_stop(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;
    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }
    kb_send_func_cmd("PTZ_STOP");
    return 1;
}

static int func_toolbar(int cmd, void *argv)
{
    CLIENT_S *pstClient = (CLIENT_S *)argv;

    if (!kb_ctrl_is_ready(pstClient)) {
        return cmd;
    }
    pstClient->stEditor.isON = 0;
    kb_ctrl_set_mode(pstClient, MODE_NVR);
    kb_send_func_cmd("Toolbar");

    return cmd;
}

static KEY_FUNC_S g_key_func[] = {
    {2, 1, {_KEY_WIN, _KEY_ENTER}, func_win_enter},
    {2, 1, {_KEY_CAM, _KEY_ENTER}, func_cam_enter},
    {2, 1, {_KEY_MON, _KEY_ENTER}, func_up_wall_enter},
    {3, 1, {_KEY_CAM, _KEY_CAM, _KEY_ENTER}, func_up_wall_enter},
    {2, 1, {_KEY_PRESET, _KEY_ENTER}, func_set_preset_enter},
    {4, 3, {_KEY_CALL, _KEY_DEL, _KEY_PRESET, _KEY_ENTER}, func_del_preset_enter},
    {3, 2, {_KEY_CALL, _KEY_PRESET, _KEY_ENTER}, func_call_preset_enter},
    {3, 2, {_KEY_CALL, _KEY_PATROL, _KEY_ENTER}, func_call_patrol_enter},
    {3, 2, {_KEY_PATROL, _KEY_PATROL, _KEY_ENTER}, func_stop_patrol_enter},
    {3, 2, {_KEY_CALL, _KEY_PATTERN, _KEY_ENTER}, func_call_pattern_enter},
    {3, 2, {_KEY_PATTERN, _KEY_PATTERN, _KEY_ENTER}, func_stop_pattern_enter},
    //{3, 2, {_KEY_CALL, _KEY_SEQ, _KEY_ENTER}, func_switch_version},//switch veersion
    {1, 1, {_KEY_J_MOVE}, func_jmove},
    {1, 1, {_KEY_J_ZOOM}, func_jzoom},
    {1, 1, {_KEY_J_BUTTON}, func_jbutton},
    {1, 1, {_KEY_A}, func_a},
    {1, 1, {_KEY_ESC}, func_esc},
    {1, 1, {_KEY_MULT}, func_mult},
    {1, 1, {_KEY_AUDIO}, func_audio},
    {1, 1, {_KEY_MENU}, func_menu},
    {1, 1, {_KEY_SEQ}, func_seq},
    {1, 1, {_KEY_REC}, func_rec},
    {1, 1, {_KEY_SNAP}, func_snap},
    {1, 1, {_KEY_PREV}, func_prev},
    {1, 1, {_KEY_NEXT}, func_next},
    {1, 1, {_KEY_T1}, func_t1},
    {1, 1, {_KEY_T2}, func_t2},
    {1, 1, {_KEY_ENTER}, func_enter},
    {1, 1, {_KEY_FOCUS_ADD}, func_focus_add},
    {1, 1, {_KEY_FOCUS_SUB}, func_focus_sub},
    {1, 1, {_KEY_ZOOM_ADD}, func_zoom_add},
    {1, 1, {_KEY_ZOOM_SUB}, func_zoom_sub},
    {1, 1, {_KEY_IRIS_ADD}, func_iris_add},
    {1, 1, {_KEY_IRIS_SUB}, func_iris_sub},
    {1, 1, {_KEY_LIGHT}, func_light},
    {1, 1, {_KEY_WIPER}, func_wiper},
    {1, 1, {_KEY_DEL}, func_del},
    {1, 1, {_KEY_DOT}, func_debug},
    {0, 0, {0}, NULL},
};

static KEY_FUNC_S g_key_func_1[] = {
    {2, 1, {_KEY_WIN, _KEY_ENTER}, func_win_enter},
    {2, 1, {_KEY_CAM, _KEY_ENTER}, func_cam_enter},
    {2, 1, {_KEY_MON, _KEY_ENTER}, func_up_wall_enter},
    {3, 1, {_KEY_CAM, _KEY_CAM, _KEY_ENTER}, func_up_wall_enter},
    {2, 1, {_KEY_PRESET, _KEY_ENTER}, func_set_preset_enter},
    {3, 2, {_KEY_PTZ_DEL, _KEY_PRESET, _KEY_ENTER}, func_del_preset_enter},
    {3, 2, {_KEY_CALL, _KEY_PRESET, _KEY_ENTER}, func_call_preset_enter},
    {3, 2, {_KEY_CALL, _KEY_PATROL, _KEY_ENTER}, func_call_patrol_enter},
    {3, 2, {_KEY_PTZ_DEL, _KEY_PATROL, _KEY_ENTER}, func_del_patrol_enter},// new
    {3, 2, {_KEY_CALL, _KEY_PATTERN, _KEY_ENTER}, func_call_pattern_enter},
    {3, 2, {_KEY_PTZ_DEL, _KEY_PATTERN, _KEY_ENTER}, func_del_pattern_enter},// new
    //{3, 2, {_KEY_CALL, _KEY_SEQ, _KEY_ENTER}, func_switch_version},//switch veersion
    {3, 2, {_KEY_CALL, _KEY_DOT, _KEY_ENTER}, func_print_heartbeat},
    {1, 1, {_KEY_J_MOVE}, func_jmove},
    {1, 1, {_KEY_J_ZOOM}, func_jzoom},
    {1, 1, {_KEY_J_BUTTON}, func_jbutton},
    {1, 1, {_KEY_SHUTTLE_I}, func_shuttle_i},// new
    {1, 1, {_KEY_SHUTTLE_O}, func_shuttle_o},// new
    {1, 1, {_KEY_A}, func_a},
    {1, 1, {_KEY_ESC}, func_esc},
    {1, 1, {_KEY_MULT}, func_mult},
    {1, 1, {_KEY_AUDIO}, func_audio},
    {1, 1, {_KEY_MENU}, func_menu},
    {1, 1, {_KEY_SEQ}, func_seq},
    {1, 1, {_KEY_REC}, func_rec},
    {1, 1, {_KEY_SNAP}, func_snap},
    {1, 1, {_KEY_PREV}, func_prev},
    {1, 1, {_KEY_NEXT}, func_next},
    {1, 1, {_KEY_T1}, func_t1},
    {1, 1, {_KEY_T2}, func_t2},
    {1, 1, {_KEY_ENTER}, func_enter},
    {1, 1, {_KEY_FOCUS_ADD}, func_focus_add},
    {1, 1, {_KEY_FOCUS_SUB}, func_focus_sub},
    {1, 1, {_KEY_ZOOM_ADD}, func_zoom_add},
    {1, 1, {_KEY_ZOOM_SUB}, func_zoom_sub},
    {1, 1, {_KEY_IRIS_ADD}, func_iris_add},
    {1, 1, {_KEY_IRIS_SUB}, func_iris_sub},
    {1, 1, {_KEY_LIGHT}, func_light},
    {1, 1, {_KEY_WIPER}, func_wiper},
    {1, 1, {_KEY_DEL}, func_del},
    {1, 1, {_KEY_DOT}, func_debug},
    {1, 1, {_KEY_R_CLICK}, func_r_click},// new
    {1, 1, {_KEY_AUTO}, func_auto},// new
    {1, 1, {_KEY_PLAY}, func_play},// new
    {1, 1, {_KEY_PAUSE}, func_pause},// new
    {1, 1, {_KEY_STOP}, func_play_stop},// new
    {1, 1, {_KEY_TOOLBAR}, func_toolbar},// new
    {1, 1, {_KEY_CALL}, func_ptz_stop},// new
    {0, 0, {0}, NULL},
};

static int key_func_match(CLIENT_S *pstClient)
{
    int i, j;
    int ret = 0;
    int match_key_num = 0;
    int count = 0;
    KEY_FUNC_S *key_func;
    int num = 0;

    if (pstClient->version == 1) {
        key_func = g_key_func_1;
        num = ARRAY_SIZE(g_key_func_1);
    } else {
        key_func = g_key_func;
        num = ARRAY_SIZE(g_key_func);
    }

    count = pstClient->keyCount ? pstClient->keyCount : 1;

    for (i = 0; i < num; i++) {
        for (j = 0; j < count;) {
            if (key_func[i].enKey[j] != pstClient->enKey[j]) {
                break;
            }

            j++;
            match_key_num = ms_max(match_key_num, j);

            if (j == key_func[i].input || j == key_func[i].count) {
                if (key_func[i].func) {
                    ret = key_func[i].func(key_func[i].count - j, pstClient);
                }
                if (ret == 0) {
                    return ret;
                }
                if (j > 1) { // 避免多余的 " PTZ_STOP " 指令
                    num -= 10;
                }
            }
        }
    }
    return match_key_num;
}

static void kb_ctrl_show_psw_wrong(CLIENT_S *pstClient)
{
    kb_ctrl_clear_screen(pstClient->fd);
    kb_msg_to_display(pstClient->fd, "Disconnected", LINE_MODE, 3, 16);
    kb_msg_to_display(pstClient->fd, "Wrong Password", LINE_DATA, 3, 16);

    pstClient->isLogin = 0;
    pstClient->psw_modified = -1;
    pstClient->stEditor.isON = 1;
    pstClient->stEditor.enEdit = EDIT_abc;
    memset(&pstClient->password, 0, sizeof(pstClient->password));
}

static int kb_ctrl_psw_modify(CLIENT_S *pstClient)
{
    char ack[] = "Command:HeartbeatACK:EOF";
    kbdebug("[KB PSW MODIFIED] 555 ip:[%s], ver:[%d], login:[%d], psw_modified:[%d]", pstClient->ipaddr, pstClient->version,
            pstClient->isLogin, pstClient->psw_modified);
    if (pstClient->psw_modified == 1) {
        if (pstClient->isLogin == 1) {
            kb_ctrl_show_psw_wrong(pstClient);
            kb_socket_send_msg(pstClient->fd, ack, sizeof(ack));
            return 1;
        }
    } else {
        kb_ctrl_show_login_ui(pstClient, pstClient->stEditor.str);
    }
    pstClient->psw_modified = 0;
    return 0;
}

#if 0
static char *get_last_str(char *src, char *str)
{
    char *p = NULL;
    char *pSrc = src;
    while ((pSrc = strstr(pSrc, str)) != NULL) {
        p = pSrc;
        pSrc += strlen(str);
    }

    return p;
}

static char *get_kb_cmd_str(char *src, CLIENT_S *pstClient)
{
    if (!src || !pstClient) {
        return src;
    }
    int i = 0;
    char *p = NULL;

    for (i = 0; i < 4; i++) {//Heartbeat   HeartbeatACK   VERSIONS-E4
        p = get_last_str(src, "IP-Controller:");
        if (p == NULL) {
            return src;
        } else if (strstr(p, "Heartbeat") || strstr(p, "Command:VERSIONS-E4:EOF")) {
            *p = '\0';
        } else {
            break;
        }
    }

    return p;
}
#endif

static int deal_kb_special_cmd(CLIENT_S *pstClient, char *msg)
{
    if (!msg || !pstClient) {
        return -1;
    }
    char *ack = "Command:HeartbeatACK:EOF";
    char *heartBeat = "IP-Controller:Heartbeat:EOF";
    char *heartBeatAck = "IP-Controller:HeartbeatACK:EOF";
    char *ver = "Command:VERSIONS-E4:EOF";
    int len = 0;
    if (strstr(msg, heartBeatAck)) {
        pstClient->stEvent.enType = TYPE_HB_ACK;
        len = strlen(heartBeatAck);
    } else if (strstr(msg, heartBeat)) {
        pstClient->stEvent.enType = TYPE_HB_REQ;
        kb_socket_send_msg(pstClient->fd, ack, strlen(ack));
        len = strlen(heartBeat);
    } else if (strstr(msg, ver)) {
        g_kb_version = 1;
        len = strlen(ver);
    }
    if (strlen(msg) == len) {
        return 0;
    }

    return 1;
}

static int deal_kb_cmd(CLIENT_S *pstClient, char *cmd)
{
    kb_parse_string(cmd, &pstClient->stEvent);
    if (pstClient->version != g_kb_version) {
        pstClient->version = g_kb_version;
        kb_ctrl_clear_screen(pstClient->fd);
        pstClient->isLogin = 0;
        pstClient->stEditor.isON = 1;
        pstClient->stEditor.enEdit = EDIT_abc;
        kb_ctrl_show_login_ui(pstClient, "");
    }
    kbdebug("[htx] [kbPtz] [kb222] version:[%d], fd:[%d], size:[%d], cmd:[%s]\n",
            g_kb_version, pstClient->fd, strlen(cmd), cmd);
    if (pstClient->stEvent.enType == TYPE_HB_REQ) {
        if (g_kb_heartbeat)
            printf("[kb_heartbeat] recv heartbeat from ip:[%s], fd:[%d]\n",
                   pstClient->ipaddr, pstClient->fd);
//        kb_socket_send_msg(pstClient->fd, ack, strlen(ack));

        return 0;
    }

    if (pstClient->stEvent.enType == TYPE_HB_ACK) {
        return 0;
    }

    if (pstClient->stEvent.enType == TYPE_KB_SET ||
        pstClient->stEvent.enType == TYPE_KB_LOCK) {
        pstClient->isLock = 1;
    } else if (pstClient->stEvent.enType == TYPE_KB_UNLOCK) {
        pstClient->isLock = 0;
        if (pstClient->version == 1) {
            if (!is_online_device_list(pstClient->ipaddr)) { //if IP is have
                kb_ctrl_show_home_ui(pstClient);
                kbdebug(" unlock  kb_ctrl_show_home_ui");
                if (pstClient->isLogin && pstClient->stEditor.isON && strlen(pstClient->stEditor.str)) {
                    kb_ctrl_show_select_ui(pstClient->fd, pstClient->stEditor.str);
                }
            } else {
                pstClient->stEditor.isON = 1;
                pstClient->stEditor.enEdit = EDIT_abc;
                kb_ctrl_show_login_ui(pstClient, "");
                kbdebug(" unlock  kb_ctrl_show_login_ui");
            }
            return 0;
        }
    }

    if (pstClient->isLock == 1) {
        return 0;
    }

    if (pstClient->version == 1) {
        kb_ctrl_nvr_ptz(&pstClient->stEvent);

        if (!pstClient->stEditor.isON && pstClient->enMode == MODE_PTZ && pstClient->stEvent.enKey == _KEY_DEL) {
            pstClient->stEvent.enKey = _KEY_PTZ_DEL;
        }
    }
    if (pstClient->stEvent.enAction != ACTION_RELEASE) {
        if (!pstClient->stEditor.isON) {
            if (strcmp(pstClient->stEvent.name, pstClient->stEvent.dsp)) {
                kb_ctrl_key_display(pstClient, pstClient->stEvent.name);
            }
        }

        //修复输入模式下按组合键导致A键卡主问题
        if (pstClient->stEditor.isON && ((pstClient->stEvent.enKey == _KEY_MON)
                                         || (pstClient->stEvent.enKey == _KEY_WIN)
                                         || (pstClient->stEvent.enKey == _KEY_CAM)
                                         || (pstClient->stEvent.enKey == _KEY_PRESET)
                                         || (pstClient->stEvent.enKey == _KEY_PATROL)
                                         || (pstClient->stEvent.enKey == _KEY_PATTERN)
                                         || (pstClient->stEvent.enKey == _KEY_CALL))) {
            return 0;
        }
        //非ptz模式下按ptz组合键无效
        if (pstClient->enMode != MODE_PTZ &&
            ((pstClient->stEvent.enKey == _KEY_PRESET)
             || (pstClient->stEvent.enKey == _KEY_PATROL)
             || (pstClient->stEvent.enKey == _KEY_PATTERN)
             || (pstClient->stEvent.enKey == _KEY_CALL))) {
            return 0;
        }
        //按下call键后按其它键可立即生效
        if (pstClient->enKey[0] == _KEY_CALL && pstClient->keyCount == 1) {
            if ((pstClient->stEvent.enKey != _KEY_PRESET)
                && (pstClient->stEvent.enKey != _KEY_PATROL)
                && (pstClient->stEvent.enKey != _KEY_PATTERN)
                && (pstClient->stEvent.enKey != _KEY_DOT)) {
                pstClient->keyCount = 0;
            }
        }
        pstClient->enKey[pstClient->keyCount] = pstClient->stEvent.enKey;
        if (pstClient->enKey[pstClient->keyCount] == _KEY_ESC) {
            pstClient->keyCount = 1;
            //esc 支持退出组合键
            if ((pstClient->enKey[0] == _KEY_CALL)
                || (pstClient->enKey[0] == _KEY_WIN)
                || (pstClient->enKey[0] == _KEY_MON)
                || (pstClient->enKey[0] == _KEY_CAM)
                || (pstClient->enKey[0] == _KEY_PRESET)
                || (pstClient->enKey[0] == _KEY_PATROL)
                || (pstClient->enKey[0] == _KEY_PATTERN)
                || (pstClient->enKey[0] == _KEY_PTZ_DEL)) {
                kb_ctrl_clear_string(LINE_DATA);
                pstClient->stEditor.isON = 0;
                pstClient->enKey[0] = _KEY_ESC;
                return 0;
            }
            pstClient->enKey[0] = _KEY_ESC;
        } else {
            pstClient->keyCount++;
            editor_insert_string(pstClient->stEvent.enKey, &pstClient->stEditor);
            //printf("pstClient->stEvent.enKey = %d\n",pstClient->stEvent.enKey);
            if (pstClient->stEvent.enKey == _KEY_A) {
                kb_ctrl_123_abc(pstClient->stEditor.enEdit, pstClient->stEditor.isLock);
            }
            kb_ctrl_data_display(pstClient);
        }
        pstClient->keyCount = key_func_match(pstClient);
    } else {
        key_func_match(pstClient);
    }

    return 0;
}

static int recv_msg_from_kb_callback(int fd, void *data, int size)
{
    char msg[2048];
    char *p;
    char cmd[64] = {0};
    CLIENT_S *pstClient = kb_ctrl_get_client(fd);
    if (pstClient == NULL) {
        kberror("no find client \n");
        return -1;
    }
    g_kb_version = pstClient->version;
    memset(msg, 0, sizeof(msg));
    memcpy(msg, data, ms_min(size, sizeof(msg)));
    kbdebug("[htx] [kbPtz] [kb111] recv_msg_from_kb:[%d][%d] psw_modified:[%d], msg:[%s]\n\n",
            fd, size, pstClient->psw_modified, msg);

    if (pstClient->psw_modified) {
        if (kb_ctrl_psw_modify(pstClient)) {
            return 0;
        }
    }
    if (!deal_kb_special_cmd(pstClient, msg)) {
        return 0;
    }

    p = msg;
    while (strlen(p) != 0 && strstr(p, "EOF")) { //deal all cmd   stick系列和zoom系列都只取最后一条指令
        snprintf(cmd, ms_min(strstr(p, "EOF") - p, sizeof(cmd)), "%s", p);
        p = strstr(p, "EOF") + strlen("EOF");
        if (strstr(cmd, ":J-") && !strstr(cmd, ":J-B")) {
            if (strstr(cmd, ":J-Z")) {
                if (strstr(p, ":J-Z")) {
                    continue ;
                }
            } else {
                if (strstr(p, ":J-ST") || strstr(p, ":J-R") || strstr(p, ":J-L")) {
                    continue ;
                }
            }
        }
        deal_kb_cmd(pstClient, cmd);
    }

    return 0;
}

static int recv_version_from_kb_callback(int fd, char *ipaddr, void *data, int size, int ver)
{
    char ack[] = "Command:HeartbeatACK:EOF";
    char req_ver[] = "Command:VERSIONS-INQUIRE:EOF";
    char tmp[1024];
    memset(tmp, 0, sizeof(tmp));
    memcpy(tmp, data, size);
    ver ++ ;
//    printf("recv_version_from_kb_callback fd:[%d], ip:[%s], tmp:[%s], size:[%d],ver:[%d]\n", fd, ipaddr, tmp, size, ver);
    if (size) {
        if (strstr(tmp, "VERSIONS-E4") != NULL) {
            ver = 1;
        } else if (strstr(tmp, "Heartbeat")) {
            kb_socket_send_msg(fd, ack, sizeof(ack));
        }
    }

    if (ver >= 0) {
        g_kb_version = ver;
        printf("recv_version_from_kb_callback, ## recv_state_from_kb_callback ## kb_version:[%d] fd:[%d], ip:[%s]\n",
               g_kb_version, fd, ipaddr);
        recv_state_from_kb_callback(fd, ipaddr, KB_STATE_CONNECTED);
    } else {
        kb_socket_send_msg(fd, req_ver, sizeof(req_ver));
    }

    return ver;
}

static int socket_init()
{
    SKT_CB_S stSktCb = {0};

    ms_sock_init(recv_msg_from_nvr_callback, NULL, SOCKET_TYPE_KB, SOCK_MSSERVER_PATH);
    stSktCb.recv = recv_msg_from_kb_callback;
    stSktCb.state = recv_state_from_kb_callback;
    stSktCb.version = recv_version_from_kb_callback;
    kb_socket_init(&stSktCb);

    return 0;
}

static int socket_uinit()
{
    ms_sock_uninit();
    kb_socket_uinit();

    return 0;
}

static int kb_core_init()
{
    memset(&g_stSever, 0, sizeof(SERVER_S));
    memset(&g_sQtStatus, 0, sizeof(struct get_qt_cmd_result));
    memset(&g_device_list, 0, sizeof(LOGIN_DEVICE_INFO)*MAX_CLIENT);
    memset(&g_db_device_list, 0, sizeof(struct NetworkKeyboards));
    kb_get_nvr_user_info();
    kb_send_func_cmd("GetStatusInfo");

    db_get_device(SQLITE_FILE_NAME, &device);
    return 0;
}

static int kb_core_uinit()
{
    int i;
    int fd = 0;

    for (i = 0; i < MAX_CLIENT; i++) {
        if (g_stSever.pClientList[i] != NULL) {
            fd = g_stSever.pClientList[i]->fd;
            kb_ctrl_clear_screen(fd);
            kb_msg_to_display(fd, "Disconnected", LINE_MODE, 3, 16);
            kb_ctrl_delete_client(fd);
        }
    }
    return 0;
}

int ms_kb_init()
{
    socket_init();
    kb_core_init();

    return 0;
}

int ms_kb_uinit()
{
    kb_core_uinit();
    socket_uinit();

    return 0;
}


