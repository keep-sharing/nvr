/*
* Copyright (c) 2012 HiSilicon Technologies Co., Ltd.
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
#include "startup.h"
/******************************************************************************/
#ifndef uart_early_puts
void uart_early_puts(const char *ss)
{
}
#endif

#ifndef reset_cpu
void reset_cpu(unsigned long addr)
{
}
#endif
/******************************************************************************/
const unsigned int IMAGE_ENTRY = (TEXT_BASE + 0x100000);

typedef void (*ENTRY)(void);
/******************************************************************************/
#define error(_s)               uart_early_puts(_s)
#define putstr(_s)              uart_early_puts(_s)

#define large_malloc(_n)        malloc(_n)
#define large_free(_n)          free(_n)

/******************************************************************************/
/*
 * this value will be (_armboot_start - CONFIG_SYS_MALLOC_LEN)
 */
static char *malloc_start;

/******************************************************************************/
static void *malloc(unsigned int size)
{
	void *ptr = malloc_start;
	malloc_start += size;
	return ptr;
}
/******************************************************************************/
static void free(void *ptr)
{
}
/******************************************************************************/

#include "unlzma.c"

/******************************************************************************/
void start_armboot(void)
{
	ENTRY entry = (ENTRY)IMAGE_ENTRY;
	unsigned char *pdst = (unsigned char *)IMAGE_ENTRY;
	unsigned int image_data_len = input_data_end - input_data;

	malloc_start = (char *)(_armboot_start - CONFIG_SYS_MALLOC_LEN);

	/* DDR should larger than 16M */
	enable_mmu();

	dcache_enable(0);

	if (input_data[0] == 0x5D) {
		decompress(input_data, image_data_len, pdst);
	} else {
		int *s = (int *)input_data;
		int *d = (int *)pdst;
		unsigned int len = ((image_data_len + 3) >> 2);
		while (len--)
			*d++ = *s++;
	}
	dcache_disable();

	stop_mmu();

	entry();
}

/******************************************************************************/
void do_undefined_instruction(void)
{
	reset_cpu(0);
}

void do_software_interrupt(void)
{
	reset_cpu(0);
}

void do_prefetch_abort(void)
{
	reset_cpu(0);
}

void do_data_abort(void)
{
	reset_cpu(0);
}

void do_not_used(void)
{
	reset_cpu(0);
}

void do_fiq(void)
{
	reset_cpu(0);
}

void do_irq(void)
{
	reset_cpu(0);
}
