// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __HDMI_REG_TX_H__
#define __HDMI_REG_TX_H__

#include "ot_type.h"

typedef union {
    struct {
        td_u32 tmds_pack_mode       : 2;  /* [1:0] */
        td_u32 reg_fifo_auto_rst_en : 1;  /* [2] */
        td_u32 reg_fifo_manu_rst    : 1;  /* [3] */
        td_u32 reg_clock_det_en     : 1;  /* [4] */
        td_u32 reg_ext_tmds_para    : 1;  /* [5] */
        td_u32 rsv_0                : 2;  /* [7:6] */
        td_u32 reg_fifo_delay_cnt   : 8;  /* [15:8] */
        td_u32 rsv_1                : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} tx_pack_fifo_ctrl;

typedef union {
    struct {
        td_u32 pclk2tclk_stable : 1;  /* [0] */
        td_u32 rsv_2            : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} tx_pack_fifo_st;

typedef union {
    struct {
        td_u32 reg_tcnt_lower_threshold : 18; /* [17:0] */
        td_u32 rsv_4                    : 14; /* [31:18] */
    } bits;
    td_u32 u32;
} tclk_lower_threshold;

typedef union {
    struct {
        td_u32 reg_tcnt_upper_threshold : 18; /* [17:0] */
        td_u32 rsv_5                    : 14; /* [31:18] */
    } bits;
    td_u32 u32;
} tclk_upper_threshold;

typedef union {
    struct {
        td_u32 avi_pkt_hb0 : 8; /* [7:0] */
        td_u32 avi_pkt_hb1 : 8; /* [15:8] */
        td_u32 avi_pkt_hb2 : 8; /* [23:16] */
        td_u32 rsv_6       : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_pkt_header;

typedef union {
    struct {
        td_u32 avi_sub_pkt0_pb0 : 8; /* [7:0] */
        td_u32 avi_sub_pkt0_pb1 : 8; /* [15:8] */
        td_u32 avi_sub_pkt0_pb2 : 8; /* [23:16] */
        td_u32 avi_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt0_l;

typedef union {
    struct {
        td_u32 avi_sub_pkt0_pb4 : 8; /* [7:0] */
        td_u32 avi_sub_pkt0_pb5 : 8; /* [15:8] */
        td_u32 avi_sub_pkt0_pb6 : 8; /* [23:16] */
        td_u32 rsv_7            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt0_h;

typedef union {
    struct {
        td_u32 avi_sub_pkt1_pb0 : 8; /* [7:0] */
        td_u32 avi_sub_pkt1_pb1 : 8; /* [15:8] */
        td_u32 avi_sub_pkt1_pb2 : 8; /* [23:16] */
        td_u32 avi_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt1_l;

typedef union {
    struct {
        td_u32 avi_sub_pkt1_pb4 : 8; /* [7:0] */
        td_u32 avi_sub_pkt1_pb5 : 8; /* [15:8] */
        td_u32 avi_sub_pkt1_pb6 : 8; /* [23:16] */
        td_u32 rsv_8            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt1_h;

typedef union {
    struct {
        td_u32 avi_sub_pkt2_pb0 : 8; /* [7:0] */
        td_u32 avi_sub_pkt2_pb1 : 8; /* [15:8] */
        td_u32 avi_sub_pkt2_pb2 : 8; /* [23:16] */
        td_u32 avi_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt2_l;

typedef union {
    struct {
        td_u32 avi_sub_pkt2_pb4 : 8; /* [7:0] */
        td_u32 avi_sub_pkt2_pb5 : 8; /* [15:8] */
        td_u32 avi_sub_pkt2_pb6 : 8; /* [23:16] */
        td_u32 rsv_9            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt2_h;

typedef union {
    struct {
        td_u32 avi_sub_pkt3_pb0 : 8; /* [7:0] */
        td_u32 avi_sub_pkt3_pb1 : 8; /* [15:8] */
        td_u32 avi_sub_pkt3_pb2 : 8; /* [23:16] */
        td_u32 avi_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt3_l;

typedef union {
    struct {
        td_u32 avi_sub_pkt3_pb4 : 8; /* [7:0] */
        td_u32 avi_sub_pkt3_pb5 : 8; /* [15:8] */
        td_u32 avi_sub_pkt3_pb6 : 8; /* [23:16] */
        td_u32 rsv_10           : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} avi_sub_pkt3_h;

typedef union {
    struct {
        td_u32 vsif_pkt_hb0 : 8; /* [7:0] */
        td_u32 vsif_pkt_hb1 : 8; /* [15:8] */
        td_u32 vsif_pkt_hb2 : 8; /* [23:16] */
        td_u32 rsv_56       : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_pkt_header;

typedef union {
    struct {
        td_u32 vsif_sub_pkt0_pb0 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt0_pb1 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt0_pb2 : 8; /* [23:16] */
        td_u32 vsif_sub_pkt0_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt0_l;

typedef union {
    struct {
        td_u32 vsif_sub_pkt0_pb4 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt0_pb5 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt0_pb6 : 8; /* [23:16] */
        td_u32 rsv_57            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt0_h;

typedef union {
    struct {
        td_u32 vsif_sub_pkt1_pb0 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt1_pb1 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt1_pb2 : 8; /* [23:16] */
        td_u32 vsif_sub_pkt1_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt1_l;

typedef union {
    struct {
        td_u32 vsif_sub_pkt1_pb4 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt1_pb5 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt1_pb6 : 8; /* [23:16] */
        td_u32 rsv_58            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt1_h;

typedef union {
    struct {
        td_u32 vsif_sub_pkt2_pb0 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt2_pb1 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt2_pb2 : 8; /* [23:16] */
        td_u32 vsif_sub_pkt2_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt2_l;

typedef union {
    struct {
        td_u32 vsif_sub_pkt2_pb4 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt2_pb5 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt2_pb6 : 8; /* [23:16] */
        td_u32 rsv_59            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt2_h;

typedef union {
    struct {
        td_u32 vsif_sub_pkt3_pb0 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt3_pb1 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt3_pb2 : 8; /* [23:16] */
        td_u32 vsif_sub_pkt3_pb3 : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt3_l;

typedef union {
    struct {
        td_u32 vsif_sub_pkt3_pb4 : 8; /* [7:0] */
        td_u32 vsif_sub_pkt3_pb5 : 8; /* [15:8] */
        td_u32 vsif_sub_pkt3_pb6 : 8; /* [23:16] */
        td_u32 rsv_60            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} vsif_sub_pkt3_h;

typedef union {
    struct {
        td_u32 cea_avi_en     : 1;  /* [0] */
        td_u32 cea_avi_rpt_en : 1;  /* [1] */
        td_u32 rsv_61         : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} cea_avi_cfg;

typedef union {
    struct {
        td_u32 cea_gen_en     : 1;  /* [0] */
        td_u32 cea_gen_rpt_en : 1;  /* [1] */
        td_u32 rsv_65         : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} cea_gen_cfg;

typedef union {
    struct {
        td_u32 cea_cp_en      : 1;  /* [0] */
        td_u32 cea_cp_rpt_en  : 1;  /* [1] */
        td_u32 cea_cp_rpt_cnt : 8;  /* [2:7] */
        td_u32 rsv_66         : 22; /* [31:8] */
    } bits;
    td_u32 u32;
} cea_cp_cfg;

typedef union {
    struct {
        td_u32 cea_vsif_en     : 1;  /* [0] */
        td_u32 cea_vsif_rpt_en : 1;  /* [1] */
        td_u32 rsv_72          : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} cea_vsif_cfg;

typedef union {
    struct {
        td_u32 hdmi_mode           : 1;  /* [0] */
        td_u32 dc_pkt_en           : 1;  /* [1] */
        td_u32 null_pkt_en         : 1;  /* [2] */
        td_u32 null_pkt_en_vs_high : 1;  /* [3] */
        td_u32 intr_encryption     : 1;  /* [4] */
        td_u32 pb_ovr_dc_pkt_en    : 1;  /* [5] */
        td_u32 pb_priotity_ctl     : 1;  /* [6] */
        td_u32 pkt_bypass_mode     : 1;  /* [7] */
        td_u32 avmute_in_phase     : 1;  /* [8] */
        td_u32 hdmi_dvi_sel        : 1;  /* [9] */
        td_u32 eess_mode_en        : 1;  /* [10] */
        td_u32 rsv_74              : 21; /* [31:11] */
    } bits;
    td_u32 u32;
} avmixer_config;

typedef union {
    struct {
        td_u32 cp_set_avmute : 1;  /* [0] */
        td_u32 cp_clr_avmute : 1;  /* [1] */
        td_u32 rsv_75        : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} cp_pkt_avmute;

typedef union {
    struct {
        td_u32 enc_hdmi2_on   : 1;  /* [0] */
        td_u32 enc_scr_on     : 1;  /* [1] */
        td_u32 enc_scr_md     : 1;  /* [2] */
        td_u32 enc_hdmi_val   : 1;  /* [3] */
        td_u32 enc_hdmi_ovr   : 1;  /* [4] */
        td_u32 enc_bypass     : 1;  /* [5] */
        td_u32 enc_ck_div_sel : 2;  /* [7:6] */
        td_u32 rsv_78         : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} hdmi_enc_ctrl;

typedef struct {
    volatile tx_pack_fifo_ctrl pack_fifo_ctrl;     /* 1800 */
    volatile tx_pack_fifo_st   pack_fifo_st;       /* 1804 */
    td_u32                     reserved_0[4];
    volatile avi_pkt_header    avi_header;         /* 1818 */
    volatile avi_sub_pkt0_l    avi_pkt0_l;         /* 181C */
    volatile avi_sub_pkt0_h    avi_pkt0_h;         /* 1820 */
    volatile avi_sub_pkt1_l    avi_pkt1_l;         /* 1824 */
    volatile avi_sub_pkt1_h    avi_pkt1_h;         /* 1828 */
    volatile avi_sub_pkt2_l    avi_pkt2_l;         /* 182C */
    volatile avi_sub_pkt2_h    avi_pkt2_h;         /* 1830 */
    volatile avi_sub_pkt3_l    avi_pkt3_l;         /* 1834 */
    volatile avi_sub_pkt3_h    avi_pkt3_h;         /* 1838 */
    td_u32                     reserved_1[81];
    volatile vsif_pkt_header   vsif_header;        /* 1980 */
    volatile vsif_sub_pkt0_l   vsif_pkt0_l;        /* 1984 */
    volatile vsif_sub_pkt0_h   vsif_pkt0_h;        /* 1988 */
    volatile vsif_sub_pkt1_l   vsif_pkt1_l;        /* 198C */
    volatile vsif_sub_pkt1_h   vsif_pkt1_h;        /* 1990 */
    volatile vsif_sub_pkt2_l   vsif_pkt2_l;        /* 1994 */
    volatile vsif_sub_pkt2_h   vsif_pkt2_h;        /* 1998 */
    volatile vsif_sub_pkt3_l   vsif_pkt3_l;        /* 199C */
    volatile vsif_sub_pkt3_h   vsif_pkt3_h;        /* 19A0 */
    volatile cea_avi_cfg       avi_cfg;            /* 19A4 */
    td_u32                     reserved_2[4];
    volatile cea_cp_cfg        cp_cfg;             /* 19B8 */
    td_u32                     reserved_3[5];
    volatile cea_vsif_cfg      vsif_cfg;           /* 19D0 */
    td_u32                     reserved_4[13];
    volatile avmixer_config    reg_avmixer_config; /* 1A08 */
    volatile cp_pkt_avmute     pkt_avmute;         /* 1A0C */
    td_u32                     reserved_6[20];
    volatile hdmi_enc_ctrl     enc_ctrl;           /* 1A60 */
} tx_hdmi_regs;

td_s32 hdmi_tx_hdmi_regs_init(td_u32 id);
td_void hdmi_tx_hdmi_regs_deinit(td_u32 id);
td_void hdmi_tx_tmds_pack_mode_set(td_u32 id, td_u32 tmds_pack_mode);
td_void hdmi_avi_pkt_header_hb_set(td_u32 id, td_u32 hb0, td_u32 hb1, td_u32 hb2);
td_void hdmi_l_avi_pkt0_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_avi_pkt0_pb_set(td_u32 id, td_u32 avi_pkt0_pb4, td_u32 avi_pkt0_pb5, td_u32 avi_pkt0_pb6);
td_void hdmi_l_avi_pkt1_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_avi_pkt1_pb_set(td_u32 id, td_u32 avi_pkt1_pb4, td_u32 avi_pkt1_pb5, td_u32 avi_pkt1_pb6);
td_void hdmi_l_avi_pkt2_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_avi_pkt2_pb_set(td_u32 id, td_u32 avi_pkt2_pb4, td_u32 avi_pkt2_pb5, td_u32 avi_pkt2_pb6);
td_void hdmi_l_avi_pkt3_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_avi_pkt3_pb_set(td_u32 id, td_u32 avi_pkt3_pb4, td_u32 avi_pkt3_pb5, td_u32 avi_pkt3_pb6);
td_void hdmi_vsif_pkt_header_hb_set(td_u32 id, td_u32 hb0, td_u32 hb1, td_u32 hb2);
td_void hdmi_l_vsif_pkt0_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_vsif_pkt0_pb_set(td_u32 id, td_u32 vsif_pkt0_pb4, td_u32 vsif_pkt0_pb5, td_u32 vsif_pkt0_pb6);
td_void hdmi_l_vsif_pkt1_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_vsif_pkt1_pb_set(td_u32 id, td_u32 vsif_pkt1_pb4, td_u32 vsif_pkt1_pb5, td_u32 vsif_pkt1_pb6);
td_void hdmi_l_vsif_pkt2_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_vsif_pkt2_pb_set(td_u32 id, td_u32 vsif_pkt2_pb4, td_u32 vsif_pkt2_pb5, td_u32 vsif_pkt2_pb6);
td_void hdmi_l_vsif_pkt3_pb_set(td_u32 id, td_u32 pb0, td_u32 pb1, td_u32 pb2, td_u32 pb3);
td_void hdmi_h_vsif_pkt3_pb_set(td_u32 id, td_u32 vsif_pkt3_pb4, td_u32 vsif_pkt3_pb5, td_u32 vsif_pkt3_pb6);
td_void hdmi_cea_avi_en_set(td_u32 id, td_u32 cea_avi_en);
td_void hdmi_cea_avi_rpt_en_set(td_u32 id, td_u32 cea_avi_rpt_en);
td_void hdmi_cea_cp_en_set(td_u32 id, td_u32 cea_cp_en);
td_void hdmi_cea_cp_rpt_cnt_set(td_u32 id, td_u32 cea_cp_rpt_cnt);
td_void hdmi_cea_cp_rpt_en_set(td_u32 id, td_u32 cea_cp_rpt_en);
td_void hdmi_cea_vsif_en_set(td_u32 id, td_u32 cea_vsif_en);
td_void hdmi_cea_vsif_rpt_en_set(td_u32 id, td_u32 cea_vsif_rpt_en);
td_void hdmi_avmixer_config_hdmi_mode_set(td_u32 id, td_u32 hdmi_mode);
td_void hdmi_avmixer_config_dc_pkt_en_set(td_u32 id, td_u32 dc_pkt_en);
td_void hdmi_avmixer_config_null_pkt_en_set(td_u32 id, td_u32 null_pkt_en);
td_void hdmi_avmixer_config_hdmi_dvi_sel_set(td_u32 id, td_u32 hdmi_dvi_sel);
td_void hdmi_avmixer_config_eess_mode_en_set(td_u32 id, td_u32 eess_mode_en);
td_void hdmi_cp_set_avmute_set(td_u32 id, td_u32 cp_set_avmute);
td_void hdmi_cp_clr_avmute_set(td_u32 id, td_u32 cp_clr_avmute);
td_void hdmi_enc_hdmi2_on_set(td_u32 id, td_u32 enc_hdmi2_on);
td_void hdmi_enc_scr_on_set(td_u32 id, td_u32 enc_scr_on);
td_void hdmi_enc_bypass_set(td_u32 id, td_u32 enc_bypass);
td_u32 hdmi_tx_pclk2tclk_stable_get(td_u32 id);
td_u32 hdmi_cea_cp_rpt_en_get(td_u32 id);
td_u32 hdmi_cp_set_avmute_get(td_u32 id);
td_u32 hdmi_enc_scr_on_get(td_u32 id);
#endif /* __HDMI_REG_TX_H__ */

