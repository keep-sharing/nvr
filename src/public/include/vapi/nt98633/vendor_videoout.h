/**
@brief Header file of vendor videoout.
This file contains the functions which is related to lcd config in the chip.

@file vendor_videoout.h

@ingroup mhdal

@note Nothing.

Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/

#ifndef _VENDOR_LCD_H_
#define _VENDOR_LCD_H_

/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hdal.h"
#include "hd_type.h"
#include "hd_util.h"
#include "hd_videoout.h"
#include "vendor_common.h"
#ifdef  __cplusplus
extern "C" {
#endif

#define VENDOR_VIDEOOUT_VERSION 0x010015 //vendor lcd version
#define VENDOR_VO_DEVID(self_id)     (self_id - HD_DAL_VIDEOOUT_BASE)
#define VENDOR_VIDEOOUT_MAX_DEVICE_ID 6
#define VENDOR_VIDEOOUT_VDDO0      0  ///< Indicate vddo device(LCD300)
#define VENDOR_VIDEOOUT_VDDO1      1  ///< Indicate vddo device(LCD210_1)
#define VENDOR_VIDEOOUT_VDDO2      2  ///< Indicate vddo device(LCD210_2)
#define VENDOR_VIDEOOUT_VDDO3      3  ///< Indicate vddo device(channel zero)
#define VENDOR_VIDEOOUT_VDDO4      4  ///< Indicate vddo value(reserved)
#define VENDOR_VIDEOOUT_VDDO5      5  ///< Indicate vddo value(reserved)
#define VENDOR_VIDEOOUT_EDID_NU	   32
#define MAX_EDID_NU 256

#define VENDOR_VIDEOOUT_HDMI_3840X2160P60  101//CIF_HD_VIDEOOUT_HDMI_3840X2160P60

#define  VENDOR_VIDEOOUT_LAYER_MAPPING_MAX 3//VOUT_LAYER_MAPPING_MAX
/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
/********************************************************************
MACRO FUNCTION DEFINITIONS
********************************************************************/
/********************************************************************
TYPE DEFINITION
********************************************************************/
typedef struct _VENDOR_VIDEOOUT_WRITEBACK{
	INT16   enabled;    ///< write back hdmi to cvbs 0:disable 1:enable
	INT16   wb_lx; 	    ///< write-back x start pixel (two-pixel alignment), Left x, for SDO1 
    INT16   wb_ty;      ///< write-back y start line, Top Y, for SDO1, range: 0 ~ 12 
    INT16   wb_rx;      ///< write-back x start pixel (two-pixel alignment), Right x, for SDO1 
    INT16   wb_by;      ///< write-back y start line, Bottom Y, for SDO1, range: 0 ~ 12 
    INT16   wb_width;   ///< write back width size
    INT16   wb_height;  ///< write back height size
    INT16   dst_lcd_id; ///< 1 or 2
    INT16   oneshut;    ///< 0: continuous, 1: oneshut
    INT16   sel;         /* 0: ch0~ch2+cursor, 1: ch0~ch2, 2: ch0 */
    INT16   fmt;	     /* 0: 422, 1: 420, 2: 420SCE */
} VENDOR_VIDEOOUT_WRITEBACK;

typedef struct _VENDOR_VIDEOOUT_WRITEBACK_EXT{
	INT16   enabled;    ///< write back hdmi to cvbs 0:disable 1:enable
	INT16   wb_lx; 	    ///< write-back x start pixel (two-pixel alignment), Left x, for SDO1 
    INT16   wb_ty;      ///< write-back y start line, Top Y, for SDO1, range: 0 ~ 12 
    INT16   wb_rx;      ///< write-back x start pixel (two-pixel alignment), Right x, for SDO1 
    INT16   wb_by;      ///< write-back y start line, Bottom Y, for SDO1, range: 0 ~ 12 
    INT16   wb_width;   ///< write back width size
    INT16   wb_height;  ///< write back height size
    INT16   dst_lcd_id; ///< only support lcd200 
    UINTPTR  wb_ba[5];	/* max is 5 piece of buffers */
	UINTPTR  share_mem_ba;	/* must be 64 bytes alignment */
	UINT16  piece_buffer_sz;
	UINT16  num_of_piece;
	UINT16  share_mem_sz;	/* must be 128 bytes alignment */
} VENDOR_VIDEOOUT_WRITEBACK_EXT;

typedef enum _VENDOR_WIN_LAYER{
	VENDOR_WIN_LAYER1 = 0,     ///< 1st layer (background, first draw, lowest, the same as HD_LAYER1)
	VENDOR_WIN_LAYER2,         ///< 2nd layer (the same as HD_LAYER2)
	VENDOR_WIN_LAYER3,         ///< 3rd layer
	VENDOR_WIN_LAYER4,         ///< 4th layer
	VENDOR_WIN_LAYER5,         ///< 5th layer
	VENDOR_WIN_LAYER6,         ///< 6th layer
	VENDOR_WIN_LAYER7,         ///< 7th layer
	VENDOR_WIN_LAYER8,         ///< 8th layer (last draw, on the top)
	VENDOR_WIN_LAYER_MAX,
	ENUM_DUMMY4WORD(VENDOR_WIN_LAYER)
} VENDOR_VIDEOOUT_WIN_LAYER;

typedef struct _VENDOR_VIDEOOUT_WIN_LAYER_ATTR {
	VENDOR_VIDEOOUT_WIN_LAYER layer;
} VENDOR_VIDEOOUT_WIN_LAYER_ATTR;

typedef enum _VENDOR_VIDEOOUT_INPUT_DIM {
	VENDOR_VIDEOOUT_IN_1440x900 = 14,       ///< IN VIDEO FORMAT IS 1440x900,start from HD_VIDEOOUT_IN_3840x2160
	VENDOR_VIDEOOUT_IN_1680x1050 = 15,      ///< IN VIDEO FORMAT IS 1680x1050
	VENDOR_VIDEOOUT_IN_1920x1200 = 16,      ///< IN VIDEO FORMAT IS 1920x1200
	VENDOR_VIDEOOUT_IN_1024x600 = 17,
	VENDOR_VIDEOOUT_INPUT_DIM_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_INPUT_DIM)
} VENDOR_VIDEOOUT_INPUT_DIM;

typedef enum _VENDOR_VIDEOOUT_BT_ID {
	VENDOR_VIDEOOUT_BT1120_1280X720P60 = 0, ///< EXT VIDEO FORMAT IS BT1120_1280X720P60	
	VENDOR_VIDEOOUT_BT1120_1280X720P30,      ///< EXT VIDEO FORMAT IS BT1120_1280X720P30
	VENDOR_VIDEOOUT_BT1120_1280X720P25,      ///< EXT VIDEO FORMAT IS BT1120_1280X720P25
	VENDOR_VIDEOOUT_BT1120_1920X1080P60,  ///< EXT VIDEO FORMAT IS BT1120_19200X1080P60
	VENDOR_VIDEOOUT_BT1120_1024X768P60,  	///< EXT VIDEO FORMAT IS BT1120_1024X768P60
	VENDOR_VIDEOOUT_BT1120_1280X1024P60,  	///< EXT VIDEO FORMAT IS BT1120_1280X1024P60
	VENDOR_VIDEOOUT_BT1120_1600X1200P60,  	///< EXT VIDEO FORMAT IS BT1120_1600X1200P60
	VENDOR_VIDEOOUT_BT1120_37M_1280X720P30,    ///< EXT VIDEO FORMAT IS BT1120_1280X720P30
    VENDOR_VIDEOOUT_BT1120_37M_1280X720P25,  ///< EXT VIDEO FORMAT IS BT1120_1280X720P25
	VENDOR_VIDEOOUT_BT1120_1920X1080P50,    //< EXT VIDEO FORMAT IS BT1120_19200X1080P50
	VENDOR_VIDEOOUT_BT1120_1440X900P60,
	
	VENDOR_VIDEOOUT_BT656_1280X720P30  = 100,     ///< EXT VIDEO FORMAT IS BT656_1280X720P30	
	VENDOR_VIDEOOUT_BT656_1280X720P25 ,      ///< EXT VIDEO FORMAT IS BT656_1280X720P25
	
	VENDOR_VIDEOOUT_BT_NO_CHANGE  = 0xFFFE, ///< EXT VIDEO FORMAT USE CURRENT SETTING
	VENDOR_VIDEOOUT_BT_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_BT_ID)
} VENDOR_VIDEOOUT_BT_ID;

typedef enum _VENDOR_VIDEOOUT_HDMI_ID {
	VENDOR_VIDEOOUT_HDMI_1440X900P60    = 60,    ///< HDMI VIDEO FORMAT IS 1440X900 & PROGRESSIVE 60FPS,start from HD_VIDEOOUT_HDMI_720X480I240_16X9
	VENDOR_VIDEOOUT_HDMI_1680X1050P60   = 61,    ///< HDMI VIDEO FORMAT IS 1680X1050 & PROGRESSIVE 60FPS
	VENDOR_VIDEOOUT_HDMI_1920X1200P60   = 62,    ///< HDMI VIDEO FORMAT IS 1920X1200 & PROGRESSIVE 60FPS
	VENDOR_VIDEOOUT_HDMI_NO_CHANGE  = 0xFFFE, ///< HDMI VIDEO FORMAT USE CURRENT SETTING
	VENDOR_VIDEOOUT_HDMI_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_HDMI_ID)
} VENDOR_VIDEOOUT_HDMI_ID;

typedef enum _VENDOR_VIDEOOUT_VGA_ID {
	VENDOR_VIDEOOUT_VGA_1440X900    = 6,    ///< VGA VIDEO FORMAT IS 1440X900,start from HD_VIDEOOUT_VGA_1920X1080
	VENDOR_VIDEOOUT_VGA_1680X1050   = 7,    ///< VGA VIDEO FORMAT IS 1680X1050
	VENDOR_VIDEOOUT_VGA_1920X1200   = 8,    ///< VGA VIDEO FORMAT IS 1920X1200
	VENDOR_VIDEOOUT_VGA_1024X600    = 9,    ///< VGA VIDEO FORMAT IS 1024X600
	VENDOR_VIDEOOUT_VGA_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_VGA_ID)
} VENDOR_VIDEOOUT_VGA_ID;

typedef enum _VENDOR_VIDEO_OUT_TYPE {
	VENDOR_VIDEO_OUT_TYPE_NONE = HD_COMMON_VIDEO_OUT_LCD,
	VENDOR_VIDEO_OUT_BT1120,      ///< BT1120
	VENDOR_VIDEO_OUT_BT656,           ///< BT656
	VENDOR_VIDEO_OUT_TYPE_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEO_OUT_TYPE)
} VENDOR_VIDEO_OUT_TYPE;

typedef struct _VENDOR_VIDEOOUT_MODE {
	HD_COMMON_VIDEO_OUT_TYPE output_type;    ///< select lcd output device
	VENDOR_VIDEOOUT_INPUT_DIM input_dim;         ///< NVR only.set input dim(IPC set input dim by HD_VIDEOOUT_PARAM_IN)
	union {
		VENDOR_VIDEOOUT_HDMI_ID   hdmi;  ///< set hdmi  output resolution
		VENDOR_VIDEOOUT_VGA_ID    vga;   ///< set vga  output resolution
	} output_mode;
} VENDOR_VIDEOOUT_MODE;

typedef struct _VENDOR_VIDEOOUT_EDID {
	UINT32 val[VENDOR_VIDEOOUT_EDID_NU];
	INT valid_num;
} VENDOR_VIDEOOUT_EDID;

typedef struct _VENDOR_VIDEOOUT_RLE_INTVAL {
    UINT16 rle_intval;
} VENDOR_VIDEOOUT_RLE_INTVAL;

typedef struct _VENDOR_EDID_TBL{
	unsigned int is_valid[MAX_EDID_NU];
    unsigned int w[MAX_EDID_NU];
    unsigned int h[MAX_EDID_NU];
	unsigned char refresh_rate[MAX_EDID_NU];
	unsigned char is_progress[MAX_EDID_NU];
	unsigned int aspect_rate[MAX_EDID_NU];
	unsigned int	edid_valid;
    unsigned int edid_length;
    unsigned char edid[512];
} VENDOR_EDID_TBL;

typedef struct _VENDOR_VIDEOOUT_EDID_CAP {
	VENDOR_EDID_TBL hdmi_edid;
} VENDOR_VIDEOOUT_EDID_CAP;

typedef struct _VENDOR_VIDEOOUT_DISP_MAX_FPS {
    unsigned int max_fps;
} VENDOR_VIDEOOUT_DISP_MAX_FPS;

typedef struct _VENDOR_VIDEOOUT_PUSHTO{
    uintptr_t  out_pa;     ///< wb out paddr
	INT16   enabled;    ///< write back hdmi to cvbs 0:disable 1:enable
	INT16   wb_lx; 	    ///< write-back x start pixel (two-pixel alignment), Left x, for SDO1
    INT16   wb_ty;      ///< write-back y start line, Top Y, for SDO1, range: 0 ~ 12
    INT16   wb_rx;      ///< write-back x start pixel (two-pixel alignment), Right x, for SDO1
    INT16   wb_by;      ///< write-back y start line, Bottom Y, for SDO1, range: 0 ~ 12
    INT16   wb_width;   ///< write back width size
    INT16   wb_height;  ///< write back height size
    INT16   oneshut;
    INT16   fmt;	     /* 0: 422, 1: 420, 2: 420SCE */
} VENDOR_VIDEOOUT_PUSHTO;

typedef struct _VENDOR_VIDEOOUT_SYSCAPS {
	UINT16	lcd_id; //0: lcd310, 1: lcd310 lite, 2: lcd210
	UINT16  pxlfmt_cnt; //total number of supporting pxlfmt
	HD_VIDEO_PXLFMT support_pxlfmt1;	///< [out] lcd310 pxlfmt
	HD_VIDEO_PXLFMT support_pxlfmt2;
	HD_VIDEO_PXLFMT support_pxlfmt3;
} VENDOR_VIDEOOUT_SYSCAPS;

typedef struct _VENDOR_VIDEOOUT_OUTPUTYPE {
	UINT32 output_type;    ///< Combination value:HD_COMMON_VIDEO_OUT_TYPE  |VENDOR_VIDEO_OUT_TYPE
} VENDOR_VIDEOOUT_OUTPUTYPE;

typedef struct _VENDOR_VIDEOOUT_VGADAC {
	UINT32 on;    ///< dac on/off
} VENDOR_VIDEOOUT_VGADAC;

typedef struct _VENDOR_VIDEOOUT_CLRWIN_ST {
	UINT8 lcd_vch; //<0~2,for videoout0~videoout2
	UINT8 status;    ///< 0:done,1:ongoing
} VENDOR_VIDEOOUT_CLRWIN_ST;

typedef struct _VENDOR_VIDEOOUT_SYS_TIME {
	UINT64 timestamp;                       ///< current system timestamp (us)
} VENDOR_VIDEOOUT_SYS_TIME;

typedef struct _VENDOR_VIDEOOUT_LAYER_POS {
	UINT32 layer_mapping;
	UINT32 pos_x;
	UINT32 pos_y;
	UINT32 blend;
} VENDOR_VIDEOOUT_LAYER_POS;

typedef struct _VENDOR_VIDEOOUT_LAYER_MAPPING_POS {
	VENDOR_VIDEOOUT_LAYER_POS  fb_info[VENDOR_VIDEOOUT_LAYER_MAPPING_MAX];
	UINT32 lcd_ch;
} VENDOR_VIDEOOUT_LAYER_MAPPING_POS;

typedef struct _VENDOR_VIDEOOUT_PIP_OTYPE {
	UINT32 src_w;
	UINT32 src_h;
	UINT32 dst_w;
	UINT32 dst_h;
} VENDOR_VIDEOOUT_PIP_OTYPE;

typedef struct _VENDOR_VIDEOOUT_PIP_OUTPUT_TYPE {
	VENDOR_VIDEOOUT_PIP_OTYPE  pip_otype;
	UINT32 lcd_ch;
} VENDOR_VIDEOOUT_PIP_OUTPUT_TYPE;

typedef struct _VENDOR_VIDEOOUT_PIP_DEV_ID {
	UINT32 vout_in_id;	    //vout in id for pip, vout only device 0 could pip
	UINT32 vproc_device_id;	//vpe device id to vout pip
	UINT32 vproc_out_id;	//vpe out id device id to vout pip
} VENDOR_VIDEOOUT_PIP_DEV_ID;

typedef struct  _VENDOR_VIDEOOUT_LINE_PARAM {
    UINT32 start_x;
    UINT32 start_y;
    UINT32 end_x;
    UINT32 end_y;
    UINT8 linewidth;
    UINT8 linecolor[3];//linecolor[0]:red, linecolor[1]:green,linecolor[2]:blue
    UINT8 draw_method;//0:bilinear, 1:nearest
} VENDOR_VIDEOOUT_LINE_PARAM;

typedef struct _VENDOR_VIDEOOUT_LINE_INFO {
	VENDOR_VIDEOOUT_LINE_PARAM *p_line_param;
	UINT32 line_param_nu;
} VENDOR_VIDEOOUT_DRAW_LINE_INFO;

//for rhd videoout clearwin
typedef struct _VENDOR_VIDEOOUT_CLEAR_WIN {
	HD_DIM input_dim;         ///< pattern buf dim, minimun input dim: 64x32
	HD_VIDEO_PXLFMT in_fmt;   ///< pattern fmt
	UINTPTR p_addr;           ///< pattern pa
	UINT32 buf_sz;			  ///< size of p_addr
	UINT32 ddr_id; 			  ///< p_addr's ddr_id
	HD_URECT output_rect;     ///< clear win region
	CLEAR_WINDOW_MODE mode;   ///< clear win mode
} VENDOR_VIDEOOUT_CLEAR_WIN;

typedef enum _VENDOR_VIDEOOUT_PARAM_ID {	
	VENDOR_VIDEOOUT_PARAM_WRITEBACK,        ///<support set/get, VENDOR_VIDEOOUT_WRITEBACK, for HDMI writeback to cvbs
	VENDOR_VIDEOOUT_PARAM_WIN_LAYER_ATTR,   ///<support set/get, VENDOR_VIDEOOUT_WIN_LAYER_ATTR, for win layer extension
	VENDOR_VIDEOOUT_PARAM_MODE,             ///<support get/set with ctrl path, using VENDOR_VIDEOOUT_MODE struct
	VENDOR_VIDEOOUT_PARAM_EDID,             ///<support get with ctrl path, using VENDOR_VIDEOOUT_EDID struct,set with unsigned int
	VENDOR_VIDEOOUT_PARAM_RLE_INTVAL,       ///<support get/set with ctrl path, using VENDOR_VIDEOOUT_RLE_INTVAL struct
	VENDOR_VIDEOOUT_PARAM_EDID_CAP,         ///<support get with ctrl path, using VENDOR_VIDEOOUT_EDID_CAP struct
	VENDOR_VIDEOOUT_PARAM_WRITEBACK_EXT,    ///<support set, VENDOR_VIDEOOUT_WRITEBACK_EXT, for HDMI writeback to cvbs with ring buf
	VENDOR_VIDEOOUT_PARAM_DISP_MAX_FPS,     ///<support set/get, using VENDOR_VIDEOOUT_DISP_MAX_FPS struct, 60fps or 30fps(default)
	VENDOR_VIDEOOUT_PARAM_WB_PUSH, 			///<support set, using VENDOR_VIDEOOUT_PUSHTO struct
	VENDOR_VIDEOOUT_PARAM_WB_PULL,          ///<support get, using VENDOR_VIDEOOUT_PUSHTO struct
	VENDOR_VIDEOOUT_PARAM_SYSCAPS,          ///<support get, VENDOR_VIDEOOUT_SYSCAPS for getting
	VENDOR_VIDEOOUT_PARAM_OUTPUTYPE,        ///<support set, VENDOR_VIDEOOUT_OUTPUTYPE struct
	VENDOR_VIDEOOUT_PARAM_VGADAC,           ///<support set, VENDOR_VIDEOOUT_VGADAC struct
	VENDOR_VIDEOOUT_PARAM_CLRWIN_ST,        ///<support get, VENDOR_VIDEOOUT_CLRWIN_ST struct
	VENDOR_VIDEOOUT_PARAM_SYS_TIME,         ///<support get, using VENDOR_VIDEOOUT_SYS_TIME struct
	VENDOR_VIDEOOUT_PARAM_LAYERMAPPING,     ///<support set/get, VENDOR_VIDEOOUT_LAYER_MAPPING_POS struct
	VENDOR_VIDEOOUT_PARAM_BLENDING,     	///<support set/get, VENDOR_VIDEOOUT_LAYER_MAPPING_POS struct
	VENDOR_VIDEOOUT_PARAM_MIXER,     		///<support set/get, VENDOR_VIDEOOUT_LAYER_MAPPING_POS struct
	VENDOR_VIDEOOUT_PARAM_PIP_OTYPE,     	///<support set/get, VENDOR_VIDEOOUT_PIP_OUTPUT_TYPE struct
	VENDOR_VIDEOOUT_PRAAM_PIP_DEVICEID,     ///<supprot set/get,  VENDOR_VIDEOOUT_PIP_DEV_ID struct
	VENDOR_VIDEOOUT_PRAAM_HDMI_IS_ON,       ///<supprot set/get,  unsigned int
	VENDOR_VIDEOOUT_PARAM_DRAW_LINE_INFO,   ///<supprot set/get,  VENDOR_VIDEOOUT_DRAW_LINE_INFO struct
	VENDOR_VIDEOOUT_PARAM_CLEAR_WIN,		///<supprot set,  VENDOR_VIDEOOUT_CLEAR_WIN struct
	VENDOR_VIDEOOUT_PARAM_OUT_STAMP_IMG,	///<supprot set,  VENDOR_OSG_STAMP_IMG struct
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_PARAM_ID)
} VENDOR_VIDEOOUT_PARAM_ID;

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoout_init(void);
HD_RESULT vendor_videoout_uninit(void);
HD_RESULT vendor_videoout_set(HD_PATH_ID path_id, VENDOR_VIDEOOUT_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoout_get(HD_PATH_ID path_id, VENDOR_VIDEOOUT_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoout_pull_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, INT32 wait_ms);
HD_RESULT vendor_videoout_push_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, HD_VIDEO_FRAME *p_user_out_video_frame, INT32 wait_ms);

#define FLOW_VENDOR_BIT        (27)
#define FLOW_VENDOR_FLAG       (0x1 << FLOW_VENDOR_BIT)
extern void hdal_flow_log_p(unsigned int flag, const char *msg_with_format, ...) __attribute__((format(printf, 2, 3)));

#define _HD_MODULE_PRINT_FLOW(flag, module, fmtstr, args...)  do { \
															hdal_flow_log_p(flag, fmtstr, ##args); \
													} while(0)
#define VENDOR_FLOW(fmtstr, args...) _HD_MODULE_PRINT_FLOW(FLOW_VENDOR_FLAG, VEN, fmtstr, ##args)

#ifdef  __cplusplus
}
#endif

#endif  /* _VENDOR_LCD_H_ */
