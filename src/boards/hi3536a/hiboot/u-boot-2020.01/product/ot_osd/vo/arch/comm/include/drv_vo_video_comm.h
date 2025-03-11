// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_VIDEO_COMM_H
#define DRV_VO_VIDEO_COMM_H

#include "drv_vo_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#define VO_ALPHA_OPACITY                    0xFF /* opacity alpha */

#if vo_desc("layer cfg")
td_void vou_drv_layer_enable(ot_vo_layer layer, td_bool enable);
td_void vo_drv_disable_layer(ot_vo_layer layer);
td_void vou_drv_set_layer_reg_up(ot_vo_layer layer);
td_void vou_drv_set_layer_data_fmt(ot_vo_layer layer, vou_layer_pixel_format data_fmt);
#endif

#if vo_desc("layer")
td_u32 vou_align_up(td_u32 val, td_u32 align);
td_void vou_drv_init_default_csc(ot_vo_layer layer);
td_void vou_drv_init_default_csc_param(ot_vo_layer layer);
td_void vou_drv_set_default_csc(ot_vo_layer layer, const ot_vo_csc *csc);
td_s32 vou_drv_get_video_layer_bind_dev(ot_vo_layer layer);
td_void vo_drv_set_layer_attr(ot_vo_layer layer, const ot_vo_video_layer_attr *vo_layer_attr);
td_void vou_drv_get_default_csc(ot_vo_layer layer, ot_vo_csc *csc);
#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* end of #ifdef __cplusplus */
#endif

#endif /* end of DRV_VO_COMM_LAYER_H */
