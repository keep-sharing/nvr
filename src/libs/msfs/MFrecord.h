/*
 * ***************************************************************
 * Filename:        MFrecord.h
 * Created at:      2017.05.10
 * Description:     record api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFRECORD_H__
#define __MFRECORD_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "utils.h"
#include "msfs_rec.h"
#include "msfs_bkp.h"
#include "MFcommon.h"
#include "disk.h"
#include "ps_packet.h"

#define MAX_REC_CHN_NUM (64)
#define INIT_CHN_ID  (255)
//
#define LPR_PRIVATE_SIZE  (96)
#define FACE_PRIVATE_SIZE (8)
#define COM_PRIVATE_SIZE  (96)
#define MOTION_PRIVATE_SIZE  (196)
#define ALARM_PRIVATE_SIZE  (196)
#define VCA_PRIVATE_SIZE  (196)
#define SMART_PRIVATE_SIZE  (196)
#define AUDIO_ALARM_PRIVATE_SIZE  (196)
#define PIC_PRIVATE_SIZE  (96)
#define MAX_DATA_NUM (64)

typedef enum {
    FILE_STATE_USING = 0,
    FILE_STATE_USED,
    FILE_STATE_ANR,
    FILE_STATE_NUM,
    FILE_STATE_EMPTY = 255,
} FILE_STATE_EN;

typedef struct ai_pic {
    MF_U32 size;
    MF_U32 width;
    MF_U32 height;
    MF_U32 offset;
} ai_pic;

typedef struct info_Iframe {
    MF_U64  pts;
    MF_U32  size;
//    MF_U32  offset;
    MF_U32  width;
    MF_U32  height;
    MF_S32  encode;
} info_Iframe;

typedef struct info_alarm {
    MF_PTS  startTime;
    MF_PTS  endTime;
    MF_U8 priSize;
    MF_S8 Private[ALARM_PRIVATE_SIZE];
} info_alarm;

typedef struct info_motion {
    MF_PTS  startTime;
    MF_PTS  endTime;
    MF_U8 priSize;
    MF_S8 Private[MOTION_PRIVATE_SIZE];
} info_motion;

typedef struct info_pic {
    MF_PTS  time;
    MF_U32  size;
//    MF_U32  offset;
    MF_U32  width;
    MF_U32  height;
    MF_S32  encode;
    MF_S8 priData[PIC_PRIVATE_SIZE];
    MF_U8 priSize;
} info_pic;

typedef struct info_lpr {
    MF_S8 Licence[MAX_LPR_NAME];
    MF_S8 Private[LPR_PRIVATE_SIZE];
    MF_U8 priSize;
    ENCODE_EN enEnctype;
    struct ai_pic stMainPic;
    struct ai_pic stSubPic;
} info_lpr; // size 200Byte

typedef struct info_face {
    MF_U8 faceNum;
    MF_S8 Private[MAX_FACE_NUM][FACE_PRIVATE_SIZE];
    MF_U8 priSize[MAX_FACE_NUM];
    struct ai_pic stMainPic;
    struct ai_pic stSubPic[MAX_FACE_NUM];
} info_face; // size 220Byte

typedef struct info_common {
    MF_U64 startTime;
    MF_U64 endTime;
    MF_S8 Private[COM_PRIVATE_SIZE];
    MF_U8 priSize;
} info_common;

typedef struct info_vca {
    DETEC_OBJ_EN enDtc;
    MF_PTS  startTime;
    MF_PTS  endTime;
    MF_U8   priSize;
    MF_S8   Private[VCA_PRIVATE_SIZE];
} info_vca;

typedef struct info_smart {
    MF_PTS  startTime;
    MF_PTS  endTime;
    MF_U8   priSize;
    MF_S8   Private[SMART_PRIVATE_SIZE];
} info_smart;

typedef struct InfoAudioAlarm {
    MF_PTS  startTime;
    MF_PTS  endTime;
    MF_U8 priSize;
    MF_S8 Private[AUDIO_ALARM_PRIVATE_SIZE];
} INFO_AUDIO_ALARM_S;

typedef struct mf_data {
    void *owner;
    void *data[MAX_DATA_NUM];
    MF_U32 num;
} mf_data;

typedef struct mf_info {
    MF_S8   srcName[MAX_SRC_NAME];
    MF_U32  majorType;
    MF_U32  minorType;
    MF_BOOL bEnd;           // end of event ??
    union {
        struct info_Iframe  stIframe;
        struct info_alarm   stAlarm;
        struct info_motion  stMotion;
        struct info_pic     stPic;
        struct info_smart   stSmart;
        struct info_vca     stVca;
        struct info_lpr     stLpr;
        struct info_common  stCom;
        struct info_face    stFace;
        INFO_AUDIO_ALARM_S stAudioAlarm;
        MF_S8  dummy[256];
    };
    MF_U32  dataOffset;
    MF_U32  dataSize;
    MF_U64  dataTime;
    MF_U32  selfOffset;
} mf_info;

typedef struct mf_tag {
    MF_BOOL bUsed;
    MF_U8   id;
    MF_S8   name[MAX_TAG_NAME];
    MF_U8   chnId;
    MF_PTS  time;
} mf_tag;

typedef struct mf_segment { //size < 512 B
    MF_U32  id;
    MF_U32  status;
    MF_U32  recEvent;
    MF_U32  encodeType;
    MF_U32  resWidth;
    MF_U32  resHeight;
    MF_PTS  recStartTime;
    MF_PTS  recEndTime;
    MF_U64  firstIframeTime;
    MF_U64  lastIframeTime;
    MF_U32  firstIframeOffset;
    MF_U32  lastIframeOffset;
    MF_U32  recStartOffset;
    MF_U32  recEndOffset;
    MF_PTS  infoStartTime;
    MF_PTS  infoEndTime;
    MF_U32  infoCount;
    MF_U32  infoTypes;
    MF_U32  infoStartOffset;
    MF_U32  infoEndOffset;
    MF_U64  firstFrameTime;
    MF_U64  lastFrameTime;
    MF_U32  infoSubTypes;
} mf_segment;

typedef struct mf_file { //size < 32 KB
    MF_U32  magic;
    MF_U16  fileType; //0 mian ,1 sub, 2 pic
    MF_U16  fileState;
    MF_U16  segNum;
    MF_U16  lockNum;
    MF_PTS  recStartTime;
    MF_PTS  recEndTime;
    off64_t fileOffset;
    MF_U32  fileNo;
    MF_U32  infoStartOffset;
    MF_U32  infoEndOffset;
    MF_U32  recStartOffset;
    MF_U32  recEndOffset;
    MF_PTS  infoStartTime;
    MF_PTS  infoEndTime;
    MF_U32  infoTypes;
    MF_U32  recEvent;
    MF_U16  tagNum;
    MF_U8   chnId;
    MF_U32  infoSubTypes;
} mf_file;

typedef struct recHDL {
    HDL_EN enHDL;
    void  *pstRecObj;
    void  *owner;
    MF_U8 id;
    struct list_head node;
} recHDL;

typedef enum ERR_REC_CODE_E {
    ERR_REC_NOT_INIT             = 1,
    ERR_REC_INVALID_PARAM        = 2,
    ERR_REC_BAD_BLOCK            = 3,
    ERR_REC_BAD_DISK             = 4,
    ERR_REC_BUTT,
} ERR_REC_CODE_E;

#define MF_ERR_REC_INVALID_PARAM     MF_DEF_ERR(MF_MOD_ID_REC, MF_ERR_LEVEL_ERROR, ERR_REC_INVALID_PARAM)
#define MF_ERR_REC_BAD_BLOCK         MF_DEF_ERR(MF_MOD_ID_REC, MF_ERR_LEVEL_ERROR, ERR_REC_BAD_BLOCK)


struct recHDL *
mf_record_vid_create(MF_U8 chnId, VID_EN enVid, MF_U32 bandwidth);
void
mf_record_vid_destory(struct recHDL *rechdl);
MF_S32
mf_record_vid_start(struct recHDL *rechdl, REC_EVENT_EN enEvent);
MF_S32
mf_record_vid_soft_stop(struct recHDL *rechdl);
MF_S32
mf_record_vid_hard_stop(struct recHDL *rechdl, MF_BOOL bUser);
MF_S32
mf_record_vid_sync(MF_U8 chnId, FILE_TYPE_EN enType);
MF_S32
mf_record_vid_set_audio(struct recHDL *rechdl, MF_BOOL bAudio);
MF_S32
mf_record_vid_set_pre_time(struct recHDL *rechdl, MF_U32 PreTime);
MF_S32
mf_record_vid_set_deadline_time(struct recHDL *rechdl, MF_U32 DeadTime);
MF_S32
mf_record_pic_set_deadline_time(struct recHDL *rechdl, MF_U32 DeadTime);
MF_S32
mf_record_vid_set_deadline_time_static(MF_U8 chnId, VID_EN enVid, MF_U32 DeadTime);
MF_S32
mf_record_pic_set_deadline_time_static(MF_U8 chnId, MF_U32 DeadTime);
void
mf_record_deadline_check_start(MF_BOOL bStatic);
void
mf_record_deadline_check_stop();
MF_S32
mf_record_jpg_encoder(struct mf_frame *frame, INFO_MAJOR_EN enMajor, struct jpg_frame *jpg);
MF_S32
mf_record_vid_event(struct recHDL *rechdl, struct rec_event_t *evt);
struct recHDL *
mf_record_pic_create(MF_U8 chnId, HDL_EN enHdl);
void
mf_record_pic_destory(struct recHDL *rechdl);
MF_S32
mf_record_pic_start(struct recHDL *rechdl);
MF_S32
mf_record_pic_stop(struct recHDL *rechdl);
MF_S32
mf_record_pic_sync(MF_U8 chnId);
MF_S32
mf_record_pic_snap(struct recHDL *rechdl, VID_EN enVid, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor, MF_PTS pts,
    MF_S8 priData[], MF_U8 priSize);
MF_S32
mf_record_pic_lpr(struct recHDL *rechdl, struct rec_lpr_t *pstLpr);
MF_S32
mf_record_pic_face(struct recHDL *rechdl, struct rec_face_t *pstFace);
MF_S32
mf_record_vid_smart(struct recHDL *rechdl, struct rec_smart_t *pstSmart, MF_U32 minorType);
void
mf_record_vid_receive(struct mf_frame *pstFrame);
void
mf_record_pic_receive(struct mf_frame *pstFrame);
MF_S32
mf_record_pic_frame(struct recHDL *rechdl, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor,
    struct mf_frame *pstFrame, MF_S8 *priData, MF_U8 priSize);
void
mf_record_data_sink(struct mf_frame *pstFrame);
void
mf_record_update();
void
mf_record_event_notify(EVENT_E event, void *argv, MF_BOOL bBlock);
MF_S32
mf_record_init(REC_USER_CB user_cb, REC_JPG_CB jpg_cb, MF_U32 max_chn);
MF_S32
mf_record_deinit();
void
mf_record_dbg_show_chn_info(MF_U8 chnId);
void
mf_record_dbg_show_mem_usage();
void
mf_record_dbg_show_task_pool();
void
mf_record_dbg_receive_enable(MF_BOOL bReceive);
void
mf_record_dbg_switch(MF_BOOL bDebug);
MF_BOOL
mf_record_is_dbg();
MF_S8 *
mf_record_type(FILE_TYPE_EN enType);
MF_S8 *
mf_record_event(REC_EVENT_EN enEvent);
MF_S8 *
mf_record_ack(EVENT_E event);
MF_S32
mf_record_mem_outdate_free(void);


#ifdef __cplusplus
}
#endif

#endif



