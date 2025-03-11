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

typedef struct jhandle_s{
    HI_HANDLE       hAvplay;
    HI_HANDLE       hWin;
    HI_HANDLE       hVenc;
}JHANDLE_S;

typedef struct jpeg_s{
	pthread_mutex_t mutex;
	pthread_mutex_t mutex_ex;
    JHANDLE_S hdl[MAX_JPEG_NUM];
}JPEG_S;

static JPEG_S g_stJpeg;

static HI_UNF_VCODEC_TYPE_E vdec_get_format(TYPE_E enType)
{   
    HI_UNF_VCODEC_TYPE_E enPT;

    switch(enType)
    {
        case ENC_TYPE_H264:
            enPT = HI_UNF_VCODEC_TYPE_H264;
            break;
        case ENC_TYPE_H265:
            enPT = HI_UNF_VCODEC_TYPE_HEVC;
            break;
        case ENC_TYPE_MJPEG:
            enPT = HI_UNF_VCODEC_TYPE_MJPEG;
            break;
        default :            
            VAPILOG("can not support type[%d], use default[H264]", enType);
            enPT = HI_UNF_VCODEC_TYPE_H264;
            break;        
    }

    return enPT;
}

static HI_S32 yuv420sp_to_jpeg(HI_UNF_VIDEO_FRAME_INFO_S * pstFrame, HI_U8 **ppData, HI_U32 *pSize, HI_U32 *pWidth, HI_U32 *pHeight)
{
    HI_UNF_VENC_CHN_ATTR_S stVencChnAttr;
    HI_UNF_VENC_STREAM_S   stStream;
    HI_UNF_VI_ATTR_S stViAttr;
    HI_HANDLE hVi;
    HI_HANDLE hVenc;
    HI_U8 *data = NULL;
    HI_U32 size = 0;
    HI_S32 n = 5;
    HI_S32 s32Ret;

    HI_U32 u32Width  = VAPI_MIN(pstFrame->u32Width, VAPI_VENC_MAX_WIDTH);
    HI_U32 u32Height = VAPI_MIN(pstFrame->u32Height, VAPI_VENC_MAX_HEIGTH);
    
    u32Width  = VAPI_MAX(u32Width, VAPI_VENC_MIN_WIDTH);
    u32Height = VAPI_MAX(u32Height, VAPI_VENC_MIN_HEIGTH);
    
//    printf("======o[%d][%d]=c[%d][%d]===\n", pstFrame->u32Width, pstFrame->u32Height,
//                                            u32Width, u32Height);
    s32Ret = HI_UNF_VENC_GetDefaultAttr(&stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_GetDefaultAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    stVencChnAttr.enVencType        = HI_UNF_VCODEC_TYPE_JPEG;
    stVencChnAttr.u32Width          = u32Width;
    stVencChnAttr.u32Height         = u32Height;
    stVencChnAttr.u32Qlevel         = 50;
    stVencChnAttr.bQuickEncode      = HI_TRUE;
    stVencChnAttr.enCapLevel        = comm_get_vcodec_cap_level(u32Width, u32Height, &stVencChnAttr.u32StrmBufSize);
    if (stVencChnAttr.enCapLevel > HI_UNF_VCODEC_CAP_LEVEL_FULLHD)
    {
        stVencChnAttr.enCapLevel = HI_UNF_VCODEC_CAP_LEVEL_FULLHD;
        stVencChnAttr.u32StrmBufSize = 1920*1080*2;
        //return HI_FAILURE;
    }
    s32Ret = HI_UNF_VENC_Create(&hVenc, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_Create failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    memset(&stViAttr, 0, sizeof(HI_UNF_VI_ATTR_S));
    stViAttr.bVirtual = HI_TRUE;
    stViAttr.stInputRect.s32X = 0;
    stViAttr.stInputRect.s32Y = 0;
    stViAttr.stInputRect.s32Width  = pstFrame->u32Width;
    stViAttr.stInputRect.s32Height = pstFrame->u32Height;
    stViAttr.enVideoFormat = HI_UNF_FORMAT_YUV_SEMIPLANAR_420;
    stViAttr.enBufMgmtMode = HI_UNF_VI_BUF_ALLOC;
    stViAttr.u32BufNum = 6;
    s32Ret = HI_UNF_VI_Create(HI_UNF_VI_PORT0, &stViAttr, &hVi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VI_Create failed with %#x!\n", s32Ret);
        HI_UNF_VENC_Destroy(hVenc);
        return HI_FAILURE;
    }
        
    s32Ret = HI_UNF_VENC_AttachInput(hVenc, hVi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_AttachWindow failed with %#x!\n", s32Ret);
        HI_UNF_VENC_Destroy(hVenc);
        HI_UNF_VI_Destroy(hVi);
        return HI_FAILURE;
    }

    s32Ret = HI_UNF_VI_Start(hVi);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VI_Start with %#x!\n", s32Ret);
        HI_UNF_VENC_Destroy(hVenc);
        HI_UNF_VI_Destroy(hVi);
        return HI_FAILURE;
    }
    
    s32Ret =  HI_UNF_VENC_Start(hVenc);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_Startfailed with %#x!\n", s32Ret);
        HI_UNF_VENC_Destroy(hVenc);
        HI_UNF_VI_Stop(hVi);
        HI_UNF_VI_Destroy(hVi);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_VI_QueueFrame(hVi, pstFrame);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VO_QueueFrame time out with %#x! \n", s32Ret);
    }

    while(n--)
    {
        s32Ret = HI_UNF_VENC_AcquireStream(hVenc, &stStream, 100);
        if (HI_ERR_VENC_BUF_EMPTY == s32Ret)
            continue;
        else if (HI_SUCCESS != s32Ret)
        {
            VAPILOG("HI_UNF_VENC_AcquireStream time out with %#x! \n", s32Ret);
            goto OUT;
        }
        else
            break;
    }

    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_AcquireStream with %#x! \n", s32Ret);
        goto OUT;
    }

    size = stStream.u32SlcLen;
    data = ms_malloc(size);
    if (data == NULL)
    {
        VAPILOG("ms_malloc failed with \n");
        s32Ret =  HI_FAILURE;
        goto OUT;
    }
    
    memcpy(data, stStream.pu8Addr, size);
    
    HI_UNF_VENC_ReleaseStream(hVenc, &stStream);
OUT:
    HI_UNF_VI_DequeueFrame(hVi, pstFrame);
    HI_UNF_VI_Stop(hVi);
    HI_UNF_VENC_Stop(hVenc);
    HI_UNF_VENC_DetachInput(hVenc);
    HI_UNF_VENC_Destroy(hVenc);
    HI_UNF_VI_Destroy(hVi);
    
    *ppData = data;
    *pSize = size;
    *pWidth = u32Width;
    *pHeight = u32Height;

    return s32Ret;
}

static HI_S32 jpeg_get_stream(HI_S32 VdChn, TYPE_E enType,
                                 HI_U8 *pu8InAddr, HI_U32 u32InLen,
                                 HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen)
{
    HI_UNF_STREAM_BUF_S stStreamBuf;
    HI_UNF_VENC_STREAM_S stStream;
    JHANDLE_S *pstHdl = &g_stJpeg.hdl[VdChn];
    HI_U8 *data = NULL;
    HI_U32 size = 0;
    HI_S32 s32Ret;
    HI_S32 n = 5;

    memset(&stStreamBuf, 0, sizeof(HI_UNF_STREAM_BUF_S));
    s32Ret = HI_UNF_AVPLAY_GetBuf(pstHdl->hAvplay, HI_UNF_AVPLAY_BUF_ID_ES_VID, u32InLen, &stStreamBuf, 0);
    if (HI_SUCCESS == s32Ret)
    {	
	if(stStreamBuf.pu8Data == NULL)
	    VAPILOG("stStreamBuf.pu8Data is NULL\n");
        memcpy(stStreamBuf.pu8Data, pu8InAddr, u32InLen);
        s32Ret = HI_UNF_AVPLAY_PutBuf(pstHdl->hAvplay, HI_UNF_AVPLAY_BUF_ID_ES_VID, u32InLen, -1);
        if (HI_SUCCESS != s32Ret)
        {
            VAPILOG("HI_UNF_AVPLAY_PutBuf [%d] with %#x!\n", VdChn, s32Ret);
            return HI_FAILURE;
        }
    }
    else
    {
        VAPILOG("HI_UNF_AVPLAY_GetBuf [%d] with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_UNF_AVPLAY_FlushStream(pstHdl->hAvplay, NULL);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_FlushStream[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }

    while(n--)
    {
        s32Ret = HI_UNF_VENC_AcquireStream(pstHdl->hVenc, &stStream, 100);
        if (HI_ERR_VENC_BUF_EMPTY == s32Ret)
            continue;
        else if (HI_SUCCESS != s32Ret)
        {
            VAPILOG("HI_UNF_VENC_AcquireStream[%d] time out with %#x! \n", VdChn, s32Ret);
            return HI_FAILURE;
        }
        else
            break;
    }

    if (n < 0)
    {
        VAPILOG("HI_UNF_VENC_AcquireStream[%d] with %#x! \n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    size = stStream.u32SlcLen;
    data = ms_malloc(size);
    if (data == NULL)
    {
        VAPILOG("ms_malloc failed with \n");
        s32Ret = HI_UNF_VENC_ReleaseStream(pstHdl->hVenc, &stStream);
        {
            VAPILOG("HI_UNF_VENC_ReleaseStream[%d] failed with %#x!\n", VdChn, s32Ret);
            return HI_FAILURE;
        }

    }

    memcpy(data, stStream.pu8Addr, size);
    s32Ret = HI_UNF_VENC_ReleaseStream(pstHdl->hVenc, &stStream);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_ReleaseStream[%d] failed with %#x!\n", VdChn, s32Ret);
        ms_free(data);
        return HI_FAILURE;
    }

    *ppu8OutAddr = data;
    *pu32OutLen = size;
    return HI_SUCCESS;
}

static HI_S32 vdec_chn_create(HI_S32 VdChn, TYPE_E enType, 
                            HI_U32 u32Width, HI_U32 u32Height)
{   
    HI_UNF_AVPLAY_ATTR_S stAvplayAttr;
    HI_UNF_VCODEC_ATTR_S stVcodecAttr;
    HI_UNF_AVPLAY_OPEN_OPT_S stVidOpenOpt;
    JHANDLE_S *pstHdl = &g_stJpeg.hdl[VdChn];
    HI_S32 s32Ret;


    if (u32Width > VAPI_VDEC_MAX_WIDTH || u32Height > VAPI_VDEC_MAX_HEIGTH)
    {
        VAPILOG("resolution must be <= %dx%d", VAPI_VDEC_MAX_WIDTH, VAPI_VDEC_MAX_HEIGTH);
        return HI_FAILURE;
    }

    s32Ret = vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_BEGIN);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("jpeg vdec no resource\n");
        return HI_FAILURE;
    }

    s32Ret  = HI_UNF_AVPLAY_GetDefaultConfig(&stAvplayAttr, HI_UNF_AVPLAY_STREAM_TYPE_ES);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_GetDefaultConfig[%d] failed with %#x!\n", VdChn, s32Ret);
        vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);
        return HI_FAILURE;
    }
        
    s32Ret = HI_UNF_AVPLAY_Create(&stAvplayAttr, &pstHdl->hAvplay);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_CreateChn[%d] failed with %#x!\n", VdChn, s32Ret);
        vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);
        return HI_FAILURE;
    }

    stVidOpenOpt.enCapLevel = comm_get_vcodec_cap_level(u32Width, u32Height, NULL);
    stVidOpenOpt.enDecType = HI_UNF_VCODEC_DEC_TYPE_ISINGLE;

    if (enType == ENC_TYPE_H264)
        stVidOpenOpt.enProtocolLevel = HI_UNF_VCODEC_PRTCL_LEVEL_H264;
    else
        stVidOpenOpt.enProtocolLevel = HI_UNF_VCODEC_PRTCL_LEVEL_MPEG;
    
    s32Ret = HI_UNF_AVPLAY_ChnOpen(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID, &stVidOpenOpt);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_ChnOpen[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_Destroy(pstHdl->hAvplay);
        vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_AVPLAY_GetAttr(pstHdl->hAvplay, HI_UNF_AVPLAY_ATTR_ID_VDEC, &stVcodecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_GetAttr[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_ChnClose(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(pstHdl->hAvplay);
        vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);
        return HI_FAILURE;
    }

    stVcodecAttr.enType = vdec_get_format(enType);
    stVcodecAttr.enMode = HI_UNF_VCODEC_MODE_I;
    stVcodecAttr.u32ErrCover = 100;
    stVcodecAttr.u32Priority = 3;    
    s32Ret = HI_UNF_AVPLAY_SetAttr(pstHdl->hAvplay, HI_UNF_AVPLAY_ATTR_ID_VDEC, &stVcodecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_SetAttr[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_ChnClose(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(pstHdl->hAvplay);
        vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_VO_AttachWindow(pstHdl->hWin, pstHdl->hAvplay);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_AttachWindow[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_ChnClose(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(pstHdl->hAvplay);
        vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);
        return s32Ret;
    }

    s32Ret = HI_UNF_AVPLAY_Start(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_Start[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_VO_SetWindowEnable(pstHdl->hWin, HI_FALSE);
        HI_UNF_VO_DetachWindow(pstHdl->hWin, pstHdl->hAvplay);
        HI_UNF_AVPLAY_ChnClose(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(pstHdl->hAvplay);
        vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);
        return s32Ret;
    }

    return HI_SUCCESS;
}


static HI_S32 vdec_chn_destory(HI_S32 VdChn)
{
    HI_UNF_AVPLAY_STOP_OPT_S    stStopOpt;
    JHANDLE_S *pstHdl = &g_stJpeg.hdl[VdChn];
    HI_S32 s32Ret;

    stStopOpt.enMode = HI_UNF_AVPLAY_STOP_MODE_BLACK;
    stStopOpt.u32TimeoutMs = 10*1000;
    s32Ret = HI_UNF_AVPLAY_Stop(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID, &stStopOpt);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_Stop[%d] failed with %#x!\n", VdChn, s32Ret);
    } 

    s32Ret = HI_UNF_VO_DetachWindow(pstHdl->hWin, pstHdl->hAvplay);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_DetachWindow handle[%d]failed with %#x!\n", pstHdl->hWin, s32Ret);
    }

    s32Ret = HI_UNF_AVPLAY_ChnClose(pstHdl->hAvplay, HI_UNF_AVPLAY_MEDIA_CHAN_VID);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_ChnClose[%d] failed with %#x!\n", VdChn, s32Ret);
    }
    
    s32Ret = HI_UNF_AVPLAY_Destroy(pstHdl->hAvplay);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_Destroy[%d] failed with %#x!\n", VdChn, s32Ret);
    }
    
    vapi_vdec_chn_update(-1, JPEG_IDX_BASE, STATE_SNAP_END);

    return s32Ret;
}

static HI_S32 venc_chn_create(HI_S32 VeChn, HI_U32 u32Qfactor, HI_U32 u32Width, HI_U32 u32Height, HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight)
{
    HI_UNF_VENC_CHN_ATTR_S stVencChnAttr;
    JHANDLE_S *pstHdl = &g_stJpeg.hdl[VeChn];
    HI_S32 s32Ret;

    u32Width  = VAPI_MIN(u32Width, VAPI_VENC_MAX_WIDTH);
    u32Height = VAPI_MIN(u32Height, VAPI_VENC_MAX_HEIGTH);
    u32Width  = ALIGN_BACK(VAPI_MAX(u32Width, VAPI_VENC_MIN_WIDTH), 4);
    u32Height = ALIGN_BACK(VAPI_MAX(u32Height, VAPI_VENC_MIN_HEIGTH), 4);
    
	*pu32OutWidth = u32Width;
	*pu32OutHeight = u32Height;
	
    s32Ret = HI_UNF_VENC_GetDefaultAttr(&stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_GetDefaultAttr[%d] failed with %#x!\n", VeChn, s32Ret);
        return HI_FAILURE;
    }
    stVencChnAttr.enVencType        = HI_UNF_VCODEC_TYPE_JPEG;
    stVencChnAttr.u32Width          = u32Width;
    stVencChnAttr.u32Height         = u32Height;
    stVencChnAttr.u32Qlevel         = u32Qfactor;
//    stVencChnAttr.bQuickEncode      = HI_TRUE;
    stVencChnAttr.enCapLevel        = comm_get_vcodec_cap_level(u32Width, u32Height, &stVencChnAttr.u32StrmBufSize);
    if (stVencChnAttr.enCapLevel > HI_UNF_VCODEC_CAP_LEVEL_FULLHD)
    {
    	stVencChnAttr.enCapLevel = HI_UNF_VCODEC_CAP_LEVEL_FULLHD;
		stVencChnAttr.u32StrmBufSize = 1920*1080*2;
        //return HI_FAILURE;
    }
        
    s32Ret = HI_UNF_VENC_Create(&pstHdl->hVenc, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_Create[%d] failed with %#x!\n", VeChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_VENC_AttachInput(pstHdl->hVenc, pstHdl->hWin);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_AttachWindow[%d] failed with %#x!\n", VeChn, s32Ret);
        HI_UNF_VENC_Destroy(pstHdl->hVenc);
        return HI_FAILURE;
    }
    
    s32Ret =  HI_UNF_VENC_Start(pstHdl->hVenc);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_Start[%d] failed with %#x!\n", VeChn, s32Ret);
        HI_UNF_VO_DetachWindow(pstHdl->hWin, pstHdl->hAvplay);
        HI_UNF_VENC_DetachInput(pstHdl->hVenc);
        HI_UNF_VENC_Destroy(pstHdl->hVenc);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

static HI_S32 venc_chn_destory(HI_S32 VeChn)
{
    JHANDLE_S *pstHdl = &g_stJpeg.hdl[VeChn];
    HI_S32 s32Ret;

    s32Ret = HI_UNF_VENC_Stop(pstHdl->hVenc);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_Stop[%d] failed with %#x!\n", VeChn, s32Ret);
    }

    s32Ret = HI_UNF_VENC_DetachInput(pstHdl->hVenc);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VENC_DetachInput handle[%d]failed with %#x!\n", pstHdl->hWin, s32Ret);
    }

    s32Ret = HI_UNF_VENC_Destroy(pstHdl->hVenc);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_Destroy[%d] failed with %#x!\n", VeChn, s32Ret);
    }
    
    return s32Ret;
}

static HI_S32 jpeg_create(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight,
                            HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight,
                            HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight)
{
    HI_S32 s32Ret;
    
    s32Ret = vdec_chn_create(VAPI_JPEG_VDEC_ID, enType, u32InWidth, u32InHeight);
    u32OutWidth = VAPI_MIN(u32InWidth, u32OutWidth);
    u32OutHeight = VAPI_MIN(u32InHeight, u32OutHeight);

    if (s32Ret == HI_SUCCESS)
    {
        s32Ret = venc_chn_create(VAPI_JPEG_VENC_ID, u32Qfactor, u32OutWidth, u32OutHeight, pu32OutWidth, pu32OutHeight);
        if (s32Ret != HI_SUCCESS)
            vdec_chn_destory(VAPI_JPEG_VDEC_ID);
    }
    return s32Ret;
}

static HI_VOID jpeg_destroy(HI_S32 VdChn, HI_S32 VeChn)
{
    vdec_chn_destory(VdChn);
    venc_chn_destory(VeChn);
}

HI_S32 vapi_jpeg_hw(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight,
                            HI_U8 *pu8InAddr, HI_U32 u32InLen,
                            HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight,
                            HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen, HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight,
                            pthread_mutex_t *mutex)
{    
    HI_S32 s32Ret = HI_FAILURE;
    
    if (comm_mutex_timelock(&g_stJpeg.mutex, 1000000) != 0)
        return s32Ret;
        
    comm_mutex_lock(mutex);
    s32Ret = jpeg_create(enType, u32InWidth, u32InHeight, u32Qfactor, u32OutWidth, u32OutHeight, pu32OutWidth, pu32OutHeight);
    comm_mutex_unlock(mutex);
    if (s32Ret != HI_SUCCESS)
    {
        comm_mutex_unlock(&g_stJpeg.mutex);
        return s32Ret;
    }
    
    s32Ret = jpeg_get_stream(VAPI_JPEG_VENC_ID, enType, pu8InAddr, u32InLen, ppu8OutAddr, pu32OutLen);
    comm_mutex_lock(mutex);
    jpeg_destroy(VAPI_JPEG_VDEC_ID, VAPI_JPEG_VENC_ID);
    comm_mutex_unlock(mutex);
    comm_mutex_unlock(&g_stJpeg.mutex);
    
    return s32Ret;
}

HI_S32 vapi_yuv2jpeg(HI_UNF_VIDEO_FRAME_INFO_S * pstFrame, HI_U8 **ppData, HI_U32 *pSize, HI_U32 *pWidth, HI_U32 *pHeight)
{
    HI_S32 s32Ret;
    comm_mutex_lock(&g_stJpeg.mutex_ex);
    s32Ret = yuv420sp_to_jpeg(pstFrame, ppData, pSize, pWidth, pHeight);
    comm_mutex_unlock(&g_stJpeg.mutex_ex);

    return s32Ret;
}


HI_VOID vapi_jpeg_init()
{
    HI_S32 s32Ret;
    memset(&g_stJpeg, 0, sizeof(JPEG_S));
    s32Ret = HI_UNF_VENC_Init();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VENC_Init failed with %#x!\n", s32Ret);
        return ;
    }

    s32Ret = HI_UNF_VI_Init();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VI_Init failed with %#x!\n", s32Ret);
        HI_UNF_VENC_DeInit();
        return ;
    }

    HI_UNF_WINDOW_ATTR_S   WinAttr;
    JHANDLE_S *pstHdl = &g_stJpeg.hdl[VAPI_JPEG_VENC_ID];
    
    memset(&WinAttr, 0, sizeof(HI_UNF_WINDOW_ATTR_S));
    WinAttr.enDisp = HI_UNF_DISPLAY0;
    WinAttr.bVirtual = HI_TRUE;
    WinAttr.enVideoFormat = HI_UNF_FORMAT_YUV_SEMIPLANAR_420;
    s32Ret = HI_UNF_VO_CreateWindow(&WinAttr, &pstHdl->hWin);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_CreateWindow[%d] failed with %#x!\n", VAPI_JPEG_VENC_ID, s32Ret);
        HI_UNF_VENC_DeInit();
        HI_UNF_VI_DeInit();
        return ;
    }
    
    s32Ret = HI_UNF_VO_SetWindowEnable(pstHdl->hWin, HI_TRUE);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VO_SetWindowEnable[%d] failed with %#x!\n", VAPI_JPEG_VENC_ID, s32Ret);
        HI_UNF_VO_DestroyWindow(pstHdl->hWin);
        HI_UNF_VENC_DeInit();
        HI_UNF_VI_DeInit();
        return ;
    }
    comm_mutex_init(&g_stJpeg.mutex);
    comm_mutex_init(&g_stJpeg.mutex_ex);

}

HI_VOID vapi_jpeg_uninit()
{
    JHANDLE_S *pstHdl = &g_stJpeg.hdl[VAPI_JPEG_VENC_ID];
    HI_S32 s32Ret;
    s32Ret = HI_UNF_VO_SetWindowEnable(pstHdl->hWin, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
        VAPILOG("HI_UNF_VO_SetWindowDisable handle[%d]failed with %#x!\n", pstHdl->hWin, s32Ret);
    s32Ret = HI_UNF_VO_DestroyWindow(pstHdl->hWin);
    if (HI_SUCCESS != s32Ret)
        VAPILOG("HI_UNF_VO_DestroyWindow[%d] failed with %#x!\n", VAPI_JPEG_VENC_ID, s32Ret);

    HI_UNF_VENC_DeInit();
    HI_UNF_VI_DeInit();
    comm_mutex_uninit(&g_stJpeg.mutex);
    comm_mutex_uninit(&g_stJpeg.mutex_ex);
}


