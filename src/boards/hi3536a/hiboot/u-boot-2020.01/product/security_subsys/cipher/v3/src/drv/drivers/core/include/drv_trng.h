// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_TRNG_H
#define DRV_TRNG_H

/*************************** Structure Definition ****************************/
/** \addtogroup     rsa */
/** @{ */ /** <!-- [rsa] */

/* ! \rsa capacity, 0-nonsupport, 1-support */
typedef struct {
    td_u32 trng : 1; /* !<  Support TRNG */
} trng_capacity;

/** @} */ /** <!-- ==== Structure Definition end ==== */

/******************************* API Declaration *****************************/
/** \addtogroup      trng */
/** @{ */ /** <!--[trng] */

td_s32 drv_trng_init(void);
td_s32 drv_trng_deinit(void);

/**
\brief get rand number.
\param[out]  randnum rand number.
\param[in]   timeout time out.
\retval     On success, TD_SUCCESS is returned.  On error, TD_FAILURE is returned.
 */
td_s32 drv_trng_randnum(td_u32 *randnum, td_u32 timeout);

/**
\brief  get the trng capacity.
\param[out] capacity The hash capacity.
\retval     NA.
 */
td_void drv_trng_get_capacity(trng_capacity *capacity);

/** @} */ /** <!-- ==== API declaration end ==== */

#endif /* DRV_TRNG_H */
