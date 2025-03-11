// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_ks.h"
#include "drv_osal_lib.h"

#define KC_TEECPU_LOCK_CMD      0xB00
#define KC_REECPU_LOCK_CMD      0xB04
#define KC_TEECPU_FLUSH_BUSY    0xB10
#define KC_REECPU_FLUSH_BUSY    0xB14
#define KC_SEND_DBG             0xB20
#define KC_ROB_ALARM            0xB24
#define KC_RD_SLOT_NUM          0xB30
#define KC_RD_LOCK_STATUS       0xB34

static volatile td_u8 *g_cipher_kc_base = TD_NULL;
#define hal_kc_read(offset)             crypto_read(g_cipher_kc_base + (offset))
#define hal_kc_write(offset, data)      crypto_write(g_cipher_kc_base + (offset), data)

typedef enum {
    OT_KC_SLOT_STAT_UNLOCK     = 0x0,
    OT_KC_SLOT_STAT_REE_LOCK   = 0x1,
    OT_KC_SLOT_STAT_TEE_LOCK   = 0x2,
    OT_KC_SLOT_STAT_MAX,
} ot_kc_slot_stat_e;

typedef union {
    struct {
        td_u32 tee_key_slot_num : 10;        /* [0:9] */
        td_u32 reserved0 : 10;               /* [10:19] */
        td_u32 tee_lock_cmd : 1;             /* [20] */
        td_u32 reserved1 : 11;               /* [21:31] */
    } bits;
    td_u32 u32;
} kc_teecpu_lock_cmd;

typedef union {
    struct {
        td_u32 ree_key_slot_num : 10;        /* [0:9] */
        td_u32 reserved0 : 10;               /* [10:19] */
        td_u32 ree_lock_cmd : 1;             /* [20] */
        td_u32 reserved1 : 11;               /* [21:31] */
    } bits;
    td_u32 u32;
} kc_reecpu_lock_cmd;

typedef union {
    struct {
        td_u32 teecpu_flush_busy : 1;        /* [0] */
        td_u32 teecpu_flush_fail : 1;        /* [1] */
        td_u32 reserved0 : 30;               /* [2:31] */
    } bits;
    td_u32 u32;
} kc_teecpu_flush_busy;

typedef union {
    struct {
        td_u32 reecpu_flush_busy : 1;        /* [0] */
        td_u32 reecpu_flush_fail : 1;        /* [1] */
        td_u32 reserved0 : 30;               /* [2:31] */
    } bits;
    td_u32 u32;
} kc_reecpu_flush_busy;

typedef union {
    struct {
        td_u32 slot_num_cfg : 8;             /* [0:7] */
        td_u32 reserved0 : 24;               /* [8:31] */
    } bits;
    td_u32 u32;
} kc_rd_slot_num;

typedef union {
    struct {
        td_u32 rd_lock_status : 2;           /* [0:1] */
        td_u32 reserved0 : 30;               /* [2:31] */
    } bits;
    td_u32 u32;
} kc_rd_lock_status;

td_s32 drv_kc_slot_init(td_void)
{
    g_cipher_kc_base = crypto_ioremap_nocache(KC_REG_BASE_ADDR_PHY, KC_REG_BASE_ADDR_SIZE);
    if (g_cipher_kc_base == TD_NULL) {
        log_error("ERROR: kc ioremap with nocache failed!!\n");
        return OT_ERR_CIPHER_FAILED_MEM;
    }
    return TD_SUCCESS;
}

td_void drv_kc_slot_deinit(td_void)
{
    if (g_cipher_kc_base != TD_NULL) {
        crypto_iounmap(g_cipher_kc_base, KC_REG_BASE_ADDR_SIZE);
        g_cipher_kc_base = TD_NULL;
    }
}

static td_s32 hal_kc_slot_status(td_u32 slot, ot_kc_slot_stat_e expect)
{
    kc_rd_slot_num slot_num;
    kc_rd_lock_status status;

    slot_num.u32 = 0;
    slot_num.bits.slot_num_cfg = slot;
    hal_kc_write(KC_RD_SLOT_NUM, slot_num.u32);

    status.u32 = hal_kc_read(KC_RD_LOCK_STATUS);
    if (status.bits.rd_lock_status == expect) {
        return TD_SUCCESS;
    } else {
        return OT_ERR_CIPHER_HARD_STATUS;
    }
}

static td_s32 hal_kc_slot_lock(td_u32 slot)
{
    td_s32 ret;

    if (module_get_secure() == TD_TRUE) {
        kc_teecpu_lock_cmd cmd;
        cmd.u32 = 0;
        cmd.bits.tee_lock_cmd = 1;
        cmd.bits.tee_key_slot_num = slot;
        hal_kc_write(KC_TEECPU_LOCK_CMD, cmd.u32);
        ret = hal_kc_slot_status(slot, OT_KC_SLOT_STAT_TEE_LOCK);
        if (ret != TD_SUCCESS) {
            log_error("tee kc slot lock fail\n");
            return ret;
        }
    } else {
        kc_reecpu_lock_cmd cmd;
        cmd.u32 = 0;
        cmd.bits.ree_lock_cmd = 1;
        cmd.bits.ree_key_slot_num = slot;
        hal_kc_write(KC_REECPU_LOCK_CMD, cmd.u32);
        ret = hal_kc_slot_status(slot, OT_KC_SLOT_STAT_REE_LOCK);
        if (ret != TD_SUCCESS) {
            log_error("ree kc slot lock fail\n");
            return ret;
        }
    }
    return ret;
}

static td_s32 hal_kc_slot_unlock(td_u32 slot)
{
    td_s32 ret;

    if (module_get_secure() == TD_TRUE) {
        kc_teecpu_lock_cmd cmd;
        kc_teecpu_flush_busy busy;

        ret = hal_kc_slot_status(slot, OT_KC_SLOT_STAT_TEE_LOCK);
        if (ret != TD_SUCCESS) {
            log_error("tee slot status fail\n");
            return ret;
        }

        cmd.u32 = 0;
        cmd.bits.tee_lock_cmd = 0;
        cmd.bits.tee_key_slot_num = slot;
        hal_kc_write(KC_TEECPU_LOCK_CMD, cmd.u32);

        busy.u32 = hal_kc_read(KC_TEECPU_FLUSH_BUSY);
        if (busy.bits.teecpu_flush_busy != 0 && busy.bits.teecpu_flush_fail != 0) {
            log_error("clean flush fail\n");
            return OT_ERR_CIPHER_HARD_STATUS;
        }

        ret = hal_kc_slot_status(slot, OT_KC_SLOT_STAT_UNLOCK);
        if (ret != TD_SUCCESS) {
            log_error("tee slot status fail\n");
            return ret;
        }
    } else {
        kc_reecpu_lock_cmd cmd;
        kc_reecpu_flush_busy busy;

        ret = hal_kc_slot_status(slot, OT_KC_SLOT_STAT_REE_LOCK);
        if (ret != TD_SUCCESS) {
            log_error("ree slot status fail\n");
            return ret;
        }

        cmd.u32 = 0;
        cmd.bits.ree_lock_cmd = 0;
        cmd.bits.ree_key_slot_num = slot;
        hal_kc_write(KC_REECPU_LOCK_CMD, cmd.u32);

        busy.u32 = hal_kc_read(KC_REECPU_FLUSH_BUSY);
        if (busy.bits.reecpu_flush_busy != 0 && busy.bits.reecpu_flush_fail != 0) {
            log_error("clean flush fail\n");
            return OT_ERR_CIPHER_HARD_STATUS;
        }

        ret = hal_kc_slot_status(slot, OT_KC_SLOT_STAT_UNLOCK);
        if (ret != TD_SUCCESS) {
            log_error("ree slot status fail\n");
            return ret;
        }
    }
    return ret;
}

td_s32 drv_kc_slot_lock(const ot_keyslot_attr *attr, td_u32 *slot_num)
{
    td_s32 ret;
    td_u32 i, max_slot;

    if (attr->type == OT_KEYSLOT_TYPE_MCIPHER) {
        max_slot = KEYSLOT_CHANNEL_MAX;
    } else {
        log_error("Invalid keyslot type!\n");
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    chk_param_err_return(attr->secure_mode >= OT_KEYSLOT_SECURE_MODE_BUTT);
    if (!crypto_is_sec_cpu() && attr->secure_mode != OT_KEYSLOT_SECURE_MODE_NONE) {
        log_error("Invalid secure mode!\n");
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    for (i = 0; i < max_slot; i++) {
        ret = hal_kc_slot_status(i, OT_KC_SLOT_STAT_UNLOCK);
        if (ret != TD_SUCCESS) {
            continue;
        }
        ret = hal_kc_slot_lock(i);
        if (ret != TD_SUCCESS) {
            continue;
        }
        *slot_num = i;
        return TD_SUCCESS;
    }

    log_error("all keyslot channel is busy!\n");
    return OT_ERR_CIPHER_HARD_STATUS;
}

td_s32 drv_kc_slot_unlock(td_u32 slot_num)
{
    return hal_kc_slot_unlock(slot_num);
}
