#ifndef __IDE_PLATFORM_H_
#define __IDE_PLATFORM_H__

#if (defined __UITRON || defined __ECOS)
#include <string.h>
#include "DrvCommon.h"
#include "interrupt.h"
#include "Utility.h"
#include "dma.h"
#include "SxCmd.h"
#include "top.h"
#include "nvtDrvProtected.h"
#include "kernel.h"
#include "DxSys.h"
#include "pll.h"
#include "pll_protected.h"
#include "Perf.h"
#include "ide.h"
#elif defined __FREERTOS
#include <string.h>
#include <stdlib.h>
#include <kwrap/semaphore.h>
#include <kwrap/flag.h>
#include <kwrap/spinlock.h>
#include <kwrap/nvt_type.h>
#include <kwrap/error_no.h>
#include <kwrap/task.h>
#include "rcw_macro.h"
#include "io_address.h"
#include "interrupt.h"
#include "pll.h"
#include "pll_protected.h"
#include "ide.h"
#include "ide_dbg.h"
#include "comm/timer.h"
#include "kdrv_videoout/kdrv_vdoout.h"
#include "top.h"
#include "dma_protected.h"
#include "nvt-sramctl.h"
#define _EMULATION_             (0)	// no emu macro in linux
// manually defined here
#define FLGPTN_IDE              FLGPTN_BIT(0)
#define FLGPTN_IDE_2            FLGPTN_BIT(1)
#define FLGPTN_IDE_3            FLGPTN_BIT(2)
#define FLGPTN_IDE_4            FLGPTN_BIT(3)
#define FLGPTN_IDE_5            FLGPTN_BIT(4)
#define FLGPTN_IDE_DMA_DONE     FLGPTN_BIT(5)

#define FLGPTN_IDE2              FLGPTN_BIT(0)
#define FLGPTN_IDE2_2            FLGPTN_BIT(1)
#define FLGPTN_IDE2_3            FLGPTN_BIT(2)
#define FLGPTN_IDE2_4            FLGPTN_BIT(3)
#define FLGPTN_IDE2_5            FLGPTN_BIT(4)
#define FLGPTN_IDE2_DMA_DONE     FLGPTN_BIT(5)


//extern UINT32 IOADDR_IDE2_REG_BASE;
#else
//#include <rcw_macro.h>
#include <common.h>
#include <asm/arch/IOAddress.h>
#include <asm/nvt-common/rcw_macro_bit.h>
//#include <linux/spinlock.h>
//#include "kwrap/type.h"//a header for basic variable type
//#include "kwrap/flag.h"
//#include "kwrap/semaphore.h"
//#include "kwrap/task.h"
//#include <linux/soc/nvt/fmem.h>
//#include <nvt-sramctl.h>
//#include <linux/delay.h>
//#include <linux/clk.h>
//#include <top.h>
//#include "ide_drv.h"
//#include "ide_dbg.h"
#include <asm/arch/nvt_ide.h>
//#include "comm/drvdump.h"
//#include "kdrv_videoout/kdrv_vdoout.h"
#include <asm/arch/kdrv_vdoout.h>
typedef unsigned int        	FLGPTN;                     ///< Flag patterns
typedef unsigned int        	*PFLGPTN;                   ///< Flag patterns (Pointer)
#define FLGPTN_BIT(n)       	((FLGPTN)(1 << (n)))        ///< Bit of flag pattern
typedef unsigned long           ULONG;

#define _EMULATION_             (0)	// no emu macro in linux
// manually defined here
#define FLGPTN_IDE              FLGPTN_BIT(0)
#define FLGPTN_IDE_2            FLGPTN_BIT(1)
#define FLGPTN_IDE_3            FLGPTN_BIT(2)
#define FLGPTN_IDE_4            FLGPTN_BIT(3)
#define FLGPTN_IDE_5            FLGPTN_BIT(4)
#define FLGPTN_IDE_DMA_DONE     FLGPTN_BIT(5)

#define FLGPTN_IDE2              FLGPTN_BIT(0)
#define FLGPTN_IDE2_2            FLGPTN_BIT(1)
#define FLGPTN_IDE2_3            FLGPTN_BIT(2)
#define FLGPTN_IDE2_4            FLGPTN_BIT(3)
#define FLGPTN_IDE2_5            FLGPTN_BIT(4)
#define FLGPTN_IDE2_DMA_DONE     FLGPTN_BIT(5)
#define dma_getNonCacheAddr(addr) addr

//extern IDEDATA_TYPE IOADDR_IDE_REG_BASE;
//extern IDEDATA_TYPE IOADDR_IDE2_REG_BASE;
#endif

/*
    @name   IDE clock divider

    IDE clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV).
*/
//@{
#define PLL_IDE_CLKDIV(x)           ((x) << (PLL_CLKSEL_IDE_CLKDIV - PLL_CLKSEL_R15_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV)
//@}

/*
    @name   IDE Output Interface clock divider

    IDE Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_IDE_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE_OUTIF_CLKDIV)
//@}

/*
    @name   IDE2 clock divider

    IDE2 clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE_CLKDIV).
*/
//@{
#define PLL_IDE2_CLKDIV(x)           ((x) << (PLL_CLKSEL_IDE2_CLKDIV - PLL_CLKSEL_R15_OFFSET))     //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE2_CLKDIV)
//@}

/*
    @name   IDE2 Output Interface clock divider

    IDE2 Output Interface clock divider

    @note This if for pll_set_clock_rate(PLL_CLKSEL_IDE2_OUTIF_CLKDIV).
*/
//@{
#define PLL_IDE2_OUTIF_CLKDIV(x)          ((x) << (PLL_CLKSEL_IDE2_OUTIF_CLKDIV - PLL_CLKSEL_R15_OFFSET))    //< Used for pll_set_clock_rate(PLL_CLKSEL_IDE2_OUTIF_CLKDIV)
//@}

#define IDE_SETREG(ofs, value)   OUTW(IOADDR_IDE_REG_BASE + (ofs), value)
#define IDE_GETREG(ofs)         INW(IOADDR_IDE_REG_BASE + (ofs))
#define IDE2_SETREG(ofs, value)  OUTW(IOADDR_IDE2_REG_BASE + (ofs), value)
#define IDE2_GETREG(ofs)        INW(IOADDR_IDE2_REG_BASE + (ofs))
//#define TV_SETREG(ofs, value)    OUTW(IOADDR_TV_REG_BASE + (ofs), value)
//#define TV_GETREG(ofs)          INW(IOADDR_TV_REG_BASE + (ofs))

//extern ER ide_platform_flg_clear(IDE_ID id, FLGPTN flg);
//extern ER ide_platform_flg_set(IDE_ID id, FLGPTN flg);
extern ER ide_platform_flg_wait(IDE_ID id, FLGPTN flg);
//extern ER ide_platform_sem_set(IDE_ID id);
//extern ER ide_platform_sem_wait(IDE_ID id);
extern ULONG ide_platform_spin_lock(IDE_ID id);
extern void ide_platform_spin_unlock(IDE_ID id, ULONG flag);
//extern void ide_platform_sram_enable(IDE_ID id);
//extern void ide_platform_sram_disable(IDE_ID id);
//extern void ide_platform_int_enable(IDE_ID id);
//extern void ide_platform_int_disable(IDE_ID id);
extern void ide_platform_delay_ms(UINT32 ms);
extern void ide_platform_delay_us(UINT32 us);
extern IDEDATA_TYPE ide_platform_va2pa(IDEDATA_TYPE addr);
extern UINT32 factor_caculate(UINT16 x, UINT16 y, BOOL h);
//extern PINMUX_LCDINIT ide_platform_get_disp_mode(UINT32 pin_func_id);
#if !(defined __UITRON || defined __ECOS)
#if defined __FREERTOS
//extern void ide_platform_create_resource(IDE_ID id);
//extern void ide_platform_release_resource(IDE_ID id);

#else
//extern int DBG_ERR(const char *fmt, ...);
//extern int DBG_WRN(const char *fmt, ...);
//extern int DBG_IND(const char *fmt, ...);

//extern void ide2_isr(void);
//extern void ide_platform_create_resource(MODULE_INFO *pmodule_info);
//extern void ide_platform_release_resource(void);
//extern PINMUX_LCDINIT ide_platform_get_disp_mode(UINT32 pin_func_id);
#endif
extern void ide_isr(void);
extern void ide2_isr(void);
extern void idec_parsing_interrupt(IDE_ID id);

#endif
extern void idec_isr_bottom(IDE_ID id, UINT32 events);
extern BOOL ide_platform_list_empty(IDE_ID id);
extern ER ide_platform_add_list(IDE_ID id, KDRV_CALLBACK_FUNC *p_callback);
extern struct _IDE_REQ_LIST_NODE* ide_platform_get_head(IDE_ID id);
extern ER ide_platform_del_list(IDE_ID id);

extern void ide_platform_set_ist_event(IDE_ID id);
extern int  ide_platform_ist(IDE_ID id, UINT32 event);


#endif
