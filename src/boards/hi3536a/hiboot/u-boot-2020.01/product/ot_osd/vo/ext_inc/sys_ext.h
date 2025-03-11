// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef SYS_EXT_H
#define SYS_EXT_H

#include "ot_type.h"
#include "ot_common_video.h"
#include "sys_drv.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

typedef enum {
    SYS_VO_PLL_POSTDIV2_SET = 0,
    SYS_VO_PLL_POSTDIV1_SET,
    SYS_VO_PLL_FRAC_SET,
    SYS_VO_PLL_REFDIV_SET,
    SYS_VO_PLL_FBDIV_SET,
    SYS_VO_PLL_POWER_CTRL,
    SYS_VO_BUS_RESET_SEL,
    SYS_VO_CFG_CLK_EN,
    SYS_VO_APB_CLK_EN,
    SYS_VO_BUS_CLK_EN,
    SYS_VO_HD_CLKOUT_PHASIC_REVERSE_EN,
    SYS_VO_OUT_DLY_TUNE,
    SYS_VO_OUT_HD_DIV,
    SYS_VO_DEV_DIV_MOD,
    SYS_VO_PPC_CFG_CLK_SEL,
    SYS_VO_PPC_CLK_SEL,
    SYS_VO_PPC_SC_CLK_SEL,
    SYS_VO_HD_CLK_SEL,
    SYS_VO_CORE_CLK_EN,
    SYS_VO_DEV_CLK_EN,
    SYS_VO_DEV_DATE_CLK_EN,
    SYS_VO_SD_DATE_CLK_EN,
    SYS_VO_SD_CLK_SEL,
    SYS_VO_LCD_CLK_EN,
    SYS_VO_LCD_MCLK_DIV,
    SYS_VO_BT1120_CLK_EN,
    SYS_VO_BT1120_CLK_SEL,
    SYS_VO_HD_DAC_SEL,
    SYS_VO_DEV_DAC_EN,
    SYS_VO_DEV_DAC_DLY_TUNE,
    SYS_VO_HDMI_SSC_VDP_DIV,
    SYS_VO_HDMI_CLK_EN,
    SYS_VO_HDMI_CLK_SEL,
    SYS_VO_HDMI1_CLK_EN,
    SYS_VO_HDMI1_CLK_SEL,
    SYS_VO_MIPI_CLK_EN,
    SYS_VO_MIPI_CLK_SEL,

    SYS_BUTT,
} sys_func;

#define call_sys_drv_ioctrl(mpp_chn, func_id, io_args) \
    sys_drv_drv_ioctrl(mpp_chn, func_id, io_args)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* SYS_EXT_H */
