/* 
 * ***************************************************************
 * Filename:      	msini.h
 * Created at:    	2019.05.23
 * Description:   	ms iniparser api.
 * Author:        	zbing&solin
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __MSINI_H__
#define __MSINI_H__

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
* reference iniparse with solin.
* the key-value of config is case-insensitive.
* 定位一个key是用"section:key"来表示的,即:"sect:key" ->获取sect下key的值
*************************************************************************/
	
#include "msstd.h"

#define MAX_INI_NAME_LEN   (64)

#define MSFS_INI_FILE   "/etc/msfs.conf"

#define MS_INI_SYS_TIMESTAMP "sys:timestamp"
#define MS_INI_SYS_POWERON   "sys:poweron"
#define MS_INI_SYS_SOFTBOOT  "sys:softboot"
#define MS_INI_DBG_LEVEL     "debug:level"

/**
* \brief get string by keyNmae.
* \param[in] key-> [section:keyName]
* \return keyValue for success, Null for failure.
*/
MS_S8 *
ms_ini_getstr(void * ini,MS_S8 * key);


/**
* \brief get int by keyNmae.
* \param[in] ini   
* \param[in] key-> [section:keyName]
* \param[in] def
* \return keyValue for success, def for failure.
*/
MS_S32
ms_ini_getint(void * ini,MS_S8 * key,MS_S32 def);

/**
* \brief get boolean by keyNmae.
* \param[in] ini   
* \param[in] key-> [section:keyName]
* \param[in] def
* \return keyValue for success, def for failure.
*/
MS_S32
ms_ini_getbool(void * ini,MS_S8 * key,MS_S32 def);

/**
* \brief set string by keyNmae.
* \param[in] ini   
* \param[in] entry-> [section:keyName]
* \param[in] val
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
MS_S32
ms_ini_setstr(void * ini,MS_S8 * entry,MS_S8 * val);

/**
* \brief set int by keyNmae.
* \param[in] ini   
* \param[in] entry-> [section:keyName]
* \param[in] val
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
MS_S32
ms_ini_setint(void * ini,MS_S8 * entry,MS_S32 in);

/**
* \brief delete a entry.
* \param[in] ini   
* \param[in] entry-> [section:keyName]
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
void
ms_ini_unset(void *ini, MS_S8 * entry);

/**
* \brief dump all config file.
* \param[in] ini   
* \return None.
*/
void
ms_ini_dump(void * ini);

/**
* \brief save config file.
* \param[in] ini   
* \return None.
*/
void
ms_ini_save(void * ini);

/**
* \brief open ini file.
* \param[in] inifile name
* \return ini pointer ,  MS_NULL for failure.
*/
void *
ms_ini_open(MS_S8 * iniFile);

/**
* \brief close ini file.
* \param[in] ini
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
MS_S32
ms_ini_close(void * ini);

/**
* \brief init api.
* \param[in] None
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
MS_S32
ms_ini_init();

/**
* \brief deinit api.
* \param[in] None
* \return MS_SUCCESS for success, MS_FAILURE for failure.
*/
MS_S32
ms_ini_deinit();

#ifdef __cplusplus
}
#endif

#endif

