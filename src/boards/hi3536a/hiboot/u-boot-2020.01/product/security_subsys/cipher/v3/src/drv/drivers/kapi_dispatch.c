// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_osal_lib.h"

td_s32 crypto_entry(td_void)
{
    td_s32 ret;

    crypto_mem_init();

    ret = module_addr_map();
    if (ret != TD_SUCCESS) {
        print_func_errno(module_addr_map, ret);
        return ret;
    }

    ret = kapi_trng_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_trng_init, ret);
        goto error;
    }

    ret = kapi_symc_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_init, ret);
        goto error1;
    }

    ret = kapi_hash_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_init, ret);
        goto error2;
    }

#if (defined(CHIP_RSA_SUPPORT) || defined(SOFT_RSA_SUPPORT))
    ret = kapi_rsa_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_init, ret);
        kapi_hash_deinit();
        goto error2;
    }
#endif

#ifdef CHIP_SM2_SUPPORT
    ret = kapi_sm2_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_sm2_init, ret);
        kapi_rsa_deinit();
        kapi_hash_deinit();
        goto error2;
    }
#endif
    return TD_SUCCESS;

error2:
    kapi_symc_deinit();
error1:
    kapi_trng_deinit();
error:
    module_addr_unmap();
    return ret;
}

td_s32 crypto_exit(td_void)
{
    td_s32 ret;

    func_enter();

    ret = kapi_symc_deinit();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_deinit, ret);
        return ret;
    }

    ret = kapi_hash_deinit();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_deinit, ret);
        return ret;
    }

    ret = kapi_rsa_deinit();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_rsa_deinit, ret);
        return ret;
    }

#ifdef CHIP_SM2_SUPPORT
    ret = kapi_sm2_deinit();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_sm2_deinit, ret);
        return ret;
    }
#endif

    ret = kapi_trng_deinit();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_trng_deinit, ret);
        return ret;
    }

    ret = module_addr_unmap();
    if (ret != TD_SUCCESS) {
        print_func_errno(module_addr_unmap, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

/** @} */ /** <!-- ==== Structure Definition end ==== */

