/*
 * Copyright (C) 2016 Novatek Microelectronics Corp. All rights reserved.
 * Author: iVoT-IM <iVoT_MailGrp@novatek.com.tw>
 *
 * Configuration settings for the Novatek NA51068 SOC.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_NA51068_H
#define __CONFIG_NA51068_H

#include <linux/sizes.h>

/*#define CONFIG_DEBUG	1	*/

#define CONFIG_ZBING

#ifdef CONFIG_DEBUG
#define DEBUG						1
#endif

#define CONFIG_SYS_CACHELINE_SIZE			32

/*#define CONFIG_NVT_FW_UPDATE_LED*/
/*#define CONFIG_NVT_PWM*/

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMV7
#define CONFIG_SYS_GENERIC_BOARD
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_NA51068
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_NVT_BOARD				1
#define CONFIG_NAND_TYPE_ONFI				0
#define CONFIG_NAND_TYPE_SPINAND			1
#define CONFIG_NAND_TYPE_SPINOR				2

#define NVT_FLASH_SOURCE_CLK				480000000
#if defined(CONFIG_NVT_SPI_NAND) || defined(CONFIG_NVT_SPI_NONE) || defined(CONFIG_NVT_SPI_NOR_NAND)
#define CONFIG_NVT_NAND_TYPE				CONFIG_NAND_TYPE_SPINAND
/*
 *	NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC
 *	NANDCTRL_SPIFLASH_USE_ONDIE_ECC
*/
#define CONFIG_NVT_NAND_ECC				NANDCTRL_SPIFLASH_USE_ONDIE_ECC
#elif defined(CONFIG_NVT_SPI_NOR)
#define CONFIG_NVT_NAND_TYPE				CONFIG_NAND_TYPE_SPINOR
#define CONFIG_MTD_DEVICE
#define CONFIG_SYS_MAX_SF_DEVICE			1
#define CONFIG_SF_DEFAULT_BUS				0
#define CONFIG_SF_DEFAULT_CS				0
#define CONFIG_SF_DEFAULT_MODE				0
#define CONFIG_SF_DEFAULT_SPEED				48000000
#else
#define CONFIG_NVT_NAND_TYPE				CONFIG_NAND_TYPE_ONFI
#endif

#define CONFIG_MISC_INIT_R

#define CONFIG_CPU_FREQ					24	/* default value, MHz */

#define CONFIG_SYS_HZ					1000

#define CONFIG_USE_ARCH_MEMCPY
#define CONFIG_USE_ARCH_MEMSET
#define CONFIG_VIDEO_LOGO

/*RTC Default Date*/
#define RTC_YEAR 2000
#define RTC_MONTH 1
#define RTC_DAY 1

/*PWM*/
#define CONFIG_PWM_NA51068

/*-----------------------------------------------------------------------
 * IP address configuration
 */
#ifdef CONFIG_NOVATEK_MAC_ENET
#define CONFIG_ETHNET
#endif

#define CONFIG_FIXED_ETH_PARAMETER

#ifdef CONFIG_FIXED_ETH_PARAMETER
#ifdef CONFIG_ETHNET
#define CONFIG_ETHADDR				{0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x01}
#define CONFIG_ETH1ADDR				{0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x02}
#define CONFIG_IPADDR				192.168.1.99	/* Target IP address */
#define CONFIG_NETMASK				255.255.255.0
#define CONFIG_SERVERIP				192.168.1.11	/* Server IP address */
#define CONFIG_GATEWAYIP			192.168.1.254
#define CONFIG_HOSTNAME				soclnx
#endif
#endif

#define ETH_PHY_HW_RESET
#define NVT_PHY_RST_PIN D_GPIO(1)

/*-----------------------------------------------------------------------
 * BOOTP options
 */
#ifdef CONFIG_ETHNET
#define CONFIG_BOOTP_SUBNETMASK
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_BOOTFILESIZE
#endif
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
 * Serial console configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	-4
#define CONFIG_SYS_NS16550_COM1		0xFE200000	/* UART0 */
/*#define CONFIG_SYS_NS16550_COM2		0xFE220000*/	/* UART1 */
/*#define CONFIG_SYS_NS16550_COM3		0xFE240000*/	/* UART2 */
/*#define CONFIG_SYS_NS16550_COM4		0xFE260000*/	/* UART3 */
#define CONFIG_SYS_NS16550_CLK 		48000000
#define CONFIG_CONS_INDEX               1
#define CONSOLE_UART1	0
#define CONSOLE_UART2	1
#define CONSOLE_UART3	2
#define CONSOLE_UART4	3

#define CONFIG_SYS_UART					CONSOLE_UART1
#define CONFIG_SYS_BAUDRATE_TABLE			{ 9600, 19200, 38400, 57600, 115200 }
#define CONFIG_BAUDRATE					115200

/*
 * UART0   X_UART0_SOUT/X_UART0_SIN
 * UART1-1 X_UART1_SOUT/X_UART1_SIN
 * UART1-2 X_I2C1_SCL/X_I2C1_SDA
 * UART1-3 X_GPIO_0/X_GPIO_1
 * UART2   X_UART2_SOUT/X_UART2_SIN
 * UART3-1 X_CPU_TDO/X_CPU_TCK
 * UART3-2 X_I2C0_SCL/X_I2C0_SDA
 * UART3-3 X_I2C2_SCL/X_I2C2_SDA
 * UART3-4 X_I2S1_SCLK/X_I2S1_FS
 * UART3-5 X_GPIO_2/X_GPIO_3
 * UART3-6 X_GPIO_7/X_GPIO_8

 */
#define PINMUX_CHANNEL_1 1
#define PINMUX_CHANNEL_2 2
#define PINMUX_CHANNEL_3 3
#define PINMUX_CHANNEL_4 4
#define PINMUX_CHANNEL_5 5
#define PINMUX_CHANNEL_6 6

#define UART0_PINMUX_SEL PINMUX_CHANNEL_1
#define UART1_PINMUX_SEL PINMUX_CHANNEL_1
#define UART2_PINMUX_SEL PINMUX_CHANNEL_1
#define UART3_PINMUX_SEL PINMUX_CHANNEL_1


/*-----------------------------------------------------------------------
 * NVT LED CONFIG
 *
 * LED GPIO selection
 * C_GPIO(x)
 * P_GPIO(x)
 * S_GPIO(x)
 * L_GPIO(x)
 * D_GPIO(x)
 * Duration Unit: ms
 */
#ifdef CONFIG_NVT_FW_UPDATE_LED
#ifdef CONFIG_NVT_PWM
#define PWM_SIGNAL_NORMAL 0
#define PWM_SIGNAL_INVERT 1
#define NVT_PWMLED (PWMID_0 | PWMID_1)
#define PWM_SIGNAL_TYPE PWM_SIGNAL_INVERT
#define	PWM_LED_ERASE 50
#define	PWM_LED_PROGRAM 5
#else
#define NVT_LED_PIN P_GPIO(12)
#define NVT_LED_ERASE_DURATION 30
#define NVT_LED_PROGRAM_DURATION 10
#endif
#endif

/*
 * USB HOST EHCI
 */

#ifdef CONFIG_USB_EHCI_NVTIVOT
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_MAX_CONTROLLER_COUNT	                2
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	        1
#define CONFIG_EHCI_IS_TDI
/*
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
*/
#endif


/*
 * Watchdog
 */
/*#define CONFIG_WDT_NVTIVOT*/
#ifdef CONFIG_WDT_NVTIVOT
#define CONFIG_HW_WATCHDOG
#define CONFIG_NVT_WDT_TIMEOUT 80 /*unit: seconds*/
#endif

#define CONFIG_USB_GADGET_NVTIVOT
#ifdef CONFIG_USB_GADGET_NVTIVOT
#define CONFIG_USB_GADGET_FOTG210
#define CONFIG_USBD_HS
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_GADGET
#define CONFIG_USB_FUNCTION_MASS_STORAGE
#define CONFIG_CMD_USB_MASS_STORAGE
#define CONFIG_USB_GADGET_VBUS_DRAW 2
#define CONFIG_USB_GADGET_DOWNLOAD
#define CONFIG_G_DNL_MANUFACTURER  "NVTIVOT_USB"
#define CONFIG_G_DNL_VENDOR_NUM    0x07B4
#define CONFIG_G_DNL_PRODUCT_NUM   0x0109
/*
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETH_CDC
#define CONFIG_USB_ETH_RNDIS
#define CONFIG_USBNET_HOST_ADDR	"de:ad:be:af:00:00"
*/
#endif


/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */

#define CONFIG_NR_DRAM_BANKS				1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1					0x00000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE				CONFIG_MEM_SIZE	/* DDR size 512MB */

/*
 * To include nvt memory layout
 */
#include "novatek/na51068_evb.h"

#define CONFIG_SYS_SDRAM_BASE				CONFIG_UBOOT_SDRAM_BASE
#define CONFIG_SYS_SDRAM_SIZE				CONFIG_UBOOT_SDRAM_SIZE

#define NVT_LINUX_BOOT_PARAM_ADDR			(CONFIG_LINUX_SDRAM_BASE + 0x100)

#define CONFIG_SYS_INIT_SP_ADDR				(CONFIG_UBOOT_SDRAM_BASE + CONFIG_UBOOT_SDRAM_SIZE - 0x1000)

/*
 * Our DDR memory always starts at 0x00000000 and U-Boot shall have
 * relocated itself to higher in memory by the time this value is used.
 * However, set this to a 32MB offset to allow for easier Linux kernel
 * booting as the default is often used as the kernel load address.
 */
#define CONFIG_SYS_LOAD_ADDR				CONFIG_LINUX_SDRAM_START

#define CONFIG_STANDALONE_LOAD_ADDR			0x1A000000

#define CONFIG_SYS_UBOOT_START				CONFIG_SYS_TEXT_BASE

#define CONFIG_CMD_MEMORY				1
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START			CONFIG_LINUX_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END				(CONFIG_LINUX_SDRAM_BASE + CONFIG_LINUX_SDRAM_SIZE)

/*
 * We typically do not contain NOR flash.  In the cases where we do, we
 * undefine this later.
 */
#define CONFIG_SYS_NO_FLASH

/* ENV related config options */
#if defined(_NVT_UBOOT_ENV_IN_STORG_SUPPORT_NAND_)
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET				0x002C0000				/* Defined by configs\cfg_gen\nvt-na51068-storage-partition.dtsi */
#define CONFIG_ENV_SIZE					(128 << 10)				/* Unit: Block size: 128 KiB */
#define CONFIG_ENV_RANGE				2 * CONFIG_ENV_SIZE			/* Defined by configs\cfg_gen\nvt-na51068-storage-partition.dtsi */
#elif defined(_NVT_UBOOT_ENV_IN_STORG_SUPPORT_NOR_)
#define CONFIG_ENV_IS_IN_SPI_FLASH
#ifdef CONFIG_NVT_IVOT_SOC_COMMON_UTILS
#define CONFIG_ENV_OFFSET				0x00040000				/* It must be aligned to an erase secrote boundary */
#else
#define CONFIG_ENV_OFFSET				0x000C0000				/* It must be aligned to an erase secrote boundary */
#endif
#define CONFIG_ENV_SIZE					0x00010000				/* Sync to configs\cfg_gen\nvt-na51068-storage-partition.dtsi */
#define CONFIG_ENV_SECT_SIZE				(64 << 10)				/* Define the SPI flash's sector size */
#elif defined(_NVT_UBOOT_ENV_IN_STORG_SUPPORT_MMC_)
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET				0x002C0000
#define CONFIG_ENV_SIZE					0x00040000
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_ENV_SIZE					(8 << 10)
#endif

/*
 * The following are general good-enough settings for U-Boot.  We set a
 * large malloc pool as we generally have a lot of DDR, and we opt for
 * function over binary size in the main portion of U-Boot as this is
 * generally easily constrained later if needed.  We enable the config
 * options that give us information in the environment about what board
 * we are on so we do not need to rely on the command prompt.  We set a
 * console baudrate of 115200 and use the default baud rate table.
 */
#define CONFIG_SYS_MALLOC_LEN				(3*1024 * 1024)	/* 3MB */
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_CONSOLE_INFO_QUIET
#define CONFIG_BAUDRATE					115200
#define CONFIG_ENV_VARS_UBOOT_CONFIG			/* Strongly encouraged */
#define CONFIG_ENV_OVERWRITE				/* Overwrite ethaddr / serial# */

/* As stated above, the following choices are optional. */
#define CONFIG_SYS_LONGHELP
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING
#define CONFIG_VERSION_VARIABLE

/* We set the max number of command args high to avoid HUSH bugs. */
#define CONFIG_SYS_MAXARGS				64

/* Console I/O Buffer Size */
#define CONFIG_SYS_CBSIZE				1024
/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE				(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
/* Boot Argument Buffer Size */
#define CONFIG_SYS_BARGSIZE				CONFIG_SYS_CBSIZE

#define CONFIG_ZERO_BOOTDELAY_CHECK						/* allow stopping of boot process */
										/* even with bootdelay=0 */
/*#define CONFIG_AUTOBOOT_KEYED				1*/
#define CONFIG_AUTOBOOT_STOP_STR			"~"

/* MMC */
#ifdef CONFIG_NVT_IVOT_MMC
#ifndef CONFIG_MMC
	#define CONFIG_MMC
#endif
#ifndef CONFIG_SDHCI
	#define CONFIG_SDHCI
#endif
#ifndef CONFIG_CMD_MMC
	#define CONFIG_CMD_MMC
#endif
#ifndef CONFIG_GENERIC_MMC
	#define CONFIG_GENERIC_MMC
#endif
#ifndef CONFIG_DOS_PARTITION
	#define CONFIG_DOS_PARTITION
#endif
#ifndef CONFIG_CMD_FAT
	#define CONFIG_CMD_FAT
#endif
#ifndef CONFIG_FS_FAT
	#define CONFIG_FS_FAT
#endif
#ifndef CONFIG_MMC_SDMA
	#define CONFIG_MMC_SDMA
#endif

/*#define CONFIG_NVT_MMC_ADJUST*/
#ifdef CONFIG_NVT_MMC_ADJUST
#define CONFIG_NVT_MMC_DRIVING 0 // 0: 4mA, 1: 8mA, 2: 12mA, 3: 16mA
#endif

#endif

/*#define CONFIG_NVT_EMMC_ONLY_HOST*/
/*#define CONFIG_MMC_TRACE*/
/*#define CONFIG_NVT_MMC_CMD_LOG*/
/*#define CONFIG_NVT_MMC_REG_DUMP*/

#ifdef  CONFIG_NVT_MMC
#ifndef CONFIG_MMC
	#define CONFIG_MMC
#endif
#define CONFIG_DOS_PARTITION
#define CONFIG_GENERIC_MMC
/*#define CONFIG_SDHCI*/
#define CONFIG_SUPPORT_EMMC_BOOT					/* Support emmc boot partition */
#endif

/* FAT */
#if defined(CONFIG_FS_FAT)
#define CONFIG_FAT_WRITE
#endif

/* CRYPTO */
#define CONFIG_MD5

/* MTD */
#define CONFIG_CMD_MTDPARTS

#define CONFIG_LZMA							/* required by uitron decompress */
#define CONFIG_MTD_PARTITIONS						/* required for UBI partition support */
#define CONFIG_MTD_DEVICE
#define CONFIG_SYS_BOOTM_LEN				(25 << 20)

#define CONFIG_CMDLINE_SIZE             		2048
#define CONFIG_CMDLINE_TAG				1		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS			1
#define CONFIG_INITRD_TAG				1
#define CONFIG_REVISION_TAG				1

#define CONFIG_BOOTARGS_COMMON				"earlyprintk console=ttyS0,115200 rootwait nprofile_irq_duration=on "

/* NVT boot related setting */
#ifdef CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT
	#define CONFIG_NVT_LINUX_AUTODETECT					/* Support for detect FW98321A.bin/FW98321T.bin automatically. (Only working on mtd device boot method) */
	#define CONFIG_NVT_BIN_CHKSUM_SUPPORT					/* This option will check rootfs/uboot checksum info. during update image flow */
	#if defined(_NVT_ROOTFS_TYPE_RAMDISK_)
		/* the ramdisk dram base/size will be defined in itron modelext info. */
		#define CONFIG_NVT_LINUX_RAMDISK_BOOT 				/* Loading ramdisk image rootfs.bin from SD card */
		#define CONFIG_BOOTARGS 		CONFIG_BOOTARGS_COMMON "root=/dev/ram0 rootfstype=ramfs rdinit=/linuxrc "
		#if defined(_EMBMEM_EMMC_)
			/* For emmc device */
			#define CONFIG_NVT_LINUX_EMMC_BOOT
			#define CONFIG_NVT_IVOT_EMMC    0
			#define CONFIG_NVT_EXT4_SUPPORT
			#define CONFIG_FASTBOOT_FLASH
		#endif
		#define CONFIG_RBTREE						/* required by CONFIG_CMD_UBI */
		#define CONFIG_LZO						/* required by CONFIG_CMD_UBIFS */
		#define CONFIG_CMD_UBI						/* UBI-formated MTD partition support */
		#define CONFIG_CMD_UBIFS					/* Read-only UBI volume operations */
		#define CONFIG_MTD_UBI_BEB_LIMIT	30
	#elif defined(_NVT_ROOTFS_TYPE_NAND_UBI_)				/* UBIFS rootfs boot */
		#define CONFIG_NVT_LINUX_SPINAND_BOOT
		#define CONFIG_NVT_UBIFS_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "root=ubi0:rootfs rootfstype=ubifs ubi.fm_autoconvert=1 init=/linuxrc "
		#define CONFIG_RBTREE						/* required by CONFIG_CMD_UBI */
		#define CONFIG_LZO						/* required by CONFIG_CMD_UBIFS */
		#define CONFIG_CMD_UBI						/* UBI-formated MTD partition support */
		#define CONFIG_CMD_UBIFS					/* Read-only UBI volume operations */
		#define CONFIG_MTD_UBI_BEB_LIMIT	30
	#elif defined(_NVT_ROOTFS_TYPE_NAND_SQUASH_)				/* SquashFs rootfs boot */
		#define CONFIG_NVT_LINUX_SPINAND_BOOT
		#define CONFIG_NVT_SQUASH_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "ubi.block=0,0 root=/dev/ubiblock0_0 rootfstype=squashfs init=/linuxrc"
		#define CONFIG_RBTREE						/* required by CONFIG_CMD_UBI */
		#define CONFIG_LZO						/* required by CONFIG_CMD_UBIFS */
		#define CONFIG_CMD_UBI						/* UBI-formated MTD partition support */
		#define CONFIG_CMD_UBIFS					/* Read-only UBI volume operations */
		#define CONFIG_MTD_UBI_BEB_LIMIT	30
	#elif defined(_NVT_ROOTFS_TYPE_NAND_JFFS2_)				/* JFFS2 rootfs boot */
		#define CONFIG_NVT_LINUX_SPINAND_BOOT				/* Boot from spinand or spinor (Support FW96680A.bin update all-in-one) */
		#define CONFIG_NVT_JFFS2_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=jffs2 rw "
	#elif defined(_NVT_ROOTFS_TYPE_NOR_SQUASH_)				/* Squashfs rootfs boot */
		#ifdef CONFIG_NVT_SPI_NOR
			#define CONFIG_NVT_LINUX_SPINOR_BOOT				/* Boot from spinand or spinor (Support FW96680A.bin update all-in-one) */
		#endif
		#define CONFIG_NVT_SQUASH_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=squashfs ro "
	#elif defined(_NVT_ROOTFS_TYPE_NOR_JFFS2_)				/* JFFS2 rootfs boot */
		#ifdef CONFIG_NVT_SPI_NOR
			#define CONFIG_NVT_LINUX_SPINOR_BOOT				/* Boot from spinand or spinor (Support FW96680A.bin update all-in-one) */
		#endif
		#define CONFIG_NVT_JFFS2_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=jffs2 rw "
	#elif defined(_NVT_ROOTFS_TYPE_EXT4_)
		#define CONFIG_NVT_LINUX_EMMC_BOOT				/* Boot from emmc (Support FW96680A.bin update all-in-one) */
		#define CONFIG_NVT_IVOT_EMMC    0
		#define CONFIG_NVT_EXT4_SUPPORT
		#define CONFIG_FASTBOOT_FLASH
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=ext4 rw "
	#elif defined(_NVT_ROOTFS_TYPE_EMMC_SQUASH_)				/* Squashfs rootfs boot */
		#define CONFIG_NVT_LINUX_EMMC_BOOT				/* Boot from emmc (Support FW96680A.bin update all-in-one) */
		#define CONFIG_NVT_IVOT_EMMC    0
		#define CONFIG_FASTBOOT_FLASH
		#define CONFIG_NVT_SQUASH_SUPPORT
		#define CONFIG_BOOTARGS			CONFIG_BOOTARGS_COMMON "rootfstype=squashfs ro "
	#else
		#define CONFIG_NVT_LINUX_SD_BOOT				/* To handle RAW SD boot (e.g. itron.bin, uImage.bin, uboot.bin...) itron.bin u-boot.bin dsp.bin dsp2.bin must be not compressed.*/
		#define CONFIG_BOOTARGS 		CONFIG_BOOTARGS_COMMON "root=/dev/mmcblk0p2 noinitrd rootfstype=ext3 init=/linuxrc "
	#endif /* _NVT_ROOTFS_TYPE_ */
#endif /* CONFIG_NVT_IVOT_SOC_FW_UPDATE_SUPPORT */

#define CONFIG_IMAGE_FORMAT_LEGACY
#define CONFIG_BOOTCOMMAND				"nvt_boot"

#include "ms_nvrcfg.h"

#endif /* __CONFIG_NA51068_H */
