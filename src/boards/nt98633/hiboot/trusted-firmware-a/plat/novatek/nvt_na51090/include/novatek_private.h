/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef NOVATEK_PRIVATE_H
#define NOVATEK_PRIVATE_H

#include <stddef.h>

#define NVT_HEADINFO_CHIP_NAME "bl51090"
void nova_setup_page_tables(uintptr_t total_base, size_t total_size,
							uintptr_t code_start, uintptr_t code_limit,
							uintptr_t rodata_start, uintptr_t rodata_limit
						   );

void plat_tmr_init(void);
void nvt_tzasc_config(void);
#endif /* NOVATEK_PRIVATE_H */
