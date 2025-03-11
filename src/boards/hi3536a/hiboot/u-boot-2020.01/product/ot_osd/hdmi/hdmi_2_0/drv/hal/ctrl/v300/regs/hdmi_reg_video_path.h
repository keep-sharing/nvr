// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __HDMI_REG_VIDEO_PATH_H__
#define __HDMI_REG_VIDEO_PATH_H__

#include "ot_type.h"

typedef union {
    struct {
        td_u32 dither_rnd_byp      : 1;  /* [0] */
        td_u32 dither_mode         : 2;  /* [2:1] */
        td_u32 dither_rnd_en       : 1;  /* [3] */
        td_u32 dither_spatial_en   : 1;  /* [4] */
        td_u32 dither_spatial_dual : 1;  /* [5] */
        td_u32 rsv_31              : 26; /* [31:6] */
    } bits;
    td_u32 u32;
} dither_config;

typedef union {
    struct {
        td_u32 reg_csc_en          : 1;  /* [0] */
        td_u32 reg_csc_coef_ext    : 1;  /* [1] */
        td_u32 reg_csc_dither_en   : 1;  /* [2] */
        td_u32 reg_csc_saturate_en : 1;  /* [3] */
        td_u32 reg_csc_mode        : 8;  /* [11:4] */
        td_u32 rsv_41              : 20; /* [31:12] */
    } bits;
    td_u32 u32;
} multi_csc_ctrl;

typedef union {
    struct {
        td_u32 reg_dwsm_hori_en   : 1;  /* [0] */
        td_u32 reg_hori_filter_en : 1;  /* [1] */
        td_u32 reg_dwsm_vert_en   : 1;  /* [2] */
        td_u32 reg_dwsm_vert_byp  : 1;  /* [3] */
        td_u32 reg_vert_cbcr_sel  : 1;  /* [4] */
        td_u32 rsv_57             : 27; /* [31:5] */
    } bits;
    td_u32 u32;
} video_dwsm_ctrl;

typedef union {
    struct {
        td_u32 reg_demux_420_en     : 1;  /* [0] */
        td_u32 reg_ddr_en           : 1;  /* [1] */
        td_u32 reg_yc_mux_en        : 1;  /* [2] */
        td_u32 reg_blank_replace_en : 1;  /* [3] */
        td_u32 reg_pixel_rate       : 2;  /* [5:4] */
        td_u32 reg_ddr_polarity     : 1;  /* [6] */
        td_u32 reg_yc_mux_polarity  : 1;  /* [7] */
        td_u32 reg_cbcr_order       : 1;  /* [8] */
        td_u32 reg_demux_cb_or_cr   : 1;  /* [9] */
        td_u32 reg_pxl_div_en       : 1;  /* [10] */
        td_u32 rsv_58               : 21; /* [31:11] */
    } bits;
    td_u32 u32;
} data_align_ctrl;

typedef union {
    struct {
        td_u32 reg_vmux_y_sel  : 3; /* [2:0] */
        td_u32 reg_vmux_cb_sel : 3; /* [5:3] */
        td_u32 reg_vmux_cr_sel : 3; /* [8:6] */
        td_u32 reg_bitmask_y   : 2; /* [10:9] */
        td_u32 reg_bitmask_cb  : 2; /* [12:11] */
        td_u32 reg_bitmask_cr  : 2; /* [14:13] */
        td_u32 reg_bitrev_en   : 3; /* [17:15] */
        td_u32 reg_datamask_en : 3; /* [20:18] */
        td_u32 reg_syncmask_en : 4; /* [24:21] */
        td_u32 reg_inver_sync  : 4; /* [28:25] */
        td_u32 rsv_62          : 3; /* [31:29] */
    } bits;
    td_u32 u32;
} video_dmux_ctrl;

typedef struct {
    volatile dither_config   dither_cfg;     /* 8EC */
    td_u32                   reserved_4[17]; /* 8F0-930 */
    volatile multi_csc_ctrl  csc_ctrl;       /* 934 */
    td_u32                   reserved_7[18]; /* 938-97c */
    volatile video_dwsm_ctrl vo_dwsm_ctrl;   /* 980 */
    td_u32                   reserved_8[2];  /* 984-988 */
    volatile data_align_ctrl align_ctrl;     /* 98C */
    td_u32                   reserved_9[6];  /* 990-9A4 */
    volatile video_dmux_ctrl vo_dmux_ctrl;   /* 9A8 */
} video_path_regs;

td_s32 hdmi_videopath_regs_init(td_u32 id);
td_void hdmi_videopath_regs_deinit(td_u32 id);
td_void hdmi_dither_rnd_byp_set(td_u32 id, td_u32 dither_rnd_byp);
td_void hdmi_dither_mode_set(td_u32 id, td_u32 dither_mode);
td_void hdmi_reg_csc_en_set(td_u32 id, td_u32 reg_csc_en);
td_void hdmi_reg_csc_saturate_en_set(td_u32 id, td_u32 reg_csc_saturate_en);
td_void hdmi_reg_csc_mode_set(td_u32 id, td_u32 reg_csc_mode);
td_u32 hdmi_reg_csc_mode_get(td_u32 id);
td_void hdmi_reg_dwsm_hori_en_set(td_u32 id, td_u32 reg_dwsm_hori_en);
td_void hdmi_reg_hori_filter_en_set(td_u32 id, td_u32 reg_hori_filter_en);
td_void hdmi_reg_dwsm_vert_en_set(td_u32 id, td_u32 reg_dwsm_vert_en);
td_void hdmi_reg_dwsm_vert_byp_set(td_u32 id, td_u32 reg_dwsm_vert_byp);
td_void hdmi_reg_demux_420_en_set(td_u32 id, td_u32 reg_demux_420_en);
td_void hdmi_reg_pxl_div_en_set(td_u32 id, td_u32 reg_pxl_div_en);
td_void hdmi_reg_vmux_y_sel_set(td_u32 id, td_u32 reg_vmux_y_sel);
td_void hdmi_reg_vmux_cb_sel_set(td_u32 id, td_u32 reg_vmux_cb_sel);
td_void hdmi_reg_vmux_cr_sel_set(td_u32 id, td_u32 reg_vmux_cr_sel);
td_void hdmi_reg_syncmask_en_set(td_u32 id, td_u32 reg_syncmask_en);
td_void hdmi_reg_inver_sync_set(td_u32 id, td_u32 reg_inver_sync);
#endif /* __HDMI_REG_VIDEO_PATH_H__ */

