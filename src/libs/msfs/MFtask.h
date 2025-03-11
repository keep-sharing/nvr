/*
 * ***************************************************************
 * Filename:        MFTask.h
 * Created at:      2017.05.10
 * Description:     task api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFTASK_H__
#define __MFTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"
#include "MFrecord.h"

typedef struct seg_node_t {
    MF_PTS startTime;
    MF_PTS endTime;
    REC_EVENT_EN enEvent;
    struct list_head node;
} seg_node_t;

typedef struct calendor_node_t {
    MF_BOOL bUpdate;
    MF_U32  date;   //unit is day
    MF_PTS  startTime;
    MF_PTS  endTime;
    REC_EVENT_EN enEvent;
    struct list_head segList; //node :seg_node_t
    struct list_head node;
} calendor_node_t;

typedef struct task_calendar_t {
    MF_BOOL bBusy;
    MF_U64  ipcMask;
    struct list_head astlist[MAX_REC_CHN_NUM][FILE_TYPE_NUM];
    MF_MUTEX_T  mutex;
} task_calendar_t;

MF_U32
mf_task_timer(void);
struct task_calendar_t *
mf_task_calendar_get();
void
mf_task_calendar_put(task_calendar_t *pstCal);
void
mf_task_calendar_dump();
void
mf_task_dbg_show_mem_usage();
MF_S32
mf_task_init();
MF_S32
mf_task_deinit();

#ifdef __cplusplus
}
#endif

#endif



