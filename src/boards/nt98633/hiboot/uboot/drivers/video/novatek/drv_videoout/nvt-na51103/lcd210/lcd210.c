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
#include <asm/arch/IOAddress.h>
#include "asm/arch/lcd210.h"
#if 0 /* TBD, eFuse */
#include "../board/novatek/common/include/nvt_rdata_sys.h"
#include "../board/novatek/common/include/nvt_rdata_cst.h"
#include "../board/novatek/common/include/nvt_rdata.h"
#endif
#include <asm/arch/efuse_protected.h>


#define CVBS_VERSION "1.1.6"

/*---------------------------------------------------------------
 * definition
 *---------------------------------------------------------------
 */
typedef enum {
    VIN_NTSC,
    VIN_PAL,
} lcd_vin_t;

typedef enum {
    OUTPUT_NTSC,
    OUTPUT_PAL,
} lcd_output_t;

typedef enum {
    INPUT_FMT_YUV422 = 0,
    INPUT_FMT_RGB888,
    INPUT_FMT_RGB565,
    INPUT_FMT_ARGB1555,
} input_fmt_t;

/*---------------------------------------------------------------
 * Configurable
 *---------------------------------------------------------------
 */
/* logo base */
//#define FRAME_BUFFER_BASE	0x10800000   //TBD, configurable
#define FRAME_BUFFER_SIZE	(1 << 20)    //1M, configurable

static lcd_output_t g_output[1] = {OUTPUT_NTSC};   //configurable
static lcd_vin_t    g_vin[1] = {VIN_NTSC};         //configurable
static input_fmt_t  g_infmt[1] = {INPUT_FMT_YUV422 /* INPUT_FMT_RGB565 */};    //configurable

#define PATGEN_ENABLE   0   /* 1 for enable pattern generation */
#define VERSION			"1.3"

#define EMU_FPGA 0
#define EFUSE_USED 1

/*************************************************************************************
 *  LCD210 IO Read/Write
 *************************************************************************************/
#define ioread32        readl
#define iowrite32       writel

/*---------------------------------------------------------------
 * Body
 *---------------------------------------------------------------
 */
static u32 diplay_pll[3] = {297000, 297000, 297000}; /* khz */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#define printk	printf

#if defined(CONFIG_ARM64)
#define CG_BASE         0x2F0020000
#define LCD210_BASE		0x2F0820000
#define TVE100_BASE     0x2F0840000
#else
#define CG_BASE         0xF0020000
#define LCD210_BASE		0xF0820000
#define TVE100_BASE     0xF0840000
#endif


/*************************************************************************************
 *  LCD TIMING MACROs Begin
 *************************************************************************************/
#define LCDHTIMING(HBP, HFP, HW, PL)  ((((HBP)-1)&0xFF)<<24|(((HFP)-1)&0xFF)<<16| \
				       (((HW)-1)&0xFF)<<8|((((PL)>>4)-1)&0xFF))
#define LCDVTIMING0(VFP, VW, LF)      (((VFP)&0xFF)<<24|(((VW)-1)&0x3F)<<16|(((LF)-1)&0xFFF))
#ifdef SHARP_OFF
#define LCDVTIMING1(VBP)              ((VBP)&0xFF)
#else
#define LCDVTIMING1(VBP)              (((VBP)-1)&0xFF)
#endif /* SHARP_OFF */

//#define LCDVTIMING1(VBP)              ((VBP)&0xFF)
#define LCDCFG(PLA_PWR, PLA_DE, PLA_CLK, PLA_HS, PLA_VS) (((PLA_PWR)&0x01)<<4| \
							  ((PLA_DE)&0x01)<<3| \
							  ((PLA_CLK)&0x01)<<2| \
							  ((PLA_HS)&0x01)<<1| \
							  ((PLA_VS)&0x01))
#define LCDPARM(PLA_PWR, PLA_DE, PLA_CLK, PLA_HS, PLA_VS)
#define LCDHVTIMING_EX(HBP, HFP, HW, VFP, VW, VBP)  \
            ((((((HBP)-1)>>8)&0x3)<<8) | (((((HFP)-1)>>8)&0x3)<<4) | (((((HW)-1)>>8)&0x3)<<0) | \
            ((((VFP)>>8)&0x3)<<16) | (((((VW)-1)>>8)&0x3)<<12) | (((((VBP)-1)>>8)&0x3)<<20))

#define CCIR656CYC(H,V)             (((V)&0xFFF)<<12|((H)&0xFFF))
#define CCIR656FIELDPLA(F1_0,F0_1)  (((F0_1)&0xFFF)<<12|((F1_0)&0xFFF))
#define CCIR656VBLANK01(VB0,VB1)    (((VB1)&0x3FF)<<12|((VB0)&0x3FF))
#define CCIR656VBLANK23(VB2,VB3)    (((VB3)&0x3FF)<<12|((VB2)&0x3FF))
#define CCIR656VACTIVE(VA0,VA1)     (((VA1)&0xFFF)<<12|((VA0)&0xFFF))
#define CCIR656HBLANK01(HB0,HB1)    (((HB1)&0x3FF)<<12|((HB0)&0x3FF))
#define CCIR656HBLANK2(HB2)         (((HB2)&0x3FF))
#define CCIR656HACTIVE(HA)          (((HA)&0xFFF))
#define CCIR656VBLANK45(VB4,VB5)    (((VB5)&0x3FF)<<12|((VB4)&0x3FF))

#define CCIR656_OUTFMT_MASK      1
#define CCIR656_OUTFMT_INTERLACE 0
#define CCIR656_OUTFMT_PROGRESS  1

#define CCIR656CONFIG(SYS,INFMT,CLK_INV) ((SYS)|(INFMT)|((CLK_INV)&0x01)<<1)
#define CCIR656_SYS_MASK        0x8
#define CCIR656_SYS_SDTV        0
#define CCIR656_SYS_HDTV        0x8
#define CCIR656_INFMT_PROGRESS  0
#define CCIR656_INFMT_INTERLACE 0x4

struct CCIR656_TIMING {
    u32 Cycle_Num;              /* in new lcd200, it includes H_cyc_num and V_Line_num */
    u32 Field_Polarity;
    u32 V_Blanking01;
    u32 V_Blanking23;
    u32 V_Active;
    u32 H_Blanking01;
    u32 H_Blanking2;
    u32 H_Active;
    u32 V_Blanking45;
    u32 Parameter;
};

typedef struct ffb_timing_info {
	int otype;
    /* CCIR656 */
    struct {
        int pixclock;
        int	xres;
        int	yres;
        int field0_switch;
        int field1_switch;
        int hblank_len;
        int vblank0_len;
        int vblank1_len;
        int vblank2_len;
    } ccir656;
    u32	config;
} ffb_timing_info_t;

static struct ffb_timing_info timing_table[] = {
	{OUTPUT_NTSC, {27000, 720, 480, 4, 266, 134, 22, 23, 0}, CCIR656CONFIG(CCIR656_SYS_SDTV, CCIR656_INFMT_PROGRESS, 1)},
	{OUTPUT_PAL,  {27000, 720, 576, 1, 313, 140, 22, 25, 2}, CCIR656CONFIG(CCIR656_SYS_SDTV, CCIR656_INFMT_PROGRESS, 1)},
};

#if 0
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

/* TBD */
    /* offset, bits_mask, lock_bits, init_val, init_mask */
static pmuReg_t pmu_reg_tbl[] = {
    /* IPs ARESETN controlled. 0: Reset, 1: Release */
    {0x50, 0x1 << 5, 0x0, 0x1 << 5, 0x1 << 5},
    /* TVE */
    {0x54, 0x1 << 13, 0x1 << 13, 0x1 << 13, 0x1 << 13},
    /* IPs PRESETN controlled. 0: Reset, 1: Release */
    {0x58, 1 << 7, 0x0, 0x1 << 7, 0x1 << 7},
	/* gating clock - AClkOff0. 0: Release, 1: Gating */
	{0x60, 0x1 << 14, 0x1 << 14, 0x0, 0x1 << 14},
	/* TVE, gating clock - HClkOff. 0: Release, 1: Gating */
	{0x68, 0x1 << 13, 0x1 << 13, 0x0, 0x1 << 13},
	/* gating clock - PClk0Off. 0: Release, 1: Gating */
	{0x70, 0x1 << 7, 0x0, 0x0, 0x1 << 7},
	/* CVBS DAC Config */
	/*{0xFC, 0xF, 0xF, 0x0, 0xF},*/
	/* LCDC Clock  */
	{0x174, 0xFFFFFFFF, 0xFFFF, 0x0, 0x0},
};
#endif

struct ffb_timing_info *get_timing(lcd_output_t out_idx)
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
static void cg_write_reg(int lcd_fd, unsigned int offset, unsigned int val, unsigned int mask)
{
    unsigned int tmp;
	u64 base = CG_BASE + offset;

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
	printf("cg_write_reg: offset: 0x%lx, val: 0x%x\n", base, tmp);
}

/* this function will be called in initialization state or module removed */
static void platform_pmu_clock_gate(int on_off)
{
	u32 bit, mask;
	int	lcd_fd = 0;
	unsigned int tmp;

	/* LCD210_1
	 */
	if (on_off == 1) {
#if 0
    	/* gating clock - AClkOff0. 0: Release, 1: Gating. lcd210a */
    	bit = (0x0 << 14);
    	mask = (0x1 << 14);
    	cg_write_reg(lcd_fd, 0x60, bit, mask);
		/* gating clock - HClkOff. 0: Release, 1: Gating. TVE */
		bit = (0x0 << 13);
		mask = (0x1 << 13);
		cg_write_reg(lcd_fd, 0x68, bit, mask);

		/* gating clock - PClk0Off. 0: Release, 1: Gating. lcd210p */
    	bit = 0x0 << 7;
    	mask = 0x1 << 7;
    	cg_write_reg(lcd_fd, 0x70, bit, mask);
#endif
    	/* turn on DAC */
		/*ftpmu010_write_reg(lcd_fd, 0xFC, 0x0, 0xF);*/
		tmp = ioread32(TVE100_BASE + 0x44);
		tmp &= ~(0x4003);
		iowrite32(tmp, TVE100_BASE + 0x44);
    } else {
#if 0
    	/* gating clock - AClkOff0. 0: Release, 1: Gating. lcd210a */
    	bit = (0x1 << 14);
    	mask = (0x1 << 14);
    	cg_write_reg(lcd_fd, 0x60, bit, mask);
		/* gating clock - HClkOff. 0: Release, 1: Gating. TVE */
		bit = (0x1 << 13);
		mask = (0x1 << 13);
		cg_write_reg(lcd_fd, 0x68, bit, mask);

		/* gating clock - PClk0Off. 0: Release, 1: Gating. lcd210p */
    	bit = 0x1 << 7;
    	mask = 0x1 << 7;
    	cg_write_reg(lcd_fd, 0x70, bit, mask);
#endif
    	/* turn off DAC */
		/*ftpmu010_write_reg(lcd_fd, 0xFC, 0xF, 0xF);*/
		tmp = ioread32(TVE100_BASE + 0x44);
		tmp |= (0x3);
		iowrite32(tmp, TVE100_BASE + 0x44);
    }
}

/* pixclk: Standard VESA clock (kHz), TBD
 */
static unsigned int platform_pmu_calculate_clk(u32 pixclk)
{
	int	div, lcd_fd = 0, clk_src;
    u32	pll = diplay_pll[0];
#if (!EMU_FPGA)
	/* 27Mhz clock, it is only D1.
	 * lcd200 clock select. 0: pll9(DIS0_CK), 1: pll10(DIS1_CK), 2: pll11(DIS2_CK)
	 */
	clk_src = 2; /* pll11 */
	div = ((pll / pixclk)-1) & 0x3F;
	printk("LCD210: chooses PLL11:%d with div=%d\n", pll, div+1);

	/* lcd210 div: 0x4C[5..0] */
	cg_write_reg(lcd_fd, 0x4C, (div), (0x3F << 0));
	/* lcd210 src: 0x20[9..8] */
	cg_write_reg(lcd_fd, 0x20, (clk_src << 8), (0x3 << 8));

	return pll / (div + 1);
#else
	return 27000000;
#endif
}

int lcd210_pinmux_init(lcd_output_t output)
{
	struct ffb_timing_info *timing;
    unsigned int i;
	//unsigned int pmu_tbl_sz = 0;
    //pmuReg_t *pmu_tbl_ptr = NULL;
	int	lcd_fd = 0;
	u32 bit, mask;

    if ((timing = get_timing(output)) == NULL)
        return -1;
#if 0
    pmu_tbl_ptr = &pmu_reg_tbl[0];
    pmu_tbl_sz = sizeof(pmu_reg_tbl) / sizeof(pmuReg_t);

    for (i = 0; i < pmu_tbl_sz; i ++) {
        ftpmu010_write_reg(lcd_fd, pmu_tbl_ptr->reg_off, pmu_tbl_ptr->init_val, pmu_tbl_ptr->init_mask);
        pmu_tbl_ptr ++;
    }
#else
	//lcd210 clken: 0x70[26], gpdec: 0x70[19]
	bit = (0x1 << 26) | (0x1 << 19);
	mask = (0x1 << 26) | (0x1 << 19);
	cg_write_reg(lcd_fd, 0x70, bit, mask);

	//lcd210 rstn: 0x94[14]
	bit = (0x1 << 14);
	mask = (0x1 << 14);
	cg_write_reg(lcd_fd, 0x94, bit, mask);
	
	//lcd210, tve prog clken: 0xE0[14], 0xE0[31]
	bit = (0x1 << 14) | (0x1 << 31);
	mask = (0x1 << 14) | (0x1 << 31);
	cg_write_reg(lcd_fd, 0xE0, bit, mask);

	//tve clken: 0x74[13]
	bit = (0x1 << 13);
	mask = (0x1 << 13);
	cg_write_reg(lcd_fd, 0x74, bit, mask);

	//tve rstn: 0x98[4]
	bit = (0x1 << 4);
	mask = (0x1 << 4);
	cg_write_reg(lcd_fd, 0x98, bit, mask);
#endif

    /* turn on gate clock */
    platform_pmu_clock_gate(1);
    platform_pmu_calculate_clk(timing->ccir656.pixclock);

    return 0;
}

static int drv_check_cvbs_available(void)
{
	int  code;
	BOOL is_found;
	int  ret = 0;
	u32  packageUID;

	code = otp_key_manager(4); // PackageUID addr[4]

	if(code == -33) {
		printf("Read package UID error\r\n");
	} else {
		packageUID = 0;

		is_found = extract_trim_valid(code, &packageUID);

		if(is_found) {
			if((packageUID == 0x383) || (packageUID == 0x364)) { //Only here can not use
				printf("can not use CVBS\n");
				ret = -1;
			}
		}
	}
	return ret;
}

static int drv_video_init(int output)
{
	u32 tmp, dac_val = 0;
	int	input = 2, lcd_fd = 0, ret = 0;
	/* TV-DAC default value */
	u32 val_1B8 = 0x8;

	if (dac_val || val_1B8 || ret || lcd_fd) {};

#if 1 /* TBD, efuse */
#if (EFUSE_USED)

		BOOL	 is_found;
		int	 code;
		u32	 dac_trim;
		u32	 reg_temp;
	
		code = otp_key_manager(OTP_CVBS_VGA_TRIM_FIELD); // 5 => HDMI / 3 => CVBS
	
		if(code == -33) {
			printf("Read cvbstrim error\r\n");
		} else {
			dac_trim = 0;
			is_found = extract_trim_valid(code, &dac_trim);
			if(is_found) {
				//printf("Read vga trim = 0x%08x\r\n", dac_trim);
				if (((dac_trim & 0x7F) >= 0x1A) && ((dac_trim & 0x7F) < 0x3C)) {
					//TV_DAC_PD_CTRL_OFS 0x44
					reg_temp = ioread32(TVE100_BASE + 0x44);
					reg_temp &= ~(0xFF << 4);
					reg_temp |= ((dac_trim & 0x7F)	<< 4);
					iowrite32(reg_temp, TVE100_BASE + 0x44);
				}
				else {
					printf("CVBS: efuse trim value over range 0x%x\n", (uint)dac_trim);
					reg_temp = ioread32(TVE100_BASE + 0x44);
					reg_temp &= ~(0xFF << 4);
					reg_temp |= (0x20  << 4);
					iowrite32(reg_temp, TVE100_BASE + 0x44);
				}
				
			} else {
				printf("Read cvbs trim = 0x%x(NULL)\r\n", (uint)dac_trim);
				reg_temp = ioread32(TVE100_BASE + 0x44);
				reg_temp &= ~(0xFF << 4);
				reg_temp |= (0x20  << 4);
				iowrite32(reg_temp, TVE100_BASE + 0x44);
			}
		}
#endif		
#else
	struct buffer_info_t  efuse;

	memset(&efuse, 0x0, sizeof(struct buffer_info_t));
	efuse.size = sizeof(dac_val);
	efuse.data = (unsigned char*)&dac_val;
	ret = nvt_read_data(SYSTEM_TYPE, SYS_VDAC_CVBS, 544, 8, &efuse);
	if (!ret) {
		dac_val &= 0xF;
		val_1B8 = dac_val;
	}
	/* 2019/4/1 02:06:53 PM */
	if (output == OUTPUT_PAL)
		val_1B8 += 2;	/* Compared to NTSC, PAL must offset 2, comment from Jimmy */

	ftpmu010_write_reg(lcd_fd, 0x1B8, val_1B8, 0xFF);
#endif

	if ((input != 2) && (input != 6)) {   /* color bar */
        return -EINVAL;
    }

    /* 0x0 */
    tmp = (output == OUTPUT_NTSC) ? 0x0 : 0x2;
    iowrite32(tmp, TVE100_BASE + 0x0);

    /* 0x4 */
    iowrite32(input, TVE100_BASE + 0x4);

    /* 0x8, 0xc */
    iowrite32(0, TVE100_BASE + 0x8);

    /* bit 0: power down, bit 1: standby */
    iowrite32(0, TVE100_BASE + 0xc);

	iowrite32(0x0, TVE100_BASE + 0x6c);

    return 0;
}

static int translate_timing(lcd_output_t output)
{
	struct ffb_timing_info *timing;
	struct CCIR656_TIMING CCIR656;
	u32	config, tmp, tmp_w, tmp_h, div = 0, h_cycle, v_cycle;
	u32	LCD_Configure = 0, LCD_H_Timing, LCD_V_Timing0;
	void *io_base = (void *)LCD210_BASE;
	u32	value;

	timing = get_timing(output);
	if (timing == NULL)
		return -1;

	memset(&CCIR656, 0, sizeof(CCIR656));
	config = timing->config;
	if ((config & CCIR656_SYS_MASK) == CCIR656_SYS_SDTV)
		config |= CCIR656_OUTFMT_INTERLACE;
	else
		config |= CCIR656_OUTFMT_PROGRESS;

	tmp_w = timing->ccir656.xres;
	tmp_h = timing->ccir656.yres;

    CCIR656.Field_Polarity = CCIR656FIELDPLA(timing->ccir656.field0_switch, timing->ccir656.field1_switch);
    CCIR656.V_Blanking01 = CCIR656VBLANK01(timing->ccir656.vblank0_len, timing->ccir656.vblank1_len);
    tmp = tmp_h - timing->ccir656.yres;
    tmp = tmp >> 1;         /* V_BLK3 */
    if (config & CCIR656_OUTFMT_PROGRESS)   /* Progressive */
        CCIR656.V_Blanking23 = CCIR656VBLANK23(timing->ccir656.vblank2_len, tmp);
    else                    /* Interlace */
        CCIR656.V_Blanking23 = CCIR656VBLANK23(timing->ccir656.vblank2_len, tmp >> 1);

    tmp = tmp_h - (tmp << 1);       /* real active image */

    if ((config & CCIR656_OUTFMT_MASK) == CCIR656_OUTFMT_INTERLACE) {
        tmp = tmp >> 1;     /* if interlace, we divide the image into two fields */
        CCIR656.V_Active = CCIR656VACTIVE(tmp, tmp);
    }

    if ((config & CCIR656_OUTFMT_MASK) == CCIR656_OUTFMT_PROGRESS) {
        CCIR656.V_Active = CCIR656VACTIVE(tmp, 0);
    }

    tmp = tmp_w - timing->ccir656.xres;

    h_cycle = tmp_w + timing->ccir656.hblank_len;
    v_cycle = tmp_h + timing->ccir656.vblank0_len + timing->ccir656.vblank1_len +
        timing->ccir656.vblank2_len;
    if ((config & CCIR656_SYS_MASK) == CCIR656_SYS_HDTV) {
        CCIR656.H_Blanking01 = CCIR656HBLANK01(timing->ccir656.hblank_len, (tmp >> 1));
        CCIR656.H_Blanking2 = CCIR656HBLANK2(((tmp + 1) >> 1));
        CCIR656.H_Active = CCIR656HACTIVE(timing->ccir656.xres);
    } else {
        CCIR656.H_Blanking01 = CCIR656HBLANK01(timing->ccir656.hblank_len << 1, (tmp >> 1) << 1);        /* 2 cycles */
        CCIR656.H_Blanking2 = CCIR656HBLANK2(((tmp + 1) >> 1) << 1);
        CCIR656.H_Active = CCIR656HACTIVE(timing->ccir656.xres << 1);
        h_cycle <<= 1;      /* double */
    }

    CCIR656.Parameter = config;
    CCIR656.Cycle_Num = CCIR656CYC(h_cycle + 8, v_cycle);  /* Note:SAV/EAV */

    LCD_Configure |= (div & 0x7F) << 8;
    LCD_H_Timing = LCDHTIMING(0, 0, 0, timing->ccir656.xres);
    LCD_V_Timing0 = LCDVTIMING0(0, 0, timing->ccir656.yres);

    value = ioread32(io_base + 0x204) & (0x1 << 2);        /* Only ImgFormat */
    value |= CCIR656.Parameter;
    iowrite32(value, io_base + 0x204);
    iowrite32(CCIR656.Cycle_Num, io_base + 0x208);
    iowrite32(CCIR656.Field_Polarity, io_base + 0x20C);
    iowrite32(CCIR656.V_Blanking01, io_base + 0x210);
    iowrite32(CCIR656.V_Blanking23, io_base + 0x214);
    iowrite32(CCIR656.V_Active, io_base + 0x218);
    iowrite32(CCIR656.H_Blanking01, io_base + 0x21C);
    iowrite32(CCIR656.H_Blanking2, io_base + 0x220);
    iowrite32(CCIR656.H_Active, io_base + 0x224);

    iowrite32(LCD_H_Timing, io_base + 0x100);
    iowrite32(LCD_V_Timing0, io_base + 0x104);
    iowrite32(LCD_Configure, io_base + 0x10C);

    return 0;
}

void set_color_management(lcd_output_t output)
{
	void *io_base = (void *)LCD210_BASE;
	u32 k0 = 2, k1 = 4, shth0 = 8, shth1 = 32;  //weak sharpness
	u32	color_mgt_p0, color_mgt_p1, color_mgt_p2, color_mgt_p3;

	color_mgt_p0 = 0x2000;
    color_mgt_p1 = 0x2000;
    color_mgt_p2 = ((k0 & 0xF) << 16) | ((k1 & 0xF) << 20) | (shth0 & 0xFF) | ((shth1 & 0xFF) << 8);
    color_mgt_p3 = 0x40000;

	/* in order to pass vector scope */
    if (output == OUTPUT_NTSC) {
    	color_mgt_p0 = 0x2000;
    }
    if (output == OUTPUT_PAL) {
    	color_mgt_p0 = 0x208d ;
    }
	iowrite32(color_mgt_p0, io_base + 0x400);
	iowrite32(color_mgt_p1, io_base + 0x404);
	iowrite32(color_mgt_p2, io_base + 0x408);
	iowrite32(color_mgt_p3, io_base + 0x40C);
}

void enable_controller(lcd_output_t output, input_fmt_t infmt)
{
	void *io_base = (void *)LCD210_BASE;
	u32	config, tmp;

	tmp = (infmt == INPUT_FMT_YUV422) ? 1 : 0;

	config = (0x1 << 17) /* Double_En */ | (PATGEN_ENABLE << 14) | (0x1 << 13) /* TVEn */ | (tmp << 3) /* EnYCbCr */ | 0x1;

	iowrite32(config, io_base);
}

int set_infmt(input_fmt_t infmt)
{
	void *io_base = (void *)LCD210_BASE;
	u32	val;

	val = ioread32(io_base + 0x4) & ~(0x3 << 7 | 0x7);

	switch (infmt) {
	case INPUT_FMT_YUV422:
		val |= 0x04;
		break;
	case INPUT_FMT_RGB888:
		val |= 0x05;
		break;
	case INPUT_FMT_RGB565:
		val |= 0x04;
		break;
	case INPUT_FMT_ARGB1555:
		val |= (0x1 << 7) | 0x04;
		break;
	default:
		printk("%s, unsupport infmt: %d \n", __func__, infmt);
		return -1;
		break;
	}
	iowrite32(val, io_base + 0x4);
	return 0;
}

void set_framebase(u32 buffer_base)
{
	void *io_base = (void *)LCD210_BASE;

	iowrite32(buffer_base & ~0x7, io_base + 0x18);
}

int lcd210_init(lcd_output_t output, lcd_vin_t vin, input_fmt_t	infmt, u32 frame_base)
{
	/* set timing */
	if (translate_timing(output))
		return -1;
	set_color_management(output);
	if (set_infmt(infmt) < 0)
		return -1;
	set_framebase(frame_base);
	enable_controller(output, infmt);

	return 0;
}

void flcd210_main(int cvbs, u32 frame_base)
{
	lcd_output_t output = g_output[0];
	lcd_vin_t	vin = g_vin[0];
	input_fmt_t	infmt = g_infmt[0];
	int	ret;

	/* only support cvbs */
	if (cvbs == 0)
		return;

	if (drv_check_cvbs_available() != 0) {
		printk("Not Support CVBS bootlogo.\n");
		return;
	}

	printk("Bootlogo CVBS ON. \n");

	ret = lcd210_pinmux_init(output);
    if (ret < 0) {
    	printk("%s pinmux init fail! \n", __func__);
        return;
	}

    ret = lcd210_init(vin, output, infmt, frame_base);
	if (!ret) {
		/* enable cvbs */
		drv_video_init(g_output[0]);
	} else {
		printk("%s lcd210 init fail! \n", __func__);
	}
}

