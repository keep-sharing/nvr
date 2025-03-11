// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_GFX_H
#define DRV_VO_GFX_H

#include "ot_type.h"
#include "ot_common_vo.h"
#include "mkp_vo.h"
#include "drv_vo.h"

td_s32 vo_drv_check_gfx_id(ot_vo_layer gfx_layer);
td_s32 vo_drv_get_hal_gfx_layer(ot_vo_layer gfx_layer, hal_disp_layer *hal_layer);
td_s32 vou_drv_get_gfx_bind_dev(ot_vo_layer layer);

td_s32 graphic_drv_init(td_void);
td_s32 vo_drv_check_gfx_layer_id(ot_vo_layer layer);
td_s32 vo_drv_check_gfx_attr_display_rect(ot_vo_layer layer, const ot_rect *rect);

td_s32 vo_drv_gfx_convert_gfx_attr(const ot_vo_gfx_attr *gfx_attr_in, ot_vo_gfx_attr *gfx_attr_out);
td_void vo_drv_gfx_open(ot_vo_layer gfx_id, const ot_vo_gfx_attr *gfx_attr);
td_void vo_drv_gfx_close(ot_vo_layer layer_id);
#endif
