#ifndef __CONFIG_MSNVR_H
#define __CONFIG_MSNVR_H

#define CONFIG_MS_NVR

#define MS_UBOOT_VERSION	"77.3.3.8"
#define MS_DDR_VERSION	3

#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING MS_UBOOT_VERSION

/*--------------------------------------------------------------------
 * DOWNLOAD CONFIG
 *----------------------------------------------------------------------*/
//#define CONFIG_CMD_DOWNLOAD

#undef CONFIG_BOOTFILE
#define CONFIG_BOOTFILE "MSFImage"        /* file to load */


/*#undef CONFIG_SPI_NOR_QUIET_TEST
#undef CONFIG_CMD_SF
#define CONFIG_CMD_SF
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH
#undef CONFIG_SPI_NOR_MAX_CHIP_NUM
#define CONFIG_SPI_NOR_MAX_CHIP_NUM	1
#undef CONFIG_SPI_NOR_QUIET_TEST
#define CONFIG_SPI_NOR_QUIET_TEST*/

/*#undef CONFIG_CMD_NAND
#define CONFIG_CMD_NAND*/
/*allow change env*/
#undef CONFIG_ENV_OVERWRITE
#define  CONFIG_ENV_OVERWRITE



#undef MEM_BASE_DDR
#define MEM_BASE_DDR _BOARD_DRAM_ADDR_
/*-----------------------------------------------------------------------
 * environment && bd_info  && global_data  configure
 * used in file
 *----------------------------------------------------------------------*/
/* env in flash instead of CFG_ENV_IS_NOWHERE */
#undef CONFIG_ENV_OFFSET
#undef CONFIG_CMD_SAVEENV

#define CONFIG_ENV_OFFSET          0x80000      // 512Kb uboot, 512KB env. /*512KB environment starts here */
#define CONFIG_CMD_SAVEENV

#undef CONFIG_SYS_REDUNDAND_ENVIRONMENT
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SECT_SIZE
#undef CONFIG_ENV_OFFSET_REDUND
#undef CONFIG_NR_DRAM_BANKS
#undef CONFIG_ENV_RANGE

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SIZE		0x40000    /*include ENV_HEADER_SIZE */
#define CONFIG_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_NR_DRAM_BANKS	1       /* we have 1 bank of DRAM */
#define CONFIG_ENV_RANGE    CONFIG_ENV_SIZE

/*-----------------------------------------------------------------------
 *  Environment   Configuration
 *-----------------------------------------------------------------------*/
#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "mmc read 0 0x44000000 ker1; bootm 0x44000000"
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	3
#undef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS		"mem=2880M console=ttyAMA0,115200 clk_ignore_unused rw rootwait root=/dev/mmcblk0p6 rootfstype=ext4"
#if 0


#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS   \
    "mdio_intf=rgmii\0" \
    "mtdids=nand0=nand\0" \
    "mtdparts=mtdparts=nand:512K(uboot),512K(env),13M(ker1),84M(fs1)\0" \
    "uboot=name:uboot\tver:0x0\tsize:0\tdate:0x0 \0"\
    "logo=name:logo\tver:0x0\tsize:0\tdate:0x0 \0"\
    "ker1=name:ker1\tver:0x0\tsize:0\tdate:0x0 \0"\
    "ker2=name:ker2\tver:0x0\tsize:0\tdate:0x0 \0"\
    "fs1=name:fs1\tver:0x0\tsize:0\tdate:0x0 \0"\
    "fs2=name:fs2\tver:0x0\tsize:0\tdate:0x0 \0"\
    "uboot_name=uboot \0" \
    "update_uboot=mw.b 0x42000000 ff 0x80000;tftp 0x42000000 ${uboot_name};nand erase 0 80000;nand write 42000000 0 80000 \0" \
    "update_ker1=tftp 0x42000000 uImage;nand erase.part ker1;nand write 0x42000000 ker1 \0" \
    "erase_env=sf probe 0;sf erase 0x60000 0x20000 \0" \
    "erase_uboot=sf probe 0;sf erase 0 0x60000 \0" 
/*-----------------------------------------------------------------------
 * Dynamic MTD partition support
 */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_MTDPARTS
//#undef CONFIG_MTD_DEVICE
//#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#undef MTDIDS_DEFAULT
#undef MTDPARTS_DEFAULT
#define MTDIDS_DEFAULT		"nand0=nand"
#define MTDPARTS_DEFAULT	"mtdparts=nand:512K(uboot)," \
                            "512K(env)," \
        					"16M(ker1)," \
                            "84M(fs1)"


/*-----------------------------------------------------------------------
 * console display  Configuration
 ------------------------------------------------------------------------*/
 #endif

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS   \
	"serverip=192.168.62.20\0" \
	"ipaddr=192.168.62.101\0" \
	"ethaddr=0A:02:03:04:05:06\0" \
	"netmask=255.255.255.0\0"  \
	"gatewayip=192.168.62.1\0" \
 	"blkdevparts=blkdevparts=mmcblk0:512k(uboot),512K(env),8M(logo),16M(ker1),16M(ker2),256M(fs1),256M(fs2),64M(cfg),64M(oem),64M(data1),256M(share),-(rsd)\0" \
 	"uboot_name=uboot_4096M \0"\
 	"fs_name=ext4fs \0"\
	"ker_name=uImage_4096M \0"\
 	"update_uboot=mw.b 0x42000000 0xff 0x100000;tftp 0x42000000  ${uboot_name};mmc erase.part uboot;mmc write 0 0x42000000 uboot\0"\
    "update_ker1=tftp 0x42000000 ${ker_name};mmc erase.part ker1;mmc write 0 0x42000000  ker1 \0" \
    "update_fs1=tftp 0x42000000  ${fs_name};mmc erase.part fs1;mmc swrite 0x42000000  fs1 \0" \
    "update_ker2=tftp 0x42000000 ${ker_name};mmc erase.part ker2;mmc write 0 0x42000000  ker2 \0" \
    "update_fs2=tftp 0x42000000  ${fs_name};mmc erase.part fs2;mmc swrite 0x42000000  fs2 \0" 
 	
#undef CONFIG_VERSION_VARIABLE
#define CONFIG_VERSION_VARIABLE  1 /*used in common/main.c*/

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT  "HI3536A # "	/* Monitor Command Prompt */


#endif /* __CONFIG_MSNVR_H */

