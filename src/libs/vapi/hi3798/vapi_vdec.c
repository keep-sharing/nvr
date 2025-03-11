/* 
 * ***************************************************************
 * Filename:      	vapi_vdec.c
 * Created at:    	2016.04.29
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
    HI_HANDLE       hAvplay[MAX_CHN_NUM];
}VDEC_S;

static VDEC_S g_stVdec;

static HI_UNF_VCODEC_TYPE_E vapi_vdec_get_format(TYPE_E enType)
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
static HI_VOID vapi_vdec_get_open_opt(TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height,
                                            HI_UNF_AVPLAY_OPEN_OPT_S *pstVidOpenOpt)
{
    
    pstVidOpenOpt->enCapLevel = comm_get_vcodec_cap_level(u32Width, u32Height, NULL);
    pstVidOpenOpt->enDecType = HI_UNF_VCODEC_DEC_TYPE_NORMAL;

    if (enType == ENC_TYPE_H264)
    {
        pstVidOpenOpt->enProtocolLevel = HI_UNF_VCODEC_PRTCL_LEVEL_H264;
    }
    else
    {
        pstVidOpenOpt->enProtocolLevel = HI_UNF_VCODEC_PRTCL_LEVEL_MPEG;
    }

//    VAPILOG(" num = %d, enCapLevel =%d, enProtocolLevel =%d \n\n", num, pstVidOpenOpt->enCapLevel, pstVidOpenOpt->enProtocolLevel);
}


HI_S32 vapi_vdec_chn_create(HI_S32 DevId, HI_S32 VdChn, TYPE_E enType, MODE_E enMode,
                                 HI_U32 u32Width, HI_U32 u32Height)
{   
    HI_UNF_AVPLAY_ATTR_S stAvplayAttr;
    HI_UNF_SYNC_ATTR_S stAvSyncAttr;
    HI_UNF_AVPLAY_OPEN_OPT_S stVidOpenOpt;
    HI_UNF_VCODEC_ATTR_S stVcodecAttr;
    HI_UNF_AVPLAY_FRMRATE_PARAM_S stFramerate;
    HI_S32 s32Ret;

    
    if (g_stVdec.stChn[VdChn].enState == STATE_BUSY)
    {
//        VAPILOG("vdec channel[%d] has existed !\n", VdChn);
        return HI_SUCCESS;
    }
    
    s32Ret  = HI_UNF_AVPLAY_GetDefaultConfig(&stAvplayAttr, HI_UNF_AVPLAY_STREAM_TYPE_ES);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_GetDefaultConfig[%d] failed with %#x!\n", VdChn, s32Ret);
        goto verr;
    }
    
    stAvplayAttr.stStreamAttr.u32VidBufSize = VAPI_MAX(512*1024, VAPI_MIN(1*u32Width*u32Height, 16*1024*1024));
    s32Ret = HI_UNF_AVPLAY_Create(&stAvplayAttr, &g_stVdec.hAvplay[VdChn]);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_MPI_VDEC_CreateChn[%d] failed with %#x!\n", VdChn, s32Ret);
        goto verr;
    }
    
    s32Ret = HI_UNF_AVPLAY_GetAttr(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_ATTR_ID_SYNC, &stAvSyncAttr);
    stAvSyncAttr.enSyncRef = HI_UNF_SYNC_REF_NONE;
    s32Ret |= HI_UNF_AVPLAY_SetAttr(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_ATTR_ID_SYNC, &stAvSyncAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_SetAttr[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_Destroy(g_stVdec.hAvplay[VdChn]);
        goto verr;
    }
    
    vapi_vdec_get_open_opt(enType, u32Width, u32Height, &stVidOpenOpt);
    s32Ret = HI_UNF_AVPLAY_ChnOpen(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID, &stVidOpenOpt);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_ChnOpen[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_Destroy(g_stVdec.hAvplay[VdChn]);
        goto verr;
    }
    
    s32Ret = HI_UNF_AVPLAY_GetAttr(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_ATTR_ID_VDEC, &stVcodecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_GetAttr[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_ChnClose(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(g_stVdec.hAvplay[VdChn]);
        goto verr;
    }

    stVcodecAttr.enType = vapi_vdec_get_format(enType);
    stVcodecAttr.enMode = HI_UNF_VCODEC_MODE_NORMAL;
//    stVcodecAttr.bOrderOutput = HI_TRUE;
    stVcodecAttr.u32ErrCover = 100;
    stVcodecAttr.u32Priority = 3;    
    s32Ret = HI_UNF_AVPLAY_SetAttr(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_ATTR_ID_VDEC, &stVcodecAttr);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_SetAttr[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_ChnClose(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(g_stVdec.hAvplay[VdChn]);
        goto verr;
    }

//    s32Ret = HI_UNF_AVPLAY_Start(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID, HI_NULL);
//    if (HI_SUCCESS != s32Ret)
//    {
//        VAPILOG("HI_UNF_AVPLAY_Start[%d] failed with %#x!\n", VdChn, s32Ret);
//        g_stVdec.stChn[VdChn].used--;
//        return HI_FAILURE;
//    }

    s32Ret = HI_UNF_AVPLAY_GetAttr(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFramerate);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_GetAttr[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_ChnClose(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(g_stVdec.hAvplay[VdChn]);
        goto verr;
    }

    if (enMode == DSP_MODE_PBK)
    {
        stFramerate.enFrmRateType = HI_UNF_AVPLAY_FRMRATE_TYPE_USER;
        stFramerate.stSetFrmRate.u32fpsInteger = 60;
    }
    else
    {
        stFramerate.enFrmRateType = HI_UNF_AVPLAY_FRMRATE_TYPE_PTS;
    }
    s32Ret = HI_UNF_AVPLAY_SetAttr(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFramerate);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_SetAttr[%d] failed with %#x!\n", VdChn, s32Ret);
        HI_UNF_AVPLAY_ChnClose(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID);
        HI_UNF_AVPLAY_Destroy(g_stVdec.hAvplay[VdChn]);
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
    
    return HI_SUCCESS;

verr:
    if (g_stVdec.stCallback.update_chn_cb != NULL)
    {
        g_stVdec.stCallback.update_chn_cb(-1, VdChn, STATE_IDLE, 0);
    }

    return HI_FAILURE;

}

HI_S32 vapi_vdec_chn_destory(HI_S32 VdChn)
{
//    HI_UNF_AVPLAY_STOP_OPT_S stStop;
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
    
//    stStop.enMode = HI_UNF_AVPLAY_STOP_MODE_BLACK;
//    stStop.u32TimeoutMs = 0;
//    s32Ret = HI_UNF_AVPLAY_Stop(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID, &stStop);
//    if (HI_SUCCESS != s32Ret)
//    {
//        VAPILOG("HI_UNF_AVPLAY_Stop[%d] failed with %#x!\n", VdChn, s32Ret);
//        return HI_FAILURE;
//    } 
    
    s32Ret = HI_UNF_AVPLAY_ChnClose(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_ChnClose[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_AVPLAY_Destroy(g_stVdec.hAvplay[VdChn]);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_Destroy[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    
    g_stVdec.stChn[VdChn].enState = STATE_IDLE;    
    g_stVdec.stChn[VdChn].stFormat.width = 0;
    g_stVdec.stChn[VdChn].stFormat.height = 0;
//    g_stVdec.stChn[VdChn].used = 0;
    g_stVdec.hAvplay[VdChn] = 0;
    
    if (g_stVdec.stCallback.update_chn_cb != NULL)
    {
        g_stVdec.stCallback.update_chn_cb(-1, VdChn, STATE_IDLE, 0);
    }
    
    return HI_SUCCESS;
}

HI_VOID vapi_vdec_update_state(HI_S32 VdChn, HI_S32 state)
{
    g_stVdec.stChn[VdChn].enState = state;
}

HI_BOOL vapi_vdec_check_state(HI_S32 VdChn, HI_S32 state)
{
    return (g_stVdec.stChn[VdChn].enState == state);
}

HI_HANDLE vapi_vdec_get_chn_handle(HI_S32 VdChn)
{
    return g_stVdec.hAvplay[VdChn];
}

HI_S32 vapi_vdec_send_frame(HI_S32 VdChn, HI_U8* pu8Addr, HI_U32 u32Len, HI_U64 u64PTS)
{        
    HI_UNF_STREAM_BUF_S stStreamBuf;
    HI_S32 s32Ret;

    if (!pu8Addr || !u32Len)
    {
        VAPILOG("pu8Addr[%p] u32Len[%d]bad params!\n", pu8Addr, u32Len);
        return HI_FAILURE;
    }
        
    memset(&stStreamBuf, 0, sizeof(HI_UNF_STREAM_BUF_S));
    s32Ret = HI_UNF_AVPLAY_GetBuf(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_BUF_ID_ES_VID, u32Len, &stStreamBuf, 0);
    if (HI_SUCCESS == s32Ret)
    {
        if (stStreamBuf.pu8Data)
            memcpy(stStreamBuf.pu8Data, pu8Addr, u32Len);
        else
            VAPILOG("HI_UNF_AVPLAY_GetBuf pu8Data is NULL\n");
        s32Ret = HI_UNF_AVPLAY_PutBuf(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_BUF_ID_ES_VID, u32Len, u64PTS/1000);
        if (HI_SUCCESS != s32Ret)
        {
            VAPILOG("HI_UNF_AVPLAY_PutBuf [%d] with %#x!\n", VdChn, s32Ret);
            return HI_FAILURE;
        }
    }
    else
    {
        if (HI_ERR_VDEC_BUFFER_FULL == s32Ret)
        {
            HI_UNF_AVPLAY_RESET_OPT_S stResetOpt;
            stResetOpt.u32SeekPtsMs = -1;
            s32Ret = HI_UNF_AVPLAY_Reset(g_stVdec.hAvplay[VdChn], &stResetOpt);
            if (HI_SUCCESS != s32Ret)
            {
                VAPILOG("HI_UNF_AVPLAY_Reset [%d] with %#x!\n", VdChn, s32Ret);
            }
            else
                return HI_FAILURE;
        }
        
        VAPILOG("HI_UNF_AVPLAY_GetBuf [%d] with %#x!\n", VdChn, s32Ret);
        return s32Ret;
    }
    
    return HI_SUCCESS;
}


HI_S32 vapi_vdec_start_stream(HI_S32 VdChn)
{
    HI_S32 s32Ret;
    
    s32Ret = HI_UNF_AVPLAY_Start(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID, HI_NULL);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_Start[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    }
    if (g_stVdec.stCallback.start_stream_cb != NULL)
        g_stVdec.stCallback.start_stream_cb(VdChn);

    
    return HI_SUCCESS;
}

HI_S32 vapi_vdec_stop_stream(HI_S32 VdChn)
{
    HI_UNF_AVPLAY_STOP_OPT_S stStop;
    HI_S32 s32Ret;
    
    if (g_stVdec.stCallback.stop_stream_cb != NULL)
        g_stVdec.stCallback.stop_stream_cb(VdChn);
        
    stStop.enMode = HI_UNF_AVPLAY_STOP_MODE_BLACK;
    stStop.u32TimeoutMs = 0;
    s32Ret = HI_UNF_AVPLAY_Stop(g_stVdec.hAvplay[VdChn], HI_UNF_AVPLAY_MEDIA_CHAN_VID, &stStop);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_Stop[%d] failed with %#x!\n", VdChn, s32Ret);
        return HI_FAILURE;
    } 

    return HI_SUCCESS;
}

HI_S32 vapi_vdec_chn_update(HI_S32 DevId, HI_S32 VdChn, STATE_E enState)
{
    if (g_stVdec.stCallback.update_chn_cb != NULL)
    {
        return g_stVdec.stCallback.update_chn_cb(DevId, VdChn, enState, 0);
    }
    return 0;
}

HI_VOID vapi_vdec_init(VDEC_CB_S *pstCb)
{
    HI_S32 i;
    HI_S32 s32Ret;
    
    for(i=0; i<MAX_CHN_NUM; i++)
    {
        g_stVdec.stChn[i].enState = STATE_IDLE;
//        g_stVdec.stChn[i].used = 0;
        g_stVdec.hAvplay[i] = 0;
    }
    g_stVdec.stCallback.start_stream_cb = pstCb->start_stream_cb;
    g_stVdec.stCallback.stop_stream_cb = pstCb->stop_stream_cb;
    g_stVdec.stCallback.update_chn_cb = pstCb->update_chn_cb;

    s32Ret = HI_UNF_AVPLAY_Init();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_Init with %#x!\n", s32Ret);
    }
}

HI_VOID vapi_vdec_uninit()
{
    HI_S32 i;
    HI_S32 s32Ret;
    
    for(i=0; i<MAX_CHN_NUM; i++)
    {
        vapi_vdec_chn_destory(i);
    }
    
    g_stVdec.stCallback.start_stream_cb = NULL;
    g_stVdec.stCallback.stop_stream_cb = NULL;
    g_stVdec.stCallback.update_chn_cb = NULL;
    
    s32Ret = HI_UNF_AVPLAY_DeInit();
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_AVPLAY_DeInit with %#x!\n", s32Ret);
    }
}

