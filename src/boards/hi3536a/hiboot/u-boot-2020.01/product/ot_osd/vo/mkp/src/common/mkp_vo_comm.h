// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef MKP_VO_COMM_H
#define MKP_VO_COMM_H

#include "inner_vo.h"

#define VO_RECT_INTERLACED_ALIGN  4

#if vo_desc("check attr pub")
td_s32 vo_check_bool_value(td_bool bool_val);
td_s32 vo_check_rect_align(const ot_rect *rect);
td_s32 vo_check_csc_no_matrix(const ot_vo_csc *csc);
#endif

td_s32 vo_def_check_dev_id(ot_vo_dev dev);
td_bool vo_def_is_gfx_layer_id(ot_vo_layer layer);

#if vo_desc("check dev pub")
td_bool vo_is_phy_dev(ot_vo_dev dev);
td_bool vo_is_typical_intf_sync(ot_vo_intf_sync intf_sync);
td_bool vo_is_user_intf_sync(ot_vo_intf_sync intf_sync);
#endif

/* macro definition for check status */
#define vo_check_null_ptr_return(ptr)                              \
    do {                                                           \
        if ((ptr) == TD_NULL) {                                    \
            vo_warn_trace("null ptr!\n");                          \
            return OT_ERR_VO_NULL_PTR;                             \
        }                                                          \
    } while (0)

/* vo device relative */
#define vo_check_dev_id_return(dev)                         \
    do {                                                    \
        if (vo_drv_check_dev_id(dev) != TD_SUCCESS) {       \
            vo_err_trace("dev %d invalid!\n", dev);         \
            return OT_ERR_VO_INVALID_DEV_ID;                \
        }                                                   \
    } while (0)

/* vo video relative */
#define vo_check_layer_id_return(layer)                     \
    do {                                                    \
        if (vo_drv_check_layer_id(layer) != TD_SUCCESS) {   \
            vo_err_trace("layer %d invalid!\n", layer);     \
            return OT_ERR_VO_INVALID_LAYER_ID;              \
        }                                                   \
    } while (0)

#define vo_check_gfx_id_return(gfx_layer)                       \
    do {                                                        \
        if (vo_check_gfx_id(gfx_layer) != TD_SUCCESS) {         \
            vo_err_trace("gfx layer %d invalid!\n", gfx_layer); \
            return OT_ERR_VO_INVALID_LAYER_ID;                  \
        }                                                       \
    } while (0)

#define vo_check_gfx_layer_return(layer)                                     \
    do {                                                                     \
        if (vo_def_is_gfx_layer_id(layer)) {                                 \
            vo_err_trace("vo layer %d is gfx layer, not support!\n", layer); \
            return OT_ERR_VO_NOT_SUPPORT;                                    \
        }                                                                    \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef MKP_VO_COMM_H */
