#include "aio-timeout.h"
#include "sys/locker.h"
#include "sys/atomic.h"
#include "sys/system.h"
#include "sys/onetime.h"
#include "twtimer.h"
#include <assert.h>

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
#define ms_timeout_print(format, ...) \
    do{\
        char stime[32] = {0};\
        ms_time_to_string(stime, sizeof(stime));\
        printf("[%s %d==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)
#else    
#define ms_timeout_print(format, ...)    
#endif
static time_wheel_t* s_timer;
static onetime_t s_init = ONETIME_INIT;

static void aio_timeout_init(void)
{
    ms_timeout_print("[david debug] aio timeout init 00");
	s_timer = time_wheel_create(system_clock());
    ms_timeout_print("[david debug] aio timeout init s_timer:%p clock:%llu 11", s_timer, system_clock());
}

static void aio_timeout_clean(void)
{
    ms_timeout_print("[david debug] aio timeout clean");
	time_wheel_destroy(s_timer);
}

void aio_timeout_process(void)
{
	onetime_exec(&s_init, aio_timeout_init);

	twtimer_process(s_timer, system_clock());
}

int aio_timeout_start(struct aio_timeout_t* timeout, int timeoutMS, void (*notify)(void* param), void* param)
{
	struct twtimer_t* timer;
	timer = (struct twtimer_t*)timeout->reserved;
	assert(sizeof(struct twtimer_t) <= sizeof(timeout->reserved));

	onetime_exec(&s_init, aio_timeout_init);

	timer->param = param;
	timer->ontimeout = notify;
	timer->expire = system_clock() + timeoutMS;
	//ms_timeout_print("[david debug] twtimer_t size:%d reserved size:%d", sizeof(struct twtimer_t), sizeof(timeout->reserved));
    ms_timeout_print("[david debug] timer:%p expire:%llu timeoutMS:%d start.", timer, timer->expire, timeoutMS);
	return twtimer_start(s_timer, timer);
}

int aio_timeout_stop(struct aio_timeout_t* timeout)
{
	struct twtimer_t* timer;
	timer = (struct twtimer_t*)timeout->reserved;
	assert(sizeof(struct twtimer_t) <= sizeof(timeout->reserved));
    ms_timeout_print("[david debug] timer:%p expire:%llu stop.", timer, timer->expire);
	return twtimer_stop(s_timer, timer);
}
