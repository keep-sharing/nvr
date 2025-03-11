// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ot_common.h"
#include "sys_drv.h"
#include "sys_hal.h"
#include "sys_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

td_s32 sys_drv_drv_ioctrl(const ot_mpp_chn *mpp_chn, sys_func func_id, const td_void *io_args)
{
    td_s32 dev = mpp_chn->dev_id;

    switch (func_id) {
        case SYS_VO_BUS_RESET_SEL: {
            td_bool *reset = (td_bool *)io_args;
            sys_hal_vo_bus_reset_sel(*reset);
            break;
        }

        case SYS_VO_CFG_CLK_EN: {
            td_bool *clk_en = (td_bool *)io_args;
            sys_hal_vo_cfg_clk_en(*clk_en);
            break;
        }

        case SYS_VO_APB_CLK_EN: {
            td_bool *clk_en = (td_bool *)io_args;
            sys_hal_vo_apb_clk_en(*clk_en);
            break;
        }

        case SYS_VO_BUS_CLK_EN: {
            td_bool *clk_en = (td_bool *)io_args;
            sys_hal_vo_bus_clk_en(*clk_en);
            break;
        }

        case SYS_VO_HD_CLKOUT_PHASIC_REVERSE_EN: {
            td_bool *clk_reverse_en = (td_bool *)io_args;
            sys_hal_vo_out_hd_phase_ctrl(dev, *clk_reverse_en);
            break;
        }

        case SYS_VO_OUT_DLY_TUNE: {
            td_u32 *dly_tune = (td_u32 *)io_args;
            sys_hal_vo_dly_tune(dev, *dly_tune);
            break;
        }

        case SYS_VO_OUT_HD_DIV: {
            td_u32 *div_mode = (td_u32 *)io_args;
            sys_hal_vo_out_hd_clk_div(dev, *div_mode);
            break;
        }

        case SYS_VO_DEV_DIV_MOD: {
            td_u32 *div_mod = (td_u32 *)io_args;
            sys_hal_vo_dev_div_mode(dev, *div_mod);
            break;
        }

        case SYS_VO_HD_CLK_SEL: {
            td_u32 *clk_sel = (td_u32 *)io_args;
            sys_hal_vo_hd_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_CORE_CLK_EN: {
            td_bool *clk_en = (td_bool *)io_args;
            sys_hal_vo_core_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_DEV_CLK_EN: {
            td_bool *clk_en = (td_bool *)io_args;
            sys_hal_vo_dev_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_DEV_DATE_CLK_EN: {
            td_bool *clk_en = (td_bool *)io_args;
            sys_hal_vo_sd_date_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_SD_CLK_SEL: {
            td_u32 *clk_sel = (td_u32 *)io_args;
            sys_hal_vo_sd_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_BT1120_CLK_EN: {
            td_bool *bt1120_clk_en = (td_bool *)io_args;
            sys_hal_vo_bt_clk_en(dev, *bt1120_clk_en);
            break;
        }

        case SYS_VO_BT1120_CLK_SEL: {
            td_u32 *bt1120_clk_ch_sel = (td_u32 *)io_args;
            sys_hal_vo_bt_clk_sel(dev, *bt1120_clk_ch_sel);
            break;
        }

        case SYS_VO_HD_DAC_SEL: {
            td_bool *clk_sel = (td_bool *)io_args;
            sys_hal_vo_hd_vdac_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_DEV_DAC_EN: {
            td_bool *clk_en = (td_bool *)io_args;
            sys_hal_vo_vdac_clk_en(dev, *clk_en);
            break;
        }

        case SYS_VO_DEV_DAC_DLY_TUNE: {
            td_u32 *dly_tune = (td_u32 *)io_args;
            sys_hal_vo_vdac_dly_tune(dev, *dly_tune);
            break;
        }

        case SYS_VO_HDMI_CLK_EN: {
            td_bool *hdmi_clk_en = (td_bool *)io_args;
            sys_hal_vo_hdmi_clk_en(dev, *hdmi_clk_en);
            break;
        }

        case SYS_VO_HDMI_CLK_SEL: {
            td_bool *clk_sel = (td_bool *)io_args;
            sys_hal_vo_hdmi_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_HDMI1_CLK_EN: {
            td_bool *hdmi_clk_en = (td_bool *)io_args;
            sys_hal_vo_hdmi1_clk_en(dev, *hdmi_clk_en);
            break;
        }

        case SYS_VO_HDMI1_CLK_SEL: {
            td_bool *clk_sel = (td_bool *)io_args;
            sys_hal_vo_hdmi1_clk_sel(dev, *clk_sel);
            break;
        }

        case SYS_VO_PLL_POSTDIV2_SET: {
            td_s32 pll = mpp_chn->dev_id;
            td_u32 bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_postdiv2(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_POSTDIV1_SET: {
            td_s32 pll = mpp_chn->dev_id;
            td_u32 bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_postdiv1(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_FRAC_SET: {
            td_s32 pll = mpp_chn->dev_id;
            td_u32 bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_frac(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_REFDIV_SET: {
            td_s32 pll = mpp_chn->dev_id;
            td_u32 bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_refdiv(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_FBDIV_SET: {
            td_s32 pll = mpp_chn->dev_id;
            td_u32 bits_set = *(td_u32 *)io_args;
            sys_hal_vo_set_pll_fbdiv(pll, bits_set);
            break;
        }

        case SYS_VO_PLL_POWER_CTRL: {
            td_s32 pll = mpp_chn->dev_id;
            td_bool *power_down = (td_bool *)io_args;
            sys_hal_vo_set_pll_power_ctrl(pll, *power_down);
            break;
        }

        default:
            return TD_FAILURE;
    }
    return TD_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif /* end of #ifdef __cplusplus */
