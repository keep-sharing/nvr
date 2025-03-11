/*
 * ***************************************************************
 * Filename:      	vapi_jpeg.c
 * Created at:    	2019.08.21
 * Description:   	video jpeg encoder
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

#define EPOLL_EVENT 1
typedef struct jpeg_s
{
    td_s32          epfd;
    td_s32          vencfd;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_ex;
} JPEG_S;

static JPEG_S g_stJpeg;

static ot_payload_type vdec_get_format(TYPE_E enType)
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

static td_s32 yuv420sp_to_jpeg(ot_video_frame_info *pstFrame, td_u8 **ppData, td_u32 *pSize, td_u32 *pWidth, td_u32 *pHeight)
{
    td_s32             s32Ret;
    ot_venc_chn_status stStat;
    ot_venc_stream     stStream;
    td_u8             *data  = NULL;
    td_u32             size  = 0;
    td_s32             i     = 0;
    td_s32             retry = 10;
    td_u32             pack_size;

    td_u32 u32Width  = VAPI_MIN(pstFrame->video_frame.width, VAPI_VENC_MAX_WIDTH);
    td_u32 u32Height = VAPI_MIN(pstFrame->video_frame.height, VAPI_VENC_MAX_HEIGTH);

    ot_venc_chn_attr stVencChnAttr                                = {0};
    stVencChnAttr.venc_attr.type                                  = OT_PT_JPEG;
    stVencChnAttr.venc_attr.max_pic_width                         = u32Width;
    stVencChnAttr.venc_attr.max_pic_height                        = u32Height;
    stVencChnAttr.venc_attr.pic_width                             = u32Width;
    stVencChnAttr.venc_attr.pic_height                            = u32Height;
    stVencChnAttr.venc_attr.buf_size                              = ALIGN_UP(u32Width, 16) * ALIGN_UP(u32Height, 16);
    stVencChnAttr.venc_attr.is_by_frame                           = TD_TRUE; /*get stream mode is field mode  or frame mode*/
    stVencChnAttr.venc_attr.profile                               = 0;
    stVencChnAttr.venc_attr.jpeg_attr.dcf_en                      = TD_FALSE;
    stVencChnAttr.venc_attr.jpeg_attr.mpf_cfg.large_thumbnail_num = 0;
    stVencChnAttr.venc_attr.jpeg_attr.recv_mode                   = OT_VENC_PIC_RECV_SINGLE;
    stVencChnAttr.gop_attr.gop_mode                               = OT_VENC_GOP_MODE_NORMAL_P;
    stVencChnAttr.gop_attr.normal_p.ip_qp_delta                   = 0;

    s32Ret = ss_mpi_venc_create_chn(VAPI_SNAP_VENC_ID, &stVencChnAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_create_chn faild with%#x!\n", s32Ret);
        return s32Ret;
    }

    ot_venc_start_param start_param = {0};
    start_param.recv_pic_num = -1;
    s32Ret                   = ss_mpi_venc_start_chn(VAPI_SNAP_VENC_ID, &start_param);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_start_chn faild with%#x!\n", s32Ret);
        goto OUT;
    }

    s32Ret = ss_mpi_venc_send_frame(VAPI_SNAP_VENC_ID, pstFrame, -1);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_send_frame faild with%#x!\n", s32Ret);
        goto OUT;
    }

    memset(&stStream, 0, sizeof(stStream));
    while (retry--) {
        s32Ret = ss_mpi_venc_query_status(VAPI_SNAP_VENC_ID, &stStat);
        if (TD_SUCCESS != s32Ret) {
            VAPILOG("ss_mpi_venc_query_status failed with %#x!\n", s32Ret);
            s32Ret = TD_FAILURE;
            goto OUT;
        }
        if (0 == stStat.cur_packs) {
            usleep(20000);
            s32Ret = TD_FAILURE;
            continue;
        }
        s32Ret = TD_SUCCESS;
        break;
    }

    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_query_status no packs !\n");
        goto OUT;
    }

    stStream.pack = (ot_venc_pack *)ms_malloc(sizeof(ot_venc_pack) * stStat.cur_packs);
    if (NULL == stStream.pack) {
        VAPILOG("malloc stream pack failed!\n");
        s32Ret = TD_FAILURE;
        goto OUT;
    }

    stStream.pack_cnt = stStat.cur_packs;
    s32Ret            = ss_mpi_venc_get_stream(VAPI_SNAP_VENC_ID, &stStream, TD_TRUE);
    if (TD_SUCCESS != s32Ret) {
        ms_free(stStream.pack);
        stStream.pack = NULL;
        VAPILOG("ss_mpi_venc_get_stream failed with %#x!\n", s32Ret);
        goto OUT;
    }
    size = 0;
    for (i = 0; i < stStream.pack_cnt; i++) {
        size += stStream.pack[i].len - stStream.pack[i].offset;
    }

    data = ms_malloc(size);
    if (data == NULL) {
        ss_mpi_venc_release_stream(VAPI_SNAP_VENC_ID, &stStream);
        ms_free(stStream.pack);
        s32Ret = TD_FAILURE;
        goto OUT;
    }
    pack_size = 0;
    for (i = 0; i < stStream.pack_cnt; i++) {
        memcpy(data + pack_size, stStream.pack[i].addr + stStream.pack[i].offset, stStream.pack[i].len - stStream.pack[i].offset);
        pack_size += stStream.pack[i].len - stStream.pack[i].offset;
    }
    s32Ret = ss_mpi_venc_release_stream(VAPI_SNAP_VENC_ID, &stStream);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_release_stream failed with %#x!\n", s32Ret);
        ms_free(data);
    }

    ms_free(stStream.pack);
OUT:
    ss_mpi_venc_stop_chn(VAPI_SNAP_VENC_ID);
    ss_mpi_venc_destroy_chn(VAPI_SNAP_VENC_ID);
    *ppData  = data;
    *pSize   = size;
    *pWidth  = u32Width;
    *pHeight = u32Height;

    return s32Ret;
}

static td_s32 vdec_chn_create(ot_vdec_chn VdChn, TYPE_E enType, td_u32 u32Width, td_u32 u32Height)
{
    ot_vdec_chn_attr  stVdecChnAttr = {0};
    ot_vdec_chn_param stParam       = {0};
    ot_payload_type   enPT;
    td_s32            buffsize;
    td_s32            s32Ret;

    if (u32Width > VAPI_VDEC_MAX_WIDTH || u32Height > VAPI_VDEC_MAX_HEIGTH) {
        VAPILOG("resolution must be <= %dX%d", VAPI_VDEC_MAX_WIDTH, VAPI_VDEC_MAX_HEIGTH);
        return TD_FAILURE;
    }

    enPT = vdec_get_format(enType);

    ot_pic_buf_attr buf_attr = {0};
    buf_attr.align           = 0;
    buf_attr.height          = u32Height;
    buf_attr.width           = u32Width;

    stVdecChnAttr.type            = enPT;
    stVdecChnAttr.mode            = OT_VDEC_SEND_MODE_FRAME;
    stVdecChnAttr.stream_buf_size = u32Width * u32Height * 1.5;
    stVdecChnAttr.pic_width       = ALIGN_UP(u32Width, 16);
    stVdecChnAttr.pic_height      = ALIGN_UP(u32Height, 16);
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

    ot_vdec_chn_config chn_config = {0};
    ss_mpi_vdec_get_chn_config(VdChn, &chn_config);
    chn_config.deployment_mode = OT_VDEC_DEPLOYMENT_MODE0;
    ss_mpi_vdec_set_chn_config(VdChn, &chn_config);

    s32Ret = ss_mpi_vdec_create_chn(VdChn, &stVdecChnAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_create_chn[%d] failed with %#x!\n", VdChn, s32Ret);
        VAPILOG("error: width = %d. height = %d encType = %d \n", u32Width, u32Height, enPT);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vdec_set_display_mode(VdChn, OT_VIDEO_DISPLAY_MODE_PREVIEW);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_set_display_mode[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vdec_get_chn_param(VdChn, &stParam);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("hi_mpi_vdec_get_chn_param[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
    }

    stParam.display_frame_num = 1;
    stParam.video_param.compress_mode = OT_COMPRESS_MODE_NONE;
    s32Ret                    = ss_mpi_vdec_set_chn_param(VdChn, &stParam);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("hi_mpi_vdec_set_chn_param[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
    }

    s32Ret = ss_mpi_vdec_start_recv_stream(VdChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vdec_start_recv_stream[%d] failed with %#x!\n", VdChn, s32Ret);
        ss_mpi_vdec_destroy_chn(VdChn);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 vdec_chn_destory(ot_vdec_chn VdChn)
{
    td_s32 s32Ret;

    if (VdChn < 0)
        return TD_FAILURE;

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

    return TD_SUCCESS;
}

static td_s32 venc_chn_create(ot_venc_chn VeChn, td_u32 u32Qfactor, td_u32 u32Width, td_u32 u32Height, td_u32 *pu32OutWidth, td_u32 *pu32OutHeight)
{
    td_s32             s32Ret;
    ot_venc_chn_attr   stVencChnAttr = {0};
    ot_venc_jpeg_param stJpegParam   = {0};

    u32Width       = ALIGN_BACK(VAPI_MIN(u32Width, VAPI_VENC_MAX_WIDTH), 4);
    u32Height      = ALIGN_BACK(VAPI_MIN(u32Height, VAPI_VENC_MAX_HEIGTH), 4);
    *pu32OutWidth  = u32Width;
    *pu32OutHeight = u32Height;

    stVencChnAttr.venc_attr.type                                  = OT_PT_JPEG;
    stVencChnAttr.venc_attr.max_pic_width                         = u32Width;
    stVencChnAttr.venc_attr.max_pic_height                        = u32Height;
    stVencChnAttr.venc_attr.pic_width                             = u32Width;
    stVencChnAttr.venc_attr.pic_height                            = u32Height;
    stVencChnAttr.venc_attr.buf_size                              = ALIGN_UP(u32Width, 16) * ALIGN_UP(u32Height, 16);
    stVencChnAttr.venc_attr.is_by_frame                           = TD_TRUE; /*get stream mode is field mode  or frame mode*/
    stVencChnAttr.venc_attr.profile                               = 0;
    stVencChnAttr.venc_attr.jpeg_attr.dcf_en                      = TD_FALSE;
    stVencChnAttr.venc_attr.jpeg_attr.mpf_cfg.large_thumbnail_num = 0;
    stVencChnAttr.venc_attr.jpeg_attr.recv_mode                   = OT_VENC_PIC_RECV_SINGLE;

    s32Ret = ss_mpi_venc_create_chn(VeChn, &stVencChnAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_create_chn [%d] faild with %#x!\n", VeChn, s32Ret);
        VAPILOG("u32Qfactor[%d] u32Width[%d] u32Height[%d]\n", u32Qfactor, u32Width, u32Height)
        return s32Ret;
    }

    s32Ret = ss_mpi_venc_get_jpeg_param(VeChn, &stJpegParam);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_get_jpeg_param [%d] faild with %#x!\n", VeChn, s32Ret);
        return s32Ret;
    }
    stJpegParam.qfactor = u32Qfactor;
    s32Ret              = ss_mpi_venc_set_jpeg_param(VeChn, &stJpegParam);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_set_jpeg_param [%d] faild with %#x!\n", VeChn, s32Ret);
        return s32Ret;
    }

    ot_venc_start_param venc_start_param = {0};
    venc_start_param.recv_pic_num = -1;
    s32Ret                        = ss_mpi_venc_start_chn(VeChn, &venc_start_param);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_start_chn faild with%#x!\n", s32Ret);
        return TD_FAILURE;
    }

    g_stJpeg.vencfd = ss_mpi_venc_get_fd(VeChn);
    comm_epoll_add(g_stJpeg.epfd, g_stJpeg.vencfd, EPOLLIN);

    return TD_SUCCESS;
}

static td_s32 venc_chn_destory(ot_venc_chn VeChn)
{
    td_s32 s32Ret;

    if (VeChn < 0)
        return TD_FAILURE;

    s32Ret = ss_mpi_venc_stop_chn(VeChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_stop_chn vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_venc_close_fd(VeChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_close_fd vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_venc_destroy_chn(VeChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_destroy_chn vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        return TD_FAILURE;
    }

    comm_epoll_del(g_stJpeg.epfd, g_stJpeg.vencfd, EPOLLIN);

    return TD_SUCCESS;
}

static td_s32 vdec_bind_venc(ot_vdec_chn VdChn, ot_venc_chn VeChn)
{
    td_s32     s32Ret;
    ot_mpp_chn stSrcChn;
    ot_mpp_chn stDestChn;

    stSrcChn.mod_id = OT_ID_VDEC;
    stSrcChn.dev_id = 0;
    stSrcChn.chn_id = VdChn;

    stDestChn.mod_id = OT_ID_VENC;
    stDestChn.dev_id = 0;
    stDestChn.chn_id = VeChn;

    s32Ret = ss_mpi_sys_bind(&stSrcChn, &stDestChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_sys_bind [%d->%d] failed with %#x!\n", VdChn, VeChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 vdec_unbind_venc(ot_vdec_chn VdChn, ot_venc_chn VeChn)
{
    td_s32     s32Ret;
    ot_mpp_chn stSrcChn;
    ot_mpp_chn stDestChn;

    stSrcChn.mod_id = OT_ID_VDEC;
    stSrcChn.dev_id = 0;
    stSrcChn.chn_id = VdChn;

    stDestChn.mod_id = OT_ID_VENC;
    stDestChn.dev_id = 0;
    stDestChn.chn_id = VeChn;

    s32Ret = ss_mpi_sys_unbind(&stSrcChn, &stDestChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_sys_unbind [%d->%d] failed with %#x!\n", VdChn, VeChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static td_s32 jpeg_create(TYPE_E enType, td_u32 u32InWidth, td_u32 u32InHeight, td_u32 u32Qfactor, td_u32 u32OutWidth, td_u32 u32OutHeight, td_u32 *pu32OutWidth,
                          td_u32 *pu32OutHeight)
{
    td_s32 s32Ret;

    s32Ret = vdec_chn_create(VAPI_JPEG_VDEC_ID, enType, u32InWidth, u32InHeight);
    if (s32Ret != TD_SUCCESS)
        return s32Ret;

    u32OutWidth  = VAPI_MIN(u32InWidth, u32OutWidth);
    u32OutHeight = VAPI_MIN(u32InHeight, u32OutHeight);
    u32OutWidth  = VAPI_MAX(ALIGN_UP(u32InWidth / 15, 16), u32OutWidth);
    u32OutHeight = VAPI_MAX(ALIGN_UP(u32InHeight / 15, 16), u32OutHeight);

    s32Ret = venc_chn_create(VAPI_JPEG_VENC_ID, u32Qfactor, u32OutWidth, u32OutHeight, pu32OutWidth, pu32OutHeight);

    if (s32Ret == TD_SUCCESS) {
        s32Ret = vdec_bind_venc(VAPI_JPEG_VDEC_ID, VAPI_JPEG_VENC_ID);
    } else {
        VAPILOG("Venc_chn_create failed with %#x", s32Ret);
        vdec_chn_destory(VAPI_JPEG_VDEC_ID);
    }

    return s32Ret;
}

static td_void jpeg_destroy(ot_vdec_chn VdChn, ot_venc_chn VeChn)
{
    vdec_unbind_venc(VdChn, VeChn);
    vdec_chn_destory(VdChn);
    venc_chn_destory(VeChn);
}

static td_s32 jpeg_get_stream(ot_venc_chn Vechn, td_u8 **ppu8OutAddr, td_u32 *pu32OutLen)
{

    struct epoll_event events[EPOLL_EVENT];
    ot_venc_chn_status stStat;
    ot_venc_stream     stStream;
    td_u8             *data = NULL;
    td_u32             size = 0;
    td_s32             nfds = 0;
    td_s32             i    = 0;
    td_u32             pack_size;
    td_s32             s32Ret;

    nfds = comm_epoll_wait(g_stJpeg.epfd, events, EPOLL_EVENT, 1000);
    if (nfds != 1) {
        VAPILOG("jpeg_get_stream chn[%d] time out !\n", Vechn);
        return TD_FAILURE;
    } else {
        if ((events[0].events & EPOLLIN) && events[0].data.fd) {
            memset(&stStream, 0, sizeof(stStream));
            s32Ret = ss_mpi_venc_query_status(Vechn, &stStat);
            if (TD_SUCCESS != s32Ret) {
                VAPILOG("ss_mpi_venc_query_status chn[%d] failed with %#x!\n", Vechn, s32Ret);
                return TD_FAILURE;
            }

            if (0 == stStat.cur_packs)
                return TD_FAILURE;

            stStream.pack = (ot_venc_pack *)ms_malloc(sizeof(ot_venc_pack) * stStat.cur_packs);
            if (NULL == stStream.pack) {
                VAPILOG("malloc stream pack failed!\n");
                return TD_FAILURE;
            }

            stStream.pack_cnt = stStat.cur_packs;
            s32Ret            = ss_mpi_venc_get_stream(Vechn, &stStream, TD_TRUE);
            if (TD_SUCCESS != s32Ret) {
                ms_free(stStream.pack);
                stStream.pack = NULL;
                VAPILOG("ss_mpi_venc_get_stream failed with %#x!\n", s32Ret);
                return TD_FAILURE;
            }

            for (i = 0; i < stStream.pack_cnt; i++)
                size += stStream.pack[i].len - stStream.pack[i].offset;

            data = ms_malloc(size);
            if (data == NULL) {
                ms_free(stStream.pack);
                return TD_FAILURE;
            }
            pack_size = 0;
            for (i = 0; i < stStream.pack_cnt; i++) {
                memcpy(data + pack_size, stStream.pack[i].addr + stStream.pack[i].offset, stStream.pack[i].len - stStream.pack[i].offset);
                pack_size += stStream.pack[i].len - stStream.pack[i].offset;
            }
            s32Ret = ss_mpi_venc_release_stream(Vechn, &stStream);
            if (TD_SUCCESS != s32Ret) {
                ms_free(stStream.pack);
                ms_free(data);
                stStream.pack = NULL;
                return TD_FAILURE;
            }

            ms_free(stStream.pack);
            stStream.pack = NULL;
        }
    }

    *ppu8OutAddr = data;
    *pu32OutLen  = size;

    return TD_SUCCESS;
}

td_s32 vapi_jpeg_hw(TYPE_E enType, td_u32 u32InWidth, td_u32 u32InHeight, td_u8 *pu8InAddr, td_u32 u32InLen, td_u32 u32Qfactor, td_u32 u32OutWidth, td_u32 u32OutHeight,
                    td_u8 **ppu8OutAddr, td_u32 *pu32OutLen, td_u32 *pu32OutWidth, td_u32 *pu32OutHeight, pthread_mutex_t *mutex)
{
    ot_vdec_stream stStream = {0};
    td_s32         s32Ret   = TD_FAILURE;

    if (comm_mutex_timelock(&g_stJpeg.mutex, 1000000) != 0)
        return s32Ret;

    comm_mutex_lock(mutex);
    s32Ret = jpeg_create(enType, u32InWidth, u32InHeight, u32Qfactor, u32OutWidth, u32OutHeight, pu32OutWidth, pu32OutHeight);
    comm_mutex_unlock(mutex);
    if (s32Ret != TD_SUCCESS) {
        comm_mutex_unlock(&g_stJpeg.mutex);
        return s32Ret;
    }

    stStream.pts           = 0;
    stStream.addr          = pu8InAddr;
    stStream.len           = u32InLen;
    stStream.end_of_frame  = TD_TRUE;
    stStream.end_of_stream = TD_TRUE;
    stStream.need_display  = TD_TRUE;
    s32Ret = ss_mpi_vdec_send_stream(VAPI_JPEG_VDEC_ID, &stStream, -1);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vdec_send_stream failed with %#x!\n", s32Ret);
        goto JOUT;
    }

    s32Ret = jpeg_get_stream(VAPI_JPEG_VENC_ID, ppu8OutAddr, pu32OutLen);

JOUT:
    comm_mutex_lock(mutex);
    jpeg_destroy(VAPI_JPEG_VDEC_ID, VAPI_JPEG_VENC_ID);
    comm_mutex_unlock(mutex);
    comm_mutex_unlock(&g_stJpeg.mutex);

    return s32Ret;
}

td_s32 vapi_yuv2jpeg(ot_video_frame_info *pstFrame, td_u8 **ppData, td_u32 *pSize, td_u32 *pWidth, td_u32 *pHeight)
{
    td_s32 s32Ret;
    comm_mutex_lock(&g_stJpeg.mutex_ex);
    s32Ret = yuv420sp_to_jpeg(pstFrame, ppData, pSize, pWidth, pHeight);
    comm_mutex_unlock(&g_stJpeg.mutex_ex);

    return s32Ret;
}

td_void vapi_jpeg_init()
{
    memset(&g_stJpeg, 0, sizeof(JPEG_S));
    g_stJpeg.epfd = comm_epoll_create(EPOLL_EVENT);
    comm_mutex_init(&g_stJpeg.mutex);
    comm_mutex_init(&g_stJpeg.mutex_ex);
}

td_void vapi_jpeg_uninit()
{
    comm_mutex_uninit(&g_stJpeg.mutex);
    comm_mutex_uninit(&g_stJpeg.mutex_ex);
    comm_epoll_destroy(g_stJpeg.epfd);
}
