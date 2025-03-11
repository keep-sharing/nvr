#ifndef _MS_P2P_H_
#define _MS_P2P_H_
#include "socket.h"

#define P2P_SERVPORT	9999
#define P2P_SERVADDR	"192.168.6.2"
#define P2P_SERVNAME	"p2p1.recovision.com"

int p2p_create_keyfile(void);
int p2p_serv_switch_keyfile(SOCKET s);
int p2p_cli_switch_keyfile(SOCKET s);
int p2p_send(SOCKET s, void *data, int dsize, void *sbuf, int ssize);
int p2p_recv(SOCKET s, void *data, int size, void *rbuf, int rsize);

#endif
