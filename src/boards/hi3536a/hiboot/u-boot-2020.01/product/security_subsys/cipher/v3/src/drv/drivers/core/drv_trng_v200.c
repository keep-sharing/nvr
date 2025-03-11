// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_trng_v200.h"
#include "drv_trng.h"

#ifdef CHIP_TRNG_VER_V200

/*************************** Internal Structure Definition ****************************/
/** \addtogroup      cipher drivers */
/** @{ */ /** <!-- [cipher] */

/* ! Define the osc sel */
#define TRNG_OSC_SEL   0x02
#define TRNG_POWER_ON  0x05
#define TRNG_POWER_OFF 0x0a

/** @} */ /** <!-- ==== Structure Definition end ==== */

/******************************* API Declaration *****************************/
/** \addtogroup      trng */
/** @{ */ /** <!--[trng] */

td_s32 drv_trng_init(void)
{
    ot_sec_com_trng_ctrl ctrl;
    static td_u32 last = 0x0A;

    func_enter();

    if (module_get_secure() != TD_TRUE) {
        return TD_SUCCESS;
    }
#ifdef TRNG_POWER_CTR_SUPPORT
    {
        ot_sec_com_trng_power_st power;

        /* power reduce enable */
        power.u32 = trng_read(OT_SEC_COM_TRNG_POWER_EN);
        if (power.bits.trng_power_en != TRNG_POWER_ON) {
            power.bits.trng_power_en = TRNG_POWER_ON;
            trng_write(OT_SEC_COM_TRNG_POWER_EN, power.u32);
        }
    }
#endif

    ctrl.u32 = trng_read(OT_SEC_COM_TRNG_CTRL);
    if (ctrl.u32 != last) {
        module_enable(CRYPTO_MODULE_ID_TRNG);

        ctrl.bits.mix_enable = 0x00;
        ctrl.bits.drop_enable = 0x00;
        ctrl.bits.pre_process_enable = 0x00;
        ctrl.bits.post_process_enable = 0x00;
        ctrl.bits.post_process_depth = 0x00;
        ctrl.bits.drbg_enable = 0x01;
        ctrl.bits.osc_sel = TRNG_OSC_SEL;
        trng_write(OT_SEC_COM_TRNG_CTRL, ctrl.u32);
        last = ctrl.u32;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 drv_trng_deinit(void)
{
    func_enter();

    func_exit();
    return TD_SUCCESS;
}

td_s32 drv_trng_randnum(td_u32 *randnum, td_u32 timeout)
{
    ot_sec_com_trng_data_st stat;
    td_u32 times = 0;

    func_enter();

    chk_ptr_err_return(randnum);

    if (module_get_secure() != TD_TRUE) {
        /* read valid randnum */
        *randnum = trng_read(OT_SEC_COM_TRNG_FIFO_DATA);
        log_dbg("randnum: 0x%x\n", *randnum);
        return TD_SUCCESS;
    }

    if (timeout == 0) { /* unblock */
        /* trng number is valid ? */
        stat.u32 = trng_read(OT_SEC_COM_TRNG_DATA_ST);
        if (stat.bits.trng_fifo_data_cnt == 0) {
            return OT_ERR_CIPHER_NO_AVAILABLE_RNG;
        }
    } else { /* block */
        while (times++ < timeout) {
            /* trng number is valid ? */
            stat.u32 = trng_read(OT_SEC_COM_TRNG_DATA_ST);
            if (stat.bits.trng_fifo_data_cnt > 0) {
                break;
            }
        }

        /* time out */
        if (times >= timeout) {
            return OT_ERR_CIPHER_NO_AVAILABLE_RNG;
        }
    }

    /* read valid randnum */
    *randnum = trng_read(OT_SEC_COM_TRNG_FIFO_DATA);
    log_dbg("randnum: 0x%x\n", *randnum);

    func_exit();
    return TD_SUCCESS;
}

/** @} */ /** <!-- ==== API declaration end ==== */

#endif  // End of CHIP_TRNG_VER_V200
