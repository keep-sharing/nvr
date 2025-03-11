#include "aio-worker.h"
#include "aio-socket.h"
#include "aio-timeout.h"
#include "sys/thread.h"
#include <stdio.h>
#include <errno.h>

#define VMIN(a, b)	((a) < (b) ? (a) : (b))

static int s_running;
static pthread_t s_thread[1000];

#if 0
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
staitc void ms_time_to_string(char *stime, int nLen)
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
#define ms_worker_rtp_print(format, ...) \
    do{\
        char stime[32] = {0};\
        ms_time_to_string(stime, sizeof(stime));\
        printf("[%s %d==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)
#else    
#define ms_worker_rtp_print(format, ...)    
#endif
static int STDCALL aio_worker(void* param)
{
	int i = 0, r = 0;
	int idx = (int)(intptr_t)param;
    ms_worker_rtp_print("############[david debug] idx:%d#####00#####\n", idx);
	while (s_running && (r >= 0 || EINTR == errno || EAGAIN == errno)) // ignore epoll EINTR
	{
		r = aio_socket_process(idx ? 2000 : 64);
		if (0 == idx && (0 == r || i++ > 100))
		{
			i = 0;
            ms_worker_rtp_print("############[david debug] r:%d idx:%d i:%d######11######", r, idx, i);
			aio_timeout_process();
		}
        ms_worker_rtp_print("############[david debug] r:%d idx:%d i:%d######22######", r, idx, i);
	}

	printf("%s[%d] exit => %d.\n", __FUNCTION__, idx, errno);
	return 0;
}

void aio_worker_init(int num)
{
	s_running = 1;
	num = VMIN(num, sizeof(s_thread) / sizeof(s_thread[0]));
	aio_socket_init(num);

	while (num-- > 0)
	{
		thread_create(&s_thread[num], aio_worker, (void*)(intptr_t)num);
	}
}

void aio_worker_clean(int num)
{
	s_running = 0;
	num = VMIN(num, sizeof(s_thread) / sizeof(s_thread[0]));
	while (num-- > 0)
	{
		thread_destroy(s_thread[num]);
	}

	aio_socket_clean();
}
