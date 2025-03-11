// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include "drv_osal_lib.h"
#include "cryp_symc.h"
#include "drv_ks.h"
#include "cipher_ext.h"

/* max number of nodes */
#define MAX_PKG_NUMBER                  100000

/* max length of CCM/GCM AAD */
#define MAX_AEAD_A_LEN                  0x100000

typedef struct {
    td_u32 open : 1;                    /* open or close */
    td_u32 config : 1;                  /* aleardy config or not */
    td_u32 attach : 1;                  /* aleardy config or not */
    symc_func *func;
    td_void *cryp_ctx;                  /* Context of cryp instance */
    CRYPTO_OWNER owner;                 /* user ID */
    ot_cipher_ctrl ctrl;                /* control information */
    td_u32 key_slot;
} kapi_symc_ctx;

/*! Context of cipher */
static kapi_symc_ctx *g_symc_ctx = TD_NULL;

/* keyslot open or close */
static td_bool g_keyslot_open[KEYSLOT_CHANNEL_MAX];

/* symc mutex */
static CRYPTO_MUTEX g_symc_mutex;

#define kapi_symc_lock_err_return()                         \
    do {                                                    \
        ret = crypto_mutex_lock(&g_symc_mutex);             \
        if (ret != TD_SUCCESS) {                            \
            log_error("error, symc lock failed\n");      \
            print_func_errno(crypto_mutex_lock, ret);  \
            return ret;                                     \
        }                                                   \
    } while (0)

#define kapi_symc_unlock()          crypto_mutex_unlock(&g_symc_mutex)
#define AES_CCM_MIN_TAG_LEN         4
#define AES_CCM_MAX_TAG_LEN         16
#define AES_GCM_MIN_TAG_LEN         1
#define AES_GCM_MAX_TAG_LEN         16

/* ****************************** API Code **************************** */
static td_s32 kapi_symc_chk_handle(td_handle handle)
{
    chk_handle_modid(handle, OT_ID_CIPHER);
    chk_handle_private_data(handle, OT_PRIVATE_ID_SYMC);
    chk_handle_chnid(handle, CRYPTO_HARD_CHANNEL_MAX);

    if (g_symc_ctx[td_handle_get_chnid(handle)].open == TD_FALSE) {
        log_error("channel %u is not open\n", td_handle_get_chnid(handle));
        return OT_ERR_CIPHER_INVALID_HANDLE;
    }

    return TD_SUCCESS;
}

td_s32 kapi_symc_init(void)
{
    td_s32 ret;

    func_enter();

    crypto_mutex_init(&g_symc_mutex);

    g_symc_ctx = crypto_calloc(sizeof(kapi_symc_ctx), CRYPTO_HARD_CHANNEL_MAX);
    if (g_symc_ctx == TD_NULL) {
        print_func_errno(crypto_calloc, OT_ERR_CIPHER_FAILED_MEM);
        return OT_ERR_CIPHER_FAILED_MEM;
    }

    ret = cryp_symc_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_symc_init, ret);
        crypto_free(g_symc_ctx);
        return ret;
    }

    ret = drv_kc_slot_init();
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_hash_init, ret);
        cryp_symc_deinit();
        crypto_free(g_symc_ctx);
        return ret;
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_symc_deinit(void)
{
    func_enter();

    drv_kc_slot_deinit();

    cryp_symc_deinit();

    crypto_mutex_destroy(&g_symc_mutex);
    crypto_free(g_symc_ctx);

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_symc_release(void)
{
    td_u32 i, chn;
    td_s32 ret;
    kapi_symc_ctx *ctx = TD_NULL;
    CRYPTO_OWNER owner;

    func_enter();

    crypto_get_owner(&owner);

    /* destroy the channel which are created by current user */
    for (i = 0; i < CRYPTO_HARD_CHANNEL_MAX; i++) {
        ctx = &g_symc_ctx[i];
        if (ctx->open == TD_FALSE) {
            continue;
        }

        if (memcmp(&owner, &ctx->owner, sizeof(owner)) == 0) {
            chn = td_handle_init(OT_ID_CIPHER, OT_PRIVATE_ID_SYMC, i);
            log_dbg("symc release chn %u\n", chn);
            ret = kapi_symc_destroy(chn);
            if (ret != TD_SUCCESS) {
                print_func_errno(kapi_symc_destroy, ret);
                return ret;
            }
        }
    }

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_symc_create(td_u32 *id, ot_cipher_type type)
{
    td_s32 ret;
    td_u32 chn = 0;
    kapi_symc_ctx *ctx = TD_NULL;

    func_enter();

    chk_ptr_err_return(id);

    kapi_symc_lock_err_return();

    /* allocate a aes soft channel for hard channel allocated */
    ret = cryp_symc_alloc_chn(&chn, type);
    if (ret != TD_SUCCESS) {
        print_func_errno(cryp_symc_alloc_chn, ret);
        kapi_symc_unlock();
        return ret;
    }
    ctx = &g_symc_ctx[chn];

    (td_void)memset_s(ctx, sizeof(kapi_symc_ctx), 0, sizeof(kapi_symc_ctx));
    crypto_get_owner(&ctx->owner);

    *id = td_handle_init(OT_ID_CIPHER, OT_PRIVATE_ID_SYMC, chn);
    ctx->open = TD_TRUE;
    ctx->config = TD_FALSE;
    ctx->attach = TD_FALSE;

    log_dbg("[SYMC] create handle 0x%x, owner 0x%x\n", *id, ctx->owner);

    kapi_symc_unlock();

    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_symc_destroy(td_u32 id)
{
    td_s32 ret;
    kapi_symc_ctx *ctx = TD_NULL;
    td_u32 soft_id;

    func_enter();

    ret = kapi_symc_chk_handle((td_handle)id);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    soft_id = td_handle_get_chnid(id);
    ctx = &g_symc_ctx[soft_id];
    crypto_chk_owner_err_return(&ctx->owner);
    if (ctx->ctrl.alg != OT_CIPHER_ALG_DMA) {
        chk_param_err_return(ctx->attach == TD_TRUE);
    }

    kapi_symc_lock_err_return();

    cryp_symc_free_chn(soft_id);

    /* Destroy the attached instance of Symmetric cipher engine */
    if ((ctx->func != TD_NULL) && (ctx->func->destroy != TD_NULL)) {
        ret = ctx->func->destroy(ctx->cryp_ctx);
        if (ret != TD_SUCCESS) {
            print_func_errno(ctx->func->destroy, ret);
            kapi_symc_unlock();
            return ret;
        }
        ctx->cryp_ctx = TD_NULL;
    }

    ctx->open = TD_FALSE;

    log_dbg("[SYMC] destroy handle 0x%x, owner 0x%x, keyslot 0x%x\n", id, ctx->owner, ctx->key_slot);

    kapi_symc_unlock();

    func_exit();
    return TD_SUCCESS;
}

static td_s32 kapi_symc_chk_aes_param(ot_cipher_alg alg, ot_cipher_work_mode mode, td_u32 width)
{
    if (mode > OT_CIPHER_WORK_MODE_BUTT) {
        log_error("Invalid alg %d and mode %d\n", alg, mode);
        print_errno(OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    if ((mode == OT_CIPHER_WORK_MODE_CFB) && (width != SYMC_DAT_WIDTH_1) &&
        (width != SYMC_DAT_WIDTH_8) && (width != SYMC_DAT_WIDTH_128)) {
        log_error("Invalid alg %d mode %d and width %u\n", alg, mode, width);
        print_errno(OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    if ((mode == OT_CIPHER_WORK_MODE_OFB) && (width != SYMC_DAT_WIDTH_128)) {
        log_error("Invalid alg %d mode %d and width %u\n", alg, mode, width);
        print_errno(OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    return TD_SUCCESS;
}

#ifdef CHIP_SM4_SUPPORT
static td_s32 kapi_symc_chk_sm4_param(ot_cipher_alg alg, ot_cipher_work_mode mode)
{
    if ((mode != OT_CIPHER_WORK_MODE_ECB) &&
        (mode != OT_CIPHER_WORK_MODE_CBC) && (mode != OT_CIPHER_WORK_MODE_CTR)) {
        log_error("Invalid alg %d and mode %d\n", alg, mode);
        print_errno(OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    return TD_SUCCESS;
}
#endif

static td_s32 kapi_symc_width_check(const symc_cfg_t *cfg, td_u32 width)
{
    td_s32 ret;

    /* the bit width depend on alg and mode, which limit to hardware
     * aes with cfb/ofb only support bit128.
     * sm1/sm4 with ofb only support bit128, cfb support bit1, bit8, bit 128.
     */
    func_enter();

    chk_param_err_return(cfg->alg >= OT_CIPHER_ALG_BUTT);
    chk_param_err_return((cfg->alg != OT_CIPHER_ALG_DMA) && (cfg->mode >= OT_CIPHER_WORK_MODE_BUTT));
    chk_param_err_return((cfg->alg != OT_CIPHER_ALG_DMA) && (width >= SYMC_DAT_WIDTH_COUNT));

    if (cfg->alg == OT_CIPHER_ALG_AES) {
        ret = kapi_symc_chk_aes_param(cfg->alg, cfg->mode, width);
        if (ret != TD_SUCCESS) {
            print_func_errno(kapi_symc_chk_aes_param, ret);
            return ret;
        }
#ifdef CHIP_SM4_SUPPORT
    } else if (cfg->alg == OT_CIPHER_ALG_SM4) {
        ret = kapi_symc_chk_sm4_param(cfg->alg, cfg->mode);
        if (ret != TD_SUCCESS) {
            print_func_errno(kapi_symc_chk_sm4_param, ret);
            return ret;
        }
#endif
    }

    func_exit();
    return TD_SUCCESS;
}

static td_s32 kapi_symc_match_width(ot_cipher_work_mode work_mode, ot_cipher_bit_width bit_width, symc_width *width)
{
    func_enter();

    /* set the bit width which depend on alg and mode */
    if (work_mode == OT_CIPHER_WORK_MODE_CFB) {
        switch (bit_width) {
            case OT_CIPHER_BIT_WIDTH_128BIT:
                *width = SYMC_DAT_WIDTH_128;
                break;
            case OT_CIPHER_BIT_WIDTH_8BIT:
                *width = SYMC_DAT_WIDTH_8;
                break;
            case OT_CIPHER_BIT_WIDTH_1BIT:
                *width = SYMC_DAT_WIDTH_1;
                break;
            default:
                log_error("Invalid width: 0x%x, mode 0x%x\n", bit_width, work_mode);
                print_errno(OT_ERR_CIPHER_INVALID_PARAM);
                return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else {
        if (bit_width == OT_CIPHER_BIT_WIDTH_128BIT) {
            *width = SYMC_DAT_WIDTH_128;
        } else {
            log_error("Invalid width: 0x%x, mode 0x%x\n", bit_width, work_mode);
            print_errno(OT_ERR_CIPHER_INVALID_PARAM);
            return OT_ERR_CIPHER_INVALID_PARAM;
        }
    }

    func_exit();
    return TD_SUCCESS;
}

static td_s32 kapi_symc_chk_param(const symc_cfg_t *cfg, symc_width *width)
{
    td_s32 ret;

    func_enter();

    if (cfg->alg == OT_CIPHER_ALG_DMA) {
        log_dbg("Alg is DMA.\n");
        return TD_SUCCESS;
    }

    if (cfg->klen >= OT_CIPHER_KEY_LEN_BUTT) {
        log_error("Invalid key len: 0x%x\n", cfg->klen);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    /* set the bit width which depend on alg and mode */
    ret = kapi_symc_match_width(cfg->mode, cfg->width, width);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_match_width, ret);
        return ret;
    }

    ret = kapi_symc_width_check(cfg, *width);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_width_check, ret);
        return ret;
    }

    if (cfg->iv_usage > OT_CIPHER_IV_CHG_ALL_PACK) {
        log_error("Invalid IV Change Flags: 0x%x\n", cfg->iv_usage);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

#ifdef CHIP_AES_CCM_GCM_SUPPORT
    if ((cfg->iv_usage == OT_CIPHER_IV_CHG_ALL_PACK) &&
        ((cfg->mode == OT_CIPHER_WORK_MODE_CCM) || (cfg->mode == OT_CIPHER_WORK_MODE_GCM))) {
        log_error("Invalid IV Change Flags: 0x%x\n", cfg->iv_usage);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }
#endif

    func_exit();
    return TD_SUCCESS;
}

#ifdef CHIP_AES_CCM_GCM_SUPPORT
static td_s32 kapi_symc_check_ccm_gcm_taglen(ot_cipher_alg alg, ot_cipher_work_mode work_mode, td_u32 tlen)
{
    chk_param_err_return(alg != OT_CIPHER_ALG_AES);

    if (work_mode == OT_CIPHER_WORK_MODE_CCM) {
        /* the parameter t denotes the octet length of T(tag)
         * t is an element of  { 4, 6, 8, 10, 12, 14, 16}
         * here t is pConfig->u32TagLen
         */
        if ((tlen & 0x01) || (tlen < AES_CCM_MIN_TAG_LEN) || (tlen > AES_CCM_MAX_TAG_LEN)) {
            log_error("Invalid ccm tag len, tlen = 0x%x.\n", tlen);
            print_errno(OT_ERR_CIPHER_INVALID_PARAM);
            return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else if (work_mode == OT_CIPHER_WORK_MODE_GCM) {
        if ((tlen < AES_GCM_MIN_TAG_LEN) || (tlen > AES_GCM_MAX_TAG_LEN)) {
            log_error("Invalid gcm tag len, tlen = 0x%x.\n", tlen);
            print_errno(OT_ERR_CIPHER_INVALID_PARAM);
            return OT_ERR_CIPHER_INVALID_PARAM;
        }
    } else {
        log_error("Aes with invalid work mode 0x%x for check tag length.\n", work_mode);
        print_errno(OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    return TD_SUCCESS;
}
#endif

static td_s32 kapi_symc_set_key_iv(const symc_cfg_t *cfg, const kapi_symc_ctx *ctx, td_u32 *klen)
{
    td_s32 ret;

    /* Set even key, may be also need set odd key */
    switch (cfg->iv_usage) {
        case OT_CIPHER_IV_CHG_ONE_PACK:
        case OT_CIPHER_IV_CHG_ALL_PACK:
            if (ctx->func->setiv != TD_NULL) {
                ret = ctx->func->setiv(ctx->cryp_ctx, cfg->iv, cfg->ivlen, cfg->iv_usage);
                if (ret != TD_SUCCESS) {
                    print_func_errno(ctx->func->setiv, ret);
                    return ret;
                }
            }
            /* fall through */
        case OT_CIPHER_IV_CHG_NONE:
            if (ctx->func->setkey != TD_NULL) {
                ret = ctx->func->setkey(ctx->cryp_ctx, klen);
                if (ret != TD_SUCCESS) {
                    print_func_errno(ctx->func->setkey, ret);
                    return ret;
                }
                cryp_symc_set_key_source(ctx->cryp_ctx, td_handle_get_chnid(ctx->key_slot));
            }
            break;
        default:
            print_errno(OT_ERR_CIPHER_INVALID_PARAM);
            return OT_ERR_CIPHER_INVALID_PARAM;
    }

    return TD_SUCCESS;
}

static td_s32 kapi_symc_save_ctrl(const symc_cfg_t *cfg, ot_cipher_ctrl *ctrl)
{
    td_u32 *iv = TD_NULL;

    (td_void)memset_s(ctrl, sizeof(ot_cipher_ctrl), 0, sizeof(ot_cipher_ctrl));
    ctrl->alg = cfg->alg;
    ctrl->work_mode = cfg->mode;

    if (ctrl->alg == OT_CIPHER_ALG_AES) {
        if (ctrl->work_mode == OT_CIPHER_WORK_MODE_CCM ||
            ctrl->work_mode == OT_CIPHER_WORK_MODE_GCM) {
            ot_cipher_ctrl_aes_ccm_gcm *aes_ccm_gcm = &ctrl->aes_ccm_gcm_ctrl;
            aes_ccm_gcm->aad_len = cfg->alen;
            aes_ccm_gcm->iv_len = cfg->ivlen;
            aes_ccm_gcm->key_len = cfg->klen;
            aes_ccm_gcm->tag_len = cfg->tlen;
            aes_ccm_gcm->aad_phys_addr = addr_u64(cfg->aad);
            iv = aes_ccm_gcm->iv;
        } else {
            ot_cipher_ctrl_aes *aes = &ctrl->aes_ctrl;
            aes->bit_width = cfg->width;
            aes->chg_flags = cfg->iv_usage;
            aes->key_len = cfg->klen;
            iv = aes->iv;
        }
    } else if (ctrl->alg == OT_CIPHER_ALG_SM4) {
        ot_cipher_ctrl_sm4 *sm4 = &ctrl->sm4_ctrl;
        sm4->chg_flags = cfg->iv_usage;
        iv = sm4->iv;
    } else if (ctrl->alg != OT_CIPHER_ALG_DMA) {
        log_error("Unsupported alg %d\n", ctrl->alg);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    if (cfg->iv != TD_NULL && iv != TD_NULL) {
        chk_param_err_return(cfg->ivlen > AES_IV_SIZE);
        if (memcpy_s(iv, AES_IV_SIZE, cfg->iv, cfg->ivlen) != EOK) {
            print_func_errno(memcpy_s, OT_ERR_CIPHER_FAILED_SEC_FUNC);
            return OT_ERR_CIPHER_FAILED_SEC_FUNC;
        }
    }

    return TD_SUCCESS;
}

static td_s32 kapi_symc_unlock_cfg(td_u32 soft_id,
    const symc_cfg_t *cfg, kapi_symc_ctx *ctx, symc_width width)
{
    td_s32 ret;
    td_u32 klen = cfg->klen;

    /* Destroy the last attached instance of Symmetric cipher engine. */
    if ((ctx->func != TD_NULL) && (ctx->func->destroy != TD_NULL)) {
        (void)ctx->func->destroy(ctx->cryp_ctx);
        ctx->cryp_ctx = TD_NULL;
    }

    /* Clone the function from template of symc engine. */
    ctx->func = cryp_get_symc_op(cfg->alg, cfg->mode);
    if (ctx->func == TD_NULL) {
        print_func_errno(cryp_get_symc_op, OT_ERR_CIPHER_UNSUPPORTED);
        return OT_ERR_CIPHER_UNSUPPORTED;
    }

    /* null means can ignore the function */
    if ((ctx->cryp_ctx == TD_NULL) && (ctx->func->create != TD_NULL)) {
        /* Create a instance from template of engine */
        ctx->cryp_ctx = ctx->func->create(soft_id);
        if (ctx->cryp_ctx == TD_NULL) {
            print_func_errno(ctx->func->create, OT_ERR_CIPHER_INVALID_POINT);
            return OT_ERR_CIPHER_INVALID_POINT;
        }
    }

    /* set mode and alg */
    if (ctx->func->setmode) {
        ctx->func->setmode(ctx->cryp_ctx, ctx->func->alg, ctx->func->mode, width);
    }

    chk_func_err_goto(kapi_symc_set_key_iv(cfg, ctx, &klen));

    /* Set AAD */
#ifdef CHIP_AES_CCM_GCM_SUPPORT
    if (ctx->func->setadd) {
        chk_func_err_goto(cipher_check_mmz_phy_addr((td_phys_addr_t)addr_u64(cfg->aad), cfg->alen));

        chk_func_err_goto(kapi_symc_check_ccm_gcm_taglen(cfg->alg, cfg->mode, cfg->tlen));

        chk_func_err_goto(ctx->func->setadd(ctx->cryp_ctx, cfg->aad, cfg->alen, cfg->tlen));
    }
#endif
    /* save ctrl */
    chk_func_err_goto(kapi_symc_save_ctrl(cfg, &ctx->ctrl));

    ctx->config = TD_TRUE;
exit__:
    return ret;
}

td_s32 kapi_symc_cfg(const symc_cfg_t *cfg)
{
    td_s32 ret;
    kapi_symc_ctx *ctx = TD_NULL;
    symc_width width = SYMC_DAT_WIDTH_COUNT;
    td_u32 soft_id;

    chk_ptr_err_return(cfg);
    chk_param_err_return(cfg->alen > MAX_AEAD_A_LEN);
    chk_param_err_return(addr_l32(cfg->aad) + cfg->alen < cfg->alen);

    ret = kapi_symc_chk_handle((td_handle)cfg->id);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    soft_id = td_handle_get_chnid(cfg->id);
    ctx = &g_symc_ctx[soft_id];
    if (ctx == TD_NULL) {
        log_error("kapi symc ctx is null.\n");
        return OT_ERR_CIPHER_INVALID_POINT;
    }
    crypto_chk_owner_err_return(&ctx->owner); /* called by user space, must check the pid */

    ret = kapi_symc_chk_param(cfg, &width);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_param, ret);
        return ret;
    }

    kapi_symc_lock_err_return();

    ret = kapi_symc_unlock_cfg(soft_id, cfg, ctx, width);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_unlock_cfg, ret);
    }

    kapi_symc_unlock();
    return ret;
}

td_s32 kapi_symc_get_cfg(td_u32 id, ot_cipher_ctrl *ctrl)
{
    td_s32 ret;
    kapi_symc_ctx *ctx = TD_NULL;
    td_u32 *piv = TD_NULL;
    td_u32 *piv_len = TD_NULL;
    td_u32 soft_id, iv_len;

    func_enter();

    chk_ptr_err_return(ctrl);

    ret = kapi_symc_chk_handle((td_handle)id);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    soft_id = td_handle_get_chnid(id);
    ctx = &g_symc_ctx[soft_id];
    crypto_chk_owner_err_return(&ctx->owner); /* called by user space, must check the pid */
    chk_param_err_return(ctx->config != TD_TRUE);

    kapi_symc_lock_err_return();

    (td_void)memcpy_s(ctrl, sizeof(ot_cipher_ctrl), &ctx->ctrl, sizeof(ot_cipher_ctrl));
    if (ctrl->alg == OT_CIPHER_ALG_AES) {
        if ((ctrl->work_mode == OT_CIPHER_WORK_MODE_CCM) ||
            (ctrl->work_mode == OT_CIPHER_WORK_MODE_GCM)) {
            piv = ctrl->aes_ccm_gcm_ctrl.iv;
            piv_len = &ctrl->aes_ccm_gcm_ctrl.iv_len;
        } else {
            piv = ctrl->aes_ctrl.iv;
        }
    } else if (ctrl->alg == OT_CIPHER_ALG_SM4) {
        piv = ctrl->sm4_ctrl.iv;
    }
    if (piv != TD_NULL && ctx->func != TD_NULL && ctx->func->getiv != TD_NULL) {
        (td_void)memset_s((td_u8 *)piv, AES_IV_SIZE, 0, AES_IV_SIZE);
        ret = ctx->func->getiv(ctx->cryp_ctx, (td_u8 *)piv, AES_IV_SIZE, &iv_len);
        if (ret != TD_SUCCESS) {
            kapi_symc_unlock();
            print_func_errno(ctx->func->getiv, ret);
            return ret;
        }
        if (piv_len != TD_NULL) {
            *piv_len = iv_len;
        }
    }

    kapi_symc_unlock();

    func_exit();

    return ret;
}

td_s32 kapi_symc_crypto(symc_encrypt_t *crypt)
{
    td_s32 ret;
    symc_node_usage usage;
    kapi_symc_ctx *ctx = TD_NULL;
    td_u32 soft_id;
    symc_multi_pack pack;

    func_enter();

    chk_ptr_err_return(crypt);

    ret = kapi_symc_chk_handle((td_handle)crypt->id);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    soft_id = td_handle_get_chnid(crypt->id);
    ctx = &g_symc_ctx[soft_id];
    crypto_chk_owner_err_return(&ctx->owner); /* called by user space, must check the pid */
    chk_param_err_return(addr_u64(crypt->input) + crypt->length < crypt->length);
    chk_param_err_return(addr_u64(crypt->output) + crypt->length < crypt->length);
    chk_ptr_err_return(ctx->func);
    chk_ptr_err_return(ctx->func->crypto);
    chk_param_err_return(ctx->config != TD_TRUE);
    if (ctx->ctrl.alg != OT_CIPHER_ALG_DMA) {
        chk_param_err_return(ctx->attach != TD_TRUE);
    }
    chk_param_err_return((crypt->operation != SYMC_OPERATION_ENCRYPT) &&
        (crypt->operation != SYMC_OPERATION_DECRYPT));

    usage = SYMC_NODE_USAGE_NORMAL;
    (td_void)memset_s(&pack, sizeof(pack), 0, sizeof(pack));
    pack.in  = &crypt->input;
    pack.out = &crypt->output;
    pack.len = &crypt->length;
    pack.usage = &usage;
    pack.num = 1; /* 1 single package encrypt or decrypt. */

    kapi_symc_lock_err_return();

    ret = ctx->func->crypto(ctx->cryp_ctx, crypt->operation, &pack, TD_TRUE);
    if (ret != TD_SUCCESS) {
        print_func_errno(ctx->func->crypto, ret);
        kapi_symc_unlock();
        return ret;
    }

    kapi_symc_unlock();
    func_exit();
    return TD_SUCCESS;
}

td_s32 kapi_symc_crypto_via(symc_encrypt_t *crypt, td_u32 is_from_user)
{
    td_s32 ret, ret_exit;
    crypto_mem mem = {0};
    symc_encrypt_t local_crypt;

    func_enter();

    chk_ptr_err_return(crypt);
    chk_ptr_err_return(addr_via_const(crypt->input));
    chk_ptr_err_return(addr_via(crypt->output));
    chk_param_err_return(crypt->length == 0x00);

    (td_void)memset_s(&local_crypt, sizeof(local_crypt), 0, sizeof(local_crypt));
    ret = crypto_mem_create(&mem, MEM_TYPE_MMZ, "AES_IN", crypt->length);
    if (ret != TD_SUCCESS) {
        print_func_errno(crypto_mem_create, ret);
        return ret;
    }

    if (is_from_user == TD_TRUE) {
        chk_func_err_goto(crypto_copy_from_user(mem.dma_virt,
            crypt->length, addr_via_const(crypt->input), crypt->length));
    } else {
        if (memcpy_s(mem.dma_virt, mem.dma_size, addr_via_const(crypt->input), crypt->length) != EOK) {
            print_func_errno(memcpy_s, OT_ERR_CIPHER_FAILED_SEC_FUNC);
            ret = OT_ERR_CIPHER_FAILED_SEC_FUNC;
            goto exit__;
        }
    }

    local_crypt.id     = crypt->id;
    local_crypt.input  = mem.dma_addr;
    local_crypt.output = mem.dma_addr;
    local_crypt.length = crypt->length;
    local_crypt.last   = crypt->last;
    local_crypt.operation = crypt->operation & SYMC_OPERATION_DECRYPT;
    chk_func_err_goto(kapi_symc_crypto(&local_crypt));

    if (is_from_user == TD_TRUE) {
        chk_func_err_goto(crypto_copy_to_user(addr_via(crypt->output), crypt->length, mem.dma_virt, crypt->length));
    } else {
        if (memcpy_s(addr_via(crypt->output), crypt->length, mem.dma_virt, crypt->length) != EOK) {
            print_func_errno(memcpy_s, OT_ERR_CIPHER_FAILED_SEC_FUNC);
            ret = OT_ERR_CIPHER_FAILED_SEC_FUNC;
            goto exit__;
        }
    }

exit__:
    ret_exit = crypto_mem_destroy(&mem);
    if (ret_exit != TD_SUCCESS) {
        print_func_errno(crypto_mem_destroy, ret_exit);
        return ret_exit;
    }

    func_exit();
    return ret;
}

static td_s32 kapi_symc_chk_multi_pack(ot_cipher_data *tmp, const ot_cipher_data *pack)
{
    td_s32 ret;

    /* copy node list from user space to kernel. */
    ret = crypto_copy_from_user(tmp, sizeof(ot_cipher_data), pack, sizeof(ot_cipher_data));
    if (ret != TD_SUCCESS) {
        print_func_errno(crypto_copy_from_user, ret);
        return ret;
    }

    if (tmp->src_phys_addr + tmp->byte_len < tmp->byte_len) {
        print_errno(OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    if (tmp->dst_phys_addr + tmp->byte_len < tmp->byte_len) {
        print_errno(OT_ERR_CIPHER_INVALID_PARAM);
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    ret = cipher_check_mmz_phy_addr(tmp->src_phys_addr, tmp->byte_len);
    if (ret != TD_SUCCESS) {
        print_func_errno(cipher_check_mmz_phy_addr, ret);
        return ret;
    }

    ret = cipher_check_mmz_phy_addr(tmp->dst_phys_addr, tmp->byte_len);
    if (ret != TD_SUCCESS) {
        print_func_errno(cipher_check_mmz_phy_addr, ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_s32 kapi_symc_crypto_multi_start(const kapi_symc_ctx *ctx,
    const ot_cipher_data *pkg, td_u32 pkg_num, td_u32 operation, td_u32 wait)
{
    td_s32 ret;
    td_void *buf = TD_NULL;
    ot_cipher_data pkg_tmp;
    td_u32 i, size;
    td_void *temp = TD_NULL;
    symc_multi_pack pack;

    chk_param_err_return((ctx == TD_NULL) ||
        (ctx->cryp_ctx == TD_NULL) || (ctx->func == TD_NULL) || (ctx->func->crypto == TD_NULL));
    chk_param_err_return((pkg == TD_NULL) || (pkg_num > MAX_PKG_NUMBER) || (pkg_num == 0x00));

    /* size of input:output:usage:length */
    size = (sizeof(compat_addr) + sizeof(compat_addr) + sizeof(symc_node_usage) + sizeof(td_u32)) * pkg_num;
    buf = crypto_calloc(1, size);
    if (buf == TD_NULL) {
        print_func_errno(crypto_calloc, OT_ERR_CIPHER_FAILED_MEM);
        return OT_ERR_CIPHER_FAILED_MEM;
    }

    (td_void)memset_s(&pack, sizeof(pack), 0, sizeof(pack));
    pack.num = pkg_num;
    temp = buf;
    pack.in = (compat_addr *)temp;
    temp = (td_u8 *)temp + sizeof(compat_addr) * pkg_num;    /* decrypt: buf + input. */
    pack.out = (compat_addr *)temp;
    temp = (td_u8 *)temp + sizeof(compat_addr) * pkg_num;    /* decrypt: buf + input + output. */
    pack.usage = (symc_node_usage *)temp;
    temp = (td_u8 *)temp + sizeof(symc_node_usage) * pkg_num;    /* decrypt: buf + input + output + usage. */
    pack.len = (td_u32 *)temp;

    /* Compute and check the nodes length. */
    for (i = 0; i < pkg_num; i++) {
        (td_void)memset_s(&pkg_tmp, sizeof(ot_cipher_data), 0, sizeof(ot_cipher_data));
        ret = kapi_symc_chk_multi_pack(&pkg_tmp, &pkg[i]);
        if (ret != TD_SUCCESS) {
            print_func_errno(kapi_symc_chk_multi_pack, ret);
            crypto_free(buf);
            return ret;
        }

        addr_u64(pack.in[i]) = pkg_tmp.src_phys_addr;
        addr_u64(pack.out[i]) = pkg_tmp.dst_phys_addr;
        pack.len[i] = pkg_tmp.byte_len;
        /* default used even key */
        pack.usage[i] = SYMC_NODE_USAGE_EVEN_KEY;
        log_dbg("pkg %u, in 0x%x, out 0x%x, length 0x%x, usage 0x%x\n", i,
            addr_l32(pack.in[i]), addr_l32(pack.out[i]), pack.len[i], pack.usage[i]);
    }

    ret = ctx->func->crypto(ctx->cryp_ctx, operation, &pack, wait);
    if (ret != TD_SUCCESS) {
        print_func_errno(ctx->func->crypto, ret);
    }

    crypto_free(buf);
    return ret;
}

td_s32 kapi_symc_crypto_multi(td_u32 id, const ot_cipher_data *pkg, td_u32 pkg_num, td_u32 operation, td_u32 last)
{
    td_s32 ret;
    kapi_symc_ctx *ctx = TD_NULL;
    td_u32 soft_id;

    crypto_unused(last);

    func_enter();

    ret = kapi_symc_chk_handle((td_handle)id);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    soft_id = td_handle_get_chnid(id);
    ctx = &g_symc_ctx[soft_id];
    crypto_chk_owner_err_return(&ctx->owner); /* called by user space, must check the pid */
    chk_ptr_err_return(ctx->func);
    chk_param_err_return(ctx->config != TD_TRUE);
    if (ctx->ctrl.alg != OT_CIPHER_ALG_DMA) {
        chk_param_err_return(ctx->attach != TD_TRUE);
    }
    chk_param_err_return((operation != 0x00) && (operation != 0x01));

    kapi_symc_lock_err_return();

    ret = kapi_symc_crypto_multi_start(ctx, pkg, pkg_num, operation, TD_TRUE);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_crypto_multi_start, ret);
        kapi_symc_unlock();
        return ret;
    }

    kapi_symc_unlock();

    func_exit();
    return TD_SUCCESS;
}

#ifdef CHIP_AES_CCM_GCM_SUPPORT
td_s32 kapi_aead_get_tag(td_u32 id, td_u32 tag[AEAD_TAG_SIZE_IN_WORD], td_u32 *taglen)
{
    td_s32 ret;
    kapi_symc_ctx *ctx = TD_NULL;
    td_u32 soft_id;

    func_enter();

    chk_ptr_err_return(tag);
    chk_ptr_err_return(taglen);
    chk_param_err_return(*taglen != AES_CCM_MAX_TAG_LEN);

    ret = kapi_symc_chk_handle((td_handle)id);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    soft_id = td_handle_get_chnid(id);
    ctx = &g_symc_ctx[soft_id];
    crypto_chk_owner_err_return(&ctx->owner); /* called by user space, must check the pid */
    chk_ptr_err_return(ctx->func);
    chk_ptr_err_return(ctx->func->gettag);

    kapi_symc_lock_err_return();

    if (ctx->func->gettag) {
        ret = ctx->func->gettag(ctx->cryp_ctx, tag, taglen);
        if (ret != TD_SUCCESS) {
            print_func_errno(ctx->func->gettag, ret);
            kapi_symc_unlock();
            return ret;
        }
    }

    kapi_symc_unlock();

    func_exit();
    return TD_SUCCESS;
}
#endif

static td_s32 kapi_keyslot_chk_handle(td_handle handle)
{
    chk_handle_modid(handle, OT_ID_KEYSLOT);
    chk_handle_private_data(handle, OT_PRIVATE_ID_KEYSLOT);
    chk_handle_chnid(handle, KEYSLOT_CHANNEL_MAX);

    if (g_keyslot_open[td_handle_get_chnid(handle)] != TD_TRUE) {
        log_error("channel %u is not open\n", td_handle_get_chnid(handle));
        return OT_ERR_CIPHER_INVALID_HANDLE;
    }

    return TD_SUCCESS;
}

td_s32 kapi_keyslot_create(const ot_keyslot_attr *attr, td_handle *keyslot)
{
    td_s32 ret;
    td_handle id;

    func_enter();

    chk_ptr_err_return(attr);
    chk_ptr_err_return(keyslot);

    kapi_symc_lock_err_return();

    ret = drv_kc_slot_lock(attr, &id);
    if (ret != TD_SUCCESS) {
        print_func_errno(drv_kc_slot_lock, ret);
        kapi_symc_unlock();
        return ret;
    }

    *keyslot = td_handle_init(OT_ID_KEYSLOT, OT_PRIVATE_ID_KEYSLOT, id);
    g_keyslot_open[id] = TD_TRUE;

    kapi_symc_unlock();

    func_exit();

    return ret;
}

td_s32 kapi_keyslot_destroy(td_handle keyslot)
{
    td_s32 ret;
    td_u32 id;

    func_enter();

    ret = kapi_keyslot_chk_handle(keyslot);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_keyslot_chk_handle, ret);
        return ret;
    }

    kapi_symc_lock_err_return();

    id = td_handle_get_chnid(keyslot);

    ret = drv_kc_slot_unlock(id);
    if (ret != TD_SUCCESS) {
        print_func_errno(drv_kc_slot_unlock, ret);
        kapi_symc_unlock();
        return ret;
    }
    g_keyslot_open[id] = TD_FALSE;

    kapi_symc_unlock();
    func_exit();
    return ret;
}

td_s32 kapi_symc_attach(td_handle cipher, td_handle keyslot)
{
    td_s32 ret;
    td_u32 id;
    kapi_symc_ctx *symc_ctx = TD_NULL;

    func_enter();

    ret = kapi_symc_chk_handle(cipher);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    ret = kapi_keyslot_chk_handle(keyslot);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_keyslot_chk_handle, ret);
        return ret;
    }

    kapi_symc_lock_err_return();

    id = td_handle_get_chnid(cipher);
    symc_ctx = &g_symc_ctx[id];

    symc_ctx->attach = TD_TRUE;
    symc_ctx->key_slot = keyslot;

    kapi_symc_unlock();

    func_exit();

    return TD_SUCCESS;
}

td_s32 kapi_symc_detach(td_handle cipher, td_handle keyslot)
{
    td_s32 ret;
    td_u32 id;
    kapi_symc_ctx *symc_ctx = TD_NULL;

    func_enter();

    ret = kapi_symc_chk_handle(cipher);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_symc_chk_handle, ret);
        return ret;
    }

    ret = kapi_keyslot_chk_handle(keyslot);
    if (ret != TD_SUCCESS) {
        print_func_errno(kapi_keyslot_chk_handle, ret);
        return ret;
    }

    kapi_symc_lock_err_return();

    id = td_handle_get_chnid(cipher);
    symc_ctx = &g_symc_ctx[id];

    if (symc_ctx->attach != TD_TRUE) {
        log_error("cipher %u isn't attached\n", id);
        kapi_symc_unlock();
        return OT_ERR_CIPHER_INVALID_PARAM;
    }

    if (symc_ctx->key_slot != keyslot) {
        log_error("handle don't match, attach 0x%x, input 0x%x\n", symc_ctx->key_slot, keyslot);
        kapi_symc_unlock();
        return OT_ERR_CIPHER_INVALID_PARAM;
    }
    symc_ctx->key_slot = OT_INVALID_HANDLE;
    symc_ctx->attach = TD_FALSE;

    kapi_symc_unlock();

    func_exit();

    return TD_SUCCESS;
}
