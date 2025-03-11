/*
 * (C) Copyright 2009, HISILICON
 * Configuation settings for the godbox board.
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

#include <asm/arch/platform.h>

#ifndef __CONFIG_HI3798CV2X_H__
#define __CONFIG_HI3798CV2X_H__
#define CONFIG_ARCH_HI3798CV2X

/* Arm configuration*/
#define CONFIG_L2_OFF

#ifndef CONFIG_SDKVERSION
#  define CONFIG_SDKVERSION               "0.1.0.1 - HiFone B02"
#endif

//zbing add
#define MS_UBOOT_VERSION	"72.1.2.22"
#define MS_DDR_VERSION	2
//end
/*-----------------------------------------------------------------------
 * for cpu/arm_cortexa9/start.S
 -----------------------------------------------------------------------*/
#define ROM_TEXT_ADRS       (MEM_BASE_FLASH)
#define NAND_TEXT_ADRS      (MEM_BASE_NAND)
#define SF_TEXT_ADRS        (MEM_BASE_SPI)

#define CONFIG_SYS_MALLOC_LEN      (0x600000)
#define CONFIG_SYS_GBL_DATA_SIZE   128  /* size in bytes reserved for initial data */

#include "hi3798cv2x_config.h"

#undef CONFIG_BOOT_SIMULATE

#ifdef CONFIG_BOOT_SIMULATE
#define step(x)  writel(x, REG_BASE_SCTL + REG_SC_GEN31)
#undef CFG_HI3798CV2X_FPGA
#else
#define step(x)
#endif

#if CFG_HI3798CV2X_FPGA
#  define CONFIG_HI3798CV2X_FPGA
#else
#  undef CONFIG_HI3798CV2X_FPGA
#endif

#  undef CONFIG_HI3798CV2X_FPGA

#define CONFIG_OF_LIBFDT
#define CONFIG_SYS_BOOTMAPSZ               (48 << 20)       /* Initial Memory map for Linux */
#define CONFIG_LMB
/*-----------------------------------------------------------------------
 * for timer configuration (udelay)
*  enable timer
 -----------------------------------------------------------------------*/
#define CFG_TIMERBASE           REG_BASE_TIMER01
#define CFG_TIMER_CTRL          0xCA
#define READ_TIMER              \
	(*(volatile unsigned long *)(CFG_TIMERBASE + REG_TIMER_VALUE))
/*-----------------------------------------------------------------------
 * environment && bd_info  && global_data  configure
*     used in file
-----------------------------------------------------------------------*/

#ifndef CFG_ENV_SIZE
#  define CFG_ENV_SIZE                  0x2000
#endif

#ifndef CFG_ENV_ADDR
#  define CFG_ENV_ADDR                  0x80000
#endif

//zbing delete
//#ifndef CFG_ENV_RANGE
//#  define CFG_ENV_RANGE                 CFG_ENV_SIZE
//#endif
//end
#ifdef CONFIG_ENV_BACKUP
#  define CONFIG_ENV_BACKUP_ADDR        CFG_ENV_BACKUP_ADDR
#endif

#define  CONFIG_ENV_OVERWRITE
//zbing delete
#if 0
#define CONFIG_ENV_OFFSET               CFG_ENV_ADDR/* environment starts here  */
#define CONFIG_ENV_NAND_ADDR            CFG_ENV_ADDR
#define CONFIG_ENV_SPI_ADDR             CFG_ENV_ADDR
#define CONFIG_ENV_ADDR                 CFG_ENV_ADDR
#define CONFIG_ENV_RANGE                CFG_ENV_RANGE
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_SIZE                 CFG_ENV_SIZE/*include ENV_HEADER_SIZE */
#define CONFIG_ENV_SECT_SIZE            CONFIG_ENV_SIZE
#endif
//end

//zbing add
#define CONFIG_ENV_OFFSET		    0xd0000 /* 832Kb uboot, 64KB env.   environment starts here */
#define CONFIG_ENV_NAND_ADDR		(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SPI_ADDR		    (CONFIG_ENV_OFFSET)
#define CONFIG_CMD_SAVEENV
#define CONFIG_ENV_SIZE			    0x10000 /*include ENV_HEADER_SIZE */
#define CONFIG_ENV_SECT_SIZE		CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	    CONFIG_ENV_SIZE

//end

#define CONFIG_NR_DRAM_BANKS            1/* we have 1 bank of DRAM */
#define CFG_BOOT_PARAMS                 (MEM_BASE_DDR + 0x0100)

/*-----------------------------------------------------------------------
 *  Environment   Configuration
 ------------------------------------------------------------------------*/
//zbing add
#define CONFIG_BOOTCOMMAND "nand read 1500000 ker1;bootm 1500000"
//end
#define CONFIG_BOOTDELAY    3
#define CONFIG_BOOTARGS		"mem=2048M console=ttyAMA0,115200 ubi.mtd=fs1 root=ubi0:rootfs rw rootfstype=ubifs mmz=ddr,0,0,100M"
#define CONFIG_NETMASK      255.255.252.0 /* talk on MY local net */
#define CONFIG_IPADDR       192.168.9.180 /* static IP I currently own */
#define CONFIG_SERVERIP     192.168.9.33 /* current IP of tftp server ip */

#define CONFIG_CMD_RUN 

//zbing add
//#define CONFIG_ETHADDR		1c:c3:16:34:56:78
#define CONFIG_MS_RANDOM_ETHADDR
#define CONFIG_ETHADDR_PRE		"1C:C3:16"
//end

#define CONFIG_BOOTFILE     "MSFImage_81.7.0.5-beta3" /* file to load */
#define CONFIG_BAUDRATE     115200

//zhing modify
#if 0
#ifdef CONFIG_HI3798CV2X_FPGA
#define CONFIG_EXTRA_ENV_SETTINGS       "phyintf=mii,mii,mii\0" \
					"phymdio=0,0,0\0"       \
					"phyaddr=2,1,2\0"       \
					"phygpio=2-3,4-5,2-3\0" \
					"gmac_debug=0\0"
#endif
#else
#if (MS_DDR_VERSION==1)
#define CONFIG_EXTRA_ENV_SETTINGS \
   "mdio_intf=rgmii\0" \
		"mtdids=nand0=hinand\0" \
		"nor_mtdparts=hi_sfc:832K(uboot),128K(env)\0" \
		"mtdparts=mtdparts=hinand:1M(baseparam),3M(logo),6M(ker1),6M(ker2),84M(fs1),84M(fs2),24M(cfg),32M(oem),-(rsd)\0" \
		"uboot=name:uboot\tver:0x48000100\tsize:663552\tdate:0x07e10703 \0"\
		"baseparam=name:baseparam\tver:0x48000000\tsize:6144\tdate:0x07e10703\0"\
		"logo=name:logo\tver:0x48000000\tsize:188416\tdate:0x07e10703 \0"\
		"ker1=name:ker1\tver:0x48000000\tsize:5105664\tdate:0x07e10703 \0"\
		"ker2=name:ker2\tver:0x48000000\tsize:5105664\tdate:0x07e10703 \0"\
		"fs1=name:fs1\tver:0x48000000\tsize:63307776\tdate:0x07e10703 \0"\
		"fs2=name:fs2\tver:0x48000000\tsize:63307776\tdate:0x07e10703 \0" \
		"uboot_name=uboot \0" \
		"uImage_name=uImage \0" \
		"update_uboot=mw.b 0x1000000 0xff 0xd0000;tftp 0x1000000 ${uboot_name};sf probe 0;sf erase 0 0xd0000;sf write 0x1000000 0 0xd0000 \0" \
		"update_ker1=tftp 0x1000000 $(uImage_name);nand erase ker1;nand write 0x1000000 ker1 \0" \
		"erase_env=sf probe 0;sf erase 0xd0000 0x20000\0"
#elif (MS_DDR_VERSION==2)
#define CONFIG_EXTRA_ENV_SETTINGS \
		"mdio_intf=rgmii\0" \
		"mtdids=nand0=hinand\0" \
		"nor_mtdparts=hi_sfc:832K(uboot),128K(env)\0" \
		"mtdparts=mtdparts=hinand:1M(baseparam),3M(logo),6M(ker1),6M(ker2),84M(fs1),84M(fs2),24M(cfg),32M(oem),-(rsd)\0" \
		"uboot=name:uboot\tver:0x48000200\tsize:663552\tdate:0x07e10703 \0"\
		"baseparam=name:baseparam\tver:0x48000000\tsize:6144\tdate:0x07e10703\0"\
		"logo=name:logo\tver:0x48000000\tsize:188416\tdate:0x07e10703 \0"\
		"ker1=name:ker1\tver:0x48000000\tsize:5105664\tdate:0x07e10703 \0"\
		"ker2=name:ker2\tver:0x48000000\tsize:5105664\tdate:0x07e10703 \0"\
		"fs1=name:fs1\tver:0x48000000\tsize:63307776\tdate:0x07e10703 \0"\
		"fs2=name:fs2\tver:0x48000000\tsize:63307776\tdate:0x07e10703 \0" \
		"uboot_name=uboot \0" \
		"uImage_name=uImage \0" \
		"update_uboot=mw.b 0x1000000 0xff 0xd0000;tftp 0x1000000 ${uboot_name};sf probe 0;sf erase 0 0xd0000;sf write 0x1000000 0 0xd0000 \0" \
		"update_ker1=tftp 0x1000000 $(uImage_name);nand erase ker1;nand write 0x1000000 ker1 \0" \
		"erase_env=sf probe 0;sf erase 0xd0000 0x20000\0"
#elif (MS_DDR_VERSION==3)
#define CONFIG_EXTRA_ENV_SETTINGS \
		"mdio_intf=rgmii\0" \
		"mtdids=nand0=hinand\0" \
		"nor_mtdparts=hi_sfc:832K(uboot),128K(env)\0" \
		"mtdparts=mtdparts=hinand:1M(baseparam),3M(logo),6M(ker1),6M(ker2),84M(fs1),84M(fs2),24M(cfg),32M(oem),-(rsd)\0" \
		"uboot=name:uboot\tver:0x48000300\tsize:663552\tdate:0x07e10703 \0"\
		"baseparam=name:baseparam\tver:0x48000000\tsize:6144\tdate:0x07e10703\0"\
		"logo=name:logo\tver:0x48000000\tsize:188416\tdate:0x07e10703 \0"\
		"ker1=name:ker1\tver:0x48000000\tsize:5105664\tdate:0x07e10703 \0"\
		"ker2=name:ker2\tver:0x48000000\tsize:5105664\tdate:0x07e10703 \0"\
		"fs1=name:fs1\tver:0x48000000\tsize:63307776\tdate:0x07e10703 \0"\
		"fs2=name:fs2\tver:0x48000000\tsize:63307776\tdate:0x07e10703 \0" \
		"uboot_name=uboot \0" \
		"uImage_name=uImage \0" \
		"update_uboot=mw.b 0x1000000 0xff 0xd0000;tftp 0x1000000 ${uboot_name};sf probe 0;sf erase 0 0xd0000;sf write 0x1000000 0 0xd0000 \0" \
		"update_ker1=tftp 0x1000000 $(uImage_name);nand erase ker1;nand write 0x1000000 ker1 \0" \
		"erase_env=sf probe 0;sf erase 0xd0000 0x20000\0"

#endif
#endif
//end
/*-----------------------------------------------------------------------
 * for bootm linux
 *
 -----------------------------------------------------------------------*/

#define CONFIG_BOOTM_LINUX 1    //for bootm
#define CONFIG_SYS_LOAD_ADDR (CFG_DDR_PHYS_OFFSET + 0x08000)   /* default load address 0x80008000*/
#define CONFIG_ZERO_BOOTDELAY_CHECK 1   /*use ^c to interrupt*/

/*-----------------------------------------------------------------------
 * for  commond configure
 -----------------------------------------------------------------------*/
//tftp comm
#define CONFIG_SYS_MAX_FLASH_BANKS    1 /* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT     256

//zbing add
#define CONFIG_CMD_DOWNLOAD
//dm2016
#define CONFIG_CMD_MSTYPE
#define CONFIG_CMD_DM2016
#define CFG_I2C_DM2016_ADDR				0xA2
//end

//do_printenv  do_setenv common/cmd_nvedit.c
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_MAXARGS 16          /* max number of command args   */

#define CONFIG_SYS_CONSOLE_INFO_QUIET         1

/*-----------------------------------------------------------------------
 * Flash Memory Configuration v100
 *----------------------------------------------------------------------*/

#  ifdef CONFIG_SPIFLASH_SUPPORT
#    define CONFIG_HIFMC100_SPI_NOR_SUPPORT
#  endif
#  ifdef CONFIG_NAND_SUPPORT
#    define CONFIG_HIFMC100_NAND_SUPPORT
#  endif
#  ifdef CONFIG_SPI_NAND_SUPPORT
#    define CONFIG_HIFMC100_SPI_NAND_SUPPORT
#  endif

#ifdef CONFIG_HIFMC100_SPI_NOR_SUPPORT
#  define CONFIG_GENERIC_SF
#  define CONFIG_HIFMC100_SPI_NOR_CHIP_NUM           2
#endif

#if defined(CONFIG_HIFMC100_SPI_NAND_SUPPORT) || defined(CONFIG_HIFMC100_NAND_SUPPORT)
#  define CONFIG_GENERIC_NAND

#  undef CONFIG_HIFMC100_HARDWARE_PAGESIZE_ECC
#  undef CONFIG_HIFMC100_PAGESIZE_AUTO_ECC_NONE
#  define CONFIG_HIFMC100_AUTO_PAGESIZE_ECC

#ifdef CONFIG_HIFMC100_SPI_NAND_SUPPORT
  #define CONFIG_SYS_SPI_NAND_MAX_CHIPS	       1
#endif

#ifdef CONFIG_HIFMC100_NAND_SUPPORT
  #define CONFIG_SYS_NAND_MAX_CHIPS            1
#endif
#endif /* CONFIG_HIFMC_SPI_NAND_SUPPORT */

#if (defined CONFIG_HIFMC100_NAND_SUPPORT) || (defined CONFIG_HIFMC100_SPI_NAND_SUPPORT) || (defined CONFIG_HIFMC100_SPI_NOR_SUPPORT)
#define CONFIG_HIFMC100_SUPPORT
#endif

#ifdef CONFIG_HIFMC100_SUPPORT
#  define CONFIG_HIFMC100_REG_BASE_ADDRESS           REG_BASE_FMC
#  define CONFIG_HIFMC100_BUFFER_BASE_ADDRESS        MEM_BASE_FMC
#  define CONFIG_HIFMC100_BUFFER_SIZE                0x1000
#endif /* CONFIG_HIFMC100_SUPPORT */

#ifdef CONFIG_GENERIC_NAND
/* env in flash instead of CFG_ENV_IS_NOWHERE */
//#  define CONFIG_ENV_IS_IN_NAND                1

#  define CONFIG_CMD_NAND
#  define CONFIG_SYS_NAND_BASE                 REG_BASE_FMC

#  define CONFIG_SYS_MAX_NAND_DEVICE           1
#  define NAND_RR_PARAM_OFFSET                 0x80000ULL
#undef CONFIG_NANDID_TAG
//#define CONFIG_NANDPTN_TAG                      1
//#define CONFIG_NANDPTN_TAG_VAL                  0x48694E70

#endif /* CONFIG_GENERIC_NAND */

#ifdef CONFIG_GENERIC_SF

/* env in flash instead of CFG_ENV_IS_NOWHERE */
#  define CONFIG_ENV_IS_IN_SPI_FLASH               1

#  define CONFIG_CMD_SF
#endif /* CONFIG_GENERIC_SF */

/*****************************************************************************
 * UBI/UBIFS Filesystem Configuration
******************************************************************************/

#if 0 /* Define ubi/ubifs supports */
/* UBI/UBIFS need much more malloc memory. */
#  warning adjust malloc macro CONFIG_SYS_MALLOC_LEN for ubi/ubifs.
#  undef CONFIG_SYS_MALLOC_LEN
#  define CONFIG_SYS_MALLOC_LEN      (CONFIG_ENV_SIZE + 128*1024 + (512 << 11))
#  define CONFIG_CMD_UBI              /* UBI Support      */
#  define CONFIG_CMD_UBIFS            /* UBIFS Support    */
#  define CONFIG_MTD_PARTITIONS
#  define CONFIG_MTD_DEVICE
#  define CONFIG_RBTREE
#  define CONFIG_LZO
#  define CONFIG_CMD_MTDPARTS
#endif

//zbing add
#if 1

#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nand0=hinand"
#define MTDPARTS_DEFAULT	"mtdparts=hinand:1M(baseparam)," \
                            "3M(logo),"         \
                            "6M(ker1),"         \
                            "6M(ker2),"         \
                            "84M(fs1),"			\
                            "84M(fs2),"			\
        					"24M(cfg),"         \
        					"32(oem),"          \
        					"-(rsd)"  
#endif
//end
/*****************************************************************************/
#if 0 /* Define net-console supports */
#  define CONFIG_NETCONSOLE              1
#endif
/*****************************************************************************/
/*****************************************************************************/
#define CFG_DDR_PHYS_OFFSET             MEM_BASE_DDR
#define CFG_DDR_SIZE		(2*1024*1024*1024)

#define CONFIG_SYS_MEMTEST_START       \
	(CFG_DDR_PHYS_OFFSET + sizeof(unsigned long))
#define CONFIG_SYS_MEMTEST_END         (CFG_DDR_PHYS_OFFSET + CFG_DDR_SIZE)
#define CONFIG_SYS_MEMTEST_SCRATCH     CFG_DDR_PHYS_OFFSET

/*****************************************************************************/
/* Reserve memory config

High|		|
Addr|		|
	--------- <-- CONFIG_RESERVE_MEM_SIZE_MAX
	|   	|
	|Reserve|
	|  mem	|
	|   	|
	|   	|
	--------- <--
	|  		|
	|  Gap	| CONFIG_RESERVE_MEM_GAP
	|   	|
	--------- <--
	|   	|
	|U-boot |
Low	|   	|
Addr

*/
/* The gap between the free memory starting addr and the reserve mem bound addr */
#define CONFIG_RESERVE_MEM_GAP         128*1024*1024
//#define CONFIG_RESERVE_MEM_SIZE_MAX    512*1024*1024

/*****************************************************************************/

#define CONFIG_CMDLINE_TAG            1   /* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG             1   /* support initrd */
#define CONFIG_SETUP_MEMORY_TAGS      1
#define CONFIG_MISC_INIT_R            1   /* call misc_init_r during start up */

#define CONFIG_SDKVERSION_TAG         1
#ifdef  CONFIG_SDKVERSION_TAG
#  define CONFIG_SDKVERSION_TAG_VAL   0x726d6d75
#endif

#undef CONFIG_BOOTREG_TAG
#ifdef  CONFIG_BOOTREG_TAG
#  define CONFIG_BOOTREG_TAG_VAL   0x53000001
#endif

#define CONFIG_ETHADDR_TAG            1
#ifdef  CONFIG_ETHADDR_TAG
#define CONFIG_ETHADDR_TAG_VAL        0x726d6d73
#define CONFIG_PHYADDR_TAG_VAL        (CONFIG_ETHADDR_TAG_VAL + 1)
#define CONFIG_PHYINTF_TAG_VAL        0x726d6d80
#define CONFIG_PHYGPIO_TAG_VAL        (CONFIG_PHYINTF_TAG_VAL + 1)
#endif

#define HISFV_DEFAULT_PHY_U                 1
#define HISFV_DEFAULT_PHY_D                 3

#define CONFIG_SYS_NO_FLASH
#define CONFIG_PL011_SERIAL           1

#ifdef CONFIG_HI3798CV2X_FPGA
#  define CONFIG_PL011_CLOCK            54000000
#  define CONFIG_CONS_INDEX             0
#  define CONFIG_CUR_UART_BASE          REG_BASE_UART0
#else
#  define CONFIG_PL011_CLOCK            75000000
#  define CONFIG_CONS_INDEX             0
#  define CONFIG_CUR_UART_BASE          REG_BASE_UART0
#endif

#define CONFIG_PL01x_PORTS            \
	{(void *)REG_BASE_UART0, (void *)REG_BASE_UART1,\
	(void *)REG_BASE_UART2, (void *)REG_BASE_UART3, (void *)REG_BASE_UART4}

#define CONFIG_PRODUCTNAME            "hi3798cv200"

/*****************************************************************************/
/* DDR training Configuration */
/******************************************************************************/
#define CONFIG_DDR_TRAINING_V2

/*****************************************************************************/
/* eMMC Flash Configuration */
/******************************************************************************/
#ifdef CONFIG_EMMC_SUPPORT
#  define CONFIG_HIMCI_V200                   1
#  define CONFIG_EMMC_PARAM_TAG
#endif

#ifdef CONFIG_HIMCI_V200
#  define CONFIG_MMC_BOOT_ADDR                0
#  define CONFIG_MMC_POWER_OFF_TIMEOUT        50
#  define CONFIG_MMC_POWER_ON_TIMEROUT        50
#  define CONFIG_MMC_RESET_LOW_TIMEOUT        10
#  define CONFIG_MMC_RESET_HIGH_TIMEROUT      300
#  define CONFIG_GENERIC_MMC
#endif /* CONFIG_HIMCI_V200 */

#ifdef CONFIG_GENERIC_MMC
/* env in flash instead of CFG_ENV_IS_NOWHERE */
#  define CONFIG_ENV_IS_IN_EMMC               1
#  define CONFIG_CMD_MMC
#  define CONFIG_EXT4
#  define CONFIG_EXT4_SPARSE
#endif /* CONFIG_GENERIC_MMC */

#ifdef CONFIG_EMMC_PARAM_TAG
#define ATAG_EMMC_PARAM 0x48694E7b
#endif
/******************************************************************************/
/*  USB Flash Configuration */
/******************************************************************************/
//#define CONFIG_USB_BOOTSTRAP

#  ifdef CONFIG_USB_DEVICE
#    define CONFIG_USB_DEVICE_DMA
#    define CONFIG_USBDEV_REV_2_93a               0x4F54293A
#    define CONFIG_USBDEV_REV_2_94a               0x4F54294A
#    define CONFIG_USBDEV_REV_3_00a               0x4F54300A
#    undef CONFIG_USB_DEVICE_DBG
#  endif

#ifdef CONFIG_USB_SUPPORT
#  define CONFIG_CMD_USB
#endif

#ifdef CONFIG_CMD_USB
/*
 * ehci and ohci only choose one
 * only ohci can support keyboard
 */
//#  define CONFIG_USB_OHCI_NEW
#  define CONFIG_USB_EHCI
#  define CONFIG_USB_HOST_BOOTSTRAP
#  define CONFIG_FILE_BOOTARGS_NAME         "bootargs.bin"
#  define CONFIG_FILE_RECOVERY_NAME         "recovery.img"
#  define CONFIG_FILE_READ_ADDR             0x1000000
#  define CONFIG_FILE_MAX_SIZE              0x1000000

#  define CONFIG_SYS_USB_OHCI_REGS_BASE     0xf9880000
#  define CONFIG_SYS_USB_OHCI_SLOT_NAME     "hiusb"
#  define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS    3
#  define CONFIG_SYS_USB_OHCI_CPU_INIT
#  define CONFIG_USB_KEYBOARD
#  define CONFIG_USB_STORAGE
#  define CONFIG_SYS_USB_EVENT_POLL
#  define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS    3

/*-----------------------------------------------------------------------
* Fat File System Configuration
 -----------------------------------------------------------------------*/
#  ifdef CONFIG_USB_STORAGE
#    define CONFIG_CMD_FAT
#  endif

#  ifdef  CONFIG_CMD_FAT
#    define CONFIG_USB_VFAT_INF
#    define CONFIG_SUPPORT_VFAT
#    define CONFIG_DOS_PARTITION
#    define CONFIG_SYS_DEVICE_DEREGISTER
#    define LITTLEENDIAN
#  endif

#endif


#define CONFIG_MEM_BASE_FREE_START  0X1000000
/******************************************************************************
 * The follow ONLY used in not CA mode.
******************************************************************************/
#if !(defined(CONFIG_SUPPORT_CA_DEBUG) || defined(CONFIG_SUPPORT_CA_RELEASE))

/******************************************************************************/
/*  lib  Uncompress/compress Configuration */
/******************************************************************************/
#define CONFIG_GZIP     1
#define CONFIG_ZLIB     1
#ifdef CONFIG_GZIP
#  define CONFIG_CMD_UNZIP
#endif

#define CONFIG_HIBERNATE
#ifdef CONFIG_HIBERNATE
#define CONFIG_HIBERNATE_HIBDRV_SIZE     0x300000
#define CONFIG_HIBERNATE_USERAPI_SIZE    0x200000
#endif

//#define CONFIG_HISI_SNAPSHOT_BOOT
#ifdef CONFIG_HISI_SNAPSHOT_BOOT
#define CONFIG_HIBERNATE_QBBOOT_SIZE     0xc00000
#endif

#undef CONFIG_SHOW_MEMORY_LAYOUT
#define CONFIG_SHOW_REG_INFO
#define CONFIG_SHOW_RESERVE_MEM_LAYOUT

#define CONFIG_BOOTHEAD_GAP           0

/*****************************************************************************/
/* switch to 64bit mode */
/******************************************************************************/
#define ARM64_WM_RST_VEC	0xFFF0000

/*-----------------------------------------------------------------------
 * console display  Configuration
 ------------------------------------------------------------------------*/

#define CONFIG_VERSION_VARIABLE      1 /*used in common/main.c*/
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT            "hi3798# " /* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE            1024  /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE            \
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)

#define CFG_LONGHELP
#define CFG_BARGSIZE            CFG_CBSIZE /* Boot Argument Buffer Size */
#undef  CFG_CLKS_IN_HZ                  /* everything, incl board info, in Hz */
#define CFG_LOAD_ADDR           \
	(CFG_DDR_PHYS_OFFSET + 0x08000)     /* default load address */
#define CONFIG_AUTO_COMPLETE    1
#define CFG_CMDLINE_HISTORYS    8
#define CONFIG_CMDLINE_EDITING

//zbing delete
#if 0 /* Enable Ctrl+C to stop autoboot.*/
#  define CONFIG_AUTOBOOT_KEYED      1
#  define CONFIG_AUTOBOOT_PROMPT     "Press Ctrl+C to stop autoboot\n"
#  define CONFIG_AUTOBOOT_DELAY_STR  "hisilicon #"
#  define CONFIG_AUTOBOOT_STOP_STR   "\x03"
#endif
//end

// cp.b
#define CONFIG_CMD_MEMORY  /*md mw cp etc.*/
#define CONFIG_CMD_LOADB  /* loadb   common/cmd_load.c*/

/*-----------------------------------------------------------------------
 * SATA  Configuration
 -----------------------------------------------------------------------*/
#ifdef CONFIG_SATA_SUPPORT
#  define CONFIG_SATA_HI3798CV2X
#  define CONFIG_CMD_SATA
#  define CONFIG_HISATA
#  define CONFIG_LBA48
#  define CONFIG_LIBATA
#  define CONFIG_SYS_SATA_MAX_DEVICE          1
#  define CFG_HZ                              93750
#endif
/*-----------------------------------------------------------------------
 * HIETH driver
 -----------------------------------------------------------------------*/
#ifdef CONFIG_NET_SUPPORT
#  define CONFIG_NET_HIGMACV300                   1
#endif
/* default is hieth-switch-fabric */
#ifdef CONFIG_NET_HISFV300
#  define CONFIG_NET_HISFV300_HI3798CV2X          1
#  define HISFV_MII_MODE                      0
#  define HISFV_RMII_MODE                     1
#  define HIETH_MII_RMII_MODE_U               HISFV_MII_MODE /* XXX only for hi3719 FPGA. */
#  define HIETH_MII_RMII_MODE_D               HISFV_RMII_MODE /* XXX only for hi3719 FPGA. */
#endif /* CONFIG_NET_HISFV300 */

/*-----------------------------------------------------------------------
 * STMMAC driver
 -----------------------------------------------------------------------*/
#undef CONFIG_NET_STMMAC
#ifdef CONFIG_NET_STMMAC
#  define STMMAC_IOADDR             (0x60140000)
#  define STMMAC_IOSIZE             (0x10000)
#  define STMMAC_FRQDIV             (0)
#  define STMMAC_PHYADDR            (3)
#  define STMMAC_PHYNAME            "0:03"
#  define CONFIG_PHY_GIGE
#endif /* CONFIG_NET_STMMAC */

/*-----------------------------------------------------------------------
 * network config
 -----------------------------------------------------------------------*/
#ifdef CONFIG_NET_SUPPORT
#  define CONFIG_ARP_TIMEOUT                50000UL
#  define CONFIG_NET_RETRY_COUNT            50
#  define CONFIG_CMD_NET
#  define CONFIG_CMD_PING
#  define CONFIG_CMD_MII
#  define CONFIG_SYS_FAULT_ECHO_LINK_DOWN    1
#endif
/******************************************************************************/

/******************************************************************************/
#else /* !(defined(CONFIG_SUPPORT_CA_DEBUG) || defined(CONFIG_SUPPORT_CA_RELEASE)) */
/* The follow ONLY define in CA mode */
/******************************************************************************/

#undef  CONFIG_DDR_TRAINING

#define DDR_TRAINING_LOG_DISABLE

#if defined(CONFIG_SUPPORT_CA_DEBUG)
#define CONFIG_GZIP     1
#define CONFIG_ZLIB     1
#ifdef CONFIG_GZIP
#  define CONFIG_CMD_UNZIP
#endif
#endif

#define CONFIG_SYS_PROMPT            "fastboot# " /* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE            1024         /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE            \
	(CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)

#ifdef CONFIG_SUPPORT_CA_DEBUG
#define CONFIG_CMD_MEMORY  /*md mw cp etc.*/
#endif

#ifdef CONFIG_SUPPORT_CA_RELEASE
#  define CONFIG_SERIAL_NO_DISPLAY
#endif

#if defined(CONFIG_SUPPORT_CA_NAGRA)
#  define CONFIG_GZIP     1
#  define CONFIG_ZLIB     1
#endif

#define CONFIG_BOOTHEAD_GAP          ((TEXT_BASE & 0x000FFFFF) + 0x1000)

#endif /* !(defined(CONFIG_SUPPORT_CA_DEBUG) || defined(CONFIG_SUPPORT_CA_RELEASE)) */

#ifdef CONFIG_DDR_TRAINING
#  undef CONFIG_DDR_TRAINING_STARTUP
#  define CONFIG_DDR_DATAEYE_TRAINING_STARTUP
#  define CONFIG_DDR_TRAINING_HI3798CV2X
#endif /* CONFIG_DDR_TRAINING */

#define CONFIG_BOOTROM_SUPPORT

#if (defined(CONFIG_BOOTROM_SUPPORT) || defined(CONFIG_BOOTROM_CA_SUPPORT)) \
	&& (!defined(REG_START_FLAG) || !defined(CONFIG_START_MAGIC))
#  error Please define CONFIG_START_MAGIC and CONFIG_START_MAGIC first
#endif

#ifdef CONFIG_UNIFIED_BOOT
/* size definition */
#define CONFIG_UNCHK_SIZE	0x100
#define CONFIG_REG_SIZE		0x2000

/* offset definition */
/*--------- key area --------------*/
#define CONFIG_KEY_AREA_POS		0x0
#define CONFIG_AUXAREA_ADR_POS		0x214
#define CONFIG_AUXAREA_LEN_POS		0x218
#define CONFIG_AUXAREA_LEN              0x3400
#define CONFIG_BOOT_ENTRY_POS		0x21C
#define CONFIG_CHECKED_AREA_START       (CONFIG_AUXCODE_AREA_POS + CONFIG_AUXAREA_LEN + CONFIG_UNCHK_SIZE)

/*--------- param area --------------*/
#define CONFIG_PARAM_AREA_POS		0x400
#define CONFIG_SCS_HASHED_AREA_OFF_POS	0x400
#define  CONFIG_SCS_HASHED_AREA_OFF_VAL	0x100

#define CONFIG_SCS_HASHED_AREA_LEN_POS	0x404
#define CONFIG_TOTAL_BOOT_AREA_LEN_POS	0x408

#define CONFIG_DEFAULT_BOOT_REG_POS	0x480

#define CONFIG_PARAM_AREA_SIG_POS	0x2EC0

/*--------- unchecked area for hisi --------------*/
#define CONFIG_UNCHK_AREA_HISI_POS	0x2FC0
#define CONFIG_SCS_SIM_FLAG_POS		0x2FC0
#define  CONFIG_SCS_SIM_FLAG_VAL	0x69875AB4

#define CONFIG_BOOT_FLAG_POS		0x2FC4
#define  CONFIG_BOOT_FLAG_VAL		0x435A590D

#define CONFIG_AUX_ENC_FLAG_POS		0x2FC8
#define  CONFIG_AUX_ENC_FLAG_VAL	0x2A13C812

#define CONFIG_SUPPORT_MULTI_PARAM_POS	0x2FE0
#define CONFIG_PARAM_START_ADDR_POS	0x2FE4
#define CONFIG_PARAM_ITEM_LEN_POS	0x2FE8
#define CONFIG_BOOT_STORE_ADDR_POS	0x2FEC
#define  CONFIG_BOOT_STRORE_ADDR_VAL	0x01000000

/*--------- auxcode area --------------*/
#define CONFIG_AUXCODE_AREA_POS		0x3000

#endif

#endif /* __CONFIG_H */
