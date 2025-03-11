// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "cryp_rsa.h"
#include "drv_osal_lib.h"
#include "drv_pke.h"
#include "cryp_trng.h"
#include "rsa_padding.h"

#if (defined(CHIP_RSA_SUPPORT) || defined(SOFT_RSA_SUPPORT))

#define RSA_KEY_LEN_MAX                 RSA_KEY_LEN_4096

int cryp_rsa_init(void)
{
    func_enter();

#ifdef CHIP_PKE_SUPPORT
    {
        td_s32 ret;
        ret = drv_pke_init();
        if (ret != TD_SUCCESS) {
            print_func_errno(drv_pke_init, ret);
            return ret;
        }
    }
#endif

    func_exit();
    return TD_SUCCESS;
}

void cryp_rsa_deinit(void)
{
#ifdef CHIP_PKE_SUPPORT
    drv_pke_deinit();
#endif
}

static td_s32 cryp_rsa_get_key(const cryp_rsa_key *key, td_u8 **pkey, rsa_key_width *width)
{
    td_s32 ret;
    static td_u8 rsa_key[RSA_KEY_LEN_MAX];

    (td_void)memset_s(rsa_key, sizeof(rsa_key), 0, sizeof(rsa_key));
    *pkey = rsa_key;

    if (key->public == TD_TRUE) {
        ret = memcpy_s(rsa_key + key->klen - sizeof(td_u32),
            RSA_KEY_LEN_MAX - (key->klen - sizeof(td_u32)), (td_u8 *)&key->e, sizeof(td_u32));
    } else {
        ret = memcpy_s(rsa_key, RSA_KEY_LEN_MAX, key->d, key->klen);
    }
    if (ret != EOK) {
        print_func_errno(memcpy_s, OT_ERR_CIPHER_FAILED_SEC_FUNC);
        return OT_ERR_CIPHER_FAILED_SEC_FUNC;
    }

    switch (key->klen) {
        case RSA_KEY_LEN_2048:
            *width = RSA_KEY_WIDTH_2048;
            break;
        case RSA_KEY_LEN_3072:
            *width = RSA_KEY_WIDTH_3072;
            break;
        case RSA_KEY_LEN_4096:
            *width = RSA_KEY_WIDTH_4096;
            break;
        default:
            log_error("Unsupported klen %u\n", key->klen);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    return TD_SUCCESS;
}

td_s32 cryp_rsa_encrypt(ot_cipher_rsa_encrypt_scheme scheme,
    const cryp_rsa_key *key, cryp_rsa_crypto_data *rsa_data)
{
    td_s32 ret;
    td_u8 *pkey = TD_NULL;
    rsa_key_width width = RSA_KEY_WIDTH_COUNT;
    td_u8 arr_em[RSA_KEY_LEN_MAX] = {0};
    ot_data_t plain_txt = {0};
    ot_data_t em_txt = {0};

    plain_txt.data = rsa_data->in;
    plain_txt.data_len = rsa_data->in_len;
    em_txt.data = arr_em;
    em_txt.data_len = sizeof(arr_em);

    ret = cryp_rsa_get_key(key, &pkey, &width);
    chk_func_return_err(cryp_rsa_get_key, ret != TD_SUCCESS, ret);

    switch (scheme) {
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_NO_PADDING:
            ret = rsa_encrypt_pad_none(key, &plain_txt, &em_txt);
            chk_func_return_err(rsa_encrypt_pad_none, ret != TD_SUCCESS, ret);
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_BLOCK_TYPE_0:
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_BLOCK_TYPE_1:
            log_error("RSA padding unsupported, mode = 0x%x. the block type shall be 02.\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_BLOCK_TYPE_2:
            ret = rsa_encrypt_pad_pkcs_type(key, &plain_txt, &em_txt, PKCS_BLOCK_TYPE_2);
            chk_func_return_err(rsa_encrypt_pad_pkcs_type, ret != TD_SUCCESS, ret);
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_PKCS1_V1_5:
            ret = rsa_encrypt_pad_pkcs_v15(key, &plain_txt, &em_txt);
            chk_func_return_err(rsa_encrypt_pad_pkcs_v15, ret != TD_SUCCESS, ret);
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA256:
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA384:
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA512:
            ret = rsa_encrypt_pad_pkcs_oaep(key, scheme, &plain_txt, &em_txt);
            chk_func_return_err(rsa_encrypt_pad_pkcs_oaep, ret != TD_SUCCESS, ret);
            break;
        default:
            log_error("Unsupported scheme %d\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    rsa_data->out_len = key->klen;

    ret = drv_ifep_rsa_exp_mod(key->n, pkey, em_txt.data, rsa_data->out, width);
    chk_func_return_err(drv_ifep_rsa_exp_mod, ret != TD_SUCCESS, ret);

    return ret;
}

td_s32 cryp_rsa_decrypt(ot_cipher_rsa_encrypt_scheme scheme,
    const cryp_rsa_key *key, cryp_rsa_crypto_data *rsa_data)
{
    td_s32 ret;
    td_u8 *pkey = TD_NULL;
    rsa_key_width width = RSA_KEY_WIDTH_COUNT;
    td_u8 arr_em[RSA_KEY_LEN_MAX] = {0};
    ot_data_t plain_txt = {0};
    ot_data_t em_txt = {0};

    em_txt.data = arr_em;
    em_txt.data_len = key->klen;
    plain_txt.data = rsa_data->out;
    plain_txt.data_len = key->klen;

    ret = cryp_rsa_get_key(key, &pkey, &width);
    chk_func_return_err(cryp_rsa_get_key, ret != TD_SUCCESS, ret);

    ret = drv_ifep_rsa_exp_mod(key->n, pkey, rsa_data->in, em_txt.data, width);
    chk_func_return_err(drv_ifep_rsa_exp_mod, ret != TD_SUCCESS, ret);

    switch (scheme) {
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_NO_PADDING:
            ret = rsa_decrypt_pad_none(key, &plain_txt, &em_txt);
            chk_func_return_err(rsa_decrypt_pad_none, ret != TD_SUCCESS, ret);
            rsa_data->out_len = key->klen;
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_BLOCK_TYPE_0:
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_BLOCK_TYPE_1:
            log_error("RSA padding unsupported, mode = 0x%x. the block type shall be 02.\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_BLOCK_TYPE_2:
            ret = rsa_decrypt_pad_pkcs_type(key, &plain_txt, &em_txt, PKCS_BLOCK_TYPE_2, &rsa_data->out_len);
            chk_func_return_err(rsa_decrypt_pad_pkcs_type, ret != TD_SUCCESS, ret);
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_PKCS1_V1_5:
            ret = rsa_decrypt_pad_pkcs_v15(key, &plain_txt, &em_txt, &rsa_data->out_len);
            chk_func_return_err(rsa_decrypt_pad_pkcs_v15, ret != TD_SUCCESS, ret);
            break;
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA256:
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA384:
        case OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA512:
            ret = rsa_decrypt_pad_pkcs_oaep(key, scheme, &plain_txt, &em_txt, &rsa_data->out_len);
            chk_func_return_err(rsa_decrypt_pad_pkcs_oaep, ret != TD_SUCCESS, ret);
            break;
        default:
            log_error("Unsupported scheme %d\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    return TD_SUCCESS;
}

td_s32 cryp_rsa_sign_hash(ot_cipher_rsa_sign_scheme scheme,
    const cryp_rsa_key *key, cryp_rsa_sign_data *rsa_sign)
{
    td_s32 ret;
    td_u8 *pkey = TD_NULL;
    rsa_key_width width = RSA_KEY_WIDTH_COUNT;
    td_u8 arr_em[RSA_KEY_LEN_MAX] = {0};
    ot_data_t em_txt;
    ot_data_t hash_txt;

    em_txt.data = arr_em;
    em_txt.data_len = key->klen;
    hash_txt.data = rsa_sign->in;
    hash_txt.data_len = rsa_sign->in_len;

    chk_ptr_err_return(key->d);
    ret = cryp_rsa_get_key(key, &pkey, &width);
    chk_func_return_err(cryp_rsa_get_key, ret != TD_SUCCESS, ret);

    switch (scheme) {
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA1:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA224:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA256:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA384:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA512:
            ret = rsa_sign_pad_pkcs_v15(key, scheme, &hash_txt, &em_txt);
            chk_func_return_err(rsa_sign_pad_pkcs_v15, ret != TD_SUCCESS, ret);
            rsa_sign->out_len = key->klen;
            break;
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA1:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA224:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA256:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA384:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA512:
            ret = rsa_sign_pad_pkcs_pss(key, scheme, &hash_txt, &em_txt, &rsa_sign->out_len);
            chk_func_return_err(rsa_sign_pad_pkcs_pss, ret != TD_SUCCESS, ret);
            break;
        default:
            log_error("Unsupported scheme %d\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    ret = drv_ifep_rsa_exp_mod(key->n, pkey, em_txt.data, rsa_sign->out, width);
    chk_func_return_err(drv_ifep_rsa_exp_mod, ret != TD_SUCCESS, ret);

    return TD_SUCCESS;
}

td_s32 cryp_rsa_verify_hash(ot_cipher_rsa_sign_scheme scheme,
    const cryp_rsa_key *key, const cryp_rsa_sign_data *rsa_verify)
{
    td_s32 ret;
    td_u8 *pkey = TD_NULL;
    rsa_key_width width = RSA_KEY_WIDTH_COUNT;
    td_u8 arr_em[RSA_KEY_LEN_MAX] = {0};
    ot_data_t hash_txt = {0};
    ot_data_t em_txt = {0};

    hash_txt.data = rsa_verify->in;
    hash_txt.data_len = rsa_verify->in_len;
    em_txt.data = arr_em;
    em_txt.data_len = key->klen;

    ret = cryp_rsa_get_key(key, &pkey, &width);
    chk_func_return_err(cryp_rsa_get_key, ret != TD_SUCCESS, ret);

    ret = drv_ifep_rsa_exp_mod(key->n, pkey, rsa_verify->out, em_txt.data, width);
    chk_func_return_err(drv_ifep_rsa_exp_mod, ret != TD_SUCCESS, ret);

    switch (scheme) {
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA1:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA224:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA256:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA384:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA512:
            ret = rsa_verify_pad_pkcs_v15(key, scheme, &hash_txt, &em_txt);
            chk_func_return_err(rsa_verify_pad_pkcs_v15, ret != TD_SUCCESS, OT_ERR_CIPHER_RSA_VERIFY);
            break;
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA1:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA224:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA256:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA384:
        case OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA512:
            ret = rsa_verify_pad_pkcs_pss(key, scheme, &hash_txt, &em_txt);
            chk_func_return_err(rsa_verify_pad_pkcs_pss, ret != TD_SUCCESS, OT_ERR_CIPHER_RSA_VERIFY);
            break;
        default:
            log_error("Unsupported scheme %d\n", scheme);
            return OT_ERR_CIPHER_UNSUPPORTED;
    }

    return TD_SUCCESS;
}
#endif

/** @} */ /** <!-- ==== API Code end ==== */
