/* 
 * ***************************************************************
 * Filename:      	vapi_vdec.c
 * Created at:    	2015.10.21
 * Description:   	video decode controller
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

typedef struct vdec_s{
    CHNINFO_S       stChn[MAX_CHN_NUM];
    VDEC_CB_S       stCallback;
    HI_U8   *       pstBuff[MAX_CHN_NUM];
    HI_S32          isBlock;
}VDEC_S;

static VDEC_S g_stVdec;

static PAYLOAD_TYPE_E vapi_vdec_get_format(TYPE_E enType)
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

HI_S32 vapi_vdec_chn_create(int DevId, VDEC_CHN VdChn, TYPE_E enType, 
                            HI_U32 u32Width, HI_U32 u32Height)
{
    VDEC_CHN_ATTR_S stVdecChnAttr;
    VDEC_CHN_PARAM_S stParam;
    PAYLOAD_TYPE_E enPT;
    VIDEO_DISPLAY_MODE_E enDisplayMode = VIDEO_DISPLAY_MODE_PREVIEW;
    HI_S32 buffsize;
    HI_S32 s32Ret;

    //how many times the Vdchn had been used
//    g_stVdec.stChn[VdChn].used++;
    
    if (g_stVdec.stChn[VdChn].enState == STATE_BUSY)
    {
//        VAPILOG("vdec channel[%d] has existed !\n", VdChn);
        return HI_SUCCESS;
    }
    
    enPT = vapi_vdec_get_format(enType);
    VB_PIC_BLK_SIZE(u32Width, u32Height, enPT, buffsize);
    
    stVdecChnAttr.enType       = enPT;
    stVdecChnAttr.u32BufSize   = buffsize;
    stVdecChnAttr.u32Priority  = 10;
    stVdecChnAttr.u32PicWidth  = u32Width;
    stVdecChnAttr.u32PicHeight = u32Height;
    if (PT_H264 == enPT || PT_MP4VIDEO == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
//        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_STREAM;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 2;
        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 0;
    }
    else if (PT_JPEG == enPT || PT_MJPEG == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
//        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_STREAM;
        stVdecChnAttr.stVdecJpegAttr.enJpegFormat = JPG_COLOR_FMT_YCBCR420;
    }
    else if (PT_H265 == enPT)
    {
        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_FRAME;
//        stVdecChnAttr.stVdecVideoAttr.enMode = VIDEO_MODE_STREAM;
        stVdecChnAttr.stVdecVideoAttr.u32RefFrameNum = 2;
        stVdecChnAttr.stVdecVideoAttr.bTemporalMvpEnable = 1;
    } 
    
    s32Ret = HI_MPI_VDEC_SetChnVBCnt(VdChn, 6);//debug notyet    
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_SetChnVBCnt[%d] failed with %#x!\n", VdChn, s32Ret);
//        g_stVdec.stChn[VdChn].used--;
        goto verr;
    }
    s32Ret = HI_MPI_VDEC_CreateChn(VdChn, &stVdecChnAttr);    
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_CreateChn[%d] failed with %#x!\n", VdChn, s32Ret);
        VAPILOG("error: width = %d. height = %d encType = %d \n", u32Width, u32Height, enPT);
//        g_stVdec.stChn[VdChn].used--;
        goto verr;
    }

    enDisplayMode = g_stVdec.isBlock? VIDEO_DISPLAY_MODE_PLAYBACK : VIDEO_DISPLAY_MODE_PREVIEW;
    s32Ret = HI_MPI_VDEC_SetDisplayMode(VdChn, enDisplayMode);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_SetDisplayMode[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_MPI_VDEC_DestroyChn(VdChn);
//        g_stVdec.stChn[VdChn].used--;
        goto verr;
    }
    
    s32Ret = HI_MPI_VDEC_GetChnParam(VdChn, &stParam);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_GetChnParam[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_MPI_VDEC_DestroyChn(VdChn);
//        g_stVdec.stChn[VdChn].used--;
        goto verr;
    }

    stParam.s32DisplayFrameNum = 4;
//    stParam.s32DecOrderOutput = 0;
    s32Ret = HI_MPI_VDEC_SetChnParam(VdChn, &stParam);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_SetChnParam[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_MPI_VDEC_DestroyChn(VdChn);
//        g_stVdec.stChn[VdChn].used--;
        goto verr;
    }
    
    s32Ret = HI_MPI_VDEC_StartRecvStream(VdChn);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_StartRecvStream[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_MPI_VDEC_DestroyChn(VdChn);
//        g_stVdec.stChn[VdChn].used--;
        goto verr;
    }
    
    g_stVdec.stChn[VdChn].enState = STATE_BUSY;    
    g_stVdec.stChn[VdChn].stFormat.enType = enType;
    g_stVdec.stChn[VdChn].stFormat.width = u32Width;
    g_stVdec.stChn[VdChn].stFormat.height = u32Height;

    if (g_stVdec.stCallback.update_chn_cb != NULL)
    {
        g_stVdec.stCallback.update_chn_cb(DevId, VdChn, STATE_BUSY, u32Width*u32Height);
    }

    if(g_stVdec.stCallback.start_stream_cb != NULL)
    {
        g_stVdec.stCallback.start_stream_cb(VdChn);
    }
    
    return HI_SUCCESS;
verr:
    if (g_stVdec.stCallback.update_chn_cb != NULL)
    {
        g_stVdec.stCallback.update_chn_cb(-1, VdChn, STATE_IDLE, 0);
    }

    return HI_FAILURE;

}

HI_S32 vapi_vdec_chn_destory(VDEC_CHN VdChn)
{
    HI_S32 s32Ret;
    
    if (VdChn < 0)
        return HI_FAILURE;
        
    if(g_stVdec.stChn[VdChn].enState != STATE_BUSY)
    {
        return HI_SUCCESS;
    }

//    g_stVdec.stChn[VdChn].used--;
//    if (g_stVdec.stChn[VdChn].used != 0)
//    {
//        return HI_SUCCESS;
//    }

    if(g_stVdec.stCallback.stop_stream_cb != NULL)
    {
        g_stVdec.stCallback.stop_stream_cb(VdChn);
    }
    
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
    
    g_stVdec.stChn[VdChn].enState = STATE_IDLE;    
    g_stVdec.stChn[VdChn].stFormat.width = 0;
    g_stVdec.stChn[VdChn].stFormat.height = 0;
//    g_stVdec.stChn[VdChn].used = 0;
    
    if (g_stVdec.stCallback.update_chn_cb != NULL)
    {
        g_stVdec.stCallback.update_chn_cb(-1, VdChn, STATE_IDLE, 0);
    }
    
    return HI_SUCCESS;
}

HI_S32 vapi_vdec_bind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
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
        VAPILOG("HI_MPI_SYS_Bind [%d->%d] failed with %#x!\n", VdChn, VpssGrp, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_S32 vapi_vdec_unbind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
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
        VAPILOG("HI_MPI_SYS_UnBind [%d->%d] failed with %#x!\n", VdChn, VpssGrp, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_VOID vapi_vdec_update_state(VDEC_CHN VdChn, HI_S32 state)
{
    g_stVdec.stChn[VdChn].enState = state;
}

HI_BOOL vapi_vdec_check_state(VDEC_CHN VdChn, HI_S32 state)
{
    return (g_stVdec.stChn[VdChn].enState == state);
}

HI_VOID vapi_vdec_dsp_block(HI_BOOL isBlock)
{
    g_stVdec.isBlock = isBlock;
}

HI_S32 vapi_vdec_send_frame(VDEC_CHN VdChn, HI_U8* pu8Addr, HI_U32 u32Len, HI_U64 u64PTS)
{        
    VDEC_STREAM_S stStream;
    VDEC_CHN_STAT_S stStat;
    HI_S32 s32Ret;

    stStream.u64PTS  = u64PTS;//us
    stStream.pu8Addr = pu8Addr;
    stStream.u32Len  = u32Len; 
    stStream.bEndOfFrame  = HI_TRUE;
    stStream.bEndOfStream = HI_FALSE;
    s32Ret=HI_MPI_VDEC_SendStream(VdChn, &stStream, 0); // noblock
    if (HI_SUCCESS != s32Ret)
    {
        // if buffer is full change display mode from playback mode to preview mode 
//        if (s32Ret == HI_ERR_VDEC_BUF_FULL)
//        {
//            s32Ret = HI_MPI_VDEC_SetDisplayMode(VdChn, VIDEO_DISPLAY_MODE_PREVIEW);
//            if (HI_SUCCESS != s32Ret)
//            {
//                VAPILOG("HI_MPI_VDEC_SetDisplayMode [%d] failed with %#x!\n", VdChn, s32Ret);        
//            }
//        }
//        else
        if (s32Ret != HI_ERR_VDEC_UNEXIST) {

            VAPILOG("HI_MPI_VDEC_SendStream [%d] failed with %#x!\n", VdChn, s32Ret);
			VAPILOG("HI_MPI_VDEC_SendStream [%d]  PTS:%llu Addr:0x%p Len:%d\n", VdChn, u64PTS, pu8Addr, u32Len);
        }
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VDEC_Query(VdChn, &stStat);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VDEC_Query [%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    return stStat.u32LeftStreamFrames;
}

HI_S32 vapi_vdec_start_stream(VDEC_CHN VdChn)
{
    if (g_stVdec.stCallback.start_stream_cb != NULL)
    {
//        if (g_stVdec.stChn[VdChn].used == 1)
        {
            g_stVdec.stCallback.start_stream_cb(VdChn);
        }
    }
    
    return HI_SUCCESS;
}

HI_S32 vapi_vdec_stop_stream(VDEC_CHN VdChn)
{
    if (g_stVdec.stCallback.stop_stream_cb != NULL)
    {
//        if (g_stVdec.stChn[VdChn].used == 1)
        {
            g_stVdec.stCallback.stop_stream_cb(VdChn);
        }
    }
    
    return HI_SUCCESS;
}

HI_VOID vapi_vdec_init(VDEC_CB_S *pstCb, int isBlock)
{
    HI_S32 i;

    for(i=0; i<MAX_CHN_NUM; i++)
    {
        g_stVdec.stChn[i].enState = STATE_IDLE;
//        g_stVdec.stChn[i].used = 0;
    }
    
    g_stVdec.stCallback.start_stream_cb = pstCb->start_stream_cb;
    g_stVdec.stCallback.stop_stream_cb = pstCb->stop_stream_cb;
    g_stVdec.stCallback.update_chn_cb = pstCb->update_chn_cb;
    g_stVdec.isBlock = isBlock;
}

HI_VOID vapi_vdec_uninit()
{
    HI_S32 i;
    
    for(i=0; i<MAX_CHN_NUM; i++)
    {
        vapi_vdec_chn_destory(i);
    }
    
    g_stVdec.stCallback.start_stream_cb = NULL;
    g_stVdec.stCallback.stop_stream_cb = NULL;
    g_stVdec.stCallback.update_chn_cb = NULL;
}

