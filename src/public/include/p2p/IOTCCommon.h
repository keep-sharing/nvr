/*! \file IOTCCommon.h
This file describes common IOTC module APIs of device and client.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
*/

#ifndef _IOTCCOMMON_H_
#define _IOTCCOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TUTKGlobalAPIs.h"

/* ============================================================================
 * Platform Dependant Macro Definition
 * ============================================================================
 */

#ifdef _WIN32
    #ifdef IOTC_STATIC_LIB
        #define P2PAPI_API
    #elif defined P2PAPI_EXPORTS
        #define P2PAPI_API __declspec(dllexport)
    #else
        #define P2PAPI_API __declspec(dllimport)
    #endif
#else
    #define P2PAPI_API
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define P2PAPI_API_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
    #ifdef IOTC_STATIC_LIB
        #define P2PAPI_API_DEPRECATED __declspec(deprecated)
    #elif defined P2PAPI_EXPORTS
        #define P2PAPI_API_DEPRECATED __declspec(deprecated, dllexport)
    #else
        #define P2PAPI_API_DEPRECATED __declspec(deprecated, dllimport)
    #endif
#else
    #define P2PAPI_API_DEPRECATED
#endif

#ifndef _WIN32
    #define __stdcall
#endif


/* ============================================================================
 * Generic Macro Definition
 * ============================================================================
 */
#define MAX_DEFAULT_IOTC_SESSION_NUMBER              128

/** The maximum size, in byte, of the buffer used in IOTC_Session_Read(),
 * IOTC_Session_Write() and IOTC_Session_Read_Check_Lost(). */
#define IOTC_MAX_PACKET_SIZE                         1388

/** The maximum number of IOTC channels for each IOTC session */
#define MAX_CHANNEL_NUMBER                           32

/** The timeout, in unit of second, for keeping an IOTC session alive since
 * the last time remote site has response */
#define IOTC_SESSION_ALIVE_TIMEOUT                   60

/** For IOTC Authorization */
#define IOTC_AUTH_KEY_LENGTH                         8


/* ============================================================================
 * Error Code Declaration
 * ============================================================================
 */
/** The function is performed successfully. */
#define IOTC_ER_NoERROR                              0

/** IOTC servers have no response, probably caused by many types of Internet connection issues. */
#define IOTC_ER_SERVER_NOT_RESPONSE                 -1

/** IOTC masters cannot be resolved their domain name, probably caused
 * by network connection or DNS setting issues. */
#define IOTC_ER_FAIL_RESOLVE_HOSTNAME               -2

/** IOTC module is already initialized. It is not necessary to re-initialize. */
#define IOTC_ER_ALREADY_INITIALIZED                 -3

/** IOTC module fails to create Mutexes when doing initialization. Please
 * check if OS has sufficient Mutexes for IOTC platform. */
#define IOTC_ER_FAIL_CREATE_MUTEX                   -4

/** IOTC module fails to create threads. Please check if OS has ability
 * to create threads for IOTC module. */
#define IOTC_ER_FAIL_CREATE_THREAD                  -5

/** IOTC module fails to create sockets. if you are using Android, please
 * check if android.permission.INTERNET setting is enabled. Otherwise,
 * please check if your platform supports socket service */
#define IOTC_ER_FAIL_CREATE_SOCKET                  -6

/** IOTC module fails to set up socket options. */
#define IOTC_ER_FAIL_SOCKET_OPT                     -7

/** IOTC module fails to bind sockets */
#define IOTC_ER_FAIL_SOCKET_BIND                    -8

/** The specified UID is not licensed or expired. */
#define IOTC_ER_UNLICENSE                           -10

/** The device is already under login process currently
 * so it is prohibited to invoke login again at this moment. */
#define IOTC_ER_LOGIN_ALREADY_CALLED                -11

/** IOTC module is not initialized yet. Please use IOTC_Initialize() or
 * IOTC_Initialize2() for initialization. */
#define IOTC_ER_NOT_INITIALIZED                     -12

/** The specified timeout has expired during the execution of some IOTC
 * module service. For most cases, it is caused by slow response of remote
 * site or network connection issues */
#define IOTC_ER_TIMEOUT                             -13

/** The specified IOTC session ID is not valid. OR the SID of the session is closed.
 * A valid SID must be -1 < SID < max_session_number which is set by
 * IOTC_Set_Max_Session_Number and be got by IOTC_Connect_*. */
#define IOTC_ER_INVALID_SID                         -14

/** The specified device's name is unknown to the IOTC servers */
#define IOTC_ER_UNKNOWN_DEVICE                      -15

/** IOTC module fails to get the local IP address */
#define IOTC_ER_FAIL_GET_LOCAL_IP                   -16

/** The device already start to listen for connections from clients. It is
 * not necessary to listen again. */
#define IOTC_ER_LISTEN_ALREADY_CALLED               -17

/** The number of IOTC sessions has reached maximum.
 * To increase the max number of IOTC sessions, please use IOTC_Set_Max_Session_Number()
 * before initializing IOTC module. */
#define IOTC_ER_EXCEED_MAX_SESSION                  -18

/** IOTC servers cannot locate the specified device, probably caused by
 * disconnection from the device or that device does not login yet. */
#define IOTC_ER_CAN_NOT_FIND_DEVICE                 -19

/** The remote site already closes this IOTC session.
 * Please call IOTC_Session_Close() to release IOTC session resource in locate site. */
#define IOTC_ER_SESSION_CLOSE_BY_REMOTE             -22

/** This IOTC session is disconnected because remote site has no any response
 * after a specified timeout expires, i.e. #IOTC_SESSION_ALIVE_TIMEOUT */
#define IOTC_ER_REMOTE_TIMEOUT_DISCONNECT           -23

/** The client fails to connect to a device because the device is not listening for connections. */
#define IOTC_ER_DEVICE_NOT_LISTENING                -24

/** The IOTC channel of specified channel ID is not turned on before transferring data. */
#define IOTC_ER_CH_NOT_ON                           -26

/** A client stops connecting to a device by calling IOTC_Connect_Stop() */
#define IOTC_ER_FAIL_CONNECT_SEARCH                 -27

/** Too few masters are specified when initializing IOTC module.
 * Two masters are required for initialization at minimum. */
#define IOTC_ER_MASTER_TOO_FEW                      -28

/** A client fails to pass certification of a device due to incorrect key. */
#define IOTC_ER_AES_CERTIFY_FAIL                    -29

/** The number of IOTC channels for a IOTC session has reached maximum, say, #MAX_CHANNEL_NUMBER */
#define IOTC_ER_SESSION_NO_FREE_CHANNEL             -31

/** Cannot connect to masters neither UDP mode nor TCP mode by IP or host name ways */
#define IOTC_ER_TCP_TRAVEL_FAILED                   -32

/** Cannot connect to IOTC servers in TCP */
#define IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED        -33

/** A client wants to connect to a device in non-secure mode while that device
 * supports secure mode only. */
#define IOTC_ER_CLIENT_NOT_SECURE_MODE              -34

/** A client wants to connect to a device in secure mode while that device does
 * not support secure mode. */
#define IOTC_ER_CLIENT_SECURE_MODE                  -35

/** A device does not support connection in secure mode */
#define IOTC_ER_DEVICE_NOT_SECURE_MODE              -36

/** A device does not support connection in non-secure mode */
#define IOTC_ER_DEVICE_SECURE_MODE                  -37

/** A device stops listening for connections from clients. */
#define IOTC_ER_EXIT_LISTEN                         -39

/** The specified device does not support advance function
 *(TCP relay and P2PTunnel module) */
#define IOTC_ER_NO_PERMISSION                       -40

/** Network is unreachable, please check the network settings */
#define IOTC_ER_NETWORK_UNREACHABLE                 -41

/** A client fails to connect to a device via relay mode */
#define IOTC_ER_FAIL_SETUP_RELAY                    -42

/** A client fails to use UDP relay mode to connect to a device
 * because UDP relay mode is not supported for that device by IOTC servers */
#define IOTC_ER_NOT_SUPPORT_RELAY                   -43

/** No IOTC server information while device login or client connect
 * because no IOTC server is running or not add IOTC server list */
#define IOTC_ER_NO_SERVER_LIST                      -44

/** The connecting device has duplicated loggin and may unconnectable. */
#define IOTC_ER_DEVICE_MULTI_LOGIN                  -45

/** The arguments passed to a function is invalid. */
#define IOTC_ER_INVALID_ARG                         -46

/** The remote device not support partial encoding. */
#define IOTC_ER_NOT_SUPPORT_PE                      -47

/** The remote device no more free session can be connected. */
#define IOTC_ER_DEVICE_EXCEED_MAX_SESSION           -48

/** The function call is a blocking call and was called by other thread. */
#define IOTC_ER_BLOCKED_CALL                        -49

/** The session was closed. */
#define IOTC_ER_SESSION_CLOSED                      -50

/** Remote doesn't support this function. */
#define IOTC_ER_REMOTE_NOT_SUPPORTED                -51

/** The function is aborted by related function. */
#define IOTC_ER_ABORTED                             -52

/** The buffer size exceed maximum packet size. */
#define IOTC_ER_EXCEED_MAX_PACKET_SIZE              -53

/** Server does not support this feature. */
#define IOTC_ER_SERVER_NOT_SUPPORT                  -54

/** Cannot find a path to write data*/
#define IOTC_ER_NO_PATH_TO_WRITE_DATA               -55

/** Start function is not called */
#define IOTC_ER_SERVICE_IS_NOT_STARTED              -56

/** Already in processing*/
#define IOTC_ER_STILL_IN_PROCESSING                 -57

/** Out of memory*/
#define IOTC_ER_NOT_ENOUGH_MEMORY                   -58

/** The device is banned and locked, this error code is no longer being used*/
#define IOTC_ER_DEVICE_IS_BANNED                    -59

/** IOTC master servers have no response, probably caused by many types of Internet connection issues. */
#define IOTC_ER_MASTER_NOT_RESPONSE                 -60

/** IOTC module has some resource allocating problem. */
#define IOTC_ER_RESOURCE_ERROR                      -61

/** IOTC Write reliable send queue is full. */
#define IOTC_ER_QUEUE_FULL                          -62

/** The feature is not supported. */
#define IOTC_ER_NOT_SUPPORT                         -63

/** Device is in sleep mode. */
#define IOTC_ER_DEVICE_IS_SLEEP                     -64

/** Device doesn't support this feature on TCP mode. */
#define IOTC_ER_TCP_NOT_SUPPORT                     -65

/** IOTC_WakeUp_Init isn't called */
#define IOTC_ER_WAKEUP_NOT_INITIALIZED              -66

/** Being rejected by device due to the wrong IOTCPort */
#define IOTC_ER_DEVICE_REJECT_BYPORT                -67

/** Being rejected by device due to the wrong auth key */
#define IOTC_ER_DEVICE_REJECT_BY_WRONG_AUTH_KEY     -68

/** Being rejected by device due to disable authentication */
#define IOTC_ER_DEVICE_NOT_USE_KEY_AUTHENTICATION   -69

/** Some functions can only be called after device login success */
#define IOTC_ER_DID_NOT_LOGIN                       -70

/** Need to login with IOTC_Device_loginEx not IOTC_Device_login */
#define IOTC_ER_DID_NOT_LOGIN_WITH_AUTHKEY          -71

/** The function call is being rejected because there is at least one IOTC session in use*/
#define IOTC_ER_SESSION_IN_USE                      -72

/** All Server response can not find device */
#define IOTC_ER_DEVICE_OFFLINE                      -90

/**    IOTC master server is invalid */
#define IOTC_ER_MASTER_INVALID                      -91


/* ============================================================================
 * Enumeration Declaration
 * ============================================================================
 */

/**
 * \details IOTC session mode, used in IOTC_Listen2(), IOTC_Connect_ByUID2()
 *            to specify what kinds of IOTC session
 *            that devices will listen or clients will connect.
 */
typedef enum {
    /// IOTC session will be established in non-secure mode. <br>
    /// - For devices, the behavior is the same as IOTC_Listen(). <br>
    /// - For clients, the behavior is the same as IOTC_Connect_ByUID()
    IOTC_NON_SECURE_MODE = 0,

    /// IOTC session will be established in secure mode. <br>
    /// - For devices, it means only secure connection will be accepted. <br>
    /// - For clients, it means only secure connection will be performed
    IOTC_SECURE_MODE = 1,

    /// IOTC session will be established in either non-secure or secure mode,
    /// depending on remote site's request. Can use IOTC_Session_Check()
    /// to check what mode is actually used. <br>
    /// - For devices, it means both non-secure or secure modes are accepted. <br>
    /// - For clients, it means either non-secure or secure modes can be performed.
    IOTC_ARBITRARY_MODE = 2
} IOTCSessionMode;

typedef enum {
    /// Create IOTC Session with authentication by Key
    AUTHENTICATE_BY_KEY,
} IOTCAuthenticationType;


/* ============================================================================
 * Structure Definition
 * ============================================================================
 */

/**
 * \details IOTC session info, containing all the information when a IOTC session is
 *  established between a device and a client. Users can use IOTC_Session_Check()
 *  to get IOTC session information.
 */
typedef struct st_SInfo {
    uint8_t Mode; //!< 0: P2P mode, 1: Relay mode, 2: LAN mode
    int8_t CorD; //!< 0: As a Client, 1: As a Device
    char UID[21]; //!< The UID of the device
    char RemoteIP[47]; //!< The IP address of remote site used during this IOTC session
    uint16_t RemotePort; //!< The port number of remote site used during this IOTC session
    uint32_t TX_Packetcount; //!< The total packets sent from the device and the client during this IOTC session
    uint32_t RX_Packetcount; //!< The total packets received in the device and the client during this IOTC session
    uint32_t IOTCVersion; //!< The IOTC version
    uint16_t VID; //!< The Vendor ID, part of VPG mechanism
    uint16_t PID; //!< The Product ID, part of VPG mechanism
    uint16_t GID; //!< The Group ID, part of VPG mechanism
    uint8_t NatType; //!< The remote NAT type
    uint8_t isSecure; //!< 0: The IOTC session is in non-secure mode, 1: The IOTC session is in secure mode
} st_SInfo_t;

/**
 * \details IOTC session info, containing all the information when a IOTC session is
 *  established between a device and a client. Users can use IOTC_Session_Check_Ex()
 *  to get IOTC session information.
 */
typedef struct st_SInfoEx {
    uint32_t size; //!< size of the structure
    uint8_t Mode; //!< 0: P2P mode, 1: Relay mode, 2: LAN mode, 255: Not connected.
    int8_t CorD; //!< 0: As a Client, 1: As a Device
    char UID[21]; //!< The UID of the device
    char RemoteIP[47]; //!< The IP address of remote site used during this IOTC session
    uint16_t RemotePort; //!< The port number of remote site used during this IOTC session
    uint32_t TX_Packetcount; //!< The total packets sent from the device and the client during this IOTC session
    uint32_t RX_Packetcount; //!< The total packets received in the device and the client during this IOTC session
    uint32_t IOTCVersion; //!< The IOTC version
    uint16_t VID; //!< The Vendor ID, part of VPG mechanism
    uint16_t PID; //!< The Product ID, part of VPG mechanism
    uint16_t GID; //!< The Group ID, part of VPG mechanism
    uint8_t isSecure; //!< 0: The IOTC session is in non-secure mode, 1: The IOTC session is in secure mode
    uint8_t LocalNatType; //!< The local NAT type, 0: Unknown type, 1: Type 1, 2: Type 2, 3: Type 3, 10: TCP only
    uint8_t RemoteNatType; //!< The remote NAT type, 0: Unknown type, 1: Type 1, 2: Type 2, 3: Type 3, 10: TCP only
    uint8_t RelayType; //!< 0: Not Relay, 1: UDP Relay, 2: TCP Relay
    uint32_t NetState; //!<If no UDP packet is ever received, the first bit of value is 1, otherwise 0
    char RemoteWANIP[47]; //!< The WAN IP address of remote site used during this IOTC session and it is only valid in P2P or Relay mode
    uint16_t RemoteWANPort; //!< The WAN port number of remote site used during this IOTC session and it is only valid in P2P or Relay mode
} st_SInfoEx_t;

/**
 * \details Device search info, containing all the information
 * when client searches devices in LAN.
 */
typedef struct st_LanSearchInfo {
    char UID[21]; //!< The UID of discovered device
    char IP[16]; //!< The IP address of discovered device
    uint16_t port; //!< The port number of discovered device used for IOTC session connection
    char Reserved; //!< Reserved, no use
} st_LanSearchInfo_t;

/**
 * \details Device search info, containing all the information and device name
 * when client searches devices in LAN.
 */
typedef struct st_LanSearchInfo2 {
    char UID[21]; //!< The UID of discovered device
    char IP[16]; //!< The IP address of discovered device
    uint16_t port; //!< The port number of discovered device used for IOTC session connection
    char DeviceName[132]; //!< The Name of discovered device
    char Reserved; //!< Reserved, no use
} st_LanSearchInfo2_t;

/**
 * \details Device search info, containing all the information and device name
 * when client searches devices in LAN.
 */
typedef struct st_SearchDeviceInfo {
    char UID[21]; //!< The UID of discovered device
    char IP[46]; //!< The IP address of discovered device
    uint16_t port; //!< The port number of discovered device used for IOTC session connection
    char DeviceName[132]; //!< The Name of discovered device
    char Reserved; //!< Reserved, no use
    int32_t  nNew; //!< New device
} st_SearchDeviceInfo_t;

typedef struct IOTCConnectInput {
    uint32_t cb; //!< Check byte for structure size
    IOTCAuthenticationType authentication_type;
    char auth_key[IOTC_AUTH_KEY_LENGTH]; //!< allow '0'~'9' & 'A'~'Z' & 'a'~'z'
    uint32_t timeout; //!< set 0 as default behavior
} IOTCConnectInput;


/* ============================================================================
 * Type Definition
 * ============================================================================
 */

/**
 * \details The prototype of getting session status function, used by a device
 *            or client to be notified if session is disconnected.
 *
 * \param nIOTCSessionID [out] The session ID of the session being disconnected
 * \param nErrorCode [out]
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *
 */
typedef void (__stdcall *sessionStatusCB)(int32_t nIOTCSessionID, int32_t nErrorCode);

/**
 * \details The prototype of connection mode change function, used by a device
 *            or client to be notified if connection mode is changed.
 *
 * \param nIOTCSessionID [out] return effected session ID
 * \param nConnMode      [out] return the current connection mode
 *             - # nConnMode = 0 : LAN Mode
 *             - # nConnMode = 1 : P2P Mode
 *             - # nConnMode = 2 : UDP RLY Mode
 *             - # nConnMode = 3 : TCP RLY Mode
 */
typedef void (__stdcall *ConnectModeChangeCB)(int32_t nIOTCSessionID, uint32_t nConnMode);

/* ============================================================================
 * Function Declaration
 * ============================================================================
 */

/**
 * \brief Get the version of IOTC module
 *
 * \details This function returns the version of IOTC module.
 *
 * \param pnVersion [out] The version of IOTC module. It contains
 *        the version from high byte to low byte, for example, 0x01020304
 *        means the version is 1.2.3.4
 *
 * \see RDT_GetRDTApiVer(), avGetAVApiVer()
 */
P2PAPI_API_DEPRECATED void IOTC_Get_Version(uint32_t *pnVersion);

/**
 * \brief Get the version of IOTC module
 *
 * \details This function returns the version of IOTC module.
 *
 */
P2PAPI_API const char* IOTC_Get_Version_String(void);

/**
 * \brief Set the max number of IOTC sessions of IOTC module
 *
 * \details This function set the max number of allowable IOTC sessions in IOTC
 *          module. The max number of IOTC session limits the max number of
 *          connected clients in device side, while it limits the max number
 *          connected devices in client side. A device or a client could use
 *          this function to reduce the number of IOTC sessions in order to save
 *          some memory usage.
 *
 * \param nMaxSessionNum [in] The max number of IOTC sessions
 *
 * \attention (1) This function is optional if users do not want to change the
 *                default max number of IOTC sessions, i.e. MAX_DEFAULT_IOTC_SESSION_NUMBER.
 *                However, if users really wants to change it, this function
 *                shall be called before IOTC platform is initialized.<br><br>
 *            (2) The maximum IOTC session number is platform dependent.
 *                See the definition of MAX_DEFAULT_IOTC_SESSION_NUMBER for each
 *                platform.
 */
P2PAPI_API void IOTC_Set_Max_Session_Number(uint32_t nMaxSessionNum);

/**
 * \brief Initialize IOTC module
 *
 * \details **This function is deprecated, you should use IOTC_Initialize instead.**
 *          This function is used by devices or clients to initialize IOTC
 *          module and shall be called before any IOTC module related
 *          function is invoked except for IOTC_Set_Max_Session_Number().
 *
 * \param nUDPPort [in] Specify a UDP port. Random UDP port is used if it is specified as 0.
 * \param cszP2PHostNamePrimary [in] Specify the host name or IP address of
 *        the primary master. Cannot be NULL. See attention below for more detail.
 * \param cszP2PHostNameSecondary [in] Specify the host name or IP address of
 *        the secondary master. Cannot be NULL. See attention below for more detail.
 * \param cszP2PHostNameThird [in] Specify the host name or IP address of
 *        the third master. Can be NULL if only two masters are required.
 *        See attention below for more detail.
 * \param cszP2PHostNameFourth [in] Specify the host name or IP address of
 *        the fourth master. Can be NULL if only two masters are required.
 *        See attention below for more detail.
 *
 * \return #IOTC_ER_NoERROR if initializing successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_FAIL_RESOLVE_HOSTNAME Cannot resolve masters' host name
 *            - #IOTC_ER_ALREADY_INITIALIZED IOTC module is already initialized
 *            - #IOTC_ER_MASTER_TOO_FEW Two masters are required as parameters at minimum
 *            - #IOTC_ER_FAIL_CREATE_MUTEX Fails to create Mutexs
 *            - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #IOTC_ER_NOT_ENOUGH_MEMORY not enough memory
 *            - #TUTK_ER_NO_LICENSE_KEY Need to call TUTK_SDK_Set_License_Key() before all module start/initialize.
 *
 * \see IOTC_Initialize2(), IOTC_DeInitialize(), TUTK_SDK_Set_License_Key()
 *
 * \attention (1) This function is the key entry to whole IOTC platform, including
 *                RDT module and AV module. That means, if you want to use
 *                RDT module, users shall still use this function to initialize IOTC
 *                module before calling RDT_Initialize(). <br><br>
 *            (2) Usually, host name is suggested to be used to specify a master.
 *                because that will ensure devices and clients can still connect
 *                to masters even the network address configuration of masters
 *                changes in the future. However, in rare case, the host name
 *                of masters can not be resolved due to network issue and it is
 *                necessary to specify IP address of masters in this function
 *                in order for successful connection.
 *            (3) This API has been deprecated and might be removed in the next version,
 *                please replace it with IOTC_Initialize2
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Initialize(uint16_t nUDPPort, const char* cszP2PHostNamePrimary,
                                const char* cszP2PHostNameSecondary, const char* cszP2PHostNameThird,
                                const char* cszP2PHostNameFourth);

/**
 * \brief Initialize IOTC module
 *
 * \details This function is used by devices or clients to initialize IOTC
 *            module and shall be called before any IOTC module related
 *            function is invoked except for IOTC_Set_Max_Session_Number().
 *            <br>
 *            The different between this function and IOTC_Initialize() is this
 *            function uses following steps to connect masters (1) IP addresses
 *            of master (2) if fails to connect in step 1, resolve predefined
 *            domain name of masters (3) try to connect again with the resolved
 *            IP address of step 2 if IP is resolved successfully.
 *
 * \param nUDPPort [in] Specify a UDP port. Random UDP port is used if it is specified as 0.
 *
 * \return #IOTC_ER_NoERROR if initializing successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_ALREADY_INITIALIZED IOTC module is already initialized
 *            - #IOTC_ER_FAIL_CREATE_MUTEX Fails to create Mutexes
 *            - #IOTC_ER_FAIL_CREATE_THREAD Fails to create threads
 *            - #IOTC_ER_NOT_ENOUGH_MEMORY not enough memory
 *            - #TUTK_ER_NO_LICENSE_KEY Need to call TUTK_SDK_Set_License_Key() before all module start/initialize.
 *
 * \see IOTC_Initialize(), IOTC_DeInitialize(), TUTK_SDK_Set_License_Key()
 *
 * \attention This function is the key entry to whole IOTC platform, including
 *                RDT module and AV module. That means, if you want to use
 *                RDT module, users shall still use this function to initialize IOTC
 *                module before calling RDT_Initialize().
 */
P2PAPI_API int32_t IOTC_Initialize2(uint16_t nUDPPort);

/**
 * \brief Deinitialize IOTC module
 *
 * \details This function will deinitialize IOTC module.
 *
 * \return #IOTC_ER_NoERROR if deinitialize successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED the IOTC module is not initialized yet.
 *
 * \see IOTC_Initialize(), IOTC_Initialize2()
 *
 * \attention IOTC_DeInitialize() will release all resources being used in IOTC module,
 *            please make sure all threads calling IOTC APIs have already exited, otherwise
 *            SDK might crash in any IOTC APIs when you call IOTC_Deinitialize parallelly.
 *
 */
P2PAPI_API int32_t IOTC_DeInitialize(void);

/**
* \brief Used by a client to get a tutk_platform_free session ID.
*
* \details This function is for a client to get a tutk_platform_free session ID used for a parameter of
*          IOTC_Connect_ByUID_Parallel().
*
* \return IOTC session ID if return value >= 0
* \return Error code if return value < 0
*            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
*            - #IOTC_ER_EXCEED_MAX_SESSION The number of IOTC sessions has reached maximum in client side
*
* \attention (1) If you already get a session ID via this function must call IOTC_Connect_ByUID_Parallel() and then
*                it will release session ID resource automatically when connection failed.<br><br>
*            (2) If you already get a session ID via this function and want to close it please use IOTC_Session_Close().
*
*/
P2PAPI_API int32_t IOTC_Get_SessionID(void);

/**
 * \brief Used by a device or a client to read data from the other
 *
 * \details A device or a client uses this function to read data through
 *          a specific IOTC channel in a IOTC session. <br>
 *          The difference between this function and IOTC_Session_Read() is
 *          this function provides packet lost information. Users may use
 *          this to check how many packets, if any, have been lost since the last
 *          time reading from this session.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to read data
 * \param abBuf [out] The array of byte buffer to receive read result
 * \param nMaxBufSize [in] The maximum length of the byte buffer
 * \param nTimeout [in] The timeout for this function in unit of millisecond, give 0 means return immediately
 * \param pnPacketSN [out] The serial number of the packet that is read successfully this time. Could be NULL.
 * \param pbFlagLost [out] A boolean value to indicate if there are some packets lost between this time and previous successful read. Could be NULL.
 * \param nIOTCChannelID [in] The IOTC channel ID in this IOTC session to read data
 *
 * \return The actual length of read result stored in abBuf if read successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_CH_NOT_ON The IOTC channel of specified channel ID is not turned on
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_TIMEOUT The timeout specified by nTimeout expires before read process is performed completely
 *
 * \attention (1) The IOTC channel of ID 0 is enabled by default when a IOTC session is established.
 *                That means nIOTCChannelID can be specified as 0 if only one IOTC channel
 *                is needed by the user. If more IOTC channels are required, users
 *                should use IOTC_Session_Get_Free_Channel() and IOTC_Session_Channel_ON()
 *                to get more IOTC channel IDs and then specifying those IOTC channel IDs
 *                in this function according to the purpose defined by the user.
 *            (2) This API is a blocking function. This function will wait until
 *                the specific IOTC channel in a IOTC session has data or some error happens
 *                during the read data process
 *            (3) Recommended value of timeout: > 1000 millisecond
 *            (4) This API has been deprecated and might be removed in the next version.
 *                Please replace it IOTC_Session_Read.
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Session_Read_Check_Lost(int32_t nIOTCSessionID, char *abBuf, int32_t nMaxBufSize,uint32_t nTimeout, uint16_t *pnPacketSN, int8_t *pbFlagLost, uint8_t nIOTCChannelID);

/**
 * \brief Used by a device or a client to check the IOTC session info
 *
 * \details A device or a client may use this function to check if the IOTC session
 *            is still alive as well as getting the IOTC session info.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to be checked
 * \param psSessionInfo [out] The session info of specified IOTC session
 *
 * \return #IOTC_ER_NoERROR if getting the IOTC session info successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_INVALID_ARG The arguments passed in to this function is invalid.
 *
 * \attention (1) This API has been deprecated and might be removed in the next version.
 *                Please replace it with IOTC_Session_Check_Ex.
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Session_Check(int32_t nIOTCSessionID, struct st_SInfo *psSessionInfo);

/**
 * \brief Used by a device or a client to check the IOTC session info
 *
 * \details A device or a client may use this function to check if the IOTC session
 *          is still alive as well as getting the IOTC session info. The structure
 *          size must be filled. For example "info.size=sizeof(struct st_SInfoEx);"
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to be checked
 * \param psSessionInfo [out] The session info of specified IOTC session
 *
 * \return #IOTC_ER_NoERROR if getting the IOTC session info successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_INVALID_ARG The size of structure is not filled
 */
P2PAPI_API int32_t IOTC_Session_Check_Ex(int32_t nIOTCSessionID, struct st_SInfoEx *psSessionInfo);

/**
 * \brief Used by a device or a client to get the session status
 *
 * \details This function gets the session status between device and client. <br>
 *          The difference of this function and IOTC_Session_Check() is
 *          this function will set callback function inside IOTC module and
 *          that callback function will be invoked whenever the session status
 *          between device and client disconnected, for example, IOTC
 *          alive timeout or one side call IOTC_Session_Close() to close
 *          this session.

 * \param nIOTCSessionID [in] The session ID of the IOTC session to check status
 * \param pfxSessionStatusFn [in] The function pointer to getting session status function
 *
 * \return #IOTC_ER_NoERROR if getting the IOTC session info successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *
 *\attention (1) This API has been deprecated and might be removed in the next version.
 *               Please replace it with IOTC_Session_Check_Ex.
 */
P2PAPI_API_DEPRECATED int32_t IOTC_Session_Check_ByCallBackFn(int32_t nIOTCSessionID, sessionStatusCB pfxSessionStatusFn);

/**
 * \brief Used by a device or a client to read data from the other
 *
 * \details A device or a client uses this function to read data through a specific IOTC channel in a IOTC session.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to read data
 * \param abBuf [out] The array of byte buffer to receive read result
 * \param nMaxBufSize [in] The maximum length of the byte buffer
 * \param nTimeout [in] The timeout for this function in unit of millisecond, give 0 means return immediately
 * \param nIOTCChannelID [in] The IOTC channel ID in this IOTC session to read data
 *
 * \return The actual length of read result stored in abBuf if read successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_CH_NOT_ON The IOTC channel of specified channel ID is not turned on
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_TIMEOUT The timeout specified by nTimeout expires before read process is performed completely
 *
 * \attention (1) The IOTC channel of ID 0 is enabled by default when a IOTC session is established.
 *                That means nIOTCChannelID can be specified as 0 if only one IOTC channel
 *                is needed by the user. If more IOTC channels are required, users
 *                should use IOTC_Session_Get_Free_Channel() and IOTC_Session_Channel_ON()
 *                to get more IOTC channel IDs and then specifying those IOTC channel IDs
 *                in this function according to the purpose defined by the user.<br><br>
 *            (2) If the size of abBuf, i.e. defined by nMaxBufSize, is less than
 *                the size of data to be read, then this function will only read
 *                the data up to nMaxBufSize and the remaining part will be truncated without
 *                error code returned. Therefore, it is suggested to allocate the size
 *                of abBuf as #IOTC_MAX_PACKET_SIZE for ensure complete reading.
 *            (3) This function will need 192 bytes stack size.
 *            (4) This API is a blocking function. This function will wait until
 *                the specific IOTC channel in a IOTC session has data or some error happens
 *                during the read data process
 *            (5) Recommended value of timeout: > 1000 millisecond
 *
 */
P2PAPI_API int32_t IOTC_Session_Read(int32_t nIOTCSessionID, char *abBuf, int32_t nMaxBufSize, uint32_t nTimeout, uint8_t nIOTCChannelID);

/**
 * \brief Used by a device or a client to write data to the other
 *
 * \details A device or a client uses this function to write data through
 *            a specific IOTC channel in a IOTC session.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to write data
 * \param cabBuf [in] The array of byte buffer containing the data to write.
 *            Its size cannot be larger than #IOTC_MAX_PACKET_SIZE
 * \param nBufSize [in] The length of the byte buffer. It cannot be larger than
 *            #IOTC_MAX_PACKET_SIZE
 * \param nIOTCChannelID [in] The IOTC channel ID in this IOTC session to write data
 *
 * \return The actual length of buffer to be written if write successfully. In non-blocking
 *         mode, the length with zero usually means the socket buffer is full and unable to
 *         write into.
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_CH_NOT_ON The IOTC channel of specified channel ID is not turned on
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_INVALID_ARG The buffer size is not accepted
 *            - #IOTC_ER_NO_PATH_TO_WRITE_DATA IOTC internal error, cannot find a path to send data
 *
 * \attention   (1) The IOTC channel of ID 0 is enabled by default when a IOTC session is established.
 *                  That means nIOTCChannelID can be specified as 0 if only one IOTC channel
 *                  is needed by the user. If more IOTC channels are required, users
 *                  should use IOTC_Session_Get_Free_Channel() and IOTC_Session_Channel_ON()
 *                  to get more IOTC channel IDs and then specifying those IOTC channel IDs
 *                  in this function according to the purpose defined by the user. <br><br>
 *              (2) This function will need 4976 bytes stack size.
 *
 */
P2PAPI_API int32_t IOTC_Session_Write(int32_t nIOTCSessionID, const char *cabBuf, int32_t nBufSize, uint8_t nIOTCChannelID);

/**
 * \brief Used by a device or a client to close a IOTC session
 *
 * \details A device or a client uses this function to close a IOTC session
 *          specified by its session ID if this IOTC session is no longer
 *          required. If the closed SID is used for other APIs, the API should
 *          return IOTC_ER_INVALID_SID.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to be closed
 *
 */
P2PAPI_API void IOTC_Session_Close(int32_t nIOTCSessionID);

/**
 * \brief Used by a device or a client to get a tutk_platform_free IOTC channel
 *
 * \details A device or a client uses this function to get a tutk_platform_free IOTC channel
 *          in a specified IOTC session. By default, IOTC channel of ID 0 is turned on
 *          once a IOTC session is established. If more IOTC channels are required
 *          by users, this function can always return a tutk_platform_free IOTC channel until
 *          maximum IOTC channels are reached.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to get tutk_platform_free IOTC channel
 *
 * \return The IOTC channel ID of a tutk_platform_free IOTC channel if successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_SESSION_NO_FREE_CHANNEL Already reach the maximum number of IOTC channels, no more tutk_platform_free IOTC channel is available
 *
 * \attention (1) The IOTC channel returned by this function is already turned on.<br><br>
 *            (2) The IOTC channel is only turned on in the local site
 *                calling this function. That means, the remote site shall use
 *                IOTC_Session_Channel_ON() to turn on the same IOTC channel at its
 *                side before communication.
 */
P2PAPI_API int32_t IOTC_Session_Get_Free_Channel(int32_t nIOTCSessionID);

/**
 * \brief Used by a device or a client to turn on a IOTC channel
 *
 * \details A device or a client uses this function to turn on a IOTC channel
 *          before sending or receiving data through this IOTC channel.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session containing the
 *        IOTC channel to be turned on
 * \param nIOTCChannelID [in] The channel ID of the IOTC channel to be turned on
 *
 * \return IOTC_ER_NoERROR if turning on the IOTC channel successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_CH_NOT_ON The specified IOTC channel ID is not valid
 *
 * \attention The IOTC channel is only turned on in the local site calling
 *                this function. That means, the remote site shall also use
 *                IOTC_Session_Channel_ON() to turn on the same IOTC channel
 *                at its side before communication.
 */
P2PAPI_API int32_t IOTC_Session_Channel_ON(int32_t nIOTCSessionID, uint8_t nIOTCChannelID);

/**
 * \brief Used by a device or a client to turn off a IOTC channel
 *
 * \details A device or a client uses this function to turn off a IOTC channel
 *          when this IOTC channel is no longer needed for communication.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session containing the
 *        IOTC channel to be turned off
 * \param nIOTCChannelID [in] The channel ID of the IOTC channel to be turned off
 *
 * \return IOTC_ER_NoERROR if turning off the IOTC channel successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_CH_NOT_ON The specified IOTC channel ID is not valid
 *
 * \attention (1) IOTC Channel 0 cannot be turned off because it is a default channel.<br><br>
 *            (2) Turn off a IOTC channel will also make the data remaining
 *                in receiving queue of this channel be deleted.
 */
P2PAPI_API int32_t IOTC_Session_Channel_OFF(int32_t nIOTCSessionID, uint8_t nIOTCChannelID);

/**
 * \brief Used by a device or a client to check whether the IOTC channel is ON or OFF
 *
 * \details A device or a client uses this function to turn on a IOTC channel
 *          before sending or receiving data through this IOTC channel.
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session containing the
 *        IOTC channel to be checked
 * \param nIOTCChannelID [in] The channel ID of the IOTC channel to be checked
 *
 * \return 1 for channel ON return 0 for channel OFF
 * \return Error code if return value < 0
 *          - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *          - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *          - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *          - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *          - #IOTC_ER_INVALID_ARG The specified IOTC channel ID is not valid
 *
 */
P2PAPI_API int32_t IOTC_Session_Channel_Check_ON_OFF(int32_t nIOTCSessionID, uint8_t nIOTCChannelID);

/**
 * \brief Used by search devices in LAN.
 *
 * \details When clients and devices are stay in a LAN environment, client can call this function
 *          to discovery devices and connect it directly.
 *
 * \param psLanSearchInfo [in] The array of struct st_LanSearchInfo to store search result
 *
 * \param nArrayLen [in] The size of the psLanSearchInfo array
 *
 * \param nWaitTimeMs [in] The timeout in milliseconds before discovery process end.
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
P2PAPI_API_DEPRECATED int32_t IOTC_Lan_Search(struct st_LanSearchInfo *psLanSearchInfo, int32_t nArrayLen, int32_t nWaitTimeMs);

/**
 * \brief Set path of log file
 *
 * \details Set the absolute path of log file
 *
 * \param path [in] The path of log file, NULL = disable Log
 *
 * \param nMaxSize [in] The maximum size of log file in Bytes, 0 = unlimited
 *
 */
P2PAPI_API_DEPRECATED void IOTC_Set_Log_Path(char *path, int32_t nMaxSize);

/**
 * \brief Set Attribute of log file
 *
 * \param logAttr [in] See #LogAttr
 *
 * \return IOTC_ER_NoERROR on success.
 * \return The value < 0
 *            - IOTC_ER_INVALID_ARG   Invalid input argument.
 */
P2PAPI_API int32_t IOTC_Set_Log_Attr(LogAttr logAttr);

/**
 * \brief Set partial encode On/Off
 *
 * \details Set partial encode On/Off
 *
 * \param nIOTCSessionID [in] The session ID of the IOTC session to be encrypted.
 *
 * \param bPartialEncryption [in] 1: Enable partial encode, 0: Disable partial encode
 *
 * \return Error code if return value < 0
 *            - #IOTC_ER_NOT_INITIALIZED The IOTC module is not initialized yet
 *            - #IOTC_ER_INVALID_SID The specified IOTC session ID is not valid
 *            - #IOTC_ER_SESSION_CLOSE_BY_REMOTE The IOTC session of specified session ID has been closed by remote site
 *            - #IOTC_ER_REMOTE_TIMEOUT_DISCONNECT The timeout defined by #IOTC_SESSION_ALIVE_TIMEOUT expires because remote site has no response
 *            - #IOTC_ER_NOT_SUPPORT_PE The remote device don't support partial encryption
 *
 */
P2PAPI_API int32_t IOTC_Set_Partial_Encryption(int32_t nIOTCSessionID, uint8_t bPartialEncryption);

/**
 * \brief Turn on only TCP relay mode function.
 *
 * \details When device call this will login via TCP and only can be connected via LAN or TCP relay mode.
 *          When client call this will connect to device only via LAN or TCP relay mode.
 *
 * \attention Can be called anywhere.
 *
 */
P2PAPI_API void IOTC_TCPRelayOnly_TurnOn(void);

/**
 * \brief ReInitialize IOTC module socket
 *
 * \details This function will reopen P2P UDP socket of IOTC module.
 *          It is useful for iOS APP from backgroud to foreground will cause socket failed.
 *
 * \param nUDPPort [in] Specify a UDP port. Random UDP port is used if it is specified as 0.
 *
 * \return #IOTC_ER_NoERROR if initializing successfully
 * \return Error code if return value < 0
 *            - #IOTC_ER_FAIL_CREATE_SOCKET Fails to create sockets
 *            - #IOTC_ER_FAIL_SOCKET_OPT Fails to set up socket options
 *            - #IOTC_ER_FAIL_SOCKET_BIND Fails to bind sockets
 *
 *
 * \attention (1) Must call it before any connection process.
 *            (2) This API can only be used in client side
 *
 */
P2PAPI_API int32_t IOTC_ReInitSocket(uint16_t nUDPPort);

/**
* \brief Setup session alive timeout value
*
* \details the default alive timeout value is 60 seconds since version 1.14.20.0,
*          this function could help you to change the alive timeout value
*
* \param nTimeout [in] The timeout for this function in unit of second
*
* \attention (1) Must be set on client and device with the same value. Please use the function before
*                IOTC_initialize or IOTC_initialize2. The minimum value is 1.
*
*            (2) Recommended value of timeout: > 30 secs
*/
P2PAPI_API void IOTC_Setup_Session_Alive_Timeout(uint32_t nTimeout);

/**
* \brief Change default LAN search port
*
* \details Both device and client must have the same LAN search port, otherwise client cannot find
*          device by IOTC_Device_Search_Start or IOTC_LAN_Search
*
* \param nPort [in] Pass the customized LAN search port
*
* \attention The IOTC_Set_LanSearchPort must be called before IOTC_Initialize or IOTC_Initialize2
*
* \return Return IOTC_ER_NoERROR if successfully.
* \return Error code if return value < 0
*            - #IOTC_ER_INVALID_ARG The arguments passed in to this function is invalid or port number > 65535.
**/
P2PAPI_API int32_t IOTC_Set_LanSearchPort(uint32_t nPort);

/**
* \brief Change IOTC master region
*
* \param region [in] The master region to be used.
*
* \attention Need to be called before #IOTC_Initialize #IOTC_Initialize2
*
* \return IOTC_ER_NoERROR if successfully set master region.
* \return Error code if return value < 0
*           - #IOTC_ER_INVALID_ARG if the region is invalid
*           - #IOTC_ER_ALREADY_INITIALIZED IOTC module is already initialized
* \attention (1) This API has been deprecated and might be removed in the next version,
*                please use TUTK_SDK_Set_Region in TUTKGlobalAPIs.h to replace this API
**/
P2PAPI_API_DEPRECATED int32_t IOTC_Set_Master_Region(TUTKRegion region);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _IOTCCOMMON_H_ */
