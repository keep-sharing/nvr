#ifndef _MS_DEFS_H_
#define _MS_DEFS_H_
#include "msstd.h"
#include "vapi/vapi.h"
#include "osa/osa.h"
#include "osa/osa_typedef.h"

#define     MAX_CAMERA                  (64)
#if defined(_HI3536_)
    #define MAX_PEOPLECNT_GROUP         (9)
    #define MAX_REAL_CAMERA             (64)
    #define MAX_RESOLUTION_WIDTH_IN     (4000)
    #define MAX_RESOLUTION_HEIGTH_IN    (3000)
    #define MAX_RESOLUTION_WIDTH_OUT    (3840)
    #define MAX_RESOLUTION_HEIGTH_OUT   (2160)
    #define MIN_RESOLUTION_WIDTH_OUT    (4000/15+1)
    #define MIN_RESOLUTION_HEIGTH_OUT   (3000/15)
    #define MAX_POS_CLIENT              (16)
#elif defined(_HI3536A_)
    #define MAX_PEOPLECNT_GROUP         (9)
    #define MAX_REAL_CAMERA             (64)
    #define MAX_RESOLUTION_WIDTH_IN     (4000)
    #define MAX_RESOLUTION_HEIGTH_IN    (3000)
    #define MAX_RESOLUTION_WIDTH_OUT    (3840)
    #define MAX_RESOLUTION_HEIGTH_OUT   (2160)
    #define MIN_RESOLUTION_WIDTH_OUT    (4000/15+1)
    #define MIN_RESOLUTION_HEIGTH_OUT   (3000/15)
    #define MAX_POS_CLIENT              (16)
#elif defined(_HI3798_)
    #define MAX_PEOPLECNT_GROUP         (9)
    #define MAX_REAL_CAMERA             (16)
    #define MAX_RESOLUTION_WIDTH_IN     (3840)
    #define MAX_RESOLUTION_HEIGTH_IN    (2160)
    #define MAX_RESOLUTION_WIDTH_OUT    (1920)
    #define MAX_RESOLUTION_HEIGTH_OUT   (1080)
    #define MIN_RESOLUTION_WIDTH_OUT    (480)
    #define MIN_RESOLUTION_HEIGTH_OUT   (320)
    #define MAX_POS_CLIENT              (8)
#elif defined(_HI3536C_)
    #define MAX_PEOPLECNT_GROUP         (4)
    #define MAX_REAL_CAMERA             (8)
    #define MAX_RESOLUTION_WIDTH_IN     (4000)
    #define MAX_RESOLUTION_HEIGTH_IN    (3000)
    #define MAX_RESOLUTION_WIDTH_OUT    (1920)
    #define MAX_RESOLUTION_HEIGTH_OUT   (1080)
    #define MIN_RESOLUTION_WIDTH_OUT    (4000/15+1)
    #define MIN_RESOLUTION_HEIGTH_OUT   (3000/15)
    #define MAX_POS_CLIENT              (4)
#elif defined(_NT98323_)
    #define MAX_PEOPLECNT_GROUP         (9)
    #define MAX_REAL_CAMERA             (16)
    #define MAX_RESOLUTION_WIDTH_IN     (3840)
    #define MAX_RESOLUTION_HEIGTH_IN    (2160)
    #define MAX_RESOLUTION_WIDTH_OUT    (1920)
    #define MAX_RESOLUTION_HEIGTH_OUT   (1080)
    #define MIN_RESOLUTION_WIDTH_OUT    (480)
    #define MIN_RESOLUTION_HEIGTH_OUT   (320)
    #define MAX_POS_CLIENT              (8)
#elif defined(_NT98633_)
    #define MAX_PEOPLECNT_GROUP         (9)
    #define MAX_REAL_CAMERA             (16)
    #define MAX_RESOLUTION_WIDTH_IN     (3840)
    #define MAX_RESOLUTION_HEIGTH_IN    (2160)
    #define MAX_RESOLUTION_WIDTH_OUT    (1920)
    #define MAX_RESOLUTION_HEIGTH_OUT   (1080)
    #define MIN_RESOLUTION_WIDTH_OUT    (480)
    #define MIN_RESOLUTION_HEIGTH_OUT   (320)
    #define MAX_POS_CLIENT              (8)
#else
    #error YOU MUST DEFINE CHIP_TYPE!
#endif

#define THTEAD_SCHED_RR_PRI         60
#define URL_ID_LIVE_MAIN            100
#define URL_ID_LIVE_SUB             400
#define URL_ID_PLAYBACK             800
#define MAX_STREAM_NUM              2
#define MAX_FRAME                   60
#define MAX_LEN_4                   4
#define MAX_LEN_7                   7
#define MAX_LEN_8                   8
#define MAX_LEN_9                   9
#define MAX_LEN_16                  16
#define MAX_LEN_17                  17
#define MAX_LEN_20                  20
#define MAX_LEN_24                  24
#define MAX_LEN_32                  32
#define MAX_LEN_64                  64
#define MAX_LEN_65                  65
#define MAX_LEN_70                  70
#define MAX_LEN_128                 128
#define MAX_LEN_256                 256
#define MAX_LEN_512                 512
#define MAX_LEN_1024                1024
#define MAX_LEN_2048                2048
#define MAX_LEN_4096                4096
#define MAX_LEN_8192                8192
#define MAX_LEN_10K                 10240
#define MAX_LEN_20K                 20480
#define MAX_LEN_50K                 51200
#define MAX_LEN_100K                102400
#define MAX_LEN_200K                204800
#define MAX_LEN_500K                512000
#define MAX_LEN_1M                  1024000

#define MAX_LEN_SNCODE              (MAX_LEN_65)
#define RTSP_MAX_USER               (100)       //10 -> 20 
#define MAX_USER_LEN                32
#define MAX_PWD_LEN                 32
#define MAX_PSNAME_LEN              31
#define MAX_MAIL_RECORD_CNT         (360)

#define MAX_FAILOVER                (32)

#define MAX_SMART_CTRL_CNT          (64)
#define MAX_SMART_PLAYBACK_CNT      (MAX_CAMERA)
#define MAX_RETRIEVE_MEM_SIZE       (4*1024*1024)
#define MAX_SEARCH_REC_CNT          (100)
#define ONE_MIN_SECOND              (60)
#define ONE_HOUR_SECOND             (3600)
#define TWO_HOUR_SECOND             (7200)
#define ONE_DAY_SECOND              (86400)
#define HALF_ADAY_SECOND            (43200)

#define MAX_IMAGE_DAY_NIGHT_DEFUALT 2
#define MAX_IMAGE_DAY_NIGHT_AUTO    5
#define MAX_IMAGE_DAY_NIGHT         ((MAX_IMAGE_DAY_NIGHT_DEFUALT)+(MAX_IMAGE_DAY_NIGHT_AUTO))
#define MAX_IMAGE_ROI_AREA          16 // 0-7 :pri_stream ,8 -15: sec-stream
#define MAX_IMAGE_ROI_NEW_AREA      8 // 0-7 :pri_stream ,8 -15: sec-stream
#define MAX_IMAGE_ROI_OLD_AREA      3 // 0-2 :pri_stream, 2 - 5: sec-stream


typedef enum {
    MS_BOOL_NO   = 0,
    MS_BOOL_YES  = 1,
} MS_BOOL_E;
#define MS_INVALID_CHAR             (0xFF)
#define MS_INVALID_VALUE            (0xABCD)
#define MS_INVALID_STRING           "Invalid.!"


#ifndef MAX_ANPR_LIST_COUNT
    #define MAX_ANPR_LIST_COUNT         10000
#endif
//schedule table
#define MAX_DAY_NUM_IPC         7
#define MAX_PLAN_NUM_PER_DAY_IPC    10
#define MAX_DAY_NUM             8
#define MAX_PLAN_NUM_PER_DAY    12
#define MAX_WND_NUM             4
#define MAX_HTTPS_STR           64
#define MAX_PUSH_TYPE           10
#define IPC_SCHE_THREE_SECTIONS (3)
#define IPC_SCHE_THREE_SECTIONS_STR_LEN (25)
#define IPC_SCHE_THREE_SECTIONS_SECTION_LEN  (8)
#define IPC_SCHE_THREE_SECTIONS_POINT_LEN   (4)
#define IPC_SCHE_TEN_SECTIONS   (10)
#define IPC_SCHE_TEN_SECTIONS_STR_LEN (120)
#define IPC_SCHE_TEN_SECTIONS_SECTION_LEN  (12)
#define IPC_SCHE_TEN_SECTIONS_POINT_LEN   (5)


#define MAX_EXPOSURE_NUM        140
#define MIN_EXPORT_DISK_FREE    (10)  //MB

#define PARAMS_SDK_VERSION          "2.4.22"
#define PARAMS_P2P_SDK_VERSION      "2.3.26"

#define FMT_DONE_FLAG               "reco.info"
#define MS_NO_REQS                  "no reqs"

#define MS_SH_PATH                  "/opt/app"
#define MS_ETC_PATH                 "/opt/app/etc"
#define MS_EXE_PATH                 "/opt/app/bin"
#define MS_WEB_PATH                 "/opt/app/www"
#define MS_CONF_DIR                 "/mnt/nand/app"
#define MS_ETC_DIR                  "/mnt/nand/etc"
#define MS_NAND_DIR                 "/mnt/nand"
#define MS_OEM_DIR                  "/mnt/nand2"
#define MS_RESERVE_DIR              "/mnt/nand3"
#define PATH_TEMP_FILE              "/tmp"
#define MS_CORE_PID                 "/tmp/mscore"
#define MS_CLI_PID                  "/tmp/mscli"
#define MS_GUI_PID                  "/tmp/gui"
#define MS_BOA_PID                  "/tmp/boa"
#define MS_MAX_BANDWIDTH            "/tmp/bandwidth"
#define FAILOVER_CFG_PATH           "/mnt/nand/app/"
#define MS_SNAPHOST_DIR             "/tmp/snapshot"

#define MS_RESET_FLAG_FILE          "eraseall"
#define MS_RESET_FLAG_IP_FILE       "eraseip"
#define MS_RESET_FLAG_USER_FILE     "eraseuser"
#define PARA_FILE_PATH              "/msdb.sql"
#define LOAD_PARA_FILE_PATH         "/tmp/loadpara.txt"
#define FAILOVER_SLAVE_INIT         "slaveinit"
#define FAILOVER_ANR_TIME           "anr_time"
#define MS_SAME_CHANNEL_FILE        "/mnt/nand/app/samechannel"

//#define KEEP_CONFIG_FILE          "loadpara.txt"
#define WATCHDOG_CONF_NAME          "watchdog.conf"
#define WEB_DOWNLOAD_NAME_FILE      "/tmp/.export_down_file"
#define GUI_LANG_INI                "/mnt/nand/etc/language.ini"
#define MS_KEEP_CONF_FILE           "keep_conf"
#define MS_KEEP_USER_CONF_FILE      "keep_user_conf"
#define MS_KEEP_ENCRYPTED_CONF_FILE "keep_encrypted_conf"
#define MS_KEEP_POEPWD_CONF_FILE    "/mnt/nand2/keep_params_conf"
#define MS_MAC0_CONF                "/mnt/nand/etc/mac.conf"

#define MS_DDNS_DOMAIN              "ddns.milesight.com"
#define UPDATEQUEST                 "/cgi-bin/ddns/1002"
#define IPINFOQUEST                 "/cgi-bin/ddns/1003"
#define DELETEQUEST                 "/cgi-bin/ddns/1005"
#define SERVERDEFAULTUSER           "milesight"
#define SERVERDEFAULTPWD            "milesight1612"
#define SERVERHTTPPORT              "80"
#define SERVERGETPUBLICIP           "ip1.dynupdate.no-ip.com"
#define SERVERGETPUBLICIP2          "www.ip138.com"
#define MS_DDNS_IPV4                "222.76.241.203"

#define TOOL_NAME                   "update"
#define TOOL_GLIBC                  "update_glibc"
#define MS_UPDATE_DB_FILE           "/mnt/nand/updb.cfg"

#define RECORD_TYPE_ALARM           "Alarm Input"
#define RECORD_TYPE_MOTION          "CH"

#define REMOTE_PB_DOWNLOAD          "/tmp/pbdownload"

#define IMAGE_PATH                  "/dev/shm"
#define IMAGE_NAME                  "updateFM.bin"
#define UPDATE_IMAGE_NAME           "/dev/shm/updateFM.bin"
#define UPDATE_IMAGE_NAME_IPC       "/dev/shm/updateIPCFM.bin"

//need to check webfile valid
#define LANGUAGE_JSON_FILE          "/opt/app/www/webfile/json/languages.json"
#define LANGUAGE_JSON_BAK           "/opt/app/www/webfile/json/languages.bak"
#define LANGUAGE_JSON_BAK_FILE      "/opt/app/www/webfile/json/languages_bak.json"

#define CAMERA_UPGRADE_FILE_PATH    UPDATE_IMAGE_NAME_IPC
#define LPR_NO_RECORED              "/mnt/nand/norecord.file"
#define LPR_NO_GUI                  "/mnt/nand/nogui.file"
#define LPR_NO_SEHDULE              "/mnt/nand/nosche.file"
#define DIAGNOSTIC_BAK_LOG          "/tmp/tmp_diagnostic"
#define DMESG_BAK_LOG               "/tmp/tmp_dmesg"
#define AUDIO_FILE_PATH             (MS_NAND_DIR "/audio_file")
#define FACE_EMAIL_PATH             (PATH_TEMP_FILE "/face")

#define CODECTYPE_H264              0
#define CODECTYPE_MPEG4             1
#define CODECTYPE_MJPEG             2
#define CODECTYPE_H265              3

//record frame rate
#define BITRATE_TYPE_CBR            0
#define BITRATE_TYPE_VBR            1

#define IFRAME_INTERVAL_30          0
#define IFRAME_INTERVAL_15          1
#define IFRAME_INTERVAL_10          2
#define IFRAME_INTERVAL_5           3
#define IFRAME_INTERVAL_1           4

#define FRAMERATE_30_25             0   //NTSC 30, PAL 25
#define FRAMERATE_15_13             1   //NTSC 15, PAL 13
#define FRAMERATE_08_06             2   //NTSC 8,  PAL 6
#define FRAMERATE_04_03             3   //NTSC 4,  PAL 3

#define RESOLUTION_D1               0
#define RESOLUTION_CIF              1
#define RESOLUTION_HALFD1           2
#define RESOLUTION_QCIF             3
#define RESOLUTION_720P             4


//db start
//user
#define MAX_USER                    100
//MOTION DETECT
#define MOTION_W_CELL               23
#define MOTION_H_CELL               13
#define MOTION_PAL_INC              3
#define MAX_MOTION_CELL             MOTION_W_CELL*(MOTION_H_CELL+MOTION_PAL_INC) //23*(13+3)
//email
#define EMAIL_RECEIVER_NUM          3
#define MAX_HOLIDAY                 32
#define MAX_MASK_AREA_NUM           24
#define MAX_MASK_AREA_NUM_OLD       8
#define MAX_MOSAIC_NUM              4
//HDD SATA NUM
#define SATA_MAX                    16
#define RAID_MAX                    5
#define SCSI_N04_MAX                2
#define SCSI_N03_MAX                4
#define SCSI_N01_MAX                1

//FOR MSFS DISK INFO
#define MAX_MSDK_GROUP_NUM  (16)
#define MAX_MSDK_LOCAL_NUM  (16)
#define MAX_MSDK_NET_NUM    (8)
#define MAX_MSDK_ESATA_NUM  (1)
#define MAX_MSDK_RAID       (8)
#define MAX_USB_NUM         (8)
#define MAX_MSDK_PORT_NUM   (MAX_MSDK_LOCAL_NUM+MAX_MSDK_NET_NUM+MAX_MSDK_ESATA_NUM+MAX_MSDK_RAID)
#define MAX_MSDK_NUM        ((MAX_MSDK_PORT_NUM<36)?(36):(MAX_MSDK_PORT_NUM))

//PTZ
#define PATTERN_MAX                 4
#define TOUR_MAX                    8
#define MS_KEY_MAX                  48
#define PRESET_MAX                  300

//fisheye stream
#define STREAM_MAX          5

//SPOT
#define DVR_SPOT_IN_CH              16
#define DVR_SPOT_OUT_CH             4
//AUDIO
#define MAX_AUDIOIN                 16
#define AUDIO_CODEC_G711            0
#define AUDIO_CODEC_AAC             1
#define AUDIO_CODEC_G711_MULAW      2
#define AUDIO_CODEC_G711_ALAW       3
#define AUDIO_CODEC_G722            4
#define AUDIO_CODEC_G726            5
#define MAX_AUDIO_FILE                      10
//DIO
#define MAX_ALARM_IN                16
#define ALARM_IN_TYPE_NO            0
#define ALARM_OUT_TYPE_NC           1

//IPC alarm
#define MAX_IPC_ALARM_NUM          (4)
#define MAX_IPC_ALARM_IN            4
#define MAX_IPC_ALARM_OUT           2

#define MAX_ALARM_OUT               4
#define ALARM_IN_TYPE_NO            0
#define ALARM_OUT_TYPE_NC           1

//end dbstart

#define CAMERA_TYPE_INTERNAL        0
#define CAMERA_TYPE_IPNC            1

#define PTZ_CONN_TYPE_INTERNAL      1
#define PTZ_CONN_TYPE_ONVIF         0

#define MAX_PORT_NUM    24//16

#define POE_TOTAL_POWER_4           (30.0)
#define POE_TOTAL_POWER_8           (105.0)
#define POE_TOTAL_POWER_16          (185.0)
#define POE_TOTAL_POWER_24          (185.0)

#define MAX_POE_TOTAL_POWER_4       (45.0)
#define MAX_POE_TOTAL_POWER_8       (135.0)
#define MAX_POE_TOTAL_POWER_16      (215.0)
#define MAX_POE_TOTAL_POWER_24      (215.0)
#define MAX_PRESET_CNT  			300
#define MAX_RAID_BITRATE            200

#define HDD_LOCAL           DISK_TYPE_LOCAL
#define HDD_CD              1
#define HDD_ESATA           2
#define HDD_USB             DISK_TYPE_USB
#define HDD_NET             4
#define HDD_RAID_DISK       5
#define HDD_RAID            6
#define HDD_GLOBAL_SPARE    7
#define HDD_LOCAL_SPARE     8

#define MAX_SQA_CNT         3
#define BIT1_MARK           (1ULL)
#define BIT1_LSHFT(shift)           (BIT1_MARK << (shift))

//LPR
#define UV_TCP_RECV_BUFF_SIZE (10*1024)
#define UV_TCP_KEEPALIVE_TIMES (5)
#define TCP_SEND_IMAGE_BUFF_SIZE (100*1024)

//LOG
#define LOG_MAX_TURN_SEND   100 // ((SOCKET_DATA_MAX_SIZE / LOG_DATA_SIZE) - 1)

//snapshot
#define MAX_SNAPSHOT_CNT    (100)

//Vac Dynamic box
#define MAX_VAC_DYNAMIC_BOX (90)

#define MAX_IPC_PCNT_REGION (4)
#define MAX_IPC_PCNT_LINE   (4)
#define EXCEPTION_GENERAL_INTERVAL (70)
#define MAX_EXCEPTION_INFO_NUM (512)

//ptz config clear
#define MAX_CONFIG_CLEAR    (511)

#define MS_UNUSED(x)        (void)(x)

#if defined(_HI3536_)
    #define MS_PLATFORM "3536"
#elif defined(_HI3798_)
    #define MS_PLATFORM "3798"
#elif defined(_HI3536C_)
    #define MS_PLATFORM "3536C"
#elif defined(_NT98323_) // wcm add
    #define MS_PLATFORM "98323"
#elif defined(_HI3536A_)
    #define MS_PLATFORM "3536A"
#elif defined(_NT98633_)
    #define MS_PLATFORM "98633"
#else
    #define MS_PLATFORM "3536"
#endif

typedef enum {
    NETMODE_MULTI = 0,
    NETMODE_LOADBALANCE,
    NETMODE_BACKUP,
    NETMODE_MAX,
} NETMODE;

typedef enum {
    TRANSPROTOCOL_UDP = 0,
    TRANSPROTOCOL_TCP,
    TRANSPROTOCOL_AUTO,
    TRANSPROTOCOL_ROH,
    TRANSPROTOCOL_MAX,
} TRANSPROTOCOL;

enum frm_type {
    FT_IFRAME = 0,
    FT_PFRAME,
};

enum strm_type {
    ST_NONE = 0,
    ST_VIDEO,
    ST_AUDIO,
    ST_DATA,
    MS_METADATA,
    MAX_ST,
};

enum ms_res_level {
    MS_RES_D1 = 1,
    MS_RES_720P,
    MS_RES_1080P,
    MS_RES_3M,
    MS_RES_5M,
};

enum record_event_type {
    LIB_EVENT_REC_NONE       = 0,
    LIB_EVENT_REC_CONT       = (1 << 0),
    LIB_EVENT_REC_MOT        = (1 << 1),
    LIB_EVENT_REC_IO         = (1 << 2),
    LIB_EVENT_REC_VLOSS      = (1 << 3),
    LIB_EVENT_REC_EMG        = (1 << 4),
    LIB_EVENT_REC_MOT_OR_IO  = (1 << 17),
    LIB_EVENT_REC_MOT_AND_IO = (1 << 18),
};

enum stream_source {
    SST_NONE = 0,
    SST_ENC_VIDEO       = 1 << 0,
    SST_ENC_MOTION      = 1 << 1,
    SST_ENC_SNAPSHOT    = 1 << 2,
    SST_LOCAL_PB        = 1 << 3,
    SST_REMOTE_PB       = 1 << 4,
    SST_IPC_STREAM      = 1 << 5,
    SST_DATA_STREAM     = 1 << 6,
    SST_LOCAL_AUDIO     = 1 << 7,
    SST_EVENT_STREAM    = 1 << 8,
    SST_REMOTE_TRANSFER = 1 << 9,
    SST_IPC_ANR         = 1 << 10,
    SST_REMOTE_BK_TRANSFER = 1 << 11,
};

typedef enum stream_type {
    STREAM_TYPE_MAINSTREAM = 0,
    STREAM_TYPE_SUBSTREAM,
    STREAM_TYPE_ALLSTREAM,
    STREAM_TYPE_MAX,
    STREAM_TYPE_NONE,
} STREAM_TYPE;

typedef enum {
    IPC_PROTOCOL_ONVIF = 0,
    IPC_PROTOCOL_RTSP,
    IPC_PROTOCOL_MILESIGHT,
    IPC_PROTOCOL_CANON,
    IPC_PROTOCOL_NESA,
    IPC_PROTOCOL_PSIA,
    IPC_PROTOCOL_ALPHAFINITY,
    IPC_PROTOCOL_MSDOMAIN,
    IPC_PROTOCOL_MAX = IPC_PROTOCOL_MSDOMAIN
} IPC_PROTOCOL;

typedef enum {
    IPC_CONNECT_INIT        = 0,
    IPC_CONNECT_SUCCESS     = 1,//Success
    IPC_CONNECT_NETWORK     = 2,//Network Error
    IPC_CONNECT_BANDWIDTH   = 3,//Bandwidth Limitation
    IPC_CONNECT_PASSWORD    = 4,//Incorrect Password
    IPC_CONNECT_ACCESS      = 5,//Access Limitation
    IPC_CONNECT_UNKNOW      = 6,//Unknown Error
} IPC_CONNECT_STATUS;

typedef enum {
    ESATA_BACKUP_INIT       = 0,//默认状态/备份未开启
    ESATA_BACKUP_NO_DEVICE  = 1,//无eSATA盘/eSATA盘为Storage类型
    ESATA_BACKUP_NO_FORMAT  = 2,//eSATA盘格式不是NTFS/FAT32
    ESATA_BACKUP_STANDBY    = 3,//备份开启正常
    ESATA_BACKUP_WORKING    = 4,//Working (xx%) ---- 备份进行中
} ESATA_BACKUP_STATUS;

//schedule record action
enum schedule_action {
    NONE = 0,
    TIMING_RECORD = 1,
    MOTION_RECORD = 2,
    ALARM_RECORD = 3,
    MOTION_OR_ALARM_RECORD = 4,
    MOTION_AND_ALARM_RECORD = 5,
    ALARMIN_ACTION = 6,
    ALARMOUT_ACTION = 7,
    MOTION_ACTION = 8,
    VIDEOLOSS_ACTION = 9,
    SMART_EVT_RECORD = 10, //10 VCA
    EVENT_RECORD = 11,
    ANPT_EVT_RECORD = 12, //12 ANPR
    PRI_PEOPLECNT_ACTION = 13, // private people cnt
    POS_RECORD = 14,
    REGIONAL_RECORD = 15,//IPC regional people cnt
    FACE_RECORD = 16,
    AUDIO_ALARM_RECORD = 17,
};

enum record_mode {
    MODE_NO_RECORD = 0,
    MODE_ALWAYS_REC = 1,
    MODE_SCHED_REC = 2,
};
enum event_mode {
    MODE_DISABLE = 0,
    MODE_ALWAYS,
    MODE_BY_SCHED,
};
    
typedef enum except_event {
    EXCEPT_NETWORK_DISCONN = 0,
    EXCEPT_DISK_FULL,
    EXCEPT_RECORD_FAIL,
    EXCEPT_DISK_FAIL,
    EXCEPT_DISK_NO_FORMAT,
    EXCEPT_NO_DISK = 5,
    EXCEPT_DATE_ERROR,
    EXCEPT_DISK_OKAY,
    EXCEPT_IP_CONFLICT,
    EXCEPT_DISK_OFFLINE,
    EXCEPT_DISK_HEAT,
    EXCEPT_DISK_MICROTHERM,
    EXCEPT_DISK_CONNECTION_EXCEPTION,
    EXCEPT_DISK_STRIKE,
    EXCEPT_COUNT,
} EXCEPT_EVETN_E;

enum push_event {
    PUSH_EVENT_DISK_FULL = 1,
    PUSH_EVENT_DISK_UNINIT,
    PUSH_EVENT_DISK_FAIL,
    PUSH_EVENT_DISK_DEL,
    PUSH_EVENT_DISK_OFFLINE,
    PUSH_EVENT_DISK_NORMAL,
    PUSH_EVENT_NO_DISK,
    PUSH_EVENT_HAVE_DISK,
    PUSH_EVENT_RECORD_FAIL,
    PUSH_EVENT_RECORD_START,
    PUSH_EVENT_RECORD_STOP,
    PUSH_EVENT_CAM_DISCONNECT,
    PUSH_EVENT_NET_UP,
    PUSH_EVENT_NET_DOWN,
    PUSH_EVENT_UDISK_DOWN,
    PUSH_EVENT_DATE_ERROR,
    PUSH_EVENT_DATE_NORMAL,
    PUSH_EVENT_IP_CONFLICT,
    PUSH_EVENT_IP_AVAILABLE,
    PUSH_EVENT_DISK_HEALTH_HEAT,
    PUSH_EVENT_DISK_HEALTH_MICROTHERM,
    PUSH_EVENT_DISK_HEALTH_CONNECTION_EXCEPTION,
    PUSH_EVENT_DISK_HEALTH_STRIKE,
    PUSH_EVENT_DISK_HEALTH_TEMPERATURE_NORMAL,
    PUSH_EVENT_DISK_HEALTH_STATUS_NORMAL
};

//OSD DATE FORMAT
enum osd_date_fmt {
    YYYY_MM_DD = 0,
    YYYY_DD_MM,
    MM_DD_YYYY,
    DD_MM_YYYY,
};
enum qt_date_fmt {
    OSD_DATEFORMAT_MMDDYYYY_AP = 0,
    OSD_DATEFORMAT_DDMMYYYY_AP,
    OSD_DATEFORMAT_YYYYDDMM_AP,
    OSD_DATEFORMAT_YYYYMMDD_AP,
    OSD_DATEFORMAT_MMDDYYYY,
    OSD_DATEFORMAT_DDMMYYYY,
    OSD_DATEFORMAT_YYYYDDMM,
    OSD_DATEFORMAT_YYYYMMDD,
    OSD_DATEFORMAT_MAX
};
enum others_type {
    VIDEO_LOST_TYPE = 0,
    MOTION_TYPE,
    ALARM_IN_TYPE
};

////////@david need to do  Start//////////
typedef enum smart_health_status {
    HDD_HEALTH_PASSED,
    HDD_HEALTH_FAILED
} HDD_HEALTH_STATUS;

typedef enum disk_state {
    DISK_UNFORMAT = 0, ///< unformatted
    DISK_FORMATING, ///< formatting
    DISK_FORMATTED, ///< formatted
    MAX_STATE
} DISK_STATE;
////////@david need to do End//////////


//p2p
typedef enum resp_errcode {
    ERR_OK = 0,
    ERR_UNAUTH,
    ERR_NOTFOUND,
    ERR_P2P_NETWORK,
    ERR_UNKNOWN,
    ERR_MAC_FORMAT,
    ERR_AUTH_FAILED,
    ERR_NO_UUID,
    ERR_P2P_INIT_FAILED,
    ERR_P2P_LOGIN_FAILED
} P2P_STATUS;

typedef enum _device_type {
    IPC = 0,
    NVR,
    APP
} DEVICE_TYPE;

//user
typedef enum {
    USERLEVEL_ADMIN = 0,
    USERLEVEL_OPERATOR,
    USERLEVEL_USER,
    USERLEVEL_LOCAL,
    USERLEVEL_MAX
} USER_LEVEL;


typedef enum osd_stream_type {
    OSD_ALL_STREAM = 0,
    OSD_MAIN_STREAM,
    OSD_SUB_STREAM,
    OSD_THD_STREAM
} OSD_STREAM_TYPE;

typedef enum IpcType {
    OTHER_IPC = 0,
    HISI_IPC,
    AMBA_IPC,
    TI_IPC,
    MS_SPEEDOME,
    HISI_4MPIPC,
    FISHEYE_IPC,
    MINI_PRO_BULLT_FULL,
    SIGMA_IPC,
    NT_IPC
} MS_IPC_TYPE;

typedef enum PTZTYPE {
    NOT_PTZ_IPC = 0,
    PTZ_IPC = 1,
} IPC_TYPE;

typedef enum db_type {
    DB_OLD = 0,
    DB_NEW,
} DB_TYPE;

typedef enum  ipc_conn_res {
    IPC_OK = 0,
    IPC_NETWORK_ERR,
    IPC_INVALID_USER,
    IPC_UNKNOWN_ERR,
    IPC_PROTO_NOT_SUPPORT,
    IPC_OUT_LIMIT_BANDWIDTH,
    IPC_MSDOMAIN_ERR
} IPC_CONN_RES;

typedef enum {
    PLAY_SPEED_0_03125X = -5,//1/32
    PLAY_SPEED_0_0625X = -4,//1/16
    PLAY_SPEED_0_125X = -3,//1/8
    PLAY_SPEED_0_25X = -2,//1/4
    PLAY_SPEED_0_5X = -1,//1/2
    PLAY_SPEED_1X = 0,
    PLAY_SPEED_2X,
    PLAY_SPEED_4X,
    PLAY_SPEED_8X,
    PLAY_SPEED_16X,
    PLAY_SPEED_32X,
    PLAY_SPEED_64X,
    PLAY_SPEED_128X
} PLAY_SPEED;

enum CameraType {
    RTSP_CONNECT = -3,
    DICONNECT = -2,
};

typedef enum Smart_Event {
    REGIONIN = 0,
    REGIONOUT,
    ADVANCED_MOTION,
    TAMPER,
    LINECROSS,
    LOITERING,
    HUMAN,
    PEOPLE_CNT,
    OBJECT_LEFTREMOVE,
    MAX_SMART_EVENT
} SMART_EVENT_TYPE;

//数据库表里保存了action_type, 所以这个枚举顺序不能变
typedef enum eventInType {
    ALARMIO = 0,
    MOTION,
    VIDEOLOSS,
    CAMERAIO,
    VOLUMEDETECT,
    REGION_EN = 5,
    REGION_EXIT,
    ADVANCED_MOT,
    TAMPER_DET,
    LINE_CROSS,
    LOITER,
    HUMAN_DET,
    PEOPLE_COUNT = 12,
    LEFTREMOVE = 13,
    EXCEPTION = 14,
    ANPR_BLACK_EVT = 15,
    ANPR_WHITE_EVT = 16,
    ANPR_VISTOR_EVT = 17,
    ALARM_CHN_IN0_EVT = 18,
    ALARM_CHN_IN1_EVT = 19,
    PRIVATE_PEOPLE_CNT = 20,
    POS_EVT = 21,
    REGIONAL_PEOPLE_CNT0 = 22,
    REGIONAL_PEOPLE_CNT1 = 23,
    REGIONAL_PEOPLE_CNT2 = 24,
    REGIONAL_PEOPLE_CNT3 = 25,
    FACE_EVT = 26,
    ALARM_CHN_IN2_EVT = 27,
    ALARM_CHN_IN3_EVT = 28,
    AUDIO_ALARM = 29,
    MAXEVT
} EVENT_IN_TYPE_E;

typedef enum {
    VCA_REGIONIN = 0,
    VCA_REGIONOUT,
    VCA_ADVANCED_MOTION,
    VCA_TAMPER,
    VCA_LINECROSS,
    VCA_LOITERING = 5,
    VCA_HUMAN,
    VCA_PEOPLECNT = 7,
    VCA_OBJECT_LEFT,
    VCA_OBJECT_REMOVE = 9,
    VCA_MAX,
} VCA_ALARM_EVENT;

typedef enum {
    LPR_SUPPORT_NO = -1,
    LPR_SUPPORT_INIT = 0,
    LPR_SUPPORT_VERSION,
    LPR_SUPPORT_ENABLE,
} LPR_SUPPORT_TYPE;

enum LPR_CONNECT_TYPE {
    CONNECT_LPR_INIT = 0,
    CONNECT_LPR_WAIT,
    CONNECT_LPR_OK,
    CONNECT_LPR_FAILED,
    CONNECT_LPR_PWD_ERR,
    CONNECT_LPR_NO_ALIVE
};

typedef enum ANPR_MODE {
    ANPR_ENABLE = -1,
    ANPR_BLACK = 0,
    ANPR_WHITE = 1,
    ANPR_VISTOR = 2,
    MAX_ANPR_MODE = 3,  //all type
} ANPR_MODE_TYPE;

typedef enum {
    ANPR_COLOR_NONE = 0,
    ANPR_COLOR_BLACK,
    ANPR_COLOR_BLUE,
    ANPR_COLOR_CYAN,
    ANPR_COLOR_GRAY = 4,
    ANPR_COLOR_GREEN,
    ANPR_COLOR_RED,
    ANPR_COLOR_WHITE,
    ANPR_COLOR_YELLOW = 8,
    ANPR_COLOR_VIOLET,
    ANPR_COLOR_ORANGE,
    ANPR_COLOR_ALL,  //all color
} ANPR_COLOR;

typedef enum {
    ANPR_VEHICLE_NONE = 0,
    ANPR_VEHICLE_CAR,
    ANPR_VEHICLE_MOTORCYCLE,
    ANPR_VEHICLE_BUS,
    ANPR_VEHICLE_TRUCK,
    ANPR_VEHICLE_VAN,
    ANPR_VEHICLE_SUV,
    ANPR_VEHICLE_FORKLIFT,
    ANPR_VEHICLE_EXCAVATOR,
    ANPR_VEHICLE_TOWTRUCK,
    ANPR_VEHICLE_POLICECAR,
    ANPR_VEHICLE_FIREENGINE,
    ANPR_VEHICLE_AMBULANCE,
    ANPR_VEHICLE_BICYCLE,
    ANPR_VEHICLE_EBIKE,
    ANPR_VEHICLE_OTHER,
    ANPR_VEHICLE_ALL,
} ANPR_VEHICLE;

typedef enum {
    ANPR_SPEED_NONE = 0,
    ANPR_SPEED_MORE,
    ANPR_SPEED_LESS,
    ANPR_SPEED_ALL,
} ANPR_SPEED_MODE;

typedef enum {
    ANPR_DIRECTION_NONE = 0,
    ANPR_DIRECTION_APPROACH,
    ANPR_DIRECTION_ALWAYS,
    ANPR_DIRECTION_ALL,
} ANPR_DIRECTION;

typedef enum AnprBrand {
    ANPR_BRAND_UNKNOW = 0,
    ANPR_BRAND_AUDI,
    ANPR_BRAND_ASTONMARTIN,
    ANPR_BRAND_ALFAROMEO,
    ANPR_BRAND_BUICK,
    ANPR_BRAND_MERCEDESBENZ,
    ANPR_BRAND_BMW,
    ANPR_BRAND_HONDA,
    ANPR_BRAND_PEUGEOT,
    ANPR_BRAND_PORSCHE,
    ANPR_BRAND_BENTLEY,
    ANPR_BRAND_BUGATTI,
    ANPR_BRAND_VOLKSWAGEN,
    ANPR_BRAND_DODGE,
    ANPR_BRAND_DAEWOO,
    ANPR_BRAND_DAIHATSU,
    ANPR_BRAND_TOYOTA,
    ANPR_BRAND_FORD,
    ANPR_BRAND_FERRARI,
    ANPR_BRAND_FIAT,
    ANPR_BRAND_GMC,
    ANPR_BRAND_MITSUOKA,
    ANPR_BRAND_HAVAL,
    ANPR_BRAND_GEELY,
    ANPR_BRAND_JEEP,
    ANPR_BRAND_JAGUAR,
    ANPR_BRAND_CADILLAC,
    ANPR_BRAND_CHRYSLER,
    ANPR_BRAND_LEXUS,
    ANPR_BRAND_LANDROVER,
    ANPR_BRAND_LINCOLN,
    ANPR_BRAND_SUZUKI,
    ANPR_BRAND_ROLLSROYCE,
    ANPR_BRAND_LAMBORGHINI,
    ANPR_BRAND_RENAULT,
    ANPR_BRAND_MAZDA,
    ANPR_BRAND_MINI,
    ANPR_BRAND_MASERATI,
    ANPR_BRAND_MAYBACH,
    ANPR_BRAND_ACURA,
    ANPR_BRAND_OPEL,
    ANPR_BRAND_CHERY,
    ANPR_BRAND_KIA,
    ANPR_BRAND_NISSAN,
    ANPR_BRAND_SKODA,
    ANPR_BRAND_MITSUBISHI,
    ANPR_BRAND_SUBARU,
    ANPR_BRAND_SMART,
    ANPR_BRAND_SSANGYONG,
    ANPR_BRAND_TESLA,
    ANPR_BRAND_ISUZU,
    ANPR_BRAND_CHEVROLET,
    ANPR_BRAND_CITROEN,
    ANPR_BRAND_HYUNDAI,
    ANPR_BRAND_INFINITY,
    ANPR_BRAND_MERCURY,
    ANPR_BRAND_SATURN,
    ANPR_BRAND_SAAB,
    ANPR_BRAND_LYNKCO,
    ANPR_BRAND_MORRISGARAGES,
    ANPR_BRAND_PAGANI,
    ANPR_BRAND_SPYKER,
    ANPR_BRAND_BYD,
    ANPR_BRAND_MCLAREN,
    ANPR_BRAND_KOENIGSEGG,
    ANPR_BRAND_VOLVO,
    ANPR_BRAND_LANCIA,
    ANPR_BRAND_SHELBY,
    ANPR_BRAND_SEAT,
    ANPR_BRAND_CUPRA,
    ANPR_BRAND_DACIA,
    ANPR_BRAND_DS,
    ANPR_BRAND_MAX
} ANPR_BRAND_E;

typedef enum {
    FACE_AGE_NONE = -1,
    FACE_AGE_CHILD,
    FACE_AGE_ADULT,
    FACE_AGE_ELDERLY,
    FACE_AGE_ALL,
}FACE_AGE;

typedef enum {
    FACE_GENDE_NONE = -1,
    FACE_GENDE_MALE,
    FACE_GENDE_FEMALE,
    FACE_GENDE_ALL,
}FACE_GENDE;

typedef enum {
    FACE_GLASSES_NONE = -1,
    FACE_GLASSES_NO,
    FACE_GLASSES_YES,
    FACE_GLASSES_ALL,
}FACE_GLASSES;

typedef enum {
    FACE_MASK_NONE = -1,
    FACE_MASK_NO,
    FACE_MASK_YES,
    FACE_MASK_ALL,
}FACE_MASK;

typedef enum {
    FACE_CAP_NONE = -1,
    FACE_CAP_NO,
    FACE_CAP_YES,
    FACE_CAP_ALL,
}FACE_CAP;

typedef enum {
    FACE_EXPRESSION_NONE = -1,
    FACE_EXPRESSION_HAPPY,
    FACE_EXPRESSION_SAD,
    FACE_EXPRESSION_CALM,
    FACE_EXPRESSION_ALL,
}FACE_EXPRESSION;

typedef enum CAMERA_ALARMIN_IN_MODE {
    ActionCameraAlarmInput1 = 0,
    ActionCameraAlarmInput2 = 1,
    ActionCameraAlarmInput3 = 2,
    ActionCameraAlarmInput4 = 3,
} CAMERA_ALARMIN_IN_MODE_TYPE;

typedef enum ms_upgrade_ipc_image {
    UPGRADE_INIT = 0,
    UPGRADE_TRANSFER,
    UPGRADE_ING,
    UPGRADE_SUCCESS,
    UPGRADE_FAILED,
    UPGRADE_MISMATCH,
    UPGRADE_UNKONW,
    UPGRADE_INIT_FAILED, // wcm add
    UPGRADE_WAITING, //kirin add
} UPGRADE_IPC;

// ipc update in 8.0.1
typedef enum UpgradeIpcErrno
{
	UP_IPC_UNDEFINE             = 1,
	UP_IPC_SUCCESS              = 0,	    // update success.                  "success"
	UP_IPC_FAILD			    = -1,	    // update faild. For unknow error.  "failed"
	UP_IPC_VERIFY_FAILD	        = -2,	    // verify update firmware faild.    "verify failed"
	UP_IPC_ANALYSIS_FAILD       = -3,	    // analysis update firmware faild.	"analysis failed"
	UP_IPC_UPDATE_PTB_ERR	    = -4,	    // update ptb faild                 "ptb failed"
	UP_IPC_UPDATE_BST_ERR	    = -5,	    // update bst faild                 
	UP_IPC_UPDATE_BLD_ERR	    = -6,	    // update bld faild                 "update bld failed"
	UP_IPC_UPDATE_HAL_ERR       = -7,	    // update hal faild                 "update hal failed"
	UP_IPC_UPDATE_DSP_ERR       = -8,	    // update dsp faild                 "update dsp failed"
	UP_IPC_UPDATE_KER_ERR       = -9,	    // update kernel faild              "update ker failed"
	UP_IPC_UPDATE_FS_ERR	    = -10,	    // update fs faild                  "update fs failed"
	UP_IPC_UPDATE_CMDLINE       = -11,	    // update cmdline faild             "update cmd failed"
	UP_IPC_KER1_MISMATCH	    = -12,	    // kernel 1 version mismatch        "update ker1 mismatch"
	UP_IPC_KER2_MISMATCH	    = -13,	    // kernel 2 version mismatch        "update ker2 mismatch"
	UP_IPC_DDR_MISMATCH	        = -14,	    // ddr memory mismatch              "update ddr mismatch"
	UP_IPC_VER_LOW	            = -15,	    //                                  "update fw mismatch"
	UP_IPC_DOWNGRADING_LIMIT    = -16,      //downgrading limit                 "update downgrading limit"
	UP_IPC_UMOUNT_AI_ERR	    = -17,      //umount ai partition faileds
	UP_IPC_UPDATE_DTB_ERR	    = -18,	    // update bst faild for 
	UP_IPC_OTHER			    = -19,	    //update process failed or other    "update other"
}UPGRADE_IPC_ERRNO_E;

typedef enum {
    HEATMAP_SUPPORT_NO                    = 0,
    HEATMAP_SUPPORT_YES                   = 1, //Support Space Heat Map and Time Heat Map
    HEATMAP_SUPPORT_NO_BY_DISTORT_CORRECT = 2, //矫正模式没有开，配置时不支持，搜索时只支持Time Heat Map
    HEATMAP_SUPPORT_NO_BY_DEWARPING_MODE  = 3, //鱼眼模式非1O，配置时不支持，搜索时只支持Time Heat Map
} HEATMAP_SUPPORT_E;


typedef enum {
    MD_INIT_IDEL        = 0,
    MD_INIT_UNLOADING   = 1,
    MD_INIT_LOADING     = 2,
    MD_INIT_FINISH      = 3,
} MD_INIT_E;

typedef enum IpcLogMain {
    IPC_LOG_MAIN_ALL = 0,
    IPC_LOG_MAIN_EVENT,
    IPC_LOG_MAIN_OP,
    IPC_LOG_MAIN_INFO,
    IPC_LOG_MAIN_EXCEPT,
    IPC_LOG_MAIN_SMART,
    IPC_LOG_MAIN_MAX = IPC_LOG_MAIN_SMART,
} IPC_LOG_MAIN_E;

typedef enum IpcLogSub {
    IPC_LOG_SUB_ALL = 10,

    IPC_LOG_SUB_EVENT_MIN = 11,
    IPC_LOG_SUB_EVENT_MOTION_START = IPC_LOG_SUB_EVENT_MIN,
    IPC_LOG_SUB_EVENT_MOTION_STOP,
    IPC_LOG_SUB_EVENT_ALARM_IN,
    IPC_LOG_SUB_EVENT_ALARM_OUT,
    IPC_LOG_SUB_EVENT_AUDIO_IN,
    IPC_LOG_SUB_EVENT_RADAR,
    IPC_LOG_SUB_EVENT_TAMPER, // delete
    // IPC_LOG_SUB_EVENT_IP_CONFLICT,
    IPC_LOG_SUB_EVENT_IRRECT_START, // delete
    IPC_LOG_SUB_EVENT_IRRECT_STOP, // delete
    IPC_LOG_SUB_EVENT_MAX = IPC_LOG_SUB_EVENT_AUDIO_IN,

    IPC_LOG_SUB_OP_REMOTE_MIN = 100,
    IPC_LOG_SUB_OP_REBOOT_REMOTE = IPC_LOG_SUB_OP_REMOTE_MIN,
    IPC_LOG_SUB_OP_LOGIN_REMOTE,
    IPC_LOG_SUB_OP_LOGOUT_REMOTE,
    IPC_LOG_SUB_OP_UPGRADE_REMOTE,
    IPC_LOG_SUB_OP_CONFIG_REMOTE,
    IPC_LOG_SUB_OP_PTZ_CONTROL_REMOTE,
    IPC_LOG_SUB_OP_PLAYBACK_REMOTE,
    IPC_LOG_SUB_OP_PROFILE_EXPORT_REMOTE,
    IPC_LOG_SUB_OP_PROFILE_IMPORT_REMOTE,
    IPC_LOG_SUB_OP_ADMIN_RESET_REMOTE,
    IPC_LOG_SUB_OP_RTSP_START,
    IPC_LOG_SUB_OP_RTSP_STOP,
    IPC_LOG_SUB_OP_SET_VIDEO_PARAM,
    IPC_LOG_SUB_OP_SET_IMAGE_PARAM,
    IPC_LOG_SUB_OP_RESET_VEHICLE_COUNT,
    IPC_LOG_SUB_OP_REMOTE_MAX = IPC_LOG_SUB_OP_RESET_VEHICLE_COUNT,

    IPC_LOG_SUB_EXCEPT_MIN = 200,
    IPC_LOG_SUB_EXCEPT_DISK_FULL = IPC_LOG_SUB_EXCEPT_MIN,
    IPC_LOG_SUB_EXCEPT_NETWORK_DISCONNECT,
    IPC_LOG_SUB_EVENT_IP_CONFLICT,
    IPC_LOG_SUB_EXCEPT_RECORD_FAIL,
    IPC_LOG_SUB_EXCEPT_SNAPSHOT_FAIL,
    IPC_LOG_SUB_EXCEPT_AVSE_DIE,
    IPC_LOG_SUB_EXCEPT_RTSP_DIE,
    IPC_LOG_SUB_EXCEPT_WEBS_DIE,
    // IPC_LOG_SUB_EXCEPT_SD_FULL,
    IPC_LOG_SUB_EXCEPT_SD_UNINIT,
    IPC_LOG_SUB_EXCEPT_SD_ERR,
    IPC_LOG_SUB_EXCEPT_NO_SD,
    IPC_LOG_SUB_EXCEPT_FTP_FILE,
    IPC_LOG_SUB_EXCEPT_SMTP_FILE,
    IPC_LOG_SUB_EXCEPT_MAX = IPC_LOG_SUB_EXCEPT_SMTP_FILE,

    IPC_LOG_SUB_INFO_MIN = 300,
    IPC_LOG_SUB_INFO_RECORD_START = IPC_LOG_SUB_INFO_MIN,
    IPC_LOG_SUB_INFO_RECORD_STOP,
    IPC_LOG_SUB_INFO_LOCAL_RESET,
    IPC_LOG_SUB_INFO_SYSTEM_RESTART,
    IPC_LOG_SUB_INFO_RTSP_OVER,
    IPC_LOG_SUB_INFO_RTSP_IP_LIMIT,
    IPC_LOG_SUB_INFO_IR_CUT_ON,
    IPC_LOG_SUB_INFO_IR_CUT_OFF,
    IPC_LOG_SUB_INFO_IR_CUT_LED_ON,
    IPC_LOG_SUB_INFO_IR_CUT_LED_OFF,
    IPC_LOG_SUB_INFO_WHITE_LED_ON,
    IPC_LOG_SUB_INFO_WHITE_LED_OFF,
    IPC_LOG_SUB_INFO_MAX = IPC_LOG_SUB_INFO_WHITE_LED_OFF,

    IPC_LOG_SUB_SMART_MIN = 400,
    IPC_LOG_SUB_SMART_VCA_INTRUSION_ENTER = IPC_LOG_SUB_SMART_MIN,
    IPC_LOG_SUB_SMART_VCA_INTRUSION_EXIT,
    IPC_LOG_SUB_SMART_VCA_LOITERING,
    IPC_LOG_SUB_SMART_VCA_ADVANCED_MOTION,
    IPC_LOG_SUB_SMART_VCA_OBJECT_LEFT,
    IPC_LOG_SUB_SMART_VCA_OBJECT_REMOVE,
    IPC_LOG_SUB_SMART_VCA_CROSS1,   //cross L1 A>B  delete
    IPC_LOG_SUB_SMART_VCA_CROSS2,   //cross L1 B>A  delete
    IPC_LOG_SUB_SMART_VCA_CROSS3,   //L2    delete
    IPC_LOG_SUB_SMART_VCA_CROSS4,   //L2    delete
    IPC_LOG_SUB_SMART_VCA_CROSS5,   //L3    delete
    IPC_LOG_SUB_SMART_VCA_CROSS6,   //L3    delete
    IPC_LOG_SUB_SMART_VCA_CROSS7,   //L4    delete
    IPC_LOG_SUB_SMART_VCA_CROSS8,   //L4    delete
    IPC_LOG_SUB_SMART_VCA_CTD,
    IPC_LOG_SUB_SMART_VCA_HUMAN,
    IPC_LOG_SUB_SMART_VCA_COUNTER,
    IPC_LOG_SUB_SMART_REGIONAL_PEOPLE,
    IPC_LOG_SUB_SMART_VCA_LIGHTON, // delete
    IPC_LOG_SUB_SMART_VCA_LIGHTOFF, // delete
    IPC_LOG_SUB_SMART_VCA_FACE, // delete
    IPC_LOG_SUB_SMART_VCA_LINE_CROSSING,
    IPC_LOG_SUB_SMART_MAX = IPC_LOG_SUB_SMART_VCA_LINE_CROSSING,
} IPC_LOG_SUB_E;

typedef enum FontSize {
    FONT_SIZE_SMALLEST = 0,
    FONT_SIZE_SMALL = 1,
    FONT_SIZE_MEDIUM = 2,
    FONT_SIZE_LARGE = 3,
    FONT_SIZE_LARGEST = 4,
    FONT_SIZE_AUTO = 5,
} FONT_SIZE_E;

/* 改版后的报警推送会把推送事件类型存进数据库，所以PUSH_TYPE顺序不可更改 */
typedef enum {
    PUSH_MOTION = 0,
    PUSH_VIDEOLOSS = 1,
    PUSH_REGIONIN = 2,
    PUSH_REGIONOUT = 3,
    PUSH_ADVANCED_MOTION = 4,
    PUSH_TAMPER = 5,
    PUSH_LINECROSS = 6,
    PUSH_LOITERING = 7,
    PUSH_HUMAN = 8,
    PUSH_OBJECT_LEFTREMOVE = 9,
    PUSH_IPC_ALARMIN1 = 10,
    PUSH_IPC_ALARMIN2 = 11,
    PUSH_ANPR_BLACK = 12,
    PUSH_ANPR_WHITE = 13,
    PUSH_ANPR_VISITOR = 14,
    PUSH_NVR_ALARMIN = 15,
    PUSH_NVR_POS = 16,
    PUSH_FACE = 17,
    PUSH_IPC_ALARMIN3 = 18,
    PUSH_IPC_ALARMIN4 = 19,
    PUSH_AUDIO_ALARM = 20,
    PUSH_PCNT = 21,
    PUSH_REGION_PCNT = 22,
    MAX_PUSH_TYPE_NUM,
} PUSH_TYPE;

typedef enum IpcSche {
    //record
    IPC_SCHE_RECORD_SD,//0
    IPC_SCHE_RECORD_NAS,
    //alarm
    IPC_SCHE_MOTION,
    IPC_SCHE_AUDIO_IN,
    IPC_SCHE_DI, // alarm input 1
    IPC_SCHE_DI_EXTRA, // alarm input 2
    IPC_SCHE_PIR,   //no use
    //other
    IPC_SCHE_RECORD_SNAPSHOT,
    IPC_SCHE_IR_RECT,
    IPC_SCHE_SIP_CALLOUT,   //no use
    //vca
    IPC_SCHE_VCA_INTRUSION_ENTER,//10
    IPC_SCHE_VCA_LOITERING,
    IPC_SCHE_VCA_ADVANCED_MOTION,
    IPC_SCHE_VCA_LINECROSSING,
    IPC_SCHE_PEOPLE_COUNT_LINE1,
    IPC_SCHE_VCA_HUMAN,
    IPC_SCHE_VCA_CTD,
    IPC_SCHE_VCA_INTRUSION_EXIT,
    IPC_SCHE_RADAR,
    IPC_SCHE_LPR,
    IPC_SCHE_FACE,//20
    IPC_SCHE_VCA_LINECROSSING2,
    IPC_SCHE_VCA_LINECROSSING3,
    IPC_SCHE_VCA_LINECROSSING4,
    // add by csl 20180918, object left & remove
    IPC_SCHE_VCA_OBJECT_LEFT_REMOVE,
    IPC_SCHE_LPR_BLACK,
    IPC_SCHE_LPR_WHITE,
    IPC_SCHE_LPR_VISITOR,
    IPC_SCHE_HEATMAP,
    IPC_SCHE_MOTION_SUB,            //29
    IPC_SCHE_MOTION_THIRD,
    IPC_SCHE_MOTION_FOURTH,
    IPC_SCHE_MOTION_FIFTH,
    IPC_SCHE_TRACKING_SUB,          //33
    IPC_SCHE_TRACKING_THIRD,
    IPC_SCHE_TRACKING_FOURTH,
    IPC_SCHE_TRACKING_FIFTH,
    IPC_SCHE_DI_THRID, // alarm input 3
    IPC_SCHE_DI_FOUR, // alarm input 4
    // people count rgn
    IPC_SCHE_REGIONAL_PEOPLE1,
    IPC_SCHE_REGIONAL_PEOPLE2,
    IPC_SCHE_REGIONAL_PEOPLE3,
    IPC_SCHE_REGIONAL_PEOPLE4,
    IPC_SCHE_CELLULAR,                                //43
    // people count line
    IPC_SCHE_VCA_COUNTER,
    IPC_SCHE_PEOPLE_COUNT_LINE2,
    IPC_SCHE_PEOPLE_COUNT_LINE3,
    IPC_SCHE_PEOPLE_COUNT_LINE4,
    IPC_SCHE_MAX,
}IPC_SCHE_E;


/***14版本准备废弃，begin***/
#define USERACCESS_PLAYBACK                         (1<<0)
#define USERACCESS_DISPLAY                          (1<<1)
#define USERACCESS_CAMERA                           (1<<2)
#define USERACCESS_RECORD                           (1<<3)
#define USERACCESS_ALARM                            (1<<4)
#define USERACCESS_STATUS                           (1<<5)
#define USERACCESS_BACKUP                           (1<<6)
#define USERACCESS_SYSTEM                           (1<<7)
#define USERACCESS_STORAGE                          (1<<8)

//User Privilege
#define ACCESS_CHANNEL_MANAGE   (1<<0)
#define ACCESS_PTZ_CTRL         (1<<1)
#define ACCESS_EMERGENCY_REC    (1<<2)
#define ACCESS_SNAPSHOT         (1<<3)
#define ACCESS_RECORD_CFG       (1<<4)
#define ACCESS_PLAYBACK         (1<<5)
#define ACCESS_PLAYBACKEXPORT   (1<<6)
#define ACCESS_EVENT_CFG        (1<<7)
#define ACCESS_STATE_LOG        (1<<8)
#define ACCESS_LIVEVIEW_CFG     (1<<9)
#define ACCESS_SYS_GENERAL      (1<<10)
#define ACCESS_NETWORK_CFG      (1<<11)
#define ACCESS_DISK_MANAGE      (1<<12)
#define ACCESS_HOLIDAY_CFG      (1<<13)
#define ACCESS_SYS_UPGRADE      (1<<14)
#define ACCESS_PROFILE          (1<<15)
#define ACCESS_SHUTDOWN_REBOOT  (1<<16)
#define ACCESS_SYS_MAINTENANCE  (1<<17)
#define ACCESS_SYS_AUTOREBOOT   (1<<18)
#define ACCESS_RETRIEVE         (1<<19)
#define ACCESS_SMART_ANALYSIS   (1<<20)
#define ACCESS_PTZ_SETTINGS     (1<<21)
#define ACCESS_SNAP_SETTINGS    (1<<22)
//预览&回放 audio按钮
#define ACCESS_PLAY_AUDIO       (1<<23)
//预览的对讲按钮
#define ACCESS_TWO_AWAY_AUDIO   (1<<24)
//camera audio settings
#define ACCESS_AUDIO_SETTINGS   (1<<25)
#define ACCESS_ACCESS_FILTER    (1<<26)
//setting->audio
#define ACCESS_AUDIO_FILE       (1<<27)
/***14版本准备废弃，end***/

/***用户权限相关，begin***/
typedef enum {
    PERM_LIVE_ALL = -1,
    PERM_LIVE_NONE = 0,
    PERM_LIVE_RECORD = 0x0001,
    PERM_LIVE_SNAPSHOT = 0x0002,
    PERM_LIVE_AUDIO = 0x0004,
    PERM_LIVE_TWOWAYAUDIO = 0x0008,
    PERM_LIVE_PTZCONTROL = 0x0010,
    PERM_LIVE_PTZSETTINGS = 0x0020,
    PERM_LIVE_IMAGE = 0x0040,
    PERM_LIVE_CAMERAALARMOUTPUT = 0x0080,
    PERM_LIVE_PLAYMODE = 0x0100,
    PERM_LIVE_TARGETMODEOPERATION = 0x0200
} PERM_LIVE_E;

typedef enum {
    PERM_PLAYBACK_ALL = -1,
    PERM_PLAYBACK_NONE = 0,
    PERM_PLAYBACK_SNAPSHOT = 0x0001,
    PERM_PLAYBACK_AUDIO = 0x0002,
    PERM_PLAYBACK_TAG = 0x0004,
    PERM_PLAYBACK_LOCK = 0x0008,
    PERM_PLAYBACK_FILEEXPORT = 0x0010
} PERM_PLAYBACK_E;

typedef enum {
    PERM_RETRIEVE_ALL = -1,
    PERM_RETRIEVE_NONE = 0
} PERM_RETRIEVE_E;

typedef enum {
    PERM_SMART_ALL = -1,
    PERM_SMART_NONE = 0
} PERM_SMART_E;

typedef enum {
    PERM_EVENT_ALL = -1,
    PERM_EVENT_NONE = 0
} PERM_EVENT_E;

typedef enum {
    PERM_CAMERA_ALL = -1,
    PERM_CAMERA_NONE = 0
} PERM_CAMERA_E;

typedef enum {
    PERM_STORAGE_ALL = -1,
    PERM_STORAGE_NONE = 0
} PERM_STORAGE_E;

typedef enum {
    PERM_SETTINGS_ALL = -1,
    PERM_SETTINGS_NONE = 0
} PERM_SETTINGS_E;

typedef enum {
    PERM_STATUS_ALL = -1,
    PERM_STATUS_NONE = 0
} PERM_STATUS_E;

typedef enum {
    PERM_SHUTDOWN_ALL = -1,
    PERM_SHUTDOWN_NONE = 0
} PERM_SHUTDOWN_E;

typedef enum{
    PERM_MODE_NONE = 0,
    PERM_MODE_LIVE = 1,
    PERM_MODE_PLAYBACK,
    PERM_MODE_RETRIEVE,
    PERM_MODE_SMART ,
    PERM_MODE_CAMERA,
    PERM_MODE_STORAGE,
    PERM_MODE_EVENT,
    PERM_MODE_SETTINGS,
    PERM_MODE_STATUS,
    PERM_MODE_LOGOUT
} PERM_MODE_E;
/***用户权限相关，end***/

/*
    兼容64位平台
    sizeof(struct reco_frame) 32位是88字节，64位是92字节
    有些地方(热备回传/导出/web/cms/p2p)直接将整个结构体加在私有协议数据里传输。
    长度是结构体的大小，32/64由于大小不同存在问题。(客户端一直按88字节来解析数据)
    由于该结构体仅仅是用于传输帧数据的一些参数，而结构体前面72字节已经包含所需的有效数据
    因此长度按32位平台的88字节算即可。
*/
#define RECO_FRAME_HEADER_SIZE  88


struct reco_frame {
    int ch;                 //channels
    int strm_type;          //video & audio & data  ST_VIDEO & ST_AUDIO (AUDIO_CODEC_G711)
    int codec_type;         //264&265 | aac & alaw & ulaw  //ST_DATA CUSTOM_FRAME_EVENT
    int stream_from;        //remote & local
    int stream_format;      //main & sub stream

    //Timestamp
    unsigned long long time_usec;
    unsigned int time_lower;
    unsigned int time_upper;

    //data size
    int size;

    //video
    int frame_type;        // FT_IFRAME &   FT_PFRAME
    int frame_rate;
    int width;
    int height;

    //audio
    int sample;
    int bps;
    int achn;

    //data buff
    void *data;

    int decch_changed;
    int sid;
    int sdk_alarm;
};

#define MS_CUSTOM_FRAME_HEADER_SIZE         (8)
#define MS_CUSTOM_SUBCLASS_HEADER_SIZE      (12)
#define MS_CUSTOM_SUBCLASS_START_CODE       (0x0002)

typedef enum MsCustomFrameEventId {
    CUSTOM_FRAME_NONE   = 0,
    CUSTOM_FRAME_POS    = 1,
} CUSTOM_FRAME_EVENT;

typedef enum PosProtocol {
    POS_COMMON = 0,
    POS_FORCOM = 1,
} POS_PROTOCOL_E;

struct CustomFrameHeader{
    int version;
    int reserved;
    int classNumber;
    int length;
};

struct CustomDataHeader{
    int startCode;//0x0002
    CUSTOM_FRAME_EVENT eventId;
    char typeId;
    int objectId;
    int dataLength;
};

struct PosDetails {
    struct CustomDataHeader header;
    int posId;
    int timeSec;
    int fontSize;
    int fontColor;
    int encoding;
    int overlayMode;
    int showTime;
    int overTime;
    int areaX;
    int areaY;
    int areaWidth;
    int areaHeight;
    int chnid;
    char reserved[120];
    int textLength;
    char *textData;
    int clearTime;
    POS_PROTOCOL_E protocol;
};

struct PosMetadata {
    struct CustomFrameHeader header;
    struct PosDetails *details;
};

struct PosMsfsPrivate {
    int posId;
    int sizeOffset;
    int txtOffset;
    UInt64 timeSec;
    int reservedSize;
    Uint8 receipt;
    Uint8 exceptTrunc;
    char reserved[64];
};

struct PosMsfsMatch {
    UInt64 posId;
    Uint8 receipt; // 0(common search), 1(search receipt only) 
    char content[MAX_LEN_1024];
};

typedef enum PosEventState {
    POS_EVENT_NONE = 0,
    POS_EVENT_START = 1,
    POS_EVENT_END = 2,
} POS_EVENT_STATE;

struct PosMetadataInfo {
    int id;
    int chnid;
    POS_EVENT_STATE state;
    UInt64 timeSec;
    Uint8 receipt;
    Uint8 exceptTrunc;
    Uint8 isTriRec;
    int headTime;
};

//mod_camera
struct search_param {
    int protocol;
    int reqfrom;
};


//////////////////////////////////////////////////////////////
//bruce.milesight add
enum language {
    LNG_ENGLISH = 0,    //英语
    LNG_CHINESE = 1,    //简体中文
    LNG_CHINESE_TW = 2, //繁體中文
    LNG_MAGYAR = 3,     //匈牙利语
    LNG_RUSSIAN = 4,    //俄语
    LNG_FRENCH = 5,     //法语
    LNG_POLSKA = 6,     //波兰语
    LNG_DANISH = 7,     //丹麦语
    LNG_ITALIAN = 8,    //意大利语
    LNG_GERMAN = 9,     //德语
    LNG_CZECH = 10,     //捷克语
    LNG_JAPANESE = 11,  //日语
    LNG_KOREAN = 12,    //韩语
    LNG_FINNISH = 13,   //芬兰语
    LNG_TURKEY = 14,    //土耳其语
    LNG_HOLLAND = 15,   //不知道什么语，没有这个语言文件
    LNG_HEBREW = 16,    //希伯来语
    LNG_ARABIC = 17,    //阿拉伯语
    LNG_ISRAEL = 18,    //不知道什么语，没有这个语言文件
    LGN_PERSIAN = 19,   //波斯语
    LGN_NEDERLANDS = 20,//荷兰语
    LGN_SPAIN = 21,     //西班牙
};

//gui debug start
#define MAX_NAME_LEN                                256
#define MAX_USERNAME_LEN                            64
#define MAX_PASSWORD_CHAR                           33
#define MAX_QUESTION_CHAR                           120
#define MAX_ANSWER_CHAR                             120
#define SQA_CUSTOMIZED_NO                           12


#define LAYOUTMODE_1                                (1 << 0)
#define LAYOUTMODE_4                                (1 << 1)
#define LAYOUTMODE_8                                (1 << 2)
#define LAYOUTMODE_8_1                              (1 << 3)
#define LAYOUTMODE_9                                (1 << 4)
#define LAYOUTMODE_12                               (1 << 5)
#define LAYOUTMODE_12_1                             (1 << 6)
#define LAYOUTMODE_14                               (1 << 7)
#define LAYOUTMODE_16                               (1 << 8)
#define LAYOUTMODE_25                               (1 << 9)
#define LAYOUTMODE_32                               (1 << 10)
#define LAYOUTMODE_36                               (1 << 11)
#define LAYOUTMODE_ZOOMIN                           (1 << 12)
#define LAYOUTMODE_32_2                             (1 << 13)
#define LAYOUTMODE_64                             (1 << 14)

#define LAYOUT_MODE_MAX                             (15)

#define IPC_FUNC_SEARCHABLE (1 << 0)
#define IPC_FUNC_SETPARAMS  (1 << 1)
#define IPC_FUNC_SETIPADDR  (1 << 2)

//prev or post record
#define PREVRECORDDURATION_SEC_MIN                  1
#define PREVRECORDDURATION_SEC_MAX                  30
#define PREVRECORDDURATION_SEC_DEFAULT              10

#define POSTRECORDDURATION_MINUTE_MIN               1
#define POSTRECORDDURATION_MINUTE_MAX               60
#define POSTRECORDDURATION_MINUTE_DEFALUT           3

typedef enum {
    OUTPUT_RESOLUTION_1080P = 0,
    OUTPUT_RESOLUTION_720P,
    OUTPUT_RESOLUTION_SXGA,
    OUTPUT_RESOLUTION_XGA,
    OUTPUT_RESOLUTION_1080P50,
    OUTPUT_RESOLUTION_2160P30,
    OUTPUT_RESOLUTION_2160P60,
    OUTPUT_RESOLUTION_MAX = OUTPUT_RESOLUTION_2160P60,
} DisplayDcMode_e;

typedef enum {
    DISP_PATH_HDMI0 = 0,
    DISP_PATH_HDMI1,
    DISP_PATH_VGA,
    DISP_PATH_SD,
} DisplayPath_e;

typedef enum {
    DISP_HDMI0_VGA = 0,
    DISP_MAIN_HDMI0,
} DisplayMode;

typedef enum {
    CONNECT_MANUAL = 0,
    CONNECT_UPNP,

} CONNECT_MODE;

typedef enum {
    POE_NO_CHAN = 0,
    POE_1_CHAN,
    POE_2_CHAN,
    POE_3_CHAN,
    POE_4_CHAN,
    POE_5_CHAN,
    POE_6_CHAN,
    POE_7_CHAN,
    POE_8_CHAN,
    POE_9_CHAN,
    POE_10_CHAN,
    POE_11_CHAN,
    POE_12_CHAN,
    POE_13_CHAN,
    POE_14_CHAN,
    POE_15_CHAN,
    POE_16_CHAN,
    POE_17_CHAN,
    POE_18_CHAN,
    POE_19_CHAN,
    POE_20_CHAN,
    POE_21_CHAN,
    POE_22_CHAN,
    POE_23_CHAN,
    POE_24_CHAN,
} POE_CHN_TYPE;

typedef enum {
    CPU_CTL = 0,
    MCU_CTL
}ALARM_CTL_TYPE;
//gui debug end

#define BACKUP_ERR_NONE                             0
#define BACKUP_ERR_ARGS                             1
#define BACKUP_ERR_CHANNEL                          2
#define BACKUP_ERR_PATH                             3
#define BACKUP_ERR_TIME                             4
#define BACKUP_ERR_CREATE                           5
#define BACKUP_ERR_INIT                             6
#define BACKUP_ERR_WRITE                            7
#define BACKUP_ERR_NOSPACE                          8
#define BACKUP_ERR_NOMEM                            9

//struct define
struct device_info {
    int devid;
    int max_cameras;
    int max_pb_num;
    int max_disk;
    int max_alarm_in;
    int max_alarm_out;
    int max_audio_in;
    int max_audio_out;
    int max_screen;//3536:two
    int max_hdmi;//3536:two
    int max_vga;
    int max_rs485;
    int camera_layout;//layout mode LAYOUT_NUM
    int max_lan;//network lan(eth0 eth1)
    int def_lang;//the qt and web default language.
    int oem_type; //@OEM_TYPE
    int power_key; //shutdown key
    int msddns;  //for oem whether suport ms ddns

    char sncode[MAX_LEN_SNCODE];
    char prefix[MAX_LEN_16];// 1 5 7 8 only one bit
    char model[MAX_LEN_32];//MS-N1009,MS-N8032
    char softver[MAX_LEN_32];
    char hardver[MAX_LEN_32];
    char company[MAX_LEN_128];
    char lang[MAX_LEN_32];
    char mac[MAX_LEN_32];

    int oemupdateonline;
    int oemappmsgpush;

    int isSupportTalk;
    char hostName[128];

    int alarm_ctl_mode;     // 3536a区分报警输入输出控制方式
    int mcu_online_update;  // 3536a是否支持MCU在线升级
};

//#define MAX_CHANNEL                   64//the real decorder
//#define MAX_DECODE_CHANNEL            128//the max decoder support 128
#define MAX_CAMERA_QT                   64//current QT support 32 only
#define MAX_PERFORM_LIMIT               32

//task name
#define THREAD_NAME_RECPROCESS      "recpro"
#define THREAD_NAME_RECORDPOST      "recpost"
#define THREAD_NAME_CHECKCONN       "chkconn"
#define THREAD_PULL_ALARM           "pullalarm"
#define THREAD_NAME_AUDIOPLAY       "audioplay"
#define THREAD_NAME_AUDIO_FILE_PLAY "audiofileplay"
#define THREAD_NAME_LIVE_SNAPSHOT   "livesnapshot"
#define THREAD_NAME_PB_SNAPSHOT     "pbsnapshot"
#define THREAD_NAME_EXPORT          "export"
#define THREAD_NAME_SEARCH          "search"
#define THREAD_NAME_DISK_FORMAT     "format"
#define THREAD_NAME_RAID_REBUILD    "rebuild"
#define THREAD_NAME_DISK_CHECK      "diskchk"
#define THREAD_NAME_P2P_IOCTL       "p2pioctl"
#define THREAD_NAME_P2P_INIT        "p2p"
#define THREAD_NAME_IRIS            "iris"
#define THREAD_NAME_PUSH_MSG        "pushmsg"
#define THREAD_NAME_SCHEDULE        "schedule"
#define THREAD_NAME_STREAM_SWITCH   "camswitch"
#define THREAD_NAME_POE_DISCOVERY   "poediscovery"
#define THREAD_NAME_PBDATE_SWITCH   "pbdateswitch"
#define THREAD_NAME_DISK_LOG        "disklog"
#define THREAD_NAME_ONVIF           "ovfhdlupdate"
#define THREAD_AUTO_REBOOT          "autoreboot"
#define THREAD_NAME_DDNS            "msddns"
#define THREAD_NAME_POWER_ADJUST    "poweradjust"
#define THREAD_NAME_UPNP            "upnp"

#define THREAD_NAME_DHCP6C          "dhcp6c"
#define THREAD_NAME_CREATE_RAID     "createRaid"
#define THREAD_NAME_RETRIEVE_EXPORT "retrieveExp"
#define THREAD_NAME_MSFS_BACKUP     "msfs_backup"
#define THREAD_NAME_MSFS_TIEMR      "msfs_timer"
#define THREAD_NAME_MSFS_SID        "msfs_sid"
#define THREAD_NAME_BACKUP_SNAPSHOT "backup_snapshot"
#define THREAD_NAME_BACKUP_AVI      "backup_avi"
#define THREAD_NAME_PB_ACION        "pb_action"


#define THREAD_NAME_MULTICAST       "multicast"
#define THREAD_NAME_MULTICAST_DEAL  "dMulticast"
#define THREAD_NAME_MULTICAST_EX    "multicast_ex"
#define THREAD_NAME_MULTICAST_POE   "multicast_poe"
#define THREAD_NAME_DHCP6C          "dhcp6c"
#define THREAD_NAME_STATUS_UPDATE   "status_update"

#define THREAD_NAME_SMART_EVENT     "smartEvent"
#define THREAD_NAME_NET_SPEED       "netSpeed"
#define THREAD_NAME_MSCORE_INIT     "mscoreInit"

#define THREAD_NAME_SOCK_TASK       "sockTask"
#define THREAD_NAME_ONVIF_SEARCH    "onvifSearch"
#define THREAD_NAME_MSSP_SEARCH     "msspSearch"
#define THREAD_NAME_CANON_SEARCH    "canonSearch"
#define THREAD_NAME_POE_SEARCH      "poeSearch"
#define THREAD_NAME_BASE_NOTIFY     "baseNotify"


#define THREAD_NAME_FAILOVER_SEARCH "failover_search"
#define THREAD_NAME_FAILOVER_LIBUV  "failover_libuv"
#define THREAD_NAME_FAILOVER        "failover"
#define THREAD_NAME_FAILOVER_WRITE  "hotspare_record"
#define THREAD_NAME_FAILOVER_MASTER_ANR "master_anr"
#define THREAD_NAME_FAILOVER_SLAVE_ANR "slave_anr"
#define THREAD_NAME_FAILOVER_REFRESH_LIST "refresh_master"
#define THREAD_NAME_SET_SYSTEM_TIME "ntp_time"
#define THREAD_NAME_SNAPSHOT_TASK   "snapshotTask"
#define THREAD_NAME_POST_RECORD_TASK    "postRecord"
#define THREAD_NAME_MAIN_RECORD_TASK    "recordTask"
#define THREAD_NAME_METADATE_TASK   "metadateTask"

#define THREAD_NAME_ONLINE_USER_TASK    "onlineUser"
#define THREAD_NAME_POS_TASK            "pos"

/* anr */
#define THREAD_NAME_ANR_CONNECT     "anr_connect"
#define THREAD_NAME_ANR_UV_RUN      "anr_uv_run"
#define THREAD_NAME_ANR_WRITE       "anr_record"

#define THREAD_NAME_LPR_LOOP        "lprloop"
#define THREAD_NAME_LPR_SWITCH      "lprSwitch"

#define THREAD_NAME_FACE_DISPATCH       "faceDispatch"

#define THREAD_NAME_HTTP_NOTIFICATION   "http_notification"
#define THREAD_NAME_DEAL_SDKP2P         "deal_sdkp2p"
#define THREAD_NAME_P2P_RECVFROM_CLIENT "p2p_recvfrom_client"
#define THREAD_NAME_P2P_SENDTO_CLIENT   "p2p_sendto_client"
#define THREAD_NAME_CHECK_IP_CONFLICT   "check_ip_conflict"


//default value (params)
#define DEF_MAX_CAMERA_NUM      32
#define DEF_MAX_ALARM_IN_NUM    MAX_ALARM_IN
#define DEF_MAX_ALARM_OUT_NUM   MAX_ALARM_OUT
#define DEF_LANG                "65535"
#define DEF_HTMI_HOTPLUG        1

//network device
#define MAX_DEVICE_NUM 3
#define DEVICE_NAME_BOND "bond0"
#define DEVICE_NAME_ETH0 "eth0"
#define DEVICE_NAME_ETH1 "eth1"
#define DEVICE_NAME_PPPOE "ppp0"

#define SNAPSHOT_EXPORT_UTIL        "picexport"
#define SNAPSHOT_QUALITY            50

#define POE_DEFAULT_PASSWORD        "1"

//end bruce.milesight add
//////////////////////////////////////////////////////////////
#define HTTP_HEADER_VERSION (20170301)
#define HTTP_HEADER_BOUNDARY (0x00112233)

#define MAX_FACE_CNT        (32)
#define MAX_FACE_SHIELD     (4)

#define MAX_AI_CONNECT_CNT  (16)

#define MAX_SMART_REGION                (4)
#define MAX_SMART_REGION_PRESET         (4)

struct frame_packet_header {
    int version;
    int ch;
    int strm_type;
    int codec_type;
    int stream_from;
    int stream_format;

    unsigned long long time_usec;
    unsigned int time_lower;
    unsigned int time_upper;
    int size;
    int frame_type;
    int frame_rate;
    int width;
    int height;
    //for audio
    int sample;
    int bps;
    int achn;
    //for expansion
    int expansion0;
    int expansion1;
};

//http_frame_packet_header_t move to ms_stream_common.h

struct UrlPara {
    char mac[MAX_LEN_32];
    char ip[MAX_LEN_32];
    UINT hPort;
    UINT rPort;
    DEVICE_TYPE type;
    char spwd[MAX_LEN_256];
    char softver[MAX_LEN_32];
};
struct squestion {
    int enable;
    int sqtype;
    char squestion[MAX_LEN_64 * 3];
    char answer[MAX_LEN_64 * 3];
};

typedef enum ms_active_camera_type {
    ACTIVATE = 1,
    SETSQA,
} ACTIVE_CAMERA_TYPE;

struct active_camera {
    char mac[MAX_LEN_32];
    char cChallengeKey[MAX_LEN_1024];
    int KeyStatus;
    long KeyID;
    ACTIVE_CAMERA_TYPE type;
};
struct active_camera_info {
    int camera_list;
    ACTIVE_CAMERA_TYPE type;
    char mac[MAX_LEN_64][MAX_LEN_32];
    char password[MAX_LEN_256];
    struct squestion sqas[3];
    char key[MAX_LEN_64];//for p2p active ipc
};
struct reset_type {
    int keepIp;
    int keepUser;
};
struct check_sqa_params {
    int allow_times;
    int lock_remain_time;
};
typedef struct ms_login_user_t {
    char login_user[MAX_LEN_128];
    int allow_times;
    long first_login_time;
    long last_login_time;
} ms_login_user_t;

struct schdule_item {
    char start_time[32];
    char end_time[32];
    int action_type;
};

struct schedule_day {
    struct schdule_item schedule_item[MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM];
    int wholeday_enable;
    int wholeday_action_type;
};

struct exposure_schedule_item {
    char start_time[32];
    char end_time[32];
    int action_type;
    int exposureTime;
    int gainLevel;
};

struct exposure_schedule_day {
    struct exposure_schedule_item schedule_item[MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM];
};

typedef struct smart_event_schedule {
    struct schedule_day schedule_day[MAX_DAY_NUM];
} SMART_SCHEDULE;

typedef struct cpu_percent_s {
    float   user;
    float   nice;
    float   system;
    float   idle;
    float   wait;
    float   hirq;
    float   sirq;
    float   steal;
    float   total;
    float   proc;
} CPU_PERCENT_S;

//这个是以前的，先保留
enum FISHEYE_DISPLAY_MODE {
    DISPLAY_1O = 0,
    DISPLAY_1P,
    DISPLAY_2P,
    DISPLAY_4R = 4,
    DISPLAY_1O3R,
    DISPLAY_1P3R,
    DISPLAY_MAX
};
//这个是新的，以后都用这个
typedef enum {
    FISHEYE_MULTI = 0,
    FISHEYE_BUNDLE = 1
} FISHEYE_TRANSFER_E;
typedef enum {
    FISHEYE_INSTALL_CEILING = 0,
    FISHEYE_INSTALL_WALL = 1,
    FISHEYE_INSTALL_FLAT = 2
} FISHEYE_INSTALL_MODE_E;
typedef enum {
    FISHEYE_DISPLAY_1O = 0,
    FISHEYE_DISPLAY_1P = 1,
    FISHEYE_DISPLAY_2P = 2,
    FISHEYE_DISPLAY_4R = 4,
    FISHEYE_DISPLAY_1O3R = 5,
    FISHEYE_DISPLAY_1P3R = 6,
    FISHEYE_DISPLAY_1O1P3R = 7
} FISHEYE_DISPLAY_MODE_E;

typedef enum {
    PTZ_SUPPORT_NONE        = 0x0000,
    PTZ_SUPPORT_YES         = 0x0001, //支持PTZ
    PTZ_SUPPORT_ZOOM_KEY    = 0x0002, //支持按钮ZOOM，按钮zoom和拉条zomm为互斥
    PTZ_SUPPORT_ZOOM_SLIDER = 0x0004, //支持拉条ZOOM
    PTZ_SUPPORT_PRESET      = 0x0008, //支持Preset
    PTZ_SUPPORT_PATROL      = 0x0010, //支持Patrol
    PTZ_SUPPORT_PATTERN     = 0x0020, //支持Pattern
    PTZ_SUPPORT_IRIS        = 0x0040, //支持手动IRIS
    PTZ_SUPPORT_FOCUS       = 0x0080, //支持手动Focus
    PTZ_SUPPORT_LENSINIT    = 0x0100, //支持lens initialization
    PTZ_SUPPORT_AUXFOCUS    = 0x0200, //支持auxiliary focus
    PTZ_SUPPORT_THIRDPARTY  = 0x8000, //非MS机型
    PTZ_SUPPORT_ALL = 0xffff & (~PTZ_SUPPORT_ZOOM_SLIDER)   //支持ptz所有功能，目前主要用于第三方机型兼容
} PTZ_SUPPORT_E;

enum PTZType {
    PTZ_TYPE_NONE = 0,
    PRESET,
    PATROL,
    PATTERN,
};

typedef enum {
    AUDIBLE_WARNING = 0,
    EMAIL_LINKAGE,
    EVENT_POPUP,
    PTZ_ACTION,
    ALARM_OUTPUT,
    WHITE_LED,
    HTTP_NOTIFICATION,
    ACTION_RECORD,
    ACTION_SNAPSHOT,
    ACTION_PUSHMSG,
    MAX_ACTION_EVT_TYPE,
} eventActionType;

enum {
    ANR_SUPPORT_DISCONNECT      = -4,
    ANR_SUPPORT_NOT_EXITS       = -3,
    ANR_SUPPORT_NOT_MSSP        = -2,
    ANR_SUPPORT_ERROR           = -1,
    ANR_SUPPORT_NOT_SUPPORT     = 0,
    ANR_SUPPORT_SUPPORT         = 1
};

typedef enum {
    RTSP_OVER_HTTP_INIT = 0,
    RTSP_OVER_HTTP_GET,
    RTSP_OVER_HTTP_POST
} RTSP_OVER_HTTP_REQ;

typedef enum IpcAlarmstatus {
    IPC_ALARMSTATUS_EXT                  = (1LL << 0),
    IPC_ALARMSTATUS_MOTION               = (1LL << 1),
    IPC_ALARMSTATUS_ETH                  = (1LL << 2),
    IPC_ALARMSTATUS_AUDIO                = (1LL << 3),
    IPC_ALARMSTATUS_TAMPER               = (1LL << 4),
    IPC_ALARMSTATUS_RECORD               = (1LL << 5),
    IPC_ALARMSTATUS_AVI                  = (1LL << 6),
    IPC_ALARMSTATUS_JPG                  = (1LL << 7),
    IPC_ALARMSTATUS_PIR                  = (1LL << 8),
    IPC_ALARMSTATUS_IP_CONFLICT          = (1LL << 8),
    IPC_ALARMSTATUS_IR_RECT              = (1LL << 9),
    IPC_ALARMSTATUS_TASK                 = (1LL << 10),
    IPC_ALARMSTATUS_VCA_INTRUSION_ENTER  = (1LL << 11),
    IPC_ALARMSTATUS_RADAR                = (1LL << 12),
    IPC_ALARMSTATUS_VCA_LOITERING        = (1LL << 13),
    IPC_ALARMSTATUS_VCA_ADVANCED_MOTION  = (1LL << 14),
    IPC_ALARMSTATUS_VCA_LINECROSSING1    = (1LL << 15),
    IPC_ALARMSTATUS_VCA_CTD              = (1LL << 16),
    IPC_ALARMSTATUS_VCA_INTRUSION_EXIT   = (1LL << 17),
    IPC_ALARMSTATUS_VCA_LINECROSSING2    = (1LL << 18),
    IPC_ALARMSTATUS_VCA_LINECROSSING3    = (1LL << 19),
    IPC_ALARMSTATUS_VCA_LINECROSSING4    = (1LL << 20),
    IPC_ALARMSTATUS_VCA_HUMAN            = (1LL << 21),
    IPC_ALARMSTATUS_VCA_COUNTING         = (1LL << 22),
    IPC_ALARMSTATUS_VCA_VEHICLE_COUNTING = (1LL << 23),

    IPC_ALARMSTATUS_IOT                  = (1 << 23),
    IPC_ALARMSTATUS_FACE                 = (1LL << 24),
    // add by csl 20180918, object left & remove
    IPC_ALARMSTATUS_VCA_OBJECT_LEFT      = (1LL << 25),
    IPC_ALARMSTATUS_VCA_OBJECT_REMOVE    = (1LL << 26),
    IPC_ALARMSTATUS_LPR_BLACK            = (1LL << 27),
    IPC_ALARMSTATUS_LPR_WHITE            = (1LL << 28),
    IPC_ALARMSTATUS_LPR_VISITOR          = (1LL << 29),
    IPC_ALARMSTATUS_SUB_MOTION           = (1LL << 30),
    IPC_ALARMSTATUS_THIRD_MOTION         = (1LL << 31),
    // add by lin 20200512
    IPC_ALARMSTATUS_RECORD_FAIL          = (1LL << 32),
    IPC_ALARMSTATUS_SD_FULL              = (1LL << 33),
    IPC_ALARMSTATUS_SD_UNINIT            = (1LL << 34),
    IPC_ALARMSTATUS_SD_ERROR             = (1LL << 35),
    IPC_ALARMSTATUS_NO_SD                = (1LL << 36),
    IPC_ALARMSTATUS_FOURTH_MOTION        = (1LL << 37),
    IPC_ALARMSTATUS_FIFTH_MOTION         = (1LL << 38),
    
    // 录像失败报警，区别于IPC_ALARMSTATUS_RECORD_FAIL(录像失败)，录失败报警只有在报警开启情况下，才会生成
    IPC_ALARMSTATUS_RECORD_FAIL_ALARM    = (1LL << 39),

    IPC_ALARMSTATUS_EXT_SECOND           = (1LL << 41),
    IPC_ALARMSTATUS_EXT_THIRD            = (1LL << 42),
    IPC_ALARMSTATUS_EXT_FOURTH           = (1LL << 43),
} IPC_ALARMSTATUS_E;


#define ONLINE_CHECK_URL         "cloud.milesight.com"
#define ONLINE_UPGRADE_URL       "image.milesight.com"
#define ONLINE_CHECK_UPGRADE_FMT "GET /version?\
platform=%d&\
softversion=%s&\
apptype=%d&\
AIType=%d&\
actived=%d&\
hardware=%s&\
model=%s"
#define CURL_CHECK_UPGRADE_FMT "http://cloud.milesight.com/version?\
platform=%d&\
softversion=%s&\
apptype=%d&\
AIType=%d&\
actived=%d&\
hardware=%s&\
model=%s&\
oemid=%d"

typedef enum {
    SNAPSHOT_NONE = 0,
    SNAPSHOT_CHAT,
    SNAPSHOT_MAIL,
} SNAPSHOT_FROM;

typedef enum _snapshot_state {
    SNAPSHOT_STOP = 0,
    SNAPSHOT_START,
    SNAPSHOT_READY_DATA,
} SNAPSHOT_STATE;

struct snapshot_params {
    SNAPSHOT_FROM snapfrom;
    int event;
    int chncnt;
    Uint64 chns;
    long nTime;
    void *data;
    int size;
};

typedef enum PcntCntBit {
    PCNT_CNT_BIT_IN         = 0,
    PCNT_CNT_BIT_OUT        = 1,
    PCNT_CNT_BIT_CAPACITY   = 2,
    PCNT_CNT_BIT_SUM        = 3,
    PCNT_CNT_BIT_MAX,
} PCNT_CNT_BIT_E;

typedef enum PcntCnt {
    PCNT_CNT_IN         = 1U << PCNT_CNT_BIT_IN,
    PCNT_CNT_OUT        = 1U << PCNT_CNT_BIT_OUT,
    PCNT_CNT_CAPACITY   = 1U << PCNT_CNT_BIT_CAPACITY,
    PCNT_CNT_SUM        = 1U << PCNT_CNT_BIT_SUM,
    PCNT_CNT_ALL        = ~0U,
} PCNT_CNT_E;

typedef enum IpcAudioMode {
    IPC_AUDIO_MODE_I = 0,
    IPC_AUDIO_MODE_O = 1,
    IPC_AUDIO_MODE_IO = 2,
} IPC_AUDIO_MODE_E;

typedef enum IpcBasicAlarm {
    IPC_BASIC_ALARM_MOTION          = 1,   
    IPC_BASIC_ALARM_MOTION_SUB      = 2,   
    IPC_BASIC_ALARM_MOTION_THIRD    = 3,   
    IPC_BASIC_ALARM_MOTION_FOURTH   = 4,   
    IPC_BASIC_ALARM_MOTION_FIFTH    = 5,   
    IPC_BASIC_ALARM_AUDIO_IN        = 6,   
    IPC_BASIC_ALARM_DINPUT          = 7,   
    IPC_BASIC_ALARM_DINPUT_EXTRA    = 8,   
    IPC_BASIC_ALARM_DINPUT_THIRD    = 9,   
    IPC_BASIC_ALARM_DINPUT_FOUR     = 10,   
    IPC_BASIC_ALARM_DOUTPUT         = 11,   
    IPC_BASIC_ALARM_DOUTPUT_EXTRA   = 12,   
    IPC_BASIC_ALARM_NETWORK_LOST    = 13,   
    IPC_BASIC_ALARM_IP_CONFLICT     = 14,   
    IPC_BASIC_ALARM_RECORD_FAILED   = 15,   
    IPC_BASIC_ALARM_SD_FULL         = 16,   
    IPC_BASIC_ALARM_SD_UNINIT       = 17,   
    IPC_BASIC_ALARM_SD_ERROR        = 18,   
    IPC_BASIC_ALARM_NO_SD           = 19,
    IPC_BASIC_ALARM_MAX,
} IPC_BASIC_ALARM_E;

typedef enum IpcAudioSupport {
    IPC_AUDIO_SUPPORT_MIC_IN    = 1 << 0,
    IPC_AUDIO_SUPPORT_LINE_IN   = 1 << 1,
    IPC_AUDIO_SUPPORT_AUDIO_OUT = 1 << 2, 
    IPC_AUDIO_SUPPORT_UNKNOW    = 1 << 31
} IPC_AUDIO_SUPPORT_E;

typedef enum EventRegion {
    EVENT_REGION_OFF        = 0,
    EVENT_REGION_REGIONIN,
    EVENT_REGION_REGIONOUT,
    EVENT_REGION_ADVANCED_MOTION,
    EVENT_REGION_LOITERING,
    EVENT_REGION_LINECROSS,
    EVENT_REGION_OBJECT_LEFTREMOVE,
    EVENT_REGION_PCNT,
    EVENT_REGION_REGION_PCNT,
} EVENT_REGION_E;

typedef struct WebParam {
    EVENT_REGION_E eventRegion;
} WEB_PARAM_S;

typedef enum VcaDynamicEvent {
    VCA_DYNAMIC_EVENT_NONE              = 0,
    VCA_DYNAMIC_EVENT_REGIONIN          = 0x00000001,
    VCA_DYNAMIC_EVENT_REGIONOUT         = 0x00000002,
    VCA_DYNAMIC_EVENT_ADVANCED_MOTION   = 0x00000004,
    VCA_DYNAMIC_EVENT_TAMPER            = 0x00000008,
    VCA_DYNAMIC_EVENT_LINECROSS         = 0x00000010,
    VCA_DYNAMIC_EVENT_LOITERING         = 0x00000020,
    VCA_DYNAMIC_EVENT_HUMAN             = 0x00000040,
    VCA_DYNAMIC_EVENT_PCNT              = 0x00000080,
    VCA_DYNAMIC_EVENT_OBJECT_LEFTREMOVE = 0x00000100,
    VCA_DYNAMIC_EVENT_REGION_PCNT       = 0x00000200,
} VCA_DYNAMIC_EVENT_E;

typedef struct smart_event {
    int id;
    int enable;
    unsigned int tri_channels;
    char tri_channels_ex[MAX_LEN_65];
    unsigned int tri_alarms;
    int buzzer_interval;
    int email_interval;

    char tri_chnout1_alarms[MAX_LEN_65];
    char tri_chnout2_alarms[MAX_LEN_65];

    int popup_interval;
    int ptzaction_interval;
    int alarmout_interval;
    int whiteled_interval;

    int email_pic_enable;
    char tri_channels_pic[MAX_LEN_65];
    int tri_audio_id;
    int http_notification_interval;
    Uint64 copyMask;
} SMART_EVENT;

//two-way audio
#define MSG_AUDIO_PLAY_KEY      0X20201022

#define AUDIO_BOA_TO_CORE       1
#define AUDIO_CORE_TO_BOA       2

typedef enum NetDev {
    NET_DEV_ETH0 = 0,
    NET_DEV_ETH1 = 1,
    NET_DEV_BOND = 2,
    NET_DEV_MAX
} NET_DEV_E;

static const short _mu2lin[] = {
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956, -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
        -15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412, -11900, -11388, -10876, -10364, -9852, -9340, -8828, -8316,
        -7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140, -5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
        -3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004, -2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
        -1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436, -1372, -1308, -1244, -1180, -1116, -1052, -988, -924,
        -876, -844, -812, -780, -748, -716, -684, -652, -620, -588, -556, -524, -492, -460, -428, -396,
        -372, -356, -340, -324, -308, -292, -276, -260, -244, -228, -212, -196, -180, -164, -148, -132,
        -120, -112, -104, -96, -88, -80, -72, -64, -56, -48, -40, -32, -24, -16, -8, 0,
        32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956, 23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
        15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412, 11900, 11388, 10876, 10364, 9852, 9340, 8828, 8316,
        7932, 7676, 7420, 7164, 6908, 6652, 6396, 6140, 5884, 5628, 5372, 5116, 4860, 4604, 4348, 4092,
        3900, 3772, 3644, 3516, 3388, 3260, 3132, 3004, 2876, 2748, 2620, 2492, 2364, 2236, 2108, 1980,
        1884, 1820, 1756, 1692, 1628, 1564, 1500, 1436, 1372, 1308, 1244, 1180, 1116, 1052, 988, 924,
        876, 844, 812, 780, 748, 716, 684, 652, 620, 588, 556, 524, 492, 460, 428, 396,
        372, 356, 340, 324, 308, 292, 276, 260, 244, 228, 212, 196, 180, 164, 148, 132,
        120, 112, 104, 96, 88, 80, 72, 64, 56, 48, 40, 32, 24, 16, 8, 0
    };

static const short _a2lin[] = {
    -5504, -5248, -6016, -5760, -4480, -4224, -4992, -4736, -7552, -7296, -8064, -7808, -6528, -6272, -7040, -6784,
        -2752, -2624, -3008, -2880, -2240, -2112, -2496, -2368, -3776, -3648, -4032, -3904, -3264, -3136, -3520, -3392,
        -22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944, -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136,
        -11008, -10496, -12032, -11520, -8960, -8448, -9984, -9472, -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568,
        -344, -328, -376, -360, -280, -264, -312, -296, -472, -456, -504, -488, -408, -392, -440, -424,
        -88, -72, -120, -104, -24, -8, -56, -40, -216, -200, -248, -232, -152, -136, -184, -168,
        -1376, -1312, -1504, -1440, -1120, -1056, -1248, -1184, -1888, -1824, -2016, -1952, -1632, -1568, -1760, -1696,
        -688, -656, -752, -720, -560, -528, -624, -592, -944, -912, -1008, -976, -816, -784, -880, -848,
        5504, 5248, 6016, 5760, 4480, 4224, 4992, 4736, 7552, 7296, 8064, 7808, 6528, 6272, 7040, 6784,
        2752, 2624, 3008, 2880, 2240, 2112, 2496, 2368, 3776, 3648, 4032, 3904, 3264, 3136, 3520, 3392,
        22016, 20992, 24064, 23040, 17920, 16896, 19968, 18944, 30208, 29184, 32256, 31232, 26112, 25088, 28160, 27136,
        11008, 10496, 12032, 11520, 8960, 8448, 9984, 9472, 15104, 14592, 16128, 15616, 13056, 12544, 14080, 13568,
        344, 328, 376, 360, 280, 264, 312, 296, 472, 456, 504, 488, 408, 392, 440, 424,
        88, 72, 120, 104, 24, 8, 56, 40, 216, 200, 248, 232, 152, 136, 184, 168,
        1376, 1312, 1504, 1440, 1120, 1056, 1248, 1184, 1888, 1824, 2016, 1952, 1632, 1568, 1760, 1696,
        688, 656, 752, 720, 560, 528, 624, 592, 944, 912, 1008, 976, 816, 784, 880, 848
    };


typedef enum {
    SDK_ERR = -1,
    SDK_SUC,
} SDK_RESP_STATUS;

typedef enum {
    OTHERS_ERROR    = -5,
    PARAM_ERROR     = -4,//params error.
    MEMORY_FAILED   = -3,//malloc failed.
    WAIT_TIME_OUT   = -2,//wait mscore response time out.
    SEND_FAILED     = -1,//send to mscore failed.
    REQUEST_SUCCESS = 0
} SDK_ERROR_CODE;

typedef enum TutkRegion {
    TUTK_REGION_ALL = 0,
    TUTK_REGION_AFR,
    TUTK_REGION_AME,
    TUTK_REGION_ASIA,
    TUTK_REGION_EU,
    TUTK_REGION_ME, // Middle East
    TUTK_REGION_OCEA,
    TUTK_REGION_MAX,
} TUTK_REGION_E;

typedef enum Continent {
    CONTINENT_AF = 0,
    CONTINENT_AN = 1,
    CONTINENT_AS = 2,
    CONTINENT_EU = 3,
    CONTINENT_NA = 4,
    CONTINENT_OC = 5,
    CONTINENT_SA = 6,
    CONTINENT_MAX,
} CONTINENT_E;

////////////////////////MSFS
///////////////////////
#endif//_MS_DEFS_H_
