/*
 * Copyright (c) 2018 Shenshu Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <config.h>

//******************************************************************************
//
//  void uart_early_init(void);
//
.text
.align	4
.global	uart_early_init
.type	uart_early_init, %function
uart_early_init:
#ifndef CONFIG_BSP_DISABLE_CONSOLE
	/* Config UART0 CRG */
	ldr	x4, reg_uart_crg_L0
	ldr	w3, [x4]
	orr	w3, w3, #BIT_UART_CKEN
	bic	w3, w3, #UART_CKL_SEL_MASK
	orr	w3, w3, #UART_CLK_24M
	bic	w3, w3, #BIT_UART_SRST
	str	w3, [x4]

	/* Config UART0 IO */
	ldr	x4, io_base_addr_L0
	ldr	w3, =0x1101
	str	w3, [x4, #0x0]
	ldr	w3, =0x1011
	str	w3, [x4, #0x4]

	ldr	x4, uart_base_addr_L0
	mov	w3, #0
	/* Disable UART */
	str	w3, [x4, #48]
	/* Set baud rate to 115200, uart clock:24M */
	add	w3, w3, #13
	str	w3, [x4, #36]
	mov	w3, #1
	str	w3, [x4, #40]
	/* Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled. */
	ldr	w3, =112
	str	w3, [x4, #44]
	/* Enable UART */
	ldr	w3, =769
	str	w3, [x4, #48]
#endif
	ret
.align	4
uart_base_addr_L0:
	.quad UART0_REG_BASE
io_base_addr_L0:
	.quad 0x102f0000
reg_uart_crg_L0:
	.quad REG_UART0_CRG
//******************************************************************************
//
//  void uart_early_puts(const char *ss);
//
.align	4
.global	uart_early_puts
.type	uart_early_puts, %function
uart_early_puts:
#ifndef CONFIG_BSP_DISABLE_CONSOLE
#if !defined(CONFIG_SUPPORT_CA_RELEASE)
	ldr	x2, uart_base_addr_L1
	b	next_char
output:
	ldr	w4, [x2, #24]
	tst	w4, #32
	bne	output
	str	w3, [x2, #0]
	add	x0, x0, #1
next_char:
	ldrb	w3, [x0]
	cmp	w3, #0
	bne	output
#endif /* CONFIG_SUPPORT_CA_RELEASE */
#endif /* CONFIG_BSP_DISABLE_CONSOLE */
	ret
.align	4
uart_base_addr_L1:
	.quad UART0_REG_BASE

//******************************************************************************
//
//  void uart_early_putc(int chr);
//
//  call example:
//    mov	w0, #'A'
//    bl	uart_early_putc
//
.align	4
.global	uart_early_putc
.type	uart_early_putc, %function
uart_early_putc:
#ifndef CONFIG_BSP_DISABLE_CONSOLE
#if !defined(CONFIG_SUPPORT_CA_RELEASE)
	ldr	x2, uart_base_addr_L3
wait3:
	ldr	w4, [x2, #24]
	tst	w4, #32
	bne	wait3
	str	w0, [x2, #0]

#endif /* CONFIG_SUPPORT_CA_RELEASE */
#endif /* CONFIG_BSP_DISABLE_CONSOLE */
	ret
.align	4
uart_base_addr_L3:
	.quad UART0_REG_BASE
