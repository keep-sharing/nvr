// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __HDMI_PRODUCT_DEFINE_H__
#define __HDMI_PRODUCT_DEFINE_H__

#include "ubi_uboot.h"
#include "boot_hdmi_intf.h"
#include "ot_type.h"

/*
 * ------------- reg base addr --------
 *            ctrl    |     dphy      |
 * hdmi0 : 0x17B40000 |  0x17BC0000   |
 * hdmi1 : 0x17B60000 |  0x17BD0000   |
 * -------------------|----------------
 */
#define HDMI_TX_PHY_ADDR           0x17BC0000
#define HDMI_TX_BASE_ADDR          0x17B40000
#define HDMI1_TX_PHY_ADDR          0x17BD0000
#define HDMI1_TX_BASE_ADDR         0x17B60000
#define CRG_BASE_ADDR              0x11010000
#define HDMI0_CRG_OFFSET           0x7F40
#define HDMI0_CRG_ADDR             ((CRG_BASE_ADDR) + (HDMI0_CRG_OFFSET))
#define HDMI1_CRG_OFFSET           0x7F80
#define HDMI1_CRG_ADDR             ((CRG_BASE_ADDR) + (HDMI1_CRG_OFFSET))
/* pin mux */
#define HDMI_ADDR_BASE_IO_CFG      0x17CA0000
#define HDMI_ADDR_IO_CFG_HOTPLUG   (HDMI_ADDR_BASE_IO_CFG + 0x4)
#define HDMI_ADDR_IO_CFG_SDA       (HDMI_ADDR_BASE_IO_CFG + 0x8)
#define HDMI_ADDR_IO_CFG_SCL       (HDMI_ADDR_BASE_IO_CFG + 0xC)
#define HDMI1_ADDR_IO_CFG_HOTPLUG  (HDMI_ADDR_BASE_IO_CFG + 0x10)
#define HDMI1_ADDR_IO_CFG_SDA      (HDMI_ADDR_BASE_IO_CFG + 0x14)
#define HDMI1_ADDR_IO_CFG_SCL      (HDMI_ADDR_BASE_IO_CFG + 0x18)
/* sub-module offset */
#define HDMI_TX_BASE_ADDR_CTRL     0x0000
#define HDMI_TX_BASE_ADDR_VIDEO    0x0800
#define HDMI_TX_BASE_ADDR_AUDIO    0x1000
#define HDMI_TX_BASE_ADDR_HDMITX   0x1800
#define HDMI_TX_BASE_ADDR_AON      0x4000
/* other macro */
#define HDMI_FILE_MODE             0777
#ifdef OT_ADVCA_FUNCTION_RELEASE
#define CONFIG_HDMI_PROC_DISABLE
#define CONFIG_HDMI_DEBUG_DISABLE
#endif

typedef struct {
    td_u32 ssc_bypass_div;
    td_u32 tmds_clk_div;
} hdmi_crg_cfg;

td_s32 hdmi_tx_reg_write(td_u32 *reg_addr, td_u32 value);

td_u32 hdmi_tx_reg_read(const td_u32 *reg_addr);

td_void hdmi_reg_write_u32(td_u32 reg_addr, td_u32 value);

td_u32 hdmi_reg_read_u32(td_u32 reg_addr);

td_void drv_hdmi_prod_io_cfg_set(td_u32 id);

td_void drv_hdmi_prod_crg_all_reset_set(td_u32 id, td_bool enable);

td_void drv_hdmi_prod_crg_gate_set(td_u32 id, td_bool enable);

td_void drv_hdmi_prod_crg_phy_reset_set(td_bool enable);

td_void drv_hdmi_prod_crg_phy_reset_get(td_bool *enable);

td_void drv_hdmi_prod_crg_init(td_u32 id);

td_void drv_hdmi_proc_crg_deinit(td_void);

td_void drv_hdmi_hardware_reset(td_u32 id);

td_void drv_hdmi_low_power_set(td_u32 id, td_bool enable);

#endif  /* __HDMI_PRODUCT_DEFINE_H__ */

