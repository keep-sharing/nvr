/* 
 * ***************************************************************
 * Filename:      	vapi_vout.c
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
#include <sys/ioctl.h>
#include "vapi_comm.h"

#define DEV_SUB_HDMI_NAME    "/dev/sil9024"
#define DEV_SUB_VGA_NAME     "/dev/ths8200"

typedef struct ths8200_reg
{
	unsigned char reg;
	unsigned char value;
}the8200_reg_t;

typedef enum ths_mode
{
	THS8200_1920_1080_60Hz_148M = 0,
	THS8200_1920_1080_50Hz_148M,
	THS8200_640_480_60Hz_25M,
	THS8200_800_600_60Hz_40M,
	THS8200_848_480_60Hz_33M,
	THS8200_1024_768_60Hz_65M,
	THS8200_1280_768_60Hz_68M,
	THS8200_1280_768_60Hz_79M,
	THS8200_1280_800_60Hz_71M,
	THS8200_1280_800_60Hz_83M,
	THS8200_1280_960_60Hz_108M,
	THS8200_1280_1024_60Hz_108M,
	THS8200_1360_768_60Hz_85M,
	THS8200_1440_1050_60Hz_101M,
	THS8200_1440_1050_60Hz_121M,
	THS8200_1440_900_60Hz_88M,
	THS8200_1440_900_60Hz_106M,
	THS8200_1600_1200_60Hz_162M,
	THS8200_1600_1050_60Hz_119M,
	THS8200_1600_1050_60Hz_146M,
	THS8200_1920_1200_60Hz_154M,
	THS8200_1366_768_60Hz_85M,
	THS8200_1600_900_60Hz_108M,
	THS8200_2048_1152_60Hz_162M,
	THS8200_1280_720_60Hz_74M,
	THS8200_1368_768_60Hz_72M,
	THS8200_1280_720_60Hz_74M_37K,
}ths_mode_e;

typedef enum sil_mode{
    HDMI_BEGIN = 0,
    HDMI_480I60_4X3 = 1,
    HDMI_576I50_4X3,
    HDMI_480P60_4X3,
    HDMI_576P50_4X3,
    HDMI_720P60,
    HDMI_720P50,
    HDMI_1080I60,
    HDMI_1080I50,
    HDMI_1080P60,
    HDMI_1080P50,
    HDMI_1080P25,
    HDMI_1080P30,
    HDMI_1024_768_60,
    HDMI_1280_1024_60,
    HDMI_800_600_60,
    HDMI_1080P24,
    HDMI_END,
}sil_mode_e;

typedef struct sil_edid{
    int     isValid;
    char    data[512];
}sil_edid_s;


/*
 * ioctl cmd
 */
#define THS8200_IOC_MAGIC		    't'
#define THS8200_GET_ALL_REG			_IO(THS8200_IOC_MAGIC, 1)
#define THS8200_GET_SINGLE_REG		_IOWR(THS8200_IOC_MAGIC, 2, struct ths8200_reg)
#define THS8200_SET_SINGLE_REG		_IOWR(THS8200_IOC_MAGIC, 3, struct ths8200_reg)
#define THS8200_CHOICE_MODE			_IOWR(THS8200_IOC_MAGIC, 4, enum ths_mode)

#define SIL9024_IOC_MAGIC		    's'
#define SIL9024_GET_EDID			_IOWR(SIL9024_IOC_MAGIC, 1, struct sil_edid)
#define SIL9024_GET_SINGLE_REG		_IOWR(SIL9024_IOC_MAGIC, 2, 0)
#define SIL9024_SET_TIMER_POLL		_IOWR(SIL9024_IOC_MAGIC, 3, int)
#define SIL9024_CHOICE_MODE			_IOWR(SIL9024_IOC_MAGIC, 4, enum sil_mode)

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
    HI_HDMI_ID_E enHdmi;
    HI_HDMI_VIDEO_FMT_E eForceFmt;
    VO_INTF_SYNC_E      enIntfSync;
}HDMI_ARGS_S;

typedef struct adaptive_s{
    pthread_t       handle;
    pthread_mutex_t mutex;           
    HI_BOOL         isRun;
    SCREEN_RES_E    enRes[SCREEN_NUM];
}ADAPTIVE_S;

typedef struct voChn_s{
    CHNINFO_S       stChn[VAPI_VO_LAYER_NUM][MAX_CHN_NUM];
    SCREEN_RES_E    enRes[SCREEN_NUM];
    CSC_E           enCSC[VAPI_VO_LAYER_NUM];
    HDMI_ARGS_S     stHdmiArgs;
    SCREEN_E        enPipBind;
    int  (*update_screen_res_cb)(int screen, int res);
    ADAPTIVE_S      stAdapt;
    HI_BOOL         isHotplug;
    HI_BOOL         isDualScreen;
    pthread_mutex_t mutex;  
    
}VOCHN_S;

static VOCHN_S g_stVoChn;

static VO_INTF_SYNC_E vapi_vo_get_srceen_sync(SCREEN_RES_E enRes)
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
            enIntfSync = VO_OUTPUT_1280x1024_60;
            break;
        case SCREEN_1600X1200_60:    
            enIntfSync = VO_OUTPUT_1600x1200_60;            
            break;
        case SCREEN_1920X1080_30:    
            enIntfSync = VO_OUTPUT_1080P30;
            break;
        case SCREEN_1920X1080_50:    
            enIntfSync = VO_OUTPUT_1080P50;
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
        default:VAPILOG("can not support srceen resolution[%d], use default[1080P60]!", enRes);
            enIntfSync = VO_OUTPUT_1080P60;
            break;
    }
    
    return enIntfSync;    
}

static HI_HDMI_VIDEO_FMT_E vapi_vo_get_srceen_hdmi(SCREEN_RES_E enRes)
{
    HI_HDMI_VIDEO_FMT_E enVideoFmt;

    switch(enRes)
    {
        case SCREEN_1024X768_60:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1024X768_60;
            break;
        case SCREEN_1280X720_60:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X720_60;
            break;
        case SCREEN_1280X1024_60:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1280X1024_60;
            break;
        case SCREEN_1600X1200_60:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_VESA_1600X1200_60;            
            break;
        case SCREEN_1920X1080_30:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_30;
            break;
        case SCREEN_1920X1080_50:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_50;
            break;
        case SCREEN_1920X1080_60:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;        
        case SCREEN_3840X2160_30:
            enVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_30;
            break;
        case SCREEN_3840X2160_60:    
            enVideoFmt = HI_HDMI_VIDEO_FMT_3840X2160P_60;           
            break;
        default:VAPILOG("can not support srceen resolution[%d], use default[1080P60]!", enRes);
            enVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
    }
    
    return enVideoFmt;  
}

static HI_S32 vapi_vo_layer_get_videofmt(VO_INTF_SYNC_E enIntfSync, HI_U32 *pu32W, HI_U32 *pu32H, HI_U32 *pu32Frm)
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
            VAPILOG("vo enIntfSync not support!\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_VOID vapi_vo_hdmi_get_videofmt(VO_INTF_SYNC_E enIntfSync, HI_HDMI_VIDEO_FMT_E *penVideoFmt)
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
            VAPILOG("Unknow VO_INTF_SYNC_E value[%d], use defualt[1080P@60]!\n", enIntfSync);
            *penVideoFmt = HI_HDMI_VIDEO_FMT_1080P_60;
            break;
    }

}

static void vapi_vo_get_sub_dev_mode(VO_INTF_SYNC_E enIntfSync, HI_S32 *hdmi, HI_S32 *vga)
{
    switch(enIntfSync)
    {
        case VO_OUTPUT_1280x1024_60:
            *hdmi = HDMI_1280_1024_60;
            *vga = THS8200_1280_1024_60Hz_108M;
            break;        
        case VO_OUTPUT_1024x768_60:
            *hdmi = HDMI_1024_768_60;
            *vga = THS8200_1024_768_60Hz_65M;
            break;
        case VO_OUTPUT_720P60:
            *hdmi = HDMI_720P60;
            *vga = THS8200_1280_720_60Hz_74M;
            break;
        case VO_OUTPUT_1080P50:
            *hdmi = HDMI_1080P50;
            *vga = THS8200_1920_1080_50Hz_148M;
            break;
        case VO_OUTPUT_1080P60:
        default:
            *hdmi = HDMI_1080P60;
            *vga = THS8200_1920_1080_60Hz_148M;
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

static HI_BOOL vapi_vo_check_edid(HI_HDMI_ID_E enHdmi, VO_INTF_SYNC_E enIntfSync)
{
    HI_HDMI_EDID_S stEdidData;
    HDMI_EDID_S *pstEDID;
    HI_U32 W,H,F;
    HI_U32 HACT, VACT;
    
    HI_S32 s32Ret = HI_SUCCESS;
    
    s32Ret = HI_MPI_HDMI_Force_GetEDID(enHdmi, &stEdidData);    
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Force_GetEDID failed with %#x!\n", s32Ret);
        return HI_FALSE;
    }
    
    pstEDID = (HDMI_EDID_S *)stEdidData.u8Edid;
    HACT = pstEDID->video[0].horiz_active + ((pstEDID->video[0].horiz_high & 0xF0) << 4);
    VACT = pstEDID->video[0].vert_active + ((pstEDID->video[0].vert_high & 0xF0) << 4);
    
    vapi_vo_layer_get_videofmt(enIntfSync, &W, &H, &F);
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

static HI_S32 vapi_vo_hdmi_unplug(HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S *)pPrivateData;
    HI_HDMI_ID_E hHdmi = pArgs->enHdmi;
    
    s32Ret = HI_MPI_HDMI_Stop(hHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Stop failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    VAPILOG("HDMI has UnPlug.\n");
    return HI_SUCCESS;
}

static HI_S32 vapi_vo_hdmi_hotplug(HI_VOID *pPrivateData)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HDMI_ARGS_S *pArgs = (HDMI_ARGS_S*)pPrivateData;
    HI_HDMI_ID_E hHdmi = pArgs->enHdmi;
    HI_HDMI_ATTR_S stHdmiAttr;
    HI_HDMI_SINK_CAPABILITY_S stSinkCap;
    HI_S32 update_flag = 0 ;  
    SCREEN_RES_E enRes;
    HI_HDMI_VIDEO_FMT_E fmt;
    HI_S32 w, h;
    
    s32Ret = HI_MPI_HDMI_GetSinkCapability(hHdmi, &stSinkCap);
    s32Ret = HI_MPI_HDMI_GetAttr(hHdmi, &stHdmiAttr);
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
//        stHdmiAttr.enSampleRate = HI_HDMI_SAMPLE_RATE_32K;
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
    if ( (pArgs->eForceFmt >= HI_HDMI_VIDEO_FMT_1080P_60
        && pArgs->eForceFmt < HI_HDMI_VIDEO_FMT_BUTT
        && stSinkCap.bVideoFmtSupported[pArgs->eForceFmt])
        || vapi_vo_check_edid(hHdmi, pArgs->enIntfSync))
    {
        stHdmiAttr.enVideoFmt = pArgs->eForceFmt;
        update_flag = 0;
    }
    else
    {
        stHdmiAttr.enVideoFmt = stSinkCap.enNativeVideoFormat;
//        VAPILOG("HDMI format[%d] unsupported! use default format[%d]", 
//                pArgs->eForceFmt, stHdmiAttr.enVideoFmt);
        update_flag = 1;
    }
    
    s32Ret = HI_MPI_HDMI_SetAttr(hHdmi, &stHdmiAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_SetAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }    

    s32Ret = HI_MPI_HDMI_Start(hHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Start failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    if (update_flag)
    {
        for (enRes=SCREEN_RES_NUM-1; enRes>SCREEN_UNDEFINE; enRes--)
        {
            fmt = vapi_vo_get_srceen_hdmi(enRes);
            if (stSinkCap.bVideoFmtSupported[fmt])
            {
                vapi_vo_calc_w_h(enRes, &w, &h);
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
    
    VAPILOG("HDMI has HotPlug.\n");

    return s32Ret;
}

HI_VOID vapi_vo_hdmi_event(HI_HDMI_EVENT_TYPE_E event, HI_VOID *pPrivateData)
{
    comm_mutex_lock(&g_stVoChn.mutex);
    switch ( event )
    {
    case HI_HDMI_EVENT_HOTPLUG:
        vapi_vo_hdmi_hotplug(pPrivateData);
        break;
    case HI_HDMI_EVENT_NO_PLUG:
        vapi_vo_hdmi_unplug(pPrivateData);
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
        VAPILOG("un-known event:%d\n",event);
        break;
    }
    comm_mutex_unlock(&g_stVoChn.mutex);
    return;
}

static HI_S32 vapi_vo_start_hdmi(VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HDMI_VIDEO_FMT_E enVideoFmt;
    HI_HDMI_ATTR_S      stAttr;
    HI_HDMI_CALLBACK_FUNC_S stCallbackFunc;

    vapi_vo_hdmi_get_videofmt(enIntfSync, &enVideoFmt);
    s32Ret |= HI_MPI_HDMI_Init();
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Init failed with %#x!\n", s32Ret);
    }

    g_stVoChn.stHdmiArgs.enHdmi       = HI_HDMI_ID_0;
    g_stVoChn.stHdmiArgs.eForceFmt    = enVideoFmt;
    g_stVoChn.stHdmiArgs.enIntfSync   =  enIntfSync;
    
    s32Ret |= HI_MPI_HDMI_Open(g_stVoChn.stHdmiArgs.enHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Open failed with %#x!\n", s32Ret);
    }
    
    if (g_stVoChn.isHotplug == HI_FALSE)
    {
        s32Ret = HI_MPI_HDMI_GetAttr(g_stVoChn.stHdmiArgs.enHdmi, &stAttr);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_HDMI_GetAttr failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }         
        stAttr.bEnableHdmi = HI_TRUE;        
        stAttr.bEnableVideo = HI_TRUE;
        stAttr.enVideoFmt = enVideoFmt;        
        stAttr.enVidOutMode = HI_HDMI_VIDEO_MODE_YCBCR444;
        stAttr.enDeepColorMode = HI_HDMI_DEEP_COLOR_OFF;
        stAttr.bxvYCCMode = HI_FALSE;
        stAttr.enDefaultMode = HI_HDMI_FORCE_HDMI;        
        stAttr.bEnableAudio = HI_TRUE;
        stAttr.enSoundIntf = HI_HDMI_SND_INTERFACE_I2S;
        stAttr.enSampleRate = HI_HDMI_SAMPLE_RATE_32K;
        stAttr.bIsMultiChannel = HI_FALSE;        
        stAttr.enBitDepth = HI_HDMI_BIT_DEPTH_16;        
        stAttr.bEnableAviInfoFrame = HI_TRUE;
        stAttr.bEnableAudInfoFrame = HI_TRUE;
        stAttr.bEnableSpdInfoFrame = HI_FALSE;
        stAttr.bEnableMpegInfoFrame = HI_FALSE;        
        stAttr.bDebugFlag = HI_FALSE;          
        stAttr.bHDCPEnable = HI_FALSE;        
        stAttr.b3DEnable = HI_FALSE;
        
        s32Ret = HI_MPI_HDMI_SetAttr(g_stVoChn.stHdmiArgs.enHdmi, &stAttr);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_HDMI_SetAttr failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        } 
        
        s32Ret = HI_MPI_HDMI_Start(g_stVoChn.stHdmiArgs.enHdmi);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_HDMI_SetAttr failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        } 
    }
    else
    {
        comm_mutex_lock(&g_stVoChn.mutex);
        stCallbackFunc.pfnHdmiEventCallback = vapi_vo_hdmi_event;
        stCallbackFunc.pPrivateData         = &g_stVoChn.stHdmiArgs;
        s32Ret |= HI_MPI_HDMI_RegCallbackFunc(g_stVoChn.stHdmiArgs.enHdmi, &stCallbackFunc);
        comm_mutex_unlock(&g_stVoChn.mutex);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_HDMI_RegCallbackFunc failed with %#x!\n", s32Ret);
            HI_MPI_HDMI_Close(g_stVoChn.stHdmiArgs.enHdmi);
            HI_MPI_HDMI_DeInit();
        }
    }
    
    return s32Ret;
}

static HI_S32 vapi_vo_stop_hdmi()
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_HDMI_CALLBACK_FUNC_S stCallbackFunc;
    
    s32Ret |= HI_MPI_HDMI_Stop(g_stVoChn.stHdmiArgs.enHdmi);    
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Stop failed with %#x!\n", s32Ret);
    }

    if (g_stVoChn.isHotplug == HI_TRUE)
    {
        
        comm_mutex_lock(&g_stVoChn.mutex);
        stCallbackFunc.pfnHdmiEventCallback = vapi_vo_hdmi_event;
        stCallbackFunc.pPrivateData         = &g_stVoChn.stHdmiArgs;
        
        s32Ret |= HI_MPI_HDMI_UnRegCallbackFunc(g_stVoChn.stHdmiArgs.enHdmi, &stCallbackFunc);
        comm_mutex_unlock(&g_stVoChn.mutex);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("HI_MPI_HDMI_UnRegCallbackFunc failed with %#x!\n", s32Ret);
        }
    }

    s32Ret |= HI_MPI_HDMI_Close(g_stVoChn.stHdmiArgs.enHdmi);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_Close failed with %#x!\n", s32Ret);
    }

    s32Ret |= HI_MPI_HDMI_DeInit();
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_HDMI_DeInit failed with %#x!\n", s32Ret);
    }
    
    return s32Ret;
}

static void vapi_vo_start_hdmi_sub(VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 Hfd = -1;
    HI_S32 Vfd = -1;
    HI_S32 hdmi_mode, vga_mode;    
    HI_S32 enable_timer = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    
    vapi_vo_get_sub_dev_mode(enIntfSync, &hdmi_mode, &vga_mode);
    
    // 1. set sub hdmi dev config    
	Hfd  = open(DEV_SUB_HDMI_NAME, O_RDWR, 0);
	if (Hfd != -1)
	{	    
	    s32Ret = ioctl(Hfd, SIL9024_CHOICE_MODE, &hdmi_mode);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG(" set sub hdmi dev mode[%d] failed \n", hdmi_mode);
        }
        
        if (g_stVoChn.isHotplug == HI_TRUE)
        {
            enable_timer = HI_TRUE;
            s32Ret = ioctl(Hfd, SIL9024_SET_TIMER_POLL, &enable_timer);
            if (s32Ret != HI_SUCCESS)
            {
                VAPILOG(" set sub hdmi dev timer[%d] failed \n", enable_timer);
            }
        }        
        close(Hfd);
	}
	else
	{
        VAPILOG("open sub hdmi dev failed \n");
	}

	
    // 2. set sub vga dev config	
    Vfd  = open(DEV_SUB_VGA_NAME, O_RDWR, 0);
	if (Vfd != -1)
	{
	    s32Ret = ioctl(Vfd, THS8200_CHOICE_MODE, &vga_mode);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG(" set sub vga dev mode[%d] failed \n", vga_mode);
        }
        close(Vfd);
	}
	else
	{
        VAPILOG("open sub vga dev failed \n");
	}
}

static HI_S32 vapi_vo_stop_hdmi_sub()
{
    HI_S32 Hfd = -1;
    HI_S32 enable_timer = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    
	Hfd  = open(DEV_SUB_HDMI_NAME, O_RDWR, 0);
	if (Hfd != -1)
	{
        if (g_stVoChn.isHotplug == HI_TRUE)
        {
            enable_timer = HI_FALSE;
            s32Ret = ioctl(Hfd, SIL9024_SET_TIMER_POLL, &enable_timer);
            if (s32Ret != HI_SUCCESS)
            {
                VAPILOG(" set sub hdmi dev timer[%d] failed \n", enable_timer);
            }
        }
        close(Hfd);
	}
	else
	{
        VAPILOG("close sub hdmi dev failed \n");
	}
	
    return HI_SUCCESS;
}

static void vapi_vo_hdmi_sub_check_edid(SCREEN_RES_E enRes)
{
    HI_S32 Hfd = -1;
    sil_edid_s stEdidData;
    HDMI_EDID_S *pstEDID;
    HI_S32 W,H;
    HI_U32 HACT, VACT;    
    HI_S32 s32Ret = HI_SUCCESS;

    if (enRes == SCREEN_UNDEFINE)
    {
        return ;
    }
    
	Hfd  = open(DEV_SUB_HDMI_NAME, O_RDWR, 0);
	if (Hfd != -1)
	{	    
	    s32Ret = ioctl(Hfd, SIL9024_GET_EDID, &stEdidData);
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG(" vapi_vo_hdmi_sub_check_edid failed \n");
        }
        close(Hfd);
	}
	else
	{
//        VAPILOG("open DEV_SUB_HDMI failed \n");
        return ;
	}

    if (stEdidData.isValid == HI_TRUE)
    {
        pstEDID = (HDMI_EDID_S *)stEdidData.data;
        HACT = pstEDID->video[0].horiz_active + ((pstEDID->video[0].horiz_high & 0xF0) << 4);
        VACT = pstEDID->video[0].vert_active + ((pstEDID->video[0].vert_high & 0xF0) << 4);

        vapi_vo_calc_w_h(enRes, &W, &H);
//        VAPILOG(" HACT[%d] VACT[%d] W[%d] H[%d] \n", HACT, VACT, W, H);
        if (W<=HACT && H<=VACT)
        {
            g_stVoChn.stAdapt.enRes[SCREEN_SUB] = SCREEN_UNDEFINE;
        }
        else
        {
            for (enRes=SCREEN_RES_NUM-1; enRes>SCREEN_UNDEFINE; enRes--)
            {
                vapi_vo_calc_w_h(enRes, &W, &H);
                if (W<=HACT && H<=VACT)
                {
                    g_stVoChn.stAdapt.enRes[SCREEN_SUB] = enRes;
                }
                else
                {
                    g_stVoChn.stAdapt.enRes[SCREEN_SUB] = SCREEN_UNDEFINE;
                }
            }
        }
    }

}

static HI_S32 vapi_vo_start_dev(VO_DEV VoDev, VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_PUB_ATTR_S stVoPubAttr;    
    
    if (VoDev == VAPI_VO_DEV_DHD0)
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
    }
    else if (VoDev == VAPI_VO_DEV_DHD1)
    {
        //BT1120 max 1080P@60
        stVoPubAttr.enIntfSync = enIntfSync;//MIN(enIntfSync, VO_OUTPUT_1080P60);
        stVoPubAttr.enIntfType = VO_INTF_BT1120;
    }
    else
    {
        VAPILOG("dev[%d] don't support! \n", VoDev);
        return HI_FAILURE;
    }
    HI_MPI_VO_Disable(VoDev); // clear first
    stVoPubAttr.u32BgColor = 0x00000000;
    s32Ret = HI_MPI_VO_SetPubAttr(VoDev, &stVoPubAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_SetPubAttr[%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_Enable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_Enable [%d]failed with %#x!\n",VoDev, s32Ret);
        return HI_FAILURE;
    }

    // config HDMI0&VGA0 device
    if (VoDev == VAPI_VO_DEV_DHD0)
    {
        vapi_vo_start_hdmi(enIntfSync);
    }
    // config HDMI1&VGA1 device
    else if (VoDev == VAPI_VO_DEV_DHD1)
    {
        vapi_vo_start_hdmi_sub(enIntfSync);
    }
    
    return HI_SUCCESS;
}

static HI_S32 vapi_vo_stop_dev(VO_DEV VoDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    if (VoDev == VAPI_VO_DEV_DHD0)
    {
        s32Ret = vapi_vo_stop_hdmi();
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("vapi_vo_stop_hdmi failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    else if (VoDev == VAPI_VO_DEV_DHD1)
    {
        s32Ret = vapi_vo_stop_hdmi_sub();
        if (s32Ret != HI_SUCCESS)
        {
            VAPILOG("vapi_vo_stop_hdmi_sub failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
    s32Ret = HI_MPI_VO_Disable(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_Disable [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 vapi_vo_start_layer(VO_LAYER VoLayer, VO_INTF_SYNC_E enIntfSync)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    HI_S32 i;

    s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_SetVideoLayerAttr [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }
    
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_420;

    s32Ret = vapi_vo_layer_get_videofmt(enIntfSync, &stLayerAttr.stDispRect.u32Width, 
                &stLayerAttr.stDispRect.u32Height, &stLayerAttr.u32DispFrmRt);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_layer_get_videofmt [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }

    if (VoLayer == VAPI_VO_LAYER_VPIP)
    {    
        stLayerAttr.bClusterMode = HI_TRUE;
    }
    else
    {
        stLayerAttr.bClusterMode = HI_FALSE;
    }
//    stLayerAttr.u32DispFrmRt = 25;
//    HI_U32 u32DispBufLen = 10;
//        
//    s32Ret = HI_MPI_VO_SetDispBufLen(VoLayer, u32DispBufLen);
//    if (s32Ret != HI_SUCCESS)
//    {
//        printf("Set display buf len failed with error code %#x!\n", s32Ret);
//        return HI_FAILURE;
//    }
    
    stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;

    s32Ret = HI_MPI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_SetVideoLayerAttr [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_EnableVideoLayer [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }

    for(i=0; i<MAX_CHN_NUM; i++)
    {
        g_stVoChn.stChn[VoLayer][i].enState = STATE_IDLE;
    }

    return HI_SUCCESS;
}

static HI_S32 vapi_vo_stop_layer(VO_LAYER VoLayer)
{
    HI_S32 s32Ret = HI_SUCCESS;    
    HI_S32 i;

    for(i=0; i<MAX_CHN_NUM; i++)
    {
        vapi_vo_chn_destory(VoLayer, i);
    }
    
    s32Ret = HI_MPI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_DisableVideoLayer [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }
    
    return s32Ret;
}

static void *vapi_vo_adaptive_pthread(void *arg)
{
    HI_S32 i;

    comm_task_set_name("adaptive");

    while(g_stVoChn.stAdapt.isRun)
    {
        for (i=0; i<SCREEN_NUM; i++)
        {
            if (!g_stVoChn.isDualScreen && i == SCREEN_SUB)
                continue;
                
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

HI_S32 vapi_vo_start_pip_layer(SCREEN_RES_E enRes)
{
    return HI_SUCCESS; // dont support !!
    VO_INTF_SYNC_E enIntfSync;
    HI_S32 s32Ret = HI_SUCCESS;

    enIntfSync = vapi_vo_get_srceen_sync(enRes);
    s32Ret = vapi_vo_start_layer(VAPI_VO_LAYER_VPIP, enIntfSync);

    return s32Ret;
}

HI_S32 vapi_vo_stop_pip_layer()
{
    return HI_SUCCESS; // dont support !!
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = vapi_vo_stop_layer(VAPI_VO_LAYER_VPIP);

    return s32Ret;
}

HI_S32 vapi_vo_pip_rebind(VO_DEV VoDev)
{
    return HI_SUCCESS; // dont support !!
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = vapi_vo_stop_pip_layer();
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }
    //解除之前的绑定关系
    s32Ret = HI_MPI_VO_UnBindVideoLayer(VAPI_VO_LAYER_VPIP, !VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_UnBindVideoLayer [%d] failed with %#x!\n", VAPI_VO_LAYER_VPIP, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VO_BindVideoLayer(VAPI_VO_LAYER_VPIP, VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_BindVideoLayer [%d->%d] failed with %#x!\n", VAPI_VO_LAYER_VPIP, VoDev, s32Ret);
        return HI_FAILURE;
    }
    
    //重新计算PIP 大小
    s32Ret = vapi_vo_start_pip_layer(g_stVoChn.enRes[VoDev]);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }

    g_stVoChn.enPipBind = VoDev;
    
    return HI_SUCCESS;
}

HI_VOID vapi_vo_get_srceen_res(SCREEN_E enScreen, HI_S32 *W, HI_S32 *H)
{
    SCREEN_RES_E enRes;
    
    enRes = g_stVoChn.enRes[enScreen];
    vapi_vo_calc_w_h(enRes, W, H);
}

HI_S32 vapi_vo_set_csc(VO_LAYER VoLayer, CSC_E enCSC)
{
    VO_CSC_S stVideoCSC;
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_GetVideoLayerCSC(VoLayer,&stVideoCSC);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_GetVideoLayerCSC [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }    

    switch(enCSC)
    {
        case CSC_STANDARD:
            stVideoCSC.u32Luma       = 65;//74;
            stVideoCSC.u32Contrast   = 40;//45;
            stVideoCSC.u32Hue        = 48; //60;
            stVideoCSC.u32Saturation = 50; //50;
            break;
        case CSC_LIGHTNESS:
            stVideoCSC.u32Luma       = 60;
            stVideoCSC.u32Contrast   = 50;
            stVideoCSC.u32Hue        = 50;
            stVideoCSC.u32Saturation = 45;
            break;
        case CSC_SOFT:
            stVideoCSC.u32Luma       = 45;
            stVideoCSC.u32Contrast   = 50;
            stVideoCSC.u32Hue        = 50;
            stVideoCSC.u32Saturation = 45;
            break;
        case CSC_COLORFUL:
            stVideoCSC.u32Luma       = 50;
            stVideoCSC.u32Contrast   = 50;
            stVideoCSC.u32Hue        = 50;
            stVideoCSC.u32Saturation = 60;
            break;
        default:
            stVideoCSC.u32Luma       = 74;
            stVideoCSC.u32Contrast   = 45;
            stVideoCSC.u32Hue        = 60;
            stVideoCSC.u32Saturation = 50;
            break;
    }
    
    s32Ret = HI_MPI_VO_SetVideoLayerCSC(VoLayer, &stVideoCSC);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_SetVideoLayerCSC [%d] failed with %#x!\n", VoLayer, s32Ret);
        return HI_FAILURE;
    }

    g_stVoChn.enCSC[VoLayer] = enCSC;
    
    return HI_SUCCESS;
    
}

HI_S32 vapi_vo_chn_create(VO_LAYER VoLayer, VO_CHN VoChn, RECT_S *pstRect, RATIO_E enRatio)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_CHN_ATTR_S stChnAttr;        
    VO_CHN_PARAM_S stChnParam;

    if (g_stVoChn.stChn[VoLayer][VoChn].enState == STATE_BUSY)
    {
        VAPILOG("vout channel[%d][%d] has existed !\n", VoLayer, VoChn);
        return HI_FAILURE;
    }

    stChnAttr.stRect.s32X       = ALIGN_BACK(pstRect->s32X, 2);
    stChnAttr.stRect.s32Y       = ALIGN_BACK(pstRect->s32Y, 2);
    stChnAttr.stRect.u32Width   = ALIGN_BACK(pstRect->u32Width, 2);
    stChnAttr.stRect.u32Height  = ALIGN_BACK(pstRect->u32Height, 2);
    stChnAttr.u32Priority       = 0;
    stChnAttr.bDeflicker        = HI_FALSE;

    s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, VoChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_SetChnAttr [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VO_GetChnParam(VoLayer, VoChn, &stChnParam);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_GetChnParam [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    stChnParam.stAspectRatio.enMode = enRatio;

    s32Ret = HI_MPI_VO_SetChnParam(VoLayer, VoChn, &stChnParam);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_SetChnParam [%d][%d] set ratio [%d] failed with %#x!\n", VoLayer, VoChn, enRatio, s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VO_EnableChn(VoLayer, VoChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_EnableChn [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }

    g_stVoChn.stChn[VoLayer][VoChn].enState = STATE_BUSY;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.x = pstRect->s32X;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.y = pstRect->s32Y;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.w = pstRect->u32Width;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.h = pstRect->u32Height;
    
    return HI_SUCCESS;
}

HI_S32 vapi_vo_chn_destory(VO_LAYER VoLayer, VO_CHN VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (g_stVoChn.stChn[VoLayer][VoChn].enState != STATE_BUSY)
    {
        return HI_SUCCESS;
    }

    s32Ret = HI_MPI_VO_DisableChn(VoLayer, VoChn);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_DisableChn [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    g_stVoChn.stChn[VoLayer][VoChn].enState = STATE_IDLE;
    
    return HI_SUCCESS;
}

HI_S32  vapi_vo_chn_clear_buff(VO_LAYER VoLayer, VO_CHN VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_VO_ClearChnBuffer(VoLayer, VoChn, HI_TRUE);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_ClearChnBuffer [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

HI_VOID vapi_vo_chn_clear_all_buff(VO_LAYER VoLayer)
{
    HI_S32 i;

    for (i=0; i<MAX_CHN_NUM; i++)
    {
        if (g_stVoChn.stChn[VoLayer][i].enState == STATE_BUSY)
        {
            vapi_vo_chn_clear_buff(VoLayer, i);
        }
    }
}

HI_S32 vapi_vo_chn_set_fps(VO_LAYER VoLayer, VO_CHN VoChn, HI_S32 s32Fps)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (g_stVoChn.stChn[VoLayer][VoChn].enState == STATE_IDLE)
    {
        return HI_SUCCESS;
    }
    
    s32Ret = HI_MPI_VO_SetChnFrameRate(VoLayer, VoChn, s32Fps);  
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_chn_set_fps[%d] @[%d][%d] failed with %#x!\n", s32Fps, VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }
    
    return HI_SUCCESS;
}

HI_S32 vapi_vo_bind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
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
        VAPILOG("HI_MPI_SYS_Bind [%d->%d] failed with %#x!\n", VpssGrp, VoChn, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_S32 vapi_vo_unbind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
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
        VAPILOG("HI_MPI_SYS_UnBind [%d->%d] failed with %#x!\n", VpssGrp, VoChn, s32Ret);
        return HI_FAILURE;
    }
        
    return HI_SUCCESS;
}

HI_S32 vapi_vo_get_pipbind()
{
    return g_stVoChn.enPipBind;
}

HI_VOID vapi_vo_update_state(VO_LAYER VoLayer, VO_CHN VoChn, HI_S32 state)
{
   g_stVoChn.stChn[VoLayer][VoChn].enState = state;
}

HI_BOOL vapi_vo_check_state(VO_LAYER VoLayer, VO_CHN VoChn, HI_S32 state)
{
    return (g_stVoChn.stChn[VoLayer][VoChn].enState == state);
}

HI_S32	vapi_vo_set_ratio(VO_LAYER VoLayer, VO_CHN VoChn, RATIO_E enRatio)
{
	VO_CHN_PARAM_S stChnParam;	
    HI_S32 s32Ret = HI_SUCCESS;
	
	s32Ret = HI_MPI_VO_GetChnParam(VoLayer, VoChn, &stChnParam);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_GetChnParam [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    }  

	switch(enRatio)
	{
		case RATIO_VO_AUTO:
			stChnParam.stAspectRatio.enMode = ASPECT_RATIO_AUTO;
			break;
		case RATIO_VO_FULL:
		default:
			stChnParam.stAspectRatio.enMode = ASPECT_RATIO_NONE;
			break;
		
	}
	
	s32Ret = HI_MPI_VO_SetChnParam(VoLayer, VoChn, &stChnParam);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("HI_MPI_VO_SetChnParam [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return HI_FAILURE;
    } 

	return HI_SUCCESS;
}

HI_S32 vapi_vo_dev_open(VO_DEV VoDev, SCREEN_RES_E enRes)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_LAYER VoLayer;
    VO_INTF_SYNC_E enIntfSync;

    g_stVoChn.stAdapt.enRes[VoDev]= SCREEN_UNDEFINE;

    enIntfSync = vapi_vo_get_srceen_sync(enRes);    
    
    s32Ret = vapi_vo_start_dev(VoDev, enIntfSync);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_dev_open [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }    

    VoLayer = VoDev == VAPI_VO_DEV_DHD0 ? VAPI_VO_LAYER_VHD0 : VAPI_VO_LAYER_VHD1;
    s32Ret = vapi_vo_start_layer(VoLayer, enIntfSync);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_start_layer [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = vapi_vo_set_csc(VoLayer, g_stVoChn.enCSC[VoLayer]);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_set_csc [%d] failed with %#x!\n", g_stVoChn.enCSC[VoLayer], s32Ret);
        return HI_FAILURE;
    }
    
    g_stVoChn.enRes[VoDev] = enRes;
    
    return HI_SUCCESS;
}

HI_S32 vapi_vo_dev_close(VO_DEV VoDev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VO_LAYER VoLayer;

    VoLayer = VoDev == VAPI_VO_DEV_DHD0 ? VAPI_VO_LAYER_VHD0 : VAPI_VO_LAYER_VHD1;
    s32Ret = vapi_vo_stop_layer(VoLayer);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_stop_layer [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = vapi_vo_stop_dev(VoDev);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vo_stop_dev [%d] failed with %#x!\n", VoDev, s32Ret);
        return HI_FAILURE;
    }

    g_stVoChn.enRes[VoDev] = SCREEN_UNDEFINE;

    return HI_SUCCESS;
}

HI_S32 vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, int isHotplug, int isDualScreen, VDEC_CB_S *pstCb)
{
    HI_S32 s32Ret = HI_SUCCESS;

    g_stVoChn.update_screen_res_cb = pstCb->update_screen_res_cb;
    g_stVoChn.isHotplug = isHotplug;
    g_stVoChn.isDualScreen = isDualScreen;

    comm_mutex_init(&g_stVoChn.mutex);
    s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_DHD0, enMain);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }

    if (isDualScreen)
    {
        s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_DHD1, enSub);
        if (s32Ret != HI_SUCCESS)
        {
            return HI_FAILURE;
        }
    }

    s32Ret = vapi_vo_start_pip_layer(enMain);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }
    
    //pip defualt bind to VAPI_VO_DEV_DHD0
    g_stVoChn.enPipBind = VAPI_VO_DEV_DHD0;

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
    vapi_vo_stop_pip_layer();    
    vapi_vo_dev_close(VAPI_VO_DEV_DHD0);
    if (g_stVoChn.isDualScreen)
            vapi_vo_dev_close(VAPI_VO_DEV_DHD1);
    if (g_stVoChn.isHotplug == HI_TRUE)
    {
        vapi_vo_adaptive_stop();
        g_stVoChn.isHotplug = HI_FALSE;
    }
    
    g_stVoChn.update_screen_res_cb = NULL;
    comm_mutex_uninit(&g_stVoChn.mutex);
}

