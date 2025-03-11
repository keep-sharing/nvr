#ifndef __NO_SOCKET_H__
#define __NO_SOCKET_H__

#ifdef _MS_WINDOWS_
#include <winsock2.h>
#define _WINDOWS_SOCKET_
#else 
#include<sys/types.h>  
#include<sys/socket.h>  
#include<netinet/in.h>
#define SOCKET 	int
#define INVALID_SOCKET 	(-1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

int socket_create(int af, int type, int protocol, SOCKET* sock);
int socket_destroy(SOCKET s);
int socket_connect(SOCKET s, const char* ip, int port);
int socket_bind(SOCKET s, int port);
int socket_listen(SOCKET s, int backlog);
int socket_accept(SOCKET s, struct sockaddr_in* cliaddr, SOCKET* clisock);
int socket_send(SOCKET s, void* buf, int len, int flags);
int socket_recv(SOCKET s, void* buf, int len, int flags);
int socket_set_send_timeout(SOCKET s, int msecs);
int sock_gethostbyname(char* name, char ip[32]);
SOCKET sock_creat_tcp(int port);

#ifdef __cplusplus
}
#endif

#endif
