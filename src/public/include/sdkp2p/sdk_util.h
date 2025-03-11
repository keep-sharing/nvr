#ifndef _MS_SDK_H_
#define _MS_SDK_H_

#include "translate.h"


#define SDK_COMMON_RESPONSE  "{\n\t\"url\":\"%s\",\n\t\"sdkversion\":\"%s\",\n\t\"responseCode\":%d,\n\t\"responseString\":\"%s\"\n}"
#define SDK_OK_STRING   "Success."

typedef enum SdkStringType {
    STRING_NONE  = 0,//没有限制
    STRING_EMPTY = 1,//不能为空字符
    STRING_MIN   = 2,//限制最小长度
    STRING_MAX   = 3,//限制最大长度
    STRING_RANGE = 4,//限制长度范围
} STRING_LIMIT_E;

typedef enum SdkParamsNessary {
    NESSARY_NO  = 0,//非必要字段
    NESSARY_YES = 1 //必要字段
} NESSARY_E;

typedef enum SdkNumLimit {
    NUM_NONE  = 0,//非必要字段
    NUM_MIN   = 1,//>=MIN
    NUM_MAX   = 2,//<=NAX
    NUM_RANGE = 3//>=MIN && <=MAX
} NUM_LIMIT_E;

typedef enum SdkParamsCode {
    //json转换/入参判断等相关错误
    PARAMS_ERR_OTHER    = -299,//入参错误
    PARAMS_ERR_PRINT    = -298,//JSON转字符串失败
    PARAMS_ERR_LENGTH   = -297,//resp缓存长度不足
    PARAMS_ERR_JSON     = -296,//字符串转JSON失败
    PARAMS_ERR_CREAT    = -295,//创建JSON对象失败
    PARAMS_ERR_CORE     = -294,//中心返回数据为空

    //字符串相关错误
    PARAMS_STR_EMPTY    = -199,//不支持空字符
    PARAMS_STR_MIN      = -198,//字符串长度最小限制
    PARAMS_STR_MAX      = -197,//字符串长度最大限制
    //TODO:
    PARAMS_STR_IP       = -196,//IP地址格式错误
    PARAMS_STR_CHARS    = -195,//禁止特殊符号

    //数字相关错误
    PARAMS_NUM_MIN      = -150,//数字最小值限制
    PARAMS_NUM_MAX      = -149,//数字最大值限制
    PARAMS_NUM_ENUM     = -148,//数字无效枚举值,用于分散的枚举，比如H264/H265

    //参数其他错误
    PARAMS_NECESSARY    = -129,//缺失必要字段
    PARAMS_FORMAT       = -128,//格式异常，数字和字符不一样
    //TODO:
    PARAMS_CONFLICT     = -127,//冲突：比如A等于1时B不能等于1、 A必须小于B、
    PARAMS_INVALID      = -126,//无效配置：比如区域点交叉

    //发送失败 --> 兼容旧版本回复值SDK_ERROR_CODE
    WAIT_TIMEOUT        = -2,//等待超时 @WAIT_TIME_OUT
    SEND_ERROR          = -1,//发给中心失败 @SEND_FAILED
    
    PARAMS_OK           = 0  //OK
} SDK_CODE_E;

int sdk_code_response(char *des, int len, char *url, char *key, SDK_CODE_E code, int min, int max);
int sdk_code_response_double(char *des, int len, char *url, char *key, SDK_CODE_E code, double min, double max);
SDK_CODE_E sdk_get_json_int(cJSON *json, char *key, int *des, NESSARY_E isNessary, NUM_LIMIT_E limitType, int min, int max, int defaultValue);
SDK_CODE_E sdk_get_json_string(cJSON *json, char *key, char *des, int len, NESSARY_E isNessary, STRING_LIMIT_E limitType, int min, int max, char *defaultValue);
SDK_CODE_E sdk_get_json_double(cJSON *json, char *key, double *des, NESSARY_E isNessary, NUM_LIMIT_E limitType, 
    double min, double max, double defaultValue);

#define sdk_ok(reqUrl, resp, respLen) sdk_code_response(resp, respLen, reqUrl, "", PARAMS_OK, 0, 0);
#define sdk_err(reqUrl, resp, respLen, errCode) sdk_code_response(resp, respLen, reqUrl, "", errCode, 0, 0);


/*以下接口用于json转struct*/
//字符串转json，声明必须是 jsonBuff, resp, respLen
#define sdk_request_start() \
    SDK_CODE_E errCode = PARAMS_OK;\
    cJSON *reqJson = cJSON_Parse(jsonBuff);\
    do{\
        if (!reqJson) { \
            sdk_code_response(resp, respLen, reqUrl, "", PARAMS_ERR_JSON, 0, 0);\
            return -1; \
        } \
    }while(0)

//结合 sdk_request_start 使用，释放msJson
#define sdk_request_end() cJSON_Delete(reqJson)


#define sdk_get_int(json, key, des, isNessary, limitType, min, max) \
    do{\
        errCode = sdk_get_json_int(json, key, des, isNessary, limitType, min, max, MS_INVALID_VALUE); \
        if (errCode != PARAMS_OK) { \
            if (reqJson != NULL) { \
                cJSON_Delete(reqJson); \
            } \
            sdk_code_response(resp, respLen, reqUrl, key, errCode, min, max);\
            return -1; \
        } \
    }while(0)

#define sdk_get_string(json, key, des, isNessary, limitType, min, max) \
    do{\
        errCode = sdk_get_json_string(json, key, des, sizeof(des), isNessary, limitType, min, max, MS_INVALID_STRING); \
        if (errCode != PARAMS_OK) { \
            if (reqJson != NULL) { \
                cJSON_Delete(reqJson); \
            } \
            sdk_code_response(resp, respLen, reqUrl, key, errCode, min, max);\
            return -1; \
        } \
    }while(0)

#define sdk_get_json(src, des, key, isNessary) \
    do{\
        des = cJSON_GetObjectItem(src, key);\
        if (isNessary && !des) {\
            sdk_request_end();\
            sdk_code_response(resp, respLen, reqUrl, key, PARAMS_NECESSARY, min, max);\
            return -1; \
        }\
    }while(0)

#define sdk_get_int64(json, key, des, isNessary, limitType, min, max) \
    do{\
        char tmp[65] = {0};\
        errCode = sdk_get_json_string(json, key, tmp, sizeof(tmp), isNessary, limitType, min, max, MS_INVALID_STRING); \
        if (errCode != PARAMS_OK) { \
            if (reqJson != NULL) { \
                cJSON_Delete(reqJson); \
            } \
            sdk_code_response(resp, respLen, reqUrl, key, errCode, min, max);\
            return -1; \
        } \
        *des = sdkp2p_get_chnmask_to_atoll(tmp);\
    }while(0)

#define sdk_get_double(json, key, des, isNessary, limitType, min, max) \
    do{\
        errCode = sdk_get_json_double(json, key, des, isNessary, limitType, min, max, MS_INVALID_VALUE); \
        if (errCode != PARAMS_OK) { \
            if (reqJson != NULL) { \
                cJSON_Delete(reqJson); \
            } \
            sdk_code_response_double(resp, respLen, reqUrl, key, errCode, min, max);\
            return -1; \
        } \
    }while(0)

/*以下接口用于struct转json*/
#define sdk_response_start() \
    int ret = 0;\
    cJSON *respJson = NULL;\
    cJSON *dataJson = NULL;\
    do{\
        respJson = cJSON_CreateObject();\
        if (!respJson) {\
            sdk_code_response(resp, respLen, reqUrl, "", PARAMS_ERR_CREAT, 0, 0);\
            return -1;\
        }\
        cJSON_AddStringToObject(respJson, "url", reqUrl);\
        cJSON_AddStringToObject(respJson, "sdkversion", "v1.0");\
        cJSON_AddNumberToObject(respJson, "responseCode", PARAMS_OK);\
        cJSON_AddStringToObject(respJson, "responseString", SDK_OK_STRING);\
        dataJson = cJSON_AddObjectToObject(respJson, "data");\
        if (!dataJson) {\
            cJSON_Delete(respJson);\
            sdk_code_response(resp, respLen, reqUrl, "", PARAMS_ERR_CREAT, 0, 0);\
            return -1;\
        }\
    }while(0)

#define sdk_response_end() \
    do{\
        char *_respPrint = cJSON_Print(respJson);\
        cJSON_Delete(respJson);\
        if (!_respPrint) {\
            sdk_code_response(resp, respLen, reqUrl, "", PARAMS_ERR_PRINT, 0, 0);\
            ret = -1;\
        } else if (strlen(_respPrint) >= respLen) {\
            cJSON_free(_respPrint);\
            ms_log(TSAR_WARN, "needSize:%d buffSize:%d", strlen(_respPrint), respLen);\
            sdk_code_response(resp, respLen, reqUrl, "", PARAMS_ERR_LENGTH, 0, 0);\
            ret = -1;\
        } else {\
            snprintf(resp, respLen, "%s", _respPrint);\
            cJSON_free(_respPrint);\
            ret = 0;\
        }\
    }while(0)

#define sdk_set_int(json, key, value) \
    do{\
        if (value != MS_INVALID_VALUE) { \
            cJSON_AddNumberToObject(json, key, value);\
        } \
    }while(0)

#define sdk_set_string(json, key, value) \
    do{\
        if (value && strcmp(value, MS_INVALID_STRING)) { \
            cJSON_AddStringToObject(json, key, value);\
        } \
    }while(0)

#define sdk_set_int64(json, key, value) \
    do{\
        char tmp[65] = {0};\
        if (value != MS_INVALID_VALUE) { \
            sdkp2p_chnmask_to_str(value, tmp, sizeof(tmp));\
            cJSON_AddStringToObject(json, key, tmp);\
        } \
    }while(0)

void req_common_chnid_json(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf);
void req_common_chnmask_json(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf);
void resp_common_set_json(void *param, int size, char *buf, int len, int *datalen);

/*
接口命名规范
请求字符串转json：ware_模块名称_接口描述_in，函数入参声明reqUrl, jsonBuff, resp, respLen不能变更名称，宏定义已经固定
回复结构体转json：ware_模块名称_接口描述_out，
模块对应urlxxx.c 文件名称，比如urlCamera.c则模块名称就用camera
接口描述依照简单的动宾格式，比如set_xxx\get_xxx等
*/

//common
int ware_common_chnid_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, int *chnid, char *key);
int ware_common_int_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_common_json_schedule_out(cJSON *json, IPC_PTZ_SCHE_TASK_WEEK_S *schedule);
int ware_common_json_schedule_in(cJSON *json, IPC_PTZ_SCHE_TASK_WEEK_S *schedule);
int ware_common_chnmask_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, Uint64 *chnMask, char *key);
//qtctrl \ test

// camera
int ware_camera_get_ipc_param_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_camera_set_ipc_param_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, REQ_SET_IPCPARAMEX *param);
int ware_camera_get_ipc_ptz_wiper_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_camera_set_ipc_ptz_wiper_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_WIPER_S *param);

int ware_camera_get_ipc_ptz_auto_home_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_camera_set_ipc_ptz_auto_home_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_AUTO_HOME_S *param);

int ware_camera_set_ipc_ptz_initial_position_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen,  IPC_PTZ_INITIAL_POSITION_S *param);

int ware_camera_set_ipc_ptz_config_clear_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen,  IPC_PTZ_CONFIG_CLEAR_S *param);

int ware_camera_get_ipc_ptz_limit_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_camera_set_ipc_ptz_limit_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_LIMIT_CONTROL_S *param);

int ware_camera_get_ipc_ptz_schedule_task_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_camera_set_ipc_ptz_schedule_task_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_SCHE_TASK_INFO_S *param);

int ware_camera_set_ipc_ptz_control_json_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_CONTROL_S *param);

int ware_camera_get_ipc_ptz_panel_status_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_camera_get_ipc_cap_image_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);

int ware_camera_get_ipc_image_splicedistance_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_camera_set_ipc_image_splicedistance_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IMAGE_SPILCEDISTANCE_S *param);
// camera request function declare end

// disk
int ware_camera_get_disk_health_management_data_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, int *param);
int ware_camera_get_disk_health_management_data_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);

int ware_camera_get_disk_health_management_log_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);

int ware_camera_set_disk_health_management_data_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, disk_health_data_t *param);
// disk request function declare end

// event
// event request function declare end

// network
int ware_network_get_general_cb(char *reqUrl, char *resp, int respLen);
// network request function declare end


// playback
// playback request function declare end
int ware_playback_picture_search_index_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
int ware_playback_picture_search_close_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
// ptz
// ptz request function declare end

// record
// record request function declare end

// schedule
// schedule request function declare end

// status
// status request function declare end


// system
int ware_system_get_online_upgrade_info_out(char *reqUrl, void *data, int cnt, char *resp, int respLen);
// system request function declare end



#endif

