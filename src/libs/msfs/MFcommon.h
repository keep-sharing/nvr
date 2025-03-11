/*
 * ***************************************************************
 * Filename:        MFcommon.h
 * Created at:      2017.05.10
 * Description:     msfs common api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFCOMMON_H__
#define __MFCOMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"
#include "msfs_notify.h"

typedef enum task_e {
    TASK_ID_COMM_EVENT = 0,
    TASK_ID_TEST,
    TASK_ID_RECORD_CALENDAR,
} TASK_E;

typedef enum module_e {
    MODULE_DISK = 1,
    MODULE_REC,
    MODULE_PB,
    MODULE_LOG,
} MODULE_E;

typedef enum sys_event {
    SE_DISK_LOAD_OK  = 0x1,
    SE_BOOT_COMPLETE = 0x2,
} SYS_EVENT;

typedef void *(*MF_FUNC)(void *);

typedef struct mf_event {
    MODULE_E enId;
    void (*func)(MODULE_E from, EVENT_E event, void *argv);
} mf_event;

typedef struct evt2task {
    MODULE_E    from;
    EVENT_E     enEvent;
    void        *argv;
} evt2task;

typedef struct mf_bar {
    BAR_FUNC func;
} mf_bar;

void
mf_comm_event_register(MODULE_E enId, void *func);
void
mf_comm_event_unregister(MODULE_E enId);
void
mf_comm_event_notify(MODULE_E module, EVENT_E event, void *argv);
void
mf_comm_task_notify(MODULE_E module, EVENT_E event, void *argv);
MF_S32
mf_comm_task_submit(void *(* work)(void *), void *argv);
MF_S32
mf_comm_task_dump(void);
void
mf_comm_mem_set(struct mem_pool_t *pstMpool);
struct mem_pool_t *
mf_comm_mem_get();
void
mf_comm_progress_bar_register(void *func);
void
mf_comm_progress_bar_unregister(void *func);
void
mf_comm_progress_bar_notify(PROGRESS_BAR_E enBar, void *argv);
void
mf_comm_debug_level(DBG_E enLevel);
void
mf_comm_debug_print(void *print);
MF_S32
mf_common_init();
MF_S32
mf_common_deinit();


#ifdef __cplusplus
}
#endif

#endif


