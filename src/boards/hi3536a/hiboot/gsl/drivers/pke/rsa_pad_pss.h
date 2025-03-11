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
#ifndef __RSA_PAD_PSS_H__
#define __RSA_PAD_PSS_H__

typedef struct {
	uint32_t hlen;
	uint32_t klen;
	uint32_t em_bit;
	uint8_t *in_data;
	uint32_t in_len;
} rsa_padding_s;

uint32_t rsa_get_bit_num(const uint8_t *big_num, uint32_t num_len);
int32_t rsa_padding_check_pkcs1_pss(rsa_padding_s *pad, const uint8_t *mhash);

#endif
