/**
	To provide the low level init.

    @file       lowlevel_init.S
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>
#include <asm/armv8/mmu.h>



ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

	mrs     x15, DAIF
	ldr     x15, =0x3C0
	msr     DAIF, x15

#ifdef CONFIG_ARMV8_MULTIENTRY
	branch_if_master x0, x1, 2f

	/*
	 * Slave should wait for master clearing spin table.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
#ifdef CONFIG_GICV2
	ldr	x0, =GICC_BASE
#endif
	bl	gic_wait_for_interrupt
#endif

	/*
	 * All slaves will enter EL1
	 */

	/* switch to EL2 */
	adr	x4, lowlevel_in_el2_v1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2
lowlevel_in_el2_v1:

	mov	lr, x29
	ret

#endif /* CONFIG_ARMV8_MULTIENTRY */

2:
	/* switch to EL2 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2
lowlevel_in_el2:


	mov	lr, x29			/* Restore LR */
	ret

ENDPROC(lowlevel_init)

#if 1
ENTRY(local_load)
    dmb     sy
    mov     x1, #0xf0630000
    movk    x1, #0x2, lsl #32
    //x1 = 0x2F0630000
    str     wzr, [x1]                       //0x2F0630000=0x0
    dmb     sy

    add     x1, x1, #0x30                   //x1=0x2F0660030
    stur    w0, [x1, #-44]                  //0x2F0630004 = rowaddress

    ldr     w0, [x1]
    dmb     sy
    dmb     sy
    str     w0, [x1]                        //->Read 0x30 write back 0x30

    dmb     sy
    mov     x2, #0x20                       // #32
    mov     w0, #0x1                        // #1
    movk    x2, #0xf063, lsl #16
    movk    x2, #0x2, lsl #32
    str     w0, [x2]                        //0x2F0660020 = 0x1

    //x1 = 0x2F0660030
    //x2 = 0x2F0660020
POLL_SET_EN_STS_R:
    ldr     w0, [x1]
    dmb     sy
    tbz     w0, #0, POLL_SET_EN_STS_R       //polling until 0x30 bit[0] = 1

    ldr     w0, [x2]
    dmb     sy
    orr     w0, w0, #0x10
    dmb     sy
    str     w0, [x2]                        //0x2F0630020=0x11

POLL_STATUS_R:
    ldr     w0, [x1]
    dmb     sy
    tbz     w0, #1, POLL_STATUS_R           //polling until 0x30 bit[1] = 1

    mov     x0, #0x8                        // #8
    movk    x0, #0xf063, lsl #16
    movk    x0, #0x2, lsl #32
    ldr     w0, [x0]                        //w0 = 0x2F0630008
    dmb     sy
    dmb     sy
    str     wzr, [x2]                       //0x20 = 0

POLL_STATUS_R_EN:
    ldr     w2, [x1]
    dmb     sy
    tbnz    w2, #0, POLL_STATUS_R_EN        //polling 0x30 until bit[0] = 0

    ldr     w2, [x1]
    dmb     sy
    dmb     sy
    str     w2, [x1]                        //read 0x30 and write back
    ret
ENDPROC(local_load)

ENTRY(local_store)
    dmb     sy
    mov     x1, #0xf0630000                 // #4033216512
    mov     w2, #0x4                        // w2 = #4
    movk    x1, #0x2, lsl #32
    str     w2, [x1]
    dmb     sy                              //0x2F0630000 = 0x4

    str     w0, [x1, #4]                    // w0 = address
    mov     x0, #0x30                       // #48
    movk    x0, #0xf063, lsl #16
    movk    x0, #0x2, lsl #32
    ldr     w1, [x0]                        //ldr 0x2F0630030
    dmb     sy
    dmb     sy
    str     w1, [x0]                        //str 0x2F0630030
    dmb     sy
    mov     x1, #0x20                       // #32
    mov     w2, #0x1                        // #1
    movk    x1, #0xf063, lsl #16
    movk    x1, #0x2, lsl #32
    str     w2, [x1]                        //

 POLL_SET_EN_STS_P:
    ldr     w2, [x0]
    dmb     sy
    tbz     w2, #0, POLL_SET_EN_STS_P
    ldr     w2, [x1]
    dmb     sy
    orr     w2, w2, #0x10
    dmb     sy
    str     w2, [x1]

 POLL_STATUS_P:
    ldr     w2, [x0]
    dmb     sy
    tbz     w2, #1, POLL_STATUS_P
    dmb     sy
#if (0)
    str     wzr, [x1]

POLL_STATUS_P_EN:
    ldr     w1, [x0]
    dmb     sy
    tbnz    w1, #0, POLL_STATUS_P_EN
#endif

    ldr     w1, [x0]
    dmb     sy
    dmb     sy
    str     w1, [x0]
    mov     x0, #0x0                        // #0
    ret
ENDPROC(local_store)

#endif
