/*
 * novatek ivot sdc021 controller driver.
 *
 * Copyright (C) 2019 Novatek Microelectronics Corp. All rights reserved.
 * Author: IVOT-ESW <IVOT_ESW_MailGrp@novatek.com.tw>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <sdhci.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <libfdt.h>

#define CONFIG_PMU_BASE         0xfe000000
#define CONFIG_IVOT_MMC_BASE    0xfa600000

#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000

#define PIN_SDIO_CFG_1ST_PINMUX  0x10
#define PIN_SDIO_CFG_2ND_PINMUX  0x20

int nvt_mmc_init(void)
{
	u32 regbase = CONFIG_IVOT_MMC_BASE;
	struct sdhci_host *host = NULL;
	uint32_t val;

	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	uint32_t pinmux_value = 0;

	sprintf(path,"/top@%x/sdio", CONFIG_PMU_BASE + 0x30000);
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", NULL);
	pinmux_value = __be32_to_cpu(cell[0]);

	if (pinmux_value & PIN_SDIO_CFG_1ST_PINMUX) {
		puts("nvt_mmc_init: 1st pinmux\n");
	} else if (pinmux_value & PIN_SDIO_CFG_2ND_PINMUX) {
		puts("nvt_mmc_init: 2nd pinmux\n");
	}

	writel(readl(CONFIG_PMU_BASE + 0x54) | (BIT10), CONFIG_PMU_BASE + 0x54);                ///< hw_rst
	writel(readl(CONFIG_PMU_BASE + 0x68) & ~(BIT10), CONFIG_PMU_BASE + 0x68);               ///< clk
	val = readl(CONFIG_PMU_BASE + 0x18c) & ~(BIT8 | BIT9 | BIT10 | BIT11 | BIT12 | BIT13);  ///< sdcclk divided value
#ifdef CONFIG_NVT_FPGA_EMULATION_CA9
	writel((val | BIT8), CONFIG_PMU_BASE + 0x18c);
#else
	writel(val, CONFIG_PMU_BASE + 0x18c);
#endif

	if (pinmux_value & PIN_SDIO_CFG_1ST_PINMUX) {
		val = readl(CONFIG_PMU_BASE + 0x30028) & ~(BIT28 | BIT29 | BIT30 | BIT31);
		writel((val | BIT28 | BIT30), CONFIG_PMU_BASE + 0x30028); ///< sd pinmux setting
		val = readl(CONFIG_PMU_BASE + 0x3002C) & ~(0xF | BIT28 | BIT29 | BIT30 | BIT31);
		writel((val | 0x5 | BIT28 | BIT30), CONFIG_PMU_BASE + 0x3002C); ///< sd pinmux setting
		val = readl(CONFIG_PMU_BASE + 0x30030) & ~(0xFFF | BIT16 | BIT17 | BIT18 | BIT19);
		writel((val | 0x555 | BIT16 | BIT18), CONFIG_PMU_BASE + 0x30030); ///< sd pinmux setting

		val = readl(CONFIG_PMU_BASE + 0x40004) & ~(BIT14 | BIT15 | BIT16 | BIT17 | BIT30 | BIT31);
		writel((val | BIT15 | BIT17 | BIT31), CONFIG_PMU_BASE + 0x40004); ///< pullup
		val = readl(CONFIG_PMU_BASE + 0x40008) & ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT8 | BIT9);
		writel((val | BIT1 | BIT3 | BIT5 | BIT9), CONFIG_PMU_BASE + 0x40008); ///< pullup
	} else if (pinmux_value & PIN_SDIO_CFG_2ND_PINMUX) {
		val = readl(CONFIG_PMU_BASE + 0x3002C) & ~(BIT4 | BIT5 | BIT6 | BIT8 | BIT9 | BIT10);
		writel((val | 0x550), CONFIG_PMU_BASE + 0x3002C); ///< sd pinmux setting
		val = readl(CONFIG_PMU_BASE + 0x30040) & ~(BIT20 | BIT21 | BIT22 | BIT24 | BIT25 | BIT26 | BIT28 | BIT29 | BIT30);
		writel((val | 0x66600000), CONFIG_PMU_BASE + 0x30040); ///< sd pinmux setting
		val = readl(CONFIG_PMU_BASE + 0x30044) & ~(BIT0 | BIT1 | BIT2 | BIT4 | BIT5 | BIT6);
		writel((val | 0x66), CONFIG_PMU_BASE + 0x30044); ///< sd pinmux setting

		val = readl(CONFIG_PMU_BASE + 0x40004) & ~(BIT18 | BIT19 | BIT20 | BIT21);
		writel((val | BIT19 | BIT21), CONFIG_PMU_BASE + 0x40004); ///< pullup X_I2C2_SCL, X_I2C2_SDA
		val = readl(CONFIG_PMU_BASE + 0x40010) & ~(BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | BIT18 | BIT19);
		writel((val | BIT11 | BIT13 | BIT15 | BIT17 | BIT19), CONFIG_PMU_BASE + 0x40010); ///< pullup X_GPIO_5, X_GPIO_6, X_GPIO_7, X_GPIO_8, X_GPIO_9
	}
	//val = readl(CONFIG_PMU_BASE + 0x1F0) & ~(BIT24 | BIT25 | BIT26 | BIT27 | BIT28 | BIT29 | BIT30 | BIT31);
	//writel((val | BIT24 | BIT25 | BIT28 | BIT29), CONFIG_PMU_BASE + 0x1F0); ///< sdc Driving Capacity and Slew Rate, 8mA

#ifdef CONFIG_NVT_MMC_ADJUST
	if (pinmux_value & PIN_SDIO_CFG_1ST_PINMUX) {
		val = readl(CONFIG_PMU_BASE + 0x40084) & ~(BIT14 | BIT15 | BIT16 | BIT17 | BIT30 | BIT31);
		writel((val | (CONFIG_NVT_MMC_DRIVING << 14) | (CONFIG_NVT_MMC_DRIVING << 16) | (CONFIG_NVT_MMC_DRIVING << 30)), CONFIG_PMU_BASE + 0x40084); ///< driving

		val = readl(CONFIG_PMU_BASE + 0x40088) & ~(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT8 | BIT9);
		writel((val | (CONFIG_NVT_MMC_DRIVING << 0) | (CONFIG_NVT_MMC_DRIVING << 2) | (CONFIG_NVT_MMC_DRIVING << 4) \
				| (CONFIG_NVT_MMC_DRIVING << 8)), CONFIG_PMU_BASE + 0x40088); ///< driving
	} else if (pinmux_value & PIN_SDIO_CFG_2ND_PINMUX) {
		val = readl(CONFIG_PMU_BASE + 0x40084);
		writel((val | (CONFIG_NVT_MMC_DRIVING << 18) | (CONFIG_NVT_MMC_DRIVING << 20)), CONFIG_PMU_BASE + 0x40084); ///< driving

		val = readl(CONFIG_PMU_BASE + 0x40090);
		writel((val | (CONFIG_NVT_MMC_DRIVING << 10) | (CONFIG_NVT_MMC_DRIVING << 12) | (CONFIG_NVT_MMC_DRIVING << 14) \
				| (CONFIG_NVT_MMC_DRIVING << 16) | (CONFIG_NVT_MMC_DRIVING << 18)), CONFIG_PMU_BASE + 0x40090); ///< driving
	}
#endif

	host = calloc(1, sizeof(struct sdhci_host));
	if (!host) {
		puts("sdh_host malloc fail!\n");
		return 1;
	}

	host->name = "NA51068_SDC";
	host->ioaddr = (void __iomem *)regbase;
	host->quirks = 0;
	host->version = SDHCI_SPEC_300;

	add_sdhci(host, 100000000, 400000);

	return 0;
}
