// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef MKP_VO_GFX_H
#define MKP_VO_GFX_H

#include "ot_type.h"
#include "ot_common.h"
#include "ot_common_vo.h"
#include "drv_vo_gfx.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

td_void vo_init_gfx(td_void);
td_bool vo_is_dev_gfx_layer_enabled(ot_vo_dev dev);
td_s32 vo_set_gfx_attr(ot_vo_layer layer, const ot_vo_gfx_attr *attr);
td_s32 vo_enable_gfx_layer(ot_vo_layer layer, const ot_vo_gfx_attr *gfx_attr);
td_s32 vo_disable_gfx_layer(ot_vo_layer layer_id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef MKP_VO_GFX_H */
