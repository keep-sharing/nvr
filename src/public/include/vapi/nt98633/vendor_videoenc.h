/**
@brief Header file of vendor videoenc
This file contains the functions which is related to videoenc in the chip.

@file vendor_videoenc.h

@ingroup mhdal

@note Nothing.

Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef __VENDOR_ENC_H__
#define __VENDOR_ENC_H__

#define VENDOR_VIDEOENC_VERSION 0x010007 //vendor encode version
/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
#include "vendor_common.h"

#ifdef  __cplusplus
extern "C" {
#endif

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/
#define MAX_SUPPORT_FORMAT 10
#define MAX_OSG_SUPPORT_FORMAT 10

/********************************************************************
TYPE DEFINITION
********************************************************************/
typedef struct _VENDOR_VIDEOENC_OSG {
	INT16      external_osg;						///< select which osg 1:3d osg 0:encode osg
} VENDOR_VIDEOENC_OSG;

typedef struct _VENDOR_VIDEOENC_QP_RATIO {
	UINT8 qp;
	UINT8 ratio;
} VENDOR_VIDEOENC_QP_RATIO;

typedef struct _VENDOR_VDOENC_MIN_COMPRESS_RATIO {
    HD_VIDEO_CODEC  codec_type;
	VENDOR_VIDEOENC_QP_RATIO qp_ratio;
} VENDOR_VIDEOENC_MIN_COMPRESS_RATIO;

typedef struct _VENDOR_VDOENC_MIN_BS_SIZE {
    HD_DIM dim;
} VENDOR_VIDEOENC_MIN_BS_SIZE;

typedef enum _VENDOR_JPEGENC_RC_MODE {
	VENDOR_JPEG_RC_MODE_CBR = 0,     ///< Constant Bitrate
	VENDOR_JPEG_RC_MODE_VBR,         ///< Variable Bitrate
	VENDOR_JPEG_RC_MODE_FIXQP,       ///< fix quality
	VENDOR_JPEG_RC_MAX,
	ENUM_DUMMY4WORD(HD_JPEGENC_RC_MODE)
} VENDOR_JPEGENC_RC_MODE;

typedef struct _VENDOR_JPEGENC_RATE_CONTROL {
	VENDOR_JPEGENC_RC_MODE vbr_mode;         ///< 0: CBR, 1: VBR
	UINT32 base_qp;                          ///< base qp
	UINT32 min_quality;                      ///< min quality
	UINT32 max_quality;                      ///< max quality
	UINT32 bitrate;	                         ///< bit rate
	UINT32 frame_rate_base;                  ///< frame rate
	UINT32 frame_rate_incr;                  ///< frame rate = frame_rate_base / frame_rate_incr
} VENDOR_JPEGENC_RATE_CONTROL;

typedef struct _VENDOR_FMT_ALIGN {
	HD_VIDEO_PXLFMT    fmt;                    ///< format
	HD_DIM             align;                  ///< the alignment depend on format
} VENDOR_FMT_ALIGN;

typedef struct _VENDOR_VIDEOENC_SYSCAPS {
    HD_VIDEO_CODEC     usr_codec_type;                   ///< [in] user specified codec type to get system capabilitys
    HD_DIM             min_dim;                          ///< [out] the minimum dimension
    HD_DIM             max_dim;                          ///< [out] the maximum dimension
    VENDOR_FMT_ALIGN   fmt_align[MAX_SUPPORT_FORMAT];    ///< [out] the supported input format and corresponding alignment
    unsigned int       fmt_cnt;                          ///< [out] the supported input format count
    unsigned int       max_osg_num;                      ///< [out] the supported maximum osg number
	HD_IRECT           osg_align;                        ///< [out] the supported osg alignment
	unsigned int       osg_line_offset_align;            ///< [out] the supported osg line offset alignment
	HD_VIDEO_PXLFMT    osg_fmt[MAX_OSG_SUPPORT_FORMAT];  ///< [out] the supported osg format
	unsigned int       osg_fmt_cnt;                      ///< [out] the supported osg format count
	unsigned int       max_mask_num;                     ///< [out] the supported maximum mask number
	HD_IRECT           mask_align;                       ///< [out] the supported mask alignment
} VENDOR_VIDEOENC_SYSCAPS;

typedef enum _VENDOR_VIDEOENC_ID {
	VENDOR_VIDEOENC_PARAM_OSG_SEL = 0,			///< support set/get, using VENDOR_VIDEOENC_OSG to use 3d or encode osg
	VENDOR_VIDEOENC_PARAM_MIN_COMPRESS_RATIO,	///< support set/get, using VENDOR_VDOENC_MIN_COMPRESS_RATIO to config enc compress ratio
	VENDOR_VIDEOENC_PARAM_MIN_BS_SIZE,          ///< support set/get, using VENDOR_VIDEOENC_MIN_BS_SIZE to config min width and height to calculate min bs size
	VENDOR_VIDEOENC_PARAM_RESET_QUEUE,			///< support set, support to clear buffered bitstream frames
	VENDOR_JPEGENC_PARAM_RATE_CONTROL,			///< support set/get, using VENDOR_JPEGENC_RATE_CONTROL to config jpeg rate control
	VENDOR_VIDEOENC_PARAM_SYSCAPS,              ///< support get, using VENDOR_VIDEOENC_SYSCAPS struct
	VENDOR_VIDEOENC_PARAM_IN_STAMP_IMG,         ///< support set/get, using VENDOR_OSG_STAMP_IMG struct
	VENDOR_VIDEOENC_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_PARAM_ID)
} VENDOR_VIDEOENC_PARAM_ID;
/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoenc_get(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoenc_set(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, void *p_param);

#ifdef  __cplusplus
}
#endif

#endif // __VENDOR_ENC_H__
