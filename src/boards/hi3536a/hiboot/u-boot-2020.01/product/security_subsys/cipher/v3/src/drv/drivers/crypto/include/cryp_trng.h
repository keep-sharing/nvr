// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef CRYP_TRNG_H
#define CRYP_TRNG_H

#include "drv_osal_lib.h"
#include "drv_trng.h"

/******************************* API Declaration *****************************/
/** \addtogroup      trng */
/** @{ */ /** <!--[trng] */

/** @} */ /** <!-- ==== Structure Definition end ==== */

/******************************* API Code *****************************/
/** \addtogroup      trng drivers */
/** @{ */ /** <!-- [trng] */

td_s32 cryp_trng_init(void);
td_s32 cryp_trng_deinit(void);

/**
\brief get rand number.
\param[out]  randnum rand number.
\param[in]   timeout time out.
\retval     On success, TD_SUCCESS is returned.  On error, TD_FAILURE is returned.
 */
td_s32 cryp_trng_get_random(td_u32 *randnum, td_u32 timeout);

/**
\brief get rand bytes.
\param[out]  randnum rand number.
\param[in]   size size of rand number.
\param[in]   timeout time out.
\retval     On success, TD_SUCCESS is returned.  On error, TD_FAILURE is returned.
 */
td_s32 cryp_trng_get_random_bytes(td_u8 *randbyte, td_u32 size, td_u32 timeout);

/** @} */ /** <!-- ==== API declaration end ==== */

#endif /* CRYP_TRNG_H */
