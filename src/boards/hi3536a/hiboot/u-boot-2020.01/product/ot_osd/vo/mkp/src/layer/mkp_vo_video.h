// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef MKP_VO_VIDEO_H
#define MKP_VO_VIDEO_H

#include "ot_common_vo.h"
#include "ot_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define VO_DISP_1080_WIDTH 1920
#define VO_DISP_1080_HEIGHT 1080

td_bool vo_is_layer_enabled(ot_vo_layer layer);
td_bool vo_is_layer_configured(ot_vo_layer layer);
td_bool vo_is_layer_bypass(ot_vo_layer layer);
td_bool vo_is_layer_cluster_mode_enabled(ot_vo_layer layer);
ot_vo_dev vo_get_video_layer_bind_dev(ot_vo_layer layer);
td_s32 vo_check_layer_binded_dev(ot_vo_layer layer, ot_vo_dev dev);
td_bool vo_is_layer_binded_dev(ot_vo_layer layer);
td_bool vo_is_dev_layer_enabled(ot_vo_dev dev);
td_bool vo_is_layer_chn_enabled(ot_vo_layer layer);
td_void vo_exit_layer(td_void);

td_s32 vo_init_layer_ctx(ot_vo_layer layer);

td_void vo_init_layer_info(td_void);
td_void vo_init_layer_resource(td_void);
td_void vo_deinit_layer_info(td_void);

td_s32 vo_enable_video_layer(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr);
td_s32 vo_disable_video_layer(ot_vo_layer layer);

td_s32 vo_set_video_layer_attr(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef MKP_VO_VIDEO_H */

