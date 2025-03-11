/* 
 * ***************************************************************
 * Filename:      	msfs_playback.c
 * Created at:    	2017.10.10
 * Description:   	msfs playback API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include "mf_type.h"
#include "MFplayback.h"
#include "MFtask.h"

struct pb_hdl_t *
msfs_pb_player_create(MF_U8 chnId, VID_EN enVid, PB_TYPE_EN enType, MF_S32 sid, struct list_head *plist, void *usrData)
{
    if (!plist)
    {
        MFerr("plist is NULL");
        return NULL;
    }
    
    if (enVid >= VID_TYPE_NUM)
    {
        MFerr("enVid[%d] overflow[%d]", enVid, VID_TYPE_NUM);
        return NULL;
    }
    
    MFinfo("chnid[%d] enVid[%d] enType[%d] sid[%d]", chnId, enVid, enType, sid);
    struct pb_hdl_t *pbhdl;
    struct player_node *player;
    struct recHDL *picture;
    struct playList stPlayList;
    struct stream_seg *s;
    
    stPlayList.chnId = chnId;
    stPlayList.head = plist;
    s = list_entry(plist->next,struct stream_seg, node);
    stPlayList.startTime = s->startTime;
    s = list_entry(plist->prev,struct stream_seg, node);
    stPlayList.endTime = s->endTime;
    
    player = mf_pb_player_create(&stPlayList);
    
    picture = mf_record_pic_create(chnId, HDL_PB);
    
    
    pbhdl = ms_malloc(sizeof(struct pb_hdl_t));
    player->owner = pbhdl;
    picture->owner = pbhdl;
    pbhdl->player = player;
    pbhdl->picture = picture;
    pbhdl->enVid   = enVid;
    pbhdl->enType  = enType;
    pbhdl->sid     = sid;
    pbhdl->usrData = usrData;
    
    return pbhdl;
}

MF_S32
msfs_pb_player_destory(struct pb_hdl_t *pbhdl)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_pb_player_destory");
    mf_record_pic_destory(pbhdl->picture);
    mf_pb_player_destroy(pbhdl->player);
    
    ms_free(pbhdl);
    
    return MF_SUCCESS;
}

MF_S32
msfs_pb_player_start(struct pb_hdl_t *pbhdl, MF_PTS time, MF_U64 *pTimer)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("time[%s]\n", mf_time_to_string(time));
    
    struct pbCmd *pCmd = mf_pb_cmd_requst();
    MF_S32 res = MF_SUCCESS;
    
    mf_record_pic_start(pbhdl->picture);
//    res = mf_pb_player_start(pbhdl->player, time, pTimer);
    pCmd->enCmd = PB_CMD_START;
    pCmd->time  = time;
    pCmd->pTimer = pTimer;
    mf_pb_cmd_push(pbhdl->player, pCmd);

    return res;
}

MF_S32
msfs_pb_player_stop(struct pb_hdl_t *pbhdl)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_pb_player_stop");
//    struct pbCmd *pCmd = mf_pb_cmd_requst();
    
    mf_record_pic_stop(pbhdl->picture);
    mf_pb_player_stop(pbhdl->player);
//    pCmd->enCmd = PB_CMD_STOP;
//    mf_pb_cmd_push(pbhdl->player, pCmd);

    return MF_SUCCESS;
}

MF_S32
msfs_pb_player_stream_add(struct pb_hdl_t *pbhdl, struct stream_seg *stream)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }

    if (!stream)
    {
        MFerr("stream is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_pb_player_stream_add");

    return mf_pb_player_stream_add(pbhdl->player, stream);
}

MF_S32
msfs_pb_player_stream_del(struct pb_hdl_t *pbhdl, struct stream_seg *stream)
{    
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }

    if (!stream)
    {
        MFerr("stream is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_pb_player_stream_del");
    
    return mf_pb_player_stream_del(pbhdl->player, stream);
}

void
msfs_pb_player_mode(struct pb_hdl_t *pbhdl, MF_BOOL bAuto)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return;
    }
    MFinfo("bAuto[%d]", bAuto);

    mf_pb_player_mode(pbhdl->player, bAuto);
}

MF_S32
msfs_pb_player_snap(struct pb_hdl_t *pbhdl, MF_PTS time)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("time[%s]", mf_time_to_string(time));

    struct stream_seg * s;
    struct mf_frame stIframe;
    MF_S32 res;
    
    s = mf_pb_get_stream(pbhdl->player, time); 

    if (!s)
        return MF_FAILURE;
    
    res = mf_record_pic_snap(pbhdl->picture, pbhdl->enVid, 
                            INFO_MAJOR_PB_PIC,
                            INFO_MINOR_NONE,
                            time, NULL, 0);
    if (res != MF_SUCCESS)
        return res;
        
    res = mf_retr_get_Iframe_by_time(s, &stIframe, time);
    if (res == MF_SUCCESS) {
#if defined(_NT98323_) || defined (_HI3536C_) || defined (_HI3798_)
    if (stIframe.height > 4000 || stIframe.width > 4000) {
        res = MF_FAILURE;
    }
#elif defined (_HI3536G_)
    if ((stIframe.height > 4000 || stIframe.width > 4000) && stIframe.codec_type == CODECTYPE_H265) {
        res = MF_FAILURE;
    }
#endif
    }
    if (res == MF_SUCCESS)
    {
        stIframe.stream_from = SST_LOCAL_PB;
        stIframe.stream_format = STREAM_TYPE_MAINSTREAM;
        mf_record_pic_receive(&stIframe);
    }
    mf_retr_put_Iframe(&stIframe);
    
    return res;
}

MF_S32
msfs_pb_player_snap_ex(struct pb_hdl_t *pbhdl, struct mf_frame *pstFrame, PIC_FORMAT_EN enFormat)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    
//    struct pb_stat_t stStat;
//    
//    mf_pb_player_stat(pbhdl->player, &stStat, MF_YES);
//    if (stStat.bStop)
//    {
//        MFinfo("player state stop");
//        return MF_FAILURE;
//    }
    
    pstFrame->frame_type    = FT_IFRAME;
    pstFrame->strm_type     = ST_VIDEO;
    pstFrame->stream_from   = SST_LOCAL_PB;
    pstFrame->codec_type    = enFormat;
    pstFrame->stream_format = STREAM_TYPE_MAINSTREAM;
//    pstFrame->time_usec     = stStat.curTime;
    
    MFinfo("player snap format[%d]", enFormat);
    return mf_record_pic_frame(pbhdl->picture, INFO_MAJOR_PB_PIC, INFO_MINOR_NONE, pstFrame, NULL, 0);
}

MF_S32
msfs_pb_player_seek(struct pb_hdl_t *pbhdl, MF_PTS time)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("time[%s]", mf_time_to_string(time));

    struct pbCmd *pCmd = mf_pb_cmd_requst();
    
    if (pCmd)
    {
        pCmd->enCmd = PB_CMD_SEEK;
        pCmd->time  = time;
        mf_pb_cmd_push(pbhdl->player, pCmd);
        return MF_SUCCESS;
    }
    
    return MF_FAILURE;
}

MF_S32
msfs_pb_player_next_stream(struct pb_hdl_t *pbhdl)
{    
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_pb_player_next_stream");
    
    return mf_pb_player_next_stream(pbhdl->player);
}

void
msfs_pb_player_action(struct pb_hdl_t *pbhdl, PB_ACTION_EN enAction)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return;
    }
    MFinfo("enAction[%d]", enAction);
    
    struct pbCmd *pCmd = mf_pb_cmd_requst();

    if (pCmd)
    {
        switch (enAction)
        {
            case PB_PAUSE:
                pCmd->enCmd = PB_CMD_PAUSE;
                break;
            case PB_PLAY:
                pCmd->enCmd = PB_CMD_PLAY;
                break;
            case PB_STOP:
                pCmd->enCmd = PB_CMD_STOP;
                break;
            case PB_BACKWARD:
                pCmd->enCmd = PB_CMD_DIR;
                pCmd->enDir = PB_DIR_BACKWARD;
                break;
            case PB_FORWARD:
                pCmd->enCmd = PB_CMD_DIR;
                pCmd->enDir = PB_DIR_FORWARD;
                break;
            case PB_STEP:
                pCmd->enCmd = PB_CMD_STEP;
                break;
            default:
                pCmd->enCmd = PB_CMD_NONE;
                break;
        }
        mf_pb_cmd_push(pbhdl->player, pCmd);
    }
}

void
msfs_pb_player_speed(struct pb_hdl_t *pbhdl, MF_FLOAT speed, 
                                MF_BOOL bFullSpeed, MF_BOOL bSoft)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return;
    }
    MFinfo("speed[%f] bFullSpeed[%d] bSoft[%d]", speed, bFullSpeed, bSoft);
    
    struct pbCmd *pCmd = mf_pb_cmd_requst();
    
    if (pCmd)
    {
        pCmd->enCmd = PB_CMD_SPEED;
        pCmd->speed = speed;
        pCmd->bSoft = bSoft;
        pCmd->bFullSpeed = bFullSpeed;
        mf_pb_cmd_push(pbhdl->player, pCmd);
    }
}

MF_S32
msfs_pb_player_stat(struct pb_hdl_t *pbhdl, struct pb_stat_t * pstStat, MF_BOOL bBlock)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }

    return mf_pb_player_stat(pbhdl->player, pstStat, bBlock);
}

MF_S32
msfs_pb_get_month_event(MF_U64 ipcMask, FILE_TYPE_EN enType,
                            MF_U32 year, MF_U32 month,
                            struct pb_month_t *pstMonth)
{
    struct task_calendar_t *pstCal;
    struct calendor_node_t *calNode;
    MF_S32 i;
	MF_U64 n = 1;
	MF_S32 index = 0;
    MF_PTS startTime, endTime;
    MF_U32 from, to;

    MFinfo("ipcMask[0x%llx] enType[%d] [%d-%d]", ipcMask, enType, year, month);
    startTime = mf_date_to_second(year, month);
    endTime = mf_date_to_second(year, month + 1) - 1;
    
    pstCal = mf_task_calendar_get();

    memset(pstMonth, 0, sizeof(struct pb_month_t));

    for (i = 0; i < 64; i++)
    {
        if (ipcMask & ( n << i))
        {
            list_for_each_entry(calNode, &pstCal->astlist[i][enType], node)
            {

                if (calNode->endTime < startTime)
                    continue; 
                if (calNode->startTime > endTime)
                    break;
                struct seg_node_t *seg;
                list_for_each_entry(seg, &calNode->segList, node)
                {
                    if (seg->endTime < startTime 
                    ||  seg->startTime > endTime)
                        continue;
                    from = seg->startTime;
                    to = seg->endTime;
                    
                    if (seg->startTime < startTime)
                        from = startTime;

                    if (seg->endTime > endTime)
                        to = endTime;
                        
                    index = (from - startTime) / DAY_SECONDS;
                    pstMonth->enEvent[index] |= seg->enEvent;
                    index = (to - startTime) / DAY_SECONDS;
                    pstMonth->enEvent[index] |= seg->enEvent;
                    pstMonth->bHave = MF_YES;
                }
            }
        }
    }
#if 0
    MFinfo("pstMonth->bHave[%d]", pstMonth->bHave);
    for (i = 0; i< MONTH_MAX_DAY; i++)
    {
        MFinfo("[%d]Event[%#x]", i+1, pstMonth->enEvent[i]);
    }
#endif
    mf_task_calendar_put(pstCal);
//    mf_task_calendar_dump();

    return MF_SUCCESS;
}

MF_S32
msfs_pb_record_range(MF_U64 ipcMask, FILE_TYPE_EN enType, MF_TIME *time)
{
    MFinfo("ipcMask[%#llx] enType[%d]", ipcMask, enType);
    return msfs_bkp_record_range(ipcMask, enType, time);
}

struct stream_seg *
msfs_pb_segment_lock(struct pb_hdl_t *pbhdl, MF_PTS time)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return NULL;
    }
    MFinfo("time[%s]", mf_time_to_string(time));
    
    return mf_pb_stream_lock(pbhdl->player, time);
}

struct item_tag_t *
msfs_pb_alloc_tag_item(struct pb_hdl_t *pbhdl, MF_S8 * name,MF_PTS time, PB_ERR_EN *penErr)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        *penErr = PB_ERR_UNKNOWN;
        return NULL;
    }
    MFinfo("name[%s] time[%s]", name? name: "NULL", mf_time_to_string(time));

    return mf_pb_alloc_tag_item(pbhdl->player, name, time, penErr);
}

void
msfs_pb_free_tag_item(struct item_tag_t *item)
{
    if (!item)
    {
        MFerr("item is NULL");
        return;
    }

    mf_pb_free_tag_item(item);
}

int
msfs_pb_get_thumb(struct pb_hdl_t *pbhdl, MF_PTS time, struct mf_frame *pThumb)
{
    if (!pbhdl)
    {
        MFerr("pbhdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("time[%s]", mf_time_to_string(time));

    struct stream_seg * s;
    
    s = mf_pb_get_stream(pbhdl->player, time); 

    if (!s)
        return MF_FAILURE;
            
    return mf_retr_get_Iframe_by_time(s, pThumb, time);;
}

void 
msfs_pb_put_thumb(struct pb_hdl_t *pbhdl, struct mf_frame *pThumb)
{
    mf_retr_put_Iframe(pThumb);
}

void
msfs_pb_dbg_mem_usage()
{
    MFinfo("msfs_pb_dbg_mem_usage");
    mf_pb_dbg_show_mem_usage();
}

void
msfs_pb_dbg_show_chn()
{
    MFinfo("msfs_pb_dbg_show_chn");
    mf_pb_dbg_show_chn();
}

void
msfs_pb_dbg_switch(MF_BOOL bDebug)
{
    MFinfo("bDebug[%d]", bDebug);
    mf_pb_dbg_switch(bDebug);
}

MF_S32
msfs_pb_init(PB_OUT_CB func)
{
    MFinfo("func[%p]", func);
    mf_pb_init(func);
    
    return MF_SUCCESS;
}

MF_S32
msfs_pb_deinit()
{
    MFinfo("msfs_pb_deinit");
    mf_pb_deinit();
    
    return MF_SUCCESS;
}

