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
#ifdef CONFIG_NVT_IVOT_DDR_RANGE_SCAN_SUPPORT
#include <asm/arch/raw_scanMP.h>
#include <asm/arch/hardware.h>
#endif
#include <linux/libfdt.h>

#define WDT_REG_ADDR(ofs)       (IOADDR_WDT_REG_BASE+(ofs))
#define WDT_GETREG(ofs)         readl(WDT_REG_ADDR(ofs))
#define WDT_SETREG(ofs,value)   writel(value, WDT_REG_ADDR(ofs))

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
#define CG_ENABLE_OFS 0x7C
#define CG_RESET_OFS 0x9C
#define CG_PLL4_DIV0_OFS 0x1318
#define CG_PLL4_DIV1_OFS 0x131C
#define CG_PLL4_DIV2_OFS 0x1320
#define WDT_POS (1 << 4)
#define WDT_RST (1 << 12)

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

void nvt_ivot_reset_cpu(void)
{
	u32 reg_value;

	reg_value = CG_GETREG(CG_RESET_OFS);
	CG_SETREG(CG_RESET_OFS, reg_value | WDT_RST);

	reg_value = CG_GETREG(CG_ENABLE_OFS);
	CG_SETREG(CG_ENABLE_OFS, reg_value | WDT_POS);

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

	sprintf(path,"/top@%x/sensor",IOADDR_TOP_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n",__func__, __LINE__);
		return ;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", &len);
	if (len == 0) {
		printf("%s(%d) len = 0\n",__func__, __LINE__);
		return ;
	}

	sensor_cfg = __be32_to_cpu(cell[0]);
	debug("%s(%d) sensor_cfg = 0x%x\n", __func__, __LINE__, sensor_cfg);

	sprintf(path,"/eth@%x",IOADDR_ETH_REG_BASE);

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	if (nodeoffset < 0) {
		printf("%s(%d) nodeoffset < 0\n", __func__, __LINE__);
		return ;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "sp-clk", &len);
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
		writel((readl(0xFF600400))|(0x1 << 21), 0xFF600400);
		writel((readl(0xFF6001C8))|(0x1 << 31), 0xFF6001C8);
	} else {
		//printf("********************%s(%d) Add USB520 INIT********************************************\n",__func__, __LINE__);
		writel((readl(0xF0600310))|(0x1 << 1), 0xF0600310);
		writel((readl(0xF06001C8))|(0x1 << 5), 0xF06001C8);
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


static void lcd_init(void)
{
	u32 reg_data;
	reg_data = readl(0x2F0020020);

	reg_data &= ~(0x3300);
	reg_data |= (0x1200);
	writel(reg_data, 0x2F0020020);
#if 1
	// lcd210 max clock div = 2
	reg_data = readl(0x2F002004C);

	reg_data &= ~(0x3F);
	reg_data |= (0x1);
	writel(reg_data, 0x2F002004C);
#endif
}


int nvt_ivot_hw_init(void)
{

	nvt_ivot_set_cpuclk();

	ethernet_init();

	axi_bridge_timeout_en();

	usbphy_init();

	lcd_init();

	return 0;
}

int nvt_ivot_hw_init_early(void)
{
	otp_init();
	return 0;
}

#ifdef CONFIG_NVT_IVOT_DDR_RANGE_SCAN_SUPPORT
typedef void (*LDR_GENERIC_CB)(void);
#define JUMP_ADDR 0x2fe000000
void nvt_ddr_scan(u32 type)
{
	int reg = 0;
	int ratio = 0;
	uint64_t	addr = 0xFE000000;
	int current_el = 0;
	register uint64_t x0 __asm__ ("x0");
	addr = *(UINT32 *)JUMP_ADDR;
	ratio = readl(0x2f00244c0|0x20) | (readl(0x2f00244c0|0x24) << 8) | (readl(0x2f00244c0|0x28) << 16);
	ratio = (ratio*12*8)/131072;
	printf("\nDRAM1_");
	if((readl(0x2f0080008)>>27)&0x1) {
		if((readl(0x2f0080008)>>30)&0x1) {
			printf("DDR4x2_");
		} else {
			printf("DDR4x1_");
		}
	} else {
		printf("DDR3_");
	}
	printf("%d_", ratio);

	if((readl(0x2f0080008)>>28)&0x1) {
		printf("16B\n");
	} else {
		printf("32B\n");
	}
	
	if ((readl(0x2f0020010)&0xc00000) != 0x0) {
		ratio = readl(0x2f0024840|0x20) | (readl(0x2f0024840|0x24) << 8) | (readl(0x2f0024840|0x28) << 16);
		ratio = (ratio*12*8)/131072;
		printf("\nDRAM2_");
		if((readl(0x2f0090008)>>27)&0x1) {
			if((readl(0x2f0090008)>>30)&0x1) {
				printf("DDR4x2_");
			} else {
				printf("DDR4x1_");
			}
		} else {
			printf("DDR3_");
		}
		printf("%d_", ratio);

		if((readl(0x2f0090008)>>28)&0x1) {
			printf("16B\n");
		} else {
			printf("32B\n");
		}
	}


	//printf("Loader ini ver = 0x%02x\r\n", readl(0xfe20001c));
//	void (*image_entry)(void) = NULL;
	printf("memcpy->[0x%08x]", (int)JUMP_ADDR);
	if (type == 1) {
		memcpy((void *)JUMP_ADDR, ddr_scan_quick, sizeof(ddr_scan_quick));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan_quick));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan_quick), ARCH_DMA_MINALIGN));
	} else if (type == 2) {
		memcpy((void *)JUMP_ADDR, ddr_scan_phy, sizeof(ddr_scan_phy));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan_phy));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan_phy), ARCH_DMA_MINALIGN));		
	} else {
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
	
	__asm__ ("mrs x0, CurrentEL;" : : : "%x0");	
	current_el = (int)(x0 >> 2);
	printf("Current EL = %d\r\n", (int)current_el);
	if(current_el == 2) {
		printf("Current EL = 2 and use EL1 to execute aarch32\r\n");
		__asm__ volatile (	"MSR SCTLR_EL1, XZR\n"
							"MRS X0, HCR_EL2\n"
							"BIC X0, X0, #(1<<31)\n"
							"MSR HCR_EL2, X0\n"
							"MOV X0, #0b10011\n"
							"MSR SPSR_EL2, X0\n"
							: : :);	
		__asm__ volatile ("	msr ELR_EL2, %0\n"
							"ERET\n"
							: : "r" (addr));	
	} else if(current_el == 3) {
		printf("Current EL = 3 and use EL2 to execute aarch32\r\n");			
		__asm__ volatile (	"MSR SCTLR_EL2, XZR\n"
							"MSR HCR_EL2, XZR\n"
							"MRS X0, SCR_EL3\n"
							"BIC X0, X0, #(1<<10)\n"
							"MSR SCR_EL3, x0\n"						
							"MOV X0, #0b10011\n"
							"MSR SPSR_EL3, X0\n"
							: : :);	
		__asm__ volatile ("	msr ELR_EL3, %0\n"
							"ERET\n"
							: : "r" (addr));	
		
	} else if(current_el == 1) {
		printf("Current EL = 1 and use EL0 to execute aarch32 => not support\r\n");		
	} else {
		printf("Unknow EL ...\r\n");
	}		
	//image_entry = (LDR_GENERIC_CB)(*((unsigned long*)JUMP_ADDR));
	//image_entry();
}
#endif
