// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "hdmi_product_define.h"
#include "hdmi_reg_crg.h"
#include "hdmi_reg_dphy.h"

#define CRG_RESET_DELAY 2
#define HDMI_IO_CFG_FUNCTION1_SEL 0x3111

td_s32 hdmi_tx_reg_write(td_u32 *reg_addr, td_u32 value)
{
    hdmi_if_null_return(reg_addr, TD_FAILURE);
    *(volatile td_u32 *)reg_addr = value;
    return TD_SUCCESS;
}

td_u32 hdmi_tx_reg_read(const td_u32 *reg_addr)
{
    hdmi_if_null_return(reg_addr, TD_FAILURE);
    return *(volatile td_u32 *)(reg_addr);
}

td_void hdmi_reg_write_u32(td_u32 reg_addr, td_u32 value)
{
    volatile td_u32 *addr = TD_NULL;

    addr = (volatile td_u32 *)(uintptr_t)reg_addr;
    if (addr != TD_NULL) {
        *addr = value;
    } else {
        hdmi_err("addr=0x%x err!\n", reg_addr);
    }

    return;
}

td_u32 hdmi_reg_read_u32(td_u32 reg_addr)
{
    td_u32 value = 0;
    volatile td_u32 *addr = TD_NULL;

    addr = (volatile td_u32 *)(uintptr_t)reg_addr;
    if (addr != TD_NULL) {
        value = *addr;
    } else {
        hdmi_err("addr=0x%x\n err!\n", reg_addr);
    }

    return value;
}

td_void drv_hdmi_prod_io_cfg_set(td_u32 id)
{
    hdmi_if_fpga_return_void();

    if (id == 0) {
        hdmi_reg_write_u32(HDMI_ADDR_IO_CFG_HOTPLUG, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI_ADDR_IO_CFG_SDA, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI_ADDR_IO_CFG_SCL, HDMI_IO_CFG_FUNCTION1_SEL);
    } else {
        hdmi_reg_write_u32(HDMI1_ADDR_IO_CFG_HOTPLUG, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI1_ADDR_IO_CFG_SDA, HDMI_IO_CFG_FUNCTION1_SEL);
        hdmi_reg_write_u32(HDMI1_ADDR_IO_CFG_SCL, HDMI_IO_CFG_FUNCTION1_SEL);
    }

    return;
}

td_void drv_hdmi_prod_crg_gate_set(td_u32 id, td_bool enable)
{
    hdmi_if_fpga_return_void();

    hdmi_reg_ctrl_osc_24m_cken_set(id, enable);
    hdmi_reg_ctrl_cec_cken_set(id, enable);
    hdmi_reg_ctrl_os_cken_set(id, enable);
    hdmi_reg_ctrl_as_cken_set(id, enable);
    hdmi_reg_hdmitx_phy_tmds_cken_set(id, enable);
    hdmi_reg_hdmitx_phy_modclk_cken_set(id, enable);
    hdmi_reg_ac_ctrl_modclk_cken_set(id, enable);
    hdmi_reg_phy_clk_pctrl_set(id, TD_FALSE);

    return;
}

td_void drv_hdmi_prod_crg_all_reset_set(td_u32 id, td_bool enable)
{
    hdmi_reg_ctrl_bus_srst_req_set(id, enable);
    hdmi_reg_ctrl_srst_req_set(id, enable);
    hdmi_reg_ctrl_cec_srst_req_set(id, enable);
    hdmi_reg_phy_srst_req_set(id, enable);
    hdmi_reg_phy_bus_srst_req_set(id, enable);
    hdmi_reg_ac_ctrl_srst_req_set(id, enable);
    hdmi_reg_ac_ctrl_bus_srst_req_set(id, enable);
    enable = !enable;
    /*
     * 2, 2us. to ensure ctrl reset success.
     * because internal clock of HDMI is smaller than APB clock.
     */
    udelay(CRG_RESET_DELAY);
    hdmi_reg_ctrl_bus_srst_req_set(id, enable);
    hdmi_reg_ctrl_srst_req_set(id, enable);
    hdmi_reg_ctrl_cec_srst_req_set(id, enable);
    hdmi_reg_phy_srst_req_set(id, enable);
    hdmi_reg_phy_bus_srst_req_set(id, enable);
    hdmi_reg_ac_ctrl_srst_req_set(id, enable);
    hdmi_reg_ac_ctrl_bus_srst_req_set(id, enable);

    return;
}

td_void drv_hdmi_low_power_set(td_u32 id, td_bool enable)
{
    hdmi_if_fpga_return_void();

    enable = !enable;
    hdmi_reg_ctrl_os_cken_set(id, enable);
    hdmi_reg_ctrl_as_cken_set(id, enable);

    return;
}

td_void drv_hdmi_prod_crg_init(td_u32 id)
{
    drv_hdmi_prod_io_cfg_set(id);
    drv_hdmi_prod_crg_gate_set(id, TD_TRUE);
    drv_hdmi_prod_crg_all_reset_set(id, TD_TRUE);
    drv_hdmi_low_power_set(id, TD_TRUE);

    return;
}

td_void drv_hdmi_hardware_reset(td_u32 id)
{
    hdmi_if_fpga_return_void();

    hdmi_reg_crg_init(id);
    /* reset all module */
    hdmi_reg_ctrl_bus_srst_req_set(id, TD_TRUE);
    hdmi_reg_ctrl_srst_req_set(id, TD_TRUE);
    hdmi_reg_ctrl_cec_srst_req_set(id, TD_TRUE);
    hdmi_reg_phy_srst_req_set(id, TD_TRUE);
    hdmi_reg_phy_bus_srst_req_set(id, TD_TRUE);
    hdmi_reg_ac_ctrl_srst_req_set(id, TD_TRUE);
    hdmi_reg_ac_ctrl_bus_srst_req_set(id, TD_TRUE);
    /* close all clk */
    drv_hdmi_prod_crg_gate_set(id, TD_FALSE);
    hdmi_reg_crg_deinit(id);

    return;
}

