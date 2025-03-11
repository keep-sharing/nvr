// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef HAL_VO_COMM_H
#define HAL_VO_COMM_H

#include "hal_vo_def.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#define io_address(x) (x)

#ifdef OT_DEBUG
#define VO_HAL_CHECK_DEBUG /* vo hal check debug, debug only */
#endif

#define REG_BYTES 4   /* 1reg 4bytes */

#if vo_desc("hal pub")
td_void hal_vo_init(td_void);
td_void hal_write_reg(td_u32 *address, td_u32 value);
td_u32 hal_read_reg(const td_u32 *address);
#endif

#if vo_desc("hal check")
td_bool vo_hal_is_phy_dev(ot_vo_dev dev);
td_bool vo_hal_is_video_layer(hal_disp_layer layer);
td_bool vo_hal_is_gfx_layer(hal_disp_layer layer);
td_bool vo_hal_is_video_gfx_layer(hal_disp_layer layer);
#endif

#ifdef VO_HAL_CHECK_DEBUG

#define vo_hal_check_phy_dev_return(dev)                              \
    do {                                                              \
        if (vo_hal_is_phy_dev(dev) != TD_TRUE) {                      \
            OT_PRINT("err dev in %s: L%d\n", __FUNCTION__, __LINE__); \
            return;                                                   \
        }                                                             \
    } while (0)

#define vo_hal_check_video_layer_return(layer)                          \
    do {                                                                \
        if (vo_hal_is_video_layer(layer) != TD_TRUE) {                  \
            OT_PRINT("err layer in %s: L%d\n", __FUNCTION__, __LINE__); \
            return;                                                     \
        }                                                               \
    } while (0)

#define vo_hal_check_gfx_layer_return(layer)                            \
    do {                                                                \
        if (vo_hal_is_gfx_layer(layer) != TD_TRUE) {                    \
            OT_PRINT("err layer in %s: L%d\n", __FUNCTION__, __LINE__); \
            return;                                                     \
        }                                                               \
    } while (0)

#define vo_hal_check_video_gfx_layer_return(layer)                      \
    do {                                                                \
        if (vo_hal_is_video_gfx_layer(layer) != TD_TRUE) {              \
            OT_PRINT("err layer in %s: L%d\n", __FUNCTION__, __LINE__); \
            return;                                                     \
        }                                                               \
    } while (0)

#else

#define vo_hal_check_phy_dev_return(dev)
#define vo_hal_check_video_layer_return(layer)
#define vo_hal_check_gfx_layer_return(layer)
#define vo_hal_check_video_gfx_layer_return(layer)

#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of HAL_VO_COMM_H */
