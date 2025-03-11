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
#include <sys/epoll.h>

#include "mpi_sys.h"
#include "mpi_vgs.h"
#include "mpi_vb.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_venc.h"
#include "mpi_vo.h"
#include "mpi_hdmi.h"
#include "mpi_ao.h"
#include "mpi_ai.h"
#include "acodec.h"
#include "vapi.h"
#include "hifb.h"
#include "msstd.h"

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

#define VAPI_VPSS_CHN_ID         0
#define VAPI_VPSS_MAX_NUM       (MAX_CHN_NUM * SCREEN_NUM)
#define VAPI_VPSS_LAST          (VAPI_VPSS_MAX_NUM - 1)

#define HISI_VPSS_MAX_NUM       (256)
#define VAPI_JPEG_VDEC_ID        MAX_LIVE_NUM
#define VAPI_JPEG_VENC_ID        MAX_VENC_NUM
#define VAPI_SNAP_VENC_ID        (MAX_VENC_NUM + 1)

#define VAPI_VENC_MAX_WIDTH     (1920)
#define VAPI_VENC_MAX_HEIGTH     (1080)

#define VAPI_VDEC_MAX_WIDTH     (4000)
#define VAPI_VDEC_MAX_HEIGTH     (4000)

/* audio codec */
typedef struct 
{
    unsigned int sample;		//0:48k, 1:32k, 2:24k, 3:16k, 4:12k, 5:8k
    unsigned int data_length;	//0:16bit 1:20bit 2:24bit 3:32bit
    unsigned int ctrl_mode;		//0:Slaver 1:Master
    unsigned int trans_mode;	//0: I2S   1:PCM
    unsigned int mono_mute;
}Nau88c10_Ctrl;

#define SOFT_RESET				0x00
#define SET_CTRL_MODE 			0x01
#define SET_TRANSFER_MODE		0x02
#define SET_DAC_SAMPLE			0x03
#define SET_ADC_SAMPLE			0x04
#define SET_DATA_LENGTH			0x05
#define DAC_VOLUME_CTRL			0x06
#define MONO_OUTPUT_MUTE		0x07

typedef enum dev_e{

    VAPI_VO_DEV_DHD0 = 0,
    VAPI_VO_DEV_DHD1,
    VAPI_VO_DEV_DSD0,
    VAPI_VO_DEV_NUM,
}DEV_E;

typedef enum layver_e{
    VAPI_VO_LAYER_VHD0 = 0,
    VAPI_VO_LAYER_VHD1,
    VAPI_VO_LAYER_VPIP,
//    VAPI_VO_LAYER_VSD0,
//    VAPI_VO_LAYER_VIRT0,
    VAPI_VO_LAYER_NUM,
}LAYER_E;

 //sys
HI_S32 vapi_sys_init();

HI_VOID vapi_sys_uninit();

//vpss
HI_S32 vapi_vpss_create(VPSS_GRP VpssGrp,VPSS_CHN VpssChn, HI_U32 u32MaxW, HI_U32 u32MaxH);

HI_S32 vapi_vpss_destory(VPSS_GRP VpssGrp,VPSS_CHN VpssChn);

HI_S32 vapi_vpss_set_crop(VPSS_GRP VpssGrp, RECT_S *pstRect);

HI_S32 vapi_vpss_snap_jpeg(VPSS_GRP VpssGrp, VAPI_JPEG_S *pstJpeg);

//vdec
HI_VOID vapi_vdec_init(VDEC_CB_S *pstCb, int isBlock);

HI_VOID vapi_vdec_uninit();

HI_S32 vapi_vdec_chn_create(int DevId, VDEC_CHN VdChn, TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height);

HI_S32 vapi_vdec_chn_destory(VDEC_CHN VdChn);

HI_S32 vapi_vdec_bind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_S32 vapi_vdec_unbind_vpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_VOID vapi_vdec_update_state(VDEC_CHN VdChn, HI_S32 state);

HI_BOOL vapi_vdec_check_state(VDEC_CHN VdChn, HI_S32 state);

HI_VOID vapi_vdec_dsp_block(HI_BOOL isBlock);

HI_S32 vapi_vdec_send_frame(VDEC_CHN VdChn, HI_U8* pu8Addr, HI_U32 u32Len, HI_U64 u64PTS);

HI_S32 vapi_vdec_start_stream(VDEC_CHN VdChn);

HI_S32 vapi_vdec_stop_stream(VDEC_CHN VdChn);

//venc
HI_S32 vapi_venc_chn_create(TYPE_E enType, HI_U32 u32Width, HI_U32 u32Height, RC_E enRc, HI_U32 u32BitRate, HI_U32 u32SrcFrmRate, HI_U32 u32DstFrmRate, void *pstPrivate);
    
HI_S32 vapi_venc_chn_destory(VENC_CHN VeChn);

HI_S32 vapi_venc_chn_config(VENC_CHN VeChn, HI_U32 u32DstFrmRate, HI_U32 u32BitRate);

HI_BOOL vapi_venc_chn_is_remain();

HI_S32 vapi_venc_bind_vdec(VDEC_CHN VdChn, VENC_CHN VeChn);
    
HI_S32 vapi_venc_unbind_vdec(VENC_CHN VeChn);
   
HI_VOID vapi_venc_init(VDEC_CB_S *pstCb);
    
HI_VOID vapi_venc_uninit();

//jpeg
HI_S32 vapi_jpeg_hw(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight, HI_U8 *pu8InAddr, HI_U32 u32InLen, HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight, HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen, HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight, pthread_mutex_t *mutex);

HI_S32 vapi_yuv2jpeg(VIDEO_FRAME_INFO_S * pstFrame, HI_U8 **ppData, HI_U32 *pSize, HI_U32 *pWidth, HI_U32 *pHeight);

HI_VOID vapi_jpeg_init();

HI_VOID vapi_jpeg_uninit();


//vout
HI_S32 vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, int isHotplug, int isDualScreen, VDEC_CB_S *pstCb);

HI_VOID vapi_vo_uninit();

HI_S32 vapi_vo_dev_open(VO_DEV VoDev, SCREEN_RES_E enIntfSync);

HI_S32 vapi_vo_dev_close(VO_DEV VoDev);

HI_S32 vapi_vo_chn_create(VO_LAYER VoLayer, VO_CHN VoChn, RECT_S *pstRect, RATIO_E enRatio);

HI_S32 vapi_vo_chn_destory(VO_LAYER VoLayer, VO_CHN VoChn);

HI_S32 vapi_vo_chn_set_fps(VO_LAYER VoLayer, VO_CHN VoChn, HI_S32 s32Fps);

HI_S32  vapi_vo_chn_clear_buff(VO_LAYER VoLayer, VO_CHN VoChn);

HI_VOID vapi_vo_chn_clear_all_buff(VO_LAYER VoLayer);

HI_S32 vapi_vo_bind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_S32 vapi_vo_unbind_vpss(VO_LAYER VoLayer, VO_CHN VoChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn);

HI_S32 vapi_vo_start_pip_layer(SCREEN_RES_E enRes);

HI_S32 vapi_vo_stop_pip_layer();

HI_S32 vapi_vo_pip_rebind(VO_DEV VoDev);

HI_S32 vapi_vo_get_pipbind();

HI_VOID vapi_vo_get_srceen_res(SCREEN_E enScreen, HI_S32 *W, HI_S32 *H);

HI_S32 vapi_vo_set_csc(VO_LAYER VoLayer, CSC_E enCSC);

HI_VOID vapi_vo_update_state(VO_LAYER VoLayer, VO_CHN VoChn, HI_S32 state);

HI_BOOL vapi_vo_check_state(VO_LAYER VoLayer, VO_CHN VoChn, HI_S32 state);

HI_S32	vapi_vo_set_ratio(VO_LAYER VoLayer, VO_CHN VoChn, RATIO_E enRatio);

//audio
HI_S32 vapi_aio_init();

HI_S32 vapi_aio_uninit();

HI_BOOL vapi_aio_is_support_talk();

HI_S32 vapi_aio_ai_read_frame(AUDIO_DEV AiDevId, AUDIO_SAMPLE_RATE_E enOutSampleRate, AENC_E enAenc, AIO_FRAME_S *pstFrame);

HI_S32 vapi_aio_ao_write_frame(AUDIO_DEV AoDevId, AIO_FRAME_S *pstFrame);

HI_S32 vapi_aio_ai_set_volume(AUDIO_DEV AiDevId, HI_U8 value);

HI_S32 vapi_aio_ao_set_volume(AUDIO_DEV AoDevId,HI_U8 value);

HI_S32 vapi_aio_ao_set_mute(HI_S32 devId, HI_S32 mute);

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

int comm_epoll_create(int epmax);

int comm_epoll_destroy(int epfd);

void comm_epoll_add(int epfd, int client, int event);

void comm_epoll_del(int epfd, int client, int event);

int comm_epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);


#ifdef __cplusplus
}
#endif


#endif

