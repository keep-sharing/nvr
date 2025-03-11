// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef MKP_VO_INTF_H
#define MKP_VO_INTF_H

#include "ot_common_vo.h"
#include "mkp_vo_dev.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#ifdef CONFIG_OT_VO_VGA
td_void vo_init_vga_param(ot_vo_vga_param *vga_param);
#endif
#ifdef CONFIG_OT_HDMI_SUPPORT
td_void vo_init_hdmi_param(ot_vo_hdmi_param *hdmi_param);
#endif
#ifdef CONFIG_OT_VO_RGB
td_void vo_init_rgb_param(ot_vo_rgb_param *rgb_param);
#endif
#ifdef CONFIG_OT_VO_BT1120
td_void vo_init_bt_param(ot_vo_bt_param *bt_param);
#endif
#ifdef CONFIG_OT_VO_MIPI
td_void vo_init_mipi_param(ot_vo_mipi_param *mipi_param);
#endif
td_void vo_init_dev_intf_param(vo_dev_info *dev_ctx);
td_void vo_set_dev_intf_param(ot_vo_dev dev);
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef MKP_VO_INTF_H */
