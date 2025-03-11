/* 
 * ***************************************************************
 * Filename:      	vapi.c
 * Created at:    	2015.10.21
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
    VDEC_CHN    vdecChn;
    VPSS_CHN    vpssGrp;
    VPSS_CHN    vpssChn;
    VO_CHN      voutChn;
    VO_LAYER    voutLayer;
}VPSS_BIND_S;

typedef struct vpssGrp_s{
    VPSS_BIND_S     stBind[VAPI_VPSS_MAX_NUM];
    HI_S32          aGroup[HISI_VPSS_MAX_NUM];
    pthread_mutex_t mutex;
}VPSSGRP_S;

static VPSSGRP_S g_stVpssGrp;

static VPSS_GRP vapi_add_vpss_group(void)
{
    HI_S32 i;

    for(i=0; i<HISI_VPSS_MAX_NUM; i++)
    {
        if (g_stVpssGrp.aGroup[i] == STATE_IDLE)
        {
            return i;
        }
    }

    return HI_FAILURE;
}

static void vapi_del_vpss_group(VPSS_GRP VpssGrp)
{
    g_stVpssGrp.aGroup[VpssGrp] = STATE_IDLE;
}

static HI_S32 vapi_bind_vpss(VDEC_CHN VdChn, 
                             VPSS_GRP VpssGrp, VPSS_CHN VpssChn, 
                             HI_U32 MaxW,HI_U32 MaxH,
                             VO_LAYER VoLayer, VO_CHN VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP GrpId = 0;
    
    if(g_stVpssGrp.stBind[VpssGrp].enState == STATE_BUSY)
    {
        return HI_FAILURE;
    }
    
    GrpId = vapi_add_vpss_group();
    if (GrpId == HI_FAILURE)
    {
        VAPILOG("no find a new vpss group !\n");
        return HI_FAILURE;   
    }
    
    s32Ret = vapi_vpss_create(GrpId, VpssChn, MaxW, MaxH);
    if (s32Ret != HI_SUCCESS)
    {
        VAPILOG("vapi_vpss_create [%d] failed with %#x!\n", VpssGrp, s32Ret);
        return HI_FAILURE;
    }
    
    vapi_vdec_bind_vpss(VdChn, GrpId, VpssChn);
    vapi_vo_bind_vpss(VoLayer, VoChn, GrpId, VpssChn);    
    vapi_vdec_start_stream(VdChn);
    
    g_stVpssGrp.stBind[VpssGrp].enState = STATE_BUSY;
    g_stVpssGrp.stBind[VpssGrp].vdecChn = VdChn;
    g_stVpssGrp.stBind[VpssGrp].vpssGrp = GrpId;
    g_stVpssGrp.stBind[VpssGrp].vpssChn = VpssChn;
    g_stVpssGrp.stBind[VpssGrp].voutChn = VoChn;
    g_stVpssGrp.stBind[VpssGrp].voutLayer = VoLayer;
    g_stVpssGrp.aGroup[GrpId] = STATE_BUSY;

    return HI_SUCCESS;
}

static HI_S32 vapi_unbind_vpss(VPSS_GRP VpssGrp)
{
    VDEC_CHN VdChn = g_stVpssGrp.stBind[VpssGrp].vdecChn; 
    VPSS_CHN VpssChn = g_stVpssGrp.stBind[VpssGrp].vpssChn; 
    VO_LAYER VoLayer = g_stVpssGrp.stBind[VpssGrp].voutLayer; 
    VO_CHN VoChn = g_stVpssGrp.stBind[VpssGrp].voutChn;
    VPSS_GRP GrpId = 0;
    
    if(g_stVpssGrp.stBind[VpssGrp].enState != STATE_BUSY)
    {
        return HI_FAILURE;
    }
    GrpId = g_stVpssGrp.stBind[VpssGrp].vpssGrp;
//    vapi_vdec_stop_stream(VdChn);
    vapi_vdec_unbind_vpss(VdChn, GrpId, VpssChn);
    vapi_vo_unbind_vpss(VoLayer, VoChn, GrpId, VpssChn);
    vapi_vpss_destory(GrpId, VpssChn);
    vapi_del_vpss_group(GrpId);
    
    g_stVpssGrp.stBind[VpssGrp].enState = STATE_IDLE;

    return HI_SUCCESS;
}

static HI_BOOL vapi_vdec_is_unbind(VDEC_CHN VdChn)
{
    HI_S32 i;
    for(i=0; i<VAPI_VPSS_MAX_NUM; i++)
    {
        if (g_stVpssGrp.stBind[i].enState == STATE_BUSY && g_stVpssGrp.stBind[i].vdecChn == VdChn)
        {
            return HI_FALSE;
        }
    }
    return HI_TRUE;
}

static void vapi_bind_init()
{
    HI_S32 i;

    for(i=0; i<VAPI_VPSS_MAX_NUM; i++)
    {
        g_stVpssGrp.stBind[i].enState = STATE_IDLE;
        g_stVpssGrp.stBind[i].voutLayer = -1;
        g_stVpssGrp.stBind[i].vpssGrp = -1;
        g_stVpssGrp.stBind[i].vpssChn = -1;
        g_stVpssGrp.stBind[i].vdecChn = -1;
        g_stVpssGrp.stBind[i].voutChn = -1;
    }
    
    for(i=0; i<HISI_VPSS_MAX_NUM; i++)
    {
        g_stVpssGrp.aGroup[i] = STATE_IDLE;
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

static inline void vapi_get_vpss(SCREEN_E enScreen, MODE_E enMode, int devID, 
                                       VPSS_GRP *pGrp, VPSS_CHN *pChn)
{
    if (enMode == DSP_MODE_ZOOMIN)
    {
        *pGrp = enScreen * MAX_CHN_NUM + MAX_CHN_NUM - 1;//ÿ����Ļ���һ��vpss��Ϊ������
    }
    else
    {
        //*pGrp = enScreen * MAX_CHN_NUM + enMode * MAX_LIVE_NUM + devID;
        *pGrp = enScreen * MAX_CHN_NUM + devID;
    }
    *pChn = VAPI_VPSS_CHN_ID;
}

static inline void vapi_get_vpss2(SCREEN_E enScreen, int VoChn, 
                                       VPSS_GRP *pGrp, VPSS_CHN *pChn)
{
    *pGrp = enScreen * MAX_CHN_NUM + VoChn;
    *pChn = VAPI_VPSS_CHN_ID;
}

static HI_S32 vapi_del_one_win(SCREEN_E enScreen, MODE_E enMode, 
                                   int devID, REFRESH_E enRefresh)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    VO_LAYER VoLayer;
    VO_CHN VoChn;
    VDEC_CHN VdChn;

    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    s32Ret = vapi_unbind_vpss(Grp);
    VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;
    VoChn   = g_stVpssGrp.stBind[Grp].voutChn;
    VdChn   = g_stVpssGrp.stBind[Grp].vdecChn;
    if (s32Ret == HI_SUCCESS)
        s32Ret |= vapi_vo_chn_destory(VoLayer, VoChn);
    if(vapi_vdec_is_unbind(VdChn))
    {
        vapi_vdec_stop_stream(VdChn);
        if (enRefresh == REFRESH_CLEAR_VDEC)
        {
            s32Ret |= vapi_vdec_chn_destory(VdChn);
            g_stVpssGrp.stBind[Grp].vdecChn = -1;
        }
    }
    else
    {
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
    }

    return s32Ret;
}

static HI_S32 vapi_del_one_win2(SCREEN_E enScreen, int VoChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    VO_LAYER VoLayer;
    VDEC_CHN VdChn;
    
    comm_mutex_lock(&g_stVpssGrp.mutex);
    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    s32Ret = vapi_unbind_vpss(Grp);
    VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;
    VoChn   = g_stVpssGrp.stBind[Grp].voutChn;
    VdChn   = g_stVpssGrp.stBind[Grp].vdecChn;
    if (s32Ret == HI_SUCCESS)
        vapi_vo_chn_destory(VoLayer, VoChn);
    if(vapi_vdec_is_unbind(VdChn))
    {
        vapi_vdec_stop_stream(VdChn);
        vapi_vdec_chn_destory(VdChn);
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
        s32Ret = -1;
    }
    else
    {
        g_stVpssGrp.stBind[Grp].vdecChn = -1;
        s32Ret = VdChn;
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return s32Ret;
}

static HI_S32 vapi_add_one_win(SCREEN_E enScreen, MODE_E enMode, WIND_S *pstWin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
//    HI_U32  maxWH;
    int devID = pstWin->stDevinfo.id;
    TYPE_E enType = pstWin->stDevinfo.stFormat.enType;
    HI_U32 u32Width = pstWin->stDevinfo.stFormat.width;
    HI_U32 u32Height = pstWin->stDevinfo.stFormat.height;
//    HI_S32 s32Fps = pstWin->stDevinfo.stFormat.fps;
    VO_LAYER VoLayer;
    VO_CHN VoChn = pstWin->stDevinfo.stBind.voutChn;
    VDEC_CHN VdChn = pstWin->stDevinfo.stBind.vdecChn;
    RECT_S stRect;
    RATIO_E enRatio = pstWin->enRatio;
	
	//enRatio = RATIO_VO_AUTO;
	
    devID = pstWin->stDevinfo.id;
    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY)
    {
        // Grp has existed ;
        return HI_SUCCESS;
    }
    //create a display channel
    stRect.s32X         = pstWin->stZone.x;
    stRect.s32Y         = pstWin->stZone.y;
    stRect.u32Width     = pstWin->stZone.w;
    stRect.u32Height    = pstWin->stZone.h;
    if(enMode ==  DSP_MODE_ZOOMIN)
    {
        VoLayer = VAPI_VO_LAYER_VPIP;
    }
    else if(enScreen == SCREEN_SUB)
    {
        VoLayer = VAPI_VO_LAYER_VHD1;        
    }
    else
    {
        VoLayer = VAPI_VO_LAYER_VHD0;
    }
    s32Ret = vapi_vo_chn_create(VoLayer, VoChn, &stRect, enRatio);

    //bind : vdec-> vpss->vout
//    maxWH = MAX(u32Height, u32Width);
    if (s32Ret == HI_SUCCESS)
    {
//        vapi_vo_chn_set_fps(VoLayer, VoChn, s32Fps);
        
        //create a vdec channel
        s32Ret = vapi_vdec_chn_create(devID, VdChn, enType, u32Width, u32Height);
        if (s32Ret == HI_SUCCESS)
        {
            //bind vdec->vpss->vo
            s32Ret = vapi_bind_vpss(VdChn, Grp, Chn, u32Width, u32Height, VoLayer, VoChn);
            if (s32Ret == HI_FAILURE)
            {
                vapi_vo_chn_destory(VoLayer, VoChn);
                vapi_vdec_chn_destory(VdChn);
            }
        }
        else
        {
            vapi_vo_chn_destory(VoLayer, VoChn);
        }
    }

    return s32Ret;
}

static HI_S32 vapi_add_one_win2(SCREEN_E enScreen, WIND_S *pstWin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    int devID = pstWin->stDevinfo.id;
    TYPE_E enType = pstWin->stDevinfo.stFormat.enType;
    HI_U32 u32Width = pstWin->stDevinfo.stFormat.width;
    HI_U32 u32Height = pstWin->stDevinfo.stFormat.height;
    VO_LAYER VoLayer;
    VO_CHN VoChn = pstWin->stDevinfo.stBind.voutChn;
    VDEC_CHN VdChn = pstWin->stDevinfo.stBind.vdecChn;
    RECT_S stRect;
    RATIO_E enRatio = pstWin->enRatio;
	
	
    devID = pstWin->stDevinfo.id;
    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY)
    {
        // Grp has existed ;
        return HI_SUCCESS;
    }
    comm_mutex_lock(&g_stVpssGrp.mutex);
    //create a display channel
    stRect.s32X         = pstWin->stZone.x;
    stRect.s32Y         = pstWin->stZone.y;
    stRect.u32Width     = pstWin->stZone.w;
    stRect.u32Height    = pstWin->stZone.h;
    if(enScreen == SCREEN_SUB)
    {
        VoLayer = VAPI_VO_LAYER_VHD1;        
    }
    else
    {
        VoLayer = VAPI_VO_LAYER_VHD0;
    }
    s32Ret = vapi_vo_chn_create(VoLayer, VoChn, &stRect, enRatio);

    //bind : vdec-> vpss->vout
    if (s32Ret == HI_SUCCESS)
    {
        //create a vdec channel
        s32Ret = vapi_vdec_chn_create(devID, VdChn, enType, u32Width, u32Height);
        if (s32Ret == HI_SUCCESS)
        {
            //bind vdec->vpss->vo
            s32Ret = vapi_bind_vpss(VdChn, Grp, Chn, u32Width, u32Height, VoLayer, VoChn);
            if (s32Ret == HI_FAILURE)
            {
                vapi_vo_chn_destory(VoLayer, VoChn);
                vapi_vdec_chn_destory(VdChn);
            }
        }
        else
        {
            vapi_vo_chn_destory(VoLayer, VoChn);
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
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    VDEC_CHN vdecChn;
    

    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
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
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    VDEC_CHN vdecChn;
    

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
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
    VO_LAYER VoLayer;
    
    if (enScreen == SCREEN_MAIN)
    {
        VoLayer = VAPI_VO_LAYER_VHD0;
    }
    else if (enScreen == SCREEN_SUB)
    {
        VoLayer = VAPI_VO_LAYER_VHD1;
    }
    else 
    {
        VAPILOG("Screen type [%d] invalid \n", enScreen);
        return ;
    }
    
    vapi_vo_chn_clear_buff(VoLayer, VoChn);
    
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
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    
    vapi_switch_screen(pstZoomin->enScreen);
    memset(&stWind, 0, sizeof(WIND_S));
    vapi_get_vpss(enScreen,  pstZoomin->enMode, pstZoomin->devID, &Grp, &Chn);
    
    stWind.stDevinfo.id = pstZoomin->devID;
    stWind.stDevinfo.stFormat = pstZoomin->stFormat;
    stWind.stDevinfo.stBind.vdecChn = pstZoomin->vdecID;
    stWind.stDevinfo.stBind.voutChn = 0;
    stWind.enRatio	= pstZoomin->enRatio;
    // 1. ����: ���������������������̨����
    vapi_clear_screen(enScreen, pstZoomin->enRefresh);

    // 2. ����������ͼ(�󴰿�)
    stWind.stZone = pstZoomin->stZone[WIN_TYPE_NORMAL];
    vapi_add_one_win(enScreen, pstZoomin->enMode, &stWind);
    
    // 3. ����ȫ��Ԥ����ͼ(С����) 
    stWind.stZone = pstZoomin->stZone[WIN_TYPE_ZOOMIN];
    vapi_add_one_win(enScreen, DSP_MODE_ZOOMIN, &stWind);
    
}

static int vapi_start_zoomin(ZOOMIN_S *pstZoomin)
{
    HI_S32 s32Ret = HI_SUCCESS;
    RECT_S stRect;
    SCREEN_E enScreen = pstZoomin->enScreen;
    MODE_E enMode = pstZoomin->enMode;
    int devID = pstZoomin->devID;    
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    HI_S32 W, H;

    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    vapi_get_screen_res(enScreen, &W, &H);
//    printf("=====ratic_w %d, ratio_h %d min_w %d, min h %d\n",W/pstZoomin->stRect.w, H/pstZoomin->stRect.h, W/16, H/16);
//    printf("===o_x %d, o_y %d, o_w %d, o_h %d\n", pstZoomin->stRect.x,pstZoomin->stRect.y,pstZoomin->stRect.w, pstZoomin->stRect.h);
    stRect.u32Width     = VAPI_MAX(pstZoomin->stRect.w, W/15);//pstZoomin->stRect.w;
    stRect.u32Height    = VAPI_MAX(pstZoomin->stRect.h, H/15);//pstZoomin->stRect.h;
    stRect.s32X         = VAPI_MIN(pstZoomin->stFormat.width-stRect.u32Width, pstZoomin->stRect.x);//pstZoomin->stRect.x;
    stRect.s32Y         = VAPI_MIN(pstZoomin->stFormat.height-stRect.u32Height, pstZoomin->stRect.y);//pstZoomin->stRect.y;
//    printf("===m_x %d, m_y %d, m_w %d, m_h %d\n", stRect.s32X,stRect.s32Y,stRect.u32Width, stRect.u32Height);
    s32Ret = vapi_vpss_set_crop(g_stVpssGrp.stBind[Grp].vpssGrp, &stRect);

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
    
    comm_mutex_lock(&g_stVpssGrp.mutex);
    //�رյ�ǰ��Ļ�ĵ��ӷŴ��
    if(vapi_vo_get_pipbind() == pstScreenRes->enScreen)
    {
        vapi_vo_stop_pip_layer();
    }
    
    s32Ret = vapi_vo_dev_close(pstScreenRes->enScreen);    
    if (s32Ret != HI_SUCCESS)
    {
        comm_mutex_unlock(&g_stVpssGrp.mutex);
        return HI_FAILURE;
    }
    
    s32Ret = vapi_vo_dev_open(pstScreenRes->enScreen, 
                              pstScreenRes->enRes);
    if (s32Ret != HI_SUCCESS)
    {
        comm_mutex_unlock(&g_stVpssGrp.mutex);
        return HI_FAILURE;
    }

    //���õ�ǰ��Ļ���ӷŴ��
    if(vapi_vo_get_pipbind() == pstScreenRes->enScreen)
    {
        vapi_vo_start_pip_layer(pstScreenRes->enRes);
    }

//    vapi_fb_init(pstScreenRes->enScreen);

    //hjh 20160212 add, for QT switch output res.
    s32Ret = vapi_switch_display(pstScreenRes->enScreen);
    if(s32Ret != HI_SUCCESS)
    {
        comm_mutex_unlock(&g_stVpssGrp.mutex);
        return HI_FAILURE;
    }
    comm_mutex_unlock(&g_stVpssGrp.mutex);

    return HI_SUCCESS;
}

int vapi_switch_screen(SCREEN_E enScreen)
{
    HI_S32 s32Ret = HI_SUCCESS;

    s32Ret = vapi_vo_pip_rebind(enScreen);
    
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
    RECT_S stRect;
    SCREEN_E enScreen = pstZoomin->enScreen;
    int VoChn = pstZoomin->VoChn;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    HI_S32 W, H;

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    vapi_get_screen_res(enScreen, &W, &H);
    stRect.u32Width     = VAPI_MAX(pstZoomin->stRect.w, W/15);//pstZoomin->stRect.w;
    stRect.u32Height    = VAPI_MAX(pstZoomin->stRect.h, H/15);//pstZoomin->stRect.h;
    stRect.s32X         = pstZoomin->stRect.x;
    stRect.s32Y         = pstZoomin->stRect.y;
    s32Ret = vapi_vpss_set_crop(g_stVpssGrp.stBind[Grp].vpssGrp, &stRect);

    return s32Ret;

}

int vapi_set_full_screen(FULL_S *pstFull)
{
    HI_S32 s32Ret = HI_SUCCESS;
    WIND_S  stWind;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    
    memset(&stWind, 0, sizeof(WIND_S));
    // 1.clear screen 
    vapi_clear_screen(pstFull->enScreen, pstFull->enRefresh);

    // 2.add a full view
    vapi_get_vpss(pstFull->enScreen,  pstFull->enMode, pstFull->devID, &Grp, &Chn);
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
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    VDEC_CHN voutChn;
    VO_LAYER VoLayer;

    vapi_get_vpss(pstRatio->enScreen, pstRatio->enMode, pstRatio->devId, &Grp, &Chn);
    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
	VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;

	if(VoLayer<0 || voutChn<0){
		VAPILOG("dev[%d] not use any vdec chn \n", pstRatio->devId);
		return -1;
	}
    s32Ret = vapi_vo_check_state(VoLayer, voutChn, STATE_BUSY);
    if (s32Ret == HI_FAILURE)
    {
        VAPILOG("dev[%d] not use any vdec chn \n", pstRatio->devId);
        return -1;
    }  

	s32Ret = vapi_vo_set_ratio(VoLayer, voutChn, pstRatio->enRatio);
	
	return s32Ret;
}

int vapi_set_vout_ratio2(RATIO2_S *pstRatio)
{
    HI_BOOL s32Ret;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    VDEC_CHN voutChn;
    VO_LAYER VoLayer;

    vapi_get_vpss2(pstRatio->enScreen, pstRatio->vouid, &Grp, &Chn);
    voutChn = g_stVpssGrp.stBind[Grp].voutChn;
	VoLayer = g_stVpssGrp.stBind[Grp].voutLayer;

	if(VoLayer<0 || voutChn<0){
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
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 i;
    VPSS_GRP Grp; 
    VPSS_CHN Chn;
    VO_LAYER VoLayer;
    VO_CHN VoChn;
    
    for (i=0; i<SCREEN_NUM; i++)
    {
        vapi_get_vpss(i, DSP_MODE_LIVE, devID, &Grp, &Chn);

        if (g_stVpssGrp.stBind[Grp].enState == STATE_BUSY)
        {
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
    HI_S32 s32Ret;
    VENC_CHN VeChn;
    
    s32Ret = vapi_vdec_chn_create(VENC_IDX_BASE, VdChn, pIf->enType, 
                                  pIf->width, pIf->height);
    if (s32Ret == HI_FAILURE)
    {
        VAPILOG("vdec chn[%d] format[%d][%dx%d]create faild with %#x!", 
                    VdChn, pIf->enType, pIf->width, pIf->height, s32Ret);
        return -1;
    }
        
    VeChn = vapi_venc_chn_create(pOf->enType, pOf->width, pOf->height,
                                pOf->enRc, pOf->bitRate,
                                pIf->fps, pOf->fps,
                                pstPrivate);
    if (VeChn < 0)
    {
        VAPILOG("vdec chn[%d] format[%d][%dx%d][%d->%d]create faild! with %#x", 
                    VdChn, pOf->enType, pOf->width, pOf->height, 
                    pIf->fps, pOf->fps, VeChn);
        vapi_vdec_chn_destory(VdChn);
        return -1;
    }
    
    s32Ret = vapi_venc_bind_vdec(VdChn, VeChn);
    if (s32Ret == HI_FAILURE)
    {
        VAPILOG("vdec chn[%d] format[%d][%dx%d]bind faild with %#x!", 
                    VdChn, pIf->enType, pIf->width, pIf->height, s32Ret);
        vapi_vdec_chn_destory(VdChn);
        vapi_venc_chn_destory(VeChn);
        return -1;
    }
    
    return VeChn;
}

void vapi_transfer_destory(int VeChn)
{
    VDEC_CHN VdChn;
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

int vapi_get_jpeg_frame(SCREEN_E enScreen, MODE_E enMode, int devID, VAPI_JPEG_S *pstJpeg)
{
    VPSS_GRP Grp; 
    VPSS_CHN Chn;

    if (enMode == DSP_MODE_PBK)
        devID += MAX_LIVE_NUM;
        
    vapi_get_vpss(enScreen, enMode, devID, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return HI_FAILURE;
        
    return vapi_vpss_snap_jpeg(g_stVpssGrp.stBind[Grp].vpssGrp, pstJpeg);
}

int vapi_get_jpeg_frame2(SCREEN_E enScreen, int VoChn, VAPI_JPEG_S *pstJpeg)
{
    VPSS_GRP Grp; 
    VPSS_CHN Chn;

    vapi_get_vpss2(enScreen, VoChn, &Grp, &Chn);
    if (g_stVpssGrp.stBind[Grp].enState == STATE_IDLE)
        return HI_FAILURE;
        
    return vapi_vpss_snap_jpeg(g_stVpssGrp.stBind[Grp].vpssGrp, pstJpeg);
}

void vapi_put_jpeg_frame(VAPI_JPEG_S *pstJpeg)
{
    if (pstJpeg->pu8InAddr)
    {
        ms_free(pstJpeg->pu8InAddr);
        pstJpeg->pu8InAddr = NULL;
    }
}

int vapi_jpeg_hw_convert(VAPI_JPEG_S *pstJpeg, unsigned char **ppu8OutAddr, unsigned int *pu32OutLen, unsigned int *pu32OutWidth, unsigned int *pu32OutHeight)
{
    HI_S32 s32Ret = HI_FAILURE;
    
    s32Ret = vapi_jpeg_hw(pstJpeg->enType, pstJpeg->u32InWidth, pstJpeg->u32InHeight, 
                pstJpeg->pu8InAddr, pstJpeg->u32InLen, 
                pstJpeg->u32Qfactor, 
                pstJpeg->u32OutWidth, pstJpeg->u32OutHeight, 
                ppu8OutAddr, pu32OutLen,pu32OutWidth, pu32OutHeight,
                &g_stVpssGrp.mutex);

    return s32Ret;
}

void vapi_display_mode(DSP_E enDsp)
{
    vapi_vdec_dsp_block(enDsp == DSP_FLUENT);
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
                          pstAttr->isDualScreen,
                          &pstAttr->stCallback);
    if (s32Ret != HI_SUCCESS)
    {
        return HI_FAILURE;
    }

    vapi_vdec_init(&pstAttr->stCallback, pstAttr->isBlock);
//    vapi_venc_init(&pstAttr->stCallback);
    vapi_jpeg_init();
    vapi_aio_init();
    vapi_fb_init(SCREEN_MAIN);
    if (pstAttr->isDualScreen)
        vapi_fb_init(SCREEN_SUB);
    VAPILOG("vapi init done");
    return HI_SUCCESS;
}
 
int vapi_uninit()
{
    vapi_bind_uninit();
//    vapi_venc_uninit();
    vapi_jpeg_uninit();
    vapi_vdec_uninit();
    vapi_vo_uninit();    
    vapi_aio_uninit();
    vapi_sys_uninit();
    
    return HI_SUCCESS;
}

