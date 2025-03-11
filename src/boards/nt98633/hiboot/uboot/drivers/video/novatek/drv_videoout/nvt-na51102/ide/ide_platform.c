#include "./include/ide_platform.h"
#include "./include/ide_reg.h"
#include "./include/ide2_int.h"
#include <asm/arch/IOAddress.h>
#include <asm/arch/hardware.h>

#define INREG32(x)          			(*((volatile UINT32*)(x)))
#define OUTREG32(x, y)      			(*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      			OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      			OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

#if defined(__FREERTOS)
#define         dma_getPhyAddr(addr)            ((((IDEDATA_TYPE)(addr))>=0x60000000UL)?((IDEDATA_TYPE)(addr)-0x60000000UL):(IDEDATA_TYPE)(addr))
#elif defined __KERNEL__
#define         dma_getPhyAddr(addr)            ((IDEDATA_TYPE)(addr)) 
#endif
#define CLOCK_GEN_ENABLE_REG0       (IOADDR_CG_REG_BASE + 0x70)
//
//  PLL Register access definition
//
#define PLL_SETREG(ofs, value)      OUTW((IOADDR_CG_REG_BASE+(ofs)), (value))
#define PLL_GETREG(ofs)             INW(IOADDR_CG_REG_BASE+(ofs))


#define PLL_PLL03_CR0_REG_OFS        (0x44c0+0x20)
#define PLL_PLL04_CR0_REG_OFS        (0x4500+0x20)
#define PLL_PLL05_CR0_REG_OFS        (0x4540+0x20)
#define PLL_PLL06_CR0_REG_OFS        (0x4580+0x20)
#define PLL_PLL07_CR0_REG_OFS        (0x45c0+0x20)
#define PLL_PLL08_CR0_REG_OFS        (0x4600+0x20)
#define PLL_PLL09_CR0_REG_OFS        (0x4640+0x20)
#define PLL_PLL10_CR0_REG_OFS        (0x4680+0x20)
#define PLL_PLL11_CR0_REG_OFS        (0x46c0+0x20)
#define PLL_PLL12_CR0_REG_OFS        (0x4700+0x20)
#define PLL_PLL13_CR0_REG_OFS        (0x4740+0x20)
#define PLL_PLL14_CR0_REG_OFS        (0x4780+0x20)
#define PLL_PLL15_CR0_REG_OFS        (0x47c0+0x20)
#define PLL_PLL16_CR0_REG_OFS        (0x4800+0x20)
#define PLL_PLL17_CR0_REG_OFS        (0x4840+0x20)




/**
    PLL ID
*/
typedef enum {
	PLL_ID_0        = 0,        ///< PLL0 (for AXI0)
	PLL_ID_1        = 1,        ///< PLL1 (for internal 480 MHz)
	PLL_ID_2        = 2,        ///< PLL2 (for AXI1)
	PLL_ID_3        = 3,        ///< PLL3 (for DRAM)
	PLL_ID_4        = 4,        ///< PLL4 (for AHB)
	PLL_ID_5        = 5,        ///< PLL5 (for SIE_MCLK2)
	PLL_ID_6        = 6,        ///< PLL6 (for SDC)
	PLL_ID_7        = 7,        ///< PLL7 (for ETH)
	PLL_ID_8        = 8,        ///< PLL8 (for CNN)
	PLL_ID_9        = 9,        ///< PLL9 (for DISP0)
	PLL_ID_10       = 10,       ///< PLL10 (for DISP1)
	PLL_ID_11       = 11,       ///< PLL11 (for SIE_MCLK)
	PLL_ID_12       = 12,       ///< PLL12 (for AUDIO)
	PLL_ID_13       = 13,       ///< PLL13 (for AXI2)
	PLL_ID_14       = 14,       ///< PLL14 (for VENC)
	PLL_ID_15       = 15,       ///< PLL15 (for IPP CODEC BAKUP0)
	PLL_ID_16       = 16,       ///< PLL16 (for CPU)
	PLL_ID_17       = 17,       ///< PLL17 (for DSI)
	PLL_ID_18       = 18,       ///< PLL18 (for IPP CODEC BAKUP1)
	PLL_ID_19       = 19,       ///< PLL19 (for SIE SENSOR)
	PLL_ID_20       = 20,       ///< PLL20 (for USB3 PHY)
	PLL_ID_21       = 21,       ///< PLL21 (for DSP)
	PLL_ID_22       = 22,       ///< PLL22 (for SATA PHY)
	PLL_ID_23       = 23,       ///< PLL23 (for VIE)
	PLL_ID_24       = 24,       ///< PLL24 (for AE)

	PLL_ID_FIXED320 = 25,		///< Fixed 320MHz PLL

	PLL_ID_MAX,
	ENUM_DUMMY4WORD(PLL_ID)
} PLL_ID;



#ifndef CHKPNT
#define CHKPNT    printf("\033[37mCHK: %d, %s\033[0m\r\n", __LINE__, __func__)
#endif

#ifndef DBGD
#define DBGD(x)   printf("\033[0;35m%s=%d\033[0m\r\n", #x, x)
#endif

#ifndef DBGH
#define DBGH(x)   printf("\033[0;35m%s=0x%08X\033[0m\r\n", #x, x)
#endif

#ifndef DBG_DUMP
#define DBG_DUMP(fmtstr, args...) printf(fmtstr, ##args)
#endif

#ifndef DBG_ERR
#define DBG_ERR(fmtstr, args...)  printf("\033[0;31mERR:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#ifndef DBG_WRN
#define DBG_WRN(fmtstr, args...)  printf("\033[0;33mWRN:%s() \033[0m" fmtstr, __func__, ##args)
#endif

#if 0
#define DBG_IND(fmtstr, args...) printf("%s(): " fmtstr, __func__, ##args)
#else
#ifndef DBG_IND
#define DBG_IND(fmtstr, args...)
#endif
#endif
#if defined __UITRON || defined __ECOS

static const DRV_INT_NUM v_ide_int_en[] = {DRV_INT_IDE, DRV_INT_IDE2};
static ID v_ide_flg_id[] = {FLG_ID_IDE, FLG_ID_IDE2};

#elif defined(__FREERTOS)
static ID v_ide_flg_id[IDE_ID_1+2];

static vk_spinlock_t v_ide_spin_locks[IDE_ID_1+2];

static SEM_HANDLE SEMID_IDE[IDE_ID_1+2];

unsigned int ide_debug_level = NVT_DBG_ERR;
static THREAD_HANDLE m_ide_tsk[IDE_ID_1+2];


//UINT32 IOADDR_IDE2_REG_BASE;

#else

//IDEDATA_TYPE IOADDR_IDE_REG_BASE;
//IDEDATA_TYPE IOADDR_IDE2_REG_BASE;

//static ID v_ide_flg_id[IDE_ID_1+2];

//static spinlock_t v_ide_spin_locks[IDE_ID_1+2];

//static struct clk *ide_clk[IDE_ID_1+2];
//static struct clk *ide_if_clk[IDE_ID_1+2];
//static SEM_HANDLE SEMID_IDE[IDE_ID_1+2];
//static struct tasklet_struct * v_p_ide_tasklet[IDE_ID_1+2];
//static THREAD_HANDLE m_ide_tsk[IDE_ID_1+2];


#endif

#define IDE_REQ_POLL_SIZE	16
static IDE_REQ_LIST_NODE v_req_pool[IDE_ID_1+2][IDE_REQ_POLL_SIZE];
static UINT32 v_req_front[IDE_ID_1+2];
static UINT32 v_req_tail[IDE_ID_1+2];
unsigned int ide_log_cnt[IDE_ID_1+2]={0,0};
unsigned int ide_exit_task[IDE_ID_1+2]={0,0};

/*
    @addtogroup mIDrvSys_CG
*/
//@{

#define PLL_CLKSEL_R1_OFFSET        32
#define PLL_CLKSEL_R2_OFFSET        64
#define PLL_CLKSEL_R3_OFFSET        96
#define PLL_CLKSEL_R4_OFFSET        128
#define PLL_CLKSEL_R5_OFFSET        160
#define PLL_CLKSEL_R6_OFFSET        192
#define PLL_CLKSEL_R7_OFFSET        224
#define PLL_CLKSEL_R8_OFFSET        256
#define PLL_CLKSEL_R9_OFFSET        288
#define PLL_CLKSEL_R10_OFFSET       320
#define PLL_CLKSEL_R11_OFFSET       352
#define PLL_CLKSEL_R12_OFFSET       384
#define PLL_CLKSEL_R13_OFFSET       416
#define PLL_CLKSEL_R14_OFFSET       448
#define PLL_CLKSEL_R15_OFFSET       480

#define PLL_CLKSEL_IDE_CLKSRC_OFFSET		20
#define PLL_CLKSEL_IDE2_CLKSRC_OFFSET		24


/**
    @addtogroup mIHALSysCG
*/
//@{

/**
    Clock Enable ID

    This is for pll_enable_clock() and pll_disable_clock().
*/
typedef enum {
	IDE_CLK =37,		    ///< IDE clock
	IDE1_CLK = IDE_CLK,
	IDE2_CLK,		    ///< IDE2 clock
	ENUM_DUMMY4WORD(CG_EN)
} CG_EN;


/*
    Clock select ID

    Clock select ID for pll_set_clock_rate() & pll_get_clock_rate().
*/
typedef enum {
	PLL_CLKSEL_IDE_CLKSRC = 	PLL_CLKSEL_R5_OFFSET + 20,	//< Clock Select Module ID: IDE clock source
	PLL_CLKSEL_IDE2_CLKSRC =	PLL_CLKSEL_R5_OFFSET + 24,	//< Clock Select Module ID: IDE2 clock source

	//Display Clock Divider bit definition
	PLL_CLKSEL_IDE_CLKDIV = 	PLL_CLKSEL_R15_OFFSET + 0,	//< Clock Select Module ID: IDE clock divider
	PLL_CLKSEL_IDE_OUTIF_CLKDIV = PLL_CLKSEL_R15_OFFSET + 8, //< Clock Select Module ID: IDE Output Interface clock divider
	PLL_CLKSEL_IDE2_CLKDIV =	PLL_CLKSEL_R15_OFFSET + 16, //< Clock Select Module ID: IDE2 clock divider
	PLL_CLKSEL_IDE2_OUTIF_CLKDIV = PLL_CLKSEL_R15_OFFSET + 24, //< Clock Select Module ID: IDE2 Output Interface clock divider

	PLL_CLKSEL_MAX_ITEM,									// magic number for pll checking usage.
	ENUM_DUMMY4WORD(PLL_CLKSEL)
} PLL_CLKSEL;

/**
    Clock frequency select ID

    @note This is for pll_set_clock_freq().
*/
typedef enum {
	IDECLK_FREQ,            ///< IDE    CLK freq Select ID
	IDE2CLK_FREQ,           ///< IDE2    CLK freq Select ID
	IDEOUTIFCLK_FREQ,       ///< IDE    output I/F CLK freq Select ID
	IDE2OUTIFCLK_FREQ,      ///< IDE2    output I/F CLK freq Select ID
	PLL_CLKFREQ_MAXNUM,
	ENUM_DUMMY4WORD(PLL_CLKFREQ)
} PLL_CLKFREQ;

/*
    @name   IDE clock source value

    IDE clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKSRC).
*/
//@{
#define PLL_CLKSEL_IDE_CLKSRC_480    (0x00 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL9   (0x01 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as PLL9 (for IDE)
#define PLL_CLKSEL_IDE_CLKSRC_192    (0x02 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as 192MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL10  (0x03 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as PLL10 (for IDE2)
#define PLL_CLKSEL_IDE_CLKSRC_PLL17  (0x04 << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE clock source as PLL17 (for DSI)
//@}


/*
    @name   IDE2 clock source value

    IDE2 clock source value

    @note This is for pll_set_clock_rate(PLL_CLKSEL_IDE2_CLKSRC).
*/
//@{
#define PLL_CLKSEL_IDE2_CLKSRC_480    (0x00 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as 480 MHz
#define PLL_CLKSEL_IDE2_CLKSRC_PLL9   (0x01 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as PLL9 (for IDE)
#define PLL_CLKSEL_IDE2_CLKSRC_192    (0x02 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as 192MHz
#define PLL_CLKSEL_IDE2_CLKSRC_PLL10  (0x03 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as PLL10 (for IDE2)
#define PLL_CLKSEL_IDE2_CLKSRC_PLL17  (0x04 << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))    //< Select IDE2 clock source as PLL17 (for DSI)

#define PLL_CLKSEL_IDE_CLKSRC_MASK      (0x0F << (PLL_CLKSEL_IDE_CLKSRC - PLL_CLKSEL_R5_OFFSET))
#define PLL_CLKSEL_IDE2_CLKSRC_MASK     (0x0F << (PLL_CLKSEL_IDE2_CLKSRC - PLL_CLKSEL_R5_OFFSET))
/*r15*/
#define PLL_CLKSEL_IDE_CLKDIV_MASK        (0xFF << (PLL_CLKSEL_IDE_CLKDIV - PLL_CLKSEL_R15_OFFSET))
#define PLL_CLKSEL_IDE_OUTIF_CLKDIV_MASK  (0xFF << (PLL_CLKSEL_IDE_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))
#define PLL_CLKSEL_IDE2_CLKDIV_MASK       (0xFF << (PLL_CLKSEL_IDE2_CLKDIV - PLL_CLKSEL_R15_OFFSET))
#define PLL_CLKSEL_IDE2_OUTIF_CLKDIV_MASK (0xFF << (PLL_CLKSEL_IDE2_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))


/*
PINMUX_LCDINIT ide_platform_get_disp_mode(UINT32 pin_func_id)
{
#if defined __UITRON || defined __ECOS
	return pinmux_getDispMode((PINMUX_FUNC_ID)pin_func_id);
#else
	return pinmux_get_dispmode((PINMUX_FUNC_ID) pin_func_id);
#endif
}
*/

ER pll_set_pll_enable(PLL_ID id, BOOL b_enable)
{
	unsigned long flags = 0;
	T_PLL_PLL_PWREN_REG pll_en_reg;
	T_PLL_PLL_STATUS_REG pll_status_reg;

	if ((id >= PLL_ID_MAX) || (id == PLL_ID_1) || (id == PLL_ID_FIXED320)) {
		printf("id out of range: PLL%d\r\n", id);
		return E_ID;
	}

	pll_en_reg.reg = PLL_GETREG(PLL_PLL_PWREN_REG_OFS);
	if (b_enable) {
		pll_en_reg.reg |= 1 << id;
	} else {
		pll_en_reg.reg &= ~(1 << id);
	}
	PLL_SETREG(PLL_PLL_PWREN_REG_OFS, pll_en_reg.reg);

	if (b_enable) {
		// Wait PLL power is powered on
		while (1) {
			pll_status_reg.reg = PLL_GETREG(PLL_PLL_STATUS_REG_OFS);
			if (pll_status_reg.reg & (1 << id)) {
				break;
			}
		}
	}

	return E_OK;
}


/**
    Enable module clock

    Enable module clock, module clock must be enabled that it could be work correctly
    @param[in] num  Module enable ID, one module at a time

    @return void
*/
void pll_enable_clock(CG_EN num)
{
	REGVALUE    reg_data;
	UINT32      ui_reg_offset;

	ui_reg_offset = (num >> 5) << 2;
	reg_data  = PLL_GETREG(PLL_CLKEN0_REG_OFS + ui_reg_offset);
	reg_data |= 1 << (num & 0x1F);
	PLL_SETREG(PLL_CLKEN0_REG_OFS + ui_reg_offset, reg_data);
}

/**
    Disable module clock

    Disable module clock

    @param[in] num  Module enable ID, one module at a time

    @return void
*/
void pll_disable_clock(CG_EN num)
{
	REGVALUE    reg_data;
	UINT32      ui_reg_offset;

	ui_reg_offset = (num >> 5) << 2;
	reg_data = PLL_GETREG(PLL_CLKEN0_REG_OFS + ui_reg_offset);
	reg_data &= ~(1 << (num & 0x1F));
	PLL_SETREG(PLL_CLKEN0_REG_OFS + ui_reg_offset, reg_data);

}

UINT32 pll_get_clock_mask(UINT32 ui_num)
{
	switch (ui_num) {

	/*R5*/
	case PLL_CLKSEL_IDE_CLKSRC:
		return PLL_CLKSEL_IDE_CLKSRC_MASK;
	case PLL_CLKSEL_IDE2_CLKSRC:
		return PLL_CLKSEL_IDE2_CLKSRC_MASK;

	/*R15*/
	case PLL_CLKSEL_IDE_CLKDIV:
		return PLL_CLKSEL_IDE_CLKDIV_MASK;
	case PLL_CLKSEL_IDE_OUTIF_CLKDIV:
		return PLL_CLKSEL_IDE_OUTIF_CLKDIV_MASK;
	case PLL_CLKSEL_IDE2_CLKDIV:
		return PLL_CLKSEL_IDE2_CLKDIV_MASK;
	case PLL_CLKSEL_IDE2_OUTIF_CLKDIV:
		return PLL_CLKSEL_IDE2_OUTIF_CLKDIV_MASK;

	default:
		DBG_ERR("Non-supported Clk Mask ID! (0x%x)\r\n", (UINT)ui_num);
		return 0;
	}
}

/**
	Set module clock rate

	Set module clock rate, one module at a time.

	@param[in] ui_num	Module ID(PLL_CLKSEL_*), one module at a time.
						Please refer to pll.h
	@param[in] ui_value Moudle clock rate(PLL_CLKSEL_*_*), please refer to pll.h

	@return void
*/
void pll_set_clock_rate(PLL_CLKSEL clk_sel, UINT32 ui_value)
{
	REGVALUE reg_data;
	UINT32 ui_mask, ui_reg_offset;


	// Check if ui_num/ui_value exceeds limitation of NT96510
	pll_chk_clk_limitation(clk_sel, ui_value);

	ui_mask = pll_get_clock_mask(clk_sel);
	ui_reg_offset = (clk_sel >> 5) << 2;

	//race condition protect. enter critical section
	//loc_multi_cores(flags);

	reg_data = PLL_GETREG(PLL_SYS_CR_REG_OFS + ui_reg_offset);
	reg_data &= ~ui_mask;
	reg_data |= ui_value;
	PLL_SETREG(PLL_SYS_CR_REG_OFS + ui_reg_offset, reg_data);

	//race condition protect. leave critical section
	//unl_multi_cores(flags);
}

/**
	Get module clock rate

	Get module clock rate, one module at a time.

	@param[in] ui_num	Module ID(PLL_CLKSEL_*), one module at a time.
						Please refer to pll.h

	@return Moudle clock rate(PLL_CLKSEL_*_*), please refer to pll.h
*/
UINT32 pll_get_clock_rate(PLL_CLKSEL clk_sel)
{
	UINT32		ui_mask, ui_reg_offset;
	REGVALUE	reg_data;

	if (clk_sel == PLL_CLKSEL_MAX_ITEM) {
		return PLL_CLKSEL_MAX_ITEM;
	}

	ui_mask = pll_get_clock_mask(clk_sel);
	ui_reg_offset = (clk_sel >> 5) << 2;

	reg_data = PLL_GETREG(PLL_SYS_CR_REG_OFS + ui_reg_offset);
	reg_data &= ui_mask;

	return (UINT32)reg_data;
}

/**
    Get PLL frequency

    Get PLL frequency (unit:Hz)
    When spread spectrum is enabled (by pll_set_pll_spread_spectrum()), this API will return lower bound frequency of spread spectrum.

    @param[in] id           PLL ID

    @return PLL frequency (unit:Hz)
*/
UINT32 pll_get_pll_freq(PLL_ID id)
{
	UINT64 pll_ratio;
	T_PLL_PLL2_CR0_REG reg0 = {0};
	T_PLL_PLL2_CR1_REG reg1 = {0};
	T_PLL_PLL2_CR2_REG reg2 = {0};
	const UINT32 pll_address[] = {PLL_PLL03_CR0_REG_OFS, PLL_PLL04_CR0_REG_OFS,
								  PLL_PLL05_CR0_REG_OFS, PLL_PLL06_CR0_REG_OFS, PLL_PLL07_CR0_REG_OFS,
								  PLL_PLL08_CR0_REG_OFS, PLL_PLL09_CR0_REG_OFS, PLL_PLL10_CR0_REG_OFS,
								  PLL_PLL11_CR0_REG_OFS, PLL_PLL12_CR0_REG_OFS, PLL_PLL13_CR0_REG_OFS,
								  PLL_PLL14_CR0_REG_OFS, PLL_PLL15_CR0_REG_OFS, PLL_PLL16_CR0_REG_OFS
								  , PLL_PLL17_CR0_REG_OFS};



	if ((id == PLL_ID_FIXED320)) {
		return 320000000;
	} else if (id == PLL_ID_1) {
		return 480000000;
	} 
	
	reg0.reg = PLL_GETREG(pll_address[id - PLL_ID_3]);
	reg1.reg = PLL_GETREG(pll_address[id - PLL_ID_3] + 0x04);
	reg2.reg = PLL_GETREG(pll_address[id - PLL_ID_3] + 0x08);


	pll_ratio = (reg2.bit.PLL_RATIO2 << 16) | (reg1.bit.PLL_RATIO1 << 8) | (reg0.bit.PLL_RATIO0 << 0);

	return 12000000 * pll_ratio / 131072;
}


/*
    Check module clock limitation

    Check input clock mux/select.
    If outputted frequency is out of spec, dump warning messsage to UART.

    @param[in] ui_num            Clock mux to check
    @param[in] ui_value          Clock rate of ui_num

    @return void
*/
void pll_chk_clk_limitation(PLL_CLKSEL ui_num, UINT32 ui_value)
{
	UINT32 ui_freq = 0;

	switch (ui_num) {
	case PLL_CLKSEL_IDE_CLKDIV:
	case PLL_CLKSEL_IDE_OUTIF_CLKDIV: {
			ui_freq = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
	
			switch (ui_freq) {
			case PLL_CLKSEL_IDE_CLKSRC_480:
				ui_freq = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL10:
				ui_freq = pll_get_pll_freq(PLL_ID_10);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL9:
				ui_freq = pll_get_pll_freq(PLL_ID_9);
				break;
	
			default:
				return;
			}
	
			ui_freq = ui_freq / ((ui_value >> (ui_num & 0x1F)) + 1);
	
			if (ui_freq > 300000000) {
				DBG_WRN("IDE(0x%X) clock must not exceed 300Mhz!\r\n", ui_num);
			}
		}
		return;
	case PLL_CLKSEL_IDE2_CLKDIV:
	case PLL_CLKSEL_IDE2_OUTIF_CLKDIV: {
			ui_freq = pll_get_clock_rate(PLL_CLKSEL_IDE2_CLKSRC);
	
			switch (ui_freq) {
			case PLL_CLKSEL_IDE2_CLKSRC_480:
				ui_freq = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL10:
				ui_freq = pll_get_pll_freq(PLL_ID_10);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL9:
				ui_freq = pll_get_pll_freq(PLL_ID_9);
				break;
	
			default:
				return;
			}
	
			ui_freq = ui_freq / ((ui_value >> (ui_num & 0x1F)) + 1);
	
			if (ui_freq > 300000000) {
				DBG_WRN("IDE(0x%X) clock must not exceed 300Mhz!\r\n", ui_num);
			}
		}
		return;

	default:
		return;
	}
}


/**
    Set the module clock frequency

    This api setup the module clock frequency by chnaging module clock divider.
    If the module has multiple source clock choices, user must set the correct
    source clock before calling this API.
\n  If the target frequency can not well divided from source frequency,this api
    would output warning message.

    @param[in] clock_id    Module select ID, refer to structure PLL_CLKFREQ.
    @param[in] ui_freq   Target clock frequency. Unit in Hertz.

    @return
     - @b E_ID:     clock_id is not support in this API.
     - @b E_PAR:    Target frequency can not be divided with no remainder.
     - @b E_OK:     Done and success.
*/
ER pll_set_clock_freq(PLL_CLKFREQ clock_id, UINT32 ui_freq)
{
	UINT32 source_clock, divider, clock_source;

	if (clock_id >= PLL_CLKFREQ_MAXNUM) {
		return E_ID;
	}

	// Get Src Clock Frequency
	switch (clock_id) {

	case IDECLK_FREQ: {
			clock_source = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			if (clock_source == PLL_CLKSEL_IDE_CLKSRC_480) {
				source_clock = pll_get_pll_freq(PLL_ID_1);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL10) {
				source_clock = pll_get_pll_freq(PLL_ID_10);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL9) {
				source_clock = pll_get_pll_freq(PLL_ID_9);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL17) {
				source_clock = pll_get_pll_freq(PLL_ID_17);
			} else {
				return E_PAR;
			}
		}
		break;

	case IDE2CLK_FREQ: {
			clock_source = pll_get_clock_rate(PLL_CLKSEL_IDE2_CLKSRC);
			if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_480) {
				source_clock = pll_get_pll_freq(PLL_ID_1);
			} else if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_PLL10) {
				source_clock = pll_get_pll_freq(PLL_ID_10);
			} else if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_PLL9) {
				source_clock = pll_get_pll_freq(PLL_ID_9);
			} else if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_PLL17) {
				source_clock = pll_get_pll_freq(PLL_ID_17);
			} else {
				return E_PAR;
			}
		}
		break;

	case IDEOUTIFCLK_FREQ: {
			clock_source = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			if (clock_source == PLL_CLKSEL_IDE_CLKSRC_480) {
				source_clock = pll_get_pll_freq(PLL_ID_1);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL10) {
				source_clock = pll_get_pll_freq(PLL_ID_10);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL9) {
				source_clock = pll_get_pll_freq(PLL_ID_9);
			} else if (clock_source == PLL_CLKSEL_IDE_CLKSRC_PLL17) {
				source_clock = pll_get_pll_freq(PLL_ID_17);
			}

			else {
				return E_PAR;
			}
		}
		break;

	case IDE2OUTIFCLK_FREQ: {
			clock_source = pll_get_clock_rate(PLL_CLKSEL_IDE2_CLKSRC);
			if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_480) {
				source_clock = pll_get_pll_freq(PLL_ID_1);
			} else if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_PLL10) {
				source_clock = pll_get_pll_freq(PLL_ID_10);
			} else if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_PLL9) {
				source_clock = pll_get_pll_freq(PLL_ID_9);
			} else if (clock_source == PLL_CLKSEL_IDE2_CLKSRC_PLL17) {
				source_clock = pll_get_pll_freq(PLL_ID_17);
			}

			else {
				return E_PAR;
			}
		}
		break;
	default:
		return E_PAR;
	}


	// Calculate the clock divider value
	divider = (source_clock + ui_freq - 1) / ui_freq;

	// prevent error case
	if (divider == 0) {
		divider = 1;
	}
	// Set Clock divider
	switch (clock_id) {
	case IDECLK_FREQ:
		pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV, PLL_IDE_CLKDIV(divider - 1));
		break;
	case IDE2CLK_FREQ:
		pll_set_clock_rate(PLL_CLKSEL_IDE2_CLKDIV, PLL_IDE2_CLKDIV(divider - 1));
		break;
	case IDEOUTIFCLK_FREQ:
		pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV, PLL_IDE_OUTIF_CLKDIV(divider - 1));
		break;
	case IDE2OUTIFCLK_FREQ:
		pll_set_clock_rate(PLL_CLKSEL_IDE2_OUTIF_CLKDIV, PLL_IDE2_OUTIF_CLKDIV(divider - 1));
		break;
	default :
		return E_PAR;
	}

	// Output warning msg if target freq can not well divided from source freq
	if (source_clock % ui_freq) {
		UINT32 ui_real_freq = source_clock / divider;

		// Truncate inaccuray under 1000 Hz
		ui_real_freq = (ui_real_freq + 50) / 1000;
		ui_freq /= 1000;
		if (ui_freq != ui_real_freq) {
			DBG_WRN("Target(%d) freq can not be divided with no remainder! Result is %dHz.\r\n", clock_id, (UINT)(source_clock / divider));
			return E_PAR;
		}
	}

	return E_OK;
}

/**
    Get the module clock frequency

    This api get the module clock frequency.

    @param[in] clock_id    Module select ID, refer to structure PLL_CLKFREQ.
    @param[out] p_freq   Return clock frequency. Unit in Hertz.

    @return
     - @b E_ID:     clock_id is not support in this API.
     - @b E_PAR:    Target frequency can not be divided with no remainder.
     - @b E_OK:     Done and success.
*/
ER pll_get_clock_freq(PLL_CLKFREQ clock_id, UINT32 *p_freq)
{
	UINT32 source_clock = 0, divider = 0, clock_source = 0;

	if (clock_id >= PLL_CLKFREQ_MAXNUM) {
		return E_ID;
	}

	if (p_freq == NULL) {
		DBG_ERR("input p_freq is NULL\r\n");
		return E_PAR;
	}

	// Get Src Clock Frequency
	switch (clock_id) {
	case IDECLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			divider = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKDIV) >> (PLL_CLKSEL_IDE_CLKDIV & 0x1F);

			switch (clock_source) {
			case PLL_CLKSEL_IDE_CLKSRC_480:
				source_clock = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL10:
				source_clock = pll_get_pll_freq(PLL_ID_10);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL9:
				source_clock = pll_get_pll_freq(PLL_ID_9);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL17:
				source_clock = pll_get_pll_freq(PLL_ID_17);
				break;

			default:
				return E_PAR;
			}
		}
		break;
	case IDE2CLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_IDE2_CLKSRC);
			divider = pll_get_clock_rate(PLL_CLKSEL_IDE2_CLKDIV) >> (PLL_CLKSEL_IDE2_CLKDIV & 0x1F);

			switch (clock_source) {
			case PLL_CLKSEL_IDE2_CLKSRC_480:
				source_clock = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL10:
				source_clock = pll_get_pll_freq(PLL_ID_10);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL9:
				source_clock = pll_get_pll_freq(PLL_ID_9);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL17:
				source_clock = pll_get_pll_freq(PLL_ID_17);
				break;

			default:
				return E_PAR;
			}
		}
		break;
	case IDEOUTIFCLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_IDE_CLKSRC);
			divider = pll_get_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV) >> (PLL_CLKSEL_IDE_OUTIF_CLKDIV & 0x1F);

			switch (clock_source) {
			case PLL_CLKSEL_IDE_CLKSRC_480:
				source_clock = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL10:
				source_clock = pll_get_pll_freq(PLL_ID_10);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL9:
				source_clock = pll_get_pll_freq(PLL_ID_9);
				break;
			case PLL_CLKSEL_IDE_CLKSRC_PLL17:
				source_clock = pll_get_pll_freq(PLL_ID_17);
				break;

			default:
				return E_PAR;
			}
		}
		break;
	case IDE2OUTIFCLK_FREQ: {
			clock_source  = pll_get_clock_rate(PLL_CLKSEL_IDE2_CLKSRC);
			divider = pll_get_clock_rate(PLL_CLKSEL_IDE2_OUTIF_CLKDIV) >> (PLL_CLKSEL_IDE2_OUTIF_CLKDIV & 0x1F);

			switch (clock_source) {
			case PLL_CLKSEL_IDE2_CLKSRC_480:
				source_clock = pll_get_pll_freq(PLL_ID_1);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL10:
				source_clock = pll_get_pll_freq(PLL_ID_10);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL9:
				source_clock = pll_get_pll_freq(PLL_ID_9);
				break;
			case PLL_CLKSEL_IDE2_CLKSRC_PLL17:
				source_clock = pll_get_pll_freq(PLL_ID_17);
				break;

			default:
				return E_PAR;
			}
		}
		break;
	default:
		return E_PAR;
	}

	*p_freq = source_clock / (divider + 1);

	return E_OK;
}


void ide_platform_delay_ms(UINT32 ms)
{
	udelay(1000 *ms);
}

void ide_platform_delay_us(UINT32 us)
{
	ndelay(1000 * us);
}

#if 0
ER ide_platform_flg_clear(IDE_ID id, FLGPTN flg)
{
	return clr_flg(v_ide_flg_id[id], flg);
}

ER ide_platform_flg_set(IDE_ID id, FLGPTN flg)
{
	 return iset_flg(v_ide_flg_id[id], flg);
}

ER ide_platform_flg_wait(IDE_ID id, FLGPTN flg)
{
	FLGPTN              ui_flag;

	return wai_flg(&ui_flag, v_ide_flg_id[id], flg, TWF_ORW | TWF_CLR);
}

ER ide_platform_sem_set(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	return sig_sem(SEMID_IDE[id]);
#else
	SEM_SIGNAL(SEMID_IDE[id]);
	return E_OK;
#endif
}

ER ide_platform_sem_wait(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	return wai_sem(SEMID_IDE[id]);
#else
	return SEM_WAIT(SEMID_IDE[id]);
#endif
}
#endif

ULONG ide_platform_spin_lock(IDE_ID id)
{
	return 0;
}

void ide_platform_spin_unlock(IDE_ID id, ULONG flag)
{
	return;
}

void ide_platform_sram_enable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	if (id == IDE_ID_1)
		pinmux_disable_sram_shutdown(IDE_SD);
	else
		;//pll_disableSramShutDown(IDE2_RSTN);
#elif defined __FREERTOS
	nvt_enable_sram_shutdown(IDE_SD);
#else
#endif
}

void ide_platform_sram_disable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	if (id == IDE_ID_1)
		pinmux_enable_sram_shutdown(IDE_SD);
	else
		;//pll_enableSramShutDown(IDE2_RSTN);
#elif defined __FREERTOS
	nvt_disable_sram_shutdown(IDE_SD);
#else
#endif
}

void ide_platform_int_enable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	drv_enableInt(v_ide_int_en[id]);
#else
#endif
}

void ide_platform_int_disable(IDE_ID id)
{
#if defined __UITRON || defined __ECOS
	drv_disableInt(v_ide_int_en[id]);
#else
#endif
}

IDEDATA_TYPE ide_platform_va2pa(IDEDATA_TYPE addr)
{

	return dma_getPhyAddr(addr);

}

UINT32 factor_caculate(UINT16 x, UINT16 y, BOOL h)
{
	UINT32 a, b, c;
	UINT64 temp;

	if (h == TRUE) {

		a = (x - 1) << 15;
		b = (y - 1);

		temp = (UINT64) a;
//#if defined __UITRON || defined __ECOS || defined __FREERTOS
		temp = temp/b;
//#else
//		do_div(temp, b);
//#endif
		c = (UINT32) temp;
	} else {
		a = (x - 1) << 12;
		b = (y - 1);
		temp = (UINT64) a;
//#if defined __UITRON || defined __ECOS || defined __FREERTOS
		temp = temp/b;
//#else
//		do_div(temp, b);
//#endif
		c = (UINT32) temp;
	}

	//c = temp - (1 << 15);

	return c;
}

void ide_platform_clk_en(IDE_ID id)
{
	DBG_IND("ide%d clk enable\n", id);

	if (id == IDE_ID_1)
		pll_enable_clock(IDE1_CLK);
	else
		pll_enable_clock(IDE2_CLK);

	DBG_IND("ide clk enable finished\n");
}

void ide_platform_clk_dis(IDE_ID id)
{
	DBG_IND("ide%d clk disable\n", id);
	if (id == IDE_ID_1)
		pll_disable_clock(IDE1_CLK);
	else
		pll_disable_clock(IDE2_CLK);

	DBG_IND("ide%d clk disable finished\n", id);
}

void ide_platform_set_iffreq(IDE_ID id, UINT32 freq)
{
	DBG_IND("ide%d if clk %d Hz\n", id,(int)freq);
	if (id == IDE_ID_1)
		pll_set_clock_freq(IDEOUTIFCLK_FREQ, freq);
	else
		pll_set_clock_freq(IDE2OUTIFCLK_FREQ, freq);

}

void ide_platform_set_freq(IDE_ID id, UINT32 freq)
{
	DBG_IND("ide%d clk %d Hz\n", id,(int)freq);
	if (id == IDE_ID_1)
		pll_set_clock_freq(IDECLK_FREQ, freq);
	else
		pll_set_clock_freq(IDE2CLK_FREQ, freq);

}

UINT32 ide_platform_get_iffreq(IDE_ID id)
{
	UINT32 rate = 0;

	if (id == IDE_ID_1)
		pll_get_clock_freq(IDEOUTIFCLK_FREQ, &rate);
	else
		pll_get_clock_freq(IDE2OUTIFCLK_FREQ, &rate);


	DBG_IND("ide if clk %d Hz\n", (int)rate);

	return rate;
}

UINT32 ide_platform_get_freq(IDE_ID id)
{
	UINT32 rate = 0;

	if (id == IDE_ID_1)
		pll_get_clock_freq(IDECLK_FREQ, &rate);
	else
		pll_get_clock_freq(IDE2CLK_FREQ, &rate);

	DBG_IND("ide clk %d Hz\n", (int)rate);

	return rate;
}

ER ide_platform_set_clksrc(IDE_ID id, UINT32 src)
{
#if 1//defined __UITRON || defined __ECOS || defined __FREERTOS
	if (id == IDE_ID_1)
		pll_set_clock_rate(PLL_CLKSEL_IDE_CLKSRC, (src<<PLL_CLKSEL_IDE_CLKSRC_OFFSET));
	else
		pll_set_clock_rate(PLL_CLKSEL_IDE2_CLKSRC, (src<<PLL_CLKSEL_IDE2_CLKSRC_OFFSET));
	
	return E_OK;

#else
	struct clk *source_clk;

	if (src == 0) {
		source_clk = clk_get(NULL, "fix480m");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
		clk_set_parent(ide_if_clk[id], source_clk);
	} else if (src == 1) {
		source_clk = clk_get(NULL, "pll6");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
		clk_set_parent(ide_if_clk[id], source_clk);
	} else if (src == 2) {
		source_clk = clk_get(NULL, "pll4");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
		clk_set_parent(ide_if_clk[id], source_clk);
	} else if (src == 3) {
		source_clk = clk_get(NULL, "pll9");
		if (IS_ERR(source_clk)) {
			DBG_ERR("ide get clk source err\n");
			return E_SYS;
		}
		clk_set_parent(ide_clk[id], source_clk);
		clk_set_parent(ide_if_clk[id], source_clk);
	} else {
		DBG_ERR("ide clk source %d not support\n", src);
		return E_SYS;
	}
	return E_OK;
#endif

}

/*
        Check if service queue is empty
*/
BOOL ide_platform_list_empty(IDE_ID id)
{
	//if (id > IDE_ID_1) {
     //           DBG_ERR("invalid id %d\r\n", id);
    //            return E_SYS;
    //    }

	if (v_req_front[id] == v_req_tail[id]) {
		// queue empty
		return TRUE;
	} else {
		return FALSE;
	}
}

/**
    Add request descriptor to service queue
*/
ER ide_platform_add_list(IDE_ID id, KDRV_CALLBACK_FUNC *p_callback)
{
	UINT32 next;
	const UINT32 tail = v_req_tail[id];
#if 0
	if (id > IDE_ID_1) {
		DBG_ERR("invalid id %d\r\n", id);
		return E_SYS;
	}
#endif
	next = (tail+1) % IDE_REQ_POLL_SIZE;
	//printk("%s: next %d\r\n", __func__, next);

	if (next == v_req_front[id]) {
		// queue full
		DBG_ERR("queue full, front %d, tail %d\r\n", (int)v_req_front[id], (int)tail);
		return E_SYS;
	}

	if (p_callback) {
		memcpy(&v_req_pool[id][tail].callback,
			p_callback,
			sizeof(KDRV_CALLBACK_FUNC));
	} else {
		memset(&v_req_pool[id][tail].callback,
                        0,
                        sizeof(KDRV_CALLBACK_FUNC));
	}

	v_req_tail[id] = next;

	return E_OK;
}

/*
	Get head request descriptor from service queue
*/
IDE_REQ_LIST_NODE* ide_platform_get_head(IDE_ID id)
{
	IDE_REQ_LIST_NODE *p_node;

	p_node = &v_req_pool[id][v_req_front[id]];
#if 0
	if (id > IDE_ID_1) {
		DBG_ERR("invalid id %d\r\n", id);
		return NULL;
	}
#endif
	if (v_req_front[id] == v_req_tail[id]) {
		// queue empty
		DBG_ERR("queue empty\r\n");
		return NULL;
	}

	return p_node;

//	memcpy(p_param, &p_node->trig_param, sizeof(KDRV_GRPH_TRIGGER_PARAM));

//	return E_OK;
}

/*
	Delete request descriptor from service queue
*/
ER ide_platform_del_list(IDE_ID id)
{
#if 0
	if (id > IDE_ID_1) {
		DBG_ERR("invalid id %d\r\n", id);
		return E_SYS;
	}
#endif
	if (v_req_front[id] == v_req_tail[id]) {
		DBG_ERR("queue already empty, front %d, tail %d\r\n", (int)v_req_front[id], (int)v_req_tail[id]);
		return E_SYS;
	}

	v_req_front[id] = (v_req_front[id]+1) % IDE_REQ_POLL_SIZE;

	return E_OK;
}


void ide_platform_set_ist_event(IDE_ID id)
{
	/*  Tasklet for bottom half mechanism */
#if defined __KERNEL__
//        tasklet_schedule(v_p_ide_tasklet[id]);
#endif
}

int ide_platform_ist(IDE_ID id, UINT32 event)
{
	idec_isr_bottom(id, event);

	return 0;
}
#if 0
int ide1_monitor_interrupt_task(void)
{
	while(1){
		ide_platform_delay_ms(1000);

		if(ide_log_cnt[IDE_ID_1] < 10){
			idec_parsing_interrupt(IDE_ID_1);
		}
		if (ide_exit_task[IDE_ID_1]) {
			break;
		}
	}
	return 0;
}

int ide2_monitor_interrupt_task(void)
{
	while(1){
		ide_platform_delay_ms(1000);

		if(ide_log_cnt[IDE_ID_2] < 10){
			idec_parsing_interrupt(IDE_ID_2);
		}
		if (ide_exit_task[IDE_ID_2]) {
			break;
		}
	}
	return 0;
}
#endif

#if !(defined __UITRON || defined __ECOS)
#if defined __FREERTOS
static int is_create[2] = {0};
irqreturn_t ide_platform_isr(int irq, void *devid)
{
	ide_isr();
	return IRQ_HANDLED;
}
irqreturn_t ide2_platform_isr(int irq, void *devid)
{
	ide2_isr();
	return IRQ_HANDLED;
}

void ide_platform_create_resource(IDE_ID id)
{
	if (!is_create[id]) {
		OS_CONFIG_FLAG(v_ide_flg_id[id]);
		SEM_CREATE(SEMID_IDE[id], 1);
		vk_spin_lock_init(&v_ide_spin_locks[id]);
		if(id == IDE_ID_1){
			request_irq(INT_ID_IDE, ide_platform_isr, IRQF_TRIGGER_HIGH, "ide", 0);
			THREAD_CREATE(m_ide_tsk[id], ide1_monitor_interrupt_task, NULL, "nvt_ide1_thread");
			THREAD_RESUME(m_ide_tsk[id]);
		}
		else{
			request_irq(INT_ID_IDE2, ide2_platform_isr, IRQF_TRIGGER_HIGH, "ide2", 0);
			THREAD_CREATE(m_ide_tsk[id], ide2_monitor_interrupt_task, NULL, "nvt_ide2_thread");
			THREAD_RESUME(m_ide_tsk[id]);
		}

		is_create[id] = 1;
	}
}
void ide_platform_release_resource(IDE_ID id)
{
	is_create[id] = 0;
	rel_flg(v_ide_flg_id[id]);
	SEM_DESTROY(SEMID_IDE[id]);
	ide_exit_task[id]=1;
	THREAD_DESTROY(m_ide_tsk[id]);
}

#else
#if 0
void ide_platform_create_resource(MODULE_INFO *pmodule_info)
{
	UINT32 i=0;
	for(i=0;i<2;i++){
		if(i==0){
			IOADDR_IDE_REG_BASE = (UINT64)pmodule_info->io_addr[i];
			THREAD_CREATE(m_ide_tsk[i], ide1_monitor_interrupt_task, NULL, "nvt_ide1_thread");
			THREAD_RESUME(m_ide_tsk[i]);
		}
		else{
			IOADDR_IDE2_REG_BASE = (UINT64)pmodule_info->io_addr[i];
			THREAD_CREATE(m_ide_tsk[i], ide2_monitor_interrupt_task, NULL, "nvt_ide2_thread");
			THREAD_RESUME(m_ide_tsk[i]);
		}
		OS_CONFIG_FLAG(v_ide_flg_id[i]);
		SEM_CREATE(SEMID_IDE[i], 1);
		//spin_lock_init(&v_ide_spin_locks[0]);

		ide_clk[i] = pmodule_info->pclk[i];
		ide_if_clk[i] = pmodule_info->ifclk[i];

		v_p_ide_tasklet[i] = &pmodule_info->ide_tasklet[i];
		v_req_front[i] = 0;
		v_req_tail[i] = 0;
	}
	//printk("0x%lx, 0x%lx\n", IOADDR_IDE_REG_BASE, IOADDR_IDE2_REG_BASE);
}

void ide_platform_release_resource(void)
{
	UINT32 i=0;
	for(i=0;i<2;i++){
		rel_flg(v_ide_flg_id[i]);
		SEM_DESTROY(SEMID_IDE[i]);
		ide_exit_task[i]=1;
		THREAD_DESTROY(m_ide_tsk[i]);
	}
}

EXPORT_SYMBOL(ide_platform_set_clksrc);
EXPORT_SYMBOL(ide_platform_set_freq);
EXPORT_SYMBOL(ide_platform_set_iffreq);
EXPORT_SYMBOL(ide_platform_get_freq);
EXPORT_SYMBOL(ide_platform_get_iffreq);
EXPORT_SYMBOL(ide_platform_clk_en);
EXPORT_SYMBOL(ide_platform_clk_dis);
#endif
#endif
#endif


