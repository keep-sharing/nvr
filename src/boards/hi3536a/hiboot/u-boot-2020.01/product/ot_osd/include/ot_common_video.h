// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __OT_COMMON_VIDEO_H__
#define __OT_COMMON_VIDEO_H__

#include "ot_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum  {
    OT_ASPECT_RATIO_NONE   = 0,        /* full screen */
    OT_ASPECT_RATIO_AUTO   = 1,        /* ratio no change, 1:1 */
    OT_ASPECT_RATIO_MANUAL = 2,        /* ratio manual set */
    OT_ASPECT_RATIO_BUTT
} ot_aspect_ratio_type;

typedef enum  {
    OT_COMPRESS_MODE_NONE = 0,      /* no compress */
    OT_COMPRESS_MODE_SEG,           /* compress unit is 256x1 bytes as a segment. */
    OT_COMPRESS_MODE_SEG_COMPACT,   /* compact compress unit is 256x1 bytes as a segment. */
    OT_COMPRESS_MODE_TILE,          /* compress unit is a tile. */
    OT_COMPRESS_MODE_LINE,          /* compress unit is the whole line. */
    OT_COMPRESS_MODE_FRAME,         /* compress unit is the whole frame. YUV for VPSS(3DNR) */

    OT_COMPRESS_MODE_BUTT
} ot_compress_mode;

typedef enum {
    OT_PIXEL_FORMAT_RGB_444 = 0,
    OT_PIXEL_FORMAT_RGB_555,
    OT_PIXEL_FORMAT_RGB_565,
    OT_PIXEL_FORMAT_RGB_888,

    OT_PIXEL_FORMAT_BGR_444,
    OT_PIXEL_FORMAT_BGR_555,
    OT_PIXEL_FORMAT_BGR_565,
    OT_PIXEL_FORMAT_BGR_888,

    OT_PIXEL_FORMAT_ARGB_1555,
    OT_PIXEL_FORMAT_ARGB_4444,
    OT_PIXEL_FORMAT_ARGB_8565,
    OT_PIXEL_FORMAT_ARGB_8888,
    OT_PIXEL_FORMAT_ARGB_2BPP,
    OT_PIXEL_FORMAT_ARGB_CLUT2,
    OT_PIXEL_FORMAT_ARGB_CLUT4,

    OT_PIXEL_FORMAT_ABGR_1555,
    OT_PIXEL_FORMAT_ABGR_4444,
    OT_PIXEL_FORMAT_ABGR_8565,
    OT_PIXEL_FORMAT_ABGR_8888,

    OT_PIXEL_FORMAT_RGB_BAYER_8BPP,
    OT_PIXEL_FORMAT_RGB_BAYER_10BPP,
    OT_PIXEL_FORMAT_RGB_BAYER_12BPP,
    OT_PIXEL_FORMAT_RGB_BAYER_14BPP,
    OT_PIXEL_FORMAT_RGB_BAYER_16BPP,

    OT_PIXEL_FORMAT_YVU_PLANAR_422,
    OT_PIXEL_FORMAT_YVU_PLANAR_420,
    OT_PIXEL_FORMAT_YVU_PLANAR_444,

    OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422,
    OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420,
    OT_PIXEL_FORMAT_YVU_SEMIPLANAR_444,

    OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422,
    OT_PIXEL_FORMAT_YUV_SEMIPLANAR_420,
    OT_PIXEL_FORMAT_YUV_SEMIPLANAR_444,

    OT_PIXEL_FORMAT_YUYV_PACKAGE_422,
    OT_PIXEL_FORMAT_YVYU_PACKAGE_422,
    OT_PIXEL_FORMAT_UYVY_PACKAGE_422,
    OT_PIXEL_FORMAT_VYUY_PACKAGE_422,
    OT_PIXEL_FORMAT_YYUV_PACKAGE_422,
    OT_PIXEL_FORMAT_YYVU_PACKAGE_422,
    OT_PIXEL_FORMAT_UVYY_PACKAGE_422,
    OT_PIXEL_FORMAT_VUYY_PACKAGE_422,
    OT_PIXEL_FORMAT_VY1UY0_PACKAGE_422,

    OT_PIXEL_FORMAT_YUV_400,
    OT_PIXEL_FORMAT_UV_420,

    /* SVP data format */
    OT_PIXEL_FORMAT_BGR_888_PLANAR,
    OT_PIXEL_FORMAT_HSV_888_PACKAGE,
    OT_PIXEL_FORMAT_HSV_888_PLANAR,
    OT_PIXEL_FORMAT_LAB_888_PACKAGE,
    OT_PIXEL_FORMAT_LAB_888_PLANAR,
    OT_PIXEL_FORMAT_S8C1,
    OT_PIXEL_FORMAT_S8C2_PACKAGE,
    OT_PIXEL_FORMAT_S8C2_PLANAR,
    OT_PIXEL_FORMAT_S8C3_PLANAR,
    OT_PIXEL_FORMAT_S16C1,
    OT_PIXEL_FORMAT_U8C1,
    OT_PIXEL_FORMAT_U16C1,
    OT_PIXEL_FORMAT_S32C1,
    OT_PIXEL_FORMAT_U32C1,
    OT_PIXEL_FORMAT_U64C1,
    OT_PIXEL_FORMAT_S64C1,

    OT_PIXEL_FORMAT_BUTT
} ot_pixel_format;

typedef enum  {
    OT_DYNAMIC_RANGE_SDR8 = 0,
    OT_DYNAMIC_RANGE_SDR10,
    OT_DYNAMIC_RANGE_HDR10,
    OT_DYNAMIC_RANGE_HLG,
    OT_DYNAMIC_RANGE_SLF,
    OT_DYNAMIC_RANGE_XDR,
    OT_DYNAMIC_RANGE_BUTT
} ot_dynamic_range;

typedef struct {
    td_u32 top_width;
    td_u32 bottom_width;
    td_u32 left_width;
    td_u32 right_width;
    td_u32 color;
} ot_border;

typedef struct {
    td_u32 width;
    td_u32 height;
} ot_size;

typedef struct {
    td_s32 x;
    td_s32 y;
    td_u32 width;
    td_u32 height;
} ot_rect;

typedef struct {
    ot_aspect_ratio_type mode;          /* aspect ratio mode: none/auto/manual */
    td_u32         bg_color;      /* background color, RGB 888 */
    ot_rect         video_rect;     /* valid in ASPECT_RATIO_MANUAL mode */
} ot_aspect_ratio;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __OT_COMMON_VIDEO_H__ */
