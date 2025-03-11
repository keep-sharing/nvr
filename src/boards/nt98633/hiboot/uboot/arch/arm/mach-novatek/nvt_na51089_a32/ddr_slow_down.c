/**
    NVT utilities for ddr slow down

    @file       ddr_slow_down.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2020.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/
#include <common.h>
#include <asm/io.h>
#include <asm/nvt-common/nvt_types.h>
#include <compiler.h>
#include <stdlib.h>
#include <asm/armv7.h>
#ifdef CONFIG_NVT_IVOT_DDR_SLOW_DOWN_SUPPORT
#include <asm/arch/raw_slowDDR.h>

#define L2_MEM_BASE			(CONFIG_SYS_PL310_BASE)
#define L2_REG1_BASE			(L2_MEM_BASE + 0x100)   /* Control */
#define L2_REG1_CONTROL			(*((volatile unsigned long *)(L2_REG1_BASE + 0x00)))
#define K_L2_REG1_CONTROL_EN_ON		1
#define K_L2_REG1_CONTROL_EN_OFF	0
#define S_L2_REG1_CONTROL_EN		(0)


typedef void (*LDR_GENERIC_CB)(void);
#define JUMP_ADDR 0xf07C0000

int nvt_ddr_slow_down(void)
{
	u32     iLibVersion;
	u32 iLibDRAMVersion;
	int      YY,MM,DD,VER;
	int      LD_MJ, LD_MIN, VER_Fix;

	uint32_t reg;
	int is_icache_en = 0, is_dcache_en = 0, is_mmu_en = 0;
	void (*image_entry)(void) = NULL;

	iLibVersion = *(UINT32 *)(ddr_slow + (0x74/4));
	iLibDRAMVersion = *(UINT32 *)(ddr_slow + (0x78/4));

	YY = 2020 + (iLibDRAMVersion & 0x7);
	MM = ((iLibDRAMVersion >> 3) & 0xF);
	DD = ((iLibDRAMVersion >> 7) & 0x1F);
	VER= ((iLibDRAMVersion >> 12) & 0xF);
	VER_Fix = ((iLibVersion >> 16) & 0xFF);
	LD_MIN = ((iLibVersion >> 24) & 0xF);
	LD_MJ = ((iLibVersion >> 28) & 0xF);
	printf("\r\n");
	printf("DR[%d-%d-%d-%d]:", YY, MM, DD, VER);
	printf("LD[%d.%d.%d]\r\n", LD_MJ, LD_MIN, VER_Fix);

	printf("memcpy->[0x%08x]", (int)JUMP_ADDR);
	memcpy((void *)JUMP_ADDR, ddr_slow, sizeof(ddr_slow));
	flush_dcache_range(JUMP_ADDR,
		JUMP_ADDR + ALIGN(sizeof(ddr_slow) + ARCH_DMA_MINALIGN - 1, ARCH_DMA_MINALIGN));
	printf("flush->");
	invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_slow), ARCH_DMA_MINALIGN));

	reg = get_cr();	/* get control reg. */
	if (reg & CR_M) {
//		printf("detect MMU enabled\r\n");
		is_mmu_en = 1;
	}
	is_dcache_en = dcache_status();
	if (is_dcache_en) {
//		printf("detect L1 D cache enabled\r\n");
		dcache_disable();
	}
	if (L2_REG1_CONTROL & (K_L2_REG1_CONTROL_EN_ON << S_L2_REG1_CONTROL_EN)) {
//		printf("detect L2 enabled: 0x%x\r\n", L2_REG1_CONTROL);
		v7_outer_cache_disable();
	}
	invalidate_dcache_all();
	is_icache_en = icache_status();
	if (is_icache_en) {
//		printf("detect I cached enabled\r\n");
		icache_disable();
		invalidate_icache_all();
	}
	if (is_mmu_en) {
		set_cr(reg & ~CR_M);
	}

	printf("Jump into sram\n");
	image_entry = (LDR_GENERIC_CB)(*((unsigned long*)JUMP_ADDR));
	image_entry();

	if (is_dcache_en) {
		dcache_enable();
	}
	if (is_icache_en) {
		icache_enable();
	}

        return 0;
}

static int do_nvt_ddr_slow_down(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return nvt_ddr_slow_down();
}
U_BOOT_CMD(
        nvt_ddr_slow_down, 1,        1,      do_nvt_ddr_slow_down,
        "nvt ddr slow down",
        ""
);
#endif
