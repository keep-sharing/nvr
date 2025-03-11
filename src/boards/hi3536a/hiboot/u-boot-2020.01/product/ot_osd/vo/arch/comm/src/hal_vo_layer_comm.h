// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_LAYER_COMM_H
#define HAL_VO_LAYER_COMM_H

#include "hal_vo_def.h"
#include "drv_vo_coef_comm.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("layer")
td_void hal_video_set_layer_alpha(hal_disp_layer layer, td_u32 alpha);
td_void hal_layer_set_csc_coef(hal_disp_layer layer, const csc_coef *coef,
    const csc_coef_param *csc_param);
td_void hal_layer_set_layer_global_alpha(hal_disp_layer layer, td_u8 alpha0);
td_void hal_layer_csc_set_enable_ck_gt_en(hal_disp_layer layer, td_bool csc_en, td_bool ck_gt_en);
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_LAYER_COMM_H */
