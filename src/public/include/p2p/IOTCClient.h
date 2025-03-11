/*! \file IOTCClient.h
This file describes IOTC module APIs for client.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _IOTCCLIENT_H_
#define _IOTCCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "IOTCCommon.h"

/**
 * \details Connect Option, containing all options of connection setup when client connects to device by P2P or relay mode.
 */
typedef struct st_ConnectOption {
    int8_t IsParallel; //!< 0: Turn off parallel connection. 1: Turn on parallel connection.
    int8_t IsLowConnectionBandwidth; //!< 0: Normal connection mode 1: Low bandwidth mode (This mode might reduce the P2P traversal rate)
    int8_t IsP2PRequestRoundRobin; //!< 0: Normal connection mode 1: P2P Request round robin mode (This mode might reduce the P2P traversal rate)
    int8_t IsNotToCheckLanIpforP2P; //!< 0: Check the P2P connected IP if it is change mode to LAN 1: Do not check the P2P connected IP
} st_ConnectOption_t;

typedef struct IOTCCheckDeviceInput {
    uint32_t cb; //!< Check byte for structure size
    IOTCAuthenticationType authentication_type;
    char auth_key[IOTC_AUTH_KEY_LENGTH];
} IOTCCheckDeviceInput;

typedef struct IOTCCheckDeviceOutput {
    uint32_t Status; //!< 0: online 1: sleep
    uint32_t LastLoginTimestamp; //!< Device last login timestamp since the Epoch, 1970-01-01 00:00:00 +0000 (UTC)
} IOTCCheckDeviceOutput;

/**
 * \details This is the handler for reporting the device on line status. It is used for 
 * IOTC_Check_Device_On_Line().
 *
 * \param result [out] The result of getting the device status.
 *         - #IOTC_ER_NoERROR No error happens during the checking flow and the Device is on line.
 *         - #IOTC_ER_NETWORK_UNREACHABLE Network is unreachable
 *         - #IOTC_ER_MASTER_NOT_RESPONSE All masters have no respond
 *         - #IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED Cannot connect to IOTC servers in TCP
 *         - #IOTC_ER_CAN_NOT_FIND_DEVICE IOTC servers cannot locate the specified device
 *         - #IOTC_ER_SERVER_NOT_RESPONSE All servers have no response
 *         - #IOTC_ER_TCP_TRAVEL_FAILED Cannot connect to masters in neither UDP nor TCP
 *         - #IOTC_ER_DEVICE_OFFLINE The device is not on line.
 *
 * \param userData [in] The data which was passed during IOTC_Check_Device_On_Line() is called.
 *
 */
typedef void(__stdcall *onLineResult)(int32_t result, void *userData);

/**
 * \brief Checking device online or not.
 *
 * \param UID [in] A device UID which is used to check the state.
 * \param input [in] Additional info of auth key, NULL if device does not use auth key
 * \param output [out] Device status, See #IOTCCheckDeviceOutput
 * \param timeout_ms [in] The time out value of checking device information in millisecond.
 * \param abort [in] Assign abort other than 0 to abort.
 *
 * \return #IOTC_ER_NoERROR if device online or sleeping
 * \return Error code if return value < 0
 *          - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *          - #IOTC_ER_UNLICENSE The UID is not a valid UID.
 *          - #IOTC_ER_EXCEED_MAX_SESSION The session is full.
 *          - #IOTC_ER_FAIL_RESOLVE_HOSTNAME Cannot resolve masters' host name
 *          - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *          - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *          - #IOTC_ER_MASTER_NOT_RESPONSE All masters have no respond
 *          - #IOTC_ER_DEVICE_OFFLINE The device is not on line.
 *          - #IOTC_ER_SESSION_IN_USE Another session uses the same UID is in process
 *          - #IOTC_ER_TIMEOUT Unable to get device status before timeout expires
 *
 * \attention (1) This API can only be used in client side
 *            (2) P2P server above version 3.1.2.0 is required
 *
 */
P2PAPI_API int32_t IOTC_Check_Device_OnlineEx(const char *UID, IOTCCheckDeviceInput *input, IOTCCheckDeviceOutput *output, uint32_t timeout_ms, int32_t *abort);

/**
 * \brief Used by a client to connect a device
 *
 * \details This function is for a client to connect a device by specifying
 *          the UID of that device. If connection is established with the
 *          help of IOTC servers, the IOTC session ID will be returned in this
 *          function and then device and client can communicate for the other
 *          later by using this IOTC session ID.
 *
 * \param cszUID [in] The UID of a device that client wants to connect
 *
 * \return IOTC session ID if return value >= 0
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_UNLICENSE The specified UID of that device is not licensed or expired
 *            - #IOTC_ER_EXCEED_MAX_SESSION The number of IOTC sessions has reached maximum in client side
 *            - #IOTC_ER_DEVICE_NOT_LISTENING The device is not listening for connection now
 *            - #IOTC_ER_FAIL_CONNECT_SEARCH The client stop connecting to the device
 *            - #IOTC_ER_FAIL_RESOLVE_HOSTNAME Cannot resolve masters' host name
 *            - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #IOTC_ER_TCP_TRAVEL_FAILED Cannot connect to masters in neither UDP nor TCP
 *            - #IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED Cannot connect to IOTC servers in TCP
 *            - #IOTC_ER_CAN_NOT_FIND_DEVICE IOTC servers cannot locate the specified device
 *            - #IOTC_ER_NO_PERMISSION The specified device does not support TCP relay
 *            - #IOTC_ER_SERVER_NOT_RESPONSE IOTC servers have no response
 *            - #IOTC_ER_FAIL_GET_LOCAL_IP Fails to get the client's local IP address
 *            - #IOTC_ER_FAIL_SETUP_RELAY Fails to connect the device via relay mode
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *            - #IOTC_ER_NOT_SUPPORT_RELAY Not support relay connection by IOTC servers
 *            - #IOTC_ER_NO_SERVER_LIST No IOTC server information while client connect
 *            - #IOTC_ER_DEVICE_MULTI_LOGIN The connecting device duplicated loggin and may unconnectable
 *            - #IOTC_ER_MASTER_NOT_RESPONSE All masters have no respond
 *            - #IOTC_ER_DEVICE_IS_SLEEP Device is in sleep mode
 *
 * \attention (1) This API is a blocking function. This function will wait until
 *                the connection to IOTC server established successfully or some error happens
 *                during the connect process
 *            (2) This API has been deprecated and might be removed in the next version.
 *                If you want to add security,please replace it with IOTC_Connect_ByUIDEx.
 *                Otherwise replace it with IOTC_Connect_ByUID_Parallel
 *            (3) This API can only be used in client side
 *
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Connect_ByUID(const char *cszUID);

/**
 * \brief Checking a device on line or not.
 *
 * \details This function is used for checking a device on line status. A device
 * is on line means the device login to a server or the device is able to be searched
 * on LAN. To check to state, it takes a little bit time to query the device status
 * from IOTC servers. In order to not blocking anything, the function returns 
 * immediately, the result will be shown in the handler.
 *
 * \attention THE "hander" MUST NOT BE BLOCKED. IT SHOULD BE RETURNED ASAP.
 * IF YOU WANT TO DO SOMETHING BLOCK (i.e. sleep, or any kind of loop.), 
 * PLEASE CREATE A THREAD TO DO IT.
 *
 * \param UID [in] A device UID which is used to check the state.
 * \param timeOut [in] The time out value of checking device information in millisecond. 
 * \param handler [in] A handle function for getting the device on line result.
 * \param userData [in] The data would like to bring to the handler for feather processing.
 *
 *
 * \return IOTC_ER_NoERROR on successful. The others are error.
 *         - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *         - IOTC_ER_UNLICENSE The UID is not a valid UID.
 *         - IOTC_ER_EXCEED_MAX_SESSION The session is full.
 *         - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *         - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *         - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *         - IOTC_ER_NOT_ENOUGH_MEMORY No enough memory to run the function.
 *
 * \attention (1) This API can only be used in client side
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Check_Device_On_Line(const char* UID, const uint32_t timeOut,
        onLineResult handler, void* userData);

/**
 * \brief Used by a client to connect a device
 *
 * \details This function is for a client to connect a device by specifying
 *          the UID and password of that device. If connection is established with the
 *          help of IOTC servers, the IOTC session ID will be returned in this
 *          function and then device and client can communicate for the other
 *          later by using this IOTC session ID.This function will wake up device if it's sleeping.
 *
 * \param cszUID [in] The UID of a device that client wants to connect
 * \param SID [in] The Session ID got from IOTC_Get_SessionID() the connection should bind to.
 * \param connectInput [in] A pointer that points to a memory which the login information is saved to.
 *
 * \return IOTC session ID if return value >= 0
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_UNLICENSE The specified UID of that device is not licensed or expired
 *            - #IOTC_ER_EXCEED_MAX_SESSION The number of IOTC sessions has reached maximum in client side
 *            - #IOTC_ER_DEVICE_NOT_LISTENING The device is not listening for connection now
 *            - #IOTC_ER_FAIL_CONNECT_SEARCH The client stop connecting to the device
 *            - #IOTC_ER_FAIL_RESOLVE_HOSTNAME Cannot resolve masters' host name
 *            - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #IOTC_ER_TCP_TRAVEL_FAILED Cannot connect to masters in neither UDP nor TCP
 *            - #IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED Cannot connect to IOTC servers in TCP
 *            - #IOTC_ER_CAN_NOT_FIND_DEVICE Not all IOTC servers respond the device login status.
 *            - #IOTC_ER_NO_PERMISSION The specified device does not support TCP relay
 *            - #IOTC_ER_SERVER_NOT_RESPONSE IOTC servers have no response
 *            - #IOTC_ER_FAIL_GET_LOCAL_IP Fails to get the client's local IP address
 *            - #IOTC_ER_FAIL_SETUP_RELAY Fails to connect the device via relay mode, perhaps the device abnormally logout from server
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *            - #IOTC_ER_NOT_SUPPORT_RELAY Not support relay connection by IOTC servers
 *            - #IOTC_ER_NO_SERVER_LIST No IOTC server information while client connect
 *            - #IOTC_ER_DEVICE_MULTI_LOGIN The connecting device duplicated loggin and may unconnectable
 *            - #IOTC_ER_MASTER_NOT_RESPONSE All masters have no respond
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is already connected or not valid
 *            - #IOTC_ER_DEVICE_IS_SLEEP Device is in sleep mode
 *            - #IOTC_ER_DEVICE_REJECT_BYPORT Device is not yet call IOTC_Listen(), please retry
 *            - #IOTC_ER_DEVICE_REJECT_BY_WRONG_AUTH_KEY connect with wrong key
 *            - #IOTC_ER_SESSION_IN_USE Another session uses the same UID is in process
 *            - #IOTC_ER_DEVICE_OFFLINE All IOTC servers respond the device is not online.
 *            - #IOTC_ER_TIMEOUT Connection timeout
 *            - #IOTC_ER_DEVICE_NOT_USE_KEY_AUTHENTICATION Being rejected because device disable authentication
 *
 * \attention (1) This API is a blocking function. This function will wait until
 *                the connection to IOTC server established successfully or some error happens
 *                during the connect process
 *            (2) This API can only be used in client side
 *
 */
P2PAPI_API int32_t IOTC_Connect_ByUIDEx(const char *cszUID, int32_t SID, IOTCConnectInput *connectInput );

/**
 * \brief Used by a client to connect a device and bind to a specified session ID.
 *
 * \details This function is for a client to connect a device by specifying
 *          the UID of that device, and bind to a tutk_platform_free session ID from IOTC_Get_SessionID().
 *          If connection is established with the help of IOTC servers,
 *          the #IOTC_ER_NoERROR will be returned in this function and then device and
 *          client can communicate for the other later by using this IOTC session ID.
 *          If this function is called by multiple threads, the connections will be
 *          processed concurrently.
 *
 * \param cszUID [in] The UID of a device that client wants to connect
 * \param SID [in] The Session ID got from IOTC_Get_SessionID() the connection should bind to.
 *
 * \return IOTC session ID if return value >= 0 and equal to the input parameter SID.
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_UNLICENSE The specified UID of that device is not licensed or expired
 *            - #IOTC_ER_EXCEED_MAX_SESSION The number of IOTC sessions has reached maximum in client side
 *            - #IOTC_ER_DEVICE_NOT_LISTENING The device is not listening for connection now
 *            - #IOTC_ER_FAIL_CONNECT_SEARCH The client stop connecting to the device
 *            - #IOTC_ER_FAIL_RESOLVE_HOSTNAME Cannot resolve masters' host name
 *            - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #IOTC_ER_TCP_TRAVEL_FAILED Cannot connect to masters in neither UDP nor TCP
 *            - #IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED Cannot connect to IOTC servers in TCP
 *            - #IOTC_ER_CAN_NOT_FIND_DEVICE Not all IOTC servers respond the device login status.
 *            - #IOTC_ER_NO_PERMISSION The specified device does not support TCP relay
 *            - #IOTC_ER_SERVER_NOT_RESPONSE IOTC servers have no response
 *            - #IOTC_ER_FAIL_GET_LOCAL_IP Fails to get the client's local IP address
 *            - #IOTC_ER_FAIL_SETUP_RELAY Fails to connect the device via relay mode, perhaps the device abnormally logout from server.
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *            - #IOTC_ER_NOT_SUPPORT_RELAY Not support relay connection by IOTC servers
 *            - #IOTC_ER_NO_SERVER_LIST No IOTC server information while client connect
 *            - #IOTC_ER_DEVICE_MULTI_LOGIN The connecting device duplicated login and may unconnectable
 *            - #IOTC_ER_MASTER_NOT_RESPONSE All masters have no respond
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is already connected or not valid
 *            - #IOTC_ER_DEVICE_IS_SLEEP Device is in sleep mode
 *            - #IOTC_ER_DEVICE_REJECT_BYPORT Device is not yet call IOTC_Listen(), please retry
 *            - #IOTC_ER_SESSION_IN_USE Another session uses the same UID is in process
 *            - #IOTC_ER_DEVICE_OFFLINE All IOTC servers respond the device is not online.
 *
 * \attention  (1) This API is a blocking function. This function will wait until
 *                 the connection to IOTC server established successfully or some error happens
 *                 during the connect process.
 *             (2) If you call IOTC_Connect_Stop_BySID() and this function not return yet, and then use the same
 *                 session ID to call again will cause session ID in wrong status.
 *             (3) This API can only be used in client side
 *
 */
P2PAPI_API int32_t IOTC_Connect_ByUID_Parallel(const char *cszUID, int32_t SID);

/**
 * \brief Used by a client to stop connecting a device
 *
 * \details This function is for a client to stop connecting a device. Since
 *          IOTC_Connect_ByUID(), IOTC_Connect_ByUID2() are all block processes, that means
 *          the client will have to wait for the return of these functions before
 *          executing sequential instructions. In some cases, users may want
 *          the client to stop connecting immediately by this function in
 *          another thread before the return of connection process.
 *
 * \attention Only use to stop IOTC_Connect_ByUID() and 2, NOT use to stop IOTC_Connect_ByUID_Parallel().
 */
P2PAPI_API void IOTC_Connect_Stop(void);

/**
 * \brief Used by a client to stop a specific session connecting a device
 *
 * \details This function is for a client to stop connecting a device. Since
 *          IOTC_Connect_ByUID_Parallel() is a block processes, that means
 *          the client will have to wait for the return of these functions before
 *          executing sequential instructions. In some cases, users may want
 *          the client to stop connecting immediately by this function in
 *          another thread before the return of connection process.
 *
 * \param SID [in] The Session ID of a connection which will be stop.
 *
 * \return #IOTC_ER_NoERROR
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 */
P2PAPI_API int32_t IOTC_Connect_Stop_BySID(int32_t SID);

/**
 * \brief Used for searching devices in LAN.
 *
 * \details When client and devices are in LAN, client can search devices and their name by calling this function.
 *
 * \param psLanSearchInfo2 [in] The array of struct st_LanSearchInfo2 store the search result and Device name.
 *
 * \param nArrayLen [in] The size of psLanSearchInfo2 array
 *
 * \param nWaitTimeMs [in] Period (or timeout) of searching LAN. (milliseconds)
 *
 * \return The number of devices found.
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_ARG The arguments passed in to this function is invalid.
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *
 * \attention (1) Not support IPv6. The IP length is only for IPv4.<br><br>*
 *            (2) This API has been deprecated. Please use IOTC_Search_Device_Start / IOTC_Search_Device_Result.<br><br>*
 *            (3) This API can only be used in client side
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Lan_Search2(struct st_LanSearchInfo2 *psLanSearchInfo2, int32_t nArrayLen, int32_t nWaitTimeMs);

/**
 * \brief Used for searching devices in LAN.
 *
 * \details When client and devices are in LAN, client can search devices and their name
 *            by calling this function.
 *
 * \param psLanSearchInfo2 [in] The array of struct st_LanSearchInfo2 store the search result and Device name.
 * \param nArrayLen [in] The size of psLanSearchInfo2 array
 * \param nWaitTimeMs [in] Period (or timeout) of searching LAN. (milliseconds)
 * \param nSendIntervalMs [in] Interval of sending broadcast for searching device in LAN. (milliseconds)
 *
 * \return The number of devices found.
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_ARG The arguments passed in to this function is invalid.
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *
 * \attention (1) Not support IPv6. The IP length is only for IPv4.<br><br>*
 *            (2) This API has been deprecated. Please use IOTC_Search_Device_Start / IOTC_Search_Device_Result.<br><br>*
 *            (3) This API can only be used in client side
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Lan_Search2_Ex(struct st_LanSearchInfo2 *psLanSearchInfo2, int32_t nArrayLen, int32_t nWaitTimeMs, int32_t nSendIntervalMs);

/**
 * \brief Start to search devices in LAN.
 *
 * \details When client and devices are in LAN, client can search devices and their name
 *            and the result can be polled by function IOTC_Search_Device_Result
 *
 * \param nWaitTimeMs [in] Period (or timeout) of searching LAN. (milliseconds)
 * \param nSendIntervalMs [in] Interval of sending broadcast for searching device in LAN. (milliseconds)
 *
 * \return The number of devices found in the LAN (it is limited by the length of array)
 * \return Error code if return value < 0
 *            - #IOTC_ER_INVALID_ARG The arguments passed in to this function is invalid.
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *            - #IOTC_ER_STILL_IN_PROCESSING The function is called more then once before it stopped
 *            - #IOTC_ER_NOT_ENOUGH_MEMORY not enough memory
 *
 * \attention (1) Recommended value of timeout: 1000 millisecond ~ 10000 millisecond
 *            (2) This API can only be used in client side
 *
 */
P2PAPI_API int32_t IOTC_Search_Device_Start(int32_t nWaitTimeMs, int32_t nSendIntervalMs);

/**
* \brief Poll the results of searched device in LAN.
*
* \details Use the function to poll the result of device search in LAN, the IOTC_Search_Device_Start should be called before use the function
*
* \param st_SearchDeviceInfo [in] The array of struct st_SearchDeviceInfo store the search result and Device name.
* \param nArrayLen [in] The length of array
* \param nGetAll [in] 0: get new queried device 1: get all queried devices
*
* \return IOTC_ER_NoERROR if search devices in LAN successfully
* \return Error code if return value < 0
*            - #IOTC_ER_INVALID_ARG The arguments passed in to this function is invalid.
*            - #IOTC_ER_SERVICE_IS_NOT_STARTED The start function is not called
*
* \attention (1) Support IPv6. The IP length supports both for IPv4/IPv6.<br><br>*
*/
P2PAPI_API int32_t IOTC_Search_Device_Result(struct st_SearchDeviceInfo *psSearchDeviceInfo, int32_t nArrayLen, int32_t nGetAll);

/**
* \brief Stop to search devices in LAN.
*
* \details Stop to do device search in LAN, the IOTC_Search_Device_Start should be called before use the function
*
* \param st_SearchDeviceInfo [in] The array of struct st_SearchDeviceInfo store the search result and Device name.
* \param nArrayLen [in] The length of array
* \param nGetAll [in] 0: get new queried device 1: get all queried devices
*
* \return IOTC_ER_NoERROR if search devices in LAN successfully
* \return Error code if return value < 0
*            - #IOTC_ER_SERVICE_IS_NOT_STARTED The start function is not called
*/
P2PAPI_API int32_t IOTC_Search_Device_Stop();

/**
* \brief Setup connect option when client connects to device.
*
* \details Client uses this function to set the Option of Connection.
*
* \param S_ConnectOption [in] the connect option that contained the option to be set.
**/
P2PAPI_API int32_t IOTC_Set_Connection_Option(struct st_ConnectOption *S_ConnectOption);

/**
 * \brief Setup LAN search and LAN connection timeout
 *
 * \details Only client can call this, it can determine how many time to try LAN search and LAN connection.
 *          Once it called the timeout value is effective forever until IOTC_DeInitialize() be called.
 *
 * \param nTimeout [in] The timeout for this function in unit of millisecond, give 0 means skip LAN flow
 *
 * \attention (1) Mast be called before start connection. Minimum is 100 millisecond.
 *            (2) This API can only be used in client side
 *
 */
P2PAPI_API void IOTC_Setup_LANConnection_Timeout(uint32_t nTimeout);

/**
* \brief Setup P2P connection timeout
*
* \details Only client can call this, it can determine how many time to try P2P connection.
*          Once it called the timeout value is effective forever until IOTC_DeInitialize() be called.
*
* \param nTimeout [in] The timeout for this function in unit of millisecond, give 0 means skip P2P flow
*
* \attention (1) Mast be called before start connection. Minimum is 100 millisecond.
*            (2) This API can only be used in client side
*
*/
P2PAPI_API void IOTC_Setup_P2PConnection_Timeout(uint32_t nTimeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _IOTCCLIENT_H_ */
