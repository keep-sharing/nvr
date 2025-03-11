// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_LAYER_COMM_H
#define DRV_VO_LAYER_COMM_H

#include "inner_vo.h"
#include "drv_vo_coef_comm.h"
#include "hal_vo_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#if vo_desc("layer")
#define VO_LAYER_CSC_SCALE2P_DEF_VAL 0xa
#define VO_LAYER_CSC_CLIP_MIN 0x0
#define VO_LAYER_CSC_CLIP_MAX 0xfff
#endif

#if vo_desc("layer csc")
td_s32 vo_drv_get_csc_matrix(ot_vo_csc_matrix csc_matrix, const csc_coef **csc_tmp);
td_void vo_drv_calculate_yuv2rgb(const hal_csc_value *csc_value, const csc_coef *csc_tmp, csc_coef *coef);
td_void vo_drv_calculate_rgb2yuv(const hal_csc_value *csc_value, const csc_coef *csc_tmp, csc_coef *coef);
td_void vo_drv_clip_layer_csc_coef(csc_coef *coef);
td_void vou_drv_calc_csc_matrix(const ot_vo_csc *csc, ot_vo_csc_matrix csc_matrix, csc_coef *coef);
td_void vou_drv_layer_csc_config(ot_vo_layer layer, const ot_vo_csc *csc);
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* end of #ifdef __cplusplus */
#endif

#endif /* end of DRV_VO_LAYER_COMM_H */
