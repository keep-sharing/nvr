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

#ifndef __HPP_ROM_H__
#define __HPP_ROM_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define KLAD_KEY_LEN      0x10
#define KLAD_KEY0_LEN	  0x10
#define KLAD_KEY1_LEN	  0x20

typedef enum {
	HPP_ALG_AES          = 0x00,
	HPP_ALG_SM4          = 0x01,
} hpp_alg_sel;

typedef struct {
	uint32_t is_odd;
	uint32_t key_slot_num;
} hpp_klad_cfg;

typedef struct {
	hpp_alg_sel alg;
	uint8_t session_key0[KLAD_KEY0_LEN];
	uint8_t session_key1[KLAD_KEY1_LEN];
	hpp_klad_cfg cfg;
} hpp_klad;

typedef enum {
	KLAD_BOOT               = 0x0,
	KLAD_ATF               	= 0x1,
	KLAD_TEE               	= 0x2,
} hpp_keyladder_sel;

int32_t hpp_klad_process(const hpp_keyladder_sel keyladder, const hpp_klad *klad);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

