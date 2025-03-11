#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sdk_util.h"
#include <time.h>

#define sdk_int_default(a, b) a = (a == MS_INVALID_VALUE ? b : a)

#define sdk_judge_null(a, b) \
    do{\
        if (!a || !b) { \
            sdk_err(reqUrl, resp, respLen, PARAMS_ERR_CORE);\
            return -1;\
        } \
    }while(0)

extern int g_p2pCurRespId;

void req_common_chnid_json(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

void req_common_chnmask_json(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    Uint64 chnMask = 0;
    if (ware_common_chnmask_in(conf->reqUrl, buf, conf->resp, conf->size, &chnMask, "chnMask")) {
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnMask, sizeof(Uint64), SDKP2P_NEED_CALLBACK);
    return;
}

void resp_common_set_json(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", g_p2pCurRespId - 5000);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

int sdk_code_response(char *des, int len, char *url, char *key, SDK_CODE_E code, int min, int max)
{
    char responseString[128] = {0};
    
    if (!des || !len || !key) {
        return -1;
    }

    switch (code) {
        case PARAMS_ERR_CORE:
            snprintf(responseString, sizeof(responseString), "core response is null.");
            break;
    
        case PARAMS_STR_EMPTY:   //不支持空字符
            snprintf(responseString, sizeof(responseString), "%s can not be empty.", key);
            break;
    
        case PARAMS_STR_MIN:     //字符串长度最小限制
            snprintf(responseString, sizeof(responseString), "The length of %s should be longer than %d", key, min);
            break;

        case PARAMS_STR_MAX:     //字符串长度最大限制
            snprintf(responseString, sizeof(responseString), "The length of %s cannot be longer than %d", key, max);
            break;

        case PARAMS_NUM_MIN:     //数字最小值限制
            snprintf(responseString, sizeof(responseString), "%s cannot smaller than %d", key, min);
            break;
        
        case PARAMS_NUM_MAX:     //数字最大值限制
            snprintf(responseString, sizeof(responseString), "%s cannot greater than %d", key, max);
            break;
        
        case PARAMS_NUM_ENUM:
            snprintf(responseString, sizeof(responseString), "%s value invalid.", key);
            break;
        
        case PARAMS_NECESSARY:   //缺失必要字段
            snprintf(responseString, sizeof(responseString), "%s are required.", key);
            break;
        
        case PARAMS_FORMAT:      //格式异常，数字和字符不一样
            snprintf(responseString, sizeof(responseString), "%s format error.", key);
            break;

        case WAIT_TIMEOUT:
            snprintf(responseString, sizeof(responseString), "%s", "Wait core response timeout.");
            break;
        
        case SEND_ERROR:
            snprintf(responseString, sizeof(responseString), "%s", "Send to core failed.");
            break;
        
        case PARAMS_OK:         //= 0 //OK
            snprintf(responseString, sizeof(responseString), "%s", SDK_OK_STRING);
            break;
        
        default:
            snprintf(responseString, sizeof(responseString), "%s", "Other error.");
            break;
    }

    snprintf(des, len, SDK_COMMON_RESPONSE, url ? url : "", "v1.0",  code, responseString);
    return 0;
}

int sdk_code_response_double(char *des, int len, char *url, char *key, SDK_CODE_E code, double min, double max)
{
    char responseString[128] = {0};
    
    if (!des || !len || !key) {
        return -1;
    }

    switch (code) {
        case PARAMS_ERR_CORE:
            snprintf(responseString, sizeof(responseString), "core response is null.");
            break;
    
        case PARAMS_STR_EMPTY:   //不支持空字符
            snprintf(responseString, sizeof(responseString), "%s can not be empty.", key);
            break;
    
        case PARAMS_STR_MIN:     //字符串长度最小限制
            snprintf(responseString, sizeof(responseString), "The length of %s should be longer than %f", key, min);
            break;

        case PARAMS_STR_MAX:     //字符串长度最大限制
            snprintf(responseString, sizeof(responseString), "The length of %s cannot be longer than %f", key, max);
            break;

        case PARAMS_NUM_MIN:     //数字最小值限制
            snprintf(responseString, sizeof(responseString), "%s cannot smaller than %f", key, min);
            break;
        
        case PARAMS_NUM_MAX:     //数字最大值限制
            snprintf(responseString, sizeof(responseString), "%s cannot greater than %f", key, max);
            break;
        
        case PARAMS_NUM_ENUM:
            snprintf(responseString, sizeof(responseString), "%s value invalid.", key);
            break;
        
        case PARAMS_NECESSARY:   //缺失必要字段
            snprintf(responseString, sizeof(responseString), "%s are required.", key);
            break;
        
        case PARAMS_FORMAT:      //格式异常，数字和字符不一样
            snprintf(responseString, sizeof(responseString), "%s format error.", key);
            break;

        case WAIT_TIMEOUT:
            snprintf(responseString, sizeof(responseString), "%s", "Wait core response timeout.");
            break;
        
        case SEND_ERROR:
            snprintf(responseString, sizeof(responseString), "%s", "Send to core failed.");
            break;
        
        case PARAMS_OK:         //= 0 //OK
            snprintf(responseString, sizeof(responseString), "%s", SDK_OK_STRING);
            break;
        
        default:
            snprintf(responseString, sizeof(responseString), "%s", "Other error.");
            break;
    }

    snprintf(des, len, SDK_COMMON_RESPONSE, url ? url : "", "v1.0",  code, responseString);
    return 0;
}

SDK_CODE_E sdk_get_json_int(cJSON *json, char *key, int *des, NESSARY_E isNessary, NUM_LIMIT_E limitType, int min, int max, int defaultValue)
{
    cJSON *obj;
    
    if (!key || !des) {
        return PARAMS_ERR_OTHER;
    }

    *des = defaultValue;
    obj = cJSON_GetObjectItemCaseSensitive(json, key); 
    if(!obj) {
        if (isNessary == NESSARY_YES) {
           return PARAMS_NECESSARY;
        }
        return PARAMS_OK;
    }

    if (obj->type != cJSON_Number) {
        return PARAMS_FORMAT;
    }
    if (limitType == NUM_MIN && obj->valueint < min) {
        return PARAMS_NUM_MIN;
    } else if (limitType == NUM_MAX && obj->valueint > max) {
        return PARAMS_NUM_MAX;
    } else if (limitType == NUM_RANGE && obj->valueint < min) {
        return PARAMS_NUM_MIN;
    } else if (limitType == NUM_RANGE && obj->valueint > max) {
        return PARAMS_NUM_MAX;
    }

    *des = obj->valueint;
    return PARAMS_OK;
}

SDK_CODE_E sdk_get_json_string(cJSON *json, char *key, char *des, int len, NESSARY_E isNessary, STRING_LIMIT_E limitType, int min, int max, char *defaultValue)
{
    cJSON *obj;
    int valueLen;
    
    if (!key || !des || !len) {
        return PARAMS_ERR_OTHER;
    }

    snprintf(des, len, "%s", MS_INVALID_STRING);
    obj = cJSON_GetObjectItemCaseSensitive(json, key); 
    if(!obj) {
        if (isNessary == NESSARY_YES) {
            return PARAMS_NECESSARY;
        }
        return PARAMS_OK;
    }

    if (obj->type != cJSON_String) {
        return PARAMS_FORMAT;
    }
    
    if (!obj->valuestring) {
        return PARAMS_NECESSARY;
    }

    valueLen = strlen(obj->valuestring);
    if (limitType == STRING_EMPTY && !valueLen) {
        return PARAMS_STR_EMPTY;
    } else if (limitType  == STRING_MIN && valueLen < min) {
        return PARAMS_STR_MIN;
    } else if (limitType  == STRING_MAX && valueLen > max) {
        return PARAMS_STR_MAX;
    } else if (limitType  == STRING_RANGE && valueLen < min) {
        return PARAMS_STR_MIN;
    } else if (limitType  == STRING_RANGE && valueLen > max) {
        return PARAMS_STR_MAX;
    }
    
    snprintf(des, len, "%s", obj->valuestring);
    return PARAMS_OK;
}

SDK_CODE_E sdk_get_json_double(cJSON *json, char *key, double *des, NESSARY_E isNessary, NUM_LIMIT_E limitType, 
    double min, double max, double defaultValue)
{
    cJSON *obj;
    
    if (!key || !des) {
        return PARAMS_ERR_OTHER;
    }

    *des = defaultValue;
    obj = cJSON_GetObjectItemCaseSensitive(json, key); 
    if(!obj) {
        if (isNessary == NESSARY_YES) {
            return PARAMS_NECESSARY;
        }
        return PARAMS_OK;
    }

    if (obj->type != cJSON_Number) {
        return PARAMS_FORMAT;
    }
    if (limitType == NUM_MIN && obj->valuedouble < min) {
        return PARAMS_NUM_MIN;
    } else if (limitType == NUM_MAX && obj->valuedouble > max) {
        return PARAMS_NUM_MAX;
    } else if (limitType == NUM_RANGE && obj->valuedouble < min) {
        return PARAMS_NUM_MIN;
    } else if (limitType == NUM_RANGE && obj->valuedouble > max) {
        return PARAMS_NUM_MAX;
    }

    *des = obj->valuedouble;
    return PARAMS_OK;
}

int ware_common_chnid_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, int *chnid, char *key)
{
    sdk_request_start();
    if (key) {
        sdk_get_int(reqJson, key, chnid, NESSARY_YES, NUM_RANGE, 0, (MAX_REAL_CAMERA-1));
    } else {
        sdk_get_int(reqJson, "chnid", chnid, NESSARY_YES, NUM_RANGE, 0, (MAX_REAL_CAMERA-1));
    }
    sdk_request_end();
    sdk_ok(reqUrl, resp, respLen);
    return 0;
}

int ware_common_chnmask_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, Uint64 *chnMask, char *key)
{
    sdk_request_start();
    char tmp[MAX_REAL_CAMERA + 1] = {0};
    sdk_get_string(reqJson, key ? key : "chnMask", tmp, NESSARY_YES, STRING_RANGE, 1, MAX_REAL_CAMERA);
    *chnMask = (Uint64)sdkp2p_get_chnmask_to_atoll(tmp);
    if (!*chnMask) {
        sdk_err(reqUrl, resp, respLen, PARAMS_STR_CHARS);
        return -1;
    }
    sdk_request_end();
    sdk_ok(reqUrl, resp, respLen);
    return 0;
}


int ware_common_int_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    int *res = (int *)data;
    
    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "res", *res);
    sdk_response_end();
    return ret;
}

int ware_common_json_schedule_out(cJSON *json, IPC_PTZ_SCHE_TASK_WEEK_S *schedule)
{
    int i, j;
    cJSON *array, *day, *obj;
    char startStr[6] = {0};
    char endStr[6] = {0};

    array = cJSON_CreateArray();
    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        day = cJSON_CreateArray();
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (!schedule->scheDay[i].scheItem[j].isValid) {
                break;
            }
            obj = cJSON_CreateArray();
            snprintf(startStr, sizeof(startStr), "%02d:%02d",schedule->scheDay[i].scheItem[j].startHour, schedule->scheDay[i].scheItem[j].startMin);
            snprintf(endStr, sizeof(endStr), "%02d:%02d",schedule->scheDay[i].scheItem[j].endHour, schedule->scheDay[i].scheItem[j].endMin);
            cJSON_AddItemToArray(obj, cJSON_CreateString(startStr));
            cJSON_AddItemToArray(obj, cJSON_CreateString(endStr));
            cJSON_AddItemToArray(obj, cJSON_CreateNumber(schedule->scheDay[i].scheItem[j].type));
            cJSON_AddItemToArray(obj, cJSON_CreateNumber(schedule->scheDay[i].scheItem[j].templateId));
            cJSON_AddItemToArray(day, obj);
        }
        cJSON_AddItemToArray(array, day);
    }

    cJSON_AddItemToObject(json, "sche", array);
    return 1;
}

int ware_common_json_schedule_in(cJSON *json, IPC_PTZ_SCHE_TASK_WEEK_S *schedule)
{
    int i = 0, j = 0;
    int dayCnt,itemCnt;
    cJSON *dayArr, *itemArr;

    if (!json || !schedule) {
        return 0;
    }
    memset(schedule, 0, sizeof(IPC_PTZ_SCHE_TASK_WEEK_S));
    dayCnt = cJSON_GetArraySize(json);
    for (i = 0; i < MAX_DAY_NUM_IPC && i < dayCnt; ++i) {
        dayArr = cJSON_GetArrayItem(json, i);
        itemCnt = cJSON_GetArraySize(dayArr);
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY && j < itemCnt; ++j) {
            itemArr = cJSON_GetArrayItem(dayArr, j);
            sscanf(cJSON_GetArrayItem(itemArr, 0)->valuestring, "%d:%d", &schedule->scheDay[i].scheItem[j].startHour, &schedule->scheDay[i].scheItem[j].startMin);
            sscanf(cJSON_GetArrayItem(itemArr, 1)->valuestring, "%d:%d", &schedule->scheDay[i].scheItem[j].endHour, &schedule->scheDay[i].scheItem[j].endMin);
            schedule->scheDay[i].scheItem[j].type = cJSON_GetArrayItem(itemArr, 2)->valueint;
            schedule->scheDay[i].scheItem[j].templateId = cJSON_GetArrayItem(itemArr, 3)->valueint;
            schedule->scheDay[i].scheItem[j].isValid = 1;
        }
    }
    return 1;
}

int ware_camera_get_ipc_param_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    cJSON *arraJson;
    cJSON *item;
    struct osd osd;
    int i;
    struct resp_get_ipc_param *info = (struct resp_get_ipc_param *)(data);

    sdk_judge_null(data, cnt);
    memset(&osd, 0, sizeof(struct osd));
    read_osd(SQLITE_FILE_NAME, &osd, info->chnid);
    sdk_response_start();
    sdk_set_int(dataJson, "chnid", info->chnid);
    sdk_set_int(dataJson, "ret", info->ret);
    sdk_set_string(dataJson, "name", osd.name);
    sdk_set_int(dataJson, "sub_enable", info->sub_enable);
    sdk_set_int(dataJson, "audio_enable", info->audio_enable);
    sdk_set_int(dataJson, "sub_stream", info->sub_stream);
    sdk_set_int(dataJson, "mainStream", info->mainStream);
    sdk_set_int(dataJson, "videoType_enable", info->videoType_enable);
    sdk_set_int(dataJson, "ipc_type", info->ipc_type);
    
    //main
    arraJson = cJSON_AddArrayToObject(dataJson, "mainResolution");
    for (i = 0; i < sizeof(info->main_range.res) / sizeof(struct ipc_resolution); i++) {
        item = cJSON_CreateObject();
        sdk_set_int(item, "width", info->main_range.res[i].width);
        sdk_set_int(item, "height", info->main_range.res[i].height);
        cJSON_AddItemToArray(arraJson, item);
    }
    sdk_set_int(dataJson, "m_framerate_max", info->main_range.framerate_max);
    sdk_set_int(dataJson, "m_framerate_min", info->main_range.framerate_min);
    sdk_set_int(dataJson, "m_bitrate_max", info->main_range.bitrate_max);
    sdk_set_int(dataJson, "m_bitrate_min", info->main_range.bitrate_min);
    sdk_set_int(dataJson, "m_vcodec_type", info->main_range.vcodec_type);
    sdk_set_int(dataJson, "m_cur_res.width", info->main_range.cur_res.width);
    sdk_set_int(dataJson, "m_cur_res.height", info->main_range.cur_res.height);
    sdk_set_int(dataJson, "m_framerate", info->main_range.framerate);
    sdk_set_int(dataJson, "m_bitrate", info->main_range.bitrate);
    sdk_set_int(dataJson, "m_iframeinterval", info->main_range.iframeinterval);
    
    //sub
    arraJson = cJSON_AddArrayToObject(dataJson, "subResolution");
    for (i = 0; i < sizeof(info->sub_range.res) / sizeof(struct ipc_resolution); i++) {
        item = cJSON_CreateObject();
        sdk_set_int(item, "width", info->sub_range.res[i].width);
        sdk_set_int(item, "height", info->sub_range.res[i].height);
        cJSON_AddItemToArray(arraJson, item);
    }
    sdk_set_int(dataJson, "s_framerate_max", info->sub_range.framerate_max);
    sdk_set_int(dataJson, "s_framerate_min", info->sub_range.framerate_min);
    sdk_set_int(dataJson, "s_bitrate_max", info->sub_range.bitrate_max);
    sdk_set_int(dataJson, "s_bitrate_min", info->sub_range.bitrate_min);
    sdk_set_int(dataJson, "s_vcodec_type", info->sub_range.vcodec_type);
    sdk_set_int(dataJson, "s_cur_res.width", info->sub_range.cur_res.width);
    sdk_set_int(dataJson, "s_cur_res.height", info->sub_range.cur_res.height);
    sdk_set_int(dataJson, "s_framerate", info->sub_range.framerate);
    sdk_set_int(dataJson, "s_bitrate", info->sub_range.bitrate);
    sdk_set_int(dataJson, "s_iframeinterval", info->sub_range.iframeinterval);
    sdk_set_int(dataJson, "SmartStreamSupport", info->supportsmartstream);
    sdk_set_int(dataJson, "SmartStream", info->smartStream);
    sdk_set_int(dataJson, "SmartStreamLevel", info->smartStream_level);
    sdk_set_int(dataJson, "SubSmartStreamSupport", info->subSupportSmartStream);
    sdk_set_int(dataJson, "SubSmartStream", info->subSmartStream);
    sdk_set_int(dataJson, "SubSmartStreamLevel", info->subSmartStream_level);
    sdk_response_end();
    return ret;
}

int ware_camera_set_ipc_param_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, REQ_SET_IPCPARAMEX *param)
{
    int chnid = -1;
    int i = 0;
    REQ_IPC_SUBPARAMS *mainParams;
    REQ_IPC_SUBPARAMS *subParams;

    sdk_request_start();
    memset(param, 0, sizeof(REQ_SET_IPCPARAMEX));
    mainParams = &param->stream[STREAM_TYPE_MAINSTREAM];
    subParams = &param->stream[STREAM_TYPE_SUBSTREAM];
    sdk_get_int(reqJson, "chnid", &chnid, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    for (i = 0; i < MAX_CAMERA; i++) {
        param->chnid[i] = '0';
    }
    param->chnid[chnid] = '1';
    sdk_get_int(reqJson, "transProtocol", &param->trans_protocol, NESSARY_NO, NUM_NONE, 0, 0);
    if (param->trans_protocol == MS_INVALID_VALUE) {
        param->trans_protocol = -1;
    }
    sdk_get_int(reqJson, "syncTime", &param->sync_time, NESSARY_NO, NUM_NONE, 0, 0);
    if (param->sync_time == MS_INVALID_VALUE) {
        param->sync_time = -1;
    }
    
    sdk_get_int(reqJson, "audioEnable", &param->audio_enable, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_int_default(param->audio_enable, -1);
    param->videoType_enable = 1;     //-1. nothing  | 0 /1... todo
    sdk_get_int(reqJson, "subEnable", &param->subEnable, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_int_default(param->subEnable, -1);
    sdk_get_int(reqJson, "fisheyeType", &param->fisheyeType, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_int_default(param->fisheyeType, -1);

    //main stream
    sdk_get_int(reqJson, "mainVideoType", &mainParams->codec, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->codec, -1);
    if (mainParams->codec != MS_INVALID_VALUE && mainParams->codec != CODECTYPE_H264 && mainParams->codec != CODECTYPE_H265) {
        sdk_code_response(resp, respLen, "", "mainVideoType", PARAMS_NUM_ENUM, 0, 0);
        sdk_request_end();
        return -1;
    }
    sdk_get_int(reqJson, "mainResWidth", &mainParams->width, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->width, -1);
    sdk_get_int(reqJson, "mainResHeight", &mainParams->height, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->height, -1);
    sdk_get_int(reqJson, "mainFramerate", &mainParams->framerate, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->framerate, -1);
    sdk_get_int(reqJson, "mainBitrate", &mainParams->bitrate, NESSARY_NO, NUM_RANGE, 16, 16384);
    sdk_int_default(mainParams->bitrate, -1);
    sdk_get_int(reqJson, "mainIframeInterval", &mainParams->iframeinterval, NESSARY_NO, NUM_RANGE, 1, 240);
    sdk_int_default(mainParams->iframeinterval, -1);
    sdk_get_int(reqJson, "mainRateControl", &mainParams->ratecontrol, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_int_default(mainParams->ratecontrol, -1);
    sdk_get_int(reqJson, "SmartStream", &mainParams->smartstream, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_int_default(mainParams->smartstream, -1);
    sdk_get_int(reqJson, "SmartStreamLevel", &mainParams->smartstreamlevel, NESSARY_NO, NUM_RANGE, 1, 10);
    sdk_int_default(mainParams->smartstreamlevel, -1);
    sdk_get_int(reqJson, "recordtype", &mainParams->recordtype, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->recordtype, -1);
    sdk_get_int(reqJson, "etframerate", &mainParams->etframerate, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->etframerate, -1);
    sdk_get_int(reqJson, "etIframeinterval", &mainParams->etIframeinterval, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->etIframeinterval, -1);
    sdk_get_int(reqJson, "etbitrate", &mainParams->etbitrate, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(mainParams->etbitrate, -1);

    //sub stream
    sdk_get_int(reqJson, "subVideoType", &subParams->codec, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(subParams->codec, -1);
    if (param->subEnable > 0 && subParams->codec != MS_INVALID_VALUE && subParams->codec != CODECTYPE_H264 && subParams->codec != CODECTYPE_H265) {
        sdk_code_response(resp, respLen, "", "subVideoType", PARAMS_NUM_ENUM, 0, 0);
        sdk_request_end();
        return -1;
    }
    sdk_get_int(reqJson, "subResWidth", &subParams->width, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(subParams->width, -1);
    sdk_get_int(reqJson, "subResHeight", &subParams->height, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(subParams->height, -1);
    sdk_get_int(reqJson, "subFramerate", &subParams->framerate, NESSARY_NO, NUM_NONE, 0, 0);
    sdk_int_default(subParams->framerate, -1);
    sdk_get_int(reqJson, "subBitrate", &subParams->bitrate, NESSARY_NO, NUM_RANGE, 16, 16384);
    sdk_int_default(subParams->bitrate, -1);
    sdk_get_int(reqJson, "subIframeInterval", &subParams->iframeinterval, NESSARY_NO, NUM_RANGE, 1, 240);
    sdk_int_default(subParams->iframeinterval, -1);
    sdk_get_int(reqJson, "subRateControl", &subParams->ratecontrol, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_int_default(subParams->ratecontrol, -1);
    sdk_get_int(reqJson, "SubSmartStream", &subParams->smartstream, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_int_default(subParams->smartstream, -1);
    sdk_get_int(reqJson, "SubSmartStreamLevel", &subParams->smartstreamlevel, NESSARY_NO, NUM_NONE, 1, 10);
    sdk_int_default(subParams->smartstreamlevel, -1);
    subParams->recordtype = -1;
    subParams->etframerate = -1;
    subParams->etIframeinterval = -1;
    subParams->etbitrate = -1;
    sdk_request_end();
    return 0;
}



int ware_network_get_general_cb(char *reqUrl, char *resp, int respLen)
{
    struct network net;

    sdk_response_start();
    memset(&net, 0, sizeof(struct network));
    read_network(SQLITE_FILE_NAME, &net);
    sdk_set_int(dataJson, "mode", net.mode);
    sdk_set_string(dataJson, "host_name", net.host_name);
    sdk_set_int(dataJson, "bond0_primary_net", net.bond0_primary_net);
    sdk_set_int(dataJson, "bond0_enable", net.bond0_enable);
    sdk_set_int(dataJson, "bond0_type", net.bond0_type);
    sdk_set_string(dataJson, "bond0_ip_address", net.bond0_ip_address);
    sdk_set_string(dataJson, "bond0_netmask", net.bond0_netmask);
    sdk_set_string(dataJson, "bond0_gateway", net.bond0_gateway);
    sdk_set_string(dataJson, "bond0_primary_dns", net.bond0_primary_dns);
    sdk_set_string(dataJson, "bond0_second_dns", net.bond0_second_dns);
    sdk_set_int(dataJson, "bond0_mtu", net.bond0_mtu);

    sdk_set_int(dataJson, "lan1_enable", net.lan1_enable);
    sdk_set_int(dataJson, "lan1_type", net.lan1_type);
    sdk_set_string(dataJson, "lan1_ip_address", net.lan1_ip_address);
    sdk_set_string(dataJson, "lan1_netmask", net.lan1_netmask);
    sdk_set_string(dataJson, "lan1_gateway", net.lan1_gateway);
    sdk_set_string(dataJson, "lan1_primary_dns", net.lan1_primary_dns);
    sdk_set_string(dataJson, "lan1_second_dns", net.lan1_second_dns);
    sdk_set_int(dataJson, "lan1_mtu", net.lan1_mtu);

    sdk_set_int(dataJson, "lan2_enable", net.lan2_enable);
    sdk_set_int(dataJson, "lan2_type", net.lan2_type);
    sdk_set_string(dataJson, "lan2_ip_address", net.lan2_ip_address);
    sdk_set_string(dataJson, "lan2_netmask", net.lan2_netmask);
    sdk_set_string(dataJson, "lan2_gateway", net.lan2_gateway);
    sdk_set_string(dataJson, "lan2_primary_dns", net.lan2_primary_dns);
    sdk_set_string(dataJson, "lan2_second_dns", net.lan2_second_dns);
    sdk_set_int(dataJson, "lan2_mtu", net.lan2_mtu);

    sdk_set_string(dataJson, "lan1_dhcp_gateway", net.lan1_dhcp_gateway);
    sdk_set_string(dataJson, "lan2_dhcp_gateway", net.lan2_dhcp_gateway);
    sdk_set_int(dataJson, "tri_alarms", net.tri_alarms);

    sdk_set_string(dataJson, "lan1_ip6_addr", net.lan1_ip6_address);
    sdk_set_string(dataJson, "lan1_ip6_netmask", net.lan1_ip6_netmask);
    sdk_set_string(dataJson, "lan1_ip6_gateway", net.lan1_ip6_gateway);
    sdk_set_string(dataJson, "lan2_ip6_addr", net.lan2_ip6_address);
    sdk_set_string(dataJson, "lan2_ip6_netmask", net.lan2_ip6_netmask);
    sdk_set_string(dataJson, "lan2_ip6_gateway", net.lan2_ip6_gateway);
    sdk_set_int(dataJson, "lan1_ip6_dhcp", net.lan1_ip6_dhcp);
    sdk_set_int(dataJson, "lan2_ip6_dhcp", net.lan2_ip6_dhcp);

    sdk_set_int(dataJson, "bond0_ip6_dhcp", net.bond0_ip6_dhcp);
    sdk_set_string(dataJson, "bond0_ip6_address", net.bond0_ip6_address);
    sdk_set_string(dataJson, "bond0_ip6_netmask", net.bond0_ip6_netmask);
    sdk_set_string(dataJson, "bond0_ip6_gateway", net.bond0_ip6_gateway);
    sdk_set_int(dataJson, "defaultRoute", net.defaultRoute); //end
    
    sdk_response_end();
    return ret;
}

int ware_camera_get_ipc_ptz_wiper_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    IPC_PTZ_WIPER_S *pInfo = (IPC_PTZ_WIPER_S *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "chnId", pInfo->chnId);
    sdk_set_int(dataJson, "autoWiper", pInfo->autoWiper);
    sdk_set_int(dataJson, "wiperSupport", pInfo->wiperSupport);
    sdk_response_end();
    return ret;
}

int ware_camera_set_ipc_ptz_wiper_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_WIPER_S *param)
{
    sdk_request_start();
    memset(param, 0, sizeof(IPC_PTZ_WIPER_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    sdk_get_int(reqJson, "autoWiper", &param->autoWiper, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_get_int(reqJson, "manualWiper", &param->manualWiper, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_get_string(reqJson, "copyChn", param->copyChn, NESSARY_NO, STRING_EMPTY, 0, 0);
    sdk_request_end();
    return 0;
}

int ware_camera_get_ipc_ptz_auto_home_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    IPC_PTZ_AUTO_HOME_S *pInfo = (IPC_PTZ_AUTO_HOME_S *)data;
    int i;
    cJSON *array, *obj;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "chnId", pInfo->chnId);
    sdk_set_int(dataJson, "enable", pInfo->autoHomeEnable);
    sdk_set_int(dataJson, "latencyTime", pInfo->latencyTime);
    sdk_set_int(dataJson, "mode", pInfo->autoHomeMode);
    sdk_set_int(dataJson, "modeNumber", pInfo->autoHomeModeNumber);
    array = cJSON_CreateArray();
    for (i = 0; i < PRESET_MAX; i++) {
        obj = cJSON_CreateObject();
        sdk_set_int(obj, "enable", pInfo->preset[i].enable);
        cJSON_AddStringToObject(obj, "name", pInfo->preset[i].name);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(dataJson, "preset", array);
    sdk_set_int(dataJson, "ptzSupport", pInfo->ptzSupport);
    sdk_set_int(dataJson, "wiperSupport", pInfo->wiperSupport);
    sdk_response_end();
    return ret;
}

int ware_camera_set_ipc_ptz_auto_home_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_AUTO_HOME_S *param)
{
    sdk_request_start();
    memset(param, 0, sizeof(IPC_PTZ_AUTO_HOME_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    sdk_get_int(reqJson, "enable", &param->autoHomeEnable, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_get_int(reqJson, "latencyTime", &param->latencyTime, NESSARY_NO, NUM_RANGE, 5, 720);
    sdk_get_int(reqJson, "mode", &param->autoHomeMode, NESSARY_NO, NUM_RANGE, 0, 0);
    sdk_get_int(reqJson, "modeNumber", &param->autoHomeModeNumber, NESSARY_NO, NUM_RANGE, 0, 300);
    sdk_request_end();
    return 0;
}

int ware_camera_set_ipc_ptz_initial_position_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen,  IPC_PTZ_INITIAL_POSITION_S *param)
{
    sdk_request_start();
    memset(param, 0, sizeof(IPC_PTZ_INITIAL_POSITION_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    sdk_get_int(reqJson, "positionCmd", &param->positionCmd, NESSARY_NO, NUM_RANGE, 0, 2);
    sdk_request_end();
    return 0;
}

int ware_camera_set_ipc_ptz_config_clear_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen,  IPC_PTZ_CONFIG_CLEAR_S *param) 
{
    sdk_request_start();
    memset(param, 0, sizeof(IPC_PTZ_CONFIG_CLEAR_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    sdk_get_int(reqJson, "clearMask", &param->clearMask, NESSARY_NO, NUM_RANGE, 0, MAX_CONFIG_CLEAR);
    sdk_get_string(reqJson, "copyChn", param->copyChn, NESSARY_NO, STRING_EMPTY, 0, 0);
    sdk_request_end();
    return 0;
}

int ware_camera_get_ipc_ptz_limit_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    IPC_PTZ_LIMIT_INFO_S *pInfo = (IPC_PTZ_LIMIT_INFO_S *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "chnId", pInfo->chnId);
    sdk_set_int(dataJson, "limitMode", pInfo->limitMode);
    sdk_set_int(dataJson, "manualLimitEnable", pInfo->manualLimitEnable);
    sdk_set_int(dataJson, "manualLimitStatus", pInfo->manualLimitStatus);
    sdk_set_int(dataJson, "manualLimited", pInfo->manualLimited);
    sdk_set_int(dataJson, "scanLimitEnable", pInfo->scanLimitEnable);
    sdk_set_int(dataJson, "scanLimitStatus", pInfo->scanLimitStatus);
    sdk_set_int(dataJson, "scanLimited", pInfo->scanLimited);
    sdk_set_int(dataJson, "ptzSupport", pInfo->ptzSupport);
    sdk_set_int(dataJson, "speedOrMiniPtzDemoSupport", pInfo->speedOrMiniPtzDemoSupport);
    sdk_response_end();
    return ret;
}

int ware_camera_set_ipc_ptz_limit_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_LIMIT_CONTROL_S *param)
{
    sdk_request_start();
    memset(param, 0, sizeof(IPC_PTZ_LIMIT_CONTROL_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    sdk_get_int(reqJson, "limitType", &param->limitType, NESSARY_YES, NUM_RANGE, 0, PTZ_LIMITS_TYPE_MAX - 1);
    sdk_get_int(reqJson, "setValue", &param->setValue, NESSARY_YES, NUM_RANGE, 0, 1);
    sdk_request_end();
    return 0;
}

int ware_camera_get_ipc_ptz_schedule_task_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    IPC_PTZ_SCHE_TASK_INFO_S *pInfo = (IPC_PTZ_SCHE_TASK_INFO_S *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "chnId", pInfo->chnId);
    sdk_set_int(dataJson, "scheTaskEnable", pInfo->scheTaskEnable);
    sdk_set_int(dataJson, "recoveryTime", pInfo->recoveryTime);
    sdk_set_int(dataJson, "ptzSupport", pInfo->ptzSupport);
    sdk_set_int(dataJson, "wiperSupport", pInfo->wiperSupport);
    sdk_set_int(dataJson, "speedDomeSupport", pInfo->speedDomeSupport);
    ware_common_json_schedule_out(dataJson, &pInfo->schedule);
    sdk_response_end();
    return ret;
}

int ware_camera_set_ipc_ptz_schedule_task_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_SCHE_TASK_INFO_S *param)
{
    sdk_request_start();
    memset(param, 0, sizeof(IPC_PTZ_SCHE_TASK_INFO_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    sdk_get_int(reqJson, "scheTaskEnable", &param->scheTaskEnable, NESSARY_NO, NUM_RANGE, 0, 1);
    sdk_get_int(reqJson, "recoveryTime", &param->recoveryTime, NESSARY_NO, NUM_RANGE, 5, 720);
    sdk_get_string(reqJson, "copyChn", param->copyChn, NESSARY_NO, STRING_EMPTY, 0, 0);
    ware_common_json_schedule_in(cJSON_GetObjectItem(reqJson, "sche"), &param->schedule);
    sdk_request_end();
    return 0;
}

int ware_camera_set_ipc_ptz_control_json_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IPC_PTZ_CONTROL_S *param)
{
    sdk_request_start();
    memset(param, 0, sizeof(IPC_PTZ_CONTROL_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA-1);
    sdk_get_int(reqJson, "controlType", &param->controlType, NESSARY_NO, NUM_RANGE, 0, IPC_PTZ_CONTORL_MAX - 1);
    sdk_get_int(reqJson, "controlAction", &param->controlAction, NESSARY_NO, NUM_RANGE, 0, 300);
    sdk_request_end();
    return 0;
}

int ware_camera_get_ipc_ptz_panel_status_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    IPC_PTZ_PANEL_STATUS_S *pInfo = (IPC_PTZ_PANEL_STATUS_S *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "chnId", pInfo->chnId);
    sdk_set_int(dataJson, "irisMode", pInfo->irisMode);
    sdk_set_int(dataJson, "onePatrol", pInfo->onePatrol);
    sdk_set_int(dataJson, "autoHome", pInfo->autoHome);
    sdk_set_int(dataJson, "manualDefog", pInfo->manualDefog);
    sdk_set_int(dataJson, "manualWiper", pInfo->manualWiper);
    sdk_set_int(dataJson, "wiperSupport", pInfo->wiperSupport);
    sdk_set_int(dataJson, "defogSupport", pInfo->defogSupport);
    sdk_response_end();
    return ret;
}

int ware_camera_get_ipc_cap_image_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    sdk_judge_null(data, cnt);
    sdk_response_start();
    cJSON_ReplaceItemInObject(respJson, "data", cJSON_Parse(data)); 
    sdk_response_end();
    return ret;
}

int ware_system_get_online_upgrade_info_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    struct resp_check_online_upgrade *pInfo = (struct resp_check_online_upgrade *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "checkState", pInfo->state);
    sdk_set_int(dataJson, "fileSize", pInfo->fileSize);
    sdk_set_string(dataJson, "softversion", pInfo->pSoftversion);
    sdk_set_string(dataJson, "url", pInfo->pUrl);
    sdk_set_string(dataJson, "description", pInfo->pDescription);
    sdk_response_end();
    return ret;
}

int ware_camera_set_ipc_image_splicedistance_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, IMAGE_SPILCEDISTANCE_S *param)
{
    sdk_request_start();
    memset(param, 0, sizeof(IMAGE_SPILCEDISTANCE_S));
    sdk_get_int(reqJson, "chnId", &param->chnId, NESSARY_YES, NUM_RANGE, 0, MAX_REAL_CAMERA - 1);
    sdk_get_double(reqJson, "spliceDistance", &param->distance, NESSARY_YES, NUM_RANGE, 0.5, 30);
    sdk_request_end();
    return 0;
}

int ware_camera_get_ipc_image_splicedistance_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    IMAGE_SPILCEDISTANCE_S *pInfo = (IMAGE_SPILCEDISTANCE_S *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "chnId", pInfo->chnId);
    sdk_set_int(dataJson, "spliceDistance", pInfo->distance);
    sdk_response_end();
    return ret;
}

int ware_camera_get_disk_health_management_data_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, int *param)
{
    sdk_request_start();
    sdk_get_int(reqJson, "diskPort", param, NESSARY_YES, NUM_RANGE, 1, 25);
    sdk_request_end();
    return 0;
}

int ware_camera_get_disk_health_management_data_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    struct disk_health_data_t *pInfo = (struct disk_health_data_t *)data;
    cJSON *tmp = NULL;
    cJSON *array = NULL;
    int i;
    char bufferTime[32];
    struct tm *tmInfo;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "diskPort", pInfo->port);
    sdk_set_int(dataJson, "diskSupport", pInfo->supportHM);
    sdk_set_int(dataJson, "diskHMStatus", pInfo->HMStatus);
    sdk_set_int(dataJson, "diskTemperatureStatus", pInfo->temperatureStatus);
    sdk_set_string(dataJson, "diskVendor", pInfo->vendor);
    sdk_set_int(dataJson, "diskEnable", pInfo->HMEnable);
    sdk_set_int(dataJson, "hasDisk", pInfo->hasDisk);
    array = cJSON_CreateArray();
    for (i = 0; i < MAX_DISK_HM_TEMPERATURE; i++) {
        tmp = cJSON_CreateObject();
        if (pInfo->temperatureList[i].temperature != -300) {
            cJSON_AddNumberToObject(tmp, "temperature", pInfo->temperatureList[i].temperature);
        } else {
            cJSON_AddNullToObject(tmp, "temperature");
        }
        tmInfo = localtime(&pInfo->temperatureList[i].timestamp);
        strftime(bufferTime, sizeof(bufferTime), "%Y-%m-%d %H:%M", tmInfo);
        cJSON_AddStringToObject(tmp, "timestamp", bufferTime);
        cJSON_AddItemToArray(array, tmp);
    }
    cJSON_AddItemToObject(dataJson, "diskTemperature", array);
    sdk_response_end();
    return ret;
}

int ware_camera_get_disk_health_management_log_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    char *logData = (char *)data;
    char filename[64] = {0};
    int fileSize = 0;

    snprintf(filename, sizeof(filename), logData);
    fileSize = *(int *)(logData + sizeof(filename));
    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "status", REQUEST_SUCCESS);
    sdk_set_string(dataJson, "filename", filename);
    sdk_set_int(dataJson, "fileSize", fileSize);
    sdk_set_string(dataJson, "data", data + sizeof(filename) + sizeof(fileSize));
    sdk_response_end();
    return ret;
}

int ware_camera_set_disk_health_management_data_in(char *reqUrl, const char *jsonBuff, char *resp, int respLen, disk_health_data_t *param)
{
    sdk_request_start();
    sdk_get_int(reqJson, "diskPort", &param->port, NESSARY_YES, NUM_RANGE, 1, 25);
    sdk_get_int(reqJson, "diskEnable", &param->HMEnable, NESSARY_YES, NUM_RANGE, 0, 1);
    sdk_request_end();
    return 0;
}

int ware_playback_picture_search_index_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    char *PicData = (char *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_string(dataJson, "data", PicData);
    sdk_response_end();
    return ret;
}

int ware_playback_picture_search_close_out(char *reqUrl, void *data, int cnt, char *resp, int respLen)
{
    int resData = *(int *)data;

    sdk_judge_null(data, cnt);
    sdk_response_start();
    sdk_set_int(dataJson, "res", resData);
    sdk_response_end();
    return ret;
}
// request function define end
