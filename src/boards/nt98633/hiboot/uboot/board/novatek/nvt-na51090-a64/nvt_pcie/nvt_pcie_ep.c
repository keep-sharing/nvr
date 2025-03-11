#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/nvt_pcie.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/efuse_protected.h>
#include <pci.h>

#include "nvt_pcie_fdt.h"

#define DRV_VER	"1.00"

static void pcie_set_inbound_region(int bar_id, u64 src_addr, u64 src_upper_addr, u64 target_addr, int region_id, int en)
{
	const uintptr_t BASE = PCIE_DBI_REG_BASE + PF0_ATU_CAP_DBIBaseAddress;

	writel(lower_32_bits(src_addr), BASE+IATU_LWR_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(src_addr), BASE+IATU_UPPER_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(lower_32_bits(src_upper_addr), BASE+IATU_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(src_upper_addr), BASE+IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(lower_32_bits(target_addr), BASE+IATU_LWR_TARGET_ADDR_OFF_INBOUND_0 + 0x200*region_id);
	writel(upper_32_bits(target_addr), BASE+IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0 + region_id*0x200);
	writel(0x00000000, BASE+IATU_REGION_CTRL_1_OFF_INBOUND_0 + region_id*0x200); //[4:0]00000 Memory
	writel((en<<31)|(1<<30)|(bar_id<<8), BASE+IATU_REGION_CTRL_2_OFF_INBOUND_0 + region_id*0x200); //[31]REGION_EN [30] MATCH_MODE [28]CFG_SHIFT_MODE [10..8] BAR_NUM
}

static void pcie_set_outbound_region(u64 src_addr, u64 src_upper_addr, u64 target_addr, int region_id, int atu_type, int en) {
	const uintptr_t BASE = PCIE_DBI_REG_BASE + PF0_ATU_CAP_DBIBaseAddress;

	writel(lower_32_bits(src_addr), BASE+IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(src_addr), BASE+IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(lower_32_bits(src_upper_addr), BASE+IATU_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(src_upper_addr), BASE+IATU_UPPER_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(lower_32_bits(target_addr), BASE+IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(upper_32_bits(target_addr), BASE+IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200);
	writel(atu_type, BASE+IATU_REGION_CTRL_1_OFF_OUTBOUND_0 + region_id*0x200); //[4:0]00100 CFG Type0

	writel((en<<31), BASE+IATU_REGION_CTRL_2_OFF_OUTBOUND_0 + region_id*0x200); //[31]REGION_EN [28]CFG_SHIFT_MODE
}

static void pcie_set_atu(void)
{
	// EP inbound regions
	pcie_set_inbound_region(0, 0x0, 0x10000000, 0x000000000ULL, 0, TRUE);	// BAR0 (MAU): map to internal addr 0
	pcie_set_inbound_region(2, 0x0, 0x10000000, 0x2F0000000ULL, 1, TRUE);	// BAR2 (APB): map to internal addr 0x2_F000_0000

	// EP outbound regions
	pcie_set_outbound_region(0xC00000000ULL, 0xDFFFFFFFFULL, 0xC00000000, 0, PCIE_ATU_TYPE_MEM, TRUE);
	pcie_set_outbound_region(0xE00000000ULL, 0xE0FFFFFFFULL, 0xE00000000, 1, PCIE_ATU_TYPE_MEM, TRUE);
}

static void pcie_phy_settings(void)
{
	u32 reg;

	// DFE vref
        reg = readl(PCIE_PHY_BASE+0xA0);
        reg |= 0x3F << 24;
        reg |= 1 << 30;
        writel(reg, PCIE_PHY_BASE+0xA0);
        reg = readl(PCIE_PHY_BASE+0xA0);

	// txodt
        reg = readl(PCIE_PHY_BASE+0x0C);
        reg &= ~(0x3<<22);
        reg |= 0x2<<22;
        reg |= 1<<20;
        reg |= 1<<21;
        writel(reg, PCIE_PHY_BASE+0x0C);

	// rxodt
        reg = readl(PCIE_PHY_BASE+0x00);
        reg |= 0xFF<<16;
        writel(reg, PCIE_PHY_BASE+0x00);

        // rxqec
        reg = readl(PCIE_PHY_BASE+0x108);
        reg |= 1<<9;
        reg &= ~(0x7<<17);
        reg |= 1<<17;
        writel(reg, PCIE_PHY_BASE+0x108);

	// bypass PTEN RESET
        reg = readl(PCIE_PHY_BASE+0x1070);
        reg |= 1<<8;
        writel(reg, PCIE_PHY_BASE+0x1070);

        // PIPE mode change
        reg = readl(PCIE_PHY_BASE+0x3010);
        reg |= 1<<14;
        writel(reg, PCIE_PHY_BASE+0x3010);

        // pclk deglitch
        reg = readl(PCIE_PHY_BASE+0x1068);
        reg |= 1<<21;
        writel(reg, PCIE_PHY_BASE+0x1068);
        reg = readl(PCIE_PHY_BASE+0x3014);
        reg |= 1<<7;
        writel(reg, PCIE_PHY_BASE+0x3014);

	// CMM
        reg = readl(PCIE_PHY_BASE+0x4060);
        reg &= ~(1<<3);
        writel(reg, PCIE_PHY_BASE+0x4060);

        // LOS comparator
        reg = readl(PCIE_PHY_BASE+0x834);
        reg &= ~(0x0F<<12);
        reg |= 0x04<<12;
        writel(reg, PCIE_PHY_BASE+0x834);

	// disable EIOS detect
        reg = readl(PCIE_PHY_BASE+0x3010);
        reg |= 1<<3;
        writel(reg, PCIE_PHY_BASE+0x3010);

	// T2R idle
        reg = readl(PCIE_PHY_BASE+0x218);
        reg |= 1<<5;
        writel(reg, PCIE_PHY_BASE+0x218);

        reg = readl(PCIE_PHY_BASE+0x1068);
        reg &= ~(1<<20);
        writel(reg, PCIE_PHY_BASE+0x1068);

	// TXCLK gate
	reg = readl(PCIE_PHY_BASE+0x085C);
        reg |= 1<<26;
        writel(reg, PCIE_PHY_BASE+0x085C);

	// fix preset
	reg = readl(PCIE_DBI_REG_BASE+0x08A8);   // force EQ preset 4
        reg &= ~(1<<24);
        reg &= ~(0xFFFF<<8);
        reg |= ((1<<4))<<8;
        writel(reg, PCIE_DBI_REG_BASE+0x08A8);

}

#define DETECT_QUIET	0x00
#define RCVRY_LOCK	0x0D
#define RECVRY_EQ0	0x20
#define RECVRY_EQ1	0x21

#define FOURCC_BOOT 0x544F4F42
static int utl_pcie_ep_load(void)
{
	u16	calc_sum = 0;
	u32	secure_boot;
	u32	last_ltssm = DETECT_QUIET;
	u32	state_start_us = 0;	// unit: us
	int	is_reset_done = 0;

	printf("%s: curr tick 0x%x\r\n", __func__, get_nvt_timer0_cnt());

	// Enable link training (Start LTSSM)
	writel(0x35, PCIE_TOP_REG_BASE+0x304);

	// Never break loop
	while (1) {
		u32 reg;
		u32 curr_ltssm;

		reg = readl(PCIE_TOP_REG_BASE+0x08);
		curr_ltssm = (reg>>4) & 0x3F;
		if (last_ltssm != curr_ltssm) {
			if (last_ltssm == RECVRY_EQ0) {
//				printf("EQ0 elps %d\r\n", timer_getSysTick()-state_start_us);
			}
			state_start_us = get_nvt_timer0_cnt();
			is_reset_done = 0;
		}

		switch (curr_ltssm) {
		case RECVRY_EQ0:
			if ((get_nvt_timer0_cnt() - state_start_us) > 2000) {	// if > 2ms
				// try reset mux
				if (is_reset_done == 0) {
					is_reset_done = 1;

					reg = readl(PCIE_PHY_BASE+0x085C);
					reg &= ~(1<<30);
					writel(reg, PCIE_PHY_BASE+0x085C);

					reg = readl(PCIE_PHY_BASE+0x3014);
	                                reg &= ~(1<<3);
	                                writel(reg, PCIE_PHY_BASE+0x3014);
	                                reg |= (1<<3);
	                                writel(reg, PCIE_PHY_BASE+0x3014);
	                                reg &= ~(1<<3);
	                                writel(reg, PCIE_PHY_BASE+0x3014);
					printf("Reset mux\r\n");
				}
			}
			break;
		default:
			break;
		}

		last_ltssm = curr_ltssm;
	}

	return 0;
}

static int config_pcie_ep(void) {
	u32 val;
	u32 PCIE_VENDOR_ID = 0x1E09;	// novatek
	u32 PCIE_DEV_ID = 0x0005;	// NT9833x
	u32 BAR0_SIZE = 8;		// 2^8 MB

	// 1. stop link training
	writel(0x30, PCIE_TOP_REG_BASE+0x304);

	// 2. release EP register reset (for access DBI)
	val = readl(PCIE_TOP_REG_BASE+0x300);
	val |= 1<<3;
	writel(val, PCIE_TOP_REG_BASE+0x300);

	// bypass PTEN RESET (patch phy)
        val = readl(PCIE_PHY_BASE+0x1070);
        val |= 1<<8;
        writel(val, PCIE_PHY_BASE+0x1070);

	// release other reset
	val = readl(PCIE_TOP_REG_BASE+0x300);
	val |= 0x1F;
	writel(val, PCIE_TOP_REG_BASE+0x300);
	udelay(2);			// wait stable

	// 3. update DBI default value
	val = readl(PCIE_DBI_REG_BASE+0x8BC);	// DBI overwrite
	val |= 1<<0;
	writel(val, PCIE_DBI_REG_BASE+0x8BC);

#if defined(CONFIG_NVT_PCIE_EP_BAR0_SIZE)
	BAR0_SIZE = CONFIG_NVT_PCIE_EP_BAR0_SIZE;
	printf("%s: define BAR0 %d\r\n", __func__, BAR0_SIZE);
	if (BAR0_SIZE > 13) BAR0_SIZE = 8;
#endif
	val = readl(PCIE_DBI_REG_BASE+0x2BC);	// RESBAR_CTRL_REG_0_REG
	val &= ~(0x3F<<8);
	val |= (BAR0_SIZE<<8);		// 2^8 MB => 256MB
	writel(val, PCIE_DBI_REG_BASE+0x2BC);

#if defined(CONFIG_NVT_PCIE_EP_VENDOR_ID)
	PCIE_VENDOR_ID = CONFIG_NVT_PCIE_EP_VENDOR_ID;
	printf("%s: defined vendor id = 0x%x\r\n", __func__, PCIE_VENDOR_ID);
#endif
#if defined(CONFIG_NVT_PCIE_EP_DEV_ID)
	PCIE_DEV_ID = CONFIG_NVT_PCIE_EP_DEV_ID;
	printf("%s: defined dev id = 0x%x\r\n", __func__, PCIE_DEV_ID);
#endif
	val = PCIE_VENDOR_ID & 0xFFFF;
	val |= (PCIE_DEV_ID&0xFFFF)<<16;
	writel(val, PCIE_DBI_REG_BASE);

	val = readl(PCIE_DBI_REG_BASE+0x7C);	// LINK_CAPABILITIES_REG
	val &= ~(0x3<<10);		// PCIE_CAP_ACTIVE_STATE_LINK_PM_SUPPORT = 0
	writel(val, PCIE_DBI_REG_BASE+0x7C);

#if (!IS_ENABLED(CONFIG_NVT_PCIE_EP_BAR2_EN))
	printf("Disable BAR2\r\n");
	writel(0, PCIE_DBI_REG_BASE+0x100018);
	writel(0, PCIE_DBI_REG_BASE+0x10001C);
#else
	printf("BAR2 enabled\r\n");
#endif

	// 4. setup iATU
	pcie_set_atu();

	// 5. phy settings
	pcie_phy_settings();

	utl_pcie_ep_load();
}


/* this function is portable for the customer */
int do_pcie_ep (cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printk("-------------- PCIe EP Version: %s -------------- \n", DRV_VER);

	config_pcie_ep();

	return 0;
}


U_BOOT_CMD(
	nvt_pcie_ep_init,	2,	1,	do_pcie_ep,
	"setup pcie ep mode",
	"command: nvt_pcie_ep_init"
);
