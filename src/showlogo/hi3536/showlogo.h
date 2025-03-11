/* 
 * ***************************************************************
 * Filename:      	showlogo.h
 * Created at:    	2015.10.21
 * Description:   	showlogo api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __SHOWLOGO_H__
#define __SHOWLOGO_H__

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vo.h"
#include "mpi_hdmi.h"
#include "vapi.h"

#ifdef __cplusplus
extern "C" {
#endif


#define LOGO_ALIGN_UP(x, a)              ((x+a-1)&(~(a-1)))
#define LOGO_ALIGN_BACK(x, a)            ((a) * (((x) / (a))))
#define LOGO_MAX(a,b)                    (((a) > (b)) ? (a) : (b))
#define LOGO_MIN(a,b)                    (((a) < (b)) ? (a) : (b))

enum {
    LOGO_VO_DEV_DHD0 = 0,
    LOGO_VO_DEV_DHD1,
    LOGO_VO_DEV_DSD0,
    LOGO_VO_DEV_NUM,
};

enum {
    LOGO_VO_LAYER_VHD0 = 0,
    LOGO_VO_LAYER_VHD1,
    LOGO_VO_LAYER_NUM,
};

#define DIS_CHN_ID 0


HI_S32 logo_vpss_create(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, HI_U32 u32MaxWH);

HI_S32 logo_vpss_destory(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_S32 logo_vdec_chn_create(VDEC_CHN VdChn, TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height);

HI_S32 logo_vdec_chn_destory(VDEC_CHN VdChn);

HI_S32 logo_vdec_bind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_S32 logo_vdec_unbind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_S32 logo_vdec_send_frame(VDEC_CHN VdChn, HI_U8* pu8Addr, HI_U32 u32Len, HI_U64 u64PTS);

HI_S32 logo_vo_dev_open(VO_DEV VoDev, SCREEN_RES_E enRes);

HI_S32 logo_vo_dev_close(VO_DEV VoDev);

HI_S32 logo_vo_bind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_S32 logo_vo_unbind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);


#ifdef __cplusplus
}
#endif


#endif

