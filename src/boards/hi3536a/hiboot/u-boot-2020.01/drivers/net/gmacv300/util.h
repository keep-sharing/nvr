// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __GMAC_UTIL_H__
#define __GMAC_UTIL_H__

#include <common.h>

#define GMAC_TRACE_ETH		2
#define GMAC_TRACE_MDIO		4
#define GMAC_TRACE_DRV		7
#define GMAC_TRACE_LEVEL		8

#define mk_bits(shift, nbits) ((((shift) & 0x1F) << 16) | ((nbits) & 0x3F))

#endif

