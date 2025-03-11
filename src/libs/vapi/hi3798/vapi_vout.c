/* 
 * ***************************************************************
 * Filename:      	vapi_vout.c
 * Created at:    	2016.04.29
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
#include <sys/ioctl.h>
#include "vapi_comm.h"

/*  Video Descriptor Block  */
typedef struct HDMI_EDID_DTD_VIDEO {
	HI_U16	pixel_clock;		/* 54-55 */
	HI_U8   horiz_active;		/* 56 */
	HI_U8	horiz_blanking;		/* 57 */
	HI_U8	horiz_high;		/* 58 */
	HI_U8	vert_active;		/* 59 */
	HI_U8	vert_blanking;		/* 60 */
	HI_U8	vert_high;		/* 61 */
	HI_U8	horiz_sync_offset;	/* 62 */
	HI_U8	horiz_sync_pulse;	/* 63 */
	HI_U8	vert_sync_pulse;	/* 64 */
	HI_U8	sync_pulse_high;	/* 65 */
	HI_U8	horiz_image_size;	/* 66 */
	HI_U8	vert_image_size;	/* 67 */
	HI_U8	image_size_high;	/* 68 */
	HI_U8	horiz_border;		/* 69 */
	HI_U8	vert_border;		/* 70 */
	HI_U8	misc_settings;		/* 71 */
}HDMI_EDID_DTD_VIDEO_S;

typedef struct HDMI_EDID {
	HI_U8   header[8];		/* 00-07 */
	HI_U16	manufacturerID;		/* 08-09 */
	HI_U16	product_id;		/* 10-11 */
	HI_U32	serial_number;		/* 12-15 */
	HI_U8	week_manufactured;	/* 16 */
	HI_U8	year_manufactured;	/* 17 */
	HI_U8	edid_version;		/* 18 */
	HI_U8	edid_revision;		/* 19 */
	HI_U8	video_in_definition;	/* 20 */
	HI_U8	max_horiz_image_size;	/* 21 */
	HI_U8	max_vert_image_size;	/* 22 */
	HI_U8	display_gamma;		/* 23 */
	HI_U8	power_features;		/* 24 */
	HI_U8	chroma_info[10];	/* 25-34 */
	HI_U8	timing_1;		/* 35 */
	HI_U8	timing_2;		/* 36 */
	HI_U8	timing_3;		/* 37 */
	HI_U8	std_timings[16];	/* 38-53 */

	HDMI_EDID_DTD_VIDEO_S video[4]; /* 54-125 */

	HI_U8	extension_edid;		/* 126 */
	HI_U8	checksum;		/* 127 */
	HI_U8	extension_tag;		/* 00 (extensions follow EDID) */
	HI_U8	extention_rev;		/* 01 */
	HI_U8	offset_dtd;		/* 02 */
	HI_U8	num_dtd;		/* 03 */

	HI_U8	data_block[123];	/* 04 - 126 */
	HI_U8	extension_checksum;	/* 127 */

	HI_U8	ext_datablock[256];
} HDMI_EDID_S;


typedef struct hiHDMI_ARGS_S
{
    HI_UNF_HDMI_ID_E enHdmi;
    HI_UNF_ENC_FMT_E enFormat;
}HDMI_ARGS_S;

typedef struct adaptive_s{
    pthread_t       handle;
    pthread_mutex_t mutex;           
    HI_BOOL         isRun;
    SCREEN_RES_E    enRes[SCREEN_NUM];
}ADAPTIVE_S;

typedef struct voChn_s{
    CHNINFO_S       stChn[VAPI_VO_DEV_NUM][MAX_CHN_NUM];
    HI_HANDLE       handle[VAPI_VO_DEV_NUM][MAX_CHN_NUM];
    SCREEN_RES_E    enRes[SCREEN_NUM];
    CSC_E           enCSC[VAPI_VO_DEV_NUM];
    HDMI_ARGS_S     stHdmiArgs;
    int  (*update_screen_res_cb)(int screen, int res);
    ADAPTIVE_S      stAdapt;
    HI_BOOL         isHotplug;
    HI_BOOL         isQuick;
}VOCHN_S;

#define DEV_HDMI2VGA_NAME           "/dev/hi_vga"
#define VGA_SET_MODE_NORMAL         _IO('A', 1) 
#define VGA_SET_MODE_4k             _IO('A', 2) 
#define VGA_SOFT_RESET              _IO('A', 3)

static VOCHN_S g_stVoChn;

static HI_UNF_ENC_FMT_E vapi_vo_get_srceen_fmt(SCREEN_RES_E enRes)
{
    HI_UNF_ENC_FMT_E enFormat;

    switch(enRes)
    {
        case SCREEN_1024X768_60:    
            enFormat = HI_UNF_ENC_FMT_VESA_1024X768_60;
            break;
        case SCREEN_1280X720_60:    
            enFormat = HI_UNF_ENC_FMT_720P_60;
            break;
        case SCREEN_1280X1024_60:    
            enFormat = HI_UNF_ENC_FMT_VESA_1280X1024_60;
            break;
        case SCREEN_1600X1200_60:    
            enFormat = HI_UNF_ENC_FMT_VESA_1600X1200_60;            
            break;
        case SCREEN_1920X1080_30:    
            enFormat = HI_UNF_ENC_FMT_1080P_30;
            break;
        case SCREEN_1920X1080_50:    
            enFormat = HI_UNF_ENC_FMT_1080P_50;
            break;
        case SCREEN_1920X1080_60:    
            enFormat = HI_UNF_ENC_FMT_1080P_60;
            break;        
        case SCREEN_3840X2160_30:
            enFormat = HI_UNF_ENC_FMT_3840X2160_30;
            break;
        case SCREEN_3840X2160_60:    
            enFormat = HI_UNF_ENC_FMT_3840X2160_60;           
            break;
        default:VAPILOG("can not support srceen resolution[%d], use default[1080P60]!", enRes);
            enFormat = HI_UNF_ENC_FMT_1080P_60;
            break;
    }
    
    return enFormat;    
}

static HI_VOID vapi_vo_get_resolution_by_formt(HI_UNF_ENC_FMT_E enFormat, HI_U32 *pu32W, HI_U32 *pu32H)
{
    switch(enFormat)
    {
        case HI_UNF_ENC_FMT_VESA_1024X768_60 :  
            *pu32W = 1024;  *pu32H = 768;   
            break;
        case HI_UNF_ENC_FMT_720P_60 :  
            *pu32W = 1280;  *pu32H = 720;
            break;        
        case HI_UNF_ENC_FMT_VESA_1280X1024_60 :
            *pu32W = 1280;  *pu32H = 1024;
            break;
        case HI_UNF_ENC_FMT_VESA_1600X1200_60 :
            *pu32W = 1600;  *pu32H = 1200;
            break;
        case HI_UNF_ENC_FMT_1080P_30 :
        case HI_UNF_ENC_FMT_1080P_50 :
        case HI_UNF_ENC_FMT_1080P_60 :
            *pu32W = 1920; *pu32H = 1080;
            break;        
        case HI_UNF_ENC_FMT_3840X2160_30 :
        case HI_UNF_ENC_FMT_3840X2160_60 :
            *pu32W = 3840; *pu32H = 2160;
            break;
        default:VAPILOG("can not support srceen formt[%d], use default[1080P60]!", enFormat);
            *pu32W = 1920; *pu32H = 1080;
            break;
    }
}

static void vapi_vo_calc_w_h(SCREEN_RES_E enRes, HI_S32 *W, HI_S32 *H)
{
    switch(enRes)
    {
        case SCREEN_1024X768_60:    
            *W  = 1024;
            *H  = 768;
            break;
        case SCREEN_1280X720_60:    
            *W  = 1280;
            *H  = 720;
            break;
        case SCREEN_1280X1024_60:    
            *W  = 1280;
            *H  = 1024;
            break;
        case SCREEN_1600X1200_60:    
            *W  = 1600;
            *H  = 1200;
            break;
        case SCREEN_1920X1080_30:    
        case SCREEN_1920X1080_50:    
        case SCREEN_1920X1080_60:    
            *W  = 1920;
            *H  = 1080;
            break;        
        case SCREEN_3840X2160_30:
        case SCREEN_3840X2160_60:    
            *W  = 3840;
            *H  = 2160;            
            break;
        default:VAPILOG("can not support srceen resolution[%d]\n\n", enRes);
            *W  = 0;
            *H  = 0;
            break;
    }

}

static HI_BOOL vapi_vo_check_edid(HI_UNF_HDMI_ID_E enHdmi, HI_UNF_ENC_FMT_E enFormat)
{
    HI_U8 u8Edid[512] = {0};
    HI_U32 u32Len = 0;
    HDMI_EDID_S *pstEDID;
    HI_U32 W,H;
    HI_U32 HACT, VACT;
    
    HI_S32 s32Ret = HI_SUCCESS;
    
    s32Ret = HI_UNF_HDMI_Force_GetEDID(enHdmi, u8Edid, &u32Len);    
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Force_GetEDID failed with %#x!\n", s32Ret);
        return HI_FALSE;
    }
    
    pstEDID = (HDMI_EDID_S *)u8Edid;
    HACT = pstEDID->video[0].horiz_active + ((pstEDID->video[0].horiz_high & 0xF0) << 4);
    VACT = pstEDID->video[0].vert_active + ((pstEDID->video[0].vert_high & 0xF0) << 4);
    
    vapi_vo_get_resolution_by_formt(enFormat, &W, &H);
    VAPILOG(" HACT[%d] VACT[%d] W[%d] H[%d] \n", HACT, VACT, W, H);
    if (W<=HACT && H<=VACT)
    {
        return HI_TRUE;
    }
    else
    {
        return HI_FALSE;
    }
}

static HI_S32 vapi_vo_hdmi_to_vga(HI_UNF_ENC_FMT_E enFormat)
{
    HI_S32 fd = -1;
    HI_S32 s32Ret = HI_SUCCESS;

    // 1. set sub hdmi dev config    
    fd  = open(DEV_HDMI2VGA_NAME, O_RDWR, 0);
    if (fd != -1)
    {
        if (enFormat == HI_UNF_ENC_FMT_3840X2160_30 || enFormat == HI_UNF_ENC_FMT_3840X2160_60)
        {
            s32Ret = ioctl(fd, VGA_SET_MODE_4k, HI_NULL);
            if (s32Ret != HI_SUCCESS)
            {
                VAPILOG("VGA_SET_MODE_NORMAL failed with %#x!\n", s32Ret);
                close(fd);
                return HI_FAILURE;
            }
        }
        else
        {
            s32Ret = ioctl(fd, VGA_SET_MODE_NORMAL, HI_NULL);
            if (s32Ret != HI_SUCCESS)
            {
                VAPILOG("VGA_SET_MODE_4k failed with %#x!\n", s32Ret);
                close(fd);
                return HI_FAILURE;
            }
        }
//        s32Ret = ioctl(fd, VGA_SOFT_RESET, HI_NULL);
//        if (s32Ret != HI_SUCCESS)
//        {
//            VAPILOG("VGA_SOFT_RESET failed with %#x!\n", s32Ret);
//        }
        close(fd);
    }
    else
    {
        VAPILOG("=========open hdmi_to_vga device faliled ! =========\n\n");
    }
    return s32Ret;
}

#if 0
static HI_S32 vapi_vo_hdmi_unplug(HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S *)pPrivateData;
    HI_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;
    
    s32Ret = HI_UNF_HDMI_Stop(hHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Stop failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    VAPILOG("HDMI has UnPlug.\n");
    return HI_SUCCESS;
}
#endif

static HI_S32 vapi_vo_hdmi_hotplug(HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S*)pPrivateData;
    HI_UNF_HDMI_ID_E hHdmi = pArgs->enHdmi;
    HI_UNF_HDMI_ATTR_S stHdmiAttr;
    HI_UNF_EDID_BASE_INFO_S stSinkCap;
    HI_UNF_HDMI_STATUS_S stHdmiStatus;
    HI_S32 update_flag = 0 ;  
    SCREEN_RES_E enRes;
    HI_UNF_ENC_FMT_E fmt;
    HI_U32 w, h;

    HI_UNF_HDMI_GetStatus(hHdmi,&stHdmiStatus);
    if (HI_FALSE == stHdmiStatus.bConnected)
    {
        VAPILOG("HDMI No Connect\n");
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_HDMI_GetSinkCapability(hHdmi, &stSinkCap);
    s32Ret |= HI_UNF_HDMI_GetAttr(hHdmi, &stHdmiAttr);
    if (s32Ret == HI_SUCCESS)
    {
        if(HI_TRUE == stSinkCap.bSupportHdmi)
        {
            stHdmiAttr.bEnableHdmi = HI_TRUE;
            if(HI_TRUE != stSinkCap.stColorSpace.bYCbCr444)
            {
                stHdmiAttr.enVidOutMode = HI_UNF_HDMI_VIDEO_MODE_RGB444;
            }
            else
            {                
                stHdmiAttr.enVidOutMode = HI_UNF_HDMI_VIDEO_MODE_YCBCR444;  /* user can choicen RGB/YUV*/
            }
        }
        else
        {
            stHdmiAttr.enVidOutMode = HI_UNF_HDMI_VIDEO_MODE_RGB444;
            stHdmiAttr.bEnableHdmi = HI_FALSE;
        }
    }
    else
    {
        stHdmiAttr.bEnableHdmi = HI_FALSE;
    }
    
    if(HI_TRUE == stHdmiAttr.bEnableHdmi)
    {
        stHdmiAttr.bEnableAudio = HI_TRUE;
        stHdmiAttr.bEnableVideo = HI_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = HI_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = HI_TRUE;
    }
    else
    {
        stHdmiAttr.bEnableAudio = HI_FALSE;
        stHdmiAttr.bEnableVideo = HI_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = HI_FALSE;
        stHdmiAttr.bEnableAviInfoFrame = HI_FALSE;
        stHdmiAttr.enVidOutMode = HI_UNF_HDMI_VIDEO_MODE_RGB444;
    }
    if ( (pArgs->enFormat >= HI_UNF_ENC_FMT_1080P_60
        && pArgs->enFormat < HI_UNF_ENC_FMT_BUTT
        && stSinkCap.bSupportFormat[pArgs->enFormat])
        || vapi_vo_check_edid(hHdmi, pArgs->enFormat))
    {
        update_flag = 0;
    }
    else
    {
        update_flag = 1;
    }
    
    s32Ret = HI_UNF_HDMI_SetAttr(hHdmi, &stHdmiAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_HDMI_SetAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }    

    s32Ret = HI_UNF_HDMI_Start(hHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Start failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    if (update_flag)
    {
        for (enRes=SCREEN_RES_NUM-1; enRes>SCREEN_UNDEFINE; enRes--)
        {
            fmt = vapi_vo_get_srceen_fmt(enRes);
            if (stSinkCap.bSupportFormat[fmt])
            {
                vapi_vo_get_resolution_by_formt(fmt, &w, &h);
                VAPILOG("HDMI current screen resolution[%d][%dX%d] \n", enRes, w, h);
                g_stVoChn.stAdapt.enRes[SCREEN_MAIN] = enRes;
                break;
            }
        }
        update_flag = 0;
    }
    else
    {
        g_stVoChn.stAdapt.enRes[SCREEN_MAIN] = SCREEN_UNDEFINE;
    }

//    vapi_vo_hdmi_to_vga(pArgs->enFormat);
    
   // VAPILOG("HDMI has HotPlug.\n");

    return s32Ret;
}

static HI_VOID vapi_vo_hdmi_event(HI_UNF_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)
{

    switch ( event )
    {
        case HI_UNF_HDMI_EVENT_HOTPLUG:
//            vapi_vo_hdmi_unplug(pPrivateData);
            vapi_vo_hdmi_hotplug(pPrivateData);
            break;
        case HI_UNF_HDMI_EVENT_NO_PLUG:
//            vapi_vo_hdmi_unplug(pPrivateData);
            break;
        case HI_UNF_HDMI_EVENT_EDID_FAIL:
            break;
        case HI_UNF_HDMI_EVENT_HDCP_FAIL:
            break;
        case HI_UNF_HDMI_EVENT_HDCP_SUCCESS:
            break;
        case HI_UNF_HDMI_EVENT_RSEN_CONNECT:
            break;
        case HI_UNF_HDMI_EVENT_RSEN_DISCONNECT:
            break;
        default:
            VAPILOG("un-known event:%d\n",event);
            break;
    }

    return;
}

static HI_S32 vapi_vo_start_hdmi(HI_UNF_ENC_FMT_E enFormat)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UNF_HDMI_ATTR_S stHdmiAttr;
    HI_UNF_HDMI_DELAY_S stDelay;
    HI_UNF_HDMI_OPEN_PARA_S stOpen;
    HI_UNF_HDMI_CALLBACK_FUNC_S stCallbackFunc;

    s32Ret = HI_UNF_HDMI_Init();
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Init failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    HI_UNF_HDMI_GetDelay(0,&stDelay);
    stDelay.bForceFmtDelay = HI_TRUE;
    stDelay.bForceMuteDelay = HI_TRUE;
    stDelay.u32FmtDelay = 500;
    stDelay.u32MuteDelay = 120;
    s32Ret = HI_UNF_HDMI_SetDelay(0,&stDelay);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_HDMI_SetDelay failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    g_stVoChn.stHdmiArgs.enHdmi       = HI_UNF_HDMI_ID_0;
    g_stVoChn.stHdmiArgs.enFormat   =  enFormat;
//    g_stVoChn.isHotplug = HI_FALSE;
    
    if (1)
    {
        if (g_stVoChn.isHotplug == HI_TRUE)
        {
            stCallbackFunc.pfnHdmiEventCallback = vapi_vo_hdmi_event;
            stCallbackFunc.pPrivateData         = &g_stVoChn.stHdmiArgs;
            s32Ret |= HI_UNF_HDMI_RegCallbackFunc(g_stVoChn.stHdmiArgs.enHdmi, &stCallbackFunc);
            if (s32Ret != HI_SUCCESS)
            {
                VAPILOG("HI_UNF_HDMI_RegCallbackFunc failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
        }
        stOpen.enDefaultMode = HI_UNF_HDMI_DEFAULT_ACTION_HDMI;
        s32Ret = HI_UNF_HDMI_Open(g_stVoChn.stHdmiArgs.enHdmi, &stOpen);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_UNF_HDMI_Open failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        s32Ret = HI_UNF_HDMI_GetAttr(g_stVoChn.stHdmiArgs.enHdmi, &stHdmiAttr);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_UNF_HDMI_GetAttr failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        stHdmiAttr.bEnableHdmi = HI_TRUE;
        stHdmiAttr.bEnableAudio = HI_TRUE;
        stHdmiAttr.bEnableVideo = HI_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = HI_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = HI_TRUE;
        
        s32Ret = HI_UNF_HDMI_SetAttr(g_stVoChn.stHdmiArgs.enHdmi, &stHdmiAttr);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_UNF_HDMI_SetAttr failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        } 
        
        s32Ret = HI_UNF_HDMI_Start(g_stVoChn.stHdmiArgs.enHdmi);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_UNF_HDMI_Start failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        } 
    }
//    else
//    {
//        stCallbackFunc.pfnHdmiEventCallback = vapi_vo_hdmi_event;
//        stCallbackFunc.pPrivateData         = &g_stVoChn.stHdmiArgs;
//        s32Ret |= HI_UNF_HDMI_RegCallbackFunc(g_stVoChn.stHdmiArgs.enHdmi, &stCallbackFunc);
//        if (s32Ret != HI_SUCCESS)
//        {
//            VAPILOG("HI_UNF_HDMI_RegCallbackFunc failed with %#x!\n", s32Ret);
//            return HI_FAILURE;
//        }

//        stOpen.enDefaultMode = HI_UNF_HDMI_DEFAULT_ACTION_HDMI;
//        s32Ret = HI_UNF_HDMI_Open(g_stVoChn.stHdmiArgs.enHdmi, &stOpen);
//        if (s32Ret != HI_SUCCESS)
//        {
//            VAPILOG("HI_UNF_HDMI_Open failed with %#x!\n", s32Ret);
//            return HI_FAILURE;
//        }
//    }
    //zbing add for 5016@3798c hdmi to vga 2016.10.26
    s32Ret = vapi_vo_hdmi_to_vga(enFormat);

    return s32Ret;
}

static HI_S32 vapi_vo_stop_hdmi()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UNF_HDMI_CALLBACK_FUNC_S stCallbackFunc;
    
    s32Ret = HI_UNF_HDMI_Stop(g_stVoChn.stHdmiArgs.enHdmi);    
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Stop failed with %#x!\n", s32Ret);
    }

    s32Ret |= HI_UNF_HDMI_Close(g_stVoChn.stHdmiArgs.enHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Close failed with %#x!\n", s32Ret);
    }
    
    if (g_stVoChn.isHotplug == HI_TRUE)
    {
        stCallbackFunc.pfnHdmiEventCallback = vapi_vo_hdmi_event;
        stCallbackFunc.pPrivateData         = &g_stVoChn.stHdmiArgs;
        
        s32Ret |= HI_UNF_HDMI_UnRegCallbackFunc(g_stVoChn.stHdmiArgs.enHdmi, &stCallbackFunc);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_HDMI_UnRegCallbackFunc failed with %#x!\n", s32Ret);
        }
    }
    
    s32Ret |= HI_UNF_HDMI_DeInit();
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_DeInit failed with %#x!\n", s32Ret);
    }
    
    return s32Ret;
}

static void vapi_vo_start_hdmi_sub(HI_UNF_ENC_FMT_E enFormat)
{

}

static HI_S32 vapi_vo_stop_hdmi_sub()
{
    return HI_SUCCESS;
}

static void vapi_vo_hdmi_sub_check_edid(SCREEN_RES_E enRes)
{

}

static HI_S32 vapi_vo_start_dev(DEV_E VoDev, HI_UNF_ENC_FMT_E enFormat)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UNF_DISP_INTF_S stDispIntf;
    HI_UNF_DISP_OFFSET_S stOffset;
    HI_UNF_DISP_BG_COLOR_S stBgcolor;
    HI_UNF_DISP_E enDisp;
    HI_U32 u32W;
    HI_U32 u32H;

    if (VoDev == VAPI_VO_DEV_DHD)
    {
        stDispIntf.enIntfType       = HI_UNF_DISP_INTF_TYPE_HDMI;
        stDispIntf.unIntf.enHdmi    = HI_UNF_HDMI_ID_0;
        enDisp                      = HI_UNF_DISPLAY1;
    }
    else if (VoDev == VAPI_VO_DEV_DSD)
    {
        return HI_SUCCESS;//hi3798 no support sub screen so far
        stDispIntf.enIntfType           = HI_UNF_DISP_INTF_TYPE_CVBS;
        stDispIntf.unIntf.stCVBS.u8Dac  = 0;
        enDisp                          = HI_UNF_DISPLAY0;

    }
    else
    {
        VAPILOG("dev[%d] don't support! \n", VoDev);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_DISP_AttachIntf(enDisp, &stDispIntf, 1);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_DISP_AttachIntf [%d]failed with %#x!\n", VoDev, s32Ret);
        return s32Ret;
    }
    
    s32Ret = HI_UNF_DISP_SetFormat(enDisp, enFormat);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_DISP_SetFormat[%d] enfmt[%d]failed with %#x!\n", VoDev, enFormat, s32Ret);
        return HI_FAILURE;
    }

    vapi_vo_get_resolution_by_formt(enFormat, &u32W, &u32H);
    s32Ret = HI_UNF_DISP_SetVirtualScreen(enDisp, u32W, u32H);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_DISP_SetVirtualScreen[%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    memset(&stOffset, 0, sizeof(HI_UNF_DISP_OFFSET_S));
    s32Ret = HI_UNF_DISP_SetScreenOffset(enDisp, &stOffset);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_DISP_SetScreenOffset[%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    memset(&stBgcolor, 0, sizeof(HI_UNF_DISP_BG_COLOR_S));
    s32Ret = HI_UNF_DISP_SetBgColor(enDisp, &stBgcolor);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_DISP_SetBgColor[%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_DISP_Open(enDisp);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_DISP_Open[%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    // config HDMI0&VGA0 device
    if (VoDev == VAPI_VO_DEV_DHD)
    {
        vapi_vo_start_hdmi(enFormat);
    }
    // config HDMI1&VGA1 device
    else if (VoDev == VAPI_VO_DEV_DSD)
    {
        vapi_vo_start_hdmi_sub(enFormat);
    }
    
    return HI_SUCCESS;
}

static HI_S32 vapi_vo_stop_dev(DEV_E VoDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    if (VoDev == VAPI_VO_DEV_DHD)
    {
        s32Ret = HI_UNF_DISP_Close(HI_UNF_DISPLAY1);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_UNF_DISP_Close failed with %#x!\n", s32Ret);
            return s32Ret;
        }
        
        s32Ret = vapi_vo_stop_hdmi();
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("vapi_vo_stop_hdmi failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    else if (VoDev == VAPI_VO_DEV_DSD)
    {
        return HI_SUCCESS;//hi3798 no support sub screen so far
        s32Ret = HI_UNF_DISP_Close(HI_UNF_DISPLAY0);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_UNF_DISP_Close failed with %#x!\n", s32Ret);
            return s32Ret;
        }
        
        s32Ret = vapi_vo_stop_hdmi_sub();
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("vapi_vo_stop_hdmi_sub failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

static void *vapi_vo_adaptive_pthread(void *arg)
{
    HI_S32 i;

    comm_task_set_name("adaptive");

    while(g_stVoChn.stAdapt.isRun)
    {
        for (i=0; i<SCREEN_NUM; i++)
        {
            if (g_stVoChn.stAdapt.enRes[i] != SCREEN_UNDEFINE)
            {
                if (g_stVoChn.update_screen_res_cb != NULL)
                {
                    g_stVoChn.update_screen_res_cb(i, g_stVoChn.stAdapt.enRes[i]);
                    g_stVoChn.stAdapt.enRes[i] = SCREEN_UNDEFINE;
                }
            }
        }
        vapi_vo_hdmi_sub_check_edid(g_stVoChn.enRes[SCREEN_SUB]);
        usleep(500000);
    }
    
    return NULL;    
}

static HI_S32 vapi_vo_adaptive_start()
{
    HI_S32 s32Ret = HI_SUCCESS;

    g_stVoChn.stAdapt.isRun = HI_TRUE;
    
    s32Ret = comm_task_create_join(&g_stVoChn.stAdapt.handle, vapi_vo_adaptive_pthread, NULL);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("create vapi_vo_adaptive_pthread failed! \n");
    }

    return s32Ret;
}

static HI_S32 vapi_vo_adaptive_stop()
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    g_stVoChn.stAdapt.isRun = HI_FALSE;
    
    s32Ret = comm_task_join(&g_stVoChn.stAdapt.handle);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("destroy vapi_vo_adaptive_pthread failed! \n");
    }

    return s32Ret;
}

HI_VOID vapi_vo_get_srceen_res(SCREEN_E enScreen, HI_S32 *W, HI_S32 *H)
{
    SCREEN_RES_E enRes;
    
    enRes = g_stVoChn.enRes[enScreen];
    vapi_vo_calc_w_h(enRes, W, H);
}

HI_S32 vapi_vo_set_csc(DEV_E VoDev, CSC_E enCSC)
{
    // TODO: 
    switch(enCSC)
    {
        case CSC_STANDARD:
            break;
        case CSC_LIGHTNESS:
            break;
        case CSC_SOFT:
            break;
        case CSC_COLORFUL:
            break;
        default:
            break;
    }    

    g_stVoChn.enCSC[VoDev] = enCSC;
    
    return HI_SUCCESS;
    
}

HI_S32 vapi_vo_chn_create(DEV_E VoDev, HI_S32 VoChn, HI_RECT_S *pstRect, RATIO_E enRatio)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UNF_WINDOW_ATTR_S stWinAttr;
    HI_UNF_DISP_E enDisp;

    if (g_stVoChn.stChn[VoDev][VoChn].enState == STATE_BUSY)
    {
        VAPILOG("vout channel[%d][%d] has existed !\n", VoDev, VoChn);
        return HI_FAILURE;
    }

    enDisp = VoDev == VAPI_VO_DEV_DHD? HI_UNF_DISPLAY1:HI_UNF_DISPLAY0;
    memset(&stWinAttr, 0, sizeof(HI_UNF_WINDOW_ATTR_S));
    stWinAttr.enDisp = enDisp;
    stWinAttr.bVirtual = HI_FALSE;
    stWinAttr.stWinAspectAttr.enAspectCvrs = enRatio;
    stWinAttr.stWinAspectAttr.bUserDefAspectRatio = HI_FALSE;
    stWinAttr.stWinAspectAttr.u32UserAspectWidth  = 0;
    stWinAttr.stWinAspectAttr.u32UserAspectHeight = 0;
    stWinAttr.bUseCropRect = HI_FALSE;
    stWinAttr.stInputRect.s32X = 0;
    stWinAttr.stInputRect.s32Y = 0;
    stWinAttr.stInputRect.s32Width = 0;
    stWinAttr.stInputRect.s32Height = 0;
    
    stWinAttr.stOutputRect.s32X = ALIGN_BACK(pstRect->s32X, 2);
    stWinAttr.stOutputRect.s32Y = ALIGN_BACK(pstRect->s32Y, 2);
    stWinAttr.stOutputRect.s32Width  = ALIGN_BACK(pstRect->s32Width, 4);
    stWinAttr.stOutputRect.s32Height = ALIGN_BACK(pstRect->s32Height, 4);

//    printf("===x %d, y %d, w %d, h %d\n", stWinAttr.stOutputRect.s32X, stWinAttr.stOutputRect.s32Y,
//    stWinAttr.stOutputRect.s32Width,stWinAttr.stOutputRect.s32Height);
    s32Ret = HI_UNF_VO_CreateWindow(&stWinAttr, &g_stVoChn.handle[VoDev][VoChn]);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_CreateWindow [%d][%d] failed with %#x!\n", VoDev, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    g_stVoChn.stChn[VoDev][VoChn].enState = STATE_BUSY;
    g_stVoChn.stChn[VoDev][VoChn].stZone.x = pstRect->s32X;
    g_stVoChn.stChn[VoDev][VoChn].stZone.y = pstRect->s32Y;
    g_stVoChn.stChn[VoDev][VoChn].stZone.w = pstRect->s32Width;
    g_stVoChn.stChn[VoDev][VoChn].stZone.h = pstRect->s32Height;
    
    return HI_SUCCESS;
}

HI_S32 vapi_vo_chn_destory(DEV_E VoDev, HI_S32 VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (g_stVoChn.stChn[VoDev][VoChn].enState != STATE_BUSY)
    {
        return HI_SUCCESS;
    }

    s32Ret = HI_UNF_VO_DestroyWindow(g_stVoChn.handle[VoDev][VoChn]);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_DestroyWindow [%d][%d] failed with %#x!\n", VoDev, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    g_stVoChn.stChn[VoDev][VoChn].enState = STATE_IDLE;
    
    return HI_SUCCESS;
}

HI_S32  vapi_vo_chn_clear_buff(DEV_E VoDev, HI_S32 VoChn)
{
    return HI_SUCCESS;
}

HI_VOID vapi_vo_dsp_quick(HI_BOOL bQuick)
{
    g_stVoChn.isQuick = bQuick;
}

HI_S32 vapi_vo_bind_vdec(DEV_E VoDev, HI_S32 VoChn, HI_S32 VdecChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE handle = g_stVoChn.handle[VoDev][VoChn];
//    HI_UNF_WINDOW_FREEZE_MODE_E enWinFreezeMode;
//    HI_UNF_AVPLAY_LOW_DELAY_ATTR_S stLowDelay;
    
    s32Ret = HI_UNF_VO_AttachWindow(handle, VdecChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_AttachWindow [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
//    enWinFreezeMode = HI_UNF_WINDOW_FREEZE_MODE_BLACK;
//    s32Ret = HI_UNF_VO_ResetWindow(handle, enWinFreezeMode);
//    if (s32Ret != HI_SUCCESS)
//    {
//        VAPILOG("HI_UNF_VO_ResetWindow [%d][%d] failed with %#x!\n", VoDev, VoChn, s32Ret);
//        return HI_FAILURE;
//    }
    s32Ret = HI_UNF_VO_SetWindowEnable(handle, HI_TRUE);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_SetWindowEnable [%d][%d] failed with %#x!\n", VoDev, VoChn, s32Ret);
        return HI_FAILURE;
    }

//    s32Ret = HI_UNF_AVPLAY_GetAttr(VdecChn, HI_UNF_AVPLAY_ATTR_ID_LOW_DELAY, &stLowDelay);
//    stLowDelay.bEnable = HI_TRUE;
//    s32Ret |= HI_UNF_AVPLAY_SetAttr(VdecChn, HI_UNF_AVPLAY_ATTR_ID_LOW_DELAY, &stLowDelay);
//    if (HI_SUCCESS != s32Ret)
//    {
//        VAPILOG("HI_UNF_AVPLAY_SetAttr[%d] failed with %#x!\n", VoDev, s32Ret);
//        return HI_FAILURE;
//    }

    s32Ret = HI_UNF_VO_SetQuickOutputEnable(handle, g_stVoChn.isQuick);
    if (HI_SUCCESS != s32Ret)
    {
        VAPILOG("HI_UNF_VO_SetQuickOutputEnable[%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

HI_S32 vapi_vo_unbind_vdec(DEV_E VoDev, HI_S32 VoChn, HI_S32 VdecChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HANDLE handle = g_stVoChn.handle[VoDev][VoChn];
    
    s32Ret = HI_UNF_VO_SetWindowEnable(handle, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_SetWindowDisable [%d] handle[%d]failed with %#x!\n", VoDev, handle, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_VO_DetachWindow(handle, VdecChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_DetachWindow [%d] handle[%d]failed with %#x!\n", VoDev, handle, s32Ret);
        return HI_FAILURE;
    }

    return  HI_SUCCESS;
}


HI_VOID vapi_vo_update_state(DEV_E VoDev, HI_S32 VoChn, HI_S32 state)
{
   g_stVoChn.stChn[VoDev][VoChn].enState = state;
}

HI_BOOL vapi_vo_check_state(DEV_E VoDev, HI_S32 VoChn, HI_S32 state)
{
    return (g_stVoChn.stChn[VoDev][VoChn].enState == state);
}

HI_S32	vapi_vo_set_ratio(DEV_E VoDev, HI_S32 VoChn, RATIO_E enRatio)
{
	HI_UNF_WINDOW_ATTR_S stWinAttr;
	HI_HANDLE handle = g_stVoChn.handle[VoDev][VoChn];
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = HI_UNF_VO_GetWindowAttr(handle, &stWinAttr);
	if (s32Ret != HI_SUCCESS)
	{
		VAPILOG("HI_UNF_VO_GetWindowAttr [%d] failed with %#x!\n", VoDev, s32Ret);
		return HI_FAILURE;
	}
	switch(enRatio)
	{
		case RATIO_VO_AUTO:
			stWinAttr.stWinAspectAttr.enAspectCvrs = HI_UNF_VO_ASPECT_CVRS_LETTERBOX;
			break;
		case RATIO_VO_FULL:
		default:
			stWinAttr.stWinAspectAttr.enAspectCvrs = HI_UNF_VO_ASPECT_CVRS_IGNORE;
			break;
	}
	s32Ret = HI_UNF_VO_SetWindowAttr(handle, &stWinAttr);
	if (s32Ret != HI_SUCCESS)
	{
		VAPILOG("HI_UNF_VO_SetWindowAttr [%d] failed with %#x!\n", VoDev, s32Ret);
		return HI_FAILURE;
	} 
	return HI_SUCCESS;
}


HI_S32 vapi_vo_set_crop(DEV_E VoDev, HI_S32 VoChn, HI_RECT_S *pstRect)
{
    HI_UNF_WINDOW_ATTR_S stWinAttr;
    HI_HANDLE handle = g_stVoChn.handle[VoDev][VoChn];
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_UNF_VO_GetWindowAttr(handle, &stWinAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_GetWindowAttr [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
    
    stWinAttr.stInputRect.s32X      = ALIGN_BACK(pstRect->s32X, 8);
    stWinAttr.stInputRect.s32Y      = ALIGN_BACK(pstRect->s32Y, 8);
    stWinAttr.stInputRect.s32Width  = ALIGN_BACK(pstRect->s32Width, 8);
    stWinAttr.stInputRect.s32Height = ALIGN_BACK(pstRect->s32Height, 8);
    s32Ret = HI_UNF_VO_SetWindowAttr(handle, &stWinAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_SetWindowAttr [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    } 
    return HI_SUCCESS;
}

HI_S32 vapi_vo_snap_jpeg(DEV_E VoDev, HI_S32 VoChn, VAPI_JPEG_S *pstJpeg)
{
    HI_UNF_VIDEO_FRAME_INFO_S   stCapPicture = {0};
    HI_HANDLE handle = g_stVoChn.handle[VoDev][VoChn];
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_UNF_VO_CapturePicture(handle, &stCapPicture);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_CapturePicture [%d][%d] failed with %#x!\n", VoDev, VoChn, s32Ret);
        return HI_FAILURE;
    }

    pstJpeg->enType = ENC_TYPE_MJPEG;
    pstJpeg->u32Qfactor = 50;

    s32Ret = vapi_yuv2jpeg(&stCapPicture, (HI_U8 **)&pstJpeg->pu8InAddr, &pstJpeg->u32InLen,
                      &pstJpeg->u32InWidth, &pstJpeg->u32InHeight);
    if (s32Ret == HI_SUCCESS && 0) //debug
    {
        FILE *pFile = fopen("/home/jpg.jpg", "wb");
        fwrite(pstJpeg->pu8InAddr, pstJpeg->u32InLen, 1, pFile);
        fclose(pFile);
    }
    
    HI_UNF_VO_CapturePictureRelease(handle, &stCapPicture);
    return s32Ret;
}

HI_S32 vapi_vo_dev_open(DEV_E VoDev, SCREEN_RES_E enRes)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_UNF_ENC_FMT_E enFormat;

    g_stVoChn.stAdapt.enRes[VoDev]= SCREEN_UNDEFINE;

    enFormat = vapi_vo_get_srceen_fmt(enRes);    
    s32Ret = vapi_vo_start_dev(VoDev, enFormat);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_dev_open [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = vapi_vo_set_csc(VoDev, g_stVoChn.enCSC[VoDev]);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_set_csc [%d] failed with %#x!\n", g_stVoChn.enCSC[VoDev], s32Ret);
        return HI_FAILURE;
    }
    
    g_stVoChn.enRes[VoDev] = enRes;
    
    return HI_SUCCESS;
}

HI_S32 vapi_vo_dev_close(DEV_E VoDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    s32Ret = vapi_vo_stop_dev(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_stop_dev [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    g_stVoChn.enRes[VoDev] = SCREEN_UNDEFINE;

    return HI_SUCCESS;
}

HI_S32 vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, HI_S32 isHotplug, HI_BOOL isQuick, VDEC_CB_S *pstCb)
{
    HI_S32 s32Ret = HI_SUCCESS;

    g_stVoChn.update_screen_res_cb = pstCb->update_screen_res_cb;
    g_stVoChn.isHotplug = isHotplug;
    g_stVoChn.isQuick = isQuick;
    s32Ret = HI_UNF_DISP_Init();
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_DISP_Init failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_UNF_VO_Init(HI_UNF_VO_DEV_MODE_NORMAL);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_UNF_VO_Init failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    
    s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_DHD, enMain);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }

    s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_DSD, enSub);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }
    
    if (g_stVoChn.isHotplug == HI_TRUE)
    {
        s32Ret = vapi_vo_adaptive_start();
        if (s32Ret != HI_SUCCESS)
        {
            return HI_FAILURE;
        }
    }

    return HI_SUCCESS;
}

HI_VOID vapi_vo_uninit()
{
    vapi_vo_dev_close(VAPI_VO_DEV_DHD);
    vapi_vo_dev_close(VAPI_VO_DEV_DSD);
    HI_UNF_VO_DeInit();
    HI_UNF_DISP_DeInit();
    
    if (g_stVoChn.isHotplug == HI_TRUE)
    {
        vapi_vo_adaptive_stop();
        g_stVoChn.isHotplug = HI_FALSE;
    }
    
    g_stVoChn.update_screen_res_cb = NULL;
}


