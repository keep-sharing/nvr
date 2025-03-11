// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hdmi_reg_tx.h"
#include "hdmi_product_define.h"

volatile tx_hdmi_regs *g_tx_hdmi_all_reg[OT_HDMI_ID_MAX] = {TD_NULL};

td_s32 hdmi_tx_hdmi_regs_init(td_u32 id)
{
    td_u32 addr;

    addr = (id == 0) ? HDMI_TX_BASE_ADDR : HDMI1_TX_BASE_ADDR;
    g_tx_hdmi_all_reg[id] = (volatile tx_hdmi_regs *)(uintptr_t)(addr + HDMI_TX_BASE_ADDR_HDMITX);

    return TD_SUCCESS;
}

td_void hdmi_tx_hdmi_regs_deinit(td_u32 id)
{
    if (g_tx_hdmi_all_reg[id] != TD_NULL) {
        g_tx_hdmi_all_reg[id] = TD_NULL;
    }
    return;
}

td_void hdmi_tx_tmds_pack_mode_set(td_u32 id, td_u32 tmds_pack_mode)
{
    td_u32 *reg_addr = TD_NULL;
    tx_pack_fifo_ctrl pack_fifo_ctrl;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->pack_fifo_ctrl.u32);
    pack_fifo_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    pack_fifo_ctrl.bits.tmds_pack_mode = tmds_pack_mode;
    hdmi_tx_reg_write(reg_addr, pack_fifo_ctrl.u32);

    return;
}

td_void hdmi_avi_pkt_header_hb_set(td_u32 id, td_u32 hb0, td_u32 hb1, td_u32 hb2)
{
    td_u32 *reg_addr = TD_NULL;
    avi_pkt_header pkt_header;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_header.u32);
    pkt_header.u32 = hdmi_tx_reg_read(reg_addr);
    pkt_header.bits.avi_pkt_hb2 = hb2;
    pkt_header.bits.avi_pkt_hb1 = hb1;
    pkt_header.bits.avi_pkt_hb0 = hb0;
    hdmi_tx_reg_write(reg_addr, pkt_header.u32);
    return;
}

td_void hdmi_l_avi_pkt0_pb_set(td_u32 id, td_u32 avi_pkt0_pb0, td_u32 avi_pkt0_pb1,
                               td_u32 avi_pkt0_pb2, td_u32 avi_pkt0_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt0_l sub_pkt0_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt0_l.u32);
    sub_pkt0_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt0_l.bits.avi_sub_pkt0_pb3 = avi_pkt0_pb3;
    sub_pkt0_l.bits.avi_sub_pkt0_pb2 = avi_pkt0_pb2;
    sub_pkt0_l.bits.avi_sub_pkt0_pb1 = avi_pkt0_pb1;
    sub_pkt0_l.bits.avi_sub_pkt0_pb0 = avi_pkt0_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt0_l.u32);

    return;
}

td_void hdmi_h_avi_pkt0_pb_set(td_u32 id, td_u32 avi_pkt0_pb4, td_u32 avi_pkt0_pb5,
                               td_u32 avi_pkt0_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt0_h sub_pkt0_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt0_h.u32);
    sub_pkt0_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt0_h.bits.avi_sub_pkt0_pb6 = avi_pkt0_pb6;
    sub_pkt0_h.bits.avi_sub_pkt0_pb5 = avi_pkt0_pb5;
    sub_pkt0_h.bits.avi_sub_pkt0_pb4 = avi_pkt0_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt0_h.u32);

    return;
}

td_void hdmi_l_avi_pkt1_pb_set(td_u32 id, td_u32 avi_pkt1_pb0, td_u32 avi_pkt1_pb1,
                               td_u32 avi_pkt1_pb2, td_u32 avi_pkt1_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt1_l sub_pkt1_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt1_l.u32);
    sub_pkt1_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt1_l.bits.avi_sub_pkt1_pb3 = avi_pkt1_pb3;
    sub_pkt1_l.bits.avi_sub_pkt1_pb2 = avi_pkt1_pb2;
    sub_pkt1_l.bits.avi_sub_pkt1_pb1 = avi_pkt1_pb1;
    sub_pkt1_l.bits.avi_sub_pkt1_pb0 = avi_pkt1_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt1_l.u32);

    return;
}

td_void hdmi_h_avi_pkt1_pb_set(td_u32 id, td_u32 avi_pkt1_pb4, td_u32 avi_pkt1_pb5,
                               td_u32 avi_pkt1_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt1_h sub_pkt1_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt1_h.u32);
    sub_pkt1_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt1_h.bits.avi_sub_pkt1_pb6 = avi_pkt1_pb6;
    sub_pkt1_h.bits.avi_sub_pkt1_pb5 = avi_pkt1_pb5;
    sub_pkt1_h.bits.avi_sub_pkt1_pb4 = avi_pkt1_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt1_h.u32);

    return;
}

td_void hdmi_l_avi_pkt2_pb_set(td_u32 id, td_u32 avi_pkt2_pb0, td_u32 avi_pkt2_pb1,
                               td_u32 avi_pkt2_pb2, td_u32 avi_pkt2_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt2_l sub_pkt2_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt2_l.u32);
    sub_pkt2_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt2_l.bits.avi_sub_pkt2_pb3 = avi_pkt2_pb3;
    sub_pkt2_l.bits.avi_sub_pkt2_pb2 = avi_pkt2_pb2;
    sub_pkt2_l.bits.avi_sub_pkt2_pb1 = avi_pkt2_pb1;
    sub_pkt2_l.bits.avi_sub_pkt2_pb0 = avi_pkt2_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt2_l.u32);

    return;
}

td_void hdmi_h_avi_pkt2_pb_set(td_u32 id, td_u32 avi_pkt2_pb4, td_u32 avi_pkt2_pb5,
                               td_u32 avi_pkt2_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt2_h sub_pkt2_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt2_h.u32);
    sub_pkt2_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt2_h.bits.avi_sub_pkt2_pb6 = avi_pkt2_pb6;
    sub_pkt2_h.bits.avi_sub_pkt2_pb5 = avi_pkt2_pb5;
    sub_pkt2_h.bits.avi_sub_pkt2_pb4 = avi_pkt2_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt2_h.u32);

    return;
}

td_void hdmi_l_avi_pkt3_pb_set(td_u32 id, td_u32 avi_pkt3_pb0, td_u32 avi_pkt3_pb1,
                               td_u32 avi_pkt3_pb2, td_u32 avi_pkt3_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt3_l sub_pkt3_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt3_l.u32);
    sub_pkt3_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt3_l.bits.avi_sub_pkt3_pb3 = avi_pkt3_pb3;
    sub_pkt3_l.bits.avi_sub_pkt3_pb2 = avi_pkt3_pb2;
    sub_pkt3_l.bits.avi_sub_pkt3_pb1 = avi_pkt3_pb1;
    sub_pkt3_l.bits.avi_sub_pkt3_pb0 = avi_pkt3_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt3_l.u32);

    return;
}

td_void hdmi_h_avi_pkt3_pb_set(td_u32 id, td_u32 avi_pkt3_pb4, td_u32 avi_pkt3_pb5,
                               td_u32 avi_pkt3_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    avi_sub_pkt3_h sub_pkt3_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_pkt3_h.u32);
    sub_pkt3_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt3_h.bits.avi_sub_pkt3_pb6 = avi_pkt3_pb6;
    sub_pkt3_h.bits.avi_sub_pkt3_pb5 = avi_pkt3_pb5;
    sub_pkt3_h.bits.avi_sub_pkt3_pb4 = avi_pkt3_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt3_h.u32);

    return;
}

td_void hdmi_vsif_pkt_header_hb_set(td_u32 id, td_u32 hb0, td_u32 hb1, td_u32 hb2)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_pkt_header pkt_header;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_header.u32);
    pkt_header.u32 = hdmi_tx_reg_read(reg_addr);
    pkt_header.bits.vsif_pkt_hb2 = hb2;
    pkt_header.bits.vsif_pkt_hb1 = hb1;
    pkt_header.bits.vsif_pkt_hb0 = hb0;
    hdmi_tx_reg_write(reg_addr, pkt_header.u32);

    return;
}

td_void hdmi_l_vsif_pkt0_pb_set(td_u32 id, td_u32 vsif_pkt0_pb0, td_u32 vsif_pkt0_pb1,
                                td_u32 vsif_pkt0_pb2, td_u32 vsif_pkt0_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt0_l sub_pkt0_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt0_l.u32);
    sub_pkt0_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt0_l.bits.vsif_sub_pkt0_pb3 = vsif_pkt0_pb3;
    sub_pkt0_l.bits.vsif_sub_pkt0_pb2 = vsif_pkt0_pb2;
    sub_pkt0_l.bits.vsif_sub_pkt0_pb1 = vsif_pkt0_pb1;
    sub_pkt0_l.bits.vsif_sub_pkt0_pb0 = vsif_pkt0_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt0_l.u32);

    return;
}

td_void hdmi_h_vsif_pkt0_pb_set(td_u32 id, td_u32 vsif_pkt0_pb4, td_u32 vsif_pkt0_pb5,
                                td_u32 vsif_pkt0_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt0_h sub_pkt0_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt0_h.u32);
    sub_pkt0_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt0_h.bits.vsif_sub_pkt0_pb6 = vsif_pkt0_pb6;
    sub_pkt0_h.bits.vsif_sub_pkt0_pb5 = vsif_pkt0_pb5;
    sub_pkt0_h.bits.vsif_sub_pkt0_pb4 = vsif_pkt0_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt0_h.u32);

    return;
}

td_void hdmi_l_vsif_pkt1_pb_set(td_u32 id, td_u32 vsif_pkt1_pb0, td_u32 vsif_pkt1_pb1,
                                td_u32 vsif_pkt1_pb2, td_u32 vsif_pkt1_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt1_l sub_pkt1_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt1_l.u32);
    sub_pkt1_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt1_l.bits.vsif_sub_pkt1_pb3 = vsif_pkt1_pb3;
    sub_pkt1_l.bits.vsif_sub_pkt1_pb2 = vsif_pkt1_pb2;
    sub_pkt1_l.bits.vsif_sub_pkt1_pb1 = vsif_pkt1_pb1;
    sub_pkt1_l.bits.vsif_sub_pkt1_pb0 = vsif_pkt1_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt1_l.u32);

    return;
}

td_void hdmi_h_vsif_pkt1_pb_set(td_u32 id, td_u32 vsif_pkt1_pb4, td_u32 vsif_pkt1_pb5,
                                td_u32 vsif_pkt1_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt1_h sub_pkt1_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt1_h.u32);
    sub_pkt1_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt1_h.bits.vsif_sub_pkt1_pb6 = vsif_pkt1_pb6;
    sub_pkt1_h.bits.vsif_sub_pkt1_pb5 = vsif_pkt1_pb5;
    sub_pkt1_h.bits.vsif_sub_pkt1_pb4 = vsif_pkt1_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt1_h.u32);

    return;
}

td_void hdmi_l_vsif_pkt2_pb_set(td_u32 id, td_u32 vsif_pkt2_pb0, td_u32 vsif_pkt2_pb1,
                                td_u32 vsif_pkt2_pb2, td_u32 vsif_pkt2_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt2_l sub_pkt2_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt2_l.u32);
    sub_pkt2_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt2_l.bits.vsif_sub_pkt2_pb3 = vsif_pkt2_pb3;
    sub_pkt2_l.bits.vsif_sub_pkt2_pb2 = vsif_pkt2_pb2;
    sub_pkt2_l.bits.vsif_sub_pkt2_pb1 = vsif_pkt2_pb1;
    sub_pkt2_l.bits.vsif_sub_pkt2_pb0 = vsif_pkt2_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt2_l.u32);

    return;
}

td_void hdmi_h_vsif_pkt2_pb_set(td_u32 id, td_u32 vsif_pkt2_pb4, td_u32 vsif_pkt2_pb5,
                                td_u32 vsif_pkt2_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt2_h sub_pkt2_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt2_h.u32);
    sub_pkt2_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt2_h.bits.vsif_sub_pkt2_pb6 = vsif_pkt2_pb6;
    sub_pkt2_h.bits.vsif_sub_pkt2_pb5 = vsif_pkt2_pb5;
    sub_pkt2_h.bits.vsif_sub_pkt2_pb4 = vsif_pkt2_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt2_h.u32);

    return;
}

td_void hdmi_l_vsif_pkt3_pb_set(td_u32 id, td_u32 vsif_pkt3_pb0, td_u32 vsif_pkt3_pb1,
                                td_u32 vsif_pkt3_pb2, td_u32 vsif_pkt3_pb3)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt3_l sub_pkt3_l;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt3_l.u32);
    sub_pkt3_l.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt3_l.bits.vsif_sub_pkt3_pb3 = vsif_pkt3_pb3;
    sub_pkt3_l.bits.vsif_sub_pkt3_pb2 = vsif_pkt3_pb2;
    sub_pkt3_l.bits.vsif_sub_pkt3_pb1 = vsif_pkt3_pb1;
    sub_pkt3_l.bits.vsif_sub_pkt3_pb0 = vsif_pkt3_pb0;
    hdmi_tx_reg_write(reg_addr, sub_pkt3_l.u32);

    return;
}

td_void hdmi_h_vsif_pkt3_pb_set(td_u32 id, td_u32 vsif_pkt3_pb4, td_u32 vsif_pkt3_pb5,
                                td_u32 vsif_pkt3_pb6)
{
    td_u32 *reg_addr = TD_NULL;
    vsif_sub_pkt3_h sub_pkt3_h;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_pkt3_h.u32);
    sub_pkt3_h.u32 = hdmi_tx_reg_read(reg_addr);
    sub_pkt3_h.bits.vsif_sub_pkt3_pb6 = vsif_pkt3_pb6;
    sub_pkt3_h.bits.vsif_sub_pkt3_pb5 = vsif_pkt3_pb5;
    sub_pkt3_h.bits.vsif_sub_pkt3_pb4 = vsif_pkt3_pb4;
    hdmi_tx_reg_write(reg_addr, sub_pkt3_h.u32);

    return;
}

td_void hdmi_cea_avi_rpt_en_set(td_u32 id, td_u32 cea_avi_rpt_en)
{
    td_u32 *reg_addr = TD_NULL;
    cea_avi_cfg avi_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_cfg.u32);
    avi_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    avi_cfg.bits.cea_avi_rpt_en = cea_avi_rpt_en;
    hdmi_tx_reg_write(reg_addr, avi_cfg.u32);

    return;
}

td_void hdmi_cea_avi_en_set(td_u32 id, td_u32 cea_avi_en)
{
    td_u32 *reg_addr = TD_NULL;
    cea_avi_cfg avi_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->avi_cfg.u32);
    avi_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    avi_cfg.bits.cea_avi_en = cea_avi_en;
    hdmi_tx_reg_write(reg_addr, avi_cfg.u32);

    return;
}

td_void hdmi_cea_cp_rpt_cnt_set(td_u32 id, td_u32 cea_cp_rpt_cnt)
{
    td_u32 *reg_addr = TD_NULL;
    cea_cp_cfg cp_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->cp_cfg.u32);
    cp_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    cp_cfg.bits.cea_cp_rpt_cnt = cea_cp_rpt_cnt;
    hdmi_tx_reg_write(reg_addr, cp_cfg.u32);

    return;
}

td_void hdmi_cea_cp_rpt_en_set(td_u32 id, td_u32 cea_cp_rpt_en)
{
    td_u32 *reg_addr = TD_NULL;
    cea_cp_cfg cp_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->cp_cfg.u32);
    cp_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    cp_cfg.bits.cea_cp_rpt_en = cea_cp_rpt_en;
    hdmi_tx_reg_write(reg_addr, cp_cfg.u32);

    return;
}

td_void hdmi_cea_cp_en_set(td_u32 id, td_u32 cea_cp_en)
{
    td_u32 *reg_addr = TD_NULL;
    cea_cp_cfg cp_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->cp_cfg.u32);
    cp_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    cp_cfg.bits.cea_cp_en = cea_cp_en;
    hdmi_tx_reg_write(reg_addr, cp_cfg.u32);

    return;
}

td_void hdmi_cea_vsif_rpt_en_set(td_u32 id, td_u32 cea_vsif_rpt_en)
{
    td_u32 *reg_addr = TD_NULL;
    cea_vsif_cfg vsif_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_cfg.u32);
    vsif_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_cfg.bits.cea_vsif_rpt_en = cea_vsif_rpt_en;
    hdmi_tx_reg_write(reg_addr, vsif_cfg.u32);

    return;
}

td_void hdmi_cea_vsif_en_set(td_u32 id, td_u32 cea_vsif_en)
{
    td_u32 *reg_addr = TD_NULL;
    cea_vsif_cfg vsif_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->vsif_cfg.u32);
    vsif_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    vsif_cfg.bits.cea_vsif_en = cea_vsif_en;
    hdmi_tx_reg_write(reg_addr, vsif_cfg.u32);

    return;
}

td_void hdmi_avmixer_config_eess_mode_en_set(td_u32 id, td_u32 eess_mode_en)
{
    td_u32 *reg_addr = TD_NULL;
    avmixer_config avmixer_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->reg_avmixer_config.u32);
    avmixer_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    avmixer_cfg.bits.eess_mode_en = eess_mode_en;
    hdmi_tx_reg_write(reg_addr, avmixer_cfg.u32);

    return;
}

td_void hdmi_avmixer_config_hdmi_dvi_sel_set(td_u32 id, td_u32 hdmi_dvi_sel)
{
    td_u32 *reg_addr = TD_NULL;
    avmixer_config avmixer_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->reg_avmixer_config.u32);
    avmixer_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    avmixer_cfg.bits.hdmi_dvi_sel = hdmi_dvi_sel;
    hdmi_tx_reg_write(reg_addr, avmixer_cfg.u32);

    return;
}

td_void hdmi_avmixer_config_dc_pkt_en_set(td_u32 id, td_u32 dc_pkt_en)
{
    td_u32 *reg_addr = TD_NULL;
    avmixer_config avmixer_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->reg_avmixer_config.u32);
    avmixer_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    avmixer_cfg.bits.dc_pkt_en = dc_pkt_en;
    hdmi_tx_reg_write(reg_addr, avmixer_cfg.u32);

    return;
}

td_void hdmi_avmixer_config_null_pkt_en_set(td_u32 id, td_u32 null_pkt_en)
{
    td_u32 *reg_addr = TD_NULL;
    avmixer_config avmixer_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->reg_avmixer_config.u32);
    avmixer_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    avmixer_cfg.bits.null_pkt_en = null_pkt_en;
    hdmi_tx_reg_write(reg_addr, avmixer_cfg.u32);

    return;
}

td_void hdmi_avmixer_config_hdmi_mode_set(td_u32 id, td_u32 hdmi_mode)
{
    td_u32 *reg_addr = TD_NULL;
    avmixer_config avmixer_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->reg_avmixer_config.u32);
    avmixer_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    avmixer_cfg.bits.hdmi_mode = hdmi_mode;
    hdmi_tx_reg_write(reg_addr, avmixer_cfg.u32);

    return;
}

td_void hdmi_cp_clr_avmute_set(td_u32 id, td_u32 cp_clr_avmute)
{
    td_u32 *reg_addr = TD_NULL;
    cp_pkt_avmute pkt_avmute;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->pkt_avmute.u32);
    pkt_avmute.u32 = hdmi_tx_reg_read(reg_addr);
    pkt_avmute.bits.cp_clr_avmute = cp_clr_avmute;
    hdmi_tx_reg_write(reg_addr, pkt_avmute.u32);

    return;
}

td_void hdmi_cp_set_avmute_set(td_u32 id, td_u32 cp_set_avmute)
{
    td_u32 *reg_addr = TD_NULL;
    cp_pkt_avmute pkt_avmute;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->pkt_avmute.u32);
    pkt_avmute.u32 = hdmi_tx_reg_read(reg_addr);
    pkt_avmute.bits.cp_set_avmute = cp_set_avmute;
    hdmi_tx_reg_write(reg_addr, pkt_avmute.u32);

    return;
}

td_void hdmi_enc_bypass_set(td_u32 id, td_u32 enc_bypass)
{
    td_u32 *reg_addr = TD_NULL;
    hdmi_enc_ctrl enc_ctrl;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->enc_ctrl.u32);
    enc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    enc_ctrl.bits.enc_bypass = enc_bypass;
    hdmi_tx_reg_write(reg_addr, enc_ctrl.u32);

    return;
}

td_void hdmi_enc_scr_on_set(td_u32 id, td_u32 enc_scr_on)
{
    td_u32 *reg_addr = TD_NULL;
    hdmi_enc_ctrl enc_ctrl;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->enc_ctrl.u32);
    enc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    enc_ctrl.bits.enc_scr_on = enc_scr_on;
    hdmi_tx_reg_write(reg_addr, enc_ctrl.u32);

    return;
}

td_void hdmi_enc_hdmi2_on_set(td_u32 id, td_u32 enc_hdmi2_on)
{
    td_u32 *reg_addr = TD_NULL;
    hdmi_enc_ctrl enc_ctrl;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->enc_ctrl.u32);
    enc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    enc_ctrl.bits.enc_hdmi2_on = enc_hdmi2_on;
    hdmi_tx_reg_write(reg_addr, enc_ctrl.u32);

    return;
}

td_u32 hdmi_tx_pclk2tclk_stable_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    tx_pack_fifo_st pack_fifo_st;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->pack_fifo_st.u32);
    pack_fifo_st.u32 = hdmi_tx_reg_read(reg_addr);
    return pack_fifo_st.bits.pclk2tclk_stable;
}

td_u32 hdmi_cea_cp_rpt_en_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    cea_cp_cfg cp_cfg;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->cp_cfg.u32);
    cp_cfg.u32 = hdmi_tx_reg_read(reg_addr);
    return cp_cfg.bits.cea_cp_rpt_en;
}

td_u32 hdmi_cp_set_avmute_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    cp_pkt_avmute pkt_avmute;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->pkt_avmute.u32);
    pkt_avmute.u32 = hdmi_tx_reg_read(reg_addr);
    return pkt_avmute.bits.cp_set_avmute;
}

td_u32 hdmi_enc_scr_on_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    hdmi_enc_ctrl enc_ctrl;

    reg_addr = (td_u32 *)&(g_tx_hdmi_all_reg[id]->enc_ctrl.u32);
    enc_ctrl.u32 = hdmi_tx_reg_read(reg_addr);
    return enc_ctrl.bits.enc_scr_on;
}

