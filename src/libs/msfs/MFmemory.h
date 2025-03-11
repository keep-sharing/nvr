/*
 * ***************************************************************
 * Filename:        MFmemory.h
 * Created at:      2018.2.23
 * Description:     memory pool api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFMEMORY_H__
#define __MFMEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mf_type.h"

typedef struct mf_mem_t {
    MUTEX_OBJECT    mutex;
    MF_U32  pageSize;
    void    *pool;
    void    *addr;
} mf_mem_t;

struct mf_mem_t *
mf_mem_pool_create(MF_S8 *name, MF_U32 size);
void
mf_mem_pool_destory(struct mf_mem_t *pstMem);
void *
mf_mem_alloc(struct mf_mem_t *pstMem, MF_U32 size);
void *
mf_mem_calloc(struct mf_mem_t *pstMem, MF_U32 n, MF_U32 size);
void
mf_mem_free(struct mf_mem_t *pstMem, void *p);
MF_U32
mf_mem_state(struct mf_mem_t *pstMem, MF_BOOL bVerbose);

#ifdef __cplusplus
}
#endif

#endif

