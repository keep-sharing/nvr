/*
 * ***************************************************************
 * Filename:      	vapi_venc.c
 * Created at:    	2018.10.16
 * Description:   	video encode controller
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

#define MAX_EPOLL_EVENT MAX_VENC_NUM + 1

typedef struct chn_s
{
    td_s32  fd;
    STATE_E enState;
    TYPE_E  enType;
    BIND_S  stBind;
    td_u32  fps;
    td_u32  width;
    td_u32  height;
    td_u32  bitRate;
    void   *pstPrivate;
} CHN_S;

typedef struct venc_s
{
    CHN_S           stChn[MAX_VENC_NUM];
    td_s32          count;
    td_s32          epfd;
    pthread_mutex_t mutex;
    pthread_t       pthread;
    td_bool         bRun;
    VDEC_CB_S       stCallback;
} VENC_S;

static VENC_S g_stVenc;

static ot_payload_type vapi_venc_get_format(TYPE_E enType)
{
    ot_payload_type enPT;

    switch (enType) {
    case ENC_TYPE_H264:
        enPT = OT_PT_H264;
        break;
#if 0
    case ENC_TYPE_H265:
        enPT = PT_H265;
        break;
    case ENC_TYPE_MJPEG:
        enPT = PT_MJPEG;
        break;
#endif
    default:
        VAPILOG("can not support type[%d], use default[H264]", enType);
        enPT = OT_PT_H264;
        break;
    }

    return enPT;
}

static void vapi_venc_alloc_frame(ot_venc_stream *pStream, VENC_FRAME_S *pFrame)
{
    td_s32 i;
    td_u32 pack_size;

    pFrame->size = 0;
    for (i = 0; i < pStream->pack_cnt; i++) {
        pFrame->size += pStream->pack[i].len - pStream->pack[i].offset;
    }

    pFrame->data = ms_malloc(pFrame->size);
    pack_size    = 0;
    for (i = 0; i < pStream->pack_cnt; i++) {
        memcpy(pFrame->data + pack_size, pStream->pack[i].addr + pStream->pack[i].offset, pStream->pack[i].len - pStream->pack[i].offset);
        pack_size += pStream->pack[i].len - pStream->pack[i].offset;
    }

    pFrame->isIframe = pStream->pack[0].data_type.h264_type == OT_VENC_H264_NALU_SPS ? 1 : 0;
    pFrame->pts      = pStream->pack[0].pts;
}

static void vapi_venc_free_frame(VENC_FRAME_S *pFrame)
{
    if (pFrame->data)
        ms_free(pFrame->data);
}

static inline ot_venc_chn vapi_venc_get_chn(td_s32 fd)
{
    td_s32 i;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        if (g_stVenc.stChn[i].fd == fd)
            return i;
    }

    return -1;
}
static ot_venc_chn vapi_venc_idel_chn()
{
    td_s32      i;
    ot_venc_chn Vechn = -1;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        if (g_stVenc.stChn[i].enState == STATE_IDLE) {
            Vechn = i;
            break;
        }
    }
    VAPILOG("find a venc chn [%d]!\n", Vechn);

    return Vechn;
}

#ifdef MAX_EPOLL_EVENT
static void *get_stream_task(void *argv)
{
    VENC_S            *pstVenc = (VENC_S *)argv;
    td_s32             i;
    td_s32             nfds  = 0;
    ot_venc_chn        Vechn = -1;
    struct epoll_event events[MAX_EPOLL_EVENT];

    ot_venc_chn_status stStat;
    ot_venc_stream     stStream;
    VENC_FRAME_S       stFrame;
    td_s32             s32Ret;

    comm_task_set_name("Venc Get Stream");

    while (pstVenc->bRun) {
        nfds = comm_epoll_wait(pstVenc->epfd, events, MAX_EPOLL_EVENT, 500);
        if (nfds == 0) {
            continue;
        } else {
            for (i = 0; i < nfds; i++) {
                if ((events[i].events & EPOLLIN) && events[i].data.fd) {
                    comm_mutex_lock(&pstVenc->mutex);
                    Vechn = vapi_venc_get_chn(events[i].data.fd);
                    if (Vechn < 0) {
                        VAPILOG("venc chn invalid");
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    if (pstVenc->stChn[Vechn].enState == STATE_IDLE) {
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }
                    memset(&stStream, 0, sizeof(stStream));
                    s32Ret = ss_mpi_venc_query_status(Vechn, &stStat);
                    if (TD_SUCCESS != s32Ret) {
                        VAPILOG("ss_mpi_venc_query_status chn[%d] failed with %#x!\n", i, s32Ret);
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    if (0 == stStat.cur_packs) {
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    stStream.pack = (ot_venc_pack *)ms_malloc(sizeof(ot_venc_pack) * stStat.cur_packs);
                    if (NULL == stStream.pack) {
                        VAPILOG("malloc stream pack failed!\n");
                        comm_mutex_unlock(&pstVenc->mutex);
                        break;
                    }

                    stStream.pack_cnt = stStat.cur_packs;
                    s32Ret            = ss_mpi_venc_get_stream(Vechn, &stStream, TD_TRUE);
                    if (TD_SUCCESS != s32Ret) {
                        ms_free(stStream.pack);
                        stStream.pack = NULL;
                        VAPILOG("ss_mpi_venc_get_stream failed with %#x!\n", s32Ret);
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    stFrame.stFormat.enType = pstVenc->stChn[Vechn].enType;
                    stFrame.stFormat.fps    = pstVenc->stChn[Vechn].fps;
                    stFrame.stFormat.width  = pstVenc->stChn[Vechn].width;
                    stFrame.stFormat.height = pstVenc->stChn[Vechn].height;
                    stFrame.pstPrivate      = pstVenc->stChn[Vechn].pstPrivate;
                    vapi_venc_alloc_frame(&stStream, &stFrame);

                    if (pstVenc->stCallback.venc_stream_cb)
                        pstVenc->stCallback.venc_stream_cb(Vechn, &stFrame);

                    vapi_venc_free_frame(&stFrame);

                    s32Ret = ss_mpi_venc_release_stream(Vechn, &stStream);
                    if (TD_SUCCESS != s32Ret) {
                        ms_free(stStream.pack);
                        stStream.pack = NULL;
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    ms_free(stStream.pack);
                    stStream.pack = NULL;
                    comm_mutex_unlock(&pstVenc->mutex);
                }
            }
        }
    }
    return TD_SUCCESS;
}

#else
static void *get_stream_task(void *argv)
{
    VENC_S            *pstVenc = (VENC_S *)argv;
    CHN_S             *pChn    = NULL;
    td_s32             i;
    td_s32             maxfd = 0;
    fd_set             read_fds;
    ot_venc_chn_status stStat;
    ot_venc_stream     stStream;
    VENC_FRAME_S       stFrame;
    td_s32             s32Ret;
    struct timeval     TimeoutVal;

    comm_task_set_name("Venc Get Stream");

    while (pstVenc->bRun) {
        comm_mutex_lock(&pstVenc->mutex);
        FD_ZERO(&read_fds);
        maxfd = 0;
        for (i = 0; i < MAX_VENC_NUM; i++) {
            pChn = &pstVenc->stChn[i];
            if (pChn->enState == STATE_BUSY) {
                FD_SET(pChn->fd, &read_fds);
                maxfd = VAPI_MAX(pChn->fd, maxfd);
            }
        }
        comm_mutex_unlock(&pstVenc->mutex);

        if (maxfd == 0) {
            usleep(100000);
            continue;
        }
        TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;
        s32Ret             = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0) {
            VAPILOG("select failed!\n");
            break;
        } else if (s32Ret == 0) {
            VAPILOG("get venc stream time out, exit thread\n");
            continue;
        } else {
            for (i = 0; i < MAX_VENC_NUM; i++) {
                pChn = &pstVenc->stChn[i];
                if (pChn->enState == STATE_BUSY) {
                    if (FD_ISSET(pChn->fd, &read_fds)) {
                        memset(&stStream, 0, sizeof(stStream));
                        s32Ret = HI_MPI_VENC_Query(i, &stStat);
                        if (TD_SUCCESS != s32Ret) {
                            VAPILOG("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                            continue;
                        }

                        if (0 == stStat.u32CurPacks) {
                            continue;
                        }

                        stStream.pstPack = (VENC_PACK_S *)ms_malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                        if (NULL == stStream.pstPack) {
                            VAPILOG("malloc stream pack failed!\n");
                            break;
                        }

                        stStream.u32PackCount = stStat.u32CurPacks;
                        s32Ret                = HI_MPI_VENC_GetStream(i, &stStream, TD_TRUE);
                        if (TD_SUCCESS != s32Ret) {
                            ms_free(stStream.pstPack);
                            stStream.pstPack = NULL;
                            VAPILOG("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                            continue;
                        }

                        stFrame.stFormat.enType = pChn->enType;
                        stFrame.stFormat.fps    = pChn->fps;
                        stFrame.stFormat.width  = pChn->width;
                        stFrame.stFormat.height = pChn->height;
                        stFrame.pstPrivate      = pChn->pstPrivate;
                        vapi_venc_alloc_frame(&stStream, &stFrame);

                        if (pstVenc->stCallback.venc_stream_cb)
                            pstVenc->stCallback.venc_stream_cb(i, &stFrame);

                        vapi_venc_free_frame(&stFrame);

                        s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                        if (TD_SUCCESS != s32Ret) {
                            ms_free(stStream.pstPack);
                            stStream.pstPack = NULL;
                            continue;
                        }

                        ms_free(stStream.pstPack);
                        stStream.pstPack = NULL;
                    }
                }
            }
        }
    }

    return TD_SUCCESS;
}
#endif

td_s32 vapi_venc_chn_create(TYPE_E enType, td_u32 u32Width, td_u32 u32Height, RC_E enRc, td_u32 u32BitRate, td_u32 u32SrcFrmRate, td_u32 u32DstFrmRate, void *pstPrivate)
{
    td_s32           s32Ret;
    ot_venc_chn      VeChn = -1;
    ot_payload_type  enPt;
    ot_venc_chn_attr stVencChnAttr = {0};

    enPt = vapi_venc_get_format(enType);

    stVencChnAttr.venc_attr.type = enPt;
    switch (enPt) {
    case OT_PT_H264: {
        stVencChnAttr.venc_attr.max_pic_width                  = u32Width;
        stVencChnAttr.venc_attr.max_pic_height                 = u32Height;
        stVencChnAttr.venc_attr.pic_width                      = u32Width;                 /*the picture width*/
        stVencChnAttr.venc_attr.pic_height                     = u32Height;                /*the picture height*/
        stVencChnAttr.venc_attr.buf_size                       = u32Width * u32Height * 2; /*stream buffer size*/
        stVencChnAttr.venc_attr.profile                        = 0;                        /*0: baseline; 1:MP; 2:HP   ? */
        stVencChnAttr.venc_attr.is_by_frame                    = TD_TRUE;                  /*get stream mode is slice mode or frame mode?*/
        stVencChnAttr.venc_attr.h264_attr.frame_buf_ratio      = 100;
        stVencChnAttr.venc_attr.h264_attr.rcn_ref_share_buf_en = TD_TRUE;

        if (ENC_RC_CBR == enRc) {
            stVencChnAttr.rc_attr.rc_mode                 = OT_VENC_RC_MODE_H264_CBR;
            stVencChnAttr.rc_attr.h264_cbr.gop            = u32DstFrmRate;
            stVencChnAttr.rc_attr.h264_cbr.stats_time     = 1;                 /* stream rate statics time(s) */
            stVencChnAttr.rc_attr.h264_cbr.src_frame_rate = u32SrcFrmRate;     /* input (vi) frame rate */
            stVencChnAttr.rc_attr.h264_cbr.dst_frame_rate = u32DstFrmRate;     /* target frame rate */
            stVencChnAttr.rc_attr.h264_cbr.bit_rate       = u32BitRate / 1024; /* average bit rate */
        } else if (ENC_RC_FIXQP == enRc) {
            stVencChnAttr.rc_attr.rc_mode                   = OT_VENC_RC_MODE_H264_FIXQP;
            stVencChnAttr.rc_attr.h264_fixqp.gop            = u32DstFrmRate;
            stVencChnAttr.rc_attr.h264_fixqp.src_frame_rate = u32SrcFrmRate;
            stVencChnAttr.rc_attr.h264_fixqp.dst_frame_rate = u32DstFrmRate;
            stVencChnAttr.rc_attr.h264_fixqp.i_qp           = 20;
            stVencChnAttr.rc_attr.h264_fixqp.p_qp           = 23;
        } else if (ENC_RC_VBR == enRc) {
            stVencChnAttr.rc_attr.rc_mode                 = OT_VENC_RC_MODE_H264_VBR;
            stVencChnAttr.rc_attr.h264_vbr.gop            = u32DstFrmRate;
            stVencChnAttr.rc_attr.h264_vbr.stats_time     = 1;
            stVencChnAttr.rc_attr.h264_vbr.src_frame_rate = u32SrcFrmRate;
            stVencChnAttr.rc_attr.h264_vbr.dst_frame_rate = u32DstFrmRate;
            stVencChnAttr.rc_attr.h264_vbr.max_bit_rate   = u32BitRate / 1024;
        } else {
            return TD_FAILURE;
        }
    } break;

    case OT_PT_MJPEG: {
        stVencChnAttr.venc_attr.max_pic_width  = u32Width;
        stVencChnAttr.venc_attr.max_pic_height = u32Height;
        stVencChnAttr.venc_attr.pic_width      = u32Width;
        stVencChnAttr.venc_attr.pic_height     = u32Height;
        stVencChnAttr.venc_attr.buf_size       = u32Width * u32Height * 2;
        stVencChnAttr.venc_attr.is_by_frame    = TD_TRUE; /*get stream mode is field mode  or frame mode*/

        if (ENC_RC_FIXQP == enRc) {
            stVencChnAttr.rc_attr.rc_mode                    = OT_VENC_RC_MODE_MJPEG_FIXQP;
            stVencChnAttr.rc_attr.mjpeg_fixqp.qfactor        = 90;
            stVencChnAttr.rc_attr.mjpeg_fixqp.src_frame_rate = u32SrcFrmRate;
            stVencChnAttr.rc_attr.mjpeg_fixqp.dst_frame_rate = u32DstFrmRate;
        } else if (ENC_RC_CBR == enRc) {
            stVencChnAttr.rc_attr.rc_mode                  = OT_VENC_RC_MODE_MJPEG_CBR;
            stVencChnAttr.rc_attr.mjpeg_cbr.stats_time     = 1;
            stVencChnAttr.rc_attr.mjpeg_cbr.src_frame_rate = u32SrcFrmRate;
            stVencChnAttr.rc_attr.mjpeg_cbr.dst_frame_rate = u32DstFrmRate;
            stVencChnAttr.rc_attr.mjpeg_cbr.bit_rate       = u32BitRate / 1024;

        } else if (ENC_RC_VBR == enRc) {
            stVencChnAttr.rc_attr.rc_mode                  = OT_VENC_RC_MODE_MJPEG_VBR;
            stVencChnAttr.rc_attr.mjpeg_vbr.stats_time     = 1;
            stVencChnAttr.rc_attr.mjpeg_vbr.src_frame_rate = u32SrcFrmRate;
            stVencChnAttr.rc_attr.mjpeg_vbr.dst_frame_rate = u32DstFrmRate;
            stVencChnAttr.rc_attr.mjpeg_vbr.max_bit_rate   = u32BitRate / 1024;
        } else {
            VAPILOG("cann't support other mode in this version!\n");
            return TD_FAILURE;
        }
    } break;

    case OT_PT_JPEG: {
        stVencChnAttr.venc_attr.max_pic_width  = u32Width;
        stVencChnAttr.venc_attr.max_pic_height = u32Height;
        stVencChnAttr.venc_attr.pic_width      = u32Width;
        stVencChnAttr.venc_attr.pic_height     = u32Height;
        stVencChnAttr.venc_attr.buf_size       = u32Width * u32Height * 2;
        stVencChnAttr.venc_attr.is_by_frame    = TD_TRUE; /*get stream mode is field mode  or frame mode*/
    } break;
    default:
        return OT_ERR_VENC_NOT_SUPPORT;
    }

    comm_mutex_lock(&g_stVenc.mutex);

    VeChn = vapi_venc_idel_chn();

    s32Ret = ss_mpi_venc_create_chn(VeChn, &stVencChnAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_create_chn [%d] faild with %#x!\n", VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return s32Ret;
    }

    ot_venc_start_param start_param;
    start_param.recv_pic_num = -1;
    s32Ret                   = ss_mpi_venc_start_chn(VeChn, &start_param);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_start_chn faild with%#x!\n", s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return TD_FAILURE;
    }

    g_stVenc.stChn[VeChn].enState    = STATE_BUSY;
    g_stVenc.stChn[VeChn].enType     = enType;
    g_stVenc.stChn[VeChn].width      = u32Width;
    g_stVenc.stChn[VeChn].height     = u32Height;
    g_stVenc.stChn[VeChn].bitRate    = u32BitRate;
    g_stVenc.stChn[VeChn].fps        = u32DstFrmRate;
    g_stVenc.stChn[VeChn].pstPrivate = pstPrivate;
    g_stVenc.count++;
    g_stVenc.stChn[VeChn].fd = ss_mpi_venc_get_fd(VeChn);
#ifdef MAX_EPOLL_EVENT
    comm_epoll_add(g_stVenc.epfd, g_stVenc.stChn[VeChn].fd, EPOLLIN);
#endif
    comm_mutex_unlock(&g_stVenc.mutex);

    return VeChn;
}

td_s32 vapi_venc_chn_destory(ot_venc_chn VeChn)
{
    td_s32 s32Ret;

    if (VeChn < 0)
        return TD_FAILURE;

    if (g_stVenc.stChn[VeChn].enState != STATE_BUSY)
        return TD_SUCCESS;

    comm_mutex_lock(&g_stVenc.mutex);

    s32Ret = ss_mpi_venc_stop_chn(VeChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_stop_chn vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_venc_close_fd(VeChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_close_fd vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_venc_destroy_chn(VeChn);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_destroy_chn vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return TD_FAILURE;
    }

    g_stVenc.stChn[VeChn].enState = STATE_IDLE;
    g_stVenc.count--;
#ifdef MAX_EPOLL_EVENT
    comm_epoll_del(g_stVenc.epfd, g_stVenc.stChn[VeChn].fd, EPOLLIN);
#endif
    comm_mutex_unlock(&g_stVenc.mutex);

    return TD_SUCCESS;
}

td_s32 vapi_venc_chn_config(ot_venc_chn VeChn, td_u32 u32DstFrmRate, td_u32 u32BitRate)
{
    ot_venc_chn_attr stAttr;
    td_s32           s32Ret;

    memset(&stAttr, 0, sizeof(ot_venc_chn_attr));
    s32Ret = ss_mpi_venc_get_chn_attr(VeChn, &stAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_get_chn_attr vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        return TD_FAILURE;
    }

    if (stAttr.venc_attr.type != OT_PT_H264)
        return TD_FAILURE;

    if (OT_VENC_RC_MODE_H264_CBR == stAttr.rc_attr.rc_mode) {
        stAttr.rc_attr.h264_cbr.dst_frame_rate = u32DstFrmRate;     /* target frame rate */
        stAttr.rc_attr.h264_cbr.bit_rate       = u32BitRate / 1024; /* average bit rate */
    } else if (OT_VENC_RC_MODE_H264_FIXQP == stAttr.rc_attr.rc_mode) {
        stAttr.rc_attr.h264_fixqp.dst_frame_rate = u32DstFrmRate;
    } else if (OT_VENC_RC_MODE_H264_VBR == stAttr.rc_attr.rc_mode) {
        stAttr.rc_attr.h264_vbr.dst_frame_rate = u32DstFrmRate;
        stAttr.rc_attr.h264_vbr.max_bit_rate   = u32BitRate / 1024;
    } else {
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_venc_set_chn_attr(VeChn, &stAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_venc_set_chn_attr vechn[%d] failed with %#x!\n", VeChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_bool vapi_venc_chn_is_remain()
{
    td_bool bRemain;

    comm_mutex_lock(&g_stVenc.mutex);
    bRemain = (MAX_VENC_NUM > g_stVenc.count);
    comm_mutex_unlock(&g_stVenc.mutex);

    return bRemain;
}

td_s32 vapi_venc_bind_vdec(ot_vdec_chn VdChn, ot_venc_chn VeChn)
{
    td_s32     s32Ret = TD_SUCCESS;
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
    g_stVenc.stChn[VeChn].stBind.vdecChn = VdChn;
    return TD_SUCCESS;
}

td_s32 vapi_venc_unbind_vdec(ot_venc_chn VeChn)
{
    if (VeChn < 0)
        return TD_FAILURE;

    td_s32      s32Ret = TD_SUCCESS;
    ot_mpp_chn  stSrcChn;
    ot_mpp_chn  stDestChn;
    ot_vdec_chn VdChn = g_stVenc.stChn[VeChn].stBind.vdecChn;

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

    return VdChn;
}

td_void vapi_venc_init(VDEC_CB_S *pstCb)
{
    td_s32 i;
    memset(&g_stVenc, 0, sizeof(VENC_S));

    for (i = 0; i < MAX_VENC_NUM; i++) {
        g_stVenc.stChn[i].enState = STATE_IDLE;
    }
#ifdef MAX_EPOLL_EVENT
    g_stVenc.epfd = comm_epoll_create(MAX_EPOLL_EVENT);
#endif
    g_stVenc.stCallback.venc_stream_cb = pstCb->venc_stream_cb;
    comm_mutex_init(&g_stVenc.mutex);

    g_stVenc.bRun = TD_TRUE;
    comm_task_create_join(&g_stVenc.pthread, get_stream_task, &g_stVenc);
}

td_void vapi_venc_uninit()
{
    g_stVenc.bRun = TD_FALSE;
    comm_task_join(&g_stVenc.pthread);
    comm_mutex_uninit(&g_stVenc.mutex);

#ifdef MAX_EPOLL_EVENT
    comm_epoll_destroy(g_stVenc.epfd);
#endif
}
