// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DDRTRN_BOOT_ADAPT_H
#define DDRTRN_BOOT_ADAPT_H
#include "security.h"
#include "ddrtrn_boot_common.h"
#include "ddrtrn_log.h"

#define BOOTLOADER DDR_BOOT_TYPE_HSL

/* register operations */
#define ddrtrn_reg_read(addr)                (*(volatile unsigned int *)((uintptr_t)(addr)))
#define ddrtrn_reg_write(val, addr)          ((*(volatile unsigned int *)((uintptr_t)(addr))) = (val))

#else /* DDRTRN_BOOT_ADAPT_H */
#if BOOTLOADER != DDR_BOOT_TYPE_HSL
#error "do not input more than one ddrtrn_boot_adapt.h"
#endif /* BOOTLOADER */
#endif /* DDRTRN_BOOT_ADAPT_H */
