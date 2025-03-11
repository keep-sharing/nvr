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
#define HD_VIDEO_PXLFMT_YUV444ONE_ALPHA  0x51181444 //<HD_VIDEO_PXLFMT_YUV444_ONE+alpha plane,p_phy_addr[0]:yuv444_one data,p_phy_addr[1]:alpha  data
#define HD_VIDEO_PXLFMT_BGR888	0x21181888 ///< 1 plane, pixel=BGR(3w,h)


 typedef struct _VENDOR_GFX_SYSCAPS {
    INT scl_rate;                                         ///< [out] scaling rate:ex:1/Nx~Nx
    HD_DIM min_dim;                                 ///< [out] minimum dimension
    HD_DIM max_dim;                                ///< [out] maximum dimension
    HD_DIM image_align;                           ///< [out] dimension alignment
} VENDOR_GFX_SYSCAPS;

HD_RESULT vendor_gfx_memset(HD_GFX_DRAW_RECT *p_param);
HD_RESULT vendor_gfx_scale_bilinear(HD_GFX_SCALE *p_param, int alpha_reserve, int reserve1, int reserve2, int reserve3);
HD_RESULT vendor_gfx_get_syscaps(VENDOR_GFX_SYSCAPS *p_param);

HD_RESULT vendor_gfx_scale(HD_GFX_SCALE *p_param, UINT8 alpha_reserve, UINT8 num_of_image);
HD_RESULT vendor_gfx_copy(HD_GFX_COPY *p_param, UINT8 num_of_image);
HD_RESULT vendor_gfx_copy_dither(HD_GFX_COPY *p_param, UINT8 num_of_image, UINT8 dithering_en);
HD_RESULT vendor_gfx_rotate(HD_GFX_ROTATE *p_param,  UINT8 num_of_image);
HD_RESULT vendor_gfx_draw_lines(HD_GFX_DRAW_LINE *p_param, UINT8 num_of_line);
HD_RESULT vendor_gfx_draw_rects(HD_GFX_DRAW_RECT *p_param, UINT8 num_of_rect);
#endif // __VENDOR_GFX_H__
