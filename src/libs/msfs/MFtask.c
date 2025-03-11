/*
 * ***************************************************************
 * Filename:        MFtask.c
 * Created at:      2017.05.10
 * Description:      task manager
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

#include "MFrecord.h"
#include "MFdisk.h"
#include "MFtask.h"
#include "MFcommon.h"
#include "MFmemory.h"

#define MAX_TIME_SPAN   (30*24*60*60)   //30 days

typedef struct mf_task {
    TASK_HANDLE     handle;
    MF_BOOL         bRun;
    struct mf_mem_t *pstMem;
    MF_U32          timer;
} mf_task;

#define CAL_FIND_BY_DAY(day, pstCal, list) \
    { \
        struct calendor_node_t* cal = NULL; \
        list_for_each_entry(cal, list, node) \
        { \
            if (day == cal->date) \
            { \
                pstCal = cal; \
                break; \
            } \
        } \
    }

static struct mf_task g_stTask;
static struct task_calendar_t *g_pstCalendar;

static void
task_mem_init(struct mf_task *pstTask)
{
    struct mem_pool_t *pstMpool = mf_comm_mem_get();

    if (pstMpool->taskSize) {
        pstTask->pstMem = mf_mem_pool_create("mftask", pstMpool->taskSize);
    }
}

static void
task_mem_uninit(struct mf_task *pstTask)
{
    if (pstTask->pstMem) {
        mf_mem_state(pstTask->pstMem, MF_YES);
        mf_mem_pool_destory(pstTask->pstMem);
    }
}

static inline void *
task_mem_alloc(MF_U32 size)
{
    void *p;

    if (g_stTask.pstMem) {
        p = mf_mem_alloc(g_stTask.pstMem, size);
    } else {
        p = ms_malloc(size);
    }

    return p;
}

static inline void *
task_mem_calloc(MF_U32 n, MF_U32 size)
{
    void *p;

    if (g_stTask.pstMem) {
        p = mf_mem_calloc(g_stTask.pstMem, n, size);
    } else {
        p = ms_calloc(n, size);
    }

    return p;
}


static inline void
task_mem_free(void *p)
{
    if (g_stTask.pstMem) {
        mf_mem_free(g_stTask.pstMem, p);
    } else {
        ms_free(p);
    }
}

static void
calendar_event_clear(struct calendor_node_t *node)
{
    struct seg_node_t *seg;
    struct seg_node_t *n;

    list_for_each_entry_safe(seg, n, &node->segList, node) {
        list_del(&seg->node);
        task_mem_free(seg);
    }
}

static struct calendor_node_t *
calendar_node_alloc()
{
    struct calendor_node_t *node;

    node = task_mem_calloc(1, sizeof(struct calendor_node_t));
    INIT_LIST_HEAD(&node->segList);
    node->startTime = INIT_START_TIME;
    node->endTime = INIT_END_TIME;

    return node;
}

static void
calendar_node_free(struct calendor_node_t *node)
{
    calendar_event_clear(node);
    task_mem_free(node);
}

static void
calendar_set_day_event(MF_U8 chnId, FILE_TYPE_EN enType,
                       REC_EVENT_EN enEvent,
                       MF_PTS startTime,
                       MF_PTS endTime)
{
    struct list_head *list;
    struct calendor_node_t *pstCal;
    MF_U32 startDay, endDay;
    MF_S32 i;

    if (startTime > endTime) {
        return;
    }

    if ((endTime - startTime) > MAX_TIME_SPAN) {
        return;
    }
    if (enEvent == REC_EVENT_NONE) {
        return;
    }

    list = &g_pstCalendar->astlist[chnId][enType];
    startDay = mf_time_second_to_day(startTime);
    endDay = mf_time_second_to_day(endTime);
#if 0
    printf("=====chn[%d]=type[%d]=====startday = %d, endDay = %d\n", chnId, enType, startDay, endDay);
    printf("starttime [%s]\n", mf_time_to_string(startTime));
    printf("endTime [%s]\n", mf_time_to_string(endTime));
#endif
    for (i = 0; i <= endDay - startDay; i++) {
        pstCal = NULL;
        CAL_FIND_BY_DAY(startDay + i, pstCal, list);
        if (pstCal) {
            if (pstCal->bUpdate == MF_NO) {
                pstCal->enEvent = REC_EVENT_NONE;
                calendar_event_clear(pstCal);
            }
            list_del(&pstCal->node);
        } else {
            pstCal = calendar_node_alloc();
        }

        struct seg_node_t *seg = task_mem_calloc(1, sizeof(struct seg_node_t));

        seg->enEvent = enEvent;
        seg->startTime = startTime;
        seg->endTime = endTime;
        list_add_tail(&seg->node, &pstCal->segList);

        pstCal->date = startDay + i;
        pstCal->startTime = MF_MIN(pstCal->startTime, startTime);
        pstCal->endTime = MF_MAX(pstCal->endTime, endTime);
        pstCal->enEvent |= enEvent;
        pstCal->bUpdate = MF_YES;
        LIST_INSERT_SORT_ASC(pstCal, list, date);
    }
}

static void
calendar_day_check(struct task_calendar_t *pstCal)
{
    struct calendor_node_t *cal;
    struct calendor_node_t *n;
    MF_S32 i, j;

    for (i = 0; i < MAX_REC_CHN_NUM; i++) {
        for (j = 0; j < FILE_TYPE_NUM; j++) {
            list_for_each_entry_safe(cal, n, &pstCal->astlist[i][j], node) {
                if (cal->bUpdate == MF_NO) {
                    list_del(&cal->node);
                    calendar_node_free(cal);
                } else {
                    cal->bUpdate = MF_NO;
                }
            }
        }
    }
}

static struct task_calendar_t *
task_calendar_init()
{
    struct task_calendar_t *pstCal;
    MF_S32 i, j;

    pstCal = task_mem_calloc(1, sizeof(struct task_calendar_t));
    for (i = 0; i < MAX_REC_CHN_NUM; i++) {
        for (j = 0; j < FILE_TYPE_NUM; j++) {
            INIT_LIST_HEAD(&pstCal->astlist[i][j]);
        }
    }
    ms_mutex_init(&pstCal->mutex);
    pstCal->bBusy = MF_NO;
    return pstCal;

}

static void
task_calendor_deinit(struct task_calendar_t *pstCal)
{
    struct calendor_node_t *cal;
    struct calendor_node_t *n;
    MF_S32 i, j;

    for (i = 0; i < MAX_REC_CHN_NUM; i++) {
        for (j = 0; j < FILE_TYPE_NUM; j++) {
            list_for_each_entry_safe(cal, n, &pstCal->astlist[i][j], node) {
                list_del(&cal->node);
                calendar_node_free(cal);
            }
        }
    }
    ms_mutex_uninit(&pstCal->mutex);
    task_mem_free(pstCal);
}

static void *
on_task_record_calendar(void *param)
{
    struct task_calendar_t *pstCal = g_pstCalendar;
    struct list_head *list;
    struct diskObj *disk;
    MF_BOOL tryLock = MF_YES;
    static MF_U32 s_times = 0;
    MF_PTS span;
    MF_U64 n = 1;
    if (pstCal->bBusy == MF_YES) {
        return MF_SUCCESS;
    }

    if (s_times == 10) {
        //10s * 10
        s_times = 0;
        MFprint("calendar get disk list cancel trylock");
        tryLock = MF_NO;
    }
    
    list = mf_disk_get_rd_list(tryLock);
    if (!list) {
        s_times++;
        return MF_SUCCESS;
    }
    s_times = 0;
    pstCal->bBusy = MF_YES;

    ms_mutex_lock(&pstCal->mutex);
    pstCal->ipcMask = 0;
    list_for_each_entry(disk, list, node) {
        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }

        if (!disk->ipcNum) {
            continue;
        }

        struct ipc_info *ipc;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            struct file_info *file;
            pstCal->ipcMask |= (n << ipc->chnId);

            ms_mutex_lock(&ipc->mutex);
            list_for_each_entry(file, &ipc->fileHead, node) {
                if (file->segNum < 0) {
                    break;
                }
                span = file->fileEndTime - file->fileStartTime;
                if (span < 0) {
                    break;
                }
                if (file->enLoad < FILE_UNLOAD) {
                    break;
                }
                if (file->enLoad == FILE_SEG_LOAD || span > MAX_TIME_SPAN) {
                    struct seg_info *seg;

                    if (file->enLoad == FILE_PRE_LOAD) {
                        if (mf_retr_seg_load(file) == MF_FAILURE) {
                            continue;
                        }
                        if (file->enLoad != FILE_SEG_LOAD) {
                            continue ;
                        }
                    }

                    list_for_each_entry(seg, &file->segHead, node) {
                        if (seg->recStartOffset == seg->recEndOffset) {
                            continue;
                        }
                        calendar_set_day_event(ipc->chnId,
                                               file->key.stRecord.type,
                                               seg->enEvent,
                                               seg->segStartTime,
                                               seg->segEndTime);
                    }
                } else {
                    calendar_set_day_event(ipc->chnId,
                                           file->key.stRecord.type,
                                           file->enEvent,
                                           file->fileStartTime,
                                           file->fileEndTime);
                }
            }
            ms_mutex_unlock(&ipc->mutex);
        }
    }

    calendar_day_check(pstCal);
    ms_mutex_unlock(&pstCal->mutex);

    mf_disk_put_rw_list(list);
    pstCal->bBusy = MF_NO;

    return MF_SUCCESS;
}

static void
on_task_record_unloading(MF_U32 timer)
{
    mf_retr_unload(timer);
}

static void
on_task_record_update(MF_U32 timer)
{
    struct list_head *list;
    MF_U8 chnId = (timer % (64 * 3)) / 3;

    list = mf_disk_get_wr_list(MF_NO);
    struct diskObj *disk, *p;
    list_for_each_entry_safe(disk, p, list, node) {
        if (list_empty(&disk->ipcHead)) {
            continue;
        }

        struct ipc_info *ipc, *n;
        list_for_each_entry_safe(ipc, n, &disk->ipcHead, node) {
            if (chnId != ipc->chnId || list_empty(&ipc->fileHead)) {
                continue;
            }

            struct file_info *file, *m;
            list_for_each_entry_safe(file, m, &ipc->fileHead, node) {
                if (file->key.stRecord.isLocked == 0xff) {
                    if (file->enLoad == FILE_PRE_LOAD) {
                        if (mf_retr_file_head_preload(file) == MF_FAILURE) {
                            continue;
                        }
                    }
                    msprintf("[wcm] mf_retr_update_index_lock begin");
                    mf_retr_update_index_lock(disk, file);
                    msprintf("[wcm] mf_retr_update_index_lock end");
                    mf_disk_put_rw_list(list);
                    return;
                }
            }
        }
    }

    mf_disk_put_rw_list(list);
}

static void *
task_schedule_pthread(void *param)
{
    struct mf_task *pstTask = param;
    int count = 0;
    ms_task_set_name("msfs_task_schedule");

    while (pstTask->bRun) {
        if (count % 10 == 0) {
            pstTask->timer++;
        }
        if (count % 100 == 0) {
            on_task_record_calendar(NULL);
        }
        if (count % 30 == 0) {
            on_task_record_unloading(pstTask->timer);
            on_task_record_update(pstTask->timer);
        }
        MF_USLEEP(100000);
        count++;
    }

    return MF_SUCCESS;
}

MF_U32
mf_task_timer(void)
{
    return g_stTask.timer;
}

struct task_calendar_t *
mf_task_calendar_get()
{
    ms_mutex_lock(&g_pstCalendar->mutex);
    return g_pstCalendar;
}

void
mf_task_calendar_put(struct task_calendar_t *pstCal)
{
    if (pstCal == g_pstCalendar) {
        ms_mutex_unlock(&pstCal->mutex);
    }
}

void
mf_task_calendar_dump()
{
    struct task_calendar_t *pstCal;
    struct calendor_node_t *cal;
    MF_U64 n = 1;
    MF_S32 i, j;

    pstCal = mf_task_calendar_get();
    MFinfo("\nmf_task_calendar_dump ipcMask[%#llx]", pstCal->ipcMask);
    for (i = 0; i < MAX_REC_CHN_NUM; i++) {
        if (!(pstCal->ipcMask & (n << i))) {
            continue;
        }

        MFinfo("chnId[%d]", i);
        for (j = 0; j < FILE_TYPE_NUM; j++) {
            MFinfo("\ttype[%d]", j);
            list_for_each_entry(cal, &pstCal->astlist[i][j], node) {
                MFinfo("\t\tdate[%d]", cal->date);
                MFinfo("\t\tevent[%#x]", cal->enEvent);
            }
        }
    }

    MFinfo("curren is day:%d\n", mf_time_second_to_day(time(0)));
    MFinfo("mf_task_calendar_dump end");
    mf_task_calendar_put(pstCal);

}

MF_S32
mf_task_init()
{
    memset(&g_stTask, 0, sizeof(g_stTask));

    task_mem_init(&g_stTask);

    g_pstCalendar = task_calendar_init();

    g_stTask.bRun = MF_YES;
    ms_task_create_join_stack_size(&g_stTask.handle, 16 * 1024, task_schedule_pthread, &g_stTask);

    return MF_SUCCESS;
}

MF_S32
mf_task_deinit()
{
    g_stTask.bRun = MF_NO;
    ms_task_join(&g_stTask.handle);
    task_calendor_deinit(g_pstCalendar);
    task_mem_uninit(&g_stTask);

    return MF_SUCCESS;
}

void
mf_task_dbg_show_mem_usage()
{
    if (g_stTask.pstMem) {
        MFinfo("task_dbg_show_mem_usage");
        mf_mem_state(g_stTask.pstMem, MF_YES);
    }
}

