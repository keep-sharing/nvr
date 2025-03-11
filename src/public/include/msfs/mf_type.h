/*
 * ***************************************************************
 * Filename:        mf_type.h
 * Created at:      2017.05.18
 * Description:     type definition
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MF_TYPE_H__
#define __MF_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "list.h"
#include "mf_error.h"

//#define MSFS_USE_MS_LOG

#include "mssocket/tsar_socket.h"


/*----------------------------------------------*
 * The common data type, will be used in the whole project.*
 *----------------------------------------------*/

typedef unsigned char           MF_U8;
typedef unsigned short          MF_U16;
typedef unsigned int            MF_U32;

typedef char                    MF_S8;
typedef short                   MF_S16;
typedef int                     MF_S32;

typedef float                   MF_FLOAT;
typedef double                  MF_DOUBLE;

typedef int                     MF_PTS;

#ifndef _M_IX86
typedef unsigned long long  MF_U64;
typedef long long           MF_S64;
#else
typedef __int64             MF_U64;
typedef __int64             MF_S64;
#endif

typedef char                    MF_CHAR;
#define MF_VOID                 void

typedef pthread_cond_t          MF_COND_T;
typedef pthread_mutex_t         MF_MUTEX_T;
/*----------------------------------------------*
 * const defination                             *
 *----------------------------------------------*/
typedef enum {
    MF_FALSE = 0,
    MF_TRUE  = 1,
    MF_NO    = 0,
    MF_YES   = 1,
} MF_BOOL;

typedef struct MF_TIME {
    MF_PTS      startTime;
    MF_PTS      endTime;
} MF_TIME;

typedef enum event_e
{
    MSFS_EVENT_NONE                      = 0,
    MSFS_EVENT_DISK_NORMAL               = 1,
    MSFS_EVENT_DISK_FORMAT               = 2,
    MSFS_EVENT_DISK_UNFORMAT             = 3,
    MSFS_EVENT_DISK_ADD                  = 4,
    MSFS_EVENT_DISK_DEL                  = 5,
    MSFS_EVENT_DISK_ERR                  = 6,
    MSFS_EVENT_DISK_REC_PERMIT           = 7,
    MSFS_EVENT_DISK_FULL                 = 8,
    MSFS_EVENT_DISK_BAD_BLOCK            = 9,
    MSFS_EVENT_DISK_OFFLINE              = 10,
    MSFS_EVENT_DISK_ONLINE               = 11,
    MSFS_EVENT_DISK_ALL_OKAY             = 12, // just send once for disk loaded entirely.
    MSFS_EVENT_NO_DISK                   = 13,
    MSFS_EVENT_HAVE_DISK                 = 14,
    MSFS_EVENT_REC_START                 = 15,
    MSFS_EVENT_REC_STOP                  = 16,
    MSFS_EVENT_REC_FAIL                  = 17,
    MSFS_EVENT_REC_SUCCEED               = 18,
    MSFS_EVENT_BOOT_COMPLETE             = 19,
    MSFS_EVENT_DISK_HEAT                 = 20,
    MSFS_EVENT_DISK_MICROTHERM           = 21,
    MSFS_EVENT_DISK_CONNECTION_EXCEPTION = 22,
    MSFS_EVENT_DISK_STRIKE               = 23,
    MSFS_EVENT_DISK_HEALTH_TEMPERATURE_NORMAL   = 24,
    MSFS_EVENT_DISK_HEALTH_STATUS_NORMAL        = 25,
} EVENT_E;

typedef struct mf_frame { //size don't over 224 byte !!!!!
//    struct msfs_frame msFrame;
    MF_S32 ch;                  //channels
    MF_S32 strm_type;           //video & audio & data  ST_VIDEO & ST_AUDIO
    MF_S32 codec_type;          //264&265 | aac & alaw & ulaw
    MF_S32 stream_from;     //remote & local
    MF_S32 stream_format;       //main & sub stream
    //Timestamp
    MF_U64 time_usec;
    MF_U32 time_lower;
    MF_U32 time_upper;

    //data size
    MF_S32 size;

    //video
    MF_S32 frame_type;         // FT_IFRAME &   FT_PFRAME
    MF_S32 frame_rate;
    MF_S32 width;
    MF_S32 height;

    //audio
    MF_S32 sample;
    MF_S32 bps;
    MF_S32 achn;

    //data buff
    void *data;

    MF_S32 decch_changed;
    MF_S32 sid;
    MF_S32 sdk_alarm;
    MF_U32  uid;
    MF_S8   dummy[128];
} mf_frame;

#ifndef NULL
#define NULL    0L
#endif

#define MF_NULL     0L
#define MF_NOTODO       (1)
#define MF_SUCCESS  0
#define MF_FAILURE      (-1)
#define MF_UNKNOWN      (-2)
#define MF_NAS_LOW_CAP  (-3)
#define MF_FILE_NOSPACE (-4)

#define INIT_START_TIME (0x7fffffff)
#define INIT_END_TIME   (0X0)
#define DAY_SECONDS     (24*60*60)

typedef void (*MF_FUNC_PRINT)(char *format, ...);

extern int g_msfs_debug;
extern MF_FUNC_PRINT g_debug_print;

#include "msstd.h"

#define RED             "\033[0;31m"
#define GREEN           "\033[0;32m"
#define YELLOW          "\033[0;33m"
#define BLUE            "\033[1;34m"
#define CLRCOLOR        "\033[0;39m "


//log for msfs debug
#define LOG_DETAIL_PKG_SIZE (384) // reference LOG_PKG_DATA_MAX in msfs_log.h
extern void mf_log_mark_debug(MF_S8 *detail, MF_S32 subtype);

typedef enum DBG_E {
    MF_NONE = 0,
    MF_WAR  = (0x01 << 0),
    MF_INFO = (0x01 << 1),
    MF_DBG  = (0x01 << 2),
    MF_ERR  = (0x01 << 3),
    MF_ALL  = MF_WAR | MF_INFO | MF_DBG,
} DBG_E;

#ifndef MSFS_USE_MS_LOG
#define MFprint(format, ...) \
    do{\
        if(g_debug_print)\
            g_debug_print(format, ##__VA_ARGS__);\
        else \
            printf(format "\n", ##__VA_ARGS__); \
    }while(0)

#define MFshow(format, ...) \
    do{\
        _debug(DEBUG_INF, "[MSFS INFO]", GREEN format CLRCOLOR, ##__VA_ARGS__);\
    }while(0)

#define MFinfo(format, ...) \
    do{\
        if(g_msfs_debug & MF_INFO)\
            _debug(DEBUG_INF, "[MSFS INFO]", GREEN format CLRCOLOR, ##__VA_ARGS__);\
    }while(0)

#define MFdbg(format, ...) \
    do{\
        if(g_msfs_debug & MF_DBG)\
            _debug(DEBUG_INF, "[MSFS DBG]", BLUE format CLRCOLOR, ##__VA_ARGS__);\
    }while(0)

#define MFwarm(format, ...) \
    do{\
        if(g_msfs_debug & MF_WAR)\
            _debug(DEBUG_INF, "[MSFS WARM]", YELLOW format CLRCOLOR, ##__VA_ARGS__);\
    }while(0)

#define MFerr(format, ...) \
    do{\
        _debug(DEBUG_INF, "[MSFS ERR]", RED format CLRCOLOR, ##__VA_ARGS__);\
        char det[LOG_DETAIL_PKG_SIZE] = {0}; \
        snprintf(det, LOG_DETAIL_PKG_SIZE - 1, "[%d==func:%s file:%s line:%d]==" format "\n", \
                 getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        mf_log_mark_debug(det, MF_ERR); \
    }while(0)

#define MFlog(level, format, ...) \
    do{\
        _debug(DEBUG_INF, "[MSFS LOG]", GREEN format CLRCOLOR, ##__VA_ARGS__);\
        char det[LOG_DETAIL_PKG_SIZE] = {0}; \
        char stime[32] = {0};\
        time_to_string_ms(stime);\
        snprintf(det, LOG_DETAIL_PKG_SIZE - 1, "[%s==func:%s file:%s line:%d]==" format "\n", \
                 stime, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        mf_log_mark_debug(det, level); \
    }while(0)
#else
#define MFprint(format, ...) \
    do{\
        if (g_debug_print) {\
            ms_log(TSAR_INFO, "[MSFS] " format, ##__VA_ARGS__);\
        } else {\
            printf(format "\n", ##__VA_ARGS__); \
        }\
    }while(0)

#define MFshow(format, ...) \
    do{\
        if (g_debug_print) {\
            ms_log(TSAR_DEBUG, "[MSFS] " format, ##__VA_ARGS__);\
        } else {\
            _debug(DEBUG_INF, "[MSFS INFO]", GREEN format CLRCOLOR, ##__VA_ARGS__);\
        }\
    }while(0)

#define MFinfo(format, ...) \
    do{\
        if (g_msfs_debug & MF_INFO) {\
            if (g_debug_print) {\
                ms_log(TSAR_INFO, "[MSFS] " format, ##__VA_ARGS__);\
            } else {\
                _debug(DEBUG_INF, "[MSFS INFO]", GREEN format CLRCOLOR, ##__VA_ARGS__);\
            }\
        }\
    }while(0)

#define MFdbg(format, ...) \
    do{\
        if (g_msfs_debug & MF_DBG) {\
            if (g_debug_print) {\
                ms_log(TSAR_DEBUG, "[MSFS] " format, ##__VA_ARGS__);\
            } else {\
                _debug(DEBUG_INF, "[MSFS DBG]", BLUE format CLRCOLOR, ##__VA_ARGS__);\
            }\
        }\
    }while(0)

#define MFwarm(format, ...) \
    do{\
        if (g_msfs_debug & MF_WAR) {\
            if (g_debug_print) {\
                ms_log(TSAR_WARN, "[MSFS] " format, ##__VA_ARGS__);\
            } else {\
                _debug(DEBUG_INF, "[MSFS WARM]", YELLOW format CLRCOLOR, ##__VA_ARGS__);\
            }\
        }\
    }while(0)

#define MFerr(format, ...) \
    do{\
        if (g_debug_print) {\
            ms_log(TSAR_ERR, "[MSFS] " format, ##__VA_ARGS__);\
        } else {\
            _debug(DEBUG_INF, "[MSFS ERR]", RED format CLRCOLOR, ##__VA_ARGS__);\
        }\
        char det[LOG_DETAIL_PKG_SIZE] = {0}; \
        snprintf(det, LOG_DETAIL_PKG_SIZE - 1, "[%d==func:%s file:%s line:%d]==" format "\n", \
                 getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        mf_log_mark_debug(det, MF_ERR); \
    }while(0)

#define MFlog(level, format, ...) \
    do{\
        if (g_debug_print) {\
            ms_log(TSAR_DEBUG, "[MSFS] " format, ##__VA_ARGS__);\
        } else {\
            _debug(DEBUG_INF, "[MSFS LOG]", GREEN format CLRCOLOR, ##__VA_ARGS__);\
        }\
        char det[LOG_DETAIL_PKG_SIZE] = {0}; \
        char stime[32] = {0};\
        time_to_string_ms(stime);\
        snprintf(det, LOG_DETAIL_PKG_SIZE - 1, "[%s==func:%s file:%s line:%d]==" format "\n", \
                 stime, __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        mf_log_mark_debug(det, level); \
    }while(0)

#endif

#define ALIGN_UP(x, a)              ((x+a-1)&(~(a-1)))
#define ALIGN_BACK(x, a)            ((a) * (((x) / (a))))
#define MF_MAX(a,b)                    (((a) > (b)) ? (a) : (b))
#define MF_MIN(a,b)                    (((a) < (b)) ? (a) : (b))
#define MF_ARRAY_SIZE(x)                  (sizeof(x) / sizeof((x)[0]))

#define  MF_POS(ptr, member) ((unsigned long)(&((typeof(*ptr) *)0)->member))
#define MF_USLEEP(us)       select_usleep(us)
//#define MF_USLEEP(us)       usleep(us)

typedef enum {
    MF_FRAME_DATA_TYPE_RAW = 0,
    MF_FRAME_DATA_TYPE_PS = 1,
} MF_FRAME_DATA_TYPE;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MF_TYPE_H__ */


