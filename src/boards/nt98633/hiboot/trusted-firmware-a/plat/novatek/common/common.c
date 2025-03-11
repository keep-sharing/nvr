/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <nvt_bin_info.h>
#include <novatek_def.h>
#include <novatek_private.h>

static const mmap_region_t nova_mmap[] = {
	/* GICC + GICD */
	MAP_REGION_FLAT(DEVICE_START_ADDR, (DEVICE_END_ADDR - DEVICE_START_ADDR),
	MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(FDT_BASE, FDT_SIZE,
	MT_MEMORY | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(PLAT_HOLD_BASE, 0x1000,
	MT_NON_CACHEABLE | MT_RW | MT_SECURE),
	{0},
};

unsigned int plat_get_syscnt_freq2(void)
{
	return OSC_CLK_IN_HZ;
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	extern HEADINFO _nvt_headinfo;
	uintptr_t image_ofs = _nvt_headinfo.Resv1[HEADINFO_BL31_RESV_IDX_RTOS_ADDR] ?
			_nvt_headinfo.Resv1[HEADINFO_BL31_RESV_IDX_RTOS_ADDR] :
			_nvt_headinfo.Resv1[HEADINFO_BL31_RESV_IDX_UBOOT_ADDR] ;
	return image_ofs;
#endif
}


void nova_setup_page_tables(uintptr_t total_base, size_t total_size,
							uintptr_t code_start, uintptr_t code_limit,
							uintptr_t rodata_start, uintptr_t rodata_limit
						   )
{
	/*
	 * Map the Trusted SRAM with appropriate memory attributes.
	 * Subsequent mappings will adjust the attributes for specific regions.
	 */
	VERBOSE("Trusted SRAM seen by this BL image: %p - %p\n",
			(void *) total_base, (void *)(total_base + total_size));
	mmap_add_region(total_base, total_base,
					total_size,
					MT_MEMORY | MT_RW | MT_SECURE);

	/* Re-map the code section */
	VERBOSE("Code region: %p - %p\n",
			(void *) code_start, (void *) code_limit);
	mmap_add_region(code_start, code_start,
					code_limit - code_start,
					MT_CODE | MT_SECURE);

	/* Re-map the read-only data section */
	VERBOSE("Read-only data region: %p - %p\n",
			(void *) rodata_start, (void *) rodata_limit);
	mmap_add_region(rodata_start, rodata_start,
					rodata_limit - rodata_start,
					MT_RO_DATA | MT_SECURE);

	mmap_add(nova_mmap);
	init_xlat_tables();

}
