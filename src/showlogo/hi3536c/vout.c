/* 
 * ***************************************************************
 * Filename:      	vout.c
 * Created at:    	2015.10.21
 * Description:   	video output controller
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

typedef struct hiHDMI_ARGS_S
{
    HI_HDMI_ID_E enHdmi;
    HI_HDMI_VIDEO_FMT_E eForceFmt;
}HDMI_ARGS_S;

HDMI_ARGS_S     stHdmiArgs;

static VO_INTF_SYNC_E logo_vo_get_srceen_sync(SCREEN_RES_E enRes)
{
    VO_INTF_SYNC_E enIntfSync;

    switch(enRes)
    {
        case SCREEN_1024X768_60:    
            enIntfSync = VO_OUTPUT_1024x768_60;
            break;
        case SCREEN_1280X720_60:    
            enIntfSync = VO_OUTPUT_720P60;
            break;
        case SCREEN_1280X1024_60:    
            enIntfSync = VO_OUTPUT_1024x768_60;
            break;
        case SCREEN_1600X1200_60:    
            enIntfSync = VO_OUTPUT_1600x1200_60;            
            break;
        case SCREEN_1920X1080_60:    
            enIntfSync = VO_OUTPUT_1080P60;
            break;        
        case SCREEN_3840X2160_30:
            enIntfSync = VO_OUTPUT_3840x2160_30;
            break;
        case SCREEN_3840X2160_60:    
            enIntfSync = VO_OUTPUT_3840x2160_60;           
            break;
        default:msprintf("can not support srceen resolution[%d], use default[1080P60]!", enRes);
            enIntfSync = VO_OUTPUT_1080P60;
            break;
    }
    
    return enIntfSync;    
}

static HI_S32 logo_vo_layer_get_videofmt(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W, HI_U32 *pu32H, HI_U32 *pu32Frm)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL       :  *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
        case VO_OUTPUT_NTSC      :  *pu32W = 720;  *pu32H = 480;  *pu32Frm = 30; break;        
        case VO_OUTPUT_576P50    :  *pu32W = 720;  *pu32H = 576;  *pu32Frm = 50; break;
        case VO_OUTPUT_480P60    :  *pu32W = 720;  *pu32H = 480;  *pu32Frm = 60; break;
        case VO_OUTPUT_800x600_60:  *pu32W = 800;  *pu32H = 600;  *pu32Frm = 60; break;
        case VO_OUTPUT_720P50    :  *pu32W = 1280; *pu32H = 720;  *pu32Frm = 50; break;
        case VO_OUTPUT_720P60    :  *pu32W = 1280; *pu32H = 720;  *pu32Frm = 60; break;        
        case VO_OUTPUT_1080I50   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
        case VO_OUTPUT_1080I60   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
        case VO_OUTPUT_1080P24   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 24; break;        
        case VO_OUTPUT_1080P25   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 25; break;
        case VO_OUTPUT_1080P30   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 30; break;
        case VO_OUTPUT_1080P50   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 50; break;
        case VO_OUTPUT_1080P60   :  *pu32W = 1920; *pu32H = 1080; *pu32Frm = 60; break;
        case VO_OUTPUT_1024x768_60:  *pu32W = 1024; *pu32H = 768;  *pu32Frm = 60; break;
        case VO_OUTPUT_1280x1024_60: *pu32W = 1280; *pu32H = 1024; *pu32Frm = 60; break;
        case VO_OUTPUT_1366x768_60:  *pu32W = 1366; *pu32H = 768;  *pu32Frm = 60; break;
        case VO_OUTPUT_1440x900_60:  *pu32W = 1440; *pu32H = 900;  *pu32Frm = 60; break;
        case VO_OUTPUT_1280x800_60:  *pu32W = 1280; *pu32H = 800;  *pu32Frm = 60; break;        
        case VO_OUTPUT_1600x1200_60: *pu32W = 1600; *pu32H = 1200; *pu32Frm = 60; break;
        case VO_OUTPUT_1680x1050_60: *pu32W = 1680; *pu32H = 1050; *pu32Frm = 60; break;
        case VO_OUTPUT_1920x1200_60: *pu32W = 1920; *pu32H = 1200; *pu32Frm = 60; break;
        case VO_OUTPUT_3840x2160_30: *pu32W = 3840; *pu32H = 2160; *pu32Frm = 30; break;
        case VO_OUTPUT_3840x2160_60: *pu32W = 3840; *pu32H = 2160; *pu32Frm = 60; break;
        case VO_OUTPUT_USER    :     *pu32W = 720;  *pu32H = 576;  *pu32Frm = 25; break;
        default: 
            msprintf("vo enIntfSync not support!\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_VOID logo_vo_hdmi_get_videofmt(VO_INTF_SYNC_E enIntfSync, HI_HDMI_VIDEO_FMT_E *penVideoFmt)
{
    switch (enIntfSync)
    {
        case VO_OUTPUT_PAL:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_PAL;
            break;
        case VO_OUTPUT_NTSC:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_NTSC;
            break;
        case VO_OUTPUT_1080P24:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_24;
            break;
        case VO_OUTPUT_1080P25:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_25;
            break;
        case VO_OUTPUT_1080P30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
            break;
        case VO_OUTPUT_720P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_50;
            break;
        case VO_OUTPUT_720P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_720P_60;
            break;
        case VO_OUTPUT_1080I50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_50;
            break;
        case VO_OUTPUT_1080I60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080i_60;
            break;
        case VO_OUTPUT_1080P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
            break;
        case VO_OUTPUT_1080P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
        case VO_OUTPUT_576P50:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_576P_50;
            break;
        case VO_OUTPUT_480P60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_480P_60;
            break;
        case VO_OUTPUT_800x600_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_800X600_60;
            break;
        case VO_OUTPUT_1024x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
            break;
        case VO_OUTPUT_1280x1024_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
            break;
        case VO_OUTPUT_1366x768_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1366X768_60;
            break;
        case VO_OUTPUT_1440x900_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1440X900_60;
            break;
        case VO_OUTPUT_1280x800_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X800_60;
            break;
        case VO_OUTPUT_1600x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1600X1200_60;
            break;
        case VO_OUTPUT_1920x1200_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1920X1200_60;
            break;    
        case VO_OUTPUT_3840x2160_30:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_30;
            break;
        case VO_OUTPUT_3840x2160_60:
            *penVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_60;    
            break;
        default :
            msprintf("Unknow VO_INTF_SYNC_E value[%d], use defualt[1080P@60]!\n", enIntfSync);
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
    }

}

static HI_S32 logo_vo_hdmi_unplug(HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S *)pPrivateData;
    HI_HDMI_ID_E hHdmi = pArgs->enHdmi;
    
    s32Ret = HI_MPI_HDMI_Stop(hHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_Stop failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    msprintf("HDMI has UnPlug.\n");
    return HI_SUCCESS;
}

static HI_S32 logo_vo_hdmi_hotplug(HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S*)pPrivateData;
    HI_HDMI_ID_E hHdmi = pArgs->enHdmi;
    HI_HDMI_ATTR_S stHdmiAttr;
    HI_HDMI_SINK_CAPABILITY_S stSinkCap;

    s32Ret = HI_MPI_HDMI_GetAttr(hHdmi, &stHdmiAttr);

    s32Ret = HI_MPI_HDMI_GetSinkCapability(hHdmi, &stSinkCap);

    if (HI_FALSE == stSinkCap.bConnected )
    {
        return HI_FAILURE;
    }
    if(HI_TRUE == stSinkCap.bSupportHdmi)
    {
        stHdmiAttr.bEnableHdmi = HI_TRUE;
        if(HI_TRUE != stSinkCap.bSupportYCbCr)
        {
            stHdmiAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_RGB444;
        }
    }
    else
    {
        stHdmiAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_RGB444;
        stHdmiAttr.bEnableHdmi = HI_FALSE;
    }
    if(HI_TRUE == stHdmiAttr.bEnableHdmi)
    {
        stHdmiAttr.bEnableAudio = HI_TRUE;
        stHdmiAttr.bEnableVideo = HI_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = HI_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = HI_TRUE;
        stHdmiAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
        stHdmiAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
        stHdmiAttr.bxvYCCMode = HI_FALSE;
        stHdmiAttr.bEnableAudio = HI_TRUE;
        stHdmiAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
        stHdmiAttr.bIsMultiChannel = HI_FALSE;
        stHdmiAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;
        stHdmiAttr.bDebugFlag = HI_FALSE;
        stHdmiAttr.bHDCPEnable = HI_FALSE;
        stHdmiAttr.b3DEnable = HI_FALSE;
        stHdmiAttr.enDefaultMode = HI_HDMI_FORCE_HDMI;
    }
    else
    {
        stHdmiAttr.bEnableAudio = HI_FALSE;
        stHdmiAttr.bEnableVideo = HI_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = HI_FALSE;
        stHdmiAttr.bEnableAviInfoFrame = HI_FALSE;
        stHdmiAttr.bEnableAudio = HI_FALSE;
        stHdmiAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_RGB444;
        stHdmiAttr.enDefaultMode = HI_HDMI_FORCE_DVI;
    }
    if ( pArgs->eForceFmt >= HI_HDMI_VIDEO_FMT_1080P_60
        && pArgs->eForceFmt < HI_HDMI_VIDEO_FMT_BUTT
        && stSinkCap.bVideoFmtSupported[pArgs->eForceFmt] )
    {
        stHdmiAttr.enVideoFmt = pArgs->eForceFmt;
    }
    else
    {
        stHdmiAttr.enVideoFmt = stSinkCap.enNativeVideoFormat;
    }
    
    s32Ret = HI_MPI_HDMI_SetAttr(hHdmi, &stHdmiAttr);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_SetAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }    

    s32Ret = HI_MPI_HDMI_Start(hHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_Start failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    msprintf("HDMI has HotPlug.\n");

    return s32Ret;
}

HI_VOID logo_vo_hdmi_event(HI_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)
{
    switch ( event )
    {
    case HI_HDMI_EVENT_HOTPLUG:
        logo_vo_hdmi_hotplug(pPrivateData);
        break;
    case HI_HDMI_EVENT_NO_PLUG:
        logo_vo_hdmi_unplug(pPrivateData);
        break;
    case HI_HDMI_EVENT_EDID_FAIL:
        break;
    case HI_HDMI_EVENT_HDCP_FAIL:
        break;
    case HI_HDMI_EVENT_HDCP_SUCCESS:
        break;
    case HI_HDMI_EVENT_HDCP_USERSETTING:
        break;
    default:
        msprintf("un-known event:%d\n",event);
        return;
    }
    
    return;
}

static HI_S32 logo_vo_start_hdmi(VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HDMI_VIDEO_FMT_E enVideoFmt;
    HI_HDMI_ATTR_S stAttr;
//    HI_HDMI_CALLBACK_FUNC_S stCallbackFunc;

    logo_vo_hdmi_get_videofmt(enIntfSync, &enVideoFmt);
    s32Ret = HI_MPI_HDMI_Init();
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_Init failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    stHdmiArgs.enHdmi       = HI_HDMI_ID_0;
    stHdmiArgs.eForceFmt    = enVideoFmt;

    s32Ret = HI_MPI_HDMI_Open(stHdmiArgs.enHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_Open failed with %#x!\n", s32Ret);
        HI_MPI_HDMI_DeInit();
        return HI_FAILURE;
    }

//    stCallbackFunc.pfnHdmiEventCallback = logo_vo_hdmi_event;
//    stCallbackFunc.pPrivateData         = &stHdmiArgs;
//    
//    s32Ret |= HI_MPI_HDMI_RegCallbackFunc(stHdmiArgs.enHdmi, &stCallbackFunc);
//    if (s32Ret != HI_SUCCESS)
//    {
//        msprintf("HI_MPI_HDMI_RegCallbackFunc failed with %#x!\n", s32Ret);
//        HI_MPI_HDMI_Close(stHdmiArgs.enHdmi);
//        HI_MPI_HDMI_DeInit();
//    }

    s32Ret = HI_MPI_HDMI_GetAttr(stHdmiArgs.enHdmi, &stAttr);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_GetAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }         
    stAttr.bEnableHdmi = HI_TRUE;        
    stAttr.bEnableVideo = HI_TRUE;
    stAttr.enVideoFmt = enVideoFmt;        
    stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
    stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
    stAttr.bxvYCCMode = HI_FALSE;
    stAttr.enDefaultMode = HI_HDMI_FORCE_HDMI;        
    stAttr.bEnableAudio = HI_FALSE;
    stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
    stAttr.bIsMultiChannel = HI_FALSE;        
    stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;        
    stAttr.bEnableAviInfoFrame = HI_TRUE;
    stAttr.bEnableAudInfoFrame = HI_TRUE;
    stAttr.bEnableSpdInfoFrame = HI_FALSE;
    stAttr.bEnableMpegInfoFrame = HI_FALSE;        
    stAttr.bDebugFlag = HI_FALSE;          
    stAttr.bHDCPEnable = HI_FALSE;        
    stAttr.b3DEnable = HI_FALSE;

    s32Ret = HI_MPI_HDMI_SetAttr(stHdmiArgs.enHdmi, &stAttr);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_SetAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } 

    s32Ret = HI_MPI_HDMI_Start(stHdmiArgs.enHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_Start failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }         
    return s32Ret;
}

static HI_S32 logo_vo_stop_hdmi()
{
    HI_S32 s32Ret = HI_SUCCESS;
//    HI_HDMI_CALLBACK_FUNC_S stCallbackFunc;
    
    s32Ret |= HI_MPI_HDMI_Stop(stHdmiArgs.enHdmi);    
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_Stop failed with %#x!\n", s32Ret);
    }
    
//    stCallbackFunc.pfnHdmiEventCallback = logo_vo_hdmi_event;
//    stCallbackFunc.pPrivateData         = &stHdmiArgs;
//    
//    s32Ret |= HI_MPI_HDMI_UnRegCallbackFunc(stHdmiArgs.enHdmi, &stCallbackFunc);
//    if (s32Ret != HI_SUCCESS)
//    {
//        msprintf("HI_MPI_HDMI_UnRegCallbackFunc failed with %#x!\n", s32Ret);
//    }
    s32Ret |= HI_MPI_HDMI_Close(stHdmiArgs.enHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_Close failed with %#x!\n", s32Ret);
    }

    s32Ret |= HI_MPI_HDMI_DeInit();
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_HDMI_DeInit failed with %#x!\n", s32Ret);
    }
    
    return s32Ret;
}

static HI_S32 logo_vo_start_dev(VO_DEV VoDev, VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_PUB_ATTR_S stVoPubAttr;    
    
    if (VoDev == LOGO_VO_DEV_DHD0)
    {
        stVoPubAttr.enIntfSync = enIntfSync;
        //HDMI max 3840x2160@60. VGA max  2560x1440@30
        if (enIntfSync > VO_OUTPUT_2560x1440_30)
        {        
            stVoPubAttr.enIntfType = VO_INTF_HDMI;
        }
        else
        {
            stVoPubAttr.enIntfType = VO_INTF_HDMI | VO_INTF_VGA;
        }
        s32Ret = logo_vo_start_hdmi(enIntfSync);
        if (s32Ret != HI_SUCCESS)
        {
            msprintf("dev[%d] start_hdmi failed! \n", VoDev);
            return HI_FAILURE;
        }
    }
    else if (VoDev == LOGO_VO_DEV_DHD1)
    {
        //BT1120 max 1080P@60
        stVoPubAttr.enIntfSync = LOGO_MIN(enIntfSync, VO_OUTPUT_1080P60);
        stVoPubAttr.enIntfType = VO_INTF_BT1120;
    }
    else
    {
        msprintf("dev[%d] don't support! \n", VoDev);
        return HI_FAILURE;
    }
    stVoPubAttr.u32BgColor = 0x00000000;
    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, &stVoPubAttr);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_SetPubAttr[%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_Enable [%d]failed with %#x!\n",VoDev, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

static HI_S32 logo_vo_stop_dev(VO_DEV VoDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    if (VoDev == LOGO_VO_DEV_DHD0)
    {
        s32Ret = logo_vo_stop_hdmi();
        if (s32Ret != HI_SUCCESS)
        {
            msprintf("logo_vo_stop_hdmi failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    
    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_Disable [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 logo_vo_chn_create(VO_LAYER VoLayer, VO_CHN VoChn, RECT_S *pstRect)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_CHN_ATTR_S stChnAttr;        

    stChnAttr.stRect.s32X       = LOGO_ALIGN_BACK(pstRect->s32X, 2);
    stChnAttr.stRect.s32Y       = LOGO_ALIGN_BACK(pstRect->s32Y, 2);
    stChnAttr.stRect.u32Width   = LOGO_ALIGN_BACK(pstRect->u32Width, 2);
    stChnAttr.stRect.u32Height  = LOGO_ALIGN_BACK(pstRect->u32Height, 2);
    stChnAttr.u32Priority       = 0;
    stChnAttr.bDeflicker        = HI_FALSE;

    s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, VoChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_SetChnAttr [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }

    
    s32Ret = HI_MPI_VO_EnableChn(VoLayer, VoChn);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_EnableChn [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

static HI_S32 logo_vo_chn_destory(VO_LAYER VoLayer, VO_CHN VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_DisableChn(VoLayer, VoChn);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_DisableChn [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

static HI_S32 logo_vo_start_layer(VO_LAYER VoLayer, VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    s32Ret = logo_vo_layer_get_videofmt(enIntfSync, &stLayerAttr.stDispRect.u32Width, 
                &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("logo_vo_layer_get_videofmt [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }

    stLayerAttr.bClusterMode = HI_FALSE;
    stLayerAttr.stDispRect.s32X = 0;
    stLayerAttr.stDispRect.s32Y = 0;
    stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_SetVideoLayerAttr [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_EnableVideoLayer [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = logo_vo_chn_create(VoLayer, DIS_CHN_ID, &stLayerAttr.stDispRect);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("logo_vo_chn_create [%d][%d] failed with %#x!\n", VoLayer, DIS_CHN_ID, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

static HI_S32 logo_vo_stop_layer(VO_LAYER VoLayer)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    s32Ret = logo_vo_chn_destory(VoLayer, DIS_CHN_ID);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("logo_vo_chn_destory [%d][%d] failed with %#x!\n", VoLayer, DIS_CHN_ID, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_VO_DisableVideoLayer [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }
    
    return s32Ret;
}

HI_S32 logo_vo_bind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_SYS_Bind [%d->%d] failed with %#x!\n", VpssGrp, VoChn, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_S32 logo_vo_unbind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    
    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_VOU;
    stDestChn.s32DevId = VoLayer;
    stDestChn.s32ChnId = VoChn;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("HI_MPI_SYS_UnBind [%d->%d] failed with %#x!\n", VpssGrp, VoChn, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}


HI_S32 logo_vo_dev_open(VO_DEV VoDev, SCREEN_RES_E enRes)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_LAYER VoLayer;
    VO_INTF_SYNC_E enIntfSync;

    enIntfSync = logo_vo_get_srceen_sync(enRes);    
    
    s32Ret = logo_vo_start_dev(VoDev, enIntfSync);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("logo_vo_dev_open [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }    

    VoLayer = VoDev == LOGO_VO_DEV_DHD0 ? LOGO_VO_LAYER_VHD0 : LOGO_VO_LAYER_VHD1;
    s32Ret = logo_vo_start_layer(VoLayer, enIntfSync);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("logo_vo_start_layer [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

HI_S32 logo_vo_dev_close(VO_DEV VoDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_LAYER VoLayer;

    VoLayer = VoDev == LOGO_VO_DEV_DHD0 ? LOGO_VO_LAYER_VHD0 : LOGO_VO_LAYER_VHD1;
    s32Ret = logo_vo_stop_layer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("logo_vo_stop_layer [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = logo_vo_stop_dev(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        msprintf("logo_vo_stop_dev [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }


    return HI_SUCCESS;
}


