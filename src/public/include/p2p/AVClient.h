/*! \file AVClient.h
This file describes AV module APIs for client.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _AVCLIENT_H_
#define _AVCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "AVCommon.h"

/**
 * \details The input configuration of avClientStartEx
 *
 * \param cb                  [in] The check byte of this structur.
 * \param iotc_session_id     [in] The session ID of the IOTC session to create AV server
 * \param iotc_channel_id     [in] The channel ID of the channel to create AV server

 * \param timeout_sec         [in] The timeout for this function in unit of second.
 *                                 Specify it as 0 will make AV server start process wait
 *                                 until an AV client connects successfully.
 * \param account_or_identity [in] The predefined view account
 * \param password_or_token   [in] The predefined view password, the maximum length of token is 1024
 * \param resend              [in] Enabling the Resend mechanism or not
 * \param security_mode       [in] Enabling the DTLS mechanism  or not
 * \param auth_type           [in] Choose the authentication mode for av connection
 * \param sync_recv_data      [in] Enabling synchronize receive video/audio data mechanism or not
 * \param dtls_cipher_suites  [in] Setup DTLS cipher suites. Please use the name of cipher suites 
 *                                 from OpenSSL and ":" to separate each cipher suite. If you are 
 *                                 using mbedtls as 3rd party library, currently, we only support 
 *                                 setting it as "PSK-AES128-CBC-SHA256".
 */
typedef struct AVClientStartInConfig {
    uint32_t cb;
    uint32_t iotc_session_id;
    uint8_t iotc_channel_id;
    uint32_t timeout_sec;
    const char* account_or_identity;
    const char* password_or_token;
    int32_t resend;
    AvSecurityMode security_mode;
    AvAuthType auth_type;
    int32_t sync_recv_data;
    const char *dtls_cipher_suites;
} AVClientStartInConfig;

typedef const AVClientStartInConfig * LPCAVCLIENT_START_IN_CONFIG;

/**
 * \details The output configuration of avClientStartEx
 *
 * \param cb                [in] The check byte of this structure.
 * \param server_type       [out] The user-defined service type set when an AV server
 *                                starts. Can be NULL.
 * \param resend            [out] Resend mechanism is enabling or not
 * \param two_way_streaming [out] two way audio/video streaming mechanism is enabling or not
 * \param sync_recv_data    [out] Synchronously receive video/audio data is enabling or not
 * \param security_mode     [out] The DTLS mechanism is enabling or not
*/
typedef struct AVClientStartOutConfig {
    uint32_t cb;
    uint32_t server_type;
    int32_t resend;
    int32_t two_way_streaming;
    int32_t sync_recv_data;
    AvSecurityMode security_mode;
} AVClientStartOutConfig;

typedef AVClientStartOutConfig * LPAVCLIENT_START_OUT_CONFIG;

/**
 * \brief Start an AV client
 *
 * \details Start an AV client by providing view account and password.
 *			It shall pass the authentication of the AV server before receiving
 *			AV data.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to start AV client
 * \param cszViewAccount [in] The view account for authentication
 * \param cszViewPassword [in] The view password for authentication
 * \param nTimeout [in] The timeout for this function in unit of second
 *						Specify it as 0 will make this AV client try connection
 *						once and this process will exit immediately.
 * \param pnServType [out] The user-defined service type set when an AV server
 *						starts. Can be NULL.
 * \param nIOTCChannelID [in] The channel ID of the channel to start AV client
 *
 * \return AV channel ID if return value >= 0
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The IOTC session ID is incorrect or reuse of IOTC session ID
 *              and IOTC channel ID.
 *			- #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *			- #AV_ER_EXCEED_MAX_CHANNEL The number of AV channels has reached maximum
 *			- #AV_ER_FAIL_CREATE_THREAD Fails to create threads
 *			- #AV_ER_SERV_NO_RESPONSE The AV server has no response
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_INVALID_SID The IOTC session ID is not valid
 *			- #AV_ER_CLIENT_EXIT Users stop this function with avClientExit() in another thread
 *			- #AV_ER_TIMEOUT The timeout specified by nTimeout expires before
 *				AV start is performed completely
 *			- #AV_ER_WRONG_VIEWACCorPWD The client fails in authentication due
 *				to incorrect view account or password
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_CHANNEL_IN_USED the IOTC channel has been used by another av channel,
 *				please check if the IOTC channel is used correctly
 *
 * \see avClientStop(), avClientExit()
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *			  (3) The IOTC channel of specified channel ID will be turned on automatically
 *				  by avClientStart().
 *            (4) This API has been deprecated and might be removed in the next version, please replace it with avClientStartEx
 *
 */
AVAPI_API_DEPRECATED int32_t avClientStart(int32_t nIOTCSessionID, const char *cszViewAccount, const char *cszViewPassword,
                            uint32_t nTimeout, uint32_t *pnServType, uint8_t nIOTCChannelID);

/**
 * \brief Start an AV client
 *
 * \details Start an AV re-send supported client by providing view account and password.
 *			It shall pass the authentication of the AV server before receiving
 *			AV data. Whether the re-send mechanism is enabled or not depends on AV server settings
 *          and will set the result into pnResend parameter.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to start AV client
 * \param cszViewAccount [in] The view account for authentication
 * \param cszViewPassword [in] The view password for authentication
 * \param nTimeout [in] The timeout for this function in unit of second
 *						Specify it as 0 will make this AV client try connection
 *						once and this process will exit immediately.
 * \param pnServType [out] The user-defined service type set when an AV server
 *						starts. Can be NULL.
 * \param nIOTCChannelID [in] The channel ID of the channel to start AV client
 * \param pnResend [out] The re-send is enabled or not.
 *
 * \return AV channel ID if return value >= 0
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The IOTC session ID is incorrect or reuse of IOTC session ID
                and IOTC channel ID.
 *			- #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *			- #AV_ER_EXCEED_MAX_CHANNEL The number of AV channels has reached maximum
 *			- #AV_ER_FAIL_CREATE_THREAD Fails to create threads
 *			- #AV_ER_SERV_NO_RESPONSE The AV server has no response
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_INVALID_SID The IOTC session ID is not valid
 *			- #AV_ER_CLIENT_EXIT Users stop this function with avClientExit() in another thread
 *			- #AV_ER_TIMEOUT The timeout specified by nTimeout expires before
 *				AV start is performed completely
 *			- #AV_ER_WRONG_VIEWACCorPWD The client fails in authentication due
 *				to incorrect view account or password
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_CHANNEL_IN_USED the IOTC channel has been used by another av channel,
 *				please check if the IOTC channel is used correctly
 *
 * \see avClientStop(), avClientExit()
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *			  (3) The IOTC channel of specified channel ID will be turned on automatically
 *				  by avClientStart2().<br><br>
 *            (4) If AV client uses avClientStart2() to enable AV re-send mechanism, it has
 *                to use avRecvFrameData2() to receive video data.
 *            (5) This API has been deprecated and might be removed in the next version, please replace it with avClientStartEx
 *
 */
AVAPI_API_DEPRECATED int32_t avClientStart2(int32_t nIOTCSessionID, const char *cszViewAccount, const char *cszViewPassword,
                             uint32_t nTimeout, uint32_t *pnServType, uint8_t nIOTCChannelID, int32_t *pnResend);

/**
 * \brief Used by an AV client exit avClientStart() process
 *
 * \details Since avClientStart() is a block process and
 *			that means the caller has to wait for AV start or specified timeout
 *			expires	before these two functions return. In some cases,
 *			caller may want	to exit AV start process immediately by this
 *			function in another thread.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to exit AV start process
 * \param nIOTCChannelID [in] The channel ID of the channel to exit AV start process
 *
 */
AVAPI_API void avClientExit(int32_t nIOTCSessionID, uint8_t nIOTCChannelID);

/**
 * \brief Stop an AV client
 *
 * \details An AV client stop AV channel by this function if this channel is
 *			no longer required.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be stopped
 *
 * \attention (1) This API is a blocking function.<br><br>
 */
AVAPI_API void avClientStop(int32_t nAVChannelID);

/**
 * \brief Get the frame count of audio buffer remaining in the queue
 *
 * \details An AV client uses this function to get the frame count of audio buffer
 *			that is still remaining in the receiving queue, then determine
 *			whether it is a right time to invoke avRecvAudioData().
 *			Keeping audio buffer in audio queue instead of receiving immediately
 *			can greatly improve the discontinuous audio issues. However,
 *			it is not good to receive audio in a very long time since audio
 *			buffer will overflow and audio data will be lost.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to check audio buffer
 *
 * \return The frame count of audio buffer
 */
AVAPI_API_DEPRECATED int32_t avCheckAudioBuf(int32_t nAVChannelID);

/**
 * \brief Set the maximum video frame buffer used in AV client
 *
 * \details AV client sets the maximum video frame buffer by this function.
 *			The size of video frame buffer will affect the streaming fluency.
 *			The default size of video frame buffer is 1MB.
 *
 * \param nMaxBufSize The maximum video frame buffer, in unit of kilo-byte
 *
 */
AVAPI_API_DEPRECATED void avClientSetMaxBufSize(uint32_t nMaxBufSize);

/**
 * \brief Set the maximum frame buffer size used in AV client with specific AV channel ID
 *
 * \details AV client sets the maximum frame buffer size by this function.
 *			The size of frame buffer will affect the streaming fluency.
 *			The default size of frame buffer is 1MB.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to setup max buffer size
 * \param nMaxBufSizeKB The maximum frame buffer size, in unit of kilo-byte
 *
 */
AVAPI_API int32_t avClientSetRecvBufMaxSize(int32_t nAVChannelID, uint32_t nMaxBufSizeKB);

/**
 * \brief Clean the video buffer both in client and device, and clean the audio buffer of the client.
 *
 * \details A client with multiple device connection application should call
 *			this function to clean AV buffer while switch to another devices.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to clean buffer
 *
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
 *          - #AV_ER_CLIENT_EXIT avClientStop() is called before this function is returned
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) This API can only be used by av client
 *
 */
AVAPI_API int32_t avClientCleanBuf(int32_t nAVChannelID);

/**
 * \brief Clean the video buffer(both in client and device)
 *        When new re-send protocal is using(avNewServStart & avNewClientStart), this function will clean both video and audio buffer
 *
 * \details A client with multiple device connection application should call
 *			this function to clean video buffer while switch to another devices.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to clean buffer
 *
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) This API can only be used by av client
 *
 */
AVAPI_API int32_t avClientCleanVideoBuf(int32_t nAVChannelID);

/**
* \brief Clean the local video and audio buffer of the client
*
* \details This function is used to clean the video and audio buffer that the client
*			has already received
*
* \param nAVChannelID [in] The channel ID of the AV channel to clean buffer
*
* \return Error code if return value < 0
*			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
* \attention (1) This API can only be used by av client.<br><br>
*/
AVAPI_API int32_t avClientCleanLocalBuf(int32_t nAVChannelID);

/**
* \brief Clean the local video buffer of the client
*
* \details This function is used to clean the video buffer that the client
*			has already received
*
* \param nAVChannelID [in] The channel ID of the AV channel to clean buffer
*
* \return Error code if return value < 0
*			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
*           - #AV_ER_NOT_SUPPORT New re-send protocal not support this function
* \attention (1) This API can only be used by av client.<br><br>
*/
AVAPI_API int32_t avClientCleanLocalVideoBuf(int32_t nAVChannelID);

/**
 * \brief Clean the local audio buffer of the client
 *
 * \details A client with multiple device connection application should call
 *			this function to clean audio buffer while switch listen to speaker.
 *
 * \param nAVChannelID [in] The channel ID of the audio channel to clean buffer
 *
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
 *          - #AV_ER_NOT_SUPPORT New re-send protocal not support this function
 * \attention (1) This API can only be used by av client.<br><br>
 */
AVAPI_API int32_t avClientCleanAudioBuf(int32_t nAVChannelID);

/**
 * \brief Used by an AV client, in order to know the usage rate of receive buffer
 *
 * \details AV client calls this function to know the usage rate of receive buffer.
 *          It will return a float value between 0 and 1.
 *          0 means that receive buffer is empty, 1 means that receive buffer is full.
 *          The others mean usage rate.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel
 *
 * \return Usage rate of receive buffer, if return value >= 0.
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV channel ID is not valid
 * \attention (1) This API can only be used by av client.<br><br>
 */
AVAPI_API float avClientRecvBufUsageRate(int32_t nAVChannelID);

/**
 * \brief Start an AV client
 *
 * \details Start an AV re-send and DTLS supported client by providing input configuration and output configuration.
 *            It shall pass the authentication of the AV server before receiving
 *            AV data. Whether the re-send mechanism is enabled or not depends on AV server settings
 *            and will set the result into pnResend parameter in LPAVSERV_START_OUT_CONFIG.
 *            It'll return Error if client enable DTLS but server don't.
 * \param AVClientInConfig [in] a pointer to structure which store all input parameters
 * \param AVClientOutConfig [out] a pointer to structure which store all output parameters
 *  more detail see AVClientStartInConfig and AVClientStartOUTConfig
 *
 * \return AV channel ID if return value >= 0
 * \return Error code if return value < 0
 *            - #AV_ER_INVALID_ARG The IOTC session ID is incorrect or reuse of IOTC session ID
                and IOTC channel ID.
 *            - #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *            - #AV_ER_EXCEED_MAX_CHANNEL The number of AV channels has reached maximum
 *            - #AV_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #AV_ER_SERV_NO_RESPONSE The AV server has no response
 *            - #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *                this IOTC session
 *            - #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *                remote site has no response.
 *            - #AV_ER_INVALID_SID The IOTC session ID is not valid
 *            - #AV_ER_CLIENT_EXIT Users stop this function with avClientExit() in another thread
 *            - #AV_ER_TIMEOUT The timeout specified by nTimeout expires before
 *                AV start is performed completely
 *            - #AV_ER_WRONG_VIEWACCorPWD The client fails in authentication due
 *                to incorrect view account or password
 *            - #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *            - #AV_ER_IOTC_CHANNEL_IN_USED the IOTC channel has been used by another av channel,
 *                please check if the IOTC channel is used correctly
 *
 * \see avClientStop(), avClientExit()
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *            (3) The IOTC channel of specified channel ID will be turned on automatically
 *                by avClientStartEx().<br><br>
 *
 */
AVAPI_API int32_t avClientStartEx(LPCAVCLIENT_START_IN_CONFIG AVClientInConfig, LPAVCLIENT_START_OUT_CONFIG AVClientOutConfig);

/**
 * \brief Get token from AV server by identity
 *
 * \details This function is used by AV client to request a token from AV server with a specific identity
 *
 * \param av_index [in] The channel ID of the AV channel to send token request
 * \param identity [in] The identity to be sent
 * \param identity_description [in] The description of identity to be sent
 * \param token [out] The buffer of the token to be received
 * \param token_buff_size [in] The max size of token buffer
 * \param status_code [out] The status code returned from AV client
 * \param timeout_sec [in] The timeout for this function in unit of second, blocks forever if 0
 *
 * \return The token length received from #avTokenRequestFn()
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG
 *          - #AV_ER_NOT_INITIALIZED
 *          - #AV_ER_BUFPARA_MAXSIZE_INSUFF The size of token buffer is smaller than the size of token sent from AV server
 *          - #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *          - #AV_ER_REMOTE_TIMEOUT_DISCONNECT The remote site has no response
 *          - #AV_ER_CLIENT_EXIT avClientStop() called in another thread
 *          - #AV_ER_TIMEOUT The timeout specified by timeout_sec expires before
 *              request token process is performed completely
 *          - #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_REMOTE_NOT_SUPPORT Server does not support token request
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *
 * \see avTokenRequestFn
 *
 */
AVAPI_API int32_t avClientRequestTokenWithIdentity(int32_t av_index,
                                               const char *identity,
                                               const char *identity_description,
                                               char *token, uint32_t token_buff_size,
                                               int32_t *status_code,
                                               uint32_t timeout_sec);

/**
 * \brief Delete token of AV server by identity
 *
 * \details This function is used by AV client to delete token of AV server with a specific identity
 *
 * \param av_index [in] The channel ID of the AV channel to send delete token request
 * \param identity [in] The identity to be sent
 * \param status_code [out] The status code returned from AV client
 * \param timeout_sec [in] The timeout for this function in unit of second, blocks forever if 0
 *
 * \return #AV_ER_NoERROR if delete token successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG
 *          - #AV_ER_NOT_INITIALIZED
 *          - #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *          - #AV_ER_REMOTE_TIMEOUT_DISCONNECT The remote site has no response
 *          - #AV_ER_CLIENT_EXIT avClientStop() called in another thread
 *          - #AV_ER_TIMEOUT The timeout specified by timeout_sec expires before
 *              delete token process is performed completely
 *          - #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_REMOTE_NOT_SUPPORT Server does not support delete token request
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *
 * \see avTokenDeleteFn
 *
 */
AVAPI_API int32_t avClientRequestDeleteTokenWithIdentity(int32_t av_index,
                                            const char *identity,
                                            int32_t *status_code,
                                            uint32_t timeout_sec);

/**
 * \brief AV Client get identity list from AV server
 *
 * \details This function is used by AV client to request identity list from AV server
 *
 * \param av_index [in] The channel ID of the AV channel to send idenetity list request
 * \param identities [out] The buffer of the identity list to be received
 * \param identity_count [out] The identity conut of identity list
 * \param status_code [out] The status code returned from AV client
 * \param timeout_sec [in] The timeout for this function in unit of second, blocks forever if 0
 *
 * \return #AV_ER_NoERROR if get identity list successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG
 *          - #AV_ER_NOT_INITIALIZED
 *          - #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *          - #AV_ER_REMOTE_TIMEOUT_DISCONNECT The remote site has no response
 *          - #AV_ER_CLIENT_EXIT avClientStop() called in another thread
 *          - #AV_ER_TIMEOUT The timeout specified by timeout_sec expires before
 *              request identity list process is performed completely
 *          - #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *          - #AV_ER_MEM_INSUFF Insufficient memory for allocation
 *			- #AV_ER_REMOTE_NOT_SUPPORT Server does not support identity list request
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *
 * \see avServSendIdentityArray, avIdentityArrayRequestFn
 *
 */
AVAPI_API int32_t avClientRequestIdentityArray(int32_t av_index,
                                           AvIdentity **identities,
                                           uint32_t *identity_count,
                                           int32_t *status_code,
                                           uint32_t timeout_sec);

/**
 * \brief AV Client free the identity list buffer
 *
 * \details This function is used by AV client to free the memory allocated by AV module.
 *          Once the AV client use avClientRequestIdentityArray() to request identity list from AV server,
 *          AV module will allocate a memory block to store the identity list.
 *          After a success call of avClientRequestIdentityArray(), AV client must use this function
 *          to free the memory, otherwise the memory leak will occur
 *
 * \param identities [in] The identity list buffer allocated by AV module to be free
 *
 * \return #AV_ER_NoERROR if memory free successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG identities is NULL
 *
 * \see avClientRequestIdentityArray()
 *
 */
AVAPI_API int32_t avClientFreeIdentityArray(AvIdentity *identities);

/**
 * \brief AV Client request ability from server
 *
 * \details This function is used by AV client to request ability from AV server
 *
 * \param av_index [in] The channel ID of the AV channel to send ability request
 * \param ability [out] The buffer of the ability to be received
 * \param ability_size [out] The size of ability to be received
 * \param timeout_sec [in] The timeout for this function in unit of second, blocks forever if 0
 *
 * \return #AV_ER_NoERROR if getting the server ability successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV index is invalid or ability is NULL or ability_size is NULL
 *          - #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *          - #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *          - #AV_ER_REMOTE_TIMEOUT_DISCONNECT The remote site has no response
 *          - #AV_ER_CLIENT_EXIT avClientStop() called in another thread
 *          - #AV_ER_TIMEOUT The timeout specified by timeout_sec expires before
 *              request server ability process is performed completely
 *          - #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *          - #AV_ER_MEM_INSUFF Insufficient memory for allocation
 *			- #AV_ER_REMOTE_NOT_SUPPORT Server does not support ability request
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *
 * \see avServSendAbility, avAbilityRequestFn
 *
 */
AVAPI_API int32_t avClientRequestServerAbility(int32_t av_index,
        uint8_t **ability, uint32_t *ability_size, uint32_t timeout_sec);

/**
 * \brief AV Client free the ability buffer
 *
 * \details This function is used by AV client to free the memory allocated by AV module.
 *          Once the AV client use avClientRequestServerAbility() to request ability from AV server,
 *          AV module will allocate a memory block to store the server ability.
 *          After a success call of avClientRequestServerAbility(), AV client must use this function
 *          to free the memory, otherwise the memory leak will occur
 *
 * \param ability [in] The server ability buffer allocated by AV module to be free
 *
 * \return #AV_ER_NoERROR if memory free successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG ability is NULL
 *
 * \see avClientRequestServerAbility()
 *
 */
AVAPI_API int32_t avClientFreeServerAbility(uint8_t *ability);

/**
 * \brief AV Client request change server password
 *
 * \details This function is used by AV client to request AV server change password.
 *          If AV server change password successfully, IOTC new authkey would force apply on AV server,
 *          then AV client should use new IOTC auth key for the next IOTC connection, and use
 *          new password for AV Connection, otherwise the connection will fail.
 *
 * \param av_index [in] The channel ID of the AV channel to send change password request
 * \param account [in] The account to be sent
 * \param old_password [in] The old password to be sent
 * \param new_password [in] The new password to be sent
 * \param new_iotc_authkey [out] The buffer of new IOTC auth key to be received
 * \param new_iotc_authkey_buff_size [in] The size of new IOTC auth key buffer
 * \param new_iotc_authkey_actual_length [out] The actual length of new IOTC auth key to be received
 * \param timeout_sec [in] The timeout for this function in unit of second, blocks forever if 0
 *
 * \return #AV_ER_NoERROR if getting the server ability successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV index is invalid or ability is NULL or ability_length is NULL
 *          - #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *          - #AV_ER_WRONG_VIEWACCorPWD The client fails in authentication due
 *          - #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *              this IOTC session
 *          - #AV_ER_REMOTE_TIMEOUT_DISCONNECT The remote site has no response
 *          - #AV_ER_CLIENT_EXIT avClientStop() called in another thread
 *          - #AV_ER_TIMEOUT The timeout specified by timeout_sec expires before
 *              change password request process is performed completely
 *          - #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *          - #AV_ER_REMOTE_NOT_SUPPORT Server does not support change password request
 *			- #AV_ER_NOT_SUPPORT Client connect without authkey does not support this function
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *
 * \see avChangePasswordRequestFn
 *
 */
AVAPI_API int32_t avClientRequestChangeServerPassword(
    int32_t av_index,
    const char *account,
    const char *old_password,
    const char *new_password,
    char *new_iotc_authkey,
    uint32_t new_iotc_authkey_buff_size,
    uint32_t *new_iotc_authkey_actual_length,
    uint32_t timeout_sec);

/**
* \brief Send AV JSON control
*
* \details This function is used by AV to send a AV IO control in json format.
*          You can get JSON respones from this API. This function is not support
*          for av server currently.
*
* \param av_index [in] The channel ID of the AV channel to send IO control
* \param json_request [in] The json data that need to send to AV server.
* \param json_response_obj [out] The pointer of av response, please do not allocate memory of this parameter
* \param timeout_sec [in] The timeout for this function in unit of second, blocks forever if 0.
*
* \return #AV_ER_NoERROR if sending successfully
* \return Error code if return value < 0
*            - #AV_ER_INVALID_ARG The AV channel ID is not valid
*            - #AV_ER_SENDIOCTRL_ALREADY_CALLED This AV channel is already in sending
*                IO control process
*            - #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
*                this IOTC session
*            - #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
*                remote site has no response.
*            - #AV_ER_SENDIOCTRL_EXIT avSendIOCtrlExit() is called before this
*                function is returned
*            - #AV_ER_EXCEED_MAX_SIZE The IO control data and type to be sent exceeds
*                #AV_MAX_IOCTRL_DATA_SIZE
*            - #AV_ER_NO_PERMISSION the AV module does not support Lite UID
*            - #AV_ER_IOTC_SESSION_CLOSED the local IOTC session has been closed
*            - #AV_ER_IOTC_DEINITIALIZED IOTC has been deinitialized
*            - #AV_ER_SERVER_EXIT avServStop() is called before this function is returned
*            - #AV_ER_CLIENT_EXIT avClientStop() is called before this function is returned
*
*
* \attention This function is not support for av server now.
* \attention This API is a blocking function and it will wait until
*              having response from the receiver.
*/
AVAPI_API int32_t avSendJSONCtrlRequest(int32_t av_index, const char *json_request, NebulaJsonObject **json_response_obj, uint32_t timeout_sec);

/**
* \brief Release response data of avSendJSONCtrlRequest
*
* \details This function is used to release response data from avSendJSONCtrlRequest,
*          you must call it after you got the JSON response data from avSendJSONCtrlRequest
*
* \param json_response_obj [in] This json response data pointer
*
* \return #AV_ER_NoERROR if sending successfully
* \return Error code if return value < 0
*            - #AV_ER_INVALID_ARG The AV channel ID is not valid
*
* \attention This function is not support for av server now.
*/
AVAPI_API int32_t avFreeJSONCtrlResponse(NebulaJsonObject *json_response_obj);

#ifdef __cplusplus
}
#endif

#endif /* _AVCLIENT_H_ */
