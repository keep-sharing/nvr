/* 
 * ***************************************************************
 * Filename:      	vapi_venc.c
 * Created at:    	2018.10.16
 * Description:   	video encode controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "vapi_comm.h"

#define MAX_EPOLL_EVENT  MAX_VENC_NUM+1

typedef struct chn_s{
    HI_S32      fd;
    STATE_E     enState;
    TYPE_E      enType;
    BIND_S      stBind;
    HI_U32      fps;
    HI_U32      width;
    HI_U32      height;
    HI_U32      bitRate;
    void        *pstPrivate;
}CHN_S;

typedef struct venc_s{
    CHN_S           stChn[MAX_VENC_NUM];
    HI_S32          count;
    HI_S32          epfd;
	pthread_mutex_t mutex;
    pthread_t       pthread;
    HI_BOOL         bRun;
    VDEC_CB_S       stCallback;
}VENC_S;

static VENC_S g_stVenc;

static PAYLOAD_TYPE_E vapi_venc_get_format(TYPE_E enType)
{   
    PAYLOAD_TYPE_E enPT;

    switch(enType)
    {
        case ENC_TYPE_H264:
            enPT = PT_H264;
            break;
#if 0
        case ENC_TYPE_H265:
            enPT = PT_H265;
            break;
        case ENC_TYPE_MJPEG:
            enPT = PT_MJPEG;
            break;
#endif
        default :            
            VAPILOG("can not support type[%d], use default[H264]", enType);
            enPT = PT_H264;
            break;        
    }

    return enPT;
}

static void vapi_venc_alloc_frame(VENC_STREAM_S *pStream, VENC_FRAME_S *pFrame)
{
    HI_S32 i;
    HI_U32 pack_size;

    pFrame->size = 0;    
    for(i = 0; i < pStream->u32PackCount; i++)
    {
        pFrame->size += pStream->pstPack[i].u32Len - pStream->pstPack[i].u32Offset;
    }

    pFrame->data = ms_malloc(pFrame->size);
    pack_size = 0;
    for(i = 0; i < pStream->u32PackCount; i++)
    {
        memcpy(pFrame->data+pack_size, pStream->pstPack[i].pu8Addr+pStream->pstPack[i].u32Offset,\
               pStream->pstPack[i].u32Len-pStream->pstPack[i].u32Offset);
        pack_size += pStream->pstPack[i].u32Len-pStream->pstPack[i].u32Offset;
    }
    
    pFrame->isIframe = pStream->pstPack[0].DataType.enH264EType == H264E_NALU_SPS ? 1 : 0;
    pFrame->pts      = pStream->pstPack[0].u64PTS;
}

static void vapi_venc_free_frame(VENC_FRAME_S *pFrame)
{
    if (pFrame->data)
        ms_free(pFrame->data);
}

static inline VENC_CHN vapi_venc_get_chn(HI_S32 fd)
{
    HI_S32 i;

    for (i=0; i<MAX_CHN_NUM; i++)
    {
        if (g_stVenc.stChn[i].fd == fd)
            return i;
    }

    return -1;
}
static VENC_CHN vapi_venc_idel_chn()
{   
    HI_S32 i;
    VENC_CHN Vechn = -1;

    for (i=0; i<MAX_CHN_NUM; i++)
    {
        if (g_stVenc.stChn[i].enState == STATE_IDLE)
        {
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
    VENC_S *pstVenc = (VENC_S *)argv;
    HI_S32 i;
    HI_S32 nfds = 0;
    VENC_CHN Vechn = -1;
    struct epoll_event events[MAX_EPOLL_EVENT];
    
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    VENC_FRAME_S stFrame;
    HI_S32 s32Ret;
    
    comm_task_set_name("Venc Get Stream");
    
    while(pstVenc->bRun)
    {
        nfds = comm_epoll_wait(pstVenc->epfd, events, MAX_EPOLL_EVENT, 500);
        if (nfds == 0)
        {
            continue;
        }
        else
        {
            for (i = 0; i < nfds; i++)
            {
                if((events[i].events & EPOLLIN ) && events[i].data.fd)
                {
                    comm_mutex_lock(&pstVenc->mutex);
                    Vechn = vapi_venc_get_chn(events[i].data.fd);
                    if (Vechn < 0)
                    {
                        VAPILOG("venc chn invalid");
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    if (pstVenc->stChn[Vechn].enState == STATE_IDLE)
                    {
                         comm_mutex_unlock(&pstVenc->mutex);
                         continue;
                    }
                    memset(&stStream, 0, sizeof(stStream));
                    s32Ret = HI_MPI_VENC_Query(Vechn, &stStat);
                    if (HI_SUCCESS != s32Ret)
                    {
                        VAPILOG("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    if(0 == stStat.u32CurPacks)
                    {
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    stStream.pstPack = (VENC_PACK_S*)ms_malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack)
                    {
                        VAPILOG("malloc stream pack failed!\n");
                        comm_mutex_unlock(&pstVenc->mutex);
                        break;
                    }

                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(Vechn, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret)
                    {
                        ms_free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        VAPILOG("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
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

                    s32Ret = HI_MPI_VENC_ReleaseStream(Vechn, &stStream);
                    if (HI_SUCCESS != s32Ret)
                    {
                        ms_free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        comm_mutex_unlock(&pstVenc->mutex);
                        continue;
                    }

                    ms_free(stStream.pstPack);
                    stStream.pstPack = NULL;
                    comm_mutex_unlock(&pstVenc->mutex);
                }
            }
        }
    }
    return HI_SUCCESS;
}

#else
static void *get_stream_task(void *argv)
{
    VENC_S *pstVenc = (VENC_S *)argv;
    CHN_S *pChn = NULL;
    HI_S32 i;
    HI_S32 maxfd = 0;
    fd_set read_fds;
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    VENC_FRAME_S stFrame;
    HI_S32 s32Ret;
    struct timeval TimeoutVal;
    
    comm_task_set_name("Venc Get Stream");

    while(pstVenc->bRun)
    {
        comm_mutex_lock(&pstVenc->mutex);
        FD_ZERO(&read_fds);
        maxfd = 0;
        for (i = 0; i < MAX_VENC_NUM; i++)
        {
            pChn = &pstVenc->stChn[i];
            if (pChn->enState == STATE_BUSY)
            {
                FD_SET(pChn->fd, &read_fds);
                maxfd = VAPI_MAX(pChn->fd, maxfd);
            }
        }
        comm_mutex_unlock(&pstVenc->mutex);

        if (maxfd == 0)
        {
            usleep(100000);
            continue;
        }
        TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)
        {
            VAPILOG("select failed!\n");
            break;
        }
        else if (s32Ret == 0)
        {
            VAPILOG("get venc stream time out, exit thread\n");
            continue;
        }
        else
        {
            for (i = 0; i < MAX_VENC_NUM; i++)
            {
                pChn = &pstVenc->stChn[i];
                if (pChn->enState == STATE_BUSY)
                {
                    if (FD_ISSET(pChn->fd, &read_fds))
                    {
                        memset(&stStream, 0, sizeof(stStream));
                        s32Ret = HI_MPI_VENC_Query(i, &stStat);
                        if (HI_SUCCESS != s32Ret)
                        {
                            VAPILOG("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                            continue;
                        }

    					if(0 == stStat.u32CurPacks)
    					{
    						continue;
    					}

                        stStream.pstPack = (VENC_PACK_S*)ms_malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                        if (NULL == stStream.pstPack)
                        {
                            VAPILOG("malloc stream pack failed!\n");
                            break;
                        }

                        stStream.u32PackCount = stStat.u32CurPacks;
                        s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                        if (HI_SUCCESS != s32Ret)
                        {
                            ms_free(stStream.pstPack);
                            stStream.pstPack = NULL;
                            VAPILOG("HI_MPI_VENC_GetStream failed with %#x!\n", \
                                   s32Ret);
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
                        if (HI_SUCCESS != s32Ret)
                        {
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

    return HI_SUCCESS;
}
#endif

HI_S32 vapi_venc_chn_create(TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height,
                                RC_E enRc, HI_U32 u32BitRate, 
                                HI_U32 u32SrcFrmRate, HI_U32 u32DstFrmRate,
                                void *pstPrivate)
{
    HI_S32 s32Ret;
    VENC_CHN VeChn = -1;
    PAYLOAD_TYPE_E enPt;
    VENC_CHN_ATTR_S stVencChnAttr;

    enPt = vapi_venc_get_format(enType);
    stVencChnAttr.stVeAttr.enType = enPt;
    switch(enPt)
    {
        case PT_H264:
        {
            VENC_ATTR_H264_S stH264Attr;
            VENC_ATTR_H264_CBR_S    stH264Cbr;
            VENC_ATTR_H264_VBR_S    stH264Vbr;
            VENC_ATTR_H264_FIXQP_S  stH264FixQp;
            
            stH264Attr.u32MaxPicWidth = u32Width;
            stH264Attr.u32MaxPicHeight = u32Height;
            stH264Attr.u32PicWidth = u32Width;/*the picture width*/
            stH264Attr.u32PicHeight = u32Height;/*the picture height*/
            stH264Attr.u32BufSize  = u32Width * u32Height * 2;/*stream buffer size*/
            stH264Attr.u32Profile  = 0;/*0: baseline; 1:MP; 2:HP   ? */
            stH264Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

            if(ENC_RC_CBR == enRc)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stH264Cbr.u32Gop            = u32DstFrmRate;
                stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
                stH264Cbr.u32SrcFrmRate     = u32SrcFrmRate;/* input (vi) frame rate */
                stH264Cbr.fr32DstFrmRate    = u32DstFrmRate;/* target frame rate */
                stH264Cbr.u32BitRate        = u32BitRate/1024;/* average bit rate */
                stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
            }
            else if (ENC_RC_FIXQP == enRc) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = u32DstFrmRate;
                stH264FixQp.u32SrcFrmRate = u32SrcFrmRate;
                stH264FixQp.fr32DstFrmRate = u32DstFrmRate;
                stH264FixQp.u32IQp = 20;
                stH264FixQp.u32PQp = 23;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
            }
            else if (ENC_RC_VBR == enRc) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stH264Vbr.u32Gop = u32DstFrmRate;
                stH264Vbr.u32StatTime = 1;
                stH264Vbr.u32SrcFrmRate = u32SrcFrmRate;
                stH264Vbr.fr32DstFrmRate = u32DstFrmRate;
                stH264Vbr.u32MinQp = 10;
                stH264Vbr.u32MaxQp = 40;
                stH264Vbr.u32MaxBitRate = u32BitRate/1024;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        }
        break;
        
        case PT_MJPEG:
        {
            VENC_ATTR_MJPEG_S stMjpegAttr;
            VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
            
            stMjpegAttr.u32MaxPicWidth = u32Width;
            stMjpegAttr.u32MaxPicHeight = u32Height;
            stMjpegAttr.u32PicWidth = u32Width;
            stMjpegAttr.u32PicHeight = u32Height;
            stMjpegAttr.u32BufSize = u32Width * u32Height * 2;
            stMjpegAttr.bByFrame = HI_TRUE;  /*get stream mode is field mode  or frame mode*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

            if(ENC_RC_FIXQP == enRc)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegeFixQp.u32Qfactor        = 90;
                stMjpegeFixQp.u32SrcFrmRate      = u32SrcFrmRate;
                stMjpegeFixQp.fr32DstFrmRate = u32DstFrmRate;
                memcpy(&stVencChnAttr.stRcAttr.stAttrMjpegeFixQp, &stMjpegeFixQp,
                       sizeof(VENC_ATTR_MJPEG_FIXQP_S));
            }
            else if (ENC_RC_CBR == enRc)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32StatTime = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32SrcFrmRate = u32SrcFrmRate;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.fr32DstFrmRate = u32DstFrmRate;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = u32BitRate/1024;
                
            }
            else if (ENC_RC_VBR == enRc) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32StatTime = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32SrcFrmRate = u32SrcFrmRate;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.fr32DstFrmRate = u32DstFrmRate;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MinQfactor = 50;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxQfactor = 95;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = u32BitRate/1024;
            }
            else 
            {
                VAPILOG("cann't support other mode in this version!\n");
                return HI_FAILURE;
            }
        }
        break;
            
        case PT_JPEG:
        {
            VENC_ATTR_JPEG_S stJpegAttr;
                        
            stJpegAttr.u32MaxPicWidth = u32Width;
            stJpegAttr.u32MaxPicHeight = u32Height;
            stJpegAttr.u32PicWidth  = u32Width;
            stJpegAttr.u32PicHeight = u32Height;
            stJpegAttr.u32BufSize = stJpegAttr.u32MaxPicWidth * stJpegAttr.u32MaxPicHeight * 2;
            stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(stJpegAttr));
        }
        break;
        default:
            return HI_ERR_VENC_NOT_SUPPORT;
    }

    comm_mutex_lock(&g_stVenc.mutex);
    
    VeChn = vapi_venc_idel_chn();

    s32Ret = HI_MPI_VENC_CreateChn(VeChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_StartRecvPic(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return HI_FAILURE;
    }
    
    g_stVenc.stChn[VeChn].enState = STATE_BUSY;
    g_stVenc.stChn[VeChn].enType  = enType;
    g_stVenc.stChn[VeChn].width   = u32Width;
    g_stVenc.stChn[VeChn].height  = u32Height;
    g_stVenc.stChn[VeChn].bitRate = u32BitRate;
    g_stVenc.stChn[VeChn].fps     = u32DstFrmRate;
    g_stVenc.stChn[VeChn].pstPrivate = pstPrivate;
    g_stVenc.count++;
    g_stVenc.stChn[VeChn].fd = HI_MPI_VENC_GetFd(VeChn);
#ifdef MAX_EPOLL_EVENT
    comm_epoll_add(g_stVenc.epfd, g_stVenc.stChn[VeChn].fd, EPOLLIN);
#endif
    comm_mutex_unlock(&g_stVenc.mutex);

    return VeChn;

}

HI_S32 vapi_venc_chn_destory(VENC_CHN VeChn)
{
    HI_S32 s32Ret;

    if (VeChn < 0)
        return HI_FAILURE;
        
    if (g_stVenc.stChn[VeChn].enState != STATE_BUSY)
        return HI_SUCCESS;
        
    comm_mutex_lock(&g_stVenc.mutex);

    s32Ret = HI_MPI_VENC_StopRecvPic(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_CloseFd(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_CloseFd vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VENC_DestroyChn(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        comm_mutex_unlock(&g_stVenc.mutex);
        return HI_FAILURE;
    }

    g_stVenc.stChn[VeChn].enState = STATE_IDLE;
    g_stVenc.count--;
#ifdef MAX_EPOLL_EVENT
    comm_epoll_del(g_stVenc.epfd, g_stVenc.stChn[VeChn].fd, EPOLLIN);
#endif
    comm_mutex_unlock(&g_stVenc.mutex);

    return HI_SUCCESS;
}

HI_S32 vapi_venc_chn_config(VENC_CHN VeChn, HI_U32 u32DstFrmRate, HI_U32 u32BitRate)
{
    VENC_CHN_ATTR_S stAttr;
    HI_S32 s32Ret;

    memset(&stAttr, 0, sizeof(VENC_CHN_ATTR_S));
    s32Ret = HI_MPI_VENC_GetChnAttr(VeChn, &stAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_GetChnAttr vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    if (stAttr.stVeAttr.enType != PT_H264)
        return HI_FAILURE;

    if(VENC_RC_MODE_H264CBR == stAttr.stRcAttr.enRcMode)
    {
        stAttr.stRcAttr.stAttrH264Cbr.fr32DstFrmRate    = u32DstFrmRate;/* target frame rate */
        stAttr.stRcAttr.stAttrH264Cbr.u32BitRate        = u32BitRate/1024;/* average bit rate */
    }
    else if (VENC_RC_MODE_H264FIXQP == stAttr.stRcAttr.enRcMode) 
    {
        stAttr.stRcAttr.stAttrH264FixQp.fr32DstFrmRate = u32DstFrmRate;
    }
    else if (VENC_RC_MODE_H264VBR == stAttr.stRcAttr.enRcMode) 
    {
        stAttr.stRcAttr.stAttrH264Vbr.fr32DstFrmRate = u32DstFrmRate;
        stAttr.stRcAttr.stAttrH264Vbr.u32MaxBitRate = u32BitRate/1024;
    }
    else
    {
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_SetChnAttr(VeChn, &stAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_SetChnAttr vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_BOOL vapi_venc_chn_is_remain()
{
    HI_BOOL bRemain;
    
    comm_mutex_lock(&g_stVenc.mutex);
    bRemain = (MAX_VENC_NUM > g_stVenc.count);
    comm_mutex_unlock(&g_stVenc.mutex);

    return bRemain;
}

HI_S32 vapi_venc_bind_vdec(VDEC_CHN VdChn, VENC_CHN VeChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_SYS_Bind [%d->%d] failed with %#x!\n", VdChn, VeChn, s32Ret);
        return HI_FAILURE;
    }
    g_stVenc.stChn[VeChn].stBind.vdecChn = VdChn;
    return HI_SUCCESS;
}

HI_S32 vapi_venc_unbind_vdec(VENC_CHN VeChn)
{
    if (VeChn < 0)
        return HI_FAILURE;
        
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    VDEC_CHN VdChn = g_stVenc.stChn[VeChn].stBind.vdecChn;

    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = VeChn;
    
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_SYS_UnBind [%d->%d] failed with %#x!\n", VdChn, VeChn, s32Ret);
        return HI_FAILURE;
    }
        
    return VdChn;
}

HI_VOID vapi_venc_init(VDEC_CB_S *pstCb)
{
    HI_S32 i;
    memset(&g_stVenc, 0, sizeof(VENC_S));

    for(i=0; i<MAX_CHN_NUM; i++)
    {
        g_stVenc.stChn[i].enState = STATE_IDLE;
    }
#ifdef MAX_EPOLL_EVENT
    g_stVenc.epfd = comm_epoll_create(MAX_EPOLL_EVENT);
#endif
    g_stVenc.stCallback.venc_stream_cb = pstCb->venc_stream_cb;
    comm_mutex_init(&g_stVenc.mutex);

    g_stVenc.bRun = HI_TRUE;
    comm_task_create_join(&g_stVenc.pthread, get_stream_task, &g_stVenc);
}

HI_VOID vapi_venc_uninit()
{
    g_stVenc.bRun = HI_FALSE;
    comm_task_join(&g_stVenc.pthread);
    comm_mutex_uninit(&g_stVenc.mutex);

#ifdef MAX_EPOLL_EVENT
    comm_epoll_destroy(g_stVenc.epfd);
#endif
}


