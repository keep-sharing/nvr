/*
 * ***************************************************************
 * Filename:        thread_pool.c
 * Created at:      2019.1.25
 * Description:     thread pool api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "tpool.h"

#define WORKER_MAX_IDLE_TIME  (300) //300S

typedef struct worker_s {
    TP_S32  UID;
    TP_THREAD_T thr;
    TP_BOOL bFire;
    TPOOL_S *owner;
    TP_BOOL bBusy;
    TP_S32 idleTime;
    struct list_head node;
} WORKER_S;

static TP_S32
thread_create(TP_THREAD_T *thr, TP_BOOL bJoin, void *(*func)(void *), void *argv)
{
    TP_S32 ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    if (bJoin) {
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    } else {
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    thr->bJoin = bJoin;
    pthread_attr_setstacksize(&attr, 128 * 1024);
    ret = pthread_create(&thr->PID, &attr, func, argv);
    if (ret) {
        TPerr("pthread create failed[%d]", ret);
        pthread_attr_destroy(&attr);
        return TP_FAILURE;
    }
    pthread_attr_destroy(&attr);

    return TP_SUCCESS;
}

static void
thread_destroy(TP_THREAD_T *thr)
{
    TP_S32 ret;

    if (thr->bJoin) {
        ret = pthread_join(thr->PID, NULL);
        if (ret) {
            TPerr("pthread join: %d\n", ret);
        }
    }
}

static inline void
has_job(TPOOL_S *tp)
{
    pthread_mutex_lock(&tp->toDOLock);
    pthread_cond_signal(&tp->toDoJob);
    pthread_mutex_unlock(&tp->toDOLock);
}

static inline TP_S32
wait_job(TPOOL_S *tp, WORKER_S *pWorker)
{
    struct timespec outtime;
    struct timeval now;
    TP_S32 ret = TP_SUCCESS;

    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + 1;
    outtime.tv_nsec = now.tv_usec * 1000;
    pthread_mutex_lock(&tp->toDOLock);
    ret = pthread_cond_timedwait(&tp->toDoJob, &tp->toDOLock, &outtime);
    pthread_mutex_unlock(&tp->toDOLock);
    if ((ret == ETIMEDOUT && (++pWorker->idleTime > tp->maxIdleTime)) || pWorker->bFire) {
        return TP_FAILURE;
    } else {
        return TP_SUCCESS;
    }
}

static TJOB_S *
job_pop(TPOOL_S *tp)
{
    TJOB_S *pJob = NULL;

    pthread_mutex_lock(&tp->jobLock);
    pJob = list_first_entry_or_null(&tp->jobsList, TJOB_S, node);
    if (pJob) {
        list_del(&pJob->node);
        tp->jobs--;
    }
    pthread_mutex_unlock(&tp->jobLock);

    return pJob;
}

static TJOB_S *
job_alloc(TPOOL_S *tp)
{
    TJOB_S *pJob = NULL;

    if (tp->bRun == TP_NO) {
        TPwarm("no ready");
        return NULL;
    }

    if (tp->maxJobs > 0 && tp->maxJobs <= tp->jobs) {
        if (tp->enPolicy == TP_JOB_DISCARD) {
            TPwarm("jobs over maxJobs[%d], discard", tp->maxJobs);
            return NULL;
        } else if (tp->enPolicy == TP_JOB_REPLACE) {
            while (tp->maxJobs <= tp->jobs) {
                pJob = job_pop(tp);
                if (pJob) {
                    tp_free(pJob);
                }
            }
        }
    }

    pJob = tp_calloc(1, sizeof(TJOB_S));

    if (!pJob) {
        TPerr("jobs calloc failed");
        return NULL;
    }

    return pJob;
}

static void
job_free(TJOB_S *pJob)
{
    if (pJob) {
        tp_free(pJob);
    }
}

static TP_S32
job_push(TPOOL_S *tp, TJOB_S *pJob, TP_BOOL bUrgent)
{
    TP_S32 ret = TP_FAILURE;

    pthread_mutex_lock(&tp->jobLock);
    if (tp->bRun) {
        if (bUrgent) {
            list_add(&pJob->node, &tp->jobsList);
        } else {
            list_add_tail(&pJob->node, &tp->jobsList);
        }
        tp->jobs++;
        ret = TP_SUCCESS;
        has_job(tp);
    }
    pthread_mutex_unlock(&tp->jobLock);

    return ret;
}

static void
job_clear(TPOOL_S *tp)
{
    TJOB_S *pJob = NULL;

    for (;;) {
        pJob = job_pop(tp);
        if (pJob) {
            tp_free(pJob);
        } else {
            break;
        }
    }
}

static void
fire_one_worker(TPOOL_S *tp, WORKER_S *pWorker)
{
    list_del(&pWorker->node);
    tp->workers--;
    pWorker->bFire = TP_YES;
    thread_destroy(&pWorker->thr);
    tp_free(pWorker);
}

static void *
worker_thread(void *argv)
{
    WORKER_S *pWorker = (WORKER_S *)argv;
    TPOOL_S *tp = pWorker->owner;
    TJOB_S *pJob = NULL;

    prctl(PR_SET_NAME, "tpool_worker");
    pWorker->UID = syscall(SYS_gettid);
    while (tp->bRun && !pWorker->bFire) {
        pJob = job_pop(tp);
        if (pJob) {
            if (pWorker->bBusy == TP_NO) {
                pthread_mutex_lock(&tp->workerLock);
                tp->busy++;
                pthread_mutex_unlock(&tp->workerLock);
                pWorker->bBusy = TP_YES;
            }

            if (pJob->work) {
                pJob->work(pJob->argv);
            }
            if (pJob->after) {
                pJob->after(pJob->argv);
            }
            tp_free(pJob);
            pWorker->idleTime = 0;
            prctl(PR_SET_NAME, "tpool_worker");
            continue;
        } else {
            if (wait_job(tp, pWorker) != TP_SUCCESS) {
                pthread_mutex_lock(&tp->workerLock);
                if (pWorker->bBusy == TP_YES) {
                    tp->busy--;
                    pWorker->bBusy = TP_NO;
                }
                if (tp->workers > tp->minWorkers || pWorker->bFire) {
                    fire_one_worker(tp, pWorker);
                    pthread_mutex_unlock(&tp->workerLock);
                    break;
                }
                pthread_mutex_unlock(&tp->workerLock);
            }
        }
    }

    return NULL;
}

static TP_S32
hire_one_worker(TPOOL_S *tp)
{
    WORKER_S *pWorker = tp_calloc(1, sizeof(WORKER_S));
    if (!pWorker) {
        TPerr("Worker calloc failed");
        return TP_FAILURE;
    }
    pWorker->owner = tp;
    pWorker->bFire = TP_NO;
    pWorker->bBusy = TP_YES;

    list_add(&pWorker->node, &tp->workersList);
    tp->workers++;
    tp->busy++;

    return thread_create(&pWorker->thr, TP_NO, worker_thread, pWorker);
}

static void
dismiss_workers(TPOOL_S *tp)
{
    WORKER_S *pWorker;
    WORKER_S *n;

    pthread_mutex_lock(&tp->workerLock);
    list_for_each_entry_safe(pWorker, n, &tp->workersList, node) {
        pWorker->bFire = TP_YES;
    }
    pthread_mutex_unlock(&tp->workerLock);
    pthread_mutex_lock(&tp->toDOLock);
    pthread_cond_broadcast(&tp->toDoJob);
    pthread_mutex_unlock(&tp->toDOLock);

    while (tp->workers) {
        usleep(50000);
    }
}

TP_S32
tpool_submit_job(TPOOL_S *tp,
                 void *(*work)(void *),
                 void *(*after)(void *),
                 void *argv,
                 TP_BOOL bUrgent)
{
    if (tp->bRun == TP_NO) {
        return TP_FAILURE;
    }

    TJOB_S *pJob = job_alloc(tp);

    if (!pJob) {
        return TP_FAILURE;
    }

    pJob->work = work;
    pJob->after = after;
    pJob->argv = argv;

    if (job_push(tp, pJob, bUrgent) != TP_SUCCESS) {
        TPwarm("job push failed");
        job_free(pJob);
        return TP_FAILURE;
    }
    //wait a worker to get the job
    usleep(50000);

    pthread_mutex_lock(&tp->workerLock);
    if (tp->jobs > 0 && tp->busy == tp->workers) {
        if (!tp->maxWorkers || tp->workers < tp->maxWorkers) {
            hire_one_worker(tp);
        } else {
            TPwarm("worker over max[%d], wait some time to work", tp->maxWorkers);
        }
    }
    pthread_mutex_unlock(&tp->workerLock);
    return TP_SUCCESS;
}

void
tpool_set_max_idle_time(TPOOL_S *tp, TP_U32 seconds)
{
    tp->maxIdleTime = seconds;
}

void
tpool_set_reject_policy(TPOOL_S *tp, REJECT_E enPolicy)
{
    tp->enPolicy = enPolicy;
}

void
tpool_state_dump(TPOOL_S *tp)
{
    WORKER_S *pWorker;
    WORKER_S *n;
    TP_U8 i = 0;

    pthread_mutex_lock(&tp->workerLock);
    TPinfo("Tpool total worker:         %d", tp->workers);
    TPinfo("Tpool busy  worker:         %d", tp->busy);
    TPinfo("Tpool unexecuted  jobs:     %d", tp->jobs);
    TPinfo("worker detail:");
    list_for_each_entry_safe(pWorker, n, &tp->workersList, node) {
        TPinfo("    [%d]worker uid:   %d", i, pWorker->UID);
        TPinfo("        state   :%s", pWorker->bBusy ? "Busy" : "Idle");
        i++;
    }
    pthread_mutex_unlock(&tp->workerLock);
}

TPOOL_S *
tpool_init(TP_U32 minWorkers, TP_U32 maxWorkers, TP_U32 maxJobs)
{
    TPOOL_S *tp = tp_calloc(1, sizeof(TPOOL_S));

    if (!tp) {
        TPerr("calloc failed!");
        return NULL;
    }

    tp->minWorkers = minWorkers;
    tp->maxWorkers = maxWorkers;
    tp->maxJobs    = maxJobs;
    tp->maxIdleTime = WORKER_MAX_IDLE_TIME;
    tp->enPolicy   = TP_JOB_DISCARD;

    pthread_mutex_init(&tp->workerLock, NULL);
    pthread_mutex_init(&tp->jobLock, NULL);
    pthread_mutex_init(&tp->toDOLock, NULL);
    pthread_cond_init(&tp->toDoJob, NULL);

    INIT_LIST_HEAD(&tp->workersList);
    INIT_LIST_HEAD(&tp->jobsList);
    tp->bRun = TP_YES;
    TPinfo("minWorkers[%d], maxWorkers[%d], maxJobs[%d]",
           minWorkers, maxWorkers, maxJobs);
    return tp;
}

void
tpool_uninit(TPOOL_S *tp)
{
    tp->bRun = TP_NO;
    job_clear(tp);
    dismiss_workers(tp);

    pthread_mutex_destroy(&tp->workerLock);
    pthread_mutex_destroy(&tp->jobLock);
    pthread_mutex_destroy(&tp->toDOLock);
    pthread_cond_destroy(&tp->toDoJob);
    tp_free(tp);
}

