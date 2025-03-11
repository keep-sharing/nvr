/*
 * ***************************************************************
 * Filename:        MFrecord.c
 * Created at:      2017.05.10
 * Description:     record API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


#include "MFdisk.h"
#include "MFindex.h"
#include "MFrecord.h"
#include "MFtask.h"
#include "MFlog.h"
#include "MFcommon.h"
#include "MFmemory.h"
#include "msstd.h"
#define isAIO       MF_YES

#define REC_INFO_SIZE (512*1024)
#define REC_PIC_SIZE  (3/2*1024*1024)
#define REC_MAX_CACHE_NUM   (100)
#define REC_MAX_PRE_SIZE   (3*1024*1024)
#define REC_MAX_PIC_SIZE   (2*1024*1024)
#define REC_MAX_PRE_TIME   (30) //s
#define REC_MAX_VID_DEAD_TIME   (120*24*60*60) //120day
#define REC_MAX_PIC_DEAD_TIME   (365*24*60*60) //365 day
#define REC_MAX_DELAY_TIME   (60) //60 S
#define REC_MAX_MEM_TIME   (60*60) //60 min
#define REC_INFO_UPDATE_TIME   (60) //60 S
#define REC_SYS_MIN_SIZE  (100*1024) // unit kb
#define REC_MIN_SYNC_SIZE  (512*1024)//512K

typedef struct bplusFile {
    MF_BOOL bValid;
    MF_BOOL bNewFile;
    MF_BOOL bContinued;
    MF_BOOL bNewInfo;
    MF_BOOL bMalloc;
    MF_S32  writeFd;
    MF_S32  dataOffset;
    MF_S32  infoOffset;
    MF_S8   *dataBuff;
    MF_S8   *infoBuff;
    MF_U32  *pCacheSize;
    MF_U32  dataSize;
    MF_U32  infoSize;
    MF_U32  syncSize;
    MF_U32  lastFrameTime;
    MF_PTS  lastInfoTime;
    MF_PTS  lastFileTime;
    MF_PTS  lastMemUsedTime;
    MUTEX_OBJECT     IOmutex;
    MF_COND_T        IOcond;
    MUTEX_OBJECT     CMDmutex;
    MF_COND_T        CMDcond;
    struct diskObj  *pstDisk;
    struct mf_file  stFileInfo;
    struct mf_segment   stSegInfo;
    struct file_info    *pstExInfo;
} bplusFile;

typedef enum PIP_EN {
    PIP_VIDEO = 0,
    PIP_PICTURE,
    PIP_OTHERS,
} PIP_EN;

typedef enum CACHE_EN {
    CACHE_TYPE_STREAM = (1 << 0),
    CACHE_TYPE_INFO = (1 << 1),
    CACHE_TYPE_STOP = (1 << 2),
    CACHE_TYPE_SYNC = (1 << 3)
} CACHE_EN;

typedef struct  nodeInfo {
    void           *rechdl;
    CACHE_EN       enCache;
    struct mf_data stData;
    struct mf_info stInfo;
    struct mf_frame stFrame;
} nodeInfo;

typedef struct recCache {
    PIP_EN enPip;
    void *owner;
    struct list_head node;
    struct nodeInfo info;
} recCache;

typedef struct recVid {
    MF_BOOL          bEnable;
    MF_BOOL          bUserStart;
    MF_BOOL          bRec;
    MF_BOOL          bAudio;
    MF_BOOL          bRestart;
    MF_U8            chnId;
    VID_EN           enVid;
    FILE_TYPE_EN     enType;
    FILE_STATE_EN    enState;
    REC_EVENT_EN     enEvent;
    MF_BOOL          bRun;
    MF_BOOL          bExit;
    MF_U32           uid;
    MF_U32           CacheSize;
    MF_U32           bandwidth;
    MF_U32           CacheNum;
    MF_U32           CacheFirstTime;
    MF_U32           CacheLastTime;
    MF_U32           DeadTime;
    MF_U32           PreTime;
    MF_U32           PreSize;
    TASK_HANDLE      pthread;
    MUTEX_OBJECT     cacheMutex;
    MUTEX_OBJECT     ctrlMutex;
    MF_COND_T        cond;
    void             *phdl;
    struct bplusFile *pstRecFile;
    struct list_head listCache;
} recVid;

typedef struct chnUsdMem {
    MF_U8            chnId;
    MF_S8            *dataBuff;
    MF_S8            *infoBuff;
    MF_U32           dataSize;
    MF_U32           infoSize;
    MF_PTS           lastMemUsedTime;
    FILE_TYPE_EN     enType;
    struct list_head node;          //list: recVid->listUsdMem
} chnUsdMem;

typedef struct picCtrl {
    void            *rechdl;
    INFO_MAJOR_EN   enMajorType;
    INFO_MINOR_EN   enMinorType;
    MF_S32         stream_from;
    STREAM_TYPE    stream_format;
    MF_PTS      pts;
    MF_S8 priData[64];
    MF_U8 priSize;
    struct list_head node;
} picCtrl;

typedef struct recPic {
    MF_U8            busy;
    MF_U8            chnId;
    MF_BOOL          bUserStart;
    MF_BOOL          bRec;
    MF_BOOL          bRestart;
    MF_BOOL          bRun;
    MF_BOOL          bExit;
    MF_U32           CacheSize;
    MF_U32           CacheNum;
    MF_U32           DeadTime;
    MF_U32           PreSize;
    TASK_HANDLE      pthread;
    MF_COND_T        cond;
    MUTEX_OBJECT     cacheMutex;
    MUTEX_OBJECT     ctrlMutex;
    struct bplusFile   *pstRecFile;
    struct list_head listCache;
    struct list_head listCtrl;
} recPic;

typedef struct mfrecorder {
    struct recVid  stRecVid[MAX_REC_CHN_NUM][VID_TYPE_NUM];
    struct recPic  stRecPic[MAX_REC_CHN_NUM];
    struct list_head HDLList;
    struct list_head PipList;
    struct list_head UsdMemList;
    struct mf_mem_t *pstMem;
    TASK_HANDLE      pipPthread;
    TASK_HANDLE      dropPthread;
    TASK_HANDLE      deadPthread;
    TASK_HANDLE      usdMemPthread;
    RWLOCK_BOJECT    listRWLock;
    RWLOCK_BOJECT    vidRWLock[MAX_REC_CHN_NUM];
    MUTEX_OBJECT     PipMutex;
    MUTEX_OBJECT     PicMutex;
    MUTEX_OBJECT     MemMutex;
    MF_BOOL          bDebug;
    MF_BOOL          bDropRun;
    MF_BOOL          bDeadRun;
    MF_BOOL          bPipRun;
    MF_BOOL          bMemRun;
    MF_BOOL          bReceive;
    REC_USER_CB user_cb;
    REC_JPG_CB jpg_cb;
    MF_BOOL  isInit;
    MF_U32   max_chn;
    MF_U32   sysMemFree;
    MF_U32   IOTotalSize;
    MF_U32   upgradeFlag;
    MF_U32   memCnt;
    MF_U32   memPlus;
    MF_U32   memMinu;
} mfrecorder;

#define HDL_FIND_BY_OBJ(obj, rechdl) \
    { \
        struct recHDL* pos = NULL; \
        struct recHDL* n = NULL; \
        ms_rwlock_rdlock(&g_stRecorder.listRWLock); \
        list_for_each_entry_safe(pos, n, &g_stRecorder.HDLList, node) \
        { \
            if (obj == pos->pstRecObj) \
            { \
                rechdl = pos; \
                break; \
            } \
        } \
        ms_rwlock_unlock(&g_stRecorder.listRWLock); \
    }

static struct mfrecorder g_stRecorder;

MF_S32
record_sync_disk(struct bplusFile *pstRecFile, MF_BOOL bImmediate);

static void
record_mem_init(struct mfrecorder *pstRec)
{
    struct mem_pool_t *pstMpool = mf_comm_mem_get();

    if (pstMpool->recSize) {
        pstRec->pstMem = mf_mem_pool_create("mfrecord", pstMpool->recSize);
    }
}

static void
record_mem_uninit(struct mfrecorder *pstRec)
{
    if (pstRec->pstMem) {
        mf_mem_state(pstRec->pstMem, MF_YES);
        mf_mem_pool_destory(pstRec->pstMem);
    }
}

static inline void *
record_mem_alloc(MF_U32 size)
{
    void *p;

    if (g_stRecorder.pstMem) {
        p = mf_mem_alloc(g_stRecorder.pstMem, size);
    } else {
        p = ms_malloc(size);
    }


    return p;
}

static inline void *
record_mem_calloc(MF_U32 n, MF_U32 size)
{
    void *p;

    if (g_stRecorder.pstMem) {
        p = mf_mem_calloc(g_stRecorder.pstMem, n, size);
    } else {
        p = ms_calloc(n, size);
    }

    return p;
}

static inline void
record_mem_free(void *p)
{
    if (g_stRecorder.pstMem) {
        mf_mem_free(g_stRecorder.pstMem, p);
    } else {
        ms_free(p);
    }
}

static void
record_user_notify(EVENT_E event, struct recHDL *rechdl, struct rec_res_t *pstRes)
{
    if (g_stRecorder.user_cb) {
        g_stRecorder.user_cb(event, rechdl->owner, rechdl->enHDL, pstRes);
    }
    MFinfo("[%s]\n", mf_record_ack(event));
}

static void
record_event_error(recHDL *rechdl)
{
    struct bplusFile *pstRecFile;
    MFdbg("record_event_error[%d] begin", rechdl->id);

    if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
        pstRecFile = ((struct recPic *)(rechdl->pstRecObj))->pstRecFile;
        pstRecFile->bValid = MF_NO;

    } else {
        pstRecFile = ((struct recVid *)(rechdl->pstRecObj))->pstRecFile;
        pstRecFile->bValid = MF_NO;
    }

    if (pstRecFile->pstDisk && pstRecFile->pstDisk->enState == DISK_STATE_NORMAL) {
        MFdbg("record_event_error[%d] doing", rechdl->id);
        pstRecFile->pstDisk->enState = DISK_STATE_BAD;
        mf_record_event_notify(MSFS_EVENT_DISK_ERR, pstRecFile->pstDisk, MF_NO);
    }
    MFdbg("record_event_error[%d] end", rechdl->id);
}

static void
record_event_bad_block(recHDL *rechdl)
{
    struct bplusFile *pstRecFile;

    if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
        pstRecFile = ((struct recPic *)(rechdl->pstRecObj))->pstRecFile;
    } else {
        pstRecFile = ((struct recVid *)(rechdl->pstRecObj))->pstRecFile;
    }

    MFdbg("have a bad block\n");
    mf_retr_file_delete(pstRecFile->pstExInfo, FILE_CONDITION_BAD);
    pstRecFile->bValid = MF_NO;
}

static void
record_event_full(recHDL *rechdl)
{
    if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
        //此处不解锁会造成死锁 PicMutex必须在listRWLock之前
        //ms_rwlock_unlock(&g_stRecorder.listRWLock);
        //mf_record_pic_stop(rechdl);//逻辑有问题：由record_pic_task触发的会因为没有发出信号而超时30s
        //ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    } else {
        mf_record_vid_hard_stop(rechdl, MF_NO);
    }
}

static MF_BOOL record_hdl_find(struct recHDL *rechdl)
{

    struct recHDL *pos = NULL;
    struct recHDL *n = NULL;

    list_for_each_entry_safe(pos, n, &g_stRecorder.HDLList, node) {
        if (rechdl == pos) {
            return MF_YES;
        }
    }
    return MF_NO;
}

static void
record_error(ERROR_E enErr, void *argv, struct recHDL *rechdl)
{
    if (rechdl == NULL) {
        MFdbg("rechdl is NULL");
        HDL_FIND_BY_OBJ(argv, rechdl);
        if (!rechdl) {
            return;
        }
    }
    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    if (record_hdl_find(rechdl) == MF_NO) {
        ms_rwlock_unlock(&g_stRecorder.listRWLock);
        return;
    }
    switch (enErr) {
        case DISK_ERR_DEV_BAD: {
            record_event_error(rechdl);
            record_user_notify(MSFS_EVENT_REC_FAIL, rechdl, NULL);
        }
        break;
        case DISK_ERR_BADBLOCK: {
            record_event_bad_block(rechdl);
        }
        break;
        case DISK_ERR_NO_SPACE: {
            record_event_full(rechdl);
            record_user_notify(MSFS_EVENT_REC_FAIL, rechdl, NULL);
        }
        break;
        case DISK_ERR_QUOTA_FULL: {
            struct rec_res_t res;
            res.majorType = REC_QTA_FULL;
            record_event_full(rechdl);
            record_user_notify(MSFS_EVENT_REC_FAIL, rechdl, &res);
        }
        break;
        case DISK_ERR_IO: {
            MFerr("record failed, DISK_ERR_IO\n");
            record_event_error(rechdl);
            record_user_notify(MSFS_EVENT_REC_FAIL, rechdl, NULL);
        }
        break;
        default : {
            MFerr("unknown record failed[%d] !\n", enErr);
            record_event_error(rechdl);
            MFdbg("record_user_notify[%d] begin", rechdl->id);
            record_user_notify(MSFS_EVENT_REC_FAIL, rechdl, NULL);
            MFdbg("record_user_notify[%d] end", rechdl->id);
        }
        break;
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);
}

static void
record_vid_get_type_state(struct recVid *pstRecVid)
{
    switch (pstRecVid->enVid) {
        case VID_TYPE_MAIN:
            pstRecVid->enType     = FILE_TYPE_MAIN;
            pstRecVid->enState    = FILE_STATE_USING;
            break;
        case VID_TYPE_SUB:
            pstRecVid->enType     = FILE_TYPE_SUB;
            pstRecVid->enState    = FILE_STATE_USING;
            break;
        case VID_TYPE_MAIN_ANR:
            pstRecVid->enType     = FILE_TYPE_MAIN;
            pstRecVid->enState    = FILE_STATE_ANR;
            break;
        case VID_TYPE_SUB_ANR:
            pstRecVid->enType     = FILE_TYPE_SUB;
            pstRecVid->enState    = FILE_STATE_ANR;
            break;
        default:
            MFerr("unknown enVid [%d]", pstRecVid->enVid);
            pstRecVid->enType     = FILE_TYPE_MAIN;
            pstRecVid->enState    = FILE_STATE_USING;
            break;
    }
}

static VID_EN
record_vid_parse_frame(struct mf_frame *pstFrame)
{
    VID_EN enVid = VID_TYPE_MAIN;

    switch (pstFrame->stream_format) {
        case STREAM_TYPE_MAINSTREAM:
            if (pstFrame->stream_from == SST_IPC_STREAM) {
                enVid = VID_TYPE_MAIN;
            } else if (pstFrame->stream_from == SST_IPC_ANR) {
                enVid = VID_TYPE_MAIN_ANR;
            } else {
                enVid = VID_TYPE_MAIN;
            }
            break;
        case STREAM_TYPE_SUBSTREAM:
            if (pstFrame->stream_from == SST_IPC_STREAM) {
                enVid = VID_TYPE_SUB;
            } else if (pstFrame->stream_from == SST_IPC_ANR) {
                enVid = VID_TYPE_SUB_ANR;
            } else {
                enVid = VID_TYPE_SUB;
            }
            break;
        default:
            MFerr("unknown type[%d]", pstFrame->stream_format);
            break;
    }

    return enVid;
}

static struct recCache *
lpr_cache_alloc(struct rec_lpr_t *pstLpr)
{
    struct recCache *cache = record_mem_calloc(1, sizeof(struct recCache));
    if (!cache) {
        return NULL;
    }

    struct info_lpr *info = &cache->info.stInfo.stLpr;

    cache->info.stData.owner = info;
    cache->info.enCache = CACHE_TYPE_STREAM | CACHE_TYPE_INFO;
    cache->info.stInfo.majorType = INFO_MAJOR_SMART;
    cache->info.stInfo.minorType = INFO_MINOR_LPR;
    cache->info.stInfo.dataTime = pstLpr->timeUs;

    memcpy(cache->info.stInfo.stLpr.Licence, pstLpr->Licence, MAX_LPR_NAME);
    cache->info.stInfo.stLpr.enEnctype = pstLpr->enEnctype;

    info->stMainPic.width = pstLpr->stMainPic.width;
    info->stMainPic.height = pstLpr->stMainPic.height;
    info->stMainPic.size = pstLpr->stMainPic.size;

    //data[0] big picture
    cache->info.stData.data[0] = record_mem_alloc(info->stMainPic.size);

    if (!cache->info.stData.data[0]) {
        record_mem_free(cache);
        return NULL;
    }
    memcpy(cache->info.stData.data[0], pstLpr->stMainPic.data, info->stMainPic.size);
    cache->info.stFrame.size += ALIGN_UP(info->stMainPic.size, 512);
    cache->info.stData.num++;

    info->stSubPic.width = pstLpr->stSubPic.width;
    info->stSubPic.height = pstLpr->stSubPic.height;
    info->stSubPic.size = pstLpr->stSubPic.size;

    //data[1] small picture
    cache->info.stData.data[1] = record_mem_alloc(info->stSubPic.size);
    if (!cache->info.stData.data[1]) {
        record_mem_free(cache->info.stData.data[0]);
        record_mem_free(cache);
        return NULL;
    }
    memcpy(cache->info.stData.data[1], pstLpr->stSubPic.data, info->stSubPic.size);
    cache->info.stFrame.size += ALIGN_UP(info->stSubPic.size, 512);
    cache->info.stData.num++;

    if (pstLpr->pstPrivate) {
        memcpy(info->Private, pstLpr->pstPrivate, pstLpr->priSize);
        info->priSize = pstLpr->priSize;
    }

    return cache;
}

static void
lpr_cache_free(struct recCache *cache)
{
    struct mf_data *pstData = &cache->info.stData;
    MF_U32 i;

    for (i = 0; i < pstData->num; i++) {
        record_mem_free(pstData->data[i]);
    }

    record_mem_free(cache);
}

static struct recCache *
face_cache_alloc(struct rec_face_t *pstFace)
{
    struct recCache *cache = record_mem_calloc(1, sizeof(struct recCache));
    if (!cache) {
        return NULL;
    }

    struct info_face *info = &cache->info.stInfo.stFace;
    MF_U8 i;

    cache->info.stData.owner = info;
    cache->info.enCache = CACHE_TYPE_STREAM | CACHE_TYPE_INFO;
    cache->info.stInfo.majorType = INFO_MAJOR_SMART;
    cache->info.stInfo.minorType = INFO_MINOR_FACE;
    cache->info.stInfo.dataTime = pstFace->timeUs;

    info->stMainPic.width = pstFace->stMainPic.width;
    info->stMainPic.height = pstFace->stMainPic.height;
    info->stMainPic.size = pstFace->stMainPic.size;


    //data[0] big picture
    cache->info.stData.data[0] = record_mem_alloc(info->stMainPic.size);

    if (!cache->info.stData.data[0]) {
        record_mem_free(cache);
        MFerr("malloc failed");
        return NULL;
    }
    memcpy(cache->info.stData.data[0], pstFace->stMainPic.data, info->stMainPic.size);
    cache->info.stFrame.size += ALIGN_UP(info->stMainPic.size, 512);
    cache->info.stData.num++;

    for (i = 0; i < pstFace->num; i++) {
        info->stSubPic[i].width = pstFace->stSubPic[i].width;
        info->stSubPic[i].height = pstFace->stSubPic[i].height;
        info->stSubPic[i].size = pstFace->stSubPic[i].size;

        //data[1~pstFace->num] small picture
        cache->info.stData.data[i + 1] = record_mem_alloc(info->stSubPic[i].size);
        if (!cache->info.stData.data[i + 1]) {
            MFerr("malloc failed");
            goto E_FAIL;
        }
        memcpy(cache->info.stData.data[i + 1], pstFace->stSubPic[i].data, info->stSubPic[i].size);
        cache->info.stFrame.size += ALIGN_UP(info->stSubPic[i].size, 512);
        cache->info.stData.num++;

        if (pstFace->pstPrivate[i]) {
            memcpy(info->Private[i], pstFace->pstPrivate[i], pstFace->priSize[i]);
            info->priSize[i] = pstFace->priSize[i];
        }
        info->faceNum++;
    }

    return cache;

E_FAIL:
    for (i = 0; i < cache->info.stData.num; i++) {
        record_mem_free(cache->info.stData.data[i]);
    }
    record_mem_free(cache);

    return NULL;
}

static void
face_cache_free(struct recCache *cache)
{
    lpr_cache_free(cache);
}

static struct recCache *
frame_cache_alloc(struct mf_frame *frame)
{
    struct recCache *cache = record_mem_calloc(1, sizeof(struct recCache));
    if (!cache) {
        return NULL;
    }

    memcpy(&cache->info.stFrame, frame, sizeof(struct mf_frame));
    if (frame->data) {
        cache->info.stFrame.data = record_mem_alloc(frame->size);
        if (!cache->info.stFrame.data) {
            record_mem_free(cache);
            return NULL;
        }
        memcpy(cache->info.stFrame.data, frame->data, frame->size);
    }

    return cache;
}

static void
frame_cache_free(struct recCache *cache)
{
    if (cache->info.stFrame.data) {
        record_mem_free(cache->info.stFrame.data);
    }
    record_mem_free(cache);
}

static void
record_cache_free(struct recCache *cache)
{
    if (!cache) {
        return;
    }

    struct mf_info *info = &cache->info.stInfo;

    if (info->majorType == INFO_MAJOR_SMART && info->minorType == INFO_MINOR_LPR) {
        lpr_cache_free(cache);
    } else if (info->majorType == INFO_MAJOR_SMART && info->minorType == INFO_MINOR_FACE) {
        face_cache_free(cache);
    } else {
        frame_cache_free(cache);
    }
}

static void
record_list_cache_push(struct recCache *cache)
{
    ms_mutex_lock(&g_stRecorder.PipMutex);
    list_add_tail(&cache->node, &g_stRecorder.PipList);
    ms_mutex_unlock(&g_stRecorder.PipMutex);
}

static struct recCache *
record_list_cache_pop()
{
    struct recCache *cache = NULL;

    ms_mutex_lock(&g_stRecorder.PipMutex);
    cache = list_first_entry_or_null(&g_stRecorder.PipList, struct recCache, node);
    if (cache) {
        list_del(&cache->node);
    }
    ms_mutex_unlock(&g_stRecorder.PipMutex);

    return cache;
}

static void
record_list_cache_clear()
{
    struct recCache *cache;

    for (;;) {
        cache = record_list_cache_pop();
        if (cache) {
            record_cache_free(cache);
        } else {
            break;
        }
    }
}

static void
record_chn_buffer_if_malloc(void *arg, FILE_TYPE_EN enType)
{
    MF_U32 max, min;
    MF_U8 chnId;
    MF_BOOL bFind = MF_NO;
    struct bplusFile *pstRecFile = NULL;

    if (enType >= FILE_TYPE_PIC) {
        struct recPic *pstRecPic = (struct recPic *)arg;
        pstRecFile = pstRecPic->pstRecFile;
        chnId = pstRecPic->chnId;
    } else {
        struct recVid *pstRecVid = (struct recVid *)arg;
        pstRecFile = pstRecVid->pstRecFile;
        chnId = pstRecVid->chnId;
    }

    pstRecFile->lastMemUsedTime = time(0);
    if (pstRecFile->bMalloc == MF_YES) {
        return;
    }

    struct list_head *list = &g_stRecorder.UsdMemList;
    ms_mutex_lock(&g_stRecorder.MemMutex);
    if (list != NULL && !list_empty(list)) {
        chnUsdMem *pos, *n;
        list_for_each_entry_safe(pos, n, list, node) {
            if (pos->chnId == chnId && pos->enType == enType) {
                max = MF_MAX(pos->dataSize, pstRecFile->dataSize);
                min = MF_MIN(pos->dataSize, pstRecFile->dataSize);
                if ((max - min) <= pstRecFile->dataSize / 4) {
                    pstRecFile->dataSize = pos->dataSize;
                    pstRecFile->dataBuff = pos->dataBuff;
                    pstRecFile->infoBuff = pos->infoBuff;
                    pstRecFile->infoSize = pos->infoSize;
                    bFind = MF_YES;
                } else {
                    ms_free(pos->dataBuff);
                    ms_free(pos->infoBuff);
                }
                list_del(&pos->node);
                record_mem_free(pos);
                g_stRecorder.memCnt--;
                g_stRecorder.memMinu++;
                msprintf("[hrz.debug] memcnt 000 cnt:%d plus:%d minu:%d\n", g_stRecorder.memCnt, g_stRecorder.memPlus, g_stRecorder.memMinu);
                break;
            }
        }
    }
    ms_mutex_unlock(&g_stRecorder.MemMutex);
    if (bFind == MF_NO) {
        msprintf("[wcm] malloc chnId = %d, dataBuff = %p, dataBuffSize = %d, infoBuff = %p, infoSize = %d", 
            chnId, pstRecFile->dataBuff,pstRecFile->dataSize, pstRecFile->infoBuff, pstRecFile->infoSize);
        pstRecFile->dataBuff = ms_valloc(pstRecFile->dataSize);
        pstRecFile->infoBuff = ms_valloc(pstRecFile->infoSize);
    }
    memset(pstRecFile->dataBuff, 0, pstRecFile->dataSize);
    memset(pstRecFile->infoBuff, 0, pstRecFile->infoSize);
    pstRecFile->dataOffset = 0;
    pstRecFile->infoOffset = pstRecFile->infoSize;
    pstRecFile->bMalloc = MF_YES;
}

static void
record_chn_buffer_if_free(void *arg, FILE_TYPE_EN enType, MF_BOOL bFree)
{
    chnUsdMem *mem = NULL;
    struct bplusFile *pstRecFile = NULL;
    struct list_head *list = &g_stRecorder.UsdMemList;
    MF_U8 chnId = -1;

    if (enType >= FILE_TYPE_PIC) {
        struct recPic *pstRecPic = (struct recPic *)arg;
        pstRecFile = pstRecPic->pstRecFile;
        chnId = pstRecPic->chnId;
    } else {
        struct recVid *pstRecVid = (struct recVid *)arg;
        pstRecFile = pstRecVid->pstRecFile;
        chnId = pstRecVid->chnId;
    }

    if (pstRecFile == NULL || pstRecFile->bMalloc == MF_NO) {
        return;
    }

    if (bFree == MF_YES) {
        ms_free(pstRecFile->dataBuff);
        ms_free(pstRecFile->infoBuff);
    } else {
        mem = (struct chnUsdMem *)record_mem_calloc(1, sizeof(struct chnUsdMem));
        mem->chnId    = chnId;
        mem->enType   = enType;
        mem->dataBuff = pstRecFile->dataBuff;
        mem->dataSize = pstRecFile->dataSize;
        mem->infoBuff = pstRecFile->infoBuff;
        mem->infoSize = pstRecFile->infoSize;
        mem->lastMemUsedTime = pstRecFile->lastMemUsedTime;
        ms_mutex_lock(&g_stRecorder.MemMutex);
        LIST_INSERT_SORT_ASC(mem, list, lastMemUsedTime);
        g_stRecorder.memCnt++;
        g_stRecorder.memPlus++;
        msprintf("[hrz.debug] memcnt 111 cnt:%d plus:%d minu:%d\n", g_stRecorder.memCnt, g_stRecorder.memPlus, g_stRecorder.memMinu);
        ms_mutex_unlock(&g_stRecorder.MemMutex);
    }
    pstRecFile->bMalloc = MF_NO;
}

static MF_S32
record_pic_cache_push(struct recPic *pstRecPic, struct recCache *cache)
{
    if (pstRecPic->busy == 0 || pstRecPic->bRec == MF_NO) {
        record_cache_free(cache);
        return MF_FAILURE;
    }

    ms_mutex_lock(&pstRecPic->cacheMutex);
    if ((pstRecPic->PreSize > REC_MAX_PIC_SIZE) && (cache->info.enCache & CACHE_TYPE_STREAM)) {
        MFerr("chn[%d] picture major[%x] minor[%d] overflow",
              pstRecPic->chnId,
              cache->info.stInfo.majorType,
              cache->info.stInfo.minorType);
        record_cache_free(cache);
        ms_mutex_unlock(&pstRecPic->cacheMutex);
        return MF_FAILURE;
    }
    list_add_tail(&cache->node, &pstRecPic->listCache);
    pstRecPic->CacheNum++;
    pstRecPic->PreSize += cache->info.stFrame.size;
    ms_mutex_unlock(&pstRecPic->cacheMutex);

    return MF_SUCCESS;
}

static struct recCache *
record_pic_cache_pop(struct recPic *pstRecPic)
{
    struct recCache *cache;

    ms_mutex_lock(&pstRecPic->cacheMutex);
    cache = list_first_entry_or_null(&pstRecPic->listCache, struct recCache, node);
    if (cache) {
        list_del(&cache->node);
        pstRecPic->CacheNum--;
        pstRecPic->PreSize -= cache->info.stFrame.size;
    }
    ms_mutex_unlock(&pstRecPic->cacheMutex);

    return cache;
}

static void
record_pic_cache_clear(struct recPic *pstRecPic)
{
    struct recCache *cache;

    for (;;) {
        cache = record_pic_cache_pop(pstRecPic);
        if (cache) {
            record_cache_free(cache);
        } else {
            break;
        }
    }
}

static void
record_pic_cmd_clear(struct recPic *pstRecPic, void *rechdl)
{
    struct picCtrl *pos = NULL;
    struct picCtrl *n = NULL;

    ms_mutex_lock(&pstRecPic->ctrlMutex);
    list_for_each_entry_safe(pos, n, &pstRecPic->listCtrl, node) {
        if (rechdl && rechdl != pos->rechdl) {
            continue;
        }
        list_del(&pos->node);
        record_mem_free(pos);
    }
    ms_mutex_unlock(&pstRecPic->ctrlMutex);
}

static MF_S32
record_pic_jpg_encoder(struct mf_frame *in, INFO_MAJOR_EN enMajorType, struct jpg_frame *out)
{
    MF_S32 res = MF_FAILURE;

    out->data = NULL;
    out->size = 0;

    if (g_stRecorder.jpg_cb) {
        res =  g_stRecorder.jpg_cb(in, enMajorType, &out->data, &out->size, &out->width, &out->height);
    }

    return res;
}

static MF_S32
record_pic_frame(struct recCache *cache, struct bplusFile *pstRecFile)
{

    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;
    struct mf_frame *pstFrame = &cache->info.stFrame;
    MF_U32 size = 0;
    MF_S32 res = MF_SUCCESS;

    cache->info.stInfo.dataOffset = pstSegment->recEndOffset;
    cache->info.stInfo.stPic.size = cache->info.stFrame.size;
    cache->info.stInfo.stPic.width  = cache->info.stFrame.width;
    cache->info.stInfo.stPic.height = cache->info.stFrame.height;
    cache->info.stInfo.stPic.encode = cache->info.stFrame.codec_type;
    size = ALIGN_UP(cache->info.stFrame.size, 512);

    if (pstRecFile->dataOffset + size > pstRecFile->dataSize) {
        res = record_sync_disk(pstRecFile, MF_NO);
        if (res == MF_NOTODO) {
            return MF_NOTODO;
        }
    }
    if (res != MF_SUCCESS || size > pstRecFile->dataSize) {
        MFerr("res[%d] jpg size[%d] must be less than [%d]", res, size, \
              pstRecFile->dataSize - pstRecFile->dataOffset);
        return MF_FAILURE;
    }
    if (pstRecFile->dataOffset + cache->info.stFrame.size > pstRecFile->dataSize) {
        msprintf("[wcm] pstRecFile->dataOffset(%u) + cache->info.stFrame.size(%u) > pstRecFile->dataSize(%u), memcpy out of boundary", 
            pstRecFile->dataOffset, cache->info.stFrame.size, pstRecFile->dataSize);
    }
    memcpy(pstRecFile->dataBuff + pstRecFile->dataOffset,
           cache->info.stFrame.data, cache->info.stFrame.size);
    pstRecFile->dataOffset += size;
    pstSegment->recEndOffset += size;
    pstSegment->recStartTime = MF_MIN(pstSegment->recStartTime,
                                      pstFrame->time_usec / 1000000);
    pstSegment->recEndTime = MF_MAX(pstSegment->recEndTime,
                                    pstFrame->time_usec / 1000000);
    pstSegment->infoTypes |= cache->info.stInfo.majorType;
    pstSegment->infoSubTypes |= cache->info.stInfo.minorType;

    return res;
}

static MF_S32
record_pic_lpr(struct recCache *cache, struct bplusFile *pstRecFile)
{
    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;
    struct info_lpr *lpr = &cache->info.stInfo.stLpr;
    MF_U32 size = 0;
    MF_S32 res = MF_SUCCESS;

    if (pstRecFile->dataOffset + cache->info.stFrame.size > pstRecFile->dataSize) {
        res = record_sync_disk(pstRecFile, MF_NO);
        if (res == MF_NOTODO) {
            return MF_NOTODO;
        }
    }
    if (res != MF_SUCCESS || cache->info.stFrame.size > pstRecFile->dataSize) {
        MFerr("res[%d] jpg size[%d] must be less than [%d]", res, cache->info.stFrame.size, \
              pstRecFile->dataSize - pstRecFile->dataOffset);
        return MF_FAILURE;
    }

    cache->info.stInfo.dataOffset = pstSegment->recEndOffset;

    // LPR big picture
    lpr->stMainPic.offset = pstSegment->recEndOffset;
    size = ALIGN_UP(lpr->stMainPic.size, 512);
    if (pstRecFile->dataOffset + lpr->stMainPic.size > pstRecFile->dataSize) {
        msprintf("[wcm] pstRecFile->dataOffset(%u) + lpr->stMainPic.size(%u) > pstRecFile->dataSize(%u), memcpy out of boundary", 
            pstRecFile->dataOffset, lpr->stMainPic.size, pstRecFile->dataSize);
    }
    memcpy(pstRecFile->dataBuff + pstRecFile->dataOffset,
           cache->info.stData.data[0], lpr->stMainPic.size);
    pstRecFile->dataOffset += size;
    pstSegment->recEndOffset += size;

    //LPR small picture
    lpr->stSubPic.offset = pstSegment->recEndOffset;
    size = ALIGN_UP(lpr->stSubPic.size, 512);
    if (pstRecFile->dataOffset + lpr->stSubPic.size > pstRecFile->dataSize) {
        msprintf("[wcm] pstRecFile->dataOffset(%u) + lpr->stSubPic.size(%u) > pstRecFile->dataSize(%u), memcpy out of boundary", 
            pstRecFile->dataOffset, lpr->stSubPic.size, pstRecFile->dataSize);
    }
    memcpy(pstRecFile->dataBuff + pstRecFile->dataOffset,
           cache->info.stData.data[1], lpr->stSubPic.size);
    pstRecFile->dataOffset += size;
    pstSegment->recEndOffset += size;

    pstSegment->recStartTime = MF_MIN(pstSegment->recStartTime,
                                      cache->info.stInfo.dataTime / 1000000);
    pstSegment->recEndTime = MF_MAX(pstSegment->recEndTime,
                                    cache->info.stInfo.dataTime / 1000000);
    pstSegment->infoTypes |= cache->info.stInfo.majorType;
    pstSegment->infoSubTypes |= cache->info.stInfo.minorType;

    MFinfo("chn[%d] Licence[%s]", pstRecFile->pstExInfo->key.stRecord.chnId,
           cache->info.stInfo.stLpr.Licence);
    return res;
}

static MF_S32
record_pic_face(struct recCache *cache, struct bplusFile *pstRecFile)
{
    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;
    struct info_face *face = &cache->info.stInfo.stFace;
    MF_U32 size = 0;
    MF_U8 i = 0;
    MF_S32 res = MF_SUCCESS;

    if (pstRecFile->dataOffset + cache->info.stFrame.size > pstRecFile->dataSize) {
        res = record_sync_disk(pstRecFile, MF_NO);
        if (res == MF_NOTODO) {
            return MF_NOTODO;
        }
    }
    if (res != MF_SUCCESS || cache->info.stFrame.size > pstRecFile->dataSize) {
        MFerr("res[%d] jpg size[%d] must be less than [%d]", res, cache->info.stFrame.size, \
              pstRecFile->dataSize - pstRecFile->dataOffset);
        return MF_FAILURE;
    }

    cache->info.stInfo.dataOffset = pstSegment->recEndOffset;

    // Face big picture
    face->stMainPic.offset = pstSegment->recEndOffset;
    size = ALIGN_UP(face->stMainPic.size, 512);
    if (pstRecFile->dataOffset + face->stMainPic.size > pstRecFile->dataSize) {
        msprintf("[wcm] pstRecFile->dataOffset(%u) + face->stMainPic.size(%u) > pstRecFile->dataSize(%u), memcpy out of boundary", 
            pstRecFile->dataOffset, face->stMainPic.size, pstRecFile->dataSize);
    }
    memcpy(pstRecFile->dataBuff + pstRecFile->dataOffset,
           cache->info.stData.data[0], face->stMainPic.size);
    pstRecFile->dataOffset += size;
    pstSegment->recEndOffset += size;

    //Face small picture
    for (i = 0; i < face->faceNum; i++) {
        face->stSubPic[i].offset = pstSegment->recEndOffset;
        size = ALIGN_UP(face->stSubPic[i].size, 512);
        if (pstRecFile->dataOffset + face->stSubPic[i].size > pstRecFile->dataSize) {
            msprintf("[wcm] pstRecFile->dataOffset(%u) + face->stSubPic[%d].size(%u) > pstRecFile->dataSize(%u), memcpy out of boundary", 
                pstRecFile->dataOffset, i, face->stSubPic[i].size, pstRecFile->dataSize);
        }
        memcpy(pstRecFile->dataBuff + pstRecFile->dataOffset,
               cache->info.stData.data[1 + i], face->stSubPic[i].size);
        pstRecFile->dataOffset += size;
        pstSegment->recEndOffset += size;
    }

    pstSegment->recStartTime = MF_MIN(pstSegment->recStartTime,
                                      cache->info.stInfo.dataTime / 1000000);
    pstSegment->recEndTime = MF_MAX(pstSegment->recEndTime,
                                    cache->info.stInfo.dataTime / 1000000);
    pstSegment->infoTypes |= cache->info.stInfo.majorType;
    pstSegment->infoSubTypes |= cache->info.stInfo.minorType;

    MFinfo("chn[%d] face nums[%d]", pstRecFile->pstExInfo->key.stRecord.chnId,
           cache->info.stInfo.stFace.faceNum);
    return res;
}

static MF_BOOL
record_vid_is_drop(struct recVid *pstRecVid)
{

    if (pstRecVid->bEnable == MF_NO) {
        return MF_NO;
    }

    if (pstRecVid->PreSize > REC_MAX_PRE_SIZE && (pstRecVid->bRec == MF_NO)) {
        return MF_YES;
    }

    if (pstRecVid->PreTime <= (pstRecVid->CacheLastTime - pstRecVid->CacheFirstTime)
        && (pstRecVid->bRec == MF_NO || mf_disk_is_ready() == MF_NO)) {
//        MFinfo("chn[%d] enVid[%d] pre time over [%u]",
//            pstRecVid->chnId, pstRecVid->enVid, pstRecVid->PreTime);
        return MF_YES;
    }

    return MF_NO;
}

static inline MF_BOOL
record_is_stream_cache(struct recCache *cache)
{
    if (cache->info.enCache & CACHE_TYPE_STREAM) {
        return MF_YES;
    } else {
        return MF_NO;
    }
}

static inline MF_BOOL
record_is_stop_cache(struct recCache *cache)
{
    if (cache->info.enCache & CACHE_TYPE_STOP) {
        return MF_YES;
    } else {
        return MF_NO;
    }
}
static inline MF_BOOL

record_is_sync_cache(struct recCache *cache)
{
    if (cache->info.enCache & CACHE_TYPE_SYNC) {
        return MF_YES;
    } else {
        return MF_NO;
    }
}

static inline MF_BOOL
record_is_audio(struct mf_frame *f)
{
    return f->strm_type == ST_AUDIO;
}

static inline MF_BOOL
record_is_video(struct mf_frame *f)
{
    return f->strm_type == ST_VIDEO;
}


static inline MF_BOOL
record_is_continuous_cache(struct bplusFile *pstRecFile, struct recCache *cache)
{
    MF_BOOL bContinuous = MF_YES;

    if (!record_is_stream_cache(cache)) {
        return MF_YES;
    }

    struct mf_frame *f = &cache->info.stFrame;
    MF_U32 tmpTime = (MF_U32)(f->time_usec / 1000000);

    if (pstRecFile->lastFrameTime == 0) {
        pstRecFile->lastFrameTime = tmpTime;
        return MF_YES;
    }

    if (tmpTime > pstRecFile->lastFrameTime + REC_MAX_DELAY_TIME
        || tmpTime < pstRecFile->lastFrameTime - REC_MAX_DELAY_TIME) {
        bContinuous = MF_NO;
    }

    pstRecFile->lastFrameTime = tmpTime;

    return bContinuous;
}

static void
reocrd_pic_notify(struct bplusFile *pstRecFile, struct recCache *cache, EVENT_E enEvt)
{
    struct rec_res_t stRes;

    stRes.majorType = cache->info.stInfo.majorType;
    stRes.minorType = cache->info.stInfo.minorType;
    stRes.size     = cache->info.stInfo.stPic.size;
    stRes.starTime = cache->info.stInfo.stPic.time;
    stRes.endTime  = cache->info.stInfo.stPic.time;
    if (pstRecFile->pstDisk) {
        stRes.port = pstRecFile->pstDisk->port;
    } else {
        stRes.port = 0;
    }

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    if (record_hdl_find(cache->info.rechdl) == MF_NO) {
        ms_rwlock_unlock(&g_stRecorder.listRWLock);
        return;
    }
    record_user_notify(enEvt, cache->info.rechdl, &stRes);
    ms_rwlock_unlock(&g_stRecorder.listRWLock);
}

static struct recCache *
record_vid_cache_catch(struct recVid *pstRecVid)
{
    struct recCache *cache;
    cache = list_first_entry_or_null(&pstRecVid->listCache, struct recCache, node);
    if (cache) {
        list_del(&cache->node);
        if (record_is_stream_cache(cache)) {
            pstRecVid->CacheFirstTime = cache->info.stFrame.time_usec / 1000000;
            pstRecVid->PreSize -= cache->info.stFrame.size;
        }
        pstRecVid->CacheNum--;
    }

    return cache;
}

static struct recCache *
record_vid_cache_pop(struct recVid *pstRecVid)
{
    struct recCache *cache;

    ms_mutex_lock(&pstRecVid->cacheMutex);
    cache = record_vid_cache_catch(pstRecVid);
    ms_mutex_unlock(&pstRecVid->cacheMutex);

    return cache;
}

static MF_S32
record_vid_cache_push(struct recVid *pstRecVid, struct recCache *cache)
{

    if (pstRecVid->bEnable == MF_NO) {
        record_cache_free(cache);
        return MF_FAILURE;
    }

    ms_mutex_lock(&pstRecVid->cacheMutex);
    list_add_tail(&cache->node, &pstRecVid->listCache);
    pstRecVid->CacheNum++;

    if (record_is_stream_cache(cache)) {
        pstRecVid->CacheLastTime = cache->info.stFrame.time_usec / 1000000;
        pstRecVid->PreSize += cache->info.stFrame.size;
        cache->info.stFrame.uid = pstRecVid->uid;
    }

    while (record_vid_is_drop(pstRecVid)) {
        struct recCache *dropCache;
        dropCache = record_vid_cache_catch(pstRecVid);
        if (dropCache == NULL) {
            break;
        }
        if (record_is_stop_cache(dropCache)) {
            pstRecVid->bRec = MF_NO;
            record_cache_free(dropCache);
        } else if (record_is_sync_cache(dropCache)) {
            list_add_tail(&dropCache->node, &pstRecVid->listCache);
            pstRecVid->CacheNum++;
        } else {
            record_cache_free(dropCache);
        }
    }
    ms_mutex_unlock(&pstRecVid->cacheMutex);

    return MF_SUCCESS;
}

static void
record_vid_cache_reclaim(struct recVid *pstRecVid, struct recCache *cache)
{
    ms_mutex_lock(&pstRecVid->cacheMutex);
    list_add(&cache->node, &pstRecVid->listCache);
    pstRecVid->PreSize += cache->info.stFrame.size;
    pstRecVid->CacheNum++;
    ms_mutex_unlock(&pstRecVid->cacheMutex);
}

static void
record_vid_cache_clear(struct recVid *pstRecVid)
{
    struct recCache *cache;

    for (;;) {
        cache = record_vid_cache_pop(pstRecVid);
        if (cache) {
            record_cache_free(cache);
        } else {
            break;
        }
    }
}

static inline MF_BOOL
record_is_iframe(struct mf_frame *f)
{
    if (f->strm_type != ST_VIDEO) {
        return MF_NO;
    }
    if (f->data == NULL) {
        return MF_NO;
    }
    if (f->frame_type == FT_IFRAME) {
        return MF_YES;
    }
    return MF_NO;
}

static void
record_requst_io_write(struct bplusFile *pstRecFile)
{
    return; //Fix Me;
    struct ioReq *pstIOReq = ms_calloc(1, sizeof(struct ioReq));

    pstIOReq->pMutex    = &pstRecFile->IOmutex;
    pstIOReq->pCond     = &pstRecFile->IOcond;
    pstIOReq->bDone     = MF_NO;
    pstIOReq->port      = pstRecFile->pstDisk->port;
    pstIOReq->fileOffset = pstRecFile->stFileInfo.fileOffset;
    mf_disk_io_request(pstIOReq);
}

static inline void
record_close_file(struct bplusFile *pstRecFile)
{
    if (pstRecFile->writeFd) {
        fsync(pstRecFile->writeFd);
        disk_close(pstRecFile->writeFd);
        pstRecFile->writeFd = 0;
    }
}

static MF_S32
record_reset_file(struct bplusFile *pstRecFile)
{

    struct mf_file *pstFileHeader = &pstRecFile->stFileInfo;
    struct MF_S8 *cache = ms_valloc(FILE_HEAD_MAX_SIZE);
    struct diskObj *pstDisk = pstRecFile->pstDisk;
    struct bplus_key *pstKey = &pstRecFile->pstExInfo->key;
    MF_S32 fd;
    MF_S32 res;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, pstKey->stRecord.fileNo, isAIO);
    pstFileHeader->chnId            = pstKey->stRecord.chnId;
    pstFileHeader->fileType         = pstKey->stRecord.type;
    pstFileHeader->fileState        = pstKey->stRecord.state;
    pstFileHeader->fileOffset       = pstKey->stRecord.fileOffset;
    pstFileHeader->fileNo           = pstKey->stRecord.fileNo;
    pstFileHeader->recStartOffset   = 0;
    pstFileHeader->recEndOffset     = 0;
    pstFileHeader->recStartTime     = INIT_START_TIME;
    pstFileHeader->recEndTime       = INIT_END_TIME;
    pstFileHeader->infoStartOffset  = INFO_START_OFFSET;
    pstFileHeader->infoEndOffset    = INFO_START_OFFSET;
    pstFileHeader->infoStartTime    = INIT_START_TIME;
    pstFileHeader->infoEndTime      = INIT_END_TIME;
    pstFileHeader->infoTypes        = INFO_MAJOR_INIT;
    pstFileHeader->infoSubTypes     = INFO_MINOR_INIT;
    pstFileHeader->recEvent         = REC_EVENT_NONE;
    pstFileHeader->magic            = FILE_MAGIC;
    pstFileHeader->tagNum           = 0;
    pstFileHeader->segNum           = 0;
    pstFileHeader->lockNum          = 0;
    memcpy(cache, pstFileHeader, sizeof(struct mf_file));
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate, fd, cache,
                                        FILE_HEAD_MAX_SIZE, pstFileHeader->fileOffset + FILE_HEAD_OFFSET);
    if (res < 0) {
        MFerr("disk_write failed");
    }
    pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
    ms_free(cache);

    ms_mutex_lock(&pstRecFile->pstDisk->mutex);
    pstRecFile->pstExInfo =  mf_retr_file_info_reset(pstRecFile->pstExInfo);
    ms_mutex_unlock(&pstRecFile->pstDisk->mutex);

    return MF_SUCCESS;
}

static void
record_release_file(struct bplusFile *pstRecFile)
{
    if (pstRecFile->pstExInfo) {
        pstRecFile->pstExInfo->pstSegUsing = NULL;
        pstRecFile->pstExInfo->bRecording = MF_NO;
        pstRecFile->pstExInfo = NULL;
    }
}

static MF_S32
record_fetch_file(MF_U8 chnId,
                  FILE_TYPE_EN enType,
                  FILE_STATE_EN enState,
                  struct bplusFile *pstRecFile)
{
    struct mf_qta *qta;
    struct list_head *list;
    struct diskObj *pstDisk = NULL;
    struct diskObj *n = NULL;
    struct file_info *pstFileInfo = NULL;
    MF_PTS  oldestTime  = INIT_START_TIME;
    MF_BOOL bResetFile = MF_NO;
    MF_S32 res = MF_SUCCESS;
    MF_U8 oldChnId = 0;
    MF_BOOL findDisk = MF_NO;

    pstRecFile->pstDisk = NULL;
    MFinfo("start fetch file chn[%d]file[%d]", chnId, enType);

    list = mf_disk_get_wr_list(MF_NO);
    mf_disk_qta_grp_info(NULL, &qta);
    if (qta->bEnable == MF_YES) {
        mf_disk_qta_chn_refresh(chnId, list);
    }

    record_release_file(pstRecFile);
    list_for_each_entry_safe(pstDisk, n, list, node) {
        MFdbg("[chn%d] port[%d] searching ...\n", chnId, pstDisk->port);
        if (pstDisk->tree == NULL) {
            MFdbg("pstDisk->tree == NULL");
            continue;
        }
        if (pstDisk->bRetr == MF_NO) {
            MFdbg("pstDisk->bRetr == MF_NO");
            continue;
        }
        if (mf_disk_can_write(pstDisk, chnId) == MF_NO) {
            MFdbg("mf_disk_can_write == MF_NO");
            continue;
        }

        findDisk = MF_YES;
        ms_mutex_lock(&pstDisk->mutex);
        //find using file
        pstFileInfo = mf_retr_disk_find_using_file(pstDisk, chnId, enType, enState);
        if (pstFileInfo) {
            MFlog(MF_INFO, "chn[%d][%d] find using file[%d][%s] in disk[%d]\n",
                  chnId, enType, pstFileInfo->key.stRecord.fileNo,
                  mf_time_to_string(pstFileInfo->fileEndTime), pstDisk->port);
            pstRecFile->pstDisk = pstDisk;
            pstRecFile->pstExInfo = pstFileInfo;
            pstRecFile->pstExInfo->bRecording = MF_YES;
            ms_mutex_unlock(&pstDisk->mutex);
            break;
        }
        //find empty file
        pstFileInfo = mf_retr_disk_find_empty_file(pstDisk, chnId, enType);
        if (pstFileInfo) {
            MFlog(MF_INFO, "chn[%d][%d] find empty file[%d] in disk[%d]\n",
                  chnId, enType, pstFileInfo->key.stRecord.fileNo, pstDisk->port);
            pstRecFile->pstDisk = pstDisk;
            pstRecFile->pstExInfo = pstFileInfo;
            pstRecFile->pstExInfo->bRecording = MF_YES;
            ms_mutex_unlock(&pstDisk->mutex);
            break;
        }
        //find used file that oldest: step loop
        if (qta->bEnable == MF_NO) {
            pstFileInfo = mf_retr_disk_find_used_file(pstDisk);
        } else {
            pstFileInfo = mf_retr_disk_find_used_file_qta(pstDisk, chnId, enType);
        }

        if (pstFileInfo) {
            MFinfo("mf_index_find_used [%d]\n", chnId);
            if (qta->bEnable == MF_YES && pstRecFile->pstExInfo != NULL) {
                if (pstRecFile->pstExInfo->key.stRecord.chnId != chnId
                    && pstFileInfo->key.stRecord.chnId == chnId) {
                    ms_mutex_unlock(&pstDisk->mutex);
                    continue;
                }
            }
            if (oldestTime > pstFileInfo->key.stRecord.startTime) {
                oldestTime = pstFileInfo->key.stRecord.startTime;
                pstRecFile->pstDisk = pstDisk;
                pstRecFile->pstExInfo = pstFileInfo;
            }
        }
        ms_mutex_unlock(&pstDisk->mutex);
    }

    if (pstRecFile->pstDisk == NULL) {
        mf_disk_put_rw_list(list);
        if (!findDisk) {
            //无可写的硬盘
            return DISK_ERR_NO_SPACE;
        }
        return qta->bEnable == MF_YES ? DISK_ERR_QUOTA_FULL : DISK_ERR_NO_SPACE;
    }

    ms_mutex_lock(&pstRecFile->pstDisk->mutex);

    if (pstRecFile->pstExInfo->key.stRecord.state == FILE_STATE_USED
        || pstRecFile->pstExInfo->key.stRecord.state == FILE_STATE_EMPTY) {
        if (pstRecFile->pstExInfo->key.stRecord.state == FILE_STATE_USED) {
            MF_S8 t0[32],  t1[32];
            oldChnId = pstRecFile->pstExInfo->key.stRecord.chnId;
            mf_time_to_string2(pstRecFile->pstExInfo->fileStartTime, t0);
            mf_time_to_string2(pstRecFile->pstExInfo->fileEndTime, t1);
            MFlog(MF_INFO, "chn[%d][%d] find used file[%d](chn[%d][%d] %s-%s) in disk[%d]\n",
                  chnId, enType, pstRecFile->pstExInfo->key.stRecord.fileNo,
                  pstRecFile->pstExInfo->key.stRecord.chnId,
                  pstRecFile->pstExInfo->key.stRecord.type,
                  t0, t1, pstRecFile->pstDisk->port);
        }
        if (mf_index_set_state_to_using(pstRecFile->pstDisk->tree, chnId,
                                        enType, enState, &pstRecFile->pstExInfo->key) == MF_YES) {
            pstRecFile->bNewFile = MF_YES;
            bResetFile = MF_YES;
            pstRecFile->pstExInfo->bRecording = MF_YES;
            MFinfo("[finder chnId[%d]type[%d]state[%d] start\n", chnId, enType, enState);
        } else {
            res = DISK_ERR_DEV_BAD;
            pstRecFile->bNewFile = MF_NO;
            MFerr("index_set_state_to_using failed");
        }
        mf_retr_file_info_delete(pstRecFile->pstExInfo);
        if (qta->bEnable == MF_YES && oldChnId != chnId) {
            mf_disk_qta_chn_refresh(oldChnId, list);
        }
    }
    MFinfo("diskName[%s] res[%d]\n", pstRecFile->pstDisk->name, res);
    ms_mutex_unlock(&pstRecFile->pstDisk->mutex);
    mf_disk_put_rw_list(list);

    if (bResetFile == MF_YES) {
        if (record_reset_file(pstRecFile) != MF_SUCCESS) {
            res = DISK_ERR_BADBLOCK;
        }
    }

    return res;
}

static MF_S32
record_update_file_info(struct bplusFile *pstRecFile, MF_PTS curTime)
{
    MF_S8 *cache = ms_valloc(MF_MAX(FILE_HEAD_MAX_SIZE, SEG_HEAD_SIZE));
    off64_t offset;
    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;
    struct mf_file *pstFile = &pstRecFile->stFileInfo;
    struct diskObj *pstDisk = pstRecFile->pstDisk;
    MF_S32 res;

    offset = pstFile->fileOffset + SEG_HEAD_OFFSET + (pstFile->segNum - 1) * SEG_HEAD_SIZE;
    //get segment staus from disk
    if (pstRecFile->pstExInfo->pstSegUsing->enState != SEG_STATE_INVALID) {
        pstSegment->status  =  pstRecFile->pstExInfo->pstSegUsing->enState;
    }
    //set segment info to disk
    memcpy(cache, pstSegment, sizeof(struct mf_segment));
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        pstRecFile->writeFd,
                                        cache,
                                        SEG_HEAD_SIZE,
                                        offset);
    if (res < 0) {
        ms_free(cache);
        MFerr("write segment info failed[%d]", res);
        return res;
    }

    offset = pstFile->fileOffset + FILE_HEAD_OFFSET;
    //set file info to disk
    pstFile->tagNum = pstRecFile->pstExInfo->tagNum;
    pstFile->lockNum = pstRecFile->pstExInfo->lockNum;
    memcpy(cache, pstFile, sizeof(struct mf_file));
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        pstRecFile->writeFd,
                                        cache,
                                        FILE_HEAD_MAX_SIZE,
                                        offset);
    if (res < 0) {
        ms_free(cache);
        MFerr("write file info failed[%d]", res);
        return res;
    }

    pstRecFile->pstExInfo->pstSegUsing->segStartTime        = pstSegment->recStartTime;
    pstRecFile->pstExInfo->pstSegUsing->segEndTime          = pstSegment->recEndTime;
    pstRecFile->pstExInfo->pstSegUsing->infoStartOffset     = pstSegment->infoStartOffset;
    pstRecFile->pstExInfo->pstSegUsing->infoEndOffset       = pstSegment->infoEndOffset;
    pstRecFile->pstExInfo->pstSegUsing->recStartOffset      = pstSegment->recStartOffset;
    pstRecFile->pstExInfo->pstSegUsing->recEndOffset        = pstSegment->recEndOffset;
    pstRecFile->pstExInfo->pstSegUsing->enState             = pstSegment->status;
    pstRecFile->pstExInfo->pstSegUsing->enEvent             = pstSegment->recEvent;
    pstRecFile->pstExInfo->pstSegUsing->infoCount           = pstSegment->infoCount;
    pstRecFile->pstExInfo->pstSegUsing->enMajor             = pstSegment->infoTypes;
    pstRecFile->pstExInfo->pstSegUsing->enMinor             = pstSegment->infoSubTypes;
    pstRecFile->pstExInfo->pstSegUsing->encodeType          = pstSegment->encodeType;
    pstRecFile->pstExInfo->pstSegUsing->resWidth            = pstSegment->resWidth;
    pstRecFile->pstExInfo->pstSegUsing->resHeight           = pstSegment->resHeight;
    pstRecFile->pstExInfo->pstSegUsing->firstFrameTime      = pstSegment->firstFrameTime;
    pstRecFile->pstExInfo->pstSegUsing->firstIframeTime     = pstSegment->firstIframeTime;
    pstRecFile->pstExInfo->pstSegUsing->firstIframeOffset   = pstSegment->firstIframeOffset;
    pstRecFile->pstExInfo->pstSegUsing->lastFrameTime       = pstSegment->lastFrameTime;
    pstRecFile->pstExInfo->pstSegUsing->lastIframeTime      = pstSegment->lastIframeTime;
    pstRecFile->pstExInfo->pstSegUsing->lastIframeOffset    = pstSegment->lastIframeOffset;

    mf_retr_file_info_update(pstRecFile->pstExInfo);
    ms_free(cache);
    pstRecFile->lastFileTime = curTime;
    pstRecFile->bNewFile = MF_NO;
//    MFdbg("Time for update");
    return MF_SUCCESS;
}

MF_S32
record_sync_disk(struct bplusFile *pstRecFile, MF_BOOL bImmediate)
{
    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;
    struct mf_file *pstFile = &pstRecFile->stFileInfo;
    struct diskObj *pstDisk = pstRecFile->pstDisk;
    MF_PTS curTime = 0;
    MF_S32 res = 0;
    MF_BOOL bUpdate = MF_NO;

    if (pstRecFile->bValid != MF_YES) {
        return MF_NOTODO;
    }

    if (!pstRecFile->dataOffset && pstRecFile->infoOffset == pstRecFile->infoSize) {
        return MF_SUCCESS;
    }

    if (pstDisk->enState != DISK_STATE_NORMAL) {
        return MF_FAILURE;
    }

    if (pstSegment->recStartOffset == pstSegment->recEndOffset) {
        pstSegment->recStartTime = pstSegment->infoStartTime;
        pstSegment->recEndTime   = pstSegment->infoEndTime;
    }

    //wait to write
    record_requst_io_write(pstRecFile);
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        pstRecFile->writeFd,
                                        pstRecFile->dataBuff, pstRecFile->dataOffset,
                                        pstFile->fileOffset + pstFile->recEndOffset);
    if (res < 0) {
        MFerr("pstRecFile->dataOffset[%d] [%d] [%lld]", pstRecFile->dataOffset, pstFile->recEndOffset, pstFile->fileOffset);
        MFerr("write data failed [%d]", res);
        return res;
    }

    pstFile->recEndOffset = pstSegment->recEndOffset;
    pstFile->recStartTime = MF_MIN(pstFile->recStartTime, pstSegment->recStartTime);
    pstFile->recEndTime = MF_MAX(pstFile->recEndTime, pstSegment->recEndTime);
    pstRecFile->dataOffset = 0;
    curTime = time(0);

    if (!bImmediate && (pstRecFile->lastInfoTime + REC_INFO_UPDATE_TIME > curTime)) {
        bUpdate = MF_NO;
    } else {
        bUpdate = MF_YES;
    }

    if (pstRecFile->infoOffset < INFO_DATA_MAX_SIZE || bUpdate == MF_YES) {
        res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                            pstRecFile->writeFd,
                                            pstRecFile->infoBuff + pstRecFile->infoOffset,
                                            pstRecFile->infoSize - pstRecFile->infoOffset,
                                            pstFile->fileOffset + pstSegment->infoEndOffset);
        if (res < 0) {
            MFerr("write info failed [%d]", res);
            return res;
        }

        pstFile->infoEndOffset = pstSegment->infoEndOffset;
        pstFile->infoStartTime = MF_MIN(pstFile->infoStartTime, pstSegment->infoStartTime);
        pstFile->infoEndTime = MF_MAX(pstFile->infoEndTime, pstSegment->infoEndTime);
        pstFile->recEvent |= pstSegment->recEvent;
        pstFile->infoTypes |= pstSegment->infoTypes;
        pstFile->infoSubTypes |= pstSegment->infoSubTypes;
        pstRecFile->infoOffset = pstRecFile->infoSize;
        pstRecFile->lastInfoTime = curTime;

        if (bImmediate || (pstRecFile->dataOffset == 0)) {
            return record_update_file_info(pstRecFile, curTime);
        }
    }

//    MFdbg("record_sync_disk file[%d]\n", pstFile->fileNo);
    return MF_SUCCESS;
}

static MF_S32
record_update_index_info(struct bplusFile *pstRecFile)
{
    struct bplus_data stData;
    MF_S32  res;

    pstRecFile->stFileInfo.fileState = FILE_STATE_USED;
    ms_mutex_lock(&pstRecFile->pstDisk->mutex);

    mf_index_key_delete(pstRecFile->pstDisk->tree, &pstRecFile->pstExInfo->key);
    stData.enType = TYPE_RECORD;
    stData.stRecord.fileOffset = pstRecFile->pstExInfo->key.stRecord.fileOffset;
    pstRecFile->pstExInfo->key.stRecord.state = FILE_STATE_USED;
    pstRecFile->pstExInfo->key.stRecord.startTime = pstRecFile->stFileInfo.recStartTime;
    pstRecFile->pstExInfo->key.stRecord.endTime = pstRecFile->stFileInfo.recEndTime;;
    pstRecFile->pstExInfo->key.stRecord.segNumLsb = pstRecFile->stFileInfo.segNum % 256;
    pstRecFile->pstExInfo->key.stRecord.segNumMsb = pstRecFile->stFileInfo.segNum / 256;
    pstRecFile->pstExInfo->key.stRecord.event |= pstRecFile->stFileInfo.recEvent;
    pstRecFile->pstExInfo->key.stRecord.infoType |= pstRecFile->stFileInfo.infoTypes;
    pstRecFile->pstExInfo->key.stRecord.infoType |= pstRecFile->stFileInfo.infoSubTypes;
    pstRecFile->pstExInfo->key.stRecord.isLocked = pstRecFile->stFileInfo.lockNum == 0 ? 0 : 1;
    res = mf_index_key_insert(pstRecFile->pstDisk->tree, &pstRecFile->pstExInfo->key, &stData);

    if (res == MF_SUCCESS) {
        MFinfo("chn[%d] index state using to used", pstRecFile->pstExInfo->key.stRecord.chnId);
        mf_index_key_backup(pstRecFile->pstDisk->tree);
        mf_retr_file_info_update(pstRecFile->pstExInfo);
    } else {
        MFwarm("chn[%d] index insert failed. [%d]", pstRecFile->stFileInfo.chnId, res);
        mf_index_key_recover(pstRecFile->pstDisk->tree);
    }

    ms_mutex_unlock(&pstRecFile->pstDisk->mutex);
    pstRecFile->bValid = MF_NO;
    record_release_file(pstRecFile);

    return res;
}

static void
record_segment_init(struct mf_segment *pstSegment)
{
    pstSegment->recStartTime = INIT_START_TIME;
    pstSegment->recEndTime = INIT_END_TIME;
    pstSegment->infoStartTime = INIT_START_TIME;
    pstSegment->infoEndTime = INIT_END_TIME;
    pstSegment->status = SEG_STATE_NORMAL;
    pstSegment->infoTypes = INFO_MAJOR_INIT;
    pstSegment->infoSubTypes = INFO_MINOR_INIT;
    pstSegment->infoCount = 0;
}

static inline MF_S32
record_fetch_segment(struct bplusFile *pstRecFile, REC_EVENT_EN enEvent)
{
    off64_t offset;
    struct diskObj *pstDisk = pstRecFile->pstDisk;
    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;
    struct mf_file *pstFile = &pstRecFile->stFileInfo;
    MF_S32 res;

    if (pstDisk == NULL) {
        return MF_FAILURE;
    }

    record_close_file(pstRecFile);

    pstFile->fileNo = pstRecFile->pstExInfo->key.stRecord.fileNo;
    if (pstRecFile->writeFd == 0) {
        pstRecFile->writeFd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, pstFile->fileNo, isAIO);
    }

    if (pstRecFile->bNewFile == MF_NO) {
        MF_S8 *cache = ms_valloc(FILE_HEAD_MAX_SIZE);
        offset = pstRecFile->pstExInfo->key.stRecord.fileOffset + FILE_HEAD_OFFSET;
        res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                           pstRecFile->writeFd,
                                           cache,
                                           FILE_HEAD_MAX_SIZE,
                                           offset);
        if (res < 0) {
            ms_free(cache);
            MFerr("read file failed [%d]", res);
            return res;
        }
        memcpy(pstFile, cache, sizeof(struct mf_file));
        ms_free(cache);
    }

    pstFile->fileOffset = pstRecFile->pstExInfo->key.stRecord.fileOffset;
    //init new segment info
    memset(pstSegment, 0, sizeof(struct mf_segment));
    record_segment_init(pstSegment);
    pstFile->recEndOffset = ALIGN_UP(pstFile->recEndOffset, 512);
    pstSegment->recEndOffset = pstSegment->recStartOffset = pstFile->recEndOffset;
    pstSegment->infoEndOffset = pstSegment->infoStartOffset = pstFile->infoEndOffset;
    pstSegment->recEvent = enEvent;
    pstSegment->id = pstFile->segNum;

    if (pstFile->segNum >= SEG_MAX_NUM) {
        MFerr("pstFile->segNum overflow\n");
        pstRecFile->stFileInfo.fileState = FILE_STATE_USED;
        record_sync_disk(pstRecFile, MF_YES);
        res = record_update_index_info(pstRecFile);
        pstRecFile->bValid = MF_NO;
        return res;
    } else {
        pstRecFile->pstExInfo->pstSegUsing = mf_retr_seg_info_new(pstRecFile->pstExInfo);
        if (pstRecFile->pstExInfo->pstSegUsing->id == -1) {
            pstRecFile->pstExInfo->pstSegUsing->id = pstSegment->id;
        }

        pstRecFile->pstExInfo->enLoad = FILE_SEG_LOAD;
        pstRecFile->pstExInfo->loadTime = mf_task_timer();
        pstFile->segNum++;
        pstRecFile->bValid = MF_YES;
        pstRecFile->dataOffset = 0;
        pstRecFile->infoOffset = pstRecFile->infoSize;
        return MF_SUCCESS;
    }

    pstRecFile->bValid = MF_NO;
    return MF_FAILURE;
}

//FILE *pfd = NULL;

static inline void
record_check_mark_time(struct mf_segment *seg, struct mf_info *pstInfo)
{
    /*修改系统时间，写入事件标签前，校准起始时间和结束时间*/
    MF_PTS *startTime = NULL;
    MF_PTS *endTime = NULL;
    
    switch (pstInfo->majorType) {
        case INFO_MAJOR_ALARMIN:
            startTime = &pstInfo->stAlarm.startTime;
            endTime = &pstInfo->stAlarm.endTime;
            break;
        case INFO_MAJOR_MOTION:
            startTime = &pstInfo->stMotion.startTime;
            endTime = &pstInfo->stMotion.endTime;
            break;
        case INFO_MAJOR_VCA:
            startTime = &pstInfo->stVca.startTime;
            endTime = &pstInfo->stVca.endTime;
            break;
        case INFO_MAJOR_SMART:
            startTime = &pstInfo->stSmart.startTime;
            endTime = &pstInfo->stSmart.endTime;
            break;
        case INFO_MAJOR_AUDIO_ALARM:
            startTime = &pstInfo->stAudioAlarm.startTime;
            endTime = &pstInfo->stAudioAlarm.endTime;
                break;
        default:
            break;
    }

    if (!startTime || !endTime) {
        return;
    }

    if (*startTime > seg->recEndTime || *startTime < seg->recStartTime) {
        *startTime = seg->recStartTime;
    }

    if (*endTime > seg->recEndTime || *endTime < seg->recStartTime) {
        *endTime = seg->recEndTime;
    }

    return;
}

static inline MF_S32
record_vid_cache_write(struct recCache *cache, struct bplusFile *pstRecFile, MF_U32 bandwidth)
{
    struct mf_frame *pstFrame = &cache->info.stFrame;
    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;
    struct ps_frame_s stPsFrame = {0};
    MF_S32 dataSize = 0;
    MF_U8  factor;
    MF_S32 res = MF_SUCCESS;

    if (pstRecFile->bValid == MF_NO) {
        return MF_NOTODO;
    }

    if (pstRecFile->bContinued == MF_NO) {
        if (pstRecFile->bNewInfo == MF_NO) {
            pstRecFile->dataOffset = 0;
            pstRecFile->infoOffset = pstRecFile->infoSize;
            record_segment_init(pstSegment);
            pstRecFile->bNewInfo = MF_YES;
        }

        if (record_is_stream_cache(cache) == MF_YES) {
            pstRecFile->bContinued = MF_YES;
        }
    }

    if (cache->info.enCache & CACHE_TYPE_STREAM) {
        if (record_is_iframe(pstFrame) == MF_YES) {
            if (pstSegment->infoEndOffset < pstSegment->recEndOffset +
                MF_MAX(bandwidth, pstRecFile->syncSize)) {
                pstRecFile->syncSize = mf_random(REC_MIN_SYNC_SIZE,  bandwidth / 3);
                pstRecFile->stFileInfo.fileState = FILE_STATE_USED;
                record_sync_disk(pstRecFile, MF_YES);
                record_update_index_info(pstRecFile);
                return MF_FILE_NOSPACE;
            }
        }
        if (ps_packet_encoder(pstFrame, &stPsFrame) == MF_FAILURE || stPsFrame.packet_size < 0) {
            MFwarm("chn[%d] ps cover failed!", pstFrame->ch);
            return MF_SUCCESS;
        }
        dataSize = stPsFrame.packet_size;
    }

    if (pstRecFile->pstDisk->enType == DISK_TYPE_NAS || pstRecFile->pstDisk->enType == DISK_TYPE_CIFS) {
        factor = 8;
    } else {
        factor = 1;
    }
    if ((pstRecFile->dataOffset + dataSize > pstRecFile->dataSize / factor)
        || (pstRecFile->infoOffset < INFO_DATA_MAX_SIZE)) {
        res = record_sync_disk(pstRecFile, MF_NO);
        if (res != MF_SUCCESS) {
            MFerr("record_sync_disk failed![%d]", res);
            pstRecFile->bContinued = MF_NO;
            ps_packet_encoder_free(&stPsFrame);
            if (res == -EIO) {
                res = DISK_ERR_IO;
            }
            return res;
        }
    }

    if (cache->info.enCache & CACHE_TYPE_STREAM) {
        if (record_is_iframe(pstFrame) == MF_YES) {
            cache->info.stInfo.majorType = INFO_MAJOR_IFRAME;
            cache->info.stInfo.minorType = INFO_MINOR_NONE;
            cache->info.stInfo.dataOffset = pstSegment->recEndOffset;
            cache->info.stInfo.dataTime = pstFrame->time_usec;
            cache->info.stInfo.stIframe.height = pstFrame->height;
            cache->info.stInfo.stIframe.width = pstFrame->width;
            cache->info.stInfo.stIframe.size = dataSize;
            cache->info.stInfo.stIframe.pts = pstFrame->time_usec;
            cache->info.stInfo.stIframe.encode = pstFrame->codec_type;
            cache->info.stInfo.stIframe.width = pstFrame->width;
            cache->info.stInfo.stIframe.height = pstFrame->height;
            pstRecFile->infoOffset -= INFO_DATA_MAX_SIZE;
            pstSegment->infoEndOffset -= INFO_DATA_MAX_SIZE;
            cache->info.stInfo.selfOffset = pstSegment->infoEndOffset;
            if (pstRecFile->infoOffset + sizeof(struct mf_info) > pstRecFile->infoSize) {
                msprintf("[wcm] pstRecFile->infoOffset(%u) + sizeof(struct mf_info)(%u) > pstRecFile->infoSize(%u), memcpy out of boundary", 
                    pstRecFile->infoOffset, sizeof(struct mf_info), pstRecFile->infoSize);
            }
            memcpy(pstRecFile->infoBuff + pstRecFile->infoOffset,
                   &cache->info.stInfo, sizeof(struct mf_info));
            pstSegment->infoCount++;
            if (pstFrame->strm_type == ST_VIDEO) {
                pstSegment->infoStartTime = MF_MIN(pstSegment->infoStartTime,
                                                   pstFrame->time_usec / 1000000);
                pstSegment->infoEndTime = MF_MAX(pstSegment->infoEndTime,
                                                 pstFrame->time_usec / 1000000);
            }
            if (!pstSegment->firstIframeTime) {
                pstSegment->firstIframeOffset = cache->info.stInfo.dataOffset;
                pstSegment->firstIframeTime = cache->info.stInfo.stIframe.pts;
                pstSegment->encodeType = cache->info.stInfo.stIframe.encode;
                pstSegment->resWidth = cache->info.stInfo.stIframe.width;
                pstSegment->resHeight = cache->info.stInfo.stIframe.height;
            }
            pstSegment->lastIframeOffset = cache->info.stInfo.dataOffset;
            pstSegment->lastIframeTime = cache->info.stInfo.stIframe.pts;
        }
        if (!pstSegment->firstFrameTime) {
            pstSegment->firstFrameTime = pstFrame->time_usec;
        }
        pstSegment->lastFrameTime = pstFrame->time_usec;

        if (pstRecFile->dataOffset + dataSize > pstRecFile->dataSize) {
            msprintf("[wcm] pstRecFile->dataOffset(%u) + dataSize(%u) > pstRecFile->dataSize(%u), memcpy out of boundary", 
                pstRecFile->dataOffset, dataSize, pstRecFile->dataSize);
        }
        memcpy(pstRecFile->dataBuff + pstRecFile->dataOffset,
               stPsFrame.data, dataSize);
        pstRecFile->dataOffset += dataSize;
        pstSegment->recEndOffset += dataSize;
        MF_PTS time_sec = pstFrame->time_usec / 1000000;
        if (pstSegment->recStartTime != INIT_START_TIME && time_sec < pstSegment->recStartTime) {
            MFerr("chn[%d] recStartTime overlap, time_sec[%d] < pstSegment->recStartTime[%d]", pstFrame->ch, time_sec, pstSegment->recStartTime);
        }
        if (pstSegment->recEndTime != INIT_START_TIME && time_sec < pstSegment->recEndTime) {
            MFerr("chn[%d] recEndTime overlap, time_sec[%d] < pstSegment->recEndTime[%d]", pstFrame->ch, time_sec, pstSegment->recEndTime);
        }
        pstSegment->recStartTime = MF_MIN(pstSegment->recStartTime, time_sec);
        pstSegment->recEndTime   = MF_MAX(pstSegment->recEndTime, time_sec);
    }

    if (cache->info.enCache & CACHE_TYPE_INFO) {
        pstRecFile->infoOffset -= INFO_DATA_MAX_SIZE;
        pstSegment->infoEndOffset -= INFO_DATA_MAX_SIZE;
        cache->info.stInfo.dataSize   = dataSize;
        cache->info.stInfo.dataOffset = pstSegment->recEndOffset - dataSize;
        cache->info.stInfo.selfOffset = pstSegment->infoEndOffset;
        record_check_mark_time(pstSegment, &cache->info.stInfo);
        if (pstRecFile->infoOffset + sizeof(struct mf_info) > pstRecFile->infoSize) {
            msprintf("[wcm] pstRecFile->infoOffset(%u) + sizeof(struct mf_info)(%u) > pstRecFile->infoSize(%u), memcpy out of boundary", 
                pstRecFile->infoOffset, sizeof(struct mf_info), pstRecFile->infoSize);
        }
        memcpy(pstRecFile->infoBuff + pstRecFile->infoOffset,
               &cache->info.stInfo, sizeof(struct mf_info));
        pstSegment->infoCount++;
        pstSegment->infoStartTime = MF_MIN(pstSegment->infoStartTime,
                                           cache->info.stInfo.dataTime / 1000000);
        pstSegment->infoEndTime = MF_MAX(pstSegment->infoEndTime,
                                         cache->info.stInfo.dataTime / 1000000);
        pstSegment->infoTypes |= cache->info.stInfo.majorType;
        pstSegment->infoSubTypes |= cache->info.stInfo.minorType;
    }

    if (pstSegment->infoEndOffset < pstSegment->recEndOffset + pstRecFile->syncSize) {
        pstRecFile->syncSize = mf_random(REC_MIN_SYNC_SIZE, bandwidth / 3);
        pstRecFile->stFileInfo.fileState = FILE_STATE_USED;
        record_sync_disk(pstRecFile, MF_YES);
        res = record_update_index_info(pstRecFile);
    } else if (pstSegment->infoStartOffset - pstSegment->infoEndOffset >= INFO_SEG_MAX_SIZE) {
        record_sync_disk(pstRecFile, MF_YES);
        pstRecFile->bValid = MF_NO;
        MFinfo("slice segmentation chn[%d]", pstRecFile->stFileInfo.chnId);
    }

    ps_packet_encoder_free(&stPsFrame);

    return res;
}

static inline MF_S32
record_pic_cache_write(struct recCache *cache, struct bplusFile *pstRecFile)
{
    struct mf_segment *pstSegment = &pstRecFile->stSegInfo;

    MF_BOOL bNotify = MF_NO;
    MF_S32 res = MF_SUCCESS;

    if (pstRecFile->bValid == MF_NO) {
        return MF_NOTODO;
    }

    if (cache->info.enCache & CACHE_TYPE_STREAM) {
        switch (cache->info.stInfo.majorType) {
            case INFO_MAJOR_SMART:
                if (cache->info.stInfo.minorType == INFO_MINOR_LPR) {
                    res = record_pic_lpr(cache, pstRecFile);
                } else if (cache->info.stInfo.minorType == INFO_MINOR_FACE) {
                    res = record_pic_face(cache, pstRecFile);
                }
                bNotify = MF_NO;
                break;
            //case  others :to do
            default:
                res = record_pic_frame(cache, pstRecFile);
                bNotify = MF_YES;
                break;
        }
        if (res == MF_NOTODO) {
            return MF_NOTODO;
        }
        if (res != MF_SUCCESS && bNotify == MF_YES) {
            reocrd_pic_notify(pstRecFile, cache, MSFS_EVENT_REC_FAIL);
            return MF_NOTODO;
        }
    }

    if (cache->info.enCache & CACHE_TYPE_INFO) {
        if (pstRecFile->infoOffset < INFO_DATA_MAX_SIZE) {
            record_sync_disk(pstRecFile, MF_NO);
        }
        pstRecFile->infoOffset -= INFO_DATA_MAX_SIZE;
        pstSegment->infoEndOffset -= INFO_DATA_MAX_SIZE;
        cache->info.stInfo.selfOffset = pstSegment->infoEndOffset;
        if (pstRecFile->infoOffset + sizeof(struct mf_info) > pstRecFile->infoSize) {
            msprintf("[wcm] pstRecFile->infoOffset(%u) + sizeof(struct mf_info)(%u) > pstRecFile->infoSize(%u), memcpy out of boundary", 
                pstRecFile->infoOffset, sizeof(struct mf_info), pstRecFile->infoSize);
        }
        memcpy(pstRecFile->infoBuff + pstRecFile->infoOffset,
               &cache->info.stInfo, sizeof(struct mf_info));
        pstSegment->infoCount++;
        pstSegment->infoStartTime = MF_MIN(pstSegment->infoStartTime,
                                           cache->info.stInfo.dataTime / 1000000);
        pstSegment->infoEndTime = MF_MAX(pstSegment->infoEndTime,
                                         cache->info.stInfo.dataTime / 1000000);
    }

    if (pstSegment->infoEndOffset < pstSegment->recEndOffset + pstRecFile->syncSize) {
        pstRecFile->syncSize = mf_random(REC_MIN_SYNC_SIZE, pstRecFile->dataSize);
        pstRecFile->stFileInfo.fileState = FILE_STATE_USED;
        record_sync_disk(pstRecFile, MF_YES);
        res = record_update_index_info(pstRecFile);
    } else if (pstSegment->infoStartOffset - pstSegment->infoEndOffset >= INFO_SEG_MAX_SIZE) {
        res = record_sync_disk(pstRecFile, MF_YES);
        pstRecFile->bValid = MF_NO;
        MFinfo("slice segmentation chn[%d]", pstRecFile->stFileInfo.chnId);
    }

    if (bNotify == MF_NO) {
        return res;
    }

    if (res == MF_SUCCESS) {
        reocrd_pic_notify(pstRecFile, cache, MSFS_EVENT_REC_SUCCEED);
    } else {
        reocrd_pic_notify(pstRecFile, cache, MSFS_EVENT_REC_FAIL);
    }

    return res;
}

static void *
record_pic_task(void *argv)
{
    struct recPic *pstRecPic = (struct recPic *)argv;
    struct recCache *cache = NULL;
    char taskName[64];
    MF_S32  res = MF_SUCCESS;
    MF_U32 n = 0;

    pstRecPic->pthread = pthread_self();
    sprintf(taskName, "pic_%d(%d,%d)",
            pstRecPic->chnId, FILE_TYPE_PIC, FILE_STATE_USING);
    ms_task_set_name(taskName);
    mf_set_task_policy(pstRecPic->pthread, THTEAD_SCHED_RR_PRI);
    pstRecPic->pstRecFile->lastMemUsedTime = time(0);
    pstRecPic->bExit = MF_NO;
    while (pstRecPic->bRun) {
        if (pstRecPic->bRec == MF_NO || pstRecPic->bRestart == MF_YES) {
            record_sync_disk(pstRecPic->pstRecFile, MF_YES);
            ms_mutex_lock(&pstRecPic->ctrlMutex);
            record_close_file(pstRecPic->pstRecFile);
            mf_cond_signal(&pstRecPic->cond);
            ms_mutex_unlock(&pstRecPic->ctrlMutex);
            pstRecPic->pstRecFile->bValid = MF_NO;
            record_release_file(pstRecPic->pstRecFile);
            if (pstRecPic->pstRecFile->lastMemUsedTime + REC_MAX_MEM_TIME < time(0)) {
                record_chn_buffer_if_free(pstRecPic, FILE_TYPE_PIC, MF_YES);
            }
            record_pic_cache_clear(pstRecPic);
            MF_USLEEP(50000);
            continue;
        }

        cache = record_pic_cache_pop(pstRecPic);
        if (cache) {
            if (record_is_stop_cache(cache)) {
                pstRecPic->bRec = MF_NO;
                record_cache_free(cache);
                continue;
            }
            record_chn_buffer_if_malloc(pstRecPic, FILE_TYPE_PIC);
            if (record_is_sync_cache(cache)) {
                MFdbg("record_is_sync_cache pstRecPic->chnId:%d",pstRecPic->chnId);
                record_sync_disk(pstRecPic->pstRecFile, MF_YES);
                record_cache_free(cache);
                ms_mutex_lock(&pstRecPic->pstRecFile->CMDmutex);
                mf_cond_signal(&pstRecPic->pstRecFile->CMDcond);
                ms_mutex_unlock(&pstRecPic->pstRecFile->CMDmutex);
                continue;
            }

            while (pstRecPic->bRec == MF_YES && pstRecPic->pstRecFile->bValid == MF_NO) {
                MFdbg("pstRecPic->bRec:%d,pstRecPic->pstRecFile->bValid:%d",pstRecPic->bRec,pstRecPic->pstRecFile->bValid);
                res = record_fetch_file(pstRecPic->chnId,
                                        FILE_TYPE_PIC,
                                        FILE_STATE_USING,
                                        pstRecPic->pstRecFile);
                if (res != MF_SUCCESS) {
                    MFerr("chn[%d]fetch file failed [%d]", pstRecPic->chnId, res);
                    if (res != DISK_ERR_NO_SPACE) {
                        record_error(res, pstRecPic, cache->info.rechdl);
                        if (res == DISK_ERR_BADBLOCK) {
                            MFdbg("bad block continue find");
                            continue;
                        }
                    } else {
                        record_close_file(pstRecPic->pstRecFile);
                        reocrd_pic_notify(pstRecPic->pstRecFile, cache, MSFS_EVENT_REC_FAIL);
                        //reocrd_pic_notify(pstRecPic->pstRecFile, cache, MSFS_EVENT_REC_FAIL);
                    }
                    break;
                } else {
                    res = record_fetch_segment(pstRecPic->pstRecFile,
                                               REC_EVENT_NONE);
                    MFdbg("record_fetch_segment++ pstRecPic->chnId:%d res =%d",pstRecPic->chnId,res);
                    if (res != MF_SUCCESS) {
                        MFerr("chn[%d]fetch segment failed [%d]", pstRecPic->chnId, res);
                        record_error(res, pstRecPic, cache->info.rechdl);
                    }
                }
            }
            res = record_pic_cache_write(cache, pstRecPic->pstRecFile);
            if (res != MF_SUCCESS && res != MF_NOTODO) {
                MFerr("chn[%d]pic_cache_write failed. [%d]", pstRecPic->chnId, res);
                if (res == -ENOSPC) {
                    record_update_index_info(pstRecPic->pstRecFile);
                    MFwarm("chn[%d] pic_cache_write no space", pstRecPic->chnId);
                } else {
                    record_error(res, pstRecPic, cache->info.rechdl);
                }
            }

            record_cache_free(cache);
            n = 0;
        } else {
            MF_USLEEP(50000);
            n++;
//            if (n % 100 == 0) //5s upate to disk
//                record_sync_disk(pstRecPic->pstRecFile, MF_YES);
        }
    }

    record_sync_disk(pstRecPic->pstRecFile, MF_YES);
    record_close_file(pstRecPic->pstRecFile);
    record_pic_cache_clear(pstRecPic);
    record_chn_buffer_if_free(pstRecPic, FILE_TYPE_PIC, MF_NO);
    pstRecPic->bExit = MF_YES;
    return NULL;
}

static void *
record_vid_task(void *argv)
{
    struct recVid *pstRecVid = (struct recVid *)argv;
    struct recCache *cache = NULL;
    char taskName[64];
    MF_S32  res = MF_SUCCESS;

    pstRecVid->pthread = pthread_self();
#if 1
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    if (pthread_setaffinity_np(pstRecVid->pthread, sizeof(mask), &mask) < 0) {
        MFerr("set thread affinity failed.");
    }
#endif
    sprintf(taskName, "vid_%d(%d,%d)",
            pstRecVid->chnId, pstRecVid->enType, pstRecVid->enState);
    ms_task_set_name(taskName);
    mf_set_task_policy(pstRecVid->pthread, THTEAD_SCHED_RR_PRI);

    if (!pstRecVid->enEvent) {
        MFlog(MF_ERR, "[wcm] record event unknow, chnid = %d", pstRecVid->chnId);
    }

    pstRecVid->bExit = MF_NO;
    pstRecVid->pstRecFile->lastMemUsedTime = time(0);
    pstRecVid->pstRecFile->lastFrameTime = 0;
    while (pstRecVid->bRun) {
        if (pstRecVid->bRec == MF_NO || pstRecVid->bRestart == MF_YES) {
            record_sync_disk(pstRecVid->pstRecFile, MF_YES);
            ms_mutex_lock(&pstRecVid->ctrlMutex);
            record_close_file(pstRecVid->pstRecFile);
            mf_cond_signal(&pstRecVid->cond);
            ms_mutex_unlock(&pstRecVid->ctrlMutex);
            pstRecVid->pstRecFile->bValid = MF_NO;
            pstRecVid->pstRecFile->bContinued = MF_NO;
            pstRecVid->pstRecFile->bNewInfo = MF_NO;
            pstRecVid->pstRecFile->lastFrameTime = 0;
            record_release_file(pstRecVid->pstRecFile);
            if (pstRecVid->pstRecFile->lastMemUsedTime + REC_MAX_MEM_TIME < time(0)) {
                record_chn_buffer_if_free(pstRecVid, pstRecVid->enType, MF_YES);
                pstRecVid->bRun = MF_NO;
                MFerr("chn[%d] stop record timeout.", pstRecVid->chnId);
                break;
            } else {
                MF_USLEEP(50000);
            }
            continue;
        }

        cache = record_vid_cache_pop(pstRecVid);
        if (cache) {
            record_chn_buffer_if_malloc(pstRecVid, pstRecVid->enType);
            if (record_is_stop_cache(cache)) {
                pstRecVid->bRec = MF_NO;
                record_cache_free(cache);
                continue;
            } else if (record_is_sync_cache(cache)) {
                record_sync_disk(pstRecVid->pstRecFile, MF_YES);
                record_cache_free(cache);
                ms_mutex_lock(&pstRecVid->pstRecFile->CMDmutex);
                mf_cond_signal(&pstRecVid->pstRecFile->CMDcond);
                ms_mutex_unlock(&pstRecVid->pstRecFile->CMDmutex);
                continue;
            }

            if (record_is_continuous_cache(pstRecVid->pstRecFile, cache) == MF_NO) {
                if (pstRecVid->pstRecFile->stSegInfo.firstIframeTime) {
                    record_sync_disk(pstRecVid->pstRecFile, MF_YES);
                }
                pstRecVid->pstRecFile->bValid = MF_NO;
                pstRecVid->pstRecFile->bContinued = MF_NO;
                pstRecVid->pstRecFile->bNewInfo = MF_NO;
                record_release_file(pstRecVid->pstRecFile);

                MFerr("record[%d] is no continuous!!!\n", pstRecVid->chnId);
            }

            while (pstRecVid->bRec == MF_YES && pstRecVid->pstRecFile->bValid == MF_NO) {
                res = record_fetch_file(pstRecVid->chnId,
                                        pstRecVid->enType,
                                        pstRecVid->enState,
                                        pstRecVid->pstRecFile);
                if (res != MF_SUCCESS) {
                    MFerr("chn[%d]fetch file failed [%d]", pstRecVid->chnId, res);
                    record_error(res, pstRecVid, pstRecVid->phdl);
                    if (res == DISK_ERR_BADBLOCK) {
                        MFdbg("bad block continue find");
                        continue;
                    }
                    break;
                } else {
                    res = record_fetch_segment(pstRecVid->pstRecFile,
                                               pstRecVid->enEvent);
                    if (res != MF_SUCCESS) {
                        MFerr("chn[%d]fetch segment failed [%d]", pstRecVid->chnId, res);
                        record_error(res, pstRecVid, pstRecVid->phdl);
                    } else {
                        record_user_notify(MSFS_EVENT_REC_START, pstRecVid->phdl, NULL);
                    }
                }
            }
            res = record_vid_cache_write(cache, pstRecVid->pstRecFile, pstRecVid->bandwidth);
            if (res != MF_SUCCESS && res != MF_NOTODO) {
                if (res == -ENOSPC) {
                    record_update_index_info(pstRecVid->pstRecFile);
                    MFwarm("chn[%d] vid_cache_write no space", pstRecVid->chnId);
                } else if (res == MF_FILE_NOSPACE) {
                    record_vid_cache_reclaim(pstRecVid, cache);
                    MFdbg("chn[%d]vid_cache_write failed, file end. [%d]", pstRecVid->chnId, res);
                    continue;
                } else {
                    MFerr("chn[%d]vid_cache_write failed. [%d]", pstRecVid->chnId, res);
                    record_error(res, pstRecVid, pstRecVid->phdl);
                }
            }

            record_cache_free(cache);
        } else {
            MF_USLEEP(50000);
        }
    }
    record_sync_disk(pstRecVid->pstRecFile, MF_YES);
    record_close_file(pstRecVid->pstRecFile);
    record_vid_cache_clear(pstRecVid);
    record_chn_buffer_if_free(pstRecVid, pstRecVid->enType, MF_NO);
    pstRecVid->bExit = MF_YES;

    return NULL;
}

static MF_BOOL
record_vid_can_write(MF_U8 chnId, VID_EN enVid,
                     struct mf_frame *pstFrame)
{
    static MF_BOOL bOverflow;

    if (chnId >= MAX_REC_CHN_NUM || enVid >= VID_TYPE_NUM) {
        MFerr("param invalid ! chnId[%d]enVid[%d]", chnId, enVid);
        return MF_FAILURE;
    }

    struct recVid *pstRecVid = &g_stRecorder.stRecVid[chnId][enVid];

    pstRecVid->uid++;
#if !(isAIO)
#if !defined(_HI3536C_) && !defined(_NT98323_)
    if (g_stRecorder.sysMemFree < REC_SYS_MIN_SIZE / 2) {
        if (bOverflow == MF_NO) {
            MFerr("sysMem is too small\n");
            bOverflow = MF_YES;
        }
        return MF_FAILURE;
    }
#endif
#endif
    if (pstRecVid->bEnable == MF_NO) {
        return MF_FAILURE;
    }

    if (pstRecVid->PreTime == 0 && pstRecVid->bUserStart == MF_NO) {
        record_vid_cache_clear(pstRecVid);
        return MF_FAILURE;
    }

    if (pstRecVid->PreSize > MF_MIN(pstRecVid->CacheSize, REC_MAX_PRE_SIZE)) {
        if (pstRecVid->bRec) {
            if (g_stRecorder.bDebug == MF_YES) {
                printf("chn[%d] enVid[%d] pre size over [%u]\n", pstRecVid->chnId,
                       pstRecVid->enVid, MF_MIN(pstRecVid->CacheSize, REC_MAX_PRE_SIZE));
            }
            return MF_FAILURE;
        }
    }

    if (record_is_audio(pstFrame)  == MF_YES && pstRecVid->bAudio == MF_NO) {
        return MF_FAILURE;
    }

    bOverflow = MF_NO;
    return MF_SUCCESS;
}


static MF_S32
record_vid_process(MF_U8 chnId, VID_EN enVid,
                   struct mf_frame *pstFrame)
{
    struct recCache *cache;
    struct recVid *pstRecVid;

    if (pstFrame->strm_type == ST_DATA) {   //see mf_record_vid_smart
        return MF_FAILURE;
    }

    if (record_vid_can_write(chnId, enVid, pstFrame) == MF_FAILURE) {
        return MF_FAILURE;
    }

    pstRecVid = &g_stRecorder.stRecVid[chnId][enVid];
    cache = frame_cache_alloc(pstFrame);
    if (cache) {
        cache->owner = pstRecVid;
        cache->enPip = PIP_VIDEO;
        cache->info.enCache = CACHE_TYPE_STREAM;
        cache->info.stFrame.uid = pstRecVid->uid;
        record_list_cache_push(cache);
    }

    return MF_SUCCESS;
}

static MF_S32
record_pic_process(MF_U8 chnId, struct mf_frame *pstFrame)
{
    if (chnId >= MAX_REC_CHN_NUM) {
        MFerr("param invalid ! chnId[%d]", chnId);
        return MF_FAILURE;
    }

    struct recPic *pstRecPic =
            &g_stRecorder.stRecPic[chnId];

    if (pstRecPic->busy == 0) {
        return MF_FAILURE;
    }

    if (pstFrame->frame_type != FT_IFRAME) {
        return MF_FAILURE;
    }

    if (pstFrame->strm_type != ST_VIDEO) {
        return MF_FAILURE;
    }

    struct picCtrl *pos = NULL;
    struct picCtrl *n = NULL;
    INFO_MAJOR_EN  enMajorType = INFO_MAJOR_NONE;
    INFO_MINOR_EN  enMinorType = INFO_MINOR_NONE;

    ms_mutex_lock(&pstRecPic->ctrlMutex);
    list_for_each_entry_safe(pos, n, &pstRecPic->listCtrl, node) {
        if (pos->stream_from != pstFrame->stream_from) {
            continue;
        }

        if (pos->stream_format != pstFrame->stream_format) {
            continue;
        }

        if (abs(pstFrame->time_usec / 1000000 - pos->pts) > 10) { //over 10s discard !
#if 0 //split playback something wrong!
            MFinfo("chn[%d] pic time out[10s] discard", pstRecPic->chnId);
            stRes.majorType = pos->enMajorType;
            stRes.minorType = pos->enMinorType;
            stRes.size      = 0;
            stRes.port      = 0;
            stRes.starTime  = pos->pts;
            stRes.endTime   = stRes.starTime;
            record_user_notify(MSFS_EVENT_REC_FAIL, pos->rechdl, &stRes);
            list_del(&pos->node);
            record_mem_free(pos);
#endif
            continue;
        }

        if (enMajorType == pos->enMajorType && enMinorType == pos->enMinorType) {
            list_del(&pos->node);
            record_mem_free(pos);
            MFinfo("chn[%d] pic repeat[%d] discard", pstRecPic->chnId, REC_MAX_PIC_SIZE);
            continue;
        }
#if defined (_HI3798_) || defined (_HI3536C_)
        if (pstFrame->width * pstFrame->height > 3840 * 2160) {
            list_del(&pos->node);
            record_mem_free(pos);
            MFinfo("3798&353c snapshot NO support resolution more than 8M");
            continue;
        }
#endif
        if (pstRecPic->PreSize > REC_MAX_PIC_SIZE) {
            if (pos->enMajorType == INFO_MAJOR_TIME_PIC) {
                list_del(&pos->node);
                record_mem_free(pos);
                printf("chn[%d] pic over[%d] discard\n", pstRecPic->chnId, REC_MAX_PIC_SIZE);
                continue;
            }
        }

        MFinfo("chn[%d] pic send to encoder", pstRecPic->chnId);
        mf_record_pic_frame(pos->rechdl, pos->enMajorType, pos->enMinorType, pstFrame, pos->priData, pos->priSize);

        enMajorType = pos->enMajorType;
        enMinorType = pos->enMinorType;
        list_del(&pos->node);
        record_mem_free(pos);
    }
    ms_mutex_unlock(&pstRecPic->ctrlMutex);
    return MF_SUCCESS;
}

struct recHDL *
record_hdl_add(MF_U8 chnId, HDL_EN enHDL, void *obj)
{
    struct recHDL *rechdl = record_mem_calloc(1, sizeof(struct recHDL));

    rechdl->enHDL = enHDL;
    rechdl->pstRecObj = obj;
    rechdl->id = chnId;
    MFinfo("chnid = [%d] enHdl[%d] obj[%p]", chnId, enHDL, obj);
    ms_rwlock_wrlock(&g_stRecorder.listRWLock);
    LIST_INSERT_SORT_ASC(rechdl, &g_stRecorder.HDLList, id);
    ms_rwlock_unlock(&g_stRecorder.listRWLock);

    return rechdl;
}

static void
record_hdl_del(struct recHDL *rechdl)
{
    ms_rwlock_wrlock(&g_stRecorder.listRWLock);
    list_del(&rechdl->node);
    ms_rwlock_unlock(&g_stRecorder.listRWLock);

    record_mem_free(rechdl);
}

static void
record_pic_restart(struct recHDL *rechdl, struct recPic *pstRecPic)
{
    if (pstRecPic->bUserStart == MF_NO) {
        return;
    }
    if (mf_disk_is_ready() == MF_NO) {
        return;
    }

    MFdbg("chn[%d]record_pic_restart begin\n", pstRecPic->chnId);
    ms_mutex_lock(&pstRecPic->ctrlMutex);
    pstRecPic->bRec = MF_NO;
    pstRecPic->bRestart = MF_YES;
    pstRecPic->pstRecFile->bValid = MF_NO;
    mf_cond_timedwait(&pstRecPic->cond, &pstRecPic->ctrlMutex, 30000000);
    record_close_file(pstRecPic->pstRecFile);
    pstRecPic->bRec = MF_YES;
    pstRecPic->bRestart = MF_NO;
    ms_mutex_unlock(&pstRecPic->ctrlMutex);
    MFdbg("chn[%d]record_pic_restart end\n", pstRecPic->chnId);
}

static void
record_vid_restart(struct recHDL *rechdl, struct recVid *pstRecVid)
{
    if (pstRecVid->bUserStart == MF_NO) {
        return;
    }
    if (mf_disk_is_ready() == MF_NO) {
        return;
    }

    if (pstRecVid->bRestart == MF_YES) {
        return;
    }

    if (pstRecVid->bRun == MF_NO) {
        pstRecVid->bRun = MF_YES;
        printf("====record_vid_restart=[%d]======\n", pstRecVid->chnId);
        mf_comm_task_submit(record_vid_task, pstRecVid);
    }

    MFdbg("chn[%d]enVid[%d]record_vid_restart begin\n", pstRecVid->chnId, pstRecVid->enVid);
    ms_mutex_lock(&pstRecVid->ctrlMutex);
    MFdbg("chn[%d]enVid[%d]record_vid_restart doing...\n", pstRecVid->chnId, pstRecVid->enVid);
    pstRecVid->bRec = MF_NO;
    pstRecVid->bRestart = MF_YES;
    pstRecVid->pstRecFile->bValid = MF_NO;
    mf_cond_timedwait(&pstRecVid->cond, &pstRecVid->ctrlMutex, 30000000);
    record_close_file(pstRecVid->pstRecFile);
    pstRecVid->bRec = MF_YES;
    pstRecVid->bRestart = MF_NO;
    ms_mutex_unlock(&pstRecVid->ctrlMutex);
    MFdbg("chn[%d]enVid[%d]record_vid_restart end\n", pstRecVid->chnId, pstRecVid->enVid);
}

static MF_S32
record_vid_event_parse(struct mf_info  *pstInfo, struct rec_event_t *evt)
{
    switch (evt->enMajor) {
        case INFO_MAJOR_ALARMIN:
            pstInfo->stAlarm.startTime  = evt->startTime;
            pstInfo->stAlarm.endTime    = evt->endTime;
            pstInfo->stAlarm.priSize    = evt->priSize;
            if (evt->priSize) {
                memcpy(pstInfo->stAlarm.Private, evt->pstPrivate, evt->priSize);
            }
            break;
        case INFO_MAJOR_MOTION:
            pstInfo->stMotion.startTime = evt->startTime;
            pstInfo->stMotion.endTime   = evt->endTime;
            pstInfo->stMotion.priSize   = evt->priSize;
            if (evt->priSize) {
                memcpy(pstInfo->stMotion.Private, evt->pstPrivate, evt->priSize);
            }
            break;
        case INFO_MAJOR_VCA:
            pstInfo->stVca.startTime    = evt->startTime;
            pstInfo->stVca.endTime      = evt->endTime;
            pstInfo->stVca.priSize      = evt->priSize;
            pstInfo->stVca.enDtc        = evt->enDtc;
            if (evt->priSize) {
                memcpy(pstInfo->stVca.Private, evt->pstPrivate, evt->priSize);
            }
            break;
        case INFO_MAJOR_SMART:
            pstInfo->stSmart.startTime  = evt->startTime;
            pstInfo->stSmart.endTime    = evt->endTime;
            pstInfo->stSmart.priSize    = evt->priSize;
            if (evt->priSize) {
                memcpy(pstInfo->stSmart.Private, evt->pstPrivate, evt->priSize);
            }
            break;
            case INFO_MAJOR_AUDIO_ALARM:
                pstInfo->stAudioAlarm.startTime  = evt->startTime;
                pstInfo->stAudioAlarm.endTime    = evt->endTime;
                pstInfo->stAudioAlarm.priSize    = evt->priSize;
                if (evt->priSize) {
                    memcpy(pstInfo->stAudioAlarm.Private, evt->pstPrivate, evt->priSize);
                }
            break;
        default :
            MFerr("unkown major[%#x]", evt->enMajor);
            return MF_FAILURE;
    }

    strncpy(pstInfo->srcName, evt->srcName, MAX_SRC_NAME);
    pstInfo->majorType          = evt->enMajor;
    pstInfo->minorType          = evt->enMinor;
    pstInfo->bEnd               = evt->bEnd;
    pstInfo->dataTime           = (MF_U64)evt->endTime * 1000000;

    return MF_SUCCESS;
}

static MF_S32
record_pic_snap_parse(struct picCtrl *pstCtrl, VID_EN enVid,
                      INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor,
                      MF_PTS pts, MF_S8 *priData, MF_U8 priSize)
{

    if (enMajor == INFO_MAJOR_PB_PIC) {
        pstCtrl->stream_from = SST_LOCAL_PB;
    } else if (enMajor & (INFO_MAJOR_LIVE_PIC | INFO_MAJOR_TIME_PIC)) {
        pstCtrl->stream_from = SST_IPC_STREAM;
    } else {
        pstCtrl->stream_from = SST_IPC_STREAM;
    }

    switch (enVid) {
        case VID_TYPE_MAIN:
        case VID_TYPE_MAIN_ANR:
            pstCtrl->stream_format = STREAM_TYPE_MAINSTREAM;
            break;
        case VID_TYPE_SUB:
        case VID_TYPE_SUB_ANR:
            pstCtrl->stream_format = STREAM_TYPE_SUBSTREAM;
            break;
        default:
            return MF_FAILURE;
    }
    pstCtrl->enMajorType = enMajor;
    pstCtrl->enMinorType = enMinor;
    pstCtrl->pts = pts;

    if (priData && priSize > 0) {
        memcpy(pstCtrl->priData, priData, priSize);
        pstCtrl->priSize = priSize;
    }

    return MF_SUCCESS;
}
/*
---------------------------------------------------------------------------------
 event process
*/

static void
record_event_disk_remove(struct diskObj *pstDisk)
{
    struct recHDL *rechdl = NULL;
    struct recHDL *n = NULL;

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    list_for_each_entry_safe(rechdl, n, &g_stRecorder.HDLList, node) {
        if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
            struct recPic *pic = (struct recPic *)rechdl->pstRecObj;
            if (pic->pstRecFile->pstDisk == pstDisk) {
                record_pic_restart(rechdl, pic);
            }
        } else if (rechdl->enHDL == HDL_VID) {
            MFdbg("remove ch[%d] start", rechdl->id);
            struct recVid *vid = (struct recVid *)rechdl->pstRecObj;
            if (vid->pstRecFile->pstDisk == pstDisk) {
                record_vid_restart(rechdl, vid);
            }
            MFdbg("remove ch[%d] end", rechdl->id);
        }
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);
}

static void
record_event_disk_add(struct diskObj *pstDisk)
{
    struct recHDL *rechdl = NULL;
    struct recHDL *n = NULL;

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    list_for_each_entry_safe(rechdl, n, &g_stRecorder.HDLList, node) {
        if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
            struct recPic *pic = (struct recPic *)rechdl->pstRecObj;
            if (pic->bRec == MF_NO && mf_disk_can_write(pstDisk, pic->chnId) == MF_YES) {
                record_pic_restart(rechdl, pic);
            }
        } else if (rechdl->enHDL == HDL_VID) {
            struct recVid *vid = (struct recVid *)rechdl->pstRecObj;
            if (vid->bRec == MF_NO && mf_disk_can_write(pstDisk, vid->chnId) == MF_YES) {
                record_vid_restart(rechdl, vid);
            }
        }
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);
}

static void
record_event_disk_format(struct diskObj *pstDisk)
{
    struct recHDL *rechdl = NULL;
    struct recHDL *n = NULL;

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    list_for_each_entry_safe(rechdl, n, &g_stRecorder.HDLList, node) {
        if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
            struct recPic *pic = (struct recPic *)rechdl->pstRecObj;
            if (pic->pstRecFile->pstDisk == pstDisk) {
                record_pic_restart(rechdl, pic);
            }
        } else if (rechdl->enHDL == HDL_VID) {
            struct recVid *vid = (struct recVid *)rechdl->pstRecObj;
            if (vid->pstRecFile->pstDisk == pstDisk) {
                record_vid_restart(rechdl, vid);
            }
        }
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);
}

static void
record_event_cb(MODULE_E from, EVENT_E event, void *argv)
{
    MFinfo("from = %d event = %d begin\n", from, event);
    switch (event) {
        case MSFS_EVENT_DISK_DEL:
        case MSFS_EVENT_DISK_ERR:
            record_event_disk_remove(argv);
            break;
        case MSFS_EVENT_DISK_ADD:
            record_event_disk_add(argv);
            break;
        case MSFS_EVENT_DISK_FORMAT:
            record_event_disk_format(argv);
            break;
        default:
            break;
    }
    MFinfo("from = %d event = %d end\n", from, event);
}

static void *
record_task_pipline(void *argv)
{
    struct mfrecorder *pstRec = argv;
    struct recCache *cache;
    ms_task_set_name("task_pipline");

    while (pstRec->bPipRun) {
        cache = record_list_cache_pop();
        if (cache) {
            switch (cache->enPip) {
                case PIP_PICTURE:
                    record_pic_cache_push(cache->owner, cache);
                    break;
                case PIP_VIDEO:
                    record_vid_cache_push(cache->owner, cache);
                    break;
                default:
                    MFerr("unKnown enPip[%d]", cache->enPip);
                    record_cache_free(cache);
                    break;
            }
            continue;
        } else {
            MF_USLEEP(50000);
        }
    }
    record_list_cache_clear();

    return MF_SUCCESS;
}

static void *
record_task_drop_caches(void *argv)
{
    struct mfrecorder *pstRec = argv;
    MF_U32 free_size = pstRec->sysMemFree;
    MF_U32 count = 0, drop_now = 0, delta = 30;
    MF_U32 upgradeSize = 84*1024*1024;
    MF_U32 flag;
    ms_task_set_name("task_drop_caches");

    cpu_set_t mask;
    CPU_ZERO(&mask);
#ifdef HI_3536
    CPU_SET(1, &mask);
#else
    CPU_SET(2, &mask);
#endif
    MFinfo("task_drop_caches run at cpu:1.");
    if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
        MFerr("set thread affinity failed.");
    }

    while (pstRec->bDropRun) {
        count++;
        flag = 0;
        if (drop_now) {
            flag = 1;
        } else if (count % 6000 == 0) {
            //600s
            flag = 1;
        } else if (count % delta == 0) {
            //3s
            free_size = ms_get_mem(MEM_TYPE_FREE);
            if (pstRec->upgradeFlag) {
                if (free_size > 0 && free_size < upgradeSize) {
                    flag = 1;
                }
            } else {
                if (free_size > 0 && free_size < 2 * REC_SYS_MIN_SIZE) {
                    flag = 1;
                }
            }
            
        }

        if (flag) {
            ms_system("echo 3 > /proc/sys/vm/drop_caches");
            free_size = ms_get_mem(MEM_TYPE_FREE);
            if (pstRec->upgradeFlag && free_size > 0 && free_size < upgradeSize) {
                drop_now = 1;
            } else if (!pstRec->upgradeFlag && free_size > 0 && free_size < REC_SYS_MIN_SIZE) {
                drop_now = 1;
            } else {
                drop_now = 0;
            }
        }

        pstRec->sysMemFree = free_size;
        usleep(100000);
    }
    return NULL;
}

MF_S32 mf_record_mem_outdate_free(void)
{
    chnUsdMem *memNode = NULL;
    chnUsdMem *m = NULL;

    if (g_stRecorder.isInit == MF_NO) {
        return MF_SUCCESS;
    }

    g_stRecorder.upgradeFlag = 1;

    ms_mutex_lock(&g_stRecorder.MemMutex);
    list_for_each_entry_safe(memNode, m, &g_stRecorder.UsdMemList, node) {
        ms_free(memNode->dataBuff);
        ms_free(memNode->infoBuff);
        list_del(&memNode->node);
        record_mem_free(memNode);
        g_stRecorder.memCnt--;
        g_stRecorder.memMinu++;
        msprintf("[hrz.debug] memcnt 222 cnt:%d plus:%d minu:%d\n", g_stRecorder.memCnt, g_stRecorder.memPlus, g_stRecorder.memMinu);
    }
    ms_mutex_unlock(&g_stRecorder.MemMutex);

    return MF_SUCCESS;
}

static void *
record_task_chn_mem_manager(void *argv)
{
    struct mfrecorder *pstRec = argv;
    struct list_head *head = &pstRec->UsdMemList;
    chnUsdMem *pos = NULL;
    chnUsdMem *n = NULL;
    MF_PTS now;

    ms_task_set_name("task_chn_mem_manager");
    while (pstRec->bMemRun) {
        ms_mutex_lock(&g_stRecorder.MemMutex);
        if (list_empty(head)) {
            ms_mutex_unlock(&g_stRecorder.MemMutex);
            usleep(100000);
            continue;
        }

        now = time(0);
        list_for_each_entry_safe(pos, n, head, node) {
            if (pos->lastMemUsedTime + REC_MAX_MEM_TIME < now) {
                MFshow("task_chn_mem_manager free chnId[%d],type[%d]", pos->chnId, pos->enType);
                ms_free(pos->dataBuff);
                ms_free(pos->infoBuff);
                list_del(&pos->node);
                record_mem_free(pos);
                g_stRecorder.memCnt--;
                g_stRecorder.memMinu++;
                msprintf("[hrz.debug] memcnt 333 cnt:%d plus:%d minu:%d\n", g_stRecorder.memCnt, g_stRecorder.memPlus, g_stRecorder.memMinu);
            }
        }
        ms_mutex_unlock(&g_stRecorder.MemMutex);

        usleep(100000);
    }

    mf_record_mem_outdate_free();

    return NULL;
}

static void *
record_task_deadline_check(void *argv)
{
    struct mfrecorder *pstRec = argv;
    struct recHDL *rechdl = NULL;
    MF_U32  count = 0;
    MF_BOOL bUpdate = MF_NO;

    ms_task_set_name("deadline_check");

    while (pstRec->bDeadRun) {
        if (count % 100 == 0) {
            ms_rwlock_rdlock(&g_stRecorder.listRWLock);
            list_for_each_entry(rechdl, &g_stRecorder.HDLList, node) {
                if (rechdl->enHDL == HDL_VID) {
                    struct recVid *vid = (struct recVid *)rechdl->pstRecObj;
                    if (vid->DeadTime > 0) {
                        if (MF_YES == mf_retr_file_deadline_check(vid->chnId, vid->enType, vid->DeadTime)) {
                            bUpdate = MF_YES;
                        }
                    }
                } else if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
                    struct recPic *pic = (struct recPic *)rechdl->pstRecObj;
                    if (pic->DeadTime > 0) {
                        if (MF_YES == mf_retr_file_deadline_check(pic->chnId, FILE_TYPE_PIC, pic->DeadTime)) {
                            bUpdate = MF_YES;
                        }
                    }
                }

            }
            ms_rwlock_unlock(&g_stRecorder.listRWLock);

            if (bUpdate == MF_YES) {
                mf_record_update();
                bUpdate = MF_NO;
            }
        }
        count++;
        usleep(100000);
    }
    return NULL;

}

static void *
record_task_deadline_check_static(void *argv)
{
    struct mfrecorder *pstRec = argv;
    struct recVid *vid = NULL;
    struct recPic *pic = NULL;
    MF_U32  count = 0;
    MF_BOOL bUpdate = MF_NO;
    MF_U32 i, j;
    ms_task_set_name("deadline_check_static");

    while (pstRec->bDeadRun) {
        if (count % 100 == 0) {
            for (i = 0; i < MAX_REC_CHN_NUM; i++) {
                for (j = 0; j < VID_TYPE_NUM; j++) {
                    vid = &pstRec->stRecVid[i][j];
                    if (vid->DeadTime > 0) {
                        if (g_stRecorder.bDebug == MF_YES) {
                            MFdbg("deadline vid[%d][%d] time[%d] check\n", i, j, vid->DeadTime);
                        }
                        if (MF_YES == mf_retr_file_deadline_check(vid->chnId, vid->enType, vid->DeadTime)) {
                            bUpdate = MF_YES;
                        }
                    }
                }

                pic = &pstRec->stRecPic[i];
                if (pic->DeadTime > 0) {
                    if (g_stRecorder.bDebug == MF_YES) {
                        MFdbg("deadline pic[%d] time[%d] check\n", i, pic->DeadTime);
                    }
                    if (MF_YES == mf_retr_file_deadline_check(pic->chnId, FILE_TYPE_PIC, pic->DeadTime)) {
                        bUpdate = MF_YES;
                    }
                }
            }
            if (bUpdate == MF_YES) {
                mf_record_update();
                bUpdate = MF_NO;
            }
        }
        count++;
        usleep(100000);
    }
    return NULL;

}

static MF_S32
record_cache_size_calc(MF_U32 bandwidth)
{
    MF_S32 min = 0;

    if (bandwidth <= 2048) {
        min = 2048 * 1024;
    } else {
        if (g_stRecorder.max_chn <= 9) {
            min = MF_MIN(bandwidth * 1024 / 3, 10 * 1024 * 1024);
        } else if (g_stRecorder.max_chn <= 16) {
            min = MF_MIN(bandwidth * 1024 * 4 / 5, 10 * 1024 * 1024);
        } else if (g_stRecorder.max_chn <= 32) {
            min = MF_MIN(bandwidth * 1024, 8 * 1024 * 1024);
        } else {
            min = MF_MIN(bandwidth * 1024 * 3 / 2, 10 * 1024 * 1024);
        }
    }

    return mf_random2(min, min * 5 / 4, (MF_U32)&min);
}
/*
---------------------------------------------------------------------------------
*/

MF_S32
mf_record_jpg_encoder(struct mf_frame *frame, INFO_MAJOR_EN enMajor, struct jpg_frame *jpg)
{
    return record_pic_jpg_encoder(frame, enMajor, jpg);
}

MF_S32
mf_record_vid_event(struct recHDL *rechdl, struct rec_event_t *evt)
{
    struct recVid *pstRecVid = (struct recVid *)rechdl->pstRecObj;
    struct recCache *cache = record_mem_calloc(1, sizeof(struct recCache));
    MF_S32 res;

    if (!cache) {
        return MF_FAILURE;
    }

    res = record_vid_event_parse(&cache->info.stInfo, evt);
    if (res != MF_SUCCESS) {
        record_mem_free(cache);
        return res;
    }

    MFdbg("chn[%d] vid[%d] major[%#x] minor[%#x] srcName[%s] bEnd[%d]\n",
          pstRecVid->chnId, pstRecVid->enType,
          cache->info.stInfo.majorType,
          cache->info.stInfo.minorType,
          cache->info.stInfo.srcName,
          cache->info.stInfo.bEnd);

    cache->info.enCache = CACHE_TYPE_INFO;
    if (record_vid_cache_push(pstRecVid, cache) == MF_FAILURE) {
        return MF_FAILURE;
    }

    return MF_SUCCESS;

}

MF_S32
mf_record_pic_snap(struct recHDL *rechdl, VID_EN enVid,
                   INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor,
                   MF_PTS pts, MF_S8 priData[], MF_U8 priSize)
{
    struct recPic *pstRecPic = (struct recPic *)rechdl->pstRecObj;

    if (pstRecPic->bRec == MF_NO) {
        return MF_FAILURE;
    }

    struct picCtrl *pstCtrl = record_mem_calloc(1, sizeof(struct picCtrl));
    MF_S32 res;

    if (!pstCtrl) {
        return MF_FAILURE;
    }
    pstCtrl->rechdl = rechdl;
    res = record_pic_snap_parse(pstCtrl, enVid, enMajor, enMinor, pts, priData, priSize);
    if (res != MF_SUCCESS) {
        record_mem_free(pstCtrl);
        return res;
    }
    MFdbg("chn[%d] major[%#x] minor[%#x] snap[%s]", pstRecPic->chnId,
          pstCtrl->enMajorType, pstCtrl->enMinorType, mf_time_to_string(pts));

    ms_mutex_lock(&pstRecPic->ctrlMutex);
    LIST_INSERT_SORT_ASC(pstCtrl, &pstRecPic->listCtrl, enMajorType);
    ms_mutex_unlock(&pstRecPic->ctrlMutex);

    return MF_SUCCESS;
}

MF_S32
mf_record_pic_lpr(struct recHDL *rechdl, struct rec_lpr_t *pstLpr)
{
    if (pstLpr->pstPrivate && pstLpr->priSize > LPR_PRIVATE_SIZE) {
        MFerr("LPR private data size[%d] over [%d]", pstLpr->priSize, LPR_PRIVATE_SIZE);
        return MF_FAILURE;
    }

    struct recPic *pstRecPic = NULL;

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    if (record_hdl_find(rechdl) == MF_YES) {
        pstRecPic = rechdl->pstRecObj;
    } else {
        ms_rwlock_unlock(&g_stRecorder.listRWLock);
        return MF_FAILURE;
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);

    struct recCache *cache = lpr_cache_alloc(pstLpr);
    if (cache) {
        cache->enPip = PIP_PICTURE;
        cache->owner = pstRecPic;
        cache->info.rechdl = rechdl;
        record_list_cache_push(cache);
        return MF_SUCCESS;
    }

    return MF_FAILURE;
}

MF_S32
mf_record_pic_face(struct recHDL *rechdl, struct rec_face_t *pstFace)
{
    if (pstFace->num > MAX_FACE_NUM) {
        MFerr("face nums[%d] over max[%d]", pstFace->num, MAX_FACE_NUM);
        return MF_FAILURE;
    }

    struct recPic *pstRecPic = NULL;

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    if (record_hdl_find(rechdl) == MF_YES) {
        pstRecPic = rechdl->pstRecObj;
    } else {
        ms_rwlock_unlock(&g_stRecorder.listRWLock);
        return MF_FAILURE;
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);

    struct recCache *cache = face_cache_alloc(pstFace);
    if (cache) {
        cache->enPip = PIP_PICTURE;
        cache->owner = pstRecPic;
        cache->info.rechdl = rechdl;
        record_list_cache_push(cache);
        return MF_SUCCESS;
    }

    return MF_FAILURE;
}

MF_S32
mf_record_vid_smart(struct recHDL *rechdl, struct rec_smart_t *pstSmart, MF_U32 minorType)
{
    struct recVid *pstRecVid = NULL;
    struct recCache *cache = NULL;
    MF_U8 chnId = pstSmart->frame->ch;
    VID_EN enVid = record_vid_parse_frame(pstSmart->frame);

    if (record_vid_can_write(chnId, enVid, pstSmart->frame) == MF_FAILURE) {
        return MF_FAILURE;
    }

    cache = frame_cache_alloc(pstSmart->frame);
    if (cache == NULL) {
        return MF_FAILURE;
    }

    cache->info.stInfo.majorType = INFO_MAJOR_SMART;
    cache->info.stInfo.minorType = minorType;

    struct info_common *info = &cache->info.stInfo.stCom;
    info->startTime = pstSmart->startTime;
    info->endTime = pstSmart->endTime;
    if (pstSmart->pstPrivate) {
        memcpy(info->Private, pstSmart->pstPrivate, pstSmart->priSize);
        info->priSize = pstSmart->priSize;
    }

    pstRecVid = &g_stRecorder.stRecVid[chnId][enVid];
    cache->owner = pstRecVid;
    cache->enPip = PIP_VIDEO;
    cache->info.enCache = CACHE_TYPE_STREAM | CACHE_TYPE_INFO;
    cache->info.stFrame.uid = pstRecVid->uid;
    cache->info.stInfo.dataTime = pstSmart->endTime * 1000000;

    record_list_cache_push(cache);

    return MF_SUCCESS;
}

struct recHDL *
mf_record_pic_create(MF_U8 chnId, HDL_EN enHdl)
{
    struct recHDL *rechdl;
    struct recPic *pstRecPic =
            &g_stRecorder.stRecPic[chnId];
    MFdbg("record_pic[%d]_create begin ...", chnId);
    ms_mutex_lock(&g_stRecorder.PicMutex);
    if (pstRecPic->busy == 0) {
        pstRecPic->chnId = chnId;
        ms_mutex_init(&pstRecPic->cacheMutex);
        ms_mutex_init(&pstRecPic->ctrlMutex);
        mf_cond_init(&pstRecPic->cond);
        INIT_LIST_HEAD(&pstRecPic->listCache);
        INIT_LIST_HEAD(&pstRecPic->listCtrl);
        pstRecPic->bRun = MF_YES;
        pstRecPic->bUserStart = MF_NO;
        pstRecPic->bExit = MF_YES;
        pstRecPic->bRec = MF_NO;
        pstRecPic->bRestart = MF_NO;
        pstRecPic->CacheSize = ALIGN_BACK(mf_random2(REC_PIC_SIZE, REC_MAX_PIC_SIZE, (MF_U32)pstRecPic), 512);
        pstRecPic->pstRecFile = record_mem_calloc(1, sizeof(struct bplusFile));
        pstRecPic->pstRecFile->bValid = MF_NO;
        pstRecPic->pstRecFile->pCacheSize = &pstRecPic->PreSize;
        pstRecPic->pstRecFile->dataSize = pstRecPic->CacheSize;
        pstRecPic->pstRecFile->dataOffset = 0;
        pstRecPic->pstRecFile->infoSize = REC_INFO_SIZE;
        pstRecPic->pstRecFile->infoOffset = REC_INFO_SIZE;
        pstRecPic->pstRecFile->syncSize = mf_random(REC_MIN_SYNC_SIZE, pstRecPic->pstRecFile->dataSize);
        pstRecPic->pstRecFile->lastInfoTime = time(0);
        pstRecPic->pstRecFile->lastFileTime = time(0);
        ms_mutex_init(&pstRecPic->pstRecFile->IOmutex);
        mf_cond_init(&pstRecPic->pstRecFile->IOcond);
        ms_mutex_init(&pstRecPic->pstRecFile->CMDmutex);
        mf_cond_init(&pstRecPic->pstRecFile->CMDcond);
        mf_comm_task_submit(record_pic_task, pstRecPic);
    }
    pstRecPic->busy++;
    rechdl = record_hdl_add(chnId, enHdl, pstRecPic);
    ms_mutex_unlock(&g_stRecorder.PicMutex);
    MFdbg("record_pic[%d]_create end ...", chnId);

    return rechdl;
}

void
mf_record_pic_destory(struct recHDL *rechdl)
{
    assert(rechdl);
    struct recPic *pstRecPic;
    MF_U8 chnId;

    ms_mutex_lock(&g_stRecorder.PicMutex);
    pstRecPic = (struct recPic *)rechdl->pstRecObj;
    assert(pstRecPic);

    chnId = pstRecPic->chnId;
    MFdbg("record_pic[%d]_destory begin ...", chnId);
    if (pstRecPic->busy == 1) {
        pstRecPic->bRun = MF_NO;
        pstRecPic->bRec = MF_NO;
        pstRecPic->bUserStart = MF_NO;
        pstRecPic->busy = 0;
        while (pstRecPic->bExit == MF_NO) {
            MF_USLEEP(50000);
        }
        ms_mutex_uninit(&pstRecPic->pstRecFile->IOmutex);
        mf_cond_uninit(&pstRecPic->pstRecFile->IOcond);
        ms_mutex_uninit(&pstRecPic->pstRecFile->CMDmutex);
        mf_cond_uninit(&pstRecPic->pstRecFile->CMDcond);

        record_mem_free(pstRecPic->pstRecFile);
        record_pic_cmd_clear(pstRecPic, NULL);
        record_pic_cache_clear(pstRecPic);
        mf_cond_uninit(&pstRecPic->cond);
        ms_mutex_uninit(&pstRecPic->cacheMutex);
        ms_mutex_uninit(&pstRecPic->ctrlMutex);
    }
    if (pstRecPic->busy) {
        pstRecPic->busy--;
    }
    record_hdl_del(rechdl);
    ms_mutex_unlock(&g_stRecorder.PicMutex);
    MFdbg("record_pic[%d]_destory end ...", chnId);
}

MF_S32
mf_record_pic_start(struct recHDL *rechdl)
{
    struct recPic *pstRecPic;
    pstRecPic = (struct recPic *)rechdl->pstRecObj;

    ms_mutex_lock(&g_stRecorder.PicMutex);
    if (pstRecPic->busy == 1) {
        pstRecPic->bRec = mf_disk_is_ready();
    }
    ms_mutex_unlock(&g_stRecorder.PicMutex);
    pstRecPic->bUserStart = MF_YES;

    return MF_SUCCESS;
}

MF_S32
mf_record_pic_stop(struct recHDL *rechdl)
{
    struct recPic *pstRecPic;
    pstRecPic = (struct recPic *)rechdl->pstRecObj;
    ms_mutex_lock(&g_stRecorder.PicMutex);
    if (pstRecPic->busy == 1) {
        struct recCache *cache;
        cache = record_mem_calloc(1, sizeof(struct recCache));
        if (!cache) {
            ms_mutex_unlock(&g_stRecorder.PicMutex);
            return MF_FAILURE;
        }
        cache->info.enCache = CACHE_TYPE_STOP;
        if (record_pic_cache_push(pstRecPic, cache) == MF_FAILURE) {
            pstRecPic->bRec = MF_NO;
        }
        ms_mutex_lock(&pstRecPic->ctrlMutex);
        mf_cond_timedwait(&pstRecPic->cond, &pstRecPic->ctrlMutex, 30000000);
        ms_mutex_unlock(&pstRecPic->ctrlMutex);
    }
    record_pic_cmd_clear(pstRecPic, rechdl);
    ms_mutex_unlock(&g_stRecorder.PicMutex);

    return MF_SUCCESS;
}

struct recHDL *
mf_record_vid_create(MF_U8 chnId, VID_EN enVid, MF_U32 bandwidth)
{
    if (chnId >= MAX_REC_CHN_NUM || enVid >= VID_TYPE_NUM) {
        MFerr("param invalid ! chnId[%d]enVid[%d]", chnId, enVid);
        return NULL;
    }

    RWLOCK_BOJECT *rwlock = &g_stRecorder.vidRWLock[chnId];
    struct recVid *pstRecVid = &g_stRecorder.stRecVid[chnId][enVid];

    MFdbg("Create chnid[%d] enVid[%d] begin\n", chnId, enVid);
    ms_rwlock_wrlock(rwlock);
    MFdbg("Create chnid[%d] enVid[%d] readying\n", chnId, enVid);
    if (pstRecVid->bEnable == MF_NO) {
        pstRecVid->chnId = chnId;
        pstRecVid->enEvent = REC_EVENT_NONE;
        pstRecVid->enVid = enVid;
        // pstRecVid->bandwidth = bandwidth * 1024;
        pstRecVid->bandwidth = ALIGN_BACK(record_cache_size_calc(bandwidth), 512);
        pstRecVid->CacheSize = ALIGN_BACK(record_cache_size_calc(bandwidth), 512);
        msprintf("[gsjt] mf_record_vid_create chnId:%d CacheSize:%d bandwidth:%d",pstRecVid->chnId, pstRecVid->CacheSize, pstRecVid->bandwidth);
        record_vid_get_type_state(pstRecVid);
        ms_mutex_init(&pstRecVid->ctrlMutex);
        ms_mutex_init(&pstRecVid->cacheMutex);
        mf_cond_init(&pstRecVid->cond);
        pstRecVid->bRun = MF_NO;
        pstRecVid->bExit = MF_YES;
        pstRecVid->bUserStart = MF_NO;
        pstRecVid->bRec = MF_NO;
        pstRecVid->bRestart = MF_NO;
        INIT_LIST_HEAD(&pstRecVid->listCache);
        pstRecVid->pstRecFile = record_mem_calloc(1, sizeof(struct bplusFile));
        pstRecVid->pstRecFile->bValid = MF_NO;
        pstRecVid->pstRecFile->bNewFile = MF_NO;
        pstRecVid->pstRecFile->bNewInfo = MF_NO;
        pstRecVid->pstRecFile->bContinued = MF_NO;
        pstRecVid->pstRecFile->pCacheSize = &pstRecVid->PreSize;
        pstRecVid->pstRecFile->dataSize = pstRecVid->CacheSize;
        pstRecVid->pstRecFile->dataOffset = 0;
        pstRecVid->pstRecFile->infoSize = REC_INFO_SIZE;
        pstRecVid->pstRecFile->infoOffset = REC_INFO_SIZE;
        pstRecVid->pstRecFile->syncSize = mf_random(REC_MIN_SYNC_SIZE, pstRecVid->pstRecFile->dataSize);
        pstRecVid->pstRecFile->lastInfoTime = time(0);
        pstRecVid->pstRecFile->lastFileTime = time(0);
        ms_mutex_init(&pstRecVid->pstRecFile->IOmutex);
        mf_cond_init(&pstRecVid->pstRecFile->IOcond);
        ms_mutex_init(&pstRecVid->pstRecFile->CMDmutex);
        mf_cond_init(&pstRecVid->pstRecFile->CMDcond);
        pstRecVid->bEnable = MF_YES;
        MFinfo("Create chnid[%d] enVid[%d], CacheSize[%d] syncSize[%d]success\n",
               chnId, enVid, pstRecVid->CacheSize, pstRecVid->pstRecFile->syncSize);
        pstRecVid->phdl = record_hdl_add(chnId, HDL_VID, pstRecVid);
        ms_rwlock_unlock(rwlock);
        MFdbg("Create chnid[%d] enVid[%d] end\n", chnId, enVid);
        return pstRecVid->phdl;
    }
    ms_rwlock_unlock(rwlock);
    MFerr("Create chnid[%d] enVid[%d] has existed\n", chnId, enVid);
    return NULL;
}

void
mf_record_vid_destory(struct recHDL *rechdl)
{
    struct recVid *pstRecVid;
    pstRecVid = (struct recVid *)rechdl->pstRecObj;
    RWLOCK_BOJECT *rwlock = &g_stRecorder.vidRWLock[pstRecVid->chnId];

    MFdbg("destory chnid[%d] enVid[%d] begin\n",
          pstRecVid->chnId, pstRecVid->enVid);
    ms_rwlock_wrlock(rwlock);
    MFdbg("destory chnid[%d] enVid[%d] readying\n",
          pstRecVid->chnId, pstRecVid->enVid);
    if (pstRecVid->bEnable == MF_YES) {
        pstRecVid->bRun = MF_NO;
        pstRecVid->bEnable = MF_NO;
        pstRecVid->bUserStart = MF_NO;
        MFdbg("destory chnid[%d] enVid[%d] task wait!\n",
              pstRecVid->chnId, pstRecVid->enVid);
        while (pstRecVid->bExit == MF_NO) {
            MF_USLEEP(50000);
        }
        MFdbg("destory chnid[%d] enVid[%d] task exit!\n",
              pstRecVid->chnId, pstRecVid->enVid);
        record_vid_cache_clear(pstRecVid);
        ms_mutex_uninit(&pstRecVid->pstRecFile->IOmutex);
        mf_cond_uninit(&pstRecVid->pstRecFile->IOcond);
        ms_mutex_uninit(&pstRecVid->pstRecFile->CMDmutex);
        mf_cond_uninit(&pstRecVid->pstRecFile->CMDcond);
        record_mem_free(pstRecVid->pstRecFile);
        mf_cond_uninit(&pstRecVid->cond);
        ms_mutex_uninit(&pstRecVid->ctrlMutex);
        ms_mutex_uninit(&pstRecVid->cacheMutex);
        record_hdl_del(rechdl);
        pstRecVid->phdl = NULL;
        MFinfo("destory chnid[%d] enVid[%d] CacheNum[%d]success\n",
               pstRecVid->chnId, pstRecVid->enVid, pstRecVid->CacheNum);
    }
    ms_rwlock_unlock(rwlock);
    MFdbg("destory chnid[%d] enVid[%d] end\n",
          pstRecVid->chnId, pstRecVid->enVid);
}

MF_S32
mf_record_vid_start(struct recHDL *rechdl, REC_EVENT_EN enEvent)
{
    struct recVid *pstRecVid;

    pstRecVid = (struct recVid *)rechdl->pstRecObj;

    if (pstRecVid->bUserStart == MF_YES) {
        return MF_SUCCESS;
    }

    if (!enEvent) {
        MFlog(MF_ERR, "[wcm] record event unknow, chnid = %d", pstRecVid->chnId);
    }

    pstRecVid->enEvent = enEvent;
    pstRecVid->bUserStart = MF_YES;
    if (pstRecVid->bRun == MF_NO) {
        pstRecVid->bRun = MF_YES;
        mf_comm_task_submit(record_vid_task, pstRecVid);
    }

    ms_mutex_lock(&pstRecVid->ctrlMutex);
    mf_cond_timedwait(&pstRecVid->cond, &pstRecVid->ctrlMutex, 30000000);
    ms_mutex_unlock(&pstRecVid->ctrlMutex);
    pstRecVid->bRec = mf_disk_is_ready();

    MFshow("chn[%d] type[%d] event[%#x]start[%d]", pstRecVid->chnId, pstRecVid->enType, enEvent, pstRecVid->bRec);
    return MF_SUCCESS;
}

MF_S32
mf_record_vid_soft_stop(struct recHDL *rechdl)
{
    struct recVid *pstRecVid;
    struct recCache *cache;

    pstRecVid = (struct recVid *)rechdl->pstRecObj;
    if (pstRecVid->bUserStart == MF_NO) {
        return MF_SUCCESS;
    }

    cache = record_mem_calloc(1, sizeof(struct recCache));
    if (!cache) {
        return MF_FAILURE;
    }

    cache->info.enCache = CACHE_TYPE_STOP;
    if (record_vid_cache_push(pstRecVid, cache) == MF_FAILURE) {
        return MF_FAILURE;
    }
    MFshow(" 000 chn[%d] type[%d] stop[%d] %p ", pstRecVid->chnId, pstRecVid->enType, pstRecVid->bRec, cache);
    ms_mutex_lock(&pstRecVid->ctrlMutex);
    mf_cond_timedwait(&pstRecVid->cond, &pstRecVid->ctrlMutex, 30000000);
    ms_mutex_unlock(&pstRecVid->ctrlMutex);
    pstRecVid->bUserStart = MF_NO;

    struct rec_res_t stRes;
    stRes.chnId = pstRecVid->chnId;
    record_user_notify(MSFS_EVENT_REC_STOP, rechdl, &stRes);
    MFshow("111 chn[%d] type[%d] stop[%d]", pstRecVid->chnId, pstRecVid->enType, pstRecVid->bRec);

    return MF_SUCCESS;
}

MF_S32
mf_record_vid_hard_stop(struct recHDL *rechdl, MF_BOOL bUser)
{
    struct recVid *pstRecVid;

    pstRecVid = (struct recVid *)rechdl->pstRecObj;

    pstRecVid->bRec = MF_NO;
    if (bUser) {
        pstRecVid->bUserStart = MF_NO;
    }
    struct rec_res_t stRes;
    stRes.chnId = pstRecVid->chnId;
    record_user_notify(MSFS_EVENT_REC_STOP, rechdl, &stRes);
    MFshow("chn[%d] type[%d] force stop[%d]", pstRecVid->chnId, pstRecVid->enType, pstRecVid->bRec);

    return MF_SUCCESS;
}

MF_S32
mf_record_vid_set_audio(struct recHDL *rechdl, MF_BOOL bAudio)
{
    struct recVid *pstRecVid;
    pstRecVid = (struct recVid *)rechdl->pstRecObj;

    ms_mutex_lock(&pstRecVid->ctrlMutex);
    pstRecVid->bAudio = bAudio;
    ms_mutex_unlock(&pstRecVid->ctrlMutex);
    MFdbg("chn[%d] type[%d] audio = %d",
          pstRecVid->chnId, pstRecVid->enType, bAudio);

    return MF_SUCCESS;
}

MF_S32
mf_record_vid_set_pre_time(struct recHDL *rechdl, MF_U32 PreTime)
{
    if (PreTime > REC_MAX_PRE_TIME) {
        MFerr("set pre time(%d) over max time(%d)", PreTime, REC_MAX_PRE_TIME);
        return MF_FAILURE;
    }

    struct recVid *pstRecVid;
    pstRecVid = (struct recVid *)rechdl->pstRecObj;

    ms_mutex_lock(&pstRecVid->ctrlMutex);
    pstRecVid->PreTime = PreTime;
    ms_mutex_unlock(&pstRecVid->ctrlMutex);
    MFdbg("chn[%d] type[%d] preTime = %d",
          pstRecVid->chnId, pstRecVid->enType, PreTime);

    return MF_SUCCESS;
}

MF_S32
mf_record_vid_set_deadline_time(struct recHDL *rechdl, MF_U32 DeadTime)
{
    if (DeadTime > REC_MAX_VID_DEAD_TIME) {
        MFerr("set deadline time(%d) over max time(%d)", DeadTime, REC_MAX_VID_DEAD_TIME);
        return MF_FAILURE;
    }

    struct recVid *pstRecVid;
    pstRecVid = (struct recVid *)rechdl->pstRecObj;

    pstRecVid->DeadTime = DeadTime;
    MFdbg("chn[%d] type[%d] deadTime = %d",
          pstRecVid->chnId, pstRecVid->enType, DeadTime);

    return MF_SUCCESS;
}

MF_S32
mf_record_pic_set_deadline_time(struct recHDL *rechdl, MF_U32 DeadTime)
{
    if (DeadTime > REC_MAX_PIC_DEAD_TIME) {
        MFerr("set deadline time(%d) over max time(%d)", DeadTime, REC_MAX_PIC_DEAD_TIME);
        return MF_FAILURE;
    }

    struct recPic *pstRecPic;
    pstRecPic = (struct recPic *)rechdl->pstRecObj;

    pstRecPic->DeadTime = DeadTime;
    MFdbg("chn[%d]  deadTime = %d", pstRecPic->chnId, DeadTime);

    return MF_SUCCESS;
}

MF_S32
mf_record_vid_set_deadline_time_static(MF_U8 chnId, VID_EN enVid, MF_U32 DeadTime)
{
    if (DeadTime > REC_MAX_VID_DEAD_TIME) {
        MFerr("set deadline time(%d) over max time(%d)", DeadTime, REC_MAX_VID_DEAD_TIME);
        return MF_FAILURE;
    }

    if (chnId >= MAX_REC_CHN_NUM || enVid >= VID_TYPE_NUM) {
        MFerr("param invalid ! chnId[%d]enVid[%d]", chnId, enVid);
        return MF_FAILURE;
    }

    struct recVid *pstRecVid = &g_stRecorder.stRecVid[chnId][enVid];

    pstRecVid->DeadTime = DeadTime;
    pstRecVid->chnId = chnId;
    pstRecVid->enVid = enVid;

    return MF_SUCCESS;
}

MF_S32
mf_record_pic_set_deadline_time_static(MF_U8 chnId, MF_U32 DeadTime)
{
    if (DeadTime > REC_MAX_PIC_DEAD_TIME) {
        MFerr("set deadline time(%d) over max time(%d)", DeadTime, REC_MAX_PIC_DEAD_TIME);
        return MF_FAILURE;
    }

    if (chnId >= MAX_REC_CHN_NUM) {
        MFerr("param invalid ! chnId[%d]", chnId);
        return MF_FAILURE;
    }

    struct recPic *pstRecPic = &g_stRecorder.stRecPic[chnId];

    pstRecPic->DeadTime = DeadTime;
    pstRecPic->chnId = chnId;

    return MF_SUCCESS;
}

MF_S32
mf_record_vid_sync(MF_U8 chnId, FILE_TYPE_EN enType)
{
    if (chnId >= MAX_REC_CHN_NUM) {
        MFerr("param invalid ! chnId[%d]", chnId);
        return MF_FAILURE;
    }

    struct recVid *pstRecVid;
    RWLOCK_BOJECT *rwlock = &g_stRecorder.vidRWLock[chnId];
    MF_S32 i;

    if (ms_rwlock_tryrdlock(rwlock) != 0) {
        return MF_FAILURE;
    }
    for (i = 0; i < VID_TYPE_NUM; i++) {
        pstRecVid = &g_stRecorder.stRecVid[chnId][i];
        if (pstRecVid->enType != enType) {
            continue;
        }
        if (pstRecVid->bEnable == MF_YES && pstRecVid->bRec == MF_YES) {
            struct recCache *cache;
            cache = record_mem_calloc(1, sizeof(struct recCache));
            if (!cache) {
                ms_rwlock_unlock(rwlock);
                return MF_FAILURE;
            }

            cache->info.enCache = CACHE_TYPE_SYNC;
            if (record_vid_cache_push(pstRecVid, cache) == MF_FAILURE) {
                ms_rwlock_unlock(rwlock);
                return MF_FAILURE;
            }
            ms_mutex_lock(&pstRecVid->pstRecFile->CMDmutex);
            mf_cond_timedwait(&pstRecVid->pstRecFile->CMDcond,
                              &pstRecVid->pstRecFile->CMDmutex, 5000000);
            ms_mutex_unlock(&pstRecVid->pstRecFile->CMDmutex);
        }
    }
    ms_rwlock_unlock(rwlock);

    return MF_SUCCESS;
}

MF_S32
mf_record_pic_sync(MF_U8 chnId)
{
    if (chnId >= MAX_REC_CHN_NUM) {
        MFerr("param invalid ! chnId[%d]", chnId);
        return MF_FAILURE;
    }

    struct recPic *pstRecPic = &g_stRecorder.stRecPic[chnId];

    ms_mutex_lock(&g_stRecorder.PicMutex);

    if (pstRecPic->busy && pstRecPic->bRec == MF_YES) {
        struct recCache *cache;
        cache = record_mem_calloc(1, sizeof(struct recCache));
        if (!cache) {
            ms_mutex_unlock(&g_stRecorder.PicMutex);
            return MF_FAILURE;
        }

        cache->info.enCache = CACHE_TYPE_SYNC;
        if (record_pic_cache_push(pstRecPic, cache) == MF_FAILURE) {
            ms_mutex_unlock(&g_stRecorder.PicMutex);
            return MF_FAILURE;
        }
        ms_mutex_lock(&pstRecPic->pstRecFile->CMDmutex);
        mf_cond_timedwait(&pstRecPic->pstRecFile->CMDcond,
                          &pstRecPic->pstRecFile->CMDmutex, 5000000);
        ms_mutex_unlock(&pstRecPic->pstRecFile->CMDmutex);
    }

    ms_mutex_unlock(&g_stRecorder.PicMutex);

    return MF_SUCCESS;
}

void
mf_record_deadline_check_start(MF_BOOL bStatic)
{
    g_stRecorder.bDeadRun = MF_YES;
    if (bStatic == MF_YES)
        ms_task_create_join_stack_size(&g_stRecorder.deadPthread, 16 * 1024,
                                       record_task_deadline_check_static, &g_stRecorder);
    else
        ms_task_create_join_stack_size(&g_stRecorder.deadPthread, 16 * 1024,
                                       record_task_deadline_check, &g_stRecorder);
}

void
mf_record_deadline_check_stop()
{
    g_stRecorder.bDeadRun = MF_NO;
    ms_task_join(&g_stRecorder.deadPthread);
}

void
mf_record_vid_receive(struct mf_frame *pstFrame)
{

    if (pstFrame->stream_from != SST_IPC_ANR
        && pstFrame->stream_from != SST_IPC_STREAM) {
        return ;
    }
    MF_U8 chnId = pstFrame->ch;
    VID_EN enVID = record_vid_parse_frame(pstFrame);

    record_vid_process(chnId, enVID, pstFrame);
}

void
mf_record_pic_receive(struct mf_frame *pstFrame)
{
    MF_U8 chnId = pstFrame->ch;

    record_pic_process(chnId, pstFrame);
}

MF_S32
mf_record_pic_frame(struct recHDL *rechdl, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor, 
    struct mf_frame *pstFrame, MF_S8 *priData, MF_U8 priSize)
{
    struct recPic *pstRecPic = NULL;

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    if (record_hdl_find(rechdl) == MF_YES) {
        pstRecPic = rechdl->pstRecObj;
    } else {
        ms_rwlock_unlock(&g_stRecorder.listRWLock);
        return MF_FAILURE;
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);

    struct recCache *cache = frame_cache_alloc(pstFrame);
    if (cache) {
        cache->enPip = PIP_PICTURE;
        cache->owner = pstRecPic;
        cache->info.enCache = CACHE_TYPE_STREAM | CACHE_TYPE_INFO;
        cache->info.stInfo.majorType = enMajor;
        cache->info.stInfo.minorType = enMinor;
        cache->info.stInfo.dataTime = pstFrame->time_usec;
        cache->info.stInfo.stPic.width = pstFrame->width;
        cache->info.stInfo.stPic.height = pstFrame->height;
        cache->info.stInfo.stPic.encode = pstFrame->codec_type;
        cache->info.stInfo.stPic.time = pstFrame->time_usec / 1000000;
        cache->info.rechdl = rechdl;
        if (priData && priSize > 0) {
            memcpy(cache->info.stInfo.stPic.priData, priData, priSize);
            cache->info.stInfo.stPic.priSize = priSize;
        }
        record_list_cache_push(cache);
        return MF_SUCCESS;
    }

    return MF_FAILURE;
}

void
mf_record_update()
{
    struct diskObj *pstDisk;
    struct recHDL *rechdl = NULL;
    MF_U8 chnId;

    if (g_stRecorder.isInit == MF_NO) {
        return ;
    }

    MFdbg("mf_record_update ready");
    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    MFdbg("mf_record_update begin");

    list_for_each_entry(rechdl, &g_stRecorder.HDLList, node) {
        if (rechdl->enHDL == HDL_PIC || rechdl->enHDL == HDL_PB) {
            struct recPic *pic = (struct recPic *)rechdl->pstRecObj;

            if (pic->bRec == MF_NO) {
                record_pic_restart(rechdl, pic);
            } else {
                chnId = pic->chnId;
                pstDisk = pic->pstRecFile->pstDisk;
                if (mf_disk_can_write(pstDisk, chnId) == MF_NO) {
                    record_pic_restart(rechdl, pic);
                }

            }
        } else if (rechdl->enHDL == HDL_VID) {
            struct recVid *vid = (struct recVid *)rechdl->pstRecObj;
            if (vid->bRec == MF_NO) {
                record_vid_restart(rechdl, vid);
            } else {
                chnId = vid->chnId;
                pstDisk = vid->pstRecFile->pstDisk;
                if (mf_disk_can_write(pstDisk, chnId) == MF_NO) {
                    record_vid_restart(rechdl, vid);
                }
            }
        }
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);
    MFdbg("mf_record_update end");
}

void
mf_record_data_sink(struct mf_frame *pstFrame)
{
    if (g_stRecorder.bReceive == MF_NO) {
        return;
    }

    mf_record_vid_receive(pstFrame);
    mf_record_pic_receive(pstFrame);
}

void
mf_record_event_notify(EVENT_E event, void *argv, MF_BOOL bBlock)
{
    if (bBlock == MF_NO) {
        mf_comm_task_notify(MODULE_REC, event, argv);
    } else {
        mf_comm_event_notify(MODULE_REC, event, argv);
    }
}

MF_S8 *
mf_record_type(FILE_TYPE_EN enType)
{
    switch (enType) {
        case FILE_TYPE_MAIN:
            return "main";
        case FILE_TYPE_SUB:
            return "sub";
        case FILE_TYPE_PIC:
            return "picture";
        default:
            return "unknown";
    }
}

MF_S8 *
mf_record_event(REC_EVENT_EN enEvent)
{
    switch (enEvent) {
        case REC_EVENT_TIME:
            return "timing";
        case REC_EVENT_MOTION:
            return "motion";
        case REC_EVENT_ALARMIN:
            return "alarmIn";
        case REC_EVENT_MANUAL:
            return "manual";
        case REC_EVENT_ANR:
            return "ANR";
        case REC_EVENT_VCA:
            return "VCA";
        case REC_EVENT_SMART:
            return "SMART";
        case REC_EVENT_AUDIO_ALARM:
            return "audio alarm";
        default:
            return "unknown";
    }
}

MF_S8 *
mf_record_ack(EVENT_E event)
{
    switch (event) {
        case MSFS_EVENT_REC_START:
            return "MSFS_EVENT_REC_START";
        case MSFS_EVENT_REC_STOP:
            return "MSFS_EVENT_REC_STOP";
        case MSFS_EVENT_REC_FAIL:
            return "MSFS_EVENT_REC/SNAP_FAIL";
        case MSFS_EVENT_REC_SUCCEED:
            return "MSFS_EVENT_SNAP_SUCCEED";
        default:
            return "nothing";
    }
}

void
mf_record_dbg_show_chn_info(MF_U8 chnId)
{
    struct recHDL *rechdl = NULL;
    struct recHDL *n = NULL;
    MF_U32 count = 0;

    ms_rwlock_rdlock(&g_stRecorder.listRWLock);
    list_for_each_entry_safe(rechdl, n, &g_stRecorder.HDLList, node) {
        if (rechdl->enHDL == HDL_PB) {
            continue;
        } else if (rechdl->enHDL == HDL_PIC) {
            struct recPic *pic = (struct recPic *)rechdl->pstRecObj;
            if (pic->chnId == chnId) {
                MFprint("picNum:      [%d]\n", pic->CacheNum);
                MFprint("picBuf:      [%d]/[%d][%d%%]\n", pic->pstRecFile->dataOffset,
                        pic->CacheSize, pic->pstRecFile->dataOffset / (pic->CacheSize / 100));
                MFprint("picSize:     [%d]\n", pic->PreSize);


            }
        } else if (rechdl->enHDL == HDL_VID) {
            struct recVid *vid = (struct recVid *)rechdl->pstRecObj;
            if (vid->chnId == chnId || chnId >= MAX_REC_CHN_NUM) {
                MFprint("-[%d]----------------------->\n", ++count);
                MFprint("id:         [%d]\n", rechdl->id);
                MFprint("chnid:      [%d]\n", vid->chnId);
                MFprint("usrRec:     [%d]\n", vid->bUserStart);
                MFprint("audio:      [%d]\n", vid->bAudio);
                MFprint("enable:     [%d]\n", vid->bRec);
                MFprint("type:       [%s]\n", mf_record_type(vid->enType));
                MFprint("event:      [%s]\n", vid->bUserStart ?
                        mf_record_event(vid->enEvent) : "unknown");
                MFprint("uid:        [%d]\n", vid->uid);
                MFprint("frame:      [%d]\n", vid->CacheNum);
                MFprint("preTIme:    [%d]\n", vid->PreTime);
                MFprint("deadTIme:   [%d]\n", vid->DeadTime);
                MFprint("preSize:    [%d]/[%d][%d%%]\n", vid->PreSize, MF_MIN(vid->CacheSize, REC_MAX_PRE_SIZE),
                        vid->PreSize / (MF_MIN(vid->CacheSize, REC_MAX_PRE_SIZE) / 100));
                MFprint("buffer:     [%d]/[%d][%d%%]\n", vid->pstRecFile->dataOffset,
                        vid->CacheSize, vid->pstRecFile->dataOffset / (vid->CacheSize / 100));
                if (vid->pstRecFile->pstDisk) {
                    MFprint("disk:      [%d]\n", vid->pstRecFile->pstDisk->port);
                }
            }
        }
    }
    ms_rwlock_unlock(&g_stRecorder.listRWLock);
}

void
mf_record_dbg_show_mem_usage()
{
    if (g_stRecorder.pstMem) {
        MFinfo("record_dbg_show_mem_usage");
        msprintf("[hrz.debug] memcnt 444 cnt:%d plus:%d minu:%d\n", g_stRecorder.memCnt, g_stRecorder.memPlus, g_stRecorder.memMinu);
        mf_mem_state(g_stRecorder.pstMem, MF_YES);
    }
    mf_comm_task_dump();
}

void
mf_record_dbg_receive_enable(MF_BOOL bReceive)
{
    MFdbg("bReceive = %d", bReceive);
    g_stRecorder.bReceive = bReceive;
}

void
mf_record_dbg_switch(MF_BOOL bDebug)
{
    g_stRecorder.bDebug = bDebug;
}

MF_BOOL
mf_record_is_dbg()
{
    return g_stRecorder.bDebug;
}

MF_S32
mf_record_init(REC_USER_CB user_cb, REC_JPG_CB jpg_cb, MF_U32 max_chn)
{
    MF_U32 i;

    memset(&g_stRecorder, 0, sizeof(struct mfrecorder));
    record_mem_init(&g_stRecorder);
    INIT_LIST_HEAD(&g_stRecorder.HDLList);
    INIT_LIST_HEAD(&g_stRecorder.PipList);
    INIT_LIST_HEAD(&g_stRecorder.UsdMemList);
    ms_rwlock_init(&g_stRecorder.listRWLock);
    for (i = 0; i < MAX_REC_CHN_NUM; i++) {
        ms_rwlock_init(&g_stRecorder.vidRWLock[i]);
    }
    ms_mutex_init(&g_stRecorder.PicMutex);
    ms_mutex_init(&g_stRecorder.PipMutex);
    ms_mutex_init(&g_stRecorder.MemMutex);
    mf_comm_event_register(MODULE_REC, record_event_cb);
    ps_packet_cb(record_mem_alloc, record_mem_free);

#if !(isAIO)
    g_stRecorder.bDropRun = MF_YES;
    ms_task_create_join_stack_size(&g_stRecorder.dropPthread, 256 * 1024, record_task_drop_caches, &g_stRecorder);
#endif
    g_stRecorder.bPipRun = MF_YES;
    ms_task_create_join_stack_size(&g_stRecorder.pipPthread, 16 * 1024, record_task_pipline, &g_stRecorder);
    mf_set_task_policy(g_stRecorder.pipPthread, THTEAD_SCHED_RR_PRI);

    g_stRecorder.bMemRun = MF_YES;
    ms_task_create_join_stack_size(&g_stRecorder.usdMemPthread, 16 * 1024, record_task_chn_mem_manager, &g_stRecorder);

    g_stRecorder.bReceive = MF_YES;
    g_stRecorder.user_cb = user_cb;
    g_stRecorder.jpg_cb = jpg_cb;
    g_stRecorder.max_chn = max_chn;
    g_stRecorder.sysMemFree = ms_get_mem(MEM_TYPE_FREE);
    g_stRecorder.isInit = MF_YES;

    return MF_SUCCESS;
}

MF_S32
mf_record_deinit()
{
    MF_U32 i;

    g_stRecorder.bReceive = MF_NO;
    g_stRecorder.bDropRun = MF_NO;
    g_stRecorder.bPipRun  = MF_NO;
    g_stRecorder.bMemRun  = MF_NO ;

    ms_task_join(&g_stRecorder.usdMemPthread);
    ms_task_join(&g_stRecorder.pipPthread);
#if !(isAIO)
    ms_task_join(&g_stRecorder.dropPthread);
#endif
    mf_comm_event_unregister(MODULE_REC);
    for (i = 0; i < MAX_REC_CHN_NUM; i++) {
        ms_rwlock_uninit(&g_stRecorder.vidRWLock[i]);
    }
    ms_rwlock_uninit(&g_stRecorder.listRWLock);
    ms_mutex_uninit(&g_stRecorder.PicMutex);
    ms_mutex_uninit(&g_stRecorder.PipMutex);
    record_mem_uninit(&g_stRecorder);

    g_stRecorder.isInit = MF_NO;

    return MF_SUCCESS;
}

