// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_vo_video.h"
#include "ot_math.h"
#include "vo.h"
#include "hal_vo.h"
#include "drv_vo.h"
#include "securec.h"

#include <common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#define VO_LAYER_DISP_RECT_W_MAX_OF_VHDX 4096
#define VO_LAYER_DISP_RECT_H_MAX_OF_VHDX 4096
#define VO_LAYER_DISP_RECT_W_MAX_OF_VSD0 720
#define VO_LAYER_DISP_RECT_H_MAX_OF_VSD0 720

#define VO_LAYER_DISP_RECT_WIDTH_BUFFER_MAX  3840

#define VO_LAYER_CSC_MIN_COEF_VAL   (-2048)   /* 12bit min */
#define VO_LAYER_CSC_MAX_COEF_VAL   2047      /* 12bit max */

#if vo_desc("layer zme & cvfir")

td_void vo_drv_get_zme_typ_pq_cfg(vo_zme_comm_pq_cfg *pq_cfg)
{
    /*
     * 32, gain typical
     * 16, coring typical
     */
    pq_cfg->vluma_offset = 0;
    pq_cfg->vchroma_offset = 0;
    pq_cfg->vbluma_offset = 0;
    pq_cfg->vbchroma_offset = 0;
    pq_cfg->vl_flatdect_mode = 1;
    pq_cfg->vl_coringadj_en = 1;
    pq_cfg->vl_gain = 32; /* 32, vl gain typical */
    pq_cfg->vl_coring = 16; /* 16, vl coring typical */
    pq_cfg->vc_flatdect_mode = 1;
    pq_cfg->vc_coringadj_en = 1;
    pq_cfg->vc_gain = 32; /* 32, vc gain typical */
    pq_cfg->vc_coring = 16; /* 16, vc coring typical */
    pq_cfg->lhfir_offset = 0;
    pq_cfg->chfir_offset = 0;
    pq_cfg->hl_flatdect_mode = 1;
    pq_cfg->hl_coringadj_en = 1;
    pq_cfg->hl_gain = 32; /* 32, hl gain typical */
    pq_cfg->hl_coring = 16; /* 16, hl coring typical */
    pq_cfg->hc_flatdect_mode = 1;
    pq_cfg->hc_coringadj_en = 1;
    pq_cfg->hc_gain = 32; /* 32, hc gain typical */
    pq_cfg->hc_coring = 16; /* 16, hc coring typical */
}

static td_void vo_drv_get_cvfir_pq_cfg(vo_zme_comm_pq_cfg *comm_pq_cfg)
{
    vo_drv_get_zme_typ_pq_cfg(comm_pq_cfg);
}

static td_void vo_drv_set_layer_cvfir_mode(td_u32 layer, const vdp_v1_cvfir_cfg *cfg)
{
    td_u32 vratio;
    vo_zme_ds_info ds_info = {0};
    vo_zme_comm_pq_cfg comm_pq_cfg = {0};

    ds_info.zme_vprec = ZME_VPREC;
    ds_info.zme_hprec = ZME_HPREC;

    vratio = (td_u32)((cfg->in_height * ds_info.zme_vprec) / cfg->in_height);
    vo_drv_get_cvfir_pq_cfg(&comm_pq_cfg);
    vo_hal_layer_set_cvfir_cfg(layer, vratio, cfg, &comm_pq_cfg);
}

#if vo_desc("layer")
td_void vo_drv_layer_info_init(td_void)
{
    td_s32 i;
    td_u32 ctx_len;
    vo_drv_layer *drv_layer_ctx = TD_NULL;

    drv_layer_ctx = vo_drv_get_layer_ctx(0);
    ctx_len = sizeof(vo_drv_layer) * OT_VO_MAX_LAYER_NUM;
    (td_void)memset_s(drv_layer_ctx, ctx_len, 0, ctx_len);

    for (i = 0; i < OT_VO_MAX_LAYER_NUM; i++) {
        drv_layer_ctx = vo_drv_get_layer_ctx(i);

        if (i == VO_SW_LAYER_VHD0) {
            drv_layer_ctx->bind_dev = VO_DEV_DHD0;
        } else if (i == VO_SW_LAYER_VHD1) {
            drv_layer_ctx->bind_dev = VO_DEV_DHD1;
        } else if (i == VO_SW_LAYER_VHD2) {
            drv_layer_ctx->bind_dev = VO_DEV_DHD0;
        } else if (i == VO_SW_LAYER_VSD0) {
            drv_layer_ctx->bind_dev = VO_DEV_DSD0;
        }
    }
}

#endif

#endif

#if vo_desc("layer")


static td_bool vo_drv_is_video_close_cvfir(ot_vo_layer layer)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    if (drv_layer_ctx->vo_video_attr.display_rect.width > VO_LAYER_DISP_RECT_WIDTH_BUFFER_MAX) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

static td_void vo_drv_video_set_cvfir_enable_cfg(ot_vo_dev dev, td_u32 layer,
    const vo_video_layer_ip_cfg *vid_cfg)
{
    td_bool cvfir_en = TD_TRUE;
    vdp_v1_cvfir_cfg cvfir_cfg = { 0 };
    if (vo_drv_is_video_close_cvfir(layer) == TD_TRUE) {
        cvfir_en = TD_FALSE;
    }

    cvfir_cfg.hfir_order = 0;
    cvfir_cfg.lhfir_en = 0;
    cvfir_cfg.chfir_en = 0;
    cvfir_cfg.lhmid_en = 0;
    cvfir_cfg.chmid_en = 0;
    cvfir_cfg.lhfir_mode = 0;
    cvfir_cfg.chfir_mode = 0;
    cvfir_cfg.hl_shootctrl_en = 0;
    cvfir_cfg.hl_shootctrl_mode = 0;
    cvfir_cfg.hc_shootctrl_en = 0;
    cvfir_cfg.hc_shootctrl_mode = 0;
    cvfir_cfg.lvfir_en = 0;
    cvfir_cfg.lvmid_en = 0;
    cvfir_cfg.lvfir_mode = 0;
    cvfir_cfg.vl_shootctrl_en = 0;
    cvfir_cfg.vl_shootctrl_mode = 0;
    cvfir_cfg.vc_shootctrl_en = 0;
    cvfir_cfg.vc_shootctrl_mode = 0;

    /* CVFIR */
    cvfir_cfg.ck_gt_en = 1;
    cvfir_cfg.cvfir_en = cvfir_en;
    cvfir_cfg.cvmid_en = 0;

    cvfir_cfg.cvfir_mode = 0;
    cvfir_cfg.out_pro = VDP_RMODE_PROGRESSIVE;

    if (vo_drv_is_progressive(dev) == TD_FALSE) {
        cvfir_cfg.out_pro = VDP_RMODE_INTERLACE;
    }
    cvfir_cfg.out_fmt = VDP_PROC_FMT_SP_422;
    cvfir_cfg.in_width = vid_cfg->vid_iw;
    cvfir_cfg.in_height = vid_cfg->vid_ih;
    cvfir_cfg.out_width = vid_cfg->vid_ow;
    cvfir_cfg.out_height = vid_cfg->vid_oh;
    vo_drv_set_layer_cvfir_mode(layer, &cvfir_cfg);
}

td_void vo_drv_video_set_zme_enable(td_u32 layer, const vo_video_layer_ip_cfg *vid_cfg)
{
    ot_vo_dev dev;

    if ((vid_cfg->vid_iw == 0) || (vid_cfg->vid_ih == 0) ||
        (vid_cfg->vid_ow == 0) || (vid_cfg->vid_oh == 0)) {
        vo_err_trace("layer(%d) ZME error: input w=%d,h=%d,output w=%d,h=%d!\n",
            layer, vid_cfg->vid_iw, vid_cfg->vid_ih, vid_cfg->vid_ow, vid_cfg->vid_oh);
        return;
    }

    dev = vou_drv_get_video_layer_bind_dev(layer);
    vo_drv_video_set_cvfir_enable_cfg(dev, layer, vid_cfg);
}

#endif

#if vo_desc("layer")

static td_void vo_drv_get_layer_attr_max_rect(ot_vo_layer layer, td_u32 *width, td_u32 *height)
{
    ot_vo_dev dev;

    *width = VO_LAYER_DISP_RECT_W_MAX_OF_VHDX;
    *height = VO_LAYER_DISP_RECT_H_MAX_OF_VHDX;

    dev = vou_drv_get_video_layer_bind_dev(layer);
    if ((layer == VO_SW_LAYER_VSD0) && (dev == VO_DEV_DSD0)) {
        *width = VO_LAYER_DISP_RECT_W_MAX_OF_VSD0;
        *height = VO_LAYER_DISP_RECT_H_MAX_OF_VSD0;
    }
}

td_s32 vo_drv_check_layer_attr_display_rect(ot_vo_layer layer, const ot_rect *rect, ot_pixel_format pixel_format)
{
    td_u32 max_width;
    td_u32 max_height;

    if ((layer >= VO_SW_LAYER_VHD0) && (layer <= VO_SW_LAYER_VSD0)) {
        vo_drv_get_layer_attr_max_rect(layer, &max_width, &max_height);
        if ((rect->width > max_width) || (rect->height > max_height)) {
            vo_err_trace("VO layer(%d) disp rect width(%d) height(%d) can't be larger than %dx%d!\n",
                layer, rect->width, rect->height,
                max_width, max_height);
            return OT_ERR_VO_ILLEGAL_PARAM;
        }

        if ((rect->width > VO_LAYER_DISP_RECT_WIDTH_BUFFER_MAX) &&
            (pixel_format != OT_PIXEL_FORMAT_YVU_SEMIPLANAR_422) &&
            (pixel_format != OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422) &&
            (pixel_format != OT_PIXEL_FORMAT_YUV_400)) {
            vo_err_trace("layer(%d)'s pixel format %u doesn't support when rect width %u larger than %u!\n",
                layer, pixel_format, rect->width, VO_LAYER_DISP_RECT_WIDTH_BUFFER_MAX);
            return OT_ERR_VO_ILLEGAL_PARAM;
        }
    } else if (layer < VOU_SW_LAYER_BUTT) {
    } else {
        return OT_ERR_VO_ILLEGAL_PARAM;
    }

    return TD_SUCCESS;
}

#endif

#if vo_desc("layer")

hal_disp_layer vou_drv_convert_layer(ot_vo_layer layer)
{
    hal_disp_layer disp_layer = HAL_DISP_LAYER_BUTT;

    switch (layer) {
        case VO_HAL_LAYER_VHD0:
            disp_layer = HAL_DISP_LAYER_VHD0;
            break;

        case VO_HAL_LAYER_VHD1:
            disp_layer = HAL_DISP_LAYER_VHD1;
            break;

        case VO_HAL_LAYER_VHD2:
            disp_layer = HAL_DISP_LAYER_VHD2;
            break;

        case VO_HAL_LAYER_VSD0:
            disp_layer = HAL_DISP_LAYER_VSD0;
            break;

        default:
            break;
    }

    return disp_layer;
}

#endif

td_void vo_drv_get_layer_csc_coef_range(td_s32 *min_coef, td_s32 *max_coef)
{
    *min_coef = VO_LAYER_CSC_MIN_COEF_VAL;
    *max_coef = VO_LAYER_CSC_MAX_COEF_VAL;
}

td_void vou_drv_layer_csc_enable(ot_vo_layer layer, td_bool csc_en)
{
    hal_layer_csc_set_enable_ck_gt_en(layer, csc_en, csc_en);
}

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
