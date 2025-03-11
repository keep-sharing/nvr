/*
 * ***************************************************************
 * Filename:        MFcommon.c
 * Created at:      2017.05.10
 * Description:     msfs common api
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "MFdisk.h"
#include "MFcommon.h"
#include "msfs_notify.h"
#include "tpool.h"

typedef struct eventNode {
    struct mf_event stEvent;
    struct list_head node;
} eventNode;

typedef struct barNode {
    struct mf_bar stBar;
    struct list_head node;
} barNode;

typedef struct mf_common {
    //event
    struct list_head    eventList;
    MUTEX_OBJECT        eventMutex;
    //task
    TPOOL_S            *pstTp;
    //progress bar
    struct list_head    barList;
    MUTEX_OBJECT        barMutex;
    //mem pool size
    struct mem_pool_t   stMpool;

} mf_common;

#define EVENT_FIND_BY_ID(id, pstEventNode) \
    { \
        struct eventNode* pos = NULL; \
        struct eventNode* n = NULL; \
        list_for_each_entry_safe(pos, n, &g_stCommon.eventList, node) \
        { \
            if (pos->stEvent.enId == id) \
            { \
                pstEventNode = pos; \
                break; \
            } \
        } \
    }

#define BAR_FIND_BY_FUNC(func, pstBarNode) \
    { \
        struct barNode* pos = NULL; \
        struct barNode* n = NULL; \
        list_for_each_entry_safe(pos, n, &g_stCommon.barList, node) \
        { \
            if (pos->stBar.func == func) \
            { \
                pstBarNode = pos; \
                break; \
            } \
        } \
    }


MF_S32 g_msfs_debug = MF_ALL;
MF_FUNC_PRINT g_debug_print = NULL;

static struct mf_common g_stCommon;

static MF_S32
comm_event_init(struct mf_common *pstCommon)
{
    ms_mutex_init(&pstCommon->eventMutex);
    INIT_LIST_HEAD(&pstCommon->eventList);

    return MF_SUCCESS;
}

static MF_S32
comm_event_deinit(struct mf_common *pstCommon)
{
    struct eventNode *pos = NULL;
    struct eventNode *n = NULL;

    ms_mutex_lock(&pstCommon->eventMutex);

    list_for_each_entry_safe(pos, n, &pstCommon->eventList, node) {
        list_del(&pos->node);
        ms_free(pos);
    }

    ms_mutex_unlock(&pstCommon->eventMutex);
    ms_mutex_uninit(&pstCommon->eventMutex);

    return MF_SUCCESS;
}

static MF_S32
comm_task_init(struct mf_common *pstCommon)
{
    pstCommon->pstTp = tpool_init(1, 0, 0);
    return MF_SUCCESS;
}

static MF_S32
comm_task_deinit(struct mf_common *pstCommon)
{
    tpool_uninit(pstCommon->pstTp);
    return MF_SUCCESS;
}

static void *
comm_task_do(void *argv)
{
    struct evt2task *pTask = argv;

    mf_comm_event_notify(pTask->from, pTask->enEvent, pTask->argv);

    ms_free(pTask);
    return NULL;
}

static MF_S32
comm_task_submit(TPOOL_S *tp, void *(* work)(void *), void *argv)
{
    return tpool_submit_job(tp, work, NULL, argv, TP_NO);
}

static MF_S32
comm_task_dump(TPOOL_S *tp)
{
    tpool_state_dump(tp);
    return MF_SUCCESS;
}

static MF_S32
comm_progress_bar_init(struct mf_common *pstCommon)
{
    ms_mutex_init(&pstCommon->barMutex);
    INIT_LIST_HEAD(&pstCommon->barList);

    return MF_SUCCESS;
}

static MF_S32
comm_progress_bar_deinit(struct mf_common *pstCommon)
{
    struct barNode *pos = NULL;
    struct barNode *n = NULL;

    ms_mutex_lock(&pstCommon->barMutex);

    list_for_each_entry_safe(pos, n, &pstCommon->barList, node) {
        list_del(&pos->node);
        ms_free(pos);
    }

    ms_mutex_unlock(&pstCommon->barMutex);
    ms_mutex_uninit(&pstCommon->barMutex);

    return MF_SUCCESS;
}

void
mf_comm_progress_bar_register(void *func)
{
    struct barNode *pstBarNode = NULL;

    ms_mutex_lock(&g_stCommon.barMutex);

    pstBarNode = ms_calloc(1, sizeof(struct barNode));
    pstBarNode->stBar.func = func;
    list_add(&pstBarNode->node, &g_stCommon.barList);

    ms_mutex_unlock(&g_stCommon.barMutex);
}

void
mf_comm_progress_bar_unregister(void *func)
{
    struct barNode *pstBarNode = NULL;

    ms_mutex_lock(&g_stCommon.barMutex);

    BAR_FIND_BY_FUNC(func, pstBarNode);
    if (pstBarNode) {
        list_del(&pstBarNode->node);
        ms_free(pstBarNode);
    }

    ms_mutex_unlock(&g_stCommon.barMutex);

}

void
mf_comm_progress_bar_notify(PROGRESS_BAR_E enBar, void *argv)
{
    struct barNode *pos = NULL;
    struct barNode *n = NULL;

    ms_mutex_lock(&g_stCommon.barMutex);

    list_for_each_entry_safe(pos, n, &g_stCommon.barList, node) {
        if (pos->stBar.func) {
            pos->stBar.func(enBar, argv);
        }
    }

    ms_mutex_unlock(&g_stCommon.barMutex);
}

void
mf_comm_event_register(MODULE_E enId, void *func)
{

    struct eventNode *pstEventNode = NULL;

    ms_mutex_lock(&g_stCommon.eventMutex);

    EVENT_FIND_BY_ID(enId, pstEventNode);
    if (!pstEventNode) {
        pstEventNode = ms_calloc(1, sizeof(struct eventNode));
        pstEventNode->stEvent.enId = enId;
        pstEventNode->stEvent.func = func;
        LIST_INSERT_SORT_ASC(pstEventNode, &g_stCommon.eventList, stEvent.enId);
    }

    ms_mutex_unlock(&g_stCommon.eventMutex);
}

void
mf_comm_event_unregister(MODULE_E enId)
{
    struct eventNode *pstEventNode = NULL;

    ms_mutex_lock(&g_stCommon.eventMutex);

    EVENT_FIND_BY_ID(enId, pstEventNode);
    if (pstEventNode) {
        list_del(&pstEventNode->node);
        ms_free(pstEventNode);
    }

    ms_mutex_unlock(&g_stCommon.eventMutex);
}

void
mf_comm_event_notify(MODULE_E module, EVENT_E event, void *argv)
{
    struct eventNode *pos = NULL;
    struct eventNode *n = NULL;

    list_for_each_entry_safe(pos, n, &g_stCommon.eventList, node) {
        if (pos->stEvent.func) {
            pos->stEvent.func(module, event, argv);
        }
    }
}

void
mf_comm_task_notify(MODULE_E module, EVENT_E event, void *argv)
{
    struct evt2task *pTask = ms_calloc(1, sizeof(struct evt2task));

    pTask->from = module;
    pTask->enEvent = event;
    pTask->argv = argv;

    mf_comm_task_submit(comm_task_do, pTask);
}

MF_S32
mf_comm_task_submit(void *(* work)(void *), void *argv)
{
    return comm_task_submit(g_stCommon.pstTp, work, argv);
}

MF_S32
mf_comm_task_dump(void)
{
    return comm_task_dump(g_stCommon.pstTp);
}

void
mf_comm_mem_set(struct mem_pool_t *pstMpool)
{
    g_stCommon.stMpool.diskSize = pstMpool->diskSize;
    g_stCommon.stMpool.recSize  = pstMpool->recSize;
    g_stCommon.stMpool.retrSize = pstMpool->retrSize;
    g_stCommon.stMpool.taskSize = pstMpool->taskSize;
    g_stCommon.stMpool.pbSize   = pstMpool->pbSize;
    g_stCommon.stMpool.logSize  = pstMpool->logSize;
}

struct mem_pool_t *
mf_comm_mem_get()
{
    return &g_stCommon.stMpool;
}

void
mf_comm_debug_level(DBG_E enLevel)
{
    g_msfs_debug = enLevel;
}

void
mf_comm_debug_print(void *print)
{
    g_debug_print = print;
}

MF_S32
mf_common_init()
{
    comm_event_init(&g_stCommon);
    comm_task_init(&g_stCommon);
    comm_progress_bar_init(&g_stCommon);

    return MF_SUCCESS;
}

MF_S32
mf_common_deinit()
{
    comm_event_deinit(&g_stCommon);
    comm_task_deinit(&g_stCommon);
    comm_progress_bar_deinit(&g_stCommon);

    return MF_SUCCESS;
}

