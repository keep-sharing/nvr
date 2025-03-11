/*
 * Copyright (c) 2021, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DRIVERS_NVT_TZASC_INT_H
#define __DRIVERS_NVT_TZASC_INT_H

#include <stdint.h>

#ifndef ENUM_DUMMY4WORD
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

#ifndef ENABLE
#define ENABLE              1           ///< Feature is enabled
#endif

/*
    DMA channel mask

    Indicate which DMA channels are required to protect/detect

    @note For DMA_WRITEPROT_ATTR
*/
#if defined(_BSP_NA51102_)
typedef struct {
	// ch 0
	uint32_t bRsv0:32;
	// ch 1
	uint32_t CPU_S: 1;
	uint32_t CPU_NS: 1;
	uint32_t bRsv1: 30;
	// ch 2
	uint32_t bRsv2:32;
	// ch 3
	uint32_t bRsv3:32;
	// ch 4
	uint32_t bRsv4:32;
	// ch 5
	uint32_t bRsv5:32;
	// ch 6
	uint32_t bRsv6:32;
	// ch 7
	uint32_t bRsv7:32;
} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
	DDR_ARB_1,                           ///< DDR Arbiter

	DDR_ARB_COUNT,                       //< Arbiter count

	ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;

typedef enum _DMA_WRITEPROT_SET {
	WPSET_0,            				// Write protect function set 0
	WPSET_1,            				// Write protect function set 1
	WPSET_2,            				// Write protect function set 2
	WPSET_3,            				// Write protect function set 3
	WPSET_4,            				// Write protect function set 4
	WPSET_5,            				// Write protect function set 5
	WPSET_COUNT,
	ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
	DMA_WPLEL_UNWRITE,      			// Not only detect write action but also denial access.
	DMA_WPLEL_DETECT,       			// Only detect write action but allow write access.
	DMA_RPLEL_UNREAD,       			// Not only detect read action but also denial access.
	DMA_RWPLEL_UNRW,        			// Not only detect read write action but also denial access.
	ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

#define DMA_WP							DMA_WPLEL_UNWRITE
#define DMA_RP							DMA_RPLEL_UNREAD

typedef enum _DMA_PROT_MODE {
	DMA_PROT_IN,
	DMA_PROT_OUT,
	ENUM_DUMMY4WORD(DMA_PROT_MODE)
} DMA_PROT_MODE;

typedef struct _DMA_PROT_RGN_ATTR {
	bool            	en;            	// enable this region
	uint64_t            starting_addr; 	// DDR3:must be 4 words alignment
	unsigned int        size;          	// DDR3:must be 4 words alignment
} DMA_PROT_RGN_ATTR, *PDMA_PROT_RGN_ATTR;

typedef struct _DMA_WRITEPROT_ATTR {
	DMA_CH_MSK          mask;       	// DMA channel masks to be protected/detected
	DMA_WRITEPROT_LEVEL level;	    	// protect level
	DMA_PROT_MODE       protect_mode; 	// in or out region
	DMA_PROT_RGN_ATTR   protect_rgn_attr;
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;

#else
typedef struct {
        // ch 0
        uint32_t bReserved0: 1;                       //< bit0: reserved (auto refresh)
        uint32_t bCPU_NS: 1;                          //< CPU ns access
        uint32_t bRsv0:30;
        // ch 32
        uint32_t bRsv1:32;
} DMA_CH_MSK, *PDMA_CH_MSK;

/**
    DDR Arbiter ID

*/
typedef enum _DDR_ARB {
        DDR_ARB_1,                           ///< DDR Arbiter
        DDR_ARB_2,                           ///< DDR Arbiter

        DDR_ARB_COUNT,                       //< Arbiter count

        ENUM_DUMMY4WORD(DDR_ARB)
} DDR_ARB;


typedef enum _DMA_WRITEPROT_SET {
        WPSET_0,            // Write protect function set 0
        WPSET_1,            // Write protect function set 1
        WPSET_2,            // Write protect function set 2
        WPSET_3,            // Write protect function set 3
        WPSET_4,            // Write protect function set 4
        OUT_WP,             // Out range write protect
        WPSET_COUNT,
        ENUM_DUMMY4WORD(DMA_WRITEPROT_SET)
} DMA_WRITEPROT_SET;

typedef enum _DMA_WRITEPROT_LEVEL {
        DMA_WP,      // Not only detect write action but also denial access.
        DMA_WD,       // Only detect write action but allow write access.
        DMA_RP,       // Not only detect read action but also denial access.
        DMA_RD,        // Only detect read action but allow read access.
        ENUM_DUMMY4WORD(DMA_WRITEPROT_LEVEL)
} DMA_WRITEPROT_LEVEL;

typedef struct _DMA_WRITEPROT_ATTR {
        DMA_CH_MSK          mask;       // DMA channel masks to be protected/detected
        DMA_WRITEPROT_LEVEL level;          // protect level
        uint64_t            starting_addr; // DDR3:must be 4 words alignment
        uint32_t            size;          // DDR3:must be 4 words alignment
} DMA_WRITEPROT_ATTR, *PDMA_WRITEPROT_ATTR;
#endif

#if defined(_BSP_NA51102_)
typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,    // represent channel 00-31
	DMA_CH_GROUP1,          // represent channel 32-63
	DMA_CH_GROUP2,          // represent channel 64-95
	DMA_CH_GROUP3,          // represent channel 96-127
	DMA_CH_GROUP4,          // represent channel 128-159
	DMA_CH_GROUP5,          // represent channel 160-191
	DMA_CH_GROUP6,          // represent channel 192-223
	DMA_CH_GROUP7,          // represent channel 224-255

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;
#else
typedef enum _DMA_CH_GROUP {
	DMA_CH_GROUP0 = 0x0,
	DMA_CH_GROUP1,

	DMA_CH_GROUP_CNT,
	ENUM_DUMMY4WORD(DMA_CH_GROUP)
} DMA_CH_GROUP;
#endif

#endif
