/**
    NVT ddr
    @file       nvt_ddr.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2016.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/


#include <common.h>
#include <asm/nvt-common/rcw_macro.h>
#include <errno.h>
#include "raw_scanMP.h"

typedef void (*LDR_GENERIC_CB)(void);
#define JUMP_ADDR 0xFE080000

int nvt_ddr_scan(u32 hvy)
{
	int ratio = 0;

	ratio = readl(0xfe0104c0|0x20) | (readl(0xfe0104c0|0x24) << 8) | (readl(0xfe0104c0|0x28) << 16);
	ratio = (ratio*12*8)/131072;

	if((readl(0xfc400008)>>27)&0x1) {
		if((readl(0xfc400008)>>30)&0x1) {
			printf("\nDDR4x2_");
		} else {
			printf("\nDDR4x1_");
		}
	} else {
		printf("\nDDR3_");
	}
	printf("%d_", ratio);

	if((readl(0xfc400008)>>28)&0x1) {
		printf("16B_");
	} else {
		printf("32B_");
	}


	if((readl(0xfc400008)>>27)&0x1) {
		writel(2,0xfc4083f8);
		if((readl(0xfc408144)&0xf0) == 0x10) {
			printf("UBT\r\n");
		} else if ((readl(0xfc408144)&0xf0) == 0x30) {
			printf("FB\r\n");
		}
	} else {
		if((readl(0xfc400384)&0xf) == 0x9) {
			printf("UBT\r\n");
		} else if ((readl(0xfc400384)&0xf) == 0xa) {
			printf("FB\r\n");
		}
	}

	printf("Loader ini ver = 0x%02x\r\n", readl(0xfe20001c));
	void (*image_entry)(void) = NULL;
	printf("memcpy->[0x%08x]", (int)JUMP_ADDR);
	if (hvy == 1) {
		memcpy((void *)JUMP_ADDR, ddr_scan_hvy, sizeof(ddr_scan_hvy));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan_hvy));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan_hvy), ARCH_DMA_MINALIGN));
	} else if (hvy == 2) {
		memcpy((void *)JUMP_ADDR, ddr_phy_scan, sizeof(ddr_phy_scan));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_phy_scan));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_phy_scan), ARCH_DMA_MINALIGN));		
	} else {
		memcpy((void *)JUMP_ADDR, ddr_scan, sizeof(ddr_scan));
		printf("->done\n");
		flush_dcache_range(JUMP_ADDR, JUMP_ADDR + sizeof(ddr_scan));
		printf("flush->");
		invalidate_dcache_range(JUMP_ADDR, JUMP_ADDR + roundup(sizeof(ddr_scan), ARCH_DMA_MINALIGN));
	}
    printf("done\r\n");
	printf("Jump into sram\n");
	image_entry = (LDR_GENERIC_CB)(*((unsigned long*)JUMP_ADDR));
	image_entry();

	return 0;
}

int do_nvt_ddr_scan(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2) {
		nvt_ddr_scan(0);
	} else {
		nvt_ddr_scan(simple_strtoul(argv[1], NULL, 16));
	}


	return 0;
}
U_BOOT_CMD(
	nvt_ddr_scan, 2,	1,	do_nvt_ddr_scan,
	"nvt ddr scan",
	""
);