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

#ifndef __BL_COMMON_H__
#define __BL_COMMON_H__

#define ATF_PARAM_EP		0x01
#define ATF_PARAM_IMAGE_BINARY	0x02
#define ATF_PARAM_BL31		0x03

#define ATF_VERSION_1	0x01

#define ATF_EP_SECURE	0x0
#define ATF_EP_NON_SECURE	0x1

#define MODE_RW_SHIFT	0x4
#define MODE_RW_MASK	0x1
#define MODE_RW_64	0x0
#define MODE_RW_32	0x1

#define MODE_EL_SHIFT	0x2
#define MODE_EL_MASK	0x3
#define MODE_EL3	0x3
#define MODE_EL2	0x2
#define MODE_EL1	0x1
#define MODE_EL0	0x0

#define MODE_SP_SHIFT	0x0
#define MODE_SP_MASK	0x1
#define MODE_SP_EL0	0x0
#define MODE_SP_ELX	0x1

#define SPSR_DAIF_SHIFT	6
#define SPSR_DAIF_MASK	0x0f

#define SPSR_FIQ             (1 << 6)
#define SPSR_IRQ             (1 << 7)
#define SPSR_SERROR          (1 << 8)
#define SPSR_DEBUG           (1 << 9)
#define SPSR_EXCEPTION_MASK  (SPSR_FIQ | SPSR_IRQ | SPSR_SERROR | SPSR_DEBUG)

#define DAIF_FIQ_BIT (1<<0)
#define DAIF_IRQ_BIT (1<<1)
#define DAIF_ABT_BIT (1<<2)
#define DAIF_DBG_BIT (1<<3)
#define DISABLE_ALL_EXECPTIONS	\
	(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT | DAIF_DBG_BIT)

#define NON_SECURE       0x1
#define DISABLE_ALL_EXCEPTIONS \
	(DAIF_FIQ_BIT | DAIF_IRQ_BIT | DAIF_ABT_BIT | DAIF_DBG_BIT)

#define PARAM_EP_SECURITY_MASK    0x1
#define get_security_state(x) ((x) & PARAM_EP_SECURITY_MASK)
#define set_security_state(x, security) \
	((x) = ((x) & ~PARAM_EP_SECURITY_MASK) | (security))

#endif /* __BL_COMMON_H__ */
