/**
@brief Header file of vendor gfx
This file contains the functions which is related to gfx in the chip.

@file vendor_gfx.h

@ingroup mhdal

@note Nothing.

Copyright Novatek Microelectronics Corp. 2019.  All rights reserved.
*/
#ifndef __VENDOR_GFX_H__
#define __VENDOR_GFX_H__

/********************************************************************
INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"
#include "hd_gfx.h"

/********************************************************************
MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
TYPE DEFINITION
********************************************************************/

/********************************************************************
EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT vendor_gfx_memset(HD_GFX_DRAW_RECT *p_param);

HD_RESULT vendor_gfx_scale_bilinear(HD_GFX_SCALE *p_param, int alpha_reserve, int reserve1, int reserve2, int reserve3);

#endif // __VENDOR_GFX_H__
