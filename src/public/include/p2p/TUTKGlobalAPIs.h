/*! \file NebulaGlobalAPIs.h
This file describes all the APIs of the IOTC module in IOTC platform.
IOTC module is a kind of data communication modules to provide basic data
transfer among devices and clients.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.

Revision Table

Version     | Name             |Date           |Description
------------|------------------|---------------|-------------------
4.0.0.0     |Terry Liu         |2019-08-07     |+ Add TUTK_SDK_Set_Region, TUTK_SDK_Set_License_Key
 */
 
#ifndef _TUTK_GLOBAL_APIs_H_
#define _TUTK_GLOBAL_APIs_H_

#include <stdint.h>
#include "TUTKGlobalDefs.h"


/* ============================================================================
 * Platform Dependant Macro Definition
 * ============================================================================
 */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* ============================================================================
 * Enumeration Declaration
 * ============================================================================
 */

typedef enum {
    REGION_ALL,
    REGION_CN,
    REGION_EU,
    REGION_US,
    REGION_ASIA,
    REGION_COUNT,
} TUTKRegion;

static const char REGION_STRING_ALL[] = "all";
static const char REGION_STRING_CN[] = "cn";
static const char REGION_STRING_EU[] = "eu";
static const char REGION_STRING_US[] = "us";
static const char REGION_STRING_ASIA[] = "asia";

typedef enum LogLevel {
    LEVEL_VERBOSE = 0,
    LEVEL_DEBUG = 1,
    LEVEL_INFO = 2,
    LEVEL_WARNING = 3,
    LEVEL_ERROR = 4,
    LEVEL_SILENCE = 5,
} logLevel_t;

/**
 * \details The Log configuration
 *
 * \param path [in] The path of log file, NULL = disable Log
 * \param log_level [in] The log message with level log_level or higher would be logged, LEVEL_SILENCE = nothing would be logged
 * \param file_max_size [in] The maximum size of log file in bytes, 0 = unlimited
 * \param file_max_count [in] The maximum number of log file if file_max_size is set, 0 = unlimited
 */
typedef struct LogAttr {
    char* path;
    logLevel_t log_level;
    int32_t file_max_size;
    int32_t file_max_count;
} LogAttr;

/* ============================================================================
 * Error Code Declaration
 * ============================================================================
 */
/** The function is performed successfully. */
#define  TUTK_ER_NoERROR                       0

/** TUTK module(IOTC, Nebula) is already initialized.*/
#define  TUTK_ER_ALREADY_INITIALIZED          -1001

/** The arguments passed to a function is invalid. */
#define  TUTK_ER_INVALID_ARG                  -1002

/** Insufficient memory for allocation */
#define  TUTK_ER_MEM_INSUFFICIENT             -1003

/** The arguments passed to a function is invalid. */
#define  TUTK_ER_INVALID_LICENSE_KEY          -1004

/** License key is not yet loaded to TUTK SDK. */
#define  TUTK_ER_NO_LICENSE_KEY               -1005

/** The feature is not supported. */
#define  TUTK_ER_NOT_SUPPORT                  -1006


/* ============================================================================
 * Function Declaration
 * ============================================================================
 */

/**
 * \brief Set Attribute of log file
 *
 * \param logAttr [in] See #LogAttr
 *
 * \return TUTK_ER_NoERROR on success.
 * \return The value < 0
 *            - TUTK_ER_INVALID_ARG   Invalid input argument.
 */
TUTK_GLOBAL_APIs int32_t TUTK_Set_Log_Attr(LogAttr logAttr);

/**
* \brief Change TUTK Server's region
*
* \param region [in] The TUTK Servers's region to be used.
*
* \attention Need to be called after TUTK_SDK_Set_License_Key and before IOTC_Device_LoginEx, IOTC_Connect_byUIDEx.
*            REGION_COUNT is an invalid value for region
*
* \return TUTK_ER_NoERROR if successfully set master region.
* \return Error code if return value < 0
*           - #TUTK_ER_INVALID_ARG if the region is invalid
**/
TUTK_GLOBAL_APIs_DEPRECATED int32_t TUTK_SDK_Set_Region(TUTKRegion region);

/**
* \brief Change TUTK Server's region
*
* \param region [in] The TUTK Servers's region to be used.
*
* \attention Need to be called after TUTK_SDK_Set_License_Key and before IOTC_Device_LoginEx, IOTC_Connect_byUIDEx.
*
* \return TUTK_ER_NoERROR if successfully set region.
* \return Error code if return value < 0
*           - #TUTK_ER_INVALID_ARG if the region is invalid
**/
TUTK_GLOBAL_APIs int32_t TUTK_SDK_Set_Region_Code(const char* region);

/**
* \brief Change TUTK Server's realm 
*
* \param key [in] The TUTK Server's realm to be used.
*
* \attention Need to be called before IOTC_Initialize2()
*
* \return TUTK_ER_NoERROR if successfully set master realm.
* \return Error code if return value < 0
*           - #TUTK_ER_INVALID_ARG if the region is invalid
*           - #TUTK_ER_INVALID_LICENSE_KEY if the key is invalid
*           - #TUTK_ER_MEM_INSUFFICIENT if the memory alllocation failed
**/
TUTK_GLOBAL_APIs int32_t TUTK_SDK_Set_License_Key(const char* key);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TUTK_GLOBAL_APIs_H_ */
