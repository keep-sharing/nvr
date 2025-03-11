/* 
 * ***************************************************************
 * Filename:      	vapi_comm.c
 * Created at:    	2016.04.28
 * Description:   	common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <paths.h>
#include<sys/time.h>
#include <sys/syscall.h>

#include "vapi_comm.h"


static const RES_S g_stRes[]=
{
    {HI_UNF_VCODEC_CAP_LEVEL_QCIF, 176, 144},
    {HI_UNF_VCODEC_CAP_LEVEL_CIF, 352, 288},
    {HI_UNF_VCODEC_CAP_LEVEL_D1, 720, 576},
    {HI_UNF_VCODEC_CAP_LEVEL_720P, 1280, 720},
    {HI_UNF_VCODEC_CAP_LEVEL_1280x800, 1280, 800},
    {HI_UNF_VCODEC_CAP_LEVEL_800x1280, 800, 1280},
    {HI_UNF_VCODEC_CAP_LEVEL_1488x1280, 1488, 1280},
    {HI_UNF_VCODEC_CAP_LEVEL_1280x1488, 1280, 1488},
    {HI_UNF_VCODEC_CAP_LEVEL_FULLHD, 1920, 1080},
    {HI_UNF_VCODEC_CAP_LEVEL_2160x1280, 2160, 1280},
    {HI_UNF_VCODEC_CAP_LEVEL_1280x2160, 1280, 2160},
    {HI_UNF_VCODEC_CAP_LEVEL_2160x2160, 2160, 2160},
    {HI_UNF_VCODEC_CAP_LEVEL_4096x2160, 4096, 2160},
    {HI_UNF_VCODEC_CAP_LEVEL_2160x4096, 2160, 4096},
    {HI_UNF_VCODEC_CAP_LEVEL_4096x4096, 4096, 4096},
    {HI_UNF_VCODEC_CAP_LEVEL_8192x4096, 8192, 4096},
    {HI_UNF_VCODEC_CAP_LEVEL_4096x8192, 4096, 8192},
    {HI_UNF_VCODEC_CAP_LEVEL_8192x8192, 8192, 8192},
};

HI_S32 comm_mutex_init(pthread_mutex_t *mutex)
{
    HI_S32 ret;
    ret = pthread_mutex_init(mutex, NULL);
    if (ret) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

int comm_mutex_trylock(pthread_mutex_t *mutex)
{
    return pthread_mutex_trylock(mutex);
}

int comm_mutex_timelock(pthread_mutex_t *mutex, int us)
{
    struct timespec tout;
    struct timeval now;
	
    gettimeofday(&now, NULL);
    now.tv_usec += us;
    if(now.tv_usec >= 1000000)
    {
        now.tv_sec += now.tv_usec / 1000000;
        now.tv_usec %= 1000000;
    }
    tout.tv_nsec = now.tv_usec * 1000;
    tout.tv_sec = now.tv_sec;
    return pthread_mutex_timedlock(mutex, &tout);
}

void comm_mutex_lock(pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
}

void comm_mutex_unlock(pthread_mutex_t *mutex)
{
    pthread_mutex_unlock(mutex);
}

void comm_mutex_uninit(pthread_mutex_t *mutex)
{
    pthread_mutex_destroy(mutex);
}

int comm_task_set_name(const char* name)
{
	char thread_name[128] = {0};
	if (!name) 
		return HI_FAILURE;
	snprintf(thread_name, sizeof(thread_name), "%s(%ld)", name, syscall(SYS_gettid));
	prctl(PR_SET_NAME, thread_name, 0, 0, 0);
	return HI_SUCCESS;
}

int comm_task_create(pthread_t *handle, void *(*func)(void *), void *arg)
{
    int ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ret = pthread_create(handle, &attr, func, arg);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
		pthread_attr_destroy(&attr);
        return HI_FAILURE;
    }
	pthread_detach(*handle);
    pthread_attr_destroy(&attr);
    return HI_SUCCESS;	
}

int comm_task_create_join(pthread_t *handle, void *(*func)(void *), void *arg)
{
    int ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    ret = pthread_create(handle, &attr, func, arg);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
		pthread_attr_destroy(&attr);
        return HI_FAILURE;
    }

    pthread_attr_destroy(&attr);
    return HI_SUCCESS;	
}

int comm_task_join(pthread_t *handle)
{
    int ret;
	if(!handle)
		return HI_FAILURE;
    ret = pthread_join(*handle, NULL);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;	
}

int comm_get_vcodec_cap_level(HI_U32 u32Width, HI_U32 u32Height, HI_U32 *pu32Buffsize)
{
    HI_S32 num = sizeof(g_stRes)/sizeof(RES_S);
    HI_S32 i;
    HI_UNF_VCODEC_CAP_LEVEL_E level = HI_UNF_VCODEC_CAP_LEVEL_FULLHD;
    for (i=0; i<num; i++)
    {
        if (g_stRes[i].u32Width >= u32Width && g_stRes[i].u32Height >= u32Height)
        {
            level = g_stRes[i].enCapLevel;
            break;
        }
    }
    if (pu32Buffsize)
        *pu32Buffsize = g_stRes[i].u32Width * g_stRes[i].u32Height * 2;
        
    return level;
}


