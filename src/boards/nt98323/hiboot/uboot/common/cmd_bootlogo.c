/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
//#include <asm/arch/soc.h>
#if 0 /* TBD, eFuse */
#include "../board/novatek/common/include/nvt_rdata_sys.h"
#include "../board/novatek/common/include/nvt_rdata_cst.h"
#include "../board/novatek/common/include/nvt_rdata.h"
#endif
#include <asm/arch-nvt-na51068/efuse_protected.h>

#include "nvthdmi/hdmi_if.h"


#define CONFIG_PMU_BASE         0xfe000000
#define CONFIG_LCDC300_BASE		0xFDA00000
#define CONFIG_FTTVE100_BASE    0xFAD00000
#define CONFIG_MAU_BASE         0xFC400000

#define DEMO_BOOT_LOGO_JPEG			// enable jpeg out
#define DEMO_BOOT_LOG_CVBS_ONOFF 0 	//0=no_cvbs 1=have
#define DEMO_BOOT_LOG_HDMI_RES_1024X768 0 //1=1024X768 0=1920X1080

/*---------------------------------------------------------------
 * definition
 *---------------------------------------------------------------
 */
typedef enum {
    VIN_720x480,
    VIN_1024x768,
    VIN_1280x720,
    VIN_1920x1080,
} lcd_vin_t;

/* combination: MSB 16bits: codec, LSB 16bit: output format */
typedef enum {
    OUTPUT_BT1120_720P,
    OUTPUT_BT1120_1080P,
    OUTPUT_BT1120_1024x768,
    /* The following is the VESA */
    OUTPUT_VESA = 0x1000,
    OUTPUT_VGA_1280x720 = OUTPUT_VESA,
    OUTPUT_VGA_1920x1080,
    OUTPUT_VGA_1024x768,
    /* for both RGB and BT1120, LCD300 use */
    OUTPUT_1280x720 = (OUTPUT_BT1120_720P | OUTPUT_VGA_1280x720),
    OUTPUT_1920x1080 = (OUTPUT_BT1120_1080P | OUTPUT_VGA_1920x1080),
    OUTPUT_1024x768 = (OUTPUT_BT1120_1024x768 | OUTPUT_VGA_1024x768),
} lcd_output_t;

typedef enum {
    INPUT_FMT_YUV422 = 0,
    INPUT_FMT_RGB888,
    INPUT_FMT_RGB565,
    INPUT_FMT_ARGB1555,
} input_fmt_t;

typedef enum {
    LCD_ID = 0,
    LCD0_ID = LCD_ID,
    LCD1_ID,
    LCD2_ID
} lcd_idx_t;

/*---------------------------------------------------------------
 * Configurable
 *---------------------------------------------------------------
 */
/* logo base */
#define FRAME_BUFFER_BASE	0x10000000     ///< 0x10000000    configurable, TBD:YUV data address
#define FRAME_CVBS_BUFFER_BASE	0x10800000 ///< 0x10800000    CVBS, TBD        :YUV data address
#define JPEGFILE_BUFFER_BASE    0x10700000          //for hdmi/vga JPEG file
#define JPEGFILE_BUFFER_BASE_CVBS    0x10600000     //for cvbs     JPEG file
#define FRAME_BUFFER_SIZE	(4 << 20)    //4M, configurable

#ifdef DEMO_BOOT_LOGO_JPEG
#if DEMO_BOOT_LOG_HDMI_RES_1024X768
static lcd_output_t g_output[1] = {OUTPUT_1024x768};   //configurable OUTPUT_1920x1080 / OUTPUT_1024x768
static lcd_vin_t    g_vin[1] = {VIN_1024x768};         //configurable VIN_1920x1080 /    VIN_1024x768
#else
static lcd_output_t g_output[1] = {OUTPUT_1920x1080};   //configurable OUTPUT_1920x1080 / OUTPUT_1024x768
static lcd_vin_t    g_vin[1] = {VIN_1920x1080};         //configurable VIN_1920x1080 /    VIN_1024x768
#endif
static input_fmt_t  g_infmt[1] = {INPUT_FMT_YUV422 /* INPUT_FMT_RGB565 */};    //configurable
#else
static lcd_output_t g_output[1] = {OUTPUT_1280x720};   //configurable
static lcd_vin_t    g_vin[1] = {VIN_1024x768};         //configurable
static input_fmt_t  g_infmt[1] = {INPUT_FMT_ARGB1555 /* INPUT_FMT_RGB565 */};    //configurable
#endif

#define LCD_PRI         0   /* 0: High, 1: Middle, 2: Low Priority */   
#define LCD310_PLL      2   /*0: PLL9, 2: PLL11, default PLL9 */
#define PATGEN_ENABLE   0   /* 1 for enable pattern generation */
#define DISPLAY_CH		0	/* 0 for plane0, 1 for plane1 */
#define VERSION			"1.1.3"

/*---------------------------------------------------------------
 * Body
 *---------------------------------------------------------------
 */
#define ioread32        readl
#define iowrite32       writel

static u32 diplay_pll[3] = {297000, 270000, 54000}; /* khz */
#define LCD_IDX_T   lcd_idx_t
#define VGA_MAX_CLK	200000	/* 200Mhz */

#define TVE100_BASE		CONFIG_FTTVE100_BASE
#define MAU_BASE        CONFIG_MAU_BASE


/*************************************************************************************
 *  LCD TIMING MACROs Begin
 *************************************************************************************/
#define CCIR656CONFIG(SYS,INFMT,CLK_INV) ((SYS)|(INFMT)|((CLK_INV)&0x01)<<1)
#define CCIR656_SYS_MASK        0x8
#define CCIR656_SYS_SDTV        0
#define CCIR656_SYS_HDTV        0x8
#define CCIR656_INFMT_PROGRESS  0
#define CCIR656_INFMT_INTERLACE 0x4

#define LCDCONFIG(PanelType,RGBSW,IPWR,IDE,ICK,IHS,IVS) (((IDE)&0x1)<<5| ((ICK)&0x1)<<4|((IHS)&0x1)<<3|((IVS)&0x1)<<2)
#define FFBCONFIG2(ExtClk)        (0)

#define LCD_PANEL_TYPE_6BIT   0
#define LCD_PANEL_TYPE_8BIT   1

#define CCIR656_HSIZE(x) ((x) & 0xFFFF)
#define CCIR656_VSIZE(x) ((x) & 0xFFFF)
#define CCIR656_HACT(x)  ((x) & 0xFFFF)
#define CCIR656_VACT(x)  ((x) & 0xFFFF)
#define CCIR656_BLANKING(x) ((x) & 0xFFFF)

#define LCD_SYNC_WIDTH(x)   ((x) & 0xFF)
#define LCD_PORCH_WIDTH(x)  ((x) & 0xFFFF)
#define LCD_LINE_LENGTH(x)  ((x) & 0xFFFF)

typedef struct ffb_timing_info {
    int otype;
    struct {
        /* LCD */
        struct {
            u32 pixclock;   /* Khz */
            u32 xres;
            u32 yres;
            u32 hsync_len;          //Hor Sync Time
            u32 vsync_len;          //Ver Sync Time
            u32 left_margin;        //H Back Porch
            u32 right_margin;       //H Front Porch
            u32 upper_margin;       //V Back Porch
            u32 lower_margin;       //V Front Porch
            u32 sp_param;           //Serial Panel Parameters
            u32 config;
        } lcd;

        /* CCIR656 */
        struct {
            u32 pixclock;   /* Khz */
            u32 xres;
            u32 yres;
            u32 field0_switch;  //useless
            u32 field1_switch;  //useless
            u32 hblank_len;
            u32 vblank0_len;
            u32 vblank1_len;
            u32 vblank2_len;
            u32 config;
        } ccir656;
    } data;
} ffb_timing_info_t;

static struct ffb_timing_info timing_table[] = {
    {OUTPUT_1280x720,  {{74250, 1280, 720, 40, 5, 220, 110, 20, 5, 0, LCDCONFIG(LCD_PANEL_TYPE_8BIT, 0, 0, 0, 0, 0, 0)},
                        {74250, 1280, 720, 1, 752, 362, 25, 0, 5, CCIR656CONFIG(CCIR656_SYS_HDTV, CCIR656_INFMT_PROGRESS, 1)}}},
    {OUTPUT_1920x1080, {{148500, 1920, 1080, 44, 5, 148, 88, 36, 4, 0, LCDCONFIG(LCD_PANEL_TYPE_8BIT, 0, 0, 0, 0, 0, 0)},
                        {148500, 1920, 1080, 1, 1128, 272, 41, 0, 4, CCIR656CONFIG(CCIR656_SYS_HDTV, CCIR656_INFMT_PROGRESS, 1)}}},
    {OUTPUT_1024x768, {{65000, 1024, 768, 136, 6, 160, 24, 29, 3, 0, LCDCONFIG(LCD_PANEL_TYPE_8BIT, 0, 0, 0, 0, 0, 0)},
                        {65000, 1024, 768, 1, 800, 312, 38, 0, 0, CCIR656CONFIG(CCIR656_SYS_HDTV, CCIR656_INFMT_PROGRESS, 1)}}},
};

/* Input resolution
 */
typedef struct {
    lcd_vin_t   vin;
    int         xres;
    int         yres;
} src_res_t;

static src_res_t resolution[] = {
    {VIN_720x480, 720, 480},
    {VIN_1024x768, 1024, 768},
    {VIN_1280x720, 1280, 720},
    {VIN_1920x1080, 1920, 1080},
};

//#define BT709
#define PMU_BASE        CONFIG_PMU_BASE
#define LCD300_BASE		CONFIG_LCDC300_BASE
#define BITS16(x)       ((x) & 0xFFFF)
/* ***********************************************************************************
 * PMU setting
 * ***********************************************************************************/
#define KHZ_TO_HZ(x)    ((x)*1000)
#define DIFF(x,y)       ((x) > (y) ? (x) - (y) : (y) - (x))

/* ***********************************************************************************
 * LCD300 reg
 * ***********************************************************************************/
enum {
    FUNC_CTRL   = 0x00,
    BG_PARM0    = 0x004,
    BG_PARM1    = 0x008,
    BG_PARM2    = 0x00C,
    CH0_SRC_PARM00 = 0x010,   /* ch0 */
    SRC_PARM02 = 0x018,
    SRC_PARM03 = 0x01C,
    SRC_PARM04 = 0x020,
    SRC_PARM05 = 0x024,
    SRC_PARM06 = 0x028,
    SRC_PARM07 = 0x02C,
    SRC_PARM08 = 0x030,
    SRC_PARM09 = 0x034,
    CH1_SRC_PARM10 = 0x38,
    CSC_PARAM00 = 0xB0, /* ch0 RGB2YUV */
    CSC_PARAM01 = 0xB4,
    CSC_PARAM02 = 0xB8,
    CSC_PARAM03 = 0xBC,
    CSC_PARAM04 = 0xC0,
    CSC_PARAM05 = 0xC4,
    CSC_PARAM06 = 0xC8,
    CSC_PARAM07 = 0xCC,
    CSC_PARAM08 = 0xD0,
    CSC_PARAM09 = 0xD4,
    CSC_PARAM00_CH1 = 0xD8,
    PRELOAD_ADDR0 = 0x170,      /* ch0 */
    PRELOAD_ADDR1 = 0x174,
    PRELOAD_ADDR0_CH1 = 0x178,	/* ch1 */
    PRELOAD_ADDR1_CH1 = 0x17C,
    MIXER_PARM0 = 0x190,
    MIXER_PARM1 = 0x194,
    INTR_MASK = 0x1A8,   /* enable mask */
    INTR_CLR = 0x1AC,   /* write only */
    COLOR_BAR = 0x1B0,
    COLOR_BAR_CTRL = 0x1D0,
    CH0_XY = 0x1E4,    /* xi/yi debug purpose only, 0x1E8, 0x1EC */
    SDI_SCA_BYPASS  = 0x400,
    SDI_SCA_ALG0    = 0x404,
    SDI_SCA_ALG1    = 0x408,
    SDI_SCA_SRC_DIM = 0x40C,
    SDI_SCA_DST_DIM = 0x410,
    SDI_SCA_STEP    = 0x414,
    SDI_SCA_DIV     = 0x418,
    SDI_SCA_AVG_PXL_MSK = 0x41C,
    SDI_SCA_SRC_DIM_CH1 = 0x420,
    SDI_CROP_CH0    = 0x45C,    /* target cropping */
    SDI_CROP_CH1	= 0x464,
    SDO_SCA_BYPASS  = 0x500,
    SDO_SCA_ALG     = 0x504,
    SDO_SCA_SRC_DIM = 0x508,
    SDO_SCA_RES0_DST_DIM= 0x50C,
    SDO_SCA_RES0_STEP   = 0x510,
    SDO_SCA_RES0_DIV    = 0x514,
    SDO_SCA_RES0_AVG_PXL_MSK = 0x518,
    SDO_SCA_RES1_DST_DIM= 0x51C,
    SDO_SCA_RES1_STEP   = 0x520,
    SDO_SCA_RES1_DIV    = 0x524,
    SDO_SCA_RES1_AVG_PXL_MSK = 0x528,
    D1_WRITE_BACK       = 0x52C,
    VTC_POLARITY_CTRL   = 0x800,
    VTC_HOR_TIMING_CTRL1= 0x804,
    VTC_HOR_TIMING_CTRL2= 0x808,
    VTC_HOR_TIMING_CTRL3= 0x80C,
    VTC_VER_TIMING_CTRL4= 0x810,
    VTC_YUV2RGB_CSC     = 0x814,
    VTC_YUV2RGB_ENABLE  = 0x838,
    VTC_TV_SIZE         = 0x840,
    VTC_TV_ACTIVE       = 0x844,
    VTC_TV_VBLK         = 0x848,
    VTC_TV_HBLK         = 0x84C,
    VTC_LBFP_TH         = 0x850,
    VTC_FRAME_RST       = 0x854,
};

#define SCALE_UP_CAP        4   /* don't change */
#define SCALE_DOWN_CAP      8   /* 1/8, don't change */
#define PRELOAD_DMA_SIZE    1024
/*
 * Structure for pinMux
 */
typedef struct
{
    unsigned int  reg_off;    /* register offset from PMU base */
    unsigned int  bits_mask;  /* bits this module covers */
    unsigned int  lock_bits;  /* bits this module locked */
    unsigned int  init_val;   /* initial value */
    unsigned int  init_mask;  /* initial mask */
} pmuReg_t;

/* offset, bits_mask, lock_bits, init_val, init_mask */
static pmuReg_t pmu_reg_tbl[] = {
    /* IPs ARESETN controlled. 0: Reset, 1: Release */
    {0x50, (0x1 << 6) | (0x1 << 2), (0x1 << 6) | (0x1 << 2), (0x1 << 6) | (0x1 << 2), (0x1 << 6) | (0x1 << 2)},
    {0x54, 0x1 << 13, 0x1 << 13, 0x1 << 13, 0x1 << 13},
    /* IPs PRESETN controlled. 0: Reset, 1: Release.  30:hdmi0p/4:gpenc apb/8:lcd310 apb */
    {0x58, (0x1 << 30) | (0x1 << 8) | (0x1 << 4), (0x1 << 8) | (0x1 << 4), (0x1 << 30) | (0x1 << 8) | (0x1 << 4), (0x1 << 30) | (0x1 << 8) | (0x1 << 4)},
	/* gating clock - AClkOff0. 0: Release, 1: Gating */
	{0x60, (0x3 << 14) | (0x1 << 11), (0x3 << 14) | (0x1 << 11), 0x0, (0x3 << 14) | (0x1 << 11)},
	{0x68, 0x1 << 13, 0x1 << 13, 0x0, 0x1 << 13},
	/*  hdmi0m */
	{0x6C, 0x1 << 2, 0x0, 0x0, (0x1 << 2)},
	/* gating clock - PClk0Off. 0: Release, 1: Gating */
	{0x70, (0x7 << 6) | (0x1 << 4), (0x1 << 8) | (0x1 << 4), 0x0, (0x7 << 6) | (0x1 << 4)},
	/*  hdmi0p */
	/*{0x74, 0x1 << 6, 0x0, 0x0, 0x1 << 6},*/
	/* CVBS/ VGA DAC Config */
	/*{0xFC, 0xFF << 8, 0xFF << 8, 0x0, 0x3F << 8},*/
	{0x80, (0xFF << 0), (0xFF << 0), 0x11, (0xFF << 0)},
	/* hdmi pinmux*/
	{0x30080, 0xFFFFFFFF, 0xFFFFF, 0x11111, 0xFFFFF},
	/* LCDC Clock  */
	{0x174, 0xFFFFFFFF, 0xFFFF << 16, 0x0, 0x0},
	/* ADC/DAC HDMI Clock, [17:16]: hdmi0 mclk select. 0: ssp12288, 1: osch1, 2: preclk (MAC_CK), [23:18]: hdmi0 mclk preclk_cntp */
	/*{0x180, 0xFFFF << 16, 0x0, 0x0, 0x0},*/	//TBD
	/* pinmux 0x1a8/0x1ac, BT1120/LCD RGB */
	/*{0x1A8, 0xFFFFFFFF, 0, 0, 0},*/
	/* BT1120 */
	/*{0x1A0, 0xF << 26, 0, 0, 0},*/
	/*{0x1AC, 0xFFFFFFFF, 0, 0, 0},*/
	/* BT1120 */
	/*{0x1B0, 0xFFF << 20, 0, 0, 0},*/
	/* BT1120 */
	/*{0x1B4, 0xFFFFF, 0, 0, 0},*/
	/* VGA/DAC trim value */
	/*{0x1B8, 0xFFFFFFFF, 0x0, 0x03030308, 0xFFFFFFFF},*/
	/* source select */
	// [1:0] HDMI source selection. 00: LCD210, 01: LCD210, 10: LCD310, 11: LCD310_1
	// [9:8]: 3-ch DAC source selection. 00: LCD210, 01: LCD210, 10: LCD310, 11: LCD310_1
	// [13:12]: LCD OUT source selection. 00: LCD210, 01: LCD210, 10: LCD310, 11: LCD310_1
	// [16]: TV OUT source selection. 0: LCD210, 1: LCD310
	/*{0x1E4, 0xFFFFFFFF, 0x0, 0 << 16 | 0x2 << 12 | 0x2 << 8 | 0x2 << 0, 0xFFFFFFFF},*/
	/* driving */
	/*{0x1F0, 0xFF << 8, 0x0, 0x0, 0x0},*/
#ifdef EXT_BT1120_CAP01
    /*{0x1F4, 0xF0000000, 0xF0000000, 0x90000000, 0xF0000000},*/
#else
	/*{0x1F4, 0xF0000000, 0x00000000, 0x00000000, 0x00000000},*/
#endif
	/* LCDPixClk DLYCHAIN */
    /*{0x210, 0xFFFFFF, 0x0, 0x0, 0x0},*/
	/* DISPLAY0_PLL cntp */
	{0x10620, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	{0x10624, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	{0x10628, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	/* DISPLAY1_PLL cntp */
	{0x10660, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	{0x10664, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	{0x10668, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	/* DISPLAY2_PLL cntp */
	{0x106A0, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	{0x106A4, 0xFFFFFFFF, 0x0, 0x0, 0x0},
	{0x106A8, 0xFFFFFFFF, 0x0, 0x0, 0x0},
};

/*************************************************************************************
 *  Local Variables
 *************************************************************************************/
/* lcdc iobase */
static u32 lcd_io_base[1] = {LCD300_BASE};
static u32 lcd_frame_base[1] = {FRAME_BUFFER_BASE};
static int lcd_init_ok[1] = {0};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#define printk	printf
#define LCD_IO_BASE(x)  lcd_io_base[(x)]

/*************************************************************************************
 *  LCD300 IO Read/Write
 *************************************************************************************/
#define ioread32        readl
#define iowrite32       writel

/*************************************************************************************
 *  LCD300 BODY
 *************************************************************************************/
static struct ffb_timing_info	*get_timing(lcd_output_t out_idx)
{
	int	i, idx = out_idx & 0xFFFF;

	for (i = 0; i < ARRAY_SIZE(timing_table); i ++) {
		if (timing_table[i].otype == idx)
			return &timing_table[i];
	}

    for (;;)
	    printk("The timing table: %d doesn't exist! \n", out_idx);

	return NULL;
}

static src_res_t *get_input_resolution(lcd_vin_t vin)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(resolution); i ++) {
        if (resolution[i].vin == vin)
            return &resolution[i];
    }
    for (;;)
	    printk("The input resolution: %d doesn't exist! \n", vin);

	return NULL;
}

#if 0
static u32 ftpmu010_read_reg(unsigned int offset)
{
	return ioread32(PMU_BASE + offset);
}
#endif

static void ftpmu010_write_reg(unsigned int offset, unsigned int val, unsigned int mask)
{
    unsigned int tmp, base = PMU_BASE + offset;

    if (!val && !mask)
        return; /* do nothing */

    if (val & ~mask) {
        for (;;) {
            printf("ftpmu010_write_reg: offset: 0x%x, val: 0x%x mask: 0x%x \n", offset, val, mask);
        } //bug
    }

    tmp = ioread32(base) & ~mask;
    tmp |= (val & mask);
    iowrite32(tmp, base);
}

static int drv_video_init(void)
{
	int	ret = 0;

#if 1 /* TBD, eFuse */
	BOOL     is_found;
    INT32    code;
    UINT32   dac_trim;
	UINT32   reg_temp;

    code = otp_key_manager(3); // 5 => HDMI / 3 => CVBS

    if(code == -33) {
        printf("Read vga trim error\r\n");
    } else {
        dac_trim = 0;
        is_found = extract_trim_valid(code, &dac_trim);
        if(is_found) {
            //printf("Read vga trim = 0x%08x\r\n", dac_trim);
			if ((((dac_trim >> 5) & 0x1F) >= 0x10) && (((dac_trim >> 5) & 0x1F) < 0x15)) {				
				//#define VGA_R_DAC_PD_CTRL_OFS 	0x5C
				//#define VGA_G_DAC_PD_CTRL_OFS 	0x60
				//#define VGA_B_DAC_PD_CTRL_OFS 	0x64
				reg_temp = ioread32(TVE100_BASE + 0x5C);
				reg_temp &= ~(0x1F << 4);
				reg_temp |= (((dac_trim >> 5) & 0x1F)  << 4);
				iowrite32(reg_temp, TVE100_BASE + 0x5C);

				reg_temp = ioread32(TVE100_BASE + 0x60);
				reg_temp &= ~(0x1F << 4);
				reg_temp |= (((dac_trim >> 5) & 0x1F)  << 4);
				iowrite32(reg_temp, TVE100_BASE + 0x60);

				reg_temp = ioread32(TVE100_BASE + 0x64);
				reg_temp &= ~(0x1F << 4);
				reg_temp |= (((dac_trim >> 5) & 0x1F)  << 4);
				iowrite32(reg_temp, TVE100_BASE + 0x64);
			}
			else {
				printf("VGA: efuse trim value over range 0x%x\n", (uint)dac_trim);
				reg_temp = ioread32(TVE100_BASE + 0x5C);
				reg_temp &= ~(0x1F << 4);
				reg_temp |= (0x12  << 4);
				iowrite32(reg_temp, TVE100_BASE + 0x5C);
				reg_temp = ioread32(TVE100_BASE + 0x60);
				reg_temp &= ~(0x1F << 4);
				reg_temp |= (0x12  << 4);
				iowrite32(reg_temp, TVE100_BASE + 0x60);
				reg_temp = ioread32(TVE100_BASE + 0x64);
				reg_temp &= ~(0x1F << 4);
				reg_temp |= (0x12  << 4);
				iowrite32(reg_temp, TVE100_BASE + 0x64);
			}
            
        } else {
            printf("Read vga trim = 0x%x(NULL)\r\n", (uint)dac_trim);
        }
    }
	
#else
	struct buffer_info_t  efuse;

	/* RGB trim value */
	memset(&efuse, 0x0, sizeof(struct buffer_info_t));
	efuse.size = sizeof(dac_val);
	efuse.data = (unsigned char*)&dac_val;
	/* R */
	ret = nvt_read_data(SYSTEM_TYPE, SYS_VDAC_R, 0, 8, &efuse);
	if (!ret) {
		dac_val &= 0xF;
		val_1B8 |= (dac_val << 8);
	} else {
		val_1B8 |= (0x3 << 8);
	}
	/* G */
	ret = nvt_read_data(SYSTEM_TYPE, SYS_VDAC_G, 0, 8, &efuse);
	if (!ret) {
		dac_val &= 0xF;
		val_1B8 |= (dac_val << 16);
	} else {
		val_1B8 |= (0x3 << 16);
	}
	/* B */
	ret = nvt_read_data(SYSTEM_TYPE, SYS_VDAC_B, 0, 8, &efuse);
	if (!ret) {
		dac_val &= 0xF;
		val_1B8 |= (dac_val << 24);
	} else {
		val_1B8 |= (0x3 << 24);
	}

	ftpmu010_write_reg(0x1B8, val_1B8, 0xFFFFFF << 8);
#endif /* eFuse */

    return ret;
}

/* TBD */
/* this function will be called in initialization state or module removed */
static void platform_pmu_clock_gate(int lcd_idx, int on_off)
{
    u32 bit, mask;

    if (on_off == 1) {
    	/* gating clock - AClkOff0. 0: Release, 1: Gating. lcd310a/gpenca */
    	bit = (0x0 << 15) | (0x0 << 11);
    	mask = (0x1 << 15) | (0x1 << 11);
    	ftpmu010_write_reg(0x60, bit, mask);

		/* gating clock - PClk0Off. 0: Release, 1: Gating. lcd310p/gpencp */
    	bit = (0x0 << 8) | (0x0 << 4) | (0x0 << 6);
    	mask = (0x1 << 8) | (0x1 << 4) | (0x1 << 6);
    	ftpmu010_write_reg(0x70, bit, mask);

		/* gating clock - Peripheral Off. 0: Release, 1: Gating. hdmi0m */
		bit = (0x0 << 2);
		mask = (0x1 << 2);
		ftpmu010_write_reg(0x6C, bit, mask);

		/* gating clock - PClk1Off0. 0: Release, 1: Gating. hdmi0p */
		bit = (0x0 << 6);
		mask = (0x1 << 6);
		ftpmu010_write_reg(0x70, bit, mask);
    } else {
    	/* gating clock - AClkOff0. 0: Release, 1: Gating. lcd310a/gpenca */
    	bit = (0x0 << 15) | (0x0 << 11);
    	mask = (0x1 << 15) | (0x1 << 11);
    	ftpmu010_write_reg(0x60, bit, mask);

		/* gating clock - PClk0Off. 0: Release, 1: Gating. lcd310p/gpencp */
    	bit = (0x0 << 8) | (0x0 << 4) | (0x0 << 6);
    	mask = (0x1 << 8) | (0x1 << 4) | (0x1 << 6);
    	ftpmu010_write_reg(0x70, bit, mask);

		/* gating clock - Peripheral Off. 0: Release, 1: Gating. hdmi0m */
		bit = (0x0 << 2);
		mask = (0x1 << 2);
		ftpmu010_write_reg(0x6C, bit, mask);

		/* gating clock - PClk1Off0. 0: Release, 1: Gating.  hdmi0p */
		bit = (0x0 << 6);
		mask = (0x1 << 6);
		ftpmu010_write_reg(0x70, bit, mask);
    }

    return;
}

static int	get_div(u32	pll, u32 want_clk, int near)
{
	int	div;

	if (near)
        pll += (want_clk >> 1);

	div = pll / want_clk;

	return div;
}

static unsigned int pll_read_disp0(void)
{
	unsigned int value = 0, data = 0, tmp = 0, offset = 0x600;

	value = ioread32(PMU_BASE + 0x10000 + offset+ 0x20);
    data = ioread32(PMU_BASE + 0x10000 + offset + 0x24);
    value |= ((data & 0xFF) << 8);
    data = ioread32(PMU_BASE + 0x10000 + offset + 0x28);
    value |= ((data & 0xFF) << 16);

    data = (value * 12) >> 17;
    tmp = data << 17;
    if (tmp < value * 12) { /* rounding issue */
        data ++;
	}

	return data * 1000000;
}

static unsigned int pll_read_disp2(void)
{
	unsigned int value = 0, data = 0, tmp = 0, offset = 0x680;

	value = ioread32(PMU_BASE + 0x10000 + offset+ 0x20);
    data = ioread32(PMU_BASE + 0x10000 + offset + 0x24);
    value |= ((data & 0xFF) << 8);
    data = ioread32(PMU_BASE + 0x10000 + offset + 0x28);
    value |= ((data & 0xFF) << 16);

    data = (value * 12) >> 17;
    tmp = data << 17;
    if (tmp < value * 12) { /* rounding issue */
        data ++;
	}

	return data * 1000000;
}


/* pixclk: Standard VESA clock (kHz), TBD
 */
static unsigned int platform_pmu_calculate_clk(int lcd_idx, u32 pixclk)
{
    int	div = 0, clk_src = 0;
	u32 pll;
	u32 vga_off = 0, val; //TBD

	if (LCD310_PLL == 0) {
		clk_src = 0; /* DISP_0 */
	} else if (LCD310_PLL == 2) {
		clk_src = 2;
	} else {
		printk("LCD300:%d chooses PLL10 not support\n", lcd_idx);
	}
	printk("pixclk %d\n", pixclk);

	if (clk_src == 0) {
		/* DISP_0*/
		pll = KHZ_TO_HZ(diplay_pll[0]);
		if ((diplay_pll[0] % pixclk) == 0) {
			div = (get_div(pll, KHZ_TO_HZ(pixclk), 1) -1) & 0x3F;
			printk("LCD300:%d chooses PLL9:%d with div=%d (drv:%s)\n", lcd_idx, pll, div+1, VERSION);
		} else {
			u32 pclk_mhz, apb_div = 0;
			volatile int i;

			pclk_mhz = pixclk / 100; /* to 10Mhz */
			apb_div = (pclk_mhz * 131072)/(12 * 10);
			ftpmu010_write_reg(0x10620, apb_div & 0xFF, 0xFFFFFFFF);
			ftpmu010_write_reg(0x10624, (apb_div >> 8) & 0xFF, 0xFFFFFFFF);
			ftpmu010_write_reg(0x10628, (apb_div >> 16) & 0xFF, 0xFFFFFFFF);
			for (i = 0; i < 0x10000; i ++) {}	//delay for a while

			pll = pll_read_disp0();
			diplay_pll[0] = pixclk;
			div = (get_div(pll, KHZ_TO_HZ(pixclk), 1) -1) & 0x3F;
			printk("LCD300:%d, change PLL9:%d with div=%d.\n", lcd_idx, pll, div+1);
		}
	} else {
		/* DISP_2 */
		pll = KHZ_TO_HZ(diplay_pll[2]);
		if ((diplay_pll[2] % pixclk) == 0) {
			div = (get_div(pll, KHZ_TO_HZ(pixclk), 1) -1) & 0x3F;
			printk("LCD300:%d chooses PLL11:%d with div=%d (drv:%s)\n", lcd_idx, pll, div+1, VERSION);
		} else {
			u32 pclk_mhz, apb_div = 0;
			volatile int i;

			pclk_mhz = pixclk / 100; /* to 10Mhz */
			apb_div = (pclk_mhz * 131072)/(12 * 10);
			ftpmu010_write_reg(0x106A0, apb_div & 0xFF, 0xFFFFFFFF);
			ftpmu010_write_reg(0x106A4, (apb_div >> 8) & 0xFF, 0xFFFFFFFF);
			ftpmu010_write_reg(0x106A8, (apb_div >> 16) & 0xFF, 0xFFFFFFFF);
			for (i = 0; i < 0x10000; i ++) {}	//delay for a while

			pll = pll_read_disp2();
			diplay_pll[2] = pixclk;
			div = (get_div(pll, KHZ_TO_HZ(pixclk), 1) -1) & 0x3F;
			printk("LCD300:%d, change PLL9:%d with div=%d.\n", lcd_idx, pll, div+1);
		}
	}
	/* [23:18]: lcd300 pixclk_cntp pvalue */
	ftpmu010_write_reg(0x174, (div << 18) | (clk_src << 16), (0x3F << 18) | (0x3 << 16));

#if 1 //TBD
	/* turn off VGA if pixel clock is larger than 200 Mhz */
	//val = (ftpmu010_read_reg(0x1E4) >> 8) & 0x3;
	val = ioread32(TVE100_BASE + 0x68) & 0x3;
	/* 1: LCD300_RGB0, 2:LCD300_RGB1 */
	if (val >= 1) {
		vga_off = (pixclk >= VGA_MAX_CLK) ? 1 : 0;
		//vga_off ? ftpmu010_write_reg(0xFC, 0x3F << 8, 0x3F << 8) : ftpmu010_write_reg(0xFC, 0x0, 0x3F << 8);
		if (vga_off) {
			iowrite32(0x1, TVE100_BASE + 0x58);
			iowrite32(0x121, TVE100_BASE + 0x5C);
			iowrite32(0x121, TVE100_BASE + 0x60);
			iowrite32(0x121, TVE100_BASE + 0x64);
		}
		else {
			iowrite32(0x0, TVE100_BASE + 0x58);
			iowrite32(0x120, TVE100_BASE + 0x5C);
			iowrite32(0x120, TVE100_BASE + 0x60);
			iowrite32(0x120, TVE100_BASE + 0x64);
		}
		
	}
#endif

	return pll / (div + 1);
}

static int lcd300_pinmux_init(LCD_IDX_T lcd_idx, lcd_output_t out_idx)
{
    static int  pmu_init_done[3] = {0, 0, 0};
    struct ffb_timing_info *timing;
    unsigned int i, pmu_tbl_sz = 0;
    pmuReg_t *pmu_tbl_ptr = NULL;
	u32 val = 0;

    if (pmu_init_done[lcd_idx])
        return 0;

    pmu_init_done[lcd_idx] = 1;

    if ((timing = get_timing(out_idx)) == NULL)
        return -1;
    pmu_tbl_ptr = &pmu_reg_tbl[0];
    pmu_tbl_sz = sizeof(pmu_reg_tbl) / sizeof(pmuReg_t);

    for (i = 0; i < pmu_tbl_sz; i ++) {
        ftpmu010_write_reg(pmu_tbl_ptr->reg_off, pmu_tbl_ptr->init_val, pmu_tbl_ptr->init_mask);
        pmu_tbl_ptr ++;
    }		
	
	val = ioread32(TVE100_BASE + 0x68);
	val &= ~0x3;
	val |= 0x1;
	iowrite32(val, TVE100_BASE + 0x68);

    /* turn on gate clock */
    platform_pmu_clock_gate(lcd_idx, 1);
    platform_pmu_calculate_clk(lcd_idx, timing->data.lcd.pixclock);

    return 0;
}

static int lcd300_set_framebase(u32 base, u32 framebase, u32 channel)
{
    u32 tmp;

	if (channel == 0) {
    	iowrite32(framebase, base + CH0_SRC_PARM00); /* CH0 Y base address */
	} else {
		iowrite32(framebase, base + CH1_SRC_PARM10); /* CH1 Y base address */
	}

	tmp = ioread32(base + FUNC_CTRL);
	tmp |= (0x1 << 31);     /* RegUpdate */
	iowrite32(tmp, base + FUNC_CTRL);

    return 0;
}

static void lcd300_control_en(u32 base, int state)
{
    u32		value;

    // clear interrupt status
    iowrite32(0x0, base + INTR_MASK);
    iowrite32(0xFF, base + INTR_CLR);

    value = ioread32(base + VTC_POLARITY_CTRL);
    value &= ~0x1;
    if (state)
        value |= 0x1;
    iowrite32(value, base + VTC_POLARITY_CTRL);
    return;
}

/* set background color and its size */
static void dev_setting_bgplane(u32 base, int bg_hs, int bg_vs, int bg_y, int bg_cb, int bg_cr)
{
    u32 tmp;

    tmp = (bg_vs & 0xFFFF) << 16 | (bg_hs & 0xFFFF);
    iowrite32(tmp, base + BG_PARM0);
    tmp = bg_y & 0xFFF;
    iowrite32(tmp, base + BG_PARM1);
    tmp = (bg_cb & 0xFFF) << 16 | (bg_cr & 0xFFF);
    iowrite32(tmp, base + BG_PARM2);
}

static void dev_setting_pattern_gen(u32 base, input_fmt_t in_fmt, int patgen_ch, int enable)
{
    u32 cbar_rgb[8], dma_ch;
    u32 i, tmp, cbar_type[4] = {0, 0, 0, 0}; /* Vertical color bar */

    if (in_fmt == INPUT_FMT_YUV422) {
        /* YUV422 */
        cbar_rgb[0] = (0xEB << 16) | (0x80 << 8) | 0x80;   /* white */
        cbar_rgb[1] = (0xD2 << 16) | (0x10 << 8) | 0x92;   /* yellow */
        cbar_rgb[2] = (0xAA << 16) | (0xA6 << 8) | 0x10;   /* cyan */
        cbar_rgb[3] = (0x91 << 16) | (0x36 << 8) | 0x22;   /* green */
        cbar_rgb[4] = (0x6A << 16) | (0xCA << 8) | 0xDE;   /* megenta */
        cbar_rgb[5] = (0x51 << 16) | (0x5A << 8) | 0xF0;   /* red */
        cbar_rgb[6] = (0x10 << 16) | (0x80 << 8) | 0x80;   /* black */
        cbar_rgb[7] = (0x29 << 16) | (0xF0 << 8) | 0x6E;   /* blue */
    } else {
        /* RGB */
        cbar_rgb[0] = (0xFF << 16) | (0xFF << 8) | 0xFF;   /* white */
        cbar_rgb[1] = (0xFF << 16) | (0xFF << 8) | 0x00;   /* yellow */
        cbar_rgb[2] = (0x00 << 16) | (0xFF << 8) | 0xFF;   /* cyan */
        cbar_rgb[3] = (0x00 << 16) | (0xFF << 8) | 0x00;   /* green */
        cbar_rgb[4] = (0xFF << 16) | (0x00 << 8) | 0xFF;   /* megenta */
        cbar_rgb[5] = (0xFF << 16) | (0x00 << 8) | 0x00;   /* red */
        cbar_rgb[6] = (0x00 << 16) | (0x00 << 8) | 0x00;   /* black */
        cbar_rgb[7] = (0x00 << 16) | (0x00 << 8) | 0xFF;   /* blue */
    }

	dma_ch = 0;

    for (i = 0; i < 8; i ++)
        iowrite32(cbar_rgb[i], base + COLOR_BAR + i * 4);

    tmp = (dma_ch << 11) | (enable << 10) | (patgen_ch << 8) | (cbar_type[3] << 6) |
          (cbar_type[2] << 4) | (cbar_type[1] << 2) | cbar_type[0];
    iowrite32(tmp, base + COLOR_BAR_CTRL);
}

/**
 *@brief get image color format from hardware register format
 */
static int dev_get_color_format(int input_fmt, int *bits_per_pixel)
{
    int tmp;

    switch (input_fmt) {
      case INPUT_FMT_YUV422:
        tmp = 8;
        *bits_per_pixel = 16;
        break;
      case INPUT_FMT_RGB888:
        tmp = 2;
        *bits_per_pixel = 32;
        break;
      case INPUT_FMT_RGB565:
        *bits_per_pixel = 16;
        tmp = 0;
        break;
      case INPUT_FMT_ARGB1555:
        *bits_per_pixel = 16;
        tmp = 1;
        break;
      default:
        for (;;) printf("%s, error format: %d \n", __func__, input_fmt);
        break;
    }
    return tmp;
}

static void dev_set_preload_fifo(u32 base, u32 plane)
{
    u32 tmp, preload_y_saddr = 0, preload_y_eaddr = (PRELOAD_DMA_SIZE >> 1) - 1;
    u32 preload_cbr_saddr = preload_y_eaddr+1, preload_cbr_eaddr = PRELOAD_DMA_SIZE-1;
	int ofs;

	ofs = plane ? (PRELOAD_ADDR0_CH1 - PRELOAD_ADDR0) : 0;
    tmp = preload_y_saddr << 16 | preload_y_eaddr;
    iowrite32(tmp, base + PRELOAD_ADDR0 + ofs);
    tmp = preload_cbr_saddr << 16 | preload_cbr_eaddr;
    iowrite32(tmp, base + PRELOAD_ADDR1 + ofs);
}

/* rgb2yuv or yuv2rgb */
typedef struct {
    u8 csc_input_clipping;
    u8 csc_output_clipping;
    short csc_in_offset[3];
    short csc_out_offset[3];
    short csc_coef[9];
} rgb2yuv_param_t;

/* Load default csc parameter
 */
static void dev_channel_load_defcsc(u32 base, rgb2yuv_param_t *rgb2yuv, u32 channel)
{
    u32  tmp, i, ofs;

#ifdef BT709
    /* only active for RGB to YUV */
    rgb2yuv->csc_input_clipping = 0;
    rgb2yuv->csc_output_clipping = 0;
    rgb2yuv->csc_in_offset[0] = 0;
    rgb2yuv->csc_in_offset[1] = 0;
    rgb2yuv->csc_in_offset[2] = 0;
    rgb2yuv->csc_out_offset[0] = 256;
    rgb2yuv->csc_out_offset[1] = 2048;
    rgb2yuv->csc_out_offset[2] = 2048;
    rgb2yuv->csc_coef[0] = 1258;
    rgb2yuv->csc_coef[1] = 127;
    rgb2yuv->csc_coef[2] = 374;
    rgb2yuv->csc_coef[3] = -693;
    rgb2yuv->csc_coef[4] = 899;
    rgb2yuv->csc_coef[5] = -206;
    rgb2yuv->csc_coef[6] = -817;
    rgb2yuv->csc_coef[7] = -83;
    rgb2yuv->csc_coef[8] = 899;
#else
    /* only active for RGB to YUV */
    rgb2yuv->csc_input_clipping = 0;
    rgb2yuv->csc_output_clipping = 0;
    rgb2yuv->csc_in_offset[0] = 0;
    rgb2yuv->csc_in_offset[1] = 0;
    rgb2yuv->csc_in_offset[2] = 0;
    rgb2yuv->csc_out_offset[0] = 256;
    rgb2yuv->csc_out_offset[1] = 2048;
    rgb2yuv->csc_out_offset[2] = 2048;
    rgb2yuv->csc_coef[0] = 1032;
    rgb2yuv->csc_coef[1] = 201;
    rgb2yuv->csc_coef[2] = 526;
    rgb2yuv->csc_coef[3] = -596;
    rgb2yuv->csc_coef[4] = 899;
    rgb2yuv->csc_coef[5] = -303;
    rgb2yuv->csc_coef[6] = -754;
    rgb2yuv->csc_coef[7] = -145;
    rgb2yuv->csc_coef[8] = 899;
#endif

	for (i = 0; i < channel + 1; i ++) {
		if (i == 0)
			ofs = 0;
		else
			ofs = CSC_PARAM00_CH1 - CSC_PARAM00;

	    tmp = BITS16(rgb2yuv->csc_in_offset[1]) << 16 | BITS16(rgb2yuv->csc_in_offset[0]);
	    iowrite32(tmp, base + CSC_PARAM00 + ofs);
	    tmp = BITS16(rgb2yuv->csc_in_offset[2]);
	    iowrite32(tmp, base + CSC_PARAM01 + ofs);
	    tmp = BITS16(rgb2yuv->csc_out_offset[1]) << 16 | BITS16(rgb2yuv->csc_out_offset[0]);
	    iowrite32(tmp, base + CSC_PARAM02 + ofs);
	    tmp = BITS16(rgb2yuv->csc_out_offset[2]);
	    iowrite32(tmp, base + CSC_PARAM03 + ofs);
	    tmp = BITS16(rgb2yuv->csc_coef[1]) << 16 | BITS16(rgb2yuv->csc_coef[0]);
	    iowrite32(tmp, base + CSC_PARAM04 + ofs);
	    tmp = BITS16(rgb2yuv->csc_coef[3]) << 16 | BITS16(rgb2yuv->csc_coef[2]);
	    iowrite32(tmp, base + CSC_PARAM05 + ofs);
	    tmp = BITS16(rgb2yuv->csc_coef[5]) << 16 | BITS16(rgb2yuv->csc_coef[4]);
	    iowrite32(tmp, base + CSC_PARAM06 + ofs);
	    tmp = BITS16(rgb2yuv->csc_coef[7]) << 16 | BITS16(rgb2yuv->csc_coef[6]);
	    iowrite32(tmp, base + CSC_PARAM07 + ofs);
	    tmp = BITS16(rgb2yuv->csc_coef[8]);
	    iowrite32(tmp, base + CSC_PARAM08 + ofs);

	    tmp = ioread32(base + CSC_PARAM09 + ofs);
	    tmp &= ~0x7;    /* csc_output_offset2 ..... default is disabled */
	    tmp |= rgb2yuv->csc_output_clipping << 1 | rgb2yuv->csc_input_clipping;
	    iowrite32(tmp, base + CSC_PARAM09 + ofs);
	}
}

static int dev_channel_cfg(u32 base, int input_fmt, int ch_hs, int ch_vs, int channel)
{
    int dma_ch = channel, cbr_dma_ch = channel, mode = 0; /* RAW */
    int ch_en, rld_len = 1, rld_cnt = 4, trans444_mode, ch_fmt, fmt_swap;
    int sc_hs, sc_vs, roi_x, roi_y, dma_max_burst, bits_per_pixel;
    int sc_diff, ch_y_xy, ch_cbr_xy, ch_y_offset, ch_cbr_offset;
    int g_alpha, alpha0, alpha1, ret = 0, sc_N = 0;
    rgb2yuv_param_t rgb2yuv;
    u32 tmp, ofs, i;

    rld_cnt = 4;   /* fixed in 7 bits */
    ch_en = 1;
    trans444_mode = 0; /* 0: duplication mode, 1: interpolation mode */
    ch_fmt = dev_get_color_format(input_fmt, &bits_per_pixel);
    fmt_swap = 0;
    roi_x = roi_y = 0;
    sc_hs = ch_hs;
    sc_vs = ch_vs;
    dma_max_burst = 32;

    sc_diff = ch_hs - sc_hs;
    ch_y_xy = ((ch_hs * roi_y + roi_x) * bits_per_pixel) >> 3; /* to bytes */
    ch_cbr_xy = 0;
    ch_y_offset = ((sc_diff + ch_hs * sc_N) * bits_per_pixel) >> 3; /* to bytes */
    ch_cbr_offset = 0;
    if (ch_y_xy & 0xF) {
        printf("%s, x:%d, y:%d, ch_y_xy: %d must be 128 bits alignment! \n", __func__, roi_x, roi_y, ch_y_xy);
        ch_y_xy = 0;
    }
    g_alpha = 255;
    alpha0 = 0;
    alpha1 = 255;

    for (i = 0; i < channel + 1; i ++) {
	    /* update to hardware */
	    ofs = (i == 1) ? (CH1_SRC_PARM10 - CH0_SRC_PARM00) : 0;
		ch_en = (channel == i) ? 1 : 0;

	    /* 0x18: hs/vs */
	    tmp = ch_vs << 16 | ch_hs;
	    iowrite32(tmp, base + SRC_PARM02 + ofs);
	    /* 0x1C: chn_y_xy */
	    iowrite32(ch_y_xy, base + SRC_PARM03 + ofs);
	    /* 0x20: ch_cbr_xy */
	    iowrite32(ch_cbr_xy, base + SRC_PARM04 + ofs);
	    /* 0x24: ch_y_offset */
	    iowrite32(ch_y_offset, base + SRC_PARM05 + ofs);
	    /* 0x28: ch_cbr_offset */
	    iowrite32(ch_cbr_offset, base + SRC_PARM06 + ofs);
	    /* 0x2C: sc_hs/sc_vs */
	    tmp = sc_vs << 16 | sc_hs;
	    iowrite32(tmp, base + SRC_PARM07 + ofs);
	    /* 0x30 */
	    tmp = (alpha1 & 0xFF) << 24 | (alpha0 & 0xFF) << 16 | (g_alpha & 0xFF) << 8;
	    iowrite32(tmp, base + SRC_PARM08 + ofs);
	    /* 0x34 */
	    tmp = ch_en << 31 | fmt_swap << 30 | trans444_mode << 23 | rld_cnt << 20 |
	          rld_len << 18 | mode << 16 | cbr_dma_ch << 13 | dma_ch << 12 |
	          dma_max_burst << 4 | ch_fmt;
	    iowrite32(tmp, base + SRC_PARM09 + ofs);
	}

    /* set the preload fifo  */
    dev_set_preload_fifo(base, channel);
    /* load default csc value */
    dev_channel_load_defcsc(base, &rgb2yuv, channel);

    /* debug purpose xi/yi */
    tmp = roi_x << 16 | roi_y;
    iowrite32(tmp, base + CH0_XY);
    return ret;
}

static int get_hsca_divisor(int src, int target)
{
    int div, value = 512;

    if (src <= target) { /* zoom in, max capability is 4 */
        div = target / src;
        if (src * SCALE_UP_CAP < target) {
            printk("scaling up is over range %d \n", SCALE_UP_CAP);
            return -1;
        }

        return 4096;
    }

    div = src / target;

    /* zoom out, src > target */
    switch (div) {
      case 1:
        value = 2048;
        break;
      case 2:
        value = (src == 2*target) ? 2048 : 1365;
        break;
      case 3:
        value = (src == 3*target) ? 1365 : 1024;
        break;
      case 4:
        value = (src == 4*target) ? 1024 : 819;
        break;
      case 5:
        value = (src == 5*target) ? 819 : 683;
        break;
      case 6:
        value = (src == 6*target) ? 683 : 585;
        break;
      case 7:
        value = (src == 7*target) ? 585 : 512;
        break;
      default:
        if (target * SCALE_DOWN_CAP < src)
            value = -1;
        break;
    }

    if (value < 0)
        printk("%s, scaling down is over range %d \n", __func__, SCALE_DOWN_CAP);

    return value;
}

static int get_vsca_divisor(int src, int target)
{
    int div, value;

    if (src <= target) { /* zoom in */
        div = target / src;
        if (src * SCALE_UP_CAP < target) {
            printk("scaling up is over range %d \n", SCALE_UP_CAP);
            return -1;
        }
        return 4096;
    }

    div = src / target;

    /* zoom out, src > target */
    switch (div) {
      case 1:
        value = 2048;
        break;
      case 2:
        value = (src == 2*target) ? 2048 : 1365;
        break;
      default: /* 3 ~ 8 */
        value = (src == 3*target) ? 1365 : 1024;
        break;
    }
    if ((div >= 8 && (src % target)) || (div > SCALE_DOWN_CAP))
        value = -1;

    if (value < 0)
        printk("%s, scaling down is over range %d \n", __func__, SCALE_DOWN_CAP);

    return value;
}

/* Horizontal average pixel mask */
static int get_havg_pxl_msk(int src, int target)
{
    int div, value = 255;

    if (src <= target) { /* zoom in */
        div = target / src;
        if (src * SCALE_UP_CAP < target) {
            printk("scaling up is over range %d \n", SCALE_UP_CAP);
            return -1;
        }
        return 8;
    }

    div = src / target;
    /* zoom out, src > target */
    switch (div) {
      case 1:
        value = 24;
        break;
      case 2:
        value = (src == 2*target) ? 24 : 56;
        break;
      case 3:
        value = (src == 3*target) ? 28 : 60;
        break;
      case 4:
        value = (src == 4*target) ? 60 : 124;
        break;
      case 5:
        value = (src == 5*target) ? 62 : 126;
        break;
      case 6:
        value = (src == 6*target) ? 126 : 254;
        break;
      case 7:
        value = (src == 7*target) ? 127 : 255;
        break;
    }
    if ((div >= 8 && (src % target)) || (div > SCALE_DOWN_CAP))
        value = -1;

    if (value < 0)
        printk("%s, scaling down is over range %d \n", __func__, SCALE_DOWN_CAP);

    return value;
}

/* Vertical average pixel mask */
static int get_vavg_pxl_msk(int src, int target)
{
    int div, value;

    if (src <= target) { /* zoom in */
        div = target / src;
        if (src * SCALE_UP_CAP < target) {
            printk("scaling up is over range %d \n", SCALE_UP_CAP);
            return -1;
        }
        return 2;
    }

    div = src / target;

    /* zoom out */
    switch (div) {
      case 1:
        value = 6;
        break;
      case 2:
        value = (src == 2*target) ? 6 : 14;
        break;
      default: /* 3 ~ 8 */
        value = (src == 3*target) ? 7 : 15;
        break;
    }
    if ((div >= 8 && (src % target)) || (div > SCALE_DOWN_CAP))
        value = -1;

    if (value < 0)
        printk("%s, scaling down is over range %d \n", __func__, SCALE_DOWN_CAP);

    return value;
}

static int dev_input_scaler(u32 base, int src_width, int src_height, int dst_width, int dst_height, int channel)
{
    int bypass, sca_map_sel, y_luma_smo_en, y_luma_mean_en, x_luma_algo_en, y_chroma_algo_en, x_chroma_algo_en;
    int hstep, vstep, crop_en, crop_x_st, crop_width, crop_y_st, crop_height, y_luma_wadi_en;
    int hsca_divisor, vsca_divisor, havg_pxl_msk, vavg_pxl_msk, kval, ret = 0;
    u32 tmp, i, ofs, ofs2, bits = (0x1 << channel);

    bypass = 0x3; /* channel 0/1 */
    sca_map_sel = 0; /* 0: without 0.5 pixel distance shift */
    y_luma_smo_en = 0; /* 0: judge by H/W algorithm, 2'b10: force disable, 2'b11: force enable */
    y_luma_mean_en = 0;
    x_luma_algo_en = 0;
    y_chroma_algo_en = 0;
    x_chroma_algo_en = 0;
    y_luma_wadi_en = 0;
    hstep = vstep = 1024;
    crop_en = 0;   /* We don't use target cropping after DMA */
    crop_x_st = 0;
    crop_width = src_width;
    crop_y_st = 0;
    crop_height = src_height;

    /* only support source cropping */
    if ((src_width != dst_width) || (src_height != dst_height)) {
        bypass &= ~bits;
        hstep = (src_width * 1024) / dst_width;
        vstep = (src_height * 1024) / dst_height;
    }
    hsca_divisor = get_hsca_divisor(src_width, dst_width);
    vsca_divisor = get_vsca_divisor(src_height, dst_height);
    havg_pxl_msk = get_havg_pxl_msk(src_width, dst_width);
    vavg_pxl_msk = get_vavg_pxl_msk(src_height, dst_height);
    kval = 768;

    if (hsca_divisor < 0 || vsca_divisor < 0 || havg_pxl_msk < 0 || vavg_pxl_msk < 0) {
        printk("%s, error resolution %d %d %d %d! \n!", __func__, src_width, src_height, dst_width, dst_height);
        ret = -1;
    }

    /* luma_smo .... for 4 channels */
    for (i = 0; i < 2; i ++) {
        tmp = x_chroma_algo_en << 26 | y_chroma_algo_en << 24 |
              x_luma_algo_en << 22 | y_luma_mean_en << 20 |
              y_luma_wadi_en << 18 | y_luma_smo_en << 16 |
              x_chroma_algo_en << 10 | y_chroma_algo_en << 8 |
              x_luma_algo_en << 6 | y_luma_mean_en << 4 |
              y_luma_wadi_en << 2 | y_luma_smo_en;
        iowrite32(tmp, base + SDI_SCA_ALG0 + i * 4);
    }

    tmp = (sca_map_sel << 16) | (crop_en << 4) | bypass;
    iowrite32(tmp, base + SDI_SCA_BYPASS);

    for (i = 0; i < channel + 1; i ++) {
    	if (i == 1) {
	    	ofs = channel ? (SDI_SCA_SRC_DIM_CH1 - SDI_SCA_SRC_DIM) : 0;
			ofs2 = channel ? (SDI_CROP_CH1 - SDI_CROP_CH0) : 0;
		} else {
			ofs = ofs2 = 0;
		}

	    tmp = src_height << 16 | src_width;
	    iowrite32(tmp, base + SDI_SCA_SRC_DIM + ofs);
	    tmp = dst_height << 16 | dst_width;
	    iowrite32(tmp, base + SDI_SCA_DST_DIM + ofs);
	    /* hstep/vstep */
	    tmp = vstep << 16 | hstep;
	    iowrite32(tmp, base + SDI_SCA_STEP + ofs);
	    /* hsca_divisor/vsca_divisor */
	    tmp = vsca_divisor << 16 | hsca_divisor;
	    iowrite32(tmp, base + SDI_SCA_DIV + ofs);
	    /* avg_pxl_msk */
	    tmp = kval << 16 | vavg_pxl_msk << 8 | havg_pxl_msk;
	    iowrite32(tmp, base + SDI_SCA_AVG_PXL_MSK + ofs);
	    /* target cropping */
	    tmp = crop_width << 16 | crop_x_st;
	    iowrite32(tmp, base + SDI_CROP_CH0 + ofs2);
	    tmp = crop_height << 16 | crop_y_st;
	    iowrite32(tmp, base + SDI_CROP_CH0 + ofs2 + 0x4);
	}

    return ret;
}

static void dev_setting_output_yuv2rgb(u32 base, rgb2yuv_param_t *yuv2rgb)
{
    u32 tmp;

#ifdef BT709
    yuv2rgb->csc_input_clipping = 1;
    yuv2rgb->csc_output_clipping = 0;
    yuv2rgb->csc_in_offset[0] = -256;
    yuv2rgb->csc_in_offset[1] = -2048;
    yuv2rgb->csc_in_offset[2] = -2048;
    yuv2rgb->csc_out_offset[0] = 0;
    yuv2rgb->csc_out_offset[1] = 0;
    yuv2rgb->csc_out_offset[2] = 0;
    yuv2rgb->csc_coef[0] = 2384;
    yuv2rgb->csc_coef[1] = -436;
    yuv2rgb->csc_coef[2] = -1094;
    yuv2rgb->csc_coef[3] = 2384;
    yuv2rgb->csc_coef[4] = 4332;
    yuv2rgb->csc_coef[5] = 0;
    yuv2rgb->csc_coef[6] = 2384;
    yuv2rgb->csc_coef[7] = 0;
    yuv2rgb->csc_coef[8] = 3672;
#else
    yuv2rgb->csc_input_clipping = 1;
    yuv2rgb->csc_output_clipping = 0;
    yuv2rgb->csc_in_offset[0] = -256;
    yuv2rgb->csc_in_offset[1] = -2048;
    yuv2rgb->csc_in_offset[2] = -2048;
    yuv2rgb->csc_out_offset[0] = 0;
    yuv2rgb->csc_out_offset[1] = 0;
    yuv2rgb->csc_out_offset[2] = 0;
    yuv2rgb->csc_coef[0] = 2384;
    yuv2rgb->csc_coef[1] = -803;
    yuv2rgb->csc_coef[2] = -1665;
    yuv2rgb->csc_coef[3] = 2384;
    yuv2rgb->csc_coef[4] = 4131;
    yuv2rgb->csc_coef[5] = 0;
    yuv2rgb->csc_coef[6] = 2384;
    yuv2rgb->csc_coef[7] = 0;
    yuv2rgb->csc_coef[8] = 3269;
#endif

    tmp = BITS16(yuv2rgb->csc_in_offset[1]) << 16 | BITS16(yuv2rgb->csc_in_offset[0]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC);
    tmp = BITS16(yuv2rgb->csc_in_offset[2]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0x4);
    tmp = BITS16(yuv2rgb->csc_out_offset[1]) << 16 | BITS16(yuv2rgb->csc_out_offset[0]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0x8);
    tmp = BITS16(yuv2rgb->csc_out_offset[2]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0xC);
    tmp = BITS16(yuv2rgb->csc_coef[1]) << 16 | BITS16(yuv2rgb->csc_coef[0]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0x10);
    tmp = BITS16(yuv2rgb->csc_coef[3]) << 16 | BITS16(yuv2rgb->csc_coef[2]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0x14);
    tmp = BITS16(yuv2rgb->csc_coef[5]) << 16 | BITS16(yuv2rgb->csc_coef[4]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0x18);
    tmp = BITS16(yuv2rgb->csc_coef[7]) << 16 | BITS16(yuv2rgb->csc_coef[6]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0x1C);
    tmp = BITS16(yuv2rgb->csc_coef[8]);
    iowrite32(tmp, base + VTC_YUV2RGB_CSC + 0x20);
    /* csc clipping */
    tmp = yuv2rgb->csc_output_clipping << 1 | yuv2rgb->csc_input_clipping;
    iowrite32(tmp, base + VTC_YUV2RGB_ENABLE);
}

static int dev_output_scaler0(u32 base, int src_width, int src_height, int dst_width, int dst_height)
{
    int sdo_en, bypass, sca_map_sel, y_luma_smo_en, y_luma_mean_en, x_luma_algo_en, y_chroma_algo_en, x_chroma_algo_en, y_luma_wadi_en;
    int hstep, vstep, hsca_divisor, vsca_divisor, havg_pxl_msk, vavg_pxl_msk, kval;
    int ret = 0;
    u32 tmp;

    sdo_en = 1; /* sdo0 only */
    bypass = 0x3; /* sdo0, 1 */
    sca_map_sel = 0;   /* 0: without 0.5 pixel distance shift */
    y_luma_smo_en = 0; /* 0: judge by H/W algorithm, 2'b10: force disable, 2'b11: force enable */
    y_luma_mean_en = 0;
    x_luma_algo_en = 0;
    y_chroma_algo_en = 0;
    x_chroma_algo_en = 0;
    y_luma_wadi_en = 0;
    hstep = vstep = 1024;
    if ((src_width != dst_width) || (src_height != dst_height)) {
        bypass = 0;
        hstep = (1024 * src_width) / dst_width;
        vstep = (1024 * src_height) / dst_height;
    }

    hsca_divisor = get_hsca_divisor(src_width, dst_width);
    vsca_divisor = get_vsca_divisor(src_height, dst_height);
    havg_pxl_msk = get_havg_pxl_msk(src_width, dst_width);
    vavg_pxl_msk = get_vavg_pxl_msk(src_height, dst_height);
    kval = 768;
    if (hsca_divisor < 0 || vsca_divisor < 0 || havg_pxl_msk < 0 || vavg_pxl_msk < 0) {
        printk("%s, error resolution %d %d %d %d! \n!", __func__, src_width, src_height, dst_width, dst_height);
        ret = -1;
        goto exit;
    }
    /* d1 write back */
    iowrite32(0, base + D1_WRITE_BACK);
    iowrite32(0, base + D1_WRITE_BACK + 4);
    iowrite32(0, base + D1_WRITE_BACK + 8);

    tmp = x_chroma_algo_en << 26 | y_chroma_algo_en << 24 |
          x_luma_algo_en << 22 | y_luma_mean_en << 20 |
          y_luma_wadi_en << 18 | y_luma_smo_en << 16 |
          x_chroma_algo_en << 10 | y_chroma_algo_en << 8 |
          x_luma_algo_en << 6 | y_luma_mean_en << 4 |
          y_luma_wadi_en << 2 | y_luma_smo_en;
    iowrite32(tmp, base + SDO_SCA_ALG);

    /* Source scaler width/height */
    tmp = src_height << 16 | src_width;
    iowrite32(tmp, base + SDO_SCA_SRC_DIM);

    tmp = (sca_map_sel << 16) | (sdo_en << 8) | bypass;
    iowrite32(tmp, base + SDO_SCA_BYPASS);

    /* dst width/height */
    tmp = dst_height << 16 | dst_width;
    iowrite32(tmp, base + SDO_SCA_RES0_DST_DIM);
    tmp = vstep << 16 | hstep;
    iowrite32(tmp, base + SDO_SCA_RES0_STEP);
    tmp = vsca_divisor << 16 | hsca_divisor;
    iowrite32(tmp, base + SDO_SCA_RES0_DIV);
    tmp = kval << 16 | vavg_pxl_msk << 8 | havg_pxl_msk;
    iowrite32(tmp, base + SDO_SCA_RES0_AVG_PXL_MSK);

exit:
    return ret;
}

static void dev_load_mixer(u32 base)
{
    int layer_id[4] = {0, 1, 2, 3}; /* index is layer id, the content is plane_id */
    u32 tmp;

    tmp = (layer_id[3] << 6) | (layer_id[2] << 4) | (layer_id[1] << 2) | layer_id[0];
    iowrite32(tmp, base + MIXER_PARM0);
    /* for layer0/1 */
    iowrite32(0, base + MIXER_PARM1);
    iowrite32(0, base + MIXER_PARM1+4);
}

/* The main is RGB out, but we need to construct TV timing due to share one timing generation
 */
static int __dev_construct_tv_timing(u32 base, struct ffb_timing_info *timing)
{
    u32 h_cycle, v_cycle, config, tv_hact, tv_hblk, tv_hsize, tv_vsize, tv_vact, tv_vblk0, tv_vblk1, tv_mode, tmp;

    config = timing->data.ccir656.config;

    /* TV
     */
    h_cycle = timing->data.ccir656.xres + timing->data.ccir656.hblank_len;
    v_cycle = timing->data.ccir656.yres + timing->data.ccir656.vblank0_len +
              timing->data.ccir656.vblank1_len + timing->data.ccir656.vblank2_len;

    if ((config & CCIR656_SYS_MASK) == CCIR656_SYS_HDTV) {

        /* HDTV */
        tv_mode = 1; /* 0: stdv mode, 1: hdtv mode */
        tv_hact = CCIR656_HACT(timing->data.ccir656.xres);
        tv_hblk = CCIR656_BLANKING(timing->data.ccir656.hblank_len);
        tv_hsize = CCIR656_HSIZE(h_cycle + 8);   /* 8: SAV/EAV */
    } else {
        int blanking_cycle = CCIR656_BLANKING((timing->data.ccir656.hblank_len + 8) << 1);

        /* SDTV */
        /* concept: the timing table comes from BT1120. When we would like to transfer to BT656, the BT1120 blanking
         * time must be equal to BT656. Thus we get real blanking time in BT656 ==> (hblanking+8)*2 - 8
         */
        tv_mode = 0; /* 0: stdv mode, 1: hdtv mode */
        tv_hact = CCIR656_HACT(timing->data.ccir656.xres << 1);
        tv_hblk = blanking_cycle - 8 /* SAVE/EAV */;
        /* bt1120 Htotal = h_cycle + 8 */
        h_cycle += 8; /* 8: SAV/EAV */
        tv_hsize = CCIR656_HSIZE(h_cycle << 1);
    }

    tv_vsize = CCIR656_VSIZE(v_cycle);
    tv_vact = CCIR656_VACT(timing->data.ccir656.yres);
    tv_vblk0 = CCIR656_BLANKING(timing->data.ccir656.vblank0_len);
    tv_vblk1 = CCIR656_BLANKING(timing->data.ccir656.vblank1_len + timing->data.ccir656.vblank2_len);

    tmp = tv_vsize << 16 | tv_hsize;
    iowrite32(tmp, base + VTC_TV_SIZE);
    tmp = tv_vact << 16 | tv_hact;
    iowrite32(tmp, base + VTC_TV_ACTIVE);
    tmp = tv_vblk1 << 16 | tv_vblk0;
    iowrite32(tmp, base + VTC_TV_VBLK);
    tmp = tv_hblk;
    iowrite32(tmp, base + VTC_TV_HBLK);

    return tv_mode;
}

/**
 *@brief Translate device timing from ffb_timing_info.
 */
static int translate_timing(u32 base, struct ffb_timing_info *timing)
{
    u32 config, polarity_ctrl_parm0, hor_timing_ctrl_parm1, hor_timing_ctrl_parm2, ver_timing_ctrl_parm3, ver_timing_ctrl_parm4, v_frame_rst;
    int twin_LR = 0, tv_mode;

    config = timing->data.lcd.config;
    polarity_ctrl_parm0 = (0x1 << 19) | (twin_LR & 0x1) << 17 | config | 0x2;
    hor_timing_ctrl_parm1 = LCD_PORCH_WIDTH(timing->data.lcd.left_margin) << 16 |
                                     LCD_PORCH_WIDTH(timing->data.lcd.right_margin);
    hor_timing_ctrl_parm2 = LCD_SYNC_WIDTH(timing->data.lcd.hsync_len) << 16 | timing->data.lcd.xres;
    ver_timing_ctrl_parm3 = LCD_PORCH_WIDTH(timing->data.lcd.upper_margin) << 16 |
                                     LCD_PORCH_WIDTH(timing->data.lcd.lower_margin);
    ver_timing_ctrl_parm4 = LCD_SYNC_WIDTH(timing->data.lcd.vsync_len) << 16 |
                                     LCD_LINE_LENGTH(timing->data.lcd.yres);
    v_frame_rst = 0;

    /* VTC LCD */
    iowrite32(hor_timing_ctrl_parm1, base + VTC_HOR_TIMING_CTRL1);
    iowrite32(hor_timing_ctrl_parm2, base + VTC_HOR_TIMING_CTRL2);
    iowrite32(ver_timing_ctrl_parm3, base + VTC_HOR_TIMING_CTRL3);
    iowrite32(ver_timing_ctrl_parm4, base + VTC_VER_TIMING_CTRL4);
    iowrite32(v_frame_rst, base +  VTC_FRAME_RST);

    /* TV timing */
    tv_mode = __dev_construct_tv_timing(base, timing);

    /* process TV */
    polarity_ctrl_parm0 |= (0x1 << 7); /* cken for both TV and LCD */
    /* TV mode */
    polarity_ctrl_parm0 &= ~(0x1 << 16);
    polarity_ctrl_parm0 |= (tv_mode << 16);
    iowrite32(polarity_ctrl_parm0, base + VTC_POLARITY_CTRL);

    return 0;
}

static int dev_switch_output(u32 base, struct ffb_timing_info *timing)
{
    return translate_timing(base, timing);
}

/*
 * lcd_id: 0/1/2
 * vin: input resolution. Currently it is ingored.
 * lcd_output: combination of lcd_output_t
 */
static int lcd300_init(lcd_idx_t lcd_id, lcd_vin_t vin, u32 lcd_output, input_fmt_t in_fmt, u32 frame_Base)
{
    struct ffb_timing_info *timing;
    rgb2yuv_param_t rgb2yuv;
    src_res_t   *src_res;
	u32	ip_io_base = LCD_IO_BASE(lcd_id);
	int channel = DISPLAY_CH, src_xres, src_yres, dst_xres, dst_yres;

    if (lcd_id >= ARRAY_SIZE(lcd_io_base))
       return -1;

    if (lcd_init_ok[lcd_id] == 1)
        return 0;

    /* output timing */
    timing = get_timing(lcd_output);
    lcd_init_ok[lcd_id] = 1;
    dst_xres = timing->data.lcd.xres;
    dst_yres = timing->data.lcd.yres;
    /* input resolution */
    src_res = get_input_resolution(vin);
    src_xres = src_res->xres;
    src_yres = src_res->yres;

    lcd300_control_en(ip_io_base, 0);
	/* set the frame base */
    lcd300_set_framebase(ip_io_base, lcd_frame_base[lcd_id], channel);
    dev_setting_bgplane(ip_io_base, src_xres, src_yres, 0, 2048, 2048);
    dev_setting_pattern_gen(ip_io_base, in_fmt, channel, PATGEN_ENABLE);
    dev_channel_cfg(ip_io_base, in_fmt, src_xres, src_yres, channel);
    if (dev_input_scaler(ip_io_base, src_xres, src_yres, src_xres, src_yres, channel))
        return -1;
    dev_setting_output_yuv2rgb(ip_io_base, &rgb2yuv);
    dev_output_scaler0(ip_io_base, src_xres, src_yres, dst_xres, dst_yres);
    dev_load_mixer(ip_io_base);
    dev_switch_output(ip_io_base, timing);
    lcd300_control_en(ip_io_base, 1);

	return 0;
}

static void dev_mau_priority(int priority)
{
	unsigned int val;

	if (priority >= 3) {
		priority = 2;
	}
	val = ioread32(MAU_BASE + 0x04);

	val &= ~(0x3 << 2);
	val |= (priority << 2);
	iowrite32(val, MAU_BASE + 0x04);
}

/* entry point
 */
static int flcd_main(LCD_IDX_T lcd_idx, lcd_vin_t vin, lcd_output_t output, input_fmt_t in_fmt, u32 frame_base)
{
    int ret;
	
    ret = lcd300_pinmux_init(lcd_idx, output);
    if (ret < 0) {
        return -1;
	}

	drv_video_init();

    /* default value */
    ret = lcd300_init(lcd_idx, vin, output, in_fmt, frame_base);

    return ret;
}

#ifdef CONFIG_MS_NVR
static int load_jpeg_file(ulong upload_addr, unsigned int size)
{
    int rc = 0;
    char cmd[CONFIG_SYS_CBSIZE];
    
    sprintf(cmd, "mw.b 0x%lx ff %d; nand read 0x%lx logo", upload_addr, size, upload_addr);    
    rc = run_command(cmd, 0);
    
    return rc;
}
#endif

extern void jpeg_deccode(unsigned char* inbuf, unsigned char* outbuf);
extern void jpeg_decode_central(unsigned int bg_w, unsigned int bg_h, unsigned char* inbuf, unsigned char* outbuf);
extern void jpeg_getdim(unsigned int* width, unsigned int* height);
/* this function is portable for the customer */
static int do_bootlogo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    int hdmi = 0, cvbs = 0;
	BOOL	 is_found;
	INT32	 code;
	UINT32	 hdmi_trim;

    if (argc == 2) {
        char *p = argv[1];
        if (!strcmp("hdmi", p)){
        	hdmi = 1;
			
			if (DEMO_BOOT_LOG_CVBS_ONOFF)
			{
				cvbs = DEMO_BOOT_LOG_CVBS_ONOFF;
				printf("Usage: logo for hdmi|CVBS\n");
			}
			else
			{		
			    printf("Usage: only for hdmi\n");
			}
        }
        else if (!strcmp("cvbs", p) && DEMO_BOOT_LOG_CVBS_ONOFF)
        {
        	cvbs = 1;
        	printf("Usage: only for cvbs\n");
        }
        else
        {
            printf("Usage: bootlogo {hdmi|cvbs} \n");
			return -1;
		}
	}
	
	if (hdmi) {
		code = otp_key_manager(4); // PackageUID addr[4] bit [11] = 1 HDMI(X)	bit[11] = 0 HDMI(O)

		if(code == -33) {
			printf("Read package UID error\r\n");
			hdmi = 0;
		} else {
			hdmi_trim = 0;
			is_found = extract_trim_valid(code, &hdmi_trim);
			if(is_found) {
				if(((hdmi_trim & (1<<11)) == (1<<11))) {
					printf("Failed to init hdmi, chip no support feature\n");
					hdmi = 0;
					return -1;
				} 
			} else {
				printf("Read package UID = 0x%x(NULL)\r\n", (uint)hdmi_trim);
			}
		}
	}

#ifdef DEMO_BOOT_LOGO_JPEG    
    {
    //#include "logo.dat"
        unsigned char *jpeg_file_buf = (unsigned char*)JPEGFILE_BUFFER_BASE;//inbuf;
        unsigned char *jpeg_file_buf_cvbs = (unsigned char*)JPEGFILE_BUFFER_BASE_CVBS;//inbuf;
#ifdef CONFIG_MS_NVR
        load_jpeg_file(JPEGFILE_BUFFER_BASE, FRAME_BUFFER_SIZE);
#endif

		unsigned char* outbuf;
		unsigned int img_width, img_height;
		
	//jpeg_decode(inbuf, outbuf);
   	if (hdmi){
  		outbuf = (unsigned char*)FRAME_BUFFER_BASE;
		printf("[hdmi] start JPEG decode\n");
		dcache_enable();
		#if DEMO_BOOT_LOG_HDMI_RES_1024X768
    	jpeg_decode_central(1024, 768, jpeg_file_buf, outbuf);//1024,768		
		#else		
    	jpeg_decode_central(1920, 1080, jpeg_file_buf, outbuf);// 1920,1080
		#endif

        jpeg_getdim(&img_width, &img_height);
        printf("hdmi/vga logo : image_size: %dx%d\n", img_width, img_height);
        dcache_disable();
    }
   	if (cvbs){
  		outbuf = (unsigned char*)FRAME_CVBS_BUFFER_BASE;
		printf("[cvbs] start JPEG decode\n");
		dcache_enable();
   		jpeg_decode_central(720, 576, jpeg_file_buf_cvbs, outbuf);

        jpeg_getdim(&img_width, &img_height);
        printf("cvbs logo : image_size: %dx%d\n", img_width, img_height);
        dcache_disable();
   	}
		printf("end\n");
   }
#endif

	dev_mau_priority(LCD_PRI);

    flcd_main(LCD0_ID, g_vin[LCD0_ID], g_output[LCD0_ID], g_infmt[LCD0_ID], lcd_frame_base[LCD0_ID]);
    /* this only for lcd0 */
    if (hdmi) {
		hdmi_video_t hdmi_video;

        switch (g_output[LCD0_ID]) {
		case OUTPUT_1280x720:
			hdmi_video = HDMI_VIDEO_720P;
			break;
		case OUTPUT_1920x1080:
			hdmi_video = HDMI_VIDEO_1080P;
			break;
		case OUTPUT_1024x768:
			hdmi_video = HDMI_VIDEO_1024x768;
			break;
		default:
			return -1;
			break;
		}
		hdmi_if_init(hdmi_video);
    }

	if (cvbs) {
		extern void flcd210_main(int cvbs);

		flcd210_main(cvbs);
	}

    return 0;
}

U_BOOT_CMD(
	bootlogo,	2,	1,	do_bootlogo,
	"show lcd bootlogo [hdmi|cvbs]",
	"no argument means VGA output only, \n"
	"[hdmi] - VGA and HDMI output simulataneously"
);


