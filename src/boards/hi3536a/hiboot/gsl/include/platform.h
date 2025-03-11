/*
 *
 * Copyright (c) 2020-2021 Shenshu Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __PLATFORM_VENDORLICON_H__
#define __PLATFORM_VENDORLICON_H__

#define reg_get(addr) (*(volatile unsigned int *)(uintptr_t)(addr))
#define reg_set(addr, val) (*(volatile unsigned int *)(uintptr_t)(addr) = (val))

#define SRAM_BASE	        CFG_RAM_BASE_ADDR
#define SRAM_SIZE	        0x1a000
#define SRAM_END	        (SRAM_BASE + SRAM_SIZE)

#define STACK_SIZE		0xa00
#define STACK_ADDR		(SRAM_BASE + STACK_SIZE)
#define CP_STEP1_ADDR		STACK_ADDR
#define CP_STEP1_SIZE		0x14000

/* Sys ctrl register */
#define REG_SYSCTRL_BASE	0x11020000

/* GSL security reinforcement */
#define MALLOC_START		STACK_ADDR
#define MALLOC_SIZE 		0x3e00
#define GSL_LATER_CODE_ADDR (MALLOC_START + CP_STEP1_SIZE + MALLOC_SIZE) /* refer to malloc_size.h */

/* Vendor_Root_Public_Key\OEM_Root_Public_Key\Third_Party_Root_Public_Key\GSL_Third_Party_Key\GSL_Key */

#define VENDOR_ROOT_PUBLIC_KEY_ADDR	CP_STEP1_ADDR		/* The start address of vendor_Root_Public_Key */

#define OEM_ROOT_PUBLIC_KEY_ADDR	(VENDOR_ROOT_PUBLIC_KEY_ADDR + 0x400)	/* The start address of vendor_Root_Public_Key */
#define GSL_THIRD_PARTY_KEY_ADDR  (VENDOR_ROOT_PUBLIC_KEY_ADDR + 0xc00)
#define GSL_KEY_AREA_ADDR  (VENDOR_ROOT_PUBLIC_KEY_ADDR + 0x1200)
#define GSL_CODE_AREA_ADDR  (VENDOR_ROOT_PUBLIC_KEY_ADDR + 0x1800)
/*-----------------------------------------------------------------
 * sec bootloader flash mapping addr
 ------------------------------------------------------------------ */
#define BOOTLOADER_KEY_AREA_ADDR_OFFSET (0x1800)
#define BOOTLOADER_PARAMS_AREA_ADDR_OFFSET (0x1e00)
#define BOOTLOADER_CFG_PARAMS_OFFSET (0x1ea0)
#define BOOTLOADER_BOOT_CODE_ID_OFFSET (0x20f0 + CFG_PARAM_SIZE)
#define BOOTLOADER_BOOT_CODE_OFFSET (0x2100 + CFG_PARAM_SIZE)
#define BOOTLOADER_BOOT_HEAD_SIZE (0x20)
#define BOOTLOADER_KEY_PARAMS_AREA_SIZE (0x8b0 + CFG_PARAM_SIZE + 0x4) /* 0x4: SCS_simulate_flag */
#define BOOTLOADER_OEM_MSID_EXT_OFFSET  (0x20e0 + CFG_PARAM_SIZE)
/*-----------------------------------------------------------------
 * sysctrl register
 ------------------------------------------------------------------ */
#define REG_BASE_SCTL		0x11020000
#define REG_SC_CTRL		0x0000
#define TIMEREN0_SEL		(1 << 16)
#define REG_SC_SYSRES		0x0004
#define OS_OTP_PO_SDIO_INFO	0x0408
#define REG_SC_SYSSTAT		0x0018
#define REG_SAVE_EMMC_OCR	0x0300
#define REG_OS_SYS_CTRL1	0x304
#define REG_PERI_EMMC_STAT	0x0404
#define EMMC_NORMAL_MODE	(0x1 << 3)
#define uart_boot_flag(x)	(((x) >> 4) & 0x1)
#define EMMC_BOOT_8BIT		(0x1 << 11)
#define SEC_BOOTRAM_CTRL        0x6080
#define BOOTRAM_RAM_SEL        (0x1 << 0)

#define REG_SC_GEN1		0x0134
#define REG_SC_GEN3		0x013C
#define REG_SC_GEN4		0x0140

#define PCIE_SLAVE_BOOT_CTL_REG     REG_SC_GEN1
#define DDR_INIT_EXCUTE_OK_FLAG     0xDCEFF002 /* step2:Ddrinit Code Excute Finished Flag:    DCEFF002 */
#define UBOOT_DOWNLOAD_OK_FLAG      0xBCDFF003 /* step3:Boot Code Download Finished Flag:     BCDFF003 */
#define READ_FOR_HEAD_AREA_FLAG     0xACBFF005
#define HEAD_AREA_OK_FLAG           0xACBFF006
#define PCIE_SLAVE_PRINT_INTERVAL	10000

#define REG_TEE_LAUNCH_FAILURE	(REG_BASE_SCTL + REG_OS_SYS_CTRL1)

/*-----------------------------------------------------------------
 * CRG register
 ------------------------------------------------------------------ */
#define REG_BASE_CRG		0x11010000
#define REG_PERI_CRG4192	0x4180
#define UART0_CLK_ENABLE	(0x1 << 4)
#define UART0_CLK_MASK		(0x3 << 12)
#define UART0_CLK_24M		(0x1 << 12)
#define UART0_SRST_REQ		(0x1 << 0)

/*-----------------------------------------------------------------
 * CA_MISC register
 *------------------------------------------------------------------ */
#define REG_BASE_CA_MISC	0x10115000
#define SCS_CTRL		0x400

/*-----------------------------------------------------------------
 * serial base address and clock
 ------------------------------------------------------------------ */
#define REG_BASE_SERIAL0	0x11040000
#define CONFIG_PL011_CLOCK	24000000	/* Serial need 24M clock input */
#define CONFIG_CONS_INDEX	0		/* select the default console */

/*-----------------------------------------------------------------
 * timer0 register
 ------------------------------------------------------------------ */
#define REG_BASE_TIMER0		0x11000000
#define TIMER_LOAD		0x0000
#define TIMER_VALUE		0x0004
#define TIMER_CONTROL		0x0008
#define TIMER_EN		(1<<7)
#define TIMER_MODE		(1<<6)
#define TIMER_PRE		(0<<2)
#define TIMER_SIZE		(1<<1)

#define CFG_TIMERBASE		REG_BASE_TIMER0
#define READ_TIMER              (*(volatile unsigned int *)(CFG_TIMERBASE + TIMER_VALUE))
#define TIMER_FEQ		(3000000)
/* how many ticks per second. show the precision of timer. */
#define TIMER_DIV		(1)
#define CONFIG_SYS_HZ		(TIMER_FEQ / TIMER_DIV)

/*-----------------------------------------------------------------
 * boot   Configuration
 ------------------------------------------------------------------ */

#define SPI_BASE_ADDR			0x0f000000
#define DDR_BASE_ADDR			0x40000000

#define DDR_DOWNLOAD_ADDR		0x41000000

#define SELF_BOOT_DOWNLOAD_MAGIC    (0x444f574e)

#define BACKUP_IMAGE_FLG_REG		(0xC8)
#define BACKUP_IMAGE_OFF_REG		(0xCC)
#define BACKUP_IMAGE_TIMES_REG		(0x314)
#define BACKUP_IMAGE_ADDR_REG		(0x318)
#define DATA_CHANNEL_TYPE_REG		(0x31c)
#define BACKUP_IMAGE_ENABLE			(0x424945)  /* Backup Image Enable */
#define BACKUP_IMAGE_FLG_BIT		4
#define BACKUP_IMAGE_FLG_MASK		(0xffffff << BACKUP_IMAGE_FLG_BIT)
#define BACKUP_IMAGE_OFF_BIT		4
#define BACKUP_IMAGE_OFF_MASK		(0xfff << BACKUP_IMAGE_OFF_BIT)
#define get_backup_image_flag(val)	((val & BACKUP_IMAGE_FLG_MASK) >> BACKUP_IMAGE_FLG_BIT)

/*
 * AHB_IOCFG
 */
#define REG_AHB_IO_CFG_BASE		0x102f0000

#define UART0_RXD_IOCFG_OFST		0x124
#define UART0_TXD_IOCFG_OFST		0x128

/* SFC_IOCFG */
#define REG_HP_IO_CFG_BASE		0x10230000

/* EMMC_IOCFG */
#define IO_CFG_EMMC_CLK			0x00
#define IO_CFG_EMMC_CMD			0x04
#define IO_CFG_EMMC_DATA0		0x08
#define IO_CFG_EMMC_DATA1		0x0C
#define IO_CFG_EMMC_DATA2		0x10
#define IO_CFG_EMMC_DATA3		0x14
#define IO_CFG_EMMC_DATA4		0x18
#define IO_CFG_EMMC_DATA5		0x1C
#define IO_CFG_EMMC_DATA6		0x20
#define IO_CFG_EMMC_DATA7		0x24
#define IO_CFG_EMMC_DS			0x28
#define IO_CFG_EMMC_RST			0x2C

#define REG_CRG_BASE			REG_BASE_CRG
#define REG_BASE_EMMC			0x10020000

#define PERI_CRG_EMMC			0x34c0
#define  MMC_RST			(0x1 << 16)
#define  MMC_CLK_EN			(0x1 << 0)
#define  MMC_AHB_CLK_EN		(0x1 << 1)
#define  MMC_CLK_SEL_MASK		(0x7 << 24)
#define  MMC_CLK_SEL_SHIFT		24
#define  MMC_CLK_400K			0x0
#define  MMC_CLK_25M			0x1
#define  MMC_CLK_50M			0x2
#define PERI_EMMC_DRV_DLL		0x34c8
#define  MMC_DRV_PHASE_MASK		(0x1f << 15)

#define  READ_DATA_BY_CPU		0x01
#define  READ_DATA_BY_DMA		0x02

/* emmc sdio macro */
#define mmc_clk_sel_val(reg_val)	((reg_val) & 0x7)
#define sdio_pad_pu(reg_val)            ((reg_val >> 0) & 0x1)

/*-----------------------------------------------------------------------------------
 * rng_gen register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_TRNG               0x10114000
#define SEC_COM_TRNG_FIFO_DATA    (REG_BASE_TRNG + 0x204)
#define SEC_COM_TRNG_DATA_ST      (REG_BASE_TRNG + 0x208)

#define SECURE_BOOT_OVER_TIME       200	/* 200 ms */

/*-----------------------------------------------------------------------------------
 * usb register
 *-----------------------------------------------------------------------------------*/
#define USB_P0_REG_BASE			0x10320000

/*-----------------------------------------------------------------------------------
 * otp user interface register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_OTP_USER_IF	0x10120000

#define OTP_USER_OP_START		(REG_BASE_OTP_USER_IF + 0x2004)
#define OTP_USER_WORK_MODE_TEE	(REG_BASE_OTP_USER_IF + 0x3000)
#define OTP_USER_REV_ADDR_TEE 	(REG_BASE_OTP_USER_IF + 0x3038)
#define OTP_USER_REV_RDATA_TEE 	(REG_BASE_OTP_USER_IF + 0x3040)
#define OTP_USER_CTRL_STA_TEE	(REG_BASE_OTP_USER_IF + 0x304c)

#define OTP_USER_LOCKABLE0	(REG_BASE_OTP_USER_IF + 0x2058)
#define OTP_USER_LOCKABLE5	(REG_BASE_OTP_USER_IF + 0x206c)
#define OTP_USER_LOCKABLE8	(REG_BASE_OTP_USER_IF + 0x2078)
#define OTP_USER_LOCKABLE9	(REG_BASE_OTP_USER_IF + 0x207c)
#define OTP_USER_LOCKABLE10	(REG_BASE_OTP_USER_IF + 0x2080)
#define OTP_USER_LOCKABLE15	(REG_BASE_OTP_USER_IF + 0x2094)
#define OTP_USER_LOCKABLE17 (REG_BASE_OTP_USER_IF + 0x209C)

#define OTP_USER_ONEWAYO        (REG_BASE_OTP_USER_IF + 0x2050)

#define TEE_OWNER_IS_VENDOR	0x1
#define TEE_OWNER_IS_OEM	0x0

/*-----------------------------------------------------------------------------------
 * otp read only register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_OTP_READ_ONLY 0x11020000
#define OTP_PO_INFO_32        (REG_BASE_OTP_READ_ONLY + 0x1550)

/*-----------------------------------------------------------------------------------
 * dbc_apb register
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_DBC_APB 	0x10113000
#define DBC_APB_JTAG_DBG_INFO  (REG_BASE_DBC_APB + 0x0d4)

/*-----------------------------------------------------------------------------------
 * boot sel type
 *-----------------------------------------------------------------------------------*/
#define	BOOT_SEL_PCIE    0x965a4b87
#define	BOOT_SEL_UART    0x69a5b478
#define	BOOT_SEL_USB     0x965ab487
#define	BOOT_SEL_SDIO    0x69a54b87
#define	BOOT_SEL_FLASH   0x96a54b78
#define	BOOT_SEL_EMMC    0x695ab487
#define	BOOT_SEL_UNKNOW  0x965a4b78

/* SDIO_IOCFG */
#define REG_VDP_AIAO_IO_CFG_BASE        0x102f0000
#define SDIO0_CCLK_OUT_OFST             0x9c
#define SDIO0_CARD_DETECT_OFST          0x80
#define SDIO0_CARD_POWEN_EN_OFST        0x84
#define SDIO0_DATA0_OFST                0x8c
#define SDIO0_DATA1_OFST                0x90
#define SDIO0_DATA2_OFST                0x94
#define SDIO0_DATA3_OFST                0x98
#define SDIO0_CCMD_OFST                 0x88

#define MMC_DRV_PHASE_SHIFT             15
#define REG_BASE_SDIO0                  0x10030000

#define PERI_CRG_SDIO0                  0x35c0
#define PERI_SDIO0_DRV_DLL              0x35c8
#define PERI_CRG_SDIO0_CLK              0x210

/*-----------------------------------------------------------------------------------
 * Secure OS
 *-----------------------------------------------------------------------------------*/
#define OS_SYS_CTRL_REG2  (REG_SYSCTRL_BASE + 0x308)
#define OS_SYS_CTRL_REG4  (REG_SYSCTRL_BASE + 0x310)

#define SEC_BOOTRAM_SEC_CFG (REG_SYSCTRL_BASE + 0x6090)
#define SEC_BOOTRAM_RO_CFG (REG_SYSCTRL_BASE + 0x6094)
#define SEC_BOOTRAM_SEC_CFG_LOCK1 (REG_SYSCTRL_BASE + 0x6098)
#define SEC_BOOTRAM_RO_CFG_LOCK2 (REG_SYSCTRL_BASE + 0x609c)

/* kernel start addr - sizeof(header) */
#define KERNEL_LOAD_ADDR	0x8c080000
#define BL33_LOAD_ADDR    (KERNEL_LOAD_ADDR - 0x40)
#define FDT_LOAD_ADDR     (KERNEL_LOAD_ADDR + 0x1F80000)
#define BL31_BASE         (KERNEL_LOAD_ADDR + 0x2F80000)
#define BL32_LOAD_ADDR    (KERNEL_LOAD_ADDR + 0x3F80000)
#define BL33_EP_INFO_ADDR (KERNEL_LOAD_ADDR + 0x4F80000)
#define BL32_EP_INFO_ADDR (KERNEL_LOAD_ADDR + 0x4F81000)
#define BL31_PARAMS_ADDR  (KERNEL_LOAD_ADDR + 0x4F82000)

/*-----------------------------------------------------------------------------------
 * TZASC
 *-----------------------------------------------------------------------------------*/
#define SEC_MODULE_BASE					0x11141000
#define TZASC_BYPASS_REG_OFFSET			0x004
#define TZASC_RGN_MAP_REG_OFFSET		0x100
#define TZASC_RGN_MAP_EXT_REG_OFFSET	0x200
#define TZASC_RGN_ATTR_REG				0x104

#define TZASC_MAX_RGN_NUMS 				16
#define TZASC_RGN_1   					1
#define TZASC_RGN_2   					2
#define TZASC_RGN_8   					8

#define tzasc_rgn_offset(rgn)			(0x10 * (rgn))

/*-----------------------------------------------------------------------------------
 *  TEE Image
 *-----------------------------------------------------------------------------------*/
#define TEE_KEY_AREA_LOAD_ADDR          (KERNEL_LOAD_ADDR + 0x4F85000)
#define TEE_KEY_AREA_SIZE	             0x600
#define ATF_AREA_ADDR                   (TEE_KEY_AREA_LOAD_ADDR + TEE_KEY_AREA_SIZE)

#define BL32_BASE                       (BL31_BASE + 0x200000)
#define TEE_DEC_ADDR                    (BL31_BASE + 0x400000)
#define DDR_BASE                        0x40000000
#define DDR_SIZE                        0x200000000
#define DDR_END                         (DDR_BASE + DDR_SIZE)

/*-----------------------------------------------------------------------------------
 *  SVB
 *-----------------------------------------------------------------------------------*/
#define SYSCTRL_REG		0x11020000
#define HPM_CORE_VOL_REG	(SYSCTRL_REG + 0x9004)
#define HPM_CPU_VOL_REG		(SYSCTRL_REG + 0x9104)
#define CYCLE_NUM           5
#define HPM_CPU_REG0		0xb008
#define HPM_CPU_REG1		0xb00c
#define HPM_CORE_REG0		0xb018
#define HPM_CORE_REG1		0xb01c

#define CORE_HPM_MAX		500
#define CORE_HPM_MIN		300
#define CPU_HPM_MAX		    540
#define CPU_HPM_MIN		    340
#define CORE_HPM_TEMP_MIN	360
#define CPU_HPM_TEMP_MIN	395
/* tsensor */
#define REG_TSENSOR_CTRL	0x1102a000
#define TSENSOR_CTRL0		0x0
#define TSENSOR_CTRL0_CFG	0x80f00000
#define TSENSOR_CTRL1		0x4
#define TSENSOR_CTRL1_CFG	0x0
#define TSENSOR_STATUS0     0x8
#define TSENSOR_MAX_CHN     2
#define TSENSOR_OFFSET      0x100
#define TSENSOR_MASK        0x3ff
#define TSENSOR_CURVE_A		146
#define TSENSOR_CURVE_B		718
#define TSENSOR_CURVE_C		165
#define TSENSOR_CURVE_D		40


#define SYSCTRL_BASE_REG	        0x11020000
#define HPM_CORE_STORAGE_REG	    0x340
#define HPM_CPU_STORAGE_REG	        0x348
#define TEMPERATURE_STORAGE_REG     0x34c
#define DELTA_V_STORAGE_REG         0x350
#define CORE_DUTY_STORAGE_REG       0x354
#define CPU_DUTY_STORAGE_REG        0x35c


/* physical max/min */
#define CORE_VOLT_MAX		920
#define CORE_VOLT_MIN		600

#define CPU_VOLT_MAX		1050
#define CPU_VOLT_MIN		600


/* curve max/min; voltage curve:  */
#define CORE_CURVE_VLOT_MAX		842
#define CORE_CURVE_VLOT_MIN		766
#define CORE_CURVE_B			11937422
#define CORE_CURVE_A			9762

#define CPU_CURVE_VLOT_MAX	    960
#define CPU_CURVE_VLOT_MIN	    845
#define CPU_CURVE_B			    15120000
#define CPU_CURVE_A			    13595
/* curve max/min; CORE HPM curve: */
#define HPM_CORE_90_CURVE_A	            1722
#define HPM_CORE_90_CURVE_B	            575000
#define HPM_CORE_70_CURVE_A	            728
#define HPM_CORE_70_CURVE_B	            233000
#define HPM_CORE_50_CURVE_A	            560
#define HPM_CORE_50_CURVE_B	            195000
#define HPM_CORE_38_SUBZERO_CURVE_A	    523
#define HPM_CORE_38_SUBZERO_CURVE_B	    119000
#define HPM_CORE_24_SUBZERO_CURVE_A	    510
#define HPM_CORE_24_SUBZERO_CURVE_B	    132000


/* curve max/min; CPU HPM curve:   */

#define HPM_CPU_90_CURVE_A	            1085
#define HPM_CPU_90_CURVE_B	            318000
#define HPM_CPU_70_CURVE_A	            645
#define HPM_CPU_70_CURVE_B	            183000
#define HPM_CPU_50_CURVE_A	            308
#define HPM_CPU_50_CURVE_B	            90000
#define HPM_CPU_38_SUBZERO_CURVE_A	    889
#define HPM_CPU_38_SUBZERO_CURVE_B	    261000
#define HPM_CPU_24_SUBZERO_CURVE_A	    823
#define HPM_CPU_24_SUBZERO_CURVE_B	    259000


#define HPM_0_SCARE	            10000
#define TEMP_SCARE	            1024

#define HPM_HIGH_MASK	        0x3ff
#define HPM_HIGH_16	            16
#define HPM_CYCLE	            2
#define DELTA_MASK	            0x7
#define DELTA_SCARE	            10
#define DELTA_OFST	            8

#define SVB_VER_REG		        0x11020168
#define SVB_VER			        0x10
#define OTP_SHPM_MDA_OFFSET	    0x153c

#define temperature_formula(val)  (((((val) - 116) * 165) / 806) - 40)
#define duty_formula(max, min, val) ((((max) - (val)) * 416 + ((max) - (min) + 1) / 2) / ((max) - (min)) - 1)
#define volt_regval_formula(val) (((val) << 16) + ((416 - 1) << 4) + 0x5)


#define REG_BASE_MISC			0x11024000

#define DDR_SCRAMB_ENABLE
#ifdef DDR_SCRAMB_ENABLE
/*-----------------------------------------------------------------------------------
 *  TRNG
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_RNG_GEN		0x10114000
#define TRNG_DSTA_FIFO_DATA_REG		(REG_BASE_RNG_GEN + 0x204)
#define TRNG_DATA_ST_REG		(REG_BASE_RNG_GEN + 0x208)
#define TRNG_FIFO_DATA_CNT_SHIFT	0x8
#define TRNG_FIFO_DATA_CNT_MASK		0xf

/*-----------------------------------------------------------------------------------
 *  DDRC
 *-----------------------------------------------------------------------------------*/
#define REG_BASE_DDRC			0x11140000

#define DMC0_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0x8000 + 0x0)
#define DMC1_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0x9000 + 0x0)
#define DMC2_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0xa000 + 0x0)
#define DMC3_DDRC_CTRL_SREF_REG		(REG_BASE_DDRC + 0xb000 + 0x0)

#define DMC0_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0x8000 + 0x50)
#define DMC1_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0x9000 + 0x50)
#define DMC2_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0xa000 + 0x50)
#define DMC3_DDRC_CFG_DDRMODE_REG	(REG_BASE_DDRC + 0xb000 + 0x50)

#define DMC0_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0x8000 + 0x294)
#define DMC1_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0x9000 + 0x294)
#define DMC2_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0xa000 + 0x294)
#define DMC3_DDRC_CURR_FUNC_REG		(REG_BASE_DDRC + 0xb000 + 0x294)

#define DDRC_SREF_DONE			(0x1 << 1)
#define DDRC_SREF_REQ			(0x1 << 0)

/*-----------------------------------------------------------------------------------
 *  MISC DDRCA
 *-----------------------------------------------------------------------------------*/
#define DDRCA_REE_RANDOM_L_REG		(REG_BASE_MISC + 0x40)
#define DDRCA_REE_RANDOM_H_REG		(REG_BASE_MISC + 0x44)
#define DDRCA_TEE_RANDOM_L_REG		(REG_BASE_MISC + 0x48)
#define DDRCA_TEE_RANDOM_H_REG		(REG_BASE_MISC + 0x4C)
#define DDRCA_EN_REG			(REG_BASE_MISC + 0x50)
#define DDRCA_REE_UPDATE_REG		(REG_BASE_MISC + 0x54)
#define DDRCA_TEE_UPDATE_REG		(REG_BASE_MISC + 0x58)
#define DDRCA_LOCK_REG			(REG_BASE_MISC + 0x5c)
#define SCRAMB_BYPASS_DIS		0x5
#define SCRAMB_ENABLE			0x1
#define SCRAMB_LOCK			0x1
#endif

#define REG_OTP_PO_VDAC_INFO	(SYSCTRL_REG + 0x1500)
#define OTP_DYING_GASP_OFST	6
#define OTP_DYING_GASP_MASK	(0x3f << OTP_DYING_GASP_OFST)

#define REG_DYING_GASP_CFG0	(REG_BASE_MISC + 0x20)
#define DYING_GASP_OFST		20
#define DYING_GASP_MASK		(0x3f << DYING_GASP_OFST)

#include "serial_reg.h"
#endif  /* __PLATFORM_VENDORLICON_H__ */
