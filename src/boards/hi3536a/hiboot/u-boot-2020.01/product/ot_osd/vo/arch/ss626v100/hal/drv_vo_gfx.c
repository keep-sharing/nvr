// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_vo_gfx.h"
#include "ot_common_vo.h"

#include "vo.h"
#include "drv_vo.h"

#define VO_GFX_DISP_RECT_W_MAX_OF_GFXX 4096
#define VO_GFX_DISP_RECT_H_MAX_OF_GFXX 4096
#define VO_GFX_DISP_RECT_W_MAX_OF_GFX4 720
#define VO_GFX_DISP_RECT_H_MAX_OF_GFX4 720

#if vo_desc("vo gfx")
td_s32 vou_drv_get_gfx_bind_dev(ot_vo_layer layer)
{
    ot_vo_dev bind_dev;
    if (layer == HAL_DISP_LAYER_GFX0) {
        bind_dev = VO_DEV_DHD0;
    } else if (layer == HAL_DISP_LAYER_GFX1) {
        bind_dev = VO_DEV_DHD1;
    } else if (layer == HAL_DISP_LAYER_GFX4) {
        bind_dev = VO_DEV_DSD0;
    } else {
        bind_dev = OT_INVALID_DEV;
    }
    return bind_dev;
}

td_s32 vo_drv_get_hal_gfx_layer(ot_vo_layer gfx_layer, hal_disp_layer *hal_layer)
{
    switch (gfx_layer) {
        case OT_VO_LAYER_G0:
            *hal_layer = HAL_DISP_LAYER_GFX0;
            break;

        case OT_VO_LAYER_G1:
            *hal_layer = HAL_DISP_LAYER_GFX1;
            break;

        case OT_VO_LAYER_G2:
            *hal_layer = HAL_DISP_LAYER_GFX2;
            break;

        case OT_VO_LAYER_G3:
            *hal_layer = HAL_DISP_LAYER_GFX3;
            break;

        case OT_VO_LAYER_G4:
            *hal_layer = HAL_DISP_LAYER_GFX4;
            break;

        default:
            return OT_ERR_VO_INVALID_LAYER_ID;
    }

    return TD_SUCCESS;
}

#endif

#if vo_desc("gfx")
td_s32 vo_drv_check_gfx_id(ot_vo_layer gfx_layer)
{
    if ((gfx_layer == VO_HAL_LAYER_G2) ||
        (gfx_layer == VO_HAL_LAYER_G3)) {
        return OT_ERR_VO_INVALID_LAYER_ID;
    }

    return TD_SUCCESS;
}

static td_void vo_drv_get_gfx_attr_max_rect(ot_vo_layer layer, td_u32 *width, td_u32 *height)
{
    *width = VO_GFX_DISP_RECT_W_MAX_OF_GFXX;
    *height = VO_GFX_DISP_RECT_H_MAX_OF_GFXX;

    if (layer == VO_HAL_LAYER_G4) {
        *width = VO_GFX_DISP_RECT_W_MAX_OF_GFX4;
        *height = VO_GFX_DISP_RECT_H_MAX_OF_GFX4;
    }
}

td_s32 vo_drv_check_gfx_attr_display_rect(ot_vo_layer layer, const ot_rect *rect)
{
    td_u32 max_width;
    td_u32 max_height;

    if ((layer >= VO_HAL_LAYER_G0) && (layer <= VO_HAL_LAYER_G4)) {
        vo_drv_get_gfx_attr_max_rect(layer, &max_width, &max_height);
        if ((rect->width > max_width) || (rect->height > max_height)) {
            vo_err_trace("gfx layer (%d) disp rect width(%d) height(%d) can't be larger than %dx%d!\n",
                layer, rect->width, rect->height,
                VO_GFX_DISP_RECT_W_MAX_OF_GFX4, VO_GFX_DISP_RECT_H_MAX_OF_GFX4);
            return OT_ERR_VO_ILLEGAL_PARAM;
        }
    } else {
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}
#endif