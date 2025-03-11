/*
 * ***************************************************************
 * Filename:        MFmemory.c
 * Created at:      2018.02.23
 * Description:     memory pool API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include "MFmemory.h"
#include "ngx_slab.h"

#define MAX_MFMEM_POOL (10)
typedef enum {
    MF_MALLOC = 0,
    MF_CALLOC = 1,
    MF_FREE = 2,
} MF_MEM_TYPE;
struct mf_mem_conf {
    int loaded;
    int nMallocCnt[MAX_MFMEM_POOL];
    int nCallocCnt[MAX_MFMEM_POOL];
    int nFreeCnt[MAX_MFMEM_POOL];
    int pstMemCnt[MAX_MFMEM_POOL];
    MF_S8 pstMemName[MAX_MFMEM_POOL][32];
    MF_U32 pstMemSize[MAX_MFMEM_POOL];
    struct mf_mem_t *pstMemPtr[MAX_MFMEM_POOL];
};
static struct mf_mem_conf mfconf = {0};
static int mf_mem_debug_init(MF_S8 *name, MF_U32 size, struct mf_mem_t *pstMem)
{
    int i = 0;
    if (!mfconf.loaded) {
        MFshow("mf_mem_debug_init loaded.");
        memset(&mfconf, 0, sizeof(struct mf_mem_conf));
        mfconf.loaded = 0;
    }
    MFshow("name:%s size:%d pstMem:%p", name, size, pstMem);
    for (i = 0; i < MAX_MFMEM_POOL; i++) {
        if (mfconf.pstMemPtr[i] == NULL) {
            snprintf(mfconf.pstMemName[i], 32, "%s", name);
            mfconf.pstMemPtr[i] = pstMem;
            mfconf.pstMemSize[i] = size;
            break;
        }
    }
    return 0;
}
static int mf_mem_debug_update(struct mf_mem_t *pstMem, MF_MEM_TYPE type)
{
    int i = 0;
    if (!mfconf.loaded) {
        return -1;
    }
    for (i = 0; i < MAX_MFMEM_POOL; i++) {
        if (mfconf.pstMemPtr[i] == pstMem) {
            mfconf.pstMemCnt[i]++;
            switch (type) {
                case MF_MALLOC:
                    mfconf.nMallocCnt[i]++;
                    break;
                case MF_CALLOC:
                    mfconf.nCallocCnt[i]++;
                    break;
                case MF_FREE:
                    mfconf.nFreeCnt[i]++;
                    break;
            }
            break;
        }
    }
    return 0;
}
static int mf_mem_debug_show()
{
    int i = 0;
    for (i = 0; i < MAX_MFMEM_POOL; i++) {
        if (!mfconf.pstMemCnt[i]) {
            continue;
        }
        if (!mfconf.pstMemSize[i]) {
            continue;
        }
        MFshow("name:%s pstMemPtr:%p size:%d cnt:%d", mfconf.pstMemName[i], mfconf.pstMemPtr[i], mfconf.pstMemSize[i],
               mfconf.pstMemCnt[i]);
        MFshow("nMallocCnt:%d nCallocCnt:%d nFreeCnt:%d", mfconf.nMallocCnt[i], mfconf.nCallocCnt[i], mfconf.nFreeCnt[i]);
    }
    return 0;
}
struct mf_mem_t *
mf_mem_pool_create(MF_S8 *name, MF_U32 size)
{
    struct mf_mem_t *pstMem;

    pstMem = ms_calloc(1, sizeof(struct mf_mem_t));
    ms_mutex_init(&pstMem->mutex);
    mf_mem_debug_init(name, size, pstMem);

    pstMem->addr = ms_calloc(1, size);
    if (pstMem->addr == NULL) {
        ms_free(pstMem);
        return NULL;
    }

    ngx_slab_pool_t *sp;
    sp = (ngx_slab_pool_t *)pstMem->addr;
    strncpy(sp->name, name, sizeof(sp->name));
    sp->addr = pstMem->addr;
    sp->min_shift = 3;
    sp->end = pstMem->addr + size;

    ngx_slab_init(sp);
    pstMem->pool = sp;
    pstMem->pageSize = sp->page_size;

    return pstMem;
}


void
mf_mem_pool_destory(struct mf_mem_t *pstMem)
{
    ms_mutex_uninit(&pstMem->mutex);
    ms_free(pstMem->addr);
    ms_free(pstMem);
}

void *
mf_mem_alloc(struct mf_mem_t *pstMem, MF_U32 size)
{
    void *p;

    if (pstMem->pageSize * 20 < size) {
        ms_mutex_lock(&pstMem->mutex);
        p = ms_malloc(size);
        mf_mem_debug_update(pstMem, MF_MALLOC);
        ms_mutex_unlock(&pstMem->mutex);
    } else {
        ms_mutex_lock(&pstMem->mutex);
        p = ngx_slab_alloc(pstMem->pool, size);
        if (p == NULL) {
            p = ms_malloc(size);
            mf_mem_debug_update(pstMem, MF_MALLOC);
        }
        ms_mutex_unlock(&pstMem->mutex);
    }
    return p;
}

void *
mf_mem_calloc(struct mf_mem_t *pstMem, MF_U32 n, MF_U32 size)
{
    void *p;

    if (pstMem->pageSize * 20 < n * size) {
        ms_mutex_lock(&pstMem->mutex);
        p = ms_calloc(n, size);
        mf_mem_debug_update(pstMem, MF_CALLOC);
        ms_mutex_unlock(&pstMem->mutex);
    } else {
        ms_mutex_lock(&pstMem->mutex);
        p = ngx_slab_calloc(pstMem->pool, n * size);
        if (p == NULL) {
            p = ms_calloc(n, size);
            mf_mem_debug_update(pstMem, MF_CALLOC);
        }
        ms_mutex_unlock(&pstMem->mutex);
    }
    return p;
}

void
mf_mem_free(struct mf_mem_t *pstMem, void *p)
{
    MF_S32 res;

    ms_mutex_lock(&pstMem->mutex);
    res = ngx_slab_free(pstMem->pool, p);
    if (res == MF_FAILURE) {
        mf_mem_debug_update(pstMem, MF_FREE);
        ms_free(p);
    }
    ms_mutex_unlock(&pstMem->mutex);
    return ;
}

MF_U32
mf_mem_state(struct mf_mem_t *pstMem, MF_BOOL bVerbose)
{
    ngx_slab_info_t stat;
    ms_mutex_lock(&pstMem->mutex);
    ngx_slab_stat(pstMem->pool, &stat, bVerbose);
    ms_mutex_unlock(&pstMem->mutex);

    if (bVerbose) {
        mf_mem_debug_show();
    }
    return stat.used_pct;
}


