// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_COEF_COMM_H
#define DRV_VO_COEF_COMM_H

#include "ot_type.h"
#include "inner_vo.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

typedef struct {
    td_s32 csc_coef00;
    td_s32 csc_coef01;
    td_s32 csc_coef02;

    td_s32 csc_coef10;
    td_s32 csc_coef11;
    td_s32 csc_coef12;

    td_s32 csc_coef20;
    td_s32 csc_coef21;
    td_s32 csc_coef22;
} vdp_csc_coef;

typedef struct {
    td_s32 csc_in_dc0;
    td_s32 csc_in_dc1;
    td_s32 csc_in_dc2;

    td_s32 csc_out_dc0;
    td_s32 csc_out_dc1;
    td_s32 csc_out_dc2;
} vdp_csc_dc_coef;

typedef struct {
    /* for old version csc */
    td_s32 csc_coef00;
    td_s32 csc_coef01;
    td_s32 csc_coef02;

    td_s32 csc_coef10;
    td_s32 csc_coef11;
    td_s32 csc_coef12;

    td_s32 csc_coef20;
    td_s32 csc_coef21;
    td_s32 csc_coef22;

    td_s32 csc_in_dc0;
    td_s32 csc_in_dc1;
    td_s32 csc_in_dc2;

    td_s32 csc_out_dc0;
    td_s32 csc_out_dc1;
    td_s32 csc_out_dc2;
} csc_coef;

typedef struct {
    td_s32 csc_scale2p;
    td_s32 csc_clip_min;
    td_s32 csc_clip_max;
} csc_coef_param;

typedef struct {
    td_s32 vga_hsp_tmp0;
    td_s32 vga_hsp_tmp1;
    td_s32 vga_hsp_tmp2;
    td_s32 vga_hsp_tmp3;
    td_u32 vga_hsp_coring;
    td_s32 vga_hsp_gainneg;
    td_s32 vga_hsp_gainpos;
    td_s32 vga_hsp_adpshooten;
    td_u32 vga_hsp_winsize;
    td_u32 vga_hsp_mixratio;
    td_u32 vga_hsp_underth;
    td_u32 vga_hsp_overth;
} hsp_hf_coef;

typedef struct {
    td_u32 vga_hsp_hf_shootdiv;
    td_u32 vga_hsp_lti_ratio;
    td_u32 vga_hsp_ldti_gain;
    td_u32 vga_hsp_cdti_gain;
    td_u32 vga_hsp_peak_ratio;
    td_u32 vga_hsp_glb_overth;
    td_u32 vga_hsp_glb_underth;
} hsp_coef;

/* CVFIR VCOEF */
typedef struct {
    td_s32 vccoef00;
    td_s32 vccoef01;
    td_s32 vccoef02;
    td_s32 vccoef03;
    td_s32 vccoef10;
    td_s32 vccoef11;
    td_s32 vccoef12;
    td_s32 vccoef13;
} cvfir_coef;

/* HFIR VCOEF */
typedef struct {
    td_s32 coef0;
    td_s32 coef1;
    td_s32 coef2;
    td_s32 coef3;
    td_s32 coef4;
    td_s32 coef5;
    td_s32 coef6;
    td_s32 coef7;
} hfir_coef;
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of #ifndef DRV_VO_COEF_COMM_H */
