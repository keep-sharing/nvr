/*
 * Copyright (C) 2019 Novatek Microelectronics Corp. All rights reserved.
 * Author: IVOT-ESW <IVOT_ESW_MailGrp@novatek.com.tw>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __SOC_H
#define __SOC_H

/*
 * Hardware register bases
 */

#define CONFIG_PMU_BASE         0xfe000000
#define CONFIG_IVOT_MMC_BASE    0xfa600000
#define CONFIG_TOP_BASE         0xFE030000
#define CONFIG_SYSC_BASE        0xFE000000
#define CONFIG_PWM_BASE         0xFE6C0000
#define CONFIG_LCDC300_BASE		0xFDA00000
#define CONFIG_LCDC210_BASE     0xFDB00000
#define CONFIG_HDMI_BASE        0xFD900000
#define CONFIG_FTTVE100_BASE    0xFAD00000
#define CONFIG_MAU_BASE         0xFC400000

#define BIT0            0x00000001
#define BIT1            0x00000002
#define BIT2            0x00000004
#define BIT3            0x00000008
#define BIT4            0x00000010
#define BIT5            0x00000020
#define BIT6            0x00000040
#define BIT7            0x00000080
#define BIT8            0x00000100
#define BIT9            0x00000200
#define BIT10           0x00000400
#define BIT11           0x00000800
#define BIT12           0x00001000
#define BIT13           0x00002000
#define BIT14           0x00004000
#define BIT15           0x00008000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000

typedef unsigned char       UINT8;
typedef unsigned int        UINT32;
typedef unsigned long long  UINT64;
typedef unsigned int        BOOL;
#define FALSE               0
#define TRUE                1

#endif	/* __SOC_H */
