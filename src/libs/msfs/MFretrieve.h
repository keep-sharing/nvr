/*
 * ***************************************************************
 * Filename:        MFretrieve.h
 * Created at:      2017.08.04
 * Description:     retrieve api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFRETRIEVE_H__
#define __MFRETRIEVE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "msfs_bkp.h"

#define SORT_IS_UP      MF_NO
#define MAX_INFO_SIZE   (INFO_SEG_MAX_SIZE + 1024)
#define MAX_DATA_SIZE   (5 * 1024 * 1024)

typedef enum {
    FILE_UNLOAD = 0,
    FILE_PRE_LOAD,
    FILE_SEG_LOAD,
    FILE_SEG_UNLOAD,
} FILE_LOAD_EN;

typedef struct {
    MF_U32 streamType;      //码流类型 bit @FILE_TYPE_EN

    /*主码流*/
    REC_EVENT_EN recType;   
    STID_MAJOR_EN majorType;
    INFO_MINOR_EN minorType;
    
    /*次码流*/
    REC_EVENT_EN recSubType;   
    STID_MAJOR_EN majorSubType;
    INFO_MINOR_EN minorSubType;
} MF_RETR_INFO_S;

typedef struct seg_info {
    MF_S16  id;
    MF_U16  encodeType;
    MF_U16  resWidth;
    MF_U16  resHeight;
    MF_U16  infoCount;
    REC_EVENT_EN enEvent;
    SEG_STATE_EN enState;
    INFO_MAJOR_EN enMajor;
    INFO_MINOR_EN enMinor;
    MF_PTS  segStartTime;
    MF_PTS  segEndTime;
    MF_U32  infoStartOffset;
    MF_U32  infoEndOffset;
    MF_U32  recStartOffset;
    MF_U32  recEndOffset;
    MF_U32  firstIframeOffset;
    MF_U32  lastIframeOffset;
    MF_U64  firstIframeTime;
    MF_U64  lastIframeTime;
    MF_U64  firstFrameTime;
    MF_U64  lastFrameTime;
    struct file_info   *owner;
    struct list_head   node;
} seg_info;

typedef struct file_info {
//    MF_U8   type;
    MF_BOOL bRecording;
    MF_U32  loadTime;
    FILE_LOAD_EN enLoad;
    REC_EVENT_EN enEvent;
    SEG_STATE_EN enState;
    INFO_MAJOR_EN enMajor;
    INFO_MINOR_EN enMinor;
//    MF_U32  fileNO;
//    off64_t fileOffset;
    MF_PTS  fileStartTime;
    MF_PTS  fileEndTime;
    MF_U32  recStartOffset;
    MF_U32  recEndOffset;
    MF_S32  segNum;
    MF_S32  tagNum;
    MF_S32  lockNum;
    MF_S32  refNum;
    struct seg_info   *pstSegUsing;
    struct bplus_key   key;
    struct ipc_info   *owner;
    struct list_head   segHead;
    struct list_head   node;
} file_info;

typedef struct ipc_info {
    MF_U8   chnId;
    MF_S32  fileNum;
    MF_PTS  ipcStartTime;
    MF_PTS  ipcEndTime;
    MF_PTS  mainStartTime;
    MF_PTS  mainEndTime;
    MF_PTS  subStartTime;
    MF_PTS  subEndTime;
    MF_PTS  picStartTime;
    MF_PTS  picEndTime;
    MUTEX_OBJECT       mutex;
    struct file_info  *pstFirstUsed;
    struct diskObj    *owner;
    struct list_head   fileHead;
    struct list_head   node;
    
    MF_U32 streamType;          //码流类型 bit @FILE_TYPE_EN
    REC_EVENT_EN recType;       /*主码流*/ 
    STID_MAJOR_EN majorType;
    INFO_MINOR_EN minorType;
    REC_EVENT_EN recSubType;    /*次码流*/  
    STID_MAJOR_EN majorSubType;
    INFO_MINOR_EN minorSubType;
} ipc_info;

typedef struct exp_info {
    MF_BOOL bTail;
    MF_S32  fd;
    MF_S32 infoIndex;
    MF_U32 infoSize;
    MF_U32 infoCount;
    MF_U32 dataOffset;
    MF_U32 dataSize;
    MF_U32 dataHead;
    MF_U32 dataTail;
    MF_S8 *dataBuff;
    MF_S8 *infoBuff;
    MF_U64 lastIframeTime;
    MF_U32 fileOffset;
    MF_U32 fileSize;
    MF_U32 alignOffset;
    off64_t fileBase;
    struct stream_seg *stream;
} exp_info;

typedef enum ERR_RETR_CODE_E {
    ERR_RETR_NOT_INIT           = 1,
    ERR_RETR_INVALID_PARAM      = 2,
    ERR_RETR_BAD_FRAME          = 3,
    ERR_RETR_DATA_OVERFLOW      = 4,
    ERR_RETR_DATA_EOF           = 5,
    ERR_RETR_BUTT,
} ERR_RETR_CODE_E;

#define MF_ERR_RETR_INVALID_PARAM       MF_DEF_ERR(MF_MOD_ID_RETR, MF_ERR_LEVEL_ERROR, ERR_RETR_INVALID_PARAM)
#define MF_ERR_RETR_BAD_FRAME           MF_DEF_ERR(MF_MOD_ID_RETR, MF_ERR_LEVEL_ERROR, ERR_RETR_BAD_FRAME)
#define MF_ERR_RETR_DATA_OVERFLOW       MF_DEF_ERR(MF_MOD_ID_RETR, MF_ERR_LEVEL_ERROR, ERR_RETR_DATA_OVERFLOW)
#define MF_ERR_RETR_DATA_EOF            MF_DEF_ERR(MF_MOD_ID_RETR, MF_ERR_LEVEL_ERROR, ERR_RETR_DATA_EOF)


MF_S32
mf_retr_disk_info_add(struct diskObj *pstDisk);
void
mf_retr_disk_info_del(struct diskObj *pstDisk);
void
mf_retr_disk_info_dump(struct diskObj *pstDisk, MF_U8 chnid);
struct file_info *
mf_retr_disk_find_used_file(struct diskObj *pstDisk);
struct file_info *
mf_retr_disk_find_using_file(struct diskObj *pstDisk, MF_U8 chnId, MF_U8 type, FILE_STATE_EN enState);
struct file_info *
mf_retr_disk_find_empty_file(struct diskObj *pstDisk, MF_U8 chnId, FILE_TYPE_EN enType);
MF_U64
mf_retr_disk_free_size(struct diskObj *pstDisk);
MF_S32
mf_retr_fetch_disk_details();
MF_S32
mf_retr_release_disk_details();
MF_S32
mf_retr_ipc_owner_update(struct ipc_info *pstIpcInfo);
void
mf_retr_ipc_record_range(MF_U8 chnId, FILE_TYPE_EN enType, MF_TIME *time);
MF_S32
mf_retr_ipc_comm_begin(MF_U8 chnId, FILE_TYPE_EN enType, REC_EVENT_EN enEvent, SEG_STATE_EN enState, MF_TIME *pstTime,
                       struct bkp_node_t *pstNode, MF_U32 total);
void
mf_retr_ipc_comm_end(struct bkp_node_t *pstNode);
MF_S32
mf_retr_ipc_event_begin(MF_U8 chnId, struct bkp_event_t *pstEvt, struct bkp_node_t *pstNode, MF_U32 total, MF_S32 baseP,
                        MF_U32 deltaP, MF_BOOL progress);
void
mf_retr_ipc_event_end(struct bkp_node_t *pstNode);
MF_S32
mf_retr_ipc_pic_begin(MF_U8 chnId, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor, MF_TIME *pstTime,
                      struct bkp_node_t *pstNode, MF_U32 total, struct bkp_pri_t pri, struct bkp_vca_t *vcaPrivate, MF_U32 maxNum);
void
mf_retr_ipc_pic_end(struct bkp_node_t *pstNode);
MF_S32
mf_retr_ipc_lpr_begin(MF_U8 chnId, MF_TIME *pstTime, MF_S8 *keyWord, EVT_MATCH match, struct bkp_pri_t *pri,
                      struct bkp_node_t *pstNode, MF_U32 total);
MF_S32
mf_retr_ipc_face_begin(MF_U8 chnId, MF_TIME *pstTime, EVT_MATCH match, struct bkp_pri_t *pri,
                       struct bkp_node_t *pstNode, MF_U32 total);
MF_S32
mf_retr_ipc_smart_begin(MF_U8 chnId, struct bkp_smart_t *pstSmart, struct bkp_node_t *pstNode, MF_U32 total,
                        void *infoBuff, void *dataBuff);
void
mf_retr_ipc_lpr_end(struct bkp_node_t *pstNode);
void
mf_retr_ipc_smart_end(struct bkp_node_t *pstNode);
void
mf_retr_ipc_face_end(struct bkp_node_t *pstNode);
MF_S32
mf_retr_ipc_tag_begin(MF_U8 chnId, FILE_TYPE_EN enType, MF_TIME *pstTime, MF_S8 *keyWord, MF_S8 *out,
                      struct bkp_node_t  *pstNode);
void
mf_retr_ipc_tag_end(struct bkp_node_t *pstNode);
MF_S32
mf_retr_seg_load(struct file_info *pstFileInfo);
MF_S32
mf_retr_unload(MF_U32 timer);
MF_S32
mf_retr_file_owner_update(struct file_info *pstFileInfo);
void
mf_retr_file_info_update(struct file_info *pstFileInfo);
void
mf_retr_file_info_delete(struct file_info *pstFileInfo);
struct file_info *
mf_retr_file_info_insert(struct file_info *pstFileInfo);
struct file_info *
mf_retr_file_info_reset(struct file_info *pstFileInfo);
MF_S32
mf_retr_file_delete(struct file_info *pstFileInfo, FILE_CONDITION_EN enCond);
MF_BOOL
mf_retr_file_deadline_check(MF_U8 chnId, FILE_TYPE_EN enType, MF_U32 deadTime);
MF_S32
mf_retr_file_info_from_disk(struct diskObj *pstDisk, struct mf_file *pstFile);
MF_S32
mf_retr_seg_owner_update(struct seg_info *pstSegInfo);
struct seg_info *
mf_retr_seg_info_new(struct file_info *pstFileInfo);
MF_S32
mf_retr_seg_info_bsearch(MF_S8 *buff, MF_U32 len, MF_U64 timeUs, MF_U16 type, MF_BOOL isUp);
MF_U32
mf_retr_seg_info_qsort(void *buff, MF_U32 len, MF_BOOL isUp);
MF_S32
mf_retr_get_search_seg_info(stream_seg *s, seg_info *seg, MF_TIME *t);
MF_S32
mf_retr_get_search_tag_info(file_info *file, MF_TIME *t, MF_S8 *keyWord, MF_S8 *out, struct bkp_node_t *pstNode);
MF_S32
mf_retr_get_search_lpr_info(MF_S8 *keyWord, EVT_MATCH match, seg_info *seg, MF_TIME *t, struct bkp_pri_t *pri,
                            struct bkp_node_t *pstNode, MF_U32 total);
MF_S32
mf_retr_get_search_face_info(EVT_MATCH match, seg_info *seg, MF_TIME *t, struct bkp_pri_t *pri,
                             struct bkp_node_t *pstNode, MF_U32 total);
MF_S32
mf_retr_get_search_pic_info(seg_info *seg, MF_TIME *t, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor,
                            struct bkp_node_t  *pstNode, MF_U32 total, struct bkp_pri_t pri, struct bkp_vca_t *vcaPrivate, MF_U32 maxNum);
struct item_tag_t *
mf_retr_tag_alloc(struct mf_tag *tag, struct stream_seg *stream, MF_S8 *name, MF_PTS time);
void
mf_retr_tag_free(struct item_tag_t *item);
MF_S32
mf_retr_tag_edit(struct item_tag_t *item, MF_S8 *rename);
MF_S32
mf_retr_seg_lock(struct stream_seg *s, MF_BOOL bLock);
MF_S32
mf_retr_get_first_Iframe(struct stream_seg *s, struct mf_frame *Iframe);
MF_S32
mf_retr_get_Iframe_by_time(struct stream_seg *s, struct mf_frame *Iframe, MF_PTS t);
void
mf_retr_put_Iframe(struct mf_frame *Iframe);
MF_S32
mf_retr_get_a_frame(MF_S8 *addr, MF_S8 *end, struct mf_frame *frame, MF_FRAME_DATA_TYPE type);
void
mf_retr_put_a_frame(struct mf_frame *frame);
MF_S32
mf_retr_last_Iframe_index(MF_S8 *infoBuff, MF_U32 infoSize);
struct mf_info *
mf_retr_get_iframe_info(MF_S8 *buff, MF_S32 index);
MF_S32
mf_retr_get_a_pic(struct stream_seg *s, INFO_MAJOR_EN m, struct jpg_frame *frame);
MF_S32
mf_retr_get_a_wh_pic(struct stream_seg *s, MF_U32 w, MF_U32 h, struct jpg_frame *frame);
void
mf_retr_put_a_pic(struct jpg_frame *frame);
MF_S32
mf_retr_get_lpr_pic(struct item_lpr_t *item, MF_BOOL bMainPic, struct jpg_frame *f);
MF_S32
mf_retr_get_lpr_auto_pic(struct item_lpr_t *item, MF_BOOL bMainPic, struct jpg_frame *f, MF_S32 fd);
void
mf_retr_put_lpr_pic(struct jpg_frame *f);
MF_S32
mf_retr_get_face_pic(struct item_face_t *item, MF_BOOL bMainPic, struct jpg_frame *f);
void
mf_retr_put_face_pic(struct jpg_frame *f);
struct exp_info *
mf_retr_export_start(struct stream_seg *s, MF_BOOL bFromIframe);
MF_S32
mf_retr_export_stop(struct exp_info *exp);
MF_S32
mf_retr_export_get_frame(struct exp_info *exp, struct mf_frame *frame, MF_FRAME_DATA_TYPE type);
MF_S32
mf_retr_export_put_frame(struct exp_info *exp, struct mf_frame *frame);
void
mf_retr_notify_bar_state(PROGRESS_BAR_E enBar,  MF_S32 percent, MF_U8 from, MF_U8 id);
MF_BOOL
mf_retr_stream_is_valid(struct stream_seg *stream);
void
mf_retr_jpg_encoder(RETR_JPG_CB jpg_cb);
void
mf_retr_dbg_show_mem_usage();
void
mf_retr_dbg_switch(MF_BOOL bDebug);
MF_BOOL
mf_retr_is_dbg();
MF_S32
mf_retr_init();
MF_S32
mf_retr_deinit();

MF_S32
mf_retr_export_get_custom(MF_S8 *data, MF_S32 len, struct mf_frame *frame);

MF_S32
mf_retr_export_put_custom(struct mf_frame *frame);

struct file_info *
mf_retr_disk_find_used_file_qta(struct diskObj *disk, MF_U8 chnId, FILE_TYPE_EN enType);
MF_S32
mf_retr_file_qta_check(MF_U8 chnId, FILE_TYPE_EN enType, MF_BOOL bCareLoop, struct list_head *list);
MF_S32
mf_retr_update_index_lock(diskObj *disk, struct file_info *file);
MF_S32
mf_retr_file_head_preload(struct file_info *pstFileInfo);



#ifdef __cplusplus
}
#endif

#endif


