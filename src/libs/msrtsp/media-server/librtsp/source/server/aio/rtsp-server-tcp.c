#include "rtsp-server-aio.h"
#include "aio-transport.h"
#include "rtp-over-rtsp.h"
#include "sys/sock.h"
#include "sys/atomic.h"
#include "sys/system.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#include <fcntl.h>
#include <unistd.h>

#define TIMEOUT_RECV 20000
#define TIMEOUT_SEND 10000

#define MS_TIMEOUT_RECV 60000
#define MS_TIMEOUT_SEND 10000

struct rtsp_session_t {
    socket_t socket;
    aio_transport_t *aio;
    struct rtp_over_rtsp_t rtp;
    int rtsp_need_more_data;
    uint8_t buffer[4 * 1024];

    struct rtsp_server_t *rtsp;
    struct sockaddr_storage addr;
    socklen_t addrlen;

    void (*onerror)(void *param, rtsp_server_t *rtsp, int code);
    void (*onrtp)(void *param, uint8_t channel, const void *data, uint16_t bytes);
    void (*onaioready)(void *param, rtsp_server_t *rtsp, size_t bytes);
    void *param;
};

#if 0
void ms_time_to_string(char *stime, int nLen)
{
    if (!stime) {
        return;
    }
    struct tm temp;
    time_t ntime = time(0);

    localtime_r((long *)&ntime, &temp);
    snprintf(stime, nLen, "%4d-%02d-%02d_%02d_%02d_%02d",
             temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday,
             temp.tm_hour, temp.tm_min, temp.tm_sec);

    return ;
}

#define ms_rtp_print(format, ...) \
    do{\
        char stime[32] = {0};\
        ms_time_to_string(stime, sizeof(stime));\
        printf("[%s %d==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)
#else
#define ms_rtp_print(format, ...)
#endif

static void rtsp_session_ondestroy(void *param)
{
    struct rtsp_session_t *session;
    session = (struct rtsp_session_t *)param;

    // user call rtsp_server_destroy
    if (session->rtsp) {
        rtsp_server_destroy(session->rtsp);
        session->rtsp = NULL;
    }

    if (session->rtp.data) {
        assert(session->rtp.capacity > 0);
        free(session->rtp.data);
        session->rtp.data = NULL;
        session->rtp.capacity = 0;
    }

#if defined(_DEBUG) || defined(DEBUG)
    memset(session, 0xCC, sizeof(*session));
#endif
    free(session);
}

static void rtsp_session_onrecv(void *param, int code, size_t bytes)
{
    int dosleep = 0;
    size_t remain;
    const uint8_t *p, *end;
    struct rtsp_session_t *session;
    session = (struct rtsp_session_t *)param;

    ms_rtp_print("[david debug] session onrecv code:%d bytes:%d 0000.", code, bytes);
    if (0 == code && 0 == bytes) {
        code = ECONNRESET;
    }
    ms_rtp_print("[david debug] session onrecv code:%d bytes:%d 1111.", code, bytes);
    if (0 == code) {
        p = session->buffer;
        end = session->buffer + bytes;
        do {
            if (0 == session->rtsp_need_more_data && ('$' == *p || 0 != session->rtp.state)) {
                p = rtp_over_rtsp(&session->rtp, p, end, &dosleep);
                //ms_rtp_print("[david debug] session onrecv p:%p end:%p state:%d 2222.", p, end, session->rtp.state);
            } else {
                remain = end - p;
                code = rtsp_server_input(session->rtsp, p, &remain);
                session->rtsp_need_more_data = code;
                if (0 == code) {
                    ms_rtp_print("[david debug] p:%p end:%p code:%d bytes:%d remain:%d dddd:%c error 2222", p, end, code, bytes, remain, *(end - remain));
                    // TODO: pipeline remain data
                    //assert(bytes > remain);
                    //assert(0 == remain || '$' == *(end - remain));
                    if (bytes <= remain) {
                        printf("[david debug] session onrecv p:%p end:%p code:%d bytes:%d remain:%d dddd:%c RESET!!\n", p, end, code, bytes, remain, *(end - remain));
                        code = ECONNRESET;
                        goto NRESET;
                    }
                    if ((0 != remain) && '$' != *(end - remain)) {
                        printf("[david debug] session onrecv p:%p end:%p code:%d bytes:%d remain:%d dddd:%c RESET!!!\n", p, end, code, bytes, remain, *(end - remain));
                        code = ECONNRESET;
                        goto NRESET;
                    }
                }
                p = end - remain;
                ms_rtp_print("[david debug] session onrecv p:%p end:%p code:%d remain:%d dddd:%c 3333.", p, end, code, remain, *(end - remain));
            }
        } while (p < end && 0 == code);

        if (code >= 0) {
            // need more data
            code = aio_transport_recv(session->aio, session->buffer, sizeof(session->buffer));
        }
    }

NRESET:   
    // error or peer closed
    if (0 != code || 0 == bytes) {
        ms_rtp_print("[david debug] session onrecv code:%d bytes:%d 4444.", code, bytes);
        session->onerror(session->param, session->rtsp, code ? code : ECONNRESET);
        aio_transport_destroy(session->aio);
    }
}

static void rtsp_session_onsend(void *param, int code, size_t bytes)
{
    struct rtsp_session_t *session;
    session = (struct rtsp_session_t *)param;
    
    //session->server->onsend(session, code, bytes);
    ms_rtp_print("[david debug] param:%p code:%d bytes:%d", param, code, bytes);
    if (0 != code) {
        //ms_rtp_print("[david debug] session_onsend code:%d bytes:%d failed.", code, bytes);
        session->onerror(session->param, session->rtsp, code);
        aio_transport_destroy(session->aio);
    } else {
        session->onaioready(session->param, session->rtsp, bytes);
    }
    
    (void)bytes;
}

static int rtsp_session_send(void *ptr, const void *data, size_t bytes)
{
    //ms_rtp_print("[david debug] rtsp_session_send ptr:%p data:%p bytes:%d start", ptr, data, bytes);
    int ret = -1;
    struct rtsp_session_t *session;
    session = (struct rtsp_session_t *)ptr;
    //return aio_tcp_transport_send(session->aio, data, bytes);
    
    // TODO: send multiple rtp packet once time
    ret = (bytes == socket_send(session->socket, data, bytes, 0)) ? 0 : -1;
    //ms_rtp_print("[david debug] rtsp_session_send ptr:%p data:%p bytes:%d end", ptr, data, bytes);

    return ret;
}

static int rtsp_session_aio_send(void *ptr, const void *data, size_t bytes)
{
    ms_rtp_print("[david debug] rtsp_session_send ptr:%p data:%p bytes:%d start", ptr, data, bytes);
    struct rtsp_session_t *session;
    session = (struct rtsp_session_t *)ptr;
    int ret = aio_transport_send(session->aio, data, bytes);
    ms_rtp_print("[david debug] rtsp_session_send ptr:%p data:%p bytes:%d end", ptr, data, bytes);
    
    return ret;
}

int rtsp_transport_tcp_create(socket_t socket, const struct sockaddr *addr, socklen_t addrlen,
                              struct aio_rtsp_handler_t *handler, void *param)
{
    ms_rtp_print("[david debug] rtsp transport tcp create start.");
    char ip[65];
    unsigned short port = 0;
    struct rtsp_session_t *session;
    struct rtsp_handler_t rtsphandler;
    struct aio_transport_handler_t h;

    if (!addr) {
        printf("rtsp_transport_tcp_create invalid params. addr is NULL.\n");
        return -1;
    }

    memset(&h, 0, sizeof(h));
    h.ondestroy = rtsp_session_ondestroy;
    h.onrecv = rtsp_session_onrecv;
    h.onsend = rtsp_session_onsend;

    memcpy(&rtsphandler, &handler->base, sizeof(rtsphandler));
    rtsphandler.send = rtsp_session_send;
    rtsphandler.aiosend = rtsp_session_aio_send;
    
    session = (struct rtsp_session_t *)calloc(1, sizeof(*session));
    if (!session) {
        return -1;
    }
    session->socket = socket;
    socket_addr_to(addr, addrlen, ip, &port);
    assert(addrlen <= sizeof(session->addr));
    session->addrlen = addrlen < sizeof(session->addr) ? addrlen : sizeof(session->addr);
    memcpy(&session->addr, addr, session->addrlen); // save client ip/port

    session->param = param;
    session->onerror = handler->onerror;
    session->onaioready = handler->onaioready;
    session->aio = aio_transport_create(socket, &h, session);
    session->rtsp = rtsp_server_create(ip, port, &rtsphandler, param,
                                       session);    // reuse-able, don't need create in every link
                                       
    ms_rtp_print("[david debug] ip:%s port:%d rtsp:%p param:%p", ip, port, session->rtsp, param);
    if (!session->rtsp || !session->aio) {
        rtsp_session_ondestroy(session);
        return -1;
    }

    session->rtp.param = param;
    session->rtp.onrtp = handler->onrtp;
    aio_transport_set_timeout(session->aio, MS_TIMEOUT_RECV, MS_TIMEOUT_SEND);
    ms_rtp_print("[david debug] aio set timeout recv:%d send:%d", MS_TIMEOUT_RECV, MS_TIMEOUT_SEND);
    if (0 != aio_transport_recv(session->aio, session->buffer, sizeof(session->buffer))) {
        ms_rtp_print("[david debug] ip:%s port:%d failed.", ip, port);
        rtsp_session_ondestroy(session);
        return -1;
    }
    ms_rtp_print("[david debug] ip:%s port:%d success.", ip, port);

    return 0;
}

int rtsp_set_socket_nonblock(void *rtsp)
{
    int fd;
    int flag;
    struct rtsp_session_t *session;

    if (!rtsp) {
        return -1;
    }
    
    session = (struct rtsp_session_t *)rtsp_server_get_session(rtsp);
    if (!session) {
        return -1;
    }

    fd = session->socket;
    flag = fcntl(fd, F_GETFL, 0);
    if (flag < 0) {
        return -2;
    }
    if (fcntl(fd, F_SETFL, flag | O_NONBLOCK) < 0) {
        return -3;
    }

    return 0;
}

