// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef OT_BOARD_H
#define OT_BOARD_H

#include "ot_common.h"

#define CRG_REGS_ADDR             0x11010000
#define VOU_REGS_ADDR             0x17A00000

#define CRG_PERCTL_PLL224_ADDR      (0x0380 + CRG_REGS_ADDR)
#define CRG_PERCTL_PLL225_ADDR      (0x0384 + CRG_REGS_ADDR)
#define CRG_PERCTL_PLL256_ADDR      (0x0400 + CRG_REGS_ADDR)
#define CRG_PERCTL_PLL257_ADDR      (0x0404 + CRG_REGS_ADDR)

#define CRG_PERCTL8336_ADDR         (0x8240 + CRG_REGS_ADDR)
#define CRG_PERCTL8338_ADDR         (0x8248 + CRG_REGS_ADDR)
#define CRG_PERCTL8340_ADDR         (0x8250 + CRG_REGS_ADDR)
#define CRG_PERCTL8341_ADDR         (0x8254 + CRG_REGS_ADDR)
#define CRG_PERCTL8342_ADDR         (0x8258 + CRG_REGS_ADDR)
#define CRG_PERCTL8348_ADDR         (0x8270 + CRG_REGS_ADDR)
#define CRG_PERCTL8349_ADDR         (0x8274 + CRG_REGS_ADDR)
#define CRG_PERCTL8350_ADDR         (0x8278 + CRG_REGS_ADDR)
#define CRG_PERCTL8351_ADDR         (0x827C + CRG_REGS_ADDR)
#define CRG_PERCTL8352_ADDR         (0x8280 + CRG_REGS_ADDR)
#define CRG_PERCTL8353_ADDR         (0x8284 + CRG_REGS_ADDR)

#endif /* OT_BOARD_H */
