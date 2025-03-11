/*
 * Copyright (c) 2020, NovaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

	.globl	plat_crash_console_init
	.globl	plat_crash_console_putc
	.globl	plat_crash_console_flush
	.globl	plat_tmr_init
	.globl  plat_is_my_cpu_primary
	.globl	plat_my_core_pos
	.globl	plat_calc_core_pos
	.globl  plat_secondary_cold_boot_setup
	.globl	platform_mem_init
	.globl	plat_report_exception
	.globl	plat_cpu_wait_to_boot

	/* Global variables */
	.globl	nova_sec_entry_point

func plat_crash_console_init
	mov_imm x0, UART0_BASE
	mov_imm x1, UART0_CLK_IN_HZ
	mov_imm x2, UART0_BAUDRATE
	b	console_16550_core_init
endfunc plat_crash_console_init

func plat_crash_console_putc
	mov_imm x1, UART0_BASE
	b	console_16550_core_putc
endfunc plat_crash_console_putc

func plat_crash_console_flush
	ret
endfunc plat_crash_console_flush

func plat_tmr_init
	/* Initialize Generic Timers */
	msr     cntvoff_el2, xzr
	ret
endfunc plat_tmr_init

        /* -----------------------------------------------------
         * unsigned int plat_is_my_cpu_primary (void);
         *
         * Find out whether the current cpu is the primary
         * cpu.
         * -----------------------------------------------------
         */
func plat_is_my_cpu_primary
        mrs     x0, mpidr_el1
        and     x0, x0, #(MPIDR_CLUSTER_MASK | MPIDR_CPU_MASK)
        cmp     x0, #PLAT_PRIMARY_CPU
        cset    x0, eq
        ret
endfunc plat_is_my_cpu_primary

func plat_my_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CLUSTER_MASK
	and	x0, x0, #MPIDR_CPU_MASK
	add	x0, x0, x1, LSR #6
	ret
endfunc plat_my_core_pos

func plat_calc_core_pos
	mrs	x0, mpidr_el1
	and	x1, x0, #MPIDR_CPU_MASK
	and	x0, x0, #MPIDR_CLUSTER_MASK
	add	x0, x1, x0, LSR #6
	ret
endfunc plat_calc_core_pos

func plat_cpu_wait_to_boot
        /* Calculate address of our hold entry */
        bl	plat_calc_core_pos
        lsl     x0, x0, #3
        mov_imm x2, PLAT_HOLD_BASE
        add     x0, x0, x2

        /*
         * This code runs way before requesting the warmboot of this core,
         * so it is possible to clear the mailbox before getting a request
         * to boot.
         */
        mov     x1, PLAT_HOLD_STATE_WAIT
        str     x1, [x0]

        /* Wait until we have a go */
poll_mailbox:
        wfe
        ldr     x1, [x0]
        cmp     x1, PLAT_HOLD_STATE_GO
        bne     poll_mailbox

        /* run bl31_warm_entrypoint -> psci_warmboot_entrypoint */
        adr     x1, nova_sec_entry_point
        ldr     x0, [x1]
        br      x0

        ret
endfunc plat_cpu_wait_to_boot

        /* -----------------------------------------------------
         * void plat_secondary_cold_boot_setup (void);
         *
         * This function performs any platform specific actions
         * needed for a secondary cpu after a cold reset. Right
         * now this is a stub function.
         * -----------------------------------------------------
         */
func plat_secondary_cold_boot_setup

        ldr     x9, =UART0_BASE 			/* print # */
        ldr     x10, ='#'
        str     x10, [x9]

        /* Initialize Generic Timers */
        bl      plat_tmr_init

        ldr     x0, =OSC_CLK_IN_HZ
        msr     cntfrq_el0, x0

        bl      plat_cpu_wait_to_boot

        ret
endfunc plat_secondary_cold_boot_setup

func platform_mem_init
	ret
endfunc platform_mem_init

func plat_report_exception
	ret
endfunc plat_report_exception

	.data
	.align 3

	/* ------------------------
	 * CPU Secure entry point
	 * ------------------------
	 */
nova_sec_entry_point:
	.quad	0
