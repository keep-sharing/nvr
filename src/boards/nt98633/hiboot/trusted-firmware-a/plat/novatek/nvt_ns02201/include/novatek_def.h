/*
 * Copyright (c) 2022, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NOVATEK_MMAP_H
#define NOVATEK_MMAP_H

#define BL31_BASE       _BOARD_BL31_ADDR_
#define BL31_SIZE       _BOARD_BL31_SIZE_
#define BL31_LIMIT      (BL31_BASE + BL31_SIZE)

#define _BSP_NS02201_

/* IP BASE */
#define UART0_BASE  0x2f0130000
#define GICD_BASE   0x2fff01000
#define GICC_BASE   0x2fff02000
#define TZPC_BASE   0x2f0220000
#define DDR_ARB_REG_BASE    0x2f0200000
#define DDR_ARB2_REG_BASE   0x2f0210000
#define RELEASE_ADDR    0x2f0090000
#define DEVICE_START_ADDR   0x2f0000000
#define DEVICE_END_ADDR     0x2ffff0000

/* UART configuration */
#define UART0_BAUDRATE      115200

#if defined(_NVT_FPGA_EMULATION_ON_)
#define UART0_CLK_IN_HZ     24000000
#define OSC_CLK_IN_HZ       24000000
#else
#define UART0_CLK_IN_HZ     48000000
#define OSC_CLK_IN_HZ       12000000
#endif

#define REG_ADDR_REMAP      0xFFE41014
#define REG_RELEASE_ADDR    0x2F009010C

#endif /* NOVATEK_MMAP_H */
