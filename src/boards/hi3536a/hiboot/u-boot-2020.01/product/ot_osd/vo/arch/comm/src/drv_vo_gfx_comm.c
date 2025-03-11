// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_vo_gfx_comm.h"
#include "drv_vo_gfx.h"
#include "mkp_vo_bmp.h"
#include "hal_vo.h"
#include "hal_vo_gfx_comm.h"
#include "hal_vo_gfx.h"
#include "hal_vo_layer_comm.h"
#include "vo.h"
#include <common.h>

#if vo_desc("UBOOT_VO")
#if vo_desc("vo gfx csc")
td_s32 vo_drv_set_gfx_layer_csc(ot_vo_layer gfx_layer, const ot_vo_csc *csc, const csc_coef_param *csc_param)
{
    td_s32 ret;
    hal_disp_layer hal_layer;
    csc_coef coef = {0};

    ret = vo_drv_get_hal_gfx_layer(gfx_layer, &hal_layer);
    if (ret != TD_SUCCESS) {
        vo_err_trace("gfx layer %d is illegal!\n", gfx_layer);
        return OT_ERR_VO_INVALID_LAYER_ID;
    }

    vou_drv_calc_csc_matrix(csc, csc->csc_matrix, &coef);
    vo_drv_csc_trans_to_register(&coef);

    /* coef need clip in range */
    vo_drv_clip_layer_csc_coef(&coef);
    hal_layer_set_csc_coef(hal_layer, &coef, csc_param);
    return TD_SUCCESS;
}
#endif

#if vo_desc("vo gfx open")
static td_s32 vo_drv_gfx_check_image_stride_and_xywh(ot_vo_gfx_attr *gfx_attr_out, const osd_logo_t *scroll_image_logo)
{
    if ((gfx_attr_out->stride != scroll_image_logo->stride) ||
        (gfx_attr_out->display_rect.width != scroll_image_logo->width) ||
        (gfx_attr_out->display_rect.height != scroll_image_logo->height)) {
        vo_err_trace("logo's stride=%d, w=%d, h=%d is not equal to gfx attr stride=%d, w=%d, h=%d\n",
            scroll_image_logo->stride, scroll_image_logo->width, scroll_image_logo->height, gfx_attr_out->stride,
            gfx_attr_out->display_rect.width, gfx_attr_out->display_rect.height);
        return OT_ERR_VO_ILLEGAL_PARAM;
    }
    return TD_SUCCESS;
}

td_s32 vo_drv_gfx_convert_gfx_attr(const ot_vo_gfx_attr *gfx_attr_in, ot_vo_gfx_attr *gfx_attr_out)
{
    td_phys_addr_t addr;
    osd_logo_t scroll_image_logo = {0};

    if (gfx_attr_in->type == OT_VO_GFX_TYPE_ARGB1555) {
        *gfx_attr_out = *gfx_attr_in;
    } else if (gfx_attr_in->type == OT_VO_GFX_TYPE_BMP1555) {
        addr = gfx_attr_in->address;
        if (load_bmp(addr, &scroll_image_logo) != TD_SUCCESS) {
            return TD_FAILURE;
        }
        gfx_attr_out->address = scroll_image_logo.rgb_buffer;
        gfx_attr_out->stride = gfx_attr_in->stride;
        gfx_attr_out->display_rect = gfx_attr_in->display_rect;
        (td_void)vo_drv_gfx_check_image_stride_and_xywh(gfx_attr_out, &scroll_image_logo);
    } else {
        vo_err_trace("gfx unknown input type %d\n", gfx_attr_in->type);
        return TD_FAILURE;
    }

    vo_dcache_range(gfx_attr_out->address, gfx_attr_out->stride * gfx_attr_out->display_rect.height);

    return TD_SUCCESS;
}

td_void vo_drv_gfx_open(ot_vo_layer gfx_id, const ot_vo_gfx_attr *gfx_attr)
{
    td_s32 ret;
    hal_disp_layer vo_layer;
    td_phys_addr_t addr;
    td_u32 strd;
    ot_rect gx_rect;

    ret = vo_drv_get_hal_gfx_layer(gfx_id, &vo_layer);
    if (ret != TD_SUCCESS) {
        vo_err_trace("get gfx id failed.\n");
        return;
    }
    addr = gfx_attr->address;
    strd = gfx_attr->stride;
    gx_rect = gfx_attr->display_rect;

    hal_gfx_set_pixel_alpha_range(vo_layer, 0xff);
    hal_layer_set_layer_global_alpha(vo_layer, 0xff);
    hal_gfx_set_gfx_pre_mult(vo_layer, TD_FALSE);

    hal_gfx_set_gfx_addr(vo_layer, addr);
    hal_gfx_set_gfx_stride(vo_layer, strd >> 4); /* stride div 16(2 ^ 4) */
    hal_gfx_set_layer_in_rect(vo_layer, &gx_rect);

    hal_gfx_set_layer_disp_rect(vo_layer, &gx_rect);
    hal_gfx_set_layer_video_rect(vo_layer, &gx_rect);

    hal_gfx_set_src_resolution(vo_layer, &gx_rect);

    hal_gfx_set_layer_data_fmt(vo_layer, HAL_INPUTFMT_ARGB_1555);

    hal_gfx_enable_layer(vo_layer, TD_TRUE);
    hal_gfx_set_ck_gt_en(vo_layer, TD_TRUE);
    hal_gfx_set_reg_up(vo_layer);
}

td_void vo_drv_gfx_close(ot_vo_layer layer_id)
{
    td_s32 ret;
    hal_disp_layer hal_gfx_layer;

    ret = vo_drv_get_hal_gfx_layer(layer_id, &hal_gfx_layer);
    if (ret != TD_SUCCESS) {
        return;
    }

    hal_gfx_set_reg_up(hal_gfx_layer);
    hal_gfx_enable_layer(hal_gfx_layer, TD_FALSE);
    hal_gfx_set_ck_gt_en(hal_gfx_layer, TD_FALSE);
    hal_gfx_set_reg_up(hal_gfx_layer);
}
#endif
#endif /* #if vo_desc("UBOOT_VO") */
