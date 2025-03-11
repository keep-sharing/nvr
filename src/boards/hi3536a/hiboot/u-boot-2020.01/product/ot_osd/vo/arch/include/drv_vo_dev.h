// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_DEV_H
#define DRV_VO_DEV_H

#include "ot_common.h"
#include "sys_ext.h"
#include "ot_common_vo.h"
#include "inner_vo.h"
#include "drv_vo_comm.h"
#include "drv_vo_dev_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

typedef enum {
    VO_CLK_DIV1 = 0,
    VO_CLK_DIV2 = 1,
    VO_CLK_DIV_BUTT
} vo_clk_div;

typedef enum {
    SSC_VDP_DIV_340_TO_600 = 0,  /* 340MHz~600MHz, 1div */
    SSC_VDP_DIV_200_TO_340 = 1,  /* 200MHz~340MHz, 2div */
    SSC_VDP_DIV_100_TO_200 = 3,  /* 100MHz~200MHz, 4div */
    SSC_VDP_DIV_50_TO_100  = 7,  /* 50MHz~100MHz, 8div */
    SSC_VDP_DIV_25_TO_50   = 15, /* 25MHz~50MHz, 16div */

    SSC_VDP_DIV_BUTT
} vo_hdmi_ssc_vdp_div_mode;

typedef struct {
    ot_vo_intf_sync index;
    ot_vo_pll pll;
    vo_hdmi_ssc_vdp_div_mode div;
} vo_pll_param;

typedef struct {
    ot_vo_intf_sync index;
    ot_vo_clk_src clk_src;
    union {
        ot_vo_fixed_clk fixed_clk;
        td_u32 lcd_mclk_div;
    };
} vo_fix_lcd_param;

typedef enum {
    VO_CLK_PLL_SRC_FOUTPOSTDIV = 0, /* FOUTVCO / (postdiv1 * postdiv2) */
    VO_CLK_PLL_SRC_FOUT4 = 1, /* FOUTVCO / (postdiv1 * postdiv2 * 8) */

    VO_CLK_PLL_SRC_BUTT
} vo_clk_pll_src;

const ot_vo_sync_info *vo_drv_get_dev_user_sync_timing(ot_vo_dev dev);
const ot_vo_user_sync_info *vo_drv_get_dev_user_sync_info(ot_vo_dev dev);

#if vo_desc("dev")
td_void vo_drv_set_dev_user_intf_sync_attr(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info);
td_void vo_drv_set_dev_div(ot_vo_dev dev, td_u32 dev_div);
td_void vo_drv_set_hdmi_div(ot_vo_dev dev, td_u32 pre_div);
td_s32 vo_drv_check_other_dev_exclution_intf(ot_vo_dev other_dev, ot_vo_intf_type intf_type);
td_s32 vo_drv_check_dev_clk_src(ot_vo_dev dev, ot_vo_clk_src clk_src);
td_s32 vo_drv_check_intf_user_dev_div(ot_vo_dev dev, td_u32 dev_div);
td_s32 vo_drv_check_dev_clkvalue(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info);
#endif

#if vo_desc("dev intf")
td_s32 vou_drv_check_hdmi_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync);
td_s32 vou_drv_check_bt_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync);
td_s32 vou_drv_check_vga_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync);
td_s32 vou_drv_check_rgb_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync);
td_s32 vou_drv_check_mipi_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync);
td_s32 vou_drv_check_dev_intf(ot_vo_dev dev, ot_vo_intf_type intf_type);
td_u32 vo_drv_get_vga_gain_def(td_void);
td_void vo_drv_set_hdmi1_param(ot_vo_dev dev, const ot_vo_hdmi_param *hdmi_param);
#endif

#if vo_desc("dev")
td_void vo_drv_set_clk_reverse(ot_vo_dev dev, td_bool reverse);
td_void vo_drv_open(ot_vo_dev dev);
td_void vo_drv_close(ot_vo_dev dev);

td_void vo_drv_dev_info_init(td_void);
ot_vo_layer vo_drv_get_layer(ot_vo_dev dev);
td_void vo_drv_default_setting(td_void);
td_s32 vo_drv_check_dev_id(ot_vo_dev dev);
td_s32 vo_drv_check_layer_id(ot_vo_layer layer);
td_void vo_drv_set_dev_clk(ot_vo_dev dev);
td_void vo_drv_set_dev_clk_en(ot_vo_dev dev, td_bool clk_en);
td_void vo_drv_set_dev_clk_sel(ot_vo_dev dev, td_u32 clk_sel);
td_void vo_drv_dev_get_clk_pll_src(ot_vo_dev dev, td_u32 *pll_src);
td_void vo_drv_set_intf_dither(ot_vo_intf_type intf_type, td_u32 in_bit_width);
td_u32 vo_drv_dev_get_max_clk(ot_vo_dev dev);
#endif

td_void vo_drv_dev_get_pll_cfg(ot_vo_dev dev, ot_vo_pll *pll);
td_void vo_drv_get_pll_cfg_no_div(ot_vo_intf_sync intf_sync, ot_vo_pll *pll);
td_bool vo_drv_is_support_hdmi_ssc_vdp_div(td_void);
td_void vo_drv_get_hdmi_ssc_vdp_div(ot_vo_intf_sync intf_sync, td_u32 *hdmi_ssc_vdp_div);
td_void vo_drv_dev_set_pll_cfg(ot_vo_dev dev, ot_vo_pll *pll);
td_s32 vo_drv_check_dev_pll_ref_div(ot_vo_dev dev, td_u32 ref_div);

#if vo_desc("dev")
td_void vo_drv_set_all_crg_clk(td_bool clk_en);
td_void vo_lpw_bus_reset(td_bool reset);
#endif

td_bool vo_drv_is_dev_support_rgb(ot_vo_dev dev);
ot_vo_layer vo_drv_get_gfx_layer(ot_vo_dev dev);
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of DRV_VO_DEV_H */
