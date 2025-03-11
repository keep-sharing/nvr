/*
* Copyright (c) 2016 HiSilicon Technologies Co., Ltd.
*
* This program is free software; you can redistribute  it and/or modify it
* under  the terms of  the GNU General Public License as published by the
* Free Software Foundation;  either version 2 of the  License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#ifndef __HI_TYPE_H__
#define __HI_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * *
 *----------------------------------------------*/
/*************************** Structure Definition ****************************/
/** \addtogroup      H_2_1_1 */
/** @{ */  /** <!--*/

typedef unsigned char           HI_U8;
typedef unsigned char           HI_UCHAR;
typedef unsigned short          HI_U16;
typedef unsigned int            HI_U32;

typedef signed char             HI_S8;
typedef short                   HI_S16;
typedef int                     HI_S32;

#ifndef _M_IX86
typedef unsigned long long      HI_U64;
typedef long long               HI_S64;
#else
typedef __int64                 HI_U64;
typedef __int64                 HI_S64;
#endif

typedef char                    HI_CHAR;
typedef char*                   HI_PCHAR;

typedef float                   HI_FLOAT;
typedef double                  HI_DOUBLE;
typedef void                    HI_VOID;

typedef unsigned long           HI_SIZE_T;
typedef unsigned long           HI_LENGTH_T;

typedef HI_U32                  HI_HANDLE;


/*----------------------------------------------*
 *                                              *
 *----------------------------------------------*/
typedef enum {
    HI_FALSE    = 0,
    HI_TRUE     = 1,
} HI_BOOL;

#ifndef NULL
#define NULL             0L
#endif
#define HI_NULL          0L
#define HI_NULL_PTR      0L

#define HI_SUCCESS       0
#define HI_FAILURE       (-1)

#define HI_INVALID_HANDLE (0xffffffff)

#define HI_OS_LINUX      0xabcd
#define HI_OS_WIN32      0xcdef

#ifdef _WIN32
#define HI_OS_TYPE      HI_OS_WIN32
#else
#define __OS_LINUX__
#define HI_OS_TYPE      HI_OS_LINUX
#endif
/** @} */  /** <!-- ==== Structure Definition end ==== */


#ifndef LOG_LEVEL        
/* ..8...*/
typedef enum hiLOG_ERRLEVEL_E
{
    HI_LOG_LEVEL_DEBUG = 0,  /* debug-level                                  */
    HI_LOG_LEVEL_INFO,       /* informational                                */
    HI_LOG_LEVEL_NOTICE,     /* normal but significant condition             */
    HI_LOG_LEVEL_WARNING,    /* warning conditions                           */
    HI_LOG_LEVEL_ERROR,      /* error conditions                             */
    HI_LOG_LEVEL_CRIT,       /* critical conditions                          */
    HI_LOG_LEVEL_ALERT,      /* action must be taken immediately             */
    HI_LOG_LEVEL_FATAL,      /* just for compatibility with previous version */
    HI_LOG_LEVEL_BUTT
} LOG_ERRLEVEL_E;

/******************************************************************************
|----------------------------------------------------------------| 
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            | 
|----------------------------------------------------------------| 
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/
#define HI_ERR_APPID  (0x80UL + 0x20UL)

#define HI_DEF_ERR( mid, level, errid) \
    ( ((HI_ERR_APPID)<<24) | ((mid) << 16 ) | ((level)<<13) | (errid) )

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __HI_TYPE_H__ */

