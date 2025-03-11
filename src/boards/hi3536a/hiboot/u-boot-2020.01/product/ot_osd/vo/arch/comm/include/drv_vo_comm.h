// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_COMM_H
#define DRV_VO_COMM_H

#include "ot_common_vo.h"
#include "ot_common_video.h"
#include "mkp_vo.h"
#include "drv_vo_coef_comm.h"
#include "hal_vo_def.h"
#include "hal_vo_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

typedef enum {
    /* for video surface interface */
    VOU_LAYER_PIXEL_FORMAT_SP_YCBCR_400 = 0x1,
    VOU_LAYER_PIXEL_FORMAT_SP_YCBCR_420 = 0x2,
    VOU_LAYER_PIXEL_FORMAT_SP_YCBCR_422 = 0x3,
    VOU_LAYER_PIXEL_FORMAT_BUTT
} vou_layer_pixel_format;

/* vou interrupt mask type */
typedef enum {
    VO_INTREPORT_ALL = 0xffffffff
} vo_int_mask;

typedef enum {
    VO_INT_MODE_FRAME = 0x0,
    VO_INT_MODE_FIELD = 0x1,
    VO_INT_MODE_BUTT
} vo_int_mode;

#if vo_desc("pub")
hal_disp_syncinfo *vo_drv_comm_get_sync_timing(ot_vo_intf_sync sync);
td_s32 vo_drv_check_cvbs_sync(ot_vo_dev dev, ot_vo_intf_type intf_type, ot_vo_intf_sync intf_sync);
hal_disp_pixel_format vo_drv_convert_data_format(vou_layer_pixel_format data_fmt);
td_void vo_drv_board_init(td_void);
td_void vo_drv_csc_trans_to_register(csc_coef *coef);
#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of DRV_VO_COMM_H */
