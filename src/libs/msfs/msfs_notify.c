/* 
 * ***************************************************************
 * Filename:      	msfs_notify.c
 * Created at:    	2017.12.20
 * Description:   	msfs notify API
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
#include "mf_type.h"
#include "MFplayback.h"
#include "MFtask.h"
#include "MFmemory.h"
#include "msfs_notify.h"

void
msfs_notify_progress_bar_add(BAR_FUNC func)
{
    mf_comm_progress_bar_register(func);
}

void
msfs_notify_progress_bar_del(BAR_FUNC func)
{
    mf_comm_progress_bar_unregister(func);
}

void
msfs_notify_progress_bar_state(PROGRESS_BAR_E enBar,void * argv)
{
    mf_comm_progress_bar_notify(enBar, argv);
}

struct mem_hdl_t *
msfs_mem_pool_create(MF_S8 *name, MF_U32 size)
{
    struct mem_hdl_t *pstHdl;
    struct mf_mem_t *pstMem;
    
    pstHdl = ms_calloc(1, sizeof(struct mem_hdl_t));
    pstMem = mf_mem_pool_create(name, size);

    if (pstMem == NULL)
    {
        ms_free(pstHdl);
        return NULL;
    }
    pstHdl->pstMem = pstMem;
    pstHdl->size   = size;

    return pstHdl;
}

void
msfs_mem_pool_destory(struct mem_hdl_t *pstHdl)
{
    mf_mem_pool_destory(pstHdl->pstMem);
    ms_free(pstHdl);
}

void *
msfs_mem_calloc(struct mem_hdl_t *pstHdl, MF_U32 n, MF_U32 size)
{
    return mf_mem_calloc(pstHdl->pstMem, n, size);
}

void *
msfs_mem_malloc(struct mem_hdl_t *pstHdl, MF_U32 size)
{
    return mf_mem_alloc(pstHdl->pstMem, size);
}

void
msfs_mem_free(struct mem_hdl_t *pstHdl, void *p)
{
    mf_mem_free(pstHdl->pstMem, p);
}

void
msfs_mem_state(struct mem_hdl_t *pstHdl)
{
    mf_mem_state(pstHdl->pstMem, MF_YES);
}

void
msfs_mem_pool_config(struct mem_pool_t *pstMpool)
{
   mf_comm_mem_set(pstMpool);
}

