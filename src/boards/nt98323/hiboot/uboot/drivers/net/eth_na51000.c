/*
 * Copyright (c) 2017, NOVATEK CORPORATION.
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
 * na51000:
 *    NOVATEK's NA51000 chip. This configuration uses an AXI master/DMA bus, an
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

#include <asm/arch/IOAddress.h>
#include <asm/arch/nvt_types.h>
#include <asm/arch/nvt_common.h>
#include <libfdt.h>

#define ETHVERSION "2.0.0.0"

/* Core registers */
#define NOVATEK_NA51000_EQOS_BASE 0xF02B0000
//#define BIT(x) (1 << (x))
#define DEFAULT_MAC_ADDRESS {0x00, 0x80, 0x48, 0xBA, 0xD1, 0x30}

#define EQOS_MAC_REGS_BASE 0x000
struct eqos_mac_regs {
	uint32_t configuration;				/* 0x000 */
	uint32_t unused_004[(0x070 - 0x004) / 4];	/* 0x004 */
	uint32_t q0_tx_flow_ctrl;			/* 0x070 */
	uint32_t unused_070[(0x090 - 0x074) / 4];	/* 0x074 */
	uint32_t rx_flow_ctrl;				/* 0x090 */
	uint32_t unused_094;				/* 0x094 */
	uint32_t txq_prty_map0;				/* 0x098 */
	uint32_t unused_09c;				/* 0x09c */
	uint32_t rxq_ctrl0;				/* 0x0a0 */
	uint32_t unused_0a4;				/* 0x0a4 */
	uint32_t rxq_ctrl2;				/* 0x0a8 */
	uint32_t unused_0ac[(0x0dc - 0x0ac) / 4];	/* 0x0ac */
	uint32_t us_tic_counter;			/* 0x0dc */
	uint32_t unused_0e0[(0x11c - 0x0e0) / 4];	/* 0x0e0 */
	uint32_t hw_feature0;				/* 0x11c */
	uint32_t hw_feature1;				/* 0x120 */
	uint32_t hw_feature2;				/* 0x124 */
	uint32_t unused_128[(0x200 - 0x128) / 4];	/* 0x128 */
	uint32_t mdio_address;				/* 0x200 */
	uint32_t mdio_data;				/* 0x204 */
	uint32_t unused_208[(0x300 - 0x208) / 4];	/* 0x208 */
	uint32_t address0_high;				/* 0x300 */
	uint32_t address0_low;				/* 0x304 */
};

#define EQOS_MAC_CONFIGURATION_GPSLCE			BIT(23)
#define EQOS_MAC_CONFIGURATION_CST			BIT(21)
#define EQOS_MAC_CONFIGURATION_ACS			BIT(20)
#define EQOS_MAC_CONFIGURATION_WD			BIT(19)
#define EQOS_MAC_CONFIGURATION_JD			BIT(17)
#define EQOS_MAC_CONFIGURATION_JE			BIT(16)
#define EQOS_MAC_CONFIGURATION_PS			BIT(15)
#define EQOS_MAC_CONFIGURATION_FES			BIT(14)
#define EQOS_MAC_CONFIGURATION_DM			BIT(13)
#define EQOS_MAC_CONFIGURATION_TE			BIT(1)
#define EQOS_MAC_CONFIGURATION_RE			BIT(0)

#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT		16
#define EQOS_MAC_Q0_TX_FLOW_CTRL_PT_MASK		0xffff
#define EQOS_MAC_Q0_TX_FLOW_CTRL_TFE			BIT(1)

#define EQOS_MAC_RX_FLOW_CTRL_RFE			BIT(0)

#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT		0
#define EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK		0xff

#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT			0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK			3
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_NOT_ENABLED		0
#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB		2

#define EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT			0
#define EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK			0xff

#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT		6
#define EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK		0x1f
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT		0
#define EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK		0x1f

#define EQOS_MAC_MDIO_ADDRESS_PA_SHIFT			21
#define EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT			16
#define EQOS_MAC_MDIO_ADDRESS_CR_SHIFT			8
#define EQOS_MAC_MDIO_ADDRESS_CR_20_35			4
#define EQOS_MAC_MDIO_ADDRESS_SKAP			BIT(4)
#define EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT			2
#define EQOS_MAC_MDIO_ADDRESS_GOC_READ			3
#define EQOS_MAC_MDIO_ADDRESS_GOC_WRITE			1
#define EQOS_MAC_MDIO_ADDRESS_C45E			BIT(1)
#define EQOS_MAC_MDIO_ADDRESS_GB			BIT(0)

#define EQOS_MAC_MDIO_DATA_GD_MASK			0xffff

#define EQOS_MTL_REGS_BASE 0xd00
struct eqos_mtl_regs {
	uint32_t txq0_operation_mode;			/* 0xd00 */
	uint32_t unused_d04;				/* 0xd04 */
	uint32_t txq0_debug;				/* 0xd08 */
	uint32_t unused_d0c[(0xd18 - 0xd0c) / 4];	/* 0xd0c */
	uint32_t txq0_quantum_weight;			/* 0xd18 */
	uint32_t unused_d1c[(0xd30 - 0xd1c) / 4];	/* 0xd1c */
	uint32_t rxq0_operation_mode;			/* 0xd30 */
	uint32_t unused_d34;				/* 0xd34 */
	uint32_t rxq0_debug;				/* 0xd38 */
};

#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT		16
#define EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK		0x1ff
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT	2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_MASK		3
#define EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED	2
#define EQOS_MTL_TXQ0_OPERATION_MODE_TSF		BIT(1)
#define EQOS_MTL_TXQ0_OPERATION_MODE_FTQ		BIT(0)

#define EQOS_MTL_TXQ0_DEBUG_TXQSTS			BIT(4)
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT		1
#define EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK			3

#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT		20
#define EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK		0x3ff
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT		14
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK		0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT		8
#define EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK		0x3f
#define EQOS_MTL_RXQ0_OPERATION_MODE_EHFC		BIT(7)
#define EQOS_MTL_RXQ0_OPERATION_MODE_RSF		BIT(5)

#define EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT			16
#define EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK			0x7fff
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT		4
#define EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK			3

#define EQOS_DMA_REGS_BASE 0x1000
struct eqos_dma_regs {
	uint32_t mode;					/* 0x1000 */
	uint32_t sysbus_mode;				/* 0x1004 */
	uint32_t unused_1008[(0x1100 - 0x1008) / 4];	/* 0x1008 */
	uint32_t ch0_control;				/* 0x1100 */
	uint32_t ch0_tx_control;			/* 0x1104 */
	uint32_t ch0_rx_control;			/* 0x1108 */
	uint32_t unused_110c;				/* 0x110c */
	uint32_t ch0_txdesc_list_haddress;		/* 0x1110 */
	uint32_t ch0_txdesc_list_address;		/* 0x1114 */
	uint32_t ch0_rxdesc_list_haddress;		/* 0x1118 */
	uint32_t ch0_rxdesc_list_address;		/* 0x111c */
	uint32_t ch0_txdesc_tail_pointer;		/* 0x1120 */
	uint32_t unused_1124;				/* 0x1124 */
	uint32_t ch0_rxdesc_tail_pointer;		/* 0x1128 */
	uint32_t ch0_txdesc_ring_length;		/* 0x112c */
	uint32_t ch0_rxdesc_ring_length;		/* 0x1130 */
};

#define EQOS_DMA_MODE_SWR				BIT(0)

#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT		16
#define EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_MASK		0xf
#define EQOS_DMA_SYSBUS_MODE_EAME			BIT(11)
#define EQOS_DMA_SYSBUS_MODE_AALE			BIT(10)
#define EQOS_DMA_SYSBUS_MODE_BLEN16			BIT(3)
#define EQOS_DMA_SYSBUS_MODE_BLEN8			BIT(2)
#define EQOS_DMA_SYSBUS_MODE_BLEN4			BIT(1)

#define EQOS_DMA_CH0_CONTROL_PBLX8			BIT(16)

#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT		16
#define EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK		0x3f
#define EQOS_DMA_CH0_TX_CONTROL_OSP			BIT(4)
#define EQOS_DMA_CH0_TX_CONTROL_ST			BIT(0)

#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT		16
#define EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK		0x3f
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT		1
#define EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK		0x3fff
#define EQOS_DMA_CH0_RX_CONTROL_SR			BIT(0)

/* Descriptors */

/* We assume ARCH_DMA_MINALIGN >= 16; 16 is the EQOS HW minimum */
#define EQOS_DESCRIPTOR_ALIGN	ARCH_DMA_MINALIGN

#define EQOS_DESCRIPTOR_BYTES   16
#define EQOS_DESCRIPTOR_DUMMY	(ARCH_DMA_MINALIGN - EQOS_DESCRIPTOR_BYTES)
#define EQOS_DESCRIPTOR_SIZE	(EQOS_DESCRIPTOR_BYTES + EQOS_DESCRIPTOR_DUMMY)

#define EQOS_DESCRIPTORS_TX	4
#define EQOS_DESCRIPTORS_RX	16
#define EQOS_DESCRIPTORS_NUM	(EQOS_DESCRIPTORS_TX + EQOS_DESCRIPTORS_RX)
#define EQOS_DESCRIPTORS_SIZE	ALIGN(EQOS_DESCRIPTORS_NUM * \
				      EQOS_DESCRIPTOR_SIZE, ARCH_DMA_MINALIGN)
#define EQOS_BUFFER_ALIGN	ARCH_DMA_MINALIGN
#define EQOS_MAX_PACKET_SIZE	ALIGN(1568, ARCH_DMA_MINALIGN)
#define EQOS_RX_BUFFER_SIZE	(EQOS_DESCRIPTORS_RX * EQOS_MAX_PACKET_SIZE)

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
/*
#if EQOS_DESCRIPTOR_SIZE < ARCH_DMA_MINALIGN
#if !defined(CONFIG_SYS_NONCACHED_MEMORY) && \
	!defined(CONFIG_SYS_DCACHE_OFF) && !defined(CONFIG_X86)
#warning Cache line size is larger than descriptor size
#endif
#endif
*/
struct eqos_desc {
	u32 des0;
	u32 des1;
	u32 des2;
	u32 des3;
	u32 dummy0;
	u32 dummy1;
	u32 dummy2;
	u32 dummy3;
	u32 dummy4;
	u32 dummy5;
	u32 dummy6;
	u32 dummy7;
	u32 dummy8;
	u32 dummy9;
	u32 dummy10;
	u32 dummy11;
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
	uint32_t regs;
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
};

static u8 mdc_div = 1;
static u32 phy_intf = 0;

// RTL8201F
#define RTL8201F_UUID             0x001CC816
#define RTL8201F_PAGESEL_REG      0x1F
#define RTL8201F_DEFAULT_PAGE     0x00
#define RTL8201F_RMII_PAGE        0x07
#define RTL8201F_RMII_REG         0x10
#define RTL8201F_RXTIMING_OFFSET  4
#define RTL8201F_RXTIMING_VALUE   4
#define RTL8201F_RXTIMING_MASK    0xF

// RTL8211E
#define RTL8211E_UUID          0x001CC915
#define RTL8211E_PAGESEL_REG   0x1F
#define RTL8211E_PAGESEL1_REG  0x1E
#define RTL8211E_DEFAULT_PAGE  0x00
#define RTL8211E_DELAY_PAGE    0x07
#define RTL8211E_DELAY1_PAGE   0xA4
#define RTL8211E_DELAY_REG     0x28
#define RTL8211E_DELAY_OFFSET  13
#define RTL8211E_TXDLY_OFFSET  12
#define RTL8211E_RXDLY_OFFSET  11

// RTL8211F
#define RTL8211F_UUID          0x001CC916
#define RTL8211F_PAGESEL_REG   0x1F
#define RTL8211F_DEFAULT_PAGE  0x00
#define RTL8211F_DELAY_PAGE    0xD08
#define RTL8211F_TXDLY_REG     0x11
#define RTL8211F_TXDLY_OFFSET  8
#define RTL8211F_RXDLY_REG     0x15
#define RTL8211F_RXDLY_OFFSET  3

// Atheros 8035
#define AT8035_UUID            0x004dd072
#define AT803X_DEBUG_ADDR_REG  0x1D
#define AT803X_DEBUG_DATA_REG  0x1E
#define AT803X_TXDLY_REG       0x05
#define AT803X_TXDLY_OFFSET	   8
#define AT803X_RXDLY_REG       0x00
#define AT803X_RXDLY_OFFSET	   15

// ICPlus1001
#define IP1001_UUID            0x02430d90
#define IP10XX_CTRL_STATUS     0x10
#define IP1001_RXPHASE_OFFSET  0
#define IP1001_TXPHASE_OFFSET  1

#if 0
static void dump_desc(struct eth_device *eth_dev)
{
	struct eqos_priv *eqos = eth_dev->priv;
    int i;

    printf("ch0_txdesc_list_haddress: 0x1110=%#x\n", *(uint32_t*)0xf02b1110);
    printf("ch0_txdesc_list_address:  0x1114=%#x\n", *(uint32_t*)0xf02b1114);
    printf("ch0_rxdesc_list_haddress: 0x1118=%#x\n", *(uint32_t*)0xf02b1118);
    printf("ch0_rxdesc_list_address:  0x111c=%#x\n", *(uint32_t*)0xf02b111c);
    printf("ch0_txdesc_tail_pointer:  0x1120=%#x\n", *(uint32_t*)0xf02b1120);
    printf("ch0_rxdesc_tail_pointer:  0x1128=%#x\n", *(uint32_t*)0xf02b1128);
    printf("ch0_txdesc_ring_length:   0x112c=%#x\n", *(uint32_t*)0xf02b112c);
    printf("ch0_rxdesc_ring_length:   0x1130=%#x\n", *(uint32_t*)0xf02b1130);

    printf("Current_App_TxDesc        0x1144=%#x\n", *(uint32_t*)0xf02b1144);
    printf("Current_App_RxDesc        0x114C=%#x\n", *(uint32_t*)0xf02b114C);
    printf("Current_App_TxBuffer_H    0x1150=%#x\n", *(uint32_t*)0xf02b1150);
    printf("Current_App_TxBuffer      0x1154=%#x\n", *(uint32_t*)0xf02b1154);
    printf("Current_App_RxBuffer_H    0x1158=%#x\n", *(uint32_t*)0xf02b1158);
    printf("Current_App_RxBuffer      0x115C=%#x\n", *(uint32_t*)0xf02b115C);
    printf("DMA_CH_Status:            0x1160=%#x\n", *(uint32_t*)0xf02b1160);

    printf("eqos->tx_desc_idx = %d\n", eqos->tx_desc_idx);
	for (i = 0; i < EQOS_DESCRIPTORS_TX; i++) {
		struct eqos_desc *tx_desc = &(eqos->tx_descs[i]);
        printf("Tx [%d]=%#x ==> ",i,(u32)tx_desc);
        printf("%#x:%#x:%#x:%#x\n",tx_desc->des0,tx_desc->des1,tx_desc->des2,tx_desc->des3);

	}

    printf("eqos->rx_desc_idx = %d\n", eqos->rx_desc_idx);
	for (i = 0; i < EQOS_DESCRIPTORS_RX; i++) {
		struct eqos_desc *rx_desc = &(eqos->rx_descs[i]);
        printf("Rx [%d]=%#x ==> ",i,(u32)rx_desc);
        printf("%#x:%#x:%#x:%#x\n",rx_desc->des0,rx_desc->des1,rx_desc->des2,rx_desc->des3);
	}
}
#endif

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
//#define CONFIG_SYS_NONCACHED_MEMORY

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
	flush_cache((unsigned long)buf, size);
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
	int i = 0, ret = -1;
	u32 reg;

	writel(EQOS_DMA_MODE_SWR, &eqos->dma_regs->mode);

	do {
		reg = readl(&eqos->dma_regs->mode);
		if (!(reg & EQOS_DMA_MODE_SWR)) {
			ret = 0;
			break;
		}
		i++;
	} while (i < 1000);

	if (phy_intf == 0x04) {              // GMII
		*(u32*) 0xF02B3000 |= 0xE;       /*Set Tx Delay*/
		*(u32*) 0xF02B3004 |= 0x80;      /*Set tx_clk inv*/
	} if (phy_intf == 0x08) {            // RGMII
		*(u32*) 0xF02B3000 |= 0x5;        /*Set Tx Delay*/
	}
	return ret;
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
		error("MDIO not idle at entry");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(mdc_div << EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_READ << EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(10);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		error("MDIO read didn't complete");
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
		error("MDIO not idle at entry");
		return ret;
	}

	writel(mdio_val, &eqos->mac_regs->mdio_data);

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(mdc_div <<  EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_WRITE << EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(10);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		error("MDIO read didn't complete");
		return ret;
	}

	return 0;
}

static void eqos_phy_rgmii_init(struct phy_device *phydev)
{
	struct mii_dev *bus = phydev->bus;
    uint32_t val;

    if (phydev->phy_id == RTL8211E_UUID)
    {
        printf("%s RTL8211E_UUID=0x%x\n",__func__,phydev->phy_id);
        // set Page
        bus->write(bus, phydev->addr, 0x00, RTL8211E_PAGESEL_REG, RTL8211E_DELAY_PAGE);
        bus->write(bus, phydev->addr, 0x00, RTL8211E_PAGESEL1_REG, RTL8211E_DELAY1_PAGE);

        val = bus->read(bus, phydev->addr, 0x00, RTL8211E_DELAY_REG);
        val = val | (1<<RTL8211E_DELAY_OFFSET) | (1<<RTL8211E_TXDLY_OFFSET) | (1<<RTL8211E_RXDLY_OFFSET);
        bus->write(bus, phydev->addr, 0x00, RTL8211E_DELAY_REG, val);

        // set Page
        bus->write(bus, phydev->addr, 0x00, RTL8211E_PAGESEL_REG, RTL8211E_DEFAULT_PAGE);
    }
    else if (phydev->phy_id == RTL8211F_UUID)
    {
        printf("%s RTL8211F_UUID=0x%x\n",__func__,phydev->phy_id);
        // set Page
        bus->write(bus, phydev->addr, 0x00, RTL8211F_PAGESEL_REG, RTL8211F_DELAY_PAGE);

        // TXDLY
        val = bus->read(bus, phydev->addr, 0x00, RTL8211F_TXDLY_REG);
        val = val | (1<<RTL8211F_TXDLY_OFFSET);
        bus->write(bus, phydev->addr, 0x00, RTL8211F_TXDLY_REG, val);

        // RXDLY
        val = bus->read(bus, phydev->addr, 0x00, RTL8211F_RXDLY_REG);
        val = val | (1<<RTL8211F_RXDLY_OFFSET);
        bus->write(bus, phydev->addr, 0x00, RTL8211F_RXDLY_REG, val);

        // set Page
        bus->write(bus, phydev->addr, 0x00, RTL8211F_PAGESEL_REG, RTL8211F_DEFAULT_PAGE);
    }
    else if ( phydev->phy_id == AT8035_UUID ) {
        printf("%s AT8035_UUID=0x%x\n",__func__,phydev->phy_id);
        // TXDLY
        bus->write(bus, phydev->addr, 0x00, AT803X_DEBUG_ADDR_REG, AT803X_TXDLY_REG);
        bus->write(bus, phydev->addr, 0x00, AT803X_DEBUG_DATA_REG, (1<<AT803X_TXDLY_OFFSET));

        // RXDLY
        bus->write(bus, phydev->addr, 0x00, AT803X_DEBUG_ADDR_REG, AT803X_RXDLY_REG);
        bus->write(bus, phydev->addr, 0x00, AT803X_DEBUG_DATA_REG, (1<<AT803X_RXDLY_OFFSET));
    }
    else if ( phydev->phy_id == IP1001_UUID ) {
        printf("%s IP1001_UUID=0x%x\n",__func__,phydev->phy_id);
        val = bus->read(bus, phydev->addr, 0x00, IP10XX_CTRL_STATUS);
        val |= ((1<<IP1001_TXPHASE_OFFSET) | (1<<IP1001_RXPHASE_OFFSET));
        bus->write(bus, phydev->addr, 0x00, IP10XX_CTRL_STATUS, val);
    }
    else {
        printf("UUID=0x%x is not support\n",phydev->phy_id);
    }
}

#define RTL8201F_UUID             0x001CC816
#define RTL8201F_PAGESEL_REG      0x1F
#define RTL8201F_DEFAULT_PAGE     0x00
#define RTL8201F_RMII_PAGE        0x07
#define RTL8201F_RMII_REG         0x10
#define RTL8201F_RXTIMING_OFFSET  4
#define RTL8201F_RXTIMING_VALUE   4
#define RTL8201F_RXTIMING_MASK    0xF

static void eqos_phy_rmii_init(struct phy_device *phydev)
{
    struct mii_dev *bus = phydev->bus;
    uint32_t val;

    if (phydev->phy_id == RTL8201F_UUID)
    {
        printf("%s RTL8201F_UUID=0x%x\n",__func__,phydev->phy_id);
        // set Page
        bus->write(bus, phydev->addr, 0x00, RTL8201F_PAGESEL_REG, RTL8201F_RMII_PAGE);

        val = bus->read(bus, phydev->addr, 0x00, RTL8201F_RMII_REG);
        debug("%s %#x\n",__func__,val);
        val = val & ~(RTL8201F_RXTIMING_MASK<<RTL8201F_RXTIMING_OFFSET);
        val = val | (RTL8201F_RXTIMING_VALUE<<RTL8201F_RXTIMING_OFFSET);
        bus->write(bus, phydev->addr, 0x00, RTL8201F_RMII_REG, val);

        val = bus->read(bus, phydev->addr, 0x00, RTL8201F_RMII_REG);
        debug("%s %#x\n",__func__,val);
        // set Page
        bus->write(bus, phydev->addr, 0x00, RTL8201F_PAGESEL_REG, RTL8201F_DEFAULT_PAGE);
    }
    else {
        printf("UUID=0x%x\n",phydev->phy_id);
    }
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
		error("eqos_set_*_duplex() failed: %d", ret);
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
		error("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}
	if (ret < 0) {
		error("eqos_set_*mii_speed*() failed: %d", ret);
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

	printf("%s(eth_dev=%p):\n", __func__, eth_dev);

	eqos->tx_desc_idx = 0;
	eqos->rx_desc_idx = 0;
	eqos->reg_access_ok = true;

	// Check DMA SW reset
	ret = eqos_dma_wait_swrst(eqos);
	if (ret) {
		error("EQOS_DMA_MODE_SWR stuck");
		goto err;
	}

	ret = phy_startup(eqos->phy);
	if (ret < 0) {
		ret = -1;
		error("phy_startup() failed: %d", ret);
		goto err;
	}

	if (!eqos->phy->link) {
		ret = -1;
		error("No link");
		goto err;
	}

	ret = eqos_adjust_link(eth_dev);
	if (ret < 0) {
		error("eqos_adjust_link() failed: %d", ret);
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
		     (EQOS_DMA_CH0_CONTROL_PBLX8 | ((EQOS_DESCRIPTOR_DUMMY/16)<<18)));

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
        eqos_inval_buffer((void*)rx_desc->des0, EQOS_MAX_PACKET_SIZE);
	}
	flush_cache((unsigned long)eqos->descs, EQOS_DESCRIPTORS_SIZE);

	writel(0, &eqos->dma_regs->ch0_txdesc_list_haddress);
	writel((ulong)eqos->tx_descs, &eqos->dma_regs->ch0_txdesc_list_address);
	writel(EQOS_DESCRIPTORS_TX - 1,
	       &eqos->dma_regs->ch0_txdesc_ring_length);

	writel(0, &eqos->dma_regs->ch0_rxdesc_list_haddress);
	writel((ulong)eqos->rx_descs, &eqos->dma_regs->ch0_rxdesc_list_address);
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
	error("FAILED: %d", ret);
	return ret;
}

void eqos_stop(struct eth_device *eth_dev)
{
	struct eqos_priv *eqos = eth_dev->priv;
	int i;

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

int eqos_send(struct eth_device *eth_dev, void *packet, int length)
{
	struct eqos_priv *eqos = eth_dev->priv;
	struct eqos_desc *tx_desc;
	int i;

	debug("%s(eth_dev=%p, packet=%p, length=%d):\n", __func__, eth_dev, packet,
	      length);

	memcpy(eqos->tx_dma_buf, packet, length);
	eqos_flush_buffer(packet, length);
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
			//printf("s\n");
			return 0;
        }
		udelay(1);
	}

	debug("%s: TX timeout\n", __func__);

	return -ETIMEDOUT;
}

int eqos_free_pkt(struct eth_device *eth_dev, uchar *packet, int length)
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


int eqos_recv(struct eth_device *eth_dev)
{
	struct eqos_priv *eqos = eth_dev->priv;
	struct eqos_desc *rx_desc;
	int length;
	uint32_t packetp;

	debug("%s(dev=%p: des indx 0x%x\n", __func__, eth_dev, eqos->rx_desc_idx);

	rx_desc = &(eqos->rx_descs[eqos->rx_desc_idx]);
	eqos_inval_buffer((void*)rx_desc, EQOS_DESCRIPTOR_SIZE);

	if (rx_desc->des3 & EQOS_DESC3_OWN) {
		debug("%s: RX packet not available\n", __func__);
		return -EAGAIN;
	}

	packetp = (u32) eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	length = rx_desc->des3 & 0x7fff;

	eqos_inval_buffer((void*)packetp, EQOS_MAX_PACKET_SIZE);
	mb();
	net_process_received_packet((void*)packetp, length);

	eqos_free_pkt(eth_dev, (void*)packetp, length);
	//printf("r(%d)\n",eqos->rx_desc_idx-1);

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

	eqos->rx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_RX_BUFFER_SIZE);
	if (!eqos->rx_dma_buf) {
		debug("%s: memalign(rx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_dma_buf;
	}
	debug("%s: rx_dma_buf=%p\n", __func__, eqos->rx_dma_buf);

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

static void eqos_probe_hwinit(void)
{
	u32 varCGSys = *(uint32_t*)0xF0020010;

	if(((varCGSys>>4)&0x3) == 2) {  // 160 MHz
		mdc_div = 5;
	} else if (((varCGSys>>4)&0x3) == 1) { // 120 MHz
		mdc_div = 4;
	} else {
		mdc_div = 1;
	}

	//Set global items
	udelay(10);
	*(uint32_t*) 0xF0020000 |= 0x1000;      /*Enable PLL12*/
	udelay(10);
	*(uint32_t*) 0xF0020084 |= 0x20000000;  /*Enable Clock*/
	udelay(10);
	*(uint32_t*) 0xF00200B4 |= 0x1000000;   /*Release AXIB reset*/
	udelay(10);
	*(uint32_t*) 0xF00200A4 &= ~0x20000000; /*Toggle Ethernet reset*/
	udelay(10);
	*(uint32_t*) 0xF00200A4 |= 0x20000000;  /*Release Ethernet reset*/
	udelay(10);
	*(uint32_t*) 0xF0020104 &= ~0x20000000; /*Release Ethernet SRAM shutdown*/
	mdelay(1);
}

#if 0
static void eqos_debug_info(void)
{
    printf("0xF0010018 = %#x\n", 	*(uint32_t*) 0xF0010018);
    printf("0xF02B0000 = %#x\n", 	*(uint32_t*) 0xF02B0000);
    printf("0xF02B011C = %#x\n", 	*(uint32_t*) 0xF02B011C);
    printf("0xF02B0120 = %#x\n", 	*(uint32_t*) 0xF02B0120);
    printf("0xF02B0124 = %#x\n", 	*(uint32_t*) 0xF02B0124);
}
#endif

static void eth_parse_phy_intf(void)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int len;

	sprintf(path,"/top@%x/eth",IOADDR_TOP_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		return;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n",__func__, __LINE__);
		return;
	}

	phy_intf = __be32_to_cpu(cell[0]);

	// set pinmux
	if (phy_intf == 0x01) {
		printf("phy interface: MII\n");
		*(uint32_t*) 0xF0010018 |= 0x10000;      /*Enable MII*/
		*(uint32_t*) 0xF00100B8 &= ~0x37FFF800;  /*Set Pinmux*/
	} else if (phy_intf == 0x02) {
		printf("phy interface: RMII\n");
		*(uint32_t*) 0xF0010018 |= 0x20000;      /*Enable RMII*/
		*(uint32_t*) 0xF00100B8 &= ~0x38CA7000;  /*Set Pinmux*/
	} else if (phy_intf == 0x04) {
		printf("phy interface: GMII\n");
		*(uint32_t*) 0xF0010018 |= 0x30000;      /*Enable GMII*/
		*(uint32_t*) 0xF00100B8 &= ~0x3FFFFF8F;  /*Set Pinmux*/
	} else if (phy_intf == 0x08) {
		printf("phy interface: RGMII\n");
		*(uint32_t*) 0xF0010018 |= 0x40000;      /*Enable RGMII*/
		*(uint32_t*) 0xF00100B8 &= ~0x3FE1F000;  /*Set Pinmux*/
		*(uint32_t*) 0xF003005C = 0x55400055;	 /*Increase pin current driving*/
		*(uint32_t*) 0xF0030060 = 0x400001;	 /*Increase pin current driving*/
	} else {
		printf("phy interface: NONE!!\n");
	}
	udelay(1);
}

int na51000_eth_initialize(bd_t *bis)
{
	struct eth_device *dev;
	struct eqos_priv *eqos;
	int ret;
	u8 phyaddr = 0;
	u32 phyval = 0, i;
#ifndef CONFIG_FIXED_ETH_PARAMETER
	u8 default_mac_addr[6] = DEFAULT_MAC_ADDRESS;
#else
	u8 default_mac_addr[6] = CONFIG_ETHADDR;
#endif
	printf("%s %s\n",__func__, ETHVERSION);

	eth_parse_phy_intf();

	dev = malloc(sizeof *dev);

	if (dev == NULL)
		return -1;

	memset(dev, 0, sizeof *dev);

	sprintf(dev->name, "eth_na51000");
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
	eqos->regs = NOVATEK_NA51000_EQOS_BASE;
	eqos->mac_regs = (void *)(eqos->regs + EQOS_MAC_REGS_BASE);
	eqos->mtl_regs = (void *)(eqos->regs + EQOS_MTL_REGS_BASE);
	eqos->dma_regs = (void *)(eqos->regs + EQOS_DMA_REGS_BASE);

	ret = eqos_probe_resources_core(dev);
	if (ret < 0) {
		error("eqos_probe_resources_core() failed: %d", ret);
		return ret;
	}

	eqos_probe_hwinit();
	//eqos_debug_info();

	eqos->mii = mdio_alloc();
	if (!eqos->mii) {
		error("mdio_alloc() failed");
		goto err_remove_resources_core;
	}
	eqos->mii->read = eqos_mdio_read;
	eqos->mii->write = eqos_mdio_write;
	eqos->mii->priv = eqos;
	strcpy(eqos->mii->name, dev->name);

	ret = mdio_register(eqos->mii);
	if (ret < 0) {
		error("mdio_register() failed: %d", ret);
		goto err_free_mdio;
	}

	memcpy(dev->enetaddr, default_mac_addr, 6);

	// scan phy address
	for (i=0;i<32;i++) {
		phyval = eqos_mdio_read(eqos->mii,i,0,MII_BMSR);
		if (phyval != 0x0000 && phyval != 0xffff) {
			phyaddr = i;
			break;
		}
	}

	eqos->phy = phy_connect(eqos->mii, phyaddr, dev, 0);
	if (!eqos->phy) {
		error("phy_connect() failed");
	}

	ret = phy_config(eqos->phy);
	if (ret < 0) {
		error("phy_config() failed: %d", ret);
	}

	if (phy_intf == 0x8) { // RGMII
		eqos_phy_rgmii_init(eqos->phy);
	} else if (phy_intf == 0x2) { // RMII
		eqos_phy_rmii_init(eqos->phy);
	}

	ret = phy_startup(eqos->phy);
	if (ret < 0) {
		error("phy_startup() failed: %d", ret);
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_mdio:
err_remove_resources_core:
	eqos_remove_resources_core(dev);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}
