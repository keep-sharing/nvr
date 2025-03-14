// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <cpu_func.h>
#include <console.h>
#include <securec.h>
#include "gmac.h"
#include "ctrl.h"
#include "mdio.h"

typedef enum {
	GSF0_PORT0,
#if GMAC_AT_LEAST_2PORT
	GSF0_PORT1,
#endif
#if GMAC_AT_LEAST_3PORT
	GSF1_PORT0,
#endif
	MAX_PORT_NUM,
} port_id_t;

static int mac_iobase[MAX_PORT_NUM] = {
	GMAC0_IOBASE,
#if GMAC_AT_LEAST_2PORT
	GMAC1_IOBASE,
#endif
#if GMAC_AT_LEAST_3PORT
	GMAC2_IOBASE,
#endif
};

/* this driver only support GMAC_NUMS = 2 or 1 */
#define MDIO_BUS_NUMS				2
static struct gmac_board_info g_gmac_board_info[CONFIG_GMAC_NUMS] = {
	{
		{
			.index		= GSF0_PORT0,
			.iobase		= GMAC0_IOBASE,
			.iobase_phys	= GMAC0_IOBASE,
		},
		.mii_name		= "mdio0",
		.phy_intf		= CONFIG_GMAC_PHY0_INTERFACE_MODE,
		.phy_addr		= CONFIG_GMAC_PHY0_ADDR,
	},
#if GMAC_AT_LEAST_2PORT
	{
		{
			.index		= GSF0_PORT1,
			.iobase		= GMAC1_IOBASE,
			.iobase_phys	= GMAC1_IOBASE,
		},
		.mii_name		= "mdio1",
		.phy_intf		= CONFIG_GMAC_PHY1_INTERFACE_MODE,
		.phy_addr		= CONFIG_GMAC_PHY1_ADDR,
	}
#endif
};

static int g_phy_link_times;
static int g_gmac_debug;

static char *phy_intf_str[INTERFACE_MODE_BUTT] = {
	"mii",
	"rmii",
	"rgmii",
};
static char *mdio_bus[CONFIG_GMAC_NUMS] = {
	"mdio0",
#if GMAC_AT_LEAST_2PORT
	"mdio1",
#endif
#if GMAC_AT_LEAST_3PORT
	"mdio2"
#endif
};

#if defined(CONFIG_MACH_GODEYES)
/*
 * (godeyes) port mode def.
 * PORT_MODE_BUTT means not support this mode.
 */
static int g_speed_portmode_table[SPEED_MODE_BUTT][INTERFACE_MODE_BUTT] = {
	{PORT_MODE_10_MII,      PORT_MODE_BUTT,         PORT_MODE_10_RGMII},
	{PORT_MODE_100_MII,     PORT_MODE_BUTT,         PORT_MODE_100_RGMII},
	{PORT_MODE_BUTT,        PORT_MODE_BUTT,         PORT_MODE_1000_RGMII}
};
#else
static int g_speed_portmode_table[SPEED_MODE_BUTT][INTERFACE_MODE_BUTT] = {
	{PORT_MODE_10_MII,      PORT_MODE_10_RMII,      PORT_MODE_10_RGMII},
	{PORT_MODE_100_MII,     PORT_MODE_100_RMII,     PORT_MODE_100_RGMII},
	{PORT_MODE_BUTT,        PORT_MODE_BUTT,         PORT_MODE_1000_RGMII}
};
#endif

static int calculate_port_mode(enum speed_mode speed, enum if_mode if_mode,
		  int is_duplex_half)
{
	if (speed < SPEED_MODE_BUTT && if_mode < INTERFACE_MODE_BUTT) {
		int ret = g_speed_portmode_table[speed][if_mode];

		if (is_duplex_half)
			ret = ((unsigned int)ret) & (~bit(4)); /* bit 4 see mac_if reg */
		return ret;
	}

	printf("Invalid speed=%d & interface=%d mode.\n", speed, if_mode);
	printf("Please assign which mode our eth will correctly work at.\n"
			"It may support 10M/100M MII, 10M/100M RMII, "
			"10M/100M/1000M RGMII interface.\n"
			"eg. if your board have two NICs, connecting each phy using "
			"mii and rgmii interface, you can use the module param "
			"'port_mode=mii,rgmii' to tell the driver.\n");
	BUG();

	/* can't reach here */
	return -1;
}

static void set_gmac_debug_switch(void)
{
	char *s = NULL;
	char *e = NULL;
	char evn_buf[ENV_BUF_LEN] = {0};
	int i = 0;

	s = env_get("gmac_debug");
	if (s != NULL) {
		while (*s != '\0' && i < sizeof(evn_buf) - 1)
			evn_buf[i++] = *s++;
		evn_buf[i] = '\0';
		g_gmac_debug = (int)simple_strtoul(evn_buf, &e, 10); /* Base 10 */
	} else {
		g_gmac_debug = 0;
	}
}

static void set_phy_link_times(void)
{
	char *s = NULL;
	char *e = NULL;
	char evn_buf[ENV_BUF_LEN] = {0};
	int i = 0;

	s = env_get("phy_link_times");
	if (s != NULL) {
		while (*s != '\0' && i < sizeof(evn_buf) - 1)
			evn_buf[i++] = *s++;
		evn_buf[i] = '\0';
		g_phy_link_times = (int)simple_strtoul(evn_buf, &e, 10); /* Base 10 */
	} else {
		g_phy_link_times = DEFAULT_PHY_LINK_TIMES;
	}
}

static void set_mdio_intf(void)
{
	int gmac = 0;
	char *s = NULL;
	char evn_buf[ENV_BUF_LEN] = {0};
	int i = 0;

	s = env_get("mdio_intf");
	if (s != NULL) {
		while (*s != '\0' && i < sizeof(evn_buf) - 1)
			evn_buf[i++] = *s++;
		evn_buf[i] = '\0';
		s = evn_buf;
	}
next_mii:
	if (s != NULL) {
		enum if_mode if_mode;

		while (*s == ' ' || *s == ',')
			s++;

		if (strncmp(s, "mii", strlen("mii")) == 0) {
				if_mode = INTERFACE_MODE_MII;
		} else if (strncmp(s, "rmii", strlen("rmii")) == 0) {
				if_mode = INTERFACE_MODE_RMII;
		} else if (strncmp(s, "rgmii", strlen("rgmii")) == 0) {
				if_mode = INTERFACE_MODE_RGMII;
		} else {
			printf("Invalid phy_intf(mii, rmii, or rgmii), " \
					"Set ETH%d default to mii\n", gmac);
			if_mode = INTERFACE_MODE_MII;
		}

		if (gmac == 0) { /* first time */
			gmac = 1;
			g_gmac_board_info[0].phy_intf = if_mode;
			/* in case phy_intf=mii */
#if GMAC_AT_LEAST_2PORT
			g_gmac_board_info[1].phy_intf = if_mode;
#endif
			s = strchr(s, ',');
			goto next_mii;
		} else {
#if GMAC_AT_LEAST_2PORT
			g_gmac_board_info[1].phy_intf = if_mode;
#endif
		}
	}
}

#if GMAC_AT_LEAST_2PORT
static void set_use_mdio(void)
{
	int gmac = 0;
	unsigned long tmp;
	char *s = NULL;
	char *e = NULL;

	/* use_mdio=0 or use_mdio=1 or use_mdio=0,1 or ... */
	s = env_get("use_mdio");
next_mdio:
	if (s && gmac < CONFIG_GMAC_NUMS) {
		while (*s == ' ' || *s == ',')
			s++;

		tmp = simple_strtoul(s, &e, 10); /* Base 10 */
		if (tmp >= MDIO_BUS_NUMS) {
			printf("Invalid use_mdio, Set GMAC%d use mdio%d.\n",
					gmac, gmac);
			tmp = gmac;
		}

		if (gmac == 0) { /* first time */
			g_gmac_board_info[0].mii_name = mdio_bus[tmp];
			g_gmac_board_info[1].mii_name = mdio_bus[tmp];
			gmac++;
			s = e;
			s = strchr(s, ',');
			goto next_mdio;
		} else {
			g_gmac_board_info[1].mii_name = mdio_bus[tmp];
		}
	}
}
#endif

static void set_phy_addr(void)
{
	int gmac = 0;
	unsigned long tmp;
	char *s = NULL;
	char *e = NULL;

	/* get phy addr */
	s = env_get("phy_addr");
next_phyaddr:
	if (s && gmac < CONFIG_GMAC_NUMS) {
		while (*s == ' ' || *s == ',')
			s++;

		tmp = simple_strtoul(s, &e, 10); /* Base 10 */
		if (tmp == 0) {
			printf("Dectected gmac%d phyaddr set to 0, " \
					"is it right?\n", gmac);
		} else if (tmp >= 32) { /* must less than 32 */
			printf("Dectected gmac%d phyaddr " \
					"set to val(%lu) >= 32, " \
					"This may not correct, " \
					"use default phyaddr=%d\n",
					gmac, tmp,
					g_gmac_board_info[gmac].phy_addr);

			tmp = g_gmac_board_info[gmac].phy_addr;
		}
		if (gmac == 0) { /* gmac0 */
			g_gmac_board_info[gmac].phy_addr = tmp;
			gmac++;
			s = e;
			s = strchr(s, ',');
			goto next_phyaddr;
		} else { /* gmac1 */
#if GMAC_AT_LEAST_2PORT
			g_gmac_board_info[gmac].phy_addr = tmp;
#endif
		}
	}
}

/* module parameter
 * 1) phy_intf=mii,rgmii if eth0=mii, eth1=rgmii
 *	phy_intf=mii if eth0=mii, eth1=mii;
 * 2) use_internal_phy=1 if eth1 use internal phy;
 * 3) phy0_addr=1, phy1_addr=2
 * 4) g_gmac_debug=1
 */
static void parse_module_parameters(void)
{
	u32 gmac;

	set_gmac_debug_switch();
	set_phy_link_times();

	/* phy_intf = {mii | rgmii | rmii} [,{mii | rgmii | rmii}] */
	set_mdio_intf();

#if GMAC_AT_LEAST_2PORT
	set_use_mdio();
#endif
	set_phy_addr();
	if (g_gmac_debug) {
		printf("phy_link_times=%d\n", g_phy_link_times);
		for (gmac = 0; gmac < CONFIG_GMAC_NUMS; gmac++)
			printf("ETH%d: port_mode=%s, " \
				"mii_name=%s, phy_addr=%d\n",
				gmac,
				phy_intf_str[g_gmac_board_info[gmac].phy_intf],
				g_gmac_board_info[gmac].mii_name,
				g_gmac_board_info[gmac].phy_addr);
		printf("\n");
	}
}

static int gmac_net_set_mac_address(struct eth_device* const dev)
{
	struct gmac_netdev_local *ld = (struct gmac_netdev_local *)dev->priv;
	unsigned char mac[MAC_LEN] = {0};
	int ret;

	ret = eth_env_get_enetaddr("ethaddr", mac);
	if (ret == 0) {
		printf("MAC address invalid!\n");
#ifdef CONFIG_NET_RANDOM_ETHADDR
		net_random_ethaddr(mac);
		printf("Set Random MAC address!\n");
		eth_setenv_enetaddr("ethaddr", mac);
#endif
	}

	gmac_hw_set_macaddress(ld, mac, MAC_LEN);
	if (memcpy_s(dev->enetaddr, sizeof(dev->enetaddr), mac, MAC_LEN))
		printf("memcpy_s failed!\n");

	return 0;
}

static gmac_desc* gmac_memalign_queue_addr(unsigned int queue_count)
{
	unsigned int size = queue_count * sizeof(gmac_desc);
	gmac_desc *queue_phy_addr = NULL;
	queue_phy_addr = (gmac_desc *)memalign(ARCH_DMA_MINALIGN, size);
	if (queue_phy_addr == NULL)
		return NULL;

	if (memset_s((void *)queue_phy_addr, size, 0, size) < 0)
		printf("memset_s failed!\n");
#ifndef CONFIG_SYS_DCACHE_OFF
	flush_cache((uintptr_t)queue_phy_addr, ALIGN(size, ARCH_DMA_MINALIGN));
#endif
	return queue_phy_addr;
}

static int gmac_init_hw_desc_queue(struct gmac_netdev_local *ld)
{
	gmac_desc *queue_phy_addr = NULL;

	/* init rx fq */
	queue_phy_addr = gmac_memalign_queue_addr(GMAC_HWQ_RX_FQ_DEPTH);
	if (queue_phy_addr == NULL) {
		printf("alloc rx fq error!\n");
		goto _error_alloc_rx_fq;
	}

	ld->rx_fq_addr = queue_phy_addr;
	gmac_set_rx_fq_hwq_addr(ld, (phys_addr_t)(uintptr_t)queue_phy_addr);

	/* init rx bq */
	queue_phy_addr = gmac_memalign_queue_addr(GMAC_HWQ_RX_BQ_DEPTH);
	if (queue_phy_addr == NULL) {
		printf("alloc rx bq error!\n");
		goto _error_alloc_rx_bq;
	}

	ld->rx_bq_addr = queue_phy_addr;
	gmac_set_rx_bq_hwq_addr(ld, (phys_addr_t)(uintptr_t)queue_phy_addr);

	/* init tx bq */
	queue_phy_addr = gmac_memalign_queue_addr(GMAC_HWQ_TX_BQ_DEPTH);
	if (queue_phy_addr == NULL) {
		printf("alloc tx bq error!\n");
		goto _error_alloc_tx_bq;
	}

	ld->tx_bq_addr = queue_phy_addr;
	gmac_set_tx_bq_hwq_addr(ld, (phys_addr_t)(uintptr_t)queue_phy_addr);

	/* init tx rq */
	queue_phy_addr = gmac_memalign_queue_addr(GMAC_HWQ_TX_RQ_DEPTH);
	if (queue_phy_addr == NULL) {
		printf("alloc tx rq error!\n");
		goto _error_alloc_tx_rq;
	}

	ld->tx_rq_addr = queue_phy_addr;
	gmac_set_tx_rq_hwq_addr(ld, (phys_addr_t)(uintptr_t)queue_phy_addr);

	return 0;

_error_alloc_tx_rq:
	free(ld->tx_bq_addr);
	ld->tx_bq_addr = NULL;

_error_alloc_tx_bq:
	free(ld->rx_bq_addr);
	ld->rx_bq_addr = NULL;

_error_alloc_rx_bq:
	free(ld->rx_fq_addr);
	ld->rx_fq_addr = NULL;

_error_alloc_rx_fq:
	return -1;
}

#define PHY_ID_KSZ8051		0x00221550
#define PHY_ID_KSZ8081		0x00221560
#define PHY_ID_KSZ9031		0x00221620
#define PHY_ID_RTL8201		0x001c1100
#ifdef CONFIG_MS_NVR
#define PHY_ID_RTL8211F     	0x001CC910
#endif /* CONFIG_MS_NVR */
#define PHY_ID_MASK		0xFFFFFFF0

/* MMD: MDIO Manageable Device */
#define MACR			0x0D
#define MAADR			0x0E
static void phy_mmd_read(const char *devname, u32 phyaddr,
			 u32 mmd_device, u32 regnum, u32 const *val)
{
	int ret;

	miiphy_write(devname, phyaddr, MACR, mmd_device);
	miiphy_write(devname, phyaddr, MAADR, regnum);
	miiphy_write(devname, phyaddr, MACR, 0x4000 | mmd_device);

	ret = miiphy_read(devname, phyaddr, MAADR, (unsigned short *)val);
	if (ret != 0)
		printf("%s mmd read phy %d dev %d reg 0x%x failed\n",
			devname, phyaddr, mmd_device, regnum);
}

static void phy_mmd_write(const char *devname, u32 phyaddr,
		  u32 mmd_device, u32 regnum, u32 val)
{
	miiphy_write(devname, phyaddr, MACR, mmd_device);
	miiphy_write(devname, phyaddr, MAADR, regnum);
	miiphy_write(devname, phyaddr, MACR, 0x4000 | mmd_device);

	miiphy_write(devname, phyaddr, MAADR, val);
}

static int phy_detected(const char* devname, unsigned int phyaddr, u32  *const phy_id)
{
	u16 id1 = 0;
	u16 id2 = 0;

	if (miiphy_read(devname, phyaddr, MII_PHYSID1, &id1) != 0) {
		printf("PHY IDR1 read failed\n");
		return 0;
	};
	if (miiphy_read(devname, phyaddr, MII_PHYSID2, &id2) != 0) {
		printf("PHY IDR2 read failed\n");
		return 0;
	};

	*phy_id = (id1 & 0xffff) << 16; /* shift 16 bit */
	*phy_id |= (id2 & 0xffff);

	/* If the phy_id is all Fs, there is no device there */
	if (*phy_id == 0xffffffff || *phy_id == 0 ||
	    *phy_id == 0xFFFF || *phy_id == 0xFFFF0000)
		return 0;

	return 1;
}

static int phy_fixup(const char *devname, unsigned int phyaddr, enum if_mode phymode)
{
	u32 phy_id = 0;

	if (phy_detected(devname, phyaddr, &phy_id) == 0)
		return -1;
	/* PHY-KSZ8051 */
	if (((phy_id & PHY_ID_MASK) == PHY_ID_KSZ8051) &&
	    (phymode == INTERFACE_MODE_RMII)) {
		unsigned int val = 0;

		if (miiphy_read(devname, phyaddr, 0x1F,
				(unsigned short *)&val) != 0) {
			printf("PHY reg read failed\n");
			return -1;
		};
		val |= bit(7);       /* set (bit7) phy RMII 50MHz clk; */
		if (miiphy_write(devname, phyaddr, 0x1F, val) != 0)
			return -1;

		if (miiphy_read(devname, phyaddr, 0x16,
				(unsigned short *)&val) != 0) {
			printf("PHY reg read failed\n");
			return -1;
		};
		val |= bit(1);       /* set phy RMII override; */
		if (miiphy_write(devname, phyaddr, 0x16, val) != 0)
			return -1;
	}

	/* PHY-KSZ8081 */
	if (((phy_id & PHY_ID_MASK) == PHY_ID_KSZ8081) &&
	    (phymode == INTERFACE_MODE_RMII)) {
		unsigned int val = 0;

		if (miiphy_read(devname, phyaddr, 0x1F,
				(unsigned short *)&val) != 0) {
			printf("PHY IDR1 read failed\n");
			return -1;
		};
		val |= bit(7);       /* set (bit7) phy RMII 50MHz clk; */
		if (miiphy_write(devname, phyaddr, 0x1F, val) != 0)
			return -1;
	}

	/* PHY-KSZ9031 */
	if ((phy_id & PHY_ID_MASK) == PHY_ID_KSZ9031) {
		unsigned int val = 0;

		/* RX_CLK Pad Skew: 1_1101(+0.84) */
		phy_mmd_read(devname, phyaddr, 0x2, 0x8, &val);
		val = (val & ~0x1F) | 0x1D;
		phy_mmd_write(devname, phyaddr, 0x2, 0x8, val);
	}
	/* PHY-RTL-8201 */
	if ((phy_id & PHY_ID_MASK) == PHY_ID_RTL8201) {
		unsigned int val = 0x07;

		miiphy_write(devname, phyaddr, 0x1F, val);
		miiphy_write(devname, phyaddr, 0x10, 0x1ffa);
	}
#ifdef CONFIG_MS_NVR
	if ((phy_id & PHY_ID_MASK) == PHY_ID_RTL8211F) {
		miiphy_write(devname, phyaddr, 0x1F, 0xd04);
		miiphy_write(devname, phyaddr, 0x10, 0xa371);
		miiphy_write(devname, phyaddr, 0x11, 0x0);
		miiphy_write(devname, phyaddr, 0x1f, 0xa42);
	}
#endif /* CONFIG_MS_NVR */
	return 0;
}

static void gmac_net_adjust_speed(struct gmac_netdev_local *ld,
		  unsigned int *link_stat, int is_duplex_half)
{
	char *mii_name = g_gmac_board_info[ld->index].mii_name;
	int phy_addr = g_gmac_board_info[ld->index].phy_addr;
	int speed, port_mode;
	unsigned int stat = *link_stat;
	enum speed_mode speed_mode = SPEED_MODE_100M;

	speed = miiphy_speed(mii_name, phy_addr);
	switch (speed) {
	case _10BASET:
		stat |= GMAC_SPD_10M;
		speed_mode = SPEED_MODE_10M;
		break;
	case _100BASET:
		stat |= GMAC_SPD_100M;
		speed_mode = SPEED_MODE_100M;
		break;
	case _1000BASET:
		stat |= GMAC_SPD_1000M;
		speed_mode = SPEED_MODE_1000M;
		break;
	default:
		printf("wired, phy speed!\n");
		break;
	}

	if (ld->link_stat != stat) {
		if (stat & GMAC_LINKED) {
			port_mode = calculate_port_mode(speed_mode,
				g_gmac_board_info[ld->index].phy_intf, is_duplex_half);
			if (is_duplex_half == 0)
				is_duplex_half = 0x1;
			else
				is_duplex_half = 0x0;

			gmac_set_macif(ld, port_mode, speed_mode);

			gmac_writel(ld, is_duplex_half,
				MAC_DUPLEX_HALF_CTRL);
		}
		ld->link_stat = stat;
	}
	*link_stat = stat;
}

static int gmac_net_adjust_link(struct gmac_netdev_local *ld)
{
	char *mii_name = g_gmac_board_info[ld->index].mii_name;
	int phy_addr = g_gmac_board_info[ld->index].phy_addr;
	enum if_mode phy_mode = g_gmac_board_info[ld->index].phy_intf;
	unsigned int stat = INVALID_LINK_STATUS;
	int phy_duplex;
	int is_duplex_half = 1;
	u32 phy_id = 0;

	if (phy_addr == INVALID_PHY_ADDR)
		return stat;

	phy_fixup(mii_name, phy_addr, phy_mode);

	if (phy_detected(mii_name, phy_addr, &phy_id) == 0)
		return stat;

	stat |= miiphy_link(mii_name, phy_addr) ? GMAC_LINKED : 0;

	phy_duplex = miiphy_duplex(mii_name, phy_addr);
	if (phy_duplex == FULL) {
		stat |= GMAC_DUP_FULL;
		is_duplex_half = 0;
	}

	gmac_net_adjust_speed(ld, &stat, is_duplex_half);

	return stat;
}

static int select_current_linked_phy(struct gmac_netdev_local *ld)
{
	int count = g_phy_link_times ? : DEFAULT_PHY_LINK_TIMES;
	unsigned int status;

	for (; count > 0; count--) {
		if (ctrlc()) {
			puts("\nAbort\n");
			goto link_failed;
		}

		status = (unsigned int)gmac_net_adjust_link(ld);
		if (status & GMAC_LINKED)
			goto link_on;
		mdelay(100); /* wait 100ms */
	}

link_failed:
	printf("ETH%d: PHY(%s, phyaddr=%d, %s) not link!\n",
			ld->index, g_gmac_board_info[ld->index].mii_name,
			g_gmac_board_info[ld->index].phy_addr,
			phy_intf_str[g_gmac_board_info[ld->index].phy_intf]);

	return -1;

link_on:
	printf("ETH%d: PHY(phyaddr=%d, %s) link UP: DUPLEX=%s : SPEED=%s\n",
			ld->index, g_gmac_board_info[ld->index].phy_addr,
			phy_intf_str[g_gmac_board_info[ld->index].phy_intf],
			(status & GMAC_DUP_FULL) ? "FULL" : "HALF",
			(status & GMAC_SPD_100M) ? "100M" :
			((status & GMAC_SPD_10M) ? "10M" : "1000M"));

	miiphy_set_current_dev(g_gmac_board_info[ld->index].mii_name);

	return 0;
}

static unsigned int get_wr_rd_dist(u32 rx_fq_wr_offset, u32 rx_fq_rd_offset)
{
	unsigned int wr_rd_dist;
	if (rx_fq_wr_offset >= rx_fq_rd_offset)
		wr_rd_dist = (GMAC_HWQ_RX_FQ_DEPTH << DESC_BYTE_SHIFT)
			- (rx_fq_wr_offset - rx_fq_rd_offset);
	else
		wr_rd_dist = rx_fq_rd_offset - rx_fq_wr_offset;

	/* offset was counted on bytes, desc size = 2^5 */
	wr_rd_dist >>= DESC_BYTE_SHIFT;
	return wr_rd_dist;
}

static void show_gmac_recv_msg(const char *addr, int len)
{
	if (g_gmac_debug) {
		int i;
		printf("got packet!\n");
		printf("[%s:%d]Packet length = %#4x\n", __FUNCTION__, __LINE__, len);
		for (i = 0; i < len; i++) {
			if (i % 16 == 0) printf("%#4.4x", i); /* Each 16 print i */
			if (i % 2 == 0) printf(" "); /* Each 2 print space */
			printf("%2.2x", ((unsigned char *)addr)[i]);
			if (i % 16 == 15) printf("\n"); /* 15 is last line of 16 */
		}
		printf("\n\n\n\n");
	}
}

#define NET_IP_ALIGN	2
static void gmac_recv_rx_fq(struct gmac_netdev_local const *ld)
{
	gmac_desc *rx_fq_desc = ld->rx_fq_addr;
	u32 rx_fq_wr_offset, rx_fq_rd_offset;
	unsigned int wr_rd_dist;
	phys_addr_t addr;
	int i;

	rx_fq_wr_offset = gmac_readl_bits(ld, RX_FQ_WR_ADDR, BITS_RX_FQ_WR_ADDR);
	rx_fq_rd_offset = gmac_readl_bits(ld, RX_FQ_RD_ADDR, BITS_RX_FQ_RD_ADDR);

	wr_rd_dist = get_wr_rd_dist(rx_fq_wr_offset, rx_fq_rd_offset);

	/* wr_rd_dist - 1 for logic reason.
	 * Logic think the desc pool is full filled, ...?
	 */
	for (i = 0; i < wr_rd_dist - 1; i++) {
		rx_fq_desc = ld->rx_fq_addr + (rx_fq_wr_offset >> DESC_BYTE_SHIFT);
		addr = (phys_addr_t)(uintptr_t)memalign(ARCH_DMA_MINALIGN,
					ETH_BUFFER_SIZE);
		if (addr == 0)
			break;
#ifndef CONFIG_SYS_DCACHE_OFF
		invalidate_dcache_range(addr,
			ALIGN(addr + ETH_BUFFER_SIZE, ARCH_DMA_MINALIGN));
#endif
		addr += NET_IP_ALIGN;
		rx_fq_desc->data_buff_addr = (u32)addr;
#if defined(CONFIG_64BIT)
		rx_fq_desc->reserve4 = ((u64)addr >> REG_BIT_WIDTH) << Q_ADDR_HI8_OFFSET;
#endif
		rx_fq_desc->descvid = DESC_VLD_FREE;
		rx_fq_desc->buffer_len = (ETH_MAX_FRAME_SIZE - 1);
		rx_fq_desc->data_len = 8; /* data length 8 */
#ifndef CONFIG_SYS_DCACHE_OFF
		flush_cache((uintptr_t)rx_fq_desc & ~(ARCH_DMA_MINALIGN - 1),
			ALIGN(sizeof(*rx_fq_desc), ARCH_DMA_MINALIGN));
#endif

		rx_fq_wr_offset += DESC_SIZE;
		if (rx_fq_wr_offset >= (GMAC_HWQ_RX_FQ_DEPTH << DESC_BYTE_SHIFT))
			rx_fq_wr_offset = 0;
		gmac_writel_bits(ld, rx_fq_wr_offset, RX_FQ_WR_ADDR,
			BITS_RX_FQ_WR_ADDR);
	}
}

static int gmac_recv(struct eth_device *dev)
{
	struct gmac_netdev_local *ld = (struct gmac_netdev_local *)dev->priv;
	gmac_desc *rx_bq_desc = ld->rx_bq_addr;
	u32 rx_bq_wr_offset, rx_bq_rd_offset;
	int timeout_us = 100000;
	phys_addr_t addr;
	int len;

	gmac_recv_rx_fq(ld);

	rx_bq_wr_offset = gmac_readl_bits(ld, RX_BQ_WR_ADDR, BITS_RX_BQ_WR_ADDR);
	rx_bq_rd_offset = gmac_readl_bits(ld, RX_BQ_RD_ADDR, BITS_RX_BQ_RD_ADDR);
	rx_bq_desc += (rx_bq_rd_offset >> DESC_BYTE_SHIFT);

	while (--timeout_us && (rx_bq_wr_offset == rx_bq_rd_offset)) {
		udelay(1);
		rx_bq_wr_offset = gmac_readl_bits(ld, RX_BQ_WR_ADDR, BITS_RX_BQ_WR_ADDR);
	}
	if (timeout_us == 0)
		return -1;

	rx_bq_rd_offset += DESC_SIZE;
	if (rx_bq_rd_offset >= (GMAC_HWQ_RX_BQ_DEPTH << DESC_BYTE_SHIFT))
		rx_bq_rd_offset = 0;

#ifndef CONFIG_SYS_DCACHE_OFF
	invalidate_dcache_range((uintptr_t)rx_bq_desc & ~(ARCH_DMA_MINALIGN - 1),
		ALIGN((uintptr_t)rx_bq_desc + sizeof(*rx_bq_desc), ARCH_DMA_MINALIGN));
#endif
	addr = rx_bq_desc->data_buff_addr;
#if defined(CONFIG_64BIT)
	addr |= (u64)(rx_bq_desc->reserve4 >> Q_ADDR_HI8_OFFSET) << REG_BIT_WIDTH;
#endif
	len = rx_bq_desc->data_len;
	if (gmac_invalid_rxpkg_len(len)) {
		gmac_writel_bits(ld, rx_bq_rd_offset, RX_BQ_RD_ADDR, BITS_RX_BQ_RD_ADDR);
		addr -= NET_IP_ALIGN;
		free((void *)(uintptr_t)addr);
		addr = 0;
		return -1;
	}

	show_gmac_recv_msg((char *)(uintptr_t)addr, len);

#ifndef CONFIG_SYS_DCACHE_OFF
	invalidate_dcache_range(addr - NET_IP_ALIGN,
		ALIGN((unsigned int)(addr + len), ARCH_DMA_MINALIGN));
#endif
	if (memcpy_s((void *)net_rx_packets[0], len, (const void *)(uintptr_t)addr, len))
		printf("memcpy_s failed!\n");

	addr -= NET_IP_ALIGN;
	free((void *)(uintptr_t)addr);
	addr = 0;

	gmac_writel_bits(ld, rx_bq_rd_offset, RX_BQ_RD_ADDR, BITS_RX_BQ_RD_ADDR);

	/* NetRecive */
	net_process_received_packet(net_rx_packets[0], len);

	return 0;
}

static int gmac_wait_tx_rq_wr(struct gmac_netdev_local const *ld)
{
	u32 tx_rq_wr_offset;
	u32 tx_rq_rd_offset;
	int timeout_us = 1000;

	tx_rq_wr_offset = gmac_readl_bits(ld,
			TX_RQ_WR_ADDR, BITS_TX_RQ_WR_ADDR);
	tx_rq_rd_offset = gmac_readl_bits(ld,
			TX_RQ_RD_ADDR, BITS_TX_RQ_RD_ADDR);

	tx_rq_rd_offset += DESC_SIZE;
	if (tx_rq_rd_offset >= (GMAC_HWQ_TX_RQ_DEPTH << DESC_BYTE_SHIFT))
		tx_rq_rd_offset = 0;

	while (--timeout_us && (tx_rq_rd_offset != tx_rq_wr_offset)) {
		udelay(1);
		tx_rq_wr_offset = gmac_readl_bits(ld,
				TX_RQ_WR_ADDR, BITS_TX_RQ_WR_ADDR);
	}

	if (timeout_us == 0)
		return -1;

	gmac_writel_bits(ld, tx_rq_rd_offset, TX_RQ_RD_ADDR, BITS_TX_RQ_RD_ADDR);
	if (g_gmac_debug)
		printf("send packet!\n");

	return 0;
}

static int gmac_send(struct eth_device *dev, void *packet, int length)
{
	struct gmac_netdev_local *ld = (struct gmac_netdev_local *)dev->priv;
	u32 tx_bq_wr_offset, tx_bq_rd_offset;
	unsigned int tso_ver;
	gmac_desc *tx_bq_desc = ld->tx_bq_addr;
	phys_addr_t addr;

	tx_bq_wr_offset = gmac_readl_bits(ld, TX_BQ_WR_ADDR, BITS_TX_BQ_WR_ADDR);
	tx_bq_rd_offset = gmac_readl_bits(ld, TX_BQ_RD_ADDR, BITS_TX_BQ_RD_ADDR);
	if (tx_bq_rd_offset != tx_bq_wr_offset) {
		gmac_writel_bits(ld, tx_bq_rd_offset,
				TX_BQ_WR_ADDR, BITS_TX_BQ_WR_ADDR);
		return -1;
	}

	tso_ver = gmac_readl_bits(ld, CRF_MIN_PACKET, BIT_TSO_VERSION);

	tx_bq_desc += (tx_bq_wr_offset >> DESC_BYTE_SHIFT);
	addr = (phys_addr_t)(uintptr_t)packet;
	tx_bq_desc->data_buff_addr = (u32)addr;
#if defined(CONFIG_64BIT)
	tx_bq_desc->reserve3 = ((u64)addr >> REG_BIT_WIDTH) & TX_DESC_HI8_MASK;
#endif
	tx_bq_desc->descvid = DESC_VLD_BUSY;

	tx_bq_desc->fl = tso_ver ? 0 : DESC_FL_FULL;

	tx_bq_desc->reserve1 = 0;
	tx_bq_desc->data_len = length;
	tx_bq_desc->buffer_len = ETH_MAX_FRAME_SIZE - 1;
#ifndef CONFIG_SYS_DCACHE_OFF
	flush_cache((uintptr_t)packet, ALIGN((u32)length, ARCH_DMA_MINALIGN));
	flush_cache((uintptr_t)tx_bq_desc & ~(ARCH_DMA_MINALIGN - 1),
		ALIGN(sizeof(*tx_bq_desc), ARCH_DMA_MINALIGN));
#endif

	tx_bq_wr_offset += DESC_SIZE;
	if (tx_bq_wr_offset >= (GMAC_HWQ_TX_BQ_DEPTH << DESC_BYTE_SHIFT))
		tx_bq_wr_offset = 0;

	gmac_writel_bits(ld, tx_bq_wr_offset,
			TX_BQ_WR_ADDR, BITS_TX_BQ_WR_ADDR);

	return gmac_wait_tx_rq_wr(ld);
}

#ifdef CFG_NET_PINCTRL
static void gmac_config_rgmii0(void)
{
	writel(VALUE_MDCK0, PHY_ADDR_MDCK0);
	writel(VALUE_MDIO0, PHY_ADDR_MDIO0);
	writel(VALUE_EPHY0_CLK, PHY_ADDR_EPHY0_CLK);
	writel(VALUE_EPHY0_RSTN, PHY_ADDR_EPHY0_RSTN);
	writel(VALUE_RGMII0_TXCKOUT, PHY_ADDR_RGMII0_TXCKOUT);
	writel(VALUE_RGMII0_TXD0, PHY_ADDR_RGMII0_TXD0);
	writel(VALUE_RGMII0_TXD1, PHY_ADDR_RGMII0_TXD1);
	writel(VALUE_RGMII0_TXD2, PHY_ADDR_RGMII0_TXD2);
	writel(VALUE_RGMII0_TXD3, PHY_ADDR_RGMII0_TXD3);
	writel(VALUE_RGMII0_TXEN, PHY_ADDR_RGMII0_TXEN);
	writel(VALUE_RGMII0_RXCK, PHY_ADDR_RGMII0_RXCK);
	writel(VALUE_RGMII0_RXD0, PHY_ADDR_RGMII0_RXD0);
	writel(VALUE_RGMII0_RXD1, PHY_ADDR_RGMII0_RXD1);
	writel(VALUE_RGMII0_RXD2, PHY_ADDR_RGMII0_RXD2);
	writel(VALUE_RGMII0_RXD3, PHY_ADDR_RGMII0_RXD3);
	writel(VALUE_RGMII0_RXDV, PHY_ADDR_RGMII0_RXDV);
}

static void gmac_config_rmii0(void)
{
	writel(VALUE_MDCK0, PHY_ADDR_MDCK0);
	writel(VALUE_MDIO0, PHY_ADDR_MDIO0);
	writel(VALUE_EPHY0_CLK, PHY_ADDR_EPHY0_CLK);
	writel(VALUE_EPHY0_RSTN, PHY_ADDR_EPHY0_RSTN);
	writel(VALUE_RMII0_CLK, PHY_ADDR_RMII0_CLK);
	writel(VALUE_RMII0_TXD0, PHY_ADDR_RMII0_TXD0);
	writel(VALUE_RMII0_TXD1, PHY_ADDR_RMII0_TXD1);
	writel(VALUE_RMII0_TXEN, PHY_ADDR_RMII0_TXEN);
	writel(VALUE_RMII0_RXD0, PHY_ADDR_RMII0_RXD0);
	writel(VALUE_RMII0_RXD1, PHY_ADDR_RMII0_RXD1);
	writel(VALUE_RMII0_RXDV, PHY_ADDR_RMII0_RXDV);
}

#if GMAC_AT_LEAST_2PORT
static void gmac_config_rgmii1(void)
{
	writel(VALUE_MDCK1, PHY_ADDR_MDCK1);
	writel(VALUE_MDIO1, PHY_ADDR_MDIO1);
	writel(VALUE_EPHY1_CLK, PHY_ADDR_EPHY1_CLK);
	writel(VALUE_EPHY1_RSTN, PHY_ADDR_EPHY1_RSTN);
	writel(VALUE_RGMII1_TXCKOUT, PHY_ADDR_RGMII1_TXCKOUT);
	writel(VALUE_RGMII1_TXD0, PHY_ADDR_RGMII1_TXD0);
	writel(VALUE_RGMII1_TXD1, PHY_ADDR_RGMII1_TXD1);
	writel(VALUE_RGMII1_TXD2, PHY_ADDR_RGMII1_TXD2);
	writel(VALUE_RGMII1_TXD3, PHY_ADDR_RGMII1_TXD3);
	writel(VALUE_RGMII1_TXEN, PHY_ADDR_RGMII1_TXEN);
	writel(VALUE_RGMII1_RXCK, PHY_ADDR_RGMII1_RXCK);
	writel(VALUE_RGMII1_RXD0, PHY_ADDR_RGMII1_RXD0);
	writel(VALUE_RGMII1_RXD1, PHY_ADDR_RGMII1_RXD1);
	writel(VALUE_RGMII1_RXD2, PHY_ADDR_RGMII1_RXD2);
	writel(VALUE_RGMII1_RXD3, PHY_ADDR_RGMII1_RXD3);
	writel(VALUE_RGMII1_RXDV, PHY_ADDR_RGMII1_RXDV);
}

static void gmac_config_rmii1(void)
{
	writel(VALUE_MDCK1, PHY_ADDR_MDCK1);
	writel(VALUE_MDIO1, PHY_ADDR_MDIO1);
	writel(VALUE_EPHY1_CLK, PHY_ADDR_EPHY1_CLK);
	writel(VALUE_EPHY1_RSTN, PHY_ADDR_EPHY1_RSTN);
	writel(VALUE_RMII1_CLK, PHY_ADDR_RMII1_CLK);
	writel(VALUE_RMII1_TXD0, PHY_ADDR_RMII1_TXD0);
	writel(VALUE_RMII1_TXD1, PHY_ADDR_RMII1_TXD1);
	writel(VALUE_RMII1_TXEN, PHY_ADDR_RMII1_TXEN);
	writel(VALUE_RMII1_RXD0, PHY_ADDR_RMII1_RXD0);
	writel(VALUE_RMII1_RXD1, PHY_ADDR_RMII1_RXD1);
	writel(VALUE_RMII1_RXDV, PHY_ADDR_RMII1_RXDV);
}
#endif

static void gmac_pinctrl_config(void)
{
	if (g_gmac_board_info[0].phy_intf == INTERFACE_MODE_RMII) // 0:mac0
		gmac_config_rmii0();
	else if (g_gmac_board_info[0].phy_intf == INTERFACE_MODE_RGMII) // 0:mac0
		gmac_config_rgmii0();

#if GMAC_AT_LEAST_2PORT
	if (g_gmac_board_info[1].phy_intf == INTERFACE_MODE_RMII) // 1:mac1
		gmac_config_rmii1();
	else if (g_gmac_board_info[1].phy_intf == INTERFACE_MODE_RGMII) // 1:mac1
		gmac_config_rgmii1();
#endif
}
#endif

#if defined(CONFIG_TARGET_SS626V100)
static void gmac0_srst_init(void)
{
	uintptr_t p;
	volatile unsigned int v;

	p = (unsigned long)(CRG_REG_BASE);
	/* gmac0_if srst init */
	v = readl(p + REG_ETH0_MACIF_CRG);
	v &= ~BIT_MACIF0_RST;
	writel(v, p + REG_ETH0_MACIF_CRG);

	/* gmac0 srst init */
	v = readl(p + REG_ETH0_GSF_CRG);
	v &= ~BIT_GMAC0_RST;
	writel(v, p + REG_ETH0_GSF_CRG);

	/* phy0 srst init */
	v = readl(p + REG_ETH0_PHY_CRG);
	v &= ~BIT_EXT_PHY0_RST;
	writel(v, p + REG_ETH0_PHY_CRG);
}
#if GMAC_AT_LEAST_2PORT
static void gmac1_srst_init(void)
{
	uintptr_t p;
	volatile unsigned int v;

	p = (unsigned long)(CRG_REG_BASE);
	/* gmac1_if srst init */
	v = readl(p + REG_ETH1_MACIF_CRG);
	v &= ~BIT_MACIF0_RST;
	writel(v, p + REG_ETH1_MACIF_CRG);

	/* gmac1 srst init */
	v = readl(p + REG_ETH1_GSF_CRG);
	v &= ~BIT_GMAC0_RST;
	writel(v, p + REG_ETH1_GSF_CRG);

	/* phy1 srst init */
	v = readl(p + REG_ETH1_PHY_CRG);
	v &= ~BIT_EXT_PHY0_RST;
	writel(v, p + REG_ETH1_PHY_CRG);
}
#endif
#endif

static int g_gmac_hw_inited = 0;
static void gmac_hw_init(void)
{
	/* init once to save time */
	if (g_gmac_hw_inited == 0) {
		gmac_sys_init();

		g_gmac_hw_inited = 1;
	}
}

static int gmac_init(struct eth_device *dev, bd_t *bd)
{
	struct gmac_netdev_local *ld = (struct gmac_netdev_local *)dev->priv;
	char *mii_devname = g_gmac_board_info[ld->index].mii_name;
	int phy_addr = g_gmac_board_info[ld->index].phy_addr;
	enum if_mode phy_intf = g_gmac_board_info[ld->index].phy_intf;
	u32 phy_id = 0;
	int ret;

	gmac_hw_init();

	/* init once to save time */
	if (ld->initalized == 0) {
		if (!phy_detected(mii_devname, phy_addr, &phy_id))
			return -1;

		miiphy_reset(mii_devname, phy_addr);

		if (phy_intf != INTERFACE_MODE_RGMII) {
			unsigned short val = 0;
			if (miiphy_read(mii_devname, phy_addr, MII_CTRL1000, &val) == 0) {
				val &= ~(PHY_1000BTCR_1000FD | PHY_1000BTCR_1000HD);
				miiphy_write(mii_devname, phy_addr, MII_CTRL1000, val);
			}
		}

		gmac_glb_preinit_dummy(ld);

		ret = gmac_set_hwq_depth(ld);
		if (ret) {
			printf("init eth%d hw desc queue depth fail!\n", ld->index);
			return ret;
		}

		ret = gmac_init_hw_desc_queue(ld);
		if (ret) {
			printf("init eth%d hw desc queue fail!\n", ld->index);
			return ret;
		}
		ld->initalized = 1;
	}

	ret = select_current_linked_phy(ld);
	if (ret)
		return ret;

	gmac_net_set_mac_address(dev);
	gmac_desc_enable(ld, RX_OUTCFF_WR_DESC_ENA | RX_CFF_RD_DESC_ENA);
	gmac_writel_bits(ld, 1, PORT_EN, BITS_RX_EN);

	gmac_writel_bits(ld, 1, PORT_EN, BITS_TX_EN);
	gmac_desc_enable(ld, TX_OUTCFF_WR_DESC_ENA | TX_CFF_RD_DESC_ENA);

	return 0;
}

static void gmac_halt(struct eth_device *dev)
{
	struct gmac_netdev_local *ld = (struct gmac_netdev_local *)dev->priv;

	gmac_writel_bits(ld, 0, PORT_EN, BITS_RX_EN);
	gmac_desc_disable(ld, RX_OUTCFF_WR_DESC_ENA | RX_CFF_RD_DESC_ENA);

	gmac_desc_disable(ld, TX_OUTCFF_WR_DESC_ENA | TX_CFF_RD_DESC_ENA);
	gmac_writel_bits(ld, 0, PORT_EN, BITS_TX_EN);
}

static int gmac_register_dev(port_id_t port_id)
{
	struct eth_device *dev;

	dev = malloc(sizeof(*dev));
	if (dev == NULL)
		return -1;
	if (memset_s(dev, sizeof(*dev), 0, sizeof(*dev)) < 0)
		printf("memset_s failed!\n");

	dev->iobase = mac_iobase[port_id];
	dev->init = gmac_init;
	dev->halt = gmac_halt;
	dev->send = gmac_send;
	dev->recv = gmac_recv;
	dev->priv = &g_gmac_board_info[port_id].gmac_ld;
	if (snprintf_s(dev->name, sizeof(dev->name), sizeof(dev->name) - 1, "gmac%d", port_id) < 0)
		printf("snprintf_s failed!func:%s, line: %d\n", __func__, __LINE__);
	eth_register(dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		free(dev);
		debug("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = gmac_mdiobus_read;
	bus->write = gmac_mdiobus_write;
	if (snprintf_s(bus->name, sizeof(bus->name), sizeof(bus->name), mdio_bus[port_id]) < 0)
		printf("snprintf_s failed!func:%s, line: %d\n", __func__, __LINE__);
	if (mdio_register(bus)) {
		mdio_free(bus);
		free(dev);
		return -1;
	}
#endif

	return 0;
}

struct gmac_board_info *get_gmac_bd_info(unsigned int index)
{
	return g_gmac_board_info + index;
}

int gmac_initialize(bd_t const *bis)
{
	int ret;

	parse_module_parameters();

#ifdef CFG_NET_PINCTRL
	/* Configure gmac pinctrl parameters in software */
	gmac_pinctrl_config();
#endif

#if defined(CONFIG_TARGET_SS626V100)
	gmac0_srst_init();
#if GMAC_AT_LEAST_2PORT
	gmac1_srst_init();
#endif
#endif

	ret = gmac_register_dev(GSF0_PORT0);
	if (ret)
		return ret;

#if GMAC_AT_LEAST_2PORT
	ret = gmac_register_dev(GSF0_PORT1);
	if (ret)
		return ret;
#endif

#if GMAC_AT_LEAST_3PORT
	ret = gmac_register_dev(GSF1_PORT0);
	if (ret)
		return ret;
#endif

	return 0;
}
