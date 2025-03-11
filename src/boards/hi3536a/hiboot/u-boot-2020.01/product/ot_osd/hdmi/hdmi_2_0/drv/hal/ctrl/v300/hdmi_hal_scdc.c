// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "hdmi_hal_scdc.h"
#include "boot_hdmi_intf.h"
#include "hdmi_product_define.h"
#include "hdmi_hal_phy.h"
#include "hdmi_hal_ddc.h"
#include "hdmi_hal_intf.h"
#include "hdmi_reg_tx.h"
#include "securec.h"

#define SCDC_OE_SCRAMBLE_INTERVAL 5

typedef struct {
    td_bool src_scramble;
    td_bool sink_scramble;
    td_bool tmds_clk_ratio40x;
} scdc_scramble;

static scdc_info g_scdc_info[HDMI_DEVICE_ID_BUTT] = {{{0}}};

static scdc_info *scdc_info_get(hdmi_device_id hdmi_id)
{
    if (hdmi_id < HDMI_DEVICE_ID_BUTT) {
        return &g_scdc_info[hdmi_id];
    }
    return TD_NULL;
}

static td_s32 scdc_scramble_set(hdmi_device_id hdmi_id, const scdc_scramble *scramble)
{
    td_s32 ret;
    scdc_tmds_config ddc_config_byte = {0};
    ddc_cfg ddc_src_cfg = {0};

    hdmi_if_null_return(scramble, TD_FAILURE);
    /* source scramble */
    hdmi_enc_scr_on_set(hdmi_id, scramble->src_scramble);
    /* sink scramble */
    ddc_config_byte.u8.screambling_enable = scramble->sink_scramble ? TD_TRUE : TD_FALSE;
    ddc_config_byte.u8.tmds_bit_clock_ratio = scramble->tmds_clk_ratio40x ? TD_TRUE : TD_FALSE;
    hal_hdmi_ddc_default_cfg_get(hdmi_id, &ddc_src_cfg);
    ddc_src_cfg.func_type  = DDC_FUNC_TYPE_SCDC;
    ddc_src_cfg.issue_mode = DDC_MODE_WRITE_MUTIL_ACK;
    ddc_src_cfg.offset     = SCDC_OFFSET_TMDS_CONFIG;
    ddc_src_cfg.data       = &ddc_config_byte.byte;
    ddc_src_cfg.data_size  = 1;
    ret = hal_hdmi_ddc_issue(hdmi_id, &ddc_src_cfg);

    return (ret == ddc_src_cfg.data_size) ? TD_SUCCESS : TD_FAILURE;
}

static td_s32 scdc_scramble_get(hdmi_device_id hdmi_id, scdc_scramble *scramble)
{
    td_s32 ret;
    scdc_scramble_status ddc_sramble_byte = {0};
    scdc_tmds_config ddc_config_byte = {0};
    ddc_cfg ddc_sink_cfg = {0};

    hdmi_if_null_return(scramble, TD_FAILURE);
    /* source scrambled */
    scramble->src_scramble = hdmi_enc_scr_on_get(hdmi_id) ? TD_TRUE : TD_FALSE;
    hal_hdmi_ddc_default_cfg_get(hdmi_id, &ddc_sink_cfg);
    /* sink scrambled status */
    ddc_sink_cfg.func_type  = DDC_FUNC_TYPE_SCDC;
    ddc_sink_cfg.issue_mode = DDC_MODE_READ_MUTIL_NO_ACK;
    ddc_sink_cfg.data_size  = 1;
    ddc_sink_cfg.offset     = SCDC_OFFSET_SCRAMBLER_STATUS;
    ddc_sramble_byte.byte   = 0;
    ddc_sink_cfg.data       = &ddc_sramble_byte.byte;
    ret = hal_hdmi_ddc_issue(hdmi_id, &ddc_sink_cfg);
    scramble->sink_scramble = ddc_sramble_byte.u8.screambling_status ? TD_TRUE : TD_FALSE;
    /* sink tmds bit clock ratio & scramble cfg */
    ddc_sramble_byte.byte = 0;
    ddc_sink_cfg.data = &ddc_config_byte.byte;
    ddc_sink_cfg.offset = SCDC_OFFSET_TMDS_CONFIG;
    ret = hal_hdmi_ddc_issue(hdmi_id, &ddc_sink_cfg);
    scramble->tmds_clk_ratio40x = ddc_config_byte.u8.tmds_bit_clock_ratio ? TD_TRUE : TD_FALSE;

    return (ret == ddc_sink_cfg.data_size) ? TD_SUCCESS : TD_FAILURE;
}

td_void hal_hdmi_scdc_init(hdmi_device_id hdmi_id)
{
    scdc_info *info = scdc_info_get(hdmi_id);
    hdmi_if_null_return_void(info);
    if (!info->status.init) {
        (td_void)memset_s(info, sizeof(scdc_info), 0, sizeof(scdc_info));
        info->status.init = TD_TRUE;
    }
    return;
}

td_void hal_hdmi_scdc_deinit(hdmi_device_id hdmi_id)
{
    scdc_info *info = scdc_info_get(hdmi_id);
    hdmi_if_null_return_void(info);
    hdmi_if_false_return_void(info->status.init);
    (td_void)memset_s(info, sizeof(scdc_info), 0, sizeof(scdc_info));
    info->status.init = TD_FALSE;
    return;
}

static td_void scdc_attr_param_check(scdc_attr *attr)
{
    if ((attr->scramble_interval < SCDC_DEFAULT_SCRAMBLE_INTERVAL) ||
        (attr->scramble_interval > SCDC_DEFAULT_SCRAMBLE_TIMEOUT)) {
        attr->scramble_interval = SCDC_DEFAULT_SCRAMBLE_INTERVAL;
    }
    if (attr->scramble_timeout < SCDC_DEFAULT_SCRAMBLE_TIMEOUT) {
        attr->scramble_timeout = SCDC_DEFAULT_SCRAMBLE_TIMEOUT;
    } else if (attr->scramble_timeout > SCDC_MAX_SCRAMBLE_TIMEOUT) {
        attr->scramble_timeout = SCDC_MAX_SCRAMBLE_TIMEOUT;
    }

    return;
}

td_s32 hal_hdmi_scdc_attr_set(hdmi_device_id hdmi_id, scdc_attr *attr)
{
    td_s32 ret;
    td_bool oe_enable = TD_FALSE;
    td_u32 timeout;
    scdc_scramble scramble = {0};
    scdc_info *info = scdc_info_get(hdmi_id);

    hdmi_if_null_return(attr, TD_FAILURE);
    hdmi_if_null_return(info, TD_FAILURE);
    hdmi_if_false_return(info->status.init, TD_FAILURE);
    scdc_attr_param_check(attr);

    (td_void)memcpy_s(&info->attr, sizeof(info->attr), attr, sizeof(scdc_attr));
    mdelay(SCDC_DEFAULT_SCRAMBLE_INTERVAL);
    /* scrameble cfg */
    timeout = info->attr.scramble_interval;
    do {
        /* oe disable */
#ifndef HDMI_FPGA_SUPPORT
        hal_hdmi_phy_oe_set(hdmi_id, TD_FALSE);
        mdelay(SCDC_OE_SCRAMBLE_INTERVAL);
        hal_hdmi_phy_oe_get(hdmi_id, &oe_enable);
#endif
        /* sink & source scramble config */
        scramble.src_scramble = info->attr.src_scramble;
        scramble.sink_scramble = info->attr.sink_scramble;
        scramble.tmds_clk_ratio40x = info->attr.tmds_clk_ratio40x;
        scdc_scramble_set(hdmi_id, &scramble);
        /* oe enable */
#ifndef HDMI_FPGA_SUPPORT
        mdelay(timeout);
        hal_hdmi_phy_oe_set(hdmi_id, TD_TRUE);
        mdelay(timeout);
        hal_hdmi_phy_oe_get(hdmi_id, &oe_enable);
#endif
        /* poll status */
        scdc_scramble_get(hdmi_id, &scramble);
        mdelay(1);
        /* Compatibility try again */
        if ((scramble.tmds_clk_ratio40x == info->attr.tmds_clk_ratio40x)) {
            ret = TD_SUCCESS;
        } else {
            ret = TD_FAILURE;
        }
        timeout += info->attr.scramble_interval;
    } while ((ret != TD_SUCCESS) && (timeout <= info->attr.scramble_timeout));
    hdmi_info("\noe_enable = %u, timeout = %u, src_scramble = %u, sink_scramble = %u, tmds_clk_ratio40x = %u\n",
              oe_enable, timeout, scramble.src_scramble, scramble.sink_scramble, scramble.tmds_clk_ratio40x);

    if (ret != TD_SUCCESS) {
        hdmi_err("scdc scramble %s fail!\n", info->attr.sink_scramble ? "ON" : "OFF");
    } else {
        hdmi_info("scdc scramble %s success!\n", info->attr.sink_scramble ? "ON" : "OFF");
    }

    return ret;
}

td_void hal_hdmi_scdc_attr_get(hdmi_device_id hdmi_id, scdc_attr *attr)
{
    errno_t ret;
    scdc_scramble scramble = {0};
    scdc_info *info = scdc_info_get(hdmi_id);

    hdmi_if_null_return_void(attr);
    hdmi_if_null_return_void(info);
    hdmi_if_false_return_void(info->status.init);
    scdc_scramble_get(hdmi_id, &scramble);
    info->attr.sink_scramble = scramble.sink_scramble;
    info->attr.src_scramble = scramble.src_scramble;
    info->attr.tmds_clk_ratio40x = scramble.tmds_clk_ratio40x;
    ret = memcpy_s(attr, sizeof(*attr), &info->attr, sizeof(scdc_attr));
    hdmi_unequal_eok(ret);

    return;
}

