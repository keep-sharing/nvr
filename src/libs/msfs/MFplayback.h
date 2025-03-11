/*
 * ***************************************************************
 * Filename:        MFplayback.h
 * Created at:      2017.08.23
 * Description:     playback api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFPLAYBACK_H__
#define __MFPLAYBACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "MFindex.h"
#include "MFrecord.h"
#include "MFretrieve.h"
#include "disk.h"
#include "msfs_pb.h"

typedef enum PB_STREAM_EN {
    PB_STREAM_MAIN = 0,
    PB_STREAM_SUB,
    PB_STREAM_PIC,
} PB_STREAM_EN;

typedef enum PB_STATE_EN {
    PB_STATE_STOP = 0,
    PB_STATE_PLAY = (1 < 1),
    PB_STATE_PAUSE = (1 < 2),
    PB_STATE_FAST = (1 < 3),
    PB_STTAE_SLOW = (1 < 4),
} PB_STATE_EN;

typedef enum PB_DIR_EN {
    PB_DIR_FORWARD = 0,
    PB_DIR_BACKWARD,
} PB_DIR_EN;

typedef enum PB_CMD_EN {
    PB_CMD_NONE = 0,
    PB_CMD_SEEK,
    PB_CMD_SPEED,
    PB_CMD_PLAY,
    PB_CMD_PAUSE,
    PB_CMD_START,
    PB_CMD_STOP,
    PB_CMD_STEP,
    PB_CMD_DIR,
    PB_CMD_SNAP,
} PB_CMD_EN;

typedef struct pbCmd {
    PB_CMD_EN   enCmd;
    MF_BOOL     bSoft;
    MF_BOOL     bPaused;
    MF_BOOL     bStep;
    MF_BOOL     bStop;
    MF_BOOL     bFullSpeed;
    MF_FLOAT    speed;
    PB_DIR_EN   enDir;
    MF_PTS      time;
    MF_U64      *pTimer;
    struct list_head node;
} pbCmd;

typedef struct pbBuffer {
    MF_U32      size;
    MF_U32      offset;
    MF_U32      last;
    MF_S32      index;
    MF_U32      count;
    MF_U32      head;
    MF_U32      tail;
    MF_S8       *data;
} pbBuffer;

typedef struct playState {
    MF_S32      fd;
    MF_BOOL     bAIO;
    MF_BOOL     bPaused;
    MF_BOOL     bStep;
    MF_BOOL     bYield;
    MF_BOOL     bFrozen;
    MF_BOOL     bEOF;
    MF_BOOL     bTail;
    MF_BOOL     bStop;
    MF_BOOL     bAuto;
    MF_BOOL     bSeek;
    MF_BOOL     bSleepBreak;
    MF_BOOL     bFullSpeed;
    MF_PTS      seekTime;
    MF_FLOAT    speed;
    PB_DIR_EN   enDir;
    MF_S32      accelerate;
    MF_S32      lastDuration;
    MF_U32      lastFrameAddr;
    MF_U32      lastFrameSize;
    MF_U64      lastFrameTime;
    MF_U64      lastCacheTime;
    MF_U32      lastCacheSize;
    MF_U64      lastCacheIFrameTime;
    MF_S32      lastCacheIFrameIndex;
    MF_U64      frameTimer;
    MF_U64      *pRealTimer;
    MF_U64      syncCode;
    MF_MUTEX_T       mutex;
    MF_MUTEX_T       sleepMutex;
    MF_COND_T        sleepCond;
    void        *owner;
    struct stream_seg *prePlayStream;
    struct stream_seg *curPlayStream;
    struct stream_seg *nextPlayStream;
    struct stream_seg *preCacheStream;
    struct stream_seg *curCacheStream;
    struct stream_seg *nextCacheStream;
    struct pbBuffer  stDataBuff;
    struct pbBuffer  stInfoBuff;
    struct pbBuffer  stTagBuff;

    struct list_head  cacheList;
    MF_MUTEX_T        cacheMutex;
    MF_U32            cacheTotal;
    MF_S32            cacheIndex;

    MF_MUTEX_T        IOmutex;
    MF_COND_T         IOcond;
} playState;

typedef struct player_node {
    MF_U32           handle;
    MF_U8            chnId;
    PB_STREAM_EN     enStream;
    TASK_HANDLE      playPthread;
    TASK_HANDLE      cachePthread;
    TASK_HANDLE      cmdPthread;
    MF_BOOL          bPlayRun;
    MF_BOOL          bCacheRun;
    MF_BOOL          bCmdRun;
    pb_hdl_t        *owner;
    struct playState *pstPlayState;
    struct playList *pstPlayList;
    struct list_head node;
    MF_MUTEX_T       cmdMutex;
    struct list_head cmdList;
} player_node;


typedef enum ERR_PB_CODE_E {
    ERR_PB_NOT_INIT             = 1,
    ERR_PB_INVALID_PARAM        = 2,
    ERR_PB_BAD_FRAME            = 3,
    ERR_PB_BUTT,
} ERR_PB_CODE_E;

#define MF_ERR_PB_INVALID_PARAM     MF_DEF_ERR(MF_MOD_ID_PB, MF_ERR_LEVEL_ERROR, ERR_PB_INVALID_PARAM)
#define MF_ERR_PB_BAD_FRAME         MF_DEF_ERR(MF_MOD_ID_PB, MF_ERR_LEVEL_ERROR, ERR_PB_BAD_FRAME)

void
mf_pb_event_notify(EVENT_E event, void *argv, MF_BOOL bBlock);
MF_S32
mf_pb_init(PB_OUT_CB func);
MF_S32
mf_pb_deinit();
struct player_node *
mf_pb_player_create(struct playList *pstPlayList);
void
mf_pb_player_destroy(struct player_node *pstPlayer);
void
mf_pb_player_pause(struct player_node *pstPlayer);
void
mf_pb_player_play(struct player_node *pstPlayer);
void
mf_pb_player_step(struct player_node *pstPlayer);
MF_S32
mf_pb_player_seek(struct player_node *pstPlayer, MF_PTS time);
void
mf_pb_player_mode(struct player_node *pstPlayer, MF_BOOL bAuto);
void
mf_pb_player_realtimer(struct player_node *pstPlayer, MF_U64 *pRealTimer);
MF_S32
mf_pb_player_next_stream(struct player_node *pstPlayer);
MF_S32
mf_pb_player_direction(struct player_node *pstPlayer, PB_DIR_EN enDir);
MF_S32
mf_pb_player_speed(struct player_node *pstPlayer, MF_FLOAT speed, MF_BOOL bFullSpeed, MF_BOOL bSoft);
MF_S32
mf_pb_player_start(struct player_node *pstPlayer, MF_PTS time, MF_U64 *pRealTimer);
void
mf_pb_player_stop(struct player_node *pstPlayer);
struct stream_seg *
mf_pb_get_stream(struct player_node *pstPlayer, MF_PTS time);
struct pbCmd *
mf_pb_cmd_requst();
void
mf_pb_cmd_push(struct player_node *pstPlayer, struct pbCmd *pCmd);
struct pbCmd *
mf_pb_cmd_pop(struct player_node *pstPlayer);
void
mf_pb_cmd_clear(struct player_node *pstPlayer);
MF_BOOL
mf_pb_cmd_todo(struct player_node *pstPlayer);
MF_S32
mf_pb_player_stream_add(struct player_node *pstPlayer, struct stream_seg *stream);
MF_S32
mf_pb_player_stream_del(struct player_node *pstPlayer, struct stream_seg *stream);
MF_S32
mf_pb_player_stat(struct player_node *pstPlayer, struct pb_stat_t *pstStat, MF_BOOL bBlock);
struct item_tag_t *
mf_pb_alloc_tag_item(struct player_node *pstPlayer, MF_S8 *name, MF_PTS time, PB_ERR_EN *penErr);
void
mf_pb_free_tag_item(struct item_tag_t *item);
struct stream_seg *
mf_pb_stream_lock(struct player_node *pstPlayer, MF_PTS time);
void
mf_pb_dbg_switch(MF_BOOL bDebug);
void
mf_pb_dbg_show_mem_usage();
void
mf_pb_dbg_show_chn();

#ifdef __cplusplus
}
#endif

#endif


