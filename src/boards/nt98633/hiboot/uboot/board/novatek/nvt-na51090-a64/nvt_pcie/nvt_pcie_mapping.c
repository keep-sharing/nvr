#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
#include <stdlib.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/nvt_pcie.h>
#include <asm/arch/IOAddress.h>
#include <asm/arch/efuse_protected.h>
#include <asm/arch/hardware.h>
#include <pci.h>

#include "nvt_pcie_fdt.h"

#define CC_RC_EP_BOOT_COMM_REG 0x620160120

#if defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64)

#define DRV_VER	"0.21"

struct nvt_pci_ctrl {
	phys_addr_t dbi_base;		// DBI base (virtual address)
	phys_addr_t atu_base;		// ATU base (virtual address)
	phys_addr_t cfg_base;		// CFG base (virtual address)
	phys_addr_t phy_base;		// PHY base (virtual address)
	phys_addr_t cg_base;		// CG base (virtual address)
	struct pci_region inbound[MAX_PCI_REGIONS];
	struct pci_region outbound[MAX_PCI_REGIONS];
	int region_count;
	int mode;	// NVT_PCIE_RC or NVT_PCIE_EP
	int last_atu;	// record last accessed ATU type (for EP multiplex)

	int (*read_dbi)(struct nvt_pci_ctrl*, int reg, u32 *);
	int (*write_dbi)(struct nvt_pci_ctrl*, int reg, u32);

	int (*read_cfg)(struct nvt_pci_ctrl*, int reg, u32 *);
	int (*write_cfg)(struct nvt_pci_ctrl*, int reg, u32);

	int (*read_atu)(struct nvt_pci_ctrl*, int reg, u32 *);
	int (*write_atu)(struct nvt_pci_ctrl*, int reg, u32);

	void (*config_oATU)(struct nvt_pci_ctrl*, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, int atu_type, bool en);
	void (*config_iATU)(struct nvt_pci_ctrl*, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, bool en);
};

static u32 tx_boost = 0;
static u32 auto_preset = 0;	// 0: fix preset, 1: auto preset
static u32 en_l1_detect = 0;

#define pcie_ctrl_readl_ep    pcie_ctrl_readl_rc
#define pcie_ctrl_writel_ep   pcie_ctrl_writel_rc
#define ioread32        readl
#define iowrite32       writel
#define read_reg		readl
#define write_reg(base, val)	writel(val, base)

#define CPU_CFG_MAP_VA		(0x600000000)
#define CPU_ATU_MAP_VA		(0x610000000)
#define CPU_PCIE_MAP_SIZE	(0x10000000)
#define PCIE_CFG_ATU_PA		(CPU_ATU_MAP_VA)

static void ep_cfg_write(unsigned int val, phys_addr_t base)
{
	iowrite32(val, (void *)(CPU_CFG_MAP_VA + base));
}

static u32 ep_cfg_read(phys_addr_t base)
{
	u32 data;

	data = ioread32((void *)(CPU_CFG_MAP_VA + base));

	return data;
}

/* local function declaration */
static void pcie_ctrl_readl_rc(phys_addr_t base, unsigned int *val)
{
	*val = ioread32((void *)base);
}

static void pcie_ctrl_writel_rc(unsigned int val, phys_addr_t base)
{
	iowrite32(val, (void *)base);
}

static u32 rc_apbm_src(phys_addr_t addr, u32 rdata_exp, u32 mask) {
	u32 data;

	pcie_ctrl_readl_rc(addr, &data);

	if ((data&mask) == (rdata_exp&mask)) {
		printf("%s:  compare pass, both 0x%x\r\n", __func__, rdata_exp);
		return 0;
	} else {
		printf("%s:  compare err, read 0x%x, expect 0x%x\r\n", __func__, data, rdata_exp);
		return 1;
	}
}


static void rc_apbm_srmw(phys_addr_t addr, u32 data, u32 start, u32 width) {
	const u32 MASK = (1<<width) - 1;
	u32 data_tmp;

	pcie_ctrl_readl_rc(addr, &data_tmp);
	data_tmp = (data_tmp & (~(MASK << start))) | ((data & MASK) << start);
	pcie_ctrl_writel_rc(data_tmp, addr);
}


static void rc_dbi_srmw(phys_addr_t reg, u32 data, u32 start, u32 width) {
	rc_apbm_srmw(PCIE_DBI_REG_BASE+reg, data, start, width);
}

static u32 rc_dbi_srp(phys_addr_t reg) {
	u32 data;

	pcie_ctrl_readl_rc((PCIE_DBI_REG_BASE+reg), &data);

	return data;
}


static void rc_dbi_sw(phys_addr_t reg, u32 data) {
	printf("%s: addr 0x%x, data 0x%x\r\n", __func__, PCIE_DBI_REG_BASE+reg, data);
	pcie_ctrl_writel_rc(data, PCIE_DBI_REG_BASE+reg);
}

static u32 rc_apbm_sr(phys_addr_t addr) {
	u32 data;

	pcie_ctrl_readl_rc((addr), &data);

	return data;
}

static void rc_slv_sw(phys_addr_t addr, u32 data) {
	pcie_ctrl_writel_rc(data, addr);
}

static u32 rc_slv_src(phys_addr_t addr, u32 rdata_exp, u32 mask) {
	return rc_apbm_src(addr, rdata_exp, mask);
}

static void config_oATU(struct nvt_pci_ctrl* ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, int atu_type, bool en) {
	uint32_t shift_mode = 0;

	if (ctrl == NULL) {
		printf("%s: NULL ctrl\r\n", __func__);
	}

	// If RC, ATU base is 0x2FD300000
	// If EP, ATU base is 0x610000000
	ctrl->write_atu(ctrl, IATU_LWR_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_BASE_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_addr>>32);
	ctrl->write_atu(ctrl, IATU_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_upper_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_LIMIT_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		src_upper_addr>>32);
	ctrl->write_atu(ctrl, IATU_LWR_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		target_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_TARGET_ADDR_OFF_OUTBOUND_0 + region_id*0x200,
		target_addr>>32);
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_1_OFF_OUTBOUND_0 + region_id*0x200,    atu_type); //[4:0]00100 CFG Type0

	if (atu_type == PCIE_ATU_TYPE_CFG0) {
		atu_type |= 1<<28;
	}
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_2_OFF_OUTBOUND_0 + region_id*0x200,    (en<<31)); //[31]REGION_EN [28]CFG_SHIFT_MODE
}

static void config_iATU(struct nvt_pci_ctrl* ctrl, uint64_t src_addr, uint64_t src_upper_addr, uint64_t target_addr, int region_id, bool en) {
	if (ctrl == NULL) {
		printf("%s: NULL ctrl\r\n", __func__);
	}

	// If RC, ATU base is 0x2FD300000
	// If EP, ATU base is 0x610000000
	ctrl->write_atu(ctrl, IATU_LWR_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_BASE_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_addr>>32);
	ctrl->write_atu(ctrl, IATU_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_upper_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_LIMIT_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		src_upper_addr>>32);
	ctrl->write_atu(ctrl, IATU_LWR_TARGET_ADDR_OFF_INBOUND_0 + 0x200*region_id,
		target_addr&0xFFFFFFFF);
	ctrl->write_atu(ctrl, IATU_UPPER_TARGET_ADDR_OFF_INBOUND_0 + region_id*0x200,
		target_addr>>32);
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_1_OFF_INBOUND_0 + region_id*0x200,
		0x00000000|(1<<13)); //[4:0]00000 Memory+[13]INCREASE_REGION_SIZE
	ctrl->write_atu(ctrl, IATU_REGION_CTRL_2_OFF_INBOUND_0 + region_id*0x200,    (en<<31)); //[31]REGION_EN [28]CFG_SHIFT_MODE
}

static int rc_read_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

	pcie_ctrl_readl_rc((ctrl->dbi_base+reg), val);

	return 0;
}

static int rc_write_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

	pcie_ctrl_writel_rc(val, (ctrl->dbi_base+reg));

	return 0;
}

static int rc_read_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

	pcie_ctrl_readl_rc((ctrl->atu_base+reg), val);

	return 0;
}

static int rc_write_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

	pcie_ctrl_writel_rc(val, (ctrl->atu_base+reg));
	printf("%s: addr 0x%lx (base 0x%lx), value 0x%lx\r\n", __func__, (ctrl->atu_base+reg), ctrl->atu_base, val);

	return 0;
}

static struct nvt_pci_ctrl nvt_rc = {
	.dbi_base =	PCIE_DBI_REG_BASE,
	.atu_base =	PCIE_DBI_REG_BASE + PF0_ATU_CAP_DBIBaseAddress,
	.phy_base =	PCIE_PHY_BASE,
	.cg_base =	IOADDR_CG_REG_BASE,
	.mode =		NVT_PCIE_RC,
	.last_atu =	-1,
	.read_dbi =	rc_read_dbi,
	.write_dbi =	rc_write_dbi,

	.read_cfg =	rc_read_dbi,	// DBI offset 0 maps to local CFG
	.write_cfg =	rc_write_dbi,	// DBI offset 0 maps to local CFG

	.read_atu =	rc_read_atu,
	.write_atu =	rc_write_atu,

	.config_oATU =	config_oATU,
	.config_iATU =	config_iATU,
};

static int ep_read_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;
printf("%s: UNDER CONSTRUCTION\r\n", __func__);
	if (ctrl->last_atu != ATU_NO_APB) {
	}

	pcie_ctrl_readl_rc((ctrl->dbi_base+reg), val);

	ctrl->last_atu = ATU_NO_APB;

	return 0;
}

static int ep_write_dbi(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;
printf("%s: UNDER CONSTRUCTION\r\n", __func__);
	if (ctrl->last_atu != ATU_NO_APB) {
	}

	pcie_ctrl_writel_rc(val, (ctrl->dbi_base+reg));

	ctrl->last_atu = ATU_NO_APB;

	return 0;
}

static int ep_read_cfg(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

	pcie_ctrl_readl_rc((ctrl->cfg_base+reg), val);

	ctrl->last_atu = ATU_NO_CFG;

	return 0;
}

static int ep_write_cfg(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

	pcie_ctrl_writel_rc(val, (ctrl->cfg_base+reg));

	ctrl->last_atu = ATU_NO_CFG;

	return 0;
}

static int ep_read_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 *val) {
	if (ctrl == NULL) return -1;
	if (val == NULL) return -1;

	pcie_ctrl_readl_rc((ctrl->atu_base+reg), val);
printf("%s: addr 0x%lx, val 0x%lx\r\n", __func__, (ctrl->atu_base+reg), val);
	ctrl->last_atu = ATU_NO_REMOTE_ATU;

	return 0;
}

static int ep_write_atu(struct nvt_pci_ctrl* ctrl, int reg, u32 val) {
	if (ctrl == NULL) return -1;

printf("%s: addr 0x%lx (base 0x%lx), value 0x%lx\r\n", __func__, (ctrl->atu_base+reg), ctrl->atu_base, val);
	pcie_ctrl_writel_rc(val, (ctrl->atu_base+reg));

	ctrl->last_atu = ATU_NO_REMOTE_ATU;

	return 0;
}

static struct nvt_pci_ctrl nvt_ep = {
	.dbi_base =	(PCIE_SLV_oATU0_BASE + 0x0D000000),
	.cfg_base =	CPU_CFG_MAP_VA,
	.atu_base =	CPU_ATU_MAP_VA,
	.phy_base =	0x6208B0000,
	.cg_base =	0x620020000,
//	.dbi_base =	CPU_PCIE_MAP_VA,
//	.cfg_base =	CPU_PCIE_MAP_VA,
//	.atu_base =	CPU_PCIE_MAP_VA,
	.mode =		NVT_PCIE_EP,
	.last_atu =	-1,
	.read_dbi =	ep_read_dbi,
	.write_dbi =	ep_write_dbi,

	.read_cfg =	ep_read_cfg,
	.write_cfg =	ep_write_cfg,

	.read_atu =	ep_read_atu,
	.write_atu =	ep_write_atu,

	.config_oATU =	config_oATU,
	.config_iATU =	config_iATU,
};

#define PRESET_COUNT	(5+1+1)		// initial preset + P0~4 + final

static void init_aeq(void)
{
	u32 reg;

	reg = readl(nvt_rc.phy_base+0x091C);	// disable AEQ R/C split
	reg &= ~(1<<16);
	writel(reg, nvt_rc.phy_base+0x091C);

	reg = readl(nvt_rc.phy_base+0x00AC);	// tune R
	reg &= ~(1<<22);
	writel(reg, nvt_rc.phy_base+0x00AC);

	reg = readl(nvt_rc.phy_base+0x00AC);	// manual fix C
	reg |= (1<<23);
	writel(reg, nvt_rc.phy_base+0x00AC);

	reg = readl(nvt_rc.phy_base+0x00A8);	// fix C = 4
	reg &= ~(0x7<<25);
	reg |= (0x4<<25);
	writel(reg, nvt_rc.phy_base+0x00A8);
}

static void restore_aeq(void)
{
	u32 reg;

	reg = readl(nvt_rc.phy_base+0x091C);	// enable AEQ R/C split
	reg |= (1<<16);
	writel(reg, nvt_rc.phy_base+0x091C);

	reg = readl(nvt_rc.phy_base+0x0214);	// AEQ frz = 1
	reg &= ~(1<<5);
	writel(reg, nvt_rc.phy_base+0x0214);

	reg = readl(nvt_rc.phy_base+0x120C);	// auto AEQ
	reg &= ~(1<<2);
	writel(reg, nvt_rc.phy_base+0x120C);
}

static void do_aeq_rc(void)
{
	u32 reg;

	// AEQ R/C frz = 0
	reg = readl(nvt_rc.phy_base+0x0214);
	reg |= 1<<5;
	reg |= 1<<6;
	writel(reg, nvt_rc.phy_base+0x0214);

	// Manual AEQ enable
	reg = readl(nvt_rc.phy_base+0x120C);
	reg |= 1<<2;
	writel(reg, nvt_rc.phy_base+0x120C);

	// Manual R on
	reg = readl(nvt_rc.phy_base+0x1000);
	reg |= 1<<30;
	writel(reg, nvt_rc.phy_base+0x1000);

	udelay(65);

	// Manual R off
	reg = readl(nvt_rc.phy_base+0x1000);
	reg &= ~(1<<30);
	writel(reg, nvt_rc.phy_base+0x1000);

	// Manual C on
	reg = readl(nvt_rc.phy_base+0x1000);
	reg |= 1<<0;
	writel(reg, nvt_rc.phy_base+0x1000);

	udelay(65);

	// Manual C off
	reg = readl(nvt_rc.phy_base+0x1000);
	reg &= ~(1<<0);
	writel(reg, nvt_rc.phy_base+0x1000);

	reg = readl(nvt_rc.phy_base+0x0214);	// AEQ frz = 1
	reg &= ~(1<<5);
	reg &= ~(1<<6);
	writel(reg, nvt_rc.phy_base+0x0214);

	reg = readl(nvt_rc.phy_base+0x120C);	// auto AEQ
	reg &= ~(1<<2);
	writel(reg, nvt_rc.phy_base+0x120C);
}

#define SW_EQ_SIMPLE	(0)
#define SW_EQ_VOTING	(1)
#define SW_EQ_MODE	(SW_EQ_VOTING)

static u32 manual_aeq(u32 preset, u32 *p_r, u32 *p_c)
{
	u32 reg;
	u32 R, Q;
	u32 FOM;
	u32 i;
#if (SW_EQ_MODE == SW_EQ_SIMPLE)
	const u32 EQ_COUNT = 1;
	const u32 TH = 3;
#else
	const u32 EQ_COUNT = 10;
	const u32 TH = 35;
#endif

	// AEQ R frz = 0
	reg = readl(nvt_rc.phy_base+0x0214);
	reg |= 1<<5;
	writel(reg, nvt_rc.phy_base+0x0214);

	// Manual AEQ enable
	reg = readl(nvt_rc.phy_base+0x120C);
	reg |= 1<<2;
	writel(reg, nvt_rc.phy_base+0x120C);

	Q = 0;

	for (i=0; i<EQ_COUNT; i++) {
		// Manual R on
		reg = readl(nvt_rc.phy_base+0x1000);
		reg |= 1<<30;
		writel(reg, nvt_rc.phy_base+0x1000);

		udelay(65);

		// Manual R off
		reg = readl(nvt_rc.phy_base+0x1000);
		reg &= ~(1<<30);
		writel(reg, nvt_rc.phy_base+0x1000);

		reg = readl(nvt_rc.phy_base+0x0A00);
		R = (reg>>8) & 0x7;
		Q += R;
	}

	*p_r = Q;

	if (Q > TH) {
		FOM = 180 - Q;
	} else {
		if (preset == 4) {
			FOM = 255;
		} else {
#if (SW_EQ_MODE == SW_EQ_SIMPLE)
			FOM = 200 + Q;
#else
			FOM = 180 + Q;
#endif
		}
	}

	return FOM;
}

static void trig_fom(void) {
	u32 reg;
	const u32 mask = 1<<17;

	reg = readl(nvt_rc.phy_base+0x102C);
	reg &= ~mask;
	writel(reg, nvt_rc.phy_base+0x102C);

	reg = readl(nvt_rc.phy_base+0x102C);
	reg |= mask;
	writel(reg, nvt_rc.phy_base+0x102C);

	reg = readl(nvt_rc.phy_base+0x102C);
	reg &= ~mask;
	writel(reg, nvt_rc.phy_base+0x102C);
}

#define DETECT_QUIET    0x00
#define RCVRY_LOCK      0x0D
#define RCVRY_SPEED     0x0E
#define RCVRY_RCVRCFG   0x0F
#define RECVRY_EQ0      0x20
#define RECVRY_EQ1      0x21
#define RECVRY_EQ2	0x22
#define RECVRY_EQ3	0x23

static int link_speed_change(UINT32 speed) {
	u32 reg;
	int i;
	int is_patched = 0;
	int is_preset_handled = 0;
	UINT32  last_ltssm = DETECT_QUIET;
        UINT32  state_start_us = 0;     // unit: us
	const int timeout = 1000000;
	const UINT32 CAP_EXP_CAP_BASE = 0x70;

	if (auto_preset) {
		printf("%s: select auto preset\r\n", __func__);
		init_aeq();
	}

	// Write to LINK_CONTROL2_LINK_STATUS2_REG . PCIE_CAP_TARGET_LINK_SPEED
	nvt_rc.read_dbi(&nvt_rc, CAP_EXP_CAP_BASE + 0x30, &reg);
	reg &= ~0xF;
	reg |= speed;
	nvt_rc.write_dbi(&nvt_rc, CAP_EXP_CAP_BASE + 0x30, reg);

	// Deassert GEN2_CTRL_OFF . DIRECT_SPEED_CHANGE in the local device
	nvt_rc.read_dbi(&nvt_rc, 0x80C, &reg);
	reg &= ~(1<<17);
	nvt_rc.write_dbi(&nvt_rc, 0x80C, reg);
	reg |= (1<<17);
	nvt_rc.write_dbi(&nvt_rc, 0x80C, reg);

	for (i=0; i<timeout; i++) {
		// wait link training is started
		nvt_rc.read_dbi(&nvt_rc, CAP_EXP_CAP_BASE + 0x10, &reg);
		if (reg & (1<<27)) break;
	}
	if (i >= timeout) {
		printf("%s: wait link training start timeout\r\n", __func__);
	}

	// read LINK_CONTROL_LINK_STATUS_REG . PCIE_CAP_LINK_SPEED
	for (i=0; i<timeout; i++) {
		UINT32 curr_ltssm;

		reg = rc_apbm_sr(PCIE_TOP_REG_BASE+0x08);
//		reg = readl(0x2F04F0008);
                curr_ltssm = (reg>>4) & 0x3F;
                if (last_ltssm != curr_ltssm) {
                        if (last_ltssm == RCVRY_SPEED) {
                                printf("RCVRY_SPEED elps %d us\r\n", get_nvt_timer0_cnt()-state_start_us);
                        }
                        state_start_us = get_nvt_timer0_cnt();
                        last_ltssm = curr_ltssm;
			is_preset_handled = 0;
                }

		if (curr_ltssm == RECVRY_EQ1) {
                        // RCVRY_EQ1
                        // eq phase 1
#if 1
                        if ((is_patched == 0) &&
					((get_nvt_timer0_cnt()-state_start_us) > 4000)) {
				reg = readl(nvt_rc.phy_base+0x085C);
				reg &= ~(1<<30);
				writel(reg, nvt_rc.phy_base+0x085C);

				reg = readl(nvt_rc.phy_base+0x3014);
                                reg &= ~(1<<3);
                                writel(reg, nvt_rc.phy_base+0x3014);
                                reg |= (1<<3);
                                writel(reg, nvt_rc.phy_base+0x3014);
                                reg &= ~(1<<3);
                                writel(reg, nvt_rc.phy_base+0x3014);

				is_patched = 1;
                                printf("try reset...\r\n");
                        }
#endif
		} else if (curr_ltssm == RECVRY_EQ3) {
			if (auto_preset && (is_preset_handled == 0)) {
				u32 local_fom = 0;
				u32 max_fom_value = 0;
				u32 max_fom_p_idx = 0;
				int i;
				u32 v_fom_history[PRESET_COUNT];
				u32 v_history_R[PRESET_COUNT];
				u32 v_history_C[PRESET_COUNT];

				is_preset_handled = 1;
				for (i=0; i<PRESET_COUNT; i++) {
					v_fom_history[i] = 0;
					v_history_R[i] = 0;
					v_history_C[i] = 0;
				}

				// #1 TX preset
				local_fom = manual_aeq(4, &v_history_R[0], &v_history_C[0]);	// initial preset (P4)
				v_fom_history[0] = local_fom;
				reg = readl(nvt_rc.phy_base+0x1338);
				reg &= ~(0xFF<<0);
				reg |= local_fom<<0;
				writel(reg, nvt_rc.phy_base+0x1338);
				udelay(2);
				trig_fom();
				max_fom_value = local_fom;
				max_fom_p_idx = 4;

				// #2 TX preset
				local_fom = manual_aeq(0, &v_history_R[1], &v_history_C[1]);	// P0
				v_fom_history[1] = local_fom;
				reg = readl(nvt_rc.phy_base+0x1338);
				reg &= ~(0xFF<<8);
				reg |= local_fom<<8;
				writel(reg, nvt_rc.phy_base+0x1338);
				udelay(2);
				trig_fom();
				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 0;
				}

				// #3 TX preset
				local_fom = manual_aeq(1, &v_history_R[2], &v_history_C[2]);	// P1
				v_fom_history[2] = local_fom;
				reg = readl(nvt_rc.phy_base+0x1338);
				reg &= ~(0xFF<<16);
				reg |= local_fom<<16;
				writel(reg, nvt_rc.phy_base+0x1338);
				udelay(2);
				trig_fom();
				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 1;
				}

				// #4 TX preset
				local_fom = manual_aeq(2, &v_history_R[3], &v_history_C[3]);	// P2
				v_fom_history[3] = local_fom;
				reg = readl(nvt_rc.phy_base+0x1338);
				reg &= ~(0xFF<<24);
				reg |= local_fom<<24;
				writel(reg, nvt_rc.phy_base+0x1338);
				udelay(2);
				trig_fom();
				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 2;
				}

				// #5 TX preset
				local_fom = manual_aeq(3, &v_history_R[4], &v_history_C[4]);	// P3
				v_fom_history[4] = local_fom;
				reg = readl(nvt_rc.phy_base+0x133C);
				reg &= ~(0xFF<<0);
				reg |= local_fom<<0;
				writel(reg, nvt_rc.phy_base+0x133C);
				udelay(2);
				trig_fom();
				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 3;
				}

				// #6 TX preset
				local_fom = manual_aeq(4, &v_history_R[5], &v_history_C[5]);	// P4
				v_fom_history[5] = local_fom;
				reg = readl(nvt_rc.phy_base+0x133C);
				reg &= ~(0xFF<<8);
				reg |= local_fom<<8;
				writel(reg, nvt_rc.phy_base+0x133C);
				udelay(2);
				trig_fom();
				if (local_fom > max_fom_value) {
					max_fom_value = local_fom;
					max_fom_p_idx = 4;
				}

				// #7 TX preset
				// If last preset is not best FOM, the best FOM will be picked up to do final test
				local_fom = manual_aeq(max_fom_p_idx, &v_history_R[6], &v_history_C[6]);
				v_fom_history[6] = local_fom;
				reg = readl(nvt_rc.phy_base+0x133C);
				reg &= ~(0xFF<<16);
				reg |= local_fom<<16;
				writel(reg, nvt_rc.phy_base+0x133C);
				udelay(2);
				trig_fom();
				restore_aeq();
				do_aeq_rc();

				printf("pick preset %d\r\nFOM: ", max_fom_p_idx);
				for (i=0; i<PRESET_COUNT; i++) {
					printf("%d ", v_fom_history[i]);
				}
				printf("\r\nR: ");
				for (i=0; i<PRESET_COUNT; i++) {
					printf("%d ", v_history_R[i]);
				}
//				printf("\r\nC: ");
				for (i=0; i<PRESET_COUNT; i++) {
//					printf("%d ", v_history_C[i]);
				}
				printf("\r\n");

				reg = readl(nvt_rc.phy_base+0x0A00);
				printf("Final R 0x%x\r\n", (reg>>8) & 0x7);
				printf("Final C 0x%x\r\n", (reg>>12) & 0x7);
			}
		}

		nvt_rc.read_dbi(&nvt_rc, CAP_EXP_CAP_BASE + 0x10, &reg);
		// check PCIE_CAP_LINK_TRAINING
		if (! (reg & (1<<27))) break;

//		udelay(1000);
	}

	nvt_rc.read_dbi(&nvt_rc, CAP_EXP_CAP_BASE + 0x10, &reg);
	printf("%s: PCIE_CAP_LINK_SPEED 0x%x\r\n", __func__, reg);

	if (i >= timeout) {
		printf("%s: switch to gen %d fail\r\n", __func__, speed);
	}

	if (((reg>>16)&0xF) != speed) return -1;

	return 0;
}

static void pcie_ini(u32 target_speed, u32 eq_enable) {
	u32 reg;
	int i;
	const int timeout = 60;

	rc_dbi_srmw(LINK_CONTROL2_LINK_STATUS2_REG, 1, LINK_CONTROL2_LINK_STATUS2_REG_PCIE_CAP_TARGET_LINK_SPEED_BitAddressOffset, 4); //adr, dat, start, width //set target gen1

	printf("%s: LINK STS2 reg 0x%x\r\n", __func__, rc_dbi_srp(LINK_CONTROL2_LINK_STATUS2_REG));
	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x304, 0x01, 2, 1);	//[2]link_train_en

	for (i=0; i<timeout; i++) {
		reg = rc_apbm_sr(PCIE_TOP_REG_BASE+0x08);
		if (reg&0x01) break;
		udelay(1000);
	}
	reg = rc_apbm_sr(PCIE_TOP_REG_BASE+0x08);
	printf("%s: phy link up/down %d, data link up/down %d, LTSSM 0x%x\r\n",
		__func__, (reg>>11)&0x01, (reg>>0)&0x01, (reg>>4)&0x3F);

	if (i >= timeout) {
		printf("%s: PCIE LINK training fail\r\n", __func__);
	}
}

void static setup_max_payload(void) {
	u32 reg;

	// change max payload size
        nvt_rc.read_cfg(&nvt_rc, 0x78, &reg);
        reg &= ~(0x7<<5);
        reg |= 0x1<<5;      // size: 256B
	printf("%s: RC write 0x%x\r\n", __func__, reg);
        nvt_rc.write_cfg(&nvt_rc, 0x78, reg);

        nvt_ep.read_cfg(&nvt_ep, 0x78, &reg);
        reg &= ~(0x7<<5);
        reg |= 0x1<<5;      // size: 256B
	printf("%s: EP write 0x%x\r\n", __func__, reg);
        nvt_ep.write_cfg(&nvt_ep, 0x78, reg);
        nvt_ep.read_cfg(&nvt_ep, 0x78, &reg);
	printf("%s: EP read 0x%x\r\n", __func__, reg);
}

static void pcie_atu_rc_ini(void) {
	printf("RC iATU outbound read/write (A64), should set before EP BAR setting ...\r\n");
	nvt_rc.config_oATU(&nvt_rc, nvt_ep.inbound[0].phys_start,
				nvt_ep.inbound[0].phys_start + nvt_ep.inbound[0].size - 1,
				nvt_ep.inbound[0].bus_start, 0, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, nvt_ep.inbound[1].phys_start,
				nvt_ep.inbound[1].phys_start + nvt_ep.inbound[1].size - 1,
				nvt_ep.inbound[1].bus_start, 1, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, nvt_ep.inbound[2].phys_start,
				nvt_ep.inbound[2].phys_start + nvt_ep.inbound[2].size - 1,
				nvt_ep.inbound[2].bus_start, 2, PCIE_ATU_TYPE_MEM, true);
	nvt_rc.config_oATU(&nvt_rc, 0x600000000ULL, 0x60FFFFFFFULL, 0x600000000ULL, 3, PCIE_ATU_TYPE_CFG0, true);	// CFG is fixed
	env_set_hex("nvt_pcie_ep0_mau_base", nvt_ep.inbound[0].phys_start);
}

static void pcie_atu_ep_ini(void) {
	// EP iATU
	nvt_ep.config_iATU(&nvt_ep, nvt_ep.inbound[0].phys_start,
			nvt_ep.inbound[0].phys_start+nvt_ep.inbound[0].size-1,
			0x000000000ULL, 0, true);
	nvt_ep.config_iATU(&nvt_ep, nvt_ep.inbound[2].phys_start,
			nvt_ep.inbound[2].phys_start+nvt_ep.inbound[2].size-1,
			PCIE_EP0_REAL_REG_BASE, 1, true);

	// EP oATU
	nvt_ep.config_oATU(&nvt_ep, nvt_rc.inbound[0].bus_start,
			nvt_rc.inbound[0].bus_start+nvt_rc.inbound[0].size-1,
			nvt_rc.inbound[0].bus_start, 0, PCIE_ATU_TYPE_MEM, true);
	nvt_ep.config_oATU(&nvt_ep, nvt_rc.inbound[1].bus_start,
			nvt_rc.inbound[1].bus_start+nvt_rc.inbound[1].size-1,
			nvt_rc.inbound[1].bus_start, 1, PCIE_ATU_TYPE_MEM, true);
}

/* bar_sz: 0:1M, 1:2M, 2:4M, 3:8M ... */
static void ep_set_resizable_barsz(int bar_idx, u64 bar_sz)
{
	u32 val, tmp, order = 0, resize_idx;

	tmp = (bar_sz >> 20) - 1;
	while (tmp) {
		order ++;
		tmp >>= 1;
	}

	switch (bar_idx) {
	case 0:
		resize_idx = 0;
		break;
	default:
		panic("error bar_idx: %d \n", bar_idx);
		break;
	}

	nvt_ep.read_cfg(&nvt_ep, 0x2B4 + (resize_idx*8) + 0x8, &val);
	val &= ~(0x3F << 8);
	val |= ((order & 0x3F) << 8);
	nvt_ep.write_cfg(&nvt_ep, 0x2B4 + (resize_idx*8) + 0x8, val);
}

static void setup_ep_bar(int bar_idx, u64 bar_base, u64 bar_sz) {
	u32 reg;

	printf("%s: bar %d, base 0x%llx, size 0x%llx\r\n", __func__, bar_idx, bar_base, bar_sz);
	if (bar_idx == 0) {
		// BAR1
		ep_set_resizable_barsz(0, bar_sz);
	}

	nvt_ep.write_cfg(&nvt_ep, 0x10 + 0x4*bar_idx, bar_base&0xFFFFFFFF);
	nvt_ep.write_cfg(&nvt_ep, 0x10 + 0x4*bar_idx + 0x4, bar_base>>32);
}

static void pcie_ep_bar_ini(void) {
	setup_ep_bar(0, nvt_ep.inbound[0].bus_start, nvt_ep.inbound[0].size);	// BAR0: MAU

	setup_ep_bar(2, nvt_ep.inbound[2].bus_start, nvt_ep.inbound[2].size);	// BAR2: APB

	setup_ep_bar(4, nvt_ep.inbound[1].bus_start, nvt_ep.inbound[1].size);	// BAR4: ATU
}

static void pcie_atu_rc_inbound_loopback_ini(void) {
	printf("RC iATU inbound ...\r\n");

	printf("%s: phy_start size %d, size size %d\r\n", __func__,
		sizeof(nvt_rc.inbound[0].bus_start),
		sizeof(nvt_rc.inbound[0].size));
	printf("%s: inbound 0: 0x%llx to 0x%llx\r\n", __func__,
		nvt_rc.inbound[0].bus_start+nvt_rc.inbound[0].size-1,
		nvt_rc.inbound[0].phys_start);
	nvt_rc.config_iATU(&nvt_rc, nvt_rc.inbound[0].bus_start,
		nvt_rc.inbound[0].bus_start+nvt_rc.inbound[0].size-1,
		nvt_rc.inbound[0].phys_start, 0, true);
	printf("%s: inbound 1: 0x%llx to 0x%llx\r\n", __func__,
		nvt_rc.inbound[1].bus_start+nvt_rc.inbound[1].size-1,
		nvt_rc.inbound[1].phys_start);
	nvt_rc.config_iATU(&nvt_rc, nvt_rc.inbound[1].bus_start,
			nvt_rc.inbound[1].bus_start+nvt_rc.inbound[1].size-1,
			nvt_rc.inbound[1].phys_start, 1, true);
}


static u32 pcie_ini0(u32 opt) {
	u32 reg;

	if (opt & (1<<3)) {	// REFCLK out
		if (readl(IOADDR_TOP_REG_BASE + 0x00) & (1<<10)) {
			printf("BS[10] is 1, conflict with refclk-out, plz check with HW\r\n");
		} else {
			reg = readl(IOADDR_TOP_REG_BASE + 0x28);	// 0x2F0010028[9]
			reg |= 1<<9;
			writel(reg, IOADDR_TOP_REG_BASE + 0x28);

			reg = readl(PCIE_TOP_REG_BASE + 0x318);		// 0x2F04F0318[22]
			reg |= 1<<22;
			writel(reg, PCIE_TOP_REG_BASE + 0x318);
		}
	}

	// assert reset
	//[3:0]phy_apb_rst_n, pipe_rst_n, power_up_rst_n, perst_n
	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x300, 0x10, 0, 4);	// reset all
	mdelay(1);

	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x304, 0x00, 2, 1);	//[2]link_train_en: disable

	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x300, 0x18, 0, 4);	// release phy_apb_rst_n
	mdelay(1);

	// (PHY) bypass PTEN RESET
        reg = readl(nvt_rc.phy_base+0x1070);
        reg |= 1<<8;
        writel(reg, nvt_rc.phy_base+0x1070);

	// suppress pll noise
	writel(0x7F, nvt_rc.cg_base+0x4104);
	printf("%s: cg base 0x%x\r\n", __func__, nvt_rc.cg_base);
	writel(0x54, nvt_rc.phy_base+0x40DC);
	printf("%s: phy base 0x%x\r\n", __func__, nvt_rc.phy_base);

	// release reset
	rc_apbm_srmw(PCIE_TOP_REG_BASE+0x300, 0x1F, 0, 4);	//[2:0]pipe_rst_n, power_up_rst_n, perst_n
	mdelay(1);

	return 3;
}

static void rc_phy_setting(struct nvt_pci_ctrl *p_ctrl) {
	u32 reg;

	printf("%s: base 0x%llx\r\n", __func__, p_ctrl->phy_base);
	// DFE vref
	reg = readl(p_ctrl->phy_base+0xA0);
	reg |= 0x3F << 24;
	reg |= 1 << 30;
	printf("%s: ofs 0xA0 = 0x%x\r\n", __func__, reg);
	writel(reg, p_ctrl->phy_base+0xA0);
	reg = readl(p_ctrl->phy_base+0xA0);
	printf("%s: ofs 0xA0 read 0x%x\r\n", __func__, reg);

	// txodt
	reg = readl(p_ctrl->phy_base+0x0C);
	reg &= ~(0x3<<22);
	reg |= 0x2<<22;
	reg |= 1<<20;
	reg |= 1<<21;
	writel(reg, p_ctrl->phy_base+0x0C);

	// rxodt
	reg = readl(p_ctrl->phy_base+0x00);
	reg |= 0xFF<<16;
	writel(reg, p_ctrl->phy_base+0x00);

	// rxqec
	reg = readl(p_ctrl->phy_base+0x108);
	reg |= 1<<9;
	reg &= ~(0x7<<17);
	reg |= 1<<17;
	writel(reg, p_ctrl->phy_base+0x108);

	// bypass PTEN RESET
	reg = readl(p_ctrl->phy_base+0x1070);
	reg |= 1<<8;
	writel(reg, p_ctrl->phy_base+0x1070);

	// PIPE mode change
	reg = readl(p_ctrl->phy_base+0x3010);
	reg |= 1<<14;
	writel(reg, p_ctrl->phy_base+0x3010);

	// pclk deglitch
	reg = readl(p_ctrl->phy_base+0x1068);
	reg |= 1<<21;
	writel(reg, p_ctrl->phy_base+0x1068);
	reg = readl(p_ctrl->phy_base+0x3014);
	reg |= 1<<7;
	writel(reg, p_ctrl->phy_base+0x3014);

	// CMM
	reg = readl(p_ctrl->phy_base+0x4060);
	reg &= ~(1<<3);
	writel(reg, p_ctrl->phy_base+0x4060);

	// LOS comparator
	reg = readl(p_ctrl->phy_base+0x834);
	reg &= ~(0x1F<<12);
	reg |= 0x04<<12;
	writel(reg, p_ctrl->phy_base+0x834);

	// disable EIOS detect
	reg = readl(p_ctrl->phy_base+0x3010);
	if ((nvt_get_chip_ver() != CHIPVER_A) &&
			(en_l1_detect == 1)) {
		reg &= ~(1<<3);
		reg |= 1<<21;
	} else {
		reg |= 1<<3;
	}
	writel(reg, p_ctrl->phy_base+0x3010);

	// div2_demux_eco
	if ((nvt_get_chip_ver() != CHIPVER_A) &&
                        (en_l1_detect == 1)) {
		reg = readl(p_ctrl->phy_base+0x0918);
		reg |= 1<<22;
		writel(reg, p_ctrl->phy_base+0x0918);
	}

	// T2R idle
	reg = readl(p_ctrl->phy_base+0x218);
	reg |= 1<<5;
	writel(reg, p_ctrl->phy_base+0x218);

	reg = readl(p_ctrl->phy_base+0x1068);
	reg &= ~(1<<20);
	writel(reg, p_ctrl->phy_base+0x1068);

	// fix preset
	// If RC, cg_base is 0x2F0020000
	// If EP, cg_base is 0x620020000
	reg = readl(p_ctrl->cg_base+0x0000);	// enable PLLs on EP
	reg |= 0x20A5;
	writel(reg, p_ctrl->cg_base+0x0000);

	// If RC, dbi_base is 0x2FD000000
	// If EP, dbi_base is 0x62D000000
	reg = readl(p_ctrl->dbi_base+0x08A8);	// force EQ preset 4
	if (auto_preset == 0) {			// fix preset
		printf("Use fix preset\r\n");
		reg &= ~(1<<24);
		reg &= ~(0xFFFF<<8);
		reg |= ((1<<4))<<8;
	} else {
		printf("Use auto preset\r\n");
		reg &= ~(0xFFFF<<8);
		reg |= ((0x1F<<0))<<8;	// fix P0~4
	}
	writel(reg, p_ctrl->dbi_base+0x08A8);

	// AEQ paremeter
	reg = readl(p_ctrl->phy_base+0x00B8);		// ctle accu th
	reg &= ~(0x7FF<<0);
	reg |= 0x400<<0;
	writel(reg, p_ctrl->phy_base+0x00B8);

	reg = readl(p_ctrl->phy_base+0x1080);		// aeq time period
	reg |= 0x3FFF;
	writel(reg, p_ctrl->phy_base+0x1080);

	// AGC/VTH parameter
	reg = readl(p_ctrl->phy_base+0x00A0);		// agc acc th
	reg &= ~(0x1FF<<10);
	reg |= 0x100<<10;
	writel(reg, p_ctrl->phy_base+0x00A0);

	reg = readl(p_ctrl->phy_base+0x1080);		// agc time period/dfe time period
	reg |= 0x3FFF<<16;
	writel(reg, p_ctrl->phy_base+0x1080);

	// DFE parameter
	reg = readl(p_ctrl->phy_base+0x00A4);		// H1 threshold
	reg &= ~(0x3FF<<12);
	reg |= (0x100<<12);
	writel(reg, p_ctrl->phy_base+0x00A4);

	reg = readl(p_ctrl->phy_base+0x00A8);		// H2 threshold
	reg &= ~(0x3FF<<12);
	reg |= (0x100<<12);
	writel(reg, p_ctrl->phy_base+0x00A8);
	
	reg = readl(p_ctrl->phy_base+0x1080);		// agc time period/dfe time period
	reg |= 0x3FFF<<16;
	writel(reg, p_ctrl->phy_base+0x1080);

	// CTS 4.2.6.7.2. L1.Idle
	reg = readl(p_ctrl->dbi_base+0x0890);		// D3-L1
	reg &= ~(1<<0);				// GEN3_ZRXDC_NONCOMPL = 0
	writel(reg, p_ctrl->dbi_base+0x0890);

	// suppress pll noise
	writel(0x7F, p_ctrl->cg_base+0x4104);
	writel(0x54, p_ctrl->phy_base+0x40DC);

#if 1	// pcie link eq prototype
	reg = readl(p_ctrl->phy_base+0x1210);	// bypass phase calibration
	reg |= 1<<4;
	writel(reg, p_ctrl->phy_base+0x1210);

	reg = readl(p_ctrl->phy_base+0x1048);	// disable phase calibration fom
	reg &= ~(1<<19);
	writel(reg, p_ctrl->phy_base+0x1048);

	reg = readl(p_ctrl->phy_base+0x1028);	// enable FOM sw ctrl
	reg |= (1<<11);
	writel(reg, p_ctrl->phy_base+0x1028);

	reg = readl(p_ctrl->phy_base+0x102C);	// enable FOM para sw
	reg |= (1<<16);
	writel(reg, p_ctrl->phy_base+0x102C);

	reg = readl(p_ctrl->phy_base+0x1304);	// total 5 preset
	reg &= ~(0xF<<22);
	reg |= 0x7<<22;		// for P0~4 experiment + initial + final
	reg |= 1<<21;				// FOM feedback manual mode
	writel(reg, p_ctrl->phy_base+0x1304);
#endif
}

static void pcie_init(void) {
	u32 target_speed;
	u32 u32_val;
	u32 ep_ver;
	int retry = 10;
	int refclk_out = 0;

	tx_boost = 0;
	auto_preset = 0;

#if defined(CONFIG_NVT_PCIE_TEST_MODE)
printf("%s: CONFIG_NVT_PCIE_TEST_MODE: %d\r\n", __func__, CONFIG_NVT_PCIE_TEST_MODE);
#else
printf("%s: no CONFIG_NVT_PCIE_TEST_MODE\r\n", __func__);
#endif

	if (nvt_pcie_dts_load_inbound(nvt_rc.inbound, "/pcie@2,f04f0000") != 0) {
		printf("%s: load RC dts node fail\r\n", __func__);
		return;
	}
	if (nvt_pcie_dts_load_ep_inbound(nvt_ep.inbound, "/nvt_ep0") != 0) {
		printf("%s: load EP dts node fail\r\n", __func__);
		return;
	}
	refclk_out = nvt_pcie_dts_load_refclk_out("/pcie@2,f04f0000");
	tx_boost = nvt_pcie_dts_load_tx_boost("/pcie@2,f04f0000");
	auto_preset = nvt_pcie_dts_load_auto_preset("/pcie@2,f04f0000");
	en_l1_detect = nvt_pcie_dts_load_l1_det("/pcie@2,f04f0000");

	while (retry--) {
		// -------------------------- //
		//  initialization
		// -------------------------- //
		target_speed = pcie_ini0((refclk_out<<3));	//opt[3]: REFCLK out

		// -------------------------- //
		//  DBI
		// -------------------------- //
		// release PRST
		u32_val = ioread32(IOADDR_TOP_REG_BASE + 0x28);
		u32_val &= ~(1<<4);	// assert preset
		iowrite32(u32_val, IOADDR_TOP_REG_BASE + 0x28);
		udelay(200);
		u32_val |= 1<<4;	// release preset
		iowrite32(u32_val, IOADDR_TOP_REG_BASE + 0x28);
		mdelay(200);

		u32_val = rc_dbi_srp(PORT_LINK_CTRL_OFF); //
		printf("%s: PORT_LINK_CTRL_OFF = 0x%x\r\n", __func__, u32_val);

		rc_phy_setting(&nvt_rc);	// apply phy setting before train en

		pcie_ini(target_speed,0);   //target_speed, eq_enable:0:eq disable

		pcie_atu_rc_ini(); //rc atu should set first to let EP bar can access
		pcie_ep_bar_ini();

		pcie_atu_rc_inbound_loopback_ini();

		setup_max_payload();

		rc_dbi_srmw(STATUS_COMMAND_REG, 1, STATUS_COMMAND_REG_PCI_TYPE0_IO_EN_BitAddressOffset, STATUS_COMMAND_REG_PCI_TYPE0_IO_EN_RegisterSize);
		rc_dbi_srmw(STATUS_COMMAND_REG, 1, STATUS_COMMAND_REG_PCI_TYPE0_MEM_SPACE_EN_BitAddressOffset, STATUS_COMMAND_REG_PCI_TYPE0_MEM_SPACE_EN_RegisterSize);
		rc_dbi_srmw(STATUS_COMMAND_REG, 1, STATUS_COMMAND_REG_PCI_TYPE0_BUS_MASTER_EN_BitAddressOffset, STATUS_COMMAND_REG_PCI_TYPE0_BUS_MASTER_EN_RegisterSize);

		// Enable EP
		nvt_ep.write_cfg(&nvt_ep, PCI_COMMAND, PCI_COMMAND_MASTER|PCI_COMMAND_MEMORY);

		// Config EP ATU after EP is enabled
		pcie_atu_ep_ini();

		// If EP CPU default is disabled, pass patched loader to EP,
		// else EP CPU is running with loader on it's flash.
		u32_val = readl(nvt_ep.cg_base+0x0090);
		if (~(u32_val & (1<<0)))
		{
			int i;
			u32 reg;
			u32 *p_ep_sram = (void*)(0x620000000+0x0E000000);

			reg = readl(nvt_ep.cg_base+0x0070);	// enable CNN CLKEN
			reg |= 1<<16;
			writel(reg, nvt_ep.cg_base+0x0070);

			if (run_command("nvt_pcie preloader", 0) != 0) {
				printf("%s: copy pre-loader failed\r\n", __func__);
				return;
			}

			if (en_l1_detect == 1) {
				writel(1, CC_RC_EP_BOOT_COMM_REG + 4);
			}

			reg = readl(nvt_ep.cg_base+0x0090);	// release CPU on EP
			reg |= 1<<0;
			writel(reg, nvt_ep.cg_base+0x0090);
			printf("Transfer patched loader to EP\r\n");

			mdelay(1000);	// wait EP loader
		}

		// apply tx boost setting
		{
			u32 reg;

			reg = readl(nvt_rc.phy_base+0x0820);
			reg &= ~(0x3<<9);
			reg |= tx_boost<<9;
			writel(reg, nvt_rc.phy_base+0x0820);

			reg = readl(nvt_ep.phy_base+0x0820);
			reg &= ~(0x3<<9);
			reg |= tx_boost<<9;
			writel(reg, nvt_ep.phy_base+0x0820);
		}

		if (link_speed_change(3) == 0) {	// switch to higher speed after phy is setup
				break;
		}

		printf("%s: retry link\r\n", __func__);
	}

#if (CONFIG_SYS_PCI_64BIT == 1)
	printf("%s: CONFIG_SYS_PCI_64BIT\r\n", __func__);
#else
	printf("%s: NO CONFIG_SYS_PCI_64BIT\r\n", __func__);
#endif
}



/* this function is portable for the customer */
int do_pcie (cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	printk("-------------- PCIe Version: %s -------------- \n", DRV_VER);
	pcie_init();

	return 0;
}


U_BOOT_CMD(
	nvt_pcie_init,	2,	1,	do_pcie,
	"create pcie-mapping",
	"command: pcie [1/2/3]"
);

#endif
