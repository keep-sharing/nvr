// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_GFX_COMM_H
#define HAL_VO_GFX_COMM_H

#include "hal_vo_def.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("gfx uboot only")
td_void hal_gfx_set_layer_disp_rect(hal_disp_layer layer, const ot_rect *rect);
td_void hal_gfx_set_layer_video_rect(hal_disp_layer layer, const ot_rect *rect);
td_void hal_gfx_enable_layer(hal_disp_layer layer, td_u32 enable);
td_void hal_gfx_set_layer_data_fmt(hal_disp_layer layer, hal_disp_pixel_format data_fmt);
td_void hal_gfx_set_layer_in_rect(hal_disp_layer layer, const ot_rect *rect);
td_void hal_gfx_set_gfx_pre_mult(hal_disp_layer layer, td_u32 enable);
td_void hal_gfx_set_gfx_addr(hal_disp_layer layer, td_phys_addr_t l_addr);
td_void hal_gfx_set_gfx_stride(hal_disp_layer layer, td_u16 pitch);
td_void hal_gfx_set_src_resolution(hal_disp_layer layer, const ot_rect *rect);
#endif

#if vo_desc("gfx")
td_void hal_gfx_set_pixel_alpha_range(hal_disp_layer layer, td_u32 alpha_range);
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_GFX_COMM_H */
