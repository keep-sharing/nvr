// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#define REG_BASE_EMMC_PHY           0x10010000
#define EMMC_PHY_INITCTRL           (REG_BASE_EMMC_PHY + 0x4)
#define  EMMC_INIT_EN               0x1
#define  EMMC_DLYMEAS_EN            (0x1 << 2)
#define  EMMC_ZCAL_EN               (0x1 << 3)
#define INITCTRL_CHECK_TIMES        100

/* CRG register */
#define PERI_CRG_MMC_CLK	(CRG_REG_BASE + 0x34c0)
#define PERI_CRG_SDIO0_CLK	(CRG_REG_BASE + 0x35c0)
#define  CRG_CLK_SEL_SHIFT	 24
#define  CRG_CLK_SEL_MASK	(0x7 << CRG_CLK_SEL_SHIFT)
#define  CRG_TX_SRST_REQ   (1 << 18)
#define  CRG_RX_SRST_REQ   (1 << 17)
#define  CRG_SRST_REQ	(1 << 16)
#define  CRG_AHB_CLK_EN		(1 << 1)
#define  CRG_CLK_EN         (1 << 0)

#define PERI_CRG_MMC_P4_DLL	(CRG_REG_BASE + 0x34c4)
#define  CRG_P4_DLL_SRST_REQ	(1 << 1)

#define PERI_CRG_MMC_DRV_DLL	(CRG_REG_BASE + 0x34c8)
#define PERI_CRG_SDIO0_DRV_DLL	(CRG_REG_BASE + 0x35c8)
#define  CRG_DRV_PHASE_SEL_SHIFT	15
#define  CRG_DRV_PHASE_SEL_MASK	(0x1F << CRG_DRV_PHASE_SEL_SHIFT)

#define PERI_CRG_MMC_STAT	(CRG_REG_BASE + 0x34d8)
#define PERI_CRG_SDIO0_STAT	(CRG_REG_BASE + 0x35d8)
#define  CRG_SAM_DLL_READY	(1 << 12)
#define  CRG_DS_DLL_READY	(1 << 10)
#define  CRG_P4_DLL_LOCKED	(1 << 9)

#define REG_EMMC_SAMPL_DLL_STATUS	PERI_CRG_MMC_STAT
#define REG_SDIO0_SAMPL_DLL_STATUS	PERI_CRG_SDIO0_STAT
#define  SDIO_SAMPL_DLL_SLAVE_READY	CRG_SAM_DLL_READY

#define REG_EMMC_DRV_DLL_CTRL		PERI_CRG_MMC_DRV_DLL
#define REG_SDIO0_DRV_DLL_CTRL		PERI_CRG_SDIO0_DRV_DLL
#define  SDIO_DRV_PHASE_SEL_MASK	CRG_DRV_PHASE_SEL_MASK
#define sdio_drv_sel(phase)		((phase) << CRG_DRV_PHASE_SEL_SHIFT)

#define REG_EMMC_SAMPLB_DLL_CTRL	0x34d4
#define REG_SDIO0_SAMPLB_DLL_CTRL	0x35d4
#define SDIO_SAMPLB_DLL_CLK_MASK	0xf
#define sdio_samplb_sel(phase)		(((phase) & 0xf) << 0)

#define REG_VDP_AIAO_MCU_AHB1_MISC 0x102E0000
#define REG_PWR_SWITCH_MISC     (REG_VDP_AIAO_MCU_AHB1_MISC + 0x10)
#define SDIO0_PWRSW_SEL_1V8		BIT(5)
#define SDIO0_PWR_EN			BIT(4)
#define SDIO0_IO_MODE_SEL_1V8	BIT(1)
#define SDIO0_PWR_CTRL_BY_MISC  BIT(0)

/* eMMC io reg */
#define REG_MMC_IO_CFG_BASE	 0x10FF0000
#define REG_MMC_CLK_IO		 0x00
#define REG_MMC_CMD_IO		 0x04
#define REG_MMC_D0_IO		 0x0c
#define REG_MMC_D1_IO		 0x10
#define REG_MMC_D2_IO		 0x14
#define REG_MMC_D3_IO		 0x18
#define REG_MMC_D4_IO		 0x2c
#define REG_MMC_D5_IO		 0x28
#define REG_MMC_D6_IO		 0x30
#define REG_MMC_D7_IO		 0x1c
#define REG_MMC_DQS_IO		 0x20
#define REG_MMC_RSTN_IO		 0x08

/* SDIO0 IO */
#define REG_SD_IO_CFG_BASE  0x017ca0000
#define REG_SDIO0_PWEN_N    0x000
#define REG_SDIO0_CMD_IO    0x110
#define REG_SDIO0_D0_IO     0x100
#define REG_SDIO0_D1_IO     0x104
#define REG_SDIO0_D2_IO     0x108
#define REG_SDIO0_D3_IO     0x10c
#define REG_SDIO0_CLK_IO    0x0fc

 /* IO CFG */
#define  IO_CFG_DRV_STR_MASK		(0xf << 4)
#define  io_cfg_drv_str_sel(str)	((str) << 4)
#define  IO_CFG_PULL_UPDOWN_EN 		(0x1 << 8)
#define  IO_CFG_PULL_UP  		(0x1 << 9)
#define  IO_CFG_PULL_SDIO_DOWN		(0x1 << 9)
#define  IO_CFG_PULL_SDIO_UP		(0x1 << 8)
#define  IO_CFG_SDIO_MASK		(IO_CFG_DRV_STR_MASK | IO_CFG_PULL_UP | \
					IO_CFG_PULL_UPDOWN_EN)

#define SD_IO_MUX			0x1
#define MMC_IO_MUX			0x1
#define IO_MUX_MASK			0xF
#define MMC_BUS_WIDTH_4_BIT		4
#define MMC_BUS_WIDTH_8_BIT		8
#define SD_IO_NUM			4

#define BOOT_FROM_EMMC_FLAG		(0x3  << 2)
#define BOOT_FLAG_MASK			(0x3  << 2)
#define EMMC_BOOT_8BIT			(0x1  << 11)

#define IO_CLK				0
#define IO_CMD				1
#define IO_DATA				2
#define IO_RST				3
#define IO_DS				4
#define EMMC_IO_TYPE_NUM	5
#define SDIO_IO_TYPE_NUM    3

static unsigned int reg_data_io[] = {
	REG_MMC_D0_IO, REG_MMC_D1_IO,
	REG_MMC_D2_IO, REG_MMC_D3_IO,
	REG_MMC_D4_IO, REG_MMC_D5_IO,
	REG_MMC_D6_IO, REG_MMC_D7_IO
};

static unsigned int reg_sdio0_data_io[] = {
	REG_SDIO0_D0_IO, REG_SDIO0_D1_IO,
	REG_SDIO0_D2_IO, REG_SDIO0_D3_IO,
};

static u32 mmc_io_cfg[][EMMC_IO_TYPE_NUM] = { /* CLK CMD DATA RST DQS */
	[MMC_LEGACY] = {
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN, /* 0x4 is 0b100 */
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN | /* 0x4 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN | /* 0x4 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN |
			IO_CFG_PULL_UP,
	},
	[MMC_HS] = {
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN, /* 0x4 is 0b100 */
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN | /* 0x4 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN | /* 0x4 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN |
			IO_CFG_PULL_UP,
	},
	[MMC_HS_52] = {
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN, /* 0x4 is 0b100 */
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN | /* 0x4 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN | /* 0x4 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN |
			IO_CFG_PULL_UP,
	},
	[MMC_HS_200] = {
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN, /* 0x4 is 0b100 */
		io_cfg_drv_str_sel(0x5) | IO_CFG_PULL_UPDOWN_EN | /* 0x5 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x5) | IO_CFG_PULL_UPDOWN_EN | /* 0x5 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN |
			IO_CFG_PULL_UP,
	},
	[MMC_HS_400] = {
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN, /* 0x4 is 0b100 */
		io_cfg_drv_str_sel(0x5) | IO_CFG_PULL_UPDOWN_EN | /* 0x5 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x5) | IO_CFG_PULL_UPDOWN_EN | /* 0x5 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN |
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN /* 0x4 is 0b100 */
	},
	[MMC_HS_400_ES] = {
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN, /* 0x4 is 0b100 */
		io_cfg_drv_str_sel(0x5) | IO_CFG_PULL_UPDOWN_EN | /* 0x5 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x5) | IO_CFG_PULL_UPDOWN_EN | /* 0x5 is 0b100 */
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN |
			IO_CFG_PULL_UP,
		io_cfg_drv_str_sel(0x4) | IO_CFG_PULL_UPDOWN_EN /* 0x4 is 0b100 */
	}
};

static u32 sdio0_io_cfg[][SDIO_IO_TYPE_NUM] = { /* CLK CMD DATA */
	[SD_LEGACY] = {
		io_cfg_drv_str_sel(0x8) | IO_CFG_PULL_SDIO_DOWN, /* 0x8 is 0b1000 */
		io_cfg_drv_str_sel(0x2) | IO_CFG_PULL_SDIO_UP,   /* 0x2 is 0b0010 */
		io_cfg_drv_str_sel(0x2) | IO_CFG_PULL_SDIO_UP    /* 0x2 is 0b0010 */
	},
	[SD_HS] = {
		io_cfg_drv_str_sel(0x9) | IO_CFG_PULL_SDIO_DOWN, /* 0x9 is 0b1001 */
		io_cfg_drv_str_sel(0x3) | IO_CFG_PULL_SDIO_UP,   /* 0x3 is 0b0011 */
		io_cfg_drv_str_sel(0x3) | IO_CFG_PULL_SDIO_UP    /* 0x3 is 0b0011 */
	},
};

/* avaliable frequency */
#define CLK_400K	400000
#define CLK_25M		25000000
#define CLK_50M		50000000
#define CLK_100M	100000000
#define CLK_150M	150000000
#define CLK_187P5M	187500000
#define CLK_200M	200000000

/* sample drive phase */
#define DRIVE		0
#define SAMPLE		1
#define PHASE_TYPE_NUM	2

static u32 mmc_phase_cfg[][PHASE_TYPE_NUM] = { /* drv, sampl phase */
	[MMC_LEGACY]    = { 16, 0 }, /* 16 for 180 degree */
	[MMC_HS]        = { 16, 4 }, /* 16 for 180 degree */
	[MMC_HS_52]     = { 16, 4 }, /* 16 for 180 degree */
	[MMC_HS_200]    = { 20, 0 }, /* 20 for 225 degree */
	[MMC_HS_400]    = {  9, 0 }, /* 9 for 101.25 degree */
	[MMC_HS_400_ES] = {  9, 0 }  /* 9 for 101.25 degree */
};

static u32 sd_phase_cfg[][PHASE_TYPE_NUM] = {
	[SD_LEGACY]     = { 16, 0 }, /* 16 for 180 degree */
	[SD_HS]         = { 16, 4 }, /* 16 for 180 degree, 4 for 45 degree */
};

/* Do ZQ resistance calibration for eMMC PHY IO */
static int resistance_calibration(void)
{
	int i;
	u32 reg_val;

	reg_val = readl(EMMC_PHY_INITCTRL);
	reg_val |= EMMC_INIT_EN | EMMC_ZCAL_EN;
	writel(reg_val, EMMC_PHY_INITCTRL);

	for (i = 0; i < INITCTRL_CHECK_TIMES; i++) {
		reg_val = readl(EMMC_PHY_INITCTRL);
		if ((reg_val & (EMMC_INIT_EN | EMMC_ZCAL_EN)) == 0)
			return 0;
		udelay(10); /* delay 10 us */
	}

	return -ETIMEDOUT;
}

unsigned int get_ocr_from_bootrom(void)
{
	return readl(REG_SAVE_HCS);
}

void wait_drv_dll_lock(struct sdhci_host *host)
{
	/* doing nothing */
}

void enable_sampl_dll_slave(struct sdhci_host *host)
{
	/* doing nothing */
}

static void dll_reset_assert(void)
{
	const uintptr_t crg_addr =  PERI_CRG_MMC_P4_DLL;
	unsigned int reg;

	reg = readl(crg_addr);
	reg |= CRG_P4_DLL_SRST_REQ;
	writel(reg, crg_addr);
}

static void dll_reset_deassert(void)
{
	const uintptr_t crg_addr = PERI_CRG_MMC_P4_DLL;
	unsigned int reg;

	reg = readl(crg_addr);
	reg &= ~CRG_P4_DLL_SRST_REQ;
	writel(reg, crg_addr);
}

static void wait_p4_dll_lock(void)
{
	const uintptr_t reg_addr = PERI_CRG_MMC_STAT;
	unsigned int timeout = 20;
	unsigned int reg;

	do {
		reg = readl(reg_addr);
		if (reg & CRG_P4_DLL_LOCKED)
			return;

		udelay(1000); /* delay 1000 us */
		timeout--;
	} while (timeout > 0);

	printf("sdhci-bsp: P4 DLL master not locked.\n");
}

static void wait_ds_dll_ready(void)
{
	const uintptr_t reg_addr = PERI_CRG_MMC_STAT;
	unsigned int timeout = 20;
	unsigned int reg;

	do {
		reg = readl(reg_addr);
		if (reg & CRG_DS_DLL_READY)
			return;

		udelay(1000); /* delay 1000 us */
		timeout--;
	} while (timeout > 0);

	printf("sdhci-bsp: DS DLL slave not ready.\n");
}

static void bsp_mmc_priv_init(struct sdhci_host *host)
{
	unsigned int reg;

	reg = sdhci_readl(host, SDHCI_AXI_MBIIU_CTRL);
	reg &= ~SDHCI_UNDEFL_INCR_EN;
	sdhci_writel(host, reg, SDHCI_AXI_MBIIU_CTRL);

	if (host->type == MMC_TYPE_MMC) {
		reg = sdhci_readl(host, SDHCI_EMMC_CTRL);
		reg |= SDHCI_CARD_IS_EMMC;
		sdhci_writel(host, reg, SDHCI_EMMC_CTRL);
	}
}

static unsigned int get_mmc_bus_width(void)
{
	unsigned int sys_stat;
	unsigned int bus_width;

	sys_stat = readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	if (get_sys_boot_mode(sys_stat) != BOOT_FROM_EMMC) {
		/* up to 4 bit mode support when spi nand start up */
		bus_width = MMC_BUS_WIDTH_4_BIT;
	} else {
		bus_width =  (sys_stat & EMMC_BOOT_8BIT) ?
			MMC_BUS_WIDTH_8_BIT : MMC_BUS_WIDTH_4_BIT;
	}

	return bus_width;
}

static void set_drv_str(unsigned int offset, unsigned int drv_str)
{
	unsigned int reg;
	const uintptr_t io_addr = REG_MMC_IO_CFG_BASE + offset;

	reg = readl(io_addr);
	reg &= ~IO_CFG_SDIO_MASK;
	reg |= drv_str;
	writel(reg, io_addr);
}

static void bsp_mmc_set_ioconfig(struct sdhci_host *host)
{
	unsigned int selected_mode = host->mmc->selected_mode;
	unsigned int bus_width;
	unsigned int i;

	if (host->type == MMC_TYPE_MMC) {
		bus_width = get_mmc_bus_width();

		set_drv_str(REG_MMC_CLK_IO, mmc_io_cfg[selected_mode][IO_CLK]);
		set_drv_str(REG_MMC_CMD_IO, mmc_io_cfg[selected_mode][IO_CMD]);
		for (i = 0; i < bus_width; i++)
			set_drv_str(reg_data_io[i],
					 mmc_io_cfg[selected_mode][IO_DATA]);
		set_drv_str(REG_MMC_RSTN_IO, mmc_io_cfg[selected_mode][IO_RST]);

		if (selected_mode == MMC_HS_400 ||
			selected_mode == MMC_HS_400_ES)
			set_drv_str(REG_MMC_DQS_IO,
					 mmc_io_cfg[selected_mode][IO_DS]);
	} else if (host->type == MMC_TYPE_SD) {
		set_drv_str(REG_SDIO0_CLK_IO, sdio0_io_cfg[selected_mode][IO_CLK]);
		set_drv_str(REG_SDIO0_CMD_IO, sdio0_io_cfg[selected_mode][IO_CMD]);
		for (i = 0; i < SD_IO_NUM; i++)
			set_drv_str(reg_sdio0_data_io[i],
					 sdio0_io_cfg[selected_mode][IO_DATA]);
	}
	sdhci_set_uhs_timing(host);
}

static void set_phase(struct sdhci_host *host)
{
	unsigned int drv_phase, sample_phase;
	unsigned int selected_mode = host->mmc->selected_mode;

	drv_phase = 0;
	sample_phase = 0;
	if (host->type == MMC_TYPE_MMC) {
		if (selected_mode == MMC_HS_400 ||
			selected_mode == MMC_HS_200)
			sample_phase = host->tuning_phase;
		else
			sample_phase = mmc_phase_cfg[selected_mode][SAMPLE];

		drv_phase = mmc_phase_cfg[selected_mode][DRIVE];
	} else if (host->type == MMC_TYPE_SD) {
		sample_phase = sd_phase_cfg[selected_mode][SAMPLE];
		drv_phase = sd_phase_cfg[selected_mode][DRIVE];
	}

	set_drv_phase(host, drv_phase);
	enable_sample(host);
	set_sampl_phase(host, sample_phase);

	udelay(25); /* delay 25 us */
}

static void set_crg(struct sdhci_host *host, unsigned int clk)
{
	uintptr_t crg_addr;
	unsigned int sel, reg;
	unsigned int clk_mux[] = {
		CLK_400K, CLK_25M, CLK_50M,
		CLK_100M, CLK_150M, CLK_187P5M, CLK_200M
	};

	crg_addr = (host->type == MMC_TYPE_MMC) ? PERI_CRG_MMC_CLK : PERI_CRG_SDIO0_CLK;
	for (sel = 0x6; sel > 0; sel--) {
		if (clk >= clk_mux[sel])
			break;
	}

	reg = readl(crg_addr);
	reg &= ~CRG_CLK_SEL_MASK;
	reg |= mmc_clk_sel(sel);
	writel(reg, crg_addr);
}

static int bsp_mmc_set_clk(struct sdhci_host *host, unsigned int clk)
{
	disable_card_clk(host);
	udelay(25); /* delay 25 us */
	disable_internal_clk(host);

	if (clk == 0)
		return 0;

	set_crg(host, clk);
	set_phase(host);

	udelay(5); /* delay 5 us */

	enable_internal_clk(host);

	if ((host->mmc->selected_mode == MMC_HS_400) ||
	    (host->mmc->selected_mode == MMC_HS_400_ES) ||
	    (host->mmc->selected_mode == MMC_HS_200)) {
		dll_reset_assert();
		dll_reset_deassert();
		wait_p4_dll_lock();
		wait_sampl_dll_slave_ready(host);
	}

	if ((host->mmc->selected_mode == MMC_HS_400) ||
	    (host->mmc->selected_mode == MMC_HS_400_ES))
		wait_ds_dll_ready();

	enable_card_clk(host);
	udelay(75); /* delay 75 us */

	return 0;
}

static void bsp_mmc_crg_init(void)
{
	unsigned int reg;

	/* eMMC ahb clk enable */
	reg = readl(PERI_CRG_MMC_CLK);
	reg |= CRG_AHB_CLK_EN;
	writel(reg, PERI_CRG_MMC_CLK);

	/* eMMC clk enable */
	reg = readl(PERI_CRG_MMC_CLK);
	reg |= CRG_CLK_EN;
	writel(reg, PERI_CRG_MMC_CLK);

	/* eMMC reset assert*/
	reg = readl(PERI_CRG_MMC_CLK);
	reg |= CRG_SRST_REQ | CRG_TX_SRST_REQ | CRG_RX_SRST_REQ;
	writel(reg, PERI_CRG_MMC_CLK);
	udelay(25); /* delay 25 us */

	reg = readl(PERI_CRG_MMC_CLK);
	reg &= ~CRG_CLK_SEL_MASK;
	reg |= (0x1 << CRG_CLK_SEL_SHIFT); //select 25M clk
	writel(reg, PERI_CRG_MMC_CLK);
	udelay(25); /* delay 25 us */

	/* eMMC reset deassert */
	reg = readl(PERI_CRG_MMC_CLK);
	reg &= ~CRG_SRST_REQ;
	reg &= ~CRG_TX_SRST_REQ;
	reg &= ~CRG_RX_SRST_REQ;
	writel(reg, PERI_CRG_MMC_CLK);
	udelay(1);
}

static int emmc_hardware_init(void)
{
	int ret = -1;
	ret = resistance_calibration();
	if (ret != 0)
		return ret;
	bsp_mmc_crg_init();

	udelay(5000); /* delay 5000 us */
	return 0;
}

static int sd_hardware_init(void)
{
	unsigned int reg;

	reg = readl(PERI_CRG_SDIO0_CLK);
	/* clk enable */
	reg |= CRG_CLK_EN;
	/* eMMC ahb clk enable */
	reg |= CRG_AHB_CLK_EN;
	writel(reg, PERI_CRG_SDIO0_CLK);

	/* reset assert*/
	reg = readl(PERI_CRG_SDIO0_CLK);
	reg |= CRG_SRST_REQ | CRG_TX_SRST_REQ | CRG_RX_SRST_REQ;
	writel(reg, PERI_CRG_SDIO0_CLK);
	udelay(25); /* delay 25 us */

	/* reset deassert */
	reg = readl(PERI_CRG_SDIO0_CLK);
	reg &= ~CRG_SRST_REQ;
	reg &= ~CRG_TX_SRST_REQ;
	reg &= ~CRG_RX_SRST_REQ;
	writel(reg, PERI_CRG_SDIO0_CLK);
	udelay(1);

	reg = readl(REG_PWR_SWITCH_MISC);
	reg |= SDIO0_PWR_EN | SDIO0_PWR_CTRL_BY_MISC;
	reg &= ~(SDIO0_IO_MODE_SEL_1V8 | SDIO0_PWRSW_SEL_1V8);
	writel(reg, REG_PWR_SWITCH_MISC);

	udelay(5000); /* delay 5000 us */

	return 0;
}
