// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hal_vo_comm.h"
#include "drv_vo.h"
#include "hal_vo.h"
#include "ot_board.h"
#include <common.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("hal pub")

td_void hal_vo_init(td_void)
{
    volatile reg_vdp_regs *reg = vo_hal_get_reg();
    if (reg == TD_NULL) {
        reg = (volatile reg_vdp_regs *)io_address(VOU_REGS_ADDR);
        vo_hal_set_reg(reg);
    }

    if (reg == TD_NULL) {
        OT_PRINT("ioremap failed\n");
    }
}

/* no ddr reg */
td_void hal_write_reg(td_u32 *address, td_u32 value)
{
    *(volatile td_u32 *)address = value;
}

td_u32 hal_read_reg(const td_u32 *address)
{
    return *(volatile td_u32 *)(address);
}

#endif
#if vo_desc("hal check")

td_bool vo_hal_is_phy_dev(ot_vo_dev dev)
{
    if ((dev >= DEV_PHY_START) && (dev <= DEV_PHY_END)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

td_bool vo_hal_is_video_layer(hal_disp_layer layer)
{
    if ((layer >= LAYER_VID_START) && (layer <= LAYER_VID_END)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

#ifdef VO_HAL_CHECK_DEBUG
td_bool vo_hal_is_gfx_layer(hal_disp_layer layer)
{
    if ((layer >= LAYER_GFX_START) && (layer <= LAYER_GFX_END)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}
#endif

#endif

#ifdef VO_HAL_CHECK_DEBUG

td_bool vo_hal_is_video_gfx_layer(hal_disp_layer layer)
{
    if (vo_hal_is_video_layer(layer) || vo_hal_is_gfx_layer(layer)) {
        return TD_TRUE;
    }

    return TD_FALSE;
}

#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
