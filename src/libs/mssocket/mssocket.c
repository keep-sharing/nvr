#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/un.h>
#include "mssocket.h"
#include "msstd.h"
#include <errno.h>
#include <sys/epoll.h>
#include <msg.h>

#if 0
    //#define MS_SOCK_DEBUG
    #ifdef MS_SOCK_DEBUG
        #define SOCK_PRINTF  printf
    #else
        #define SOCK_PRINTF(...)
    #endif
#endif

//#define MS_SOCK_USE_PTHREAD_JOIN
#ifdef MS_SOCK_USE_PTHREAD_JOIN
    #define MS_SOCK_PTHREAD_JOIN
#else
    #undef MS_SOCK_PTHREAD_JOIN
#endif
#define MS_SOCK_SEND_LONG_TIMES_OUT (3)

int g_mssock_debug = 0;
#define SOCK_PRINTF(msg,args...) \
    { if (g_mssock_debug || (access("/mnt/nand/mssock.txt", 0)!= -1) ) \
            msdebug(DEBUG_INF,msg,##args);}

//#define HARD_TEST
static int global_socket_state = 0;
//#define MS_SOCK_ADDR  "/var/run/mssocket" //bruce.milesight

void mssock_set_debug(int value)
{
    g_mssock_debug = value;
}

static int read_from_tcp(int fd, char *buf, int size)
{
    int offset = 0;
    int bytes;
    int wanted = size;

    while (wanted > 0) {
        bytes = read(fd, buf + offset, wanted);
        if (bytes <= 0) {
            return -1;
        }
        wanted -= bytes;
        offset += bytes;
    }
    return 0;
}

//has timeout
static int read_from_tcp_ex(int fd, char *buf, int size)
{
    int tryCnt = 0;
    int offset = 0;
    int bytes;
    int wanted = size;
    while (wanted > 0) {
        bytes = read(fd, buf + offset, wanted);
        //printf("######### size:%d offset:%d bytes:%d errno:%d tryCnt:%d #########\n", size, offset, bytes, errno, tryCnt);
        if (bytes <= 0) {
            if (errno == EAGAIN) {
                tryCnt++;
                if (tryCnt >= 3) {
                    return -1;
                }
            } else {
                return -1;
            }
        } else {
            tryCnt = 0;
            wanted -= bytes;
            offset += bytes;
        }
    }

    return 0;
}


//server
struct sock_client_node {
    int fd;
    struct sockaddr_un addr_un;
    pthread_mutex_t lock;
    int state;
    int from;
};

struct sock_server_params {
    int sockfd;
    int epfd;
    struct sockaddr_un addr_un;
    MS_SOCK_CALLBACK_NEW_MSG callback;
    void *arg;
    pthread_t thread;
    pthread_mutex_t lock;
    struct sock_client_node client[MAX_CLIENTS];
    fd_set readfds;
    char *send_buf;
    char *recv_buf;
#ifdef MS_SOCK_PTHREAD_JOIN
    int thread_run;
#endif
};
static struct sock_server_params *global_server;

static int get_new_client_node(struct sock_server_params *server, int fd, struct sockaddr_un *addr)
{
    int i;
    char buf[sizeof(struct packet_header) + sizeof(int)];
    struct packet_header *header;
    struct epoll_event ev;
    int *msg;

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->client[i].state == CLI_STATE_DISCONNECT) {
            pthread_mutex_lock(&server->client[i].lock);

            ev.data.fd = fd;
            ev.events = EPOLLIN; //read
            epoll_ctl(server->epfd, EPOLL_CTL_ADD, fd, &ev);

            server->client[i].fd = fd;
            memcpy(&server->client[i].addr_un, addr, sizeof(struct sockaddr_un));
            server->client[i].state = CLI_STATE_ACCEPT;
            header = (struct packet_header *)buf;
            header->type = SOCKET_MSG_CTRL_CONNECT;
            header->condition = SOCKET_CONDITION_CODE;
            header->from = SOCKET_TYPE_CORE;
            header->size = sizeof(int);
            header->count = 0;
            msg = (int *)(buf + sizeof(struct packet_header));
            *msg = i + 1;
            //printf("[MSSOCK]line :%d, SER get_new_client_node fd [%d] No:%d\n", __LINE__ , fd, i);
            write(fd, buf, sizeof(buf));
            pthread_mutex_unlock(&server->client[i].lock);
            return i;
        }
    }
    return -1;
}

static int need_check_reqs_type(int type)
{
    if (type == RESPONSE_FLAG_GET_REMOTEPB_DOWNLOAD ||
        type == RESPONSE_FLAG_REMOTE_BACKUP_START ||
        type == RESPONSE_FLAG_REMOTE_BACKUP_START_DATA) {
        return 1;
    }
    return 0;
}

static int set_client_node(struct sock_server_params *server, int fd, int from)
{
    int i;
    int res = -1;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->client[i].fd == fd && server->client[i].state != CLI_STATE_DISCONNECT) {
            pthread_mutex_lock(&server->client[i].lock);
            server->client[i].from = from;
            server->client[i].state = CLI_STATE_CONNECT;
            pthread_mutex_unlock(&server->client[i].lock);
            break;
        }
    }
    return res;
}

static int get_client_node_by_fd(struct sock_server_params *server, int fd)
{
    int i;
    int res = -1;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (server->client[i].fd == fd && server->client[i].state != CLI_STATE_DISCONNECT) {
            res = i;
            break;
        }
    }
    return res;
}

static int get_client_node_by_from(struct sock_server_params *server, int from)
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++) {
        pthread_mutex_lock(&server->client[i].lock);
        if (server->client[i].state != CLI_STATE_DISCONNECT) {
            if ((0x0000FFFF & server->client[i].from) == (0x0000FFFF & from)) {
                pthread_mutex_unlock(&server->client[i].lock);
                return i;
            }
        }
        pthread_mutex_unlock(&server->client[i].lock);
    }
    return -1;
}

static int del_client_node(struct sock_server_params *server, int fd)
{
    int i;
    int res = -1;
    struct epoll_event ev;

    for (i = 0; i < MAX_CLIENTS; i++) {
        SOCK_PRINTF("server->client[%d].fd = %d= [%d]=\n\n", i, server->client[i].fd, fd);
        if (server->client[i].fd == fd) {
            pthread_mutex_lock(&server->client[i].lock);

            ev.data.fd = fd;
            ev.events = EPOLLIN; //read
            epoll_ctl(server->epfd, EPOLL_CTL_DEL, fd, &ev);

            server->client[i].state = CLI_STATE_DISCONNECT;
            server->client[i].from = 0;

            close(server->client[i].fd);
            pthread_mutex_unlock(&server->client[i].lock);
            res = 0;
            break;
        }
    }

    return res;
}

static int fd_is_read(int fd, struct epoll_event *pEvent, int num)
{
    int i = 0;
    int res = 0;

    for (i = 0; i < num; i++) {
        if (fd == pEvent[i].data.fd && (pEvent[i].events & EPOLLIN)) {
            res = 1;
            break;
        }
    }

    return res;
}

static void *sock_server_listen(void *arg)
{
    int res;
    int i;
    int num = 0;
#ifndef MS_SOCK_PTHREAD_JOIN
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
#endif

    struct epoll_event events[MAX_CLIENTS];

    struct sock_server_params *server;
    server = (struct sock_server_params *)arg;

    listen(server->sockfd, 50);

    char *recvbuf = server->recv_buf;

    int len = 0;
    struct sockaddr_un address;

#ifdef MS_SOCK_PTHREAD_JOIN
    while (server->thread_run)
#else
    while (1)
#endif
    {
        int fd;
        num = 0;
        num = epoll_wait(server->epfd, events, FD_MAX, 100);
#ifndef MS_SOCK_PTHREAD_JOIN
        pthread_testcancel();
#endif
        if (num < 1) {
            continue;
        }
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (server->client[i].state == CLI_STATE_DISCONNECT) {
                SOCK_PRINTF("[MSSOCK] server->client[%d].state = %d!\n", i, server->client[i].state);
                continue;
            }

            if (!fd_is_read(server->client[i].fd, events, num)) {
                SOCK_PRINTF("[MSSOCK] server->client[%d].fd = %d is no set!\n", i, server->client[i].fd);
                continue;
            }

            fd = server->client[i].fd;
            struct packet_header header;
            memset(&header, 0, sizeof(struct packet_header));
            res = read_from_tcp(fd, (char *)&header, sizeof(struct packet_header));
            if (res < 0) {
                SOCK_PRINTF("[MSSOCK] del_client_node MSG type!\n");
                del_client_node(server, fd);
                continue;
            }

            if (header.size > SOCKET_DATA_MAX_SIZE || header.condition != SOCKET_CONDITION_CODE) {
                SOCK_PRINTF("[MSSOCK] unkown MSG type!\n");
                del_client_node(server, fd);
                continue;
            }

            if (header.size > 0) {
                res = read_from_tcp(fd, recvbuf, header.size);
                if (res < 0) {
                    del_client_node(server, fd);
                    continue;
                }
                if (header.size < SOCKET_DATA_MAX_SIZE) {
                    recvbuf[header.size] = '\0';
                }
            }

            i = get_client_node_by_fd(server, fd);
            if (i < 0) {
                del_client_node(server, fd);
                continue;
            }
            if (server->client[i].state == CLI_STATE_ACCEPT) {
                if (header.type == SOCKET_MSG_CTRL_CONNECT) {
                    set_client_node(server, fd, header.from);
                } else {
                    del_client_node(server, fd);
                }
                continue;
            }

            //bruce.milesight add  version:2.0.7 2014.10.17
            if (server->client[i].state == CLI_STATE_CONNECT) {
                if (header.type == SOCKET_MSG_CTRL_DISCONNECT) {
                    del_client_node(server, fd);
                    continue;
                }
            }
            //bruce.milesight end

            if (server->client[i].state == CLI_STATE_CONNECT && server->callback != NULL) {
                //set_client_node(server, fd, header.from);
                server->callback(server->arg, &header, recvbuf);
            }
        }

        if (fd_is_read(server->sockfd, events, num)) { //(FD_ISSET(server->sockfd, &testfds))
            len = sizeof(address);
            fd = accept(server->sockfd, (struct sockaddr *)&address, (socklen_t *)&len);
            ms_sock_set_cloexec(fd);
            struct timeval val = {1, 0};
            setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &val, sizeof(val));
            if (get_new_client_node(server, fd, &address) == -1) {
                msprintf("[david debug] server listen fd is busy now!!!");
            }
        }
    }

    pthread_exit(NULL);
}

static int sock_init_server(MS_SOCK_CALLBACK_NEW_MSG callback, void *arg, int ipc_type, const char *sun_path)
{
    int res;
    int i;
    pthread_attr_t attr;
    struct sock_server_params *server;
    struct epoll_event ev;
    server = (struct sock_server_params *)ms_malloc(sizeof(struct sock_server_params));
    global_server = server;
    server->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    ms_sock_set_cloexec(server->sockfd);
    server->addr_un.sun_family = AF_UNIX;
    strcpy(server->addr_un.sun_path, sun_path);
    unlink(sun_path);
    server->callback = callback;
    server->arg = arg;
    server->send_buf = (char *)ms_malloc(SOCKET_DATA_MAX_SIZE + sizeof(struct packet_header));
    server->recv_buf = (char *)ms_malloc(SOCKET_DATA_MAX_SIZE);

    int reuse = 1;
    if (setsockopt(server->sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0) {
        SOCK_PRINTF("[MSSOCK] setsocketopt failed\n");
    }

    res = bind(server->sockfd, (struct sockaddr *)&server->addr_un, sizeof(server->addr_un));
    if (res) {
        SOCK_PRINTF("Bind error\n[MSSOCK] Server is already running\n");
        return -1;
    }
    server->epfd = epoll_create(FD_MAX);
    ev.data.fd = server->sockfd;
    ev.events = EPOLLIN; //read
    epoll_ctl(server->epfd, EPOLL_CTL_ADD, server->sockfd, &ev);

    pthread_mutex_init(&server->lock, NULL);
    for (i = 0; i < MAX_CLIENTS; i++) {
        pthread_mutex_init(&server->client[i].lock, NULL);
    }

#ifdef MS_SOCK_PTHREAD_JOIN
    server->thread_run = 1;
    res = ms_task_create_join(&server->thread, sock_server_listen, (void *)server);
#else
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, THREAD_STACK_1M_SIZE);
    res = pthread_create(&server->thread, &attr, sock_server_listen, (void *)server);
#endif
    if (res) {
        SOCK_PRINTF("[MSSOCK] pthread_create failed\n");
        return -1;
    }

    sleep(1);
    return 0;
}

static int sock_uninit_server()
{
    struct sock_server_params *server;
    int i;

    server = global_server;
    if (server != NULL) {
        pthread_mutex_lock(&server->lock);
#ifdef MS_SOCK_PTHREAD_JOIN
        server->thread_run = 0;
        ms_task_join(&server->thread);
#else
        pthread_cancel(server->thread);
#endif
        pthread_mutex_unlock(&server->lock);
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (server->client[i].state != CLI_STATE_DISCONNECT) {
                pthread_mutex_lock(&server->client[i].lock);
                if (server->client[i].fd > 0) {
                    close(server->client[i].fd);
                }
                server->client[i].fd = -1;
                server->client[i].state = CLI_STATE_DISCONNECT;
                pthread_mutex_unlock(&server->client[i].lock);
                pthread_mutex_destroy(&server->client[i].lock);//bruce.milesight add
            }
        }
        pthread_mutex_lock(&server->lock);
        if (server->sockfd > 0) {
            close(server->sockfd);
        }
        if (server->epfd > 0) {
            close(server->epfd);
        }
        server->sockfd = -1;
        server->epfd = -1;
        ms_free(server->send_buf);
        ms_free(server->recv_buf);
        pthread_mutex_unlock(&server->lock);
        pthread_mutex_destroy(&server->lock);//bruce.milesight add
        ms_free(server);
        global_server = NULL;
    }

    return 0;
}

static int server_send_msg(int sendto, int type, int size, void *data, UInt64 clientId)
{
    if (global_socket_state == 1) {
        return 0;
    }

    int index = -1;
    static int count;
    struct sock_server_params *server;
    struct packet_header *header;
    struct epoll_event ev;
    int len;
    int i;
    int ret = 0;
    //fd_set testfds;

    server = global_server;
    if (server == NULL) {
        printf("[MSSOCK]: connect error.sendto:%d\n", sendto);
        return -1;
    }

    if (size > SOCKET_DATA_MAX_SIZE) {
        printf("[MSSOCK]: Data size(%d) is too big\n", size);
        return -1;
    }
    i = get_client_node_by_from(server, sendto);
    if (i < 0) {
        printf("[MSSOCK]: can not find the client %x msg:%d\n", sendto, type);
        return -1;
    }
    if (type == SOCKET_MSG_CTRL_DISCONNECT) {
        printf("[MSSOCK]: SOCKET_MSG_CTRL_DISCONNECT connect error.sendto: \n");
        del_client_node(server, server->client[i].fd);
        return -1;
    }

    pthread_mutex_lock(&server->client[i].lock);
    int client_state = server->client[i].state;
    int client_fd = server->client[i].fd;
    pthread_mutex_unlock(&server->client[i].lock);

    len = sizeof(struct packet_header);
    if (size > 0) {
        len = sizeof(struct packet_header) + size;
    }
    //pthread_mutex_lock(&server->client[i].lock);
    pthread_mutex_lock(&server->lock);
    header = (struct packet_header *)server->send_buf;
    header->from = SOCKET_TYPE_CORE;
    header->type = type;
    header->size = size;
    header->clientId = clientId;
    header->count = count ++;
    header->condition = SOCKET_CONDITION_CODE;
    header->crc = 0;
    if (size > 0) {
        memcpy(server->send_buf + sizeof(struct packet_header), data, size);
    }
    if (client_state == CLI_STATE_CONNECT) {
        index = 0;
    }

    if (need_check_reqs_type(type)) {
        do {
            if ((index < 0) || (client_state == CLI_STATE_DISCONNECT)) {
                ret = -1;
                break;
            }
            int offset = 0;
            int write_len = 0;
            int times = 0;

            while (offset < len) {
                pthread_mutex_lock(&server->client[i].lock);
                if (server->client[i].state == CLI_STATE_DISCONNECT) {
                    ret = -1;
                    msdebug(DEBUG_INF, "Socket close. write failed(%d)", client_fd);
                    pthread_mutex_unlock(&server->client[i].lock);
                    break;
                }
                write_len = write(client_fd, server->send_buf + offset, len - offset);
                pthread_mutex_unlock(&server->client[i].lock);
                if (write_len < 0) {
                    if (errno == EAGAIN) {
                        msdebug(DEBUG_INF, "Socket busy. client_fd:%d", client_fd);
                        ret = SOCKET_MSG_CTRL_BUFFER_FULL;
                    } else {
                        msdebug(DEBUG_INF, "Socket fail. write failed(%d)", client_fd);
                        ret = -1;
                    }
                    break;
                } else {
                    offset += write_len;
                }
                if (offset < len) {
                    times++;
                    usleep(50 * 1000);
                    if (times >= MS_SOCK_SEND_LONG_TIMES_OUT) {
                        break;
                    }
                }
            }
        } while (0);
    } else {
        while (index >= 0) {
            SOCK_PRINTF("[MSSOCK]==test by mjx 0000000000000=%d=\n", client_state);
            if (client_state == CLI_STATE_DISCONNECT) {
                SOCK_PRINTF("[MSSOCK]==test by mjx 11111111111==\n");
                index++;
                usleep(50000);
                if (index == 40) { //time out 2 sec, then ignore it!  ------add by mjx.milesight 2014.10.16
                    SOCK_PRINTF("[MSSOCK]==%d===sendto:%d====len:%d==size:%d==0000 Fail==\n", type, sendto, len, size);
                    break;
                }
            } else if (write(client_fd, server->send_buf, len) == -1) {
                SOCK_PRINTF("[MSSOCK]==test by mjx 22222222222222222222==\n");
                if (need_check_reqs_type(type)) {
                    ret = -1;
                    break;
                }
                client_state = CLI_STATE_DISCONNECT;

                pthread_mutex_lock(&server->client[i].lock);
                server->client[i].state = CLI_STATE_DISCONNECT;
                client_state = CLI_STATE_DISCONNECT;
                //FD_CLR(client_fd, &server->readfds);//version:2.0.8 add
                ev.data.fd = client_fd;
                ev.events = EPOLLIN; //read
                epoll_ctl(server->epfd, EPOLL_CTL_DEL, client_fd, &ev);//FD_CLR(fd, &server->readfds);
                pthread_mutex_unlock(&server->client[i].lock);

                //del_client_node(server, client_fd);
                switch (errno) {
                    case EBADF:
                        perror("Bad file descriptor!");
                        break;
                    case ENOSPC:
                        perror("No space left on device!");
                        break;
                    case EINVAL:
                        perror("Invalid parameter: buffer was NULL!");
                        break;
                    default:
                        // An unrelated error occured
                        perror("Unexpected error!");
                        break;
                }
                index++;
                usleep(50000);
                if (index == 40) {
                    SOCK_PRINTF("[MSSOCK]==%d===sendto:%d====len:%d==size:%d==1111 Fail==%d==\n", type, sendto, len, size, errno);
                    break;
                }
            } else {
                SOCK_PRINTF("[MSSOCK]==test by mjx 333333333333333==\n");
                SOCK_PRINTF("[MSSOCK]==%d===sendto:%d====len:%d==size:%d==9999 OK==\n", type, sendto, len, size);
                break;
            }
        }
    }

    //pthread_mutex_unlock(&server->client[i].lock);
    pthread_mutex_unlock(&server->lock);
    SOCK_PRINTF("[MSSOCK]==test by mjx end==\n");
    return ret;
}
//end server

//client
struct sock_client_params {
    int sockfd;
    int epfd;
    struct sockaddr_un addr;
    pthread_mutex_t lock;
    pthread_t thread;
    MS_SOCK_CALLBACK_NEW_MSG callback;
    void *arg;
    int state;
    int from;
    int ipc_type;
    fd_set readfds;
    unsigned char *send_buf;
    unsigned char *recv_buf;
#ifdef MS_SOCK_PTHREAD_JOIN
    int thread_run;
#endif
};

static struct sock_client_params *global_client;

static int is_need_set_overtime(int sockType)
{
    return sockType == SOCKET_TYPE_URL;
}

static int ms_client_connect(struct sock_client_params *client)
{
    int res;
    struct epoll_event ev;
    pthread_mutex_lock(&client->lock);
    if (client->sockfd > 0) {
        close(client->sockfd);
    }
    client->sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    ms_sock_set_cloexec(client->sockfd);
    client->addr.sun_family = AF_UNIX;
    //strcpy(client->addr.sun_path, MS_SOCK_ADDR);//bruce.milesight delete
    SOCK_PRINTF("[MSSOCK %d]client path:%s\n", client->ipc_type,  client->addr.sun_path);
    pthread_mutex_unlock(&client->lock);

    res = connect(client->sockfd, (struct sockaddr *) & (client->addr), sizeof(client->addr));
    //SOCK_PRINTF("[MSSOCK]: ms_client_connect===connect\n");
    if (res) {
        static int timer = 0;
        if (timer % 5 == 0) {
            SOCK_PRINTF("[MSSOCK]can not connect server\n");
        }
        timer++;
        return -1;
    }
    client->epfd = epoll_create(FD_MAX);
    pthread_mutex_lock(&client->lock);
//  FD_SET(client->sockfd, &client->readfds);
    ev.data.fd = client->sockfd;
    ev.events = EPOLLIN; //read
    epoll_ctl(client->epfd, EPOLL_CTL_ADD, client->sockfd, &ev);
    client->state = CLI_STATE_ACCEPT;
    pthread_mutex_unlock(&client->lock);
    SOCK_PRINTF("[MSSOCK]: ms_client_connect===success\n");

    if (is_need_set_overtime(client->ipc_type)) {
        struct timeval val = {2, 0};
        setsockopt(client->sockfd, SOL_SOCKET, SO_RCVTIMEO, &val, sizeof(val));
    }
    return 0;
}

static int ms_client_disconnect(struct sock_client_params *client)
{
    struct epoll_event ev;

    if (client == NULL) {
        return -1;
    }

    pthread_mutex_lock(&client->lock);
//  FD_CLR(client->sockfd, &client->readfds);
    ev.data.fd = client->sockfd;
    ev.events = EPOLLIN; //read
    epoll_ctl(client->epfd, EPOLL_CTL_DEL, client->sockfd, &ev);//FD_CLR(fd, &server->readfds);

    if (client->state != CLI_STATE_DISCONNECT) {
        if (client->sockfd > 0) {
            close(client->sockfd);
        }
        if (client->epfd > 0) {
            close(client->epfd);
        }
        client->state = CLI_STATE_DISCONNECT;
    }
    pthread_mutex_unlock(&client->lock);

    return 0;
}

static void *sock_clinet_listen(void *arg)
{
#ifndef MS_SOCK_PTHREAD_JOIN
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
#endif

    struct sock_client_params *client;
    client = (struct sock_client_params *)arg;

    int res;
    int num = 0;
    struct epoll_event events[MAX_CLIENTS];
    char *buf = (char *)client->recv_buf;

    int overtime = 0;
    if (is_need_set_overtime(client->ipc_type)) {
        overtime = 1;
    }

#ifdef MS_SOCK_PTHREAD_JOIN
    while (client->thread_run)
#else
    while (1)
#endif
    {
        num = 0;
        if (client->state == CLI_STATE_DISCONNECT) {
            res = ms_client_connect(client);
            if (res != 0) {
                SOCK_PRINTF("[MSSOCK]: sock_clinet_listen===CLI_STATE_DISCONNECT\n");
                usleep(50000);
                continue;
            }
        }

        num = epoll_wait(client->epfd, events, FD_MAX, 1000);
#ifndef MS_SOCK_PTHREAD_JOIN
        pthread_testcancel();
#endif
        if (num < 1) {
            continue;
        }

        if (!fd_is_read(client->sockfd, events, num)) {
            SOCK_PRINTF("[MSSOCK]: sock_clinet_listen fd [%d] is no read \n", client->sockfd);
            continue;
        }

        //read header
        struct packet_header header;
        memset(&header, 0, sizeof(struct packet_header));
        if (overtime) {
            res = read_from_tcp_ex(client->sockfd, (char *)&header, sizeof(struct packet_header));
        } else {
            res = read_from_tcp(client->sockfd, (char *)&header, sizeof(struct packet_header));
        }
        if (res < 0 || header.size > SOCKET_DATA_MAX_SIZE || header.condition != SOCKET_CONDITION_CODE) {
            SOCK_PRINTF("[MSSOCK]: sock_clinet_listen===SOCKET_DATA_MAX_SIZE==res:%d==head.size:%d==condi:%d==\n", res, header.size,
                        header.condition);
            ms_client_disconnect(client);
            continue;
        }

        if (header.size > 0) {
            if (overtime) {
                res = read_from_tcp_ex(client->sockfd, buf, header.size);
            } else {
                res = read_from_tcp(client->sockfd, buf, header.size);
            }
            if (res < 0) {
                SOCK_PRINTF("[MSSOCK]: sock_clinet_listen===read_from_tcp\n");
                ms_client_disconnect(client);
                continue;
            }
        }

        if (client->state == CLI_STATE_ACCEPT) {
            if (header.type == SOCKET_MSG_CTRL_CONNECT) {
                SOCK_PRINTF("[MSSOCK]line :%d, CLI_STATE_ACCEPT type:SOCKET_MSG_CTRL_CONNECT\n", __LINE__);
                pthread_mutex_lock(&client->lock);
                client->state = CLI_STATE_CONNECT;
                //client->from = (*(int*)buf << 16) | client->ipc_type;
                //david.milesight
                if (client->ipc_type != SOCKET_TYPE_WEB) {
                    client->from = (*(int *)buf << 16) | client->ipc_type;
                } else {
                    client->from = getpid();
                }
                pthread_mutex_unlock(&client->lock);
                SOCK_PRINTF("[MSSOCK]: sock_clinet_listen===SOCKET_MSG_CTRL_CONNECT\n");
                ms_sock_send_msg(SOCKET_TYPE_CORE, SOCKET_MSG_CTRL_CONNECT, (void *)&client->from, sizeof(client->from), 0);
                continue;
            } else {
                SOCK_PRINTF("[MSSOCK]: sock_clinet_listen===ms_client_disconnect\n");
                ms_client_disconnect(client);
                continue;
            }
        }
        if (client->callback != NULL && client->state == CLI_STATE_CONNECT) {
            //if (client->ipc_type == SOCKET_TYPE_URL)
            //  printf("[MSSOCK]: sock_clinet_listen===callback==0000 start===\n");
            client->callback(client->arg, &header, buf);
            //if (client->ipc_type == SOCKET_TYPE_URL)
            //  printf("[MSSOCK]: sock_clinet_listen===callback==1111 end===\n");
        }
    }

    pthread_exit(NULL);
}

static int sock_init_client(MS_SOCK_CALLBACK_NEW_MSG callback, void *arg, int ipc_type, const char *sun_path)
{
    int res;
    pthread_attr_t attr;
    struct sock_client_params *client;
    client = (struct sock_client_params *)ms_malloc(sizeof(struct sock_client_params));
    global_client = client;
    client->ipc_type = ipc_type;
    client->callback = callback;
    strcpy(client->addr.sun_path, sun_path);
    client->arg = arg;
    pthread_mutex_init(&client->lock, NULL);
    client->send_buf = (unsigned char *)ms_malloc(sizeof(struct packet_header) + SOCKET_DATA_MAX_SIZE);
    client->recv_buf = (unsigned char *)ms_malloc(SOCKET_DATA_MAX_SIZE);
#ifdef MS_SOCK_PTHREAD_JOIN
    client->thread_run = 1;
    res = ms_task_create_join(&client->thread, sock_clinet_listen, (void *)client);
#else
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, THREAD_STACK_1M_SIZE);
    res = pthread_create(&client->thread, &attr, sock_clinet_listen, (void *)client);
#endif
    if (res) {
        SOCK_PRINTF("[MSSOCK] %d pthread_create failed.\n", __LINE__);
        return -1;
    }
    //sleep(1);

    return 0;
}

static int sock_uninit_client()
{
    struct sock_client_params *client;
    client = global_client;
    if (client != NULL) {
        ms_sock_send_msg(SOCKET_TYPE_CORE, SOCKET_MSG_CTRL_DISCONNECT, 0, 0, 0);//bruce.milesight version:2.0.7 2014.10.17
        pthread_mutex_lock(&client->lock);
#ifdef MS_SOCK_PTHREAD_JOIN
        client->thread_run = 0;
        ms_task_join(&client->thread);
#else
        pthread_cancel(client->thread);
#endif
        pthread_mutex_unlock(&client->lock);
        ms_client_disconnect(client);
        if (client->send_buf) {
            ms_free(client->send_buf);
        }
        if (client->recv_buf) {
            ms_free(client->recv_buf);
        }
        pthread_mutex_destroy(&client->lock);//bruce.milesight add
        ms_free(client);
        global_client = NULL;
    }
    return 0;
}

int ms_get_sock_independent(int type)
{
    int ret = IND_NO;

    switch (type) {
        case REQUEST_FLAG_GET_IPCPARAM:
        case REQUEST_FLAG_UPDATE_IPC:
        case REQUEST_FLAG_UPDATE_IPC_BATCH:
        case REQUEST_FLAG_REMOTE_PLAYBACK_PLAY:
        case REQUEST_FLAG_SEARCH_MSFS_NAS:
        case REQUEST_FLAG_GET_EVENT_STATUS:
        case REQUEST_FLAG_GET_IPC_STATUS:
        case REQUEST_FLAG_GET_MOTION_EVENT_STATUS:
        case REQUEST_FLAG_FAILOVER_GET_MASTER_STATUS:
        case REQUEST_FLAG_GET_PBPLAYTIME:
        case REQUEST_FLAG_GET_PLAYBACK_REALTIME:
        case REQUEST_FLAG_GET_SYSTEM_RESOURCE:
        case REQUEST_FLAG_LOG_WRITE:
        case REQUEST_FLAG_GET_REMOTEPB_TIME:
        case REQUEST_FLAG_SET_ALL_RECORD:
        case REQUEST_FLAG_GET_NETWORK_SPEED:
        case REQUEST_FLAG_GET_VAC_SUPPORT_ALL:
        case REQUEST_FLAG_GET_VAC_PERSONPOINT:
        case REQUEST_FLAG_EXPORT_ANPR_BACKUP:
        case REQUEST_FLAG_UPGRADE_IPC_IMAGE:
        case REQUEST_FLAG_GET_AUTO_TRACK_POINT:
        case REQUEST_FLAG_GET_SPLIT_PLAYBACK_TIME:
        case REQUEST_FLAG_REMOVE_IPC:
        case REQUEST_FLAG_SET_IPCPARAM:
        case REQUEST_FLAG_REMOTE_BACKUP_SPACE:
        case REQUEST_FLAG_FORMAT_ESATA_DISK:
        case REQUEST_FLAG_LOG_WEB_SEARCH:
        case REQUEST_FLAG_LOG_SEARCH:
        case REQUEST_FLAG_LOG_EXPORT:
        case REQUEST_FLAG_LOG_WEB_EXPORT:
        case REQUEST_FLAG_P2P_STATUS_INFO:
        case REQUEST_FLAG_SEARCH_POS_EXPORT:
        case REQUEST_FLAG_SEARCH_POS_OPEN:
        case REQUEST_FLAG_TEST_MAIL:
        case REQUEST_FLAG_SET_SYSTIME:
        case REQUEST_FLAG_EXPORT_DIAGNOSTIC_LOG:
        case REQUEST_FLAG_CREATE_RAID:
        case REQUEST_FLAG_SET_REC_RECYCLEMODE:
        case REQUEST_FLAG_FORMAT_MSFS_DISK:
        case REQUEST_FLAG_SEARCH_NVR:
        case REQUEST_FLAG_FAILOVER_SEARCH_NVR:
        case REQUEST_FLAG_SEARCH_IPC:
        case REQUEST_FLAG_CHECK_ONLINE_IPC_BATCH:
        case REQUEST_FLAG_GET_FISHEYE_MODE:
        case REQUEST_FLAG_SEARCH_COM_PLAYBACK_OPEN:
        case REQUEST_FLAG_SEARCH_EVT_PLAYBACK_OPEN:
        case REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN:
        case REQUEST_FLAG_SEARCH_COM_REMOTEPB_OPEN:
        case REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN_REMOTE:
        case REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_OPEN:
        case REQUEST_FLAG_SEARCH_EVT_BACKUP:
        case REQUEST_FLAG_SEARCH_COM_BACKUP:
        case REQUEST_FLAG_SEARCH_PIC_BACKUP:
        case REQUEST_FLAG_SEARCH_ANPR_BACKUP:
        case REQUEST_FLAG_SEARCH_FACE_BACKUP_REMOTE:
        case REQUEST_FLAG_SEARCH_FACE_BACKUP:
        case REQUEST_FLAG_SEARCH_ANPR_BACKUP_REMOTE:
        case REQUEST_FLAG_REMOTE_BACKUP_START:
        case REQUEST_FLAG_EXPORT_COMMON_PLAYBACK:
        case REQUEST_FLAG_EXPORT_COMMON_BACKUP:
        case REQUEST_FLAG_EXPORT_EVENT_BACKUP:
        case REQUEST_FLAG_EXPORT_PICTURE_BACKUP:
        case REQUEST_FLAG_EXPORT_FACE_BACKUP:
        case REQUEST_FLAG_REMOTE_UPGRADE_CAMERA_INIT:
        case REQUEST_FLAG_ONLINE_UPGRADE_CAMERA:
        case REQUEST_FLAG_PLAY_COM_PLAYBACK:
        case REQUEST_FLAG_STOP_COM_PLAYBACK:
        case REQUEST_FLAG_START_ALL_PLAYBACK:
        case REQUEST_FLAG_STOP_ALL_PLAYBACK:
        case REQUEST_FLAG_SEARCH_EVT_BASE_INFO:
        case REQUEST_FLAG_GET_MSFS_DISKINFO:
        case REQUEST_FLAG_UPDATE_MSFS_NAS:
        case REQUEST_FLAG_SET_IPC_FISHEYE_INFO:
        case REQUEST_FLAG_FORMAT_EXPORT_DISK:
        case REQUEST_FLAG_GET_IPC_DIAGNOSE:
        case REQUEST_FLAG_GET_IPC_LOG:
        case REQUEST_FLAG_GET_IPC_CFG:
        case REQUEST_FLAG_FAILOVER_REMOVE_CAMS:
            ret = IND_YES;
            break;
        default:
            break;
    }

    return ret;
}

int ms_get_sock_more_than_cnt(int type)
{
    if (type == REQUEST_FLAG_GET_EVENT_STATUS || type == REQUEST_FLAG_GET_IPC_STATUS
        || type == REQUEST_FLAG_GET_MOTION_EVENT_STATUS || type == REQUEST_FLAG_FAILOVER_GET_MASTER_STATUS
        || type == REQUEST_FLAG_GET_PBPLAYTIME || type == REQUEST_FLAG_GET_PLAYBACK_REALTIME
        || type == REQUEST_FLAG_GET_SYSTEM_RESOURCE || type == REQUEST_FLAG_GET_VAC_PERSONPOINT
        || type == REQUEST_FLAG_GET_REMOTEPB_TIME || type == REQUEST_FLAG_GET_NETWORK_SPEED) {
        return IND_YES;
    }

    return IND_NO;
}

static int client_send_msg(int sendto, int type, int size, void *data, UInt64 clientId)
{
    if (global_socket_state == 1) {
        return 0;
    }

    static int count;
    struct sock_client_params *client;
    struct packet_header *header;
    int len;

    client = global_client;
    if (type == SOCKET_MSG_CTRL_DISCONNECT) {
        return ms_client_disconnect(client);
    }

    if (client == NULL) {
        printf("[MSSOCK]: connect error sendto:%d type:%d size:%d\n", sendto, type, size);
        return -1;
    }
    if (size > SOCKET_DATA_MAX_SIZE) {
        printf("[MSSOCK]: Data size(%d) is too big\n", size);
        return -1;
    }

    pthread_mutex_lock(&client->lock);
    if (client->state != CLI_STATE_CONNECT) {
        pthread_mutex_unlock(&client->lock);
        SOCK_PRINTF("[MSSOCK] %d CLI_STATE_DISCONNECT.\n", __LINE__);
        return -1;
    }

    len = sizeof(struct packet_header) + size;
    header = (struct packet_header *)client->send_buf;
    header->from = client->from;
    header->type = type;
    header->size = size;
    header->count = count;
    header->clientId = clientId;
    header->independent = ms_get_sock_independent(type);
    header->condition = SOCKET_CONDITION_CODE;
    if (size) {
        memcpy(client->send_buf + sizeof(struct packet_header), data, size);
    }
    if (client->state == CLI_STATE_CONNECT) {
        write(client->sockfd, client->send_buf, len);
        count ++;
    }
    pthread_mutex_unlock(&client->lock);
    SOCK_PRINTF("[MSSOCK]: client_send_msg size %d client->sockfd %d\n", size, client->sockfd);

    return 0;
}
//end client

static int sock_client_send_wait(void *arg, int type, void *data, int size)
{
    if (global_socket_state == 1) {
        return 0;
    }

    struct sock_client_params *client;
    client = (struct sock_client_params *)arg;
    int res;
    char *buf = (char *)client->recv_buf;
    int nCount = 0;
    fd_set testfds;
    struct timeval timeout;
    int nHaveSendData = 0;
    FD_ZERO(&client->readfds);
    while (nCount < 5) {
        //printf("========sock_client_send_wait==select====111111==%d===\n", client->state);
        if (client->state == CLI_STATE_DISCONNECT) {
            res = ms_client_connect(client);
            if (res != 0) {
                SOCK_PRINTF("[MSSOCK]: sock_client_send_wait=%d=%d=CLI_STATE_DISCONNECT\n", client->sockfd, client->state);
                sleep(1);
                nCount++;
                if (client->sockfd > 0) {
                    close(client->sockfd);
                }
                client->sockfd = 0;
                if (nCount == 2) {
                    return -1;
                }
                continue;
            }
        } else if (client->state == CLI_STATE_CONNECT) {
            if (nHaveSendData == 0) {
                struct packet_header *header;
                int len;
                if (size > SOCKET_DATA_MAX_SIZE) {
                    printf("[MSSOCK]: Data size(%d) is too big\n", size);
                    return -1;
                }
                pthread_mutex_lock(&client->lock);
                if (client->state != CLI_STATE_CONNECT) {
                    pthread_mutex_unlock(&client->lock);
                    SOCK_PRINTF("[MSSOCK] %d CLI_STATE_CONNECT.\n", __LINE__);
                    return -1;
                }
                len = sizeof(struct packet_header) + size;
                header = (struct packet_header *)client->send_buf;
                header->from = client->from;
                header->type = type;
                header->size = size;
                header->count = 0;
                header->condition = SOCKET_CONDITION_CODE;
                if (size) {
                    memcpy(client->send_buf + sizeof(struct packet_header), data, size);
                }
                if (client->state == CLI_STATE_CONNECT) {
                    write(client->sockfd, client->send_buf, len);
                }
                pthread_mutex_unlock(&client->lock);
                //printf("====HaveSendData==========\n");
                nHaveSendData = 1;
            }
        }
        //printf("========sock_client_send_wait==select=====%d====\n", client->state);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        testfds = client->readfds;
        res = select(FD_MAX, &testfds, NULL, NULL, &timeout);
        if (res <= 0) {
            nCount++;
            continue;
        }

        //read header
        struct packet_header header;
        memset(&header, 0, sizeof(struct packet_header));
        res = read_from_tcp(client->sockfd, (char *)&header, sizeof(struct packet_header));
        if (res < 0 || header.size > SOCKET_DATA_MAX_SIZE || header.condition != SOCKET_CONDITION_CODE) {
            SOCK_PRINTF("[MSSOCK]: sock_client_send_wait===SOCKET_DATA_MAX_SIZE==res:%d==head.size:%d==condi:%d==\n", res,
                        header.size, header.condition);
            ms_client_disconnect(client);
            return -1;
        }

        if (header.size > 0) {
            res = read_from_tcp(client->sockfd, buf, header.size);
            if (res < 0) {
                SOCK_PRINTF("[MSSOCK]: sock_client_send_wait===read_from_tcp\n");
                ms_client_disconnect(client);
                return -1;
            }
            if (header.size > size) {
                SOCK_PRINTF("[MSSOCK]: sock_client_send_wait==%d===header.size > size====%d===\n", header.size, size);
                return -1;
            }
        }

        if (client->state == CLI_STATE_ACCEPT) {
            if (header.type == SOCKET_MSG_CTRL_CONNECT) {
                SOCK_PRINTF("[MSSOCK]line :%d, CLI_STATE_ACCEPT type:SOCKET_MSG_CTRL_CONNECT\n", __LINE__);
                pthread_mutex_lock(&client->lock);
                client->state = CLI_STATE_CONNECT;
                client->from = (*(int *)buf << 16) | client->ipc_type;
                pthread_mutex_unlock(&client->lock);
                SOCK_PRINTF("[MSSOCK]: sock_client_send_wait===SOCKET_MSG_CTRL_CONNECT\n");
                struct packet_header *header;
                int len;
                pthread_mutex_lock(&client->lock);
                len = sizeof(struct packet_header);
                header = (struct packet_header *)client->send_buf;
                header->from = client->from;
                header->type = SOCKET_MSG_CTRL_CONNECT;
                header->size = 0;
                header->count = 0;
                header->condition = SOCKET_CONDITION_CODE;
                if (header->size) {
                    memcpy(client->send_buf + sizeof(struct packet_header), data, header->size);
                }
                if (client->state == CLI_STATE_CONNECT) {
                    write(client->sockfd, client->send_buf, len);
                }
                pthread_mutex_unlock(&client->lock);
                //ms_sock_send_msg(SOCKET_TYPE_CORE, SOCKET_MSG_CTRL_CONNECT, (void*)&client->from, sizeof(client->from), 0);
                continue;
            } else {
                SOCK_PRINTF("[MSSOCK]: sock_client_send_wait===ms_client_disconnect\n");
                ms_client_disconnect(client);
                return -1;
            }
        }
        memcpy(data, buf, header.size);
        break;
    }
    if (nCount == 5) {
        return -1;
    }
    return 0;
}

int ms_sock_send_msg_wait(int type, void *data, int size, enum ms_socket_type sock_type, const char *sun_path)
{
    if (global_socket_state == 1) {
        return 0;
    }

    int res;
    struct sock_client_params *client;
    client = (struct sock_client_params *)ms_malloc(sizeof(struct sock_client_params));
    client->ipc_type = sock_type;
    client->callback = NULL;
    strcpy(client->addr.sun_path, sun_path);
    client->arg = NULL;
    pthread_mutex_init(&client->lock, NULL);
    client->send_buf = (unsigned char *)ms_malloc(sizeof(struct packet_header) + SOCKET_DATA_MAX_SIZE);
    client->recv_buf = (unsigned char *)ms_malloc(SOCKET_DATA_MAX_SIZE);
    res = sock_client_send_wait((void *)client, type, data, size);
    ms_client_disconnect(client);
    if (client->send_buf) {
        ms_free(client->send_buf);
    }
    if (client->recv_buf) {
        ms_free(client->recv_buf);
    }
    pthread_mutex_destroy(&client->lock);//bruce.milesight add
    ms_free(client);
    return res;
}


//interface
int ms_sock_send_msg(int sendto, int type, void *data, int size, UInt64 clientId)
{
    if (global_socket_state == 1) {
        return 0;
    }

    int res;

    if (sendto == SOCKET_TYPE_CORE) {
        res = client_send_msg(sendto, type, size, data, clientId);
    } else {
        res = server_send_msg(sendto, type, size, data, clientId);
    }

    return res;
}

int ms_sock_set_cloexec(int sockfd)
{
    if (sockfd < 0) {
        return -1;
    }
    int tFlags = fcntl(sockfd, F_GETFD);
    fcntl(sockfd, F_SETFD, tFlags | FD_CLOEXEC);
    return 0;
}
int ms_set_socket_state(int state)
{
    global_socket_state = state;
    return 0;
}

int ms_client_socket_state()
{
    struct sock_client_params *client;
    client = global_client;
    if (client == NULL) {
        printf("[MSSOCK]: ms_client_socket_state fail\n");
        return -1;
    }
    return client->state;
}

int ms_sock_init(MS_SOCK_CALLBACK_NEW_MSG callback, void *arg, enum ms_socket_type sock_type, const char *sun_path)
{
    int res;

    if (sock_type == SOCKET_TYPE_CORE) {
        res = sock_init_server(callback, arg, sock_type, sun_path);
    } else {
        res = sock_init_client(callback, arg, sock_type, sun_path);
    }

    return res;
}

int ms_sock_uninit()
{
    sock_uninit_server();
    usleep(100 * 1000);
    sock_uninit_client();
    usleep(100 * 1000);
    return 0;
}
//end interface

