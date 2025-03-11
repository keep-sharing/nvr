/* 
 * ***************************************************************
 * Filename:      	msfs_notify.h
 * Created at:    	2017.10.10
 * Description:   	msfs notify api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __MSFS_NOTIFY_H__
#define __MSFS_NOTIFY_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum progress_bar_e{
    PROGRESS_BAR_DISK_FORMAT = 0,
    PROGRESS_BAR_BKP_FORMAT = 1,
    PROGRESS_BAR_RAID_REBUILD = 2,
    PROGRESS_BAR_RAID_CREATE = 3,
    PROGRESS_BAR_RETRIEVE = 4,
    PROGRESS_BAR_RETRIEVE_EXPORT = 5,
    PROGRESS_BAR_LOG_SEARCH = 6,
    PROGRESS_BAR_LOG_EXPORT = 7,
    PROGRESS_BAR_DISK_LOADING = 8,
    PROGRESS_BAR_DISK_RESTORE = 9,
}PROGRESS_BAR_E;

typedef struct progress_bar_t{
    MF_U32  sid;
    MF_U8   percent;
    MF_U8   port;//for disk: format / raid create&rebuild
    MF_U32  type;// value:see TYPE_EN, just for disk
    MF_U8   searchfrom;// for retrieve
    MF_U8   searchid;// for retrieve
}PROGRESS_BAR_T;

typedef struct mem_hdl_t{
    void  *pstMem;
    MF_U32  size;
}MEM_HDL_T;

typedef struct mem_pool_t{
    MF_U32  recSize;
    MF_U32  pbSize;
    MF_U32  retrSize;
    MF_U32  diskSize;
    MF_U32  logSize;
    MF_U32  taskSize;
}MEM_POOL_T;
typedef void (*BAR_FUNC)(PROGRESS_BAR_E enBar, PROGRESS_BAR_T *pstBar);

/**
* \brief add a progress bar state callback.
* \param[in] BAR_FUNC
* \return void.
*/
void
msfs_notify_progress_bar_add(BAR_FUNC func);

/**
* \brief del specific progress bar state callback.
* \param[in] BAR_FUNC
* \return void.
*/
void
msfs_notify_progress_bar_del(BAR_FUNC func);

/**
* \brief broadcast  progress bar state.
* \param[in] enBar
* \param[in] argv
* \return void.
*/
void
msfs_notify_progress_bar_state(PROGRESS_BAR_E enBar,void * argv);

/**
* \brief create  memory pool.
* \param[in] name
* \param[in] size
* \return pstHdl.
*/
MEM_HDL_T *
msfs_mem_pool_create(MF_S8 *name, MF_U32 size);

/**
* \brief destory  memory pool.
* \param[in] pstHdl
* \return void.
*/
void
msfs_mem_pool_destory(struct mem_hdl_t *pstHdl);

/**
* \brief calloc  mem from pool && memset 0;
* \param[in] pstHdl
* \param[in] n
* \param[in] size
* \return void.
*/
void *
msfs_mem_calloc(struct mem_hdl_t *pstHdl, MF_U32 n, MF_U32 size);

/**
* \brief malloc  mem from pool.
* \param[in] pstHdl
* \param[in] size
* \return void.
*/
void *
msfs_mem_malloc(struct mem_hdl_t *pstHdl, MF_U32 size);

/**
* \brief release  mem to pool.
* \param[in] pstHdl
* \param[in] p
* \return void.
*/
void
msfs_mem_free(struct mem_hdl_t *pstHdl, void *p);

/**
* \brief   mem usage state.
* \param[in] pstHdl
* \return void.
*/
void
msfs_mem_state(struct mem_hdl_t *pstHdl);

/**
* \brief config  mem  pool size .
* \param[in] pstMpool
* \return void.
*/
void
msfs_mem_pool_config(struct mem_pool_t *pstMpool);
#ifdef __cplusplus
}
#endif

#endif


