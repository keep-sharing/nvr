// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_osal_lib.h"
#include "cryp_rsa.h"

#if (defined(CHIP_RSA_SUPPORT) || defined(SOFT_RSA_SUPPORT))
td_s32 kapi_rsa_init(td_void)
{
    td_s32 ret;

    func_enter();

    ret = cryp_rsa_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_rsa_init, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/*
 * brief   Kapi Deinitialize.
 * retval     On success, TD_SUCCESS is returned.  On error, TD_FAILURE is returned.
 */
td_s32 kapi_rsa_deinit(td_void)
{
    func_enter();

    cryp_rsa_deinit();

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_rsa_encrypt(ot_cipher_rsa_encrypt_scheme scheme,
    const cryp_rsa_key *key, cryp_rsa_crypto_data *rsa_data)
{
    td_s32 ret;

    func_enter();

    ret = cryp_rsa_encrypt(scheme, key, rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_rsa_encrypt, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_rsa_decrypt(ot_cipher_rsa_encrypt_scheme scheme,
    const cryp_rsa_key *key, cryp_rsa_crypto_data *rsa_data)
{
    td_s32 ret;

    func_enter();

    ret = cryp_rsa_decrypt(scheme, key, rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_rsa_decrypt, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_rsa_sign_hash(ot_cipher_rsa_sign_scheme scheme,
    const cryp_rsa_key *key, cryp_rsa_sign_data *rsa_data)
{
    td_s32 ret;

    func_enter();

    ret = cryp_rsa_sign_hash(scheme, key, rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_rsa_sign_hash, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_rsa_verify_hash(ot_cipher_rsa_sign_scheme scheme,
    const cryp_rsa_key *key, const cryp_rsa_sign_data *rsa_data)
{
    td_s32 ret;

    func_enter();

    ret = cryp_rsa_verify_hash(scheme, key, rsa_data);
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_rsa_verify_hash, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}
#endif
