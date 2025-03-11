// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hal_vo_video_comm.h"
#include "drv_vo.h"
#include "hal_vo.h"
#include "ot_math.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */
#if vo_desc("UBOOT_VO")

#if vo_desc("video")

td_void hal_video_set_layer_ck_gt_en(hal_disp_layer layer, td_bool ck_gt_en)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_voctrl voctrl;

    /* v3 not support to return */
    if (layer > HAL_DISP_LAYER_VHD2) {
        return;
    }

    voctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)&(vo_reg->voctrl.u32));
    if (layer == HAL_DISP_LAYER_VHD0) {
        voctrl.bits.v0_ck_gt_en = ck_gt_en;
    } else if (layer == HAL_DISP_LAYER_VHD1) {
        voctrl.bits.v1_ck_gt_en = ck_gt_en;
    } else {
        voctrl.bits.v2_ck_gt_en = ck_gt_en;
    }
    hal_write_reg((td_u32 *)(td_uintptr_t)&(vo_reg->voctrl.u32), voctrl.u32);
}

td_void hal_video_set_layer_up_mode(hal_disp_layer layer, td_u32 up_mode)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_ctrl v0_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_ctrl.u32));
    v0_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_ctrl.bits.rgup_mode = up_mode;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_ctrl.u32);
}

td_void hal_video_set_layer_disp_rect(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_dfpos v0_dfpos;
    volatile reg_v0_dlpos v0_dlpos;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_dfpos.u32));
    v0_dfpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_dfpos.bits.disp_xfpos = rect->x;
    v0_dfpos.bits.disp_yfpos = rect->y;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_dfpos.u32);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_dlpos.u32));
    v0_dlpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_dlpos.bits.disp_xlpos = rect->x + rect->width - 1;
    v0_dlpos.bits.disp_ylpos = rect->y + rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_dlpos.u32);
}

td_void hal_video_set_layer_video_rect(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_vfpos v0_vfpos;
    volatile reg_v0_vlpos v0_vlpos;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_vfpos.u32));
    v0_vfpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_vfpos.bits.video_xfpos = rect->x;
    v0_vfpos.bits.video_yfpos = rect->y;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_vfpos.u32);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_vlpos.u32));
    v0_vlpos.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_vlpos.bits.video_xlpos = rect->x + rect->width - 1;
    v0_vlpos.bits.video_ylpos = rect->y + rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_vlpos.u32);
}

td_void hal_video_set_multi_area_lum_addr(hal_disp_layer layer, td_phys_addr_t addr, td_u32 stride)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile td_ulong reg_addr;
    volatile reg_vid_stride vid_stride;

    vo_hal_check_video_layer_return(layer);

    /* low 32 bits to vid_addr_l */
    reg_addr = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_addr_l));
    hal_write_reg((td_u32 *)(td_uintptr_t)reg_addr, get_low_addr(addr));

    /* high 32 bits to vid_addr_h */
    reg_addr = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_addr_h));
    hal_write_reg((td_u32 *)(td_uintptr_t)reg_addr, get_high_addr(addr));

    reg_addr = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_stride.u32));
    vid_stride.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)reg_addr);
    vid_stride.bits.lm_stride = stride;
    hal_write_reg((td_u32 *)(td_uintptr_t)reg_addr, vid_stride.u32);
}

td_void hal_video_set_multi_area_chm_addr(hal_disp_layer layer, td_phys_addr_t addr, td_u32 stride)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile td_ulong reg_addr;
    volatile reg_vid_stride vid_stride;

    vo_hal_check_video_layer_return(layer);

    /* low 32 bits to vid_caddr_l */
    reg_addr = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_caddr_l));
    hal_write_reg((td_u32 *)(td_uintptr_t)reg_addr, get_low_addr(addr));

    /* high 32 bits to vid_caddr_h */
    reg_addr = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_caddr_h));
    hal_write_reg((td_u32 *)(td_uintptr_t)reg_addr, get_high_addr(addr));

    reg_addr = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_stride.u32));
    vid_stride.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)reg_addr);
    vid_stride.bits.chm_stride = stride;
    hal_write_reg((td_u32 *)(td_uintptr_t)reg_addr, vid_stride.u32);
}

td_void hal_layer_set_layer_zme_info(hal_disp_layer layer, td_u32 width, td_u32 height,
    hal_disp_zme_outfmt zme_out_fmt)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_zme_vinfo v0_zme_vinfo;
    volatile reg_v0_zme_hinfo v0_zme_hinfo;
    volatile reg_v1_cvfir_vinfo v1_cvfir_vinfo;
    volatile reg_v2_cvfir_vinfo v2_cvfir_vinfo;
    volatile td_ulong addr_reg;

    if (layer == HAL_DISP_LAYER_VHD0) {
        addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_zme_vinfo.u32);
        v0_zme_vinfo.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
        v0_zme_vinfo.bits.vzme_ck_gt_en = 1;
        v0_zme_vinfo.bits.out_height = height - 1;
        v0_zme_vinfo.bits.out_fmt = zme_out_fmt;
        hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_zme_vinfo.u32);

        addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v0_zme_hinfo.u32);
        v0_zme_hinfo.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
        v0_zme_hinfo.bits.out_width = width - 1;
        hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_zme_hinfo.u32);
    } else if (layer == HAL_DISP_LAYER_VHD1) {
        /* vertical info */
        addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_vinfo.u32);
        v1_cvfir_vinfo.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
        v1_cvfir_vinfo.bits.vzme_ck_gt_en = 1;
        v1_cvfir_vinfo.bits.out_height = height - 1;
        v1_cvfir_vinfo.bits.out_fmt = zme_out_fmt;
        hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_vinfo.u32);
    } else if (layer == HAL_DISP_LAYER_VHD2) {
        /* vertical info */
        addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v2_cvfir_vinfo.u32);
        v2_cvfir_vinfo.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
        v2_cvfir_vinfo.bits.vzme_ck_gt_en = 1;
        v2_cvfir_vinfo.bits.out_height = height - 1;
        v2_cvfir_vinfo.bits.out_fmt = zme_out_fmt;
        hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v2_cvfir_vinfo.u32);
    }
}

td_void hal_layer_enable_layer(hal_disp_layer layer, td_u32 enable)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_ctrl v0_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_ctrl.u32));
    v0_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_ctrl.bits.surface_en = enable;
    v0_ctrl.bits.nosec_flag = 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_ctrl.u32);
}

td_void hal_layer_set_layer_data_fmt(hal_disp_layer layer, hal_disp_pixel_format data_fmt)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_vid_src_info vid_src_info;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_src_info.u32));
    vid_src_info.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    vid_src_info.bits.data_type = data_fmt;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, vid_src_info.u32);
}

td_void hal_layer_set_src_resolution(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_vid_src_reso vid_src_reso;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_src_reso.u32));
    vid_src_reso.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    vid_src_reso.bits.src_w = rect->width - 1;
    vid_src_reso.bits.src_h = rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, vid_src_reso.u32);
}

td_void vo_hal_layer_set_v1_cvfir_cfg(td_u32 vratio, const vdp_v1_cvfir_cfg *cfg,
    const vo_zme_comm_pq_cfg *pq_cfg)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v1_cvfir_vinfo v1_cvfir_vinfo;
    volatile reg_v1_cvfir_vsp v1_cvfir_vsp;
    volatile reg_v1_cvfir_voffset v1_cvfir_voffset;
    volatile reg_v1_cvfir_vboffset v1_cvfir_vboffset;
    volatile td_ulong addr_reg;

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_vinfo.u32);
    v1_cvfir_vinfo.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v1_cvfir_vinfo.bits.out_height = (td_u32)cfg->out_height - 1;
    v1_cvfir_vinfo.bits.out_fmt = cfg->out_fmt;
    v1_cvfir_vinfo.bits.out_pro = cfg->out_pro;
    v1_cvfir_vinfo.bits.vzme_ck_gt_en = cfg->ck_gt_en;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_vinfo.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_vsp.u32);
    v1_cvfir_vsp.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v1_cvfir_vsp.bits.vratio = vratio;
    v1_cvfir_vsp.bits.cvfir_mode = cfg->cvfir_mode;
    v1_cvfir_vsp.bits.cvmid_en = cfg->cvmid_en;
    v1_cvfir_vsp.bits.cvfir_en = cfg->cvfir_en;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_vsp.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_voffset.u32);
    v1_cvfir_voffset.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v1_cvfir_voffset.bits.vchroma_offset = pq_cfg->vchroma_offset;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_voffset.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_vboffset.u32);
    v1_cvfir_vboffset.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v1_cvfir_vboffset.bits.vbchroma_offset = pq_cfg->vbchroma_offset;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_vboffset.u32);
}

td_void vo_hal_layer_set_v1_cvfir_coef(const cvfir_coef *coef)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v1_cvfir_vcoef0 v1_cvfir_vcoef0;
    volatile reg_v1_cvfir_vcoef1 v1_cvfir_vcoef1;
    volatile reg_v1_cvfir_vcoef2 v1_cvfir_vcoef2;
    volatile td_ulong addr_reg;

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_vcoef0.u32);
    v1_cvfir_vcoef0.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v1_cvfir_vcoef0.bits.vccoef00 = coef->vccoef00;
    v1_cvfir_vcoef0.bits.vccoef01 = coef->vccoef01;
    v1_cvfir_vcoef0.bits.vccoef02 = coef->vccoef02;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_vcoef0.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_vcoef1.u32);
    v1_cvfir_vcoef1.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v1_cvfir_vcoef1.bits.vccoef03 = coef->vccoef03;
    v1_cvfir_vcoef1.bits.vccoef10 = coef->vccoef10;
    v1_cvfir_vcoef1.bits.vccoef11 = coef->vccoef11;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_vcoef1.u32);

    addr_reg = (td_ulong)(td_uintptr_t)&(vo_reg->v1_cvfir_vcoef2.u32);
    v1_cvfir_vcoef2.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v1_cvfir_vcoef2.bits.vccoef12 = coef->vccoef12;
    v1_cvfir_vcoef2.bits.vccoef13 = coef->vccoef13;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v1_cvfir_vcoef2.u32);
}

td_void vo_hal_layer_set_chroma_copy(td_u32 layer, td_bool chroma_copy_flag)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_vid_read_ctrl vid_read_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = (td_ulong)(td_uintptr_t)(&(vo_reg->vid_read_ctrl.u32) + layer * FDR_VID_OFFSET);
    vid_read_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    vid_read_ctrl.bits.chm_copy_en = chroma_copy_flag;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, vid_read_ctrl.u32);
}

td_void hal_video_hfir_set_ck_gt_en(hal_disp_layer layer, td_u32 ck_gt_en)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_hfir_ctrl v0_hfir_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_hfir_ctrl.u32));
    v0_hfir_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_hfir_ctrl.bits.ck_gt_en = ck_gt_en;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_hfir_ctrl.u32);
}

td_void hal_video_hfir_set_mid_en(hal_disp_layer layer, td_u32 mid_en)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_hfir_ctrl v0_hfir_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_hfir_ctrl.u32));
    v0_hfir_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_hfir_ctrl.bits.mid_en = mid_en;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_hfir_ctrl.u32);
}

td_void hal_video_hfir_set_hfir_mode(hal_disp_layer layer, td_u32 hfir_mode)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_hfir_ctrl v0_hfir_ctrl;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_hfir_ctrl.u32));
    v0_hfir_ctrl.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_hfir_ctrl.bits.hfir_mode = hfir_mode;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_hfir_ctrl.u32);
}

td_void hal_video_hfir_set_coef(hal_disp_layer layer, const hfir_coef *coef)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_hfircoef01 v0_hfir_coef01;
    volatile reg_v0_hfircoef23 v0_hfir_coef23;
    volatile reg_v0_hfircoef45 v0_hfir_coef45;
    volatile reg_v0_hfircoef67 v0_hfir_coef67;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_hfircoef01.u32));
    v0_hfir_coef01.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_hfir_coef01.bits.coef0 = coef->coef0;
    v0_hfir_coef01.bits.coef1 = coef->coef1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_hfir_coef01.u32);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_hfircoef23.u32));
    v0_hfir_coef23.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_hfir_coef23.bits.coef2 = coef->coef2;
    v0_hfir_coef23.bits.coef3 = coef->coef3;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_hfir_coef23.u32);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_hfircoef45.u32));
    v0_hfir_coef45.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_hfir_coef45.bits.coef4 = coef->coef4;
    v0_hfir_coef45.bits.coef5 = coef->coef5;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_hfir_coef45.u32);

    addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_hfircoef67.u32));
    v0_hfir_coef67.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    v0_hfir_coef67.bits.coef6 = coef->coef6;
    v0_hfir_coef67.bits.coef7 = coef->coef7;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_hfir_coef67.u32);
}

td_void hal_layer_set_layer_in_rect(hal_disp_layer layer, const ot_rect *rect)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_vid_in_reso vid_in_reso;
    volatile td_ulong addr_reg;

    vo_hal_check_video_layer_return(layer);

    addr_reg = vou_get_vid_abs_addr(layer, (td_uintptr_t)&(vo_reg->vid_in_reso.u32));
    vid_in_reso.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
    vid_in_reso.bits.ireso_w = rect->width - 1;
    vid_in_reso.bits.ireso_h = rect->height - 1;
    hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, vid_in_reso.u32);
}

td_void hal_layer_set_reg_up(hal_disp_layer layer)
{
    volatile reg_vdp_regs *vo_reg = vo_hal_get_reg();
    volatile reg_v0_upd v0_upd;
    volatile td_ulong addr_reg;

    if (vo_hal_is_video_layer(layer)) {
        addr_reg = vou_get_abs_addr(layer, (td_uintptr_t)&(vo_reg->v0_upd.u32));
        v0_upd.u32 = hal_read_reg((td_u32 *)(td_uintptr_t)addr_reg);
        /* video layer register update */
        v0_upd.bits.regup = 0x1;
        hal_write_reg((td_u32 *)(td_uintptr_t)addr_reg, v0_upd.u32);
    }
}

td_bool hal_cbm_get_cbm_mixer_layer_id(const vo_hal_cbm_mixer *cbm_mixer, td_u32 cbm_len,
    ot_vo_layer layer, td_u8 *layer_id)
{
    td_u32 index;

    for (index = 0; index < cbm_len; index++) {
        if (layer == cbm_mixer[index].layer) {
            *layer_id = cbm_mixer[index].layer_id;
            return TD_TRUE;
        }
    }

    vo_err_trace("error layer id %d found\n", layer);
    return TD_FALSE;
}

#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
