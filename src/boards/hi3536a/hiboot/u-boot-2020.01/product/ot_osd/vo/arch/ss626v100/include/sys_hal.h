// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ot_type.h"
#include "ot_common.h"

#ifndef SYS_HAL_H
#define SYS_HAL_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

td_s32 sys_hal_vo_bus_reset_sel(td_bool reset);
td_s32 sys_hal_vo_cfg_clk_en(td_bool clk_en);
td_s32 sys_hal_vo_apb_clk_en(td_bool clk_en);
td_s32 sys_hal_vo_bus_clk_en(td_bool clk_en);
td_s32 sys_hal_vo_out_hd_phase_ctrl(td_s32 vo_dev, td_bool reverse);
td_s32 sys_hal_vo_dly_tune(td_s32 vo_dev, td_u32 dly_tune);
td_s32 sys_hal_vo_out_hd_clk_div(td_s32 vo_dev, td_u32 clk_div);
td_s32 sys_hal_vo_hd_clk_sel(td_s32 dev, td_u32 clk_sel);
td_s32 sys_hal_vo_core_clk_en(td_s32 dev, td_bool clk_en);
td_s32 sys_hal_vo_dev_clk_en(td_s32 vo_dev, td_bool clk_en);
td_s32 sys_hal_vo_sd_date_clk_en(td_s32 vo_dev, td_bool clk_en);
td_s32 sys_hal_vo_dev_div_mode(td_s32 vo_dev, td_u32 div_mod);
td_s32 sys_hal_vo_sd_clk_sel(td_s32 dev, td_u32 clk_sel);
td_s32 sys_hal_vo_bt_clk_en(td_s32 vo_dev, td_bool bt_clk_en);
td_s32 sys_hal_vo_bt_clk_sel(td_s32 vo_dev, td_u32 bt_clk_ch_sel);
td_s32 sys_hal_vo_hd_vdac_clk_sel(td_s32 vo_dev, td_bool clk_sel);
td_s32 sys_hal_vo_vdac_clk_en(td_s32 vo_dev, td_bool clk_en);
td_s32 sys_hal_vo_vdac_dly_tune(td_s32 vo_dev, td_u32 dly_tune);
td_s32 sys_hal_vo_hdmi_clk_en(td_s32 vo_dev, td_bool hdmi_clk_en);
td_s32 sys_hal_vo_hdmi_clk_sel(td_s32 vo_dev, td_u32 clk_ch_sel);
td_s32 sys_hal_vo_hdmi1_clk_en(td_s32 vo_dev, td_bool hdmi_clk_en);
td_s32 sys_hal_vo_hdmi1_clk_sel(td_s32 vo_dev, td_u32 clk_ch_sel);
td_s32 sys_hal_vo_set_pll_postdiv2(td_s32 pll, td_u32 bits_set);
td_s32 sys_hal_vo_set_pll_postdiv1(td_s32 pll, td_u32 bits_set);
td_s32 sys_hal_vo_set_pll_frac(td_s32 pll, td_u32 bits_set);
td_s32 sys_hal_vo_set_pll_refdiv(td_s32 pll, td_u32 bits_set);
td_s32 sys_hal_vo_set_pll_fbdiv(td_s32 pll, td_u32 bits_set);
td_s32 sys_hal_vo_set_pll_power_ctrl(td_s32 pll, td_bool power_down);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef SYS_HAL_H */
