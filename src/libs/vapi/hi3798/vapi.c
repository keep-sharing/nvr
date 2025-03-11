/* 
 * ***************************************************************
 * Filename:      	vapi.c
 * Created at:    	2016.4.28
 * Description:   	public API
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
#include "vapi.h"

typedef struct vpss_bind_s{
    STATE_E     enState;
    HI_S32      vdecChn;
    HI_S32      voutChn;
    DEV_E       voDev;
}VPSS_BIND_S;

typedef struct vpssGrp_s{
    VPSS_BIND_S     stBind[VAPI_VPSS_MAX_NUM];
    pthread_mutex_t mutex;
}VPSSGRP_S;

static VPSSGRP_S g_stVpssGrp;

static void vapi_bind_vpss(HI_S32 VdChn, 
                             HI_S32 VpssGrp,
                             HI_U32 MaxW,HI_U32 MaxH,
                             DEV_E VoDev, HI_S32 VoChn)
{
    HI_HANDLE vdecHDL;
    
    if(g_stVpssGrp.stBind[VpssGrp].enState == STATE_BUSY)
    {
        return;
    }

    vdecHDL = vapi_vdec_get_chn_handle(VdChn);
    vapi_vo_bind_vdec(VoDev, VoChn, vdecHDL);
    vapi_vdec_start_stream(VdChn);
    
    g_stVpssGrp.stBind[VpssGrp].enState = STATE_BUSY;
    g_stVpssGrp.stBind[VpssGrp].vdecChn = VdChn;
    g_stVpssGrp.stBind[VpssGrp].voutChn = VoChn;
    g_stVpssGrp.stBind[VpssGrp].voDev   = VoDev;    
}

static HI_S32 vapi_unbind_vpss(HI_S32 VpssGrp)
{
    HI_S32 VdChn = g_stVpssGrp.stBind[VpssGrp].vdecChn; 
    DEV_E voDev = g_stVpssGrp.stBind[VpssGrp].voDev; 
    HI_S32 VoChn = g_stVpssGrp.stBind[VpssGrp].voutChn;    
    HI_HANDLE vdecHDL;
    
    if(g_stVpssGrp.stBind[VpssGrp].enState != STATE_BUSY)
    {
        return HI_FAILURE;
    }
    
    vapi_vdec_stop_stream(VdChn);    
    vdecHDL = vapi_vdec_get_chn_handle(VdChn); 
    vapi_vo_unbind_vdec(voDev, VoChn, vdecHDL);
    
    g_stVpssGrp.stBind[VpssGrp].enState = STATE_IDLE;
//    g_stVpssGrp.stBind[VpssGrp].vdecChn = -1;

    return HI_SUCCESS;
}

static void vapi_bind_init()
{
    HI_S32 i;

    for(i=0; i<VAPI_VPSS_MAX_NUM; i++)
    {
        g_stVpssGrp.stBind[i].enState = STATE_IDLE;
        g_stVpssGrp.stBind[i].voDev = -1;
        g_stVpssGrp.stBind[i].vdecChn = -1;
        g_stVpssGrp.stBind[i].voutChn = -1;
    }
    comm_mutex_init(&g_stVpssGrp.mutex);
}

static void vapi_bind_uninit()
{
    HI_S32 i;

    for(i=0; i<VAPI_VPSS_MAX_NUM; i++)
    {
        vapi_unbind_vpss(i);
    }
    comm_mutex_uninit(&g_stVpssGrp.mutex);
}

static inline void vapi_get_vpss(SCREEN_E enScreen, MODE_E enMode, 
                                    int devID, HI_S32 *pGrp)
{
    if (enMode == DSP_MODE_ZOOMIN)
    {
        *pGrp = enScreen * MAX_CHN_NUM + MAX_CHN_NUM - 1;//每个屏幕最后一个vpss作为缩放组
    }
    else
    {
        //*pGrp = enScreen * MAX_CHN_NUM + enMode * MAX_LIVE_NUM + devID;
        *pGrp = enScreen * MAX_CHN_NUM + devID;
    }
}

static inline void vapi_get_vpss2(SCREEN_E enScreen, int VoChn, HI_S32 *pGrp)
{
    *pGrp = enScreen * MAX_CHN_NUM + VoChn;
}

static HI_S32 vapi_del_one_win(SCREEN_E enScreen, MODE_E enMode, 
                                   int devID, REFRESH_E enRefresh)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 Grp; 
    DEV_E  voDev;
    HI_S32 VoChn;
    HI_S32 VdChn;
    
    comm_mutex_lock(&g_stVpssGrp.mutex);

    vapi_get_vpss(enScreen, enMode, devID, &Grp);
    s32Ret = vapi_unbind_vpss(Grp);
    voDev   = g_stVpssGrp.stBind[Grp].voDev;
    VoChn   = g_stVpssGrp.stBind[Grp].voutChn;
    VdChn   = g_stVpssGrp.stBind[Grp].vdecChn;
    if (s32Ret == HI_SUCCESS)
    {
        s32Ret = vapi_vo_chn_destory(voDev, VoChn);
    }
    if (enRefresh == REFRESH_CLEAR_VDEC)
    {
        s32Ret  |= vapi_vdec_chn_destory(VdChn);
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
    }
    
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

static HI_S32 vapi_del_one_win2(SCREEN_E enScreen, int VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 Grp; 
    DEV_E  voDev;
    HI_S32 VdChn;
    
    comm_mutex_lock(&g_stVpssGrp.mutex);
    vapi_get_vpss2(enScreen, VoChn, &Grp);
    s32Ret = vapi_unbind_vpss(Grp);
    voDev   = g_stVpssGrp.stBind[Grp].voDev;
    VoChn   = g_stVpssGrp.stBind[Grp].voutChn;
    VdChn   = g_stVpssGrp.stBind[Grp].vdecChn;
    if (s32Ret == HI_SUCCESS)
    {
        s32Ret = vapi_vo_chn_destory(voDev, VoChn);
        s32Ret |= vapi_vdec_chn_destory(VdChn);
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
        if (s32Ret == HI_SUCCESS)
        {
            comm_mutex_unlock(&g_stVpssGrp.mutex);
            return -1;
        }
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

static HI_S32 vapi_add_one_win(SCREEN_E enScreen, MODE_E enMode, WIND_S *pstWin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 Grp; 
    int devID = pstWin->stDevinfo.id;
    TYPE_E enType = pstWin->stDevinfo.stFormat.enType;
    HI_U32 u32Width = pstWin->stDevinfo.stFormat.width;
    HI_U32 u32Height = pstWin->stDevinfo.stFormat.height;
    DEV_E voDev;
    HI_S32 VoChn = pstWin->stDevinfo.stBind.voutChn;
    HI_S32 VdChn = pstWin->stDevinfo.stBind.vdecChn;
    HI_RECT_S stRect;
    RATIO_E enRatio = pstWin->enRatio;

    //zbing debug
    //enRatio = RATIO_VO_FULL;

    comm_mutex_lock(&g_stVpssGrp.mutex);

    devID = pstWin->stDevinfo.id;
    vapi_get_vpss(enScreen, enMode, devID, &Grp);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY)
    {
        // Grp has existed ;
        comm_mutex_unlock(&g_stVpssGrp.mutex);
        return HI_SUCCESS;
    }

    //create a display channel
    stRect.s32X         = pstWin->stZone.x;
    stRect.s32Y         = pstWin->stZone.y;
    stRect.s32Width     = pstWin->stZone.w;
    stRect.s32Height    = pstWin->stZone.h;

    voDev = enScreen == SCREEN_MAIN? VAPI_VO_DEV_DHD: VAPI_VO_DEV_DSD;
    s32Ret = vapi_vo_chn_create(voDev, VoChn, &stRect, enRatio);

    //bind : vdec-> vpss->vout
    if (s32Ret == HI_SUCCESS)
    {
        //create a vdec channel
        s32Ret = vapi_vdec_chn_create(devID, VdChn, enType, enMode, u32Width, u32Height);
        if (s32Ret == HI_SUCCESS)
        {
            //bind vdec->vpss->vo
            vapi_bind_vpss(VdChn, Grp, u32Width, u32Height, voDev, VoChn);        
        }
        else
        {
            vapi_vo_chn_destory(voDev, VoChn);
        }
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

static HI_S32 vapi_add_one_win2(SCREEN_E enScreen, WIND_S *pstWin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 Grp; 
    int devID = pstWin->stDevinfo.id;
    TYPE_E enType = pstWin->stDevinfo.stFormat.enType;
    HI_U32 u32Width = pstWin->stDevinfo.stFormat.width;
    HI_U32 u32Height = pstWin->stDevinfo.stFormat.height;
    DEV_E voDev;
    HI_S32 VoChn = pstWin->stDevinfo.stBind.voutChn;
    HI_S32 VdChn = pstWin->stDevinfo.stBind.vdecChn;
    HI_RECT_S stRect;
    RATIO_E enRatio = pstWin->enRatio;

    //zbing debug
    //enRatio = RATIO_VO_FULL;

    devID = pstWin->stDevinfo.id;
    vapi_get_vpss2(enScreen, VoChn, &Grp);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY)
    {
        // Grp has existed ;
        return HI_SUCCESS;
    }
    comm_mutex_lock(&g_stVpssGrp.mutex);

    //create a display channel
    stRect.s32X         = pstWin->stZone.x;
    stRect.s32Y         = pstWin->stZone.y;
    stRect.s32Width     = pstWin->stZone.w;
    stRect.s32Height    = pstWin->stZone.h;

    voDev = enScreen == SCREEN_MAIN? VAPI_VO_DEV_DHD: VAPI_VO_DEV_DSD;
    s32Ret = vapi_vo_chn_create(voDev, VoChn, &stRect, enRatio);

    //bind : vdec-> vpss->vout
    if (s32Ret == HI_SUCCESS)
    {
        //create a vdec channel
        s32Ret = vapi_vdec_chn_create(devID, VdChn, enType, DSP_MODE_LIVE, u32Width, u32Height);
        if (s32Ret == HI_SUCCESS)
        {
            //bind vdec->vpss->vo
            vapi_bind_vpss(VdChn, Grp, u32Width, u32Height, voDev, VoChn);        
        }
        else
        {
            vapi_vo_chn_destory(voDev, VoChn);
        }
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

/*
* *********************************public************************************
*/
int vapi_get_vdec_chn(SCREEN_E enScreen, MODE_E enMode, int devID)
{
    HI_BOOL s32Ret;
    HI_S32 Grp; 
    HI_S32 vdecChn;

    vapi_get_vpss(enScreen, enMode, devID, &Grp);
    vdecChn = g_stVpssGrp.stBind[Grp].vdecChn;

    s32Ret = vapi_vdec_check_state(vdecChn, STATE_BUSY);
    
    if (s32Ret == HI_TRUE)
    {
        return vdecChn;
    }
    else
    {
        VAPILOG("dev[%d] not use any vdec chn \n", devID);
        return -1;
    }    
}

int vapi_get_vdec_chn2(SCREEN_E enScreen, int VoChn)
{
    HI_BOOL s32Ret;
    HI_S32 Grp; 
    HI_S32 vdecChn;

    vapi_get_vpss2(enScreen, VoChn, &Grp);
    vdecChn = g_stVpssGrp.stBind[Grp].vdecChn;

    s32Ret = vapi_vdec_check_state(vdecChn, STATE_BUSY);
    
    if (s32Ret == HI_TRUE)
    {
        return vdecChn;
    }
    else
    {
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
    DEV_E voDev;
    
    if (enScreen == SCREEN_MAIN)
    {
        voDev = VAPI_VO_DEV_DHD;
    }
    else if (enScreen == SCREEN_SUB)
    {
        voDev = VAPI_VO_DEV_DSD;
    }
    else 
    {
        VAPILOG("Screen type [%d] invalid \n", enScreen);
        return ;
    }
    
    vapi_vo_chn_clear_buff(voDev, VoChn);
    
}

void vapi_clear_screen(SCREEN_E enScreen, REFRESH_E enRefresh)
{
    HI_S32 i;

    for(i=0; i<MAX_CHN_NUM; i++)
    {
        vapi_del_one_win(enScreen, 0, i, enRefresh);
    }
}

void vapi_clear_screen2(SCREEN_E enScreen)
{
    HI_S32 i;

    for(i=0; i<MAX_CHN_NUM; i++)
    {
        vapi_del_one_win2(enScreen, i);
    }
}

static void vapi_enter_zoomin(ZOOMIN_S *pstZoomin)
{
    SCREEN_E enScreen = pstZoomin->enScreen;
    WIND_S  stWind;
    HI_S32 Grp; 
    
    memset(&stWind, 0, sizeof(WIND_S));
    vapi_get_vpss(enScreen,  pstZoomin->enMode, pstZoomin->devID, &Grp);
    
    stWind.stDevinfo.id = pstZoomin->devID;
    stWind.stDevinfo.stFormat = pstZoomin->stFormat;
    stWind.stDevinfo.stBind.vdecChn = pstZoomin->vdecID;
    stWind.stDevinfo.stBind.voutChn = 0;
    
    // 1. 清屏: 如果解码性能允许保留后台解码
    vapi_clear_screen(enScreen, pstZoomin->enRefresh);

    // 2. 添加缩放视图(大窗口)
    stWind.stZone = pstZoomin->stZone[WIN_TYPE_NORMAL];
    vapi_add_one_win(enScreen, pstZoomin->enMode, &stWind);
    
    stWind.stDevinfo.stBind.voutChn = 1;
    // 3. 添加全景预览视图(小窗口) 
    stWind.stZone = pstZoomin->stZone[WIN_TYPE_ZOOMIN];
    vapi_add_one_win(enScreen, DSP_MODE_ZOOMIN, &stWind);
    
}

static int vapi_start_zoomin(ZOOMIN_S *pstZoomin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_RECT_S stRect;
    SCREEN_E enScreen = pstZoomin->enScreen;
    MODE_E enMode = pstZoomin->enMode;
    int devID = pstZoomin->devID;    
    HI_S32 Grp; 
    DEV_E  voDev;
    HI_S32 voChn;
    HI_S32 W, H;

    vapi_get_vpss(enScreen, enMode, devID, &Grp);
    voDev = g_stVpssGrp.stBind[Grp].voDev;    
    voChn = g_stVpssGrp.stBind[Grp].voutChn;
    vapi_get_screen_res(enScreen, &W, &H);
//    printf("=====ratic_w %d, ratio_h %d min_w %d, min h %d\n",W/pstZoomin->stRect.w, H/pstZoomin->stRect.h, W/16, H/16);
//    printf("===o_x %d, o_y %d, o_w %d, o_h %d\n", pstZoomin->stRect.x,pstZoomin->stRect.y,pstZoomin->stRect.w, pstZoomin->stRect.h);
    stRect.s32Width     = VAPI_MAX(pstZoomin->stRect.w, W/15);//pstZoomin->stRect.w;
    stRect.s32Height    = VAPI_MAX(pstZoomin->stRect.h, H/15);//pstZoomin->stRect.h;
    stRect.s32X         = VAPI_MIN(pstZoomin->stFormat.width-stRect.s32Width, pstZoomin->stRect.x);//pstZoomin->stRect.x;
    stRect.s32Y         = VAPI_MIN(pstZoomin->stFormat.height-stRect.s32Height, pstZoomin->stRect.y);//pstZoomin->stRect.y;
//    printf("===m_x %d, m_y %d, m_w %d, m_h %d\n", stRect.s32X,stRect.s32Y,stRect.u32Width, stRect.u32Height);
//    s32Ret = vapi_vpss_set_crop(Grp, &stRect);
    vapi_vo_set_crop(voDev, voChn, &stRect);

    return s32Ret;
}

int vapi_update_layout(LAYOUT_S *pstLayout)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
        
    //add new window
    for(i=0; i<pstLayout->winds_num; i++)
    {
        s32Ret |= vapi_add_one_win(pstLayout->enScreen, 
                                   pstLayout->enMode, 
                                  &pstLayout->stWinds[i]);
    }

    return s32Ret;
}

int vapi_set_screen_resolution(SCREEN_RES_S *pstScreenRes)
{
    HI_S32 s32Ret = HI_SUCCESS;

    //clear current screen
    vapi_clear_screen2(pstScreenRes->enScreen);
    printf("=======vapi_set_screen_resolution===\n\n");
    s32Ret = vapi_vo_dev_close(pstScreenRes->enScreen);    
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }
    
    s32Ret = vapi_vo_dev_open(pstScreenRes->enScreen, 
                              pstScreenRes->enRes);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }
    
    //hjh 20160212 add, for QT switch output res.
    s32Ret = vapi_switch_display(pstScreenRes->enScreen);
    if(s32Ret != HI_SUCCESS)
    {
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
    HI_S32 s32Ret = HI_SUCCESS;
    
    if (pstZoomin->enState == STATE_ENTER)
    {
        vapi_enter_zoomin(pstZoomin);
//        s32Ret = vapi_start_zoomin(pstZoomin);
    }
    else if (pstZoomin->enState == STATE_ZOOMIN)
    {        
        s32Ret = vapi_start_zoomin(pstZoomin);
    }

    return s32Ret;
}

int vapi_set_zoomin2(ZOOMIN2_S *pstZoomin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_RECT_S stRect;
    SCREEN_E enScreen = pstZoomin->enScreen;
    int VoChn = pstZoomin->VoChn;    
    HI_S32 Grp; 
    DEV_E  voDev;
    HI_S32 voChn;
    HI_S32 W, H;

    vapi_get_vpss2(enScreen, VoChn, &Grp);
    voDev = g_stVpssGrp.stBind[Grp].voDev;    
    voChn = g_stVpssGrp.stBind[Grp].voutChn;
    vapi_get_screen_res(enScreen, &W, &H);
    stRect.s32Width     = VAPI_MAX(pstZoomin->stRect.w, W/15);//pstZoomin->stRect.w;
    stRect.s32Height    = VAPI_MAX(pstZoomin->stRect.h, H/15);//pstZoomin->stRect.h;
    stRect.s32X         = pstZoomin->stRect.x;
    stRect.s32Y         = pstZoomin->stRect.y;
    vapi_vo_set_crop(voDev, voChn, &stRect);

    return s32Ret;
}


int vapi_set_full_screen(FULL_S *pstFull)
{
    HI_S32 s32Ret = HI_SUCCESS;
    WIND_S  stWind;
    HI_S32 Grp; 
    
    memset(&stWind, 0, sizeof(WIND_S));
    // 1.clear screen 
    vapi_clear_screen(pstFull->enScreen, pstFull->enRefresh);

    // 2.add a full view
    vapi_get_vpss(pstFull->enScreen, pstFull->enMode, pstFull->devID, &Grp);
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
    HI_S32 s32Ret = HI_SUCCESS;
    
    switch(pstAction->enAction)
    {
        case ACTION_DEL_ONE_WIN: 
            s32Ret = vapi_del_one_win(pstAction->enScreen, pstAction->enMode,
                             pstAction->stWind.stDevinfo.id, REFRESH_CLEAR_VDEC);
             break;
        case ACTION_ADD_ONE_WIN:
            s32Ret = vapi_add_one_win(pstAction->enScreen, pstAction->enMode,
                             &pstAction->stWind);
             break;
        default :
            VAPILOG("Unknown action [%d] !\n", pstAction->enAction);
            s32Ret = HI_FAILURE;
            break;
    }
    
    return s32Ret;

}   

int vapi_set_action2(ACTION2_S *pstAction)
{
    HI_S32 s32Ret = HI_SUCCESS;
    
    switch(pstAction->enAction)
    {
        case ACTION_DEL_ONE_WIN: 
            s32Ret = vapi_del_one_win2(pstAction->enScreen,
                             pstAction->stWind.stDevinfo.stBind.voutChn);
             break;
        case ACTION_ADD_ONE_WIN:
            s32Ret = vapi_add_one_win2(pstAction->enScreen, &pstAction->stWind);
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
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = vapi_vo_set_csc(pstImg->enScreen, pstImg->enCSC);
    
    return s32Ret;
}

int vapi_set_vout_ratio(RATIO_S *pstRatio)
{
    HI_BOOL s32Ret;
    HI_S32 Grp; 
    HI_S32 voutChn;
    DEV_E  voDev;


    vapi_get_vpss(pstRatio->enScreen, pstRatio->enMode, pstRatio->devId, &Grp);
    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
    voDev = g_stVpssGrp.stBind[Grp].voDev;

	if(voDev<0 || voutChn<0){
		VAPILOG("dev[%d] not use any vdec chn \n", pstRatio->devId);
		return -1;
	}
	
    s32Ret = vapi_vo_check_state(voDev, voutChn, STATE_BUSY);
    if (s32Ret == HI_FAILURE)
    {
        VAPILOG("dev[%d] not use any vdec chn \n", pstRatio->devId);
        return -1;
    }  

	s32Ret = vapi_vo_set_ratio(voDev, voutChn, pstRatio->enRatio);
	
	return s32Ret;
}

int vapi_set_vout_ratio2(RATIO2_S *pstRatio)
{
    HI_BOOL s32Ret;
    HI_S32 Grp; 
    HI_S32 voutChn;
    DEV_E  voDev;


    vapi_get_vpss2(pstRatio->enScreen, pstRatio->vouid, &Grp);
    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
    voDev = g_stVpssGrp.stBind[Grp].voDev;

	if(voDev<0 || voutChn<0){
		VAPILOG("voDev[%d] voutChn[%d] invalid paramter\n", voDev, voutChn);
		return -1;
	}
	
    if (!vapi_vo_check_state(voDev, voutChn, STATE_BUSY))
        return -1;

	s32Ret = vapi_vo_set_ratio(voDev, voutChn, pstRatio->enRatio);
	
	return s32Ret;
}

int vapi_set_vout_fps(int devID, int fps)
{
    return HI_SUCCESS;
}

int vapi_send_frame(int VdChn, unsigned char *pAddr, int Len, unsigned long long pts, int flag)
{
    return vapi_vdec_send_frame(VdChn, pAddr, Len, pts);    
}

int vapi_audio_read(int AiDevId, ARATE_E enArate, AENC_E enAenc, AIO_FRAME_S * pstFrame)
{
    return vapi_aio_ai_read_frame(AiDevId, enArate, enAenc, pstFrame);
}

int vapi_audio_write(AO_E AoDevId, AIO_FRAME_S * pstFrame)
{
    return vapi_aio_ao_write_frame(AoDevId,  pstFrame);
}

int vapi_audio_ctrl(AIO_CTRL_S * pstCtrl)
{
    HI_S32 s32Ret = HI_FAILURE;
    
    switch (pstCtrl->enCmd)
    {
        case ACMD_VI_SET_VOLUME:
            s32Ret = vapi_aio_ai_set_volume(pstCtrl->devID, pstCtrl->value);
            break;
        case ACMD_VO_SET_VOLUME:
            s32Ret = vapi_aio_ao_set_volume(pstCtrl->devID, pstCtrl->value);
            break;
		case ACMD_VO_SET_MUTE:
			s32Ret = vapi_aio_ao_set_mute(pstCtrl->value);
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

int vapi_jpeg_hw_convert(VAPI_JPEG_S *pstJpeg, unsigned char **ppu8OutAddr, unsigned int *pu32OutLen, unsigned int *pu32OutWidth, unsigned int *pu32OutHeight)
{    
    HI_S32 s32Ret = HI_FAILURE;
    
//    if (comm_mutex_timelock(&g_stVpssGrp.mutex, 1000000) != 0)
//        return s32Ret;
    s32Ret = vapi_jpeg_hw(pstJpeg->enType, pstJpeg->u32InWidth, pstJpeg->u32InHeight, 
                pstJpeg->pu8InAddr, pstJpeg->u32InLen, 
                pstJpeg->u32Qfactor, 
                pstJpeg->u32OutWidth, pstJpeg->u32OutHeight, 
                ppu8OutAddr, pu32OutLen, pu32OutWidth, pu32OutHeight,
                &g_stVpssGrp.mutex);
//    comm_mutex_unlock(&g_stVpssGrp.mutex);
    return s32Ret;
}

void vapi_display_mode(DSP_E enDsp)
{
    vapi_vo_dsp_quick(enDsp == DSP_REALTIME);
}

int vapi_get_jpeg_frame(SCREEN_E enScreen, MODE_E enMode, int devID, VAPI_JPEG_S *pstJpeg)
{
    //has something bugs
    return HI_FAILURE;
    
    HI_S32 Grp; 
    HI_S32 voutChn;
    DEV_E  voDev;
    
    if (enMode == DSP_MODE_PBK)
        devID += MAX_LIVE_NUM;
        
    vapi_get_vpss(enScreen, enMode, devID, &Grp);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return HI_FAILURE;

    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
    voDev = g_stVpssGrp.stBind[Grp].voDev;
    
    return vapi_vo_snap_jpeg(voDev, voutChn, pstJpeg);
}

int vapi_get_jpeg_frame2(SCREEN_E enScreen, int VoChn, VAPI_JPEG_S *pstJpeg)
{
    //has something bugs
    return HI_FAILURE;

    HI_S32 Grp; 
    HI_S32 voutChn;
    DEV_E  voDev;

    vapi_get_vpss2(enScreen, VoChn, &Grp);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return HI_FAILURE;
        
    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
    voDev = g_stVpssGrp.stBind[Grp].voDev;
    
    return vapi_vo_snap_jpeg(voDev, voutChn, pstJpeg);
}

void vapi_put_jpeg_frame(VAPI_JPEG_S *pstJpeg)
{
    return;
    if (pstJpeg->pu8InAddr)
    {
        ms_free(pstJpeg->pu8InAddr);
        pstJpeg->pu8InAddr = NULL;
    }
}

int vapi_init(VAPI_ATTR_S *pstAttr)
{
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = vapi_sys_init();
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }
    
    vapi_bind_init();
    
    s32Ret = vapi_vo_init(pstAttr->enRes[SCREEN_MAIN], 
                          pstAttr->enRes[SCREEN_SUB],
                          pstAttr->isHotplug,
                          !pstAttr->isBlock,
                          &pstAttr->stCallback);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }

    vapi_vdec_init(&pstAttr->stCallback);
    vapi_jpeg_init();
    vapi_aio_init();
    vapi_fb_init(SCREEN_MAIN);
//    vapi_fb_init(SCREEN_SUB);
    
    return HI_SUCCESS;
}
 
int vapi_uninit()
{
    vapi_bind_uninit();
    vapi_jpeg_uninit();
    vapi_vdec_uninit();
    vapi_vo_uninit(); 
    vapi_aio_uninit();
    vapi_sys_uninit();
    
    return HI_SUCCESS;
}

