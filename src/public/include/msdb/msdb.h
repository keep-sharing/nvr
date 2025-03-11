#ifndef __MSDB_H__
#define __MSDB_H__
#include "msstd.h"
#include "msdefs.h"
#include "msfs/msfs_disk.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MS_DB_VERSION           "1.0"
#define DB_BACKUP_NAME          "msdb.db"
#define DB_BACKUP_PASSWD        "1"

#define MAX_LEN_MAP             320
#define MAX_NUM_LEVEL           4
#define NAS_MOUNT_NUM           5
#define MAX_PEOPLECNT_LOG_LIMIT (20000)

//db file path
#define SQLITE_INIT_SQL_PATH          "/opt/app/db"
#define SQLITE_INIT_MYDB_PATH         "/opt/app/mydb"
#define SQLITE_INIT_FILE_PATH         "/opt/app/msdb.db"
#define SQLITE_INIT_ANPR_PATH         "/opt/app/anpr.db"
#define SQLITE_INIT_PCNT_PATH         "/opt/app/peoplecnt.db"
#define SQLITE_INIT_PUSHMSG_PATH      "/opt/app/pushmsg.db"
#define SQLITE_INIT_UPDATE_PATH       "/opt/app/db/db.tar.gz"
#define SQLITE_INIT_UPDATE_BAK_PATH   "/opt/app/db/db_bak.tar.gz"

//#define SQLITE_INIT_OEM_PATH  "/opt/app/oem.db"

#define SQLITE_FILE_NAME        "/mnt/nand/app/msdb.db"
#define SQLITE_OEM_NAME         "/mnt/nand2/oem.db"//oem mount block
#define SQLITE_BAK_NAME         "/mnt/nand/app/msdb_bak.db"
#define SQLITE_ANPR_NAME        "/mnt/nand/app/anpr.db"
#define SQLITE_PEOPLE_NAME      "/mnt/nand/app/peoplecnt.db"
#define SQLITE_PUSHMSG_NAME     "/mnt/nand/app/pushmsg.db"

// HTTPS SSL
#define SSL_DEFAULT_PATH        "/mnt/nand/ssl/tmp"
#define SSL_FILE_PATH           "/mnt/nand/ssl"
#define SSL_KEY_NAME            "ssl_key.pem"
#define SSL_CERT_NAME           "ssl_cert.pem"
#define SSL_REQUEST_NAME        "ssl_cert.csr"
#define SSL_CONFIG              "/etc/openssl/ssl/openssl.cnf"
#define SSL_CA_CERT             "ssl_ca_crt.pem"
#define SSL_CA_KEY              "ssl_ca_key.pem"
#define SSL_CERT_SAVE_NAME      "ssl_cert_save.pem"
#define CHECK_UPDATE_FILE       "/tmp/check_update.txt"

//param key
#define PARAM_DB_VERSION            "dbversion"
#define PARAM_MSSOCK_PATH_KEY       "mssockpath"
#define PARAM_DEBUG_LEVEL           "debug"
#define PARAM_DEBUG_ENABLE          "mscli"
#define PARAM_NOR_DI_STATUS         "dinor"
#define PARAM_NOR_DO_STATUS         "donor"
#define PARAM_ANONY                 "anony"
#define PARAM_THREAD_SWITCH_PRI     "swpri"
#define PARAM_THREAD_REC_PRI        "recpri"

#define PARAM_DEVICE_ID             "deviceid"
#define PARAM_DEVICE_NO             "prefix"
#define PARAM_DEVICE_SN             "sn_code"
#define PARAM_DEVICE_MODEL          "model"
#define PARAM_DEVICE_COMPANY        "company"
#define PARAM_DEVICE_SV             "software_version"
#define PARAM_DEVICE_HV             "hardware_version"
#define PARAM_DEVICE_DEF_LANG       "deflang"
#define PARAM_DEVICE_LANG           "lang"
#define PARAM_DEVICE_HOTPLUG        "hdmihotplug"
#define PARAM_PLUG_VER              "plugin_version"
#define PARAM_OEM_TYPE              "oem_type"
#define PARAM_MAX_DISK              "max_disk"
#define PARAM_MAX_CAM               "max_cameras"
#define PARAM_MAX_PB_CAM            "max_pb_cam"
#define PARAM_MAX_ALARM_IN          "alarm_in"
#define PARAM_MAX_ALARM_OUT         "alarm_out"
#define PARAM_MAX_AUDIO_IN          "audio_in"
#define PARAM_MAX_AUDIO_OUT         "audio_out"
#define PARAM_MAX_NETWORKS          "max_lan"
#define PARAM_MAX_SCREEN            "max_screen"
#define PARAM_MAX_HDMI              "max_hdmi"
#define PARAM_MAX_VGA               "max_vga"
#define PARAM_CAMERA_LAYOUT         "max_layout"
#define PARAM_MAX_RS485             "max_rs485"
#define PARAM_DECODE                "decode"
#define PARAM_PUSH_MSG              "enable_push_msg"
#define PARAM_FRAME_BUFFER          "frame_buff"
#define PARAM_RAID_MODE             "raidmode"
#define PARAM_POE_PSD               "poepsd"
#define PARAM_REPLACE               "msdb_replacement"
#define PARAM_POE_NUM               "max_poe_num"
#define PARAM_LAYOUT1_CHNID         "layout1_chnid"
#define PARAM_SUB_LAYOUT1_CHNID     "sub_layout1_chnid"
#define PARAM_MPLUG_VER             "mplugin_version"
#define PARAM_SDK_VERSION           "sdkversion"
#define PARAM_POWER_KEY             "power_key"
#define PARAM_MS_DDNS               "msddns"
#define PARAM_ALARM_CTL_MODE        "alarm_ctl_mode"
#define PARAM_MCU_ONLINE_UPDATE     "mcu_online_update"

#define PARAM_RECYCLE_MODE          "recycle_mode"
#define PARAM_ESATA_TYPE            "esata_type"
#define PARAM_DISK_MODE             "group_mode"
#define PARAM_MSFS_DBUP             "msfs_dbup"
#define PARAM_HTTPS_ENABLE          "httpsenable"
#define PARAM_ACCESS_ENABLE         "ch_access"
#define PARAM_PUSH_TYPE             "push_type"
#define PARAM_FAILOVER_MODE         "failover_mode"
#define PARAM_PSW_CLEAN             "psw_clean"
#define PARAM_CACHE_FRAME           "cache_frame"
#define PARAM_TRANSPORT_PROTOCOL    "transport_protocol"
#define PARAM_SAME_CHANNEL          "same_channel_exist" //0=forbid, 1=need calibration, 2=allowed
#define PARAM_STREAM_INFORMATION    "stream_information"
#define PARAM_MY_DB_VERSION         "my_dbversion"
#define PARAM_ACTION_ENABLE         "action_enable"
#define PARAM_DOUBLE_SCREAN         "double_screen"
#define PARAM_HOMOLOGOUS            "homologous"
#define PARAM_STREAM_PLAY_MODE      "streamplaymode"

#define PARAM_OEM_UPDATE_ONLINE     "oemupdateonline"
#define PARAM_OEM_APP_MSG_PUSH      "oemappmsgpush"

#define PARAM_ACCESS_FILTER_ENABLE  "access_filter_enable"
#define PARAM_ACCESS_FILTER_TYPE    "access_filter_type"
#define PARAM_QUOTA_MODE            "quota_mode"
#define PARAM_SUPPORT_TALK          "support_talk"
#define PARAM_AUTOBACKUP_DBUP       "autobackup_dbup"

#define PARAM_DEMOTION_LIMIT        "demotion_limit"


//GUI
#define PARAM_GUI_CONFIG            "layout_use_one_config"
#define PARAM_GUI_CHAN_COLOR        "channel_name_color"
#define PARAM_GUI_SKIP_BLANK        "skip_blank_page"
#define PARAM_GUI_AUTH              "local_auth_enable"
#define PARAM_GUI_MENU_AUTH         "menu_auth_enable"
#define PARAM_GUI_MENU_TIME_OUT     "menu_timeout"
#define PARAM_GUI_WIZARD_ENABLE     "wizard_enable"
#define PARAM_GUI_DISPLAY_MODE      "display_mode"
#define PARAM_GUI_ANPR_MODE         "anpr_mode"
#define PARAM_GUI_OCCUPANCY_MODE    "occupancy_mode"
#define PARAM_GUI_BOTTOMBAR_STATE   "bottombar_state"
#define PARAM_GUI_AUTO_LOGOUT       "auto_logout"

#define PARAM_PUSH_VIDEO_STREAM     "video_push_stream"
#define PARAM_QUICK_SCREEN_SWITCH   "quick_screen"

#define PARAM_PUSH_ALARMIN_TYPE     "pushmsg_alarmin_type"

#define PARAM_PERMISSION_UPDATE     "permission_update"

//table name & column name
/* camera start */
#define DB_TABLE_NAME_CAMERA            "camera"
#define DB_CAMERA_KEY_ID                "id"
#define DB_CAMERA_KEY_RECORDSTREAM      "record_stream"
#define DB_CAMERA_KEY_ANR               "anr"

/* camera end */

/* record start */
#define DB_TABLE_NAME_RECORD            "record"
#define DB_RECORD_KEY_ID                "id"
#define DB_RECORD_KEY_MODE              "mode"

/* record end*/

#define PARAM_MS_ANPR_TYPE_BLACK    "Black"
#define PARAM_MS_ANPR_TYPE_WHITE    "White"
#define PARAM_MS_ANPR_TYPE_VISITOR  "Visitor"
#define PARAM_MS_ANPR_TYPE_ALL      "All"

#define PARAM_MS_ANPR_VERSION1      (3)
#define PARAM_MS_ANPR_VERSION2      (1)
#define PARAM_MS_ANPR_VERSION3      (2)

#define PARAM_MS_ANPR_DERECTION_0   "N/A"
#define PARAM_MS_ANPR_DERECTION_1   "Approach"
#define PARAM_MS_ANPR_DERECTION_2   "Away"

//white led effective schedule
#define MOT_WLED_ESCHE              "motion_whiteled_esche"
#define VDL_WLED_ESCHE              "videoloss_whiteled_esche"
#define AIN_WLED_ESCHE              "alarmin_whiteled_esche"
#define AINCH_WLED_ESCHE            "alarm_chnIn%d_whiteled_esche"
#define AINCH0_WLED_ESCHE           "alarm_chnIn0_whiteled_esche"
#define AINCH1_WLED_ESCHE           "alarm_chnIn1_whiteled_esche"
#define AINCH2_WLED_ESCHE           "alarm_chnIn2_whiteled_esche"
#define AINCH3_WLED_ESCHE           "alarm_chnIn3_whiteled_esche"
#define VREIN_WLED_ESCHE            "vca_regionin_whiteled_esche"
#define VREEX_WLED_ESCHE            "vca_regionexit_whiteled_esche"
#define VMOT_WLED_ESCHE             "vca_motion_whiteled_esche"
#define VTEP_WLED_ESCHE             "vca_tamper_whiteled_esche"
#define VLSS_WLED_ESCHE             "vca_linecross_whiteled_esche"
#define VLSS1_WLED_ESCHE            "vca_linecross1_whiteled_esche"
#define VLSS2_WLED_ESCHE            "vca_linecross2_whiteled_esche"
#define VLSS3_WLED_ESCHE            "vca_linecross3_whiteled_esche"
#define VLER_WLED_ESCHE             "vca_loiter_whiteled_esche"
#define VHMN_WLED_ESCHE             "vca_human_whiteled_esche"
#define VPPE_WLED_ESCHE             "vca_people_whiteled_esche"
#define VOBJ_WLED_ESCHE             "vca_object_whiteled_esche"
#define LPRB_WLED_ESCHE             "lpr_black_whiteled_esche"
#define LPRW_WLED_ESCHE             "lpr_white_whiteled_esche"
#define LPRV_WLED_ESCHE             "lpr_vistor_whiteled_esche"
#define POS_WLED_ESCHE              "pos_whiteled_sche"
#define AUD_WLED_SCHE               "audio_alarm_whiteled_sche"

//white led params
#define MOT_WLED_PARAMS             "motion_white_led_params"
#define VDL_WLED_PARAMS             "video_loss_white_led_params"
#define AIN_WLED_PARAMS             "alarm_in_white_led_params"
#define AINCH_WLED_PARAMS           "alarm_chnIn%d_white_led_params"
#define AINCH0_WLED_PARAMS          "alarm_chnIn0_white_led_params"
#define AINCH1_WLED_PARAMS          "alarm_chnIn1_white_led_params"
#define AINCH2_WLED_PARAMS          "alarm_chnIn2_white_led_params"
#define AINCH3_WLED_PARAMS          "alarm_chnIn3_white_led_params"
#define VREIN_WLED_PARAMS           "vca_regionin_white_led_params"
#define VREEX_WLED_PARAMS           "vca_regionexit_white_led_params"
#define VMOT_WLED_PARAMS            "vca_motion_white_led_params"
#define VTEP_WLED_PARAMS            "vca_tamper_white_led_params"
#define VLSS_WLED_PARAMS            "vca_linecross_white_led_params"
#define VLER_WLED_PARAMS            "vca_loiter_white_led_params"
#define VHMN_WLED_PARAMS            "vca_human_white_led_params"
#define VPPE_WLED_PARAMS            "vca_people_white_led_params"
#define VOBJ_WLED_PARAMS            "vca_object_white_led_params"
#define LPRB_WLED_PARAMS            "lpr_black_white_led_params"
#define LPRW_WLED_PARAMS            "lpr_white_white_led_params"
#define LPRV_WLED_PARAMS            "lpr_vistor_white_led_params"
#define POS_WLED_PARAMS             "pos_whiteled_params"
#define REGIONAL_PCNT_WLED_PARAMS   "regional_pcnt_whiteled_params"
#define REGIONAL_PCNT1_WLED_PARAMS  "regional_pcnt1_whiteled_params"
#define REGIONAL_PCNT2_WLED_PARAMS  "regional_pcnt2_whiteled_params"
#define REGIONAL_PCNT3_WLED_PARAMS  "regional_pcnt3_whiteled_params"
#define FACE_WLED_PARAMS            "face_whiteled_params"
#define AUD_WLED_PARAMS             "audio_alarm_whiteled_params"

// http notification effective schedule
#define MOT_HTTP_SCHE               "motion_http_notification_sche"
#define VDL_HTTP_SCHE               "videoloss_http_notification_sche"
#define AIN_HTTP_SCHE               "alarmin_http_notification_sche"
#define AINCH_HTTP_SCHE             "alarm_chnIn%d_http_notification_sche"
#define AINCH0_HTTP_SCHE            "alarm_chnIn0_http_notification_sche"
#define AINCH1_HTTP_SCHE            "alarm_chnIn1_http_notification_sche"
#define AINCH2_HTTP_SCHE            "alarm_chnIn2_http_notification_sche"
#define AINCH3_HTTP_SCHE            "alarm_chnIn3_http_notification_sche"
#define VREIN_HTTP_SCHE             "vca_regionin_http_notification_sche"
#define VREEX_HTTP_SCHE             "vca_regionexit_http_notification_sche"
#define VMOT_HTTP_SCHE              "vca_motion_http_notification_sche"
#define VTEP_HTTP_SCHE              "vca_tamper_http_notification_sche"
#define VLSS_HTTP_SCHE              "vca_linecross_http_notification_sche"
#define VLSS1_HTTP_SCHE             "vca_linecross1_http_notification_sche"
#define VLSS2_HTTP_SCHE             "vca_linecross2_http_notification_sche"
#define VLSS3_HTTP_SCHE             "vca_linecross3_http_notification_sche"
#define VLER_HTTP_SCHE              "vca_loiter_http_notification_sche"
#define VHMN_HTTP_SCHE              "vca_human_http_notification_sche"
#define VPPE_HTTP_SCHE              "vca_people_http_notification_sche"
#define VOBJ_HTTP_SCHE              "vca_object_leftremove_http_notification_sche"
#define LPRB_HTTP_SCHE              "lpr_black_mode_http_notification_sche"
#define LPRW_HTTP_SCHE              "lpr_white_mode_http_notification_sche"
#define LPRV_HTTP_SCHE              "lpr_visitor_mode_http_notification_sche"
#define POS_HTTP_SCHE               "pos_http_notification_sche"
#define FACE_HTTP_SCHE              "face_http_notification_sche"
#define PCNT_HTTP_SCHE              "people_cnt_http_notification_sche"
#define AUD_HTTP_SCHE               "audio_alarm_http_notification_sche"

// http notification params
#define MOT_HTTP_PARAMS             "motion_http_notification_params"
#define VDL_HTTP_PARAMS             "videoloss_http_notification_params"
#define AIN_HTTP_PARAMS             "alarmin_http_notification_params"
#define AINCH_HTTP_PARAMS           "alarm_chnIn%d_http_notification_params"
#define AINCH0_HTTP_PARAMS          "alarm_chnIn0_http_notification_params"
#define AINCH1_HTTP_PARAMS          "alarm_chnIn1_http_notification_params"
#define AINCH2_HTTP_PARAMS          "alarm_chnIn2_http_notification_params"
#define AINCH3_HTTP_PARAMS          "alarm_chnIn3_http_notification_params"
#define VREIN_HTTP_PARAMS           "vca_regionin_http_notification_params"
#define VREEX_HTTP_PARAMS           "vca_regionexit_http_notification_params"
#define VMOT_HTTP_PARAMS            "vca_motion_http_notification_params"
#define VTEP_HTTP_PARAMS            "vca_tamper_http_notification_params"
#define VLSS_HTTP_PARAMS            "vca_linecross_http_notification_params"
#define VLER_HTTP_PARAMS            "vca_loiter_http_notification_params"
#define VHMN_HTTP_PARAMS            "vca_human_http_notification_params"
#define VPPE_HTTP_PARAMS            "vca_people_http_notification_params"
#define VOBJ_HTTP_PARAMS            "vca_object_leftremove_http_notification_params"
#define LPRB_HTTP_PARAMS            "lpr_black_mode_http_notification_params"
#define LPRW_HTTP_PARAMS            "lpr_white_mode_http_notification_params"
#define LPRV_HTTP_PARAMS            "lpr_visitor_mode_http_notification_params"
#define POS_HTTP_PARAMS             "pos_http_notification_params"
#define REGIONAL_PCNT_HTTP_PARAMS   "regional_pcnt_http_notification_params"
#define REGIONAL_PCNT1_HTTP_PARAMS  "regional_pcnt1_http_notification_params"
#define REGIONAL_PCNT2_HTTP_PARAMS  "regional_pcnt2_http_notification_params"
#define REGIONAL_PCNT3_HTTP_PARAMS  "regional_pcnt3_http_notification_params"
#define FACE_HTTP_PARAMS            "face_http_notification_params"
#define EXCEPT_HTTP_PARAMS          "except_http_notification_params"
#define PCNT_HTTP_PARAMS            "people_cnt_http_notification_params"
#define AUD_HTTP_PARAMS             "audio_alarm_http_notification_params"

#define AUD_EFFE_SCHE               "audio_alarm_effective_sche"
#define AUD_AUDI_SCHE               "audio_alarm_audible_sche"
#define AUD_EMAIL_SCHE              "audio_alarm_email_sche"
#define AUD_PTZ_SCHE                "audio_alarm_ptz_sche"
#define AUD_POP_SCHE                "audio_alarm_popup_sche"

#define MAX_ONCE_PUSH_MSG_NUM       (100)
#define MAX_MSG_NUM                 (1000)
#define WRITE_MSG_INTERVAL          (60)    //单位: 秒(s)
#define MAX_APP                     (100)
#define MAX_DB_UPDATE_FILE          (256)
/* ******************************* STRUCTURE ******************************* */
/*                                                                           */
/*                                                                           */
/* ************************************************************************* */

typedef enum {
    SCHE_EFFECTIVE  = 0,
    SCHE_AUDIO      = 1,
    SCHE_EMAIL      = 2,
    SCHE_PTZ        = 3,
    SCHE_WHITELED   = 4,
    SCHE_POPUP      = 5,
    SCHE_HTTP       = 6,
    SCHE_MAX,
} SCHEDULE_TYPE;

struct osd {
    int id;
    char name[64];
    int show_name;
    int show_date;
    int show_week;
    int date_format;
    int time_format;
    double date_pos_x;
    double date_pos_y;
    double name_pos_x;
    double name_pos_y;
    int alpha;
};

struct motion {
    int id;
    int enable;
    int sensitivity;
    unsigned int tri_channels;
    unsigned int tri_alarms;
    unsigned char motion_table[MAX_MOTION_CELL];
    int buzzer_interval;
    int ipc_sched_enable;
    char tri_channels_ex[MAX_LEN_65];
    int email_enable;
    int email_buzzer_interval;
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
};

struct motion_batch {
    int chanId[64];
    int size;
};

struct motion_schedule {
    struct schedule_day schedule_day[MAX_DAY_NUM];
};

struct video_loss {
    int id;
    int enable;
    unsigned int tri_alarms;
    int buzzer_interval;
    int email_enable;
    int email_buzzer_interval;
    char tri_chnout1_alarms[MAX_LEN_65];
    char tri_chnout2_alarms[MAX_LEN_65];
    int popup_interval;
    int ptzaction_interval;
    int alarmout_interval;
    int whiteled_interval;
    int tri_audio_id;
    int http_notification_interval;
};

struct video_loss_schedule {
    struct schedule_day schedule_day[MAX_DAY_NUM];
};

struct alarm_out {
    int id;
    char name[MAX_LEN_256];
    int type;
    int enable;
    int duration_time;
//    int buzzer_interval;
};

struct alarm_out_schedule {
    struct schedule_day schedule_day[MAX_DAY_NUM];
};

struct camera {
    int id;
    int type;
    int enable;

    int covert_on;
    int brightness;
    int contrast;
    int saturation;
    char username[64];
    char password[64];

    char ip_addr[64];

    int main_rtsp_port;
    char main_source_path[256];

    int sub_rtsp_enable;

    int sub_rtsp_port;
    char sub_source_path[256];

    int manage_port;
    int camera_protocol;
    int transmit_protocol;
    int play_stream;
    int record_stream;

    int sync_time;
    int codec;
    char main_rtspurl[MAX_LEN_256];
    char sub_rtspurl[MAX_LEN_256];

    int poe_channel;
    char mac_addr[32];
    int physical_port;
    int anr;
    char anr_start[32];
    char anr_end[32];

    //for fisheye cam
    int minorid;     /*8bit transfer | 8bit installation | 8bit display | 8bit channel id */

    //ms ddns
    char ddns[MAX_LEN_64];

    //rtsp over https
    int https_port;
};

struct record {
    int id;
    int mode;
    int input_video_signal_type;
    int codec_type;
    int resolution;
    int fps;
    int kbps;
    int iframe_interval;
    int bitrate_type;
    int audio_enable;
    int audio_codec_type;
    int audio_samplerate;
    int prev_record_on;
    int prev_record_duration;
    int post_record_on;
    int post_record_duration;
    int record_expiration_date;
    int photo_expiration_date;
};

struct record_schedule {
    struct schedule_day schedule_day[MAX_DAY_NUM];
    int enable;
};

struct holiday {
    int id;
    char name[MAX_LEN_256];
    int enable;
    int type;
    int start_year;
    int start_mon;      //1-12
    int start_mday;
    int start_mweek;    //1-n
    int start_wday;     //Sunday-Saturday(0-6)
    int end_year;
    int end_mon;
    int end_mday;
    int end_mweek;
    int end_wday;
};

struct ptz_port {
    int id;
    int baudrate;
    int data_bit;
    int stop_bit;
    int parity_type;
    int protocol;
    int address;
    int com_type;
    int connect_type;
};

typedef enum AlarmInLinkageAction {
    ALARMIN_LINKAGE_ACTION_ALARM = 0,
    ALARMIN_LINKAGE_ACTION_DISARMING
} ALARMIN_LINKAGE_ACTION_E;

struct alarm_in {
    int id;
    int enable;
    char name[MAX_LEN_256];
    int type;
    unsigned int tri_channels;
    unsigned int tri_alarms;
    int buzzer_interval;
    char tri_channels_ex[MAX_LEN_65];
    //modify for action PZT preset & patrol
    int acto_ptz_channel;
    int acto_ptz_preset_enable;
    int acto_ptz_preset;
    int acto_ptz_patrol_enable;
    int acto_ptz_patrol;
    //for email
    int email_enable;
    int email_buzzer_interval;
    char tri_channels_snapshot[MAX_LEN_65];//for snapshot
    int tri_channels_popup;

    char tri_chnout1_alarms[MAX_LEN_65];
    char tri_chnout2_alarms[MAX_LEN_65];

    int popup_interval;
    int ptzaction_interval;
    int alarmout_interval;
    int whiteled_interval;
    int http_notification_interval;

    int email_pic_enable;
    char tri_channels_pic[MAX_LEN_65];

    int event_popup_layout;//@ LAYOUTMODE_1
    int event_popup_channel[MAX_CAMERA];
    int tri_audio_id;
};

typedef struct Disarming {
    int id;
    ALARMIN_LINKAGE_ACTION_E linkageAction;
    Uint32 disarmActionMask;
    int status;
} DISARMING_S;

struct alarm_in_schedule {
    struct schedule_day schedule_day[MAX_DAY_NUM];
};

struct db_user {
    int id;
    int enable;
    char username[MAX_USER_LEN * 2];
    char password[MAX_PWD_LEN + 1];
    int type;
    char local_live_view_ex[MAX_LEN_65];
    char local_playback_ex[MAX_LEN_65];
    char remote_live_view_ex[MAX_LEN_65];
    char remote_playback_ex[MAX_LEN_65];
    char password_ex[MAX_PWD_LEN + 1];
    char pattern_psw[MAX_LEN_16];

    PERM_LIVE_E perm_local_live;
    PERM_PLAYBACK_E perm_local_playback;
    PERM_RETRIEVE_E perm_local_retrieve;
    PERM_SMART_E perm_local_smart;
    PERM_EVENT_E perm_local_event;
    PERM_CAMERA_E perm_local_camera;
    PERM_STORAGE_E perm_local_storage;
    PERM_SETTINGS_E perm_local_settings;
    PERM_STATUS_E perm_local_status;
    PERM_SHUTDOWN_E perm_local_shutdown;

    PERM_LIVE_E perm_remote_live;
    PERM_PLAYBACK_E perm_remote_playback;
    PERM_RETRIEVE_E perm_remote_retrieve;
    PERM_SMART_E perm_remote_smart;
    PERM_EVENT_E perm_remote_event;
    PERM_CAMERA_E perm_remote_camera;
    PERM_STORAGE_E perm_remote_storage;
    PERM_SETTINGS_E perm_remote_settings;
    PERM_STATUS_E perm_remote_status;
    PERM_SHUTDOWN_E perm_remote_shutdown;

    /**14版本废弃，begin**/
    int permission;
    int remote_permission;
    unsigned long local_live_view;
    unsigned long local_playback;
    unsigned long remote_live_view;
    unsigned long remote_playback;
    /**14版本废弃，end**/
};

struct db_user_oem {
    int id;
    int enable;
    char username[MAX_USER_LEN * 2];
    char password[MAX_PWD_LEN + 1];
    int type;
    int permission;
    int remote_permission;
    unsigned long local_live_view;
    unsigned long local_playback;
    unsigned long remote_live_view;
    unsigned long remote_playback;
};

struct display {
    int main_resolution;
    int main_seq_enable;
    int main_seq_interval;
    int hdmi_audio_on;
    int audio_output_on;
    int volume;
    int stereo;

    int sub_enable;
    int sub_resolution;
    int sub_seq_enable;
    int sub_seq_interval;
    int spot_resolution;
    int spot_output_channel;

    int border_line_on;
    int date_format;
    int show_channel_name;
    int camera_info;
    int start_screen;
    int page_info;
    int eventPop_screen;
    int eventPop_time;
    int occupancy_screen;
    int time_info;

    int event_region;  //0 off, 1 REGIONIN...
    FONT_SIZE_E fontSize; // valid values only 1(small)/2(medium)/3(large)
};

struct spot {
    int id;
    int seq_enable;
    int seq_interval;
    int input_channels;
};

struct time {
    int ntp_enable;
    int dst_enable;
    char time_zone[32];
    char time_zone_name[32];
    char ntp_server[64];
    int sync_enable;
    int sync_interval;//minutes
    int sync_pc;
};

struct pppoe {
    int enable;
    int auto_connect;
    char username[64];
    char password[64];
};

struct ddns {
    int enable;
    char domain[64];
    char username[32];
    char password[32];
    char host_name[128];
    char free_dns_hash[128];
    int update_freq;
    int http_port;
    int rtsp_port;
    char ddnsurl[128];
};

struct upnp {
    int enable;
    int type;
    char name[64];
    char extern_ip[64];
    int http_port;
    int http_status;
    int rtsp_port;
    int rtsp_status;
};


struct email_receiver {
    int id;
    char address[MAX_LEN_65];
    char name[MAX_LEN_65];
};

struct email {
    int enable;
    char username[MAX_LEN_65];
    char password[MAX_LEN_65];
    char smtp_server[MAX_LEN_65];
    int port;
    char sender_addr[MAX_LEN_65];
    char sender_name[MAX_LEN_65];
    int enable_tls;
    int enable_attach;
    int capture_interval;
    struct email_receiver receiver[EMAIL_RECEIVER_NUM];
};

struct network {
    int mode;
    char host_name[128];
    int miimon;
    int bond0_primary_net;
    int bond0_enable;
    int bond0_type;
    char bond0_ip_address[MAX_LEN_16];
    char bond0_netmask[MAX_LEN_16];
    char bond0_gateway[MAX_LEN_16];
    char bond0_primary_dns[MAX_LEN_16];
    char bond0_second_dns[MAX_LEN_16];
    int bond0_mtu;

    int lan1_enable;//0=disable(down),1=enable(up)
    int lan1_type;//0=manual,1=dhcp
    char lan1_ip_address[MAX_LEN_16];
    char lan1_netmask[MAX_LEN_16];
    char lan1_gateway[MAX_LEN_16];
    char lan1_primary_dns[MAX_LEN_16];
    char lan1_second_dns[MAX_LEN_16];
    int lan1_mtu;

    int lan2_enable;
    int lan2_type;
    char lan2_ip_address[MAX_LEN_16];
    char lan2_netmask[MAX_LEN_16];
    char lan2_gateway[MAX_LEN_16];
    char lan2_primary_dns[MAX_LEN_16];
    char lan2_second_dns[MAX_LEN_16];
    int lan2_mtu;
    //david.milesight
    char lan1_dhcp_gateway[MAX_LEN_16];
    char lan2_dhcp_gateway[MAX_LEN_16];
    int tri_alarms;

    char lan1_ip6_address[MAX_LEN_64];//hrz.milesight
    char lan1_ip6_netmask[MAX_LEN_16];
    char lan1_ip6_gateway[MAX_LEN_64];

    char lan2_ip6_address[MAX_LEN_64];
    char lan2_ip6_netmask[MAX_LEN_16];
    char lan2_ip6_gateway[MAX_LEN_64];
    int lan1_ip6_dhcp;//0=manual,1=Router Advertisement,2=dhcp
    int lan2_ip6_dhcp;//0=manual,1=Router Advertisement,2=dhcp

    char bond0_ip6_address[MAX_LEN_64];
    char bond0_ip6_netmask[MAX_LEN_16];
    char bond0_ip6_gateway[MAX_LEN_64];//end
    int bond0_ip6_dhcp;

    int defaultRoute;
};

struct network_more {
    int enable_ssh;
    int ssh_port;
    int http_port;
    int rtsp_port;
    int sdk_port;
    char url[256];
    int url_enable;
    int https_port;
    int posPort;    //out of use
};

struct snmp {
    int v1_enable;
    int v2c_enable;
    char write_community[MAX_LEN_64];
    char read_community[MAX_LEN_64];

    int v3_enable;
    char read_security_name[MAX_LEN_64];
    int read_level_security;//0=auth,priv 1=auth,no priv 2=no auth,no priv
    int read_auth_algorithm;//0=MD5 1=SHA
    char read_auth_password[MAX_LEN_64];
    int read_pri_algorithm;//0=DES 1=AES
    char read_pri_password[MAX_LEN_64];

    char write_security_name[MAX_LEN_64];
    int write_level_security;//0=auth,priv 1=auth,no priv 2=no auth,no priv
    int write_auth_algorithm;//0=MD5 1=SHA
    char write_auth_password[MAX_LEN_64];
    int write_pri_algorithm;//0=DES 1=AES
    char write_pri_password[MAX_LEN_64];

    int port;
};


struct audio_in {
    int id;
    int enable;
    int samplerate;
    int volume;
};

struct layout {
    int main_output;
    int sub_output;
    int main_layout_mode;
    int sub_layout_mode;
};

struct layout_custom {
    int screen;
    char name[64];
    int type;
    int page;
};

struct ptz_speed {
    int id;
    int pan;
    int tilt;
    int zoom;
    int focus;
    int timeout;
};


struct storage {
    int id;
    int recycle_mode;
};

struct ptz_key {
    int preset_id;
    int timeout;
    int speed;
};

struct ptz_tour {
    struct ptz_key keys[MS_KEY_MAX];
};

struct ipc_protocol {
    int pro_id;
    char pro_name[24];
    int function;
    int enable;
    int display_model;
};

struct ipc_model {
    int mod_id;
    int pro_id;
    char mod_name[32];
    int alarm_type;
    int default_model;
};

struct mosaic {
    int layoutmode;
    int channels[MAX_CAMERA];
};

struct mosaic_position {
    int index;
    int row;
    int column;
    int rowSpan;
    int columnSpan;
};

struct custom_mosaic {
    char name[64];
    int screen;
    int type;
    int baseRow;
    int baseColumn;
    struct mosaic_position positions[MAX_CAMERA];
    int channels[MAX_CAMERA];
};

struct disk_maintain_info {
    int log;
    int photo;
};

struct p2p_info {
    int enable;
    char model[32];
    char company[128];
    char email[128];
    char dealer[128];
    char ipc[64];
    TUTK_REGION_E region;
};

struct mask_area {
    int area_id;
    int enable;
    int start_x;
    int start_y;
    int area_width;
    int area_height;
    int width;
    int height;
    int fill_color;
    int left;
    int top;
    int right;
    int bottom;
};

struct privacy_mask {
    int id;
    struct mask_area area[MAX_MASK_AREA_NUM];
};

struct set_privacy_mask {
    int id;
    int enable;
};

struct set_privacy_mask_batch {
    int id[MAX_CAMERA];
    int enable;
    int size;
};

#define PUSH_FACE_OFFSET            (15)

/*sdk push alarm id*/
struct alarm_pid {
    char id[256];
    char sn[256];
    int enable;
    int from;
    int push_interval;
    long last_push[MAX_PUSH_TYPE_NUM];
    char name[128];
    char old_id[256];
    time_t last_time;
    int push_sum;
    char region[4];
    time_t lastNvrAlarmin[MAX_ALARM_IN];
    time_t lastIpcAlarmin[MAX_IPC_ALARM_IN][MAX_REAL_CAMERA];
    int userId;
};

typedef enum TRIGGER_ALARM_MASK {
    TRIGGER_EVENT_NOTIFICATION = 3,
    TRIGGER_AUDIBLE = 4,
    TRIGGER_EMAIL = 5,
    TRIGGER_ALARM_OUT0 = 6,
    TRIGGER_ALARM_OUT1 = 7,
    TRIGGER_ALARM_OUT2 = 8,
    TRIGGER_ALARM_OUT3 = 9,
} TRIGGER_ALARM_MASK;

// exception config
struct trigger_alarms {
    int network_disconn;  // (1<<0 disable) (1<<3 ~event notification) (1<<4 audible) (1<<5 email) (1<<6 ~1<<9 alarm output) (1<<10 ~1<<13 audio id) (1 << 14 http notification)
    int disk_full;        // (1<<0 disable) (1<<3 ~event notification) (1<<4 audible) (1<<5 email) (1<<6 ~1<<9 alarm output) (1<<10 ~1<<13 audio id) (1 << 14 http notification)
    int record_fail;      // (1<<0 disable) (1<<3 ~event notification) (1<<4 audible) (1<<5 email) (1<<6 ~1<<9 alarm output) (1<<10 ~1<<13 audio id) (1 << 14 http notification)
    int disk_fail;        // (1<<0 disable) (1<<3 ~event notification) (1<<4 audible) (1<<5 email) (1<<6 ~1<<9 alarm output) (1<<10 ~1<<13 audio id) (1 << 14 http notification)
    int disk_unformat;    // (1<<0 disable) (1<<3 ~event notification) (1<<4 audible) (1<<5 email) (1<<6 ~1<<9 alarm output) (1<<10 ~1<<13 audio id) (1 << 14 http notification)
    int no_disk;          // (1<<0 disable) (1<<3 ~event notification) (1<<4 audible) (1<<5 email) (1<<6 ~1<<9 alarm output) (1<<10 ~1<<13 audio id) (1 << 14 http notification)
    int ipConflict;
    int diskOffline;
    int diskHeat;
    int diskMicrotherm;
    int diskConnectionException;
    int diskStrike;
    int record_mail_interval;
};


struct camera_io {
    int chanid;
    int enable;
    unsigned int tri_channels;
    unsigned int tri_actions;
    char tri_channels_ex[MAX_LEN_65];
};

struct volume_detect {
    int chanid;
    int enable;
    unsigned int tri_channels;
    unsigned int tri_actions;
    char tri_channels_ex[MAX_LEN_65];
};

struct ip_conflict {
    char dev[16];
    char ipaddr[16];
};
typedef enum DAY_INTERVAL {
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY,
    EVERYDAY,
} DAY_INTERVAL;

typedef struct {
    int enable;
    DAY_INTERVAL wday;
    int hour;
    int minutes;
    int seconds;
    int login;
    char username[100];
    int reboot;
} reboot_conf;


typedef struct diskInfo {
    int disk_port;                      //disk port ID
    int enable;
    char disk_vendor[MAX_LEN_64];       //nas ip address
    char disk_address[MAX_LEN_64];      //nas ip address
    char disk_directory[MAX_LEN_128];   //nas directory
    int disk_type;                      //local, eSata, Nas @TYPE_EN
    int disk_group;                     //group ID
    int disk_property;                  // R/W
    int raid_level;                 // raid level
    char user[MAX_USER_LEN+1];
    char password[MAX_PWD_LEN+1];
} DISK_INFO_S;

struct cert_info {
    int type;
    char country[3];
    char common_name[MAX_HTTPS_STR];
    int validity;
    char password[16];
    char province[MAX_HTTPS_STR];
    char region[MAX_HTTPS_STR];
    char organization[MAX_HTTPS_STR];
    char company[MAX_HTTPS_STR];
    char email[MAX_HTTPS_STR];
    char subject[256];
    char dates[128];
    char option_company[MAX_HTTPS_STR];
    char startTime[16];
    char endTime[16];
};

enum {
    PRIVATE_CERT = 0,
    DIRECT_CERT,
    CERT_REQ,
};

struct other_poe_camera {
    int id;
    int other_poe_channel;
};

typedef struct groupInfo {
    int groupid;                        //disk port ID
    char chnMaskl[MAX_LEN_65];
    char chnMaskh[MAX_LEN_65];
} GROUP_INFO_S;

//setting->Audio
typedef struct settingAudio {
    char audioFilePath[MAX_LEN_256];
    char audioFileName[MAX_LEN_32];
}SETTING_AUDIO;

struct exposure {
    int exposureTime;
    int gainLevel;
};

struct channel_batch {
    int chanid[MAX_CAMERA];
    int size;
};

//failover mode
typedef enum {
    FAILOVER_MODE_DISABLE = 0,
    FAILOVER_MODE_MASTER,
    FAILOVER_MODE_SLAVE
} HOTSPARE_MODE_E;

//connect status
enum {
    CONNECT_INIT            = 0,    //init
    CONNECT_READY           = 1,    //Link is up Ready
    CONNECT_BUSY            = 2,    //Link is up Busy -- working for maser A or returning
    CONNECT_RETURN          = 3,    //Link is up Returning
    CONNECT_DISCONNECTED    = 4,    //Link is down connect failed
    CONNECT_NOT_AVAILABLE   = 5,    //not reply
    CONNECT_NOT_SUPPORTED   = 6,    //not support
    CONNECT_UNPAIRED        = 7,    //unpaired
    CONNECT_AUTH_FAILED     = 8,    //password error
    NO_STORAGE_SPACE        = 9,    //No Storage Space
    CONNECT_ERROR_OTHER     = 10,   //other error
    CONNECT_ERROR_OTHER_1,          //socket uv error code:UV_EOF(-104)
    CONNECT_ERROR_OTHER_2,          //socket uv error code:UV_ECONNRESET(-4095)
    CONNECT_ERROR_OTHER_3,          //socket uv error code:others
    CONNECT_ERROR_OTHER_4,          //anr break
    CONNECT_ERROR_OTHER_5,          //bandwidth
};

//master list connect status
typedef enum {
    CONNECT_STATUS_INIT        = 0,
    CONNECT_STATUS_ONLINE      = 0,
    CONNECT_STATUS_OFFLINE     = 1
} MASTER_CONN_STATUS;

//master list working status
typedef enum {
    WORKING_STATUS_INIT        = 0,
    WORKING_STATUS_NORMAL      = 0,
    WORKING_STATUS_BACKING_UP  = 1,
    WORKING_STATUS_RETURNING   = 2,
    WORKING_STATUS_SLAVE_BUSY  = 3,
    WORKING_STATUS_NO_NEED     = 4,
    WORKING_STATUS_NO_SPACE    = 5,
    WORKING_STATUS_BANDWIDTH   = 6
} MASTER_WORKING_STATUS;

enum {
    CAMERA_PASSWORD = 0,
    CAMERA_USERNAME = 1,
    CAMERA_POE_CHANNEL = 2
};

struct failover_list {
    int id;
    int enable;
    char ipaddr[MAX_LEN_64];
    char mac[MAX_LEN_32];
    char model[MAX_LEN_32];
    int port;
    char username[MAX_USER_LEN * 2];
    char password[MAX_PWD_LEN + 1];
    char start_time[MAX_LEN_64];
    char end_time[MAX_LEN_64];
    int maxCamera;
};

typedef struct ptz_action_params {
    int chn_id;
    int acto_ptz_channel;
    int acto_fish_channel;
    int acto_ptz_type;
    int acto_ptz_preset;
    int acto_ptz_patrol;
    int acto_ptz_pattern;
} PTZ_ACTION_PARAMS;

typedef struct vca_ptz_action_params {
    PTZ_ACTION_PARAMS ptzActionParams[MAX_CAMERA][MAX_CAMERA];
} VCA_PTZ_ACTION_PARAMS;

struct snapshot {
    int id;
    int stream_type;    //0=main, 1=sub
    int quality;        //0=high, 1=middle, 2=low
    int interval;
    int interval_unit;  //0=seconds,1=minutes,2=hours
    int width;
    int height;
    int expiration_date;    //0=unlimit, others=days
    int resolution;     //0=auto,1=704*576,2=640*360
};

struct snapshot_schedule {
    struct schedule_day schedule_day[MAX_DAY_NUM];
    int enable;
};

struct anpr_list {
    //char timeId[32];
    char type[16];
    char plate[64];
};

struct alarm_chn {
    int chnid;
    int alarmid;
};

struct alarm_chn_name {
    int chnid;
    int alarmid;
    char name[MAX_LEN_64];
};

struct alarm_chn_out_name {
    int chnid;
    int alarmid;
    int delay_time;
    char name[MAX_LEN_64];
};

struct alarm_out_trigger {
    int chnid;
    int alarmid;
};

struct alarm_out_trigger_chns {
    int chnid;
    int alarmid;
    char tri_chn_alarms[MAX_LEN_65];
};

struct push_msg_event {
    int chnid;
    int push_type;
};

typedef struct white_led_info {
    int chnid;
    char pDbTable[MAX_LEN_64];  //MOT_WLED_ESCHE... | MOT_WLED_PARAMS...
} WLED_INFO;

typedef struct white_led_params {
    int chnid;
    int acto_chn_id;
    int flash_mode;
    int flash_time;
} WHITE_LED_PARAMS;

typedef struct white_led_params_evts {
    WHITE_LED_PARAMS ledPrms[MAX_REAL_CAMERA][MAX_REAL_CAMERA];
} WHITE_LED_PARAMS_EVTS;


typedef struct peoplecnt_wled_params {
    int groupid;
    int acto_chn_id;
    int flash_mode;
    int flash_time;
} PEOPLECNT_WLED_PARAMS;

typedef struct pcnt_wled_params_evts {
    PEOPLECNT_WLED_PARAMS ledPrms[MAX_PEOPLECNT_GROUP][MAX_REAL_CAMERA];
} PCNT_WLED_PARAMS_EVTS;

typedef struct peoplecnt_http_params {
    int groupId;
    char url[512];
    char username[64];
    char password[64];
} PEOPLECNT_HTTP_PARAMS_S;

typedef struct peoplecnt_ptz_params {
    int groupid;
    int acto_ptz_channel;
    int acto_fish_channel;
    int acto_ptz_type;
    int acto_ptz_preset;
    int acto_ptz_patrol;
    int acto_ptz_pattern;
} PEOPLECNT_PTZ_PARAMS;

typedef struct peoplecnt_act_ptz_params {
    PEOPLECNT_PTZ_PARAMS ptzActionParams[MAX_PEOPLECNT_GROUP][MAX_REAL_CAMERA];
} PEOPLECNT_ACT_PTZ_PARAMS;

typedef struct peoplecnt_event {
    int groupid;
    unsigned int tri_alarms;
    char tri_chnout1_alarms[MAX_LEN_65];
    char tri_chnout2_alarms[MAX_LEN_65];

    int buzzer_interval;
    int email_interval;
    int ptzaction_interval;
    int alarmout_interval;
    int whiteled_interval;

    int email_pic_enable;
    char tri_channels_pic[MAX_LEN_65];
    int tri_audio_id;
    int http_notification_interval;
} PEOPLECNT_EVENT;

typedef struct peoplecnt_logs {
    int groupid;
    int chnid;
    int date;
    Int16 pcntin[MAX_LEN_24];
    Int16 pcntout[MAX_LEN_24];
} PEOPLECNT_LOGS;

typedef struct peoplecnt_log_hours {
    int groupid;
    int chnid;
    int date;
    int hour;
    Int16 pcntin;
    Int16 pcntout;
} PEOPLECNT_LOG_HOURS;

typedef enum {
    PEOPLECNT_SEARCH_BY_GROUP = 0,
    PEOPLECNT_SEARCH_BY_CAMERA = 1,
    PEOPLECNT_SEARCH_BY_REGION = 2,
    PEOPLECNT_SEARCH_TYPE_COUNT
} PEOPLECNT_SEARCH_TYPE_E;

typedef enum {
    PEOPLECNT_DAILY_REPORT,
    PEOPLECNT_WEEKLY_REPORT,
    PEOPLECNT_MONTHLY_REPORT
} PEOPLECNT_REPORT_TYPE_E;

typedef enum {
    PEOPLECNT_STATISTIC_ENTERED,
    PEOPLECNT_STATISTIC_EXITED,
    PEOPLECNT_STATISTIC_SUM
} PEOPLECNT_STATISTIC_TYPE_E;

typedef struct search_peoplecnt_logs {
    PEOPLECNT_SEARCH_TYPE_E type;
    Uint64 id;
    int pdstart;  //search by date
    int pdend;
} SEARCH_PEOPLECNT_LOGS;

typedef enum {
    PEOPLECNT_DISPLAY_NONE = 0x0,
    PEOPLECNT_DISPLAY_AVALIABLE = 0x01,
    PEOPLECNT_DISPLAY_STAY = 0x02,
    PEOPLECNT_DISPLAY_IN = 0x04,
    PEOPLECNT_DISPLAY_OUT = 0x08
} PEOPLECNT_DISPLAY_E;

typedef struct peoplecnt_setting {
    int groupid;
    int enable;
    int stays;
    int liveview_auto_reset;
    int auto_day;
    int liveview_font_size;
    char name[MAX_LEN_64];
    char auto_day_time[MAX_LEN_64];
    char tri_channels[MAX_LEN_65];
    char liveview_green_tips[MAX_LEN_128];
    char liveview_red_tips[MAX_LEN_128];
    int liveview_display;    //bitwise, PEOPLECNT_DISPLAY_E
} PEOPLECNT_SETTING;

typedef struct peoplecnt_current {
    int groupid;
    Int16 pcntin;
    Int16 pcntout;
} PEOPLECNT_CURRENT;

typedef struct people_cnt_info {
    int curdate;
    int curhour;
    PEOPLECNT_SETTING sets[MAX_PEOPLECNT_GROUP];
    PEOPLECNT_CURRENT cur[MAX_PEOPLECNT_GROUP];
    PEOPLECNT_LOGS logs[MAX_PEOPLECNT_GROUP][MAX_REAL_CAMERA];

    Uint64 updateDBChnsFlag[MAX_PEOPLECNT_GROUP];
} PEOPLECNT_INFO;

typedef enum
{
    AUTO_BACKUP_SELECT  = -1,
    AUTO_BACKUP_ESATA   = 0,
    AUTO_BACKUP_USB     = 1,
    AUTO_BACKUP_NAS     = 2,
} AUTO_BACKUP_DEVICE_E;

typedef struct esata_auto_backup {
    int enable;
    int backup_day;//0:Every, 1=Sunday, 2=Monday ... 7=Saturday
    char backup_time[MAX_LEN_32];
    int stream_type;//0=primary 1=secondary
    int file_type;//0=MP4, 1=AVI, 2=PS, 1<<2=jpg, 1<<3=cvs
    int recycle;
    char tri_channels[MAX_LEN_65];
    AUTO_BACKUP_DEVICE_E device;
    int port;
} ESATA_AUTO_BACKUP;

typedef struct PushMsg {
    Uint64 chnId;
    char alarmTime[32];
    int eventType;
    Uint64 recordChn;
    int UTCTime;
    char recordType[128];
    int port;
    MS_BOOL hasSnapshot;
    char objStr[MAX_REAL_CAMERA];
} PUSH_MSG_S;

typedef struct MsgCache {
    PUSH_MSG_S *pushMsgArr;
    int offset;
    int cnt;
    pthread_mutex_t mutex;
    time_t t;
} MSG_CACHE_S;

typedef struct HttpNotificationParams {
    int id; // chnId, alarminId, posId, exceptId
    char url[MAX_LEN_512 + 1];
    char username[96 + 1]; // utf-8 Chinese occupies 3 bytes
    char password[MAX_LEN_65];
} HTTP_NOTIFICATION_PARAMS_S;



typedef struct MsActionEffective {
    SMART_SCHEDULE sche;
    MS_BOOL_E scheIsValid;
} ACTION_EFFECTIVE_S;

typedef struct MsActionAudible {
    SMART_SCHEDULE sche;
    MS_BOOL_E scheIsValid;
    int interval;
    int id;
    int mask;
} ACTION_AUDIBLE_S;

typedef struct MsActionEmail {
    SMART_SCHEDULE sche;
    MS_BOOL_E scheIsValid;
    int interval;
    int picEnable;
    char triSnapshot[MAX_LEN_65];
} ACTION_EMAIL_S;

typedef struct MsActionPtz {
    SMART_SCHEDULE sche;
    MS_BOOL_E scheIsValid;
    int interval;
    int cnt;
    PTZ_ACTION_PARAMS params[MAX_CAMERA];
    MS_BOOL_E paramsIsValid;
} ACTION_PTZ_S;

typedef struct MsActionAlarmout {
    int interval;
    unsigned int nvrAlarm;
    char ipcAlarm[MAX_ALARM_OUT][MAX_LEN_65];
} ACTION_ALARMOUT_S;

typedef struct MsActionWhite {
    SMART_SCHEDULE sche;
    MS_BOOL_E scheIsValid;
    int interval;
    int cnt;
    WHITE_LED_PARAMS params[MAX_CAMERA];
    MS_BOOL_E paramsIsValid;
} ACTION_WHITE_S;

typedef struct MsActionHttp {
    SMART_SCHEDULE sche;
    MS_BOOL_E scheIsValid;
    int interval;
    HTTP_NOTIFICATION_PARAMS_S params;
} ACTION_HTTP_S;

typedef struct MsActionOthers {
    char triRecord[MAX_LEN_65];
    char triSnapshot[MAX_LEN_65];
} ACTION_OTHERS_S;

typedef struct MsActionStr {
    int chnid;
    ACTION_EFFECTIVE_S effective;
    ACTION_AUDIBLE_S audible;
    ACTION_EMAIL_S email;
    ACTION_PTZ_S ptz;
    ACTION_ALARMOUT_S alarmout;
    ACTION_WHITE_S white;
    ACTION_HTTP_S http;
    ACTION_OTHERS_S others;
} MS_ACTION_S;


/********************begin*****************/
//Report Auto Backup Settings
//以下枚举定义顺序不可修改
typedef enum {
    REPORT_AUTO_BACKUP_PEOPLECNT_BY_GROUP  = 0,
    REPORT_AUTO_BACKUP_PEOPLECNT_BY_CAMERA = 1,
    REPORT_AUTO_BACKUP_PEOPLECNT_BY_REGION = 2,
    REPORT_AUTO_BACKUP_HEATMAP             = 3,
    MAX_REPORT_AUTO_BACKUP_TYPE_COUNT
} REPORT_AUTO_BACKUP_TYPE_E;
typedef enum {
    REPORT_AUTO_BACKUP_STAY_LENGTH_ALL      = 0,
    REPORT_AUTO_BACKUP_STAY_LENGTH_MORETHAN = 1,
    REPORT_AUTO_BACKUP_STAY_LENGTH_LESSTHAN = 2
} REPORT_AUTO_BACKUP_STAY_LENGTH_E;
typedef enum {
    REPORT_AUTO_BACKUP_DAY_EVERYDAY  = 0,
    REPORT_AUTO_BACKUP_DAY_SUNDAY    = 1,
    REPORT_AUTO_BACKUP_DAY_MONDAY    = 2,
    REPORT_AUTO_BACKUP_DAY_TUESDAY   = 3,
    REPORT_AUTO_BACKUP_DAY_WEDNESDAY = 4,
    REPORT_AUTO_BACKUP_DAY_THURSDAY  = 5,
    REPORT_AUTO_BACKUP_DAY_FRIDAY    = 6,
    REPORT_AUTO_BACKUP_DAY_SATURDAY  = 7
} REPORT_AUTO_BACKUP_DAY_E;
typedef enum {
    REPORT_AUTO_BACKUP_TIME_RANGE_ALL             = 0,
    REPORT_AUTO_BACKUP_TIME_RANGE_LAST_DAY        = 1,
    REPORT_AUTO_BACKUP_TIME_RANGE_LAST_WEEK       = 2,
    REPORT_AUTO_BACKUP_TIME_RANGE_LAST_HOUR       = 3,
    REPORT_AUTO_BACKUP_TIME_RANGE_INTERVAL_1_HOUR = 4,
} REPORT_AUTO_BACKUP_TIME_RANGE_E;
typedef enum {
    REPORT_AUTO_BACKUP_FORMAT_NONE          = 0x0,
    REPORT_AUTO_BACKUP_FORMAT_CSV           = 0x0001,
    REPORT_AUTO_BACKUP_FORMAT_HEATMAP_SPACE = 0x0002,
    REPORT_AUTO_BACKUP_FORMAT_HEATMAP_TIME  = 0x0004
} REPORT_AUTO_BACKUP_FORMAT_E;
typedef enum {
    REPORT_AUTO_BACKUP_TO_NONE   = 0x0,
    REPORT_AUTO_BACKUP_TO_DEVICE = 0x0001,
    REPORT_AUTO_BACKUP_TO_EMAIL  = 0x0002
} REPORT_AUTO_BACKUP_TO_E;
typedef struct {
    int reportType;               // REPORT_AUTO_BACKUP_TYPE_E
    int enable;                   //
    char triChannels[MAX_LEN_65]; //
    char triGroups[MAX_LEN_17];   //
    int stayLengthType;           // REPORT_AUTO_BACKUP_STAY_LENGTH_E, 0: All, 1: More than, 2: Less than
    int stayLengthValue;          //
    int backupDay;                // REPORT_AUTO_BACKUP_DAY_E, 0: Every, 1: Sunday, 2: Monday ... 7: Saturday
    char backupTime[MAX_LEN_32];  //
    int fileFormat;               // REPORT_AUTO_BACKUP_FORMAT_E, bit0: csv, bit1: space heat map, bit2: time heat map
    int timeRange;                // REPORT_AUTO_BACKUP_TIME_RANGE_E, 0: Export All, 1: Last Day, 2: Last Week, 3: Last Hour, 4: Interval 1 Hour
    int backupTo;                 // REPORT_AUTO_BACKUP_TO_E, bit0: External Device, bit1: Email
    int lineMask;                 // 8bits(0: disable, 1:enable) 0 0 0 total line3 line2 line1 line0
} REPORT_AUTO_BACKUP_SETTING_S;
typedef struct {
    REPORT_AUTO_BACKUP_SETTING_S settings[MAX_REPORT_AUTO_BACKUP_TYPE_COUNT];
} REPORT_AUTO_BACKUP_S;
/********************end*****************/

//
#define MAX_AUDIO_FILE_NUM              (10)
#define AUDIO_FILE_NAME_SIZE            (MAX_LEN_128)
typedef struct audioFile
{
    int id;
    int enable;
    char name[AUDIO_FILE_NAME_SIZE];
}AUDIO_FILE;

typedef struct DbPosConfig {
    int id;
    int enable;
    char name[33];
    int protocol;
    int connectionMode;//0=tcp,1=udp
    char ip[64];
    int liveviewDisplay;
    int displayChannel;
    int startX;
    int startY;
    int areaWidth;
    int areaHeight;
    int characterEncodeing;
    int fontSize;
    int fontColor;
    int overlayMode;
    int displayTime;
    int timeout;
    char privacy1[64];
    char privacy2[64];
    char privacy3[64];
    int port;
    int clearTime; // Forcom screen clear time
    int nvrPort;
} Db_POS_CONFIG;

struct PushMsgNvrEvent
{
    char pos[MAX_LEN_65];
};

typedef struct DbVerInfo {
    char file[32];
    int execute;
} DB_VER_INFO_S;

struct DbMulticast
{
    int enable;
    char ip[MAX_LEN_65];
};

typedef struct NetworkKeyboards {
    char ip_addr[64];
    char password[64];
} NETWORK_KEYBOARD_S;

////////////////////////////////////////////////////////////////////////////////
//function
int sqlite_open(const char *pSqlFile, MS_EXE exeId);
int sqlite_close(const char *pSqlFile, MS_EXE exeId);

int db_run_sql(const char *db_file, const char *sql);
int db_query_sql(const char *db_file, const char *sql, char *str_result, int size);
int ms_get_executable_name(char *pNanme, int size);

int translate_pwd(char *dst, const char *src, int len);

/* all modules use */
void get_event_type(char *eventType, int event);
EVENT_IN_TYPE_E converse_chn_alarmid_to_event_in(int alarmId);
int converse_event_in_to_chn_alarmid(EVENT_IN_TYPE_E event);
int get_exception_name(int exceptId, char *exceptName, int len);
Uint64 get_channel_mask(char *copyChn);
SMART_EVENT_TYPE converse_vca_to_smart_event(VCA_ALARM_EVENT type);



/* osd */
int read_osd(const char *pDbFile, struct osd *osd, int chn_id);
int read_osd_name(const char *pDbFile, char *name, int chn_id);
int write_osd(const char *pDbFile, struct osd *osd);
int read_osds(const char *pDbFile, struct osd osds[], int *pCount);
int insert_osd(const char *pDbFile, struct osd *osd);

/* motion */
int read_motion(const char *pDbFile, struct motion *move, int id);
int read_motions(const char *pDbFile, struct motion moves[], int *pCnt);
int write_motion(const char *pDbFile, struct motion *move);
int write_motions(const char *pDbFile, struct motion *moves, int cnt, long long changeFlag);
int copy_motions(const char *pDbFile, struct motion *move, Uint64 changeFlag);
int read_motion_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id);
int delete_motion_schedule(const char *pDbFile, int chn_id);
int write_motion_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id);
int insert_motion(const char *pDbFile, struct motion *move);
//motion add
int read_motion_audible_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id);
int delete_motion_audible_schedule(const char *pDbFile, int chn_id);
int write_motion_audible_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id);
int copy_motion_audible_schedules(const char *pDbFile, struct motion_schedule *schedule, long long changeFlag);

/* alarm out */
int read_alarm_out(const char *pDbFile, struct alarm_out *alarm, int alarm_id);
int read_alarm_outs(const char *pDbFile, struct alarm_out alarms[], int *pCount);
int write_alarm_out(const char *pDbFile, struct alarm_out *alarm);
int write_alarm_outs(const char *pDbFile, struct alarm_out *alarm, int count, long long changeFlag);
int read_alarm_out_schedule(const char *pDbFile, struct alarm_out_schedule *schedule, int alarm_id);
int write_alarm_out_schedule(const char *pDbFile, struct alarm_out_schedule *schedule, int alarm_id);
int copy_alarm_out_schedules(const char *pDbFile, struct alarm_out_schedule *schedule, long long changeFlag);


/* video loss */
int read_video_lost(const char *pDbFile, struct video_loss *videolost, int chn_id);
int read_video_losts(const char *pDbFile, struct video_loss videolosts[], int *pCnt);
int write_video_lost(const char *pDbFile, struct video_loss *videolost);
int write_video_losts(const char *pDbFile, struct video_loss *videolosts, int count, long long changeFlag);
int copy_video_losts(const char *pDbFile, struct video_loss *videolost, long long changeFlag);
int read_video_loss_schedule(const char *pDbFile, struct video_loss_schedule *videolostSchedule, int chn_id);
int write_video_loss_schedule(const char *pDbFile, struct video_loss_schedule *schedule, int chn_id);
int insert_video_lost(const char *pDbFile, struct video_loss *videolost);
//video loss add
int read_videoloss_audible_schedule(const char *pDbFile, struct video_loss_schedule *videolostSchedule, int chn_id);
int write_videoloss_audible_schedule(const char *pDbFile, struct video_loss_schedule *schedule, int chn_id);
int copy_videoloss_audible_schedules(const char *pDbFile, struct video_loss_schedule *schedule, long long changeFlag);

/* camera */
int read_cameras_type(const char *pDbFile, int types[]);
int read_camera(const char *pDbFile,  struct camera *cam, int chn_id);
int read_cameras(const char *pDbFile, struct camera cams[], int *cnt);
int write_camera_codec(const char *pDbFile, struct camera *cam);
int write_camera(const char *pDbFile, struct camera *cam);
int write_cameras(const char *pDbFile, struct camera cams[], Uint64 changeFlag);
int insert_camera(const char *pDbFile, struct camera *cam);
int disable_ip_camera(const char *pDbFile, int nCamId);
int clear_cameras(const char *pDbFile);
int write_camera_ip(const char *pDbFile, int id, const char *ip);



/* ip camera */
/*
int read_ip_cameras(const char *pDbFile, struct camera ipCams[], int *pCnt);
int write_ip_camera(const char *pDbFile, struct camera *ipCam);
*/

/* record */
int read_record(const char *pDbFile, struct record *record, int chn_id);
int read_records(const char *pDbFile, struct record records[], int *pCnt);
int read_records_ex(const char *pDbFile, struct record records[], long long changeFlag);
int write_record(const char *pDbFile, struct record *record);
int write_records(const char *pDbFile, struct record records[], long long changeFlag);
int read_record_schedule(const char *pDbFile, struct record_schedule *schedule, int chn_id);
int read_record_schedules(const char *pDbFile, struct record_schedule *schedule, int count, int *pCnt);
int write_record_schedule(const char *pDbFile, struct record_schedule *schedule, int chn_id);
int write_record_schedules(const char *pDbFile, struct record_schedule *schedules, struct channel_batch *batch);
int copy_record_schedules(const char *pDbFile, struct record_schedule *schedule, long long changeFlag);
int reset_record_schedule(const char *pDbFile, int chn_id);
int write_record_prev_post(const char *pDbFile, struct record *record);
int insert_record(const char *pDbFile, struct record *record);
int check_record_post_time(const char *pDbFile);


/* holiday */
int read_holidays(const char *pDbFile, struct holiday holidays[], int *pCnt);
int write_holiday(const char *pDbFile, struct holiday *holiday);

/* ptz port */
int read_ptz_port(const char *pDbFile, struct ptz_port *port, int chn_id);
int write_ptz_port(const char *pDbFile, struct ptz_port *port);
int read_ptz_ports(const char *pDbFile, struct ptz_port ports[], int *pCnt);

/* alarm in */
int read_alarm_in(const char *pDbFile, struct alarm_in *alarm, int alarm_id);
int read_alarm_ins(const char *pDbFile, struct alarm_in alarms[], int *pCnt);
int write_alarm_in(const char *pDbFile, struct alarm_in *alarm);
int write_alarm_ins(const char *pDbFile, struct alarm_in *alarm, int count, long long changeFlag);
int copy_alarm_in_events(const char *pDbFile, struct alarm_in *alarms, int count, Uint64 changeFlag);
int read_alarm_in_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id);
int write_alarm_in_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id);
//alarm in add
int read_alarmin_audible_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id);
int write_alarmin_audible_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id);
int copy_alarmin_audible_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag);
int read_alarmin_ptz_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id);
int write_alarmin_ptz_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id);
int copy_alarmin_ptz_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag);
int read_alarmin_popup_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id);
int write_alarmin_popup_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id);
int copy_alarmin_popup_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag);

/* user */
int read_user_by_name(const char *pDbFile, struct db_user *user, const char *username);
int read_users(const char *pDbFile, struct db_user users[], int *pCnt);
int add_user(const char *pDbFile, struct db_user *user);
int write_user(const char *pDbFile, struct db_user *user);
int read_user_count(const char *pDbFile, int *pCnt);
int read_user(const char *pDbFile, struct db_user *user, int id);
int get_user_default_permission(struct db_user *user);

/* display */
int read_display(const char *pDbFile, struct display *display);
int read_display_oem(const char *pDbFile, struct display *display);
int write_display(const char *pDbFile, struct display *display);

/* spot */
int read_spots(const char *pDbFile, struct spot spots[], int *pCnt);
int write_spot(const char *pDbFile, struct spot *spot);

/* time */
int read_time(const char *pDbFile, struct time *times);
int write_time(const char *pDbFile, struct time *times);

/* network */
int read_network_mode(const char *pDbFile, int *pMode);
int read_network_bond(const char *pDbFile, struct network *net);
int read_network_muti(const char *pDbFile, struct network *net);
int write_network_bond(const char *pDbFile, struct network *net);
int write_network_muti(const char *pDbFile, struct network *net);
int read_network(const char *pDbFile, struct network *net);
int write_network(const char *pDbFile, struct network *net);


/* pppoe */
int read_pppoe(const char *pDbFile, struct pppoe *pppoe);
int write_pppoe(const char *pDbFile, struct pppoe *pppoe);

/* ddns */
int read_ddns(const char *pDbFile, struct ddns *ddns);
int write_ddns(const char *pDbFile, struct ddns *ddns);

/* upnp */
int read_upnp(const char *pDbFile, struct upnp *upnp);
int write_upnp(const char *pDbFile, struct upnp *upnp);

/* email */
int read_email(const char *pDbFile, struct email *email);
int write_email(const char *pDbFile, struct email *email);

/* network_more */
int read_port(const char *pDbFile, struct network_more *more);
int write_port(const char *pDbFile, struct network_more *more);

/* snmp */
int read_snmp(const char *pDbFile, struct snmp *snmp);
int write_snmp(const char *pDbFile, struct snmp *snmp);

/* audio in */
int read_audio_ins(const char *pDbFile, struct audio_in audioins[MAX_AUDIOIN], int *pCnt);
int write_audio_in(const char *pDbFile, struct audio_in *audioin);

/* layout */
int read_layout(const char *pDbFile, struct layout *layout);
int write_layout(const char *pDbFile, struct layout *layout);

int read_layout_custom(const char *pDbFile, struct layout_custom *layout, int *count);
int write_layout_custom(const char *pDbFile, struct layout_custom *layout);

int read_layout_custom_64Screen(const char *pDbFile, struct layout_custom *layout, int *count);
int write_layout_custom_64Screen(const char *pDbFile, struct layout_custom *layout);

/* ptz speed */
int read_ptz_speeds(const char *pDbFile, struct ptz_speed ptzspeeds[MAX_CAMERA], int *pCnt);
int read_ptz_speed(const char *pDbFile, struct ptz_speed *ptzspeed, int chn_id);
int write_ptz_speed(const char *pDbFile, struct ptz_speed *ptzspeed);

/* storage */
int read_storages(const char *pDbFile, struct storage storages[SATA_MAX], int *pCnt);
int read_storage(const char *pDbFile, struct storage *storage, int port_id);
int write_storage(const char *pDbFile, struct storage *storage);

/* ptz tour */
int read_ptz_tour(const char *pDbFile, struct ptz_tour tour[TOUR_MAX], int ptz_id);
int write_ptz_tour(const char *pDbFile, struct ptz_tour tour[TOUR_MAX], int ptz_id);

/* key-value */
int get_param_value(const char *pDbFile, const char *key, char *value, int value_len, const char *sdefault);
int get_param_int(const char *pDbFile, const char *key, int defvalue);
int set_param_value(const char *pDbFile, const char *key, const char *value);
int set_param_int(const char *pDbFile, const char *key, int value);
int check_param_key(const char *pDbFile, const char *key);
int add_param_value(const char *pDbFile, const char *key, const char *value);
int add_param_int(const char *pDbFile, const char *key, int value);
//params_oem
int get_param_oem_value(const char *pDbFile, const char *key, char *value, int value_len, const char *sdefault);
int get_param_oem_int(const char *pDbFile, const char *key, int defvalue);
int set_param_oem_value(const char *pDbFile, const char *key, char *value);
int set_param_oem_int(const char *pDbFile, const char *key, int value);
int check_param_oem_key(const char *pDbFile, const char *key);
int add_param_oem_value(const char *pDbFile, const char *key, char *value);
int add_param_oem_int(const char *pDbFile, const char *key, int value);

/* mosaic */
int read_mosaic(const char *pDbFile, struct mosaic *mosaic);
int read_mosaics(const char *pDbFile, struct mosaic mosaic[LAYOUT_MODE_MAX], int *pCnt);
int write_mosaic(const char *pDbFile, struct mosaic *mosaic);
int update_mosaics(const char *pDbFile, struct mosaic *mosaic, int count);
int read_custom_mosaics(const char *pDbFile, struct custom_mosaic *mosaics, int *count);
int write_custom_mosaics(const char *pDbFile, struct custom_mosaic *mosaics, int count);
int read_custom_mosaics_64Screen(const char *pDbFile, struct custom_mosaic *mosaics, int *count);
int write_custom_mosaics_64Screen(const char *pDbFile, struct custom_mosaic *mosaics, int count);

int read_disk_maintain_info(const char *pDbFile, struct disk_maintain_info *info);
int write_disk_maintain_info(const char *pDbFile, struct disk_maintain_info *info);

int read_p2p_info(const char *pDbFile, struct p2p_info *info);
int write_p2p_info(const char *pDbFile, struct p2p_info *info);

int read_privacy_masks(const char *pDbFile, struct privacy_mask mask[], int *pCnt, DB_TYPE o_flag);
int read_privacy_mask(const char *pDbFile, struct privacy_mask *mask, int chn_id);
int write_privacy_mask(const char *pDbFile, struct privacy_mask *mask);
int insert_privacy_mask(const char *pDbFile, struct privacy_mask *mask);

int read_alarm_pids(const char *pDbFile, struct alarm_pid **pid, int *pCnt);
int read_alarm_pid(const char *pDbFile, struct alarm_pid *pid);
int read_alarm_pid_by_sn(const char *pDbFile, struct alarm_pid *info);
int write_alarm_pid(const char *pDbFile, struct alarm_pid *pid);
int update_alarm_pid(const char *pDbFile, struct alarm_pid *pid);
int update_alarm_pid_by_sn(const char *pDbFile, struct alarm_pid *pid);
int delete_alarm_pid(const char *pDbFile, char *id);
int delete_alarm_pid_by_user_id(const char *pDbFile, int id);
void release_alarm_pid(struct alarm_pid **pid);



/////////////////////////////////////////////////////////////////////////////////////////////////////
int read_trigger_alarms(const char *pDbFile, struct trigger_alarms *pa);
int write_trigger_alarms(const char *pDbFile, const struct trigger_alarms *pa);
/////////////////////////////////////////////////////////////////////////////////////////////////////

/* camera io */
int read_cameraios(const char *pDbFile, struct camera_io cio[], int *cnt);
int read_cameraio(const char *pDbFile, struct camera_io *pio, int chanid);
int write_cameraio(const char *pDbFile, const struct camera_io *pio);
int insert_cameraio(const char *pDbFile, const struct camera_io *pio);

/* voluem detect */
int read_volumedetects(const char *pDbFile, struct volume_detect vd[], int *cnt);
int read_volumedetect(const char *pDbFile, struct volume_detect *pvd, int chanid);
int write_volumedetect(const char *pDbFile, const struct volume_detect *pvd);
int insert_volume_detect(const char *pDbFile, const struct volume_detect *pvd);

/* ipc protocol */
int read_ipc_protocols(const char *pDbFile, struct ipc_protocol **protocols, int *pCnt);
int read_ipc_protocol(const char *pDbFile, struct ipc_protocol *protocol, int pro_id);
int write_ipc_protocol(const char *pDbFile, struct ipc_protocol *protocol);
int insert_ipc_protocol(const char *pDbFile, struct ipc_protocol *protocol);
int delete_ipc_protocols(const char *pDbFile);
void release_ipc_protocol(struct ipc_protocol **protocols);


/* ipc model */
int read_ipc_models(const char *pDbFile, struct ipc_model **models, int *pCnt);
int read_ipc_models_by_protocol(const char *pDbFile, struct ipc_model **models, int pro_id, int *pCnt);
int read_ipc_model(const char *pDbFile, struct ipc_model *model, int mod_id);
int read_ipc_default_model(const char *pDbFile, struct ipc_model *model, int pro_id);
int write_ipc_model(const char *pDbFile, struct ipc_model *model);
int insert_ipc_model(const char *pDbFile, struct ipc_model *model);
int delete_ipc_models(const char *pDbFile);
void ipc_destroy(void *ptr);

//bruce.milesight add for param device
int db_get_device(const char *pDbFile, struct device_info *device);
int db_set_device(const char *pDbFile, struct device_info *device);
int db_get_device_oem(const char *pDbFile, struct device_info *device);
int db_set_device_oem(const char *pDbFile, struct device_info *device);

//bruce.milesight add for tz db
int db_get_tz_filename(const char *pDbFile, const char *tz, char *tzname, int tzname_size, char *tzfname,
                       int tzfname_size);

int write_layout1_chn(const char *pDbFile, int chnid);
int read_layout1_chn(const char *pDbFile, int *chnid);

//#########for default DB#########//
int db_read_user_by_name(const char *pDbFile, struct db_user *user, char *username);

int read_users_oem(const char *pDbFile, struct db_user_oem users[], int *pCnt);

int write_sub_layout1_chn(const char *pDbFile, int chnid);
int read_sub_layout1_chn(const char *pDbFile, int *chnid);

int read_poe_cameras(const char *pDbFile, struct camera cams[], int *cnt);
int write_autoreboot_conf(const char *pDbFile, reboot_conf *conf);
int read_autoreboot_conf(const char *pDbFile, reboot_conf *conf);

//7.0.6
int read_privacy_mask_by_areaid(const char *pDbFile, struct privacy_mask *mask, int chn_id, int area_id);
int write_privacy_mask_by_areaid(const char *pDbFile, struct privacy_mask *mask, int area_id);

int read_motion_email_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id);
int write_motion_email_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id);
int copy_motion_email_schedules(const char *pDbFile, struct motion_schedule *schedule, long long changeFlag);
int read_videoloss_email_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id);
int write_videoloss_email_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id);
int copy_videoloss_email_schedules(const char *pDbFile, struct video_loss_schedule  *schedule, long long changeFlag);
int read_videoloss_popup_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id);
int write_videoloss_popup_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id);
int copy_videoloss_popup_schedules(const char *pDbFile, struct video_loss_schedule  *schedule, long long changeFlag);

int read_alarmin_email_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id);
int write_alarmin_email_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id);
int copy_alarmin_email_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag);
int read_motion_popup_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id);
int write_motion_popup_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id);
int copy_motion_popup_schedules(const char *pDbFile, struct motion_schedule *schedule, Uint64 changeFlag);
int read_motion_ptz_schedule(const char *pDbFile, struct motion_schedule    *motionSchedule, int chn_id);
int write_motion_ptz_schedule(const char *pDbFile, struct motion_schedule   *schedule, int chn_id);
int copy_motion_ptz_schedules(const char *pDbFile, struct motion_schedule *schedule, long long changeFlag);

int read_alarm_in_ex(const char *pDbFile, struct alarm_in *alarm, int alarm_id);
int read_alarm_ins_ex(const char *pDbFile, struct alarm_in alarms[], int *pCnt);
int write_alarm_in_ex(const char *pDbFile, struct alarm_in *alarm);

int check_table_key(const char *pDbFile, char *table, const char *key);
int check_table_key_ex(const char *pDbFile, char *table, const char *key);

int read_disks(const char *pDbFile, struct diskInfo disks[], int *pCnt);
int read_disk(const char *pDbFile, struct diskInfo *disk, int portId);
int write_disk(const char *pDbFile, struct diskInfo *disk);
int insert_disk(const char *pDbFile, struct diskInfo *diskInfo);

int read_groups(const char *pDbFile, struct groupInfo groups[], int *pCnt);
int write_group_st(const char *pDbFile, struct groupInfo *group);

int write_ssl_params(const char *pDbFile, struct cert_info *cert);
int read_ssl_params(const char *pDbFile, struct cert_info *cert, int type);

int write_encrypted_list(const char *pDbFile, struct squestion *sqa, int sorder);
int read_encrypted_list(const char *pDbFile, struct squestion sqas[]);
int read_smart_event_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                        SMART_EVENT_TYPE type);
int write_smart_event_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                         SMART_EVENT_TYPE type);
int copy_smart_event_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type);

int read_smart_event_audible_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                      SMART_EVENT_TYPE type);
int write_smart_event_audible_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                       SMART_EVENT_TYPE type);
int copy_smart_event_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type);

int read_smart_event_mail_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                   SMART_EVENT_TYPE type);
int write_smart_event_mail_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                    SMART_EVENT_TYPE type);
int copy_smart_event_mail_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type);

int read_smart_event_popup_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                    SMART_EVENT_TYPE type);
int write_smart_event_popup_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                     SMART_EVENT_TYPE type);
int copy_smart_event_popup_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type);

int read_smart_event(const char *pDbFile, struct smart_event *smartevent, int id, SMART_EVENT_TYPE type);
int write_smart_event(const char *pDbFile, struct smart_event *smartevent, SMART_EVENT_TYPE type);
int copy_smart_events(const char *pDbFile, SMART_EVENT *smartEvent, Uint64 chnMask, SMART_EVENT_TYPE type);
int copy_smart_event_action(SMART_EVENT_TYPE type, int chnId, Uint64 chnMask, int popup);
void get_vca_white_table_name(int type, int event, char *des, int size);
void get_vca_http_table_name(int type, int event, char *des, int size);
void get_anpr_http_table_name(int type, ANPR_MODE_TYPE mode, char *name, int size);
void get_action_type_http_table_name(int type, EVENT_IN_TYPE_E event, char *des, int size);

int read_motion_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id);
int write_motion_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id);
int copy_motion_effective_schedules(const char *pDbFile, struct smart_event_schedule *schedule, long long changeFlag);

int read_smart_events(const char *pDbFile, struct smart_event *smartevent, int chnid);
int read_smart_event_effective_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int read_smart_event_audible_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int read_smart_event_mail_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int read_smart_event_popup_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);

int write_smart_event_effective_schedule_default(const char *pDbFile, int chn_id);

int read_failover(const char *pDbFile, struct failover_list *failover, int id);
int read_failovers(const char *pDbFile, struct failover_list failovers[], int *cnt);
int delete_failovers_by_ip(const char *pDbFile, char ips[][MAX_LEN_16], int cnt, Uint32 *pMask);
int write_failover(const char *pDbFile, struct failover_list *failover);
int reset_schedule_for_failover(const char *pDbFile);

int write_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, int type);
int read_ptz_params(const char *pDbFile, struct ptz_action_params ptzActionParams[], int type, int chn_id, int *cnt);
int delete_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, int type);
int update_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, int type, int oldChan);
int write_ptz_params_all(const char *pDbFile, struct ptz_action_params ptzActionParams[], int type, int chan_id);
int copy_ptz_params_all(const char *pDbFile, struct ptz_action_params ptzActionParams[], int type,
                        long long changeFlag);
int read_videoloss_ptz_schedule(const char *pDbFile, struct video_loss_schedule *videolostSchedule, int chn_id);
int write_videoloss_ptz_schedule(const char *pDbFile, struct video_loss_schedule *videolostSchedule, int chn_id);
int copy_videoloss_ptz_schedules(const char *pDbFile, struct video_loss_schedule *videolostSchedule,
                                 long long changeFlag);
int read_smart_event_ptz_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int write_smart_event_ptz_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                   SMART_EVENT_TYPE type);
int read_smart_event_ptz_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                  SMART_EVENT_TYPE type);
int copy_smart_event_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type);

int write_alarmin_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id);
int copy_alarmin_effective_schedules(const char *pDbFile, struct smart_event_schedule *schedule, long long changeFlag);
int read_alarmin_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id);

int write_exposure(const char *pDbFile, struct exposure exposures[], int chnid, int cnt);
int read_exposure(const char *pDbFile, struct exposure exposures[], int chnid, int *cnt);

int update_camera(const char *pDbFile, struct camera *cam, int type);

int read_snapshots(const char *pDbFile, struct snapshot snapshots[], int count, int *pCnt);
int read_snapshot(const char *pDbFile, struct snapshot *snapshot, int chn_id);
int write_snapshot(const char *pDbFile, struct snapshot *snapshot);
int write_snapshots(const char *pDbFile, struct snapshot *snapshots, long long changeFlag);
int copy_snapshots(const char *pDbFile, struct snapshot *snapshot, long long changeFlag);


int read_snapshot_schedule(const char *pDbFile, struct snapshot_schedule *schedule, int chn_id);
int read_snapshot_schedules(const char *pDbFile, struct snapshot_schedule *schedule, int count, int *pCnt);
int write_snapshot_schedule(const char *pDbFile, struct snapshot_schedule *schedule, int chn_id);
int write_snapshot_schedules(const char *pDbFile, struct snapshot_schedule *schedules, struct channel_batch *batch);
int copy_snapshot_schedules(const char *pDbFile, struct snapshot_schedule *schedule, long long changeFlag);
int reset_snapshot_schedule(const char *pDbFile, int chn_id);

int db_version_update(const char *pDbFile, int old_ver, int new_ver, const char *pTxtFile);
int db_version_update_by_info(const char *pDbFile, int updateCnt, DB_VER_INFO_S updateArr[], const char *pTxtFile);

int copy_table_column_int(const char *pDbFile, const char *table, const char *key, int value, long long  changeFlag,
                          const char *idKey);
int copy_table_column_string(const char *pDbFile, const char *table, const char *key, const char  *value,
                             long long  changeFlag, const char *idKey);
int write_privacy_masks(const char *pDbFile, struct privacy_mask *mask, long long changeFlag);


int check_msdb_database(const char *pDbFile);
int backup_msdb_database();
int update_msdb_database();

int read_videoloss_audible_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_videoloss_email_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_videoloss_popup_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_videoloss_ptz_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_ptz_params_init(const char *pDbFile, struct ptz_action_params *ptzActionParams[], int type, int chnCnt);

int read_motion_effective_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_motion_audible_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_motion_email_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_motion_popup_schedule_init(const char *pDbFile, void *params, int chnCnt);
int read_motion_ptz_schedule_init(const char *pDbFile, void *params, int chnCnt);

int read_record_schedule_init(const char *pDbFile, struct record_schedule *schedule, int chnCnt);

int read_smart_event_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_smart_event_audible_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_smart_event_email_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_smart_event_popup_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_smart_event_ptz_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_smart_event_led_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_smart_event_init(const char *pDbFile, struct smart_event *smartevent[], int allChn);
int read_smart_event_ptz_params_init(const char *pDbFile, struct vca_ptz_action_params *vca, int type, int chnCnt);

int write_anpr_list(const char *pDbFile, const struct anpr_list *info);
int write_anpr_list_replace(const char *pDbFile, const struct anpr_list *info);
int write_anpr_list_replace_batch(const char *pDbFile, const struct anpr_list *info, int count);
int write_anpr_list_ignore(const char *pDbFile, const struct anpr_list *info);
int write_anpr_lists(const char *pDbFile, const struct anpr_list *info, int count);
int read_anpr_lists(const char *pDbFile, struct anpr_list **info, int *pCnt);
void release_anpr_lists(struct anpr_list **info);
int read_anpr_list_plate(const char *pDbFile, struct anpr_list *info, const char *plate);
int read_anpr_list_cnt(const char *pDbFile);
int update_anpr_list(const char *pDbFile, const struct anpr_list *info);
int delete_anpr_list(const char *pDbFile, const struct anpr_list *info);
int delete_anpr_lists(const char *pDbFile, const struct anpr_list *info, int count);
int read_anpr_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                 ANPR_MODE_TYPE type);
int read_anpr_effective_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int write_anpr_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                  ANPR_MODE_TYPE type);
int read_anpr_audible_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                               ANPR_MODE_TYPE type);
int read_anpr_audible_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int write_anpr_audible_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                ANPR_MODE_TYPE type);
int read_anpr_mail_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                            ANPR_MODE_TYPE type);
int read_anpr_mail_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int write_anpr_mail_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                             ANPR_MODE_TYPE type);
int read_anpr_popup_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                             ANPR_MODE_TYPE type);
int read_anpr_popup_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int write_anpr_popup_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                              ANPR_MODE_TYPE type);
int read_anpr_ptz_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                           ANPR_MODE_TYPE type);
int read_anpr_ptz_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int write_anpr_ptz_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                            ANPR_MODE_TYPE type);
int read_anpr_event(const char *pDbFile, struct smart_event *smartevent, int id, ANPR_MODE_TYPE type);
int read_anpr_events(const char *pDbFile, struct smart_event *smartevent, int chnid);
int write_anpr_event(const char *pDbFile, struct smart_event *smartevent, ANPR_MODE_TYPE type);
int read_anpr_ptz_params(const char *pDbFile, struct ptz_action_params ptzActionParams[], ANPR_MODE_TYPE type,
                         int chn_id, int *cnt);
int write_anpr_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, ANPR_MODE_TYPE type);
int update_anpr_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, ANPR_MODE_TYPE type,
                           int oldChan);
int delete_anpr_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, ANPR_MODE_TYPE type);
int read_anpr_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_anpr_audible_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_anpr_email_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_anpr_popup_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_anpr_ptz_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_anpr_event_init(const char *pDbFile, struct smart_event *smartevent[], int chnCnt);
int read_anpr_ptz_params_init(const char *pDbFile, struct vca_ptz_action_params *anpr, ANPR_MODE_TYPE type, int chnCnt);
int read_anpr_enable_effective_schedule_init(const char *pDbFile, void *params, int chnCnt);

int read_alarm_chnIn_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int write_alarm_chnIn_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int copy_alarm_chnIn_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid,
                                         long long changeFlag);
int read_alarm_chnIn_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int write_alarm_chnIn_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int copy_alarm_chnIn_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid,
                                       long long changeFlag);
int read_alarm_chnIn_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int write_alarm_chnIn_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int copy_alarm_chnIn_mail_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, long long changeFlag);
int read_alarm_chnIn_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int write_alarm_chnIn_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int copy_alarm_chnIn_popup_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag);
int read_alarm_chnIn_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int write_alarm_chnIn_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int copy_alarm_chnIn_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, long long changeFlag);
int read_alarm_chnIn_event(const char *pDbFile, SMART_EVENT *smartevent, struct alarm_chn *info);
int read_alarm_chnIn_events(const char *pDbFile, SMART_EVENT *smartevent, int count, struct alarm_chn *info);
int write_alarm_chnIn_event(const char *pDbFile, SMART_EVENT *smartevent, struct alarm_chn *info);
int write_alarm_chnIn_events(const char *pDbFile, SMART_EVENT *smartevent, struct alarm_chn *info, int count,
                             long long changeFlag);
int copy_alarm_chnIn_events(const char *pDbFile, SMART_EVENT *smartevent, int alarmid, long long changeFlag);
int read_alarm_chnIn_event_name(const char *pDbFile, struct alarm_chn_name *param, struct alarm_chn *info);
int write_alarm_chnIn_event_name(const char *pDbFile, struct alarm_chn_name *param);
int clear_alarm_chnIn_event_name(const char *pDbFile, int chnid);
int read_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params ptzActionParams[], struct alarm_chn *info,
                                int *cnt);
int write_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams,
                                 struct alarm_chn *info);
int update_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams,
                                  struct alarm_chn *info);
int delete_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams,
                                  struct alarm_chn *info);

int read_alarm_chnOut_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int write_alarm_chnOut_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info);
int copy_alarm_chnOut_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info,
                                          long long changeFlag);
int read_alarm_chnOut_event_name(const char *pDbFile, struct alarm_chn_out_name *param, struct alarm_chn *info);
int read_alarm_chnOut_event_names(const char *pDbFile, struct alarm_chn_out_name *param, int count, int alarmid);

int write_alarm_chnOut_event_name(const char *pDbFile, struct alarm_chn_out_name *param);
int write_alarm_chnOut_event_names(const char *pDbFile, struct alarm_chn_out_name *param, int count,
                                   long long changeFlag);
int clear_alarm_chnOut_event_name(const char *pDbFile, int chnid);

int update_database_by_file(const char *pDbFile, char *db_file);

int read_alarm_chnIn_event_names(const char *pDbFile, struct alarm_chn_name *param, int count, int alarmid);
int read_alarm_out_trigger(const char *pDbFile, int dbtype, struct alarm_out_trigger *inParam,
                           struct alarm_out_trigger_chns *outParam);
int write_alarm_out_trigger(const char *pDbFile, int dbtype, struct alarm_out_trigger_chns *param);
int copy_alarm_out_triggers(const char *pDbFile, int dbtype, struct alarm_out_trigger_chns *param,
                            long long changeFlag);

int read_push_msg_event(const char *pDbFile, struct push_msg_event *param, int chn_id);
int write_push_msg_event(const char *pDbFile, struct push_msg_event *param);
int read_push_msg_events(const char *pDbFile, struct push_msg_event param[], int chnCnt);
int write_push_msg_events(const char *pDbFile, struct push_msg_event param[], int chnCnt);

int read_ipc_chnIn_events_init(const char *pDbFile, struct smart_event *smartevent[], int maxChnCnt);
int read_ipc_chnIn_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[],
                                            int maxChnCnt);
int read_ipc_chnIn_audible_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt);
int read_ipc_chnIn_email_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt);
int read_ipc_chnIn_popup_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt);
int read_ipc_chnIn_ptz_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt);
int read_ipc_chnIn_ptz_params_init(const char *pDbFile, struct vca_ptz_action_params *params, int alarmid,
                                   int maxChnCnt);
int read_ipc_chnOut_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[],
                                             int maxChnCnt);
int read_db_version(const char *pDbFile);

int read_whiteled_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, WLED_INFO *info);
int write_whiteled_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, WLED_INFO *info);
int copy_whiteled_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, WLED_INFO *info, Uint64 chnMasks);
int read_whiteled_effective_schedule_init(const char *pDbFile, const char *pDbTable, void *params, int maxChnCnt);

int read_whiteled_params(const char *pDbFile, WHITE_LED_PARAMS *ledParams, WLED_INFO *info, int *cnt);
int write_whiteled_params(const char *pDbFile, WHITE_LED_PARAMS *ledParams, const char *pDbTable);
int write_whiteled_params_all(const char *pDbFile, WHITE_LED_PARAMS *ledParams, const char *pDbTable, int chn_id);
int delete_whiteled_params(const char *pDbFile, WHITE_LED_PARAMS *ledParams, const char *pDbTable);
int copy_whiteled_params(const char *pDbFile, const char *pDbTable, WHITE_LED_PARAMS ledParams[], Uint64 chnMasks);
int read_whiteled_params_init(const char *pDbFile, const char *pDbTable, WHITE_LED_PARAMS *ledparams[], int maxChnCnt);

int read_smart_event_led_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id);
int read_smart_event_led_params_init(const char *pDbFile, WHITE_LED_PARAMS_EVTS *vca, int type, int chnCnt);
int read_anpr_led_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_anpr_led_params_init(const char *pDbFile, WHITE_LED_PARAMS_EVTS *anpr, ANPR_MODE_TYPE type, int chnCnt);
int read_ipc_chnIn_led_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt);
int read_ipc_chnIn_led_params_init(const char *pDbFile, WHITE_LED_PARAMS_EVTS *params, int alarmid, int maxChnCnt);

int read_access_filter(const char *pDbFile, struct access_list limit_list[], int *pCnt);
int write_access_filter(const char *pDbFile, struct access_list limit_list[], int count);

int read_peoplecnt_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int write_peoplecnt_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int copy_peoplecnt_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag);
int read_peoplecnt_email_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int write_peoplecnt_email_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int copy_peoplecnt_email_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag);
int read_peoplecnt_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int write_peoplecnt_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int copy_peoplecnt_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag);
int read_peoplecnt_wled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int write_peoplecnt_wled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid);
int copy_peoplecnt_wled_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag);
int read_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams, int groupid, int *cnt);
int write_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams);
int write_peoplecnt_wled_params_all(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams, int chn_id);
int delete_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams);
int copy_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS ledParams[], Uint64 chnMasks);
int read_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS ptzActionParams[], int groupid, int *cnt);
int write_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS *ptzActionParams);
int write_peoplecnt_ptz_params_ex(const char *pDbFile, PEOPLECNT_PTZ_PARAMS ptzActionParams[], int groupid);
int update_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS *ptzActionParams, int oldGroupid);
int delete_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS *ptzActionParams);
int read_peoplecnt_event(const char *pDbFile, PEOPLECNT_EVENT *smartevent, int groupid);
int write_peoplecnt_event(const char *pDbFile, PEOPLECNT_EVENT *smartevent);

int write_peoplecnt_logs(const char *pDbFile, struct peoplecnt_logs *log);
int write_peoplecnt_log_by_hours(const char *pDbFile, struct peoplecnt_log_hours *log);
int read_peoplecnt_logs(const char *pDbFile, struct peoplecnt_logs **log, int *pCnt, SEARCH_PEOPLECNT_LOGS *info);
void release_peoplecnt_log(struct peoplecnt_logs **log);
int read_peoplecnt_logs_by_date(const char *pDbFile, PEOPLECNT_INFO *peopleCnt, int ndate);

int read_peoplecnt_setting(const char *pDbFile, PEOPLECNT_SETTING *pSettings, int groupid);
int read_peoplecnt_settings(const char *pDbFile, PEOPLECNT_SETTING pSet[]);
int write_peoplecnt_setting(const char *pDbFile, PEOPLECNT_SETTING *pSettings);
int write_peoplecnt_settings(const char *pDbFile, PEOPLECNT_SETTING *pSettings, int count);

int delete_peoplecnt_logs_by_grouid(const char *pDbFile, int groupid);
int delete_peoplecnt_logs_by_date(const char *pDbFile, int date);

int read_peoplecnt_current(const char *pDbFile, PEOPLECNT_CURRENT *pCur, int groupid);
int read_peoplecnt_currents(const char *pDbFile, PEOPLECNT_CURRENT pCur[]);
int write_peoplecnt_current(const char *pDbFile, PEOPLECNT_CURRENT *pCur);

int read_quota(const char *pDbFile, quota_info_t quotas[], int *pCnt);
int write_quota(const char *pDbFile, quota_info_t *quota);

int read_peoplecnt_event_init(const char *pDbFile, void *params, int groudCnt);
int read_peoplecnt_audible_schedules_init(const char *pDbFile, void *params, int groudCnt);
int read_peoplecnt_email_schedules_init(const char *pDbFile, void *params, int groudCnt);
int read_peoplecnt_ptz_schedules_init(const char *pDbFile, void *params, int groudCnt);
int read_peoplecnt_wled_schedules_init(const char *pDbFile, void *params, int groudCnt);
int read_peoplecnt_wled_params_init(const char *pDbFile, PCNT_WLED_PARAMS_EVTS *params, int groudCnt);
int read_peoplecnt_ptz_params_init(const char *pDbFile, PEOPLECNT_ACT_PTZ_PARAMS *params, int groudCnt);

int read_peoplecnt_auto_backup(const char *pDbFile, REPORT_AUTO_BACKUP_S *backup);
int write_peoplecnt_auto_backup(const char *pDbFile, REPORT_AUTO_BACKUP_S *backup);

int read_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup, const char *sqlQuery);
int write__auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup, const char *sqlQuery);
int read_esata_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup);
int write_esata_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup);
int read_anpr_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup);
int write_anpr_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup);

int read_alarm_audio_file(const char *pDbFile, AUDIO_FILE audioFile[]);
int write_alarm_audio_file(const char *pDbFile, AUDIO_FILE *audioFile);
int clear_alarm_audio_file(const char *pDbFile);
int read_pos_setting(const char *pDbFile, Db_POS_CONFIG *pos, int id);
int read_pos_settings(const char *pDbFile, Db_POS_CONFIG pos[]);
int write_pos_setting(const char *pDbFile, Db_POS_CONFIG *pos);
int read_pos_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id);
int write_pos_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id);
int read_pos_event(const char *pDbFile, SMART_EVENT *smartevent, int id);
int write_pos_event(const char *pDbFile, SMART_EVENT *smartevent);
int read_pos_event_init(const char *pDbFile, void *params, int cnt);
int read_pos_schedules_init(const char *pDbFile, SCHEDULE_TYPE type, void *params, int cnt);
int read_pos_setting_init(const char *pDbFile, void *params, int cnt);

int read_regional_pcnt_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id, int regionNo);
int write_regional_pcnt_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id, int regionNo);
int copy_regional_pcnt_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, Uint64 chnMask, int regionNo);
int read_regional_pcnt_event(const char *pDbFile, SMART_EVENT *smartevent, int id, int regionNo);
int write_regional_pcnt_event(const char *pDbFile, SMART_EVENT *smartevent, int regionNo);
int copy_regional_pcnt_event(const char *pDbFile, SMART_EVENT *smartEvent, int regionNo, Uint64 chnMask);
int read_regional_pcnt_event_init(const char *pDbFile, void *params, int cnt, int regionNo);
int read_regional_pcnt_schedules_init(const char *pDbFile, SCHEDULE_TYPE type, void *params, int cnt, int regionNo);

int read_push_msg_nvr_event(const char *pDbFile, struct PushMsgNvrEvent *db);
int write_push_msg_nvr_event(const char *pDbFile, struct PushMsgNvrEvent *db); 

int read_ipc_ptz_params_init(const char *pDbFile, int type, VCA_PTZ_ACTION_PARAMS *params, int maxChnCnt);
int read_ipc_led_params_init(const char *pDbFile, int type, WHITE_LED_PARAMS_EVTS *params, int maxChnCnt);

int read_push_msgs_by_times(const char *pDbFile, int startTime, int endTime, MSG_CACHE_S *msgCache);
int get_push_msg_cnt(const char *pDbFile, int *cnt);
int write_push_msgs_from_cache(const char *pDbFile, int cnt, MSG_CACHE_S *msgCache, int *id);
int read_push_msg_index(const char *pDbFile,int * index);
int update_push_msg_index(const char *pDbFile,int index);

int read_db_ver_info(const char *pDbFile, DB_VER_INFO_S dbVerInfo[], int *cnt);
int write_db_ver_info(const char *pDbFile, DB_VER_INFO_S updateArr[], int updateCnt);

int check_table_exist(const char *pDbFile, const char *tblName, int *state);
int create_tbl_db_version(const char *pDbFile, int msdbVer);

int read_face_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int read_face_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt);
int write_face_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int copy_face_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag);
int read_face_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int read_face_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt);
int write_face_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int copy_face_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag);
int read_face_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int read_face_mail_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt);
int write_face_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int copy_face_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag);
int read_face_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int read_face_popup_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt);
int write_face_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int copy_face_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag);
int read_face_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int read_face_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt);
int write_face_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int copy_face_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag);
int read_face_whiteled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int read_face_whiteled_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt);
int write_face_whiteled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id);
int copy_face_whiteled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag);
int read_face_event(const char *pDbFile, struct smart_event *smartevent, int id);
int read_face_events(const char *pDbFile, SMART_EVENT *smartevent, int count);
int write_face_event(const char *pDbFile, SMART_EVENT *smartevent);
int write_face_events(const char *pDbFile, SMART_EVENT *smartevent, int count, long long changeFlag);
int copy_face_events(const char *pDbFile, SMART_EVENT *smartevent, long long changeFlag);
int read_multicast(const char *pDbFile, struct DbMulticast *multicast);
int write_multicast(const char *pDbFile, struct DbMulticast *multicast);

int read_http_notification_schedule_init(const char *pDbFile, const char *pDbTable, void *params, int maxCnt);
int read_http_notification_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int id);
int write_http_notification_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int id);
int copy_http_notification_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, Uint64 masks);

int read_http_notification_params_init(const char *pDbFile, const char *pDbTable, HTTP_NOTIFICATION_PARAMS_S *httpPrms, int maxCnt);
int read_http_notification_params(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *httpPrms, const char *pDbTable, int id);
int write_http_notification_params(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, const char *pDbTable);
int write_http_notification_params_batch(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, const char *pDbTable, int cnt);
int copy_http_notification_params(const char *pDbFile, const char *pDbTable, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, Uint64 masks);
int read_event_http_notification_params_init(const char *pDbFile, int type, HTTP_NOTIFICATION_PARAMS_S *params, int maxCnt);

int read_smart_event_http_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chnId);
int read_smart_event_http_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_smart_event_http_params(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S **pHttpPrms, int chnId);
int read_smart_event_http_params_init(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *httpPrms, int type, int chnCnt);
int read_anpr_http_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt);
int read_anpr_http_params_init(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *httpPrms, ANPR_MODE_TYPE type, int chnCnt);
int read_ipc_chnIn_http_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt);
int read_ipc_chnIn_http_params_init(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *params, int alarmid, int maxChnCnt);

int copy_http_notification_action(EVENT_IN_TYPE_E type, int chnId, Uint64 chnMask);

int read_audio_alarm(const char * pDbFile, SMART_EVENT *audioAlarm, int id);
int read_audio_alarms(const char *pDbFile, SMART_EVENT *audioAlarm, int cnt);
int write_audio_alarm(const char * pDbFile, SMART_EVENT *audioAlarm);
int copy_audio_alarms(const char * pDbFile, SMART_EVENT * audioAlarm, Uint64 chnMask);
int read_audio_alarm_sche(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int chnId);
int read_audio_alarm_sches(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int cnt);
int copy_audio_alarm_sche(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, Uint64 chnMask);

int read_web_param(const char *pDbFile, WEB_PARAM_S *pParam);
int update_web_param(const char *pDbFile, WEB_PARAM_S *pParam);

int read_network_keyboard(const char *pDbFile, NETWORK_KEYBOARD_S pParam[], int *cnt);
int insert_network_keyboard(const char *pDbFile, NETWORK_KEYBOARD_S *pParam);
int delete_network_keyboard(const char *pDbFile, NETWORK_KEYBOARD_S *pParam);

int read_disarming(const char * pDbFile, DISARMING_S *pDisarming);
int write_disarming(const char * pDbFile, DISARMING_S *pDisarming);

int write_db_update(const char * pDbFile, DB_VER_INFO_S oldArr[], int oldCnt, DB_VER_INFO_S updateArr[], int updateCnt,
    const char *pTxtFile);

int read_disk_health(const char * pDbFile, Uint64 *enableMask);
int write_disk_health(const char * pDbFile, Uint64 *enableMask);
int update_pushmsg();

#ifdef __cplusplus
}
#endif

#endif

#if 0

    1.update table mosaic table.set channels blog.
    2.init table params default value.

#endif

