#include "sdk_util.h"


int get_http_notification_json(void *data, int dataLen, char *resp, int respLen)
{
    int type = MAXEVT;
    int chnId = -1;
    int alarmId = 0;
    int interval = 0;
    int event = 0;
    int i;
    char exceptName[32] = {0};
    char scheTable[MAX_LEN_64] = {0};
    char paramsTable[MAX_LEN_64] = {0};
    SMART_SCHEDULE schedule;
    HTTP_NOTIFICATION_PARAMS_S httpPrms;
    HTTP_NOTIFICATION_PARAMS_S httpPrmsArr[EXCEPT_COUNT];
    struct alarm_chn alarmInfo;
    SMART_EVENT smartEvent;
    struct motion move;
    struct video_loss videoloss;
    struct alarm_in nvrAlarm;
    cJSON *reqRoot = NULL;
    cJSON *respRoot = NULL;
    cJSON *httpPrmsObj = NULL;
    cJSON *scheArr = NULL;
    cJSON *item = NULL;
    char *respStr = NULL;

    reqRoot = cJSON_Parse(data);
    if (!reqRoot) {
        return -1;
    }

    item = cJSON_GetObjectItem(reqRoot, "type");
    if (!item) {
        cJSON_Delete(reqRoot);
        return -1;
    }
    type = item->valueint;

    if (type != EXCEPTION) {
        item = cJSON_GetObjectItem(reqRoot, "chnId");
        if (!item) {
            cJSON_Delete(reqRoot);
            return -1;
        }
        chnId = item->valueint;
        if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
            cJSON_Delete(reqRoot);
            return -1;
        }
    }

    item = cJSON_GetObjectItem(reqRoot, "alarmId");
    if (item) {
        alarmId = item->valueint;
        if (alarmId < 0 || alarmId >= MAX_IPC_ALARM_IN) {
            alarmId = 0;
        }
    }

    cJSON_Delete(reqRoot);

    alarmInfo.chnid = chnId;
    alarmInfo.alarmid = alarmId;
    switch (type) {
        case MOTION: // motion
            snprintf(scheTable, sizeof(scheTable), MOT_HTTP_SCHE);
            snprintf(paramsTable, sizeof(paramsTable), MOT_HTTP_PARAMS);
            memset(&move, 0x0, sizeof(struct motion));
            read_motion(SQLITE_FILE_NAME, &move, chnId);
            interval = move.http_notification_interval;
            break;

        case VIDEOLOSS: // videolss
            snprintf(scheTable, sizeof(scheTable), VDL_HTTP_SCHE);
            snprintf(paramsTable, sizeof(paramsTable), VDL_HTTP_PARAMS);
            memset(&videoloss, 0x0, sizeof(struct video_loss));
            read_video_lost(SQLITE_FILE_NAME, &videoloss, chnId);
            interval = videoloss.http_notification_interval;
            break;

        case ALARMIO: // nvr alarm in
            snprintf(scheTable, sizeof(scheTable), AIN_HTTP_SCHE);
            snprintf(paramsTable, sizeof(paramsTable), AIN_HTTP_PARAMS);
            memset(&nvrAlarm, 0x0, sizeof(struct smart_event));
            read_alarm_in(SQLITE_FILE_NAME, &nvrAlarm, chnId);
            interval = nvrAlarm.http_notification_interval;
            break;

        // vca
        case REGION_EN:
        case REGION_EXIT:
        case ADVANCED_MOT:
        case TAMPER_DET:
        case LINE_CROSS:
        case LOITER:
        case HUMAN_DET:
        case PEOPLE_COUNT:
        case LEFTREMOVE:
            if (type == REGION_EN) {
                snprintf(scheTable, sizeof(scheTable), VREIN_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VREIN_HTTP_PARAMS);
                event = REGIONIN;
            } else if (type == REGION_EXIT) {
                snprintf(scheTable, sizeof(scheTable), VREEX_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VREEX_HTTP_PARAMS);
                event = REGIONOUT;
            } else if (type == ADVANCED_MOT) {
                snprintf(scheTable, sizeof(scheTable), VMOT_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VMOT_HTTP_PARAMS);
                event = ADVANCED_MOTION;
            } else if (type == TAMPER_DET) {
                snprintf(scheTable, sizeof(scheTable), VTEP_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VTEP_HTTP_PARAMS);
                event = TAMPER;
            } else if (type == LINE_CROSS) {
                snprintf(scheTable, sizeof(scheTable), VLSS_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VLSS_HTTP_PARAMS);
                event = LINECROSS;
            } else if (type == LOITER) {
                snprintf(scheTable, sizeof(scheTable), VLER_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VLER_HTTP_PARAMS);
                event = LOITERING;
            } else if (type == HUMAN_DET) {
                snprintf(scheTable, sizeof(scheTable), VHMN_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VHMN_HTTP_PARAMS);
                event = HUMAN;
            } else if (type == PEOPLE_COUNT) {
                snprintf(scheTable, sizeof(scheTable), VPPE_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VPPE_HTTP_PARAMS);
                event = PEOPLE_CNT;
            } else {
                snprintf(scheTable, sizeof(scheTable), VOBJ_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VOBJ_HTTP_PARAMS);
                event = OBJECT_LEFTREMOVE;
            }
            memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
            read_smart_event(SQLITE_FILE_NAME, &smartEvent, chnId, event);
            interval = smartEvent.http_notification_interval;
            break;
            
        // exception
        case EXCEPTION:
            snprintf(paramsTable, sizeof(paramsTable), EXCEPT_HTTP_PARAMS);
            break;

        // anpr
        case ANPR_BLACK_EVT:
        case ANPR_WHITE_EVT:
        case ANPR_VISTOR_EVT:
            if (type == ANPR_BLACK_EVT) {
                snprintf(scheTable, sizeof(scheTable), LPRB_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), LPRB_HTTP_PARAMS);
                event = ANPR_BLACK;
            } else if (type == ANPR_WHITE_EVT) {
                snprintf(scheTable, sizeof(scheTable), LPRW_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), LPRW_HTTP_PARAMS);
                event = ANPR_WHITE;
            } else {
                snprintf(scheTable, sizeof(scheTable), LPRV_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), LPRV_HTTP_PARAMS);
                event = ANPR_VISTOR;
            }
            memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
            read_anpr_event(SQLITE_FILE_NAME, &smartEvent, chnId, event);
            interval = smartEvent.http_notification_interval;
            break;

        //camera alarm in
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(scheTable, sizeof(scheTable), AINCH_HTTP_SCHE, alarmInfo.alarmid);
            snprintf(paramsTable, sizeof(paramsTable), AINCH_HTTP_PARAMS, alarmInfo.alarmid);
            memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
            read_alarm_chnIn_event(SQLITE_FILE_NAME, &smartEvent, &alarmInfo);
            interval = smartEvent.http_notification_interval;
            break;

        default:
            cJSON_Delete(reqRoot);
            return -1;
    }

    if (type == EXCEPTION) {
        read_http_notification_params_init(SQLITE_FILE_NAME, paramsTable, httpPrmsArr, EXCEPT_COUNT);
    
        respRoot = cJSON_CreateObject();
        for (i = 0; i < EXCEPT_COUNT; ++i) {
            if (!httpPrmsArr[i].url[0]) {
                continue;
            }
        
            get_exception_name(i, exceptName, sizeof(exceptName));
            httpPrmsObj = cJSON_AddObjectToObject(respRoot, exceptName);
            cJSON_AddStringToObject(httpPrmsObj, "url", httpPrmsArr[i].url);
            cJSON_AddStringToObject(httpPrmsObj, "username", httpPrmsArr[i].username);
            cJSON_AddStringToObject(httpPrmsObj, "password", httpPrmsArr[i].password);
        }
    } else {
        respRoot = cJSON_CreateObject();

        cJSON_AddNumberToObject(respRoot, "httpInterval", interval);
        
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_http_notification_schedule(SQLITE_FILE_NAME, &schedule, scheTable, chnId);
        scheArr = get_json_schedule_object(&schedule);
        cJSON_AddItemToObject(respRoot, "httpSche", scheArr);

        httpPrmsObj =cJSON_AddObjectToObject(respRoot, "httpParams");
        read_http_notification_params(SQLITE_FILE_NAME, &httpPrms, paramsTable, chnId);
        cJSON_AddStringToObject(httpPrmsObj, "url", httpPrms.url);
        cJSON_AddStringToObject(httpPrmsObj, "username", httpPrms.username);
        cJSON_AddStringToObject(httpPrmsObj, "password", httpPrms.password);
    }
    respStr = cJSON_Print(respRoot);
    if (!respStr) {
        cJSON_Delete(respRoot);
        return -1;
    }
    snprintf(resp, respLen, respStr);

    cJSON_free(respStr);
    cJSON_Delete(respRoot);

    return 0;
}

int set_http_notification_json(void *data, int dataLen, char *resp, int respLen, REQUEST_S *pReq)
{
    int type = MAXEVT;
    int chnId = -1;
    int alarmId = 0;
    int interval = 0;
    int setSchedule = 0, setParams = 0;
    int reqNumber = 0;
    void *info = NULL;
    int len;
    Uint64 batch = 0;
    Uint64 copy = 0;
    char scheTable[MAX_LEN_64] = {0};
    char paramsTable[MAX_LEN_64] = {0};
    SMART_SCHEDULE schedule;
    HTTP_NOTIFICATION_PARAMS_S httpPrms;
    HTTP_NOTIFICATION_PARAMS_S httpPrmsArr[EXCEPT_COUNT];
    struct alarm_chn alarmInfo;
    SMART_EVENT smartEvent;
    struct req_anpr_event anprInfo;
    int event = 0;
    REQ_UPDATE_CHN smart = {0};
    cJSON *reqRoot;
    cJSON *item;
    int i;
    char exceptName[32] = {0};

    memset(httpPrmsArr, 0, sizeof(httpPrmsArr));

    reqRoot = cJSON_Parse(data);
    if (!reqRoot) {
        return -1;
    }

    item = cJSON_GetObjectItem(reqRoot, "type");
    if (!item) {
        cJSON_Delete(reqRoot);
        return -1;
    }
    type = item->valueint;

    if (type != EXCEPTION) {
        item = cJSON_GetObjectItem(reqRoot, "chnId");
        if (!item) {
            cJSON_Delete(reqRoot);
            return -1;
        }
        chnId = item->valueint;
        if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
            cJSON_Delete(reqRoot);
            return -1;
        }
        batch |= (Uint64)1 << chnId;
    }

    item = cJSON_GetObjectItem(reqRoot, "alarmId");
    if (item) {
        alarmId = item->valueint;
        if (alarmId < 0 || alarmId >= MAX_IPC_ALARM_IN) {
            alarmId = 0;
        }
    }
    
    item = cJSON_GetObjectItem(reqRoot, "interval");
    if (item) {
        interval = item->valueint;
    }

    copy = get_batch_by_json(reqRoot, "copy");

    if (get_schedule_by_cjson(reqRoot, "httpSche", &schedule) == 0) {
        setSchedule = 1;
    }

    if (type == EXCEPTION) {
        for (i = 0; i < EXCEPT_COUNT; ++i) {
            get_exception_name(i, exceptName, sizeof(exceptName));
            item = cJSON_GetObjectItem(reqRoot, exceptName);
            if (!item) {
                httpPrmsArr[i].id = -1;
                continue;
            }
            
            httpPrmsArr[i].id = i;
            snprintf(httpPrmsArr[i].url, sizeof(httpPrmsArr[i].url), cJSON_GetObjectItem(item, "url")->valuestring);
            snprintf(httpPrmsArr[i].username, sizeof(httpPrmsArr[i].username), cJSON_GetObjectItem(item, "username")->valuestring);
            snprintf(httpPrmsArr[i].password, sizeof(httpPrmsArr[i].password), cJSON_GetObjectItem(item, "password")->valuestring);

            setParams = 1;
        }
    } else {
        memset(&httpPrms, 0x0, sizeof(HTTP_NOTIFICATION_PARAMS_S));
        item = cJSON_GetObjectItem(reqRoot, "httpParams");
        if (item) {
            httpPrms.id = chnId;
            snprintf(httpPrms.url, sizeof(httpPrms.url), cJSON_GetObjectItem(item, "url")->valuestring);
            snprintf(httpPrms.username, sizeof(httpPrms.username), cJSON_GetObjectItem(item, "username")->valuestring);
            snprintf(httpPrms.password, sizeof(httpPrms.password), cJSON_GetObjectItem(item, "password")->valuestring);

            setParams = 1;
        }
    }

    alarmInfo.chnid = chnId;
    alarmInfo.alarmid = alarmId;
    anprInfo.chnid = chnId;
    switch (type) {
        case MOTION://motion
            snprintf(scheTable, sizeof(scheTable), MOT_HTTP_SCHE);
            snprintf(paramsTable, sizeof(paramsTable), MOT_HTTP_PARAMS);
            if (interval) {
                struct motion move;
                memset(&move, 0x0, sizeof(struct motion));
                read_motion(SQLITE_FILE_NAME, &move, chnId);
                if (move.http_notification_interval != interval) {
                    move.http_notification_interval = interval;
                    write_motion(SQLITE_FILE_NAME, &move);
                }
                if (copy) {
                    copy_motions(SQLITE_FILE_NAME, &move, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_MOTION;
            info = &chnId;
            len = sizeof(int);
            //copy:
            if (copy != 0) {
                copy_http_notification_schedules(SQLITE_FILE_NAME, &schedule, scheTable, copy);
                copy_http_notification_params(SQLITE_FILE_NAME, paramsTable, &httpPrms, copy);
            }
            break;

        case VIDEOLOSS://videolss
            snprintf(scheTable, sizeof(scheTable), VDL_HTTP_SCHE);
            snprintf(paramsTable, sizeof(paramsTable), VDL_HTTP_PARAMS);
            if (interval) {
                struct video_loss videoloss;
                memset(&videoloss, 0x0, sizeof(struct video_loss));
                read_video_lost(SQLITE_FILE_NAME, &videoloss, chnId);
                if (videoloss.http_notification_interval != interval) {
                    videoloss.http_notification_interval = interval;
                    write_video_lost(SQLITE_FILE_NAME, &videoloss);
                }
                if (copy) {
                    copy_video_losts(SQLITE_FILE_NAME, &videoloss, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_VIDEOLOSS_BATCH;
            info = &batch;
            len = sizeof(Uint64);
            // copy:
            if (copy != 0) {
                copy_http_notification_schedules(SQLITE_FILE_NAME, &schedule, scheTable, copy);
                copy_http_notification_params(SQLITE_FILE_NAME, paramsTable, &httpPrms, copy);
            }
            break;

        case ALARMIO://nvr alarm in
            snprintf(scheTable, sizeof(scheTable), AIN_HTTP_SCHE);
            snprintf(paramsTable, sizeof(paramsTable), AIN_HTTP_PARAMS);
            if (interval) {
                struct alarm_in nvrAlarm;
                memset(&nvrAlarm, 0x0, sizeof(struct smart_event));
                read_alarm_in(SQLITE_FILE_NAME, &nvrAlarm, chnId);
                if (nvrAlarm.http_notification_interval != interval) {
                    nvrAlarm.http_notification_interval = interval;
                    write_alarm_in(SQLITE_FILE_NAME, &nvrAlarm);
                }
                if (copy) {
                    copy_alarm_in_events(SQLITE_FILE_NAME, &nvrAlarm, MAX_ALARM_IN, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_ALARMIN_BATCH;
            info = &batch;
            len = sizeof(Uint64);
            // copy:
            if (copy != 0) {
                copy_http_notification_schedules(SQLITE_FILE_NAME, &schedule, scheTable, copy);
                copy_http_notification_params(SQLITE_FILE_NAME, paramsTable, &httpPrms, copy);
            }
            break;

        //vca
        case REGION_EN:
        case REGION_EXIT:
        case ADVANCED_MOT:
        case TAMPER_DET:
        case LINE_CROSS:
        case LOITER:
        case HUMAN_DET:
        case PEOPLE_COUNT:
        case LEFTREMOVE:
            if (type == REGION_EN) {
                snprintf(scheTable, sizeof(scheTable), VREIN_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VREIN_HTTP_PARAMS);
                event = REGIONIN;
            } else if (type == REGION_EXIT) {
                snprintf(scheTable, sizeof(scheTable), VREEX_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VREEX_HTTP_PARAMS);
                event = REGIONOUT;
            } else if (type == ADVANCED_MOT) {
                snprintf(scheTable, sizeof(scheTable), VMOT_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VMOT_HTTP_PARAMS);
                event = ADVANCED_MOTION;
            } else if (type == TAMPER_DET) {
                snprintf(scheTable, sizeof(scheTable), VTEP_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VTEP_HTTP_PARAMS);
                event = TAMPER;
            } else if (type == LINE_CROSS) {
                snprintf(scheTable, sizeof(scheTable), VLSS_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VLSS_HTTP_PARAMS);
                event = LINECROSS;
            } else if (type == LOITER) {
                snprintf(scheTable, sizeof(scheTable), VLER_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VLER_HTTP_PARAMS);
                event = LOITERING;
            } else if (type == HUMAN_DET) {
                snprintf(scheTable, sizeof(scheTable), VHMN_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VHMN_HTTP_PARAMS);
                event = HUMAN;
            } else if (type == PEOPLE_COUNT) {
                snprintf(scheTable, sizeof(scheTable), VPPE_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VPPE_HTTP_PARAMS);
                event = PEOPLE_CNT;
            } else {
                snprintf(scheTable, sizeof(scheTable), VOBJ_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), VOBJ_HTTP_PARAMS);
                event = OBJECT_LEFTREMOVE;
            }

            if (interval) {
                memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
                read_smart_event(SQLITE_FILE_NAME, &smartEvent, chnId, event);
                if (smartEvent.http_notification_interval != interval) {
                    smartEvent.http_notification_interval = interval;
                    write_smart_event(SQLITE_FILE_NAME, &smartEvent, event);
                }
                if (copy) {
                    copy_smart_events(SQLITE_FILE_NAME, &smartEvent, copy, event);
                }
            }
            reqNumber = REQUEST_FLAG_SET_SMART_EVENT;
            ms_set_bit(&smart.chnMask, chnId, 1);
            info = &smart;
            len = sizeof(smart);
            //copy:
            if (copy != 0) {
                copy_http_notification_schedules(SQLITE_FILE_NAME, &schedule, scheTable, copy);
                copy_http_notification_params(SQLITE_FILE_NAME, paramsTable, &httpPrms, copy);
            }
            break;
            
        case EXCEPTION:
            snprintf(paramsTable, sizeof(paramsTable), EXCEPT_HTTP_PARAMS);
            reqNumber = REQUEST_FLAG_SET_EXCEPT_HTTP_PARAMS;
            len = 0;
            break;
        
        //anpr
        case ANPR_BLACK_EVT:
        case ANPR_WHITE_EVT:
        case ANPR_VISTOR_EVT:
            if (type == ANPR_BLACK_EVT) {
                snprintf(scheTable, sizeof(scheTable), LPRB_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), LPRB_HTTP_PARAMS);
                event = ANPR_BLACK;
            } else if (type == ANPR_WHITE_EVT) {
                snprintf(scheTable, sizeof(scheTable), LPRW_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), LPRW_HTTP_PARAMS);
                event = ANPR_WHITE;
            } else {
                snprintf(scheTable, sizeof(scheTable), LPRV_HTTP_SCHE);
                snprintf(paramsTable, sizeof(paramsTable), LPRV_HTTP_PARAMS);
                event = ANPR_VISTOR;
            }
            if (interval) {
                memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
                read_anpr_event(SQLITE_FILE_NAME, &smartEvent, chnId, event);
                if (smartEvent.whiteled_interval != interval) {
                    smartEvent.whiteled_interval = interval;
                    write_anpr_event(SQLITE_FILE_NAME, &smartEvent, event);
                }
            }
            anprInfo.modeType = event;
            reqNumber = REQUEST_FLAG_SET_ANPR_EVENT;
            info = &anprInfo;
            len = sizeof(struct req_anpr_event);
            break;

        //camera alarm in
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(scheTable, sizeof(scheTable), AINCH_HTTP_SCHE, alarmInfo.alarmid);
            snprintf(paramsTable, sizeof(paramsTable), AINCH_HTTP_PARAMS, alarmInfo.alarmid);
            if (interval) {
                memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
                read_alarm_chnIn_event(SQLITE_FILE_NAME, &smartEvent, &alarmInfo);
                if (smartEvent.whiteled_interval != interval) {
                    smartEvent.whiteled_interval = interval;
                    write_alarm_chnIn_event(SQLITE_FILE_NAME, &smartEvent, &alarmInfo);
                }
                if (copy) {
                    copy_alarm_chnIn_events(SQLITE_FILE_NAME, &smartEvent, alarmInfo.alarmid, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_IPC_ALARMIN_EVENT;
            info = &batch;
            len = sizeof(Uint64);
            //copy:
            if (copy != 0) {
                copy_http_notification_schedules(SQLITE_FILE_NAME, &schedule, scheTable, copy);
                copy_http_notification_params(SQLITE_FILE_NAME, paramsTable, &httpPrms, copy);
            }
            break;

        default:
            // url_writeback_error_resp(req, PARAM_ERROR);
            cJSON_Delete(reqRoot);
            return -1;
    }

    if (setSchedule) {
        write_http_notification_schedule(SQLITE_FILE_NAME, &schedule, scheTable, chnId);
    }

    if (setParams) {
        if (type == EXCEPTION) {
            write_http_notification_params_batch(SQLITE_FILE_NAME, &httpPrms, paramsTable, EXCEPT_COUNT);
        } else {
            write_http_notification_params(SQLITE_FILE_NAME, &httpPrms, paramsTable);
        }
    }

    pReq->reqNum = reqNumber;
    if (len > 0) {
        pReq->data = ms_malloc(len);
        memcpy(pReq->data, info, len);
    }
    pReq->len = len;
    
    cJSON_Delete(reqRoot);
    return 0;
}

int sdkp2p_get_ipc_audio_alarm(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(IPC_AUDIO_ALARM_S) || !resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *root = NULL;
    IPC_AUDIO_ALARM_S *pInfo = (IPC_AUDIO_ALARM_S *)data;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    cJSON_AddNumberToObject(root, "chnId", pInfo->chnId);
    cJSON_AddNumberToObject(root, "alarmEnable", pInfo->alarmEnable);
    cJSON_AddNumberToObject(root, "alarmThreshold", pInfo->alarmThreshold);
    cJSON_AddNumberToObject(root, "sampleValue", pInfo->sampleValue);
    cJSON_AddItemToObject(root, "action", get_action_audio_alarm(pInfo->chnId));

    sResp = cJSON_Print(root);
    cJSON_Delete(root);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, sResp);
    cJSON_free(sResp);

    return 0;
}

SDK_ERROR_CODE sdkp2p_set_ipc_audio_alarm(const char *req, IPC_AUDIO_ALARM_S *param)
{
    cJSON *root = NULL;
    cJSON *item = NULL;
    SMART_SCHEDULE sche;
    Uint64 chnMask = 0;
    HTTP_NOTIFICATION_PARAMS_S httpPrms;

    memset(&sche, 0, sizeof(SMART_SCHEDULE));
    memset(&httpPrms, 0, sizeof(HTTP_NOTIFICATION_PARAMS_S));

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

    item = cJSON_GetObjectItem(root, "alarmEnable");
    if (item) {
        param->alarmEnable = item->valueint;
    } else {
        param->alarmEnable = MS_INVALID_VALUE;
    }

    item = cJSON_GetObjectItem(root, "alarmThreshold");
    if (item) {
        param->alarmThreshold = item->valueint;
    } else {
        param->alarmThreshold = MS_INVALID_VALUE;
    }

    item = cJSON_GetObjectItem(root, "copyChn");
    if (item) {
        snprintf(param->copyChn, sizeof(param->copyChn), item->valuestring);
        chnMask = get_channel_mask(param->copyChn);
    } else {
        snprintf(param->copyChn, sizeof(param->copyChn), MS_INVALID_STRING);
    }
    chnMask |= ((Uint64)1<<param->chnId);
    set_action_audio_alarm(cJSON_GetObjectItem(root, "action"), param->chnId, chnMask);
    cJSON_Delete(root);

    return REQUEST_SUCCESS;
}

SDK_ERROR_CODE sdkp2p_get_ipc_audio_alarm_sample(void *data, int dataLen, char *resp, int respSize)
{
    if (!data || dataLen < sizeof(int) || !resp || respSize <= 0) {
        return OTHERS_ERROR;
    }
    
    cJSON *root = NULL;
    char *rootStr = NULL;
    int sample = *(int *)data;

    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    cJSON_AddNumberToObject(root, "sample", sample);

    rootStr = cJSON_Print(root);
    cJSON_Delete(root);
    if (!rootStr) {
        return OTHERS_ERROR;
    }
    snprintf(resp, respSize, rootStr);
    cJSON_free(rootStr);

    return REQUEST_SUCCESS;
}

static void req_event_get_alarmout(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int id = 0;
    char sValue[256] = {0};
    struct alarm_out alout = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        id = atoi(sValue);
    }

    read_alarm_out(SQLITE_FILE_NAME, &alout, id);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id=%d&", alout.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "name=%s&", alout.name);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "type=%d&", alout.type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", alout.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "duration_time=%d&", alout.duration_time);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_event_set_alarmout(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    struct alarm_out alarm = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        alarm.id = atoi(sValue);
    }
    *res = alarm.id;
    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        snprintf(alarm.name, sizeof(alarm.name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        alarm.type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        alarm.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "duration_time=", sValue, sizeof(sValue))) {
        alarm.duration_time = atoi(sValue);
    }

    write_alarm_out(SQLITE_FILE_NAME, &alarm);
    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);

    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMOUTPUT, alarm.id + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);

}

static void req_event_get_alarmin(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int alarmId = 0;
    char sValue[256] = {0};
    struct alarm_in alin = {0};
    DISARMING_S disarm;
    memset(&disarm, 0, sizeof(DISARMING_S));
    // char sChnMask[MAX_REAL_CAMERA] = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        alarmId = atoi(sValue);
    }

    read_alarm_in(SQLITE_FILE_NAME, &alin, alarmId);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id=%d&", alin.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", alin.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "name=%s&", alin.name);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "type=%d&", alin.type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels=%u&", alin.tri_channels);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_alarms=%u&", alin.tri_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "buzzer_interval=%d&", alin.buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&",
             alin.email_buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_pic_enable=%d&",
             alin.email_pic_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_pic=%s&",
             alin.tri_channels_pic);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&", alin.ptzaction_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "alarmout_interval=%d&",
             alin.alarmout_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_ex=%s&", alin.tri_channels_ex);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_chnout1_alarms=%s&",
             alin.tri_chnout1_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_chnout2_alarms=%s&",
             alin.tri_chnout2_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_snapshot=%s&",
             alin.tri_channels_snapshot);
    if (alarmId == 0) {
        read_disarming(SQLITE_FILE_NAME, &disarm);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "linkageAction=%d&", disarm.linkageAction);
        // sdkp2p_chnmask_to_str(alin.disarmChnMask, sChnMask, sizeof(sChnMask));
        // snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "disarmChnMask=%s&", sChnMask);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "disarmActionMask=%d&", disarm.disarmActionMask);
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_event_set_alarmin(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    int i;
    char sValue[256] = {0};
    struct alarm_in alarm = {0};
    DISARMING_S disarm;
    memset(&disarm, 0, sizeof(DISARMING_S));

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        alarm.id = atoi(sValue);
    }

    *res = alarm.id;
    read_alarm_in(SQLITE_FILE_NAME, &alarm, alarm.id);

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        alarm.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        snprintf(alarm.name, sizeof(alarm.name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        alarm.type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_channels=", sValue, sizeof(sValue))) {
        alarm.tri_channels = (unsigned int)atoll(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_channels_ex=", sValue, sizeof(sValue))) {
        snprintf(alarm.tri_channels_ex, sizeof(alarm.tri_channels_ex), "%s", sValue);
        for (i = 0; i < strlen(alarm.tri_channels_ex); i++) {
            if (alarm.tri_channels_ex[i] == '1') {
                alarm.tri_channels |= (1 << i);
            } else {
                alarm.tri_channels &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (alarm.tri_channels & (1 << i)) {
                alarm.tri_channels_ex[i] = '1';
            } else {
                alarm.tri_channels_ex[i] = '0';
            }
        }
    }
    if (!sdkp2p_get_section_info(buf, "tri_alarms=", sValue, sizeof(sValue))) {
        alarm.tri_alarms = (unsigned int)atoll(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "buzzer_interval=", sValue, sizeof(sValue))) {
        alarm.buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "email_interval=", sValue, sizeof(sValue))) {
        alarm.email_buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "email_pic_enable=", sValue, sizeof(sValue))) {
        alarm.email_pic_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_channels_pic=", sValue, sizeof(sValue))) {
        snprintf(alarm.tri_channels_pic, sizeof(alarm.tri_channels_pic), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        alarm.ptzaction_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "alarmout_interval=", sValue, sizeof(sValue))) {
        alarm.alarmout_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_chnout1_alarms=", sValue, sizeof(sValue))) {
        snprintf(alarm.tri_chnout1_alarms, sizeof(alarm.tri_chnout1_alarms), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_chnout2_alarms=", sValue, sizeof(sValue))) {
        snprintf(alarm.tri_chnout2_alarms, sizeof(alarm.tri_chnout2_alarms), "%s", sValue);
    }
    if (alarm.id == 0) {
        read_disarming(SQLITE_FILE_NAME, &disarm);
        if (!sdkp2p_get_section_info(buf, "linkageAction=", sValue, sizeof(sValue))) {
            disarm.linkageAction = atoi(sValue);
        }

        // if (!sdkp2p_get_section_info(buf, "disarmChnMask=", sValue, sizeof(sValue))) {
        //     alarm.disarmChnMask = sdkp2p_get_chnmask_to_atoll(sValue);
        // }
        if (!sdkp2p_get_section_info(buf, "disarmActionMask=", sValue, sizeof(sValue))) {
            disarm.disarmActionMask = atoi(sValue);
        }
        write_disarming(SQLITE_FILE_NAME, &disarm);
    }

    write_alarm_in(SQLITE_FILE_NAME, &alarm);

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMINPUT, alarm.id + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}


static void req_event_get_holidays(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i, nCnt = 0;
    struct holiday holiday[MAX_CAMERA];
    char encode[512] = {0};

    memset(holiday, 0, sizeof(struct holiday)*MAX_CAMERA);
    read_holidays(SQLITE_FILE_NAME, holiday, &nCnt);

    for (i = 0; i < nCnt; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch[%d]=%d&", i, holiday[i].id);
        get_url_encode(holiday[i].name, strlen(holiday[i].name), encode, sizeof(encode));
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "name[%d]=%s&", i, encode);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable[%d]=%d&", i, holiday[i].enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "type[%d]=%d&", i, holiday[i].type);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_year[%d]=%d&", i,
                 holiday[i].start_year);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_mon[%d]=%d&", i,
                 holiday[i].start_mon);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_mday[%d]=%d&", i,
                 holiday[i].start_mday);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_mweek[%d]=%d&", i,
                 holiday[i].start_mweek);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_wday[%d]=%d&", i,
                 holiday[i].start_wday);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_year[%d]=%d&", i, holiday[i].end_year);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_mon[%d]=%d&", i, holiday[i].end_mon);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_mday[%d]=%d&", i, holiday[i].end_mday);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_mweek[%d]=%d&", i,
                 holiday[i].end_mweek);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_wday[%d]=%d&", i, holiday[i].end_wday);
    }

    *(conf->len) = strlen(conf->resp) + 1;
}


static void req_event_get_osd(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct osd osds[MAX_CAMERA];
    int i, cnt = 0;
    char encode[256] = {0};

    memset(osds, 0, sizeof(osds));
    read_osds(SQLITE_FILE_NAME, osds, &cnt);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cnt=%d&", cnt);
    for (i = 0; i < cnt; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch[%d]=%d&", i, osds[i].id);
        get_url_encode(osds[i].name, strlen(osds[i].name), encode, sizeof(encode));
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "name[%d]=%s&", i, encode);
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void get_motion_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.ch=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }
    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void set_motion_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_sched_motion *res = (struct req_sched_motion *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.ch=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->ch));
    }
    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->enable));
    }
    if (!sdkp2p_get_section_info(buf, "r.tri_channels=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%u", &(res->tri_channels));
    }
    if (!sdkp2p_get_section_info(buf, "r.tri_alarms=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%u", &(res->tri_alarms));
    }
    *datalen = sizeof(struct req_sched_motion);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_event_get_videoloss(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = 0;
    char sValue[256] = {0};
    struct video_loss loss;

    if (!sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    read_video_lost(SQLITE_FILE_NAME, &loss, chnid);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch=%d&", loss.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", loss.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_alarms=%d&", loss.tri_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "buzzer_interval=%d&", loss.buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&",
             loss.email_buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&", loss.ptzaction_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "alarmout_interval=%d&",
             loss.alarmout_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarms1=%s&",
             loss.tri_chnout1_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarms2=%s&",
             loss.tri_chnout2_alarms);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_event_set_videoloss(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    struct video_loss loss = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        loss.id = atoi(sValue);
    }
    *res = loss.id;
    read_video_lost(SQLITE_FILE_NAME, &loss, loss.id);

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        loss.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_alarms=", sValue, sizeof(sValue))) {
        loss.tri_alarms = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "buzzer_interval=", sValue, sizeof(sValue))) {
        loss.buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "email_interval=", sValue, sizeof(sValue))) {
        loss.email_buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        loss.ptzaction_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "alarmout_interval=", sValue, sizeof(sValue))) {
        loss.alarmout_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarms1=", sValue, sizeof(sValue))) {
        snprintf(loss.tri_chnout1_alarms, sizeof(loss.tri_chnout1_alarms), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarms2=", sValue, sizeof(sValue))) {
        snprintf(loss.tri_chnout2_alarms, sizeof(loss.tri_chnout2_alarms), "%s", sValue);
    }

    write_video_lost(SQLITE_FILE_NAME, &loss);
    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);

    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_VIDEOLOSS, loss.id + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_event_set_holidays(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[2048] = {0};
    struct holiday holiday = {0};
    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        holiday.id = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        holiday.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(holiday.name, sizeof(holiday.name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        holiday.type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "start_year=", sValue, sizeof(sValue))) {
        holiday.start_year = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "start_mon=", sValue, sizeof(sValue))) {
        holiday.start_mon = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "start_mday=", sValue, sizeof(sValue))) {
        holiday.start_mday = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "start_mweek=", sValue, sizeof(sValue))) {
        holiday.start_mweek = (int)atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "start_wday=", sValue, sizeof(sValue))) {
        holiday.start_wday = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "end_year=", sValue, sizeof(sValue))) {
        holiday.end_year = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "end_mon=", sValue, sizeof(sValue))) {
        holiday.end_mon = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "end_mday=", sValue, sizeof(sValue))) {
        holiday.end_mday = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "end_mweek=", sValue, sizeof(sValue))) {
        holiday.end_mweek = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "end_wday=", sValue, sizeof(sValue))) {
        holiday.end_wday = atol(sValue);
    }

    write_holiday(SQLITE_FILE_NAME, &holiday);
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_HOLIDAY, holiday.id + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_event_set_osd(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct osd osd = {0};
    int chnid = -1;
    struct req_set_osdparam req_osd = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
        if (chnid == -1) {
            return;
        }
    }

    read_osd(SQLITE_FILE_NAME, &osd, chnid);
    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(osd.name, sizeof(osd.name), "%s", sValue);
    }
    write_osd(SQLITE_FILE_NAME, &osd);
    req_osd.type = 1;
    req_osd.chanid = osd.id;

    sdkp2p_send_msg(conf, conf->req, &req_osd, sizeof(req_osd), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_event_get_motion_map(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};

    *res = 0;
    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void req_event_set_motion_map(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, j = 0;
    long long batch_flag = 0;
    int chnid = 0, num = 0;
    int change_flag = 0;
    /*param start*/
    int enable = 0;
    int sensitivity = 1;;
    unsigned int tri_channels = 0;
    unsigned int tri_alarms = 0;
    unsigned char motion_table[MAX_MOTION_CELL] = {0};
    int buzzer_interval = 0;
    int ipc_sched_enable = 0;
    char tri_channels_ex[66] = {0};
    /*param end*/
    char sValue[512] = {0};
    struct motion motions[MAX_CAMERA];
    struct req_set_motionmap motionmap;
    struct req_set_motionsce tmp;
    struct req_enable_motion enableMotion;
    memset(&motionmap, 0, sizeof(struct req_set_motionmap));
    memset(&enableMotion, 0x0, sizeof(struct req_enable_motion));

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    memset(motions, 0x0, sizeof(struct motion) * MAX_CAMERA);
    read_motions(SQLITE_FILE_NAME, motions, &num);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {

        change_flag |= 1 << 0;
        enable = atoi(sValue);
        motions[chnid].enable = enable;
    }

    if (!sdkp2p_get_section_info(buf, "alarmout_interval=", sValue, sizeof(sValue))) {
        motions[chnid].alarmout_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "tri_chnout1_alarms=", sValue, sizeof(sValue))) {
        snprintf(motions[chnid].tri_chnout1_alarms, sizeof(motions[chnid].tri_chnout1_alarms), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "tri_chnout2_alarms=", sValue, sizeof(sValue))) {
        snprintf(motions[chnid].tri_chnout2_alarms, sizeof(motions[chnid].tri_chnout2_alarms), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "tri_alarms=", sValue, sizeof(sValue))) {
        change_flag |= 1 << 1;
        tri_alarms = (unsigned int)atoll(sValue);
        motions[chnid].tri_alarms = tri_alarms;
    }

    if (!sdkp2p_get_section_info(buf, "tri_channels=", sValue, sizeof(sValue))) {
        change_flag |= 1 << 2;
        tri_channels = (unsigned int)atoll(sValue);
        motions[chnid].tri_channels = tri_channels;
    }

    memset(tri_channels_ex, 0x0, sizeof(tri_channels_ex));
    //tri_channels = 0;
    memset(sValue, 0, sizeof(sValue));
    if (!sdkp2p_get_section_info(buf, "tri_channels_ex=", sValue, sizeof(sValue))) {
        change_flag |= 1 << 3;
        snprintf(tri_channels_ex, sizeof(tri_channels_ex), "%s", sValue);
        tri_channels = 0;
        for (i = 0; i < strlen(tri_channels_ex); i++) {
            if (tri_channels_ex[i] == '1') {
                tri_channels |= (1 << i);
            }
        }
        snprintf(motions[chnid].tri_channels_ex, sizeof(motions[chnid].tri_channels_ex), "%s", tri_channels_ex);
        motions[chnid].tri_channels = tri_channels;
    } else {
        change_flag |= 1 << 4;
        for (i = 0; i < 32; i++) {
            if (tri_channels & (1 << i)) {
                tri_channels_ex[i] = '1';
            } else {
                tri_channels_ex[i] = '0';
            }
        }
        snprintf(motions[chnid].tri_channels_ex, sizeof(motions[i].tri_channels_ex), "%s", tri_channels_ex);
    }

    if (!sdkp2p_get_section_info(buf, "sensitivity=", sValue, sizeof(sValue))) {
        change_flag |= 1 << 5;
        sensitivity = atoi(sValue);
        motions[chnid].sensitivity = sensitivity;
    }

    if (!sdkp2p_get_section_info(buf, "buzzer_interval=", sValue, sizeof(sValue))) {
        change_flag |= 1 << 6;
        buzzer_interval = atoi(sValue);
        motions[chnid].buzzer_interval = buzzer_interval;
    }

    if (!sdkp2p_get_section_info(buf, "ipc_sched_enable=", sValue, sizeof(sValue))) {
        change_flag |= 1 << 7;
        ipc_sched_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "motion_table=", sValue, sizeof(sValue))) {
        change_flag |= 1 << 8;
        snprintf((char *)motion_table, sizeof(motion_table), "%s", sValue);
        snprintf((char *)motions[chnid].motion_table, sizeof(motions[chnid].motion_table), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }

    if (change_flag) {
        write_motion(SQLITE_FILE_NAME, &motions[chnid]);
        sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    }

    batch_flag |= ((UInt64)1 << chnid);
    if (batch_flag != 0) {
        for (i = 0; i < MAX_CAMERA; i++) {
            if (!(batch_flag >> i & 0x01)) {
                continue;
            }

            if (change_flag & 0x01) {
                motions[i].enable = enable;
            }

            if (change_flag >> 1 & 0x01) {
                motions[i].tri_alarms = tri_alarms;
            }
#if 0
            if (change_flag >> 2 & 0x01) {
                motions[i].tri_channels = tri_channels;
            }

            if (change_flag >> 3 & 0x01) {
                snprintf(motions[i].tri_channels_ex, sizeof(motions[i].tri_channels_ex), "%s", tri_channels_ex);
                motions[i].tri_channels = tri_channels;
            }

            if (change_flag >> 4 & 0x01) {
                snprintf(motions[i].tri_channels_ex, sizeof(motions[i].tri_channels_ex), "%s", tri_channels_ex);
            }
#endif

            if (change_flag >> 5 & 0x01) {
                motions[i].sensitivity = sensitivity;
            }

            if (change_flag >> 6 & 0x01) {
                motions[i].buzzer_interval = buzzer_interval;
            }

            if (change_flag >> 7 & 0x01) {
                motions[i].ipc_sched_enable = ipc_sched_enable;
            }

            if (change_flag >> 8 & 0x01) {
                snprintf((char *)motions[i].motion_table, sizeof(motions[i].motion_table), "%s", motion_table);
            }
        }

        write_motions(SQLITE_FILE_NAME, motions, MAX_CAMERA, batch_flag);
        sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION_BATCH, &batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    }

    if (ipc_sched_enable == 0) {
        memset(&tmp, 0, sizeof(tmp));
        for (i = 0; i < 7; i++) {
            tmp.scedule[i].day = i;
            tmp.scedule[i].onoff = 1;
            for (j = 0; j < 4; j++) {
                tmp.scedule[i].tmdetail[j].begin_hour = 0;
                tmp.scedule[i].tmdetail[j].begin_minute = 0;
                tmp.scedule[i].tmdetail[j].end_hour = 24;
                tmp.scedule[i].tmdetail[j].end_minute = 0;
            }
        }
        for (i = 0; i < MAX_CAMERA; i++) {
            if (batch_flag >> i & 0x01) {
                tmp.id[tmp.size] = i;
                tmp.size++;
                enableMotion.id[enableMotion.size] = i;
                enableMotion.size++;
            }
        }
        enableMotion.enable = 1;
        sdkp2p_send_msg(conf, REQUEST_FLAG_OVF_ENABLE_MOT, &enableMotion, sizeof(struct req_enable_motion),
                        SDKP2P_NOT_CALLBACK);
        sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPCMTSCE, &tmp, sizeof(struct req_set_motionsce), SDKP2P_NOT_CALLBACK);
    } else {
        for (i = 0; i < MAX_CAMERA; i++) {
            if (batch_flag >> i & 0x01) {
                enableMotion.id[enableMotion.size] = i;
                enableMotion.size++;
            }
        }
        enableMotion.enable = 0;
        sdkp2p_send_msg(conf, REQUEST_FLAG_OVF_ENABLE_MOT, &enableMotion, sizeof(struct req_enable_motion),
                        SDKP2P_NOT_CALLBACK);
    }

    motionmap.enable = ipc_sched_enable;
    motionmap.sensitivity = sensitivity;
    snprintf(motionmap.mapbuf, sizeof(motionmap.mapbuf), "%s", motion_table);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (batch_flag >> i & 0x01) {
            motionmap.id[motionmap.size] = i;
            motionmap.size++;
        }
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPCMTMAP, &motionmap, sizeof(struct req_set_motionmap), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log_batch(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_MOTION, batch_flag);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}


static void resp_event_get_motion_map(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_get_motionmap *res = (struct resp_get_motionmap *)param;
    struct motion motion;

    read_motion(SQLITE_FILE_NAME, &motion, res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "result=%d&", res->result);
    snprintf(buf + strlen(buf), len - strlen(buf), "id=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", motion.enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "tri_alarms=%u&", motion.tri_alarms);
    snprintf(buf + strlen(buf), len - strlen(buf), "tri_channels=%u&", motion.tri_channels);
    snprintf(buf + strlen(buf), len - strlen(buf), "tri_channels_ex=%s&", motion.tri_channels_ex);
    snprintf(buf + strlen(buf), len - strlen(buf), "motion_table=%s&", res->mapbuf);
    snprintf(buf + strlen(buf), len - strlen(buf), "sensitivity=%d&", res->sensitivity);
    snprintf(buf + strlen(buf), len - strlen(buf), "buzzer_interval=%d&", motion.buzzer_interval);
    snprintf(buf + strlen(buf), len - strlen(buf), "ipc_sched_enable=%d&", res->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "transfer=%d&", res->transfer);
    snprintf(buf + strlen(buf), len - strlen(buf), "install=%d&", res->install);
    snprintf(buf + strlen(buf), len - strlen(buf), "display=%d&", res->fishdisplay);
    snprintf(buf + strlen(buf), len - strlen(buf), "modelType=%d&", res->modelType);
    snprintf(buf + strlen(buf), len - strlen(buf), "alarmout_interval=%d&", motion.alarmout_interval);
    snprintf(buf + strlen(buf), len - strlen(buf), "triChnAlarms1=%s&", motion.tri_chnout1_alarms);
    snprintf(buf + strlen(buf), len - strlen(buf), "triChnAlarms2=%s&", motion.tri_chnout2_alarms);

    *datalen = strlen(buf) + 1;
}

#if 0
static void resp_event_set_motion_map(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}
#endif

static void get_ipcmtsce_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_motionscedule;
    int i, j;
    struct resp_get_motionscedule *res = (struct resp_get_motionscedule *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chanid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.result=%d&", res->result);

    for (i = 0; i < 7; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.scedule[%d].msstr=%s&", i, res->scedule[i].msstr);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.scedule[%d].day=%d&", i, res->scedule[i].day);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.scedule[%d].onoff=%d&", i, res->scedule[i].onoff);
        for (j = 0; j < 4; j++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r.scedule[%d].tmdetail[%d].begin_hour=%d&", i, j,
                     res->scedule[i].tmdetail[j].begin_hour);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.scedule[%d].tmdetail[%d].begin_minute=%d&", i, j,
                     res->scedule[i].tmdetail[j].begin_minute);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.scedule[%d].tmdetail[%d].end_hour=%d&", i, j,
                     res->scedule[i].tmdetail[j].end_hour);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.scedule[%d].tmdetail[%d].end_minute=%d&", i, j,
                     res->scedule[i].tmdetail[j].end_minute);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void set_ipcmtsce_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int ret;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_event_get_except_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct trigger_alarms info = {0};
    read_trigger_alarms(SQLITE_FILE_NAME, &info);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "network_disconn=%d&", info.network_disconn);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "disk_full=%d&", info.disk_full);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "record_fail=%d&", info.record_fail);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "disk_fail=%d&", info.disk_fail);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "disk_unformat=%d&", info.disk_unformat);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "no_disk=%d&", info.no_disk);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "record_mail_interval=%d&", 
        info.record_mail_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ipConflict=%d&", info.ipConflict);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "diskOffline=%d&", info.diskOffline);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_event_set_except_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct trigger_alarms info = {0};

    if (!sdkp2p_get_section_info(buf, "network_disconn=", sValue, sizeof(sValue))) {
        info.network_disconn = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "disk_full=", sValue, sizeof(sValue))) {
        info.disk_full = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "record_fail=", sValue, sizeof(sValue))) {
        info.record_fail = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "disk_fail=", sValue, sizeof(sValue))) {
        info.disk_fail = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "disk_unformat=", sValue, sizeof(sValue))) {
        info.disk_unformat = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "no_disk=", sValue, sizeof(sValue))) {
        info.no_disk = atoi(sValue);
    }

    info.record_mail_interval = !sdkp2p_get_section_info(buf, "record_mail_interval=", sValue, sizeof(sValue)) 
        ? atoi(sValue) : MS_INVALID_VALUE;
    info.ipConflict = (!sdkp2p_get_section_info(buf, "ipConflict=", sValue, sizeof(sValue)) ? atoi(sValue) : MS_INVALID_VALUE);
    info.diskOffline = (!sdkp2p_get_section_info(buf, "diskOffline=", sValue, sizeof(sValue)) ? atoi(sValue) : MS_INVALID_VALUE);

    write_trigger_alarms(SQLITE_FILE_NAME, &info);
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}


static void resp_event_get_exception(void *param, int size, char *buf, int len, int *datalen)
{
    int iCnt;
    char curtime[32] = {0};
    long sec = 0;
    int cnt = 0;
    struct tm *p_tm = NULL;
    struct resp_get_exception *exception = (struct resp_get_exception *)param;

    cnt = size / sizeof(struct resp_get_exception);
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", cnt);
    for (iCnt = 0; iCnt < cnt; iCnt++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "event[%d]=%s&", iCnt, exception[iCnt].pEvent);
        snprintf(buf + strlen(buf), len - strlen(buf), "information[%d]=%s&", iCnt, exception[iCnt].pInformation);
        sec = exception[iCnt].time;
        p_tm = localtime(&sec);
        strftime(curtime, sizeof(curtime), "%Y-%m-%d %H:%M:%S", p_tm);
        snprintf(buf + strlen(buf), len - strlen(buf), "time[%d]=%s&", iCnt, curtime);
    }

    *datalen = strlen(buf) + 1;
}

static void req_disk_get_maintain_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct disk_maintain_info info = { 0 };

    read_disk_maintain_info(SQLITE_FILE_NAME, &info);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "log=%d&", info.log);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "photo=%d&", info.photo);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void set_disk_maintainance_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_disk_maintain_info *res = (struct req_disk_maintain_info *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.log=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->log));
    }
    if (!sdkp2p_get_section_info(buf, "r.photo=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->photo));
    }

    *datalen = sizeof(struct req_disk_maintain_info);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_event_get_lpr_list(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0;
    int licenseCount = 0;
    char encode[256] = {0};
    struct anpr_list *license_array = NULL;
    struct anpr_list search;

    memset(&search, 0x0, sizeof(struct anpr_list));
    if (!sdkp2p_get_section_info(buf, "plate=", sValue, sizeof(sValue))) {
        snprintf(search.plate, sizeof(search.plate), "%s", sValue);
    }

    if (sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        snprintf(search.type, sizeof(search.type), "%s", sValue);
    } else {
        snprintf(search.type, sizeof(search.type), "%s", "All");
    }

    read_anpr_lists(SQLITE_ANPR_NAME, &license_array, &licenseCount);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "allCnt=%d&", licenseCount);
    for (i = 0; i < licenseCount; ++i) {
        if (license_array) {
            get_url_encode(license_array[i].plate, strlen(license_array[i].plate), encode, sizeof(encode));
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "p%d=%s&", i, encode);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "t%d=%s&", i, license_array[i].type);
        }
    }

    if (license_array) {
        release_anpr_lists(&license_array);
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_event_add_lpr_list(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[32] = {0};
    int cnt = 0;
    int realCnt = 0;
    int i = 0;
    struct anpr_list *anpr_info;

    if (sdkp2p_get_section_info(buf, "cnt=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    cnt = atoi(sValue);
    if (cnt <= 0 || cnt >= MAX_ANPR_LIST_COUNT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    anpr_info = ms_malloc(sizeof(struct anpr_list) * cnt);
    if (!anpr_info) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    for (i = 0; i < cnt; i++) {
        snprintf(tmp, sizeof(tmp), "type%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            continue;
        }
        if (strlen(sValue) == 0) {
            continue;
        }
        snprintf(anpr_info[realCnt].type, sizeof(anpr_info[realCnt].type), "%s", sValue);
        snprintf(tmp, sizeof(tmp), "plate%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            continue;
        }
        if (strlen(sValue) == 0) {
            continue;
        }
        if (strchr(sValue, ' ')) {
            continue;
        }
        get_url_decode(sValue);
        snprintf(anpr_info[realCnt].plate, sizeof(anpr_info[realCnt].plate), "%s", sValue);
        realCnt++;
    }

    if (realCnt <= 0) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        ms_free(anpr_info);
        return;
    }

    int anprListCount = read_anpr_list_cnt(SQLITE_ANPR_NAME);
    if (anprListCount >= MAX_ANPR_LIST_COUNT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        ms_free(anpr_info);
        return;
    }

    //msprintf("cnt:%d realCnt:%d current:%d", cnt, realCnt, anprListCount);

    if (anprListCount + realCnt > MAX_ANPR_LIST_COUNT) {
        realCnt = MAX_ANPR_LIST_COUNT - anprListCount;
    }

    write_anpr_list_replace_batch(SQLITE_ANPR_NAME, anpr_info, realCnt);
    ms_free(anpr_info);

    snprintf(conf->resp, conf->size, "res=0&count=%d&", realCnt);
    *(conf->len) = strlen(conf->resp) + 1;
    return ;
}

static void req_event_del_lpr_list(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct anpr_list *anpr_info;
    int cnt = 0;
    int realCnt = 0;
    int i = 0;
    char tmp[64] = {0};

    if (sdkp2p_get_section_info(buf, "cnt=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    cnt = atoi(sValue);
    if (cnt <= 0 || cnt > MAX_ANPR_LIST_COUNT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    anpr_info = ms_malloc(sizeof(struct anpr_list) * cnt);
    if (!anpr_info) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    for (i = 0; i < cnt; i++) {
        snprintf(tmp, sizeof(tmp), "plate%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            get_url_decode(sValue);
            snprintf(anpr_info[realCnt].plate, sizeof(anpr_info[realCnt].plate), "%s", sValue);
            realCnt++;
        }
    }

    if (realCnt <= 0) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        ms_free(anpr_info);
        return;
    }
    delete_anpr_lists(SQLITE_ANPR_NAME, anpr_info, realCnt);
    ms_free(anpr_info);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}


static void req_event_edit_lpr_list(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct anpr_list anpr_info;
    struct anpr_list old;

    //delete
    if (!sdkp2p_get_section_info(buf, "oldPlate=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(old.plate, sizeof(old.plate), "%s", sValue);
        if (strlen(old.plate)) {
            delete_anpr_list(SQLITE_ANPR_NAME, &old);
        }
    }

    memset(&anpr_info, 0x0, sizeof(struct anpr_list));
    if (sdkp2p_get_section_info(buf, "plate=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    get_url_decode(sValue);
    snprintf(anpr_info.plate, sizeof(anpr_info.plate), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    snprintf(anpr_info.type, sizeof(anpr_info.type), "%s", sValue);

    if (strlen(anpr_info.plate) == 0) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    if (strchr(anpr_info.plate, ' ')) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    int anprListCount = read_anpr_list_cnt(SQLITE_ANPR_NAME);
    if (anprListCount >= MAX_ANPR_LIST_COUNT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    struct anpr_list tmp;
    memset(&tmp, 0, sizeof(struct anpr_list));
    read_anpr_list_plate(SQLITE_ANPR_NAME, &tmp, anpr_info.plate);
    if (strlen(tmp.plate)) {
        update_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
    } else {
        write_anpr_list(SQLITE_ANPR_NAME, &anpr_info);
    }

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_event_get_lpr_mode_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[10 * 1024] = {0};
    int chnid = 0, mode = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "mode=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    mode = atoi(sValue);
    if (mode < ANPR_ENABLE || mode >= MAX_ANPR_MODE) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    SMART_SCHEDULE schedule;
    SMART_EVENT event;
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch=%d&", chnid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "mode=%d&", mode);
    if (mode == ANPR_ENABLE) {
        //only effective
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_anpr_effective_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), "e") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
    } else {
        //event
        memset(&event, 0x0, sizeof(SMART_EVENT));
        read_anpr_event(SQLITE_FILE_NAME, &event, chnid, mode);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", event.enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%d&", event.tri_alarms);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "buzzer_interval=%d&",
                 event.buzzer_interval);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&", event.email_interval);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_pic_enable=%d&",
                 event.email_pic_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_pic=%s&",
                 event.tri_channels_pic);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&",
                 event.ptzaction_interval);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "alarmout_interval=%d&",
                 event.alarmout_interval);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChannels=%s&", event.tri_channels_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarms1=%s&",
                 event.tri_chnout1_alarms);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarms2=%s&",
                 event.tri_chnout2_alarms);
        //effective
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_anpr_effective_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), "e") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
        //audible
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_anpr_audible_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), "a") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
        //mail
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_anpr_mail_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), "m") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
        //ptz
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_anpr_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), "p") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
    return ;
}

static void req_event_set_lpr_mode_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0, mode = 0;
    SMART_SCHEDULE schedule;
    SMART_EVENT event;
    int flag = 0;
    struct req_anpr_event info;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "mode=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    mode = atoi(sValue);
    if (mode < ANPR_ENABLE || mode >= MAX_ANPR_MODE) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    //effective
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "e") == 0) {
        write_anpr_effective_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
    }

    //audible
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "a") == 0) {
        write_anpr_audible_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
    }

    //mail
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "m") == 0) {
        write_anpr_mail_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
    }

    //ptz
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "p") == 0) {
        write_anpr_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid, mode);
    }

    //event
    memset(&event, 0x0, sizeof(SMART_EVENT));
    read_anpr_event(SQLITE_FILE_NAME, &event, chnid, mode);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        flag = 1;
        event.enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        flag = 1;
        event.enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
        flag = 1;
        event.tri_alarms = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "buzzer_interval=", sValue, sizeof(sValue))) {
        flag = 1;
        event.buzzer_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "email_interval=", sValue, sizeof(sValue))) {
        flag = 1;
        event.email_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "email_pic_enable=", sValue, sizeof(sValue))) {
        flag = 1;
        event.email_pic_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "tri_channels_pic=", sValue, sizeof(sValue))) {
        flag = 1;
        snprintf(event.tri_channels_pic, sizeof(event.tri_channels_pic), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        flag = 1;
        event.ptzaction_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "alarmout_interval=", sValue, sizeof(sValue))) {
        flag = 1;
        event.alarmout_interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChannels=", sValue, sizeof(sValue))) {
        flag = 1;
        snprintf(event.tri_channels_ex, sizeof(event.tri_channels_ex), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarms1=", sValue, sizeof(sValue))) {
        flag = 1;
        snprintf(event.tri_chnout1_alarms, sizeof(event.tri_chnout1_alarms), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarms2=", sValue, sizeof(sValue))) {
        flag = 1;
        snprintf(event.tri_chnout2_alarms, sizeof(event.tri_chnout2_alarms), "%s", sValue);
    }

    if (flag == 1) {
        write_anpr_event(SQLITE_FILE_NAME, &event, mode);
    }


    memset(&info, 0x0, sizeof(struct req_anpr_event));
    info.modeType = mode;
    info.chnid = chnid;
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_anpr_event), SDKP2P_NOT_CALLBACK);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return;
}

static void req_event_get_lpr_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_get_lpr_settings(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0, j = 0;
    struct ms_lpr_settings *info = (struct ms_lpr_settings *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", info->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "resolution_width=%d&", info->resolution_width);
    snprintf(buf + strlen(buf), len - strlen(buf), "resolution_height=%d&", info->resolution_height);
    snprintf(buf + strlen(buf), len - strlen(buf), "area_x=%d&", info->area_x);
    snprintf(buf + strlen(buf), len - strlen(buf), "area_y=%d&", info->area_y);
    snprintf(buf + strlen(buf), len - strlen(buf), "area_width=%d&", info->area_width);
    snprintf(buf + strlen(buf), len - strlen(buf), "area_height=%d&", info->area_height);
    snprintf(buf + strlen(buf), len - strlen(buf), "day_night_mode_enable=%d&", info->day_night_mode_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "scene=%d&", info->scene);
    snprintf(buf + strlen(buf), len - strlen(buf), "direction_enable=%d&", info->direction_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "region_enable=%d&", info->region_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "roiid_enable=%d&", info->roiid_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "process_mode=%d&", info->process_mode);
    snprintf(buf + strlen(buf), len - strlen(buf), "repeat_check_time_value=%d&", info->repeat_check_time_value);
    snprintf(buf + strlen(buf), len - strlen(buf), "repeat_check_time_unit=%d&", info->repeat_check_time_unit);
    snprintf(buf + strlen(buf), len - strlen(buf), "post_enable=%d&", info->post_enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "post_type=%d&", info->post_type);
    snprintf(buf + strlen(buf), len - strlen(buf), "post_tcp_port=%d&", info->post_tcp_port);
    snprintf(buf + strlen(buf), len - strlen(buf), "post_http_user=%s&", info->post_http_user);
    snprintf(buf + strlen(buf), len - strlen(buf), "post_http_password=%s&", info->post_http_password);
    snprintf(buf + strlen(buf), len - strlen(buf), "post_http_url=%s&", info->post_http_url);
    snprintf(buf + strlen(buf), len - strlen(buf), "confidence=%d&", info->confidence);
    for (i = 0; i < MAX_LEN_32; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "region[%d]=%s&", i, info->area_list[i]);
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "cur_area=%s&", info->cur_area);

    for (i = 0; i < 4; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "roi[%d].name=%s&", i, info->roi[i].name);
        snprintf(buf + strlen(buf), len - strlen(buf), "roi[%d].startX=%d&", i, info->roi[i].startX);
        snprintf(buf + strlen(buf), len - strlen(buf), "roi[%d].startY=%d&", i, info->roi[i].startY);
        snprintf(buf + strlen(buf), len - strlen(buf), "roi[%d].endX=%d&", i, info->roi[i].endX);
        snprintf(buf + strlen(buf), len - strlen(buf), "roi[%d].endY=%d&", i, info->roi[i].endY);
    }

    struct ms_lpr_position_info *one = NULL;
    for (i = 0; i < 4; i++) {
        if (i == 0) {
            one = &(info->preset1[0]);
        } else if (i == 1) {
            one = &(info->preset2[0]);
        } else if (i == 2) {
            one = &(info->preset3[0]);
        } else if (i == 3) {
            one = &(info->preset4[0]);
        }

        for (j = 0; j < 4; j++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "preset[%d][%d].name=%s&", i, j, one[i].name);
            snprintf(buf + strlen(buf), len - strlen(buf), "preset[%d][%d].startX=%d&", i, j, one[i].startX);
            snprintf(buf + strlen(buf), len - strlen(buf), "preset[%d][%d].startY=%d&", i, j, one[i].startY);
            snprintf(buf + strlen(buf), len - strlen(buf), "preset[%d][%d].endX=%d&", i, j, one[i].endX);
            snprintf(buf + strlen(buf), len - strlen(buf), "preset[%d][%d].endY=%d&", i, j, one[i].endY);
        }
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "license_chnid=%d&", info->license.chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "license_value=%s&", info->license.license_value);
    snprintf(buf + strlen(buf), len - strlen(buf), "license_status=%d&", info->license.license_status);
    snprintf(buf + strlen(buf), len - strlen(buf), "night_mode_enable=%d&", info->night_mode_info.enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "night_mode_level=%d&", info->night_mode_info.level);
    snprintf(buf + strlen(buf), len - strlen(buf), "night_mode_starthour=%d&", info->night_mode_info.starthour);
    snprintf(buf + strlen(buf), len - strlen(buf), "night_mode_startminute=%d&", info->night_mode_info.startminute);
    snprintf(buf + strlen(buf), len - strlen(buf), "night_mode_stophour=%d&", info->night_mode_info.stophour);
    snprintf(buf + strlen(buf), len - strlen(buf), "night_mode_stopminute=%d&", info->night_mode_info.stopminute);
    snprintf(buf + strlen(buf), len - strlen(buf), "night_mode_effective_mode=%d&", info->night_mode_info.effective_mode);
    snprintf(buf + strlen(buf), len - strlen(buf), "in_night_mode=%d&", info->in_night_mode);

    snprintf(buf + strlen(buf), len - strlen(buf), "platecolorEnable=%d&", info->platecolorEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "vehicletypeEnable=%d&", info->vehicletypeEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "vehiclecolorEnable=%d&", info->vehiclecolorEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "jsonSupport=%d&", info->jsonSupport);
    snprintf(buf + strlen(buf), len - strlen(buf), "irLedExternalSupport=%d&", info->irLedExternalSupport);
    snprintf(buf + strlen(buf), len - strlen(buf), "vehicleEnhancement=%d&", info->vehicleEnhancement);
    snprintf(buf + strlen(buf), len - strlen(buf), "vehicleBrandEnable=%d&", info->vehicleBrandEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "aitype=%d&", info->aitype);

    if (info->jsonSupport && sdk_is_ms_lpr(info->aitype)) {
        for (i = 0; i < MAX_SMART_REGION; ++i) {
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "normalPolygons[%d].name=%s&", i, 
                info->normalPolygons[i].name);
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "normalPolygons[%d].polygonX=%s&", i, 
                info->normalPolygons[i].polygonX);
            snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "normalPolygons[%d].polygonY=%s&", i, 
                info->normalPolygons[i].polygonY);
        }
    }
    
    *datalen = strlen(buf) + 1;
}

static void req_event_set_lpr_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[256] = {0};
    int i = 0, j = 0;
    struct ms_lpr_settings info;

    memset(&info, 0x0, sizeof(struct ms_lpr_settings));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    info.chanid = atoi(sValue);
    if (info.chanid < 0 || info.chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        info.enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "resolution_width=", sValue, sizeof(sValue))) {
        info.resolution_width = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "resolution_height=", sValue, sizeof(sValue))) {
        info.resolution_height = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "area_x=", sValue, sizeof(sValue))) {
        info.area_x = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "area_y=", sValue, sizeof(sValue))) {
        info.area_y = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "area_width=", sValue, sizeof(sValue))) {
        info.area_width = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "area_height=", sValue, sizeof(sValue))) {
        info.area_height = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "day_night_mode_enable=", sValue, sizeof(sValue))) {
        info.day_night_mode_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "scene=", sValue, sizeof(sValue))) {
        info.scene = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "direction_enable=", sValue, sizeof(sValue))) {
        info.direction_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "region_enable=", sValue, sizeof(sValue))) {
        info.region_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "roiid_enable=", sValue, sizeof(sValue))) {
        info.roiid_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "process_mode=", sValue, sizeof(sValue))) {
        info.process_mode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "repeat_check_time_value=", sValue, sizeof(sValue))) {
        info.repeat_check_time_value = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "repeat_check_time_unit=", sValue, sizeof(sValue))) {
        info.repeat_check_time_unit = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "post_enable=", sValue, sizeof(sValue))) {
        info.post_enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "post_type=", sValue, sizeof(sValue))) {
        info.post_type = atoi(sValue);
    }


    if (!sdkp2p_get_section_info(buf, "post_tcp_port=", sValue, sizeof(sValue))) {
        info.post_tcp_port = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "post_http_user=", sValue, sizeof(sValue))) {
        snprintf(info.post_http_user, sizeof(info.post_http_user), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "post_http_password=", sValue, sizeof(sValue))) {
        snprintf(info.post_http_password, sizeof(info.post_http_password), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "post_http_url=", sValue, sizeof(sValue))) {
        snprintf(info.post_http_url, sizeof(info.post_http_url), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "confidence=", sValue, sizeof(sValue))) {
        info.confidence = atoi(sValue);
    }
    //region
    for (i = 0; i < MAX_LEN_32; i++) {
        snprintf(tmp, sizeof(tmp), "region[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(info.area_list[i], sizeof(info.area_list[i]), "%s", sValue);
        }
    }
    if (!sdkp2p_get_section_info(buf, "cur_area=", sValue, sizeof(sValue))) {
        snprintf(info.cur_area, sizeof(info.cur_area), "%s", sValue);
    }

    //roi
    for (i = 0; i < 4; i++) {
        snprintf(tmp, sizeof(tmp), "roi[%d].name=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(info.roi[i].name, sizeof(info.roi[i].name), "%s", sValue);
        }

        snprintf(tmp, sizeof(tmp), "roi[%d].startX=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.roi[i].startX = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "roi[%d].startY=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.roi[i].startY = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "roi[%d].endX=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.roi[i].endX = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "roi[%d].endY=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.roi[i].endY = atoi(sValue);
        }
    }

    struct ms_lpr_position_info *one = NULL;
    //present
    for (i = 0; i < 4; i++) {
        if (i == 0) {
            one = &(info.preset1[0]);
        } else if (i == 1) {
            one = &(info.preset2[0]);
        } else if (i == 2) {
            one = &(info.preset3[0]);
        } else if (i == 3) {
            one = &(info.preset4[0]);
        }

        for (j = 0; j < 4; j++) {
            snprintf(tmp, sizeof(tmp), "preset[%d][%d].name=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(one[i].name, sizeof(one[i].name), "%s", sValue);
            }

            snprintf(tmp, sizeof(tmp), "preset[%d][%d].startX=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                one[i].startX = atoi(sValue);
            }

            snprintf(tmp, sizeof(tmp), "preset[%d][%d].startY=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                one[i].startY = atoi(sValue);
            }

            snprintf(tmp, sizeof(tmp), "preset[%d][%d].endX=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                one[i].endX = atoi(sValue);
            }

            snprintf(tmp, sizeof(tmp), "preset[%d][%d].endY=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                one[i].endY = atoi(sValue);
            }
        }
    }

    //license
    if (!sdkp2p_get_section_info(buf, "license_chnid=", sValue, sizeof(sValue))) {
        info.license.chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "license_value=", sValue, sizeof(sValue))) {
        snprintf(info.license.license_value, sizeof(info.license.license_value), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "license_status=", sValue, sizeof(sValue))) {
        info.license.license_status = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "night_mode_enable=", sValue, sizeof(sValue))) {
        info.night_mode_info.enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "night_mode_level=", sValue, sizeof(sValue))) {
        info.night_mode_info.level = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "night_mode_starthour=", sValue, sizeof(sValue))) {
        info.night_mode_info.starthour = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "night_mode_startminute=", sValue, sizeof(sValue))) {
        info.night_mode_info.startminute = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "night_mode_stophour=", sValue, sizeof(sValue))) {
        info.night_mode_info.stophour = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "night_mode_stopminute=", sValue, sizeof(sValue))) {
        info.night_mode_info.stopminute = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "night_mode_effective_mode=", sValue, sizeof(sValue))) {
        info.night_mode_info.effective_mode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "platecolorEnable=", sValue, sizeof(sValue))) {
        info.platecolorEnable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "vehicletypeEnable=", sValue, sizeof(sValue))) {
        info.vehicletypeEnable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "vehiclecolorEnable=", sValue, sizeof(sValue))) {
        info.vehiclecolorEnable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "vehicleEnhancement=", sValue, sizeof(sValue))) {
        info.vehicleEnhancement = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "vehicleBrandEnable=", sValue, sizeof(sValue))) {
        info.vehicleBrandEnable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "jsonSupport=", sValue, sizeof(sValue))) {
        info.jsonSupport = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "aitype=", sValue, sizeof(sValue))) {
        info.aitype = atoi(sValue);
    }

    if (info.jsonSupport && sdk_is_ms_lpr(info.aitype)) {
        for (i = 0; i < MAX_SMART_REGION; ++i) {
            snprintf(tmp, sizeof(tmp), "normalPolygons[%d].name=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(info.normalPolygons[i].name, sizeof(info.normalPolygons[i].name), sValue);
            }
            snprintf(tmp, sizeof(tmp), "normalPolygons[%d].polygonX=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(info.normalPolygons[i].polygonX, sizeof(info.normalPolygons[i].polygonX), sValue);
            }
            snprintf(tmp, sizeof(tmp), "normalPolygons[%d].polygonY=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(info.normalPolygons[i].polygonY, sizeof(info.normalPolygons[i].polygonY), sValue);
            }
        }
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct ms_lpr_settings), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_event_set_lpr_settings(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_event_get_lpr_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_get_lpr_support(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_lpr_support_info *res = (struct ms_lpr_support_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "vca_support=%d&", res->vca_support);
    snprintf(buf + strlen(buf), len - strlen(buf), "vca_version=%d&", res->vca_version);
    *datalen = strlen(buf) + 1;
    return ;
}

static void req_event_get_camera_alarmin(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_ipc_alarm_in reqPrm;
    memset(&reqPrm, 0, sizeof(reqPrm));

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    reqPrm.chanid = atoi(sValue);
    if (reqPrm.chanid < 0 || reqPrm.chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (!sdkp2p_get_section_info(buf, "maxSupport=", sValue, sizeof(sValue))) {
        reqPrm.alarmCnt = atoi(sValue);
    } else {
        reqPrm.alarmCnt = 2;
    }

    sdkp2p_send_msg(conf, conf->req, &reqPrm, sizeof(struct ms_ipc_alarm_in), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_get_camera_alarmin(void *param, int size, char *buf, int len, int *datalen)
{
    char tmp[10 * 1024] = {0};
    char mask[32] = {0};
    int i = 0;
    struct ms_ipc_alarm_in *res = (struct ms_ipc_alarm_in *)param;
    struct alarm_chn alarmInfo;
    struct alarm_chn_name alarmName;
    SMART_EVENT alarmEvent;
    SMART_SCHEDULE schedule;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "alarmCnt=%d&", res->alarmCnt);

    alarmInfo.chnid = res->chanid;
    for (i = 0; i < res->alarmCnt && i < MAX_IPC_ALARM_IN; i++) {
        alarmInfo.alarmid = i;
        snprintf(buf + strlen(buf), len - strlen(buf), "enable%d=%d&", i, res->alarmEnable[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "type%d=%d&", i, res->alarmType[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "status%d=%d&", i, res->alarmStatus[i]);

        memset(&alarmEvent, 0x0, sizeof(SMART_EVENT));
        read_alarm_chnIn_event(SQLITE_FILE_NAME, &alarmEvent, &alarmInfo);


        //name
        memset(&alarmName, 0x0, sizeof(struct alarm_chn_name));
        alarmName.chnid = res->chanid;
        read_alarm_chnIn_event_name(SQLITE_FILE_NAME, &alarmName, &alarmInfo);
        snprintf(buf + strlen(buf), len - strlen(buf), "name%d=%s&", i, alarmName.name);

        //effective
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_effective_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        snprintf(mask, sizeof(mask), "e%d", i);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), mask) == 0) {
            snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
        }

        //audible
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_audible_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        snprintf(mask, sizeof(mask), "a%d", i);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), mask) == 0) {
            snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
        }
        snprintf(buf + strlen(buf), len - strlen(buf), "buzzer_interval%d=%d&", i, alarmEvent.buzzer_interval);

        //email
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_mail_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        snprintf(mask, sizeof(mask), "m%d", i);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), mask) == 0) {
            snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
        }
        snprintf(buf + strlen(buf), len - strlen(buf), "email_interval%d=%d&", i, alarmEvent.email_interval);
        snprintf(buf + strlen(buf), len - strlen(buf), "email_pic_enable%d=%d&", i, alarmEvent.email_pic_enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "tri_channels_pic%d=%s&", i, alarmEvent.tri_channels_pic);

        //ptz
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_ptz_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        snprintf(mask, sizeof(mask), "p%d", i);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), mask) == 0) {
            snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
        }
        snprintf(buf + strlen(buf), len - strlen(buf), "triAlarms%d=%d&", i, alarmEvent.tri_alarms);
        snprintf(buf + strlen(buf), len - strlen(buf), "triChannels%d=%s&", i, alarmEvent.tri_channels_ex);
        snprintf(buf + strlen(buf), len - strlen(buf), "ptz_interval%d=%d&", i, alarmEvent.ptzaction_interval);

        snprintf(buf + strlen(buf), len - strlen(buf), "alarmout_interval%d=%d&", i, alarmEvent.alarmout_interval);
        snprintf(buf + strlen(buf), len - strlen(buf), "tri_chnout1_alarms%d=%s&", i, alarmEvent.tri_chnout1_alarms);
        snprintf(buf + strlen(buf), len - strlen(buf), "tri_chnout2_alarms%d=%s&", i, alarmEvent.tri_chnout2_alarms);
    }
    *datalen = strlen(buf) + 1;
    return ;
}

static void req_event_set_camera_alarmin(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char mask[128] = {0};
    long long batch = 0;
    int chnid = 0;
    int change = 0;
    int alarmCnt = 0;
    int i = 0;
    SMART_EVENT amarmEvent;
    struct alarm_chn_name alarmName;
    SMART_SCHEDULE schedule;
    struct alarm_chn alarmInfo;
    struct ms_ipc_alarm_in_batch_cfg info;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "alarmCnt=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    alarmCnt = atoi(sValue);
    batch = sdkp2p_get_batch_by_buff(buf, "copy_channel=");

    if (alarmCnt > MAX_IPC_ALARM_IN) {
        alarmCnt = MAX_IPC_ALARM_IN;
    }

    info.cnt = alarmCnt;
    info.batch = batch;
    alarmName.chnid = chnid;
    alarmInfo.chnid = chnid;
    for (i = 0; i < alarmCnt; i++) {
        change = 0;
        alarmName.alarmid = i;
        alarmInfo.alarmid = i;
        snprintf(mask, sizeof(mask), "e%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buf, mask) == 0) {
            write_alarm_chnIn_effective_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        }

        snprintf(mask, sizeof(mask), "a%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buf, mask) == 0) {
            write_alarm_chnIn_audible_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        }

        snprintf(mask, sizeof(mask), "m%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buf, mask) == 0) {
            write_alarm_chnIn_mail_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        }

        snprintf(mask, sizeof(mask), "p%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buf, mask) == 0) {
            write_alarm_chnIn_ptz_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        }

        memset(&amarmEvent, 0x0, sizeof(SMART_EVENT));
        read_alarm_chnIn_event(SQLITE_FILE_NAME, &amarmEvent, &alarmInfo);

        snprintf(mask, sizeof(mask), "tri_chnout1_alarms%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            snprintf(amarmEvent.tri_chnout1_alarms, sizeof(amarmEvent.tri_chnout1_alarms), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "tri_chnout2_alarms%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            snprintf(amarmEvent.tri_chnout2_alarms, sizeof(amarmEvent.tri_chnout2_alarms), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "triAlarms%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            amarmEvent.tri_alarms = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "triChannels%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            snprintf(amarmEvent.tri_channels_ex, sizeof(amarmEvent.tri_channels_ex), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "buzzer_interval%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            amarmEvent.buzzer_interval = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "email_interval%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            amarmEvent.email_interval = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "email_pic_enable%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            amarmEvent.email_pic_enable = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "tri_channels_pic%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            snprintf(amarmEvent.tri_channels_pic, sizeof(amarmEvent.tri_channels_pic), "%s", sValue);
        }

        snprintf(mask, sizeof(mask), "ptz_interval%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            amarmEvent.ptzaction_interval = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "alarmout_interval%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            change = 1;
            amarmEvent.alarmout_interval = atoi(sValue);
        }

        if (change) {
            write_alarm_chnIn_event(SQLITE_FILE_NAME, &amarmEvent, &alarmInfo);
        }

        info.cfg[i].alarmid = i;
        snprintf(mask, sizeof(mask), "enable%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            info.cfg[i].enable = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "type%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            info.cfg[i].alarmType = atoi(sValue);
        }

        snprintf(mask, sizeof(mask), "name%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            snprintf(alarmName.name, sizeof(alarmName.name), "%s", sValue);
            write_alarm_chnIn_event_name(SQLITE_FILE_NAME, &alarmName);
        }

        //copy
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_effective_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        copy_alarm_chnIn_effective_schedules(SQLITE_FILE_NAME, &schedule, i, batch);

        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_audible_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        copy_alarm_chnIn_audible_schedules(SQLITE_FILE_NAME, &schedule, i, batch);

        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_mail_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        copy_alarm_chnIn_mail_schedules(SQLITE_FILE_NAME, &schedule, i, batch);

        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnIn_ptz_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        copy_alarm_chnIn_ptz_schedules(SQLITE_FILE_NAME, &schedule, i, batch);

        struct ptz_action_params ptzActionParams[MAX_CAMERA];
        int cnt = 0;
        memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params)*MAX_CAMERA);
        read_ptz_params(SQLITE_FILE_NAME, ptzActionParams, converse_chn_alarmid_to_event_in(i), chnid, &cnt);
        copy_ptz_params_all(SQLITE_FILE_NAME, ptzActionParams, converse_chn_alarmid_to_event_in(i), batch);

        //event
        copy_alarm_chnIn_events(SQLITE_FILE_NAME, &amarmEvent, i, batch);
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPC_ALARM_INPUT_BATCH, &info, sizeof(struct ms_ipc_alarm_in_batch_cfg),
                    SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPC_ALARMIN_EVENT, &batch, sizeof(long long), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}


static void req_event_get_camera_alarmout(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_get_camera_alarmout(void *param, int size, char *buf, int len, int *datalen)
{
    char tmp[10 * 1024] = {0};
    char mask[32] = {0};
    int i = 0;
    struct ms_ipc_alarm_out *res = (struct ms_ipc_alarm_out *)param;
    struct alarm_chn alarmInfo;
    struct alarm_chn_out_name alarmName;
    SMART_SCHEDULE schedule;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "alarmCnt=%d&", res->alarmCnt);

    alarmInfo.chnid = res->chanid;
    for (i = 0; i < res->alarmCnt && i < MAX_IPC_ALARM_OUT; i++) {
        alarmInfo.alarmid = i;
        snprintf(buf + strlen(buf), len - strlen(buf), "type%d=%d&", i, res->alarmType[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "status%d=%d&", i, res->alarmStatus[i]);

        memset(&alarmName, 0x0, sizeof(struct alarm_chn_name));
        read_alarm_chnOut_event_name(SQLITE_FILE_NAME, &alarmName, &alarmInfo);
        snprintf(buf + strlen(buf), len - strlen(buf), "name%d=%s&", i, alarmName.name);
        snprintf(buf + strlen(buf), len - strlen(buf), "delay_time%d=%d&", i, alarmName.delay_time);

        //effective
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnOut_effective_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        snprintf(mask, sizeof(mask), "e%d", i);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), mask) == 0) {
            snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
        }
    }

    *datalen = strlen(buf) + 1;
    return ;
}

static void req_event_set_camera_alarmout(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char mask[128] = {0};
    long long batch = 0;
    int chnid = 0;
    int alarmCnt = 0;
    int i = 0, j = 0;
    struct alarm_chn_out_name alarmName;
    SMART_SCHEDULE schedule;
    struct alarm_chn alarmInfo;
    struct ms_ipc_alarm_out_batch_cfg info;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "alarmCnt=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    alarmCnt = atoi(sValue);
    batch = sdkp2p_get_batch_by_buff(buf, "copy_channel=");

    if (alarmCnt > MAX_IPC_ALARM_OUT) {
        alarmCnt = MAX_IPC_ALARM_OUT;
    }

    info.cnt = alarmCnt;
    info.batch = batch;
    alarmName.chnid = chnid;
    alarmInfo.chnid = chnid;
    for (i = 0; i < alarmCnt; i++) {
        alarmInfo.alarmid = i;
        snprintf(mask, sizeof(mask), "e%d", i);
        if (sdkp2p_get_schedule_by_buff(&schedule, buf, mask) == 0) {
            write_alarm_chnOut_effective_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        }

        alarmName.alarmid = i;
        snprintf(mask, sizeof(mask), "name%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            snprintf(alarmName.name, sizeof(alarmName.name), "%s", sValue);
        }
        snprintf(mask, sizeof(mask), "delay_time%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            alarmName.delay_time = atoi(sValue);
        }
        write_alarm_chnOut_event_name(SQLITE_FILE_NAME, &alarmName);

        info.cfg[i].alarmid = i;
        snprintf(mask, sizeof(mask), "type%d=", i);
        if (!sdkp2p_get_section_info(buf, mask, sValue, sizeof(sValue))) {
            info.cfg[i].alarmType = atoi(sValue);
        }

        //copy
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_alarm_chnOut_effective_schedule(SQLITE_FILE_NAME, &schedule, &alarmInfo);
        copy_alarm_chnOut_effective_schedules(SQLITE_FILE_NAME, &schedule, &alarmInfo, batch);

        struct alarm_chn_out_name all[MAX_CAMERA];
        read_alarm_chnOut_event_names(SQLITE_FILE_NAME, all, MAX_CAMERA, i);
        for (j = 0; j < MAX_CAMERA; j++) {
            if (!(batch >> j & 0x01)) {
                continue;
            }
            all[j].delay_time = alarmName.delay_time;
        }
        write_alarm_chnOut_event_names(SQLITE_FILE_NAME, all, MAX_CAMERA, batch);
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPC_ALARM_OUTPUT_BATCH, &info, sizeof(struct ms_ipc_alarm_out_batch_cfg),
                    SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPC_ALARMOUT_EVENT, &batch, sizeof(long long), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_event_set_clear_camera_alarmout(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_ipc_alarm_out_state info;

    memset(&info, 0x0, sizeof(struct ms_ipc_alarm_out_state));

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    info.chanid = atoi(sValue);
    if (info.chanid < 0 || info.chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "alarmId=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    info.alarmid = atoi(sValue);
    if (info.alarmid < 0 || info.alarmid >= MAX_IPC_ALARM_OUT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (sdkp2p_get_section_info(buf, "state=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    info.state = atoi(sValue);

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct ms_ipc_alarm_out_state), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_event_get_alarmout_name(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int count = 0;
    struct alarm_out nvrOut[MAX_ALARM_OUT];
    struct alarm_chn_out_name cameraOut[MAX_IPC_ALARM_OUT][MAX_CAMERA];
    int i = 0, j = 0;

    //nvr
    memset(nvrOut, 0x0, sizeof(struct alarm_out) *MAX_ALARM_OUT);
    read_alarm_outs(SQLITE_FILE_NAME, nvrOut, &count);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "nvrCnt=%d&", count);
    for (i = 0; i < count; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "nvr%d=%s&", i, nvrOut[i].name);
    }

    //camera
    for (i = 0; i < MAX_IPC_ALARM_OUT; i++) {
        memset(&cameraOut[i][0], 0x0, sizeof(struct alarm_chn_out_name) * MAX_CAMERA);
        read_alarm_chnOut_event_names(SQLITE_FILE_NAME, &cameraOut[i][0], MAX_CAMERA, i);
    }

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cameraCnt=%d&", MAX_CAMERA);
    for (i = 0; i < MAX_CAMERA; i++) {
        for (j = 0; j < MAX_IPC_ALARM_OUT; j++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camer[%d][%d]=%s&", i, j,
                     cameraOut[j][i].name);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
    return ;
}


static void req_event_get_camera_white_led(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    int type = MAXEVT;
    int chnid = -1;
    int alarmId = 0;
    int interval = 0;
    int cnt;
    char sValue[368] = {0};
    char tmp[10 * 1024] = {0};
    char table[256] = {0};
    SMART_SCHEDULE schedule;
    WHITE_LED_PARAMS_EVTS whiteParams;
    WLED_INFO whiteInfo;
    struct alarm_chn alarmInfo;
    SMART_EVENT smartEvent;
    struct motion move;
    struct video_loss videoloss;
    struct alarm_in nvrAlarm;
    int event = 0;

    if (sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    type = atoi(sValue);
    if (type < 0 || type >= MAXEVT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "alarmId=", sValue, sizeof(sValue))) {
        alarmId = atoi(sValue);
        if (alarmId < 0 || alarmId >= MAX_IPC_ALARM_IN) {
            alarmId = 0;
        }
    }

    alarmInfo.chnid = chnid;
    alarmInfo.alarmid = alarmId;
    whiteInfo.chnid = chnid;
    switch (type) {
        case MOTION://motion
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", MOT_WLED_ESCHE);
            snprintf(table, sizeof(table), "%s", MOT_WLED_PARAMS);
            memset(&move, 0x0, sizeof(struct motion));
            read_motion(SQLITE_FILE_NAME, &move, chnid);
            interval = move.whiteled_interval;
            break;

        case VIDEOLOSS://videolss
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VDL_WLED_ESCHE);
            snprintf(table, sizeof(table), "%s", VDL_WLED_PARAMS);
            memset(&videoloss, 0x0, sizeof(struct video_loss));
            read_video_lost(SQLITE_FILE_NAME, &videoloss, chnid);
            interval = videoloss.whiteled_interval;
            break;

        case ALARMIO://nvr alarm in
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", AIN_WLED_ESCHE);
            snprintf(table, sizeof(table), "%s", AIN_WLED_PARAMS);
            memset(&nvrAlarm, 0x0, sizeof(struct smart_event));
            read_alarm_in(SQLITE_FILE_NAME, &nvrAlarm, chnid);
            interval = nvrAlarm.whiteled_interval;
            break;

        //vca
        case REGION_EN:
        case REGION_EXIT:
        case ADVANCED_MOT:
        case TAMPER_DET:
        case LINE_CROSS:
        case LOITER:
        case HUMAN_DET:
        case PEOPLE_COUNT:
        case LEFTREMOVE:
            if (type == REGION_EN) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VREIN_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VREIN_WLED_PARAMS);
                event = REGIONIN;
            } else if (type == REGION_EXIT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VREEX_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VREEX_WLED_PARAMS);
                event = REGIONOUT;
            } else if (type == ADVANCED_MOT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VMOT_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VMOT_WLED_PARAMS);
                event = ADVANCED_MOTION;
            } else if (type == TAMPER_DET) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VTEP_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VTEP_WLED_PARAMS);
                event = TAMPER;
            } else if (type == LINE_CROSS) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VLSS_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VLSS_WLED_PARAMS);
                event = LINECROSS;
            } else if (type == LOITER) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VLER_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VLER_WLED_PARAMS);
                event = LOITERING;
            } else if (type == HUMAN_DET) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VHMN_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VHMN_WLED_PARAMS);
                event = HUMAN;
            } else if (type == PEOPLE_COUNT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VPPE_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VPPE_WLED_PARAMS);
                event = PEOPLE_CNT;
            } else {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VOBJ_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VOBJ_WLED_PARAMS);
                event = OBJECT_LEFTREMOVE;
            }
            memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
            read_smart_event(SQLITE_FILE_NAME, &smartEvent, chnid, event);
            interval = smartEvent.whiteled_interval;
            break;

        //anpr
        case ANPR_BLACK_EVT:
        case ANPR_WHITE_EVT:
        case ANPR_VISTOR_EVT:
            if (type == ANPR_BLACK_EVT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", LPRB_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", LPRB_WLED_PARAMS);
                event = ANPR_BLACK;
            } else if (type == ANPR_WHITE_EVT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", LPRW_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", LPRW_WLED_PARAMS);
                event = ANPR_WHITE;
            } else {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", LPRV_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", LPRV_WLED_PARAMS);
                event = ANPR_VISTOR;
            }
            memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
            read_anpr_event(SQLITE_FILE_NAME, &smartEvent, chnid, event);
            interval = smartEvent.whiteled_interval;
            break;

        //camera alarm in
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), AINCH_WLED_ESCHE, alarmInfo.alarmid);
            snprintf(table, sizeof(table), AINCH_WLED_PARAMS, alarmInfo.alarmid);
            memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
            read_alarm_chnIn_event(SQLITE_FILE_NAME, &smartEvent, &alarmInfo);
            interval = smartEvent.whiteled_interval;
            break;

        default:
            *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
            return;
    }

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", interval);

    memset(&schedule, 0, sizeof(SMART_SCHEDULE));
    read_whiteled_effective_schedule(SQLITE_FILE_NAME, &schedule, &whiteInfo);
    sdkp2p_get_buff_by_schedule(&schedule, tmp, sizeof(tmp), "w");
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);

    snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", table);
    memset(&whiteParams, 0x0, sizeof(WHITE_LED_PARAMS_EVTS));
    read_whiteled_params(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnid][0], &whiteInfo, &cnt);
    for (i = 0; i < MAX_REAL_CAMERA && i < cnt; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch%d=%d&", i,
                 whiteParams.ledPrms[chnid][i].acto_chn_id);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "mode%d=%d&", i,
                 whiteParams.ledPrms[chnid][i].flash_mode);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "time%d=%d&", i,
                 whiteParams.ledPrms[chnid][i].flash_time);
    }

    *(conf->len) = strlen(conf->resp) + 1;
    return;
}

static void req_event_set_camera_white_led(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char tmp[128] = {0};
    int type = MAXEVT;
    int chnid = -1;
    int alarmId = 0;
    int interval = 0;
    int setSchedule = 0, setParams = 0;
    int reqNumber = 0;
    void *info = NULL;
    int size;
    Uint64 batch = 0;
    char sValue[368] = {0};
    char table[256] = {0};
    SMART_SCHEDULE schedule;
    WHITE_LED_PARAMS_EVTS whiteParams;
    WLED_INFO whiteInfo;
    struct alarm_chn alarmInfo;
    SMART_EVENT smartEvent;
    struct req_anpr_event anprInfo;
    int event = 0;
    Uint64 copy = 0;
    REQ_UPDATE_CHN req = {0};

    if (sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    type = atoi(sValue);
    if (type < 0 || type >= MAXEVT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    batch |= (Uint64)1 << chnid;

    if (!sdkp2p_get_section_info(buf, "alarmId=", sValue, sizeof(sValue))) {
        alarmId = atoi(sValue);
        if (alarmId < 0 || alarmId >= MAX_IPC_ALARM_IN) {
            alarmId = 0;
        }
    }

    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        interval = atoi(sValue);
    }

    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "w") == 0) {
        setSchedule = 1;
    }

    copy = sdkp2p_get_batch_by_buff(buf, "copy=");

    memset(&whiteParams, 0x0, sizeof(WHITE_LED_PARAMS_EVTS));
    whiteParams.ledPrms[chnid][0].chnid = chnid;
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        whiteParams.ledPrms[chnid][i].chnid = chnid;

        snprintf(tmp, sizeof(tmp), "ch%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            break;
        }
        whiteParams.ledPrms[chnid][i].acto_chn_id = atoi(sValue);

        snprintf(tmp, sizeof(tmp), "mode%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            break;
        }
        whiteParams.ledPrms[chnid][i].flash_mode = atoi(sValue);

        snprintf(tmp, sizeof(tmp), "time%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            break;
        }
        whiteParams.ledPrms[chnid][i].flash_time = atoi(sValue);

        setParams = 1;
    }

    alarmInfo.chnid = chnid;
    alarmInfo.alarmid = alarmId;
    whiteInfo.chnid = chnid;
    anprInfo.chnid = chnid;
    switch (type) {
        case MOTION://motion
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", MOT_WLED_ESCHE);
            snprintf(table, sizeof(table), "%s", MOT_WLED_PARAMS);
            if (interval) {
                struct motion move;
                memset(&move, 0x0, sizeof(struct motion));
                read_motion(SQLITE_FILE_NAME, &move, chnid);
                if (move.whiteled_interval != interval) {
                    move.whiteled_interval = interval;
                    write_motion(SQLITE_FILE_NAME, &move);
                }
                if (copy) {
                    copy_motions(SQLITE_FILE_NAME, &move, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_MOTION;
            info = &chnid;
            size = sizeof(int);
            //copy:
            if (copy != 0) {
                copy_whiteled_effective_schedules(SQLITE_FILE_NAME, &schedule, &whiteInfo, copy);
                copy_whiteled_params(SQLITE_FILE_NAME, table, &whiteParams.ledPrms[chnid][0], copy);
            }
            break;

        case VIDEOLOSS://videolss
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VDL_WLED_ESCHE);
            snprintf(table, sizeof(table), "%s", VDL_WLED_PARAMS);
            if (interval) {
                struct video_loss videoloss;
                memset(&videoloss, 0x0, sizeof(struct video_loss));
                read_video_lost(SQLITE_FILE_NAME, &videoloss, chnid);
                if (videoloss.whiteled_interval != interval) {
                    videoloss.whiteled_interval = interval;
                    write_video_lost(SQLITE_FILE_NAME, &videoloss);
                }
                if (copy) {
                    copy_video_losts(SQLITE_FILE_NAME, &videoloss, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_VIDEOLOSS_BATCH;
            info = &batch;
            size = sizeof(Uint64);
            //copy:
            if (copy != 0) {
                copy_whiteled_effective_schedules(SQLITE_FILE_NAME, &schedule, &whiteInfo, copy);
                copy_whiteled_params(SQLITE_FILE_NAME, table, &whiteParams.ledPrms[chnid][0], copy);
            }
            break;

        case ALARMIO://nvr alarm in
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", AIN_WLED_ESCHE);
            snprintf(table, sizeof(table), "%s", AIN_WLED_PARAMS);
            if (interval) {
                struct alarm_in nvrAlarm;
                memset(&nvrAlarm, 0x0, sizeof(struct smart_event));
                read_alarm_in(SQLITE_FILE_NAME, &nvrAlarm, chnid);
                if (nvrAlarm.whiteled_interval != interval) {
                    nvrAlarm.whiteled_interval = interval;
                    write_alarm_in(SQLITE_FILE_NAME, &nvrAlarm);
                }
                if (copy) {
                    copy_alarm_in_events(SQLITE_FILE_NAME, &nvrAlarm, MAX_ALARM_IN, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_ALARMIN_BATCH;
            info = &batch;
            size = sizeof(Uint64);
            //copy:
            if (copy != 0) {
                copy_whiteled_effective_schedules(SQLITE_FILE_NAME, &schedule, &whiteInfo, copy);
                copy_whiteled_params(SQLITE_FILE_NAME, table, &whiteParams.ledPrms[chnid][0], copy);
            }
            break;

        //vca
        case REGION_EN:
        case REGION_EXIT:
        case ADVANCED_MOT:
        case TAMPER_DET:
        case LINE_CROSS:
        case LOITER:
        case HUMAN_DET:
        case PEOPLE_COUNT:
        case LEFTREMOVE:
            if (type == REGION_EN) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VREIN_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VREIN_WLED_PARAMS);
                event = REGIONIN;
            } else if (type == REGION_EXIT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VREEX_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VREEX_WLED_PARAMS);
                event = REGIONOUT;
            } else if (type == ADVANCED_MOT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VMOT_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VMOT_WLED_PARAMS);
                event = ADVANCED_MOTION;
            } else if (type == TAMPER_DET) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VTEP_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VTEP_WLED_PARAMS);
                event = TAMPER;
            } else if (type == LINE_CROSS) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VLSS_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VLSS_WLED_PARAMS);
                event = LINECROSS;
            } else if (type == LOITER) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VLER_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VLER_WLED_PARAMS);
                event = LOITERING;
            } else if (type == HUMAN_DET) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VHMN_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VHMN_WLED_PARAMS);
                event = HUMAN;
            } else if (type == PEOPLE_COUNT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VPPE_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VPPE_WLED_PARAMS);
                event = PEOPLE_CNT;
            } else {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", VOBJ_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", VOBJ_WLED_PARAMS);
                event = OBJECT_LEFTREMOVE;
            }

            if (interval) {
                memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
                read_smart_event(SQLITE_FILE_NAME, &smartEvent, chnid, event);
                if (smartEvent.whiteled_interval != interval) {
                    smartEvent.whiteled_interval = interval;
                    write_smart_event(SQLITE_FILE_NAME, &smartEvent, event);
                }
                if (copy) {
                    copy_smart_events(SQLITE_FILE_NAME, &smartEvent, copy, event);
                }
            }
            reqNumber = REQUEST_FLAG_SET_SMART_EVENT;
            ms_set_bit(&req.chnMask, chnid, 1);
            info = &req;
            size = sizeof(req);
            //copy:
            if (copy != 0) {
                copy_whiteled_effective_schedules(SQLITE_FILE_NAME, &schedule, &whiteInfo, copy);
                copy_whiteled_params(SQLITE_FILE_NAME, table, &whiteParams.ledPrms[chnid][0], copy);
            }
            
            break;

        //anpr
        case ANPR_BLACK_EVT:
        case ANPR_WHITE_EVT:
        case ANPR_VISTOR_EVT:
            if (type == ANPR_BLACK_EVT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", LPRB_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", LPRB_WLED_PARAMS);
                event = ANPR_BLACK;
            } else if (type == ANPR_WHITE_EVT) {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", LPRW_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", LPRW_WLED_PARAMS);
                event = ANPR_WHITE;
            } else {
                snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), "%s", LPRV_WLED_ESCHE);
                snprintf(table, sizeof(table), "%s", LPRV_WLED_PARAMS);
                event = ANPR_VISTOR;
            }
            if (interval) {
                memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
                read_anpr_event(SQLITE_FILE_NAME, &smartEvent, chnid, event);
                if (smartEvent.whiteled_interval != interval) {
                    smartEvent.whiteled_interval = interval;
                    write_anpr_event(SQLITE_FILE_NAME, &smartEvent, event);
                }
            }
            anprInfo.modeType = event;
            reqNumber = REQUEST_FLAG_SET_ANPR_EVENT;
            info = &anprInfo;
            size = sizeof(struct req_anpr_event);
            break;

        //camera alarm in
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(whiteInfo.pDbTable, sizeof(whiteInfo.pDbTable), AINCH_WLED_ESCHE, alarmInfo.alarmid);
            snprintf(table, sizeof(table), AINCH_WLED_PARAMS, alarmInfo.alarmid);
            if (interval) {
                memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
                read_alarm_chnIn_event(SQLITE_FILE_NAME, &smartEvent, &alarmInfo);
                if (smartEvent.whiteled_interval != interval) {
                    smartEvent.whiteled_interval = interval;
                    write_alarm_chnIn_event(SQLITE_FILE_NAME, &smartEvent, &alarmInfo);
                }
                if (copy) {
                    copy_alarm_chnIn_events(SQLITE_FILE_NAME, &smartEvent, alarmInfo.alarmid, copy);
                }
            }
            reqNumber = REQUEST_FLAG_SET_IPC_ALARMIN_EVENT;
            info = &batch;
            size = sizeof(Uint64);
            //copy:
            if (copy != 0) {
                copy_whiteled_effective_schedules(SQLITE_FILE_NAME, &schedule, &whiteInfo, copy);
                copy_whiteled_params(SQLITE_FILE_NAME, table, &whiteParams.ledPrms[chnid][0], copy);
            }
            break;

        default:
            *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
            return;
    }

    if (setSchedule) {
        write_whiteled_effective_schedule(SQLITE_FILE_NAME, &schedule, &whiteInfo);
    }

    if (setParams) {
        write_whiteled_params_all(SQLITE_FILE_NAME, &whiteParams.ledPrms[chnid][0], table, chnid);
    }

    sdkp2p_send_msg(conf, reqNumber, info, size, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    return ;
}

static void req_event_get_heatmap_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_get_heatmap_settings(void *param, int size, char *buf, int len, int *datalen)
{
    char tmp[10 * 1024] = {0};
    struct ms_heat_map_setting *info = (struct ms_heat_map_setting *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", info->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "sensitivity=%d&", info->sensitivity);
    snprintf(buf + strlen(buf), len - strlen(buf), "minsize=%d&", info->minsize);
    snprintf(buf + strlen(buf), len - strlen(buf), "mintime=%d&", info->mintime);
    snprintf(buf + strlen(buf), len - strlen(buf), "adaptability=%d&", info->adaptability);
    snprintf(buf + strlen(buf), len - strlen(buf), "isNtPlatform=%d&", info->isNtPlatform);
    snprintf(buf + strlen(buf), len - strlen(buf), "map_mask=%s&", info->map_mask);
    if (sdkp2p_get_buff_by_schedule(&info->sche, tmp, sizeof(tmp), "e") == 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "%s", tmp);
    }
    *datalen = strlen(buf) + 1;
}

static void req_event_set_heatmap_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[512] = {0};
    int chnid = 0;
    struct ms_heat_map_setting_batch info;
    SMART_SCHEDULE schedule;

    memset(&info, 0x0, sizeof(struct ms_heat_map_setting_batch));
    memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
        if (chnid < 0 || chnid >= MAX_CAMERA) {
            chnid = 0;
        }
    }

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        info.info.enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "sensitivity=", sValue, sizeof(sValue))) {
        info.info.sensitivity = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "minsize=", sValue, sizeof(sValue))) {
        info.info.minsize = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "mintime=", sValue, sizeof(sValue))) {
        info.info.mintime = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "map_mask=", sValue, sizeof(sValue))) {
        snprintf(info.info.map_mask, sizeof(info.info.map_mask), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "adaptability=", sValue, sizeof(sValue))) {
        info.info.adaptability = atoi(sValue);
    }

    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "e") == 0) {
        memcpy(&info.info.sche, &schedule.schedule_day, sizeof(SMART_EVENT));
    }

    info.batch = sdkp2p_get_batch_by_buff(buf, "copy=");
    info.batch |= (long long)1 << chnid;
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct ms_heat_map_setting_batch), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_set_heatmap_settings(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_event_get_heatmap_report(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_heat_map_report info;

    memset(&info, 0x0, sizeof(struct ms_heat_map_report));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    info.chnid = atoi(sValue);
    if (info.chnid < 0 || info.chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    if (!sdkp2p_get_section_info(buf, "mainType=", sValue, sizeof(sValue))) {
        info.mainType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "subType=", sValue, sizeof(sValue))) {
        info.subType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "time=", sValue, sizeof(sValue))) {
        snprintf(info.pTime, sizeof(info.pTime), "%s", sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct ms_heat_map_report), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_get_heatmap_report(void *param, int size, char *buf, int len, int *datalen)
{
    snprintf(buf + strlen(buf), len - strlen(buf), "data=%s&", (char *)param);
    *datalen = strlen(buf) + 1;
}

static void req_event_get_push_msg(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int startTime = 0;
    int endTime = 0;
    int page = 0;
    int appOEM = 0;
    int nvrOEM;
    char company[64] = {0};
    char sValue[256] = {0};
    char eventType[32] = {0};
    char chnId[256] = {0};
    char recordChn[256] = {0};
    int i, j, k;
    int offset;
    int chnSum;

    if (!sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        startTime = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        endTime = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "page=", sValue, sizeof(sValue))) {
        page = atoi(sValue);
    }
    if (page < 1) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    offset = (page - 1) * MAX_ONCE_PUSH_MSG_NUM;
    
    if (!sdkp2p_get_section_info(buf, "oem=", sValue, sizeof(sValue))) {
        appOEM = atoi(sValue);
    }

    nvrOEM = get_param_int(SQLITE_FILE_NAME, PARAM_OEM_TYPE, -1);
    get_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_COMPANY, company, sizeof(company), "0");
    // oem nvr with milesight logo is treated as standard nvr
    if (nvrOEM > 1 && !strcmp(company, "Milesight")) {
        nvrOEM = 0;
    }
    
    if (appOEM != nvrOEM) {
        if (nvrOEM == 0 || nvrOEM == 1 || nvrOEM == 105) {
            return;
        }
        if (nvrOEM != 0 && nvrOEM != 1 && nvrOEM != 105 && appOEM != 1) {
            return;
        }
    }

    MSG_CACHE_S msgCache = {0};
    if (!msgCache.pushMsgArr) {
        msgCache.pushMsgArr = (PUSH_MSG_S *)ms_malloc(sizeof(PUSH_MSG_S) * MAX_MSG_NUM);
        msgCache.cnt = 0;
    }

    // 从数据库获取符合UTCTime的报警消息
    read_push_msgs_by_times(SQLITE_PUSHMSG_NAME, startTime, endTime, &msgCache);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "msgSum=%d&", msgCache.cnt);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "page=%d&", page);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "num=%d&", ms_min(MAX_ONCE_PUSH_MSG_NUM, msgCache.cnt - offset));

    for (i = 0; i < MAX_ONCE_PUSH_MSG_NUM && i < msgCache.cnt - offset; ++i) {
        if (msgCache.pushMsgArr[offset + i].eventType == PUSH_NVR_ALARMIN) {
            chnSum = MAX_ALARM_IN;
        } else if (msgCache.pushMsgArr[offset + i].eventType == PUSH_NVR_POS) {
            chnSum = MAX_POS_CLIENT;
        } else {
            chnSum = MAX_REAL_CAMERA;
        }
        
        for (j = 0; j < chnSum; ++j) {
            if ((msgCache.pushMsgArr[offset + i].chnId >> j) & 0x1) {
                snprintf(chnId + strlen(chnId), sizeof(chnId) - strlen(chnId), "%d ", j + 1);
            }
        }
        for (k = 0; k < MAX_REAL_CAMERA; ++k) {
            if ((msgCache.pushMsgArr[offset + i].recordChn >> k) & 0x1) {
                snprintf(recordChn + strlen(recordChn), sizeof(recordChn) - strlen(recordChn), "%d ", k + 1);
            }
        }

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].chnId=%s&", i, chnId);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].alarmTime=%s&", i,
                 msgCache.pushMsgArr[offset + i].alarmTime);
        get_event_type(eventType, msgCache.pushMsgArr[offset + i].eventType);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].eventType=%s&", i, eventType);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].recordChn=%s&", i, recordChn);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].UTCTime=%d&", i,
                 msgCache.pushMsgArr[offset + i].UTCTime);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].recordType=%s&", i,
                 msgCache.pushMsgArr[offset + i].recordType);

        if (msgCache.pushMsgArr[offset + i].eventType == PUSH_IPC_ALARMIN1
            || msgCache.pushMsgArr[offset + i].eventType == PUSH_IPC_ALARMIN2
            || msgCache.pushMsgArr[offset + i].eventType == PUSH_IPC_ALARMIN3
            || msgCache.pushMsgArr[offset + i].eventType == PUSH_IPC_ALARMIN4) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].alarmId=%d&", i,
                msgCache.pushMsgArr[offset + i].port);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].port=0&", i);
        } else {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].port=%d&", i,
                msgCache.pushMsgArr[offset + i].port);
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].object=%s&", i,
            msgCache.pushMsgArr[offset + i].objStr);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "r[%d].hasSnapshot=%d&", i,
            msgCache.pushMsgArr[offset + i].hasSnapshot);

        memset(chnId, 0, sizeof(chnId));
        memset(recordChn, 0, sizeof(recordChn));
    }

    *(conf->len) = strlen(conf->resp) + 1;

    ms_free(msgCache.pushMsgArr);

    return;
}

static void req_event_get_pos_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char encode[256] = {0};
    char *tmp = NULL;
    int tmpLen = 10 * 1024;
    int id = -1;
    int action = 1;
    Db_POS_CONFIG posCfg;
    SMART_SCHEDULE schedule;
    SMART_EVENT smartevent;
    VCA_PTZ_ACTION_PARAMS ptzParams;
    WHITE_LED_PARAMS_EVTS whiteParams;
    WLED_INFO info;
    int tmpCnt;
    int i;

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        id = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "action=", sValue, sizeof(sValue))) {
        action = atoi(sValue);
    }

    if (id < 0 || id >= MAX_POS_CLIENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    memset(&posCfg, 0, sizeof(Db_POS_CONFIG));
    read_pos_setting(SQLITE_FILE_NAME, &posCfg, id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id=%d&", posCfg.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", posCfg.enable);
    get_url_encode(posCfg.name, strlen(posCfg.name), encode, sizeof(encode));
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "name=%s&", encode);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "protocol=%d&", posCfg.protocol);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "connectionMode=%d&", posCfg.connectionMode);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ip=%s&", posCfg.ip);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "liveviewDisplay=%d&",
             posCfg.liveviewDisplay);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "displayChannel=%d&", posCfg.displayChannel);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "startX=%d&", posCfg.startX);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "startY=%d&", posCfg.startY);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "areaWidth=%d&", posCfg.areaWidth);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "areaHeight=%d&", posCfg.areaHeight);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "characterEncodeing=%d&",
             posCfg.characterEncodeing);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "fontSize=%d&", posCfg.fontSize);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "fontColor=%d&", posCfg.fontColor);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "overlayMode=%d&", posCfg.overlayMode);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "displayTime=%d&", posCfg.displayTime);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "timeout=%d&", posCfg.timeout);
    get_url_encode(posCfg.privacy1, strlen(posCfg.privacy1), encode, sizeof(encode));
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "privacy1=%s&", encode);
    get_url_encode(posCfg.privacy2, strlen(posCfg.privacy2), encode, sizeof(encode));
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "privacy2=%s&", encode);
    get_url_encode(posCfg.privacy3, strlen(posCfg.privacy3), encode, sizeof(encode));
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "privacy3=%s&", encode);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "port=%d&", posCfg.port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "nvrPort=%d&", posCfg.nvrPort);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "clearTime=%d&", posCfg.clearTime);

    if (!action) {
        *(conf->len) = strlen(conf->resp) + 1;
        return;
    }

    //action
    tmp = ms_malloc(tmpLen);
    if (tmp) {
        memset(&smartevent, 0, sizeof(SMART_EVENT));
        read_pos_event(SQLITE_FILE_NAME, &smartevent, id);

        //effective
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_EFFECTIVE, &schedule, id);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "e") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }

        //audible
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &schedule, id);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "a") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "buzzer_interval=%d&",
                 smartevent.buzzer_interval);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_audio_id=%d&", smartevent.tri_audio_id);

        //email
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &schedule, id);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "m") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&",
                 smartevent.email_interval);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_pic_enable=%d&",
                 smartevent.email_pic_enable);

        //ptz
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &schedule, id);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "p") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&",
                 smartevent.ptzaction_interval);
        memset(&ptzParams, 0, sizeof(VCA_PTZ_ACTION_PARAMS));
        read_ptz_params(SQLITE_FILE_NAME, &ptzParams.ptzActionParams[id][0], POS_EVT, id, &tmpCnt);
        for (i = 0; i < tmpCnt && i < MAX_REAL_CAMERA; i++) {
            if (ptzParams.ptzActionParams[id][i].acto_ptz_channel > 0) {
                snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptzCh%d=%d&", i,
                         ptzParams.ptzActionParams[id][i].acto_ptz_channel);
                snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptzType%d=%d&", i,
                         ptzParams.ptzActionParams[id][i].acto_ptz_type);
                snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptzPreset%d=%d&", i,
                         ptzParams.ptzActionParams[id][i].acto_ptz_preset);
                snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptzPatrol%d=%d&", i,
                         ptzParams.ptzActionParams[id][i].acto_ptz_patrol);
            }
        }

        //alarm
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%d&", smartevent.tri_alarms);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_chnout1_alarms=%s&",
                 smartevent.tri_chnout1_alarms);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_chnout2_alarms=%s&",
                 smartevent.tri_chnout2_alarms);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "alarmout_interval=%d&",
                 smartevent.alarmout_interval);

        //white
        memset(&schedule, 0x0, sizeof(SMART_SCHEDULE));
        read_pos_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &schedule, id);
        if (sdkp2p_get_buff_by_schedule(&schedule, tmp, tmpLen, "w") == 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s", tmp);
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "white_interval=%d&",
                 smartevent.whiteled_interval);
        memset(&whiteParams, 0, sizeof(WHITE_LED_PARAMS_EVTS));
        info.chnid = id;
        snprintf(info.pDbTable, sizeof(info.pDbTable), "%s", POS_WLED_PARAMS);
        read_whiteled_params(SQLITE_FILE_NAME, &whiteParams.ledPrms[id][0], &info, &tmpCnt);
        for (i = 0; i < MAX_REAL_CAMERA && i < tmpCnt; i++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "whiteCh%d=%d&", i,
                     whiteParams.ledPrms[id][i].acto_chn_id);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "whiteMode%d=%d&", i,
                     whiteParams.ledPrms[id][i].flash_mode);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "whiteTime%d=%d&", i,
                     whiteParams.ledPrms[id][i].flash_time);
        }

        //others
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChannels=%s&",
                 smartevent.tri_channels_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_pic=%s&",
                 smartevent.tri_channels_pic);

        ms_free(tmp);
    }

    *(conf->len) = strlen(conf->resp) + 1;
    return;
}

static void req_event_set_pos_settings(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[64] = {0};
    int flag;
    int id = -1;
    int i;
    Db_POS_CONFIG posCfg;
    SMART_SCHEDULE schedule;
    SMART_EVENT smartevent;
    VCA_PTZ_ACTION_PARAMS ptzParams;
    WHITE_LED_PARAMS_EVTS whiteParams;

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        id = atoi(sValue);
    }

    if (id < 0 || id >= MAX_POS_CLIENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    memset(&posCfg, 0, sizeof(Db_POS_CONFIG));
    read_pos_setting(SQLITE_FILE_NAME, &posCfg, id);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        posCfg.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(posCfg.name, sizeof(posCfg.name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "protocol=", sValue, sizeof(sValue))) {
        posCfg.protocol = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "connectionMode=", sValue, sizeof(sValue))) {
        posCfg.connectionMode = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ip=", sValue, sizeof(sValue))) {
        snprintf(posCfg.ip, sizeof(posCfg.ip), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "liveviewDisplay=", sValue, sizeof(sValue))) {
        posCfg.liveviewDisplay = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "displayChannel=", sValue, sizeof(sValue))) {
        posCfg.displayChannel = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "startX=", sValue, sizeof(sValue))) {
        posCfg.startX = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "startY=", sValue, sizeof(sValue))) {
        posCfg.startY = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "areaWidth=", sValue, sizeof(sValue))) {
        posCfg.areaWidth = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "areaHeight=", sValue, sizeof(sValue))) {
        posCfg.areaHeight = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "characterEncodeing=", sValue, sizeof(sValue))) {
        posCfg.characterEncodeing = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "fontSize=", sValue, sizeof(sValue))) {
        posCfg.fontSize = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "fontColor=", sValue, sizeof(sValue))) {
        posCfg.fontColor = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "overlayMode=", sValue, sizeof(sValue))) {
        posCfg.overlayMode = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "displayTime=", sValue, sizeof(sValue))) {
        posCfg.displayTime = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "timeout=", sValue, sizeof(sValue))) {
        posCfg.timeout = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "privacy1=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(posCfg.privacy1, sizeof(posCfg.privacy1), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "privacy2=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(posCfg.privacy2, sizeof(posCfg.privacy2), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "privacy3=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(posCfg.privacy3, sizeof(posCfg.privacy3), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        posCfg.port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "nvrPort=", sValue, sizeof(sValue))) {
        posCfg.nvrPort = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "clearTime=", sValue, sizeof(sValue))) {
        posCfg.clearTime = atoi(sValue);
    }
    write_pos_setting(SQLITE_FILE_NAME, &posCfg);

    //action
    memset(&smartevent, 0, sizeof(SMART_EVENT));
    read_pos_event(SQLITE_FILE_NAME, &smartevent, id);

    //effective
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "e") == 0) {
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_EFFECTIVE, &schedule, id);
    }

    //audible
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "a") == 0) {
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_AUDIO, &schedule, id);
    }
    if (!sdkp2p_get_section_info(buf, "buzzer_interval=", sValue, sizeof(sValue))) {
        smartevent.buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_audio_id=", sValue, sizeof(sValue))) {
        smartevent.tri_audio_id = atoi(sValue);
    }

    //email
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "m") == 0) {
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_EMAIL, &schedule, id);
    }
    if (!sdkp2p_get_section_info(buf, "email_interval=", sValue, sizeof(sValue))) {
        smartevent.email_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "email_pic_enable=", sValue, sizeof(sValue))) {
        smartevent.email_pic_enable = atoi(sValue);
    }

    //ptz
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "p") == 0) {
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_PTZ, &schedule, id);
    }
    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        smartevent.ptzaction_interval = atoi(sValue);
    }

    flag = 0;
    memset(&ptzParams, 0, sizeof(VCA_PTZ_ACTION_PARAMS));
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        snprintf(tmp, sizeof(tmp), "ptzCh%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            break;
        }
        ptzParams.ptzActionParams[id][i].acto_ptz_channel = atoi(sValue);

        snprintf(tmp, sizeof(tmp), "ptzType%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            ptzParams.ptzActionParams[id][i].acto_ptz_type = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "ptzPreset%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            ptzParams.ptzActionParams[id][i].acto_ptz_preset = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "ptzPatrol%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            ptzParams.ptzActionParams[id][i].acto_ptz_patrol = atoi(sValue);
        }

        flag = 1;
    }
    if (flag) {
        write_ptz_params_all(SQLITE_FILE_NAME, &ptzParams.ptzActionParams[id][0], POS_EVT, id);
    }

    //alarm
    if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
        smartevent.tri_alarms = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_chnout1_alarms=", sValue, sizeof(sValue))) {
        snprintf(smartevent.tri_chnout1_alarms, sizeof(smartevent.tri_chnout1_alarms), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_chnout2_alarms=", sValue, sizeof(sValue))) {
        snprintf(smartevent.tri_chnout2_alarms, sizeof(smartevent.tri_chnout2_alarms), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "alarmout_interval=", sValue, sizeof(sValue))) {
        smartevent.alarmout_interval = atoi(sValue);
    }

    //white
    if (sdkp2p_get_schedule_by_buff(&schedule, buf, "w") == 0) {
        write_pos_schedule(SQLITE_FILE_NAME, SCHE_WHITELED, &schedule, id);
    }
    if (!sdkp2p_get_section_info(buf, "white_interval=", sValue, sizeof(sValue))) {
        smartevent.whiteled_interval = atoi(sValue);
    }
    flag = 0;
    memset(&whiteParams, 0, sizeof(WHITE_LED_PARAMS_EVTS));
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        snprintf(tmp, sizeof(tmp), "whiteCh%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            break;
        }
        whiteParams.ledPrms[id][i].acto_chn_id = atoi(sValue);
        snprintf(tmp, sizeof(tmp), "whiteMode%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            whiteParams.ledPrms[id][i].flash_mode = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "whiteTime%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            whiteParams.ledPrms[id][i].flash_time = atoi(sValue);
        }

        whiteParams.ledPrms[id][i].chnid = id;
        flag = 1;
    }
    if (flag) {
        write_whiteled_params_all(SQLITE_FILE_NAME, &whiteParams.ledPrms[id][0], POS_WLED_PARAMS, id);
    }

    //others
    if (!sdkp2p_get_section_info(buf, "triChannels=", sValue, sizeof(sValue))) {
        snprintf(smartevent.tri_channels_ex, sizeof(smartevent.tri_channels_ex), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "tri_channels_pic=", sValue, sizeof(sValue))) {
        snprintf(smartevent.tri_channels_pic, sizeof(smartevent.tri_channels_pic), "%s", sValue);
    }
    write_pos_event(SQLITE_FILE_NAME, &smartevent);
    sdkp2p_send_msg(conf, conf->req, &id, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_event_set_pos_settings(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);
    *datalen = strlen(buf) + 1;
}

static void req_event_get_http_notification(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret = -1;
    ret = get_http_notification_json(buf, len, conf->resp, conf->size);
    if (ret) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }

    *(conf->len) = strlen(conf->resp) + 1;
    return;
}

static void req_event_set_http_notification(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret = -1;
    REQUEST_S req;
    memset(&req, 0, sizeof(REQUEST_S));

    ret = set_http_notification_json(buf, len, NULL, 0, &req);
    if (ret) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }

    sdkp2p_send_msg(conf, req.reqNum, req.data, req.len, SDKP2P_NOT_CALLBACK);
    ms_free(req.data);
    *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, REQUEST_SUCCESS, NULL);
    return;
}

int get_pos_all_info_json(char *resp, int respLen)
{
    int i;
    Db_POS_CONFIG posCfg[MAX_POS_CLIENT];
    if (!resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    cJSON *tmp = NULL;
    cJSON *array = NULL;

    memset(&posCfg[0], 0, sizeof(Db_POS_CONFIG)*MAX_POS_CLIENT);
    read_pos_settings(SQLITE_FILE_NAME, posCfg);
    json = cJSON_CreateObject();
    array = cJSON_CreateArray();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    for (i = 0; i < MAX_POS_CLIENT; i++) {
        tmp = cJSON_CreateObject();
        cJSON_AddNumberToObject(tmp, "id", posCfg[i].id);
        cJSON_AddNumberToObject(tmp, "enable", posCfg[i].enable);
        cJSON_AddStringToObject(tmp, "name", posCfg[i].name);
        cJSON_AddNumberToObject(tmp, "protocol", posCfg[i].protocol);
        cJSON_AddNumberToObject(tmp, "connectionMode", posCfg[i].connectionMode);
        cJSON_AddStringToObject(tmp, "ip", posCfg[i].ip);
        cJSON_AddNumberToObject(tmp, "liveviewDisplay", posCfg[i].liveviewDisplay);
        cJSON_AddNumberToObject(tmp, "displayChannel", posCfg[i].displayChannel);
        cJSON_AddNumberToObject(tmp, "startX", posCfg[i].startX);
        cJSON_AddNumberToObject(tmp, "startY", posCfg[i].startY);
        cJSON_AddNumberToObject(tmp, "areaWidth", posCfg[i].areaWidth);
        cJSON_AddNumberToObject(tmp, "areaHeight", posCfg[i].areaHeight);
        cJSON_AddNumberToObject(tmp, "characterEncodeing", posCfg[i].characterEncodeing);
        cJSON_AddNumberToObject(tmp, "fontSize", posCfg[i].fontSize);
        cJSON_AddNumberToObject(tmp, "fontColor", posCfg[i].fontColor);
        cJSON_AddNumberToObject(tmp, "overlayMode", posCfg[i].overlayMode);
        cJSON_AddNumberToObject(tmp, "displayTime", posCfg[i].displayTime);
        cJSON_AddNumberToObject(tmp, "timeout", posCfg[i].timeout);
        cJSON_AddStringToObject(tmp, "privacy1", posCfg[i].privacy1);
        cJSON_AddStringToObject(tmp, "privacy2", posCfg[i].privacy2);
        cJSON_AddStringToObject(tmp, "privacy3", posCfg[i].privacy3);
        cJSON_AddNumberToObject(tmp, "port", posCfg[i].port);
        cJSON_AddNumberToObject(tmp, "nvrPort", posCfg[i].nvrPort);
        cJSON_AddNumberToObject(tmp, "clearTime", posCfg[i].clearTime);
        cJSON_AddItemToArray(array, tmp);
    }

    cJSON_AddItemToObject(json, "pos", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

static void req_event_get_ipc_audio_alarm(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    chnId = sdkp2p_req_common_get_chnid(buf, strlen(buf));
    if (chnId == MS_INVALID_VALUE) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);

    return;
}

static void resp_event_get_ipc_audio_alarm(void *param, int size, char *buf, int len, int *datalen)
{
    if (!param || size <= sizeof(IPC_AUDIO_ALARM_S)) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    int ret;

    ret = sdkp2p_get_ipc_audio_alarm(param, size, buf, len);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    *datalen = strlen(buf) + 1;
}

static void req_event_set_ipc_audio_alarm(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret = -1;
    IPC_AUDIO_ALARM_S reqParam;
    Uint64 chnMask = 0;

    memset(&reqParam, 0, sizeof(IPC_AUDIO_ALARM_S));

    ret = sdkp2p_set_ipc_audio_alarm(buf, &reqParam);
    if (ret != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, ret, NULL);
        return;
    }

    chnMask = get_channel_mask(reqParam.copyChn);
    chnMask |= (Uint64)1 << reqParam.chnId;
    
    sdkp2p_send_msg(conf, conf->req, &chnMask, sizeof(Uint64), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, conf->req, &reqParam, sizeof(IPC_AUDIO_ALARM_S), SDKP2P_NEED_CALLBACK);
    
    return;
}

static void req_event_get_ipc_audio_alarm_sample(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnId;

    chnId = sdkp2p_req_common_get_chnid(buf, strlen(buf));
    if (chnId == MS_INVALID_VALUE) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);

    return;
}

static void resp_event_get_ipc_audio_alarm_sample(void *param, int size, char *buf, int len, int *datalen)
{
    if (!param || size <= sizeof(int)) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    int ret;

    ret = sdkp2p_get_ipc_audio_alarm_sample(param, size, buf, len);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    *datalen = strlen(buf) + 1;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_ALARMIN, req_event_get_alarmin, -1, NULL},//get.event.alarmin
    {REQUEST_FLAG_SET_ALARMIN, req_event_set_alarmin, RESPONSE_FLAG_SET_ALARMIN, NULL},//set.event.alarmin
    {REQUEST_FLAG_GET_ALARMOUT, req_event_get_alarmout, RESPONSE_FLAG_GET_ALARMOUT, NULL},//get.event.alarmout
    {REQUEST_FLAG_SET_ALARMOUT, req_event_set_alarmout, -1, NULL},//set.event.alarmout
    {REQUEST_FLAG_GET_HOLIDAYS, req_event_get_holidays, -1, NULL},//get.event.holidays
    {REQUEST_FLAG_SET_HOLIDAY, req_event_set_holidays, -1, NULL},//set.event.holidays
    {REQUEST_FLAG_GET_VIDEOLOSS, req_event_get_videoloss, RESPONSE_FLAG_GET_VIDEOLOSS, NULL},//get.event.videoloss
    {REQUEST_FLAG_SET_VIDEOLOSS, req_event_set_videoloss, -1, NULL},//set.event.videoloss
    {REQUEST_FLAG_GET_IPCMTMAP, req_event_get_motion_map, RESPONSE_FLAG_GET_IPCMTMAP, resp_event_get_motion_map},//get.event.motion
    {REQUEST_FLAG_SET_IPCMTMAP, req_event_set_motion_map, RESPONSE_FLAG_SET_IPCMTMAP, NULL},//set.event.motion
    {REQUEST_FLAG_GET_OSDPARAM, req_event_get_osd, -1, NULL},//get.event.osd
    {REQUEST_FLAG_SET_OSDPARAM, req_event_set_osd, -1, NULL},//set.event.osd
    {REQUEST_FLAG_GET_MAINTAIN_INFO, req_disk_get_maintain_info, -1, NULL},//get.event.maintain_info
    {REQUEST_FLAG_SET_DISK_MAINTAINANCE, set_disk_maintainance_to_struct, -1, NULL},//set.event.maintain_info
    {REQUEST_FLAG_GET_EXCEPT_INFO, req_event_get_except_info, -1, NULL},//get.event.exception
    {REQUEST_FLAG_SET_TRIGGER_ALARMS, req_event_set_except_info, -1, NULL},//set.event.exception
    {REQUEST_FLAG_GET_EXCEPTION, NULL, RESPONSE_FLAG_GET_EXCEPTION, resp_event_get_exception},//get.event.exception_info
    {REQUEST_FLAG_GET_LPR_LIST, req_event_get_lpr_list, RESPONSE_FLAG_GET_LPR_LIST, NULL},//get.event.lpr_list_info
    {REQUEST_FLAG_ADD_LPR_LIST, req_event_add_lpr_list, RESPONSE_FLAG_ADD_LPR_LIST, NULL},//set.event.add_lpr_list
    {REQUEST_FLAG_DEL_LPR_LIST, req_event_del_lpr_list, RESPONSE_FLAG_DEL_LPR_LIST, NULL},//set.event.del_lpr_list
    {REQUEST_FLAG_EDIT_LPR_LIST, req_event_edit_lpr_list, RESPONSE_FLAG_EDIT_LPR_LIST, NULL},//set.event.update_lpr_list
    {REQUEST_FLAG_GET_LPR_MODE_INFO, req_event_get_lpr_mode_info, RESPONSE_FLAG_GET_LPR_MODE_INFO, NULL},//get.event.anpr_mode_params
    {REQUEST_FLAG_SET_LPR_MODE_INFO, req_event_set_lpr_mode_info, RESPONSE_FLAG_SET_LPR_MODE_INFO, NULL},//set.event.anpr_mode_params
    {REQUEST_FLAG_GET_LPR_SETTINGS, req_event_get_lpr_settings, RESPONSE_FLAG_GET_LPR_SETTINGS, resp_event_get_lpr_settings},//get.event.lpr_settings
    {REQUEST_FLAG_SET_LPR_SETTINGS, req_event_set_lpr_settings, RESPONSE_FLAG_SET_LPR_SETTINGS, resp_event_set_lpr_settings},//set.event.lpr_settings
    {REQUEST_FLAG_GET_LPR_SUPPORT, req_event_get_lpr_support, RESPONSE_FLAG_GET_LPR_SUPPORT, resp_event_get_lpr_support},//get.event.support_anpr
    {REQUEST_FLAG_GET_IPC_ALARM_INPUT, req_event_get_camera_alarmin, RESPONSE_FLAG_GET_IPC_ALARM_INPUT, resp_event_get_camera_alarmin},//get.event.camera_alarmin
    {REQUEST_FLAG_SET_IPC_ALARM_INPUT, req_event_set_camera_alarmin, RESPONSE_FLAG_SET_IPC_ALARM_INPUT, NULL},//set.event.camera_alarmin
    {REQUEST_FLAG_GET_IPC_ALARM_OUTPUT, req_event_get_camera_alarmout, RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT, resp_event_get_camera_alarmout},//get.event.camera_alarmout
    {REQUEST_FLAG_SET_IPC_ALARM_OUTPUT, req_event_set_camera_alarmout, RESPONSE_FLAG_SET_IPC_ALARM_OUTPUT, NULL},//set.event.camera_alarmout
    {REQUEST_FLAG_SET_IPC_ALARMOUT_STATE, req_event_set_clear_camera_alarmout, RESPONSE_FLAG_SET_IPC_ALARMOUT_STATE, NULL},//set.event.clear_camera_alarmout
    {REQUEST_FLAG_GET_ALARM_OUT_NAME, req_event_get_alarmout_name, RESPONSE_FLAG_GET_ALARM_OUT_NAME, NULL},//get.event.alarmout_name
    {REQUEST_FLAG_GET_ALARM_WHITE_LED, req_event_get_camera_white_led, RESPONSE_FLAG_GET_ALARM_WHITE_LED, NULL},//get.event.camera_white_led
    {REQUEST_FLAG_SET_ALARM_WHITE_LED, req_event_set_camera_white_led, RESPONSE_FLAG_SET_ALARM_WHITE_LED, NULL},//set.event.camera_white_led

    {REQUEST_FLAG_GET_HEAT_MAP_SETTING, req_event_get_heatmap_settings, RESPONSE_FLAG_GET_HEAT_MAP_SETTING, resp_event_get_heatmap_settings},//get.event.heatmap_settings
    {REQUEST_FLAG_SET_HEAT_MAP_SETTING_BATCH, req_event_set_heatmap_settings, RESPONSE_FLAG_SET_HEAT_MAP_SETTING_BATCH, resp_event_set_heatmap_settings},//set.event.heatmap_settings
    {REQUEST_FLAG_SEARCH_HEAT_MAP_REPORT, req_event_get_heatmap_report, RESPONSE_FLAG_SEARCH_HEAT_MAP_REPORT, resp_event_get_heatmap_report},//get.event.heatmap_report

    {REQUEST_FLAG_GET_IPCMTSCE, NULL, RESPONSE_FLAG_GET_IPCMTSCE, get_ipcmtsce_to_str},
    {REQUEST_FLAG_SET_IPCMTSCE, NULL, RESPONSE_FLAG_SET_IPCMTSCE, set_ipcmtsce_to_str},
    {REQUEST_FLAG_GET_MOTION, get_motion_to_struct, RESPONSE_FLAG_GET_MOTION, NULL},
    {REQUEST_FLAG_SET_MOTION, set_motion_to_struct, -1, NULL},
    {REQUEST_FLAG_GET_PUSH_MSG, req_event_get_push_msg, RESPONSE_FLAG_GET_PUSH_MSG, NULL},// get.event.push_msg
    {REQUEST_FLAG_GET_POS_SETTING, req_event_get_pos_settings, RESPONSE_FLAG_GET_POS_SETTING, NULL},//get.event.pos_settings
    {REQUEST_FLAG_SET_POS_SETTING, req_event_set_pos_settings, RESPONSE_FLAG_SET_POS_SETTING, resp_event_set_pos_settings},//set.event.pos_settings
    {REQUEST_FLAG_GET_HTTP_NOTIFICATION, req_event_get_http_notification, RESPONSE_FLAG_GET_HTTP_NOTIFICATION, NULL},// get.event.http_notification
    {REQUEST_FLAG_SET_HTTP_NOTIFICATION, req_event_set_http_notification, RESPONSE_FLAG_SET_HTTP_NOTIFICATION, NULL},// set.event.http_notification
    {REQUEST_FLAG_GET_IPC_AUDIO_ALARM, req_event_get_ipc_audio_alarm, RESPONSE_FLAG_GET_IPC_AUDIO_ALARM, resp_event_get_ipc_audio_alarm},// get.event.ipc.audio.alarm
    {REQUEST_FLAG_SET_IPC_AUDIO_ALARM, req_event_set_ipc_audio_alarm, RESPONSE_FLAG_SET_IPC_AUDIO_ALARM, sdkp2p_resp_common_set},// set.event.ipc.audio.alarm
    {REQUEST_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE, req_event_get_ipc_audio_alarm_sample, RESPONSE_FLAG_GET_IPC_AUDIO_ALARM_SAMPLE, resp_event_get_ipc_audio_alarm_sample},// get.event.ipc.audio.alarm.sample
    // request register array end
};


void p2p_event_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_event_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

