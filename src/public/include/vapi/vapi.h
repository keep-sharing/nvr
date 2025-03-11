/* 
 * ***************************************************************
 * Filename:      	vapi.h
 * Created at:    	2015.10.21
 * Description:   	common api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __VAPI_H__
#define __VAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

 /*
 ************************************  video  ************************************             
 */

#if defined(_HI3536C_)
    #define MAX_LIVE_NUM    (8)
    #define MAX_PBK_NUM     (8)
#elif defined(_HI3798_) || defined(_NT98323_) || defined(_NT98633_)
    #define MAX_LIVE_NUM    (16)
    #define MAX_PBK_NUM     (16)
#elif defined(_HI3536_)
    #define MAX_LIVE_NUM    (64)
    #define MAX_PBK_NUM     (64)
#elif defined(_HI3536A_)
    #define MAX_LIVE_NUM    (64)
    #define MAX_PBK_NUM     (64)
#else
     #error YOU MUST DEFINE  CHIP_TYPE!
#endif
#define MAX_PIP_NUM     (1)
#define MAX_VENC_NUM    (8)
#define MAX_JPEG_NUM    (2)
//per srceen max display channel number
#define MAX_CHN_NUM     (MAX_LIVE_NUM + MAX_PBK_NUM + MAX_PIP_NUM + MAX_VENC_NUM)
#define PBK_IDX_BASE    MAX_LIVE_NUM
#define PIP_IDX_BASE    MAX_LIVE_NUM + MAX_PBK_NUM 
#define VENC_IDX_BASE   (MAX_LIVE_NUM + MAX_PBK_NUM + MAX_PIP_NUM)
#define JPEG_IDX_BASE   (VENC_IDX_BASE + 1)

#define FE_DEBUG
/***    显示器分辨率***/

typedef enum screen_res_e{
    SCREEN_UNDEFINE = -1,
    SCREEN_1024X768_60 = 0,
    SCREEN_1280X720_60,
    SCREEN_1280X1024_60,
    SCREEN_1600X1200_60,
    SCREEN_1920X1080_30,
    SCREEN_1920X1080_50,
    SCREEN_1920X1080_60,
    SCREEN_3840X2160_30,  
    SCREEN_3840X2160_60,
    SCREEN_RES_NUM,
}SCREEN_RES_E;

/***解码支持格式***/

typedef enum type_e{
    ENC_TYPE_H264 = 0,
	ENC_TYPE_MPEG4,
    ENC_TYPE_MJPEG,
    ENC_TYPE_H265,
    ENC_TYPE_NUM,
}TYPE_E;

/*** 编码模式***/
typedef enum rc_e
{
    ENC_RC_CBR = 0,
    ENC_RC_VBR,
    ENC_RC_FIXQP,
    ENC_RC_NUM,
}RC_E;

/***码流支持类型***/

typedef enum stream_e{
    STREAM_MAIN = 0,
    STREAM_SUB,
    STREAM_TRI,
    STREAM_NUM,
}STREAM_E;

/***  状态标识类型***/

typedef enum state_e{
//  device state
    STATE_INVALID = 0,
    STATE_DISCONNECT,
    STATE_CONNECT,
    STATE_CONNECT_MAIN,
    STATE_CONNECT_SUB,
    STATE_CONNECT_TRI,
    STATE_OVERFLOW,
//   channel  state
    STATE_IDLE,
    STATE_READY,
    STATE_BUSY,
//    zoomin state
    STATE_ENTER,
    STATE_ZOOMIN,
    STATE_EXIT,
    STATE_SNAP_BEGIN,
    STATE_SNAP_END,
}STATE_E;

/*** 刷新视图方式***/

typedef enum refresh_e{
    REFRESH_CLEAR_VDEC = 0,     //清除所有解码通道
    REFRESH_STILL_VDEC,         //保留后台解码通道
    REFRESH_NUM,
}REFRESH_E;

/*** 功能模式类型***/

typedef enum mode_e{
    DSP_MODE_LIVE = 0,          //实时预览
    DSP_MODE_PBK,               //录像回放
    DSP_MODE_ZOOMIN,            //电子放大
    DSP_MODE_ANPR,
    DSP_MODE_NUM,
}MODE_E;

/*** 显示布局类型***/

typedef enum layout_e{
    LAYOUT_1 = 1,
    LAYOUT_1x2,
    LAYOUT_2X2,
    LAYOUT_5_1,
    LAYOUT_7_1,
    LAYOUT_4_4,
    LAYOUT_3X3,    
    LAYOUT_3X4,
    LAYOUT_11_1,
    LAYOUT_12_2,
    LAYOUT_4X4,    
    LAYOUT_5X5,
    LAYOUT_32,//milesight
    LAYOUT_32_1,//hikvision
    LAYOUT_32_2,//milesight
    LAYOUT_6X6,
    LAYOUT_8x8,
    LAYOUT_ZOOMIN,
    LAYOUT_PIP,
    LAYOUT_BACKUP,
    LAYOUT_NUM,
}LAYOUT_E;

typedef enum view_e{
    VIEW_NORMAL = 0,
    VIEW_LPR,
}VIEW_E;

/*** 多屏标识类型***/

typedef enum screen_e{
    SCREEN_MAIN = 0, //主屏
    SCREEN_SUB  = 1, //辅屏
    SCREEN_NUM  = 2,
} SCREEN_E;

/*** 电子放大窗口类型***/

typedef enum win_type_e{
    WIN_TYPE_NORMAL = 0,    //缩放大窗口
    WIN_TYPE_ZOOMIN,        //全景小窗口
    WIN_TYPE_NUM,
}WIN_TYPE_E;

/*** 区域坐标类型***/

typedef enum zone_mode_e{
    ZONE_MODE_AUTO = 0,
    ZONE_MODE_USER,
}ZONE_MODE_E;

/*** 输出图像效果类型***/

typedef enum csc_e{
    CSC_STANDARD = 0,           //标准
    CSC_LIGHTNESS,          //明亮
    CSC_SOFT,               //柔和
    CSC_COLORFUL,           //鲜艳
    CSC_NUM,    
}CSC_E;

/*** 单窗口操作类型***/

typedef enum action_e{
    ACTION_DEL_ONE_WIN = (0x01L<<0),                                      //删除单个指定窗口
    ACTION_ADD_ONE_WIN = (0x01L<<1),                                      //添加单个指定窗口
    ACTION_UPDATE_ONE_WIN = ACTION_DEL_ONE_WIN | ACTION_ADD_ONE_WIN,      //更新单个指定窗口
}ACTION_E;

/*** 输出图像宽高比***/

typedef enum ratio_e{
    RATIO_VO_FULL = 0,          //全屏
    RATIO_VO_AUTO,          // 1:1
    RATIO_VO_MANUAL,        //手动设置
}RATIO_E;

typedef enum fish_e{
    FISH_MODE_NONE = 0,
    FISH_MODE_1O,
    FISH_MODE_1P,
    FISH_MODE_1W,
    FISH_MODE_2P,
    FISH_MODE_1P1R,
    FISH_MODE_1W1R,
    FISH_MODE_1O3R,
    FISH_MODE_1P4R,
    FISH_MODE_1W4R,
    FISH_MODE_1P6R,
    FISH_MODE_4R,
    FISH_MODE_1O8R,
    FISH_MODE_1O1R1W2P1P,
    FISH_MODE_1O1R,
    FISH_MODE_NUM,
}FISH_E;
/*** 通道绑定关系***/

typedef struct bind_s{
    int         vdecChn;        //解码通道
    int         voutChn;        //显示通道
}BIND_S;

/*** 解码/编码参数***/

typedef struct format_s{
    TYPE_E      enType;
    int         fps;
    int         width;
    int         height;
    int         bitRate;
    RC_E        enRc;
}FORMAT_S;

/*** 窗口显示坐标***/

typedef struct zone_s{
    ZONE_MODE_E enMode;
    int         x;
    int         y;
    int         w;
    int         h;
}ZONE_S;

/*** IPC设备属性***/

typedef struct devInfo_s{
    int         id;
    BIND_S      stBind;
    FORMAT_S    stFormat;
}DEVINFO_S;

typedef struct dev_info_s{
    int         devID;
    int         voutID;
	RATIO_E		enRatio;
    FORMAT_S    stFormat[STREAM_NUM];
}DEV_INFO_S;

/*** 通道属性***/

typedef struct chnInfo_s{
    STATE_E     enState;
    FORMAT_S    stFormat;
    ZONE_S      stZone;
    BIND_S      stBind;
    int         used;
}CHNINFO_S;

/*** 显示窗口属性***/

typedef struct wind_s{
    DEVINFO_S   stDevinfo;
    ZONE_S      stZone;
    RATIO_E     enRatio;
    FISH_E      enFish;
}WIND_S;

/*** 编码帧信息***/

typedef struct venc_frame_s{
    FORMAT_S    stFormat;
    int     isIframe;
    unsigned long long pts;
    void    *data;
    int     size;
    void  *pstPrivate;
}VENC_FRAME_S;

/*** 解码回调***/

typedef struct  vdec_cb_s{
    int  (*stop_stream_cb)(int chnID);
    int  (*start_stream_cb)(int chnID);
    int  (*update_chn_cb)(int devid, int chnID, STATE_E enState, int bandwidth);
    int  (*update_screen_res_cb)(int screen, int res);
    int  (*venc_stream_cb)(int Vechn, VENC_FRAME_S *pstEncFrame);
}VDEC_CB_S;

/*
 ************************************  audio  ************************************             
 */
 
/***音频输出设备类型***/

typedef enum ao_e{
    AO_SPK_MIC = 0,             //外接扬声器/ 耳机输出
    AO_MAIN_HDMI,               //主屏HDMI接口音频输出
    AO_SUB_HDMI,                //辅屏HDMI接口音频输出
    AO_TYPE_NUM,
}AO_E;

/***音频编码类型***/

typedef enum aenc_e{
    AENC_PCM_RAW = 0,
    AENC_PCM_ULAW,
    AENC_PCM_ALAW,
    AENC_PCM_AAC,
    AENC_PCM_G722,
    AENC_PCM_G726,
    AENC_NUM,
}AENC_E;

/***音频采样率类型***/

typedef enum arate_e{
    ARATE_8000  = 8000,
    ARATE_16000 = 16000,
    ARATE_24000 = 24000,
    ARATE_32000 = 32000,
    ARATE_44100 = 44100,
    ARATE_48000 = 48000,
}ARATE_E;

/***音频数据位宽类型***/

typedef enum abit_e{
    ABIT_8_BIT = 8,
    ABIT_16_BIT = 16,
}ABIT_E;

typedef enum acmd_e{
    ACMD_VO_SET_VOLUME = 0,
    ACMD_VI_SET_VOLUME,
    ACMD_VO_SET_MUTE,
    ACMD_NUM,
}ACMD_E;

typedef enum dsp_e{
    DSP_REALTIME = 0,
    DSP_FLUENT,
}DSP_E;
/*
 ************************************  Public  ************************************             
 */
 
/*** 音频帧信息***/

 typedef struct aio_frame_s{
    ARATE_E             enArate;    //采样率
    AENC_E              enAenc;     //编码格式
    ABIT_E              enAbit;     //位宽
    unsigned long long  pts;        //时间戳(us)
    unsigned char      *date;      
    unsigned int        len;
}AIO_FRAME_S;

/*** 音频控制信息***/

typedef struct aio_ctrl_s{
    ACMD_E          enCmd;
    int             devID;
    unsigned char   value;
}AIO_CTRL_S;

/*** 视图配置信息***/

typedef struct layout_s{
    SCREEN_E    enScreen;
    MODE_E      enMode;
    WIND_S      stWinds[MAX_CHN_NUM];
    int         winds_num;
    REFRESH_E   enRefresh;
}LAYOUT_S;

/***电子放大配置信息 ***/

typedef struct zoomin_s{
    STATE_E     enState;
    SCREEN_E    enScreen;
    MODE_E      enMode;
    int         devID;
    int         vdecID;
    FORMAT_S    stFormat;
    ZONE_S      stZone[WIN_TYPE_NUM];  //两个视图位置坐标
    ZONE_S      stRect;                //选中的缩放区域
    REFRESH_E   enRefresh;             //是否后台保留解码通道
	RATIO_E		enRatio;
}ZOOMIN_S;

typedef struct zoomin2_s{
    SCREEN_E    enScreen;
    int         VoChn;
    ZONE_S      stZone;                //两个视图位置坐标
    ZONE_S      stRect;                //选中的缩放区域
}ZOOMIN2_S;

/*** 全屏配置信息***/

typedef struct full_s{
    SCREEN_E    enScreen;
    MODE_E      enMode;
    int         devID;
    int         vdecID;
    FORMAT_S    stFormat;
    ZONE_S      stZone;
    REFRESH_E   enRefresh;
}FULL_S;

/***屏幕分辨率配置信息 ***/

typedef struct screen_res_s{
    SCREEN_E        enScreen;
    SCREEN_RES_E    enRes;
}SCREEN_RES_S;

/***输出图像效果信息 ***/

typedef struct img_s{
    SCREEN_E    enScreen;
    CSC_E       enCSC;
}IMG_S;

/***输出窗口的比例 ***/

typedef struct ratio_s{
	int			devId;
    SCREEN_E    enScreen;
	MODE_E		enMode;
	RATIO_E		enRatio;
}RATIO_S;

typedef struct ratio2_s{
    int         vouid;
    SCREEN_E    enScreen;
	RATIO_E		enRatio;
}RATIO2_S;

/*** 单窗口配置信息***/

typedef struct action_s{
    ACTION_E    enAction;
    SCREEN_E    enScreen;
    MODE_E      enMode;
    WIND_S      stWind;
}ACTION_S;

typedef struct action2_s{
    ACTION_E    enAction;
    SCREEN_E    enScreen;
    WIND_S      stWind;
}ACTION2_S;

/*** 转码handle***/

typedef struct transfer_s{
    int id;
    int VeChn;
    int VdChn;
}TRANSTER_S;

/*** vapi 截图帧信息***/

typedef struct VAPI_YUV_S{
    void *data;
    int size;
    int width;
    int height;
    int phy;
}VAPI_YUV_S;

typedef struct VAPI_RGB_S{
    void *data;
    int size;
    int width;
    int height;
}VAPI_RGB_S;

typedef struct vapi_jpeg_s{
    TYPE_E enType;
    unsigned int u32InWidth;
    unsigned int u32InHeight;
    void *pu8InAddr;
    unsigned int u32InLen;
    unsigned int u32Qfactor;  // 1-99
    unsigned int u32OutWidth;
    unsigned int u32OutHeight;
}VAPI_JPEG_S;

/*** vapi 初始化信息***/

typedef struct  vapi_attr_s{
    SCREEN_RES_E    enRes[SCREEN_NUM];
    VDEC_CB_S       stCallback;
    int             isHotplug;
    int             isBlock;
    int             isDualScreen;
    int             isogeny;
    char            prefix[16];// 1 5 7 8 only one bit
}VAPI_ATTR_S;

int vapi_init(VAPI_ATTR_S *pstAttr);

int vapi_uninit();

int yu_vapi_add_one_win(SCREEN_E enScreen, MODE_E enMode, WIND_S *pstWin);
int yu_vapi_del_one_win(SCREEN_E enScreen, MODE_E enMode, int devID, REFRESH_E enRefresh);

int vapi_update_layout(LAYOUT_S *pstLayout);

int vapi_set_screen_resolution(SCREEN_RES_S *pstScreenRes);

int vapi_switch_screen(SCREEN_E enScreen);

int vapi_set_zoomin(ZOOMIN_S *pstZoomin);

int vapi_set_zoomin2(ZOOMIN2_S *pstZoomin);

int vapi_set_full_screen(FULL_S *pstFull);

int vapi_set_action(ACTION_S *pstAction);

int vapi_set_action2(ACTION2_S *pstAction);

int vapi_get_vdec_chn(SCREEN_E enScreen, MODE_E enMode, int devID);

void vapi_get_screen_res(SCREEN_E enScreen, int *W, int *H);

void vapi_clear_vout_chn(SCREEN_E enScreen, int VoChn);

void vapi_clear_screen(SCREEN_E enScreen, REFRESH_E enRefresh);

void vapi_clear_screen2(SCREEN_E enScreen);

int vapi_set_vout_img(IMG_S *pstImg);

int vapi_set_vout_ratio(RATIO_S *pstRatio);

int vapi_set_vout_ratio2(RATIO2_S *pstRatio);

int vapi_send_frame(int VdChn, unsigned char *pAddr, int Len, unsigned long long pts, int flag);

int vapi_audio_read(int AiDevId, ARATE_E enArate, AENC_E enAenc, AIO_FRAME_S * pstFrame);

int vapi_audio_write(AO_E AoDevId, AIO_FRAME_S * pstFrame);

int vapi_audio_ctrl(AIO_CTRL_S *pstCtrl);

int vapi_audio_is_support_talk(void);

int vapi_fb_clear_buffer(SCREEN_E enScreen);

int vapi_set_vout_fps(int devID, int fps);

int vapi_transfer_create(int VdChn, FORMAT_S * pIf, FORMAT_S * pOf, void *pstPrivate);

void vapi_transfer_destory(int VeChn);

int vapi_transfer_config(int VeChn, int FPS, int bitRate);

int vapi_transfer_check();

int vapi_get_jpeg_frame(SCREEN_E enScreen, MODE_E enMode, int devID, VAPI_JPEG_S *pstJpeg);

void vapi_put_jpeg_frame(VAPI_JPEG_S *pstJpeg);

int vapi_get_jpeg_frame(SCREEN_E enScreen, MODE_E enMode, int devID, VAPI_JPEG_S *pstJpeg);

int vapi_get_jpeg_frame2(SCREEN_E enScreen, int VoChn, VAPI_JPEG_S *pstJpeg);

void vapi_put_jpeg_frame(VAPI_JPEG_S *pstJpeg);

int vapi_jpeg_hw_convert(VAPI_JPEG_S *pstJpeg, unsigned char **ppu8OutAddr, unsigned int *pu32OutLen, unsigned int *pu32OutWidth, unsigned int *pu32OutHeight);

void vapi_display_mode(DSP_E enDsp);

#if defined(_HI3536G_)
int vapi_vpool_new(int blkSize, int blkCnt, char *poolName);
int vapi_vpool_delete(int poolId);
int vapi_fish_vo(SCREEN_E enScreen, int VoChn, int poolId, int inWidth, int inHeight, void *pVirAddr, int phyAddr);
int vapi_grab_yuv(SCREEN_E enScreen, int VoChn, void *pVirAddr, int phyAddr);
int vapi_fish_src_wh(SCREEN_E enScreen, int VoChn, int *pInWidth, int *pInHeight, int *pOutWidth, int *pOutHeight);
#endif

#ifdef __cplusplus
}
#endif

#endif

