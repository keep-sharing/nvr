/*
 * ***************************************************************
 * Filename:        mf_error.h
 * Created at:      2018.03.08
 * Description:     error code  definition
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MF_ERROR_H__
#define __MF_ERROR_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define MF_ERR_LIB_ID  (0x80000000L + 0x10000000L)

typedef enum MF_MOD_ID_E {
    MF_MOD_ID_DISK = 0,   /*disk */
    MF_MOD_ID_REC,        /*record */
    MF_MOD_ID_RETR,        /*retrieve*/
    MF_MOD_ID_PB,         /*playback */
    MF_MOD_ID_LOG,        /*log*/
    MF_MOD_ID_BUTT,
} MF_MOD_ID_E;

typedef enum MF_ERR_LEVEL_E {
    MF_ERR_LEVEL_DEBUG = 0,  /* debug-level                                  */
    MF_ERR_LEVEL_INFO,       /* informational                                */
    MF_ERR_LEVEL_NOTICE,     /* normal but significant condition             */
    MF_ERR_LEVEL_WARNING,    /* warning conditions                           */
    MF_ERR_LEVEL_ERROR,      /* error conditions                             */
    MF_ERR_LEVEL_CRIT,       /* critical conditions                          */
    MF_ERR_LEVEL_ALERT,      /* action must be taken immediately             */
    MF_ERR_LEVEL_FATAL,      /* just for compatibility with previous version */
    MF_ERR_LEVEL_BUTT
} MF_ERR_LEVEL_E;


/******************************************************************************
|----------------------------------------------------------------|
| 1 |   LIB_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define MF_DEF_ERR( module, level, errid) \
    ((MF_S32)( (MF_ERR_LIB_ID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

/* NOTE! the following defined all common error code,
** all module must reserved 0~63 for their common error code
*/
typedef enum MF_ERR_CODE_E {
    MF_ERR_INVALID_DEVID = 1, /* invlalid device ID                           */
    MF_ERR_INVALID_CHNID = 2, /* invlalid channel ID                          */
    MF_ERR_ILLEGAL_PARAM = 3, /* at lease one parameter is illagal
                                                    * eg, an illegal enumeration value             */
    MF_ERR_EXIST         = 4, /* resource exists                              */
    MF_ERR_UNEXIST       = 5, /* resource unexists                            */

    MF_ERR_NULL_PTR      = 6, /* using a NULL point                           */

    MF_ERR_NOT_CONFIG    = 7, /* try to enable or initialize system, device
                                                    ** or channel, before configing attribute       */

    MF_ERR_NOT_SUPPORT   = 8, /* operation or type is not supported by NOW    */
    MF_ERR_NOT_PERM      = 9, /* operation is not permitted
                                                    ** eg, try to change static attribute           */

    MF_ERR_NOMEM         = 12,/* failure caused by malloc memory              */
    MF_ERR_NOBUF         = 13,/* failure caused by malloc buffer              */

    MF_ERR_BUF_EMPTY     = 14,/* no data in buffer                            */
    MF_ERR_BUF_FULL      = 15,/* no buffer for new data                       */

    MF_ERR_SYS_NOTREADY  = 16,/* System is not ready,maybe not initialed or
                                                    ** loaded. Returning the error code when opening
                                                    ** a device file failed.                        */

    MF_ERR_BADADDR       = 17,/* bad address,
                                                    ** eg. used for copy_from_user & copy_to_user   */

    MF_ERR_BUSY          = 18,/* resource is busy,
                                                    ** eg. destroy a venc chn without unregister it */

    MF_ERR_BUTT          = 63,/* maxium code, private error code of all modules
                                                    ** must be greater than it                      */
} MF_ERR_CODE_E;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MF_TYPE_H__ */


