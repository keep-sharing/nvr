// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "ot_mpi_cipher.h"
#include "drv_osal_lib.h"
#include "drv_cipher_kapi.h"

#define MAX_TAG_LEN             16
#define CIPHER_MAX_MULTIPAD_NUM 5000
static td_bool g_init_flag = TD_FALSE;

#define mpi_chk_init_err_return() do {      \
    if (g_init_flag != TD_TRUE) {           \
        return OT_ERR_CIPHER_NOT_INIT;      \
    }                                       \
} while (0)

/*
 * Read E in public key from array to U32,
 * so only use last byte0~byte3, others are zero
 */
#define cipher_get_pub_exponent(pub_e, pub_rsa)                 \
    do {                                                        \
        td_u8 *_buf = (pub_rsa)->e;                             \
        td_u8 *_pub = (td_u8 *)(pub_e);                         \
        td_u32 _len = (pub_rsa)->e_len;                         \
        td_u32 _i;                                              \
        for (_i = 0; _i < crypto_min(WORD_WIDTH, _len); _i++) { \
            _pub[WORD_WIDTH - _i - 1] = _buf[_len - _i - 1];    \
        }                                                       \
    } while (0)

/*
 * brief  Init the cipher device.
 */
td_s32 ot_mpi_cipher_init(td_void)
{
    td_s32 ret = TD_FAILURE;
    if (g_init_flag == TD_TRUE) {
        return TD_SUCCESS;
    }
    (td_void)crypto_get_cpu_secure_sta();
    ret = crypto_entry();
    if (ret != TD_SUCCESS) {
        return ret;
    }
    g_init_flag = TD_TRUE;
    return TD_SUCCESS;
}

/*
 * brief  Deinit the cipher device.
 */
td_s32 ot_mpi_cipher_deinit(td_void)
{
    if (g_init_flag == TD_TRUE) {
        g_init_flag = TD_FALSE;
        return crypto_exit();
    }
    return TD_SUCCESS;
}

/*
 * brief Obtain a cipher handle for encryption and decryption.
 */
td_s32 ot_mpi_cipher_create(td_handle *handle, const ot_cipher_attr *cipher_attr)
{
    td_s32 ret;
    td_u32 id = 0;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(handle);
    chk_ptr_err_return(cipher_attr);
    chk_param_err_return(cipher_attr->cipher_type >= OT_CIPHER_TYPE_BUTT);

    ret = kapi_symc_create(&id, cipher_attr->cipher_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_create, ret);
        return ret;
    }

    *handle = (td_handle)id;

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief Destroy the existing cipher handle.
 */
td_s32 ot_mpi_cipher_destroy(td_handle handle)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    ret = kapi_symc_destroy(handle);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_destroy, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

static td_s32 mpi_symc_set_cfg(symc_cfg_t *config, td_u32 id, const ot_cipher_ctrl *ctrl)
{
    const td_u32 *iv = TD_NULL;

    (td_void)memset_s(config, sizeof(symc_cfg_t), 0, sizeof(symc_cfg_t));
    config->id = id;
    config->alg = ctrl->alg;
    config->mode = ctrl->work_mode;
    config->width = OT_CIPHER_BIT_WIDTH_128BIT;
    config->ivlen = AES_IV_SIZE;

    if (ctrl->alg == OT_CIPHER_ALG_AES) {
        if (ctrl->work_mode == OT_CIPHER_WORK_MODE_CCM ||
            ctrl->work_mode == OT_CIPHER_WORK_MODE_GCM) {
            const ot_cipher_ctrl_aes_ccm_gcm *aes_ccm_gcm = &ctrl->aes_ccm_gcm_ctrl;
            config->klen = aes_ccm_gcm->key_len;
            config->ivlen = aes_ccm_gcm->iv_len;
            config->iv_usage = OT_CIPHER_IV_CHG_ONE_PACK;
            config->tlen = aes_ccm_gcm->tag_len;
            config->alen = aes_ccm_gcm->aad_len;
            addr_u64(config->aad) = aes_ccm_gcm->aad_phys_addr;
            iv = aes_ccm_gcm->iv;
        } else {
            const ot_cipher_ctrl_aes *aes = &ctrl->aes_ctrl;
            config->klen = aes->key_len;
            config->iv_usage = aes->chg_flags;
            config->width = aes->bit_width;
            iv = aes->iv;
        }
    } else if (ctrl->alg == OT_CIPHER_ALG_SM4) {
        const ot_cipher_ctrl_sm4 *sm4 = &ctrl->sm4_ctrl;
        config->iv_usage = sm4->chg_flags;
        iv = sm4->iv;
    } else if (ctrl->alg != OT_CIPHER_ALG_DMA) {
        log_error("Unsupported alg %d\n", ctrl->alg);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    if (iv != TD_NULL) {
        if (memcpy_s(config->iv, sizeof(config->iv), iv, config->ivlen) != EOK) {
            print_func_errno(memcpy_s, OT_ERR_CIPHER_FAILED_SEC_FUNC);
            return OT_ERR_CIPHER_FAILED_SEC_FUNC;
        }
    }

    return TD_SUCCESS;
}

/*
 * brief Configures the cipher control information.
 */
td_s32 ot_mpi_cipher_set_cfg(td_handle handle, const ot_cipher_ctrl *ctrl)
{
    td_s32 ret;
    symc_cfg_t config;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(ctrl);

    ret = mpi_symc_set_cfg(&config, handle, ctrl);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_symc_set_cfg, ret);
        return ret;
    }

    ret = kapi_symc_cfg(&config);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_cfg, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_get_cfg(td_handle handle, ot_cipher_ctrl *ctrl)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(ctrl);

    ret = kapi_symc_get_cfg(handle, ctrl);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_get_cfg, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief Performs encryption.
 */
td_s32 ot_mpi_cipher_encrypt(td_handle handle,
    td_phys_addr_t src_phys_addr, td_phys_addr_t dst_phys_addr, td_u32 byte_len)
{
    td_s32 ret;
    symc_encrypt_t crypto;

    mpi_chk_init_err_return();
    func_enter();

    crypto.id = handle;
    crypto.operation = SYMC_OPERATION_ENCRYPT;
    crypto.last = TD_TRUE;
    crypto.length = byte_len;
    addr_u64(crypto.input) = src_phys_addr;
    addr_u64(crypto.output) = dst_phys_addr;
    ret = kapi_symc_crypto(&crypto);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_crypto, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief Performs description.
 */
td_s32 ot_mpi_cipher_decrypt(td_handle handle,
    td_phys_addr_t src_phys_addr, td_phys_addr_t dst_phys_addr, td_u32 byte_len)
{
    td_s32 ret;
    symc_encrypt_t crypto;

    mpi_chk_init_err_return();
    func_enter();

    crypto.id = handle;
    crypto.operation = SYMC_OPERATION_DECRYPT;
    crypto.last = TD_TRUE;
    crypto.length = byte_len;
    addr_u64(crypto.input) = src_phys_addr;
    addr_u64(crypto.output) = dst_phys_addr;

    ret = kapi_symc_crypto(&crypto);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_crypto, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief Performs encryption.
 */
td_s32 ot_mpi_cipher_encrypt_virt(td_handle handle, const td_u8 *src_data, td_u8 *dst_data, td_u32 byte_len)
{
    td_s32 ret;
    symc_encrypt_t crypto;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(src_data);
    chk_ptr_err_return(dst_data);

    crypto.id = handle;
    crypto.operation = SYMC_OPERATION_ENCRYPT;
    crypto.last = TD_TRUE;
    crypto.length = byte_len;
    addr_via_const(crypto.input) = src_data;
    addr_via(crypto.output) = dst_data;

    ret = kapi_symc_crypto(&crypto);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_crypto, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief Performs decryption.
 */
td_s32 ot_mpi_cipher_decrypt_virt(td_handle handle, const td_u8 *src_data, td_u8 *dst_data, td_u32 byte_len)
{
    td_s32 ret;
    symc_encrypt_t crypto;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(src_data);
    chk_ptr_err_return(dst_data);

    crypto.id = handle;
    crypto.operation = SYMC_OPERATION_DECRYPT;
    crypto.last = TD_TRUE;
    crypto.length = byte_len;
    addr_via_const(crypto.input) = src_data;
    addr_via(crypto.output) = dst_data;

    ret = kapi_symc_crypto(&crypto);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_crypto, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief Encrypt multiple packaged data.
 */
td_s32 ot_mpi_cipher_encrypt_multi_pack(td_handle handle, const ot_cipher_data *data_pack, td_u32 data_pack_num)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(data_pack);
    chk_param_err_return(data_pack_num == 0x00);
    chk_param_err_return(data_pack_num >= CIPHER_MAX_MULTIPAD_NUM);

    ret = kapi_symc_crypto_multi(handle, data_pack, data_pack_num, SYMC_OPERATION_ENCRYPT, TD_TRUE);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_crypto_multi, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief Encrypt multiple packaged data.
 */
td_s32 ot_mpi_cipher_decrypt_multi_pack(td_handle handle, const ot_cipher_data *data_pack, td_u32 data_pack_num)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(data_pack);
    chk_param_err_return(data_pack_num == 0x00);
    chk_param_err_return(data_pack_num >= CIPHER_MAX_MULTIPAD_NUM);

    ret = kapi_symc_crypto_multi(handle, data_pack, data_pack_num, SYMC_OPERATION_DECRYPT, TD_TRUE);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_crypto_multi, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_get_tag(td_handle handle, td_u8 *tag, td_u32 tag_len)
{
    td_s32 ret;
    td_u8 real_tag[MAX_TAG_LEN] = {0};
    td_u32 real_tag_len = MAX_TAG_LEN;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(tag);

    ret = kapi_aead_get_tag(handle, (td_u32 *)real_tag, &real_tag_len);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_aead_get_tag, ret);
        return ret;
    }

    if (tag_len < real_tag_len) {
        log_error("Error: tag_len is less than the output tag len\n");
        (td_void)memset_s(real_tag, sizeof(real_tag), 0, sizeof(real_tag));
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    ret = memcpy_s(tag, tag_len, real_tag, real_tag_len);
    if (ret != EOK) {
        log_error("Error: memcpy_s for tag failed\n");
        (td_void)memset_s(real_tag, sizeof(real_tag), 0, sizeof(real_tag));
        return OT_ERR_CIPHER_FAILED_SEC_FUNC;
    }

    func_exit();

    (td_void)memset_s(real_tag, sizeof(real_tag), 0, sizeof(real_tag));
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_get_random_num(td_u32 *random_num)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(random_num);

    ret = kapi_trng_get_random(random_num, 0);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_trng_get_random, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_hash_init(const ot_cipher_hash_attr *hash_attr, td_handle *handle)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(hash_attr);
    chk_ptr_err_return(handle);

    ret = kapi_hash_start(handle, hash_attr->sha_type, hash_attr->hmac_key, hash_attr->hmac_key_len);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_start, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_hash_update(td_handle handle, const td_u8 *in_data, td_u32 in_data_len)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(in_data);

    ret = kapi_hash_update(handle, in_data, in_data_len, HASH_CHUNCK_SRC_LOCAL);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_update, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_hash_final(td_handle handle, td_u8 *out_hash, td_u32 out_hash_len)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(out_hash);
    chk_param_err_return(out_hash_len == 0);
    chk_param_err_return(handle == (td_handle)OT_INVALID_HANDLE);

    ret = kapi_hash_finish(handle, out_hash, out_hash_len, &out_hash_len);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_finish, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

static td_s32 mpi_rsa_chk_private_key(const ot_cipher_rsa_private_key *rsa_key)
{
    chk_ptr_err_return(rsa_key);
    chk_ptr_err_return(rsa_key->n);
    chk_param_err_return(rsa_key->n_len < RSA_MIN_KEY_LEN);
    chk_param_err_return(rsa_key->n_len > RSA_MAX_KEY_LEN);

    if (rsa_key->d == TD_NULL) {
        chk_ptr_err_return(rsa_key->p);
        chk_ptr_err_return(rsa_key->q);
        chk_ptr_err_return(rsa_key->dp);
        chk_ptr_err_return(rsa_key->dq);
        chk_ptr_err_return(rsa_key->qp);
        chk_param_err_return((rsa_key->n_len >> 1) != rsa_key->p_len);
        chk_param_err_return((rsa_key->n_len >> 1) != rsa_key->q_len);
        chk_param_err_return((rsa_key->n_len >> 1) != rsa_key->dp_len);
        chk_param_err_return((rsa_key->n_len >> 1) != rsa_key->dq_len);
        chk_param_err_return((rsa_key->n_len >> 1) != rsa_key->qp_len);
    } else {
        chk_param_err_return(rsa_key->n_len != rsa_key->d_len);
    }

    return TD_SUCCESS;
}

static td_void mpi_rsa_init_private_key(cryp_rsa_key *key, const ot_cipher_rsa_private_key *rsa_key)
{
    (td_void)memset_s(key, sizeof(cryp_rsa_key), 0, sizeof(cryp_rsa_key));
    key->public  = TD_FALSE;
    key->klen    = rsa_key->n_len;
    key->n       = rsa_key->n;
    key->d       = rsa_key->d;
    key->p       = rsa_key->p;
    key->q       = rsa_key->q;
    key->dp      = rsa_key->dp;
    key->dq      = rsa_key->dq;
    key->qp      = rsa_key->qp;
}

static td_s32 mpi_rsa_get_crypto_attr(ot_cipher_rsa_encrypt_scheme *rsa_scheme,
    ot_cipher_rsa_scheme scheme, ot_cipher_hash_type sha_type)
{
    if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V15) {
        *rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_PKCS1_V1_5;
    } else if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V21) {
        switch (sha_type) {
            case OT_CIPHER_HASH_TYPE_SHA256:
                *rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA256;
                break;
            case OT_CIPHER_HASH_TYPE_SHA384:
                *rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA384;
                break;
            case OT_CIPHER_HASH_TYPE_SHA512:
                *rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_RSAES_OAEP_SHA512;
                break;
            default:
                log_error("Invalid sha type %d\n", sha_type);
                return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else {
        log_error("Invalid scheme %d\n", scheme);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    return TD_SUCCESS;
}

static td_void cipher_rsa_get_hashlen(td_u32 *hash_len, ot_cipher_hash_type sha_type)
{
    switch (sha_type) {
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
            log_error("Invalid sha type %d\n", sha_type);
    }
}

static td_s32 cipher_rsa_pubkey_len_check(ot_cipher_rsa_scheme scheme, const ot_cipher_rsa_public_key *public_key,
    const ot_cipher_common_data *plain_txt, ot_cipher_hash_type sha_type)
{
    if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V15) {
        chk_param_err_return(plain_txt->data_len > public_key->n_len - 11);
    } else if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V21) {
        td_u32 hash_len = 0;
        cipher_rsa_get_hashlen(&hash_len, sha_type);
        /* the max len of plain txt is key_len - 2 * hash_len - 1 */
        chk_param_err_return(plain_txt->data_len > public_key->n_len - 2 * hash_len - 1);
    }
    return TD_SUCCESS;
}

static td_s32 cipher_rsa_prikey_len_check(ot_cipher_rsa_scheme scheme, const ot_cipher_rsa_private_key *private_key,
    const ot_cipher_common_data *plain_txt, ot_cipher_hash_type sha_type)
{
    if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V15) {
        chk_param_err_return(plain_txt->data_len > private_key->n_len - 11);
    } else if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V21) {
        td_u32 hash_len = 0;
        cipher_rsa_get_hashlen(&hash_len, sha_type);
        /* the max len of plain txt is key_len - 2 * hash_len - 1 */
        chk_param_err_return(plain_txt->data_len > private_key->n_len - 2 * hash_len - 1);
    }
    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_rsa_public_encrypt(ot_cipher_rsa_scheme scheme,
    ot_cipher_hash_type sha_type, const ot_cipher_rsa_public_key *rsa_key,
    const ot_cipher_common_data *plain_txt, ot_cipher_common_data *cipher_txt)
{
    td_s32 ret;
    cryp_rsa_key key;
    cryp_rsa_crypto_data rsa_data = {0};
    ot_cipher_rsa_encrypt_scheme rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_BUTT;

    mpi_chk_init_err_return();
    chk_ptr_err_return(rsa_key);
    chk_ptr_err_return(rsa_key->n);
    chk_ptr_err_return(rsa_key->e);
    chk_param_err_return((rsa_key->n_len < RSA_MIN_KEY_LEN) || (rsa_key->n_len > RSA_MAX_KEY_LEN));
    chk_param_err_return(rsa_key->n_len < rsa_key->e_len);

    chk_ptr_err_return(plain_txt);
    chk_ptr_err_return(plain_txt->data);
    chk_param_err_return(plain_txt->data_len == 0);

    chk_ptr_err_return(cipher_txt);
    chk_ptr_err_return(cipher_txt->data);
    chk_param_err_return((cipher_txt->data_len == 0) || (cipher_txt->data_len < rsa_key->n_len));

    rsa_data.in = plain_txt->data;
    rsa_data.in_len = plain_txt->data_len;
    rsa_data.out = cipher_txt->data;
    rsa_data.out_len = rsa_key->n_len;

    ret = mpi_rsa_get_crypto_attr(&rsa_scheme, scheme, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_get_crypto_attr, ret);
        return ret;
    }
    ret = cipher_rsa_pubkey_len_check(scheme, rsa_key, plain_txt, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(cipher_rsa_keylen_check, ret);
        return ret;
    }

    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    key.public = TD_TRUE;
    key.klen   = rsa_key->n_len;
    key.n      = rsa_key->n;

    cipher_get_pub_exponent(&key.e, rsa_key);

    ret = kapi_rsa_encrypt(rsa_scheme, &key, &rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_encrypt, ret);
    }
    cipher_txt->data_len = rsa_data.out_len;
    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    return ret;
}

td_s32 ot_mpi_cipher_rsa_private_decrypt(ot_cipher_rsa_scheme scheme,
    ot_cipher_hash_type sha_type, const ot_cipher_rsa_private_key *rsa_key,
    const ot_cipher_common_data *cipher_txt, ot_cipher_common_data *plain_txt)
{
    td_s32 ret;
    cryp_rsa_key key;
    cryp_rsa_crypto_data rsa_data = {0};
    ot_cipher_rsa_encrypt_scheme rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_BUTT;

    mpi_chk_init_err_return();
    ret = mpi_rsa_chk_private_key(rsa_key);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_chk_private_key, ret);
        return ret;
    }

    chk_ptr_err_return(plain_txt);
    chk_ptr_err_return(plain_txt->data);
    chk_param_err_return(plain_txt->data_len == 0);

    chk_ptr_err_return(cipher_txt);
    chk_ptr_err_return(cipher_txt->data);
    chk_param_err_return((cipher_txt->data_len == 0) || (cipher_txt->data_len != rsa_key->n_len));

    rsa_data.in = cipher_txt->data;
    rsa_data.in_len = cipher_txt->data_len;
    rsa_data.out = plain_txt->data;
    rsa_data.out_len = rsa_key->n_len;

    ret = mpi_rsa_get_crypto_attr(&rsa_scheme, scheme, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_get_crypto_attr, ret);
        return ret;
    }

    mpi_rsa_init_private_key(&key, rsa_key);

    ret = kapi_rsa_decrypt(rsa_scheme, &key, &rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_decrypt, ret);
    }
    plain_txt->data_len = rsa_data.out_len;
    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    return ret;
}

td_s32 ot_mpi_cipher_rsa_private_encrypt(ot_cipher_rsa_scheme scheme,
    ot_cipher_hash_type sha_type, const ot_cipher_rsa_private_key *rsa_key,
    const ot_cipher_common_data *plain_txt, ot_cipher_common_data *cipher_txt)
{
    td_s32 ret;
    cryp_rsa_key key;
    cryp_rsa_crypto_data rsa_data = {0};
    ot_cipher_rsa_encrypt_scheme rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_BUTT;

    mpi_chk_init_err_return();
    ret = mpi_rsa_chk_private_key(rsa_key);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_chk_private_key, ret);
        return ret;
    }

    chk_ptr_err_return(plain_txt);
    chk_ptr_err_return(plain_txt->data);
    chk_param_err_return(plain_txt->data_len == 0);

    chk_ptr_err_return(cipher_txt);
    chk_ptr_err_return(cipher_txt->data);
    chk_param_err_return((cipher_txt->data_len == 0) || (cipher_txt->data_len < rsa_key->n_len));

    rsa_data.in = plain_txt->data;
    rsa_data.in_len = plain_txt->data_len;
    rsa_data.out = cipher_txt->data;
    rsa_data.out_len = rsa_key->n_len;

    ret = mpi_rsa_get_crypto_attr(&rsa_scheme, scheme, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_get_crypto_attr, ret);
        return ret;
    }
    ret = cipher_rsa_prikey_len_check(scheme, rsa_key, plain_txt, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(cipher_rsa_keylen_check, ret);
        return ret;
    }

    mpi_rsa_init_private_key(&key, rsa_key);

    ret = kapi_rsa_encrypt(rsa_scheme, &key, &rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_encrypt, ret);
    }
    cipher_txt->data_len = rsa_data.out_len;
    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    return ret;
}

td_s32 ot_mpi_cipher_rsa_public_decrypt(ot_cipher_rsa_scheme scheme,
    ot_cipher_hash_type sha_type, const ot_cipher_rsa_public_key *rsa_key,
    const ot_cipher_common_data *cipher_txt, ot_cipher_common_data *plain_txt)
{
    td_s32 ret;
    cryp_rsa_key key;
    cryp_rsa_crypto_data rsa_data = {0};
    ot_cipher_rsa_encrypt_scheme rsa_scheme = OT_CIPHER_RSA_ENCRYPT_SCHEME_BUTT;

    mpi_chk_init_err_return();
    chk_ptr_err_return(rsa_key);
    chk_ptr_err_return(rsa_key->n);
    chk_ptr_err_return(rsa_key->e);
    chk_param_err_return((rsa_key->n_len < RSA_MIN_KEY_LEN) || (rsa_key->n_len > RSA_MAX_KEY_LEN));
    chk_param_err_return(rsa_key->n_len < rsa_key->e_len);

    chk_ptr_err_return(plain_txt);
    chk_ptr_err_return(plain_txt->data);
    chk_param_err_return(plain_txt->data_len == 0);

    chk_ptr_err_return(cipher_txt);
    chk_ptr_err_return(cipher_txt->data);
    chk_param_err_return((cipher_txt->data_len == 0) || (cipher_txt->data_len != rsa_key->n_len));

    rsa_data.in = cipher_txt->data;
    rsa_data.in_len = cipher_txt->data_len;
    rsa_data.out = plain_txt->data;
    rsa_data.out_len = rsa_key->n_len;

    ret = mpi_rsa_get_crypto_attr(&rsa_scheme, scheme, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_get_crypto_attr, ret);
        return ret;
    }

    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    key.public  = TD_TRUE;
    key.klen    = rsa_key->n_len;
    key.n       = rsa_key->n;
    cipher_get_pub_exponent(&key.e, rsa_key);

    ret = kapi_rsa_decrypt(rsa_scheme, &key, &rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_decrypt, ret);
    }
    plain_txt->data_len = rsa_data.out_len;
    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    return ret;
}

static td_s32 mpi_rsa_chk_sign_param(const ot_cipher_rsa_private_key *rsa_key,
    const ot_cipher_sign_in_data *rsa_data, const ot_cipher_common_data *sign_data)
{
    td_s32 ret;
    ret = mpi_rsa_chk_private_key(rsa_key);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_chk_private_key, ret);
        return ret;
    }

    /* check sign data */
    chk_ptr_err_return(rsa_data);
    chk_ptr_err_return(rsa_data->input);
    chk_param_err_return(rsa_data->sign_type >= OT_CIPHER_SIGN_TYPE_BUTT);
    chk_param_err_return(rsa_data->input_len == 0);

    chk_ptr_err_return(sign_data);
    chk_ptr_err_return(sign_data->data);
    chk_param_err_return((sign_data->data_len == 0) || (sign_data->data_len < rsa_key->n_len));

    return TD_SUCCESS;
}

static td_s32 mpi_rsa_get_sign_attr(ot_cipher_rsa_sign_scheme *rsa_scheme,
    td_u32 *hash_len, ot_cipher_rsa_scheme scheme, ot_cipher_hash_type sha_type)
{
    if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V15) {
        switch (sha_type) {
            case OT_CIPHER_HASH_TYPE_SHA256:
                *rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA256;
                *hash_len = SHA256_RESULT_SIZE;
                break;
            case OT_CIPHER_HASH_TYPE_SHA384:
                *rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA384;
                *hash_len = SHA384_RESULT_SIZE;
                break;
            case OT_CIPHER_HASH_TYPE_SHA512:
                *rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_V15_SHA512;
                *hash_len = SHA512_RESULT_SIZE;
                break;
            default:
                log_error("Invalid sha type %d\n", sha_type);
                return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else if (scheme == OT_CIPHER_RSA_SCHEME_PKCS1_V21) {
        switch (sha_type) {
            case OT_CIPHER_HASH_TYPE_SHA256:
                *rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA256;
                *hash_len = SHA256_RESULT_SIZE;
                break;
            case OT_CIPHER_HASH_TYPE_SHA384:
                *rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA384;
                *hash_len = SHA384_RESULT_SIZE;
                break;
            case OT_CIPHER_HASH_TYPE_SHA512:
                *rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_RSASSA_PKCS1_PSS_SHA512;
                *hash_len = SHA512_RESULT_SIZE;
                break;
            default:
                log_error("Invalid sha type %d\n", sha_type);
                return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else {
        log_error("Invalid scheme %d\n", scheme);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 mpi_cipher_hash(ot_cipher_hash_type sha_type,
    const td_u8 *in_data, td_u32 in_len, td_u8 *hash_data, td_u32 hash_len)
{
    td_s32 ret;
    td_handle hash_id;

    ret = kapi_hash_start(&hash_id, sha_type, TD_NULL, 0);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_init, ret);
        return ret;
    }

    ret = kapi_hash_update(hash_id, in_data, in_len, HASH_CHUNCK_SRC_LOCAL);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_update, ret);
        return ret;
    }

    ret = kapi_hash_finish(hash_id, hash_data, hash_len, &hash_len);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_finish, ret);
        return ret;
    }

    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_rsa_sign(ot_cipher_rsa_scheme scheme,
    ot_cipher_hash_type sha_type, const ot_cipher_rsa_private_key *rsa_key,
    const ot_cipher_sign_in_data *rsa_data, ot_cipher_common_data *sign_data)
{
    td_s32 ret;
    cryp_rsa_key key;
    td_u8 hash[HASH_RESULT_MAX_SIZE] = {0};
    cryp_rsa_sign_data sys_data = {0};
    ot_cipher_rsa_sign_scheme rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_BUTT;

    mpi_chk_init_err_return();
    ret = mpi_rsa_chk_sign_param(rsa_key, rsa_data, sign_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_chk_sign_param, ret);
        return ret;
    }

    (td_void)memset_s(&sys_data, sizeof(sys_data), 0, sizeof(sys_data));
    ret = mpi_rsa_get_sign_attr(&rsa_scheme, &sys_data.in_len, scheme, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_get_sign_attr, ret);
        return ret;
    }

    /* hash value of context, if OT_CIPHER_SIGN_TYPE_HASH, compute hash = Hash(in_data). */
    if (rsa_data->sign_type == OT_CIPHER_SIGN_TYPE_MSG) {
        ret = mpi_cipher_hash(sha_type, rsa_data->input, rsa_data->input_len, hash, sizeof(hash));
        if (ret != TD_SUCCESS) {
            print_func_errno(mpi_cipher_hash, ret);
            return ret;
        }
        sys_data.in = hash;
    } else if (rsa_data->sign_type == OT_CIPHER_SIGN_TYPE_HASH) {
        sys_data.in = rsa_data->input;
        if (rsa_data->input_len != sys_data.in_len) {
            log_error("Invalid input len %u\n", rsa_data->input_len);
            return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else {
        log_error("Invalid sign type %d\n", rsa_data->sign_type);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }
    sys_data.out = sign_data->data;
    sys_data.out_len = rsa_key->n_len;

    mpi_rsa_init_private_key(&key, rsa_key);

    ret = kapi_rsa_sign_hash(rsa_scheme, &key, &sys_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_sign_hash, ret);
    }
    sign_data->data_len = sys_data.out_len;
    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    return ret;
}

static td_s32 mpi_rsa_chk_verify_param(const ot_cipher_rsa_public_key *rsa_key,
    const ot_cipher_sign_in_data *rsa_data, const ot_cipher_common_data *sign_data)
{
    /* check public key */
    chk_ptr_err_return(rsa_key);
    chk_ptr_err_return(rsa_key->n);
    chk_ptr_err_return(rsa_key->e);
    chk_param_err_return((rsa_key->n_len < RSA_MIN_KEY_LEN) || (rsa_key->n_len > RSA_MAX_KEY_LEN));
    chk_param_err_return(rsa_key->n_len < rsa_key->e_len);

    /* check verify data */
    chk_ptr_err_return(rsa_data);
    chk_ptr_err_return(rsa_data->input);
    chk_param_err_return(rsa_data->sign_type >= OT_CIPHER_SIGN_TYPE_BUTT);
    chk_param_err_return(rsa_data->input_len == 0);

    chk_ptr_err_return(sign_data);
    chk_ptr_err_return(sign_data->data);
    chk_param_err_return((sign_data->data_len == 0) || (sign_data->data_len != rsa_key->n_len));

    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_rsa_verify(ot_cipher_rsa_scheme scheme,
    ot_cipher_hash_type sha_type, const ot_cipher_rsa_public_key *rsa_key,
    const ot_cipher_sign_in_data *rsa_data, const ot_cipher_common_data *sign_data)
{
    td_s32 ret;
    cryp_rsa_key key;
    td_u8 hash[HASH_RESULT_MAX_SIZE] = {0};
    cryp_rsa_sign_data sys_data;
    ot_cipher_rsa_sign_scheme rsa_scheme = OT_CIPHER_RSA_SIGN_SCHEME_BUTT;

    mpi_chk_init_err_return();
    ret = mpi_rsa_chk_verify_param(rsa_key, rsa_data, sign_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_chk_verify_param, ret);
        return ret;
    }

    (td_void)memset_s(&sys_data, sizeof(sys_data), 0, sizeof(sys_data));
    ret = mpi_rsa_get_sign_attr(&rsa_scheme, &sys_data.in_len, scheme, sha_type);
    if (ret != TD_SUCCESS) {
        print_func_errno(mpi_rsa_get_sign_attr, ret);
        return ret;
    }

    /* hash value of context, if OT_CIPHER_SIGN_TYPE_HASH, compute hash = Hash(in_data). */
    if (rsa_data->sign_type == OT_CIPHER_SIGN_TYPE_MSG) {
        ret = mpi_cipher_hash(sha_type, rsa_data->input, rsa_data->input_len, hash, sizeof(hash));
        if (ret != TD_SUCCESS) {
            print_func_errno(mpi_cipher_hash, ret);
            return ret;
        }
        sys_data.in = hash;
    } else if (rsa_data->sign_type == OT_CIPHER_SIGN_TYPE_HASH) {
        sys_data.in = rsa_data->input;
        if (rsa_data->input_len != sys_data.in_len) {
            log_error("Invalid input len %u\n", rsa_data->input_len);
            return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else {
        log_error("Invalid sign type %d\n", rsa_data->sign_type);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }
    sys_data.out = sign_data->data;
    sys_data.out_len = sign_data->data_len;

    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    key.public = TD_TRUE;
    key.klen   = rsa_key->n_len;
    key.n      = rsa_key->n;
    cipher_get_pub_exponent(&key.e, rsa_key);

    ret = kapi_rsa_verify_hash(rsa_scheme, &key, &sys_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_verify_hash, ret);
    }

    (td_void)memset_s(&key, sizeof(key), 0, sizeof(key));
    return ret;
}

td_s32 ot_mpi_cipher_sm2_sign(const ot_cipher_sm2_sign *sm2_sign,
    const ot_cipher_sign_in_data *sm2_data, ot_cipher_sm2_sign_data *sign_data)
{
#ifdef CHIP_SM2_SUPPORT
    td_s32 ret;
    sm2_sign_t sign;

    func_enter();

    chk_ptr_err_return(sm2_sign);
    chk_ptr_err_return(sm2_sign->id);
    chk_param_err_return(sm2_sign->id_len > SM2_ID_MAX_LEN);

    chk_ptr_err_return(sm2_data);
    chk_ptr_err_return(sm2_data->input);
    chk_param_err_return(sm2_data->sign_type != OT_CIPHER_SIGN_TYPE_MSG);
    chk_param_err_return(sm2_data->input_len == 0);

    chk_ptr_err_return(sign_data);

    (td_void)memset_s(&sign, sizeof(sm2_sign_t), 0, sizeof(sm2_sign_t));
    (td_void)memcpy_s(sign.d, SM2_LEN_IN_BYTE, sm2_sign->d, SM2_LEN_IN_BYTE);
    (td_void)memcpy_s(sign.px, SM2_LEN_IN_BYTE, sm2_sign->px, SM2_LEN_IN_BYTE);
    (td_void)memcpy_s(sign.py, SM2_LEN_IN_BYTE, sm2_sign->py, SM2_LEN_IN_BYTE);
    sign.msg = sm2_data->input;
    sign.msglen = sm2_data->input_len;

    ret = kapi_sm2_sign(&sign, sm2_sign->id, sm2_sign->id_len, HASH_CHUNCK_SRC_LOCAL);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_sm2_sign, ret);
    }
    (td_void)memcpy_s(sign_data->r, SM2_LEN_IN_BYTE, sign.r, SM2_LEN_IN_BYTE);
    (td_void)memcpy_s(sign_data->s, SM2_LEN_IN_BYTE, sign.s, SM2_LEN_IN_BYTE);

    func_exit();
    return ret;
#else
    crypto_unused(sm2_sign);
    crypto_unused(sm2_data);
    crypto_unused(sign_data);
    log_error("Unsupported SM2\n");
    return OT_ERR_CIPHER_UNSUPPORTED;
#endif
}

td_s32 ot_mpi_cipher_sm2_verify(const ot_cipher_sm2_verify *sm2_verify,
    const ot_cipher_sign_in_data *sm2_data, const ot_cipher_sm2_sign_data *sign_data)
{
#ifdef CHIP_SM2_SUPPORT
    td_s32 ret;
    sm2_verify_t verify;

    func_enter();

    chk_ptr_err_return(sm2_verify);
    chk_ptr_err_return(sm2_verify->id);
    chk_param_err_return(sm2_verify->id_len > SM2_ID_MAX_LEN);

    chk_ptr_err_return(sm2_data);
    chk_ptr_err_return(sm2_data->input);
    chk_param_err_return(sm2_data->sign_type != OT_CIPHER_SIGN_TYPE_MSG);
    chk_param_err_return(sm2_data->input_len == 0);

    chk_ptr_err_return(sign_data);

    (td_void)memset_s(&verify, sizeof(sm2_verify_t), 0, sizeof(sm2_verify_t));
    (td_void)memcpy_s(verify.px, SM2_LEN_IN_BYTE, sm2_verify->px, SM2_LEN_IN_BYTE);
    (td_void)memcpy_s(verify.py, SM2_LEN_IN_BYTE, sm2_verify->py, SM2_LEN_IN_BYTE);
    verify.msg = sm2_data->input;
    verify.msglen = sm2_data->input_len;
    (td_void)memcpy_s(verify.r, SM2_LEN_IN_BYTE, sign_data->r, SM2_LEN_IN_BYTE);
    (td_void)memcpy_s(verify.s, SM2_LEN_IN_BYTE, sign_data->s, SM2_LEN_IN_BYTE);
    ret = kapi_sm2_verify(&verify, sm2_verify->id, sm2_verify->id_len, HASH_CHUNCK_SRC_LOCAL);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_sm2_verify, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
#else
    crypto_unused(sm2_verify);
    crypto_unused(sm2_data);
    crypto_unused(sign_data);
    log_error("Unsupported SM2\n");
    return OT_ERR_CIPHER_UNSUPPORTED;
#endif
}

td_s32 ot_mpi_cipher_sm2_encrypt(const ot_cipher_sm2_public_key *sm2_key,
    const ot_cipher_common_data *plain_txt, ot_cipher_common_data *cipher_txt)
{
#ifdef CHIP_SM2_SUPPORT
    td_s32 ret;

    func_enter();

    chk_ptr_err_return(sm2_key);

    chk_ptr_err_return(plain_txt);
    chk_ptr_err_return(plain_txt->data);
    chk_param_err_return(plain_txt->data_len == 0);

    chk_ptr_err_return(cipher_txt);
    chk_ptr_err_return(cipher_txt->data);
    chk_param_err_return(cipher_txt->data_len == 0);

    ret = kapi_sm2_encrypt(sm2_key, plain_txt, cipher_txt);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_sm2_encrypt, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
#else
    crypto_unused(sm2_key);
    crypto_unused(plain_txt);
    crypto_unused(cipher_txt);
    log_error("Unsupported SM2\n");
    return OT_ERR_CIPHER_UNSUPPORTED;
#endif
}

td_s32 ot_mpi_cipher_sm2_decrypt(const ot_cipher_sm2_private_key *sm2_key,
    const ot_cipher_common_data *cipher_txt, ot_cipher_common_data *plain_txt)
{
#ifdef CHIP_SM2_SUPPORT
    td_s32 ret;

    func_enter();

    chk_ptr_err_return(sm2_key);

    chk_ptr_err_return(plain_txt);
    chk_ptr_err_return(plain_txt->data);
    chk_param_err_return(plain_txt->data_len == 0);

    chk_ptr_err_return(cipher_txt);
    chk_ptr_err_return(cipher_txt->data);
    chk_param_err_return(cipher_txt->data_len == 0);

    ret = kapi_sm2_decrypt(sm2_key, cipher_txt, plain_txt);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_sm2_decrypt, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
#else
    crypto_unused(sm2_key);
    crypto_unused(plain_txt);
    crypto_unused(cipher_txt);
    log_error("Unsupported SM2\n");
    return OT_ERR_CIPHER_UNSUPPORTED;
#endif
}

td_s32 ot_mpi_keyslot_create(const ot_keyslot_attr *attr, td_handle *keyslot)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    chk_ptr_err_return(attr);
    chk_ptr_err_return(keyslot);

    ret = kapi_keyslot_create(attr, keyslot);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_keyslot_create, ret);
        return ret;
    }

    func_exit();

    return TD_SUCCESS;
}

td_s32 ot_mpi_keyslot_destroy(td_handle keyslot)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    ret = kapi_keyslot_destroy(keyslot);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_keyslot_destroy, ret);
        return ret;
    }

    func_exit();

    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_attach(td_handle cipher, td_handle keyslot)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    ret = kapi_symc_attach(cipher, keyslot);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_attach, ret);
        return ret;
    }

    func_exit();

    return TD_SUCCESS;
}

td_s32 ot_mpi_cipher_detach(td_handle cipher, td_handle keyslot)
{
    td_s32 ret;

    mpi_chk_init_err_return();
    func_enter();

    ret = kapi_symc_detach(cipher, keyslot);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_detach, ret);
        return ret;
    }

    func_exit();

    return TD_SUCCESS;
}

/** @} */ /** <!-- ==== Compat Code end ==== */
