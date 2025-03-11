// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __HDMI_REG_CRG_H__
#define __HDMI_REG_CRG_H__

#include "ot_type.h"

typedef union {
    struct {
        td_u32 rsv_0                    : 2;  /* [1..0] */
        td_u32 hdmitx_ctrl_osc_24m_cken : 1;  /* [2] */
        td_u32 hdmitx_ctrl_cec_cken     : 1;  /* [3] */
        td_u32 hdmitx_ctrl_os_cken      : 1;  /* [4] */
        td_u32 hdmitx_ctrl_as_cken      : 1;  /* [5] */
        td_u32 hdmitx_ctrl_bus_srst_req : 1;  /* [6] */
        td_u32 hdmitx_ctrl_srst_req     : 1;  /* [7] */
        td_u32 hdmitx_ctrl_cec_srst_req : 1;  /* [8] */
        td_u32 rsv_1                    : 23; /* [31..9] */
    } bits;
    td_u32 u32;
} peri_crg8144;

typedef union {
    struct {
        td_u32 hdmitx_phy_tmds_cken    : 1;  /* [0] */
        td_u32 hdmitx_phy_modclk_cken  : 1;  /* [1] */
        td_u32 ac_ctrl_modclk_cken     : 1;  /* [2] */
        td_u32 rsv_0                   : 1;  /* [3] */
        td_u32 hdmitx_phy_srst_req     : 1;  /* [4] */
        td_u32 hdmitx_phy_bus_srst_req : 1;  /* [5] */
        td_u32 ac_ctrl_srst_req        : 1;  /* [6] */
        td_u32 ac_ctrl_bus_srst_req    : 1;  /* [7] */
        td_u32 hdmitx_phy_clk_pctrl    : 1;  /* [8] */
        td_u32 rsv_1                   : 23; /* [31..9] */
    } bits;
    td_u32 u32;
} peri_crg8152;

typedef struct {
    volatile peri_crg8144 crg8144; /* 0x7F40 */
    volatile td_u32 rsv[7];        /* 0x7F44~0x0x7F5C */
    volatile peri_crg8152 crg8152; /* 0x7F60 */
} hdmi_reg_crg;

td_void hdmi_reg_crg_init(td_u32 id);
td_void hdmi_reg_crg_deinit(td_u32 id);
td_void hdmi_reg_ctrl_osc_24m_cken_set(td_u32 id, td_u32 en);
td_void hdmi_reg_ctrl_cec_cken_set(td_u32 id, td_u32 en);
td_void hdmi_reg_ctrl_os_cken_set(td_u32 id, td_u32 en);
td_void hdmi_reg_ctrl_as_cken_set(td_u32 id, td_u32 en);
td_void hdmi_reg_ctrl_bus_srst_req_set(td_u32 id, td_u32 reg);
td_void hdmi_reg_ctrl_srst_req_set(td_u32 id, td_u32 reg);
td_void hdmi_reg_ctrl_cec_srst_req_set(td_u32 id, td_u32 reg);
td_void hdmi_reg_hdmitx_phy_tmds_cken_set(td_u32 id, td_u32 en);
td_void hdmi_reg_hdmitx_phy_modclk_cken_set(td_u32 id, td_u32 en);
td_void hdmi_reg_ac_ctrl_modclk_cken_set(td_u32 id, td_u32 en);
td_void hdmi_reg_phy_srst_req_set(td_u32 id, td_u32 reg);
td_u8 hdmi_reg_phy_srst_req_get(td_u32 id);
td_void hdmi_reg_phy_bus_srst_req_set(td_u32 id, td_u32 reg);
td_void hdmi_reg_ac_ctrl_srst_req_set(td_u32 id, td_u32 reg);
td_void hdmi_reg_ac_ctrl_bus_srst_req_set(td_u32 id, td_u32 reg);
td_void hdmi_reg_phy_clk_pctrl_set(td_u32 id, td_u32 clk_pctrl);
#endif /* __HDMI_REG_CRG_H__ */

