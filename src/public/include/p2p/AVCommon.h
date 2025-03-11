/*! \file AVCommon.h
This file describes common AV module APIs of server and client.

\copyright Copyright (c) 2010 by Throughtek Co., Ltd. All Rights Reserved.
 */

#ifndef _AVCOMMON_H_
#define _AVCOMMON_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "TUTKGlobalAPIs.h"
#include "NebulaJsonAPIs.h"


/* ============================================================================
 * Platform Dependant Macro Definition
 * ============================================================================
 */

#ifdef _WIN32
    /** @cond */
    #ifdef IOTC_STATIC_LIB
        #define AVAPI_API
    #elif defined AVAPI_EXPORTS
        #define AVAPI_API __declspec(dllexport)
    #else
        #define AVAPI_API __declspec(dllimport)
    #endif
    /** @endcond */
#else
	#define AVAPI_API
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define AVAPI_API_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
    #ifdef IOTC_STATIC_LIB
        #define AVAPI_API_DEPRECATED __declspec(deprecated)
    #elif defined P2PAPI_EXPORTS
        #define AVAPI_API_DEPRECATED __declspec(deprecated, dllexport)
    #else
        #define AVAPI_API_DEPRECATED __declspec(deprecated, dllimport)
    #endif
#else
    #define AVAPI_API_DEPRECATED
#endif

#ifndef _WIN32
    #define __stdcall
#endif


/* ============================================================================
 * Generic Macro Definition
 * ============================================================================
 */

/** The maximum size of AV IO Control data to be sent, in unit of byte. */
#define AV_MAX_IOCTRL_DATA_SIZE					1024

/** The maximum size of audio data to be sent, in unit of byte. */
#define AV_MAX_AUDIO_DATA_SIZE					1280

/** The maximum length of identity, exclude the Null character '\0', in unit of byte.
 *  If DTLS security mode is enabled, or auth type is set to AV_AUTH_TOKEN,
 *  the the max lenth of account or identity should not exceed this value.
 */
#define MAX_IDENTITY_LENGTH                     119

/** The maximum length of identity description, exclude the Null character '\0', in unit of byte. */
#define MAX_IDENTITY_DESCRIPTION_LENGTH         127

/** The maximum length of token, exclude the Null character character '\0', in unit of byte. */
#define MAX_TOKEN_LENGTH                        1024

/** The maximum length of account, include the Null character '\0', in unit of byte.
 *  If security mode is AV_SECURITY_SIMPLE, or avClientStart/avClientStart2 is used,
 *  the the max lenth of account should not exceed this value.
 */
#define NEW_MAXSIZE_VIEWACC                     257

/** The maximum length of password, include the Null character '\0', in unit of byte. */
#define NEW_MAXSIZE_VIEWPWD                     257
#define OLD_MAXSIZE_VIEWACC                     16
#define OLD_MAXSIZE_VIEWPWD                     16

/** The maximum length of timestamp, exclude the Null character character '\0', in unit of byte. */
#define MAX_TIMESTAMP_LENGTH                    16


/* ============================================================================
 * Error Code Declaration
 * ============================================================================
 */
/** The function is performed successfully. */
#define		AV_ER_NoERROR						 0

/** The passed-in arguments for the function are incorrect */
#define		AV_ER_INVALID_ARG					-20000

/** The buffer to receive is too small to store */
#define		AV_ER_BUFPARA_MAXSIZE_INSUFF		-20001

/** The number of AV channels has reached maximum.
 * The maximum number of AV channels is determined by the passed-in
 * argument of avInitialize() */
#define		AV_ER_EXCEED_MAX_CHANNEL			-20002

/** Insufficient memory for allocation */
#define		AV_ER_MEM_INSUFF					-20003

/** AV fails to create threads. Please check if OS has ability to create threads for AV. */
#define		AV_ER_FAIL_CREATE_THREAD			-20004

/** A warning error code to indicate that the sending queue of video frame of an AV server
 * is almost full, probably caused by slow handling of an AV client or network
 * issue. Please note that this is just a warning, the video frame is actually
 * put in the queue. */
#define 	AV_ER_EXCEED_MAX_ALARM				-20005

/** The frame to be sent exceeds the currently remaining video frame buffer.
 * The maximum of video frame buffer is controlled by avServSetResendSize() */
#define		AV_ER_EXCEED_MAX_SIZE				-20006

/** The specified AV server has no response */
#define		AV_ER_SERV_NO_RESPONSE				-20007

/** An AV client does not call avClientStart() yet */
#define		AV_ER_CLIENT_NO_AVLOGIN				-20008

/** The client fails in authentication due to incorrect view account or password */
#define		AV_ER_WRONG_VIEWACCorPWD			-20009

/** The IOTC session of specified AV channel is not valid */
#define		AV_ER_INVALID_SID					-20010

/** The specified timeout has expired during some operation */
#define		AV_ER_TIMEOUT						-20011

/** The data is not ready for receiving yet. */
#define		AV_ER_DATA_NOREADY					-20012

/** Some parts of a frame are lost during receiving */
#define		AV_ER_INCOMPLETE_FRAME				-20013

/** The whole frame is lost during receiving */
#define		AV_ER_LOSED_THIS_FRAME				-20014

/** The remote site already closes the IOTC session.
 * Please call IOTC_Session_Close() to release local IOTC session resource */
#define		AV_ER_SESSION_CLOSE_BY_REMOTE		-20015

/** This IOTC session is disconnected because remote site has no any response
 * after a specified timeout expires. */
#define		AV_ER_REMOTE_TIMEOUT_DISCONNECT		-20016

/** The AV server API process is terminated by avServExit() or avServStop()*/
#define		AV_ER_SERVER_EXIT		    		-20017

/** The AV client API process is terminated by avClientExit() or avClientStop()*/
#define		AV_ER_CLIENT_EXIT		    		-20018

/** AV module has not been initialized */
#define		AV_ER_NOT_INITIALIZED	    		-20019

/** By design, an AV client cannot send frame and audio data to an AV server */
#define		AV_ER_CLIENT_NOT_SUPPORT	   		-20020

/** The AV channel of specified AV channel ID is already in sending IO control process */
#define		AV_ER_SENDIOCTRL_ALREADY_CALLED	   	-20021

/** The sending IO control process is terminated by avSendIOCtrlExit() */
#define		AV_ER_SENDIOCTRL_EXIT		    	-20022

/** The UID is a lite UID */
#define		AV_ER_NO_PERMISSION                     -20023

/** The length of password is wrong */
#define		AV_ER_WRONG_ACCPWD_LENGTH				-20024

/** IOTC session has been closed */
#define		AV_ER_IOTC_SESSION_CLOSED				-20025

/** IOTC is deinitialized */
#define		AV_ER_IOTC_DEINITIALIZED				-20026

/** IOTC channel is used by other av channel */
#define		AV_ER_IOTC_CHANNEL_IN_USED				-20027

/** AV channel is waiting key frame */
#define		AV_ER_WAIT_KEY_FRAME				    -20028

/** The AV channel of specified AV channel ID is already in reset buffer process */
#define		AV_ER_CLEANBUF_ALREADY_CALLED	   	    -20029

/** IOTC UDP/TCP socket send queue is full. */
#define 	AV_ER_SOCKET_QUEUE_FULL					-20030

/** AV module is already initialized. It is not necessary to re-initialize. */
#define     AV_ER_ALREADY_INITIALIZED               -20031

/** Dynamic Adaptive Streaming over AVAPI notified program to clean buffer */
#define		AV_ER_DASA_CLEAN_BUFFER                 -20032

/** Function is not support, need to using correct AV Server and AV Client  */
#define		AV_ER_NOT_SUPPORT                       -20033

/** AV fails to initialize DTLS module. */
#define     AV_ER_FAIL_INITIALIZE_DTLS              -20034

/** AV fails to create channel for DTLS connection. */
#define     AV_ER_FAIL_CREATE_DTLS                  -20035

/** The AV channel of specified AV channel ID is already in request process */
#define		AV_ER_REQUEST_ALREADY_CALLED	   	    -20036

/** Function is not supported on remote side. */
#define		AV_ER_REMOTE_NOT_SUPPORT                -20037

/** The token to be sent exceeds MAX_TOKEN_LENGTH. */
#define		AV_ER_TOKEN_EXCEED_MAX_SIZE             -20038

/** The server fail because client not support DTLS */
#define     AV_ER_REMOTE_NOT_SUPPORT_DTLS           -20039

/** The server fail in authentication due to incorrect password/token with DTLS security mode enabled*/
#define     AV_ER_DTLS_WRONG_PWD					-20040

/** The server fail in authentication due to the passowrd/token auth function return value is smaller than 0*/
#define     AV_ER_DTLS_AUTH_FAIL					-20041

/** Device don't enable VSaaS pulling*/
#define     AV_ER_VSAAS_PULLING_NOT_ENABLE          -20042

/** Device can't connect to VSaaS Server by https */
#define     AV_ER_FAIL_CONNECT_TO_VSAAS             -20043

/** This JSON format cannot be parsed */
#define     AV_ER_PARSE_JSON_FAIL                   -20044

/** Push notification does not enable */
#define     AV_ER_PUSH_NOTIFICATION_NOT_ENABLE      -20045

/** Push notification already enabled */
#define     AV_ER_PUSH_NOTIFICATION_ALREADY_ENABLED -20046

/** No notification settings */
#define     AV_ER_NO_NOTIFICATION_LIST              -20047

/** An error occurred during the http connection */
#define     AV_ER_HTTP_ERROR                        -20048

/** The client fail because server require DTLS */
#define     AV_ER_LOCAL_NOT_SUPPORT_DTLS            -20049

/** VSaaS server return unexpected error code */
#define AV_ER_VSAAS_UNEXPECTED_ERROR                -20100

/** VSaaS server return invalid parameter */
#define AV_ER_VSAAS_INVALID_PARAMETER               -20101

/** VSaaS server return invalid device */
#define AV_ER_VSAAS_INVALID_DEVICE                  -20102

/** VSaaS server return invalid binding */
#define AV_ER_VSAAS_INVALID_BINDING                 -20103

/** VSaaS server return invalid contract */
#define AV_ER_VSAAS_INVALID_CONTRACT                -20104

/** VSaaS server return invalid start time range */
#define AV_ER_VSAAS_INVALID_START_TIME_RANGE        -20105

/** VSaaS server return invalid start time format */
#define AV_ER_VSAAS_INVALID_START_TIME_FORMAT       -20106

/** VSaaS server return contract not found */
#define AV_ER_VSAAS_CONTRACT_NOT_FOUND              -20107

/** The SDK Version not support DTLS */
#define     AV_ER_SDK_NOT_SUPPORT_DTLS              -21334


/* ============================================================================
 * Enumeration Declaration
 * ============================================================================
 */

/**
 * \details The types for AV IO control. AV IO control are used	for AV servers
 *			and AV clients to communicate control message with each other.
 *			The types contains system defined types and user defined ones.
 *			The range of system defined	types is from 0x00 to 0xFF, while
 *			user defined ones is starting from 0x100.
 */
typedef enum {
	/// Used by AV clients to tell AV servers to change the time interval
	/// of sending packets by avSendFrameData() when AV clients are too slow
	/// to handle frame data.
	IOTYPE_INNER_SND_DATA_DELAY = 0xFF,

    /// Used by AV servers to tell AV clients statistical data
    IOTYPE_INNER_STATISTICAL_DATA = 0x01,

    /// Used by AV clients to tell AV servers statistical data
    IOTYPE_INNER_STATISTICAL_CLIENT_DATA = 0x02,

	/// The starting enum value of user defined IO types. It is suggested
	/// to assign this to the first one of user defined types, like <br>
	/// enum IOTYPE_USER_DEFINED { <br>
	/// IOTYPE_USER_PLAY = IOTYPE_USER_DEFINED_START, <br>
	/// IOTYPE_USER_STOP <br>
	/// }
	IOTYPE_USER_DEFINED_START = 0x100
} AVIOCtrlType;

typedef enum AvSecurityMode {
	AV_SECURITY_SIMPLE,
	AV_SECURITY_DTLS,

    /// Only can be used by avClientStartEx,
    /// client will detect the security mode
    /// of av server and connect it
    AV_SECURITY_AUTO,
} AvSecurityMode;

typedef enum AvAuthType {
	AV_AUTH_PASSWORD,
	AV_AUTH_TOKEN,
} AvAuthType;


/* ============================================================================
 * Type Definition
 * ============================================================================
 */

typedef struct AvIdentity {
    uint32_t length;
    char identity[MAX_IDENTITY_LENGTH + 1];
} AvIdentity;


/* ============================================================================
 * Function Declaration
 * ============================================================================
 */

/**
 * \brief Get the version of AV module
 *
 * \details This function returns the version of AV module.
 *
 * \return The version of AV module from high byte to low byte, for example,
 *			0x01020304 means the version is 1.2.3.4
 *
 * \see IOTC_Get_Version(), RDT_GetRDTApiVer()
 */
AVAPI_API_DEPRECATED int32_t avGetAVApiVer(void);

/**
 * \brief Get the version of AV module
 *
 * \details This function returns the version of AV module.
 *
 */
AVAPI_API const char* avGetAVApiVersionString(void);

/**
 * \brief Initialize AV module
 *
 * \details This function is used by AV servers or AV clients to initialize AV
 *			module and shall be called before any AV module related function
 *			is invoked.
 *
 * \param nMaxChannelNum [in] The max number of AV channels. If it is
 *			specified less than 1, AV will set max number of AV channels as 1.
 *
 * \return The actual maximum number of AV channels to be set.
 * \return Error code if return value < 0
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #TUTK_ER_NO_LICENSE_KEY Need to call TUTK_SDK_Set_License_Key() before all module start/initialize.
 *
 * \see TUTK_SDK_Set_License_Key()
 *
 */
AVAPI_API int32_t avInitialize(int32_t nMaxChannelNum);

/**
 * \brief Deinitialize AV module
 *
 * \details This function will deinitialize AV module.
 *
 * \return #AV_ER_NoERROR if deinitialize successfully
 * \return Error code if return value < 0
 *			- #AV_ER_NOT_INITIALIZED the AV module is not initialized yet
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) AV module shall be deinitialized before IOTC module is
 *				deinitialized.
 */
AVAPI_API int32_t avDeInitialize(void);

/**
 * \brief An AV server sends frame data to an AV client
 *
 * \details An AV server uses this function to send frame data to AV client
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be sent
 * \param cabFrameData [in] The frame data to be sent
 * \param nFrameDataSize [in] The size of the frame data
 * \param cabFrameInfo [in] The video frame information to be sent
 * \param nFrameInfoSize [in] The size of the video frame information
 *
 * \return #AV_ER_NoERROR if sending successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
 *			- #AV_ER_CLIENT_NOT_SUPPORT An AV client uses this function to send frame data
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_CLIENT_NO_AVLOGIN An AV client does not pass authentication yet
 *			- #AV_ER_EXCEED_MAX_SIZE The frame data and frame info to be sent exceeds
 *				currently remaining video frame buffer. The max size of video frame
 *				buffer is determined by avServSetResendSize()
 *			- #AV_ER_MEM_INSUFF Insufficient memory for allocation
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_SESSION_CLOSED the local IOTC session has been closed
 *			- #AV_ER_IOTC_DEINITIALIZED IOTC has been deinitialized
 *          - #AV_ER_DASA_CLEAN_BUFFER This error returned when DASA is enabled, that notify the user this AV channel need to do clean buffer.
 *              This error is just a notice, the frame data to be send still put in the video frame buffer
 *
 * \see avSendAudioData()
 *
 * \attention (1) If av client and av server both version are equal to or bigger than 3.1.10,
 *                this API could be used by both av client and av server,
 *                otherwise it can only be used by av server
 */
AVAPI_API int32_t avSendFrameData(int32_t nAVChannelID, const char *cabFrameData, int32_t nFrameDataSize,
                              const void *cabFrameInfo, int32_t nFrameInfoSize);

/**
 * \brief An AV server sends audio data to an AV client
 *
 * \details An AV server uses this function to send audio data to AV client
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be sent
 * \param cabAudioData [in] The audio data to be sent
 * \param nAudioDataSize [in] The size of the audio data
 * \param cabFrameInfo [in] The audio frame information to be sent
 * \param nFrameInfoSize [in] The size of the audio frame information
 *
 * \return #AV_ER_NoERROR if sending successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
 *			- #AV_ER_CLIENT_NOT_SUPPORT An AV client uses this function to send frame data
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_CLIENT_NO_AVLOGIN An AV client does not pass authentication yet
 *			- #AV_ER_MEM_INSUFF Insufficient memory for allocation
 *			- #AV_ER_EXCEED_MAX_SIZE The audio data and frame info to be sent exceeds
 *				#AV_MAX_AUDIO_DATA_SIZE
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_SESSION_CLOSED the local IOTC session has been closed
 *			- #AV_ER_IOTC_DEINITIALIZED IOTC has been deinitialized
 *
 * \see avSendFrameData()
 *
* \attention (1) If av client and av server both version are equal to or bigger than 3.1.10,
 *                this API could be used by both av client and av server,
 *                otherwise it can only be used by av server
 */
AVAPI_API int32_t avSendAudioData(int32_t nAVChannelID, const char *cabAudioData, int32_t nAudioDataSize,
                              const void *cabFrameInfo, int32_t nFrameInfoSize);

/**
 * \brief A new version AV client receives frame data from an AV server
 *
 * \details An AV client uses this function to receive frame data from AV server
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be received
 * \param abFrameData [out] The frame data to be received
 * \param nFrameDataMaxSize [in] The max size of the frame data
 * \param pnActualFrameSize [in] The actual size of frame data to be received, maybe less than expected size
 * \param pnExpectedFrameSize [in] The size of frame data expect to be received that sent from av server
 * \param abFrameInfo [out] The video frame information to be received
 * \param nFrameInfoMaxSize [in] The max size of the video frame information
 * \param pnActualFrameInfoSize [in] The actual size of the video frame information to be received
 * \param pnFrameIdx [out] The index of current receiving video frame
 *
 * \return The actual length of received result stored in abFrameData if successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_DATA_NOREADY The data is not ready for receiving yet.
 *			- #AV_ER_LOSED_THIS_FRAME The whole frame is lost during receiving
 *			- #AV_ER_BUFPARA_MAXSIZE_INSUFF The frame to be received exceeds
 *				the size of abFrameData, i.e. nFrameDataMaxSize
 *			- #AV_ER_MEM_INSUFF Insufficient memory for allocation
 *			- #AV_ER_INCOMPLETE_FRAME Some parts of a frame are lost during receiving
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_SESSION_CLOSED the local IOTC session has been closed
 *			- #AV_ER_IOTC_DEINITIALIZED IOTC has been deinitialized
 *
 * \see avRecvAudioData()
 *
 * \attention (1) If av client and av server both version are equal to or bigger than 3.1.10,
 *                this API could be used by both av client and av server,
 *                otherwise it can only be used by av client
 * \attention (2) If av client set sync_recv_data = 1 and the sync_recv_data in return AVClientOutConfig equal to 1,
 *                then av client will get synchronized video/audio data via avRecvFrameData2() and avRecvAudioData().
 *                If user stop calling avRecvFrameData2() to receive latest video data when sync receive data is enabled,
 *                it will result in data blocking, then avRecvAudioData() will not receive any audio data.
 */
AVAPI_API int32_t avRecvFrameData2(int32_t nAVChannelID, char *abFrameData, int32_t nFrameDataMaxSize, int32_t *pnActualFrameSize,
                               int32_t *pnExpectedFrameSize, char *abFrameInfo, int32_t nFrameInfoMaxSize,
                               int32_t *pnActualFrameInfoSize, uint32_t *pnFrameIdx);

/**
 * \brief An AV client receives audio data from an AV server
 *
 * \details An AV client uses this function to receive audio data from AV server
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to be received
 * \param abAudioData [out] The audio data to be received
 * \param nAudioDataMaxSize [in] The max size of the audio data
 * \param abFrameInfo [out] The audio frame information to be received
 * \param nFrameInfoMaxSize [in] The max size of the audio frame information
 * \param pnFrameIdx [out] The index of current receiving audio frame
 *
 * \return The actual length of received result stored in abAudioData if successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or frame data is null
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_DATA_NOREADY The data is not ready for receiving yet.
 *			- #AV_ER_LOSED_THIS_FRAME The whole frame is lost during receiving
 *			- #AV_ER_BUFPARA_MAXSIZE_INSUFF The data and frame info to be received
 *				exceeds	the size of abAudioData and abFrameInfo, respectively.
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_SESSION_CLOSED the local IOTC session has been closed
 *			- #AV_ER_IOTC_DEINITIALIZED IOTC has been deinitialized
 *
 * \see avRecvFrameData2()
 *
 * \attention (1) If av client and av server both version are equal to or bigger than 3.1.10,
 *                this API could be used by both av client and av server,
 *                otherwise it can only be used by av client
 * \attention (2) If av client set sync_recv_data = 1 and the sync_recv_data in return AVClientOutConfig equal to 1,
 *                then av client will get synchronized video/audio data via avRecvFrameData2() and avRecvAudioData().
 *                If user stop calling avRecvAudioData() to receive latest audio data when sync receive data is enabled,
 *                it will result in data blocking, then avRecvFrameData2() will not receive any video data.
 */
AVAPI_API int32_t avRecvAudioData(int32_t nAVChannelID, char *abAudioData, int32_t nAudioDataMaxSize,
                              char *abFrameInfo, int32_t nFrameInfoMaxSize, uint32_t *pnFrameIdx);

/**
 * \brief Send AV IO control
 *
 * \details This function is used by AV servers or AV clients to send a
 *			AV IO control.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to send IO control
 * \param nIOCtrlType [in] The type of IO control
 * \param cabIOCtrlData [in] The buffer of IO control data
 * \param nIOCtrlDataSize [in] The length of IO control data
 *
 * \return #AV_ER_NoERROR if sending successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid or the nIOCtrlMaxDataSize is less than 0.
 *			- #AV_ER_SENDIOCTRL_ALREADY_CALLED This AV channel is already in sending
 *				IO control process
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_SENDIOCTRL_EXIT avSendIOCtrlExit() is called before this
 *				function is returned
 *			- #AV_ER_EXCEED_MAX_SIZE The IO control data and type to be sent exceeds
 *				#AV_MAX_IOCTRL_DATA_SIZE
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_SESSION_CLOSED the local IOTC session has been closed
 *			- #AV_ER_IOTC_DEINITIALIZED IOTC has been deinitialized
 *          - #AV_ER_SERVER_EXIT avServStop() is called before this function is returned
 *          - #AV_ER_CLIENT_EXIT avClientStop() is called before this function is returned
 *
 * \see avRecvIOCtrl(), avSendIOCtrlExit()
 *
 * \attention (1) This API is a blocking function and it will wait until
 *              having acknowledgment from the receiver.
 *            (2) On Big-endian device, users should handle the byte order translation of cabIOCtrlData,
 *                TUTK SDK will handle the byte order translation of nIOCtrlType in AVAPI.
 *            (3) The nIOCtrlDataSize value must be less than 131068 bytes.
 */
AVAPI_API int32_t avSendIOCtrl(int32_t nAVChannelID, uint32_t nIOCtrlType, const char *cabIOCtrlData, int32_t nIOCtrlDataSize);

/**
 * \brief Receive AV IO control
 *
 * \details This function is used by AV servers or AV clients to receive a
 *			AV IO control.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel to receive IO control
 * \param pnIOCtrlType [out] The type of received IO control
 * \param abIOCtrlData [out] The buffer of received IO control data
 * \param nIOCtrlMaxDataSize [in] The max length of buffer of received IO control data
 * \param nTimeout [in] The timeout for this function in unit of million-second, give 0 means return immediately
 *
 * \return The actual length of received result stored in abIOCtrlData if
 *			receiving IO control successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid, the IO contrl type data is null or the nIOCtrlMaxDataSize is less than 0.
 *			- #AV_ER_SESSION_CLOSE_BY_REMOTE The remote site already closes
 *				this IOTC session
 *			- #AV_ER_REMOTE_TIMEOUT_DISCONNECT The timeout expires because
 *				remote site has no response.
 *			- #AV_ER_DATA_NOREADY The IO control is not ready for receiving yet.
 *			- #AV_ER_TIMEOUT The timeout specified by nTimeout expires before
 *				receiving IO control process is performed completely
 *			- #AV_ER_BUFPARA_MAXSIZE_INSUFF The IO control data to be received exceeds
 *				the size of abIOCtrlData, i.e. nIOCtrlMaxDataSize
 *			- #AV_ER_NO_PERMISSION the AV module does not support Lite UID
 *			- #AV_ER_IOTC_SESSION_CLOSED the local IOTC session has been closed
 *			- #AV_ER_IOTC_DEINITIALIZED IOTC has been deinitialized
 *          - #AV_ER_SERVER_EXIT avServStop() is called before this function is returned
 *          - #AV_ER_CLIENT_EXIT avClientStop() is called before this function is returned
 *
 * \attention (1) This API is a blocking function.<br><br>
 *            (2) Recommended value of timeout: > 1000 millisecond
 *            (3) On Big-endian device, users should handle the byte order translation of abIOCtrlData,
 *                TUTK SDK will handle the byte order translation of pnIOCtrlType in AVAPI.
 *
 * \see avSendIOCtrl()
 *
 */
AVAPI_API int32_t avRecvIOCtrl(int32_t nAVChannelID, uint32_t *pnIOCtrlType, char *abIOCtrlData, int32_t nIOCtrlMaxDataSize, uint32_t nTimeout);

/**
 * \brief Used by an AV server or an AV client to exit sending IO control process
 *
 * \details Since avSendIOCtrl() process is a block process and that means
 *			the caller has to wait for the acknowledgment from the receipt
 *			before avSendIOCtrl() returns. In some cases, caller may want
 *			to exit sending immediately by this function in another thread.
 *
 * \param nAVChannelID [in] The channel ID of the AV channel in sending IO control
 *
 * \return #AV_ER_NoERROR if sending successfully
 * \return Error code if return value < 0
 *			- #AV_ER_INVALID_ARG The AV channel ID is not valid
 */
AVAPI_API int32_t avSendIOCtrlExit(int32_t nAVChannelID);

/**
 * \brief Set path of log file
 *
 * \details Set the absolute path of log file
 *
 * \param path [in] The path of log file, NULL = disable Log
 *
 * \param nMaxSize [in] The maximum size of log file in Bytes, 0 = unlimit
 *
 */
AVAPI_API_DEPRECATED void AV_Set_Log_Path(char *path, int32_t nMaxSize);

/**
 * \brief Set Attribute of log file
 *
 * \param logAttr [in] See #LogAttr
 *
 * \return AV_ER_NoERROR on success.
 * \return The value < 0
 *			- AV_ER_INVALID_ARG   Invalid input argument.
 */
AVAPI_API int32_t AV_Set_Log_Attr(LogAttr logAttr);

/**
 * \brief Set Audio Lost Condition
 *
 * \param lostCondition [in] maximum audio frame number different using in check audio lost
 *
 * \return Error code if return value < 0
 *          - #AV_ER_INVALID_ARG The AV channel ID is not valid
 *          - #AV_ER_NOT_SUPPORT New re-send protocal not support this function
 *
 */
AVAPI_API int32_t avSetAudioLostCondition(int32_t nAVChannelID, int32_t lostCondition);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AVCOMMON_H_ */
