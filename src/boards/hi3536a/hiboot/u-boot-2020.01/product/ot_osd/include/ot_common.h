// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __OT_COMMON_H__
#define __OT_COMMON_H__

#include "ot_type.h"
#ifndef __KERNEL__
#include "securec.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#define ot_unused(x)             ((td_void)(x))

#define OT_INVALID_CHN         (-1)
#define OT_INVALID_WAY         (-1)
#define OT_INVALID_LAYER       (-1)
#define OT_INVALID_DEV         (-1)
#define OT_INVALID_HANDLE      (-1)
#define OT_INVALID_VALUE       (-1)
#define OT_INVALID_TYPE        (-1)

typedef td_s32 ot_vo_dev;
typedef td_s32 ot_vo_layer;
typedef td_s32 ot_vo_chn;
typedef td_s32 ot_vo_wbc;
typedef td_s32 ot_gfx_layer;

typedef enum {
    OT_ID_VO      = 15,

    OT_ID_BUTT,
} ot_mod_id;

typedef struct {
    ot_mod_id    mod_id;
    td_s32      dev_id;
    td_s32      chn_id;
} ot_mpp_chn;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* __OT_COMMON_H__ */
