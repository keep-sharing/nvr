/* 
 * ***************************************************************
 * Filename:      	msfs_pb.h
 * Created at:    	2017.10.10
 * Description:   	msfs playback api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __MSFS_PB_H__
#define __MSFS_PB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"

#define MONTH_MAX_DAY (32)

typedef enum{
    PB_TYPE_LOCAL  = 0,
    PB_TYPE_REMOTE,
    PB_TYPE_ANR,
    PB_TYYE_CNT,
}PB_TYPE_EN;

typedef enum {
    PB_PAUSE = 0,
    PB_PLAY,
    PB_STOP,
    PB_STEP,
    PB_FORWARD,
    PB_BACKWARD,
}PB_ACTION_EN;

typedef struct pb_hdl_t{
    VID_EN enVid;       //mian/sub/anr
    PB_TYPE_EN enType;  // local/remote/other
    MF_S32  sid;        // session id of rstp
    void *player;       //video handle
    void *picture;      //picture handle
    void *usrData;      //user data
}pb_hdl_t;

typedef enum{
    PB_ERR_NONE = 0,
    PB_ERR_UNKNOWN = -1,
    PB_ERR_TAG_OVERFLOW = -2,
    PB_ERR_STREAM_INVALID = -3,
}PB_ERR_EN;

typedef int  (*PB_OUT_CB)(struct pb_hdl_t *, struct mf_frame *);

typedef struct pb_init_t{
    PB_OUT_CB   pbOutFunc;
}pb_init_t;

typedef struct playList{
    MF_U8   chnId;          // channel id
//    MF_U32  count;          // stream count
    MF_PTS  startTime;      // list start time(s)
    MF_PTS  endTime;        //list end time(s)
    struct list_head *head; //node :struct stream_seg   
}playList;

typedef struct pb_month_t{
    MF_BOOL bHave;  // 0: invalid 1:valid
    REC_EVENT_EN enEvent[MONTH_MAX_DAY]; //events array
}pb_month_t;

typedef struct pb_stat_t{
    MF_U64  curTime;    //current data time
    MF_BOOL bEOF;       //flag end of file
    MF_BOOL bStop;       //no data now
    MF_BOOL bPaused;       //paused state
    struct stream_seg *pstNextStream; //next stream info, Null is invaild
}pb_stat_t;

/**
 * \brief initialize playback module after msfs_disk_init().
 * \param[in] func that user callback api
 */
MF_S32
msfs_pb_init(PB_OUT_CB func);

/**
 * \brief deinitialize playback module.
 */
MF_S32
msfs_pb_deinit();

/**
* \brief create a playback handle when channel selected.
* \param[in] chnid
* \param[in] enVid
* \param[in] sid
* \param[in] enType
* \param[in] plist : play list (node :struct stream_seg)
* \param[in] usrData : user data
* \return pb_hdl_t.
*/
struct pb_hdl_t *
msfs_pb_player_create(MF_U8 chnId, VID_EN enVid, PB_TYPE_EN enType, MF_S32 sid, struct list_head *plist, void *usrData);

/**
* \brief destroy specific playback handle.
* \param[in] pb_hdl_t
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_destory(struct pb_hdl_t *pbhdl);

/**
* \brief start playback fram specific  UTC time(s).
* \param[in] pb_hdl_t
* \param[in] time
* \param[in] pTimer : user reattimer  , NULL for dont care 
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_start(struct pb_hdl_t *pbhdl, MF_PTS time, MF_U64 *pTimer);

/**
* \brief stop playback.
* \param[in] pb_hdl_t
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_stop(struct pb_hdl_t *pbhdl);

/**
* \brief get playback state.
* \param[in] pb_hdl_t
* \param[in] pb_stat_t
* \param[in] MF_BOOL
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_stat(struct pb_hdl_t *pbhdl, struct pb_stat_t *pstStat, MF_BOOL bBlock);

/**
* \brief add a stream to  playList.
* \param[in] pb_hdl_t
* \param[in] stream_seg
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_stream_add(struct pb_hdl_t *pbhdl, struct stream_seg *stream);

/**
* \brief remove the stream from  playList.
* \param[in] pb_hdl_t
* \param[in] stream_seg
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_stream_del(struct pb_hdl_t *pbhdl, struct stream_seg *stream);

/**
* \brief set playback mode.
* \param[in] pb_hdl_t
* \param[in] bAuto : MF_YES-auto play next stream when last stream done, default MF_NO;
* \return none.
*/
void
msfs_pb_player_mode(struct pb_hdl_t *pbhdl, MF_BOOL bAuto);

/**
* \brief snap a playback picture with timestamp(s).
* \param[in] pb_hdl_t
* \param[in] time
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_snap(struct pb_hdl_t *pbhdl, MF_PTS time);

/**
* \brief snap a playback picture with format.
* \param[in] pb_hdl_t
* \param[in] pstFrame
* \param[in] enFormat
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_snap_ex(struct pb_hdl_t * pbhdl, struct mf_frame * pstFrame, PIC_FORMAT_EN enFormat);

/**
* \brief seek the specific time to jump.
* \param[in] pb_hdl_t
* \param[in] time
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_seek(struct pb_hdl_t *pbhdl, MF_PTS time);

/**
* \brief jump to play next(or  previous) stream segment whit .
* \param[in] pb_hdl_t
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_player_next_stream(struct pb_hdl_t *pbhdl);

/**
* \brief playback controller.
* \param[in] pb_hdl_t
* \param[in] enAction
* \return void.
*/
void
msfs_pb_player_action(struct pb_hdl_t *pbhdl, PB_ACTION_EN enAction);

/**
* \brief playback speed controller.
* \param[in] pb_hdl_t
* \param[in] speed (0.125~128.0)
* \param[in] bFullSpeed  MF_YES for full speed , MF_NO for I frame speed
* \param[in] bSoft  MF_YES for calibrating , MF_NO for normal
* \return void.
*/
void
msfs_pb_player_speed(struct pb_hdl_t *pbhdl, MF_FLOAT speed, MF_BOOL bFullSpeed, MF_BOOL bSoft);

/**
* \brief get a month events dynamically  by channel mask & file type  .
* \param[in] ipcMask
* \param[in] enType
* \param[in] year
* \param[in] month
* \param[out] pstMonth
* \return MF_SUCCESS for success, MF_FAILURE for failure.
*/
MF_S32
msfs_pb_get_month_event(MF_U64 ipcMask, FILE_TYPE_EN enType, MF_U32 year, MF_U32 month, struct pb_month_t *pstMonth);

/**
 * \brief get record range dynamically by ipcMask & fileType at the runtime.
 * \param[in] ipcMaks
 * \param[in] enType
 * \param[out] time
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
MF_S32
msfs_pb_record_range(MF_U64 ipcMask, FILE_TYPE_EN enType, MF_TIME *time);

/**
 * \brief alloc a tag item at the playack.
 * \param[in] pb_hdl_t
 * \param[in] name
 * \param[in] time (UTC)
 * \param[in] err : errNo.
 * \return item_tag_t for success, NULL for failure.
 */
struct item_tag_t *
msfs_pb_alloc_tag_item(struct pb_hdl_t *pbhdl, MF_S8 * name,MF_PTS time, PB_ERR_EN *penErr);

/**
 * \brief free specific tag after  msfs_pb_alloc_tag_item();
 * \param[in] item_tag_t
 * \return none.
 */
void
msfs_pb_free_tag_item(struct item_tag_t *item);

/**
 * \brief lock the stream segment within time
 * \param[in] pbhdl
 * \param[in] time(UTC)
 * \return  NULL or  the stream that locked.
 */
struct stream_seg *
msfs_pb_segment_lock(struct pb_hdl_t *pbhdl, MF_PTS time);

/**
 * \brief get a playback thumb with time
 * \param[in] pbhdl
 * \param[in] time(UTC)
 * \param[out] pThumb
 * \return MF_SUCCESS for success, MF_FAILURE for failure.
 */
int
msfs_pb_get_thumb(struct pb_hdl_t *pbhdl, MF_PTS time, struct mf_frame *pThumb);

/**
 * \brief free specific playback thumb after  msfs_pb_get_thumb();
 * \param[in] pbhdl
 * \param[int] pThumb
 * \return none.
 */
void 
msfs_pb_put_thumb(struct pb_hdl_t *pbhdl, struct mf_frame *pThumb);

/**
* \brief  playback debug switch.
* \param[in] bDebug MF_YES for enable, MF_NO for disable
* \return none
*/
void
msfs_pb_dbg_switch(MF_BOOL bDebug);

/**
* \brief show playback memory pool usage.
* \return none
*/
void
msfs_pb_dbg_mem_usage();

/**
* \brief show playback all chn info.
* \return none
*/
void
msfs_pb_dbg_show_chn();

#ifdef __cplusplus
}
#endif

#endif


