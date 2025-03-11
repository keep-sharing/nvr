// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hal_vo_gfx.h"
#include "hal_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

td_void hal_gfx_set_ck_gt_en(hal_disp_layer layer, td_u32 ck_gt_en)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_voctrl voctrl;
    volatile td_ulong addr_reg;

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->voctrl.u32);
    voctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    switch (layer) {
        case HAL_DISP_LAYER_GFX0:
            voctrl.bits.g0_ck_gt_en = ck_gt_en;
            break;
        case HAL_DISP_LAYER_GFX1:
            voctrl.bits.g1_ck_gt_en = ck_gt_en;
            break;
        default:
            break;
    }
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, voctrl.u32);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
