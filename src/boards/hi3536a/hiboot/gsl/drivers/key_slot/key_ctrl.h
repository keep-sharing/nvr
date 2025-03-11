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
#ifndef __KEY_CTRL_KS_H__
#define __KEY_CTRL_KS_H__
#define KC_MC_SLOT_NUM 16
#define KC_TS_SLOT_NUM 256

typedef enum {
	KC_SLOT_MC               = 0x0,
	KC_SLOT_TS               = 0x1,
	KC_SLOT_MAX
} kc_slot_ind_e;

int32_t kc_slot_unlock(const kc_slot_ind_e slot_ind, const uint32_t slot_num);
int32_t kc_slot_auto_lock(const kc_slot_ind_e slot_ind, uint32_t *slot_num);
int32_t kc_slot_is_lock(const kc_slot_ind_e slot_ind, const uint32_t slot_num);

#endif /* __KEY_CTRL_KS_H__ */
