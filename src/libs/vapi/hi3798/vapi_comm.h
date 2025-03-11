/* 
 * ***************************************************************
 * Filename:      	vapi_comm.h
 * Created at:    	2015.10.21
 * Description:   	common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __VAPI_COMM_H__
#define __VAPI_COMM_H__

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "hi_unf_mce.h"
#include "hi_unf_venc.h"
#include "hi_unf_vi.h"
#include "msstd.h"
#include "vapi.h"
#include "hifb.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RED 			"\033[0;31m"
#define GREEN 			"\033[0;32m"
#define CLRCOLOR		"\033[0;39m "

typedef enum debug_s{
    DEBUG_VDEC = (1 << 0),
    DEBUG_VPSS = (1 << 1),
    DEBUG_VOUT = (1 << 2),
    DEBUG_ALL = DEBUG_VDEC | DEBUG_VPSS | DEBUG_VOUT,
}DEBUG_S;

#define VAPILOG(format, ...) \
    do{\
		printf(RED format CLRCOLOR "@func:%s file:%s line:%d\n", \
		##__VA_ARGS__, __func__, __FILE__, __LINE__); \
    }while(0);
    
            
#define ALIGN_UP(x, a)              ((x+a-1)&(~(a-1)))
#define ALIGN_BACK(x, a)            ((a) * (((x) / (a))))
#define VAPI_MAX(a,b)                    (((a) > (b)) ? (a) : (b))
#define VAPI_MIN(a,b)                    (((a) < (b)) ? (a) : (b))

#define VAPI_VPSS_MAX_NUM       (MAX_CHN_NUM * SCREEN_NUM)
#define VAPI_JPEG_VDEC_ID        0
#define VAPI_JPEG_VENC_ID        VAPI_JPEG_VDEC_ID

#define VAPI_VENC_MAX_WIDTH     (1920)
#define VAPI_VENC_MAX_HEIGTH     (1080)

#define VAPI_VENC_MIN_WIDTH     (480)
#define VAPI_VENC_MIN_HEIGTH    (320)

#define VAPI_VDEC_MAX_WIDTH     (3840)
#define VAPI_VDEC_MAX_HEIGTH     (3840)

 typedef enum dev_e{
     VAPI_VO_DEV_DHD = 0,
     VAPI_VO_DEV_DSD,
     VAPI_VO_DEV_NUM,
 }DEV_E;

 typedef struct res_s{
     HI_UNF_VCODEC_CAP_LEVEL_E enCapLevel;
     HI_U32 u32Width;
     HI_U32 u32Height;
 }RES_S;

 //sys
HI_S32 vapi_sys_init();

HI_VOID vapi_sys_uninit();

//vdec
HI_VOID vapi_vdec_init(VDEC_CB_S *pstCb);

HI_VOID vapi_vdec_uninit();

HI_S32 vapi_vdec_chn_create(HI_S32 DevId, HI_S32 VdChn, TYPE_E enType, MODE_E enMode, HI_U32 u32Width, HI_U32 u32Height);

HI_S32 vapi_vdec_chn_destory(HI_S32 VdChn);

HI_VOID vapi_vdec_update_state(HI_S32 VdChn, HI_S32 state);

HI_BOOL vapi_vdec_check_state(HI_S32 VdChn, HI_S32 state);

HI_S32 vapi_vdec_send_frame(HI_S32 VdChn, HI_U8* pu8Addr, HI_U32 u32Len, HI_U64 u64PTS);

HI_S32 vapi_vdec_start_stream(HI_S32 VdChn);

HI_S32 vapi_vdec_stop_stream(HI_S32 VdChn);

HI_HANDLE vapi_vdec_get_chn_handle(HI_S32 VdChn);

HI_S32 vapi_vdec_chn_update(HI_S32 DevId, HI_S32 VdChn, STATE_E enState);

//jpeg
HI_S32 vapi_jpeg_hw(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight, HI_U8 *pu8InAddr, HI_U32 u32InLen, HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight, HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen, HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight, pthread_mutex_t *mutex);

HI_S32 vapi_yuv2jpeg(HI_UNF_VIDEO_FRAME_INFO_S * pstFrame, HI_U8 **ppData, HI_U32 *pSize, HI_U32 *pWidth, HI_U32 *pHeight);

HI_VOID vapi_jpeg_init();

HI_VOID vapi_jpeg_uninit();


//vout
HI_S32 vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, HI_S32 isHotplug, HI_BOOL isQuick, VDEC_CB_S *pstCb);

HI_VOID vapi_vo_uninit();

HI_S32 vapi_vo_dev_open(DEV_E VoDev, SCREEN_RES_E enRes);

HI_S32 vapi_vo_dev_close(DEV_E VoDev);

HI_S32 vapi_vo_chn_create(DEV_E VoDev, HI_S32 VoChn, HI_RECT_S *pstRect, RATIO_E enRatio);

HI_S32 vapi_vo_chn_destory(DEV_E VoDev, HI_S32 VoChn);

HI_S32  vapi_vo_chn_clear_buff(DEV_E VoDev, HI_S32 VoChn);

HI_VOID vapi_vo_dsp_quick(HI_BOOL bQuick);

HI_S32 vapi_vo_bind_vdec(DEV_E VoDev, HI_S32 VoChn, HI_S32 VdecChn);

HI_S32 vapi_vo_unbind_vdec(DEV_E VoDev, HI_S32 VoChn, HI_S32 VdecChn);

HI_VOID vapi_vo_get_srceen_res(SCREEN_E enScreen, HI_S32 *W, HI_S32 *H);

HI_S32 vapi_vo_set_csc(DEV_E VoDev, CSC_E enCSC);

HI_VOID vapi_vo_update_state(DEV_E VoDev, HI_S32 VoChn, HI_S32 state);

HI_BOOL vapi_vo_check_state(DEV_E VoDev, HI_S32 VoChn, HI_S32 state);

HI_S32	vapi_vo_set_ratio(DEV_E VoDev, HI_S32 VoChn, RATIO_E enRatio);

HI_S32 vapi_vo_set_crop(DEV_E VoDev, HI_S32 VoChn, HI_RECT_S *pstRect);

HI_S32 vapi_vo_snap_jpeg(DEV_E VoDev, HI_S32 VoChn, VAPI_JPEG_S *pstJpeg);

//audio
HI_S32 vapi_aio_init();

HI_S32 vapi_aio_uninit();

HI_BOOL vapi_aio_is_support_talk();

HI_S32 vapi_aio_ai_read_frame(HI_S32 AiDevId, HI_UNF_SAMPLE_RATE_E enOutSampleRate, AENC_E enAenc, AIO_FRAME_S *pstFrame);

HI_S32 vapi_aio_ao_write_frame(HI_S32 AoDevId, AIO_FRAME_S *pstFrame);

HI_S32 vapi_aio_ai_set_volume(HI_S32 AiDevId, HI_U8 value);

HI_S32 vapi_aio_ao_set_volume(HI_S32 AoDevId, HI_U8 value);

HI_S32 vapi_aio_ao_set_mute(HI_S32 mute);


//hifb
HI_S32 vapi_fb_init(SCREEN_E enScreen);

HI_S32 vapi_switch_display(SCREEN_E enScreen);

//HI_S32 vapi_fb_clear_buffer(SCREEN_E enScreen);

HI_S32 vapi_fb_uninit();

//utilty
HI_S32 comm_mutex_init(pthread_mutex_t * mutex);

void comm_mutex_uninit(pthread_mutex_t *mutex);

int comm_mutex_trylock(pthread_mutex_t *mutex);

int comm_mutex_timelock(pthread_mutex_t *mutex, int us);

void comm_mutex_lock(pthread_mutex_t *mutex);

void comm_mutex_unlock(pthread_mutex_t * mutex);

int comm_task_set_name(const char* name);

int comm_task_create(pthread_t *handle, void *(*func)(void *), void *arg);

int comm_task_create_join(pthread_t *handle, void *(*func)(void *), void *arg);

int comm_task_join(pthread_t *handle);

int comm_get_vcodec_cap_level(HI_U32 u32Width, HI_U32 u32Height, HI_U32 *pu32Buffsize);

#ifdef __cplusplus
}
#endif


#endif


