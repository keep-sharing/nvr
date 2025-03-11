/**
	@brief Header file of video output module.\n
	This file contains the functions which is related to video output in the chip.

	@file hd_videoout.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_VIDEOOUT_H_
#define _HD_VIDEOOUT_H_

/********************************************************************
    INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
/********************************************************************
    MACRO CONSTANT DEFINITIONS
********************************************************************/
#define HD_VIDEOOUT_MAX_IN  32 ///< max count of input of this device (interface)
#define HD_VIDEOOUT_MAX_OUT 1  ///< max count of output of this device (interface)
#define HD_FB_MAX           4  ///< max count of FB (interface)

/********************************************************************
    MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
    DRIVER RELATED TYPE DEFINITION
********************************************************************/

typedef enum _HD_VIDEOOUT_INPUT_DIM {
	HD_VIDEOOUT_IN_AUTO = 0,       ///< IN VIDEO FORMAT IS SAME AS output
	HD_VIDEOOUT_IN_640x480,        ///< IN VIDEO FORMAT IS 640x480
	HD_VIDEOOUT_IN_720x480,        ///< IN VIDEO FORMAT IS 720X480
	HD_VIDEOOUT_IN_720x576,        ///< IN VIDEO FORMAT IS 720x576
	HD_VIDEOOUT_IN_1024x768,       ///< IN VIDEO FORMAT IS 1024x768
	HD_VIDEOOUT_IN_800x600,        ///< IN VIDEO FORMAT IS 800x600
	HD_VIDEOOUT_IN_1280x720,       ///< IN VIDEO FORMAT IS 1280x720
	HD_VIDEOOUT_IN_1280x1024,      ///< IN VIDEO FORMAT IS 1280x1024
	HD_VIDEOOUT_IN_1600x1200,      ///< IN VIDEO FORMAT IS 1600x1200
	HD_VIDEOOUT_IN_2560x720,       ///< IN VIDEO FORMAT IS 2560x720
	HD_VIDEOOUT_IN_2560x1440,      ///< IN VIDEO FORMAT IS 2560x1440
	HD_VIDEOOUT_IN_1920x1080,      ///< IN VIDEO FORMAT IS 1920x1080
	HD_VIDEOOUT_IN_3840x1080,      ///< IN VIDEO FORMAT IS 3840x1080
	HD_VIDEOOUT_IN_3840x2160,      ///< IN VIDEO FORMAT IS 3840x2160
	HD_VIDEOOUT_IN_1440x900,       ///< IN VIDEO FORMAT IS 1440x900
	HD_VIDEOOUT_IN_1680x1050,      ///< IN VIDEO FORMAT IS 1680x1050
	HD_VIDEOOUT_IN_1920x1200,      ///< IN VIDEO FORMAT IS 1920x1200
	HD_VIDEOOUT_INPUT_DIM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOOUT_INPUT_DIM)
} HD_VIDEOOUT_INPUT_DIM;

typedef enum _HD_VIDEOOUT_LCD_ID {
	HD_VIDEOOUT_LCD_0,          ///< LCD ID 0
	HD_VIDEOOUT_LCD_1,          ///< LCD ID 1
	HD_VIDEOOUT_LCD_2,          ///< LCD ID 2
	HD_VIDEOOUT_LCD_3,          ///< LCD ID 3
	HD_VIDEOOUT_LCD_4,          ///< LCD ID 4
	ENUM_DUMMY4WORD(HD_VIDEOOUT_LCD_ID)
} HD_VIDEOOUT_LCD_ID;

typedef enum _HD_VIDEOOUT_HDMI_ID {
	HD_VIDEOOUT_HDMI_640X480P60         = 1,    ///< HDMI VIDEO FORMAT IS 640X480 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_720X480P60         = 2,    ///< HDMI VIDEO FORMAT IS 720X480 & PROGRESSIVE 60FPS & 4:3
	HD_VIDEOOUT_HDMI_720X480P60_16X9    = 3,    ///< HDMI VIDEO FORMAT IS 720X480 & PROGRESSIVE 60FPS & 16:9
	HD_VIDEOOUT_HDMI_1280X720P60        = 4,    ///< HDMI VIDEO FORMAT IS 1280X720 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_1920X1080I60       = 5,    ///< HDMI VIDEO FORMAT IS 1920X1080 & INTERLACED 60FPS
	HD_VIDEOOUT_HDMI_720X480I60         = 6,    ///< HDMI VIDEO FORMAT IS 720X480 & INTERLACED 60FPS
	HD_VIDEOOUT_HDMI_720X480I60_16X9    = 7,    ///< HDMI VIDEO FORMAT IS 720X480 & INTERLACED 60FPS & 16:9
	HD_VIDEOOUT_HDMI_720X240P60         = 8,    ///< HDMI VIDEO FORMAT IS 720X240 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_720X240P60_16X9    = 9,    ///< HDMI VIDEO FORMAT IS 720X240 & PROGRESSIVE 60FPS & 16:9
	HD_VIDEOOUT_HDMI_1440X480I60        = 10,   ///< HDMI VIDEO FORMAT IS 1440X480 & INTERLACED 60FPS
	HD_VIDEOOUT_HDMI_1440X480I60_16X9   = 11,   ///< HDMI VIDEO FORMAT IS 1440X480 & INTERLACED 60FPS & 16:9
	HD_VIDEOOUT_HDMI_1440X240P60        = 12,   ///< HDMI VIDEO FORMAT IS 1440X240 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_1440X240P60_16X9   = 13,   ///< HDMI VIDEO FORMAT IS 1440X240 & PROGRESSIVE 60FPS & 16:9
	HD_VIDEOOUT_HDMI_1440X480P60        = 14,   ///< HDMI VIDEO FORMAT IS 1440X480 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_1440X480P60_16X9   = 15,   ///< HDMI VIDEO FORMAT IS 1440X480 & PROGRESSIVE 60FPS & 16:9
	HD_VIDEOOUT_HDMI_1920X1080P60       = 16,   ///< HDMI VIDEO FORMAT IS 1920X1080 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_720X576P50         = 17,   ///< HDMI VIDEO FORMAT IS 720X576 & PROGRESSIVE 50FPS
	HD_VIDEOOUT_HDMI_720X576P50_16X9    = 18,   ///< HDMI VIDEO FORMAT IS 720X576 & PROGRESSIVE 50FPS & 16:9
	HD_VIDEOOUT_HDMI_1280X720P50        = 19,   ///< HDMI VIDEO FORMAT IS 1280X720 & PROGRESSIVE 50FPS
	HD_VIDEOOUT_HDMI_1920X1080I50       = 20,   ///< HDMI VIDEO FORMAT IS 1920X1080 & INTERLACED 50FPS
	HD_VIDEOOUT_HDMI_720X576I50         = 21,   ///< HDMI VIDEO FORMAT IS 720X576 & INTERLACED 50FPS
	HD_VIDEOOUT_HDMI_720X576I50_16X9    = 22,   ///< HDMI VIDEO FORMAT IS 720X576 & INTERLACED 50FPS & 16:9
	HD_VIDEOOUT_HDMI_720X288P50         = 23,   ///< HDMI VIDEO FORMAT IS 720X288 & PROGRESSIVE 50FPS
	HD_VIDEOOUT_HDMI_720X288P50_16X9    = 24,   ///< HDMI VIDEO FORMAT IS 720X288 & PROGRESSIVE 50FPS & 16:9
	HD_VIDEOOUT_HDMI_1440X576I50        = 25,   ///< HDMI VIDEO FORMAT IS 1440X576 & INTERLACED 50FPS
	HD_VIDEOOUT_HDMI_1440X576I50_16X9   = 26,   ///< HDMI VIDEO FORMAT IS 1440X576 & INTERLACED 50FPS & 16:9
	HD_VIDEOOUT_HDMI_1440X288P50        = 27,   ///< HDMI VIDEO FORMAT IS 1440X288 & PROGRESSIVE 50FPS
	HD_VIDEOOUT_HDMI_1440X288P50_16X9   = 28,   ///< HDMI VIDEO FORMAT IS 1440X288 & PROGRESSIVE 50FPS & 16:9
	HD_VIDEOOUT_HDMI_1440X576P50        = 29,   ///< HDMI VIDEO FORMAT IS 1440X576 & PROGRESSIVE 50FPS
	HD_VIDEOOUT_HDMI_1440X576P50_16X9   = 30,   ///< HDMI VIDEO FORMAT IS 1440X576 & PROGRESSIVE 50FPS & 16:9
	HD_VIDEOOUT_HDMI_1920X1080P50       = 31,   ///< HDMI VIDEO FORMAT IS 1920X1080 & PROGRESSIVE 50FPS
	HD_VIDEOOUT_HDMI_1920X1080P24       = 32,   ///< HDMI VIDEO FORMAT IS 1920X1080 & PROGRESSIVE 24FPS
	HD_VIDEOOUT_HDMI_1920X1080P25       = 33,   ///< HDMI VIDEO FORMAT IS 1920X1080 & PROGRESSIVE 25FPS
	HD_VIDEOOUT_HDMI_1920X1080P30       = 34,   ///< HDMI VIDEO FORMAT IS 1920X1080 & PROGRESSIVE 30FPS
	HD_VIDEOOUT_HDMI_1920X1080I50_VT1250 = 39,  ///< HDMI VIDEO FORMAT IS 1920X1080 & INTERLACED  50FPS & V-TOTAL IS 1250 LINES
	HD_VIDEOOUT_HDMI_1920X1080I100      = 40,   ///< HDMI VIDEO FORMAT IS 1920X1080 & INTERLACED  100FPS
	HD_VIDEOOUT_HDMI_1280X720P100       = 41,   ///< HDMI VIDEO FORMAT IS 1280X720  & PROGRESSIVE 100FPS
	HD_VIDEOOUT_HDMI_720X576P100        = 42,   ///< HDMI VIDEO FORMAT IS 720X576   & PROGRESSIVE 100FPS
	HD_VIDEOOUT_HDMI_720X576P100_16X9   = 43,   ///< HDMI VIDEO FORMAT IS 720X576   & PROGRESSIVE 100FPS & 16:9
	HD_VIDEOOUT_HDMI_720X576I100        = 44,   ///< HDMI VIDEO FORMAT IS 720X576  & INTERLACED 100FPS
	HD_VIDEOOUT_HDMI_720X576I100_16X9   = 45,   ///< HDMI VIDEO FORMAT IS 720X576  & INTERLACED 100FPS & 16:9
	HD_VIDEOOUT_HDMI_1920X1080I120      = 46,   ///< HDMI VIDEO FORMAT IS 1920X1080 & INTERLACED 120FPS
	HD_VIDEOOUT_HDMI_1280X720P120       = 47,   ///< HDMI VIDEO FORMAT IS 1280X720  & PROGRESSIVE 120FPS
	HD_VIDEOOUT_HDMI_720X480P120        = 48,   ///< HDMI VIDEO FORMAT IS 720X480   & PROGRESSIVE 120FPS
	HD_VIDEOOUT_HDMI_720X480P120_16X9   = 49,   ///< HDMI VIDEO FORMAT IS 720X480   & PROGRESSIVE 120FPS & 16:9
	HD_VIDEOOUT_HDMI_720X480I120        = 50,   ///< HDMI VIDEO FORMAT IS 720X480  & INTERLACED 120FPS
	HD_VIDEOOUT_HDMI_720X480I120_16X9   = 51,   ///< HDMI VIDEO FORMAT IS 720X480  & INTERLACED 120FPS & 16:9
	HD_VIDEOOUT_HDMI_720X576P200        = 52,   ///< HDMI VIDEO FORMAT IS 720X576  & PROGRESSIVE 200FPS
	HD_VIDEOOUT_HDMI_720X576P200_16X9   = 53,   ///< HDMI VIDEO FORMAT IS 720X576  & PROGRESSIVE 200FPS & 16:9
	HD_VIDEOOUT_HDMI_720X576I200        = 54,   ///< HDMI VIDEO FORMAT IS 720X576  & INTERLACED 200FPS
	HD_VIDEOOUT_HDMI_720X576I200_16X9   = 55,   ///< HDMI VIDEO FORMAT IS 720X576  & INTERLACED 200FPS & 16:9
	HD_VIDEOOUT_HDMI_720X480P240        = 56,   ///< HDMI VIDEO FORMAT IS 720X480  & PROGRESSIVE 240FPS
	HD_VIDEOOUT_HDMI_720X480P240_16X9   = 57,   ///< HDMI VIDEO FORMAT IS 720X480  & PROGRESSIVE 240FPS & 16:9
	HD_VIDEOOUT_HDMI_720X480I240        = 58,   ///< HDMI VIDEO FORMAT IS 720X480  & INTERLACED 240FPS
	HD_VIDEOOUT_HDMI_720X480I240_16X9   = 59,   ///< HDMI VIDEO FORMAT IS 720X480  & INTERLACED 240FPS & 16:9
	HD_VIDEOOUT_HDMI_1440X900P60        = 60,   ///< HDMI VIDEO FORMAT IS 1440X900 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_1680X1050P60       = 61,   ///< HDMI VIDEO FORMAT IS 1680X1050 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_1920X1200P60       = 62,   ///< HDMI VIDEO FORMAT IS 1920X1200 & PROGRESSIVE 60FPS
	HD_VIDEOOUT_HDMI_1920X1200P75       = 63,   ///< HDMI VIDEO FORMAT IS 1920X1200 & PROGRESSIVE 75FPS

	HD_VIDEOOUT_HDMI_3840X2160P24       = 93,   ///< HDMI VIDEO FORMAT IS 3840X2160 & PROGRESSIVE 24FPS
	HD_VIDEOOUT_HDMI_3840X2160P25       = 94,   ///< HDMI VIDEO FORMAT IS 3840X2160 & PROGRESSIVE 25FPS
	HD_VIDEOOUT_HDMI_3840X2160P30       = 95,   ///< HDMI VIDEO FORMAT IS 3840X2160 & PROGRESSIVE 30FPS
	HD_VIDEOOUT_HDMI_4096X2160P24       = 98,   ///< HDMI VIDEO FORMAT IS 4096X2160 & PROGRESSIVE 24FPS
	HD_VIDEOOUT_HDMI_4096X2160P25       = 99,   ///< HDMI VIDEO FORMAT IS 4096X2160 & PROGRESSIVE 25FPS
	HD_VIDEOOUT_HDMI_4096X2160P30       = 100,  ///< HDMI VIDEO FORMAT IS 4096X2160 & PROGRESSIVE 30FPS

	// Non-std HDMI formats
	HD_VIDEOOUT_HDMI_1024X768P60        = 35 + 256, ///< HDMI VIDEO FORMAT IS 1024X768 & Progressive 60fps
	HD_VIDEOOUT_HDMI_1280X1024P30       = 36 + 256, ///< HDMI VIDEO FORMAT IS 1280X1024 & PROGRESSIVE 30FPS
	HD_VIDEOOUT_HDMI_1280X1024P60       = 37 + 256, ///< HDMI VIDEO FORMAT IS 1280X1024 & Progressive 60fps
	HD_VIDEOOUT_HDMI_1600X1200P30       = 38 + 256, ///< HDMI VIDEO FORMAT IS 1600X1200 & PROGRESSIVE 30FPS
	HD_VIDEOOUT_HDMI_1600X1200P60       = 39 + 256, ///< HDMI VIDEO FORMAT IS 1600X1200 & Progressive 60fps
	HD_VIDEOOUT_HDMI_2560X1440P30       = 40 + 256, ///< HDMI VIDEO FORMAT IS 2560X1440 & PROGRESSIVE 30FPS
	HD_VIDEOOUT_HDMI_2560X1440P60       = 41 + 256, ///< HDMI VIDEO FORMAT IS 2560X1440 & Progressive 60fps

	HD_VIDEOOUT_HDMI_NO_CHANGE          = 0xFFFE, ///< HDMI VIDEO FORMAT USE CURRENT SETTING
	HD_VIDEOOUT_HDMI_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOOUT_HDMI_ID)
} HD_VIDEOOUT_HDMI_ID;

typedef enum _HD_VIDEOOUT_VGA_ID {
	HD_VIDEOOUT_VGA_720X480     = 0,   ///< VGA VIDEO FORMAT IS 720X480
	HD_VIDEOOUT_VGA_1024X768,          ///< VGA VIDEO FORMAT IS 1024X768
	HD_VIDEOOUT_VGA_1280X720,          ///< VGA VIDEO FORMAT IS 1280X720
	HD_VIDEOOUT_VGA_1280X1024,         ///< VGA VIDEO FORMAT IS 1280X1024
	HD_VIDEOOUT_VGA_1600X1200,         ///< VGA VIDEO FORMAT IS 1600X1200
	HD_VIDEOOUT_VGA_1920X1080,         ///< VGA VIDEO FORMAT IS 1920X1080
	HD_VIDEOOUT_VGA_1440X900,          ///< VGA VIDEO FORMAT IS 1440X900
	HD_VIDEOOUT_VGA_1680X1050,         ///< VGA VIDEO FORMAT IS 1680X1050
	HD_VIDEOOUT_VGA_1920X1200,         ///< VGA VIDEO FORMAT IS 1920X1200
	HD_VIDEOOUT_VGA_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOOUT_VGA_ID)
} HD_VIDEOOUT_VGA_ID;

typedef enum _HD_VIDEOOUT_CVBS_ID {
	HD_VIDEOOUT_CVBS_NTSC                = 0,          ///< CVBS MODE IS NTSC
	HD_VIDEOOUT_CVBS_PAL                 = 1,          ///< CVBS MODE IS NTSC
	HD_VIDEOOUT_CVBS_NTSC_SD             = 2,          ///< CVBS MODE IS SD NTSC
	HD_VIDEOOUT_CVBS_PAL_SD              = 3,          ///< CVBS MODE IS SD PAL
	HD_VIDEOOUT_CVBS_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOOUT_CVBS_ID)
} HD_VIDEOOUT_CVBS_ID;

typedef struct _HD_VIDEOOUT_MODE {
	HD_COMMON_VIDEO_OUT_TYPE output_type;    ///< select lcd output device
	HD_VIDEOOUT_INPUT_DIM input_dim;         ///< set input dim
	union {
		HD_VIDEOOUT_LCD_ID lcd;    ///< set lcd  output resolution
		HD_VIDEOOUT_HDMI_ID hdmi;  ///< set hdmi  output resolution
		HD_VIDEOOUT_VGA_ID vga;    ///< set vga  output resolution
		HD_VIDEOOUT_CVBS_ID cvbs;  ///< set cvbs  output resolution
	} output_mode;
} HD_VIDEOOUT_MODE;

typedef struct _HD_VIDEOOUT_DEVNVR_CONFIG {
	HD_VIDEOOUT_MODE mode;           ///< set lcd mode
	int homology;                    ///< only suitable to lcd210. When it is set, it will not register to videograph
	int chip_state;                  ///< only suitable to lcd210. to enable/disable lcd200_0 hw
	struct {
		int max_w;                  ///< fb max width
		int max_h;                  ///< fb max height
		int max_bpp;                ///< fb max bit per pixel
		int gui_rld_enable;         ///< rld enabled
		int ddr_id;                 ///< fb buf ddr id
		unsigned int buf_paddr;     ///< fb buffer physical address
		unsigned int buf_len;       ///< fb buffer length
		unsigned int rle_buf_paddr; ///< rle buffer physical address, only valid for gui compress
		unsigned int rle_buf_len;   ///< rle buffer length, only valid for gui compress
	} plane[3];
} HD_VIDEOOUT_DEVNVR_CONFIG;

typedef enum _HD_VIDEOOUT_INFUNC {
	HD_VIDEOOUT_INFUNC_ONEBUF =     0x00000002, ///< enable one-buffer from vdoprc
	ENUM_DUMMY4WORD(HD_VIDEOOUT_INFUNC)
} HD_VIDEOOUT_INFUNC;

typedef struct _HD_VIDEOOUT_DEVIPC_CONFIG {
} HD_VIDEOOUT_DEVIPC_CONFIG;

typedef struct _HD_VIDEOOUT_CONFIG {
	union {
		HD_VIDEOOUT_DEVNVR_CONFIG  devnvr_cfg;   ///< device config
		HD_VIDEOOUT_DEVIPC_CONFIG  devipc_cfg;   ///< device config
	};
} HD_VIDEOOUT_DEV_CONFIG;

/**
	@name func config
*/
typedef struct _HD_VIDEOOUT_FUNC_CONFIG {
	UINT32 ddr_id;                      ///< DDR ID
	HD_VIDEOOUT_INFUNC in_func;         ///< additional function of in (bit-wise mask)
} HD_VIDEOOUT_FUNC_CONFIG;
/********************************************************************
    TYPE DEFINITION
********************************************************************/

/**
     @name HD_VIDEOOUT capability
*/
typedef enum HD_VIDEOOUT_INCAPS {
	HD_VIDEOOUT_INCAPS_ONEBUF =     0x02000000, ///< caps of HD_VIDEOOUT_INFUNC_ONEBUF
	ENUM_DUMMY4WORD(HD_VIDEOOUT_INCAPS)
} HD_VIDEOOUT_INCAPS;

typedef enum _HD_VIDEOOUT_CAPS {
	HD_VIDEOOUT_CAPS_CVBS           = 0x00000100, ///< support cvbs display
	HD_VIDEOOUT_CAPS_VGA            = 0x00000200, ///< support vga display
	HD_VIDEOOUT_CAPS_HDMI           = 0x00000400, ///< support hdmi display
	HD_VIDEOOUT_CAPS_LCD            = 0x00000800, ///< support lcd display
	ENUM_DUMMY4WORD(HD_VIDEOOUT_CAPS)
} HD_VIDEOOUT_CAPS;

typedef enum _HD_FB_CAPS {
	HD_FB_CAPS_YUV422           = 0x00000001, ///< support YUV422 format
	HD_FB_CAPS_YUV420           = 0x00000002, ///< support YUV420 format
	HD_FB_CAPS_INDEX_1          = 0x00000010, ///< support INDEX 1bit format
	HD_FB_CAPS_INDEX_2          = 0x00000020, ///< support INDEX 2bit format
	HD_FB_CAPS_INDEX_4          = 0x00000040, ///< support INDEX 4bit format
	HD_FB_CAPS_INDEX_8          = 0x00000080, ///< support INDEX 8bit format
	HD_FB_CAPS_ARGB_1555        = 0x00000100, ///< support ARGB 1555 format
	HD_FB_CAPS_ARGB_4444        = 0x00000200, ///< support ARGB 4444 format
	HD_FB_CAPS_ARGB_8888        = 0x00000400, ///< support ARGB 8888 format
	HD_FB_CAPS_ARGB_8565        = 0x00000800, ///< support ARGB 8565 format
	HD_FB_CAPS_RGBA_5551        = 0x00001000, ///< support RGBA 5551 format
	HD_FB_CAPS_RGB_565          = 0x00002000, ///< support RGB 565 format
	ENUM_DUMMY4WORD(HD_FB_CAPS)
} HD_FB_CAPS;

typedef struct _HD_VIDEOOUT_SYSCAPS {
	HD_DAL dev_id;                  ///< device id
	UINT32 chip_id;                 ///< chip id of this device
	UINT32 max_in_count;            ///< max count of input of this device
	UINT32 max_out_count;           ///< max count of output of this device
	UINT32 max_fb_count;            ///< max count of fb of this device
	HD_DEVICE_CAPS dev_caps;        ///< support value:HD_VIDEOOUT_CAPS|HD_DEVICE_CAPS
	HD_VIDEO_CAPS in_caps[HD_VIDEOOUT_MAX_IN];     ///< capability of input of HD_VIDEO_CAPS and HD_VIDEOOUT_INCAPS
	HD_VIDEO_CAPS out_caps[HD_VIDEOOUT_MAX_OUT];   ///< capability of output
	UINT32 fb_caps[HD_FB_MAX];      ///< capability of fb, support value:HD_FB_CAPS
	HD_DIM output_dim;              ///< output dimension width/height
	HD_DIM input_dim;               ///< input dimension width/height
	UINT32 fps;                     ///< display framw rate
	UINT32 max_scale_up_w;          ///< max width scling up factor
	UINT32 max_scale_up_h;          ///< max height scling up factor
	UINT32 max_scale_down_w;        ///< max width scaling down factor
	UINT32 max_scale_down_h;        ///< max height scaling down factor
	UINT32 max_out_stamp;           ///< max output stamp
	UINT32 max_out_stamp_ex;        ///< max output stamp_ex
	UINT32 max_out_mask;            ///< max output mask
	UINT32 max_out_mask_ex;         ///< max output mask_ex
} HD_VIDEOOUT_SYSCAPS;

typedef enum _HD_FB_ID {
	HD_FB0 = 0,     ///< frame buf id 0
	HD_FB1,         ///< frame buf id 1
	HD_FB2,         ///< frame buf id 2
	HD_FB3,         ///< frame buf id 3
	ENUM_DUMMY4WORD(HD_FB_ID)
} HD_FB_ID;

typedef enum _CLEAR_WINDOW_MODE{
	HD_ACTIVE_BY_APPLY = 0,     ///< clear videoout win input data after set HD_VIDEOOUT_PARAM_MODE
	HD_ACTIVE_IMMEDIATELY,      ///< clear videoout win input data immediately
} CLEAR_WINDOW_MODE;

typedef struct _HD_VIDEOOUT_CLEAR_WIN {
	HD_DIM input_dim;         ///< pattern buf dim, minimun input dim: 64x32
	HD_VIDEO_PXLFMT in_fmt;   ///< pattern fmt
	UINT8 *buf;               ///< pattern buf
	HD_URECT output_rect;     ///< clear win region
	CLEAR_WINDOW_MODE mode;   ///< clear win mode
} HD_VIDEOOUT_CLEAR_WIN;

typedef struct _HD_FB_FMT {
	HD_FB_ID fb_id;         ///< select fb layer
	HD_VIDEO_PXLFMT fmt;    ///< set fb input fmt
} HD_FB_FMT;

typedef struct _HD_FB_ATTR {
	HD_FB_ID   fb_id;      ///< select fb layer
	UINT8   alpha_blend;   ///< 0~255,except ARBG format,ARBG format would refer pixel alpha,lcd0:support fb0~2,lcd_1/lcd_2:fb0~1
	UINT8   alpha_1555;    ///< 0~255,only for ARBG1555 format,the pixel alpha 1 would refer the value
	BOOL    colorkey_en;   ///< 0:disable/1:enable
	UINT16  r_ckey;        ///< r would be format r,ex:ARGB444 format Red:0xF00
	UINT16  g_ckey;        ///< g would be format g,ex:ARGB444 format Green:0x0F0
	UINT16  b_ckey;        ///< b would be format b,ex:ARGB444 format Blue:0x00F
} HD_FB_ATTR;

typedef struct _HD_FB_ENABLE {
	HD_FB_ID fb_id;    ///< select fb layer
	BOOL enable;       ///< 0:disable, 1:enable
} HD_FB_ENABLE;

typedef struct _HD_FB_LAYER_ORDER {
	HD_FB_ID fb_id;         ///< select fb layer
	UINT8 fb_to_plane_num;  ///< 0~2
} HD_FB_LAYER_ORDER;

typedef struct _HD_FB_DIM {
	HD_FB_ID fb_id;         ///< select fb layer
	HD_DIM input_dim;       ///< set fb in dim
	HD_URECT output_rect;   ///< set fb out dim and x/y
} HD_FB_DIM;

typedef struct _HD_FB_PALETTE_TBL {
	HD_FB_ID  fb_id;    ///< select fb layer
	UINT16  table_size; ///< palette size
	UINT32  *p_table;   ///< palette table value
} HD_FB_PALETTE_TBL;

#define HD_VIDEOOUT_PALETTE_TABLE HD_FB_PALETTE_TBL

typedef enum _HD_WIN_LAYER {
	HD_LAYER1 = 0,         ///< 1st layer (background, first draw, lowest)
	HD_LAYER2 = 1,             ///< 2nd layer
	HD_LAYER3 = 2,             ///< 3rd layer
	HD_LAYER4 = 3,             ///< 4th layer
	HD_LAYER5 = 4,             ///< 5th layer
	HD_LAYER6 = 5,             ///< 6th layer
	HD_LAYER7 = 6,             ///< 7th layer
	HD_LAYER8 = 7,             ///< 8th layer (last draw, on the top)
	HD_WIN_LAYER_MAX,
	ENUM_DUMMY4WORD(HD_WIN_LAYER)
} HD_WIN_LAYER;

typedef struct _HD_VIDEOOUT_WIN_ATTR {
	BOOL visible;   ///< decide this win to show
	HD_URECT rect;  ///< set lcd input port dim
	HD_WIN_LAYER layer;
} HD_VIDEOOUT_WIN_ATTR;


typedef struct _HD_VIDEOOUT_WIN_PSR_ATTR {
	BOOL visible;   ///< decide the Picture-Second-Region's win to show
	HD_URECT rect;  ///< set lcd input port dim
} HD_VIDEOOUT_WIN_PSR_ATTR;

typedef struct _HD_VIDEOOUT_IN {
	HD_DIM dim;               ///< dimension
	HD_VIDEO_PXLFMT pxlfmt;   ///< format
	HD_VIDEO_DIR dir;         ///< direction like mirror/flip
} HD_VIDEOOUT_IN;

typedef struct {
	HD_VIDEOOUT_HDMI_ID video_id[32]; ///< supported HDMI EDID.
	UINT32 len;                        ///< Number of video formats supported by the HDMI.
} HD_VIDEOOUT_HDMI_ABILITY;

typedef enum _HD_VIDEOOUT_PARAM_ID {
	///Common Start:all members of struct //
	HD_VIDEOOUT_PARAM_DEVCOUNT  = 0,          ///< support get with ctrl path, using HD_DEVCOUNT struct
	HD_VIDEOOUT_PARAM_SYSCAPS,                ///< support get with ctrl path, using HD_VIDEOOUT_SYSCAPS struct
	HD_VIDEOOUT_PARAM_DEV_CONFIG,             ///< support set with ctrl path, using HD_VIDEOOUT_DEV_CONFIG struct
	HD_VIDEOOUT_PARAM_MODE,                   ///< support get/set with ctrl path, using HD_VIDEOOUT_MODE struct
	HD_VIDEOOUT_PARAM_FB_PALETTE_TABLE,       ///< support get/set with ctrl path, using HD_FB_PALETTE_TBL struct
	HD_VIDEOOUT_PARAM_CLEAR_WIN,              ///< support set with ctrl path, using HD_VIDEOOUT_CLEAR_WIN struct, set active
	HD_VIDEOOUT_PARAM_IN,                     ///< support get/set with i path, using HD_VIDEOOUT_IN struct
	HD_VIDEOOUT_PARAM_IN_WIN_ATTR,            ///< support get/set with i path, using HD_VIDEOOUT_WIN_ATTR struct
	HD_VIDEOOUT_PARAM_IN_WIN_PSR_ATTR,        ///< support get/set with i path, using HD_VIDEOOUT_WIN_PSR_ATTR struct
	HD_VIDEOOUT_PARAM_WRITEBACK_BUF,          ///< support get/set with ctrl path,  using HD_VIDEO_FRAME struct
	HD_VIDEOOUT_PARAM_FB_FMT,                 ///< support get/set with ctrl path,  using HD_FB_FMT struct
	HD_VIDEOOUT_PARAM_FB_ATTR,                ///< support get/set with ctrl path,  using HD_FB_ATTR struct
	HD_VIDEOOUT_PARAM_FB_ENABLE,              ///< support get/set with ctrl path,  using HD_FB_ENABLE struct
	HD_VIDEOOUT_PARAM_FB_LAYER_ORDER,         ///< support get/set with ctrl path,  using HD_FB_LAYER_ORDER struct
	HD_VIDEOOUT_PARAM_HDMI_ABILITY,           ///< support get with ctrl path, using HD_VIDEOOUT_HDMI_ABILITY struct
	HD_VIDEOOUT_PARAM_OUT_STAMP_BUF,          ///< support get/set with o/stamp path, using HD_OSG_STAMP_BUF struct (stamp buffer parameter)
	HD_VIDEOOUT_PARAM_OUT_STAMP_IMG,          ///< support get/set with o/stamp path, using HD_OSG_STAMP_IMG struct (stamp image parameter)
	HD_VIDEOOUT_PARAM_OUT_STAMP_ATTR,         ///< support get/set with o/stamp path, using HD_OSG_STAMP_ATTR struct (stamp display attribute)
	HD_VIDEOOUT_PARAM_OUT_MASK_ATTR,          ///< support get/set with o/mask path, using HD_OSG_MASK_ATTR struct (mask display attribute)
	HD_VIDEOOUT_PARAM_OUT_MOSAIC_ATTR,        ///< support get/set with o/mask path, using HD_OSG_MOSAIC_ATTR struct (mosaic display attribute)
	HD_VIDEOOUT_PARAM_FB_DIM,                 ///< support get/set with ctrl path,  using HD_FB_DIM struct
	HD_VIDEOOUT_PARAM_OUT_PALETTE_TABLE,      ///< support get/set with o path, using HD_PALETTE_TBL struct
	HD_VIDEOOUT_PARAM_FUNC_CONFIG,            ///< support get/set with i/o path, using HD_VIDEOOUT_FUNC_CONFIG struct (path func config)
	///Common End//

	HD_VIDEOOUT_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOOUT_PARAM_ID)
} HD_VIDEOOUT_PARAM_ID;

#define HD_VIDEOOUT_PARAM_PALETTE_TABLE HD_VIDEOOUT_PARAM_FB_PALETTE_TABLE

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT hd_videoout_init(VOID);
HD_RESULT hd_videoout_bind(HD_OUT_ID out_id, HD_IN_ID dest_in_id);
HD_RESULT hd_videoout_unbind(HD_OUT_ID out_id);
HD_RESULT hd_videoout_open(HD_IN_ID in_id, HD_OUT_ID out_id, HD_PATH_ID *p_path_id);

HD_RESULT hd_videoout_start(HD_PATH_ID path_id);
HD_RESULT hd_videoout_stop(HD_PATH_ID path_id);
HD_RESULT hd_videoout_start_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_videoout_stop_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_videoout_get(HD_PATH_ID path_id, HD_VIDEOOUT_PARAM_ID id, VOID *param);
HD_RESULT hd_videoout_set(HD_PATH_ID path_id, HD_VIDEOOUT_PARAM_ID id, VOID *param);
HD_RESULT hd_videoout_push_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_in_video_frame, HD_VIDEO_FRAME *p_user_out_video_frame, INT32 wait_ms);
HD_RESULT hd_videoout_pull_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, INT32 wait_ms);
HD_RESULT hd_videoout_release_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame);
HD_RESULT hd_videoout_close(HD_PATH_ID path_id);
HD_RESULT hd_videoout_uninit(VOID);


#define ARGB8565_A_ADDR(rgb_addr, w, h) ALIGN_CEIL_16(rgb_addr + w*2*h) //RGB565 buf size = (line offset * height)

#endif

