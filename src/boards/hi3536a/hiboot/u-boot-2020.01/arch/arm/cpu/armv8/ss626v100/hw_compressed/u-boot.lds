/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
	. = 0x48700000;
	__image_copy_start =.;
	. = ALIGN(8);
	.text :
	{
	__text_start = .;
		start.o (.text*)
		init_registers.o (.text*)
		lowlevel_init_v300.o (.text*)
		ddrtrn_api_pm.o (.text*)
		ddrtrn_api_training_item.o (.text*)
		ddrtrn_common_ctl.o (.text*)
		ddrtrn_boot.o (.text*)
		ddrtrn_exec_time.o (.text*)
		ddrtrn_training_custom.o (.text*)
		ddrtrn_hal_auto_pd.o (.text*)
		ddrtrn_hal_ddrc.o (.text*)
		ddrtrn_ddrc_s14.o (.text*)
		ddrtrn_hal_ddrt.o (.text*)
		ddrtrn_hal_ac.o (.text*)
		ddrtrn_hal_adjust.o (.text*)
		ddrtrn_hal_dataeye.o (.text*)
		ddrtrn_hal_dcc.o (.text*)
		ddrtrn_hal_gate.o (.text*)
		ddrtrn_hal_hw.o (.text*)
		ddrtrn_hal_low_freq.o (.text*)
		ddrtrn_hal_lpca.o (.text*)
		ddrtrn_hal_pcode.o (.text*)
		ddrtrn_hal_phy.o (.text*)
		ddrtrn_hal_phy_common.o (.text*)
		ddrtrn_hal_vref.o (.text*)
		ddrtrn_hal_wakeup.o (.text*)
		ddrtrn_hal_write_leveling.o (.text*)
		ddrtrn_phy_s14.o (.text*)
		ddrtrn_hal_sysctrl_common.o (.text*)
		ddrtrn_hal_uart.o (.text*)
		ddrtrn_hal_context.o (.text*)
		ddrtrn_ac.o (.text*)
		ddrtrn_adjust.o (.text*)
		ddrtrn_auto_pd.o (.text*)
		ddrtrn_console.o (.text*)
		ddrtrn_dataeye.o (.text*)
		ddrtrn_dcc.o (.text*)
		ddrtrn_ddrc.o (.text*)
		ddrtrn_ddrt.o (.text*)
		ddrtrn_gate.o (.text*)
		ddrtrn_hw.o (.text*)
		ddrtrn_low_freq.o (.text*)
		ddrtrn_lpca.o (.text*)
		ddrtrn_pcode.o (.text*)
		ddrtrn_phy.o (.text*)
		ddrtrn_retrain_enable.o (.text*)
		ddrtrn_vref.o (.text*)
		ddrtrn_wakeup.o (.text*)
		ddrtrn_write_leveling.o (.text*)
		uart.o (.text*)
		div0.o (.text*)
		sdhci_boot.o (.text*)
		image_data.o (.text*)
		startup.o(.text*)
		reset.o(.text*)
	__init_end = .;
	ASSERT(((__init_end - __text_start) < 0x14000), "init sections too big!");
		*(.text*)
	}

	. = ALIGN(8);
	.image : { *(.image) }

	. = ALIGN(8);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(8);
	.data : {
		*(.data*)
	}

	. = ALIGN(8);

	.got : { *(.got) }

	. = ALIGN(8);
	__image_copy_end =.;
	__bss_start = .;
	.bss :
	{
		*(.bss)
	}
	__bss_end = .;


	_end = .;
}
