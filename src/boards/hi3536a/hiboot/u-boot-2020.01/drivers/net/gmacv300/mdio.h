// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __GMAC_MDIO_H__
#define __GMAC_MDIO_H__

#include <phy.h>
#include "util.h"

#define	REG_MDIO_SINGLE_CMD			0x000003C0
#define	REG_MDIO_SINGLE_DATA		0x000003C4
#define	REG_MDIO_RDATA_STATUS		0x000003D0

/* 0:mdio operation done,1: start mdio operation */
#define	MDIO_CMD					mk_bits(20, 1)

#define	MDIO_WR_DATA				mk_bits(0, 16)
#define	MDIO_RDATA_STATUS			mk_bits(0, 1)

#define	MDIO_CMD_READ				2
#define	MDIO_CMD_WRITE				1

#define mdio_mk_rwctl(rw, phy_exaddr, phy_regnum) \
	(((0x1) << 20) | (((rw) & 0x3) << 16) | \
	(((phy_exaddr) & 0x1f) << 8) | ((phy_regnum) & 0x1f))

#define mdio_start_phyread(ld, phy_addr, regnum) \
	gmac_writel(ld, mdio_mk_rwctl(MDIO_CMD_READ, phy_addr, regnum), \
			REG_MDIO_SINGLE_CMD)

#define mdio_get_phyread_val(ld) (gmac_readl(ld, REG_MDIO_SINGLE_DATA) >> 16)

#define mdio_phywrite(ld, phy_addr, regnum) \
	gmac_writel(ld, mdio_mk_rwctl(MDIO_CMD_WRITE, phy_addr, regnum), \
			REG_MDIO_SINGLE_CMD)

#define test_mdio_ready(ld)     (gmac_readl_bits((ld), \
			REG_MDIO_SINGLE_CMD, MDIO_CMD) == 0)

#define test_mdio_read_data_done(ld)    (gmac_readl_bits(ld, \
			REG_MDIO_RDATA_STATUS, MDIO_RDATA_STATUS) == 0)

int gmac_mdiobus_read(struct mii_dev* const bus, int addr, int devad, int reg);
int gmac_mdiobus_write(struct mii_dev* const bus, int addr, int devad,
		  int reg, unsigned short val);
#endif
