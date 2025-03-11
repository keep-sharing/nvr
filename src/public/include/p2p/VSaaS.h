/*! \file VSaaS.h
This file describes AV module APIs for VSaaS.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _VSAAS_H_
#define _VSAAS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "AVCommon.h"

typedef enum VSaaSType {
    VSAAS_AWS,
    VSAAS_ALI,
} VSaaSType;

typedef enum VSaaSContractType {
    VSAAS_UNKNOWN_TYPE,
    VSAAS_FULLTIME,
    VSAAS_EVENT,
} VSaaSContractType;

typedef enum VSaaSPullStreamType {
    VSAAS_EVENT_GENERAL = 999,
} VSaaSPullStreamType;

/**
* \details The VSaaS contract information and the video format requested by VSaaS server
*
* \param cb                           [in] The check byte of this structure.
* \param contract_type                [out] The user's contract type of VSaaS.(event, fulltime)
* \param event_recording_max_sec      [out] The max length of event recording video in sec.
* \param video_max_fps                [out] The max fps of video.
* \param recording_max_kbps           [out] The max kbps fo recording.
* \param video_max_high               [out] The max height of video.
* \param video_max_width              [out] two max width of video.
*/
typedef struct VSaaSContractInfo {
    uint32_t cb;
    VSaaSContractType contract_type;
    uint32_t event_recording_max_sec;
    uint32_t video_max_fps;
    uint32_t recording_max_kbps;
    uint32_t video_max_high;
    uint32_t video_max_width;
} VSaaSContractInfo;

/**
 * \details The prototype of storage VSaaS configuration function used for an AV server.
 *          The storage configuration function must be set in avEnableVSaaS.
 *          This function will be called while receiving VSaaS configuration in json format from av client.
 *
 * \param VSaaSconfig [in] The VSaaS configuration that user need to storage.
 *
 * \return 0 if storage configuration successfully.
 *
 * \see avAbilityRequestFn()
 *
 */
typedef void(__stdcall *OnVSaaSConfigChangedCb)(const char *VSaaSconfig);

/**
 * \details The prototype of update VSaaS contractinfo function used for an AV server.
 *          The update contractinfo function must be set in avEnableVSaaS.
 *          This function will be called while receiving VSaaS pullstream message in json format from VSaaS server.
 *
 * \param ContractInfo [in] current VSaaS contractinfo.
 *
 * \return 0 for successfully.
 *
 */
typedef void(__stdcall *OnVSaaSUpdateContractInfoCb)(const VSaaSContractInfo *ContractInfo);

/**
 * \brief After Enabling VSaaS event notification ,device can send notification to server and
 *  ask server to  pull streaming by AVAPI.
 *
 * \details This function is used by device to enable VSaaS server pulling stream.
 *          This function must be called before avInitialize().
 *
 * \param uid [in] The uid of device.
 * \param VSaaSconfig [in] The information that can connect to VSaas server.
 *                         You can get this from the registed OnVSaaSConfigChangedCb.
 *                         This param can be NULL if you never get information by OnVSaaSConfigChangedCb before.
 * \param config_callback [in] This callback function will be called while receiving VSaaSconfig from av client.
 * \param contract_callback [in] This callback function will be called while receiving pullstream message from VSaaS server.
 *
 * \return #AV_ER_NoERROR if enable VSaaS pulling stream successfully.
 * \return Error code if return value < 0
 *          - #AV_ER_MEM_INSUFF out of memory
 *          - #AV_ER_ALREADY_INITIALIZED already calling avInitialize()
 *
 *
 * \attention VSaaSconfig can be NULL if you never get information by OnVSaaSConfigChangedCb before.
 * \attention This function must be called before avInitialize().
 *
 */
AVAPI_API int32_t avEnableVSaaS(const char *uid, const char *VSaaSconfig, OnVSaaSConfigChangedCb config_callback, OnVSaaSUpdateContractInfoCb contract_callback);

/**
 * \brief Sending event notification to TUTK cloud and asking cloud to pull streaming
 *
 * \details This function is used by device to notify TUTK cloud of pulling stream by AVAPI.
 *          This function must be called after avEnableVSaaS() or avEnableVSaaSByNebula().
 *
 * \param attr_obj [in] the information of event which including event type and timestamp.
 * \param timeout_sec [in] timeout of this function.
 *
 * \return #AV_ER_NoERROR if notify TUTK cloud successfully.
 * \return Error code if return value < 0
 *          - #AV_ER_VSAAS_PULLING_NOT_ENABLE not enable VSaaS pulling stream.
 *          - #AV_ER_FAIL_CONNECT_TO_VSAAS  get error from https while connect to TUTK cloud.
 *          - #AV_ER_EXCEED_MAX_SIZE too manay params in attr_obj.
 *
 * \see avEnableVSaaS()
 *
 */
AVAPI_API int32_t avServNotifyCloudRecordStream(NebulaJsonObject *attr_obj, uint32_t timeout_msec, uint32_t *abort_flag);

#ifdef __cplusplus
}
#endif

#endif /* _VSAAS_H_ */
