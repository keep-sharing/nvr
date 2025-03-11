#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include "msstd.h"
#include "mssocket.h"
#include "kb_socket.h"
#include "mskb.h"

#define SOCKET_PORT     (34567)
#define LISTEN_MAX      MAX_CLIENT
#define EPOLL_MAX       (2048)
#define EVENT_MAX       (20)
#define BUFF_SIZE       (1024)

typedef struct kb_skt_s {
    SOCKET serverfd;
    SOCKET clientfd;
    SOCKET epfd;
    pthread_t phtread;
    char *recv_buf;
    SKT_CB_S cb;
    int start;
} KB_SKT_S;

typedef struct kb_client_ver {
    int fd;
    char ipaddr[64];
    int version;
} KB_CLIENT_VER;

typedef struct kb_accept_client {
    int iswait_ver;
    KB_CLIENT_VER client[MAX_CLIENT];
} KB_ACCEPT_CLIENT;


//#define KB_CLIENT_IP   "192.168.9.177"
static KB_SKT_S g_socket = {0};

static void kb_socket_noblock(SOCKET sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

static int kb_socket_create(int af, int type, int protocol)
{
    SOCKET sock = socket(af, type, protocol);
    if (sock == INVALID_SOCKET) {
        return -1;
    }

    return sock;
}

static int kb_socket_destroy(SOCKET s)
{
    if (s <= 0) {
        return -1;
    }

    close(s);

    return 0;
}

int kb_socket_connect(SOCKET s, const char *ip, int port)
{
    struct sockaddr_in addr;

    if (s <= 0) {
        return -1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);
    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        kbdebug("connect to ip:%s port:%d fail", ip, port);
        perror("connect");
        return -1;
    }

    printf("connect to ip:%s port:%d successful\n", ip, port);

    return 0;
}

int kb_socket_disconnect(SOCKET s, SOCKET c)
{
    return 0;
}

static int kb_socket_bind(SOCKET s, int port)
{
    struct sockaddr_in addr;
    int opt =  1;

    if (s <= 0) {
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        return -1;
    }

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        kbdebug("bind to port:%d fail", port);
        return -1;
    }

    printf("bind to port:%d successful\n", port);

    return 0;
}

static int kb_socket_listen(SOCKET s, int max)
{
    if (s <= 0) {
        return -1;
    }

    return listen(s, max);
}

static int kb_socket_accept(SOCKET s, char *ipaddr, int len)
{
    SOCKET clisock;
    struct sockaddr_in cliaddr;
    socklen_t addrlen = 0;
    //char peer[64] = {0};

    if (s <= 0) {
        return -1;
    }

    addrlen = sizeof(struct sockaddr_in);
    clisock = accept(s, (struct sockaddr *)&cliaddr, &addrlen);
    if (clisock == INVALID_SOCKET) {
        kbdebug("accept new client fail");
        return -1;
    }

    snprintf(ipaddr, len, "%s", inet_ntoa(cliaddr.sin_addr));
    printf("accept new client:%s\n", ipaddr);

    return clisock;
}

static int kb_socket_send(SOCKET s, void *buf, int len, int flags)
{
    int sendcount = 0;
    if (s <= 0 || !buf || len <= 0) {
        return -1;
    }

    while (sendcount < len) {
        int result = send(s, (const char *)(buf) + sendcount, len - sendcount, flags);
        if (result <= 0) {
            return -1;
        }
        sendcount += result;
    }

    return sendcount;
}

static int kb_socket_recv(SOCKET s, void *buf, int len, int flags)
{
    if (s <= 0) {
        return -1;
    }

    return (recv(s, (char *)(buf), len, flags));
}

static SOCKET kb_epoll_create(int epmax)
{
    return epoll_create(epmax);
}

static SOCKET kb_epoll_destroy(SOCKET epfd)
{
    return close(epfd);
}

static void kb_epoll_add(SOCKET epfd, SOCKET client, U32 event)
{
    struct epoll_event ev;

    ev.data.fd = client;
    ev.events = event | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
}

static void kb_epoll_del(SOCKET epfd, SOCKET client, U32 event)
{
    struct epoll_event ev;

    ev.data.fd = client;
    ev.events = event | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_DEL, client, &ev);
}

#ifdef KB_CLIENT_IP
static void *kb_socket_pthread(void *argv)
{
    int i, n;
    int nfds = 0;
    struct epoll_event events[EVENT_MAX];
    KB_SKT_S *socketInfo = (KB_SKT_S *)argv;

    socketInfo->epfd = kb_epoll_create(EPOLL_MAX);
    kb_epoll_add(socketInfo->epfd, socketInfo->serverfd, EPOLLIN);
//    socketInfo->clientfd = socketInfo->serverfd;

    ms_task_set_name("kb_socket_pthread");

    while (socketInfo->start) {
        nfds = epoll_wait(socketInfo->epfd, events, EVENT_MAX, 500);
        for (i = 0; i < nfds; ++i) {
            if (events[i].data.fd < 0) {
                continue;
            }
            if (events[i].data.fd == socketInfo->serverfd && socketInfo->clientfd == 0) { //only for one client to connect
                kbdebug("ACCPET");
                socketInfo->clientfd = socketInfo->serverfd;
                if (socketInfo->cb.state) {
                    socketInfo->cb.state(events[i].data.fd, NULL, KB_STATE_CONNECTED);
                }
            }
            if ((events[i].events & EPOLLIN) && events[i].data.fd) {
                n = kb_socket_recv(events[i].data.fd, (char *)socketInfo->recv_buf, BUFF_SIZE, 0);
                if (n <= 0) {
                    kb_epoll_del(socketInfo->epfd, events[i].data.fd, EPOLLIN);
                    if (socketInfo->cb.state) {
                        socketInfo->cb.state(events[i].data.fd, NULL, KB_STATE_DISCONNECTED);
                    }
                    close(events[i].data.fd);
                    kbdebug("DISCONNECTED");
                    continue;
                } else if (socketInfo->cb.recv) {
                    socketInfo->cb.recv(events[i].data.fd, socketInfo->recv_buf, n);
                }
            }
        }
    }

    if (socketInfo->cb.state) {
        socketInfo->cb.state(socketInfo->serverfd, NULL, KB_STATE_DISCONNECTED);
    }

    kb_epoll_del(socketInfo->epfd, socketInfo->serverfd, EPOLLIN);
    kb_epoll_destroy(socketInfo->epfd);
//    socketInfo->clientfd = 0;

    return 0;
}

#else
static void *kb_socket_pthread(void *argv)
{
    int i, n, j;
    int nfds = 0;
    int clear = 0;
    int toNext = 0;
    char ipaddr[64] = {0};

    struct epoll_event events[EVENT_MAX];
    KB_SKT_S *socketInfo = (KB_SKT_S *)argv;

    socketInfo->epfd = kb_epoll_create(EPOLL_MAX);
    kb_epoll_add(socketInfo->epfd, socketInfo->serverfd, EPOLLIN);

    KB_ACCEPT_CLIENT new_client;
    new_client.iswait_ver = 0;
    for (j = 0; j < MAX_CLIENT; j++) {
        new_client.client[j].fd = -1;
        new_client.client[j].version = -1;
        memset(new_client.client[j].ipaddr, 0, sizeof(new_client.client[j].ipaddr));
    }

    ms_task_set_name("kb_socket_pthread");

    while (socketInfo->start) {
        nfds = epoll_wait(socketInfo->epfd, events, EVENT_MAX, 500);
        for (i = 0; i < nfds; ++i) {
            if (events[i].data.fd < 0) {
                continue;
            }

            if (new_client.iswait_ver) {
                for (j = 0; j < MAX_CLIENT; j++) {
                    if (new_client.client[j].fd == events[i].data.fd) {
                        if (events[i].events & EPOLLIN) {
                            n = kb_socket_recv(new_client.client[j].fd, (char *)socketInfo->recv_buf, BUFF_SIZE, 0);
                            if (n > 0 && socketInfo->cb.version) {
                                new_client.client[j].version = socketInfo->cb.version(new_client.client[j].fd, new_client.client[j].ipaddr,
                                                                                      socketInfo->recv_buf, n, new_client.client[j].version);
                                printf("11111 ipaddr:[%s], version:[%d]\n", new_client.client[j].ipaddr, new_client.client[j].version);
                                break;
                            }
                        }
                        new_client.client[j].version = socketInfo->cb.version(new_client.client[j].fd, new_client.client[j].ipaddr, NULL, 0,
                                                                              new_client.client[j].version);
                        printf("22222 ipaddr:[%s], version:[%d]\n", new_client.client[j].ipaddr, new_client.client[j].version);
                        break;
                    }
                }

                if (j < MAX_CLIENT) {
                    if (new_client.client[j].version >= 0) { // reconize finished
                        new_client.client[j].fd = -1;
                        memset(new_client.client[j].ipaddr, 0, sizeof(new_client.client[j].ipaddr));
                    }
                    toNext = 1;
                    clear = 50;
                    printf("33333 fd:[%d], ipaddr:[%s], version:[%d]\n", new_client.client[j].fd, new_client.client[j].ipaddr,
                           new_client.client[j].version);
                }

                if (!clear--) {
                    for (j = 0; j < MAX_CLIENT; j++) {
                        if (new_client.client[j].fd > 0) {
                            kb_epoll_del(socketInfo->epfd, new_client.client[j].fd, EPOLLIN);
                            close(new_client.client[j].fd);
                        }
                        new_client.client[j].fd = -1;
                    }
                }

                new_client.iswait_ver = 0;
                for (j = 0; j < MAX_CLIENT; j++) {
                    if (new_client.client[j].fd >= 0) {
                        new_client.iswait_ver = 1;
                        break;
                    }
                }
//                printf("##### iswait_ver:[%d]; toNext:[%d]\n", new_client.iswait_ver, toNext);
                if (toNext) {
                    toNext = 0;
                    continue;
                }
            }
            if (events[i].data.fd == socketInfo->serverfd) { //&& socketInfo->clientfd == 0) //only for one client to connect
                socketInfo->clientfd = kb_socket_accept(socketInfo->serverfd, ipaddr, sizeof(ipaddr));
                kb_epoll_add(socketInfo->epfd, socketInfo->clientfd, EPOLLIN);

                for (j = 0; j < MAX_CLIENT; j++) {
                    if (new_client.client[j].fd < 0) {
                        clear = 50;
                        new_client.iswait_ver = 1;
                        new_client.client[j].version = -2;
                        new_client.client[j].fd = socketInfo->clientfd;
                        snprintf(new_client.client[j].ipaddr, sizeof(new_client.client[j].ipaddr), "%s", ipaddr);
                        break;
                    }
                }
                if (j < MAX_CLIENT) {
                    printf("##### ACCPET  iswait_ver:[%d], fd:[%d]; ipaddr:[%s]; j:[%d]; version:[%d]\n", new_client.iswait_ver,
                           new_client.client[j].fd, new_client.client[j].ipaddr, j, new_client.client[j].version);
                    socketInfo->cb.version(new_client.client[j].fd, new_client.client[j].ipaddr, NULL, 0, new_client.client[j].version);
                }
                continue;
            } else if ((events[i].events & EPOLLIN) && events[i].data.fd != socketInfo->serverfd) {
                n = kb_socket_recv(events[i].data.fd, (char *)socketInfo->recv_buf, BUFF_SIZE, 0);
                if (n <= 0) {
                    kb_epoll_del(socketInfo->epfd, events[i].data.fd, EPOLLIN);
                    if (socketInfo->cb.state) {
                        socketInfo->cb.state(events[i].data.fd, NULL, KB_STATE_DISCONNECTED);
                    }
                    close(events[i].data.fd);
                    socketInfo->clientfd = 0;
                    kbdebug("DISCONNECTED");
                    continue;
                } else if (socketInfo->cb.recv) {
                    socketInfo->cb.recv(events[i].data.fd, socketInfo->recv_buf, n);
                }
            }
        }
    }

    kb_epoll_del(socketInfo->epfd, socketInfo->serverfd, EPOLLIN);
    kb_epoll_destroy(socketInfo->epfd);

    return 0;
}

#endif

void kb_socket_client_close(SOCKET clientFd)
{
    kb_epoll_del(g_socket.epfd, clientFd, EPOLLIN);
    close(clientFd);
}

int kb_socket_send_msg(SOCKET fd, void *buf, int len)
{
    int ret = 0;

    ret = kb_socket_send(fd, buf, len, 0);
    kbdebug("send_msg_to_kb:ret:[%d], fd:[%d], len:[%d], buf:[%s]\n", ret, fd, len, (char *)buf);
    return ret;
}

int kb_msg_to_display(SOCKET fd, const char *data, int row, int col, int len)
{
    char dsp[64];
    int ret = 0;

    if (row > 4 || col > 16 || data == NULL) {
        return -1;
    }

    if (g_kb_version == 1) {
        row = (row * 2) + 5;
        col += 12;
    }
    sprintf(dsp, "Display:R%02d-C%02d-%-*.*s:EOF", row, col, len, len, data);
    ret = kb_socket_send_msg(fd, dsp, strlen(dsp));
    return ret;
}

int kb_msg_line_clear(SOCKET fd, int row, int col, int len)
{
    char dsp[64];
    int ret = 0;

    if (row > 15 || col > 40) {
        return -1;
    }

    sprintf(dsp, "Display:R%02d-C%02d-%-*.*s:EOF", row, col, len, len, " ");
    ret = kb_socket_send_msg(fd, dsp, strlen(dsp));
    return ret;
}


int kb_socket_init(SKT_CB_S *pstSktCb)
{
    memset(&g_socket, 0, sizeof(KB_SKT_S));

    g_socket.recv_buf = ms_malloc(BUFF_SIZE);
    if (g_socket.recv_buf == NULL) {
        kbdebug("recv_buf ms_malloc failed !");
        return -1;
    }

    g_socket.cb.recv = pstSktCb->recv;
    g_socket.cb.state = pstSktCb->state;
    g_socket.cb.version = pstSktCb->version;

    g_socket.serverfd = kb_socket_create(AF_INET, SOCK_STREAM, 0);
#ifdef KB_CLIENT_IP
    kb_socket_connect(g_socket.serverfd, KB_CLIENT_IP, SOCKET_PORT);
#else
    kb_socket_noblock(g_socket.serverfd);
    kb_socket_bind(g_socket.serverfd, SOCKET_PORT);
    kb_socket_listen(g_socket.serverfd, LISTEN_MAX);
#endif
    g_socket.start = 1;
    ms_task_create_join(&g_socket.phtread, kb_socket_pthread, &g_socket);

    return 0;
}

int kb_socket_uinit()
{
    g_socket.start = 0;
    ms_task_join(&g_socket.phtread);

    kb_socket_destroy(g_socket.serverfd);

    if (g_socket.recv_buf) {
        ms_free(g_socket.recv_buf);
        g_socket.recv_buf = NULL;
    }

    return 0;
}

