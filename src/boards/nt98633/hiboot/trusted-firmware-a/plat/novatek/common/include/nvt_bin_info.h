/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BINI_NFO_H
#define _BIN_INFO_H

/**
     Header information
*/
//HEADINFO::Resv field definition for bl31
typedef enum HEADINFO_BL31_RESV_IDX {
	HEADINFO_BL31_RESV_IDX_LOAD_ADDR = 0,
	HEADINFO_BL31_RESV_IDX_UBOOT_ADDR = 1, //< write by loader
	HEADINFO_BL31_RESV_IDX_TEEOS_ADDR = 2, //< write by loader
	HEADINFO_BL31_RESV_IDX_RTOS_ADDR = 3,  //< write by loader
	HEADINFO_BL31_RESV_IDX_COUNTS = 19,
} HEADINFO_BL31_RESV_IDX;

typedef struct HEADINFO {
	unsigned long CodeEntry;   ///< [0x00] fw CODE entry (4) ----- r by Ld
	unsigned long Resv1[19];   ///< [0x04~0x50] reserved (4*19) -- reserved, its mem value will filled by Ld
	char BinInfo_1[8];  ///< [0x50~0x58] CHIP-NAME (8) ---- r by Ep
	char BinInfo_2[8];  ///< [0x58~0x60] SDK version (8)
	char BinInfo_3[8];  ///< [0x60~0x68] SDK releasedate (8)
	unsigned int BinLength;   ///< [0x68] Bin File Length (4) --- w by Ep/bfc
	unsigned int Checksum;    ///< [0x6c] Check Sum or CRC (4) ----- w by Ep/Epcrc
	unsigned int CRCLenCheck; ///< [0x70~0x74] Length check for CRC (4) ----- w by Epcrc (total len ^ 0xAA55)
	unsigned int ModelextAddr;///< [0x74~0x78] where modelext data is. w by Ld / u-boot
	unsigned int BinCtrl;     ///< [0x78~0x7C] Bin flag (4) --- w by bfc
	///<             BIT 0.compressed enable (w by bfc)
	///<             BIT 1.Linux SMP enable (1: SMP/VOS, 0: Dual OS)
	unsigned int CRCBinaryTag;///< [0x7C~0x80] Binary Tag for CRC (4) ----- w by Epcrc (0xAA55 + "NT")
}
HEADINFO;


#endif
