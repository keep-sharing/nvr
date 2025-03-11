/*! \file IOTCDevice.h
This file describes IOTC module APIs of device.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _IOTCDEVICE_H_
#define _IOTCDEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "IOTCCommon.h"

/// For IOTC_Device_LoginEx
typedef struct DeviceLoginInput {
    uint32_t cb; //!< Check byte for structure size
    IOTCAuthenticationType authentication_type;
    char auth_key[IOTC_AUTH_KEY_LENGTH]; //!< Allow '0'~'9' & 'A'~'Z' & 'a'~'z'
} DeviceLoginInput;

/**
 * \details The prototype of getting login info function, used by a device
 *          to be notified if it is still kept login with IOTC servers or is
 *          disconnected with IOTC servers.
 *
 * \param nLoginInfo [out] The login info with meanings of following bits
 *            - bit 0: the device is ready for connection by client from LAN if this bit is 1
 *            - bit 1: once the device can contact with masters and p2p servers, this bit will be set to 1
 *            - bit 2: if this bit is 1, it means the device has received login response from IOTC servers.
 *                     This bit will be set to 0 if p2p server has not responsed about 75 seconds.
 *          For example,
 *                   if (nLoginInfo & 0x01) equals 1, it means the device is ready for connection from LAN
 *                   if (nLoginInfo & 0x04) equals 1, it means the device is ready for connection through the internet
 *
 */
typedef void (__stdcall *loginInfoCB)(uint32_t nLoginInfo);

/**
 * \brief Used by a device to login to IOTC servers
 *
 * \details This function will let a device login to IOTC servers. UID is required
 *          when login to IOTC servers. The device name and password are applicable
 *          only in LAN mode when the device cannot login to IOTC servers
 *          due to network issue.
 *
 * \param cszUID [in] The UID of that device login to IOTC servers
 * \param cszDeviceName [in] The name of that device, used in LAN mode for clients to connect
 * \param cszDevicePWD [in] This parameter is no longer used
 *
 * \return #IOTC_ER_NoERROR if device had obtained server list and sent login packets successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_UNLICENSE The UID is not licensed or expired
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_LOGIN_ALREADY_CALLED The device is already under login process currently
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *            - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #IOTC_ER_FAIL_RESOLVE_HOSTNAME Cannot resolve masters' host name
 *            - #IOTC_ER_TCP_TRAVEL_FAILED Cannot connect to masters in neither UDP nor TCP
 *            - #IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED Cannot connect to IOTC servers in TCP
 *            - #IOTC_ER_NO_PERMISSION The device does not support TCP relay
 *            - #IOTC_ER_SERVER_NOT_RESPONSE All servers have no response
 *            - #IOTC_ER_FAIL_GET_LOCAL_IP Fails to get the device's local IP address
 *            - #IOTC_ER_NO_SERVER_LIST No IOTC server information while device login
 *            - #IOTC_ER_MASTER_NOT_RESPONSE All masters have no respond
 *
 * \attention (1) This API is a blocking function. That means this function will wait until
 *                the device can login into IOTC servers successfully or some error happens
 *                during the login process. It is suggested to use another thread to perform
 *                login process so that sequential instructions will not blocked. <br><br>
 *            (2) Please use IOTC_Get_Login_Info() to check if device really logins
 *                into server successfully.
 *            (3) This API can only be used in device side
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Device_Login(const char *cszUID, const char *cszDeviceName, const char *cszDevicePWD);

/**
 * \brief Used by a device to login to IOTC servers
 *
 * \details This function will let a device login to IOTC servers with authentication.
 *          UID and key is required when login to IOTC servers.
 *
 * \param cszUID [in] The UID of that device login to IOTC servers
 * \param optionalInput [in] A pointer that points to a memory which the login
 *                           information is saved to.
 *
 *
 * \return #IOTC_ER_NoERROR if device had obtained server list and sent login packets successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_UNLICENSE The UID is not licensed or expired
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_LOGIN_ALREADY_CALLED The device is already under login process currently
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *            - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #IOTC_ER_FAIL_RESOLVE_HOSTNAME Cannot resolve masters' host name
 *            - #IOTC_ER_TCP_TRAVEL_FAILED Cannot connect to masters in neither UDP nor TCP
 *            - #IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED Cannot connect to IOTC servers in TCP
 *            - #IOTC_ER_NO_PERMISSION The device does not support TCP relay
 *            - #IOTC_ER_SERVER_NOT_RESPONSE All servers have no response
 *            - #IOTC_ER_FAIL_GET_LOCAL_IP Fails to get the device's local IP address
 *            - #IOTC_ER_NO_SERVER_LIST No IOTC server information while device login
 *            - #IOTC_ER_MASTER_NOT_RESPONSE All masters have no respond
 *
 * \attention (1) This API is a blocking function. That means this function will wait until
 *                the device can login into IOTC servers successfully or some error happens
 *                during login the process. It is suggested to use another thread to perform
 *                login process so that sequential instructions will not blocked. <br><br>
 *            (2) Please use IOTC_Get_Login_Info() to check if device really logins
 *                into server successfully.
 *            (3) This API can only be used in device side
 */
P2PAPI_API int32_t IOTC_Device_LoginEx(const char *cszUID, const DeviceLoginInput *optionalInput);


/**
 * \brief Used by a device to get the login information
 *
 * \details This function gets the login information of a device to IOTC servers.
 *
 * \param pnLoginInfo [out] The login info with meanings of following bits
 *            - bit 0: the device is ready for connection by client from LAN if this bit is 1
 *            - bit 1: once the device can contact with masters and p2p servers, this bit will be set to 1
 *            - bit 2: if this bit is 1, it means the device has received login response from IOTC servers.
 *                   This bit will be set to 0 if p2p server has not responsed about 75 seconds.
 *          For example,
 *                   if (pnLoginInfo & 0x01) equals 1, it means the device is ready for connection from LAN
 *                   if (pnLoginInfo & 0x04) equals 1, it means the device is ready for connection through the internet
 *
 * \return The number of fails to login to IOTC servers.
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *
 * \attention (1) This API can only be used in device side
 *
 */
P2PAPI_API int32_t IOTC_Get_Login_Info(uint32_t *pnLoginInfo);

/**
 * \brief Used by a device to get the login information
 *
 * \details This function gets the login information of a device to IOTC servers. <br>
 *          The difference of this function and IOTC_Get_Login_Info() is
 *          this function will set callback function inside IOTC module and
 *          that callback function will be invoked whenever the login status
 *          of that device is updated from IOTC servers, for example, IOTC
 *          servers response login message to that device or the connection
 *          between IOTC servers and that device has been lost.
 *
 * \param pfxLoginInfoFn [in] The function pointer to getting login info function
 *
 * \attention (1) This API can only be used in device side
 */
P2PAPI_API void IOTC_Get_Login_Info_ByCallBackFn(loginInfoCB pfxLoginInfoFn);

/**
 * \brief Used by a device to listen connections from clients
 *
 * \details This function is for a device to listen any connection from clients.
 *          If connection is established with the help of IOTC servers, the
 *          IOTC session ID will be returned in this function and then device and
 *          client can communicate for the other later by using this IOTC session ID.
 *
 * \param nTimeout [in] The timeout for this function in unit of millisecond, give 0 means block forever
 *
 * \return IOTC session ID if return value >= 0
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_EXCEED_MAX_SESSION The number of IOTC sessions has reached maximum in device side
 *            - #IOTC_ER_TIMEOUT No connection is established from clients before timeout expires
 *            - #IOTC_ER_EXIT_LISTEN The device stops listening for connections from clients.
 *
 * \attention (1) This API is a blocking function, waiting for following two
 *                conditions happens before executing sequential instructions
 *                (a) any connection from clients is established
 *                (b) timeout expires.<br><br>
 *            (2) nTimeout has no effect in 8051 platform.
 *            (3) Recommended value of timeout: 1000 millisecond ~ 30000 millisecond
 *            (4) IOTC_Listen_Exit can only make IOTC_Listen return immediately, not to stop listening
 *                the incoming connection, you can still listen a new session if you call IOTC_Listen again
 *            (5) This API can only be used in device side
 */
P2PAPI_API int32_t IOTC_Listen(uint32_t nTimeout);

/**
 * \brief Used by a device to exit listen process
 *
 * \details Since listen process is a block process and that means a device
 *          will wait for following two conditions happens before executing
 *          sequential instructions (1) any connection from clients is established
 *          (2) timeout expires. In some cases, users may want the device to
 *          exit listen immediately by this function in another thread before
 *          the two conditions above happens.Please note that the device still can accept
 *          incoming connection even IOTC_Listen_Exit has been called.
 */
P2PAPI_API void IOTC_Listen_Exit(void);

/**
 * \brief Set device name.
 *
 * \details Device can let client know its name when client call IOTC_Lan_Search2().
 *          The maximum size of device name is 131 Byte. We filled in 0 at the 132th Byte.
 *
 * \param cszDeviceName  [in] This is user-defined device name. Clients will get it by calling IOTC_Lan_Search2().
 *
 */
P2PAPI_API  void IOTC_Set_Device_Name(const char *cszDeviceName);

/**
* \brief Change IOTC authentication key
*
* \details custom can change the authentication key after calling this API
*
* \param new_authkey [in] Pass the new authentication key
*
* \attention This API can be called only if the authentication type of device is AUTHENTICATE_BY_KEY
*            or it will return IOTC_ER_NOT_SUPPORT
* \attention The key format only allow '0'~'9' & 'A'~'Z' & 'a'~'z'
* \attention This API can only be used in device side
*
* \return Return IOTC_ER_NoERROR if successfully.
* \return Error code if return value < 0
*           - #IOTC_ER_INVALID_ARG if the format of key is invalid
*           - #IOTC_ER_DID_NOT_LOGIN if IOTC_Device_LoginEx did not login successfully
*           - #IOTC_ER_DID_NOT_LOGIN_WITH_AUTHKEY if device login with IOTC_Device_Login
**/
P2PAPI_API int32_t IOTC_Device_Update_Authkey(const char *new_authkey);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _IOTCDEVICE_H_ */
