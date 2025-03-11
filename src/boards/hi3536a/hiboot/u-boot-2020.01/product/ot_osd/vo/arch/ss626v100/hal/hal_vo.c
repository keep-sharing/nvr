// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hal_vo.h"
#include "hal_vo_reg.h"
#include "hal_vo_def.h"
#include "inner_vo.h"
#include "hal_vo_comm.h"
#include "mkp_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

static volatile reg_vdp_regs *g_vo_reg = TD_NULL;

#if vo_desc("pub")

volatile reg_vdp_regs *vo_hal_get_reg(td_void)
{
    return g_vo_reg;
}

td_void vo_hal_set_reg(volatile reg_vdp_regs *reg)
{
    g_vo_reg = reg;
}

#endif

#if vo_desc("get abs addr")

td_ulong vou_get_abs_addr(hal_disp_layer layer, td_ulong reg)
{
    volatile td_ulong reg_abs_addr;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0:
        case HAL_DISP_LAYER_VHD1:
        case HAL_DISP_LAYER_VHD2:
        case HAL_DISP_LAYER_VSD0:
            reg_abs_addr = reg + (layer - HAL_DISP_LAYER_VHD0) * VHD_REGS_LEN;
            break;

        case HAL_DISP_LAYER_GFX0:
        case HAL_DISP_LAYER_GFX1:
        case HAL_DISP_LAYER_GFX2:
        case HAL_DISP_LAYER_GFX3:
        case HAL_DISP_LAYER_GFX4:
            reg_abs_addr = reg + (layer - HAL_DISP_LAYER_GFX0) * GFX_REGS_LEN;
            break;

        default:
            vo_err_trace("invalid layer %d!\n", layer);
            reg_abs_addr = reg;
            break;
    }

    return reg_abs_addr;
}

td_ulong vou_get_chn_abs_addr(ot_vo_dev dev, td_ulong reg)
{
    volatile td_ulong reg_abs_addr;

    switch (dev) {
        case VO_DEV_DHD0:
        case VO_DEV_DHD1:
        case VO_DEV_DSD0:
            reg_abs_addr = reg + (dev - VO_DEV_DHD0) * DHD_REGS_LEN;
            break;

        default:
            vo_err_trace("invalid dev %d!\n", dev);
            reg_abs_addr = reg;
            break;
    }

    return reg_abs_addr;
}

td_ulong vou_get_vid_abs_addr(hal_disp_layer layer, td_ulong reg)
{
    volatile td_ulong reg_abs_addr;

    switch (layer) {
        case HAL_DISP_LAYER_VHD0:
        case HAL_DISP_LAYER_VHD1:
        case HAL_DISP_LAYER_VHD2:
        case HAL_DISP_LAYER_VSD0:
            reg_abs_addr = reg + (layer - HAL_DISP_LAYER_VHD0) * VID_REGS_LEN;
            break;

        default:
            vo_err_trace("invalid layer %d!\n", layer);
            reg_abs_addr = reg;
            break;
    }

    return reg_abs_addr;
}

td_ulong vou_get_gfx_abs_addr(hal_disp_layer layer, td_ulong reg)
{
    volatile td_ulong reg_abs_addr;

    switch (layer) {
        case HAL_DISP_LAYER_GFX0:
        case HAL_DISP_LAYER_GFX1:
        case HAL_DISP_LAYER_GFX2:
        case HAL_DISP_LAYER_GFX3:
        case HAL_DISP_LAYER_GFX4:
            reg_abs_addr = reg + (layer - HAL_DISP_LAYER_GFX0) * GRF_REGS_LEN;
            break;

        default:
            vo_err_trace("invalid layer %d!\n", layer);
            reg_abs_addr = reg;
            break;
    }

    return reg_abs_addr;
}
#endif

td_bool hal_gfx_set_reg_up(hal_disp_layer layer)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_g0_upd g0_upd;
    volatile td_ulong addr_reg;

    switch (layer) {
        case HAL_DISP_LAYER_GFX0:
        case HAL_DISP_LAYER_GFX1:
        case HAL_DISP_LAYER_GFX4:
            addr_reg = vou_get_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->g0_upd.u32));
            g0_upd.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
            /* graphic layer register update */
            g0_upd.bits.regup = 0x1;
            hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, g0_upd.u32);
            break;

        default:
            vo_err_trace("err layer id %d\n", layer);
            return TD_FALSE;
    }

    return TD_TRUE;
}

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
