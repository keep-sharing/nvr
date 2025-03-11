/*
 * ***************************************************************
 * Filename:      	vapi_vout.c
 * Created at:    	2015.10.21
 * Description:   	video output controller
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include "vapi_comm.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEV_SUB_HDMI_NAME "/dev/sil9024"
#define DEV_SUB_VGA_NAME  "/dev/ths8200"

typedef struct ths8200_reg
{
    unsigned char reg;
    unsigned char value;
} the8200_reg_t;

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
} ths_mode_e;

typedef enum sil_mode
{
    HDMI_BEGIN      = 0,
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
} sil_mode_e;

typedef struct sil_edid
{
    int  isValid;
    char data[512];
} sil_edid_s;

/*
 * ioctl cmd
 */
#define THS8200_IOC_MAGIC      't'
#define THS8200_GET_ALL_REG    _IO(THS8200_IOC_MAGIC, 1)
#define THS8200_GET_SINGLE_REG _IOWR(THS8200_IOC_MAGIC, 2, struct ths8200_reg)
#define THS8200_SET_SINGLE_REG _IOWR(THS8200_IOC_MAGIC, 3, struct ths8200_reg)
#define THS8200_CHOICE_MODE    _IOWR(THS8200_IOC_MAGIC, 4, enum ths_mode)

#define SIL9024_IOC_MAGIC      's'
#define SIL9024_GET_EDID       _IOWR(SIL9024_IOC_MAGIC, 1, struct sil_edid)
#define SIL9024_GET_SINGLE_REG _IOWR(SIL9024_IOC_MAGIC, 2, 0)
#define SIL9024_SET_TIMER_POLL _IOWR(SIL9024_IOC_MAGIC, 3, int)
#define SIL9024_CHOICE_MODE    _IOWR(SIL9024_IOC_MAGIC, 4, enum sil_mode)

/*  Video Descriptor Block  */
typedef struct HDMI_EDID_DTD_VIDEO
{
    td_u16 pixel_clock;       /* 54-55 */
    td_u8  horiz_active;      /* 56 */
    td_u8  horiz_blanking;    /* 57 */
    td_u8  horiz_high;        /* 58 */
    td_u8  vert_active;       /* 59 */
    td_u8  vert_blanking;     /* 60 */
    td_u8  vert_high;         /* 61 */
    td_u8  horiz_sync_offset; /* 62 */
    td_u8  horiz_sync_pulse;  /* 63 */
    td_u8  vert_sync_pulse;   /* 64 */
    td_u8  sync_pulse_high;   /* 65 */
    td_u8  horiz_image_size;  /* 66 */
    td_u8  vert_image_size;   /* 67 */
    td_u8  image_size_high;   /* 68 */
    td_u8  horiz_border;      /* 69 */
    td_u8  vert_border;       /* 70 */
    td_u8  misc_settings;     /* 71 */
} HDMI_EDID_DTD_VIDEO_S;

typedef struct HDMI_EDID
{
    td_u8  header[8];            /* 00-07 */
    td_u16 manufacturerID;       /* 08-09 */
    td_u16 product_id;           /* 10-11 */
    td_u32 serial_number;        /* 12-15 */
    td_u8  week_manufactured;    /* 16 */
    td_u8  year_manufactured;    /* 17 */
    td_u8  edid_version;         /* 18 */
    td_u8  edid_revision;        /* 19 */
    td_u8  video_in_definition;  /* 20 */
    td_u8  max_horiz_image_size; /* 21 */
    td_u8  max_vert_image_size;  /* 22 */
    td_u8  display_gamma;        /* 23 */
    td_u8  power_features;       /* 24 */
    td_u8  chroma_info[10];      /* 25-34 */
    td_u8  timing_1;             /* 35 */
    td_u8  timing_2;             /* 36 */
    td_u8  timing_3;             /* 37 */
    td_u8  std_timings[16];      /* 38-53 */

    HDMI_EDID_DTD_VIDEO_S video[4]; /* 54-125 */

    td_u8 extension_edid; /* 126 */
    td_u8 checksum;       /* 127 */
    td_u8 extension_tag;  /* 00 (extensions follow EDID) */
    td_u8 extention_rev;  /* 01 */
    td_u8 offset_dtd;     /* 02 */
    td_u8 num_dtd;        /* 03 */

    td_u8 data_block[123];    /* 04 - 126 */
    td_u8 extension_checksum; /* 127 */

    td_u8 ext_datablock[256];
} HDMI_EDID_S;

typedef struct hiHDMI_ARGS_S
{
    ot_hdmi_id           enHdmi;
    ot_hdmi_video_format eForceFmt;
    ot_vo_intf_sync      enIntfSync;
    ot_vo_intf_type      enIntfType;
    SCREEN_E             enScreen;
    DEV_E                enDev;
} HDMI_ARGS_S;

typedef struct adaptive_s
{
    pthread_t       handle;
    pthread_mutex_t mutex;
    td_bool         isRun;
    SCREEN_RES_E    enRes[SCREEN_NUM];
} ADAPTIVE_S;

typedef struct voChn_s
{
    CHNINFO_S    stChn[VAPI_VO_LAYER_NUM][MAX_CHN_NUM];
    SCREEN_RES_E enRes[SCREEN_NUM];
    CSC_E        enCSC[VAPI_VO_LAYER_NUM];
    HDMI_ARGS_S  stHdmiArgs[VAPI_VO_DEV_NUM];
    SCREEN_E     enPipBind;
    int (*update_screen_res_cb)(int screen, int res);
    ADAPTIVE_S      stAdapt;
    td_bool         isHotplug;
    td_bool         isDualScreen;
    td_bool         isogeny;
    pthread_mutex_t mutex;

} VOCHN_S;

static VOCHN_S g_stVoChn;
static char g_prefix;

static ot_vo_intf_sync vapi_vo_get_srceen_sync(SCREEN_RES_E enRes)
{
    ot_vo_intf_sync enIntfSync;

    switch (enRes) {
    case SCREEN_1024X768_60:
        enIntfSync = OT_VO_OUT_1024x768_60;
        break;
    case SCREEN_1280X720_60:
        enIntfSync = OT_VO_OUT_720P60;
        break;
    case SCREEN_1280X1024_60:
        enIntfSync = OT_VO_OUT_1280x1024_60;
        break;
    case SCREEN_1600X1200_60:
        enIntfSync = OT_VO_OUT_1600x1200_60;
        break;
    case SCREEN_1920X1080_30:
        enIntfSync = OT_VO_OUT_1080P30;
        break;
    case SCREEN_1920X1080_50:
        enIntfSync = OT_VO_OUT_1080P50;
        break;
    case SCREEN_1920X1080_60:
        enIntfSync = OT_VO_OUT_1080P60;
        break;
    case SCREEN_3840X2160_30:
        enIntfSync = OT_VO_OUT_3840x2160_30;
        break;
    case SCREEN_3840X2160_60:
        enIntfSync = OT_VO_OUT_3840x2160_60;
        break;
    default:
        VAPILOG("can not support srceen resolution[%d], use default[1080P60]!", enRes);
        enIntfSync = OT_VO_OUT_1080P60;
        break;
    }

    return enIntfSync;
}

static ot_hdmi_video_format vapi_vo_get_srceen_hdmi(SCREEN_RES_E enRes)
{
    ot_hdmi_video_format enVideoFmt;

    switch (enRes) {
    case SCREEN_1024X768_60:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1024X768_60;
        break;
    case SCREEN_1280X720_60:
        // yuyu，没有1280x720
        // enVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1280X720_60;
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1280X800_60;
        break;
    case SCREEN_1280X1024_60:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1280X1024_60;
        break;
    case SCREEN_1600X1200_60:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1600X1200_60;
        break;
    case SCREEN_1920X1080_30:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_30;
        break;
    case SCREEN_1920X1080_50:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_50;
        break;
    case SCREEN_1920X1080_60:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_60;
        break;
    case SCREEN_3840X2160_30:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_3840X2160P_30;
        break;
    case SCREEN_3840X2160_60:
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_3840X2160P_60;
        break;
    default:
        VAPILOG("can not support srceen resolution[%d], use default[1080P60]!", enRes);
        enVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_60;
        break;
    }

    return enVideoFmt;
}

static td_s32 vapi_vo_layer_get_videofmt(ot_vo_intf_sync enIntfSync, td_u32 *pu32W, td_u32 *pu32H, td_u32 *pu32Frm)
{
    switch (enIntfSync) {
    case OT_VO_OUT_PAL:
        *pu32W   = 720;
        *pu32H   = 576;
        *pu32Frm = 25;
        break;
    case OT_VO_OUT_NTSC:
        *pu32W   = 720;
        *pu32H   = 480;
        *pu32Frm = 30;
        break;
    case OT_VO_OUT_576P50:
        *pu32W   = 720;
        *pu32H   = 576;
        *pu32Frm = 50;
        break;
    case OT_VO_OUT_480P60:
        *pu32W   = 720;
        *pu32H   = 480;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_800x600_60:
        *pu32W   = 800;
        *pu32H   = 600;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_720P50:
        *pu32W   = 1280;
        *pu32H   = 720;
        *pu32Frm = 50;
        break;
    case OT_VO_OUT_720P60:
        *pu32W   = 1280;
        *pu32H   = 720;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1080I50:
        *pu32W   = 1920;
        *pu32H   = 1080;
        *pu32Frm = 50;
        break;
    case OT_VO_OUT_1080I60:
        *pu32W   = 1920;
        *pu32H   = 1080;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1080P24:
        *pu32W   = 1920;
        *pu32H   = 1080;
        *pu32Frm = 24;
        break;
    case OT_VO_OUT_1080P25:
        *pu32W   = 1920;
        *pu32H   = 1080;
        *pu32Frm = 25;
        break;
    case OT_VO_OUT_1080P30:
        *pu32W   = 1920;
        *pu32H   = 1080;
        *pu32Frm = 30;
        break;
    case OT_VO_OUT_1080P50:
        *pu32W   = 1920;
        *pu32H   = 1080;
        *pu32Frm = 50;
        break;
    case OT_VO_OUT_1080P60:
        *pu32W   = 1920;
        *pu32H   = 1080;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1024x768_60:
        *pu32W   = 1024;
        *pu32H   = 768;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1280x1024_60:
        *pu32W   = 1280;
        *pu32H   = 1024;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1366x768_60:
        *pu32W   = 1366;
        *pu32H   = 768;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1440x900_60:
        *pu32W   = 1440;
        *pu32H   = 900;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1280x800_60:
        *pu32W   = 1280;
        *pu32H   = 800;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1600x1200_60:
        *pu32W   = 1600;
        *pu32H   = 1200;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1680x1050_60:
        *pu32W   = 1680;
        *pu32H   = 1050;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_1920x1200_60:
        *pu32W   = 1920;
        *pu32H   = 1200;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_3840x2160_30:
        *pu32W   = 3840;
        *pu32H   = 2160;
        *pu32Frm = 30;
        break;
    case OT_VO_OUT_3840x2160_60:
        *pu32W   = 3840;
        *pu32H   = 2160;
        *pu32Frm = 60;
        break;
    case OT_VO_OUT_USER:
        *pu32W   = 720;
        *pu32H   = 576;
        *pu32Frm = 25;
        break;
    default:
        VAPILOG("vo enIntfSync not support!\n");
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_void vapi_vo_hdmi_get_videofmt(ot_vo_intf_sync enIntfSync, ot_hdmi_video_format *penVideoFmt)
{
    switch (enIntfSync) {
    case OT_VO_OUT_PAL:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_PAL;
        break;
    case OT_VO_OUT_NTSC:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_NTSC;
        break;
    case OT_VO_OUT_1080P24:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_24;
        break;
    case OT_VO_OUT_1080P25:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_25;
        break;
    case OT_VO_OUT_1080P30:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_30;
        break;
    case OT_VO_OUT_720P50:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_720P_50;
        break;
    case OT_VO_OUT_720P60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_720P_60;
        break;
    case OT_VO_OUT_1080I50:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080i_50;
        break;
    case OT_VO_OUT_1080I60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080i_60;
        break;
    case OT_VO_OUT_1080P50:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_50;
        break;
    case OT_VO_OUT_1080P60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_60;
        break;
    case OT_VO_OUT_576P50:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_576P_50;
        break;
    case OT_VO_OUT_480P60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_480P_60;
        break;
    case OT_VO_OUT_800x600_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_800X600_60;
        break;
    case OT_VO_OUT_1024x768_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1024X768_60;
        break;
    case OT_VO_OUT_1280x1024_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1280X1024_60;
        break;
    case OT_VO_OUT_1366x768_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1366X768_60;
        break;
    case OT_VO_OUT_1440x900_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1440X900_60;
        break;
    case OT_VO_OUT_1280x800_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1280X800_60;
        break;
    case OT_VO_OUT_1600x1200_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1600X1200_60;
        break;
    case OT_VO_OUT_1920x1200_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_VESA_1920X1200_60;
        break;
    case OT_VO_OUT_3840x2160_30:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_3840X2160P_30;
        break;
    case OT_VO_OUT_3840x2160_60:
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_3840X2160P_60;
        break;
    default:
        VAPILOG("Unknow ot_vo_intf_sync value[%d], use defualt[1080P@60]!\n", enIntfSync);
        *penVideoFmt = OT_HDMI_VIDEO_FORMAT_1080P_60;
        break;
    }
}

static void vapi_vo_get_sub_dev_mode(ot_vo_intf_sync enIntfSync, td_s32 *hdmi, td_s32 *vga)
{
    switch (enIntfSync) {
    case OT_VO_OUT_1280x1024_60:
        *hdmi = HDMI_1280_1024_60;
        *vga  = THS8200_1280_1024_60Hz_108M;
        break;
    case OT_VO_OUT_1024x768_60:
        *hdmi = HDMI_1024_768_60;
        *vga  = THS8200_1024_768_60Hz_65M;
        break;
    case OT_VO_OUT_720P60:
        *hdmi = HDMI_720P60;
        *vga  = THS8200_1280_720_60Hz_74M;
        break;
    case OT_VO_OUT_1080P50:
        *hdmi = HDMI_1080P50;
        *vga  = THS8200_1920_1080_50Hz_148M;
        break;
    case OT_VO_OUT_1080P60:
    default:
        *hdmi = HDMI_1080P60;
        *vga  = THS8200_1920_1080_60Hz_148M;
        break;
    }
}

static void vapi_vo_calc_w_h(SCREEN_RES_E enRes, td_s32 *W, td_s32 *H)
{
    switch (enRes) {
    case SCREEN_1024X768_60:
        *W = 1024;
        *H = 768;
        break;
    case SCREEN_1280X720_60:
        *W = 1280;
        *H = 720;
        break;
    case SCREEN_1280X1024_60:
        *W = 1280;
        *H = 1024;
        break;
    case SCREEN_1600X1200_60:
        *W = 1600;
        *H = 1200;
        break;
    case SCREEN_1920X1080_30:
    case SCREEN_1920X1080_50:
    case SCREEN_1920X1080_60:
        *W = 1920;
        *H = 1080;
        break;
    case SCREEN_3840X2160_30:
    case SCREEN_3840X2160_60:
        *W = 3840;
        *H = 2160;
        break;
    default:
        VAPILOG("can not support srceen resolution[%d]\n\n", enRes);
        *W = 0;
        *H = 0;
        break;
    }
}

static td_bool vapi_vo_check_edid(ot_hdmi_id enHdmi, ot_vo_intf_sync enIntfSync)
{
    ot_hdmi_edid stEdidData;
    HDMI_EDID_S *pstEDID;
    td_u32       W, H, F;
    td_u32       HACT, VACT;

    td_s32 s32Ret = TD_SUCCESS;

    s32Ret = ss_mpi_hdmi_force_get_edid(enHdmi, &stEdidData);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_force_get_edid failed with %#x!\n", s32Ret);
        return TD_FALSE;
    }

    pstEDID = (HDMI_EDID_S *)stEdidData.edid;
    HACT    = pstEDID->video[0].horiz_active + ((pstEDID->video[0].horiz_high & 0xF0) << 4);
    VACT    = pstEDID->video[0].vert_active + ((pstEDID->video[0].vert_high & 0xF0) << 4);

    vapi_vo_layer_get_videofmt(enIntfSync, &W, &H, &F);
    VAPILOG(" HACT[%d] VACT[%d] W[%d] H[%d] \n", HACT, VACT, W, H);
    if (W <= HACT && H <= VACT) {
        return TD_TRUE;
    } else {
        return TD_FALSE;
    }
}

static td_s32 vapi_vo_hdmi_unplug(td_void *pPrivateData)
{
    td_s32       s32Ret = TD_SUCCESS;
    HDMI_ARGS_S *pArgs  = (HDMI_ARGS_S *)pPrivateData;
    ot_hdmi_id   hHdmi  = pArgs->enHdmi;

    s32Ret = ss_mpi_hdmi_stop(hHdmi);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_stop[%d] failed with %#x!\n", hHdmi, s32Ret);
        return TD_FAILURE;
    }

    VAPILOG("HDMI[%d] has UnPlug.\n", hHdmi);
    return TD_SUCCESS;
}

static td_s32 vapi_vo_hdmi_hotplug(td_void *pPrivateData)
{
    // BUG 回调回来的数据只有结构体的第一个数据有效，不确定SDK内部是怎么处理这个数据的，保险起见，只用enHdmi
    td_s32                  s32Ret = TD_SUCCESS;
    HDMI_ARGS_S            *pArgs  = (HDMI_ARGS_S *)pPrivateData;
    ot_hdmi_id              hHdmi  = pArgs->enHdmi;
    ot_hdmi_attr            stHdmiAttr;
    ot_hdmi_sink_capability stSinkCap;
    td_s32                  update_flag = 0;
    SCREEN_RES_E            enRes;
    ot_hdmi_video_format    fmt;
    td_s32                  w, h;

    HDMI_ARGS_S *hdmi_arg = NULL;
    for (int i = 0; i < VAPI_VO_DEV_NUM; ++i) {
        if (g_stVoChn.stHdmiArgs[i].enHdmi == hHdmi) {
            hdmi_arg = &g_stVoChn.stHdmiArgs[i];
            break;
        }
    }
    if (!hdmi_arg) {
        return TD_FAILURE;
    }

    VAPILOG("----HDMI[%d] HotPlug begin----\n", hHdmi);

    s32Ret = ss_mpi_hdmi_get_sink_capability(hHdmi, &stSinkCap);
    s32Ret = ss_mpi_hdmi_get_attr(hHdmi, &stHdmiAttr);
    if (TD_FALSE == stSinkCap.is_connected) {
        return TD_FAILURE;
    }
    if (TD_TRUE == stSinkCap.support_hdmi) {
        stHdmiAttr.hdmi_en         = TD_TRUE;
        stHdmiAttr.audio_en        = TD_TRUE;
        stHdmiAttr.deep_color_mode = OT_HDMI_DEEP_COLOR_24BIT;
        stHdmiAttr.bit_depth       = OT_HDMI_BIT_DEPTH_16;
    } else {
        stHdmiAttr.hdmi_en         = TD_FALSE;
        stHdmiAttr.audio_en        = TD_FALSE;
        stHdmiAttr.deep_color_mode = OT_HDMI_DEEP_COLOR_24BIT;
        stHdmiAttr.sample_rate     = OT_HDMI_SAMPLE_RATE_32K;
        stHdmiAttr.bit_depth       = OT_HDMI_BIT_DEPTH_16;
    }

    td_bool support_video_format = stSinkCap.support_video_format[hdmi_arg->eForceFmt];
    VAPILOG("--eForceFmt = [%d]", hdmi_arg->eForceFmt);
    VAPILOG("--support_hdmi = [%d]", stSinkCap.support_hdmi);
    VAPILOG("--support_ycbcr = [%d]", stSinkCap.support_ycbcr);
    VAPILOG("--support_video_format[%d] = [%d]", hdmi_arg->eForceFmt, support_video_format);
    VAPILOG("--native_video_format = [%d]", stSinkCap.native_video_format);

    if (stSinkCap.support_hdmi == TD_FALSE || stSinkCap.support_ycbcr == TD_FALSE) {
        if (hdmi_arg->enIntfType & OT_VO_INTF_HDMI) {
            ot_vo_hdmi_param param;
            s32Ret               = ss_mpi_vo_get_hdmi_param(hdmi_arg->enDev, &param);
            param.csc.csc_matrix = OT_VO_CSC_MATRIX_BT709FULL_TO_RGBFULL;
            s32Ret               = ss_mpi_vo_set_hdmi_param(hdmi_arg->enDev, &param);
        }
        if (hdmi_arg->enIntfType & OT_VO_INTF_HDMI1) {
            ot_vo_hdmi_param param;
            s32Ret               = ss_mpi_vo_get_hdmi1_param(hdmi_arg->enDev, &param);
            param.csc.csc_matrix = OT_VO_CSC_MATRIX_BT709FULL_TO_RGBFULL;
            s32Ret               = ss_mpi_vo_set_hdmi1_param(hdmi_arg->enDev, &param);
        }
    }

    if ((hdmi_arg->eForceFmt >= OT_HDMI_VIDEO_FORMAT_1080P_60 && hdmi_arg->eForceFmt < OT_HDMI_VIDEO_FORMAT_BUTT && support_video_format) ||
        vapi_vo_check_edid(hHdmi, hdmi_arg->enIntfSync)) {
        stHdmiAttr.video_format = hdmi_arg->eForceFmt;
        update_flag             = 0;
    } else {
        stHdmiAttr.video_format = stSinkCap.native_video_format;
        //        VAPILOG("HDMI format[%d] unsupported! use default format[%d]",
        //                pArgs->eForceFmt, stHdmiAttr.enVideoFmt);
        update_flag             = 1;
    }

    VAPILOG("--update_flag = [%d]", update_flag);

    s32Ret = ss_mpi_hdmi_set_attr(hHdmi, &stHdmiAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_set_attr failed with %#x!\n", s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_hdmi_start(hHdmi);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_start failed with %#x!\n", s32Ret);
        return TD_FAILURE;
    }

    SCREEN_E screen = hdmi_arg->enScreen;
    if (update_flag) {
        for (enRes = SCREEN_RES_NUM - 1; enRes > SCREEN_UNDEFINE; enRes--) {
            fmt = vapi_vo_get_srceen_hdmi(enRes);
            if (stSinkCap.support_video_format[fmt]) {
                vapi_vo_calc_w_h(enRes, &w, &h);
                VAPILOG("HDMI current screen resolution[%d][%dX%d] \n", enRes, w, h);
                g_stVoChn.stAdapt.enRes[screen] = enRes;
                break;
            }
        }
        update_flag = 0;
    } else {
        g_stVoChn.stAdapt.enRes[screen] = SCREEN_UNDEFINE;
    }

    VAPILOG("----HDMI[%d] HotPlug end----\n", hHdmi);

    return s32Ret;
}

td_void vapi_vo_hdmi_event(ot_hdmi_event_type event, td_void *pPrivateData)
{
    comm_mutex_lock(&g_stVoChn.mutex);
    switch (event) {
    case OT_HDMI_EVENT_HOTPLUG:
        vapi_vo_hdmi_hotplug(pPrivateData);
        break;
    case OT_HDMI_EVENT_NO_PLUG:
        vapi_vo_hdmi_unplug(pPrivateData);
        break;
    case OT_HDMI_EVENT_EDID_FAIL:
        break;
    default:
        VAPILOG("un-known event:%d\n", event);
        break;
    }
    comm_mutex_unlock(&g_stVoChn.mutex);
    return;
}

static td_s32 vapi_vo_start_hdmi(ot_vo_intf_sync enIntfSync, ot_hdmi_id hdmi_id)
{
    td_s32                s32Ret = TD_SUCCESS;
    ot_hdmi_video_format  enVideoFmt;
    ot_hdmi_attr          stAttr         = {0};
    ot_hdmi_callback_func stCallbackFunc = {0};

    HDMI_ARGS_S *hdmi_arg = NULL;
    for (int i = 0; i < VAPI_VO_DEV_NUM; ++i) {
        if (g_stVoChn.stHdmiArgs[i].enHdmi == hdmi_id) {
            hdmi_arg = &g_stVoChn.stHdmiArgs[i];
            break;
        }
    }
    if (!hdmi_arg) {
        return TD_FAILURE;
    }

    vapi_vo_hdmi_get_videofmt(enIntfSync, &enVideoFmt);
    s32Ret |= ss_mpi_hdmi_init();
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_init failed with %#x!\n", s32Ret);
    }

    hdmi_arg->eForceFmt  = enVideoFmt;
    hdmi_arg->enIntfSync = enIntfSync;

    s32Ret |= ss_mpi_hdmi_open(hdmi_id);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_open failed with %#x!\n", s32Ret);
    }

    if (g_stVoChn.isHotplug == TD_FALSE) {
        s32Ret = ss_mpi_hdmi_get_attr(hdmi_id, &stAttr);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("ss_mpi_hdmi_get_attr failed with %#x!\n", s32Ret);
            return TD_FAILURE;
        }
        stAttr.hdmi_en             = TD_TRUE;
        stAttr.video_format        = enVideoFmt;
        stAttr.deep_color_mode     = OT_HDMI_DEEP_COLOR_24BIT;
        stAttr.audio_en            = TD_TRUE;
        stAttr.sample_rate         = OT_HDMI_SAMPLE_RATE_32K;
        stAttr.bit_depth           = OT_HDMI_BIT_DEPTH_16;
        stAttr.auth_mode_en        = TD_FALSE;
        stAttr.deep_color_adapt_en = TD_FALSE;

        s32Ret = ss_mpi_hdmi_set_attr(hdmi_id, &stAttr);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("ss_mpi_hdmi_set_attr failed with %#x!\n", s32Ret);
            return TD_FAILURE;
        }

        s32Ret = ss_mpi_hdmi_start(hdmi_id);
        VAPILOG("ss_mpi_hdmi_start[%d] with %#x!\n", hdmi_id, s32Ret);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("ss_mpi_hdmi_start failed with %#x!\n", s32Ret);
            return TD_FAILURE;
        }
    } else {
        comm_mutex_lock(&g_stVoChn.mutex);
        stCallbackFunc.hdmi_event_callback = vapi_vo_hdmi_event;
        stCallbackFunc.private_data        = hdmi_arg;
        s32Ret |= ss_mpi_hdmi_register_callback(hdmi_id, &stCallbackFunc);
        comm_mutex_unlock(&g_stVoChn.mutex);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("ss_mpi_hdmi_register_callback failed with %#x!\n", s32Ret);
            ss_mpi_hdmi_close(hdmi_id);
            ss_mpi_hdmi_deinit();
        }
    }

    return s32Ret;
}

static td_s32 vapi_vo_stop_hdmi(ot_hdmi_id hdmi_id)
{
    td_s32                s32Ret = TD_SUCCESS;
    ot_hdmi_callback_func stCallbackFunc;

    HDMI_ARGS_S *hdmi_arg = NULL;
    for (int i = 0; i < VAPI_VO_DEV_NUM; ++i) {
        if (g_stVoChn.stHdmiArgs[i].enHdmi == hdmi_id) {
            hdmi_arg = &g_stVoChn.stHdmiArgs[i];
            break;
        }
    }
    if (!hdmi_arg) {
        return TD_FAILURE;
    }

    s32Ret |= ss_mpi_hdmi_stop(hdmi_id);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_stop[%d] failed with %#x!\n", hdmi_id, s32Ret);
    }

    if (g_stVoChn.isHotplug == TD_TRUE) {

        comm_mutex_lock(&g_stVoChn.mutex);
        stCallbackFunc.hdmi_event_callback = vapi_vo_hdmi_event;
        stCallbackFunc.private_data        = hdmi_arg;

        s32Ret |= ss_mpi_hdmi_unregister_callback(hdmi_id, &stCallbackFunc);
        comm_mutex_unlock(&g_stVoChn.mutex);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("ss_mpi_hdmi_unregister_callback[%d] failed with %#x!\n", hdmi_id, s32Ret);
        }
    }

    s32Ret |= ss_mpi_hdmi_close(hdmi_id);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_close[%d] failed with %#x!\n", hdmi_id, s32Ret);
    }

    return s32Ret;
}

static void vapi_vo_start_hdmi_sub(ot_vo_intf_sync enIntfSync)
{
    td_s32 Hfd = -1;
    td_s32 Vfd = -1;
    td_s32 hdmi_mode, vga_mode;
    td_s32 enable_timer = 0;
    td_s32 s32Ret       = TD_SUCCESS;

    vapi_vo_get_sub_dev_mode(enIntfSync, &hdmi_mode, &vga_mode);

    VAPIINFO("enIntfSync[%d] vga_mode[%d]", enIntfSync, vga_mode);
    // set sub vga dev config
    Vfd = open(DEV_SUB_VGA_NAME, O_RDWR, 0);
    if (Vfd != -1) {
        s32Ret = ioctl(Vfd, THS8200_CHOICE_MODE, &vga_mode);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG(" set sub vga dev mode[%d] failed \n", vga_mode);
        }
        close(Vfd);
    } else {
        VAPILOG("open sub vga dev failed \n");
    }
}

static td_s32 vapi_vo_stop_hdmi_sub()
{
    td_s32 Hfd          = -1;
    td_s32 enable_timer = 0;
    td_s32 s32Ret       = TD_SUCCESS;

    Hfd = open(DEV_SUB_HDMI_NAME, O_RDWR, 0);
    if (Hfd != -1) {
        if (g_stVoChn.isHotplug == TD_TRUE) {
            enable_timer = TD_FALSE;
            s32Ret       = ioctl(Hfd, SIL9024_SET_TIMER_POLL, &enable_timer);
            if (s32Ret != TD_SUCCESS) {
                VAPILOG(" set sub hdmi dev timer[%d] failed \n", enable_timer);
            }
        }
        close(Hfd);
    } else {
        VAPILOG("close sub hdmi dev failed \n");
    }

    return TD_SUCCESS;
}

static td_s32 vapi_vo_start_dev(ot_vo_dev VoDev, ot_vo_intf_sync enIntfSync, int isogeny)
{
    VAPIINFO("vapi_vo_start_dev[%d] enIntfSync[%d] isogeny[%d]", VoDev, enIntfSync, isogeny);

    td_s32         s32Ret      = TD_SUCCESS;
    ot_vo_pub_attr stVoPubAttr = {0};

    // HDMI1当做主屏，主屏和VGA绑定
    memset(&stVoPubAttr, 0, sizeof(stVoPubAttr));
    if (VoDev == VAPI_VO_DEV_DHD0) {
        stVoPubAttr.intf_sync = enIntfSync;
        // HDMI max 3840x2160@60. VGA max  2560x1440@30
        if (enIntfSync > OT_VO_OUT_2560x1440_30) {
            stVoPubAttr.intf_type = OT_VO_INTF_HDMI1;
        } else {
            if (isogeny) {
                stVoPubAttr.intf_type = OT_VO_INTF_HDMI1 | OT_VO_INTF_VGA;
            } else {
                stVoPubAttr.intf_type = OT_VO_INTF_HDMI1;
            }
        }
        g_stVoChn.stHdmiArgs[VoDev].enDev      = VoDev;
        g_stVoChn.stHdmiArgs[VoDev].enHdmi     = OT_HDMI_ID_1;
        g_stVoChn.stHdmiArgs[VoDev].enIntfType = stVoPubAttr.intf_type;
        g_stVoChn.stHdmiArgs[VoDev].enScreen   = SCREEN_MAIN;
    } else if (VoDev == VAPI_VO_DEV_DHD1) {
        stVoPubAttr.intf_sync = enIntfSync;
        // HDMI max 3840x2160@60. VGA max  2560x1440@30
        if (enIntfSync > OT_VO_OUT_2560x1440_30) {
            stVoPubAttr.intf_type = OT_VO_INTF_HDMI;
        } else {
            if (isogeny) {
                stVoPubAttr.intf_type = OT_VO_INTF_HDMI | OT_VO_INTF_BT1120;
            } else {
                if (g_prefix == '7') {
                    stVoPubAttr.intf_type = OT_VO_INTF_HDMI  | OT_VO_INTF_VGA;
                } else{
                    stVoPubAttr.intf_type = OT_VO_INTF_HDMI;
                }
            }
        }
        g_stVoChn.stHdmiArgs[VoDev].enDev      = VoDev;
        g_stVoChn.stHdmiArgs[VoDev].enHdmi     = OT_HDMI_ID_0;
        g_stVoChn.stHdmiArgs[VoDev].enIntfType = stVoPubAttr.intf_type;
        g_stVoChn.stHdmiArgs[VoDev].enScreen   = SCREEN_SUB;
    } else {
        VAPILOG("dev[%d] don't support! \n", VoDev);
        return TD_FAILURE;
    }

    SCREEN_E screen                 = g_stVoChn.stHdmiArgs[VoDev].enScreen;
    g_stVoChn.stAdapt.enRes[screen] = SCREEN_UNDEFINE;

    ss_mpi_vo_set_pub_attr(VoDev, &stVoPubAttr);
    ss_mpi_vo_disable(VoDev); // clear first
    stVoPubAttr.bg_color = 0x00000000;
    s32Ret               = ss_mpi_vo_set_pub_attr(VoDev, &stVoPubAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_set_pub_attr[%d] failed with %#x!\n", VoDev, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vo_enable(VoDev);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_enable [%d]failed with %#x!\n", VoDev, s32Ret);
        return TD_FAILURE;
    }

    // config HDMI0&VGA0 device
    if (VoDev == VAPI_VO_DEV_DHD0) {
        vapi_vo_start_hdmi(enIntfSync, g_stVoChn.stHdmiArgs[VoDev].enHdmi);
    }
    // config HDMI1&VGA1 device
    else if (VoDev == VAPI_VO_DEV_DHD1) {
        vapi_vo_start_hdmi(enIntfSync, g_stVoChn.stHdmiArgs[VoDev].enHdmi);
        vapi_vo_start_hdmi_sub(enIntfSync);
    }

    return TD_SUCCESS;
}

static td_s32 vapi_vo_stop_dev(ot_vo_dev VoDev)
{
    td_s32 s32Ret = TD_SUCCESS;

    ot_hdmi_id hdmi_id = g_stVoChn.stHdmiArgs[VoDev].enHdmi;
    if (VoDev == VAPI_VO_DEV_DHD0) {
        s32Ret = vapi_vo_stop_hdmi(hdmi_id);
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("vapi_vo_stop_hdmi[%d] failed with %#x!\n", hdmi_id, s32Ret);
            return TD_FAILURE;
        }
    } else if (VoDev == VAPI_VO_DEV_DHD1) {
        s32Ret = vapi_vo_stop_hdmi(hdmi_id);
        // s32Ret = vapi_vo_stop_hdmi_sub();
        if (s32Ret != TD_SUCCESS) {
            VAPILOG("vapi_vo_stop_hdmi[%d] failed with %#x!\n", hdmi_id, s32Ret);
            return TD_FAILURE;
        }
    }
    s32Ret = ss_mpi_vo_disable(VoDev);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_disable [%d] failed with %#x!\n", VoDev, s32Ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static td_s32 vapi_vo_start_layer(ot_vo_layer VoLayer, ot_vo_intf_sync enIntfSync)
{
    td_s32                 s32Ret      = TD_SUCCESS;
    ot_vo_video_layer_attr stLayerAttr = {0};
    td_s32                 i;

    s32Ret = ss_mpi_vo_get_video_layer_attr(VoLayer, &stLayerAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_get_video_layer_attr [%d] failed with %#x!\n", VoLayer, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = vapi_vo_layer_get_videofmt(enIntfSync, &stLayerAttr.display_rect.width, &stLayerAttr.display_rect.height, &stLayerAttr.display_frame_rate);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_vo_layer_get_videofmt [%d] failed with %#x!\n", VoLayer, s32Ret);
        return TD_FAILURE;
    }

    if (VoLayer == VAPI_VO_LAYER_VPIP) {
        stLayerAttr.cluster_mode_en = TD_TRUE;
    } else {
        stLayerAttr.cluster_mode_en = TD_FALSE;
    }
    //    stLayerAttr.u32DispFrmRt = 25;
    //    td_u32 u32DispBufLen = 10;
    //
    //    s32Ret = HI_MPI_VO_SetDispBufLen(VoLayer, u32DispBufLen);
    //    if (s32Ret != TD_SUCCESS)
    //    {
    //        printf("Set display buf len failed with error code %#x!\n", s32Ret);
    //        return TD_FAILURE;
    //    }

    stLayerAttr.double_frame_en = TD_FALSE;
    stLayerAttr.pixel_format    = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stLayerAttr.img_size.width  = stLayerAttr.display_rect.width;
    stLayerAttr.img_size.height = stLayerAttr.display_rect.height;
    stLayerAttr.display_buf_len = 3;
    stLayerAttr.partition_mode  = OT_VO_PARTITION_MODE_MULTI;

    s32Ret = ss_mpi_vo_set_video_layer_attr(VoLayer, &stLayerAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_set_video_layer_attr [%d] failed with %#x!\n", VoLayer, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vo_enable_video_layer(VoLayer);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_enable_video_layer [%d] failed with %#x!\n", VoLayer, s32Ret);
        return TD_FAILURE;
    }

    for (i = 0; i < MAX_CHN_NUM; i++) {
        g_stVoChn.stChn[VoLayer][i].enState = STATE_IDLE;
    }

    return TD_SUCCESS;
}

static td_s32 vapi_vo_stop_layer(ot_vo_layer VoLayer)
{
    td_s32 s32Ret = TD_SUCCESS;
    td_s32 i;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        vapi_vo_chn_destory(VoLayer, i);
    }

    s32Ret = ss_mpi_vo_disable_video_layer(VoLayer);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_disable_video_layer [%d] failed with %#x!\n", VoLayer, s32Ret);
        return TD_FAILURE;
    }

    return s32Ret;
}

static void *vapi_vo_adaptive_pthread(void *arg)
{
    td_s32 i;

    comm_task_set_name("adaptive");

    while (g_stVoChn.stAdapt.isRun) {
        for (i = 0; i < SCREEN_NUM; i++) {
            if (!g_stVoChn.isDualScreen && i == SCREEN_SUB)
                continue;

            if (g_stVoChn.stAdapt.enRes[i] != SCREEN_UNDEFINE) {
                if (g_stVoChn.update_screen_res_cb != NULL) {
                    g_stVoChn.update_screen_res_cb(i, g_stVoChn.stAdapt.enRes[i]);
                    g_stVoChn.stAdapt.enRes[i] = SCREEN_UNDEFINE;
                }
            }
        }
        usleep(500000);
    }

    return NULL;
}

static td_s32 vapi_vo_adaptive_start()
{
    td_s32 s32Ret = TD_SUCCESS;

    g_stVoChn.stAdapt.isRun = TD_TRUE;

    s32Ret = comm_task_create_join(&g_stVoChn.stAdapt.handle, vapi_vo_adaptive_pthread, NULL);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("create vapi_vo_adaptive_pthread failed! \n");
    }

    return s32Ret;
}

static td_s32 vapi_vo_adaptive_stop()
{
    td_s32 s32Ret = TD_SUCCESS;

    g_stVoChn.stAdapt.isRun = TD_FALSE;

    s32Ret = comm_task_join(&g_stVoChn.stAdapt.handle);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("destroy vapi_vo_adaptive_pthread failed! \n");
    }

    return s32Ret;
}

td_s32 vapi_vo_start_pip_layer(SCREEN_RES_E enRes)
{
    return TD_SUCCESS; // discard
    ot_vo_intf_sync enIntfSync;
    td_s32          s32Ret = TD_SUCCESS;

    enIntfSync = vapi_vo_get_srceen_sync(enRes);
    s32Ret     = vapi_vo_start_layer(VAPI_VO_LAYER_VPIP, enIntfSync);

    return s32Ret;
}

td_s32 vapi_vo_stop_pip_layer()
{
    return TD_SUCCESS; // discard
    td_s32 s32Ret = TD_SUCCESS;

    s32Ret = vapi_vo_stop_layer(VAPI_VO_LAYER_VPIP);

    return s32Ret;
}

td_s32 vapi_vo_pip_rebind(ot_vo_dev VoDev)
{
    return TD_SUCCESS; // discard
    td_s32 s32Ret = TD_SUCCESS;

    s32Ret = vapi_vo_stop_pip_layer();
    if (s32Ret != TD_SUCCESS) {
        return TD_FAILURE;
    }
    //
    s32Ret = ss_mpi_vo_unbind_layer(VAPI_VO_LAYER_VPIP, !VoDev);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_unbind_layer [%d] failed with %#x!\n", VAPI_VO_LAYER_VPIP, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vo_bind_layer(VAPI_VO_LAYER_VPIP, VoDev);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_bind_layer [%d->%d] failed with %#x!\n", VAPI_VO_LAYER_VPIP, VoDev, s32Ret);
        return TD_FAILURE;
    }

    //
    s32Ret = vapi_vo_start_pip_layer(g_stVoChn.enRes[VoDev]);
    if (s32Ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    g_stVoChn.enPipBind = VoDev;

    return TD_SUCCESS;
}

td_void vapi_vo_get_srceen_res(SCREEN_E enScreen, td_s32 *W, td_s32 *H)
{
    SCREEN_RES_E enRes;

    enRes = g_stVoChn.enRes[enScreen];
    vapi_vo_calc_w_h(enRes, W, H);
}

td_s32 vapi_vo_set_csc(ot_vo_layer VoLayer, CSC_E enCSC)
{
    ot_vo_csc stVideoCSC = {0};
    td_s32    s32Ret = TD_SUCCESS;

    s32Ret = ss_mpi_vo_get_layer_csc(VoLayer, &stVideoCSC);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_get_layer_csc [%d] failed with %#x!\n", VoLayer, s32Ret);
        return TD_FAILURE;
    }

    switch (enCSC) {
    case CSC_STANDARD:
        stVideoCSC.luma       = 65; // 74;
        stVideoCSC.contrast   = 40; // 45;
        stVideoCSC.hue        = 48; // 60;
        stVideoCSC.saturation = 50; // 50;
        break;
    case CSC_LIGHTNESS:
        stVideoCSC.luma       = 60;
        stVideoCSC.contrast   = 50;
        stVideoCSC.hue        = 50;
        stVideoCSC.saturation = 45;
        break;
    case CSC_SOFT:
        stVideoCSC.luma       = 45;
        stVideoCSC.contrast   = 50;
        stVideoCSC.hue        = 50;
        stVideoCSC.saturation = 45;
        break;
    case CSC_COLORFUL:
        stVideoCSC.luma       = 50;
        stVideoCSC.contrast   = 50;
        stVideoCSC.hue        = 50;
        stVideoCSC.saturation = 60;
        break;
    default:
        stVideoCSC.luma       = 74;
        stVideoCSC.contrast   = 45;
        stVideoCSC.hue        = 60;
        stVideoCSC.saturation = 50;
        break;
    }

    s32Ret = ss_mpi_vo_set_layer_csc(VoLayer, &stVideoCSC);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_set_layer_csc [%d] failed with %#x!\n", VoLayer, s32Ret);
        return TD_FAILURE;
    }

    g_stVoChn.enCSC[VoLayer] = enCSC;

    return TD_SUCCESS;
}

td_s32 vapi_vo_chn_create(ot_vo_layer VoLayer, ot_vo_chn VoChn, ot_rect *pstRect, RATIO_E enRatio)
{
    td_s32          s32Ret     = TD_SUCCESS;
    ot_vo_chn_attr  stChnAttr  = {0};
    ot_vo_chn_param stChnParam = {0};

    if (g_stVoChn.stChn[VoLayer][VoChn].enState == STATE_BUSY) {
        VAPILOG("vout channel[%d][%d] has existed !\n", VoLayer, VoChn);
        return TD_FAILURE;
    }

    stChnAttr.rect.x       = ALIGN_BACK(pstRect->x, 2);
    stChnAttr.rect.y       = ALIGN_BACK(pstRect->y, 2);
    stChnAttr.rect.width   = ALIGN_BACK(pstRect->width, 2);
    stChnAttr.rect.height  = ALIGN_BACK(pstRect->height, 2);
    stChnAttr.priority     = 0;
    stChnAttr.deflicker_en = TD_FALSE;

    s32Ret = ss_mpi_vo_set_chn_attr(VoLayer, VoChn, &stChnAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_set_chn_attr [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vo_get_chn_param(VoLayer, VoChn, &stChnParam);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_get_chn_param [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }
    stChnParam.aspect_ratio.mode = enRatio;

    s32Ret = ss_mpi_vo_set_chn_param(VoLayer, VoChn, &stChnParam);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_set_chn_param [%d][%d] set ratio [%d] failed with %#x!\n", VoLayer, VoChn, enRatio, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vo_enable_chn(VoLayer, VoChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_enable_chn [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }

    g_stVoChn.stChn[VoLayer][VoChn].enState  = STATE_BUSY;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.x = pstRect->x;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.y = pstRect->y;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.w = pstRect->width;
    g_stVoChn.stChn[VoLayer][VoChn].stZone.h = pstRect->height;

    return TD_SUCCESS;
}

td_s32 vapi_vo_chn_destory(ot_vo_layer VoLayer, ot_vo_chn VoChn)
{
    td_s32 s32Ret = TD_SUCCESS;

    if (g_stVoChn.stChn[VoLayer][VoChn].enState != STATE_BUSY) {
        return TD_SUCCESS;
    }

    s32Ret = ss_mpi_vo_disable_chn(VoLayer, VoChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_disable_chn [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }

    g_stVoChn.stChn[VoLayer][VoChn].enState = STATE_IDLE;

    return TD_SUCCESS;
}

td_s32 vapi_vo_chn_clear_buff(ot_vo_layer VoLayer, ot_vo_chn VoChn)
{
    td_s32 s32Ret = TD_SUCCESS;

    s32Ret = ss_mpi_vo_clear_chn_buf(VoLayer, VoChn, TD_TRUE);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_clear_chn_buf [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_void vapi_vo_chn_clear_all_buff(ot_vo_layer VoLayer)
{
    td_s32 i;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        if (g_stVoChn.stChn[VoLayer][i].enState == STATE_BUSY) {
            vapi_vo_chn_clear_buff(VoLayer, i);
        }
    }
}

td_s32 vapi_vo_chn_set_fps(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_s32 s32Fps)
{
    td_s32 s32Ret = TD_SUCCESS;

    if (g_stVoChn.stChn[VoLayer][VoChn].enState == STATE_IDLE) {
        return TD_SUCCESS;
    }

    s32Ret = ss_mpi_vo_set_chn_frame_rate(VoLayer, VoChn, s32Fps);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_set_chn_frame_rate[%d] @[%d][%d] failed with %#x!\n", s32Fps, VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 vapi_vo_chn_get_wh(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_u32 *pu32Width, td_u32 *pu32Height)
{
    td_s32         s32Ret = TD_SUCCESS;
    ot_vo_chn_attr stChnAttr;

    if (g_stVoChn.stChn[VoLayer][VoChn].enState == STATE_IDLE) {
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_vo_get_chn_attr(VoLayer, VoChn, &stChnAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_get_chn_attr[%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }
    *pu32Width  = stChnAttr.rect.width;
    *pu32Height = stChnAttr.rect.height;

    return TD_SUCCESS;
}

td_s32 vapi_vo_bind_vpss(ot_vo_layer VoLayer, ot_vo_chn VoChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn)
{
    td_s32     s32Ret = TD_SUCCESS;
    ot_mpp_chn stSrcChn;
    ot_mpp_chn stDestChn;

    stSrcChn.mod_id = OT_ID_VPSS;
    stSrcChn.dev_id = VpssGrp;
    stSrcChn.chn_id = VpssChn;

    stDestChn.mod_id = OT_ID_VO;
    stDestChn.dev_id = VoLayer;
    stDestChn.chn_id = VoChn;

    s32Ret = ss_mpi_sys_bind(&stSrcChn, &stDestChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_sys_bind [%d->%d] failed with %#x!\n", VpssGrp, VoChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 vapi_vo_unbind_vpss(ot_vo_layer VoLayer, ot_vo_chn VoChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn)
{
    td_s32     s32Ret = TD_SUCCESS;
    ot_mpp_chn stSrcChn;
    ot_mpp_chn stDestChn;

    stSrcChn.mod_id = OT_ID_VPSS;
    stSrcChn.dev_id = VpssGrp;
    stSrcChn.chn_id = VpssChn;

    stDestChn.mod_id = OT_ID_VO;
    stDestChn.dev_id = VoLayer;
    stDestChn.chn_id = VoChn;

    s32Ret = ss_mpi_sys_unbind(&stSrcChn, &stDestChn);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_sys_unbind [%d->%d] failed with %#x!\n", VpssGrp, VoChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 vapi_vo_get_pipbind()
{
    return g_stVoChn.enPipBind;
}

td_void vapi_vo_update_state(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_s32 state)
{
    g_stVoChn.stChn[VoLayer][VoChn].enState = state;
}

td_bool vapi_vo_check_state(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_s32 state)
{
    return (g_stVoChn.stChn[VoLayer][VoChn].enState == state);
}

td_s32 vapi_vo_set_ratio(ot_vo_layer VoLayer, ot_vo_chn VoChn, RATIO_E enRatio)
{
    ot_vo_chn_param stChnParam;
    td_s32          s32Ret = TD_SUCCESS;

    s32Ret = ss_mpi_vo_get_chn_param(VoLayer, VoChn, &stChnParam);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_get_chn_param [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }

    switch (enRatio) {
    case RATIO_VO_AUTO:
        stChnParam.aspect_ratio.mode = OT_ASPECT_RATIO_AUTO;
        break;
    case RATIO_VO_FULL:
    default:
        stChnParam.aspect_ratio.mode = OT_ASPECT_RATIO_NONE;
        break;
    }

    s32Ret = ss_mpi_vo_set_chn_param(VoLayer, VoChn, &stChnParam);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_set_chn_param [%d][%d] failed with %#x!\n", VoLayer, VoChn, s32Ret);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 vapi_vo_dev_open(ot_vo_dev VoDev, SCREEN_RES_E enRes)
{
    td_s32          s32Ret = TD_SUCCESS;
    ot_vo_layer     VoLayer;
    ot_vo_intf_sync enIntfSync;

    enIntfSync = vapi_vo_get_srceen_sync(enRes);

    s32Ret = vapi_vo_start_dev(VoDev, enIntfSync, g_stVoChn.isogeny);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_vo_dev_open [%d] enIntfSync[%d] isogeny[%d] failed with %#x!\n", VoDev, enIntfSync, g_stVoChn.isogeny, s32Ret);
        return TD_FAILURE;
    }

    VoLayer = VoDev == VAPI_VO_DEV_DHD0 ? VAPI_VO_LAYER_VHD0 : VAPI_VO_LAYER_VHD1;
    s32Ret  = vapi_vo_start_layer(VoLayer, enIntfSync);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_vo_start_layer [%d] enIntfSync[%d] failed with %#x!\n", VoDev, enIntfSync, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = vapi_vo_set_csc(VoLayer, CSC_STANDARD);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_vo_set_csc [%d] failed with %#x!\n", CSC_STANDARD, s32Ret);
        return TD_FAILURE;
    }

    SCREEN_E screen         = g_stVoChn.stHdmiArgs[VoDev].enScreen;
    g_stVoChn.enRes[screen] = enRes;

    return TD_SUCCESS;
}

td_s32 vapi_vo_dev_close(ot_vo_dev VoDev)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_vo_layer VoLayer;

    VoLayer = VoDev == VAPI_VO_DEV_DHD0 ? VAPI_VO_LAYER_VHD0 : VAPI_VO_LAYER_VHD1;
    s32Ret  = vapi_vo_stop_layer(VoLayer);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_vo_stop_layer [%d] failed with %#x!\n", VoDev, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = vapi_vo_stop_dev(VoDev);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_vo_stop_dev [%d] failed with %#x!\n", VoDev, s32Ret);
        return TD_FAILURE;
    }

    g_stVoChn.enRes[VoDev] = SCREEN_UNDEFINE;

    return TD_SUCCESS;
}

td_s32 vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, int isHotplug, int isDualScreen, int isogeny, char prefix, VDEC_CB_S *pstCb)
{
    td_s32 s32Ret = TD_SUCCESS;

    g_stVoChn.update_screen_res_cb = pstCb->update_screen_res_cb;
    g_stVoChn.isHotplug            = isHotplug;
    g_stVoChn.isDualScreen         = isDualScreen;
    g_stVoChn.isogeny              = isogeny;
    g_prefix                       = prefix;

    comm_mutex_init(&g_stVoChn.mutex);
    s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_DHD0, enMain);
    if (s32Ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    if (isDualScreen) {
        s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_DHD1, enSub);
        if (s32Ret != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    s32Ret = vapi_vo_start_pip_layer(enMain);
    if (s32Ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    // pip defualt bind to VAPI_VO_DEV_DHD0
    g_stVoChn.enPipBind = VAPI_VO_DEV_DHD0;

    if (g_stVoChn.isHotplug == TD_TRUE) {
        s32Ret = vapi_vo_adaptive_start();
        if (s32Ret != TD_SUCCESS) {
            return TD_FAILURE;
        }
    }

    return TD_SUCCESS;
}

td_void vapi_vo_uninit()
{
    td_s32 s32Ret;
    vapi_vo_stop_pip_layer();
    vapi_vo_dev_close(VAPI_VO_DEV_DHD0);
    if (g_stVoChn.isDualScreen)
        vapi_vo_dev_close(VAPI_VO_DEV_DHD1);
    if (g_stVoChn.isHotplug == TD_TRUE) {
        vapi_vo_adaptive_stop();
        g_stVoChn.isHotplug = TD_FALSE;
    }
    s32Ret = ss_mpi_hdmi_deinit();
    if (s32Ret = TD_SUCCESS) {
        VAPILOG("ss_mpi_hdmi_deinit failed with %#x!\n", s32Ret);
    }

    g_stVoChn.update_screen_res_cb = NULL;
    comm_mutex_uninit(&g_stVoChn.mutex);
}
