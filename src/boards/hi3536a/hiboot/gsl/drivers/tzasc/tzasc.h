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
#ifndef __TZASC_H__
#define __TZASC_H__

#define TZASC_ATTR_SEC_R    (0x5 << 28)
#define TZASC_ATTR_SEC_W    (0x5 << 24)
#define TZASC_ATTR_NOSEC_R  (0x5 << 20)
#define TZASC_ATTR_NOSEC_W  (0x5 << 16)
#define TZASC_ATTR_MID_INV  (1 << 9)
#define TZASC_ATTR_MID_EN   (1 << 8)
#define TZASC_ATTR_SEC_INV  (1 << 4)

#define TZASC_ALGIN_BITS	12	// 4k alignment

void tzasc_bypass_disable();
void tzasc_set_rgn_attr(uint8_t rgn, uint32_t attr);
// Notice: addr and size of function "tzasc_set_rgn_map" must align to 4K
void tzasc_set_rgn_map(uint8_t rgn, uint64_t addr, size_t size);
void tzasc_rgn_enable(uint8_t rgn);

#endif /* __TZASC_H__ */
