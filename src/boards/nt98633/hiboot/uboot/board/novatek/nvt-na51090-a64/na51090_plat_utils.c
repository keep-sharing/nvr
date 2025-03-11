/**
    NVT utilities for command customization

    @file       na51090_plat_utils.c
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/IOAddress.h>

#define RAM_SRC_SELECT_OFFSET 0x20

static char nvt_ddr_remap_help_text[] =
"\n"
"nvt_ddr_remap 1 - DDR1 base address on 2G bits\n"
"nvt_ddr_remap 2 - DDR1 base address on 4G bits\n"
"nvt_ddr_remap 3 - DDR1 base address on 8G bits\n"
"nvt_ddr_remap 4 - DDR1 base address on 16G bits\n"
"";

static int do_nvt_ddr_remap(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned int reg;
	unsigned int val;

	if (argc < 2) {
		return -1;
	}
	val = (unsigned int)simple_strtoul(argv[1], NULL, 16);

	reg = readl(IOADDR_TOP_REG_BASE + RAM_SRC_SELECT_OFFSET);
	reg |= 1 << (val + 3);
	writel(reg, IOADDR_TOP_REG_BASE + RAM_SRC_SELECT_OFFSET);
	return 0;
}

U_BOOT_CMD(
		nvt_ddr_remap, 2,    1,      do_nvt_ddr_remap,
		"Represent DRAMx select",
		nvt_ddr_remap_help_text
	  );


