/*! \file DASA.h
This file describes AV module APIs for DASA(Dynamic Adaptive Streaming over AVAPI).

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _DASA_H_
#define _DASA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "AVCommon.h"

typedef enum _AV_DASA_LEVEL_ {
    AV_DASA_LEVEL_QUALITY_HIGH          = 0,
    AV_DASA_LEVEL_QUALITY_BTWHIGHNORMAL = 1,
    AV_DASA_LEVEL_QUALITY_NORMAL        = 2,
    AV_DASA_LEVEL_QUALITY_BTWNORMALLOW  = 3,
    AV_DASA_LEVEL_QUALITY_LOW           = 4,
} AV_DASA_LEVEL;

/**
 * \brief Set DASA Parameter
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be enabled DASA
 * \param nEnable [in] Set 1 to enable DASA, 0 to disable DASA
 * \param nCleanBufferCondition [in] Set clean buffer condition in secs, must greater than or equal to 3 and smaller than or equal to 9
 * \param nCleanBufferRatio [in] Set clean buffer ratio, must greater than or equal to 50 and smaller than or equal to 100
 * \param nAdjustmentKeepTime [in] Set increase AV_DASA_LEVEL quality keep time, must bigger than or equal to 5 secs
 * \param nIncreaseQualityCond [in] Set accumulation network stable time for increasing AV_DASA_LEVEL quality, must bigger than or equal to 3 secs
 * \param nDecreaseRatio [in] Set decrease AV_DASA_LEVEL quality ratio, set nDecreaseRatio smaller will trigger decreasing more offen (1~100).
 * \param nInitLevel [in] Set initial quality level
 *
 * \return #AV_ER_NoERROR if setting successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV channel ID is not valid or not support resend
 *          - #AV_ER_CLIENT_NOT_SUPPORT An AV client uses this function
 *          - #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 */
AVAPI_API int32_t avDASASetting(int32_t nAVChannelID, int32_t nEnable, int32_t nCleanBufferCondition, int32_t nCleanBufferRatio, int32_t nAdjustmentKeepTime, int32_t nIncreaseQualityCond, int32_t nDecreaseRatio, AV_DASA_LEVEL nInitLevel);

/**
 * \brief DASA Status Check
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be checked
 *
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV channel ID is not valid or not support resend
 *          - #AV_ER_CLIENT_NOT_SUPPORT An AV client uses this function
 *          - #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *
 * \return #AV_DASA_LEVEL
 *	AV_DASA_LEVEL_QUALITY_HIGH   : Set Video Quality High
 *	AV_DASA_LEVEL_QUALITY_NORMAL : Set Video Quality Normal
 *	AV_DASA_LEVEL_QUALITY_LOW    : Set Video Quality Low
 */
AVAPI_API int32_t avDASACheck(int32_t nAVChannelID);

/**
 * \brief DASA Status Reset
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be reset
 * \param nLevel [in] Set reset quality level
 *
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV channel ID is not valid or not support resend
 *          - #AV_ER_CLIENT_NOT_SUPPORT An AV client uses this function
 *          - #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *
 * \return #AV_ER_NoERROR if setting successfully
 */
AVAPI_API int32_t avDASAReset(int32_t nAVChannelID, AV_DASA_LEVEL nLevel);

#ifdef __cplusplus
}
#endif

#endif /* _DASA_H_ */
