// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __CHIP_REGS_H__
#define __CHIP_REGS_H__

/* -------------------------------------------------------------------- */
/* SRAM */
/* -------------------------------------------------------------------- */
#define SRAM_BASE			0x04020000
#define SRAM_SIZE			0x1A000
#define SRAM_END			(SRAM_BASE + SRAM_SIZE)

/* -------------------------------------------------------------------- */
/* Communication Register and flag used by bootrom */
/* -------------------------------------------------------------------- */
#define REG_START_FLAG			(SYS_CTRL_REG_BASE + REG_SC_GEN1)
#define START_MAGIC			(0x444f574e)
#define SELF_BOOT_TYPE_USBDEV           0x2
#define CP_STEP1_ADDR                   (SRAM_BASE + 0xa00)
#define PCIE_SLAVE_BOOT_CTL_REG         0x0134
#define DDR_INIT_DOWNLOAD_OK_FLAG       0xDCDFF001 /* step1:Ddrinit Code Download Finished Flag:  DCDFF001 */
#define DDR_INIT_EXCUTE_OK_FLAG         0xDCEFF002 /* step2:Ddrinit Code Excute Finished Flag:    DCEFF002 */
#define UBOOT_DOWNLOAD_OK_FLAG          0xBCDFF003 /* step3:Boot Code Download Finished Flag:     BCDFF003 */

/* -------------------------------------------------------------------- */
/* System Control */
/* -------------------------------------------------------------------- */
#define SYS_CTRL_REG_BASE			0x11020000
#define REG_BASE_SCTL				SYS_CTRL_REG_BASE
#define REG_SC_CTRL				0x0000
#define REG_SC_SYSRES				0x0004
#define REG_PERISTAT				0x0030
#define REG_SYSSTAT				0x0018
#define REG_DATA_CHANNEL_TYPE                   0x31c
#define EMMC_BOOT_8BIT				(0x1 << 11)
#define REG_PERI_EMMC_STAT			0x0404
#define mmc_boot_clk_sel(val)           	((val) & 0x3)
#define MMC_BOOT_CLK_50M                	0x2
#define EMMC_NORMAL_MODE			(0x1 << 3)
#define get_spi_nor_addr_mode(_reg)		(((_reg) >> 11) & 0x1)
#define get_spi_device_type(_reg)		(((_reg) >> 2) & 0x1)
#define get_sys_boot_mode(_reg)			(((_reg) >> 2) & 0x3)
#define BOOT_FROM_SPI				0
#define BOOT_FROM_SPI_NAND			1
#define BOOT_FROM_NAND				2
#define BOOT_FROM_EMMC				3

#define SEC_UBOOT_DATA_ADDR			0x41000000
#define OTP_USER_LOCKABLE14			0x10122090
#define get_sec_boot_mode(x)			((((x) & 0xf) == 0x5) ? 0 : 1)

#define REG_SC_GEN1				0x013c
#define REG_SC_GEN2				0x0140
#define REG_SC_GEN3				0x0144
#define REG_SC_GEN4				0x0148
#define REG_SC_GEN9				0x0154
#define REG_SC_GEN20				0x0090

/* -------------------------------------------------------------------- */
/* CPU SUBSYS */
/* -------------------------------------------------------------------- */
#define REG_CRG_CLUSTER0_CLK_RST	0x0190 /* CPU SUBSYS clock and reset control */
#define CLUSTER0_GLB_SRST_REQ	        (0x1 << 17)

#define REG_PERI_CPU_RVBARADDR_SOC	0x12030020

#define REG_CRG_CLUSTER1_CLK_RST	0x0194
#define CLUSTER1_GLB_SRST_REQ           (0x1 << 9)
#define CLUSTER1_GLB_CKEN               (0x1 << 8)

/* -------------------------------------------------------------------- */
/* CRG */
/* -------------------------------------------------------------------- */
#define CRG_REG_BASE			        0x11010000

/* -------------------------------------------------------------------- */
/* Peripheral Control REG */
/* -------------------------------------------------------------------- */
#define MISC_REG_BASE			        0x11024000

/* -------------------------------------------------------------------- */
/* IO configuration REG:mux and driver */
/* -------------------------------------------------------------------- */
#define AHB_IO_CONFIG_REG_BASE			0x11180000

/* -------------------------------------------------------------------- */
/* TIMER */
/* -------------------------------------------------------------------- */
#define TIMER0_REG_BASE			        0x11000000
#define REG_TIMER_RELOAD		        0x0
#define REG_TIMER_VALUE			        0x4
#define REG_TIMER_CONTROL		        0x8
#define CFG_TIMER_CLK                   	(3000000)
#define CFG_TIMERBASE                   	TIMER0_REG_BASE
/* enable timer.32bit, periodic,mask irq, 1 divider. */
#define CFG_TIMER_CTRL                  	0xC2

/* -------------------------------------------------------------------- */
/* UART */
/* -------------------------------------------------------------------- */
#define UART0_REG_BASE			        0x11040000
#define UART1_REG_BASE			        0x11041000
#define UART2_REG_BASE			        0x11042000
#define UART3_REG_BASE			        0x11043000

#define REG_UART0_CRG				(CRG_REG_BASE + 0x4180)

#define UART_CKL_SEL_MASK			0x3000
#define UART_CLK_24M				0x1000
#define BIT_UART_CKEN				0x10
#define BIT_UART_SRST				0x1

/* -------------------------------------------------------------------- */
/* DDRC */
/* -------------------------------------------------------------------- */
#define DDRC0_REG_BASE			        0x11140000
#define DDR_MEM_BASE			        0x40000000

/* -------------------------------------------------------------------- */
/* FMC */
/* -------------------------------------------------------------------- */
#define FMC_REG_BASE			        0x10000000
#define FMC_MEM_BASE			        0x0f000000

/* FMC CRG register offset */
#define FMC_CLK_REG				0x3F40
#define REG_FMC_CRG				FMC_CLK_REG

#define fmc_clk_sel(_clk)			(((_clk) & 0x7) << 12)
#define FMC_CLK_SEL_MASK			(0x7 << 12)
#define get_fmc_clk_type(_reg)			(((_reg) >> 12) & 0x7)

/* SDR/DDR clock */
#define FMC_CLK_24M				0
#define FMC_CLK_100M				1
#define FMC_CLK_150M				3
#define FMC_CLK_200M				4
/* Only DDR clock */
#define FMC_CLK_250M				5
#define FMC_CLK_300M				6
#define FMC_CLK_400M				7

#define FMC_CLK_ENABLE				BIT(4)
#define FMC_SOFT_RST_REQ		    	BIT(0)

/*--------------------------------------------------------------------- */
/* EMMC / SD */
/* -------------------------------------------------------------------- */

/* eMMC CRG register offset */
#define REG_EMMC_CRG				(CRG_REG_BASE + 0x34c0)
#define mmc_clk_sel(_clk)			(((_clk) & 0x7) << 24)
#define MMC_CLK_SEL_MASK			(0x7 << 24)
#define REG_SAVE_HCS				0x11020300

/* EMMC REG */
#define EMMC_BASE_REG			        0x10020000
#define NO_EMMC_PHY				1

#define NF_BOOTBW_MASK				(1 << 11)
#define REG_BASE_PERI_CTRL			REG_BASE_SCTL
#define REG_BASE_IO_CONFIG			IO_CONFIG_REG_BASE

/* SDIO0 REG */
#define REG_SDIO0_CRG				(CRG_REG_BASE + 0x35c0)
#define SDIO0_BASE_REG			        0x10030000

/*--------------------------------------------------------------------- */
/* GMAC */
/* -------------------------------------------------------------------- */
#define GMAC0_IOBASE			0x10290000
#define GMAC1_IOBASE			0x102A0000

/* Ethernet MAC0 MAC_IF CRG register offset */
#define REG_ETH0_MACIF_CRG              0x37c0
/* Ethernet MAC0 MAC_If CRG register bit map */
#define BIT_MACIF0_RST                  BIT(0)
#define BIT_GMACIF0_CLK_EN              BIT(4)
#define BIT_RMII0_CLKSEL_PAD            BIT(12)

/* Ethernet MAC0 GSF CRG register offset */
#define REG_ETH0_GSF_CRG                0x37c4
/* Ethernet MAC0 GSF CRG register bit map */
#define BIT_GMAC0_RST                   BIT(0)
#define BIT_GMAC0_CLK_EN                BIT(4)

/* Ethernet MAC0 PHY CRG register offset */
#define REG_ETH0_PHY_CRG                0x37cc
/* Ethernet MAC0 PHY CRG register bit map */
#define BIT_EXT_PHY0_RST		BIT(0)
#define BIT_EXT_PHY0_CLK_SELECT		BIT(12)


/* Ethernet MAC1 MAC_IF CRG register offset */
#define REG_ETH1_MACIF_CRG              0x3800
/* Ethernet MAC1 MAC_If CRG register bit map */
#define BIT_MACIF1_RST                  BIT(0)
#define BIT_GMACIF1_CLK_EN              BIT(4)
#define BIT_RMII1_CLKSEL_PAD            BIT(12)

/* Ethernet MAC1 GSF CRG register offset */
#define REG_ETH1_GSF_CRG                0x3804
/* Ethernet MAC1 GSF CRG register bit map */
#define BIT_GMAC1_RST                   BIT(0)
#define BIT_GMAC1_CLK_EN                BIT(4)

/* Ethernet MAC1 PHY CRG register offset */
#define REG_ETH1_PHY_CRG                0x380c
/* Ethernet MAC1 PHY CRG register bit map */
#define BIT_EXT_PHY1_RST		BIT(0)
#define BIT_EXT_PHY1_CLK_SELECT		BIT(12)

#define PHY0_CLK_25M			0
#define PHY0_CLK_50M			BIT_EXT_PHY0_CLK_SELECT
#define PHY1_CLK_25M			0
#define PHY1_CLK_50M			BIT_EXT_PHY1_CLK_SELECT

#define GMAC_MACIF0_CTRL		(GMAC0_IOBASE + 0x300c)
#define GMAC_MACIF1_CTRL		(GMAC1_IOBASE + 0x300c)
#define GMAC_DUAL_MAC_CRF_ACK_TH	(GMAC0_IOBASE + 0x3004)

/* Configure gmac pinctrl parameters in software */
#define CFG_NET_PINCTRL

/* MDIO0 pinctrl phyical addr */
#define PHY_ADDR_MDCK0			0x17CA0068
#define PHY_ADDR_MDIO0			0x17CA006C
/* MDIO1 pinctrl phyical addr */
#define PHY_ADDR_MDCK1			0x17CA00AC
#define PHY_ADDR_MDIO1			0x17CA00B0

/* PHY0 pinctrl phyical addr */
#define PHY_ADDR_EPHY0_CLK		0x17CA0070
#define PHY_ADDR_EPHY0_RSTN		0x17CA0074
/* PHY1 pinctrl phyical addr */
#define PHY_ADDR_EPHY1_CLK		0x17CA00B4
#define PHY_ADDR_EPHY1_RSTN		0x17CA00B8

/* RGMII0 pinctrl phyical addr */
#define PHY_ADDR_RGMII0_TXCKOUT		0x17CA00A4
#define PHY_ADDR_RGMII0_TXD0		0x17CA00A0
#define PHY_ADDR_RGMII0_TXD1		0x17CA009C
#define PHY_ADDR_RGMII0_TXD2		0x17CA0098
#define PHY_ADDR_RGMII0_TXD3		0x17CA0094
#define PHY_ADDR_RGMII0_TXEN		0x17CA0090
#define PHY_ADDR_RGMII0_RXCK		0x17CA008C
#define PHY_ADDR_RGMII0_RXD0		0x17CA0088
#define PHY_ADDR_RGMII0_RXD1		0x17CA0084
#define PHY_ADDR_RGMII0_RXD2		0x17CA0080
#define PHY_ADDR_RGMII0_RXD3		0x17CA007C
#define PHY_ADDR_RGMII0_RXDV		0x17CA0078
/* RGMII1 pinctrl phyical addr */
#define PHY_ADDR_RGMII1_TXCKOUT	0x17CA00E8
#define PHY_ADDR_RGMII1_TXD0	0x17CA00E4
#define PHY_ADDR_RGMII1_TXD1	0x17CA00E0
#define PHY_ADDR_RGMII1_TXD2	0x17CA00DC
#define PHY_ADDR_RGMII1_TXD3	0x17CA00D8
#define PHY_ADDR_RGMII1_TXEN	0x17CA00D4
#define PHY_ADDR_RGMII1_RXCK	0x17CA00D0
#define PHY_ADDR_RGMII1_RXD0	0x17CA00CC
#define PHY_ADDR_RGMII1_RXD1	0x17CA00C8
#define PHY_ADDR_RGMII1_RXD2	0x17CA00C4
#define PHY_ADDR_RGMII1_RXD3	0x17CA00C0
#define PHY_ADDR_RGMII1_RXDV	0x17CA00BC

/* RMII0 pinctrl phyical addr */
#define PHY_ADDR_RMII0_CLK		0x17CA00A4
#define PHY_ADDR_RMII0_TXD0		0x17CA00A0
#define PHY_ADDR_RMII0_TXD1		0x17CA009C
#define PHY_ADDR_RMII0_TXEN		0x17CA0090
#define PHY_ADDR_RMII0_RXD0		0x17CA0088
#define PHY_ADDR_RMII0_RXD1		0x17CA0084
#define PHY_ADDR_RMII0_RXDV		0x17CA0078

/* RMII1 pinctrl phyical addr */
#define PHY_ADDR_RMII1_CLK		0x17CA00E8
#define PHY_ADDR_RMII1_TXD0		0x17CA00E4
#define PHY_ADDR_RMII1_TXD1		0x17CA00E0
#define PHY_ADDR_RMII1_TXEN		0x17CA00D4
#define PHY_ADDR_RMII1_RXD0		0x17CA00CC
#define PHY_ADDR_RMII1_RXD1		0x17CA00C8
#define PHY_ADDR_RMII1_RXDV		0x17CA00BC

/* MDIO0 config value */
#define VALUE_MDCK0				0x1241
#define VALUE_MDIO0				0x1241
/* MDIO1 config value */
#define VALUE_MDCK1				0x1241
#define VALUE_MDIO1				0x1241

/* PHY0 config value */
#define VALUE_EPHY0_CLK			0x1271
#define VALUE_EPHY0_RSTN		0x1271
/* PHY1 config value */
#define VALUE_EPHY1_CLK			0x1271
#define VALUE_EPHY1_RSTN		0x1271

/* RGMII0 config value */
#define VALUE_RGMII0_TXCKOUT	0x1271
#define VALUE_RGMII0_TXD0		0x1271
#define VALUE_RGMII0_TXD1		0x1271
#define VALUE_RGMII0_TXD2		0x1271
#define VALUE_RGMII0_TXD3		0x1271
#define VALUE_RGMII0_TXEN		0x1271
#define VALUE_RGMII0_RXCK		0x1271
#define VALUE_RGMII0_RXD0		0x1271
#define VALUE_RGMII0_RXD1		0x1271
#define VALUE_RGMII0_RXD2		0x1271
#define VALUE_RGMII0_RXD3		0x1271
#define VALUE_RGMII0_RXDV		0x1271
/* RGMII1 config value */
#define VALUE_RGMII1_TXCKOUT	0x1271
#define VALUE_RGMII1_TXD0		0x1271
#define VALUE_RGMII1_TXD1		0x1271
#define VALUE_RGMII1_TXD2		0x1271
#define VALUE_RGMII1_TXD3		0x1271
#define VALUE_RGMII1_TXEN		0x1271
#define VALUE_RGMII1_RXCK		0x1271
#define VALUE_RGMII1_RXD0		0x1271
#define VALUE_RGMII1_RXD1		0x1271
#define VALUE_RGMII1_RXD2		0x1271
#define VALUE_RGMII1_RXD3		0x1271
#define VALUE_RGMII1_RXDV		0x1271

/* RMII0 config value */
#define VALUE_RMII0_CLK			0x1272
#define VALUE_RMII0_TXD0		0x1271
#define VALUE_RMII0_TXD1		0x1271
#define VALUE_RMII0_TXEN		0x1271
#define VALUE_RMII0_RXD0		0x1271
#define VALUE_RMII0_RXD1		0x1271
#define VALUE_RMII0_RXDV		0x1271
/* RMII1 config value */
#define VALUE_RMII1_CLK			0x1262
#define VALUE_RMII1_TXD0		0x1261
#define VALUE_RMII1_TXD1		0x1261
#define VALUE_RMII1_TXEN		0x1261
#define VALUE_RMII1_RXD0		0x1271
#define VALUE_RMII1_RXD1		0x1271
#define VALUE_RMII1_RXDV		0x1271

/* -------------------------------------------------------------------- */
/* USB */
/* -------------------------------------------------------------------- */
#define USB3_CTRL_REG_BASE		0x10300000
#define USB2_CTRL_REG_BASE		0x10340000

/* USB CRG register base */
#define PERI_CRG3632		(CRG_REG_BASE + 0x38c0)
#define PERI_CRG3636		(CRG_REG_BASE + 0x38d0)
#define PERI_CRG3637		(CRG_REG_BASE + 0x38d4)
#define PERI_CRG3664		(CRG_REG_BASE + 0x3940)
#define PERI_CRG3672		(CRG_REG_BASE + 0x3960)
#define PERI_CRG3676		(CRG_REG_BASE + 0x3970)

/* USB CTRL register base */
#define USB2_CTRL_BASE      0x10340000

/* USB PHY register EYE diagram para base */
#define USB2_PHY0_BASE		0x10310000
#define USB2_PHY1_BASE		0x10330000
#define USB2_PHY2_BASE		0x10350000

/* hp subsys misc register base */
#define AHB_SUBSYS_MISC_REG	0x102e0000

/* -------------------------------------------------------------------- */
/* PCIE */
/* -------------------------------------------------------------------- */
#define SYS_SATA			0x8c
#define PCIE_MODE			12

#define PERI_CRG98			0x188
#define PHY0_SRS_REQ            0
#define PHY0_SRS_REQ_SEL        1
#define PHY1_SRS_REQ            16
#define PHY1_SRS_REQ_SEL        17

#define MISC_CTRL5			0x14
#define NUM_0					0
#define NUM_1					1
#define NUM_2					2
#define NUM_3					3
#define NUM_4					4
#define NUM_5					5
#define NUM_6					6
#define NUM_7					7
#define NUM_8					8
#define NUM_9					9
#define NUM_10					10

/*-----------------------------------------------------------------------------------
 * boot sel type
 *-----------------------------------------------------------------------------------*/
#define BOOT_SEL_PCIE    0x965a4b87
#define BOOT_SEL_UART    0x69a5b478
#define BOOT_SEL_USB     0x965ab487
#define BOOT_SEL_SDIO    0x69a54b87
#define BOOT_SEL_FLASH   0x96a54b78
#define BOOT_SEL_EMMC    0x695ab487
#define BOOT_SEL_UNKNOW  0x965a4b78

/* -------------------------------------------------------------------- */
/* GZIP */
/* -------------------------------------------------------------------- */
#define HW_DEC_INTR             97

/* -------------------------------------------------------------------- */
/* VMCU */
/* -------------------------------------------------------------------- */
#define REG_VMCU_CRG	(CRG_REG_BASE + 0x6400)
#define VMCU_CRG_VAL	0x4011

#endif /* End of __CHIP_REGS_H__ */
