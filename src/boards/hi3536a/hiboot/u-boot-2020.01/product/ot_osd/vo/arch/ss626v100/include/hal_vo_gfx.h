// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_GFX_H
#define HAL_VO_GFX_H

#include "inner_vo.h"
#include "hal_vo_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */
#if vo_desc("UBOOT_VO")
td_void hal_gfx_set_ck_gt_en(hal_disp_layer layer, td_u32 ck_gt_en);
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_GFX_H */
