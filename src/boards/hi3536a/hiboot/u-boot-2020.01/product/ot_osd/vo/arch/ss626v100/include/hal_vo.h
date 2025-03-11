// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_H
#define HAL_VO_H

#include "hal_vo_reg.h"
#include "hal_vo_def.h"
#include "inner_vo.h"
#include "hal_vo_comm.h"
#include "hal_vo_dev.h"
#include "hal_vo_video.h"
#include "hal_vo_layer_comm.h"
#include "hal_vo_gfx_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#if vo_desc("hal pub")
volatile reg_vdp_regs *vo_hal_get_reg(td_void);
td_void vo_hal_set_reg(volatile reg_vdp_regs *reg);
#endif

#if vo_desc("get abs addr")
td_ulong vou_get_abs_addr(hal_disp_layer layer, td_ulong reg);
td_ulong vou_get_chn_abs_addr(ot_vo_dev dev, td_ulong reg);
td_ulong vou_get_vid_abs_addr(hal_disp_layer layer, td_ulong reg);
td_ulong vou_get_gfx_abs_addr(hal_disp_layer layer, td_ulong reg);
#endif
td_bool hal_gfx_set_reg_up(hal_disp_layer layer);
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_H */
