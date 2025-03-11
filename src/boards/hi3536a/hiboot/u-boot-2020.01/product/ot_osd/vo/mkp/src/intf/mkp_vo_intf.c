// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "mkp_vo_intf.h"

#include "mkp_vo_dev.h"

#if vo_desc("UBOOT_VO")
#ifdef CONFIG_OT_HDMI_SUPPORT
td_void vo_init_hdmi_param(ot_vo_hdmi_param *hdmi_param)
{
    hdmi_param->csc.csc_matrix = OT_VO_CSC_MATRIX_BT601FULL_TO_BT601FULL;
    hdmi_param->csc.luma = VO_CSC_DEF_VAL;
    hdmi_param->csc.contrast = VO_CSC_DEF_VAL;
    hdmi_param->csc.hue = VO_CSC_DEF_VAL;
    hdmi_param->csc.saturation = VO_CSC_DEF_VAL;
    hdmi_param->csc.ex_csc_en = TD_FALSE;
}
#endif

#ifdef CONFIG_OT_VO_VGA
td_void vo_init_vga_param(ot_vo_vga_param *vga_param)
{
    vga_param->csc.csc_matrix = OT_VO_CSC_MATRIX_BT601FULL_TO_RGBFULL;
    vga_param->csc.luma = VO_CSC_DEF_VAL;
    vga_param->csc.contrast = VO_CSC_DEF_VAL;
    vga_param->csc.hue = VO_CSC_DEF_VAL;
    vga_param->csc.saturation = VO_CSC_DEF_VAL;
    vga_param->csc.ex_csc_en = TD_FALSE;
    vga_param->gain = vo_drv_get_vga_gain_def();
    vga_param->sharpen_strength = VO_VGA_SHARPEN_STRENGTH_DEF;
}
#endif

#ifdef CONFIG_OT_VO_RGB
td_void vo_init_rgb_param(ot_vo_rgb_param *rgb_param)
{
    rgb_param->csc.csc_matrix = OT_VO_CSC_MATRIX_BT601FULL_TO_RGBFULL;
    rgb_param->csc.luma = VO_CSC_DEF_VAL;
    rgb_param->csc.contrast = VO_CSC_DEF_VAL;
    rgb_param->csc.hue = VO_CSC_DEF_VAL;
    rgb_param->csc.saturation = VO_CSC_DEF_VAL;
    rgb_param->csc.ex_csc_en = TD_FALSE;
    rgb_param->rgb_inverted_en = TD_FALSE;
    rgb_param->bit_inverted_en = TD_FALSE;
}
#endif

#ifdef CONFIG_OT_VO_BT1120
td_void vo_init_bt_param(ot_vo_bt_param *bt_param)
{
    bt_param->yc_inverted_en = TD_FALSE;
    bt_param->bit_inverted_en = TD_FALSE;
    bt_param->clk_edge = OT_VO_CLK_EDGE_SINGLE;
}
#endif

#ifdef CONFIG_OT_VO_MIPI
td_void vo_init_mipi_param(ot_vo_mipi_param *mipi_param)
{
    mipi_param->csc.csc_matrix = OT_VO_CSC_MATRIX_BT601FULL_TO_RGBFULL;
    mipi_param->csc.luma = VO_CSC_DEF_VAL;
    mipi_param->csc.contrast = VO_CSC_DEF_VAL;
    mipi_param->csc.hue = VO_CSC_DEF_VAL;
    mipi_param->csc.saturation = VO_CSC_DEF_VAL;
    mipi_param->csc.ex_csc_en = TD_FALSE;
}
#endif

td_void vo_init_dev_intf_param(vo_dev_info *dev_ctx)
{
#ifdef CONFIG_OT_VO_HDMI
    vo_init_hdmi_param(&dev_ctx->hdmi_param);
#endif

#ifdef CONFIG_OT_VO_HDMI1
    vo_init_hdmi_param(&dev_ctx->hdmi1_param);
#endif

#ifdef CONFIG_OT_VO_VGA
    vo_init_vga_param(&dev_ctx->vga_param);
#endif

#ifdef CONFIG_OT_VO_RGB
    vo_init_rgb_param(&dev_ctx->rgb_param);
#endif

#ifdef CONFIG_OT_VO_BT1120
    vo_init_bt_param(&dev_ctx->bt_param);
#endif

#ifdef CONFIG_OT_VO_MIPI
    vo_init_mipi_param(&dev_ctx->mipi_param);
#endif
}

#if vo_desc("intf param")

td_void vo_set_dev_intf_param(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);
    ot_vo_intf_type intf_type = dev_ctx->vou_attr.intf_type;

#ifdef CONFIG_OT_VO_HDMI
    if (vo_drv_is_hdmi_intf(intf_type)) {
        vo_drv_set_hdmi_param(dev, &dev_ctx->hdmi_param);
    }
#endif

#ifdef CONFIG_OT_VO_HDMI1
    if (vo_drv_is_hdmi1_intf(intf_type)) {
        vo_drv_set_hdmi1_param(dev, &dev_ctx->hdmi1_param);
    }
#endif

#ifdef CONFIG_OT_VO_VGA
    if (vo_drv_is_vga_intf(intf_type)) {
        vo_drv_set_vga_param(dev, &dev_ctx->vga_param);
    }
#endif

#ifdef CONFIG_OT_VO_RGB
    if ((vo_drv_is_dev_support_rgb(dev) == TD_TRUE) &&
        (vo_drv_is_rgb_intf(intf_type))) {
        vo_drv_set_rgb_param(dev, &dev_ctx->rgb_param);
    }
#endif

#ifdef CONFIG_OT_VO_BT1120
    if (vo_drv_is_bt_intf(intf_type)) {
        /* clk edge maybe configured when vo enable, so refresh the value */
        dev_ctx->bt_param.clk_edge = vo_drv_get_bt_param_clk_edge(dev);
        vo_drv_set_bt_param(dev, &dev_ctx->bt_param);
    }
#endif

#ifdef CONFIG_OT_VO_MIPI
    if (vo_drv_is_mipi_intf(intf_type)) {
        vo_drv_set_mipi_param(dev, &dev_ctx->mipi_param);
    }
#endif
}

#endif /* #if vo_desc("intf param") */

#endif /* #if vo_desc("UBOOT_VO") */
