// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_vo_comm.h"
#include "drv_vo.h"
#include "hal_vo_dev_comm.h"
#include "hal_vo_dev.h"
#include "hal_vo_video.h"
#include "ot_math.h"
#include <common.h>
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("dev drv api")

td_u32 vo_drv_get_dev_bg_color(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    return drv_dev_ctx->vou_attr.bg_color;
}

ot_vo_intf_type vo_drv_get_dev_intf_type(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    return drv_dev_ctx->vou_attr.intf_type;
}

ot_vo_intf_sync vo_drv_get_dev_intf_sync(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    return drv_dev_ctx->vou_attr.intf_sync;
}

td_void vou_drv_set_disp_max_size(ot_vo_dev dev, td_u32 max_width, td_u32 max_height)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    drv_dev_ctx->max_width = max_width;
    drv_dev_ctx->max_height = max_height;
}

#endif

#if vo_desc("dev")

td_void vo_drv_get_sync_info(ot_vo_dev dev, hal_disp_syncinfo *sync_info)
{
    hal_disp_syncinfo *hal_sync = TD_NULL;
    ot_vo_sync_info *vo_sync_info = TD_NULL;
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);

    /* user sync info */
    if (drv_dev_ctx->vou_attr.intf_sync == OT_VO_OUT_USER) {
        vo_sync_info = &drv_dev_ctx->vou_attr.sync_info;
        sync_info->syncm = vo_sync_info->syncm;
        sync_info->iop = vo_sync_info->iop;
        sync_info->intfb = vo_sync_info->intfb;
        sync_info->vact = vo_sync_info->vact;
        sync_info->vbb = vo_sync_info->vbb;
        sync_info->vfb = vo_sync_info->vfb;
        sync_info->hact = vo_sync_info->hact;
        sync_info->hbb = vo_sync_info->hbb;
        sync_info->hfb = vo_sync_info->hfb;
        sync_info->hmid = vo_sync_info->hmid;
        sync_info->bvact = vo_sync_info->bvact;
        sync_info->bvbb = vo_sync_info->bvbb;
        sync_info->bvfb = vo_sync_info->bvfb;
        sync_info->hpw = vo_sync_info->hpw;
        sync_info->vpw = vo_sync_info->vpw;
        sync_info->idv = vo_sync_info->idv;
        sync_info->ihs = vo_sync_info->ihs;
        sync_info->ivs = vo_sync_info->ivs;
        return;
    }

    /* standard sync info */
    hal_sync = vo_drv_comm_get_sync_timing(drv_dev_ctx->vou_attr.intf_sync);
    (td_void)memcpy_s(sync_info, sizeof(hal_disp_syncinfo), hal_sync, sizeof(hal_disp_syncinfo));
}

td_void vo_drv_set_sync_info(ot_vo_dev dev, const hal_disp_syncinfo *sync_info)
{
    vo_hal_intf_set_sync_info(dev, sync_info);
}

td_bool vo_drv_is_progressive(ot_vo_dev dev)
{
    hal_disp_syncinfo sync_info = { 0 };

    vo_drv_get_sync_info(dev, &sync_info);
    return sync_info.iop;
}

td_bool vou_drv_get_dev_enable(ot_vo_dev dev)
{
    td_bool intf_en = TD_FALSE;

    hal_disp_get_intf_enable(dev, &intf_en);
    return intf_en;
}

static td_s32 vo_drv_check_dev_pll_param(ot_vo_dev dev, const ot_vo_pll *pll)
{
    if ((pll->fb_div > 0xfff) || (pll->frac > 0xffffff) || (pll->ref_div > 0x3f) || (pll->ref_div == 0) ||
        (pll->post_div1 > 0x7) || (pll->post_div1 == 0) || (pll->post_div2 > 0x7) || (pll->post_div2 == 0)) {
        vo_err_trace("dev(%d) pll param (fb_div,frac,ref_div,post_div1,post_div2)="
            "(0x%x,0x%x,0x%x,0x%x,0x%x) illegal.\n",
            dev, pll->fb_div, pll->frac, pll->ref_div, pll->post_div1, pll->post_div2);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return vo_drv_check_dev_pll_ref_div(dev, pll->ref_div);
}

static td_s32 vo_drv_check_dev_pll_postdiv(ot_vo_dev dev, const ot_vo_pll *pll)
{
    if (pll->post_div1 < pll->post_div2) {
        vo_err_trace("dev(%d) pll postdiv (post_div1,post_div2)=(0x%x,0x%x) illegal, it should be div1 >= div2.\n",
            dev, pll->post_div1, pll->post_div2);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_pll_foutvco(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    td_u32 foutvco;
    td_u32 src_clk;
    td_u32 pixel_clk;
    td_u32 max_clk;
    const td_u32 pll_fout4_divisor = 8; /* 8: pll fout4 */
    const ot_vo_pll *pll = &(sync_info->user_sync_attr.vo_pll);

    /* 0x01000000: 2^24 */
    foutvco = VO_PLL_FREF * pll->fb_div / pll->ref_div + VO_PLL_FREF * pll->frac / VO_PLL_FRAC_PREC / pll->ref_div;
    if ((foutvco < VO_PLL_FOUTVCO_MIN) || (foutvco > VO_PLL_FOUTVCO_MAX)) {
        vo_err_trace("dev(%d) pll foutvco (fb_div,frac,ref_div)=(%u,%u,%u) illegal. foutvco %u, not in [%u, %u]\n", dev,
            pll->fb_div, pll->frac, pll->ref_div, foutvco, VO_PLL_FOUTVCO_MIN, VO_PLL_FOUTVCO_MAX);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    /* check the pixel clk */
    src_clk = foutvco * VO_CLK_MHZ_UNIT / (pll->post_div1 * pll->post_div2);
    if (sync_info->user_sync_attr.clk_src == OT_VO_CLK_SRC_PLL_FOUT4) {
        src_clk = src_clk / pll_fout4_divisor;
    }
    pixel_clk = src_clk / (sync_info->pre_div * sync_info->dev_div);
    max_clk = vo_drv_dev_get_max_clk(dev);
    if (pixel_clk > max_clk) {
        vo_err_trace("dev(%d) pll (fb_div,frac,ref_div,post_div1,post_div2,pre_div,dev_div)=(%u,%u,%u,%u,%u,%u,%u)"
            " is illegal. pixel clk %uKHz is larger than %uKHz.\n",
            dev, pll->fb_div, pll->frac, pll->ref_div, pll->post_div1, pll->post_div2, sync_info->pre_div,
            sync_info->dev_div, pixel_clk, max_clk);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

td_s32 vo_drv_check_dev_pll(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    td_s32 ret;

    ret = vo_drv_check_dev_pll_param(dev, &sync_info->user_sync_attr.vo_pll);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = vo_drv_check_dev_pll_postdiv(dev, &sync_info->user_sync_attr.vo_pll);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return vo_drv_check_dev_pll_foutvco(dev, sync_info);
}

td_s32 vou_drv_check_dev_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync)
{
    td_s32 ret = TD_SUCCESS;

    if ((intf_sync == OT_VO_OUT_960H_PAL) || (intf_sync == OT_VO_OUT_960H_NTSC)) {
        vo_err_trace("vo%d's intfsync %d illegal, vo doesn't support interlaced sync %d!\n", dev, intf_sync, intf_sync);
        return OT_ERR_VO_NOT_SUPPORT;
    }

#ifdef CONFIG_OT_HDMI_SUPPORT
    ret = vou_drv_check_hdmi_sync(dev, intf_type, intf_sync);
    if (ret != TD_SUCCESS) {
        return ret;
    }
#endif

#ifdef CONFIG_OT_VO_BT1120
    ret = vou_drv_check_bt_sync(dev, intf_type, intf_sync);
    if (ret != TD_SUCCESS) {
        return ret;
    }
#endif

#ifdef CONFIG_OT_VO_VGA
    ret = vou_drv_check_vga_sync(dev, intf_type, intf_sync);
    if (ret != TD_SUCCESS) {
        return ret;
    }
#endif

#ifdef CONFIG_OT_VO_CVBS
    ret = vo_drv_check_cvbs_sync(dev, intf_type, intf_sync);
    if (ret != TD_SUCCESS) {
        return ret;
    }
#endif

#ifdef CONFIG_OT_VO_MIPI
    ret = vou_drv_check_mipi_sync(dev, intf_type, intf_sync);
    if (ret != TD_SUCCESS) {
        return ret;
    }
#endif

#ifdef CONFIG_OT_VO_RGB
    ret = vou_drv_check_rgb_sync(dev, intf_type, intf_sync);
#endif

    return ret;
}

static td_s32 vo_drv_check_bool_value(td_bool bool_val)
{
    if ((bool_val != TD_TRUE) && (bool_val != TD_FALSE)) {
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_bool_sync_info(ot_vo_dev dev, const ot_vo_sync_info *sync_info)
{
    if ((vo_drv_check_bool_value(sync_info->syncm) != TD_SUCCESS) ||
        (vo_drv_check_bool_value(sync_info->iop) != TD_SUCCESS) ||
        (vo_drv_check_bool_value(sync_info->idv) != TD_SUCCESS) ||
        (vo_drv_check_bool_value(sync_info->ihs) != TD_SUCCESS) ||
        (vo_drv_check_bool_value(sync_info->ivs) != TD_SUCCESS)) {
        vo_err_trace("vo%d: sync's syncm(%u),iop(%u),idv(%u),ihs(%u),ivs(%u) should be %u or %u!\n", dev,
            sync_info->syncm, sync_info->iop, sync_info->idv, sync_info->ihs, sync_info->ivs, TD_TRUE, TD_FALSE);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_vertical_sync_info(ot_vo_dev dev, const ot_vo_sync_info *sync_info)
{
    if ((sync_info->vact < VO_MIN_USER_SYNC_VACT) || (sync_info->vact > VO_MAX_USER_SYNC_VACT)) {
        vo_err_trace("vo%d: sync's vact(%u) should be [%u, %u]!\n", dev, sync_info->vact, VO_MIN_USER_SYNC_VACT,
            VO_MAX_USER_SYNC_VACT);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if ((sync_info->vbb < VO_MIN_USER_SYNC_VBB) || (sync_info->vbb > VO_MAX_USER_SYNC_VBB)) {
        vo_err_trace("vo%d: sync's vbb(%u) should be [%u, %u]!\n", dev, sync_info->vbb, VO_MIN_USER_SYNC_VBB,
            VO_MAX_USER_SYNC_VBB);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if ((sync_info->vfb < VO_MIN_USER_SYNC_VFB) || (sync_info->vfb > VO_MAX_USER_SYNC_VFB)) {
        vo_err_trace("vo%d: sync's vfb(%u) should be [%u, %u]!\n", dev, sync_info->vfb, VO_MIN_USER_SYNC_VFB,
            VO_MAX_USER_SYNC_VFB);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_horizontal_sync_info(ot_vo_dev dev, const ot_vo_sync_info *sync_info)
{
    if ((sync_info->hact < VO_MIN_USER_SYNC_HACT) || (sync_info->hact > VO_MAX_USER_SYNC_HACT)) {
        vo_err_trace("vo%d: sync's hact(%u) should be [%u, %u]!\n", dev, sync_info->hact, VO_MIN_USER_SYNC_HACT,
            VO_MAX_USER_SYNC_HACT);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if (sync_info->hbb < VO_MIN_USER_SYNC_HBB) {
        vo_err_trace("vo%d: sync's hbb(%u) should be [%u, %u]!\n", dev, sync_info->hbb, VO_MIN_USER_SYNC_HBB,
            VO_MAX_USER_SYNC_HBB);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if (sync_info->hfb < VO_MIN_USER_SYNC_HFB) {
        vo_err_trace("vo%d: sync's hfb(%u) should be [%u, %u]!\n", dev, sync_info->hfb, VO_MIN_USER_SYNC_HFB,
            VO_MAX_USER_SYNC_HFB);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_bottom_sync_info(ot_vo_dev dev, const ot_vo_sync_info *sync_info)
{
    if ((sync_info->iop == 0) && ((sync_info->bvact == 0) || (sync_info->bvbb == 0) || (sync_info->bvfb == 0))) {
        vo_err_trace("vo%d: sync's bvact(%u) bvbb(%u) bvfb(%u) can't be 0 when interlaced timing!\n", dev,
            sync_info->bvact, sync_info->bvbb, sync_info->bvfb);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if (sync_info->bvact > VO_MAX_USER_SYNC_BVACT) {
        vo_err_trace("vo%d: sync's bvact(%u) should be [0, %u]!\n", dev, sync_info->bvact, VO_MAX_USER_SYNC_BVACT);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if (sync_info->bvbb > VO_MAX_USER_SYNC_BVBB) {
        vo_err_trace("vo%d: sync's bvbb(%u) should be [0, %u]!\n", dev, sync_info->bvbb, VO_MAX_USER_SYNC_BVBB);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if (sync_info->bvfb > VO_MAX_USER_SYNC_BVFB) {
        vo_err_trace("vo%d: sync's bvfb(%u) should be [0, %u]!\n", dev, sync_info->bvfb, VO_MAX_USER_SYNC_BVFB);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_pulse_sync_info(ot_vo_dev dev, const ot_vo_sync_info *sync_info)
{
    if (sync_info->hpw < VO_MIN_USER_SYNC_HPW) {
        vo_err_trace("vo%d: sync's hpw(%u) should be [%u, %u]!\n", dev, sync_info->hpw, VO_MIN_USER_SYNC_HPW,
            VO_MAX_USER_SYNC_HPW);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if ((sync_info->vpw < VO_MIN_USER_SYNC_VPW) || (sync_info->vpw > VO_MAX_USER_SYNC_VPW)) {
        vo_err_trace("vo%d: sync's vpw(%u) should be [%u, %u]!\n", dev, sync_info->vpw, VO_MIN_USER_SYNC_VPW,
            VO_MAX_USER_SYNC_VPW);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_drv_check_dev_sync_info(ot_vo_dev dev, const ot_vo_sync_info *sync_info)
{
    td_s32 ret;

    ret = vo_drv_check_dev_bool_sync_info(dev, sync_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = vo_drv_check_dev_vertical_sync_info(dev, sync_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = vo_drv_check_dev_horizontal_sync_info(dev, sync_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = vo_drv_check_dev_bottom_sync_info(dev, sync_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return vo_drv_check_dev_pulse_sync_info(dev, sync_info);
}

td_s32 vou_drv_check_dev_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr)
{
    td_s32 ret;

    if (vo_drv_is_phy_dev_hd(dev) || vo_drv_is_phy_dev_sd(dev)) {
        ret = vou_drv_check_dev_intf(dev, pub_attr->intf_type);
        if (ret != TD_SUCCESS) {
            vo_err_trace("vo%d check device's interface %d illegal!\n", dev, pub_attr->intf_type);
            return ret;
        }
    }

    if (pub_attr->intf_sync == OT_VO_OUT_USER) {
        /*
         * user defined sync will be welcome for all interface type
         * WARNING: user must make sure to config right sync, otherwise, screen doesn't work!
         */
        return vo_drv_check_dev_sync_info(dev, &(pub_attr->sync_info));
    }

    return vou_drv_check_dev_sync(dev, pub_attr->intf_type, pub_attr->intf_sync);
}

td_void vo_drv_get_dev_reg_up(ot_vo_dev dev)
{
    td_u64 start;
    start = get_timer(0);

    do {
        if (get_timer(start) >= VO_DEV_REG_UP_MS_TIMEOUT) { /* 50 ms */
            break;
        }
    } while (hal_disp_get_reg_up(dev) != 0);
}

#endif

#if vo_desc("dev layer prio")

td_void vou_drv_set_layer_priority(ot_vo_dev dev, ot_vo_layer layer, td_u32 priority)
{
    hal_cbm_set_cbm_mixer_prio(layer, priority, dev);

    /* need regup */
    hal_disp_set_reg_up(dev);
}
#endif

#if vo_desc("dev intf")

td_void vou_drv_intf_csc_config(ot_vo_intf_type intf, const ot_vo_csc *csc)
{
    csc_coef coef;

    (td_void)memset_s(&coef, sizeof(csc_coef), 0x0, sizeof(csc_coef));
    vou_drv_calc_csc_matrix(csc, csc->csc_matrix, &coef);
    vo_drv_csc_trans_to_register(&coef);
    vo_hal_intf_set_csc_cfg(intf, &coef);
}

#ifdef CONFIG_OT_VO_VGA
td_void vo_drv_set_vga_param(ot_vo_dev dev, const ot_vo_vga_param *vga_param)
{
    vo_hal_intf_set_csc_enable(OT_VO_INTF_VGA, TD_TRUE);
    vou_drv_intf_csc_config(OT_VO_INTF_VGA, &vga_param->csc);
    vo_hal_intf_set_dac_gc(OT_VO_INTF_VGA, vga_param->gain);
    vo_hal_intf_set_vga_hsp_cfg(TD_TRUE, (td_u32)vga_param->sharpen_strength);
    hal_disp_set_reg_up(dev);
}
#endif

#ifdef CONFIG_OT_HDMI_SUPPORT
static td_void vo_drv_set_hdmi_mode(ot_vo_dev dev, const ot_vo_hdmi_param *hdmi_param)
{
    if ((hdmi_param->csc.csc_matrix >= OT_VO_CSC_MATRIX_BT601LIMIT_TO_RGBFULL) &&
        (hdmi_param->csc.csc_matrix <= OT_VO_CSC_MATRIX_BT709FULL_TO_RGBLIMIT)) {
        hal_disp_set_hdmi_mode(dev, 1); /* 1: RGB */
    } else {
        hal_disp_set_hdmi_mode(dev, 0); /* 0: YUV */
    }
}

td_void vo_drv_set_hdmi_param(ot_vo_dev dev, const ot_vo_hdmi_param *hdmi_param)
{
    vo_hal_intf_set_csc_enable(OT_VO_INTF_HDMI, TD_TRUE);
    vou_drv_intf_csc_config(OT_VO_INTF_HDMI, &hdmi_param->csc);
    vo_drv_set_hdmi_mode(dev, hdmi_param);
    hal_disp_set_reg_up(dev);
}
#endif

#ifdef CONFIG_OT_VO_RGB
td_void vo_drv_set_rgb_param(ot_vo_dev dev, const ot_vo_rgb_param *rgb_param)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);

    vo_hal_intf_set_csc_enable(OT_VO_INTF_RGB_6BIT, TD_TRUE);
    vou_drv_intf_csc_config(OT_VO_INTF_RGB_6BIT, &rgb_param->csc);
    if (vo_drv_is_rgb_parallel_intf(dev_ctx->vou_attr.intf_type)) {
        /* only parallel interface */
        vo_hal_set_intf_rgb_component_order(rgb_param->rgb_inverted_en);
    }

    /* both parallel and serial interface */
    vo_hal_set_intf_rgb_bit_inverse(rgb_param->bit_inverted_en);
    hal_disp_set_reg_up(dev);
}
#endif

#ifdef CONFIG_OT_VO_BT1120
td_void vo_drv_set_bt_param(ot_vo_dev dev, const ot_vo_bt_param *bt_param)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);

    if (vo_drv_is_bt1120_intf(dev_ctx->vou_attr.intf_type)) {
        /* only bt.1120 interface */
        vo_hal_set_intf_bt_component_order(bt_param->yc_inverted_en);
    }

    /* both bt.1120 and bt.656 interface */
    vo_hal_set_intf_bt_bit_inverse(bt_param->bit_inverted_en);
    /* clk_edge is configured when dev enabled, not configured here */
    hal_disp_set_reg_up(dev);
}

td_void vo_drv_set_bt_param_clk_edge(ot_vo_dev dev, ot_vo_clk_edge clk_edge)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);

    /* clk_edge is configured when dev enabled, also set bt param */
    dev_ctx->clk_edge = clk_edge;
    dev_ctx->bt_param_config = TD_TRUE;
}

ot_vo_clk_edge vo_drv_get_bt_param_clk_edge(ot_vo_dev dev)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);
    return dev_ctx->clk_edge;
}

#endif

#ifdef CONFIG_OT_VO_BT1120_DUAL_EDGE
td_void vo_drv_set_out_clk_div(td_u32 dev, td_u32 vo_out_clk_div)
{
    ot_mpp_chn mpp_chn = {0};

    vo_drv_set_dev_mpp_chn(dev, &mpp_chn);
    call_sys_drv_ioctrl(&mpp_chn, SYS_VO_OUT_HD_DIV, &vo_out_clk_div);
}

static vo_clk_div vo_drv_get_dev_clk_div(ot_vo_clk_edge clk_edge)
{
    return ((clk_edge == OT_VO_CLK_EDGE_DUAL) ? VO_CLK_DIV2 : VO_CLK_DIV1);
}

static ot_vo_clk_edge vo_drv_get_dev_clk_edge(ot_vo_dev dev, ot_vo_clk_edge def_clk_edge)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);
    if (dev_ctx->bt_param_config != TD_TRUE) {
        dev_ctx->clk_edge = def_clk_edge;   /* set drv clk edge value to default */
    }

    return dev_ctx->clk_edge;
}

static td_u32 vo_drv_get_user_sync_pixel_clk(const ot_vo_user_sync_info *sync_info)
{
    td_u32 foutvco;
    td_u32 src_clk;
    td_u32 pixel_clk;
    const ot_vo_pll *pll = &(sync_info->user_sync_attr.vo_pll);

    foutvco = VO_PLL_FREF * pll->fb_div / pll->ref_div + VO_PLL_FREF * pll->frac / VO_PLL_FRAC_PREC / pll->ref_div;
    src_clk = foutvco * VO_CLK_MHZ_UNIT / (pll->post_div1 * pll->post_div2);
    pixel_clk = src_clk / (sync_info->pre_div * sync_info->dev_div);
    return pixel_clk;
}

static ot_vo_clk_edge vo_drv_get_user_sync_def_clk_edge(const ot_vo_user_sync_info *sync_info)
{
    /* more than 4K BT1120 default 2div, dual */
    td_u32 pixel_clk = vo_drv_get_user_sync_pixel_clk(sync_info);
    return (pixel_clk >= VO_PLL_MAX_CLK_297000) ? OT_VO_CLK_EDGE_DUAL : OT_VO_CLK_EDGE_SINGLE;
}

td_void vo_drv_set_dev_user_sync_div_clk(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);
    ot_vo_clk_edge clk_edge, def_clk_edge;
    vo_clk_div clk_div;

    if (vo_drv_is_bt1120_intf(dev_ctx->vou_attr.intf_type) != TD_TRUE) {
        return;
    }

    def_clk_edge = vo_drv_get_user_sync_def_clk_edge(sync_info);
    clk_edge = vo_drv_get_dev_clk_edge(dev, def_clk_edge);
    clk_div = vo_drv_get_dev_clk_div(clk_edge);
    vo_drv_set_out_clk_div(dev, clk_div);
}

static ot_vo_clk_edge vo_drv_get_dev_def_clk_edge(ot_vo_intf_sync intf_sync)
{
    /* more than 4K BT1120 default 2div, dual */
    return (intf_sync >= OT_VO_OUT_3840x2160_24) ? OT_VO_CLK_EDGE_DUAL : OT_VO_CLK_EDGE_SINGLE;
}

td_void vo_drv_set_dev_div_clk(ot_vo_dev dev)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);
    ot_vo_clk_edge clk_edge, def_clk_edge;
    vo_clk_div clk_div;

    if (vo_drv_is_bt1120_intf(dev_ctx->vou_attr.intf_type) != TD_TRUE) {
        return;
    }

    def_clk_edge = vo_drv_get_dev_def_clk_edge(dev_ctx->vou_attr.intf_sync);
    clk_edge = vo_drv_get_dev_clk_edge(dev, def_clk_edge);
    clk_div = vo_drv_get_dev_clk_div(clk_edge);
    vo_drv_set_out_clk_div(dev, clk_div);
}
#endif

#ifdef CONFIG_OT_VO_MIPI
td_void vo_drv_set_mipi_param(ot_vo_dev dev, const ot_vo_mipi_param *mipi_param)
{
    vo_hal_intf_set_csc_enable(OT_VO_INTF_MIPI, TD_TRUE);
    vou_drv_intf_csc_config(OT_VO_INTF_MIPI, &mipi_param->csc);
    hal_disp_set_reg_up(dev);
}
#endif

td_void vo_drv_set_dev_mpp_chn(ot_vo_dev dev, ot_mpp_chn *mpp_chn)
{
    mpp_chn->mod_id = OT_ID_VO;
    mpp_chn->dev_id = dev;
    mpp_chn->chn_id = 0;
}

td_bool vo_drv_is_hdmi_intf(ot_vo_intf_type intf_type)
{
    if (intf_type & OT_VO_INTF_HDMI) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_hdmi1_intf(ot_vo_intf_type intf_type)
{
    if (intf_type & OT_VO_INTF_HDMI1) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_bt_intf(ot_vo_intf_type intf_type)
{
    if ((intf_type & OT_VO_INTF_BT1120) || (intf_type & OT_VO_INTF_BT656)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_bt1120_intf(ot_vo_intf_type intf_type)
{
    if ((intf_type & OT_VO_INTF_BT1120)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_bt656_intf(ot_vo_intf_type intf_type)
{
    if ((intf_type & OT_VO_INTF_BT656)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_vga_intf(ot_vo_intf_type intf_type)
{
    if (intf_type & OT_VO_INTF_VGA) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_cvbs_intf(ot_vo_intf_type intf_type)
{
    if (intf_type & OT_VO_INTF_CVBS) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

#ifdef CONFIG_OT_VO_RGB

td_bool vo_drv_is_rgb_intf(ot_vo_intf_type intf_type)
{
    if ((intf_type & OT_VO_INTF_RGB_6BIT) || (intf_type & OT_VO_INTF_RGB_8BIT) || (intf_type & OT_VO_INTF_RGB_16BIT) ||
        (intf_type & OT_VO_INTF_RGB_18BIT) || (intf_type & OT_VO_INTF_RGB_24BIT)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_rgb_serial_intf(ot_vo_intf_type intf_type)
{
    if ((intf_type & OT_VO_INTF_RGB_6BIT) || (intf_type & OT_VO_INTF_RGB_8BIT)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

td_bool vo_drv_is_rgb_parallel_intf(ot_vo_intf_type intf_type)
{
    if ((intf_type & OT_VO_INTF_RGB_16BIT) || (intf_type & OT_VO_INTF_RGB_18BIT) ||
        (intf_type & OT_VO_INTF_RGB_24BIT)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

#endif

td_bool vo_drv_is_mipi_intf(ot_vo_intf_type intf_type)
{
    if ((intf_type & OT_VO_INTF_MIPI) || (intf_type & OT_VO_INTF_MIPI_SLAVE)) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

static td_s32 vo_drv_check_same_intf(ot_vo_dev dev, ot_vo_dev other_dev, ot_vo_intf_type intf_type)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(other_dev);
    if (intf_type & drv_dev_ctx->vou_attr.intf_type) {
        vo_err_trace("vo(%d), the intf %d is used in dev(%d)!\n",
            dev, intf_type & drv_dev_ctx->vou_attr.intf_type, other_dev);
        return OT_ERR_VO_NOT_SUPPORT;
    }
    return TD_SUCCESS;
}

td_s32 vo_drv_check_intf_share(ot_vo_dev dev, ot_vo_intf_type intf_type)
{
    ot_vo_dev other_dev;
    vo_drv_dev *drv_dev_ctx = TD_NULL;
    td_s32 ret;

    if ((dev != VO_DEV_DHD0) && (dev != VO_DEV_DHD1)) {
        return TD_SUCCESS;
    }

    for (other_dev = VO_DEV_DHD0; other_dev <= VO_DEV_DHD1; other_dev++) {
        if (other_dev == dev) {
            continue;
        }

        drv_dev_ctx = vo_drv_get_dev_ctx(other_dev);
        if (drv_dev_ctx->config != TD_TRUE) {
            continue;
        }

        ret = vo_drv_check_same_intf(dev, other_dev, intf_type);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        ret = vo_drv_check_other_dev_exclution_intf(other_dev, intf_type);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }
    return TD_SUCCESS;
}
#endif

#if vo_desc("check dev id")

td_s32 vo_drv_check_dev_id_pub(ot_vo_dev dev)
{
    if ((dev < 0) || (dev >= OT_VO_MAX_DEV_NUM)) {
        return OT_ERR_VO_INVALID_DEV_ID;
    }
    return TD_SUCCESS;
}

#endif

#if vo_desc("check layer id")
td_s32 vo_drv_check_layer_id_pub(ot_vo_layer layer)
{
    if ((layer < 0) || (layer >= OT_VO_MAX_LAYER_NUM)) {
        return OT_ERR_VO_INVALID_LAYER_ID;
    }
    return TD_SUCCESS;
}

#endif

td_void vo_drv_set_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);

    (td_void)memcpy_s(&drv_dev_ctx->vou_attr, sizeof(ot_vo_pub_attr), pub_attr, sizeof(ot_vo_pub_attr));
    drv_dev_ctx->config = TD_TRUE;
}

td_void vo_drv_enable(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    drv_dev_ctx->vo_enable = TD_TRUE;
}

td_void vo_drv_disable(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);

    drv_dev_ctx->config = TD_FALSE;
    drv_dev_ctx->vo_enable = TD_FALSE;
    drv_dev_ctx->bt_param_config = TD_FALSE;
    drv_dev_ctx->clk_edge = OT_VO_CLK_EDGE_SINGLE;
    (td_void)memset_s(&drv_dev_ctx->vou_attr, sizeof(ot_vo_pub_attr), 0, sizeof(ot_vo_pub_attr));
}

td_u32 vou_drv_get_dev_layer_num(ot_vo_dev dev)
{
    vo_drv_dev *dev_ctx = vo_drv_get_dev_ctx(dev);
    return dev_ctx->layer_num;
}

td_void vou_drv_get_dev_layer(ot_vo_dev dev, ot_vo_layer *layer, td_u32 vo_layer_num)
{
    td_u32 layer_num;
    td_u32 i, j;
    vo_drv_layer *layer_ctx = TD_NULL;

    layer_num = vou_drv_get_dev_layer_num(dev);
    if (layer_num > vo_layer_num) {
        vo_err_trace("the layer num(%u) is larger than the input num(%u).\n", layer_num, vo_layer_num);
        layer_num = vo_layer_num;
    }

    for (i = 0, j = 0; i < OT_VO_MAX_LAYER_NUM; i++) {
        layer_ctx = vo_drv_get_layer_ctx(i);
        if (dev == layer_ctx->bind_dev) {
            *(layer + j) = i;
            j++;
        }

        if (j == layer_num) {
            return;
        }
    }
}

#if vo_desc("dev")
td_bool vo_drv_is_phy_dev_uhd(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);

    if (drv_dev_ctx->dev_cap.dev_type == VO_UHD_HW_DEV) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

td_bool vo_drv_is_phy_dev_hd(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    vo_dev_capability *dev_cap = &drv_dev_ctx->dev_cap;

    if ((dev_cap->dev_type == VO_UHD_HW_DEV) || (dev_cap->dev_type == VO_HD_HW_DEV)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

td_bool vo_drv_is_phy_dev_sd(ot_vo_dev dev)
{
    vo_drv_dev *drv_dev_ctx = vo_drv_get_dev_ctx(dev);
    if (drv_dev_ctx->dev_cap.dev_type == VO_SD_HW_DEV) {
        return TD_TRUE;
    }

    return TD_FALSE;
}
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
