// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef RSA_PADDING_H
#define RSA_PADDING_H

#include "ot_type.h"
#include "drv_cipher_ioctl.h"

typedef struct {
    td_u8 *data;
    td_u32 data_len;
} ot_data_t;

#define PKCS_BLOCK_TYPE_0               0
#define PKCS_BLOCK_TYPE_1               1
#define PKCS_BLOCK_TYPE_2               2

td_s32 rsa_encrypt_pad_none(const cryp_rsa_key *key, const ot_data_t *plain_txt, ot_data_t *em_txt);

td_s32 rsa_decrypt_pad_none(const cryp_rsa_key *key, ot_data_t *plain_txt, const ot_data_t *em_txt);

td_s32 rsa_encrypt_pad_pkcs_type(const cryp_rsa_key *key,
    const ot_data_t *plain_txt, ot_data_t *em_txt, td_u32 block_type);

td_s32 rsa_decrypt_pad_pkcs_type(const cryp_rsa_key *key,
    ot_data_t *plain_txt, const ot_data_t *em_txt, td_u32 block_type, td_u32 *outlen);

td_s32 rsa_encrypt_pad_pkcs_v15(const cryp_rsa_key *key,
    const ot_data_t *plain_txt, ot_data_t *em_txt);

td_s32 rsa_decrypt_pad_pkcs_v15(const cryp_rsa_key *key,
    ot_data_t *plain_txt, const ot_data_t *em_txt, td_u32 *outlen);

td_s32 rsa_encrypt_pad_pkcs_oaep(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *plain_txt, ot_data_t *em_txt);

td_s32 rsa_decrypt_pad_pkcs_oaep(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, ot_data_t *plain_txt, ot_data_t *em_txt, td_u32 *outlen);

td_s32 rsa_sign_pad_pkcs_v15(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, ot_data_t *em_txt);

td_s32 rsa_verify_pad_pkcs_v15(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, const ot_data_t *em_txt);

td_s32 rsa_sign_pad_pkcs_pss(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, ot_data_t *em_txt, td_u32 *outlen);

td_s32 rsa_verify_pad_pkcs_pss(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, ot_data_t *em_txt);

#endif /* RSA_PADDING_H */
