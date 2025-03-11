// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_DEV_H
#define HAL_VO_DEV_H

#include "hal_vo_dev_comm.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

td_void hal_sys_set_outstanding(td_void);

#if vo_desc("dev intf")
#define VO_CVBS_DATE_PAL 0x628412dc
#define VO_CVBS_DATE_NTSC 0x108412dc
#define VO_CVBS_DATE_PAL_GAIN 0x80808383
#define VO_CVBS_DATE_NTSC_GAIN 0x80818282
td_void vo_hal_intf_set_cvbs_dac_cfg(td_void);
td_void vo_hal_intf_set_date_cvbs_gain(td_u32 data);
td_void vo_hal_intf_set_date_cvbs_burst_start(td_void);
td_void hal_disp_set_hdmi_mode(ot_vo_dev dev, td_u32 color_space);
td_void hal_disp_set_hdmi1_mode(ot_vo_dev dev, td_u32 color_space);
td_void vo_hal_intf_set_hdmi1_sync_inv(const hal_disp_syncinv *inv);
td_void vo_hal_intf_set_mux_sel(ot_vo_dev dev, ot_vo_intf_type intf);
td_void vo_hal_intf_set_csc_enable(ot_vo_intf_type intf, td_bool enable);
td_void vo_hal_intf_set_csc_cfg(ot_vo_intf_type intf, const csc_coef *csc_cfg);
#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_DEV_H */
