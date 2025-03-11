/*
 * Configuation settings for the hi3536c board.
 *
 * Copyright (c) 2016-2017 HiSilicon Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
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

#ifndef __CONFIG_HI3536C_H__
#define __CONFIG_HI3536C_H__

#include <asm/arch/platform.h>

#define MS_UBOOT_VERSION	"73.1.0.10"
#define MS_DDR_VERSION	0

/*-----------------------------------------------------------------------
 * cpu_init configuration
 * if bit[3:2] = 01b, AXI = 200M
 * if bit[3:2] = 11b, AXI = 300M
 *----------------------------------------------------------------------*/
#define HW_REG(a) (*(unsigned long *)(a))
#define get_bus_clk() ({ \
	unsigned int regval, busclk = 0; \
	regval = HW_REG(CRG_REG_BASE + REG_SOC_CRG); \
	regval = GET_SYS_BUS_CLK(regval); \
	if (PERI_CLK_300M == regval) \
		busclk = 300000000; \
	else if (PERI_CLK_200M == regval) \
		busclk = 200000000; \
	busclk; \
})

/* cpu_init configuration */
#define CFG_CLK_BUS  get_bus_clk()

#define CFG_TIMER_PER 4		/* AXI:APB is 4:1 */
/* #define CFG_TIMER_CLK (CFG_CLK_BUS / CFG_TIMER_PER) */
#define CFG_TIMER_CLK 3000000

#define CONFIG_HI3536C		1

//#define CONFIG_SVB_ENABLE

/*-----------------------------------------------------------------------
 * Hisilicon Flash Memory Controller Configuration
 *----------------------------------------------------------------------*/
#define CONFIG_HIFMC

#ifdef CONFIG_HIFMC
	#define CONFIG_HIFMC_SPI_NOR
	#define CONFIG_HIFMC_SPI_NAND

	#define CONFIG_HIFMC_REG_BASE		FMC_REG_BASE
	#define CONFIG_HIFMC_BUFFER_BASE	FMC_MEM_BASE

	#define CONFIG_HIFMC_MAX_CS_NUM		2
#endif

#ifdef CONFIG_HIFMC_SPI_NOR
	#define CONFIG_CMD_SF
	#define CONFIG_ENV_IS_IN_SPI_FLASH
	#define CONFIG_SPI_NOR_MAX_CHIP_NUM	2
	#define CONFIG_SPI_NOR_QUIET_TEST
//	#define CONFIG_SPI_BLOCK_PROTECT
#endif

#ifdef CONFIG_HIFMC_SPI_NAND
	#define CONFIG_CMD_NAND
//	#define CONFIG_ENV_IS_IN_NAND
	#define CONFIG_SPI_NAND_MAX_CHIP_NUM	1
	#define CONFIG_SYS_MAX_NAND_DEVICE	CONFIG_SPI_NAND_MAX_CHIP_NUM
	#define CONFIG_SYS_NAND_MAX_CHIPS	CONFIG_SPI_NAND_MAX_CHIP_NUM
	#define CONFIG_SYS_NAND_BASE		FMC_MEM_BASE
	#define CONFIG_SYS_NAND_QUIET_TEST

	#undef CONFIG_HIFMC100_HARDWARE_PAGESIZE_ECC
	#define CONFIG_HIFMC100_AUTO_PAGESIZE_ECC
	#undef CONFIG_HIFMC100_PAGESIZE_AUTO_ECC_NONE
#endif

#define CONFIG_CLOSE_SPI_8PIN_4IO

/*-----------------------------------------------------------------------
 * for cpu/hi3536c/start.S
 *----------------------------------------------------------------------*/
#define FMC_TEXT_ADRS		(FMC_MEM_BASE)

#define MEM_BASE_DDR		(DDR_MEM_BASE)
#define CONFIG_SYS_MALLOC_LEN	(CONFIG_ENV_SIZE + 128*1024)
/* size in bytes reserved for initial data */
#define CONFIG_SYS_GBL_DATA_SIZE	128

/*-----------------------------------------------------------------------
 * for timer configuration (udelay)	cpu/hi3536c/hi3536c/timer.c
 * enable timer				board/hi3536c/board.c
 *----------------------------------------------------------------------*/
#define CFG_TIMERBASE           TIMER0_REG_BASE
/* enable timer.32bit, periodic,mask irq,256 divider. */
#define CFG_TIMER_CTRL		0xCA
#define READ_TIMER		(*(volatile unsigned long *)\
					(CFG_TIMERBASE + REG_TIMER_VALUE))
/* how many ticks per second. show the precision of timer. */
#define CONFIG_SYS_HZ           (CFG_TIMER_CLK / 256)
#define CFG_HZ			CONFIG_SYS_HZ

/*allow change env*/
#define  CONFIG_ENV_OVERWRITE
/*-----------------------------------------------------------------------
 * environment && bd_info  && global_data  configure
 * used in file
 *----------------------------------------------------------------------*/
#include "../../product/env_setup.h"
/* env in flash instead of CFG_ENV_IS_NOWHERE */
#define CONFIG_ENV_OFFSET          0x60000      // 384Kb uboot, 128KB env. /*512KB environment starts here */
#define CONFIG_ENV_NAND_ADDR	(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SPI_ADDR	(CONFIG_ENV_OFFSET)
#define CONFIG_CMD_SAVEENV

#define CONFIG_ENV_SIZE		0x10000    /*include ENV_HEADER_SIZE */
#define CONFIG_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	    CONFIG_ENV_SIZE

#define CONFIG_NR_DRAM_BANKS	1          /* we have 1 bank of DRAM */
/* kernel parameter list phy addr */
#define CFG_BOOT_PARAMS		(MEM_BASE_DDR + 0x0100)

/*-----------------------------------------------------------------------
 *  Environment   Configuration
 *-----------------------------------------------------------------------*/
#define CONFIG_BOOTCOMMAND "nboot 0x82000000 ker1;bootm 0x82000000"
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTARGS		"mem=316M console=ttyAMA0,115200 ubi.mtd=fs1 root=ubi0:rootfs rw rootfstype=ubifs"
#define CONFIG_NETMASK  255.255.254.0       /* talk on MY local net */
#define CONFIG_IPADDR   192.168.1.10        /* static IP I currently own */
#define CONFIG_SERVERIP 192.168.1.2     /* current IP of tftp server ip */
#define CONFIG_ETHADDR  1c:c3:16:34:56:78
#define CONFIG_BOOTFILE "uImage"        /* file to load */
#define CONFIG_BAUDRATE         115200
#define CONFIG_USE_MDIO	"0,1"


//"mtdparts=mtdparts=hinand:512K(uboot),512K(env),3M(logo),6M(ker1),6M(ker2),84M(fs1),84M(fs2),24M(cfg),32M(oem),-(rsd)\0" 
//develop board only 128MB Nand flash  , MTDPARTS_DEFAULT don't forget.
#if (MS_DDR_VERSION == 0)
#define CONFIG_EXTRA_ENV_SETTINGS   \
    "mdio_intf=rmii\0" \
    "mtdids=nand0=hinand\0" \
    "nor_mtdparts=hi_sfc:384K(uboot),128K(env)\0" \
    "mtdparts=mtdparts=hinand:4M(logo),6M(ker1),6M(ker2),84M(fs1),84M(fs2),24M(cfg),32M(oem),-(rsd)\0" \
    "uboot=name:uboot\tver:0x07000000\tsize:296960\tdate:0x07e10704 \0"\
    "logo=name:logo\tver:0x07000000\tsize:264192\tdate:0x07e10704 \0"\
    "ker1=name:ker1\tver:0x07000000\tsize:3784704\tdate:0x07e10704 \0"\
    "ker2=name:ker2\tver:0x07000000\tsize:3784704\tdate:0x07e10704 \0"\
    "fs1=name:fs1\tver:0x49000000\tsize:58327040\tdate:0x07e10704 \0"\
    "fs2=name:fs2\tver:0x49000000\tsize:58327040\tdate:0x07e10704 \0"\
    "uboot_name=uboot \0" \
    "update_uboot=mw.b 0x82000000 0xff 0x80000;tftp 0x82000000 ${uboot_name};sf probe 0;sf erase 0 0x60000;sf write 0x82000000 0 0x60000 \0" \
    "update_ker1=tftp 0x82000000 uImage;nand erase ker1;nand write 0x82000000 ker1 \0" \
    "erase_env=sf probe 0;sf erase 0x60000 0x20000 \0" \
    "erase_uboot=sf probe 0;sf erase 0 0x60000 \0" 
#else
#define CONFIG_EXTRA_ENV_SETTINGS   \
    "mdio_intf=rmii\0" \
    "mtdids=nand0=hinand\0" \
    "nor_mtdparts=hi_sfc:384K(uboot),128K(env)\0" \
    "mtdparts=mtdparts=hinand:4M(logo),6M(ker1),6M(ker2),84M(fs1),84M(fs2),24M(cfg),32M(oem),-(rsd)\0" \
    "uboot=name:uboot\tver:0x07000100\tsize:296960\tdate:0x07e10704 \0"\
    "logo=name:logo\tver:0x07000000\tsize:264192\tdate:0x07e10704 \0"\
    "ker1=name:ker1\tver:0x07000000\tsize:3784704\tdate:0x07e10704 \0"\
    "ker2=name:ker2\tver:0x07000000\tsize:3784704\tdate:0x07e10704 \0"\
    "fs1=name:fs1\tver:0x49000000\tsize:58327040\tdate:0x07e10704 \0"\
    "fs2=name:fs2\tver:0x49000000\tsize:58327040\tdate:0x07e10704 \0"\
    "uboot_name=uboot \0" \
    "update_uboot=mw.b 0x82000000 0xff 0x80000;tftp 0x82000000 ${uboot_name};sf probe 0;sf erase 0 0x60000;sf write 0x82000000 0 0x60000 \0" \
    "update_ker1=tftp 0x82000000 uImage;nand erase ker1;nand write 0x82000000 ker1 \0" \
    "erase_env=sf probe 0;sf erase 0x60000 0x20000 \0" \
    "erase_uboot=sf probe 0;sf erase 0 0x60000 \0" 
#endif
/*-----------------------------------------------------------------------
 * Dynamic MTD partition support
 */
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#define MTDIDS_DEFAULT		"nand0=hinand"
#define MTDPARTS_DEFAULT	"mtdparts=hinand:4M(logo),"	\
        					"6M(ker1),"			\
                            "6M(ker2),"         \
                            "84M(fs1),"			\
                            "84M(fs2),"			\
        					"24M(cfg),"         \
        					"32M(oem),"          \
        					"-(rsd)"  

/*-----------------------------------------------------------------------
 * for bootm linux
 * used in file  board/hi3536c/board.c
 -----------------------------------------------------------------------*/

#define CONFIG_BOOTM_LINUX 1
/* default load address 0x80008000*/
#define CONFIG_SYS_LOAD_ADDR (CFG_DDR_PHYS_OFFSET + 0x08000)
#define CONFIG_ZERO_BOOTDELAY_CHECK 1   /*use ^c to interrupt*/
/*-----------------------------------------------------------------------
 * for  commond configure
 -----------------------------------------------------------------------*/
/* tftp comm */
#define CONFIG_TFTP_TSIZE

/* do_printenv  do_setenv common/cmd_nvedit.c */
#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_SYS_MAXARGS 32          /* max number of command args   */


#define CONFIG_CMD_RUN
//#define CONFIG_CMD_LOADB  /* loadb common/cmd_load.c*/

/*-----------------------------------------------------------------------
 * network config
 -----------------------------------------------------------------------*/
#define CONFIG_ARP_TIMEOUT		50000UL
#define CONFIG_NET_RETRY_COUNT		50
#define CONFIG_CMD_NET
#define CONFIG_CMD_PING
#define CONFIG_CMD_MII
#define CONFIG_SYS_FAULT_ECHO_LINK_DOWN	1

/*--------------------------------------------------------------------
 * DOWNLOAD CONFIG
 *----------------------------------------------------------------------*/
#define CONFIG_CMD_DOWNLOAD

/*-----------------------------------------------------------------------
 * HIETH-GMAC driver
 -----------------------------------------------------------------------*/
#define CONFIG_NET_HIGMACV300
#ifdef CONFIG_NET_HIGMACV300
	#define CONFIG_GMAC_NUMS		2
	#define CONFIG_ETH_TAG
	#define HIGMAC0_IOBASE		GSF_REG_BASE
	#define HIGMAC1_IOBASE		(GSF_REG_BASE + 0x1000)
	#define CONFIG_HIGMAC_PHY1_ADDR		3
	#define CONFIG_HIGMAC_PHY2_ADDR		5
	#define CONFIG_HIGMAC_PHY1_INTERFACE_MODE	2
	#define CONFIG_HIGMAC_PHY2_INTERFACE_MODE	2
	#define CONFIG_HIGMAC_DESC_4_WORD
#endif

/* no nor flash */
#define CONFIG_SYS_NO_FLASH
/* cp.b */
#define CONFIG_CMD_MEMORY  /*md mw cp etc.*/

/*-----------------------------------------------------------------------
 * console display  Configuration
 ------------------------------------------------------------------------*/

#define CONFIG_VERSION_VARIABLE  1 /*used in common/main.c*/
#define CONFIG_SYS_PROMPT  "hi3536C # "	/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE  1024            /* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE  (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CFG_LONGHELP
#define CFG_BARGSIZE    CFG_CBSIZE      /* Boot Argument Buffer Size */
#undef  CFG_CLKS_IN_HZ              /* everything, incl board info, in Hz */
/* default load address */
#define CFG_LOAD_ADDR   (CFG_DDR_PHYS_OFFSET + 0x08000)
#define CONFIG_AUTO_COMPLETE    1
#define CFG_CMDLINE_HISTORYS    8
#define CONFIG_CMDLINE_EDITING
#define CFG_DDR_PHYS_OFFSET MEM_BASE_DDR
#define CFG_DDR_SIZE		(512*1024*1024)

#define CONFIG_SYS_MEMTEST_START       \
	(CFG_DDR_PHYS_OFFSET + sizeof(unsigned long))
#define CONFIG_SYS_MEMTEST_END         (CFG_DDR_PHYS_OFFSET + CFG_DDR_SIZE)
#define CONFIG_SYS_MEMTEST_SCRATCH     CFG_DDR_PHYS_OFFSET

#define CONFIG_CMDLINE_TAG      1   /* enable passing of ATAGs  */
#define CONFIG_INITRD_TAG       1   /* support initrd */
#define CONFIG_SETUP_MEMORY_TAGS    1
#define CONFIG_MISC_INIT_R      1   /* call misc_init_r during start up */
#define CONFIG_ETHADDR_TAG            1
#ifdef  CONFIG_ETHADDR_TAG
#define CONFIG_ETHADDR_TAG_VAL        0x726d6d73
#endif

#define CONFIG_ETHMDIO_INF     1
#ifdef  CONFIG_ETHMDIO_INF
#define CONFIG_ETH_MDIO_INF_TAG_VAL 0x726d6d80
#endif

#define CONFIG_PHYADDR_TAG     1
#ifdef  CONFIG_PHYADDR_TAG
#define CONFIG_ETH_PHY_ADDR_TAG_VAL 0x726d6d74
#define CONFIG_ETH_PHY_SPEED_TAG_VAL 0x726d6d90
#endif

/* Define encryption chip info */
//#define CONFIG_CMD_DM2016	//eric.milesight add command to modify dm2016
#define CONFIG_CMD_MSTYPE

/*===================*/
/* I2C Configuration */
/*===================*/   
#define CONFIG_SYS_I2C_SPEED	100000	/* 100Kbps */
#define CONFIG_SYS_I2C_SLAVE	10	/* Bogus, master-only in U-Boot */

/* Software (bit-bang) I2C driver configuration */
#define CONFIG_SOFT_I2C

//eric.milesight modify 2014.1.1
//#define EEPROM_DM2016
#ifdef EEPROM_DM2016
//#define DEBUG_I2C

/*====================================================*/
/* EEPROM definitions for dm2016 encode chip */
/* We can read more than 1 byte everytime but write only 1 byte everytime */
/* on Sonata/DV_EVM board. No EEPROM on schmoogie.    */
/*====================================================*/
#define CFG_I2C_DM2016_MAX_SIZE			128
#define CFG_I2C_DM2016_ADDR_LEN			1
#define CFG_I2C_DM2016_ADDR				0x51
#define CFG_DM2016_PAGE_WRITE_BITS		3	// 8 byte per page
#define CFG_DM2016_PAGE_WRITE_DELAY_MS 	10
#endif
//eric.milesight end 2014.1.1

#undef CONFIG_SPIID_TAG
/* serial driver */
#define CONFIG_PL011_SERIAL 1
/* Except bootrom, hi3536c use XTAL OSC clk 24M */
#define CONFIG_PL011_CLOCK	(24000000)

#define CONFIG_PL01x_PORTS	\
	{(void *)UART0_REG_BASE, (void *)UART1_REG_BASE, \
	(void *)UART2_REG_BASE}
#define CONFIG_CONS_INDEX	0

#define CFG_SERIAL0                   UART0_REG_BASE

/* according to CONFIG_CONS_INDEX */
#define CONFIG_CUR_UART_BASE          CFG_SERIAL0
#define CONFIG_PRODUCTNAME		"hi3536c"

/*-----------------------------------------------------------------------
 * bootrom Configuration
 -----------------------------------------------------------------------*/
#define CONFIG_BOOTROM_SUPPORT
#if defined(CONFIG_BOOTROM_SUPPORT) \
	&& (!defined(REG_START_FLAG) || !defined(CONFIG_START_MAGIC))
#  error Please define CONFIG_START_MAGIC and CONFIG_START_MAGIC first
#endif

/*-----------------------------------------------------------------------
 * usb storage system update
 * ----------------------------------------------------------------------*/
/* #define CONFIG_AUTO_UPDATE			1 */
#ifdef CONFIG_AUTO_UPDATE
	#define CONFIG_AUTO_USB_UPDATE		1
#endif

//#define __LITTLE_ENDIAN				1
//#define CONFIG_DOS_PARTITION			1

//#define CONFIG_CMD_FAT				1
//#define CONFIG_CMD_EXT2				1

/*-----------------------------------------------------------------------
 * usb
 * ----------------------------------------------------------------------*/
//#define CONFIG_CMD_USB			1
//#define CONFIG_USB_OHCI			1 /* FIXME: CONFIG_USB_OHCI_NEW */
//#define CONFIG_USB_STORAGE		1
//#define CONFIG_LEGACY_USB_INIT_SEQ

/*-----------------------------------------------------------------------
 * DDR Training
 * ----------------------------------------------------------------------*/
#define CONFIG_DDR_TRAINING_V2	1

/* #define CONFIG_OSD_ENABLE */
#define CONFIG_OSD_ENABLE

#endif	/* __CONFIG_H */

