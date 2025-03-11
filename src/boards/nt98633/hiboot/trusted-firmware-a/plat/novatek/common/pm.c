/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

extern uint64_t nova_sec_entry_point;
extern void plat_cpu_wait_to_boot(void);

#define mpidr_is_valid(mpidr) ( \
	MPIDR_AFFLVL3_VAL(mpidr) == 0 && \
	MPIDR_AFFLVL2_VAL(mpidr) == 0 && \
	MPIDR_AFFLVL1_VAL(mpidr) < PLATFORM_CLUSTER_COUNT && \
	MPIDR_AFFLVL0_VAL(mpidr) < PLATFORM_MAX_CPUS_PER_CLUSTER)

void nova_cpu_off(u_register_t mpidr)
{
}

static int nova_pwr_domain_on(u_register_t mpidr)
{
	unsigned int pos = plat_core_pos_by_mpidr(mpidr);
	uint64_t *hold_base = (uint64_t *)PLAT_HOLD_BASE;

	if (mpidr_is_valid(mpidr) == 0)
		return PSCI_E_INTERN_FAIL;

	assert(pos < PLATFORM_CORE_COUNT);

	hold_base[pos] = PLAT_HOLD_STATE_GO;

	/* Make sure that the write has completed */
	dsb();
	isb();

	sev();

	return PSCI_E_SUCCESS;
}

static void nova_pwr_domain_off(const psci_power_state_t *target_state)
{
	gicv2_cpuif_disable();
}

static void __dead2 nova_pwr_down_wfi(const psci_power_state_t *target_state)
{
	nova_cpu_off(read_mpidr());

	while (1)
		plat_cpu_wait_to_boot();
}

static void nova_pwr_domain_on_finish(const psci_power_state_t *target_state)
{

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

static int nova_validate_ns_entrypoint(uintptr_t ns_entrypoint)
{
	return PSCI_E_SUCCESS;
}

static plat_psci_ops_t nova_psci_ops = {
	.pwr_domain_on			= nova_pwr_domain_on,
	.pwr_domain_off			= nova_pwr_domain_off,
	.pwr_domain_pwr_down_wfi	= nova_pwr_down_wfi,
	.pwr_domain_on_finish		= nova_pwr_domain_on_finish,

	.validate_ns_entrypoint		= nova_validate_ns_entrypoint,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{

	nova_sec_entry_point = sec_entrypoint;
	flush_dcache_range((uint64_t)&nova_sec_entry_point, sizeof(uint64_t));

	/*
	* Initialize PSCI ops struct
	*/
	*psci_ops = &nova_psci_ops;
	return 0;
}
