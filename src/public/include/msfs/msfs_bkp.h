/* 
 * ***************************************************************
 * Filename:      	msfs_backup.h
 * Created at:    	2017.10.10
 * Description:   	msfs backup api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __MSFS_BKP_H__
#define __MSFS_BKP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"
#include "msfs_rec.h"
#include "msfs_disk.h"
#include "msfs_notify.h"

#define BKP_MAX_RESULT_NUM   (8640)

typedef struct bkp_pri_t{
    MF_U8 size;
    void *data;
}bkp_pri_t;

typedef MF_BOOL  (* EVT_MATCH)( struct bkp_pri_t *dst, struct bkp_pri_t *src);  // return MF_YES or MF_NO
typedef MF_S32 (* RETR_JPG_CB)(struct mf_frame * frame, MF_U32 w, MF_U32 h, MF_U8 **ppAddr, MF_U32 *pSize, MF_U32 *pOutWidth, MF_U32 *pOutHeight);

typedef struct stream_seg{
    MF_BOOL bDiscard;           //discard when data is overwrite
    MF_U8   chnid;              // channel id
    MF_U16  type;               // main/sub/pic
    MF_U32  fileNo;             // file No.
    MF_U32  encodeType;         // h264/h265
    MF_U32  resWidth;           //resolution width
    MF_U32  resHeight;          //resolution height
    REC_EVENT_EN enEvent;       // event type
    SEG_STATE_EN enState;       //lock / unlock
    MF_PTS  startTime;          // stream start time (s)
    MF_PTS  endTime;            //stream end time(s)
    off64_t baseOffset;         //data base offset
    MF_U32  infoStartOffset;    //info start relative offset
    MF_U32  infoEndOffset;      //info end relative offset
    MF_U32  recStartOffset;     //data start relative offset
    MF_U32  recEndOffset;       //data end relative offset
    MF_U32  size;               //stream size (data+info)
    MF_U32  firstIframeOffset;  // first I frame relative offset
    MF_U32  lastIframeOffset;   // last I frame relative offset
    MF_U64  firstIframeTime;    // first I frame timestamp(us)
    MF_U64  lastIframeTime;     //last I frame timestamp(us)
    MF_U64  firstFrameTime;    // first  frame timestamp(us)
    MF_U64  lastFrameTime;     //last  frame timestamp(us)
    struct seg_info *pstSegInfo; // owner segment info 
    struct file_info *pstFileInfo; // owner file info 
    struct ipc_info *pstIpcInfo; // owner ipc info 
    struct diskObj  *pstDisk;   // owner disk info 
    struct list_head node;      //list : struct item_comm_t/item_evt_t/item_pic_t
}stream_seg;

typedef struct src_node_t{
    MF_S8 srcName[MAX_SRC_NAME]; //event source name
    struct list_head node;       //list : struct bkp_event_t
}src_node_t;

//扩展参数与底层搜索无关，直接修改上层范围太大先放这里
typedef struct BkpReqUserCfg {
    MF_BOOL all;
    MF_BOOL close;
} BKP_USER_CFG;

typedef struct bkp_comm_t{
    MF_U64  ipcMask;        // channel(0~63) mask 
    MF_U32  ipcNum;         //channel num
    MF_BOOL bCancel;        //stop searching  
    FILE_TYPE_EN enType;    // main/sub/pic
    REC_EVENT_EN enEvent;   //event  segment
    SEG_STATE_EN enState;   // lock/unlock
    MF_TIME     time;       // start/end time(s)
    MF_U8 searchfrom;       //who search
    MF_U8 searchid;         //id
    BKP_USER_CFG userCfg;
}bkp_comm_t;

typedef struct bkp_event_t{
    MF_U64  ipcMask;        //channel(0~63)mask
    MF_U32  ipcNum;         //channel num
    MF_BOOL bCancel;        //stop searching  
    FILE_TYPE_EN enType;    //main/sub/pic
    INFO_MAJOR_EN enMajor;  //event major type
    INFO_MINOR_EN enMinor;  //event minor type
    MF_TIME time;           //start/endTime(s)
    MF_U32  ahead;          //ahead time(s)
    MF_U32  delay;          //delay time(s)
    MF_U32  srcCount;       // event source count
    struct list_head srcList;//node :struct src_node_t
    MF_U8 searchfrom;       //who search
    MF_U8 searchid;         //id
    EVT_MATCH   match;      // custom match function;
    struct bkp_pri_t  pri; //private data
    DETEC_OBJ_EN enDtc;		//human/vehicle
    BKP_USER_CFG userCfg;
    MF_U32 maxNum;
}bkp_event_t;

typedef struct bkp_vca_t {
    SMART_EVENT_TYPE event;
    DETEC_OBJ_EN dec;
}bkp_vca_t;

typedef struct bkp_pic_t{
    MF_U64  ipcMask;        //channel(0~63)mask
    MF_U32  ipcNum;         //channel num
    MF_BOOL bCancel;        //stop searching  
    INFO_MAJOR_EN enMajor;  //pic major type
    INFO_MINOR_EN enMinor;  //pic minor type
    MF_TIME time;           //start/end time(s)
    MF_U8 searchfrom;       //who search
    MF_U8 searchid;         //id
    BKP_USER_CFG userCfg;
    struct bkp_pri_t pri;   // private data
    struct bkp_vca_t vca;        //Vca detec object
    MF_U32 maxNum;          //max search num
}bkp_pic_t;

typedef struct bkp_lpr_t{
    MF_U64  ipcMask;        //channel(0~63)mask
    MF_U32  ipcNum;         //channel num
    MF_BOOL bCancel;        //stop searching  
    MF_S8   keyWord[MAX_LPR_NAME]; //key word string
    MF_TIME time;           //start/end time(s)
    MF_U8 searchfrom;       //who search
    MF_U8 searchid;         //id
    EVT_MATCH   match;      // custom match function;
    struct bkp_pri_t  pri; //private data
    BKP_USER_CFG userCfg;
}bkp_lpr_t;

typedef struct bkp_face_t{
    MF_U64  ipcMask;        //channel(0~63)mask
    MF_U32  ipcNum;         //channel num
    MF_BOOL bCancel;        //stop searching  
    MF_TIME time;           //start/end time(s)
    MF_U8 searchfrom;       //who search
    MF_U8 searchid;         //id
    EVT_MATCH   match;      // custom match function;
    struct bkp_pri_t  pri; //private data
    BKP_USER_CFG userCfg;
}bkp_face_t;

typedef struct bkp_smart_t{
    MF_U64  ipcMask;        //channel(0~63)mask
    MF_U32  ipcNum;         //channel num
    MF_BOOL bCancel;        //stop searching  
    MF_TIME time;           //start/end time(s)
    MF_U8 searchfrom;       //who search
    MF_U8 searchid;         //id
    FILE_TYPE_EN enType;    //main/sub/pic
    INFO_MINOR_EN enMinor;  //minor type
    EVT_MATCH info_match;   // custom match function;
    EVT_MATCH strm_match;
    struct bkp_pri_t infoPri; //private data
    struct bkp_pri_t strmPri;
    BKP_USER_CFG userCfg;
}bkp_smart_t;

typedef struct bkp_tag_t{
    MF_U64  ipcMask;        //channel(0~63)mask
    MF_U32  ipcNum;         //channel num
    MF_BOOL bCancel;        //stop searching  
    MF_S8   keyWord[MAX_TAG_NAME]; //key word string
    FILE_TYPE_EN enType;    //main/sub/pic
    MF_TIME time;           //start/end time(s)
    MF_U8 searchfrom;       //who search
    MF_U8 searchid;         //id
    BKP_USER_CFG userCfg;
}bkp_tag_t;

typedef struct item_comm_t{
    MF_U8   chnId;          //channel id
    MF_U8   port;           //disk port id
    MF_TIME time;           //start/end time(s)
    MF_U32  size;           //item size
    MF_BOOL isLock;         //lock/Unlock
    MF_U32  streamCount;    //contain stream count 
    MF_U32  encodeType;     // h264/h265
    struct list_head streamList;//node : struct stream_seg
    struct list_head node;//list : struct bkp_node_t
}item_comm_t;

typedef struct item_evt_t{
    MF_S8   srcName[MAX_SRC_NAME];//event source name 
    MF_U8   chnId;          //channel id
    MF_U8   port;           //disk port id
    MF_TIME time;           //start / end time (s)
    MF_U64  size;           // item size
    INFO_MAJOR_EN enMajor;  //major type
    INFO_MINOR_EN enMinor;  //minor type
    MF_U8   priSize;        //private  size;
    void   *pstPrivate;     // private data
    MF_U32  streamCount;    //contain stream count
    MF_TIME streamTime;     //streams start / end time (s)
    struct list_head streamList;//node : struct stream_seg
    struct list_head node;//list : struct bkp_node_t
    MF_U32 objectType;
}item_evt_t;

typedef struct item_pic_t{
    MF_U8   chnId;          //channel id
    MF_U8   port;           //disk port id
    INFO_MAJOR_EN enMajor;  //pic major type
    INFO_MINOR_EN enMinor;  //pic minor type
    MF_U64  size;           //pic size
    MF_U64  pts;            //start/end time(us)
    MF_U32  width;          //pic width
    MF_U32  height;         //pic height
    struct stream_seg *stream;//stream detail info
    struct list_head node;//list : struct bkp_node_t
    MF_U32  objectType;
}item_pic_t;

typedef struct item_tag_t{
    MF_U8   id;
    MF_S8   name[MAX_TAG_NAME];
    MF_U8   chnId;
    MF_PTS  time;
    struct stream_seg *stream;//stream detail info
    struct list_head node;
}item_tag_t;

typedef struct item_lpr_t{
    MF_S8   licence[MAX_LPR_NAME];//LPR li name 
    MF_U8   chnId;          //channel id
    MF_U8   port;           //disk port id
    MF_U8   priSize;        //private  size;
    void   *pstPrivate;     // private data
    MF_U32  fileNo;         // file No.
    MF_U32  picOffset;      // picture offset in file
    MF_U32  picMainSize;    // main picture size
    MF_U32  picSubSize;     // sub picture size
    MF_U32  picMainWidth;   // main picture width
    MF_U32  picMainHeight;  // main picture height
    MF_U32  picSubWidth;    // sub picture width
    MF_U32  picSubHeight;   // sub picture height
    MF_U64  pts;            //time us
    struct seg_info *pstSegInfo; // owner segment info 
    struct file_info *pstFileInfo; // owner file info 
    struct ipc_info *pstIpcInfo; // owner ipc info 
    struct diskObj  *pstDisk;   // owner disk info 
    struct list_head node;//list : struct bkp_node_t
}item_lpr_t;

typedef struct item_face_t{
    MF_U8   chnId;          //channel id
    MF_U8   port;           //disk port id
    MF_U8   priSize;        //private  size;
    MF_U8   priData[64];    // private data
    MF_U32  fileNo;         // file No.
    MF_U32  picMainOffset;  // main picture offset in file
    MF_U32  picSubOffset;   // main picture offset in file
    MF_U32  picMainSize;    // main picture size
    MF_U32  picSubSize;     // sub picture size
    MF_U32  picMainWidth;   // main picture width
    MF_U32  picMainHeight;  // main picture height
    MF_U32  picSubWidth;    // sub picture width
    MF_U32  picSubHeight;   // sub picture height
    MF_U64  pts;            //time us
    struct seg_info *pstSegInfo; // owner segment info 
    struct file_info *pstFileInfo; // owner file info 
    struct ipc_info *pstIpcInfo; // owner ipc info 
    struct diskObj  *pstDisk;   // owner disk info 
    struct list_head node;//list : struct bkp_node_t
}item_face_t;

typedef struct item_smt_t {
    MF_U8   chnId;          //channel id
    MF_U8   port;           //disk port id
    MF_U8   priSize;        //private  size;
    void   *pstPrivate;     //private info data    
    MF_U32  fileNo;         // file No.
    MF_U64  pts;            //time us    
    void   *frameData;      //frame data
    void   *dataBuff;       //struct mf_frame.data
    MF_U32  dataOffset;     //frame data offset
    MF_U32  dataSize;       //frame data size
    INFO_MINOR_EN enMinor;  //minor type
    struct seg_info *pstSegInfo; // owner segment info
    struct file_info *pstFileInfo; // owner file info
    struct ipc_info *pstIpcInfo; // owner ipc info
    struct diskObj  *pstDisk;   // owner disk info
    struct list_head node;//list : struct bkp_node_t
}item_smt_t;

typedef struct bkp_node_t{
    MF_U8   chnId;          //channel id
    MF_U32  count;          //item count
    MF_U64  size;          //item size
    MF_TIME range;          //item streams start / end time (s)
    struct list_head list;//node:struct item_pic_t/item_evt_t/item_comm_t/item_tag_t/item_lpr_t
    struct list_head node;//list : struct bkp_res_t
}bkp_node_t;

typedef struct bkp_res_t{
    MF_U64   size;           //result total size
    MF_U32   count;          //result total count
    MF_TIME range;          //result streams start / end time (s)
    struct list_head list;//node : struct bkp_node_t
}bkp_res_t;

typedef struct item_dev_t{
	MF_U8 port;
    MF_S8 *vendor;          //extern device(USB) vendor 
    MF_S8 *mnt;             // mount path
    void  *disk;            //device detail info
    MF_U64 free;            // free size Mb
    MF_BOOL bformat;         //right format or not
    MF_BOOL bprotect;
	TYPE_EN enType;
    MF_U64 capacity;
    STATE_EN enState;
    MF_BOOL bRec;
    struct list_head node;//list : struct bkp_dev_t
}item_dev_t;

typedef struct bkp_dev_t{
    MF_U32   count;          //extern device total count
    struct list_head list; //node : struct item_dev_t
}bkp_dev_t;

typedef struct bkp_exp_t{
    void *handle;
}bkp_exp_t;
/**
 * \brief initialize retrieve module after msfs_disk_init().
 * \param[in] RETR_JPG_CB
 */
MF_S32
msfs_bkp_init(RETR_JPG_CB jpg_cb);

/**
 * \brief deinitialize retrieve module .
 */
MF_S32
msfs_bkp_deinit();

/**
 * \brief get record range dynamically by ipcMask & fileType at the runtime.
 * \param[in] ipcMaks
 * \param[in] enType
 * \param[out] time
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_record_range(MF_U64 ipcMask, FILE_TYPE_EN enType, MF_TIME *time);

/**
 * \brief common search begin  at the runtime.
 * \param[in] pstComm
 * \return bkp_res_t that a result list
 */
struct bkp_res_t *
msfs_bkp_common_begin(struct bkp_comm_t *pstComm, MF_BOOL progress);

/**
 * \brief common search end after msfs_bkp_common_begin().
 * \param[in] bkp_res_t
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_common_end(struct bkp_res_t *pstRes);

/**
 * \brief common search result dump.
 * \param[in] bkp_res_t
 * \return void
 */
void
msfs_bkp_common_dump(struct bkp_res_t * pstRes);

/**
 * \brief set common item segment state  at the runtime.
 * \param[in] item_comm_t
 * \param[in] bLock
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_common_lock(struct item_comm_t *item, MF_BOOL bLock);

/**
 * \brief event search begin  at the runtime.
 * \param[in] bkp_event_t
 * \return bkp_res_t that a result list.
 */
struct bkp_res_t *
msfs_bkp_event_begin(struct bkp_event_t *pstEvt, MF_BOOL progress);

/**
 * \brief event search end after msfs_bkp_evt_begin().
 * \param[in] bkp_event_t
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_event_end(struct bkp_res_t *pstRes);

/**
 * \brief event search result dump.
 * \param[in] bkp_res_t
 * \return void
 */
void
msfs_bkp_event_dump(struct bkp_res_t *pstRes);

/**
 * \brief set event item segment state  at the runtime.
 * \param[in] item_evt_t
 * \param[in] bLock
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_event_lock(struct item_evt_t *item, MF_BOOL bLock);

/**
 * \brief pic search begin  at the runtime.
 * \param[in] bkp_pic_t
 * \return bkp_res_t that a result list.
 */
struct bkp_res_t *
msfs_bkp_pic_begin(struct bkp_pic_t * pstPic, MF_BOOL progress);

/**
 * \brief pic search end after msfs_bkp_pic_begin().
 * \param[in] bkp_res_t
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_pic_end(struct bkp_res_t * pstRes);

/**
 * \brief pic search result dump
 * \param[in] bkp_res_t
 * \return void.
 */
void
msfs_bkp_pic_dump(struct bkp_res_t * pstRes);

/**
 * \brief lpr search begin  at the runtime.
 * \param[in] bkp_lpr_t
 * \return bkp_res_t that a result list.
 */
struct bkp_res_t *
msfs_bkp_lpr_begin(struct bkp_lpr_t *pstLpr, MF_BOOL progress);

/**
 * \brief lpr search end after msfs_bkp_lpr_begin().
 * \param[in] bkp_res_t
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_lpr_end(struct bkp_res_t * pstRes);

/**
 * \brief lpr search result dump
 * \param[in] bkp_res_t
 * \return void.
 */
void
msfs_bkp_lpr_dump(struct bkp_res_t * pstRes);

/**
 * \brief face search begin  at the runtime.
 * \param[in] bkp_face_t
 * \return bkp_res_t that a result list.
 */
struct bkp_res_t *
msfs_bkp_face_begin(struct bkp_face_t *pstLpr, MF_BOOL progress);

/**
 * \brief face search end after msfs_bkp_face_begin().
 * \param[in] bkp_res_t
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_face_end(struct bkp_res_t * pstRes);

/**
 * \brief face search result dump
 * \param[in] bkp_res_t
 * \return void.
 */
void
msfs_bkp_face_dump(struct bkp_res_t * pstRes);

/**
 * \brief smart search begin  at the runtime.
 * \param[in] bkp_smart_t
 * \return bkp_res_t that a result list.
 */
struct bkp_res_t *
msfs_bkp_smart_begin(struct bkp_smart_t *pstSmart, MF_BOOL progress);

/**
 * \brief smart search end after msfs_bkp_smart_begin().
 * \param[in] bkp_res_t
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_smart_end(struct bkp_res_t *pstRes);

/**
 * \brief smart search result dump
 * \param[in] bkp_res_t
 * \return void.
 */
void
msfs_bkp_smt_dump(struct bkp_res_t * pstRes);

/**
 * \brief tag search begin  at the runtime.
 * \param[in] bkp_tag_t
 * \return bkp_res_t that a result list.
 */
struct bkp_res_t *
msfs_bkp_tag_begin(struct bkp_tag_t *pstTag, MF_BOOL progress);

/**
 * \brief pic search end after msfs_bkp_tag_begin().
 * \param[in] bkp_res_t
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_tag_end(struct bkp_res_t *pstRes);

/**
 * \brief tag search result dump
 * \param[in] bkp_res_t
 * \return void.
 */
void
msfs_bkp_tag_dump(struct bkp_res_t * pstRes);

/**
 * \brief edit tag info
 * \param[in] item_tag_t
 * \param[in] reName : NULL for deletion, other for rename
 * \return void.
 */
MF_S32
msfs_bkp_tag_edit(struct item_tag_t * item, MF_S8 *reName);

/**
 * \brief get first I frame from specific stream.
 * \param[in] stream_seg
 * \param[out] mf_frame 
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_get_Iframe(struct stream_seg *s, struct mf_frame *f);

 /**
 * \brief release the I frame which msfs_bkp_get_Iframe().
 * \param[in] mf_frame
 * \return void
 */
void
msfs_bkp_put_Iframe(struct mf_frame *f);

/**
 * \brief get a frame from specific addr.
 * \param[in] addr
 * \param[in] end
 * \param[out] mf_frame 
 * \return frame size(head + data) for success, MF_FAILURE for failure.
 * \return  
 */
MF_S32
msfs_bkp_get_frame(MF_S8 *addr, MF_S8 *end, struct mf_frame *f, MF_FRAME_DATA_TYPE type);

 /**
 * \brief release the I frame which msfs_bkp_get_Iframe().
 * \param[in] mf_frame
 * \return void
 */
void
msfs_bkp_put_frame(struct mf_frame *f);

/**
 * \brief get a jpg pic from specific stream.
 * \param[in] stream_seg
 * \param[in] INFO_MAJOR_EN
 * \param[out] jpg_frame 
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_get_pic(struct stream_seg * s, INFO_MAJOR_EN m, struct jpg_frame * f);

/**
 * \brief get a jpg pic with w*h resolution from specific stream.
 * \param[in] stream_seg
 * \param[in] width
 * \param[in] height
 * \param[out] jpg_frame 
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_get_wh_pic(struct stream_seg * s, MF_U32 w, MF_U32 h, struct jpg_frame * f);

/**
* \brief release the jpg pic which msfs_bkp_get_pic().
* \param[in] jpg_frame
* \return void
*/
void
msfs_bkp_put_pic(struct jpg_frame * f);

/**
 * \brief get a jpg pic from specific lpr item.
 * \param[in] item_lpr_t
 * \param[in] bMainPic  : lpr has two picture !!, MF_YES for main_pic, MF_NO for sub_pic 
 * \param[out] jpg_frame 
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_lpr_get_pic(struct item_lpr_t * item, MF_BOOL bMainPic, struct jpg_frame * f);

MF_S32
msfs_bkp_lpr_auto_get_pic(struct item_lpr_t * item, MF_BOOL bMainPic, struct jpg_frame * f, int fd);

/**
* \brief release the jpg pic which msfs_bkp_lpr_get_pic().
* \param[in] jpg_frame
* \return void
*/
void
msfs_bkp_lpr_put_pic(struct jpg_frame * f);

/**
 * \brief get a jpg face from specific lpr item.
 * \param[in] item_face_t
 * \param[in] bMainPic  : lpr has two picture !!, MF_YES for main_pic, MF_NO for sub_pic 
 * \param[out] jpg_frame 
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_bkp_face_get_pic(struct item_face_t * item, MF_BOOL bMainPic, struct jpg_frame * f);

/**
* \brief release the jpg pic which msfs_bkp_lpr_get_pic().
* \param[in] jpg_frame
* \return void
*/
void
msfs_bkp_face_put_pic(struct jpg_frame * f);

/**
* \brief open stream file fd.
* \param[in] stream_seg
* \return MF_SUCCESS for success, MF_FAILURE for failure
*/
MF_S32
msfs_bkp_export_open(struct stream_seg *s);

MF_S32
msfs_bkp_anpr_open(struct item_lpr_t *item);

/**
* \brief close stream file fd.
* \param[in] fd
* \return void
*/
void
msfs_bkp_export_close(MF_S32 fd);

/**
* \brief read count byte from offset .
* \param[in] fd
* \param[in] buf
* \param[in] count
* \param[in] offset
* \return MF_S32
*/
MF_S32
msfs_bkp_export_read(MF_S32 fd, void * buf, MF_S32 count, MF_U64 offset);

/**
* \brief start a export task
* \param[in] stream_seg
* \param[in] bFromIframe : MF_YES :first frame is I frame.
* \return bkp_exp_t for success, NULL for failure
*/
struct bkp_exp_t *
msfs_bkp_export_start(struct stream_seg * stream, MF_BOOL bFromIframe);

/**
* \brief stop a export task
* \param[in] bkp_exp_t
* \return MF_SUCCESS for success, MF_FAILURE for failure
*/

MF_S32
msfs_bkp_export_stop(struct bkp_exp_t *pstExp);

/**
* \brief fetch a frame;
* \param[in] bkp_exp_t
* \param[in] mf_frame
* \param[out] pbEOF : end of file ? 
* \return MF_SUCCESS for success, MF_FAILURE for failure
*/
MF_S32
msfs_bkp_export_get_frame(struct bkp_exp_t *pstExp, struct mf_frame *frame, MF_FRAME_DATA_TYPE type, MF_BOOL *pbEOF);

/**
* \brief release a frame;
* \param[in] bkp_exp_t
* \param[in] mf_frame
* \return MF_SUCCESS for success, MF_FAILURE for failure
*/
MF_S32
msfs_bkp_export_put_frame(struct bkp_exp_t *pstExp, struct mf_frame *frame);

/**
* \brief format extern backup device (eg: USB).
* \param[in] item_dev_t
* \param[in] enFormat
* \return MF_SUCCESS for success, MF_FAILURE for failure
*/
MF_S32
msfs_bkp_dev_format(struct item_dev_t *item, FORMAT_EN enFormat);
/**
* \brief get extern backup device list (eg: USB).
* \param[out] bkp_dev_t
* \return MF_SUCCESS for success, MF_FAILURE for failure
*/
MF_S32
msfs_bkp_get_dev_list(struct bkp_dev_t *pstDev);

/**
* \brief release extern backup device list (eg: USB).
* \param[in] bkp_dev_t
* \return void
*/
void
msfs_bkp_put_dev_list(struct bkp_dev_t *pstDev);

/**
* \brief check whether the stream is valid when file has removed.
* \param[in] stream_seg
* \return MF_YES or MF_NO
*/
MF_BOOL
msfs_bkp_stream_is_valid(struct stream_seg * stream);

/**
* \brief show retrieve memory pool usage.
* \return none
*/
void
msfs_bkp_dbg_mem_usage();

/**
* \brief  retrieve debug switch.
* \param[in] bDebug MF_YES for enable, MF_NO for disable
* \return none
*/
void
msfs_bkp_dbg_switch(MF_BOOL bDebug);

MF_S32
msfs_bkp_export_get_custom(MF_S8 *data, MF_S32 len, struct mf_frame *frame);

MF_S32
msfs_bkp_export_put_custom(struct mf_frame *frame);


#ifdef __cplusplus
}
#endif

#endif


