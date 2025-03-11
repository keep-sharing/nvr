/*
 * ***************************************************************
 * Filename:      	vapi.c
 * Created at:    	2015.10.21
 * Description:   	public API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vapi.h"
#include "vapi_comm.h"

#define MAX_IVE_DMA_WIDTH  (1920)
#define MAX_IVE_DMA_HEIGHT (1080)
#define MAX_IVE_DMA_SIZE   (MAX_IVE_DMA_WIDTH * MAX_IVE_DMA_HEIGHT)

typedef struct vpss_bind_s
{
    STATE_E     enState;
    ot_vdec_chn vdecChn;
    ot_vpss_chn vpssGrp;
    ot_vpss_chn vpssChn;
    ot_vo_chn   voutChn;
    ot_vo_layer voutLayer;
    FISH_E      enFish;
} VPSS_BIND_S;

typedef struct vpssGrp_s
{
    VPSS_BIND_S     stBind[VAPI_VPSS_MAX_NUM];
    td_s32          aGroup[HISI_VPSS_MAX_NUM];
    pthread_mutex_t mutex;
} VPSSGRP_S;

static VPSSGRP_S g_stVpssGrp;

static inline td_s32 ive_copy(int srcPhy, void *srcVir, int dstPhy, void *dstVir, td_s32 offset, td_u16 u16Width, td_u16 u16Height)
{
    ot_ive_dma_ctrl stDmaCtrl = {OT_IVE_DMA_MODE_DIRECT_COPY, 0};
    ot_ive_handle   iveHandle;
    ot_svp_data     iveSrc;
    ot_svp_dst_data iveDst;
    td_s32          s32Ret;

    iveSrc.width     = u16Width;
    iveSrc.height    = u16Height;
    iveSrc.stride    = iveSrc.width;
    iveSrc.virt_addr = srcVir + offset;
    iveSrc.phys_addr = srcPhy + offset;

    iveDst.virt_addr = dstVir + offset;
    iveDst.phys_addr = dstPhy + offset;
    iveDst.width     = iveSrc.width;
    iveDst.height    = iveSrc.height;
    iveDst.stride    = iveSrc.stride;
    s32Ret           = ss_mpi_ive_dma(&iveHandle, &iveSrc, &iveDst, &stDmaCtrl, TD_TRUE);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ive dma cpy fail %#x !", s32Ret);
        return TD_FAILURE;
    }
    return TD_SUCCESS;
}

static inline td_s32 vapi_ive_copy(int srcPhy, void *srcVir, int dstPhy, void *dstVir, int width, int height)
{
    td_s32 i, n, m;
    td_s32 s32Ret;
    td_s32 offset = 0;

    n = (width * height * 3 / 2) / MAX_IVE_DMA_SIZE;
    m = (width * height * 3 / 2) % MAX_IVE_DMA_SIZE;
    for (i = 0; i < n; i++) {
        s32Ret = ive_copy(srcPhy, srcVir, dstPhy, dstVir, offset, MAX_IVE_DMA_WIDTH, MAX_IVE_DMA_HEIGHT);
        if (s32Ret != TD_SUCCESS)
            return TD_FAILURE;
        offset += MAX_IVE_DMA_SIZE;
    }

    n = m / MAX_IVE_DMA_WIDTH;
    m = m % MAX_IVE_DMA_WIDTH;

    if (n) {
        s32Ret = ive_copy(srcPhy, srcVir, dstPhy, dstVir, offset, MAX_IVE_DMA_WIDTH, n);
        if (s32Ret != TD_SUCCESS)
            return TD_FAILURE;

        offset += MAX_IVE_DMA_WIDTH * n;
    }

    if (m) {
        s32Ret = ive_copy(srcPhy, srcVir, dstPhy, dstVir, offset, m, 1);
        if (s32Ret != TD_SUCCESS)
            return TD_FAILURE;
    }

    return TD_SUCCESS;
}

static inline td_s32 vapi_vgs_scale(ot_video_frame_info *pstVoutFrame, td_u32 VinPhyAddr, td_u32 width, td_u32 height)
{
    ot_vgs_handle    hHandle;
    ot_vgs_task_attr stTask;
    td_s32           s32Ret;

    s32Ret = ss_mpi_vgs_begin_job(&hHandle);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vgs_begin_job fail for %#x!\n", s32Ret);
        return s32Ret;
    }

    memcpy(&stTask.img_in, pstVoutFrame, sizeof(ot_video_frame_info));
    stTask.img_in.video_frame.width        = width;
    stTask.img_in.video_frame.height       = height;
    stTask.img_in.video_frame.phys_addr[0] = VinPhyAddr;
    stTask.img_in.video_frame.phys_addr[1] = VinPhyAddr + (width * height);
    stTask.img_in.video_frame.stride[0]    = width;
    stTask.img_in.video_frame.stride[1]    = width;
    memcpy(&stTask.img_out, pstVoutFrame, sizeof(ot_video_frame_info));

    s32Ret = ss_mpi_vgs_add_scale_task(hHandle, &stTask, OT_VGS_SCALE_COEF_NORM);
    if (s32Ret != TD_SUCCESS) {
        ss_mpi_vgs_cancel_job(hHandle);
        VAPILOG("ss_mpi_vgs_add_scale_task fail for %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = ss_mpi_vgs_end_job(hHandle);
    if (s32Ret != TD_SUCCESS) {
        ss_mpi_vgs_cancel_job(hHandle);
        VAPILOG("ss_mpi_vgs_end_job fail for %#x!\n", s32Ret);
        return s32Ret;
    }

    ss_mpi_vgs_cancel_job(hHandle);
    return s32Ret;
}

static ot_vpss_grp vapi_add_vpss_group(void)
{
    td_s32 i;

    for (i = 0; i < HISI_VPSS_MAX_NUM; i++) {
        if (g_stVpssGrp.aGroup[i] == STATE_IDLE) {
            return i;
        }
    }

    return TD_FAILURE;
}

static void vapi_del_vpss_group(ot_vpss_grp VpssGrp)
{
    g_stVpssGrp.aGroup[VpssGrp] = STATE_IDLE;
}

static td_s32 vapi_bind_vpss(ot_vdec_chn VdChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn, td_u32 MaxW, td_u32 MaxH, ot_vo_layer VoLayer, ot_vo_chn VoChn, FISH_E enFish)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_vpss_grp GrpId  = 0;

    if (g_stVpssGrp.stBind[VpssGrp].enState == STATE_BUSY) {
        return TD_FAILURE;
    }

    GrpId = vapi_add_vpss_group();
    if (GrpId == TD_FAILURE) {
        VAPILOG("no find a new vpss group !\n");
        return TD_FAILURE;
    }

    s32Ret = vapi_vpss_create(GrpId, VpssChn, enFish, MaxW, MaxH);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_vpss_create [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return TD_FAILURE;
    }

    vapi_vdec_bind_vpss(VdChn, GrpId, VpssChn);

    if (enFish == FISH_MODE_NONE) {
        vapi_vo_bind_vpss(VoLayer, VoChn, GrpId, VpssChn);
    }
    vapi_vdec_start_stream(VdChn);

    g_stVpssGrp.stBind[VpssGrp].enState   = STATE_BUSY;
    g_stVpssGrp.stBind[VpssGrp].vdecChn   = VdChn;
    g_stVpssGrp.stBind[VpssGrp].vpssGrp   = GrpId;
    g_stVpssGrp.stBind[VpssGrp].vpssChn   = VpssChn;
    g_stVpssGrp.stBind[VpssGrp].voutChn   = VoChn;
    g_stVpssGrp.stBind[VpssGrp].voutLayer = VoLayer;
    g_stVpssGrp.stBind[VpssGrp].enFish    = enFish;
    g_stVpssGrp.aGroup[GrpId]             = STATE_BUSY;

    return TD_SUCCESS;
}

static td_s32 vapi_unbind_vpss(ot_vpss_grp VpssGrp)
{
    ot_vdec_chn VdChn   = g_stVpssGrp.stBind[VpssGrp].vdecChn;
    ot_vpss_chn VpssChn = g_stVpssGrp.stBind[VpssGrp].vpssChn;
    ot_vo_layer VoLayer = g_stVpssGrp.stBind[VpssGrp].voutLayer;
    ot_vo_chn   VoChn   = g_stVpssGrp.stBind[VpssGrp].voutChn;
    FISH_E      enFish  = g_stVpssGrp.stBind[VpssGrp].enFish;
    ot_vpss_grp GrpId   = 0;

    if (g_stVpssGrp.stBind[VpssGrp].enState != STATE_BUSY) {
        return TD_FAILURE;
    }
    GrpId = g_stVpssGrp.stBind[VpssGrp].vpssGrp;
    //    vapi_vdec_stop_stream(VdChn);
    vapi_vdec_unbind_vpss(VdChn, GrpId, VpssChn);
    if (enFish == FISH_MODE_NONE)
        vapi_vo_unbind_vpss(VoLayer, VoChn, GrpId, VpssChn);
    vapi_vpss_destory(GrpId, VpssChn);
    vapi_del_vpss_group(GrpId);

    g_stVpssGrp.stBind[VpssGrp].enState = STATE_IDLE;

    return TD_SUCCESS;
}

static td_bool vapi_vdec_is_unbind(ot_vdec_chn VdChn)
{
    td_s32 i;
    for (i = 0; i < VAPI_VPSS_MAX_NUM; i++) {
        if (g_stVpssGrp.stBind[i].enState == STATE_BUSY && g_stVpssGrp.stBind[i].vdecChn == VdChn) {
            return TD_FALSE;
        }
    }
    return TD_TRUE;
}

static void vapi_bind_init()
{
    td_s32 i;

    for (i = 0; i < VAPI_VPSS_MAX_NUM; i++) {
        g_stVpssGrp.stBind[i].enState   = STATE_IDLE;
        g_stVpssGrp.stBind[i].voutLayer = -1;
        g_stVpssGrp.stBind[i].vpssGrp   = -1;
        g_stVpssGrp.stBind[i].vpssChn   = -1;
        g_stVpssGrp.stBind[i].vdecChn   = -1;
        g_stVpssGrp.stBind[i].voutChn   = -1;
    }

    for (i = 0; i < HISI_VPSS_MAX_NUM; i++) {
        g_stVpssGrp.aGroup[i] = STATE_IDLE;
    }

    comm_mutex_init(&g_stVpssGrp.mutex);
}

static void vapi_bind_uninit()
{
    td_s32 i;

    for (i = 0; i < VAPI_VPSS_MAX_NUM; i++) {
        vapi_unbind_vpss(i);
    }
    comm_mutex_uninit(&g_stVpssGrp.mutex);
}

static inline void vapi_get_vpss(SCREEN_E enScreen, MODE_E enMode, int devID, ot_vpss_grp *pGrp, ot_vpss_chn *pChn)
{
    if (enMode == DSP_MODE_ZOOMIN) {
        *pGrp = enScreen * MAX_CHN_NUM + MAX_CHN_NUM - 1;
    } else {
        //*pGrp = enScreen * MAX_CHN_NUM + enMode * MAX_LIVE_NUM + devID;
        *pGrp = enScreen * MAX_CHN_NUM + devID;
    }
    *pChn = VAPI_VPSS_CHN_ID;
}

static inline void vapi_get_vpss2(SCREEN_E enScreen, int VoChn, ot_vpss_grp *pGrp, ot_vpss_chn *pChn)
{
    *pGrp = enScreen * MAX_CHN_NUM + VoChn;
    *pChn = VAPI_VPSS_CHN_ID;
}

static td_s32 vapi_del_one_win(SCREEN_E enScreen, MODE_E enMode, int devID, REFRESH_E enRefresh)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    ot_vo_layer VoLayer;
    ot_vo_chn   VoChn;
    ot_vdec_chn VdChn;

    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    s32Ret  = vapi_unbind_vpss(Grp);
    VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;
    VoChn   = g_stVpssGrp.stBind[Grp].voutChn;
    VdChn   = g_stVpssGrp.stBind[Grp].vdecChn;
    if (s32Ret == TD_SUCCESS)
        s32Ret |= vapi_vo_chn_destory(VoLayer, VoChn);
    if (vapi_vdec_is_unbind(VdChn)) {
        vapi_vdec_stop_stream(VdChn);
        if (enRefresh == REFRESH_CLEAR_VDEC) {
            s32Ret |= vapi_vdec_chn_destory(VdChn);
            g_stVpssGrp.stBind[Grp].vdecChn = -1;
        }
    } else {
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
    }

    return s32Ret;
}

static td_s32 vapi_del_one_win2(SCREEN_E enScreen, int VoChn)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    ot_vo_layer VoLayer;
    ot_vdec_chn VdChn;

    comm_mutex_lock(&g_stVpssGrp.mutex);
    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    s32Ret  = vapi_unbind_vpss(Grp);
    VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;
    VoChn   = g_stVpssGrp.stBind[Grp].voutChn;
    VdChn   = g_stVpssGrp.stBind[Grp].vdecChn;
    if (s32Ret == TD_SUCCESS)
        vapi_vo_chn_destory(VoLayer, VoChn);
    if (vapi_vdec_is_unbind(VdChn)) {
        vapi_vdec_stop_stream(VdChn);
        vapi_vdec_chn_destory(VdChn);
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
        s32Ret                          = -1;
    } else {
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
        s32Ret                          = VdChn;
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

static td_s32 vapi_add_one_win(SCREEN_E enScreen, MODE_E enMode, WIND_S *pstWin)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    int         devID     = pstWin->stDevinfo.id;
    TYPE_E      enType    = pstWin->stDevinfo.stFormat.enType;
    td_u32      u32Width  = pstWin->stDevinfo.stFormat.width;
    td_u32      u32Height = pstWin->stDevinfo.stFormat.height;
    ot_vo_layer VoLayer;
    ot_vo_chn   VoChn = pstWin->stDevinfo.stBind.voutChn;
    ot_vdec_chn VdChn = pstWin->stDevinfo.stBind.vdecChn;
    ot_rect     stRect;
    RATIO_E     enRatio = pstWin->enRatio;
    FISH_E      enFish  = pstWin->enFish;

    devID = pstWin->stDevinfo.id;
    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY) {
        // Grp has existed ;
        return TD_SUCCESS;
    }
    // create a display channel
    stRect.x      = pstWin->stZone.x;
    stRect.y      = pstWin->stZone.y;
    stRect.width  = pstWin->stZone.w;
    stRect.height = pstWin->stZone.h;
    if (enMode == DSP_MODE_ZOOMIN) {
        VoLayer = VAPI_VO_LAYER_VPIP;
    } else if (enScreen == SCREEN_SUB) {
        VoLayer = VAPI_VO_LAYER_VHD1;
    } else {
        VoLayer = VAPI_VO_LAYER_VHD0;
    }

    s32Ret = vapi_vo_chn_create(VoLayer, VoChn, &stRect, enRatio);

    // bind : vdec->vpss->vout
    if (s32Ret == TD_SUCCESS) {
        // create a vdec channel
        s32Ret = vapi_vdec_chn_create(devID, VdChn, enType, u32Width, u32Height);
        if (s32Ret == TD_SUCCESS) {
            // bind vdec->vpss->vo
            s32Ret = vapi_bind_vpss(VdChn, Grp, Chn, u32Width, u32Height, VoLayer, VoChn, enFish);
            if (s32Ret == TD_FAILURE) {
                vapi_vo_chn_destory(VoLayer, VoChn);
                vapi_vdec_chn_destory(VdChn);
            }
        } else {
            vapi_vo_chn_destory(VoLayer, VoChn);
        }
    }

    return s32Ret;
}

static td_s32 vapi_add_one_win2(SCREEN_E enScreen, WIND_S *pstWin)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    int         devID     = pstWin->stDevinfo.id;
    TYPE_E      enType    = pstWin->stDevinfo.stFormat.enType;
    td_u32      u32Width  = pstWin->stDevinfo.stFormat.width;
    td_u32      u32Height = pstWin->stDevinfo.stFormat.height;
    ot_vo_layer VoLayer;
    ot_vo_chn   VoChn = pstWin->stDevinfo.stBind.voutChn;
    ot_vdec_chn VdChn = pstWin->stDevinfo.stBind.vdecChn;
    ot_rect     stRect;
    RATIO_E     enRatio = pstWin->enRatio;
    FISH_E      enFish  = pstWin->enFish;

    devID = pstWin->stDevinfo.id;
    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY) {
        // Grp has existed ;
        return TD_SUCCESS;
    }
    comm_mutex_lock(&g_stVpssGrp.mutex);
    // create a display channel
    stRect.x      = pstWin->stZone.x;
    stRect.y      = pstWin->stZone.y;
    stRect.width  = pstWin->stZone.w;
    stRect.height = pstWin->stZone.h;
    if (enScreen == SCREEN_SUB) {
        VoLayer = VAPI_VO_LAYER_VHD1;
    } else {
        VoLayer = VAPI_VO_LAYER_VHD0;
    }
    s32Ret = vapi_vo_chn_create(VoLayer, VoChn, &stRect, enRatio);

    // bind : vdec-> vpss->vout
    if (s32Ret == TD_SUCCESS) {
        // create a vdec channel
        s32Ret = vapi_vdec_chn_create(devID, VdChn, enType, u32Width, u32Height);
        if (s32Ret == TD_SUCCESS) {
            // bind vdec->vpss->vo
            s32Ret = vapi_bind_vpss(VdChn, Grp, Chn, u32Width, u32Height, VoLayer, VoChn, enFish);
            if (s32Ret == TD_FAILURE) {
                vapi_vo_chn_destory(VoLayer, VoChn);
                vapi_vdec_chn_destory(VdChn);
            }
        } else {
            vapi_vo_chn_destory(VoLayer, VoChn);
        }
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

int yu_vapi_add_one_win(SCREEN_E enScreen, MODE_E enMode, WIND_S *pstWin)
{
    return vapi_add_one_win(enScreen, enMode, pstWin);
}
int yu_vapi_del_one_win(SCREEN_E enScreen, MODE_E enMode, int devID, REFRESH_E enRefresh)
{
    return vapi_del_one_win(enScreen, enMode, devID, enRefresh);
}

/*
 * *********************************public************************************
 */
int vapi_get_vdec_chn(SCREEN_E enScreen, MODE_E enMode, int devID)
{
    td_bool     s32Ret;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    ot_vdec_chn vdecChn;

    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    vdecChn = g_stVpssGrp.stBind[Grp].vdecChn;

    s32Ret = vapi_vdec_check_state(vdecChn, STATE_BUSY);

    if (s32Ret == TD_TRUE) {
        return vdecChn;
    } else {
        VAPILOG("dev[%d] not use any vdec chn \n", devID);
        return -1;
    }
}

int vapi_get_vdec_chn2(SCREEN_E enScreen, int VoChn)
{
    td_bool     s32Ret;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    ot_vdec_chn vdecChn;

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    vdecChn = g_stVpssGrp.stBind[Grp].vdecChn;

    s32Ret = vapi_vdec_check_state(vdecChn, STATE_BUSY);

    if (s32Ret == TD_TRUE) {
        return vdecChn;
    } else {
        VAPILOG("dev[%d] not use any vdec chn \n", VoChn);
        return -1;
    }
}

void vapi_get_screen_res(SCREEN_E enScreen, int *W, int *H)
{
    vapi_vo_get_srceen_res(enScreen, W, H);
}

void vapi_clear_vout_chn(SCREEN_E enScreen, int VoChn)
{
    ot_vo_layer VoLayer;

    if (enScreen == SCREEN_MAIN) {
        VoLayer = VAPI_VO_LAYER_VHD0;
    } else if (enScreen == SCREEN_SUB) {
        VoLayer = VAPI_VO_LAYER_VHD1;
    } else {
        VAPILOG("Screen type [%d] invalid \n", enScreen);
        return;
    }

    vapi_vo_chn_clear_buff(VoLayer, VoChn);
}
void vapi_clear_screen(SCREEN_E enScreen, REFRESH_E enRefresh)
{
    td_s32 i;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        vapi_del_one_win(enScreen, 0, i, enRefresh);
    }
}

void vapi_clear_screen2(SCREEN_E enScreen)
{
    td_s32 i;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        vapi_del_one_win2(enScreen, i);
    }
}

static void vapi_enter_zoomin(ZOOMIN_S *pstZoomin)
{
    SCREEN_E    enScreen = pstZoomin->enScreen;
    WIND_S      stWind;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;

    vapi_switch_screen(pstZoomin->enScreen);
    memset(&stWind, 0, sizeof(WIND_S));
    vapi_get_vpss(enScreen, pstZoomin->enMode, pstZoomin->devID, &Grp, &Chn);

    stWind.stDevinfo.id             = pstZoomin->devID;
    stWind.stDevinfo.stFormat       = pstZoomin->stFormat;
    stWind.stDevinfo.stBind.vdecChn = pstZoomin->vdecID;
    stWind.stDevinfo.stBind.voutChn = 0;
    stWind.enRatio                  = pstZoomin->enRatio;
    //
    vapi_clear_screen(enScreen, pstZoomin->enRefresh);

    //
    stWind.stZone = pstZoomin->stZone[WIN_TYPE_NORMAL];
    vapi_add_one_win(enScreen, pstZoomin->enMode, &stWind);

    //
    stWind.stZone = pstZoomin->stZone[WIN_TYPE_ZOOMIN];
    vapi_add_one_win(enScreen, DSP_MODE_ZOOMIN, &stWind);
}

static int vapi_start_zoomin(ZOOMIN_S *pstZoomin)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_rect     stRect;
    SCREEN_E    enScreen = pstZoomin->enScreen;
    MODE_E      enMode   = pstZoomin->enMode;
    int         devID    = pstZoomin->devID;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    td_s32      W, H;

    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    vapi_get_screen_res(enScreen, &W, &H);
    //    printf("=====ratic_w %d, ratio_h %d min_w %d, min h %d\n",W/pstZoomin->stRect.w, H/pstZoomin->stRect.h, W/16, H/16);
    //    printf("===o_x %d, o_y %d, o_w %d, o_h %d\n", pstZoomin->stRect.x,pstZoomin->stRect.y,pstZoomin->stRect.w, pstZoomin->stRect.h);
    stRect.width  = VAPI_MAX(pstZoomin->stRect.w, W / 15);                                     // pstZoomin->stRect.w;
    stRect.height = VAPI_MAX(pstZoomin->stRect.h, H / 15);                                     // pstZoomin->stRect.h;
    stRect.x      = VAPI_MIN(pstZoomin->stFormat.width - stRect.width, pstZoomin->stRect.x);   // pstZoomin->stRect.x;
    stRect.y      = VAPI_MIN(pstZoomin->stFormat.height - stRect.height, pstZoomin->stRect.y); // pstZoomin->stRect.y;
    //    printf("===m_x %d, m_y %d, m_w %d, m_h %d\n", stRect.s32X,stRect.s32Y,stRect.u32Width, stRect.u32Height);
    s32Ret        = vapi_vpss_set_crop(g_stVpssGrp.stBind[Grp].vpssGrp, &stRect);

    return s32Ret;
}

int vapi_update_layout(LAYOUT_S *pstLayout)
{
    td_s32 s32Ret = TD_SUCCESS;
    td_s32 i;

    // add new window
    for (i = 0; i < pstLayout->winds_num; i++) {
        s32Ret |= vapi_add_one_win(pstLayout->enScreen, pstLayout->enMode, &pstLayout->stWinds[i]);
    }

    return s32Ret;
}

int vapi_set_screen_resolution(SCREEN_RES_S *pstScreenRes)
{
    td_s32 s32Ret = TD_SUCCESS;

    DEV_E dev_id = -1;
    switch (pstScreenRes->enScreen) {
    case SCREEN_MAIN:
        dev_id = VAPI_VO_DEV_DHD0;
        break;
    case SCREEN_SUB:
        dev_id = VAPI_VO_DEV_DHD1;
        break;
    default:
        break;
    }
    if (dev_id < 0) {
        VAPILOG("invalid dev_id[%d]", dev_id);
        return TD_FAILURE;
    }

    // clear current screen
    vapi_clear_screen2(pstScreenRes->enScreen);

    comm_mutex_lock(&g_stVpssGrp.mutex);
    //
    if (vapi_vo_get_pipbind() == pstScreenRes->enScreen) {
        vapi_vo_stop_pip_layer();
    }

    s32Ret = vapi_vo_dev_close(dev_id);
    if (s32Ret != TD_SUCCESS) {
        comm_mutex_unlock(&g_stVpssGrp.mutex);
        return TD_FAILURE;
    }

    sleep(1);

    s32Ret = vapi_vo_dev_open(dev_id, pstScreenRes->enRes);
    if (s32Ret != TD_SUCCESS) {
        comm_mutex_unlock(&g_stVpssGrp.mutex);
        return TD_FAILURE;
    }

    //
    if (vapi_vo_get_pipbind() == pstScreenRes->enScreen) {
        vapi_vo_start_pip_layer(pstScreenRes->enRes);
    }

    //    vapi_fb_init(pstScreenRes->enScreen);

    // hjh 20160212 add, for QT switch output res.
    s32Ret = vapi_switch_display(pstScreenRes->enScreen);
    if (s32Ret != TD_SUCCESS) {
        comm_mutex_unlock(&g_stVpssGrp.mutex);
        return TD_FAILURE;
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return TD_SUCCESS;
}

int vapi_switch_screen(SCREEN_E enScreen)
{
    td_s32 s32Ret = TD_SUCCESS;

    s32Ret = vapi_vo_pip_rebind(enScreen);

    return s32Ret;
}

int vapi_set_zoomin(ZOOMIN_S *pstZoomin)
{
    td_s32 s32Ret = TD_SUCCESS;

    if (pstZoomin->enState == STATE_ENTER) {
        vapi_enter_zoomin(pstZoomin);
        //        s32Ret = vapi_start_zoomin(pstZoomin);
    } else if (pstZoomin->enState == STATE_ZOOMIN) {
        s32Ret = vapi_start_zoomin(pstZoomin);
    }

    return s32Ret;
}

int vapi_set_zoomin2(ZOOMIN2_S *pstZoomin)
{
    td_s32      s32Ret = TD_SUCCESS;
    ot_rect     stRect;
    SCREEN_E    enScreen = pstZoomin->enScreen;
    int         VoChn    = pstZoomin->VoChn;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    td_s32      W, H;

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    vapi_get_screen_res(enScreen, &W, &H);
    stRect.width  = VAPI_MAX(pstZoomin->stRect.w, W / 15); // pstZoomin->stRect.w;
    stRect.height = VAPI_MAX(pstZoomin->stRect.h, H / 15); // pstZoomin->stRect.h;
    stRect.x      = pstZoomin->stRect.x;
    stRect.y      = pstZoomin->stRect.y;
    s32Ret        = vapi_vpss_set_crop(g_stVpssGrp.stBind[Grp].vpssGrp, &stRect);

    return s32Ret;
}

int vapi_set_full_screen(FULL_S *pstFull)
{
    td_s32      s32Ret = TD_SUCCESS;
    WIND_S      stWind;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;

    memset(&stWind, 0, sizeof(WIND_S));
    // 1.clear screen
    vapi_clear_screen(pstFull->enScreen, pstFull->enRefresh);

    // 2.add a full view
    vapi_get_vpss(pstFull->enScreen, pstFull->enMode, pstFull->devID, &Grp, &Chn);
    stWind.stDevinfo.id             = pstFull->devID;
    stWind.stDevinfo.stFormat       = pstFull->stFormat;
    stWind.stDevinfo.stBind.vdecChn = pstFull->vdecID;
    stWind.stDevinfo.stBind.voutChn = 0;
    stWind.stZone                   = pstFull->stZone;

    s32Ret = vapi_add_one_win(pstFull->enScreen, pstFull->enMode, &stWind);

    return s32Ret;
}

int vapi_set_action(ACTION_S *pstAction)
{
    td_s32 s32Ret = TD_SUCCESS;

    switch (pstAction->enAction) {
    case ACTION_DEL_ONE_WIN:
        s32Ret = vapi_del_one_win(pstAction->enScreen, pstAction->enMode, pstAction->stWind.stDevinfo.id, REFRESH_CLEAR_VDEC);
        break;
    case ACTION_ADD_ONE_WIN:
        s32Ret = vapi_add_one_win(pstAction->enScreen, pstAction->enMode, &pstAction->stWind);
        break;
    default:
        VAPILOG("Unknown action [%d] !\n", pstAction->enAction);
        s32Ret = TD_FAILURE;
        break;
    }

    return s32Ret;
}

int vapi_set_action2(ACTION2_S *pstAction)
{
    td_s32 s32Ret = TD_SUCCESS;

    switch (pstAction->enAction) {
    case ACTION_DEL_ONE_WIN:
        s32Ret = vapi_del_one_win2(pstAction->enScreen, pstAction->stWind.stDevinfo.stBind.voutChn);
        break;
    case ACTION_ADD_ONE_WIN:
        s32Ret = vapi_add_one_win2(pstAction->enScreen, &pstAction->stWind);
        break;
    default:
        VAPILOG("Unknown action [%d] !\n", pstAction->enAction);
        s32Ret = TD_FAILURE;
        break;
    }

    return s32Ret;
}

int vapi_set_vout_img(IMG_S *pstImg)
{
    td_s32 s32Ret = TD_SUCCESS;

    s32Ret = vapi_vo_set_csc(pstImg->enScreen, pstImg->enCSC);

    return s32Ret;
}

int vapi_set_vout_ratio(RATIO_S *pstRatio)
{
    td_bool     s32Ret;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    ot_vdec_chn voutChn;
    ot_vo_layer VoLayer;

    vapi_get_vpss(pstRatio->enScreen, pstRatio->enMode, pstRatio->devId, &Grp, &Chn);
    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
    VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;

    if (VoLayer < 0 || voutChn < 0) {
        VAPILOG("VoLayer[%d] voutChn[%d] invalid paramter\n", VoLayer, voutChn);
        return -1;
    }

    if (!vapi_vo_check_state(VoLayer, voutChn, STATE_BUSY)) {
        VAPILOG("VoLayer[%d] voutChn[%d] no existed\n", VoLayer, voutChn);
        return -1;
    }

    s32Ret = vapi_vo_set_ratio(VoLayer, voutChn, pstRatio->enRatio);

    return s32Ret;
}

int vapi_set_vout_ratio2(RATIO2_S *pstRatio)
{
    td_bool     s32Ret;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    ot_vdec_chn voutChn;
    ot_vo_layer VoLayer;

    vapi_get_vpss2(pstRatio->enScreen, pstRatio->vouid, &Grp, &Chn);
    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
    VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;

    if (VoLayer < 0 || voutChn < 0) {
        VAPILOG("VoLayer[%d] voutChn[%d] invalid paramter\n", VoLayer, voutChn);
        return -1;
    }

    if (!vapi_vo_check_state(VoLayer, voutChn, STATE_BUSY))
        return -1;

    s32Ret = vapi_vo_set_ratio(VoLayer, voutChn, pstRatio->enRatio);

    return s32Ret;
}

int vapi_set_vout_fps(int devID, int fps)
{
    td_s32      s32Ret = TD_SUCCESS;
    td_s32      i;
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;
    ot_vo_layer VoLayer;
    ot_vo_chn   VoChn;

    for (i = 0; i < SCREEN_NUM; i++) {
        vapi_get_vpss(i, DSP_MODE_LIVE, devID, &Grp, &Chn);

        if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY) {
            VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;
            VoChn   = g_stVpssGrp.stBind[Grp].voutChn;
            s32Ret |= vapi_vo_chn_set_fps(VoLayer, VoChn, fps);
        }
    }

    return s32Ret;
}

int vapi_send_frame(int VdChn, unsigned char *pAddr, int Len, unsigned long long pts, int flag)
{
    return vapi_vdec_send_frame(VdChn, pAddr, Len, pts);
}

int vapi_audio_read(int AiDevId, ARATE_E enArate, AENC_E enAenc, AIO_FRAME_S *pstFrame)
{
    return vapi_aio_ai_read_frame(AiDevId, enArate, enAenc, pstFrame);
}

int vapi_audio_write(AO_E AoDevId, AIO_FRAME_S *pstFrame)
{
    return vapi_aio_ao_write_frame(AoDevId, pstFrame);
}

int vapi_audio_ctrl(AIO_CTRL_S *pstCtrl)
{
    td_s32 s32Ret = TD_FAILURE;

    switch (pstCtrl->enCmd) {
    case ACMD_VI_SET_VOLUME:
        s32Ret = vapi_aio_ai_set_volume(pstCtrl->devID, pstCtrl->value);
        break;
    case ACMD_VO_SET_VOLUME:
        s32Ret = vapi_aio_ao_set_volume(pstCtrl->devID, pstCtrl->value);
        break;
    default:
        break;
    }

    return s32Ret;
}

int vapi_audio_is_support_talk(void)
{
    return vapi_aio_is_support_talk();
}

int vapi_transfer_create(int VdChn, FORMAT_S *pIf, FORMAT_S *pOf, void *pstPrivate)
{
    td_s32      s32Ret;
    ot_venc_chn VeChn;

    s32Ret = vapi_vdec_chn_create(VENC_IDX_BASE, VdChn, pIf->enType, pIf->width, pIf->height);
    if (s32Ret == TD_FAILURE) {
        VAPILOG("vdec chn[%d] format[%d][%dx%d]create faild with %#x!", VdChn, pIf->enType, pIf->width, pIf->height, s32Ret);
        return -1;
    }

    VeChn = vapi_venc_chn_create(pOf->enType, pOf->width, pOf->height, pOf->enRc, pOf->bitRate, pIf->fps, pOf->fps, pstPrivate);
    if (VeChn < 0) {
        VAPILOG("vdec chn[%d] format[%d][%dx%d][%d->%d]create faild! with %#x", VdChn, pOf->enType, pOf->width, pOf->height, pIf->fps, pOf->fps, VeChn);
        vapi_vdec_chn_destory(VdChn);
        return -1;
    }

    s32Ret = vapi_venc_bind_vdec(VdChn, VeChn);
    if (s32Ret == TD_FAILURE) {
        VAPILOG("vdec chn[%d] format[%d][%dx%d]bind faild with %#x!", VdChn, pIf->enType, pIf->width, pIf->height, s32Ret);
        vapi_vdec_chn_destory(VdChn);
        vapi_venc_chn_destory(VeChn);
        return -1;
    }

    return VeChn;
}

void vapi_transfer_destory(int VeChn)
{
    ot_vdec_chn VdChn;
    VdChn = vapi_venc_unbind_vdec(VeChn);
    vapi_vdec_chn_destory(VdChn);
    vapi_venc_chn_destory(VeChn);
}

int vapi_transfer_config(int VeChn, int FPS, int bitRate)
{
    return vapi_venc_chn_config(VeChn, FPS, bitRate);
}

int vapi_transfer_check()
{
    if (!vapi_venc_chn_is_remain())
        return -1;
    return 0;
}

int vapi_vpool_new(int blkSize, int blkCnt, char *poolName)
{
    ot_vb_pool     poolId;
    ot_vb_pool_cfg vb_pool_cfg;
    vb_pool_cfg.blk_cnt  = blkCnt;
    vb_pool_cfg.blk_size = blkSize;
    snprintf(vb_pool_cfg.mmz_name, OT_MAX_MMZ_NAME_LEN, "%s", poolName);
    vb_pool_cfg.remap_mode = OT_VB_REMAP_MODE_NONE;
    poolId                 = ss_mpi_vb_create_pool(&vb_pool_cfg);
    // yuyu，没有mmap
    if (OT_VB_INVALID_POOL_ID == poolId) {
        VAPILOG("ss_mpi_vb_create_pool failed with %#x!\n", poolId);
        return TD_FAILURE;
    }
    return poolId;
}

int vapi_vpool_delete(int poolId)
{
    td_u32 s32Ret = TD_SUCCESS;

    // yuyu，没有unmap
    s32Ret = ss_mpi_vb_destroy_pool(poolId);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_vb_destroy_pool failed with %#x!\n", poolId);
        return s32Ret;
    }
    return s32Ret;
}

int vapi_fish_vo(SCREEN_E enScreen, int VoChn, int poolId, int inWidth, int inHeight, void *pVirAddr, int phyAddr)
{
    ot_video_frame_info stVoFrame;
    ot_vo_chn_attr      stChnAttr;
    td_u32              u32BlkHandle;
    td_phys_addr_t      u32PhyAddr;
    td_void            *pu8VirAddr;
    ot_vo_layer         VoLayer;
    td_u32              outWidth, outHeight;
    td_u32              s32Ret = TD_SUCCESS;

    VoLayer = enScreen == SCREEN_SUB ? VAPI_VO_LAYER_VHD1 : VAPI_VO_LAYER_VHD0;
    s32Ret  = ss_mpi_vo_get_chn_attr(VoLayer, VoChn, &stChnAttr);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_get_chn_attr failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    outWidth  = stChnAttr.rect.width;
    outHeight = stChnAttr.rect.height;

    ot_pic_buf_attr buf_attr;
    buf_attr.width         = inWidth;
    buf_attr.height        = inHeight;
    buf_attr.align         = 0;
    buf_attr.bit_width     = OT_DATA_BIT_WIDTH_8;
    buf_attr.pixel_format  = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    td_u32 buf_size        = ot_common_get_pic_buf_size(&buf_attr);

    u32BlkHandle = ss_mpi_vb_get_blk(poolId, buf_size, TD_NULL);
    if (OT_VB_INVALID_HANDLE == u32BlkHandle) {
        VAPILOG("ss_mpi_vb_get_blk fail %#x!\n", u32BlkHandle);
        return TD_FAILURE;
    }

    u32PhyAddr = ss_mpi_vb_handle_to_phys_addr(u32BlkHandle);
    if (u32PhyAddr == 0) {
        ss_mpi_vb_release_blk(u32BlkHandle);
        VAPILOG("ss_mpi_vb_handle_to_phys_addr fail!\n");
        return s32Ret;
    }

    pu8VirAddr = ss_mpi_sys_mmap(u32PhyAddr, buf_size);
    if (pu8VirAddr == TD_NULL) {
        ss_mpi_vb_release_blk(u32BlkHandle);
        VAPILOG("ss_mpi_sys_mmap fail!\n");
        return s32Ret;
    }

    stVoFrame.pool_id                   = poolId;
    stVoFrame.video_frame.compress_mode = OT_COMPRESS_MODE_NONE;
    stVoFrame.video_frame.video_format  = OT_VIDEO_FORMAT_LINEAR;
    stVoFrame.video_frame.pixel_format  = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    stVoFrame.video_frame.field         = OT_VIDEO_FIELD_FRAME;
    stVoFrame.video_frame.pts           = 0;
    stVoFrame.video_frame.phys_addr[0]  = u32PhyAddr;

    if (outWidth > inWidth || outHeight > inHeight) {
        stVoFrame.video_frame.width        = outWidth;
        stVoFrame.video_frame.height       = outHeight;
        stVoFrame.video_frame.phys_addr[1] = u32PhyAddr + (outWidth * outHeight);
        stVoFrame.video_frame.stride[0]    = outWidth;
        stVoFrame.video_frame.stride[1]    = outWidth;
        vapi_vgs_scale(&stVoFrame, phyAddr, inWidth, inHeight);

    } else {
        stVoFrame.video_frame.width        = inWidth;
        stVoFrame.video_frame.height       = inHeight;
        stVoFrame.video_frame.phys_addr[1] = u32PhyAddr + (inWidth * inHeight);
        stVoFrame.video_frame.stride[0]    = inWidth;
        stVoFrame.video_frame.stride[1]    = inWidth;
        vapi_ive_copy(phyAddr, pVirAddr, u32PhyAddr, pu8VirAddr, inWidth, inHeight);
    }

    s32Ret = ss_mpi_vo_send_frame(VoLayer, VoChn, &stVoFrame, 0);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_vo_send_frame failed with %#x!\n", s32Ret);
    }

    s32Ret = ss_mpi_vb_release_blk(u32BlkHandle);

    return s32Ret;
}

int vapi_grab_yuv(SCREEN_E enScreen, int VoChn, void *pVirAddr, int phyAddr)
{
    td_u32              s32Ret = TD_SUCCESS;
    ot_video_frame_info stFrame;
    VAPI_YUV_S          stYUV;
    ot_vpss_grp         Grp;
    ot_vpss_chn         Chn;

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return TD_FAILURE;

    s32Ret = vapi_vpss_snap_yuv(g_stVpssGrp.stBind[Grp].vpssGrp, &stYUV, &stFrame);
    if (s32Ret == TD_SUCCESS) {
        s32Ret = vapi_ive_copy(stYUV.phy, stYUV.data, phyAddr, pVirAddr, stYUV.width, stYUV.height);
        vapi_vpss_snap_free(&stYUV, &stFrame);
    }

    return s32Ret;
}

int vapi_fish_src_wh(SCREEN_E enScreen, int VoChn, int *pInWidth, int *pInHeight, int *pOutWidth, int *pOutHeight)
{
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return TD_FAILURE;

    if (vapi_vpss_chn_get_wh(g_stVpssGrp.stBind[Grp].vpssGrp, Chn, (td_u32 *)pInWidth, (td_u32 *)pInHeight) != TD_SUCCESS)
        return TD_FAILURE;

    if (vapi_vo_chn_get_wh(g_stVpssGrp.stBind[Grp].voutLayer, VoChn, (td_u32 *)pOutWidth, (td_u32 *)pOutHeight) != TD_SUCCESS)
        return TD_FAILURE;

    return TD_SUCCESS;
}

int vapi_get_jpeg_frame(SCREEN_E enScreen, MODE_E enMode, int devID, VAPI_JPEG_S *pstJpeg)
{
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;

    if (enMode == DSP_MODE_PBK)
        devID += MAX_LIVE_NUM;

    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return TD_FAILURE;

    return vapi_vpss_snap_jpeg(g_stVpssGrp.stBind[Grp].vpssGrp, pstJpeg);
}

int vapi_get_jpeg_frame2(SCREEN_E enScreen, int VoChn, VAPI_JPEG_S *pstJpeg)
{
    ot_vpss_grp Grp;
    ot_vpss_chn Chn;

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return TD_FAILURE;

    return vapi_vpss_snap_jpeg(g_stVpssGrp.stBind[Grp].vpssGrp, pstJpeg);
}

void vapi_put_jpeg_frame(VAPI_JPEG_S *pstJpeg)
{
    if (pstJpeg->pu8InAddr) {
        ms_free(pstJpeg->pu8InAddr);
        pstJpeg->pu8InAddr = NULL;
    }
}

int vapi_jpeg_hw_convert(VAPI_JPEG_S *pstJpeg, unsigned char **ppu8OutAddr, unsigned int *pu32OutLen, unsigned int *pu32OutWidth, unsigned int *pu32OutHeight)
{
    td_s32 s32Ret = TD_FAILURE;

    //    if (comm_mutex_timelock(&g_stVpssGrp.mutex, 1000000) != 0)
    //        return s32Ret;
    s32Ret = vapi_jpeg_hw(pstJpeg->enType, pstJpeg->u32InWidth, pstJpeg->u32InHeight, pstJpeg->pu8InAddr, pstJpeg->u32InLen, pstJpeg->u32Qfactor, pstJpeg->u32OutWidth,
                          pstJpeg->u32OutHeight, ppu8OutAddr, pu32OutLen, pu32OutWidth, pu32OutHeight, &g_stVpssGrp.mutex);
    //    comm_mutex_unlock(&g_stVpssGrp.mutex);
    return s32Ret;
}

void vapi_display_mode(DSP_E enDsp)
{
    vapi_vdec_dsp_block(enDsp == DSP_FLUENT);
}

int vapi_init(VAPI_ATTR_S *pstAttr)
{
    td_s32 s32Ret = TD_FAILURE;

    s32Ret = vapi_sys_init();
    if (s32Ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    vapi_bind_init();

    s32Ret = vapi_vo_init(pstAttr->enRes[SCREEN_MAIN], pstAttr->enRes[SCREEN_SUB], pstAttr->isHotplug, pstAttr->isDualScreen, pstAttr->isogeny, pstAttr->prefix[0], &pstAttr->stCallback);
    if (s32Ret != TD_SUCCESS) {
        return TD_FAILURE;
    }

    vapi_vdec_init(&pstAttr->stCallback, pstAttr->isBlock);
    vapi_venc_init(&pstAttr->stCallback);
    vapi_jpeg_init();
    vapi_aio_init();
    vapi_fb_init(SCREEN_MAIN);
    if (pstAttr->isDualScreen) {
        vapi_fb_init(SCREEN_SUB);
    }
    VAPILOG("vapi init done");

    return TD_SUCCESS;
}

int vapi_uninit()
{
    vapi_bind_uninit();
    vapi_venc_uninit();
    vapi_jpeg_uninit();
    vapi_vdec_uninit();
    vapi_vo_uninit();
    vapi_aio_uninit();
    vapi_sys_uninit();

    return TD_SUCCESS;
}
