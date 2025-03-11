// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DDRTRN_UART_H
#define DDRTRN_UART_H

#define DDR_UART_BASE_REG  0x12090000
#define UART_PL01X_FR      0x18 /* Flag register (Read only). */
#define UART_PL01X_FR_RXFE 0x10
#define UART_PL01X_DR      0x00 /* Data read or written from the interface. */
#define UART_PL01X_ECR     0x04 /* Error clear register (Write). */
#endif /* DDRTRN_UART_H */
