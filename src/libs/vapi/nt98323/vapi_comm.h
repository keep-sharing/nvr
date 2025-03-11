/*
 * ***************************************************************
 * Filename:        vapi_comm.h
 * Created at:      2015.10.21
 * Description:     common api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __VAPI_COMM_H__
#define __VAPI_COMM_H__

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/epoll.h>

#include "vendor_videoproc.h"
#include "vendor_common.h"
#include "msstd.h"
#include "vapi.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

typedef unsigned char           HI_U8;
typedef unsigned short          HI_U16;
typedef unsigned int            HI_U32;
typedef float                   HI_FLOAT;
typedef double                  HI_DOUBLE;
typedef signed char             HI_S8;
typedef short                   HI_S16;
typedef int                     HI_S32;


#ifndef _M_IX86
typedef unsigned long long  HI_U64;
typedef long long           HI_S64;
#else
typedef __int64             HI_U64;
typedef __int64             HI_S64;
#endif

typedef char                    HI_CHAR;
#define HI_VOID                 void

/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
typedef enum {
    HI_FALSE = 0,
    HI_TRUE  = 1,
} HI_BOOL;

#ifndef NULL
#define NULL    0L
#endif

#define HI_NULL     0L
#define HI_SUCCESS  0
#define HI_FAILURE  (-1)

#define RED             "\033[0;31m"
#define GREEN           "\033[0;32m"
#define CLRCOLOR        "\033[0;39m "

typedef enum debug_s {
    DEBUG_VDEC = (1 << 0),
    DEBUG_VPSS = (1 << 1),
    DEBUG_VOUT = (1 << 2),
    DEBUG_ALL = DEBUG_VDEC | DEBUG_VPSS | DEBUG_VOUT,
} DEBUG_S;

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
#define VAPI_JPEG_VDEC_ID        MAX_LIVE_NUM
#define VAPI_JPEG_VENC_ID        MAX_VENC_NUM
#define VAPI_JPEG_VPE_ID         (VAPI_JPEG_VDEC_ID+1)

#define VAPI_VENC_MAX_WIDTH     (1920)
#define VAPI_VENC_MAX_HEIGTH     (1080)

#define VAPI_VENC_MIN_WIDTH     (480)
#define VAPI_VENC_MIN_HEIGTH    (320)

#define VAPI_VDEC_MAX_WIDTH     (3840)
#define VAPI_VDEC_MAX_HEIGTH     (3840)

/* audio codec */
typedef struct {
    unsigned int sample;        //0:48k, 1:32k, 2:24k, 3:16k, 4:12k, 5:8k
    unsigned int data_length;   //0:16bit 1:20bit 2:24bit 3:32bit
    unsigned int ctrl_mode;     //0:Slaver 1:Master
    unsigned int trans_mode;    //0: I2S   1:PCM
    unsigned int mono_mute;
} Nau88c10_Ctrl;

#define SOFT_RESET              0x00
#define SET_CTRL_MODE           0x01
#define SET_TRANSFER_MODE       0x02
#define SET_DAC_SAMPLE          0x03
#define SET_ADC_SAMPLE          0x04
#define SET_DATA_LENGTH         0x05
#define DAC_VOLUME_CTRL         0x06
#define MONO_OUTPUT_MUTE        0x07

typedef enum dev_e {
    VAPI_VO_DEV_VIDEO = 0,
    VAPI_VO_DEV_GUI,
    VAPI_VO_DEV_NUM,
} DEV_E;

typedef struct res_s {
    int enCapLevel;
    HI_U32 u32Width;
    HI_U32 u32Height;
} RES_S;

typedef struct frame_s {
    TYPE_E enType;
    UINT32 quality;
    UINT32 width;
    UINT32 height;
    UINT32 size;
    UINT32 phyAddr;
    void *vriAddr;
} FRAME_T;
//sys

HD_RESULT vapi_sys_init();
HD_RESULT vapi_sys_uninit();

//vpss
HD_RESULT vapi_vpss_init();

HD_RESULT vapi_vpss_uninit();

HD_RESULT vapi_vpss_create(HI_S32 vpssChn, HI_S32 vdChn, HI_S32 voChn);

HD_RESULT vapi_vpss_destory(HI_S32 vpssChn);

HD_RESULT vapi_vpss_bind_vo(HI_S32 vpssChn, HI_S32 voChn);

HD_RESULT vapi_vpss_unbind_vo(HI_S32 vpssChn);

HD_RESULT vapi_vpss_start_stream(HI_S32 vpssChn);

HD_RESULT vapi_vpss_stop_stream(HI_S32 vpssChn);

HD_RESULT vapi_vpss_set_crop(int vpssChn, int vdChn, HD_URECT *pstRect, HD_CROP_MODE enMode);

HI_S32 vapi_vpss_snap_jpeg(int vpssChn, VAPI_JPEG_S *pstJpeg);

//vdec
HD_RESULT vapi_vdec_init(VDEC_CB_S *pstCb);

HD_RESULT vapi_vdec_uninit();

HD_RESULT vapi_vdec_chn_create(HI_S32 devId, HI_S32 vdChn, TYPE_E enType, MODE_E enMode, HI_U32 u32Width,
                               HI_U32 u32Height);

HD_RESULT vapi_vdec_chn_destory(HI_S32 vdChn);

HD_RESULT vapi_vdec_bind_vpss(HI_S32 vdChn, HI_S32 vpssChn);

HD_RESULT vapi_vdec_unbind_vpss(HI_S32 vdChn);

HI_VOID vapi_vdec_update_state(HI_S32 vdChn, HI_S32 state);

HI_BOOL vapi_vdec_check_state(HI_S32 vdChn, HI_S32 state);

HI_S32 vapi_vdec_send_frame(HI_S32 vdChn, HI_U8 *pu8Addr, HI_U32 u32Len, HI_U64 u64PTS, int flag);

HD_RESULT vapi_vdec_start_stream(HI_S32 vdChn);

HD_RESULT vapi_vdec_stop_stream(HI_S32 vdChn);

HD_PATH_ID vapi_vdec_get_chn_path(HI_S32 vdChn);

HI_VOID vapi_vdec_get_chn_wh(HI_S32 vdChn, UINT32 *W, UINT32 *H);

HI_S32 vapi_vdec_chn_update(HI_S32 devId, HI_S32 vdChn, STATE_E enState);


//venc
HD_RESULT vapi_venc_init();

HD_RESULT vapi_venc_uninit();

//jpeg
HI_S32 vapi_jpeg_hw(TYPE_E enType, HI_U32 u32InWidth, HI_U32 u32InHeight, HI_U8 *pu8InAddr, HI_U32 u32InLen,
                    HI_U32 u32Qfactor, HI_U32 u32OutWidth, HI_U32 u32OutHeight, HI_U8 **ppu8OutAddr, HI_U32 *pu32OutLen,
                    HI_U32 *pu32OutWidth, HI_U32 *pu32OutHeight, pthread_mutex_t *mutex);

HI_S32 vapi_yuv2jpeg(HI_S32 vdChn, VAPI_JPEG_S *pstJpeg);

HD_RESULT vapi_jpeg_init();

HD_RESULT vapi_jpeg_uninit();


//vout
HD_RESULT vapi_vo_init(SCREEN_RES_E enMain, SCREEN_RES_E enSub, HI_S32 isHotplug, HI_BOOL isQuick, VDEC_CB_S *pstCb);

HD_RESULT vapi_vo_uninit();

HD_RESULT vapi_vo_dev_open(DEV_E VoDev, SCREEN_RES_E enRes);

HD_RESULT vapi_vo_dev_close(DEV_E VoDev);

HD_RESULT vapi_vo_chn_create(HI_S32 voChn, HD_URECT  *pstRect, RATIO_E enRatio);

HD_RESULT vapi_vo_chn_destory(HI_S32 voChn);

HD_RESULT vapi_vo_start_stream(HI_S32 voChn);

HD_RESULT vapi_vo_stop_stream(HI_S32 voChn);

HI_S32  vapi_vo_chn_clear_buff(DEV_E VoDev, HI_S32 voChn);

HI_VOID vapi_vo_dsp_quick(HI_BOOL bQuick);

HI_S32 vapi_vo_bind_vdec(HI_S32 voChn, HI_S32 VdecChn);

HI_S32 vapi_vo_unbind_vdec(HI_S32 voChn, HI_S32 vdecChn);

HI_VOID vapi_vo_get_srceen_res(SCREEN_E enScreen, HI_S32 *W, HI_S32 *H);

HD_RESULT vapi_vo_get_win_rect(HI_S32 voChn, HD_URECT  *pstRect, HD_DIM *pstDim);

HD_RESULT vapi_vo_get_fb_fmt(HD_FB_FMT *pstFbFmt);

HI_VOID vapi_vo_update_state(HI_S32 voChn, HI_S32 state);

HI_BOOL vapi_vo_check_state(HI_S32 voChn, HI_S32 state);

HI_S32  vapi_vo_set_ratio(HI_S32 voChn, RATIO_E enRatio);

HI_S32 vapi_vo_set_crop(HI_S32 voChn, HD_URECT *pstRect);

HI_S32 vapi_vo_snap_jpeg(HI_S32 voChn, VAPI_JPEG_S *pstJpeg);

//audio
HD_RESULT vapi_aio_init();

HD_RESULT vapi_aio_uninit();

HI_BOOL vapi_aio_is_support_talk();

HI_S32 vapi_aio_ai_read_frame(HI_S32 aiDevId, ARATE_E enOutSampleRate, AENC_E enAenc, AIO_FRAME_S *pstFrame);

HI_S32 vapi_aio_ao_write_frame(HI_S32 aoDevId, AIO_FRAME_S *pstFrame);

HI_S32 vapi_aio_ai_set_volume(HI_S32 aiDevId, HI_U8 value);

HI_S32 vapi_aio_ao_set_volume(HI_S32 aoDevId, HI_U8 value);

HI_S32 vapi_aio_ao_set_mute(HI_S32 aoDevId, HI_S32 mute);


//hifb
HI_S32 vapi_fb_init(SCREEN_E enScreen);

HI_S32 vapi_switch_display(SCREEN_E enScreen);

//HI_S32 vapi_fb_clear_buffer(SCREEN_E enScreen);

HI_S32 vapi_fb_uninit();

//utilty
HI_S32 comm_mutex_init(pthread_mutex_t *mutex);

void comm_mutex_uninit(pthread_mutex_t *mutex);

int comm_mutex_trylock(pthread_mutex_t *mutex);

int comm_mutex_timelock(pthread_mutex_t *mutex, int us);

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

HD_VIDEO_CODEC comm_codec_type(TYPE_E enType);

#ifdef __cplusplus
}
#endif


#endif


