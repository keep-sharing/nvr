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

#define VENDOR_VIDEOENC_VERSION 0x010002 //vendor encode version
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

typedef enum _VENDOR_VIDEOENC_ID {	
	VENDOR_VIDEOENC_PARAM_OSG_SEL = 0,   ///<support set/get,VENDOR_VIDEOENC_OSG to use 3d or encode osg
	VENDOR_VIDEOENC_PARAM_MIN_COMPRESS_RATIO,   ///<support set/get,VENDOR_VDOENC_MIN_COMPRESS_RATIO to config enc compress ratio
	VENDOR_VIDEOENC_MAX,
	ENUM_DUMMY4WORD(VENDOR_VIDEOENC_PARAM_ID)
} VENDOR_VIDEOENC_PARAM_ID;
/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/
HD_RESULT vendor_videoenc_get(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, void *p_param);
HD_RESULT vendor_videoenc_set(HD_PATH_ID path_id, VENDOR_VIDEOENC_PARAM_ID id, void *p_param);

#endif // __VENDOR_ENC_H__
