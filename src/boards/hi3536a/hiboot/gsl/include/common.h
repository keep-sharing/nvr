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
#ifndef __COMMON_H_
#define __COMMON_H_

void call_reset(void);
#define ALWAYS_INLINE inline __attribute__((always_inline))

#define STEP_AUTH                   0x5A3C

#define random_delay()

#define RSA_4096_LEN      0x200

/*-----------------------------------------------------------------
 * serial interface
------------------------------------------------------------------*/
int serial_init();
int serial_deinit();
void serial_putc(const char c);
void serial_putchar(const char c);
void serial_puts(const char *s);
void serial_flush();
int serial_getc(void);
int serial_tstc(void);
void uart_init();
void uart_deinit();
void uart_reset();
void uart_port_init();
/*-----------------------------------------------------------------
 * mmc interface
------------------------------------------------------------------*/
int mmc_init();
void mmc_deinit();
size_t mmc_read(void *ptr, size_t src, size_t size, size_t read_type);
void mmc_muxctrl_config(void);

/*-----------------------------------------------------------------
 * timer interface
------------------------------------------------------------------*/
void timer_init();
void timer_deinit();
void timer_start();
unsigned long timer_get_val();
unsigned long timer_get_divider();
void mdelay(unsigned long msec);
void err_print(uint8_t err_type, uint8_t err_idx);
int copy_from_usb(const void *dest, size_t count);

#endif /* __COMMON_H_ */
