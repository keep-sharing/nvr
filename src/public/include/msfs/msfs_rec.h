/*
 * ***************************************************************
 * Filename:        msfs_record.h
 * Created at:      2017.10.10
 * Description:     msfs record api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MSFS_RECORD_H__
#define __MSFS_RECORD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"
#include "msfs_notify.h"

#define MAX_SRC_NAME (64)
#define MAX_TAG_NAME (64)
#define MAX_LPR_NAME (64)
#define MAX_FACE_NUM (8)

typedef enum {
    FILE_TYPE_MAIN = 0,
    FILE_TYPE_SUB,
    FILE_TYPE_PIC,
    FILE_TYPE_NUM,
    FILE_TYPE_INIT = 255,
} FILE_TYPE_EN;

typedef enum {
    SEG_STATE_INVALID = 0,
    SEG_STATE_NORMAL = (1 << 0),
    SEG_STATE_LOCK = (1 << 1),
    SEG_STATE_ALL = ~0,
} SEG_STATE_EN;

typedef enum {
    DETEC_NONE = 0,
    DETEC_HUMAN = (1 << 0),
    DETEC_VEHICLE = (1 << 1),
    DETEC_ALL = ~0,
} DETEC_OBJ_EN;

typedef enum {
    REC_EVENT_NONE = 0U,
    REC_EVENT_TIME = (1U << 0),
    REC_EVENT_MOTION = (1U << 1),
    REC_EVENT_ALARMIN = (1U << 2),
    REC_EVENT_MANUAL = (1U << 3),
    REC_EVENT_ANR = (1U << 4),
    REC_EVENT_VCA = (1U << 5),
    REC_EVENT_SMART = (1U << 6),
    REC_EVENT_AUDIO_ALARM = (1U << 7),
    REC_EVENT_EVENT = (REC_EVENT_MOTION | REC_EVENT_ALARMIN | REC_EVENT_VCA | REC_EVENT_SMART | REC_EVENT_AUDIO_ALARM),
    REC_EVENT_ALL = ~0U,
} REC_EVENT_EN;

typedef enum {
    STID_MAJOR_IFRAME = 0,
    STID_MAJOR_MOTION = 1,
    STID_MAJOR_ALARMIN = 2,
    STID_MAJOR_PB_PIC = 3,
    STID_MAJOR_LIVE_PIC = 4,
    STID_MAJOR_TIME_PIC = 5,
    STID_MAJOR_VCA = 6,
    STID_MAJOR_SMART = 7,
    STID_MAJOR_IPC_ALARMIN = 8,
    STID_MAJOR_EVENT_PIC = 9,
    STID_MAJOR_AUDIO_ALARM = 10,
    MAX_STID_MAJOR,
} STID_MAJOR_EN;

typedef enum {
    INFO_MAJOR_NONE = 0U,
    INFO_MAJOR_IFRAME = (1U << 0),
    INFO_MAJOR_MOTION = (1U << 1),
    INFO_MAJOR_ALARMIN = (1U << 2),
    INFO_MAJOR_PB_PIC = (1U << 3),
    INFO_MAJOR_LIVE_PIC = (1U << 4),
    INFO_MAJOR_TIME_PIC = (1U << 5),
    INFO_MAJOR_VCA = (1U << 6),
    INFO_MAJOR_SMART = (1U << 7),
    INFO_MAJOR_IPC_ALARMIN = (1U << 8),
    INOF_MAJOR_EVENT_PIC = (1U << 9),
    INFO_MAJOR_AUDIO_ALARM = (1U << 10),
    INFO_MAJOR_PUSHMSG_PIC = (1U << 11),
    INFO_MAJOR_ALL = ~0U,
    INFO_MAJOR_INIT = (1U << 31),
    INFO_MAJOR_MASK = (~(INFO_MAJOR_INIT)),
} INFO_MAJOR_EN;

typedef enum {
    INFO_MINOR_NONE = 0,
    // VCA
    INFO_MINOR_REGIONIN = (1 << 0),
    INFO_MINOR_REGIONOUT = (1 << 1),
    INFO_MINOR_ADVANCED_MOTION = (1 << 2),
    INFO_MINOR_TAMPER = (1 << 3),
    INFO_MINOR_LINECROSS = (1 << 4),
    INFO_MINOR_LOITERING = (1 << 5),
    INFO_MINOR_HUMAN = (1 << 6),
    INFO_MINOR_PEOPLE_CNT = (1 << 7), // 9.0.17 deprecated, move to smart analysis
    INFO_MINOR_OBJECT = (1 << 8),
    INFO_MINOR_ALL_0 = (INFO_MINOR_REGIONIN | INFO_MINOR_REGIONOUT | INFO_MINOR_ADVANCED_MOTION | INFO_MINOR_TAMPER | INFO_MINOR_LINECROSS | INFO_MINOR_LOITERING | INFO_MINOR_HUMAN | INFO_MINOR_PEOPLE_CNT | INFO_MINOR_OBJECT),
    //Smart Analysis
    INFO_MINOR_LPR = (1 << 0),
    INFO_MINOR_FACE = (1 << 1),
    INFO_MINOR_IOT = (1 << 2),
    INFO_MINOR_POS = (1 << 3),
    INFO_MINOR_REGION = (1 << 4),
    INFO_MINOR_PCNT = (1 << 5),
    //snaphost schedule
    INFO_MINOR_PIC_MOTION = (1 << 0),
    INFO_MINOR_PIC_ALARM = (1 << 1),
    INFO_MINOR_PIC_VCA = (1 << 2),
    INFO_MINOR_PIC_LPR = (1 << 3),
    INFO_MINOR_PIC_AUDIO_ALARM = (1 << 4),
    INFO_MINOR_PIC_EVENT = (INFO_MINOR_PIC_MOTION | INFO_MINOR_PIC_ALARM | INFO_MINOR_PIC_VCA | INFO_MINOR_PIC_LPR | INFO_MINOR_PIC_AUDIO_ALARM),
    INFO_MINOR_ALL = ~0,
    INFO_MINOR_INIT = (1 << 31),
    INFO_MINOR_MASK = (~(INFO_MINOR_INIT)),
} INFO_MINOR_EN;

typedef enum PIC_EN {
    PIC_LIVE_CAP = 0,
    PIC_PB_CAP,
    PIC_TIME_CAP,
} PIC_EN;

typedef enum PIC_FORMAT_EN {
    PIC_FORMAT_JPEG = 100,
    PIC_FORMAT_YUV420P,
    PIC_FORMAT_H264,
    PIC_FORMAT_H265,
} PIC_FORMAT_EN;

typedef enum SNAP_EN {
    SNAP_LIVE = 0,
    SNAP_MOTION,
    SNAP_ALARMIN,
    SNAP_PB,
    SNAP_TIME,
} SNAP_EN;

typedef enum VID_EN {
    VID_TYPE_MAIN = 0,
    VID_TYPE_SUB,
    VID_TYPE_MAIN_ANR,
    VID_TYPE_SUB_ANR,
    VID_TYPE_NUM,
} VID_EN;

typedef enum HDL_EN {
    HDL_PIC = 1, // liveView picture
    HDL_VID,     // liveView video
    HDL_PB,      // playback picture
} HDL_EN;

typedef enum ENCODE_EN {
    ENCODE_UTF8 = 0,
    ENCODE_NUM,
} ENCODE_EN;

typedef enum REC_FAIL_EN {
    REC_QTA_FULL = 1,
    //others
} REC_FAIL_EN;

typedef struct jpg_frame {
    MF_U32 size;
    MF_U32 width;
    MF_U32 height;
    MF_S8 *data;
} jpg_frame;

typedef struct rec_res_t {
    MF_PTS  starTime;
    MF_PTS  endTime;
    MF_U32  size;
    MF_U8   port;
    MF_U8   chnId;
    MF_U32  majorType;
    MF_U32  minorType;
} rec_res_t;

typedef void (* REC_USER_CB)(EVENT_E, void *, HDL_EN, struct rec_res_t *);
typedef MF_S32(* REC_JPG_CB)(struct mf_frame *frame, INFO_MAJOR_EN enMajorType, MF_S8 **ppAddr, MF_U32 *pSize,
                             MF_U32 *pOutWidth, MF_U32 *pOutHeight);


typedef struct rec_hdl_t {
    VID_EN  enVid;  //main/sub/anr stream
    void *picHdl;   //picture handle
    void *vidHdl;   //video handle
} rec_hdl_t;

typedef struct rec_init_t {
    REC_USER_CB user_cb;    //user callback
    REC_JPG_CB  jpg_cb;    //jpg convert callback
    MF_U32      max_chn;   //max channel
} rec_init_t;

typedef struct rec_event_t {
    MF_S8   srcName[MAX_SRC_NAME];//event source name
    INFO_MAJOR_EN   enMajor;    //info major type
    INFO_MINOR_EN   enMinor;    //info minor type
    MF_BOOL bEnd;               //end flag
    MF_U64 startTime;           //info start time(us)
    MF_U64 endTime;             //info end time(us)
    void   *pstPrivate;          //user private data
    MF_U8  priSize;              //user private size , must be less than 96 Byte !!!!!
    DETEC_OBJ_EN enDtc;         //human or vehicle
} rec_event_t;

typedef struct rec_lpr_t {
    MF_S8 Licence[MAX_LPR_NAME]; //licence plate
    ENCODE_EN enEnctype;         //string encode type
    struct jpg_frame stMainPic;  //big picture info
    struct jpg_frame stSubPic;   //small picture info
    void   *pstPrivate;          //user private data
    MF_U8  priSize;              //user private size , must be less than 96 Byte !!!!!
    MF_U64 timeUs;               //PTS (us)
} rec_lpr_t;

typedef struct rec_face_t {
    struct jpg_frame stMainPic;                 //big picture info
    struct jpg_frame stSubPic[MAX_FACE_NUM];    //small picture info
    void   *pstPrivate[MAX_FACE_NUM];           //user private data
    MF_U8  priSize[MAX_FACE_NUM];               //user private size , each one must be less than 8 Byte !!!!!
    MF_U8  num;                                 // face num <= MAX_FACE_NUM;
    MF_U64 timeUs;                              //PTS (us)
} rec_face_t;

typedef struct rec_smart_t {
    struct mf_frame *frame;
    void *pstPrivate;
    MF_U8 priSize;
    MF_U64 startTime;
    MF_U64 endTime;
} rec_smart_t;

/**
 * \brief initialize record module after msfs_disk_init().
 */
MF_S32
msfs_rec_init(struct rec_init_t *pstInit);

/**
 * \brief deinitialize record module.
 */
MF_S32
msfs_rec_deinit();

/**
* \brief create a record handle when IPC connected.
* \param[in] chnId
* \param[in] enVid
* \param[in] bandwidth : max bitRate
* \return rec_hdl_t
*/
struct rec_hdl_t *
msfs_rec_create(MF_U8 chnId, VID_EN enVid, MF_U32 bandwidth);

/**
* \brief destroy specific record handle when IPC disconnected.
* \param[in] rec_hdl_t
* \param[in] enVid
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_destory(struct rec_hdl_t *rechdl);

/**
* \brief start recording by event trigger.
* \param[in] rechdl
* \param[in] enEvent
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_start(struct rec_hdl_t *rechdl, REC_EVENT_EN enEvent);

/**
* \brief stop recording by specific record handle.
* \param[in] rechdl
* \param[in] isForce :MF_YES force to stop record without watiing cache empty
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_stop(struct rec_hdl_t *rechdl, MF_BOOL isForce);

/**
* \brief update recording if necessary.
* \param[in] no
* \return no.
*/
void
msfs_rec_update();

/**
* \brief pour the frame data into recorder.
* \param[in] mf_frame
* \return MF_SUCCESS for success, MF_FAILURE for failure
*/
MF_S32
msfs_rec_write(struct mf_frame *frame);

/**
* \brief sanpshot a pic by event trigger at the runtime.
* \param[in] rechdl
* \param[in] enMajor
* \param[in] enMinor
* \param[in] pts
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_snapshot(struct rec_hdl_t *rechdl, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor, MF_PTS pts,
    MF_S8 *priData, MF_U8 priSize);

/**
* \brief sanpshot a pic with user(event) frame.
* \param[in] rechdl
* \param[in] enMajor
* \param[in] enMinor
* \param[in] pts
* \param[in] pstFrame : user frame data.
* \param[in] enFormat : YUV/JPEG
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_snapshot_ex(struct rec_hdl_t *rechdl, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor, MF_U64 pts,
                     struct mf_frame *pstFrame, PIC_FORMAT_EN enFormat);

/**
* \brief record a LPR event with two pictures.
* \param[in] rechdl
* \param[in] rec_lpr_t
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_event_lpr(struct rec_hdl_t *rechdl, struct rec_lpr_t *pstLpr);

/**
* \brief record a LPR event with two pictures.
* \param[in] rechdl
* \param[in] rec_lpr_t
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_event_face(struct rec_hdl_t *rechdl, struct rec_face_t *pstFace);

/**
* \brief record a POS event with two pictures.
* \param[in] rechdl
* \param[in] rec_smart_t
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_event_pos(struct rec_hdl_t *rechdl, struct rec_smart_t *pstPos);

/**
* \brief mark a event flag at the runtime.
* \param[in] rechdl
* \param[in] evt
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_mark_event(struct rec_hdl_t *rechdl, struct rec_event_t *evt);

/**
* \brief audio record  enbale att the runtime.
* \param[in] rechdl
* \param[in] bEnable
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_set_audio_enable(struct rec_hdl_t *rechdl, MF_BOOL bEnable);

/**
* \brief set pre record time at the runtime.
* \param[in] rechdl
* \param[in] PreTime (uint second)
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_set_pre_time(struct rec_hdl_t *rechdl, MF_U32 PreTime);

/**
* \brief set  record  video deadline time at the runtime.
* \param[in] rechdl
* \param[in] DeadDay (uint day) , 0: disable
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_set_vid_deadline_time(struct rec_hdl_t *rechdl, MF_U32 DeadDay);

/**
* \brief set  record picture deadline time at the runtime.
* \param[in] rechdl
* \param[in] DeadDay (uint day) , 0: disable
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_set_pic_deadline_time(struct rec_hdl_t *rechdl, MF_U32 DeadDay);

/**
* \brief set  record video deadline time by chnid at the static.
* \param[in] chnid
* \param[in] enVid
* \param[in] DeadDay (uint day) , 0: disable
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_set_vid_deadline_time_static(MF_U8 chnId, VID_EN enVid, MF_U32 DeadDay);

/**
* \brief set  record picture deadline time by chnid at the static.
* \param[in] chnid
* \param[in] DeadDay (uint day) , 0: disable
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_rec_set_pic_deadline_time_static(MF_U8 chnId, MF_U32 DeadDay);

/**
* \brief start dealine check task;
* \param[in] bStatic : MF_YES : use msfs_rec_set_xxx_deadline_time_static()
* \                           MF_NO  : use msfs_rec_set_xxx_deadline_time()
* \return none
*/
void
msfs_rec_deadline_check_start(MF_BOOL bStatic);

/**
* \brief stop dealine check task;
* \return none
*/
void
msfs_rec_deadline_check_stop();

/**
* \brief show spectify chnid's record info.
* \param[in] chnId : if chnid > max_rec_max(64), show all chn info
* \return none
*/
void
msfs_rec_dbg_rec_info(MF_U8 chnId);

/**
* \brief show record memory pool usage.
* \return none
*/
void
msfs_rec_dbg_mem_usage();

/**
* \brief set record  receive enable.
* \return none
*/
void
msfs_rec_dbg_set_receive(MF_BOOL bReceive);

/**
* \brief  record debug switch.
* \param[in] bDebug MF_YES for enable, MF_NO for disable
* \return none
*/
void
msfs_rec_dbg_switch(MF_BOOL bDebug);

/**
* \brief free all record cache.
* \always return MF_SUCCESS.
*/
MF_S32 msfs_rec_mem_outdate_free(void);


#ifdef __cplusplus
}
#endif

#endif

