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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "gfbg.h"
#include "msstd.h"
#include "ot_acodec.h"
#include "ot_buffer.h"
#include "ss_mpi_audio.h"
#include "ss_mpi_hdmi.h"
#include "ss_mpi_ive.h"
#include "ss_mpi_sys.h"
#include "ss_mpi_vb.h"
#include "ss_mpi_vdec.h"
#include "ss_mpi_venc.h"
#include "ss_mpi_vgs.h"
#include "ss_mpi_vo.h"
#include "ss_mpi_vpss.h"
#include "vapi.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define RED      "\033[0;31m"
#define GREEN    "\033[0;32m"
#define CLRCOLOR "\033[0;39m "

typedef enum debug_s
{
    DEBUG_VDEC = (1 << 0),
    DEBUG_VPSS = (1 << 1),
    DEBUG_VOUT = (1 << 2),
    DEBUG_ALL  = DEBUG_VDEC | DEBUG_VPSS | DEBUG_VOUT,
} DEBUG_S;

#define VAPILOG(format, ...)                                                                                                                                                       \
    do {                                                                                                                                                                           \
        printf(RED "[VAPI]" format CLRCOLOR "@func:%s file:%s line:%d\n", ##__VA_ARGS__, __func__, __FILE__, __LINE__);                                                                     \
    } while (0);

#define VAPIINFO(format, ...)                                                                                                                                                       \
    do {                                                                                                                                                                           \
        printf(GREEN "[VAPI]" format CLRCOLOR "@func:%s file:%s line:%d\n", ##__VA_ARGS__, __func__, __FILE__, __LINE__);                                                                     \
    } while (0);

#define ALIGN_UP(x, a)   ((x + a - 1) & (~(a - 1)))
#define ALIGN_BACK(x, a) ((a) * (((x) / (a))))
#define VAPI_MAX(a, b)   (((a) > (b)) ? (a) : (b))
#define VAPI_MIN(a, b)   (((a) < (b)) ? (a) : (b))

#define VAPI_VPSS_CHN_ID  0
#define VAPI_VPSS_MAX_NUM (MAX_CHN_NUM * SCREEN_NUM)
#define VAPI_VPSS_LAST    (VAPI_VPSS_MAX_NUM - 1)

#define HISI_VPSS_MAX_NUM (256)
#define VAPI_JPEG_VDEC_ID MAX_LIVE_NUM
#define VAPI_JPEG_VENC_ID MAX_VENC_NUM
#define VAPI_SNAP_VENC_ID (MAX_VENC_NUM + 1)

#define VAPI_VENC_MAX_WIDTH  (1920)
#define VAPI_VENC_MAX_HEIGTH (1080)

#define VAPI_VDEC_MAX_WIDTH  (8000)
#define VAPI_VDEC_MAX_HEIGTH (8000)

#define VAPI_FISH_MAX_WIDTH  (1920)
#define VAPI_FISH_MAX_HEIGTH (VAPI_FISH_MAX_WIDTH)

typedef enum dev_e
{
    VAPI_VO_DEV_DHD0 = 0,
    VAPI_VO_DEV_DHD1,
    VAPI_VO_DEV_DSD0,
    VAPI_VO_DEV_NUM,
} DEV_E;

typedef enum layver_e
{
    VAPI_VO_LAYER_VHD0 = 0,
    VAPI_VO_LAYER_VHD1,
    VAPI_VO_LAYER_VPIP,
    //    VAPI_VO_LAYER_VSD0,
    //    VAPI_VO_LAYER_VIRT0,
    VAPI_VO_LAYER_NUM,
} LAYER_E;

// sys
td_s32 vapi_sys_init();

td_void vapi_sys_uninit();

// vpss
td_s32 vapi_vpss_create(ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn, FISH_E enFish, td_u32 u32MaxW, td_u32 u32MaxH);

td_s32 vapi_vpss_destory(ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn);

td_s32 vapi_vpss_set_crop(ot_vpss_grp VpssGrp, ot_rect *pstRect);

td_s32 vapi_vpss_chn_get_wh(ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn, td_u32 *pu32Width, td_u32 *pu32Height);

td_s32 vapi_vpss_snap_yuv(ot_vpss_grp VpssGrp, VAPI_YUV_S *pstYUV, ot_video_frame_info *pstFrame);

void vapi_vpss_snap_free(VAPI_YUV_S *pstYUV, ot_video_frame_info *pstFrame);

td_s32 vapi_vpss_snap_jpeg(ot_vpss_grp VpssGrp, VAPI_JPEG_S *pstJpeg);

// vdec
td_void vapi_vdec_init(VDEC_CB_S *pstCb, int isBlock);

td_void vapi_vdec_uninit();

td_s32 vapi_vdec_chn_create(int DevId, ot_vdec_chn VdChn, TYPE_E enType, td_u32 u32Width, td_u32 u32Height);

td_s32 vapi_vdec_chn_destory(ot_vdec_chn VdChn);

td_s32 vapi_vdec_bind_vpss(ot_vdec_chn VdChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn);

td_s32 vapi_vdec_unbind_vpss(ot_vdec_chn VdChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn);

td_void vapi_vdec_update_state(ot_vdec_chn VdChn, td_s32 state);

td_bool vapi_vdec_check_state(ot_vdec_chn VdChn, td_s32 state);

td_void vapi_vdec_dsp_block(td_bool isBlock);

td_s32 vapi_vdec_send_frame(ot_vdec_chn VdChn, td_u8 *pu8Addr, td_u32 u32Len, td_u64 u64PTS);

td_s32 vapi_vdec_start_stream(ot_vdec_chn VdChn);

td_s32 vapi_vdec_stop_stream(ot_vdec_chn VdChn);

// venc
td_s32 vapi_venc_chn_create(TYPE_E enType, td_u32 u32Width, td_u32 u32Height, RC_E enRc, td_u32 u32BitRate, td_u32 u32SrcFrmRate, td_u32 u32DstFrmRate, void *pstPrivate);

td_s32 vapi_venc_chn_destory(ot_venc_chn VeChn);

td_s32 vapi_venc_chn_config(ot_venc_chn VeChn, td_u32 u32DstFrmRate, td_u32 u32BitRate);

td_bool vapi_venc_chn_is_remain();

td_s32 vapi_venc_bind_vdec(ot_vdec_chn VdChn, ot_venc_chn VeChn);

td_s32 vapi_venc_unbind_vdec(ot_venc_chn VeChn);

td_void vapi_venc_init(VDEC_CB_S *pstCb);

td_void vapi_venc_uninit();

// jpeg
td_s32 vapi_jpeg_hw(TYPE_E enType, td_u32 u32InWidth, td_u32 u32InHeight, td_u8 *pu8InAddr, td_u32 u32InLen, td_u32 u32Qfactor, td_u32 u32OutWidth, td_u32 u32OutHeight,
                    td_u8 **ppu8OutAddr, td_u32 *pu32OutLen, td_u32 *pu32OutWidth, td_u32 *pu32OutHeight, pthread_mutex_t *mutex);

td_s32  vapi_yuv2jpeg(ot_video_frame_info *pstFrame, td_u8 **ppData, td_u32 *pSize, td_u32 *pWidth, td_u32 *pHeight);
td_void vapi_jpeg_init();

td_void vapi_jpeg_uninit();

// vout
td_s32 vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, int isHotplug, int isDualScreen, int isogeny, char prefix, VDEC_CB_S *pstCb);

td_void vapi_vo_uninit();

td_s32 vapi_vo_dev_open(ot_vo_dev VoDev, SCREEN_RES_E enIntfSync);

td_s32 vapi_vo_dev_close(ot_vo_dev VoDev);

td_s32 vapi_vo_chn_create(ot_vo_layer VoLayer, ot_vo_chn VoChn, ot_rect *pstRect, RATIO_E enRatio);

td_s32 vapi_vo_chn_destory(ot_vo_layer VoLayer, ot_vo_chn VoChn);

td_s32 vapi_vo_chn_set_fps(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_s32 s32Fps);

td_s32 vapi_vo_chn_clear_buff(ot_vo_layer VoLayer, ot_vo_chn VoChn);

td_void vapi_vo_chn_clear_all_buff(ot_vo_layer VoLayer);

td_s32 vapi_vo_bind_vpss(ot_vo_layer VoLayer, ot_vo_chn VoChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn);

td_s32 vapi_vo_unbind_vpss(ot_vo_layer VoLayer, ot_vo_chn VoChn, ot_vpss_grp VpssGrp, ot_vpss_chn VpssChn);

td_s32 vapi_vo_start_pip_layer(SCREEN_RES_E enRes);

td_s32 vapi_vo_stop_pip_layer();

td_s32 vapi_vo_pip_rebind(ot_vo_dev VoDev);

td_s32 vapi_vo_get_pipbind();

td_void vapi_vo_get_srceen_res(SCREEN_E enScreen, td_s32 *W, td_s32 *H);

td_s32 vapi_vo_set_csc(ot_vo_layer VoLayer, CSC_E enCSC);

td_void vapi_vo_update_state(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_s32 state);

td_bool vapi_vo_check_state(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_s32 state);

td_s32 vapi_vo_set_ratio(ot_vo_layer VoLayer, ot_vo_chn VoChn, RATIO_E enRatio);

td_s32 vapi_vo_chn_get_wh(ot_vo_layer VoLayer, ot_vo_chn VoChn, td_u32 *pu32Width, td_u32 *pu32Height);

// audio
td_s32 vapi_aio_init();

td_s32 vapi_aio_uninit();

td_bool vapi_aio_is_support_talk();

td_s32 vapi_aio_ai_read_frame(ot_audio_dev AiDevId, ot_audio_sample_rate enOutSampleRate, AENC_E enAenc, AIO_FRAME_S *pstFrame);

td_s32 vapi_aio_ao_write_frame(ot_audio_dev AoDevId, AIO_FRAME_S *pstFrame);

td_s32 vapi_aio_ai_set_volume(ot_audio_dev AiDevId, td_u8 value);

td_s32 vapi_aio_ao_set_volume(ot_audio_dev AoDevId, td_u8 value);

// hifb

td_s32 vapi_fb_init(SCREEN_E enScreen);

td_s32 vapi_switch_display(SCREEN_E enScreen);

// td_s32 vapi_fb_clear_buffer(SCREEN_E enScreen);

td_s32 vapi_fb_uninit();

// utilty
td_s32 comm_mutex_init(pthread_mutex_t *mutex);

void comm_mutex_uninit(pthread_mutex_t *mutex);

int comm_mutex_timelock(pthread_mutex_t *mutex, int us);

int comm_mutex_trylock(pthread_mutex_t *mutex);

void comm_mutex_lock(pthread_mutex_t *mutex);

void comm_mutex_unlock(pthread_mutex_t *mutex);

int comm_task_set_name(const char *name);

int comm_task_create(pthread_t *handle, void *(*func)(void *), void *arg);

int comm_task_create_join(pthread_t *handle, void *(*func)(void *), void *arg);

int comm_task_join(pthread_t *handle);

int comm_epoll_create(int epmax);

int comm_epoll_destroy(int epfd);

void comm_epoll_add(int epfd, int client, int event);

void comm_epoll_del(int epfd, int client, int event);

int comm_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#ifdef __cplusplus
}
#endif

#endif
