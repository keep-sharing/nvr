/* 
 * ***************************************************************
 * Filename:      	vdec.c
 * Created at:    	2015.10.21
 * Description:   	video decoder controller
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

#include "msstd.h"
#include "showlogo.h"

static PAYLOAD_TYPE_E logo_vdec_get_format(TYPE_E enType)
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
            msprintf("can not support type[%d], use default[H264]", enType);
            enPT = PT_H264;
            break;        
    }

    return enPT;
}

HI_S32 logo_vdec_chn_create(VDEC_CHN VdChn, TYPE_E enType, 
                            HI_U32 u32Width, HI_U32 u32Height)
{
    VDEC_CHN_ATTR_S stVdecChnAttr;
    PAYLOAD_TYPE_E enPT;
    HI_S32 s32Ret;
    
    enPT = logo_vdec_get_format(enType);
    
    stVdecChnAttr.enType       = enPT;
    stVdecChnAttr.u32BufSize   = 3 * u32Width * u32Height;
    stVdecChnAttr.u32Priority  = 5;
    stVdecChnAttr.u32PicWidth  = u32Width;
    stVdecChnAttr.u32PicHeight = u32Height;
    if (PT_H264 == enPT || PT_MP4VIDEO == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 1;
        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
    }
    else if (PT_JPEG == enPT || PT_MJPEG == enPT)
    {
        stVdecChnAttr.stVdecJpegAttr.enMode = VIDEO_MODE_FRAME;
        stVdecChnAttr.stVdecJpegAttr.enJpegFormat = JPG_COLOR_FMT_YCBCR420;
    }
    else if (PT_H265 == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 2;
        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 1;
    } 

//    HI_MPI_VDEC_SetChnVBCnt(VdChn, 10);
    s32Ret = HI_MPI_VDEC_CreateChn(VdChn, &stVdecChnAttr);    
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VDEC_CreateChn[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VDEC_SetDisplayMode(VdChn, VIDEO_DISPLAY_MODE_PLAYBACK);
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VDEC_SetDisplayMode[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VDEC_StartRecvStream(VdChn);
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VDEC_StartRecvStream[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 logo_vdec_chn_destory(VDEC_CHN VdChn)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_VDEC_StopRecvStream(VdChn);    
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VDEC_StopRecvStream[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VDEC_DestroyChn(VdChn);
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VDEC_DestroyChn[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    } 
        
    return HI_SUCCESS;
}

HI_S32 logo_vdec_bind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    
    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = VpssChn;
    
    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_SYS_Bind [%d->%d] failed with %#x!\n", VdChn, VpssGrp, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_S32 logo_vdec_unbind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    
    stSrcChn.enModId = HI_ID_VDEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = VdChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = VpssChn;
    
    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_SYS_UnBind [%d->%d] failed with %#x!\n", VdChn, VpssGrp, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_S32 logo_vdec_send_frame(VDEC_CHN VdChn, HI_U8* pu8Addr, HI_U32 u32Len, HI_U64 u64PTS)
{        
    VDEC_STREAM_S stStream;
    HI_S32 s32Ret;

    stStream.u64PTS  = u64PTS;//us
    stStream.pu8Addr = pu8Addr;
    stStream.u32Len  = u32Len; 
    stStream.bEndOfFrame  = HI_TRUE;
    stStream.bEndOfStream = HI_FALSE;
    s32Ret=HI_MPI_VDEC_SendStream(VdChn, &stStream, 0); // noblock
    if (HI_SUCCESS != s32Ret)
    {
        msprintf("HI_MPI_VDEC_SendStream [%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}


