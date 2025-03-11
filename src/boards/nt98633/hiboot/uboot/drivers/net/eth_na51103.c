/*
 * Copyright (c) 2021, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * Portions based on U-Boot's rtl8169.c. dwc_eth_qos.c
 */

/*
 * This driver supports the Synopsys Designware Ethernet QOS (Quality Of
 * Service) IP block. The IP supports multiple options for bus type, clocking/
 * reset structure, and feature list.
 *
 * The driver is written such that generic core logic is kept separate from
 * configuration-specific logic. Code that interacts with configuration-
 * specific resources is split out into separate functions to avoid polluting
 * common code. If/when this driver is enhanced to support multiple
 * configurations, the core code should be adapted to call all configuration-
 * specific functions through function pointers, with the definition of those
 * function pointers being supplied by struct udevice_id eqos_ids[]'s .data
 * field.
 *
 * The following configurations are currently supported:
 * na51103:
 *    NOVATEK's NA51103 chip. This configuration uses an AXI master/DMA bus, an
 *    AHB slave/register bus, contains the DMA, MTL, and MAC sub-blocks, and
 *    supports a single RGMII PHY. This configuration also has SW control over
 *    all clock and reset signals to the HW block.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <asm/io.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <linux/libfdt.h>

#define ETHVERSION "1.0.0.3"

#define SW_RESET_ADDRESS     (0x3800)
#define SW_RESET_MASK        0x00000002
#define PLLREGWR_MASK        0x00000008
#define REG32_MASK          0x00000000FFFFFFFF

#define phy_sw_reset_enable(base_addr) do { \
        unsigned long v; \
        v = readl((void *)((base_addr)+SW_RESET_ADDRESS)); \
        v |= SW_RESET_MASK; \
        writel(v, (void *)((base_addr)+SW_RESET_ADDRESS));\
} while(0)

#define phy_sw_reset_disable(base_addr) do { \
        unsigned long v; \
        v = readl((void *)((base_addr)+SW_RESET_ADDRESS)); \
        v |= PLLREGWR_MASK; \
        v &= ~(SW_RESET_MASK); \
        writel(v, (void *)((base_addr)+SW_RESET_ADDRESS));\
} while(0)

#define set_break_link_timer(base_addr) do { \
        writel(0x53, (void *)((base_addr) + 0x3A88));\
        writel(0x07, (void *)((base_addr) + 0x3A8C));\
        writel(0xC9, (void *)((base_addr) + 0x3A84));\
} while(0)

#define set_led_blinking(base_addr) do { \
        writel(0x40, (void *)((base_addr) + 0x3900));\
        writel(0x312, (void *)((base_addr) + 0x300C));\
} while(0)

#define set_led_default(base_addr) do { \
        writel(0x3FF, (void *)((base_addr) + 0x300C));\
} while (0)

/* Core registers */
#define DEFAULT_MAC_ADDRESS {0x00, 0x80, 0x48, 0xBA, 0xD1, 0x30}

#define EQOS_MAC_REGS_BASE 0x000
struct eqos_mac_regs {
	uint32_t configuration;                         /* 0x000 */
	uint32_t unused_004[(0x070 - 0x004) / 4];       /* 0x004 */
	uint32_t q0_tx_flow_ctrl;                       /* 0x070 */
	uint32_t unused_070[(0x090 - 0x074) / 4];       /* 0x074 */
	uint32_t rx_flow_ctrl;                          /* 0x090 */
	uint32_t unused_094;                            /* 0x094 */
	uint32_t txq_prty_map0;                         /* 0x098 */
	uint32_t unused_09c;                            /* 0x09c */
	uint32_t rxq_ctrl0;                             /* 0x0a0 */
	uint32_t unused_0a4;                            /* 0x0a4 */
	uint32_t rxq_ctrl2;                             /* 0x0a8 */
	uint32_t unused_0ac[(0x0dc - 0x0ac) / 4];       /* 0x0ac */
	uint32_t us_tic_counter;                        /* 0x0dc */
	uint32_t unused_0e0[(0x11c - 0x0e0) / 4];       /* 0x0e0 */
	uint32_t hw_feature0;                           /* 0x11c */
	uint32_t hw_feature1;                           /* 0x120 */
	uint32_t hw_feature2;                           /* 0x124 */
	uint32_t unused_128[(0x200 - 0x128) / 4];       /* 0x128 */
	uint32_t mdio_address;                          /* 0x200 */
	uint32_t mdio_data;                             /* 0x204 */
	uint32_t unused_208[(0x300 - 0x208) / 4];       /* 0x208 */
	uint32_t address0_high;                         /* 0x300 */
	uint32_t address0_low;                          /* 0x304 */
};

#define EQOS_MAC_CONFIGURATION_GPSLCE                   BIT(23)
#define EQOS_MAC_CONFIGURATION_CST                      BIT(21)
#define EQOS_MAC_CONFIGURATION_ACS                      BIT(20)
#define EQOS_MAC_CONFIGURATION_WD                       BIT(19)
#define EQOS_MAC_CONFIGURATION_JD                       BIT(17)
#define EQOS_MAC_CONFIGURATION_JE                       BIT(16)
#define EQOS_MAC_CONFIGURATION_PS                       BIT(15)
#define EQOS_MAC_CONFIGURATION_FES                      BIT(14)
#define EQOS_MAC_CONFIGURATION_DM                       BIT(13)
#define EQOS_MAC_CONFIGURATION_TE                       BIT(1)
#define EQOS_MAC_CONFIGURATION_RE                       BIT(0)

#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT               16
#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_MASK                0xffff
#define EQOS_MAC_Q0_TX_FLOW_CTRL_TFE                    BIT(1)

#define EQOS_MAC_RX_FLOW_CTRL_RFE                       BIT(0)

#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT              0
#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK               0xff

#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT                 0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK                  3
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_NOT_ENABLED           0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB           2

#define EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT                  0
#define EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK                   0xff

#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT           6
#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK            0x1f
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT           0
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK            0x1f

#define EQOS_MAC_MDIO_ADDRESS_PA_SHIFT                  21
#define EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT                 16
#define EQOS_MAC_MDIO_ADDRESS_CR_SHIFT                  8
#define EQOS_MAC_MDIO_ADDRESS_CR_20_35                  2
#define EQOS_MAC_MDIO_ADDRESS_SKAP                      BIT(4)
#define EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT                 2
#define EQOS_MAC_MDIO_ADDRESS_GOC_READ                  3
#define EQOS_MAC_MDIO_ADDRESS_GOC_WRITE                 1
#define EQOS_MAC_MDIO_ADDRESS_C45E                      BIT(1)
#define EQOS_MAC_MDIO_ADDRESS_GB                        BIT(0)

#define EQOS_MAC_MDIO_DATA_GD_MASK                      0xffff

#define EQOS_MTL_REGS_BASE 0xd00
struct eqos_mtl_regs {
	uint32_t txq0_operation_mode;                   /* 0xd00 */
	uint32_t unused_d04;                            /* 0xd04 */
	uint32_t txq0_debug;                            /* 0xd08 */
	uint32_t unused_d0c[(0xd18 - 0xd0c) / 4];       /* 0xd0c */
	uint32_t txq0_quantum_weight;                   /* 0xd18 */
	uint32_t unused_d1c[(0xd30 - 0xd1c) / 4];       /* 0xd1c */
	uint32_t rxq0_operation_mode;                   /* 0xd30 */
	uint32_t unused_d34;                            /* 0xd34 */
	uint32_t rxq0_debug;                            /* 0xd38 */
};

#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT          16
#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK           0x1ff
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT        2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_MASK         3
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED      2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TSF                BIT(1)
#define EQOS_MTL_TXQ0_OPERATION_MODE_FTQ                BIT(0)

#define EQOS_MTL_TXQ0_DEBUG_TXQSTS                      BIT(4)
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT                1
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK                 3

#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT          20
#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK           0x3ff
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT          14
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK           0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT          8
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK           0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_EHFC               BIT(7)
#define EQOS_MTL_RXQ0_OPERATION_MODE_RSF                BIT(5)

#define EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT                  16
#define EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK                   0x7fff
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT                4
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK                 3

#define EQOS_DMA_REGS_BASE 0x1000
struct eqos_dma_regs {
	uint32_t mode;                                  /* 0x1000 */
	uint32_t sysbus_mode;                           /* 0x1004 */
	uint32_t unused_1008[(0x1100 - 0x1008) / 4];    /* 0x1008 */
	uint32_t ch0_control;                           /* 0x1100 */
	uint32_t ch0_tx_control;                        /* 0x1104 */
	uint32_t ch0_rx_control;                        /* 0x1108 */
	uint32_t unused_110c;                           /* 0x110c */
	uint32_t ch0_txdesc_list_haddress;              /* 0x1110 */
	uint32_t ch0_txdesc_list_address;               /* 0x1114 */
	uint32_t ch0_rxdesc_list_haddress;              /* 0x1118 */
	uint32_t ch0_rxdesc_list_address;               /* 0x111c */
	uint32_t ch0_txdesc_tail_pointer;               /* 0x1120 */
	uint32_t unused_1124;                           /* 0x1124 */
	uint32_t ch0_rxdesc_tail_pointer;               /* 0x1128 */
	uint32_t ch0_txdesc_ring_length;                /* 0x112c */
	uint32_t ch0_rxdesc_ring_length;                /* 0x1130 */
	uint32_t ch0_inetn;                		/* 0x1134 */
	uint32_t ch0_rxwdt;                		/* 0x1138 */
	uint32_t unused_113c;                		/* 0x113C */
	uint32_t unused_1140;                		/* 0x1140 */
	uint32_t ch0_txdesc_curr_pointer;		/* 0x1144 */
};

#define EQOS_DMA_MODE_SWR                               BIT(0)

#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT           16
#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_MASK            0xf
#define EQOS_DMA_SYSBUS_MODE_EAME                       BIT(11)
#define EQOS_DMA_SYSBUS_MODE_AALE                       BIT(10)
#define EQOS_DMA_SYSBUS_MODE_BLEN16                     BIT(3)
#define EQOS_DMA_SYSBUS_MODE_BLEN8                      BIT(2)
#define EQOS_DMA_SYSBUS_MODE_BLEN4                      BIT(1)

#define EQOS_DMA_CH0_CONTROL_PBLX8                      BIT(16)

#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT             16
#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK              0x3f
#define EQOS_DMA_CH0_TX_CONTROL_OSP                     BIT(4)
#define EQOS_DMA_CH0_TX_CONTROL_ST                      BIT(0)

#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT             16
#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK              0x3f
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT              1
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK               0x3fff
#define EQOS_DMA_CH0_RX_CONTROL_SR                      BIT(0)

/* Descriptors */

/* We assume ARCH_DMA_MINALIGN >= 16; 16 is the EQOS HW minimum */
#define EQOS_DESCRIPTOR_ALIGN   ARCH_DMA_MINALIGN

#define EQOS_DESCRIPTOR_BYTES   16
#define EQOS_DESCRIPTOR_DUMMY   (ARCH_DMA_MINALIGN - EQOS_DESCRIPTOR_BYTES)
#define EQOS_DESCRIPTOR_SIZE    (EQOS_DESCRIPTOR_BYTES + EQOS_DESCRIPTOR_DUMMY)

#define EQOS_DESCRIPTORS_TX     4
#define EQOS_DESCRIPTORS_RX     16
#define EQOS_DESCRIPTORS_NUM    (EQOS_DESCRIPTORS_TX + EQOS_DESCRIPTORS_RX)
#define EQOS_DESCRIPTORS_SIZE   ALIGN(EQOS_DESCRIPTORS_NUM * \
				      EQOS_DESCRIPTOR_SIZE, ARCH_DMA_MINALIGN)
#define EQOS_BUFFER_ALIGN       ARCH_DMA_MINALIGN
#define EQOS_MAX_PACKET_SIZE    ALIGN(1568, ARCH_DMA_MINALIGN)
#define EQOS_RX_BUFFER_SIZE     (EQOS_DESCRIPTORS_RX * EQOS_MAX_PACKET_SIZE)

/*
 * Warn if the cache-line size is larger than the descriptor size. In such
 * cases the driver will likely fail because the CPU needs to flush the cache
 * when requeuing RX buffers, therefore descriptors written by the hardware
 * may be discarded. Architectures with full IO coherence, such as x86, do not
 * experience this issue, and hence are excluded from this condition.
 *
 * This can be fixed by defining CONFIG_SYS_NONCACHED_MEMORY which will cause
 * the driver to allocate descriptors from a pool of non-cached memory.
 */
#if EQOS_DESCRIPTOR_SIZE != ARCH_DMA_MINALIGN
#warning Cache line size is NOT equal to descriptor size
#endif

struct eqos_desc {
	u32 des0;
	u32 des1;
	u32 des2;
	u32 des3;
#if (ARCH_DMA_MINALIGN > 16)
	u32 dummy0;
	u32 dummy1;
	u32 dummy2;
	u32 dummy3;
#endif
#if (ARCH_DMA_MINALIGN > 32)
	u32 dummy4;
	u32 dummy5;
	u32 dummy6;
	u32 dummy7;
	u32 dummy9;
	u32 dummy10;
	u32 dummy11;
	u32 dummy12;
#endif
#if (ARCH_DMA_MINALIGN > 64)
#warning Driver is not designed to handle 128 bytes cache line size
#endif
};

#define EQOS_DESC3_OWN		BIT(31)
#define EQOS_DESC3_FD		BIT(29)
#define EQOS_DESC3_LD		BIT(28)
#define EQOS_DESC3_BUF1V	BIT(24)

struct eqos_config {
	bool reg_access_always_ok;
};

struct eqos_priv {
	struct eth_device *dev;
	const struct eqos_config *config;
	unsigned long regs;
	struct eqos_mac_regs *mac_regs;
	struct eqos_mtl_regs *mtl_regs;
	struct eqos_dma_regs *dma_regs;
	struct mii_dev *mii;
	struct phy_device *phy;
	void *descs;
	struct eqos_desc *tx_descs;
	struct eqos_desc *rx_descs;
	int tx_desc_idx, rx_desc_idx;
	void *tx_dma_buf;
	void *rx_dma_buf;
	void *rx_pkt;
	bool started;
	bool reg_access_ok;
	u32 phyaddr;
};

static u8 mdc_div = 1;

/*
 * TX and RX descriptors are 16 bytes. This causes problems with the cache
 * maintenance on CPUs where the cache-line size exceeds the size of these
 * descriptors. What will happen is that when the driver receives a packet
 * it will be immediately requeued for the hardware to reuse. The CPU will
 * therefore need to flush the cache-line containing the descriptor, which
 * will cause all other descriptors in the same cache-line to be flushed
 * along with it. If one of those descriptors had been written to by the
 * device those changes (and the associated packet) will be lost.
 *
 * To work around this, we make use of non-cached memory if available. If
 * descriptors are mapped uncached there's no need to manually flush them
 * or invalidate them.
 *
 * Note that this only applies to descriptors. The packet data buffers do
 * not have the same constraints since they are 1536 bytes large, so they
 * are unlikely to share cache-lines.
 */

static void *eqos_alloc_descs(unsigned int num)
{
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	return (void *)noncached_alloc(EQOS_DESCRIPTORS_SIZE,
				      EQOS_DESCRIPTOR_ALIGN);
#else
	return memalign(EQOS_DESCRIPTOR_ALIGN, EQOS_DESCRIPTORS_SIZE);
#endif
}

static void eqos_free_descs(void *descs)
{
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	/* FIXME: noncached_alloc() has no opposite */
#else
	free(descs);
#endif
}

static void eqos_inval_desc(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = (unsigned long)desc & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + EQOS_DESCRIPTOR_SIZE,
				  ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
#endif
}

static void eqos_flush_desc(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	flush_cache((unsigned long)desc, EQOS_DESCRIPTOR_SIZE);
#endif
}

static void eqos_inval_buffer(void *buf, size_t size)
{
	unsigned long start = (unsigned long)buf & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long end = ALIGN(start + size, ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

static void eqos_flush_buffer(void *buf, size_t size)
{
	size_t align_size = ALIGN(size + ARCH_DMA_MINALIGN - 1, ARCH_DMA_MINALIGN);
	flush_cache((unsigned long)buf, align_size);
}

static int eqos_mdio_wait_idle(struct eqos_priv *eqos)
{
	int i = 0;
	u32 reg;

	do {
		reg = readl(&eqos->mac_regs->mdio_address);
		if (!(reg & EQOS_MAC_MDIO_ADDRESS_GB))
			return 0;

		i++;
	} while (i < 1000000);

	return -1;
}

static int eqos_dma_wait_swrst(struct eqos_priv *eqos)
{
	int i = 0;
	u32 reg;

	writel(EQOS_DMA_MODE_SWR, &eqos->dma_regs->mode);

	do {
		reg = readl(&eqos->dma_regs->mode);
		if (!(reg & EQOS_DMA_MODE_SWR))
			return 0;

		i++;
	} while (i < 100);

	return -1;
}


static int eqos_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			  int mdio_reg)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d):\n", __func__, eqos->dev, mdio_addr,
	      mdio_reg);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(mdc_div <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_READ <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(10);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_data);
	val &= EQOS_MAC_MDIO_DATA_GD_MASK;

	debug("%s: val=%x\n", __func__, val);

	return val;
}

static int eqos_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			   int mdio_reg, u16 mdio_val)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d, val=%x):\n", __func__, eqos->dev,
	      mdio_addr, mdio_reg, mdio_val);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	writel(mdio_val, &eqos->mac_regs->mdio_data);

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(mdc_div <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_WRITE <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(10);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	return 0;
}

static int eqos_set_full_duplex(struct eth_device *dev)
{
	struct eqos_priv *eqos = dev->priv;

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	return 0;
}

static int eqos_set_half_duplex(struct eth_device *dev)
{
	struct eqos_priv *eqos = dev->priv;

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	/* WAR: Flush TX queue when switching to half-duplex */
	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_FTQ);

	return 0;
}

static int eqos_set_gmii_speed(struct eth_device *dev)
{
	struct eqos_priv *eqos = dev->priv;

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_100(struct eth_device *dev)
{
	struct eqos_priv *eqos = dev->priv;

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_10(struct eth_device *dev)
{
	struct eqos_priv *eqos = dev->priv;

	debug("%s(dev=%p):\n", __func__, dev);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_FES, EQOS_MAC_CONFIGURATION_PS);

	return 0;
}

static int eqos_adjust_link(struct eth_device *dev)
{
	struct eqos_priv *eqos = dev->priv;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	if (eqos->phy->duplex)
		ret = eqos_set_full_duplex(dev);
	else
		ret = eqos_set_half_duplex(dev);
	if (ret < 0) {
		pr_err("eqos_set_*_duplex() failed: %d", ret);
		return ret;
	}

	switch (eqos->phy->speed) {
	case SPEED_1000:
		ret = eqos_set_gmii_speed(dev);
		break;
	case SPEED_100:
		ret = eqos_set_mii_speed_100(dev);
		break;
	case SPEED_10:
		ret = eqos_set_mii_speed_10(dev);
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}
	if (ret < 0) {
		pr_err("eqos_set_*mii_speed*() failed: %d", ret);
		return ret;
	}

	return 0;
}

static int eqos_write_hwaddr(struct eth_device *eth_dev)
{
	struct eqos_priv *eqos = eth_dev->priv;
	uint32_t val;

	/*
	 * This function may be called before start() or after stop(). At that
	 * time, on at least some configurations of the EQoS HW, all clocks to
	 * the EQoS HW block will be stopped, and a reset signal applied. If
	 * any register access is attempted in this state, bus timeouts or CPU
	 * hangs may occur. This check prevents that.
	 *
	 * A simple solution to this problem would be to not implement
	 * write_hwaddr(), since start() always writes the MAC address into HW
	 * anyway. However, it is desirable to implement write_hwaddr() to
	 * support the case of SW that runs subsequent to U-Boot which expects
	 * the MAC address to already be programmed into the EQoS registers,
	 * which must happen irrespective of whether the U-Boot user (or
	 * scripts) actually made use of the EQoS device, and hence
	 * irrespective of whether start() was ever called.
	 *
	 * Note that this requirement by subsequent SW is not valid for
	 * Tegra186, and is likely not valid for any non-PCI instantiation of
	 * the EQoS HW block. This function is implemented solely as
	 * future-proofing with the expectation the driver will eventually be
	 * ported to some system where the expectation above is true.
	 */
	if (!eqos->config->reg_access_always_ok && !eqos->reg_access_ok)
		return 0;

	/* Update the MAC address */
	val = (eth_dev->enetaddr[5] << 8) |
		(eth_dev->enetaddr[4]);
	writel(val, &eqos->mac_regs->address0_high);
	val = (eth_dev->enetaddr[3] << 24) |
		(eth_dev->enetaddr[2] << 16) |
		(eth_dev->enetaddr[1] << 8) |
		(eth_dev->enetaddr[0]);
	writel(val, &eqos->mac_regs->address0_low);

	return 0;
}

static int eqos_start(struct eth_device *eth_dev, bd_t *bis)
{
	struct eqos_priv *eqos = eth_dev->priv;
	int ret, i;
	u32 val, tx_fifo_sz, rx_fifo_sz, tqs, rqs, pbl;
	ulong last_rx_desc;
    ulong addr;

	debug("%s(eth_dev=%p):\n", __func__, eth_dev);

	if (!eqos->phy) {
		ret = -1;
		pr_err("no phy is connected");
		goto err;
	}

	eqos->tx_desc_idx = 0;
	eqos->rx_desc_idx = 0;
	eqos->reg_access_ok = true;

	// Check DMA SW reset
	ret = eqos_dma_wait_swrst(eqos);
	if (ret) {
		pr_err("EQOS_DMA_MODE_SWR reset fail, ethernet function will be error, plz check rx_clk waveform");
		goto err;
	}

	ret = phy_startup(eqos->phy);
	if (ret < 0) {
		ret = -1;
		pr_err("phy_startup() failed: %d", ret);
		goto err;
	}

	if (!eqos->phy->link) {
		ret = -1;
		pr_err("No link");
		goto err;
	}

	ret = eqos_adjust_link(eth_dev);
	if (ret < 0) {
		pr_err("eqos_adjust_link() failed: %d", ret);
		goto err;
	}

	/* Configure MTL */

	/* Enable Store and Forward mode for TX */
	/* Program Tx operating mode */

	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_TSF |
		     (EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED <<
		      EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT));

//		/* Transmit Queue weight */
//		writel(0x10, &eqos->mtl_regs->txq0_quantum_weight);

	/* Enable Store and Forward mode for RX, since no jumbo frame */
	setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
		     EQOS_MTL_RXQ0_OPERATION_MODE_RSF);

	/* Transmit/Receive queue fifo size; use all RAM for 1 queue */
	val = readl(&eqos->mac_regs->hw_feature1);
	tx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK;
	rx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

	/*
	 * r/tx_fifo_sz is encoded as log2(n / 128). Undo that by shifting.
	 * r/tqs is encoded as (n / 256) - 1.
	 */
	tqs = (128 << tx_fifo_sz) / 256 - 1;
	rqs = (128 << rx_fifo_sz) / 256 - 1;

	clrsetbits_le32(&eqos->mtl_regs->txq0_operation_mode,
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK <<
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT,
			tqs << EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
	clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK <<
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT,
			rqs << EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT);

	/* Flow control used only if each channel gets 4KB or more FIFO */
	if (rqs >= ((4096 / 256) - 1)) {
		u32 rfd, rfa;

		setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			     EQOS_MTL_RXQ0_OPERATION_MODE_EHFC);

		/*
		 * Set Threshold for Activating Flow Contol space for min 2
		 * frames ie, (1500 * 1) = 1500 bytes.
		 *
		 * Set Threshold for Deactivating Flow Contol for space of
		 * min 1 frame (frame size 1500bytes) in receive fifo
		 */
		if (rqs == ((4096 / 256) - 1)) {
			/*
			 * This violates the above formula because of FIFO size
			 * limit therefore overflow may occur inspite of this.
			 */
			rfd = 0x3;	/* Full-3K */
			rfa = 0x1;	/* Full-1.5K */
		} else if (rqs == ((8192 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0xa;	/* Full-6K */
		} else if (rqs == ((16384 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x12;	/* Full-10K */
		} else {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x1E;	/* Full-16K */
		}

		clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT),
				(rfd <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(rfa <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT));
	}

	/* Configure MAC */

	clrsetbits_le32(&eqos->mac_regs->rxq_ctrl0,
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

	/* Set TX flow control parameters */
	/* Set Pause Time */
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		      0xffff << EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     0x0 << EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);
//		/* Assign priority for TX flow control */
//		clrbits_le32(&eqos->mac_regs->txq_prty_map0,
//			     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK <<
//			     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT);
//		/* Assign priority for RX flow control */
//		clrbits_le32(&eqos->mac_regs->rxq_ctrl2,
//			     EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK <<
//			     EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT);

//		setbits_le32(&eqos->mac_regs->rxq_ctrl2,
//			     BIT(0));

	/* Enable flow control */
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     EQOS_MAC_Q0_TX_FLOW_CTRL_TFE);
	setbits_le32(&eqos->mac_regs->rx_flow_ctrl,
		     EQOS_MAC_RX_FLOW_CTRL_RFE);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_GPSLCE |
			EQOS_MAC_CONFIGURATION_WD |
			EQOS_MAC_CONFIGURATION_JD |
			EQOS_MAC_CONFIGURATION_JE,
			EQOS_MAC_CONFIGURATION_CST |
			EQOS_MAC_CONFIGURATION_ACS);

	eqos_write_hwaddr(eth_dev);

	/* Configure DMA */

	/* Enable OSP mode */
	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_OSP);

	/* RX buffer size. Must be a multiple of bus width */
	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT,
			EQOS_MAX_PACKET_SIZE <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT);

	setbits_le32(&eqos->dma_regs->ch0_control,
		     (EQOS_DMA_CH0_CONTROL_PBLX8 | ((EQOS_DESCRIPTOR_DUMMY/8)<<18)));

	/*
	 * Burst length must be < 1/2 FIFO size.
	 * FIFO size in tqs is encoded as (n / 256) - 1.
	 * Each burst is n * 8 (PBLX8) * 16 (AXI width) == 128 bytes.
	 * Half of n * 256 is n * 128, so pbl == tqs, modulo the -1.
	 */
	pbl = tqs + 1;
	if (pbl > 32)
		pbl = 32;

	clrsetbits_le32(&eqos->dma_regs->ch0_tx_control,
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK <<
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT,
			16 << EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT,
			16 << EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT);

	/* DMA performance configuration */
	val = (2 << EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
		EQOS_DMA_SYSBUS_MODE_AALE;

//		val = (2 << EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
//			EQOS_DMA_SYSBUS_MODE_EAME | EQOS_DMA_SYSBUS_MODE_BLEN16 |
//			EQOS_DMA_SYSBUS_MODE_BLEN8 | EQOS_DMA_SYSBUS_MODE_BLEN4;

	writel(val, &eqos->dma_regs->sysbus_mode);

	/* Set up descriptors */

	memset(eqos->descs, 0, EQOS_DESCRIPTORS_SIZE);
	for (i = 0; i < EQOS_DESCRIPTORS_RX; i++) {
		struct eqos_desc *rx_desc = &(eqos->rx_descs[i]);
		rx_desc->des0 = (u32)(ulong)(eqos->rx_dma_buf +
					     (i * EQOS_MAX_PACKET_SIZE));
		rx_desc->des3 |= EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
        eqos_inval_buffer((void *)(uintptr_t)rx_desc->des0, EQOS_MAX_PACKET_SIZE);
	}
	flush_cache((unsigned long)eqos->descs, EQOS_DESCRIPTORS_SIZE);

    addr = (ulong)eqos->tx_descs;
    writel( addr & REG32_MASK, &eqos->dma_regs->ch0_txdesc_list_address);
//    writel( addr >> 32 , &eqos->dma_regs->ch0_txdesc_list_haddress);
    writel(EQOS_DESCRIPTORS_TX - 1,
        &eqos->dma_regs->ch0_txdesc_ring_length);

    addr = (ulong)eqos->rx_descs;
    writel( addr & REG32_MASK, &eqos->dma_regs->ch0_rxdesc_list_address);
//    writel( addr >>32, &eqos->dma_regs->ch0_rxdesc_list_haddress);
    writel(EQOS_DESCRIPTORS_RX - 1,
        &eqos->dma_regs->ch0_rxdesc_ring_length);

	/* Enable everything */

	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);
	setbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);

	/* TX tail pointer not written until we need to TX a packet */
	/*
	 * Point RX tail pointer at last descriptor. Ideally, we'd point at the
	 * first descriptor, implying all descriptors were available. However,
	 * that's not distinguishable from none of the descriptors being
	 * available.
	 */
	last_rx_desc = (ulong)&(eqos->rx_descs[(EQOS_DESCRIPTORS_RX - 1)]);
	writel(last_rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);

	eqos->started = true;

	debug("%s: OK\n", __func__);
	return 0;

err:
	printf("ERROR: %s FAILED: %d\n", eth_dev->name, ret);
	return ret;
}

static void eqos_stop(struct eth_device *eth_dev)
{
	struct eqos_priv *eqos = eth_dev->priv;
	int i;

	debug("%s(eth_dev=%p):\n", __func__, eth_dev);

	if (!eqos->started)
		return;

	eqos->started = false;
	eqos->reg_access_ok = false;

	/* Disable TX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);

	/* Wait for TX all packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->txq0_debug);
		u32 trcsts = (val >> EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
			EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK;
		u32 txqsts = val & EQOS_MTL_TXQ0_DEBUG_TXQSTS;
		if ((trcsts != 1) && (!txqsts))
			break;
	}

	/* Turn off MAC TX and RX */
	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	/* Wait for all RX packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->rxq0_debug);
		u32 prxq = (val >> EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK;
		u32 rxqsts = (val >> EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK;
		if ((!prxq) && (!rxqsts))
			break;
	}

	/* Turn off RX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);

}

static int eqos_send(struct eth_device *eth_dev, void *packet, int length)
{
	struct eqos_priv *eqos = eth_dev->priv;
	struct eqos_desc *tx_desc;
	int i;

	debug("%s(eth_dev=%p, packet=%p, length=%d):\n", __func__, eth_dev, packet,
	      length);

	memcpy(eqos->tx_dma_buf, packet, length);
	eqos_flush_buffer(eqos->tx_dma_buf, length);

	tx_desc = &(eqos->tx_descs[eqos->tx_desc_idx]);
	eqos->tx_desc_idx++;
	eqos->tx_desc_idx %= EQOS_DESCRIPTORS_TX;

	tx_desc->des0 = (ulong)eqos->tx_dma_buf;
	tx_desc->des1 = 0;
	tx_desc->des2 = length;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	tx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_FD | EQOS_DESC3_LD | length;
	eqos_flush_desc(tx_desc);

	writel((ulong)(tx_desc + 1), &eqos->dma_regs->ch0_txdesc_tail_pointer);

	for (i = 0; i < 1000000; i++) {
		eqos_inval_desc(tx_desc);
		if (!(readl(&tx_desc->des3) & EQOS_DESC3_OWN))
		{
			return 0;
		}
		udelay(1);
	}

	debug("%s: TX timeout\n", __func__);

	return -ETIMEDOUT;
}

static int eqos_free_pkt(struct eth_device *eth_dev, uchar *packet, int length)
{
	struct eqos_priv *eqos = eth_dev->priv;
	uchar *packet_expected;
	struct eqos_desc *rx_desc;

	debug("%s(rx_desc_idx=%d, packet=%p, length=%d)\n", __func__, eqos->rx_desc_idx, packet, length);

	packet_expected = eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	if (packet != packet_expected) {
		debug("%s: Unexpected packet (expected %p)\n", __func__,
		      packet_expected);
		return -EINVAL;
	}

	rx_desc = &(eqos->rx_descs[eqos->rx_desc_idx]);
	rx_desc->des0 = (u32)(ulong)packet;
	rx_desc->des1 = 0;
	rx_desc->des2 = 0;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	rx_desc->des3 |= EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
	eqos_flush_desc(rx_desc);

	writel((ulong)rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);

	eqos->rx_desc_idx++;
	eqos->rx_desc_idx %= EQOS_DESCRIPTORS_RX;

	return 0;
}


static int eqos_recv(struct eth_device *eth_dev)
{
	struct eqos_priv *eqos = eth_dev->priv;
	struct eqos_desc *rx_desc;
	int length;
	uintptr_t packetp;

	debug("%s(dev=%p: des indx 0x%x\n", __func__, eth_dev, eqos->rx_desc_idx);

	rx_desc = &(eqos->rx_descs[eqos->rx_desc_idx]);
	eqos_inval_buffer((void*)rx_desc, EQOS_DESCRIPTOR_SIZE);

	if (rx_desc->des3 & EQOS_DESC3_OWN) {
		debug("%s: RX packet not available\n", __func__);
		return -EAGAIN;
	}

	packetp = (uintptr_t) eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	length = rx_desc->des3 & 0x7fff;

	eqos_inval_buffer((void*)packetp, EQOS_MAX_PACKET_SIZE);
	mb();
	net_process_received_packet((void*)packetp, length);

	eqos_free_pkt(eth_dev, (void*)packetp, length);

	return length;
}

static int eqos_probe_resources_core(struct eth_device *dev)
{
	struct eqos_priv *eqos = dev->priv;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->descs = eqos_alloc_descs(EQOS_DESCRIPTORS_TX +
				       EQOS_DESCRIPTORS_RX);

	if (!eqos->descs) {
		debug("%s: eqos_alloc_descs() failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}

    if((ulong)eqos->descs > REG32_MASK ){
        pr_err("<---------Warn desc alloc over 32 bit -------------> %p \r\n", eqos->descs);
        goto err_free_tx_dma_buf;
    }

	eqos->tx_descs = (struct eqos_desc *)eqos->descs;
	eqos->rx_descs = (eqos->tx_descs + EQOS_DESCRIPTORS_TX);
	debug("%s: tx_descs=%p, rx_descs=%p\n", __func__, eqos->tx_descs,
	      eqos->rx_descs);

	eqos->tx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_MAX_PACKET_SIZE);
	if (!eqos->tx_dma_buf) {
		debug("%s: memalign(tx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_descs;
	}
	debug("%s: tx_dma_buf=%p\n", __func__, eqos->tx_dma_buf);

    if((ulong)eqos->tx_dma_buf > REG32_MASK ){
        pr_err("<---------fail Tx Buffer alloc over 32 bit -------------> %p \r\n", eqos->tx_dma_buf);
        goto err_free_tx_dma_buf;
    }

	eqos->rx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_RX_BUFFER_SIZE);
	if (!eqos->rx_dma_buf) {
		debug("%s: memalign(rx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_dma_buf;
	}
	debug("%s: rx_dma_buf=%p\n", __func__, eqos->rx_dma_buf);

    if((ulong)eqos->tx_dma_buf > REG32_MASK ){
        pr_err("<---------fail Rx Buffer alloc over 32 bit -------------> %p \r\n", eqos->rx_dma_buf);
        goto err_free_rx_dma_buf;
    }

	eqos->rx_pkt = malloc(EQOS_MAX_PACKET_SIZE);
	if (!eqos->rx_pkt) {
		debug("%s: malloc(rx_pkt) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_rx_dma_buf;
	}
	debug("%s: rx_pkt=%p\n", __func__, eqos->rx_pkt);

	debug("%s: OK\n", __func__);
	return 0;

err_free_rx_dma_buf:
	free(eqos->rx_dma_buf);
err_free_tx_dma_buf:
	free(eqos->tx_dma_buf);
err_free_descs:
	eqos_free_descs(eqos->descs);
err:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove_resources_core(struct eth_device *eth_dev)
{
	struct eqos_priv *eqos = eth_dev->priv;

	debug("%s(dev=%p):\n", __func__, eth_dev);

	free(eqos->rx_pkt);
	free(eqos->rx_dma_buf);
	free(eqos->tx_dma_buf);
	eqos_free_descs(eqos->descs);

	debug("%s: OK\n", __func__);
	return 0;
}

static void eth_phy_poweron(unsigned long base_addr)
{
	unsigned long reg;

	reg = readl(base_addr + 0x3800 + 0xF8);
	writel(reg | (1<<7), base_addr + 0x3800 + 0xF8);
	udelay(20);
	reg = readl(base_addr + 0x3800 + 0xC8);
	writel(reg & (~(1<<0)), base_addr + 0x3800 + 0xC8);
	udelay(200);
	reg = readl(base_addr + 0x3800 + 0xC8);
	writel(reg & (~(1<<1)), base_addr + 0x3800 + 0xC8);
	udelay(250);
	reg = readl(base_addr + 0x3800 + 0x2E8);
	writel(reg & (~(1<<0)), base_addr + 0x3800 + 0x2E8);
	reg = readl(base_addr + 0x3800 + 0xCC);
	writel(reg & (~(1<<0)), base_addr + 0x3800 + 0xCC);
	reg = readl(base_addr + 0x3800 + 0xDC);
	writel(reg | (1<<0), base_addr + 0x3800 + 0xDC);
	reg = readl(base_addr + 0x3800 + 0x9C);
	writel(reg & (~(1<<0)), base_addr + 0x3800 + 0x9C);
}

typedef enum {
    PIN_ETH_CFG_NONE,

    PIN_ETH_CFG_RGMII_1ST_PINMUX  = 0x001,    ///< Enable RGMII.
    PIN_ETH_CFG_RGMII_2ND_PINMUX  = 0x002,    ///< Enable RGMII.

    PIN_ETH_CFG_RMII_1ST_PINMUX   = 0x010,    ///< Enable RMII.
    PIN_ETH_CFG_RMII_2ND_PINMUX   = 0x020,    ///< Enable RMII.

    PIN_ETH_CFG_REFCLK_PINMUX     = 0x100,    ///< Enable REFCLK. (E_GPIO[0] or S_GPIO[0])
    PIN_ETH_CFG_RST_PINMUX        = 0x200,    ///< Enable RST. (E_GPIO[1] or S_GPIO[1])
    PIN_ETH_CFG_MDC_MDIO_PINMUX   = 0x400,    ///< Enable MDC/MDIO. (E_GPIO[15..14] or S_GPIO[15..14])

    PIN_ETH2_CFG_RGMII_1ST_PINMUX  = 0x1000,  ///< Enable RGMII.
    PIN_ETH2_CFG_RGMII_2ND_PINMUX  = 0x2000,  ///< Enable RGMII.

    PIN_ETH2_CFG_RMII_1ST_PINMUX   = 0x10000, ///< Enable RMII.
    PIN_ETH2_CFG_RMII_2ND_PINMUX   = 0x20000, ///< Enable RMII.

    PIN_ETH2_CFG_REFCLK_PINMUX     = 0x100000,///< Enable REFCLK. (E_GPIO[16] or S_GPIO[18])
    PIN_ETH2_CFG_RST_PINMUX        = 0x200000,///< Enable RST. (E_GPIO[17] or S_GPIO[19])
    PIN_ETH2_CFG_MDC_MDIO_PINMUX   = 0x400000,///< Enable MDC/MDIO. (E_GPIO[31..30] or S_GPIO[33..32])

    PIN_ETH_CFG_ACT_LED_1ST_PINMUX  = 0x1000000, ///< ETH_ACT_LED (J_GPIO[1])
    PIN_ETH_CFG_ACT_LED_2ND_PINMUX  = 0x2000000, ///< ETH_2_ACT_LED (D_GPIO[8])
    PIN_ETH_CFG_LINK_LED_1ST_PINMUX = 0x4000000, ///< ETH_LINK_LED (J_GPIO[2])
    PIN_ETH_CFG_LINK_LED_2ND_PINMUX = 0x8000000, ///< ETH_2_LINK_LED (D_GPIO[9])

    PIN_ETH_CFG_PHY_SEL             = 0x10000000, ///< ETH PHY Sel from GMAC0 or GMAC1

}PIN_ETH_CFG;

static void mac0_clk_en(phy_interface_t phy_intf)
{

	UINT reg;
/*

	reg = readl(IOADDR_CG_REG_BASE + 0x54);
	reg &= ~(1<<1);	// assert ETH0 AXI reset
	writel(reg, IOADDR_CG_REG_BASE + 0x54);

	reg = readl(IOADDR_CG_REG_BASE + 0x58);
	reg &= ~(1<<9);	// assert ETH0 module reset
	writel(reg, IOADDR_CG_REG_BASE + 0x58);

	reg = readl(IOADDR_CG_REG_BASE + 0xE4);
	reg &= ~(1<<8);	// assert ETH0 glue reset
	writel(reg, IOADDR_CG_REG_BASE + 0xE4);

	reg = readl(IOADDR_CG_REG_BASE + 0x60);
	reg &= ~(1<<16);// release ETH0 bus clk gating
	writel(reg, IOADDR_CG_REG_BASE + 0x60);

	reg = readl(IOADDR_CG_REG_BASE + 0x70);
	reg &= ~(1<<9);// release ETH0 gating
	writel(reg, IOADDR_CG_REG_BASE + 0x70);

	reg = readl(IOADDR_CG_REG_BASE + 0x74);
	reg &= ~(1<<5);// release GPIO2 gating
	writel(reg, IOADDR_CG_REG_BASE + 0x74);

	switch (phy_intf) {
	case PHY_INTERFACE_MODE_RMII:
		break;
	case PHY_INTERFACE_MODE_RGMII:
		break;
	default: // embedded phy (MII)
		reg = readl(IOADDR_CG_REG_BASE + 0xE4);
		reg |= (1<<4);	// REF_CLK_I gating
		writel(reg, IOADDR_CG_REG_BASE + 0xE4);
		break;
	}

	reg = readl(IOADDR_CG_REG_BASE + 0x54);
	reg |= (1<<1);	// release ETH0 AXI reset
	writel(reg, IOADDR_CG_REG_BASE + 0x54);

	reg = readl(IOADDR_CG_REG_BASE + 0x58);
	reg |= (1<<9);	// release ETH0 module reset
	reg |= (1<<29);	// release GPIO2 module reset
	writel(reg, IOADDR_CG_REG_BASE + 0x58);

	reg = readl(IOADDR_CG_REG_BASE + 0xE4);
	reg |= (1<<8);	// release ETH0 glue reset
	writel(reg, IOADDR_CG_REG_BASE + 0xE4);
*/

    reg = readl(IOADDR_CG_REG_BASE + 0x0);
	reg |= (1<<7);	// ENABLE PLL7
	writel(reg, IOADDR_CG_REG_BASE + 0x0);

    udelay(10);

    reg = readl(IOADDR_CG_REG_BASE + 0x70);
	reg |= (1<<27);	// ENABLE ETH0 Module CLK
	writel(reg, IOADDR_CG_REG_BASE + 0x70);

    udelay(10);

    reg = readl(IOADDR_CG_REG_BASE + 0x94);
	reg |= (1<<16);	// realse eth0 rst
	writel(reg, IOADDR_CG_REG_BASE + 0x94);

    udelay(10);




}

static void mac1_clk_en(phy_interface_t phy_intf)
{
	UINT reg;
/*
	reg = readl(IOADDR_CG_REG_BASE + 0x54);
	reg &= ~(1<<2);	// assert ETH1 AXI reset
	writel(reg, IOADDR_CG_REG_BASE + 0x54);

	reg = readl(IOADDR_CG_REG_BASE + 0x58);
	reg &= ~(1<<10);// assert ETH1 module reset
	writel(reg, IOADDR_CG_REG_BASE + 0x58);

	reg = readl(IOADDR_CG_REG_BASE + 0xE4);
	reg &= ~(1<<24);// assert ETH1 glue reset
	writel(reg, IOADDR_CG_REG_BASE + 0xE4);

	reg = readl(IOADDR_CG_REG_BASE + 0x60);
	reg &= ~(1<<17);// release ETH1 bus clk gating
	writel(reg, IOADDR_CG_REG_BASE + 0x60);

	reg = readl(IOADDR_CG_REG_BASE + 0x70);
	reg &= ~(1<<10);// release ETH1 gating
	writel(reg, IOADDR_CG_REG_BASE + 0x70);

	reg = readl(IOADDR_CG_REG_BASE + 0x74);
	reg &= ~(1<<4);// release GPIO1 gating
	writel(reg, IOADDR_CG_REG_BASE + 0x74);

	switch (phy_intf) {
	case PHY_INTERFACE_MODE_RMII:
		break;
	case PHY_INTERFACE_MODE_RGMII:
		break;
	default: // embedded phy (MII)
		reg = readl(IOADDR_CG_REG_BASE + 0xE4);
		reg &= ~(1<<20);	// REF_CLK_I gating
		writel(reg, IOADDR_CG_REG_BASE + 0xE4);
		break;
	}

	reg = readl(IOADDR_CG_REG_BASE + 0x54);
	reg |= (1<<2);	// release ETH1 AXI reset
	writel(reg, IOADDR_CG_REG_BASE + 0x54);

	reg = readl(IOADDR_CG_REG_BASE + 0x58);
	reg |= (1<<10);	// release ETH1 module reset
	reg |= (1<<28);	// release GPIO1 module reset
	writel(reg, IOADDR_CG_REG_BASE + 0x58);

	reg = readl(IOADDR_CG_REG_BASE + 0xE4);
	reg |= (1<<24);	// release ETH1 glue reset
	writel(reg, IOADDR_CG_REG_BASE + 0xE4);

	*/

    reg = readl(IOADDR_CG_REG_BASE + 0x0);
	reg |= (1<<7);	// ENABLE PLL7
	writel(reg, IOADDR_CG_REG_BASE + 0x0);

    udelay(10);

    reg = readl(IOADDR_CG_REG_BASE + 0x70);
	reg |= (1<<28);	// ENABLE ETH1 Module CLK
	writel(reg, IOADDR_CG_REG_BASE + 0x70);

    udelay(10);

    reg = readl(IOADDR_CG_REG_BASE + 0x94);
	reg |= (1<<18);	// realse eth1 rst
	writel(reg, IOADDR_CG_REG_BASE + 0x94);

    udelay(10);


}

static void extphy0_clk_en(phy_interface_t phy_intf, int phy_clk, int ref_clk_out, int pin_num)
{
	UINT reg;

	switch (phy_intf) {
	case PHY_INTERFACE_MODE_RMII:
		reg = readl(IOADDR_ETH_REG_BASE + 0x3014);
        // pre-assume refclk: phy--> mac

        reg |= 1<<0;
        reg &= ~(1<<4);
        reg |=  (1<<5);

		reg &= ~(1<<2);// refclk referenced from pad
		reg &= ~(0x3<<30);	// TXD_SRC: RMII
		if (ref_clk_out) {
			reg &= ~(1<<0);
			reg |= 1<<4;
		}
		writel(reg, IOADDR_ETH_REG_BASE + 0x3014);

        reg = readl(IOADDR_ETH_REG_BASE + 0x3004);
        reg |= (1<<4);	// select external phy
        writel(reg, IOADDR_ETH_REG_BASE + 0x3004);

		// fall through
	case PHY_INTERFACE_MODE_RGMII:
		if (phy_intf == PHY_INTERFACE_MODE_RGMII) {
			reg = readl(IOADDR_ETH_REG_BASE + 0x3014);
            reg &= (0x3<<30);
			reg |= (0x1<<30);	// TXD_SRC: RGMII
			reg |= (3<<4);// select external phy
			writel(reg, IOADDR_ETH_REG_BASE + 0x3014);
		}

		reg = readl(IOADDR_ETH_REG_BASE + 0x3004);
        reg |= (1<<4);// select external phy

		writel(reg, IOADDR_ETH_REG_BASE + 0x3004);
		writel(0x1FF, IOADDR_ETH_REG_BASE + 0x300C);

		/*if (phy_clk > 0) {
			reg = readl(IOADDR_CG_REG_BASE + 0xE4);
			reg |= (1<<7);// release ETH0 ext phy clk output
			writel(reg, IOADDR_CG_REG_BASE + 0xE4);

			// enable pinmux
			reg = readl(IOADDR_TOP_REG_BASE + 0x70);
			reg &= ~(0x7<<0);
			if (phy_intf == PHY_INTERFACE_MODE_RGMII) {
				reg |= (1<<0);
			} else if (phy_intf == PHY_INTERFACE_MODE_RMII) {
				reg |= (2<<0);
			}
			writel(reg, IOADDR_TOP_REG_BASE + 0x70);
		}*/


		break;
	default:

		reg = readl(IOADDR_ETH_REG_BASE + 0x3004);
		reg &= ~(0x3<<4);// select embedded phy
		writel(reg, IOADDR_ETH_REG_BASE + 0x3004);

		eth_phy_poweron(IOADDR_ETH_REG_BASE);
		// phy setting
		phy_sw_reset_enable(IOADDR_ETH_REG_BASE);
		set_break_link_timer(IOADDR_ETH_REG_BASE);
		set_led_blinking(IOADDR_ETH_REG_BASE);
		mdelay(10);
		phy_sw_reset_disable(IOADDR_ETH_REG_BASE);

		break;
	}

    if (phy_clk > 0) {
        reg = readl(IOADDR_CG_REG_BASE + 0x74); // CG EXT PHY EN
        reg |= (1<<7);
        writel(reg, IOADDR_CG_REG_BASE + 0x74);

        reg = readl(IOADDR_TOP_REG_BASE + 0xC); // TOP EXT PHY EN
        reg |= (1<<8);
        writel(reg, IOADDR_TOP_REG_BASE + 0xC);

        if(pin_num == 1){
            reg = readl(IOADDR_TOP_REG_BASE + 0xB0); // TOP EXT PHY GPIO FUNCTION NORMAL
            reg &= ~(1<<0);
            writel(reg, IOADDR_TOP_REG_BASE + 0xB0);
        }/*else if(pin_num == 2){
            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // TOP EXT PHY GPIO FUNCTION NORMAL
            reg &= ~(1<<0);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);
        }*/

    }
    if(pin_num == 1){
        // reset external phy
        gpio_request(E_GPIO(1), "PHY0_RST");
        gpio_direction_output(E_GPIO(1), 0);
        mdelay(10);	// assert low at least 10ms
        gpio_set_value(E_GPIO(1), 1);
        mdelay(50);	// wait 50ms before access phy register
        gpio_free(E_GPIO(1));
    }
    /*else if(pin_num == 2){
        gpio_request(S_GPIO(1), "PHY1_RST");
        gpio_direction_output(S_GPIO(1), 0);
        mdelay(10);	// assert low at least 10ms
        gpio_set_value(S_GPIO(1), 1);
        mdelay(50);	// wait 50ms before access phy register
        gpio_free(S_GPIO(1));
	}*/
}

static void extphy1_clk_en(phy_interface_t phy_intf, int phy_clk, int ref_clk_out , int pin_num)
{
	UINT reg;

	switch (phy_intf) {
	case PHY_INTERFACE_MODE_RMII:
		reg = readl(IOADDR_ETH1_REG_BASE + 0x3014);
        // pre-assume refclk: phy--> mac

        reg |= 1<<0;
        reg &= ~(1<<4);
        reg |=  (1<<5);

		reg &= ~(1<<2);// refclk referenced from pad
		reg &= ~(0x3<<30);	// TXD_SRC: RMII
		if (ref_clk_out) {
			reg &= ~(1<<0);
			reg |= 1<<4;
		}
		writel(reg, IOADDR_ETH1_REG_BASE + 0x3014);

        reg = readl(IOADDR_ETH1_REG_BASE + 0x3004);
        reg |= (1<<4);	// select external phy
        writel(reg, IOADDR_ETH1_REG_BASE + 0x3004);

		// fall through
	case PHY_INTERFACE_MODE_RGMII:
		if (phy_intf == PHY_INTERFACE_MODE_RGMII) {
			reg = readl(IOADDR_ETH1_REG_BASE + 0x3014);
            reg &= (0x3<<30);
			reg |= (0x1<<30);	// TXD_SRC: RGMII
			reg |= (3<<4);// select external phy
			writel(reg, IOADDR_ETH1_REG_BASE + 0x3014);
		}

		reg = readl(IOADDR_ETH1_REG_BASE + 0x3004);
        reg |= (1<<4);// select external phy

		writel(reg, IOADDR_ETH1_REG_BASE + 0x3004);
		writel(0x1FF, IOADDR_ETH1_REG_BASE + 0x300C);

		/*if (phy_clk > 0) {
			reg = readl(IOADDR_CG_REG_BASE + 0xE4);
			reg |= (1<<7);// release ETH0 ext phy clk output
			writel(reg, IOADDR_CG_REG_BASE + 0xE4);

			// enable pinmux
			reg = readl(IOADDR_TOP_REG_BASE + 0x70);
			reg &= ~(0x7<<0);
			if (phy_intf == PHY_INTERFACE_MODE_RGMII) {
				reg |= (1<<0);
			} else if (phy_intf == PHY_INTERFACE_MODE_RMII) {
				reg |= (2<<0);
			}
			writel(reg, IOADDR_TOP_REG_BASE + 0x70);
		}*/


		break;
	default:

		reg = readl(IOADDR_ETH1_REG_BASE + 0x3004);
		reg &= ~(0x3<<4);// select embedded phy
		writel(reg, IOADDR_ETH1_REG_BASE + 0x3004);

		eth_phy_poweron(IOADDR_ETH1_REG_BASE);
		// phy setting
		phy_sw_reset_enable(IOADDR_ETH1_REG_BASE);
		set_break_link_timer(IOADDR_ETH1_REG_BASE);
		set_led_blinking(IOADDR_ETH1_REG_BASE);
		mdelay(10);
		phy_sw_reset_disable(IOADDR_ETH1_REG_BASE);

		break;
	}

    if (phy_clk > 0) {
        reg = readl(IOADDR_CG_REG_BASE + 0x74); // CG EXT PHY EN
        reg |= (1<<8);
        writel(reg, IOADDR_CG_REG_BASE + 0x74);

        reg = readl(IOADDR_TOP_REG_BASE + 0xC); // TOP EXT PHY EN
        reg |= (1<<16);
        writel(reg, IOADDR_TOP_REG_BASE + 0xC);

        if(pin_num == 1){
            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // TOP EXT PHY GPIO FUNCTION NORMAL
            reg &= ~(1<<0);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);
        }/*else if(pin_num == 2){
            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // TOP EXT PHY GPIO FUNCTION NORMAL
            reg &= ~(1<<18);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);
        }*/
    }
    if(pin_num == 1){
        // reset external phy
        gpio_request(S_GPIO(1), "PHY1_RST");
        gpio_direction_output(S_GPIO(1), 0);
        mdelay(10);	// assert low at least 10ms
        gpio_set_value(S_GPIO(1), 1);
        mdelay(50);	// wait 50ms before access phy register
        gpio_free(S_GPIO(1));
    }
    /*else if(pin_num == 2){
        // reset external phy
        gpio_request(S_GPIO(1), "PHY1_RST");
        gpio_direction_output(S_GPIO(1), 0);
        mdelay(10);	// assert low at least 10ms
        gpio_set_value(S_GPIO(1), 1);
        mdelay(50);	// wait 50ms before access phy register
        gpio_free(S_GPIO(1));
    }*/

}

static void mac0_pinmux_en(phy_interface_t phy_intf, int pin_num)
{
	UINT reg;

	switch (phy_intf) {
	case PHY_INTERFACE_MODE_RMII:
        /*
		// set pad driving
		reg = readl(IOADDR_PAD_REG_BASE + 0xC0);
		reg &= ~((0x3<<22) | (0x3<<20) | (0x3<<18) | (0x3<<4) | (0x3<<0));
		writel(reg | 0x01, IOADDR_PAD_REG_BASE + 0xC0);	// PHYCLK (25M) 8 mA, others 4 mA
		*/
        if(pin_num == 1){

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RMII mode
            reg &= ~(0x7<<0);
            writel(reg | (0x2<<0), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<12);
            writel(reg| (0x1<<12), IOADDR_TOP_REG_BASE + 0x0C);

			reg = readl(IOADDR_TOP_REG_BASE + 0xB0); // GPIO EN
            reg &= ~(0x0000CE3C);
            writel(reg, IOADDR_TOP_REG_BASE + 0xB0);


        }
#if 0
        else if(pin_num == 2){
            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RMII mode
            reg &= ~(0x7<<0);
            writel(reg | (0x4<<0), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<12);
            writel(reg| (0x2<<12), IOADDR_TOP_REG_BASE + 0x0C);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // GPIO EN
            reg &= ~(0xCE3C);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);
        }
#endif

		break;
	case PHY_INTERFACE_MODE_RGMII:
        /*
		// set pad driving
		reg = readl(IOADDR_PAD_REG_BASE + 0xC0);
		// PHYCLK, TXCLK, TXCTL, TXD 8 mA
		reg &= ~((0x3<<26) | (0x3<<24) | (0x3<<22) | (0x3<<20) | (0x3<<18) | (0x3<<0));
		reg |= (0x1<<26) | (0x1<<24) | (0x1<<22) | (0x1<<20) | (0x1<<18) | (0x1<<0);
		writel(reg, IOADDR_PAD_REG_BASE + 0xC0);
		*/

		// set pinmux
        if(pin_num == 1){

            // set pad driving
            reg = readl(IOADDR_PAD_REG_BASE + 0x90);
            // PHYCLK 8 mA
            reg &= ~ (0xf<<0);
            reg |= (0x1<<0);
            writel(reg, IOADDR_PAD_REG_BASE + 0x90);
            reg = readl(IOADDR_PAD_REG_BASE + 0x94);
            // TXCLK, TXCTL, TXD 8 mA
            reg &= ~((0xf<<20) | (0xf<<16) | (0xf<<12) | (0xf<<8) | (0xf<<4) | (0xf<<0));
            reg |= (0x1<<20) | (0x1<<16) | (0x1<<12) | (0x1<<8) | (0x1<<4) | (0x1<<0);
            writel(reg, IOADDR_PAD_REG_BASE + 0x94);

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RGMII mode
            reg &= ~(0x7<<0);
            writel(reg | (0x1<<0), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<12);
            writel(reg| (0x1<<12), IOADDR_TOP_REG_BASE + 0x0C);

            reg = readl(IOADDR_TOP_REG_BASE + 0xB0); // GPIO EN
            reg &= ~(0x0000FFFC);
            writel(reg, IOADDR_TOP_REG_BASE + 0xB0);

        }
#if 0
        else if(pin_num == 2){

            // set pad driving
            reg = readl(IOADDR_PAD_REG_BASE + 0xA0);
            // PHYCLK 8 mA
            reg &= ~ (0xf<<0);
            reg |= (0x1<<0);
            writel(reg, IOADDR_PAD_REG_BASE + 0xA0);
            reg = readl(IOADDR_PAD_REG_BASE + 0xA4);
            // TXCLK, TXCTL, TXD 8 mA
            reg &= ~((0xf<<20) | (0xf<<16) | (0xf<<12) | (0xf<<8) | (0xf<<4) | (0xf<<0));
            reg |= (0x1<<20) | (0x1<<16) | (0x1<<12) | (0x1<<8) | (0x1<<4) | (0x1<<0);
            writel(reg, IOADDR_PAD_REG_BASE + 0xA4);


            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RGMII mode
            reg &= ~(0x7<<0);
            writel(reg | (0x3<<0), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<12);
            writel(reg| (0x2<<12), IOADDR_TOP_REG_BASE + 0x0C);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // GPIO EN
            reg &= ~(0xFFFC);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);
        }
#endif

		break;
	default: // embedded phy (MII)

		reg = readl(IOADDR_TOP_REG_BASE + 0xC);
		reg &= ~(0x1<<31);
		reg |= 1<<31;	// connect EMB phy to MAC0
		writel(reg, IOADDR_TOP_REG_BASE + 0xC);
		break;
	}
}

static void mac1_pinmux_en(phy_interface_t phy_intf, int pin_num)
{
	UINT reg;

	switch (phy_intf) {
	case PHY_INTERFACE_MODE_RMII:
        /*
		// set pad driving
		reg = readl(IOADDR_PAD_REG_BASE + 0xC0);
		reg &= ~((0x3<<22) | (0x3<<20) | (0x3<<18) | (0x3<<4) | (0x3<<0));
		writel(reg | 0x01, IOADDR_PAD_REG_BASE + 0xC0);	// PHYCLK (25M) 8 mA, others 4 mA
		*/
        if(pin_num == 1){
            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RMII mode
            reg &= ~(0x7<<4);
            writel(reg | (0x2<<4), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<20);
            writel(reg| (0x1<<20), IOADDR_TOP_REG_BASE + 0x0C);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // GPIO EN
            reg &= ~(0x0000CE3C);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);

        }

#if 0
        else if(pin_num == 2){
            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RMII mode
            reg &= ~(0x7<<4);
            writel(reg | (0x4<<4), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<20);
            writel(reg| (0x2<<20), IOADDR_TOP_REG_BASE + 0x0C);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // GPIO EN
            reg &= ~(0x38F00000);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC4); // GPIO EN
            reg &= ~(0x3);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC4);

        }
#endif
		break;
	case PHY_INTERFACE_MODE_RGMII:
        /*
		// set pad driving
		reg = readl(IOADDR_PAD_REG_BASE + 0xC0);
		// PHYCLK, TXCLK, TXCTL, TXD 8 mA
		reg &= ~((0x3<<26) | (0x3<<24) | (0x3<<22) | (0x3<<20) | (0x3<<18) | (0x3<<0));
		reg |= (0x1<<26) | (0x1<<24) | (0x1<<22) | (0x1<<20) | (0x1<<18) | (0x1<<0);
		writel(reg, IOADDR_PAD_REG_BASE + 0xC0);
		*/

		// set pinmux
        if(pin_num == 1){

            // set pad driving
            reg = readl(IOADDR_PAD_REG_BASE + 0x98);
            // PHYCLK 8 mA
            reg &= ~ (0xf<<0);
            reg |= (0x1<<0);
            writel(reg, IOADDR_PAD_REG_BASE + 0x98);
            reg = readl(IOADDR_PAD_REG_BASE + 0x9C);
            // TXCLK, TXCTL, TXD 8 mA
            reg &= ~((0xf<<20) | (0xf<<16) | (0xf<<12) | (0xf<<8) | (0xf<<4) | (0xf<<0));
            reg |=  (0x1<<20) | (0x1<<16) | (0x1<<12) | (0x1<<8) | (0x1<<4) | (0x1<<0) ;
            writel(reg, IOADDR_PAD_REG_BASE + 0x9C);

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RGMII mode
            reg &= ~(0x7<<4);
            writel(reg | (0x1<<4), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<20);
            writel(reg| (0x1<<20), IOADDR_TOP_REG_BASE + 0x0C);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // GPIO EN
            reg &= ~(0x0000FFFC);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);
        }
#if 0
        else if(pin_num == 2){

            // set pad driving
            reg = readl(IOADDR_PAD_REG_BASE + 0xA8);
            // PHYCLK 8 mA
            reg &= ~ (0xf<<8);
            reg |= (0x1<<8);
            writel(reg, IOADDR_PAD_REG_BASE + 0xA8);
            reg = readl(IOADDR_PAD_REG_BASE + 0xAC);
            // TXCLK, TXCTL, TXD 8 mA
            reg &= ~((0xf<<28) | (0xf<<24) | (0xf<<20) | (0xf<<16) | (0xf<<12) | (0xf<<8));
            reg |=  (0x1<<28) | (0x1<<24) | (0x1<<20) | (0x1<<16) | (0x1<<12) | (0x1<<8);
            writel(reg, IOADDR_PAD_REG_BASE + 0xAC);

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // RGMII mode
            reg &= ~(0x7<<4);
            writel(reg | (0x3<<4), IOADDR_TOP_REG_BASE + 0x0C);

            /*reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // REF CLK EN
            reg |= (0x1<<8);
            writel(reg , IOADDR_TOP_REG_BASE + 0x0C);*/

            reg = readl(IOADDR_TOP_REG_BASE + 0x0C); // MDIO EN
            reg &= ~(0x3<<20);
            writel(reg| (0x2<<20), IOADDR_TOP_REG_BASE + 0x0C);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC0); // GPIO EN
            reg &= ~(0xFFF00000);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC0);

            reg = readl(IOADDR_TOP_REG_BASE + 0xC4); // GPIO EN
            reg &= ~(0x3);
            writel(reg, IOADDR_TOP_REG_BASE + 0xC4);
        }
#endif
		break;
	default: // embedded phy (MII)

		reg = readl(IOADDR_TOP_REG_BASE + 0xC);
		reg &= ~(0x1<<31);
		reg &= ~(1<<31);// connect EMB phy to MAC1
		writel(reg, IOADDR_TOP_REG_BASE + 0xC);
		break;
	}
}

struct NVT_PLAT_INFO {
	phys_addr_t	base_pa;		// base physical address
	char	*dtb_name;
	void	(*mac_pinmux_en)(phy_interface_t phy_intf, int pin_num);
	void	(*mac_clk_en)(phy_interface_t phy_intf);
	void	(*extphy_clk_en)(phy_interface_t phy_intf, int phy_clk, int ref_clk_out, int pin_num);
	char	*rmii_refclk_i_name;	// name of rmii_refclk_i
	char	*rxclk_i_name;
	char	*txclk_i_name;
	char	*ext_phy_clk_name;

    UINT    pinmux_ext_phy;

	UINT	pinmux_rgmii;
    UINT	pinmux_rgmii_2;

	UINT	pinmux_rmii;
    UINT	pinmux_rmii_2;

	UINT	pinmux_emb;

	UINT	gpio_reset;
    UINT	gpio_reset_2;
};

static const struct NVT_PLAT_INFO v_nvt_plat_info[2] = {
	// MAC0 (0xF044_0000)
	{
		0x2F0440000,
#ifdef CONFIG_TARGET_NA51103
		"eth0@f0440000",
#else
		"eth0@2,f0440000",
#endif
		mac0_pinmux_en,
		mac0_clk_en,
		extphy0_clk_en,
		"rmii0_refclk_i",
		"eth0_rxclk_i",
		"eth0_txclk_i",
		"eth0_extphy_clk",

        PIN_ETH_CFG_REFCLK_PINMUX,

		PIN_ETH_CFG_RGMII_1ST_PINMUX,
		PIN_ETH_CFG_RGMII_2ND_PINMUX,

		PIN_ETH_CFG_RMII_1ST_PINMUX,
		PIN_ETH_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_PHY_SEL,

		E_GPIO(1),			// gpio reset
		S_GPIO(1),			// gpio reset
	},
	// MAC1 (0xF045_0000)
	{
		0x2F0450000,
#ifdef CONFIG_TARGET_NA51103
		"eth1@f0450000",
#else
		"eth1@2,f0450000",
#endif
		mac1_pinmux_en,
		mac1_clk_en,
		extphy1_clk_en,
		"rmii1_refclk_i",
		"eth1_rxclk_i",
		"eth1_txclk_i",
		"eth1_extphy_clk",

        PIN_ETH2_CFG_REFCLK_PINMUX,

		PIN_ETH2_CFG_RGMII_1ST_PINMUX,
		PIN_ETH2_CFG_RGMII_2ND_PINMUX,

		PIN_ETH2_CFG_RMII_1ST_PINMUX,
		PIN_ETH2_CFG_RMII_2ND_PINMUX,

		PIN_ETH_CFG_NONE,

		S_GPIO(1),			// gpio reset
		S_GPIO(19),			// gpio reset
	},
};

static int eth_parse_phy_intf(unsigned long base_addr)
{
	UINT ETH_QOS_INDEX;	// index to MAC (0: MAC0, 1: MAC1);
	UINT ETH_PHY_INTF = 0;
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int len;
    int pin_num = 0;
//	u32 phy_intf, led_intf; no embd phy so do not control led.
    u32 phy_intf;
	u32 phy_clk = 0;
	u32 ref_clk_out = 0;

	if (base_addr == v_nvt_plat_info[0].base_pa) {
		ETH_QOS_INDEX = 0;
	} else {
		ETH_QOS_INDEX = 1;
	}
	printf("%s: get IO MEM 0x%lx\r\n", __func__, base_addr);

	sprintf(path,"/%s", v_nvt_plat_info[ETH_QOS_INDEX].dtb_name);
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		return -1;
	}
	printf("DTS %s found\r\n", path);

#ifdef CONFIG_TARGET_NA51103
	sprintf(path,"/top@%x/eth",( (u32)(REG32_MASK&IOADDR_TOP_REG_BASE)));
#else
	sprintf(path,"/top@2,%x/eth",( (u32)(REG32_MASK&IOADDR_TOP_REG_BASE)));
#endif

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		printf("%s: path %s not found\n", __func__, path);
		return -1;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n",__func__, __LINE__);
		return -1;
	}
	phy_intf = __be32_to_cpu(cell[0]);

	sprintf(path,"/%s", v_nvt_plat_info[ETH_QOS_INDEX].dtb_name);
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		printf("%s: path %s not found\n", __func__, path);
		return -1;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "sp-clk", &len);
	if (len != 0) {
		phy_clk = __be32_to_cpu(cell[0]);
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "ref-clk-out", &len);
	if (len != 0) {
		ref_clk_out = __be32_to_cpu(cell[0]);
	}
	printf("%s: ref-clk-out %d\r\n", __func__, ref_clk_out);

	if (phy_intf & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii) {
		printf("%s: pinmux detect RMII 0x%x\r\n", __func__, phy_intf);
		ETH_PHY_INTF = PHY_INTERFACE_MODE_RMII;
        pin_num = 1;
	} else if (phy_intf & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rmii_2) {
		printf("%s: pinmux detect RMII_2 0x%x\r\n", __func__, phy_intf);
		ETH_PHY_INTF = PHY_INTERFACE_MODE_RMII;
        pin_num = 2;
	} else if (phy_intf & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii) {
		printf("%s: pinmux detect RGMII 0x%x\r\n", __func__, phy_intf);
		ETH_PHY_INTF = PHY_INTERFACE_MODE_RGMII;
        pin_num = 1;
	} else if (phy_intf & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_rgmii_2) {
		printf("%s: pinmux detect RGMII 0x%x\r\n", __func__, phy_intf);
		ETH_PHY_INTF = PHY_INTERFACE_MODE_RGMII;
        pin_num = 2;
	} else if( phy_intf & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_emb){
		printf("%s: pinmux detect emb phy 0x%x\r\n", __func__, phy_intf);
                ETH_PHY_INTF = PHY_INTERFACE_MODE_MII;
	}
	else {
		printf("%s: pinmux not detect, use emb phy 0x%x\r\n", __func__, phy_intf);
		ETH_PHY_INTF = PHY_INTERFACE_MODE_MII;
	}

	if (phy_intf & v_nvt_plat_info[ETH_QOS_INDEX].pinmux_ext_phy){
		phy_clk = 1;
	}

	//LED ACT MUX
	if (phy_intf & PIN_ETH_CFG_ACT_LED_1ST_PINMUX) {
		UINT reg;

		printf("LED_ACT 1st(JGPIO1) pinmux 0x%x\r\n", (int)phy_intf);
		reg = readl(IOADDR_TOP_REG_BASE + 0x14);
		reg &= ~(0x3);
		writel(reg | 0x1, IOADDR_TOP_REG_BASE + 0x14);

		reg = readl(IOADDR_TOP_REG_BASE + 0xA4);
		reg &= ~(0x2);
		writel(reg , IOADDR_TOP_REG_BASE + 0xA4);
	} else if (phy_intf & PIN_ETH_CFG_ACT_LED_2ND_PINMUX) {
		UINT reg;

		printf("LED_ACT 1nd(DGPIO8) pinmux 0x%x\r\n", (int)phy_intf);
		reg = readl(IOADDR_TOP_REG_BASE + 0x14);
		reg &= ~(0x3);
		writel(reg | 0x2, IOADDR_TOP_REG_BASE + 0x14);

		reg = readl(IOADDR_TOP_REG_BASE + 0xB4);
		reg &= ~(0x100);
		writel(reg , IOADDR_TOP_REG_BASE + 0xB4);
	}

	//LED LINK MUX
	if (phy_intf & PIN_ETH_CFG_LINK_LED_1ST_PINMUX   ) {
		UINT reg;

		printf("LED_LINK 1st(JGPIO2) pinmux 0x%x\r\n", (int)phy_intf);
		reg = readl(IOADDR_TOP_REG_BASE + 0x14);
		reg &= ~(0x30);
		writel(reg | 0x10, IOADDR_TOP_REG_BASE + 0x14);

		reg = readl(IOADDR_TOP_REG_BASE + 0xA4);
		reg &= ~(0x4);
		writel(reg , IOADDR_TOP_REG_BASE + 0xA4);
	} else if (phy_intf & PIN_ETH_CFG_LINK_LED_2ND_PINMUX   ) {
		UINT reg;

		printf("LED_LINK 2nd(DGPIO9) pinmux 0x%x\r\n", (int)phy_intf);
		reg = readl(IOADDR_TOP_REG_BASE + 0x14);
		reg &= ~(0x30);
		writel(reg | 0x20, IOADDR_TOP_REG_BASE + 0x14);

		reg = readl(IOADDR_TOP_REG_BASE + 0xB4);
		reg &= ~(0x200);
		writel(reg , IOADDR_TOP_REG_BASE + 0xB4);
	}

	v_nvt_plat_info[ETH_QOS_INDEX].mac_pinmux_en(ETH_PHY_INTF, pin_num);
	v_nvt_plat_info[ETH_QOS_INDEX].mac_clk_en(ETH_PHY_INTF);	// also assert async reset
	v_nvt_plat_info[ETH_QOS_INDEX].extphy_clk_en(ETH_PHY_INTF, phy_clk, ref_clk_out, pin_num);
#if 0
	if (phy_intf & 0x1) {
		printf("phy interface: RMII\n");
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0x18) |= 0x3000;      /*Enable Pinmux*/
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0xB8) &= ~0x40077F;   /*Enable Pinmux*/
	}

	if (phy_intf & 0x02) {
		printf("phy interface: INTERNAL MII\n");
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0x18) &= ~0x10000000; /*Enable Pinmux*/
	} else if (phy_intf & 0x04) {
		printf("phy interface: EXT PHY CLK\n");
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0x18) |= 0x10000000;  /*Enable Pinmux*/
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0xB8) &= ~0x80;       /*Enable Pinmux*/
	}

	if (phy_intf & 0x10) {
		printf("phy interface: LED1\n");
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0xD0) &= ~0x3;       /*Enable Pinmux*/
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0x18) &= ~0x6000;    /*Enable LED 1*/
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0x18) |= 0x2000;     /*Enable LED 1*/
	} else if (phy_intf & 0x20) {
		printf("phy interface: LED2\n");
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0xD0) &= ~0x40;      /*Enable Pinmux*/
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0x18) &= ~0x6000;    /*Enable LED 2*/
		*(uint32_t*) (IOADDR_TOP_REG_BASE + 0x18) |= 0x4000;     /*Enable LED 2*/
	}
#endif
	udelay(1);

	return 0;
}

static int eqos_initialize(bd_t *bis, unsigned long base_addr)
{
	struct eth_device *dev;
	struct eqos_priv *eqos;
	int ret;
	u8 phyaddr = 0;
	u32 phyval = 0, i;
#ifndef FIXED_ETH_PARAMETER
	u8 default_mac_addr[6] = DEFAULT_MAC_ADDRESS;
#else
	u8 default_mac_addr[6] = CONFIG_ETHADDR;
	u8 default_mac1_addr[6] = CONFIG_ETH1ADDR;
#endif

	printf("%s 0x%lx\n",__func__, base_addr);

	ret = eth_parse_phy_intf(base_addr);
	if (ret < 0) return ret;

	dev = malloc(sizeof *dev);

	if (dev == NULL)
		return -1;

	memset(dev, 0, sizeof *dev);

	if (base_addr == IOADDR_ETH_REG_BASE)
		sprintf(dev->name, "eth0");
	else
		sprintf(dev->name, "eth1");
	dev->iobase = base_addr;
	dev->init = eqos_start;
	dev->halt = eqos_stop;
	dev->send = eqos_send;
	dev->recv = eqos_recv;
	dev->write_hwaddr = eqos_write_hwaddr;
	eth_register(dev);

	eqos = malloc(sizeof *eqos);

	if (eqos == NULL)
		return -1;

	memset(eqos, 0, sizeof *eqos);

	dev->priv = eqos;
	eqos->dev = dev;
	eqos->regs = base_addr;
	eqos->mac_regs = (void *)(eqos->regs + EQOS_MAC_REGS_BASE);
	eqos->mtl_regs = (void *)(eqos->regs + EQOS_MTL_REGS_BASE);
	eqos->dma_regs = (void *)(eqos->regs + EQOS_DMA_REGS_BASE);


	ret = eqos_probe_resources_core(dev);
	if (ret < 0) {
		pr_err("eqos_probe_resources_core() failed: %d", ret);
		return ret;
	}

//	eqos_probe_hwinit();

	eqos->mii = mdio_alloc();
	if (!eqos->mii) {
		pr_err("mdio_alloc() failed");
		goto err_remove_resources_core;
	}
	eqos->mii->read = eqos_mdio_read;
	eqos->mii->write = eqos_mdio_write;
	eqos->mii->priv = eqos;
	strcpy(eqos->mii->name, dev->name);

	ret = mdio_register(eqos->mii);
	if (ret < 0) {
		pr_err("mdio_register() failed: %d", ret);
		goto err_free_mdio;
	}

	if (base_addr == IOADDR_ETH_REG_BASE)
		memcpy(dev->enetaddr, default_mac_addr, 6);
	else
		memcpy(dev->enetaddr, default_mac1_addr, 6);

	// scan phy address
	for (i = 0; i < 32; i++) {
		phyval = eqos_mdio_read(eqos->mii,i, 0, MII_BMSR);
		if (phyval != 0x0000 && phyval != 0xffff) {
			phyaddr = i;
			break;
		}
	}
	eqos->phyaddr = phyaddr;

	eqos->phy = phy_connect(eqos->mii, eqos->phyaddr, dev, 0);
	if (!eqos->phy) {
		pr_err("phy_connect() failed");
		ret = -1;
		goto err_free_phy;
	}

	ret = phy_config(eqos->phy);
	if (ret < 0) {
		pr_err("phy_config() failed: %d", ret);
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_phy:
err_free_mdio:
err_remove_resources_core:
	eqos_remove_resources_core(dev);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}
/*
static void eqos_probe_hwinit(void)
{
	mdc_div = 1;	// APB = 150MHz

	set_led_default(v_nvt_plat_info[0].base_pa);
	set_led_default(v_nvt_plat_info[1].base_pa);
}
*/

int na51103_eth_initialize(bd_t *bis)
{
	int ret = 0;
	u32 i;
	char path[20] = {0};
	int nodeoffset;
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);

	printf("%s %s\n",__func__, ETHVERSION);

	//eqos_probe_hwinit();

	for (i=0; i<ARRAY_SIZE(v_nvt_plat_info); i++) {
		sprintf(path,"/%s", v_nvt_plat_info[i].dtb_name);
		nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
		if (nodeoffset < 0) {
			printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
			printf("%s: path %s not found\n", __func__, path);
			continue;
		}

		printf("%s: dtb node %s found\n", __func__, path);
		eqos_initialize(bis, v_nvt_plat_info[i].base_pa);
	}

	return ret;
}
