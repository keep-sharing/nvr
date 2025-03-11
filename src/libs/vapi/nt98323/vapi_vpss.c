/*
 * ***************************************************************
 * Filename:        vapi_vpss.c
 * Created at:      2015.10.21
 * Description:     video process sub system controller
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

#include "vapi_comm.h"

typedef struct vpss_s {
    STATE_E       enState[MAX_CHN_NUM];
    HD_PATH_ID     pathId[MAX_CHN_NUM];
} VPSS_S;

static VPSS_S g_stVpss;

HD_RESULT vapi_vpss_create(HI_S32 vpssChn, HI_S32 vdChn, HI_S32 voChn)
{
    HD_RESULT ret = HD_OK;
    HD_PATH_ID pathId = 0;
    HD_VIDEOPROC_DEV_CONFIG vpeConfig;
    HD_VIDEOPROC_OUT procOut;
    HD_FB_FMT fbFmt;
    HD_URECT stRect;

    if (g_stVpss.enState[vpssChn] == STATE_BUSY) {
        return ret;
    }

    if ((ret = hd_videoproc_open(HD_VIDEOPROC_IN(vpssChn, 0), HD_VIDEOPROC_OUT(vpssChn, 0), &pathId)) != HD_OK) {
        VAPILOG("hd_videoproc_open  vpssChn[%d]voChn[%d] failed with %d !\n", vpssChn, voChn, ret);

        return ret;
    }

    /* Set videoprocess out pool */
    memset(&vpeConfig, 0x0, sizeof(HD_VIDEOPROC_DEV_CONFIG));
    vpeConfig.data_pool[0].mode = HD_VIDEOPROC_POOL_ENABLE;
    vpeConfig.data_pool[0].ddr_id = 0;
    vpeConfig.data_pool[0].counts = HD_VIDEOPROC_SET_COUNT(3, 0);

    vpeConfig.data_pool[1].mode = HD_VIDEOPROC_POOL_DISABLE;
    vpeConfig.data_pool[2].mode = HD_VIDEOPROC_POOL_DISABLE;
    vpeConfig.data_pool[3].mode = HD_VIDEOPROC_POOL_DISABLE;

    if ((ret = hd_videoproc_set(pathId, HD_VIDEOPROC_PARAM_DEV_CONFIG, &vpeConfig)) != HD_OK) {
        VAPILOG("hd_videoproc_set HD_VIDEOPROC_PARAM_DEV_CONFIG vpssChn[%d] failed with %d !\n", vpssChn, ret);
        return ret;
    }

    g_stVpss.pathId[vpssChn] = pathId;
    stRect.x = 0;
    stRect.y = 0;
    vapi_vdec_get_chn_wh(vdChn, &stRect.w, &stRect.h);
    vapi_vpss_set_crop(vpssChn, vdChn, &stRect, HD_CROP_AUTO);

    vapi_vo_get_win_rect(voChn, &procOut.rect, &procOut.bg);
    fbFmt.fb_id = HD_FB0;
    vapi_vo_get_fb_fmt(&fbFmt);
    procOut.pxlfmt = fbFmt.fmt;
    procOut.dir = HD_VIDEO_DIR_NONE;
    ret = hd_videoproc_set(pathId, HD_VIDEOPROC_PARAM_OUT, &procOut);
    if (ret != HD_OK) {
        VAPILOG("hd_videoproc_set HD_VIDEOPROC_PARAM_OUT vpssChn[%d] failed with %d !\n", vpssChn, ret);
        return ret;
    }

    g_stVpss.enState[vpssChn] = STATE_BUSY;

    return ret;
}

HD_RESULT vapi_vpss_destory(HI_S32 vpssChn)
{
    HD_RESULT ret = HD_OK;

    if (g_stVpss.enState[vpssChn] == STATE_IDLE) {
        return ret;
    }

    if ((ret = hd_videoproc_close(g_stVpss.pathId[vpssChn])) != HD_OK) {
        VAPILOG("hd_videoproc_close  vpssChn[%d] failed with %d !\n", vpssChn, ret);
        return ret;
    }

    g_stVpss.enState[vpssChn] = STATE_IDLE;
    g_stVpss.pathId[vpssChn] = 0;

    return ret;
}

HD_RESULT vapi_vpss_bind_vo(HI_S32 vpssChn, HI_S32 voChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoproc_bind(HD_VIDEOPROC_OUT(vpssChn, 0), HD_VIDEOOUT_IN(0, voChn))) != HD_OK) {
        VAPILOG("hd_videodec_bind vpssChn[%d]->voChn[%d] failed with %d !\n", vpssChn, voChn, ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vpss_unbind_vo(HI_S32 vpssChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoproc_unbind(HD_VIDEOPROC_OUT(vpssChn, 0))) != HD_OK) {
        VAPILOG("hd_videodec_unbind vpssChn[%d] failed with %d !\n", vpssChn, ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vpss_start_stream(HI_S32 vpssChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoproc_start(g_stVpss.pathId[vpssChn])) != HD_OK) {
        VAPILOG("hd_videoproc_start vpssChn[%d]failed with %d !\n", vpssChn, ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vpss_stop_stream(HI_S32 vpssChn)
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoproc_stop(g_stVpss.pathId[vpssChn])) != HD_OK) {
        VAPILOG("hd_videoproc_stop vdchn[%d]failed with %d !\n", vpssChn, ret);
        return ret;
    }

    return ret;
}

HI_S32 vapi_vpss_snap_jpeg(int vpssChn, VAPI_JPEG_S *pstJpeg)
{
    return HD_OK;
}

HD_RESULT vapi_vpss_set_crop(int vpssChn, int vdChn, HD_URECT *pstRect, HD_CROP_MODE enMode)
{
    HD_VIDEOPROC_CROP stCrop;
    HD_RESULT ret = HD_OK;

    stCrop.mode = enMode;
    vapi_vdec_get_chn_wh(vdChn, &stCrop.win.coord.w, &stCrop.win.coord.h);
    stCrop.win.rect.x = ALIGN_CEIL(pstRect->x, 2);
    stCrop.win.rect.y = ALIGN_CEIL(pstRect->y, 2);
    stCrop.win.rect.w = ALIGN_CEIL(pstRect->w, 8);
    stCrop.win.rect.h = ALIGN_CEIL(pstRect->h, 2);
    ret = hd_videoproc_set(g_stVpss.pathId[vpssChn], HD_VIDEOPROC_PARAM_IN_CROP, (void *)&stCrop);
    if (ret != HD_OK) {
        VAPILOG("vapi_vpss_set_crop vdchn[%d]failed with %d !\n", vpssChn, ret);
        return ret;
    }

    return ret;
}

HD_RESULT vapi_vpss_init()
{
    HD_RESULT ret = HD_OK;

    memset(&g_stVpss, 0, sizeof(VPSS_S));
    if ((ret = hd_videoproc_init()) != HD_OK) {
        VAPILOG("hd_videoproc_init fail with %d \n", ret);
    }

    return ret;
}

HD_RESULT vapi_vpss_uninit()
{
    HD_RESULT ret = HD_OK;

    if ((ret = hd_videoproc_uninit()) != HD_OK) {
        VAPILOG("hd_videoproc_uninit fail with %d \n", ret);
    }

    return ret;
}


