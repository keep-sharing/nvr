/**
@brief Header file of vendor videoproc
This file contains the functions which is related to videoproc in the chip.

@file vendor_videoenc.h

@ingroup mhdal

@note Nothing.

Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef __VENDOR_PROC_H__
#define __VENDOR_PROC_H__

#define VENDOR_VIDEOPROC_VERSION 0x010010 //vendor videoproc version
/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
#include "hd_videoprocess.h"

#ifdef  __cplusplus
extern "C" {
#endif

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDOR_GEO_LUT_X	65
#define VENDOR_GEO_LUT_Y	68
#define VENDOR_GEO_LUT		(VENDOR_GEO_LUT_X * VENDOR_GEO_LUT_Y)
/**
	@name VENDOR_VIDEOPROC_FLAG for VENDOR_VIDEOPROC_USER_FLAG
*/
/********************************************************************
TYPE DEFINITION
********************************************************************/

typedef struct _VENDOR_VDOPROC_STATUS {
	UINT32 dti_buf_cnt;								///< datain available buffer count
} VENDOR_VDOPROC_STATUS;

/**
	@name VENDOR_VDOPROC_USER_SUB_RATIO for videoprocess

	As a sub-yuv ratio threshold for library level
*/
typedef struct _VENDOR_VDOPROC_SUB_RATIO_THLD {
	UINT32 numer;									///< numerator of sub-yuv ratio threshold
	UINT32 denom;									///< denominator of sub-yuv ratio threshold
} VENDOR_VDOPROC_SUB_RATIO_THLD;

/**
     @name push list
*/
typedef struct _VENDOR_VIDEOPROC_PUSH_LIST {
	HD_PATH_ID *p_path_id;              ///< NVR only. path id
	HD_VIDEO_FRAME *p_in_video_frame;   ///< NVR only. input video frame
	HD_VIDEO_FRAME *p_out_video_frame;	///< NVR only. output video frame
	HD_DIM *p_llc_dim;					///< NVR only. dim of LLC pxlfmt
} VENDOR_VIDEOPROC_PUSH_LIST;

typedef struct _VENDOR_VDOPROC_SLICE_MODE {
	BOOL enable;
	unsigned int slice_height;	//VPE support 4K output without column mode
} VENDOR_VDOPROC_SLICE_MODE;

typedef struct _VENDOR_VDOPROC_UV_SWAP {
	UINT8 uv_swap; //VPE swap uv data, 0:false, 1:true
} VENDOR_VDOPROC_UV_SWAP;

typedef struct _VENDOR_VDOPROC_IN_POOL {
	HD_VIDEOPROC_POOL data_pool[HD_VP_MAX_DATA_TYPE]; //HD_VIDEOPROC_POOL_ENABLE mode: set datain pool from vendor api
	                                                  //HD_VIDEOPROC_POOL_AUTO mode  : set datain pool from vpe pool config
} VENDOR_VDOPROC_IN_POOL;

typedef enum _VENDOR_VIDEOPROC_PARAM_ID {
	VENDOR_VIDEOPROC_STATUS,				///<  support get     for i/o path
	VENDOR_VIDEOPROC_USER_FLAG,             ///<  support set 	  for i/o path, using VENDOR_VIDEOPROC_FLAG definitions
	VENDOR_VIDEOPROC_SUB_RATIO_THLD,        ///<  support set/get for i/o path, using VENDOR_VDOPROC_SUB_RATIO_THLD struct
	VENDOR_VIDEOPROC_DEWARP_CTRL,			///<  support set/get for i/o path, using VENDOR_DEWARP_CTRL struct
	VENDOR_VIDEOPROC_DEWARP_IMPROVE_CTRL,	///<  support set/get for i/o path, using VENDOR_DEWARP_TMNR_CTRL struct
	VENDOR_VIDEOPROC_DEWARP_INFO,			///<  support set/get for i/o path, using VENDOR_DEWARP_INFO struct
	VENDOR_VIDEOPROC_DEWARP_TABLE,			///<  support set 	  for i/o path, using VENDOR_DEWARP_2DLUT_TABLE struct
	VENDOR_VIDEOPROC_DEWARP_DCTG_INFO,  	///<  support set/get for i/o path, using VENDOR_DEWARP_DCTG_INFO struct
	VENDOR_VIDEOPROC_DEWARP_DCTG_PARM,  	///<  support set/get for i/o path, using VENDOR_DEWARP_DCTG_PARM struct
	VENDOR_VIDEOPROC_PARAM_SYSCAPS,			///<  support get, using VENDOR_VIDEOPROC_SYSCAPS struct
	VENDOR_VIDEOPROC_DCE_REL_FD,			///<  support set
	VENDOR_VIDEOPROC_SLICE_MODE,			///<  support set/get for 4K without column mode, using VENDOR_VDOPROC_SLICE_MODE struct
	VENDOR_VIDEOPROC_USR_INIT_FRAME,		///<  support set     for i/o path, using VENDOR_VIDEOPROC_USR_INIT_FRAME_INFO
	VENDOR_VIDEOPROC_UV_SWAP,		        ///<  support set for i/o path, using VENDOR_VDOPROC_UV_SWAP
	VENDOR_VIDEOPROC_IN_POOL,				///<  support set/get for i/o path, using VENDOR_VDOPROC_IN_POOL struct
	ENUM_DUMMY4WORD(VENDOR_VIDEOPROC_PARAM_ID)
} VENDOR_VIDEOPROC_PARAM_ID;//typo for backward compatible

typedef struct _VENDOR_DEWARP_CTRL {
	BOOL dc_enable;
	BOOL dctg_enable;
} VENDOR_DEWARP_CTRL;

typedef struct _VENDOR_DEWARP_IMPROVE_CTRL {
	BOOL improve_en;
	BOOL use_tmnr;	//use vpe out to calculate buffer size
} VENDOR_DEWARP_IMPROVE_CTRL;

typedef enum _VENDOR_DEWARP_MODE {
	VENDOR_DEWARP_DEWARP_MODE_GDC	= 0,
	VENDOR_DEWARP_DEWARP_MODE_2DLUT	= 1,
	ENUM_DUMMY4WORD(VENDOR_DEWARP_DEWARP_MODE)
} VENDOR_DEWARP_MODE;

typedef struct _VENDOR_DEWARP_DGC_PARM {
	INT32 cent_x_s;		///< Lens center of x axis (signed)
	INT32 cent_y_s;  	///< Lens center of y axis (signed)
	UINT32 lens_r;		///< Radius of Lens
	UINT32 xdist;  		///< X input distance factor, for oval shape modeling
	UINT32 ydist;  		///< Y input distance factor, for oval shape modeling
	UINT8 normfact;  	///< Radius normalization factor (u1.7)
	UINT8 normbit;  	///< Radius normalization shift bit
	UINT16 geo_lut[VENDOR_GEO_LUT_X]; ///< GDC look-up table
} VENDOR_DEWARP_DGC_PARM;

typedef struct _VENDOR_DEWARP_2DLUT_PARM {
	UINT8 lut2d_sz;		///< Size selection of 2D look-up table, 0:9x9, 3:65x65
	UINT32 hfact;  		///< Horizontal scaling factor for 2DLut scaling up (u0.24)
	UINT32 vfact;  		///< Vertical scaling factor for 2DLut scaling up (u0.24)
	UINT8 xofs_i;  		///< 2DLUT x offset, integer part
	UINT32 xofs_f;  	///< 2DLUT x offset, fraction part
	UINT8 yofs_i;  		///< 2DLUT y offset, integer part
	UINT32 yofs_f;  	///< 2DLUT xy offset, fraction part
} VENDOR_DEWARP_2DLUT_PARM;

typedef struct _VENDOR_DEWARP_FOV_PARM {
	UINT8 fovbound;  	///< FOV boundary process method selection
	UINT16 boundy;   	///< Bound value for Y component (u8.2)
	UINT16 boundu;   	///< Bound value for U component (u8.2)
	UINT16 boundv;   	///< Bound value for V component (u8.2)
	UINT16 fovgain;  	///< Scale down factor for FOV preservation (u2.10)
} VENDOR_DEWARP_FOV_PARM;

typedef struct _VENDOR_DEWARP_INFO {
	VENDOR_DEWARP_MODE mode;
	VENDOR_DEWARP_DGC_PARM dgc;
	VENDOR_DEWARP_2DLUT_PARM lut2d;
	VENDOR_DEWARP_FOV_PARM fov;
} VENDOR_DEWARP_INFO;

typedef struct _VENDOR_DEWARP_2DLUT_TABLE {
	UINT32 tbl[VENDOR_GEO_LUT];
} VENDOR_DEWARP_2DLUT_TABLE;

typedef enum _VENDOR_DEWARP_DCTG_MODE {
	VENDOR_DEWARP_DCTG_MODE_90	= 0,
	VENDOR_DEWARP_DCTG_MODE_360	= 1,
	ENUM_DUMMY4WORD(VENDOR_DEWARP_DCTG_MODE)
} VENDOR_DEWARP_DCTG_MODE;

typedef struct _VENDOR_DEWARP_DCTG_LENS_PARM {
	UINT8 mount_type;	///< Camera mount type. 0:Ceiling mount, 1:Floor mount
	UINT8 lut2d_sz;  	///< Size selection of 2D look-up table, 0:9x9, 3:65x65, should the same with DCE setting.
	UINT32 lens_r;  	///< Radius of Lens
	UINT32 lens_x_st; 	///< Lens start x position at a source image
	UINT32 lens_y_st; 	///< Lens start y position at a source image
} VENDOR_DEWARP_DCTG_LENS_PARM;

typedef struct _VENDOR_DEWARP_DCTG_FOV_PARM {
	INT32 theta_st;  	///< FOV theta start radian (s4.16) Range: -*pi ~ *pi
	INT32 theta_ed;  	///< FOV theta end radian (s4.16) Range: -*pi ~ *pi
	INT32 phi_st;  		///< FOV phi start radian (s4.16) Range: -2*pi ~ 2*pi
	INT32 phi_ed;  		///< FOV phi end radian (s4.16) Range: -2*pi ~ 2*pi
	INT32 rot_z;  		///< Z-axis rotate radian (s4.16) Range: -2*pi ~ 2*pi
	INT32 rot_y;  		///< Y-axis rotate radian (s4.16) Range: -2*pi ~ 2*pi
} VENDOR_DEWARP_DCTG_FOV_PARM;

typedef struct _VENDOR_DEWARP_DCTG_INFO {
	VENDOR_DEWARP_DCTG_MODE mode;
	VENDOR_DEWARP_DCTG_LENS_PARM lens;
	VENDOR_DEWARP_DCTG_FOV_PARM fov;
} VENDOR_DEWARP_DCTG_INFO;

typedef struct _VENDOR_DEWARP_DCTG_PARM {
	INT32 y_degree;
	INT32 z_degree;
} VENDOR_DEWARP_DCTG_PARM;

typedef struct _VENDOR_VIDEOPROC_SYSCAPS {
	HD_VIDEO_PXLFMT in_src_fmt; ///< [in]  source image pixel format
	HD_VIDEO_PXLFMT in_dst_fmt; ///< [in]  destination image pixel format
	HD_VIDEO_DIR in_direction;	///< [in]  image direction
	INT is_interlace;			///< [in]  iamge interlace / progressive
	UINT8 out_src_fmt_is_valid;	///< [out] source image pixel format is validated
	char* out_src_fmt_str;		///< [out] source image pixel format string
	HD_DIM out_src_min_dim;		///< [out] source image min width / height
	HD_DIM out_src_max_dim;		///< [out] source image max width / height
	HD_VIDEO_CROP out_src_crop;	///< [out] source crop alignment rect / coord
	UINT8 out_dst_fmt_is_valid;	///< [out] destination image pixel format is validated
	char* out_dst_fmt_str;		///< [out] destination image pixel format string
	HD_DIM out_dst_min_dim;		///< [out] destination image min width / height
	HD_DIM out_dst_max_dim;		///< [out] destination image max width / height
	HD_VIDEO_CROP out_dst_pip;	///< [out] destination pip alignment rect / coord
	HD_VIDEO_CROP out_dst_psr;	///< [out] destination psr alignment rect / coord
} VENDOR_VIDEOPROC_SYSCAPS;


typedef struct _VENDOR_VIDEOPROC_USR_INIT_FRAME_INFO {
	HD_VIDEO_FRAME image;               ///< the image frame be set initially
} VENDOR_VIDEOPROC_USR_INIT_FRAME_INFO;

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoproc_set(HD_PATH_ID path_id, VENDOR_VIDEOPROC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoproc_get(HD_PATH_ID path_id, VENDOR_VIDEOPROC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoproc_pull_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, INT32 wait_ms);
HD_RESULT vendor_videoproc_release_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame);
HD_RESULT vendor_videoproc_push_list(VENDOR_VIDEOPROC_PUSH_LIST *p_videoproc_list, UINT32 num, INT32 wait_ms);

#ifdef  __cplusplus
}
#endif

#endif // __VENDOR_PROC_H__
