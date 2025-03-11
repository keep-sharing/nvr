#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <stdarg.h>
#include "tsar_socket.h"


typedef enum {
    SOCK_STATE_IDEL     = 0,
    SOCK_STATE_ACCEPT   = 1,
    SOCK_STATE_CONNECT  = 2,
} SOCK_STATE_E;

struct SockNode {
    int fd;
    SOCK_STATE_E state;
    int from;
    MUTEX_OBJECT lock;
};

struct CommSocketServer {
    int sockFd;
    int run;
    TASK_HANDLE taskHandle;
    int epollFd;
    MUTEX_OBJECT epollMutex;
    struct SockNode client[MS_COMMON_SOCKET_MAX];
    void *arg;
    COMM_CALLBACK_RECV callback;
    TSAR_SOCK_TYPE type;
    TSAR_DEBUG_LEVEL debugLevel;
};

struct CommSocketClient {
    int run;
    TASK_HANDLE taskHandle;
    int epollFd;
    MUTEX_OBJECT epollMutex;
    char sunPath[256];
    struct SockNode client;
    void *arg;
    COMM_CALLBACK_RECV callback;
    TSAR_SOCK_TYPE type;
    TSAR_DEBUG_LEVEL debugLevel;
};

struct TsarSocketHandle {
    TSAR_SOCK_TYPE type;
    void *handle;
};

typedef struct CommListHandle {
    MUTEX_OBJECT lock;
    int cnt;
    int size;
    int limit;
    struct list_head list;
} COMM_LIST_HANDLE;


struct TsarDebugConf {
    TSAR_DEBUG_LEVEL debugLevel;
    TSAR_SOCK_TYPE type;
    void *tsarSocket;
    int run;
    TASK_HANDLE taskHandle;
    void *queue;
};

static struct TsarDebugConf *g_tsarSocket = NULL;

int tsar_sock_set_cloexec(int sockfd)
{
    int tFlags;
    
    if (sockfd < 0) {
        return -1;
    }
    tFlags = fcntl(sockfd, F_GETFD);
    fcntl(sockfd, F_SETFD, tFlags|FD_CLOEXEC);
    return 0;
}


static int add_common_epoll_in(int epollFd, int eventFd, MUTEX_OBJECT *mutex)
{
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (mutex) {
        pthread_mutex_lock(mutex);
    }
    ev.data.fd = eventFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, eventFd, &ev);
    if (mutex) {
        pthread_mutex_unlock(mutex);
    }

    return 0;
}

static int del_common_epoll_in(int epollFd, int eventFd, MUTEX_OBJECT *mutex)
{
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (mutex) {
        pthread_mutex_lock(mutex);
    }
    ev.data.fd = eventFd;
    epoll_ctl(epollFd, EPOLL_CTL_DEL, eventFd, &ev);
    if (mutex) {
        pthread_mutex_unlock(mutex);
    }

    return 0;
}

static void common_server_get_new_connect(struct CommSocketServer *server)
{
    int fd;
    int i;
    int index = -1;
    struct sockaddr_in cliaddr = {0};
    socklen_t cliaddrLen = sizeof(cliaddr);
    TSAR_SOCKET_HEADER header;
    
    fd = accept(server->sockFd, (struct sockaddr *)&cliaddr, &cliaddrLen);
    if (fd == -1) {
        msdebug(DEBUG_INF, "common socket accept failed!");
        return;
    }
    for (i = 0; i < MS_COMMON_SOCKET_MAX; i++) {
        ms_mutex_lock(&server->client[i].lock);
        if (server->client[i].state != SOCK_STATE_IDEL) {
            ms_mutex_unlock(&server->client[i].lock);
            continue;
        }
        server->client[i].state = SOCK_STATE_ACCEPT;
        server->client[i].fd = fd;
        tsar_sock_set_cloexec(fd);
        ms_mutex_unlock(&server->client[i].lock);
        add_common_epoll_in(server->epollFd, fd, &server->epollMutex);
        memset(&header, 0, sizeof(TSAR_SOCKET_HEADER));
        header.type = TSAR_CTRL_CONNECT;
        header.condition = TSAR_CONDITION_CODE;
        header.level = server->debugLevel;
        write(fd, &header, sizeof(TSAR_SOCKET_HEADER));
        index = i;
        break;
    }
    
    if (index == -1) {
        msdebug(DEBUG_INF, "tsar server has not free connect.");
        close(fd);
    }

    return;
}

static int tsar_socket_read(int fd, char * buf, int size)
{
    int offset = 0;
    int bytes;
    int wanted = size;

    while (wanted > 0) {
        bytes = read(fd, buf+offset, wanted);
        if (bytes <= 0) {
            return -1;
        }
        wanted -= bytes;
        offset += bytes;
    }
    return 0;
}

static void common_server_recv_from_client(struct CommSocketServer *server, int fd)
{
    int i;
    int ret;
    int err = 0;
    TSAR_SOCKET_HEADER header;
    char buff[TSAR_DEBUG_MAX_LEN] = {0};
    
    for (i = 0; i < MS_COMMON_SOCKET_MAX; i++) {
        //used lock?
        if (server->client[i].state == SOCK_STATE_IDEL) {
            continue;
        }
        if (server->client[i].fd != fd) {
            continue;
        }

        ret = tsar_socket_read(fd, (char *)&header, sizeof(TSAR_SOCKET_HEADER));
        if (ret != 0 || header.size > TSAR_DEBUG_MAX_LEN) {
            err = 1;
            break;
        }

        ret = tsar_socket_read(fd, buff, header.size);
        if (ret == 0) {
            if (server->client[i].state == SOCK_STATE_ACCEPT) {
                if (header.type == TSAR_CTRL_CONNECT) {
                    server->client[i].state = SOCK_STATE_CONNECT;
                    server->client[i].from = header.from;
                } else {
                    err = 1;
                }
            } else {
                if (server->callback) {
                    server->callback(server->arg, &header, buff);
                }
            }
        } else {
            err = 1;
        }
        break;
    }

    if (err) {
        del_common_epoll_in(server->epollFd, server->client[i].fd, &server->epollMutex);
        server->client[i].state = SOCK_STATE_IDEL;
        close(server->client[i].fd);
    }

    return;
}

static void *thread_common_socket_server(void *param)
{
    int i;
    int fdNum = -1;
    int maxevents = MS_COMMON_SOCKET_MAX;
    struct epoll_event *event;
    struct CommSocketServer *server = (struct CommSocketServer *)param;

    //init
    ms_task_set_name("tsarServer");
    server->epollFd = epoll_create(128);
    if (server->epollFd == -1) {
        msdebug(DEBUG_INF, "common socket creat epoll failed");
        server->run = 0;
        ms_task_quit(NULL);
        return NULL;
    }
    event = (struct epoll_event *)ms_malloc(maxevents * sizeof(struct epoll_event));
    if (!event) {
        msdebug(DEBUG_INF, "common socket malloc epoll_event failed!");
        server->run = 0;
        close(server->epollFd);
        ms_task_quit(NULL);
        return NULL;
    }
    ms_mutex_init(&server->epollMutex);
    for (i = 0; i < MS_COMMON_SOCKET_MAX; i++) {
        ms_mutex_init(&server->client[i].lock);
    }
    add_common_epoll_in(server->epollFd, server->sockFd, &server->epollMutex);
    while (server->run) {
        fdNum = epoll_wait(server->epollFd, event, maxevents, 1000);
        if (fdNum < 1) {
            continue;
        }

        for(i = 0; i < fdNum; i++) {
            if (!(event[i].events & EPOLLIN)) {
                continue;
            }

            if(event[i].data.fd == server->sockFd) {
                common_server_get_new_connect(server);
            } else {
                common_server_recv_from_client(server, event[i].data.fd);
            }
        }
    }

    //deinit
    del_common_epoll_in(server->epollFd, server->sockFd, &server->epollMutex);
    for (i = 0; i < MS_COMMON_SOCKET_MAX; i++) {
        ms_mutex_lock(&server->client[i].lock);
        if (server->client[i].state == SOCK_STATE_IDEL) {
            ms_mutex_unlock(&server->client[i].lock);
            ms_mutex_uninit(&server->client[i].lock);
            continue;
        }

        del_common_epoll_in(server->epollFd, server->client[i].fd, &server->epollMutex);
        server->client[i].state = SOCK_STATE_IDEL;
        close(server->client[i].fd);
        server->client[i].fd = -1;
        ms_mutex_unlock(&server->client[i].lock);
        ms_mutex_uninit(&server->client[i].lock);
    }

    close(server->epollFd);
    ms_free(event);
    ms_mutex_uninit(&server->epollMutex);
    ms_task_quit(NULL);
    return NULL;
}

static void *creat_common_socket_server(TSAR_SOCK_TYPE type, TSAR_DEBUG_LEVEL level, const char *sunPath, COMM_CALLBACK_RECV callback, void *arg)
{
    int reuse;
    int ret;
    struct sockaddr_un addrUn;
    struct CommSocketServer *server = NULL;
    server = ms_malloc(sizeof(struct CommSocketServer));
    if (!server) {
        return NULL;
    }

    server->type = type;
    server->arg = arg;
    server->callback = callback;
    server->debugLevel = level;
    server->sockFd = socket(AF_UNIX, SOCK_STREAM, 0);
    tsar_sock_set_cloexec(server->sockFd);
    addrUn.sun_family = AF_UNIX;
    strcpy(addrUn.sun_path, sunPath);
    unlink(sunPath);
    reuse = 1;
    ret = setsockopt(server->sockFd, SOL_SOCKET,SO_REUSEADDR, &reuse, sizeof(int));
    if (ret < 0) {
        msdebug(DEBUG_INF, "common socket set SO_REUSEADDR failed. sunpath:%s ret:%d errno:%d", sunPath, ret, errno);
    }
    ret = bind(server->sockFd, (struct sockaddr *)&addrUn, sizeof(addrUn));
    if (ret) {
        msdebug(DEBUG_INF, "common socket bind failed. sunpath:%s ret:%d errno:%d", sunPath, ret, errno);
        ms_free(server);
        return NULL;
    }

    listen(server->sockFd, 50);
    server->run = 1;
    if (ms_task_create_join_stack_size(&server->taskHandle, THREAD_STACK_MIN_SIZE, thread_common_socket_server, server) != 0) {
        server->run = 0;
        close(server->sockFd);
        ms_free(server);
        return NULL;
    }
    
    return (void *)server;
}

static void destroy_common_socket_server(void *handle)
{
    struct CommSocketServer *server;
    
    if (!handle) {
        return;
    }

    server = (struct CommSocketServer *)handle;
    if (server->run) {
        server->run = 0;
        ms_task_join(&server->taskHandle);
    }

    if (server->sockFd != -1) {
        close(server->sockFd);
    }

    ms_free(server);
}

static int common_socket_server_send_msg(void *handle, TSAR_SOCK_TYPE type, void *data, int len)
{
    int i;
    int n = -1;
    TSAR_SOCKET_HEADER *header;
    struct CommSocketServer *server;
    char buff[TSAR_DEBUG_MAX_LEN] = {0};
    
    if (!handle) {
        return -1;
    }

    server = (struct CommSocketServer *)handle;
    if (len > 500) {
        return -2;
    }

    header = (TSAR_SOCKET_HEADER *)buff;
    header->from = server->type;
    header->type = 2021;
    header->size = len;
    header->condition = TSAR_CONDITION_CODE;
    memcpy(buff+sizeof(TSAR_SOCKET_HEADER), data, len);
    for (i = 0; i < MS_COMMON_SOCKET_MAX; i++) {
        ms_mutex_lock(&server->client[i].lock);
        if (server->client[i].from != type) {
            continue;
        }
        if (server->client[i].state != SOCK_STATE_CONNECT) {
            ms_mutex_unlock(&server->client[i].lock);
            continue;
        }
        ms_mutex_unlock(&server->client[i].lock);
        n = write(server->client[i].fd, buff, len+sizeof(TSAR_SOCKET_HEADER));
        break;
    }

    if (n < 0) {
        return -1;
    }
    return 0;
}

static int common_socket_client_connect(struct CommSocketClient *client)
{
    int ret;
    struct sockaddr_un addrUn;
    
    client->client.fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client->client.fd < 0) {
        return -1;
    }
    tsar_sock_set_cloexec(client->client.fd);
    addrUn.sun_family = AF_UNIX;
    strcpy(addrUn.sun_path, client->sunPath);
    ret = connect(client->client.fd, (struct sockaddr *)&(addrUn), sizeof(addrUn));
    if (ret) {
        close(client->client.fd);
        client->client.fd = -1;
        return -1;
    }
    ms_mutex_lock(&client->client.lock);
    client->client.state = SOCK_STATE_ACCEPT;
    ms_mutex_unlock(&client->client.lock);
    add_common_epoll_in(client->epollFd, client->client.fd, &client->epollMutex);
    return 0;
}

static void common_socket_client_disconnect(struct CommSocketClient *client)
{
    ms_mutex_lock(&client->client.lock);
    client->client.state = SOCK_STATE_IDEL;
    ms_mutex_unlock(&client->client.lock);
    if (client->client.fd > 0) {
        del_common_epoll_in(client->epollFd, client->client.fd, &client->epollMutex);
        close(client->client.fd);
    }
}

static int common_client_recv_from_server(struct CommSocketClient *client, int fd)
{
    int n;
    TSAR_SOCKET_HEADER *header;
    char buff[TSAR_DEBUG_MAX_LEN] = {0};

    n = read(fd, buff, sizeof(buff));
    if (n <= 0) {
        return -1;
    }
    
    header = (TSAR_SOCKET_HEADER *)buff;
    ms_mutex_lock(&client->client.lock);
    if (client->client.state == SOCK_STATE_CONNECT) {
        if (client->callback) {
            client->callback(client->arg, header, buff+sizeof(TSAR_SOCKET_HEADER));
        }
        ms_mutex_unlock(&client->client.lock);
        return 0;
    }

    if (client->client.state == SOCK_STATE_ACCEPT && header->type != TSAR_CTRL_CONNECT) {
        ms_mutex_unlock(&client->client.lock);
        return -1;
    }

    if (client->callback) {
        client->callback(client->arg, header, buff+sizeof(TSAR_SOCKET_HEADER));
    }
    client->client.state = SOCK_STATE_CONNECT;
    ms_mutex_unlock(&client->client.lock);
    memset(header, 0, sizeof(TSAR_SOCKET_HEADER));
    header->type = TSAR_CTRL_CONNECT;
    header->condition = TSAR_CONDITION_CODE;
    header->from = client->type;
    n = write(fd, header, sizeof(TSAR_SOCKET_HEADER));
    if (n <= 0) {
        return -1;
    }

    return 0;
}

static void *thread_common_socket_client(void *param)
{
    int i;
    int fdNum = -1;
    int maxevents = MS_COMMON_SOCKET_MAX;
    int connState = -1;
    struct epoll_event *event;
    struct CommSocketClient *client = (struct CommSocketClient *)param;

    //init
    ms_task_set_name("tsarClient");
    client->epollFd = epoll_create(maxevents);
    if (client->epollFd == -1) {
        msdebug(DEBUG_INF, "common socket creat epoll failed");
        client->run = 0;
        return NULL;
    }
    event = (struct epoll_event *)ms_malloc(maxevents * sizeof(struct epoll_event));
    if (!event) {
        msdebug(DEBUG_INF, "common socket malloc epoll_event failed!");
        client->run = 0;
        close(client->epollFd);
        ms_task_quit(NULL);
        return 0;
    }
    ms_mutex_init(&client->epollMutex);
    ms_mutex_init(&client->client.lock);
    while (client->run) {
        if (connState == -1) {
            connState = common_socket_client_connect(client);
        }
        if (connState == -1) {
            usleep(1000*1000);
            continue;
        }
        fdNum = epoll_wait(client->epollFd, event, maxevents, 1000);
        if (fdNum < 1) {
            continue;
        }

        for (i = 0; i < fdNum; i++) {
            if (!(event[i].events & EPOLLIN)) {
                continue;
            }

            if (event[i].data.fd != client->client.fd) {
                continue;
            }

            if (common_client_recv_from_server(client, event[i].data.fd) != 0) {
                connState = -1;
                common_socket_client_disconnect(client);
                break;
            }
        }
    }

    if (connState != -1) {
        common_socket_client_disconnect(client);
    }
    close(client->epollFd);
    ms_free(event);
    ms_mutex_uninit(&client->epollMutex);
    ms_mutex_uninit(&client->client.lock);
    ms_task_quit(NULL);
    return NULL;
}

static void *creat_common_socket_client(TSAR_SOCK_TYPE type, TSAR_DEBUG_LEVEL level, const char *sunPath, COMM_CALLBACK_RECV callback, void *arg)
{
    struct CommSocketClient *client = NULL;

    client = ms_malloc(sizeof(struct CommSocketClient));
    if (!client) {
        return NULL;
    }

    client->type = type;
    client->callback = callback;
    client->arg = arg;
    client->debugLevel = level;
    client->run = 1;
    snprintf(client->sunPath, sizeof(client->sunPath), "%s", sunPath);
    if (ms_task_create_join_stack_size(&client->taskHandle, THREAD_STACK_MIN_SIZE, thread_common_socket_client, client) != 0) {
        client->run = 0;
        ms_free(client);
        client = NULL;
    }
    
    return client;
}

static void destroy_common_socket_client(void *handle)
{
    struct CommSocketClient *client;
    
    if (!handle) {
        return;
    }

    client = (struct CommSocketClient *)handle;
    if (client->run) {
        client->run = 0;
        ms_task_join(&client->taskHandle);
    }

    ms_free(client);
}

/*
    data:   struct TSAR_SOCKET_HEADER + data
    len:    sizeof(struct CommSocketClient) + dataLen
*/
static int common_socket_client_send_msg(void *handle, void *data, int len)
{
    struct CommSocketClient *client;
    
    client = (struct CommSocketClient *)handle;
    if (!client) {
        return -1;
    }
    ms_mutex_lock(&client->client.lock);
    if (client->client.state != SOCK_STATE_CONNECT) {
        ms_mutex_unlock(&client->client.lock);
        return -2;
    }
    ms_mutex_unlock(&client->client.lock);
    write(client->client.fd, data, len);
    return 0;
}

void *creat_tsar_socket(TSAR_SOCK_TYPE type, TSAR_DEBUG_LEVEL level, COMM_CALLBACK_RECV callback, void *arg)
{
    struct TsarSocketHandle *commSock = NULL;
    void *handle = NULL;

    if (type <= TSAR_SOCK_IDEL || type >= TSAR_SOCK_MAX) {
        return NULL;
    }
    
    commSock = ms_malloc(sizeof(struct TsarSocketHandle));
    if (!commSock) {
        return NULL;
    }
    if (type == TSAR_SOCK_MSOP) {
        handle = creat_common_socket_server(type, level, SOCK_TSAR_PATH, callback, arg);
    } else {
        handle = creat_common_socket_client(type, level, SOCK_TSAR_PATH, callback, arg);
    }

    if (!handle) {
        ms_free(commSock);
        commSock = NULL;
    } else {
       commSock->handle = handle;
       commSock->type = type;
    }

    return commSock;
}

void destroy_tsar_socket(void *handle)
{
    struct TsarSocketHandle *commSock;

    commSock = (struct TsarSocketHandle *)handle;
    if (!commSock) {
        return;
    }
    if (commSock->type == TSAR_SOCK_MSOP) {
        if (commSock->handle) {
            destroy_common_socket_server(commSock->handle);
        }
    } else {
        if (commSock->handle) {
            destroy_common_socket_client(commSock->handle);
        }
    }

    ms_free(commSock);
    return;
}

int tsar_socket_server_send_msg(void *handle, TSAR_SOCK_TYPE sendto, void *data, int len)
{
    struct TsarSocketHandle *commSock;

    commSock = (struct TsarSocketHandle *)handle;
    if (!commSock || !commSock->handle || commSock->type != TSAR_SOCK_MSOP) {
        return -1;
    }

    return common_socket_server_send_msg(commSock->handle, sendto, data, len);
}

int tsar_socket_client_send_msg(void *handle, void *data, int len)
{
    struct TsarSocketHandle *commSock;

    commSock = (struct TsarSocketHandle *)handle;
    if (!commSock || !commSock->handle || commSock->type == TSAR_SOCK_MSOP) {
        return -1;
    }

    return common_socket_client_send_msg(commSock->handle, data, len);
}

static void tsar_debuger_node_free(COMM_LIST_NODE *node)
{
    if (node) {
        if (node->data) {
            ms_free(node->data);
        }

        ms_free(node);
    }
}

static void *thread_tsar_debuger(void *param)
{
    int ret;
    struct TsarDebugConf *arg = (struct TsarDebugConf *)param;
    COMM_LIST_NODE *node;

    ms_task_set_name("tsarDebuger");
    while (arg->run) {
        node = popfront_common_list_node(arg->queue);
        if (!node) {
            usleep(40*1000);
            continue;
        }

        ret = tsar_socket_client_send_msg(arg->tsarSocket, node->data, node->size);
        if (ret != 0) {
            ;//msdebug(DEBUG_INF, "send msg failed. ret:%d", ret);
        }
        tsar_debuger_node_free(node);
    }

    //clear list
    do {
        node = popfront_common_list_node(arg->queue);
        if (!node) {
            break;
        }
        tsar_debuger_node_free(node);
    } while(1);

    ms_task_quit(NULL);
    return NULL;
}

static void get_tsar_type_text(TSAR_SOCK_TYPE type, char *buf, int len)
{
    switch(type) {
        case TSAR_SOCK_MSOP:
            snprintf(buf, len, "%s(%d)", "MSOP", type);
            break;
        
        case TSAR_SOCK_MSCORE:
            snprintf(buf, len, "%s(%d)", "MSCORE", type);
            break;
        
        case TSAR_SOCK_BOA:
            snprintf(buf, len, "%s(%d)", "BOA", type);
            break;

        default:
            snprintf(buf, len, "%s(%d)", "UNKNOW", type);
            break;
            break;
    }
}

static void get_tsar_level_text(TSAR_DEBUG_LEVEL level, char *buf, int len)
{
    switch(level) {
        case TSAR_INFO:
            snprintf(buf, len, "%s(%d)", "INFO", level);
            break;
        
        case TSAR_DEBUG:
            snprintf(buf, len, "%s(%d)", "DEBUG", level);
            break;

        case TSAR_WARN:
            snprintf(buf, len, "%s(%d)", "WARN", level);
            break;

        case TSAR_ERR:
            snprintf(buf, len, "%s(%d)", "ERROR", level);
            break;

        case TSAR_FATAL:
            snprintf(buf, len, "%s(%d)", "FATAL", level);
            break;

        default:
            snprintf(buf, len, "%s(%d)", "UNKNOW", level);
            break;
    }
}


static void tsar_debug_recv_callback(void *arg, TSAR_SOCKET_HEADER *header, void *data)
{
    struct TsarDebugConf *debuger = (struct TsarDebugConf *)arg;
    char type[32] = {0};
    char level[32] = {0};

    if (!arg || !header) {
        return;
    }

    if (header->type == TSAR_CTRL_CONNECT) {
        debuger->debugLevel = header->level;
        get_tsar_type_text(debuger->type, type, sizeof(type));
        get_tsar_level_text(header->level, level, sizeof(level));
        ms_log(TSAR_INFO, "tsar %s set debug level %s", type, level);
    }
}

int init_tsar_debuger(TSAR_DEBUG_LEVEL level, TSAR_SOCK_TYPE type)
{
    if (g_tsarSocket != NULL) {
        return -1;
    }
    
    struct TsarDebugConf *debuger = NULL;

    debuger = ms_malloc(sizeof(struct TsarDebugConf));
    if (!debuger) {
        return -1;
    }

    debuger->debugLevel = level;
    //socket
    debuger->type = type;
    debuger->tsarSocket = creat_tsar_socket(type, level, tsar_debug_recv_callback, debuger);
    if (!debuger->tsarSocket) {
        ms_free(debuger);
        return -1;
    }
    //list
    debuger->queue = creat_common_list(MS_COMMON_LIST_MAX);
    if (!debuger->queue) {
        destroy_tsar_socket(debuger->tsarSocket);
        ms_free(debuger);
        return -1;
    }
    //task
    debuger->run = 1;
    if (ms_task_create_join_stack_size(&debuger->taskHandle, THREAD_STACK_MIN_SIZE, thread_tsar_debuger, debuger) != 0) {
        destroy_tsar_socket(debuger->tsarSocket);
        destroy_common_list(debuger->queue);
        ms_free(debuger);
        return -1;
    }

    g_tsarSocket = debuger;
    return 0;
}

void uninit_tsar_debuger()
{
    struct TsarDebugConf *debuger = (struct TsarDebugConf *)g_tsarSocket;

    if (!debuger) {
        return;
    }

    if (debuger->tsarSocket) {
        destroy_tsar_socket(debuger->tsarSocket);
    }
    if (debuger->run) {
        debuger->run = 0;
        ms_task_join(&debuger->taskHandle);
    }
    if (debuger->queue) {
        destroy_common_list(debuger->queue);
    }
    ms_free(g_tsarSocket);
    g_tsarSocket = NULL;
    return;
}

int tsar_debuger_support(TSAR_DEBUG_LEVEL level)
{
    struct TsarDebugConf *debuger = (struct TsarDebugConf *)g_tsarSocket;
    
    if (debuger && debuger->tsarSocket && level >= debuger->debugLevel) {
        return 1;
    }
    //TODO filter module

    return 0;
}

void tsar_log(int level, const char *file, const char *func, int line, const char* format, ...)
{
    char buf[TSAR_DEBUG_MAX_LEN] = {0};
    va_list ap;

    if (!tsar_debuger_support(level)) {
        return;
    }

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    tsar_debuger_output(level, file, func, line, buf);
}

int tsar_debuger_output(TSAR_DEBUG_LEVEL level, const char *file, const char *func, int line, char *msg)
{
    struct TsarDebugConf *debuger = (struct TsarDebugConf *)g_tsarSocket;
    COMM_LIST_NODE *node = NULL;
    TSAR_SOCKET_HEADER *header;
    int len;
    
    if (!tsar_debuger_support(level)) {
        return -1;
    }
    
    len = strlen(msg);
    node = ms_malloc(sizeof(COMM_LIST_NODE));
    if (!node) {
        return -1;
    }
    node->size = sizeof(TSAR_SOCKET_HEADER) + len + 1;//'\0'
    node->data = ms_malloc(node->size);
    if (!node->data) {
        ms_free(node);
        return -1;
    }
    header = (TSAR_SOCKET_HEADER *)node->data;
    header->condition = TSAR_CONDITION_CODE;
    header->type = TSAR_CTRL_DEBUG_MSG;
    header->from = debuger->type;
    header->level = level;
    header->timestamp = time(NULL);
    snprintf(header->fileName, sizeof(header->fileName), "%s", file);
    snprintf(header->funcName, sizeof(header->funcName), "%s", func);
    header->fileLine = line;
    header->size = len + 1;
    memcpy(node->data+sizeof(TSAR_SOCKET_HEADER), msg, len);
    if (insert_common_list_node(debuger->queue, node) != 0) {
        tsar_debuger_node_free(node);
        return -1;
    }
/*
    int retry = 0;
    if (level == TSAR_FATAL) {
        //make sure msg send to msop
        do {
            if (get_common_list_node_count(debuger->queue) == 0) {
                break;
            }
            usleep(40*1000);
            retry++;
            if (retry >= 10) {
                break;
            }
        } while(1);
    }
*/
    return 0;
}

void *creat_common_list(int maxSize)
{
    COMM_LIST_HANDLE *handle = NULL;

    handle = ms_malloc(sizeof(COMM_LIST_HANDLE));
    if (!handle) {
        return NULL;
    }
    ms_mutex_init(&handle->lock);
    handle->cnt = 0;
    handle->size = 0;
    handle->limit = maxSize;
    INIT_LIST_HEAD(&handle->list);

    return (void *)handle;
}

void destroy_common_list(void *list)
{
    COMM_LIST_HANDLE *handle = (COMM_LIST_HANDLE *)list;

    if (handle) {
        ms_mutex_uninit(&handle->lock);
        ms_free(handle);
    }
    
    return;
}

int insert_common_list_node(void *handle, COMM_LIST_NODE *node)
{
    COMM_LIST_HANDLE *queue = (COMM_LIST_HANDLE *)handle;

    if (!queue) {
        return -1;
    }

    ms_mutex_lock(&queue->lock);
    if (queue->limit && queue->size >= queue->limit) {
        ms_mutex_unlock(&queue->lock);
        return -2;
    }
    list_add_tail(&node->node, &queue->list);
    queue->cnt++;
    queue->size += node->size;
    ms_mutex_unlock(&queue->lock);
    return 0;
}

COMM_LIST_NODE *popfront_common_list_node(void *handle)
{
    COMM_LIST_NODE *node = NULL;
    COMM_LIST_HANDLE *queue = (COMM_LIST_HANDLE *)handle;

    if (!queue) {
        return NULL;
    }

    ms_mutex_lock(&queue->lock);
    do {
        if (list_empty(&queue->list)) {
            break;
        }
        node = list_first_entry(&queue->list, COMM_LIST_NODE, node);
        queue->cnt--;
        queue->size -= node->size;
        list_del(&node->node);
    } while (0);
    ms_mutex_unlock(&queue->lock);

    return node;
}

int get_common_list_node_count(void *handle)
{
    int ret = 0;
    COMM_LIST_HANDLE *queue = (COMM_LIST_HANDLE *)handle;

    if (!queue) {
        return 0;
    }

    ms_mutex_lock(&queue->lock);
    ret = queue->cnt;
    ms_mutex_unlock(&queue->lock);
    return ret;
}



