// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __HDMI_REG_CTRL_H__
#define __HDMI_REG_CTRL_H__

#include "ot_type.h"

typedef union {
    struct {
        td_u32 tx_pwd_srst_req    : 1;  /* [0] */
        td_u32 tx_sys_srst_req    : 1;  /* [1] */
        td_u32 tx_vid_srst_req    : 1;  /* [2] */
        td_u32 tx_hdmi_srst_req   : 1;  /* [3] */
        td_u32 tx_hdcp1x_srst_req : 1;  /* [4] */
        td_u32 tx_phy_srst_req    : 1;  /* [5] */
        td_u32 tx_aud_srst_req    : 1;  /* [6] */
        td_u32 tx_acr_srst_req    : 1;  /* [7] */
        td_u32 tx_afifo_srst_req  : 1;  /* [8] */
        td_u32 tx_hdcp2x_srst_req : 1;  /* [9] */
        td_u32 tx_mcu_srst_req    : 1;  /* [10] */
        td_u32 rsv_0              : 21; /* [31:11] */
    } bits;
    td_u32 u32;
} tx_pwd_rst_ctrl;

typedef union {
    struct {
        td_u32 pwd_fifo_data_out : 8;  /* [7:0] */
        td_u32 rsv_5             : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} pwd_fifo_rdata;

typedef union {
    struct {
        td_u32 pwd_fifo_data_in : 8;  /* [7:0] */
        td_u32 rsv_6            : 24; /* [31:8] */
    } bits;

    td_u32 u32;
} pwd_fifo_wdata;

typedef union {
    struct {
        td_u32 pwd_fifo_data_cnt : 5;  /* [4:0] */
        td_u32 rsv_7             : 3;  /* [7:5] */
        td_u32 pwd_data_out_cnt  : 10; /* [17:8] */
        td_u32 rsv_8             : 14; /* [31:18] */
    } bits;
    td_u32 u32;
} pwd_data_cnt;

typedef union {
    struct {
        td_u32 pwd_slave_addr   : 8; /* [7:0] */
        td_u32 pwd_slave_offset : 8; /* [15:8] */
        td_u32 pwd_slave_seg    : 8; /* [23:16] */
        td_u32 rsv_9            : 8; /* [31:24] */
    } bits;
    td_u32 u32;
} pwd_slave_cfg;

typedef union {
    struct {
        td_u32 pwd_i2c_no_ack     : 1;  /* [0] */
        td_u32 pwd_i2c_bus_low    : 1;  /* [1] */
        td_u32 pwd_i2c_in_prog    : 1;  /* [2] */
        td_u32 pwd_fifo_wr_in_use : 1;  /* [3] */
        td_u32 pwd_fifo_rd_in_use : 1;  /* [4] */
        td_u32 pwd_fifo_empty     : 1;  /* [5] */
        td_u32 pwd_fifo_half_full : 1;  /* [6] */
        td_u32 pwd_fifo_full      : 1;  /* [7] */
        td_u32 rsv_10             : 24; /* [31:8] */
    } bits;
    td_u32 u32;
} pwd_mst_state;

typedef union {
    struct {
        td_u32 pwd_mst_cmd : 4;  /* [3:0] */
        td_u32 rsv_11      : 28; /* [31:4] */
    } bits;
    td_u32 u32;
} pwd_mst_cmd;

typedef union {
    struct {
        td_u32 cpu_ddc_req : 1;  /* [0] */
        td_u32 rsv_14      : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} ddc_mst_arb_req;

typedef union {
    struct {
        td_u32 cpu_ddc_req_ack : 1;  /* [0] */
        td_u32 rsv_15          : 31; /* [31:1] */
    } bits;
    td_u32 u32;
} ddc_mst_arb_ack;

typedef struct {
    td_u32             reserved_0[4]; /* 0-C */
    volatile tx_pwd_rst_ctrl pwd_rst_ctrl;  /* 10 */
    td_u32             reserved_1[9]; /* 14-2c */
    volatile pwd_fifo_rdata  fifo_rdata;    /* 38 */
    volatile pwd_fifo_wdata  fifo_wdata;    /* 3C */
    volatile pwd_data_cnt    data_cnt;      /* 40 */
    volatile pwd_slave_cfg   slave_cfg;     /* 44 */
    volatile pwd_mst_state   mst_state;     /* 48 */
    volatile pwd_mst_cmd     mst_cmd;       /* 4C */
    td_u32             reserved_2[6]; /* 50-64 */
    volatile ddc_mst_arb_req ddc_mst_req;   /* 68 */
    volatile ddc_mst_arb_ack ddc_mst_ack;   /* 6C */
} tx_ctrl_regs;

td_s32 hdmi_tx_ctrl_regs_init(td_u32 id);
td_void hdmi_tx_ctrl_regs_deinit(td_u32 id);
td_void hdmi_pwd_tx_afifo_srst_req_set(td_u32 id, td_u32 tx_afifo_srst_req);
td_void hdmi_pwd_tx_acr_srst_req_set(td_u32 id, td_u32 tx_acr_srst_req);
td_void hdmi_pwd_tx_aud_srst_req_set(td_u32 id, td_u32 tx_aud_srst_req);
td_void hdmi_pwd_tx_hdmi_srst_req_set(td_u32 id, td_u32 tx_hdmi_srst_req);
td_void hdmi_pwd_fifo_data_in_set(td_u32 id, td_u32 pwd_fifo_data_in);
td_void hdmi_pwd_data_out_cnt_set(td_u32 id, td_u32 pwd_data_out_cnt);
td_void hdmi_pwd_slave_addr_set(td_u32 id, td_u32 pwd_slave_addr);
td_void hdmi_pwd_slave_offset_set(td_u32 id, td_u32 pwd_slave_offset);
td_void hdmi_pwd_slave_seg_set(td_u32 id, td_u32 pwd_slave_seg);
td_void hdmi_pwd_mst_cmd_set(td_u32 id, td_u32 cmd);
td_void hdmi_ddc_cpu_ddc_req_set(td_u32 id, td_u32 cpu_ddc_req);
td_u32 hdmi_pwd_fifo_data_out_get(td_u32 id);
td_u32 hdmi_pwd_fifo_data_cnt_get(td_u32 id);
td_u32 hdmi_pwd_i2c_in_prog_get(td_u32 id);
td_u32 hdmi_pwd_fifo_empty_get(td_u32 id);
td_u32 hdmi_ddc_cpu_ddc_req_ack_get(td_u32 id);
#endif /* __HDMI_REG_CTRL_H__ */

