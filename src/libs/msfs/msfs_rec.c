/*
 * ***************************************************************
 * Filename:        msfs_record.c
 * Created at:      2017.10.10
 * Description:     msfs record API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include "MFdisk.h"
#include "MFrecord.h"
#include "MFtask.h"
#include "msfs_rec.h"

struct rec_hdl_t *
msfs_rec_create(MF_U8 chnId, VID_EN enVid, MF_U32 bandwidth)
{
    if (enVid >= VID_TYPE_NUM) {
        MFerr("enVid[%d] overflow[%d]", enVid, VID_TYPE_NUM);
        return NULL;
    }
    MFinfo("chnid[%d] enVid[%d] bandwidth[%d]", chnId, enVid, bandwidth);

    struct rec_hdl_t *rechdl;
    struct recHDL *vidHdl;
    struct recHDL *picHdl;

    vidHdl = mf_record_vid_create(chnId, enVid, bandwidth);
    if (vidHdl == NULL) {
        MFinfo("chnid[%d] enVid[%d] REC failed", chnId, enVid);
        return NULL;
    }
    rechdl = ms_malloc(sizeof(struct rec_hdl_t));
    vidHdl->owner = rechdl;
    rechdl->vidHdl = vidHdl;
    rechdl->enVid = enVid;

    picHdl = mf_record_pic_create(chnId, HDL_PIC);
    mf_record_pic_start(picHdl);
    picHdl->owner = rechdl;
    rechdl->picHdl = picHdl;

    return rechdl;
}

MF_S32
msfs_rec_destory(struct rec_hdl_t *rechdl)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }
    mf_record_pic_stop(rechdl->picHdl);
    mf_record_pic_destory(rechdl->picHdl);
    mf_record_vid_destory(rechdl->vidHdl);
    ms_free(rechdl);

    return MF_SUCCESS;
}

MF_S32
msfs_rec_start(struct rec_hdl_t *rechdl, REC_EVENT_EN enEvent)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }

    MFinfo("rechdl[%p] enEvent[%#x]", rechdl, enEvent);

    return  mf_record_vid_start(rechdl->vidHdl, enEvent);
}

MF_S32
msfs_rec_stop(struct rec_hdl_t *rechdl, MF_BOOL isForce)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("rechdl[%p] isForce[%d]", rechdl, isForce);

    if (isForce) {
        return mf_record_vid_hard_stop(rechdl->vidHdl, MF_YES);
    } else {
        return mf_record_vid_soft_stop(rechdl->vidHdl);
    }
}

MF_S32
msfs_rec_snapshot(struct rec_hdl_t *rechdl, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor, MF_PTS pts,
    MF_S8 *priData, MF_U8 priSize)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("enMajor[%#x] enMinor[%#x] pts[%s]", enMajor, enMinor, mf_time_to_string(pts));

    return mf_record_pic_snap(rechdl->picHdl, rechdl->enVid, enMajor, enMinor, pts, priData, priSize);
}

MF_S32
msfs_rec_snapshot_ex(struct rec_hdl_t *rechdl, INFO_MAJOR_EN enMajor, INFO_MINOR_EN enMinor, MF_U64 pts,
                     struct mf_frame *pstFrame, PIC_FORMAT_EN enFormat)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }

    pstFrame->frame_type    = FT_IFRAME;
    pstFrame->strm_type     = ST_VIDEO;
    pstFrame->stream_from   = SST_EVENT_STREAM;
    pstFrame->codec_type    = enFormat;
    pstFrame->stream_format = STREAM_TYPE_MAINSTREAM;
    pstFrame->time_usec     = pts;

    return mf_record_pic_frame(rechdl->picHdl, enMajor, enMinor, pstFrame, NULL, 0);
}

MF_S32
msfs_rec_mark_event(struct rec_hdl_t *rechdl, struct rec_event_t *evt)
{
    if (!rechdl || !evt) {
        MFerr("rechdl[%p] evt[%p]", rechdl, evt);
        return MF_FAILURE;
    }
    MFinfo("major[%#x] minor[%#x] srcName[%s] Time[%d,%d] enDtc[%d] bEnd[%d]",
           evt->enMajor, evt->enMinor, evt->srcName, (MF_S32)evt->startTime, (MF_S32)evt->endTime, evt->enDtc, evt->bEnd);
    return mf_record_vid_event(rechdl->vidHdl, evt);
}

MF_S32
msfs_rec_event_lpr(struct rec_hdl_t *rechdl, struct rec_lpr_t *pstLpr)
{
    if (!rechdl || !pstLpr) {
        MFerr("rechdl[%p] evt[%p]", rechdl, pstLpr);
        return MF_FAILURE;
    }

    MFinfo("Licence[%s]", pstLpr->Licence);
    return mf_record_pic_lpr(rechdl->picHdl, pstLpr);
}

MF_S32
msfs_rec_event_face(struct rec_hdl_t *rechdl, struct rec_face_t *pstFace)
{
    if (!rechdl || !pstFace) {
        MFerr("rechdl[%p] evt[%p]", rechdl, pstFace);
        return MF_FAILURE;
    }

    MFinfo("face nums[%d]", pstFace->num);
    return mf_record_pic_face(rechdl->picHdl, pstFace);
}

MF_S32
msfs_rec_event_pos(struct rec_hdl_t *rechdl, struct rec_smart_t *pstPos)
{
    if (!rechdl || !pstPos || !pstPos->frame) {
        MFerr("rechdl[%p] evt[%p]", rechdl, pstPos);
        return MF_FAILURE;
    }

    return mf_record_vid_smart(rechdl->vidHdl, pstPos, INFO_MINOR_POS);
}

MF_S32
msfs_rec_set_audio_enable(struct rec_hdl_t *rechdl, MF_BOOL bEnable)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("bEnable[%d]", bEnable);

    return mf_record_vid_set_audio(rechdl->vidHdl, bEnable);
}

MF_S32
msfs_rec_set_pre_time(struct rec_hdl_t *rechdl, MF_U32 PreTime)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("PreTime[%d]", PreTime);

    return mf_record_vid_set_pre_time(rechdl->vidHdl, PreTime);
}

MF_S32
msfs_rec_set_vid_deadline_time(struct rec_hdl_t *rechdl, MF_U32 DeadDay)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("DeadDay[%d]", DeadDay);

    return mf_record_vid_set_deadline_time(rechdl->vidHdl, DeadDay * 24 * 60 * 60);
}

MF_S32
msfs_rec_set_pic_deadline_time(struct rec_hdl_t *rechdl, MF_U32 DeadDay)
{
    if (!rechdl) {
        MFerr("rechdl is NULL");
        return MF_FAILURE;
    }
    MFinfo("DeadDay[%d]", DeadDay);

    return mf_record_vid_set_deadline_time(rechdl->picHdl, DeadDay * 24 * 60 * 60);
}

MF_S32
msfs_rec_set_vid_deadline_time_static(MF_U8 chnId, VID_EN enVid, MF_U32 DeadDay)
{
    if (enVid >= VID_TYPE_NUM) {
        MFerr("enVid[%d] overflow[%d]", enVid, VID_TYPE_NUM);
        return MF_FAILURE;
    }
    MFinfo("chnid[%d] enVid[%d] DeadDay[%d]", chnId, enVid, DeadDay);

    return mf_record_vid_set_deadline_time_static(chnId, enVid, DeadDay * 24 * 60 * 60);
}

MF_S32
msfs_rec_set_pic_deadline_time_static(MF_U8 chnId, MF_U32 DeadDay)
{
    MFinfo("chnid[%d] DeadDay[%d]", chnId,  DeadDay);

    return mf_record_pic_set_deadline_time_static(chnId, DeadDay * 24 * 60 * 60);
}

void
msfs_rec_deadline_check_start(MF_BOOL bStatic)
{
    MFinfo("bStatic[%d]", bStatic);
    mf_record_deadline_check_start(bStatic);
}

void
msfs_rec_deadline_check_stop()
{
    MFinfo("msfs_rec_deadline_check_stop");
    mf_record_deadline_check_stop();
}

MF_S32
msfs_rec_write(struct mf_frame *frame)
{
    mf_record_data_sink(frame);

    return MF_SUCCESS;
}

void
msfs_rec_update()
{
    MFinfo("msfs_rec_update");
    mf_record_update();
}

void
msfs_rec_dbg_rec_info(MF_U8 chnId)
{
    MFinfo("chnid[%d]", chnId);
    mf_record_dbg_show_chn_info(chnId);
}

void
msfs_rec_dbg_mem_usage()
{
    MFinfo("msfs_rec_dbg_mem_usage");
    mf_record_dbg_show_mem_usage();
}

void
msfs_rec_dbg_set_receive(MF_BOOL bReceive)
{
    MFinfo("bReceive[%d]", bReceive);
    mf_record_dbg_receive_enable(bReceive);
}

void
msfs_rec_dbg_switch(MF_BOOL bDebug)
{
    MFinfo("bDebug[%d]", bDebug);
    mf_record_dbg_switch(bDebug);
}

MF_S32
msfs_rec_init(struct rec_init_t *pstInit)
{
    if (!pstInit) {
        MFerr("pstInit is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_rec_init");
    mf_task_init();
    mf_record_init(pstInit->user_cb, pstInit->jpg_cb, pstInit->max_chn);

    return MF_SUCCESS;
}

MF_S32
msfs_rec_deinit()
{
    MFinfo("msfs_rec_deinit");
    mf_record_deinit();
    mf_task_deinit();

    return MF_SUCCESS;
}

MF_S32 msfs_rec_mem_outdate_free(void)
{
    return mf_record_mem_outdate_free();
}


