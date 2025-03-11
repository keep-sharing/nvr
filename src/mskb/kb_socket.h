/*
 * ***************************************************************
 * Filename:        kb_socket.h
 * Created at:      2017.01.17
 * Description:     milesight keyboard socket communication.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __KB_SCOKET_H__
#define __KB_SCOKET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "kb_util.h"


#define SOCKET  int
#define INVALID_SOCKET  -1
#define MAX_CLIENT      (16)
typedef enum kb_state_e {
    KB_STATE_DISCONNECTED = 0,
    KB_STATE_CONNECTED,
    KB_STATE_NUM,
} KB_STATE_E;

typedef struct  skt_cb_s {
    int (*recv)(int fd, void *data, int size);
    void (*state)(int fd, char *ipaddr, KB_STATE_E enState);
    int (*version)(int fd, char *ipaddr, void *recvbuf, int size, int ver);
} SKT_CB_S;

int kb_socket_init(SKT_CB_S *pstSktCb);
int kb_socket_uinit();
int kb_socket_send_msg(SOCKET fd, void *buf, int len);
int kb_msg_to_display(SOCKET fd, const char *data, int row, int col, int len);
int kb_msg_line_clear(SOCKET fd, int row, int col, int len);
void kb_socket_client_close(SOCKET clientFd);
#ifdef __cplusplus
}
#endif

#endif



