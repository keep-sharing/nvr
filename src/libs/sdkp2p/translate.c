#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include "translate.h"


struct sdkp2p_translate_conf {
    //p2p
    struct translate_request_p2p req_p2p[MAX_SKDP2P_REQ_COUNT];
    char req_p2p_fill[MAX_SKDP2P_REQ_COUNT];
    int req_p2p_used;

    //sdk
    struct translate_request_p2p req_sdk[MAX_SKDP2P_REQ_COUNT];
    char req_sdk_fill[MAX_SKDP2P_REQ_COUNT];
    int req_sdk_used;
};

static struct sdkp2p_translate_conf global_conf;
int g_p2pCurRespId = 0;

#define is_valid_int(_n) (((_n) == MS_INVALID_VALUE) ? 0 : 1)

#define ms_get_int(_a, _b) \
    do {\
        if ((_b) != MS_INVALID_VALUE) {\
            _a = _b;\
        } \
    }while(0)

#define ms_get_string(_a, _s, _b) \
        do {\
            if ((_b) != NULL && !strstr(_b, MS_INVALID_STRING)) {\
                snprintf(_a, (_s), "%s", _b);\
            } \
        }while(0)

static inline int ms_need_copy(Uint64 mask, int chnid)
{
    if (((Uint64)1<<chnid) == mask) {
        return 0;
    }

    return 1;
}

static inline int ms_has_data(MS_BOOL_E hasData)
{
    return hasData != MS_BOOL_NO;
}

static inline int ms_need_write_action(int needCopy, MS_BOOL_E hasData)
{
    if (needCopy || ms_has_data(hasData)) {
        return 1;
    }

    return 0;
}

static int get_json_section_int(cJSON *json, const char *key, int *des, int *typeError)
{
    if (!des) {
        return -1;
    }

    *des = MS_INVALID_VALUE;
    if (!json || !key) {
        return -1;
    }

    cJSON *obj = cJSON_GetObjectItem(json, key); 
    if(!obj) {
        return -1;
    }

    if (obj->type != cJSON_Number) {
        if (typeError) {
            *typeError = 1;
        }
    } else {
        if (typeError) {
            *typeError = 0;
        }
        
        *des = obj->valueint;
    }   
    
    return 0;
}

/**
 * @return int -1：格式错误，0：空字符，1：有效字符
 */
static int get_json_section_string(cJSON *json, const char *key, char *des, int desLen, int *typeError)
{
    if (!des || !desLen) {
        return -1;
    }

    snprintf(des, desLen, "%s", MS_INVALID_STRING);
    if (!json || !key) {
        return -1;
    }

    cJSON *obj = cJSON_GetObjectItem(json, key); 
    if(!obj) {
        return -1;
    }

    if (obj->type != cJSON_String) {
        if (typeError) {
            *typeError = 1;
        }
    } else if (obj->valuestring != NULL){
        if (strlen(obj->valuestring) == 0) {
            des[0] = '\0';
            return 0;
        }
        snprintf(des, desLen, "%s", obj->valuestring);
        if (typeError) {
            *typeError = 0;
        }
        
    } else {
        return -1;
    }

    return 1;
}

static cJSON *get_json_action_schedule(SMART_SCHEDULE *schedule)
{
    int i, j;
    cJSON *array, *day, *obj;

    array = cJSON_CreateArray();
    for (i = 0; i < MAX_DAY_NUM; i++) {
        day = cJSON_CreateArray();
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (!strlen(schedule->schedule_day[i].schedule_item[j].start_time)) {
                break;
            }
            obj = cJSON_CreateArray();
            cJSON_AddItemToArray(obj, cJSON_CreateString(schedule->schedule_day[i].schedule_item[j].start_time));
            cJSON_AddItemToArray(obj, cJSON_CreateString(schedule->schedule_day[i].schedule_item[j].end_time));
            cJSON_AddItemToArray(obj, cJSON_CreateNumber(schedule->schedule_day[i].schedule_item[j].action_type));
            cJSON_AddItemToArray(day, obj);
        }
        cJSON_AddItemToArray(array, day);
    }

    return array;
}

MS_BOOL_E get_struct_action_schedule(cJSON *json, SMART_SCHEDULE *schedule)
{
    int i = 0, j = 0;
    int dayCnt,itemCnt;
    cJSON *dayArr, *itemArr;

    if (!json || !schedule) {
        return MS_BOOL_NO;
    }
    memset(schedule, 0, sizeof(SMART_SCHEDULE));
    dayCnt = cJSON_GetArraySize(json);
    for (i = 0; i < MAX_DAY_NUM && i < dayCnt; ++i) {
        dayArr = cJSON_GetArrayItem(json, i);
        itemCnt = cJSON_GetArraySize(dayArr);
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY && j < itemCnt; ++j) {
            itemArr = cJSON_GetArrayItem(dayArr, j);
            snprintf(schedule->schedule_day[i].schedule_item[j].start_time,
                     sizeof(schedule->schedule_day[i].schedule_item[j].start_time), 
                     cJSON_GetArrayItem(itemArr, 0)->valuestring);
            snprintf(schedule->schedule_day[i].schedule_item[j].end_time,
                     sizeof(schedule->schedule_day[i].schedule_item[j].end_time), 
                     cJSON_GetArrayItem(itemArr, 1)->valuestring);
            schedule->schedule_day[i].schedule_item[j].action_type = cJSON_GetArrayItem(itemArr, 2)->valueint;
        }
    }

    return MS_BOOL_YES;
}

static cJSON *get_json_action_effective(ACTION_EFFECTIVE_S *effective)
{
    cJSON *json;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddItemToObject(json, "sche", get_json_action_schedule(&effective->sche));
    return json;
}

static void get_struct_action_effective(cJSON *json, ACTION_EFFECTIVE_S *effective)
{
    effective->scheIsValid = MS_BOOL_NO;
    if (!json) {
        return;
    }
    effective->scheIsValid = get_struct_action_schedule(cJSON_GetObjectItem(json, "sche"), &effective->sche);
}

static cJSON *get_json_action_audible(ACTION_AUDIBLE_S *audible)
{
    cJSON *json;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddItemToObject(json, "sche", get_json_action_schedule(&audible->sche));
    cJSON_AddNumberToObject(json, "interval", audible->interval);
    cJSON_AddNumberToObject(json, "id", audible->id);
    cJSON_AddNumberToObject(json, "mask", audible->mask);
    return json;
}

static void get_struct_action_audible(cJSON *json, ACTION_AUDIBLE_S *audible)
{
    audible->scheIsValid = MS_BOOL_NO;
    audible->interval = MS_INVALID_VALUE;
    audible->id = MS_INVALID_VALUE;
    
    if (!json) {
        return;
    }
    audible->scheIsValid = get_struct_action_schedule(cJSON_GetObjectItem(json, "sche"), &audible->sche);
    get_json_section_int(json, "interval", &audible->interval, NULL);
    get_json_section_int(json, "id", &audible->id, NULL);

    ms_log(TSAR_DEBUG, "audble interval:%d", audible->interval);
    ms_log(TSAR_DEBUG, "audble id:%d", audible->id);
}

static cJSON *get_json_action_email(ACTION_EMAIL_S *email)
{
    cJSON *json;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddItemToObject(json, "sche", get_json_action_schedule(&email->sche));
    cJSON_AddNumberToObject(json, "interval", email->interval);
    cJSON_AddNumberToObject(json, "picEnable", email->picEnable);
    cJSON_AddStringToObject(json, "triSnapshot", email->triSnapshot);
    return json;
}

static void get_struct_action_email(cJSON *json, ACTION_EMAIL_S *email)
{
    email->scheIsValid = MS_BOOL_NO;
    email->interval = MS_INVALID_VALUE;
    email->picEnable = MS_INVALID_VALUE;
    snprintf(email->triSnapshot, sizeof(email->triSnapshot), "%s", MS_INVALID_STRING);
    if (!json) {
        return;
    }
    
    email->scheIsValid = get_struct_action_schedule(cJSON_GetObjectItem(json, "sche"), &email->sche);
    get_json_section_int(json, "interval", &email->interval, NULL);
    get_json_section_int(json, "picEnable", &email->picEnable, NULL);
    get_json_section_string(json, "triSnapshot", email->triSnapshot, sizeof(email->triSnapshot), NULL);

    ms_log(TSAR_DEBUG, "email interval:%d", email->interval);
    ms_log(TSAR_DEBUG, "email picEnable:%d", email->picEnable);
}

static cJSON *get_json_action_ptz(ACTION_PTZ_S *ptz)
{
    int i;
    cJSON *json, *array, *obj;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddItemToObject(json, "sche", get_json_action_schedule(&ptz->sche));
    cJSON_AddNumberToObject(json, "interval", ptz->interval);
    array = cJSON_CreateArray();
    for (i = 0; i < ptz->cnt && i < MAX_REAL_CAMERA; i++) {
        obj = cJSON_CreateArray();
        if (!obj) {
            continue;
        }
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(ptz->params[i].acto_ptz_channel));
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(ptz->params[i].acto_ptz_type));
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(ptz->params[i].acto_ptz_preset));
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(ptz->params[i].acto_ptz_patrol));
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "params", array);
    return json;
}

static void get_struct_action_ptz(cJSON *json, ACTION_PTZ_S *ptz)
{
    int i = 0;
    int cnt;
    cJSON *params, *itemArr;
    
    ptz->scheIsValid = MS_BOOL_NO;
    ptz->interval = MS_INVALID_VALUE;
    ptz->paramsIsValid = MS_BOOL_NO;
    if (!json) {
        return;
    }
    
    ptz->scheIsValid = get_struct_action_schedule(cJSON_GetObjectItem(json, "sche"), &ptz->sche);
    get_json_section_int(json, "interval", &ptz->interval, NULL);
    ms_log(TSAR_DEBUG, "ptz interval:%d", ptz->interval);
    params = cJSON_GetObjectItem(json, "params");
    if (params) {
        cnt = cJSON_GetArraySize(params);
        ms_log(TSAR_DEBUG, "ptz cnt:%d", cnt);
        for (i = 0; i < MAX_REAL_CAMERA && i < cnt; i++) {
            itemArr = cJSON_GetArrayItem(params, i);
            if (cJSON_GetArraySize(itemArr) < 4) {
                continue;
            }
            ptz->params[i].acto_ptz_channel = cJSON_GetArrayItem(itemArr, 0)->valueint;
            ptz->params[i].acto_ptz_type = cJSON_GetArrayItem(itemArr, 1)->valueint;
            ptz->params[i].acto_ptz_preset = cJSON_GetArrayItem(itemArr, 2)->valueint;
            ptz->params[i].acto_ptz_patrol = cJSON_GetArrayItem(itemArr, 3)->valueint;
            ms_log(TSAR_DEBUG, "ptz cnt:%d i:%d ch:%d type:%d preset:%d patrol:%d", cnt, i, ptz->params[i].chn_id,
                ptz->params[i].acto_ptz_type, ptz->params[i].acto_ptz_preset, ptz->params[i].acto_ptz_patrol);
        }
    }
    ptz->paramsIsValid = MS_BOOL_YES;
}

static cJSON *get_json_action_alarmout(ACTION_ALARMOUT_S *alarmout)
{
    int i;
    cJSON *json, *array;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    cJSON_AddNumberToObject(json, "interval", alarmout->interval);
    cJSON_AddNumberToObject(json, "nvrAlarm", alarmout->nvrAlarm);
    array = cJSON_CreateArray();
    if (array) {
        //for (i = 0; i < MAX_ALARM_OUT; i++) {
        for (i = 0; i < 2; i++) {
            cJSON_AddItemToArray(array, cJSON_CreateString(alarmout->ipcAlarm[i]));
        }
        cJSON_AddItemToObject(json, "ipcAlarm", array);
    }
    return json;
}

static void get_struct_action_alarmout(cJSON *json, ACTION_ALARMOUT_S *alarmout)
{
    int i = 0;
    int cnt;
    cJSON *params;
    
    alarmout->interval = MS_INVALID_VALUE;
    alarmout->nvrAlarm = MS_INVALID_VALUE;
    for (i = 0; i < MAX_ALARM_OUT; i++) {
        snprintf(alarmout->ipcAlarm[i], sizeof(alarmout->ipcAlarm[i]), "%s", MS_INVALID_STRING);
    }
    if (!json) {
        return;
    }

    get_json_section_int(json, "interval", &alarmout->interval, NULL);
    get_json_section_int(json, "nvrAlarm", (int *)&alarmout->nvrAlarm, NULL);
    ms_log(TSAR_DEBUG, "alarmout interval:%d", alarmout->interval);
    ms_log(TSAR_DEBUG, "alarmout nvrAlarm:%d", alarmout->nvrAlarm);
    params = cJSON_GetObjectItem(json, "ipcAlarm");
    if (params) {
        cnt = cJSON_GetArraySize(params);
        for (i = 0; i < MAX_ALARM_OUT && i < cnt; i++) {
            snprintf(alarmout->ipcAlarm[i], sizeof(alarmout->ipcAlarm[i]), "%s", cJSON_GetArrayItem(params, i)->valuestring);
            ms_log(TSAR_DEBUG, "alarmout ipcAlarm cnt:%d i:%d mask:%s", cnt, i, alarmout->ipcAlarm[i]);
        }
    }
}

static cJSON *get_json_action_white(ACTION_WHITE_S *white)
{
    int i;
    cJSON *json, *array, *obj;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddItemToObject(json, "sche", get_json_action_schedule(&white->sche));
    cJSON_AddNumberToObject(json, "interval", white->interval);
    array = cJSON_CreateArray();
    for (i = 0; i < white->cnt && i < MAX_REAL_CAMERA; i++) {
        obj = cJSON_CreateArray();
        if (!obj) {
            continue;
        }
        //cJSON_AddItemToArray(obj, cJSON_CreateNumber(white->params[i].chnid));
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(white->params[i].acto_chn_id));
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(white->params[i].flash_mode));
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(white->params[i].flash_time));
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "params", array);
    return json;
}

static void get_struct_action_white(cJSON *json, ACTION_WHITE_S *white)
{
    int i = 0;
    int cnt;
    cJSON *params, *itemArr;
    
    white->scheIsValid = MS_BOOL_NO;
    white->interval = MS_INVALID_VALUE;
    white->paramsIsValid = MS_BOOL_NO;
    if (!json) {
        return;
    }
    white->scheIsValid = get_struct_action_schedule(cJSON_GetObjectItem(json, "sche"), &white->sche);
    get_json_section_int(json, "interval", &white->interval, NULL);
    ms_log(TSAR_DEBUG, "white interval:%d", white->interval);
    params = cJSON_GetObjectItem(json, "params");
    if (params) {
        cnt = cJSON_GetArraySize(params);
        for (i = 0; i < MAX_REAL_CAMERA && i < cnt; i++) {
            itemArr = cJSON_GetArrayItem(params, i);
            if (cJSON_GetArraySize(itemArr) < 3) {
                continue;
            }
            //white->params[i].chnid = cJSON_GetArrayItem(itemArr, 0)->valueint;
            white->params[i].acto_chn_id = cJSON_GetArrayItem(itemArr, 0)->valueint;
            white->params[i].flash_mode = cJSON_GetArrayItem(itemArr, 1)->valueint;
            white->params[i].flash_time = cJSON_GetArrayItem(itemArr, 2)->valueint;
            ms_log(TSAR_DEBUG, "white led cnt:%d i:%d ch:%d mode:%d time:%d", cnt, i,
                white->params[i].acto_chn_id, white->params[i].flash_mode, white->params[i].flash_time);
        }
    }
    white->paramsIsValid = MS_BOOL_YES;
}

static cJSON *get_json_action_http(ACTION_HTTP_S *http)
{
    cJSON *json;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }

    cJSON_AddItemToObject(json, "sche", get_json_action_schedule(&http->sche));
    cJSON_AddNumberToObject(json, "interval", http->interval);
    cJSON_AddNumberToObject(json, "id", http->params.id);
    cJSON_AddStringToObject(json, "url", http->params.url);
    cJSON_AddStringToObject(json, "username", http->params.username);
    cJSON_AddStringToObject(json, "password", http->params.password);
    return json;
}

static void get_struct_action_http(cJSON *json, ACTION_HTTP_S *http)
{
    http->scheIsValid = MS_BOOL_NO;
    http->interval = MS_INVALID_VALUE;
    snprintf(http->params.url, sizeof(http->params.url), "%s", MS_INVALID_STRING);
    snprintf(http->params.username, sizeof(http->params.username), "%s", MS_INVALID_STRING);
    snprintf(http->params.password, sizeof(http->params.password), "%s", MS_INVALID_STRING);
    if (!json) {
        return;
    }
    http->scheIsValid = get_struct_action_schedule(cJSON_GetObjectItem(json, "sche"), &http->sche);
    get_json_section_int(json, "interval", &http->interval, NULL);
    get_json_section_string(json, "url", http->params.url, sizeof(http->params.url), NULL);
    get_json_section_string(json, "username", http->params.username, sizeof(http->params.username), NULL);
    get_json_section_string(json, "password", http->params.password, sizeof(http->params.password), NULL);
}

static cJSON *get_json_action_others(ACTION_OTHERS_S *others)
{
    cJSON *json;

    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    cJSON_AddStringToObject(json, "triRecord", others->triRecord);
    cJSON_AddStringToObject(json, "triSnapshot", others->triSnapshot);
    return json;
}

static void get_struct_action_others(cJSON *json, ACTION_OTHERS_S *others)
{
    snprintf(others->triRecord, sizeof(others->triRecord), "%s", MS_INVALID_STRING);
    snprintf(others->triSnapshot, sizeof(others->triSnapshot), "%s", MS_INVALID_STRING);
    if (!json) {
        return;
    }
    get_json_section_string(json, "triRecord", others->triRecord, sizeof(others->triRecord), NULL);
    get_json_section_string(json, "triSnapshot", others->triSnapshot, sizeof(others->triSnapshot), NULL);
    ms_log(TSAR_DEBUG, "others triRecord:%s", others->triRecord);
    ms_log(TSAR_DEBUG, "others triSnapshot:%s", others->triSnapshot);
}

static cJSON *get_action_json(MS_ACTION_S *action)
{
    cJSON *json;
    
    json = cJSON_CreateObject();
    if (!json) {
        return NULL;
    }
    cJSON_AddNumberToObject(json, "chnid", action->chnid);
    cJSON_AddItemToObject(json, "effective", get_json_action_effective(&action->effective));
    cJSON_AddItemToObject(json, "audible", get_json_action_audible(&action->audible));
    cJSON_AddItemToObject(json, "email", get_json_action_email(&action->email));
    cJSON_AddItemToObject(json, "ptz", get_json_action_ptz(&action->ptz));
    cJSON_AddItemToObject(json, "alarmout", get_json_action_alarmout(&action->alarmout));
    cJSON_AddItemToObject(json, "white", get_json_action_white(&action->white));
    cJSON_AddItemToObject(json, "http", get_json_action_http(&action->http));
    cJSON_AddItemToObject(json, "others", get_json_action_others(&action->others));
    return json;
}

static int get_action_struct(cJSON *json, MS_ACTION_S *action)
{
    if (!json || !action) {
        return -1;
    }

    get_struct_action_effective(cJSON_GetObjectItem(json, "effective"), &action->effective);
    get_struct_action_audible(cJSON_GetObjectItem(json, "audible"), &action->audible);
    get_struct_action_email(cJSON_GetObjectItem(json, "email"), &action->email);
    get_struct_action_ptz(cJSON_GetObjectItem(json, "ptz"), &action->ptz);
    get_struct_action_alarmout(cJSON_GetObjectItem(json, "alarmout"), &action->alarmout);
    get_struct_action_white(cJSON_GetObjectItem(json, "white"), &action->white);
    get_struct_action_http(cJSON_GetObjectItem(json, "http"), &action->http);
    get_struct_action_others(cJSON_GetObjectItem(json, "others"), &action->others);
    return 0;
}

cJSON *get_action_audio_alarm(int chnId)
{
    MS_ACTION_S *action;
    SMART_EVENT event;
    WLED_INFO whiteInfo;
    cJSON *json = NULL;

    action = ms_malloc(sizeof(MS_ACTION_S));
    if (!action) {
        return NULL;
    }
    memset(&event, 0, sizeof(SMART_EVENT));
    read_audio_alarm(SQLITE_FILE_NAME, &event, chnId);
    
    //effective;
    read_audio_alarm_sche(SQLITE_FILE_NAME, &action->effective.sche, AUD_EFFE_SCHE, chnId);
    
    //audible;
    read_audio_alarm_sche(SQLITE_FILE_NAME, &action->audible.sche, AUD_AUDI_SCHE, chnId);
    action->audible.interval = event.buzzer_interval;
    action->audible.id = event.tri_audio_id;
    action->audible.mask = get_audio_file_id_mask_ex();
    
    //email;
    read_audio_alarm_sche(SQLITE_FILE_NAME, &action->email.sche, AUD_EMAIL_SCHE, chnId);
    action->email.interval = event.email_interval;
    action->email.picEnable = event.email_pic_enable;
    
    //ptz;
    read_audio_alarm_sche(SQLITE_FILE_NAME, &action->ptz.sche, AUD_PTZ_SCHE, chnId);
    action->ptz.interval = event.ptzaction_interval;
    read_ptz_params(SQLITE_FILE_NAME, action->ptz.params, AUDIO_ALARM, chnId, &action->ptz.cnt);
    
    //alarmout;
    action->alarmout.interval = event.alarmout_interval;
    action->alarmout.nvrAlarm = event.tri_alarms;
    snprintf(action->alarmout.ipcAlarm[0], sizeof(action->alarmout.ipcAlarm[0]), "%s", event.tri_chnout1_alarms);
    snprintf(action->alarmout.ipcAlarm[1], sizeof(action->alarmout.ipcAlarm[0]), "%s", event.tri_chnout2_alarms);
    
    //white;
    read_audio_alarm_sche(SQLITE_FILE_NAME, &action->white.sche, AUD_WLED_SCHE, chnId);
    action->white.interval = event.whiteled_interval;
    whiteInfo.chnid = chnId;
    snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", AUD_WLED_PARAMS);
    read_whiteled_params(SQLITE_FILE_NAME, action->white.params, &whiteInfo, &action->white.cnt);
    
    //http;
    read_audio_alarm_sche(SQLITE_FILE_NAME, &action->http.sche, AUD_HTTP_SCHE, chnId);
    action->http.interval = event.http_notification_interval;
    read_http_notification_params(SQLITE_FILE_NAME, &action->http.params, AUD_HTTP_PARAMS, chnId);
    
    //others;
    snprintf(action->others.triRecord, sizeof(action->others.triRecord), "%s", event.tri_channels_ex);
    snprintf(action->others.triSnapshot, sizeof(action->others.triSnapshot), "%s", event.tri_channels_pic);

    json = get_action_json(action);
    ms_free(action);
    return json;
}

int set_action_audio_alarm(cJSON *json, int chnId, Uint64 chnMask)
{
    MS_ACTION_S *action;
    SMART_EVENT event;
    HTTP_NOTIFICATION_PARAMS_S http;
    WLED_INFO whiteInfo;
    int needCopy = 0;

    if (!json) {
        return -1;
    }

    action = ms_malloc(sizeof(MS_ACTION_S));
    if (!action) {
        return -1;
    }
    memset(&event, 0, sizeof(SMART_EVENT));
    memset(&http, 0, sizeof(HTTP_NOTIFICATION_PARAMS_S));
    read_audio_alarm(SQLITE_FILE_NAME, &event, chnId);
    read_http_notification_params(SQLITE_FILE_NAME, &http, AUD_HTTP_PARAMS, chnId);
    get_action_struct(json, action);

    needCopy = ms_need_copy(chnMask, chnId);

    /*effective*/
    if (ms_need_write_action(needCopy, action->effective.scheIsValid)) {
        if (!ms_has_data(action->effective.scheIsValid)) {
            read_audio_alarm_sche(SQLITE_FILE_NAME, &action->effective.sche, AUD_EFFE_SCHE, chnId);
        }
        copy_audio_alarm_sche(SQLITE_FILE_NAME, &action->effective.sche, AUD_EFFE_SCHE, chnMask);
    }
    
    /*audible*/
    if (ms_need_write_action(needCopy, action->audible.scheIsValid)) {
        if (!ms_has_data(action->audible.scheIsValid)) {
            read_audio_alarm_sche(SQLITE_FILE_NAME, &action->audible.sche, AUD_AUDI_SCHE, chnId);
        }
        copy_audio_alarm_sche(SQLITE_FILE_NAME, &action->audible.sche, AUD_AUDI_SCHE, chnMask);
    }
    ms_get_int(event.buzzer_interval, action->audible.interval);
    ms_get_int(event.tri_audio_id, action->audible.id);
    
    /*email*/
    if (ms_need_write_action(needCopy, action->email.scheIsValid)) {
        if (!ms_has_data(action->email.scheIsValid)) {
            read_audio_alarm_sche(SQLITE_FILE_NAME, &action->email.sche, AUD_EMAIL_SCHE, chnId);
        }
        copy_audio_alarm_sche(SQLITE_FILE_NAME, &action->email.sche, AUD_EMAIL_SCHE, chnMask);
    }
    ms_get_int(event.email_interval, action->email.interval);
    ms_get_int(event.email_pic_enable, action->email.picEnable);

    /*ptz*/
    if (ms_need_write_action(needCopy, action->ptz.scheIsValid)) {
        if (!ms_has_data(action->ptz.scheIsValid)) {
            read_audio_alarm_sche(SQLITE_FILE_NAME, &action->ptz.sche, AUD_PTZ_SCHE, chnId);
        }
        copy_audio_alarm_sche(SQLITE_FILE_NAME, &action->ptz.sche, AUD_PTZ_SCHE, chnMask);
    }
    ms_get_int(event.ptzaction_interval, action->ptz.interval);
    if (ms_need_write_action(needCopy, action->ptz.paramsIsValid)) {
        if (!ms_has_data(action->ptz.paramsIsValid)) {
            read_ptz_params(SQLITE_FILE_NAME, action->ptz.params, AUDIO_ALARM, chnId, &action->ptz.cnt);
        }
        copy_ptz_params_all(SQLITE_FILE_NAME, &action->ptz.params[0], AUDIO_ALARM, chnMask);
    }

    /*alarmout*/
    ms_get_int(event.alarmout_interval, action->alarmout.interval);
    ms_get_int(event.tri_alarms, action->alarmout.nvrAlarm);
    ms_get_string(event.tri_chnout1_alarms, sizeof(event.tri_chnout1_alarms), action->alarmout.ipcAlarm[0]);
    ms_get_string(event.tri_chnout2_alarms, sizeof(event.tri_chnout2_alarms), action->alarmout.ipcAlarm[1]);

    /*white*/
    if (ms_need_write_action(needCopy, action->white.scheIsValid)) {
        if (!ms_has_data(action->white.scheIsValid)) {
            read_audio_alarm_sche(SQLITE_FILE_NAME, &action->white.sche, AUD_WLED_SCHE, chnId);
        }
        copy_audio_alarm_sche(SQLITE_FILE_NAME, &action->white.sche, AUD_WLED_SCHE, chnMask);
    }
    ms_get_int(event.whiteled_interval, action->white.interval);
    if (ms_need_write_action(needCopy, action->white.paramsIsValid)) {
        if (!ms_has_data(action->white.paramsIsValid)) {
            whiteInfo.chnid = chnId;
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", AUD_WLED_PARAMS);
            read_whiteled_params(SQLITE_FILE_NAME, action->white.params, &whiteInfo, &action->white.cnt);
        }
        copy_whiteled_params(SQLITE_FILE_NAME, AUD_WLED_PARAMS, action->white.params, chnMask);
    }

    /*http*/
    if (ms_need_write_action(needCopy, action->http.scheIsValid)) {
        if (!ms_has_data(action->http.scheIsValid)) {
            read_audio_alarm_sche(SQLITE_FILE_NAME, &action->http.sche, AUD_HTTP_SCHE, chnId);
        }
        copy_audio_alarm_sche(SQLITE_FILE_NAME, &action->http.sche, AUD_HTTP_SCHE, chnMask);
    }
    ms_get_int(event.http_notification_interval, action->http.interval);
    ms_get_string(http.url, sizeof(http.url), action->http.params.url);
    ms_get_string(http.username, sizeof(http.username), action->http.params.username);
    ms_get_string(http.password, sizeof(http.password), action->http.params.password);
    copy_http_notification_params(SQLITE_FILE_NAME, AUD_HTTP_PARAMS, &http, chnMask);
    
    /*others*/
    ms_get_string(event.tri_channels_ex, sizeof(event.tri_channels_ex), action->others.triRecord);
    ms_get_string(event.tri_channels_pic, sizeof(event.tri_channels_pic), action->others.triSnapshot);

    /*event*/
    copy_audio_alarms(SQLITE_FILE_NAME, &event, chnMask);
    ms_free(action);
    return 0;
}

int p2p_request_register(struct translate_request_p2p *req, int count)
{
    int index = 0;
    int i = 0;

    if (!req || count <= 0) {
        return -1;
    }

    if (global_conf.req_p2p_used + count >= MAX_SKDP2P_REQ_COUNT) {
        msdebug(DEBUG_INF, "p2p_request_register count over limit %d", MAX_SKDP2P_REQ_COUNT);
        return -1;
    }

    for (i = 0; i < MAX_SKDP2P_REQ_COUNT; i++) {
        if (index >= count) {
            break;
        }

        if (global_conf.req_p2p_fill[i] != 0) {
            continue;
        }

        memcpy(global_conf.req_p2p + i, req + index, sizeof(struct translate_request_p2p));
        global_conf.req_p2p_fill[i] = 1;
        index++;
        global_conf.req_p2p_used++;
    }

    return 0;
}

int p2p_request_unregister(struct translate_request_p2p *req, int count)
{
    int i = 0;
    int index = 0;

    if (!req || count <= 0) {
        return -1;
    }

    for (i = 0; i < MAX_SKDP2P_REQ_COUNT; i++) {
        if (global_conf.req_p2p_fill[i] == 0) {
            continue;
        }

        for (index = 0; index < count; index++) {
            if (global_conf.req_p2p[i].req == req[index].req) {
                memset(global_conf.req_p2p + i, 0x0, sizeof(struct translate_request_p2p));
                global_conf.req_p2p_fill[i] = 0;
                global_conf.req_p2p_used--;
                break;
            }
        }
    }

    return 0;
}

int sdk_request_register(struct translate_request_sdk *req, int count)
{
    return -1;
}

int sdk_request_unregister(struct translate_request_sdk *req, int count)
{
    return -1;
}

static void sdkp2p_dispatch_request_p2p(struct sdkp2p_request *req, p2p_send_cb cb)
{
    int i = 0;
    int need_cb = 0;
    struct req_conf_str conf;

    if (!req || !cb) {
        return;
    }

    for (i = 0; i < MAX_SKDP2P_REQ_COUNT; i++) {
        if (global_conf.req_p2p_fill[i] == 0) {
            continue;
        }

        if (req->request != global_conf.req_p2p[i].req) {
            continue;
        }

        conf.req = req->request;
        conf.response = req->response;
        conf.resp = req->resp;
        conf.size = req->resp_size;
        conf.len = &req->resp_len;
        conf.cb = cb;
        snprintf(conf.user, sizeof(conf.user), "%s", req->user);
        snprintf(conf.ip_addr, sizeof(conf.ip_addr), "%s", req->ip_addr);
        conf.p2psid = req->p2psid;
        conf.sessionId = req->sessionId;
        conf.resp_no = req->resp_no;
        conf.req_usec = req->req_usec;
        snprintf(conf.reqUrl, sizeof(conf.reqUrl), "%d", req->request);

        if (global_conf.req_p2p[i].handler_req) {
            (global_conf.req_p2p[i].handler_req)(req->buf, req->buf_size, req->param, &req->param_len, &conf);
        } else {
            if (global_conf.req_p2p[i].handler_resp) {
                need_cb = SDKP2P_NEED_CALLBACK;
            } else {
                need_cb = SDKP2P_NOT_CALLBACK;
            }

            sdkp2p_send_msg(&conf, req->request, NULL, 0, need_cb);
        }

        break;
    }

    return;
}

int sdkp2p_dispatch_request(int type, struct sdkp2p_request *req, p2p_send_cb cb)
{
    if (!req) {
        return -1;
    }

    switch (type) {
        case TRANSLATE_TYPE_P2P:
            sdkp2p_dispatch_request_p2p(req, cb);
            break;

        case TRANSLATE_TYPE_SDK:
            break;

        default:
            return -1;
            break;
    }

    return 0;
}

static void sdkp2p_dispatch_reponse_p2p(int resp, void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    if (!param) {
        *datalen = sdkp2p_get_resp(SDKP2P_RESP_ERROR, buf, len);
        return;
    }

    for (i = 0; i < MAX_SKDP2P_REQ_COUNT; i++) {
        if (global_conf.req_p2p_fill[i] == 0) {
            continue;
        }

        if (resp != global_conf.req_p2p[i].resp) {
            continue;
        }

        if (global_conf.req_p2p[i].handler_resp) {
            g_p2pCurRespId = resp;
            (global_conf.req_p2p[i].handler_resp)(param, size, buf, len, datalen);
        } else {
            memcpy(buf, param, size);
        }

        break;
    }

    return;
}

int sdkp2p_dispatch_reponse(int from, int resp, void *param, int size, char *buf, int len, int *datalen)
{
    switch (from) {
        case TRANSLATE_TYPE_P2P:
            sdkp2p_dispatch_reponse_p2p(resp, param, size, buf, len, datalen);
            break;

        case TRANSLATE_TYPE_SDK:
            break;

        default:
            return -1;
            break;
    }

    return 0;
}


int sdkp2p_get_section_info(const char *pRawInfo, const char *pSection, char *pDataBuf, int nBufLen)
{
    if (!pRawInfo || !pSection || pSection[0] == 0 || !pDataBuf || nBufLen <= 0) {
        return -1;
    }
    const char *pFind = strstr(pRawInfo, pSection);
    if (!pFind) {
        return -1;
    }
    pFind += strlen(pSection);
    int i = 0;
    for (; i < nBufLen; i++) {
        if (pFind[i] == '&' || pFind[i] == 0) {
            pDataBuf[i] = 0;
            break;
        }
        pDataBuf[i] = pFind[i];
    }

    return 0;
}

int sdk_get_field_int(const char *info, const char *field, int *pVal)
{
    if (!pVal) {
        return -1;
    }
    
    int ret;
    char sValue[256] = {0};
    
    ret = sdkp2p_get_section_info(info, field, sValue, sizeof(sValue));
    if (ret) {
        *pVal = MS_INVALID_VALUE;
        return -1;
    }

    *pVal = atoi(sValue);
    return 0;
}

int sdk_get_field_string(const char *info, const char *field, char *pVal, int size)
{
    if (!pVal) {
        return -1;
    }
    
    int ret;
    char sValue[256] = {0};
    
    ret = sdkp2p_get_section_info(info, field, sValue, sizeof(sValue));
    if (ret) {
        snprintf(pVal, size, "%s", MS_INVALID_STRING);
        return -1;
    }

    snprintf(pVal, size, "%s", sValue);
    return 0;
}

static int sdkp2p_get_section_len_info(const char *pRawInfo, const char *pSection, char *pDataBuf, int nBufLen,
                                       int req_len)
{
    if (!pRawInfo || !pSection || pSection[0] == 0 || !pDataBuf || nBufLen <= 0) {
        return -1;
    }
    const char *pFind = strstr(pRawInfo, pSection);
    if (!pFind) {
        return -1;
    }
    pFind += strlen(pSection);
    int i = 0;
    for (; i < nBufLen; i++) {
        if (i == req_len || pFind[i] == 0) {
            pDataBuf[i] = 0;
            break;
        }
        pDataBuf[i] = pFind[i];
    }
    return 0;
}


int sdkp2p_get_special_info(const char *pRawInfo, const char *pSection, char *pDataBuf, int nBufLen, char *key_word)
{
    char sValue[256] = {0};
    int len = 0;

    if (!sdkp2p_get_section_info(pRawInfo, key_word, sValue, sizeof(sValue))) {
        len = atoi(sValue);
    }

    if (len == 0) {
        if (!sdkp2p_get_section_info(pRawInfo, pSection, sValue, sizeof(sValue))) {
            snprintf(pDataBuf, nBufLen, "%s", sValue);
        } else {
            return -1;
        }
    } else {
        if (!sdkp2p_get_section_len_info(pRawInfo, pSection, sValue, sizeof(sValue), len)) {
            snprintf(pDataBuf, nBufLen, "%s", sValue);
        } else {
            return -1;
        }
    }

    return 0;
}

int sdkp2p_get_resp(int type, char *resp, int len)
{
    int ret = 0;
    if (type == SDKP2P_RESP_OK) {
        snprintf(resp, len, "res=%d&", 0);
        ret = strlen(resp) + 1;
    } else if (type == SDKP2P_RESP_ERROR) {
        snprintf(resp, len, "res=%d&", -1);
        ret = strlen(resp) + 1;
    }

    return ret;
}

static void sdkp2p_set_resp(struct req_conf_str *conf)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *(conf->req_usec) = tv.tv_sec * 1000000 + tv.tv_usec;
    *(conf->resp_no) = conf->response;
}

void sdkp2p_send_msg(struct req_conf_str *conf, int msg, void *data, int datalen, int need_cb)
{
    UInt64 sessionId;
    
    if (!conf) {
        return;
    }

    if (need_cb == SDKP2P_NEED_CALLBACK) {
        sdkp2p_set_resp(conf);
        sessionId = conf->sessionId;
    } else {
        sessionId = SDKP2P_SESSIONID_INVALID;
    }

    conf->cb(SOCKET_TYPE_CORE, msg, data, datalen, sessionId);
}

long long sdkp2p_get_chnmask_to_atoll(char chnmask[])
{
    long long ret = 0;
    int i = 0;

    for (i = 0; i < MAX_LEN_64; i++) {
        if (chnmask[i] == '1') {
            ret |= (long long)1 << i;
        }
    }

    return ret;
}

void sdkp2p_common_write_log_batch(struct req_conf_str *conf, int main_type, int sub_type, int param_type,
                                   long long channels)
{
    struct req_write_log_batch log_batch;

    memset(&log_batch, 0, sizeof(struct req_write_log_batch));
    snprintf(log_batch.log.log_data_info.ip, sizeof(log_batch.log.log_data_info.ip), "%s", conf->ip_addr);
    snprintf(log_batch.log.log_data_info.user, sizeof(log_batch.log.log_data_info.user), "%s", conf->user);
    log_batch.log.log_data_info.mainType = main_type;
    log_batch.log.log_data_info.subType = sub_type;
    log_batch.log.log_data_info.parameter_type = param_type;
    log_batch.channels = channels;
    sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE_BATCH, (void *)&log_batch, sizeof(struct req_write_log_batch),
                    SDKP2P_NOT_CALLBACK);
}

void sdkp2p_common_write_log(struct req_conf_str *conf, int main_type, int sub_type, int param_type, int chnid,
                             int detail_type, void *detail_data, int detail_size)
{
    struct log_data log_data;

    snprintf(log_data.log_data_info.ip, sizeof(log_data.log_data_info.ip), "%s", conf->ip_addr);
    snprintf(log_data.log_data_info.user, sizeof(log_data.log_data_info.user), "%s", conf->user);
    log_data.log_data_info.mainType = main_type;
    log_data.log_data_info.subType = sub_type;
    log_data.log_data_info.parameter_type = param_type;
    log_data.log_data_info.chan_no = chnid;
    if (detail_data && detail_size > 0) {
        msfs_log_pack_detail(&log_data, detail_type, detail_data, detail_size);
    }
    sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE, (void *)&log_data, sizeof(struct log_data), SDKP2P_NOT_CALLBACK);
}

int sdkp2p_get_schedule_by_buff(SMART_SCHEDULE *schedule, const char *buff, char *mask)
{
    int i = 0, j = 0;
    char tmp[256] = {0};
    char sValue[256] = {0};
    int flag = 0;

    if (!schedule || !buff || !mask) {
        return -1;
    }

    memset(schedule, 0x0, sizeof(SMART_SCHEDULE));
    for (i = 0; i < MAX_DAY_NUM; i++) {
        snprintf(tmp, sizeof(tmp), "%swe[%d]=", mask, i);//wholeday_enable
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            schedule->schedule_day[i].wholeday_enable = atoi(sValue);
            flag++;
        }

        snprintf(tmp, sizeof(tmp), "%swt[%d]=", mask, i);//wholeday_action_type
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            schedule->schedule_day[i].wholeday_action_type = atoi(sValue);
            flag++;
        }

        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(tmp, sizeof(tmp), "%ss[%d][%d]=", mask, i, j);//start_time
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].start_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].start_time), "%s", sValue);
                flag++;
            }
            snprintf(tmp, sizeof(tmp), "%se[%d][%d]=", mask, i, j);//end_time
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].end_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].end_time), "%s", sValue);
                flag++;
            }
            snprintf(tmp, sizeof(tmp), "%st[%d][%d]=", mask, i, j);//action_type
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                schedule->schedule_day[i].schedule_item[j].action_type = atoi(sValue);
                flag++;
            }
        }
    }

    return flag?0:-1;
}

//mask 前缀
int sdkp2p_get_buff_by_schedule(SMART_SCHEDULE *schedule, char *buff, int len, char *mask)
{
    int i = 0, j = 0;

    if (!schedule || !buff || len < 0) {
        return -1;
    }


    memset(buff, 0x0, len);
    for (i = 0; i < MAX_DAY_NUM; i++) {
        snprintf(buff + strlen(buff), len - strlen(buff), "%swe[%d]=%d&", mask, i,
                 schedule->schedule_day[i].wholeday_enable); //wholeday_enable
        snprintf(buff + strlen(buff), len - strlen(buff), "%swt[%d]=%d&", mask, i,
                 schedule->schedule_day[i].wholeday_action_type);//wholeday_action_type
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(buff + strlen(buff), len - strlen(buff), "%ss[%d][%d]=%s&", mask, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time);//start_time
            snprintf(buff + strlen(buff), len - strlen(buff), "%se[%d][%d]=%s&", mask, i, j,
                     schedule->schedule_day[i].schedule_item[j].end_time);//end_time
            snprintf(buff + strlen(buff), len - strlen(buff), "%st[%d][%d]=%d&", mask, i, j,
                     schedule->schedule_day[i].schedule_item[j].action_type);//action_type
        }
    }

    return 0;
}

long long sdkp2p_get_batch_by_buff(const char *buff, char *key)
{
    long long batch = 0;
    char sValue[256] = {0};
    int i = 0;

    memset(sValue, 0x0, sizeof(sValue));
    if (!sdkp2p_get_section_info(buff, key, sValue, sizeof(sValue))) {
        for (i = 0; i < MAX_CAMERA; i++) {
            if (sValue[i] == '1') {
                batch |= (long long)1 << i;
            }
        }
    }

    return batch;
}

Uint64 get_batch_by_json(cJSON *obj, const char *key)
{
    Uint64 batch = 0;
    cJSON *batchObj = NULL;
    int i;
    
    batchObj = cJSON_GetObjectItem(obj, key);
    if (!batchObj) {
        return 0;
    }

    for (i = 0; i < MAX_CAMERA; ++i) {
        if (batchObj->valuestring[i] == '1') {
            batch |= (Uint64)1 << i;
        }
    }

    return batch;
}

int get_schedule_by_cjson(cJSON *obj, const char *key, SMART_SCHEDULE *schedule)
{
    int i = 0, j = 0;
    cJSON *scheArr, *dayArr, *itemArr;

    if (!obj || !key || !schedule) {
        return -1;
    }

    scheArr = cJSON_GetObjectItem(obj, key);
    if (!scheArr) {
        return -1;
    }
    memset(schedule, 0, sizeof(SMART_SCHEDULE));
    for (i = 0; i < MAX_DAY_NUM; ++i) {
        dayArr = cJSON_GetArrayItem(scheArr, i);
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; ++j) {
            itemArr = cJSON_GetArrayItem(dayArr, j);
            snprintf(schedule->schedule_day[i].schedule_item[j].start_time,
                     sizeof(schedule->schedule_day[i].schedule_item[j].start_time), 
                     cJSON_GetArrayItem(itemArr, 0)->valuestring);
            snprintf(schedule->schedule_day[i].schedule_item[j].end_time,
                     sizeof(schedule->schedule_day[i].schedule_item[j].end_time), 
                     cJSON_GetArrayItem(itemArr, 1)->valuestring);
            schedule->schedule_day[i].schedule_item[j].action_type = cJSON_GetArrayItem(itemArr, 2)->valueint;
        }
    }

    return 0;
}

cJSON *get_json_schedule_object(struct smart_event_schedule *schedule)
{
    int i = 0;
    int j = 0;
    int itemId = 0;
    cJSON *scheduleArray;
    cJSON *dayArray[MAX_PLAN_NUM_PER_DAY];
    cJSON *itemObj[MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY];

    scheduleArray = cJSON_CreateArray();
    for (i = 0; i < MAX_DAY_NUM; i++) {
        dayArray[i] = cJSON_CreateArray();
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            itemId = i * j;
            itemObj[itemId] = cJSON_CreateObject();
            cJSON_AddStringToObject(itemObj[itemId], "startTime", schedule->schedule_day[i].schedule_item[j].start_time);
            cJSON_AddStringToObject(itemObj[itemId], "endTime", schedule->schedule_day[i].schedule_item[j].end_time);
            cJSON_AddNumberToObject(itemObj[itemId], "actionType", schedule->schedule_day[i].schedule_item[j].action_type);

            cJSON_AddItemToArray(dayArray[i], itemObj[itemId]);
        }
        cJSON_AddItemToArray(scheduleArray, dayArray[i]);
    }

    return scheduleArray;
}


int get_http_params_by_cjson(int id, cJSON *obj, const char *key, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms)
{
    if (!obj || !key || !pHttpPrms) {
        return -1;
    }

    cJSON *httpObj = NULL;
    cJSON *item = NULL;
    
    httpObj = cJSON_GetObjectItem(obj, key);
    if (!httpObj) {
        return -1;
    }

    pHttpPrms->id = id;
    
    item = cJSON_GetObjectItem(httpObj, "url");
    if (item) {
        snprintf(pHttpPrms->url, sizeof(pHttpPrms->url), item->valuestring);
    }
    
    item = cJSON_GetObjectItem(httpObj, "username");
    if (item) {
        snprintf(pHttpPrms->username, sizeof(pHttpPrms->username), item->valuestring);
    }

    item = cJSON_GetObjectItem(httpObj, "password");
    if (item) {
        snprintf(pHttpPrms->password, sizeof(pHttpPrms->password), item->valuestring);
    }

    return 0;
}

int get_white_params_by_cjson(int chnId, cJSON *obj, char *key, WHITE_LED_PARAMS_EVTS *params)
{
    if (!obj || !key || !params) {
        return -1;
    }

    int i = 0;
    int size;
    cJSON *arr, *arrItem, *item;

    arr = cJSON_GetObjectItem(obj, key);
    if (arr) {
        return -1;
    }

    params->ledPrms[chnId][0].chnid = chnId;

    
    size = cJSON_GetArraySize(arr);
    for (i = 0; i < size; ++i) {
        arrItem = cJSON_GetArrayItem(arr, i);
        if (arrItem == NULL) {
            return -1;
        }
    
        params->ledPrms[chnId][i].chnid = chnId;
        
        item = cJSON_GetObjectItem(arrItem, "chnId");
        if (item) {
            params->ledPrms[chnId][i].acto_chn_id = item->valueint;
        }

        item = cJSON_GetObjectItem(arrItem, "mode");
        if (item) {
            params->ledPrms[chnId][i].flash_mode = item->valueint;
        }

        item = cJSON_GetObjectItem(arrItem, "time");
        if (item) {
            params->ledPrms[chnId][i].flash_mode = item->valueint;
        }
    }

    return 0;
}

int get_ptz_params_by_cjson(int id, EVENT_IN_TYPE_E type, cJSON *obj, char *key, void *pPtzPrms)
{
    if (sdkp2p_check_chnid(id) || !obj || !key || !pPtzPrms) {
        return -1;
    }

    int i = 0;
    int size = -1;
    PEOPLECNT_PTZ_PARAMS *pPcntPtzPrms = NULL;
    PTZ_ACTION_PARAMS * pOtherPtzPrms =NULL;
    cJSON *arr = NULL;
    cJSON *arrItem = NULL;
    cJSON *item = NULL;
    
    arr = cJSON_GetObjectItem(obj, "ptzParams");
    if (!arr) {
        return -1;
    }

    if (type == PRIVATE_PEOPLE_CNT) {
        pPcntPtzPrms = pPtzPrms;
    } else {
        pOtherPtzPrms = pPtzPrms;
    }
    
    size = cJSON_GetArraySize(arr);
    for (i = 0; i < size && i < MAX_REAL_CAMERA; ++i) {
        arrItem = cJSON_GetArrayItem(arr, i);
        if (arrItem) {
            return -1;
        }

        if (type == PRIVATE_PEOPLE_CNT) {
            pPcntPtzPrms->groupid = id;
            
            item = cJSON_GetObjectItem(arrItem, "acto_ptz_channel");
            if (item) {
                pPcntPtzPrms->acto_ptz_channel = item->valueint;
            }

            item = cJSON_GetObjectItem(arrItem, "acto_ptz_type");
            if (item) {
                pPcntPtzPrms->acto_ptz_type = item->valueint;
            }

            item = cJSON_GetObjectItem(arrItem, "acto_ptz_preset");
            if (item) {
                pPcntPtzPrms->acto_ptz_preset = item->valueint;
            }

            item = cJSON_GetObjectItem(arrItem, "acto_ptz_patrol");
            if (item) {
                pPcntPtzPrms->acto_ptz_patrol = item->valueint;
            }

            item = cJSON_GetObjectItem(arrItem, "acto_ptz_preset");
            if (item) {
                pPcntPtzPrms->acto_ptz_pattern = item->valueint;
            }
        } else {
            pOtherPtzPrms->chn_id = id;
            
            item = cJSON_GetObjectItem(arrItem, "acto_ptz_channel");
            if (item) {
                pOtherPtzPrms->acto_ptz_channel = item->valueint;
            }
            
            item = cJSON_GetObjectItem(arrItem, "acto_ptz_type");
            if (item) {
                pOtherPtzPrms->acto_ptz_type = item->valueint;
            }
            
            item = cJSON_GetObjectItem(arrItem, "acto_ptz_preset");
            if (item) {
                pOtherPtzPrms->acto_ptz_preset = item->valueint;
            }
            
            item = cJSON_GetObjectItem(arrItem, "acto_ptz_patrol");
            if (item) {
                pOtherPtzPrms->acto_ptz_patrol = item->valueint;
            }
            
            item = cJSON_GetObjectItem(arrItem, "acto_ptz_preset");
            if (item) {
                pOtherPtzPrms->acto_ptz_pattern = item->valueint;
            }
        }
    }

    return 0;
}

cJSON *get_json_schedule_object_ex(struct alarm_in_schedule *schedule)
{
    int i = 0;
    int j = 0;
    int itemId = 0;
    cJSON *scheduleArray;
    cJSON *dayArray[MAX_PLAN_NUM_PER_DAY];
    cJSON *itemObj[MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY];

    scheduleArray = cJSON_CreateArray();
    for (i = 0; i < MAX_DAY_NUM; i++) {
        dayArray[i] = cJSON_CreateArray();
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            itemId = i * j;
            itemObj[itemId] = cJSON_CreateObject();
            cJSON_AddStringToObject(itemObj[itemId], "startTime", schedule->schedule_day[i].schedule_item[j].start_time);
            cJSON_AddStringToObject(itemObj[itemId], "endTime", schedule->schedule_day[i].schedule_item[j].end_time);
            cJSON_AddNumberToObject(itemObj[itemId], "actionType", schedule->schedule_day[i].schedule_item[j].action_type);

            cJSON_AddItemToArray(dayArray[i], itemObj[itemId]);
        }
        cJSON_AddItemToArray(scheduleArray, dayArray[i]);
    }

    return scheduleArray;
}

cJSON *get_json_schedule_object_ipc(struct smart_event_schedule *schedule)
{
    int i = 0;
    int j = 0;
    int itemId = 0;
    cJSON *scheduleArray;
    cJSON *dayArray[MAX_PLAN_NUM_PER_DAY_IPC];
    cJSON *itemObj[MAX_DAY_NUM_IPC * MAX_PLAN_NUM_PER_DAY_IPC];

    scheduleArray = cJSON_CreateArray();
    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        dayArray[i] = cJSON_CreateArray();
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            itemId = i * j;
            itemObj[itemId] = cJSON_CreateObject();
            cJSON_AddStringToObject(itemObj[itemId], "startTime", schedule->schedule_day[i].schedule_item[j].start_time);
            cJSON_AddStringToObject(itemObj[itemId], "endTime", schedule->schedule_day[i].schedule_item[j].end_time);

            cJSON_AddItemToArray(dayArray[i], itemObj[itemId]);
        }
        cJSON_AddItemToArray(scheduleArray, dayArray[i]);
    }

    return scheduleArray;
}

cJSON *get_json_white_params_object(int chnId, const char *dbTable)
{
    int i = 0;
    int cnt = 0;
    cJSON *item = NULL;
    cJSON *array = NULL;
    WLED_INFO whiteInfo;
    WHITE_LED_PARAMS_EVTS whiteParams;

    whiteInfo.chnid = chnId;
    snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", dbTable);
    memset(&whiteParams, 0, sizeof(WHITE_LED_PARAMS_EVTS));
    read_whiteled_params(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnId][0], &whiteInfo, &cnt);

    array = cJSON_CreateArray();
    for (i = 0; i < MAX_REAL_CAMERA && i < cnt; i++) {
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "chnid", whiteParams.ledPrms[chnId][i].acto_chn_id);
        cJSON_AddNumberToObject(item, "mode", whiteParams.ledPrms[chnId][i].flash_mode);
        cJSON_AddNumberToObject(item, "time", whiteParams.ledPrms[chnId][i].flash_time);
        cJSON_AddItemToArray(array, item);
    }

    return array;
}

cJSON *get_json_http_params_object(int id, const char *dbTable)
{
    cJSON *obj = NULL;
    HTTP_NOTIFICATION_PARAMS_S httpPrms;

    memset(&httpPrms, 0, sizeof(HTTP_NOTIFICATION_PARAMS_S));
    read_http_notification_params(SQLITE_FILE_NAME, &httpPrms, dbTable, id);

    obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "id", id);
    cJSON_AddStringToObject(obj, "url", httpPrms.url);
    cJSON_AddStringToObject(obj, "username", httpPrms.username);
    cJSON_AddStringToObject(obj, "password", httpPrms.password);

    return obj;
}

cJSON *get_json_chn_alarmout_object(const char *triChnAlarms1, const char *triChnAlarms2)
{
    cJSON *array = NULL;

    array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, cJSON_CreateString(triChnAlarms1));
    cJSON_AddItemToArray(array, cJSON_CreateString(triChnAlarms2));

    return array;
}

cJSON *get_json_ptz_params_object(int type, int id)
{
    int i = 0;
    int cnt = 0;
    cJSON *obj = NULL;
    cJSON *array = NULL;

    struct ptz_action_params ptzActionParams[MAX_CAMERA];
    memset(&ptzActionParams, 0, sizeof(struct ptz_action_params) * MAX_CAMERA);
    array = cJSON_CreateArray();
    if (type == PRIVATE_PEOPLE_CNT) {
        PEOPLECNT_PTZ_PARAMS ptzInfo[MAX_REAL_CAMERA];
        read_peoplecnt_ptz_params(SQLITE_FILE_NAME, ptzInfo, id, &cnt);
        for (i = 0; i < cnt && i < MAX_REAL_CAMERA; i++) {
            if (ptzInfo[i].acto_ptz_channel <= 0) {
                continue;
            }

            obj = cJSON_CreateObject();
            cJSON_AddNumberToObject(obj, "acto_ptz_channel", ptzInfo[i].acto_ptz_channel);
            cJSON_AddNumberToObject(obj, "acto_ptz_type", ptzInfo[i].acto_ptz_type);
            cJSON_AddNumberToObject(obj, "acto_ptz_preset", ptzInfo[i].acto_ptz_preset);
            cJSON_AddNumberToObject(obj, "acto_ptz_patrol", ptzInfo[i].acto_ptz_patrol);
            cJSON_AddItemToArray(array, obj);
        }
    } else {
        read_ptz_params(SQLITE_FILE_NAME, ptzActionParams, type, id, &cnt);
        for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
            if (ptzActionParams[i].acto_ptz_channel > 0) {
                obj = cJSON_CreateObject();
                cJSON_AddNumberToObject(obj, "acto_ptz_channel", ptzActionParams[i].acto_ptz_channel);
                cJSON_AddNumberToObject(obj, "acto_ptz_type", ptzActionParams[i].acto_ptz_type);
                cJSON_AddNumberToObject(obj, "acto_ptz_preset", ptzActionParams[i].acto_ptz_preset);
                cJSON_AddNumberToObject(obj, "acto_ptz_patrol", ptzActionParams[i].acto_ptz_patrol);
                cJSON_AddItemToArray(array, obj);
            }
        }
    }

    return array;
}

cJSON *get_json_pcnt_white_params_object(int groupid)
{
    int i = 0;
    int cnt = 0;
    cJSON *item = NULL;
    cJSON *array = NULL;
    PEOPLECNT_WLED_PARAMS whiteParams[MAX_REAL_CAMERA];

    read_peoplecnt_wled_params(SQLITE_FILE_NAME, &whiteParams[0], groupid, &cnt);

    array = cJSON_CreateArray();
    for (i = 0; i < MAX_REAL_CAMERA && i < cnt; i++) {
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "chnid", whiteParams[i].acto_chn_id);
        cJSON_AddNumberToObject(item, "mode", whiteParams[i].flash_mode);
        cJSON_AddNumberToObject(item, "time", whiteParams[i].flash_time);
        cJSON_AddItemToArray(array, item);
    }

    return array;
}

int get_audio_file_id_mask_ex()
{
    int i = 0;
    int idMask = 0;
    AUDIO_FILE audioFile[MAX_AUDIO_FILE_NUM] = {{0}};
    read_alarm_audio_file(SQLITE_FILE_NAME, audioFile);
    for (i = 0; i < MAX_AUDIO_FILE_NUM; i++) {
        if (audioFile[i].enable == 1) {
            idMask |= 1 << i;
        }
    }

    return idMask;
}

int get_common_num_json(char *fieldName, void *data, int dataLen, char *resp, int respLen)
{
    if (!fieldName || !fieldName[0] || !data || dataLen < sizeof(int) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    int res = *(int *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, fieldName, res);

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

void sdkp2p_json_string_free(char *data)
{
    if (data) {
        cJSON_free(data);
    }
    
    return;
}

int sdkp2p_json_resp(char *resp, int len, SDK_ERROR_CODE code, int *result)
{
    if (result) {
        snprintf(resp, len, "{\"status\":%d, \"res\":%d}", code, *result);
    } else {
        snprintf(resp, len, "{\"status\":%d}", code);
    }
    
    return (strlen(resp) + 1);
}

int sdkp2p_check_chnid(int chnid)
{
    if (chnid < 0 || chnid >= MAX_REAL_CAMERA) {
        return -1;
    }

    return 0;
}

int sdkp2p_chnmask_to_str(Uint64 chnMask, char *buf, int size)
{
    if (!buf || size <= MAX_REAL_CAMERA) {
        return -1;
    }

    int i, state;

    memset(buf, 0, size);
    for (i = 0; i < MAX_REAL_CAMERA; ++i) {
        state = ((chnMask >> i) & 1);
        snprintf(buf + strlen(buf), size - strlen(buf), "%d", state);
    }

    return 0;
}

int sdkp2p_req_common_get_chnid(const char *buf, int bufLen)
{
    if (!buf || bufLen <= 0) {
        return MS_INVALID_VALUE;
    }

    char sValue[256] = {0};
    int chnId = MS_INVALID_VALUE;

    if (sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        return MS_INVALID_VALUE;
    }
    chnId = atoi(sValue);

    if (sdkp2p_check_chnid(chnId)) {
        return MS_INVALID_VALUE;
    }

    return chnId;
}

void sdkp2p_resp_common_set(void *param, int size, char *buf, int len, int *datalen)
{
    if (!param || size <= 0) {
        *datalen = sdkp2p_json_resp(buf, len, REQUEST_SUCCESS, NULL);
        return;
    }

    int resp = *(int *)param;

    *datalen = sdkp2p_json_resp(buf, len, REQUEST_SUCCESS, &resp);
}

int p2p_format_json(char *buf)
{
    if (buf && strstr(buf, "&format=json")) {
        return 1;
    }

    return 0;
}

MS_BOOL sdk_is_ms_lpr(IPC_AITYPE_E aitype)
{
    if (aitype == IPC_AITYPE_LPR_AM
        || aitype == IPC_AITYPE_LPR_AP
        || aitype == IPC_AITYPE_LPR_EU
        || aitype == IPC_AITYPE_LPR_ME) {
        return MS_TRUE;
    }

    return MS_FALSE;
}

int sdkp2p_init()
{
    memset(&global_conf, 0x0, sizeof(struct sdkp2p_translate_conf));
    p2p_camera_load_module();
    p2p_disk_load_module();
    p2p_event_load_module();
    p2p_network_load_module();
    p2p_playback_load_module();
    p2p_ptz_load_module();
    p2p_record_load_module();
    p2p_status_load_module();
    p2p_system_load_module();
    p2p_user_load_module();
    p2p_schedule_load_module();
    p2p_qtctrl_load_module();

    return 0;
}

int sdkp2p_deinit()
{
    //p2p_camera_unload_module();
    //p2p_disk_unload_module();
    //p2p_event_unload_module();
    //p2p_network_unload_module();
    //p2p_playback_unload_module();
    //p2p_ptz_unload_module();
    //p2p_record_unload_module();
    //p2p_status_unload_module();
    //p2p_system_unload_module();
    //p2p_user_unload_module();
    //p2p_schedule_unload_module();
    //p2p_qtctrl_unload_module();
    memset(&global_conf, 0x0, sizeof(struct sdkp2p_translate_conf));

    return 0;
}

