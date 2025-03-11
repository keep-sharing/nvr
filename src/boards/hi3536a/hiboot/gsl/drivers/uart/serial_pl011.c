/*
 *
 * Copyright (c) 2020-2021 Shenshu Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

/* Simple U-Boot driver for the PrimeCell PL011 UARTs on the IntegratorCP */
/* Should be fairly simple to make it work with the PL010 as well */

#include <platform.h>
#include <types.h>
#include <lib.h>

#ifdef CFG_PL011_SERIAL

#define io_write(addr, val) (*(volatile unsigned int *)((uintptr_t)addr) = (val))
#define io_read(addr) (*(volatile unsigned int *)((uintptr_t)addr))

/*
 * IntegratorCP has two UARTs, use the first one, at 38400-8-N-1
 * Versatile PB has four UARTs.
 */

#define CONSOLE_PORT CONFIG_CONS_INDEX
#define BAUDRATE CONFIG_BAUDRATE
#define MAX_PORT_NUM 1

/*
notes: all writable global variables should be placed in bss section,
should not have default value except zero
 */
static unsigned int uart_port[MAX_PORT_NUM];

static void pl011_putc(int portnum, char c);
static int pl011_getc(int portnum);
static int pl011_tstc(int portnum);

void uart_port_init(void)
{
	uart_port[CONSOLE_PORT] = REG_BASE_SERIAL0;
}

int serial_init(void)
{
	unsigned int temp;
	unsigned int divider;
	unsigned int remainder;
	unsigned int fraction;
	unsigned int reg_data;

	/* Enable uart0 clk and selet 24MHz */
	reg_data = reg_get(REG_BASE_CRG + REG_PERI_CRG4192);
	reg_data |= UART0_CLK_ENABLE;
	reg_data &= ~UART0_CLK_MASK;
	reg_data |= UART0_CLK_24M;
	reg_data &= ~UART0_SRST_REQ;
	reg_set(REG_BASE_CRG + REG_PERI_CRG4192, reg_data);

	uart_port[CONSOLE_PORT] = REG_BASE_SERIAL0;

	/*
	 ** First, disable everything.
	 */
	io_write(uart_port[CONSOLE_PORT] + UART_PL011_CR, 0x0);

	/*
	 ** Set baud rate
	 **
	 ** IBRD = UART_CLK / (16 * BAUD_RATE)
	 ** FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
	 */
	temp = 16 * BAUDRATE;  /* 16:Multiple Value */
	divider = CONFIG_PL011_CLOCK / temp;
	remainder = CONFIG_PL011_CLOCK % temp;
	temp = (8 * remainder) / BAUDRATE;  /* 8:Multiple Value */
	fraction = (temp >> 1) + (temp & 1);
	io_write(uart_port[CONSOLE_PORT] + UART_PL011_IBRD, divider);
	io_write(uart_port[CONSOLE_PORT] + UART_PL011_FBRD, fraction);

	/*
	 ** Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled.
	 */
	io_write(uart_port[CONSOLE_PORT] + UART_PL011_LCRH,
		 (UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN));

	/*
	 ** Finally, enable the UART
	 */
	io_write(uart_port[CONSOLE_PORT] + UART_PL011_CR,
		 (UART_PL011_CR_UARTEN | UART_PL011_CR_TXE |
		  UART_PL011_CR_RXE));

	return 0;
}

int serial_deinit()
{
	io_write(uart_port[CONSOLE_PORT] + UART_PL011_CR,
		 (io_read(uart_port[CONSOLE_PORT] + UART_PL011_CR) &
		  ~UART_PL011_CR_UARTEN));
	return 0;
}

void serial_putc(const char c)
{
	pl011_putc(CONSOLE_PORT, c);
}

int serial_getc(void)
{
	return pl011_getc(CONSOLE_PORT);
}

int serial_tstc(void)
{
	return pl011_tstc(CONSOLE_PORT);
}

static void pl011_putc(int portnum, char c)
{
	if (portnum >= MAX_PORT_NUM)
		return;

	/* Wait until there is space in the FIFO */
	while (io_read(uart_port[portnum] + UART_PL01_FR) & UART_PL01_FR_TXFF);

	/* Send the character */
	io_write(uart_port[portnum] + UART_PL01_DR, c);
}

static int pl011_getc(int portnum)
{
	unsigned int data;

	if (portnum >= MAX_PORT_NUM)
		return -1;

	/* Wait until there is data in the FIFO */
	while (io_read(uart_port[portnum] + UART_PL01_FR) & UART_PL01_FR_RXFE);

	data = io_read(uart_port[portnum] + UART_PL01_DR);

	/* Check for an error flag */
	if (data & 0xFFFFFF00) {
		/* Clear the error */
		io_write(uart_port[portnum] + UART_PL01_ECR, 0xFFFFFFFF);
		return -1;
	}

	return (int)data;
}

static int pl011_tstc(int portnum)
{
	return !(io_read(uart_port[portnum] + UART_PL01_FR) &
		 UART_PL01_FR_RXFE);
}

#endif /* CFG_PL011_SERIAL */
