/*
 * ***************************************************************
 * Filename:      	vapi_vdec.c
 * Created at:    	2015.10.21
 * Description:   	video decode controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vapi_comm.h"

typedef struct vdec_s
{
    CHNINFO_S stChn[MAX_CHN_NUM];
    VDEC_CB_S stCallback;
    td_u8    *pstBuff[MAX_CHN_NUM];
    td_s32    isBlock;
} VDEC_S;

static VDEC_S g_stVdec;

static ot_payload_type vapi_vdec_get_format(TYPE_E enType)
{
    ot_payload_type enPT;

    switch (enType) {
    case ENC_TYPE_H264:
        enPT = OT_PT_H264;
        break;
    case ENC_TYPE_H265:
        enPT = OT_PT_H265;
        break;
    case ENC_TYPE_MJPEG:
        enPT = OT_PT_MJPEG;
        break;
    default:
        VAPILOG("can not support type[%d], use default[H264]", enType);
        enPT = OT_PT_H264;
        break;
    }

    return enPT;
}

td_s32 vapi_vdec_chn_create(int DevId, ot_vdec_chn VdChn, TYPE_E enType, td_u32 u32Width, td_u32 u32Height)
{
    ot_vdec_chn_attr      stVdecChnAttr = {0};
    ot_vdec_chn_param     stParam       = {0};
    ot_payload_type       enPT;
    ot_video_display_mode enDisplayMode = OT_VIDEO_DISPLAY_MODE_PREVIEW;
    td_s32                s32Ret        = TD_SUCCESS;

    if (g_stVdec.stChn[VdChn].enState == STATE_BUSY) {
        // VAPILOG("vdec channel[%d] has existed !\n", VdChn);
        return TD_SUCCESS;
    }

    enPT = vapi_vdec_get_format(enType);

    ot_pic_buf_attr buf_attr = {0};
    buf_attr.align           = 0;
    buf_attr.height          = u32Height;
    buf_attr.width           = u32Width;

    stVdecChnAttr.type            = enPT;
    stVdecChnAttr.mode            = OT_VDEC_SEND_MODE_FRAME;
    stVdecChnAttr.stream_buf_size = u32Width * u32Height;
    stVdecChnAttr.pic_width       = u32Width;
    stVdecChnAttr.pic_height      = u32Height;
    if (OT_PT_H264 == enPT || OT_PT_MP4VIDEO == enPT) {
        buf_attr.bit_width                       = OT_DATA_BIT_WIDTH_8;
        buf_attr.pixel_format                    = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVdecChnAttr.video_attr.ref_frame_num   = 2;
        stVdecChnAttr.video_attr.temporal_mvp_en = 0;
        stVdecChnAttr.frame_buf_cnt              = 5;
        stVdecChnAttr.frame_buf_size             = ot_vdec_get_pic_buf_size(stVdecChnAttr.type, &buf_attr);
    } else if (OT_PT_JPEG == enPT || OT_PT_MJPEG == enPT) {
        buf_attr.bit_width           = OT_DATA_BIT_WIDTH_8;
        buf_attr.pixel_format        = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVdecChnAttr.frame_buf_cnt  = 3;
        stVdecChnAttr.frame_buf_size = ot_vdec_get_pic_buf_size(stVdecChnAttr.type, &buf_attr);
    } else if (OT_PT_H265 == enPT) {
        buf_attr.bit_width                       = OT_DATA_BIT_WIDTH_8;
        buf_attr.pixel_format                    = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stVdecChnAttr.video_attr.ref_frame_num   = 2;
        stVdecChnAttr.video_attr.temporal_mvp_en = 1;
        stVdecChnAttr.video_attr.tmv_buf_size    = ot_vdec_get_tmv_buf_size(enPT, u32Width, u32Height);
        stVdecChnAttr.frame_buf_cnt              = 5;
        stVdecChnAttr.frame_buf_size             = ot_vdec_get_pic_buf_size(stVdecChnAttr.type, &buf_attr);
    }

    ot_vdec_chn_config chn_config;
    ss_mpi_vdec_get_chn_config(VdChn, &chn_config);
    chn_config.deployment_mode = OT_VDEC_DEPLOYMENT_MODE0;
    ss_mpi_vdec_set_chn_config(VdChn, &chn_config);

    s32Ret = ss_mpi_vdec_create_chn(VdChn, &stVdecChnAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_create_chn[%d] failed with %#x!\n", VdChn, s32Ret);
        VAPILOG("error: width = %d. height = %d encType = %d \n", u32Width, u32Height, enPT);
        goto verr;
    }

    enDisplayMode = g_stVdec.isBlock ? OT_VIDEO_DISPLAY_MODE_PLAYBACK : OT_VIDEO_DISPLAY_MODE_PREVIEW;
    s32Ret        = ss_mpi_vdec_set_display_mode(VdChn, enDisplayMode);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_set_display_mode[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
        goto verr;
    }

    s32Ret = ss_mpi_vdec_get_chn_param(VdChn, &stParam);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_get_chn_param[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
        goto verr;
    }
    stParam.display_frame_num = 4;
    stParam.video_param.compress_mode = OT_COMPRESS_MODE_NONE;
    s32Ret = ss_mpi_vdec_set_chn_param(VdChn, &stParam);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_set_chn_param[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
        goto verr;
    }

    s32Ret = ss_mpi_vdec_start_recv_stream(VdChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_start_recv_stream[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
        goto verr;
    }

    g_stVdec.stChn[VdChn].enState         = STATE_BUSY;
    g_stVdec.stChn[VdChn].stFormat.enType = enType;
    g_stVdec.stChn[VdChn].stFormat.width  = u32Width;
    g_stVdec.stChn[VdChn].stFormat.height = u32Height;

    if (g_stVdec.stCallback.update_chn_cb != NULL) {
        g_stVdec.stCallback.update_chn_cb(DevId, VdChn, STATE_BUSY, u32Width * u32Height);
    }

    if (g_stVdec.stCallback.start_stream_cb != NULL) {
        g_stVdec.stCallback.start_stream_cb(VdChn);
    }
    return TD_SUCCESS;
verr:
    if (g_stVdec.stCallback.update_chn_cb != NULL) {
        g_stVdec.stCallback.update_chn_cb(-1, VdChn, STATE_IDLE, 0);
    }
    return TD_FAILURE;
}

td_s32 vapi_vdec_chn_destory(ot_vdec_chn VdChn)
{
    td_s32 s32Ret;

    if (VdChn < 0)
        return TD_FAILURE;

    if (g_stVdec.stChn[VdChn].enState != STATE_BUSY) {
        return TD_SUCCESS;
    }

    if (g_stVdec.stCallback.stop_stream_cb != NULL) {
        g_stVdec.stCallback.stop_stream_cb(VdChn);
    }

    s32Ret = ss_mpi_vdec_stop_recv_stream(VdChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_stop_recv_stream[%d] failed with %#x!\n", VdChn, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vdec_destroy_chn(VdChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_destroy_chn[%d] failed with %#x!\n", VdChn, s32Ret);
        return TD_FAILURE;
    }

    g_stVdec.stChn[VdChn].enState         = STATE_IDLE;
    g_stVdec.stChn[VdChn].stFormat.width  = 0;
    g_stVdec.stChn[VdChn].stFormat.height = 0;

    if (g_stVdec.stCallback.update_chn_cb != NULL) {
        g_stVdec.stCallback.update_chn_cb(-1, VdChn, STATE_IDLE, 0);
    }

    return TD_SUCCESS;
}

td_s32 vapi_vdec_bind_vpss(ot_vdec_chn VdChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn)
{
    td_s32     s32Ret;
    ot_mpp_chn stSrcChn;
    ot_mpp_chn stDestChn;

    stSrcChn.mod_id = OT_ID_VDEC;
    stSrcChn.dev_id = 0;
    stSrcChn.chn_id = VdChn;

    stDestChn.mod_id = OT_ID_VPSS;
    stDestChn.dev_id = VpssGrp;
    stDestChn.chn_id = VpssChn;

    s32Ret = ss_mpi_sys_bind(&stSrcChn, &stDestChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_sys_bind [%d->%d] failed with %#x!\n", VdChn, VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 vapi_vdec_unbind_vpss(ot_vdec_chn VdChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn)
{
    td_s32     s32Ret;
    ot_mpp_chn stSrcChn;
    ot_mpp_chn stDestChn;

    stSrcChn.mod_id = OT_ID_VDEC;
    stSrcChn.dev_id = 0;
    stSrcChn.chn_id = VdChn;

    stDestChn.mod_id = OT_ID_VPSS;
    stDestChn.dev_id = VpssGrp;
    stDestChn.chn_id = VpssChn;

    s32Ret = ss_mpi_sys_unbind(&stSrcChn, &stDestChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_sys_unbind [%d->%d] failed with %#x!\n", VdChn, VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_void vapi_vdec_update_state(ot_vdec_chn VdChn, td_s32 state)
{
    g_stVdec.stChn[VdChn].enState = state;
}

td_bool vapi_vdec_check_state(ot_vdec_chn VdChn, td_s32 state)
{
    return (g_stVdec.stChn[VdChn].enState == state);
}

td_void vapi_vdec_dsp_block(td_bool isBlock)
{
    g_stVdec.isBlock = isBlock;
}

td_s32 vapi_vdec_send_frame(ot_vdec_chn VdChn, td_u8 *pu8Addr, td_u32 u32Len, td_u64 u64PTS)
{
    ot_vdec_stream     stStream;
    ot_vdec_chn_status stStat;
    td_s32             s32Ret;

    memset(&stStream, 0, sizeof(stStream));
    stStream.pts           = u64PTS; // us
    stStream.addr          = pu8Addr;
    stStream.len           = u32Len;
    stStream.end_of_frame  = TD_TRUE;
    stStream.end_of_stream = TD_FALSE;
    stStream.need_display  = TD_TRUE;
    s32Ret                 = ss_mpi_vdec_send_stream(VdChn, &stStream, 0); // noblock
    if (TD_SUCCESS != s32Ret) {
        if (s32Ret != OT_ERR_VDEC_UNEXIST) {
            VAPILOG("ss_mpi_vdec_send_stream [%d] failed with %#x!\n", VdChn, s32Ret);
            VAPILOG("ss_mpi_vdec_send_stream [%d]  PTS:%llu Addr:0x%p Len:%d\n", VdChn, u64PTS, pu8Addr, u32Len);
        }
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vdec_query_status(VdChn, &stStat);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vdec_query_status [%d] failed with %#x!\n", VdChn, s32Ret);
        return TD_FAILURE;
    }

    return stStat.left_stream_frames;
}

td_s32 vapi_vdec_start_stream(ot_vdec_chn VdChn)
{
    if (g_stVdec.stCallback.start_stream_cb != NULL) {
        g_stVdec.stCallback.start_stream_cb(VdChn);
    }

    return TD_SUCCESS;
}

td_s32 vapi_vdec_stop_stream(ot_vdec_chn VdChn)
{
    if (g_stVdec.stCallback.stop_stream_cb != NULL) {
        g_stVdec.stCallback.stop_stream_cb(VdChn);
    }

    return TD_SUCCESS;
}

td_void vapi_vdec_init(VDEC_CB_S *pstCb, int isBlock)
{
    td_s32            i, s32Ret;
    ot_vdec_mod_param mod_param = {0};

    s32Ret = ss_mpi_vdec_get_mod_param(&mod_param);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_get_mod_param failed!\n");
        return;
    }
    mod_param.vb_src = OT_VB_SRC_PRIVATE;

    s32Ret = ss_mpi_vdec_set_mod_param(&mod_param);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_set_mod_param failed!\n");
        return;
    }

    for (i = 0; i < MAX_CHN_NUM; i++) {
        g_stVdec.stChn[i].enState = STATE_IDLE;
    }

    g_stVdec.stCallback.start_stream_cb = pstCb->start_stream_cb;
    g_stVdec.stCallback.stop_stream_cb  = pstCb->stop_stream_cb;
    g_stVdec.stCallback.update_chn_cb   = pstCb->update_chn_cb;
    g_stVdec.isBlock                    = isBlock;
}

td_void vapi_vdec_uninit()
{
    td_s32 i;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        vapi_vdec_chn_destory(i);
    }

    g_stVdec.stCallback.start_stream_cb = NULL;
    g_stVdec.stCallback.stop_stream_cb  = NULL;
    g_stVdec.stCallback.update_chn_cb   = NULL;
}
