/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#include <lib/utils_def.h>
#include <plat/common/common_def.h>

#include <novatek_def.h>

#define CFG_NVT_IVOT 1

#define FDT_BASE		_BOARD_FDT_ADDR_
#define FDT_SIZE		_BOARD_FDT_SIZE_

#if USE_OPTEE >= 1
#define BL32_BASE       _BOARD_TEEOS_ADDR_
#define BL32_SIZE       _BOARD_TEEOS_SIZE_
#endif

#define BL_COHERENT_BASE        _BOARD_SHMEM_ADDR_
#define BL_COHERENT_SIZE        _BOARD_SHMEM_SIZE_

#define PLAT_NS_IMAGE_OFFSET    _BOARD_UBOOT_ADDR_

#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)


/* PSCI function */

#define PLAT_MAX_PWR_LVL_STATES		U(2)
#define PLAT_MAX_RET_STATE		U(1)
#define PLAT_MAX_OFF_STATE		U(2)

#define PLAT_MAX_PWR_LVL		U(2)
#define PLAT_NUM_PWR_DOMAINS		(1 + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)

#define PLAT_PRIMARY_CPU       0x0
#define ES_TO_AARCH64          1

#define PLAT_HOLD_STATE_WAIT    ULL(0)
#define PLAT_HOLD_STATE_GO      ULL(1)
#define PLAT_HOLD_BASE		(BL31_BASE + BL31_SIZE - 0x1000)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define MAX_MMAP_REGIONS		16
#define MAX_XLAT_TABLES			20
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 34)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 34)

#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_MAX_CPUS_PER_CLUSTER)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PLATFORM_STACK_SIZE		(0x1000 / PLATFORM_CORE_COUNT)

#endif /* PLATFORM_DEF_H */
