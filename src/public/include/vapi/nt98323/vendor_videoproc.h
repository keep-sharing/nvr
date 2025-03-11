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

#define VENDOR_VIDEOPROC_VERSION 0x010008 //vendor videoproc version
/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/

/**
	@name VENDOR_VIDEOPROC_FLAG for VENDOR_VIDEOPROC_USER_FLAG
*/
#define VENDOR_VIDEOPROC_FLAG_UV_SWAP       0x00000001   ///< uv swap when vproc getting yuv from decoder

/********************************************************************
TYPE DEFINITION
********************************************************************/

typedef struct _VENDOR_VDOPROC_STATUS {
	UINT32 dti_buf_cnt;								///< datain available buffer count
} VENDOR_VDOPROC_STATUS;

/**
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
} VENDOR_VIDEOPROC_PUSH_LIST;

typedef enum _VENDOR_VIDEOPROC_PARAM_ID {
	VENDOR_VIDEOPROC_STATUS,				///<  support get
	VENDOR_VIDEOPROC_USER_FLAG,             ///<  support set for i/o path, using VENDOR_VIDEOPROC_FLAG definitions
	VENDOR_VIDEOPROC_SUB_RATIO_THLD,        ///<  support set/get for i/o path, using VENDOR_VDOPROC_SUB_RATIO_THLD struct
	ENUM_DUMMY4WORD(VENDOR_VIDEOPROC_PARAM_ID)
} VENDOR_VIDEOPROC_PARAM_ID;//typo for backward compatible

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoproc_set(HD_PATH_ID path_id, VENDOR_VIDEOPROC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoproc_get(HD_PATH_ID path_id, VENDOR_VIDEOPROC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoproc_pull_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, INT32 wait_ms);
HD_RESULT vendor_videoproc_release_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame);
HD_RESULT vendor_videoproc_push_list(VENDOR_VIDEOPROC_PUSH_LIST *p_videoproc_list, UINT32 num, INT32 wait_ms);
#endif // __VENDOR_PROC_H__
