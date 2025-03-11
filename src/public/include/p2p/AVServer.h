/*! \file AVServer.h
This file describes AV module APIs for server.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _AVSERVER_H_
#define _AVSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "AVCommon.h"

typedef struct st_AvStatus {
    uint32_t AvVersion;
    uint16_t RoundTripTime;
    uint16_t LostRate;
    uint16_t BandWidth;
    uint32_t MinRTT;
    uint32_t LastBw;
    uint32_t LastRtt;
    uint32_t LastCwnd;
    uint32_t InFlight;
} st_AvStatus;

typedef enum _AV_RESET_TARGET {
    RESET_VIDEO = 0,
    RESET_AUDIO,
    RESET_ALL,
} AV_RESET_TARGET;

/**
 * \details The prototype of authentication function used for an AV server.
 *			The authentication function is set when an AV server starts by avServStart2().
 *			The AV server will call back authentication function when an AV
 *			client wants to connect with szViewAccount and szViewAccount
 * \param szViewAccount [in] The view account provided by AV clients for authentication
 * \param szViewPassword [in] The view password provided by AV clients for authentication
 *
 * \return true if authentication passes and false if fails.
 *
 */
typedef int32_t(__stdcall *authFn)(const char *szViewAccount, const char *szViewPassword);

/**
 * \details The prototype of account/password authentication function used for an AV server.
 *          The authentication function is set when an AV server starts by avServStartEx().
 *          The AV server will call back avPasswordAuthFn function when an AV
 *          client wants to connect with account and password
 * \param account [in] The account provided by AV clients for authentication
 * \param password [out] The buffer that server need to fill in the password string for authentication
 * \param password_buf_size [in] The size of password buffer
 *
 * \return 0 if account/password authentication passes.
 *
 */
typedef int32_t(__stdcall *avPasswordAuthFn)(const char *account, char *password, uint32_t password_buf_size);

/**
 * \details The prototype of identity/token authentication function used for an AV server.
 *          The authentication function is set when an AV server starts by avServStartEx().
 *          The AV server will call back avTokenAuthFn function when an AV
 *          client wants to connect with identity and token
 *
 * \param identity [in] The identity provided by AV clients for authentication
 * \param token [out] The buffer that AV server need to fill in the token string for authentication
 * \param token_buf_size [in] The size of token buffer
 *
 * \return 0 if identity/token authentication passes.
 *
 */
typedef int32_t(__stdcall *avTokenAuthFn)(const char *identity, char *token, uint32_t token_buf_size);

/**
 * \details The prototype of token request function used for an AV server.
 *          The token request function is set when an AV server starts by avServStartEx().
 *          The AV server will call back avTokenRequestFn function when an AV client use
 *          avClientRequestTokenWithIdentity() to request a token with a specific identity
 *
 * \param av_index [in] The channel ID of the AV channel who request the token
 * \param identity [in] The identity provided by AV clients for request token
 * \param identity_description [in] The identity description provided by AV clients that describes the identity
 * \param token [out] The buffer that AV server need to fill in the token string
 * \param token_buf_size [in] The buffer size provided by AV client who called #avClientRequestTokenWithIdentity
 *
 * \return The status_code #avClientRequestTokenWithIdentity() receives.
 *
 * \see avClientRequestTokenWithIdentity()
 *
 */
typedef int32_t(__stdcall *avTokenRequestFn)(int32_t av_index,
                                         const char *identity,
                                         const char *identity_description,
                                         char *token, uint32_t token_buf_size);

/**
 * \details The prototype of token delete function used for an AV server.
 *          The token delete function is set when an AV server starts by avServStartEx().
 *          The AV server will call back avTokenDeleteFn function when an AV client use
 *          avClientRequestDeleteTokenWithIdentity() to delete a token with a specific identity
 *
 * \param av_index [in] The channel ID of the AV channel who request to delete token
 * \param identity [in] The identity provided by AV clients for delete token
 *
 * \return 0 if delete token success.
 *
 * \see avClientRequestDeleteTokenWithIdentity()
 *
 */
typedef int32_t(__stdcall *avTokenDeleteFn)(int32_t av_index, const char *identity);

/**
 * \details The prototype of send identity array function used for an AV server.
 *          The send identity array function must be called in avIdentityArrayRequestFn
 *          call back function to send identity array to AV client
 *
 * \param av_index [in] The channel ID of the target AV channel
 * \param identities [in] The identity array to be sent
 * \param identity_count [in] The identity count to be sent
 * \param status_code [in] The status_code #avClientRequestIdentityArray() receives.
 *
 * \return #AV_ER_NoERROR if send identity array successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG AV index is invalid or non-zero identity_count with NULL identities
 *          - #AV_ER_NOT_INITIALIZED
 *
 * \see avIdentityArrayRequestFn
 *
 */
typedef int32_t(__stdcall *avServSendIdentityArray)(int32_t av_index,
                                      const AvIdentity *identities,
                                      uint32_t identity_count,
                                      int32_t status_code);

/**
 * \details The prototype of identity array request function used for an AV server.
 *          The identity array request function is set when an AV server starts by avServStartEx().
 *          The AV server will call back avIdentityArrayRequestFn function when an AV client use
 *          avClientRequestIdentityArray() to request the identity array stored in AV server
 *
 * \param av_index [in] The channel ID of the AV channel who request the identity array
 * \param send_identity_array [in] The function pointer of avServSendIdentityArray function, used for send identity array to AV client.
 *
 * \see avClientRequestIdentityArray(), avServSendIdentityArray
 *
 */
typedef void(__stdcall *avIdentityArrayRequestFn)(int32_t av_index, avServSendIdentityArray send_identity_array);

/**
 * \details The prototype of send ability function used for an AV server.
 *          The send ability function must be called in avAbilityRequestFn
 *          call back function to send ability to AV client
 *
 * \param av_index [in] The channel ID of the target AV channel
 * \param ability [in] The ability buffer to be sent
 * \param ability_size [in] The size of ability
 *
 * \return #AV_ER_NoERROR if valid ablility
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG AV index is invalid or ability is NULL
 *          - #AV_ER_NOT_INITIALIZED
 *
 * \see avAbilityRequestFn()
 *
 */
typedef int32_t (__stdcall *avServSendAbility)(int32_t av_index, const uint8_t *ability, uint32_t ability_size);

/**
 * \details The prototype of ability request function used for an AV server.
 *          The ability request function is set when an AV server starts by avServStartEx().
 *          The AV server will call back avAbilityRequestFn function when an AV client use
 *          avClientRequestServerAbility() to request the ability of AV server
 *
 * \param av_index [in] The channel ID of the AV channel who request the AV server ability
 * \param send_ability [in] The function pointer of avServSendAbility function, used for send server ability to AV client.
 *
 * \see avClientRequestServerAbility()
 *
 */
typedef void(__stdcall *avAbilityRequestFn)(int32_t av_index, avServSendAbility send_ability);

/**
 * \details The prototype of change password request function used for an AV server.
 *          The change password function is set when an AV server starts by avServStartEx().
 *          The AV server will call back avChangePasswordRequestFn function when an AV client use
 *          avClientRequestChangeServerPassword() to request change password.
 *          This function is only supported with IOTC authkey enabled, AV server should verify
 *          the correctness of account and old password provide by AV client,
 *          once the account/password authentication succeed and the new password is applied,
 *          AV server should save the new IOTC auth key, and this new IOTC auth key must be applied
 *          in the next call of IOTC_Device_LoginEx()
 *
 * \param av_index [in] The channel ID of the AV channel who request change password
 * \param account [in] The account provide by AV client
 * \param old_password [in] The old password provide by AV client
 * \param new_password [in] The new password provide by AV client
 * \param new_iotc_authkey [in] The new IOTC auth key provide by AV module
 *
 * \return 0 if change password success, then AV module will apply new IOTC auth key to IOTC login process
 *           immediately, and login to server with this new IOTC key until the AV server restart whole proccess.
 *
 * \see avClientRequestChangeServerPassword()
 *
 */
typedef int32_t(__stdcall *avChangePasswordRequestFn)(
    int32_t av_index,
    const char *account,
    const char *old_password,
    const char *new_password,
    const char *new_iotc_authkey);

typedef int32_t(__stdcall *avRecvJSONCtrlRequestFn)(int32_t av_index, const char *func, const NebulaJsonObject *json_args_obj, NebulaJsonObject **json_response_obj);

/**
 * \brief The prototype of push notification settings change handle callback function.
 *
 * \details This callback function is called when settings of push notification changed.
 *          AV device should have a safekeeping of this setting string and should use
 *          avServEnablePushNotification() to load the settings when AV device restart next time.
 *
 * \param setting_str [in] the encrypted string of push notification settings
 *
 * \see avServEnablePushNotification()
 */
typedef int32_t(__stdcall *avNotificationSettingChangeFn)(const char *setting_str);

/**
 * \details The input configuration of avServStartEx
 *
 * \param cb [in] The check byte of this structure.
 * \param iotc_session_id [in] The session ID of the IOTC session to create AV server
 * \param iotc_channel_id [in] The channel ID of the channel to create AV server
 * \param timeout_sec     [in] The timeout for this function in unit of second.
 *                             Specify it as 0 will make AV server start process wait
 *                             until an AV client connects successfully.
 * \param server_type     [in] The user-defined service type. An AV client will get
 *                             this value when it invokes avClientStart() successfully.
 * \param resend          [in] Enabling the Resend mechanism or not
 * \param security_mode   [in] Security mode with options
 * \param password_auth   [in] The function pointer of password authentication function
 * \param token_auth      [in] The function pointer of token authentication function
 * \param token_request   [in] The function pointer will be called if avClientRequestTokenWithIdentity
 *                             is called in client side to request a token
 * \param token_delete    [in] The function pointer will be called if avClientRequestDeleteTokenWithIdentity
 *                             is called in client side to delete a token
 * \param identity_array_request  [in] The function pointer will be called if avClientRequestIdentityArray
 *                                     is called in client side to request the identity list
 * \param ability_request [in] The function pointer will be called if avClientRequestServerAbility
 *                             is called in client side to request server ability
 * \param change_password_request [in] The function pointer will be called if avClientRequestChangeServerPassword
 *                                     is called in client side to request server change password
 * \param dtls_cipher_suites  [in] Setup DTLS cipher suites. Please use the name of cipher suites 
 *                                 from OpenSSL and ":" to separate each cipher suite. If you are 
 *                                 using mbedtls as 3rd party library, currently, we only support 
 *                                 setting it as "PSK-AES128-CBC-SHA256".
 */
typedef struct AVServStartInConfig {
    uint32_t cb;
    uint32_t iotc_session_id;
    uint8_t  iotc_channel_id;
    uint32_t timeout_sec;
    uint32_t server_type;
    int32_t resend;
    AvSecurityMode security_mode;
    avPasswordAuthFn password_auth;
    avTokenAuthFn token_auth;
    avTokenRequestFn token_request;
    avTokenDeleteFn token_delete;
    avIdentityArrayRequestFn identity_array_request;
    avAbilityRequestFn ability_request;
    avChangePasswordRequestFn change_password_request;
    avRecvJSONCtrlRequestFn json_request;
    const char *dtls_cipher_suites;
} AVServStartInConfig;

typedef const AVServStartInConfig * LPCAVSERV_START_IN_CONFIG;

/**
 * \details The output configuration of avServStartEx
 *
 * \param cb                   [in] The check byte of this structure.
 * \param resend               [out] Resend mechanism is enable or not
 * \param two_way_streaming    [out] two way audio/video streaming mechanism is enabling or not
 *                                   1: enabled, 0: disabled
 * \param auth_type            [out] The connection is authenticated with password or token
 * \param account_or_identity  [out] The client identity or account name
 */

typedef struct AVServStartOutConfig {
    uint32_t cb;
    int32_t resend;
    int32_t two_way_streaming;
    AvAuthType auth_type;
    char account_or_identity[256];
} AVServStartOutConfig;

typedef AVServStartOutConfig * LPAVSERV_START_OUT_CONFIG;


/**
 * \brief Reset the Video or Audio buffer of AV server
 *
 * \details This function returns AV_ER_NoERROR if no error
 *
 * \param avIndex [in] The target avIndex
 * \param eTarget [in] Reset type could be RESET_VDEIO, RESET_AUDIO, RESET_ALL
 * \param Timeout_ms [in] The timeout value in ms to execute this function, set value 0 to wait forever
 *
 * \return This function returns AV_ER_NoERROR if no error
 * \return Error code if return value < 0
 *          - #AV_ER_SERVER_EXIT avServStop() is called before this function is returned
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5000 millisecond
 *            (3) This API can only be used by av server
 *
 */
AVAPI_API int32_t avServResetBuffer(int32_t avIndex, AV_RESET_TARGET eTarget, uint32_t Timeout_ms);

/**
 * \brief Start an AV server
 *
 * \details Start an AV server with user defined authentication function.
 *			Any AV client wanting to connect with this AV server shall
 *			pass the authentication with view account and password.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to create AV server
 * \param pfxAuthFn [in] The function pointer to an authentication function
 * \param nTimeout [in] The timeout for this function in unit of second.
 *						Specify it as 0 will make AV server start process wait
 *						until an AV client connects successfully.
 * \param nServType [in] The user-defined service type. An AV client will get
 *						this value when it invokes avClientStart() successfully.
 * \param nIOTCChannelID [in] The channel ID of the channel to create AV server
 *
 * \return AV channel ID if return value >= 0
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The IOTC session ID is incorrect or reuse of IOTC session ID
                and IOTC channel ID.
 *			- #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *			- #AV_ER_EXCEED_MAX_CHANNEL The number of AV channels has reached maximum
 *			- #AV_ER_FAIL_CREATE_THREAD Fails to create threads
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_INVALID_SID The IOTC session ID is not valid
 *			- #AV_ER_SERVER_EXIT Users stop this function with avServExit() in another thread
 *			- #AV_ER_TIMEOUT The timeout specified by nTimeout expires before
 *				AV start is performed completely
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_CHANNEL_IN_USED the IOTC channel has been used by another av channel,
 *				please check if the IOTC channel is used correctly
 *
 * \see avServExit(), avServStop()
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *			  (3) The IOTC channel of specified channel ID will be turned on automatically
 *				  by avServStart2()
 *            (4) This API has been deprecated and might be removed in the next version, please replace it with avServStartEX
 *
 */
AVAPI_API_DEPRECATED int32_t avServStart2(int32_t nIOTCSessionID, authFn pfxAuthFn, uint32_t nTimeout, uint32_t nServType, uint8_t nIOTCChannelID);

/**
 * \brief Start an AV server
 *
 * \details Start an AV re-send supported server with user defined authentication function.
 *			Any AV client wanting to connect with this AV server shall
 *			pass the authentication with view account and password. Whether the re-send mechanism
 *          is enabled or not depends on AV client settings and will set the result into
 *          pnResend parameter.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to create AV server
 * \param pfxAuthFn [in] The function pointer to an authentication function
 * \param nTimeout [in] The timeout for this function in unit of second.
 *						Specify it as 0 will make AV server start process wait
 *						until an AV client connects successfully.
 * \param nServType [in] The user-defined service type. An AV client will get
 *                       this value when it invokes avClientStart() successfully.
 * \param nIOTCChannelID [in] The channel ID of the channel to create AV server
 *
 * \param pnResend [out] Set the re-send is enabled or not.
 *
 * \return AV channel ID if return value >= 0
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The IOTC session ID is incorrect or reuse of IOTC session ID
 *               and IOTC channel ID.
 *			- #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *			- #AV_ER_EXCEED_MAX_CHANNEL The number of AV channels has reached maximum
 *			- #AV_ER_FAIL_CREATE_THREAD Fails to create threads
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_INVALID_SID The IOTC session ID is not valid
 *			- #AV_ER_SERVER_EXIT Users stop this function with avServExit() in another thread
 *			- #AV_ER_TIMEOUT The timeout specified by nTimeout expires before
 *				AV start is performed completely
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_CHANNEL_IN_USED the IOTC channel has been used by another av channel,
 *				please check if the IOTC channel is used correctly
 *
 * \see avServStart2(), avServExit(), avServStop()
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *			  (3) The IOTC channel of specified channel ID will be turned on automatically
 *			 	  by avServStart3()
 *            (4) This API has been deprecated and might be removed in the next version, please replace it with avServStartEX
 *
 */
AVAPI_API_DEPRECATED int32_t avServStart3(int32_t nIOTCSessionID, authFn pfxAuthFn, uint32_t nTimeout, uint32_t nServType, uint8_t nIOTCChannelID, int32_t *pnResend);

/**
 * \brief Used by an AV server exit avServStart2() process
 *
 * \details Since avServStart2() is block processes and
 *			that means the caller has to wait for AV start or specified timeout
 *			expires	before these two functions return. In some cases,
 *			caller may want	to exit AV start process immediately by this
 *			function in another thread.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to exit AV start process
 * \param nIOTCChannelID [in] The channel ID of the channel to exit AV start process
 *
 */
AVAPI_API void avServExit(int32_t nIOTCSessionID, uint8_t nIOTCChannelID);

/**
 * \brief Stop an AV server
 *
 * \details An AV server stop AV channel by this function if this channel is
 *			no longer required.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be stopped
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) This API can only be used by av server
 */
AVAPI_API void avServStop(int32_t nAVChannelID);

/**
 * \brief Set re-send buffer size.
 *
 * \details Use this API to set the re-send buffer size if re-send mechanism is enabled.
 *          Default re-send buffer size is 256KB and recommend size is 1 second data.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be set.
 * \param nSize [in] The size of re-send buffer, in unit of kilo-byte.
 *
 * \attention (1) This API can only be used by av server
 */
AVAPI_API void avServSetResendSize(int32_t nAVChannelID, uint32_t nSize);

/**
* \brief Get re-send buffer size.
*
* \details Use this API to get the re-send buffer size if re-send mechanism is enabled.
*
* \param nAVChannelID [in] The channel ID of the AV channel to be set.
* \param pnSize [out] The size of re-send buffer, in unit of kilo-byte.
*
* \return #AV_ER_NoERROR if sending successfully
* \return Error code if return value < 0
*			- #AV_ER_NOT_INITIALIZED AV module is not initialized yet
*			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
*
* \attention (1) This API can only be used by av server
*/
AVAPI_API int32_t avServGetResendSize(int32_t avIndex, uint32_t *pnSize);

/**
* \brief Get frame count of re-send buffer.
*
* \details Use this API to get the frame count of re-send buffer if re-send mechanism is enabled.
*
* \param nAVChannelID [in] The channel ID of the AV channel to be set.
* \param pnCount [out] The frame count of re-send buffer.
*
* \return #AV_ER_NoERROR if sending successfully
* \return Error code if return value < 0
*			- #AV_ER_NOT_INITIALIZED AV module is not initialized yet
*			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
*			- #AV_ER_NOT_SUPPORT New re-send protocal not support this function
*
* \attention (1) This API can only be used by av server
*/
AVAPI_API int32_t avServGetResendFrmCount(int32_t avIndex, uint32_t *pnCount);

/**
 * \brief Set interval of sending video data in AV server
 *
 * \details An AV server how to send video data to AV client.
 *			It determined the delay time at regular interval between how many
 *			packets. The default value of nPacketNum is 1 and nDelayMs is 0,
 *			so the default behavior is to send frame packets without any delay.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be sent
 * \param nPacketNum [in] How many number of packet as a regular interval
 * \param nDelayMs [in] Delay time in unit of million-second
 *
 * \return #AV_ER_NoERROR if set successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid
 *          - #AV_ER_NOT_SUPPORT New re-send protocal not support this function
 *
 *
 */
AVAPI_API int32_t avServSetDelayInterval(int32_t nAVChannelID, uint16_t nPacketNum, uint16_t nDelayMs);

/**
 * \brief Used by an AV server, in order to know the usage rate of resend buffer
 *
 * \details AV server calls this function to know the usage rate of resend buffer.
 *          It will return a float value between 0 and 1.
 *          0 means that resend buffer is empty, 1 means that resend buffer is full.
 *          The others mean usage rate.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel
 *
 * \return Usage rate of resend buffer, if return value >= 0.
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV channel ID is not valid
 * \attention (1) This API can only be used by av server.<br><br>
 */
AVAPI_API float avResendBufUsageRate(int32_t nAVChannelID);

/**
 * \brief Start an AV server support re-send and DTLS
 *
 * \details Start an AV re-send and DTLS supported server by providing input configuration and output configuration.
 *          Whether the re-send mechanism is enabled or not depends on AV client settings and will set
 *          the result into output configuration. It'll return Error if server enable DTLS but client don't.
 *
 * \param AVServerInConfig [in] a pointer to structure which store all input parameters
 * \param AVServerOutConfig [out] a pointer to structure which store all output parameters
 *  more detail see AVServStartInConfig and AVServStartOUTConfig
 *
 * \return AV channel ID if return value >= 0
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The IOTC session ID is incorrect or reuse of IOTC session ID
 *             and IOTC channel ID or the pointer of input parameter is NULL.
 *			- #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *			- #AV_ER_EXCEED_MAX_CHANNEL The number of AV channels has reached maximum
 *			- #AV_ER_FAIL_CREATE_THREAD Fails to create threads
 *			- #AV_ER_SERV_NO_RESPONSE The AV server has no response
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_INVALID_SID The IOTC session ID is not valid
 *			- #AV_ER_TIMEOUT The timeout specified by nTimeout expires before
 *				AV start is performed completely
 *			- #AV_ER_WRONG_VIEWACCorPWD The client fails in authentication due
 *				to incorrect view account or password
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_CHANNEL_IN_USED the IOTC channel has been used by another av channel,
 *				please check if the IOTC channel is used correctly
 *			- #AV_ER_REMOTE_NOT_SUPPORT_DTLS If client does not support DTLS
 *			- #AV_ER_DTLS_WRONG_ACC_PWD If client enable the security mode with DTLS and gives
 *				a wrong account name or password
 *			- #AV_ER_REMOTE_NOT_SUPPORT_DTLS The server fail because client not support DTLS
 *			- #AV_ER_DTLS_WRONG_PWD The server fail in authentication due to incorrect
 *				password with DTLS security mode eanbled
 *			- #AV_ER_DTLS_AUTH_FAIL The server fail in authentication due to the pfxExAuthFn
 *				return value is smaller than 0
 *
 * \see avServStart2(), avServStart3(), avServExit(), avServStop()
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 5 secs
 *			  (3) The IOTC channel of specified channel ID will be turned on automatically
 *			 	  by avNewServStart()
 *
 */

AVAPI_API int32_t avServStartEx(LPCAVSERV_START_IN_CONFIG AVServerInConfig, LPAVSERV_START_OUT_CONFIG AVServerOutConfig);

/**
 * \brief Get status of the connected AV channel
 *
 * \details An AV server may use this function to get information of the connected AV channel
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be checked
 * \param psAvStatus [out] The connection info of specified AV channel
 *
 * \return #AV_ER_NoERROR if getting the information successfully
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV channel ID is not valid or the input pointer is NULL
 *          - #AV_ER_NOT_SUPPORT The AV server does not support new re-send protocal
 *          - #AV_ER_CLIENT_NOT_SUPPORT Not supported for AV client
 *          - #AV_ER_NOT_INITIALIZED AV module is not initialized yet
 *
 */
AVAPI_API int32_t avStatusCheck(int32_t nAVChannelID, st_AvStatus *psAvStatus);

#ifdef __cplusplus
}
#endif

#endif /* _AVSERVER_H_ */
