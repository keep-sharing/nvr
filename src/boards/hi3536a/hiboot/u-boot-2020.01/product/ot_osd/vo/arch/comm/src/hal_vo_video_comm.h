// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_VIDEO_COMM_H
#define HAL_VO_VIDEO_COMM_H

#include "hal_vo_def.h"
#include "drv_vo_coef_comm.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

typedef struct {
    ot_vo_layer layer;
    td_u8 layer_id;
} vo_hal_cbm_mixer;

#if vo_desc("layer")
td_void hal_video_set_layer_ck_gt_en(hal_disp_layer layer, td_bool ck_gt_en);
td_void hal_video_set_layer_up_mode(hal_disp_layer layer, td_u32 up_mode);
td_void hal_video_set_layer_disp_rect(hal_disp_layer layer, const ot_rect *rect);
td_void hal_video_set_layer_video_rect(hal_disp_layer layer, const ot_rect *rect);
td_void hal_video_set_multi_area_lum_addr(hal_disp_layer layer, td_phys_addr_t addr, td_u32 stride);
td_void hal_video_set_multi_area_chm_addr(hal_disp_layer layer, td_phys_addr_t addr, td_u32 stride);
#endif

#if vo_desc("cvfir")
td_void vo_hal_layer_set_v1_cvfir_cfg(td_u32 vratio, const vdp_v1_cvfir_cfg *cfg,
    const vo_zme_comm_pq_cfg *pq_cfg);
td_void vo_hal_layer_set_v1_cvfir_coef(const cvfir_coef *coef);
td_void vo_hal_layer_set_chroma_copy(td_u32 layer, td_bool chroma_copy_flag);
#endif

#if vo_desc("layer")
/* video layer HFIR relative hal functions */
td_void hal_video_hfir_set_ck_gt_en(hal_disp_layer layer, td_u32 ck_gt_en);
td_void hal_video_hfir_set_mid_en(hal_disp_layer layer, td_u32 mid_en);
td_void hal_video_hfir_set_hfir_mode(hal_disp_layer layer, td_u32 hfir_mode);
td_void hal_video_hfir_set_coef(hal_disp_layer layer, const hfir_coef *coef);

td_void hal_layer_enable_layer(hal_disp_layer layer, td_u32 enable);
td_void hal_layer_set_layer_data_fmt(hal_disp_layer layer, hal_disp_pixel_format data_fmt);
td_void hal_layer_set_src_resolution(hal_disp_layer layer, const ot_rect *rect);
td_void hal_layer_set_layer_in_rect(hal_disp_layer layer, const ot_rect *rect);
td_void hal_layer_set_layer_zme_info(hal_disp_layer layer, td_u32 width, td_u32 height,
    hal_disp_zme_outfmt zme_out_fmt);
td_void hal_layer_set_reg_up(hal_disp_layer layer);
td_bool hal_cbm_get_cbm_mixer_layer_id(const vo_hal_cbm_mixer *cbm_mixer, td_u32 cbm_len,
    ot_vo_layer layer, td_u8 *layer_id);
#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_VIDEO_COMM_H */
