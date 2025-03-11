// Timing Wheel Timer(timeout)
// 64ms per bucket
// http://www.cs.columbia.edu/~nahum/w6998/papers/sosp87-timing-wheels.pdf

#include "twtimer.h"
#include "sys/spinlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

#define TIME_RESOLUTION 3 // (0xFFFFFFFF << 3) / (24 * 3600 * 1000) ~= 397day
#define TIME(clock) ((clock) >> TIME_RESOLUTION) // per 8ms

#define TVR_BITS 8
#define TVN_BITS 6
#define TVR_SIZE (1 << TVR_BITS)
#define TVN_SIZE (1 << TVN_BITS)
#define TVR_MASK (TVR_SIZE - 1)
#define TVN_MASK (TVN_SIZE - 1)

#define TVR_INDEX(clock)    ((int)((clock >> TIME_RESOLUTION) & TVR_MASK))
#define TVN_INDEX(clock, n) ((int)((clock >> (TIME_RESOLUTION + TVR_BITS + (n * TVN_BITS))) & TVN_MASK))

struct time_bucket_t
{
	struct twtimer_t* first;
};

struct time_wheel_t
{
	spinlock_t locker;

	uint64_t count;
	uint64_t clock;
	struct time_bucket_t tv1[TVR_SIZE];
	struct time_bucket_t tv2[TVN_SIZE];
	struct time_bucket_t tv3[TVN_SIZE];
	struct time_bucket_t tv4[TVN_SIZE];
	struct time_bucket_t tv5[TVN_SIZE];
};

static int twtimer_add(struct time_wheel_t* tm, struct twtimer_t* timer);
static int twtimer_cascade(struct time_wheel_t* tm, struct time_bucket_t* tv, int index);

#if 1
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

#define ms_time_wheel_print(format, ...) \
    do{\
        char stime[32] = {0};\
        ms_time_to_string(stime, sizeof(stime));\
        printf("[%s %d==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)
#else    
#define ms_time_wheel_print(format, ...)    
#endif

struct time_wheel_t* time_wheel_create(uint64_t clock)
{
//#if defined(_HI3536C_)
//    ms_time_wheel_print("[david debug] time_wheel_create is 3536c!");
//#else
//    ms_time_wheel_print("[david debug] time_wheel_create is no 3536c!");
//#endif
	struct time_wheel_t* tm;
	tm = (struct time_wheel_t*)calloc(1, sizeof(*tm));
	if (tm)
	{
		tm->count = 0;
		tm->clock = clock;
		spinlock_create(&tm->locker);
        //ms_time_wheel_print("[david debug] time wheel create tm:%p locker:%p count:%llu clock:%llu 11", tm, &tm->locker, tm->count, tm->clock);
	}
	return tm;
}

int time_wheel_destroy(struct time_wheel_t* tm)
{
	assert(0 == tm->count);
	return spinlock_destroy(&tm->locker);
}

int twtimer_start(struct time_wheel_t* tm, struct twtimer_t* timer)
{
	int r;
	assert(timer->ontimeout);
	spinlock_lock(&tm->locker);
	r = twtimer_add(tm, timer);
    //ms_time_wheel_print("[david debug] tm:%p count:%llu timer:%p r:%d", tm, tm->count, timer, r);
	spinlock_unlock(&tm->locker);
	return r;
}

int twtimer_stop(struct time_wheel_t* tm, struct twtimer_t* timer)
{
	struct twtimer_t** pprev;
	spinlock_lock(&tm->locker);
	pprev = timer->pprev;
	if (timer->pprev)
	{
		--tm->count; // timer validation ???
		if (tm->count < 0) {
            //ms_time_wheel_print("[david debug] twtimer_stop count:%d error!!!", tm->count);
        }
		*timer->pprev = timer->next;
	}
	if (timer->next)
		timer->next->pprev = timer->pprev;
	timer->pprev = NULL;
	timer->next = NULL;
    //ms_time_wheel_print("[david debug] tm:%p count:%llu timer:%p", tm, tm->count, timer);
	spinlock_unlock(&tm->locker);
	return pprev ? 0 : -1;
}

int twtimer_process(struct time_wheel_t* tm, uint64_t clock)
{
	int index, i = 0;
	struct twtimer_t* timer;
    struct time_bucket_t bucket;

	spinlock_lock(&tm->locker);
    //ms_time_wheel_print("[david debug] twtimer_process tm:%p locker:%p tmclock:%llu(%llu) clock:%llu(%llu) count:%llu start.", tm, &tm->locker, tm->clock, TIME(tm->clock), clock, TIME(clock), tm->count);
	while(TIME(tm->clock) < TIME(clock))
	{
	    //i++;
		index = TVR_INDEX(tm->clock);
        if (index == 0) {
            //ms_time_wheel_print("[david debug] index:%d tv1:%p tv2:%p tv3:%p tv4:%p tv5:%p", index, tm->tv1, tm->tv2, tm->tv3, tm->tv4, tm->tv5);
            //ms_time_wheel_print("[david debug] index:%d tmclock:%llu clock:%llu", index, TIME(tm->clock), TIME(clock));
            //ms_time_wheel_print("[david debug] TVN_INDEX 0:%llu 1:%llu 2:%llu 3:%llu", TVN_INDEX(tm->clock, 0), TVN_INDEX(tm->clock, 1), TVN_INDEX(tm->clock, 2), TVN_INDEX(tm->clock, 3));
        }
        
		if (0 == index 
			&& 0 == twtimer_cascade(tm, tm->tv2, TVN_INDEX(tm->clock, 0))
			&& 0 == twtimer_cascade(tm, tm->tv3, TVN_INDEX(tm->clock, 1))
			&& 0 == twtimer_cascade(tm, tm->tv4, TVN_INDEX(tm->clock, 2)))
		{
			twtimer_cascade(tm, tm->tv5, TVN_INDEX(tm->clock, 3));
		}

		// move bucket
		bucket.first = tm->tv1[index].first;
		tm->tv1[index].first = NULL; // clear
		tm->clock += (1 << TIME_RESOLUTION);

        //ms_time_wheel_print("[david debug] i:%d index:%d first:%p", ++i, index, bucket.first);
		// trigger timer
        while (bucket.first)
		{
            timer = bucket.first;
			if (timer->next)
				timer->next->pprev = &bucket.first;
            bucket.first = timer->next;
            
			timer->next = NULL;
			timer->pprev = NULL;
			--tm->count;
			if (timer->ontimeout)
			{
				spinlock_unlock(&tm->locker);
                //assert(timer->expire >= clock - 2 * (1<<TIME_RESOLUTION));
                //assert(timer->expire <= clock + 2 * (1<<TIME_RESOLUTION));
                //ms_time_wheel_print("[david debug] timer:%p i:%d index:%d ontimeout done.", timer, i, index);
				timer->ontimeout(timer->param);
				spinlock_lock(&tm->locker);
			}
		}	
    }
    //ms_time_wheel_print("[david debug] twtimer_process tm:%p locker:%p tmclock:%llu(%llu) clock:%llu(%llu) count:%llu i:%d end.", tm, &tm->locker, tm->clock, TIME(tm->clock), clock, TIME(clock), tm->count, i);
	spinlock_unlock(&tm->locker);
	return (int)(tm->clock - clock);
}

static int twtimer_cascade(struct time_wheel_t* tm, struct time_bucket_t* tv, int index)
{
    //david mark index < 64
    if (index >= TVN_SIZE) {
        //ms_time_wheel_print("[david debug] index:%d TVN_SIZE:%d ERROR!", index, TVN_SIZE);
        return index;
    } else {
        //ms_time_wheel_print("[david debug] tm:%p tv:%p index:%d TVN_SIZE:%d OOK!", tm, tv, index, TVN_SIZE);
    }
    int i = 0;
	struct twtimer_t* timer = NULL;
	struct twtimer_t* next = NULL;
	next = tv[index].first;
	tv[index].first = NULL; // clear

    if (next) {
        //ms_time_wheel_print("[david debug] timer:%p next:%p index:%d !!!", timer, next, index);
    }
    
	for (timer = next; timer; timer = next) {
	    //ms_time_wheel_print("[david debug] tm next:%p count:%llu i:%d start!!!", next, tm->count, i++);
        //ms_time_wheel_print("[david debug] tm tv1:%p tv2:%p tv3:%p tv4:%p tv5:%p tv:%p", tm->tv1, tm->tv2, tm->tv3, tm->tv4, tm->tv5, tv);
		--tm->count; // start will add count
		next = timer->next;
		timer->next = NULL;
		timer->pprev = NULL;
		twtimer_add(tm, timer);
        //ms_time_wheel_print("[david debug] tm next:%p count:%llu i:%d end!!!", next, tm->count, i);
	}

	return index;
}

static int twtimer_add(struct time_wheel_t* tm, struct twtimer_t* timer)
{
    //ms_time_wheel_print("[david debug] tm:%p count:%llu timer:%p", tm, tm->count, timer);
	uint64_t diff;
	struct time_bucket_t* tv;

	assert(timer->ontimeout);
	if (timer->pprev)
	{
		assert(0); // timer have been started
		return EEXIST;
	}

	diff = TIME(timer->expire - tm->clock); // per 64ms

	if (timer->expire < tm->clock)
	{
	    //ms_time_wheel_print("[david debug] diff:%llu timer:%p count:%llu clock:%d 0000", diff, timer, tm->count, TVR_INDEX(tm->clock));
		tv = tm->tv1 + TVR_INDEX(tm->clock);
	}
	else if (diff < (1 << TVR_BITS))                        //256
	{
	    //ms_time_wheel_print("[david debug] diff:%llu timer:%p count:%llu expire idx:%d 1111", diff, timer, tm->count, TVR_INDEX(timer->expire));
		tv = tm->tv1 + TVR_INDEX(timer->expire);
	}
	else if (diff < (1 << (TVR_BITS + TVN_BITS)))         //16384
	{
	    //ms_time_wheel_print("[david debug] diff:%llu timer:%p count:%llu expire idx:%d 2222", diff, timer, tm->count, TVN_INDEX(timer->expire, 0));
		tv = tm->tv2 + TVN_INDEX(timer->expire, 0);
	}
	else if (diff < (1 << (TVR_BITS + 2 * TVN_BITS)))     //1048576
	{
	    //ms_time_wheel_print("[david debug] diff:%llu timer:%p count:%llu expire idx:%d 3333", diff, timer, tm->count, TVN_INDEX(timer->expire, 1));
		tv = tm->tv3 + TVN_INDEX(timer->expire, 1);
	}
	else if (diff < (1 << (TVR_BITS + 3 * TVN_BITS)))     //16777216
	{
	    //ms_time_wheel_print("[david debug] diff:%llu timer:%p count:%llu expire idx:%d 4444", diff, timer, tm->count, TVN_INDEX(timer->expire, 2));
		tv = tm->tv4 + TVN_INDEX(timer->expire, 2);
	}
	else if (diff < (1ULL << (TVR_BITS + 4 * TVN_BITS))) //4294967296
	{
	    //ms_time_wheel_print("[david debug] diff:%llu timer:%p count:%llu expire idx:%d 5555", diff, timer, tm->count, TVN_INDEX(timer->expire, 3));
		tv = tm->tv5 + TVN_INDEX(timer->expire, 3);
	}
	else
	{
	    //ms_time_wheel_print("[david debug] diff:%llu timer:%p count:%llu 6666", diff, timer, tm->count);
		spinlock_unlock(&tm->locker);
		assert(0); // exceed max timeout value
		return -1;
	}

	// list insert
	timer->pprev = &tv->first;
	timer->next = tv->first;
	if (timer->next)
		timer->next->pprev = &timer->next;
	tv->first = timer;
	++tm->count;
    
	return 0;
}
