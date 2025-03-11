#include "sdk_util.h"


int get_face_support_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(int) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    int support = *(int *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "support", support);

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_ai_cam_cnt_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(int) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    int res = *(int *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "res", res);

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_ipc_custom_param_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(struct resp_ipc_custom_params) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    struct resp_ipc_custom_params *info = (struct resp_ipc_custom_params *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "res", info->res);
    cJSON_AddNumberToObject(json, "chnid", info->chnid);
    cJSON_AddStringToObject(json, "data", info->data);

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

static cJSON * get_image_enhancement_sche(struct schedule_day *sche, int cnt)
{
    int i = 0, j = 0;
    cJSON *scheArray, *dayArray, *itemObj;

    scheArray = cJSON_CreateArray();
    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        dayArray = cJSON_CreateArray();
        for (j = 0; j < cnt; j++) {
            itemObj = cJSON_CreateObject();
            cJSON_AddStringToObject(itemObj, "startTime", sche[i].schedule_item[j].start_time);
            cJSON_AddStringToObject(itemObj, "endTime", sche[i].schedule_item[j].end_time);
            cJSON_AddNumberToObject(itemObj, "actionType", sche[i].schedule_item[j].action_type);

            cJSON_AddItemToArray(dayArray, itemObj);
        }
        cJSON_AddItemToArray(scheArray, dayArray);
    }

    return scheArray;
}

static int check_exposure_add(cJSON *array, int level, int pTime)
{
    int i = 0;
    int gainLevel = 0;
    int exposureTime = 0;
    cJSON *obj = NULL;
    cJSON *item = NULL;
    int cnt = cJSON_GetArraySize(array);
    if (level <= 0 && pTime <= 0) {
        return -1;
    }
    if (cnt > 0) {
        for (i = 0; i < cnt; i++) {
            gainLevel = -1;
            exposureTime = -1;
            obj = cJSON_GetArrayItem(array, i);
            item = cJSON_GetObjectItem(obj, "gainLevel");
            if (item) {
                gainLevel = item->valueint;
            }
            item = cJSON_GetObjectItem(obj, "exposureTime");
            if (item) {
                exposureTime = item->valueint;
            }
            if (gainLevel == level && exposureTime == pTime) {
                return 1;
            }
        }
    }

    return 0;
}

static int get_image_enhancement_exposure_sche(cJSON *json, struct exposure_schedule_day *sche)
{
    int i = 0, j = 0;
    cJSON *array, *obj, *scheArray, *dayArray, *itemObj;

    scheArray = cJSON_CreateArray();
    array = cJSON_CreateArray();
    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        dayArray = cJSON_CreateArray();
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            itemObj = cJSON_CreateObject();
            cJSON_AddStringToObject(itemObj, "startTime", sche[i].schedule_item[j].start_time);
            cJSON_AddStringToObject(itemObj, "endTime", sche[i].schedule_item[j].end_time);
            cJSON_AddNumberToObject(itemObj, "actionType", sche[i].schedule_item[j].action_type);
            cJSON_AddNumberToObject(itemObj, "exposureTime", sche[i].schedule_item[j].exposureTime);
            cJSON_AddNumberToObject(itemObj, "gainLevel", sche[i].schedule_item[j].gainLevel);
            cJSON_AddItemToArray(dayArray, itemObj);

            if (!check_exposure_add(array, sche[i].schedule_item[j].gainLevel, sche[i].schedule_item[j].exposureTime)) {
                obj = cJSON_CreateObject();
                cJSON_AddNumberToObject(obj, "gainLevel", sche[i].schedule_item[j].gainLevel);
                cJSON_AddNumberToObject(obj, "exposureTime", sche[i].schedule_item[j].exposureTime);
                cJSON_AddItemToArray(array, obj);
            }
        }
        cJSON_AddItemToArray(scheArray, dayArray);
    }
    cJSON_AddItemToObject(json, "exposure", array);
    cJSON_AddItemToObject(json, "exposureSche", scheArray);

    return 0;
}

int get_image_enhancement_info_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(struct EnhancementSchedule) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    struct EnhancementSchedule *info = (struct EnhancementSchedule *)data;
    int res = (info->chnId >= 0 && info->chnId < MAX_REAL_CAMERA) ? SDK_SUC : SDK_ERR;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", res);

    if (res == SDK_SUC) {
        cJSON_AddItemToObject(json, "bwhSchedule", 
            get_image_enhancement_sche(info->bwhSchedule, MAX_PLAN_NUM_PER_DAY));
        cJSON_AddItemToObject(json, "whiteBalanceSche", 
            get_image_enhancement_sche(info->whiteBalanceSche, MAX_PLAN_NUM_PER_DAY_IPC));
        get_image_enhancement_exposure_sche(json, info->exposureSche);
    }

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_vca_base_info_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(struct MsVcaBaseInfo) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    struct MsVcaBaseInfo *baseInfo = (struct MsVcaBaseInfo *)(data);

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", (baseInfo->res == 0 ? SDK_SUC : SDK_ERR));

    cJSON_AddNumberToObject(json, "chnId", baseInfo->chnId);
    cJSON_AddNumberToObject(json, "vcaSupport", baseInfo->vcaSupport);
    cJSON_AddNumberToObject(json, "vcaType", baseInfo->vcaType);
    cJSON_AddNumberToObject(json, "vcaLicenseStatus", baseInfo->vcaLicenseStatus);
    cJSON_AddStringToObject(json, "sdkVersion", baseInfo->sdkVersion);
    cJSON_AddStringToObject(json, "vcaLicense", baseInfo->vcaLicense);

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_heatmap_support_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(int) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    int support = *(int *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "support", support);

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

SDK_ERROR_CODE get_ipc_peoplecnt_linecnt_str(const char *buff, UInt64 *info)
{
    *info = (UInt64)sdkp2p_get_batch_by_buff(buff, "chnMask=");
    return REQUEST_SUCCESS;
}

int get_ipc_peoplecnt_linecnt_json(MS_PEOPLECNT_DATA *data, int cnt, UInt64 mask, char *resp, int respLen)
{
    if (!data || !mask || !resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *obj = NULL;
    cJSON *lineCnt;
    cJSON *inCnt;
    cJSON *outCnt;
    cJSON *sumCnt;
    cJSON *capacityCnt;
    cJSON *item;
    int i, j;
    
    obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(obj, "status", SDK_SUC);
    lineCnt = cJSON_AddArrayToObject(obj, "lineCnt");
    for (i = 0; i < cnt && i < MAX_REAL_CAMERA; ++i) {
        if (!(mask>>i&0x01)) {
            continue;
        }

        item = cJSON_CreateObject();
        cJSON_AddItemToArray(lineCnt, item);
        cJSON_AddNumberToObject(item, "chnId", data[i].chnId);
        inCnt = cJSON_AddArrayToObject(item, "inCnt");
        outCnt = cJSON_AddArrayToObject(item, "outCnt");
        sumCnt = cJSON_AddArrayToObject(item, "sumCnt");
        capacityCnt = cJSON_AddArrayToObject(item, "capacityCnt");

        for (j = 0; j < MAX_IPC_PCNT_LINE; ++j) {
            cJSON_AddItemToArray(inCnt, cJSON_CreateNumber(data[i].lineInCnt[j]));
            cJSON_AddItemToArray(outCnt, cJSON_CreateNumber(data[i].lineOutCnt[j]));
            cJSON_AddItemToArray(sumCnt, cJSON_CreateNumber(data[i].lineSumCnt[j]));
            cJSON_AddItemToArray(capacityCnt, cJSON_CreateNumber(data[i].lineCapacityCnt[j]));
        }
    }

    sResp = cJSON_PrintUnformatted(obj);
    cJSON_Delete(obj);
    if (!sResp) {
        return -1;
    }
    
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}

int set_vca_common_region_by_text(const char *req, struct ms_smart_event_info *param)
{
    if (!req || !param) {
        return -1;
    }

    char sValue[512] = {0};
    char tmp[256] = {0};
    int i, j;

    if (!sdkp2p_get_section_info(req, "copyChn=", sValue, sizeof(sValue))) {
        snprintf(param->copyChn, sizeof(param->copyChn), sValue);
    }

    if (!sdkp2p_get_section_info(req, "regionType=", sValue, sizeof(sValue))) {
        param->regionType = atoi(sValue);
    }
    if (param->regionType != REGION_MULTIPLE && param->regionType != REGION_PRESET) {
        return 0;
    }
    
    if (!sdkp2p_get_section_info(req, "regionScene=", sValue, sizeof(sValue))) {
        param->regionScene = atoi(sValue);
    }

    for (i = 0; i < MAX_SMART_REGION; ++i) {
        snprintf(tmp, sizeof(tmp), "regionEnable%d=", i);
        if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
            param->regionInfo[i].enable = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "regionObjType%d=", i);
        if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
            param->regionInfo[i].objType = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "regionMinTime%d=", i);
        if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
            param->regionInfo[i].minTime = atoi(sValue);
        }

        for (j = SCENE_NORMAL; j < MAX_SMART_SCENE; ++j) {
            snprintf(tmp, sizeof(tmp), "region%dPolygonX%d=", i, j);
            if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
                snprintf(param->regionInfo[i].region[j].polygonX, sizeof(param->regionInfo[i].region[j].polygonX), sValue);
            }

            snprintf(tmp, sizeof(tmp), "region%dPolygonY%d=", i, j);
            if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
                snprintf(param->regionInfo[i].region[j].polygonY, sizeof(param->regionInfo[i].region[j].polygonY), sValue);
            }
        }
    }

    return 0;
}

int set_vca_leftremove_region_by_text(const char *req, struct ms_smart_leftremove_info *param)
{
    if (!req || !param) {
        return -1;
    }

    char sValue[512] = {0};
    char tmp[256] = {0};
    int i, j;

    if (!sdkp2p_get_section_info(req, "copyChn=", sValue, sizeof(sValue))) {
        snprintf(param->copyChn, sizeof(param->copyChn), sValue);
    }

    if (!sdkp2p_get_section_info(req, "regionType=", sValue, sizeof(sValue))) {
        param->regionType = atoi(sValue);
    }
    if (param->regionType != REGION_MULTIPLE && param->regionType != REGION_PRESET) {
        return 0;
    }

    if (!sdkp2p_get_section_info(req, "regionScene=", sValue, sizeof(sValue))) {
        param->regionScene = atoi(sValue);
    }

    for (i = 0; i < MAX_SMART_REGION; ++i) {
        snprintf(tmp, sizeof(tmp), "regionLeftEnable%d=", i);
        if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
            param->regionInfo[i].enable = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "regionRemoveEnable%d=", i);
        if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
            param->regionInfo[i].removeEnable = atoi(sValue);
        }

        for (j = SCENE_NORMAL; j < MAX_SMART_SCENE; ++j) {
            snprintf(tmp, sizeof(tmp), "region%dPolygonX%d=", i, j);
            if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
                snprintf(param->regionInfo[i].region[j].polygonX, sizeof(param->regionInfo[i].region[j].polygonX), sValue);
            }

            snprintf(tmp, sizeof(tmp), "region%dPolygonY%d=", i, j);
            if (!sdkp2p_get_section_info(req, tmp, sValue, sizeof(sValue))) {
                snprintf(param->regionInfo[i].region[j].polygonY, sizeof(param->regionInfo[i].region[j].polygonY), sValue);
            }
        }
    }

    return 0;
}

int get_vca_common_json(void *data, int dataLen, char *resp, int respLen, SMART_EVENT_TYPE type)
{
    if (!data || dataLen < sizeof(struct ms_smart_event_info) || !resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    cJSON *obj = NULL;
    cJSON *array = NULL;
    cJSON *regionObj = NULL;
    cJSON *regionArray = NULL;
    int i, j;
    struct ms_smart_event_info *param = (struct ms_smart_event_info *)data;
    
    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);

    cJSON_AddStringToObject(json, "region", param->area);
    cJSON_AddNumberToObject(json, "enable", param->enable);
    cJSON_AddNumberToObject(json, "sensitivity", param->sensitivity);
    cJSON_AddNumberToObject(json, "objtype", param->objtype);
    cJSON_AddStringToObject(json, "polygonX", param->polygonX);
    cJSON_AddStringToObject(json, "polygonY", param->polygonY);
    if (type == ADVANCED_MOTION) {
        cJSON_AddNumberToObject(json, "ignore_time", param->ignore_time);
    } else if (type == LOITERING) {
        cJSON_AddNumberToObject(json, "time", param->min_loitering_time);
        cJSON_AddNumberToObject(json, "size", param->loitering_object_size);
    }

    cJSON_AddNumberToObject(json, "regionType", param->regionType);
    cJSON_AddNumberToObject(json, "regionScene", param->regionScene);

    if (param->regionType > REGION_SINGLE) {
        regionArray = cJSON_CreateArray();
        for (i = 0; i < MAX_SMART_REGION; i++) {
            regionObj = cJSON_CreateObject();
            cJSON_AddNumberToObject(regionObj, "enable", param->regionInfo[i].enable);
            cJSON_AddNumberToObject(regionObj, "objType", param->regionInfo[i].objType);
            if (type == LOITERING) {
                cJSON_AddNumberToObject(regionObj, "time", param->regionInfo[i].minTime);
            }

            array = cJSON_CreateArray();
            for (j = SCENE_NORMAL; j < MAX_SMART_SCENE; j++) {
                obj = cJSON_CreateObject();
                cJSON_AddStringToObject(obj, "polygonX", param->regionInfo[i].region[j].polygonX);
                cJSON_AddStringToObject(obj, "polygonY", param->regionInfo[i].region[j].polygonY);
                cJSON_AddItemToArray(array, obj);
                if (param->regionType < REGION_PRESET) {
                    break;
                }
            }
            cJSON_AddItemToObject(regionObj, "region", array);

            cJSON_AddItemToArray(regionArray, regionObj);
        }
        cJSON_AddItemToObject(json, "regionInfo", regionArray);

        array = cJSON_CreateArray();
        for (i = SCENE_NORMAL; i < MAX_SMART_SCENE; i++) {
            cJSON_AddItemToArray(array, cJSON_CreateNumber(param->presetExist[i]));
        }
        cJSON_AddItemToObject(json, "presetExist", array);
    }

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_vca_leftremove_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(struct ms_smart_leftremove_info) || !resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    cJSON *obj = NULL;
    cJSON *array = NULL;
    cJSON *regionObj = NULL;
    cJSON *regionArray = NULL;
    int i, j;
    struct ms_smart_leftremove_info *param = (struct ms_smart_leftremove_info *)data;
    
    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);

    cJSON_AddStringToObject(json, "region", param->area);
    cJSON_AddNumberToObject(json, "left_enable", param->left_enable);
    cJSON_AddNumberToObject(json, "remove_enable", param->remove_enable);
    cJSON_AddNumberToObject(json, "min_time", param->min_time);
    cJSON_AddNumberToObject(json, "sensitivity", param->sensitivity);
    cJSON_AddStringToObject(json, "polygonX", param->polygonX);
    cJSON_AddStringToObject(json, "polygonY", param->polygonY);

    cJSON_AddNumberToObject(json, "regionType", param->regionType);
    cJSON_AddNumberToObject(json, "regionScene", param->regionScene);

    if (param->regionType > REGION_SINGLE) {
        regionArray = cJSON_CreateArray();
        for (i = 0; i < MAX_SMART_REGION; i++) {
            regionObj = cJSON_CreateObject();
            cJSON_AddNumberToObject(regionObj, "leftEnable", param->regionInfo[i].enable);
            cJSON_AddNumberToObject(regionObj, "removeEnable", param->regionInfo[i].removeEnable);

            array = cJSON_CreateArray();
            for (j = SCENE_NORMAL; j < MAX_SMART_SCENE; j++) {
                obj = cJSON_CreateObject();
                cJSON_AddStringToObject(obj, "polygonX", param->regionInfo[i].region[j].polygonX);
                cJSON_AddStringToObject(obj, "polygonY", param->regionInfo[i].region[j].polygonY);
                cJSON_AddItemToArray(array, obj);
                if (param->regionType < REGION_PRESET) {
                    break;
                }
            }
            cJSON_AddItemToObject(regionObj, "region", array);

            cJSON_AddItemToArray(regionArray, regionObj);
        }
        cJSON_AddItemToObject(json, "regionInfo", regionArray);

        array = cJSON_CreateArray();
        for (i = SCENE_NORMAL; i < MAX_SMART_SCENE; i++) {
            cJSON_AddItemToArray(array, cJSON_CreateNumber(param->presetExist[i]));
        }
        cJSON_AddItemToObject(json, "presetExist", array);
    }

    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}


static cJSON *get_face_action(int chnId)
{
    cJSON *action = NULL;
    cJSON *schedulePtz = NULL;
    cJSON *scheduleEmail = NULL;
    cJSON *scheduleWhite = NULL;
    cJSON *scheduleAudible = NULL;
    cJSON *scheduleEffective = NULL;
    cJSON *whiteParams = NULL;
    cJSON *triChnAlarmOut = NULL;
    cJSON *httpSche = NULL;
    cJSON *httpPrms = NULL;
    struct smart_event smart;
    SMART_SCHEDULE schedule;

    memset(&smart, 0, sizeof(SMART_EVENT));
    read_face_event(SQLITE_FILE_NAME, &smart, chnId);

    //schedule
    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_face_effective_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    scheduleEffective = get_json_schedule_object(&schedule);

    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_face_audible_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    scheduleAudible = get_json_schedule_object(&schedule);

    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_face_mail_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    scheduleEmail = get_json_schedule_object(&schedule);

    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_face_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    schedulePtz = get_json_schedule_object(&schedule);

    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_face_whiteled_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    scheduleWhite = get_json_schedule_object(&schedule);

    whiteParams = get_json_white_params_object(chnId, FACE_WLED_PARAMS);

    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_http_notification_schedule(SQLITE_FILE_NAME, &schedule, FACE_HTTP_SCHE, chnId);
    httpSche = get_json_schedule_object(&schedule);
    httpPrms = get_json_http_params_object(chnId, FACE_HTTP_PARAMS);

    //alarm out
    triChnAlarmOut = get_json_chn_alarmout_object(smart.tri_chnout1_alarms, smart.tri_chnout2_alarms);

    action = cJSON_CreateObject();
    cJSON_AddNumberToObject(action, "chnid", chnId);
    cJSON_AddNumberToObject(action, "enable", smart.enable);
    cJSON_AddNumberToObject(action, "triAlarms", smart.tri_alarms);// 4bit 0001
    cJSON_AddNumberToObject(action, "interval", smart.buzzer_interval);//sec
    cJSON_AddNumberToObject(action, "email_interval", smart.email_interval);//sec
    cJSON_AddNumberToObject(action, "email_pic_enable", smart.email_pic_enable);//sec
    cJSON_AddStringToObject(action, "tri_channels_pic", smart.tri_channels_pic);
    cJSON_AddNumberToObject(action, "ptz_interval", smart.ptzaction_interval);//sec
    cJSON_AddNumberToObject(action, "alarmout_interval", smart.alarmout_interval);//sec
    cJSON_AddStringToObject(action, "triChannels", smart.tri_channels_ex);
    cJSON_AddItemToObject(action, "effective", scheduleEffective);
    cJSON_AddItemToObject(action, "audible", scheduleAudible);
    cJSON_AddItemToObject(action, "email", scheduleEmail);
    cJSON_AddItemToObject(action, "ptz", schedulePtz);
    cJSON_AddItemToObject(action, "triChnAlarms", triChnAlarmOut);
    cJSON_AddItemToObject(action, "white", scheduleWhite);
    cJSON_AddItemToObject(action, "whiteInfo", whiteParams);
    cJSON_AddNumberToObject(action, "white_interval", smart.whiteled_interval);
    cJSON_AddItemToObject(action, "httpSche", httpSche);
    cJSON_AddItemToObject(action, "httpParams", httpPrms);
    cJSON_AddNumberToObject(action, "http_notification_interval", smart.http_notification_interval);
    cJSON_AddNumberToObject(action, "audioId", smart.tri_audio_id);
    cJSON_AddNumberToObject(action, "audioIdMask", get_audio_file_id_mask_ex());
    cJSON_AddItemToObject(action, "ptzParams", get_json_ptz_params_object(FACE_EVT, chnId));

    return action;
}

int get_camera_face_config_json(void *data, int dataLen, char *resp, int respLen, int action)
{
    int i = 0;
    char *buff = NULL;
    cJSON *obj = NULL;
    cJSON *json = NULL;
    cJSON *array = NULL;
    
    if (!data || dataLen != sizeof(MS_FACE_CONFIG) || !resp || !respLen) {
        return -1;
    }
    MS_FACE_CONFIG *params = (MS_FACE_CONFIG *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "res", params->res);
    cJSON_AddNumberToObject(json, "chnId", params->chnId);
    cJSON_AddNumberToObject(json, "enable", params->enable);
    cJSON_AddNumberToObject(json, "width", params->width);
    cJSON_AddNumberToObject(json, "height", params->height);
    cJSON_AddNumberToObject(json, "minPixel", params->minPixel);
    cJSON_AddNumberToObject(json, "snapshotType", params->snapshotType);
    cJSON_AddNumberToObject(json, "snapshotInterval", params->snapshotInterval);
    cJSON_AddNumberToObject(json, "snapshotBackground", params->snapshotBackground);
    cJSON_AddNumberToObject(json, "poseYaw", params->poseYaw);
    cJSON_AddNumberToObject(json, "poseRoll", params->poseRoll);
    cJSON_AddNumberToObject(json, "posePitch", params->posePitch);
    cJSON_AddNumberToObject(json, "blurLimit", params->blurLimit);
    cJSON_AddStringToObject(json, "detection_polygonX", params->detection.polygonX);
    cJSON_AddStringToObject(json, "detection_polygonY", params->detection.polygonY);
    cJSON_AddNumberToObject(json, "uploadEnable", params->rule.uploadEnable);
    cJSON_AddNumberToObject(json, "uploadFileType", params->rule.uploadFileType);
    cJSON_AddNumberToObject(json, "nasEnable", params->rule.nasEnable);
    cJSON_AddNumberToObject(json, "nasFileType", params->rule.nasFileType);
    cJSON_AddNumberToObject(json, "emailEnable", params->rule.emailEnable);
    cJSON_AddNumberToObject(json, "emailFileType", params->rule.emailFileType);
    cJSON_AddNumberToObject(json, "httpEnable", params->rule.httpEnable);
    cJSON_AddNumberToObject(json, "httpNotifyInterval", params->rule.httpNotifyInterval);
    cJSON_AddStringToObject(json, "httpUrl", params->rule.httpUrl);
    cJSON_AddStringToObject(json, "httpUser", params->rule.httpUser);
    cJSON_AddStringToObject(json, "httpPassword", params->rule.httpPassword);
    cJSON_AddNumberToObject(json, "recordSeconds", params->rule.recordSeconds);
    cJSON_AddNumberToObject(json, "prerecordSeconds", params->rule.prerecordSeconds);
    cJSON_AddNumberToObject(json, "postEnable", params->postEnable);
    cJSON_AddNumberToObject(json, "postType", params->postType);

    array = cJSON_CreateArray();
    for (i = 0; i < MAX_FACE_SHIELD; i++) {
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "enable", params->shield[i].enable);
        cJSON_AddStringToObject(obj, "polygonX", params->shield[i].region.polygonX);
        cJSON_AddStringToObject(obj, "polygonY", params->shield[i].region.polygonY);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "shield", array);

    cJSON_AddNumberToObject(json, "captureMode", params->captureMode);

    array = cJSON_CreateArray();
    for (i = 0; i < FACE_CAPTURE_MAX; i++) {
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "captureQuality", params->captureQuality[i]);
        cJSON_AddNumberToObject(obj, "snapshot", params->rule.snapshot[i]);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "captureModeArray", array);

    cJSON_AddNumberToObject(json, "attributeEnable", params->attributeEnable);
    cJSON_AddNumberToObject(json, "attributeType", params->attributeType);
    cJSON_AddNumberToObject(json, "mosaicEnable", params->mosaicEnable);
    cJSON_AddNumberToObject(json, "mosaicModeStatus", params->mosaicModeStatus);
    cJSON_AddNumberToObject(json, "mutuallyExclusive", params->mutuallyExclusive);
    cJSON_AddNumberToObject(json, "isFaceCustomizeModeExists", params->isFaceCustomizeModeExists);
    cJSON_AddNumberToObject(json, "isNtPlatform", params->isNtPlatform);
    cJSON_AddNumberToObject(json, "ntFaceMosaicSupport", params->ntFaceMosaicSupport);

    if (action) {
        cJSON_AddItemToObject(json, "action", get_face_action(params->chnId));
    }

    buff = cJSON_Print(json);
    cJSON_Delete(json);
    if (!buff) {
        return -1;
    }
    snprintf(resp, respLen, "%s", buff);
    cJSON_free(buff);
    return 0;
}

int get_ipc_file_data_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen <= 0 || !resp || respLen <= 0) {
        return -1;
    }

    FILE_INFO_S fileInfo;
    char filename[64] = {0};
    char *sResp = NULL;
    cJSON *root = NULL;
    char *buf = NULL;
    int bufSize = 0;
    
    memset(&fileInfo, 0, sizeof(fileInfo));
    fileInfo.data = (char *)data;
    fileInfo.filenameSize = 64;
    snprintf(filename, sizeof(filename), data);
    fileInfo.fileSize = *(int *)(fileInfo.data + fileInfo.filenameSize);
    fileInfo.isBinary = *(Uint8 *)(fileInfo.data + fileInfo.filenameSize + sizeof(int));

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", REQUEST_SUCCESS);
    cJSON_AddStringToObject(root, "filename", filename);
    cJSON_AddNumberToObject(root, "fileSize", fileInfo.fileSize);
    if (fileInfo.isBinary) {
        bufSize = fileInfo.fileSize * 3 / 2;
        buf = ms_malloc(bufSize);
        if (!buf) {
            msdebug(DEBUG_INF, "malloc err, malloc size = %d", bufSize);
            cJSON_Delete(root);
            return -1;
        }
        base64EncodeUrl((MS_U8 *)buf, bufSize, 
            (MS_U8 *)fileInfo.data + fileInfo.filenameSize + sizeof(fileInfo.filenameSize) + sizeof(fileInfo.isBinary), 
            fileInfo.fileSize);
        cJSON_AddStringToObject(root, "data", buf);
        
        ms_free(buf);
    } else {
        cJSON_AddStringToObject(root, "data", fileInfo.data + fileInfo.filenameSize + sizeof(fileInfo.fileSize) + sizeof(fileInfo.isBinary));
    }

    sResp = cJSON_Print(root);
    cJSON_Delete(root);
    if (!sResp) {
        return -1;
    }

    snprintf(resp, respLen, sResp);
    cJSON_free(sResp);

    return 0;
}

static void update_face_action(const char *pBuff, int chnId)
{
    int i = 0;
    int update = 0;
    char tmp[256] = {0};
    char sValue[256] = {0};
    SMART_EVENT smartEvent;
    SMART_SCHEDULE schedule;
    VCA_PTZ_ACTION_PARAMS ptzParams;
    WHITE_LED_PARAMS_EVTS whiteParams;

    memset(&smartEvent, 0, sizeof(SMART_EVENT));
    read_face_event(SQLITE_FILE_NAME, &smartEvent, chnId);

    //audio
    if (sdkp2p_get_schedule_by_buff(&schedule, pBuff, "a") == 0) {
        read_face_audible_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    }
    if (!sdkp2p_get_section_info(pBuff, "buzzer_interval=", sValue, sizeof(sValue))) {
        smartEvent.buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pBuff, "tri_audio_id=", sValue, sizeof(sValue))) {
        smartEvent.tri_audio_id = atoi(sValue);
    }

    //email
    if (sdkp2p_get_schedule_by_buff(&schedule, pBuff, "m") == 0) {
        write_face_mail_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    }
    if (!sdkp2p_get_section_info(pBuff, "email_interval=", sValue, sizeof(sValue))) {
        smartEvent.email_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pBuff, "email_pic_enable=", sValue, sizeof(sValue))) {
        smartEvent.email_pic_enable = atoi(sValue);
    }

    //ptz
    if (sdkp2p_get_schedule_by_buff(&schedule, pBuff, "p") == 0) {
        write_face_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    }
    if (!sdkp2p_get_section_info(pBuff, "ptz_interval=", sValue, sizeof(sValue))) {
        smartEvent.ptzaction_interval = atoi(sValue);
    }
    update = 0;
    memset(&ptzParams, 0, sizeof(VCA_PTZ_ACTION_PARAMS));
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        snprintf(tmp, sizeof(tmp), "ptzCh%d=", i);
        if (sdkp2p_get_section_info(pBuff, tmp, sValue, sizeof(sValue))) {
            break;
        }
        ptzParams.ptzActionParams[chnId][i].acto_ptz_channel = atoi(sValue);
        snprintf(tmp, sizeof(tmp), "ptzType%d=", i);
        if (!sdkp2p_get_section_info(pBuff, tmp, sValue, sizeof(sValue))) {
            ptzParams.ptzActionParams[chnId][i].acto_ptz_type = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "ptzPreset%d=", i);
        if (!sdkp2p_get_section_info(pBuff, tmp, sValue, sizeof(sValue))) {
            ptzParams.ptzActionParams[chnId][i].acto_ptz_preset = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "ptzPatrol%d=", i);
        if (!sdkp2p_get_section_info(pBuff, tmp, sValue, sizeof(sValue))) {
            ptzParams.ptzActionParams[chnId][i].acto_ptz_patrol = atoi(sValue);
        }
        update = 1;
    }

    if (update) {
        write_ptz_params_all(SQLITE_FILE_NAME, &ptzParams.ptzActionParams[chnId][0], FACE_EVT, chnId);
    }

    //alarm
    if (!sdkp2p_get_section_info(pBuff, "triAlarms=", sValue, sizeof(sValue))) {
        smartEvent.tri_alarms = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pBuff, "tri_chnout1_alarms=", sValue, sizeof(sValue))) {
        snprintf(smartEvent.tri_chnout1_alarms, sizeof(smartEvent.tri_chnout1_alarms), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(pBuff, "tri_chnout2_alarms=", sValue, sizeof(sValue))) {
        snprintf(smartEvent.tri_chnout2_alarms, sizeof(smartEvent.tri_chnout2_alarms), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(pBuff, "alarmout_interval=", sValue, sizeof(sValue))) {
        smartEvent.alarmout_interval = atoi(sValue);
    }
    
    //white
    if (sdkp2p_get_schedule_by_buff(&schedule, pBuff, "w") == 0) {
        write_face_whiteled_schedule(SQLITE_FILE_NAME, &schedule, chnId);
    }
    if (!sdkp2p_get_section_info(pBuff, "white_interval=", sValue, sizeof(sValue))) {
        smartEvent.whiteled_interval = atoi(sValue);
    }

    update = 0;
    memset(&whiteParams, 0, sizeof(WHITE_LED_PARAMS_EVTS));
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        snprintf(tmp, sizeof(tmp), "whiteCh%d=", i);
        if (sdkp2p_get_section_info(pBuff, tmp, sValue, sizeof(sValue))) {
            break;
        }
        whiteParams.ledPrms[chnId][i].acto_chn_id = atoi(sValue);
        snprintf(tmp, sizeof(tmp), "whiteMode%d=", i);
        if (!sdkp2p_get_section_info(pBuff, tmp, sValue, sizeof(sValue))) {
            whiteParams.ledPrms[chnId][i].flash_mode = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "whiteTime%d=", i);
        if (!sdkp2p_get_section_info(pBuff, tmp, sValue, sizeof(sValue))) {
            whiteParams.ledPrms[chnId][i].flash_time = atoi(sValue);
        }
        
        whiteParams.ledPrms[chnId][i].chnid = chnId;
        update = 1;
    }
    if (update) {
        write_whiteled_params_all(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnId][0], FACE_WLED_PARAMS, chnId);
    }

    //others
    if (!sdkp2p_get_section_info(pBuff, "triChannels=", sValue, sizeof(sValue))) {
        snprintf(smartEvent.tri_channels_ex, sizeof(smartEvent.tri_channels_ex), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(pBuff, "tri_channels_pic=", sValue, sizeof(sValue))) {
        snprintf(smartEvent.tri_channels_pic, sizeof(smartEvent.tri_channels_pic), "%s", sValue);
    }
    write_face_event(SQLITE_FILE_NAME, &smartEvent);

    return;
}


SDK_ERROR_CODE get_camera_face_config_str(const char *buff, MS_FACE_CONFIG *info)
{
    int i = 0;
    int chnId = 0;
    char tmp[256] = {0};
    char sValue[256] = {0};

    memset(info, 0, sizeof(MS_FACE_CONFIG));
    if (sdkp2p_get_section_info(buff, "chnId=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    chnId = atoi(sValue);
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
        return PARAM_ERROR;
    }
    info->chnId = chnId;

    if (!sdkp2p_get_section_info(buff, "enable=", sValue, sizeof(sValue))) {
        info->enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "width=", sValue, sizeof(sValue))) {
        info->width = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "height=", sValue, sizeof(sValue))) {
        info->height = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "minPixel=", sValue, sizeof(sValue))) {
        info->minPixel = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "captureMode=", sValue, sizeof(sValue))) {
        info->captureMode = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "snapshotType=", sValue, sizeof(sValue))) {
        info->snapshotType = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "snapshotInterval=", sValue, sizeof(sValue))) {
        info->snapshotInterval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "snapshotBackground=", sValue, sizeof(sValue))) {
        info->snapshotBackground = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "poseYaw=", sValue, sizeof(sValue))) {
        info->poseYaw = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "poseRoll=", sValue, sizeof(sValue))) {
        info->poseRoll = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "posePitch=", sValue, sizeof(sValue))) {
        info->posePitch = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "blurLimit=", sValue, sizeof(sValue))) {
        info->blurLimit = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "detection_polygonX=", sValue, sizeof(sValue))) {
        snprintf(info->detection.polygonX, sizeof(info->detection.polygonX), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "detection_polygonY=", sValue, sizeof(sValue))) {
        snprintf(info->detection.polygonY, sizeof(info->detection.polygonY), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "uploadEnable=", sValue, sizeof(sValue))) {
        info->rule.uploadEnable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "uploadFileType=", sValue, sizeof(sValue))) {
        info->rule.uploadFileType = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "nasEnable=", sValue, sizeof(sValue))) {
        info->rule.nasEnable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "nasFileType=", sValue, sizeof(sValue))) {
        info->rule.nasFileType = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "emailEnable=", sValue, sizeof(sValue))) {
        info->rule.emailEnable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "emailFileType=", sValue, sizeof(sValue))) {
        info->rule.emailFileType = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "httpEnable=", sValue, sizeof(sValue))) {
        info->rule.httpEnable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "httpNotifyInterval=", sValue, sizeof(sValue))) {
        info->rule.httpNotifyInterval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "httpUrl=", sValue, sizeof(sValue))) {
        snprintf(info->rule.httpUrl, sizeof(info->rule.httpUrl), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "httpUser=", sValue, sizeof(sValue))) {
        snprintf(info->rule.httpUser, sizeof(info->rule.httpUser), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "httpPassword=", sValue, sizeof(sValue))) {
        snprintf(info->rule.httpPassword, sizeof(info->rule.httpPassword), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "recordSeconds=", sValue, sizeof(sValue))) {
        info->rule.prerecordSeconds = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "prerecordSeconds=", sValue, sizeof(sValue))) {
        info->rule.prerecordSeconds = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "postEnable=", sValue, sizeof(sValue))) {
        info->postEnable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "postType=", sValue, sizeof(sValue))) {
        info->postType = atoi(sValue);
    }
    
    for (i = 0; i < MAX_FACE_SHIELD; i++) {
        snprintf(tmp, sizeof(tmp), "shield[%d]_enable=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->shield[i].enable = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "shield[%d]_polygonX=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(info->shield[i].region.polygonX, sizeof(info->shield[i].region.polygonX), "%s", sValue);
        }
        snprintf(tmp, sizeof(tmp), "shield[%d]_polygonY=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(info->shield[i].region.polygonY, sizeof(info->shield[i].region.polygonY), "%s", sValue);
        }
    }
    
    for (i = 0; i < FACE_CAPTURE_MAX; i++) {
        snprintf(tmp, sizeof(tmp), "captureQuality[%d]=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->captureQuality[i] = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "snapshot[%d]=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->rule.snapshot[i] = atoi(sValue);
        }
    }
    if (!sdkp2p_get_section_info(buff, "attributeEnable=", sValue, sizeof(sValue))) {
        info->attributeEnable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "attributeType=", sValue, sizeof(sValue))) {
        info->attributeType = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "mosaicEnable=", sValue, sizeof(sValue))) {
        info->mosaicEnable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "mosaicModeStatus=", sValue, sizeof(sValue))) {
        info->mosaicModeStatus = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "mutuallyExclusive=", sValue, sizeof(sValue))) {
        info->mutuallyExclusive = atoi(sValue);
    }

    //action
    update_face_action(buff, chnId);
    return REQUEST_SUCCESS;
}

static void req_p2p_upload_speed(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_upload_speed *res = (struct req_upload_speed *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.time=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->time));
    }
    if (!sdkp2p_get_section_info(buf, "r.size=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->size));
    }
    *datalen = sizeof(struct req_upload_speed);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void upload_speed_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_upload_speed *res = (struct resp_upload_speed *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.time=%lld&", res->uploadTime);
    *datalen = strlen(buf) + 1;
}

int sdk_resp_ipc_alarmstatus_to_json(char *dst, const int dstSize, void *src)
{
    if (!dst || dstSize <= 0 || !src) {
        return -1;
    }

    RESP_IPC_ALARMSTATUS_S *pData = (RESP_IPC_ALARMSTATUS_S *)src;
    cJSON *root = NULL;
    cJSON *arr = NULL;
    cJSON *item = NULL;
    int i;
    char buf[65] = {0};
    char *rootStr = NULL;
    
    root = cJSON_CreateObject();
    if (!root) {
        return -1;
    }

    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    sdkp2p_chnmask_to_str(pData->respStatus, buf, sizeof(buf));
    arr = cJSON_AddArrayToObject(root, "alarmstatuses");
    for (i = 0; i < MAX_REAL_CAMERA; ++i) {
        if (!ms_get_bit(pData->respStatus, i)) {
            continue;
        }
        
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "chnId", i);
        sdkp2p_chnmask_to_str(pData->alarmstatuses[i], buf, sizeof(buf));
        cJSON_AddStringToObject(item, "alarm", buf);
        cJSON_AddItemToArray(arr, item);
    }

    rootStr = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!rootStr) {
        return -1;
    }

    snprintf(dst, dstSize, rootStr);
    cJSON_free(rootStr);

    return 0;
}

SDK_ERROR_CODE sdk_text_to_req_upgrade_camera(REQ_UPGRADE_CAMERA_S *dst, const char *src)
{
    if (!dst || !src) {
        return OTHERS_ERROR;
    }

    char sValue[256] = {0};

    if (sdkp2p_get_section_info(src, "chnMask=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    dst->chnIdMask = get_channel_mask(sValue);
    if (!dst->chnIdMask) {
        return PARAM_ERROR;
    }
    
    if (sdkp2p_get_section_info(src, "url=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    get_url_decode(sValue);
    snprintf(dst->url, sizeof(dst->url), "%s", sValue);
    
    if (!sdkp2p_get_section_info(src, "keepConfMask=", sValue, sizeof(sValue))) {
        dst->keepConfMask = get_channel_mask(sValue);
    } else {
        dst->keepConfMask = -1;
    }

    return REQUEST_SUCCESS;
}

int sdk_resp_upgrade_camera_to_json(char *dst, const int dstSize, void *src)
{
    if (!dst || dstSize <= 0 || !src) {
        return -1;
    }

    RESP_UPGRADE_CAMERA_S *pData = (RESP_UPGRADE_CAMERA_S *)src;
    cJSON *root = NULL;
    cJSON *arr = NULL;
    cJSON *item = NULL;
    int i;
    char buf[65] = {0};
    char *rootStr = NULL;
    
    root = cJSON_CreateObject();
    if (!root) {
        return -1;
    }

    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    sdkp2p_chnmask_to_str(pData->chnIdMask, buf, sizeof(buf));
    arr = cJSON_AddArrayToObject(root, "res");
    for (i = 0; i < MAX_REAL_CAMERA; ++i) {
        if (!ms_get_bit(pData->chnIdMask, i)) {
            continue;
        }
        
        item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "chnId", i);
        cJSON_AddNumberToObject(item, "upgradeStatus", pData->statusArr[i]);
        cJSON_AddItemToArray(arr, item);
    }

    rootStr = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if (!rootStr) {
        return -1;
    }

    snprintf(dst, dstSize, rootStr);
    cJSON_free(rootStr);

    return 0;
}

int sdkp2p_get_ipc_ptz_wiper(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(IPC_PTZ_WIPER_S) || !resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *root = NULL;
    IPC_PTZ_WIPER_S *pInfo = (IPC_PTZ_WIPER_S *)data;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    cJSON_AddNumberToObject(root, "chnId", pInfo->chnId);
    cJSON_AddNumberToObject(root, "autoWiper", pInfo->autoWiper);
    cJSON_AddNumberToObject(root, "wiperSupport", pInfo->wiperSupport);

    sResp = cJSON_Print(root);
    cJSON_Delete(root);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, sResp);
    cJSON_free(sResp);

    return 0;
}

SDK_ERROR_CODE sdkp2p_set_ipc_ptz_wiper(const char *req, IPC_PTZ_WIPER_S *param)
{
    cJSON *root = NULL;
    cJSON *item = NULL;

    root = cJSON_Parse(req);
    if (!root) {
        return PARAM_ERROR;
    }

    item = cJSON_GetObjectItem(root, "chnId");
    if (!item) {
        cJSON_Delete(root);
        return PARAM_ERROR;
    }
    param->chnId = item->valueint;
    if (sdkp2p_check_chnid(param->chnId)) {
        cJSON_Delete(root);
        return PARAM_ERROR;
    }

    item = cJSON_GetObjectItem(root, "autoWiper");
    if (item) {
        param->autoWiper = item->valueint;
    } else {
        param->autoWiper = MS_INVALID_VALUE;
    }

    item = cJSON_GetObjectItem(root, "manualWiper");
    if (item) {
        param->manualWiper = item->valueint;
    } else {
        param->manualWiper = MS_INVALID_VALUE;
    }

    item = cJSON_GetObjectItem(root, "copyChn");
    if (item) {
        snprintf(param->copyChn, sizeof(param->copyChn), item->valuestring);
    } else {
        snprintf(param->copyChn, sizeof(param->copyChn), MS_INVALID_STRING);
    }

    cJSON_Delete(root);

    return REQUEST_SUCCESS;
}

static void req_camera_test_ipc_connect(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_test_ipcconnect *res = (struct req_test_ipcconnect *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.ip=", sValue, sizeof(sValue))) {
        snprintf(res->ip, sizeof(res->ip), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.port=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->port));
    }
    if (!sdkp2p_get_section_info(buf, "r.user=", sValue, sizeof(sValue))) {
        snprintf(res->user, sizeof(res->user), "%s", sValue);
    }

    if (!sdkp2p_get_special_info(buf, "r.password=", sValue, sizeof(sValue), "r.psw_len=")) {
        get_url_decode(sValue);
        snprintf(res->password, sizeof(res->password), "%s", sValue);
    }
    /*
    if(!sdkp2p_get_section_info(buf, "r.password=", sValue, sizeof(sValue))){
        snprintf(res->password, sizeof(res->password), "%s", sValue);
    }
    */
    if (!sdkp2p_get_section_info(buf, "r.protocol=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->protocol));
    }
    if (!sdkp2p_get_section_info(buf, "r.model=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->model));
    }
    *datalen = sizeof(struct req_test_ipcconnect);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);

}

static void resp_camera_test_ipc_connect(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0, chnid = 0;
    REQ_SET_IPCPARAMEX info;

    memset(&info, 0, sizeof(REQ_SET_IPCPARAMEX));
    if (sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        info.chnid[i] = '0';
    }
    info.chnid[chnid] = '1';

    if (!sdkp2p_get_section_info(buf, "r.transProtocol=", sValue, sizeof(sValue))) {
        info.trans_protocol = atoi(sValue);
    } else {
        info.trans_protocol = -1;    //-1. nothing  | 0 /1... todo
    }

    if (!sdkp2p_get_section_info(buf, "r.syncTime=", sValue, sizeof(sValue))) {
        info.sync_time = atoi(sValue);
    } else {
        info.sync_time = -1;    //-1. nothing  | 0 /1... todo
    }

    if (!sdkp2p_get_section_info(buf, "r.audio_enable=", sValue, sizeof(sValue))) {
        info.audio_enable = atoi(sValue);
    } else {
        info.audio_enable = -1;
    }

    info.videoType_enable = 1;  //-1. nothing  | 0 /1... todo

    if (!sdkp2p_get_section_info(buf, "r.substream_enable=", sValue, sizeof(sValue))) {
        info.subEnable = atoi(sValue);
    } else {
        info.subEnable = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.fisheyeType=", sValue, sizeof(sValue))) {
        info.fisheyeType = atoi(sValue);
    } else {
        info.fisheyeType = -1;
    }


    if (!sdkp2p_get_section_info(buf, "r.codec=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].codec = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].codec = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.width=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].width = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].width = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.height=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].height = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].height = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.framerate=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].framerate = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].framerate = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.bitrate=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].bitrate = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].bitrate = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.iframeinterval=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].iframeinterval = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].iframeinterval = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.mainRateControl=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].ratecontrol = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].ratecontrol = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.SmartStream=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].smartstream = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].smartstream = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.SmartStreamLevel=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].smartstreamlevel = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].smartstreamlevel = -1;
    }

    if (!sdkp2p_get_section_info(buf, "recordtype=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].recordtype = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].recordtype = -1;
    }

    if (!sdkp2p_get_section_info(buf, "etframerate=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].etframerate = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].etframerate = -1;
    }

    if (!sdkp2p_get_section_info(buf, "etIframeinterval=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].etIframeinterval = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].etIframeinterval = -1;
    }

    if (!sdkp2p_get_section_info(buf, "etbitrate=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_MAINSTREAM].etbitrate = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_MAINSTREAM].etbitrate = -1;
    }


    if (!sdkp2p_get_section_info(buf, "r.sub_codec=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].codec = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].codec = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.sub_width=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].width = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].width = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.sub_height=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].height = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].height = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.sub_framerate=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].framerate = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].framerate = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.sub_bitrate=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].bitrate = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].bitrate = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.sub_iframeinterval=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].iframeinterval = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].iframeinterval = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.subRateControl=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].ratecontrol = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].ratecontrol = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.SubSmartStream=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].smartstream = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].smartstream = -1;
    }

    if (!sdkp2p_get_section_info(buf, "r.SubSmartStreamLevel=", sValue, sizeof(sValue))) {
        info.stream[STREAM_TYPE_SUBSTREAM].smartstreamlevel = atoi(sValue);
    } else {
        info.stream[STREAM_TYPE_SUBSTREAM].smartstreamlevel = -1;
    }

    info.stream[STREAM_TYPE_SUBSTREAM].recordtype = -1;
    info.stream[STREAM_TYPE_SUBSTREAM].etframerate = -1;
    info.stream[STREAM_TYPE_SUBSTREAM].etIframeinterval = -1;
    info.stream[STREAM_TYPE_SUBSTREAM].etbitrate = -1;

    sdkp2p_common_write_log(conf, MAIN_OP, SUP_OP_EDIT_IP_CHANNEL_REMOTE, SUB_PARAM_EDIT_IPC, chnid + 1, 0, NULL, 0);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(REQ_SET_IPCPARAMEX), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipc_param(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    Uint64 res = *(Uint64 *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%lld&", res);

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipcaddr(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};

    struct req_set_ipcaddr edit;
    memset(&edit, 0, sizeof(struct req_set_ipcaddr));

    if (sdkp2p_get_section_info(buf, "mac=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.mac, sizeof(edit.mac), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "old_ip=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.oldipaddr, sizeof(edit.oldipaddr), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "new_ip=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.newipaddr, sizeof(edit.newipaddr), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "netmask=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.netmask, sizeof(edit.netmask), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "gateway=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.gateway, sizeof(edit.gateway), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "primary_dns=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.primarydns, sizeof(edit.primarydns), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.username, sizeof(edit.username), "%s", sValue);

    if (sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    get_url_decode(sValue);
    snprintf(edit.password, sizeof(edit.password), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "protocol=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    edit.protocol_id = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    edit.port = atoi(sValue);


    if (sdkp2p_get_section_info(buf, "netif=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(edit.netif_form, sizeof(edit.netif_form), "%s", sValue);

    sdkp2p_send_msg(conf, conf->req, &edit, sizeof(struct req_set_ipcaddr), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipcaddr(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    if (res != 0) {
        res = -1;
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_camera_active_ipc(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char tmp[128] = {0};
    char sValue[256] = {0};

    struct active_camera_info active;
    memset(&active, 0, sizeof(struct active_camera_info));

    active.type = ACTIVATE;
    if (sdkp2p_get_section_info(buf, "camera_list=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    active.camera_list = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "password=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(active.password, sizeof(active.password), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "key_id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(active.key, sizeof(active.key), "%s", sValue);

    for (i = 0; i < active.camera_list; i++) {
        snprintf(tmp, sizeof(tmp), "mac%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
            return ;
        }
        snprintf(active.mac[i], sizeof(active.mac[i]), "%s", sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &active, sizeof(struct active_camera_info), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_active_ipc(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    if (res != 0) {
        res = -1;
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_camera_test_ip_conflict(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char ipaddr[32] = {0};

    if (sdkp2p_get_section_info(buf, "ip=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(ipaddr, sizeof(ipaddr), "%s", sValue);
    sdkp2p_send_msg(conf, conf->req, (void *)&ipaddr, sizeof(ipaddr), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_test_ip_conflict(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void get_recparam_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.ch=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }
    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void set_recparam_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_rec_param *res = (struct req_rec_param *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.ch=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->ch));
    }
    if (!sdkp2p_get_section_info(buf, "r.mode=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->mode));
    }
    if (!sdkp2p_get_section_info(buf, "r.audio_on=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->audio_on));
    }
    if (!sdkp2p_get_section_info(buf, "r.prev_rec_on=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->prev_rec_on));
    }

    if (!sdkp2p_get_section_info(buf, "r.prev_rec_duration=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->prev_rec_duration));
    }
    if (!sdkp2p_get_section_info(buf, "r.post_rec_on=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->post_rec_on));
    }
    if (!sdkp2p_get_section_info(buf, "r.post_rec_duration=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->post_rec_duration));
    }
    /*if(!sdkp2p_get_section_info(buf, "r.rec_expiration_date=", sValue, sizeof(sValue))){
        sscanf(sValue,"%d",&(res->rec_expiration_date));
    }*/
    *datalen = sizeof(struct req_rec_param);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void req_camera_set_fisheye_mode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[64] = {0};
    struct resp_fishmode_param fisheye = {0};

    memset(&fisheye, 0x0, sizeof(struct resp_fishmode_param));
    fisheye.fishcorrect = -1;
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    fisheye.chnid = chnid;
    if (!sdkp2p_get_section_info(buf, "fishmount=", sValue, sizeof(sValue))) {
        fisheye.fishmount = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "fishdisplay=", sValue, sizeof(sValue))) {
        fisheye.fishdisplay = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "fishcorrect=", sValue, sizeof(sValue))) {
        fisheye.fishcorrect = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&fisheye, sizeof(struct resp_fishmode_param), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_camera_get_fisheye_mode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_fisheye_mode(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_fishmode_param *res = (struct resp_fishmode_param *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", res->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.fishmount=%d&", res->fishmount);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.fishdisplay=%d&", res->fishdisplay);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.fishcorrect=%d&", res->fishcorrect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.result=%d&", res->result);

    *datalen = strlen(buf) + 1;
}

static void req_camera_get_image_enhancement(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;

    if (sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_image_enhancement(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_image_enhancement *res = (struct resp_image_enhancement *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.id=%d&", res->id);
    snprintf(buf + strlen(buf), len - strlen(buf), "type=%d&", res->type);
    if (res->type == IPC_IMAGE_TYPE_SINGLE) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.smartir=%u&", res->image.smartir);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteblance=%u&", res->image.whiteblance);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.focusmode=%u&", res->image.focusmode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.antiblurry=%u&", res->image.antiblurry);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.wdrenable=%u&", res->image.wdrenable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.wdrlevel=%u&", res->image.wdrlevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.wdrstarthour=%u&", res->image.wdrstarthour);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.wdrstartminute=%u&", res->image.wdrstartminute);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.wdrstophour=%u&", res->image.wdrstophour);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.wdrstopminute=%u&", res->image.wdrstopminute);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.regiontype=%u&", res->image.regiontype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.hlcmode=%u&", res->image.hlcmode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.exclude=%u&", res->image.exclude);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.exposemode=%u&", res->image.exposemode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.exposeregion=%s&", res->image.exposeregion);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.modelType=%d&", res->image.modelType);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.imageschemode=%u&", res->image.imageschemode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.hlcday=%u&", res->image.hlcday);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.hlcnight=%u&", res->image.hlcnight);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.redgain=%u&", res->image.redgain);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.bluegain=%u&", res->image.bluegain);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.defogmode=%u&", res->image.defogmode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.defoglevel=%u&", res->image.defoglevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.dismode=%u&", res->image.dismode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.exposetime=%u&", res->image.exposetime);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.exposegain=%u&", res->image.exposegain);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.wdraflevel=%u&", res->image.wdraflevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.exposurectrl=%u&", res->image.exposurectrl);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.hlclevel=%u&", res->image.hlclevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.enblur=%u&", res->image.enblur);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.deblur=%u&", res->image.deblur);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.sdkversion=%s&", res->image.sdkversion);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.lensType=%u&", res->image.lensType);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.reduceStuttering=%d&", res->image.reduce_stuttering);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.fullcolorSupport=%d&", res->image.fullcolorSupport);
    } else if (res->type == IPC_IMAGE_TYPE_MULTI) {
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.irBalanceMode=%d&", res->imgMulti.scenes[0].irBalanceMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.reduceMotionBlur=%d&", res->imgMulti.scenes[0].reduceMotionBlur);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.deblur=%d&", res->imgMulti.scenes[0].deblur);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.defogMode=%d&", res->imgMulti.scenes[0].defogMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.antiFogIntensity=%d&", res->imgMulti.scenes[0].antiFogIntensity);

        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.whitebalanceMode=%d&", res->imgMulti.scenes[0].whitebalanceMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.whitebalanceRedGain=%d&", res->imgMulti.scenes[0].whitebalanceRedGain);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.whitebalanceBlueGain=%d&", res->imgMulti.scenes[0].whitebalanceBlueGain);

        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.exposureMode=%d&", res->imgMulti.scenes[0].exposureMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.exposureTime=%d&", res->imgMulti.scenes[0].exposureTime);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.exposureGain=%d&", res->imgMulti.scenes[0].exposureGain);

        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.blcRegion=%d&", res->imgMulti.scenes[0].blcRegion);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.blcRegionRange=%d&", res->imgMulti.scenes[0].blcRegionRange);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.blcRegionArea=%s&", res->imgMulti.scenes[0].blcRegionArea);

        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.wdrMode=%d&", res->imgMulti.scenes[0].wdrMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.wdrStartHour=%d&", res->imgMulti.scenes[0].wdrStartHour);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.wdrStartMin=%d&", res->imgMulti.scenes[0].wdrStartMin);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.wdrEndHour=%d&", res->imgMulti.scenes[0].wdrEndHour);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.wdrEndMin=%d&", res->imgMulti.scenes[0].wdrEndMin);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.wdrLevel=%d&", res->imgMulti.scenes[0].wdrLevel);

        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.hlcMode=%d&", res->imgMulti.scenes[0].hlcMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scence0.hlcLevel=%d&", res->imgMulti.scenes[0].hlcLevel);
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_image_enhancement(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[64] = {0};
    struct req_image_enhancement image = {0};

    if (sdkp2p_get_section_info(buf, "r.id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    image.chnid = chnid;

    sdk_get_field_int(buf, "type=", (int *)&image.type);
    if (image.type == MS_INVALID_VALUE) {
        image.type = IPC_IMAGE_TYPE_SINGLE;
    } else if (image.type < IPC_IMAGE_TYPE_SINGLE || image.type >= IPC_IMAGE_TYPE_MAX) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (image.type == IPC_IMAGE_TYPE_SINGLE) {
        if (!sdkp2p_get_section_info(buf, "r.image.smartir=", sValue, sizeof(sValue))) {
            image.image.smartir = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.whiteblance=", sValue, sizeof(sValue))) {
            image.image.whiteblance = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.focusmode=", sValue, sizeof(sValue))) {
            image.image.focusmode = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.antiblurry=", sValue, sizeof(sValue))) {
            image.image.antiblurry = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.wdrenable=", sValue, sizeof(sValue))) {
            image.image.wdrenable = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.wdrlevel=", sValue, sizeof(sValue))) {
            image.image.wdrlevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.wdrstarthour=", sValue, sizeof(sValue))) {
            image.image.wdrstarthour = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.wdrstartminute=", sValue, sizeof(sValue))) {
            image.image.wdrstartminute = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.wdrstophour=", sValue, sizeof(sValue))) {
            image.image.wdrstophour = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.wdrstopminute=", sValue, sizeof(sValue))) {
            image.image.wdrstopminute = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.regiontype=", sValue, sizeof(sValue))) {
            image.image.regiontype = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.hlcmode=", sValue, sizeof(sValue))) {
            image.image.hlcmode = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.exclude=", sValue, sizeof(sValue))) {
            image.image.exclude = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.exposemode=", sValue, sizeof(sValue))) {
            image.image.exposemode = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.modelType=", sValue, sizeof(sValue))) {
            image.image.modelType = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.exposeregion=", sValue, sizeof(sValue))) {
            snprintf(image.image.exposeregion, sizeof(image.image.exposeregion), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.imageschemode=", sValue, sizeof(sValue))) {
            image.image.imageschemode = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.hlcday=", sValue, sizeof(sValue))) {
            image.image.hlcday = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.hlcnight=", sValue, sizeof(sValue))) {
            image.image.hlcnight = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.redgain=", sValue, sizeof(sValue))) {
            image.image.redgain = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.bluegain=", sValue, sizeof(sValue))) {
            image.image.bluegain = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.exposetime=", sValue, sizeof(sValue))) {
            image.image.exposetime = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.exposegain=", sValue, sizeof(sValue))) {
            image.image.exposegain = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.wdraflevel=", sValue, sizeof(sValue))) {
            image.image.wdraflevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.hlclevel=", sValue, sizeof(sValue))) {
            image.image.hlclevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.defogmode=", sValue, sizeof(sValue))) {
            image.image.defogmode = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.defoglevel=", sValue, sizeof(sValue))) {
            image.image.defoglevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.enblur=", sValue, sizeof(sValue))) {
            image.image.enblur = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.deblur=", sValue, sizeof(sValue))) {
            image.image.deblur = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.lensType=", sValue, sizeof(sValue))) {
            image.image.lensType = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.reduceStuttering=", sValue, sizeof(sValue))) {
            image.image.reduce_stuttering = atoi(sValue);
        }
    } else if (IPC_IMAGE_TYPE_MULTI) {
        sdk_get_field_int(buf, "scence0.irBalanceMode=", (int *)&image.imgMulti.scenes[0].irBalanceMode);
        sdk_get_field_int(buf, "scence0.reduceMotionBlur=", (int *)&image.imgMulti.scenes[0].reduceMotionBlur);
        sdk_get_field_int(buf, "scence0.deblur=", (int *)&image.imgMulti.scenes[0].deblur);
        sdk_get_field_int(buf, "scence0.defogMode=", (int *)&image.imgMulti.scenes[0].defogMode);
        sdk_get_field_int(buf, "scence0.antiFogIntensity=", (int *)&image.imgMulti.scenes[0].antiFogIntensity);

        sdk_get_field_int(buf, "scence0.whitebalanceMode=", (int *)&image.imgMulti.scenes[0].whitebalanceMode);
        sdk_get_field_int(buf, "scence0.whitebalanceRedGain=", (int *)&image.imgMulti.scenes[0].whitebalanceRedGain);
        sdk_get_field_int(buf, "scence0.whitebalanceBlueGain=", (int *)&image.imgMulti.scenes[0].whitebalanceBlueGain);

        sdk_get_field_int(buf, "scence0.exposureMode=", (int *)&image.imgMulti.scenes[0].exposureMode);
        sdk_get_field_int(buf, "scence0.exposureTime=", (int *)&image.imgMulti.scenes[0].exposureTime);
        sdk_get_field_int(buf, "scence0.exposureGain=", (int *)&image.imgMulti.scenes[0].exposureGain);

        sdk_get_field_int(buf, "scence0.blcRegion=", (int *)&image.imgMulti.scenes[0].blcRegion);
        sdk_get_field_int(buf, "scence0.blcRegionRange=", (int *)&image.imgMulti.scenes[0].blcRegionRange);
        sdk_get_field_string(buf, "scence0.blcRegionArea=", image.imgMulti.scenes[0].blcRegionArea, 
            sizeof(image.imgMulti.scenes[0].blcRegionArea));

        sdk_get_field_int(buf, "scence0.wdrMode=", (int *)&image.imgMulti.scenes[0].wdrMode);
        sdk_get_field_int(buf, "scence0.wdrStartHour=", (int *)&image.imgMulti.scenes[0].wdrStartHour);
        sdk_get_field_int(buf, "scence0.wdrStartMin=", (int *)&image.imgMulti.scenes[0].wdrStartMin);
        sdk_get_field_int(buf, "scence0.wdrEndHour=", (int *)&image.imgMulti.scenes[0].wdrEndHour);
        sdk_get_field_int(buf, "scence0.wdrEndMin=", (int *)&image.imgMulti.scenes[0].wdrEndMin);
        sdk_get_field_int(buf, "scence0.wdrLevel=", (int *)&image.imgMulti.scenes[0].wdrLevel);

        sdk_get_field_int(buf, "scence0.hlcMode=", (int *)&image.imgMulti.scenes[0].hlcMode);
        sdk_get_field_int(buf, "scence0.hlcLevel=", (int *)&image.imgMulti.scenes[0].hlcLevel);
    }
    
    sdkp2p_send_msg(conf, conf->req, (void *)&image, sizeof(struct req_image_enhancement), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_camera_get_image_osd(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;

    if (sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_image_osd(void *param, int size, char *buf, int len, int *datalen)
{
    struct req_ovf_osd *res = (struct req_ovf_osd *)param;
    char encode[1024] = {0};

    snprintf(buf + strlen(buf), len - strlen(buf), "r.ch=%d&", res->ch);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable_text=%d&", res->enable_text);
    get_url_encode(res->text_name, strlen(res->text_name), encode, sizeof(encode));
    snprintf(buf + strlen(buf), len - strlen(buf), "r.text_name=%s&", encode);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.textpos=%d&", res->textpos);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable_date=%d&", res->enable_date);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.datepos=%d&", res->datepos);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.dateformat=%d&", res->dateformat);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.fontSize=%d&", res->fontSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable_text_sub=%d&", res->enable_text_sub);
    get_url_encode(res->text_name_sub, strlen(res->text_name_sub), encode, sizeof(encode));
    snprintf(buf + strlen(buf), len - strlen(buf), "r.text_name_sub=%s&", encode);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.textpos_sub=%d&", res->textpos_sub);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable_date_sub=%d&", res->enable_date_sub);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.datepos_sub=%d&", res->datepos_sub);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.dateformat_sub=%d&", res->dateformat_sub);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.fontSizeSub=%d&", res->fontSizeSub);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable_text_thd=%d&", res->enable_text_thd);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.text_name_thd=%s&", res->text_name_thd);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.textpos_thd=%d&", res->textpos_thd);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable_date_thd=%d&", res->enable_date_thd);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.datepos_thd=%d&", res->datepos_thd);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.dateformat_thd=%d&", res->dateformat_thd);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.fontSizeThd=%d&", res->fontSizeThd);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.stream_type=%d&", res->stream_type);

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_image_osd(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_ovf_osd osd;
    char sValue[1024] = {0};

    memset(&osd, 0x0, sizeof(struct req_ovf_osd));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.ch));
    }
    if (!sdkp2p_get_section_info(buf, "r.enable_text=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.enable_text));
    }
    if (!sdkp2p_get_section_info(buf, "r.text_name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(osd.text_name, sizeof(osd.text_name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.textpos=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.textpos));
    }
    if (!sdkp2p_get_section_info(buf, "r.enable_date=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.enable_date));
    }
    if (!sdkp2p_get_section_info(buf, "r.datepos=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.datepos));
    }
    if (!sdkp2p_get_section_info(buf, "r.dateformat=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.dateformat));
    }
    if (!sdkp2p_get_section_info(buf, "r.fontSize=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", (int *)&(osd.fontSize));
    } else {
        osd.fontSize = MS_INVALID_VALUE;
    }
    if (!sdkp2p_get_section_info(buf, "r.enable_text_sub=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.enable_text_sub));
    }
    if (!sdkp2p_get_section_info(buf, "r.text_name_sub=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(osd.text_name_sub, sizeof(osd.text_name_sub), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.textpos_sub=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.textpos_sub));
    }
    if (!sdkp2p_get_section_info(buf, "r.enable_date_sub=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.enable_date_sub));
    }
    if (!sdkp2p_get_section_info(buf, "r.datepos_sub=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.datepos_sub));
    }
    if (!sdkp2p_get_section_info(buf, "r.dateformat_sub=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.dateformat_sub));
    }
    if (!sdkp2p_get_section_info(buf, "r.fontSizeSub=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", (int *)&(osd.fontSizeSub));
    } else {
        osd.fontSizeSub = MS_INVALID_VALUE;
    }
    if (!sdkp2p_get_section_info(buf, "r.enable_text_thd=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.enable_text_thd));
    }
    if (!sdkp2p_get_section_info(buf, "r.text_name_thd=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(osd.text_name_thd, sizeof(osd.text_name_thd), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.textpos_thd=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.textpos_thd));
    }
    if (!sdkp2p_get_section_info(buf, "r.enable_date_thd=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.enable_date_thd));
    }
    if (!sdkp2p_get_section_info(buf, "r.datepos_thd=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.datepos_thd));
    }
    if (!sdkp2p_get_section_info(buf, "r.dateformat_thd=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(osd.dateformat_thd));
    }
    if (!sdkp2p_get_section_info(buf, "r.fontSizeThd=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", (int *)&(osd.fontSizeThd));
    } else {
        osd.fontSizeThd = MS_INVALID_VALUE;
    }
    if (!sdkp2p_get_section_info(buf, "r.stream_type=", sValue, sizeof(sValue))) {
        osd.stream_type = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&osd, sizeof(struct req_ovf_osd), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_set_image_osd(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
    return ;
}

static void req_camera_get_image_display(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;

    if (sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_image_display(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_image_display *res = (struct resp_image_display *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.id=%d&", res->id);
    snprintf(buf + strlen(buf), len - strlen(buf), "type=%d&", res->type);
    if (res->type == IPC_IMAGE_TYPE_SINGLE) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.colorkiller=%d&", res->image.colorkiller);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.modestarthour=%d&", res->image.modestarthour);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.modestartminute=%d&", res->image.modestartminute);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.modestophour=%d&", res->image.modestophour);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.modestopminute=%d&", res->image.modestopminute);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.exposurectrl=%d&", res->image.exposurectrl);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.lighttype=%d&", res->image.lighttype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.mirctrl=%d&", res->image.mirctrl);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.localdisplay=%d&", res->image.localdisplay);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.ircutaddaytonight=%d&", res->image.ircutaddaytonight);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.ircutadnighttoday=%d&", res->image.ircutadnighttoday);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.corridormode=%d&", res->image.corridormode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.imagerotation=%d&", res->image.imagerotation);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.saveCorridor=%d&", res->image.saveCorridor);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.modelType=%d&", res->image.modelType);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.lencorrect=%d&", res->image.lencorrect);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.smartIRType=%d&", res->image.smartIRType);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.nearIRLevel=%d&", res->image.nearIRLevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.farIRLevel=%d&", res->image.farIRLevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.midIRLevel=%d&", res->image.midIRLevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.smartIrMath=%d&", res->image.smartIrMath);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.smartIrLimit=%d&", res->image.smartIrLimit);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.chipninterface=%s&", res->image.chipninterface);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.nearSmartirValue=%d&", res->image.nearSmartirValue);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.farSmartirValue=%d&", res->image.farSmartirValue);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.midSmartirValue=%d&", res->image.midSmartirValue);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.lenCorrectType=%d&", res->image.lenCorrectType);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.fovadjust=%d&", res->image.fovadjust);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.sensortype=%s&", res->image.sensortype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.productmodel=%s&", res->image.productmodel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.softversion=%s&", res->image.softversion);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.smokeddomecover=%d&", res->image.smokeddomecover);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.smartwhitestrength=%d&", res->image.smartwhitestrength);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.dnRefocus=%d&", res->image.dnRefocus);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.structuretype=%d&", res->image.structuretype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.aspectRatio=%d&", res->image.aspect_ratio);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.d2nSensitivity=%d&", res->image.d2n_sensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.n2dSensitivity=%d&", res->image.n2d_sensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.zoomLimit=%d&", res->image.zoom_limit);
        
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledSupport=%d&", res->image.whiteled.support);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledLightCtrl=%d&", res->image.whiteled.lightCtrl);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledSensitivity=%d&", res->image.whiteled.sensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledDelay=%d&", res->image.whiteled.delay);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledStart=%d&", res->image.whiteled.start);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledStop=%d&", res->image.whiteled.stop);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledBrightnessCtrl=%d&", res->image.whiteled.brightnessCtrl);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.whiteledBrightness=%d&", res->image.whiteled.brightness);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.system_image_slider_type=%d&", res->image.system_image_slider_type);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.image.isNtPlatform=%d&", res->image.isNtPlatform);
    } else if (res->type == IPC_IMAGE_TYPE_MULTI) {
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.powerlineFreq=%d&", res->imgMulti.scenes[0].powerlineFreq);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.dnMode=%d&", res->imgMulti.scenes[0].dnMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.d2nSensitivity=%d&", res->imgMulti.scenes[0].d2nSensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.n2dSensitivity=%d&", res->imgMulti.scenes[0].n2dSensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.nightStartHour=%d&", res->imgMulti.scenes[0].nightStartHour);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.nightStartMinute=%d&", res->imgMulti.scenes[0].nightStartMinute);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.nightEndHour=%d&", res->imgMulti.scenes[0].nightEndHour);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.nightEndMinute=%d&", res->imgMulti.scenes[0].nightEndMinute);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.smartIrMode=%d&", res->imgMulti.scenes[0].smartIrMode);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.irLedLevel=%d&", res->imgMulti.scenes[0].irLedLevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.imageRotation=%d&", res->imgMulti.scenes[0].imageRotation);
        // snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.lighttype=%d&", res->imgMulti.scenes[0].doorMode);
    }

    *datalen = strlen(buf) + 1;
    return ;
}

static void req_camera_set_image_display(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[64] = {0};
    struct req_image_display image = {0};

    memset(&image, 0x0, sizeof(struct req_image_display));
    if (sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    image.chnid = chnid;

    sdk_get_field_int(buf, "type=", (int *)&image.type);
    if (image.type == MS_INVALID_VALUE) {
        image.type = IPC_IMAGE_TYPE_SINGLE;
    } else if (image.type < IPC_IMAGE_TYPE_SINGLE || image.type >= IPC_IMAGE_TYPE_MAX) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (image.type == IPC_IMAGE_TYPE_SINGLE) {
        if (!sdkp2p_get_section_info(buf, "r.image.colorkiller=", sValue, sizeof(sValue))) {
            image.image.colorkiller = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.modestarthour=", sValue, sizeof(sValue))) {
            image.image.modestarthour = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.modestartminute=", sValue, sizeof(sValue))) {
            image.image.modestartminute = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.modestophour=", sValue, sizeof(sValue))) {
            image.image.modestophour = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.modestopminute=", sValue, sizeof(sValue))) {
            image.image.modestopminute = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.exposurectrl=", sValue, sizeof(sValue))) {
            image.image.exposurectrl = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.lighttype=", sValue, sizeof(sValue))) {
            image.image.lighttype = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.mirctrl=", sValue, sizeof(sValue))) {
            image.image.mirctrl = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.localdisplay=", sValue, sizeof(sValue))) {
            image.image.localdisplay = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.ircutaddaytonight=", sValue, sizeof(sValue))) {
            image.image.ircutaddaytonight = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.ircutadnighttoday=", sValue, sizeof(sValue))) {
            image.image.ircutadnighttoday = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.corridormode=", sValue, sizeof(sValue))) {
            image.image.corridormode = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.imagerotation=", sValue, sizeof(sValue))) {
            image.image.imagerotation = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.saveCorridor=", sValue, sizeof(sValue))) {
            image.image.saveCorridor = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.modelType=", sValue, sizeof(sValue))) {
            image.image.modelType = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.image.lencorrect=", sValue, sizeof(sValue))) {
            image.image.lencorrect = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "smartIRType=", sValue, sizeof(sValue))) {
            image.image.smartIRType = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "nearIRLevel=", sValue, sizeof(sValue))) {
            image.image.nearIRLevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "farIRLevel=", sValue, sizeof(sValue))) {
            image.image.farIRLevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "midIRLevel=", sValue, sizeof(sValue))) {
            image.image.midIRLevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "smartwhitestrength=", sValue, sizeof(sValue))) {
            image.image.smartwhitestrength = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "fovadjust=", sValue, sizeof(sValue))) {
            image.image.fovadjust = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "smokeddomecover=", sValue, sizeof(sValue))) {
            image.image.smokeddomecover = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "dnRefocus=", sValue, sizeof(sValue))) {
            image.image.dnRefocus = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "aspectRatio=", sValue, sizeof(sValue))) {
            image.image.aspect_ratio = atoi(sValue);
        }

        if (!sdkp2p_get_section_info(buf, "d2nSensitivity=", sValue, sizeof(sValue))) {
            image.image.d2n_sensitivity = atoi(sValue);
        }

        if (!sdkp2p_get_section_info(buf, "n2dSensitivity=", sValue, sizeof(sValue))) {
            image.image.n2d_sensitivity = atoi(sValue);
        }

        if (!sdkp2p_get_section_info(buf, "zoomLimit=", sValue, sizeof(sValue))) {
            image.image.zoom_limit = atoi(sValue);
        }

        sdk_get_field_int(buf, "whiteledLightCtrl=", (int *)&image.image.whiteled.lightCtrl);
        sdk_get_field_int(buf, "whiteledSensitivity=", &image.image.whiteled.sensitivity);
        sdk_get_field_int(buf, "whiteledDelay=", &image.image.whiteled.delay);
        sdk_get_field_int(buf, "whiteledBrightnessCtrl=", &image.image.whiteled.brightnessCtrl);
        sdk_get_field_int(buf, "whiteledBrightness=", &image.image.whiteled.brightness);
    } else if (image.type == IPC_IMAGE_TYPE_MULTI) {
        sdk_get_field_int(buf, "scenes0.powerlineFreq=", (int *)&image.imgMulti.scenes[0].powerlineFreq);
        sdk_get_field_int(buf, "scenes0.dnMode=", (int *)&image.imgMulti.scenes[0].dnMode);
        sdk_get_field_int(buf, "scenes0.d2nSensitivity=", (int *)&image.imgMulti.scenes[0].d2nSensitivity);
        sdk_get_field_int(buf, "scenes0.n2dSensitivity=", (int *)&image.imgMulti.scenes[0].n2dSensitivity);
        sdk_get_field_int(buf, "scenes0.nightStartHour=", (int *)&image.imgMulti.scenes[0].nightStartHour);
        sdk_get_field_int(buf, "scenes0.nightStartMinute=", (int *)&image.imgMulti.scenes[0].nightStartMinute);
        sdk_get_field_int(buf, "scenes0.nightEndHour=", (int *)&image.imgMulti.scenes[0].nightEndHour);
        sdk_get_field_int(buf, "scenes0.nightEndMinute=", (int *)&image.imgMulti.scenes[0].nightEndMinute);
        sdk_get_field_int(buf, "scenes0.smartIrMode=", (int *)&image.imgMulti.scenes[0].smartIrMode);
        sdk_get_field_int(buf, "scenes0.irLedLevel=", (int *)&image.imgMulti.scenes[0].irLedLevel);
        sdk_get_field_int(buf, "scenes0.imageRotation=", (int *)&image.imgMulti.scenes[0].imageRotation);
        // sdk_get_field_int(buf, "scenes0.doorMode=", (int *)&image.imgMulti.scenes[0].doorMode);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&image, sizeof(struct req_image_display), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_camera_get_image_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_image_param(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_imageparam;

    struct resp_get_imageparam *res = (struct resp_get_imageparam *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chanid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.brightness=%d&", res->brightness);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.contrast=%d&", res->contrast);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.saturation=%d&", res->saturation);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sharpness=%d&", res->sharpness);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.nflevel=%d&", res->nflevel);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.nf2level=%d&", res->nf2level);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.result=%d&", res->result);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.supportLpr=%d&", res->supportLpr);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.isNtPlatform=%d&", res->isNtPlatform);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.isPanoramaStitch=%d&", res->isPanoramaStitch);
    
    *datalen = strlen(buf) + 1;
}

static int on_get_iamge_param_value(int value)
{
    float tmp = 0.0;
    int ret = 0;

    tmp = 1.0 * value * 100 / 255;
    ret = (int) tmp;

    if ((tmp - 0.5) >= ret) {
        ret += 1;
    }

    return ret;
}

static void req_camera_set_image_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_set_imageparam *req = (struct req_set_imageparam *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->chanid));
    }
    if (!sdkp2p_get_section_info(buf, "r.brightness=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->brightness));
    }
    if (!sdkp2p_get_section_info(buf, "r.contrast=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->contrast));
    }
    if (!sdkp2p_get_section_info(buf, "r.saturation=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->saturation));
    }
    if (!sdkp2p_get_section_info(buf, "r.sharpness=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->sharpness));
    }
    if (!sdkp2p_get_section_info(buf, "r.nflevel=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->nflevel));
    }
    if (!sdkp2p_get_section_info(buf, "r.nf2level=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->nf2level));
    }

    *datalen = sizeof(struct req_set_imageparam);
    sdkp2p_send_msg(conf, conf->req, req, *datalen, SDKP2P_NOT_CALLBACK);

    struct op_lr_setvideo_para olsp;
    memset(&olsp, 0, sizeof(struct op_lr_setvideo_para));
    olsp.brightness = on_get_iamge_param_value(req->brightness);
    olsp.contrast = on_get_iamge_param_value(req->contrast);
    olsp.saturation = on_get_iamge_param_value(req->saturation);
    olsp.sharpness = on_get_iamge_param_value(req->sharpness);
    olsp.nflevel = on_get_iamge_param_value(req->nflevel);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_SET_VIDEO_PARAM, SUB_PARAM_NONE, req->chanid + 1, OP_SET_VIDEO_PARA,
                            &olsp, sizeof(struct op_lr_setvideo_para));
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_camera_get_common_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_http_common_param *req = (struct req_http_common_param *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->chnid));
    }
    if (!sdkp2p_get_section_info(buf, "r.info.pagename=", sValue, sizeof(sValue))) {
        snprintf(req->info.pagename, sizeof(req->info.pagename), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.httpname=", sValue, sizeof(sValue))) {
        snprintf(req->info.httpname, sizeof(req->info.httpname), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.key=", sValue, sizeof(sValue))) {
        snprintf(req->info.key, sizeof(req->info.key), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.value=", sValue, sizeof(sValue))) {
        snprintf(req->info.value, sizeof(req->info.value), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.noparatest=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->info.noparatest));
    }

    /*
    【Alice-丹麦-Secpro：V2.4.0.12-r3 通过P2P 添加NVR，之后进入IPC 图像配置界面，IR Light Sensor Value 值始终为0】https://www.tapd.cn/21417321/bugtrace/bugs/view?bug_id=1121417321001058175
    */
    if (strlen(req->info.pagename) == 0) {
        req->info.noparatest = 1;
    }

    *datalen = sizeof(struct req_http_common_param);
    sdkp2p_send_msg(conf, conf->req, req, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_common_param(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_http_common_param *res = (struct resp_http_common_param *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", res->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res->res);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.info.pagename=%s&", res->info.pagename);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.info.httpname=%s&", res->info.httpname);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.info.key=%s&", res->info.key);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.info.value=%s&", res->info.value);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.info.noparatest=%d&", res->info.noparatest);

    *datalen = strlen(buf) + 1;
    return ;
}

static void req_camera_set_common_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_http_common_param *req = (struct req_http_common_param *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->chnid));
    }
    if (!sdkp2p_get_section_info(buf, "r.info.pagename=", sValue, sizeof(sValue))) {
        snprintf(req->info.pagename, sizeof(req->info.pagename), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.httpname=", sValue, sizeof(sValue))) {
        snprintf(req->info.httpname, sizeof(req->info.httpname), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.key=", sValue, sizeof(sValue))) {
        snprintf(req->info.key, sizeof(req->info.key), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.value=", sValue, sizeof(sValue))) {
        snprintf(req->info.value, sizeof(req->info.value), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.info.noparatest=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(req->info.noparatest));
    }

    *datalen = sizeof(struct req_http_common_param);
    sdkp2p_send_msg(conf, conf->req, req, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_set_common_param(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
    return ;
}

static void req_camera_get_vca_region_entrance(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.format=", sValue, sizeof(sValue))) {
        res->format = atoi(sValue);
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_vca_region_entrance(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_event_info *resp = (struct ms_smart_event_info *)param;

    if (!resp->format) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.region=%s&", resp->area);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.sensitivity=%d&", resp->sensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.objtype=%d&", resp->objtype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonX=%s&", resp->polygonX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonY=%s&", resp->polygonY);
    } else {
        get_vca_common_json(param, size, buf, len, REGIONIN);
    }
    
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_vca_region_entrance(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[512] = {0};
    int ret = -1;
    memset(res, 0x0, sizeof(struct ms_smart_event_info));
    
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.sensitivity=", sValue, sizeof(sValue))) {
        res->sensitivity = atoi(sValue);
    }

    ret = set_vca_common_region_by_text(buf, res);
    if (ret) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (res->regionType == REGION_SINGLE) {
        if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
            res->enable = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.area=", sValue, sizeof(sValue))) {
            snprintf(res->area, sizeof(res->area), "%s", sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.objtype=", sValue, sizeof(sValue))) {
            res->objtype = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonX=", sValue, sizeof(sValue))) {
            snprintf(res->polygonX, sizeof(res->polygonX), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonY=", sValue, sizeof(sValue))) {
            snprintf(res->polygonY, sizeof(res->polygonY), "%s", sValue);
        }
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_set_vca_region_entrance(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_region_exit(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_event_info *resp = (struct ms_smart_event_info *)param;

    if (!resp->format) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.region=%s&", resp->area);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.sensitivity=%d&", resp->sensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.objtype=%d&", resp->objtype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonX=%s&", resp->polygonX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonY=%s&", resp->polygonY);
    } else {
        get_vca_common_json(param, size, buf, len, REGIONOUT);
    }
    
    *datalen = strlen(buf) + 1;
    return;
}

static void resp_camera_set_vca_region_exit(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_advanced_motion(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_event_info *resp = (struct ms_smart_event_info *)param;

    if (!resp->format) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.sensitivity=%d&", resp->sensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.region=%s&", resp->area);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.objtype=%d&", resp->objtype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.ignore_time=%d&", resp->ignore_time);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonX=%s&", resp->polygonX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonY=%s&", resp->polygonY);
    } else {
        get_vca_common_json(param, size, buf, len, ADVANCED_MOTION);
    }
    
    *datalen = strlen(buf) + 1;
    return;
}

static void resp_camera_set_vca_advanced_motion(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}


static void resp_camera_get_vca_tamper_detection(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_event_info *resp = (struct ms_smart_event_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sensitivity=%d&", resp->sensitivity);

    *datalen = strlen(buf) + 1;
}

static void resp_camera_set_vca_tamper_detection(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_line_crossing(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct ms_linecrossing_info *resp = (struct ms_linecrossing_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.width=%d&", resp->width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.height=%d&", resp->height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.objtype=%d&", resp->objtype);
    for (i = 0; i < 4; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startX=%d&", i, resp->line[i].startX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startY=%d&", i, resp->line[i].startY);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopX=%d&", i, resp->line[i].stopX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopY=%d&", i, resp->line[i].stopY);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].direction=%d&", i, resp->line[i].direction);
    }

    *datalen = strlen(buf) + 1;
}

static void resp_camera_set_vca_line_crossing(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}


static void resp_camera_set_vca_loitering(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}


static void resp_camera_get_vca_human_detection(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_event_info *resp = (struct ms_smart_event_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.tracks=%d&", resp->show_tracks_enable);

    *datalen = strlen(buf) + 1;
}

static void resp_camera_set_vca_human_detection(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_people_count(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_event_people_cnt *resp = (struct ms_smart_event_people_cnt *)param;
    int i, j;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.width=%d&", resp->width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.height=%d&", resp->height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.osdEnable=%d&", resp->show_osd_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.fontSize=%d&", resp->osd_font_size);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.osdTextPosition=%d&", resp->osd_text_position);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.osdType=%d&", resp->osdType);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sensitivity=%d&", resp->sensitivity);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.lineNum=%d&", resp->lineNum);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.triggerType=%d&", resp->triggerType);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.triggerEnable=%d&", resp->triggerEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.totalAlarmEnable=%d&", resp->totalAlarmEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.supportOsdLarger=%d&", resp->supportOsdLarger);
    for (i = 0; i < PCNT_CNT_BIT_MAX; ++i) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.totalAlarmThresholds[%d]=%d&", i, resp->totalAlarmThresholds[i]);
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->lines[0].lineEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.startX=%d&", resp->lines[0].startX);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.startY=%d&", resp->lines[0].startY);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.stopX=%d&", resp->lines[0].stopX);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.stopY=%d&", resp->lines[0].stopY);

    for (i = 0; i < MAX_DAY_NUM_IPC; ++i) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; ++j) {
            snprintf(buf + strlen(buf), len - strlen(buf), "startTime[%d][%d]=%s&", i, j,
                resp->sche.schedule_day[i].schedule_item[j].start_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "endTime[%d][%d]=%s&", i, j,
                resp->sche.schedule_day[i].schedule_item[j].end_time);
        }
    }

    for (i = 0; i < MAX_IPC_PCNT_LINE; ++i) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].direction=%d&", i, resp->lines[i].direction);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].showCntEnable=%d&", i, resp->lines[i].showCntEnable);
        
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].autoResetEnable=%d&", i, resp->lines[i].autoResetEnable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].autoResetWeekday=%d&", i, resp->lines[i].autoResetWeekday);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].autoResetHour=%d&", i, resp->lines[i].autoResetHour);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].autoResetMin=%d&", i, resp->lines[i].autoResetMin);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].autoResetSec=%d&", i, resp->lines[i].autoResetSec);

        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].alarmEnable=%d&", i, resp->lines[i].alarmEnable);
        for (j = 0; j < PCNT_CNT_BIT_MAX; ++j) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].alarmThresholds[%d]=%d&", i, j, resp->lines[i].alarmThresholds[j]);
        }

        if (i == 0) {
            continue;
        }

        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].lineEnable=%d&", i, resp->lines[i].lineEnable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startX=%d&", i, resp->lines[i].startX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startY=%d&", i, resp->lines[i].startY);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopX=%d&", i, resp->lines[i].stopX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopY=%d&", i, resp->lines[i].stopY);
    }

    *datalen = strlen(buf) + 1;
}

static void resp_camera_set_vca_people_count(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_settings(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_vca_settings_info *resp = (struct ms_vca_settings_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.width=%d&", resp->width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.height=%d&", resp->height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.processFps=%d&", resp->vca_process_fps);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.scenarioMode=%d&", resp->vca_scenario_mode);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.cameraInstallation=%d&", resp->vca_camera_installation);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.analysisType=%d&", resp->vca_analysis_type);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectX=%d&", resp->minobject_window_rectangle_x);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectY=%d&", resp->minobject_window_rectangle_y);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectWidth=%d&", resp->minobject_window_rectangle_width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectHeight=%d&", resp->minobject_window_rectangle_height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectX=%d&", resp->maxobject_window_rectangle_x);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectY=%d&", resp->maxobject_window_rectangle_y);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectWidth=%d&", resp->maxobject_window_rectangle_width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectHeight=%d&", resp->maxobject_window_rectangle_height);

    *datalen = strlen(buf) + 1;
}

static void resp_camera_set_vca_settings(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_license(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_vca_license *resp = (struct ms_vca_license *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.licenseStatus=%d&", resp->vca_license_status);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.license=%s&", resp->vca_license);

    *datalen = strlen(buf) + 1;
}

static void resp_camera_set_vca_license(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_set_vca_clear_count(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_support(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_vca_support_info *resp = (struct ms_vca_support_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.support=%d&", resp->vca_support);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.type=%d&", resp->vca_type);

    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_vca_support_all(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    char support[MAX_CAMERA + 1] = {0};
    char vca_type[MAX_CAMERA + 1] = {0};
    struct ms_vca_support_all *resp = (struct ms_vca_support_all *)param;

    for (i = 0; i < MAX_CAMERA; i++) {
        if (resp->info[i].vca_support == 1) {
            support[i] = '1';
        } else {
            support[i] = '0';
        }
        if (resp->info[i].vca_type == 1) {
            vca_type[i] = '1';
        } else if (resp->info[i].vca_type == 2) {
            vca_type[i] = '2';
        } else {
            vca_type[i] = '0';
        }
    }
    support[MAX_CAMERA] = '\0';
    vca_type[MAX_CAMERA] = '\0';
    snprintf(buf + strlen(buf), len - strlen(buf), "r.support=%s&", support);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.type=%s&", vca_type);

    *datalen = strlen(buf) + 1;
}


static void resp_camera_get_vca_person_point(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct ms_personpoint_info *resp = (struct ms_personpoint_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.support=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.width=%d&", resp->width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.height=%d&", resp->height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.person_point_cnt=%d&", resp->person_point_cnt);

    for (i = 0; i < resp->person_point_cnt; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.point[%d].id=%d&", i, resp->point[i].id);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.point[%d].type=%d&", i, resp->point[i].type);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.point[%d].startX=%d&", i, resp->point[i].startX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.point[%d].startY=%d&", i, resp->point[i].startY);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.point[%d].stopX=%d&", i, resp->point[i].stopX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.point[%d].stopY=%d&", i, resp->point[i].stopY);

    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_get_vca_region_exit(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.format=", sValue, sizeof(sValue))) {
        res->format = atoi(sValue);
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}


static void req_camera_set_vca_region_exit(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[512] = {0};
    int ret = -1;
    memset(res, 0x0, sizeof(struct ms_smart_event_info));

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.sensitivity=", sValue, sizeof(sValue))) {
        res->sensitivity = atoi(sValue);
    }

    ret = set_vca_common_region_by_text(buf, res);
    if (ret) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (res->regionType == REGION_SINGLE) {
        if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
            res->enable = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.area=", sValue, sizeof(sValue))) {
            snprintf(res->area, sizeof(res->area), "%s", sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.objtype=", sValue, sizeof(sValue))) {
            res->objtype = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonX=", sValue, sizeof(sValue))) {
            snprintf(res->polygonX, sizeof(res->polygonX), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonY=", sValue, sizeof(sValue))) {
            snprintf(res->polygonY, sizeof(res->polygonY), "%s", sValue);
        }
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_advanced_motion(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.format=", sValue, sizeof(sValue))) {
        res->format = atoi(sValue);
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_set_vca_advanced_motion(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[512] = {0};
    int ret = -1;
    memset(res, 0x0, sizeof(struct ms_smart_event_info));

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.sensitivity=", sValue, sizeof(sValue))) {
        res->sensitivity = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.ignore_time=", sValue, sizeof(sValue))) {
        res->ignore_time = atoi(sValue);
    }

    ret = set_vca_common_region_by_text(buf, res);
    if (ret) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    
    if (res->regionType == REGION_SINGLE) {
        if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
            res->enable = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.region=", sValue, sizeof(sValue))) {
            snprintf(res->area, sizeof(res->area), "%s", sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.objtype=", sValue, sizeof(sValue))) {
            res->objtype = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.polygonX=", sValue, sizeof(sValue))) {
            snprintf(res->polygonX, sizeof(res->polygonX), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonY=", sValue, sizeof(sValue))) {
            snprintf(res->polygonY, sizeof(res->polygonY), "%s", sValue);
        }
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_tamper_detection(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}


static void req_camera_set_vca_tamper_detection(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[512] = {0};
    memset(res, 0x0, sizeof(struct ms_smart_event_info));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        res->enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.sensitivity=", sValue, sizeof(sValue))) {
        res->sensitivity = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.copyChn=", sValue, sizeof(sValue))) {
        snprintf(res->copyChn, sizeof(res->copyChn), sValue);
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}


static void req_camera_get_vca_line_crossing(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_set_vca_line_crossing(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_linecrossing_info *res = (struct ms_linecrossing_info *)param;
    char sValue[256] = {0};
    char temp[256] = {0};
    int i = 0;
    memset(res, 0x0, sizeof(struct ms_linecrossing_info));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        res->enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.objtype=", sValue, sizeof(sValue))) {
        res->objtype = atoi(sValue);
    }

    for (i = 0; i < 4; i++) {
        snprintf(temp, sizeof(temp), "r.line[%d].startX=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].startX = atoi(sValue);
        }

        snprintf(temp, sizeof(temp), "r.line[%d].startY=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].startY = atoi(sValue);
        }

        snprintf(temp, sizeof(temp), "r.line[%d].stopX=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].stopX = atoi(sValue);
        }

        snprintf(temp, sizeof(temp), "r.line[%d].stopY=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].stopY = atoi(sValue);
        }

        snprintf(temp, sizeof(temp), "r.line[%d].direction=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].direction = atoi(sValue);
        }
    }

    *datalen = sizeof(struct ms_linecrossing_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}


static void req_camera_get_vca_loitering(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.format=", sValue, sizeof(sValue))) {
        res->format = atoi(sValue);
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_vca_loitering(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_event_info *resp = (struct ms_smart_event_info *)param;

    if (!resp->format) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", resp->enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.minLoiteringTime=%d&", resp->min_loitering_time);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.loiteringObjectSize=%d&", resp->loitering_object_size);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.region=%s&", resp->area);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.objtype=%d&", resp->objtype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonX=%s&", resp->polygonX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonY=%s&", resp->polygonY);
    } else {
        get_vca_common_json(param, size, buf, len, LOITERING);
    }
    
    *datalen = strlen(buf) + 1;
    return;
}


static void req_camera_set_vca_loitering(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[512] = {0};
    int ret = -1;
    memset(res, 0x0, sizeof(struct ms_smart_event_info));

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    ret = set_vca_common_region_by_text(buf, res);
    if (ret) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (res->regionType == REGION_SINGLE) {
        if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
            res->enable = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.minLoiteringTime=", sValue, sizeof(sValue))) {
            res->min_loitering_time = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.loiteringObjectSize=", sValue, sizeof(sValue))) {
            res->loitering_object_size = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.region=", sValue, sizeof(sValue))) {
            snprintf(res->area, sizeof(res->area), "%s", sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.objtype=", sValue, sizeof(sValue))) {
            res->objtype = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.polygonX=", sValue, sizeof(sValue))) {
            snprintf(res->polygonX, sizeof(res->polygonX), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonY=", sValue, sizeof(sValue))) {
            snprintf(res->polygonY, sizeof(res->polygonY), "%s", sValue);
        }
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_human_detection(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_set_vca_human_detection(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_info *res = (struct ms_smart_event_info *)param;
    char sValue[512] = {0};
    memset(res, 0x0, sizeof(struct ms_smart_event_info));

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        res->enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.tracks=", sValue, sizeof(sValue))) {
        res->show_tracks_enable = atoi(sValue);
    }

    *datalen = sizeof(struct ms_smart_event_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_people_count(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_set_vca_people_count(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_event_people_cnt *res = (struct ms_smart_event_people_cnt *)param;
    int i, j;
    char sValue[256] = {0};
    char tmp[256];
    memset(res, 0x0, sizeof(struct ms_smart_event_people_cnt));

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.width=", sValue, sizeof(sValue))) {
        res->width = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.height=", sValue, sizeof(sValue))) {
        res->height = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.osdEnable=", sValue, sizeof(sValue))) {
        res->show_osd_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.fontSize=", sValue, sizeof(sValue))) {
        res->osd_font_size = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.osdTextPosition=", sValue, sizeof(sValue))) {
        res->osd_text_position = atoi(sValue);
    }
    
    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        res->lines[0].lineEnable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.startX=", sValue, sizeof(sValue))) {
        res->lines[0].startX = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.startY=", sValue, sizeof(sValue))) {
        res->lines[0].startY = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.stopX=", sValue, sizeof(sValue))) {
        res->lines[0].stopX = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.stopY=", sValue, sizeof(sValue))) {
        res->lines[0].stopY = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.lineNum=", sValue, sizeof(sValue))) {
        res->lineNum = atoi(sValue);
    } else {
        res->lineNum = 1;
    }

    if (!sdkp2p_get_section_info(buf, "r.triggerType=", sValue, sizeof(sValue))) {
        res->triggerType = atoi(sValue);
    }
    
    if (!sdkp2p_get_section_info(buf, "r.triggerEnable=", sValue, sizeof(sValue))) {
        res->triggerEnable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.totalAlarmEnable=", sValue, sizeof(sValue))) {
        res->totalAlarmEnable = atoi(sValue);
    }

    for (i = 0; i < PCNT_CNT_BIT_MAX; ++i) {
        snprintf(tmp, sizeof(tmp), "r.totalAlarmThreshold[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->totalAlarmThresholds[i] = atoi(sValue);
        }
    }

    for (i = 0; i < MAX_DAY_NUM_IPC; ++i) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; ++j) {
            snprintf(tmp, sizeof(tmp) - 1, "r.startTime[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(res->sche.schedule_day[i].schedule_item[j].start_time, 
                    sizeof(res->sche.schedule_day[i].schedule_item[j].start_time), sValue);
            }
            snprintf(tmp, sizeof(tmp) - 1, "r.endTime[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(res->sche.schedule_day[i].schedule_item[j].end_time, 
                    sizeof(res->sche.schedule_day[i].schedule_item[j].end_time), sValue);
            }
        }
    }

    if (res->lineNum == 4) {
        if (!sdkp2p_get_section_info(buf, "r.osdType=", sValue, sizeof(sValue))) {
            res->osdType = atoi(sValue);
        }
        
        if (!sdkp2p_get_section_info(buf, "r.sensitivity=", sValue, sizeof(sValue))) {
            res->sensitivity = atoi(sValue);
        }

        for (i = 0; i < MAX_IPC_PCNT_LINE; ++i) {
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].direction=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].direction = atoi(sValue);
            }
            
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].showCntEnable=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].showCntEnable = atoi(sValue);
            }
        
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].autoResetEnable=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].autoResetEnable = atoi(sValue);
            }
        
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].autoResetWeekday=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].autoResetWeekday = atoi(sValue);
            }
        
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].autoResetHour=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].autoResetHour = atoi(sValue);
            }
        
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].autoResetMin=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].autoResetMin = atoi(sValue);
            }
        
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].autoResetSec=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].autoResetSec = atoi(sValue);
            }
            
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].alarmEnable=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].alarmEnable = atoi(sValue);
            }

            for (j = 0; j < PCNT_CNT_BIT_MAX; ++i) {
                snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].alarmThreshold[%d]=", i, j);
                if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                    res->lines[i].alarmThresholds[j] = atoi(sValue);
                }
            }
            
            if (i == 0) {
                continue;
            } 
            
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].lineEnable=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].lineEnable = atoi(sValue);
            }
            
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].startX=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].startX = atoi(sValue);
            }
            
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].startY=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].startY = atoi(sValue);
            }
            
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].stopX=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].stopX = atoi(sValue);
            }
            
            snprintf(tmp, sizeof(tmp) - 1, "r.line[%d].stopY=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                res->lines[i].stopY = atoi(sValue);
            }
        }

    }
    
    *datalen = sizeof(struct ms_smart_event_people_cnt);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_set_vca_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_vca_settings_info *res = (struct ms_vca_settings_info *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(struct ms_vca_settings_info));

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.processFps=", sValue, sizeof(sValue))) {
        res->vca_process_fps = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.scenarioMode=", sValue, sizeof(sValue))) {
        res->vca_scenario_mode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.cameraInstallation=", sValue, sizeof(sValue))) {
        res->vca_camera_installation = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.analysisType=", sValue, sizeof(sValue))) {
        res->vca_analysis_type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.minObjectX=", sValue, sizeof(sValue))) {
        res->minobject_window_rectangle_x = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.minObjectY=", sValue, sizeof(sValue))) {
        res->minobject_window_rectangle_y = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.minObjectWidth=", sValue, sizeof(sValue))) {
        res->minobject_window_rectangle_width = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.minObjectHeight=", sValue, sizeof(sValue))) {
        res->minobject_window_rectangle_height = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.maxObjectX=", sValue, sizeof(sValue))) {
        res->maxobject_window_rectangle_x = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.maxObjectY=", sValue, sizeof(sValue))) {
        res->maxobject_window_rectangle_y = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.maxObjectWidth=", sValue, sizeof(sValue))) {
        res->maxobject_window_rectangle_width = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.maxObjectHeight=", sValue, sizeof(sValue))) {
        res->maxobject_window_rectangle_height = atoi(sValue);
    }

    *datalen = sizeof(struct ms_vca_settings_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}


static void req_camera_get_vca_license(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_set_vca_license(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_vca_license *res = (struct ms_vca_license *)param;
    char sValue[512] = {0};
    memset(res, 0x0, sizeof(struct ms_vca_license));

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.license=", sValue, sizeof(sValue))) {
        snprintf(res->vca_license, sizeof(res->vca_license), "%s", sValue);
    }

    *datalen = sizeof(struct ms_vca_license);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_set_vca_clear_count(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_vca_cleancount_info info;

    memset(&info, 0x0, sizeof(struct ms_vca_cleancount_info));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        info.chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.incount=", sValue, sizeof(sValue))) {
        info.incount = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.outcount=", sValue, sizeof(sValue))) {
        info.outcount = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.line=", sValue, sizeof(sValue))) {
        info.line = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.resetdb=", sValue, sizeof(sValue))) {
        info.resetdb = atoi(sValue);
    }
    
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct ms_vca_cleancount_info), SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_support_all(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    *datalen = 0;
    sdkp2p_send_msg(conf, conf->req, NULL, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_vca_person_point(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}


static void req_camera_get_search_nvr(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_search_nvr(void *param, int size, char *buf, int len, int *datalen)
{
    int cnt = 0;
    struct resq_search_nvr *nvr = (struct resq_search_nvr *)param;
    int i = 0;

    cnt = size / sizeof(struct resq_search_nvr);
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", cnt);
    for (i = 0; i < cnt; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "hostname[%d]=%s&", i, nvr[i].hostname);
        snprintf(buf + strlen(buf), len - strlen(buf), "ipaddr[%d]=%s&", i, nvr[i].ipaddr);
        //snprintf(buf+strlen(buf),len-strlen(buf), "port[%d]=%d&", i, nvr[i].port);
        snprintf(buf + strlen(buf), len - strlen(buf), "netmask[%d]=%s&", i, nvr[i].netmask);
        snprintf(buf + strlen(buf), len - strlen(buf), "gateway[%d]=%s&", i, nvr[i].gateway);
        snprintf(buf + strlen(buf), len - strlen(buf), "dns[%d]=%s&", i, nvr[i].dns);
        snprintf(buf + strlen(buf), len - strlen(buf), "sn[%d]=%s&", i, nvr[i].sn);
        snprintf(buf + strlen(buf), len - strlen(buf), "devicetype[%d]=%s&", i, nvr[i].devicetype);
        snprintf(buf + strlen(buf), len - strlen(buf), "software[%d]=%s&", i, nvr[i].software);
        snprintf(buf + strlen(buf), len - strlen(buf), "model[%d]=%s&", i, nvr[i].model);
        snprintf(buf + strlen(buf), len - strlen(buf), "uptime[%d]=%s&", i, nvr[i].uptime);
        snprintf(buf + strlen(buf), len - strlen(buf), "nonce[%d]=%s&", i, nvr[i].nonce);
        snprintf(buf + strlen(buf), len - strlen(buf), "http_port[%d]=%d&", i, nvr[i].http_port);
        //snprintf(buf+strlen(buf),len-strlen(buf), "oem_model[%d]=%s&", i, nvr[i].oem_model);
        snprintf(buf + strlen(buf), len - strlen(buf), "upsec[%d]=%d&", i, nvr[i].upsec);
        snprintf(buf + strlen(buf), len - strlen(buf), "uptone[%d]=%s&", i, nvr[i].uptone);
        snprintf(buf + strlen(buf), len - strlen(buf), "active[%d]=%d&", i, nvr[i].active_status);
        snprintf(buf + strlen(buf), len - strlen(buf), "failover_mode[%d]=%d&", i, nvr[i].failover_mode);
        //snprintf(buf+strlen(buf),len-strlen(buf), "failover_ip[%d]=%s&", i, nvr[i].failover_ip);
        //snprintf(buf+strlen(buf),len-strlen(buf), "password[%d]=%s&", i, nvr[i].password);
        //snprintf(buf+strlen(buf),len-strlen(buf), "max_camera[%d]=%d&", i, nvr[i].max_camera);
        //snprintf(buf+strlen(buf),len-strlen(buf), "bond_width[%d]=%d&", i, nvr[i].bond_width);
        //snprintf(buf+strlen(buf),len-strlen(buf), "device_name[%d]=%s&", i, nvr[i].device_name);
        //snprintf(buf+strlen(buf),len-strlen(buf), "username[%d]=%s&", i, nvr[i].username);
        snprintf(buf + strlen(buf), len - strlen(buf), "mac[%d]=%s&", i, nvr[i].mac);
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *channel = (int *)buf;

    if (*channel < 0 || *channel >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, channel, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_ipc_list(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    *datalen = 0;
    sdkp2p_send_msg(conf, conf->req, NULL, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_ipc_param(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_ipc_param;
    int i;
    struct resp_get_ipc_param *res = (struct resp_get_ipc_param *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.ret=%d&", res->ret);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", res->chnid);
    //main
    for (i = 0; i < 24; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.res[%d].height=%d&", i, res->main_range.res[i].height);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.res[%d].width=%d&", i, res->main_range.res[i].width);
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.framerate_max=%d&", res->main_range.framerate_max);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.framerate_min=%d&", res->main_range.framerate_min);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.bitrate_max=%d&", res->main_range.bitrate_max);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.bitrate_min=%d&", res->main_range.bitrate_min);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.vcodec_type=%d&", res->main_range.vcodec_type);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.cur_res.height=%d&", res->main_range.cur_res.height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.cur_res.width=%d&", res->main_range.cur_res.width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.framerate=%d&", res->main_range.framerate);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.bitrate=%d&", res->main_range.bitrate);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.iframeinterval=%d&", res->main_range.iframeinterval);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.iframeIntervalMin=%d&", res->main_range.iframeIntervalMin);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.main_range.iframeIntervalMax=%d&", res->main_range.iframeIntervalMax);
    //sub
    for (i = 0; i < 24; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.res[%d].height=%d&", i, res->sub_range.res[i].height);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.res[%d].width=%d&", i, res->sub_range.res[i].width);
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.framerate_max=%d&", res->sub_range.framerate_max);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.framerate_min=%d&", res->sub_range.framerate_min);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.bitrate_max=%d&", res->sub_range.bitrate_max);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.bitrate_min=%d&", res->sub_range.bitrate_min);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.vcodec_type=%d&", res->sub_range.vcodec_type);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.cur_res.height=%d&", res->sub_range.cur_res.height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.cur_res.width=%d&", res->sub_range.cur_res.width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.framerate=%d&", res->sub_range.framerate);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.bitrate=%d&", res->sub_range.bitrate);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.iframeinterval=%d&", res->sub_range.iframeinterval);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.iframeIntervalMin=%d&", res->sub_range.iframeIntervalMin);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_range.iframeIntervalMax=%d&", res->sub_range.iframeIntervalMax);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.sub_enable=%d&", res->sub_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.audio_enable=%d&", res->audio_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.videoType_enable=%d&", res->videoType_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.ipc_type=%d&", res->ipc_type);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.supportsmartstream=%d&", res->supportsmartstream);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.smartStream=%d&", res->smartStream);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.smartStream_level=%d&", res->smartStream_level);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.subSupportSmartStream=%d&", res->subSupportSmartStream);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.subSmartStream=%d&", res->subSmartStream);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.subSmartStream_level=%d&", res->subSmartStream_level);
    snprintf(buf + strlen(buf), len - strlen(buf), "sub_stream=%d&", res->sub_stream);
    snprintf(buf + strlen(buf), len - strlen(buf), "mainStream=%d&", res->mainStream);
    snprintf(buf + strlen(buf), len - strlen(buf), "mainGop=%d&", res->streamGop[0]);
    snprintf(buf + strlen(buf), len - strlen(buf), "subGop=%d&", res->streamGop[1]);
    snprintf(buf + strlen(buf), len - strlen(buf), "thirdGop=%d&", res->streamGop[2]);

    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_ipc_list(void *param, int size, char *buf, int len, int *datalen)
{
    int i;
    struct resq_get_ipcdev *ipnc_list = (struct resq_get_ipcdev *)param;
    int num = size / sizeof(struct resq_get_ipcdev);

    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", num);
    for (i = 0; i < num; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].chanid=%d&", i, ipnc_list[i].chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].port=%d&", i, ipnc_list[i].port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].protocol=%d&", i, ipnc_list[i].protocol);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].state=%d&", i, ipnc_list[i].state);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].main_rtsp_port=%d&", i, ipnc_list[i].main_rtsp_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ipaddr=%s&", i, ipnc_list[i].ipaddr);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].manufacturer=%s&", i, ipnc_list[i].manufacturer);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].hwid=%s&", i, ipnc_list[i].hwid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].sn=%s&", i, ipnc_list[i].sn);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].snReal=%s&", i, ipnc_list[i].real_sn);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].fwversion=%s&", i, ipnc_list[i].fwversion);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].mac=%s&", i, ipnc_list[i].mac);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].model=%s&", i, ipnc_list[i].model);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].model_id=%d&", i, ipnc_list[i].model_id);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].mrespath=%s&", i, ipnc_list[i].mrespath);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].srespath=%s&", i, ipnc_list[i].srespath);

        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].poe_channel=%d&", i, ipnc_list[i].poe_channel);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].physical_port=%d&", i, ipnc_list[i].physical_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].alarmin=%d&", i, ipnc_list[i].alarmin);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].alarmout=%d&", i, ipnc_list[i].alarmout);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].whiteled=%d&", i, ipnc_list[i].whiteled);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].led_alarm=%d&", i, ipnc_list[i].led_alarm);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].led_manual=%d&", i, ipnc_list[i].led_manual);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].minorid=%d&", i, ipnc_list[i].minorid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].subState=%d&", i, ipnc_list[i].sub_state);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].connectState=%d&", i, ipnc_list[i].connect_state);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ddns=%s&", i, ipnc_list[i].ddns);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].chipinfo=%s&", i, ipnc_list[i].chipinfo);
    }

    *datalen = strlen(buf) + 1;
}

static void resp_camera_search_ipc(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resq_search_ipc*;
    struct resq_search_ipc *res = (struct resq_search_ipc *)param;
    int num = size / sizeof(struct resq_search_ipc);
    int i;
    char encode[256] = {0};
    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", num);
    for (i = 0; i < num; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "ip[%d]=%s&", i, res[i].ipaddr);
        snprintf(buf + strlen(buf), len - strlen(buf), "mac[%d]=%s&", i, res[i].mac);
        snprintf(buf + strlen(buf), len - strlen(buf), "port[%d]=%d&", i, res[i].port);
        snprintf(buf + strlen(buf), len - strlen(buf), "protocol[%d]=%d&", i, res[i].protocol);
        //snprintf(buf+strlen(buf),len-strlen(buf),"manu[%d]=%s&",i,res[i].manufacturer);
        //snprintf(buf+strlen(buf),len-strlen(buf),"hwid[%d]=%s&",i,res[i].hwid);
        snprintf(buf+strlen(buf),len-strlen(buf),"sn[%d]=%s&",i,res[i].sn);
        snprintf(buf+strlen(buf),len-strlen(buf),"snReal[%d]=%s&",i,res[i].real_sn);
        snprintf(buf + strlen(buf), len - strlen(buf), "fwversion[%d]=%s&", i, res[i].fwversion);
        snprintf(buf + strlen(buf), len - strlen(buf), "model[%d]=%s&", i, res[i].model);
        //snprintf(buf+strlen(buf),len-strlen(buf),"model_id[%d]=%d&",i,res[i].model_id);
        snprintf(buf + strlen(buf), len - strlen(buf), "netmask[%d]=%s&", i, res[i].netmask);
        snprintf(buf + strlen(buf), len - strlen(buf), "dns[%d]=%s&", i, res[i].dns);
        snprintf(buf + strlen(buf), len - strlen(buf), "gateway[%d]=%s&", i, res[i].gateway);
        snprintf(buf + strlen(buf), len - strlen(buf), "netif[%d]=%s&", i, res[i].netif_from);
        get_url_encode(res[i].device_name, strlen(res[i].device_name), encode, sizeof(encode));
        snprintf(buf + strlen(buf), len - strlen(buf), "r.text_name_sub=%s&", encode);
        snprintf(buf + strlen(buf), len - strlen(buf), "name[%d]=%s&", i, encode);
        snprintf(buf + strlen(buf), len - strlen(buf), "active[%d]=%d&", i, res[i].active_status);
    }

    *datalen = strlen(buf) + 1;
}

static void get_poe_power_start_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resq_get_poe_power;
    int i;
    struct resq_get_poe_power *res = (struct resq_get_poe_power *)param;
    for (i = 0; i < MAX_PORT_NUM; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.poe_port_power[%d]=%d&", i, res->poe_port_power[i]);
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "r.port_num=%d&", res->port_num);

    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_all_ipctype(void *param, int size, char *buf, int len, int *datalen)
{
    int iCnt;
    struct resp_cam_model_info *resp = (struct resp_cam_model_info *)(param);

    snprintf(buf + strlen(buf), len - strlen(buf), "ret_cnt=%d&", MAX_CAMERA);
    for (iCnt = 0; iCnt < MAX_CAMERA; iCnt++) {
        if (len - strlen(buf) < 300) {
            break;
        }
        snprintf(buf + strlen(buf), len - strlen(buf), "chanid[%d]=%d&", iCnt, resp->modelInfo[iCnt].chnid);
        snprintf(buf + strlen(buf), len - strlen(buf), "model[%d]=%s&", iCnt, resp->modelInfo[iCnt].model);
        snprintf(buf + strlen(buf), len - strlen(buf), "fwversion[%d]=%s&", iCnt, resp->modelInfo[iCnt].fwversion);
    }

    *datalen = strlen(buf) + 1;
}

static void resp_camera_get_remotelive_rtspaddrs(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_remotelive_rtspaddrs;
    struct resp_get_remotelive_rtspaddrs *res = (struct resp_get_remotelive_rtspaddrs *)param;
    int num = size / sizeof(struct resp_get_remotelive_rtspaddrs);
    int i;
    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", num);
    for (i = 0; i < num; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].chanid=%d&", i, res[i].chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ipc_main=%s&", i, res[i].ipc_main);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ipc_sub=%s&", i, res[i].ipc_sub);
        //main
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.chanid=%d&", i, res[i].nvr_main.chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.port=%d&", i, res[i].nvr_main.port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.external_port=%d&", i, res[i].nvr_main.external_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.external_http_port=%d&", i,
                 res[i].nvr_main.external_http_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.ddns_port=%d&", i, res[i].nvr_main.ddns_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.ddns_http_port=%d&", i, res[i].nvr_main.ddns_http_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.direct=%d&", i, res[i].nvr_main.direct);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.result=%d&", i, res[i].nvr_main.result);

        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.username=%s&", i, res[i].nvr_main.username);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.password=%s&", i, res[i].nvr_main.password);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.rtspaddr=%s&", i, res[i].nvr_main.rtspaddr);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.respath=%s&", i, res[i].nvr_main.respath);
        //sub
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.chanid=%d&", i, res[i].nvr_sub.chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.port=%d&", i, res[i].nvr_sub.port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.external_port=%d&", i, res[i].nvr_sub.external_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.external_http_port=%d&", i,
                 res[i].nvr_sub.external_http_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.ddns_port=%d&", i, res[i].nvr_sub.ddns_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.ddns_http_port=%d&", i, res[i].nvr_sub.ddns_http_port);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.direct=%d&", i, res[i].nvr_sub.direct);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.result=%d&", i, res[i].nvr_sub.result);

        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.username=%s&", i, res[i].nvr_sub.username);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.password=%s&", i, res[i].nvr_sub.password);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.rtspaddr=%s&", i, res[i].nvr_sub.rtspaddr);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.respath=%s&", i, res[i].nvr_sub.respath);

        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_main.httppath=/ms/streaming/%s&", i, res[i].nvr_main.respath);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].nvr_sub.httppath=/ms/streaming/%s&", i, res[i].nvr_sub.respath);
    }
    *datalen = strlen(buf) + 1;
}

static void get_ipcalarm_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_ipcalarm;
    struct resp_get_ipcalarm *res = (struct resp_get_ipcalarm *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.ret=%d&", res->ret);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.chanid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", res->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.type=%d&", res->type);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.tri_actions=%u&", res->tri_actions);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.tri_channels=%u&", res->tri_channels);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.tri_channels_ex=%s&", res->tri_channels_ex);

    *datalen = strlen(buf) + 1;
}

static void set_ipcalarm_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void get_ipcvolume_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_ipcvolume;

    struct resp_get_ipcvolume *res = (struct resp_get_ipcvolume *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.ret=%d&", res->ret);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.chanid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", res->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.type=%d&", res->type);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.volume=%d&", res->volume);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.duration=%d&", res->duration);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.on_event=%d&", res->on_event);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.off_event=%d&", res->off_event);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.ongoing_event=%d&", res->ongoing_event);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.tri_actions=%u&", res->tri_actions);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.tri_channels=%u&", res->tri_channels);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.tri_channels_ex=%s&", res->tri_channels_ex);

    *datalen = strlen(buf) + 1;
}

static void set_ipcvolume_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void get_ipc_disconnect_num_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int ipc_disconnect_num[MAX_CAMERA];
    int i;
    int *res = (int *)param;
    for (i = 0; i < MAX_CAMERA; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d]=%d&", i, res[i]);
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_add_ipc_channel(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct camera ipnc = {0};
    struct osd osd = {0};

    if (sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    ipnc.id = atoi(sValue);
    read_camera(SQLITE_FILE_NAME, &ipnc, ipnc.id);
    read_osd(SQLITE_FILE_NAME, &osd, ipnc.id);

    if (!sdkp2p_get_section_info(buf, "Name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(osd.name, sizeof(osd.name), "%s", sValue);
    }

    write_osd(SQLITE_FILE_NAME, &osd);

    if (!sdkp2p_get_section_info(buf, "minorid=", sValue, sizeof(sValue))) {
        ipnc.minorid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        ipnc.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ip_addr=", sValue, sizeof(sValue))) {
        snprintf(ipnc.ip_addr, sizeof(ipnc.ip_addr), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "manage_port=", sValue, sizeof(sValue))) {
        ipnc.manage_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "camera_protocol=", sValue, sizeof(sValue))) {
        ipnc.camera_protocol = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(ipnc.username, sizeof(ipnc.username), "%s", sValue);
    }
    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(ipnc.password, sizeof(ipnc.password), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "transmit_protocol=", sValue, sizeof(sValue))) {
        ipnc.transmit_protocol = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_rtsp_enable=", sValue, sizeof(sValue))) {
        ipnc.sub_rtsp_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sync_time=", sValue, sizeof(sValue))) {
        ipnc.sync_time = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "main_rtsp_port=", sValue, sizeof(sValue))) {
        ipnc.main_rtsp_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "main_source_path=", sValue, sizeof(sValue))) {
        snprintf(ipnc.main_source_path, sizeof(ipnc.main_source_path), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "main_rtspurl=", sValue, sizeof(sValue))) {
        snprintf(ipnc.main_rtspurl, sizeof(ipnc.main_rtspurl), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_rtsp_port=", sValue, sizeof(sValue))) {
        ipnc.sub_rtsp_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_source_path=", sValue, sizeof(sValue))) {
        snprintf(ipnc.sub_source_path, sizeof(ipnc.sub_source_path), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_rtspurl=", sValue, sizeof(sValue))) {
        snprintf(ipnc.sub_rtspurl, sizeof(ipnc.sub_rtspurl), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "record_stream=", sValue, sizeof(sValue))) {
        ipnc.record_stream = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "poe_channel=", sValue, sizeof(sValue))) {
        ipnc.poe_channel = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "physical_port=", sValue, sizeof(sValue))) {
        ipnc.physical_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "mac_addr=", sValue, sizeof(sValue))) {
        snprintf(ipnc.mac_addr, sizeof(ipnc.mac_addr), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ddns=", sValue, sizeof(sValue))) {
        snprintf(ipnc.ddns, sizeof(ipnc.ddns), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "httpsPort=", sValue, sizeof(sValue))) {
        ipnc.https_port = atoi(sValue);
    }
    write_camera(SQLITE_FILE_NAME, &ipnc);
    sdkp2p_send_msg(conf, REQUEST_FLAG_ADDNEW_IPC, (void *)&ipnc.id, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUP_OP_ADD_IP_CHANNEL_REMOTE, SUB_PARAM_ADD_IPC, ipnc.id + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_camera_modify_ipc_channel(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[64] = {0};
    struct camera ipnc = {0};
    struct osd osd = {0};

    if (sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    ipnc.id = atoi(sValue);
    read_camera(SQLITE_FILE_NAME, &ipnc, ipnc.id);
    read_osd(SQLITE_FILE_NAME, &osd, ipnc.id);

    if (!sdkp2p_get_section_info(buf, "Name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(osd.name, sizeof(osd.name), "%s", sValue);
    }

    write_osd(SQLITE_FILE_NAME, &osd);

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        ipnc.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ip_addr=", sValue, sizeof(sValue))) {
        snprintf(ipnc.ip_addr, sizeof(ipnc.ip_addr), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "manage_port=", sValue, sizeof(sValue))) {
        ipnc.manage_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "camera_protocol=", sValue, sizeof(sValue))) {
        ipnc.camera_protocol = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(ipnc.username, sizeof(ipnc.username), "%s", sValue);
    }
    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(ipnc.password, sizeof(ipnc.password), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "transmit_protocol=", sValue, sizeof(sValue))) {
        ipnc.transmit_protocol = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_rtsp_enable=", sValue, sizeof(sValue))) {
        ipnc.sub_rtsp_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sync_time=", sValue, sizeof(sValue))) {
        ipnc.sync_time = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "main_rtsp_port=", sValue, sizeof(sValue))) {
        ipnc.main_rtsp_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "main_source_path=", sValue, sizeof(sValue))) {
        snprintf(ipnc.main_source_path, sizeof(ipnc.main_source_path), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "main_rtspurl=", sValue, sizeof(sValue))) {
        snprintf(ipnc.main_rtspurl, sizeof(ipnc.main_rtspurl), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_rtsp_port=", sValue, sizeof(sValue))) {
        ipnc.sub_rtsp_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_source_path=", sValue, sizeof(sValue))) {
        snprintf(ipnc.sub_source_path, sizeof(ipnc.sub_source_path), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sub_rtspurl=", sValue, sizeof(sValue))) {
        snprintf(ipnc.sub_rtspurl, sizeof(ipnc.sub_rtspurl), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "poe_channel=", sValue, sizeof(sValue))) {
        ipnc.poe_channel = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "physical_port=", sValue, sizeof(sValue))) {
        ipnc.physical_port = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "record_stream=", sValue, sizeof(sValue))) {
        ipnc.record_stream = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "anr=", sValue, sizeof(sValue))) {
        ipnc.anr = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "minorid=", sValue, sizeof(sValue))) {
        ipnc.minorid = atoi(sValue);
    }

    write_camera(SQLITE_FILE_NAME, &ipnc);
    sdkp2p_send_msg(conf, REQUEST_FLAG_UPDATE_IPC, (void *)&ipnc.id, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_ADDNEW_IPC, (void *)&ipnc.id, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RECPARAM, (void *)&ipnc.id, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_EDIT_IPC, ipnc.id + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_camera_remove_ipc_channel(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    int i = 0;
    char sValue[256] = {0};
    struct privacy_mask mask = {0};
    struct camera ipnc;
    struct osd osd;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid > MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    memset(&mask, 0, sizeof(struct privacy_mask));
    read_privacy_mask(SQLITE_FILE_NAME, &mask, chnid);
    if (mask.id == chnid) {
        for (i = 0; i < MAX_MASK_AREA_NUM; i++) {
            mask.area[i].start_x = 0;
            mask.area[i].start_y = 0;
            mask.area[i].area_width = 0;
            mask.area[i].area_height = 0;
            mask.area[i].enable = 0;
        }
    }
    write_privacy_mask(SQLITE_FILE_NAME, &mask);

    read_camera(SQLITE_FILE_NAME, &ipnc, chnid);
    int record_stream = ipnc.record_stream;
    memset(&ipnc, 0, sizeof(struct camera));
    ipnc.id = chnid;
    ipnc.type = CAMERA_TYPE_IPNC;
    ipnc.record_stream = record_stream;
    ipnc.enable = 0;
    write_camera(SQLITE_FILE_NAME, &ipnc);

    memset(&osd, 0, sizeof(struct osd));
    read_osd(SQLITE_FILE_NAME, &osd, chnid);
    snprintf(osd.name, sizeof(osd.name), "CAM%d", (chnid + 1));
    write_osd(SQLITE_FILE_NAME, &osd);
    sdkp2p_send_msg(conf, REQUEST_FLAG_REMOVE_IPC, (void *)&chnid, sizeof(int), SDKP2P_NOT_CALLBACK);

    struct other_poe_camera other_ipc;
    other_ipc.other_poe_channel = 0;
    other_ipc.id = chnid;
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_OTHER_POE, (void *)&other_ipc, sizeof(struct other_poe_camera),
                    SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUP_OP_DELETE_IP_CHANNEL_REMOTE, SUB_PARAM_DEL_IPC, chnid + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_camera_get_camera_protocol(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i, cnt = 0;
    struct ipc_protocol *ipc = NULL;

    read_ipc_protocols(SQLITE_FILE_NAME, &ipc, &cnt);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ret_cnt=%d&", cnt);
    for (i = 0; i < cnt; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "pro_id[%d]=%d&", i, ipc[i].pro_id);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "pro_name[%d]=%s&", i, ipc[i].pro_name);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "function[%d]=%d&", i, ipc[i].function);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable[%d]=%d&", i, ipc[i].enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "display_model[%d]=%d&", i,
                 ipc[i].display_model);
    }

    release_ipc_protocol(&ipc);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_camera_search_ipc(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int pProId = -1;

    if (sdkp2p_get_section_info(buf, "pro_id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    pProId = atoi(sValue);
    if (pProId < IPC_PROTOCOL_ONVIF || pProId > IPC_PROTOCOL_MAX) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, &pProId, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void req_camera_set_poe_conn_password(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char username[64] = {0};
    char password[64] = {0};
    struct camera ipnc;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (sdkp2p_get_section_info(buf, "userName=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(username, sizeof(username), "%s", sValue);
    if (sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(password, sizeof(password), "%s", sValue);
    memset(&ipnc, 0, sizeof(struct camera));
    read_camera(SQLITE_FILE_NAME, &ipnc, chnid);
    snprintf(ipnc.username, sizeof(ipnc.username), "%s", username);
    snprintf(ipnc.password, sizeof(ipnc.password), "%s", password);
    write_camera(SQLITE_FILE_NAME, &ipnc);
    sdkp2p_send_msg(conf, REQUEST_FLAG_UPDATE_IPC, (void *)&ipnc.id, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_EDIT_IPC, chnid + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_camera_try_test_ipcconnect(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_try_test_ipcconnect req_info;

    memset(&req_info, 0, sizeof(struct req_try_test_ipcconnect));
    if (sdkp2p_get_section_info(buf, "ip=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(req_info.ip, sizeof(req_info.ip), "%s", sValue);
    if (!sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        req_info.port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "user=", sValue, sizeof(sValue))) {
        snprintf(req_info.user, sizeof(req_info.user), "%s", sValue);
    }
    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(req_info.password, sizeof(req_info.password), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "protocol=", sValue, sizeof(sValue))) {
        req_info.protocol = atoi(sValue);
    }
    if (req_info.port < 80) {
        req_info.port = 80;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&req_info, sizeof(struct req_try_test_ipcconnect), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_try_test_ipcconnect(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_model_type(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_ipc_model_type(void *param, int size, char *buf, int len, int *datalen)
{
    //int res;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_camera_get_vca_leftremove(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_leftremove_info *res = (struct ms_smart_leftremove_info *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.format=", sValue, sizeof(sValue))) {
        res->format = atoi(sValue);
    }

    *datalen = sizeof(struct ms_smart_leftremove_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}
static void resp_camera_get_vca_leftremove(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_smart_leftremove_info *resp = (struct ms_smart_leftremove_info *)param;
    if (!resp->format) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.left_enable=%d&", resp->left_enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.remove_enable=%d&", resp->remove_enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.min_time=%d&", resp->min_time);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.region=%s&", resp->area);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.sensitivity=%d&", resp->sensitivity);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonX=%s&", resp->polygonX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.polygonY=%s&", resp->polygonY);
    } else {
        get_vca_leftremove_json(param, size, buf, len);
    }

    *datalen = strlen(buf) + 1;
    return;
}
static void req_camera_set_vca_leftremove(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_smart_leftremove_info *res = (struct ms_smart_leftremove_info *)param;
    char sValue[512] = {0};
    int ret = -1;
    memset(res, 0x0, sizeof(struct ms_smart_leftremove_info));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.sensitivity=", sValue, sizeof(sValue))) {
        res->sensitivity = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.min_time=", sValue, sizeof(sValue))) {
        res->min_time = atoi(sValue);
    }

    ret = set_vca_leftremove_region_by_text(buf, res);
    if (ret) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    
    if (res->regionType == REGION_SINGLE) {
        if (!sdkp2p_get_section_info(buf, "r.left_enable=", sValue, sizeof(sValue))) {
            res->left_enable = atoi(sValue);
        }

        if (!sdkp2p_get_section_info(buf, "r.remove_enable=", sValue, sizeof(sValue))) {
            res->remove_enable = atoi(sValue);
        }

        if (!sdkp2p_get_section_info(buf, "r.region=", sValue, sizeof(sValue))) {
            snprintf(res->area, sizeof(res->area), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonX=", sValue, sizeof(sValue))) {
            snprintf(res->polygonX, sizeof(res->polygonX), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buf, "r.polygonY=", sValue, sizeof(sValue))) {
            snprintf(res->polygonY, sizeof(res->polygonY), "%s", sValue);
        }
    }

    *datalen = sizeof(struct ms_smart_leftremove_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}
static void resp_camera_set_vca_leftremove(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_image_roi(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, chanid = -1;
    char sValue[512] = {0}, tmp[64] = {0};
    struct set_image_roi *res = (struct set_image_roi *)param;
    memset(res, 0x0, sizeof(struct set_image_roi));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chanid = atoi(sValue);
    }
    if (chanid < 0 || chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    res->chanid = chanid;
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        res->enable = atoi(sValue);
    }
    for (i = 0; i < MAX_IMAGE_ROI_AREA; i++) {
        res->area[i].enable = 1;
        snprintf(tmp, sizeof(tmp), "left[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->area[i].left = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "top[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->area[i].top = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "right[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->area[i].right = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "bottom[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->area[i].bottom = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "enable[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->area[i].enable = atoi(sValue);
        }
    }
    *datalen = sizeof(struct set_image_roi);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_image_roi(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_image_roi(void *param, int size, char *buf, int len, int *datalen)
{
    int iCnt;
    struct set_image_roi *resp = (struct set_image_roi *)(param);
    snprintf(buf + strlen(buf), len - strlen(buf), "retCnt=%d&", MAX_IMAGE_ROI_AREA);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", resp->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "type=%d&", resp->type);
    snprintf(buf + strlen(buf), len - strlen(buf), "sdkversion=%s&", resp->sdkversion);
    for (iCnt = 0; iCnt < MAX_IMAGE_ROI_AREA; iCnt++) {
        if (len - strlen(buf) < 300) {
            break;
        }
        snprintf(buf + strlen(buf), len - strlen(buf), "id[%d]=%d&", iCnt, iCnt);
        snprintf(buf + strlen(buf), len - strlen(buf), "enable[%d]=%d&", iCnt, resp->area[iCnt].enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "left[%d]=%d&", iCnt, resp->area[iCnt].left);
        snprintf(buf + strlen(buf), len - strlen(buf), "top[%d]=%d&", iCnt, resp->area[iCnt].top);
        snprintf(buf + strlen(buf), len - strlen(buf), "right[%d]=%d&", iCnt, resp->area[iCnt].right);
        snprintf(buf + strlen(buf), len - strlen(buf), "bottom[%d]=%d&", iCnt, resp->area[iCnt].bottom);
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_white_balance_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, j = 0, chanid = -1;
    char sValue[256] = {0}, tmp[64] = {0};

    struct white_balance_schedule *schedule = (struct white_balance_schedule *)param;
    memset(schedule, 0x0, sizeof(struct white_balance_schedule));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chanid = atoi(sValue);
    }
    if (chanid < 0 || chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    schedule->chanid = atoi(sValue);

    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].start_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].start_time), "%s", sValue);
            }

            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].end_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].end_time), "%s", sValue);
            }

            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule->schedule_day[i].schedule_item[j].action_type = atoi(sValue);
            }
        }
    }

    *datalen = sizeof(struct white_balance_schedule);
    sdkp2p_send_msg(conf, conf->req, schedule, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}


static void req_camera_get_white_balance_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_white_balance_sche(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0, j = 0;
    struct white_balance_schedule *sche = (struct white_balance_schedule *)param;

    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            if (len - strlen(buf) < 300) {
                break;
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "start_time[%d][%d]=%s&", i, j,
                     sche->schedule_day[i].schedule_item[j].start_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "end_time[%d][%d]=%s&", i, j,
                     sche->schedule_day[i].schedule_item[j].end_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "action_type[%d][%d]=%d&", i, j,
                     sche->schedule_day[i].schedule_item[j].action_type);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_image_exposure_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, j = 0, chanid = -1;
    char sValue[256] = {0}, tmp[64] = {0};

    struct exposure_schedule_batch *schedule = (struct exposure_schedule_batch *)param;
    memset(schedule, 0x0, sizeof(struct exposure_schedule_batch));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chanid = atoi(sValue);
    }
    if (chanid < 0 || chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        schedule->chanid[i] = -1;
    }

    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].start_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].start_time), "%s", sValue);
            }

            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].end_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].end_time), "%s", sValue);
            }

            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule->schedule_day[i].schedule_item[j].action_type = atoi(sValue);
            }

            snprintf(tmp, sizeof(tmp), "exposure_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule->schedule_day[i].schedule_item[j].exposureTime = atoi(sValue);
            }

            snprintf(tmp, sizeof(tmp), "gain_level[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule->schedule_day[i].schedule_item[j].gainLevel = atoi(sValue);
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        for (i = 0; i < MAX_CAMERA && i < strlen(sValue); i++) {
            if (sValue[i] == '1') {
                schedule->chanid[i] = i;
            }
        }
    } else {
        schedule->chanid[0] = chanid;
    }

    *datalen = sizeof(struct exposure_schedule_batch);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_EXPOSURE_SCHE_BATCH, schedule, *datalen, SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_camera_get_image_exposure_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_image_exposure_sche(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0, j = 0;
    struct exposure_schedule *sche = (struct exposure_schedule *)param;

    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            if (len - strlen(buf) < 300) {
                break;
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "start_time[%d][%d]=%s&", i, j,
                     sche->schedule_day[i].schedule_item[j].start_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "end_time[%d][%d]=%s&", i, j,
                     sche->schedule_day[i].schedule_item[j].end_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "action_type[%d][%d]=%d&", i, j,
                     sche->schedule_day[i].schedule_item[j].action_type);
            snprintf(buf + strlen(buf), len - strlen(buf), "exposure_time[%d][%d]=%d&", i, j,
                     sche->schedule_day[i].schedule_item[j].exposureTime);
            snprintf(buf + strlen(buf), len - strlen(buf), "gain_level[%d][%d]=%d&", i, j,
                     sche->schedule_day[i].schedule_item[j].gainLevel);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_bwh_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, j = 0, chanid = -1;
    char sValue[256] = {0}, tmp[64] = {0};

    struct bwh_schedule *schedule = (struct bwh_schedule *)param;
    memset(schedule, 0x0, sizeof(struct bwh_schedule));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chanid = atoi(sValue);
    }
    if (chanid < 0 || chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    schedule->chanid = atoi(sValue);

    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].start_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].start_time), "%s", sValue);
            }

            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule->schedule_day[i].schedule_item[j].end_time,
                         sizeof(schedule->schedule_day[i].schedule_item[j].end_time), "%s", sValue);
            }

            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule->schedule_day[i].schedule_item[j].action_type = atoi(sValue);
            }
        }
    }

    *datalen = sizeof(struct bwh_schedule);
    sdkp2p_send_msg(conf, conf->req, schedule, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void req_camera_get_bwh_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_bwh_sche(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0, j = 0;
    struct bwh_schedule *sche = (struct bwh_schedule *)param;

    for (i = 0; i < MAX_DAY_NUM_IPC; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; j++) {
            if (len - strlen(buf) < 300) {
                break;
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "start_time[%d][%d]=%s&", i, j,
                     sche->schedule_day[i].schedule_item[j].start_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "end_time[%d][%d]=%s&", i, j,
                     sche->schedule_day[i].schedule_item[j].end_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "action_type[%d][%d]=%d&", i, j,
                     sche->schedule_day[i].schedule_item[j].action_type);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_day_night_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int id = -1, chanid = -1;
    int i;
    char tmp[64] = {0};
    char sValue[256] = {0};
    struct set_image_day_night_str *info = (struct set_image_day_night_str *)param;
    memset(info, 0x0, sizeof(struct set_image_day_night_str));

    if (!sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        id = atoi(sValue);
    }
    if (chanid < 0 || chanid >= MAX_CAMERA
        || id < 0 || id >= MAX_IMAGE_DAY_NIGHT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdk_get_field_int(buf, "type=", (int *)&info->type);
    if (info->type == MS_INVALID_VALUE) {
        info->type = IPC_IMAGE_TYPE_SINGLE;
    } else if (info->type < IPC_IMAGE_TYPE_SINGLE || info->type >= IPC_IMAGE_TYPE_MAX) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    info->chanid = chanid;
    if (info->type == IPC_IMAGE_TYPE_SINGLE) {
        info->imgSingle.image[id].valid = 1;

        if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].enable = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "startHour=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].startHour = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "startMinute=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].startMinute = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "endHour=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].endHour = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "endMinute=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].endMinute = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "exposureLevel=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].exposureLevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "minShutter=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].minShutter = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "maxShutter=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].maxShutter = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "gainLevel=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].gainLevel = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "irCutLatency=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].irCutLatency = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "irCutState=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].irCutState = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "irLedState=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].irLedState = atoi(sValue);
        }
        if (!sdkp2p_get_section_info(buf, "colorMode=", sValue, sizeof(sValue))) {
            info->imgSingle.image[id].colorMode = atoi(sValue);
        }

        if (!sdkp2p_get_section_info(buf, "hasSche=", sValue, sizeof(sValue))) {
            info->imgSingle.hasSche = atoi(sValue);
        }
        for (i = 0; i < IPC_SCHE_TEN_SECTIONS; ++i) {
            snprintf(tmp, sizeof(tmp), "startTime[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(info->imgSingle.dnSche[i].start_time, sizeof(info->imgSingle.dnSche[i].start_time), sValue);
            }

            snprintf(tmp, sizeof(tmp), "endTime[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(info->imgSingle.dnSche[i].end_time, sizeof(info->imgSingle.dnSche[i].end_time), sValue);
            }

            snprintf(tmp, sizeof(tmp), "templateId[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.dnSche[i].action_type = atoi(sValue);
            }
        }
    } else if (info->type == IPC_IMAGE_TYPE_MULTI) {
        if (id >= MAX_IMAGE_DAY_NIGHT_DEFUALT) {
            *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
            return ;
        }
        info->imgMulti.scenes[0].valid[id] = 1;
        snprintf(tmp, sizeof(tmp), "scenes0.minShutter%d=", id);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info->imgMulti.scenes[0].minShutter[id] = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "scenes0.maxShutter%d=", id);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info->imgMulti.scenes[0].maxShutter[id] = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "scenes0.limitGain%d=", id);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info->imgMulti.scenes[0].limitGain[id] = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "scenes0.irCutInterval%d=", id);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info->imgMulti.scenes[0].irCutInterval[id] = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "scenes0.irCutStatus%d=", id);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info->imgMulti.scenes[0].irCutStatus[id] = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "scenes0.irLedStatus%d=", id);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info->imgMulti.scenes[0].irLedStatus[id] = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "scenes0.colorMode%d=", id);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info->imgMulti.scenes[0].colorMode[id] = atoi(sValue);
        }
    }

    *datalen = sizeof(struct set_image_day_night_str);
    sdkp2p_send_msg(conf, conf->req, info, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_set_day_night_info(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);

    *datalen = strlen(buf) + 1;
}

static void req_camera_get_day_night_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_day_night_info(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct set_image_day_night_str *resp = (struct set_image_day_night_str *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "type=%d&", resp->type);
    if (resp->type == IPC_IMAGE_TYPE_SINGLE) {
        for (i = 0; i < MAX_IMAGE_DAY_NIGHT; i++) {
            if (len - strlen(buf) < 300) {
                break;
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "id[%d]=%d&", i, i);
            snprintf(buf + strlen(buf), len - strlen(buf), "enable[%d]=%d&", i, resp->imgSingle.image[i].enable);
            snprintf(buf + strlen(buf), len - strlen(buf), "startHour[%d]=%d&", i, resp->imgSingle.image[i].startHour);
            snprintf(buf + strlen(buf), len - strlen(buf), "startMinute[%d]=%d&", i, resp->imgSingle.image[i].startMinute);
            snprintf(buf + strlen(buf), len - strlen(buf), "endHour[%d]=%d&", i, resp->imgSingle.image[i].endHour);
            snprintf(buf + strlen(buf), len - strlen(buf), "endMinute[%d]=%d&", i, resp->imgSingle.image[i].endMinute);
            snprintf(buf + strlen(buf), len - strlen(buf), "exposureLevel[%d]=%d&", i, resp->imgSingle.image[i].exposureLevel);
            snprintf(buf + strlen(buf), len - strlen(buf), "maxShutter[%d]=%d&", i, resp->imgSingle.image[i].maxShutter);
            snprintf(buf + strlen(buf), len - strlen(buf), "minShutter[%d]=%d&", i, resp->imgSingle.image[i].minShutter);
            snprintf(buf + strlen(buf), len - strlen(buf), "gainLevel[%d]=%d&", i, resp->imgSingle.image[i].gainLevel);
            snprintf(buf + strlen(buf), len - strlen(buf), "irCutLatency[%d]=%d&", i, resp->imgSingle.image[i].irCutLatency);
            snprintf(buf + strlen(buf), len - strlen(buf), "irCutState[%d]=%d&", i, resp->imgSingle.image[i].irCutState);
            snprintf(buf + strlen(buf), len - strlen(buf), "irLedState[%d]=%d&", i, resp->imgSingle.image[i].irLedState);
            snprintf(buf + strlen(buf), len - strlen(buf), "colorMode[%d]=%d&", i, resp->imgSingle.image[i].colorMode);
        }

        snprintf(buf + strlen(buf), len - strlen(buf), "hasSche=%d&", resp->imgSingle.hasSche);
        for (i = 0; i < IPC_SCHE_TEN_SECTIONS; ++i) {
            if (resp->imgSingle.dnSche[i].start_time[0] == '\0') {
                continue;
            }

            snprintf(buf + strlen(buf), len - strlen(buf), "startTime[%d]=%s&", i, resp->imgSingle.dnSche[i].start_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "endTime[%d]=%s&", i, resp->imgSingle.dnSche[i].end_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "templateId[%d]=%d&", i, resp->imgSingle.dnSche[i].action_type);
        }

        snprintf(buf + strlen(buf), len - strlen(buf), "fullcolorSupport=%d&", resp->imgSingle.fullcolorSupport);
    } else if (resp->type == IPC_IMAGE_TYPE_MULTI) {
        for (i = 0; i < MAX_IMAGE_DAY_NIGHT_DEFUALT; ++i) {
            snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.minShutter%d=%d&", i, resp->imgMulti.scenes[0].minShutter[i]);
            snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.maxShutter%d=%d&", i, resp->imgMulti.scenes[0].maxShutter[i]);
            snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.limitGain%d=%d&", i, resp->imgMulti.scenes[0].limitGain[i]);
            snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.irCutInterval%d=%d&", i, resp->imgMulti.scenes[0].irCutInterval[i]);
            snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.irCutStatus%d=%d&", i, resp->imgMulti.scenes[0].irCutStatus[i]);
            snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.irLedStatus%d=%d&", i, resp->imgMulti.scenes[0].irLedStatus[i]);
            snprintf(buf + strlen(buf), len - strlen(buf), "scenes0.colorMode%d=%d&", i, resp->imgMulti.scenes[0].colorMode[i]);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_day_night_info_batch(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    int i = 0, chanid = -1;
    char sValue[256] = {0}, tmp[64] = {0};
    struct set_image_day_night_str *info = (struct set_image_day_night_str *)param;
    memset(info, 0x0, sizeof(struct set_image_day_night_str));

    if (!sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        chanid = atoi(sValue);
    }
    if (chanid < 0 || chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdk_get_field_int(buf, "type=", (int *)&info->type);
    if (info->type == MS_INVALID_VALUE) {
        info->type = IPC_IMAGE_TYPE_SINGLE;
    } else if (info->type < IPC_IMAGE_TYPE_SINGLE || info->type >= IPC_IMAGE_TYPE_MAX) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    info->chanid = chanid;
    if (info->type == IPC_IMAGE_TYPE_SINGLE) {
        for (i = 0; i < MAX_IMAGE_DAY_NIGHT; i++) {
            info->imgSingle.image[i].valid = 1;

            snprintf(tmp, sizeof(tmp), "enable[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].enable = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "startHour[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].startHour = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "startMinute[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].startMinute = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "endHour[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].endHour = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "endMinute[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].endMinute = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "exposureLevel[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].exposureLevel = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "minShutter[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].minShutter = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "maxShutter[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].maxShutter = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "gainLevel[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].gainLevel = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "irCutLatency[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].irCutLatency = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "irCutState[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].irCutState = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "irLedState[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].irLedState = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "colorMode[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.image[i].colorMode = atoi(sValue);
            }
        }
        
        if (!sdkp2p_get_section_info(buf, "hasSche=", sValue, sizeof(sValue))) {
            info->imgSingle.hasSche = atoi(sValue);
        }
        for (i = 0; i < IPC_SCHE_TEN_SECTIONS; ++i) {
            snprintf(tmp, sizeof(tmp), "startTime[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(info->imgSingle.dnSche[i].start_time, sizeof(info->imgSingle.dnSche[i].start_time), sValue);
            }

            snprintf(tmp, sizeof(tmp), "endTime[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(info->imgSingle.dnSche[i].end_time, sizeof(info->imgSingle.dnSche[i].end_time), sValue);
            }

            snprintf(tmp, sizeof(tmp), "templateId[%d]=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgSingle.dnSche[i].action_type = atoi(sValue);
            }
        }
    } else if (info->type == IPC_IMAGE_TYPE_MULTI) {
        for (i = 0; i < MAX_IMAGE_DAY_NIGHT_DEFUALT; ++i) {
            info->imgMulti.scenes[0].valid[i] = 1;
            snprintf(tmp, sizeof(tmp), "scenes0.minShutter%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgMulti.scenes[0].minShutter[i] = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "scenes0.maxShutter%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgMulti.scenes[0].maxShutter[i] = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "scenes0.limitGain%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgMulti.scenes[0].limitGain[i] = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "scenes0.irCutInterval%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgMulti.scenes[0].irCutInterval[i] = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "scenes0.irCutStatus%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgMulti.scenes[0].irCutStatus[i] = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "scenes0.irLedStatus%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgMulti.scenes[0].irLedStatus[i] = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "scenes0.colorMode%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                info->imgMulti.scenes[0].colorMode[i] = atoi(sValue);
            }
        }
    }

    *datalen = sizeof(struct set_image_day_night_str);
    sdkp2p_send_msg(conf, conf->req, info, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_set_day_night_info_batch(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_exposure_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, nCamId = -1, cnt = 0;
    char sValue[256] = {0}, tmp[64] = {0};
    struct exposure exposures[MAX_EXPOSURE_NUM];
    memset(exposures, 0, sizeof(struct exposure)*MAX_EXPOSURE_NUM);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        nCamId = atoi(sValue);
    }
    if (nCamId < 0 || nCamId >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }
    for (i = 0; i < cnt && i < MAX_EXPOSURE_NUM; i++) {
        snprintf(tmp, sizeof(tmp), "exposureTime[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            exposures[i].exposureTime = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "gainLevel[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            exposures[i].gainLevel = atoi(sValue);
        }
    }
    write_exposure(SQLITE_FILE_NAME, exposures, nCamId, cnt);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_camera_get_exposure_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, nCamId = -1, cnt = 0;
    char sValue[256] = {0};
    struct exposure exposures[MAX_EXPOSURE_NUM];
    memset(exposures, 0, sizeof(struct exposure)*MAX_EXPOSURE_NUM);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        nCamId = atoi(sValue);
    }
    if (nCamId < 0 || nCamId >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_exposure(SQLITE_FILE_NAME, exposures, nCamId, &cnt);
    for (i = 0; i < cnt && i < MAX_EXPOSURE_NUM; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "exposureTime[%d]=%d&", i,
                 exposures[i].exposureTime);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "gainLevel[%d]=%d&", i,
                 exposures[i].gainLevel);
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_camera_get_anr_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_get_anr_support(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_anr_support *resp = (struct resp_anr_support *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", resp->chn);
    snprintf(buf + strlen(buf), len - strlen(buf), "anr_support=%d&", resp->anr_support);

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_anr_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char sValue[256] = {0};
    long long batch_flag = 0;
    struct req_set_anr_enable_batch info;

    if (sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    info.enable = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "channels=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    info.info.size = 0;
    for (i = 0; i < MAX_CAMERA; i++) {
        if (sValue[i] == '1') {
            info.info.chanid[info.info.size] = i;
            info.info.size++;
            batch_flag |= (long long)1 << i;
        }
    }

    copy_table_column_int(SQLITE_FILE_NAME, DB_TABLE_NAME_CAMERA, DB_CAMERA_KEY_ANR, info.enable, batch_flag,
                          DB_CAMERA_KEY_ID);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_set_anr_enable_batch), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_anr_support(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct resp_set_anr_enable_batch *resp = (struct resp_set_anr_enable_batch *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", resp->enable);
    for (i = 0; i < resp->info.size; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "chnid[%d]=%d&", resp->info.chanid[i], resp->info.res[i]);
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_del_privacy_mask(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct delete_arae_params info;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    info.chan_id = atoi(sValue);
    if (info.chan_id < 0 || info.chan_id >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "area_id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.area_id = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct delete_arae_params), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_del_privacy_mask(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);
    *datalen = strlen(buf) + 1;
    return;
}


static void req_camera_del_image_roi(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct delete_arae_params info;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    info.chan_id = atoi(sValue);
    if (info.chan_id < 0 || info.chan_id >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "area_id=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.area_id = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct delete_arae_params), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_del_image_roi(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);
    *datalen = strlen(buf) + 1;
    return;
}

static void set_ipc_alarm_out(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_ipc_alarm_out_state *res = (struct ms_ipc_alarm_out_state *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->chanid));
    }
    if (!sdkp2p_get_section_info(buf, "r.alarmid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->alarmid));
    }
    if (!sdkp2p_get_section_info(buf, "r.state=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->state));
    }

    *datalen = sizeof(struct ms_ipc_alarm_out_state);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void req_camera_get_ipc_snapshot(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_snapshot(void *param, int size, char *buf, int len, int *datalen)
{
    struct RespSnapshotChatHeader *header = NULL;
    int imageLen = 0;
    char *imageData = NULL;
    int tmp;

    if (size >= sizeof(struct RespSnapshotChatHeader)) {
        header = (struct RespSnapshotChatHeader *)param;
        imageLen = size - sizeof(struct RespSnapshotChatHeader);
        imageData = param + sizeof(struct RespSnapshotChatHeader);
    }

    if (header) {
        snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", header->chnid);
        snprintf(buf + strlen(buf), len - strlen(buf), "result=%d&", header->result);
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "size=%d&", imageLen);
    snprintf(buf + strlen(buf), len - strlen(buf), "%s", "image=");
    tmp = strlen(buf);
    if (imageLen && imageData && ((len - tmp) >= imageLen)) {
        memcpy(buf + tmp, imageData, imageLen);
        *datalen = tmp + imageLen;
    } else {
        *datalen = tmp;
    }
}

static void req_camera_get_auto_track_point(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_auto_track_point(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct ms_personpoint_info *info = (struct ms_personpoint_info *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", info->person_point_cnt);
    snprintf(buf + strlen(buf), len - strlen(buf), "width=%d&", info->width);
    snprintf(buf + strlen(buf), len - strlen(buf), "height=%d&", info->height);
    for (i = 0; i < info->person_point_cnt && i < 10; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "p[%d].id=%d&", i, info->point[i].id);
        snprintf(buf + strlen(buf), len - strlen(buf), "p[%d].type=%d&", i, info->point[i].type);
        snprintf(buf + strlen(buf), len - strlen(buf), "p[%d].startX=%d&", i, info->point[i].startX);
        snprintf(buf + strlen(buf), len - strlen(buf), "p[%d].startY=%d&", i, info->point[i].startY);
        snprintf(buf + strlen(buf), len - strlen(buf), "p[%d].stopX=%d&", i, info->point[i].stopX);
        snprintf(buf + strlen(buf), len - strlen(buf), "p[%d].stopY=%d&", i, info->point[i].stopY);
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ptz_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ptz_support(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_auto_track_setting(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_auto_track_setting(void *param, int size, char *buf, int len, int *datalen)
{
    char tmp[10 * 1024] = {0};
    struct ms_auto_tracking *info = (struct ms_auto_tracking *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", info->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "show=%d&", info->show);
    snprintf(buf + strlen(buf), len - strlen(buf), "sensitivity=%d&", info->sensitivity);
    snprintf(buf + strlen(buf), len - strlen(buf), "time=%d&", info->time);
    snprintf(buf + strlen(buf), len - strlen(buf), "manual_tracking=%d&", info->manual_tracking);
    snprintf(buf + strlen(buf), len - strlen(buf), "zoom_mode=%d&", info->zoom_mode);
    snprintf(buf + strlen(buf), len - strlen(buf), "zoom_number=%d&", info->zoom_number);
    snprintf(buf + strlen(buf), len - strlen(buf), "humanVehicleSupport=%d&", info->system_human_vehicle_support);
    snprintf(buf + strlen(buf), len - strlen(buf), "trackingObject=%d&", info->auto_tracking_object);
    snprintf(buf + strlen(buf), len - strlen(buf), "area=%s&", info->area);
    snprintf(buf + strlen(buf), len - strlen(buf), "reportMotions=%d&", info->reportMotion);
    SMART_SCHEDULE schedule;
    memcpy(schedule.schedule_day, &info->schedule_day, sizeof(struct schedule_day)*MAX_DAY_NUM);
    if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), "e") == 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
    }
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_auto_track_setting(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[512] = {0};
    struct req_auto_tracking_batch info;

    memset(&info, 0x0, sizeof(struct req_auto_tracking_batch));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.info.chanid = atoi(sValue);
    if (info.info.chanid < 0 || info.info.chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        info.info.enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "show=", sValue, sizeof(sValue))) {
        info.info.show = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "sensitivity=", sValue, sizeof(sValue))) {
        info.info.sensitivity = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "time=", sValue, sizeof(sValue))) {
        info.info.time = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "manual_tracking=", sValue, sizeof(sValue))) {
        info.info.manual_tracking = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "zoom_mode=", sValue, sizeof(sValue))) {
        info.info.zoom_mode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "zoom_number=", sValue, sizeof(sValue))) {
        info.info.zoom_number = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "humanVehicleSupport=", sValue, sizeof(sValue))) {
        info.info.system_human_vehicle_support = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "trackingObject=", sValue, sizeof(sValue))) {
        info.info.auto_tracking_object = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "area=", sValue, sizeof(sValue))) {
        snprintf(info.info.area, sizeof(info.info.area), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "reportMotion=", sValue, sizeof(sValue))) {
        info.info.reportMotion = atoi(sValue);
    } else {
        info.info.reportMotion = -1;
    }

    SMART_SCHEDULE schedule;
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "e") == 0) {
        memcpy(info.info.schedule_day, &schedule.schedule_day, sizeof(struct schedule_day)*MAX_DAY_NUM);
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        info.channels = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    info.channels |= (Uint64)1 << info.info.chanid;

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct req_auto_tracking_batch), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_auto_track_setting(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_privacy_mask_edit(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_privary_mask_edit info;

    memset(&info, 0x0, sizeof(struct ms_privary_mask_edit));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chnid = atoi(sValue);
    if (info.chnid < 0 || info.chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "maskid=", sValue, sizeof(sValue))) {
        info.maskid = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ms_privary_mask_edit), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_privacy_mask_edit(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_zoom_state(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_zoom_state(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_digitpos_zoom_state *info = (struct ms_digitpos_zoom_state *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "state=%d&", info->state);
    snprintf(buf + strlen(buf), len - strlen(buf), "zoomNumber=%d&", info->zoomNumber);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_wildcards(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_wildcards(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct ms_lpr_wildcards *info = (struct ms_lpr_wildcards *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "filter=%d&", info->filter);
    snprintf(buf + strlen(buf), len - strlen(buf), "maxPlateCharNum=%d&", info->maxPlateCharNum);
    for (i = 0; i < MAX_LEN_9; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "enable%d=%d&", i, info->enable[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "cr_count%d=%d&", i, info->cr_count[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "format%d=%s&", i, info->format[i]);
    }
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_wildcards(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0;
    char tmp[64] = {0};
    struct ms_lpr_wildcards info;

    memset(&info, 0x0, sizeof(struct ms_lpr_wildcards));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chanid = atoi(sValue);
    if (info.chanid < 0 || info.chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (sdkp2p_get_section_info(buf, "filter=", sValue, sizeof(sValue))) {
        info.filter = atoi(sValue);
    }

    if (sdkp2p_get_section_info(buf, "maxPlateCharNum=", sValue, sizeof(sValue))) {
        info.maxPlateCharNum = atoi(sValue);
    }

    for (i = 0; i < MAX_LEN_9; i++) {
        snprintf(tmp, sizeof(tmp), "enable%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.enable[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "cr_count%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.cr_count[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "format%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(info.format[i], sizeof(info.format[i]), "%s", sValue);
        }
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ms_lpr_wildcards), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipc_wildcards(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_watermark(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_watermark(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_water_mark *info = (struct ms_water_mark *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", info->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "watermarkSupport=%d&", info->watermarkSupport);
    snprintf(buf + strlen(buf), len - strlen(buf), "watermark=%s&", info->pWaterMark);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_watermark(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_water_mark_batch info;

    memset(&info, 0x0, sizeof(struct ms_water_mark_batch));
    if (sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        info.info.enable = atoi(sValue);
    }

    if (sdkp2p_get_section_info(buf, "watermark=", sValue, sizeof(sValue))) {
        snprintf(info.info.pWaterMark, sizeof(info.info.pWaterMark), "%s", sValue);
    }

    if (sdkp2p_get_section_info(buf, "copy=", sValue, sizeof(sValue))) {
        snprintf(info.chanmask, sizeof(info.chanmask), "%s", sValue);
    }

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.info.chanid = atoi(sValue);
        if (info.info.chanid >= 0 && info.info.chanid <= MAX_CAMERA) {
            info.chanmask[info.info.chanid] = '1';
        }
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ms_water_mark_batch), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipc_watermark(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_vca_linecrossing2(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_vca_linecrossing2(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0, j = 0;
    struct ms_linecrossing_info2 *resp = (struct ms_linecrossing_info2 *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sensitivity=%d&", resp->sensitivity);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.width=%d&", resp->width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.height=%d&", resp->height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.lineType=%d&", resp->lineType);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.lineScene=%d&", resp->lineScene);

    for (i = 0; i < MAX_LEN_4; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].enable=%d&", i, resp->line[i].enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].objtype=%d&", i, resp->line[i].objtype);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startX=%d&", i, resp->line[i].startX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startY=%d&", i, resp->line[i].startY);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopX=%d&", i, resp->line[i].stopX);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopY=%d&", i, resp->line[i].stopY);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].direction=%d&", i, resp->line[i].direction);

        if (resp->lineType == REGION_PRESET) {
            for (j = SCENE_PRESET_1; j < MAX_SMART_SCENE; ++j) {
                snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startX[%d]=%d&", i, j, resp->line[i].line[j].startX);
                snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].startY[%d]=%d&", i, j, resp->line[i].line[j].startY);
                snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopX[%d]=%d&", i, j, resp->line[i].line[j].stopX);
                snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].stopY[%d]=%d&", i, j, resp->line[i].line[j].stopY);
                snprintf(buf + strlen(buf), len - strlen(buf), "r.line[%d].direction[%d]=%d&", i, j, resp->line[i].line[j].direction);
            }
        }
    }
    if (resp->lineType == REGION_PRESET) {
        for (i = SCENE_NORMAL; i < MAX_SMART_SCENE; ++i) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r.presetExist[%d]=%d&", i, resp->presetExist[i]);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_vca_linecrossing2(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_linecrossing_info2 *res = (struct ms_linecrossing_info2 *)param;
    char sValue[256] = {0};
    char temp[256] = {0};
    int i = 0, j = 0;
    memset(res, 0x0, sizeof(struct ms_linecrossing_info2));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.sensitivity=", sValue, sizeof(sValue))) {
        res->sensitivity = atoi(sValue);
    }
    
    if (!sdkp2p_get_section_info(buf, "r.lineType=", sValue, sizeof(sValue))) {
        res->lineType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.lineScene=", sValue, sizeof(sValue))) {
        res->lineScene = atoi(sValue);
    }
    
    if (!sdkp2p_get_section_info(buf, "r.copyChn=", sValue, sizeof(sValue))) {
        snprintf(res->copyChn, sizeof(res->copyChn), sValue);
    }

    for (i = 0; i < MAX_LEN_4; i++) {
        snprintf(temp, sizeof(temp), "r.line[%d].enable=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].enable = atoi(sValue);
        }

        snprintf(temp, sizeof(temp), "r.line[%d].objtype=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].objtype = atoi(sValue);
        }

        snprintf(temp, sizeof(temp), "r.line[%d].startX=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].startX = atoi(sValue);
            res->line[i].line[SCENE_NORMAL].startX = atoi(sValue);
        }
        
        snprintf(temp, sizeof(temp), "r.line[%d].startY=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].startY = atoi(sValue);
            res->line[i].line[SCENE_NORMAL].startY = atoi(sValue);
        }
        
        snprintf(temp, sizeof(temp), "r.line[%d].stopX=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].stopX = atoi(sValue);
            res->line[i].line[SCENE_NORMAL].stopX = atoi(sValue);
        }
        
        snprintf(temp, sizeof(temp), "r.line[%d].stopY=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].stopY = atoi(sValue);
            res->line[i].line[SCENE_NORMAL].stopY = atoi(sValue);
        }
        
        snprintf(temp, sizeof(temp), "r.line[%d].direction=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            res->line[i].direction = atoi(sValue);
            res->line[i].line[SCENE_NORMAL].direction = atoi(sValue);
        }
        
        if (res->lineType == REGION_PRESET) {
            for (j = SCENE_PRESET_1; j < MAX_SMART_SCENE; ++j) {
                snprintf(temp, sizeof(temp), "r.line[%d].startX[%d]=", i, j);
                if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
                    res->line[i].line[j].startX = atoi(sValue);
                }
                
                snprintf(temp, sizeof(temp), "r.line[%d].startY[%d]=", i, j);
                if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
                    res->line[i].line[j].startY = atoi(sValue);
                }
                
                snprintf(temp, sizeof(temp), "r.line[%d].stopX[%d]=", i, j);
                if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
                    res->line[i].line[j].stopX = atoi(sValue);
                }
                
                snprintf(temp, sizeof(temp), "r.line[%d].stopY[%d]=", i, j);
                if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
                    res->line[i].line[j].stopY = atoi(sValue);
                }
                
                snprintf(temp, sizeof(temp), "r.line[%d].direction[%d]=", i, j);
                if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
                    res->line[i].line[j].direction = atoi(sValue);
                }
            } 
        } 
    }

    *datalen = sizeof(struct ms_linecrossing_info2);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_set_vca_linecrossing2(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_vca_settings2(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(int));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_get_vca_settings2(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct ms_vca_settings_info2 *resp = (struct ms_vca_settings_info2 *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", resp->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.vcaType=%d&", resp->vca_type);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.width=%d&", resp->width);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.height=%d&", resp->height);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.processFps=%d&", resp->vca_process_fps);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.scenarioMode=%d&", resp->vca_scenario_mode);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.cameraInstallation=%d&", resp->vca_camera_installation);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.analysisType=%d&", resp->vca_analysis_type);
    for (i = 0; i < MAX_LEN_8; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectX%d=%d&", i, resp->minobject_window_rectangle_x[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectY%d=%d&", i, resp->minobject_window_rectangle_y[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectWidth%d=%d&", i, resp->minobject_window_rectangle_width[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.minObjectHeight%d=%d&", i,
                 resp->minobject_window_rectangle_height[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectX%d=%d&", i, resp->maxobject_window_rectangle_x[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectY%d=%d&", i, resp->maxobject_window_rectangle_y[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectWidth%d=%d&", i, resp->maxobject_window_rectangle_width[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.maxObjectHeight%d=%d&", i,
                 resp->maxobject_window_rectangle_height[i]);
    }

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_vca_settings2(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_vca_settings_info2 *res = (struct ms_vca_settings_info2 *)param;
    char sValue[256] = {0};
    int i = 0;
    char tmp[64] = {0};

    memset(res, 0x0, sizeof(struct ms_vca_settings_info));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.vcaType=", sValue, sizeof(sValue))) {
        res->vca_type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.width=", sValue, sizeof(sValue))) {
        res->width = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.height=", sValue, sizeof(sValue))) {
        res->height = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.processFps=", sValue, sizeof(sValue))) {
        res->vca_process_fps = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.scenarioMode=", sValue, sizeof(sValue))) {
        res->vca_scenario_mode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.cameraInstallation=", sValue, sizeof(sValue))) {
        res->vca_camera_installation = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.analysisType=", sValue, sizeof(sValue))) {
        res->vca_analysis_type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.copyChn=", sValue, sizeof(sValue))) {
        snprintf(res->copyChn, sizeof(res->copyChn), sValue);
    }

    for (i = 0; i < MAX_LEN_8; i++) {
        snprintf(tmp, sizeof(tmp), "r.minObjectX%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->minobject_window_rectangle_x[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "r.minObjectY%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->minobject_window_rectangle_y[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "r.minObjectWidth%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->minobject_window_rectangle_width[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "r.minObjectHeight%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->minobject_window_rectangle_height[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "r.maxObjectX%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->maxobject_window_rectangle_x[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "r.maxObjectY%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->maxobject_window_rectangle_y[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "r.maxObjectWidth%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->maxobject_window_rectangle_width[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "r.maxObjectHeight%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            res->maxobject_window_rectangle_height[i] = atoi(sValue);
        }
    }

    *datalen = sizeof(struct ms_vca_settings_info2);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_camera_set_vca_settings2(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_event_stream(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_stream_info info;

    memset(&info, 0x0, sizeof(struct req_stream_info));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chnid = atoi(sValue);
    if (info.chnid < 0 || info.chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "fisheyeType=", sValue, sizeof(sValue))) {
        info.fisheyeType  = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct req_stream_info), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_event_stream(void *param, int size, char *buf, int len, int *datalen)
{
    struct event_stream_info *info = (struct event_stream_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", info->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "framerate=%d&", info->framerate);
    snprintf(buf + strlen(buf), len - strlen(buf), "bitrate=%d&", info->bitrate);
    snprintf(buf + strlen(buf), len - strlen(buf), "iframeinterval=%d&", info->iframeinterval);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_event_stream(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct event_stream_info info;

    memset(&info, 0x0, sizeof(struct event_stream_info));

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chnid  = atoi(sValue);
    if (info.chnid  < 0 || info.chnid  >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        info.enable  = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "framerate=", sValue, sizeof(sValue))) {
        info.framerate  = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "bitrate=", sValue, sizeof(sValue))) {
        info.bitrate  = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "iframeinterval=", sValue, sizeof(sValue))) {
        info.iframeinterval  = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct event_stream_info), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_event_stream(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_manual_tracking_region(char *buf, int len, void *param, int *datalen,
                                                  struct req_conf_str *conf)
{
    char sValue[256] = {0};
    MS_TRK_REGION info;

    memset(&info, 0x0, sizeof(MS_TRK_REGION));

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chanid  = atoi(sValue);
    if (info.chanid  < 0 || info.chanid  >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "manual_tracking=", sValue, sizeof(sValue))) {
        info.manual_tracking  = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "startX=", sValue, sizeof(sValue))) {
        info.startX  = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "startY=", sValue, sizeof(sValue))) {
        info.startY  = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "width=", sValue, sizeof(sValue))) {
        info.width  = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "height=", sValue, sizeof(sValue))) {
        info.height  = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(MS_TRK_REGION), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_manual_tracking_region(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_lpr_night_mode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_lpr_night_mode(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_lpr_night_mode *info = (struct resp_lpr_night_mode *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", info->nightMode);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_autoiris_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_autoiris_status(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_ipc_autoiris_status *info = (struct resp_ipc_autoiris_status *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", info->status);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_audio_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_audio_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct ipc_audio_info *info = (struct ipc_audio_info *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", info->res);
    snprintf(buf + strlen(buf), len - strlen(buf), "support=%d&", info->support);
    snprintf(buf + strlen(buf), len - strlen(buf), "supportMode=%d&", info->supportMode);
    snprintf(buf + strlen(buf), len - strlen(buf), "supportInputMode=%d&", info->supportInputMode);

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->config.chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "audioEnable=%d&", info->config.audioEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "audioMode=%d&", info->config.audioMode);
    snprintf(buf + strlen(buf), len - strlen(buf), "denoise=%d&", info->config.denoise);
    snprintf(buf + strlen(buf), len - strlen(buf), "encoding=%d&", info->config.encoding);
    snprintf(buf + strlen(buf), len - strlen(buf), "sampleRate=%d&", info->config.sampleRate);
    snprintf(buf + strlen(buf), len - strlen(buf), "inputGain=%d&", info->config.inputGain);
    snprintf(buf + strlen(buf), len - strlen(buf), "inputMode=%d&", info->config.inputMode);
    snprintf(buf + strlen(buf), len - strlen(buf), "audioGainControl=%d&", info->config.audioGainControl);
    snprintf(buf + strlen(buf), len - strlen(buf), "outputVolume=%d&", info->config.outputVolume);
    snprintf(buf + strlen(buf), len - strlen(buf), "audioBitrate=%d&", info->config.audioBitrate);

    int i, j, y;
    for (i = 0; i < info->options.encodeNumber; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "audio_codec_%d=%d&", i, info->options.encodeInfo[i].encode);
        for (j = 0; j < info->options.encodeInfo[i].sampleRateNumber; j++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "audio_codec_%d_sample_rate_%d=%d&", info->options.encodeInfo[i].encode,
                     j, info->options.encodeInfo[i].sampleRate[j].sampleRate);
            for (y = 0; y < info->options.encodeInfo[i].sampleRate[j].bitrateNumber; y++) {
                snprintf(buf + strlen(buf), len - strlen(buf), "audio_sample_rate_%d_bit_rate_%d=%d&",
                         info->options.encodeInfo[i].sampleRate[j].sampleRate, y, info->options.encodeInfo[i].sampleRate[j].bitrateValue[y]);
            }
        }
    }
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_audio_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0;
    struct req_ipc_audio_config allInfo;

    memset(&allInfo, 0x0, sizeof(struct req_ipc_audio_config));
    if (!sdkp2p_get_section_info(buf, "channels=", sValue, sizeof(sValue))) {
        for (i = 0; i < MAX_CAMERA; i++) {
            if (sValue[i] == '1') {
                allInfo.channels |= (Uint64)1 << i;
            }
            if (sValue[i] == '\0') {
                break;
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        allInfo.info.chnid = atoi(sValue);
        allInfo.channels |= (Uint64)1 << allInfo.info.chnid;
    }

    if (!sdkp2p_get_section_info(buf, "audioEnable=", sValue, sizeof(sValue))) {
        allInfo.info.audioEnable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "audioMode=", sValue, sizeof(sValue))) {
        allInfo.info.audioMode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "denoise=", sValue, sizeof(sValue))) {
        allInfo.info.denoise = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "encoding=", sValue, sizeof(sValue))) {
        allInfo.info.encoding = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "sampleRate=", sValue, sizeof(sValue))) {
        allInfo.info.sampleRate = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "inputGain=", sValue, sizeof(sValue))) {
        allInfo.info.inputGain = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "inputMode=", sValue, sizeof(sValue))) {
        allInfo.info.inputMode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "audioGainControl=", sValue, sizeof(sValue))) {
        allInfo.info.audioGainControl = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "outputVolume=", sValue, sizeof(sValue))) {
        allInfo.info.outputVolume = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "audioBitrate=", sValue, sizeof(sValue))) {
        allInfo.info.audioBitrate = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&allInfo, sizeof(struct req_ipc_audio_config), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipc_audio_info(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_check_ipc_fisheye_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_req_ipc_info info;

    memset(&info, 0x0, sizeof(struct ms_req_ipc_info));
    if (!sdkp2p_get_section_info(buf, "ddns=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.ddns, sizeof(info.ddns), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "ipaddr=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.ipaddr, sizeof(info.ipaddr), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.username, sizeof(info.username), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "password=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.password, sizeof(info.password), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        info.port = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ms_req_ipc_info), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_check_ipc_fisheye_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_ipc_fisheye_info *info = (struct ms_ipc_fisheye_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "support=%d&", info->support == 1 ? 1 : 0);
    snprintf(buf + strlen(buf), len - strlen(buf), "transfer=%d&", info->transfer);
    snprintf(buf + strlen(buf), len - strlen(buf), "installation=%d&", info->installation);
    snprintf(buf + strlen(buf), len - strlen(buf), "display=%d&", info->display);
    snprintf(buf + strlen(buf), len - strlen(buf), "streamCnt=%d&", info->streamCnt);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_fisheye_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_ipc_set_fisheye_info info;

    memset(&info, 0x0, sizeof(struct req_ipc_set_fisheye_info));
    if (!sdkp2p_get_section_info(buf, "ddns=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.ddns, sizeof(info.ddns), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "ipaddr=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.ipaddr, sizeof(info.ipaddr), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.username, sizeof(info.username), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "password=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.password, sizeof(info.password), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        info.port = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "transfer=", sValue, sizeof(sValue))) {
        info.transfer = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "installation=", sValue, sizeof(sValue))) {
        info.installation = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "display=", sValue, sizeof(sValue))) {
        info.display = atoi(sValue);
    }


    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct req_ipc_set_fisheye_info), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipc_fisheye_info(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_peoplecnt_db_reset(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int groupid = 0;;

    if (!sdkp2p_get_section_info(buf, "groupid=", sValue, sizeof(sValue))) {
        groupid = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&groupid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_peoplecnt_db_reset(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_peoplecnt_liveview_reset(char *buf, int len, void *param, int *datalen,
                                                    struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int groupid = 0;;

    if (!sdkp2p_get_section_info(buf, "groupid=", sValue, sizeof(sValue))) {
        groupid = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&groupid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_peoplecnt_liveview_reset(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_peoplecnt_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char encode[256] = {0};
    PEOPLECNT_SETTING pSets[MAX_PEOPLECNT_GROUP];

    memset(pSets, 0, sizeof(PEOPLECNT_SETTING)*MAX_PEOPLECNT_GROUP);
    read_peoplecnt_settings(SQLITE_FILE_NAME, &pSets[0]);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cnt=%d&", MAX_PEOPLECNT_GROUP);
    for (i = 0; i < MAX_PEOPLECNT_GROUP; i++) {

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "groupid%d=%d&", i, pSets[i].groupid);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable%d=%d&", i, pSets[i].enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "stays%d=%d&", i, pSets[i].stays);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "liveview_auto_reset%d=%d&", i,
                 pSets[i].liveview_auto_reset);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "auto_day%d=%d&", i, pSets[i].auto_day);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "liveview_font_size%d=%d&", i,
                 pSets[i].liveview_font_size);

        get_url_encode(pSets[i].name, strlen(pSets[i].name), encode, sizeof(encode));
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "name%d=%s&", i, encode);

        get_url_encode(pSets[i].auto_day_time, strlen(pSets[i].auto_day_time), encode, sizeof(encode));
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "auto_day_time%d=%s&", i, encode);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels%d=%s&", i,
                 pSets[i].tri_channels);

        get_url_encode(pSets[i].liveview_green_tips, strlen(pSets[i].liveview_green_tips), encode, sizeof(encode));
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "liveview_green_tips%d=%s&", i, encode);

        get_url_encode(pSets[i].liveview_red_tips, strlen(pSets[i].liveview_red_tips), encode, sizeof(encode));
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "liveview_red_tips%d=%s&", i, encode);
    }

    *datalen = strlen(conf->resp) + 1;
}

static void req_camera_set_peoplecnt_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char mask[64] = {0};
    char sValue[256] = {0};
    int i = 0;
    int groupid = 0;
    PEOPLECNT_SETTING pSets[MAX_PEOPLECNT_GROUP];

    memset(pSets, 0, sizeof(PEOPLECNT_SETTING)*MAX_PEOPLECNT_GROUP);
    read_peoplecnt_settings(SQLITE_FILE_NAME, &pSets[0]);
    for (i = 0; i < MAX_PEOPLECNT_GROUP; i++) {
        snprintf(mask, sizeof(mask), "enable%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            pSets[i].enable = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "stays%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            pSets[i].stays = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "liveview_auto_reset%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            pSets[i].liveview_auto_reset = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "auto_day%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            pSets[i].auto_day = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "liveview_font_size%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            pSets[i].liveview_font_size = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "name%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            get_url_decode(sValue);
            snprintf(pSets[i].name, sizeof(pSets[i].name), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "auto_day_time%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            get_url_decode(sValue);
            snprintf(pSets[i].auto_day_time, sizeof(pSets[i].auto_day_time), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "tri_channels%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            snprintf(pSets[i].tri_channels, sizeof(pSets[i].tri_channels), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "liveview_green_tips%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            get_url_decode(sValue);
            snprintf(pSets[i].liveview_green_tips, sizeof(pSets[i].liveview_green_tips), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "liveview_red_tips%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            get_url_decode(sValue);
            snprintf(pSets[i].liveview_red_tips, sizeof(pSets[i].liveview_red_tips), "%s", sValue);
        }
    }
    write_peoplecnt_settings(SQLITE_FILE_NAME, &pSets[0], MAX_PEOPLECNT_GROUP);

    sdkp2p_send_msg(conf, REQUEST_FLAG_UPDATE_PEOPLECNT_SETTING, &groupid, sizeof(groupid), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return;
}

static void req_camera_get_peoplecnt_action(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char *tmp = NULL;
    int tmpLen = 10 * 1024;
    int i = 0, cnt = 0;
    int groupid = 0;
    char sValue[256] = {0};
    PEOPLECNT_EVENT event;
    SMART_SCHEDULE schedule;
    PEOPLECNT_WLED_PARAMS white_params[MAX_REAL_CAMERA];

    if (sdkp2p_get_section_info(buf, "groupid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    groupid = atoi(sValue);
    if (groupid < 0 || groupid >= MAX_PEOPLECNT_GROUP) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    tmp = ms_malloc(tmpLen);
    if (!tmp) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "groupid=%d&", groupid);
    //event
    memset(&event, 0, sizeof(PEOPLECNT_EVENT));
    read_peoplecnt_event(SQLITE_FILE_NAME, &event, groupid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%d&", event.tri_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", event.buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&", event.email_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_pic_enable=%d&",
             event.email_pic_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_pic=%s&",
             event.tri_channels_pic);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&",
             event.ptzaction_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "alarmout_interval=%d&",
             event.alarmout_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "white_interval=%d&",
             event.whiteled_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarms1=%s&",
             event.tri_chnout1_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarms2=%s&",
             event.tri_chnout2_alarms);

    //audio schedule
    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_peoplecnt_audible_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "a") == 0) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
    }

    //email schedule
    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_peoplecnt_email_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "e") == 0) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
    }

    //ptz schedule
    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_peoplecnt_ptz_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "p") == 0) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
    }

    //white schedule
    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_peoplecnt_wled_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "w") == 0) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
    }

    //white params
    read_peoplecnt_wled_params(SQLITE_FILE_NAME, &white_params[0], groupid, &cnt);
    for (i = 0; i < MAX_REAL_CAMERA && i < cnt; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch%d=%d&", i, white_params[i].acto_chn_id);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "mode%d=%d&", i, white_params[i].flash_mode);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "time%d=%d&", i, white_params[i].flash_time);
    }

    ms_free(tmp);
    *datalen = strlen(conf->resp) + 1;
}

static void req_camera_set_peoplecnt_action(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char mask[32] = {0};
    char sValue[256] = {0};
    int groupid = 0;
    int i = 0;
    PEOPLECNT_EVENT event;
    SMART_SCHEDULE schedule;
    PEOPLECNT_WLED_PARAMS white_params[MAX_REAL_CAMERA];

    if (sdkp2p_get_section_info(buf, "groupid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    groupid = atoi(sValue);
    if (groupid < 0 || groupid >= MAX_PEOPLECNT_GROUP) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    //event
    memset(&event, 0, sizeof(PEOPLECNT_EVENT));
    read_peoplecnt_event(SQLITE_FILE_NAME, &event, groupid);
    if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
        event.tri_alarms = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        event.buzzer_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "email_interval=", sValue, sizeof(sValue))) {
        event.email_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "email_pic_enable=", sValue, sizeof(sValue))) {
        event.email_pic_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "tri_channels_pic=", sValue, sizeof(sValue))) {
        snprintf(event.tri_channels_pic, sizeof(event.tri_channels_pic), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        event.ptzaction_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "alarmout_interval=", sValue, sizeof(sValue))) {
        event.alarmout_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "white_interval=", sValue, sizeof(sValue))) {
        event.whiteled_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarms1=", sValue, sizeof(sValue))) {
        snprintf(event.tri_chnout1_alarms, sizeof(event.tri_chnout1_alarms), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarms2=", sValue, sizeof(sValue))) {
        snprintf(event.tri_chnout2_alarms, sizeof(event.tri_chnout2_alarms), "%s", sValue);
    }

    write_peoplecnt_event(SQLITE_FILE_NAME, &event);

    //audio schedule
    if (!sdkp2p_get_schedule_by_buff(&schedule, buf, "a")) {
        write_peoplecnt_audible_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    }

    //email schedule
    if (!sdkp2p_get_schedule_by_buff(&schedule, buf, "e")) {
        write_peoplecnt_email_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    }

    //ptz schedule
    if (!sdkp2p_get_schedule_by_buff(&schedule, buf, "p")) {
        write_peoplecnt_ptz_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    }

    //white schedule
    if (!sdkp2p_get_schedule_by_buff(&schedule, buf, "w")) {
        write_peoplecnt_wled_schedule(SQLITE_FILE_NAME, &schedule, groupid);
    }

    //white params
    int flag = 0;
    memset(&white_params[0], 0, sizeof(PEOPLECNT_WLED_PARAMS)*MAX_REAL_CAMERA);
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        white_params[i].groupid = groupid;

        snprintf(mask, sizeof(mask), "ch%d=", i);
        if (sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            break;
        }
        white_params[i].acto_chn_id = atoi(sValue);

        snprintf(mask, sizeof(mask), "mode%d=", i);
        if (sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            break;
        }
        white_params[i].flash_mode = atoi(sValue);

        snprintf(mask, sizeof(mask), "time%d=", i);
        if (sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            break;
        }
        white_params[i].flash_time = atoi(sValue);

        flag = 1;
    }
    if (flag) {
        write_peoplecnt_wled_params_all(SQLITE_FILE_NAME, &white_params[0], groupid);
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_UPDATE_PEOPLECNT_SETTING, &groupid, sizeof(groupid), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return;
}

static void req_camera_get_ipc_sysytem_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_sysytem_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct ipc_system_info *info = (struct ipc_system_info *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", info->res);
    snprintf(buf + strlen(buf), len - strlen(buf), "audioSupport=%d&", info->audioSupport);
    snprintf(buf + strlen(buf), len - strlen(buf), "audioSupportMode=%d&", info->audioSupportMode);
    snprintf(buf + strlen(buf), len - strlen(buf), "audioLineInputSupport=%d&", info->audioLineInputSupport);
    snprintf(buf + strlen(buf), len - strlen(buf), "systemSpeakerSupport=%d&", info->systemSpeakerSupport);
    snprintf(buf + strlen(buf), len - strlen(buf), "fwversion=%s&", info->fwversion);
    snprintf(buf + strlen(buf), len - strlen(buf), "productmodel=%s&", info->productmodel);
    snprintf(buf + strlen(buf), len - strlen(buf), "chipinfo=%s&", info->chipinfo);
    snprintf(buf + strlen(buf), len - strlen(buf), "autoTrackingSupport=%d&", info->system_auto_tracking_support);

    *datalen = strlen(buf) + 1;
}

static void req_camera_set_peoplecnt_cache_db(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_peoplecnt_cache_db(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_frame_resolution(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_frame_resolution(void *param, int size, char *buf, int len, int *datalen)
{
    snprintf(buf + strlen(buf), len - strlen(buf), "len=%d&", size);
    snprintf(buf + strlen(buf), len - strlen(buf), "data=%s&", (char *)param);
    *datalen = strlen(buf) + 1;
    return ;
}

static void req_camera_get_pcnt_data(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0, j = 0, cnt = 0;
    SEARCH_PEOPLECNT_LOGS search;
    PEOPLECNT_LOGS *resp = NULL;

    memset(&search, 0, sizeof(SEARCH_PEOPLECNT_LOGS));
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        search.type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "mask=", sValue, sizeof(sValue))) {
        search.id = (Uint64)sdkp2p_get_chnmask_to_atoll(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "pdstart=", sValue, sizeof(sValue))) {
        search.pdstart = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "pdend=", sValue, sizeof(sValue))) {
        search.pdend = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        search.pdstart = ms_get_current_date_cnt_ex(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        search.pdend = ms_get_current_date_cnt_ex(sValue);
    }

    read_peoplecnt_logs(SQLITE_PEOPLE_NAME, &resp, &cnt, &search);
    if (resp) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cnt=%d&", cnt);
        for (i = 0; i < cnt; i++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "g%d=%d&", i, resp[i].groupid);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "c%d=%d&", i, resp[i].chnid);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "d%d=%d&", i, resp[i].date);
            for (j = 0; j < 24; j++) {
                snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "i%d_%d=%d&", i, j, resp[i].pcntin[j]);
                snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "o%d_%d=%d&", i, j, resp[i].pcntout[j]);
            }
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "&");
        release_peoplecnt_log(&resp);
    } else {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cnt=%d&", 0);
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_camera_set_3d_ptz_ctrl(char *buf, int len, void *param, int *datalen,
                                                    struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_3d_ptz_control_info info;

    memset(&info, 0, sizeof(struct ms_3d_ptz_control_info));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "cmd=", sValue, sizeof(sValue))) {
        info.ptz_cmd = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "positionX=", sValue, sizeof(sValue))) {
        info.ptz_3dposition_x = (float)atof(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "positionY=", sValue, sizeof(sValue))) {
        info.ptz_3dposition_y = (float)atof(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "zoom=", sValue, sizeof(sValue))) {
        info.ptz_3dposition_zoom = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ms_3d_ptz_control_info), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_3d_ptz_ctrl(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_manual_tracking_info(char *buf, int len, void *param, int *datalen,
                                                    struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_manual_tracking info;

    memset(&info, 0, sizeof(struct ms_manual_tracking));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "manualTracking=", sValue, sizeof(sValue))) {
        info.manual_tracking = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ms_manual_tracking), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_manual_tracking_info(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}


static void req_camera_get_regional_detec_data(char *buf, int len, void *param, int *datalen,
                                                    struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    chnid = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_regional_detec_data(void *param, int size, char *buf, int len, int *datalen)
{
    int i;
    struct MsIpcRegionalData *info = (struct MsIpcRegionalData *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "result=%d&", info->result);
    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "frameAlarm=%d&", info->frameAlarm);
    snprintf(buf + strlen(buf), len - strlen(buf), "stayAlarm=%d&", info->stayAlarm);
    for (i = 0; i < MAX_LEN_4; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "num%d=%d&", i, info->num[i]);
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "objCount=%d&", info->objCount);
    for (i = 0; i < info->objCount && i < 60; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "id%d=%d&", i, info->obj[i].id);
        snprintf(buf + strlen(buf), len - strlen(buf), "class%d=%d&", i, info->obj[i].classType);
        snprintf(buf + strlen(buf), len - strlen(buf), "left%d=%d&", i, info->obj[i].left);
        snprintf(buf + strlen(buf), len - strlen(buf), "top%d=%d&", i, info->obj[i].top);
        snprintf(buf + strlen(buf), len - strlen(buf), "right%d=%d&", i, info->obj[i].right);
        snprintf(buf + strlen(buf), len - strlen(buf), "bottom%d=%d&", i, info->obj[i].bottom);
        snprintf(buf + strlen(buf), len - strlen(buf), "stayAlarm%d=%d&", i, info->obj[i].stayAlarm);
        snprintf(buf + strlen(buf), len - strlen(buf), "timeExsit%d=%d&", i, info->obj[i].timeExsit);
        snprintf(buf + strlen(buf), len - strlen(buf), "stayTime0_%d=%d&", i, info->obj[i].stayTime[0]);
        snprintf(buf + strlen(buf), len - strlen(buf), "stayTime1_%d=%d&", i, info->obj[i].stayTime[1]);
        snprintf(buf + strlen(buf), len - strlen(buf), "stayTime2_%d=%d&", i, info->obj[i].stayTime[2]);
        snprintf(buf + strlen(buf), len - strlen(buf), "stayTime3_%d=%d&", i, info->obj[i].stayTime[3]);
    }
    
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_people_report(char *buf, int len, void *param, int *datalen,
                                                    struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ReqIpcPeopleReport info;
    
    memset(&info, 0, sizeof(struct ReqIpcPeopleReport));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    info.chnid = atoi(sValue);
    if (info.chnid < 0 || info.chnid >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (!sdkp2p_get_section_info(buf, "mainType=", sValue, sizeof(sValue))) {
        info.mainType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "reportType=", sValue, sizeof(sValue))) {
        info.reportType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "region=", sValue, sizeof(sValue))) {
        info.region = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lengthOfStayType=", sValue, sizeof(sValue))) {
        info.lengthOfStayType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lengthOfStayValue=", sValue, sizeof(sValue))) {
        info.lengthOfStayValue = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.startTime, sizeof(info.startTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lineMask=", sValue, sizeof(sValue))) {
        info.lineMask = atoi(sValue);
    } else {
        info.lineMask = 1; // default line1
    }

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ReqIpcPeopleReport), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_people_report(void *param, int size, char *buf, int len, int *datalen)
{
    char *data = (char *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "size=%d&", size);
    snprintf(buf + strlen(buf), len - strlen(buf), "data=%s&", data);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_report_auto_backup(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    REPORT_AUTO_BACKUP_S report;
    
    memset(&report, 0, sizeof(REPORT_AUTO_BACKUP_S));
    read_peoplecnt_auto_backup(SQLITE_FILE_NAME, &report);
    for (i = 0; i < MAX_REPORT_AUTO_BACKUP_TYPE_COUNT; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "reportType%d=%d&", i, report.settings[i].reportType);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable%d=%d&", i, report.settings[i].enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChannels%d=%s&", i, report.settings[i].triChannels);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triGroups%d=%s&", i, report.settings[i].triGroups);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "stayLengthType%d=%d&", i, report.settings[i].stayLengthType);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "stayLengthValue%d=%d&", i, report.settings[i].stayLengthValue);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "backupDay%d=%d&", i, report.settings[i].backupDay);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "backupTime%d=%s&", i, report.settings[i].backupTime);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "fileFormat%d=%d&", i, report.settings[i].fileFormat);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "timeRange%d=%d&", i, report.settings[i].timeRange);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "backupTo%d=%d&", i, report.settings[i].backupTo);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lineMask%d=%d&", i, report.settings[i].lineMask);

    }
    *datalen = strlen(conf->resp) + 1;
}

static void req_camera_set_report_auto_backup(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[64] = {0};
    int i;
    REPORT_AUTO_BACKUP_S report;
    
    memset(&report, 0, sizeof(REPORT_AUTO_BACKUP_S));
    read_peoplecnt_auto_backup(SQLITE_FILE_NAME, &report);
    for (i = 0; i < MAX_REPORT_AUTO_BACKUP_TYPE_COUNT; i++) {
        snprintf(tmp, sizeof(tmp), "reportType%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].reportType = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "enable%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].enable = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "triChannels%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(report.settings[i].triChannels, sizeof(report.settings[i].triChannels), "%s", sValue);
        }

        snprintf(tmp, sizeof(tmp), "triGroups%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(report.settings[i].triGroups, sizeof(report.settings[i].triGroups), "%s", sValue);
        }

        snprintf(tmp, sizeof(tmp), "stayLengthType%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].stayLengthType = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "stayLengthValue%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].stayLengthValue = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "backupDay%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].backupDay = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "backupTime%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(report.settings[i].backupTime, sizeof(report.settings[i].backupTime), "%s", sValue);
        }

        snprintf(tmp, sizeof(tmp), "fileFormat%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].fileFormat = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "timeRange%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].timeRange = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "backupTo%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].backupTo = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "lineMask%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            report.settings[i].lineMask = atoi(sValue);
        } else {
            report.settings[i].lineMask = 31; // default 0001 1111
        }
    }
    write_peoplecnt_auto_backup(SQLITE_FILE_NAME, &report);

    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_report_auto_backup(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_regional_people(char *buf, int len, void *param, int *datalen,
                                                    struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    chnid = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, (void *)&chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_regional_people(void *param, int size, char *buf, int len, int *datalen)
{
    int i, j;
    int chnid;
    char *tmp = NULL;
    int tmpLen = 10 * 1024;
    char mask[32] = {0};
    SMART_SCHEDULE schedule;
    SMART_EVENT smartevent;
    VCA_PTZ_ACTION_PARAMS ptzParams;
    WHITE_LED_PARAMS_EVTS whiteParams;
    WLED_INFO wInfo;
    int tmpCnt;
    struct MsIpcRegionalPeople *info = (struct MsIpcRegionalPeople *)param;

    chnid = info->chnid;
    snprintf(buf + strlen(buf), len - strlen(buf), "result=%d&", info->result);
    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "sensitivity=%d&", info->sensitivity);
    snprintf(buf + strlen(buf), len - strlen(buf), "minObjX=%d&", info->minObjX);
    snprintf(buf + strlen(buf), len - strlen(buf), "minObjY=%d&", info->minObjY);
    snprintf(buf + strlen(buf), len - strlen(buf), "minObjWidth=%d&", info->minObjWidth);
    snprintf(buf + strlen(buf), len - strlen(buf), "minObjHeight=%d&", info->minObjHeight);
    snprintf(buf + strlen(buf), len - strlen(buf), "maxObjX=%d&", info->maxObjX);
    snprintf(buf + strlen(buf), len - strlen(buf), "maxObjY=%d&", info->maxObjY);
    snprintf(buf + strlen(buf), len - strlen(buf), "maxObjWidth=%d&", info->maxObjWidth);
    snprintf(buf + strlen(buf), len - strlen(buf), "maxObjHeight=%d&", info->maxObjHeight);
    for (i = 0; i < MAX_IPC_PCNT_REGION; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "regionalEnable%d=%d&", i, info->regional[i].enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "polygonX%d=%s&", i, info->regional[i].polygonX);
        snprintf(buf + strlen(buf), len - strlen(buf), "polygonY%d=%s&", i, info->regional[i].polygonY);
        snprintf(buf + strlen(buf), len - strlen(buf), "maxStayEnable%d=%d&", i, info->regional[i].maxStayEnable);
        snprintf(buf + strlen(buf), len - strlen(buf), "maxStayValue%d=%d&", i, info->regional[i].maxStayValue);
        snprintf(buf + strlen(buf), len - strlen(buf), "minStayEnable%d=%d&", i, info->regional[i].minStayEnable);
        snprintf(buf + strlen(buf), len - strlen(buf), "minStayValue%d=%d&", i, info->regional[i].minStayValue);
        snprintf(buf + strlen(buf), len - strlen(buf), "maxStayTimeEnable%d=%d&", i, info->regional[i].maxStayTimeEnable);
        snprintf(buf + strlen(buf), len - strlen(buf), "maxStayTimeValue%d=%d&", i, info->regional[i].maxStayTimeValue);
    }

    for (i = 0; i < MAX_DAY_NUM_IPC; ++i) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; ++j) {
            snprintf(buf + strlen(buf), len - strlen(buf), "startTime[%d][%d]=%s&", i, j,
                info->sche.schedule_day[i].schedule_item[j].start_time);
            snprintf(buf + strlen(buf), len - strlen(buf), "endTime[%d][%d]=%s&", i, j,
                info->sche.schedule_day[i].schedule_item[j].end_time);
        }
    }

    //action
    tmp = ms_malloc(tmpLen);
    if (tmp) {
        for (i = 0; i < MAX_IPC_PCNT_REGION; i++) {
            memset(&smartevent, 0, sizeof(SMART_EVENT));
            read_regional_pcnt_event(SQLITE_FILE_NAME, &smartevent, chnid, i);
        
            //audio
            memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
            read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &schedule, chnid, i);
            snprintf(mask, sizeof(mask), "a%d", i);
            if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, mask) == 0) {
                snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "buzzer_interval%d=%d&", i, smartevent.buzzer_interval);
            snprintf(buf + strlen(buf), len - strlen(buf), "tri_audio_id%d=%d&", i, smartevent.tri_audio_id);
            
            //email
            memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
            read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &schedule, chnid, i);
            snprintf(mask, sizeof(mask), "m%d", i);
            if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, mask) == 0) {
                snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "email_interval%d=%d&", i, smartevent.email_interval);
            snprintf(buf + strlen(buf), len - strlen(buf), "email_pic_enable%d=%d&", i, smartevent.email_pic_enable);

            //ptz
            memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
            read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &schedule, chnid, i);
            snprintf(mask, sizeof(mask), "p%d", i);
            if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, mask) == 0) {
                snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "ptz_interval%d=%d&", i, smartevent.ptzaction_interval);
            memset(&ptzParams, 0, sizeof(VCA_PTZ_ACTION_PARAMS));
            tmpCnt = 0;
            read_ptz_params(SQLITE_FILE_NAME, &ptzParams.ptzActionParams[chnid][0], REGIONAL_PEOPLE_CNT0+i, chnid, &tmpCnt);
            for (j = 0; j < tmpCnt && j < MAX_REAL_CAMERA; j++) {
                if (ptzParams.ptzActionParams[chnid][j].acto_ptz_channel > 0) {
                    snprintf(buf + strlen(buf), len - strlen(buf), "ptzCh%d%d=%d&", i, j,
                             ptzParams.ptzActionParams[chnid][j].acto_ptz_channel);
                    snprintf(buf + strlen(buf), len - strlen(buf), "ptzType%d%d=%d&", i, j,
                             ptzParams.ptzActionParams[chnid][j].acto_ptz_type);
                    snprintf(buf + strlen(buf), len - strlen(buf), "ptzPreset%d%d=%d&", i, j,
                             ptzParams.ptzActionParams[chnid][j].acto_ptz_preset);
                    snprintf(buf + strlen(buf), len - strlen(buf), "ptzPatrol%d%d=%d&", i, j,
                             ptzParams.ptzActionParams[chnid][j].acto_ptz_patrol);
                }
            }
            
            //alarm
            snprintf(buf + strlen(buf), len - strlen(buf), "triAlarms%d=%d&", i, smartevent.tri_alarms);
            snprintf(buf + strlen(buf), len - strlen(buf), "tri_chnout1_alarms%d=%s&", i, smartevent.tri_chnout1_alarms);
            snprintf(buf + strlen(buf), len - strlen(buf), "tri_chnout2_alarms%d=%s&", i, smartevent.tri_chnout2_alarms);
            snprintf(buf + strlen(buf), len - strlen(buf), "alarmout_interval%d=%d&", i, smartevent.alarmout_interval);
            
            
            //white
            memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
            read_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &schedule, chnid, i);
            snprintf(mask, sizeof(mask), "p%d", i);
            if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, mask) == 0) {
                snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "white_interval%d=%d&", i, smartevent.whiteled_interval);
            wInfo.chnid = chnid;
            if (i == 0) {
                snprintf(wInfo.pDbTable, sizeof(wInfo.pDbTable), REGIONAL_PCNT_WLED_PARAMS);
            } else if (i == 1) {
                snprintf(wInfo.pDbTable, sizeof(wInfo.pDbTable), REGIONAL_PCNT1_WLED_PARAMS);
            } else if (i == 2) {
                snprintf(wInfo.pDbTable, sizeof(wInfo.pDbTable), REGIONAL_PCNT2_WLED_PARAMS);
            } else if (i == 3) {
                snprintf(wInfo.pDbTable, sizeof(wInfo.pDbTable), REGIONAL_PCNT3_WLED_PARAMS);
            }
            memset(&whiteParams, 0, sizeof(WHITE_LED_PARAMS_EVTS));
            tmpCnt = 0;
            read_whiteled_params(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnid][0], &wInfo, &tmpCnt);
            for (j = 0; j < MAX_REAL_CAMERA && j < tmpCnt; j++) {
                snprintf(buf + strlen(buf), len - strlen(buf), "whiteCh%d%d=%d&", i, j,
                         whiteParams.ledPrms[chnid][j].acto_chn_id);
                snprintf(buf + strlen(buf), len - strlen(buf), "whiteMode%d%d=%d&", i, j, 
                         whiteParams.ledPrms[chnid][j].flash_mode);
                snprintf(buf + strlen(buf), len - strlen(buf), "whiteTime%d%d=%d&", i, j,
                         whiteParams.ledPrms[chnid][j].flash_time);
            }
            
            //others
            snprintf(buf + strlen(buf), len - strlen(buf), "triChannels%d=%s&", i, smartevent.tri_channels_ex);
            snprintf(buf + strlen(buf), len - strlen(buf), "tri_channels_pic%d=%s&", i, smartevent.tri_channels_pic);
        }

        ms_free(tmp);
    }
    
    *datalen = strlen(buf) + 1;
}

SDK_ERROR_CODE get_regional_pcnt_str(const char *buff, struct MsIpcRegionalPeople *info)
{
    char sValue[256] = {0};
    char tmp[64] = {0};
    int chnid = -1;
    int i, j;
    int flag;
    SMART_SCHEDULE schedule;
    SMART_EVENT smartevent;
    VCA_PTZ_ACTION_PARAMS ptzParams;
    WHITE_LED_PARAMS_EVTS whiteParams;
    
    memset(info, 0, sizeof(struct MsIpcRegionalPeople));
    if (!sdkp2p_get_section_info(buff, "chnid=", sValue, sizeof(sValue))) {
        info->chnid = atoi(sValue);
        chnid = info->chnid;
    }
    if (chnid < 0 || chnid >= MAX_REAL_CAMERA) {
        return PARAM_ERROR;
    }
    if (!sdkp2p_get_section_info(buff, "sensitivity=", sValue, sizeof(sValue))) {
        info->sensitivity = atoi(sValue);
    } else {
        info->sensitivity = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "minObjX=", sValue, sizeof(sValue))) {
        info->minObjX = atoi(sValue);
    } else {
        info->minObjX = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "minObjY=", sValue, sizeof(sValue))) {
        info->minObjY = atoi(sValue);
    } else {
        info->minObjY = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "minObjWidth=", sValue, sizeof(sValue))) {
        info->minObjWidth = atoi(sValue);
    } else {
        info->minObjWidth = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "minObjHeight=", sValue, sizeof(sValue))) {
        info->minObjHeight = atoi(sValue);
    } else {
        info->minObjHeight = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "maxObjX=", sValue, sizeof(sValue))) {
        info->maxObjX = atoi(sValue);
    } else {
        info->maxObjX = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "maxObjY=", sValue, sizeof(sValue))) {
        info->maxObjY = atoi(sValue);
    } else {
        info->maxObjY = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "maxObjWidth=", sValue, sizeof(sValue))) {
        info->maxObjWidth = atoi(sValue);
    } else {
        info->maxObjWidth = MS_INVALID_VALUE;
    }
    
    if (!sdkp2p_get_section_info(buff, "maxObjHeight=", sValue, sizeof(sValue))) {
        info->maxObjHeight = atoi(sValue);
    } else {
        info->maxObjHeight = MS_INVALID_VALUE;
    }

    for (i = 0; i < MAX_IPC_PCNT_REGION; i++) {
        snprintf(tmp, sizeof(tmp), "regionalEnable%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->regional[i].enable = atoi(sValue);
        } else {
            info->regional[i].enable = MS_INVALID_VALUE;
        }
        snprintf(tmp, sizeof(tmp), "polygonX%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(info->regional[i].polygonX, sizeof(info->regional[i].polygonX), "%s", sValue);
        } else {
            snprintf(info->regional[i].polygonX, sizeof(info->regional[i].polygonX), "%s", MS_INVALID_STRING);
        }
        snprintf(tmp, sizeof(tmp), "polygonY%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(info->regional[i].polygonY, sizeof(info->regional[i].polygonY), "%s", sValue);
        } else {
            snprintf(info->regional[i].polygonY, sizeof(info->regional[i].polygonY), "%s", MS_INVALID_STRING);
        }
        snprintf(tmp, sizeof(tmp), "maxStayEnable%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->regional[i].maxStayEnable = atoi(sValue);
        } else {
            info->regional[i].maxStayEnable = MS_INVALID_VALUE;
        }
        snprintf(tmp, sizeof(tmp), "maxStayValue%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->regional[i].maxStayValue = atoi(sValue);
        } else {
            info->regional[i].maxStayValue = MS_INVALID_VALUE;
        }
        snprintf(tmp, sizeof(tmp), "minStayEnable%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->regional[i].minStayEnable = atoi(sValue);
        } else {
            info->regional[i].minStayEnable = MS_INVALID_VALUE;
        }
        snprintf(tmp, sizeof(tmp), "minStayValue%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->regional[i].minStayValue = atoi(sValue);
        } else {
            info->regional[i].minStayValue = MS_INVALID_VALUE;
        }
        snprintf(tmp, sizeof(tmp), "maxStayTimeEnable%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->regional[i].maxStayTimeEnable = atoi(sValue);
        } else {
            info->regional[i].maxStayTimeEnable = MS_INVALID_VALUE;
        }
        snprintf(tmp, sizeof(tmp), "maxStayTimeValue%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            info->regional[i].maxStayTimeValue = atoi(sValue);
        } else {
            info->regional[i].maxStayTimeValue = MS_INVALID_VALUE;
        }
    }

    for (i = 0; i < MAX_DAY_NUM_IPC; ++i) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY_IPC; ++j) {
            snprintf(tmp, sizeof(tmp) - 1, "startTime[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                snprintf(info->sche.schedule_day[i].schedule_item[j].start_time, 
                    sizeof(info->sche.schedule_day[i].schedule_item[j].start_time), sValue);
            }
            snprintf(tmp, sizeof(tmp) - 1, "endTime[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                snprintf(info->sche.schedule_day[i].schedule_item[j].end_time, 
                    sizeof(info->sche.schedule_day[i].schedule_item[j].end_time), sValue);
            }
        }
    }

    //action
    for (i = 0; i < MAX_IPC_PCNT_REGION; i++) {
        memset(&smartevent, 0, sizeof(SMART_EVENT));
        read_regional_pcnt_event(SQLITE_FILE_NAME, &smartevent, chnid, i);
        
        //audio
        snprintf(tmp, sizeof(tmp), "a%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buff, tmp) == 0) {
            write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &schedule, chnid, i);
        }
        snprintf(tmp, sizeof(tmp), "buzzer_interval%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.buzzer_interval = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "tri_audio_id%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.tri_audio_id = atoi(sValue);
        }
        
        //email
        snprintf(tmp, sizeof(tmp), "m%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buff, tmp) == 0) {
            write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &schedule, chnid, i);
        }
        snprintf(tmp, sizeof(tmp), "email_interval%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.email_interval = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "email_pic_enable%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.email_pic_enable = atoi(sValue);
        }
        
        //ptz
        snprintf(tmp, sizeof(tmp), "p%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buff, tmp) == 0) {
            write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &schedule, chnid, i);
        }
        snprintf(tmp, sizeof(tmp), "ptz_interval%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.ptzaction_interval = atoi(sValue);
        }
        flag = 0;
        memset(&ptzParams, 0, sizeof(VCA_PTZ_ACTION_PARAMS));
        ptzParams.ptzActionParams[chnid][0].chn_id = chnid;
        for (j = 0; j < MAX_REAL_CAMERA; j++) {
            snprintf(tmp, sizeof(tmp), "ptzCh%d%d=", i, j);
            if (sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                break;
            }
            ptzParams.ptzActionParams[chnid][j].acto_ptz_channel = atoi(sValue);
            snprintf(tmp, sizeof(tmp), "ptzType%d%d=", i, j);
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                ptzParams.ptzActionParams[chnid][j].acto_ptz_type = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "ptzPreset%d%d=", i, j);
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                ptzParams.ptzActionParams[chnid][j].acto_ptz_preset = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "ptzPatrol%d%d=", i, j);
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                ptzParams.ptzActionParams[chnid][j].acto_ptz_patrol = atoi(sValue);
            }
            flag = 1;
        }
        if (flag) {
            write_ptz_params_all(SQLITE_FILE_NAME, &ptzParams.ptzActionParams[chnid][0], REGIONAL_PEOPLE_CNT0+i, chnid);
        }

        //alarm
        snprintf(tmp, sizeof(tmp), "triAlarms%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.tri_alarms = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "tri_chnout1_alarms%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(smartevent.tri_chnout1_alarms, sizeof(smartevent.tri_chnout1_alarms), "%s", sValue);
        }
        snprintf(tmp, sizeof(tmp), "tri_chnout2_alarms%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(smartevent.tri_chnout2_alarms, sizeof(smartevent.tri_chnout2_alarms), "%s", sValue);
        }
        snprintf(tmp, sizeof(tmp), "alarmout_interval%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.alarmout_interval = atoi(sValue);
        }
        
        //white
        snprintf(tmp, sizeof(tmp), "w%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buff, tmp) == 0) {
            write_regional_pcnt_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &schedule, chnid, i);
        }
        snprintf(tmp, sizeof(tmp), "white_interval%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            smartevent.whiteled_interval = atoi(sValue);
        }

        flag = 0;
        memset(&whiteParams, 0, sizeof(WHITE_LED_PARAMS_EVTS));
        for (j = 0; j < MAX_REAL_CAMERA; j++) {
            snprintf(tmp, sizeof(tmp), "whiteCh%d%d=", i, j);
            if (sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                break;
            }
            whiteParams.ledPrms[chnid][j].acto_chn_id = atoi(sValue);
            snprintf(tmp, sizeof(tmp), "whiteMode%d%d=", i, j);
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                whiteParams.ledPrms[chnid][j].flash_mode = atoi(sValue);
            }
            snprintf(tmp, sizeof(tmp), "whiteTime%d%d=", i, j);
            if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
                whiteParams.ledPrms[chnid][j].flash_time = atoi(sValue);
            }
            
            whiteParams.ledPrms[chnid][j].chnid = chnid;
            flag = 1;
        }
        if (flag) {
            if (i == 0) {
                write_whiteled_params_all(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnid][0], REGIONAL_PCNT_WLED_PARAMS, chnid);
            } else if (i == 1) {
                write_whiteled_params_all(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnid][0], REGIONAL_PCNT1_WLED_PARAMS, chnid);
            } else if (i == 2) {
                write_whiteled_params_all(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnid][0], REGIONAL_PCNT2_WLED_PARAMS, chnid);
            } else if (i == 3) {
                write_whiteled_params_all(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnid][0], REGIONAL_PCNT3_WLED_PARAMS, chnid);
            }
        }

        //others
        snprintf(tmp, sizeof(tmp), "triChannels%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(smartevent.tri_channels_ex, sizeof(smartevent.tri_channels_ex), "%s", sValue);
        }
        snprintf(tmp, sizeof(tmp), "tri_channels_pic%d=", i);
        if (!sdkp2p_get_section_info(buff, tmp, sValue, sizeof(sValue))) {
            snprintf(smartevent.tri_channels_pic, sizeof(smartevent.tri_channels_pic), "%s", sValue);
        }
        write_regional_pcnt_event(SQLITE_FILE_NAME, &smartevent, i);
    }
    return REQUEST_SUCCESS;
}

static void req_camera_set_ipc_regional_people(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct MsIpcRegionalPeople info;

    if (get_regional_pcnt_str(buf, &info) != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct MsIpcRegionalPeople), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipc_regional_people(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_face_config(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnId;

    if (sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    chnId = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, (void *)&chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_face_config(void *param, int size, char *buf, int len, int *datalen)
{
    int action = 1;

    get_camera_face_config_json(param, size, buf, len, action);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_face_config(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    SDK_ERROR_CODE ret;
    MS_FACE_CONFIG info;

    ret = get_camera_face_config_str(buf, &info);
    if (ret != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, ret, NULL);
        return ;
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_FACE_EVENT, &info.chnId, sizeof(int), SDKP2P_NOT_CALLBACK); 
    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(MS_FACE_CONFIG), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_face_config(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    *datalen = sdkp2p_json_resp(buf, len, REQUEST_SUCCESS, &res);
}

static void req_camera_get_face_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnId;

    if (sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    chnId = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, (void *)&chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_face_support(void *param, int size, char *buf, int len, int *datalen)
{
    get_face_support_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ai_connect_cnt(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ai_connect_cnt(void *param, int size, char *buf, int len, int *datalen)
{
    get_ai_cam_cnt_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_custom_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;
    char *p = NULL;
    char sValue[256] = {0};
    const char *url = "&url=";
    struct req_ipc_custom_params info;

    if (sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    chnId = atoi(sValue);
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    p = strstr(buf, url);
    if (!p) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    p += strlen(url);

    memset(&info, 0, sizeof(info));
    info.chnid = chnId;
    snprintf(info.url, sizeof(info.url), "%s", p);
    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(info), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_custom_param(void *param, int size, char *buf, int len, int *datalen)
{
    get_ipc_custom_param_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_image_enhancement_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    chnId = atoi(sValue);
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_image_enhancement_sche(void *param, int size, char *buf, int len, int *datalen)
{
    get_image_enhancement_info_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_vca_base_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    chnId = atoi(sValue);
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_vca_base_info(void *param, int size, char *buf, int len, int *datalen)
{
    get_vca_base_info_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_heatmap_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    chnId = atoi(sValue);
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_heatmap_support(void *param, int size, char *buf, int len, int *datalen)
{
    get_heatmap_support_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_camera_get_ipc_peoplecnt_linecnt(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    Uint64 chnMask = 0;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "chnMask=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    chnMask = atoi(sValue);
    if (!chnMask) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    
    sdkp2p_send_msg(conf, conf->req, (void *)&chnMask, sizeof(Uint64), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_peoplecnt_linecnt(void *param, int size, char *buf, int len, int *datalen)
{
    UInt64 mask = (UInt64)-1;
    MS_PEOPLECNT_DATA *line = (MS_PEOPLECNT_DATA *)param;
    int cnt = size / sizeof(MS_PEOPLECNT_DATA);
    
    get_ipc_peoplecnt_linecnt_json(line, cnt, mask, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_reboot(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId = 0;
    cJSON *root = NULL;
    cJSON *item = NULL;
    root = cJSON_Parse(buf);
    if (!root) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    
    item = cJSON_GetObjectItem(root, "chnId");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    chnId = item->valueint;
    if (0 < chnId || chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    
    sdkp2p_send_msg(conf, conf->req, (void *)&chnId, sizeof(int), SDKP2P_NEED_CALLBACK);

    cJSON_Delete(root);
}

static void resp_camera_common_set(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    *datalen = sdkp2p_json_resp(buf, len, REQUEST_SUCCESS, &resp);

    return;
}

static void req_camera_set_ipc_reset(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    REQ_IPC_RESET_S reqPrm;
    cJSON *root = NULL;
    cJSON *item = NULL;

    memset(&reqPrm, 0, sizeof(reqPrm));
    root = cJSON_Parse(buf);
    if (!root) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    
    item = cJSON_GetObjectItem(root, "chnId");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    reqPrm.chnId = item->valueint;
    if (0 < reqPrm.chnId || reqPrm.chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }

    item = cJSON_GetObjectItem(root, "options");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    reqPrm.options = item->valueint;
    
    sdkp2p_send_msg(conf, conf->req, (void *)&reqPrm, sizeof(REQ_IPC_RESET_S), SDKP2P_NEED_CALLBACK);

    cJSON_Delete(root);
}

static void req_camera_get_ipc_diagnose(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    REQ_IPC_DIAGNOSE_S req;
    cJSON *root = NULL;
    cJSON *item = NULL;

    memset(&req, 0, sizeof(REQ_IPC_DIAGNOSE_S));
    
    root = cJSON_Parse(buf);
    if (!root) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    
    item = cJSON_GetObjectItem(root, "chnId");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    req.chnId = item->valueint;
    if (req.chnId < 0 || req.chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&req, sizeof(REQ_IPC_DIAGNOSE_S), SDKP2P_NEED_CALLBACK);

    cJSON_Delete(root);
}

static void resp_camera_get_ipc_file(void *param, int size, char *buf, int len, int *datalen)
{
    int ret = -1;
    
    if (!param || size < 0) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        return;
    }

    snprintf(buf, len, (char *)param);
    *datalen = strlen(buf) + 1;

    return;
}

static void req_camera_get_ipc_cfg(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    REQ_IPC_CFG_S req;
    cJSON *root = NULL;
    cJSON *item = NULL;

    memset(&req, 0, sizeof(REQ_IPC_CFG_S));
    
    root = cJSON_Parse(buf);
    if (!root) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    
    item = cJSON_GetObjectItem(root, "chnId");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    req.chnId = item->valueint;
    if (req.chnId < 0 || req.chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }

    item = cJSON_GetObjectItem(root, "pwd");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    snprintf(req.pwd, sizeof(req.pwd), item->valuestring);

    sdkp2p_send_msg(conf, conf->req, (void *)&req, sizeof(REQ_IPC_CFG_S), SDKP2P_NEED_CALLBACK);

    
    cJSON_Delete(root);
}

static void resp_camera_get_ipc_file_data(void *param, int size, char *buf, int len, int *datalen)
{
    int ret = -1;
    
    if (!param || size < 0) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        return;
    }
    
    ret = get_ipc_file_data_json(param, size, buf, len);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        return;
    }

    *datalen = strlen(buf) + 1;

    return;
}

static void req_camera_set_ipc_cfg(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    if (!buf || len <= 0) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    
    sdkp2p_send_msg(conf, conf->req, (void *)buf, len, SDKP2P_NEED_CALLBACK);
}

static void req_camera_get_ipc_log(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    REQ_IPC_LOG_S req;
    cJSON *root = NULL;
    cJSON *item = NULL;

    memset(&req, 0, sizeof(REQ_IPC_LOG_S));
    
    root = cJSON_Parse(buf);
    if (!root) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    
    item = cJSON_GetObjectItem(root, "chnId");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    req.chnId = item->valueint;
    if (req.chnId < 0 || req.chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }

    item = cJSON_GetObjectItem(root, "mainType");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    req.mainType = item->valueint;

    item = cJSON_GetObjectItem(root, "subType");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    req.subType = item->valueint;

    item = cJSON_GetObjectItem(root, "startTime");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    snprintf(req.startTime, sizeof(req.startTime), item->valuestring);

    item = cJSON_GetObjectItem(root, "endTime");
    if (!item) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        cJSON_Delete(root);
        return;
    }
    snprintf(req.endTime, sizeof(req.endTime), item->valuestring);

    sdkp2p_send_msg(conf, conf->req, (void *)&req, sizeof(REQ_IPC_LOG_S), SDKP2P_NEED_CALLBACK);
    
    cJSON_Delete(root);
}

static void req_camera_get_ipc_alarmstatus_batch(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    Uint64 chnMask = 0;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "chnMask=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    chnMask = (Uint64)sdkp2p_get_chnmask_to_atoll(sValue);
    if (!chnMask) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    
    sdkp2p_send_msg(conf, conf->req, &chnMask, sizeof(Uint64), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_get_ipc_alarmstatus_batch(void *param, int size, char *buf, int len, int *datalen)
{
    if (!param || size != sizeof(RESP_IPC_ALARMSTATUS_S)) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    int ret = -1;

    ret = sdk_resp_ipc_alarmstatus_to_json(buf, len, param);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_onlineupgrade_batch(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    REQ_UPGRADE_CAMERA_S req;
    int ret = -1;
    memset(&req, 0, sizeof(REQ_UPGRADE_CAMERA_S));

    ret = sdk_text_to_req_upgrade_camera(&req, buf);
    if (ret) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, ret, NULL);
        return ;
    }
    
    sdkp2p_send_msg(conf, conf->req, &req, sizeof(REQ_UPGRADE_CAMERA_S), SDKP2P_NEED_CALLBACK);
}

static void resp_camera_set_ipc_onlineupgrade_batch(void *param, int size, char *buf, int len, int *datalen)
{
    if (!param || size != sizeof(RESP_UPGRADE_CAMERA_S)) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    int ret = -1;

    ret = sdk_resp_upgrade_camera_to_json(buf, len, param);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }
    *datalen = strlen(buf) + 1;
}

static void req_camera_set_ipc_onlineupgrade_stop_batch(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int stop = 1;

    sdkp2p_send_msg(conf, conf->req, &stop, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, REQUEST_SUCCESS, NULL);
}

static void req_camera_get_ipc_ptz_wiper(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {
        //解析参数失败
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_get_ipc_ptz_wiper(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_IPC_PTZ_WIPER);
    ware_camera_get_ipc_ptz_wiper_out(reqUrl, param, size/sizeof(IPC_PTZ_WIPER_S), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_ptz_wiper(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IPC_PTZ_WIPER_S info;

    if (ware_camera_set_ipc_ptz_wiper_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, conf->req, &param, sizeof(IPC_PTZ_WIPER_S), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_ipc_ptz_wiper(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SET_IPC_PTZ_WIPER);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_get_ipc_ptz_auto_home(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {
        //解析参数失败
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_get_ipc_ptz_auto_home(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_IPC_PTZ_AUTO_HOME);
    ware_camera_get_ipc_ptz_auto_home_out(reqUrl, param, size/sizeof(IPC_PTZ_AUTO_HOME_S), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_ptz_auto_home(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IPC_PTZ_AUTO_HOME_S info;

    if (ware_camera_set_ipc_ptz_auto_home_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, conf->req, &param, sizeof(IPC_PTZ_AUTO_HOME_S), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_ipc_ptz_auto_home(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SET_IPC_PTZ_AUTO_HOME);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_ptz_initial_position(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IPC_PTZ_INITIAL_POSITION_S info;

    if (ware_camera_set_ipc_ptz_initial_position_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, conf->req, &param, sizeof(IPC_PTZ_INITIAL_POSITION_S), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_ipc_ptz_initial_position(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SET_IPC_PTZ_INITIAL_POSITION);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_ptz_config_clear(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IPC_PTZ_INITIAL_POSITION_S info;

    if (ware_camera_set_ipc_ptz_config_clear_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, conf->req, &param, sizeof(IPC_PTZ_CONFIG_CLEAR_S), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_ipc_ptz_config_clear(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SET_IPC_PTZ_CONFIG_CLEAR);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_get_ipc_ptz_limit(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {
        //解析参数失败
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_get_ipc_ptz_limit(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_IPC_PTZ_LIMIT);
    ware_camera_get_ipc_ptz_limit_out(reqUrl, param, size/sizeof(IPC_PTZ_LIMIT_INFO_S), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_ptz_limit(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IPC_PTZ_LIMIT_CONTROL_S info;

    if (ware_camera_set_ipc_ptz_limit_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, conf->req, &param, sizeof(IPC_PTZ_LIMIT_CONTROL_S), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_ipc_ptz_limit(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SET_IPC_PTZ_LIMIT);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_get_ipc_ptz_sche_task(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {
        //解析参数失败
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_get_ipc_ptz_sche_task(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_IPC_PTZ_SCHE_TASK);
    ware_camera_get_ipc_ptz_schedule_task_out(reqUrl, param, size/sizeof(IPC_PTZ_SCHE_TASK_INFO_S), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_ptz_sche_task(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IPC_PTZ_SCHE_TASK_INFO_S info;

    if (ware_camera_set_ipc_ptz_schedule_task_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, conf->req, &param, sizeof(IPC_PTZ_SCHE_TASK_INFO_S), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_ipc_ptz_sche_task(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SET_IPC_PTZ_SCHE_TASK);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_ptz_control_json(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IPC_PTZ_CONTROL_S info;

    if (ware_camera_set_ipc_ptz_control_json_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }
    sdkp2p_send_msg(conf, conf->req, &param, sizeof(IPC_PTZ_CONTROL_S), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_set_ipc_ptz_control_json(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SET_IPC_PTZ_CONTROL_JSON);
    ware_common_int_out(reqUrl, param, size/sizeof(int), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_get_ipc_ptz_panel_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {
        //解析参数失败
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_get_ipc_ptz_panel_status(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_IPC_PTZ_PANEL_STATUS);
    ware_camera_get_ipc_ptz_panel_status_out(reqUrl, param, size/sizeof(IPC_PTZ_PANEL_STATUS_S), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_get_ipc_cap_image(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    if (ware_common_chnid_in(conf->reqUrl, buf, conf->resp, conf->size, &chnId, "chnId")) {
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_camera_get_ipc_cap_image(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_IPC_CAP_IMAGE);
    ware_camera_get_ipc_cap_image_out(reqUrl, param, size, buf, len);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_camera_set_ipc_image_splicedistance(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    IMAGE_SPILCEDISTANCE_S info;

    if (ware_camera_set_ipc_image_splicedistance_in(conf->reqUrl, buf, conf->resp, conf->size, &info)) {
        *(conf->len) = strlen(conf->resp);
        return;
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPC_IMAGE_SPLICEDISTANCE, &info, sizeof(IMAGE_SPILCEDISTANCE_S), SDKP2P_NEED_CALLBACK);

    return;
}

void resp_camera_get_ipc_image_splicedistance(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[128] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE);
    ware_camera_get_ipc_image_splicedistance_out(reqUrl, param, size, buf, len);
    *datalen = strlen(buf) + 1;
    return;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_IPCPARAM, req_camera_get_ipc_param, RESPONSE_FLAG_GET_IPCPARAM, resp_camera_get_ipc_param},//get.camera.ipcparam
    {REQUEST_FLAG_SET_IPCPARAM, req_camera_set_ipc_param, RESPONSE_FLAG_SET_IPCPARAM, resp_camera_set_ipc_param},//set.camera.ipcparam
    {REQUEST_FLAG_GET_IPCLIST, req_camera_get_ipc_list, RESPONSE_FLAG_GET_IPCLIST, resp_camera_get_ipc_list},//get.camera.ipclist
    {REQUEST_FLAG_SET_ADD_IPC_CHANNEL, req_camera_add_ipc_channel, -1, NULL},//add.camera.channel
    {REQUEST_FLAG_SET_MODIFY_IPC_CHANNEL, req_camera_modify_ipc_channel, -1, NULL},//modify.camera.channel
    {REQUEST_FLAG_SET_REMOVE_IPC_CHANNEL, req_camera_remove_ipc_channel, -1, NULL},//remove.camera.channel
    {REQUEST_FLAG_GET_CAMERA_PROTOCOL, req_camera_get_camera_protocol, -1, NULL},//get.camera.protocol
    {REQUEST_FLAG_SEARCH_IPC, req_camera_search_ipc, RESPONSE_FLAG_SEARCH_IPC, resp_camera_search_ipc},//search.camera.chns
    {REQUEST_FLAG_TEST_IP_CONFLICT, req_camera_test_ip_conflict, RESPONSE_FLAG_TEST_IP_CONFLICT, resp_camera_test_ip_conflict},//test.camera.ipconflict
    {REQUEST_FLAG_TEST_IPCCONNECT, req_camera_test_ipc_connect, RESPONSE_FLAG_TEST_IPCCONNECT, resp_camera_test_ipc_connect},//test.camera.ipcconn
    {REQUEST_FLAG_GET_REMOTELIVE_RTSPADDRS, NULL, RESPONSE_FLAG_GET_REMOTELIVE_RTSPADDRS, resp_camera_get_remotelive_rtspaddrs},//get.camera.rtsp
    {REQUEST_FLAG_GET_IMAGEPARAM, req_camera_get_image_param, RESPONSE_FLAG_GET_IMAGEPARAM, resp_camera_get_image_param},//get.camera.imageparam
    {REQUEST_FLAG_SET_CAMERA_IMAGEPARAM, req_camera_set_image_param, -1, NULL},//set.camera.imageparam
    {REQUEST_FLAG_SET_POE_CONN_PASSWORD, req_camera_set_poe_conn_password, -1, NULL},//set.camera.poepasswd
    {REQUEST_FLAG_GET_IPC_COMMON_PARAM, req_camera_get_common_param, RESPONSE_FLAG_GET_IPC_COMMON_PARAM, resp_camera_get_common_param},//get.camera.commomparam //2023
    {REQUEST_FLAG_SET_IPC_COMMON_PARAM, req_camera_set_common_param, RESPONSE_FLAG_SET_IPC_COMMON_PARAM, resp_camera_set_common_param},//set.camera.commomparam //2024
    {REQUEST_FLAG_TRY_TEST_IPCCONNECT, req_camera_try_test_ipcconnect, RESPONSE_FLAG_TRY_TEST_IPCCONNECT, resp_camera_try_test_ipcconnect},//test.try.camera.ipcconn
    {REQUEST_FLAG_GET_IPCIMAGE_ENHANCEMENT, req_camera_get_image_enhancement, RESPONSE_FLAG_GET_IPCIMAGE_ENHANCEMENT, resp_camera_get_image_enhancement},//get.camera.ipcimage.enhance
    {REQUEST_FLAG_SET_IPCIMAGE_ENHANCEMENT, req_camera_set_image_enhancement, RESPONSE_FLAG_SET_IPCIMAGE_ENHANCEMENT, NULL},//set.camera.ipcimage.enhance
    {REQUEST_FLAG_GET_IPCIMAGE_DISPLAY, req_camera_get_image_display, RESPONSE_FLAG_GET_IPCIMAGE_DISPLAY, resp_camera_get_image_display},//get.camera.ipcimage.display
    {REQUEST_FLAG_SET_IPCIMAGE_DISPLAY, req_camera_set_image_display, RESPONSE_FLAG_SET_IPCIMAGE_DISPLAY, NULL},//set.camera.ipcimage.display
    {REQUEST_FLAG_GET_ALL_IPCTYPE, NULL, RESPONSE_FLAG_GET_ALL_IPCTYPE, resp_camera_get_all_ipctype},//get.camera.all.modelinfo
    {REQUEST_FLAG_GET_FISHEYE_MODE, req_camera_get_fisheye_mode, RESPONSE_FLAG_GET_FISHEYE_MODE, resp_camera_get_fisheye_mode},//get.camera.fisheye.mode
    {REQUEST_FLAG_SET_FISHEYE_MODE, req_camera_set_fisheye_mode, RESPONSE_FLAG_SET_FISHEYE_MODE, NULL},//set.camera.fisheye.mode
    {REQUEST_FLAG_OVF_GET_OSD, req_camera_get_image_osd, RESPONSE_FLAG_OVF_GET_OSD, resp_camera_get_image_osd},//get.camera.osd
    {REQUEST_FLAG_OVF_SET_OSD, req_camera_set_image_osd, RESPONSE_FLAG_OVF_SET_OSD, resp_camera_set_image_osd},//set.camera.osd
    {REQUEST_FLAG_GET_IPC_MODEL_TYPE, req_camera_get_ipc_model_type, RESPONSE_FLAG_GET_IPC_MODEL_TYPE, resp_camera_get_ipc_model_type},//get.camera.ipctype
    {REQUEST_FLAG_GET_VCA_REGIONENTRANCE, req_camera_get_vca_region_entrance, RESPONSE_FLAG_GET_VCA_REGIONENTRANCE, resp_camera_get_vca_region_entrance},//get.camera.vca.region.entarnce
    {REQUEST_FLAG_SET_VCA_REGIONENTRANCE, req_camera_set_vca_region_entrance, RESPONSE_FLAG_SET_VCA_REGIONENTRANCE, resp_camera_set_vca_region_entrance},//set.camera.vca.region.entarnce
    {REQUEST_FLAG_GET_VCA_REGIONEXIT, req_camera_get_vca_region_exit, RESPONSE_FLAG_GET_VCA_REGIONEXIT, resp_camera_get_vca_region_exit},//get.camera.vca.region.exiting
    {REQUEST_FLAG_SET_VCA_REGIONEXIT, req_camera_set_vca_region_exit, RESPONSE_FLAG_SET_VCA_REGIONEXIT, resp_camera_set_vca_region_exit},//set.camera.vca.region.exiting
    {REQUEST_FLAG_GET_VCA_ADVANCEDMOTION, req_camera_get_vca_advanced_motion, RESPONSE_FLAG_GET_VCA_ADVANCEDMOTION, resp_camera_get_vca_advanced_motion},//get.camera.vca.advanced.motion
    {REQUEST_FLAG_SET_VCA_ADVANCEDMOTION, req_camera_set_vca_advanced_motion, RESPONSE_FLAG_SET_VCA_ADVANCEDMOTION, resp_camera_set_vca_advanced_motion},//set.camera.vca.advanced.motion
    {REQUEST_FLAG_GET_VCA_TAMPER, req_camera_get_vca_tamper_detection, RESPONSE_FLAG_GET_VCA_TAMPER, resp_camera_get_vca_tamper_detection},//get.camera.vca.tamper.detection
    {REQUEST_FLAG_SET_VCA_TAMPER, req_camera_set_vca_tamper_detection, RESPONSE_FLAG_SET_VCA_TAMPER, resp_camera_set_vca_tamper_detection},//set.camera.vca.tamper.detection
    {REQUEST_FLAG_GET_VCA_LINECROSSING, req_camera_get_vca_line_crossing, RESPONSE_FLAG_GET_VCA_LINECROSSING, resp_camera_get_vca_line_crossing},//get.camera.vca.line.crossing
    {REQUEST_FLAG_SET_VCA_LINECROSSING, req_camera_set_vca_line_crossing, RESPONSE_FLAG_SET_VCA_LINECROSSING, resp_camera_set_vca_line_crossing},//set.camera.vca.line.crossing
    {REQUEST_FLAG_GET_VCA_LOITERING, req_camera_get_vca_loitering, RESPONSE_FLAG_GET_VCA_LOITERING, resp_camera_get_vca_loitering},//get.camera.vca.loitering
    {REQUEST_FLAG_SET_VCA_LOITERING, req_camera_set_vca_loitering, RESPONSE_FLAG_SET_VCA_LOITERING, resp_camera_set_vca_loitering},//set.camera.vca.loitering
    {REQUEST_FLAG_GET_VCA_HUMANDETECTION, req_camera_get_vca_human_detection, RESPONSE_FLAG_GET_VCA_HUMANDETECTION, resp_camera_get_vca_human_detection},//get.camera.vca.human.detection
    {REQUEST_FLAG_SET_VCA_HUMANDETECTION, req_camera_set_vca_human_detection, RESPONSE_FLAG_SET_VCA_HUMANDETECTION, resp_camera_set_vca_human_detection},//set.camera.vca.human.detection
    {REQUEST_FLAG_GET_VCA_PEOPLE_COUNT, req_camera_get_vca_people_count, RESPONSE_FLAG_GET_VCA_PEOPLE_COUNT, resp_camera_get_vca_people_count},//get.camera.vca.people.counting
    {REQUEST_FLAG_SET_VCA_PEOPLE_COUNT, req_camera_set_vca_people_count, RESPONSE_FLAG_SET_VCA_PEOPLE_COUNT, resp_camera_set_vca_people_count},//set.camera.vca.people.counting
    {REQUEST_FLAG_GET_VAC_SETTINGS, req_camera_get_vca_settings, RESPONSE_FLAG_GET_VAC_SETTINGS, resp_camera_get_vca_settings},//get.camera.vca.settings
    {REQUEST_FLAG_SET_VAC_SETTINGS, req_camera_set_vca_settings, RESPONSE_FLAG_SET_VAC_SETTINGS, resp_camera_set_vca_settings},//set.camera.vca.settings
    {REQUEST_FLAG_GET_VCA_LICENSE, req_camera_get_vca_license, RESPONSE_FLAG_GET_VCA_LICENSE, resp_camera_get_vca_license},//get.camera.vca.lisence
    {REQUEST_FLAG_SET_VAC_LISENCE, req_camera_set_vca_license, RESPONSE_FLAG_SET_VAC_LISENCE, resp_camera_set_vca_license},//set.camera.vca.lisence
    {REQUEST_FLAG_SET_VAC_CLEANCOUNT, req_camera_set_vca_clear_count, RESPONSE_FLAG_SET_VAC_CLEANCOUNT, resp_camera_set_vca_clear_count},//set.camera.vca.clearcount
    {REQUEST_FLAG_GET_VAC_SUPPORT, req_camera_get_vca_support, RESPONSE_FLAG_GET_VAC_SUPPORT, resp_camera_get_vca_support},//get.camera.vca.support
    {REQUEST_FLAG_GET_VAC_SUPPORT_ALL, req_camera_get_vca_support_all, RESPONSE_FLAG_GET_VAC_SUPPORT_ALL, resp_camera_get_vca_support_all},//get.camera.vca.support_all
    {REQUEST_FLAG_GET_VAC_PERSONPOINT, req_camera_get_vca_person_point, RESPONSE_FLAG_GET_VAC_PERSONPOINT, resp_camera_get_vca_person_point},//get.camera.vca.person_point
    {REQUEST_FLAG_SEARCH_NVR, req_camera_get_search_nvr, RESPONSE_FLAG_SEARCH_NVR, resp_camera_get_search_nvr},//get.camera.search_nvr
    {REQUEST_FLAG_P2P_UPLOAD_SPEED, req_p2p_upload_speed, RESPONSE_FLAG_P2P_UPLOAD_SPEED, upload_speed_to_str},
    {REQUEST_FLAG_SET_IPCADDR, req_camera_set_ipcaddr, RESPONSE_FLAG_SET_IPCADDR, resp_camera_set_ipcaddr},//set.camera.edit_ipaddr
    {REQUEST_FLAG_GET_CAMERA_CHALLENGE, req_camera_active_ipc, RESPONSE_FLAG_GET_CAMERA_CHALLENGE, resp_camera_active_ipc},
    {REQUEST_FLAG_GET_RECPARAM, get_recparam_to_struct, RESPONSE_FLAG_GET_RECPARAM, NULL},
    {REQUEST_FLAG_SET_RECPARAM, set_recparam_to_struct, RESPONSE_FLAG_SET_RECPARAM, NULL},
    {REQUEST_FLAG_GET_POE_POWER_STATE, NULL, RESPONSE_FLAG_GET_POE_POWER_STATE, get_poe_power_start_to_str},
    {REQUEST_FLAG_GET_IPCALARM, NULL, RESPONSE_FLAG_GET_IPCALARM, get_ipcalarm_to_str},
    {REQUEST_FLAG_SET_IPCALARM, NULL, RESPONSE_FLAG_SET_IPCALARM, set_ipcalarm_to_str},
    {REQUEST_FLAG_GET_IPCVOLUME, NULL, RESPONSE_FLAG_GET_IPCVOLUME, get_ipcvolume_to_str},
    {REQUEST_FLAG_SET_IPCVOLUME, NULL, RESPONSE_FLAG_SET_IPCVOLUME, set_ipcvolume_to_str},
    {REQUEST_FLAG_GET_IPC_DISCONNECT_NUM, NULL, RESPONSE_FLAG_GET_IPC_DISCONNECT_NUM, get_ipc_disconnect_num_to_str},
    {REQUEST_FLAG_GET_VCA_LEFTREMOVE, req_camera_get_vca_leftremove, RESPONSE_FLAG_GET_VCA_LEFTREMOVE, resp_camera_get_vca_leftremove},//get.camera.vca.leftremove
    {REQUEST_FLAG_SET_VCA_LEFTREMOVE, req_camera_set_vca_leftremove, RESPONSE_FLAG_SET_VCA_LEFTREMOVE, resp_camera_set_vca_leftremove},//set.camera.vca.leftremove
    {REQUEST_FLAG_SET_ROI, req_camera_set_image_roi, RESPONSE_FLAG_SET_ROI, NULL},//set.camera.image.roi
    {REQUEST_FLAG_GET_ROI, req_camera_get_image_roi, RESPONSE_FLAG_GET_ROI, resp_camera_get_image_roi},//get.camera.image.roi
    {REQUEST_FLAG_SET_WHITE_BALANCE_SCHE, req_camera_set_white_balance_sche, -1, NULL},//set.camera.image.whitebalance.schedule
    {REQUEST_FLAG_GET_WHITE_BALANCE_SCHE, req_camera_get_white_balance_sche, RESPONSE_FLAG_GET_WHITE_BALANCE_SCHE, resp_camera_get_white_balance_sche},//get.camera.image.whitebalance.schedule
    {REQUEST_FLAG_SET_EXPOSURE_SCHE, req_camera_set_image_exposure_sche, -1, NULL},//set.camera.image.exposure.schedule
    {REQUEST_FLAG_GET_EXPOSURE_SCHE, req_camera_get_image_exposure_sche, RESPONSE_FLAG_GET_EXPOSURE_SCHE, resp_camera_get_image_exposure_sche},//get.camera.image.exposure.schedule
    {REQUEST_FLAG_SET_BWH_SCHE, req_camera_set_bwh_sche, -1, NULL},//set.camera.image.bwh.schedule
    {REQUEST_FLAG_GET_BWH_SCHE, req_camera_get_bwh_sche, RESPONSE_FLAG_GET_BWH_SCHE, resp_camera_get_bwh_sche},//get.camera.image.bwh.schedule
    {REQUEST_FLAG_SET_DAY_NIGHT_INFO, req_camera_set_day_night_info, RESPONSE_FLAG_SET_DAY_NIGHT_INFO, resp_camera_set_day_night_info},//set.camera.image.daynight
    {REQUEST_FLAG_GET_DAY_NIGHT_INFO, req_camera_get_day_night_info, RESPONSE_FLAG_GET_DAY_NIGHT_INFO, resp_camera_get_day_night_info},//get.camera.image.daynight
    {REQUEST_FLAG_SET_DAY_NIGHT_INFO_BATCH, req_camera_set_day_night_info_batch, RESPONSE_FLAG_SET_DAY_NIGHT_INFO_BATCH, resp_camera_set_day_night_info_batch},//set.camera.image.daynight.all
    {REQUEST_FLAG_SET_EXPOSURE_INFO, req_camera_set_exposure_info, -1, NULL},//set.camera.image.exposure
    {REQUEST_FLAG_GET_EXPOSURE_INFO, req_camera_get_exposure_info, -1, NULL},//get.camera.image.exposure
    {REQUEST_FLAG_GET_ANR_SUPPORT, req_camera_get_anr_support, RESPONSE_FLAG_GET_ANR_SUPPORT, resp_camera_get_anr_support},//get.camera.anr_support
    {REQUEST_FLAG_SET_ANR_SUPPORT, req_camera_set_anr_support, RESPONSE_FLAG_SET_ANR_SUPPORT, resp_camera_set_anr_support}, //set.camera.anr_support
    {REQUEST_FLAG_DELETE_PRIVACY_MASK, req_camera_del_privacy_mask, RESPONSE_FLAG_DELETE_PRIVACY_MASK, resp_camera_del_privacy_mask},//set.camera.del_privacy_mask
    {REQUEST_FLAG_DELETE_ROI_AREA, req_camera_del_image_roi, RESPONSE_FLAG_DELETE_ROI_AREA, resp_camera_del_image_roi},//set.camera.del.image.roi
    {REQUEST_FLAG_SET_IPC_ALARMOUT_STATE, set_ipc_alarm_out, -1, NULL},//set.camera.alarm.out
    {REQUEST_FLAG_GET_IPC_SNAPHOST, req_camera_get_ipc_snapshot, RESPONSE_FLAG_GET_IPC_SNAPHOST, resp_camera_get_ipc_snapshot},//get.camera.snapshot_image
    {REQUEST_FLAG_GET_AUTO_TRACK_POINT, req_camera_get_auto_track_point, RESPONSE_FLAG_GET_AUTO_TRACK_POINT, resp_camera_get_auto_track_point},//get.camera.auto_tracking_point
    {REQUEST_FLAG_PTZ_SUPPORT, req_camera_get_ptz_support, RESPONSE_FLAG_PTZ_SUPPORT, resp_camera_get_ptz_support},//get.camera.ptz_support
    {REQUEST_FLAG_GET_AUTO_TRACKING, req_camera_get_auto_track_setting, RESPONSE_FLAG_GET_AUTO_TRACKING, resp_camera_get_auto_track_setting},//"get.camera.auto_tracking_setting"
    {REQUEST_FLAG_SET_AUTO_TRACKING_BATCH, req_camera_set_auto_track_setting, RESPONSE_FLAG_SET_AUTO_TRACKING_BATCH, resp_camera_set_auto_track_setting},//"set.camera.auto_tracking_setting"
    {REQUEST_FLAG_SET_PRIVACY_MASK_EDIT, req_camera_set_privacy_mask_edit, RESPONSE_FLAG_SET_PRIVACY_MASK_EDIT, resp_camera_set_privacy_mask_edit},//set.camera.on_edit_privacy_mask
    {REQUEST_FLAG_GET_IPC_DIGITPOS_ZOOM, req_camera_get_ipc_zoom_state, RESPONSE_FLAG_GET_AUTO_TRACKING, resp_camera_get_ipc_zoom_state},//get.camera.ipc_zoom_state
    {REQUEST_FLAG_GET_IPC_LPR_WILDCARDS, req_camera_get_ipc_wildcards, RESPONSE_FLAG_GET_IPC_LPR_WILDCARDS, resp_camera_get_ipc_wildcards},//get.camera.ipc_wildcards
    {REQUEST_FLAG_SET_IPC_LPR_WILDCARDS, req_camera_set_ipc_wildcards, RESPONSE_FLAG_SET_IPC_LPR_WILDCARDS, resp_camera_set_ipc_wildcards},//set.camera.ipc_wildcards
    {REQUEST_FLAG_GET_IPC_WATERMARK, req_camera_get_ipc_watermark, RESPONSE_FLAG_GET_IPC_WATERMARK, resp_camera_get_ipc_watermark},//get.camera.ipc_watermark
    {REQUEST_FLAG_SET_IPC_WATERMARK_BATCH, req_camera_set_ipc_watermark, RESPONSE_FLAG_SET_IPC_WATERMARK_BATCH, resp_camera_set_ipc_watermark},//set.camera.ipc_watermark
    {REQUEST_FLAG_GET_IPC_VCA_LINECROSSING2, req_camera_get_vca_linecrossing2, RESPONSE_FLAG_GET_IPC_VCA_LINECROSSING2, resp_camera_get_vca_linecrossing2},//get.camera.vca_linecrossing2
    {REQUEST_FLAG_SET_IPC_VCA_LINECROSSING2, req_camera_set_vca_linecrossing2, RESPONSE_FLAG_SET_IPC_VCA_LINECROSSING2, resp_camera_set_vca_linecrossing2},//set.camera.vca_linecrossing2
    {REQUEST_FLAG_GET_VAC_SETTINGS2, req_camera_get_vca_settings2, RESPONSE_FLAG_GET_VAC_SETTINGS2, resp_camera_get_vca_settings2},//get.camera.vca_settings2
    {REQUEST_FLAG_SET_VAC_SETTINGS2, req_camera_set_vca_settings2, RESPONSE_FLAG_SET_VAC_SETTINGS2, resp_camera_set_vca_settings2},//set.camera.vca_settings2
    {REQUEST_FLAG_GET_IPC_EVENT_STREAM_INFO, req_camera_get_event_stream, RESPONSE_FLAG_GET_IPC_EVENT_STREAM_INFO, resp_camera_get_event_stream},//get.camera.event_stream
    {REQUEST_FLAG_SET_IPC_EVENT_STREAM_INFO, req_camera_set_event_stream, RESPONSE_FLAG_SET_IPC_EVENT_STREAM_INFO, resp_camera_set_event_stream},//set.camera.event_stream
    {REQUEST_FLAG_SET_MANUAL_TRACKING_REGION, req_camera_set_manual_tracking_region, RESPONSE_FLAG_SET_MANUAL_TRACKING_REGION, resp_camera_set_manual_tracking_region}, //set.camera.manual_tracking_region
    {REQUEST_FLAG_GET_IPC_LPR_NIGHT_MODE, req_camera_get_ipc_lpr_night_mode, RESPONSE_FLAG_GET_IPC_LPR_NIGHT_MODE, resp_camera_get_ipc_lpr_night_mode},//get.camera.ipc_lpr_night_mode
    {REQUEST_FLAG_GET_IPC_AUTOIRIS_STATUS, req_camera_get_ipc_autoiris_status, RESPONSE_FLAG_GET_IPC_AUTOIRIS_STATUS, resp_camera_get_ipc_autoiris_status},//get.camera.ipc_autoiris_statu
    {REQUEST_FLAG_GET_IPC_AUDIO_INFO, req_camera_get_ipc_audio_info, RESPONSE_FLAG_GET_IPC_AUDIO_INFO, resp_camera_get_ipc_audio_info},//get.camera.ipc_audio_info
    {REQUEST_FLAG_SET_IPC_AUDIO_INFO, req_camera_set_ipc_audio_info, RESPONSE_FLAG_SET_IPC_AUDIO_INFO, resp_camera_set_ipc_audio_info},//set.camera.ipc_audio_info
    {REQUEST_FLAG_CHECK_IPC_FISHEYE_INFO, req_camera_check_ipc_fisheye_info, RESPONSE_FLAG_CHECK_IPC_FISHEYE_INFO, resp_camera_check_ipc_fisheye_info},//get.camera.check_fisheye_info
    {REQUEST_FLAG_SET_IPC_FISHEYE_INFO, req_camera_set_ipc_fisheye_info, RESPONSE_FLAG_SET_IPC_FISHEYE_INFO, resp_camera_set_ipc_fisheye_info},//set.camera.set_fisheye_info
    {REQUEST_FLAG_SET_PEOPLECNT_DB_RESET, req_camera_set_peoplecnt_db_reset, RESPONSE_FLAG_SET_PEOPLECNT_DB_RESET, resp_camera_set_peoplecnt_db_reset},//set.camera.people_couting_db_reset
    {REQUEST_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET, req_camera_set_peoplecnt_liveview_reset, RESPONSE_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET, resp_camera_set_peoplecnt_liveview_reset},//set.camera.people_couting_liveview_reset
    {REQUEST_FLAG_GET_PEOPLECNT_SETTINGS, req_camera_get_peoplecnt_settings, RESPONSE_FLAG_GET_PEOPLECNT_SETTINGS, NULL},//get.camera.people_counting_settings
    {REQUEST_FLAG_SET_PEOPLECNT_SETTINGS, req_camera_set_peoplecnt_settings, RESPONSE_FLAG_SET_PEOPLECNT_SETTINGS, NULL},//set.camera.people_counting_settings
    {REQUEST_FLAG_GET_PEOPLECNT_ACTION, req_camera_get_peoplecnt_action, RESPONSE_FLAG_GET_PEOPLECNT_ACTION, NULL},//get.camera.people_counting_action
    {REQUEST_FLAG_SET_PEOPLECNT_ACTION, req_camera_set_peoplecnt_action, RESPONSE_FLAG_SET_PEOPLECNT_ACTION, NULL},//set.camera.people_counting_action
    {REQUEST_FLAG_GET_IPC_SYSTEM_INFO, req_camera_get_ipc_sysytem_info, RESPONSE_FLAG_GET_IPC_SYSTEM_INFO, resp_camera_get_ipc_sysytem_info},//get.camera.ipc_system_info
    {REQUEST_FLAG_SET_PEOPLECNT_CACHETODB, req_camera_set_peoplecnt_cache_db, RESPONSE_FLAG_SET_PEOPLECNT_CACHETODB, resp_camera_set_peoplecnt_cache_db},//set.camera.people_counting_cache_db
    {REQUEST_FLAG_GET_IPC_FRAME_RESOLUTION, req_camera_get_ipc_frame_resolution, RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION, resp_camera_get_ipc_frame_resolution},//get.camera.frame_resolution
    {REQUEST_FLAG_P2P_GET_PCNT_DATA, req_camera_get_pcnt_data, RESPONSE_FLAG_P2P_GET_PCNT_DATA, NULL},//get.camera.people_counting_data
    {REQUEST_FLAG_SET_3D_PTZ_CTRL, req_camera_set_3d_ptz_ctrl, RESPONSE_FLAG_SET_3D_PTZ_CTRL, resp_camera_set_3d_ptz_ctrl},//set.camera.3d_ptz_ctrl
    {REQUEST_FLAG_SET_MANUAL_TRACKING, req_camera_set_manual_tracking_info, RESPONSE_FLAG_SET_MANUAL_TRACKING, resp_camera_set_manual_tracking_info},//set.camera.manual_tracking_info
    {REQUEST_FLAG_GET_REGIONAL_DETECT_DATA, req_camera_get_regional_detec_data, RESPONSE_FLAG_GET_REGIONAL_DETECT_DATA, resp_camera_get_regional_detec_data},//get.camera.regional_detect_data
    {REQUEST_FLAG_GET_IPC_PEOPLE_REPORT, req_camera_get_ipc_people_report, RESPONSE_FLAG_GET_IPC_PEOPLE_REPORT, resp_camera_get_ipc_people_report},//get.camera.ipc_people_report
    {REQUEST_FLAG_GET_REPORT_AUTO_BACKUP_SETTINGS, req_camera_get_report_auto_backup, RESPONSE_FLAG_GET_REPORT_AUTO_BACKUP_SETTINGS, NULL},//get.camera.report_auto_backup
    {REQUEST_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS, req_camera_set_report_auto_backup, RESPONSE_FLAG_SET_REPORT_AUTO_BACKUP_SETTINGS, resp_camera_set_report_auto_backup},//set.camera.report_auto_backup
    {REQUEST_FLAG_GET_IPC_REGIONAL_PEOPLE, req_camera_get_ipc_regional_people, RESPONSE_FLAG_GET_IPC_REGIONAL_PEOPLE, resp_camera_get_ipc_regional_people},//get.camera.ipc_regional_people
    {REQUEST_FLAG_SET_IPC_REGIONAL_PEOPLE, req_camera_set_ipc_regional_people, RESPONSE_FLAG_SET_IPC_REGIONAL_PEOPLE, resp_camera_set_ipc_regional_people},//set.camera.ipc_regional_people
    {REQUEST_FLAG_GET_FACE_CONFIG, req_camera_get_face_config, RESPONSE_FLAG_GET_FACE_CONFIG, resp_camera_get_face_config},//get.camera.face_config
    {REQUEST_FLAG_SET_FACE_CONFIG, req_camera_set_face_config, RESPONSE_FLAG_SET_FACE_CONFIG, resp_camera_set_face_config},//set.camera.face_config
    {REQUEST_FLAG_GET_FACE_SUPPORT, req_camera_get_face_support, RESPONSE_FLAG_GET_FACE_SUPPORT, resp_camera_get_face_support},//get.camera.face_support
    {REQUEST_FLAG_GET_AI_CONNECT_CNT, req_camera_get_ai_connect_cnt, RESPONSE_FLAG_GET_AI_CONNECT_CNT, resp_camera_get_ai_connect_cnt},//get.camera.face_support
    {REQUEST_FLAG_GET_IPC_CUSTOM_PARAM, req_camera_get_ipc_custom_param, RESPONSE_FLAG_GET_IPC_CUSTOM_PARAM, resp_camera_get_ipc_custom_param},//get.camera.ipc_custom_param
    {REQUEST_FLAG_GET_IMAGE_ENHANCEMENT_SCHEDULE, req_camera_get_image_enhancement_sche, RESPONSE_FLAG_GET_IMAGE_ENHANCEMENT_SCHEDULE, resp_camera_get_image_enhancement_sche},//get.camera.image_enhancement_info
    {REQUEST_FLAG_GET_VAC_BASE_INFO, req_camera_get_vca_base_info, RESPONSE_FLAG_GET_VAC_BASE_INFO, resp_camera_get_vca_base_info},//get.camera.vca.base_info
    {REQUEST_FLAG_GET_IPC_HEATMAP_SUPPORT, req_camera_get_ipc_heatmap_support, RESPONSE_FLAG_GET_IPC_HEATMAP_SUPPORT, resp_camera_get_ipc_heatmap_support},//get.camera.ipc_heatmap_support
    {REQUEST_FLAG_GET_IPC_PEOPLECNT_LINECNT, req_camera_get_ipc_peoplecnt_linecnt, RESPONSE_FLAG_GET_IPC_PEOPLECNT_LINECNT, resp_camera_get_ipc_peoplecnt_linecnt},//get.camera.ipc_peoplecnt_linecnt
    {REQUEST_FLAG_SET_IPC_REBOOT, req_camera_set_ipc_reboot, RESPONSE_FLAG_SET_IPC_REBOOT, resp_camera_common_set},//set.camera.ipc_reboot
    {REQUEST_FLAG_SET_IPC_RESET, req_camera_set_ipc_reset, RESPONSE_FLAG_SET_IPC_RESET, resp_camera_common_set},//set.camera.ipc_reset
    {REQUEST_FLAG_GET_IPC_DIAGNOSE, req_camera_get_ipc_diagnose, RESPONSE_FLAG_GET_IPC_DIAGNOSE, resp_camera_get_ipc_file},//get.camera.ipc_diagnose
    {REQUEST_FLAG_GET_IPC_CFG, req_camera_get_ipc_cfg, RESPONSE_FLAG_GET_IPC_CFG, resp_camera_get_ipc_file_data},//get.camera.ipc_cfg
    {REQUEST_FLAG_SET_IPC_CFG, req_camera_set_ipc_cfg, RESPONSE_FLAG_SET_IPC_CFG, resp_camera_common_set},//set.camera.ipc_cfg
    {REQUEST_FLAG_GET_IPC_LOG, req_camera_get_ipc_log, RESPONSE_FLAG_GET_IPC_LOG, resp_camera_get_ipc_file_data},//get.camera.ipc_log
    {REQUEST_FLAG_GET_IPC_ALARMSTATUS_BATCH, req_camera_get_ipc_alarmstatus_batch, RESPONSE_FLAG_GET_IPC_ALARMSTATUS_BATCH, resp_camera_get_ipc_alarmstatus_batch}, // get.camera.ipc.alarmstatus.batch
    {REQUEST_FLAG_ONLINE_UPGRADE_CAMERA, req_camera_set_ipc_onlineupgrade_batch, RESPONSE_FLAG_ONLINE_UPGRADE_CAMERA, resp_camera_set_ipc_onlineupgrade_batch}, // set.camera.ipc.onlineupgrade.batch
    {REQUEST_FLAG_ONLINE_UPGRADE_CAMERA_STOP, req_camera_set_ipc_onlineupgrade_stop_batch, -1, NULL}, // set.camera.ipc.onlineupgrade.stop.batch
    {REQUEST_FLAG_GET_IPC_PTZ_WIPER, req_camera_get_ipc_ptz_wiper, RESPONSE_FLAG_GET_IPC_PTZ_WIPER, resp_camera_get_ipc_ptz_wiper},// get.camera.ipc.ptz.wiper
    {REQUEST_FLAG_SET_IPC_PTZ_WIPER, req_camera_set_ipc_ptz_wiper, RESPONSE_FLAG_SET_IPC_PTZ_WIPER, resp_camera_set_ipc_ptz_wiper},// set.camera.ipc.ptz.wiper
    {REQUEST_FLAG_GET_IPC_PTZ_AUTO_HOME, req_camera_get_ipc_ptz_auto_home, RESPONSE_FLAG_GET_IPC_PTZ_AUTO_HOME, resp_camera_get_ipc_ptz_auto_home},// get.camera.ipc.ptz.auto.home
    {REQUEST_FLAG_SET_IPC_PTZ_AUTO_HOME, req_camera_set_ipc_ptz_auto_home, RESPONSE_FLAG_SET_IPC_PTZ_AUTO_HOME, resp_camera_set_ipc_ptz_auto_home},// set.camera.ipc.ptz.auto.home
    {REQUEST_FLAG_SET_IPC_PTZ_INITIAL_POSITION, req_camera_set_ipc_ptz_initial_position, RESPONSE_FLAG_SET_IPC_PTZ_INITIAL_POSITION, resp_camera_set_ipc_ptz_initial_position},// set.camera.ipc.initial.position
    {REQUEST_FLAG_SET_IPC_PTZ_CONFIG_CLEAR, req_camera_set_ipc_ptz_config_clear, RESPONSE_FLAG_SET_IPC_PTZ_CONFIG_CLEAR, resp_camera_set_ipc_ptz_config_clear},// set.camera.ipc.initial.position
    {REQUEST_FLAG_GET_IPC_PTZ_LIMIT, req_camera_get_ipc_ptz_limit, RESPONSE_FLAG_GET_IPC_PTZ_LIMIT, resp_camera_get_ipc_ptz_limit},// get.camera.ipc.ptz.limit
    {REQUEST_FLAG_SET_IPC_PTZ_LIMIT, req_camera_set_ipc_ptz_limit, RESPONSE_FLAG_SET_IPC_PTZ_LIMIT, resp_camera_set_ipc_ptz_limit},// set.camera.ipc.ptz.limit
    {REQUEST_FLAG_GET_IPC_PTZ_SCHE_TASK, req_camera_get_ipc_ptz_sche_task, RESPONSE_FLAG_GET_IPC_PTZ_SCHE_TASK, resp_camera_get_ipc_ptz_sche_task},// get.camera.ipc.ptz.sche.task
    {REQUEST_FLAG_SET_IPC_PTZ_SCHE_TASK, req_camera_set_ipc_ptz_sche_task, RESPONSE_FLAG_SET_IPC_PTZ_SCHE_TASK, resp_camera_set_ipc_ptz_sche_task},// set.camera.ipc.ptz.sche.task
    {REQUEST_FLAG_SET_IPC_PTZ_CONTROL_JSON, req_camera_set_ipc_ptz_control_json, RESPONSE_FLAG_SET_IPC_PTZ_CONTROL_JSON, resp_camera_set_ipc_ptz_control_json},// set.camera.ipc.ptz.control
    {REQUEST_FLAG_GET_IPC_PTZ_PANEL_STATUS, req_camera_get_ipc_ptz_panel_status, RESPONSE_FLAG_GET_IPC_PTZ_PANEL_STATUS, resp_camera_get_ipc_ptz_panel_status},// get.camera.ipc.ptz.panel.status
    {REQUEST_FLAG_GET_IPC_CAP_IMAGE, req_camera_get_ipc_cap_image, RESPONSE_FLAG_GET_IPC_CAP_IMAGE, resp_camera_get_ipc_cap_image},// get.camera.ipc.cap.image
    {REQUEST_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE, req_common_chnid_json, RESPONSE_FLAG_GET_IPC_IMAGE_SPLICEDISTANCE, resp_camera_get_ipc_image_splicedistance},// get.camera.ipc.image.splicedistance
    {REQUEST_FLAG_SET_IPC_IMAGE_SPLICEDISTANCE, req_camera_set_ipc_image_splicedistance, RESPONSE_FLAG_SET_IPC_IMAGE_SPLICEDISTANCE, resp_common_set_json},// set.camera.ipc.image.splicedistance
    // request register array end
};

void p2p_camera_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_camera_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

