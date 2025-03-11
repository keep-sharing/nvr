// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __HDMI_REG_AON_H__
#define __HDMI_REG_AON_H__

#include "ot_type.h"

typedef union {
    struct {
        td_u32 tx_hw_day   : 8;  /* [7:0] */
        td_u32 tx_hw_month : 8;  /* [15:8] */
        td_u32 tx_hw_year  : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} tx_hw_info;

typedef union {
    struct {
        td_u32 tx_reg_version : 8; /* [7:0] */
        td_u32 tx_drv_version : 8; /* [15:8] */
        td_u32 tx_rtl_version : 8; /* [23:16] */
        td_u32 rsv_0          : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} tx_hw_version;

typedef union {
    struct {
        td_u32 rsv_1 : 32; /* [31:0] */
    } bits;
    td_u32 u32;
} tx_hw_feature;

typedef union {
    struct {
        td_u32 tx_aon_soft_arst_req : 1;  /* [0] */
        td_u32 rsv_2                : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} tx_aon_rst_ctrl;

typedef union {
    struct {
        td_u32 hpd_fillter_en   : 1;  /* [0] */
        td_u32 hpd_override_en  : 1;  /* [1] */
        td_u32 hpd_polarity_ctl : 1;  /* [2] */
        td_u32 hpd_soft_value   : 1;  /* [3] */
        td_u32 osc_div_cnt      : 12; /* [15:4] */
        td_u32 rsv_3            : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} hotplug_st_cfg;

typedef union {
    struct {
        td_u32 hpd_high_reshold : 16; /* [15:0] */
        td_u32 hpd_low_reshold  : 16; /* [31:16] */
    } bits;
    td_u32 u32;
} hotplug_fillter_cfg;

typedef union {
    struct {
        td_u32 hotplug_state : 1;  /* [0] */
        td_u32 phy_rx_sense  : 1;  /* [1] */
        td_u32 rsv_4         : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} tx_aon_state;

typedef union {
    struct {
        td_u32 aon_intr_mask0  : 1;  /* [0] */
        td_u32 aon_intr_mask1  : 1;  /* [1] */
        td_u32 rsv_5           : 1;  /* [2] */
        td_u32 rsv_6           : 1;  /* [3] */
        td_u32 aon_intr_mask4  : 1;  /* [4] */
        td_u32 aon_intr_mask5  : 1;  /* [5] */
        td_u32 aon_intr_mask6  : 1;  /* [6] */
        td_u32 aon_intr_mask7  : 1;  /* [7] */
        td_u32 aon_intr_mask8  : 1;  /* [8] */
        td_u32 aon_intr_mask9  : 1;  /* [9] */
        td_u32 aon_intr_mask10 : 1;  /* [10] */
        td_u32 aon_intr_mask11 : 1;  /* [11] */
        td_u32 aon_intr_mask12 : 1;  /* [12] */
        td_u32 aon_intr_mask13 : 1;  /* [13] */
        td_u32 rsv_7           : 18; /* [31:14] */
    } bits;
    td_u32 u32;
} tx_aon_intr_mask;

typedef union {
    struct {
        td_u32 aon_intr_stat0  : 1;  /* [0] */
        td_u32 aon_intr_stat1  : 1;  /* [1] */
        td_u32 rsv_8           : 1;  /* [2] */
        td_u32 rsv_9           : 1;  /* [3] */
        td_u32 aon_intr_stat4  : 1;  /* [4] */
        td_u32 aon_intr_stat5  : 1;  /* [5] */
        td_u32 aon_intr_stat6  : 1;  /* [6] */
        td_u32 aon_intr_stat7  : 1;  /* [7] */
        td_u32 aon_intr_stat8  : 1;  /* [8] */
        td_u32 aon_intr_stat9  : 1;  /* [9] */
        td_u32 aon_intr_stat10 : 1;  /* [10] */
        td_u32 aon_intr_stat11 : 1;  /* [11] */
        td_u32 aon_intr_stat12 : 1;  /* [12] */
        td_u32 aon_intr_stat13 : 1;  /* [13] */
        td_u32 rsv_10          : 18; /* [31:14] */
    } bits;
    td_u32 u32;
} tx_aon_intr_state;

typedef union {
    struct {
        td_u32 ddc_aon_access : 1;  /* [0] */
        td_u32 dcc_man_en     : 1;  /* [1] */
        td_u32 rsv_11         : 2;  /* [3:2] */
        td_u32 ddc_speed_cnt  : 9;  /* [12:4] */
        td_u32 rsv_12         : 19; /* [31:13] */
    } bits;
    td_u32 u32;
} ddc_mst_ctrl;

typedef union {
    struct {
        td_u32 ddc_fifo_data_out : 8;  /* [7:0] */
        td_u32 rsv_13            : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} ddc_fifo_rdata;

typedef union {
    struct {
        td_u32 ddc_fifo_data_in : 8;  /* [7:0] */
        td_u32 rsv_14           : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} ddc_fifo_wdata;

typedef union {
    struct {
        td_u32 ddc_fifo_data_cnt : 5;  /* [4:0] */
        td_u32 rsv_15            : 3;  /* [7:5] */
        td_u32 ddc_data_out_cnt  : 10; /* [17:8] */
        td_u32 rsv_16            : 14; /* [31:18] */
    } bits;
    td_u32 u32;
} ddc_data_cnt;

typedef union {
    struct {
        td_u32 ddc_slave_addr   : 8; /* [7:0] */
        td_u32 ddc_slave_offset : 8; /* [15:8] */
        td_u32 ddc_slave_seg    : 8; /* [23:16] */
        td_u32 rsv_17           : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} ddc_slave_cfg;

typedef union {
    struct {
        td_u32 ddc_i2c_no_ack     : 1;  /* [0] */
        td_u32 ddc_i2c_bus_low    : 1;  /* [1] */
        td_u32 ddc_i2c_in_prog    : 1;  /* [2] */
        td_u32 ddc_fifo_wr_in_use : 1;  /* [3] */
        td_u32 ddc_fifo_rd_in_use : 1;  /* [4] */
        td_u32 ddc_fifo_empty     : 1;  /* [5] */
        td_u32 ddc_fifo_half_full : 1;  /* [6] */
        td_u32 ddc_fifo_full      : 1;  /* [7] */
        td_u32 rsv_18             : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} ddc_mst_state;

typedef union {
    struct {
        td_u32 ddc_mst_cmd : 4;  /* [3:0] */
        td_u32 rsv_19      : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} ddc_mst_cmd;

typedef union {
    struct {
        td_u32 ddc_scl_st  : 1;  /* [0] */
        td_u32 ddc_sda_st  : 1;  /* [1] */
        td_u32 ddc_scl_oen : 1;  /* [2] */
        td_u32 ddc_sda_oen : 1;  /* [3] */
        td_u32 rsv_20      : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} ddc_man_ctrl;

typedef union {
    struct {
        td_u32 ddc_clr_bus_low : 1;  /* [0] */
        td_u32 ddc_clr_no_ack  : 1;  /* [1] */
        td_u32 rsv_21          : 30; /* [31:2] */
    } bits;
    td_u32 u32;
} ddc_state_clr;

typedef struct {
    volatile tx_hw_info          hw_info;            /* 4000 */
    volatile tx_hw_version       hw_version;         /* 4004 */
    volatile tx_hw_feature       hw_feature;         /* 4008 */
    td_u32                       reserved_0[4];      /* 400C-4018 */
    volatile tx_aon_rst_ctrl     aon_rst_ctrl;       /* 401C */
    volatile hotplug_st_cfg      hph_st_cfg;         /* 4020 */
    volatile hotplug_fillter_cfg hpd_fillter_cfg;    /* 4024 */
    volatile tx_aon_state        aon_state;          /* 4028 */
    td_u32                       reserved_2[1];      /* 402C */
    volatile tx_aon_intr_mask    aon_intr_mask;      /* 4030 */
    volatile tx_aon_intr_state   aon_intr_state;     /* 4034 */
    td_u32                       reserved_3[2];      /* 4038-403C */
    volatile ddc_mst_ctrl        reg_ddc_mst_ctrl;   /* 4040 */
    volatile ddc_fifo_rdata      reg_ddc_fifo_rdata; /* 4044 */
    volatile ddc_fifo_wdata      reg_ddc_fifo_wdata; /* 4048 */
    volatile ddc_data_cnt        reg_ddc_data_cnt;   /* 404C */
    volatile ddc_slave_cfg       reg_ddc_slave_cfg;  /* 4050 */
    volatile ddc_mst_state       reg_ddc_mst_state;  /* 4054 */
    volatile ddc_mst_cmd         reg_ddc_mst_cmd;    /* 4058 */
    volatile ddc_man_ctrl        reg_ddc_man_ctrl;   /* 405C */
    volatile ddc_state_clr       reg_ddc_state_clr;  /* 4060 */
} tx_aon_regs;

td_s32 hdmi_tx_aon_regs_init(td_u32 id);
td_void hdmi_tx_aon_regs_deinit(td_u32 id);
td_void hdmi_dcc_man_en_set(td_u32 id, td_u32 man_en);
td_void hdmi_ddc_scl_oen_set(td_u32 id, td_u32 scl_oen);
td_void hdmi_ddc_sda_oen_set(td_u32 id, td_u32 sda_oen);
td_u32 hdmi_ddc_i2c_bus_low_get(td_u32 id);
td_u32 hdmi_ddc_i2c_no_ack_get(td_u32 id);
td_u32 hdmi_ddc_scl_st_get(td_u32 id);
td_u32 hdmi_ddc_sda_st_get(td_u32 id);

#endif /* __HDMI_REG_AON_H__ */

