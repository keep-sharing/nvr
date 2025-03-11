// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "hdmi_hal_intf.h"
#include "hdmi_hal_phy.h"
#include "hdmi_reg_crg.h"
#include "hdmi_product_define.h"
#include "boot_hdmi_intf.h"
#include "ubi_uboot.h"
#include "malloc.h"
#include "securec.h"

#define TMDS_MAX_CLK        600
#define TMDS_STABLE_GET_CNT 20

static hdmi_hal_cfg g_hal_cfg[HDMI_DEVICE_ID_BUTT];

static hdmi_hal_cfg *intf_hal_info_get(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        return &g_hal_cfg[hdmi_id];
    }
    return TD_NULL;
}

static td_void intf_tx_capability_init(hdmi_device_id hdmi_id)
{
    hdmi_hal_cfg *hal_cfg = intf_hal_info_get(hdmi_id);

    hdmi_if_null_return_void(hal_cfg);
    hal_cfg->tx_capability.tx_hdmi_14  = TD_TRUE;
    hal_cfg->tx_capability.tx_hdmi_20  = TD_TRUE;
    hal_cfg->tx_capability.tx_hdcp_14  = TD_TRUE;
    hal_cfg->tx_capability.tx_hdcp_22  = TD_TRUE;
    hal_cfg->tx_capability.tx_rgb444   = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr444 = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr422 = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr420 = TD_TRUE;
    hal_cfg->tx_capability.tx_deep_clr10_bit = TD_TRUE;
    hal_cfg->tx_capability.tx_deep_clr12_bit = TD_TRUE;
    hal_cfg->tx_capability.tx_deep_clr16_bit = TD_FALSE;
    hal_cfg->tx_capability.tx_rgb_ycbcr444 = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr444_422 = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr422_420 = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr420_422 = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr422_444 = TD_TRUE;
    hal_cfg->tx_capability.tx_ycbcr444_rgb = TD_TRUE;
    hal_cfg->tx_capability.tx_scdc         = TD_TRUE;
    hal_cfg->tx_capability.tx_max_tmds_clk = TMDS_MAX_CLK;

    return;
}

static td_void hal_hdmi_hardware_init(const struct ot_hdmi_hal_ *hdmi_hal)
{
    drv_hdmi_prod_crg_init(hdmi_hal->hal_ctx.hdmi_id);
    return;
}

static td_void hal_hdmi_tmds_mode_set(const struct ot_hdmi_hal_ *hdmi_hal, hdmi_tmds_mode tmds_mode)
{
    hdmi_if_null_return_void(hdmi_hal);
    hal_hdmi_ctrl_tmds_mode_set(hdmi_hal->hal_ctx.hdmi_id, tmds_mode);
    return;
}

static td_void hal_hdmi_avmute_set(const struct ot_hdmi_hal_ *hdmi_hal, td_bool avmute)
{
    hdmi_avmute_cfg avmute_cfg = {0};
    hdmi_if_null_return_void(hdmi_hal);
    hal_hdmi_ctrl_avmute_set(hdmi_hal->hal_ctx.hdmi_id, &avmute_cfg);
    return;
}

static td_void hal_hdmi_csc_param_set(struct ot_hdmi_hal_ *hdmi_hal, const hdmi_video_config *video_cfg)
{
    hdmi_if_null_return_void(hdmi_hal);
    hdmi_if_null_return_void(video_cfg);

    hal_hdmi_ctrl_csc_set(hdmi_hal->hal_ctx.hdmi_id, video_cfg);
    hdmi_hal->hal_ctx.video_cfg.in_colorspace  = video_cfg->in_colorspace;
    hdmi_hal->hal_ctx.video_cfg.out_colorspace = video_cfg->out_colorspace;
    hdmi_hal->hal_ctx.video_cfg.out_csc_quantization = video_cfg->out_csc_quantization;
    hdmi_hal->hal_ctx.video_cfg.conv_std = video_cfg->conv_std;

    return;
}

static td_void hal_hdmi_infoframe_set(const struct ot_hdmi_hal_ *hdmi_hal,
                                      hdmi_infoframe_id infoframe_id, const td_u8 *in_buffer)
{
    hdmi_if_null_return_void(hdmi_hal);
    hdmi_if_null_return_void(in_buffer);
    hal_hdmi_ctrl_infoframe_data_set(hdmi_hal->hal_ctx.hdmi_id, infoframe_id, in_buffer);
    return;
}

static td_void hal_hdmi_infoframe_enable_set(const struct ot_hdmi_hal_ *hdmi_hal,
                                             hdmi_infoframe_id infoframe_id, td_bool enable)
{
    hdmi_if_null_return_void(hdmi_hal);
    hal_hdmi_ctrl_infoframe_en_set(hdmi_hal->hal_ctx.hdmi_id, infoframe_id, enable);
    return;
}

static hdmi_deep_color depth_convert_to_deep_color(hdmi_video_bit_depth user_depth)
{
    hdmi_deep_color deep_color;

    switch (user_depth) {
        case HDMI_VIDEO_BITDEPTH_8:
            deep_color = HDMI_DEEP_COLOR_24BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_10:
            deep_color = HDMI_DEEP_COLOR_30BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_12:
            deep_color = HDMI_DEEP_COLOR_36BIT;
            break;
        case HDMI_VIDEO_BITDEPTH_16:
            deep_color = HDMI_DEEP_COLOR_48BIT;
            break;
        default:
            deep_color = HDMI_DEEP_COLOR_OFF;
            break;
    }

    return deep_color;
}
static td_s32 hal_hdmi_video_path_set(const struct ot_hdmi_hal_ *hdmi_hal, const hdmi_video_config *video_cfg)
{
    hdmi_video_path video_path = {0};
    hdmi_hal_cfg *hal_cfg = TD_NULL;
    hdmi_phy_tmds_cfg phy_tmds = {0};

    hdmi_if_null_return(hdmi_hal, TD_FAILURE);
    hdmi_if_null_return(video_cfg, TD_FAILURE);
    hal_cfg = intf_hal_info_get(hdmi_hal->hal_ctx.hdmi_id);
    hdmi_if_null_return(hal_cfg, TD_FAILURE);
    hal_cfg->tmds_clk = video_cfg->tmds_clk;

    video_path.in_colorspace = video_cfg->in_colorspace;
    video_path.out_colorspace = video_cfg->out_colorspace;
    video_path.in_deep_color = depth_convert_to_deep_color(video_cfg->in_bit_depth);
    video_path.out_deep_color = video_cfg->deep_color;
    video_path.out_hv_sync_pol.de_invert    = video_cfg->de_pol;
    video_path.out_hv_sync_pol.h_pol_invert = video_cfg->h_sync_pol;
    video_path.out_hv_sync_pol.v_pol_invert = video_cfg->v_sync_pol;

    switch (video_cfg->conv_std) {
        case HDMI_CONV_STD_BT_601:
            video_path.out_colormetry = HDMI_COLORMETRY_BT601;
            break;
        case HDMI_CONV_STD_BT_2020_NON_CONST_LUMINOUS:
            video_path.out_colormetry = HDMI_COLORMETRY_BT2020;
            break;
        case HDMI_CONV_STD_BT_2020_CONST_LUMINOUS:
            video_path.out_colormetry = HDMI_COLORMETRY_BT2020_CONST;
            break;
        default:
            video_path.out_colormetry = HDMI_COLORMETRY_BT709;
            break;
    }
    video_path.in_colormetry = video_path.out_colormetry;
    /* should set according to the sink; */
    video_path.in_quantization  = video_cfg->out_csc_quantization;
    video_path.out_quantization = video_cfg->out_csc_quantization;
    video_path.in_pixel_clk     = video_cfg->pixel_clk;
    video_path.out_tmds_clk     = video_cfg->tmds_clk;
    video_path.timing           = video_cfg->timing;

    hal_hdmi_ctrl_videopath_set(hdmi_hal->hal_ctx.hdmi_id, &video_path);
    /* PHY config */
    phy_tmds.emi_enable = video_cfg->emi_enable;
    phy_tmds.deep_color = video_cfg->deep_color;
    phy_tmds.pixel_clk  = video_cfg->pixel_clk;
    phy_tmds.tmds_clk   = video_cfg->tmds_clk;
    phy_tmds.mode_cfg   = HDMI_PHY_MODE_CFG_TMDS;
    phy_tmds.trace_len  = video_cfg->trace_len;
    (td_void)hal_hdmi_phy_tmds_set(hdmi_hal->hal_ctx.hdmi_id, &phy_tmds);

    return TD_SUCCESS;
}

static td_void hal_hdmi_ctrl_reset(const struct ot_hdmi_hal_ *hdmi_hal)
{
    td_u32 i;
    td_bool tmds_stable = TD_FALSE;
    td_bool output = TD_FALSE;

    hdmi_if_null_return_void(hdmi_hal);
    hal_hdmi_phy_oe_get(hdmi_hal->hal_ctx.hdmi_id, &output);
    if (output == TD_TRUE) {
        hdmi_info("oe enable, do not reset!\n");
        return;
    }
    hal_hdmi_ctrl_data_reset(hdmi_hal->hal_ctx.hdmi_id, TD_FALSE, 0);
    for (i = 0; (!tmds_stable) && (i < TMDS_STABLE_GET_CNT); i++) {
        mdelay(1);
        hal_hdmi_ctrl_tmds_stable_get(hdmi_hal->hal_ctx.hdmi_id, &tmds_stable);
    }
    hdmi_info("wait %ums, tmds_stable = %u\n", i, tmds_stable);

    return;
}

static td_void hal_hdmi_phy_output_enable_set(const struct ot_hdmi_hal_ *hdmi_hal, td_bool enable)
{
    hdmi_if_null_return_void(hdmi_hal);
    hdmi_if_fpga_return_void();
    hal_hdmi_phy_oe_set(hdmi_hal->hal_ctx.hdmi_id, enable);

    return;
}

static td_void hal_hdmi_phy_power_enable_set(const struct ot_hdmi_hal_ *hdmi_hal, td_bool enable)
{
    td_bool old_eable = TD_FALSE;
    hdmi_if_fpga_return_void();
    hal_hdmi_phy_power_get(hdmi_hal->hal_ctx.hdmi_id, &old_eable);
    if (enable != old_eable) {
        hal_hdmi_phy_power_set(hdmi_hal->hal_ctx.hdmi_id, enable);
    }

    return;
}

static td_void hal_hdmi_phy_set(const struct ot_hdmi_hal_ *hdmi_hal, const hdmi_phy_cfg *phy_cfg)
{
    hdmi_phy_tmds_cfg cfg = {0};

    hdmi_if_fpga_return_void();
    hdmi_if_null_return_void(phy_cfg);

    cfg.deep_color = phy_cfg->deep_color;
    cfg.emi_enable = phy_cfg->emi_enable;
    cfg.mode_cfg   = phy_cfg->mode_cfg;
    cfg.pixel_clk  = phy_cfg->pixel_clk;
    cfg.tmds_clk   = phy_cfg->tmds_clk;
    cfg.trace_len  = phy_cfg->trace_len;
    hal_hdmi_phy_tmds_set(hdmi_hal->hal_ctx.hdmi_id, &cfg);

    return;
}

static td_void hal_hdmi_tx_capability_get(const struct ot_hdmi_hal_ *hdmi_hal, hdmi_tx_capability *tx_capability)
{
    errno_t ret;
    hdmi_hal_cfg *hal_cfg = TD_NULL;

    hdmi_if_null_return_void(hdmi_hal);
    hdmi_if_null_return_void(tx_capability);
    hal_cfg = intf_hal_info_get(hdmi_hal->hal_ctx.hdmi_id);
    hdmi_if_null_return_void(hal_cfg);
    hdmi_if_false_return_void(hal_cfg->init);
    ret = memcpy_s(tx_capability, sizeof(*tx_capability), &hal_cfg->tx_capability, sizeof(hdmi_tx_capability));
    hdmi_unequal_eok(ret);
    return;
}

#ifdef HDMI_SCDC_SUPPORT
static td_void hal_hdmi_scdc_status_set(const struct ot_hdmi_hal_ *hdmi_hal, const hdmi_scdc_status *status)
{
    scdc_attr attr = {0};

    hdmi_if_null_return_void(hdmi_hal);
    hdmi_if_null_return_void(status);
    attr.sink_read_quest   = TD_FALSE;
    attr.sink_scramble     = status->sink_scramble_on;
    attr.src_scramble      = status->source_scramble_on;
    attr.tmds_clk_ratio40x = (status->tmds_bit_clk_ratio == HDMI_2_0_RATIO_1_40) ? TD_TRUE : TD_FALSE;
    attr.scramble_interval = status->scramble_interval;
    attr.scramble_timeout  = status->scramble_timeout;
    hal_hdmi_scdc_attr_set(hdmi_hal->hal_ctx.hdmi_id, &attr);

    return;
}

static td_void hal_hdmi_scdc_status_get(const struct ot_hdmi_hal_ *hdmi_hal, hdmi_scdc_status *status)
{
    scdc_attr attr = {0};

    hdmi_if_null_return_void(hdmi_hal);
    hdmi_if_null_return_void(status);
    hal_hdmi_scdc_attr_get(hdmi_hal->hal_ctx.hdmi_id, &attr);
    status->sink_read_quest    = TD_FALSE;
    status->sink_scramble_on   = attr.sink_scramble;
    status->source_scramble_on = attr.src_scramble;
    status->tmds_bit_clk_ratio = (attr.tmds_clk_ratio40x == TD_TRUE) ? HDMI_2_0_RATIO_1_40 : HDMI_1_4_RATIO_1_10;
    status->scramble_interval  = attr.scramble_interval;
    status->scramble_timeout   = attr.scramble_timeout;

    return;
}
#endif

static td_s32 hal_hdmi_phy_hw_spec_get(const struct ot_hdmi_hal_ *hdmi_hal, hdmi_hw_spec *hw_spec)
{
    td_s32 ret;
    hdmi_phy_hw_param phy_hw_param = {0};

    hdmi_if_fpga_return(TD_SUCCESS);
    hdmi_if_null_return(hdmi_hal, TD_FAILURE);
    hdmi_if_null_return(hw_spec, TD_FAILURE);

    ret = hal_hdmi_phy_params_get(hdmi_hal->hal_ctx.hdmi_id, &phy_hw_param);
    if (ret == TD_SUCCESS) {
        ret = memcpy_s(hw_spec, sizeof(*hw_spec), &phy_hw_param.spec_drv_use, sizeof(hdmi_hw_spec));
        hdmi_unequal_eok(ret);
    }

    return ret;
}

static td_s32 hal_hdmi_phy_hw_spec_set(const struct ot_hdmi_hal_ *hdmi_hal, td_u32 tmds_clk, hdmi_hal_hw_param param)
{
    td_s32 ret;

    hdmi_if_fpga_return(TD_SUCCESS);
    hdmi_if_null_return(hdmi_hal, TD_FAILURE);
    hdmi_if_null_return(param.hw_spec, TD_FAILURE);

    ret = hal_hdmi_phy_params_set(hdmi_hal->hal_ctx.hdmi_id, tmds_clk, param.hw_spec);

    return ret;
}

static td_void hal_pfn_init(hdmi_hal *hal)
{
    hal->hal_hdmi_hardware_init         = hal_hdmi_hardware_init;
    hal->hal_hdmi_avmute_set            = hal_hdmi_avmute_set;
    hal->hal_hdmi_tmds_mode_set         = hal_hdmi_tmds_mode_set;
    hal->hal_hdmi_infoframe_set         = hal_hdmi_infoframe_set;
    hal->hal_hdmi_infoframe_enable_set  = hal_hdmi_infoframe_enable_set;
    hal->hal_hdmi_video_path_set        = hal_hdmi_video_path_set;
    hal->hal_hdmi_phy_power_enable_set  = hal_hdmi_phy_power_enable_set;
    hal->hal_hdmi_phy_output_enable_set = hal_hdmi_phy_output_enable_set;
    hal->hal_hdmi_tx_capability_get     = hal_hdmi_tx_capability_get;
    hal->hal_hdmi_csc_param_set         = hal_hdmi_csc_param_set;
    hal->hal_hdmi_phy_set               = hal_hdmi_phy_set;
    hal->hal_hdmi_ctrl_reset            = hal_hdmi_ctrl_reset;
#ifdef HDMI_SCDC_SUPPORT
    hal->hal_hdmi_scdc_status_get = hal_hdmi_scdc_status_get;
    hal->hal_hdmi_scdc_status_set = hal_hdmi_scdc_status_set;
#endif
    hal->hal_hdmi_phy_hw_spec_get = hal_hdmi_phy_hw_spec_get;
    hal->hal_hdmi_phy_hw_spec_set = hal_hdmi_phy_hw_spec_set;

    return;
}

td_s32 hal_hdmi_open(const hdmi_hal_init *hal_init, hdmi_hal **hal_handle)
{
    hdmi_hal *hal = TD_NULL;
    hdmi_hal_cfg *hal_cfg = TD_NULL;

    hal = (hdmi_hal *)vmalloc(sizeof(hdmi_hal));
    if (hal == TD_NULL) {
        hdmi_err("Alloc hdmi_hal struct memory fail\n");
        return TD_FAILURE;
    }
    (td_void)memset_s(hal, sizeof(hdmi_hal), 0, sizeof(hdmi_hal));
    if (hal_init != TD_NULL) {
        hal->hal_ctx.callback  = hal_init->event_callback;
        hal->hal_ctx.hdmi_dev  = hal_init->event_data;
        hal->hal_ctx.hdmi_id   = hal_init->hdmi_dev_id;
        hal->hal_ctx.base_addr = hal_init->base_addr;
        hal->hal_ctx.phy_addr  = hal_init->phy_addr;
    } else {
        hdmi_info("hal_init null! open in boot!\n");
    }

    hal_cfg = intf_hal_info_get(hal->hal_ctx.hdmi_id);
    if (hal_cfg == TD_NULL) {
        vfree(hal);
        hdmi_err("hal_cfg null, fail!\n");
        return TD_FAILURE;
    }

    intf_tx_capability_init(hal->hal_ctx.hdmi_id);
#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_init(hal_init);
#endif
    hal_hdmi_ctrl_init(hal->hal_ctx.hdmi_id, hal_init);
    hal_hdmi_ddc_init(hal->hal_ctx.hdmi_id);
#ifdef HDMI_SCDC_SUPPORT
    hal_hdmi_scdc_init(hal->hal_ctx.hdmi_id);
#endif
    hal_pfn_init(hal);
    *hal_handle = hal;
    hal_cfg->init = TD_TRUE;

    return TD_SUCCESS;
}

td_void hal_hdmi_close(hdmi_hal **hal)
{
    hdmi_hal_cfg *hal_cfg = (hdmi_hal_cfg *)TD_NULL;

    hdmi_if_null_return_void(hal);
    hdmi_if_null_return_void(*hal);
    hal_cfg = intf_hal_info_get((*hal)->hal_ctx.hdmi_id);
    hdmi_if_null_return_void(hal_cfg);

#ifdef HDMI_SCDC_SUPPORT
    hal_hdmi_scdc_deinit((*hal)->hal_ctx.hdmi_id);
#endif
    hal_hdmi_ddc_deinit((*hal)->hal_ctx.hdmi_id);
    hal_hdmi_ctrl_deinit((*hal)->hal_ctx.hdmi_id);
#ifndef HDMI_FPGA_SUPPORT
    hal_hdmi_phy_deinit((*hal)->hal_ctx.hdmi_id);
#endif
    hdmi_reg_crg_deinit((*hal)->hal_ctx.hdmi_id);
    (*hal)->hal_ctx.base_addr = TD_NULL;
    (*hal)->hal_ctx.phy_addr  = TD_NULL;
    (td_void)memset_s(*hal, sizeof(hdmi_hal), 0, sizeof(hdmi_hal));
    vfree(*hal);
    *hal = TD_NULL;
    hal_cfg->init = TD_FALSE;

    return;
}

