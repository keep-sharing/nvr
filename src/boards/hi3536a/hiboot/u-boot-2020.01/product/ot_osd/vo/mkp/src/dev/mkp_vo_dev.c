// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "mkp_vo_dev.h"
#include "mkp_vo_intf.h"
#include "mkp_vo_gfx.h"
#include "ot_math.h"
#include "mkp_vo.h"
#include "mkp_vo_init.h"
#include "mkp_vo_video.h"
#include "mkp_vo_comm.h"
#include "ot_board.h"
#include "common.h"
#include "securec.h"

#if vo_desc("dev pub api")
static vo_dev_info g_vo_dev[OT_VO_MAX_DEV_NUM] = {0};

vo_dev_info *vo_get_dev_ctx(ot_vo_dev vo_dev)
{
    return &(g_vo_dev[vo_dev]);
}

static td_void vo_init_pub_dev_ctx(vo_dev_info *dev_ctx)
{
    vo_init_dev_intf_param(dev_ctx);
}

td_s32 vo_init_dev_ctx(ot_vo_dev dev)
{
    td_u32 dev_ctx_len;
    vo_dev_info *dev_ctx = TD_NULL;

    dev_ctx = vo_get_dev_ctx(dev);

    dev_ctx_len = sizeof(vo_dev_info);

    (td_void)memset_s(dev_ctx, dev_ctx_len, 0, dev_ctx_len);
    vo_init_pub_dev_ctx(dev_ctx);
    return TD_SUCCESS;
}

td_void vo_init_dev_info(td_void)
{
    if ((vo_get_vo_init_flag() == TD_FALSE)) {
        vo_drv_dev_info_init();
    }
}

td_bool vo_is_dev_enabled(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = TD_NULL;

    if (vo_def_check_dev_id(dev) != TD_SUCCESS) {
        vo_warn_trace("Vo device id %d is invalid!\n", dev);
        return TD_FALSE;
    }

    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx == TD_NULL) {
        return TD_FALSE;
    }

    return dev_ctx->vo_enable;
}

static td_bool vo_is_dev_interlaced_user_intf_sync(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    if ((dev_ctx->vou_attr.intf_sync == OT_VO_OUT_USER) &&
        (dev_ctx->vou_attr.sync_info.iop == TD_FALSE)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

td_bool vo_is_dev_interlaced_typical_intf_sync(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);
    ot_vo_intf_sync intf_sync = dev_ctx->vou_attr.intf_sync;

    if ((intf_sync == OT_VO_OUT_PAL) ||
        (intf_sync == OT_VO_OUT_NTSC) ||
        (intf_sync == OT_VO_OUT_1080I50) ||
        (intf_sync == OT_VO_OUT_1080I60)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

td_bool vo_is_dev_interlaced_intf_sync(ot_vo_dev dev)
{
    if ((vo_is_dev_interlaced_typical_intf_sync(dev) == TD_TRUE) ||
        (vo_is_dev_interlaced_user_intf_sync(dev) == TD_TRUE)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

td_void vo_get_dev_max_size(ot_vo_dev dev, ot_size *dev_size)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);
    dev_size->width = dev_ctx->max_width;
    dev_size->height = dev_ctx->max_height;
}

#endif

#if vo_desc("intf sync info")
static vo_intf_sync_info g_vo_intf_sync_info[OT_VO_OUT_BUTT] = {
    { OT_VO_OUT_PAL,          720,  576,  25 },
    { OT_VO_OUT_NTSC,         720,  480,  30 },
    { OT_VO_OUT_960H_PAL,     960,  576,  25 },
    { OT_VO_OUT_960H_NTSC,    960,  480,  30 },

    { OT_VO_OUT_640x480_60,   640,  480,  60 },
    { OT_VO_OUT_480P60,       720,  480,  60 },
    { OT_VO_OUT_576P50,       720,  576,  50 },
    { OT_VO_OUT_800x600_60,   800,  600,  60 },
    { OT_VO_OUT_1024x768_60,  1024, 768,  60 },
    { OT_VO_OUT_720P50,       1280, 720,  50 },
    { OT_VO_OUT_720P60,       1280, 720,  60 },
    { OT_VO_OUT_1280x800_60,  1280, 800,  60 },
    { OT_VO_OUT_1280x1024_60, 1280, 1024, 60 },
    { OT_VO_OUT_1366x768_60,  1366, 768,  60 },
    { OT_VO_OUT_1400x1050_60, 1400, 1050, 60 },
    { OT_VO_OUT_1440x900_60,  1440, 900,  60 },
    { OT_VO_OUT_1680x1050_60, 1680, 1050, 60 },

    { OT_VO_OUT_1080P24,      1920, 1080, 24 },
    { OT_VO_OUT_1080P25,      1920, 1080, 25 },
    { OT_VO_OUT_1080P30,      1920, 1080, 30 },
    { OT_VO_OUT_1080I50,      1920, 1080, 25 },
    { OT_VO_OUT_1080I60,      1920, 1080, 30 },
    { OT_VO_OUT_1080P50,      1920, 1080, 50 },
    { OT_VO_OUT_1080P60,      1920, 1080, 60 },

    { OT_VO_OUT_1600x1200_60, 1600, 1200, 60 },
    { OT_VO_OUT_1920x1200_60, 1920, 1200, 60 },
    { OT_VO_OUT_1920x2160_30, 1920, 2160, 30 },
    { OT_VO_OUT_2560x1440_30, 2560, 1440, 30 },
    { OT_VO_OUT_2560x1440_60, 2560, 1440, 60 },
    { OT_VO_OUT_2560x1600_60, 2560, 1600, 60 },

    { OT_VO_OUT_3840x2160_24, 3840, 2160, 24 },
    { OT_VO_OUT_3840x2160_25, 3840, 2160, 25 },
    { OT_VO_OUT_3840x2160_30, 3840, 2160, 30 },
    { OT_VO_OUT_3840x2160_50, 3840, 2160, 50 },
    { OT_VO_OUT_3840x2160_60, 3840, 2160, 60 },
    { OT_VO_OUT_4096x2160_24, 4096, 2160, 24 },
    { OT_VO_OUT_4096x2160_25, 4096, 2160, 25 },
    { OT_VO_OUT_4096x2160_30, 4096, 2160, 30 },
    { OT_VO_OUT_4096x2160_50, 4096, 2160, 50 },
    { OT_VO_OUT_4096x2160_60, 4096, 2160, 60 },
    { OT_VO_OUT_7680x4320_30, 7680, 4320, 30 },

    { OT_VO_OUT_240x320_50,   240,  320,  50 },
    { OT_VO_OUT_320x240_50,   320,  240,  50 },
    { OT_VO_OUT_240x320_60,   240,  320,  60 },
    { OT_VO_OUT_320x240_60,   320,  240,  60 },
    { OT_VO_OUT_800x600_50,   800,  600,  50 },

    { OT_VO_OUT_720x1280_60,  720,  1280, 60 },
    { OT_VO_OUT_1080x1920_60, 1080, 1920, 60 },

    { OT_VO_OUT_USER,         1920, 1080, 60 },
};

static vo_intf_sync_info *vo_get_intf_sync_info(ot_vo_intf_sync intf_sync)
{
    return &g_vo_intf_sync_info[intf_sync];
}

static td_void vo_get_intf_sync_size(ot_vo_intf_sync intf_sync, td_u32 *width, td_u32 *height)
{
    vo_intf_sync_info *sync_info = TD_NULL;

    sync_info = vo_get_intf_sync_info(intf_sync);
    if (sync_info->intf_sync == intf_sync) {
        *width = sync_info->width;
        *height = sync_info->height;
    }
}

static td_void vo_get_intf_sync_frame_rate(ot_vo_intf_sync intf_sync, td_u32 *frame_rate)
{
    vo_intf_sync_info *sync_info = TD_NULL;

    if (vo_is_typical_intf_sync(intf_sync) != TD_TRUE) {
        return;
    }

    sync_info = vo_get_intf_sync_info(intf_sync);
    if (sync_info->intf_sync == intf_sync) {
        *frame_rate = sync_info->frame_rate;
    }
}

static td_void vo_get_intf_sync_size_user(const ot_vo_sync_info *sync_info, td_u32 *width, td_u32 *height)
{
    *width = sync_info->hact;
    *height = (sync_info->iop) ? (sync_info->vact) : (sync_info->vact * 2); /* 2 times */
}
#endif

#if vo_desc("enable dev")
td_void vo_reset_dev_vars(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    vo_init_pub_dev_ctx(dev_ctx);
}

static td_void vo_enable_clk(td_void)
{
    vo_lpw_bus_reset(TD_FALSE);
    vo_drv_set_all_crg_clk(TD_TRUE);
}

static td_s32 vo_check_enable_user_div(ot_vo_dev dev)
{
    td_s32 ret;
    td_u32 dev_div;
    td_u32 pre_div;
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    if ((dev_ctx->vou_attr.intf_sync != OT_VO_OUT_USER) || (dev_ctx->user_config != TD_TRUE)) {
        return TD_SUCCESS;
    }

    dev_div = dev_ctx->vo_user_sync_info.dev_div;
    pre_div = dev_ctx->vo_user_sync_info.pre_div;
    ret = vo_check_dev_user_div(dev, dev_div, pre_div);
    if (ret != TD_SUCCESS) {
        vo_err_trace("vo dev %d user sync dev_div %u or pre_div %u is illegal, pls check the user sync info!\n",
            dev, dev_div, pre_div);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 vo_check_enable(ot_vo_dev dev)
{
    td_s32 ret;
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    if (dev_ctx->config != TD_TRUE) {
        vo_err_trace("vo device %d doesn't configured!\n", dev);
        return OT_ERR_VO_NOT_CFG;
    }

    if (dev_ctx->vo_enable == TD_TRUE) {
        vo_err_trace("vo device %d has enabled!\n", dev);
        return OT_ERR_VO_NOT_DISABLE;
    }

    ret = vo_check_enable_user_div(dev);
    if (ret != TD_SUCCESS) {
        vo_err_trace("vo%d's user sync info div is illegal!\n", dev);
        return ret;
    }

    return TD_SUCCESS;
}

static td_void vo_enable_dev_clk_sel(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->vou_attr.intf_sync == OT_VO_OUT_USER) {
        return;
    }
    vo_drv_set_dev_clk_sel(dev, 0);
}

static td_void vo_enable_dev_clk(ot_vo_dev dev)
{
    vo_drv_set_dev_clk(dev);

    vo_drv_set_dev_clk_en(dev, TD_TRUE);

    vo_enable_dev_clk_sel(dev);
}
static td_void vo_do_enable_dev(ot_vo_dev dev)
{
    vo_enable_dev_clk(dev);
    vo_drv_open(dev);
    vo_set_dev_intf_param(dev);
}

static td_void vo_had_enable(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = TD_NULL;
    dev_ctx = vo_get_dev_ctx(dev);

    vo_drv_enable(dev);
    dev_ctx->vo_enable = TD_TRUE;
}

static td_s32 vo_do_enable(ot_vo_dev dev)
{
    td_s32 ret;
    vo_enable_clk();

    ret = vo_check_enable(dev);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vo_do_enable_dev(dev);
    vo_had_enable(dev);
    return TD_SUCCESS;
}

td_s32 vo_enable(ot_vo_dev dev)
{
    td_s32 ret;
    vo_check_dev_id_return(dev);
    ret = vo_do_enable(dev);
    return ret;
}
#endif

#if vo_desc("disable dev")

static td_s32 vo_check_disable(ot_vo_dev dev)
{
    if (vo_is_dev_layer_enabled(dev)) {
        return OT_ERR_VO_NOT_DISABLE;
    }

    if (vo_is_dev_gfx_layer_enabled(dev)) {
        return OT_ERR_VO_NOT_DISABLE;
    }

    return TD_SUCCESS;
}

static td_void vo_disable_clk(td_void)
{
    ot_vo_dev dev;
    vo_dev_info *dev_ctx = TD_NULL;

    for (dev = 0; dev < OT_VO_MAX_PHYS_DEV_NUM; dev++) {
        dev_ctx = vo_get_dev_ctx(dev);
        if (dev_ctx == TD_NULL) {
            continue;
        }
        if ((dev_ctx->vo_enable) || (vou_drv_get_dev_enable(dev) == TD_TRUE)) {
            break;
        }
    }

    if (dev == OT_VO_MAX_PHYS_DEV_NUM) {
        vo_drv_set_all_crg_clk(TD_FALSE);
    }
}

static td_void vo_disable_phy_dev(ot_vo_dev dev)
{
    vo_drv_close(dev);
    vo_drv_set_dev_clk_en(dev, TD_FALSE);
}

static td_s32 vo_do_disable(ot_vo_dev dev)
{
    td_s32 ret;
    vo_dev_info *dev_ctx = TD_NULL;
    vo_drv_set_all_crg_clk(TD_TRUE);

    ret = vo_check_disable(dev);
    if (ret != TD_SUCCESS) {
        return ret;
    }
    if (vo_is_phy_dev(dev)) {
        vo_disable_phy_dev(dev);
    }

    vo_drv_disable(dev);

    dev_ctx = vo_get_dev_ctx(dev);
    dev_ctx->config = TD_FALSE;
    dev_ctx->vo_enable = TD_FALSE;
    dev_ctx->user_config = TD_FALSE;
    vo_reset_dev_vars(dev);

    vo_disable_clk();

    return TD_SUCCESS;
}

td_s32 vo_disable(ot_vo_dev dev)
{
    td_s32 ret;

    vo_check_dev_id_return(dev);

    ret = vo_do_disable(dev);
    return ret;
}
#endif

#if vo_desc("pub attr")

static td_s32 vo_check_set_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr)
{
    td_s32 ret;
    vo_dev_info *dev_ctx = TD_NULL;

    dev_ctx = vo_get_dev_ctx(dev);
    if (dev_ctx->vo_enable == TD_TRUE) {
        vo_err_trace("vo%d doesn't disabled!\n", dev);
        return OT_ERR_VO_NOT_DISABLE;
    }

    ret = vou_drv_check_dev_pub_attr(dev, pub_attr);
    if (ret != TD_SUCCESS) {
        vo_err_trace("vo%d's pub attr is illegal!\n", dev);
        return ret;
    }
    return TD_SUCCESS;
}

static td_void vo_set_disp_max_size(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr)
{
    td_u32 max_width = VO_DISP_1080_WIDTH;
    td_u32 max_height = VO_DISP_1080_HEIGHT;
    ot_vo_intf_sync intf_sync = pub_attr->intf_sync;
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    if (vo_is_typical_intf_sync(intf_sync)) {
        vo_get_intf_sync_size(intf_sync, &max_width, &max_height);
    } else {
        vo_get_intf_sync_size_user(&pub_attr->sync_info, &max_width, &max_height);
    }

    dev_ctx->max_width = max_width;
    dev_ctx->max_height = max_height;
    vou_drv_set_disp_max_size(dev, max_width, max_height);
}

static td_void vo_set_full_frame_rate(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr)
{
    td_u32 frame_rate = VO_DISP_FREQ_VGA;
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    vo_get_intf_sync_frame_rate(pub_attr->intf_sync, &frame_rate);
    dev_ctx->full_frame_rate = frame_rate;
}

static td_s32 vo_do_set_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr)
{
    td_s32 ret;
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    ret = vo_check_set_pub_attr(dev, pub_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vo_set_full_frame_rate(dev, pub_attr);
    vo_set_disp_max_size(dev, pub_attr);
    vo_drv_set_pub_attr(dev, pub_attr);

    (td_void)memcpy_s(&dev_ctx->vou_attr, sizeof(ot_vo_pub_attr), pub_attr, sizeof(ot_vo_pub_attr));
    dev_ctx->config = TD_TRUE;

    return TD_SUCCESS;
}

td_s32 vo_set_pub_attr(ot_vo_dev dev, const ot_vo_pub_attr *pub_attr)
{
    td_s32 ret;

    vo_check_null_ptr_return(pub_attr);
    vo_check_dev_id_return(dev);

    ret = vo_do_set_pub_attr(dev, pub_attr);

    return ret;
}
#endif

#if vo_desc("user sync info")
static td_s32 vo_check_dev_user_dev_div(ot_vo_dev dev, td_u32 dev_div)
{
    if ((dev_div < VO_MIN_DIV_MODE) || (dev_div > VO_MAX_DIV_MODE)) {
        vo_err_trace("vo(%d) dev div %u is illegal, it must be in [%u,%u].\n", dev, dev_div, VO_MIN_DIV_MODE,
            VO_MAX_DIV_MODE);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return vo_drv_check_intf_user_dev_div(dev, dev_div);
}

static td_s32 vo_check_intf_user_pre_div(ot_vo_dev dev, td_u32 pre_div)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);
    ot_vo_intf_type intf_type = dev_ctx->vou_attr.intf_type;

    if ((vo_drv_is_hdmi_intf(intf_type) != TD_TRUE) &&
        (vo_drv_is_hdmi1_intf(intf_type) != TD_TRUE)) {
        if (pre_div != VO_INTF_NO_HDMI_PRE_DIV_MODE) {
            vo_err_trace("vo(%d) pre div %u is illegal, it must be %u when intf type is %u.\n", dev, pre_div,
                VO_INTF_NO_HDMI_PRE_DIV_MODE, intf_type);
            return OT_ERR_VO_ILLEGAL_PARAM;
        }
    } else {
        if (vo_drv_is_support_hdmi_ssc_vdp_div() != TD_TRUE) {
            if (pre_div != VO_INTF_HDMI_PRE_DIV_MODE) {
                vo_err_trace("vo(%d) pre div %u is illegal, it must be %u when intf type is %u.\n", dev, pre_div,
                    VO_INTF_HDMI_PRE_DIV_MODE, intf_type);
                return OT_ERR_VO_ILLEGAL_PARAM;
            }
        }
    }

    return TD_SUCCESS;
}

td_s32 vo_check_dev_user_pre_div(ot_vo_dev dev, td_u32 pre_div)
{
    if ((pre_div < VO_MIN_PRE_DIV_MODE) || (pre_div > VO_MAX_PRE_DIV_MODE)) {
        vo_err_trace("vo(%d) pre div %u is illegal, it must be in [%u,%u].\n", dev, pre_div,
            VO_MIN_PRE_DIV_MODE, VO_MAX_PRE_DIV_MODE);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return vo_check_intf_user_pre_div(dev, pre_div);
}

td_s32 vo_check_dev_user_div(ot_vo_dev dev, td_u32 dev_div, td_u32 pre_div)
{
    td_s32 ret;

    ret = vo_check_dev_user_dev_div(dev, dev_div);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return vo_check_dev_user_pre_div(dev, pre_div);
}

static td_s32 vo_check_dev_user_sync_info(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    td_s32 ret;

    ret = vo_check_dev_user_div(dev, sync_info->dev_div, sync_info->pre_div);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (vo_check_bool_value(sync_info->clk_reverse_en) != TD_SUCCESS) {
        vo_err_trace("vo(%d) clk_reverse_en %u is illegal!\n", dev, sync_info->clk_reverse_en);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return vo_drv_check_dev_clkvalue(dev, sync_info);
}
static td_s32 vo_check_do_user_sync_info(ot_vo_dev dev)
{
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    if (dev_ctx->config != TD_TRUE) {
        vo_err_trace("vo device %d doesn't configured!\n", dev);
        return OT_ERR_VO_NOT_CFG;
    }

    if (dev_ctx->vou_attr.intf_sync != OT_VO_OUT_USER) {
        vo_err_trace("vo device %d only support this in user sync.\n", dev);
        return OT_ERR_VO_NOT_SUPPORT;
    }

    return TD_SUCCESS;
}
static td_s32 vo_check_set_user_sync_info(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    td_s32 ret;
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);

    ret = vo_check_do_user_sync_info(dev);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (dev_ctx->vo_enable == TD_TRUE) {
        vo_err_trace("vo device %d has enabled!\n", dev);
        return OT_ERR_VO_NOT_DISABLE;
    }

    ret = vo_check_dev_user_sync_info(dev, sync_info);
    if (ret != TD_SUCCESS) {
        vo_err_trace("VO%d's user sync info is illegal ret=0x%x!\n", dev, ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 vo_do_set_user_sync_info(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    td_s32 ret;
    vo_dev_info *dev_ctx = vo_get_dev_ctx(dev);
    vo_drv_set_all_crg_clk(TD_TRUE);

    ret = vo_check_set_user_sync_info(dev, sync_info);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    vo_drv_set_dev_user_intf_sync_attr(dev, sync_info);

    vo_drv_set_dev_div(dev, sync_info->dev_div);

    vo_drv_set_hdmi_div(dev, sync_info->pre_div);

    vo_drv_set_clk_reverse(dev, sync_info->clk_reverse_en);

    (td_void)memcpy_s(&dev_ctx->vo_user_sync_info, sizeof(ot_vo_user_sync_info), sync_info,
        sizeof(ot_vo_user_sync_info));
    dev_ctx->user_config = TD_TRUE;
    return TD_SUCCESS;
}

td_s32 vo_set_user_sync_info(ot_vo_dev dev, const ot_vo_user_sync_info *sync_info)
{
    td_s32 ret;

    vo_check_dev_id_return(dev);
    vo_check_null_ptr_return(sync_info);

    ret = vo_do_set_user_sync_info(dev, sync_info);

    return ret;
}
#endif
