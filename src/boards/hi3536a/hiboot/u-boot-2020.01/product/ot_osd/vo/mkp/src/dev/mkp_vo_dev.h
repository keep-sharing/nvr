// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef MKP_VO_DEV_H
#define MKP_VO_DEV_H

#include "ot_common_vo.h"
#include "drv_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define VO_DEV_TIME_REF_STEP        2

#define VO_DEV_MAX_FRAME_RATE       240
#define VO_DISP_FREQ_VGA  60

typedef struct {
    ot_vo_intf_sync intf_sync;
    td_u32 width;
    td_u32 height;
    td_u32 frame_rate;
} vo_intf_sync_info;

typedef struct {
    td_bool vo_enable;
    td_bool config;
    ot_vo_pub_attr vou_attr;

    td_u32 max_width;
    td_u32 max_height;
    td_u32 full_frame_rate;

    td_bool user_config;
    ot_vo_user_sync_info vo_user_sync_info;
    ot_vo_vga_param vga_param;
    ot_vo_hdmi_param hdmi_param;
    ot_vo_hdmi_param hdmi1_param;
    ot_vo_rgb_param rgb_param;
    ot_vo_bt_param bt_param;
    ot_vo_mipi_param mipi_param;
} vo_dev_info;

td_void vo_init_dev_info(td_void);
vo_dev_info *vo_get_dev_ctx(ot_vo_dev vo_dev);
td_void vo_reset_dev_vars(ot_vo_dev dev);
td_s32 vo_init_dev_ctx(ot_vo_dev dev);

td_bool vo_is_dev_enabled(ot_vo_dev dev);
td_void vo_get_dev_max_size(ot_vo_dev dev, ot_size *dev_size);
td_bool vo_is_dev_interlaced_intf_sync(ot_vo_dev dev);

td_s32 vo_enable(ot_vo_dev dev);
td_s32 vo_disable(ot_vo_dev dev);

td_s32 vo_set_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr);
td_s32 vo_get_pub_attr(ot_vo_dev dev, ot_vo_pub_attr *pub_attr);

td_s32 vo_check_dev_user_div(ot_vo_dev dev, td_u32 dev_div, td_u32 pre_div);
td_s32 vo_set_user_sync_info(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info);

td_u32 vo_dev_get_bg_color(ot_vo_dev dev);
td_void vo_dev_set_bg_color(ot_vo_dev dev, td_u32 bg_color);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef MKP_VO_DEV_H */
