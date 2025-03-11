/**
@brief Header file of vendor videodec
This file contains the functions which is related to videodec in the chip.

@file vendor_videodec.h

@ingroup mhdal

@note Nothing.

Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef __VENDOR_DEC_H__
#define __VENDOR_DEC_H__

#define VENDOR_VIDEODEC_VERSION 0x010007 //vendor decode version
/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
TYPE DEFINITION
********************************************************************/
typedef enum _VENDOR_VIDEODEC_SUB_RATIO {
	VENDOR_SUB_RATIO_OFF     = 1,      ///< turn off sub-yuv output
	VENDOR_SUB_RATIO_2x      = 2,      ///< enable, w : 1/2  h : 1/2
	VENDOR_SUB_RATIO_4x      = 4,      ///< enable, w : 1/4  h : 1/4
	ENUM_DUMMY4WORD(VENDOR_VIDEODEC_SUB_RATIO)
} VENDOR_VIDEODEC_SUB_RATIO;

typedef struct _VENDOR_VIDEODEC_JPEG_WORK_BUFFER {
	UINT32  phy_addr;                               ///< physical address of jpeg working buffer
	UINT32  ddr_id;                                 ///< the ddr_id of jpeg working buffer
	UINT32  size;                                   ///< the size of jpeg working buffer
} VENDOR_VIDEODEC_JPEG_WORK_BUFFER;

typedef struct _VENDOR_VIDEODEC_SUB_YUV {
	VENDOR_VIDEODEC_SUB_RATIO ratio;                ///< sub-yuv ratio
} VENDOR_VIDEODEC_SUB_YUV;

typedef struct _VENDOR_VIDEODEC_QP_CONFIG {
	UINT32 qp_value;                                ///< quantization parameter value, using VENDOR_VIDEODEC_QP_DEFAULT to set default
} VENDOR_VIDEODEC_QP_CONFIG;

typedef struct _VENDOR_VIDEODEC_ADJUST_TIMESTAMP {
	UINT32 time_diff_ms;                            ///< recalculate bitstream buffer timestamp by time_diff_ms 
} VENDOR_VIDEODEC_ADJUST_TIMESTAMP;

typedef struct _VENDOR_VIDEODEC_STATUS_NEXT_PATH_ID {
	HD_PATH_ID path_id;                             ///< next path_id for querying done_frames by HD_VIDEODEC_PARAM_STATUS parameter
} VENDOR_VIDEODEC_STATUS_NEXT_PATH_ID;

typedef enum _VENDOR_VIDEODEC_ID {
	VENDOR_VIDEODEC_PARAM_JPEG_WORK_BUFFER,         ///< support set/get,VENDOR_VIDEODEC_JPEG_WORK_BUFFER to setup jpeg's working buffer
	VENDOR_VIDEODEC_PARAM_FREE_JPEG_WORK_BUFFER,    ///< support set, no param structure, free jpeg's working buffer
	VENDOR_VIDEODEC_PARAM_SUB_YUV_RATIO,            ///< support set/get, using VENDOR_VIDEODEC_SUB_YUV struct
	VENDOR_VIDEODEC_PARAM_QP_CONFIG,                ///< support set/get, using VENDOR_VIDEODEC_QP_CONFIG struct
	VENDOR_VIDEODEC_PARAM_ADJUST_TIMESTAMP,         ///< support set, using VENDOR_VIDEODEC_ADJUST_TIMESTAMP struct
	VENDOR_VIDEODEC_PARAM_STATUS_NEXT_PATH_ID,       ///< support set, using VENDOR_VIDEODEC_STATUS_NEXT_PATH_ID struct
	VENDOR_VIDEODEC_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEODEC_PARAM_ID)
} VENDOR_VIDEODEC_PARAM_ID;
/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videodec_get(HD_PATH_ID path_id, VENDOR_VIDEODEC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videodec_set(HD_PATH_ID path_id, VENDOR_VIDEODEC_PARAM_ID id, void *p_param);

#endif // __VENDOR_DEC_H__
