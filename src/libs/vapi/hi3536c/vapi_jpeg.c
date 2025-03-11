/* 
 * ***************************************************************
 * Filename:      	vapi_jpeg.c
 * Created at:    	2019.08.21
 * Description:   	video jpeg encoder
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

#define EPOLL_EVENT   1

typedef struct jpeg_s{
    HI_S32          epfd;
    HI_S32          vencfd;
	pthread_mutex_t mutex;
	pthread_mutex_t mutex_ex;
}JPEG_S;

static JPEG_S g_stJpeg;

static PAYLOAD_TYPE_E vdec_get_format(TYPE_E enType)
{   
    PAYLOAD_TYPE_E enPT;

    switch(enType)
    {
        case ENC_TYPE_H264:
            enPT = PT_H264;
            break;
        case ENC_TYPE_H265:
            enPT = PT_H265;
            break;
        case ENC_TYPE_MJPEG:
            enPT = PT_MJPEG;
            break;
        default :            
            VAPILOG("can not support type[%d], use default[H264]", enType);
            enPT = PT_H264;
            break;        
    }

    return enPT;
}

static HI_S32 yuv420sp_to_jpeg(VIDEO_FRAME_INFO_S * pstFrame, HI_U8 **ppData, HI_U32 *pSize, HI_U32 *pWidth, HI_U32 *pHeight)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_JPEG_S stJpegAttr;
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_U8 *data = NULL;
    HI_U32 size = 0;
    HI_S32 i = 0;
    HI_S32 retry = 10;
    HI_U32 pack_size;
        
    HI_U32 u32Width  = VAPI_MIN(pstFrame->stVFrame.u32Width, VAPI_VENC_MAX_WIDTH);
    HI_U32 u32Height = VAPI_MIN(pstFrame->stVFrame.u32Height, VAPI_VENC_MAX_HEIGTH);
    
    stVencChnAttr.stVeAttr.enType = PT_JPEG;
    stJpegAttr.u32MaxPicWidth = u32Width;
    stJpegAttr.u32MaxPicHeight = u32Height;
    stJpegAttr.u32PicWidth  = u32Width;
    stJpegAttr.u32PicHeight = u32Height;
    stJpegAttr.u32BufSize = stJpegAttr.u32MaxPicWidth * stJpegAttr.u32MaxPicHeight;
    stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
    stJpegAttr.bSupportDCF = HI_FALSE;
    memcpy(&stVencChnAttr.stVeAttr.stAttrJpege, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));

    s32Ret = HI_MPI_VENC_CreateChn(VAPI_SNAP_VENC_ID, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_CreateChn faild with%#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_StartRecvPic(VAPI_SNAP_VENC_ID);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        goto OUT;
    }

    s32Ret = HI_MPI_VENC_SendFrame(VAPI_SNAP_VENC_ID, pstFrame, -1);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_SendFrame faild with%#x!\n", s32Ret);
        goto OUT;
    }

    memset(&stStream, 0, sizeof(stStream));
    while(retry--)
    {
        s32Ret = HI_MPI_VENC_Query(VAPI_SNAP_VENC_ID, &stStat);
        if (HI_SUCCESS != s32Ret)
        {
            VAPILOG("HI_MPI_VENC_Query failed with %#x!\n", s32Ret);
            s32Ret = HI_FAILURE;
            goto OUT;
        }
        if(0 == stStat.u32CurPacks)
        {
            usleep(20000);
            s32Ret = HI_FAILURE;
            continue;
        }
        s32Ret = HI_SUCCESS;
        break;
    }

    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_Query no packs !\n");
        goto OUT;
    }
    
    stStream.pstPack = (VENC_PACK_S*)ms_malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
    if (NULL == stStream.pstPack)
    {
        VAPILOG("malloc stream pack failed!\n");
        s32Ret = HI_FAILURE;
        goto OUT;
    }

    stStream.u32PackCount = stStat.u32CurPacks;
    s32Ret = HI_MPI_VENC_GetStream(VAPI_SNAP_VENC_ID, &stStream, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        ms_free(stStream.pstPack);
        stStream.pstPack = NULL;
        VAPILOG("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
        goto OUT;
    }
    size = 0;
    for(i = 0; i < stStream.u32PackCount; i++)
        size += stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;

    data = ms_malloc(size);
    if (data == NULL)
    {
        HI_MPI_VENC_ReleaseStream(VAPI_SNAP_VENC_ID, &stStream);
        ms_free(stStream.pstPack);
        s32Ret = HI_FAILURE;
        goto OUT;
    }                
    pack_size = 0;
    for(i = 0; i < stStream.u32PackCount; i++)
    {
        memcpy(data + pack_size, stStream.pstPack[i].pu8Addr+stStream.pstPack[i].u32Offset,\
               stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
        pack_size += stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;
    }
    s32Ret = HI_MPI_VENC_ReleaseStream(VAPI_SNAP_VENC_ID, &stStream);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_ReleaseStream failed with %#x!\n", s32Ret);
        ms_free(data);
    }

    ms_free(stStream.pstPack);
OUT:
    HI_MPI_VENC_StopRecvPic(VAPI_SNAP_VENC_ID);
    HI_MPI_VENC_DestroyChn(VAPI_SNAP_VENC_ID);
    *ppData = data;
    *pSize = size;
    *pWidth = u32Width;
    *pHeight = u32Height;
    
    return s32Ret;

}

static HI_S32 vdec_chn_create(VDEC_CHN VdChn, TYPE_E enType, 
                            HI_U32 u32Width, HI_U32 u32Height)
{
    VDEC_CHN_ATTR_S stVdecChnAttr;
    PAYLOAD_TYPE_E enPT;
    HI_S32 buffsize;
    HI_S32 s32Ret;

    if (u32Width > VAPI_VDEC_MAX_WIDTH || u32Height > VAPI_VDEC_MAX_HEIGTH)
    {
        VAPILOG("resolution must be <= %dx%d", VAPI_VDEC_MAX_WIDTH, VAPI_VDEC_MAX_HEIGTH);
        return HI_FAILURE;
    }
    enPT = vdec_get_format(enType);
    VB_PIC_BLK_SIZE(u32Width, u32Height, enPT, buffsize);
    
    stVdecChnAttr.enType       = enPT;
    stVdecChnAttr.u32BufSize   = buffsize;
    stVdecChnAttr.u32Priority  = 10;
    stVdecChnAttr.u32PicWidth  = ALIGN_UP(u32Width, 16);
    stVdecChnAttr.u32PicHeight = ALIGN_UP(u32Height, 16);
    if (PT_H264 == enPT || PT_MP4VIDEO == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 2;
        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
    }
    else if (PT_JPEG == enPT || PT_MJPEG == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
        stVdecChnAttr.stVdecJpegAttr.enJpegFormat = JPG_COLOR_FMT_YCBCR420;
    }
    else if (PT_H265 == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 2;
        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 1;
    } 
    
    s32Ret = HI_MPI_VDEC_SetChnVBCnt(VdChn, 1);//debug notyet    
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_SetChnVBCnt[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VDEC_CreateChn(VdChn, &stVdecChnAttr);    
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_CreateChn[%d] failed with %#x!\n", VdChn, s32Ret);
        VAPILOG("error: width = %d. height = %d encType = %d \n", u32Width, u32Height, enPT);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VDEC_SetDisplayMode(VdChn, VIDEO_DISPLAY_MODE_PREVIEW);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_SetDisplayMode[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_MPI_VDEC_DestroyChn(VdChn);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VDEC_StartRecvStream(VdChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_StartRecvStream[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_MPI_VDEC_DestroyChn(VdChn);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

static HI_S32 vdec_chn_destory(VDEC_CHN VdChn)
{
    HI_S32 s32Ret;
    
    if (VdChn < 0)
        return HI_FAILURE;

    s32Ret = HI_MPI_VDEC_StopRecvStream(VdChn);    
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_StopRecvStream[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VDEC_DestroyChn(VdChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_DestroyChn[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    } 
    
    return HI_SUCCESS;
}

static HI_S32 venc_chn_create(VENC_CHN VeChn, HI_U32 u32Qfactor, HI_U32 u32Width, HI_U32 u32Height, HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_JPEG_S stJpegAttr;
    VENC_PARAM_JPEG_S stJpegParam;
    
    u32Width  = ALIGN_BACK(VAPI_MIN(u32Width, VAPI_VENC_MAX_WIDTH), 4);
    u32Height = ALIGN_BACK(VAPI_MIN(u32Height, VAPI_VENC_MAX_HEIGTH), 4);
	*pu32OutWidth = u32Width;
	*pu32OutHeight = u32Height;
    
    stVencChnAttr.stVeAttr.enType = PT_JPEG;
    stJpegAttr.u32MaxPicWidth = u32Width;
    stJpegAttr.u32MaxPicHeight = u32Height;
    stJpegAttr.u32PicWidth  = u32Width;
    stJpegAttr.u32PicHeight = u32Height;
    stJpegAttr.u32BufSize = stJpegAttr.u32MaxPicWidth * stJpegAttr.u32MaxPicHeight;
    stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
    stJpegAttr.bSupportDCF = HI_FALSE;
    memcpy(&stVencChnAttr.stVeAttr.stAttrJpege, &stJpegAttr, sizeof(stJpegAttr));

    s32Ret = HI_MPI_VENC_CreateChn(VeChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VeChn, s32Ret);
        VAPILOG("u32Qfactor[%d] u32Width[%d] u32Height[%d]\n", u32Qfactor, u32Width, u32Height);
        return s32Ret;
    }

    s32Ret = HI_MPI_VENC_GetJpegParam(VeChn, &stJpegParam);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_GetJpegParam [%d] faild with %#x!\n",\
                VeChn, s32Ret);
        return s32Ret;
    }
    stJpegParam.u32Qfactor = u32Qfactor;
    s32Ret = HI_MPI_VENC_SetJpegParam(VeChn, &stJpegParam);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_SetJpegParam [%d] faild with %#x!\n",\
                VeChn, s32Ret);
        return s32Ret;
    }
    
    s32Ret = HI_MPI_VENC_StartRecvPic(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    g_stJpeg.vencfd = HI_MPI_VENC_GetFd(VeChn);
    comm_epoll_add(g_stJpeg.epfd, g_stJpeg.vencfd , EPOLLIN);
    
    return HI_SUCCESS;

}

static HI_S32 venc_chn_destory(VENC_CHN VeChn)
{
    HI_S32 s32Ret;

    if (VeChn < 0)
        return HI_FAILURE;
        
    s32Ret = HI_MPI_VENC_StopRecvPic(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VENC_CloseFd(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_CloseFd vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VENC_DestroyChn(VeChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VeChn, s32Ret);
        return HI_FAILURE;
    }
    
    comm_epoll_del(g_stJpeg.epfd, g_stJpeg.vencfd, EPOLLIN);

    return HI_SUCCESS;
}

static HI_S32 vdec_bind_venc(VDEC_CHN VdChn, VENC_CHN VeChn)
{
    HI_S32 s32Ret;
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
        
    return HI_SUCCESS;
}

static HI_S32 vdec_unbind_venc(VDEC_CHN VdChn, VENC_CHN VeChn)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    
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
        
    return HI_SUCCESS;
}

static HI_S32 jpeg_create(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight,
                            HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight,HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight)
{
    HI_S32 s32Ret;

    s32Ret = vdec_chn_create(VAPI_JPEG_VDEC_ID, enType, u32InWidth, u32InHeight);
    if (s32Ret != HI_SUCCESS) {
        return s32Ret;
    }

    u32OutWidth  = VAPI_MIN(u32InWidth, u32OutWidth);
    u32OutHeight = VAPI_MIN(u32InHeight, u32OutHeight);
    u32OutWidth  = VAPI_MAX(ALIGN_UP(u32InWidth / 15, 16), u32OutWidth);
    u32OutHeight = VAPI_MAX(ALIGN_UP(u32InHeight / 15, 16), u32OutHeight);

    s32Ret = venc_chn_create(VAPI_JPEG_VENC_ID, u32Qfactor, u32OutWidth, u32OutHeight, pu32OutWidth, pu32OutHeight);
    if (s32Ret == HI_SUCCESS) {
        s32Ret = vdec_bind_venc(VAPI_JPEG_VDEC_ID, VAPI_JPEG_VENC_ID);
    } else {
        vdec_chn_destory(VAPI_JPEG_VDEC_ID);
    }

    return s32Ret;
}

static HI_VOID jpeg_destroy(VDEC_CHN VdChn, VENC_CHN VeChn)
{
    vdec_unbind_venc(VdChn, VeChn);
    vdec_chn_destory(VdChn);
    venc_chn_destory(VeChn);
}

static HI_S32 jpeg_get_stream(VENC_CHN Vechn, HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen)
{
    struct epoll_event events[EPOLL_EVENT];
    VENC_CHN_STAT_S    stStat;
    VENC_STREAM_S      stStream;
    HI_U8             *data = NULL;
    HI_U32             size = 0;
    HI_S32             nfds = 0;
    HI_S32             i    = 0;
    HI_U32             pack_size;
    HI_S32             s32Ret;

    nfds = comm_epoll_wait(g_stJpeg.epfd, events, EPOLL_EVENT, 1000);
    if (nfds != 1) {
        VAPILOG("jpeg_get_stream chn[%d] time out !\n", Vechn);
        return HI_FAILURE;
    } else {
        if ((events[0].events & EPOLLIN) && events[0].data.fd) {
            memset(&stStream, 0, sizeof(stStream));
            s32Ret = HI_MPI_VENC_Query(Vechn, &stStat);
            if (HI_SUCCESS != s32Ret) {
                VAPILOG("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                return HI_FAILURE;
            }

            if (0 == stStat.u32CurPacks) {
                return HI_FAILURE;
            }

            stStream.pstPack = (VENC_PACK_S *)ms_malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
            if (NULL == stStream.pstPack) {
                VAPILOG("malloc stream pack failed!\n");
                return HI_FAILURE;
            }

            stStream.u32PackCount = stStat.u32CurPacks;
            s32Ret                = HI_MPI_VENC_GetStream(Vechn, &stStream, -1);
            if (HI_SUCCESS != s32Ret) {
                ms_free(stStream.pstPack);
                stStream.pstPack = NULL;
                VAPILOG("HI_MPI_VENC_GetStream failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }

            for (i = 0; i < stStream.u32PackCount; i++) {
                size += stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;
            }

            data = ms_malloc(size);
            if (data == NULL) {
                ms_free(stStream.pstPack);
                return HI_FAILURE;
            }
            pack_size = 0;
            for (i = 0; i < stStream.u32PackCount; i++) {
                memcpy(data + pack_size, stStream.pstPack[i].pu8Addr + stStream.pstPack[i].u32Offset, stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset);
                pack_size += stStream.pstPack[i].u32Len - stStream.pstPack[i].u32Offset;
            }
            s32Ret = HI_MPI_VENC_ReleaseStream(Vechn, &stStream);
            if (HI_SUCCESS != s32Ret) {
                ms_free(stStream.pstPack);
                ms_free(data);
                stStream.pstPack = NULL;
                VAPILOG("HI_MPI_VENC_ReleaseStream failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }

            ms_free(stStream.pstPack);
            stStream.pstPack = NULL;
        } else {
            VAPILOG("[ERROR]epoll error!\n");
        }
    }

    *ppu8OutAddr = data;
    *pu32OutLen  = size;

    return HI_SUCCESS;
}


HI_S32 vapi_jpeg_hw(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight,
                            HI_U8 *pu8InAddr, HI_U32 u32InLen,
                            HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight,
                            HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen, HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight,
                            pthread_mutex_t *mutex)
{
    VDEC_STREAM_S stStream;
    HI_S32        s32Ret = HI_FAILURE;

    if (comm_mutex_timelock(&g_stJpeg.mutex, 1000000) != 0) {
        VAPILOG("[ERROR]mutex timeout!\n");
        return s32Ret;
    }

    comm_mutex_lock(mutex);
    s32Ret = jpeg_create(enType, u32InWidth, u32InHeight, u32Qfactor, u32OutWidth, u32OutHeight, pu32OutWidth, pu32OutHeight);
    comm_mutex_unlock(mutex);
    if (s32Ret != HI_SUCCESS) {
        comm_mutex_unlock(&g_stJpeg.mutex);
        VAPILOG("[ERROR]jpeg_create failed!\n");
        return s32Ret;
    }

    stStream.u64PTS       = 0;
    stStream.pu8Addr      = pu8InAddr;
    stStream.u32Len       = u32InLen;
    stStream.bEndOfFrame  = HI_TRUE;
    stStream.bEndOfStream = HI_TRUE;

    s32Ret = HI_MPI_VDEC_SendStream(VAPI_JPEG_VDEC_ID, &stStream, -1);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_MPI_VDEC_SendStream failed with %#x!\n", s32Ret);
        goto JOUT;
    }

    s32Ret = jpeg_get_stream(VAPI_JPEG_VENC_ID, ppu8OutAddr, pu32OutLen);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("[ERROR]jpeg_get_stream failed!\n");
    }

JOUT:
    comm_mutex_lock(mutex);
    jpeg_destroy(VAPI_JPEG_VDEC_ID, VAPI_JPEG_VENC_ID);
    comm_mutex_unlock(mutex);
    comm_mutex_unlock(&g_stJpeg.mutex);

    return s32Ret;
}

HI_S32 vapi_yuv2jpeg(VIDEO_FRAME_INFO_S * pstFrame, HI_U8 **ppData, HI_U32 *pSize, HI_U32 *pWidth, HI_U32 *pHeight)
{
    HI_S32 s32Ret;
    comm_mutex_lock(&g_stJpeg.mutex_ex);
    s32Ret = yuv420sp_to_jpeg(pstFrame, ppData, pSize, pWidth, pHeight);
    comm_mutex_unlock(&g_stJpeg.mutex_ex);

    return s32Ret;
}

HI_VOID vapi_jpeg_init()
{
    memset(&g_stJpeg, 0, sizeof(JPEG_S));
    g_stJpeg.epfd = comm_epoll_create(EPOLL_EVENT);
    comm_mutex_init(&g_stJpeg.mutex);
    comm_mutex_init(&g_stJpeg.mutex_ex);

}

HI_VOID vapi_jpeg_uninit()
{
    comm_mutex_uninit(&g_stJpeg.mutex);
    comm_mutex_uninit(&g_stJpeg.mutex_ex);
    comm_epoll_destroy(g_stJpeg.epfd);
}


