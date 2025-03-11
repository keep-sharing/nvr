/*
 * ***************************************************************
 * Filename:        MFretrieve.c
 * Created at:      2017.05.10
 * Description:     retrieve API
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
#include "MFindex.h"
#include "MFtask.h"
#include "MFmemory.h"
#include "MFlog.h"
#include "mf_type.h"
#include "msfs_bkp.h"
#include "msg.h"
#include "msstd.h"

#define RETR_FASTER  1

#define  RETR_EXPORT_BUFF_SIZE  (4*1024*1024)
#define RETR_UNLOAD_TIME    (24*60*60)  // 24 hours
#define IPCINFO_FIND_BY_ID(id, pstIpcInfo, head) \
    { \
        struct ipc_info* pos = NULL; \
        struct ipc_info* n = NULL; \
        list_for_each_entry_safe(pos, n, head, node) \
        { \
            if (id == pos->chnId) \
            { \
                pstIpcInfo = pos; \
                break; \
            } \
        } \
    }

typedef struct mf_retr {
    MF_BOOL bDebug;
    struct mf_mem_t *pstMem;
    RETR_JPG_CB jpg_cb;
} mf_retr;

static struct mf_retr g_stRetr;

static void
retr_mem_init(struct mf_retr *pstRetr)
{
    struct mem_pool_t *pstMpool = mf_comm_mem_get();

    if (pstMpool->retrSize) {
        pstRetr->pstMem = mf_mem_pool_create("mfretrieve", pstMpool->retrSize);
    }
}

static void
retr_mem_uninit(struct mf_retr *pstRetr)
{
    if (pstRetr->pstMem) {
        mf_mem_state(pstRetr->pstMem, MF_YES);
        mf_mem_pool_destory(pstRetr->pstMem);
    }
}

static inline void *
retr_mem_alloc(MF_U32 size)
{
    void *p;

    if (g_stRetr.pstMem) {
        p = mf_mem_alloc(g_stRetr.pstMem, size);
    } else {
        p = ms_malloc(size);
    }

    return p;
}

static inline void *
retr_mem_calloc(MF_U32 n, MF_U32 size)
{
    void *p;

    if (g_stRetr.pstMem) {
        p = mf_mem_calloc(g_stRetr.pstMem, n, size);
    } else {
        p = ms_calloc(n, size);
    }

    return p;
}

static inline void
retr_mem_free(void *p)
{
    if (g_stRetr.pstMem) {
        mf_mem_free(g_stRetr.pstMem, p);
    } else {
        ms_free(p);
    }
}

static inline MF_U32
retr_mem_usage_percent(MF_U32 def)
{
    if (g_stRetr.pstMem) {
        return mf_mem_state(g_stRetr.pstMem, MF_NO);
    } else {
        return def;
    }
}

static struct seg_info *
retr_seg_info_alloc()
{
    struct seg_info *pstSegInfo;

    pstSegInfo = retr_mem_calloc(1, sizeof(struct seg_info));
    pstSegInfo->segStartTime = INIT_START_TIME;
    pstSegInfo->segEndTime = INIT_END_TIME;
    pstSegInfo->id = -1;

    INIT_LIST_HEAD(&pstSegInfo->node);

    return pstSegInfo;
}

static void
retr_seg_info_free(struct seg_info *pstSegInfo)
{
    retr_mem_free(pstSegInfo);
}

static MF_S32
retr_seg_info_add(struct seg_info *pstSegInfo, struct file_info *pstFileInfo)
{
    if (pstSegInfo == NULL) {
        return MF_FAILURE;
    }
    pstSegInfo->owner = pstFileInfo;
    LIST_INSERT_SORT_ASC(pstSegInfo, &pstFileInfo->segHead, segStartTime);
    pstFileInfo->segNum++;

    return MF_SUCCESS;
}

static void
retr_seg_info_del(struct seg_info *pstSegInfo)
{
    if (pstSegInfo == NULL) {
        return;
    }
    list_del(&pstSegInfo->node);
    pstSegInfo->owner->segNum--;
}

static void
retr_seg_info_dump(struct seg_info *pstSegInfo)
{
    MF_S8 startTime [32];
    MF_S8 endTime [32];

    time_to_string_local(pstSegInfo->segStartTime, startTime);
    time_to_string_local(pstSegInfo->segEndTime, endTime);
    MFprint("\t\tsegInfo:\n");
    MFprint("\t\t\tID: [%d]\n", pstSegInfo->id);
    MFprint("\t\t\tevent: [%#x]%s\n", pstSegInfo->enEvent, mf_record_event(pstSegInfo->enEvent));
    MFprint("\t\t\tmajor: [%#x]\n", pstSegInfo->enMajor);
    MFprint("\t\t\tminor: [%#x]\n", pstSegInfo->enMinor);
    MFprint("\t\t\tstate: %d\n", pstSegInfo->enState);
    MFprint("\t\t\tsize: %d\n", pstSegInfo->recEndOffset - pstSegInfo->recStartOffset);
    MFprint("\t\t\ttime: %s -- %s\n", startTime, endTime);
    MFprint("\t\t\tdata: %#x -- %#x\n", pstSegInfo->recStartOffset, pstSegInfo->recEndOffset);
    MFprint("\t\t\tinfo: %#x -- %#x\n", pstSegInfo->infoStartOffset, pstSegInfo->infoEndOffset);
    MFprint("\t\t\tcount: %d\n", pstSegInfo->infoCount);
    MFprint("\t\t\tfirstframeTime: %s\n", mf_time_to_string(pstSegInfo->firstFrameTime / 1000000));
    MFprint("\t\t\tfirstIframeTime: %s\n", mf_time_to_string(pstSegInfo->firstIframeTime / 1000000));
    MFprint("\t\t\tlastIframeTime: %s\n", mf_time_to_string(pstSegInfo->lastIframeTime / 1000000));
}

static struct file_info *
retr_file_info_alloc(struct bplus_key *key)
{
    struct file_info *pstFileInfo;

    pstFileInfo = retr_mem_calloc(1, sizeof(struct file_info));
    INIT_LIST_HEAD(&pstFileInfo->segHead);
    pstFileInfo->key = *key;
    pstFileInfo->fileStartTime = INIT_START_TIME;
    pstFileInfo->fileEndTime = INIT_END_TIME;
    pstFileInfo->enLoad = FILE_UNLOAD;
    pstFileInfo->segNum = 0;
    pstFileInfo->lockNum = 0;
    pstFileInfo->tagNum = 0;
    pstFileInfo->refNum = 0;
    pstFileInfo->loadTime = 0;

    return pstFileInfo;
}

static void
retr_file_info_free(struct file_info *pstFileInfo)
{
    retr_mem_free(pstFileInfo);
}

static MF_S32
retr_file_info_add(struct file_info *pstFileInfo, struct ipc_info *pstIpcInfo)
{
    struct diskObj *pstDisk = pstIpcInfo->owner;
    MF_S8 *cache = NULL;
    off64_t offset;
    MF_S32  i, res, n = 0;
    MF_U32 segNum = 0;
    MF_U32 len = 0;
    MF_S32  fd;
    MF_U32 lastSegRecEndOffset = 0;
    MF_U32 fileRecStartTime;
    MF_U32 fileRecEndTime;
    struct mf_file *pstFile;
    struct mf_segment *pstSegment;
    struct bplus_key stKey;
    struct bplus_data stData;

    len = FILE_PER_SIZE - SEG_HEAD_OFFSET;
    cache = ms_valloc(len);
    if (!cache) {
        MFerr("valloc failed");
        return MF_FAILURE;
    }

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, pstFileInfo->key.stRecord.fileNo, MF_YES);
    offset = pstFileInfo->key.stRecord.fileOffset + SEG_HEAD_OFFSET;
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, cache, len, offset);
    if (res < 0) {
        pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
        retr_mem_free(cache);
        MFerr("disk_read failed");
        return MF_FAILURE;
    }

    pstFile = (struct mf_file *)(cache + len - FILE_HEAD_MAX_SIZE);
    if (pstFile->magic != FILE_MAGIC
        || pstFile->segNum > SEG_MAX_NUM
        || (pstFileInfo->key.stRecord.state == FILE_STATE_USED && pstFile->segNum == 0)) {
        // the file is bad, delete it;
        MFerr(" port[%d] file[%d] magic[%#x] segNum[%d] is bad, delete it!",
              pstDisk->port, pstFileInfo->key.stRecord.fileNo,
              pstFile->magic, pstFile->segNum);
        if (mf_index_key_delete(pstDisk->tree, &pstFileInfo->key) == MF_SUCCESS) {
            mf_index_key_init(&stKey, TYPE_RECORD);
            stKey.stRecord.condition = FILE_CONDITION_BAD;
            stKey.stRecord.fileNo = pstFileInfo->key.stRecord.fileNo;
            stKey.stRecord.fileOffset = pstFileInfo->key.stRecord.fileOffset;
            stData.stRecord.fileOffset = stKey.stRecord.fileOffset;
            res = mf_index_key_insert(pstDisk->tree, &stKey, &stData);
            if (res == MF_SUCCESS) {
                mf_index_key_backup(pstDisk->tree);
                mf_index_data_init(pstDisk->tree, &stKey);
            } else {
                mf_index_key_recover(pstDisk->tree);
            }
        }
        pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
        retr_mem_free(cache);
        return MF_FAILURE;
    }

    segNum = pstFile->segNum;
    fileRecStartTime = pstFile->recStartTime;
    fileRecEndTime = pstFile->recEndTime;
    pstFileInfo->owner = pstIpcInfo;
    pstFileInfo->lockNum = pstFile->lockNum;
    pstFileInfo->tagNum = pstFile->tagNum;
    for (i = 0; i < segNum; i++) {
        pstSegment = (struct mf_segment *)(cache + i * SEG_HEAD_SIZE);
//        MFprint("===No[%d]===status=%d=====\n",pstFileInfo->key.stRecord.fileNo, pstSegment->status);
        if (pstSegment->status != SEG_STATE_LOCK &&  pstSegment->status != SEG_STATE_NORMAL) {
            continue;
        }
        if (pstSegment->recStartTime < fileRecStartTime || pstSegment->recEndTime > fileRecEndTime) {
            continue;
        }
        if (lastSegRecEndOffset > pstSegment->recStartOffset) {
            break;
        }
        struct seg_info *pstSegInfo    = retr_seg_info_alloc();
        pstSegInfo->id                  = pstSegment->id;
        pstSegInfo->segStartTime        = pstSegment->recStartTime;
        pstSegInfo->segEndTime          = pstSegment->recEndTime;
        pstSegInfo->infoStartOffset     = pstSegment->infoStartOffset;
        pstSegInfo->infoEndOffset       = pstSegment->infoEndOffset;
        pstSegInfo->infoCount           = pstSegment->infoCount;
        pstSegInfo->recStartOffset      = pstSegment->recStartOffset;
        pstSegInfo->recEndOffset        = pstSegment->recEndOffset;
        pstSegInfo->enState             = pstSegment->status;
        pstSegInfo->enEvent             = pstSegment->recEvent;
        pstSegInfo->enMajor             = pstSegment->infoTypes;
        pstSegInfo->enMinor             = pstSegment->infoSubTypes;
        pstSegInfo->firstFrameTime      = pstSegment->firstFrameTime;
        pstSegInfo->firstIframeTime     = pstSegment->firstIframeTime;
        pstSegInfo->firstIframeOffset   = pstSegment->firstIframeOffset;
        pstSegInfo->lastFrameTime       = pstSegment->lastFrameTime;
        pstSegInfo->lastIframeTime      = pstSegment->lastIframeTime;
        pstSegInfo->lastIframeOffset    = pstSegment->lastIframeOffset;
        pstSegInfo->encodeType          = pstSegment->encodeType;
        pstSegInfo->resWidth            = pstSegment->resWidth;
        pstSegInfo->resHeight           = pstSegment->resHeight;
        if (retr_seg_info_add(pstSegInfo,  pstFileInfo) == MF_FAILURE) {
            retr_seg_info_free(pstSegInfo);
        } else {
            lastSegRecEndOffset = pstSegment->recEndOffset;
            mf_retr_seg_owner_update(pstSegInfo);
            n++;
        }
    }

    retr_mem_free(cache);
    pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);

    if (segNum == 0 || n) {
        return MF_SUCCESS;
    } else {
        MFerr("file add failed[%d][%d][%d][%d]", pstDisk->port, pstFileInfo->key.stRecord.fileNo, segNum, n);
        MFerr("file startTime[%s]", mf_time_to_string(pstFileInfo->key.stRecord.startTime));
        MFerr("file endTime[%s]", mf_time_to_string(pstFileInfo->key.stRecord.endTime));
        if (mf_index_key_delete(pstDisk->tree, &pstFileInfo->key) == MF_SUCCESS) {
            mf_index_key_init(&stKey, TYPE_RECORD);
            stKey.stRecord.condition = FILE_CONDITION_BAD;
            stKey.stRecord.fileNo = pstFileInfo->key.stRecord.fileNo;
            stKey.stRecord.fileOffset = pstFileInfo->key.stRecord.fileOffset;
            stData.stRecord.fileOffset = stKey.stRecord.fileOffset;
            res = mf_index_key_insert(pstDisk->tree, &stKey, &stData);
            if (res == MF_SUCCESS) {
                mf_index_key_backup(pstDisk->tree);
                mf_index_data_init(pstDisk->tree, &stKey);
            } else {
                mf_index_key_recover(pstDisk->tree);
            }
        }
        return MF_FAILURE;
    }

    return MF_SUCCESS;
}

static void
retr_file_info_del(struct file_info *pstFileInfo)
{
    struct seg_info *pos = NULL;
    struct seg_info *n = NULL;

    list_for_each_entry_safe(pos, n, &pstFileInfo->segHead, node) {
        retr_seg_info_del(pos);
        mf_retr_seg_owner_update(pos);
        retr_seg_info_free(pos);
    }
    pstFileInfo->pstSegUsing = NULL;
    list_del(&pstFileInfo->node);
    pstFileInfo->owner->fileNum--;
    pstFileInfo->enLoad = FILE_UNLOAD;
}

static MF_S32
retr_file_info_seg_load(struct file_info *pstFileInfo)
{
    struct ipc_info *pstIpcInfo = pstFileInfo->owner;
    MF_S32 res = MF_FAILURE;

    ms_mutex_lock(&pstIpcInfo->mutex);
    if (pstFileInfo->enLoad == FILE_SEG_LOAD) {
        ms_mutex_unlock(&pstIpcInfo->mutex);
        return MF_SUCCESS;
    }
    res = retr_file_info_add(pstFileInfo,  pstIpcInfo);
    if (res == MF_SUCCESS) {
        pstFileInfo->enLoad = FILE_SEG_LOAD;
        pstFileInfo->loadTime = mf_task_timer();
        mf_retr_file_owner_update(pstFileInfo);
    } else {
        retr_file_info_del(pstFileInfo);
        mf_retr_file_owner_update(pstFileInfo);
        retr_file_info_free(pstFileInfo);
    }

    mf_retr_ipc_owner_update(pstIpcInfo);
    ms_mutex_unlock(&pstIpcInfo->mutex);

    return res;
}

static MF_S32
retr_file_info_seg_unload(struct file_info *pstFileInfo)
{
    //info(state machine) -> preload -> reload -> unload->preload
    struct ipc_info *pstIpcInfo = pstFileInfo->owner;

    ms_mutex_lock(&pstIpcInfo->mutex);
    if (pstFileInfo->enLoad != FILE_SEG_LOAD) {
        ms_mutex_unlock(&pstIpcInfo->mutex);
        return MF_FAILURE;
    }

    struct seg_info *pos = NULL;
    struct seg_info *n = NULL;

    list_for_each_entry_safe(pos, n, &pstFileInfo->segHead, node) {
        retr_seg_info_del(pos);
        retr_seg_info_free(pos);
    }
    pstFileInfo->enLoad = FILE_SEG_UNLOAD;
    pstFileInfo->loadTime = 0;
    ms_mutex_unlock(&pstIpcInfo->mutex);

    return MF_SUCCESS;
}

static MF_S32
retr_file_info_pre_load(struct file_info *pstFileInfo, struct ipc_info *pstIpcInfo)
{
    struct bplus_key *key = &pstFileInfo->key;

    if (pstFileInfo->enLoad != FILE_UNLOAD) {
        return MF_FAILURE;
    }

    if (key->stRecord.state == FILE_STATE_USING || key->stRecord.state == FILE_STATE_ANR) {
        pstFileInfo->enLoad = FILE_SEG_LOAD;
        pstFileInfo->loadTime = mf_task_timer();
        return retr_file_info_add(pstFileInfo, pstIpcInfo);
    } else {
        pstFileInfo->enLoad = FILE_PRE_LOAD;
        pstFileInfo->enEvent = key->stRecord.event;
        pstFileInfo->fileStartTime = key->stRecord.startTime;
        pstFileInfo->fileEndTime = key->stRecord.endTime;
        pstFileInfo->owner = pstIpcInfo;
    }
    return MF_SUCCESS;
}

static void
retr_file_info_dump(struct file_info *pstFileInfo)
{
    struct seg_info *pos = NULL;
    struct seg_info *n = NULL;
    MF_S8 startTime [32];
    MF_S8 endTime [32];

    time_to_string_local(pstFileInfo->fileStartTime, startTime);
    time_to_string_local(pstFileInfo->fileEndTime, endTime);
    MFprint("\tfileInfo[chn %d]:\n", pstFileInfo->owner->chnId);
    MFprint("\t\tNO: %d-%llx\n", pstFileInfo->key.stRecord.fileNo,
            pstFileInfo->key.stRecord.fileOffset);
    MFprint("\t\ttype: %s\n", mf_record_type(pstFileInfo->key.stRecord.type));
    MFprint("\t\tstate: %d\n", pstFileInfo->key.stRecord.state);
    MFprint("\t\tevnet: [%#x]\n", pstFileInfo->enEvent);
    MFprint("\t\tmajor: [%#x]\n", pstFileInfo->enMajor);
    MFprint("\t\tminor: [%#x]\n", pstFileInfo->enMinor);
    MFprint("\t\tsize: %d\n", pstFileInfo->recEndOffset - pstFileInfo->recStartOffset);
    MFprint("\t\ttime: %s -- %s\n", startTime, endTime);
    MFprint("\t\tsegNum: %d\n", pstFileInfo->segNum);
    MFprint("\t\tlockNum: %d\n", pstFileInfo->lockNum);
    MFprint("\t\trefNum: %d\n", pstFileInfo->refNum);
    MFprint("\t\tenLoad: %d\n", pstFileInfo->enLoad);
    MFprint("\t\tbRecording: %d\n", pstFileInfo->bRecording);
    list_for_each_entry_safe(pos, n, &pstFileInfo->segHead, node) {
        retr_seg_info_dump(pos);
    }
}

static struct ipc_info *
retr_ipc_info_alloc(MF_U8 chnId)
{
    struct ipc_info *pstIpcInfo;

    pstIpcInfo = retr_mem_calloc(1, sizeof(struct ipc_info));
    INIT_LIST_HEAD(&pstIpcInfo->fileHead);
    ms_mutex_init(&pstIpcInfo->mutex);
    pstIpcInfo->ipcStartTime = INIT_START_TIME;
    pstIpcInfo->ipcEndTime = INIT_END_TIME;
    pstIpcInfo->chnId = chnId;

    return pstIpcInfo;
}

static void
retr_ipc_info_free(struct ipc_info *pstIpcInfo)
{
    ms_mutex_uninit(&pstIpcInfo->mutex);
    retr_mem_free(pstIpcInfo);
}

static void
retr_ipc_info_del(struct ipc_info *pstIpcInfo)
{
    struct file_info *pos = NULL;
    struct file_info *n = NULL;

    ms_mutex_lock(&pstIpcInfo->mutex);
    list_for_each_entry_safe(pos, n, &pstIpcInfo->fileHead, node) {
        retr_file_info_del(pos);
        mf_retr_file_owner_update(pos);
        retr_file_info_free(pos);
    }
    ms_mutex_unlock(&pstIpcInfo->mutex);
    list_del(&pstIpcInfo->node);
    pstIpcInfo->owner->ipcNum--;
}

static MF_S32
retr_ipc_info_add(struct ipc_info *pstIpcInfo, struct diskObj *pstDiskInfo)
{
    struct file_info *pstFileInfo = NULL;
    struct ipc_info *find = NULL;
    struct bplus_result stResult;
    struct bplus_node *node = NULL;
    struct bplus_key *key = NULL;
    MF_S32  index = 0;

    IPCINFO_FIND_BY_ID(pstIpcInfo->chnId, find, &pstDiskInfo->ipcHead);
    if (!find) {
        if (mf_index_find_chnid(pstDiskInfo->tree, pstIpcInfo->chnId, &stResult) == MF_YES) {
            pstIpcInfo->owner = pstDiskInfo;
            LIST_INSERT_SORT_ASC(pstIpcInfo, &pstDiskInfo->ipcHead, chnId);
            pstDiskInfo->ipcNum++;

            ms_mutex_lock(&pstIpcInfo->mutex);
            index = stResult.keyIndex;
            node = mf_index_node_fetch(pstDiskInfo->tree, stResult.nodeSelf);
            while (node) {
                if (pstDiskInfo->bExit == MF_YES) {
                    mf_index_node_release(pstDiskInfo->tree, node);
                    ms_mutex_unlock(&pstIpcInfo->mutex);
                    retr_ipc_info_del(pstIpcInfo);
                    return MF_FAILURE;
                }
                if (index < node->children) {
                    key = mf_index_node_key(node, index);
                    if (key->stRecord.chnId == pstIpcInfo->chnId
                        && key->stRecord.condition == FILE_CONDITION_GOOD) { //empty file chnid is 255;
                        pstFileInfo = retr_file_info_alloc(key);
                        //if (retr_file_info_add(pstFileInfo, pstIpcInfo) == MF_FAILURE)
                        if (retr_file_info_pre_load(pstFileInfo, pstIpcInfo) == MF_FAILURE) {
                            retr_file_info_free(pstFileInfo);
                        } else {
                            LIST_INSERT_SORT_ASC(pstFileInfo, &pstIpcInfo->fileHead, fileStartTime);
                            pstIpcInfo->fileNum++;
                            mf_retr_file_owner_update(pstFileInfo);
                        }
                        index++;
                    } else {
                        break;
                    }
                } else {
                    mf_index_node_release(pstDiskInfo->tree, node);
                    node = mf_index_node_fetch(pstDiskInfo->tree, node->next);
                    index = 0;
                }
            }
            mf_index_node_release(pstDiskInfo->tree, node);
            ms_mutex_unlock(&pstIpcInfo->mutex);
            return MF_SUCCESS;
        }
        return MF_FAILURE;
    } else {
        MFerr("chnid[%d] has exist", pstIpcInfo->chnId);
        return MF_FAILURE;
    }

    return MF_SUCCESS;
}

static void
retr_ipc_info_dump(struct ipc_info *pstIpcInfo)
{
    struct file_info *pos = NULL;
    struct file_info *n = NULL;
    MF_S8 startTime [32];
    MF_S8 endTime [32];

    time_to_string_local(pstIpcInfo->ipcStartTime, startTime);
    time_to_string_local(pstIpcInfo->ipcEndTime, endTime);
    MFprint("ipcInfo:\n");
    MFprint("\tchnId: %d\n", pstIpcInfo->chnId);
    MFprint("\ttime: %s -- %s\n", startTime, endTime);

    time_to_string_local(pstIpcInfo->mainStartTime, startTime);
    time_to_string_local(pstIpcInfo->mainEndTime, endTime);
    MFprint("\tmain: %s -- %s\n", startTime, endTime);

    time_to_string_local(pstIpcInfo->subStartTime, startTime);
    time_to_string_local(pstIpcInfo->subEndTime, endTime);
    MFprint("\tsub: %s -- %s\n", startTime, endTime);

    MFprint("\tfileNum: %d\n", pstIpcInfo->fileNum);
    if (pstIpcInfo->pstFirstUsed)
        MFprint("\tfirstUsed: %d-%llx\n", pstIpcInfo->pstFirstUsed->key.stRecord.fileNo,
                pstIpcInfo->pstFirstUsed->key.stRecord.fileOffset);

    ms_mutex_lock(&pstIpcInfo->mutex);
    list_for_each_entry_safe(pos, n, &pstIpcInfo->fileHead, node) {
        retr_file_info_dump(pos);
    }
    ms_mutex_unlock(&pstIpcInfo->mutex);
}

static MF_S32
retr_cmp_down(const void *b, const void *a)
{
    struct mf_info *ainfo = (struct mf_info *)a;
    struct mf_info *binfo = (struct mf_info *)b;

    if (ainfo->majorType < binfo->majorType) {
        return -1;
    } else if (ainfo->majorType > binfo->majorType) {
        return 1;
    } else {
        if (ainfo->dataTime < binfo->dataTime) {
            return -1;
        } else {
            return 1;
        }
    }
}

static MF_S32
retr_cmp_up(const void *a, const void *b)
{
    struct mf_info *ainfo = (struct mf_info *)a;
    struct mf_info *binfo = (struct mf_info *)b;

    if (ainfo->majorType < binfo->majorType) {
        return -1;
    } else if (ainfo->majorType > binfo->majorType) {
        return 1;
    } else {
        if (ainfo->dataTime < binfo->dataTime) {
            return -1;
        } else {
            return 1;
        }
    }
}

static MF_S32
retr_cmp_bsearch(struct mf_info *info, MF_U64 timeUs, MF_U16 majorType)
{
//    MFprint("=====majorType[%d]====time==%llu, info-time %llu \n", majorType, (MF_U64)timeUs, (MF_U64)info->dataTime);
    if (info->majorType > majorType) {
        return 1;
    } else if (info->majorType < majorType) {
        return -1;
    } else if (info->dataTime > timeUs) {
        return 1;
    } else if (info->dataTime < timeUs) {
        return -1;
    } else {
        return 0;
    }
}

static MF_BOOL
retr_is_iframe(struct mf_frame *f)
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

static MF_S32
retr_find_next_iframe(struct exp_info *exp)
{
    struct mf_info *info;
    MF_S32 index;

    if (exp->lastIframeTime == 0) {
        index = exp->infoIndex;
    } else {
        index = mf_retr_seg_info_bsearch(exp->infoBuff,
                                         exp->infoSize,
                                         exp->lastIframeTime,
                                         INFO_MAJOR_IFRAME,
                                         SORT_IS_UP);
        if (SORT_IS_UP) {
            index++;
        } else {
            index--;
        }
    }

    if (index < 0 || index > exp->infoCount) {
        return MF_FAILURE;
    }

    info = mf_retr_get_iframe_info(exp->infoBuff, index);

    exp->fileOffset = info->dataOffset - exp->stream->recStartOffset;
    exp->infoIndex  = index;
    exp->lastIframeTime = info->dataTime;

    return MF_SUCCESS;
}

static MF_S32
retr_load_frame_buff(struct exp_info *exp)
{
    MF_S32 readSize;

    if (exp->bTail == MF_YES) {
        return MF_FAILURE;
    }

    if (exp->fileOffset >= exp->fileSize) {
        return MF_FAILURE;
    }

    exp->dataSize = ALIGN_BACK(MF_MIN(exp->fileSize - exp->fileOffset, RETR_EXPORT_BUFF_SIZE), 512);

    off64_t offset = ALIGN_BACK(exp->fileBase + exp->fileOffset, 512);
    exp->alignOffset = (exp->fileBase + exp->fileOffset) % 512;

    readSize = exp->stream->pstDisk->ops->disk_file_read(exp->stream->pstDisk->pstPrivate,
                                                         exp->fd, exp->dataBuff,
                                                         exp->dataSize, offset);
    if (readSize > 0) {
        exp->bTail = exp->dataSize != RETR_EXPORT_BUFF_SIZE ? MF_YES : MF_NO;
        exp->dataSize = readSize;
        exp->dataHead = exp->fileOffset;
        exp->dataTail = exp->dataHead + exp->dataSize;
        exp->dataOffset = 0;
        return MF_SUCCESS;
    } else {
        MFerr("disk_read failed");
        return MF_FAILURE;
    }
}

static void
retr_get_event_info_time(INFO_MAJOR_EN enMajor, struct mf_info *info,
                         MF_PTS *startTime, MF_PTS *endTime)
{
    switch (enMajor) {
        case INFO_MAJOR_MOTION:
            *startTime  = info->stMotion.startTime;
            *endTime    = info->stMotion.endTime;
            break;
        case INFO_MAJOR_ALARMIN:
            *startTime  = info->stAlarm.startTime;
            *endTime    = info->stAlarm.endTime;
            break;
        case INFO_MAJOR_VCA:
            *startTime  = info->stVca.startTime;
            *endTime    = info->stVca.endTime;
            break;
        case INFO_MAJOR_SMART:
            *startTime  = info->stSmart.startTime;
            *endTime    = info->stSmart.endTime;
            break;
        case INFO_MAJOR_AUDIO_ALARM:
            *startTime  = info->stAudioAlarm.startTime;
            *endTime    = info->stAudioAlarm.endTime;
            break;
        default:
            break;
    }
}

static MF_BOOL
retr_usr_match(struct mf_info *info, EVT_MATCH match, void *pri, struct bkp_pri_t *src)
{
    src->data = NULL;
    src->size = 0;

    switch (info->majorType) {
        case INFO_MAJOR_MOTION:
            src->data = info->stMotion.Private;
            src->size = info->stMotion.priSize;
            break;
        case INFO_MAJOR_ALARMIN:
            src->data = info->stAlarm.Private;
            src->size = info->stAlarm.priSize;
            break;
        case INFO_MAJOR_VCA:
            src->data = info->stVca.Private;
            src->size = info->stVca.priSize;
            break;
        case INFO_MAJOR_SMART:
            src->data = info->stSmart.Private;
            src->size = info->stSmart.priSize;
            break;
        case INFO_MAJOR_AUDIO_ALARM:
            src->data = info->stAudioAlarm.Private;
            src->size = info->stAudioAlarm.priSize;
            break;
        default:
            return MF_YES;
    }

    if (match) {
        return match(pri, src);
    } else {
        return MF_YES;
    }
}

static inline void
retr_check_mark_time(struct seg_info *seg, MF_PTS *startTime, MF_PTS *endTime)
{
    /*
        兼容：旧版本因修改系统时间，存在标签起始结束时间异常的情况
        14-r8从源头修改，搜索兼容若干版本后可删除
    */
    MF_U64 interval = 1800000000;//30min --> us;
    MF_PTS firstIframeTime;
    MF_PTS lastIframeTime;
    
    if ((*startTime > *endTime)
        || (seg->lastIframeTime > seg->lastFrameTime)
        || ((seg->lastFrameTime - seg->lastIframeTime) >= interval)) {

        firstIframeTime = seg->firstIframeTime / 1000000;
        lastIframeTime = seg->lastIframeTime / 1000000;
        if (*startTime > lastIframeTime || *startTime < firstIframeTime) {
            *startTime = firstIframeTime;
        }

        if (*endTime > lastIframeTime || *endTime < firstIframeTime) {
            *endTime = lastIframeTime;
        }
    }
}

static MF_S32
retr_pos_content_match(MF_S32 fd, struct mf_info *info, struct diskObj * pstDisk, void *pri,
                                struct file_info *pstFile, struct searchTrack *track, EVT_MATCH match)
{
    struct ps_frame_s packet_info;
    struct mf_frame frame;
    struct bkp_pri_t usr;
    MF_S32 res;
    struct PosMsfsPrivate *pPosPrivate = (struct PosMsfsPrivate *)info->stCom.Private;
    Uint8 isTriRec = 0;
    
    if (!pPosPrivate || pPosPrivate->exceptTrunc) {
        return MS_FALSE;
    }
    
    memcpy(&isTriRec, pPosPrivate->reserved + sizeof(MF_PTS), sizeof(Uint8));
    if (!isTriRec) {
        return MS_FALSE;
    }

    if (info->dataOffset < track->offStart || (info->dataOffset + info->dataSize) > track->offEnd) {
        memset(track->buff, 0, MAX_DATA_SIZE);
        res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, track->buff, MAX_DATA_SIZE, pstFile->key.stRecord.fileOffset + info->dataOffset);
        track->fd = fd;
        track->offStart = info->dataOffset;
        track->offEnd = info->dataOffset + res;
    }

    ps_packet_get_info(track->buff + (info->dataOffset - track->offStart), info->dataSize, &packet_info);
    frame.data = retr_mem_calloc(1, packet_info.frame_size);
    if (frame.data == NULL) {
        MFerr("retr mem calloc for frame.data fail!");
        return MF_FALSE;
    }

    ps_packet_decoder(&packet_info, &frame);
    usr.data = &frame;
    usr.size = sizeof(frame);

    if (match(pri, &usr) == MF_FALSE) {
        retr_mem_free(frame.data);
        return MF_FALSE;
    }

    retr_mem_free(frame.data);
    return MF_TRUE;
}

static MF_S32
retr_seg_event_traverse(struct seg_info *seg,
                        INFO_MAJOR_EN enMajor,
                        INFO_MINOR_EN enMinor,
                        DETEC_OBJ_EN enDtc,
                        MF_TIME *pstTime,
                        struct list_head *srcList,
                        struct list_head *list,
                        EVT_MATCH match,
                        void *pri,
                        MF_S32 *pCnt,
                        MF_S32 maxCnt,
                        MF_BOOL needUserData)
{
    struct mf_info *info;
    MF_U32  total;
    MF_S32  i;
    MF_S32  res;
    MF_U32  infoSize = seg->infoStartOffset - seg->infoEndOffset;

    struct file_info *pstFile = seg->owner;
    struct ipc_info *pstIpc = pstFile->owner;
    struct diskObj *pstDisk = pstIpc->owner;
    struct searchTrack track;
    memset(&track, 0, sizeof(struct searchTrack));
    MF_S8 *buff = ms_valloc(infoSize);
    if (!buff) {
        return MF_FAILURE;
    }

    if (enMajor == INFO_MAJOR_SMART && enMinor == INFO_MINOR_POS && match) {
        track.buff = ms_valloc(MAX_DATA_SIZE);
        if (!track.buff) {
            retr_mem_free(buff);
            return MF_FAILURE;
        }
    }

    MF_S32 fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate,
                                             pstFile->key.stRecord.fileNo, MF_YES);
    if (fd <= 0) {
        MFerr("open disk failed");
        retr_mem_free(buff);
        if (track.buff) {
            retr_mem_free(track.buff);
        }
        return MF_FAILURE;
    }

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, buff, infoSize,
                                       pstFile->key.stRecord.fileOffset + seg->infoEndOffset);
    if (res < 0) {
        MFerr("read disk failed");
        pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
        retr_mem_free(buff);
        if (track.buff) {
            retr_mem_free(track.buff);
        }
        return MF_FAILURE;
    }

    total = mf_retr_seg_info_qsort(buff, infoSize, SORT_IS_UP);

#if 0
    MF_S32 index;
#if 1
    index = mf_retr_seg_info_bsearch(buff, infoSize,
                                     (MF_U64)pstTime->startTime * 1000000, enMajor, SORT_IS_UP);
    for (; index >= 0 ; index--) {
        info = (struct mf_info *)(buff + index * INFO_DATA_MAX_SIZE);
        if (info->majorType == enMajor) {
            break;
        }
    }
#else
    for (index = total - 1; index >= 0; index--) {
        info = (struct mf_info *)(buff + index * INFO_DATA_MAX_SIZE);
        if (info->majorType == enMajor) {
            break;
        }
    }
#endif
    total = index;
#endif

    MF_PTS startTime = 0;
    MF_PTS endTime = 0;
    struct src_node_t *src;
    struct bkp_pri_t usr;
    MF_U32  count = 0;

    usr.size = 0;
//    for (i = total; i >= 0; i--)
    for (i = 0; i < total; i++) {
        if (count >= maxCnt) {
            break;
        }
        info = (struct mf_info *)(buff + i * INFO_DATA_MAX_SIZE);
        if (info->majorType == INFO_MAJOR_IFRAME) {
            break;
        }

        if (!(info->majorType & enMajor)) {
            continue;
        }

        if (enMinor != INFO_MINOR_NONE && !(info->minorType & enMinor)) {
            continue;
        }
        
        retr_get_event_info_time(info->majorType, info, &startTime, &endTime);
        retr_check_mark_time(seg, &startTime, &endTime);
        if (startTime > pstTime->endTime || endTime < pstTime->startTime) {
            continue;
        }

        if (enMajor == INFO_MAJOR_SMART && enMinor == INFO_MINOR_POS && match) {
            if (info->dataSize <= 0) {
                continue;
            }

            res = retr_pos_content_match(fd, info, pstDisk, pri, pstFile, &track, match);
            if (res == MF_NO) {
                continue;
            }
        } else  {
            if (info->bEnd != MF_YES) {
                continue;
            }

            if (startTime > pstTime->endTime || endTime < pstTime->startTime) {
                continue;
            }

            if (retr_usr_match(info, match, pri, &usr) == MF_NO) {
                continue;
            }
        }

        if (enDtc != DETEC_ALL && enDtc != DETEC_NONE && (info->stVca.enDtc & enDtc) == 0) {
            continue;
        }
        
        struct item_evt_t *item;
        if (!list_empty(srcList)) {
            list_for_each_entry(src, srcList, node) {
                if (!strcmp(src->srcName, info->srcName)) {
                    item = retr_mem_calloc(1, sizeof(struct item_evt_t));
                    INIT_LIST_HEAD(&item->streamList);
                    INIT_MF_TIME(&item->streamTime);
                    item->port = pstDisk->port;
                    item->chnId = pstIpc->chnId;
                    item->enMajor = info->majorType;
                    item->enMinor = info->minorType;
                    item->objectType = info->stVca.enDtc;
                    strcpy(item->srcName, info->srcName);
                    if (needUserData && usr.size) {
                        item->priSize = usr.size;
                        item->pstPrivate = retr_mem_calloc(1, item->priSize);
                        memcpy(item->pstPrivate, usr.data, item->priSize);
                    }
                    item->time.startTime = MF_MAX(pstTime->startTime, startTime);
                    item->time.endTime = MF_MIN(pstTime->endTime, endTime);                        
                    if (!item->time.endTime && enMajor == INFO_MAJOR_SMART && enMinor == INFO_MINOR_POS) {
                        item->time.endTime = item->time.startTime;
                    }
                    LIST_INSERT_SORT_ASC(item, list, time.startTime);
                    count++;
                }
            }
        } else {
            item = retr_mem_calloc(1, sizeof(struct item_evt_t));
            INIT_LIST_HEAD(&item->streamList);
            INIT_MF_TIME(&item->streamTime);
            item->port = pstDisk->port;
            item->chnId = pstIpc->chnId;
            item->enMajor = info->majorType;
            item->enMinor = info->minorType;
            item->objectType = info->stVca.enDtc;
            strcpy(item->srcName, info->srcName);
            if (needUserData && usr.size) {
                item->priSize = usr.size;
                item->pstPrivate = retr_mem_calloc(1, item->priSize);
                memcpy(item->pstPrivate, usr.data, item->priSize);
            }
            item->time.startTime = MF_MAX(pstTime->startTime, startTime);
            item->time.endTime = MF_MIN(pstTime->endTime, endTime);
            if (!item->time.endTime && enMajor == INFO_MAJOR_SMART && enMinor == INFO_MINOR_POS) {
                item->time.endTime = item->time.startTime;
            }
            LIST_INSERT_SORT_ASC(item, list, time.startTime);
            count++;
        }
    }

    retr_mem_free(buff);
    if (track.buff) {
        retr_mem_free(track.buff);
    }
    pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
    *pCnt = count;
    return MF_SUCCESS;
}

static MF_S32
retr_seg_state_update(struct seg_info *seg, SEG_STATE_EN enState)
{

    if (seg->enState == enState) {
        return MF_SUCCESS;
    }

    struct file_info *file = seg->owner;
    struct ipc_info *ipc = file->owner;
    struct diskObj *pstDisk = ipc->owner;
    struct key_record *keyRec = &file->key.stRecord;
    MF_S32 fd;
    off64_t offset;
    MF_S32 res;

    MF_S8 *cache = ms_valloc(MF_MAX(FILE_HEAD_MAX_SIZE, SEG_HEAD_SIZE));
    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, keyRec->fileNo, MF_YES);


    offset = keyRec->fileOffset + SEG_HEAD_OFFSET +  seg->id * SEG_HEAD_SIZE;
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, cache, SEG_HEAD_SIZE, offset);
    if (res < 0) {
        retr_mem_free(cache);
        disk_close(fd);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    struct mf_segment *Stmp = (struct mf_segment *)cache;
    Stmp->status = enState;
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        fd, cache, SEG_HEAD_SIZE, offset);
    if (res < 0) {
        retr_mem_free(cache);
        disk_close(fd);
        MFerr("write disk failed");
        return MF_FAILURE;
    }

    if (enState == SEG_STATE_INVALID) {
        retr_seg_info_del(seg);
        mf_retr_seg_owner_update(seg);
        retr_seg_info_free(seg);

        mf_retr_file_owner_update(file);
        mf_retr_ipc_owner_update(ipc);
        retr_mem_free(cache);
        disk_close(fd);
        return MF_SUCCESS;
    }
    seg->enState = enState;
    mf_retr_seg_owner_update(seg);
    offset = keyRec->fileOffset + FILE_HEAD_OFFSET;
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, cache, FILE_HEAD_MAX_SIZE, offset);
    if (res < 0) {
        retr_mem_free(cache);
        disk_close(fd);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    struct mf_file *Ftmp = (struct mf_file *)cache;
    MF_U16 lockNum = Ftmp->lockNum;

    if (enState == SEG_STATE_LOCK) {
        Ftmp->lockNum++;
    } else if (enState == SEG_STATE_NORMAL) {
        Ftmp->lockNum--;
    }
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        fd, cache, FILE_HEAD_MAX_SIZE, offset);
    if (res < 0) {
        retr_mem_free(cache);
        disk_close(fd);
        MFerr("write disk failed");
        return MF_FAILURE;
    }
    file->lockNum = Ftmp->lockNum;

    retr_mem_free(cache);
    disk_close(fd);

    mf_retr_file_owner_update(file);
    mf_retr_ipc_owner_update(ipc);

    // update index isLock
    if ((lockNum == 0 && file->lockNum > 0) || (lockNum > 0 && file->lockNum == 0)) {
        mf_retr_update_index_lock(pstDisk, file);
    }

    // if file is unlocked then restart recording
    if (file->lockNum == 0) {
        mf_record_update();
    }

    return MF_SUCCESS;
}

static MF_S32
retr_file_tag_count(struct file_info *file, MF_BOOL isAdd)
{
    struct ipc_info *ipc = file->owner;
    struct diskObj *pstDisk = ipc->owner;
    struct key_record *keyRec = &file->key.stRecord;
    MF_S32 fd;
    off64_t offset;
    MF_S32 res;

    MF_S8 *cache = ms_valloc(FILE_HEAD_MAX_SIZE);
    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, keyRec->fileNo, MF_YES);

    offset = keyRec->fileOffset + FILE_HEAD_OFFSET;
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, cache, FILE_HEAD_MAX_SIZE, offset);
    if (res < 0) {
        retr_mem_free(cache);
        disk_close(fd);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    struct mf_file *Ftmp = (struct mf_file *)cache;

    if (isAdd == MF_YES) {
        Ftmp->tagNum++;
    } else {
        Ftmp->tagNum--;
    }
    file->tagNum = Ftmp->tagNum;
    if (file->tagNum < 0 || file->tagNum > TAG_MAX_NUM) {
        MFerr("tagNum[%d] overflow!\n", file->tagNum);
        retr_mem_free(cache);
        disk_close(fd);
        return MF_FAILURE;
    }

    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        fd, cache, FILE_HEAD_MAX_SIZE, offset);
    if (res < 0) {
        retr_mem_free(cache);
        disk_close(fd);
        MFerr("write disk failed");
        return MF_FAILURE;
    }
    retr_mem_free(cache);
    disk_close(fd);

    return MF_SUCCESS;
}

static MF_BOOL
retr_quota_chn_is_cfg(MF_U8 chnId, FILE_TYPE_EN enType)
{
    struct mf_qta *qta;

    mf_disk_qta_grp_info(NULL, &qta);
    if ((enType >= FILE_TYPE_PIC && qta->cq[chnId].picQta == 0)
        || (enType < FILE_TYPE_PIC && qta->cq[chnId].vidQta == 0)) {
        return MF_NO;
    }

    return MF_YES;
}

static MF_BOOL
retr_quota_chn_is_full(MF_U8 chnId, FILE_TYPE_EN enType)
{
    MF_BOOL ret;
    struct mf_qta *qta = NULL;
    struct chn_qta_t cq;

    mf_disk_qta_grp_info(NULL, &qta);
    cq = qta->cq[chnId];

    if (enType < FILE_TYPE_PIC) {
        ret = cq.vidUsd >= cq.vidQta ? MF_YES : MF_NO;
    } else {
        ret = cq.picUsd >= cq.picQta ? MF_YES : MF_NO;
    }

    return ret;
}

static struct file_info *
retr_quota_find_file(MF_U8 chnId, struct diskObj *disk, FILE_TYPE_EN enType)
{
    if (disk == NULL) {
        return NULL;
    }

    struct ipc_info *ipc = NULL, *n = NULL;
    list_for_each_entry_safe(ipc, n, &disk->ipcHead, node) {
        if (ipc->chnId != chnId || list_empty(&ipc->fileHead)) {
            continue;
        }

        struct file_info *file = NULL, *m = NULL;
        list_for_each_entry_safe(file, m, &ipc->fileHead, node) {
            if ((enType >= FILE_TYPE_PIC && file->key.stRecord.type < FILE_TYPE_PIC)
                || (enType < FILE_TYPE_PIC && file->key.stRecord.type >= FILE_TYPE_PIC)) {
                continue;
            }

            if (file->enLoad == FILE_PRE_LOAD) {
                if (mf_retr_file_head_preload(file) == MF_FAILURE) {
                    continue;
                }
            }

            if (file->bRecording != MF_NO
                || file->lockNum > 0
                || file->refNum > 0
                || file->key.stRecord.isLocked == 1
                || file->key.stRecord.state != FILE_STATE_USED) {
                continue;
            }

            return file;
        }
    }
    return NULL;
}

static struct file_info *
retr_quota_find_over_file(struct diskObj *disk)
{
    MF_U8 chnId;
    struct mf_qta *qta;
    struct chn_qta_t *cq;
    struct file_info *file = NULL;
    struct file_info *tempfile = NULL;

    mf_disk_qta_grp_info(NULL, &qta);
    for (chnId = 0; chnId < MAX_REC_CHN_NUM; chnId++) {
        cq = &qta->cq[chnId];
        if (cq->vidQta > 0 && cq->vidUsd > cq->vidQta) {
            tempfile = retr_quota_find_file(chnId, disk, FILE_TYPE_MAIN);
        } else if (cq->picQta > 0 && cq->picUsd > cq->picQta) {
            tempfile = retr_quota_find_file(chnId, disk, FILE_TYPE_PIC);
        }

        if (!tempfile) {
            continue;
        }
        if (!file) {
            file = tempfile;
            continue;
        }
        if (tempfile->fileStartTime < file->fileStartTime) {
            file = tempfile;
        }
    }
    return file;
}

static struct file_info *
retr_quota_find_nocfg_file(struct diskObj *disk)
{
    MF_U8 chnId;
    struct mf_qta *qta;
    struct chn_qta_t *cq;
    struct file_info *file = NULL;
    struct file_info *tempfile = NULL;

    mf_disk_qta_grp_info(NULL, &qta);
    for (chnId = 0; chnId < MAX_REC_CHN_NUM; chnId++) {
        cq = &qta->cq[chnId];
        if (cq->vidQta == 0) {
            tempfile = retr_quota_find_file(chnId, disk, FILE_TYPE_MAIN);
        } else if (cq->picQta == 0) {
            tempfile = retr_quota_find_file(chnId, disk, FILE_TYPE_PIC);
        }

        if (!tempfile) {
            continue;
        }
        if (!file) {
            file = tempfile;
            continue;
        }
        if (tempfile->fileStartTime < file->fileStartTime) {
            file = tempfile;
        }
    }
    return file;
}



/*************************************************extern API mf_retr_xxxx*************************************/
MF_S32
mf_retr_get_search_lpr_info(MF_S8 *keyWord, EVT_MATCH match,
                            seg_info *seg, MF_TIME *t,
                            struct bkp_pri_t *pri,
                            struct bkp_node_t *pstNode,
                            MF_U32 total)

{
    struct file_info *file = seg->owner;
    struct ipc_info *ipc = file->owner;
    struct diskObj *pstDisk = ipc->owner;
    struct key_record *keyRec = &file->key.stRecord;
    struct mf_info *info = NULL;
    struct bkp_pri_t src;
    MF_U32  infoSize = seg->infoStartOffset - seg->infoEndOffset;
    MF_S8 *buff = ms_valloc(infoSize);
    MF_S32 fd = -1;
    MF_S32 num;
    MF_S32 i;
    MF_S32 res;
    MF_PTS at;

    if (!buff) {
        return MF_FAILURE;
    }

    num = infoSize / INFO_DATA_MAX_SIZE;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, keyRec->fileNo, MF_YES);
    if (fd <= 0) {
        MFerr("open disk failed!");
        retr_mem_free(buff);
        return MF_FAILURE;
    }
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, buff, infoSize,
                                       keyRec->fileOffset + seg->infoEndOffset);
    if (res < 0) {
        MFerr("read disk failed!");
        disk_close(fd);
        retr_mem_free(buff);
        return MF_FAILURE;
    }

    for (i = num - 1; i >= 0; i--) {
        info = (struct mf_info *)(buff + i * INFO_DATA_MAX_SIZE);
        at = info->dataTime / 1000000;
        if (at > t->endTime || at < t->startTime) {
            continue;
        }

        if (!(info->majorType & INFO_MAJOR_SMART)) {
            continue;
        }

        if (!(info->minorType & INFO_MINOR_LPR)) {
            continue;
        }

        if (match) {
            src.data = info->stLpr.Private;
            src.size = info->stLpr.priSize;
            if (match(pri, &src) == MF_NO) {
                continue;
            }
        }

        if (keyWord && strlen(keyWord) != 0 && strcasestr(info->stLpr.Licence, keyWord) == NULL) {
            continue;
        }

        struct item_lpr_t *item = retr_mem_calloc(1, sizeof(struct item_lpr_t));
        if (info->stLpr.priSize) {
            item->pstPrivate = retr_mem_calloc(1, info->stLpr.priSize);
        }

        item->port          = pstDisk->port;
        item->chnId         = keyRec->chnId;
        item->fileNo        = keyRec->fileNo;
        item->priSize       = info->stLpr.priSize;
        item->picOffset     = info->dataOffset;
        item->pts           = info->dataTime;
        item->pstDisk       = pstDisk;
        item->pstIpcInfo    = ipc;
        item->pstFileInfo   = file;
        item->pstSegInfo    = seg;

        item->picMainSize   = info->stLpr.stMainPic.size;
        item->picSubSize    = info->stLpr.stSubPic.size;
        item->picMainWidth  = info->stLpr.stMainPic.width;
        item->picMainHeight = info->stLpr.stMainPic.height;
        item->picSubWidth   = info->stLpr.stSubPic.width;
        item->picSubHeight  = info->stLpr.stSubPic.height;
        memcpy(item->licence, info->stLpr.Licence, MAX_LPR_NAME);
        if (item->pstPrivate) {
            memcpy(item->pstPrivate, info->stLpr.Private, item->priSize);
        }

        LIST_INSERT_SORT_ASC(item, &pstNode->list, pts);
        file->refNum++;
        pstNode->count++;
        if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
            break;
        }
    }
    disk_close(fd);
    retr_mem_free(buff);

    return MF_SUCCESS;
}

MF_S32
mf_retr_get_search_face_info(EVT_MATCH match,
                             seg_info *seg, MF_TIME *t,
                             struct bkp_pri_t *pri,
                             struct bkp_node_t *pstNode,
                             MF_U32 total)

{
    struct file_info *file = seg->owner;
    struct ipc_info *ipc = file->owner;
    struct diskObj *pstDisk = ipc->owner;
    struct key_record *keyRec = &file->key.stRecord;
    struct mf_info *info = NULL;
    struct bkp_pri_t src;
    MF_U32  infoSize = seg->infoStartOffset - seg->infoEndOffset;
    MF_S8 *buff = ms_valloc(infoSize);
    MF_S32 fd = -1;
    MF_S32 num;
    MF_S32 i, j;
    MF_S32 res;
    MF_PTS at;

    if (!buff) {
        return MF_FAILURE;
    }

    num = infoSize / INFO_DATA_MAX_SIZE;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, keyRec->fileNo, MF_YES);
    if (fd <= 0) {
        MFerr("open disk failed!");
        retr_mem_free(buff);
        return MF_FAILURE;
    }
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, buff, infoSize,
                                       keyRec->fileOffset + seg->infoEndOffset);
    if (res < 0) {
        MFerr("read disk failed!");
        disk_close(fd);
        retr_mem_free(buff);
        return MF_FAILURE;
    }

    for (i = num - 1; i >= 0; i--) {
        info = (struct mf_info *)(buff + i * INFO_DATA_MAX_SIZE);
        at = info->dataTime / 1000000;
        if (at > t->endTime || at < t->startTime) {
            continue;
        }

        if (!(info->majorType & INFO_MAJOR_SMART)) {
            continue;
        }

        if (!(info->minorType & INFO_MINOR_FACE)) {
            continue;
        }

        for (j = 0; j < info->stFace.faceNum; j++) {
            if (match) {
                src.data = info->stFace.Private[j];
                src.size = info->stFace.priSize[j];
                if (match(pri, &src) == MF_NO) {
                    continue;
                } else {
                    struct item_face_t *item = retr_mem_calloc(1, sizeof(struct item_face_t));
                    item->port          = pstDisk->port;
                    item->chnId         = keyRec->chnId;
                    item->fileNo        = keyRec->fileNo;
                    item->priSize       = info->stFace.priSize[j];
                    memcpy(item->priData, src.data, src.size);

                    item->pts           = info->dataTime;
                    item->pstDisk       = pstDisk;
                    item->pstIpcInfo    = ipc;
                    item->pstFileInfo   = file;
                    item->pstSegInfo    = seg;

                    item->picMainOffset = info->stFace.stMainPic.offset;
                    item->picMainSize   = info->stFace.stMainPic.size;
                    item->picMainWidth  = info->stFace.stMainPic.width;
                    item->picMainHeight = info->stFace.stMainPic.height;
                    item->picSubOffset  = info->stFace.stSubPic[j].offset;
                    item->picSubSize    = info->stFace.stSubPic[j].size;
                    item->picSubWidth   = info->stFace.stSubPic[j].width;
                    item->picSubHeight  = info->stFace.stSubPic[j].height;

                    LIST_INSERT_SORT_ASC(item, &pstNode->list, pts);
                    file->refNum++;
                    pstNode->count++;
                }
            }
        }

        if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
            break;
        }
    }
    disk_close(fd);
    retr_mem_free(buff);

    return MF_SUCCESS;
}

static MF_S32
mf_retr_get_search_smart_info(seg_info *seg, MF_TIME *t,
                              struct bkp_smart_t *pstSmart,
                              struct bkp_node_t *pstNode,
                              MF_U32 total,
                              void *infoBuff, void *dataBuff)
{
    struct file_info *file = seg->owner;
    struct ipc_info *ipc = file->owner;
    struct diskObj *pstDisk = ipc->owner;
    struct key_record *keyRec = &file->key.stRecord;
    struct mf_info *info = NULL;
    struct bkp_pri_t src;
    struct searchTrack track;
    struct ps_frame_s packet_info;
    struct mf_frame frame;
    MF_U32 infoSize;
    MF_S32 i, num, res;
    MF_S32 fd = -1;
    MF_PTS at;

    memset(&track, 0, sizeof(track));
    track.buff = dataBuff;
    infoSize = seg->infoStartOffset - seg->infoEndOffset;
    if (infoSize > MAX_INFO_SIZE) {
        MFerr("infoSize[%d] over MAX_INFO_SIZE[%d]!", infoSize, MAX_INFO_SIZE);
        return MF_FAILURE;
    }

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, keyRec->fileNo, MF_YES);
    if (fd <= 0) {
        MFerr("open disk failed!");
        return MF_FAILURE;
    }

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, infoBuff, infoSize, keyRec->fileOffset + seg->infoEndOffset);
    if (res < 0) {
        MFerr("read disk failed!");
        disk_close(fd);
        return MF_FAILURE;
    }

    num = infoSize / INFO_DATA_MAX_SIZE;
    for (i = num - 1; i >= 0; i--) {
        info = (struct mf_info *)(infoBuff + i * INFO_DATA_MAX_SIZE);
        at = info->dataTime / 1000000;
        if (at > t->endTime ||
            at < t->startTime ||
            info->dataSize <= 0 ||
            !(info->majorType & INFO_MAJOR_SMART) ||
            !(info->minorType & pstSmart->enMinor)) {
            continue;
        }

        if (pstSmart->info_match) {
            src.data = info->stCom.Private;
            src.size = info->stCom.priSize;
            if (pstSmart->info_match(&pstSmart->infoPri, &src) == MF_NO) {
                continue;
            }
        }
        if (info->dataOffset < track.offStart || (info->dataOffset + info->dataSize) > track.offEnd) {
            memset(track.buff, 0, MAX_DATA_SIZE);
            res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                               fd, track.buff, MAX_DATA_SIZE, keyRec->fileOffset + info->dataOffset);
            track.fd = fd;
            track.offStart = info->dataOffset;
            track.offEnd = info->dataOffset + res;
        }
        /* data is already in track.buff */
        ps_packet_get_info(track.buff + (info->dataOffset - track.offStart), info->dataSize, &packet_info);
        frame.data = retr_mem_calloc(1, packet_info.frame_size);
        if (frame.data == NULL) {
            MFerr("retr mem calloc for frame.data fail!");
            disk_close(fd);
            return MF_FAILURE;
        }

        ps_packet_decoder(&packet_info, &frame);
        src.data = &frame;
        src.size = sizeof(frame);
        if (pstSmart->strm_match) {
            if (pstSmart->strm_match(&pstSmart->strmPri, &src) == MF_NO) {
                retr_mem_free(frame.data);
                continue;
            }
        }

        struct item_smt_t *item = retr_mem_calloc(1, sizeof(struct item_smt_t));
        if (info->stCom.priSize) {
            item->pstPrivate = retr_mem_calloc(1, info->stCom.priSize);
        }

        item->port          = pstDisk->port;
        item->chnId         = keyRec->chnId;
        item->fileNo        = keyRec->fileNo;
        item->priSize       = info->stCom.priSize;
        item->dataOffset    = info->dataOffset;
        item->dataSize      = info->dataSize;
        item->enMinor       = info->minorType;
        item->pts           = info->dataTime;
        item->pstDisk       = pstDisk;
        item->pstIpcInfo    = ipc;
        item->pstFileInfo   = file;
        item->pstSegInfo    = seg;
        item->dataBuff      = frame.data;
        item->frameData     = retr_mem_calloc(1, src.size);
        memcpy(item->frameData, src.data, src.size);
        if (item->pstPrivate) {
            memcpy(item->pstPrivate, info->stCom.Private, info->stCom.priSize);
        }

        LIST_INSERT_SORT_ASC(item, &pstNode->list, pts);
        file->refNum++;
        pstNode->count++;
        if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
            break;
        }
    }

    disk_close(fd);

    return MF_SUCCESS;
}

MF_S32
mf_retr_get_search_tag_info(file_info *file,
                            MF_TIME *t,
                            MF_S8 *keyWord,
                            MF_S8 *out,
                            struct bkp_node_t *pstNode)
{
    struct ipc_info *ipc = file->owner;
    struct diskObj *disk = ipc->owner;
    struct bplus_key *key = &file->key;
    struct mf_tag *tag;
    off64_t offset;
    MF_S32 fd;
    MF_S32 i;
    MF_S32 res;

    offset = key->stRecord.fileOffset + TAG_HEAD_OFFSET;
    fd = disk->ops->disk_file_open(disk->pstPrivate, key->stRecord.fileNo, MF_YES);
    if (fd <= 0) {
        MFerr("open disk failed!");
        return MF_FAILURE;
    }
    res = disk->ops->disk_file_read(disk->pstPrivate,
                                    fd, out, TAG_MAX_NUM * TAG_HEAD_SIZE, offset);
    if (res < 0) {
        MFerr("read disk failed!");
        disk_close(fd);
        return MF_FAILURE;
    }

    struct item_tag_t *item = NULL;
    for (i = 0; i < TAG_MAX_NUM; i++) {
        tag = (mf_tag *)(out + TAG_HEAD_SIZE * i);
        if (tag->bUsed == MF_NO) {
            continue;
        }

        if (tag->time > t->endTime || tag->time < t->startTime) {
            continue;
        }

        //out of date
        if (tag->time > file->fileEndTime || tag->time < file->fileStartTime) {
            continue;
        }

        if (keyWord && strlen(keyWord) != 0 && strstr(tag->name, keyWord) == NULL) {
            continue;
        }

        item = retr_mem_calloc(1, sizeof(struct item_tag_t));
        item->stream = retr_mem_calloc(1, sizeof(struct stream_seg));
        item->chnId = ipc->chnId;
        item->id = i;
        item->time = tag->time;
        strcpy(item->name, tag->name);
        item->stream->pstSegInfo    = NULL;
        item->stream->pstFileInfo   = file;
        item->stream->pstIpcInfo    = file->owner;
        item->stream->pstDisk       = file->owner->owner;
        LIST_INSERT_SORT_ASC(item, &pstNode->list, time);
        file->refNum++;
        pstNode->count++;
    }

    disk_close(fd);
    return MF_SUCCESS;
}

static MF_BOOL match_pushmsg_pic(struct bkp_pri_t pri, struct mf_info *info)
{
    if (!pri.data || pri.size <= 0 || !info || info->stPic.priSize <= 0) {
        return MF_FALSE;
    }

    time_t t1, t2;
    memcpy(&t1, pri.data, sizeof(time_t));
    memcpy(&t2, info->stPic.priData, sizeof(time_t));
    if(t2 < t1 - 1 || t2 > t1) {
        return MF_FALSE;
    }

    return MF_TRUE;
}

MF_S32
mf_retr_get_search_pic_info(seg_info *seg, MF_TIME *t,
                            INFO_MAJOR_EN enMajor,
                            INFO_MINOR_EN enMinor,
                            struct bkp_node_t *pstNode,
                            MF_U32 total,
                            struct bkp_pri_t pri,
                            struct bkp_vca_t *vcaPrivate, MF_U32 maxNum)
{
    struct file_info *file = seg->owner;
    struct ipc_info *ipc = file->owner;
    struct diskObj *pstDisk = ipc->owner;
    struct key_record *keyRec = &file->key.stRecord;
    struct mf_info *info = NULL;
    MF_U32  infoSize = seg->infoStartOffset - seg->infoEndOffset;
    MF_S8 *buff = ms_valloc(infoSize);
    MF_S32 fd = -1;
    MF_S32 num;
    MF_S32 i;
    MF_S32 res;
    MF_U32 objectType = 0;

    num = infoSize / INFO_DATA_MAX_SIZE;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, keyRec->fileNo, MF_YES);
    if (fd <= 0) {
        MFerr("open disk failed!");
        retr_mem_free(buff);
        return MF_FAILURE;
    }
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, buff, infoSize,
                                       keyRec->fileOffset + seg->infoEndOffset);
    if (res < 0) {
        MFerr("read disk failed!");
        retr_mem_free(buff);
        disk_close(fd);
        return MF_FAILURE;
    }

    for (i = num - 1; i >= 0; i--) {
        info = (struct mf_info *)(buff + i * INFO_DATA_MAX_SIZE);
        if (info->stPic.time > t->endTime || info->stPic.time < t->startTime) {
            continue;
        }

        if (!(info->majorType & enMajor)) {
            continue;
        }

        if (info->stPic.size == 0) {
            continue;
        }

        if (enMinor != INFO_MINOR_NONE && !(info->minorType & enMinor)) {
            continue;
        }

        if (enMajor == INFO_MAJOR_PUSHMSG_PIC
            && !match_pushmsg_pic(pri, info)) {
            continue;
        }
        //msprintf("gsjt enMinor:%d", enMinor);
        if (info->stPic.priSize) {
            struct MsVcaPrivate* vcaData = (struct MsVcaPrivate*)(info->stPic.priData);
            objectType = vcaData->dec;
            if (enMinor & INFO_MINOR_PIC_VCA) {
                if (vcaPrivate->event != MAX_SMART_EVENT && vcaPrivate->event != vcaData->event) {
                    continue;
                }
                if (vcaPrivate->dec != DETEC_ALL && vcaPrivate->dec != vcaData->dec) {
                    continue;
                }
            }
        }

        struct item_pic_t *item = retr_mem_calloc(1, sizeof(struct item_pic_t));
        item->stream = retr_mem_calloc(1, sizeof(struct stream_seg));
        item->chnId = keyRec->chnId;
        item->port = pstDisk->port;
        item->size = info->stPic.size;
        item->width = info->stPic.width;
        item->height = info->stPic.height;
        item->enMajor = info->majorType;
        item->enMinor = info->minorType;
        item->pts = info->dataTime;
        item->objectType = objectType;

        item->stream->baseOffset = keyRec->fileOffset;
        item->stream->pstDisk = pstDisk;
        item->stream->pstIpcInfo = ipc;
        item->stream->pstFileInfo = file;
        item->stream->pstSegInfo = seg;
        item->stream->fileNo = keyRec->fileNo;
        item->stream->firstIframeOffset = info->dataOffset;
        item->stream->firstIframeTime = info->dataTime;
        item->stream->lastIframeOffset = info->dataOffset;
        item->stream->lastIframeTime = info->dataTime;
        item->stream->startTime = info->stPic.time;
        item->stream->endTime = info->stPic.time;
        item->stream->chnid = keyRec->chnId;
        item->stream->type = keyRec->type;
        item->stream->size = info->stPic.size;
        item->stream->encodeType = info->stPic.encode;
        item->stream->resWidth = info->stPic.width;
        item->stream->resHeight = info->stPic.height;
        item->stream->infoEndOffset = info->selfOffset;
        item->stream->infoStartOffset = info->selfOffset + INFO_DATA_MAX_SIZE;
        item->stream->recStartOffset = info->dataOffset;
        item->stream->recEndOffset = info->dataOffset + info->stPic.size;

        LIST_INSERT_SORT_ASC(item, &pstNode->list, pts);
        file->refNum++;
        pstNode->count++;
        pstNode->size += item->size;
        pstNode->range.startTime = MF_MIN(pstNode->range.startTime, item->stream->startTime);
        pstNode->range.endTime = MF_MAX(pstNode->range.endTime, item->stream->endTime);
        if (total + pstNode->count >= maxNum) {
            break;
        }
    }
    disk_close(fd);
    retr_mem_free(buff);

    return MF_SUCCESS;
}

MF_S32
mf_retr_get_search_seg_info(stream_seg *s, seg_info *seg, MF_TIME *t)
{
    struct mf_info *info;
    MF_S32 res = MF_SUCCESS;
    MF_U8 Ts = 0;
    MF_U8 Te = 0;
    MF_U32  infoSize = seg->infoStartOffset - seg->infoEndOffset;
    MF_U64 interval = 1800000000;//30min --> us;

    if ((seg->segStartTime > seg->segEndTime)
        || (seg->lastIframeTime > seg->lastFrameTime)
        || ((seg->lastFrameTime - seg->lastIframeTime) >= interval)) {
        /*
            14-r8补充I帧范围判断，因修改时间写入音频帧导致的片段异常问题兼容，若干版本后可删除
            【Dawn-日本-NBC：NVR修改时间后再改回正确的时间，部分通道录像异常】
            https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001072529
        */
        s->startTime            = seg->firstIframeTime / 1000000;
        s->endTime              = seg->lastIframeTime / 1000000;
    } else {
        s->startTime            = seg->segStartTime;
        s->endTime              = seg->segEndTime;
    }
    s->infoStartOffset      = seg->infoStartOffset;
    s->infoEndOffset        = seg->infoEndOffset;
    s->recStartOffset       = seg->recStartOffset;
    s->recEndOffset         = seg->recEndOffset;
    s->firstIframeTime      = seg->firstIframeTime;
    s->firstFrameTime       = seg->firstFrameTime;
    s->firstIframeOffset    = seg->firstIframeOffset;
    s->lastFrameTime        = seg->lastFrameTime;
    s->lastIframeTime       = seg->lastIframeTime;
    s->lastIframeOffset     = seg->lastIframeOffset;
    s->encodeType           = seg->encodeType;
    s->resWidth             = seg->resWidth;
    s->resHeight            = seg->resHeight;
    s->enEvent              = seg->enEvent;
    s->enState              = seg->enState;

    if (t->startTime > s->startTime) {
        s->startTime = t->startTime;
        Ts = 1;
    }
    if (t->endTime < s->endTime) {
        s->endTime = t->endTime;
        Te = 1;
    }
//    MFinfo("startTime[%s]", mf_time_to_string(s->startTime));
//    MFinfo("endTime[%s]", mf_time_to_string(s->endTime));
    if (Ts || Te) {
        MF_S32 index;
        struct diskObj *pstDisk = s->pstDisk;
        MF_S8 *buff = ms_valloc(infoSize);
        MF_S32 fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, s->fileNo, MF_YES);

        if (fd <= 0) {
            MFerr("open disk failed!");
            retr_mem_free(buff);
            return MF_FAILURE;
        }
        res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                           fd, buff, infoSize, s->baseOffset + seg->infoEndOffset);
        if (res < 0) {
            MFerr("read disk failed!");
            retr_mem_free(buff);
            pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
            return MF_FAILURE;
        }
        mf_retr_seg_info_qsort(buff, infoSize, SORT_IS_UP);
        if (Ts) {
            index = mf_retr_seg_info_bsearch(buff, infoSize,
                                             (MF_U64)s->startTime * 1000000, INFO_MAJOR_IFRAME, SORT_IS_UP);
            info = (struct mf_info *)(buff + index * INFO_DATA_MAX_SIZE);
            if (info->majorType != INFO_MAJOR_IFRAME) {
                retr_mem_free(buff);
                pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
                return MF_FAILURE;
            }
            s->infoStartOffset      = info->selfOffset + INFO_DATA_MAX_SIZE;
            s->recStartOffset       = info->dataOffset;
            s->firstIframeTime      = info->dataTime;
            s->firstIframeOffset    = info->dataOffset;
            s->firstIframeTime      = s->firstIframeTime;
        }
        if (Te) {
            index = mf_retr_seg_info_bsearch(buff, infoSize,
                                             (MF_U64)s->endTime * 1000000, INFO_MAJOR_IFRAME, SORT_IS_UP);
            info = (struct mf_info *)(buff + index * INFO_DATA_MAX_SIZE);
            if (info->majorType != INFO_MAJOR_IFRAME) {
                retr_mem_free(buff);
                pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
                return MF_FAILURE;
            }
            s->infoEndOffset    = info->selfOffset;
            s->recEndOffset     = info->dataOffset + info->stIframe.size;
            s->lastIframeTime   = info->dataTime;
            s->lastIframeOffset = info->dataOffset;
            s->lastFrameTime    = s->lastIframeTime;
        }
        retr_mem_free(buff);
        pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
    }

    if (s->recEndOffset <= s->recStartOffset) {
        return MF_FAILURE;
    }
    s->size = s->recEndOffset - s->recStartOffset;
    return MF_SUCCESS;
}

MF_U32
mf_retr_seg_info_qsort(void *buff, MF_U32 len, MF_BOOL isUp)
{
    MF_U32 num = len / INFO_DATA_MAX_SIZE;

//    struct mf_info *info = NULL;
//    MF_S32 i;

//    for (i = 0; i < num; i++)
//    {
//        struct mf_info *info = (struct mf_info *)(buff + i * INFO_DATA_MAX_SIZE);
//        MFprint("[%d] type[%d], time[%lld] before\n", i, info->majorType, info->dataTime);
//    }

    if (isUp) {
        qsort(buff, num, INFO_DATA_MAX_SIZE, retr_cmp_up);
    } else {
        qsort(buff, num, INFO_DATA_MAX_SIZE, retr_cmp_down);
    }

//    for (i = 0; i < num; i++)
//    {
//        struct mf_info *info = (struct mf_info *)(buff + i * INFO_DATA_MAX_SIZE);
//        MFprint("[%d] type[%d], time[%lld] after\n", i, info->majorType, info->dataTime);
//    }

    return num;
}

MF_S32
mf_retr_seg_info_bsearch(MF_S8 *buff, MF_U32 len, MF_U64 timeUs, MF_U16 type, MF_BOOL isUp)
{
    MF_S32 low = 0;
    MF_S32 high = len / INFO_DATA_MAX_SIZE - 1;
    MF_S32 mid = 0;
    struct mf_info *info = NULL;

//    MFprint("====time[%lld]====high = %d\n", timeUs, high);
    while (low <= high) {
        mid = (low + high) / 2;
        info = (struct mf_info *)(buff + mid * INFO_DATA_MAX_SIZE);
        if (retr_cmp_bsearch(info, timeUs, type) < 0) {
//            MFprint("=====000======mid= %d\n", mid);
            if (isUp) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        } else if (retr_cmp_bsearch(info, timeUs, type) > 0) {
//            MFprint("====1111=======mid= %d\n", mid);
            if (isUp) {
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        } else {
            break;
        }
    }

    if (info && info->majorType != type) {
        mid = isUp ? mid - 1 : mid + 1;
    }

    return mid;
}

MF_S32
mf_retr_disk_info_add(struct diskObj *pstDisk)
{
    MF_U8 i;
    MF_S32 percent = 0;

    MFinfo("port[%d]_add ready", pstDisk->port);

    if (pstDisk->bRec == MF_NO) {
        return MF_FAILURE;
    }
    if (pstDisk->enState != DISK_STATE_NORMAL) {
        return MF_FAILURE;
    }
    if (!pstDisk->tree) {
        return MF_FAILURE;
    }
//    mf_index_dump(pstDisk->tree);
    MFinfo("port[%d]_add begin", pstDisk->port);
    pstDisk->startTime = INIT_START_TIME;
    pstDisk->endTime = INIT_END_TIME;

    ms_mutex_lock(&pstDisk->mutex);
    for (i = 0; i < MAX_REC_CHN_NUM; i++) {
        if (pstDisk->bExit == MF_YES) {
            ms_mutex_unlock(&pstDisk->mutex);
            pstDisk->bRetr = MF_NO;
            if (percent > 0) {
                mf_disk_notify_bar(PROGRESS_BAR_DISK_LOADING, pstDisk->port, 100, pstDisk->enType);
            }
            return MF_FAILURE;
        }
        percent = mf_random(100 * i / MAX_REC_CHN_NUM, 100 * (i + 1) / MAX_REC_CHN_NUM - 1);
        mf_disk_notify_bar(PROGRESS_BAR_DISK_LOADING, pstDisk->port, percent, pstDisk->enType);
        struct ipc_info *pstIpcInfo = retr_ipc_info_alloc(i);
        if (retr_ipc_info_add(pstIpcInfo, pstDisk) == MF_FAILURE) {
            retr_ipc_info_free(pstIpcInfo);
        } else {
            mf_retr_ipc_owner_update(pstIpcInfo);
        }
    }
    mf_disk_notify_bar(PROGRESS_BAR_DISK_LOADING, pstDisk->port, 100, pstDisk->enType);
    pstDisk->bRetr = MF_YES;
    MFdbg("disk[%d] retr add\n", pstDisk->port);
    ms_mutex_unlock(&pstDisk->mutex);
    MFinfo("port[%d]_add end", pstDisk->port);
    return MF_SUCCESS;
}

void
mf_retr_disk_info_del(struct diskObj *pstDisk)
{
    struct ipc_info *pos = NULL;
    struct ipc_info *n = NULL;

    ms_mutex_lock(&pstDisk->opsMutex);
    list_for_each_entry_safe(pos, n, &pstDisk->ipcHead, node) {
        retr_ipc_info_del(pos);
        mf_retr_ipc_owner_update(pos);
        retr_ipc_info_free(pos);
    }
    pstDisk->bRetr = MF_NO;
    MFdbg("disk[%d] retr delete\n", pstDisk->port);
    ms_mutex_unlock(&pstDisk->opsMutex);
}

void
mf_retr_disk_info_dump(struct diskObj *pstDisk, MF_U8 chnid)
{
    struct ipc_info *pos = NULL;
    struct ipc_info *n = NULL;
    MF_S8 startTime [32];
    MF_S8 endTime [32];

    if (!pstDisk->tree) {
        return;
    }
    ms_mutex_lock(&pstDisk->mutex);
    time_to_string_local(pstDisk->startTime, startTime);
    time_to_string_local(pstDisk->endTime, endTime);

    MFprint("diskInfo:\n");
    MFprint("name: %s\n", pstDisk->name);
    MFprint("time: %s -- %s\n", startTime, endTime);
    MFprint("ipcNum: %d\n", pstDisk->ipcNum);
    MFprint("fileNum: %d\n", pstDisk->fileNum);
    if (pstDisk->pstFirstUsed)
        MFprint("firstUsed: %d-%llx\n", pstDisk->pstFirstUsed->key.stRecord.fileNo,
                pstDisk->pstFirstUsed->key.stRecord.fileOffset);
    list_for_each_entry_safe(pos, n, &pstDisk->ipcHead, node) {
        if (chnid == ALL_CHNID || pos->chnId == chnid) {
            retr_ipc_info_dump(pos);
        }
    }
    ms_mutex_unlock(&pstDisk->mutex);
}
#if 0

void
mf_retr_disk_find_firstUsed_key(struct diskObj *pstDisk)
{
    struct ipc_info *pos, *n;
    MF_PTS tmp = INIT_START_TIME;

    list_for_each_entry_safe(pos, n, &pstDisk->ipcHead, node) {
        if (!pos->pstFirstUsed) {
            continue;
        }
        if (pos->pstFirstUsed->key.stRecord.startTime < tmp) {
            tmp = pos->pstFirstUsed->key.stRecord.startTime;
            pstDisk->pstFirstUsed = pos->pstFirstUsed;
        }
    }
}
#endif

MF_S32
mf_retr_fetch_disk_details()
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *head = mf_disk_get_rd_list(MF_NO);

    list_for_each_entry_safe(pos, n, head, node) {
        mf_retr_disk_info_add(pos);
    }
    mf_disk_put_rw_list(head);

    return  MF_SUCCESS;
}

MF_S32
mf_retr_release_disk_details()
{
    struct diskObj *pos = NULL;
    struct diskObj *n = NULL;
    struct list_head *head = mf_disk_get_rd_list(MF_NO);

    list_for_each_entry_safe(pos, n, head, node) {
        mf_retr_disk_info_del(pos);
        //this dont ms_free(pos) , pos maybe used by others!!!!
    }
    mf_disk_put_rw_list(head);

    return  MF_SUCCESS;
}

MF_S32
mf_retr_ipc_owner_update(struct ipc_info *pstIpcInfo)
{
    MF_PTS tmp = INIT_START_TIME;
    struct ipc_info *pos = NULL;
    struct ipc_info *n = NULL;
    struct diskObj *pstDisk = pstIpcInfo->owner;

    ms_mutex_lock(&pstDisk->infoMutex);
    pstDisk->startTime = INIT_START_TIME;
    pstDisk->endTime = INIT_END_TIME;
    pstDisk->fileNum = 0;
    pstDisk->pstFirstUsed = NULL;
    pstDisk->streamType = 0;
    pstDisk->recType = 0;
    pstDisk->majorType = 0;
    pstDisk->minorType = 0;
    pstDisk->recSubType = 0;
    pstDisk->majorSubType = 0;
    pstDisk->minorSubType = 0;
    list_for_each_entry_safe(pos, n, &pstDisk->ipcHead, node) {
        pstDisk->fileNum += pos->fileNum;
        pstDisk->startTime = MF_MIN(pstDisk->startTime, pos->ipcStartTime);
        pstDisk->endTime = MF_MAX(pstDisk->endTime, pos->ipcEndTime);
        pstDisk->streamType |= pos->streamType;
        pstDisk->recType |= pos->recType;
        pstDisk->majorType |= pos->majorType;
        pstDisk->minorType |= pos->minorType;
        pstDisk->recSubType |= pos->recSubType;
        pstDisk->majorSubType |= pos->majorSubType;
        pstDisk->minorSubType |= pos->minorSubType;
        //find the first used-file with same disk
        if (!pos->pstFirstUsed) {
            continue;
        }
        if (pos->pstFirstUsed->key.stRecord.endTime < tmp) {
            tmp = pos->pstFirstUsed->key.stRecord.endTime;
            pstDisk->pstFirstUsed = pos->pstFirstUsed;
//            MFprint("=======disk used = %d\n", pos->pstFirstUsed->key.stRecord.fileNo);
        }
    }
    if (pstDisk->enType != DISK_TYPE_NAS && pstDisk->enType != DISK_TYPE_CIFS) {
        pstDisk->free = mf_retr_disk_free_size(pstDisk);
    }
    ms_mutex_unlock(&pstDisk->infoMutex);

    return MF_SUCCESS;
}

MF_S32
mf_retr_file_owner_update(struct file_info *pstFileInfo)
{
    struct ipc_info *pstIpcInfo = pstFileInfo->owner;
    struct diskObj *pstDisk = pstIpcInfo->owner;

    ms_mutex_lock(&pstDisk->infoMutex);

    pstIpcInfo->ipcStartTime = INIT_START_TIME;
    pstIpcInfo->ipcEndTime = INIT_END_TIME;
    pstIpcInfo->mainStartTime = INIT_START_TIME;
    pstIpcInfo->mainEndTime = INIT_END_TIME;
    pstIpcInfo->subStartTime = INIT_START_TIME;
    pstIpcInfo->subEndTime = INIT_END_TIME;
    pstIpcInfo->picStartTime = INIT_START_TIME;
    pstIpcInfo->picEndTime = INIT_END_TIME;
    pstIpcInfo->pstFirstUsed = NULL;
    pstIpcInfo->streamType = 0;
    pstIpcInfo->recType = 0;
    pstIpcInfo->majorType = 0;
    pstIpcInfo->minorType = 0;
    pstIpcInfo->recSubType = 0;
    pstIpcInfo->majorSubType = 0;
    pstIpcInfo->minorSubType = 0;
    if (!list_empty(&pstIpcInfo->fileHead)) {
        struct file_info *pos;
        struct file_info *n = NULL;
        MF_PTS tmp = INIT_START_TIME;
//        //the fisrt element of list is oldest
//        pos = list_entry(pstIpcInfo->fileHead.next, struct file_info, node);
        list_for_each_entry_safe(pos, n, &pstIpcInfo->fileHead, node) {
            pstIpcInfo->ipcStartTime = MF_MIN(pstIpcInfo->ipcStartTime, pos->fileStartTime);
            pstIpcInfo->ipcEndTime = MF_MAX(pstIpcInfo->ipcEndTime, pos->fileEndTime);
            pstIpcInfo->streamType |= 1<<pos->key.stRecord.type;
            if (pos->key.stRecord.type == FILE_TYPE_MAIN) {
                pstIpcInfo->mainStartTime = MF_MIN(pstIpcInfo->mainStartTime, pos->fileStartTime);
                pstIpcInfo->mainEndTime = MF_MAX(pstIpcInfo->mainEndTime, pos->fileEndTime);
                if (pos->key.stRecord.state == FILE_STATE_USING) {
                    pstIpcInfo->recType |= pos->enEvent;
                    pstIpcInfo->majorType |= pos->enMajor;
                    pstIpcInfo->minorType |= pos->enMinor;
                } else {
                    pstIpcInfo->recType |= pos->key.stRecord.event;
                    pstIpcInfo->majorType |= pos->key.stRecord.infoType;
                    pstIpcInfo->minorType |= pos->key.stRecord.infoType;
                }
            } else if (pos->key.stRecord.type == FILE_TYPE_SUB) {
                pstIpcInfo->subStartTime = MF_MIN(pstIpcInfo->subStartTime, pos->fileStartTime);
                pstIpcInfo->subEndTime = MF_MAX(pstIpcInfo->subEndTime, pos->fileEndTime);
                if (pos->key.stRecord.state == FILE_STATE_USING) {
                    pstIpcInfo->recSubType |= pos->enEvent;
                    pstIpcInfo->majorSubType |= pos->enMajor;
                    pstIpcInfo->minorSubType |= pos->enMinor;
                } else {
                    pstIpcInfo->recSubType |= pos->key.stRecord.event;
                    pstIpcInfo->majorSubType |= pos->key.stRecord.infoType;
                    pstIpcInfo->minorSubType |= pos->key.stRecord.infoType;
                }
            } else if (pos->key.stRecord.type == FILE_TYPE_PIC) {
                pstIpcInfo->picStartTime = MF_MIN(pstIpcInfo->picStartTime, pos->fileStartTime);
                pstIpcInfo->picEndTime = MF_MAX(pstIpcInfo->picEndTime, pos->fileEndTime);
                /*
                lpr&face是以图片的方式存储
                【Winni-阿尔巴尼亚-ALComm：17-r4版本NVR，ANPR Smart Search 搜索不到 LPR 日志】
                https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001099805
                */
                if (pos->key.stRecord.state == FILE_STATE_USING) {
                    pstIpcInfo->majorType |= pos->enMajor;
                    pstIpcInfo->minorType |= pos->enMinor;
                } else {
                    pstIpcInfo->majorType |= pos->key.stRecord.infoType;
                    pstIpcInfo->minorType |= pos->key.stRecord.infoType;
                }
            }
            //find the first used-file whth same chnId
            if (pos->key.stRecord.state == FILE_STATE_USED
                && pos->lockNum == 0) {
                if (tmp > pos->fileEndTime) {
                    tmp = pos->fileEndTime;
                    pstIpcInfo->pstFirstUsed = pos;
                }
            }
        }
    }
    ms_mutex_unlock(&pstDisk->infoMutex);

    return MF_SUCCESS;
}

MF_S32
mf_retr_seg_owner_update(struct seg_info *pstSegInfo)
{
    struct file_info *pstFileInfo = pstSegInfo->owner;
    struct seg_info *tmp = NULL;

    pstFileInfo->fileStartTime  = INIT_START_TIME;
    pstFileInfo->fileEndTime    = INIT_END_TIME;
    pstFileInfo->recStartOffset = 0;
    pstFileInfo->recEndOffset   = 0;
    pstFileInfo->enEvent = 0;
    pstFileInfo->enMajor = 0;
    pstFileInfo->enMinor = 0;
    pstFileInfo->enState = 0;

    if (!list_empty(&pstFileInfo->segHead)) {
        list_for_each_entry(tmp, &pstFileInfo->segHead, node) {
            pstFileInfo->fileStartTime = MF_MIN(pstFileInfo->fileStartTime, tmp->segStartTime);
            pstFileInfo->fileEndTime = MF_MAX(pstFileInfo->fileEndTime, tmp->segEndTime);

            pstFileInfo->recStartOffset = MF_MIN(pstFileInfo->recStartOffset, tmp->recStartOffset);
            pstFileInfo->recEndOffset = MF_MAX(pstFileInfo->recEndOffset, tmp->recEndOffset);
            pstFileInfo->enEvent |= tmp->enEvent;
            pstFileInfo->enMajor |= tmp->enMajor;
            pstFileInfo->enMinor |= tmp->enMinor;
            pstFileInfo->enState |= tmp->enState;
        }
    }

    return MF_SUCCESS;
}

void
mf_retr_file_info_update(struct file_info *pstFileInfo)
{
    struct ipc_info *pstIpcInfo = pstFileInfo->owner;
    MF_S32 res;
    //update segment info
    ms_mutex_lock(&pstIpcInfo->mutex);
    retr_seg_info_del(pstFileInfo->pstSegUsing);
    res = retr_seg_info_add(pstFileInfo->pstSegUsing, pstFileInfo);
    //update owner info
    if (res == MF_SUCCESS) {
        mf_retr_seg_owner_update(pstFileInfo->pstSegUsing);
        mf_retr_file_owner_update(pstFileInfo);
        mf_retr_ipc_owner_update(pstIpcInfo);
    }
    ms_mutex_unlock(&pstIpcInfo->mutex);
}

struct file_info *
mf_retr_disk_find_using_file(struct diskObj *pstDisk,
                             MF_U8 chnId, MF_U8 type,
                             FILE_STATE_EN enState)
{
    struct ipc_info *pstIpcInfo = NULL;
    struct file_info *pos = NULL;
    struct file_info *n = NULL;

    IPCINFO_FIND_BY_ID(chnId, pstIpcInfo, &pstDisk->ipcHead);
    if (pstIpcInfo) {
        list_for_each_entry_safe_reverse(pos, n, &pstIpcInfo->fileHead, node) {
            if (pos->key.stRecord.state == enState
                && pos->key.stRecord.type == type) {
                return pos;
            }
        }
        return NULL;
    }
    return NULL;
}

struct file_info *
mf_retr_disk_find_anr_file(struct diskObj *pstDisk,
                           MF_U8 chnId, MF_U8 type)
{
    struct ipc_info *pstIpcInfo = NULL;
    struct file_info *pos = NULL;
    struct file_info *n = NULL;

    IPCINFO_FIND_BY_ID(chnId, pstIpcInfo, &pstDisk->ipcHead);
    if (pstIpcInfo) {
        list_for_each_entry_safe_reverse(pos, n, &pstIpcInfo->fileHead, node) {
            if (pos->key.stRecord.state == FILE_STATE_ANR
                && pos->key.stRecord.type == type) {
                return pos;
            }
        }
        return NULL;
    }
    return NULL;
}

struct file_info *
mf_retr_disk_find_used_file(struct diskObj *pstDisk)
{
    if (pstDisk->bLoop == MF_NO) {
        MFdbg("pstDisk->bLoop == MF_NO");
        return NULL;
    } else {
        MF_U32 Try = 0;
        MF_S32 res;
        while (pstDisk->pstFirstUsed) {
            if (pstDisk->pstFirstUsed->enLoad == FILE_PRE_LOAD) {
                res = retr_file_info_seg_load(pstDisk->pstFirstUsed);
                MFshow("[%d] disk[%d] file is segment loading...res[%d]", Try++, pstDisk->port, res);
            } else {
                break;
            }
        }
        return pstDisk->pstFirstUsed;
    }
}

struct file_info *
mf_retr_disk_find_empty_file(struct diskObj *pstDisk,
                             MF_U8 chnId, FILE_TYPE_EN enType)
{
    struct bplus_result stResult;
    struct file_info *pstFileInfo = NULL;
    struct mf_qta *qta = NULL;


    if ((pstDisk->enType == DISK_TYPE_NAS || pstDisk->enType == DISK_TYPE_CIFS) && pstDisk->ops->disk_size)
        if (pstDisk->ops->disk_size(pstDisk->pstPrivate) == MF_FAILURE) {
            MFerr("chn[%d] get size failed", chnId);
            return NULL;
        }

    if (pstDisk->free < FILE_PER_SIZE) {
        MFdbg("pstDisk->free < FILE_PER_SIZE\n");
        return NULL;
    }

    mf_disk_qta_grp_info(NULL, &qta);
    if (qta->bEnable == MF_YES
        && retr_quota_chn_is_cfg(chnId, enType) == MF_YES
        && retr_quota_chn_is_full(chnId, enType) == MF_YES) {
        MFdbg("chn[%d][%d] quota is Full\n", chnId, enType);
        return NULL;
    }

    if (mf_index_find_empty(pstDisk->tree, &stResult) == MF_NO) {
        MFdbg("mf_index_find_empty == MF_NO\n");
        return NULL;
    }

    struct ipc_info *pstIpcInfo = NULL;
    IPCINFO_FIND_BY_ID(chnId, pstIpcInfo, &pstDisk->ipcHead);
    if (!pstIpcInfo) {
        pstIpcInfo = retr_ipc_info_alloc(chnId);
        pstIpcInfo->owner = pstDisk;
        LIST_INSERT_SORT_ASC(pstIpcInfo, &pstDisk->ipcHead, chnId);
        pstDisk->ipcNum++;
    }
    ms_mutex_lock(&pstIpcInfo->mutex);
    pstFileInfo = retr_file_info_alloc(&stResult.key);
    LIST_INSERT_SORT_ASC(pstFileInfo, &pstIpcInfo->fileHead, fileStartTime);
    pstFileInfo->owner = pstIpcInfo;
    pstIpcInfo->fileNum++;
    ms_mutex_unlock(&pstIpcInfo->mutex);

    return pstFileInfo;
}

MF_U64
mf_retr_disk_free_size(struct diskObj *pstDisk)
{
    MF_S32 fileBad = 0;
    MF_S32 total = 0;

    if (pstDisk->tree) {
        fileBad = pstDisk->tree->fileBad;
        total = pstDisk->tree->total;
    } else {
        total = pstDisk->pstHeader->fileCount;
    }

    if (total - fileBad < pstDisk->fileNum) {
        return 0;
    } else
        return pstDisk->pstHeader->perFileSize * \
               (total - pstDisk->fileNum - fileBad);
}

MF_S32
mf_retr_file_delete(struct file_info *pstFileInfo, FILE_CONDITION_EN enCond)
{
    if (pstFileInfo == NULL) {
        return MF_FAILURE;
    }

    struct ipc_info *pstIpcInfo = pstFileInfo->owner;
    struct diskObj *pstDiskInfo = pstIpcInfo->owner;
    struct bplus_key stKey;
    struct bplus_data stData;
    MF_S32 res = MF_FAILURE;

    ms_mutex_lock(&pstIpcInfo->mutex);

    if (mf_index_key_delete(pstDiskInfo->tree, &pstFileInfo->key) == MF_SUCCESS) {

        if (enCond == FILE_CONDITION_GOOD) {
            mf_index_key_init(&stKey, TYPE_RECORD);
            stKey.stRecord.condition = enCond;
            stKey.stRecord.fileNo = pstFileInfo->key.stRecord.fileNo;
            stKey.stRecord.fileOffset = pstFileInfo->key.stRecord.fileOffset;
        } else {
            stKey = pstFileInfo->key;
            stKey.stRecord.startTime = pstFileInfo->fileStartTime;
            stKey.stRecord.endTime = pstFileInfo->fileEndTime;
            stKey.stRecord.condition = enCond;
        }

        stData.stRecord.fileOffset = stKey.stRecord.fileOffset;
        res = mf_index_key_insert(pstDiskInfo->tree, &stKey, &stData);
        if (res == MF_SUCCESS) {
            MFdbg("@@@@@@@@@@@@@ start delete !");
            mf_index_key_print(&pstFileInfo->key);
            mf_index_key_backup(pstDiskInfo->tree);
            retr_file_info_del(pstFileInfo);
            mf_retr_file_owner_update(pstFileInfo);
            mf_retr_ipc_owner_update(pstIpcInfo);
            retr_file_info_free(pstFileInfo);
            MFdbg("@@@@@@@@@@@@@ end delete !");
        } else {
            mf_index_key_recover(pstDiskInfo->tree);
        }
//        mf_index_data_init(pstDiskInfo->tree, &stKey);
    }

    ms_mutex_unlock(&pstIpcInfo->mutex);

    return res;
}

struct seg_info *
mf_retr_seg_info_new(struct file_info *pstFileInfo)
{
    struct seg_info *pstSegInfo = NULL;

    if (!list_empty(&pstFileInfo->segHead)) {
        pstSegInfo = list_entry(pstFileInfo->segHead.prev, struct seg_info, node);
        if (pstSegInfo->segEndTime == INIT_END_TIME) {
            return pstSegInfo;
        } else {
            pstSegInfo = retr_seg_info_alloc();
            retr_seg_info_add(pstSegInfo, pstFileInfo);
        }
    } else {
        pstSegInfo = retr_seg_info_alloc();
        retr_seg_info_add(pstSegInfo, pstFileInfo);
    }

    return pstSegInfo;
}

void
mf_retr_file_info_delete(struct file_info *pstFileInfo)
{
    struct ipc_info *pstIpcInfo = pstFileInfo->owner;

    ms_mutex_lock(&pstIpcInfo->mutex);
    retr_file_info_del(pstFileInfo);
    //update owner info
    mf_retr_file_owner_update(pstFileInfo);
    mf_retr_ipc_owner_update(pstIpcInfo);
    ms_mutex_unlock(&pstIpcInfo->mutex);
}

struct file_info *
mf_retr_file_info_insert(struct file_info *pstFileInfo)
{
    struct ipc_info *pstIpcInfo = NULL;
    struct diskObj *pstDisk = pstFileInfo->owner->owner;
    MF_U8 chnId = pstFileInfo->key.stRecord.chnId;
    MF_S32  res = MF_SUCCESS;

    IPCINFO_FIND_BY_ID(chnId, pstIpcInfo, &pstDisk->ipcHead);
    if (!pstIpcInfo) {
        pstIpcInfo = retr_ipc_info_alloc(chnId);
        pstIpcInfo->owner = pstDisk;
        LIST_INSERT_SORT_ASC(pstIpcInfo, &pstDisk->ipcHead, chnId);
        pstDisk->ipcNum++;
    }
    pstFileInfo->fileStartTime = INIT_START_TIME;
    pstFileInfo->fileEndTime = INIT_END_TIME;
    ms_mutex_lock(&pstIpcInfo->mutex);
    res = retr_file_info_add(pstFileInfo, pstIpcInfo);
    //update owner info
    if (res == MF_SUCCESS) {
        mf_retr_file_owner_update(pstFileInfo);
        mf_retr_ipc_owner_update(pstIpcInfo);
    }
    ms_mutex_unlock(&pstIpcInfo->mutex);

    if (res != MF_SUCCESS) {
        return NULL;
    }

    return pstFileInfo;
}

struct file_info *
mf_retr_file_info_reset(struct file_info *pstFileInfo)
{
    struct ipc_info *pstIpcInfo = NULL;
    struct diskObj *pstDisk = pstFileInfo->owner->owner;
    MF_U8 chnId = pstFileInfo->key.stRecord.chnId;

    IPCINFO_FIND_BY_ID(chnId, pstIpcInfo, &pstDisk->ipcHead);
    if (!pstIpcInfo) {
        pstIpcInfo = retr_ipc_info_alloc(chnId);
        pstIpcInfo->owner = pstDisk;
        LIST_INSERT_SORT_ASC(pstIpcInfo, &pstDisk->ipcHead, chnId);
        pstDisk->ipcNum++;
    }

    pstFileInfo->fileStartTime = INIT_START_TIME;
    pstFileInfo->fileEndTime = INIT_END_TIME;
    pstFileInfo->enMajor = INFO_MAJOR_INIT;
    pstFileInfo->enMinor = INFO_MINOR_INIT;
    pstFileInfo->owner = pstIpcInfo;
    pstFileInfo->lockNum = 0;
    pstFileInfo->tagNum = 0;
    pstFileInfo->loadTime = 0;
    pstFileInfo->refNum = 0;
    pstFileInfo->enLoad = FILE_PRE_LOAD;
    ms_mutex_lock(&pstIpcInfo->mutex);
    LIST_INSERT_SORT_ASC(pstFileInfo, &pstIpcInfo->fileHead, fileStartTime);
    pstIpcInfo->fileNum++;
    //update owner info
    mf_retr_file_owner_update(pstFileInfo);
    mf_retr_ipc_owner_update(pstIpcInfo);
    ms_mutex_unlock(&pstIpcInfo->mutex);
    return pstFileInfo;
}

MF_BOOL
mf_retr_file_deadline_check(MF_U8 chnId, FILE_TYPE_EN enType, MF_U32 deadTime)
{
    struct diskObj *disk = NULL;
    struct list_head *head = mf_disk_get_wr_list(MF_YES);
    MF_PTS targetTime = time(0) - deadTime;
    MF_BOOL bDone = MF_NO;

    if (!head) {
        return MF_NO;
    }

    list_for_each_entry(disk, head, node) {
        if (disk->bRec == MF_NO) {
            continue;
        }

        if (disk->enRw == DISK_R_ONLY) {
            continue;
        }

        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > targetTime) {
            continue;
        }

        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > targetTime) {
                continue;
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != enType) {
                    continue;
                }

                if (file->fileStartTime > targetTime) {
                    continue;
                }

                if (file->enLoad != FILE_SEG_LOAD)
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }

                if (file->fileEndTime <= targetTime
                    && file->bRecording == MF_NO
                    && file->lockNum == 0
                    && file->refNum == 0) {
                    if (mf_retr_file_delete(file, FILE_CONDITION_GOOD) == MF_SUCCESS) {
                        bDone = MF_YES;
                    }
                } else {
                    struct seg_info *seg = NULL;
                    struct seg_info *n = NULL;
                    list_for_each_entry_safe(seg, n, &file->segHead, node) {
                        if (seg->segStartTime < targetTime) {
                            if (file->pstSegUsing == seg) {
                                continue;
                            }
                            if (seg->enState & SEG_STATE_LOCK) {
                                continue;
                            }
                            MFdbg("@@@@@@@@@@@@@ start delete segment deadTime[%d]!", deadTime / 60 / 60 / 24);
                            retr_seg_state_update(seg, SEG_STATE_INVALID);
                            if (file->segNum == 0 && file->bRecording == MF_NO) {
                                if (mf_retr_file_delete(file, FILE_CONDITION_GOOD) == MF_SUCCESS) {
                                    bDone = MF_YES;
                                }
                            }
                            MFdbg("@@@@@@@@@@@@@ stop delete segment !");
                        }
                    }

                }

            }
        }
    }
    mf_disk_put_rw_list(head);
    return bDone;
}

MF_S32
mf_retr_file_info_from_disk(struct diskObj *pstDisk, struct mf_file *pstFile)
{
    MF_S8 *cache = ms_valloc(FILE_HEAD_MAX_SIZE);
    off64_t offset;
    MF_S32  res;
    MF_S32  fd;
    struct mf_file *tmp;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, pstFile->fileNo, MF_YES);
    offset = pstFile->fileOffset + FILE_HEAD_OFFSET;
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, cache, FILE_HEAD_MAX_SIZE, offset);
    if (res < 0) {
        pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
        retr_mem_free(cache);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    tmp = (struct mf_file *)cache;
    if (tmp->magic != FILE_MAGIC) {
        pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
        retr_mem_free(cache);
        return MF_FAILURE;
    }

    tmp->fileState = FILE_STATE_USED;
    memcpy(pstFile, tmp, sizeof(struct mf_file));
    pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
    retr_mem_free(cache);

    return MF_SUCCESS;
}

MF_S32
mf_retr_file_head_preload(struct file_info *pstFileInfo)
{
    struct ipc_info *pstIpcInfo = pstFileInfo->owner;
    struct diskObj *pstDisk = pstIpcInfo->owner;
    struct mf_file *pstFile;
    struct bplus_data stData;
    off64_t offset;
    MF_S32 fd, res = 0;

    if (pstFileInfo->enLoad == FILE_SEG_LOAD) {
        return MF_SUCCESS;
    }

    MF_S8 *cache = ms_valloc(MF_MAX(FILE_HEAD_MAX_SIZE, SEG_HEAD_MAX_SIZE));
    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, pstFileInfo->key.stRecord.fileNo, MF_YES);
    offset = pstFileInfo->key.stRecord.fileOffset + FILE_HEAD_OFFSET;
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, cache, FILE_HEAD_MAX_SIZE, offset);
    if (res < 0) {
        pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
        retr_mem_free(cache);
        MFerr("disk_read failed");
        return MF_FAILURE;
    }

    pstFile = (struct mf_file *)cache;
    if (pstFile->magic != FILE_MAGIC
        || pstFile->segNum > SEG_MAX_NUM
        || (pstFileInfo->key.stRecord.state == FILE_STATE_USED && pstFile->segNum == 0)) {
        MFerr("port[%d] file[%d] magic[%#x] segNum[%d] bad,set FILE_CONDITION_BAD!",
              pstDisk->port, pstFileInfo->key.stRecord.fileNo,
              pstFile->magic, pstFile->segNum);
        if (mf_index_key_delete(pstDisk->tree, &pstFileInfo->key) == MF_SUCCESS) {
            pstFileInfo->key.stRecord.condition = FILE_CONDITION_BAD;
            stData.enType = TYPE_RECORD;
            stData.stRecord.fileOffset = pstFileInfo->key.stRecord.fileOffset;
            res = mf_index_key_insert(pstDisk->tree, &pstFileInfo->key, &stData);
            if (res == MF_SUCCESS) {
                mf_index_key_backup(pstDisk->tree);
            } else {
                mf_index_key_recover(pstDisk->tree);
            }
        }
    }

    pstFileInfo->segNum = pstFile->segNum;
    pstFileInfo->lockNum = pstFile->lockNum;
    //others
    pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);
    retr_mem_free(cache);

    return MF_SUCCESS;
}

void
mf_retr_ipc_record_range(MF_U8 chnId, FILE_TYPE_EN enType, MF_TIME *time)
{
    struct diskObj *disk = NULL;
    struct list_head *head = mf_disk_get_rd_list(MF_NO);
    list_for_each_entry(disk, head, node) {
        if (disk->bRec == MF_NO) {
            continue;
        }
        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }
        if (disk->ipcNum == 0) {
            continue;
        }

        ms_mutex_lock(&disk->mutex);
        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId == chnId) {
                if (enType == FILE_TYPE_MAIN) {
                    time->startTime = MF_MIN(time->startTime, ipc->mainStartTime);
                    time->endTime = MF_MAX(time->endTime, ipc->mainEndTime);
                } else if (enType == FILE_TYPE_SUB) {
                    time->startTime = MF_MIN(time->startTime, ipc->subStartTime);
                    time->endTime = MF_MAX(time->endTime, ipc->subEndTime);
                } else if (enType == FILE_TYPE_PIC) {
                    time->startTime = MF_MIN(time->startTime, ipc->picStartTime);
                    time->endTime = MF_MAX(time->endTime, ipc->picEndTime);
                } else {
                    time->startTime = MF_MIN(time->startTime, ipc->ipcStartTime);
                    time->endTime = MF_MAX(time->endTime, ipc->ipcEndTime);
                }

            }
        }
        ms_mutex_unlock(&disk->mutex);
    }
    mf_disk_put_rw_list(head);
}

static MF_S32
retr_item_stream_add(MF_U8 chnId,
                     struct list_head *diskList,
                     struct item_evt_t *item,
                     FILE_TYPE_EN enType,
                     REC_EVENT_EN enEvent,
                     SEG_STATE_EN enState,
                     MF_TIME *pstTime,
                     MF_U32 total,
                     MF_S32 *pCnt)
{
    struct diskObj *disk = NULL;
    struct diskObj *n = NULL;
    struct stream_seg *s;
    MF_U32  streamCount = 0;
    MF_S32  res;

    list_for_each_entry_safe(disk, n, diskList, node) {
        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > pstTime->endTime
            || disk->endTime < pstTime->startTime) {
            continue;
        }

        ms_mutex_lock(&disk->opsMutex);

        if (disk->bRetr == MF_NO) {
            ms_mutex_unlock(&disk->opsMutex);
            continue;
        }

        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstTime->endTime
                || ipc->ipcEndTime < pstTime->startTime) {
                continue;
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != enType) {
                    continue;
                }
                if (!(file->enEvent & enEvent)) {
                    continue;
                }

                if (file->fileStartTime > pstTime->endTime
                    || file->fileEndTime < pstTime->startTime) {
                    continue;
                }

                if (file->enLoad != FILE_SEG_LOAD)
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }

                struct seg_info *seg = NULL;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg->segStartTime > pstTime->endTime
                        || seg->segEndTime < pstTime->startTime) {
                        continue;
                    }

                    if (!(seg->enEvent & enEvent)) {
                        continue;
                    }

                    if (!(seg->enState & enState)) {
                        continue;
                    }

                    if (seg->recStartOffset == seg->recEndOffset) {
                        continue;
                    }

                    if (seg->firstIframeTime == 0) {
                        continue;
                    }

                    s = retr_mem_calloc(1, sizeof(struct stream_seg));
                    s->pstDisk      = disk;
                    s->chnid        = chnId;
                    s->type         = enType;
                    s->fileNo       = file->key.stRecord.fileNo;
                    s->baseOffset   = file->key.stRecord.fileOffset;
                    s->pstSegInfo   = seg;
                    s->pstFileInfo  = file;
                    s->pstIpcInfo   = ipc;

                    res = mf_retr_get_search_seg_info(s, seg, pstTime);
                    if (res != MF_SUCCESS) {
                        retr_mem_free(s);
                        break;
                    }

                    LIST_INSERT_SORT_ASC(s, &item->streamList, startTime);
                    streamCount++;
                    file->refNum++;
                    item->size += s->size;
                    item->streamTime.startTime = MF_MIN(item->streamTime.startTime, s->startTime);
                    item->streamTime.endTime = MF_MAX(item->streamTime.endTime, s->endTime);
                    if (total + streamCount >= BKP_MAX_RESULT_NUM) {
                        ms_mutex_unlock(&disk->opsMutex);
                        *pCnt = streamCount;
                        return MF_FAILURE;
                    }
                }
            }
        }
        ms_mutex_unlock(&disk->opsMutex);
    }

    *pCnt = streamCount;
    return MF_SUCCESS;
}

static void
retr_item_stream_del(struct list_head *list)
{
    struct stream_seg *stream = NULL;
    struct stream_seg *n = NULL;

    list_for_each_entry_safe(stream, n, list, node) {
        stream->pstFileInfo->refNum--;
        list_del(&stream->node);
        retr_mem_free(stream);
    }
}

MF_S32
mf_retr_seg_load(struct file_info *pstFileInfo)     // pre lock ipc Mutex
{
    struct ipc_info *pstIpcInfo = pstFileInfo->owner;
    MF_S32 res = MF_FAILURE;

    if (pstFileInfo->enLoad == FILE_SEG_LOAD) {
        return MF_SUCCESS;
    }

    res = retr_file_info_add(pstFileInfo,  pstIpcInfo);
    if (res == MF_SUCCESS) {
        pstFileInfo->enLoad = FILE_SEG_LOAD;
        pstFileInfo->loadTime = mf_task_timer();
        mf_retr_file_owner_update(pstFileInfo);
    } else {
        retr_file_info_del(pstFileInfo);
        mf_retr_file_owner_update(pstFileInfo);
        retr_file_info_free(pstFileInfo);
    }

    mf_retr_ipc_owner_update(pstIpcInfo);

    return res;
//  return retr_file_info_seg_load(pstFileInfo);
}

MF_S32
mf_retr_unload(MF_U32 timer)
{
    if (retr_mem_usage_percent(100) < 70) {
        return MF_FAILURE;
    }

    struct list_head *list;
    struct diskObj *disk;
    list = mf_disk_get_wr_list(MF_YES);
    if (!list) {
        return MF_FAILURE;
    }

    list_for_each_entry(disk, list, node) {
        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }

        struct ipc_info *ipc;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            struct file_info *file;
            struct file_info *n;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->enLoad == FILE_SEG_LOAD) {
                    if (timer - file->loadTime > RETR_UNLOAD_TIME &&
                        file->bRecording == MF_NO && file->refNum == 0) {
                        MFshow("unloading IPC[%d] file starTime[%s] ...",
                               ipc->chnId, mf_time_to_string(file->fileStartTime));
                        retr_file_info_seg_unload(file);
                        MFshow("unloading IPC[%d] file endTime[%s] ...",
                               ipc->chnId, mf_time_to_string(file->fileEndTime));
                    } else if (file->refNum > 0 || file->bRecording == MF_YES) {
                        file->loadTime = timer;
                    }
                }
            }
        }
    }

    mf_disk_put_rw_list(list);
    return MF_SUCCESS;
}


MF_BOOL
mf_retr_stream_is_valid(struct stream_seg *stream)
{
    struct list_head *diskList;

    if (stream == NULL) {
        MFerr("stream is NULL\n");
        return MF_NO;
    }

    if (stream->pstSegInfo == NULL) {
        MFerr("pstSegInfo is NULL\n");
        return MF_NO;
    }

    diskList = mf_disk_get_rd_list(MF_NO);
    struct diskObj *disk;

    list_for_each_entry(disk, diskList, node) {
        if (disk != stream->pstDisk) {
            continue;
        }

        if (disk->enState != DISK_STATE_NORMAL) {
            MFerr("disk state != DISK_STATE_NORMAL");
            break;
        }

        struct ipc_info *ipc;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc != stream->pstIpcInfo) {
                continue;
            }

            struct file_info *file;
            list_for_each_entry(file, &ipc->fileHead, node) {
                if (file != stream->pstFileInfo) {
                    continue;
                }

                struct seg_info *seg;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg != stream->pstSegInfo) {
                        continue;
                    }

                    mf_disk_put_rw_list(diskList);
                    return MF_YES;
                }
            }
        }
    }

    mf_disk_put_rw_list(diskList);
    stream->bDiscard = MF_YES;
    MFerr("stream is invalid !");
    return MF_NO;
}

MF_S32
mf_retr_ipc_comm_begin(MF_U8 chnId,
                       FILE_TYPE_EN enType,
                       REC_EVENT_EN enEvent,
                       SEG_STATE_EN enState,
                       MF_TIME *pstTime,
                       struct bkp_node_t *pstNode,
                       MF_U32 total)
{
    struct list_head *diskList;
    MF_S32 res = MF_SUCCESS;
    MF_U64 searchStartTime;
    MF_U64 searchEndTime;

    searchStartTime = (MF_U64)1000000 * pstTime->startTime;
    searchEndTime = (MF_U64)1000000 * pstTime->endTime;
    mf_record_vid_sync(chnId, enType);
    diskList = mf_disk_get_rd_list(MF_NO);

//    MFprint("=========chn[%d]==event %#x\n", chnId, enEvent);
    struct diskObj *disk = NULL;

    list_for_each_entry(disk, diskList, node) {
        if (disk->bRec == MF_NO) {
            continue;
        }

        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }

        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > pstTime->endTime
            || disk->endTime < pstTime->startTime) {
            continue;
        }

        if (RETR_FASTER == 1) {
            //MFshow("common disk port:%d enType:%u enEvent:%u stream:%u major:[%u %u %u] minor[%u %u %u]", 
            //    disk->port, enType, enEvent, disk->streamType,
            //    disk->recType, disk->majorType, disk->minorType, 
            //    disk->recSubType, disk->majorSubType, disk->minorSubType);
     
            if (!(disk->streamType>>enType&0x01)) {
                continue;
            }

            if (enType == FILE_TYPE_MAIN && (!(disk->recType & enEvent))) {
                continue;
            }

            if (enType == FILE_TYPE_SUB && (!(disk->recSubType & enEvent))) {
                continue;
            }
        }

        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstTime->endTime
                || ipc->ipcEndTime < pstTime->startTime) {
                break;
            }

            if (RETR_FASTER == 1) {
                //MFshow("common ipc enType:%d enEvent:%u stream:%u major:[%u %u %u] minor[%u %u %u]", 
                //    enType, enEvent, ipc->streamType,
                //    ipc->recType, ipc->majorType, ipc->minorType, 
                //    ipc->recSubType, ipc->majorSubType, ipc->minorSubType);
                if (!(ipc->streamType>>enType&0x01)) {
                    break;
                }

                if (enType == FILE_TYPE_MAIN && (!(ipc->recType & enEvent))) {
                    break;
                }

                if (enType == FILE_TYPE_SUB && (!(ipc->recSubType & enEvent))) {
                    break;
                }
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != enType) {
                    continue;
                }

                if (!(file->enEvent & enEvent)) { 
                    continue;
                }

                if (file->fileStartTime > pstTime->endTime
                    || file->fileEndTime < pstTime->startTime) {
                    continue;
                }

                if (file->enLoad == FILE_SEG_LOAD || file->enLoad == FILE_SEG_UNLOAD) {
                    if (!(file->enState & enState)) {
                        continue;
                    }
                }

                if (file->enLoad != FILE_SEG_LOAD) {
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }
                }

                if (!(file->enState & enState)) {
                    continue;
                }

                struct seg_info *seg = NULL;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg->infoCount == 0) {
                        continue;
                    }

                    if (seg->segStartTime > pstTime->endTime
                        || seg->segEndTime < pstTime->startTime) {
                        continue;
                    }

                    if (enEvent != REC_EVENT_ALL && seg->enEvent != enEvent) {
                        continue;
                    }

                    if (!(seg->enState & enState)) {
                        continue;
                    }

                    if (seg->recStartOffset == seg->recEndOffset) {
                        continue;
                    }

                    if (seg->firstIframeTime == 0) {
                        continue;
                    }

                    /*
                        14-r8补充I帧范围判断，因修改时间写入音频帧导致的片段异常问题兼容，若干版本后可删除
                        【Dawn-日本-NBC：NVR修改时间后再改回正确的时间，部分通道录像异常】
                        https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001072529
                    */
                    if (seg->firstIframeTime > searchEndTime
                        || seg->lastIframeTime < searchStartTime) {
                        continue;
                    }
                        
                    struct stream_seg *s;
                    struct item_comm_t *item;
                    item = retr_mem_calloc(1, sizeof(struct item_comm_t));
                    INIT_LIST_HEAD(&item->streamList);

                    s = retr_mem_calloc(1, sizeof(struct stream_seg));
                    s->pstDisk      = disk;
                    s->chnid        = chnId;
                    s->type         = enType;
                    s->fileNo       = file->key.stRecord.fileNo;
                    s->baseOffset   = file->key.stRecord.fileOffset;
                    s->pstSegInfo   = seg;
                    s->pstFileInfo  = file;
                    s->pstIpcInfo   = ipc;

                    res = mf_retr_get_search_seg_info(s, seg, pstTime);
                    if (res != MF_SUCCESS) {
                        retr_mem_free(s);
                        retr_mem_free(item);
                        break;
                    }
                    list_add_tail(&s->node, &item->streamList);
                    item->streamCount++;
                    item->size              = s->size;
                    item->port              = disk->port;
                    item->chnId             = s->chnid;
                    item->encodeType        = s->encodeType;
                    item->time.startTime    = s->startTime;
                    item->time.endTime      = s->endTime;
                    item->isLock = seg->enState == SEG_STATE_LOCK ? MF_YES : MF_NO;
                    LIST_INSERT_SORT_ASC(item, &pstNode->list, time.startTime);
                    file->refNum++;
                    pstNode->count++;
                    pstNode->size += item->size;
                    pstNode->range.startTime = MF_MIN(pstNode->range.startTime, item->time.startTime);
                    pstNode->range.endTime = MF_MAX(pstNode->range.endTime, item->time.endTime);
                    if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
                        mf_disk_put_rw_list(diskList);
                        return MF_FAILURE;
                    }
                }
            }
            break;
        }
    }

    mf_disk_put_rw_list(diskList);
    return MF_SUCCESS;
}

void
mf_retr_ipc_comm_end(struct bkp_node_t *pstNode)
{
    struct item_comm_t *item = NULL;
    struct item_comm_t *n = NULL;
    struct stream_seg *stream = NULL;
    struct stream_seg *m = NULL;

    list_for_each_entry_safe(item, n, &pstNode->list, node) {
        list_for_each_entry_safe(stream, m, &item->streamList, node) {
            stream->pstFileInfo->refNum--;
            list_del(&stream->node);
            retr_mem_free(stream);
        }
        list_del(&item->node);
        retr_mem_free(item);
    }
}

MF_S32
mf_retr_ipc_event_begin(MF_U8 chnId,
                        struct bkp_event_t *pstEvt,
                        struct bkp_node_t *pstNode,
                        MF_U32 total,
                        MF_S32 baseP,
                        MF_U32 deltaP,
                        MF_BOOL progress)
{
    struct list_head *diskList;
    MF_U32 eventCount = 0;
    MF_S32 cnt = 0;
    MF_S32 res = MF_SUCCESS;
    MF_U64 searchStartTime;
    MF_U64 searchEndTime;

    searchStartTime = (MF_U64)1000000 * pstEvt->time.startTime;
    searchEndTime = (MF_U64)1000000 * pstEvt->time.endTime;
    mf_record_vid_sync(chnId, pstEvt->enType);
    diskList = mf_disk_get_rd_list(MF_NO);
    mf_disk_put_rw_list(diskList);

    struct diskObj *disk = NULL;
    struct diskObj *n = NULL;

    list_for_each_entry_safe(disk, n, diskList, node) {

        if (disk->bRec == MF_NO) {
            continue;
        }

        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }
        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > pstEvt->time.endTime
            || disk->endTime < pstEvt->time.startTime) {
            continue;
        }

        if (RETR_FASTER == 1) {
            //MFshow("event disk port:%d enType:%d major:%u minor:%u stream:%u major:[%u %u %u] minor[%u %u %u]", 
            //    disk->port, pstEvt->enType, pstEvt->enMajor, pstEvt->enMinor, disk->streamType,
            //    disk->recType, disk->majorType, disk->minorType, 
            //    disk->recSubType, disk->majorSubType, disk->minorSubType);
            if (!(disk->streamType>>pstEvt->enType&0x01)) {
                continue;
            }

            if (pstEvt->enType == FILE_TYPE_MAIN) {
                if (!(disk->majorType & pstEvt->enMajor)) {
                    continue;
                }

                if (pstEvt->enMinor != INFO_MINOR_NONE && (!(disk->minorType & pstEvt->enMinor))) {
                    continue;
                }
            }

            if (pstEvt->enType == FILE_TYPE_SUB) {
                if (!(disk->majorSubType & pstEvt->enMajor)) {
                    continue;
                }

                if (pstEvt->enMinor != INFO_MINOR_NONE && (!(disk->minorSubType & pstEvt->enMinor))) {
                    continue;
                }
            }
        }
        ms_mutex_lock(&disk->opsMutex);
        if (disk->bRetr == MF_NO) {
            ms_mutex_unlock(&disk->opsMutex);
            continue;
        }
        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstEvt->time.endTime
                || ipc->ipcEndTime < pstEvt->time.startTime) {
                break;
            }
            if (RETR_FASTER == 1) {
                //MFshow("event ipc enType:%d major:%u minor:%u stream:%u major:[%u %u %u] minor[%u %u %u]", 
                //    pstEvt->enType, pstEvt->enMajor, pstEvt->enMinor, ipc->streamType,
                //    ipc->recType, ipc->majorType, ipc->minorType, 
                //    ipc->recSubType, ipc->majorSubType, ipc->minorSubType);
                if (!(ipc->streamType>>pstEvt->enType&0x01)) {
                    break;
                }

                if (pstEvt->enType == FILE_TYPE_MAIN) {
                    if (!(ipc->majorType & pstEvt->enMajor)) {
                        break;
                    }
                    if (pstEvt->enMinor != INFO_MINOR_NONE && (!(ipc->minorType & pstEvt->enMinor))) {
                        break;
                    }
                }

                if (pstEvt->enType == FILE_TYPE_SUB) {
                    if (!(ipc->majorSubType & pstEvt->enMajor)) {
                        break;
                    }

                    if (pstEvt->enMinor != INFO_MINOR_NONE && (!(ipc->minorSubType & pstEvt->enMinor))) {
                        break;
                    }
                }
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != pstEvt->enType) {
                    continue;
                }

                if (file->fileStartTime > pstEvt->time.endTime
                    || file->fileEndTime < pstEvt->time.startTime) {
                    continue;
                }

                if (file->key.stRecord.infoType) {
                    if (!(file->key.stRecord.infoType & pstEvt->enMajor) &&
                        !(file->key.stRecord.infoType & pstEvt->enMinor)) {
                        continue;
                    }
                }

                if (file->enLoad == FILE_SEG_LOAD || file->enLoad == FILE_SEG_UNLOAD) {
                    if (!(file->enMajor & pstEvt->enMajor)) {
                        continue;
                    }
                    if ((pstEvt->enMinor != INFO_MINOR_NONE) &&
                        (file->enMinor & INFO_MINOR_INIT) &&
                        !(file->enMinor & pstEvt->enMinor)) {
                        continue;
                    }
                }

                if (file->enLoad != FILE_SEG_LOAD) {
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }
                }

                if (!(file->enMajor & pstEvt->enMajor)) {
                    continue;
                }

                if ((pstEvt->enMinor != INFO_MINOR_NONE) &&
                    (file->enMinor & INFO_MINOR_INIT) &&
                    !(file->enMinor & pstEvt->enMinor)) {
                    continue;
                }

                struct seg_info *seg = NULL;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg->infoCount == 0) {
                        continue;
                    }

                    if (seg->segStartTime > pstEvt->time.endTime
                        || seg->segEndTime < pstEvt->time.startTime) {
                        continue;
                    }

                    if (seg->firstIframeTime > searchEndTime
                        || seg->lastIframeTime < searchStartTime) {
                        continue;
                    }
                    if (!(seg->enMajor & pstEvt->enMajor)) {
                        continue;
                    }

                    if ((pstEvt->enMinor != INFO_MINOR_NONE) &&
                        (seg->enMinor & INFO_MINOR_INIT) &&
                        !(seg->enMinor & pstEvt->enMinor)) {
                        continue;
                    }

                    cnt = 0;
                    res = retr_seg_event_traverse(seg, pstEvt->enMajor,
                                                  pstEvt->enMinor,
                                                  pstEvt->enDtc,
                                                  &pstEvt->time,
                                                  &pstEvt->srcList,
                                                  &pstNode->list,
                                                  pstEvt->match,
                                                  &pstEvt->pri,
                                                  &cnt,
                                                  (pstEvt->maxNum - total - eventCount),
                                                  MF_YES);
                    if (res != MF_SUCCESS) {
                        break;
                    }
                    eventCount += cnt;
                }

                if (pstEvt->bCancel == MF_YES) {
                    pstNode->count = eventCount;
                    pstNode->size  = 0;
                    ms_mutex_unlock(&disk->opsMutex);
                    return MF_SUCCESS;
                }
            }
            break;
        }
        ms_mutex_unlock(&disk->opsMutex);
    }
    if (pstEvt->ahead == 0 && pstEvt->delay == 0) {
        pstNode->count = eventCount;
        pstNode->size  = 0;
    } else {
        struct item_evt_t *item = NULL;
        struct item_evt_t *n = NULL;
        struct item_evt_t *pNext = NULL;
        MF_TIME range;
        MF_S32 percent = baseP;
        MF_S32 lastP = percent;
        MF_S32 m = 0;

        list_for_each_entry_safe(item, n, &pstNode->list, node) {
            cnt = 0;
            range.startTime = item->time.startTime - pstEvt->ahead;
            range.endTime   = item->time.endTime + pstEvt->delay;
            retr_item_stream_add(chnId, diskList,
                                 item, pstEvt->enType,
                                 REC_EVENT_ALL, SEG_STATE_ALL,
                                 &range, total + pstNode->count,
                                 &cnt);
            item->streamCount = cnt;
            m++;
            percent = baseP + deltaP * m / eventCount;
            if (progress && lastP != percent) {
                lastP = percent;
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent,
                                         pstEvt->searchfrom, pstEvt->searchid);
            }

            pstNode->count += item->streamCount;
            pstNode->size  += item->size;
            pstNode->range.startTime = MF_MIN(item->streamTime.startTime, pstNode->range.startTime);
            pstNode->range.endTime = MF_MAX(item->streamTime.endTime, pstNode->range.endTime);
            pNext = list_entry(item->node.next, struct item_evt_t, node);

            if (item->streamCount == 0) {
                retr_item_stream_del(&item->streamList);
                list_del(&item->node);
                if (item->pstPrivate) {
                    retr_mem_free(item->pstPrivate);
                }
                retr_mem_free(item);
            }
            if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
                break;
            }
            if (pstEvt->bCancel == MF_YES) {
                break;
            }
        }

        if (pNext) {
            item = pNext;
            list_for_each_entry_safe_from(item, n, &pstNode->list, node) {
                retr_item_stream_del(&item->streamList);
                list_del(&item->node);
                if (item->pstPrivate) {
                    retr_mem_free(item->pstPrivate);
                }
                retr_mem_free(item);
            }
        }
    }
    return MF_SUCCESS;
}

void
mf_retr_ipc_event_end(struct bkp_node_t *pstNode)
{
    struct item_evt_t *item = NULL;
    struct item_evt_t *n = NULL;

    list_for_each_entry_safe(item, n, &pstNode->list, node) {
        retr_item_stream_del(&item->streamList);
        list_del(&item->node);
        if (item->pstPrivate) {
            retr_mem_free(item->pstPrivate);
        }
        retr_mem_free(item);
    }
}

MF_S32
mf_retr_ipc_pic_begin(MF_U8 chnId,
                      INFO_MAJOR_EN enMajor,
                      INFO_MINOR_EN enMinor,
                      MF_TIME *pstTime,
                      struct bkp_node_t *pstNode,
                      MF_U32 total,
                      struct bkp_pri_t pri,
                      struct bkp_vca_t *vcaPrivate, MF_U32 maxNum)
{
    struct list_head *diskList;
    mf_record_pic_sync(chnId);
    diskList = mf_disk_get_rd_list(MF_NO);

    struct diskObj *disk = NULL;

    list_for_each_entry(disk, diskList, node) {
        if (disk->bRec == MF_NO) {
            continue;
        }

        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }

        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > pstTime->endTime
            || disk->endTime < pstTime->startTime) {
            continue;
        }

        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstTime->endTime
                || ipc->ipcEndTime < pstTime->startTime) {
                break;
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != FILE_TYPE_PIC) {
                    continue;
                }

                if (file->fileStartTime > pstTime->endTime
                    || file->fileEndTime < pstTime->startTime) {
                    continue;
                }

                if (file->key.stRecord.infoType) {
                    if (!(file->key.stRecord.infoType & enMajor) &&
                        !(file->key.stRecord.infoType & enMinor)) {
                        continue;
                    }
                }

                if (file->enLoad == FILE_SEG_LOAD || file->enLoad == FILE_SEG_UNLOAD) {
                    if (!(file->enMajor & enMajor)) {
                        continue;
                    }
                    if ((enMinor != INFO_MINOR_NONE) &&
                        (file->enMinor & INFO_MINOR_INIT) &&
                        !(file->enMinor & enMinor)) {
                        continue;
                    }
                }


                if (file->enLoad != FILE_SEG_LOAD)
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }

                if (!(file->enMajor & enMajor)) {
                    continue;
                }

                if ((enMinor != INFO_MINOR_NONE) &&
                    (file->enMinor & INFO_MINOR_INIT) &&
                    !(file->enMinor & enMinor)) {
                    continue;
                }

                struct seg_info *seg = NULL;
                MF_S32  res;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg->infoCount == 0) {
                        continue;
                    }

                    if (seg->segStartTime > pstTime->endTime
                        || seg->segEndTime < pstTime->startTime) {
                        continue;
                    }

                    if (!(seg->enMajor & enMajor)) {
                        continue;
                    }

                    if ((enMinor != INFO_MINOR_NONE) &&
                        (seg->enMinor & INFO_MINOR_INIT) &&
                        !(seg->enMinor & enMinor)) {
                        continue;
                    }

                    res = mf_retr_get_search_pic_info(seg, pstTime, enMajor, enMinor, pstNode, total, pri, vcaPrivate, maxNum);
                    if (res != MF_SUCCESS) {
                        break;
                    }

                    if (total + pstNode->count >= maxNum) {
                        mf_disk_put_rw_list(diskList);
                        return MF_FAILURE;
                    }
                }
            }
            break;
        }
    }

    mf_disk_put_rw_list(diskList);

    return MF_SUCCESS;
}


void
mf_retr_ipc_pic_end(struct bkp_node_t *pstNode)
{
    struct item_pic_t *item = NULL;
    struct item_pic_t *n = NULL;

    list_for_each_entry_safe(item, n, &pstNode->list, node) {
        item->stream->pstFileInfo->refNum--;
        list_del(&item->node);
        retr_mem_free(item->stream);
        retr_mem_free(item);
    }
}

MF_S32
mf_retr_ipc_lpr_begin(MF_U8 chnId,
                      MF_TIME *pstTime,
                      MF_S8 *keyWord,
                      EVT_MATCH match,
                      struct bkp_pri_t *pri,
                      struct bkp_node_t *pstNode,
                      MF_U32 total)
{
    struct list_head *diskList;

    mf_record_pic_sync(chnId);
    diskList = mf_disk_get_rd_list(MF_NO);

    struct diskObj *disk = NULL;

    list_for_each_entry(disk, diskList, node) {
        if (disk->bRec == MF_NO) {
            continue;
        }

        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }

        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > pstTime->endTime
            || disk->endTime < pstTime->startTime) {
            continue;
        }

        if (RETR_FASTER == 1) {
            if (!(disk->majorType & INFO_MAJOR_SMART)) {
                continue;
            }

            if (!(disk->minorType & INFO_MINOR_LPR)) {
                continue;
            }
        }

        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstTime->endTime
                || ipc->ipcEndTime < pstTime->startTime) {
                break;
            }

            if (RETR_FASTER == 1) {
                if (!(ipc->majorType & INFO_MAJOR_SMART)) {
                    continue;
                }

                if (!(ipc->minorType & INFO_MINOR_LPR)) {
                    continue;
                }
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != FILE_TYPE_PIC) {
                    continue;
                }

                if (file->fileStartTime > pstTime->endTime
                    || file->fileEndTime < pstTime->startTime) {
                    continue;
                }

                if (file->key.stRecord.infoType) {
                    if (!(file->key.stRecord.infoType & INFO_MAJOR_SMART) &&
                        !(file->key.stRecord.infoType & INFO_MINOR_LPR)) {
                        continue;
                    }
                }

                if (file->enLoad == FILE_SEG_LOAD || file->enLoad == FILE_SEG_UNLOAD) {
                    if (!(file->enMajor & INFO_MAJOR_SMART)) {
                        continue;
                    }

                    if ((file->enMinor & INFO_MINOR_INIT) &&
                        !(file->enMinor & INFO_MINOR_LPR)) {
                        continue;
                    }
                }


                if (file->enLoad != FILE_SEG_LOAD)
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }

                if (!(file->enMajor & INFO_MAJOR_SMART)) {
                    continue;
                }

                if ((file->enMinor & INFO_MINOR_INIT) &&
                    !(file->enMinor & INFO_MINOR_LPR)) {
                    continue;
                }

                struct seg_info *seg = NULL;
                MF_S32  res;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg->infoCount == 0) {
                        continue;
                    }

                    if (seg->segStartTime > pstTime->endTime
                        || seg->segEndTime < pstTime->startTime) {
                        continue;
                    }

                    if (!(seg->enMajor & INFO_MAJOR_SMART)) {
                        continue;
                    }

                    if ((seg->enMinor & INFO_MINOR_INIT) &&
                        !(seg->enMinor & INFO_MINOR_LPR)) {
                        continue;
                    }

                    res = mf_retr_get_search_lpr_info(keyWord, match,
                                                      seg, pstTime, pri,
                                                      pstNode, total);
                    if (res != MF_SUCCESS) {
                        break;
                    }

                    if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
                        mf_disk_put_rw_list(diskList);
                        return MF_FAILURE;
                    }
                }
            }
            break;
        }
    }

    mf_disk_put_rw_list(diskList);

    return MF_SUCCESS;
}


void
mf_retr_ipc_lpr_end(struct bkp_node_t *pstNode)
{
    struct item_lpr_t *item = NULL;
    struct item_lpr_t *n = NULL;

    list_for_each_entry_safe(item, n, &pstNode->list, node) {
        item->pstFileInfo->refNum--;
        list_del(&item->node);
        if (item->pstPrivate) {
            retr_mem_free(item->pstPrivate);
        }
        retr_mem_free(item);
    }
}

MF_S32
mf_retr_ipc_face_begin(MF_U8 chnId,
                       MF_TIME *pstTime,
                       EVT_MATCH match,
                       struct bkp_pri_t *pri,
                       struct bkp_node_t *pstNode,
                       MF_U32 total)
{
    struct list_head *diskList;

    mf_record_pic_sync(chnId);
    diskList = mf_disk_get_rd_list(MF_NO);

    struct diskObj *disk = NULL;

    list_for_each_entry(disk, diskList, node) {
        if (disk->bRec == MF_NO) {
            continue;
        }

        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }

        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > pstTime->endTime
            || disk->endTime < pstTime->startTime) {
            continue;
        }

        if (RETR_FASTER == 1) {
            if (!(disk->majorType & INFO_MAJOR_SMART)) {
                continue;
            }

            if (!(disk->minorType & INFO_MINOR_FACE)) {
                continue;
            }
        }

        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstTime->endTime
                || ipc->ipcEndTime < pstTime->startTime) {
                break;
            }

            if (RETR_FASTER == 1) {
                if (!(ipc->majorType & INFO_MAJOR_SMART)) {
                    break;
                }

                if (!(ipc->minorType & INFO_MINOR_FACE)) {
                    break;
                }
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != FILE_TYPE_PIC) {
                    continue;
                }

                if (file->fileStartTime > pstTime->endTime
                    || file->fileEndTime < pstTime->startTime) {
                    continue;
                }

                if (file->key.stRecord.infoType) {
                    if (!(file->key.stRecord.infoType & INFO_MAJOR_SMART) &&
                        !(file->key.stRecord.infoType & INFO_MINOR_FACE)) {
                        continue;
                    }
                }

                if (file->enLoad == FILE_SEG_LOAD || file->enLoad == FILE_SEG_UNLOAD) {
                    if (!(file->enMajor & INFO_MAJOR_SMART)) {
                        continue;
                    }
                    if ((file->enMinor & INFO_MINOR_INIT) &&
                        !(file->enMinor & INFO_MINOR_FACE)) {
                        continue;
                    }
                }


                if (file->enLoad != FILE_SEG_LOAD)
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }

                if (!(file->enMajor & INFO_MAJOR_SMART)) {
                    continue;
                }

                if ((file->enMinor & INFO_MINOR_INIT) &&
                    !(file->enMinor & INFO_MINOR_FACE)) {
                    continue;
                }

                struct seg_info *seg = NULL;
                MF_S32  res;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg->infoCount == 0) {
                        continue;
                    }

                    if (seg->segStartTime > pstTime->endTime
                        || seg->segEndTime < pstTime->startTime) {
                        continue;
                    }

                    if (!(seg->enMajor & INFO_MAJOR_SMART)) {
                        continue;
                    }

                    if ((seg->enMinor & INFO_MINOR_INIT) &&
                        !(seg->enMinor & INFO_MINOR_FACE)) {
                        continue;
                    }

                    res = mf_retr_get_search_face_info(match, seg,
                                                       pstTime, pri,
                                                       pstNode, total);
                    if (res != MF_SUCCESS) {
                        break;
                    }

                    if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
                        mf_disk_put_rw_list(diskList);
                        return MF_FAILURE;
                    }
                }
            }
            break;
        }
    }

    mf_disk_put_rw_list(diskList);

    return MF_SUCCESS;
}


void
mf_retr_ipc_face_end(struct bkp_node_t *pstNode)
{
    struct item_face_t *item = NULL;
    struct item_face_t *n = NULL;

    list_for_each_entry_safe(item, n, &pstNode->list, node) {
        item->pstFileInfo->refNum--;
        list_del(&item->node);
        retr_mem_free(item);
    }
}

MF_S32
mf_retr_ipc_smart_begin(MF_U8 chnId, struct bkp_smart_t *pstSmart,
                        struct bkp_node_t *pstNode, MF_U32 total,
                        void *infoBuff, void *dataBuff)
{
    struct list_head *diskList;
    MF_TIME *pstTime = &pstSmart->time;

    mf_record_vid_sync(chnId, pstSmart->enType);
    diskList = mf_disk_get_rd_list(MF_NO);

    struct diskObj *disk = NULL;
    list_for_each_entry(disk, diskList, node) {
        if (disk->bRec == MF_NO ||
            !disk->ipcNum ||
            disk->enState != DISK_STATE_NORMAL ||
            disk->startTime > pstTime->endTime ||
            disk->endTime < pstTime->startTime) {
            continue;
        }

        if (RETR_FASTER == 1) {
            if (!(disk->streamType>>pstSmart->enType&0x01)) {
                continue;
            }

            if (pstSmart->enType == FILE_TYPE_MAIN) {
                if (!(disk->majorType & INFO_MAJOR_SMART)) {
                    continue;
                }

                if (pstSmart->enMinor != INFO_MINOR_NONE && (!(disk->minorType & pstSmart->enMinor))) {
                    continue;
                }
            }

            if (pstSmart->enType == FILE_TYPE_SUB) {
                if (!(disk->majorSubType & INFO_MAJOR_SMART)) {
                    continue;
                }

                if (pstSmart->enMinor != INFO_MINOR_NONE && (!(disk->minorSubType & pstSmart->enMinor))) {
                    continue;
                }
            }
        }
        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstTime->endTime || ipc->ipcEndTime < pstTime->startTime) {
                break;
            }
            
            if (RETR_FASTER == 1) {
                if (!(ipc->streamType>>pstSmart->enType&0x01)) {
                    break;
                }

                if (pstSmart->enType == FILE_TYPE_MAIN
                    && (!(ipc->majorType & INFO_MAJOR_SMART) && !(ipc->minorType & pstSmart->enMinor))) {
                    break;
                }

                if (pstSmart->enType == FILE_TYPE_SUB
                    && (!(ipc->majorSubType & INFO_MAJOR_SMART) && !(ipc->minorSubType & pstSmart->enMinor))) {
                    break;
                }
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != pstSmart->enType ||
                    file->fileStartTime > pstTime->endTime ||
                    file->fileEndTime < pstTime->startTime) {
                    continue;
                }

                if (file->key.stRecord.infoType) {
                    if (!(file->key.stRecord.infoType & INFO_MAJOR_SMART) &&
                        !(file->key.stRecord.infoType & pstSmart->enMinor)) {
                        continue;
                    }
                }

                if (file->enLoad == FILE_SEG_LOAD ||
                    file->enLoad == FILE_SEG_UNLOAD) {
                    if (!(file->enMajor & INFO_MAJOR_SMART)) {
                        continue;
                    }

                    if ((pstSmart->enMinor != INFO_MINOR_NONE) &&
                        (file->enMinor & INFO_MINOR_INIT) &&
                        !(file->enMinor & pstSmart->enMinor)) {
                        continue;
                    }
                }

                if (file->enLoad != FILE_SEG_LOAD) {
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }
                }

                if (!(file->enMajor & INFO_MAJOR_SMART)) {
                    continue;
                }

                if ((pstSmart->enMinor != INFO_MINOR_NONE) &&
                    (file->enMinor & INFO_MINOR_INIT) &&
                    !(file->enMinor & pstSmart->enMinor)) {
                    continue;
                }

                struct seg_info *seg = NULL;
                MF_S32  res;
                list_for_each_entry(seg, &file->segHead, node) {
                    if (seg->infoCount == 0 ||
                        seg->segStartTime > pstTime->endTime ||
                        seg->segEndTime < pstTime->startTime ||
                        !(seg->enMajor & INFO_MAJOR_SMART)) {
                        continue;
                    }

                    res = mf_retr_get_search_smart_info(seg, pstTime, pstSmart, pstNode,
                                                        total, infoBuff, dataBuff);
                    if (res != MF_SUCCESS) {
                        break;
                    }

                    if (total + pstNode->count >= BKP_MAX_RESULT_NUM) {
                        mf_disk_put_rw_list(diskList);
                        return MF_FAILURE;
                    }

                    if (pstSmart->bCancel == MF_YES) {
                        mf_disk_put_rw_list(diskList);
                        return MF_FAILURE;
                    }
                }
            }
        }
    }
    mf_disk_put_rw_list(diskList);

    return MF_SUCCESS;
}

void
mf_retr_ipc_smart_end(struct bkp_node_t *pstNode)
{
    struct item_smt_t *item = NULL;
    struct item_smt_t *n = NULL;

    list_for_each_entry_safe(item, n, &pstNode->list, node) {
        item->pstFileInfo->refNum--;
        list_del(&item->node);
        retr_mem_free(item->frameData);
        retr_mem_free(item->dataBuff);
        if (item->pstPrivate) {
            retr_mem_free(item->pstPrivate);
        }
        retr_mem_free(item);
    }
}


MF_S32
mf_retr_ipc_tag_begin(MF_U8 chnId,
                      FILE_TYPE_EN enType,
                      MF_TIME *pstTime,
                      MF_S8 *keyWord,
                      MF_S8 *out,
                      struct bkp_node_t *pstNode)
{
    struct list_head *diskList;

    mf_record_vid_sync(chnId, enType);
    diskList = mf_disk_get_rd_list(MF_NO);

    struct diskObj *disk = NULL;

    list_for_each_entry(disk, diskList, node) {
        if (disk->bRec == MF_NO) {
            continue;
        }

        if (disk->enState != DISK_STATE_NORMAL) {
            continue;
        }

        if (!disk->ipcNum) {
            continue;
        }

        if (disk->startTime > pstTime->endTime
            || disk->endTime < pstTime->startTime) {
            continue;
        }

        struct ipc_info *ipc = NULL;
        list_for_each_entry(ipc, &disk->ipcHead, node) {
            if (ipc->chnId != chnId) {
                continue;
            }

            if (ipc->ipcStartTime > pstTime->endTime
                || ipc->ipcEndTime < pstTime->startTime) {
                continue;
            }

            struct file_info *file = NULL;
            struct file_info *n = NULL;
            MF_S32 res;
            list_for_each_entry_safe(file, n, &ipc->fileHead, node) {
                if (file->key.stRecord.type != enType) {
                    continue;
                }

                if (file->fileStartTime > pstTime->endTime
                    || file->fileEndTime < pstTime->startTime) {
                    continue;
                }

                if (file->enLoad != FILE_SEG_LOAD)
                    if (retr_file_info_seg_load(file) == MF_FAILURE) {
                        continue;
                    }

                if (file->tagNum == 0) {
                    continue;
                }

                res = mf_retr_get_search_tag_info(file, pstTime, keyWord, out, pstNode);
                if (res != MF_SUCCESS) {
                    break;
                }
            }
        }
    }

    mf_disk_put_rw_list(diskList);

    return MF_SUCCESS;
}


void
mf_retr_ipc_tag_end(struct bkp_node_t *pstNode)
{
    struct item_tag_t *item = NULL;
    struct item_tag_t *n = NULL;

    list_for_each_entry_safe(item, n, &pstNode->list, node) {
        item->stream->pstFileInfo->refNum--;
        list_del(&item->node);
        retr_mem_free(item->stream);
        retr_mem_free(item);
    }
}

struct item_tag_t *
mf_retr_tag_alloc(struct mf_tag *tag, struct stream_seg *stream, MF_S8 *name, MF_PTS time)
{
    struct item_tag_t *item = retr_mem_calloc(1, sizeof(struct item_tag_t));
    item->stream = retr_mem_calloc(1, sizeof(struct stream_seg));
    item->stream->pstSegInfo    = stream->pstSegInfo;
    item->stream->pstFileInfo   = stream->pstFileInfo;
    item->stream->pstIpcInfo    = stream->pstIpcInfo;
    item->stream->pstDisk       = stream->pstDisk;
    item->chnId = stream->chnid;
    item->time = time;
    strcpy(item->name, name);
    item->id = tag->id ;

    tag->bUsed = MF_YES;
    tag->chnId = item->chnId;
    tag->time  = item->time;
    strcpy(tag->name, item->name);
    retr_file_tag_count(stream->pstFileInfo, MF_YES);

    return item;
}

void
mf_retr_tag_free(struct item_tag_t *item)
{
    if (item) {
        retr_mem_free(item->stream);
        retr_mem_free(item);
    }
}

MF_S32
mf_retr_tag_edit(struct item_tag_t *item, MF_S8 *rename)
{
    struct file_info *file = item->stream->pstFileInfo;
    struct diskObj *disk = item->stream->pstDisk;
    struct bplus_key *key = &file->key;
    struct mf_tag *tag;
    MF_S8 *buff = ms_valloc(TAG_MAX_NUM * TAG_HEAD_SIZE);
    off64_t offset;
    MF_S32 fd;
    MF_S32 res = MF_SUCCESS;

    fd = disk->ops->disk_file_open(disk->pstPrivate, key->stRecord.fileNo, MF_YES);
    offset = key->stRecord.fileOffset + TAG_HEAD_OFFSET;
    res = disk->ops->disk_file_read(disk->pstPrivate,
                                    fd, buff, TAG_MAX_NUM * TAG_HEAD_SIZE, offset);
    if (res < 0) {
        disk_close(fd);
        retr_mem_free(buff);
        MFerr("read disk failed");
        return res;
    }
    tag = (struct mf_tag *)(buff + item->id * TAG_HEAD_SIZE);

    if (rename != NULL) {
        strcpy(tag->name, rename);
    } else {
        memset(tag, 0, sizeof(struct mf_tag));
        tag->bUsed = MF_NO;
        retr_file_tag_count(file, MF_NO);
    }

    res = disk->ops->disk_file_write(disk->pstPrivate,
                                     fd, buff, TAG_MAX_NUM * TAG_HEAD_SIZE, offset);
    if (res < 0) {
        MFerr("write disk failed");
    }
    disk_close(fd);
    retr_mem_free(buff);
    return res;
}


struct exp_info *
mf_retr_export_start(struct stream_seg *s, MF_BOOL bFromIframe)
{
    if (mf_retr_stream_is_valid(s) == MF_NO) {
        return NULL;
    }

    struct exp_info *exp = retr_mem_calloc(1, sizeof(struct exp_info));

    MF_U32  infoSize = s->infoStartOffset - s->infoEndOffset;
    off64_t offset = s->baseOffset + s->infoEndOffset;
    struct diskObj *pstDisk = s->pstDisk;
    MF_S32  res;
    MF_U32 size;
    MF_S8 *off;
    MF_S32  index;
    MF_S8 *infoBuff = ms_valloc(infoSize);

    exp->fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, s->fileNo, MF_YES);
    if (exp->fd <= 0) {
        retr_mem_free(exp);
        retr_mem_free(infoBuff);
        return NULL;
    }

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       exp->fd, infoBuff, infoSize, offset);
    if (res == MF_FAILURE) {
        retr_mem_free(exp);
        retr_mem_free(infoBuff);
        MFerr("read disk failed");
        return NULL;
    }

    mf_retr_seg_info_qsort(infoBuff, infoSize, SORT_IS_UP);
    index = mf_retr_last_Iframe_index(infoBuff, infoSize);
    if (index < 0) {
        retr_mem_free(exp);
        retr_mem_free(infoBuff);
        return NULL;
    }

    exp->infoIndex = index;
    if (SORT_IS_UP) {
        size = (index + 1) * INFO_DATA_MAX_SIZE;
        off = infoBuff;
    } else {
        size = infoSize - index * INFO_DATA_MAX_SIZE;
        off = (MF_S8 *)mf_retr_get_iframe_info(infoBuff, index);
    }
    exp->infoBuff = retr_mem_calloc(1, size);
    exp->infoSize = size;
    exp->infoCount = size / INFO_DATA_MAX_SIZE;
    memcpy(exp->infoBuff, off, size);
    retr_mem_free(infoBuff);

    if (bFromIframe) {
        exp->fileBase = s->baseOffset + s->firstIframeOffset;
        exp->fileSize = s->recEndOffset - s->firstIframeOffset;
    } else {
        exp->fileBase = s->baseOffset + s->recStartOffset;
        exp->fileSize = s->size;
    }

    exp->alignOffset = 0;
    exp->fileOffset = 0;
    exp->stream = s;

    exp->dataBuff = ms_valloc(RETR_EXPORT_BUFF_SIZE);
    exp->dataOffset = 0;
    exp->dataSize = 0;
    exp->dataHead = exp->fileOffset;
    exp->dataTail = exp->dataHead + exp->dataSize;

    return exp;

}

MF_S32
mf_retr_export_stop(struct exp_info *exp)
{
    if (exp == NULL) {
        return MF_FAILURE;
    }

    if (exp->fd) {
        disk_close(exp->fd);
    }
    if (exp->dataBuff) {
        retr_mem_free(exp->dataBuff);
    }
    if (exp->infoBuff) {
        retr_mem_free(exp->infoBuff);
    }

    retr_mem_free(exp);

    return MF_SUCCESS;
}

MF_S32
mf_retr_export_get_frame(struct exp_info *exp, struct mf_frame *frame, MF_FRAME_DATA_TYPE type)
{
    MF_S32 readSize = 0;
    MF_S32 res;

    do {
        res = mf_retr_get_a_frame(exp->dataBuff + exp->dataOffset + exp->alignOffset,
                                  exp->dataBuff + exp->dataSize, frame, type);
        if (res == MF_ERR_RETR_BAD_FRAME) {
            MFerr("MF_ERR_RETR_BAD_FRAME");
            if (retr_find_next_iframe(exp) == MF_FAILURE) {
                return MF_FAILURE;
            } else {
                if (exp->fileOffset < exp->dataHead || exp->fileOffset >= exp->dataTail) {
                    if (retr_load_frame_buff(exp) == MF_FAILURE) {
                        return MF_FAILURE;
                    } else {
                        continue;
                    }
                }
                continue;
            }
        } else if (res == MF_ERR_RETR_DATA_OVERFLOW) {
            if (exp->bTail == MF_YES) {
                return MF_ERR_RETR_DATA_EOF;
            }

            if (retr_load_frame_buff(exp) == MF_FAILURE) {
                return MF_FAILURE;
            } else {
                continue;
            }
        } else {
            readSize = res;
            exp->dataOffset += readSize;
            exp->fileOffset += readSize;
            if (retr_is_iframe(frame) == MF_YES) {
                exp->lastIframeTime = frame->time_usec;
            }
            break;
        }

    } while (1);

    return MF_SUCCESS;
}

MF_S32
mf_retr_export_put_frame(struct exp_info *exp, struct mf_frame *frame)
{
    mf_retr_put_a_frame(frame);
    return MF_SUCCESS;
}

MF_S32
mf_retr_export_put_custom(struct mf_frame *frame)
{
    mf_retr_put_a_frame(frame);
    return MF_SUCCESS;
}


MF_S32
mf_retr_seg_lock(struct stream_seg *s, MF_BOOL bLock)
{
    SEG_STATE_EN enState;

    if (bLock == MF_YES) {
        enState = SEG_STATE_LOCK;
    } else {
        enState = SEG_STATE_NORMAL;
    }

    return retr_seg_state_update(s->pstSegInfo, enState);
}

MF_S32
mf_retr_get_first_Iframe(struct stream_seg *s, struct mf_frame *Iframe)
{
    struct diskObj *pstDisk = s->pstDisk;
    struct ps_frame_s stPsInfo;
    MF_S32 fd;
    MF_U32 headSize = FRAME_HEAD_MAX_SIZE;
    MF_U32 dataSize = 0;
    MF_S8 *PsHead = ms_valloc(headSize + 512);
    MF_S8 *PsData;
    off64_t dataOffset;
    MF_S32 res;

    dataOffset = ALIGN_BACK(s->baseOffset + s->firstIframeOffset, 512);
    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, s->fileNo, MF_YES);
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, PsHead, headSize,
                                       dataOffset);
    if (res <= 0) {
        retr_mem_free(PsHead);
        disk_close(fd);
        MFerr("read disk failed");
        return res;
    }

    res = ps_packet_get_info(PsHead + ((s->baseOffset + s->firstIframeOffset) % 512),
                             FRAME_HEAD_MAX_SIZE, &stPsInfo);
    if (res != MF_SUCCESS) {
        retr_mem_free(PsHead);
        disk_close(fd);
        return res;
    }
    retr_mem_free(PsHead);

    dataSize = ALIGN_UP(stPsInfo.packet_size, 512);
    PsData = ms_valloc(dataSize + 512);
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, PsData, dataSize,
                                       dataOffset);
    if (res <= 0) {
        retr_mem_free(PsData);
        disk_close(fd);
        MFerr("read disk failed");
        return res;
    }

    stPsInfo.data = PsData + ((s->baseOffset + s->firstIframeOffset) % 512);
    Iframe->data = retr_mem_calloc(1, stPsInfo.frame_size);
    ps_packet_decoder(&stPsInfo, Iframe);
    disk_close(fd);
    retr_mem_free(PsData);

    return MF_SUCCESS;
}

MF_S32
mf_retr_get_Iframe_by_time(struct stream_seg *s, struct mf_frame *Iframe, MF_PTS t)
{
    struct ps_frame_s stPsInfo;
    struct diskObj *pstDisk = s->pstDisk;
    struct mf_info *info = NULL;
    MF_U32  infoSize = s->infoStartOffset - s->infoEndOffset;
    off64_t offset = s->baseOffset + s->infoEndOffset;
    MF_U32 dataSize = 0;
    MF_S32 fd;
    MF_S32  res = MF_SUCCESS;
    MF_S32  index;
    MF_S8 *infoBuff = NULL;
    MF_S8 *PsData = NULL;

    Iframe->data = NULL;
    stPsInfo.data = NULL;
    infoBuff = ms_valloc(infoSize);
    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, s->fileNo, MF_YES);

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, infoBuff, infoSize, offset);
    if (res <= 0) {
        res = MF_FAILURE;
        MFerr("read disk failed");
        goto DONE;
    }

    mf_retr_seg_info_qsort(infoBuff, infoSize, SORT_IS_UP);
    index = mf_retr_seg_info_bsearch(infoBuff, infoSize,
                                     (MF_U64)t * 1000000, INFO_MAJOR_IFRAME, SORT_IS_UP);
    info = mf_retr_get_iframe_info(infoBuff, index);
    if (info->majorType != INFO_MAJOR_IFRAME) {
        MFerr("no find I fream");
        res = MF_FAILURE;
        goto DONE;
    }

    if (abs(info->dataTime / 1000000 - t) > 10) {
        MFerr("fream pts over 10s");
        res = MF_FAILURE;
        goto DONE;
    }

    dataSize = ALIGN_UP(info->stIframe.size, 512);
    PsData = ms_valloc(dataSize + 512);
    offset = ALIGN_BACK(s->baseOffset + info->dataOffset, 512);

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, PsData, dataSize,
                                       offset);
    if (res <= 0) {
        res = MF_FAILURE;
        MFerr("read disk failed");
        goto DONE;
    }
    stPsInfo.data = PsData + ((s->baseOffset + info->dataOffset) % 512);
    res = ps_packet_get_info(stPsInfo.data, info->stIframe.size, &stPsInfo);
    Iframe->data = retr_mem_calloc(1, stPsInfo.frame_size);
    ps_packet_decoder(&stPsInfo, Iframe);

DONE:
    disk_close(fd);
    if (infoBuff) {
        retr_mem_free(infoBuff);
    }
    if (PsData) {
        retr_mem_free(PsData);
    }
//    stFrame.data free by mf_retr_put_Iframe();
    return res;
}

void
mf_retr_put_Iframe(struct mf_frame *Iframe)
{
    if (Iframe->data) {
        retr_mem_free(Iframe->data);
    }
    Iframe->data = NULL;
}

MF_S32
mf_retr_get_a_frame(MF_S8 *addr, MF_S8 *end, struct mf_frame *frame, MF_FRAME_DATA_TYPE type)
{
    struct ps_frame_s stPsInfo;
    struct mf_frame tmpFrame;
    MF_S32 res = MF_FAILURE;

    if (end <= addr) {
        return MF_ERR_RETR_DATA_OVERFLOW;
    }

    res = ps_packet_get_info(addr, end - addr, &stPsInfo);

    if (res == PS_ERR_BAD_FRAME) {
        return MF_ERR_RETR_BAD_FRAME;
    }

    if (res == PS_ERR_OVERFLOW) {
        return MF_ERR_RETR_DATA_OVERFLOW;
    }

    if (end - addr < stPsInfo.packet_size) {
        return MF_ERR_RETR_DATA_OVERFLOW;
    }

    if (type == MF_FRAME_DATA_TYPE_PS) {
        frame->data = retr_mem_calloc(1, stPsInfo.packet_size);
        memcpy(frame->data, addr, stPsInfo.packet_size);
        frame->size = stPsInfo.packet_size;
        int ret = ps_packet_get_header(addr, stPsInfo.packet_size, &tmpFrame);
        if (ret == MF_SUCCESS) {
            frame->strm_type = tmpFrame.strm_type;
        }
    } else {
        frame->data = retr_mem_calloc(1, stPsInfo.frame_size);
        ps_packet_decoder(&stPsInfo, frame);
    }

    return stPsInfo.packet_size;
}

void
mf_retr_put_a_frame(struct mf_frame *frame)
{
    if (frame->data) {
        retr_mem_free(frame->data);
    }
    frame->data = NULL;
}

MF_S32
mf_retr_get_a_pic(struct stream_seg *s, INFO_MAJOR_EN m, struct jpg_frame *frame)
{
    struct diskObj *pstDisk = s->pstDisk;
    struct mf_frame mframe;
    MF_S32 fd;
    MF_U32 dataSize = 0;
    MF_S32 res;

    mframe.ch = s->chnid;
    mframe.codec_type = s->encodeType;
    mframe.width = s->resWidth;
    mframe.height = s->resHeight;
    mframe.size = s->size;
    mframe.data = NULL;
    frame->data = NULL;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, s->fileNo, MF_YES);

    dataSize = ALIGN_UP(s->size, 512);
    mframe.data = ms_valloc(dataSize);
    if (!mframe.data) {
        MFerr("malloc failed");
        disk_close(fd);
        return MF_FAILURE;
    }

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, mframe.data, dataSize,
                                       s->baseOffset + s->firstIframeOffset);
    if (res <= 0) {
        retr_mem_free(mframe.data);
        disk_close(fd);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    if (mf_record_jpg_encoder(&mframe, m, frame) == MF_FAILURE) {
        retr_mem_free(mframe.data);
        disk_close(fd);
        MFerr("jpg encoder failed");
        return MF_FAILURE;
    }

    disk_close(fd);
    retr_mem_free(mframe.data);

    return MF_SUCCESS;
}

MF_S32
mf_retr_get_a_wh_pic(struct stream_seg *s, MF_U32 w, MF_U32 h, struct jpg_frame *frame)
{
    struct diskObj *pstDisk = s->pstDisk;
    struct mf_frame mframe;
    MF_S32 fd;
    MF_U32 dataSize = 0;
    MF_S32 res;

    mframe.ch = s->chnid;
    mframe.codec_type = s->encodeType;
    mframe.width = s->resWidth;
    mframe.height = s->resHeight;
    mframe.size = s->size;
    mframe.data = NULL;
    frame->data = NULL;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, s->fileNo, MF_YES);

    dataSize = ALIGN_UP(s->size, 512);
    mframe.data = ms_valloc(dataSize);
    if (!mframe.data) {
        MFerr("malloc failed");
        disk_close(fd);
        return MF_FAILURE;
    }

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, mframe.data, dataSize,
                                       s->baseOffset + s->firstIframeOffset);
    if (res <= 0) {
        retr_mem_free(mframe.data);
        disk_close(fd);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    res = g_stRetr.jpg_cb(&mframe, w, h, (MF_U8 **)&frame->data, &frame->size, &frame->width, &frame->height);

    disk_close(fd);
    retr_mem_free(mframe.data);

    return res;
}


void
mf_retr_put_a_pic(struct jpg_frame *frame)
{
    if (frame->data) {
        retr_mem_free(frame->data);
    }
    frame->data = NULL;
}

MF_S32
mf_retr_get_lpr_pic(struct item_lpr_t *item, MF_BOOL bMainPic, struct jpg_frame *f)
{
    struct stream_seg *s  = retr_mem_alloc(sizeof(struct stream_seg));
    s->pstDisk      = item->pstDisk;
    s->pstIpcInfo   = item->pstIpcInfo;
    s->pstFileInfo  = item->pstFileInfo;
    s->pstSegInfo   = item->pstSegInfo;

    if (mf_retr_stream_is_valid(s) == MF_NO) {
        retr_mem_free(s);
        return MF_FAILURE;
    }

    retr_mem_free(s);
    struct diskObj *pstDisk = item->pstDisk;
    MF_S32 fd;
    MF_U32 dataSize = 0;
    off64_t dataOffset = item->pstFileInfo->key.stRecord.fileOffset;
    MF_S32 res;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, item->fileNo, MF_YES);

    f->data = NULL;
    if (bMainPic) {
        dataSize    = ALIGN_UP(item->picMainSize, 512);
        f->size     = item->picMainSize;
        f->width    = item->picMainWidth;
        f->height   = item->picMainHeight;
        dataOffset += item->picOffset;
    } else {
        dataSize    = ALIGN_UP(item->picSubSize, 512);
        f->size     = item->picSubSize;
        f->width    = item->picSubWidth;
        f->height   = item->picSubHeight;
        dataOffset += item->picOffset + ALIGN_UP(item->picMainSize, 512);
    }

    f->data = ms_valloc(dataSize);
    if (!f->data) {
        MFerr("mem alloc[%d] failed", dataSize);
        disk_close(fd);
        return MF_FAILURE;
    }
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, f->data, dataSize, dataOffset);
    if (res <= 0) {
        retr_mem_free(f->data);
        f->data = NULL;
        disk_close(fd);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    disk_close(fd);

    return MF_SUCCESS;
}

MF_S32
mf_retr_get_lpr_auto_pic(struct item_lpr_t *item, MF_BOOL bMainPic, struct jpg_frame *f, MF_S32 fd)
{
    struct stream_seg *s  = retr_mem_alloc(sizeof(struct stream_seg));
    s->pstDisk      = item->pstDisk;
    s->pstIpcInfo   = item->pstIpcInfo;
    s->pstFileInfo  = item->pstFileInfo;
    s->pstSegInfo   = item->pstSegInfo;

    if (mf_retr_stream_is_valid(s) == MF_NO) {
        retr_mem_free(s);
        return MF_FAILURE;
    }

    retr_mem_free(s);
    struct diskObj *pstDisk = item->pstDisk;
    MF_U32 dataSize = 0;
    off64_t dataOffset = item->pstFileInfo->key.stRecord.fileOffset;
    MF_S32 res;

    f->data = NULL;
    if (bMainPic) {
        dataSize    = ALIGN_UP(item->picMainSize, 512);
        f->size     = item->picMainSize;
        f->width    = item->picMainWidth;
        f->height   = item->picMainHeight;
        dataOffset += item->picOffset;
    } else {
        dataSize    = ALIGN_UP(item->picSubSize, 512);
        f->size     = item->picSubSize;
        f->width    = item->picSubWidth;
        f->height   = item->picSubHeight;
        dataOffset += item->picOffset + ALIGN_UP(item->picMainSize, 512);
    }

    f->data = ms_valloc(dataSize);
    if (!f->data) {
        MFerr("mem alloc[%d] failed", dataSize);
        return MF_FAILURE;
    }
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, f->data, dataSize, dataOffset);
    if (res <= 0) {
        retr_mem_free(f->data);
        f->data = NULL;
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    return MF_SUCCESS;
}

void
mf_retr_put_lpr_pic(struct jpg_frame *f)
{
    mf_retr_put_a_pic(f);
}

MF_S32
mf_retr_get_face_pic(struct item_face_t *item, MF_BOOL bMainPic, struct jpg_frame *f)
{
    struct stream_seg *s  = retr_mem_alloc(sizeof(struct stream_seg));
    s->pstDisk      = item->pstDisk;
    s->pstIpcInfo   = item->pstIpcInfo;
    s->pstFileInfo  = item->pstFileInfo;
    s->pstSegInfo   = item->pstSegInfo;

    if (mf_retr_stream_is_valid(s) == MF_NO) {
        retr_mem_free(s);
        return MF_FAILURE;
    }

    retr_mem_free(s);
    struct diskObj *pstDisk = item->pstDisk;
    MF_S32 fd;
    MF_U32 dataSize = 0;
    off64_t dataOffset = item->pstFileInfo->key.stRecord.fileOffset;
    MF_S32 res;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, item->fileNo, MF_YES);

    f->data = NULL;
    if (bMainPic) {
        dataSize    = ALIGN_UP(item->picMainSize, 512);
        f->size     = item->picMainSize;
        f->width    = item->picMainWidth;
        f->height   = item->picMainHeight;
        dataOffset += item->picMainOffset;
    } else {
        dataSize    = ALIGN_UP(item->picSubSize, 512);
        f->size     = item->picSubSize;
        f->width    = item->picSubWidth;
        f->height   = item->picSubHeight;
        dataOffset += item->picSubOffset;
    }

    f->data = ms_valloc(dataSize);
    if (!f->data) {
        MFerr("mem alloc[%d] failed", dataSize);
        disk_close(fd);
        return MF_FAILURE;
    }
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       fd, f->data, dataSize, dataOffset);
    if (res <= 0) {
        retr_mem_free(f->data);
        disk_close(fd);
        MFerr("read disk failed");
        return MF_FAILURE;
    }

    disk_close(fd);

    return MF_SUCCESS;
}

void
mf_retr_put_face_pic(struct jpg_frame *f)
{
    mf_retr_put_a_pic(f);
}

MF_S32
mf_retr_last_Iframe_index(MF_S8 *infoBuff, MF_U32 infoSize)
{
    struct mf_info *info;
    MF_U32 count = infoSize / INFO_DATA_MAX_SIZE;
    MF_U32 i;

    if (SORT_IS_UP) {
        for (i = 0; i < count; i++) {
            info = (struct mf_info *)(infoBuff + i * INFO_DATA_MAX_SIZE);
            if (info->majorType != INFO_MAJOR_IFRAME) {
                break;
            }
        }
        return i - 1;
    } else {
        for (i = 0; i < count; i++) {
            info = (struct mf_info *)(infoBuff + i * INFO_DATA_MAX_SIZE);
            if (info->majorType == INFO_MAJOR_IFRAME) {
                break;
            }
        }
        return i;
    }
}

struct mf_info *
mf_retr_get_iframe_info(MF_S8 *buff, MF_S32 index)
{
    return (struct mf_info *)(buff + index * INFO_DATA_MAX_SIZE);
}

void
mf_retr_notify_bar_state(PROGRESS_BAR_E enBar,  MF_S32 percent, MF_U8 from, MF_U8 id)
{
    PROGRESS_BAR_T bar;
    memset(&bar, 0, sizeof(PROGRESS_BAR_T));
    bar.percent     = percent;
    bar.searchid    = id;
    bar.searchfrom  = from;
    msfs_notify_progress_bar_state(enBar, &bar);
}

void
mf_retr_jpg_encoder(RETR_JPG_CB jpg_cb)
{
    g_stRetr.jpg_cb = jpg_cb;
}

void
mf_retr_dbg_show_mem_usage()
{
    if (g_stRetr.pstMem) {
        MFinfo("retr_dbg_show_mem_usage");
        mf_mem_state(g_stRetr.pstMem, MF_YES);
    }
}

void
mf_retr_dbg_switch(MF_BOOL bDebug)
{
    g_stRetr.bDebug = bDebug;
}

MF_BOOL
mf_retr_is_dbg()
{
    return g_stRetr.bDebug;
}

MF_S32
mf_retr_init()
{
    memset(&g_stRetr, 0, sizeof(struct mf_retr));
    retr_mem_init(&g_stRetr);

    return MF_SUCCESS;
}

MF_S32
mf_retr_deinit()
{
    retr_mem_uninit(&g_stRetr);
    return MF_SUCCESS;
}


MF_S32
mf_retr_export_get_custom(MF_S8 *data, MF_S32 len, struct mf_frame *frame)
{
    struct ps_frame_s stPsInfo;

    if (ps_packet_custom(data, len, &stPsInfo) != MF_SUCCESS) {
        return MF_FAILURE;
    }

    frame->data = retr_mem_calloc(1, stPsInfo.packet_size);
    memcpy(frame->data, stPsInfo.data, stPsInfo.packet_size);
    frame->size = stPsInfo.packet_size;
    ps_packet_encoder_free(&stPsInfo);
    return frame->size;
}

struct file_info *
mf_retr_disk_find_used_file_qta(struct diskObj *disk, MF_U8 chnId, FILE_TYPE_EN enType)
{
    struct file_info *file = NULL;
    struct mf_grp *grp;
    struct mf_qta *qta;

    mf_disk_qta_grp_info(&grp, &qta);
    if (list_empty(&disk->ipcHead) || disk->bLoop == MF_NO) {
        return NULL;
    }

    /* quota full */
    if (retr_quota_chn_is_cfg(chnId, enType) == MF_YES
        && retr_quota_chn_is_full(chnId, enType) == MF_YES) {
        file = retr_quota_find_file(chnId, disk, enType);
        if (file != NULL) {
            MFinfo("chn[%d] quota full, found itself file chn[%d], time[%ld - %ld]\n",
                   chnId, file->owner->chnId, file->fileStartTime, file->fileEndTime);
            return file;
        }
    }

    /* others */
    file = retr_quota_find_over_file(disk);
    if (file != NULL) {
        MFinfo("chn[%d] quota full, found over file chn[%d], time[%ld - %ld]\n",
               chnId, file->owner->chnId, file->fileStartTime, file->fileEndTime);
        return file;
    }

    if (retr_quota_chn_is_cfg(chnId, enType) == MF_YES
        && retr_quota_chn_is_full(chnId, enType) == MF_NO) {
        file = retr_quota_find_nocfg_file(disk);
        if (file != NULL) {
            MFinfo("chn[%d] quota not full, found nocfg file chn[%d], time[%ld - %ld]\n",
                   chnId, file->owner->chnId, file->fileStartTime, file->fileEndTime);
            return file;
        }
    }

    file = retr_quota_find_nocfg_file(disk);
    if (file) {
        MFinfo("chn[%d] quota nocfg, found nocfg file chn[%d], time[%ld - %ld]\n",
               chnId, file->owner->chnId, file->fileStartTime, file->fileEndTime);
    } else {
        MFinfo("chn[%d] quota found file null]\n", chnId);
    }

    return file;
}

MF_S32
mf_retr_update_index_lock(diskObj *disk, struct file_info *file)
{
    MF_S32  res = MF_FAILURE;
    struct bplus_data stData;

    if (disk == NULL || file == NULL) {
        return MF_FAILURE;
    }

    stData.enType = TYPE_RECORD;
    stData.stRecord.fileOffset = file->key.stRecord.fileOffset;

    ms_mutex_lock(&disk->mutex);
    if (disk->tree != NULL) {
        mf_index_key_delete(disk->tree, &file->key);
        file->key.stRecord.isLocked = file->lockNum == 0 ? 0 : 1;
        res = mf_index_key_insert(disk->tree, &file->key, &stData);
        if (res == MF_SUCCESS) {
            mf_index_key_backup(disk->tree);
        } else {
            mf_index_key_recover(disk->tree);
        }
    }
    ms_mutex_unlock(&disk->mutex);

    return res;
}

