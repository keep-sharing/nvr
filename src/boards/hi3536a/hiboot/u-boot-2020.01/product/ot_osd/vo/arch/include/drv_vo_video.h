// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_VO_VIDEO_H
#define DRV_VO_VIDEO_H

#include "drv_vo_video_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* end of #ifdef __cplusplus */

#if vo_desc("UBOOT_VO")

#if vo_desc("layer")

typedef struct {
    td_u32 vid_iw;
    td_u32 vid_ih;
    td_u32 vid_ow;
    td_u32 vid_oh;

    td_u32 zme_en;
    td_u32 hfir_en;
    td_u32 csc_en;
    td_u32 hdr_en;
} vo_video_layer_ip_cfg;

typedef struct {
    td_u32 zme_vprec;
    td_u32 zme_hprec;
} vo_zme_ds_info;

#endif

#if vo_desc("layer zme")
hal_disp_layer vou_drv_convert_layer(ot_vo_layer layer);
#endif

#if vo_desc("layer")
td_void vo_drv_get_zme_typ_pq_cfg(vo_zme_comm_pq_cfg *pq_cfg);
td_void vo_drv_layer_info_init(td_void);
td_void vo_drv_video_set_zme_enable(td_u32 layer, const vo_video_layer_ip_cfg *vid_cfg);
#endif

td_void vou_drv_layer_csc_enable(ot_vo_layer layer, td_bool csc_en);
td_void vo_drv_get_layer_csc_coef_range(td_s32 *min_coef, td_s32 *max_coef);
td_s32 vo_drv_check_layer_attr_display_rect(ot_vo_layer layer, const ot_rect *rect, ot_pixel_format pixel_format);

#if vo_desc("layer open")
td_void vo_drv_layer_open(ot_vo_layer layer_id, const ot_vo_video_layer_attr *video_attr);
#endif

#endif /* #if vo_desc("UBOOT_VO") */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* end of #ifdef __cplusplus */

#endif /* end of DRV_VO_VIDEO_H */
