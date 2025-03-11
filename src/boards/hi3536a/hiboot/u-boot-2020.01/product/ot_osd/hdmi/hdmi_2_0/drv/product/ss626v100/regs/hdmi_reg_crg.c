// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hdmi_reg_crg.h"
#include "hdmi_product_define.h"

volatile hdmi_reg_crg *g_crg_reg[OT_HDMI_ID_MAX] = {0};
td_s32 g_crg_addr[OT_HDMI_ID_MAX] = { HDMI0_CRG_ADDR, HDMI1_CRG_ADDR };

td_void hdmi_reg_crg_init(td_u32 id)
{
    if (g_crg_reg[id] != TD_NULL) {
        return;
    }

    g_crg_reg[id] = (volatile hdmi_reg_crg *)(td_uintptr_t)(g_crg_addr[id]);

    return;
}

td_void hdmi_reg_crg_deinit(td_u32 id)
{
    if (g_crg_reg[id] != TD_NULL) {
        g_crg_reg[id] = TD_NULL;
    }
    return;
}

td_void hdmi_reg_ctrl_osc_24m_cken_set(td_u32 id, td_u32 en)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8144 crg8144;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8144.u32);
    crg8144.u32 = hdmi_tx_reg_read(reg_addr);
    crg8144.bits.hdmitx_ctrl_osc_24m_cken = en;
    hdmi_tx_reg_write(reg_addr, crg8144.u32);

    return;
}

td_void hdmi_reg_ctrl_cec_cken_set(td_u32 id, td_u32 en)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8144 crg8144;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8144.u32);
    crg8144.u32 = hdmi_tx_reg_read(reg_addr);
    crg8144.bits.hdmitx_ctrl_cec_cken = en;
    hdmi_tx_reg_write(reg_addr, crg8144.u32);

    return;
}

td_void hdmi_reg_ctrl_os_cken_set(td_u32 id, td_u32 en)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8144 crg8144;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8144.u32);
    crg8144.u32 = hdmi_tx_reg_read(reg_addr);
    crg8144.bits.hdmitx_ctrl_os_cken = en;
    hdmi_tx_reg_write(reg_addr, crg8144.u32);

    return;
}

td_void hdmi_reg_ctrl_as_cken_set(td_u32 id, td_u32 en)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8144 crg8144;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8144.u32);
    crg8144.u32 = hdmi_tx_reg_read(reg_addr);
    crg8144.bits.hdmitx_ctrl_as_cken = en;
    hdmi_tx_reg_write(reg_addr, crg8144.u32);

    return;
}

td_void hdmi_reg_ctrl_bus_srst_req_set(td_u32 id, td_u32 reg)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8144 crg8144;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8144.u32);
    crg8144.u32 = hdmi_tx_reg_read(reg_addr);
    crg8144.bits.hdmitx_ctrl_bus_srst_req = reg;
    hdmi_tx_reg_write(reg_addr, crg8144.u32);

    return;
}

td_void hdmi_reg_ctrl_srst_req_set(td_u32 id, td_u32 reg)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8144 crg8144;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8144.u32);
    crg8144.u32 = hdmi_tx_reg_read(reg_addr);
    crg8144.bits.hdmitx_ctrl_srst_req = reg;
    hdmi_tx_reg_write(reg_addr, crg8144.u32);

    return;
}

td_void hdmi_reg_ctrl_cec_srst_req_set(td_u32 id, td_u32 reg)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8144 crg8144;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8144.u32);
    crg8144.u32 = hdmi_tx_reg_read(reg_addr);
    crg8144.bits.hdmitx_ctrl_cec_srst_req = reg;
    hdmi_tx_reg_write(reg_addr, crg8144.u32);

    return;
}

td_void hdmi_reg_hdmitx_phy_tmds_cken_set(td_u32 id, td_u32 cken)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.hdmitx_phy_tmds_cken = cken;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

td_void hdmi_reg_hdmitx_phy_modclk_cken_set(td_u32 id, td_u32 cken)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.hdmitx_phy_modclk_cken = cken;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

td_void hdmi_reg_ac_ctrl_modclk_cken_set(td_u32 id, td_u32 cken)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.ac_ctrl_modclk_cken = cken;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

td_void hdmi_reg_phy_srst_req_set(td_u32 id, td_u32 req)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.hdmitx_phy_srst_req = req;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

td_u8 hdmi_reg_phy_srst_req_get(td_u32 id)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    return crg_8152.bits.hdmitx_phy_srst_req;
}

td_void hdmi_reg_phy_bus_srst_req_set(td_u32 id, td_u32 req)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.hdmitx_phy_bus_srst_req = req;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

td_void hdmi_reg_ac_ctrl_srst_req_set(td_u32 id, td_u32 req)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.ac_ctrl_srst_req = req;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

td_void hdmi_reg_ac_ctrl_bus_srst_req_set(td_u32 id, td_u32 req)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.ac_ctrl_bus_srst_req = req;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

td_void hdmi_reg_phy_clk_pctrl_set(td_u32 id, td_u32 clk_pctrl)
{
    td_u32 *reg_addr = TD_NULL;
    peri_crg8152 crg_8152;

    reg_addr = (td_u32 *)&(g_crg_reg[id]->crg8152.u32);
    crg_8152.u32 = hdmi_tx_reg_read(reg_addr);
    crg_8152.bits.hdmitx_phy_clk_pctrl = clk_pctrl;
    hdmi_tx_reg_write(reg_addr, crg_8152.u32);

    return;
}

