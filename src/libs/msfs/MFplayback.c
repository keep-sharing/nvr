/*
 * ***************************************************************
 * Filename:        MFplayback.c
 * Created at:      2017.05.10
 * Description:     playback API
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
#include "MFplayback.h"
#include "MFcommon.h"
#include "MFmemory.h"

#define PB_DATA_SIZE    (2*1024*1024)
#define PB_CACHE_SIZE    (PB_DATA_SIZE/5)
#define SYNC_THRESHOLD_MAX  (1500*1000)     //1.5s

typedef struct pbCache {
    MF_U32 cacheSize;
    MF_U32 cacheAddr;
    MF_U64 cacheTime;
    MF_U64 syncCode;
    MF_S32 sid;
    MF_S32 lastIframeIndex;
    struct stream_seg *preCacheStream;
    struct stream_seg *curCacheStream;
    struct stream_seg *nextCacheStream;
    struct mf_frame  stFrame;
    struct list_head node;
} pbCache;

typedef struct mf_player {
    MF_BOOL bDebug;
    struct mf_mem_t *pstMem;
    MUTEX_OBJECT     mutex;
    MUTEX_OBJECT     IOMutex;
    struct list_head list;
    PB_OUT_CB       func;
} mf_player;

#define OBJ_FIND_BY_STREAM(obj, stream, head) \
    { \
        struct stream_seg* s = NULL; \
        struct stream_seg* n = NULL; \
        list_for_each_entry_safe(s, n, head, node) \
        { \
            if (stream->startTime == s->startTime \
                && stream->endTime == s->endTime) \
            { \
                obj = s; \
                break; \
            } \
        } \
    }


static MF_S32
get_next_iframe(struct playState *ps, struct mf_frame *frame);
static MF_S32
get_prev_iframe(struct playState *ps, struct mf_frame *frame);

static struct mf_player g_stPlayer;

static void
pb_mem_init(struct mf_player *pstPb)
{
    struct mem_pool_t *pstMpool = mf_comm_mem_get();

    if (pstMpool->pbSize) {
        pstPb->pstMem = mf_mem_pool_create("mfpb", pstMpool->pbSize);
    }
}

static void
pb_mem_uninit(struct mf_player *pstPb)
{
    if (pstPb->pstMem) {
        mf_mem_state(pstPb->pstMem, MF_YES);
        mf_mem_pool_destory(pstPb->pstMem);
    }
}

static inline void *
pb_mem_alloc(MF_U32 size)
{
    void *p;

    if (g_stPlayer.pstMem) {
        p = mf_mem_alloc(g_stPlayer.pstMem, size);
    } else {
        p = ms_malloc(size);
    }

    return p;
}

static inline void *
pb_mem_calloc(MF_U32 n, MF_U32 size)
{
    void *p;

    if (g_stPlayer.pstMem) {
        p = mf_mem_calloc(g_stPlayer.pstMem, n, size);
    } else {
        p = ms_calloc(n, size);
    }

    return p;
}

static inline void
pb_mem_free(void *p)
{
    if (g_stPlayer.pstMem) {
        mf_mem_free(g_stPlayer.pstMem, p);
    } else {
        ms_free(p);
    }
}

static MF_S32
sleep_start(struct playState *ps, MF_U32 us)
{
    MF_S32 res = MF_SUCCESS;

    if (!us || us < 15000 / ps->speed) {
        return MF_SUCCESS;
    }
    ms_mutex_lock(&ps->sleepMutex);
    ps->bSleepBreak = MF_NO;
    mf_cond_timedwait(&ps->sleepCond, &ps->sleepMutex, us);
    if (ps->bSleepBreak == MF_YES) {
        res = MF_FAILURE;
    }
    ms_mutex_unlock(&ps->sleepMutex);

    return res;
}

static void
sleep_stop(struct playState *ps)
{
    ms_mutex_lock(&ps->sleepMutex);
    ps->bSleepBreak = MF_YES;
    mf_cond_signal(&ps->sleepCond);
    ps->frameTimer = get_time_us();
    ms_mutex_unlock(&ps->sleepMutex);
}

static void
play_yield(struct playState *ps)
{
    MFdbg("play_yield");
    ps->bYield = MF_YES;
    while (!ps->bFrozen) {
        sleep_stop(ps);
        usleep(20000);
    }
    MFdbg("play_yield done");
}

static void
play_resume(struct playState *ps)
{
    ps->bYield = MF_NO;
}

static void
update_sync_code(struct playState *ps)
{
    ps->syncCode = get_time_us();
}

static MF_BOOL
is_video(struct mf_frame *f)
{
    if (f->strm_type != ST_VIDEO) {
        return MF_NO;
    } else {
        return MF_YES;
    }
}

static MF_BOOL
is_iframe(struct mf_frame *f)
{
    if (is_video(f) == MF_NO) {
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

static inline MF_BOOL
check_sync_code(struct playState *ps, MF_U64 syncCode)
{
    return ps->syncCode == syncCode;
}

static inline MF_BOOL
check_frame_continuity(struct playState *ps, struct mf_frame *f)
{
    if (is_video(f)) {
        if (!f->width || !f->height) {
            return MF_NO;
        }
    }

    if (ps->enDir == PB_DIR_FORWARD) {
        if (ps->lastFrameTime > f->time_usec) {
            return MF_NO;
        } else if (ps->lastFrameTime == f->time_usec) {
            return  ps->bStep ? ps->bSeek : MF_YES;
        } else {
            return MF_YES;
        }
    } else {
        if (ps->lastFrameTime > f->time_usec) {
            return MF_YES;
        } else if (ps->lastFrameTime == f->time_usec) {
            return ps->bStep ? ps->bSeek : MF_YES;
        } else {
            return MF_NO;
        }
    }
}

static void play_list_dump(struct list_head *plist)
{
    struct stream_seg *s;
    list_for_each_entry(s, plist, node) {
        MFprint("\tstartTime[%s]\n", mf_time_to_string(s->startTime));
        MFprint("\tendTime[%s]\n", mf_time_to_string(s->endTime));
        MFprint("\tchnid[%d]\n", s->chnid);
        MFprint("\ttype[%d]\n", s->type);
        MFprint("\tevent[%d]\n", s->enEvent);
        MFprint("\t=======================\n");
    }
}

static MF_S32
stream_seek_frame(struct pbBuffer *b, struct stream_seg *s, MF_U64 us)
{
    if (s->startTime > us / 1000000 || s->endTime < us / 1000000) {
        return -1;
    }

    return mf_retr_seg_info_bsearch(b->data, b->size, us, INFO_MAJOR_IFRAME, SORT_IS_UP);
}

static void
requst_io_read(struct playState *ps, MF_U8 port, off64_t offset)
{
    return;
    struct ioReq *pstIOReq = ms_calloc(1, sizeof(struct ioReq));

    pstIOReq->fileOffset = offset;
    pstIOReq->pMutex    = &ps->IOmutex;
    pstIOReq->pCond     = &ps->IOcond;
    pstIOReq->bDone     = MF_NO;
    pstIOReq->port      = port;
    mf_disk_io_request(pstIOReq);
}

static MF_S32
stream_buff_refresh(struct playState *ps, MF_U32 off, MF_U32 delta)
{
    struct pbBuffer *b = &ps->stDataBuff;
    struct stream_seg *s = ps->curCacheStream;
    off64_t offset;
    MF_S32  res;

    if (s == NULL ||
        s->bDiscard == MF_YES ||
        s->pstDisk->enState != DISK_STATE_NORMAL) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }

    if (ps->bEOF == MF_YES) {
        return MF_FAILURE;
    }

    if (ps->enDir == PB_DIR_FORWARD) {
        if (s->recEndOffset <= off || s->recStartOffset > off) {
            ps->bEOF = MF_YES;
            return MF_FAILURE;
        }
        b->head = ALIGN_BACK(off, 512);
        b->size = ALIGN_BACK(MF_MIN(s->recEndOffset - b->head, PB_DATA_SIZE), 512);
        b->offset = off % 512;
    } else {
        delta = FRAME_DATA_MAX_SIZE(delta);
        if (off + delta <= s->recStartOffset || off + delta > s->recEndOffset) {
            ps->bEOF = MF_YES;
            return MF_FAILURE;
        }
        b->size = MF_MIN(off + delta - s->recStartOffset, PB_DATA_SIZE);
        b->head = off + delta - b->size;
        b->offset = off - b->head;
    }

    if (ps->bTail == MF_YES) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }

    offset = s->baseOffset + b->head;
    requst_io_read(ps, s->pstDisk->port, offset);
    res = s->pstDisk->ops->disk_file_read(s->pstDisk->pstPrivate,
                                          ps->fd, b->data, b->size, offset);
    if (res < 0) {
        MFerr("disk_read failed");
        ps->bAIO = MF_NO;
    }
    b->tail = b->head + b->size;
    ps->bEOF = MF_NO;
    ps->bTail = b->size != PB_DATA_SIZE ? MF_YES : MF_NO;

    return res >= 0 ? MF_SUCCESS : res;
}

static MF_S32
goto_the_pos(struct playState *ps, struct mf_info *info, MF_S32 index)
{
    struct pbBuffer *ib = &ps->stInfoBuff;
    struct pbBuffer *db = &ps->stDataBuff;
    struct info_Iframe *i = &info->stIframe;

    if (info->dataOffset >= db->head && info->dataOffset < db->tail) {
        db->offset = info->dataOffset - db->head;
    } else {
        if (stream_buff_refresh(ps, info->dataOffset, i->size) != MF_SUCCESS) {
            return MF_FAILURE;
        }
    }
    ib->index = index;
    ps->lastCacheIFrameTime = info->dataTime;
    ps->lastCacheIFrameIndex = index;
    MFinfo(" goto seek [%s]\n", mf_time_to_string(info->dataTime / 1000000));
    return MF_SUCCESS;
}

static MF_S32
get_iframe_info_buff(struct pbBuffer *b, MF_S8 *src, MF_S32 index, MF_U32 len)
{
    MF_S8 *off = NULL;
    MF_U32 size = 0;

    if (SORT_IS_UP) {
        size = (index + 1) * INFO_DATA_MAX_SIZE;
        off = src;
    } else {
        size = len - index * INFO_DATA_MAX_SIZE;
        off = (MF_S8 *)mf_retr_get_iframe_info(src, index);
    }
    if (!size) {
        MFwarm("info buff empty");
        return MF_FAILURE;
    }
    b->data = pb_mem_alloc(size);
    memcpy(b->data, off, size);

    b->size = size;
    b->count = len / INFO_DATA_MAX_SIZE;
    b->offset = 0;
    b->index = index;
    return MF_SUCCESS;
}

static void
put_iframe_info_buff(struct pbBuffer *b)
{
    if (b->data) {
        pb_mem_free(b->data);
    }
    b->data = NULL;
}

static MF_S32
find_neighbor_stream(struct player_node *p, struct stream_seg *curStream, MF_PTS t)
{

    struct stream_seg *s = NULL;
    struct playList *pl = p->pstPlayList;
    struct playState *ps = p->pstPlayState;

    if (curStream == NULL) {
        ps->nextCacheStream = NULL;
        ps->preCacheStream = NULL;
        list_for_each_entry(s, pl->head, node) {
            if (s->bDiscard == MF_YES) {
                continue;
            }
            if (s->startTime > t) {
                ps->nextCacheStream = s;
                break;
            }

            if (s->endTime < t) {
                ps->preCacheStream = s;
            }
        }
        return MF_SUCCESS;
    }

    if (pl->head != curStream->node.prev) {
        s = list_entry(curStream->node.prev, struct stream_seg, node);
        ps->preCacheStream = s;
    } else {
        ps->preCacheStream = NULL;
    }

    if (pl->head != curStream->node.next) {
        s = list_entry(curStream->node.next, struct stream_seg, node);
        ps->nextCacheStream = s;
    } else {
        ps->nextCacheStream = NULL;
    }


    return MF_SUCCESS;
}

static MF_S32
open_stream(struct playState *ps, struct stream_seg *s)
{
    if (mf_retr_stream_is_valid(s) == MF_NO) {
        return MF_FAILURE;
    }

    MF_U32  infoSize = s->infoStartOffset - s->infoEndOffset;
    off64_t offset = s->baseOffset + s->infoEndOffset;
    struct diskObj *pstDisk = s->pstDisk;
    MF_S32  res;
    MF_S32  index;
    MF_S8 *infoBuff = ms_valloc(infoSize);

    ps->fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, s->fileNo, ps->bAIO);

    requst_io_read(ps, pstDisk->port, offset);
    res = s->pstDisk->ops->disk_file_read(s->pstDisk->pstPrivate,
                                          ps->fd, infoBuff, infoSize, offset);
    if (res <= 0) {
        pb_mem_free(infoBuff);
        disk_close(ps->fd);
        ps->fd = 0;
        MFerr("disk_read failed");
        return MF_FAILURE;
    }
    mf_retr_seg_info_qsort(infoBuff, infoSize, SORT_IS_UP);

    index = mf_retr_last_Iframe_index(infoBuff, infoSize);
    if (index < 0) {
        pb_mem_free(infoBuff);
        disk_close(ps->fd);
        ps->fd = 0;
        return MF_FAILURE;
    }
    res = get_iframe_info_buff(&ps->stInfoBuff, infoBuff, index, infoSize);
    if (res != MF_SUCCESS) {
        pb_mem_free(infoBuff);
        disk_close(ps->fd);
        ps->fd = 0;
        return MF_FAILURE;
    }

    ps->curCacheStream = s;
    find_neighbor_stream(ps->owner, s, 0);
    ps->bEOF = MF_NO;
    ps->bTail = MF_NO;

    pb_mem_free(infoBuff);
    return MF_SUCCESS;
}

static void
close_stream(struct playState *ps)
{
    if (ps->curCacheStream) {
        put_iframe_info_buff(&ps->stInfoBuff);
        if (ps->fd > 0) {
            disk_close(ps->fd);
        }

        ps->fd = 0;
        ps->curCacheStream = NULL;
        ps->bEOF = MF_YES;
        ps->bTail = MF_YES;
    }
}

static MF_S32
load_prev_stream(struct player_node *p, struct stream_seg *s)
{
    struct playState *ps = p->pstPlayState;
    struct stream_seg *prev = NULL;
    MF_S32 res;
    MF_S32 index;

    prev = ps->preCacheStream;
    if (prev == NULL) {
        return MF_FAILURE;
    }

    close_stream(ps);
    res = open_stream(ps, prev);
    if (res == MF_FAILURE) {
        return res;
    }
    MFdbg("start find prev stream %s", mf_time_to_string(prev->startTime));
    //find last Iframe
    index = mf_retr_last_Iframe_index(ps->stInfoBuff.data, ps->stInfoBuff.size);
    struct mf_info *info = mf_retr_get_iframe_info(ps->stInfoBuff.data, index);
    res = goto_the_pos(ps, info, index);
    if (res == MF_SUCCESS) {
        ps->lastCacheTime = prev->lastIframeTime;
    }

    return res;
}

static MF_S32
load_next_stream(struct player_node *p, struct stream_seg *s)
{
    struct playState *ps = p->pstPlayState;
    struct stream_seg *next = NULL;
    MF_S32 res;

    next = ps->nextCacheStream;
    if (next == NULL) {
        return MF_FAILURE;
    }

    close_stream(ps);
    res = open_stream(ps, next);
    if (res == MF_FAILURE) {
        return res;
    }
    MFdbg("start find next stream %s", mf_time_to_string(next->startTime));

    res = stream_buff_refresh(ps, next->recStartOffset, 0);
    if (res == MF_SUCCESS) {
        ps->lastCacheTime = next->firstIframeTime;
    }

    return res;
}

static MF_S32
reload_stream_buff(struct playState *ps)
{
    MF_S32 res = MF_SUCCESS;
    struct pbBuffer *b = &ps->stDataBuff;

    sleep_stop(ps);
    ms_mutex_lock(&ps->mutex);
    update_sync_code(ps);
    ps->lastCacheTime = ps->lastFrameTime;
    ps->lastCacheIFrameTime = 0;

    if (ps->curCacheStream != ps->curPlayStream) {
        close_stream(ps);
        res = open_stream(ps, ps->curPlayStream);
        if (res == MF_FAILURE) {
            ms_mutex_unlock(&ps->mutex);
            return res;
        }
        res = stream_buff_refresh(ps, ps->lastFrameAddr, ps->lastFrameSize);
        ms_mutex_unlock(&ps->mutex);
        return res;
    }

    if (ps->lastFrameAddr == 0) {
        ms_mutex_unlock(&ps->mutex);
        return MF_FAILURE;
    }

    if (ps->lastFrameAddr >= b->head && ps->lastFrameAddr < b->tail) {
        b->offset = ps->lastFrameAddr - b->head;
        ps->bEOF = MF_NO;
    } else {
        res = stream_buff_refresh(ps, ps->lastFrameAddr, ps->lastFrameSize);
    }
    ms_mutex_unlock(&ps->mutex);
    sleep_stop(ps);

    return res;
}

static MF_S32
seek_iframe(struct playState *ps)
{
    struct pbBuffer *ib = &ps->stInfoBuff;
    MF_S32 index = ib->index;
    MF_FLOAT speed = ps->speed;
    MF_S32 delta = 1;

    if (speed > 64.0) {
        delta = 16 + 10 * ps->accelerate;
    } else if (speed > 32.0) {
        delta = 8 + 8 * ps->accelerate;
    } else if (speed > 16.0) {
        delta = 2 + 4 * ps->accelerate;
    } else {
        delta = 1 + 2 * ps->accelerate;
    }

    if (ps->enDir == PB_DIR_FORWARD) {
        if (SORT_IS_UP) {
            index = index + delta;
        } else {
            index = index - delta;
        }
    } else {
        if (SORT_IS_UP) {
            index = index - delta;
        } else {
            index = index + delta;
        }
    }

    return index;
}

static MF_S32
read_frame(struct playState *ps, struct mf_frame *frame)
{
    struct pbBuffer *b = &ps->stDataBuff;
    struct ps_frame_s stPsInfo;
    MF_S32 dataSize;
    MF_S32 res;
    struct player_node *player = ps->owner;

    if (ps->bEOF == MF_YES) {
        MFinfo("ps->bEOF == MF_YES\n");
        return MF_FAILURE;

    }
    if (b->size <= b->offset) {
        MFinfo("b->size[%d] <= b->offset[%d]chnid[%d]\n", b->size, b->offset, player->chnId);
        return MF_FAILURE;
    }

    res = ps_packet_get_info(b->data + b->offset,
                             b->size - b->offset,
                             &stPsInfo);
    if (res != MF_SUCCESS) {
        MFerr("chn[%d] bad frame", player->chnId);
        if (ps->enDir == PB_DIR_FORWARD) {
            res = get_next_iframe(ps, frame);
        } else {
            res = get_prev_iframe(ps, frame);
        }
        MFinfo("next i res[%d]", res);
        return res;
    }

    dataSize = stPsInfo.packet_size;
//    MFdbg("datasize = %d", dataSize);
//    MFdbg("b->offset = %d", b->offset);
//    MFdbg("b->size = %d", b->size);
    if (b->offset + dataSize <= b->size) {
        if (frame->data == NULL) {
            frame->data = pb_mem_calloc(1, stPsInfo.frame_size);
        }

        ps_packet_decoder(&stPsInfo, frame);

        frame->stream_from = SST_LOCAL_PB;
        frame->stream_format = STREAM_TYPE_MAINSTREAM;
        frame->ch = player->chnId;
        b->last = b->head + b->offset;
        b->offset += dataSize;
        ps->lastCacheSize = dataSize;

        return MF_SUCCESS;
    }

    return MF_FAILURE;
}

static MF_S32
get_frame(struct playState *ps, struct mf_frame *frame)
{
    struct pbBuffer *b = &ps->stDataBuff;
    MF_S32 res = MF_FAILURE;

    if (read_frame(ps, frame) == MF_SUCCESS) {
        return MF_SUCCESS;
    }

    if (stream_buff_refresh(ps, b->head + b->offset, 0) != MF_SUCCESS) {
        return MF_FAILURE;
    } else {
        res = get_frame(ps, frame);
    }

    return res;
}

static MF_S32
get_frame2(struct playState *ps, struct mf_info *info, struct mf_frame *frame)
{
    struct info_Iframe *i = &info->stIframe;
    MF_S32 res = MF_FAILURE;

    if (read_frame(ps, frame) == MF_SUCCESS) {
        return MF_SUCCESS;
    }

    if (stream_buff_refresh(ps, info->dataOffset, i->size) != MF_SUCCESS) {
        return MF_FAILURE;
    } else {
        res = get_frame2(ps, info, frame);
    }

    return res;
}

static MF_S32
get_next_iframe(struct playState *ps, struct mf_frame *frame)
{
    struct pbBuffer *ib = &ps->stInfoBuff;
    struct mf_info *info;
    MF_S32 index = ib->index;
    MF_S32 res = MF_FAILURE;

    if (!ib->data) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }

    if (index < 0 || index >= ib->count) {
        MFerr("000 size:%u offset:%u last:%u index:%d count:%d head:%u tail:%u", 
            ib->size, ib->offset, ib->last, index, ib->count,ib->head, ib->tail);
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }

    info = mf_retr_get_iframe_info(ib->data, index);
    if (info->dataTime != ps->lastCacheIFrameTime) {
        index = stream_seek_frame(ib, ps->curCacheStream,
                                  ps->lastCacheTime);
        ps->lastCacheIFrameTime = info->dataTime;

    }

    if (index < 0 || index >= ib->count) {
        MFerr("111 size:%u offset:%u last:%u index:%d count:%d head:%u tail:%u", 
            ib->size, ib->offset, ib->last, index, ib->count,ib->head, ib->tail);
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }
    
    info = mf_retr_get_iframe_info(ib->data, index);
    if (info->dataTime == ps->lastCacheIFrameTime) {
        if (info->dataTime == ps->lastCacheTime) {
            index = seek_iframe(ps);
        }
    }

    if (index < 0 || index >= ib->count) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }

    info = mf_retr_get_iframe_info(ib->data, index);
    if (goto_the_pos(ps, info, index) == MF_SUCCESS) {
        ps->lastCacheTime = info->dataTime;
        res = get_frame(ps, frame);
    }

    return res;
}

static MF_S32
get_prev_iframe(struct playState *ps, struct mf_frame *frame)
{
    struct pbBuffer *ib = &ps->stInfoBuff;
    struct mf_info *info;
    MF_S32 index = ib->index;
    MF_S32 res = MF_FAILURE;

    if (!ib->data) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }

    if (index < 0 || index >= ib->count) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }
    
    info = mf_retr_get_iframe_info(ib->data, index);
    if (info->dataTime != ps->lastCacheIFrameTime) {
        index = stream_seek_frame(ib, ps->curCacheStream,
                                  ps->lastCacheTime);
        ps->lastCacheIFrameTime = info->dataTime;
    }

    if (index < 0 || index >= ib->count) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }
    
    info = mf_retr_get_iframe_info(ib->data, index);
    if (info->dataTime == ps->lastCacheIFrameTime) {
        if (info->dataTime == ps->lastCacheTime) {
            index = seek_iframe(ps);
        }
    }
//    MFdbg("========index %d =total=[%d]===\n\n", index, ib->count);
    if (index < 0 || index >= ib->count) {
        ps->bEOF = MF_YES;
        return MF_FAILURE;
    }

    info = mf_retr_get_iframe_info(ib->data, index);
    if (goto_the_pos(ps, info, index) == MF_SUCCESS) {
        ps->lastCacheTime = info->dataTime;
        res = get_frame2(ps, info, frame);
    }

    return res;
}

static struct stream_seg *
find_stream(struct playList *pl, MF_PTS t)
{
    if (pl->startTime > t || pl->endTime < t) {
        return NULL;
    }

    struct stream_seg *s = NULL;

    list_for_each_entry(s, pl->head, node) {
        if (s->bDiscard == MF_YES) {
            continue;
        }
        if (s->startTime == s->endTime) {
            continue;
        }
        if (s->startTime <= t && s->endTime > t) {
            return s;
        }
    }

    return NULL;
}

static struct stream_seg *
find_backward_stream(struct playList *pl, MF_PTS t)
{
    if (pl->startTime > t || pl->endTime < t) {
        return NULL;
    }

    struct stream_seg *s = NULL;

    list_for_each_entry(s, pl->head, node) {
        if (s->bDiscard == MF_YES) {
            continue;
        }
        if (s->startTime == s->endTime) {
            continue;
        }
        if (s->startTime <= t && s->endTime >= t) {
            return s;
        }
    }

    return NULL;
}

static MF_S32 inline
compute_delay(struct playState *ps, struct mf_frame *f)
{
    MF_S32 duration = 0;
    MF_S32 delay = 0;
    MF_U64 currSysTime = 0;
    MF_U64 realtime = 0;
    MF_DOUBLE speed = ps->speed;

    if (is_video(f) == MF_NO) {
        return delay;
    }

    if (ps->pRealTimer) {
        realtime = (*(ps->pRealTimer));
        if (realtime > 0) {
            if (ps->enDir == PB_DIR_FORWARD) {
                if (realtime >= f->time_usec) {
                    delay = 0;
                } else {
                    delay = (f->time_usec - realtime) / speed;
                }
            } else {
                if (realtime <= f->time_usec) {
                    delay = 0;
                } else {
                    delay = (realtime - f->time_usec) / speed;
                }
            }
        }
    } else {
        currSysTime = get_time_us();
        //jump to new stream
        if (ps->curPlayStream->lastFrameTime < f->time_usec ||
            ps->curPlayStream->firstFrameTime > f->time_usec) {
            delay = 0;
            ps->frameTimer = currSysTime;
        } else {
            if ((ps->enDir == PB_DIR_FORWARD && ps->lastFrameTime > f->time_usec) ||
                (ps->enDir == PB_DIR_BACKWARD && ps->lastFrameTime < f->time_usec)) {
                MFdbg("time overlap.");
                delay = 0;
            } else {
                duration = abs(f->time_usec - ps->lastFrameTime);
                ps->frameTimer += (MF_U64)(duration / speed);
                if (ps->frameTimer <= currSysTime) {
                    delay = 0;
                } else {
                    delay = ps->frameTimer - currSysTime;
                }
            }
        }
    }

    if (!delay) {
        ps->accelerate++;
    } else {
        ps->accelerate = 0;
    }
//    MFshow("delay = %d, %llu, f->t[%llu] ps->t[%llu]\n", delay, realtime, f->time_usec, ps->lastFrameTime);
    return delay;
}


static MF_BOOL
is_play_next_stream(struct playState *ps)
{
    if (ps->curCacheStream == NULL) {
        return MF_NO;
    }

    struct stream_seg *curStream = ps->curCacheStream;
    struct stream_seg *preStream = ps->preCacheStream;
    struct stream_seg *nextStream = ps->nextCacheStream;

    if (ps->enDir == PB_DIR_FORWARD) {
        if (nextStream == NULL) {
            return MF_NO;
        } else if (ps->bAuto == MF_YES) {
            return MF_YES;
        } else {
            if (nextStream->startTime <= curStream->endTime) {
                return MF_YES;
            } else if (nextStream->startTime - curStream->endTime < 2 * SYNC_THRESHOLD_MAX / 1000000) {
                return MF_YES;
            } else {
                return MF_NO;
            }
        }
    } else {
        if (preStream == NULL) {
            return MF_NO;
        } else if (ps->bAuto == MF_YES) {
            return MF_YES;
        } else {
            if (preStream->endTime >= curStream->startTime) {
                return MF_YES;
            } else if (curStream->startTime - preStream->endTime < 2 * SYNC_THRESHOLD_MAX / 1000000) {
                return MF_YES;
            } else {
                return MF_NO;
            }
        }
    }

    return MF_NO;
}

static MF_S32
pb_get_frame(struct playState *ps, struct mf_frame *frame)
{
    MF_BOOL needIframe = MF_NO;
    MF_S32 res = MF_FAILURE;

    if (ps->bEOF == MF_YES) {
        return MF_FAILURE;
    }
    if (!ps->curCacheStream) {
        return MF_FAILURE;
    }
    do {
        if (ps->enDir == PB_DIR_FORWARD) {
            if (ps->bFullSpeed == MF_NO || needIframe) {
                res = get_next_iframe(ps, frame);
            } else {
                res = get_frame(ps, frame);
            }
        } else {
            res = get_prev_iframe(ps, frame);
        }

        if (res != MF_SUCCESS) {
            if (is_play_next_stream(ps) == MF_YES) {
                if (ps->enDir == PB_DIR_FORWARD) {
                    if ((ps->nextCacheStream->firstFrameTime - ps->curCacheStream->lastFrameTime) >= ps->lastDuration*2) {
                        needIframe = MF_YES;
                        MFdbg("need start from iframe\n");
                    }
                    res = load_next_stream(ps->owner, ps->curCacheStream);
                } else {
                    res = load_prev_stream(ps->owner, ps->curCacheStream);
                }
                if (res == MF_SUCCESS) {
                    continue;
                }
            }
        }
        break;

    } while (1);

    return res;
}

static void
pb_cache_free(struct pbCache *cache)
{
    if (!cache) {
        return;
    }
    if (cache->stFrame.data) {
        pb_mem_free(cache->stFrame.data);
    }
    pb_mem_free(cache);
}

static struct pbCache *
pb_cache_alloc(struct playState *ps)
{
    struct pbCache *cache = pb_mem_calloc(1, sizeof(struct pbCache));
    MF_S32 res;

    if (!cache) {
        return NULL;
    }
    ms_mutex_lock(&ps->mutex);
    res = pb_get_frame(ps, &cache->stFrame);
    if (res == MF_SUCCESS) {
        struct pbBuffer *b = &ps->stDataBuff;

        cache->cacheSize        = ps->lastCacheSize;
        cache->cacheAddr        = b->last;
        cache->cacheTime        = cache->stFrame.time_usec;
        cache->curCacheStream   = ps->curCacheStream;
        cache->preCacheStream   = ps->preCacheStream;
        cache->nextCacheStream  = ps->nextCacheStream;
        cache->syncCode         = ps->syncCode;
        cache->lastIframeIndex  = ps->lastCacheIFrameIndex;
        ps->lastDuration        = cache->stFrame.time_usec - ps->lastCacheTime;
        ps->lastCacheTime       = cache->cacheTime;
        if (is_iframe(&cache->stFrame) == MF_YES) {
            ps->lastCacheIFrameTime = ps->lastCacheTime;
        }
    } else {
        pb_cache_free(cache);
        ms_mutex_unlock(&ps->mutex);
        return NULL;
    }
    ms_mutex_unlock(&ps->mutex);
    return cache;
}

static void
pb_cache_push(struct playState *ps, struct pbCache *cache)
{
    ms_mutex_lock(&ps->cacheMutex);
    list_add_tail(&cache->node, &ps->cacheList);
    ps->cacheTotal += cache->cacheSize;
    ps->cacheIndex++;
    cache->sid = ps->cacheIndex;
    ms_mutex_unlock(&ps->cacheMutex);
}

static struct pbCache *
pb_cache_pop(struct playState *ps)
{
    struct pbCache *cache;

    ms_mutex_lock(&ps->cacheMutex);
    cache = list_first_entry_or_null(&ps->cacheList, struct pbCache, node);
    if (cache) {
        list_del(&cache->node);
        ps->cacheTotal -= cache->cacheSize;
    }
    ms_mutex_unlock(&ps->cacheMutex);

    return cache;
}

static void
pb_cache_reclaim(struct playState *ps, struct pbCache *cache)
{
    ms_mutex_lock(&ps->cacheMutex);
    list_add(&cache->node, &ps->cacheList);
    ps->cacheTotal += cache->cacheSize;
    ms_mutex_unlock(&ps->cacheMutex);
}

static void
pb_cache_clear(struct playState *ps)
{
    struct pbCache *cache;

    for (;;) {
        cache = pb_cache_pop(ps);
        if (cache) {
            pb_cache_free(cache);
        } else {
            break;
        }
    }
}

static void *
pb_av_play_task(void *argv)
{
    struct player_node *pstPlayer = argv;
    struct playState *ps = pstPlayer->pstPlayState;
    struct pbCache *cache;
    MF_S32 delay = 0;
    MF_U8  retry = 0;

    ms_task_set_name("pb_av_play_task");

    while (pstPlayer->bPlayRun) {
        if ((ps->bPaused && ps->bStep == MF_NO) || ps->bYield) {
            ps->bFrozen = MF_YES;
            usleep(50000);
            continue;
        } else {
            ps->bFrozen = MF_NO;
            cache = pb_cache_pop(ps);
            if (cache) {
                if (check_sync_code(ps, cache->syncCode) == MF_NO) {
                    pb_cache_free(cache);
                    continue;
                }

//                if (check_frame_continuity(ps, &cache->stFrame) == MF_NO)
//                {
//                    pb_cache_free(cache);
//                    continue;
//                }

                delay = compute_delay(ps, &cache->stFrame);
                if (delay >= 0) {
                    if (ps->bStep == MF_NO) {
                        if (sleep_start(ps, delay) == MF_FAILURE) {
                            pb_cache_reclaim(ps, cache);
                            continue;
                        }
                    }
                    if (check_sync_code(ps, cache->syncCode) == MF_YES) {
                        if (ps->bStep == MF_YES) {
#if defined(_HI3798_) //hi3798 SDK 3 frames of old data are left in vdec buffer;
                            if (is_iframe(&cache->stFrame)) {
                                retry = 3;
                            } else {
                                retry = 1;
                            }
#else
                            retry = 1;
#endif
                            while (retry--) {
                                g_stPlayer.func(pstPlayer->owner, &cache->stFrame);
                            }
                        } else {
                            g_stPlayer.func(pstPlayer->owner, &cache->stFrame);
                        }
                        if (g_stPlayer.bDebug)
                            MFdbg("chnid[%d] type[%d] size[%d] wh[%dx%d] frame uid[%d]\n",
                                  cache->stFrame.ch, cache->stFrame.frame_type, cache->stFrame.size,
                                  cache->stFrame.width, cache->stFrame.height, cache->stFrame.uid);
                        if (is_video(&cache->stFrame) == MF_YES) {
                            ps->bStop = MF_NO;
                            ps->curPlayStream   = cache->curCacheStream;
                            ps->nextPlayStream  = cache->nextCacheStream;
                            ps->prePlayStream   = cache->preCacheStream;
                            ps->lastFrameTime   = cache->stFrame.time_usec;
                            ps->lastFrameSize   = cache->cacheSize;
                            ps->lastFrameAddr   = cache->cacheAddr;
                            ps->bStep = MF_NO;
                            ps->bSeek = MF_NO;
//                            MFdbg("play=frame[%llu]==index[%d]=delay[%d]=play sid = %d\n", cache->stFrame.time_usec, cache->lastIframeIndex, delay,cache->sid);
                        }
                    }
                }
                pb_cache_free(cache);
                continue;
            } else {
                usleep(50000);
                ps->bStop = MF_YES;
            }
        }

    }
//    mf_pb_cmd_clear(pstPlayer);
    ps->bFrozen = MF_YES;
    MFdbg("pb_av_play_task[%d] exit", pstPlayer->chnId);
    return MF_SUCCESS;
}

static void *
pb_av_cache_task(void *argv)
{
    struct player_node *pstPlayer = argv;
    struct playState *ps = pstPlayer->pstPlayState;
    struct pbCache *cache;

    ms_task_set_name("pb_av_cache_task");
    while (pstPlayer->bCacheRun) {
        if (mf_pb_cmd_todo(pstPlayer) == MF_YES) {
            continue;
        }

        if (ps->cacheTotal < PB_CACHE_SIZE) {
            cache = pb_cache_alloc(ps);
            if (cache) {
                pb_cache_push(ps, cache);
//                MFdbg("cache=isVideo[%d] frame[%llu]==index[%d]=play sid = %d\n",is_video(&cache->stFrame), cache->stFrame.time_usec, cache->lastIframeIndex,cache->sid);
                continue;
            }
        }

        MF_USLEEP(50000);
    }

    pb_cache_clear(ps);
    mf_pb_cmd_clear(pstPlayer);
    MFdbg("pb_av_cache_task[%d] exit", pstPlayer->chnId);

    return MF_SUCCESS;
}

static void
pb_player_chn_add(struct player_node *pstPlayer)
{
    ms_mutex_lock(&g_stPlayer.mutex);
    list_add_tail(&pstPlayer->node, &g_stPlayer.list);
    ms_mutex_unlock(&g_stPlayer.mutex);
}

static void
pb_player_chn_del(struct player_node *pstPlayer)
{
    ms_mutex_lock(&g_stPlayer.mutex);
    list_del(&pstPlayer->node);
    ms_mutex_unlock(&g_stPlayer.mutex);
}

static void
pb_player_list_update(struct playList *pl)
{
    struct stream_seg *s = NULL;

    s = list_entry(pl->head->next, struct stream_seg, node);
    pl->startTime = s->startTime;
    s = list_entry(pl->head->prev, struct stream_seg, node);
    pl->endTime = s->endTime;
}

MF_S32
mf_pb_player_next_stream(struct player_node *pstPlayer)
{
    struct playState *ps = pstPlayer->pstPlayState;
    MF_S32 res = MF_FAILURE;


    ms_mutex_lock(&ps->mutex);
    if (ps->enDir == PB_DIR_FORWARD) {
        res = load_next_stream(pstPlayer, ps->curPlayStream);
    } else {
        res = load_prev_stream(pstPlayer, ps->curPlayStream);
    }

    if (res == MF_SUCCESS) {
        update_sync_code(ps);
    }

    ms_mutex_unlock(&ps->mutex);

    return res;
}

void
mf_pb_player_pause(struct player_node *pstPlayer)
{
    struct playState *ps = pstPlayer->pstPlayState;

    ps->bPaused = MF_YES;
    sleep_stop(ps);
    //mf_pb_player_speed(pstPlayer, 1.0, MF_YES, MF_NO);  //@zbing no need to do
    MFdbg("playback chn[%d]", pstPlayer->chnId);
}

void
mf_pb_player_play(struct player_node *pstPlayer)
{
    struct playState *ps = pstPlayer->pstPlayState;

    ps->frameTimer = get_time_us();
    ps->bPaused = MF_NO;
    sleep_stop(ps);
    MFdbg("playback chn[%d]", pstPlayer->chnId);
}

void
mf_pb_player_step(struct player_node *pstPlayer)
{
    struct playState *ps = pstPlayer->pstPlayState;

    ps->bStep = MF_YES;
    MFdbg("playback chn[%d]", pstPlayer->chnId);
}

void
mf_pb_player_mode(struct player_node *pstPlayer, MF_BOOL bAuto)
{
    struct playState *ps = pstPlayer->pstPlayState;
    ps->bAuto = bAuto;
    MFdbg("playback chn[%d]", pstPlayer->chnId);
}

void
mf_pb_player_realtimer(struct player_node *pstPlayer, MF_U64 *pRealTimer)
{
    struct playState *ps = pstPlayer->pstPlayState;
    ps->pRealTimer = pRealTimer;
}

MF_S32
mf_pb_player_seek(struct player_node *pstPlayer, MF_PTS time)
{
    struct stream_seg *stream = NULL;
    struct playList  *pl = pstPlayer->pstPlayList;
    struct playState *ps = pstPlayer->pstPlayState;
    MF_S32 index;
    MF_S32  res = MF_FAILURE;

    sleep_stop(ps);
    ms_mutex_lock(&ps->mutex);
    update_sync_code(ps);
    ps->seekTime = time;
    if (ps->curCacheStream == NULL
        || ps->curCacheStream->startTime > time
        || ps->curCacheStream->endTime < time) {
        close_stream(ps);
        //find new stream
        if (ps->enDir == PB_DIR_BACKWARD) {
            stream = find_backward_stream(pl, time);
        } else {
            stream = find_stream(pl, time);
        }
        if (stream) {
            res = open_stream(ps, stream);
            if (res == MF_FAILURE) {
                ms_mutex_unlock(&ps->mutex);
                return res;
            }
        } else {
            find_neighbor_stream(pstPlayer, NULL, time);
        }
    }

    if (ps->curCacheStream) {
        index = stream_seek_frame(&ps->stInfoBuff, ps->curCacheStream, (MF_U64)time * 1000000);
        if (index != -1) {
            ps->bEOF = MF_NO;
            ps->bTail = MF_NO;
            struct mf_info *info;
            info = mf_retr_get_iframe_info(ps->stInfoBuff.data, index);
            res = goto_the_pos(ps, info, index);
            if (res == MF_SUCCESS) {
                ps->lastFrameTime = info->dataTime;
                ps->bSeek = MF_YES;
            } else {
                ps->bEOF = MF_YES;
            }
        }
    }
    ps->curPlayStream = ps->curCacheStream;
    ps->prePlayStream = ps->preCacheStream;
    ps->nextPlayStream = ps->nextCacheStream;

    ms_mutex_unlock(&ps->mutex);
    MFinfo("playback chn[%d] seek [%s] res[%d]\n", pstPlayer->chnId,  mf_time_to_string(time), res);
    if (res == MF_FAILURE && ps->bAuto == MF_YES) {
        if (ps->enDir == PB_DIR_FORWARD && ps->nextPlayStream) {
            MFinfo("chn[%d] PB_DIR_FORWARD recursion", pstPlayer->chnId);
            res = mf_pb_player_seek(pstPlayer, ps->nextPlayStream->startTime);
        } else if (ps->enDir == PB_DIR_BACKWARD && ps->prePlayStream) {
            MFinfo("chn[%d] PB_DIR_BACKWARD recursion", pstPlayer->chnId);
            res = mf_pb_player_seek(pstPlayer, ps->prePlayStream->endTime);
        }
    }

    if (g_stPlayer.bDebug) {
        if (ps->curPlayStream) {
            MFinfo("playback cur startTime[%s]\n", mf_time_to_string(ps->curPlayStream->startTime));
            MFinfo("playback cur endTime[%s]\n", mf_time_to_string(ps->curPlayStream->endTime));
        }
        if (ps->prePlayStream) {
            MFinfo("playback prex startTime[%s]\n", mf_time_to_string(ps->prePlayStream->startTime));
            MFinfo("playback prex endTime[%s]\n", mf_time_to_string(ps->prePlayStream->endTime));
        }
        if (ps->nextPlayStream) {
            MFinfo("playback next startTime[%s]\n", mf_time_to_string(ps->nextPlayStream->startTime));
            MFinfo("playback next endTime[%s]\n", mf_time_to_string(ps->nextPlayStream->endTime));
        }
    }
    return res;
}


MF_S32
mf_pb_player_direction(struct player_node *pstPlayer, PB_DIR_EN enDir)
{
    struct playState *ps = pstPlayer->pstPlayState;

//    pb_player_pause(pstPlayer);
    if (ps->enDir == enDir) {
        return MF_FAILURE;
    }

    ps->enDir = enDir;
    ps->accelerate = 0;
    if (ps->bEOF) {
        mf_pb_player_seek(pstPlayer, ps->lastFrameTime / 1000000);
        printf("mf_pb_player_direction[%s]\n", mf_time_to_string(ps->lastFrameTime / 1000000));
    } else {
        reload_stream_buff(ps);
    }
//    mf_pb_player_seek(pstPlayer, ps->lastFrameTime/1000000);
//    pb_player_play(pstPlayer);

    return MF_SUCCESS;
}

MF_S32
mf_pb_player_speed(struct player_node *pstPlayer, MF_FLOAT speed, MF_BOOL bFullSpeed, MF_BOOL bSoft)
{
    struct playState *ps = pstPlayer->pstPlayState;

    if (ps->speed == speed && bFullSpeed == ps->bFullSpeed) {
        return MF_FAILURE;
    }

    if (bFullSpeed != ps->bFullSpeed) {
        update_sync_code(ps);
        reload_stream_buff(ps);
    }

    ps->speed = speed;
    ps->bFullSpeed = bFullSpeed;

    MFinfo("playback speed %f fullSpeed[%d]\n", ps->speed, ps->bFullSpeed);
    MFdbg("playback chn[%d]", pstPlayer->chnId);
    return MF_SUCCESS;
}

struct stream_seg *
mf_pb_get_stream(struct player_node *pstPlayer, MF_PTS time)
{
    struct stream_seg *s = NULL;

    s = find_stream(pstPlayer->pstPlayList, time);
    if (mf_retr_stream_is_valid(s) == MF_YES) {
        return s;
    } else {
        return NULL;
    }
}

struct pbCmd *
mf_pb_cmd_requst()
{
    return (struct pbCmd *)pb_mem_calloc(1, sizeof(pbCmd));
}

void
mf_pb_cmd_push(struct player_node *pstPlayer, struct pbCmd *pCmd)
{
    struct pbCmd *pos;
    struct pbCmd *n;

    switch (pCmd->enCmd) {
        case PB_CMD_PAUSE:
            mf_pb_player_pause(pstPlayer);
            pb_mem_free(pCmd);
            return;
        case PB_CMD_PLAY:
            mf_pb_player_play(pstPlayer);
            pb_mem_free(pCmd);
            return;
//        case PB_CMD_STEP:
//            mf_pb_player_step(pstPlayer);
//            pb_mem_free(pCmd);
//            return;
        default:
            break;
    }

    ms_mutex_lock(&pstPlayer->cmdMutex);

    list_for_each_entry_safe(pos, n, &pstPlayer->cmdList, node) {
        if (pos->enCmd == pCmd->enCmd) {
            list_del(&pos->node);
            pb_mem_free(pos);
        }
    }
    list_add_tail(&pCmd->node, &pstPlayer->cmdList);
    ms_mutex_unlock(&pstPlayer->cmdMutex);
}

struct pbCmd *
mf_pb_cmd_pop(struct player_node *pstPlayer)
{
    struct pbCmd *pCmd = NULL;

    ms_mutex_lock(&pstPlayer->cmdMutex);
    pCmd = list_first_entry_or_null(&pstPlayer->cmdList, struct pbCmd, node);
    if (pCmd) {
        list_del(&pCmd->node);
    }
    ms_mutex_unlock(&pstPlayer->cmdMutex);

    return pCmd;
}

void
mf_pb_cmd_clear(struct player_node *pstPlayer)
{
    struct pbCmd *pCmd;

    for (;;) {
        pCmd = mf_pb_cmd_pop(pstPlayer);
        if (pCmd) {
            pb_mem_free(pCmd);
        } else {
            break;
        }
    }
}

MF_BOOL
mf_pb_cmd_todo(struct player_node *pstPlayer)
{
    struct pbCmd *pCmd = mf_pb_cmd_pop(pstPlayer);

    if (pCmd == NULL) {
        return MF_NO;
    }

    play_yield(pstPlayer->pstPlayState);
    MFdbg("pb chn[%d] cmd[%d] begin\n", pstPlayer->chnId, pCmd->enCmd);
    switch (pCmd->enCmd) {
        case PB_CMD_SEEK:
            if (mf_pb_player_seek(pstPlayer, pCmd->time) == MF_SUCCESS) {
                if (pstPlayer->pstPlayState->bPaused) {
                    mf_pb_player_step(pstPlayer);    //display first frame
                }
            }
            break;
        case PB_CMD_SPEED:
            mf_pb_player_speed(pstPlayer, pCmd->speed, pCmd->bFullSpeed, pCmd->bSoft);
            break;
        case PB_CMD_PLAY:
            mf_pb_player_play(pstPlayer);
            break;
        case PB_CMD_PAUSE:
            mf_pb_player_pause(pstPlayer);
            break;
        case PB_CMD_START:
            mf_pb_player_start(pstPlayer, pCmd->time, pCmd->pTimer);
            break;
        case PB_CMD_STOP:
            mf_pb_player_stop(pstPlayer);
            break;
        case PB_CMD_STEP:
            mf_pb_player_step(pstPlayer);
            break;
        case PB_CMD_DIR:
            mf_pb_player_direction(pstPlayer, pCmd->enDir);
            break;
        default:
            break;
    }

    MFdbg("pb chn[%d] cmd[%d] end\n", pstPlayer->chnId, pCmd->enCmd);
    pb_mem_free(pCmd);
    play_resume(pstPlayer->pstPlayState);
    return MF_YES;
}

MF_S32
mf_pb_player_start(struct player_node *pstPlayer, MF_PTS time, MF_U64 *pRealTimer)
{
    MF_S32 res;
    MF_S8 strTime[32];

    time_to_string_local(time, strTime);
    mf_pb_player_realtimer(pstPlayer, pRealTimer);
    res = mf_pb_player_seek(pstPlayer, time);
    mf_pb_player_play(pstPlayer);
    MFdbg("pb_player_start time[%s] res[%d]", strTime, res);

    return res;
}

void
mf_pb_player_stop(struct player_node *pstPlayer)
{
    struct playState *ps = pstPlayer->pstPlayState;
    ps->bPaused = MF_YES;
    sleep_stop(ps);
}

struct player_node *
mf_pb_player_create(struct playList *pstPlayList)
{
    struct player_node *pstPlayer;

    MFdbg("mf_pb_player_create begin ...\n");
    pstPlayer = pb_mem_calloc(1, sizeof(struct player_node));

    pstPlayer->pstPlayState = pb_mem_calloc(1, sizeof(struct playState));
    pstPlayer->pstPlayState->owner = pstPlayer;
    //data
    pstPlayer->pstPlayState->stDataBuff.size = PB_DATA_SIZE;
    pstPlayer->pstPlayState->stDataBuff.data = ms_valloc(PB_DATA_SIZE);
    //tag
    pstPlayer->pstPlayState->stTagBuff.size = TAG_HEAD_MAX_SIZE;
    pstPlayer->pstPlayState->stTagBuff.data = ms_valloc(TAG_HEAD_MAX_SIZE);

    pstPlayer->pstPlayList = pb_mem_calloc(1, sizeof(struct playList));
    pstPlayer->pstPlayList->chnId = pstPlayList->chnId;
    pstPlayer->pstPlayList->head = pstPlayList->head;
    pstPlayer->pstPlayList->startTime = pstPlayList->startTime;
    pstPlayer->pstPlayList->endTime = pstPlayList->endTime;

    pstPlayer->chnId = pstPlayList->chnId;

    pstPlayer->pstPlayState->bPaused = MF_YES;
    pstPlayer->pstPlayState->bStop = MF_YES;
    pstPlayer->pstPlayState->bEOF  = MF_YES;
    pstPlayer->pstPlayState->bStep = MF_NO;
    pstPlayer->pstPlayState->bAuto = MF_NO;
    pstPlayer->pstPlayState->bAIO  = MF_YES;
    pstPlayer->pstPlayState->bFullSpeed = MF_YES;
    pstPlayer->pstPlayState->speed = 1.0;
    pstPlayer->pstPlayState->accelerate = 0;
    pstPlayer->pstPlayState->enDir = PB_DIR_FORWARD;

    ms_mutex_init(&pstPlayer->cmdMutex);
    INIT_LIST_HEAD(&pstPlayer->cmdList);

    find_neighbor_stream(pstPlayer, NULL, pstPlayList->startTime - 1);
    ms_mutex_init(&pstPlayer->pstPlayState->cacheMutex);
    INIT_LIST_HEAD(&pstPlayer->pstPlayState->cacheList);
    pstPlayer->bCacheRun = MF_YES;
    ms_task_create_join_stack_size(&pstPlayer->cachePthread, 256 * 1024, pb_av_cache_task, pstPlayer);
    mf_set_task_policy(pstPlayer->cachePthread, THTEAD_SCHED_RR_PRI + 5);

    ms_mutex_init(&pstPlayer->pstPlayState->mutex);
    ms_mutex_init(&pstPlayer->pstPlayState->IOmutex);
    ms_mutex_init(&pstPlayer->pstPlayState->sleepMutex);
    mf_cond_init(&pstPlayer->pstPlayState->sleepCond);
    mf_cond_init(&pstPlayer->pstPlayState->IOcond);
    pstPlayer->bPlayRun = MF_YES;
    ms_task_create_join_stack_size(&pstPlayer->playPthread, 256 * 1024, pb_av_play_task, pstPlayer);

    pb_player_chn_add(pstPlayer);
    MFdbg("mf_pb_player_create end ...\n");

    return pstPlayer;
}

void
mf_pb_player_destroy(struct player_node *pstPlayer)
{
    MFdbg("mf_pb_player_destroy chnid : %d pstPlayer:%p begin ...\n", pstPlayer->chnId, pstPlayer);
    pstPlayer->bPlayRun = MF_NO;
    pstPlayer->bCacheRun = MF_NO;
    sleep_stop(pstPlayer->pstPlayState);
    MFdbg("join cache task ...\n");
    ms_task_join(&pstPlayer->cachePthread);
    MFdbg("join play task ...\n");
    ms_task_join(&pstPlayer->playPthread);
    close_stream(pstPlayer->pstPlayState);

    ms_mutex_uninit(&pstPlayer->pstPlayState->mutex);
    ms_mutex_uninit(&pstPlayer->pstPlayState->IOmutex);
    ms_mutex_uninit(&pstPlayer->pstPlayState->sleepMutex);
    ms_mutex_uninit(&pstPlayer->pstPlayState->cacheMutex);
    mf_cond_uninit(&pstPlayer->pstPlayState->IOcond);
    mf_cond_uninit(&pstPlayer->pstPlayState->sleepCond);

    pb_player_chn_del(pstPlayer);
    pb_mem_free(pstPlayer->pstPlayState->stDataBuff.data);
    pb_mem_free(pstPlayer->pstPlayState->stTagBuff.data);
    pb_mem_free(pstPlayer->pstPlayState);
    pb_mem_free(pstPlayer->pstPlayList);
    pb_mem_free(pstPlayer);
    MFdbg("mf_pb_player_destroy end ...\n");
}

MF_S32
mf_pb_player_stream_add(struct player_node *pstPlayer, struct stream_seg *stream)
{
    MFdbg("mf_pb_player_stream_add %s", mf_time_to_string(stream->startTime));
    struct playState *ps = pstPlayer->pstPlayState;
    struct playList *pl = pstPlayer->pstPlayList;
    struct stream_seg *obj = NULL;

    OBJ_FIND_BY_STREAM(obj, stream, pl->head);
    if (obj == NULL) {
        ms_mutex_lock(&ps->mutex);

        LIST_INSERT_SORT_ASC(stream, pl->head, startTime);
        pb_player_list_update(pl);

        ms_mutex_unlock(&ps->mutex);
    }

    return MF_SUCCESS;

}

MF_S32
mf_pb_player_stream_del(struct player_node *pstPlayer, struct stream_seg *stream)
{
    struct playState *ps = pstPlayer->pstPlayState;
    struct playList *pl = pstPlayer->pstPlayList;
    struct stream_seg *obj = NULL;

    OBJ_FIND_BY_STREAM(obj, stream, pl->head);
    if (obj != NULL) {
        ms_mutex_lock(&ps->mutex);

        list_del(&stream->node);
        pb_player_list_update(pl);

        ms_mutex_unlock(&ps->mutex);
    }

    return MF_SUCCESS;
}

MF_S32
mf_pb_player_stat(struct player_node *pstPlayer, struct pb_stat_t *pstStat, MF_BOOL bBlock)
{
    struct playState *ps = pstPlayer->pstPlayState;

    if (bBlock == MF_YES) {
        ms_mutex_lock(&ps->mutex);
    } else if (ms_mutex_trylock(&ps->mutex) != 0) {
        return MF_FAILURE;
    }

    pstStat->curTime = ps->lastFrameTime;
    pstStat->bPaused = ps->bPaused;
    pstStat->bStop = (ps->bStop && ps->bEOF);// && !ps->bSeek);
    if (ps->enDir == PB_DIR_FORWARD) {
        pstStat->pstNextStream = ps->nextPlayStream;
    } else {
        pstStat->pstNextStream = ps->prePlayStream;
    }
//    MFdbg("chn[%d] bStop[%d] ps->stop[%d] ps->bEOF[%d]", pstPlayer->chnId, pstStat->bStop, ps->bStop, ps->bEOF);
    ms_mutex_unlock(&ps->mutex);

    return MF_SUCCESS;
}

struct item_tag_t *
mf_pb_alloc_tag_item(struct player_node *pstPlayer, MF_S8 *name, MF_PTS time, PB_ERR_EN *penErr)
{
    struct playState *ps = pstPlayer->pstPlayState;
    struct playList *pl = pstPlayer->pstPlayList;
    struct pbBuffer *b = &ps->stTagBuff;
    struct stream_seg *s = find_stream(pl, time);

    if (mf_retr_stream_is_valid(s) == MF_NO) {
        *penErr = PB_ERR_UNKNOWN;
        return NULL;
    }

    off64_t offset;
    struct mf_tag *tag;
    MF_S32 res;

    if (!strlen(name)) {
        *penErr = PB_ERR_UNKNOWN;
        return NULL;
    }

    if (s->startTime > time || s->endTime < time) {
        *penErr = PB_ERR_UNKNOWN;
        return NULL;
    }

    offset = s->baseOffset + TAG_HEAD_OFFSET;
    requst_io_read(ps, s->pstDisk->port, offset);
    res = s->pstDisk->ops->disk_file_read(s->pstDisk->pstPrivate,
                                          ps->fd, b->data, b->size, offset);
    if (res < 0) {
        *penErr = PB_ERR_UNKNOWN;
        MFerr("disk_read failed");
        return NULL;
    }

    MF_S32 i;
    if (s->pstFileInfo->tagNum == 0) {
        // disk initialized,  value of tag->bUsed is not clear
        memset(b->data, 0x0, TAG_HEAD_MAX_SIZE);
    }
    for (i = 0; i < TAG_MAX_NUM; i++) {
        tag = (struct mf_tag *)(b->data + i * TAG_HEAD_SIZE);
        if (tag->bUsed == MF_YES) {
            //clear old tag
            if (tag->time < s->pstFileInfo->fileStartTime
                || tag->time > s->pstFileInfo->fileEndTime) {
                tag->bUsed = MF_NO;
            }
        }
        if (tag->bUsed == MF_NO) {
            tag->id = i;
            struct item_tag_t *item = mf_retr_tag_alloc(tag, s, name, time);
            res = s->pstDisk->ops->disk_file_write(s->pstDisk->pstPrivate,
                                                   ps->fd, b->data, b->size, offset);
            if (res < 0) {
                mf_retr_tag_free(item);
                *penErr = PB_ERR_UNKNOWN;
                MFerr("disk_write failed");
                return NULL;
            } else {
                return item;
            }
        }
    }
    *penErr = PB_ERR_TAG_OVERFLOW;
    return NULL;
}

void
mf_pb_free_tag_item(struct item_tag_t *item)
{
    mf_retr_tag_free(item);
}

struct stream_seg *
mf_pb_stream_lock(struct player_node *pstPlayer, MF_PTS time)
{

    struct playList *pl = pstPlayer->pstPlayList;
    struct stream_seg *s;

    s = find_stream(pl, time);
    if (mf_retr_stream_is_valid(s) == MF_NO) {
        return NULL;
    }

    if (s->startTime > time || s->endTime < time) {
        return NULL;
    }

    mf_retr_seg_lock(s, MF_YES);

    return s;
}

/*
---------------------------------------------------------------------------------
 evnet process
*/
static MF_S32
pb_event_disk_format(void *argv)
{
    return MF_SUCCESS;
}

static void
pb_event_cb(MODULE_E from, EVENT_E event, void *argv)
{
    MFinfo("from = %d event = %d begin\n", from, event);
    switch (event) {
        case MSFS_EVENT_DISK_FORMAT:
            pb_event_disk_format(argv);
            break;
        default:
            break;
    }
    MFinfo("from = %d event = %d end\n", from, event);
}

void
mf_pb_event_notify(EVENT_E event, void *argv, MF_BOOL bBlock)
{
    if (bBlock == MF_NO) {
        mf_comm_task_notify(MODULE_PB, event, argv);
    } else {
        mf_comm_event_notify(MODULE_PB, event, argv);
    }
}

/*
---------------------------------------------------------------------------------
*/

void
mf_pb_dbg_switch(MF_BOOL bDebug)
{
    g_stPlayer.bDebug = bDebug;
}

void
mf_pb_dbg_show_mem_usage()
{
    if (g_stPlayer.pstMem) {
        MFinfo("pb_dbg_show_mem_usage");
        mf_mem_state(g_stPlayer.pstMem, MF_YES);
    }
}

void
mf_pb_dbg_show_chn()
{
    struct player_node *pos = NULL;
    struct player_node *n = NULL;
    MF_U32 count = 0;

    ms_mutex_lock(&g_stPlayer.mutex);
    list_for_each_entry_safe(pos, n, &g_stPlayer.list, node) {
        MFprint("handle ID[%d] tag[%p]\n", count++, pos);
        MFprint("chnid : %d\n", pos->chnId);
        play_list_dump(pos->pstPlayList->head);
        MFprint("state : %s\n", pos->pstPlayState->bPaused ? "Pause" : "Play");
        MFprint("bStop : %d\n", pos->pstPlayState->bStop);
        MFprint("bEOF : %d\n", pos->pstPlayState->bEOF);
        MFprint("bAuto : %d\n", pos->pstPlayState->bAuto);
        MFprint("bAIO : %d\n", pos->pstPlayState->bAIO);
    }
    ms_mutex_unlock(&g_stPlayer.mutex);
}


MF_S32
mf_pb_init(PB_OUT_CB func)
{

    memset(&g_stPlayer, 0, sizeof(struct mf_player));
    pb_mem_init(&g_stPlayer);
    INIT_LIST_HEAD(&g_stPlayer.list);
    ms_mutex_init(&g_stPlayer.mutex);
    g_stPlayer.func = func;
    mf_comm_event_register(MODULE_PB, pb_event_cb);

    return MF_SUCCESS;
}

MF_S32
mf_pb_deinit()
{
    struct player_node *pos = NULL;
    struct player_node *n = NULL;

    list_for_each_entry_safe(pos, n, &g_stPlayer.list, node) {
        mf_pb_player_stop(pos);
        mf_pb_player_destroy(pos);
    }
    ms_mutex_uninit(&g_stPlayer.mutex);
    mf_comm_event_unregister(MODULE_PB);
    pb_mem_uninit(&g_stPlayer);

    return MF_SUCCESS;
}



