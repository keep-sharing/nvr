/* 
 * ***************************************************************
 * Filename:      	thread_pool.h
 * Created at:    	2019.1.25
 * Description:   	thread pool api.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#ifndef _THPOOL_
#define _THPOOL_

#ifdef __cplusplus
extern "C" {
#endif

#include <list.h>

#define MY_TYPE  "msfs" //user-defined 

#ifdef MY_TYPE
#include "msstd.h"
#include "mf_type.h"

#define TPshow(format, ...)  MFshow(format, ##__VA_ARGS__)
#define TPinfo(format, ...)  MFinfo(format, ##__VA_ARGS__)
#define TPdbg(format, ...)   MFdbg(format, ##__VA_ARGS__)
#define TPwarm(format, ...)  MFwarm(format, ##__VA_ARGS__)
#define TPerr(format, ...)   MFerr(format, ##__VA_ARGS__)

#define tp_malloc(len)     ms_malloc(len)
#define tp_calloc(n, size) ms_calloc(n, size);
#define tp_free(p)		   ms_free(p)
#else
#include <pthread.h>

#define TPshow(prefix, format, ...) \
    do{\
        printf("[%s]" format "@func:%s file:%s line:%d\n", \
            prefix, ##__VA_ARGS__, __func__, __FILE__, __LINE__); \
    }while(0)

#define TPinfo(format, ...) TPshow("TPinfo", format, ##__VA_ARGS__ )
#define TPdgb(format, ...)  TPshow("TPdgb", format, ##__VA_ARGS__ )
#define TPwarm(format, ...) TPshow("TPwarm", format, ##__VA_ARGS__ )
#define TPerr(format, ...)  TPshow("TPerr", format, ##__VA_ARGS__ )

#define tp_malloc(len)      malloc(len)
#define tp_calloc(n, size)  calloc(n, size);
#define tp_free(p)		    free(p)

#endif 
/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

typedef unsigned char           TP_U8;
typedef unsigned short          TP_U16;
typedef unsigned int            TP_U32;

typedef char                    TP_S8;
typedef short                   TP_S16;
typedef int                     TP_S32;

typedef float                   TP_FLOAT;
typedef double                  TP_DOUBLE;

#ifndef _M_IX86
    typedef unsigned long long  TP_U64;
    typedef long long           TP_S64;
#else
    typedef __int64             TP_U64;
    typedef __int64             TP_S64;
#endif

typedef char                    TP_CHAR;
#define TP_VOID                 void

typedef pthread_cond_t          TP_COND_T;
typedef pthread_mutex_t         TP_MUTEX_T;


/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
typedef enum {
    TP_FALSE = 0,
    TP_TRUE  = 1,
    TP_NO    = 0,
    TP_YES   = 1,
} TP_BOOL;

typedef struct tp_thread_t{
    pthread_t   PID;
    TP_BOOL     bJoin;
}TP_THREAD_T;

#ifndef NULL
    #define NULL    0L
#endif

#define TP_NULL     0L
#define TP_SUCCESS  0
#define TP_FAILURE      (-1)
#define TP_UNKNOWN      (-2)

typedef enum reject_e{
    TP_JOB_DISCARD = 0,
    TP_JOB_REPLACE,
}REJECT_E;

typedef struct tjob_s{
    void *(*work)(void*);
    void *(*after)(void*);
    void *argv;
    struct list_head node;
}TJOB_S;

/* Threadpool */
typedef struct tpool_s{
    TP_U32  minWorkers;
    TP_U32 maxWorkers;
    TP_U32 maxJobs;
    time_t maxIdleTime;
    REJECT_E enPolicy;
    TP_U32 workers;
    TP_U32 busy;
    TP_U32 jobs;
    TP_BOOL bRun;
    TP_MUTEX_T  workerLock;
    TP_MUTEX_T  jobLock;
    TP_MUTEX_T  toDOLock;
    TP_COND_T   toDoJob;
    struct list_head jobsList;
    struct list_head workersList;
} TPOOL_S;


/**
* \ create a thread pool that dynamic thread addition and subtraction
* \param[in] minWorkers -> must be greater than 0
* \param[in] maxWorkers -> 0: ulimit
* \param[in] maxJobs ->0: ulimit
* \return TPOOL_S* for success, NULL for failure.
*/
TPOOL_S* 
tpool_init(TP_U32 minWorkers, TP_U32 maxWorkers, TP_U32 maxJobs);

/**
* \ destroy specific thread pool;
* \param[in] TPOOL_S *
* \return None.
*/
void 
tpool_uninit(TPOOL_S *tp);

/**
* \ submit a regular job that waitting for workers
* \param[in] tp 
* \param[in] work -> job to do  
* \param[in] after -> to do after job
* \param[in] argv-> for work/after
* \param[in] bUrgent-> to do urgent jobs first
* \return None.
*/
TP_S32 
tpool_submit_job(TPOOL_S *tp, void *(*work)(void*), void *(*after)(void*), void *argv, TP_BOOL bUrgent);

/**
* \ dump thread pool details;
* \param[in] TPOOL_S *
* \return None.
*/
void
tpool_state_dump(TPOOL_S *tp);

#ifdef __cplusplus
}
#endif

#endif

