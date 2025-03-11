#ifndef __OVFCLI_H
#define __OVFCLI_H

#include <poll.h>
#include <sys/poll.h>
#include "msg.h"
#include "curl/curl.h"

// wcm add
#include "../../../msgui/mscore/ms_common_socket.h"
#include "../sdkp2p/cJSON.h"

#define MAX_STREAMID_NUM    (5)
#define IPCADDR_MAXLEN      (64) // hrz.milesight for ipv6 16-->64
#define MAX_DOWNLOAD_SIZE   (100 * 1024)
#define MAX_RESOLUTION_NUM              (24)
#define MAX_VIDEO_ENCODER_TYPE          (2)  //CODECTYPE_H264 or CODECTYPE_H265
#define DEFAULT_BITRATE_MIN             (128)
#define DEFAULT_SUB_BITRATE_MAX         (1024)
#define DEFAULT_BITRATE_MAX             (8192)
#define DEFAULT_FRAMERATE_MIN           (1)
#define DEFAULT_FRAMERATE_MAX           (25)
#define HTTP_DIGEST_AUTH                (2)
#define HTTP_STATUS_OK                  (200)
#define CURL_DEFAULT_TIMEOUT            (2)


typedef void (*MS_IPC_SOCK_CALLBACK)(void *data, int size, void *resp);
typedef size_t (*MS_CURL_SOCK_CALLBACK)(void *ptr, size_t size, size_t nmemb, void *stream);
typedef int (*MS_CURL_SAFE_CALLBACK)(void *req, void *resp);

typedef enum {
    ERR_UNKNOW = -1,
    ERR_ALLOK = 0,
    ERR_NETWORK,
    ERR_PASSWORD,
    ERR_PARAMETER, //参数错误
    ERR_NOT_SUPPORT, //IPC不支持
    ERR_OTHER
} IPC_STAT;

struct vstreamcfg {
    int codectype;
    int width;
    int height;
    int fps;
    int bitrate;
    int rtspport;
    char respath[256];
    char media_pf[128];         //profile token
    char vsen_pf[128];          //video encoder configuration token
    float quality;
    char profile[128];
};

struct astreamcfg {
    int codectype;
    int bitrate;
    int samplerate;
};

struct deviceinfo {
    char ipaddr[32];
    char hwaddr[32];
    char manufacturer[64];
    char model[64];
    char fwversion[64];
    char sn[128];
    char hwid[128];
    struct vstreamcfg vms;
    struct vstreamcfg vss;
    struct astreamcfg as;
    int manageport;
    char netmask[32];
    char dns[32];
    char gateway[32];
    char search_from_netif[6];
    int active_status;
    char device_name[64];
    struct vstreamcfg streammCfgInfo[MAX_STREAMID_NUM];
    int streamCfgCnt;
    char real_sn[64];
};

enum protocol_type {
    PROTOCOL_TYPE_ONVIF = 0,
    PROTOCOL_TYPE_MILESIGHT = 2,
    PROTOCOL_TYPE_CANON = 3,
    PROTOCOL_TYPE_POE = 4,
    PROTOCOL_TYPE_ONVIF_POE = 5,
    MAX_PROTOCOL_TYPE,
};
struct devlist {
    struct deviceinfo *devs;
    int cnt;
};

struct ipc_network {
    char oldipaddr[16];
    int  port;
    char username[64];
    char password[64];
    char newipaddr[16];
    char newnetmask[16];
    char newgataway[16];
    char newdns[16];
    char macaddr[16];
    char model[32];
    char netif_form[64];
};

struct vsencode_cfg {
    int mainorsub;  //mainstream = 0, substream = 1
    int encoding;   //CODECTYPE_H264 = 0, CODECTYPE_H265 = 3
    int width;
    int height;
    int framerate;
    int bitrate;
    int iframeinterval;
    int cbr;
    char profile[32];   //编码复杂度
    float quality;
};
struct vsencode_cfgs {
    int vsencfgscnt;
    struct vsencode_cfg vsencfgs[2];
};
struct resolution {
    int width;
    int height;
};
struct vsencode_opts {
    int mainresoscnt;
    struct resolution mresolutions[MAX_RESOLUTION_NUM];
    int mainfpsmin;
    int mainfpsmax;
    int mainbpsmin;
    int mainbpsmax;
    int mainIframeMin;
    int mainIframeMax;
    int subresoscnt;
    struct resolution sresolutions[MAX_RESOLUTION_NUM];
    int subfpsmin;
    int subfpsmax;
    int subbpsmin;
    int subbpsmax;
    int subIframeMin;
    int subIframeMax;
    int encodeType;     //for profile T
};

struct IntRange {
    int min;
    int max;
};

struct FloatRange {
    Float min;
    Float max;
};

struct VidCfgOpts {
    int encoding;                               //CODECTYPE_H264 or CODECTYPE_H265
    int resCnt;                                 //resolution cnt
    struct resolution res[MAX_RESOLUTION_NUM];
    struct IntRange bitRate;
    struct FloatRange qltRng;                   //quality Range vbr下的图像质量范围
    char profiles[MAX_LEN_64];                  //编码复杂度:Baseline Main High
    char frmRateRng[640];                       //frame Rate Range:25 22 20 18 16 15 12 10 8 6 4 2 1 0.5 0.25 0.125 0.0625
    char ifrmIntvRng[MAX_LEN_32];                //iframe Interval Range:1 400
    int isCbrSupport;                           //Support cbr(ConstantBitRate)
};

struct VidEncCfgOpts {              //video encoder configuration options
    int cnt;
    struct VidCfgOpts cfgOpts[MAX_VIDEO_ENCODER_TYPE];
};

struct imgattr {
    float brightness;
    float saturation;
    float contrast;
    float sharpness;
};

enum alarm_mode {
    ALARM_MODE_NULL   = 0,
    ALARM_MODE_PULL   = (1 << 0),
    ALARM_MODE_STREAM = (1 << 1),
    ALARM_MODE_NOTIFY = (1 << 2),
    ALARM_MODE_ALL    = (~0),
};

int ovf_event_mode(void *ovfcli);
#ifndef __EVSMSG
#define __EVSMSG

typedef enum {
    EVS_UNKNOW = 0,
    EVS_MOTION,
} EVENT_TYPE;

struct evsmsg {
    int have_event;
    int window_num;
    EVENT_TYPE evs_detail;
};
#endif

//pan tilt params
struct ptprms {
    float x;
    float y;
    long timeout; //for continuous move
};
//zoom params
struct zxprms {
    float x;
    long timeout; //for continuous move
};
struct srl_info {
    int baudrate;
    int paritybit;
    int charlen;
    int srltype;
    float stopbit;
};
//move type
typedef enum {
    MOV_ABS = 0,
    MOV_CON,
    MOV_REL
} MOVTYPE;

typedef enum {
    TOURSTART = 0,
    TOURSTOP,
    TOURPAUSE
} TOURMOVE;

typedef enum {
    PRETOKENADD = 0,
    PRETOKENDEL,
    PRETOKENUP,
    PRETOKENDOWN,
    PRETOKENCLEAR,
} TOURACT;

struct tour_prms {
    float pt_spd_x;
    float pt_spd_y;
    float zo_spd_x;
    long long time;

    TOURACT action;
    int sequence;
    char *pretoken;
};

struct time_detail_ {
    int begin_hour;
    int begin_minute;
    int end_hour;
    int end_minute;
};

struct motion_scedule_ {
    char msstr[64];
    int day;    /* mon=0, tue=1, ... sun=6 */
    int onoff;  /* on=1 off=0 */
    struct time_detail_ tmdetail[4];
};
struct ipc_motion_map {
    int motion_enable;
    char mapbuf[300];
    int sensitivity;    /** ms: [1,10] */
};

struct ipc_motion_scedule {
    struct motion_scedule_ scedule[7];
};

/* get/set motion detect area (end) */
struct ipc_osd_http {
    char ipcaddr[64];
    int port;
    char username[64];
    char password[64];

    char version[64];

    int enable_text;
    char text_name[256];
    int textpos ;

    int enable_date;
    int datepos;
    int dateformat; //  0 yyyy/mm/dd 1 mm/dd/yyyy  2 dd/mm/yyyy
    FONT_SIZE_E fontSize;

    int enable_text_sub;
    char text_name_sub[256];
    int textpos_sub;
    int enable_date_sub;
    int datepos_sub;
    int dateformat_sub;
    FONT_SIZE_E fontSizeSub;

    int enable_text_thd;
    char text_name_thd[256];
    int textpos_thd;
    int enable_date_thd;
    int datepos_thd;
    int dateformat_thd;
    FONT_SIZE_E fontSizeThd;
    
    OSD_STREAM_TYPE stream_type;
};

struct canon_ipc_Volume_Detection {
    int Volume_Detection_enable;
    int detect_cri;
    int volume_level ;
    int det_c_duration;     //0-10 s
    int on_event_opr;
    int off_event_opr;
    int ongoing_on_event_opr;
};

struct ovf_canon_Device_Input {
    int device_input_enable;
    int Operation_Mode;
};

#define MAX_MASK_REGION 24
struct cam_mask {
    int index;
    int type;   // 1: white; 2: Black; 3: ...
    int left;
    int top;
    int right;
    int bottom;
    int enable;
};

typedef struct devinfo {
    char manufacturer[64];
    char model[64];
    char sn[128];
} DEVINFO;

typedef struct ovfcli_net {
    int lan1_enable;
    int lan1_type;     //poe 0 | 1
    char lan1_name[64];
    char lan1_ipaddr[64];
    int lan2_enable;
    int lan2_type;     //poe 0 | 1
    char lan2_name[64];
    char lan2_ipaddr[64];
} OVFCLI_NET;

typedef struct ovfcli_info {
    int poe_num;
    int oem_type;
    int max_lan;
    OVFCLI_NET network;
    void (*cbBaseNotify)(int chnid, int sessionId, MS_BOOL isMotion);
} OVFCLI_INFO;

struct nvrinfo {
    char hostname[32];
    char ipaddr[32];
    int port;
    char netmask[16];
    char gateway[16];
    char dns[32];
    char sn[MAX_LEN_SNCODE];
    char devicetype[16];
    char software[24];
    char model[32];
    char uptime[32];
    char nonce[65];
    int http_port;
    char oem_model[32];
    int upsec;
    char uptone[32];
    int active_status;
    int failover_mode;//
    char failover_ip[32];
    char password[32];
    int max_camera;
    int bond_width;
    char device_name[16];

    char username[32];
    char mac[32];
};


struct nvrlist {
    struct nvrinfo *devs;
    int cnt;
};

typedef struct ms_ipc_sock {
    char req[MAX_LEN_256];
    void *resp;
    MS_IPC_SOCK_CALLBACK recvCb;
} MS_IPC_SOCK;

typedef struct ms_ipc_sock_curl {
    char req[MAX_LEN_256];
    void *resp;
    MS_CURL_SOCK_CALLBACK recvCurl_Cb;
} MS_IPC_SOCK_CURL;

typedef struct ms_curl_params {
    void *data;
    int size;
} MS_CURL_PARAMS;

typedef struct ms_curl_buff_info {
    char *data;
    int cursize;
    int allsize;
} MS_CURL_BUFF_INFO;

typedef enum {
    PROFILE_NONE = 0,
    PROFILE_S,
    PROFILE_T,
    PROFILE_MAX,
} ONVIF_MODE;

struct ms_ovfcli_info {
    char ipcaddr[IPCADDR_MAXLEN];
    int port;
    char username[64];
    char password[64];
    char version[64];
};

IPC_STAT ovf_test_network(const char *ipaddr, int port, const char *username, const char *password);
void *ovf_create(int chnid, int stream, const char *ipaddr, int port, const char *user, const char *pass, int *error, void *param);
void ovf_destroy(void *ovfcli);
void ovf_errmsg(int errcode, char buf[128]);
void ovf_print_allparm(void *ovfcli, void *(*print_cb)(char *format, ...));
int ovf_base_notify(void *ovfcli, char *localIp1, char *localIp2);
void ovf_notify_debug(int chnid, int opt);



int ovf_dev_discovery(int protocol, struct devlist *devlist, const char *src_addr, char *dev_name);
int ovf_dev_nvr_discovery(struct nvrlist *devlist, const char *src_addr, char *dev_name);
int ovf_dev_discovery_safe_allfree();
int ovf_dev_get_devinfo(void *ovfcli, struct deviceinfo *devinfo);
int ovf_dev_set_devinfo(void *ovfcli, struct deviceinfo *devinfo);
int ovf_dev_set_network(int protocol, struct ipc_network *netval);
int ovf_dev_set_ntp(void *ovfcli);
int poe_set_ipcnetwork(struct ipc_network *netval);

int ovf_media_get_vsencodecfg(void *ovfcli, struct vsencode_cfgs *vsen);
int ovf_media_set_vsencodecfg(void *ovfcli, struct vsencode_cfg *cfg);
int ovf_media_get_vsencodecfgopt(void *ovfcli, struct vsencode_opts *avaireso);
int ovf_media_set_audioonoff(void *ovfcli, int onoff);
int ovf_media_get_snapshot(void *ovfcli, char *imgpath);

int ovf_img_get_imgattr(void *ovfcli, struct imgattr *imgattr);
int ovf_img_set_brightness(void *ovfcli, float brightness);
int ovf_img_set_saturation(void *ovfcli, float saturation);
int ovf_img_set_contrast(void *ovfcli, float contrast);
int ovf_img_set_sharpness(void *ovfcli, float sharpness);

int ovf_get_sub_stream(void *ovfhdl);
int ovf_get_main_stream(void *ovfhdl);
int ovf_check_is_milesight_camera(void *ovfhdl);
int ovf_check_camera_by_sn(char *pSncode);
int ovf_check_camera_by_manufacturer(char *pManufacturer);
int ovf_check_is_ipv6_address(const char *ipaddr);
int ovf_get_media_profiles_bitrate(const char *ipaddr, int port, const char *user, const char *pass, int doauth);
int ovf_get_media_bitrate_by_handle(void *ovfcli, int *mbitrate, int *sbitrate);
//int ovf_get_osd_information(const char* ipaddr, int port, const char* user, const char* pass, int doauth, struct ipc_osd_http *information);
int ovf_get_mask_information(const char *ipaddr, int port, const char *user, const char *pass, int doauth, int *enable,
                             struct cam_mask mask[MAX_MASK_REGION], char *sdkversion, int sdklen);
int ovf_ipc_upgrade_image(void *ovfhdl, struct ms_upgrade_ipc_image_info *param);

int IsHisiIPC(const char *sModel);   // Model: MS-Cxxxx-FPN(B)
int IsAmbaIPC(const char *sModel); // Model: MS-Cxxxx-FPN(A)
int IsTiIPC(const char *sModel);      // Model: MS-Cxxxx-xxx
int IsSigmaIPC(const char *sModel);     // Model: MS-Cxxxx-FPD
int is_nt_ipc(const char *sModel);

MS_BOOL is_ms_lpr(IPC_AITYPE_E aitype);
int ovfcli_init(OVFCLI_INFO *ovf_info);
int ovfcli_deinit(void);

int update_ovfcli_search_handle(int onvif, int msp, int can);
int ovf_event_pullmsg(void *ovfcli, struct evsmsg *outmsg);

int ovf_ptz_init(void *ovfcli, int chanid);
int check_ovf_ptz_init(void *ovfcli);
int ovf_ptz_color(void *ovfcli);
int ovf_ptz_support(void *ovfcli);
void ovf_ptz_get_presets(void *ovfcli, struct resp_ptz_preset pre[PRESET_MAX]);
int ovf_ptz_set_preset(void *ovfcli, char *gui, char *pstoken, char *pname);
int ovf_ptz_goto_preset(void *ovfcli, char *gui);
int ovf_ptz_clr_preset(void *ovfcli, char *gui);

int ovf_ptz_operate_preset_tour(void *ovfcli, char *tourid, TOURMOVE op);
int ovf_ptz_modify_preset_tour(void *ovfcli, char *tourid, struct tour_prms *data, int chanid);
int ovf_ptz_set_preset_tour(void *ovfcli, char *tourid, struct tour_prms *data, int chanid);

int ovf_ptz_set_pantilt_mov(void *ovfcli, MOVTYPE type, struct ptprms *data);
int ovf_ptz_set_zoom_mov(void *ovfcli, MOVTYPE type, struct zxprms *data);
int ovf_ptz_send_aux(void *ovfcli, char *aux);
int ovf_ptz_stop(void *ovfcli, int pantilt, int zoom);
int ovf_ptz_set_srl(void *ovfcli, struct srl_info *srl);
int ovf_ptz_set_iris(void *ovfcli, float iris);
int ovf_ptz_focus_move(void *ovfcli, float speed);
int ovf_ptz_focus_stop(void *ovfcli);

int ovf_ptz_track_start(void *ovfcli, int track_id);
int ovf_ptz_track_stop(void *ovfcli, int track_id);
int ovf_ptz_track_run(void *ovfcli, int track_id);
int ovf_ptz_track_stoprun(void *ovfcli, int track_id);
int ovf_ptz_preset(void *ovfcli, struct ptz_preset *sPresetId);

int ovf_canon_voice_get(void *ovfcli, struct canon_ipc_Volume_Detection *info);
int ovf_canon_voice_set(void *ovfcli, struct canon_ipc_Volume_Detection *info);
int ovf_canon_device_input_get(void *ovfcli, struct ovf_canon_Device_Input *info);
int ovf_canon_device_input_set(void *ovfcli, struct ovf_canon_Device_Input *info);
int ovf_canon_audio_detect_state(void *ovfcli);
int ovf_canon_input_state(void *ovfcli);
void ovfcli_set_debug(int on);
void ovfcli_set_profile(ONVIF_MODE mode);
ONVIF_MODE ovfcli_get_onvif_mode();
int is_profile_s_mode();
int is_profile_t_mode();
int is_profile_enabled();
int is_support_t(int isSupportT);
int ovfcli_profile_t_test(void *ovfhdl);
int ovfcli_my_test(void *ovfhdl);


//@david##############################
int ovf_poe_camera_PasswordChallenge(void *ovfcli, char *pk, char *key, int *keyId);
#if 0
    int ovf_poe_camera_NewModifyPassword(void *ovfcli, char *oldPassword, char *sNewPassword, int keyId);
    int ovf_ipc_set_motionmap(void *ovfhdl, struct ipc_motion_map *mtmap);
    int ovf_ipc_get_motionsce(void *ovfhdl, struct ipc_motion_scedule *mtsce);
    int ovf_ipc_set_motionsce(void *ovfhdl, struct ipc_motion_scedule *mtsce);
    int ovf_get_osd(void *ovfhdl, struct ipc_osd_http *info);
    int ovf_set_osd(void *ovfhdl, struct ipc_osd_http *info);
    int ovf_get_mask(void *ovfhdl, int *enable, struct cam_mask mask[MAX_MASK_REGION]);
#endif
int ovf_set_mask(void *ovfhdl, int enable, struct cam_mask mask[MAX_MASK_REGION], int type);

//##############################


//##################

int curl_poe_camera_PasswordChallenge(void *ovfhdl, struct ms_ipc_psw_challenge_resp *resp);
int curl_poe_camera_NewModifyPassword(void *ovfhdl, char *oldPassword, char *sNewPassword, int keyId);

int curl_ipc_set_motionmap(void *ovfhdl, struct ipc_motion_map *mtmap);

int curl_ipc_get_motionsce(void *ovfhdl, struct ipc_motion_scedule *mtsce);////????????
int curl_ipc_set_motionsce(void *ovfhdl, struct ipc_motion_scedule *mtsce);

int curl_get_osd_information(struct ipc_osd_http *info);
int curl_get_osd(void *ovfhdl, struct ipc_osd_http *info);
int curl_set_osd(void *ovfhdl, struct ipc_osd_http *information);

#if 0
int curl_get_mask_information(const char *ipaddr, int port, const char *user, const char *pass, int doauth,
                              struct ms_ipc_mask_resp *resp);
int curl_get_mask(void *ovfhdl, struct ms_ipc_mask_resp *resp);
int curl_set_mask(void *ovfhdl, int enable, struct cam_mask mask[MAX_MASK_REGION], int type);
#endif

//##################

int curl_get_ipc_led_status(MS_CURL_INFO *info, struct ms_ipc_led_status_resp *resp);
int curl_get_all_alarm_status(MS_CURL_INFO *info, struct ms_ipc_event_status *resp);
int curl_get_vca_alarmstatus(MS_CURL_INFO *info, struct ms_vca_alarm_status *resp);
int curl_get_ipc_alarm_count(MS_CURL_INFO *info, struct ms_ipc_alarm_cnt *resp);
int curl_get_ipc_led_params(MS_CURL_INFO *info, struct ms_ipc_led_params_resp *resp);
int curl_ipc_check_online_upgrade(char *param, struct resp_check_online_upgrade *resp);
int curl_nvr_check_online_upgrade(struct ms_check_online_upgrade *param, struct resp_check_online_upgrade *resp);
int curl_get_online_upgrade_image(struct ms_online_upgrade_image *info);
int curl_get_ipc_online_upgrade_info(MS_CURL_INFO *info, struct ipc_online_upgrade_info *resp);
int curl_set_manual_tracking_region(MS_CURL_INFO *info, MS_TRK_REGION *params);

int socket_of_download_ipc_image_init(SOCKET_T *pFd, const char *url);
int recv_ipc_image(SOCKET_T fd, REQ_UPGRADE_CAMERA_S *pReqInfo, char *recvPacket, int cnt,
                   int *pPacketLen, int *pDownloadSize);
int send_ipc_image(struct pollfd *pollFd, REQ_UPGRADE_CAMERA_S *pReqInfo, char *sendPacket, int *pPacketLen);
int recv_resp_header_of_download_ipc_image(SOCKET_T fd, REQ_UPGRADE_CAMERA_S *pReqInfo);
int curl_hdl_of_send_image_init(struct pollfd **pollFd, struct ms_ovfcli_info *pOvfInfo,
                                REQ_UPGRADE_CAMERA_S *pReqInfo, char *sendPacket, void *ovfHdl);
int curl_hdls_of_send_image_init(struct pollfd pollFdArr[], struct ms_ovfcli_info ovfInfoArr[],
                                REQ_UPGRADE_CAMERA_S *pReqInfo, char *sendPacket, void *ovfHdlArr[],
                                RESP_UPGRADE_CAMERA_S *pResp);
int curl_send_image_head(struct pollfd pollFdArr[], struct ms_ovfcli_info ovfInfoArr[], REQ_UPGRADE_CAMERA_S *pReqInfo,
                         char *sendPacket, RESP_UPGRADE_CAMERA_S *pResp);
int curl_send_image_tail(struct pollfd *pollFd, char *sendPacket);
int curl_recv_ipc_upgrade_res(struct pollfd *pollFd);
int socket_of_recv_local_ipc_image_init(SOCKET_T * pServFd);
int socket_of_recv_local_ipc_image_accept(SOCKET_T servFd, SOCKET_T * pClieFd, REQ_UPGRADE_CAMERA_S *pReqInfo, RESP_UPGRADE_CAMERA_S *pResp);

int curl_set_ipc_led_params(MS_CURL_INFO *info, struct white_led_alarm *param);
int curl_set_ipc_ptz_control(MS_CURL_INFO *info, int ptzCmd);
int curl_set_manual_tracking(MS_CURL_INFO *info, int manual);
int curl_set_ipc_heat_map_setting(MS_CURL_INFO *info, struct ms_heat_map_setting *param);
int curl_set_ipc_image_dnrlevel(MS_CURL_INFO *info, struct req_set_imageparam *param);
int curl_set_ipc_image_enhancement(MS_CURL_INFO *info, struct image_enhancement *param);
int curl_set_ipc_image_display(MS_CURL_INFO *info, struct image_display *param);
int curl_set_ipc_edit_privacy_mask(MS_CURL_INFO *info, int maskId);
int curl_set_ipc_upgrade_reset(void *ovfhdl, int param);
int curl_set_ipcs_upgrade_reset(void *ovfhdlArr[], const Uint64 keepConfMask);
int curl_set_ipc_alarmout_state(MS_CURL_INFO *info, struct ms_ipc_alarm_out_state *param);

int curl_set_ipc_privacy_mask(MS_CURL_INFO *info, struct resp_privacy_mask *param);
int curl_delete_all_privacy_mask(MS_CURL_INFO *info);
int curl_set_ipc_alarm_in(MS_CURL_INFO *info, struct ms_ipc_alarm_in_cfg *param);
int curl_set_ipc_alarm_out(MS_CURL_INFO *info, struct ms_ipc_alarm_out_cfg *param);
int curl_set_auto_tracking(MS_CURL_INFO *info, struct ms_auto_tracking *param);
int curl_set_lpr_license(MS_CURL_INFO *info, struct ms_lpr_license *param);
int curl_set_lpr_settings(MS_CURL_INFO *info, struct ms_lpr_settings *param);
int curl_del_ipc_privacy_mask_single(MS_CURL_INFO *info, int id);
int curl_del_ipc_image_roi_single(MS_CURL_INFO *info, int id);
int curl_set_vca_leftremove(MS_CURL_INFO *info, void *reqData);

int curl_search_heat_map_report(MS_CURL_INFO *info, struct ms_heat_map_report *req, struct ms_heat_map_report_resp *resp);
int curl_get_ipc_heat_map_setting(MS_CURL_INFO *info, struct ms_heat_map_setting *resp);
int curl_get_ipc_auto_track_point(MS_CURL_INFO *info, struct ms_personpoint_info *resp);
int curl_get_ipc_image_dnrlevel(MS_CURL_INFO *info, struct resp_get_imageparam *resp);
int curl_get_ipc_image_enhancement(MS_CURL_INFO *info, struct image_enhancement *resp);
int curl_get_ipc_image_display(MS_CURL_INFO *info, struct image_display *resp);
int curl_get_ipc_privacy_mask(void *ovfhdl, struct resp_privacy_mask *resp);
int curl_get_ipc_alarm_in(MS_CURL_INFO *info, struct ms_ipc_alarm_in *resp);
int curl_get_ipc_alarm_out(MS_CURL_INFO *info, struct ms_ipc_alarm_out *resp);

int curl_get_auto_tracking(MS_CURL_INFO *info, struct ms_auto_tracking *resp);
int curl_get_lpr_support(MS_CURL_INFO *info, struct ms_lpr_support_info *resp);
int curl_get_lpr_license(MS_CURL_INFO *info, struct ms_lpr_license *resp);
int curl_get_lpr_settings(MS_CURL_INFO *info, struct ms_lpr_settings *resp);

int curl_get_anr_support(MS_CURL_INFO *info);
int curl_get_vca_leftremove(MS_CURL_INFO *info, struct ms_smart_leftremove_info *resp);
int curl_get_camera_general(const char *ipaddr, const char *username, const char *password, int port,
                            struct camera_general *resp);

int curl_set_image_exposure_sche(MS_CURL_INFO *info, struct exposure_schedule *param) ;
int curl_get_image_exposure_sche(MS_CURL_INFO *info, struct exposure_schedule *resp);

int curl_set_image_white_balance_sche(MS_CURL_INFO *info, struct white_balance_schedule *param);
int curl_get_image_white_balance_sche(MS_CURL_INFO *info, struct white_balance_schedule *resp);

int curl_set_image_bwh_sche_single(MS_CURL_INFO *info, struct bwh_schedule_single *param);
int curl_set_image_white_balance_sche_single(MS_CURL_INFO *info, struct white_balance_schedule_single *param);
int curl_set_image_exposure_sche_single(MS_CURL_INFO *info, struct exposure_schedule_single *param);

int curl_set_image_bwh_sche(MS_CURL_INFO *info, struct bwh_schedule *sche);
int curl_get_image_bwh_sche(MS_CURL_INFO *info, struct bwh_schedule *resp);

int curl_get_ipc_sdkversion(void *ovfhdl, char *resp);

int curl_set_ipc_image_roi(void *ovfhdl, struct set_image_roi *param);
int curl_get_ipc_image_roi(MS_CURL_INFO *info, struct set_image_roi *resp);

int curl_set_ipc_image_day_night(MS_CURL_INFO *info, IMAGE_DAYNIGHT_SINGLE_S *param);
int curl_get_ipc_image_day_night(MS_CURL_INFO *info, IMAGE_DAYNIGHT_SINGLE_S *resp);

int curl_get_ipc_vca_personpoint(MS_CURL_INFO *info, struct ms_personpoint_info *resp);
int curl_set_ipc_vca_cleancount(MS_CURL_INFO *info, struct ms_vca_cleancount_info *param);
int curl_set_ipc_vca_settings(MS_CURL_INFO *info, struct ms_vca_settings_info *param);
int curl_get_ipc_vca_settings(MS_CURL_INFO *info, struct ms_vca_settings_info *resp);
int curl_set_ipc_vca_license(MS_CURL_INFO *info, struct ms_vca_license *param);
int curl_get_ipc_vca_license(MS_CURL_INFO *info, struct ms_vca_license *resp);

int curl_uuid_to_msserver(struct ms_uid_info *param, struct ms_uid_resp_info *resp);

int curl_set_ipc_vca_people_count(MS_CURL_INFO *info, void *reqData);
int curl_set_ipc_vca_humandetection(MS_CURL_INFO *info, void *reqData);
int curl_set_ipc_vca_linecrossing(MS_CURL_INFO *info, void *reqData);
int curl_set_ipc_vca_tamper(MS_CURL_INFO *info, void *reqData);
int curl_set_ipc_vca_loitering(MS_CURL_INFO *info, void *reqData);
int curl_set_ipc_vca_advancedmotion(MS_CURL_INFO *info, void *reqData);
int curl_set_ipc_vca_regionexit(MS_CURL_INFO *info, void *reqData);
int curl_set_ipc_vca_regionentrance(MS_CURL_INFO *info, void *reqData);

int curl_get_ipc_vca_regionentrance(MS_CURL_INFO *info, struct ms_smart_event_info *resp);
int curl_get_ipc_vca_regionexit(MS_CURL_INFO *info, struct ms_smart_event_info *resp);
int curl_get_ipc_vca_advancedmotion(MS_CURL_INFO *info, struct ms_smart_event_info *resp);
int curl_get_ipc_vca_tamper(MS_CURL_INFO *info, struct ms_smart_event_info *resp);
int curl_get_ipc_vca_linecrossing(MS_CURL_INFO *info, struct ms_linecrossing_info *resp);
int curl_get_ipc_vca_loitering(MS_CURL_INFO *info, struct ms_smart_event_info *resp);
int curl_get_ipc_vca_humandetection(MS_CURL_INFO *info, struct ms_smart_event_info *resp);
int curl_get_ipc_vca_people_count(MS_CURL_INFO *info, struct ms_smart_event_people_cnt *resp);

int curl_get_ipc_vca_support(struct ipc_system_info *info, struct ms_vca_support_info *resp);

int curl_get_ipc_lpr_wildcards(MS_CURL_INFO *info, struct ms_lpr_wildcards *resp);
int curl_set_ipc_lpr_wildcards(MS_CURL_INFO *info, struct ms_lpr_wildcards *param);

int curl_get_ipc_vca_alllinecrossing(MS_CURL_INFO *info, struct ms_linecrossing_info2 *resp);
int curl_set_ipc_vca_alllinecrossing(MS_CURL_INFO *info, void *reqData);

int curl_get_ipc_vca_settings2(MS_CURL_INFO *info, struct ms_vca_settings_info2 *resp);
int curl_set_ipc_vca_settings2(MS_CURL_INFO *info, void *reqData);

int curl_get_ipc_watermark(MS_CURL_INFO *info, struct ms_water_mark *resp);
int curl_set_ipc_watermark(MS_CURL_INFO *info, struct ms_water_mark *param);

int curl_get_ipc_digitpos_zoom_state(MS_CURL_INFO *info, struct ms_digitpos_zoom_state *resp);

int curl_check_ipc_is_ptztype(const char *model);

int curl_set_ipc_ptz_speed(void *ovfhdl, int speed);
int curl_set_ipc_3dptz_control(void *ovfhdl, struct ms_3d_ptz_control_info *param);

int curl_set_ipc_ptz_fisheye_settour(void *ovfhdl, struct fisheye_ptz_tour *param);

int curl_get_ipc_ptz_fisheye_info(void *ovfhdl, int streamid, struct resp_fishmode_info *param);

int curl_set_ipc_ptz_fisheye_action(void *ovfhdl, struct fisheye_ptz_control *param);
int curl_get_ipc_fisheye_info(MS_CURL_INFO *info, struct resp_fishmode_param *resp);
int curl_set_ipc_fisheye_info(MS_CURL_INFO *info, struct resp_fishmode_param *param);
int curl_set_ipc_ptz_track_del(void *ovfhdl, int param);
int curl_get_ipc_ptz_track_info(void *ovfhdl, int resp[PATTERN_MAX]);
int curl_get_ipc_product_model(const char *model);

int curl_get_ipc_common_param(MS_CURL_INFO *info, struct http_common_param *param);
int curl_set_ipc_common_param(MS_CURL_INFO *info, struct http_common_param *param);

int curl_get_ipc_image_display_info(MS_CURL_INFO *info, struct image_display *resp);
int curl_set_ipc_image_display_info(MS_CURL_INFO *info, struct image_display *param);

int curl_get_ipc_image_enhancement_info(MS_CURL_INFO *info, struct image_enhancement *resp);
int curl_set_ipc_image_enhancement_info(MS_CURL_INFO *info, struct image_enhancement *param);

int curl_set_ipc_sub_videotype(MS_CURL_INFO *info, int profile, int videoType);
int curl_set_ipc_main_videotype(MS_CURL_INFO *info, int profile, int videoType);

int curl_get_ipc_main_videotype(void *ovfhdl);
int curl_get_ipc_sub_videotype(void *ovfhdl);

int curl_get_ipc_smart_stream(void *ovfhdl, int streamid);
int curl_set_ipc_smart_stream(MS_CURL_INFO *info, int profile, int streamtype);

int curl_get_ipc_smart_stream_level(void *ovfhdl, int streamid);
int curl_set_ipc_smart_stream_level(MS_CURL_INFO *info, int profile, int streamlevel);

int curl_get_ipc_videotype_enable(void *ovfhdl);
int curl_get_ipc_model_info(MS_CURL_INFO *info, CAM_MODEL_INFO *modelInfo);
int curl_get_ipc_model_type(const char *model);

int curl_get_ipc_audioenable(MS_CURL_INFO *info);
int curl_set_ipc_audioenable(MS_CURL_INFO *info, int param);

int curl_set_ipc_substream_enable(MS_CURL_INFO *info, int profile, int enable);

int curl_set_ipc_audioalarm(MS_CURL_INFO *info, int enable);
int curl_set_ipc_alarmstate(void *ovfhdl, int enable);

int curl_get_ipc_motion_map(void *ovfhdl, struct ipc_motion_map *resp);

int curl_get_ipc_event_stream_info(MS_CURL_INFO *info, struct event_stream_info *resp);
int curl_set_ipc_event_stream_info(MS_CURL_INFO *info, struct event_stream_info *param);

int curl_get_ipc_lpr_nightmode(MS_CURL_INFO *info, struct resp_lpr_night_mode *resp);
int curl_get_ipc_autoiris_status(MS_CURL_INFO *info, struct resp_ipc_autoiris_status *resp);

int curl_set_ipc_login_log(MS_CURL_INFO *info, struct resp_ipc_autoiris_status *resp);

int curl_get_ipc_system_info(void *ovfhdl, struct ipc_system_info *resp);
int curl_get_ipc_chipninterface(void *ovfhdl, struct ipc_system_info *resp);

int curl_get_ipc_audio(MS_CURL_INFO *info, struct ipc_audio_info *resp);
int curl_set_ipc_audio(MS_CURL_INFO *info, struct ipc_audio_config *param);

int curl_get_ipc_fish_viewfield(MS_CURL_INFO *info, int *resp);
int curl_set_ipc_fish_viewfield(MS_CURL_INFO *info, int *param);
int curl_get_fisheye_support(MS_CURL_INFO *info, int *resp);

int curl_get_ipc_fisheye_check(struct ms_req_ipc_info *reqs, struct ms_ipc_fisheye_info *resp);
int curl_set_ipc_fisheye_check(struct req_ipc_set_fisheye_info *reqs);

int get_fishinfo_conv_minorid(struct ms_ipc_fisheye_info *param, int chnStreamId);
int get_minorid_conv_fishinfo(int minorid, struct ms_ipc_fisheye_info *info);
int get_minorid_conv_channelId(int minorid);
int get_minorid_conv_transfer(int minorid);
int get_minorid_conv_display(int minorid);
int get_minorid_conv_installation(int minorid);
int get_minorid_conv_streamcnt(int minorid);

int curl_reqs_info_init(void *ovfhdl, MS_CURL_INFO *info);
int ovf_get_stream_number(const char *token);
int ovf_get_stream_number_ex(const char *token);

int curl_set_ipc_parameters(MS_CURL_INFO *info, REQ_SET_IPCPARAMEX *param);
int curl_set_ipc_anr_info(struct req_ipc_set_anr_active *reqs);
int curl_get_ipc_custom_param(MS_CURL_INFO *info, char *url, struct resp_ipc_custom_params *param);

int curl_get_ipc_frame_json(MS_CURL_INFO *info, struct resp_ipc_frame_resolution *param);
int curl_get_ipc_event_stream_info_safe(MS_CURL_INFO *info, struct event_stream_info *resp);

int curl_set_ipc_mssp_devive(char *ip, int port, char *username, char *password);

int curl_get_ipc_ptz_basic(MS_CURL_INFO *info, struct PtzBasicInfo *resp);
int curl_set_ipc_ptz_basic(MS_CURL_INFO *info, struct PtzBasicInfo *param);

int curl_get_ipc_regional_people(MS_CURL_INFO *info, struct MsIpcRegionalPeople *resp);
int curl_set_ipc_regional_people(MS_CURL_INFO *info, void *reqData);
int curl_get_ipc_people_report(MS_CURL_INFO *info, struct ReqIpcPeopleReport *req, MS_SOCKET_RESPONSE *resp, int line);
int curl_get_ipc_regional_detect_data(MS_CURL_INFO *info, struct MsIpcRegionalData *resp);

int curl_get_face_config(MS_CURL_INFO *info, MS_FACE_CONFIG *resp);
int curl_set_face_config(MS_CURL_INFO *info, MS_FACE_CONFIG *param);
int curl_get_face_support(MS_CURL_INFO *info, int *resp);

int curl_get_vca_base_info(MS_CURL_INFO *info, struct MsVcaBaseInfo *resp);
int curl_get_image_enhancement_sche(MS_CURL_INFO *info, struct EnhancementSchedule *resp);

int curl_set_ipc_reboot(MS_CURL_INFO *info);
int curl_set_ipc_reset(MS_CURL_INFO *info, Uint8 options);
int curl_get_ipc_diagnose(MS_CURL_INFO *info, FILE *writeFp);
int curl_get_ipc_cfg(MS_CURL_INFO *info, FILE_INFO_S *resp, const char *pwd);
int curl_set_ipc_cfg(MS_CURL_INFO *info, const char *data, const char *pwd);
int curl_get_ipc_log(MS_CURL_INFO *info, FILE_INFO_S *resp, const REQ_IPC_LOG_S *req);
int curl_get_ipc_import_status(MS_CURL_INFO *info, int *pStatus);
int curl_set_ipc_import_status(MS_CURL_INFO *info);

int curl_set_ipc_schedule(MS_CURL_INFO *info, IPC_SCHE_E type, char sche[MAX_DAY_NUM_IPC][IPC_SCHE_THREE_SECTIONS_STR_LEN + 1]);
int curl_get_fisheye_frame_json(MS_CURL_INFO *info, int minorId, struct resp_ipc_frame_resolution *param);
int curl_get_ipc_audio_alarm(MS_CURL_INFO *info, IPC_AUDIO_ALARM_S *resp);
int curl_set_ipc_audio_alarm(MS_CURL_INFO *info, void *data);
int curl_get_ipc_audio_alarm_sample(MS_CURL_INFO*info, int *pSample);
int curl_get_ipc_alarmstatus(MS_CURL_INFO *info, Uint64 *pStatus);
int curl_set_ipc_image_setting(MS_CURL_INFO *info, IPC_IMAGE_SETTING_S *req);
int curl_get_ipc_display_whiteled(MS_CURL_INFO *info, IPC_DISPLAY_WHITELED_S *resp);
int curl_set_ipc_display_whiteled(MS_CURL_INFO *info, IPC_DISPLAY_WHITELED_S *req);
int curl_get_ipc_ptz_wiper(MS_CURL_INFO *info, IPC_PTZ_WIPER_S *resp);
int curl_set_ipc_ptz_wiper(MS_CURL_INFO *info, void *data);
int curl_get_ipc_ptz_auto_home(MS_CURL_INFO *info, IPC_PTZ_AUTO_HOME_S *resp);
int curl_set_ipc_ptz_auto_home(MS_CURL_INFO *info, IPC_PTZ_AUTO_HOME_S *req);
int curl_set_ipc_ptz_initial_position(MS_CURL_INFO *info, IPC_PTZ_INITIAL_POSITION_S *req);
int curl_set_ipc_ptz_config_clear(MS_CURL_INFO *info, void *data);
int curl_get_ipc_ptz_limit(MS_CURL_INFO *info, IPC_PTZ_LIMIT_INFO_S *resp);
int curl_set_ipc_ptz_limit(MS_CURL_INFO *info, IPC_PTZ_LIMIT_CONTROL_S *req);
int curl_get_ipc_ptz_sche_task(MS_CURL_INFO *info, IPC_PTZ_SCHE_TASK_INFO_S *resp);
int curl_set_ipc_ptz_sche_task(MS_CURL_INFO *info, void *data);
int curl_set_ipc_ptz_control_json(MS_CURL_INFO *info, IPC_PTZ_CONTROL_S *req);
int curl_get_ipc_ptz_cap_info(MS_CURL_INFO *info, IPC_PTZ_CAP_INFO_S *resp);
int curl_get_ipc_global_support_info(void *ovfhdl, struct ipc_system_info *resp);
int curl_get_ipc_ptz_panel_status(MS_CURL_INFO *info, IPC_PTZ_PANEL_STATUS_S *resp);
int curl_get_ipc_general_video(MS_CURL_INFO *info, struct resp_get_ipc_param *param);
int curl_get_ipc_image_setting_json(MS_CURL_INFO *info, IPC_IMAGE_SETTING_S *param);
int curl_get_lpr_settings_json(MS_CURL_INFO *info, struct ms_lpr_settings *resp);
int curl_set_lpr_settings_json(MS_CURL_INFO *info, struct ms_lpr_settings *param);
int curl_get_ipc_cap_image(MS_CURL_INFO *info, MS_SOCKET_RESPONSE *data);
int curl_get_ipc_image_display_multi(MS_CURL_INFO *info, IMAGE_DISPLAY_MULTI_S *resp);
int curl_set_ipc_image_display_multi(MS_CURL_INFO *info, IMAGE_DISPLAY_MULTI_S *req);
int curl_get_ipc_image_enhancement_multi(MS_CURL_INFO *info, IMAGE_ENHANCEMENT_MULTI_S *resp);
int curl_set_ipc_image_enhancement_multi(MS_CURL_INFO *info, IMAGE_ENHANCEMENT_MULTI_S *req);
int curl_get_ipc_image_splicedistance(MS_CURL_INFO *info, IMAGE_SPILCEDISTANCE_S *resp);
int curl_set_ipc_image_splicedistance(MS_CURL_INFO *info, IMAGE_SPILCEDISTANCE_S *param);
int curl_get_ipc_image_daynight_multi(MS_CURL_INFO *info, IMAGE_DAYNIGHT_MULTI_S *resp);
int curl_set_ipc_image_daynight_multi(MS_CURL_INFO *info, IMAGE_DAYNIGHT_MULTI_S *param);
// request function declare end

#endif // __OVFCLI_H
