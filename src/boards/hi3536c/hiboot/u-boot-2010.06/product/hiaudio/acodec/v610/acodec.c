/*
* Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include <common.h>
#include <command.h>
#include <version.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <asm/sizes.h>
#include <config.h>

#if (defined CONFIG_ARCH_HI3519 || defined CONFIG_ARCH_HI3519V101)
#include "hi3519_ao.h"
#endif

#if (defined CONFIG_ARCH_HI3516AV200)
#include "hi3516av200_ao.h"
#endif

#if (defined CONFIG_ARCH_HI3559 || defined CONFIG_ARCH_HI3556)
#include "hi3559_ao.h"
#endif

#include "acodec_def.h"
#include "acodec.h"

#define IO_ADDRESS(x) (x)

void *g_acodec_crg_reg = NULL;

static unsigned int u32AcodecBase;

unsigned int ACODEC_HAL_ReadReg(unsigned int u32Offset)
{
    return (*(volatile unsigned int*)((unsigned int)u32AcodecBase + (unsigned int)u32Offset));
}

void ACODEC_HAL_WriteReg(unsigned int u32Offset, unsigned int u32Value)
{
    *(volatile unsigned int*)((unsigned int)(u32AcodecBase) + (unsigned int)(u32Offset)) = u32Value;
}

void ACODEC_HAL_DUMP(unsigned int u32Offset)
{
    printf("addr: %8x  value: %x\n",((unsigned int)u32AcodecBase + (unsigned int)u32Offset),\
        ACODEC_HAL_ReadReg( u32Offset));
}

//static ACODEC_REGS_S *g_pstAcodecReg = NULL;

static inline void acodec_regsetbit(
        unsigned long value,
        unsigned long offset,
        unsigned long addr)
{
    unsigned long t, mask;

    mask = 1 << offset;
    t = readl(addr);
    t &= ~mask;
    t |= (value << offset) & mask;
    writel(t, addr);
}

static int acodec_soft_reset(void)
{
    ACODEC_DIGCTRL0_U   acodec_digctrl0; 
    ACODEC_DIGCTRL1_U   acodec_digctrl1;
    ACODEC_DIGCTRL2_U   acodec_digctrl2;
    ACODEC_DIGCTRL3_U   acodec_digctrl3;

    ACODEC_ANAREG0_U   acodec_anareg0;
    ACODEC_ANAREG1_U   acodec_anareg1;
    
    if (0 == u32AcodecBase) {
        printf("Haven't ioremap acodec regs!");
        return -1;
    }

    /* ×óÓÒÉùµÀÉÏµç£¬±£³Ömute¡¢popfree*/
    acodec_anareg0.ul32 = 0x6400e402;//0x6405e402;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* ÊäÈëÔöÒæ20db*/
    acodec_anareg1.ul32 = 0x11404004;//ACODEC_ANAREG1_DEFAULT
    ACODEC_HAL_WriteReg(ACODEC_ANAREG1_ADDR, acodec_anareg1.ul32);
  
    /* ×óÓÒÉùµÀÊ¹ÄÜ,*/
    acodec_digctrl0.ul32   = 0xff035144;
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL0_ADDR, acodec_digctrl0.ul32);

    /*Ò²¿É0x00000001,¼´²»¿ªÆôsoft mute,²»¸üÐÂÒôÁ¿£¬²»ÉèÖÃÈ¥¼ÓÖØÂË²¨Æ÷*/
    acodec_digctrl1.ul32 = 0x08000001;
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL1_ADDR, acodec_digctrl1.ul32);
    
    /*ÉèÖÃ×îÐ¡ÒôÁ¿*/
    acodec_digctrl2.ul32 = 0x1b1b2424;   /*ÒôÁ¿×îÐ¡*/ 
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL2_ADDR, acodec_digctrl2.ul32);
    
    /*Ò²¿É0x1E1E0001£¬¼´²»¿ªÆô¸ßÍ¨ÂË²¨Æ÷*/
    acodec_digctrl3.ul32 = 0x1e1ec001;
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL3_ADDR, acodec_digctrl3.ul32);


    return 0;

}
int acodec_i2s_set(AUDIO_SAMPLE_RATE_E enSample)
{
    unsigned int i2s_fs_sel = 0;
    ACODEC_DIGCTRL0_U    unDigctrl0;

    if ((AUDIO_SAMPLE_RATE_8000 == enSample)
        || (AUDIO_SAMPLE_RATE_11025 == enSample)
        || (AUDIO_SAMPLE_RATE_12000 == enSample)) 
    {
        i2s_fs_sel = 0x18;
    } 
    else if ((AUDIO_SAMPLE_RATE_16000 == enSample)
        || (AUDIO_SAMPLE_RATE_22050 == enSample)
        || (AUDIO_SAMPLE_RATE_24000 == enSample)) 
    {
        i2s_fs_sel = 0x19;
    } 
    else if ((AUDIO_SAMPLE_RATE_32000 == enSample)
        || (AUDIO_SAMPLE_RATE_44100 == enSample)
        || (AUDIO_SAMPLE_RATE_48000 == enSample)) 
    {
        i2s_fs_sel = 0x1a;
    } 
    else 
    {
        printf("%s: not support enSample:%d\n", __FUNCTION__, enSample);
        return -1;
    }

    unDigctrl0.ul32 = ACODEC_HAL_ReadReg(ACODEC_DIGCTRL0_ADDR);
    unDigctrl0.bits.i2s1_fs_sel = i2s_fs_sel;
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL0_ADDR, unDigctrl0.ul32);

    return 0;
}

int acodec_device_init(void)
{
    ACODEC_DIGCTRL0_U   acodec_digctrl0; 
    ACODEC_DIGCTRL1_U   acodec_digctrl1;
    ACODEC_DIGCTRL2_U   acodec_digctrl2;
    
    ACODEC_ANAREG0_U   acodec_anareg0;
    /*ACODEC_ANAREG1_U   acodec_anareg1;*/
    ACODEC_ANAREG2_U   acodec_anareg2;
    ACODEC_ANAREG3_U   acodec_anareg3;

    unsigned int aiao_crg = 0;
    unsigned int aiao_cfg = 0;
    
    u32AcodecBase = (unsigned int)IO_ADDRESS(ACODEC_REGS_BASE);
    if ( 0 == u32AcodecBase) {
        printf("could not ioremap acodec regs!");
        return -1;
    }
    ACODEC_HAL_WriteReg(ACODEC_ANAREG56_ADDR, 0xD);
    /* µÍ¹¦ºÄ²ßÂÔ£¬ÔÚ¼ÓÔØacodecÊ±³·Ïú¸´Î» */
    g_acodec_crg_reg = (void *)IO_ADDRESS(ACODEC_REGS_CRG);
    if (NULL == g_acodec_crg_reg) {
        printf("could not ioremap acodec regs!");
        return -1;
    }

    acodec_regsetbit(0 , 0, (unsigned long)g_acodec_crg_reg);

    acodec_digctrl2.ul32 = 0x7f7f2424;   /*ÒôÁ¿×îÐ¡*/ 
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL2_ADDR, acodec_digctrl2.ul32);
    
    aiao_crg = (unsigned int)IO_ADDRESS(AIAO_CLK_TX0_CRG);
    *(volatile unsigned int*)((unsigned int)(aiao_crg)) = 0x00100000;
    aiao_cfg = (unsigned int)IO_ADDRESS(AIAO_CLK_TX0_CFG);
    *(volatile unsigned int*)((unsigned int)(aiao_cfg)) = 0x00000115;
    udelay(50*1000);

    acodec_anareg0.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG0_ADDR);
    /*acodec_anareg1.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG1_ADDR);*/
    acodec_anareg2.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG2_ADDR);
    acodec_anareg3.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG3_ADDR);

    /* ×óÓÒÉùµÀÊ¹ÄÜ,*/
    acodec_digctrl0.ul32   = 0xff035144;
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL0_ADDR, acodec_digctrl0.ul32);

    /*Ò²¿É0x00000001,¼´²»¿ªÆôsoft mute,²»¸üÐÂÒôÁ¿£¬²»ÉèÖÃÈ¥¼ÓÖØÂË²¨Æ÷*/
    acodec_digctrl1.ul32 = 0x08000001;
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL1_ADDR, acodec_digctrl1.ul32);

    acodec_digctrl2.ul32 = 0x7e7e2424;
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL2_ADDR, acodec_digctrl2.ul32);

    /*POP_RES_SEL<1:0>=01£¬POP_RAMPCLK_SEL<1£º0>=01*/
    acodec_anareg3.bits.acodec_popres_sel = 0x1;
    acodec_anareg3.bits.acodec_poprampclk_sel = 3;    
    ACODEC_HAL_WriteReg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    acodec_anareg2.bits.acodec_vref_sel = 0x0;//0x4
    acodec_anareg2.bits.acodec_sel_clk_chop_adc_ph = 0x0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);

    /* mute popfree */
    acodec_anareg0.bits.acodec_mute_dacr = 1;
    acodec_anareg0.bits.acodec_mute_dacl = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);    

    /* direct 0 */
    acodec_anareg3.bits.acodec_dacl_pop_direct = 0;
    acodec_anareg3.bits.acodec_dacr_pop_direct = 0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);
    
    /* power on: vref, micbias, ctcm */
    acodec_anareg0.bits.acodec_pd_micbias1 = 0;
    acodec_anareg0.bits.acodec_pdb_ctcm_ibias = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_anareg2.bits.acodec_pd_ctcm = 0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);

    acodec_anareg0.bits.acodec_popfreer = 1;//0;
    acodec_anareg0.bits.acodec_popfreel = 1;//0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_anareg0.bits.acodec_pd_vref = 0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    /* direct 1 */
    acodec_anareg3.bits.acodec_dacl_pop_direct = 1;
    acodec_anareg3.bits.acodec_dacr_pop_direct = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    udelay(1000*1000);//

    /* dac power on */
    acodec_anareg0.bits.acodec_pd_dacl = 0;
    acodec_anareg0.bits.acodec_pd_dacr = 0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_anareg0.bits.acodec_mute_dacl = 0;
    acodec_anareg0.bits.acodec_mute_dacr = 0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_anareg0.bits.acodec_popfreer = 0;//1;
    acodec_anareg0.bits.acodec_popfreel = 0;//1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_soft_reset();

    printf("acodec inited!\n");
    
    return 0;
}       
       
int acodec_device_exit(void)
{  
    ACODEC_DIGCTRL2_U  acodec_digctrl2;

    ACODEC_ANAREG0_U   acodec_anareg0;
    ACODEC_ANAREG1_U   acodec_anareg1;
    ACODEC_ANAREG2_U   acodec_anareg2;
    ACODEC_ANAREG3_U   acodec_anareg3;

    unsigned int aiao_crg = 0;
    unsigned int aiao_cfg = 0;
     
    //VALG_TimerDelete(&g_AcodecTimer);
    
    acodec_regsetbit(0 , 0, (unsigned long)g_acodec_crg_reg);
    acodec_digctrl2.ul32 = 0x7f7f2424;   /*ÒôÁ¿×îÐ¡*/ 
    ACODEC_HAL_WriteReg(ACODEC_DIGCTRL2_ADDR, acodec_digctrl2.ul32);

    aiao_crg = (unsigned int)IO_ADDRESS(AIAO_CLK_TX0_CRG);
    *(volatile unsigned int*)((unsigned int)(aiao_crg)) = 0x00100000;//0x00200000;//0x001D6666;//0x00155555;//0x00100000;
    aiao_cfg = (unsigned int)IO_ADDRESS(AIAO_CLK_TX0_CFG);
    *(volatile unsigned int*)((unsigned int)(aiao_cfg)) = 0x00000115;
    udelay(50*1000);

    acodec_anareg0.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG0_ADDR);
    acodec_anareg1.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG1_ADDR);
    acodec_anareg2.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG2_ADDR);
    acodec_anareg3.ul32 = ACODEC_HAL_ReadReg(ACODEC_ANAREG3_ADDR);

    /*POP_RES_SEL<1:0>=01£¬POP_RAMPCLK_SEL<1£º0>=01*/
    acodec_anareg3.bits.acodec_popres_sel = 0x3;
    acodec_anareg3.bits.acodec_poprampclk_sel = 0x3;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);
    
    /* ±£³Ödirect 1*/
    acodec_anareg3.bits.acodec_dacl_pop_direct = 1;
    acodec_anareg3.bits.acodec_dacr_pop_direct = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);
    
    /* mute popfree on*/
    acodec_anareg0.bits.acodec_mute_dacr = 1;
    acodec_anareg0.bits.acodec_mute_dacl = 1;
    acodec_anareg0.bits.acodec_popfreer = 1;
    acodec_anareg0.bits.acodec_popfreel = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);
    
    acodec_anareg0.bits.acodec_pd_dacl = 1;
    acodec_anareg0.bits.acodec_pd_dacr = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);
    
    /* ±£³Ödirect 0*/
    acodec_anareg3.bits.acodec_dacl_pop_direct = 0;
    acodec_anareg3.bits.acodec_dacr_pop_direct = 0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG3_ADDR, acodec_anareg3.ul32);

    /* power on: vref, micbias, ctcm */  
    acodec_anareg0.bits.acodec_pd_vref = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    acodec_anareg2.bits.acodec_pd_ctcm = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG2_ADDR, acodec_anareg2.ul32);

    acodec_anareg0.bits.acodec_pd_micbias1 = 1;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG0_ADDR, acodec_anareg0.ul32);

    //input mod reset - Ïû³ýÅÜÒµÎñºó³õ´ÎÉÏµçµÄpop
    acodec_anareg1.bits.acodec_linein_l_sel = 0;
    acodec_anareg1.bits.acodec_linein_r_sel = 0;
    ACODEC_HAL_WriteReg(ACODEC_ANAREG1_ADDR, acodec_anareg1.ul32);

    printf("acodec exited!\n");
    return 0;
}


