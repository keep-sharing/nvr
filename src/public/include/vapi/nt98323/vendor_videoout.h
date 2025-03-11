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
#define VENDOR_VIDEOOUT_VERSION 0x01000A //vendor lcd version
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
    INT16   dst_lcd_id; ///< only support cvbs 
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
    UINT32  wb_ba[5];	/* max is 5 piece of buffers */
	UINT16  piece_buffer_sz;
	UINT16  num_of_piece;
	UINT32  share_mem_ba;	/* must be 64 bytes alignment */
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
	VENDOR_VIDEOOUT_INPUT_DIM_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_INPUT_DIM)
} VENDOR_VIDEOOUT_INPUT_DIM;
typedef enum _VENDOR_VIDEOOUT_LCD_ID {
	VENDOR_VIDEOOUT_BT1120_1920X1080P   = 0,///< EXT VIDEO FORMAT IS BT1120_1920X1080
	VENDOR_VIDEOOUT_BT565_1280X720P ,       ///< EXT VIDEO FORMAT IS BT565_1280X720
	VENDOR_VIDEOOUT_BT1120_1280X720P ,      ///< EXT VIDEO FORMAT IS BT1120_1280X720
	VENDOR_VIDEOOUT_BT1120_1280X720P25 ,      ///< EXT VIDEO FORMAT IS BT1120_1280X720
	VENDOR_VIDEOOUT_LCD_NO_CHANGE  = 0xFFFE, ///< EXT VIDEO FORMAT USE CURRENT SETTING
	VENDOR_VIDEOOUT_LCD_MAX,
	ENUM_DUMMY4WORD(_VENDOR_VIDEOOUT_LCD_ID)
} VENDOR_VIDEOOUT_LCD_ID;
typedef enum _VENDOR_VIDEOOUT_HDMI_ID {
	VENDOR_VIDEOOUT_HDMI_1440X900P60    = 60,    ///< HDMI VIDEO FORMAT IS 1440X900 & PROGRESSIVE 60FPS,start from HD_VIDEOOUT_HDMI_720X480I240_16X9
	VENDOR_VIDEOOUT_HDMI_1680X1050P60   = 61,    ///< HDMI VIDEO FORMAT IS 1680X1050 & PROGRESSIVE 60FPS
	VENDOR_VIDEOOUT_HDMI_1920X1200P60   = 62,    ///< HDMI VIDEO FORMAT IS 1920X1200 & PROGRESSIVE 60FPS
	VENDOR_VIDEOOUT_HDMI_NO_CHANGE  = 0xFFFE, ///< HDMI VIDEO FORMAT USE CURRENT SETTING
	VENDOR_VIDEOOUT_HDMI_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_HDMI_ID)
} VENDOR_VIDEOOUT_HDMI_ID;
typedef enum _VENDOR_VIDEOOUT_VGA_ID {
	VENDOR_VIDEOOUT_VGA_800X600     = (HD_VIDEOOUT_VGA_MAX + 1),    ///< VGA VIDEO FORMAT IS 800X600
	VENDOR_VIDEOOUT_VGA_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_VGA_ID)
} VENDOR_VIDEOOUT_VGA_ID;
typedef struct _VENDOR_VIDEOOUT_MODE {
	HD_COMMON_VIDEO_OUT_TYPE output_type;    ///< select lcd output device
	VENDOR_VIDEOOUT_INPUT_DIM input_dim;         ///< NVR only.set input dim(IPC set input dim by HD_VIDEOOUT_PARAM_IN)
	union {
		VENDOR_VIDEOOUT_LCD_ID   lcd;  ///< set  ext output resolution
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

typedef struct _VENDOR_VIDEOOUT_TVE_ENABLE {
    unsigned int enable;
} VENDOR_VIDEOOUT_TVE_ENABLE;

typedef enum _VENDOR_VIDEOOUT_PARAM_ID {	
	VENDOR_VIDEOOUT_PARAM_WRITEBACK,        ///<support set/get, VENDOR_VIDEOOUT_WRITEBACK, for HDMI writeback to cvbs
	VENDOR_VIDEOOUT_PARAM_WIN_LAYER_ATTR,   ///<support set/get, VENDOR_VIDEOOUT_WIN_LAYER_ATTR, for win layer extension
	VENDOR_VIDEOOUT_PARAM_MODE,             ///<support get/set with ctrl path, using VENDOR_VIDEOOUT_MODE struct
	VENDOR_VIDEOOUT_PARAM_EDID,             ///<support get with ctrl path, using VENDOR_VIDEOOUT_EDID struct,set with unsigned int
	VENDOR_VIDEOOUT_PARAM_RLE_INTVAL,       ///<support get/set with ctrl path, using VENDOR_VIDEOOUT_RLE_INTVAL struct
	VENDOR_VIDEOOUT_PARAM_EDID_CAP,         ///<support get with ctrl path, using VENDOR_VIDEOOUT_EDID_CAP struct
	VENDOR_VIDEOOUT_PARAM_WRITEBACK_EXT,    ///<support set, VENDOR_VIDEOOUT_WRITEBACK_EXT, for HDMI writeback to cvbs with ring buf
	VENDOR_VIDEOOUT_PARAM_DISP_MAX_FPS,     ///<support set, using VENDOR_VIDEOOUT_DISP_MAX_FPS struct, 60fps or 30fps(default)
	VENDOR_VIDEOOUT_PARAM_TVE_ENABLE,       ///<support set, using VENDOR_VIDEOOUT_TVE_ENABLE
	ENUM_DUMMY4WORD(VENDOR_VIDEOOUT_PARAM_ID)
} VENDOR_VIDEOOUT_PARAM_ID;

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoout_set(HD_PATH_ID path_id, VENDOR_VIDEOOUT_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoout_get(HD_PATH_ID path_id, VENDOR_VIDEOOUT_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoout_set_cursor(void);
#define FLOW_VENDOR_BIT        (27)
#define FLOW_VENDOR_FLAG       (0x1 << FLOW_VENDOR_BIT)
extern void hdal_flow_log_p(unsigned int flag, const char *msg_with_format, ...) __attribute__((format(printf, 2, 3)));

#define _HD_MODULE_PRINT_FLOW(flag, module, fmtstr, args...)  do { \
															hdal_flow_log_p(flag, fmtstr, ##args); \
													} while(0)
#define VENDOR_FLOW(fmtstr, args...) _HD_MODULE_PRINT_FLOW(FLOW_VENDOR_FLAG, VEN, fmtstr, ##args)
#endif  /* _VENDOR_LCD_H_ */
