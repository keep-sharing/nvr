/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <nvt_bin_info.h>
#include <novatek_private.h>

extern void bl31_entrypoint(void);
#define __string(_x) #_x
#define __xstring(_x) __string(_x)

const HEADINFO _nvt_headinfo __attribute__((section(".nvt.headinfo"))) = {
	(unsigned long) &bl31_entrypoint, //<- fw CODE entry (4)
	{_BOARD_BL31_ADDR_, 0}, ///<- reserved (4*19): Reference nvt_bin_info.h HEADINFO_BL31_RESV_IDX
	{NVT_HEADINFO_CHIP_NAME}, //<- CHIP-NAME / TAG-NAME (8)
	{"0"}, //<- version (8)
	{"0"}, //<- releasedate (8)
	0x0, //<- Bin File Length (4)
	0x0, //<- Check Sum or CRC (4)
	0,///<- Length check for CRC (4)
	0,///<- where modelext data is. w by Ld / u-boot (4)
	0,
	0,///<- Binary Tag for CRC (4)
};
