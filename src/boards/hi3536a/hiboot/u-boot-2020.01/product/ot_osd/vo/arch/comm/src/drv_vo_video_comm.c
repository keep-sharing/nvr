// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_vo_video_comm.h"
#include "drv_vo.h"
#include "drv_vo_video.h"
#include "hal_vo_layer_comm.h"
#include "hal_vo_video_comm.h"
#include "vo.h"
#include <common.h>
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("video")
td_void vou_drv_init_default_csc(ot_vo_layer layer)
{
    ot_vo_csc *csc = TD_NULL;
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);

    csc = &drv_layer_ctx->csc;
    csc->csc_matrix = OT_VO_CSC_MATRIX_BT601FULL_TO_BT601FULL;
    csc->luma = VO_CSC_DEF_VAL;
    csc->contrast = VO_CSC_DEF_VAL;
    csc->hue = VO_CSC_DEF_VAL;
    csc->saturation = VO_CSC_DEF_VAL;
    csc->ex_csc_en = TD_FALSE;
}

td_void vou_drv_init_default_csc_param(ot_vo_layer layer)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    csc_coef_param *csc_param = &drv_layer_ctx->csc_param;

    csc_param->csc_scale2p = VO_LAYER_CSC_SCALE2P_DEF_VAL;
    csc_param->csc_clip_min = VO_LAYER_CSC_CLIP_MIN;
    csc_param->csc_clip_max = VO_LAYER_CSC_CLIP_MAX;
}

td_void vo_drv_set_layer_attr(ot_vo_layer layer, const ot_vo_video_layer_attr *vo_layer_attr)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);

    (td_void)memcpy_s(&drv_layer_ctx->vo_video_attr, sizeof(ot_vo_video_layer_attr), vo_layer_attr,
        sizeof(ot_vo_video_layer_attr));
    drv_layer_ctx->video_config = TD_TRUE;
}

td_void vou_drv_get_default_csc(ot_vo_layer layer, ot_vo_csc *csc)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    (td_void)memcpy_s(csc, sizeof(ot_vo_csc), &drv_layer_ctx->csc, sizeof(ot_vo_csc));
}

td_void vo_drv_layer_set_zme_info(ot_vo_layer layer, td_u32 width, td_u32 height, hal_disp_zme_outfmt zme_out_fmt)
{
    hal_layer_set_layer_zme_info(layer, width, height, zme_out_fmt);
}

td_void vo_drv_disable_layer(ot_vo_layer layer)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);

    drv_layer_ctx->video_enable = TD_FALSE;
}

td_s32 vou_drv_get_video_layer_bind_dev(ot_vo_layer layer)
{
    vo_drv_layer *layer_ctx = TD_NULL;
    layer_ctx = vo_drv_get_layer_ctx(layer);
    return layer_ctx->bind_dev;
}

#endif

#if vo_desc("video open close")
static td_void video_layer_set_zme_cfg(unsigned int layer, const ot_rect *disp_rect)
{
    vo_video_layer_ip_cfg vid_cfg = {0};

    vid_cfg.csc_en = 0;
    vid_cfg.hfir_en = 1;
    vid_cfg.vid_iw = disp_rect->width;
    vid_cfg.vid_ih = disp_rect->height;

    vid_cfg.vid_ow = disp_rect->width;
    vid_cfg.vid_oh = disp_rect->height;
    vid_cfg.zme_en = TD_FALSE;
    vo_drv_video_set_zme_enable(layer, &vid_cfg);
}

td_u32 vou_align_up(td_u32 val, td_u32 align)
{
    return ((val) + (align) - 1) & (~((align) - 1));
}

td_void vo_drv_layer_open(ot_vo_layer layer_id, const ot_vo_video_layer_attr *video_attr)
{
    hal_disp_layer layer;
    ot_rect disp_rect = {0};
    td_ulong addr;
    td_u32 strd;

    disp_rect = video_attr->display_rect;
    addr = video_attr->address;
    strd = video_attr->stride;

    layer = vou_drv_convert_layer(layer_id);

    hal_layer_set_layer_data_fmt(layer, HAL_INPUTFMT_YCBCR_SEMIPLANAR_420);
    hal_video_set_layer_disp_rect(layer, &disp_rect);
    hal_video_set_layer_video_rect(layer, &disp_rect);
    hal_layer_set_layer_in_rect(layer, &disp_rect);
    hal_layer_set_layer_global_alpha(layer, VO_ALPHA_OPACITY);
    hal_layer_set_src_resolution(layer, &disp_rect);

    video_layer_set_zme_cfg(layer, &disp_rect);

    vo_drv_layer_set_zme_info(layer, disp_rect.width, disp_rect.height, HAL_DISP_ZME_OUTFMT422);

    hal_video_set_multi_area_lum_addr(layer, addr, strd);
    hal_video_set_multi_area_chm_addr(layer, addr + strd * vou_align_up(disp_rect.height, VO_STRIDE_ALIGN), strd);
    hal_layer_enable_layer(layer, TD_TRUE);
    hal_video_set_layer_ck_gt_en(layer, TD_TRUE);
    hal_layer_set_reg_up(layer);
}

#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
