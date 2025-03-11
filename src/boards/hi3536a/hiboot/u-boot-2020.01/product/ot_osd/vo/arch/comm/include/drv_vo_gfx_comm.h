// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_GFX_COMM_H
#define DRV_VO_GFX_COMM_H

#include "ot_type.h"
#include "ot_common_vo.h"
#include "mkp_vo.h"
#include "inner_vo.h"
#include "hal_vo_def.h"
#include "drv_vo_coef_comm.h"

#if vo_desc("UBOOT_VO")
#define GFX_CSC_SCALE    0xa
#define GFX_CSC_CLIP_MIN 0x0
#define GFX_CSC_CLIP_MAX 0x3ff
td_s32 vo_drv_set_gfx_layer_csc(ot_vo_layer gfx_layer, const ot_vo_csc *csc, const csc_coef_param *csc_param);

typedef struct {
    td_bool opened;

    ot_vo_csc gfx_csc;
    csc_coef_param csc_param;
} vo_gfxlayer_context;

td_s32 graphic_drv_set_csc_coef(hal_disp_layer gfx_layer, ot_vo_csc *gfx_csc, csc_coef_param *csc_param);
td_void gfx_drv_init_default_csc(ot_vo_layer layer, ot_vo_csc *csc);
td_void gfx_drv_init_default_csc_param(ot_vo_layer layer, csc_coef_param *csc_param);
#endif /* #if vo_desc("UBOOT_VO") */

#endif
