/*
 * ***************************************************************
 * Filename:        vapi.c
 * Created at:      2016.4.28
 * Description:     public API
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
#include <sys/time.h>

#include "vapi_comm.h"
#include "vapi.h"

typedef struct vpss_bind_s {
    STATE_E     enState;
    HI_S32      vdecChn;
    HI_S32      voutChn;
} VPSS_BIND_S;

typedef struct vpssGrp_s {
    VPSS_BIND_S     stBind[VAPI_VPSS_MAX_NUM];
    pthread_mutex_t mutex;
} VPSSGRP_S;

static VPSSGRP_S g_stVpssGrp;

static void bind_vpss(HI_S32 vdChn, HI_S32 vpssChn, HI_S32 voChn)
{
    if (g_stVpssGrp.stBind[vpssChn].enState == STATE_BUSY) {
        return;
    }

    vapi_vpss_create(vpssChn, vdChn, voChn);
    vapi_vdec_bind_vpss(vdChn, vpssChn);
    vapi_vpss_bind_vo(vpssChn, voChn);

    vapi_vdec_start_stream(vdChn);
    vapi_vpss_start_stream(vpssChn);
    vapi_vo_start_stream(voChn);

    g_stVpssGrp.stBind[vpssChn].enState = STATE_BUSY;
    g_stVpssGrp.stBind[vpssChn].vdecChn = vdChn;
    g_stVpssGrp.stBind[vpssChn].voutChn = voChn;
}

static HI_S32 unbind_vpss(HI_S32 vpssChn)
{
    HI_S32 vdChn = g_stVpssGrp.stBind[vpssChn].vdecChn;
    HI_S32 voChn = g_stVpssGrp.stBind[vpssChn].voutChn;

    if (g_stVpssGrp.stBind[vpssChn].enState != STATE_BUSY) {
        return HI_FAILURE;
    }

    vapi_vdec_stop_stream(vdChn);
    vapi_vpss_stop_stream(vpssChn);
    vapi_vo_stop_stream(voChn);

    vapi_vdec_unbind_vpss(vdChn);
    vapi_vpss_unbind_vo(vpssChn);
    vapi_vpss_destory(vpssChn);

    g_stVpssGrp.stBind[vpssChn].enState = STATE_IDLE;

    return HI_SUCCESS;
}

static void bind_init()
{
    HI_S32 i;

    for (i = 0; i < VAPI_VPSS_MAX_NUM; i++) {
        g_stVpssGrp.stBind[i].enState = STATE_IDLE;
        g_stVpssGrp.stBind[i].vdecChn = -1;
        g_stVpssGrp.stBind[i].voutChn = -1;
    }
    comm_mutex_init(&g_stVpssGrp.mutex);
}

static void bind_uninit()
{
    HI_S32 i;

    for (i = 0; i < VAPI_VPSS_MAX_NUM; i++) {
        unbind_vpss(i);
    }
    comm_mutex_uninit(&g_stVpssGrp.mutex);
}

static inline HI_S32 get_vpss(SCREEN_E enScreen, HI_S32 voChn)
{
    return enScreen * MAX_CHN_NUM + voChn;
}

static HI_S32 del_one_win(SCREEN_E enScreen, int voChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 vdChn;
    HI_S32 vpssChn;

    comm_mutex_lock(&g_stVpssGrp.mutex);
    vpssChn = get_vpss(enScreen, voChn);
    s32Ret = unbind_vpss(vpssChn);
    voChn   = g_stVpssGrp.stBind[vpssChn].voutChn;
    vdChn   = g_stVpssGrp.stBind[vpssChn].vdecChn;
    if (s32Ret == HI_SUCCESS) {
        s32Ret = vapi_vo_chn_destory(voChn);
        s32Ret |= vapi_vdec_chn_destory(vdChn);
        g_stVpssGrp.stBind[vpssChn].vdecChn = -1;
        if (s32Ret == HI_SUCCESS) {
            comm_mutex_unlock(&g_stVpssGrp.mutex);
            return -1;
        }
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

static HI_S32 add_one_win(SCREEN_E enScreen, WIND_S *pstWin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    int devID = pstWin->stDevinfo.id;
    TYPE_E enType = pstWin->stDevinfo.stFormat.enType;
    HI_U32 u32Width = pstWin->stDevinfo.stFormat.width;
    HI_U32 u32Height = pstWin->stDevinfo.stFormat.height;
    HI_S32 voChn = pstWin->stDevinfo.stBind.voutChn;
    HI_S32 vdChn = pstWin->stDevinfo.stBind.vdecChn;
    HI_S32 vpssChn;
    HD_URECT stRect;
    RATIO_E enRatio = pstWin->enRatio;

    //zbing debug
    //enRatio = RATIO_VO_FULL;

    devID = pstWin->stDevinfo.id;
    vpssChn = get_vpss(enScreen, voChn);
    if (g_stVpssGrp.stBind[vpssChn].enState == STATE_BUSY) {
        // Grp has existed ;
        return HI_SUCCESS;
    }
    comm_mutex_lock(&g_stVpssGrp.mutex);

    //create a display channel
    stRect.x        = pstWin->stZone.x;
    stRect.y        = pstWin->stZone.y;
    stRect.w        = pstWin->stZone.w;
    stRect.h        = pstWin->stZone.h;

    s32Ret = vapi_vo_chn_create(voChn, &stRect, enRatio);

    //bind : vdec-> vpss->vout
    if (s32Ret == HI_SUCCESS) {
        //create a vdec channel
        s32Ret = vapi_vdec_chn_create(devID, vdChn, enType, DSP_MODE_LIVE, u32Width, u32Height);
        if (s32Ret == HI_SUCCESS) {
            //bind vdec->vpss->vo
            bind_vpss(vdChn, vpssChn, voChn);
        } else {
            vapi_vo_chn_destory(voChn);
        }
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

int yu_vapi_add_one_win(SCREEN_E enScreen, MODE_E enMode, WIND_S *pstWin)
{
    return add_one_win(enScreen, pstWin);
}
int yu_vapi_del_one_win(SCREEN_E enScreen, MODE_E enMode, int devID, REFRESH_E enRefresh)
{
    return del_one_win(enScreen, devID);
}

/*
* *********************************public************************************
*/
int vapi_get_vdec_chn(SCREEN_E enScreen, MODE_E enMode, int devID)
{
    //discard
    return HI_FAILURE;
}

int vapi_get_vdec_chn2(SCREEN_E enScreen, int voChn)
{
    HI_BOOL s32Ret;
    HI_S32 vdecChn;
    HI_S32 vpssChn;

    vpssChn = get_vpss(enScreen, voChn);
    vdecChn = g_stVpssGrp.stBind[vpssChn].vdecChn;

    s32Ret = vapi_vdec_check_state(vdecChn, STATE_BUSY);

    if (s32Ret == HI_TRUE) {
        return vdecChn;
    } else {
        VAPILOG("dev[%d] not use any vdec chn \n", voChn);
        return -1;
    }
}

void vapi_get_screen_res(SCREEN_E enScreen, int *W, int *H)
{
    vapi_vo_get_srceen_res(enScreen, W, H);
}

void vapi_clear_vout_chn(SCREEN_E enScreen, int voChn)
{
    DEV_E voDev;

    if (enScreen == SCREEN_MAIN) {
        voDev = VAPI_VO_DEV_VIDEO;
    } else if (enScreen == SCREEN_SUB) {
        voDev = VAPI_VO_DEV_GUI;
    } else {
        VAPILOG("Screen type [%d] invalid \n", enScreen);
        return ;
    }

    vapi_vo_chn_clear_buff(voDev, voChn);

}

void vapi_clear_screen(SCREEN_E enScreen, REFRESH_E enRefresh)
{
    //discard
}

void vapi_clear_screen2(SCREEN_E enScreen)
{
    HI_S32 i;

    for (i = 0; i < MAX_CHN_NUM; i++) {
        del_one_win(enScreen, i);
    }
}

int vapi_update_layout(LAYOUT_S *pstLayout)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;

    //add new window
    for (i = 0; i < pstLayout->winds_num; i++) {
        s32Ret |= add_one_win(pstLayout->enScreen, &pstLayout->stWinds[i]);
    }

    return s32Ret;
}

int vapi_set_screen_resolution(SCREEN_RES_S *pstScreenRes)
{
    HI_S32 s32Ret = HI_SUCCESS;

    if (pstScreenRes->enScreen == SCREEN_SUB) {
        return HI_SUCCESS; //have no subscreen
    }
    //clear current screen
    vapi_clear_screen2(pstScreenRes->enScreen);
    vapi_vo_dev_close(VAPI_VO_DEV_VIDEO);
    vapi_vo_dev_close(VAPI_VO_DEV_GUI);

    s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_GUI,
                              pstScreenRes->enRes);
    if (s32Ret != HI_SUCCESS) {
        return HI_FAILURE;
    }

    s32Ret = vapi_vo_dev_open(VAPI_VO_DEV_VIDEO,
                              pstScreenRes->enRes);
    if (s32Ret != HI_SUCCESS) {
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

int vapi_switch_screen(SCREEN_E enScreen)
{
    HI_S32 s32Ret = HI_SUCCESS;
    return s32Ret;
}

int vapi_set_zoomin(ZOOMIN_S *pstZoomin)
{
    //discard
    return HI_FAILURE;
}

int vapi_set_zoomin2(ZOOMIN2_S *pstZoomin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HD_URECT  stRect;
    SCREEN_E enScreen = pstZoomin->enScreen;
    int voChn = pstZoomin->VoChn;
    HI_S32 vpssChn;
    HI_S32 W, H;

    vpssChn = get_vpss(enScreen, voChn);
    vapi_get_screen_res(enScreen, &W, &H);
    stRect.w        = VAPI_MAX(pstZoomin->stRect.w, W / 7); //pstZoomin->stRect.w;
    stRect.h        = VAPI_MAX(pstZoomin->stRect.h, H / 7); //pstZoomin->stRect.h;
    //98633 双目全景 1080P下高度低于280放大无法解析
    if (pstZoomin->stRect.w / pstZoomin->stRect.h >= 3) {
        stRect.w        = VAPI_MAX(pstZoomin->stRect.w, (double)W / 3.8); //pstZoomin->stRect.w;
        stRect.h        = VAPI_MAX(pstZoomin->stRect.h, (double)H / 3.8);
    }
    stRect.x        = pstZoomin->stRect.x;
    stRect.y        = pstZoomin->stRect.y;
    s32Ret = vapi_vpss_set_crop(vpssChn, g_stVpssGrp.stBind[vpssChn].vdecChn, &stRect, HD_CROP_ON);

    return s32Ret;
}


int vapi_set_full_screen(FULL_S *pstFull)
{
    //discard
    return HI_FAILURE;
}

int vapi_set_action(ACTION_S *pstAction)
{
    //discard
    return HI_FAILURE;
}

int vapi_set_action2(ACTION2_S *pstAction)
{
    HI_S32 s32Ret = HI_SUCCESS;

    switch (pstAction->enAction) {
        case ACTION_DEL_ONE_WIN:
            s32Ret = del_one_win(pstAction->enScreen,
                                 pstAction->stWind.stDevinfo.stBind.voutChn);
            break;
        case ACTION_ADD_ONE_WIN:
            s32Ret = add_one_win(pstAction->enScreen, &pstAction->stWind);
            break;
        default :
            VAPILOG("Unknown action [%d] !\n", pstAction->enAction);
            s32Ret = HI_FAILURE;
            break;
    }

    return s32Ret;


}

int vapi_set_vout_img(IMG_S *pstImg)
{
    //discard
    return HI_FAILURE;
}

int vapi_set_vout_ratio(RATIO_S *pstRatio)
{
    //discard
    return HI_FAILURE;
}

int vapi_set_vout_ratio2(RATIO2_S *pstRatio)
{
    HI_BOOL s32Ret;
    HI_S32 vpssChn;
    HI_S32 voutChn;


    vpssChn = get_vpss(pstRatio->enScreen, pstRatio->vouid);
    voutChn = g_stVpssGrp.stBind[vpssChn].voutChn;

    if (voutChn < 0) {
        VAPILOG("voutChn[%d] invalid paramter\n", voutChn);
        return HI_FAILURE;
    }

    if (!vapi_vo_check_state(voutChn, STATE_BUSY)) {
        return HI_FAILURE;
    }

    s32Ret = vapi_vo_set_ratio(voutChn, pstRatio->enRatio);

    return s32Ret;
}

int vapi_set_vout_fps(int devID, int fps)
{
    return HI_SUCCESS;
}

int vapi_send_frame(int VdChn, unsigned char *pAddr, int Len, unsigned long long pts, int flag)
{	
    return vapi_vdec_send_frame(VdChn, pAddr, Len, pts, flag);
}

int vapi_audio_read(int AiDevId, ARATE_E enArate, AENC_E enAenc, AIO_FRAME_S *pstFrame)
{
    return vapi_aio_ai_read_frame(AiDevId, enArate, enAenc, pstFrame);
}

int vapi_audio_write(AO_E AoDevId, AIO_FRAME_S *pstFrame)
{
    return vapi_aio_ao_write_frame(AoDevId,  pstFrame);
}

int vapi_audio_ctrl(AIO_CTRL_S *pstCtrl)
{
    HI_S32 s32Ret = HI_FAILURE;

    switch (pstCtrl->enCmd) {
        case ACMD_VI_SET_VOLUME:
            s32Ret = vapi_aio_ai_set_volume(pstCtrl->devID, pstCtrl->value);
            break;
        case ACMD_VO_SET_VOLUME:
            s32Ret = vapi_aio_ao_set_volume(pstCtrl->devID, pstCtrl->value);
            break;
        case ACMD_VO_SET_MUTE:
            s32Ret = vapi_aio_ao_set_mute(pstCtrl->devID, pstCtrl->value);
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
    return -1;
}

void vapi_transfer_destory(int VeChn)
{
}

int vapi_transfer_config(int VeChn, int FPS, int bitRate)
{
    return -1;
}

int vapi_transfer_check()
{
    return -1;
}

int vapi_jpeg_hw_convert(VAPI_JPEG_S *pstJpeg, unsigned char **ppu8OutAddr, unsigned int *pu32OutLen,
                         unsigned int *pu32OutWidth, unsigned int *pu32OutHeight)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = vapi_jpeg_hw(pstJpeg->enType, pstJpeg->u32InWidth, pstJpeg->u32InHeight,
                          pstJpeg->pu8InAddr, pstJpeg->u32InLen,
                          pstJpeg->u32Qfactor,
                          pstJpeg->u32OutWidth, pstJpeg->u32OutHeight,
                          ppu8OutAddr, pu32OutLen, pu32OutWidth, pu32OutHeight,
                          &g_stVpssGrp.mutex);

    return s32Ret;
}

void vapi_display_mode(DSP_E enDsp)
{
    vapi_vo_dsp_quick(enDsp == DSP_REALTIME);
}

int vapi_get_jpeg_frame(SCREEN_E enScreen, MODE_E enMode, int devID, VAPI_JPEG_S *pstJpeg)
{
    //discard
    return HI_FAILURE;
}

int vapi_get_jpeg_frame2(SCREEN_E enScreen, int voChn, VAPI_JPEG_S *pstJpeg)
{
    HI_S32 vpssChn;
    HI_S32 vdChn;

    vpssChn = get_vpss(enScreen, voChn);
    if (g_stVpssGrp.stBind[vpssChn].enState == STATE_IDLE) {
        return HI_FAILURE;
    }

    vdChn = g_stVpssGrp.stBind[vpssChn].vdecChn;

    return vapi_yuv2jpeg(vdChn, pstJpeg);
}

void vapi_put_jpeg_frame(VAPI_JPEG_S *pstJpeg)
{
    return;
    if (pstJpeg->pu8InAddr) {
        ms_free(pstJpeg->pu8InAddr);
        pstJpeg->pu8InAddr = NULL;
    }
}

int vapi_init(VAPI_ATTR_S *pstAttr)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = vapi_sys_init();
    if (s32Ret != HI_SUCCESS) {
        return HI_FAILURE;
    }

    bind_init();

    s32Ret = vapi_vo_init(pstAttr->enRes[SCREEN_MAIN],
                          pstAttr->enRes[SCREEN_SUB],
                          pstAttr->isHotplug,
                          !pstAttr->isBlock,
                          &pstAttr->stCallback);
    if (s32Ret != HI_SUCCESS) {
        return HI_FAILURE;
    }

    vapi_vpss_init();
    vapi_vdec_init(&pstAttr->stCallback);
    vapi_venc_init(&pstAttr->stCallback);
    vapi_jpeg_init();
    vapi_aio_init();
    vapi_fb_init(SCREEN_MAIN);

    return HI_SUCCESS;
}

int vapi_uninit()
{
    bind_uninit();
    vapi_jpeg_uninit();
    vapi_vdec_uninit();
    vapi_vo_uninit();
    vapi_aio_uninit();
    vapi_fb_uninit();
    vapi_venc_uninit();
    vapi_vpss_uninit();
    vapi_sys_uninit();

    return HI_SUCCESS;
}

