// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_VIDEO_H
#define HAL_VO_VIDEO_H

#include "hal_vo_video_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

td_void vo_hal_layer_set_cvfir_cfg(hal_disp_layer layer, td_u32 vratio, const vdp_v1_cvfir_cfg *cfg,
    const vo_zme_comm_pq_cfg *pq_cfg);
td_void vo_hal_layer_set_cvfir_coef(hal_disp_layer layer, const cvfir_coef *coef);
td_void hal_cbm_set_cbm_attr(hal_disp_layer layer, ot_vo_dev dev);
td_void hal_cbm_set_cbm_bkg(hal_cbmmix mixer, const hal_disp_bkcolor *bkg);
td_void hal_cbm_set_cbm_mixer_prio(ot_vo_layer layer, td_u8 prio, td_u8 mixer_id);
td_u32 hal_layer_get_layer_max_area_num(hal_disp_layer layer);
td_void hal_cbm_release_cbm_attr(hal_disp_layer layer);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_VIDEO_H */
