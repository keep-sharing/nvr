// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __ETH_SYS_H__
#define __ETH_SYS_H__

void bspeth_sys_init(void);
void bspeth_sys_exit(void);

void bspeth_set_crg_phy_mode(unsigned char is_rmii_mode);
void set_inner_phy_addr(u32 phyaddr);
void set_efuse_unread(void);

void bspeth_sys_startup(void);
void bspeth_sys_allstop(void);

void set_phy_valtage(void);
#endif
