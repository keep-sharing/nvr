/*! \file PushNotification.h
This file describes AV module APIs for push notification service.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _PUSHNOTIFICATION_H_
#define _PUSHNOTIFICATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "AVCommon.h"

/**
 * \brief Enable push notification service
 *
 * \details This function is used by av device to enable push notification service with the
 *          sepicified push notification settings string.
 *
 * \param key[in] The key used to encrypt/decrypt push notification setting string.
 * \param setting_str[in] The encrypted string that contain the push notification settings.
 *                        This encryptd string can be derived from registed avNotificationSettingChangeFn
 *                        callback when push notification settings changed, and this parameter can be
 *                        NULL if you never get this settings from avNotificationSettingChangeFn before.
 * \param callback[in] Push notification settings change handler callback function
 *
 * \return #AV_ER_NoERROR if enable push notification service successfully
 * \return Error code if return value < 0
 *            - #AV_ER_INVALID_ARG The key or callback is null
 *            - #AV_ER_NOT_INITIALIZE  The AV module is not initialized yet
 *            - #AV_ER_MEM_INSUFF Out of memory
 *
 * \attention This function is not support for av client
 *
 * \see avServPushNotification(), avNotificationSettingChangeFn
 */
AVAPI_API int32_t avServEnablePushNotification(const char *key, const char *setting_str, avNotificationSettingChangeFn callback);

/**
 * \brief AV device push a notification to server
 *
 * \details This function is used by device to push a notification to server
 *          when some event happened and device want to notify AV clients
 *          with event messages.
 *
 * \param notification_obj[in] The json object contains a list of key value pair for push server to generate the push message.
 *                             The key and value should be a string.
 * \param timeout_msec[in] The timeout for this function in unit of millisecond, give 0 means block forever
 *
 * \return #AV_ER_NoERROR if push notification successfully
 * \return Error code if return value < 0
 *            - #AV_ER_INVALID_ARG The notification_obj is null
 *            - #AV_ER_NOT_INITIALIZE  The AV module is not initialized yet
 *            - #AV_ER_TIMEOUT Push notification to server timeout
 *            - #AV_ER_HTTP_ERROR Http error occurred when push notification to server
 *            - #AV_ER_MEM_INSUFF Out of memory
 *
 * \attention (1) Recommended value of timeout: 1000 millisecond ~ 30000 millisecond
 * \attension (2) This function is not support for av client
 *
 * \see avServEnablePushNotification()
 */
AVAPI_API int32_t avServPushNotification(NebulaJsonObject *notification_obj, uint32_t timeout_msec);

#ifdef __cplusplus
}
#endif

#endif /* _PUSHNOTIFICATION_H_ */
