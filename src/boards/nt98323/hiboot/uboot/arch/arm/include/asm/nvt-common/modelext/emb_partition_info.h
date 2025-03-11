/**

    @file       emb_partition_info.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __ASM_NVT_COMMON_EMB_PARTITION_INFO_H__
#define __ASM_NVT_COMMON_EMB_PARTITION_INFO_H__

#define EMB_PARTITION_INFO_VER 0x16072117 ///< YYYY/MM/DD HH

/**
    Partition Infomation
    This is common header used between firmware of uITRON, eCos, Linux, DSP
    so !!!!!! DO NOT modify it !!!!!!
*/

#define EMB_PARTITION_INFO_COUNT  20

enum {
	FLASHTYPE_NOR    = 0x00,
	FLASHTYPE_NAND   = 0x01,
	FLASHTYPE_EMMC   = 0x02,
};

enum {
	EMBTYPE_UNKNOWN  = 0x00,
	EMBTYPE_LOADER   = 0x01,  /* loader must always put in partition[0] */
	EMBTYPE_FDT      = 0x02,  /* modelext must always put in partition[1] */
	EMBTYPE_UITRON   = 0x03,
	EMBTYPE_ECOS     = 0x04,
	EMBTYPE_UBOOT    = 0x05,
	EMBTYPE_LINUX    = 0x06,
	EMBTYPE_DSP      = 0x07,
	EMBTYPE_PSTORE   = 0x08,
	EMBTYPE_FAT      = 0x09,
	EMBTYPE_EXFAT    = 0x0A,
	EMBTYPE_ROOTFS   = 0x0B,
	EMBTYPE_RAMFS    = 0x0C,
	EMBTYPE_UENV     = 0x0D, /* u-boot environment data */
	EMBTYPE_MBR      = 0x0E, /* for emmc partition, mbr always put in partition[0] instead of loader */
	EMBTYPE_ROOTFSL  = 0x0F, /* for emmc logical partition */
	EMBTYPE_RTOS     = 0x10, /* freertos */
	EMBTYPE_APP      = 0x11, /* for APP , like IQ configs */
	EMBTYPE_TEEOS    = 0x12, /* for secure os*/
	EMBTYPE_SIZE,
};
/**
    customer defined data partition format
*/
enum {
	EMBTYPE_USER	= 0x80,
	EMBTYPE_USERRAW	= 0x81,
	EMBTYPE_USR_SIZE,
};

#define GET_USER_PART_NUM(m)	((m) - 0x80)

#define EMBTYPE_TOTAL_SIZE	(EMBTYPE_SIZE + EMBTYPE_USR_SIZE - 0x80)
/* for reason of compatiable linux, we use original type to decalre */

typedef struct _EMB_PARTITION {
	unsigned short		FlashType;       /* FLASHTYPE_ */
	unsigned short		EmbType;         /* EMBTYPE_ */
	unsigned short		OrderIdx;        /* Order index of the same EmbType based on '0' */
	unsigned short		FileExist;       /* File name if exist */
	unsigned long long	PartitionOffset; /* Phyical offset of partition */
	unsigned long long	PartitionSize;   /* Size of this partition */
	unsigned long long	ReversedSize;    /* Reserved size for bad block */
} EMB_PARTITION, *PEMB_PARTITION;

typedef struct _EMB_PARTITION_FDT_TRANSLATE_TABLE {
	char fdt_node_name[30];
	unsigned short emb_type;
} EMB_PARTITION_FDT_TRANSLATE_TABLE;
#endif /* __ASM_NVT_COMMON_EMB_PARTITION_INFO_H__ */
