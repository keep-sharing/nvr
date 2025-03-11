#ifndef __CONFIG_MSNVR_H
#define __CONFIG_MSNVR_H

#define CONFIG_MS_NVR

#define MS_UBOOT_VERSION	"75.1.1.9"
#define MS_DDR_VERSION	1

#undef CONFIG_IDENT_STRING
#define CONFIG_IDENT_STRING MS_UBOOT_VERSION

/*--------------------------------------------------------------------
 * DOWNLOAD CONFIG
 *----------------------------------------------------------------------*/
#define CONFIG_CMD_DOWNLOAD

#undef CONFIG_BOOTFILE
#define CONFIG_BOOTFILE "MSFImage"        /* file to load */

#undef CONFIG_SPI_NOR_QUIET_TEST
#undef CONFIG_CMD_SF
#define CONFIG_CMD_SF
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_IS_IN_SPI_FLASH
#undef CONFIG_SPI_NOR_MAX_CHIP_NUM
#define CONFIG_SPI_NOR_MAX_CHIP_NUM	1
#undef CONFIG_SPI_NOR_QUIET_TEST
#define CONFIG_SPI_NOR_QUIET_TEST

#undef CONFIG_CMD_NAND
#define CONFIG_CMD_NAND
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
#undef CONFIG_ENV_NAND_ADDR
#undef CONFIG_ENV_SPI_ADDR
#undef CONFIG_CMD_SAVEENV
#define CONFIG_ENV_OFFSET          0x60000      // 384Kb uboot, 128KB env. /*512KB environment starts here */
#define CONFIG_ENV_NAND_ADDR	(CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SPI_ADDR	(CONFIG_ENV_OFFSET)
#define CONFIG_CMD_SAVEENV

#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SECT_SIZE
#undef CONFIG_ENV_OFFSET_REDUND
#undef CONFIG_ENV_SIZE_REDUND
#undef CONFIG_NR_DRAM_BANKS
#undef CONFIG_SYS_REDUNDAND_ENVIRONMENT
#undef CONFIG_ENV_RANGE

#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SIZE		0x10000    /*include ENV_HEADER_SIZE */
#define CONFIG_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_ENV_SIZE_REDUND	    CONFIG_ENV_SIZE
#define CONFIG_NR_DRAM_BANKS	1       /* we have 1 bank of DRAM */
#define CONFIG_ENV_RANGE    CONFIG_ENV_SIZE

/*-----------------------------------------------------------------------
 *  Environment   Configuration
 *-----------------------------------------------------------------------*/
#undef CONFIG_BOOTCOMMAND
#define CONFIG_BOOTCOMMAND "nboot ker1;nvt_boot"
#undef CONFIG_BOOTDELAY
#define CONFIG_BOOTDELAY	3
#undef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS		"earlyprintk console=ttyS0,115200 rootwait nprofile_irq_duration=on ubi.mtd=fs1 root=ubi0:rootfs rootfstype=ubifs rw init=/linuxrc"


#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS   \
    "mdio_intf=rmii\0" \
    "mtdids=nand0=spi_nand.0\0" \
    "nor_mtdparts=spi_nor.0:384K(uboot),128K(env)\0" \
    "mtdparts=mtdparts=spi_nand.0:4M(logo),6M(ker1),6M(ker2),84M(fs1),84M(fs2),24M(cfg),32M(oem),-(rsd)\0" \
    "uboot=name:uboot\tver:0x07000100\tsize:296960\tdate:0x07e10704 \0"\
    "logo=name:logo\tver:0x07000000\tsize:264192\tdate:0x07e10704 \0"\
    "ker1=name:ker1\tver:0x07000000\tsize:3784704\tdate:0x07e10704 \0"\
    "ker2=name:ker2\tver:0x07000000\tsize:3784704\tdate:0x07e10704 \0"\
    "fs1=name:fs1\tver:0x4B000000\tsize:58327040\tdate:0x07e10704 \0"\
    "fs2=name:fs2\tver:0x4B000000\tsize:58327040\tdate:0x07e10704 \0"\
    "uboot_name=uboot \0" \
    "update_uboot=mw.b 0xB00000 0xff 0x80000;tftp 0xB00000 ${uboot_name};sf probe 0;sf erase 0 0x60000;sf write 0xB00000 0 0x60000 \0" \
    "update_ker1=tftp 0xB00000 uImage;nand erase.part ker1;nand write 0xB00000 ker1 \0" \
    "erase_env=sf probe 0;sf erase 0x60000 0x20000 \0" \
    "erase_uboot=sf probe 0;sf erase 0 0x60000 \0" 
/*-----------------------------------------------------------------------
 * Dynamic MTD partition support
 */
#undef CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_MTDPARTS
#undef CONFIG_MTD_DEVICE
#define CONFIG_MTD_DEVICE		/* needed for mtdparts commands */
#undef MTDIDS_DEFAULT
#undef MTDPARTS_DEFAULT
#define MTDIDS_DEFAULT		"nand0=spi_nand.0"
#define MTDPARTS_DEFAULT	"mtdparts=spi_nand.0:4M(logo),"	\
        					"6M(ker1),"	\
                            "6M(ker2)," \
                            "84M(fs1)," \
                            "84M(fs2)," \
        					"24M(cfg)," \
        					"32M(oem)," \
        					"-(rsd)"  


/*-----------------------------------------------------------------------
 * console display  Configuration
 ------------------------------------------------------------------------*/
#undef CONFIG_VERSION_VARIABLE
#define CONFIG_VERSION_VARIABLE  1 /*used in common/main.c*/
#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT  "NT98323 # "	/* Monitor Command Prompt */


#endif /* __CONFIG_MSNVR_H */

