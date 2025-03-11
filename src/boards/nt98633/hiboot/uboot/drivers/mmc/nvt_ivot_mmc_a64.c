/*
 *  driver/mmc/nvt_ivot_mmc_a64.c
 *
 *  Copyright:	Novatek Inc.
 *
 */

#include <asm/io.h>
#include "common.h"
#include <errno.h>
#include <dm.h>
#include <command.h>
#include <mmc.h>
#include <malloc.h>
#include "nvt_ivot_mmc_a64.h"

#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <linux/libfdt.h>

#define mmc_resp_type (MMC_RSP_PRESENT | MMC_RSP_136 | MMC_RSP_CRC | MMC_RSP_BUSY | MMC_RSP_OPCODE)

int mmc_nvt_start_command(struct mmc_nvt_host *host);

#ifdef CONFIG_DM_MMC
/* Novatek IVOT MMC board definitions */
struct nvt_mmc_plat
{
	struct mmc_config cfg;
	struct mmc mmc;
};
#endif

static u32 default_pad_driving[SDIO_MAX_MODE_DRIVING] = {2, 1, 1, 2, 1, 1, 3, 2, 2, 3, 2, 2};

static void mmc_nvt_parse_driving(struct mmc_nvt_host *host)
{
	/* Enable it after dts parsing ready*/
	ulong fdt_addr = nvt_readl((ulong)nvt_shminfo_boot_fdt_addr);
	int nodeoffset;
	u32 *cell = NULL;
	char path[20] = {0};
	int i;

	sprintf(path,"mmc%d", host->id);
	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "max-frequency", NULL);

	if (cell > 0) {
		host->mmc_default_clk = __be32_to_cpu(cell[0]);
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "driving", NULL);

	if (cell < 0) {
		for (i = 0; i < SDIO_MAX_MODE_DRIVING; i++)
			host->pad_driving[i] = default_pad_driving[i];
		printf("\nUse default driving table\n");
	} else {
		for (i = 0; i < SDIO_MAX_MODE_DRIVING; i++) {
			host->pad_driving[i] = __be32_to_cpu(cell[i]);
		}
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "mmc-hs200-1_8v", NULL);
	if (cell > 0)
		host->ext_caps |= MMC_MODE_HS200;

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "indly_sel", NULL);
	if (cell > 0) {
		host->indly_sel = __be32_to_cpu(cell[0]);
	} else {
		host->indly_sel = -1;
	}

	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "neg-sample-edge", NULL);
	if (cell > 0) {
		host->neg_sample_edge = __be32_to_cpu(cell[0]);
	} else {
		host->neg_sample_edge = -1;
	}

	sprintf(path,"/top/sdio");

	nodeoffset = fdt_path_offset((const void*)fdt_addr, path);
	cell = (u32*)fdt_getprop((const void*)fdt_addr, nodeoffset, "pinmux", NULL);
	host->pinmux_value = __be32_to_cpu(cell[0]);

	printf("sdio pinmux(0x%x)\n", host->pinmux_value);
}

//=============================================================================================================

/*
	Get SDIO controller register.

	@param[in] host			host data structure
	@param[in] offset       register offset in SDIO controller (word alignment)

	@return register value
*/
static REGVALUE sdiohost_getreg(struct mmc_nvt_host *host, u32 offset)
{
	return readl(host->base + offset);
}

/*
	Set SDIO controller register.

	@param[in] host		host data structure
	@param[in] offset   offset in SDIO controller (word alignment)
	@param[in] value    register value

	@return void
*/
static void sdiohost_setreg(struct mmc_nvt_host *host, u32 offset, REGVALUE value)
{
	writel(value, host->base + offset);
}

static void sdiohost_setblkfifoen(struct mmc_nvt_host *host, bool enable)
{
	union SDIO_PHY_REG phyreg;

	phyreg.reg = sdiohost_getreg(host, SDIO_PHY_REG_OFS);
	phyreg.bit.BLK_FIFO_EN = enable;
	sdiohost_setreg(host, SDIO_PHY_REG_OFS, phyreg.reg);
}

#define RESET_TIMEOUT	10000
static void sdiohost_setphyrst(struct mmc_nvt_host *host)
{
	union SDIO_PHY_REG phyreg, phyreg_read;
	u8 i;

	phyreg.reg = sdiohost_getreg(host, SDIO_PHY_REG_OFS);
	phyreg.bit.PHY_SW_RST = 1;
	sdiohost_setreg(host, SDIO_PHY_REG_OFS, phyreg.reg);
	while (1)
	{
		phyreg_read.reg = sdiohost_getreg(host, SDIO_PHY_REG_OFS);
		if ((phyreg_read.bit.PHY_SW_RST == 0) || (i == RESET_TIMEOUT))
			break;

		i++;
	}

	if (i == RESET_TIMEOUT)
		printf("phy reset timeout\n");
}

static void sdiohost_setphysample(struct mmc_nvt_host *host, BOOL internal, BOOL before)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, SDIO_DLY0_REG_OFS);
	if (host->neg_sample_edge >= 0) {
		dly0_reg.bit.SAMPLE_CLK_EDGE = host->neg_sample_edge;
	}
	dly0_reg.bit.SRC_CLK_SEL = internal;    // 0: from pad, 1: from internal
	dly0_reg.bit.PAD_CLK_SEL = before;      // 0: after pad, 1: before pad

	sdiohost_setreg(host, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}


static void sdiohost_setinvoutdly(struct mmc_nvt_host *host, BOOL data_outdly_inv, BOOL cmd_outdly_inv)
{
	union SDIO_DLY5_REG dly5_reg;

	dly5_reg.reg = sdiohost_getreg(host, SDIO_DLY5_REG_OFS);
	dly5_reg.bit.DATA_OUT_DLY_INV = data_outdly_inv;
	dly5_reg.bit.CMD_OUT_DLY_INV = cmd_outdly_inv;

	sdiohost_setreg(host, SDIO_DLY5_REG_OFS, dly5_reg.reg);
}

static void sdiohost_setphyclkoutdly(struct mmc_nvt_host *host, u32 dly_setting)
{
	union SDIO_CLOCK_CTRL2_REG clk_crtl_reg;

	clk_crtl_reg.reg = sdiohost_getreg(host, SDIO_CLOCK_CTRL2_REG_OFS);

	clk_crtl_reg.bit.OUTDLY_SEL = dly_setting;

	sdiohost_setreg(host, SDIO_CLOCK_CTRL2_REG_OFS, clk_crtl_reg.reg);
}

static void sdiohost_setphyclkindly(struct mmc_nvt_host *host, u32 dly_setting)
{
	union SDIO_CLOCK_CTRL2_REG clk_crtl_reg;

	clk_crtl_reg.reg = sdiohost_getreg(host, SDIO_CLOCK_CTRL2_REG_OFS);

	clk_crtl_reg.bit.INDLY_SEL = dly_setting;

	sdiohost_setreg(host, SDIO_CLOCK_CTRL2_REG_OFS, clk_crtl_reg.reg);
}

/*
	Set SDIO bus width.

	@param[in] host		host data structure
	@param[in] Width    SDIO controller bus width
			- @b SDIO_BUS_WIDTH1: 1 bit data bus
			- @b SDIO_BUS_WIDTH4: 4 bits data bus
			- @b SDIO_BUS_WIDTH8: 8 bits data bus

	@return void
*/
void sdiohost_setbuswidth(struct mmc_nvt_host *host, u32 width)
{
	union SDIO_BUS_WIDTH_REG widthreg;

	widthreg.reg = 0;
	widthreg.bit.BUS_WIDTH = width;
	sdiohost_setreg(host, SDIO_BUS_WIDTH_REG_OFS, widthreg.reg);
}

/*
	Set SDIO clock enable or disable

	When set to TRUE, SD controller will output SD clock to SD card.
	When set to FALSE, SD controller will not output SD clock to SD card.

	@param[in] host data structure
	@param[in] enableflag   enable clock output
				- @b TRUE: enable SD clock output
				- @b FALSE: disable SD clock output

	@return void
*/
void sdiohost_enclkout(struct mmc_nvt_host *host, BOOL enableflag)
{
	union SDIO_CLOCK_CTRL_REG clkctrlreg;

	clkctrlreg.reg = sdiohost_getreg(host, SDIO_CLOCK_CTRL_REG_OFS);

	if (enableflag == TRUE) {
		/* enabke SDIO CLK */
		clkctrlreg.bit.CLK_DIS = 0;
	} else {
		/* disable SDIO CLK */
		clkctrlreg.bit.CLK_DIS = 1;
	}

	sdiohost_setreg(host, SDIO_CLOCK_CTRL_REG_OFS, clkctrlreg.reg);
}

/**
    Get module clock source
*/
u32 pll_get_sdioclock_src_rate(int id)
{
	u32 clksel = 0;
	u32 clksrc = 0;

	if (id == SDIO_HOST_ID_1) {
		clksel = readl(CG_SDIO_CLKSEL_REG);
		clksel &= (CG_SDIO_CLKSEL_MASK);
		clksel = clksel >> (CG_SDIO_CLKSEL_SHIFT);
	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (id == SDIO_HOST_ID_2) {
		clksel = readl(CG_SDIO2_CLKSEL_REG);
		clksel &= (CG_SDIO2_CLKSEL_MASK);
		clksel = clksel >> (CG_SDIO2_CLKSEL_SHIFT);
	}
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	else if (id == SDIO_HOST_ID_3) {
		clksel = readl(CG_SDIO3_CLKSEL_REG);
		clksel &= (CG_SDIO3_CLKSEL_MASK);
		clksel = clksel >> (CG_SDIO3_CLKSEL_SHIFT);
	}
#endif

	switch (clksel) {
	case _CLKSEL_192MHZ:
		clksrc = SRCCLK_192MHZ;
		break;
#if defined(_CLKSEL_480MHZ)
	case _CLKSEL_480MHZ:
		clksrc = SRCCLK_480MHZ;
		break;
#endif
#if defined(_CLKSEL_320MHZ)
	case _CLKSEL_320MHZ:
		clksrc = SRCCLK_320MHZ;
		break;
#endif
	default:
		break;
	}

	return clksrc;
}

/**
    Set module clock source
*/
void pll_set_sdioclock_src(int id, u32 value)
{
	REGVALUE regdata;

	if (id == SDIO_HOST_ID_1) {
		regdata = readl(CG_SDIO_CLKSEL_REG);
		regdata &= ~(CG_SDIO_CLKSEL_MASK);
		regdata |= (value << (CG_SDIO_CLKSEL_SHIFT));
		writel(regdata, CG_SDIO_CLKSEL_REG);
	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (id == SDIO_HOST_ID_2){
		regdata = readl(CG_SDIO2_CLKSEL_REG);
		regdata &= ~(CG_SDIO2_CLKSEL_MASK);
		regdata |= (value << (CG_SDIO2_CLKSEL_SHIFT));
		writel(regdata, CG_SDIO2_CLKSEL_REG);
	}
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	else if (id == SDIO_HOST_ID_3){
		regdata = readl(CG_SDIO3_CLKSEL_REG);
		regdata &= ~(CG_SDIO3_CLKSEL_MASK);
		regdata |= (value << (CG_SDIO3_CLKSEL_SHIFT));
		writel(regdata, CG_SDIO3_CLKSEL_REG);
	}
#endif
}

/**
    Get module clock rate
*/
u32 pll_get_sdioclock_rate(int id)
{
	REGVALUE regdata = 0;

	if (id == SDIO_HOST_ID_1) {
		regdata = readl(CG_SDIO_CLKDIV_REG);
		regdata &= CG_SDIO_CLKDIV_MASK;
	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (id == SDIO_HOST_ID_2) {
		regdata = readl(CG_SDIO2_CLKDIV_REG);
		regdata &= CG_SDIO2_CLKDIV_MASK;
	}
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	else if (id == SDIO_HOST_ID_3) {
		regdata = readl(CG_SDIO3_CLKDIV_REG);
		regdata &= CG_SDIO3_CLKDIV_MASK;
	}
#endif

	return (u32)regdata;
}

/**
    Set module clock rate
*/
void pll_set_sdioclock_rate(int id, u32 value)
{
	REGVALUE regdata;

	if (id == SDIO_HOST_ID_1) {
		regdata = readl(CG_SDIO_CLKDIV_REG);
		regdata &= ~CG_SDIO_CLKDIV_MASK;
		regdata |= value << CG_SDIO_CLKDIV_SHIFT;
		writel(regdata, CG_SDIO_CLKDIV_REG);
	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (id == SDIO_HOST_ID_2){
		regdata = readl(CG_SDIO2_CLKDIV_REG);
		regdata &= ~CG_SDIO2_CLKDIV_MASK;
		regdata |= value << CG_SDIO2_CLKDIV_SHIFT;
		writel(regdata, CG_SDIO2_CLKDIV_REG);
	}
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	else if (id == SDIO_HOST_ID_3){
		regdata = readl(CG_SDIO3_CLKDIV_REG);
		regdata &= ~CG_SDIO3_CLKDIV_MASK;
		regdata |= value << CG_SDIO3_CLKDIV_SHIFT;
		writel(regdata, CG_SDIO3_CLKDIV_REG);
	}
#endif
}

/*
	Set bus clock.

	@param[in] id		SDIO channel
	@param[in] uiclock  SD bus clock in Hz

	@return void
*/
void nvt_clk_set_rate(int id, u32 uiclock)
{
	u32 divider, src_clk = pll_get_sdioclock_src_rate(id);

	divider = (src_clk + uiclock-1)/uiclock;
	if (!divider)
		divider = 1;

	pll_set_sdioclock_rate(id, divider-1);
}

/*
	Set SDIO bus clock.

	@param[in] host data structure
	@param[in] uiclock  SD bus clock in Hz

	@return void
*/
void sdiohost_setbusclk(struct mmc_nvt_host *host, u32 uiclock, u32 *ns)
{
	/* Disable SDIO clk */
	sdiohost_enclkout(host, FALSE);

	if (uiclock == 0) {
		return;
	}

	nvt_clk_set_rate(host->id, uiclock);

	if (ns)
		*ns = (1000000) / (uiclock/1000);

	/* Enable SDIO clk */
	sdiohost_enclkout(host, TRUE);
}

/*
	Get SDIO bus clock.

	@return unit of Hz
*/
static u32 sdiohost_getbusclk(int id)
{
	u32 uisourceclock;
	u32 uiclockdivider;

	uisourceclock = pll_get_sdioclock_src_rate(id);

	uiclockdivider = pll_get_sdioclock_rate(id);

	return uisourceclock / (uiclockdivider + 1);
}

/*
	Get SDIO Busy or not

	@return TRUE: ready
		FALSE: busy
*/
BOOL sdiohost_getrdy(struct mmc_nvt_host *host)
{
	union SDIO_BUS_STATUS_REG stsreg;

	stsreg.reg = sdiohost_getreg(host, SDIO_BUS_STATUS_REG_OFS);

	return stsreg.bit.CARD_BUSY;
}

/*
	Reset SDIO host controller.

	@return void
*/
void sdiohost_reset(struct mmc_nvt_host *host)
{
	union SDIO_CMD_REG cmdreg;

	cmdreg.reg = sdiohost_getreg(host, SDIO_CMD_REG_OFS);
	cmdreg.bit.SDC_RST = 1;
	sdiohost_setreg(host, SDIO_CMD_REG_OFS, cmdreg.reg);

	while (1) {
		cmdreg.reg = sdiohost_getreg(host, SDIO_CMD_REG_OFS);

		if (cmdreg.bit.SDC_RST == 0)
			break;
	}

	if (host->mmc_input_clk > 1000000)
		udelay(1);
	else
		udelay((1000000/ host->mmc_input_clk) + 1);
}

/*
	Reset SDIO controller data state machine.

	@return void
*/
void sdiohost_resetdata(struct mmc_nvt_host *host)
{
	union SDIO_DATA_CTRL_REG    datactrlreg;
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;
	union SDIO_DLY1_REG dlyreg;
	union SDIO_PHY_REG  phyreg;
	union SDIO_PHY_REG  phyreg_read;
	union SDIO_FIFO_SWITCH_REG  fifoswitch;

	/* //#NT#Fix SDIO data state machine abnormal when DATA CRC/Timeout
	occurs before FIFO count complete */
	union SDIO_STATUS_REG	stsreg;

	/* SDIO bug: force to clear data end status to exit
	waiting data end state*/
	stsreg.reg          = 0;
	stsreg.bit.DATA_END = 1;
	sdiohost_setreg(host, SDIO_STATUS_REG_OFS, stsreg.reg);


	fifoctrlreg.reg = 0;
	sdiohost_setreg(host, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);

	while (1) {
		fifoctrlreg.reg = sdiohost_getreg(host, SDIO_FIFO_CONTROL_REG_OFS);
		if (fifoctrlreg.bit.FIFO_EN == 0)
			break;
	}

	datactrlreg.reg = sdiohost_getreg(host, SDIO_DATA_CTRL_REG_OFS);
	datactrlreg.bit.DATA_EN = 0;
	sdiohost_setreg(host, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);

	/* Fix SDIO data state machine abnormal when DATA
	CRC/Timeout occurs before FIFO count complete */
	/* Do software reset to reset SD state machine */
	sdiohost_reset(host);

	/* patch begin for sd write hang-up or write byte access error */
	fifoswitch.reg = sdiohost_getreg(host, SDIO_FIFO_SWITCH_REG_OFS);
	fifoswitch.bit.FIFO_SWITCH_DLY = 1;
	sdiohost_setreg(host, SDIO_FIFO_SWITCH_REG_OFS, fifoswitch.reg);
	/* patch end for sd write hang-up or write byte access error */

	phyreg.reg = sdiohost_getreg(host, SDIO_PHY_REG_OFS);
	phyreg.bit.PHY_SW_RST = 1;
	sdiohost_setreg(host, SDIO_PHY_REG_OFS, phyreg.reg);
	while (1)
	{
		phyreg_read.reg = sdiohost_getreg(host, SDIO_PHY_REG_OFS);
		if (phyreg_read.bit.PHY_SW_RST == 0)
			break;
	}

	dlyreg.reg = sdiohost_getreg(host, SDIO_DLY1_REG_OFS);
	dlyreg.bit.DATA_READ_DLY = 2;
	dlyreg.bit.DET_READ_DLY = 2;
	sdiohost_setreg(host, SDIO_DLY1_REG_OFS, dlyreg.reg);
}

/*
	Set SDIO controller data timeout.

	@param[in] timeout  time out value between data blocks (unit: SD clock)

	@return void
*/
void sdiohost_setdatatimeout(struct mmc_nvt_host *host, u32 timeout)
{
	union SDIO_DATA_TIMER_REG timerreg;

	timerreg.bit.TIMEOUT = timeout;
	sdiohost_setreg(host, SDIO_DATA_TIMER_REG_OFS, timerreg.reg);
}

/*
	Set PAD driving Sink

	@param[in] name driving sink name
	@param[in] paddriving driving current

	@return
		- @b E_OK: sucess
		- @b Else: fail
*/

static int pad_setdrivingsink(u32 name, u32 paddriving)
{
	unsigned long padreg;
	unsigned long dwoffset = 0x0, bitoffset = 0x0;
	unsigned long driving = paddriving;

	bitoffset = name & 0x1F;
	dwoffset = (name >> 5);

	HAL_READ_UINT32(IOADDR_PAD_REG_BASE + PAD_PUPD0_REG_OFS + (dwoffset << 2), padreg);
	padreg &= ~(0xf << bitoffset);
	padreg |=  (driving << bitoffset);
	HAL_WRITE_UINT32(IOADDR_PAD_REG_BASE + PAD_PUPD0_REG_OFS + (dwoffset << 2), padreg);

	return E_OK;
}

/*
	Set PAD drive/sink of clock pin for specified SDIO channel.

	@param[in] id       SDIO channel ID
			- @b SDIO_HOST_ID_1: SDIO1
			- @b SDIO_HOST_ID_2: SDIO2
	@param[in] driving  desired driving value * 10, unit: mA
				valid value: 50 ~ 200

	@return
		- @b E_OK: sucess
		- @b Else: fail
*/
static int sdiohost_setpaddriving(struct mmc_nvt_host *host, SDIO_SPEED_MODE mode)
{
	UINT32 data_uidriving, cmd_uidriving, clk_uidriving;

	if (mode == SDIO_MODE_DS) {
		data_uidriving = host->pad_driving[SDIO_DS_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_DS_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_DS_MODE_CLK];
	} else if (mode == SDIO_MODE_HS) {
		data_uidriving = host->pad_driving[SDIO_HS_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_HS_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_HS_MODE_CLK];
	} else if (mode == SDIO_MODE_SDR50) {
		data_uidriving = host->pad_driving[SDIO_SDR50_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_SDR50_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_SDR50_MODE_CLK];
	} else {
		data_uidriving = host->pad_driving[SDIO_SDR104_MODE_DATA];
		cmd_uidriving = host->pad_driving[SDIO_SDR104_MODE_CMD];
		clk_uidriving = host->pad_driving[SDIO_SDR104_MODE_CLK];
	}

	if (host->id == SDIO_HOST_ID_1) {
		pad_setdrivingsink(PAD_DS_SDIO_CMD, cmd_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO_D0, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO_D1, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO_D2, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO_D3, data_uidriving);
		return pad_setdrivingsink(PAD_DS_SDIO_CLK, clk_uidriving);
	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (host->id == SDIO_HOST_ID_2) {
		#if defined(TOP_SDIO2_WIDTH_REG)
		if (host->enable_8bits) {
			pad_setdrivingsink(PAD_DS_SDIO2_D4, data_uidriving);
			pad_setdrivingsink(PAD_DS_SDIO2_D5, data_uidriving);
			pad_setdrivingsink(PAD_DS_SDIO2_D6, data_uidriving);
			pad_setdrivingsink(PAD_DS_SDIO2_D7, data_uidriving);
		}
		#endif

		pad_setdrivingsink(PAD_DS_SDIO2_CMD, cmd_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO2_D0, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO2_D1, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO2_D2, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO2_D3, data_uidriving);
		return pad_setdrivingsink(PAD_DS_SDIO2_CLK, clk_uidriving);
	}
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	else if (host->id == SDIO_HOST_ID_3) {
		#if defined(TOP_SDIO3_WIDTH_REG)
		if (host->enable_8bits) {
			pad_setdrivingsink(PAD_DS_SDIO3_D4, data_uidriving);
			pad_setdrivingsink(PAD_DS_SDIO3_D5, data_uidriving);
			pad_setdrivingsink(PAD_DS_SDIO3_D6, data_uidriving);
			pad_setdrivingsink(PAD_DS_SDIO3_D7, data_uidriving);
		}
		#endif

		pad_setdrivingsink(PAD_DS_SDIO3_CMD, cmd_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO3_D0, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO3_D1, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO3_D2, data_uidriving);
		pad_setdrivingsink(PAD_DS_SDIO3_D3, data_uidriving);
		return pad_setdrivingsink(PAD_DS_SDIO3_CLK, clk_uidriving);
	}
#endif

	return 0;
}

/*
	Delay for SDIO module

	@param[in] uid the count for dummy read

	@return void
*/
void sdiohost_delayd(struct mmc_nvt_host *host, u32 uid)
{
	u32 i;

	for (i = uid; i; i--)
		sdiohost_getreg(host, SDIO_CMD_REG_OFS);
}

/*
	Set SDIO controller block size.

	@param[in] host data structure

	@return void
*/
void sdiohost_setblksize(struct mmc_nvt_host *host)
{
	union SDIO_DATA_CTRL_REG datactrlreg;

	datactrlreg.reg = sdiohost_getreg(host, SDIO_DATA_CTRL_REG_OFS);
	datactrlreg.bit.BLK_SIZE = host->data->blocksize;
	sdiohost_setreg(host, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);
}

/*
	Setup SDIO controller data transfer in DMA mode.

	@param[in] uidmaaddress buffer DRAM address
			- possible value: 0x000_0000 ~ 0xFFF_FFFF
	@param[in] uidatalength total transfer length
			- possible value: 0x000_0001 ~ 0x3FF_FFFF
	@param[in] bisread      read/write mode
			- @b TRUE: indicate data read transfer
			- @b FALSE: indicate data write transfer

	@return void
*/
void sdiohost_setupdatatransferdma(struct mmc_nvt_host *host ,
unsigned long uidmaaddress, unsigned long uidatalength, BOOL bisread)
{
	union SDIO_DATA_CTRL_REG datactrlreg;
	union SDIO_DATA_LENGTH_REG datalenreg;
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;
	union SDIO_DMA_START_ADDR_REG dmaaddrreg;
	unsigned long uibusclk;

	/* dummy read for patch */
	sdiohost_delayd(host, 2);

	uibusclk = sdiohost_getbusclk(host->id);

	if (uibusclk >= 48000000)
		sdiohost_delayd(host, 3);
	else if ((uibusclk >= 24000000) && (uibusclk < 48000000))
		sdiohost_delayd(host, 6);
	else if ((uibusclk >= 12000000) && (uibusclk < 24000000))
		sdiohost_delayd(host, 9);
	else
		sdiohost_delayd(host, 21);

	/* patch for sd fifo bug end */

	datactrlreg.reg = sdiohost_getreg(host, SDIO_DATA_CTRL_REG_OFS);
	/* multiple read => disable SDIO INT detection after transfer end */
	if (bisread && (uidatalength > datactrlreg.bit.BLK_SIZE))
		datactrlreg.bit.DIS_SDIO_INT_PERIOD = 1;
	else
		datactrlreg.bit.DIS_SDIO_INT_PERIOD = 0;

	/*move data en after fifo en*/
	/*datactrlreg.bit.DATA_EN = 1;*/
	sdiohost_setreg(host, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);

	dmaaddrreg.reg = 0;
	dmaaddrreg.bit.DRAM_ADDR = (unsigned long)virt_to_phys((void*)uidmaaddress);
	sdiohost_setreg(host, SDIO_DMA_START_ADDR_REG_OFS, dmaaddrreg.reg);

	if (uidmaaddress & DDR_MASK) {
		unsigned long addr_shift = (uidmaaddress & DDR_MASK) >> 32;

		sdiohost_setreg(host, SDIO_DMA_DES_HIGH_START_ADDR_REG_OFS, addr_shift);
	} else
		sdiohost_setreg(host, SDIO_DMA_DES_HIGH_START_ADDR_REG_OFS, 0);

	datalenreg.reg = 0;
	datalenreg.bit.LENGTH = uidatalength;
	sdiohost_setreg(host, SDIO_DATA_LENGTH_REG_OFS, datalenreg.reg);

	fifoctrlreg.reg = 0;

	/* Flush cache in DMA mode*/
	if (!bisread)
		fifoctrlreg.bit.FIFO_DIR = 1;
	else
		fifoctrlreg.bit.FIFO_DIR = 0;

	fifoctrlreg.bit.FIFO_MODE = 1;
	sdiohost_setreg(host, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);

	datactrlreg.reg = sdiohost_getreg(host, SDIO_DATA_CTRL_REG_OFS);
	datactrlreg.bit.DATA_EN = 1;
	sdiohost_setreg(host, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);

	fifoctrlreg.bit.FIFO_EN = 1;
	sdiohost_setreg(host, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);
}

/*
	Setup SDIO controller data transfer in PIO mode.

	@param[in] uidatalength total transfer length
			- possible value: 0x000_0001 ~ 0x3FF_FFFF
	@param[in] bisread      read/write mode
			- @b TRUE: indicate data read transfer
			- @b FALSE: indicate data write transfer

	@return void
*/
void sdiohost_setupdatatransferpio(struct mmc_nvt_host *host,  u32 uidatalength, BOOL bisread)
{
	union SDIO_DATA_CTRL_REG datactrlreg;
	union SDIO_DATA_LENGTH_REG datalenreg;
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;
	u32 uibusclk;

	/* dummy read for patch */
	sdiohost_delayd(host, 2);

	uibusclk = sdiohost_getbusclk(host->id);

	if (uibusclk >= 48000000)
		sdiohost_delayd(host, 3);
	else if ((uibusclk >= 24000000) && (uibusclk < 48000000))
		sdiohost_delayd(host, 6);
	else if ((uibusclk >= 12000000) && (uibusclk < 24000000))
		sdiohost_delayd(host, 9);
	else
		sdiohost_delayd(host, 21);

	/* patch for sd fifo bug end */


	datactrlreg.reg = sdiohost_getreg(host, SDIO_DATA_CTRL_REG_OFS);
	/* multiple read => disable SDIO INT detection after transfer end */
	if (bisread && (uidatalength > datactrlreg.bit.BLK_SIZE))
		datactrlreg.bit.DIS_SDIO_INT_PERIOD = 1;
	else
		datactrlreg.bit.DIS_SDIO_INT_PERIOD = 0;

	datactrlreg.bit.DATA_EN = 1;
	sdiohost_setreg(host, SDIO_DATA_CTRL_REG_OFS, datactrlreg.reg);

	datalenreg.reg = 0;
	datalenreg.bit.LENGTH = uidatalength;
	sdiohost_setreg(host, SDIO_DATA_LENGTH_REG_OFS, datalenreg.reg);

	fifoctrlreg.reg = 0;

	if (!bisread)
		fifoctrlreg.bit.FIFO_DIR = 1;
	else
		fifoctrlreg.bit.FIFO_DIR = 0;

	sdiohost_setreg(host, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);

	fifoctrlreg.bit.FIFO_EN = 1;
	sdiohost_setreg(host, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);

}

/*
	Write SDIO data blocks.


	@note This function should only be called in PIO mode.

	@param[in] pbuf         buffer DRAM address
	@param[in] uiLength     total length (block alignment)

	@return
			- @b E_OK: success
			- @b E_SYS: data CRC or data timeout error
*/
ER sdiohost_writeblock(struct mmc_nvt_host *host, u8 *pbuf, u32 uilength)
{
	u32  uiwordcount, i, *pbufword;
	u32  uifullcount, uiremaincount;
	BOOL    bwordalignment;
	union SDIO_DATA_PORT_REG    datareg;
	union SDIO_FIFO_STATUS_REG  fifostsreg;
	union SDIO_STATUS_REG       stsreg;

	uiwordcount     = (uilength + sizeof(u32) - 1) / sizeof(u32);
	uifullcount     = uiwordcount / SDIO_HOST_DATA_FIFO_DEPTH;
	uiremaincount   = uiwordcount % SDIO_HOST_DATA_FIFO_DEPTH;
	pbufword        = (u32 *)pbuf;

	if ((unsigned long)pbuf & 0x3)
		bwordalignment = FALSE;
	else
		bwordalignment = TRUE;

	while (uifullcount) {
		fifostsreg.reg = sdiohost_getreg(host, SDIO_FIFO_STATUS_REG_OFS);

		if (fifostsreg.bit.FIFO_EMPTY) {
			if (bwordalignment == TRUE) {
				/* Word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					sdiohost_setreg(host, SDIO_DATA_PORT_REG_OFS, \
						*pbufword++);
				}
			} else {
				/* Not word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					datareg.reg = *pbuf++;
					datareg.reg |= (*pbuf++) << 8;
					datareg.reg |= (*pbuf++) << 16;
					datareg.reg |= (*pbuf++) << 24;

					sdiohost_setreg(host, SDIO_DATA_PORT_REG_OFS, \
						datareg.reg);
				}
			}

			uifullcount--;
		}

		stsreg.reg = sdiohost_getreg(host, SDIO_STATUS_REG_OFS);

		if (stsreg.bit.DATA_CRC_FAIL || stsreg.bit.DATA_TIMEOUT) {
			printf("write block fail\n");
			return E_SYS;
		}
	}

	if (uiremaincount) {
		while (1) {

			fifostsreg.reg = \
				sdiohost_getreg(host, SDIO_FIFO_STATUS_REG_OFS);

			if (fifostsreg.bit.FIFO_EMPTY)
				break;

			stsreg.reg = sdiohost_getreg(host, SDIO_STATUS_REG_OFS);

			if (stsreg.bit.DATA_CRC_FAIL || stsreg.bit.DATA_TIMEOUT)
				return E_SYS;
		}

		if (bwordalignment == TRUE) {
			/* Word alignment*/
			for (i = uiremaincount; i; i--) {
				sdiohost_setreg(host, SDIO_DATA_PORT_REG_OFS, \
					*pbufword++);
			}
		} else {
			/* Not word alignment*/
			for (i = uiremaincount; i; i--) {
				datareg.reg = *pbuf++;
				datareg.reg |= (*pbuf++) << 8;
				datareg.reg |= (*pbuf++) << 16;
				datareg.reg |= (*pbuf++) << 24;

				sdiohost_setreg(host, SDIO_DATA_PORT_REG_OFS, \
					datareg.reg);
			}
		}
	}

	return E_OK;
}

/*
	Read SDIO data blocks.

	@note This function should only be called in PIO mode.

	@param[out] pbuf        buffer DRAM address
	@param[in] uiLength     total length (block alignment)

	@return
		- @b E_OK: success
		- @b E_SYS: data CRC or data timeout error
*/
ER sdiohost_readblock(struct mmc_nvt_host *host, u8 *pbuf, u32 uilength)
{
	u32  uiwordcount, i, *pbufword;
	u32  uifullcount, uiremaincount;
	BOOL    bwordalignment;
	union SDIO_DATA_PORT_REG    datareg;
	union SDIO_FIFO_STATUS_REG  fifostsreg;
	union SDIO_STATUS_REG       stsreg;

	uiwordcount     = (uilength + sizeof(u32) - 1) / sizeof(u32);
	uifullcount     = uiwordcount / SDIO_HOST_DATA_FIFO_DEPTH;
	uiremaincount   = uiwordcount % SDIO_HOST_DATA_FIFO_DEPTH;
	pbufword        = (u32 *)pbuf;

	if ((unsigned long)pbuf & 0x3)
		bwordalignment = FALSE;
	else
		bwordalignment = TRUE;

	while (uifullcount) {
		fifostsreg.reg = sdiohost_getreg(host, SDIO_FIFO_STATUS_REG_OFS);

		if (fifostsreg.bit.FIFO_FULL) {
			if (bwordalignment == TRUE) {
				/* Word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					*pbufword++ = \
					sdiohost_getreg(host, SDIO_DATA_PORT_REG_OFS);
				}
			} else {
				/* Not word alignment*/
				for (i = SDIO_HOST_DATA_FIFO_DEPTH; i; i--) {
					datareg.reg = \
					sdiohost_getreg(host, SDIO_DATA_PORT_REG_OFS);

					*pbuf++ = datareg.reg & 0xFF;
					*pbuf++ = (datareg.reg>>8) & 0xFF;
					*pbuf++ = (datareg.reg>>16) & 0xFF;
					*pbuf++ = (datareg.reg>>24) & 0xFF;
				}
			}

			uifullcount--;
		}

		stsreg.reg = sdiohost_getreg(host, SDIO_STATUS_REG_OFS);

		if (stsreg.bit.DATA_CRC_FAIL || stsreg.bit.DATA_TIMEOUT)
			return E_SYS;
	}

	if (uiremaincount) {
		while (1) {
			fifostsreg.reg = \
				sdiohost_getreg(host, SDIO_FIFO_STATUS_REG_OFS);

			if (fifostsreg.bit.FIFO_CNT == uiremaincount)
				break;

			stsreg.reg = sdiohost_getreg(host, SDIO_STATUS_REG_OFS);
			if (stsreg.bit.DATA_CRC_FAIL || stsreg.bit.DATA_TIMEOUT)
				return E_SYS;
		}

		if (bwordalignment == TRUE) {
			/* Word alignment*/
			for (i = uiremaincount; i; i--) {
				*pbufword++ = \
					sdiohost_getreg(host, SDIO_DATA_PORT_REG_OFS);
			}
		} else {
			/* Not word alignment*/
			for (i = uiremaincount; i; i--) {
				datareg.reg = \
					sdiohost_getreg(host, SDIO_DATA_PORT_REG_OFS);

				*pbuf++ = datareg.reg & 0xFF;
				*pbuf++ = (datareg.reg>>8) & 0xFF;
				*pbuf++ = (datareg.reg>>16) & 0xFF;
				*pbuf++ = (datareg.reg>>24) & 0xFF;
			}
		}
	}

	return E_OK;
}

u32 sdiohost_getstatus(struct mmc_nvt_host *host)
{
	return sdiohost_getreg(host, SDIO_STATUS_REG_OFS);
}

void sdiohost_setstatus(struct mmc_nvt_host *host, u32 status)
{
	sdiohost_setreg(host, SDIO_STATUS_REG_OFS, status);
}

static void sdiohost_fifo_data_trans(struct mmc_nvt_host *host,
					unsigned int n)
{
	if (host->data_dir == SDIO_HOST_WRITE_DATA) {
		host->buffer = (u8*) host->data->src;
		sdiohost_writeblock(host, (u8 *)host->buffer, \
			host->buffer_bytes_left);
		host->bytes_left -= host->buffer_bytes_left;
	} else {
		host->buffer = (u8*) host->data->dest;
		sdiohost_readblock(host, (u8 *)host->buffer, \
			host->buffer_bytes_left);
		host->bytes_left -= host->buffer_bytes_left;
	}
}

void sdiohost_clrfifoen(struct mmc_nvt_host *host)
{
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;

	fifoctrlreg.reg = sdiohost_getreg(host, SDIO_FIFO_CONTROL_REG_OFS);
	fifoctrlreg.bit.FIFO_EN = 0;
	sdiohost_setreg(host, SDIO_FIFO_CONTROL_REG_OFS, fifoctrlreg.reg);
}

u32 sdiohost_getfifodir(struct mmc_nvt_host *host)
{
	union SDIO_FIFO_CONTROL_REG fifoctrlreg;

	fifoctrlreg.reg = sdiohost_getreg(host, SDIO_FIFO_CONTROL_REG_OFS);

	return fifoctrlreg.bit.FIFO_DIR;
}

void sdiohost_waitfifoempty(struct mmc_nvt_host *host)
{
	union SDIO_FIFO_STATUS_REG fifostsreg;
	u32 read0, read1;

	read0 = 0;
	while (1) {
		fifostsreg.reg = sdiohost_getreg(host, SDIO_FIFO_STATUS_REG_OFS);
		read1 = fifostsreg.bit.FIFO_EMPTY;
		if (read0 & read1)
			break;
		else
			read0 = read1;
	}
}

void sdiohost_getlongrsp(struct mmc_nvt_host *host, u32 *prsp3, u32 *prsp2, \
					u32 *prsp1, u32 *prsp0)
{
	union SDIO_RSP0_REG rsp0reg;
	union SDIO_RSP1_REG rsp1reg;
	union SDIO_RSP2_REG rsp2reg;
	union SDIO_RSP3_REG rsp3reg;

	rsp0reg.reg = sdiohost_getreg(host, SDIO_RSP0_REG_OFS);
	*prsp0 = (u32) rsp0reg.reg;
	rsp1reg.reg = sdiohost_getreg(host, SDIO_RSP1_REG_OFS);
	*prsp1 = (u32) rsp1reg.reg;
	rsp2reg.reg = sdiohost_getreg(host, SDIO_RSP2_REG_OFS);
	*prsp2 = (u32) rsp2reg.reg;
	rsp3reg.reg = sdiohost_getreg(host, SDIO_RSP3_REG_OFS);
	*prsp3 = (u32) rsp3reg.reg;
}

void sdiohost_getshortrsp(struct mmc_nvt_host *host, u32 *prsp)
{
	union SDIO_RSP0_REG rspreg;

	rspreg.reg = sdiohost_getreg(host, SDIO_RSP0_REG_OFS);
	*prsp = (u32) rspreg.reg;
}

static void mmc_nvt_cmd_done(struct mmc_nvt_host *host)
{
	struct mmc_cmd *cmd = host->cmd;
	host->cmd = NULL;

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			/* response type 2 */
			sdiohost_getlongrsp(host, \
			(u32 *)&cmd->response[0], (u32 *)&cmd->response[1],
			(u32 *)&cmd->response[2], (u32 *)&cmd->response[3]);
			debug("lrsp 0x%x 0x%x 0x%x 0x%x\n", cmd->response[0], cmd->response[1], cmd->response[2], cmd->response[3]);
		} else {
			/* response types 1, 1b, 3, 4, 5, 6 */
			sdiohost_getshortrsp(host, (u32 *)&cmd->response[0]);
			debug("rsp 0x%x\n", cmd->response[0]);
		}
	}
}

static ER sdiohost_transfer(struct mmc_nvt_host *host)
{
	u32 status, qstatus = 0;
	long end_command = 0;
	long end_transfer = 0;
	long err_sts = 1, timeout = 0;
	struct mmc_data *data = host->data;


	while(1) {
		status = sdiohost_getstatus(host);
		//printf("cmd status is 0x%x\n", status);
		if (status & SDIO_STATUS_REG_CMD_SEND) {
			qstatus = status;
			sdiohost_setstatus(host, SDIO_STATUS_REG_CMD_SEND);
			break;
		}
	}

	if (data && host->do_dma) {
		while(1) {
			status = sdiohost_getstatus(host);
			//printf("data status is 0x%x\n", status);
			if ((status & SDIO_STATUS_REG_DATA_END) || \
				(status & SDIO_STATUS_REG_DATA_TIMEOUT)) {
				qstatus = status;
				sdiohost_setstatus(host, SDIO_STATUS_REG_DATA_END);
				break;
			} else if ((status & SDIO_STATUS_REG_RSP_CRC_FAIL) || \
				(status & SDIO_STATUS_REG_DATA_CRC_FAIL)) {
				qstatus = status;
				//printf("crc status is 0x%x\n", status);
				break;
			}
		}
	}


	if ((qstatus & SDIO_STATUS_REG_RSP_CRC_FAIL) ||
		(qstatus & SDIO_STATUS_REG_DATA_CRC_FAIL) ||
		(qstatus & SDIO_STATUS_REG_RSP_TIMEOUT) ||
		(qstatus & SDIO_STATUS_REG_DATA_TIMEOUT)) {
		err_sts = 1;
	}
	else {
		err_sts = 0;
	}
	//printf("qstatus is 0x%x, err_sts %d\n", qstatus, err_sts);

	if (qstatus & MMCST_RSP_TIMEOUT) {
		/* Command timeout */
		if (host->cmd) {
			printf("CMD%d timeout, status %x\n",host->cmd->cmdidx, qstatus);
			if (host->data)
				sdiohost_resetdata(host);
		}
		//printf("MMCST_RSP_TIMEOUT\r\n");
		sdiohost_setstatus(host, MMCST_RSP_TIMEOUT);
		timeout = 1;
	}

	if (qstatus & MMCST_DATA_TIMEOUT) {
		printf("data timeout (CMD%d), status 0x%x\n", host->cmd->cmdidx, qstatus);
		if (host->data)
			sdiohost_resetdata(host);

		//printf("MMCST_DATA_TIMEOUT\r\n");
		sdiohost_setstatus(host, MMCST_DATA_TIMEOUT);
	}

	if (qstatus & MMCST_RSP_CRC_FAIL) {
		/* Command CRC error */
		printf("Command CRC error\n");
		//printf("MMCST_RSP_CRC_FAIL\r\n");
		sdiohost_setstatus(host, MMCST_RSP_CRC_FAIL);
	}

	if (qstatus & MMCST_DATA_CRC_FAIL) {
		/* Data CRC error */
		printf("data %s error\n", \
			(host->data->flags & MMC_DATA_WRITE) ? "write" : "read");
		printf("(CMD%d), status 0x%x\n", host->cmd->cmdidx, qstatus);
		sdiohost_resetdata(host);
		//printf("MMCST_DATA_CRC_FAIL\r\n");
		sdiohost_setstatus(host, MMCST_DATA_CRC_FAIL);
	}

	if (((qstatus & MMCST_RSP_CRC_OK) || (qstatus & MMCST_CMD_SENT)) && !timeout) {
		/* End of command phase */
		if (data == NULL)
			end_command = (unsigned long) host->cmd;
		else {
			if ((host->bytes_left > 0) && (host->do_dma == 0)) {
				/* if datasize < rw_threshold
				 * no RX ints are generated
				 */
				sdiohost_fifo_data_trans(host, host->bytes_left);
			}
		}
		if (qstatus & MMCST_RSP_CRC_OK)
			sdiohost_setstatus(host, MMCST_RSP_CRC_OK);

		if (qstatus & MMCST_CMD_SENT)
			sdiohost_setstatus(host, MMCST_CMD_SENT);
	}

	if ((qstatus & MMCST_RSP_CRC_OK))
		end_command = (unsigned long) host->cmd;

	if (data && !host->do_dma && !timeout) {
		while(1) {
			status = sdiohost_getstatus(host);
			if ((status & SDIO_STATUS_REG_DATA_END) || \
				(status & SDIO_STATUS_REG_DATA_TIMEOUT)) {
				qstatus = status;
				sdiohost_setstatus(host, SDIO_STATUS_REG_DATA_END);
				break;
			} else if ((status & SDIO_STATUS_REG_RSP_CRC_FAIL) || (status & SDIO_STATUS_REG_RSP_TIMEOUT) || \
				(status & SDIO_STATUS_REG_DATA_CRC_FAIL)) {
				qstatus = status;
				break;
			}
		}
	}

	if ((qstatus & MMCST_DATA_END) || (qstatus & MMCST_DATA_CRC_OK)) {
		end_transfer = 1;
		data->dest += data->blocksize;
		if (!sdiohost_getfifodir(host)) {
			if (qstatus & MMCST_DATA_END) {
				if ((qstatus & MMCST_DMA_ERROR)
					!= MMCST_DMA_ERROR) {
					sdiohost_waitfifoempty(host);
				}
				sdiohost_clrfifoen(host);
			}
		}
		if (qstatus & MMCST_DATA_END)
			sdiohost_setstatus(host, MMCST_DATA_END);

		if (qstatus & MMCST_DATA_CRC_OK)
			sdiohost_setstatus(host, MMCST_DATA_CRC_OK);
	}

	if (end_command)
		mmc_nvt_cmd_done(host);

	if (end_transfer && (!host->cmd)) {
		host->data = NULL;
		host->cmd = NULL;
	}

	if (err_sts) {
		if (timeout)
			return -ETIMEDOUT;

		status = sdiohost_getstatus(host);
		if(status)
			printf("end status is 0x%x\n", status);
		return -ECOMM;
	}

	return SDIO_HOST_CMD_OK;
}

/*
	Send SD command to SD bus.

	@param[in] cmd      command value
	@param[in] rsptype  response type
			- @b SDIO_HOST_RSP_NONE: no response is required
			- @b SDIO_HOST_RSP_SHORT: need short (32 bits) response
			- @b SDIO_HOST_RSP_LONG: need long (128 bits) response
	@param[in] beniointdetect enable SDIO INT detect after command end
			- @b TRUE: enable SDIO INT detection
			- @b FALSE: keep SDIO INT detection

	@return command result
	- @b SDIO_HOST_CMD_OK: command execution success
	- @b SDIO_HOST_RSP_TIMEOUT: response timeout. no response got from card.
	- @b SDIO_HOST_RSP_CRCFAIL: response CRC fail.
	- @b SDIO_HOST_CMD_FAIL: other fail.
*/
ER sdiohost_sendcmd(struct mmc_nvt_host *host, \
	u32 cmd, SDIO_HOST_RESPONSE rsptype, BOOL beniointdetect)
{
	union SDIO_CMD_REG cmdreg;
	u32 status;

	/*cmdreg.reg = 0;*/
	cmdreg.reg = sdiohost_getreg(host, SDIO_CMD_REG_OFS);
	cmdreg.bit.CMD_IDX = 0;
	cmdreg.bit.NEED_RSP = 0;
	cmdreg.bit.LONG_RSP = 0;
	cmdreg.bit.RSP_TIMEOUT_TYPE = 0;
	cmdreg.bit.ENABLE_SDIO_INT_DETECT = beniointdetect;

	if (rsptype != SDIO_HOST_RSP_NONE) {
		/* Need response */
		cmdreg.bit.NEED_RSP = 1;

		switch (rsptype) {

		default:
			break;

		case SDIO_HOST_RSP_LONG:
			cmdreg.bit.LONG_RSP = 1;
			break;

		case SDIO_HOST_RSP_SHORT_TYPE2:
			cmdreg.bit.RSP_TIMEOUT_TYPE = 1;
			break;

		case SDIO_HOST_RSP_LONG_TYPE2:
			cmdreg.bit.RSP_TIMEOUT_TYPE = 1;
			cmdreg.bit.LONG_RSP = 1;
			break;
		}
	}

	cmdreg.bit.CMD_IDX = cmd;
	sdiohost_setreg(host, SDIO_CMD_REG_OFS, cmdreg.reg);

	/*Clear all status*/
	status = sdiohost_getstatus(host);
	sdiohost_setstatus(host, status);

	/* Start command/data transmits */
	cmdreg.bit.CMD_EN = 1;
	sdiohost_setreg(host, SDIO_CMD_REG_OFS, cmdreg.reg);

	return sdiohost_transfer(host);
}

ER sdiohost_sendsdcmd(struct mmc_nvt_host *host, u32 cmdpart)
{
	BOOL benintdetect = FALSE;
	SDIO_HOST_RESPONSE rsptype = SDIO_HOST_RSP_NONE;
	u32 param = host->cmd->cmdarg;

	if ((cmdpart & SDIO_CMD_REG_LONG_RSP) == SDIO_CMD_REG_LONG_RSP) {
		if (cmdpart & SDIO_CMD_REG_RSP_TYPE2)
			rsptype = SDIO_HOST_RSP_LONG_TYPE2;
		else
			rsptype = SDIO_HOST_RSP_LONG;
	} else if (cmdpart & SDIO_CMD_REG_VOLTAGE_SWITCH_DETECT) {
		rsptype = SDIO_HOST_RSP_VOLT_DETECT;
	} else if (cmdpart & SDIO_CMD_REG_NEED_RSP) {

		if (cmdpart & SDIO_CMD_REG_RSP_TYPE2)
			rsptype = SDIO_HOST_RSP_SHORT_TYPE2;
		else
			rsptype = SDIO_HOST_RSP_SHORT;
	}

	if (cmdpart & SDIO_CMD_REG_ABORT)
		benintdetect = TRUE;

	sdiohost_setreg(host, SDIO_ARGU_REG_OFS, param);
	//printf("bEnIntDetect %d\r\n",benintdetect);

	return sdiohost_sendcmd(host, cmdpart & SDIO_CMD_REG_INDEX, \
			rsptype, benintdetect);
}

static void mmc_nvt_prepare_data(struct mmc_nvt_host *host)
{
	unsigned long size;

	if (host->data == NULL) {
		return;
	}

	sdiohost_setblksize(host);

	host->buffer = (u8*)(host->data->dest);
	host->bytes_left = host->data->blocks * host->data->blocksize;
	host->data_dir = ((host->data->flags & MMC_DATA_WRITE) ?
		SDIO_HOST_WRITE_DATA : SDIO_HOST_READ_DATA);

#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64))
	if ((host->bytes_left % ARCH_DMA_MINALIGN) || ((unsigned long)host->buffer % ARCH_DMA_MINALIGN) || (host->mmc_input_clk < 24000000)) {
		if (host->mmc_input_clk < 24000000) {
			debug("clk(%dHz) < 24MHz, force to PIO\n", host->mmc_input_clk);
		}
#else
	if ((host->bytes_left % ARCH_DMA_MINALIGN) || ((unsigned long)host->buffer % ARCH_DMA_MINALIGN)) {
#endif
		host->do_dma = 0;
		host->buffer_bytes_left = host->bytes_left;
	} else {
		host->do_dma = 1;
	}

#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64))
	if ((host->mmc_input_clk >= 50000000) && host->do_dma) {
		if (host->data_dir == SDIO_HOST_WRITE_DATA) {
			debug("over 50MHz and cmd write, blkfifoen = 0\n");
			sdiohost_setblkfifoen(host, false);
		} else {
			debug("blkfifoen = 1\n");
			sdiohost_setblkfifoen(host, true);
		}
	}
#else
	sdiohost_setblkfifoen(host, true);
#endif

	if (host->do_dma) {
		size = (unsigned long)(host->buffer) + host->bytes_left;

		if (host->data_dir == SDIO_HOST_WRITE_DATA)
			flush_dcache_range(ALIGN_FLOOR((unsigned long)host->buffer, ARCH_DMA_MINALIGN), (unsigned long)roundup(size,ARCH_DMA_MINALIGN));
		else
			invalidate_dcache_range(ALIGN_FLOOR((unsigned long)host->buffer, ARCH_DMA_MINALIGN), (unsigned long)roundup(size,ARCH_DMA_MINALIGN));

		sdiohost_setupdatatransferdma(host, (unsigned long)host->buffer, \
			host->bytes_left, host->data_dir);
		/* zero this to ensure we take no PIO paths */
		host->bytes_left = 0;
	} else {
		sdiohost_setupdatatransferpio(host, \
			host->buffer_bytes_left, \
			host->data_dir);
	}
}

int mmc_nvt_start_command(struct mmc_nvt_host *host)
{
	u32 cmd_reg = 0;
	char *s;

	switch (host->cmd->resp_type & mmc_resp_type) {
	case MMC_RSP_R1: /* 48 bits, CRC */
		s = ", R1";
		cmd_reg |= SDIO_CMD_REG_NEED_RSP;
		break;
	case MMC_RSP_R1b:
		s = ", R1b";
		/* There's some spec confusion about when R1B is
		 * allowed, but if the card doesn't issue a BUSY
		 * then it's harmless for us to allow it.
		 */
		/*need to check card busy CARD_BUSY2READY bit or
		 *send _SDIO_SD_SEND_STATUS to check
		*/
		cmd_reg |= SDIO_CMD_REG_NEED_RSP;
		/* FALLTHROUGH */
		break;
	case MMC_RSP_R2: /* 136 bits, CRC */
		s = ", R2";
		cmd_reg |= SDIO_CMD_REG_LONG_RSP;
		break;
	case MMC_RSP_R3: /* 48 bits, no CRC */
		s = ", R3/R4";
		cmd_reg |= SDIO_CMD_REG_NEED_RSP;
		break;
	default:
		s = ", Rx";
		cmd_reg |= 0;
		break;
	};

	debug("CMD%d, arg 0x%08x %s\n", host->cmd->cmdidx, host->cmd->cmdarg, s);

	/* Set command index */
	cmd_reg |= host->cmd->cmdidx;

	return sdiohost_sendsdcmd(host, cmd_reg);
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int host_request(struct mmc *dev,
			struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct mmc_nvt_host *host = dev->priv;
#else
static int host_request(struct udevice *udev,
			struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct mmc_nvt_host *host = dev_get_priv(udev);
	struct mmc *dev = mmc_get_mmc_dev(udev);
#endif

	int result;
	unsigned int time_start, time_now, mmcst = 0;
	host->data = data;
	host->cmd = cmd;
	static int cur_clk = 0;

#ifdef CONFIG_NVT_IVOT_EMMC
	if (data || (host->id == SDIO_HOST_ID_2)) {
#else
	if (data) {
#endif
		time_start = get_timer (0);
		while(1) {
			mmcst = sdiohost_getrdy(host);
			if (mmcst == true)
				break;
#ifdef CONFIG_NVT_IVOT_EMMC
			if (host->id != SDIO_HOST_ID_2) {
				time_now = get_timer (0);
				if ((time_now - time_start) > 1000)
					break;
			}
#else
			time_now = get_timer (0);
			if ((time_now - time_start) > 1000)
				break;
#endif
		}

		if (mmcst == false) {
			printf("still busy\n");
			return -ETIMEDOUT;
		}
	}

	if(dev->clock != cur_clk) {
		sdiohost_setdatatimeout(host, (dev->clock/1000)*300);
	}

	cur_clk = dev->clock;

	mmc_nvt_prepare_data(host);
	result = mmc_nvt_start_command(host);

	return result;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
/* MMC uses open drain drivers in the enumeration phase */
static int mmc_host_reset(struct mmc *dev)
{
	return 0;
}
#endif

static int nvt_emmc_arch_host_preinit(struct mmc_nvt_host *host)
{
	unsigned long reg_value;

	if (host->id == SDIO_HOST_ID_1) {
		/* TOP */
		if (host->pinmux_value & PIN_SDIO_CFG_SDIO_1) {
			reg_value = readl(TOP_SDIO_EN_REG);
			reg_value &= ~(TOP_SDIO_EN_MASK);
			reg_value |= (TOP_SDIO_EN_MASK);
			writel(reg_value, TOP_SDIO_EN_REG);
			udelay(10);

			reg_value = readl(TOP_SDIO_EN_GPIO_REG);
			reg_value &= ~(TOP_SDIO_EN_GPIO_MASK);
			writel(reg_value, TOP_SDIO_EN_GPIO_REG);
			udelay(10);
		}

		/* RSTN */
		reg_value = readl(CG_SDIO_RSTN_REG);
		reg_value &= ~(CG_SDIO_RSTN_MASK);
		writel(reg_value, CG_SDIO_RSTN_REG);
		udelay(10);
		reg_value = readl(CG_SDIO_RSTN_REG);
		reg_value |= (CG_SDIO_RSTN_MASK);
		writel(reg_value, CG_SDIO_RSTN_REG);
		udelay(10);

		/* CLKSEL */
		pll_set_sdioclock_src(host->id, _CLKSEL_192MHZ);

		/* CLKDIV */
		reg_value = readl(CG_SDIO_CLKDIV_REG);
		reg_value &= ~(CG_SDIO_CLKDIV_MASK);
		reg_value |= (0x1F0 << CG_SDIO_CLKDIV_SHIFT);
		writel(reg_value, CG_SDIO_CLKDIV_REG);
		udelay(10);

		/* CLKEN */
		reg_value = readl(CG_SDIO_CLKEN_REG);
		reg_value |= (CG_SDIO_CLKEN_MASK);
		writel(reg_value, CG_SDIO_CLKEN_REG);
		udelay(10);
	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (host->id == SDIO_HOST_ID_2) {
		/* TOP */
		if (host->pinmux_value & PIN_SDIO_CFG_SDIO2_1) {
			reg_value = readl(TOP_SDIO2_EN_REG);
			reg_value &= ~(TOP_SDIO2_EN_MASK);
			reg_value |= (TOP_SDIO2_EN_MASK);
			writel(reg_value, TOP_SDIO2_EN_REG);
			udelay(10);

			reg_value = readl(TOP_SDIO2_EN_GPIO_REG);
			reg_value &= ~(TOP_SDIO2_EN_GPIO_MASK);
			writel(reg_value, TOP_SDIO2_EN_GPIO_REG);
			udelay(10);
		}
		#if defined(TOP_SDIO2_WIDTH_REG)
		if (host->pinmux_value & PIN_SDIO_CFG_SDIO2_BUS_WIDTH) {
			host->enable_8bits = 1;

			reg_value = readl(TOP_SDIO2_WIDTH_REG);
			reg_value &= ~(TOP_SDIO2_WIDTH_MASK);
			reg_value |= (TOP_SDIO2_WIDTH_MASK);
			writel(reg_value, TOP_SDIO2_WIDTH_REG);
			udelay(10);

			reg_value = readl(TOP_SDIO2_WIDTH_GPIO_REG);
			reg_value &= ~(TOP_SDIO2_WIDTH_GPIO_MASK);
			writel(reg_value, TOP_SDIO2_WIDTH_GPIO_REG);
			udelay(10);
		}
		#endif

		/* RSTN */
		reg_value = readl(CG_SDIO2_RSTN_REG);
		reg_value &= ~(CG_SDIO2_RSTN_MASK);
		writel(reg_value, CG_SDIO2_RSTN_REG);
		udelay(10);
		reg_value = readl(CG_SDIO2_RSTN_REG);
		reg_value |= (CG_SDIO2_RSTN_MASK);
		writel(reg_value, CG_SDIO2_RSTN_REG);
		udelay(10);

		/* CLKSEL */
		pll_set_sdioclock_src(host->id, _CLKSEL_192MHZ);

		/* CLKDIV */
		reg_value = readl(CG_SDIO2_CLKDIV_REG);
		reg_value &= ~(CG_SDIO2_CLKDIV_MASK);
		reg_value |= (0x1F0 << CG_SDIO2_CLKDIV_SHIFT);
		writel(reg_value, CG_SDIO2_CLKDIV_REG);
		udelay(10);

		/* CLKEN */
		reg_value = readl(CG_SDIO2_CLKEN_REG);
		reg_value |= (CG_SDIO2_CLKEN_MASK);
		writel(reg_value, CG_SDIO2_CLKEN_REG);
		udelay(10);
	}
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	else if (host->id == SDIO_HOST_ID_3) {
		/* TOP */
		if (host->pinmux_value & PIN_SDIO_CFG_SDIO3_1) {
			reg_value = readl(TOP_SDIO3_EN_REG);
			reg_value &= ~(TOP_SDIO3_EN_MASK);
			reg_value |= (TOP_SDIO3_EN_MASK);
			writel(reg_value, TOP_SDIO3_EN_REG);
			udelay(10);

			reg_value = readl(TOP_SDIO3_EN_GPIO_REG);
			reg_value &= ~(TOP_SDIO3_EN_GPIO_MASK);
			writel(reg_value, TOP_SDIO3_EN_GPIO_REG);
			udelay(10);
		}
		#if defined(TOP_SDIO3_WIDTH_REG)
		if (host->pinmux_value & PIN_SDIO_CFG_SDIO3_BUS_WIDTH) {
			host->enable_8bits = 1;

			reg_value = readl(TOP_SDIO3_WIDTH_REG);
			reg_value &= ~(TOP_SDIO3_WIDTH_MASK);
			reg_value |= (TOP_SDIO3_WIDTH_MASK);
			writel(reg_value, TOP_SDIO3_WIDTH_REG);
			udelay(10);

			reg_value = readl(TOP_SDIO3_WIDTH_GPIO_REG);
			reg_value &= ~(TOP_SDIO3_WIDTH_GPIO_MASK);
			writel(reg_value, TOP_SDIO3_WIDTH_GPIO_REG);
			udelay(10);
		}
		#endif

		/* RSTN */
		reg_value = readl(CG_SDIO3_RSTN_REG);
		reg_value &= ~(CG_SDIO3_RSTN_MASK);
		writel(reg_value, CG_SDIO3_RSTN_REG);
		udelay(10);
		reg_value = readl(CG_SDIO3_RSTN_REG);
		reg_value |= (CG_SDIO3_RSTN_MASK);
		writel(reg_value, CG_SDIO3_RSTN_REG);
		udelay(10);

		/* CLKSEL */
		pll_set_sdioclock_src(host->id, _CLKSEL_192MHZ);

		/* CLKDIV */
		reg_value = readl(CG_SDIO3_CLKDIV_REG);
		reg_value &= ~(CG_SDIO3_CLKDIV_MASK);
		reg_value |= (0x1F0 << CG_SDIO3_CLKDIV_SHIFT);
		writel(reg_value, CG_SDIO3_CLKDIV_REG);
		udelay(10);

		/* CLKEN */
		reg_value = readl(CG_SDIO3_CLKEN_REG);
		reg_value |= (CG_SDIO3_CLKEN_MASK);
		writel(reg_value, CG_SDIO3_CLKEN_REG);
		udelay(10);
	}
#endif

	sdiohost_enclkout(host, TRUE);

	sdiohost_setpaddriving(host, SDIO_MODE_DS);

	#if (defined(CONFIG_TARGET_NA51000) || defined(CONFIG_TARGET_NA51055))
	sdiohost_setphysample(host, TRUE, FALSE);
	#else
	sdiohost_setphysample(host, FALSE, TRUE);
	#endif

	sdiohost_resetdata(host);

	/* Delay 1 ms (SD spec) after clock is outputted. */
	/* (Delay 1024 us to reduce code size) */
	udelay(1024);

	sdiohost_setreg(host, SDIO_INT_MASK_REG_OFS, 0xFF);
	sdiohost_setdatatimeout(host, 0x10000000);

	sdiohost_setreg(host, 0x1FC, 0x1);

	return E_OK;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int host_set_ios(struct mmc *dev)
{
	struct mmc_nvt_host *host = dev->priv;
#else
static int host_set_ios(struct udevice *udev)
{
	struct mmc *dev = mmc_get_mmc_dev(udev);
	struct mmc_nvt_host *host = dev_get_priv(udev);
#endif
	//kwinyee debug
	//printf("%s called !,clk = %d, bwidth = %d\n",__func__, dev->clock, dev->bus_width);

	if (host->id >= 3) {
		printf("invalid host id %d\n", host->id);
		return -1;
	}

	if (dev->bus_width) {
		switch (dev->bus_width) {
		case 8:
			sdiohost_setbuswidth(host, SDIO_BUS_WIDTH8);
			break;
		case 4:
			sdiohost_setbuswidth(host, SDIO_BUS_WIDTH4);
			break;
		case 1:
			sdiohost_setbuswidth(host, SDIO_BUS_WIDTH1);
			break;
		}
	}

	if (dev->clock) {
		static int cur_clk[3] = {0};
		if(dev->clock != cur_clk[host->id]) {
			sdiohost_setbusclk(host, dev->clock, (u32*)&host->ns_in_one_cycle);

			/*Set host sampling strategy according to different speeds and modes*/
			if (dev->clock >= 192000000) {
				sdiohost_setpaddriving(host, SDIO_MODE_SDR104);
				sdiohost_setblkfifoen(host, true);
			} else if (dev->clock >= 160000000) {
				sdiohost_setpaddriving(host, SDIO_MODE_SDR104);
				sdiohost_setblkfifoen(host, true);
			} else if (dev->clock >= 120000000) {
				sdiohost_setpaddriving(host, SDIO_MODE_SDR104);
				sdiohost_setblkfifoen(host, true);
			} else if (dev->clock >= 96000000) {
				sdiohost_setpaddriving(host, SDIO_MODE_SDR50);
				sdiohost_setblkfifoen(host, true);
			} else if (dev->clock >= 64000000) {
				sdiohost_setpaddriving(host, SDIO_MODE_SDR50);
				sdiohost_setblkfifoen(host, true);
			} else if (dev->clock >= 25000000) {
				sdiohost_setpaddriving(host, SDIO_MODE_HS);
				sdiohost_setblkfifoen(host, false);
			} else {
				sdiohost_setpaddriving(host, SDIO_MODE_DS);
				sdiohost_setblkfifoen(host, false);
			}

            sdiohost_setphyclkindly(host, nvt_mmc_get_indly(host->id, dev->clock));
            sdiohost_setphyclkoutdly(host, nvt_mmc_get_outdly(host->id, dev->clock));

			if (host->indly_sel >= 0) {
				sdiohost_setphyclkindly(host, host->indly_sel);
				printf("use dts indly(0x%x)\n", host->indly_sel);
			}

			sdiohost_setinvoutdly(host, false, false);

			/*Reset phy to take effect*/
			sdiohost_setphyrst(host);

			sdiohost_reset(host);
		}

		cur_clk[host->id] = dev->clock;
		host->mmc_input_clk = dev->clock;
	}

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
static int mmc_host_getcd(struct mmc *mmc)
#else
static int mmc_host_getcd(struct udevice *dev)
#endif
{
	return 1;
}

static char dev_name_0[] = "NVT_MMC0";
#if (SDIO_HOST_ID_COUNT > 1)
static char dev_name_1[] = "NVT_MMC1";
#endif
#if (SDIO_HOST_ID_COUNT > 2)
static char dev_name_2[] = "NVT_MMC2";
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)
static const struct mmc_ops nvt_hsmmc_ops = {
	.send_cmd = host_request,
	.set_ios = host_set_ios,
	.init = mmc_host_reset,
	.getcd = mmc_host_getcd,
};
/*
 * mmc_host_init - initialize the mmc controller.
 * Set initial clock and power for mmc slot.
 * Initialize mmc struct and register with mmc framework.
 */
int nvt_mmc_init(int id)
{
	struct mmc *dev;
	struct mmc_nvt_host *host;
	struct mmc_config *cfg;

	host = malloc(sizeof(struct mmc_nvt_host));
	if (!host)
		return -ENOMEM;

	memset(host, 0, sizeof(struct mmc_nvt_host));

	cfg = malloc(sizeof(struct mmc_config));
	if (!cfg)
		return -ENOMEM;

	memset(cfg, 0, sizeof(struct mmc_config));

	//init host controler
	host->id = id;
	if (id == SDIO_HOST_ID_1) {
		host->base = IOADDR_SDIO_REG_BASE;
		cfg->name = dev_name_0;
	} else if (id == SDIO_HOST_ID_2) {
		host->base = IOADDR_SDIO2_REG_BASE;
		cfg->name = dev_name_1;
	}

	host->mmc_input_clk = 312500;

	host->mmc_default_clk = 48000000;

	mmc_nvt_parse_driving(host);

	nvt_emmc_arch_host_preinit(host);

	cfg->voltages = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 \
			| MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 \
			| MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36;
	cfg->f_min = 312500;
#ifdef CONFIG_NVT_FPGA_EMULATION
	cfg->f_max = 6000000;
#else
	cfg->f_max = host->mmc_default_clk;
#endif
	cfg->b_max = (32*1024);
	cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;

	if(host->enable_8bits)
		cfg->host_caps |= MMC_MODE_8BIT;

	cfg->ops = &nvt_hsmmc_ops;

	dev = mmc_create(cfg, NULL);
	if (dev == NULL) {
		free(cfg);
		return -1;
	}

	dev->priv = host;

	debug("MMC DEBUG : %s done \n", __FUNCTION__);

	return 0;
}

#else

#ifdef MMC_SUPPORTS_TUNING
static void sdiohost_set_detmode(struct mmc_nvt_host *host, bool det_mode)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DET_MODE = det_mode;
	sdiohost_setreg(host, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

static void sdiohost_set_detdata(struct mmc_nvt_host *host, bool data_is_8bit)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DET_DATA = data_is_8bit;
	sdiohost_setreg(host, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

static void sdiohost_set_phasecmd(struct mmc_nvt_host *host, u32 sel)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DLY_PHASE_SEL = sel;
	sdiohost_setreg(host, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

static void sdiohost_set_phasedata(struct mmc_nvt_host *host, u32 sel)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DATA_DLY_PHASE_SEL = sel;
	sdiohost_setreg(host, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

static void sdiohost_set_phaseunit(struct mmc_nvt_host *host, u32 sel)
{
	union SDIO_DLY0_REG dly0_reg;

	dly0_reg.reg = sdiohost_getreg(host, SDIO_DLY0_REG_OFS);
	dly0_reg.bit.DLY_SEL = sel;
	sdiohost_setreg(host, SDIO_DLY0_REG_OFS, dly0_reg.reg);
}

static void sdiohost_set_detclr(struct mmc_nvt_host *host)
{
	union SDIO_DLY3_REG dly3_reg;

	dly3_reg.reg = sdiohost_getreg(host, SDIO_DLY3_REG_OFS);

	dly3_reg.bit.DET_CLR = 1;
	sdiohost_setreg(host, SDIO_DLY3_REG_OFS, dly3_reg.reg);

	dly3_reg.bit.DET_CLR = 0;
	sdiohost_setreg(host, SDIO_DLY3_REG_OFS, dly3_reg.reg);
}

static uint32_t sdiohost_get_detcmdout(struct mmc_nvt_host *host)
{
	union SDIO_DLY6_REG dly6_reg;

	dly6_reg.reg = sdiohost_getreg(host, SDIO_DLY6_REG_OFS);

	return dly6_reg.bit.DET_CMD_OUT;
}

static uint32_t sdiohost_get_detdataout(struct mmc_nvt_host *host)
{
	union SDIO_DLY2_REG dly2_reg;

	dly2_reg.reg = sdiohost_getreg(host, SDIO_DLY2_REG_OFS);

	return dly2_reg.bit.DET_DATA_OUT;
}

static void nvt_mmc_check_edge(u32 edge_result, u32 sample_clk_edge, u32 *zeropos, u32 *zerolength)
{
	u32 max_length = 0;
	u32 max_pos = 0;
	u32 this_length = 0;
	u32 this_pos = 0;
	u32 pos;

    if (sample_clk_edge == SDIO_HOST_SAMPLE_CLK_EDGE_NEG) {
        edge_result = (edge_result >> SDIO_DELAY_PHASE_SEL_MAX);
    } else if (sample_clk_edge == SDIO_HOST_SAMPLE_CLK_EDGE_BOTH) {
        edge_result |= (edge_result >> SDIO_DELAY_PHASE_SEL_MAX);
    }

	for (pos = 0; pos < SDIO_DELAY_PHASE_SEL_MAX; pos++) {
		if ((edge_result & (1 << pos)) == 0) {
			if (this_length == 0) {
				this_pos = pos;
			}
			this_length++;
		} else {
			if (this_length > max_length) {
				max_length = this_length;
				max_pos = this_pos;
			}
			this_length = 0;
		}
	}

	if ((max_length == 0) || (this_length > max_length)) {
		max_pos = this_pos;
		max_length = this_length;
	}

    // move BestPos to the center of zero bit
    if (max_length) {
        max_pos += (max_length >> 1);
    }

	*zeropos = max_pos;
	*zerolength = max_length;
}

static int nvt_mmc_auto_tuning(struct udevice *dev, uint opcode)
{
    struct nvt_mmc_plat *plat = dev_get_platdata(dev);
    struct mmc *mmc = plat ? &plat->mmc : NULL;
    struct mmc_nvt_host *host = dev_get_priv(dev);
    SDIO_HOST_SAMPLE_CLK_EDGE sample_clk_edge;
    u32 det_cmd_out, det_data_out;
    u32 cmd_phase_sel, data_phase_sel, zero_len;
    u32 i, j, result;

    // check tuning requirement
    if (mmc == NULL) {
        printf("cannot find mmc param\n");
        return E_PAR;
    }

    if (mmc->signal_voltage != MMC_SIGNAL_VOLTAGE_180) {
        printf("signal_voltage %d not 1.8V, tuning cmd is illegal\n",
               mmc->signal_voltage);
        return E_SYS;
    }

    if (mmc->clock <= 48000000) {
        printf("clk %d is less/equal than 48MHz, skip tuning\n",
               mmc->clock);
        return E_OK;
    }

    if (host->ext_caps & MMC_MODE_HS400) {
        printf("skip tuning on HS400\n");
        return E_OK;
    }

    sdiohost_set_detmode(host, SDIO_HOST_DET_MODE_AUTO);

    sdiohost_set_detdata(host, (mmc->bus_width == 8));

    // TODO: use SDIO_HOST_SAMPLE_CLK_EDGE_BOTH in DDR speed mode(e.x. DDR50, HS400)
    sample_clk_edge = host->neg_sample_edge;

    for (i = (SDIO_DELAY_PHASE_UNIT_MAX-1); i < SDIO_DELAY_PHASE_UNIT_MAX; i--) {
        det_cmd_out = 0;
        det_data_out= 0;
        sdiohost_set_phaseunit(host, i);
        sdiohost_setphyrst(host);

        for (j = 0; j < SDIO_AUTO_TUNE_REPEAT; j++) {
            sdiohost_set_detclr(host);

            result = mmc_send_tuning(mmc, opcode, (int *)&result);
            if(result) {
                // use HW default
                sdiohost_set_phaseunit(host, 0);
                sdiohost_setphyrst(host);

                return E_SYS;
            }

            det_cmd_out |= sdiohost_get_detcmdout(host);
            det_data_out |= sdiohost_get_detdataout(host);
        }

        if (det_cmd_out && det_data_out) {
            nvt_mmc_check_edge(det_cmd_out, sample_clk_edge, &cmd_phase_sel, &zero_len);
            if (zero_len < SDIO_EDGE_RESULT_THD) {
                continue;
            }

            nvt_mmc_check_edge(det_data_out, sample_clk_edge, &data_phase_sel, &zero_len);
            if (zero_len < SDIO_EDGE_RESULT_THD) {
                continue;
            }

            sdiohost_set_phasecmd(host, cmd_phase_sel);
            sdiohost_set_phasedata(host, data_phase_sel);
            sdiohost_setphyrst(host);

            break;
        }
    }

	return E_OK;
}
#endif

static const struct dm_mmc_ops nvt_hsmmc_ops = {
	.send_cmd = host_request,
	.set_ios = host_set_ios,
	.get_cd = mmc_host_getcd,
#ifdef MMC_SUPPORTS_TUNING
	.execute_tuning = nvt_mmc_auto_tuning,
#endif
};

static int nvt_mmc_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct nvt_mmc_plat *plat = dev_get_platdata(dev);
	struct mmc_nvt_host *host = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	unsigned long base_addr;

	host->base = (void*)dev_read_addr(dev);
	base_addr = (unsigned long) host->base;
	if (base_addr == (IOADDR_SDIO_REG_BASE)) {
		host->id = SDIO_HOST_ID_1;
		cfg->name = dev_name_0;
		host->base = (void*)IOADDR_SDIO_REG_BASE;

	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (base_addr == (IOADDR_SDIO2_REG_BASE)) {
		host->id = SDIO_HOST_ID_2;
		cfg->name = dev_name_1;
		host->base = (void*)IOADDR_SDIO2_REG_BASE;
	}
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	else if (base_addr == (IOADDR_SDIO3_REG_BASE)) {
		host->id = SDIO_HOST_ID_3;
		cfg->name = dev_name_2;
		host->base = (void*)IOADDR_SDIO3_REG_BASE;
	}
#endif

	host->mmc_input_clk = 312500;

	host->mmc_default_clk = 48000000;

	host->ext_caps = 0;

	host->do_dma = 1;

	mmc_nvt_parse_driving(host);

	nvt_emmc_arch_host_preinit(host);

	cfg->voltages = MMC_VDD_27_28 | MMC_VDD_28_29 | MMC_VDD_29_30 \
			| MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 \
			| MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36 \
			| MMC_VDD_165_195;

#ifdef CONFIG_NVT_FPGA_EMULATION
	cfg->f_min = 312500;
	cfg->f_max = 12000000;
#else
	cfg->f_min = 312500;
	cfg->f_max = host->mmc_default_clk;
#endif

	cfg->b_max = (32*1024);
	cfg->host_caps = MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS;

	if (host->enable_8bits)
		cfg->host_caps |= MMC_MODE_8BIT;

	if (host->ext_caps) {
		cfg->host_caps |= host->ext_caps;
	}

	upriv->mmc = &plat->mmc;

	return 0;
}

#if CONFIG_BLK
static int nvt_mmc_bind(struct udevice *dev)
{
	struct nvt_mmc_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}
#endif

static int nvt_mmc_remove(struct udevice *dev)
{
	struct mmc_nvt_host *host = dev_get_priv(dev);

	if (host->id == SDIO_HOST_ID_1) {
#ifdef  CONFIG_SD_CARD1_POWER_PIN
		u32 reg_val;

		/* Set as GPIO */
		gpio_request(CONFIG_SD_CARD1_POWER_PIN, "sdio_power");
		gpio_request(GPIO_SDIO_CLK, "sdio_clk");
		gpio_request(GPIO_SDIO_CMD, "sdio_cmd");
		gpio_request(GPIO_SDIO_D0, "sdio_d0");
		gpio_request(GPIO_SDIO_D1, "sdio_d1");
		gpio_request(GPIO_SDIO_D2, "sdio_d2");
		gpio_request(GPIO_SDIO_D3, "sdio_d3");

		if (CONFIG_SD_CARD1_ON_STATE)
			gpio_direction_output(CONFIG_SD_CARD1_POWER_PIN, 0);
		else
			gpio_direction_output(CONFIG_SD_CARD1_POWER_PIN, 1);

		reg_val = readl(TOP_SDIO_EN_GPIO_REG);
		reg_val |= (TOP_SDIO_EN_GPIO_MASK);
		writel(reg_val, TOP_SDIO_EN_GPIO_REG);

#if (defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64))
		reg_val = readl(TOP_SDIO_EN_GPIO_2_REG);
		reg_val |= (TOP_SDIO_EN_GPIO_2_MASK);
		writel(reg_val, TOP_SDIO_EN_GPIO_2_REG);
#endif

		/* Set PAD pull down */
		reg_val = readl(PAD_PUPD_SDIO_CLK_REG);
		reg_val &= ~(PAD_PUPD_SDIO_CLK_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO_CLK_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO_CLK_REG);

		reg_val = readl(PAD_PUPD_SDIO_CMD_REG);
		reg_val &= ~(PAD_PUPD_SDIO_CMD_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO_CMD_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO_CMD_REG);

		reg_val = readl(PAD_PUPD_SDIO_D0_REG);
		reg_val &= ~(PAD_PUPD_SDIO_D0_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO_D0_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO_D0_REG);

		reg_val = readl(PAD_PUPD_SDIO_D1_REG);
		reg_val &= ~(PAD_PUPD_SDIO_D1_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO_D1_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO_D1_REG);

		reg_val = readl(PAD_PUPD_SDIO_D2_REG);
		reg_val &= ~(PAD_PUPD_SDIO_D2_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO_D2_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO_D2_REG);

		reg_val = readl(PAD_PUPD_SDIO_D3_REG);
		reg_val &= ~(PAD_PUPD_SDIO_D3_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO_D3_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO_D3_REG);

		/* Set GPIO output low */
		gpio_direction_output(GPIO_SDIO_CLK, 0);
		gpio_direction_output(GPIO_SDIO_CMD, 0);
		gpio_direction_output(GPIO_SDIO_D0, 0);
		gpio_direction_output(GPIO_SDIO_D1, 0);
		gpio_direction_output(GPIO_SDIO_D2, 0);
		gpio_direction_output(GPIO_SDIO_D3, 0);

		/* Disable CLK */
		reg_val = readl(CG_SDIO_CLKEN_REG);
		reg_val &= ~(CG_SDIO_CLKEN_MASK);
		writel(reg_val, CG_SDIO_CLKEN_REG);
#endif
	}
#if (SDIO_HOST_ID_COUNT > 1)
	else if (host->id == SDIO_HOST_ID_2) {
#ifdef CONFIG_SD_CARD2_POWER_PIN
		u32 reg_val;

		/* Set as GPIO */
		gpio_request(CONFIG_SD_CARD2_POWER_PIN, "sdio2_power");
		gpio_request(GPIO_SDIO2_CLK, "sdio2_clk");
		gpio_request(GPIO_SDIO2_CMD, "sdio2_cmd");
		gpio_request(GPIO_SDIO2_D0, "sdio2_d0");
		gpio_request(GPIO_SDIO2_D1, "sdio2_d1");
		gpio_request(GPIO_SDIO2_D2, "sdio2_d2");
		gpio_request(GPIO_SDIO2_D3, "sdio2_d3");

		if (CONFIG_SD_CARD2_ON_STATE)
			gpio_direction_output(CONFIG_SD_CARD2_POWER_PIN, 0);
		else
			gpio_direction_output(CONFIG_SD_CARD2_POWER_PIN, 1);

		reg_val = readl(TOP_SDIO2_EN_GPIO_REG);
		reg_val |= (TOP_SDIO2_EN_GPIO_MASK);
		writel(reg_val, TOP_SDIO2_EN_GPIO_REG);

		/* Set PAD pull down */
		reg_val = readl(PAD_PUPD_SDIO2_CLK_REG);
		reg_val &= ~(PAD_PUPD_SDIO2_CLK_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO2_CLK_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO2_CLK_REG);

		reg_val = readl(PAD_PUPD_SDIO2_CMD_REG);
		reg_val &= ~(PAD_PUPD_SDIO2_CMD_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO2_CMD_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO2_CMD_REG);

		reg_val = readl(PAD_PUPD_SDIO2_D0_REG);
		reg_val &= ~(PAD_PUPD_SDIO2_D0_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO2_D0_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO2_D0_REG);

		reg_val = readl(PAD_PUPD_SDIO2_D1_REG);
		reg_val &= ~(PAD_PUPD_SDIO2_D1_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO2_D1_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO2_D1_REG);

		reg_val = readl(PAD_PUPD_SDIO2_D2_REG);
		reg_val &= ~(PAD_PUPD_SDIO2_D2_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO2_D2_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO2_D2_REG);

		reg_val = readl(PAD_PUPD_SDIO2_D3_REG);
		reg_val &= ~(PAD_PUPD_SDIO2_D3_MASK);
		reg_val |= (PAD_PULLDOWN << PAD_PUPD_SDIO2_D3_SHIFT);
		writel(reg_val, PAD_PUPD_SDIO2_D3_REG);

		/* Set GPIO output low */
		gpio_direction_output(GPIO_SDIO2_CLK, 0);
		gpio_direction_output(GPIO_SDIO2_CMD, 0);
		gpio_direction_output(GPIO_SDIO2_D0, 0);
		gpio_direction_output(GPIO_SDIO2_D1, 0);
		gpio_direction_output(GPIO_SDIO2_D2, 0);
		gpio_direction_output(GPIO_SDIO2_D3, 0);

		/* Disable CLK */
		reg_val = readl(CG_SDIO2_CLKEN_REG);
		reg_val &= ~(CG_SDIO2_CLKEN_MASK);
		writel(reg_val, CG_SDIO2_CLKEN_REG);
#endif
	}
#endif

	return 0;
}

static const struct udevice_id nvt_mmc_ids[] = {
	{
		.compatible = "nvt,nvt_mmc",
	},
#if (SDIO_HOST_ID_COUNT > 1)
	{
		.compatible = "nvt,nvt_mmc2",
	},
#endif
#if (SDIO_HOST_ID_COUNT > 2)
	{
		.compatible = "nvt,nvt_mmc3",
	},
	{
		.compatible = "nvt,nvt_emmc",
	},
#endif
	{},
};

U_BOOT_DRIVER(nvt_mmc_drv) = {
	.name = "nvtivot_mmc",
	.id		= UCLASS_MMC,
	.of_match	= nvt_mmc_ids,
#if CONFIG_BLK
	.bind		= nvt_mmc_bind,
#endif
	.probe = nvt_mmc_probe,
	.ops = &nvt_hsmmc_ops,
	.platdata_auto_alloc_size = sizeof(struct nvt_mmc_plat),
	.priv_auto_alloc_size = sizeof(struct mmc_nvt_host),
	.remove		= nvt_mmc_remove,
	.flags		= DM_FLAG_OS_PREPARE,
};
#endif
