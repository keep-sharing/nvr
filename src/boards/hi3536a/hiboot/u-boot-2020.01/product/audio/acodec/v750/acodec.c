// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <common.h>
#include <command.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <config.h>

#include "audio_ao.h"
#include "acodec_def.h"
#include "acodec.h"

#define io_address(x) (x)

void *g_acodec_crg_reg = NULL;

static unsigned long g_acodec_base;

unsigned int acodec_hal_read_reg(unsigned int offset)
{
    return (*(volatile unsigned int *)((unsigned long)g_acodec_base + (unsigned int)offset));
}

void acodec_hal_write_reg(unsigned int offset, unsigned int value)
{
    *(volatile unsigned int *)((unsigned long)(g_acodec_base) + (unsigned int)(offset)) = value;
}

void acodec_hal_dump(unsigned int offset)
{
    printf("addr: %8lx  value: %x\n", ((unsigned long)g_acodec_base + (unsigned int)offset),
        acodec_hal_read_reg(offset));
}

static inline void acodec_reg_write32(unsigned long value, unsigned long mask, unsigned long addr)
{
    unsigned long t;

    t = readl((const volatile void *)addr);
    t &= ~mask;
    t |= value & mask;
    writel(t, (volatile void *)addr);
}

static inline void acodec_regsetbit(unsigned long value, unsigned long offset, unsigned long addr)
{
    unsigned long t, mask;

    mask = 1 << offset;
    t = readl(addr);
    t &= ~mask;
    t |= (value << offset) & mask;
    writel(t, addr);
}

static inline void reg_read(unsigned long *pvalue, unsigned long addr)
{
    *pvalue = readl((const volatile void *)addr);
}

static void acodec_ana_power_on_step_0(void)
{
    acodec_anareg0 acodec_anareg0;
    acodec_anareg1 acodec_anareg1;
    acodec_anareg2 acodec_anareg2;
    acodec_anareg3 acodec_anareg3;
    acodec_anareg4 acodec_anareg4;
    acodec_anareg5 acodec_anareg5;

    acodec_anareg0.ul32 = ACODEC_ANAREG0_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);
    acodec_anareg1.ul32 = ACODEC_ANAREG1_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, acodec_anareg1.ul32);
    acodec_anareg2.ul32 = ACODEC_ANAREG2_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);
    acodec_anareg3.ul32 = ACODEC_ANAREG3_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);
    acodec_anareg4.ul32 = ACODEC_ANAREG4_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG4_ADDR, acodec_anareg4.ul32);
    acodec_anareg5.ul32 = ACODEC_ANAREG5_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG5_ADDR, acodec_anareg5.ul32);
}

static void acodec_ana_power_on_step_1_6(void)
{
    acodec_anareg0 acodec_anareg0;
    acodec_anareg3 acodec_anareg3;

    /* ************ depop ************ */
    /* 2. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.rstn = 0x1;
    acodec_anareg3.bits.pd_dac_clk = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 3. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.pop_rst = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 5. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_ctcm_rx = 0x0;
    acodec_anareg0.bits.pd_ctcm_tx = 0x0;
    acodec_anareg0.bits.pd_dac_vref = 0x0;
    acodec_anareg0.bits.pd_ibias = 0x0;
    acodec_anareg0.bits.pd_ldo = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* 6. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.pd_dacl_dff = 0x0;
    acodec_anareg3.bits.pd_dacr_dff = 0x0;
    acodec_anareg3.bits.mute_dacl = 0x0;
    acodec_anareg3.bits.mute_dacr = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);
}

static void acodec_ana_power_on_step_7_13(void)
{
    acodec_anareg0 acodec_anareg0;
    acodec_anareg1 acodec_anareg1;
    acodec_anareg2 acodec_anareg2;
    acodec_anareg3 acodec_anareg3;

    /* 7. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.pu_pop_pullb_reg = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 8. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_vref = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);
    acodec_anareg2.ul32 = acodec_hal_read_reg(ACODEC_ANAREG2_ADDR);
    acodec_anareg2.bits.vref_pu_pdb = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);

    udelay(20 * 1000); /* 20 * 1000 us */

    /* 9. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.pop_rst = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 10. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_lineoutl = 0x0;
    acodec_anareg0.bits.pd_lineoutr = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    udelay(10 * 1000); /* 10 * 1000 us */

    /* 11. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.pop_lineout_pull_en = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 12. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_micbias1 = 0x0;
    acodec_anareg0.bits.pd_micbias2 = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* 13. */
    acodec_anareg1.ul32 = acodec_hal_read_reg(ACODEC_ANAREG1_ADDR);
    acodec_anareg1.bits.pd_rctune = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, acodec_anareg1.ul32);
}

static void acodec_ana_power_on(void)
{
    acodec_anareg0 acodec_anareg0;
    acodec_anareg1 acodec_anareg1;
    acodec_anareg3 acodec_anareg3;

    acodec_ana_power_on_step_0();

    acodec_ana_power_on_step_1_6();

    acodec_ana_power_on_step_7_13();

    /* 14. */
    acodec_anareg1.ul32 = acodec_hal_read_reg(ACODEC_ANAREG1_ADDR);
    acodec_anareg1.bits.rctune = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, acodec_anareg1.ul32);

    udelay(1 * 1000); /* 1 * 1000 us */

    /* 15. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_linein_l = 0x0;
    acodec_anareg0.bits.pd_linein_r = 0x0;
    acodec_anareg0.bits.pd_adcl = 0x0;
    acodec_anareg0.bits.pd_adcr = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* 16. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.rstb_dac = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);
}

static void acodec_ana_power_down_step_1_6(void)
{
    acodec_anareg0 acodec_anareg0;
    acodec_anareg3 acodec_anareg3;

    /* ************ depop ************ */
    /* 1. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_linein_l = 0x1;
    acodec_anareg0.bits.pd_linein_r = 0x1;
    acodec_anareg0.bits.pd_adcl = 0x1;
    acodec_anareg0.bits.pd_adcr = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* 2. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_micbias1 = 0x1;
    acodec_anareg0.bits.pd_micbias2 = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* 3. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.rstb_dac = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 4. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.pop_lineout_pull_en = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 5. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_lineoutl = 0x1;
    acodec_anareg0.bits.pd_lineoutr = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* 6. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_dac_vref = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* 7. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_vref = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);
}

static void acodec_ana_power_down_step_9_10(void)
{
    acodec_anareg0 acodec_anareg0;
    acodec_anareg1 acodec_anareg1;
    acodec_anareg2 acodec_anareg2;
    acodec_anareg3 acodec_anareg3;
    acodec_anareg4 acodec_anareg4;
    acodec_anareg5 acodec_anareg5;

    /* 9. */
    acodec_anareg3.ul32 = acodec_hal_read_reg(ACODEC_ANAREG3_ADDR);
    acodec_anareg3.bits.pu_pop_pullb_reg = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* 10. */
    acodec_anareg0.ul32 = acodec_hal_read_reg(ACODEC_ANAREG0_ADDR);
    acodec_anareg0.bits.pd_ctcm_rx = 0x1;
    acodec_anareg0.bits.pd_ctcm_tx = 0x1;
    acodec_anareg0.bits.pd_ibias = 0x1;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_anareg0.ul32 = ACODEC_ANAREG0_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);
    acodec_anareg1.ul32 = ACODEC_ANAREG1_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, acodec_anareg1.ul32);
    acodec_anareg2.ul32 = ACODEC_ANAREG2_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);
    acodec_anareg3.ul32 = ACODEC_ANAREG3_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);
    acodec_anareg4.ul32 = ACODEC_ANAREG4_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG4_ADDR, acodec_anareg4.ul32);
    acodec_anareg5.ul32 = ACODEC_ANAREG5_DEFAULT;
    acodec_hal_write_reg(ACODEC_ANAREG5_ADDR, acodec_anareg5.ul32);
}

static void acodec_ana_power_down(void)
{
    int i;
    unsigned int value;
    acodec_anareg2 acodec_anareg2;
    acodec_anareg4 acodec_anareg4;

    acodec_ana_power_down_step_1_6();

    value = 0x800;
    for (i = 0; i < 12; i++) { /* 12:bit cnt */
        acodec_anareg4.ul32 = acodec_hal_read_reg(ACODEC_ANAREG4_ADDR);
        acodec_anareg4.bits.vref_pd_res_sel = acodec_anareg4.bits.vref_pd_res_sel | value;
        acodec_hal_write_reg(ACODEC_ANAREG4_ADDR, acodec_anareg4.ul32);
        value = value >> 1;
        udelay(1 * 1000); /* 1 * 1000 us */
    }

    acodec_anareg2.ul32 = acodec_hal_read_reg(ACODEC_ANAREG2_ADDR);
    acodec_anareg2.bits.vref_pu_pdb = 0x0;
    acodec_hal_write_reg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);

    /* 8. */
    value = 0x80;
    for (i = 0; i < 8; i++) { /* 8:bit cnt */
        acodec_anareg4.ul32 = acodec_hal_read_reg(ACODEC_ANAREG4_ADDR);
        acodec_anareg4.bits.pop_res_sel = acodec_anareg4.bits.pop_res_sel | value;
        acodec_hal_write_reg(ACODEC_ANAREG4_ADDR, acodec_anareg4.ul32);
        value = value >> 1;
        udelay(1 * 1000); /* 1 * 1000 us */
    }

    acodec_ana_power_down_step_9_10();
}

static int acodec_soft_reset(void)
{
    acodec_digctrl1 acodec_digctrl1;
    acodec_digctrl2 acodec_digctrl2;
    acodec_digctrl3 acodec_digctrl3;
    acodec_digctrl4 acodec_digctrl4;
    acodec_digctrl5 acodec_digctrl5;

    acodec_anareg0 acodec_anareg0;
    acodec_anareg1 acodec_anareg1;
    acodec_anareg2 acodec_anareg2;
    acodec_anareg3 acodec_anareg3;
    acodec_anareg4 acodec_anareg4;
    acodec_anareg5 acodec_anareg5;

    acodec_anareg0.ul32 = 0x1C1C0000;
    acodec_hal_write_reg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_anareg1.ul32 = 0xDF605E66;
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, acodec_anareg1.ul32);

    acodec_anareg2.ul32 = 0x00255544;
    acodec_hal_write_reg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);

    acodec_anareg3.ul32 = 0x072D0451;
    acodec_hal_write_reg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    acodec_anareg4.ul32 = 0x00000000;
    acodec_hal_write_reg(ACODEC_ANAREG4_ADDR, acodec_anareg4.ul32);

    acodec_anareg5.ul32 = 0x00000000;
    acodec_hal_write_reg(ACODEC_ANAREG5_ADDR, acodec_anareg5.ul32);

    acodec_digctrl1.ul32 = 0xff035a00;
    acodec_hal_write_reg(ACODEC_DIGCTRL1_ADDR, acodec_digctrl1.ul32);

    acodec_digctrl2.ul32 = 0x08000001;
    acodec_hal_write_reg(ACODEC_DIGCTRL2_ADDR, acodec_digctrl2.ul32);

    acodec_digctrl3.ul32 = 0x06062424;
    acodec_hal_write_reg(ACODEC_DIGCTRL3_ADDR, acodec_digctrl3.ul32);

    acodec_digctrl4.ul32 = 0x1e1ec001;
    acodec_hal_write_reg(ACODEC_DIGCTRL4_ADDR, acodec_digctrl4.ul32);

    acodec_digctrl5.ul32 = 0x24242424;
    acodec_hal_write_reg(ACODEC_DIGCTRL5_ADDR, acodec_digctrl5.ul32);

    return 0;
}

static int acodec_geti2sfs(acodec_fs acodec_fs)
{
    switch (acodec_fs) {
        case ACODEC_FS_8000:
        case ACODEC_FS_11025:
        case ACODEC_FS_12000:
            return ACODEC_I2S_FS_8000;
        case ACODEC_FS_16000:
        case ACODEC_FS_22050:
        case ACODEC_FS_24000:
            return ACODEC_I2S_FS_16000;
        case ACODEC_FS_32000:
        case ACODEC_FS_44100:
        case ACODEC_FS_48000:
            return ACODEC_I2S_FS_32000;
        case ACODEC_FS_64000:
        case ACODEC_FS_96000:
            return ACODEC_I2S_FS_64000;
        default:
            printf("Unsupport sample_rate %d.\n", acodec_fs);
            return ACODEC_I2S_FS_BUTT;
    }
}

static int acodec_getadcmodesel(acodec_fs acodec_fs)
{
    switch (acodec_fs) {
        case ACODEC_FS_8000:
        case ACODEC_FS_16000:
        case ACODEC_FS_32000:
        case ACODEC_FS_64000:
            return ACODEC_ADC_MODESEL_4096;
        case ACODEC_FS_11025:
        case ACODEC_FS_12000:
        case ACODEC_FS_22050:
        case ACODEC_FS_24000:
        case ACODEC_FS_44100:
        case ACODEC_FS_48000:
        case ACODEC_FS_96000:
            return ACODEC_ADC_MODESEL_6144;
        default:
            printf("Unsupport sample_rate %d.\n", acodec_fs);
            return ACODEC_I2S_FS_BUTT;
    }
}

static acodec_fs acodec_getacodecfs(audio_sample_rate sample_rate)
{
    acodec_fs acodec_fs = ACODEC_FS_BUTT;
    switch (sample_rate) {
        case AUDIO_SAMPLE_RATE_8000:
            acodec_fs = ACODEC_FS_8000;
            break;

        case AUDIO_SAMPLE_RATE_11025:
            acodec_fs = ACODEC_FS_11025;
            break;

        case AUDIO_SAMPLE_RATE_12000:
            acodec_fs = ACODEC_FS_12000;
            break;

        case AUDIO_SAMPLE_RATE_16000:
            acodec_fs = ACODEC_FS_16000;
            break;

        case AUDIO_SAMPLE_RATE_22050:
            acodec_fs = ACODEC_FS_22050;
            break;

        case AUDIO_SAMPLE_RATE_24000:
            acodec_fs = ACODEC_FS_24000;
            break;

        case AUDIO_SAMPLE_RATE_32000:
            acodec_fs = ACODEC_FS_32000;
            break;

        case AUDIO_SAMPLE_RATE_44100:
            acodec_fs = ACODEC_FS_44100;
            break;

        case AUDIO_SAMPLE_RATE_48000:
            acodec_fs = ACODEC_FS_48000;
            break;

        default:
            printf("Unsupport sample_rate %d.\n", sample_rate);
            break;
    }

    return acodec_fs;
}

int acodec_i2s_set(audio_sample_rate sample_rate)
{
    acodec_fs acodec_fs;
    acodec_digctrl1 digctrl1;
    acodec_anareg1 ana_reg1;

    acodec_fs = acodec_getacodecfs(sample_rate);
    if (acodec_fs == ACODEC_FS_BUTT) {
        printf("%s: not support enSample:%d.\n", __FUNCTION__, sample_rate);
        return -1;
    }

    digctrl1.ul32 = acodec_hal_read_reg(ACODEC_DIGCTRL1_ADDR);
    digctrl1.bits.i2s1_fs_sel = acodec_geti2sfs(acodec_fs);
    acodec_hal_write_reg(ACODEC_DIGCTRL1_ADDR, digctrl1.ul32);

    ana_reg1.ul32 = acodec_hal_read_reg(ACODEC_ANAREG1_ADDR);
    ana_reg1.bits.mode_adcr = acodec_getadcmodesel(acodec_fs);
    ana_reg1.bits.mode_adcl = acodec_getadcmodesel(acodec_fs);
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, ana_reg1.ul32);

    /* rctune */
    ana_reg1.ul32 = acodec_hal_read_reg(ACODEC_ANAREG1_ADDR);
    ana_reg1.bits.rctune = 0;
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, ana_reg1.ul32);

    udelay(30); /* 30us */
    ana_reg1.ul32 = acodec_hal_read_reg(ACODEC_ANAREG1_ADDR);
    ana_reg1.bits.rctune = 1;
    acodec_hal_write_reg(ACODEC_ANAREG1_ADDR, ana_reg1.ul32);

    return 0;
}

int acodec_device_init(void)
{
    audio_reg_1 acodec_audio_reg;
    acodec_digctrl1 acodec_digctrl1;
    acodec_digctrl2 acodec_digctrl2;
    acodec_digctrl3 acodec_digctrl3;

    unsigned int aiao_crg;
    unsigned int aiao_cfg;

    g_acodec_base = (unsigned int)io_address(ACODEC_REGS_BASE);
    if (g_acodec_base == 0) {
        printf("could not ioremap acodec regs!");
        return -1;
    }

    g_acodec_crg_reg = (void *)io_address(ACODEC_REGS_CRG);
    if (g_acodec_crg_reg == NULL) {
        printf("could not ioremap acodec regs!");
        return -1;
    }

    udelay(1 * 1000); /* 1 * 1000 us */

    /* audio crg */
    acodec_regsetbit(1, 1, (unsigned long)(uintptr_t)g_acodec_crg_reg);
    acodec_regsetbit(1, 2, (unsigned long)(uintptr_t)g_acodec_crg_reg); /* 2: bit */

    /* aiao clk */
    aiao_crg = (unsigned int)io_address(AIAO_CLK_TX0_CRG);
    *(volatile unsigned int *)((unsigned long)(aiao_crg)) = 0x00152EF0;
    aiao_cfg = (unsigned int)io_address(AIAO_CLK_TX0_CFG);
    *(volatile unsigned int *)((unsigned long)(aiao_cfg)) = 0x00000115;

    udelay(50 * 1000); /* 50 * 1000 us */

    /* acodec mux */
    acodec_audio_reg.ul32 = 0xf;
    acodec_hal_write_reg(ACODEC_AUDIO_REG, acodec_audio_reg.ul32);

    /* digctrl */
    acodec_digctrl1.ul32 = 0xff035a00;
    acodec_hal_write_reg(ACODEC_DIGCTRL1_ADDR, acodec_digctrl1.ul32);

    acodec_digctrl2.ul32 = 0x08000001;
    acodec_hal_write_reg(ACODEC_DIGCTRL2_ADDR, acodec_digctrl2.ul32);

    acodec_digctrl3.ul32 = 0x7e7e2424;
    acodec_hal_write_reg(ACODEC_DIGCTRL3_ADDR, acodec_digctrl3.ul32);

    udelay(1 * 1000); /* 1 * 1000 us */

    acodec_ana_power_on();

    udelay(1 * 1000); /* 1 * 1000 us */

    acodec_soft_reset();

    printf("acodec inited!\n");

    return 0;
}

int acodec_device_exit(void)
{
    acodec_digctrl3 acodec_digctrl3;

    unsigned int aiao_crg;
    unsigned int aiao_cfg;

    acodec_soft_reset();

    udelay(1 * 1000); /* 1 * 1000 us */

    acodec_ana_power_down();

    /* digctrl */
    acodec_digctrl3.ul32 = 0x7f7f2424;
    acodec_hal_write_reg(ACODEC_DIGCTRL3_ADDR, acodec_digctrl3.ul32);
    /* audio crg */
    acodec_regsetbit(0, 1, (unsigned long)(uintptr_t)g_acodec_crg_reg);
    acodec_regsetbit(0, 2, (unsigned long)(uintptr_t)g_acodec_crg_reg); /* 2: bit */

    /* aiao clk */
    aiao_crg = (unsigned int)io_address(AIAO_CLK_TX0_CRG);
    *(volatile unsigned int *)((unsigned long)(aiao_crg)) = 0x00152EF0;
    aiao_cfg = (unsigned int)io_address(AIAO_CLK_TX0_CFG);
    *(volatile unsigned int *)((unsigned long)(aiao_cfg)) = 0x00000115;

    printf("acodec exited!\n");
    return 0;
}
