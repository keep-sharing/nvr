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

#ifndef __OT_DRV_PKE_H__
#define __OT_DRV_PKE_H__

/******************************* API Declaration *****************************/
#define SM2_LEN_IN_WROD              8
#define SM2_LEN_IN_BYTE              (SM2_LEN_IN_WROD * 4)

#define RSA_KEY_LEN_2048             256
#define RSA_KEY_LEN_4096             512
#define HASH_LEN_256                 32
#define WORD_WIDTH                   0x04

/**
* \brief RSA verify a ciphertext with a RSA public key.
*/
int32_t drv_rsa_verify_hash(const unsigned char *n, const unsigned char *e,
			       const uint8_t *hash, const unsigned char *sign);

/**
\brief   Kapi Init.
\retval     On success, TD_SUCCESS is returned.  On error, TD_FAILURE is returned.
*/
void drv_pke_reset(void);

/** @} */  /** <!-- ==== API Code end ==== */

#endif    /* End of #ifndef __DRV_CIPHER_KAPI_H__ */
