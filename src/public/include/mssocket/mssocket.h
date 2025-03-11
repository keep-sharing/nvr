#ifndef __MS_SOCKET_H__
#define __MS_SOCKET_H__

#include "osa/osa_typedef.h"

#define SOCK_MSSERVER_PATH "/var/run/mssocket"
#define SOCK_PLAYBACK_PATH "/var/run/playback"
//#define SOCK_AVSERVER_PATH "/var/run/avsocket"

enum ms_socket_type {
    SOCKET_TYPE_NONE = 0,
    SOCKET_TYPE_CORE = 1,
    SOCKET_TYPE_GUI  = 2,
    SOCKET_TYPE_WEB  = 3,
    SOCKET_TYPE_FIO  = 4,
    SOCKET_TYPE_SDK  = 5,
    SOCKET_TYPE_CLI  = 6,
    SOCKET_TYPE_P2P  = 7,
    SOCKET_TYPE_MSSP = 8,
    SOCKET_TYPE_URL  = 9,
    SOCKET_TYPE_KB   = 10,
    SOCKET_TYPE_JS   = 11,
    SOCKET_TYPE_CONN_MAX,
};

// communication within the mscore process
typedef enum MscoreCommMod {
    MSCORE_COMM_MOD_CORE  = 0,
    MSCORE_COMM_MOD_GUI   = 1,
    MSCORE_COMM_MOD_P2P   = 2,
    MSCORE_COMM_MOD_MAX   = 3,
} MSCORE_COMM_MOD_E;

//1.5M
#define SOCKET_DATA_MAX_SIZE        1572864
#define MAX_CLIENTS                 (SOCKET_TYPE_CONN_MAX)

#define SOCKET_CONDITION_CODE       0x20120610

#define SOCKET_MSG_CTRL_CONNECT     0xFFFFFF00
#define SOCKET_MSG_CTRL_DISCONNECT  0xFFFFFF01
#define SOCKET_MSG_CTRL_BUFFER_FULL (-11)
#define SOCKET_MSG_CTRL_RESEND_CNT  (20)

#define FD_MAX      (2048)
enum client_state {
    CLI_STATE_DISCONNECT = 0,
    CLI_STATE_ACCEPT,
    CLI_STATE_CONNECT,
};

enum independent_type {
    IND_NO = 0,
    IND_YES,
};

struct packet_header {
    UInt64 clientId;
    int condition;
    int type;
    int from;
    int size;
    int count;
    int crc;
    int independent;  //do thread (Yes:1  No:0)
    int reserve;
};

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*MS_SOCK_CALLBACK_NEW_MSG)(void *arg, struct packet_header *header, void *data);
int ms_sock_send_msg(int sendto, int type, void *data, int size, UInt64 clientId);
int ms_sock_send_msg_wait(int type, void *data, int size, enum ms_socket_type sock_type, const char *sun_path);
int ms_set_socket_state(int state);
int ms_client_socket_state();
int ms_sock_init(MS_SOCK_CALLBACK_NEW_MSG callback, void *arg, enum ms_socket_type socket_type, const char *sun_path);
int ms_sock_uninit();
void mssock_set_debug(int value);
int ms_get_sock_independent(int type);
int ms_get_sock_more_than_cnt(int type);
int ms_sock_set_cloexec(int sockfd);

#ifdef __cplusplus
}
#endif

#endif
