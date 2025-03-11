/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <novatek_def.h>
#include <novatek_private.h>

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static console_16550_t console;

static const gicv2_driver_data_t gic_data = {
	.gicd_base = GICD_BASE,
	.gicc_base = GICC_BASE,
};

/*
 * Try to find a DTB loaded in memory by previous stages.
 *
 * At the moment we implement a heuristic to find the DTB attached to U-Boot:
 * U-Boot appends its DTB to the end of the image. Assuming that BL33 is
 * U-Boot, try to find the size of the U-Boot image to learn the DTB address.
 * The generic ARMv8 U-Boot image contains the load address and its size
 * as u64 variables at the beginning of the image. There might be padding
 * or other headers before that data, so scan the first 2KB after the BL33
 * entry point to find the load address, which should be followed by the
 * size. Adding those together gives us the address of the DTB.
 */
static void *find_dtb(void)
{
	uint32_t *dtb_base;

	dtb_base = (void *)FDT_BASE;

	if (fdt_check_header(dtb_base) != 0) {
		NOTICE("FDT check fail.\n");
		return NULL;
	}
	return dtb_base;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
								u_register_t arg2, u_register_t arg3)
{
	/* Initialize the debug console as soon as possible */
	console_16550_register(UART0_BASE, UART0_CLK_IN_HZ,
						   UART0_BAUDRATE, &console);

#if USE_OPTEE >= 1
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = mmio_read_64(BL32_BASE + 0x100);
	INFO("BL31: Have optee, jump to 0x%lx\n", bl32_image_ep_info.pc);
	bl32_image_ep_info.spsr = 0;
#endif

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX,
									  DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

}

void bl31_plat_arch_setup(void)
{
	plat_tmr_init();

	mmio_write_32(REG_RELEASE_ADDR, BL31_BASE);
	/* set TrustZone */
	mmio_write_64(TZPC_BASE, 0);
	mmio_write_64(TZPC_BASE + 0x8, 0);
	mmio_write_64(TZPC_BASE + 0x10, 0);
	mmio_write_64(TZPC_BASE + 0x18, 0);


	nova_setup_page_tables(BL31_BASE, (PLAT_HOLD_BASE - BL31_BASE),
						   BL_CODE_BASE, BL_CODE_END,
						   BL_RO_DATA_BASE, BL_RO_DATA_END
						  );
	enable_mmu_el3(0);
}

void bl31_platform_setup(void)
{
	void *fdt;
	flush_dcache_range(FDT_BASE, FDT_SIZE);

	fdt = find_dtb();
	if (fdt) {
		NOTICE("BL31: Found DTB\n");
	} else {
		NOTICE("BL31: No DTB found.\n");
	}

	generic_delay_timer_init();
#ifndef _NVT_FPGA_EMULATION_ON_
	nvt_tzasc_config();
#endif

	/* Configure the interrupt controller */
	gicv2_driver_init(&gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();

	INFO("BL31: Platform setup done\n");
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type) != 0);

	if (type == NON_SECURE) {
		return &bl33_image_ep_info;
	}

	if ((type == SECURE) && bl32_image_ep_info.pc) {
		return &bl32_image_ep_info;
	} else {
		NOTICE("not find OPTEE, if system not have OPTEEe, don't care OPTEE error or warning message\n");
	}
	return NULL;
}
