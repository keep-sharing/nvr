// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hal_vo_gfx_comm.h"
#include "hal_vo.h"
#include "ot_math.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("gfx uboot only")
td_void hal_gfx_set_layer_disp_rect(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_g0_dfpos g0_dfpos;
    volatile reg_g0_dlpos g0_dlpos;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->g0_dfpos.u32));
    g0_dfpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    g0_dfpos.bits.disp_xfpos = rect->x;
    g0_dfpos.bits.disp_yfpos = rect->y;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, g0_dfpos.u32);

    addr_reg = vou_get_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->g0_dlpos.u32));
    g0_dlpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    g0_dlpos.bits.disp_xlpos = rect->x + rect->width - 1;
    g0_dlpos.bits.disp_ylpos = rect->y + rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, g0_dlpos.u32);
}

td_void hal_gfx_set_layer_video_rect(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_g0_vfpos g0_vfpos;
    volatile reg_g0_vlpos g0_vlpos;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->g0_vfpos.u32));
    g0_vfpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    g0_vfpos.bits.video_xfpos = rect->x;
    g0_vfpos.bits.video_yfpos = rect->y;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, g0_vfpos.u32);

    addr_reg = vou_get_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->g0_vlpos.u32));
    g0_vlpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    g0_vlpos.bits.video_xlpos = rect->x + rect->width - 1;
    g0_vlpos.bits.video_ylpos = rect->y + rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, g0_vlpos.u32);
}

td_void hal_gfx_enable_layer(hal_disp_layer layer, td_u32 enable)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_g0_ctrl g0_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);
    addr_reg = vou_get_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->g0_ctrl.u32));
    g0_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    g0_ctrl.bits.surface_en = enable;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, g0_ctrl.u32);
}

td_void hal_gfx_set_layer_data_fmt(hal_disp_layer layer, hal_disp_pixel_format data_fmt)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_gfx_src_info gfx_src_info;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);
    addr_reg = vou_get_gfx_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->gfx_src_info.u32));
    gfx_src_info.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    gfx_src_info.bits.ifmt = data_fmt;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, gfx_src_info.u32);
}

td_void hal_gfx_set_layer_in_rect(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_gfx_ireso gfx_ireso;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);
    addr_reg = vou_get_gfx_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->gfx_ireso.u32));
    gfx_ireso.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    gfx_ireso.bits.ireso_w = rect->width - 1;
    gfx_ireso.bits.ireso_h = rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, gfx_ireso.u32);
}

td_void hal_gfx_set_gfx_pre_mult(hal_disp_layer layer, td_u32 enable)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_gfx_out_ctrl gfx_out_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);
    addr_reg = vou_get_gfx_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->gfx_out_ctrl.u32));
    gfx_out_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    gfx_out_ctrl.bits.premulti_en = enable;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, gfx_out_ctrl.u32);
}

td_void hal_gfx_set_gfx_addr(hal_disp_layer layer, td_phys_addr_t l_addr)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile td_ulong ul_gfx_addr_h;
    volatile td_ulong ul_gfx_addr_l;

    vo_hal_check_gfx_layer_return(layer);
    /* write low address to register */
    ul_gfx_addr_l = vou_get_gfx_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->gfx_addr_l));
    hal_write_reg((td_u32 *)(td_uintptr_t)ul_gfx_addr_l, get_low_addr(l_addr));
    /* write high address to register */
    ul_gfx_addr_h = vou_get_gfx_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->gfx_addr_h));
    hal_write_reg((td_u32 *)(td_uintptr_t)ul_gfx_addr_h, get_high_addr(l_addr));
}

td_void hal_gfx_set_gfx_stride(hal_disp_layer layer, td_u16 pitch)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_gfx_stride gfx_stride;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);
    addr_reg = vou_get_gfx_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->gfx_stride.u32));
    gfx_stride.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    gfx_stride.bits.surface_stride = pitch;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, gfx_stride.u32);
}

td_void hal_gfx_set_src_resolution(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_gfx_src_reso gfx_src_reso;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);
    addr_reg = vou_get_gfx_abs_addr(layer, (td_ulong)(td_uintptr_t)&(vo_reg->gfx_src_reso.u32));
    gfx_src_reso.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    gfx_src_reso.bits.src_w = rect->width - 1;
    gfx_src_reso.bits.src_h = rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, gfx_src_reso.u32);
}
#endif

#if vo_desc("gfx")
td_void hal_gfx_set_pixel_alpha_range(hal_disp_layer layer, td_u32 alpha_range)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_gfx_out_ctrl gfx_out_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_gfx_layer_return(layer);
    addr_reg = vou_get_gfx_abs_addr(layer, (td_uintptr_t)&(vo_reg->gfx_out_ctrl.u32));
    gfx_out_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    gfx_out_ctrl.bits.palpha_range = alpha_range;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, gfx_out_ctrl.u32);
}
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
