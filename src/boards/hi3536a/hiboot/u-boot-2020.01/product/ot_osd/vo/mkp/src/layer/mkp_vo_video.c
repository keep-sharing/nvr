// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "mkp_vo_video.h"
#include "ot_common_vo.h"
#include "mkp_vo.h"
#include "vo.h"
#include "sys_ext.h"

#include "mkp_vo_comm.h"
#include "mkp_vo_dev.h"
#include "common.h"
#include "securec.h"

#if vo_desc("public api")
static vo_layer_info g_vo_layer[OT_VO_MAX_LAYER_NUM];

vo_layer_info *vo_get_layer_ctx(ot_vo_layer vo_layer)
{
    return &(g_vo_layer[vo_layer]);
}

static td_void vo_init_def_layer_ctx(ot_vo_layer layer, vo_layer_info *ctx)
{
    vou_drv_init_default_csc(layer);
    vou_drv_init_default_csc_param(layer);
    vou_drv_get_default_csc(layer, &ctx->layer_csc);
}

td_s32 vo_init_layer_ctx(ot_vo_layer layer)
{
    td_u32 layer_ctx_len;
    vo_layer_info *ctx = TD_NULL;

    ctx = vo_get_layer_ctx(layer);

    layer_ctx_len = sizeof(vo_layer_info);

    (td_void)memset_s(ctx, layer_ctx_len, 0, layer_ctx_len);
    vo_init_def_layer_ctx(layer, ctx);
    return TD_SUCCESS;
}

td_void vo_init_layer_resource(td_void)
{
    vo_drv_layer_info_init();
}

ot_vo_dev vo_get_video_layer_bind_dev(ot_vo_layer layer)
{
    return vou_drv_get_video_layer_bind_dev(layer);
}

td_s32 vo_check_layer_binded_dev(ot_vo_layer layer, ot_vo_dev dev)
{
    if (dev == OT_INVALID_DEV) {
        return OT_ERR_VO_NOT_BINDED;
    }

    if (vo_def_check_dev_id(dev) != TD_SUCCESS) {
        vo_warn_trace("vo layer %d binded dev %d is invalid!\n", layer, dev);
        return OT_ERR_VO_INVALID_DEV_ID;
    }

    return TD_SUCCESS;
}

td_bool vo_is_layer_enabled(ot_vo_layer layer)
{
    vo_layer_info *ctx = TD_NULL;

    ctx = vo_get_layer_ctx(layer);
    if (ctx == TD_NULL) {
        return TD_FALSE;
    }

    return ctx->video_enable;
}

td_bool vo_is_dev_layer_enabled(ot_vo_dev dev)
{
    td_u32 layer_num;
    td_u32 loop;
    ot_vo_layer layer[OT_VO_MAX_LAYER_IN_DEV] = {0};

    layer_num = vou_drv_get_dev_layer_num(dev);
    if (layer_num == 0) {
        return TD_FALSE;
    }
    vou_drv_get_dev_layer(dev, &(layer[0]), OT_VO_MAX_LAYER_IN_DEV);
    for (loop = 0; loop < layer_num; loop++) {
        if (vo_is_layer_enabled(layer[loop]) == TD_TRUE) {
            vo_err_trace("vo %d's video layer %d doesn't disable!\n", dev, layer[loop]);
            return TD_TRUE;
        }
    }

    return TD_FALSE;
}

#endif

#if vo_desc("enable video layer")
td_bool vo_is_layer_configured(ot_vo_layer layer)
{
    vo_layer_info *ctx = TD_NULL;

    ctx = vo_get_layer_ctx(layer);
    return ctx->video_config;
}

static td_s32 vo_check_enable_video_layer(ot_vo_layer layer)
{
    ot_vo_dev dev;

    if (vo_is_layer_configured(layer) != TD_TRUE) {
        vo_err_trace("layer%d's video layer hasn't configured!\n", layer);
        return OT_ERR_VO_NOT_CFG;
    }

    dev = vo_get_video_layer_bind_dev(layer);
    if (vo_check_layer_binded_dev(layer, dev) != TD_SUCCESS) {
        vo_err_trace("layer(%d) is not bind any dev!\n", layer);
        return OT_ERR_VO_NOT_BINDED;
    }

    if (vo_is_dev_enabled(dev) != TD_TRUE) {
        vo_err_trace("vo device %d doesn't enable!\n", dev);
        return OT_ERR_VO_NOT_ENABLE;
    }

    return TD_SUCCESS;
}

static td_s32 vo_do_enable_video_layer(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    td_s32 ret;
    vo_layer_info *layer_ctx = TD_NULL;

    layer_ctx = vo_get_layer_ctx(layer);
    if (layer_ctx->video_enable == TD_TRUE) {
        vo_warn_trace("video layer%d has been enabled!\n", layer);
        return TD_SUCCESS;
    }

    ret = vo_check_enable_video_layer(layer);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    layer_ctx->video_enable = TD_TRUE;

    vo_drv_layer_open(layer, video_attr);

    return TD_SUCCESS;
}

td_s32 vo_enable_video_layer(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    td_s32 ret;

    vo_check_layer_id_return(layer);
    vo_check_gfx_layer_return(layer);

    ret = vo_do_enable_video_layer(layer, video_attr);
    return ret;
}

#endif

#if vo_desc("disable video layer")
td_void vo_reset_layer_vars(ot_vo_layer layer)
{
    vo_layer_info *layer_ctx = TD_NULL;

    layer_ctx = vo_get_layer_ctx(layer);

    vou_drv_init_default_csc(layer);
    vou_drv_init_default_csc_param(layer);

    vou_drv_get_default_csc(layer, &layer_ctx->layer_csc);
}

static td_s32 vo_check_disable_video_layer(ot_vo_layer layer)
{
    ot_vo_dev dev;

    dev = vo_get_video_layer_bind_dev(layer);
    if (vo_check_layer_binded_dev(layer, dev) != TD_SUCCESS) {
        vo_err_trace("layer(%d) is not bind any dev!\n", layer);
        return OT_ERR_VO_NOT_BINDED;
    }

    return TD_SUCCESS;
}

static td_s32 vo_do_disable_video_layer(ot_vo_layer layer)
{
    td_s32 ret;
    td_s32 dev;
    vo_layer_info *layer_ctx = TD_NULL;

    ret = vo_check_disable_video_layer(layer);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    layer_ctx = vo_get_layer_ctx(layer);
    if (layer_ctx->video_enable == TD_FALSE) {
        return TD_SUCCESS;
    }

    dev = vou_drv_get_video_layer_bind_dev(layer);
    if (vo_is_phy_dev(dev)) {
        vou_drv_layer_enable(layer, TD_FALSE);
        vou_drv_set_layer_reg_up(layer);
        /* sleep: msleep(40); */
    }

    layer_ctx->video_enable = TD_FALSE;
    layer_ctx->video_config = TD_FALSE;
    vo_drv_disable_layer(layer);

    vo_reset_layer_vars(layer);

    return TD_SUCCESS;
}

td_s32 vo_disable_video_layer(ot_vo_layer layer_id)
{
    vo_check_layer_id_return(layer_id);
    vo_check_gfx_layer_return(layer_id);

    return vo_do_disable_video_layer(layer_id);
}

#endif

#if vo_desc("layer attr")

static td_s32 vo_check_layer_attr_display_rect(ot_vo_dev dev, ot_vo_layer layer,
    const ot_vo_video_layer_attr *video_attr)
{
    ot_size dev_size;
    const ot_rect *rect = &video_attr->display_rect;

    vo_get_dev_max_size(dev, &dev_size);
    if ((rect->x < 0) || (rect->y < 0) || (rect->width < VO_RESO_MIN_WDT) || (rect->height < VO_RESO_MIN_HGT) ||
        ((rect->width + rect->x) > dev_size.width) || ((rect->height + rect->y) > dev_size.height)) {
        vo_err_trace("layer(%d) display rect(%d, %d, %u, %u) is illegal, should between (%u, %u) and (%u, %u)\n",
            layer, rect->x, rect->y, rect->width, rect->height,
            VO_RESO_MIN_WDT, VO_RESO_MIN_HGT, dev_size.width, dev_size.height);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if (vo_check_rect_align(rect) != TD_SUCCESS) {
        vo_err_trace("layer(%d), display rect (%d, %d, %u, %u) doesn't aligned by 2 pixels!\n",
            layer, rect->x, rect->y, rect->width, rect->height);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return vo_drv_check_layer_attr_display_rect(layer, rect, video_attr->pixel_format);
}

static td_s32 vo_check_layer_attr_img_size(ot_vo_dev dev, ot_vo_layer layer,
    const ot_vo_video_layer_attr *video_attr)
{
    const ot_rect *img_size = TD_NULL;

    img_size = &(video_attr->display_rect);
    if (vo_is_dev_interlaced_intf_sync(dev)) {
        if ((img_size->height % VO_RECT_INTERLACED_ALIGN != 0) &&
            (video_attr->pixel_format == OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420)) {
            vo_err_trace("layer(%d) img height (%u) do not align by 4 pixels "
                "when pixel format is yuv420 and dev's sync is interlaced.\n",
                layer, img_size->height);
            return OT_ERR_VO_ILLEGAL_PARAM;
        }
    }

    return TD_SUCCESS;
}

static td_s32 vo_check_layer_attr_stride(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    if (((video_attr->stride % VO_RECT_ALIGN) != 0)) {
        vo_err_trace("layer%d's stride %d is illegal!\n", layer, video_attr->stride);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    if ((video_attr->address % VO_ADDR_ALIGN) != 0) {
        vo_err_trace("layer%d's addr 0x%lx is illegal, it should be %d bytes align!\n", layer, video_attr->address,
            VO_ADDR_ALIGN);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 vo_check_layer_attr(ot_vo_dev dev, ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    td_s32 ret;
    ret = vo_check_layer_attr_display_rect(dev, layer, video_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    ret = vo_check_layer_attr_img_size(dev, layer, video_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    return vo_check_layer_attr_stride(layer, video_attr);
}

static td_s32 vo_check_set_video_layer_attr(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    ot_vo_dev dev = vo_get_video_layer_bind_dev(layer);
    if (vo_check_layer_binded_dev(layer, dev) != TD_SUCCESS) {
        vo_err_trace("vo layer %d can't set layer attr when the layer has not been binded!\n", layer);
        return OT_ERR_VO_NOT_BINDED;
    }

    if (vo_is_dev_enabled(dev) != TD_TRUE) {
        vo_err_trace("vo layer %d bind dev %d doesn't enable!\n", layer, dev);
        return OT_ERR_VO_NOT_ENABLE;
    }

    return vo_check_layer_attr(dev, layer, video_attr);
}

static td_bool vo_is_layer_only_pos_changed(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr_set)
{
    ot_vo_video_layer_attr video_attr_old;
    vo_layer_info *layer_ctx = TD_NULL;

    layer_ctx = vo_get_layer_ctx(layer);

    (td_void)memcpy_s(&video_attr_old, sizeof(ot_vo_video_layer_attr), &layer_ctx->vou_video_attr,
        sizeof(ot_vo_video_layer_attr));

    video_attr_old.display_rect.x = video_attr_set->display_rect.x;
    video_attr_old.display_rect.y = video_attr_set->display_rect.y;

    if (memcmp(&video_attr_old, video_attr_set, sizeof(ot_vo_video_layer_attr)) == 0) {
        return TD_TRUE;
    }
    return TD_FALSE;
}

static td_s32 vo_set_layer_attr_with_layer_enable(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    vo_layer_info *layer_ctx = TD_NULL;

    layer_ctx = vo_get_layer_ctx(layer);
    if ((vo_is_layer_only_pos_changed(layer, video_attr))) {
            (td_void)memcpy_s(&layer_ctx->vou_video_attr, sizeof(ot_vo_video_layer_attr), video_attr,
                sizeof(ot_vo_video_layer_attr));
            vo_drv_set_layer_attr(layer, video_attr);
            return TD_SUCCESS;
    }

    vo_err_trace("layer(%d): change layer attr, but layer doesn't disabled!\n", layer);
    return OT_ERR_VO_NOT_DISABLE;
}

static td_void vo_set_layer_attr_with_layer_disable(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    vo_layer_info *layer_ctx = TD_NULL;

    layer_ctx = vo_get_layer_ctx(layer);

    (td_void)memcpy_s(&layer_ctx->vou_video_attr, sizeof(ot_vo_video_layer_attr), video_attr,
        sizeof(ot_vo_video_layer_attr));
    layer_ctx->video_config = TD_TRUE;
}

static td_void vo_set_layer_attr_in_drv(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    ot_vo_dev dev;
    vo_layer_info *layer_ctx = TD_NULL;

    dev = vou_drv_get_video_layer_bind_dev(layer);
    layer_ctx = vo_get_layer_ctx(layer);

    vo_drv_set_layer_attr(layer, video_attr);

    if (vo_is_phy_dev(dev)) {
        vou_drv_get_default_csc(layer, &layer_ctx->layer_csc);
        vou_drv_layer_csc_config(layer, &layer_ctx->layer_csc);
        vou_drv_layer_csc_enable(layer, TD_TRUE);
    }
}

static td_s32 vo_do_set_video_layer_attr(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    td_s32 ret;

    ret = vo_check_set_video_layer_attr(layer, video_attr);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    if (vo_is_layer_enabled(layer)) {
        ret = vo_set_layer_attr_with_layer_enable(layer, video_attr);
        return ret;
    }

    vo_set_layer_attr_with_layer_disable(layer, video_attr);
    vo_set_layer_attr_in_drv(layer, video_attr);

    return TD_SUCCESS;
}

td_s32 vo_set_video_layer_attr(ot_vo_layer layer, const ot_vo_video_layer_attr *video_attr)
{
    td_s32 ret;

    vo_check_null_ptr_return(video_attr);
    vo_check_layer_id_return(layer);
    vo_check_gfx_layer_return(layer);

    ret = vo_do_set_video_layer_attr(layer, video_attr);
    return ret;
}

#endif

