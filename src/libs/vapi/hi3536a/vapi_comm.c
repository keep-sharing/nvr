/*
 * ***************************************************************
 * Filename:      	vapi_comm.c
 * Created at:    	2015.10.21
 * Description:   	common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <errno.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "vapi_comm.h"

td_s32 comm_mutex_init(pthread_mutex_t *mutex)
{
    td_s32 ret;
    ret = pthread_mutex_init(mutex, NULL);
    if (ret) {
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

int comm_mutex_timelock(pthread_mutex_t *mutex, int us)
{
    struct timespec tout;
    struct timeval  now;

    gettimeofday(&now, NULL);
    now.tv_usec += us;
    if (now.tv_usec >= 1000000) {
        now.tv_sec += now.tv_usec / 1000000;
        now.tv_usec %= 1000000;
    }
    tout.tv_nsec = now.tv_usec * 1000;
    tout.tv_sec  = now.tv_sec;
    return pthread_mutex_timedlock(mutex, &tout);
}

int comm_mutex_trylock(pthread_mutex_t *mutex)
{
    return pthread_mutex_trylock(mutex);
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

int comm_task_set_name(const char *name)
{
    char thread_name[128] = {0};
    if (!name)
        return TD_FAILURE;
    snprintf(thread_name, sizeof(thread_name), "%s(%ld)", name, syscall(SYS_gettid));
    prctl(PR_SET_NAME, thread_name, 0, 0, 0);
    return TD_SUCCESS;
}

int comm_task_create(pthread_t *handle, void *(*func)(void *), void *arg)
{
    int            ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, THREAD_STACK_1M_SIZE);
    ret = pthread_create(handle, &attr, func, arg);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
        pthread_attr_destroy(&attr);
        return TD_FAILURE;
    }
    pthread_detach(*handle);
    pthread_attr_destroy(&attr);
    return TD_SUCCESS;
}

int comm_task_create_join(pthread_t *handle, void *(*func)(void *), void *arg)
{
    int            ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, THREAD_STACK_1M_SIZE);
    ret = pthread_create(handle, &attr, func, arg);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
        pthread_attr_destroy(&attr);
        return TD_FAILURE;
    }

    pthread_attr_destroy(&attr);
    return TD_SUCCESS;
}

int comm_task_join(pthread_t *handle)
{
    int ret;
    if (!handle)
        return TD_FAILURE;
    ret = pthread_join(*handle, NULL);
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

int comm_epoll_create(int epmax)
{
    return epoll_create(epmax);
}

int comm_epoll_destroy(int epfd)
{
    return close(epfd);
}

void comm_epoll_add(int epfd, int client, int event)
{
    struct epoll_event ev;

    ev.data.fd = client;
    ev.events  = event | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
}

void comm_epoll_del(int epfd, int client, int event)
{
    struct epoll_event ev;

    ev.data.fd = client;
    ev.events  = event | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_DEL, client, &ev);
}

int comm_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    return epoll_wait(epfd, events, maxevents, timeout);
}
