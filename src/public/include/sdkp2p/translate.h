#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include "msg.h"
#include "msdb.h"
#include "mssocket.h"
#include "log/log.h"
#include "msfs/msfs_log.h"
#include "msstd.h"
#include "cJSON.h"
#include "tsar_socket.h"
#include "msoem.h"


#define SDK_URL ""
#define MAX_SKDP2P_REQ_COUNT        512
#define MAX_SKDP2P_RESP_COUNT       MAX_SKDP2P_REQ_COUNT
#define SDKP2P_SESSIONID_INVALID    0xffffffff00000000


typedef void (*p2p_send_cb)(int peer, int msg, void* data, int datalen, UInt64 sessionId);

enum {
    SDKP2P_NEED_CALLBACK = 0,
    SDKP2P_NOT_CALLBACK,
};

enum {
    TRANSLATE_TYPE_P2P = 0,
    TRANSLATE_TYPE_SDK = 1
};

enum {
    SDKP2P_RESP_OK = 0,
    SDKP2P_RESP_ERROR = 1
};

struct req_conf_str
{
    int req;
    int response;
    char *resp;
    int size;
    int *len;
    p2p_send_cb cb;

    //login info
    char user[64];
    char ip_addr[64];
    int p2psid;
    UInt64 sessionId;

    //for resp
    int *resp_no;
    unsigned long long *req_usec;

    char reqUrl[128];
};

struct translate_request_p2p
{
    int req;
    void (*handler_req)(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf);//string(buf) --> struct(param) 

    int resp;
    void (*handler_resp)(void *param, int size, char *buf, int len, int *datalen);//struct(param) --> string(buf)
};

struct translate_request_sdk
{
    char url[128];
    void (*handler_req)(char *buf, int len, void *param, int *datalen, char *resp, int size, int *resp_len);//string(buf) --> struct(param) 

    int resp;
    void (*handler_resp)(void *param, int size, char *buf, int len, int *datalen);//struct(param) --> string(buf)
};


struct sdkp2p_request
{
    int type;//0=p2p, 1=sdk
    int request; // request No.
    int response; // response No.

    //string
    char *buf; // p2p request content
    int buf_size;//buf size
    int buf_len; //real buf len

    //struct
    void *param; // for @deal_special_p2p_req, storage data conversed from req string
    int param_size;
    int param_len;

    //resp
    char *resp;
    int resp_size;
    int resp_len;

    char user[64];
    char ip_addr[64];
    int p2psid;
    UInt64 sessionId;

    int *resp_no;
    unsigned long long *req_usec;
};

typedef struct Sdkp2pResp {
    int sid;
    int respNo;
    void *data; // storage data from mscore to p2p
    int dataLen;
} SDKP2P_RESP_S;

typedef struct Request {
    int reqNum;
    void *data;
    int len;
} REQUEST_S;

int sdkp2p_dispatch_request(int type, struct sdkp2p_request *req, p2p_send_cb cb);
int sdkp2p_dispatch_reponse(int from, int resp, void *param, int size, char *buf, int len, int *datalen);

//string to struct 
int p2p_request_register(struct translate_request_p2p* req, int count);
int p2p_request_unregister(struct translate_request_p2p* req, int count);

int sdk_request_register(struct translate_request_sdk* req, int count);
int sdk_request_unregister(struct translate_request_sdk* req, int count);

int sdkp2p_get_section_info(const char* pRawInfo, const char* pSection, char* pDataBuf, int nBufLen);
int sdkp2p_get_special_info(const char* pRawInfo, const char* pSection, char* pDataBuf, int nBufLen, char *key_word);

int sdkp2p_get_resp(int type, char *resp, int len);
void sdkp2p_send_msg(struct req_conf_str *conf, int msg, void *data, int datalen, int need_cb);
long long sdkp2p_get_chnmask_to_atoll(char chnmask[]);
void sdkp2p_common_write_log_batch(struct req_conf_str *conf, int main_type, int sub_type, int param_type, long long channels);
void sdkp2p_common_write_log(struct req_conf_str *conf, int main_type, int sub_type, int param_type, int chnid, int detail_type, void *detail_data, int detail_size);
int sdkp2p_get_schedule_by_buff(SMART_SCHEDULE *schedule, const char *buff, char *mask);
int sdkp2p_get_buff_by_schedule(SMART_SCHEDULE *schedule, char *buff, int len, char *mask);
long long sdkp2p_get_batch_by_buff(const char *buff, char *key);
Uint64 get_batch_by_json(cJSON *obj, const char *key);
int get_schedule_by_cjson(cJSON *obj, const char *key, SMART_SCHEDULE *schedule);
int get_http_params_by_cjson(int id, cJSON *obj, const char *key, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms);
int get_white_params_by_cjson(int chnId, cJSON *obj, char *key, WHITE_LED_PARAMS_EVTS *params);
int get_ptz_params_by_cjson(int id, EVENT_IN_TYPE_E type, cJSON *obj, char *key, void *pPtzPrms);

int p2p_format_json(char *buf);

void p2p_camera_load_module();
void p2p_camera_unload_module();

void p2p_disk_load_module();
void p2p_disk_unload_module();

void p2p_event_load_module();
void p2p_event_unload_module();

void p2p_network_load_module();
void p2p_network_unload_module();

void p2p_playback_load_module();
void p2p_playback_unload_module();

void p2p_ptz_load_module();
void p2p_ptz_unload_module();

void p2p_record_load_module();
void p2p_record_unload_module();

void p2p_status_load_module();
void p2p_status_unload_module();

void p2p_system_load_module();
void p2p_system_unload_module();

void p2p_user_load_module();
void p2p_user_unload_module();

void p2p_schedule_load_module();
void p2p_schedule_unload_module();

void p2p_qtctrl_load_module();
void p2p_qtctrl_unload_module();


int sdkp2p_init();
int sdkp2p_deinit();

int sdk_get_field_int(const char *info, const char *field, int *pVal);
int sdk_get_field_string(const char *info, const char *field, char *pVal, int size);

//json start

cJSON *get_action_audio_alarm(int chnId);
int set_action_audio_alarm(cJSON *json, int chnId, Uint64 chnMask);


void sdkp2p_json_string_free(char *data);
int sdkp2p_json_resp(char *resp, int len, SDK_ERROR_CODE code, int *result);
cJSON *get_json_schedule_object(struct smart_event_schedule *schedule);
cJSON *get_json_schedule_object_ex(struct alarm_in_schedule *schedule);
cJSON *get_json_schedule_object_ipc(struct smart_event_schedule *schedule);
cJSON *get_json_white_params_object(int chnId, const char *dbTable);
cJSON *get_json_http_params_object(int chnId, const char *dbTable);
cJSON *get_json_chn_alarmout_object(const char *triChnAlarms1, const char *triChnAlarms2);
cJSON *get_json_ptz_params_object(int type, int id);
cJSON *get_json_pcnt_white_params_object(int groupid);
int get_audio_file_id_mask_ex();
int get_common_num_json(char *fieldName, void *data, int dataLen, char *resp, int respLen);

//camera
int get_face_support_json(void *data, int dataLen, char *resp, int respLen);
int get_ai_cam_cnt_json(void *data, int dataLen, char *resp, int respLen);
int get_ipc_custom_param_json(void *data, int dataLen, char *resp, int respLen);
int get_image_enhancement_info_json(void *data, int dataLen, char *resp, int respLen);
int get_vca_base_info_json(void *data, int dataLen, char *resp, int respLen);
int get_camera_face_config_json(void *data, int dataLen, char *resp, int respLen, int action);
SDK_ERROR_CODE get_camera_face_config_str(const char *buff, MS_FACE_CONFIG *info);
SDK_ERROR_CODE get_regional_pcnt_str(const char *buff, struct MsIpcRegionalPeople *info);
int get_heatmap_support_json(void *data, int dataLen, char *resp, int respLen);

SDK_ERROR_CODE get_ipc_peoplecnt_linecnt_str(const char *buff, UInt64 *info);
int get_ipc_peoplecnt_linecnt_json(MS_PEOPLECNT_DATA *data, int cnt, UInt64 mask, char *resp, int respLen);
int get_ipc_file_data_json(void *data, int dataLen, char *resp, int respLen);
int set_vca_common_region_by_text(const char *req, struct ms_smart_event_info *param);
int set_vca_leftremove_region_by_text(const char *req, struct ms_smart_leftremove_info *param);
int get_vca_common_json(void *data, int dataLen, char *resp, int respLen, SMART_EVENT_TYPE type);
int get_vca_leftremove_json(void *data, int dataLen, char *resp, int respLen);
int sdk_resp_ipc_alarmstatus_to_json(char *dst, const int dstSize, void *src);
SDK_ERROR_CODE sdk_text_to_req_upgrade_camera(REQ_UPGRADE_CAMERA_S *dst, const char *src);
int sdk_resp_upgrade_camera_to_json(char *dst, const int dstSize, void *src);

//disk
int get_raid_mode_json(void *data, int dataLen, char *resp, int respLen);
int get_raid_component_size_json(void *data, int dataLen, char *resp, int respLen);
SDK_ERROR_CODE get_raid_req_info(const char *buff, REQ_RAID_T *info);
SDK_ERROR_CODE get_format_export_disk_str(const char *buff, struct req_usb_info *info);
int get_storage_info_json(void *data, int cnt, char *resp, int respLen);

//event
int get_http_notification_json(void *data, int dataLen, char *resp, int respLen);
int set_http_notification_json(void *data, int dataLen, char *resp, int respLen, REQUEST_S *pReq);
int get_pos_all_info_json(char *resp, int respLen);
int sdkp2p_get_ipc_audio_alarm(void *data, int dataLen, char *resp, int respLen);
SDK_ERROR_CODE sdkp2p_set_ipc_audio_alarm(const char *req, IPC_AUDIO_ALARM_S *param);
SDK_ERROR_CODE sdkp2p_get_ipc_audio_alarm_sample(void *data, int dataLen, char *resp, int respSize);


//network
int get_network_check_port_json(const char *data, char *resp, int respLen);


//playback
int get_face_backup_search_json(void *data, int dataLen, char *resp, int respLen);
int get_search_face_backup_close_json(void *data, int dataLen, char *resp, int respLen);
int get_face_backup_search_bigimg_json(void *data, int dataLen, char *resp, int respLen);
int get_playback_common_record_info_json(void *data, int cnt, char *resp, int respLen);
int get_playback_picture_record_info_json(void *data, int cnt, char *resp, int respLen);
int get_playback_common_base_info_json(void *data, int dataLen, char *resp, int respLen);
int get_playback_play_json(void *data, int cnt, char *resp, int respLen);
int get_playback_event_record_info_json(void *data, int cnt, char *resp, int respLen);
int get_playback_month_json(void *data, char *resp, int respLen);
int get_playback_event_base_info_json(void *data, int cnt, char *resp, int respLen);
SDK_ERROR_CODE sdk_str_to_req_pushmsg_pictures(REQ_PUSHMSG_PICTURES_S *dst, const char *src);

//ptz
SDK_ERROR_CODE get_ptz_cmd_info(const char *buff, struct req_ptz_action *info);

//qtctrl

//record
int get_export_disk_json(void *data, int cnt, char *resp, int respLen);
int get_export_auto_backup_json(char *resp, int respLen, int backupType);
SDK_ERROR_CODE get_export_auto_backup_str(const char *buff, int backupType);
int get_auto_backup_status_json(void *data, char *resp, int respLen);
int get_disk_format_json(void *data, char *resp, int respLen);



//schedule

//status
int get_poe_port_status_json(void *data, int cnt, char *resp, int respLen);


//system
SDK_ERROR_CODE get_system_environment_str(const char *buff, struct ReqEnvInfo *info);
int get_day_time_json(char *resp, int respLen);
SDK_ERROR_CODE set_hotspare_mode_by_text(const char *buff, struct ms_failover_change_mode *info);
int search_hotspare_masters_json(void *data, int cnt, char *resp, int respLen);
#if 0 // wcm delete
int init_hotspare_log(int action, const char *masterIp, const char *remoteIp, const char *user, struct log_data *pLogData);
#endif 
SDK_ERROR_CODE add_hotspare_masters_by_text(const char *buf, struct failover_list *info, 
    struct ms_failover_update_master *pUpdate, Uint32 *pMasterMask);
SDK_ERROR_CODE get_hotspare_masters_status_json(void *data, int cnt, char *resp, int respSize);
SDK_ERROR_CODE del_hotspare_masters_by_text(const char *buf, char ips[][MAX_LEN_16], int *pSum, Uint32 *pMask);
SDK_ERROR_CODE get_hotspare_slave_status_json(void *data, int dataSize, char *resp, int respSize);

int sdkp2p_check_chnid(int chnid);
int sdkp2p_chnmask_to_str(Uint64 chnMask, char *buf, int size);
int sdkp2p_req_common_get_chnid(const char *buf, int bufLen);
void sdkp2p_resp_common_set(void *param, int size, char *buf, int len, int *datalen);
MS_BOOL sdk_is_ms_lpr(IPC_AITYPE_E aitype);

//user

//json end

#endif

