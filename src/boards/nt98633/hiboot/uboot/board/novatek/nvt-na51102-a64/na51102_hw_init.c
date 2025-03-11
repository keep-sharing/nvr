/**
    NVT evb board file
    To handle na51xxx HW init.
    @file       na51xxx_hw_init.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/


#include <common.h>
#include <asm/mach-types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/efuse_protected.h>
#ifdef CONFIG_NVT_IVOT_STBC_PMC_SUPPORT
#include <asm/arch/raw_stbcPMC.h>
#include <asm/arch/hardware.h>
#endif
#ifdef CONFIG_NVT_IVOT_DDR_RANGE_SCAN_SUPPORT
#include <asm/arch/raw_scanMP.h>
#include <asm/arch/hardware.h>
#endif
#include <linux/libfdt.h>

#define WDT_REG_ADDR(ofs)       (IOADDR_WDT_REG_BASE+(ofs))
#define WDT_GETREG(ofs)         readl(WDT_REG_ADDR(ofs))
#define WDT_SETREG(ofs,value)   writel(value, WDT_REG_ADDR(ofs))

#define STBC_REG_ADDR(ofs)       (IOADDR_STBC_CG_REG_BASE+(ofs))
#define STBC_GETREG(ofs)         readl(STBC_REG_ADDR(ofs))
#define STBC_SETREG(ofs,value)   writel(value, STBC_REG_ADDR(ofs))


#define CG_REG_ADDR(ofs)       (IOADDR_CG_REG_BASE+(ofs))
#define CG_GETREG(ofs)         readl(CG_REG_ADDR(ofs))
#define CG_SETREG(ofs,value)   writel(value, CG_REG_ADDR(ofs))

#define PAD_REG_ADDR(ofs)       (IOADDR_PAD_REG_BASE+(ofs))
#define PAD_GETREG(ofs)         readl(PAD_REG_ADDR(ofs))
#define PAD_SETREG(ofs,value)   writel(value, PAD_REG_ADDR(ofs))

#define TOP_REG_ADDR(ofs)       (IOADDR_TOP_REG_BASE+(ofs))
#define TOP_GETREG(ofs)         readl(TOP_REG_ADDR(ofs))
#define TOP_SETREG(ofs,value)   writel(value, TOP_REG_ADDR(ofs))

#define CG_PLL_ENABLE_OFS 0x0
#define STBC_ENABLE_OFS 0x140
#define CG_RESET_OFS 0x9C
#define CG_PLL4_DIV0_OFS 0x1318
#define CG_PLL4_DIV1_OFS 0x131C
#define CG_PLL4_DIV2_OFS 0x1320
#define WDT_POS (1 << 16)
#define WDT_RST (1 << 4)

#define WDT_CTRL_OFS 0x0
#define WDT_MANUL_OFS 0xC

#if defined(CONFIG_SD_CARD1_POWER_PIN) || defined(CONFIG_SD_CARD2_POWER_PIN) || defined(ETH_PHY_HW_RESET)
static void gpio_set_output(u32 pin)
{
	u32 reg_data;
	u32 ofs = (pin >> 5) << 2;

	pin &= (32 - 1);

	reg_data = readl((unsigned long)(IOADDR_GPIO_REG_BASE + 0x20 + ofs));
	reg_data |= (1 << pin);    //output
	writel(reg_data, (unsigned long)(IOADDR_GPIO_REG_BASE + 0x20 + ofs));
}

static void gpio_set_pin(u32 pin)
{
	u32 tmp;
	u32 ofs = (pin >> 5) << 2;

	pin &= (32 - 1);
	tmp = (1 << pin);

	writel(tmp, (unsigned long)(IOADDR_GPIO_REG_BASE + 0x40 + ofs));
}

static void gpio_clear_pin(u32 pin)
{
	u32 tmp;
	u32 ofs = (pin >> 5) << 2;

	pin &= (32 - 1);
	tmp = (1 << pin);

	writel(tmp, (unsigned long)(IOADDR_GPIO_REG_BASE + 0x60 + ofs));
}
#endif

void nvt_pllen(u32 id, u32 b_enable)
{
	u32 reg_data;

	reg_data = CG_GETREG(CG_PLL_ENABLE_OFS);
	if (b_enable) {
		reg_data |= (1 << id);
	} else {
		reg_data &= ~(1 << id);
	}
	CG_SETREG(CG_PLL_ENABLE_OFS, reg_data);
}

void nvt_ivot_reset_cpu(void)
{
	u32 reg_value;

	reg_value = CG_GETREG(CG_RESET_OFS);
	CG_SETREG(CG_RESET_OFS, reg_value | WDT_RST);

	reg_value = STBC_GETREG(STBC_ENABLE_OFS);
	STBC_SETREG(STBC_ENABLE_OFS, reg_value | WDT_POS);

	WDT_SETREG(WDT_CTRL_OFS, 0x5A960112);

	udelay(80);

	WDT_SETREG(WDT_CTRL_OFS, 0x5A960113);

	WDT_SETREG(WDT_MANUL_OFS, 0x1);
}



static void ethernet_init(void)
{
#if 0
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset, len;
	u32 *cell = NULL;
	char path[20] = {0};
	u32 sensor_cfg = 0x0, eth_cfg = 0x0, reg_val = 0x0;

	sprintf(path, "/top@%x/sensor", IOADDR_TOP_REG_BASE);

	nodeoffset = fdt_path_offset((const void *)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n", __func__, __LINE__);
		return ;
	}

	cell = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "pinmux", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n", __func__, __LINE__);
		return ;
	}

	sensor_cfg = __be32_to_cpu(cell[0]);
	debug("%s(%d) sensor_cfg = 0x%x\n", __func__, __LINE__, sensor_cfg);

	sprintf(path, "/eth@%x", IOADDR_ETH_REG_BASE);

	nodeoffset = fdt_path_offset((const void *)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n", __func__, __LINE__);
		return ;
	}

	cell = (u32 *)fdt_getprop((const void *)fdt_addr, nodeoffset, "sp-clk", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n", __func__, __LINE__);
		return ;
	}

	eth_cfg = __be32_to_cpu(cell[0]);
	debug("%s(%d) eth_cfg = 0x%x\n", __func__, __LINE__, eth_cfg);

	if ((sensor_cfg & 0x20000) || (eth_cfg == 1)) {
		/* Disable PLL4 */
		reg_val = CG_GETREG(CG_PLL_ENABLE_OFS);
		reg_val &= ~(0x1 << 4);
		CG_SETREG(CG_PLL_ENABLE_OFS, reg_val);

		CG_SETREG(CG_PLL4_DIV0_OFS, 0x0);
		CG_SETREG(CG_PLL4_DIV1_OFS, 0x0);
		CG_SETREG(CG_PLL4_DIV2_OFS, 0x32);

		/* spclk_sel: pll4 */
		reg_val = CG_GETREG(0x24);
		reg_val &= ~0x4300;
		reg_val |= (0x1 << 8);
		CG_SETREG(0x24, reg_val);

		reg_val = CG_GETREG(0x3C);
		reg_val &= ~0xFF000000;
		reg_val |= (0xB << 24);
		CG_SETREG(0x3C, reg_val);

		/* Enable PLL4 */
		reg_val = CG_GETREG(CG_PLL_ENABLE_OFS);
		reg_val |= (0x1 << 4);
		CG_SETREG(CG_PLL_ENABLE_OFS, reg_val);

		/* Enable SP_CLK */
		reg_val = CG_GETREG(0x70);
		reg_val |= (0x1 << 12);
		CG_SETREG(0x70, reg_val);

		/* Pinmux SP_CLK1 */
		reg_val = TOP_GETREG(0xC);
		reg_val &= ~(0x3 << 18);
		reg_val |= (0x1 << 18);
		TOP_SETREG(0xC, reg_val);

		/* Pinmux P_GPIO17 */
		reg_val = TOP_GETREG(0xA8);
		reg_val &= ~(0x1 << 17);
		TOP_SETREG(0xA8, reg_val);
	}
#endif

#ifdef ETH_PHY_HW_RESET
	gpio_set_output(NVT_PHY_RST_PIN);
	gpio_clear_pin(NVT_PHY_RST_PIN);
	mdelay(20);
	gpio_set_pin(NVT_PHY_RST_PIN);
	mdelay(50);
#endif
}

static void usbphy_init(void)
{
#if 0
	if (TOP_GETREG(0xF0) == 0x50210000) {
		//printf("********************%s(%d) Add USB528 INIT********************************************\n",__func__, __LINE__);
		writel((readl(0xFF600400)) | (0x1 << 21), 0xFF600400);
		writel((readl(0xFF6001C8)) | (0x1 << 31), 0xFF6001C8);
	} else {
		//printf("********************%s(%d) Add USB520 INIT********************************************\n",__func__, __LINE__);
		writel((readl(0xF0600310)) | (0x1 << 1), 0xF0600310);
		writel((readl(0xF06001C8)) | (0x1 << 5), 0xF06001C8);
	}
#endif
}

static void axi_bridge_timeout_en(void)
{
#if 0
	writel(0x7c, 0xF0012018);
	writel(0x7c, 0xF0013018);
#endif
}


static void nvt_usb3_init(void)
{
	int reg = 0;

	reg = readl(0x2f0580140);
	reg |= (0x30);
	writel(reg, 0x2f0580140);
}

typedef enum {
	PAD_POWERID_SN   =          0x10,      ///< Pad power id for SN  REG
	PAD_POWERID_P1   =          0x20,      ///< Pad power id for P1  REG
	PAD_POWERID_SD3  =          0x40,      ///< Pad power id for SD3 REG

	ENUM_DUMMY4WORD(PAD_POWERID)
} PAD_POWERID;

typedef enum {
	PAD_3P3V         =          0x00,      ///< Pad power is 1.8V
	PAD_1P8V         =          0x01,      ///< Pad power is 3.3V

	ENUM_DUMMY4WORD(PAD_POWER)
} PAD_POWER;

typedef struct {
	PAD_POWERID         pad_power_id;      ///< Pad power id
	PAD_POWER           pad_power;         ///< Pad power (for set)
	PAD_POWER           pad_status;        ///< Pad cell MS1/MS2 status (for get)
} PAD_POWER_STRUCT;

#define PAD_PWR1_REG_OFS                     0x204
#define PAD_PWR2_REG_OFS                     0x208
#define PAD_PWR3_REG_OFS                     0x20C

#define PAD_1P8V_SN_MAGIC                    0x00005378
#define PAD_1P8V_P1_MAGIC                    0x00008031
#define PAD_1P8V_SD3_MAGIC                   0x00534433

int pad_set_power(PAD_POWER_STRUCT *pad_power)
{
	if (pad_power == NULL) {
		pr_err("%s: Not Existed PAD_POWER_STRUCT\r\n", __func__);
		return -1;
	}

	if (pad_power->pad_power_id == PAD_POWERID_SN) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set the magic number of SN (bootstrap[13] needs to be 1 before setting)
			PAD_SETREG(PAD_PWR1_REG_OFS, PAD_1P8V_SN_MAGIC);
		} else if (pad_power->pad_power == PAD_3P3V) {
			PAD_SETREG(PAD_PWR1_REG_OFS, 0);
		} else {
			pr_err("%s: Not Existed PAD_POWER for SN\r\n", __func__);
			return -1;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_P1) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set the magic number of P1 (bootstrap[14] needs to be 1 before setting)
			PAD_SETREG(PAD_PWR2_REG_OFS, PAD_1P8V_P1_MAGIC);
		} else if (pad_power->pad_power == PAD_3P3V) {
			PAD_SETREG(PAD_PWR2_REG_OFS, 0);
		} else {
			pr_err("%s: Not Existed PAD_POWER for P1\r\n", __func__);
			return -1;
		}
	} else if (pad_power->pad_power_id == PAD_POWERID_SD3) {
		if (pad_power->pad_power == PAD_1P8V) {
			// Set the magic number of SD3 (bootstrap[15] needs to be 1 before setting)
			PAD_SETREG(PAD_PWR3_REG_OFS, PAD_1P8V_SD3_MAGIC);
		} else if (pad_power->pad_power == PAD_3P3V) {
			PAD_SETREG(PAD_PWR3_REG_OFS, 0);
		} else {
			pr_err("%s: Not Existed PAD_POWER for SD3\r\n", __func__);
			return -1;
		}
	} else {
		pr_err("%s: Not Existed PAD_POWERID\r\n", __func__);
		return -1;
	}

	return 0;
}

static void power_init(void)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	PAD_POWER_STRUCT pad_power;
	u32 top_reg0 = TOP_GETREG(0x0);

	sprintf(path,"/top/sgpio_0_26");
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "power_config", NULL);
	if (cell < 0) {
		printf("%s: Not Existed power_config for sgpio_0_26, skip\n", __func__);
	} else {
		pad_power.pad_power_id = __be32_to_cpu(cell[0]);
		pad_power.pad_power = __be32_to_cpu(cell[1]);
		if (pad_power.pad_power == PAD_1P8V) {
			if (!(top_reg0 & 0x2000)) {
				printf("WARNING: sgpio_0_26 is <1P8V>, but bootstrap[13] is <3P3V>\n");

				/* Stop boot */
				printf("!!! Stop booting because your power_config dtsi and bootstrap do not match !!!\n");
				while (1);
			}

			pad_set_power(&pad_power);
		} else if (pad_power.pad_power == PAD_3P3V) {
			if (top_reg0 & 0x2000) {
				printf("WARNING: sgpio_0_26 is <3P3V>, but bootstrap[13] is <1P8V>\n");

				/* Stop boot */
				printf("!!! Stop booting because your power_config dtsi and bootstrap do not match !!!\n");
				while (1);
			}

			pad_set_power(&pad_power);
		} else {
			printf("%s: Unknown power_config, id(0x%x), power(0x%x)\n", __func__, pad_power.pad_power_id, pad_power.pad_power);
		}
	}

	sprintf(path,"/top/pgpio_0_19");
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "power_config", NULL);
	if (cell < 0) {
		printf("%s: Not Existed power_config for pgpio_0_19, skip\n", __func__);
	} else {
		pad_power.pad_power_id = __be32_to_cpu(cell[0]);
		pad_power.pad_power = __be32_to_cpu(cell[1]);
		if (pad_power.pad_power == PAD_1P8V) {
			if (!(top_reg0 & 0x4000)) {
				//printf("WARNING: pgpio_0_19 is <1P8V>, but bootstrap[14] is <3P3V>\n");
				/* TODO: Stop boot */
			}

			pad_set_power(&pad_power);
		} else if (pad_power.pad_power == PAD_3P3V) {
			if (top_reg0 & 0x4000) {
				//printf("WARNING: pgpio_0_19 is <3P3V>, but bootstrap[14] is <1P8V>\n");
				/* TODO: Stop boot */
			}

			pad_set_power(&pad_power);
		} else {
			printf("%s: Unknown power_config, id(0x%x), power(0x%x)\n", __func__, pad_power.pad_power_id, pad_power.pad_power);
		}
	}

	sprintf(path,"/top/cgpio_0_10");
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "power_config", NULL);
	if (cell < 0) {
		printf("%s: Not Existed power_config for cgpio_0_10, skip\n", __func__);
	} else {
		pad_power.pad_power_id = __be32_to_cpu(cell[0]);
		pad_power.pad_power = __be32_to_cpu(cell[1]);
		if (pad_power.pad_power == PAD_1P8V) {
			if (!(top_reg0 & 0x8000)) {
				printf("WARNING: cgpio_0_10 is <1P8V>, but bootstrap[15] is <3P3V>\n");

				/* Stop boot */
				printf("!!! Stop booting because your power_config dtsi and bootstrap do not match !!!\n");
				while (1);
			}

			pad_set_power(&pad_power);
		} else if (pad_power.pad_power == PAD_3P3V) {
			if (top_reg0 & 0x8000) {
				printf("WARNING: cgpio_0_10 is <3P3V>, but bootstrap[15] is <1P8V>\n");

				/* Stop boot */
				printf("!!! Stop booting because your power_config dtsi and bootstrap do not match !!!\n");
				while (1);
			}

			pad_set_power(&pad_power);
		} else {
			printf("%s: Unknown power_config, id(0x%x), power(0x%x)\n", __func__, pad_power.pad_power_id, pad_power.pad_power);
		}
	}
}

int nvt_ivot_hw_init(void)
{
	nvt_ivot_set_cpuclk();

	ethernet_init();

	axi_bridge_timeout_en();

	usbphy_init();

	nvt_usb3_init();

	power_init();

	return 0;
}

int nvt_ivot_hw_init_early(void)
{
	otp_init();
	return 0;
}

#ifdef CONFIG_NVT_IVOT_DDR_RANGE_SCAN_SUPPORT
typedef void (*LDR_GENERIC_CB)(void);
#define JUMP_ADDR 0x2f0e00000
void nvt_ddr_scan(u32 type)
{
	int reg = 0;
	uint64_t    addr = 0xF0E00000;
	int current_el = 0;
	register uint64_t x0 __asm__("x0");
	addr = *(UINT32 *)JUMP_ADDR;


	//printf("Loader ini ver = 0x%02x\r\n", readl(0xfe20001c));
//	void (*image_entry)(void) = NULL;
	printf("memcpy->[0x%08x]", (int)JUMP_ADDR);
	if (type) {
		writel(type, 0x2f0050084);
		memcpy((void *)JUMP_ADDR, ddr_scan, sizeof(ddr_scan));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan), ARCH_DMA_MINALIGN));
	} else {
		writel(0x20050e1c, 0x2f0050084);
		memcpy((void *)JUMP_ADDR, ddr_scan, sizeof(ddr_scan));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan), ARCH_DMA_MINALIGN));
	}
	printf("done\r\n");
	printf("Jump into sram entry point[0x%08x]\n", (int)addr);

	//1. switch register type 0x2Fxxx_xxxx to 0xFxxx_xxxx
	reg = readl(0x2FFE41014);
	reg &= ~(1);
	writel(reg, 0x2FFE41014);

	__asm__("mrs x0, CurrentEL;" : : : "%x0");
	current_el = (int)(x0 >> 2);
	printf("Current EL = %d\r\n", (int)current_el);
	if (current_el == 2) {
		printf("Current EL = 2 and use EL1 to execute aarch32\r\n");
		__asm__ volatile("MSR SCTLR_EL1, XZR\n"
						 "MRS X0, HCR_EL2\n"
						 "BIC X0, X0, #(1<<31)\n"
						 "MSR HCR_EL2, X0\n"
						 "MOV X0, #0b10011\n"
						 "MSR SPSR_EL2, X0\n"
						 : : :);
		__asm__ volatile("	msr ELR_EL2, %0\n"
						 "ERET\n"
						 : : "r"(addr));
	} else if (current_el == 3) {
		printf("Current EL = 3 and use EL2 to execute aarch32\r\n");
		__asm__ volatile("MSR SCTLR_EL2, XZR\n"
						 "MSR HCR_EL2, XZR\n"
						 "MRS X0, SCR_EL3\n"
						 "BIC X0, X0, #(1<<10)\n"
						 "MSR SCR_EL3, x0\n"
						 "MOV X0, #0b10011\n"
						 "MSR SPSR_EL3, X0\n"
						 : : :);
		__asm__ volatile("	msr ELR_EL3, %0\n"
						 "ERET\n"
						 : : "r"(addr));

	} else if (current_el == 1) {
		printf("Current EL = 1 and use EL0 to execute aarch32 => not support\r\n");
	} else {
		printf("Unknow EL ...\r\n");
	}
	//image_entry = (LDR_GENERIC_CB)(*((unsigned long*)JUMP_ADDR));
	//image_entry();
}
#endif

#ifdef CONFIG_NVT_IVOT_STBC_PMC_SUPPORT
#define FW_CHECK_PSEUDO_STR   ""
#define _TAG_OFFSET           0x30
#define _TAG_VALUE            0xAA55
static BOOL check_loader(UINT32 uiAddr, UINT32 uiSize)
{
	return TRUE;
	UINT16  *puiValue, uiSum;
	UINT32  i;
	puiValue    = (UINT16 *)uiAddr;
	uiSum       = 0;

	for (i = 0; i < (uiSize >> 1); i++) {
		uiSum += (*puiValue + i);
		puiValue++;
	}
	if ((*(UINT16 *)(uiAddr + _TAG_OFFSET) != _TAG_VALUE) ||
		(uiSum != 0)) {
		return FALSE;
	} else {
		return TRUE;
	}
}

#define OUTREG32(x, y)          writel(y, x)
#define INREG32(x)              readl(x)

#ifndef OUTW
#define OUTW(x, y)              writel(y, x)
#endif

#ifndef INW
#define INW(x)                  readl(x)
#endif


#define SETREG32(x, y)          OUTW((x), INW(x) | (y))
#define CLRREG32(x, y)          OUTW((x), INW(x) & ~(y))

#define PD_MODE_CASE_1                  0
#define PD_MODE_CASE_2                  1               //STBC
#define PD_MODE                         PD_MODE_CASE_2
#define RO_PHY_CLKSEL_LPRO              (0x0<<0)
#define RO_PHY_CLKSEL_LNRO              (0x1<<0)
#define RO_PHY_CLKSEL_PRO               (0x2<<0)
#define RO_PHY_CLKSEL_NRO               (0x3<<0)

#define RO_PHY_FREQ_HIGH_FREQ           (0x0<<4)
#define RO_PHY_FREQ_LOW_FREQ            (0x1<<4)


#define pmc_en                          0x1
#if (PD_MODE == PD_MODE_CASE_2)
#define pmc_iso_en                      0x3
#define pmc_io_en                       0xf
#else
#define pmc_iso_en                      0x0
#define pmc_io_en                       0x0
#endif

//#define pmc_off_stage                   0x2
#define pmc_off_stage                   0x4
//#define pmc_on_stage                    0x5
#define pmc_on_stage                    0xa
#define pmc_wakeup_sel                  0x0             //wakeup select
#define pmc_wakeup_oen                  0x3
#define pmc_wakeup_from                 0x1 //CG
//#define   pmc_wakeup_from             0x0 //USB
//#define   pmc_wakeup_dbg                  0x0

#define pmc_wakeup_dbg                  0x1

#define pmc_debug_probe1                0x1
#define pmc_debug_probe2                0x2
#define pmc_debug_always_on             0x1
#define pmc_debug_always_off            0x0


#define PMC_SHARE_MEMORY_REG0           0xF05C00A0

#define PMC_DEBUG_HALT_REG_OFS          0x00000000
//bit[0] = 0 -> stbc case1
//bit[0] = 1 -> stbc case2
//bit[31..28]-> debug probe
#define PMC_DEBUG_CONFIG_REG_OFS        0x00000004
#define PMC_CFG_DEF_0_OFS               0x00000008
#define PMC_CFG_DEF_1_OFS               0x0000000C

#define PMC_ISO_STAGE_DEF_0_OFS         0x00000010      //stage0~7
#define PMC_ISO_STAGE_DEF_1_OFS         0x00000014      //stage8~15
#define PMC_PAD_OUT_STAGE_DEF_0_OFS     0x00000018      //stage0~7
#define PMC_PAD_OUT_STAGE_DEF_1_OFS     0x0000001C      //stage8~15

#define PMC_PAD_OEN_STAGE_DEF_0_OFS     0x00000020      //stage0~7
#define PMC_PAD_OEN_STAGE_DEF_1_OFS     0x00000024      //stage8~15
#define PMC_USB_SHARE_REGISTER_OFS      0x00000028      //SUSPEND reg
#define STBC_RINGOSC_CLK_CTRL_REG_OFS   0x0000002C      //STBC ring cfg


#define PMC_DEBUG_HALT_REG              (PMC_SHARE_MEMORY_REG0 + PMC_DEBUG_HALT_REG_OFS)
//bit[0] = 0 -> stbc case1
//bit[0] = 1 -> stbc case2
//bit[31..28]-> debug probe
#define PMC_DEBUG_CONFIG_REG            (PMC_SHARE_MEMORY_REG0 + PMC_DEBUG_CONFIG_REG_OFS)
#define PMC_CFG_DEF_0                   (PMC_SHARE_MEMORY_REG0 + PMC_CFG_DEF_0_OFS)
#define PMC_CFG_DEF_1                   (PMC_SHARE_MEMORY_REG0 + PMC_CFG_DEF_1_OFS)

#define PMC_ISO_STAGE_DEF_0             (PMC_SHARE_MEMORY_REG0 + PMC_ISO_STAGE_DEF_0_OFS)       //stage0~7
#define PMC_ISO_STAGE_DEF_1             (PMC_SHARE_MEMORY_REG0 + PMC_ISO_STAGE_DEF_1_OFS)       //stage8~15
#define PMC_PAD_OUT_STAGE_DEF_0         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OUT_STAGE_DEF_0_OFS)   //stage0~7
#define PMC_PAD_OUT_STAGE_DEF_1         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OUT_STAGE_DEF_1_OFS)   //stage8~15

#define PMC_PAD_OEN_STAGE_DEF_0         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OEN_STAGE_DEF_0_OFS)   //stage0~7
#define PMC_PAD_OEN_STAGE_DEF_1         (PMC_SHARE_MEMORY_REG0 + PMC_PAD_OEN_STAGE_DEF_1_OFS)   //stage8~15
#define PMC_USB_SHARE_REGISTER          (PMC_SHARE_MEMORY_REG0 + PMC_USB_SHARE_REGISTER_OFS)    //SUSPEND reg
#define STBC_RINGOSC_CLK_CTRL_REG       (PMC_SHARE_MEMORY_REG0 + STBC_RINGOSC_CLK_CTRL_REG_OFS) //STBC ring cfg

#define IS_CASE_2()                     (INREG32(PMC_DEBUG_CONFIG_REG) & 0x1)

#define SET_CASE_1()                    CLRREG32(PMC_DEBUG_CONFIG_REG, 0x1)
#define SET_CASE_2()                    SETREG32(PMC_DEBUG_CONFIG_REG, 0x1)

#define SET_OUT_MSG_ON()                SETREG32(PMC_DEBUG_CONFIG_REG, 0x2)
#define SET_OUT_MSG_OFF()               CLRREG32(PMC_DEBUG_CONFIG_REG, 0x2)


#define SET_ISO_STAGE_0(m)              OUTREG32(PMC_ISO_STAGE_DEF_0, m)
#define SET_ISO_STAGE_1(m)              OUTREG32(PMC_ISO_STAGE_DEF_1, m)

#define SET_PAD_OUT_STAGE_0(m)          OUTREG32(PMC_PAD_OUT_STAGE_DEF_0, m)
#define SET_PAD_OUT_STAGE_1(m)          OUTREG32(PMC_PAD_OUT_STAGE_DEF_1, m)

#define SET_PAD_OEN_STAGE_0(m)          OUTREG32(PMC_PAD_OEN_STAGE_DEF_0, m)
#define SET_PAD_OEN_STAGE_1(m)          OUTREG32(PMC_PAD_OEN_STAGE_DEF_1, m)

#define SET_PMC_CFG_0(m)                OUTREG32(PMC_CFG_DEF_0, m)
#define SET_PMC_CFG_1(m)                OUTREG32(PMC_CFG_DEF_1, m)

#define SET_STBC_RING_OSC_CTRL_REG(m)   OUTREG32(STBC_RINGOSC_CLK_CTRL_REG, m)

typedef void (*LDR_GENERIC_CB)(void);
#define JUMP_ADDR 0x2f0e00000
static inline unsigned int get_cpuid(void)
{
	unsigned int val;
	/*
	 * core 0 is EL1, but core 1 ~ 3 is EL3
	 * cor 1 ~ core 3 need to change to EL1
	 */

	asm volatile("mrs %0, MPIDR_EL1" : "=r"(val)
				 :
				 : "cc");
	return (val & 0xFF);
}
void nvt_stbc_pmc(UINT32 debug)
{
	int reg = 0;
	uint64_t addr = 0xF0E00000;

	int current_el = 0;

	UINT32  pmc_config;
	UINT32 iLibVersion, iLibDRAMVersion;
	UINT32 YY, MM, DD, VER;
	UINT32 LD_MJ, LD_MIN, VER_Fix;

	register uint64_t x0 __asm__("x0");
	addr = *(UINT32 *)JUMP_ADDR;

	static UINT32 *stbc_loader;
	UINT32  stbc_loader_size = 0x10000;
	stbc_loader = (UINT32 *)&stbc_PMC[0];
	stbc_loader_size = *(volatile UINT32 *)(stbc_PMC + (0x24 / 4));

	iLibVersion = *(volatile UINT32 *)(stbc_PMC + 29);     //0x74 / 4 = 29
	iLibDRAMVersion = *(volatile UINT32 *)(stbc_PMC + 30); //0x78 / 4 = 30

	YY = 2020 + (iLibDRAMVersion & 0x7);
	MM = ((iLibDRAMVersion >> 3) & 0xF);
	DD = ((iLibDRAMVersion >> 7) & 0x1F);
	VER = ((iLibDRAMVersion >> 12) & 0xF);
	VER_Fix = ((iLibVersion >> 16) & 0xFF);
	LD_MIN = ((iLibVersion >> 24) & 0xF);
	LD_MJ = ((iLibVersion >> 28) & 0xF);
	printf("Enter power mode 3 @cpu[%d] debug_en[%d]\r\n", get_cpuid(), debug);

	writel(3, 0x2F029001C);
	writel(0xF0E00028, 0x2F0090104);
	printf("PowerDown Isolate version:\r\n");
	printf("DR[%d-%d-%d-%d]:", (int)YY, (int)MM, (int)DD, (int)VER);
	printf("LD[%d.%d.%d]", (int)LD_MJ, (int)LD_MIN, (int)VER_Fix);
	if (check_loader(stbc_PMC, stbc_loader_size) == TRUE) {
		printf("check sum OK\r\n");
	} else {
		printf("check sum Fail\r\n");
	}
	printf("memcpy->[0x%08x][0x%08x][0x%08x]", (int)JUMP_ADDR, sizeof(stbc_PMC), stbc_loader_size);
	memcpy((void *)JUMP_ADDR, stbc_PMC, sizeof(stbc_PMC));
	printf("->done\n");
	printf("flush-> size[0x%08x]", (int)stbc_loader_size);

	reg = readl(0x2FFE41010);
	printf("MPIDR_EL1[%02d]->", (int)get_cpuid());
	reg &= ~0x8000000F;
	reg |= (1 << get_cpuid());
	printf("MPIDR_EL1[0x%08x]\r\n", (int)reg);
	writel(reg, 0x2FFE41010);

	reg = readl(0x2FFE41014);
	reg &= ~(1);
	writel(reg, 0x2FFE41014);

#if (PD_MODE == PD_MODE_CASE_1)
	SET_CASE_1();
#else
	SET_CASE_2();
#endif
	SET_ISO_STAGE_0(0x11111110);
	SET_ISO_STAGE_1(0x00000011);

	SET_PAD_OUT_STAGE_0(0x31013777);
	SET_PAD_OUT_STAGE_1(0x00007777);

	SET_PAD_OEN_STAGE_0(0x00000088);
	SET_PAD_OEN_STAGE_1(0x00008880);



	pmc_config =    pmc_en              << 0  |
					pmc_off_stage       << 8  |
					pmc_on_stage        << 12 |
					pmc_wakeup_sel      << 16 |
					pmc_wakeup_oen      << 20 |
					pmc_wakeup_from     << 24 |
					debug               << 25 |
					pmc_debug_probe2    << 28 |
					pmc_debug_always_on << 31 |
					0;
	SET_PMC_CFG_0(pmc_config);

	pmc_config =    pmc_en              << 0  |
					pmc_iso_en          << 2  |
					pmc_io_en           << 4  |
					pmc_off_stage       << 8  |
					pmc_on_stage        << 12 |
					pmc_wakeup_sel      << 16 |
					pmc_wakeup_oen      << 20 |
					pmc_wakeup_from     << 24 |
					debug               << 25 |
					pmc_debug_probe2    << 28 |
					pmc_debug_always_on << 31 |
					0;

	SET_PMC_CFG_1(pmc_config);
	//clock div bit[15..4] = 0

	//[1]. Setup ring osc
	//CLRREG32(0xF0580124, 0x000FF000);
	//SETREG32(0xF0580124, 0x000FF000);
	SET_STBC_RING_OSC_CTRL_REG(RO_PHY_CLKSEL_NRO | RO_PHY_FREQ_LOW_FREQ);

	OUTREG32(0xF05C0040, 0x2FF);
	OUTREG32(0xF05C0044, 0x2FF);
	OUTREG32(0xF05C0048, 0x2FF);
	OUTREG32(0xF05C004C, 0x2FF);

	OUTREG32(0xF05C0050, 0x2FF);
	OUTREG32(0xF05C0054, 0x2FFFD);
	OUTREG32(0xF05C0058, 0x2FF);
	OUTREG32(0xF05C005C, 0x2FF);

	OUTREG32(0xF05C0060, 0x2FF);
	OUTREG32(0xF05C0064, 0x2FF);
	OUTREG32(0xF05C0068, 0x2FF);
	OUTREG32(0xF05C006C, 0x2FF);

	OUTREG32(0xF05C0070, 0x2FF);
	OUTREG32(0xF05C0074, 0x2FF);
	OUTREG32(0xF05C0078, 0x2FF);
	OUTREG32(0xF05C007C, 0x2FF);


	SET_OUT_MSG_ON();
	printf("Debug config = 0x%08x\r\n", INREG32(PMC_DEBUG_CONFIG_REG));
	printf(" 0xF0580124  = 0x%08x\r\n", INREG32(0xF0580124));
	printf(" 0xF05800B0  = 0x%08x\r\n", INREG32(0xF05800B0));

	printf("Jump into sram entry point[0x%08x]\n", (int)JUMP_ADDR);
	//1. switch register type 0x2Fxxx_xxxx to 0xFxxx_xxxx
#if 0
	image_entry = (LDR_GENERIC_CB)(JUMP_ADDR);
//while(test){};
	image_entry();
#if 0
	__asm__ volatile("mrs	 x0, RMR_EL3\n\t"
					 "bic  x0, x0, #1\n\t"   /* Clear enter-in-64 bit */
					 "orr  x0, x0, #2\n\t"   /* set reset request bit */
					 "msr  RMR_EL3, x0\n\t"
					 "isb  sy\n\t"
					 "nop\n\t"
					 "wfi\n\t"
					 "b    .\n"
					 ::: "x0");
#endif
#else
	__asm__("mrs x0, CurrentEL;" : : : "%x0");
	current_el = (int)(x0 >> 2);
	printf("Current EL = %d\r\n", (int)current_el);
	if (current_el == 2) {
		printf("Current EL = 2 and use EL1 to execute aarch32\r\n");
		__asm__ volatile("MSR SCTLR_EL1, XZR\n"
						 "MRS X0, HCR_EL2\n"
						 "BIC X0, X0, #(1<<31)\n"
						 "MSR HCR_EL2, X0\n"
						 "MOV X0, #0b10011\n"
						 "MSR SPSR_EL2, X0\n"
						 : : :);
		__asm__ volatile("	msr ELR_EL2, %0\n"
						 "ERET\n"
						 : : "r"(addr));
	} else if (current_el == 3) {
		printf("Current EL = 3 and use EL2 to execute aarch32\r\n");
		__asm__ volatile("MSR SCTLR_EL2, XZR\n"
						 "MSR HCR_EL2, XZR\n"
						 "MRS X0, SCR_EL3\n"
						 "BIC X0, X0, #(1<<10)\n"
						 "MSR SCR_EL3, x0\n"
						 "MOV X0, #0b10011\n"
						 "MSR SPSR_EL3, X0\n"
						 : : :);
		__asm__ volatile("	msr ELR_EL3, %0\n"
						 "ERET\n"
						 : : "r"(addr));

	} else if (current_el == 1) {
		printf("Current EL = 1 and use EL0 to execute aarch32 => not support\r\n");
	} else {
		printf("Unknow EL ...\r\n");
	}
	//image_entry = (LDR_GENERIC_CB)(*((unsigned long*)JUMP_ADDR));
	//image_entry();
#endif
}
#endif
