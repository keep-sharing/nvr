/*
 * ***************************************************************
 * Filename:        vapi_vout.c
 * Created at:      2016.04.29
 * Description:     video output controller
 * Author:          zbing
 * Copyright (C)    milesight
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

#define ALIGN16_UP(x)       ALIGN_CEIL(x, 16)      /* for 16 bytes alignment */
#define ALIGN256_UP(x)      ALIGN_CEIL(x, 256)     /* for 256 bytes alignment */
#define ALIGN4096_UP(x)     ALIGN_CEIL(x, 4096)    /* for hw dma 4K bytes alignment */

/*  Video Descriptor Block  */
typedef struct HDMI_EDID_DTD_VIDEO {
    HI_U16  pixel_clock;        /* 54-55 */
    HI_U8   horiz_active;       /* 56 */
    HI_U8   horiz_blanking;     /* 57 */
    HI_U8   horiz_high;     /* 58 */
    HI_U8   vert_active;        /* 59 */
    HI_U8   vert_blanking;      /* 60 */
    HI_U8   vert_high;      /* 61 */
    HI_U8   horiz_sync_offset;  /* 62 */
    HI_U8   horiz_sync_pulse;   /* 63 */
    HI_U8   vert_sync_pulse;    /* 64 */
    HI_U8   sync_pulse_high;    /* 65 */
    HI_U8   horiz_image_size;   /* 66 */
    HI_U8   vert_image_size;    /* 67 */
    HI_U8   image_size_high;    /* 68 */
    HI_U8   horiz_border;       /* 69 */
    HI_U8   vert_border;        /* 70 */
    HI_U8   misc_settings;      /* 71 */
} HDMI_EDID_DTD_VIDEO_S;

typedef struct HDMI_EDID {
    HI_U8   header[8];      /* 00-07 */
    HI_U16  manufacturerID;     /* 08-09 */
    HI_U16  product_id;     /* 10-11 */
    HI_U32  serial_number;      /* 12-15 */
    HI_U8   week_manufactured;  /* 16 */
    HI_U8   year_manufactured;  /* 17 */
    HI_U8   edid_version;       /* 18 */
    HI_U8   edid_revision;      /* 19 */
    HI_U8   video_in_definition;    /* 20 */
    HI_U8   max_horiz_image_size;   /* 21 */
    HI_U8   max_vert_image_size;    /* 22 */
    HI_U8   display_gamma;      /* 23 */
    HI_U8   power_features;     /* 24 */
    HI_U8   chroma_info[10];    /* 25-34 */
    HI_U8   timing_1;       /* 35 */
    HI_U8   timing_2;       /* 36 */
    HI_U8   timing_3;       /* 37 */
    HI_U8   std_timings[16];    /* 38-53 */

    HDMI_EDID_DTD_VIDEO_S video[4]; /* 54-125 */

    HI_U8   extension_edid;     /* 126 */
    HI_U8   checksum;       /* 127 */
    HI_U8   extension_tag;      /* 00 (extensions follow EDID) */
    HI_U8   extention_rev;      /* 01 */
    HI_U8   offset_dtd;     /* 02 */
    HI_U8   num_dtd;        /* 03 */

    HI_U8   data_block[123];    /* 04 - 126 */
    HI_U8   extension_checksum; /* 127 */

    HI_U8   ext_datablock[256];
} HDMI_EDID_S;

typedef struct adaptive_s {
    pthread_t       handle;
    pthread_mutex_t mutex;
    HI_BOOL         isRun;
    HD_PATH_ID      devPath;
    SCREEN_RES_E    enRes[SCREEN_NUM];
} ADAPTIVE_S;

typedef struct voChn_s {
    CHNINFO_S       stChn[MAX_CHN_NUM];
    HD_PATH_ID      pathId[MAX_CHN_NUM];
    SCREEN_RES_E    enRes[SCREEN_NUM];
    int (*update_screen_res_cb)(int screen, int res);
    ADAPTIVE_S      stAdapt;
    HI_BOOL         isHotplug;
    HI_BOOL         isQuick;
} VOCHN_S;

static VOCHN_S g_stVoChn;

static void vo_get_mode(SCREEN_RES_E enRes, HD_VIDEOOUT_MODE *pstMode)
{
    pstMode->output_type = HD_COMMON_VIDEO_OUT_HDMI;
    switch (enRes) {
        case SCREEN_1024X768_60:
            pstMode->input_dim = HD_VIDEOOUT_IN_1024x768;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_1024X768P60;
            break;
        case SCREEN_1280X720_60:
            pstMode->input_dim = HD_VIDEOOUT_IN_1280x720;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_1280X720P60;
            break;
        case SCREEN_1600X1200_60:
            pstMode->input_dim = HD_VIDEOOUT_IN_1600x1200;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_1600X1200P60;
            break;
        case SCREEN_1920X1080_30:
            pstMode->input_dim = HD_VIDEOOUT_IN_1920x1080;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_1920X1080P30;
            break;
        case SCREEN_1920X1080_50:
            pstMode->input_dim = HD_VIDEOOUT_IN_1920x1080;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_1920X1080P50;
            break;
        case SCREEN_1920X1080_60:
            pstMode->input_dim = HD_VIDEOOUT_IN_1920x1080;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_1920X1080P60;
            break;
        case SCREEN_3840X2160_30:
            pstMode->input_dim = HD_VIDEOOUT_IN_3840x2160;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_3840X2160P30;
            break;
        case SCREEN_1280X1024_60:
            pstMode->input_dim = HD_VIDEOOUT_IN_1280x1024;
            pstMode->output_mode.hdmi = HD_VIDEOOUT_HDMI_1280X1024P60;
            break;
        default:
            VAPILOG("not support enRes[%d]\n", enRes);
            assert(0);
            break;
    }
}

static void vapi_vo_calc_w_h(SCREEN_RES_E enRes, HI_S32 *W, HI_S32 *H)
{
    switch (enRes) {
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
        default:
            VAPILOG("can not support srceen resolution[%d]\n\n", enRes);
            *W  = 0;
            *H  = 0;
            break;
    }

}

static HD_RESULT clear_buffer_black(DEV_E voDev, int pxlfmt, int width, int height)
{
    UINT i;
    void *pfb0Va = 0;
    UINT *pbuf;
    HD_COMMON_MEM_POOL_INFO pooLDisp;
    UINT bufPaddr;
    UINT bufSize;

    if (voDev == VAPI_VO_DEV_VIDEO) {
        pooLDisp.type = HD_COMMON_MEM_DISP0_IN_POOL;
    } else {
        pooLDisp.type = HD_COMMON_MEM_DISP0_FB_POOL;
    }

    pooLDisp.ddr_id = DDR_ID0;
    if (hd_common_mem_get(HD_COMMON_MEM_PARAM_POOL_CONFIG, (VOID *)&pooLDisp) != HD_OK) {
        VAPILOG("Error hd_common_mem_get:pool_type %d ddr_id %d\r\n", pooLDisp.type, pooLDisp.ddr_id);
        return HD_ERR_NG;
    }

    bufSize = pooLDisp.blk_size * pooLDisp.blk_cnt;
    bufPaddr = pooLDisp.start_addr;

    pfb0Va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, bufPaddr, bufSize);
    if (pfb0Va == NULL) {
        printf("hd_common_mem_mmap pa(0x%x) fail\n", (int)bufPaddr);
        return HD_ERR_NG;
    }
    pbuf = (UINT *)(pfb0Va);
    if (pxlfmt == HD_VIDEO_PXLFMT_ARGB1555 || pxlfmt == HD_VIDEO_PXLFMT_ARGB8888) {
        memset(pbuf, 0, bufSize);
    }  else if (pxlfmt == HD_VIDEO_PXLFMT_YUV420_NVX3) {
        ////(c400 0000 0000 0000 0000 0000) y pattern for y_size
        ////(e000 0000 0000 0000 0000 0000) uv pattern
        UINT y_size = ((width * height * 3) >> 2);
        UINT yuv_size = ((width * height * 9) >> 3);//(w*h*3/2)*0.75
        UINT loop_cnt = (yuv_size >> 2);
        UINT buf_offset = 0;
        UINTPTR buf_start = 0;
        UINT buf_len = 0;
        //fill first buf to yuv420_sce black
        for (i = 0; i < loop_cnt; i++) {
            buf_offset = ((i * 4) % yuv_size);
            if (buf_offset < y_size) {
                pbuf[i++] = 0x000000c4;
                pbuf[i++] = 0;
                pbuf[i] = 0;
            } else {
                pbuf[i++] = 0x000000e0;
                pbuf[i++] = 0;
                pbuf[i] = 0;
            }

        }
        buf_start = ALIGN256_UP(((UINTPTR)pbuf + yuv_size));
        //copy first buf to others buf,that align to 256 start addr
        while (ALIGN256_UP(buf_start + yuv_size) < ((UINTPTR)pfb0Va + bufSize)) {
            if ((((UINTPTR)pfb0Va + bufSize) - buf_start) > yuv_size) {
                buf_len = yuv_size;
                memcpy((void *)buf_start, (void *)pbuf, buf_len);
            }
            buf_start = ALIGN256_UP(buf_start + yuv_size);
        }

    }  else if (pxlfmt == HD_VIDEO_PXLFMT_YUV420) {

        UINT y_size = (width * height);
        UINT yuv_size = ((width * height * 3) >> 1);//(w*h*3/2)
        UINT loop_cnt = (yuv_size >> 2);
        UINT buf_offset = 0;
        UINTPTR buf_start = 0;
        UINT buf_len = 0;
        //fill first buf to yuv420 black
        for (i = 0; i < loop_cnt; i++) {
            buf_offset = ((i * 4) % yuv_size);
            if (buf_offset < y_size) {
                pbuf[i] = 0x10101010;
            } else {
                pbuf[i] = 0x80808080;
            }

        }
        buf_start = ALIGN256_UP(((UINTPTR)pbuf + yuv_size));
        //copy first buf to others buf,that align to 256 start addr
        while (ALIGN256_UP(buf_start + yuv_size) < ((UINTPTR)pfb0Va + bufSize)) {
            if ((((UINTPTR)pfb0Va + bufSize) - buf_start) > yuv_size) {
                buf_len = yuv_size;
                memcpy((void *)buf_start, (void *)pbuf, buf_len);
            }
            buf_start = ALIGN256_UP(buf_start + yuv_size);
        }

    } else { /*yuv422*/
        ///UINT yuv_size = ALIGN16_UP(width) * ALIGN16_UP(height); /* internal buffer allocation is 16 alignment */
        for (i = 0; i < (bufSize >> 2); i++) {
            if (pxlfmt == HD_VIDEO_PXLFMT_YUV422_ONE) {
                pbuf[i] = 0x10801080;
            }
        }
    }

    if (HD_OK != hd_common_mem_munmap(pfb0Va, bufSize)) {
        VAPILOG("hd_common_mem_munmap fail\n");
        return HD_ERR_NG;
    }

    return HD_OK;
}

static HD_RESULT clear_win(HD_PATH_ID devPath, UINT8 mode, HD_URECT *region, UINT32 pattern)
{
#define BUFFER_SIZE     (1024*1024)
    HD_RESULT ret = HD_OK;
    UINT32 i;
    HD_VIDEOOUT_CLEAR_WIN win;

    UINT64                pool = HD_COMMON_MEM_USER_BLK;
    HD_COMMON_MEM_VB_BLK  blk;
    char *clearwin_buffer_va;
    UINTPTR               addr;

    blk = hd_common_mem_get_block(pool, BUFFER_SIZE, 0);
    if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
        VAPILOG("hd_common_mem_get_block fail\r\n");
        return HD_ERR_SYS;
    }
    addr = hd_common_mem_blk2pa(blk);
    if (addr == 0) {
        VAPILOG("hd_common_mem_blk2pa fail, blk = %#lx\r\n", blk);
        hd_common_mem_release_block(blk);
        return HD_ERR_SYS;
    }
    clearwin_buffer_va = (char *)hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, addr, BUFFER_SIZE);
    if (clearwin_buffer_va == 0) {
        VAPILOG("hd_common_mem_mmap fail, blk = %#lx\r\n", blk);
        hd_common_mem_release_block(blk);
        return HD_ERR_SYS;
    }

    win.input_dim.w = 512; /* min is 128x32 when two channel scaling, otherwise 64x32, 4K display needs 256x128 */
    win.input_dim.h = 512; /* min is 128x32 when two channel scaling, otherwise 64x32 */
    win.in_fmt = HD_VIDEO_PXLFMT_YUV422_ONE;
    win.buf = (unsigned char *)clearwin_buffer_va;

    for (i = 0; i < (win.input_dim.w * win.input_dim.h / 2); i++) {
        *(UINT *)(win.buf + 4 * i) = pattern;
    }
    win.output_rect.x = region->x;
    win.output_rect.y = region->y;
    win.output_rect.w = region->w;
    win.output_rect.h = region->h;
    win.mode = (CLEAR_WINDOW_MODE)mode;

    if (hd_videoout_set(devPath, HD_VIDEOOUT_PARAM_CLEAR_WIN, &win) != HD_OK) {
        VAPILOG("error in clear_window\n");
        return ret;
    }

    ret = hd_common_mem_munmap(clearwin_buffer_va, BUFFER_SIZE);
    if (ret != HD_OK) {
        VAPILOG("hd_common_mem_munmap fail\n");
        return ret;
    }
    ret = hd_common_mem_release_block(blk);
    if (ret != HD_OK) {
        VAPILOG("hd_common_mem_release_block fail\n");
        return ret;
    }

    return ret;
}

static void *vapi_vo_adaptive_pthread(void *arg)
{
    HI_S32 i;

    comm_task_set_name("adaptive");

    while (g_stVoChn.stAdapt.isRun) {
        for (i = 0; i < SCREEN_NUM; i++) {
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

static HI_S32 vapi_vo_adaptive_start()
{
    HD_RESULT ret = HD_OK;

    g_stVoChn.stAdapt.isRun = HI_TRUE;

    ret = comm_task_create_join(&g_stVoChn.stAdapt.handle, vapi_vo_adaptive_pthread, NULL);
    if (ret != HD_OK) {
        VAPILOG("create vapi_vo_adaptive_pthread failed! \n");
    }

    return ret;
}

static HI_S32 vapi_vo_adaptive_stop()
{
    HD_RESULT ret = HD_OK;

    g_stVoChn.stAdapt.isRun = HI_FALSE;

    ret = comm_task_join(&g_stVoChn.stAdapt.handle);
    if (ret != HD_OK) {
        VAPILOG("destroy vapi_vo_adaptive_pthread failed! \n");
    }

    return ret;
}

HI_VOID vapi_vo_get_srceen_res(SCREEN_E enScreen, HI_S32 *W, HI_S32 *H)
{
    SCREEN_RES_E enRes;

    enRes = g_stVoChn.enRes[enScreen];
    vapi_vo_calc_w_h(enRes, W, H);
}

HD_RESULT vapi_vo_get_win_rect(HI_S32 voChn, HD_URECT  *pstRect, HD_DIM *pstDim)
{
    HD_RESULT ret = HD_OK;
    HD_VIDEOOUT_SYSCAPS sysCaps;

    /* get lcd capability */
    if ((ret = hd_videoout_get(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_SYSCAPS, &sysCaps)) != HD_OK) {
        VAPILOG("hd_videoout_get HD_VIDEOOUT_PARAM_SYSCAPS voChn[%d] failed with %d !\n", voChn, ret);
        return ret;
    }

    pstDim->w = sysCaps.input_dim.w;
    pstDim->h = sysCaps.input_dim.h;
    pstRect->x = g_stVoChn.stChn[voChn].stZone.x;
    pstRect->y = g_stVoChn.stChn[voChn].stZone.y;
    pstRect->w = g_stVoChn.stChn[voChn].stZone.w;
    pstRect->h = g_stVoChn.stChn[voChn].stZone.h;

    return ret;

}

HD_RESULT vapi_vo_get_fb_fmt(HD_FB_FMT *pstFbFmt)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoout_get(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_FB_FMT, pstFbFmt)) != HD_OK) {
        VAPILOG("hd_videoout_get HD_VIDEOOUT_PARAM_FB_FMT failed with %d !\n", ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vo_chn_create(HI_S32 voChn, HD_URECT  *pstRect, RATIO_E enRatio)
{
    HD_RESULT ret = HD_OK;
    HD_PATH_ID pathId = 0;
    HD_VIDEOOUT_WIN_ATTR win;

    if (g_stVoChn.stChn[voChn].enState == STATE_BUSY) {
        VAPILOG("vout channel[%d] has existed !\n", voChn);
        return ret;
    }

    if ((ret = hd_videoout_open(HD_VIDEOOUT_IN(0, voChn), HD_VIDEOOUT_OUT(0, 0), &pathId)) != HD_OK) {
        VAPILOG("hd_videoout_open voChn[%d] failed with %d !\n", voChn, ret);
        return ret;
    }


    /* Set videoout input setting */
    win.rect.x = pstRect->x;
    win.rect.y = pstRect->y;
    win.rect.w = pstRect->w;
    win.rect.h = pstRect->h;
    win.visible = 1;
    if ((ret = hd_videoout_set(pathId, HD_VIDEOOUT_PARAM_IN_WIN_ATTR, &win)) != HD_OK) {
        VAPILOG("hd_videoout_set HD_VIDEOOUT_PARAM_IN_WIN_ATTR voChn[%d] failed with %d !\n", voChn, ret);
        return ret;
    }

    g_stVoChn.stChn[voChn].enState = STATE_BUSY;
    g_stVoChn.stChn[voChn].stZone.x = pstRect->x;
    g_stVoChn.stChn[voChn].stZone.y = pstRect->y;
    g_stVoChn.stChn[voChn].stZone.w = pstRect->w;
    g_stVoChn.stChn[voChn].stZone.h = pstRect->h;
    g_stVoChn.pathId[voChn] = pathId;

    return ret;
}

HD_RESULT vapi_vo_chn_destory(HI_S32 voChn)
{
    HD_RESULT ret = HD_OK;
    HD_URECT region;

    if (g_stVoChn.stChn[voChn].enState != STATE_BUSY) {
        return ret;
    }

    region.x = g_stVoChn.stChn[voChn].stZone.x;
    region.y = g_stVoChn.stChn[voChn].stZone.y;
    region.w = g_stVoChn.stChn[voChn].stZone.w;
    region.h = g_stVoChn.stChn[voChn].stZone.h;
    clear_win(g_stVoChn.stAdapt.devPath, HD_ACTIVE_IMMEDIATELY, &region, 0x10801080);

    if ((ret = hd_videoout_close(g_stVoChn.pathId[voChn])) != HD_OK) {
        VAPILOG("hd_videoout_close voChn[%d] failed with %d !\n", voChn, ret);
        return ret;
    }

    g_stVoChn.stChn[voChn].enState = STATE_IDLE;

    return ret;
}

HD_RESULT vapi_vo_start_stream(HI_S32 voChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoout_start(g_stVoChn.pathId[voChn])) != HD_OK) {
        VAPILOG("hd_videoout_start vdchn[%d]failed with %d !\n", voChn, ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vo_stop_stream(HI_S32 voChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoout_stop(g_stVoChn.pathId[voChn])) != HD_OK) {
        VAPILOG("hd_videoout_start vdchn[%d]failed with %d !\n", voChn, ret);
        return ret;
    }

    return ret;
}

HI_S32  vapi_vo_chn_clear_buff(DEV_E voDev, HI_S32 voChn)
{
    return HI_SUCCESS;
}

HI_VOID vapi_vo_dsp_quick(HI_BOOL bQuick)
{
    g_stVoChn.isQuick = bQuick;
}

HI_S32 vapi_vo_bind_vdec(HI_S32 voChn, HI_S32 VdecChn)
{
    return HI_SUCCESS;
}

HI_S32 vapi_vo_unbind_vdec(HI_S32 voChn, HI_S32 vdecChn)
{
    return  HI_SUCCESS;
}


HI_VOID vapi_vo_update_state(HI_S32 voChn, HI_S32 state)
{
    g_stVoChn.stChn[voChn].enState = state;
}

HI_BOOL vapi_vo_check_state(HI_S32 voChn, HI_S32 state)
{
    return (g_stVoChn.stChn[voChn].enState == state);
}

HI_S32  vapi_vo_set_ratio(HI_S32 voChn, RATIO_E enRatio)
{

    return HI_SUCCESS;
}


HI_S32 vapi_vo_set_crop(HI_S32 voChn, HD_URECT *pstRect)
{
    return HI_SUCCESS;
}

HI_S32 vapi_vo_snap_jpeg(HI_S32 voChn, VAPI_JPEG_S *pstJpeg)
{
    HI_S32 s32Ret = HI_SUCCESS;

    return s32Ret;
}

HD_RESULT vapi_vo_dev_open(DEV_E voDev, SCREEN_RES_E enRes)
{
    HD_RESULT ret = HD_OK;
    HD_VIDEOOUT_MODE mode = {0};
    HD_FB_DIM fbDim = {0};
    HD_FB_FMT fbFmt = {0};
    HD_FB_ENABLE fbCfg = {0};

    g_stVoChn.stAdapt.enRes[voDev] = SCREEN_UNDEFINE;

    ret = hd_videoout_get(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_MODE, &mode);
    if (ret != HD_OK) {
        VAPILOG("hd_videoout_get HD_VIDEOOUT_PARAM_MODE fail with %d \n", ret);
        return ret;
    }

    vo_get_mode(enRes, &mode);
    if (voDev == VAPI_VO_DEV_GUI) {
        mode.input_dim = HD_VIDEOOUT_IN_1920x1080;
        fbDim.input_dim.w = 1920;
        fbDim.input_dim.h = 1080;
        fbDim.fb_id = HD_FB1;
    } else {
        fbDim.fb_id = HD_FB0;
        vapi_vo_calc_w_h(enRes, (HI_S32 *)&fbDim.input_dim.w, (HI_S32 *)&fbDim.input_dim.h);
    }
    ret = hd_videoout_set(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_MODE, &mode);
    if (ret != HD_OK) {
        VAPILOG("hd_videoout_set HD_VIDEOOUT_PARAM_MODE fail with %d \n", ret);
        return ret;
    }

    fbCfg.fb_id = fbDim.fb_id;
    fbCfg.enable = 0;
    ret = hd_videoout_set(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_FB_ENABLE, &fbCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_videoout_set HD_VIDEOOUT_PARAM_FB_ENABLE fail with %d \n", ret);
        return ret;
    }

    /* update fb */
    fbDim.output_rect.x = 0;
    fbDim.output_rect.y = 0;
    fbDim.output_rect.w = fbDim.input_dim.w;
    fbDim.output_rect.h = fbDim.input_dim.h;
    ret = hd_videoout_set(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_FB_DIM, &fbDim);
    if (ret != HD_OK) {
        VAPILOG("hd_videoout_set HD_VIDEOOUT_PARAM_FB_DIM fail with %d \n", ret);
        return ret;
    }

    fbFmt.fb_id = fbDim.fb_id;
    ret = hd_videoout_get(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_FB_FMT, &fbFmt);
    if (ret != HD_OK) {
        VAPILOG("hd_videoout_get HD_VIDEOOUT_PARAM_FB_FMT failed with %d !\n", ret);
        return ret;
    }

    clear_buffer_black(voDev, fbFmt.fmt, fbDim.output_rect.w, fbDim.output_rect.h);

    fbCfg.enable = 1;
    ret = hd_videoout_set(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_FB_ENABLE, &fbCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_videoout_set HD_VIDEOOUT_PARAM_FB_ENABLE fail with %d \n", ret);
        return ret;
    }

    g_stVoChn.enRes[voDev] = enRes;

    return HI_SUCCESS;
}

HD_RESULT vapi_vo_dev_close(DEV_E voDev)
{
    HD_RESULT ret = HD_OK;
    HD_FB_ENABLE fbCfg;

    g_stVoChn.enRes[voDev] = SCREEN_UNDEFINE;

    fbCfg.fb_id = voDev == VAPI_VO_DEV_GUI ? HD_FB1 : HD_FB0;
    fbCfg.enable = 0;
    ret = hd_videoout_set(g_stVoChn.stAdapt.devPath, HD_VIDEOOUT_PARAM_FB_ENABLE, &fbCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_videoout_set HD_VIDEOOUT_PARAM_FB_ENABLE fail with %d \n", ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, HI_S32 isHotplug, HI_BOOL isQuick, VDEC_CB_S *pstCb)
{
    HD_RESULT ret = HD_OK;

    memset(&g_stVoChn, 0, sizeof(VOCHN_S));

    g_stVoChn.update_screen_res_cb = pstCb->update_screen_res_cb;
    g_stVoChn.isHotplug = isHotplug;
    g_stVoChn.isQuick = isQuick;

    if ((ret = hd_videoout_init()) != HD_OK) {
        VAPILOG("hd_videoout_init fail with %d \n", ret);
        return ret;
    }

    if ((ret = hd_videoout_open(0, HD_VIDEOOUT_0_CTRL, &g_stVoChn.stAdapt.devPath)) != HD_OK) {
        VAPILOG("hd_videoout_open fail with %d \n", ret);
        return ret;
    }

    if ((ret = vapi_vo_dev_open(VAPI_VO_DEV_GUI, enMain)) != HD_OK) {
        VAPILOG("VAPI_VO_DEV_VIDEO open fail with %d \n", ret);
        return ret;
    }

    if ((ret = vapi_vo_dev_open(VAPI_VO_DEV_VIDEO, enMain)) != HD_OK) {
        VAPILOG("VAPI_VO_DEV_GUI open fail with %d \n", ret);
        return ret;
    }

    if (g_stVoChn.isHotplug == HI_TRUE) {
        if ((ret = vapi_vo_adaptive_start()) != HD_OK) {
            return ret;
        }
    }

    return ret;
}

HD_RESULT vapi_vo_uninit()
{
    HD_RESULT ret = HD_OK;

    vapi_vo_dev_close(VAPI_VO_DEV_VIDEO);
    vapi_vo_dev_close(VAPI_VO_DEV_GUI);

    if ((ret = hd_videoout_close(g_stVoChn.stAdapt.devPath)) != HD_OK) {
        VAPILOG("hd_videoout_close fail with %d \n", ret);
        return ret;
    }

    if ((ret = hd_videoout_uninit()) != HD_OK) {
        VAPILOG("hd_videoout_uninit fail with %d \n", ret);
    }

    if (g_stVoChn.isHotplug == HI_TRUE) {
        if ((ret = vapi_vo_adaptive_stop()) != HD_OK) {
            return ret;
        }
    }

    if (g_stVoChn.isHotplug == HI_TRUE) {
        g_stVoChn.isHotplug = HI_FALSE;
    }

    g_stVoChn.update_screen_res_cb = NULL;

    return ret;
}


