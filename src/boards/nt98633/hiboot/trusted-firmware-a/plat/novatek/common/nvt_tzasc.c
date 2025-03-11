/*
 * Copyright (c) 2021, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <arch_helpers.h>
#include <common/debug.h>
#include <assert.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <nvt_tzasc_int.h>
#include <nvt_tzasc_reg.h>

#define DRAM2_STARTING_PHYS_ADDRESS		0x100000000

#define PROT_START_OFS(ch)				(MAU_PROTECT0_START_ADDR + ch * 8)
#define PROT_END_OFS(ch)				(MAU_PROTECT0_END_ADDR + ch * 8)
#define PROT_EN0_OFS(ch)				(MAU_IN_RANGE_PROTECT0_ENABLE_OFS0 + ch * 8)
#define PROT_EN1_OFS(ch)				(MAU_IN_RANGE_PROTECT0_ENABLE_OFS1 + ch * 8)

/*
 * Implementation defined values used to validate inputs later.
 * Filters : max of 6 ; 0 to 5
 * Regions : max of 4 ; 0 to 3
 * Address width : Values between 32 to 64
 */
struct tzc_instance {
	uint32_t addr_width;
	uint32_t num_filters;
	uint32_t num_regions;
};

static struct tzc_instance tzc;

static uint64_t arb_base_addr(DDR_ARB ddr_id)
{
	if (ddr_id == DDR_ARB_1) {
		return DDR_ARB_REG_BASE;
	} else {
		return DDR_ARB2_REG_BASE;
	}
}

static void arb_set_reg(DDR_ARB ddr_id, uint32_t ofs, uint32_t value)
{
	mmio_write_32((arb_base_addr(ddr_id) + ofs), value);
}

static uint32_t arb_get_reg(DDR_ARB ddr_id, uint32_t ofs)
{
	return mmio_read_32((arb_base_addr(ddr_id) + ofs));

}

#if 0
static paddr_t nvt_tzasc_virt_to_phys(vaddr_t addr)
{
	if (cpu_mmu_enabled()) {
		return virt_to_phys((void *)addr);
	} else {
		return addr;
	}
}
#endif

void tzc_enable_filters(DDR_ARB id, DMA_WRITEPROT_SET set, DMA_WRITEPROT_ATTR *p_attr)
{
	//unsigned long lock = 0;
	uint32_t v_mask[DMA_CH_GROUP_CNT];

	T_MAU_INT_ENABLE_REG intctrl_reg = {0};
	T_MAU_INT_STATUS_REG sts_reg;
	T_MAU_PROTECT_MODE_REG mode;

	if (id > DDR_ARB_2) {
		ERROR("invalid arb ch %d\r\n", id);
		return;
	}

	if (set > WPSET_4) {
		ERROR("invalid protect ch %d\r\n", set);
		return;
	}

	if (p_attr->starting_addr & 0x7f) {
		ERROR("starting addr not 32 word aligned 0x%08x\r\n", (unsigned int)p_attr->starting_addr);
		return;
	}

	mode.reg = arb_get_reg(id, MAU_PROTECT_MODE_OFS);
	memcpy(v_mask, &p_attr->mask, sizeof(DMA_CH_MSK));

	//loc_multi_cores(lock);

	// disable wp int
	intctrl_reg.reg = arb_get_reg(id, MAU_INT_ENABLE_OFS);
	intctrl_reg.reg &= ~0xffff;
	arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.reg);

	// clr status
	sts_reg.reg = 0xffff;
	arb_set_reg(id, MAU_INT_STATUS_OFS, sts_reg.reg);

	// set addr
	if (set > WPSET_4) {
		arb_set_reg(id, MAU_OUT_PROTECT_START_ADDR, p_attr->starting_addr);
		arb_set_reg(id, MAU_OUT_PROTECT_END_ADDR, p_attr->starting_addr + p_attr->size - 1);
	} else {
		arb_set_reg(id, PROT_START_OFS(set), p_attr->starting_addr);
		arb_set_reg(id, PROT_END_OFS(set), p_attr->starting_addr + p_attr->size - 1);
	}

	if (set == WPSET_0) {
		mode.bit.WP0_MODE = p_attr->level;
	} else if (set == WPSET_1) {
		mode.bit.WP1_MODE = p_attr->level;
	} else if (set == WPSET_2) {
		mode.bit.WP2_MODE = p_attr->level;
	} else if (set == WPSET_3) {
		mode.bit.WP3_MODE = p_attr->level;
	} else if (set == WPSET_4) {
		mode.bit.WP4_MODE = p_attr->level;
	} else {
		mode.bit.OUT_WP_MODE = p_attr->level;
	}
	arb_set_reg(id, MAU_PROTECT_MODE_OFS, mode.reg);

	// set en
	arb_set_reg(id, PROT_EN0_OFS(set), v_mask[0]);
	arb_set_reg(id, PROT_EN1_OFS(set), v_mask[1]);

	// enable interrupt
//	intctrl_reg.reg |= 0xffff;
//	arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.reg);

	//unl_multi_cores(lock);
}

void tzc_disable_filters(DDR_ARB id, DMA_WRITEPROT_SET set)
{
	//uint32_t lock = 0;
	T_MAU_INT_ENABLE_REG intctrl_reg = {0};
	T_MAU_INT_STATUS_REG sts_reg;

	if (id > DDR_ARB_2) {
            ERROR("invalid arb ch %d\r\n", id);
            return;
    }

    if (set > WPSET_4) {
            ERROR("invalid protect ch %d\r\n", set);
            return;
    }

	//loc_multi_cores(lock);

	// disable wp
	arb_set_reg(id, PROT_EN0_OFS(set), 0);
	arb_set_reg(id, PROT_EN1_OFS(set), 0);

    // disable inten
    intctrl_reg.reg = arb_get_reg(id, MAU_INT_ENABLE_OFS);
    intctrl_reg.reg &= ~0xffff;
    arb_set_reg(id, MAU_INT_ENABLE_OFS, intctrl_reg.reg);

    // ensure int sts is cleared
    sts_reg.reg = 0xffff;
    arb_set_reg(id, MAU_INT_STATUS_OFS, sts_reg.reg);

	//unl_multi_cores(lock);
}

void nvt_tzasc_init(void)
{
	/* Save values we will use later. */
	tzc.num_filters = WPSET_COUNT;
	tzc.addr_width  = 36;
	tzc.num_regions = 1;
}


/**
    nvt_wp_configure_region is use to protected specific area of memory to avoid non secure access

    nvt_wp_configure_region is used to program regions into the TrustZone.
    Sush as ARM provided (TZASC:TrustZone Address Space Controller). There are
    6 protect set, 4 region in each set.
    @note (Region can not cross between DRAM1 & DRAM2)
    DRAM1
    +-----+========>each set N within 4 regions
    |set0 |-----+  +-------+
    +-----+	    +->|region0|
    | --- |  	+  +-------+
    | --- |  	+->  ....
    +-----+  	+    ....
    |set5 |	 	+  +-------+
    +-----+	 	+->|region3|
                   +-------+

    DRAM2
    +-----+========>each set N within 4 regions
    |set0 |-----+  +-------+
    +-----+	    +->|region0|
    | --- |  	+  +-------+
    | --- |  	+->  ....
    +-----+  	+    ....
    |set5 |	 	+  +-------+
    +-----+	 	+->|region3|
                   +-------+

    @param[in] filter_set               memory protect function set
    @param[in] region                   memory protect region in a set
    @param[in] region_base              physical start address of memory protect region of a set (need 4 words alignment)
    @param[in] region_size              size of memory protect region of a set (need 4 words alignment)
    @param[in] sec_protected_level      protect policy(un-write/un-read/un-both w/r)
    @param[in] allow_ns_device_access   1: allow REE non secure access, 0: Not allow REE non secure access
	Example:
    @code
    {
    	vaddr_t addr[2] = {0};
        addr[0] = core_mmu_get_va(TZASC_BASE, MEM_AREA_IO_SEC);
        addr[1] = core_mmu_get_va(TZASC2_BASE, MEM_AREA_IO_SEC);
		//use WPSET_0 / region0 / from addr[0] ~ addr[0]+size / UR&UW / Not allow REE access
        nvt_wp_configure_region_enable(WPSET_0, DMA_PROT_RGN0, addr[0], addr[0]_size, DMA_RWPLEL_UNRW, 0);
        //use WPSET_0 / region1 / from addr[0] ~ addr[0]+size / UR&UW / Not allow REE access
        nvt_wp_configure_region_enable(WPSET_0, DMA_PROT_RGN1, addr[1], addr[1]_size, DMA_RWPLEL_UNRW, 0);

    }
    @endcode
 */
void nvt_wp_configure_region_enable(DMA_WRITEPROT_SET filter_set,
									uint64_t     region_base,
									uint32_t    region_size,
									DMA_WRITEPROT_LEVEL sec_protected_level,
									uint32_t allow_ns_device_access)
{
	DDR_ARB id;
	DMA_WRITEPROT_ATTR  protect_attr = {0};
	/* Do range checks on filters and regions. */
	assert((filters < tzc.num_filters) && (region < tzc.num_regions));
	memset(&(protect_attr.mask), 0x0, sizeof(protect_attr.mask));
	protect_attr.mask.bCPU_NS = ~allow_ns_device_access;
	protect_attr.level = sec_protected_level;
	protect_attr.starting_addr = region_base;
	protect_attr.size = region_size;

	if(protect_attr.starting_addr >= DRAM2_STARTING_PHYS_ADDRESS) {
		id = DDR_ARB_2;
		protect_attr.starting_addr -= DRAM2_STARTING_PHYS_ADDRESS;
	} else {	// starting_addr @ DDR1 but size exceed DDR1
		if((protect_attr.starting_addr + protect_attr.size) >= DRAM2_STARTING_PHYS_ADDRESS) {
			ERROR("Address cross DDR1 & DDR2 physical addr[0x%08x] ~ addr[0x%08x]\r\n", (int)protect_attr.starting_addr, (int)(protect_attr.starting_addr+protect_attr.size));
			return;
		}
		id = DDR_ARB_1;
	}

	//protect_attr.protect_rgn_attr[region].en = ~allow_ns_device_access;

 	if(allow_ns_device_access == 0)	{//enable WP
		tzc_enable_filters(id, filter_set, &protect_attr);
	} else {
		tzc_disable_filters(id, filter_set);
	}

}

void nvt_tzasc_config(void)
{
	NOTICE("TZASC config:\n");
	NOTICE("\r\t0x%08x@0x%08x \t(RW/N)\tRee\n", BL31_SIZE, BL31_BASE);
	nvt_wp_configure_region_enable(WPSET_2, BL31_BASE, BL31_SIZE, DMA_WP, 0);
	nvt_wp_configure_region_enable(WPSET_3, BL31_BASE, BL31_SIZE, DMA_RP, 0);
#ifdef BOOT_AFTER_UBOOT
#if USE_OPTEE >= 1
	NOTICE("\r\t0x%08x@0x%08x \t(RW/N)\tRee\n", BL32_SIZE, BL32_BASE);
	nvt_wp_configure_region_enable(WPSET_0, BL32_BASE, BL32_SIZE, DMA_WP, 0);
	nvt_wp_configure_region_enable(WPSET_1, BL32_BASE, BL32_SIZE, DMA_RP, 0);
#endif
#endif
}

#if TRACE_LEVEL >= TRACE_DEBUG
#if 0
static uint32_t tzc_read_region_attributes(vaddr_t base, uint32_t region)
{
	return mmio_read_32(base + REGION_ATTRIBUTES_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_base_low(vaddr_t base, uint32_t region)
{
	return mmio_read_32(base + REGION_BASE_LOW_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_base_high(vaddr_t base, uint32_t region)
{
	return mmio_read_32(base + REGION_BASE_HIGH_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_top_low(vaddr_t base, uint32_t region)
{
	return mmio_read_32(base + REGION_TOP_LOW_OFF + REGION_NUM_OFF(region));
}

static uint32_t tzc_read_region_top_high(vaddr_t base, uint32_t region)
{
	return mmio_read_32(base + REGION_TOP_HIGH_OFF + REGION_NUM_OFF(region));
}

#define REGION_MAX      8
static const __maybe_unused char *const tzc_attr_msg[] = {
	"TZC_REGION_S_NONE",
	"TZC_REGION_S_RD",
	"TZC_REGION_S_WR",
	"TZC_REGION_S_RDWR"
};

void tzc_dump_state(void)
{
	uint32_t n;
	uint32_t temp_32reg, temp_32reg_h;

	DMSG("enter");
	for (n = 0; n <= REGION_MAX; n++) {
		temp_32reg = tzc_read_region_attributes(tzc.base, n);
		if (!(temp_32reg & REG_ATTR_F_EN_MASK)) {
			continue;
		}

		DMSG("\n");
		DMSG("region %d", n);
		temp_32reg = tzc_read_region_base_low(tzc.base, n);
		temp_32reg_h = tzc_read_region_base_high(tzc.base, n);
		DMSG("region_base: 0x%08x%08x", temp_32reg_h, temp_32reg);
		temp_32reg = tzc_read_region_top_low(tzc.base, n);
		temp_32reg_h = tzc_read_region_top_high(tzc.base, n);
		DMSG("region_top: 0x%08x%08x", temp_32reg_h, temp_32reg);
		temp_32reg = tzc_read_region_attributes(tzc.base, n);
		DMSG("secure rw: %s",
			 tzc_attr_msg[temp_32reg >> REG_ATTR_SEC_SHIFT]);
		if (temp_32reg & (1 << 0)) {
			DMSG("filter 0 enable");
		}
		if (temp_32reg & (1 << 1)) {
			DMSG("filter 1 enable");
		}
		if (temp_32reg & (1 << 2)) {
			DMSG("filter 2 enable");
		}
		if (temp_32reg & (1 << 3)) {
			DMSG("filter 3 enable");
		}
	}
	DMSG("exit");
}
#endif
#endif /* CFG_TRACE_LEVEL >= TRACE_DEBUG */



