// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __BSP_SOC_H__
#define __BSP_SOC_H__

extern unsigned int blank_zone_start;
extern unsigned int get_blank_start(void);
extern void init_registers(unsigned int base, unsigned int pm);
extern void emmc_boot_read(void *ptr, unsigned int size);
extern void memcpy_32(void *dst, void *src, unsigned int size);

#ifdef CONFIG_DDR_TRAINING_V2
	extern void start_ddr_training(unsigned int base);
#endif /* CONFIG_DDR_TRAINING_V2 */

#endif
