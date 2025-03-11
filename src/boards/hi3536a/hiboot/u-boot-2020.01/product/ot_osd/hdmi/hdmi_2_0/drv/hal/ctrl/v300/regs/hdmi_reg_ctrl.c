// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hdmi_reg_ctrl.h"
#include "hdmi_product_define.h"

volatile tx_ctrl_regs *g_tx_ctrl_all_reg[OT_HDMI_ID_MAX] = {TD_NULL};

td_s32 hdmi_tx_ctrl_regs_init(td_u32 id)
{
    td_u32 addr;

    addr = (id == 0) ? HDMI_TX_BASE_ADDR : HDMI1_TX_BASE_ADDR;
    g_tx_ctrl_all_reg[id] = (volatile tx_ctrl_regs *)(uintptr_t)(addr + HDMI_TX_BASE_ADDR_CTRL);

    return TD_SUCCESS;
}

td_void hdmi_tx_ctrl_regs_deinit(td_u32 id)
{
    if (g_tx_ctrl_all_reg[id] != TD_NULL) {
        g_tx_ctrl_all_reg[id] = TD_NULL;
    }
    return;
}

td_void hdmi_pwd_tx_afifo_srst_req_set(td_u32 id, td_u32 tx_afifo_srst_req)
{
    td_u32 *reg_addr = TD_NULL;
    tx_pwd_rst_ctrl pwd_rst_ctrl;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->pwd_rst_ctrl.u32);
    pwd_rst_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pwd_rst_ctrl.bits.tx_afifo_srst_req = tx_afifo_srst_req;
    hdmi_tx_reg_write(reg_addr, pwd_rst_ctrl.u32);

    return;
}

td_void hdmi_pwd_tx_acr_srst_req_set(td_u32 id, td_u32 tx_acr_srst_req)
{
    td_u32 *reg_addr = TD_NULL;
    tx_pwd_rst_ctrl pwd_rst_ctrl;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->pwd_rst_ctrl.u32);
    pwd_rst_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pwd_rst_ctrl.bits.tx_acr_srst_req = tx_acr_srst_req;
    hdmi_tx_reg_write(reg_addr, pwd_rst_ctrl.u32);

    return;
}

td_void hdmi_pwd_tx_aud_srst_req_set(td_u32 id, td_u32 tx_aud_srst_req)
{
    td_u32 *reg_addr = TD_NULL;
    tx_pwd_rst_ctrl pwd_rst_ctrl;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->pwd_rst_ctrl.u32);
    pwd_rst_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pwd_rst_ctrl.bits.tx_aud_srst_req = tx_aud_srst_req;
    hdmi_tx_reg_write(reg_addr, pwd_rst_ctrl.u32);

    return;
}

td_void hdmi_pwd_tx_hdmi_srst_req_set(td_u32 id, td_u32 tx_hdmi_srst_req)
{
    td_u32 *reg_addr = TD_NULL;
    tx_pwd_rst_ctrl pwd_rst_ctrl;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->pwd_rst_ctrl.u32);
    pwd_rst_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pwd_rst_ctrl.bits.tx_hdmi_srst_req = tx_hdmi_srst_req;
    hdmi_tx_reg_write(reg_addr, pwd_rst_ctrl.u32);

    return;
}

td_void hdmi_pwd_fifo_data_in_set(td_u32 id, td_u32 pwd_fifo_data_in)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_fifo_wdata fifo_wdata;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->fifo_wdata.u32);
    fifo_wdata.u32 = hdmi_tx_reg_read(reg_addr);
    fifo_wdata.bits.pwd_fifo_data_in = pwd_fifo_data_in;
    hdmi_tx_reg_write(reg_addr, fifo_wdata.u32);

    return;
}

td_void hdmi_pwd_data_out_cnt_set(td_u32 id, td_u32 pwd_data_out_cnt)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_data_cnt data_cnt;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->data_cnt.u32);
    data_cnt.u32 = hdmi_tx_reg_read(reg_addr);
    data_cnt.bits.pwd_data_out_cnt = pwd_data_out_cnt;
    hdmi_tx_reg_write(reg_addr, data_cnt.u32);

    return;
}

td_void hdmi_pwd_slave_seg_set(td_u32 id, td_u32 pwd_slave_seg)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_slave_cfg slave_cfg;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->slave_cfg.u32);
    slave_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    slave_cfg.bits.pwd_slave_seg = pwd_slave_seg;
    hdmi_tx_reg_write(reg_addr, slave_cfg.u32);

    return;
}

td_void hdmi_pwd_slave_offset_set(td_u32 id, td_u32 pwd_slave_offset)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_slave_cfg slave_cfg;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->slave_cfg.u32);
    slave_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    slave_cfg.bits.pwd_slave_offset = pwd_slave_offset;
    hdmi_tx_reg_write(reg_addr, slave_cfg.u32);

    return;
}

td_void hdmi_pwd_slave_addr_set(td_u32 id, td_u32 pwd_slave_addr)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_slave_cfg slave_cfg;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->slave_cfg.u32);
    slave_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    slave_cfg.bits.pwd_slave_addr = pwd_slave_addr;
    hdmi_tx_reg_write(reg_addr, slave_cfg.u32);

    return;
}

td_void hdmi_pwd_mst_cmd_set(td_u32 id, td_u32 cmd)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_mst_cmd mst_cmd;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->mst_cmd.u32);
    mst_cmd.u32 = hdmi_tx_reg_read(reg_addr);
    mst_cmd.bits.pwd_mst_cmd = cmd;
    hdmi_tx_reg_write(reg_addr, mst_cmd.u32);

    return;
}

td_void hdmi_ddc_cpu_ddc_req_set(td_u32 id, td_u32 cpu_ddc_req)
{
    td_u32 *reg_addr = TD_NULL;
    ddc_mst_arb_req mst_arb_req;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->ddc_mst_req.u32);
    mst_arb_req.u32 = hdmi_tx_reg_read(reg_addr);
    mst_arb_req.bits.cpu_ddc_req = cpu_ddc_req;
    hdmi_tx_reg_write(reg_addr, mst_arb_req.u32);

    return;
}

td_u32 hdmi_pwd_fifo_data_out_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_fifo_rdata fifo_rdata;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->fifo_rdata.u32);
    fifo_rdata.u32 = hdmi_tx_reg_read(reg_addr);
    return fifo_rdata.bits.pwd_fifo_data_out;
}

td_u32 hdmi_pwd_fifo_data_cnt_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_data_cnt data_cnt;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->data_cnt.u32);
    data_cnt.u32 = hdmi_tx_reg_read(reg_addr);
    return data_cnt.bits.pwd_fifo_data_cnt;
}

td_u32 hdmi_pwd_fifo_empty_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_mst_state mst_state;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->mst_state.u32);
    mst_state.u32 = hdmi_tx_reg_read(reg_addr);
    return mst_state.bits.pwd_fifo_empty;
}

td_u32 hdmi_pwd_i2c_in_prog_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    pwd_mst_state mst_state;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->mst_state.u32);
    mst_state.u32 = hdmi_tx_reg_read(reg_addr);
    return mst_state.bits.pwd_i2c_in_prog;
}

td_u32 hdmi_ddc_cpu_ddc_req_ack_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    ddc_mst_arb_ack mst_arb_ack;

    reg_addr = (td_u32 *)&(g_tx_ctrl_all_reg[id]->ddc_mst_ack.u32);
    mst_arb_ack.u32 = hdmi_tx_reg_read(reg_addr);
    return mst_arb_ack.bits.cpu_ddc_req_ack;
}

