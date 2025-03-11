// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef CRYP_HASH_H
#define CRYP_HASH_H

#include "drv_osal_lib.h"
#include "drv_hash.h"

#define HASH_MAX_BUFFER_SIZE    0x10000 /* 64K */

/******************************* API Code *****************************/
/* Initialize crypto of hash. */
td_s32 cryp_hash_init(td_void);

/* Deinitialize crypto of hash. */
td_void cryp_hash_deinit(td_void);

/*
 * \brief          Create hash handle
 *
 * \param mode     Hash mode
 * \return         ctx if successful, or NULL
 */
typedef td_void *(*func_hash_create)(hash_mode mode);

/*
 * \brief          Clear hash context
 *
 * \param ctx      symc handle to be destroy
 */
typedef td_s32 (*func_hash_destroy)(td_void *ctx);

/*
 * \brief          Hash message chunk calculation
 *
 * Note: the message must be write to the buffer
 * which get from cryp_hash_get_cpu_addr, and the length of message chunk
 * can't large than the length which get from cryp_hash_get_cpu_addr.
 *
 * \param ctx      hash handle to be destroy
 * \param chunk    hash message to update
 * \param length   length of hash message
 * \param src      source of hash message
 */
typedef td_s32 (*func_hash_update)(td_void *ctx, const td_u8 *chunk, td_u32 chunk_len, hash_chunk_src src);

/*
 * \brief          HASH final digest
 *
 * \param ctx      Hash handle
 * \param hash     HASH checksum result
 * \param hashlen  Length of HASH checksum result
 */
typedef td_s32 (*func_hash_finish)(td_void *ctx,  td_void *hash, td_u32 hash_buf_len, td_u32 *hashlen);

/* struct of Hash function template. */
typedef struct {
    td_u32 valid;                  /* vliad or not */
    td_u32 mode;                   /* Mode of Hash */
    td_u32 block_size;             /* block size */
    td_u32 size;                   /* hash output size */
    func_hash_create  create;      /* Create function */
    func_hash_destroy destroy;     /* destroy function */
    func_hash_update  update;      /* update function */
    func_hash_finish  finish;      /* finish function */
} hash_func;

/*
 * \brief  Clone the function from template of hash engine.
 * \param[out]  func The struct of function.
 * \param[in]  mode The work mode.
 * \retval     On success, TD_SUCCESS is returned.  On error, TD_FAILURE is returned.
 */
hash_func *cryp_get_hash(hash_mode mode);

#endif /* CRYP_HASH_H */
