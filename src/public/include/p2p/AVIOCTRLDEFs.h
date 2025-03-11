/*
 * AVIOCTRLDEFs.h
 *	Define AVIOCTRL Message Type and Context
 *  Created on: 2011-08-12
 *  Author: TUTK
 *
 */

//Change Log:
//  2013-12-25 - 1> Add set and get dropbox feature of device.
//                      Add IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_REQ
//                          Client request device to return dropbox status
//                      Add IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_RESP
//                          Device return to client the dropbox status
//                      Add IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_REQ
//                          Client request device to set dropbox info
//                      Add IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_RESP
//                          Device acknowledge the set dropbox info
//
//  2013-06-25 - 1> Add set and get time zone of device.
//                      Add IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ
//                          Client request device to return time zone
//	                    Add IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP
//	                        Device return to client the time zone
//	                    Add IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ
//	                        Client request device to set time zone
//	                    Add IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP 
//	                        Device acknowledge the set time zone request
//
//  2013-06-05 - 1> Add customer defined message type, start from #FF000000
//                  Naming role of message type : IOTYPE_[Company_name]_[function_name]
//                      ex : IOTYPE_TUTK_TEST_REQ, IOTYPE_TUTK_TEST_RESP
//                  Naming role of data structure : [Company_name]_[data_structure_name]
//                      ex : TUTK_SMsgTestReq, TUTK_SMsgTestResp
//                  
//
//  2013-03-10 - 1> Add flow information collection mechanism.
//						Add IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ
//							Device request client to collect flow information.
//						Add IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP
//							Client acknowledge device that request is received.
//						Add IOTYPE_USER_IPCAM_CURRENT_FLOWINFO
//							Client send collected flow information to device.
//				 2> Add IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME command.
//
//	2013-02-19 - 1> Add more detail of status of SWifiAp
//				 2> Add more detail description of STimeDay
//
//	2012-10-26 - 1> SMsgAVIoctrlGetEventConfig
//						Add field: externIoOutIndex, externIoInIndex
//				 2> SMsgAVIoctrlSetEventConfig, SMsgAVIoctrlGetEventCfgResp
//						Add field: externIoOutStatus, externIoInStatus
//
//	2012-10-19 - 1> SMsgAVIoctrlGetWifiResp: -->SMsgAVIoctrlGetWifiResp2
//						Add status description
//				 2> SWifiAp:
//				 		Add status 4: selected but not connected
//				 3> WI-FI Password 32bit Change to 64bit
//				 4> ENUM_AP_ENCTYPE: Add following encryption types
//				 		AVIOTC_WIFIAPENC_WPA_PSK_TKIP		= 0x07,
//						AVIOTC_WIFIAPENC_WPA_PSK_AES		= 0x08,
//						AVIOTC_WIFIAPENC_WPA2_PSK_TKIP		= 0x09,
//						AVIOTC_WIFIAPENC_WPA2_PSK_AES		= 0x0A,
//
//				 5> IOTYPE_USER_IPCAM_SETWIFI_REQ_2:
//						Add struct SMsgAVIoctrlSetWifiReq2
//				 6> IOTYPE_USER_IPCAM_GETWIFI_RESP_2:
//						Add struct SMsgAVIoctrlGetWifiResp2

//  2012-07-18 - added: IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ, IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP
//	2012-05-29 - IOTYPE_USER_IPCAM_DEVINFO_RESP: Modify firmware version
//	2012-05-24 - SAvEvent: Add result type
//  2014-07-07 - Change Dropbox struct SMsgAVIoctrlSetDropbox from 32 bytes to 128byes
//

#ifndef _AVIOCTRL_DEFINE_H_
#define _AVIOCTRL_DEFINE_H_

/////////////////////////////////////////////////////////////////////////////////
/////////////////// Message Type Define//////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// AVIOCTRL Message Type
typedef enum 
{
	IOTYPE_USER_IPCAM_START 					= 0x01FF,
	IOTYPE_USER_IPCAM_STOP	 					= 0x02FF,
    
    IOTYPE_USER_IPCAM_START_CLIENT              = 0x03FF,
    IOTYPE_USER_IPCAM_STOP_CLIENT               = 0x04FF,
    
	IOTYPE_USER_IPCAM_AUDIOSTART 				= 0x0300,
	IOTYPE_USER_IPCAM_AUDIOSTOP 				= 0x0301,
    
	IOTYPE_USER_IPCAM_SPEAKERSTART 				= 0x0350,
	IOTYPE_USER_IPCAM_SPEAKERSTOP 				= 0x0351,

	IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ			= 0x0320,
	IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP		= 0x0321,
	IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ			= 0x0322,
	IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP		= 0x0323,

	IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ		= 0x0324,
	IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP		= 0x0325,
	IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ		= 0x0326,
	IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP		= 0x0327,
	
	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ		= 0x0328,	// Get Support Stream
	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP		= 0x0329,
    
	IOTYPE_USER_IPCAM_DEVINFO_REQ				= 0x0330,
	IOTYPE_USER_IPCAM_DEVINFO_RESP				= 0x0331,

	IOTYPE_USER_IPCAM_SETPASSWORD_REQ			= 0x0332,
	IOTYPE_USER_IPCAM_SETPASSWORD_RESP			= 0x0333,

    IOTYPE_USER_IPCAM_GET_PLAYBACK_REQ          = 0x033A,
    IOTYPE_USER_IPCAM_GET_PLAYBACK_RESP         = 0x033B,
    IOTYPE_USER_IPCAM_SET_RECORD_PROGRESS_REQ   = 0x033C,
    IOTYPE_USER_IPCAM_SET_RECORD_PROGRESS_RESP  = 0x033D,
    
	IOTYPE_USER_IPCAM_LISTWIFIAP_REQ			= 0x0340,
	IOTYPE_USER_IPCAM_LISTWIFIAP_RESP			= 0x0341,
	IOTYPE_USER_IPCAM_SETWIFI_REQ				= 0x0342,
	IOTYPE_USER_IPCAM_SETWIFI_RESP				= 0x0343,
	IOTYPE_USER_IPCAM_GETWIFI_REQ				= 0x0344,
	IOTYPE_USER_IPCAM_GETWIFI_RESP				= 0x0345,
	IOTYPE_USER_IPCAM_SETWIFI_REQ_2				= 0x0346,
	IOTYPE_USER_IPCAM_GETWIFI_RESP_2			= 0x0347,
    
    IOTYPE_USER_IPCAM_SETRECORD_REQ             = 0x0310,
    IOTYPE_USER_IPCAM_SETRECORD_RESP            = 0x0311,
    IOTYPE_USER_IPCAM_GETRECORD_REQ             = 0x0312,
    IOTYPE_USER_IPCAM_GETRECORD_RESP            = 0x0313,
    
    IOTYPE_USER_IPCAM_SETRCD_DURATION_REQ       = 0x0314,
    IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP      = 0x0315,
    IOTYPE_USER_IPCAM_GETRCD_DURATION_REQ       = 0x0316,
    IOTYPE_USER_IPCAM_GETRCD_DURATION_RESP      = 0x0317,
    
    IOTYPE_USER_IPCAM_LISTEVENT_REQ             = 0x0318,
    IOTYPE_USER_IPCAM_LISTEVENT_RESP            = 0x0319,
    
    IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL        = 0x031A,
    IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP   = 0x031B,
    
    IOTYPE_USER_IPCAM_LISTFILE_REQ              = 0x031C,
    IOTYPE_USER_IPCAM_LISTFILE_RESP             = 0x031D, // refer to ENUM_EVENTTYPE
    
    IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ        = 0x032A,
    IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP       = 0x032B,
    
    IOTYPE_USER_IPCAM_CHECKSUPPORTMESSAGETYPE_REQ  = 0x032C,
    IOTYPE_USER_IPCAM_CHECKSUPPORTMESSAGETYPE_RESP = 0x032D,

	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_REQ		= 0x0400,	// Get Event Config Msg Request
	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_RESP		= 0x0401,	// Get Event Config Msg Response
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_REQ		= 0x0402,	// Set Event Config Msg req
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_RESP		= 0x0403,	// Set Event Config Msg resp

	IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ		= 0x0360,
	IOTYPE_USER_IPCAM_SET_ENVIRONMENT_RESP		= 0x0361,
	IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ		= 0x0362,
	IOTYPE_USER_IPCAM_GET_ENVIRONMENT_RESP		= 0x0363,
	
	IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ			= 0x0370,	// Set Video Flip Mode
	IOTYPE_USER_IPCAM_SET_VIDEOMODE_RESP		= 0x0371,
	IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ			= 0x0372,	// Get Video Flip Mode
	IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP		= 0x0373,
	
	IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ		= 0x0380,	// Format external storage
	IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_RESP		= 0x0381,	
	
    IOTYPE_USER_IPCAM_LIVEVIEW_OPTIONS_REQ      = 0x3C0,
    IOTYPE_USER_IPCAM_LIVEVIEW_OPTIONS_RESP     = 0x3C1,
    
	IOTYPE_USER_IPCAM_PTZ_COMMAND				= 0x1001,	// P2P PTZ Command Msg

	IOTYPE_USER_IPCAM_EVENT_REPORT				= 0x1FFF,	// Device Event Report Msg
	IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME		= 0x1002,	// Send from client, used to talk to device that
															// client had received the first I frame
	
	IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ			= 0x0390,
	IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP			= 0x0391,
	IOTYPE_USER_IPCAM_CURRENT_FLOWINFO			= 0x0392,
	
	IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ          = 0x3A0,
	IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP         = 0x3A1,
	IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ          = 0x3B0,
	IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP         = 0x3B1,
    
    IOTYPE_USER_IPCAM_GET_BATTERY_LEVEL_REQ     = 0x03C2,   
    IOTYPE_USER_IPCAM_GET_BATTERY_LEVEL_RESP    = 0x03C3,   
    
    IOTYPE_USER_IPCAM_GET_BATTERY_REQ           = 0x60A,    
    IOTYPE_USER_IPCAM_GET_BATTERY_RESP          = 0x60B,    
    IOTYPE_USER_IPCAM_GET_BATTERY_REQ_Other     = 0x806,    
    IOTYPE_USER_IPCAM_GET_BATTERY_RESP_Other    = 0x807,    
    
    IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ_EXT      = 0x471,
    IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP_EXT     = 0x472,
    IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP_EXT     = 0x474,
    IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ_EXT      = 0x473,
    
    IOTYPE_XM_CALL_REQ                          = 0x700,
    IOTYPE_XM_CALL_RESP                         = 0x701,
    IOTYPE_XM_CALL_IND                          = 0x702,
    IOTYPE_XM_GETDOORLIST_REQ                   = 0x710,
    IOTYPE_XM_GETDOORLIST_RESP                  = 0x711,
    IOTYPE_XM_SETDOORINFO_REQ                   = 0x712,
    IOTYPE_XM_SETDOORINFO_RESP                  = 0x713,
    IOTYPE_XM_GETUNLOCKPASSWDSWITCH_REQ         = 0x714,
    IOTYPE_XM_GETUNLOCKPASSWDSWITCH_RESP        = 0x715,
    IOTYPE_XM_SETUNLOCKPASSWDSWITCH_REQ         = 0x716,
    IOTYPE_XM_SETUNLOCKPASSWDSWITCH_RESP        = 0x717,
    IOTYPE_XM_SETUNLOCKPASSWD_REQ               = 0x718,
    IOTYPE_XM_SETUNLOCKPASSWD_RESP              = 0x719,
    IOTYPE_XM_UNLOCKPASSWD_IND                  = 0x71A,
    IOTYPE_XM_UNLOCK_REQ                        = 0x71B,
    IOTYPE_XM_UNLOCK_RESP                       = 0x71C,
    
    IOTYPE_USER_IPCAM_DEVICESLEEP_REQ           = 0x720,
    IOTYPE_USER_IPCAM_DEVICESLEEP_RESP          = 0x721,
    IOTYPE_USER_IPCAM_GET_WiFiSIGNAL_REQ        = 0X0730,
    IOTYPE_USER_IPCAM_PUSH_DEVICENAME_REQ       = 0X0736,
    IOTYPE_USER_IPCAM_GET_WiFiSIGNAL_RESP       = 0X0731,
    IOTYPE_USER_IPCAM_PUSH_DEVICENAME_RESP      = 0X0737,
    IOTYPE_USER_IPCAM_SET_WAKEUPSWITCH_REQ      = 0X0740,
    IOTYPE_USER_IPCAM_SET_WAKEUPSWITCH_RESP     = 0X0741,
    IOTYPE_USER_IPCAM_GET_WAKEUPSWITCH_REQ      = 0X0742,
    IOTYPE_USER_IPCAM_GET_WAKEUPSWITCH_RESP     = 0X0743,
    
    IOTYPE_USER_IPCAM_SETREBOOT_REQ             = 0x8004,
    IOTYPE_USER_IPCAM_SETREBOOT_RESP            = 0x8005,
    
    //IOTYPE_USER_IPCAM_SET_VSAASINFO_REQ       = 0x0410,   //deprecated, pls use IOTYPE_USER_IPCAM_SET_CLOUD_RECORDING_ENDPOINT_REQ
    //IOTYPE_USER_IPCAM_SET_VSAASINFO_RESP      = 0x0411,   //deprecated, pls use IOTYPE_USER_IPCAM_SET_CLOUD_RECORDING_ENDPOINT_RESP
    IOTYPE_USER_IPCAM_SET_CLOUD_RECORDING_ENDPOINT_REQ         = 0x0412,
    IOTYPE_USER_IPCAM_SET_CLOUD_RECORDING_ENDPOINT_RESP        = 0x0413,

    // dropbox support
    IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_REQ      = 0x500,
    IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_RESP     = 0x501,
    IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_REQ      = 0x502,
    IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_RESP     = 0x503,

    // get local ip
    IOTYPE_USER_IPCAM_GET_LOCAL_IP_REQ          = 0x510,
    IOTYPE_USER_IPCAM_GET_LOCAL_IP_RESP         = 0x511,
    
    //easy set wifi
    IOTYPE_USER_IPCAM_SET_EZWIFI_REQ            = 0x512,
    IOTYPE_USER_IPCAM_SET_EZWIFI_RESP           = 0x513,
    
    // ptz preset
	IOTYPE_HICHIP_CRUISE_START                  = 0x600,
	IOTYPE_HICHIP_CRUISE_STOP                   = 0x601,
    
    // adjust brightness
    IOTYPE_HICHIP_GETBRIGHT_REQ                 =0x602,
    IOTYPE_HICHIP_GETBRIGHT_RESP                =0x603,
    IOTYPE_HICHIP_SETBRIGHT_REQ                 =0x604,
    IOTYPE_HICHIP_SETBRIGHT_RESP                =0x605,
	
    // adjust contrast
    IOTYPE_HICHIP_GETCONTRAST_REQ               =0x606,
    IOTYPE_HICHIP_GETCONTRAST_RESP              =0x607,
    IOTYPE_HICHIP_SETCONTRAST_REQ               =0x608,
    IOTYPE_HICHIP_SETCONTRAST_RESP              =0x609,

	IOTYPE_USER_IPCAM_SETPRESET_REQ             = 0x440,
	IOTYPE_USER_IPCAM_SETPRESET_RESP            = 0x441,
	IOTYPE_USER_IPCAM_GETPRESET_REQ             = 0x442,
	IOTYPE_USER_IPCAM_GETPRESET_RESP            = 0x443,
    

    IOTYPE_USER_IPCAM_GETGUARD_REQ                = 0x420,
    IOTYPE_USER_IPCAM_GETGUARD_RESP                = 0x421,
    IOTYPE_USER_IPCAM_SETGUARD_REQ                = 0x422,
    IOTYPE_USER_IPCAM_SETGUARD_RESP                = 0x423,
	
    //FTP config
    IOTYPE_USER_IPCAM_SET_FTP_REQ                = 0x055A,
    IOTYPE_USER_IPCAM_SET_FTP_RESP                = 0x055B,
    IOTYPE_USER_IPCAM_GET_FTP_REQ                = 0x055C,
    IOTYPE_USER_IPCAM_GET_FTP_RESP                = 0x055D,
	
    //mail config
    IOTYPE_USEREX_IPCAM_GET_SMTP_REQ            =0x4005,
    IOTYPE_USEREX_IPCAM_GET_SMTP_RESP           =0x4006,
    IOTYPE_USEREX_IPCAM_SET_SMTP_REQ            =0x4007,
    IOTYPE_USEREX_IPCAM_SET_SMTP_RESP           =0x4008,
    
    //========================== Ambarella IO Command =======================
    
    IOTYPE_USER_SETAMBCROPSET_REQ               = 0xFF001004,
    IOTYPE_USER_SETAMBCROPSET_RESP              = 0xFF001005,
    
    IOTYPE_USER_GETAMBCROPSTATUS_REQ            = 0xFF001006,
    IOTYPE_USER_GETAMBCROPSTATUS_RESP           = 0xFF001007,
    
    IOTYPE_USER_SETAMBCROP_GETAMBCROPIMAGECHANGE_RESP    = 0xFF001008,
    IOTYPE_USER_SETAMBLDCSET_REQ                         = 0xFF001009,
    IOTYPE_USER_SETAMBLDCSET_RESP                        = 0xFF00100A,
    IOTYPE_USER_GETAMBLDCSTATUS_REQ                      = 0xFF00100B,
    IOTYPE_USER_GETAMBLDCSTATUS_RESP                     = 0xFF00100C,
    
    //Novatek WiFi command
    IOTYPE_USER_IPCAM_NOVATEK_WIFICMD_REQ    = 0x7F000000,
    IOTYPE_USER_IPCAM_NOVATEK_WIFICMD_RESP   = 0x7F000001,
    
    IOTYPE_USER_SETCLIENTRECVREADY_REQ       = 0x7F000002,
    IOTYPE_USER_SETCLIENTRECVREADY_RESP      = 0x7F000003,
    
    IOTYPE_USER_IPCAM_DOWNLOAD_FILE_REQ      = 0x7F000004,
    IOTYPE_USER_IPCAM_DOWNLOAD_FILE_RESP     = 0x7F000005,
    
    IOTYPE_USER_IPCAM_UPLOAD_FILE_REQ        = 0x7F000006,
    
    // AirLive
//    IOTYPE_USER_GETIPAddress_REQ             = 0x701,
//    IOTYPE_USER_GETIPAddress_RESP            = 0x702,
    
    
    //========================== Kalay VSaaS Server IOCtrl =======================
    
    IOTYPE_USER_IPCAM_SET_VSAAS_PLAYBACK_INFO_REQ   = 0x00010000,
    IOTYPE_USER_IPCAM_SET_VSAAS_PLAYBACK_INFO_RESP  = 0x00010001,
    IOTYPE_USER_IPCAM_VSAAS_RECORD_PLAYCONTROL_REQ  = 0x00010002,
    IOTYPE_USER_IPCAM_VSAAS_RECORD_PLAYCONTROL_RESP = 0x00010003,
    
    //========================= WWM IOCtrl =======================================
    
    //NVR Time zone
    IOTYPE_USER_IPCAM_GET_NVR_TIMEZONE_REQ      = 0x5A0,
    IOTYPE_USER_IPCAM_GET_NVR_TIMEZONE_RESP     = 0x5A1,
    IOTYPE_USER_IPCAM_SET_NVR_TIMEZONE_REQ      = 0x5A2,
    IOTYPE_USER_IPCAM_SET_NVR_TIMEZONE_RESP     = 0x5A3,
    
    
    //NVR NUM
    IOTYPE_USER_IPCAM_GET_NVR_CHANNEL_NUMBER_REQ  = 0x5A4,
    IOTYPE_USER_IPCAM_GET_NVR_CHANNEL_NUMBER_RESP = 0x5A5,
    
    IOTYPE_USER_IPCAM_GET_RECORD_DURATION_REQ   = 0x5A6,
    IOTYPE_USER_IPCAM_GET_RECORD_DURATION_RESP  = 0x5A7,
    IOTYPE_USER_IPCAM_SET_RECORD_DURATION_REQ   = 0x5A8,
    IOTYPE_USER_IPCAM_SET_RECORD_DURATION_RESP  = 0x5A9,
    
    IOTYPE_USER_IPCAM_GET_CHANNEL_NAME_REQ      = 0x5B0,
    IOTYPE_USER_IPCAM_GET_CHANNEL_NAME_RESP     = 0x5B1,
    IOTYPE_USER_IPCAM_SET_CHANNEL_NAME_REQ      = 0x5B2,
    IOTYPE_USER_IPCAM_SET_CHANNEL_NAME_RESP     = 0x5B3,
    
    IOTYPE_USER_IPCAM_GET_VIDEO_STREAM_REQ      = 0x5B4,
    IOTYPE_USER_IPCAM_GET_VIDEO_STREAM_RESP     = 0x5B5,
    IOTYPE_USER_IPCAM_SET_VIDEO_STREAM_REQ      = 0x5B6,
    IOTYPE_USER_IPCAM_SET_VIDEO_STREAM_RESP     = 0x5B7,
    
    IOTYPE_USER_IPCAM_GET_EVENT_IMAGE_REQ       = 0x5B8,
    IOTYPE_USER_IPCAM_GET_EVENT_IMAGE_RESP      = 0x5B9,
    
    IOTYPE_USER_IPCAM_OTA_REQ                   = 0x8001,
    IOTYPE_USER_IPCAM_OTA_RESP                  = 0x8002,
    IOTYPE_USER_IPCAM_DEVICE_INFO_REQ           = 0x8015,
    IOTYPE_USER_IPCAM_DEVICE_INFO_RESP          = 0x8016,
    IOTYPE_USER_IPCAM_DEVICE_SUPPORT_OTA_REQ    = 0x800A,
    IOTYPE_USER_IPCAM_DEVICE_SUPPORT_OTA_RESP   = 0x800B,
    
    IOTYPE_USER_IPCAM_DEVICE_SUPPORT_CLOUD_REQ  = 0x800C,
    IOTYPE_USER_IPCAM_DEVICE_SUPPORT_CLOUD_RESP = 0x800D,
    IOTYPE_USER_IPCAM_UPLOAD_CLOUD_INFO_REQ     = 0x800E,
    IOTYPE_USER_IPCAM_UPLOAD_CLOUD_INFO_RESP    = 0x800F,
    
    IOTYPE_USER_IPCAM_DEVICE_SET_CLOUD_REQ      = 0x8010,
    IOTYPE_USER_IPCAM_DEVICE_SET_CLOUD_RESP     = 0x8011,
    IOTYPE_USER_IPCAM_DEVICE_GET_CLOUD_REQ      = 0x8012,
    IOTYPE_USER_IPCAM_DEVICE_GET_CLOUD_RESP     = 0x8013,
    
    IOTYPE_USER_IPCAM_GET_EVENT_DATE_REQ        = 0x9000,
    IOTYPE_USER_IPCAM_GET_EVENT_DATE_RESP       = 0x9001,
    
    IOTYPE_USER_IPCAM_GET_HUMANDETECTION_REQ    = 0x9002,
    IOTYPE_USER_IPCAM_GET_HUMANDETECTION_RESP   = 0x9003,
    
    IOTYPE_USER_IPCAM_SET_HUMANDETECTION_REQ    = 0x9004,
    IOTYPE_USER_IPCAM_SET_HUMANDETECTION_RESP   = 0x9005,
    
    IOTYPE_USER_IPCAM_GET_NIGHTVISION_REQ       = 0x9006,
    IOTYPE_USER_IPCAM_GET_NIGHTVISION_RESP      = 0x9007,
    
    IOTYPE_USER_IPCAM_SET_NIGHTVISION_REQ       = 0x9008,
    IOTYPE_USER_IPCAM_SET_NIGHTVISION_RESP      = 0x9009,
    
    IOTYPE_USER_IPCAM_GET_SUMMERTIME_REQ        = 0x9010,
    IOTYPE_USER_IPCAM_GET_SUMMERTIME_RESP       = 0x9011,
    
    IOTYPE_USER_IPCAM_SET_SUMMERTIME_REQ        = 0x9012,
    IOTYPE_USER_IPCAM_SET_SUMMERTIME_RESP       = 0x9013,

	IOTYPE_USER_IPCAM_DOOROPEN_REQ              = 0x800,
	IOTYPE_USER_IPCAM_DOOROPEN_RESP             = 0x801,
    
	// for IP02/IPC product
	IOTYPE_USER_IPCAM_SET_PUSH_REQ              = 0x802,
	IOTYPE_USER_IPCAM_SET_PUSH_RESP             = 0x803,

	IOTYPE_USER_IPCAM_DEL_PUSH_REQ              = 0x804,
	IOTYPE_USER_IPCAM_DEL_PUSH_RESP             = 0x805,

	IOTYPE_USER_IPCAM_SET_MDP_REQ				= 0x806,
	IOTYPE_USER_IPCAM_SET_MDP_RESP				= 0x807,
	IOTYPE_USER_IPCAM_GET_MDP_REQ				= 0x808,
	IOTYPE_USER_IPCAM_GET_MDP_RESP				= 0x809,

	IOTYPE_USER_IPCAM_DOORPASS_REQ              = 0x812,
	IOTYPE_USER_IPCAM_DOORPASS_RESP             = 0x813,

	IOTYPE_USER_IPCAM_SET_DEVICETIME_REQ        = 0x816,
	IOTYPE_USER_IPCAM_SET_DEVICETIME_RESP       = 0x817,

	IOTYPE_USER_IPCAM_ALARMING_REQ				= 0x850,
	IOTYPE_USER_IPCAM_ALARMING_RESP				= 0x851,

    IOTYPE_USER_IPCAM_SET_SYSTEM_REQ			= 0x852,
    IOTYPE_USER_IPCAM_SET_SYSTEM_RESP			= 0x853,

    IOTYPE_USER_IPCAM_GET_SYSTEM_REQ			= 0x854,
	IOTYPE_USER_IPCAM_GET_SYSTEM_RESP			= 0x855,

    IOTYPE_USER_IPCAM_GET_SDCARD_REQ            = 0x856,
    IOTYPE_USER_IPCAM_GET_SDCARD_RESP           = 0x857,

    IOTYPE_USER_IPCAM_GET_OSD_REQ               = 0x858,
    IOTYPE_USER_IPCAM_GET_OSD_RESP              = 0x859,
    IOTYPE_USER_IPCAM_SET_OSD_REQ               = 0x860,
    IOTYPE_USER_IPCAM_SET_OSD_RESP              = 0x861,
	IOTYPE_USER_IPCAM_SET_433_REQ				= 0x862,
	IOTYPE_USER_IPCAM_SET_433_RESP				= 0x863,
	IOTYPE_USER_IPCAM_GET_433_REQ				= 0x864,
	IOTYPE_USER_IPCAM_GET_433_RESP				= 0x865,
	IOTYPE_USER_IPCAM_CFG_433_REQ				= 0x866,
	IOTYPE_USER_IPCAM_CFG_433_RESP				= 0x867,
	IOTYPE_USER_IPCAM_DEL_433_REQ				= 0x868,
	IOTYPE_USER_IPCAM_DEL_433_RESP				= 0x869,
	IOTYPE_USER_IPCAM_CFG_433_EXIT_REQ			= 0x870,
	IOTYPE_USER_IPCAM_CFG_433_EXIT_RESP			= 0x871,

    IOTYPE_USER_IPCAM_GET_CAPACITY_REQ          = 0x880,
    IOTYPE_USER_IPCAM_GET_CAPACITY_RESP         = 0x881,

    IOTYPE_USER_IPCAM_GET_MD_ALAM_REQ           = 0x882,
    IOTYPE_USER_IPCAM_GET_MD_ALAM_RESP          = 0x883,
    IOTYPE_USER_IPCAM_SET_MD_ALAM_REQ           = 0x884,
    IOTYPE_USER_IPCAM_SET_MD_ALAM_RESP          = 0x885,

    IOTYPE_USER_IPCAM_GET_AUDIO_REQ             = 0x886,
    IOTYPE_USER_IPCAM_GET_AUDIO_RESP            = 0x887,
    IOTYPE_USER_IPCAM_SET_AUDIO_REQ             = 0x888,
    IOTYPE_USER_IPCAM_SET_AUDIO_RESP            = 0x889,

    IOTYPE_USER_IPCAM_UPDATE_REQ                = 0x88a,
    IOTYPE_USER_IPCAM_UPDATE_RESP               = 0x88b,
    IOTYPE_USER_IPCAM_UPDATE_PROG_REQ           = 0x88c,
    IOTYPE_USER_IPCAM_UPDATE_PROG_RESP          = 0x88d,
  
    IOTYPE_USER_IPCAM_SHELL_REQ                 = 0x88e,    
    IOTYPE_USER_IPCAM_SHELL_RESP                = 0x88f,
    IOTYPE_USER_IPCAM_HEARTBEAT_REQ             = 0x890,
    IOTYPE_USER_IPCAM_HEARTBEAT_RESP            = 0x891,

	IOTYPE_USER_IPCAM_SEND_META_START           = 0x200,
    IOTYPE_USER_IPCAM_SEND_META_STOP            = 0x201,
	
    IOTYPE_USER_IPCAM_PUSH_STREAMING_START      = 0x210,
	IOTYPE_USER_IPCAM_PUSH_STREAMING_START_RESP = 0x211,
    IOTYPE_USER_IPCAM_PUSH_STREAMING_FINISH     = 0x212,
	IOTYPE_USER_IPCAM_PUSH_STREAMING_FINISH_RESP= 0x213,
	IOTYPE_USER_IPCAM_PUSH_STREAMING_CLOSE      = 0x214,
    
    IOTYPE_USER_IPCAM_START_CHANNEL             = 0x05ff,
    IOTYPE_USER_IPCAM_STOP_CHANNEL              = 0x06ff,

    IOTYPE_USER_CMD_MAX
}ENUM_AVIOCTRL_MSGTYPE;



/////////////////////////////////////////////////////////////////////////////////
/////////////////// Type ENUM Define ////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
typedef enum
{
	AVIOCTRL_OK					= 0x00,
	AVIOCTRL_ERR				= -0x01,
	AVIOCTRL_ERR_PASSWORD		= AVIOCTRL_ERR - 0x01,
	AVIOCTRL_ERR_STREAMCTRL		= AVIOCTRL_ERR - 0x02,
	AVIOCTRL_ERR_MONTIONDETECT	= AVIOCTRL_ERR - 0x03,
	AVIOCTRL_ERR_DEVICEINFO		= AVIOCTRL_ERR - 0x04,
	AVIOCTRL_ERR_LOGIN			= AVIOCTRL_ERR - 5,
	AVIOCTRL_ERR_LISTWIFIAP		= AVIOCTRL_ERR - 6,
	AVIOCTRL_ERR_SETWIFI		= AVIOCTRL_ERR - 7,
	AVIOCTRL_ERR_GETWIFI		= AVIOCTRL_ERR - 8,
	AVIOCTRL_ERR_SETRECORD		= AVIOCTRL_ERR - 9,
	AVIOCTRL_ERR_SETRCDDURA		= AVIOCTRL_ERR - 10,
	AVIOCTRL_ERR_LISTEVENT		= AVIOCTRL_ERR - 11,
	AVIOCTRL_ERR_PLAYBACK		= AVIOCTRL_ERR - 12,

	AVIOCTRL_ERR_INVALIDCHANNEL	= AVIOCTRL_ERR - 0x20,
}ENUM_AVIOCTRL_ERROR; //APP don't use it now


// ServType, unsigned long, 32 bits, is a bit mask for function declareation
// bit value "0" means function is valid or enabled
// in contract, bit value "1" means function is invalid or disabled.
// ** for more details, see "ServiceType Definitation for AVAPIs"
// 
// Defined bits are listed below:
//----------------------------------------------
// bit		fuction
// 0		Audio in, from Device to Mobile
// 1		Audio out, from Mobile to Device 
// 2		PT function
// 3		Event List function
// 4		Play back function (require Event List function)
// 5		Wi-Fi setting function
// 6		Event Setting Function
// 7		Recording Setting function
// 8		SDCard formattable function
// 9		Video flip function
// 10		Environment mode
// 11		Multi-stream selectable
// 12		Audio out encoding format

// The original enum below is obsoleted.
typedef enum
{
	SERVTYPE_IPCAM_DWH					= 0x00,
	SERVTYPE_RAS_DWF					= 0x01,
	SERVTYPE_IOTCAM_8125				= 0x10,
	SERVTYPE_IOTCAM_8125PT				= 0x11,
	SERVTYPE_IOTCAM_8126				= 0x12,
	SERVTYPE_IOTCAM_8126PT				= 0x13,	
}ENUM_SERVICE_TYPE;

// AVIOCTRL Quality Type
typedef enum 
{
	AVIOCTRL_QUALITY_UNKNOWN			= 0x00,	
	AVIOCTRL_QUALITY_MAX				= 0x01,	// ex. 640*480, 15fps, 320kbps (or 1280x720, 5fps, 320kbps)
	AVIOCTRL_QUALITY_HIGH				= 0x02,	// ex. 640*480, 10fps, 256kbps
	AVIOCTRL_QUALITY_MIDDLE				= 0x03,	// ex. 320*240, 15fps, 256kbps
	AVIOCTRL_QUALITY_LOW				= 0x04, // ex. 320*240, 10fps, 128kbps
	AVIOCTRL_QUALITY_MIN				= 0x05,	// ex. 160*120, 10fps, 64kbps
}ENUM_QUALITY_LEVEL;


typedef enum
{
	AVIOTC_WIFIAPMODE_NULL				= 0x00,
	AVIOTC_WIFIAPMODE_MANAGED			= 0x01,
	AVIOTC_WIFIAPMODE_ADHOC				= 0x02,
}ENUM_AP_MODE;


typedef enum
{
	AVIOTC_WIFIAPENC_INVALID			= 0x00, 
	AVIOTC_WIFIAPENC_NONE				= 0x01, //
	AVIOTC_WIFIAPENC_WEP				= 0x02, //WEP, for no password
	AVIOTC_WIFIAPENC_WPA_TKIP			= 0x03, 
	AVIOTC_WIFIAPENC_WPA_AES			= 0x04, 
	AVIOTC_WIFIAPENC_WPA2_TKIP			= 0x05, 
	AVIOTC_WIFIAPENC_WPA2_AES			= 0x06,

	AVIOTC_WIFIAPENC_WPA_PSK_TKIP  = 0x07,
	AVIOTC_WIFIAPENC_WPA_PSK_AES   = 0x08,
	AVIOTC_WIFIAPENC_WPA2_PSK_TKIP = 0x09,
	AVIOTC_WIFIAPENC_WPA2_PSK_AES  = 0x0A,

}ENUM_AP_ENCTYPE;


// AVIOCTRL Event Type
typedef enum
{
    AVIOCTRL_EVENT_ALL                   = 0x00,          // all event type
    AVIOCTRL_EVENT_MOTIONDECT            = 0x01,          // motion detect start
    AVIOCTRL_EVENT_VIDEOLOST             = 0x02,             // video lost alarm
    AVIOCTRL_EVENT_IOALARM               = 0x03,                   // IO alarm start
    AVIOCTRL_EVENT_MOTIONPASS            = 0x04,             // motion detect end
    AVIOCTRL_EVENT_VIDEORESUME           = 0x05,         // video resume
    AVIOCTRL_EVENT_IOALARMPASS           = 0x06,         // IO alarm end
    AVIOCTRL_EVENT_MOVIE                 = 0x07,
    AVIOCTRL_EVENT_TIME_LAPSE            = 0x08,
    AVIOCTRL_EVENT_EMERGENCY             = 0x09,
    AVIOCTRL_EVENT_EXPT_REBOOT           = 0x10,                   // system exception reboot
    AVIOCTRL_EVENT_SDFAULT               = 0x11,                             // SDCard record exception
    AVIOCTRL_EVENT_FULLTIME_RECORDING    = 0x12,    // Fulltime event
    AVIOCTRL_EVENT_PIR                   = 0x13,                                          // PIR event
    AVIOCTRL_EVENT_RINGBELL              = 0x14,                             // user call
    AVIOCTRL_EVENT_SOUND                 = 0x15,
    AVIOCTRL_EVENT_HUMANOID_DETECTION    = 0x16  // humanoid detection
  }ENUM_EVENTTYPE;

// AVIOCTRL Record Type
typedef enum
{
	AVIOTC_RECORDTYPE_OFF				= 0x00,
	AVIOTC_RECORDTYPE_FULLTIME			= 0x01,
	AVIOTC_RECORDTYPE_ALARM				= 0x02,
	AVIOTC_RECORDTYPE_MANUAL			= 0x03,
}ENUM_RECORD_TYPE;

// AVIOCTRL Play Record Command
typedef enum 
{
	AVIOCTRL_RECORD_PLAY_PAUSE			= 0x00,
	AVIOCTRL_RECORD_PLAY_STOP			= 0x01,
	AVIOCTRL_RECORD_PLAY_STEPFORWARD	= 0x02, //now, APP no use
	AVIOCTRL_RECORD_PLAY_STEPBACKWARD	= 0x03, //now, APP no use
	AVIOCTRL_RECORD_PLAY_FORWARD		= 0x04, //now, APP no use
	AVIOCTRL_RECORD_PLAY_BACKWARD		= 0x05, //now, APP no use
	AVIOCTRL_RECORD_PLAY_SEEKTIME		= 0x06, //now, APP no use
	AVIOCTRL_RECORD_PLAY_END			= 0x07,
	AVIOCTRL_RECORD_PLAY_START			= 0x10,
    AVIOCTRL_RECORD_PLAY_NEXT           = 0xf0,
    AVIOCTRL_RECORD_PLAY_IFRAME         = 0xf1,
}ENUM_PLAYCONTROL;

// AVIOCTRL Environment Mode
typedef enum
{
	AVIOCTRL_ENVIRONMENT_INDOOR_50HZ 	= 0x00,
	AVIOCTRL_ENVIRONMENT_INDOOR_60HZ	= 0x01,
	AVIOCTRL_ENVIRONMENT_OUTDOOR		= 0x02,
	AVIOCTRL_ENVIRONMENT_NIGHT			= 0x03,	
}ENUM_ENVIRONMENT_MODE;

// AVIOCTRL Video Flip Mode
typedef enum
{
	AVIOCTRL_VIDEOMODE_NORMAL 			= 0x00,
	AVIOCTRL_VIDEOMODE_FLIP				= 0x01,
	AVIOCTRL_VIDEOMODE_MIRROR			= 0x02,
	AVIOCTRL_VIDEOMODE_FLIP_MIRROR 		= 0x03,
}ENUM_VIDEO_MODE;

// AVIOCTRL PTZ Command Value
typedef enum 
{
	AVIOCTRL_PTZ_STOP					= 0,
	AVIOCTRL_PTZ_UP						= 1,
	AVIOCTRL_PTZ_DOWN					= 2,
	AVIOCTRL_PTZ_LEFT					= 3,
	AVIOCTRL_PTZ_LEFT_UP				= 4,
	AVIOCTRL_PTZ_LEFT_DOWN				= 5,
	AVIOCTRL_PTZ_RIGHT					= 6, 
	AVIOCTRL_PTZ_RIGHT_UP				= 7, 
	AVIOCTRL_PTZ_RIGHT_DOWN				= 8, 
	AVIOCTRL_PTZ_AUTO					= 9, 
	AVIOCTRL_PTZ_SET_POINT				= 10,
	AVIOCTRL_PTZ_CLEAR_POINT			= 11,
	AVIOCTRL_PTZ_GOTO_POINT				= 12,

	AVIOCTRL_PTZ_SET_MODE_START			= 13,
	AVIOCTRL_PTZ_SET_MODE_STOP			= 14,
	AVIOCTRL_PTZ_MODE_RUN				= 15,

	AVIOCTRL_PTZ_MENU_OPEN				= 16, 
	AVIOCTRL_PTZ_MENU_EXIT				= 17,
	AVIOCTRL_PTZ_MENU_ENTER				= 18,

	AVIOCTRL_PTZ_FLIP					= 19,
	AVIOCTRL_PTZ_START					= 20,

	AVIOCTRL_LENS_APERTURE_OPEN			= 21,
	AVIOCTRL_LENS_APERTURE_CLOSE		= 22,

	AVIOCTRL_LENS_ZOOM_IN				= 23, 
	AVIOCTRL_LENS_ZOOM_OUT				= 24,

	AVIOCTRL_LENS_FOCAL_NEAR			= 25,
	AVIOCTRL_LENS_FOCAL_FAR				= 26,

	AVIOCTRL_AUTO_PAN_SPEED				= 27,
	AVIOCTRL_AUTO_PAN_LIMIT				= 28,
	AVIOCTRL_AUTO_PAN_START				= 29,

	AVIOCTRL_PATTERN_START				= 30,
	AVIOCTRL_PATTERN_STOP				= 31,
	AVIOCTRL_PATTERN_RUN				= 32,

	AVIOCTRL_SET_AUX					= 33,
	AVIOCTRL_CLEAR_AUX					= 34,
	AVIOCTRL_MOTOR_RESET_POSITION		= 35,
}ENUM_PTZCMD;


typedef enum {
    AVIOCTRL_IMAGE_PAUSE            = 0x00,
    AVIOCTRL_IMAGE_STOP             = 0x01,
    AVIOCTRL_IMAGE_STEPFORWARD      = 0x02,
    AVIOCTRL_IMAGE_STEPBACKWARD     = 0x03,
    AVIOCTRL_IMAGE_FORWARD          = 0x04,
    AVIOCTRL_IMAGE_BACKWARD         = 0x05,
    AVIOCTRL_IMAGE_SEEKTIME         = 0x06,
    AVIOCTRL_IMAGE_END              = 0x07,
    AVIOCTRL_IMAGE_START            = 0x10
} ENUM_IMAGECONTROL;

/////////////////////////////////////////////////////////////////////////////
///////////////////////// Message Body Define ///////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/*
IOTYPE_USER_IPCAM_START 				= 0x01FF,
IOTYPE_USER_IPCAM_STOP	 				= 0x02FF,
IOTYPE_USER_IPCAM_AUDIOSTART 			= 0x0300,
IOTYPE_USER_IPCAM_AUDIOSTOP 			= 0x0301,
IOTYPE_USER_IPCAM_SPEAKERSTART 			= 0x0350,
IOTYPE_USER_IPCAM_SPEAKERSTOP 			= 0x0351,
IOTYPE_USER_IPCAM_PUSH_STREAMING_START  = 0x600,
IOTYPE_USER_IPCAM_PUSH_STREAMING_STOP   = 0x601,
** @struct SMsgAVIoctrlAVStream
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
} SMsgAVIoctrlAVStream;


/*
IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ		= 0x0322,
** @struct SMsgAVIoctrlGetStreamCtrlReq
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetStreamCtrlReq;

/*
IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ		= 0x0320,
IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP	= 0x0323,
** @struct SMsgAVIoctrlSetStreamCtrlReq, SMsgAVIoctrlGetStreamCtrlResq
*/
typedef struct
{
	unsigned int  channel;	// Camera Index
	unsigned char quality;	//refer to ENUM_QUALITY_LEVEL
	unsigned char reserved[3];
} SMsgAVIoctrlSetStreamCtrlReq, SMsgAVIoctrlGetStreamCtrlResp;

/*
IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP	= 0x0321,
** @struct SMsgAVIoctrlSetStreamCtrlResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetStreamCtrlResp;


/*
IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ	= 0x0326,
** @struct SMsgAVIoctrlGetMotionDetectReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetMotionDetectReq;


/*
IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ		= 0x0324,
IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP		= 0x0327,
** @struct SMsgAVIoctrlSetMotionDetectReq, SMsgAVIoctrlGetMotionDetectResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned int sensitivity; 	// 0(Disabled) ~ 100(MAX):
								// index		sensitivity value
								// 0			0
								// 1			25
								// 2			50
								// 3			75
								// 4			100
}SMsgAVIoctrlSetMotionDetectReq, SMsgAVIoctrlGetMotionDetectResp;


/*
IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP	= 0x0325,
** @struct SMsgAVIoctrlSetMotionDetectResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetMotionDetectResp;


/*
IOTYPE_USER_IPCAM_DEVINFO_REQ			= 0x0330,
** @struct SMsgAVIoctrlDeviceInfoReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlDeviceInfoReq;


/*
IOTYPE_USER_IPCAM_DEVINFO_RESP			= 0x0331,
** @struct SMsgAVIoctrlDeviceInfo
*/
typedef struct
{
	unsigned char model[16];	// IPCam mode
	unsigned char vendor[16];	// IPCam manufacturer
	unsigned int version;		// IPCam firmware version	ex. v1.2.3.4 => 0x01020304;  v1.0.0.2 => 0x01000002
	unsigned int channel;		// Camera index
	unsigned int total;			// 0: No cards been detected or an unrecognizeable sdcard that could not be re-formatted.
								// -1: if camera detect an unrecognizable sdcard, and could be re-formatted
								// otherwise: return total space size of sdcard (MBytes)								
								
	unsigned int free;			// Free space size of sdcard (MBytes)
	unsigned char reserved[8];	// reserved
}SMsgAVIoctrlDeviceInfoResp;

/*
IOTYPE_USER_IPCAM_SETPASSWORD_REQ		= 0x0332,
** @struct SMsgAVIoctrlSetPasswdReq
*/
typedef struct
{
	char oldpasswd[32];			// The old security code
	char newpasswd[32];			// The new security code
}SMsgAVIoctrlSetPasswdReq;


/*
IOTYPE_USER_IPCAM_SETPASSWORD_RESP		= 0x0333,
** @struct SMsgAVIoctrlSetPasswdResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetPasswdResp;


/*
IOTYPE_USER_IPCAM_LISTWIFIAP_REQ		= 0x0340,
** @struct SMsgAVIoctrlListWifiApReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlListWifiApReq;

typedef struct
{
	char ssid[32]; 				// WiFi ssid
	char mode;	   				// refer to ENUM_AP_MODE
	char enctype;  				// refer to ENUM_AP_ENCTYPE
	char signal;   				// signal intensity 0--100%
	char status;   				// 0 : invalid ssid or disconnected
								// 1 : connected with default gateway
								// 2 : unmatched password
								// 3 : weak signal and connected
								// 4 : selected:
								//		- password matched and
								//		- disconnected or connected but not default gateway
}SWifiAp;

/*
IOTYPE_USER_IPCAM_LISTWIFIAP_RESP		= 0x0341,
** @struct SMsgAVIoctrlListWifiApResp
*/
typedef struct
{
	unsigned int number; // MAX number: 1024(IOCtrl packet size) / 36(bytes) = 28
	SWifiAp stWifiAp[1];
}SMsgAVIoctrlListWifiApResp;

/*
IOTYPE_USER_IPCAM_SETWIFI_REQ			= 0x0342,
** @struct SMsgAVIoctrlSetWifiReq
*/
typedef struct
{
	unsigned char ssid[32];			//WiFi ssid
	unsigned char password[32];		//if exist, WiFi password
	unsigned char mode;				//refer to ENUM_AP_MODE
	unsigned char enctype;			//refer to ENUM_AP_ENCTYPE
	unsigned char reserved[10];
}SMsgAVIoctrlSetWifiReq;

//IOTYPE_USER_IPCAM_SETWIFI_REQ_2		= 0x0346,
typedef struct
{
	unsigned char ssid[32];		// WiFi ssid
	unsigned char password[64];	// if exist, WiFi password
	unsigned char mode;			// refer to ENUM_AP_MODE
	unsigned char enctype;		// refer to ENUM_AP_ENCTYPE
	unsigned char reserved[10];
}SMsgAVIoctrlSetWifiReq2;

/*
IOTYPE_USER_IPCAM_SETWIFI_RESP			= 0x0343,
** @struct SMsgAVIoctrlSetWifiResp
*/
typedef struct
{
	int result; //0: wifi connected; 1: failed to connect
	unsigned char reserved[4];
}SMsgAVIoctrlSetWifiResp;

/*
IOTYPE_USER_IPCAM_GETWIFI_REQ			= 0x0344,
** @struct SMsgAVIoctrlGetWifiReq
*/
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlGetWifiReq;

/*
IOTYPE_USER_IPCAM_GETWIFI_RESP			= 0x0345,
** @struct SMsgAVIoctrlGetWifiResp //if no wifi connected, members of SMsgAVIoctrlGetWifiResp are all 0
*/
typedef struct
{
	unsigned char ssid[32];		// WiFi ssid
	unsigned char password[32]; // WiFi password if not empty
	unsigned char mode;			// refer to ENUM_AP_MODE
	unsigned char enctype;		// refer to ENUM_AP_ENCTYPE
	unsigned char signal;		// signal intensity 0--100%
	unsigned char status;		// refer to "status" of SWifiAp
}SMsgAVIoctrlGetWifiResp;

//changed: WI-FI Password 32bit Change to 64bit 
//IOTYPE_USER_IPCAM_GETWIFI_RESP_2    = 0x0347,
typedef struct
{
 unsigned char ssid[32];	 // WiFi ssid
 unsigned char password[64]; // WiFi password if not empty
 unsigned char mode;	// refer to ENUM_AP_MODE
 unsigned char enctype; // refer to ENUM_AP_ENCTYPE
 unsigned char signal;  // signal intensity 0--100%
 unsigned char status;  // refer to "status" of SWifiAp
}SMsgAVIoctrlGetWifiResp2;

/*
IOTYPE_USER_IPCAM_GETRECORD_REQ			= 0x0312,
** @struct SMsgAVIoctrlGetRecordReq
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetRecordReq;

/*
IOTYPE_USER_IPCAM_SETRECORD_REQ			= 0x0310,
IOTYPE_USER_IPCAM_GETRECORD_RESP		= 0x0313,
** @struct SMsgAVIoctrlSetRecordReq, SMsgAVIoctrlGetRecordResp
*/
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned int recordType;	// Refer to ENUM_RECORD_TYPE
	unsigned char reserved[4];
}SMsgAVIoctrlSetRecordReq, SMsgAVIoctrlGetRecordResp;

/*
IOTYPE_USER_IPCAM_SETRECORD_RESP		= 0x0311,
** @struct SMsgAVIoctrlSetRecordResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetRecordResp;


/*
IOTYPE_USER_IPCAM_GETRCD_DURATION_REQ	= 0x0316,
** @struct SMsgAVIoctrlGetRcdDurationReq
*/
typedef struct
{
	unsigned int channel; // Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetRcdDurationReq;

/*
IOTYPE_USER_IPCAM_SETRCD_DURATION_REQ	= 0x0314,
IOTYPE_USER_IPCAM_GETRCD_DURATION_RESP  = 0x0317,
** @struct SMsgAVIoctrlSetRcdDurationReq, SMsgAVIoctrlGetRcdDurationResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned int presecond; 	// pre-recording (sec)
	unsigned int durasecond;	// recording (sec)
}SMsgAVIoctrlSetRcdDurationReq, SMsgAVIoctrlGetRcdDurationResp;


/*
IOTYPE_USER_IPCAM_SETRCD_DURATION_RESP  = 0x0315,
** @struct SMsgAVIoctrlSetRcdDurationResp
*/
typedef struct
{
	int result;	// 0: success; otherwise: failed.
	unsigned char reserved[4];
}SMsgAVIoctrlSetRcdDurationResp;


typedef struct
{
	unsigned short year;	// The number of year.
	unsigned char month;	// The number of months since January, in the range 1 to 12.
	unsigned char day;		// The day of the month, in the range 1 to 31.
	unsigned char wday;		// The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
	unsigned char hour;     // The number of hours past midnight, in the range 0 to 23.
	unsigned char minute;   // The number of minutes after the hour, in the range 0 to 59.
	unsigned char second;   // The number of seconds after the minute, in the range 0 to 59.
}STimeDay;

/*
IOTYPE_USER_IPCAM_LISTEVENT_REQ			= 0x0318,
** @struct SMsgAVIoctrlListEventReq
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	STimeDay stStartTime; 		// Search event from ...
	STimeDay stEndTime;	  		// ... to (search event)
	unsigned char event;  		// event type, refer to ENUM_EVENTTYPE
	unsigned char status; 		// 0x00: Recording file exists, Event unreaded
								// 0x01: Recording file exists, Event readed
								// 0x02: No Recording file in the event
	unsigned char reserved[2];
}SMsgAVIoctrlListEventReq;


typedef struct
{
	STimeDay stTime;
	unsigned char event;
	unsigned char status;	// 0x00: Recording file exists, Event unreaded
							// 0x01: Recording file exists, Event readed
							// 0x02: No Recording file in the event
	unsigned char reserved[2];
}SAvEvent;
	
/*
IOTYPE_USER_IPCAM_LISTEVENT_RESP		= 0x0319,
** @struct SMsgAVIoctrlListEventResp
*/
typedef struct
{
	unsigned int  channel;		// Camera Index
	unsigned int  total;		// Total event amount in this search session
	unsigned char index;		// package index, 0,1,2...; 
								// because avSendIOCtrl() send package up to 1024 bytes one time, you may want split search results to serveral package to send.
	unsigned char endflag;		// end flag; endFlag = 1 means this package is the last one.
	unsigned char count;		// how much events in this package
	unsigned char reserved[1];
	SAvEvent stEvent[1];		// The first memory address of the events in this package
}SMsgAVIoctrlListEventResp;

	
/*
IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL 	= 0x031A,
** @struct SMsgAVIoctrlPlayRecord
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned int command;	// play record command. refer to ENUM_PLAYCONTROL
	unsigned int Param;		// command param, that the user defined
	STimeDay stTimeDay;		// Event time from ListEvent
	unsigned char reserved[4];
} SMsgAVIoctrlPlayRecord;

/*
IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP 	= 0x031B,
** @struct SMsgAVIoctrlPlayRecordResp
*/
typedef struct
{
	unsigned int command;       // Play record command. refer to ENUM_PLAYCONTROL
	unsigned int result;        // Depends on command
                                // when is AVIOCTRL_RECORD_PLAY_START:
                                //	result>=0   real channel no used by device for playback
                                //	result <0	error
                                //			-1	playback error
                                //			-2	exceed max allow client amount
    unsigned int size;          
    unsigned char respond;
    unsigned char reserved[3];
} SMsgAVIoctrlPlayRecordResp; // only for play record start command


/*
IOTYPE_USER_IPCAM_PTZ_COMMAND	= 0x1001,	// P2P Ptz Command Msg 
** @struct SMsgAVIoctrlPtzCmd
*/
typedef struct
{
	unsigned char control;	// PTZ control command, refer to ENUM_PTZCMD
	unsigned char speed;	// PTZ control speed
	unsigned char point;	// no use in APP so far. preset position, for RS485 PT
	unsigned char limit;	// no use in APP so far. 
	unsigned char aux;		// no use in APP so far. auxiliary switch, for RS485 PT
	unsigned char channel;	// camera index
	unsigned char reserve[2];
} SMsgAVIoctrlPtzCmd;

/*
IOTYPE_USER_IPCAM_EVENT_REPORT	= 0x1FFF,	// Device Event Report Msg 
*/
/** @struct SMsgAVIoctrlEvent
 */
typedef struct
{
	STimeDay stTime;
	unsigned long time; 	// UTC Time
	unsigned int  channel; 	// Camera Index
	unsigned int  event; 	// Event Type
	unsigned char reserved[4];
} SMsgAVIoctrlEvent;

/**
 IOTYPE_USER_IPCAM_GET_PLAYBACK_REQ          = 0x033A
 
 @struct SMsgAVIoctrlGetPlaybackReq
 */
typedef struct{
    
    unsigned int channel; // Camera Index
    STimeDay stTimeDay; // Event time from ListEvent
    unsigned char reserved[4];
    
}SMsgAVIoctrlGetPlaybackReq;

/**
 IOTYPE_USER_IPCAM_GET_PLAYBACK_RESP          = 0x033B
 
 @struct SMsgAVIoctrlGetPlaybackResp
 */
typedef struct{
    
    unsigned int videoTime;//event video time in seconds
    unsigned long size;//event size in byte
    unsigned char reserved[4];
    
}SMsgAVIoctrlGetPlaybackResp;

/**
 IOTYPE_USER_IPCAM_SET_RECORD_PROGRESS_REQ          = 0x033C
 
 @struct SMsgAVIoctrlSetRecordProgessReq
 */
typedef struct{
    
    unsigned int channel; // Camera Index
    STimeDay stTimeDay; // Event time from ListEvent
    unsigned int progressTime;//event progress time in seconds
    unsigned char reserved[4];
    
}SMsgAVIoctrlSetRecordProgessReq;

/**
 IOTYPE_USER_IPCAM_SET_RECORD_PROGRESS_RESP          = 0x033D
 
 @struct SMsgAVIoctrlSeRecordProgressResp
 */
typedef struct{
    
    unsigned char result;//0 means success
    unsigned char reserved[3];
    
}SMsgAVIoctrlSeRecordProgressResp;

#if 0

/* 	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_REQ	= 0x0400,	// Get Event Config Msg Request 
 */
/** @struct SMsgAVIoctrlGetEventConfig
 */
typedef struct
{
	unsigned int	channel; 		  //Camera Index
	unsigned char   externIoOutIndex; //extern out index: bit0->io0 bit1->io1 ... bit7->io7;=1: get this io value or not get
    unsigned char   externIoInIndex;  //extern in index: bit0->io0 bit1->io1 ... bit7->io7; =1: get this io value or not get
	char reserved[2];
} SMsgAVIoctrlGetEventConfig;
 
/*
	IOTYPE_USER_IPCAM_GET_EVENTCONFIG_RESP	= 0x0401,	// Get Event Config Msg Response 
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_REQ	= 0x0402,	// Set Event Config Msg req 
*/
/* @struct SMsgAVIoctrlSetEventConfig
 * @struct SMsgAVIoctrlGetEventCfgResp
 */
typedef struct
{
	unsigned int    channel;        // Camera Index
	unsigned char   mail;           // enable send email
	unsigned char   ftp;            // enable ftp upload photo
	unsigned char   externIoOutStatus;   // enable extern io output //bit0->io0 bit1->io1 ... bit7->io7; 1:on; 0:off
	unsigned char   p2pPushMsg;			 // enable p2p push msg
	unsigned char   externIoInStatus;    // enable extern io input  //bit0->io0 bit1->io1 ... bit7->io7; 1:on; 0:off
	char            reserved[3];
}SMsgAVIoctrlSetEventConfig, SMsgAVIoctrlGetEventCfgResp;

/*
	IOTYPE_USER_IPCAM_SET_EVENTCONFIG_RESP	= 0x0403,	// Set Event Config Msg resp 
*/
/** @struct SMsgAVIoctrlSetEventCfgResp
 */
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned int result;	// 0: success; otherwise: failed.
}SMsgAVIoctrlSetEventCfgResp;

#endif


/*
IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ		= 0x0360,
** @struct SMsgAVIoctrlSetEnvironmentReq
*/
typedef struct
{
	unsigned int channel;		// Camera Index
	unsigned char mode;			// refer to ENUM_ENVIRONMENT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetEnvironmentReq;


/*
IOTYPE_USER_IPCAM_SET_ENVIRONMENT_RESP		= 0x0361,
** @struct SMsgAVIoctrlSetEnvironmentResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char result;		// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetEnvironmentResp;


/*
IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ		= 0x0362,
** @struct SMsgAVIoctrlGetEnvironmentReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetEnvironmentReq;

/*
IOTYPE_USER_IPCAM_GET_ENVIRONMENT_RESP		= 0x0363,
** @struct SMsgAVIoctrlGetEnvironmentResp
*/
typedef struct
{
	unsigned int channel; 		// Camera Index
	unsigned char mode;			// refer to ENUM_ENVIRONMENT_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetEnvironmentResp;


/*
IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ			= 0x0370,
** @struct SMsgAVIoctrlSetVideoModeReq
*/
typedef struct
{
	unsigned int channel;	// Camera Index
	unsigned char mode;		// refer to ENUM_VIDEO_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlSetVideoModeReq;


/*
IOTYPE_USER_IPCAM_SET_VIDEOMODE_RESP		= 0x0371,
** @struct SMsgAVIoctrlSetVideoModeResp
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char result;	// 0: success; otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlSetVideoModeResp;


/*
IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ			= 0x0372,
** @struct SMsgAVIoctrlGetVideoModeReq
*/
typedef struct
{
	unsigned int channel; 	// Camera Index
	unsigned char reserved[4];
}SMsgAVIoctrlGetVideoModeReq;


/*
IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP		= 0x0373,
** @struct SMsgAVIoctrlGetVideoModeResp
*/
typedef struct
{
	unsigned int  channel; 	// Camera Index
	unsigned char mode;		// refer to ENUM_VIDEO_MODE
	unsigned char reserved[3];
}SMsgAVIoctrlGetVideoModeResp;


/*
/IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ			= 0x0380,
** @struct SMsgAVIoctrlFormatExtStorageReq
*/
typedef struct
{
	unsigned int storage; 	// Storage index (ex. sdcard slot = 0, internal flash = 1, ...)
	unsigned char reserved[4];
}SMsgAVIoctrlFormatExtStorageReq;


/*
IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ		= 0x0381,
** @struct SMsgAVIoctrlFormatExtStorageResp
*/
typedef struct
{
	unsigned int  storage; 	// Storage index
	unsigned char result;	// 0: success;
							// -1: format command is not supported.
							// otherwise: failed.
	unsigned char reserved[3];
}SMsgAVIoctrlFormatExtStorageResp;


typedef struct
{
	unsigned short index;		// the stream index of camera
	unsigned short channel;		// the channel index used in AVAPIs, that is ChID in avServStart2(...,ChID)
	char reserved[4];
}SStreamDef;


/*	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ			= 0x0328,
 */
typedef struct
{
	unsigned char reserved[4];
}SMsgAVIoctrlGetSupportStreamReq;


/*	IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP			= 0x0329,
 */
typedef struct
{
	unsigned int number; 		// the quanity of supported audio&video stream or video stream
	SStreamDef streams[1];
}SMsgAVIoctrlGetSupportStreamResp;


/* IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ			= 0x032A, //used to speak. but once camera is connected by App, send this at once.
 */
typedef struct
{
	unsigned int channel;		// camera index
	char reserved[4];
}SMsgAVIoctrlGetAudioOutFormatReq;

/* IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP			= 0x032B,
 */
typedef struct
{
	unsigned int channel;		// camera index
	int codecId;				// refer to ENUM_CODECID in AVFRAMEINFO.h
    char sample_rate;
    char bitdata;
    char channels;  //how many channels is used
    char avservchannel; 
}SMsgAVIoctrlGetAudioOutFormatResp;

/* IOTYPE_USER_IPCAM_RECEIVE_FIRST_IFRAME			= 0x1002,
 */
typedef struct
{
	unsigned int channel;		// camera index
	char reserved[4];
}SMsgAVIoctrlReceiveFirstIFrame;

/* IOTYPE_USER_IPCAM_GET_FLOWINFO_REQ              = 0x390
 */
typedef struct
{
	unsigned int channel;			// camera index
	unsigned int collect_interval;	// seconds of interval to collect flow information
									// send 0 indicates stop collecting.
}SMsgAVIoctrlGetFlowInfoReq;

/* IOTYPE_USER_IPCAM_GET_FLOWINFO_RESP            = 0x391
 */
typedef struct
{
	unsigned int channel;			// camera index
	unsigned int collect_interval;	// seconds of interval client will collect flow information
}SMsgAVIoctrlGetFlowInfoResp;

/* IOTYPE_USER_IPCAM_CURRENT_FLOWINFO              = 0x392
 */
typedef struct
{
	unsigned int channel;						// camera index
	unsigned int total_frame_count;				// Total frame count in the specified interval
	unsigned int lost_incomplete_frame_count;	// Total lost and incomplete frame count in the specified interval
	unsigned int total_expected_frame_size;		// Total expected frame size from avRecvFrameData2()
	unsigned int total_actual_frame_size;		// Total actual frame size from avRecvFrameData2()
	unsigned int timestamp_ms;					// Timestamp in millisecond of this report.
	char reserved[8];
}SMsgAVIoctrlCurrentFlowInfo;

/* IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ               = 0x3A0
 * IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP              = 0x3A1
 * IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ               = 0x3B0
 * IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP              = 0x3B1
 */
typedef struct
{
	int cbSize;							// the following package size in bytes, should be sizeof(SMsgAVIoctrlTimeZone)
	int nIsSupportTimeZone;
	int nGMTDiff;						// the difference between GMT in hours
	char szTimeZoneString[256];			// the timezone description string in multi-bytes char format
}SMsgAVIoctrlTimeZone;

/*
// dropbox support
IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_REQ      = 0x500,
IOTYPE_USER_IPCAM_GET_SAVE_DROPBOX_RESP     = 0x501,
*/
typedef struct
{
    unsigned short nSupportDropbox;     // 0:no support/ 1: support dropbox
    unsigned short nLinked;             // 0:no link/ 1:linked
    char szLinkUDID[64];                // Link UDID for App
}SMsgAVIoctrlGetDropbox;


/*
 // dropbox support
 IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_REQ      = 0x502,
 IOTYPE_USER_IPCAM_SET_SAVE_DROPBOX_RESP     = 0x503,
 */
typedef struct
{
    unsigned short nLinked;             // 0:no link/ 1:linked
    char szLinkUDID[64];                // UDID for App
    char szAccessToken[128];             // Oauth token
    char szAccessTokenSecret[128];       // Oauth token secret
	char szAppKey[128];                  // App Key (reserved)
	char szSecret[128];                  // Secret  (reserved)
}SMsgAVIoctrlSetDropbox;

/*
IOTYPE_USER_IPCAM_PUSH_STREAMING_START		= 0x202,
** @struct SMsgAVIoctrlPushStream
*/
typedef struct
{
	unsigned int channel;	// Camera Channel
	char szDeviceUID[20];	// Device UID
	unsigned int  event; 	// Event Type, refer to ENUM_EVENTTYPE
	unsigned char reserved[4];
} SMsgAVIoctrlPushStream;

typedef struct
{
	int AlarmType;
	long long AlarmTime;
	char AlarmDID[64];
} SMsgAVIoctrlAlarmingReq;

typedef struct
{
	char AppKey[64];
	char Master[64];
	char Alias[64];
	int Type;
} SMsgAVIoctrlSetPushReq,SMsgAVIoctrlDelPushReq;

/*
IOTYPE_USER_IPCAM_SET_CLOUD_RECORDING_ENDPOINT_REQ        = 0x0412
IOTYPE_USER_IPCAM_SET_CLOUD_RECORDING_ENDPOINT_RESP        = 0x0413
** @struct SMsgAVIoctrlSetCloudRecordingEndpoint
*/
typedef struct
{
    char Url[1024];
} SMsgAVIoctrlSetCloudRecordingEndpointReq;

typedef struct
{
    int result;    // 0: success; otherwise: failed.
    unsigned char reserved[4];
} SMsgAVIoctrlSetCloudRecordingEndpointResp;

// get local ip
//IOTYPE_USER_IPCAM_GET_LOCAL_IP_REQ          = 0x510,
typedef struct
{
    unsigned int channel;                        // camera index
}SMsgAVIoctrlGetLocalIPReq;


//IOTYPE_USER_IPCAM_GET_LOCAL_IP_RESP         = 0x511,
typedef struct
{
    unsigned int channel;                        // camera index
    char szLocalIP[32];                         // device local ip
}SMsgAVIoctrlGetLocalIPResp;


//自动巡航 ///////////////////////////////////////////////////////////
typedef enum
{
    AVIOCTRL_CRUISEMODE_VERTICAL   =0x00,
    AVIOCTRL_CRUISEMODE_HORIZONTAL =0x01,
}ENUM_CRUISE_MODE;
/*IOTYPE_HICHIP_CRUISE_START=0x600
 */
typedef struct
{
    unsigned int channel;      // Camera Index
    unsigned char mode;        // refer to ENUM_CRUISE_MODE
    unsigned char reserved[3];
}SMsgAVIoctrlCruiseStart;
/*IOTYPE_HICHIP_CRUISE_STOP=0x601
 */
typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char reserved[4];
} SMsgAVIoctrlCruiseStop;


//亮度调节////////////////////////////////////////////////////////////
/*IOTYPE_HICHIP_GETBRIGHT_REQ=0x602
 */
typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char reserved[4];
} SMsgAVIoctrlGetBrightReq;
/*IOTYPE_HICHIP_GETBRIGHT_RESP=0x603
 IOTYPE_HICHIP_SETBRIGHT_REQ=0x604
 */
typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char bright; // refer to ENUM_BRIGHT_LEVEL
    unsigned char reserved[3];
} SMsgAVIoctrlSetBrightReq, SMgAVIoctrlGetBrightResp;
/* AVIOCTRL BRIGHT Type */
typedef enum
{
    AVIOCTRL_BRIGHT_MAX            = 0x01,
    AVIOCTRL_BRIGHT_HIGH           = 0x02,
    AVIOCTRL_BRIGHT_MIDDLE         = 0x03,
    AVIOCTRL_BRIGHT_LOW            = 0x04,
    AVIOCTRL_BRIGHT_MIN            = 0x05,
}ENUM_BRIGHT_LEVEL;
/*IOTYPE_HICHIP_SETBRIGHT_RESP=0x605
 */
typedef struct
{
    unsigned int result; // 0: success; otherwise: failed.
    unsigned char reserved[4];
} SMsgAVIoctrSeltBrightResp;

/*IOTYPE_HICHIP_GETCONTRAST_REQ=0x606
 */
typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char reserved[4];
} SMsgAVIoctrlGetContrastReq;
/*IOTYPE_HICHIP_GETCONTRAST_RESP=0x607
 IOTYPE_HICHIP_SETCONTRAST_REQ=0x608
 */
typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char contrast; // refer to ENUM_CONTRAST_LEVEL
    unsigned char reserved[3];
} SMsgAVIoctrlSetContrastReq, SMgAVIoctrlGetContrastResp;
/* AVIOCTRL CONTRAST Type */
typedef enum
{
    AVIOCTRL_CONTRAST_MAX            = 0x01,
    AVIOCTRL_CONTRAST_HIGH           = 0x02,
    AVIOCTRL_CONTRAST_MIDDLE         = 0x03,
    AVIOCTRL_CONTRAST_LOW            = 0x04,
    AVIOCTRL_CONTRAST_MIN            = 0x05,
}ENUM_CONTRAST_LEVEL;
/*IOTYPE_HICHIP_SETCONTRAST_RESP=0x609
 */
typedef struct
{
    unsigned int result; // 0: success; otherwise: failed.
    unsigned char reserved[4];
} SMsgAVIoctrSetContrastResp;

typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nPresetIdx;  
    
}
SMsgAVIoctrlSetPresetReq,SMsgAVIoctrlGetPresetResp;

/* IOTYPE_USER_IPCAM_SETPRESET_RESP                = 0x441
 */
typedef struct
{
    int result;    // 回傳值    0: success; otherwise: failed
    unsigned char reserved[4];
}SMsgAVIoctrlSetPresetResp;

/* IOTYPE_USER_IPCAM_GETPRESET_REQ                = 0x442
 */
typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int nPresetIdx;    
}SMsgAVIoctrlGetPresetReq;



/* IOTYPE_USER_IPCAM_GETGUARD_REQ                = 0x420
 */
typedef struct
{
    unsigned int channel;       // AvServer Index
    unsigned char reserved[4];
} SMsgAVIoctrlGetGuardReq;

/* IOTYPE_USER_IPCAM_GETGUARD_RESP                = 0x421
 * IOTYPE_USER_IPCAM_SETGUARD_REQ                = 0x422
 */

typedef struct
{
    unsigned int channel;       // AvServer Index
    unsigned char alarm_motion_armed;        // ON=1, OFF=0
    unsigned char alarm_motion_sensitivity; // 1(MIN) ~ 100(MAX) 
    unsigned char alarm_preset;  /*OFF=0*/
    unsigned char alarm_mail;   /*Not allow=0, Allow=1*/
    
    unsigned int alarm_upload_interval; 
} SMsgAVIoctrlGetGuardResp, SMsgAVIoctrlSetGuardReq;

/* IOTYPE_USER_IPCAM_SETGUARD_RESP                = 0x423
 */
typedef struct
{
    int result;    // 0: success; otherwise: failed.
    unsigned char reserved[4];
    
} SMsgAVIoctrlSetGuardResp;



//FTP
typedef struct
{
    unsigned int channel;       // Camera Index
    unsigned char ftpServer[68] ; // 10.1.1.1
    int ftpPort;                   // 21
    unsigned char userName[20];
    unsigned char password[20];
    unsigned char path[68];
    int  passiveMode;// 0 - off, 1 - on
}SMsgAVIoctrlSetFtpReq, SMsgAVIoctrlGetFtpResp;

typedef struct
{
    unsigned int channel;         // Camera Index
    int result; //0: ok ; 1: failed
}SMsgAVIoctrlSetFtpResp;

typedef struct
{
    unsigned int channel;         // Camera Index
    unsigned char reserved[4];
}SMsgAVIoctrlGetFtpReq;


//mail设置////////////////////////////////////////////////////////
typedef struct {
    unsigned int channel;         // Camera Index
    unsigned char reserved[4];
} SMsgAVIoctrlExGetSmtpReq;

typedef struct {
    unsigned int channel;       // Camera Index
    char sender[64];        
    char receiver1[64];   
    char server[64];          /* mail server address*/
    unsigned int port;  
    unsigned int mail_tls;            /*0: no tls, 1: tls, 2:STARTLS*/
    char user[32];     
    char pwd[32];    
} SMsgAVIoctrlExSetSmtpReq, SMsgAVIoctrlExGetSmtpResp;

typedef struct
{
    int result; //0: ok ; 1: failed
    unsigned char reserved[4];
} SMsgAVIoctrlExSetSmtpResp;


//============================ Ambarella IO Command ===========================

// IOTYPE_USER_SETAMBCROPSET_REQ (0xFF001004)
typedef struct
{
    unsigned int x;     // x-axis from left
    unsigned int y;     // y-axis from top
    unsigned int width; // the width of cropped area
    unsigned int height; // the height of cropped area
    float        zoomScale;
    unsigned char channel; // camera index
    unsigned char reserved[3];
} SMsgAVIoctrlSetAmbCropSetReq;

//IOTYPE_USER_SETAMBCROPSET_RESP (0xFF001005)
typedef struct
{
    unsigned int result;    //0: success; otherwise: failed.
    unsigned char reserved[4];
} SMsgAVIoctrlSetAmbCropSetResp;



//IOTYPE_USER_GETAMBCROPSTATUS_REQ (0xFF001006)
typedef struct
{
    unsigned char channel; // camera index
    unsigned char reserved[3];
} SMsgAVIoctrlGetAmbCropStatusReq;


//IOTYPE_USER_GETAMBCROPSTATUS_RESP (0xFF001007)
typedef struct
{
    unsigned int status; //1: enable; 0: disanle.
    unsigned int x;     // x-axis from left
    unsigned int y;     // y-axis from top
    unsigned int width; // the width of cropped area
    unsigned int height; // the height of cropped area
    float        zoomScale;
    unsigned char reserved[4];
} SMsgAVIoctrlGetAmbCropStatusResp;


//IOTYPE_USER_SETAMBCROP_GETAMBCROPIMAGECHANGE_RESP (0xFF001008)
typedef struct
{
    unsigned int status;    //1: success; 0: fail.
    unsigned char reserved[4];
} SMsgAVIoctrlGetAmbCropImageChangeReq;

/* IOTYPE_USER_GETAMBLDCSTATUS_REQ             = 0xFF00100B
 */
typedef struct
{
    unsigned char channel;   // camera index
    unsigned char reserved[3];
}SMsgAVIoctrlGetAmbLDCStatusReq;

/* IOTYPE_USER_GETAMBLDCSTATUS_RESP            = 0xFF00100C
 */
typedef struct
{
    int status; //1:LDC ; 0:Normal
    unsigned char reserved[4];
}SMsgAVIoctrlGetAmbLDCStatusResp;


/*
 IOTYPE_USER_SETAMBLDCSET_REQ  = 0xFF001009
 ** @struct SMsgAVIoctrlSetAmbLDCSetReq;
 */
typedef struct
{
    unsigned int channel;  // Camera Index
    unsigned char mode;   // 1: set LDC mode; 0: set normal mode
    unsigned char reserved[3];
}SMsgAVIoctrlSetAmbLDCSetReq;


/*
 IOTYPE_USER_SETAMBLDCSET_RESP  = 0xFF00100A
 ** @struct SMsgAVIoctrlSetAmbLDCSetResp;
 */
typedef struct
{
    unsigned int channel;   // Camera Index
    unsigned char result;  // 0: success; otherwise: failed.
    unsigned char reserved[3];
}SMsgAVIoctrlSetAmbLDCSetResp;


//IOTYPE_USER_IPCAM_CHECKSUPPORTMESSAGETYPE_REQ (0x032C)
typedef struct
{
    unsigned int channel;       // avIndex
    unsigned int messageType;  // cmd ID
    unsigned char reserved[8];
} SMsgAVIoctrlCheckSupportMessageTypeReq;


//IOTYPE_USER_IPCAM_CHECKSUPPORTMESSAGETYPE_RESP (0x032D)
typedef struct
{
    unsigned int channel;       // avIndex
    unsigned int messageType;  // cmd ID
    unsigned char support;     // 0:not support, 1:support
    unsigned char reserved[7];
} SMsgAVIoctrlCheckSupportMessageTypeResp;


typedef struct
{
    STimeDay time;
    unsigned char event;
    unsigned char status;
    unsigned char type;             // type of file. 0:video 1:photo
    unsigned char length;           // path length
    unsigned char path[72];         // path
    
} SAvFile;

//IOTYPE_USER_IPCAM_LISTFILE_REQ     = 0x031C,
//IOTYPE_USER_IPCAM_LISTFILE_RESP    = 0x031D, // refer to ENUM_EVENTTYPE
typedef struct
{
    unsigned int channel;             // AvServer Index
    STimeDay stStartTime;             // start time
    STimeDay stEndTime;               // end time
    unsigned char event;              // enum ENUM_EVENTTYPE
    unsigned char status;             // enum EVENT_STATUS
    unsigned char type;               // type of file. 0:video 1:photo
    unsigned char reversed[9];
    
} SMsgAVIoctrlListFileReq;


typedef struct
{
    unsigned int channel;       // AvServer Index
    unsigned int total;
    unsigned char index;
    unsigned char endflag;
    unsigned char count;
    unsigned char type;         // type of file, 0:video 1:photo
    SAvFile stfile;
    
} SMsgAVIoctrlListFileResp;

//============================ NOVATEK IO Command ===========================
// IOTYPE_USER_IPCAM_NOVATEK_WIFICMD_REQ (0x7F000000)
// IOTYPE_USER_IPCAM_NOVATEK_WIFICMD_RESP (0x7F000001)
typedef struct
{
    unsigned int channel;               // AvServer Index
    unsigned int nCmdIdentify;          // (Option) a number let app can distinguish the response 0 (zero) means disable
    unsigned int nAVChannel4LongReq;    // (Option) if large sent data transfer required when request 0 (zero) means disable
    unsigned int nCmdID;                // The same as Command ID value defined in Doc: Novatek_IM_wifi_command list.xls
    unsigned int nTotalCount;           // The total amount of io command transfer times for this wifi command
    unsigned int nIndex;                // The index of io command transfer times for this wifi command
    unsigned int nDataLength;           // The valid data length in bytes of the param
    unsigned char param[972];           // The parameter string
    
} SMsgAVIoctrlWifiCmdReq,SMsgAVIoctrlWifiCmdResp;

// IOTYPE_USER_SETCLIENTRECVREADY_REQ  (0x7F000002)
// IOTYPE_USER_SETCLIENTRECVREADY_RESP (0x7F000003)
typedef struct
{
    unsigned int channel;    // nAVChannel4LongResp
    unsigned int reserved;
    
} SMsgAVIoctrlSetClientRecvReadyReq;


typedef struct
{
    unsigned int channel;    // AvServer Index
    unsigned int result;    // 0: success ; != 0: failure
    
} SMsgAVIoctrlSetClientRecvReadyResp;


//IOTYPE_USER_IPCAM_DOWNLOAD_FILE_REQ (0x7F000004)
typedef struct
{
    unsigned int channel;       // AvServer Index
    unsigned int length;        // path length
    unsigned char path[72];     // path
    unsigned char purpose;      // 0 : for download file, 1 : for show photo
    unsigned char reversed[7];
    
} SMsgAVIoctrlDownloadReq;


//IOTYPE_USER_IPCAM_DOWNLOAD_FILE_RESP (0x7F000005)
typedef struct
{
    unsigned int channel;         // AvServer Index
    unsigned int result;         // the channel which AvServStart
    unsigned char purpose;       // 0 : for download file, 1 : for show photo, refer to ENUM_DOWNLOAD_FILE
    unsigned char reversed[7];
    
} SMsgAVIoctrlDownloadResp;


//IOTYPE_USER_IPCAM_UPLOAD_FILE_REQ    (0x7F000006)

typedef struct {

    unsigned int channel;   // Camera Index
    unsigned int result;    // the channel which avServStart
} SMsgAVIoctrlUploadReq;

/*
IOTYPE_USER_IPCAM_LIVEVIEW_OPTIONS_REQ      = 0x03C0
*/
typedef struct
{
    unsigned int channel;           // AvServer Index
    unsigned char reversed[4];
    
} SMsgAVIoctrlLiveViewOptionsReq;


//IOTYPE_USER_IPCAM_LIVEVIEW_OPTIONS_RESP  = 0x3C1,
typedef struct
{
    unsigned int channel;              // AvServer Index
    unsigned char quality;            // reference by 0x0323
    unsigned char envMode;            // reference by 0x0363
    unsigned char contrast;           // reference by 0x607
    unsigned char bright;             // reference by 0x603
    unsigned char videoMode;          // reference by 0x0373
    unsigned char battery;            // reference by custom=1&cmd=3019
    unsigned char wdr;                // reference by custom=1&cmd=2020
    unsigned char timelapse;          // reference by custom=1&cmd=2021
    unsigned char reversed[4];
    
} SMsgAVIoctrlLiveViewOptionsResp;

/*
 IOTYPE_USER_IPCAM_SET_VSAAS_PLAYBACK_INFO_REQ    = 0x00010000,
 ** @struct SMsgAVIoctrlSetVSaaSPlaybackReq
 */
typedef struct
{
    char            szUID[20];
    int             nChannel;
    int             nRecType;       // 0:Fulltime recording 1:Event recording
    unsigned char   reserved[4];
}SMsgAVIoctrlSetVSaaSPlaybackReq;
/*
 IOTYPE_USER_IPCAM_SET_VSAAS_PLAYBACK_INFO_RESP   = 0x00010001,
 ** @struct SMsgAVIoctrlSetVSaaSPlaybackResp
 */
typedef struct
{
    int result; // 0: success; otherwise: failed.
    unsigned char reserved[4];
}SMsgAVIoctrlSetVSaaSPlaybackResp;
/*
 IOTYPE_USER_IPCAM_VSAAS_RECORD_PLAYCONTROL_REQ  = 0x00010002,
 ** @struct SMsgAVIoctrlVSaaSPlayRecordReq
 */
typedef struct
{
    unsigned int command;   // play record command. refer to ENUM_PLAYCONTROL
    STimeDay stTimeDay;     // Event time from ListEvent
    unsigned int offset;    // assign offset (seconds) in the same recording file
    unsigned char reserved[4];
} SMsgAVIoctrlVSaaSPlayRecordReq;
/*
 IOTYPE_USER_IPCAM_VSAAS_RECORD_PLAYCONTROL_RESP     = 0x00010003,
 ** @struct SMsgAVIoctrlVSaaSPlayRecordResp
 */
typedef struct
{
    unsigned int command;   // Play record command. refer to ENUM_PLAYCONTROL
    unsigned int result;   // 0: success; otherwise: failed.
    unsigned char reserved[4];
} SMsgAVIoctrlVSaaSPlayRecordResp; // only for play record start command


/// AirLive
/*
 IOTYPE_USER_GETIPAddress_REQ = 0x701
 */
typedef struct
{
    unsigned int value;    
} SMsgAVIoctrlGetIPAddressReq;
/*
 IOTYPE_USER_GETIPAddress_RESP = 0x702
 */
typedef struct
{
    int First;       
    int Second;   
    int Third;    
    int Fourth;    
    int Port;   
    
} SMsgAVIoctrlGetIPAddressResp;


//NVR Time zone
typedef struct
{
    unsigned char reserved[4];
}SMsgAVIoctrlGetNVRTIMEZONEReq;

typedef struct
{
    int index; //Index in timezone value
}SMsgAVIoctrlSetNVRTIMEZONEReq,SMsgAVIoctrlGetNVRTIMEZONEResp;

typedef struct
{
    char index; //0:success;otherwise:failed.
    unsigned char reserved[3];
}SMsgAVIoctrlSetNVRTIMEZONEResp;


typedef struct{
    unsigned int channel;
}SMsgAVIoctrlGetRecordDurationReq;

typedef struct{
    unsigned int channel;
    unsigned int index; // Refer to ENUM_RECORD_DURATION
    unsigned char reserved[4];
}SMsgAVIoctrlGetRecordDurationResp, SMsgAVIoctrlSetRecordDurationReq;

typedef struct{
    int result;  // 0: success; otherwise: failed.
    unsigned char reserved[4];
}SMsgAVIoctrlSetRecordDurationResp;


typedef struct{
    unsigned char reserved[4];
}SMsgAVIoctrlGetChannelNameReq;

typedef struct{
    unsigned int channel; //camera index
    unsigned char name[24]; //channel name
} SChannelInfo;

typedef struct{
    unsigned char count; // quantity of  channels
    unsigned char reserved[3];
    SChannelInfo sChannelInfo[0]; //first channelInfo, and total count channelInfo
}SMsgAVIoctrlGetChannelNameResp;

typedef struct{
    unsigned char count; // quantity of  SChannelInfos
    unsigned char reserved[3];
    SChannelInfo sChannelInfo[0]; //first channelInfo, and total count channelInfo
}SMsgAVIoctrlSetChannelNameReq;

typedef struct{
    unsigned char result;    // 0 success, other failed
    unsigned char reserved[3];
}SMsgAVIoctrlSetChannelNameResp;

//码流切换
typedef enum{
    TYPE_MASTER      = 0x0,
    TYPE_SUB         = 0x1
} ENUM_VIDEO_TYPE;

typedef struct{
    unsigned char channel; // camera index
    unsigned char videoType; // refer to ENUM_VIDEO_TYPE
    unsigned char reserved[2];
}SMsgAVIoctrlGetVideoStreamReq;

typedef struct{
    unsigned char channel; // camera index
    unsigned char videoType; // refer to ENUM_VIDEO_TYPE
    unsigned char reserved[2];
}SMsgAVIoctrlGetVideoStreamResp;

typedef struct{
    unsigned char channel; // camera index
    unsigned char videoType; // refer to ENUM_VIDEO_TYPE
    unsigned char reserved[2];
}SMsgAVIoctrlSetVideoStreamReq;

typedef struct{
    unsigned char result;    // 0 success, other failed
    unsigned char reserved[3];
}SMsgAVIoctrlSetVideoStreamResp;

typedef struct{
    unsigned int channel;        // Camera Index
    unsigned int index;          
    STimeDay stStartTime;      // Search event from ...
    STimeDay stEndTime;       // ... to (search event)
    unsigned char event;         // event type, refer to ENUM_EVENTTYPE
    unsigned char status;        // 0x00: Recording file exists, Event unreaded
    // 0x01: Recording file exists, Event readed
    // 0x02: No Recording file in the event
    unsigned char recordType;  //0:Alarm 1:Alarm+Manual
    unsigned char imageCommand; // refer to ENUM_IMAGECONTROL
}SMsgAVIoctrlGetEventImageReq;

typedef struct{
    unsigned char channel;        // Camera Index
    unsigned char flag;        //2:start flag  3:image flag 4:end image
    STimeDay eventName;
    unsigned char reserved[14];
}SEventImageInfo;

typedef struct{
    unsigned char imageCommand; // refer to ENUM_IMAGECONTROL
    unsigned char channel;        // camera index
    unsigned char reserved[2];
}SMsgAVIoctrlGetEventImageResp;

//IOTYPE_USER_IPCAM_SET_WAKEUPSWITCH_REQ      = 0X0740
//SMsgAVIoctrlSetWakeupSwitchReq
typedef struct
{
    unsigned int channel ; //camera index
    unsigned int value; //switch value 0:off, 1:on
    char reserved[4];
}SMsgAVIoctrlSetWakeupSwitchReq;

//IOTYPE_USER_IPCAM_SET_WAKEUPSWITCH_RESP     = 0X0741
//SMsgAVIoctrlSetWakeupSwitchResp
typedef struct
{
    int result ; //0:success, others failed
    char reserved[4];
}SMsgAVIoctrlSetWakeupSwitchResp;

//IOTYPE_USER_IPCAM_GET_WAKEUPSWITCH_REQ      = 0X0742
//SMsgAVIoctrlGetWakeupSwitchReq
typedef struct
{
    unsigned int channel ; //camera index
    char reserved[4];
}SMsgAVIoctrlGetWakeupSwitchReq;

//IOTYPE_USER_IPCAM_GET_WAKEUPSWITCH_RESP     = 0X0743
//SMsgAVIoctrlGetWakeupSwitchResp
typedef struct
{
    unsigned int value; //switch value 0:off, 1:on
    char reserved[4];
}SMsgAVIoctrlGetWakeupSwitchResp;

//Battery Level
typedef struct
{
    unsigned int channel;           // AvServer Index
    unsigned char reversed[4];
} SMsgAVIoctrlLiveviewOptionsReq,SMsgAVIoctrlGetBatteryLevelReq;

typedef struct
{
    unsigned int channel;           // AvServer Index
    unsigned char battery;          // reference by custom=1&cmd=3019
    unsigned char reversed[3];
} SMsgAVIoctrlGetBatteryLevelResp;

typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char reserved[4];
} SMsgAVIoctrlGetBatteryReq;

typedef struct
{
    unsigned int channel; // Camera Index
    unsigned char battery; // 0 – 100 (%) ; -1 : charging
    unsigned char reserved[3];
} SMsgAVIoctrlGetBatteryResp;


typedef struct
{
    unsigned int channel;         // Camera Index
    unsigned char reserved[4];
}SMsgAVIoctrlGetBatteryOtherReq;

typedef struct
{
    unsigned int battery; // 0 – 100 (%) ; -1 : charging
    unsigned char reserved[4];
}SMsgAVIoctrlGetBatteryOtherResp;


typedef struct
{
    int cbSize;                     // the following package size in bytes, should be sizeof(SMsgAVIoctrlTimeZone)
    int nIsSupportTimeZone;
    int nGMTDiff;                   // the difference between GMT in hours
    char szTimeZoneString[256];     // the timezone description string in multi-bytes char format
    long local_utc_time;            // the number of seconds passed
                                    // since the UNIX epoch (January 1, 1970 UTC)
    int dst_on;                     // daylight saving time, 0:off, 1:on
}SMsgAVIoctrlTimeZoneExt;


// Door Phone
typedef struct{
    unsigned  char  index;      //0:Door1,1:Door2
    STimeDay stTime;            //event time
    unsigned char reserved[3];
} SMsgAVIoctrlCallReq;

typedef struct{
    unsigned  char  index;      
    int nAnswered;              
    unsigned char reserved[3];
}SMsgAVIoctrlCallResp;

typedef struct{
    unsigned  char  index;      
    unsigned  char  type;      
    STimeDay stTime;            
    unsigned  char reserved[3];
}SMsgAVIoctrlCallInd;

typedef struct
{
    unsigned char bEnable;      
    unsigned int nTime;         
    unsigned char reserved[3];
}SAVDoor;

typedef struct
{
    unsigned  int count;        
    SAVDoor Door[0];            
} SMsgAVIoctrlGetDoorListResp;

typedef struct
{
    unsigned  char  index;      
    unsigned char nOpen;        
    unsigned int time;          
    unsigned char reserved[2];
} SMsgAVIoctrlSetDoorInfoReq;

typedef struct
{
    unsigned  char  index;      //0:Door1,1:Door2
    int result;                 //0x00 set password successfully,  or not failed
    unsigned char reserved[3];
} SMsgAVIoctrlSetDoorInfoResp;

typedef struct
{
    int nEnable;                
    unsigned char reserved[4];
}SMsgAVIoctrlGetUnlockSwitchResp;

typedef struct
{
    int nEnable;               
    char passwd[32];            
    unsigned char reserved[4];
}SMsgAVIoctrlSetUnlockSwitchReq;

typedef struct
{
    int result;                 //0:set password successfully, 1:passwd error, otherwise:failed
    unsigned char reserved[4];
}SMsgAVIoctrlSetUnlockSwitchResp;

typedef struct
{
    char oldpasswd[32];         
    char newpasswd[32];         
}SMsgAVIoctrlSetUnlockPasswdReq;

typedef struct
{
    int result;                 //0x00 set password successfully,  or not failed
    unsigned char reserved[4];
}SMsgAVIoctrlSetUnlockPasswdResp;

typedef struct
{
    int  nType;                 
    STimeDay stTime;            
    unsigned char reserved[4];
} SMsgAVIoctrlVerifyUnlockPasswdInd;

typedef struct{
    unsigned  int  channel;     // Camera Index
    unsigned  char  index;      
    char passwd[32];            
    unsigned char reserved[3];
} SMsgAVIoctrlUnlockReq;

typedef struct{
    unsigned  char  index;      //0�Door1,1:Door2
    int result;                 //0�success,1�password error, otherwise�failed
    unsigned char reserved[3];
} SMsgAVIoctrlUnlockResp;

typedef struct
{
    int  channel;               // Camera Index
    unsigned char reserved[4];
} SMsgAVIoctrlSetDeviceSleepReq;

//IOTYPE_USER_IPCAM_DEVICESLEEP_RESP = 0x721
typedef struct
{
    unsigned int result;
    unsigned char reserved[4];
} SMsgAVIoctrlSetDeviceSleepResp;


typedef struct
{
    unsigned int channel;       // Camera Index
    unsigned char reserved[4];
}SMsgAVIoctrlGetWifiSignalReq;

typedef struct
{
    int channel; // Camera Index
    unsigned int result;
    unsigned char reserved[3];
}SMsgAVIoctrlGetWifiSignalResp;

typedef struct
{
    unsigned int channel ;      //camera index
    char devicename[150];      
}SMsgAVIoctrlPushDeviceNameReq;

typedef struct
{
    int res ;                   //0:success, others failed
    char reserved[4];
}SMsgAVIoctrlPushDeviceNameResp;

typedef struct
{
    unsigned char reserved[8];
}SMsgAVIoctrlSetRebootReq;

typedef struct
{
    unsigned int  result;       // 0: success; otherwise: failed.
    unsigned char reserved[8];
}SMsgAVIoctrlSetRebootResp;


// IOTYPE_USER_IPCAM_GET_NVR_CHANNEL_NUMBER_REQ
typedef struct
{
    unsigned int channel;       // Camera Index
    unsigned char reserved[4];
}SMsgAVIoctrlGetNVRChannelNumReq;

//IOTYPE_USER_IPCAM_GET_NVR_CHANNEL_NUMBER_RESP

typedef struct
{
    int res ;                   //0:success, others failed
    char reserved[4];
}SMsgAVIoctrlGetNVRChannelNumResp;

/*
 IOTYPE_USER_IPCAM_OTA_REQ = 0x8001,
 ** @struct SMsgAVIoctrlOTAReq;
 */
typedef struct
{
    unsigned char file_checksum[32]; //MD5
    unsigned char url[256]; 
    unsigned int file_size; 
    unsigned char reserved[4];
}SMsgAVIoctrlOTAReq;

/*
 IOTYPE_USER_IPCAM_OTA_RESP = 0x8002
 ** @struct SMsgAVIoctrlOTAResp;
*/

typedef struct
{
    unsigned int progress; 
    unsigned int endflag; 
    unsigned char reserved[8];
}SMsgAVIoctrlOTAResp;


/*
 IOTYPE_USER_IPCAM_DEVICE_INFO_REQ = 0x8015,
 ** @struct SMsgAVIoctrlDeviceInfoReqEx;
*/
typedef struct
{
    unsigned char reserved[8];
}SMsgAVIoctrlDeviceInfoReqEx;


/*
 IOTYPE_USER_IPCAM_DEVICE_INFO_RESP = 0x8016,
 ** @struct SMsgAVIoctrlDeviceInfoRespEx;
*/
typedef struct
{
    unsigned char model[32]; 
    unsigned char product[32]; 
    unsigned char vender[32]; 
    unsigned int version; 
    unsigned int free; 
    unsigned int total;
    unsigned char region; 
    unsigned char reserved[3];
} SMsgAVIoctrlDeviceInfoRespEx;


/*
 IOTYPE_USER_IPCAM _DEVICE_SUPPORT_OTA_REQ = 0x800A,
 ** @struct SMsgAVIoctrlDeviceSupportOTAReq;
*/
typedef struct
{
    unsigned char reserved[8];
}SMsgAVIoctrlDeviceSupportOTAReq;


/*
 IOTYPE_USER_IPCAM_DEVICE_SUPPORT_OTA_RESP = 0x800B,
 ** @struct SMsgAVIoctrlDeviceSupportOTAResp;
*/
typedef struct
{
    unsigned char isSupport;
    unsigned char reserved[4];
} SMsgAVIoctrlDeviceSupportOTAResp;

/**
IOTYPE_USER_IPCAM_DEVICE_SUPPORT_CLOUD_REQ
@struct SMsgAVIoctrlSupportCloudReq
 */
typedef struct
{
    unsigned char reserved[8];
}SMsgAVIoctrlSupportCloudReq;

/**
IOTYPE_USER_IPCAM_DEVICE_SUPPORT_CLOUD_RESP
@struct SMsgAVIoctrlSupportCloudResp
 */
typedef struct
{
    unsigned int result;    // 0: Not Support, 1: Support
    unsigned char reserved[4];
}SMsgAVIoctrlSupportCloudResp;

/**
IOTYPE_USER_IPCAM_UPLOAD_CLOUD_INFO_REQ
@struct SMsgAVIoctrlUploadCloudInfoReq
 */
typedef struct
{
    unsigned char dm_token[108]; //dm token
    unsigned char url[128];
    unsigned char reserved[4];
}SMsgAVIoctrlUploadCloudInfoReq;

/**
IOTYPE_USER_IPCAM_UPLOAD_CLOUD_INFO_RESP
@struct SMsgAVIoctrlUploadCloudInfoResp
 */
typedef struct
{
    unsigned int result;    // 0: Not Received, 1: Received
    unsigned char reserved[4];
}SMsgAVIoctrlUploadCloudInfoResp;

/**
 IOTYPE_USER_IPCAM_UPLOAD_CLOUD_INFO_RESP
@struct SMsgAVIoctrlSetCloudReq
 */
typedef struct
{
    unsigned int channel;       // camera index
    unsigned int isOn;          // 1 on, 0 off
}SMsgAVIoctrlSetCloudReq;

/**
 IOTYPE_USER_IPCAM_DEVICE_SET_CLOUD_RESP
@struct SMsgAVIoctrlSetCloudResp
 */
typedef struct
{
    unsigned int channel;       // camera index
    unsigned int result;        // 0 successful, 1 fail
}SMsgAVIoctrlSetCloudResp;

/**
 IOTYPE_USER_IPCAM_DEVICE_GET_CLOUD_REQ
@struct SMsgAVIoctrlGetCloudReq
 */
typedef struct
{
    unsigned int channel;       // camera index
    unsigned char reserved[4];
}SMsgAVIoctrlGetCloudReq;

/**
 IOTYPE_USER_IPCAM_DEVICE_GET_CLOUD_RESP
@struct SMsgAVIoctrlGetCloudResp
 */
typedef struct
{
    unsigned int channel;       // camera index
    unsigned int isOn;          // 1 on, 0 off
}SMsgAVIoctrlGetCloudResp;

typedef struct
{
  unsigned char date[8]; 
} EventDate;

//IOTYPE_USER_IPCAM_GET_EVENT_DATE_REQ
typedef struct
{
    unsigned int channel; //Camera Index
    unsigned int eventType; 

    STimeDay stStartTime;
    STimeDay stEndTime; 
    unsigned char reserved[8];
} SMsgAVIoctrlGetEventDateReq;

//IOTYPE_USER_IPCAM_GET_EVENT_DATE_RESP
typedef struct
{
    unsigned int count; 
    unsigned char reserved[4];
    EventDate eventDate[1]; 
} SMsgAVIoctrlGetEventDateResp;

/**
 IOTYPE_USER_IPCAM_GET_HUMANDETECTION_REQ = 0x9002
@struct SMsgAVIoctrlGetHumanDetectionReq
 */
typedef struct
{
    unsigned char reserved[4];
} SMsgAVIoctrlGetHumanDetectionReq;

/**
 IOTYPE_USER_IPCAM_GET_HUMANDETECTION_RESP = 0x9003
 @struct SMsgAVIoctrlGetHumanDetectionResp
 */
typedef struct
{
unsigned int result;             // return 1 if open, otherwise close
unsigned char reserved[4];
} SMsgAVIoctrlGetHumanDetectionResp;

/**
 IOTYPE_USER_IPCAM_SET_HUMANDETECTION_REQ = 0x9004
 @struct SMsgAVIoctrlSetHumanDetectionReq
 */
typedef struct
{
    unsigned int isOn;        // 1 open, 0 close
    unsigned char reserved[4];
} SMsgAVIoctrlSetHumanDetectionReq;

/**
 IOTYPE_USER_IPCAM_SET_HUMANDETECTION_RESP = 0x9005
 @struct SMsgAVIoctrlSetHumanDetectionResp
 */
typedef struct
{
unsigned int result;             // return 0 if successful, otherwise fail
unsigned char reserved[4];
} SMsgAVIoctrlSetHumanDetectionResp;

/**
 IOTYPE_USER_IPCAM_GET_NIGHTVISION_REQ = 0x9006
 @struct SMsgAVIoctrlGetNightVisionReq
 */
typedef struct
{
    unsigned char reserved[4];
} SMsgAVIoctrlGetNightVisionReq;

/**
 IOTYPE_USER_IPCAM_GET_NIGHTVISION_RESP = 0x9007
 @struct SMsgAVIoctrlGetNightVisionResp
 */
typedef struct
{
unsigned int result;             // return 1 if open, otherwise close
unsigned char reserved[4];
} SMsgAVIoctrlGetNightVisionResp;

/**
 IOTYPE_USER_IPCAM_SET_NIGHTVISION_REQ = 0x9008
 @struct SMsgAVIoctrlSetNightVisionReq
 */
typedef struct
{
    unsigned int isOn;        // 1 open, 0 close
    unsigned char reserved[4];
} SMsgAVIoctrlSetNightVisionReq;

/**
 IOTYPE_USER_IPCAM_SET_NIGHTVISION_RESP = 0x9009
 @struct SMsgAVIoctrlSetNightVisionResp
 */
typedef struct
{
unsigned int result;             // return 0 if successful, otherwise fail
unsigned char reserved[4];
} SMsgAVIoctrlSetNightVisionResp;

/**
 IOTYPE_USER_IPCAM_GET_SUMMERTIME_REQ = 0x9010
 @struct SMsgAVIoctrlGetSummerTimeReq
 */
typedef struct
{
    unsigned char reserved[4];
} SMsgAVIoctrlGetSummerTimeReq;

/**
 IOTYPE_USER_IPCAM_GET_SUMMERTIME_RESP = 0x9011
 @struct SMsgAVIoctrlGetSummerTimeResp
 */
typedef struct
{
unsigned int result;             // return 1 if open, otherwise close
unsigned char reserved[4];
} SMsgAVIoctrlGetSummerTimeResp;

/**
 IOTYPE_USER_IPCAM_SET_SUMMERTIME_REQ = 0x9012
 @struct SMsgAVIoctrlSetSummerTimeReq
 */
typedef struct
{
    unsigned int isOn;        // 1 open, 0 close
    unsigned char reserved[4];
} SMsgAVIoctrlSetSummerTimeReq;

/**
 IOTYPE_USER_IPCAM_SET_SUMMERTIME_REQ = 0x9013
 @struct SMsgAVIoctrlSetSummerTimeResp
 */
typedef struct
{
    unsigned int result;             // return 0 if successful, otherwise fail
    unsigned char reserved[4];
} SMsgAVIoctrlSetSummerTimeResp;

#endif
