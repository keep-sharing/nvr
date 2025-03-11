// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "rsa_padding.h"
#include "ot_common_cipher.h"
#include "drv_cipher_kapi.h"
#include "drv_osal_lib.h"
#include "drv_pke.h"

#define HASH_RESULT_MAX_LEN             64

#define PKCS_V15_MIN_PAD_LEN            11U
#define PKCS_V15_MIN_PS_LEN             8U
#define PKCS_V15_MIN_OTHER_LEN          (PKCS_V15_MIN_PAD_LEN - PKCS_V15_MIN_PS_LEN)

#define PKCS_PSS_PAD1_LEN              8

static const td_u8 g_asn1_sha256[] = {
    0x30, 0x31, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
    0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x05,
    0x00, 0x04, 0x20,
};

static const td_u8 g_asn1_sha384[] = {
    0x30, 0x41, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
    0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02, 0x05,
    0x00, 0x04, 0x30,
};

static const td_u8 g_asn1_sha512[] = {
    0x30, 0x51, 0x30, 0x0d, 0x06, 0x09, 0x60, 0x86,
    0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03, 0x05,
    0x00, 0x04, 0x40,
};

static const td_u8 g_empty_l_sha256[] = {
    0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14,
    0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
    0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c,
    0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55,
};

static const td_u8 g_empty_l_sha384[] = {
    0x38, 0xb0, 0x60, 0xa7, 0x51, 0xac, 0x96, 0x38,
    0x4c, 0xd9, 0x32, 0x7e, 0xb1, 0xb1, 0xe3, 0x6a,
    0x21, 0xfd, 0xb7, 0x11, 0x14, 0xbe, 0x07, 0x43,
    0x4c, 0x0c, 0xc7, 0xbf, 0x63, 0xf6, 0xe1, 0xda,
    0x27, 0x4e, 0xde, 0xbf, 0xe7, 0x6f, 0x65, 0xfb,
    0xd5, 0x1a, 0xd2, 0xf1, 0x48, 0x98, 0xb9, 0x5b,
};

static const td_u8 g_empty_l_sha512[] = {
    0xcf, 0x83, 0xe1, 0x35, 0x7e, 0xef, 0xb8, 0xbd,
    0xf1, 0x54, 0x28, 0x50, 0xd6, 0x6d, 0x80, 0x07,
    0xd6, 0x20, 0xe4, 0x05, 0x0b, 0x57, 0x15, 0xdc,
    0x83, 0xf4, 0xa9, 0x21, 0xd3, 0x6c, 0xe9, 0xce,
    0x47, 0xd0, 0xd1, 0x3c, 0x5d, 0x85, 0xf2, 0xb0,
    0xff, 0x83, 0x18, 0xd2, 0x87, 0x7e, 0xec, 0x2f,
    0x63, 0xb9, 0x31, 0xbd, 0x47, 0x41, 0x7a, 0x81,
    0xa5, 0x38, 0x32, 0x7a, 0xf9, 0x27, 0xda, 0x3e,
};

typedef struct {
    td_u32 modbits;
    td_u32 embits;
    td_u32 emlen;
    ot_data_t salt_txt;
    td_u8 hash[HASH_RESULT_MAX_LEN];
} rsa_pss_t;

static td_s32 rsa_get_multi_random(td_u8 *randnum, td_u32 num_len, td_u32 size)
{
    td_s32 ret;
    td_u32 i, num;
    if (size > num_len) {
        log_error("Data length too large!\n");
        return TD_FAILURE;
    }
    (td_void)memset_s(randnum, size, 0, size);
    for (i = 0; i < size; i += 4) { /* 4 word bytes */
        ret = kapi_trng_get_random(&num, -1);
        chk_func_return_err(kapi_trng_get_random, ret != TD_SUCCESS, ret);

        ret = memcpy_s(randnum, size - i,
            (td_u8 *)&num, (((size - i) > 4) ? 4 : (size - i))); /* 4 word bytes */
        chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);
    }

    /* non-zero randnum octet string */
    for (i = 0; i < size; i++) {
        if (randnum[i] != 0x00) {
            continue;
        }

        ret = kapi_trng_get_random(&num, -1);
        chk_func_return_err(kapi_trng_get_random, ret != TD_SUCCESS, ret);
        randnum[i] = (td_u8)(num) & 0xFF;

        i--;
    }

    return TD_SUCCESS;
}

static td_u32 rsa_get_bit_num(const td_u8 *big_num, td_u32 num_len)
{
    static const td_s8 bits[16] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4}; /* 16 bits size */
    td_u32 i;

    chk_ptr_err_return(big_num);

    for (i = 0; i < num_len; i++) {
        td_u32 num;
        num = bits[(big_num[i] & 0xF0) >> 4]; /* 4 right shift */
        if (num > 0) {
            return (num_len - i - 1) * 8 + num + 4; /* 8, 4 */
        }

        num = bits[big_num[i] & 0xF];
        if (num > 0) {
            return (num_len - i - 1) * 8 + num; /* 8 */
        }
    }
    return 0;
}

static td_s32 rsa_get_hash_len(ot_cipher_hash_type hash_type, td_u32 *hash_len)
{
    switch (hash_type) {
        case OT_CIPHER_HASH_TYPE_SHA1:
        case OT_CIPHER_HASH_TYPE_SHA224:
            log_error("Unsupported hash %d\n", hash_type);
            return OT_ERR_CIPHER_UNSUPPORTED;
        case OT_CIPHER_HASH_TYPE_SHA256:
            *hash_len = SHA256_RESULT_SIZE;
            break;
        case OT_CIPHER_HASH_TYPE_SHA384:
            *hash_len = SHA384_RESULT_SIZE;
            break;
        case OT_CIPHER_HASH_TYPE_SHA512:
            *hash_len = SHA512_RESULT_SIZE;
            break;
        default:
            log_error("hash type is invalid.\n");
            return OT_ERR_CIPHER_INVALID_PARAM;
    }
    return TD_SUCCESS;
}

static td_s32 rsa_crypto_get_oaep_hash(ot_cipher_rsa_encrypt_scheme scheme,
    ot_cipher_hash_type *hash_type, const td_u8 **l_hash, td_u32 *hash_len)
{
    switch (scheme) {
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA256:
            *l_hash = (td_u8 *)g_empty_l_sha256;
            *hash_type = OT_CIPHER_HASH_TYPE_SHA256;
            *hash_len = SHA256_RESULT_SIZE;
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA384:
            *l_hash = (td_u8 *)g_empty_l_sha384;
            *hash_type = OT_CIPHER_HASH_TYPE_SHA384;
            *hash_len = SHA384_RESULT_SIZE;
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA512:
            *l_hash = (td_u8 *)g_empty_l_sha512;
            *hash_type = OT_CIPHER_HASH_TYPE_SHA512;
            *hash_len = SHA512_RESULT_SIZE;
            break;
        default:
            log_error("Unsupported scheme %d\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    return TD_SUCCESS;
}

static td_s32 rsa_sign_get_pkcs_v15_hash(ot_cipher_rsa_sign_scheme scheme,
    ot_cipher_hash_type *hash_type, ot_data_t *asn1)
{
    switch (scheme) {
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA256:
            asn1->data = (td_u8 *)g_asn1_sha256;
            asn1->data_len = sizeof(g_asn1_sha256);
            *hash_type = OT_CIPHER_HASH_TYPE_SHA256;
            break;
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA384:
            asn1->data = (td_u8 *)g_asn1_sha384;
            asn1->data_len = sizeof(g_asn1_sha384);
            *hash_type = OT_CIPHER_HASH_TYPE_SHA384;
            break;
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA512:
            asn1->data = (td_u8 *)g_asn1_sha512;
            asn1->data_len = sizeof(g_asn1_sha512);
            *hash_type = OT_CIPHER_HASH_TYPE_SHA512;
            break;
        default:
            log_error("Unsupported scheme %d\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    return TD_SUCCESS;
}

static td_s32 rsa_sign_get_pss_hash_type(ot_cipher_rsa_sign_scheme scheme,
    ot_cipher_hash_type *hash_type)
{
    switch (scheme) {
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA256:
            *hash_type = OT_CIPHER_HASH_TYPE_SHA256;
            break;
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA384:
            *hash_type = OT_CIPHER_HASH_TYPE_SHA384;
            break;
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA512:
            *hash_type = OT_CIPHER_HASH_TYPE_SHA512;
            break;
        default:
            log_error("Unsupported scheme %d\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    return TD_SUCCESS;
}

static td_s32 rsa_sign_calcu_pss_h(ot_cipher_hash_type hash_type,
    const ot_data_t *hash_txt, rsa_pss_t *pss)
{
    td_s32 ret;
    td_handle handle;
    ot_cipher_hash_attr hash_attr;
    td_u8 pss_pad1[PKCS_PSS_PAD1_LEN] = {0};
    td_u32 hash_len = 0;

    (td_void)memset_s(&hash_attr, sizeof(ot_cipher_hash_attr), 0, sizeof(ot_cipher_hash_attr));
    hash_attr.sha_type = hash_type;

    ret = kapi_hash_start(&handle, hash_type, TD_NULL, 0);
    chk_func_return_err(kapi_hash_start, ret != TD_SUCCESS, ret);

    ret = kapi_hash_update(handle, pss_pad1, sizeof(pss_pad1), HASH_CHUNCK_SRC_LOCAL);
    chk_func_return_err(kapi_hash_update, ret != TD_SUCCESS, ret);

    ret = kapi_hash_update(handle, hash_txt->data, hash_txt->data_len, HASH_CHUNCK_SRC_LOCAL);
    chk_func_return_err(kapi_hash_update, ret != TD_SUCCESS, ret);

    ret = kapi_hash_update(handle, pss->salt_txt.data, pss->salt_txt.data_len, HASH_CHUNCK_SRC_LOCAL);
    chk_func_return_err(kapi_hash_update, ret != TD_SUCCESS, ret);

    ret = kapi_hash_finish(handle, pss->hash, sizeof(pss->hash), &hash_len);
    chk_func_return_err(kapi_hash_finish, ret != TD_SUCCESS, ret);

    return TD_SUCCESS;
}

static td_s32 mgf(ot_cipher_hash_type hash_type,
    const td_u8 *seed, td_u32 seed_len, td_u8 *mask, td_u32 mask_len)
{
    td_s32 ret;
    td_u32 i, out_len, hash_len;
    td_u8 hash[HASH_RESULT_MAX_LEN] = {0};
    td_u8 cnt[4]; /* 4 */

    ret = rsa_get_hash_len(hash_type, &hash_len);
    chk_func_return_err(rsa_get_hash_len, ret != TD_SUCCESS, ret);

    for (i = 0, out_len = 0; out_len < mask_len; i++) {
        td_handle hash_handle;
        td_u32 j;

        cnt[0] = (td_u8)((i >> 24) & 0xFF); /* 0 ptr_cnt index, 24 right shift */
        cnt[1] = (td_u8)((i >> 16) & 0xFF); /* 1 ptr_cnt index, 16 right shift */
        cnt[2] = (td_u8)((i >>  8) & 0xFF); /* 2 ptr_cnt index, 8  right shift */
        cnt[3] = (td_u8)((i >>  0) & 0xFF); /* 3 ptr_cnt index, 0  right shift */

        ret = kapi_hash_start(&hash_handle, hash_type, TD_NULL, 0);
        chk_func_return_err(kapi_hash_start, ret != TD_SUCCESS, ret);

        ret = kapi_hash_update(hash_handle, seed, seed_len, HASH_CHUNCK_SRC_LOCAL);
        chk_func_return_err(kapi_hash_update, ret != TD_SUCCESS, ret);

        ret = kapi_hash_update(hash_handle, cnt, sizeof(cnt), HASH_CHUNCK_SRC_LOCAL);
        chk_func_return_err(kapi_hash_update, ret != TD_SUCCESS, ret);

        ret = kapi_hash_finish(hash_handle, hash, sizeof(hash), &hash_len);
        chk_func_return_err(kapi_hash_finish, ret != TD_SUCCESS, ret);

        for (j = 0; (j < hash_len) && (out_len < mask_len); j++) {
            mask[out_len++] ^= hash[j];
        }
    }

    return ret;
}

td_s32 rsa_encrypt_pad_none(const cryp_rsa_key *key, const ot_data_t *plain_txt, ot_data_t *em_txt)
{
    td_s32 ret;

    chk_param_err_return(em_txt->data_len < key->klen);
    chk_param_err_return(plain_txt->data_len != key->klen);

    ret = memcpy_s(em_txt->data, em_txt->data_len, plain_txt->data, plain_txt->data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    return TD_SUCCESS;
}

td_s32 rsa_decrypt_pad_none(const cryp_rsa_key *key, ot_data_t *plain_txt, const ot_data_t *em_txt)
{
    td_s32 ret;

    chk_param_err_return(em_txt->data_len < key->klen);
    chk_param_err_return(plain_txt->data_len != key->klen);

    ret = memcpy_s(plain_txt->data, plain_txt->data_len, em_txt->data, em_txt->data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    return TD_SUCCESS;
}

/* PKCS #1: block type 0,1,2 message padding */
/* ************************************************
formula: EB = 00 || BT || PS || 00 || M

formula: PS_LEN >= 8, mlen < klen - 11
 ************************************************ */
td_s32 rsa_encrypt_pad_pkcs_type(const cryp_rsa_key *key,
    const ot_data_t *plain_txt, ot_data_t *em_txt, td_u32 block_type)
{
    td_s32 ret;
    td_u32 em_idx = 0;
    td_u32 ps_len;

    chk_param_err_return(em_txt->data_len < key->klen);
    chk_param_err_return(plain_txt->data_len > (td_u32)(key->klen - PKCS_V15_MIN_PAD_LEN));

    em_txt->data[em_idx++] = 0;
    em_txt->data[em_idx++] = block_type;

    /* ps len: klen - data len - 3 */
    ps_len = key->klen - plain_txt->data_len - PKCS_V15_MIN_OTHER_LEN;

    /* BLOCK TYPE:
     *  0: PS PADDING 0x00,
     *  1: PS PADDING 0xFF,
     *  2: PS PADDING random
     */
    if (block_type == PKCS_BLOCK_TYPE_0) {
        ret = memset_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, 0x00, ps_len);
        chk_func_return_err(memset_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);
    } else if (block_type == PKCS_BLOCK_TYPE_1) {
        ret = memset_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, 0xFF, ps_len);
        chk_func_return_err(memset_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);
    } else if (block_type == PKCS_BLOCK_TYPE_2) {
        ret = rsa_get_multi_random(&em_txt->data[em_idx], em_txt->data_len - em_idx, ps_len);
        chk_func_return_err(rsa_get_multi_random, ret != TD_SUCCESS, ret);
    } else {
        log_error("BT(0x%x) is invalid.\n", block_type);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    em_idx += ps_len;

    em_txt->data[em_idx++] = 0x00;

    ret = memcpy_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, plain_txt->data, plain_txt->data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    return TD_SUCCESS;
}

td_s32 rsa_decrypt_pad_pkcs_type(const cryp_rsa_key *key,
    ot_data_t *plain_txt, const ot_data_t *em_txt, td_u32 block_type, td_u32 *outlen)
{
    td_s32 ret;
    td_u32 em_idx = 0;

    /* 1. check data[0] = 0 */
    chk_param_err_return(em_txt->data[em_idx++] != 0x00);

    /* 2. check data[1] = block_type */
    chk_param_err_return(em_txt->data[em_idx++] != block_type);

    if (block_type == PKCS_BLOCK_TYPE_0) {
        for (; em_idx < key->klen - PKCS_V15_MIN_OTHER_LEN; em_idx++) {
            if (em_txt->data[em_idx] == 0x00 && em_txt->data[em_idx + 1] != 0x00) {
                break;
            }
        }
    } else if (block_type == PKCS_BLOCK_TYPE_1) {
        for (; em_idx < key->klen - PKCS_V15_MIN_OTHER_LEN; em_idx++) {
            if (em_txt->data[em_idx] == 0x00) {
                break;
            } else if (em_txt->data[em_idx] != 0xFF) {
                log_error("PS data isn't 0xFF\n");
                return OT_ERR_CIPHER_FAILED_DECRYPT;
            }
        }
    } else if (block_type == PKCS_BLOCK_TYPE_2) {
        for (; em_idx < key->klen - PKCS_V15_MIN_OTHER_LEN; em_idx++) {
            if (em_txt->data[em_idx] == 0x00) {
                break;
            }
        }
    }

    em_idx++;
    *outlen = em_txt->data_len - em_idx;
    if (*outlen > key->klen - PKCS_V15_MIN_PAD_LEN) {
        log_error("PS Error!\n");
        return OT_ERR_CIPHER_FAILED_DECRYPT;
    }

    ret = memcpy_s(plain_txt->data, plain_txt->data_len, &em_txt->data[em_idx], *outlen);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    return TD_SUCCESS;
}

/* PKCS #1: RSAES-PKCS1-V1_5-ENCRYPT */
/*************************************************
formula: EM = 0x00 || 0x02 || PS || 0x00 || M

formula: PS_LEN > 8, mlen < klen - 11
 *************************************************/
td_s32 rsa_encrypt_pad_pkcs_v15(const cryp_rsa_key *key,
    const ot_data_t *plain_txt, ot_data_t *em_txt)
{
    return rsa_encrypt_pad_pkcs_type(key, plain_txt, em_txt, PKCS_BLOCK_TYPE_2);
}

td_s32 rsa_decrypt_pad_pkcs_v15(const cryp_rsa_key *key,
    ot_data_t *plain_txt, const ot_data_t *em_txt, td_u32 *outlen)
{
    td_s32 ret = TD_FAILURE;
    ret = rsa_decrypt_pad_pkcs_type(key, plain_txt, em_txt, PKCS_BLOCK_TYPE_2, outlen);
    if (ret != TD_SUCCESS) {
        return OT_ERR_CIPHER_FAILED_DECRYPT;
    }
    return ret;
}

/* PKCS #1: EME-OAEP encoding */
/* ************************************************************
   +----------+---------+--+-------+
   DB = |  lHash   |    PS   |01|   M   |
   +----------+---------+--+-------+
   |
   +----------+              V
   |   ptr_seed   |--> MGF ---> xor
   +----------+              |
   |                   |
   +--+     V                   |
   |00|    xor <----- MGF <-----|
   +--+     |                   |
   |      |                   |
   V      V                   V
   +--+----------+----------------------------+
   EM =  |00|maskedSeed|          maskedDB          |
   +--+----------+----------------------------+
   1     hlen              k - hlen- 1

so: PS_LEN = k - hlen - 1 -  (hlen  + mlen + 1) = k - 2hlen - mlen - 2 > 0
so: mlen < k - 2hlen - 2
 ************************************************************ */
td_s32 rsa_encrypt_pad_pkcs_oaep(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *plain_txt, ot_data_t *em_txt)
{
    td_s32 ret;
    td_u8 *ptr_db = TD_NULL;
    td_u8 *ptr_seed = TD_NULL;
    const td_u8 *l_hash = TD_NULL;
    ot_cipher_hash_type hash_type = OT_CIPHER_HASH_TYPE_BUTT;
    td_u32 hash_len = 0;
    td_u32 em_idx = 0;

    ret = rsa_crypto_get_oaep_hash(scheme, &hash_type, &l_hash, &hash_len);
    chk_func_return_err(rsa_crypto_get_oaep_hash, ret != TD_SUCCESS, ret);

    chk_param_err_return(plain_txt->data_len > key->klen - 2 * hash_len - 2); /* 2 */

    ptr_seed = em_txt->data + 1;
    ptr_db = em_txt->data + hash_len + 1;

    /* 1. set data[0] = 0 */
    em_txt->data[em_idx++] = 0;

    /* 2. set data[1, hash_len + 1] = random */
    ret = rsa_get_multi_random(&em_txt->data[em_idx], em_txt->data_len - em_idx, hash_len);
    chk_func_return_err(rsa_get_multi_random, ret != TD_SUCCESS, ret);
    em_idx += hash_len;

    /* 3. set data[hash_len + 1, 2hash_len + 1] = lHash */
    ret = memcpy_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, l_hash, hash_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);
    em_idx += hash_len;

    /* 4. set PS with 0x00 */
    ret = memset_s(&em_txt->data[em_idx], em_txt->data_len - em_idx,
        0, key->klen - plain_txt->data_len - 2 * hash_len - 2);  /* 2 */
    chk_func_return_err(memset_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);
    em_idx += key->klen - plain_txt->data_len - 2 * hash_len - 2; /* 2 */

    /* 5. set 0x01 after PS */
    em_txt->data[em_idx++] = 0x01;

    /* 6. set M */
    ret = memcpy_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, plain_txt->data, plain_txt->data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    /* 7. MGF: seed -> db */
    ret = mgf(hash_type, ptr_seed, hash_len, ptr_db, key->klen - hash_len - 1);
    chk_func_return_err(mgf, ret != TD_SUCCESS, ret);

    /* 8. MGF: db -> seed */
    ret = mgf(hash_type, ptr_db, key->klen - hash_len - 1, ptr_seed, hash_len);
    chk_func_return_err(mgf, ret != TD_SUCCESS, ret);

    return ret;
}

td_s32 rsa_decrypt_pad_pkcs_oaep(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, ot_data_t *plain_txt, ot_data_t *em_txt, td_u32 *outlen)
{
    td_s32 ret;
    td_u8 *ptr_db = TD_NULL;
    td_u8 *ptr_seed = TD_NULL;
    const td_u8 *l_hash = TD_NULL;
    ot_cipher_hash_type hash_type = OT_CIPHER_HASH_TYPE_BUTT;
    td_u32 hash_len = 0;
    td_s32 em_idx = 0;

    ret = rsa_crypto_get_oaep_hash(scheme, &hash_type, &l_hash, &hash_len);
    chk_func_return_err(rsa_crypto_get_oaep_hash, ret != TD_SUCCESS, ret);

    ptr_seed = em_txt->data + 1;
    ptr_db = em_txt->data + hash_len + 1;

    /* 1. check data[0] = 0 */
    if (em_txt->data[em_idx++] != 0x00) {
        return OT_ERR_CIPHER_FAILED_DECRYPT;
    }

    /* 2. MGF: db -> seed */
    ret = mgf(hash_type, ptr_db, key->klen - hash_len - 1, ptr_seed, hash_len);
    chk_func_return_err(mgf, ret != TD_SUCCESS, ret);

    /* 3. MGF: seed -> db */
    ret = mgf(hash_type, ptr_seed, hash_len, ptr_db, key->klen - hash_len - 1);
    chk_func_return_err(mgf, ret != TD_SUCCESS, ret);

    /* 4. check data[hash + 1, 2hash + 1] */
    em_idx += hash_len;
    ret = memcmp(&em_txt->data[em_idx], l_hash, hash_len);
    chk_func_return_err(memcmp, ret != 0, OT_ERR_CIPHER_FAILED_DECRYPT);

    /* 5. remove PS */
    em_idx += hash_len;
    for (; em_idx < key->klen - 1; em_idx++) {
        if (em_txt->data[em_idx] != 0x00) {
            break;
        }
    }

    /* 6. check 0x01 */
    if (em_txt->data[em_idx] != 0x01) {
        log_error("em_txt->data[%u] = %u isn't 0x01!\n", em_idx, em_txt->data[em_idx]);
        return OT_ERR_CIPHER_FAILED_DECRYPT;
    }
    em_idx++;

    /* 7. check data length */
    *outlen = em_txt->data_len - em_idx;
    if ((*outlen == 0) || (*outlen > key->klen - 2 * hash_len - 2)) { /* 2 */
        log_error("PS error.\n");
        return OT_ERR_CIPHER_FAILED_DECRYPT;
    }

    ret = memcpy_s(plain_txt->data, plain_txt->data_len, &em_txt->data[em_idx], *outlen);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    return TD_SUCCESS;
}

td_s32 rsa_sign_pad_pkcs_v15(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, ot_data_t *em_txt)
{
    td_s32 ret;
    td_u32 em_idx = 0;
    td_u32 ps_len;
    ot_data_t asn1 = {0};
    ot_cipher_hash_type hash_type = OT_CIPHER_HASH_TYPE_BUTT;

    chk_param_err_return(em_txt->data_len < key->klen);

    ret = rsa_sign_get_pkcs_v15_hash(scheme, &hash_type, &asn1);
    chk_func_return_err(rsa_sign_get_pkcs_v15_hash, ret != TD_SUCCESS, ret);

    chk_param_err_return((hash_txt->data_len + asn1.data_len) > (key->klen - PKCS_V15_MIN_PAD_LEN));

    /* 1. set data[0] = 0 */
    em_txt->data[em_idx++] = 0x00;

    /* 2. set data[1] = 1 */
    em_txt->data[em_idx++] = 0x01;

    /* 3. padding PS 0xFF */
    ps_len = key->klen - asn1.data_len - hash_txt->data_len - PKCS_V15_MIN_OTHER_LEN;
    ret = memset_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, 0xFF, ps_len);
    chk_func_return_err(memset_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);
    em_idx += ps_len;

    /* 4. set data[2 + pslen] = 0 */
    em_txt->data[em_idx++] = 0x00;

    /* 5. combine asn1 */
    ret = memcpy_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, asn1.data, asn1.data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);
    em_idx += asn1.data_len;

    /* 6. combine hash */
    ret = memcpy_s(&em_txt->data[em_idx], em_txt->data_len - em_idx, hash_txt->data, hash_txt->data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    return TD_SUCCESS;
}

td_s32 rsa_verify_pad_pkcs_v15(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, const ot_data_t *em_txt)
{
    td_s32 ret;
    td_u32 em_idx = 0;
    td_u32 ps_len;
    ot_data_t asn1 = {0};
    ot_cipher_hash_type hash_type = OT_CIPHER_HASH_TYPE_BUTT;

    chk_param_err_return(em_txt->data_len < key->klen);

    ret = rsa_sign_get_pkcs_v15_hash(scheme, &hash_type, &asn1);
    chk_func_return_err(rsa_sign_get_pkcs_v15_hash, ret != TD_SUCCESS, ret);

    chk_param_err_return((hash_txt->data_len + asn1.data_len) > (key->klen - PKCS_V15_MIN_PAD_LEN));

    /* 1. check data[0] = 0 */
    chk_param_err_return(em_txt->data[em_idx++] != 0x00);

    /* 2. check data[1] = 0x01 */
    chk_param_err_return(em_txt->data[em_idx++] != 0x01);

    /* 3. padding PS */
    ps_len = key->klen - asn1.data_len - hash_txt->data_len - PKCS_V15_MIN_OTHER_LEN;
    for (; em_idx < ps_len + 2; em_idx++) { /* cur em_idx offset 2 bytes */
        if (em_txt->data[em_idx] != 0xFF) {
            log_error("em_txt[%u] %x isn't 0xFF\n", em_idx, em_txt->data[em_idx]);
            return OT_ERR_CIPHER_FAILED_DECRYPT;
        }
    }

    /* 4. check data[2 + ps_len] = 0x00 */
    if (em_txt->data[em_idx] != 0x00) {
        log_error("em_txt[%u] %x isn't 0xFF\n", em_idx, em_txt->data[em_idx]);
        return OT_ERR_CIPHER_FAILED_DECRYPT;
    }
    em_idx++;

    /* 5. check asn1 */
    ret = memcmp(&em_txt->data[em_idx], asn1.data, asn1.data_len);
    chk_func_return_err(memcmp, ret != 0, OT_ERR_CIPHER_FAILED_DECRYPT);
    em_idx += asn1.data_len;

    /* 6. check hash */
    ret = memcmp(&em_txt->data[em_idx], hash_txt->data, hash_txt->data_len);
    chk_func_return_err(memcmp, ret != 0, OT_ERR_CIPHER_FAILED_DECRYPT);

    return TD_SUCCESS;
}

/* *****************************************************************
   +-----------+
   |     M     |
   +-----------+
   |
   V
   Hash
   |
   V
   +--------+----------+----------+
   M' =  |Padding1|  mHash   |   salt   |
   +--------+----------+----------+
   |
   +--------+----------+       V
   DB =   |Padding2|maskedseed|      Hash
   +--------+----------+       |
   |                    |
   V                    |      +--+
   xor <----- MGF <----|      |bc|
   |                    |      +--+
   |                    |      |
   V                    V      V
   +-------------------+----- -------+--+
   EM =    |    maskedDB       |  maskedseed |bc|
   +-------------------+-------------+--+

Note: SIGN only support the length of salt that is equal to hash'length
 ***************************************************************** */
td_s32 rsa_sign_pad_pkcs_pss(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, ot_data_t *em_txt, td_u32 *outlen)
{
    td_s32 ret;
    td_u32 ps_len;
    rsa_pss_t pss = {0};
    td_u8 *ptr_db = TD_NULL;
    td_u8 *ptr_seed = TD_NULL;
    ot_cipher_hash_type hash_type = OT_CIPHER_HASH_TYPE_BUTT;
    td_u8 salt[HASH_RESULT_MAX_LEN] = {0};

    ret = rsa_sign_get_pss_hash_type(scheme, &hash_type);
    chk_func_return_err(rsa_sign_get_pss_hash_type, ret != TD_SUCCESS, ret);

    pss.modbits = rsa_get_bit_num(key->n, key->klen);
    pss.embits = pss.modbits - 1;
    pss.emlen = (pss.modbits + 7) / 8; /* 7, 8, round up */

    /* check emlen min length */
    chk_param_err_return(pss.emlen < (hash_txt->data_len * 2  + 2)); /* 2 */

    ptr_db = em_txt->data;
    ptr_seed = em_txt->data + pss.emlen - hash_txt->data_len - 1;

    ret = rsa_get_multi_random(salt, HASH_RESULT_MAX_LEN, hash_txt->data_len);
    chk_func_return_err(rsa_get_multi_random, ret != TD_SUCCESS, ret);
    pss.salt_txt.data = salt;
    pss.salt_txt.data_len = hash_txt->data_len;

    /* 1. calculate H */
    ret = rsa_sign_calcu_pss_h(hash_type, hash_txt, &pss);
    chk_func_return_err(rsa_sign_calcu_pss_h, ret != TD_SUCCESS, ret);

    ret = memcpy_s(ptr_seed, em_txt->data_len - (ptr_seed - em_txt->data), pss.hash, hash_txt->data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    /* 2. set DB PAD2 */
    ps_len = pss.emlen - 2 * hash_txt->data_len - 2;
    ret = memset_s(ptr_db, em_txt->data_len, 0, ps_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    /* 3. set DB data[ps_len] = 0x01 */
    em_txt->data[ps_len] = 0x01;

    /* 4. set DB salt */
    ret = memcpy_s(ptr_db + ps_len + 1, em_txt->data_len - (ps_len + 1), salt, hash_txt->data_len);
    chk_func_return_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    if ((pss.embits & 0x07) == 0) {
        ptr_db++;
        ps_len--;
    }

    /* 5. mgf */
    ret = mgf(hash_type, ptr_seed, hash_txt->data_len, ptr_db, ps_len + hash_txt->data_len + 1);
    chk_func_return_err(mgf, ret != TD_SUCCESS, ret);

    /* 6. Set the leftmost 8emLen - emBits bits of the leftmost octet in maskedDB to zero */
    em_txt->data[0] &= (0xFF >> (pss.emlen * 8 - pss.embits)); /* 8 BYTE BITS */

    /* 7. Set data[emlen - 1] = 0xBC */
    em_txt->data[pss.emlen - 1] = 0xBC;

    *outlen = pss.emlen;

    return TD_SUCCESS;
}

td_s32 rsa_verify_pad_pkcs_pss(const cryp_rsa_key *key,
    ot_cipher_rsa_encrypt_scheme scheme, const ot_data_t *hash_txt, ot_data_t *em_txt)
{
    td_s32 ret;
    td_s32 sign_len;
    rsa_pss_t pss = {0};
    td_u8 *ptr_db = TD_NULL;
    td_u8 *ptr_seed = TD_NULL;
    ot_cipher_hash_type hash_type = OT_CIPHER_HASH_TYPE_BUTT;

    ret = rsa_sign_get_pss_hash_type(scheme, &hash_type);
    chk_func_return_err(rsa_sign_get_pss_hash_type, ret != TD_SUCCESS, ret);

    pss.modbits = rsa_get_bit_num(key->n, key->klen);
    pss.embits = pss.modbits - 1;
    pss.emlen = (pss.modbits + 7) / 8; /* 7, 8, round up */
    sign_len = pss.emlen;

    /* check emlen min length */
    chk_param_err_return(pss.emlen < (hash_txt->data_len * 2  + 2)); /* 2 */

    /* 1. check em[0] */
    chk_param_err_return(em_txt->data[0] & ~(0xFF >> (pss.emlen * 8 - pss.embits))); /* 8 BYTE BITS */

    /* 2. check em[emlen - 1] != 0xBC */
    chk_param_err_return(em_txt->data[pss.emlen - 1] != 0xBC);

    ptr_db = em_txt->data;
    ptr_seed = em_txt->data + pss.emlen - hash_txt->data_len - 1;

    if ((pss.embits & 0x07) == 0) {
        ptr_db++;
        sign_len--;
    }

    /* 3. calculate DB */
    ret = mgf(hash_type, ptr_seed, hash_txt->data_len, ptr_db, sign_len - hash_txt->data_len - 1);
    chk_func_return_err(mgf, ret != TD_SUCCESS, ret);

    /* 4. Set the leftmost 8emLen - emBits bits of the leftmost octet in maskedDB to zero */
    em_txt->data[0] &= (0xFF >> (pss.emlen * 8 - pss.embits)); /* 8 BYTE BITS */

    /* 5. Remove PS */
    while (ptr_db < ptr_seed - 1 && *ptr_db == 0) {
        ptr_db++;
    }

    /* 6. check DB[pslen] whether is 0x01 */
    chk_param_err_return(*ptr_db++ != 0x01);

    /* 7. get salt */
    pss.salt_txt.data_len = ptr_seed - ptr_db;
    pss.salt_txt.data = (td_u8 *)crypto_calloc(pss.salt_txt.data_len, 1);
    chk_func_return_err(crypto_calloc, pss.salt_txt.data == TD_NULL, OT_ERR_CIPHER_FAILED_MEM);

    ret = memcpy_s(pss.salt_txt.data, pss.salt_txt.data_len, ptr_db, pss.salt_txt.data_len);
    chk_func_goto_err(memcpy_s, ret != EOK, OT_ERR_CIPHER_FAILED_SEC_FUNC);

    /* 8. calculate H */
    ret = rsa_sign_calcu_pss_h(hash_type, hash_txt, &pss);
    chk_func_goto_err(rsa_sign_calcu_pss_h, ret != TD_SUCCESS, ret);

    /* 9. compare H */
    ret = memcmp(pss.hash, ptr_seed, hash_txt->data_len);
    chk_func_goto_err(memcmp, ret != 0, OT_ERR_CIPHER_RSA_VERIFY);

goto_exit:
    if (pss.salt_txt.data != TD_NULL) {
        crypto_free(pss.salt_txt.data);
        pss.salt_txt.data = TD_NULL;
    }
    return ret;
}

