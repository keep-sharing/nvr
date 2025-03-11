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

#define VENDOR_VIDEODEC_VERSION 0x010010 //vendor decode version
/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
#include "hd_videodec.h"

#ifdef  __cplusplus
extern "C" {
#endif

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define VENDOR_VIDEODEC_TIME_ALIGN_TIMESTAMP 0xFEFE0AFE ///< playback with user timestamp
#define VENDOR_VIDEODEC_FLAG_TIMESTAMP_SYNC  0x00000002   ///< timestamp sync frame (bit2)
#define VENDOR_VIDEODEC_STATUS_LIST_SIZE(num)  (sizeof(VENDOR_VIDEODEC_STATUS_OBJ) * (num) + sizeof(UINT))

/********************************************************************
TYPE DEFINITION
********************************************************************/
typedef enum _VENDOR_VIDEODEC_SUB_RATIO {
	VENDOR_SUB_RATIO_OFF     = 1,                   ///< turn off sub-yuv output
	VENDOR_SUB_RATIO_2x      = 2,                   ///< enable, w : 1/2  h : 1/2
	VENDOR_SUB_RATIO_3x      = 3,                   ///< enable, w : 1/3  h : 1/3
	VENDOR_SUB_RATIO_4x      = 4,                   ///< enable, w : 1/4  h : 1/4
	ENUM_DUMMY4WORD(VENDOR_VIDEODEC_SUB_RATIO)
} VENDOR_VIDEODEC_SUB_RATIO;

typedef enum _VENDOR_VIDEODEC_OUTMODE {
	VENDOR_VIDEODEC_OUTMODE_COMPRESS   = 1,         ///< compression output mode
	VENDOR_VIDEODEC_OUTMODE_UNCOMPRESS = 2,         ///< uncompressed ouptut mode
	ENUM_DUMMY4WORD(VENDOR_VIDEODEC_OUTMODE)
} VENDOR_VIDEODEC_OUTMODE;

typedef struct _VENDOR_VIDEODEC_JPEG_WORK_BUFFER {
	UINTPTR  phy_addr;                              ///< physical address of jpeg working buffer
	UINT32  ddr_id;                                 ///< the ddr_id of jpeg working buffer
	UINT32  size;                                   ///< the size of jpeg working buffer
} VENDOR_VIDEODEC_JPEG_WORK_BUFFER;

typedef struct _VENDOR_VIDEODEC_SUB_YUV {
	VENDOR_VIDEODEC_SUB_RATIO ratio;                ///< sub-yuv ratio
} VENDOR_VIDEODEC_SUB_YUV;

typedef struct _VENDOR_VIDEODEC_QP_CONFIG {
	UINT32 qp_value;                                ///< quantization parameter value, using VENDOR_VIDEODEC_QP_DEFAULT to set default
} VENDOR_VIDEODEC_QP_CONFIG;

typedef struct _VENDOR_VIDEODEC_JPEG_HEADER {
	UINT8  *inbuf;                                  ///< input buffer address of jpeg
	INT32  size;                                    ///< the size of jpeg header
} VENDOR_VIDEODEC_JPEG_HEADER;

typedef struct _VENDOR_VIDEODEC_H26X_OUT {
	VENDOR_VIDEODEC_OUTMODE first_out;              ///< H.26x first output mode
	VENDOR_VIDEODEC_OUTMODE extra_out;              ///< H.26x extra output mode
} VENDOR_VIDEODEC_H26X_OUT;

typedef struct _VENDOR_VIDEODEC_SYSCAPS {
    HD_VIDEO_CODEC usr_codec_type;                  ///< [in]  user codec type to get system capabilitys
    VENDOR_VIDEODEC_OUTMODE usr_first_mode;         ///< [in]  user first output mode (compress / uncompress)
    VENDOR_VIDEODEC_OUTMODE usr_extra_mode;         ///< [in]  user extra output mode (compress / uncompress)
    HD_DIM first_align;                             ///< [out] first output buffer dimension alignment
    HD_DIM extra_align;                             ///< [out] extra output buffer dimension alignment
    HD_VIDEO_PXLFMT first_pxlfmt;                   ///< [out] first output buffer pixel format
    HD_VIDEO_PXLFMT extra_pxlfmt;                   ///< [out] extra output buffer pixel format
    HD_DIM min_dim;                                 ///< [out] minimum dimension
    HD_DIM max_dim;                                 ///< [out] maximum dimension
} VENDOR_VIDEODEC_SYSCAPS;

typedef struct _VENDOR_VIDEODEC_H265_TILE_BUF {
	UINTPTR addr_pa;                                ///< physical address of H265 tile buffer
	UINT32  buf_size;                               ///< size of H265 tile buffer
	INT32   ddr_id;                                 ///< ddr id of H265 tile buffer
} VENDOR_VIDEODEC_H265_TILE_BUF;

typedef struct _VENDOR_VIDEODEC_STATUS_NEXT_PATH_ID {
	HD_PATH_ID path_id;                             ///< next path_id for querying done_frames by HD_VIDEODEC_PARAM_STATUS parameter
} VENDOR_VIDEODEC_STATUS_NEXT_PATH_ID;

typedef struct _VENDOR_VIDEODEC_USER_INFO {
	UINT32 dec_user_flag;                           ///< lastest decoded user flag
} VENDOR_VIDEODEC_USER_INFO;

typedef struct _VENDOR_VIDEODEC_BS_PARSER {
	UINT32 enable;                                  ///< enable bitstream parser, support h.264/h.265
	CHAR   *wkbuf_va;                               ///< virtual address of working buffer
	UINT32 wkbuf_sz;                                ///< size of working buffer
} VENDOR_VIDEODEC_BS_PARSER;

typedef struct _VENDOR_VIDEODEC_STATUS_OBJ {
	HD_PATH_ID path_id;                             ///< path id
	UINT32 left_frames;                             ///< number of frames to be decoded
	UINT32 reserved_ref_frame;                      ///< reserved reference frame when unbind unit
	UINT32 done_frames;                             ///< number of decoded frames
	UINT32 dti_buf_cnt;                             ///< available datain buffer count (i.e. decode in buffer count)
} VENDOR_VIDEODEC_STATUS_OBJ;

typedef struct _VENDOR_VIDEODEC_STATUS_LIST {
	UINT num;                                       ///< number of status informations
	VENDOR_VIDEODEC_STATUS_OBJ status_objs[];       ///< status list objects array
} VENDOR_VIDEODEC_STATUS_LIST;

typedef enum _VENDOR_VIDEODEC_ID {
	VENDOR_VIDEODEC_PARAM_JPEG_WORK_BUFFER,         ///< support set/get,VENDOR_VIDEODEC_JPEG_WORK_BUFFER to setup jpeg's working buffer
	VENDOR_VIDEODEC_PARAM_FREE_JPEG_WORK_BUFFER,    ///< support set, no param structure, free jpeg's working buffer
	VENDOR_VIDEODEC_PARAM_SUB_YUV_RATIO,            ///< support set/get, using VENDOR_VIDEODEC_SUB_YUV struct
	VENDOR_VIDEODEC_PARAM_QP_CONFIG,                ///< support set/get, using VENDOR_VIDEODEC_QP_CONFIG struct
	VENDOR_VIDEODEC_PARAM_H26X_OUT,                 ///< support set/get, using VENDOR_VIDEODEC_H26X_OUT struct
	VENDOR_VIDEODEC_PARAM_SYSCAPS,                  ///< support get, using VENDOR_VIDEODEC_SYSCAPS struct
	VENDOR_VIDEODEC_PARAM_H265_TILE_BUF,            ///< support set/get, using VENDOR_VIDEODEC_H265_TILE_BUF struct
	VENDOR_VIDEODEC_PARAM_STATUS_NEXT_PATH_ID,      ///< support set, using VENDOR_VIDEODEC_STATUS_NEXT_PATH_ID struct
	VENDOR_VIDEODEC_PARAM_USER_INFO,                ///< support get, using VENDOR_VIDEODEC_USER_INFO struct
	VENDOR_VIDEODEC_PARAM_BS_PARSER,                ///< support set/get, using VENDOR_VIDEODEC_BS_PARSER struct
	VENDOR_VIDEODEC_PARAM_STATUS_LIST,              ///< support get, using VENDOR_VIDEODEC_STATUS_LIST struct
	VENDOR_VIDEODEC_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEODEC_PARAM_ID)
} VENDOR_VIDEODEC_PARAM_ID;
/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videodec_get(HD_PATH_ID path_id, VENDOR_VIDEODEC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videodec_set(HD_PATH_ID path_id, VENDOR_VIDEODEC_PARAM_ID id, void *p_param);

#ifdef  __cplusplus
}
#endif

#endif // __VENDOR_DEC_H__
