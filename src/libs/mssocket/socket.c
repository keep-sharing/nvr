#include "socket.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>


#ifdef _WINDOWS_SOCKET_
#include<winsock2.h>   
#pragma comment(lib, "ws2_32.lib") 
#define snprintf _snprintf
#else
#include <netdb.h>
#endif 

static int global_debugon = 0;
static void debug_info(char* format, ...)
{
	char debug[512] = {0};
	va_list  va;
	if (!global_debugon)
		return;
	snprintf(debug, sizeof(debug), "[socket debug] ");
	va_start (va, format);
	vsprintf((char*)(debug)+strlen(debug), format, va);
	va_end(va);
	printf("%s\n", debug);
}


int socket_create(int af, int type, int protocol, SOCKET* sock)
{
#ifdef _WINDOWS_SOCKET_
	WSADATA wsa = {0};
    WORD version = MAKEWORD(2, 2);  
    if (WSAStartup(version, &wsa))
	{
		return -1;
	}
#endif	
	*sock = socket(af, type, protocol);
	if (*sock == INVALID_SOCKET)
		return -1;
	printf("socket_create. af:%d type:%d protocol:%d sock:%d\n", af, type, protocol, *sock);
	return 0;
}

int socket_destroy(SOCKET s)
{
	if (s <= 0) 
		return -1;
#ifdef _WINDOWS_SOCKET_
	closesocket(s);
	WSACleanup();
	return 0;
#else
	close(s);
	return 0;
#endif
}

int socket_connect(SOCKET s, const char* ip, int port)
{
	struct sockaddr_in addr;  
	if (s <= 0)
		return -1;
    addr.sin_family = AF_INET;  
    addr.sin_addr.s_addr = inet_addr(ip);  
    addr.sin_port = htons(port);
	if (connect(s, (struct sockaddr*)(&addr), sizeof(addr)) != 0)
	{
		debug_info("connect to ip:%s port:%d fail", ip, port);
		return -1;
	}
	debug_info("connect to ip:%s port:%d successful", ip, port);
	return 0;
}

int socket_bind(SOCKET s, int port)
{
	struct sockaddr_in addr;  
	int opt =  1; 
	if (s <= 0)
		return -1;
	addr.sin_family = AF_INET;  
	addr.sin_addr.s_addr = INADDR_ANY;  
	addr.sin_port = htons(port);   
    	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0 ) 
		return -1;
	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) != 0)
	{
		debug_info("bind to port:%d fail", port);
		return -1;
	}
	debug_info("bind to port:%d successful", port);
    	return 0;
}

int socket_listen(SOCKET s, int backlog)
{
	if (s <= 0) 
		return -1;
	return listen(s, backlog);  
}

int socket_accept(SOCKET s, struct sockaddr_in* cliaddr, SOCKET* clisock)
{
	int addrlen = 0;
	char peer[64] = {0};
	if (s<=0 || !cliaddr) return -1;
	addrlen = sizeof(struct sockaddr_in);
	*clisock = accept(s, (struct sockaddr*)cliaddr, (socklen_t *)&addrlen);
	if (*clisock == INVALID_SOCKET)
	{
		debug_info("accept new client fail");
		return -1;
	}
	snprintf(peer, sizeof(peer), "%s", (char*)inet_ntoa(cliaddr->sin_addr)); 
	debug_info("accept new client:%s", peer);
    	return 0;
}

int socket_send(SOCKET s, void* buf, int len, int flags)
{
	int sendlen = 0;  
	if(s<=0 || !buf || len<=0) 
		return -1;
    	while(sendlen < len)
	{  
		int result = send(s, (const char*)(buf) + sendlen, len - sendlen, flags);  
		if (result <= 0)
			return -1;
		sendlen += result;  
    	}   
	return sendlen;
}

int socket_recv(SOCKET s, void* buf, int len, int flags)
{
	int recvlen = 0;
	if (s <= 0)
		return -1;
	while(recvlen < len)
	{
		int result = recv(s, (char*)(buf) + recvlen, len - recvlen, flags);
		if (result <= 0) return -1;
		recvlen += result;
	}
	return recvlen;  
}

int socket_set_recv_timeout(SOCKET s, int msecs)
{
	if (s <= 0) 
		return -1;
#ifdef _WINDOWS_SOCKET_
	return setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (void *)&msecs, sizeof(msecs));
#else
	struct timeval val = {0};
	val.tv_sec = msecs / 1000;
	val.tv_usec = (msecs % 1000) * 1000;
	return setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (void *)&val, sizeof(val));
#endif
}

int socket_set_send_timeout(SOCKET s, int msecs)
{
	if (s <= 0)
		return -1;
#ifdef _WINDOWS_SOCKET_
	return setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (void *)&msecs, sizeof(msecs));
#else
	struct timeval val = {0};
	val.tv_sec = msecs / 1000;
	val.tv_usec = (msecs % 1000) * 1000;
	return setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (void *)&val, sizeof(val));
#endif
}

int sock_gethostbyname(char* name, char ip[32])
{
	struct hostent* host = NULL;
	struct in_addr addr;
	memset(&addr, 0, sizeof(addr));
#ifdef _WINDOWS_SOCKET_
	int idx = 0;
	WSADATA wsa = {0};
	WORD version = MAKEWORD(2, 2);  
	if (WSAStartup(version, &wsa))
		return -1;
	if (!name || !ip)
		return -1;
	host = gethostbyname(name);
	if (!host)
	{
		debug_info("get host by name:%s error", name);
		return -1;
	}
	if (host->h_addrtype != AF_INET)
		return -1;
	if (host->h_addr_list[idx] != 0)
	{
		addr.s_addr = *(u_long *) host->h_addr_list[idx++];
		snprintf(ip, 32, "%s", inet_ntoa(addr));
	}
#else
	if (!name || !ip)
		return -1;
	host = gethostbyname(name);
	if (!host)
	{
		debug_info("get host by name:%s error", name);
		return -1;
	}
	if (inet_ntop(host->h_addrtype, *(host->h_addr_list), ip, (socklen_t)32) == NULL) 
	{
		debug_info("inet_ntop error", name);
		return -1;
	}
#endif
	debug_info("get host by name:%s host:%s", name, ip);
	return 0;
}

int sock_creat_tcp(int port)
{
    int sockOpt = 1;
    int sockfd = 0;
    struct sockaddr_in servaddr = {0};
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        return -1;
    }

    int tFlags = fcntl(sockfd, F_GETFD);
    fcntl(sockfd, F_SETFD, tFlags|FD_CLOEXEC);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &sockOpt,sizeof (sockOpt));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        close(sockfd);
        return -2;
    }
    
    if (listen(sockfd, 128) == -1)
    {
        close(sockfd);
        return -3;
    }
    
    return sockfd;
}

