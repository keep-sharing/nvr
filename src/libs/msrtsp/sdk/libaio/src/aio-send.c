#include "aio-send.h"
#include "sys/atomic.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

enum { AIO_STATUS_INIT = 0, AIO_STATUS_START, AIO_STATUS_TIMEOUT };

#define AIO_SEND_START(send) {assert(AIO_STATUS_INIT == send->status);send->status = AIO_STATUS_START;}

#define AIO_START_TIMEOUT(aio, timeout, callback)   \
    if (timeout > 0) {                              \
        aio_timeout_start(&aio->timeout, timeout, callback, aio); \
    } else {}

#define AIO_STOP_TIMEOUT_ON_FAILED(aio, r, timeout) \
    if (0 != r) aio->status = AIO_STATUS_INIT;      \
    if (0 != r && timeout > 0) {                    \
        aio_timeout_stop(&aio->timeout);            \
    } else {}

#if 0
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
static void ms_time_to_string(char *stime, int nLen)
{
    if (!stime) {
        return;
    }
    struct tm temp;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &temp);
    snprintf(stime, nLen, "%4d-%02d-%02d_%02d_%02d_%02d.%03d",
             temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday,
             temp.tm_hour, temp.tm_min, temp.tm_sec, (int)(tv.tv_usec/1000));
    return ;
}
#define ms_aio_send_print(format, ...) \
            do{\
                char stime[32] = {0};\
                ms_time_to_string(stime, sizeof(stime));\
                printf("[%s %d==func:%s file:%s line:%d]==" format "\n", \
                       stime, getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
            }while(0)
#else    
#define ms_aio_send_print(format, ...)    
#endif
static void aio_send_timeout(void *param)
{
    struct aio_send_t *send;
    send = (struct aio_send_t *)param;

	if (atomic_cas32(&send->status, AIO_STATUS_START, AIO_STATUS_TIMEOUT) && send->onsend)
        send->onsend(send->param, ETIMEDOUT, 0);
}

static void aio_send_handler(void *param, int code, size_t bytes)
{
    struct aio_send_t *send;
    send = (struct aio_send_t *)param;

    aio_timeout_stop(&send->timeout);
	if (atomic_cas32(&send->status, AIO_STATUS_START, AIO_STATUS_INIT) && send->onsend)
        send->onsend(send->param, code, bytes);
}

int aio_send(struct aio_send_t* send, int timeout, aio_socket_t aio, const void* buffer, size_t bytes, aio_onsend onsend, void* param)
{
    int r;
    AIO_SEND_START(send);
    send->param = param;
    send->onsend = onsend;
    memset(&send->timeout, 0, sizeof(send->timeout));
    AIO_START_TIMEOUT(send, timeout, aio_send_timeout);
    r = aio_socket_send(aio, buffer, bytes, aio_send_handler, send);
    AIO_STOP_TIMEOUT_ON_FAILED(send, r, timeout);
    return r;
}

int aio_send_v(struct aio_send_t* send, int timeout, aio_socket_t aio, socket_bufvec_t* vec, int n, aio_onsend onsend, void* param)
{
    int r;
    if (AIO_STATUS_INIT != send->status) {
        ms_aio_send_print("[david debug] aio_send_v status:%d is err!", send->status);
    }
    AIO_SEND_START(send);
    send->param = param;
    send->onsend = onsend;
    memset(&send->timeout, 0, sizeof(send->timeout));
    AIO_START_TIMEOUT(send, timeout, aio_send_timeout);
    r = aio_socket_send_v(aio, vec, n, aio_send_handler, send);
    AIO_STOP_TIMEOUT_ON_FAILED(send, r, timeout);
    return r;
}

int aio_sendto(struct aio_send_t* send, int timeout, aio_socket_t aio, const struct sockaddr *addr, socklen_t addrlen, const void* buffer, size_t bytes, aio_onsend onsend, void* param)
{
    int r;
    AIO_SEND_START(send);
    send->param = param;
    send->onsend = onsend;
    memset(&send->timeout, 0, sizeof(send->timeout));
    AIO_START_TIMEOUT(send, timeout, aio_send_timeout);
    r = aio_socket_sendto(aio, addr, addrlen, buffer, bytes, aio_send_handler, send);
    AIO_STOP_TIMEOUT_ON_FAILED(send, r, timeout);
    return r;
}

int aio_sendto_v(struct aio_send_t* send, int timeout, aio_socket_t aio, const struct sockaddr *addr, socklen_t addrlen, socket_bufvec_t* vec, int n, aio_onsend onsend, void* param)
{
    int r;
    AIO_SEND_START(send);
    send->param = param;
    send->onsend = onsend;
    memset(&send->timeout, 0, sizeof(send->timeout));
    AIO_START_TIMEOUT(send, timeout, aio_send_timeout);
    r = aio_socket_sendto_v(aio, addr, addrlen, vec, n, aio_send_handler, send);
    AIO_STOP_TIMEOUT_ON_FAILED(send, r, timeout);
    return r;
}
