// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_osal_lib.h"
#include "cryp_trng.h"

td_s32 kapi_trng_init(void)
{
    td_s32 ret;

    func_enter();

    ret = cryp_trng_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_trng_init, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_trng_deinit(void)
{
    td_s32 ret;

    func_enter();

    ret = cryp_trng_deinit();
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_trng_deinit, ret);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_trng_get_random(td_u32 *randnum, td_u32 timeout)
{
    td_s32 ret;

    func_enter();

    chk_ptr_err_return(randnum);

    ret = cryp_trng_get_random(randnum, timeout);
    if (ret != TD_SUCCESS) {
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}
