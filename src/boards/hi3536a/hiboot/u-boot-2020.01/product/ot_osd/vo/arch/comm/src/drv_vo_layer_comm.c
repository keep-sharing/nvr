// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_vo_layer_comm.h"
#include "drv_vo.h"
#include "hal_vo_layer_comm.h"
#include "hal_vo_video_comm.h"
#include "drv_vo_coef_org_comm.h"
#include "vo.h"
#include "ot_math.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")
#if vo_desc("layer csc")

td_s32 vo_drv_get_csc_matrix(ot_vo_csc_matrix csc_matrix, const csc_coef **csc_tmp)
{
    *csc_tmp = vo_get_csc_coef(csc_matrix);
    if (*csc_tmp == TD_NULL) {
        return OT_ERR_VO_NULL_PTR;
    }
    return TD_SUCCESS;
}

td_void vo_drv_calculate_yuv2rgb(const hal_csc_value *csc_value, const csc_coef *csc_tmp, csc_coef *coef)
{
    td_s32 luma;
    td_s32 contrast;
    td_s32 hue;
    td_s32 satu;
    const td_s32 csc_value_times = 100;
    const td_s32 table_times = 1000;
    td_s32 square_cv_times = csc_value_times * csc_value_times;
    const td_s32 *cos_table = vo_get_cos_table();
    const td_s32 *sin_table = vo_get_sin_table();

    luma = csc_value->luma;
    contrast = csc_value->cont;
    hue = csc_value->hue;
    satu = csc_value->satu;

    /* yuv->rgb */
    coef->csc_coef00 = (contrast * csc_tmp->csc_coef00) / csc_value_times;
    coef->csc_coef01 = (contrast * satu * ((csc_tmp->csc_coef01 * cos_table[hue] - csc_tmp->csc_coef02 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef02 = (contrast * satu * ((csc_tmp->csc_coef01 * sin_table[hue] + csc_tmp->csc_coef02 *
                                            cos_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef10 = (contrast * csc_tmp->csc_coef10) / csc_value_times;
    coef->csc_coef11 = (contrast * satu * ((csc_tmp->csc_coef11 * cos_table[hue] - csc_tmp->csc_coef12 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef12 = (contrast * satu * ((csc_tmp->csc_coef11 * sin_table[hue] + csc_tmp->csc_coef12 *
                                            cos_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef20 = (contrast * csc_tmp->csc_coef20) / csc_value_times;
    coef->csc_coef21 = (contrast * satu * ((csc_tmp->csc_coef21 * cos_table[hue] - csc_tmp->csc_coef22 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef22 = (contrast * satu * ((csc_tmp->csc_coef21 * sin_table[hue] + csc_tmp->csc_coef22 *
                                            cos_table[hue]) / table_times)) / square_cv_times;
    coef->csc_in_dc0 += ((contrast != 0) ? (luma * 100 / contrast) : (luma * 100));  /* 100 : trans coef */
}

td_void vo_drv_calculate_rgb2yuv(const hal_csc_value *csc_value, const csc_coef *csc_tmp, csc_coef *coef)
{
    td_s32 luma;
    td_s32 contrast;
    td_s32 hue;
    td_s32 satu;
    const td_s32 csc_value_times = 100;
    const td_s32 table_times = 1000;
    td_s32 square_cv_times = csc_value_times * csc_value_times;
    const td_s32 *cos_table = vo_get_cos_table();
    const td_s32 *sin_table = vo_get_sin_table();

    luma = csc_value->luma;
    contrast = csc_value->cont;
    hue = csc_value->hue;
    satu = csc_value->satu;

    /* rgb->yuv or yuv->yuv */
    coef->csc_coef00 = (contrast * csc_tmp->csc_coef00) / csc_value_times;
    coef->csc_coef01 = (contrast * csc_tmp->csc_coef01) / csc_value_times;
    coef->csc_coef02 = (contrast * csc_tmp->csc_coef02) / csc_value_times;
    coef->csc_coef10 = (contrast * satu * ((csc_tmp->csc_coef10 * cos_table[hue] + csc_tmp->csc_coef20 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef11 = (contrast * satu * ((csc_tmp->csc_coef11 * cos_table[hue] + csc_tmp->csc_coef21 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef12 = (contrast * satu * ((csc_tmp->csc_coef12 * cos_table[hue] + csc_tmp->csc_coef22 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef20 = (contrast * satu * ((csc_tmp->csc_coef20 * cos_table[hue] - csc_tmp->csc_coef10 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef21 = (contrast * satu * ((csc_tmp->csc_coef21 * cos_table[hue] - csc_tmp->csc_coef11 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_coef22 = (contrast * satu * ((csc_tmp->csc_coef22 * cos_table[hue] - csc_tmp->csc_coef12 *
                                            sin_table[hue]) / table_times)) / square_cv_times;
    coef->csc_out_dc0 += luma;
}

td_void vou_drv_calc_csc_matrix(const ot_vo_csc *csc, ot_vo_csc_matrix csc_matrix, csc_coef *coef)
{
    td_s32 ret;
    const csc_coef *csc_tmp = TD_NULL;
    hal_csc_value csc_value;

    if (csc->ex_csc_en == TD_FALSE) {
        csc_value.luma = (td_s32)csc->luma * 64 / 100 - 32; /* 64: -32~32 100: trans coef  */
    } else {
        csc_value.luma = (td_s32)csc->luma * 256 / 100 - 128; /* 256: -128~128 128 100  */
    }

    csc_value.cont = ((td_s32)csc->contrast - 50) * 2 + 100; /* 50 2 100 trans coef */
    csc_value.hue = (td_s32)csc->hue * 60 / 100;             /* 60 100 trans coef */
    csc_value.satu = ((td_s32)csc->saturation - 50) * 2 + 100; /* 50 2 100 trans coef */

    ret = vo_drv_get_csc_matrix(csc_matrix, &csc_tmp);
    if (ret != TD_SUCCESS) {
        return;
    }

    coef->csc_in_dc0 = csc_tmp->csc_in_dc0;
    coef->csc_in_dc1 = csc_tmp->csc_in_dc1;
    coef->csc_in_dc2 = csc_tmp->csc_in_dc2;
    coef->csc_out_dc0 = csc_tmp->csc_out_dc0;
    coef->csc_out_dc1 = csc_tmp->csc_out_dc1;
    coef->csc_out_dc2 = csc_tmp->csc_out_dc2;

    if ((csc_matrix >= OT_VO_CSC_MATRIX_BT601LIMIT_TO_RGBFULL) &&
        (csc_matrix <= OT_VO_CSC_MATRIX_BT709FULL_TO_RGBLIMIT)) {
        vo_drv_calculate_yuv2rgb(&csc_value, csc_tmp, coef);
    } else {
        vo_drv_calculate_rgb2yuv(&csc_value, csc_tmp, coef);
    }
}

/* coef need clip in range */
td_void vo_drv_clip_layer_csc_coef(csc_coef *coef)
{
    td_s32 min_coef = 0;
    td_s32 max_coef = 0;

    vo_drv_get_layer_csc_coef_range(&min_coef, &max_coef);
    coef->csc_coef00 = clip3(coef->csc_coef00, min_coef, max_coef);
    coef->csc_coef01 = clip3(coef->csc_coef01, min_coef, max_coef);
    coef->csc_coef02 = clip3(coef->csc_coef02, min_coef, max_coef);

    coef->csc_coef10 = clip3(coef->csc_coef10, min_coef, max_coef);
    coef->csc_coef11 = clip3(coef->csc_coef11, min_coef, max_coef);
    coef->csc_coef12 = clip3(coef->csc_coef12, min_coef, max_coef);

    coef->csc_coef20 = clip3(coef->csc_coef20, min_coef, max_coef);
    coef->csc_coef21 = clip3(coef->csc_coef21, min_coef, max_coef);
    coef->csc_coef22 = clip3(coef->csc_coef22, min_coef, max_coef);
}

td_void vo_drv_layer_get_csc_param(ot_vo_layer layer, csc_coef_param *csc_param)
{
    vo_drv_layer *drv_layer_ctx = vo_drv_get_layer_ctx(layer);
    (td_void)memcpy_s(csc_param, sizeof(csc_coef_param), &(drv_layer_ctx->csc_param),
        sizeof(csc_coef_param));
}

td_void vou_drv_layer_csc_config(ot_vo_layer layer, const ot_vo_csc *csc)
{
    csc_coef coef = {0};
    csc_coef_param csc_param = {0};

    vou_drv_calc_csc_matrix(csc, csc->csc_matrix, &coef);
    vo_drv_layer_get_csc_param(layer, &csc_param);
    vo_drv_csc_trans_to_register(&coef);
    vo_drv_clip_layer_csc_coef(&coef);
    hal_layer_set_csc_coef(layer, &coef, &csc_param);
}

#endif

#if vo_desc("layer cfg")

ot_vo_layer vo_drv_get_hw_layer(ot_vo_layer layer)
{
    return layer;
}

td_void vou_drv_layer_enable(ot_vo_layer layer, td_bool enable)
{
    ot_vo_layer hw_layer = vo_drv_get_hw_layer(layer);

    hal_video_hfir_set_ck_gt_en(hw_layer, enable);
    hal_layer_enable_layer(hw_layer, enable);
    hal_video_set_layer_ck_gt_en(layer, enable);
}

td_void vou_drv_set_layer_reg_up(ot_vo_layer layer)
{
    ot_vo_layer hw_layer = vo_drv_get_hw_layer(layer);
    hal_layer_set_reg_up(hw_layer);
}

td_void vou_drv_set_layer_data_fmt(ot_vo_layer layer, vou_layer_pixel_format data_fmt)
{
    ot_vo_layer hw_layer = vo_drv_get_hw_layer(layer);
    hal_disp_layer hal_layer;
    hal_disp_pixel_format disp_data_fmt;

    hal_layer = vou_drv_convert_layer(hw_layer);
    disp_data_fmt = vo_drv_convert_data_format(data_fmt);
    hal_layer_set_layer_data_fmt(hal_layer, disp_data_fmt);
}
#endif
#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */
