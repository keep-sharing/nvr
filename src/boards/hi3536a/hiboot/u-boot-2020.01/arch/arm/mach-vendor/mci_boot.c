// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <config.h>
#include <asm/arch/platform.h>
#include <mci_reg.h>
#include "soc.h"
#include <compiler.h>

#define DELAY_US        1000
#define CP_STEP1_SIZE   0x6000

static inline unsigned int mci_readl(unsigned addr)
{
	return *((volatile unsigned *)(uintptr_t)(addr));
}

static inline void mci_writel(unsigned val, unsigned addr)
{
	(*(volatile unsigned *)(uintptr_t)(addr)) = (val);
}

static inline void delay(unsigned int cnt)
{
	while (cnt--)
		__asm__ __volatile__("nop");
}

static void emmc_deinit(void)
{
	unsigned int mmc_base = EMMC_REG_BASE;
	unsigned int val;

	val = mci_readl(mmc_base + MCI_CTRL);
	val |= (CTRL_RESET | FIFO_RESET | DMA_RESET);
	mci_writel(val, mmc_base + MCI_CTRL);

	/* clear MMC host intr */
	mci_writel(ALL_INT_CLR, mmc_base + MCI_RINTSTS);
}

static void emmc_boot_mode_read(void *ptr, unsigned int size)
{
	unsigned int count, val;
	unsigned int tmp_reg;
	unsigned int *buf = NULL;
	unsigned int data;

	if (size <= CP_STEP1_SIZE) {
		memcpy_32(ptr, (void *)RAM_START_ADRS, size);
		return;
	}
	memcpy_32(ptr, (void *)RAM_START_ADRS, CP_STEP1_SIZE);
	buf = (unsigned int *)(ptr + CP_STEP1_SIZE);
	size = (size - CP_STEP1_SIZE) >> NUM_2;

	while (size > 0) {
		tmp_reg = mci_readl(EMMC_REG_BASE + MCI_STATUS);
		count = (tmp_reg >> FIFO_COUNT) & FIFO_COUNT_MASK;

		if (count > size)
			count = size;

		/* start to read data */
		while (1) {
			val = (DCRC_INT_STATUS | FRUN_INT_STATUS | HLE_INT_STATUS |
						SBE_INT_STATUS | EBE_INT_STATUS);
			tmp_reg = mci_readl(EMMC_REG_BASE + MCI_RINTSTS);
			if (tmp_reg & val)
				return;

			if (tmp_reg & RXDR_INT_STATUS)
				break;
		}

		mci_writel(ALL_INT_CLR, EMMC_REG_BASE + MCI_RINTSTS);

		for (; count != 0; --count) {
			data = mci_readl(EMMC_REG_BASE + MCI_FIFO_START);

			*buf = data;
			buf++;
			--size;
		}
	}

	mci_writel(START_CMD | DISABLE_BOOT | STOP_ABORT_CMD, EMMC_REG_BASE + MCI_CMD);
	count = 1000; /* 1000: Cycle */
	do {
		delay(DELAY_US);
		count--;
		tmp_reg = mci_readl(EMMC_REG_BASE + MCI_CMD);
	} while ((tmp_reg & START_CMD) && count);

	count = 1000; /* 1000: Cycle */
	do {
		delay(DELAY_US);
		count--;
		tmp_reg = mci_readl(EMMC_REG_BASE + MCI_RINTSTS);
	} while (!(tmp_reg & DTO_INT_STATUS) && count);
}

static void emmc_boot_mode_deinit(void)
{
	unsigned int mmc_base = EMMC_REG_BASE;

	mci_writel(START_CMD | DISABLE_BOOT | STOP_ABORT_CMD, mmc_base + MCI_CMD);
}

void emmc_boot_read(void *ptr, unsigned int size)
{
	emmc_boot_mode_read(ptr, size);
	emmc_boot_mode_deinit();
	emmc_deinit();
}
