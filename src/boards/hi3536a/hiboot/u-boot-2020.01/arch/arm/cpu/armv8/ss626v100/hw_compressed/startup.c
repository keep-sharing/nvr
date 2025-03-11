// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <config.h>
#include <asm/io.h>
#include <compiler.h>
#include <cpu_common.h>
#include <linux/kconfig.h>

#define error(_s)               uart_early_puts(_s)
#define putstr(_s)              uart_early_puts(_s)
#define EXCEPTION_LEVEL1   	1
#define EXCEPTION_LEVEL2   	2
#include "hw_decompress.c"
#define GZIP_SIZE_OFFSET 0x4

void icache_enable(void);
void invalidate_icache_all(void);
void __asm_invalidate_icache_all(void);

void start_armboot(void)
{
	unsigned char *pdst_h32 = NULL;
	unsigned char *pdst_l32 = NULL;
	unsigned char *input_data_h32 = NULL;
	unsigned int image_data_len;
	int pdst_len;
	int ret;
	const uintptr_t image_entry = (CONFIG_SYS_TEXT_BASE);
	uart_early_init();
	uart_early_puts("\r\nUncompress ");

	/* enable I-Cache  */
	icache_enable();

	/* use direct address mode */
	hw_dec_type = 0;
	/* init hw decompress IP */
	hw_dec_init();

	/* start decompress */
	pdst_l32 = (unsigned char *)(uintptr_t)image_entry;
	image_data_len = input_data_end - input_data;
	for (int i = 0; i < sizeof(pdst_len); i++)
		*((char *)(&pdst_len) + i) = *((char *)(input_data_end -
							GZIP_SIZE_OFFSET) + i);

	ret = hw_dec_decompress(pdst_h32, pdst_l32, &pdst_len, input_data_h32,
					input_data, image_data_len, NULL);
	if (ret == 0) {
		uart_early_puts("Ok!");
	} else {
		uart_early_puts("Fail!");
		while (1);
	}

	/* uinit hw decompress IP */
	hw_dec_uinit();
	void (*uboot)(void);
	uboot = (void (*))CONFIG_SYS_TEXT_BASE;
	invalidate_icache_all();
	uboot();
}

void hang(void)
{
	uart_early_puts("### ERROR ### Please RESET the board ###\n");
	for (; ;);
}

void invalidate_icache_all(void)
{
	__asm_invalidate_icache_all();
}

static inline unsigned int current_el(void)
{
	unsigned int el;
	asm volatile("mrs %0, CurrentEL" : "=r"(el) : : "cc");
	return el >> 2; /* Move Left 2bit */
}

static void set_sctlr(unsigned int val)
{
	unsigned int el;

	el = current_el();
	if (el == EXCEPTION_LEVEL1)
		asm volatile("msr sctlr_el1, %0" : : "r"(val) : "cc");
	else if (el == EXCEPTION_LEVEL2)
		asm volatile("msr sctlr_el2, %0" : : "r"(val) : "cc");
	else
		asm volatile("msr sctlr_el3, %0" : : "r"(val) : "cc");

	asm volatile("isb");
}

static unsigned int get_sctlr(void)
{
	unsigned int el, val;

	el = current_el();
	if (el == EXCEPTION_LEVEL1)
		asm volatile("mrs %0, sctlr_el1" : "=r"(val) : : "cc");
	else if (el == EXCEPTION_LEVEL2)
		asm volatile("mrs %0, sctlr_el2" : "=r"(val) : : "cc");
	else
		asm volatile("mrs %0, sctlr_el3" : "=r"(val) : : "cc");

	return val;
}

void icache_enable(void)
{
	invalidate_icache_all();
#define CR_I    (1 << 12)   /* Icache enable */
	set_sctlr(get_sctlr() | CR_I);
}

void do_bad_sync(void)
{
	uart_early_puts("bad sync abort\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}

void do_sync(void)
{
	uart_early_puts("sync abort\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}

void do_bad_error(void)
{
	uart_early_puts("bad error\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}

void do_error(void)
{
	uart_early_puts("error\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}

void do_bad_fiq(void)
{
	uart_early_puts("bad fast interrupt request\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}

void do_bad_irq(void)
{
	uart_early_puts("bad interrupt request\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}

void do_fiq(void)
{
	uart_early_puts("fast interrupt request\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}

void do_irq(void)
{
	uart_early_puts("interrupt request\r\n");
	uart_early_puts("Resetting CPU ...\r\n");
	reset_cpu(0);
}
