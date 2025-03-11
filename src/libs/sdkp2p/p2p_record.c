#include "sdk_util.h"


int get_export_disk_json(void *data, int cnt, char *resp, int respLen)
{
    if (!resp || respLen <= 0) {
        return -1;
    }

    
    int i;
    char *sResp = NULL;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    char encode[MAX_LEN_256] = {0};
    struct resp_usb_info *info = (struct resp_usb_info *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);

    array = cJSON_CreateArray();
    for (i = 0; i < cnt; i++) {
        obj = cJSON_CreateObject();
        if (!obj) {
            continue;
        }
        cJSON_AddNumberToObject(obj, "port", info[i].port);
        cJSON_AddNumberToObject(obj, "protect", info[i].bprotect);
        cJSON_AddNumberToObject(obj, "format", info[i].bformat);
        cJSON_AddNumberToObject(obj, "free", info[i].free);
        get_url_encode(info[i].dev_name, strlen(info[i].dev_name), encode, sizeof(encode));
        cJSON_AddStringToObject(obj, "name", encode);
        get_url_encode(info[i].dev_path, strlen(info[i].dev_path), encode, sizeof(encode));
        cJSON_AddStringToObject(obj, "path", encode);
        cJSON_AddNumberToObject(obj, "type", info[i].type);
        cJSON_AddNumberToObject(obj, "state", info[i].state);
        cJSON_AddNumberToObject(obj, "capacity", info[i].capacity);
        cJSON_AddNumberToObject(obj, "autoBackupCapacity", info[i].autoBackupCapacity);
        cJSON_AddNumberToObject(obj, "bRec", info[i].bRec);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "disk", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}

int get_export_auto_backup_json(char *resp, int respLen, int backupType)
{
    if (!resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    ESATA_AUTO_BACKUP info;
    ESATA_AUTO_BACKUP anprInfo;

    json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }
    memset(&info, 0, sizeof(ESATA_AUTO_BACKUP));
    memset(&anprInfo, 0, sizeof(ESATA_AUTO_BACKUP));
    read_esata_auto_backup(SQLITE_FILE_NAME, &info);
    read_anpr_auto_backup(SQLITE_FILE_NAME, &anprInfo);
    
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "enable", info.enable);
    cJSON_AddNumberToObject(json, "anprEnable", anprInfo.enable);    
    if (backupType == 1) {
        memcpy(&info, &anprInfo, sizeof(ESATA_AUTO_BACKUP));
    }
    cJSON_AddNumberToObject(json, "streamType", info.stream_type);
    cJSON_AddNumberToObject(json, "backupDay", info.backup_day);
    cJSON_AddNumberToObject(json, "fileType", info.file_type);
    cJSON_AddNumberToObject(json, "recycle", info.recycle);
    cJSON_AddNumberToObject(json, "device", info.device);
    cJSON_AddNumberToObject(json, "port", info.port);
    cJSON_AddStringToObject(json, "channel", info.tri_channels);
    cJSON_AddStringToObject(json, "backupTime", info.backup_time);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}

SDK_ERROR_CODE get_export_auto_backup_str(const char *buff, int backupType)
{
    char sValue[256] = {0};
    int flag = 0;
    ESATA_AUTO_BACKUP info;
    
    memset(&info, 0, sizeof(ESATA_AUTO_BACKUP));
    if (backupType == 0) {
        read_esata_auto_backup(SQLITE_FILE_NAME, &info);
        if (!sdkp2p_get_section_info(buff, "enable=", sValue, sizeof(sValue))) {
            info.enable = (atoi(sValue) == 0) ? 0 : 1;
            flag = 1;
        }
    } else {
        read_anpr_auto_backup(SQLITE_FILE_NAME, &info);
        if (!sdkp2p_get_section_info(buff, "anprEnable=", sValue, sizeof(sValue))) {
            info.enable = (atoi(sValue) == 0) ? 0 : 1;
            flag = 1;
        }
    }
    
    if (!sdkp2p_get_section_info(buff, "streamType=", sValue, sizeof(sValue))) {
        info.stream_type = atoi(sValue);
        flag = 1;
    }

    if (!sdkp2p_get_section_info(buff, "backupDay=", sValue, sizeof(sValue))) {
        info.backup_day = atoi(sValue);
        flag = 1;
    }

    if (!sdkp2p_get_section_info(buff, "fileType=", sValue, sizeof(sValue))) {
        info.file_type = atoi(sValue);
        flag = 1;
    }

    if (!sdkp2p_get_section_info(buff, "recycle=", sValue, sizeof(sValue))) {
        info.recycle = atoi(sValue);
        flag = 1;
    }

    if (!sdkp2p_get_section_info(buff, "device=", sValue, sizeof(sValue))) {
        info.device = atoi(sValue);
        flag = 1;
    }

    if (!sdkp2p_get_section_info(buff, "port=", sValue, sizeof(sValue))) {
        info.port = atoi(sValue);
        flag = 1;
    }

    if (!sdkp2p_get_section_info(buff, "channel=", sValue, sizeof(sValue))) {
        snprintf(info.tri_channels, sizeof(info.tri_channels), "%s", sValue);
        flag = 1;
    }

    if (!sdkp2p_get_section_info(buff, "backupTime=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.backup_time, sizeof(info.backup_time), "%s", sValue);
        flag = 1;
    }

    if (flag) {
        if (backupType == 0) {
            write_esata_auto_backup(SQLITE_FILE_NAME, &info);
        } else {
            write_anpr_auto_backup(SQLITE_FILE_NAME, &info);
        }
    }

    return REQUEST_SUCCESS;
}

int get_auto_backup_status_json(void *data, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    struct resp_esata_backup_status *info = (struct resp_esata_backup_status *)data;

    json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "state", info->status);
    cJSON_AddStringToObject(json, "lastTime", info->last_time);
    cJSON_AddNumberToObject(json, "percent", info->percent);
    cJSON_AddNumberToObject(json, "capacity", info->capacity);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}

int get_disk_format_json(void *data, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    struct RespForamtInfo *info = (struct RespForamtInfo *)data;

    json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    
    cJSON_AddNumberToObject(json, "state", info->esataStatus);
    cJSON_AddNumberToObject(json, "percent", info->percent);
    cJSON_AddNumberToObject(json, "port", info->port);
    cJSON_AddNumberToObject(json, "type", info->type);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}

static void record_task_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_record_task *res = (struct req_record_task *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.chanid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->chanid));
    }
    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->enable));
    }
    if (!sdkp2p_get_section_info(buf, "r.event=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->recType));
    }
    *datalen = sizeof(struct req_record_task);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void record_task_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_ret_rectask
    struct resp_ret_rectask *res = (struct resp_ret_rectask *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chanid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", res->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.event=%d&", res->recType);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.result=%d&", res->result);

    *datalen = strlen(buf) + 1;
}

static void req_record_get_record_advanced(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[256] = {0};
    struct record record = {0};
    struct camera db_camera = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    read_record(SQLITE_FILE_NAME, &record, chnid);
    read_camera(SQLITE_FILE_NAME, &db_camera, chnid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id=%d&", record.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "mode=%d&", record.mode);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "audio_enable=%d&", record.audio_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "prev_record_on=%d&", record.prev_record_on);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "prev_record_duration=%d&",
             record.prev_record_duration);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "post_record_on=%d&", record.post_record_on);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "post_record_duration=%d&",
             record.post_record_duration);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "record_expiration_date=%d&",
             record.record_expiration_date);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "photo_expiration_date=%d&",
             record.photo_expiration_date);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "record_stream=%d&",
             db_camera.record_stream);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "is_record_primary_secondary=1&");

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_record_set_record_advanced(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = 0;
    char sValue[256] = {0};
    struct record record = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    read_record(SQLITE_FILE_NAME, &record, chnid);
    if (!sdkp2p_get_section_info(buf, "mode=", sValue, sizeof(sValue))) {
        record.mode = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "audio_enable=", sValue, sizeof(sValue))) {
        record.audio_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "prev_record_on=", sValue, sizeof(sValue))) {
        record.prev_record_on = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "prev_record_duration=", sValue, sizeof(sValue))) {
        record.prev_record_duration = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "post_record_on=", sValue, sizeof(sValue))) {
        record.post_record_on = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "post_record_duration=", sValue, sizeof(sValue))) {
        record.post_record_duration = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "record_expiration_date=", sValue, sizeof(sValue))) {
        record.record_expiration_date = atoi(sValue);
    }

    write_record(SQLITE_FILE_NAME, &record);

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RECORDMODE, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RECPARAM, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_record_set_record_mode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = 0;
    char sValue[256] = {0};
    struct record record = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    read_record(SQLITE_FILE_NAME, &record, chnid);
    if (!sdkp2p_get_section_info(buf, "mode=", sValue, sizeof(sValue))) {
        record.mode = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "audio_enable=", sValue, sizeof(sValue))) {
        record.audio_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "prev_record_on=", sValue, sizeof(sValue))) {
        record.prev_record_on = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "prev_record_duration=", sValue, sizeof(sValue))) {
        record.prev_record_duration = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "post_record_on=", sValue, sizeof(sValue))) {
        record.post_record_on = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "post_record_duration=", sValue, sizeof(sValue))) {
        record.post_record_duration = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "record_expiration_date=", sValue, sizeof(sValue))) {
        record.record_expiration_date = atoi(sValue);
    }

    write_record(SQLITE_FILE_NAME, &record);

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_RECORD_MODE, 0, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}


static void req_record_get_record_schedule(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = 0;
    struct record_schedule record;

    if (!sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        chanid = atoi(sValue);
    }
    memset(&record, 0, sizeof(struct record_schedule));
    read_record_schedule(SQLITE_FILE_NAME, &record, chanid);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch=%d&", chanid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", record.enable);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_enable[%d]=%d&", iday,
                 record.schedule_day[iday].wholeday_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_action_type[%d]=%d&", iday,
                 record.schedule_day[iday].wholeday_action_type);
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     record.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     record.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     record.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_record_set_record_schedule(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct record_schedule record;

    memset(&record, 0, sizeof(struct record_schedule));
    if (!sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    read_record_schedule(SQLITE_FILE_NAME, &record, chnid);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        record.enable = atoi(sValue);
    }
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(tmp, sizeof(tmp), "wholeday_enable[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            record.schedule_day[iday].wholeday_enable = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "wholeday_action_type[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            record.schedule_day[iday].wholeday_action_type = atoi(sValue);
        }
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; item++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(record.schedule_day[iday].schedule_item[item].start_time,
                         sizeof(record.schedule_day[iday].schedule_item[item].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(record.schedule_day[iday].schedule_item[item].end_time,
                         sizeof(record.schedule_day[iday].schedule_item[item].end_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                record.schedule_day[iday].schedule_item[item].action_type = atoi(sValue);
            }
        }
    }

    write_record_schedule(SQLITE_FILE_NAME, &record, chnid);

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_record_set_all_record(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char sRecordValue[64] = {0};
    char sStopValue[64] = {0};
    struct resp_set_all_record all_record = {0};

    if (!sdkp2p_get_section_info(buf, "record=", sRecordValue, sizeof(sRecordValue))) {

    }
    if (!sdkp2p_get_section_info(buf, "stop=", sStopValue, sizeof(sStopValue))) {

    }
    for (i = 0; i < strlen(sStopValue); i++) {
        all_record.enable = 0;
        if (sStopValue[i] == '1') {
            all_record.channel[i] = 1;
        } else {
            all_record.channel[i] = 0;
        }
    }
    sdkp2p_send_msg(conf, conf->req, &all_record, sizeof(struct resp_set_all_record), SDKP2P_NOT_CALLBACK);

    memset(&all_record, 0, sizeof(struct resp_set_all_record));
    for (i = 0; i < strlen(sRecordValue); i++) {
        all_record.enable = 1;
        if (sRecordValue[i] == '1') {
            all_record.channel[i] = 1;
        } else {
            all_record.channel[i] = 0;
        }
    }
    sdkp2p_send_msg(conf, conf->req, &all_record, sizeof(struct resp_set_all_record), SDKP2P_NOT_CALLBACK);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_record_get_snapshot_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[256] = {0};
    struct snapshot db_snapshot = {0};

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    chnid = atoi(sValue);

    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    memset(&db_snapshot, 0x0, sizeof(struct snapshot));
    read_snapshot(SQLITE_FILE_NAME, &db_snapshot, chnid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id=%d&", db_snapshot.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "stream_type=%d&", db_snapshot.stream_type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "quality=%d&", db_snapshot.quality);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", db_snapshot.interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval_unit=%d&",
             db_snapshot.interval_unit);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "width=%d&", db_snapshot.width);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "height=%d&", db_snapshot.height);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "expiration_date=%d&",
             db_snapshot.expiration_date);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "resolution=%d&", db_snapshot.resolution);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_record_set_snapshot_param(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    int i = 0;
    char sValue[256] = {0};
    struct snapshot db_snapshot;
    long long change_flag = 0;
    char channels[MAX_CAMERA + 1] = {0};
    struct channel_for_batch info;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);

    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    memset(&db_snapshot, 0x0, sizeof(struct snapshot));
    read_snapshot(SQLITE_FILE_NAME, &db_snapshot, chnid);

    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        db_snapshot.interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "interval_unit=", sValue, sizeof(sValue))) {
        db_snapshot.interval_unit = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "resolution=", sValue, sizeof(sValue))) {
        db_snapshot.resolution = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "stream_type=", sValue, sizeof(sValue))) {
        db_snapshot.stream_type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "quality=", sValue, sizeof(sValue))) {
        db_snapshot.quality = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "expiration_date=", sValue, sizeof(sValue))) {
        db_snapshot.expiration_date = atoi(sValue);
    }

    memset(&info, 0x0, sizeof(struct channel_for_batch));
    memset(channels, 0x0, sizeof(channels));
    if (!sdkp2p_get_section_info(buf, "copy=", sValue, sizeof(sValue))) {
        snprintf(channels, sizeof(channels), "%s", sValue);
        for (i = 0; i < MAX_CAMERA; i++) {
            if (channels[i] == '0' && i != chnid) {
                continue;
            }
            change_flag |= (Uint64)1 << i;
            info.chanid[info.size] = i;
            info.size++;
        }
    } else {
        //only one channel
        change_flag = (Uint64)1<<chnid ;
        info.chanid[info.size] = chnid;
        info.size++;
    }

    copy_snapshots(SQLITE_FILE_NAME, &db_snapshot, change_flag);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct channel_for_batch), SDKP2P_NOT_CALLBACK);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_record_get_snapshot_schedule(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct snapshot_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chanid = atoi(sValue);
    if (chanid < 0 || chanid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    memset(&schedule, 0, sizeof(struct snapshot_schedule));
    read_snapshot_schedule(SQLITE_FILE_NAME, &schedule, chanid);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ch=%d&", chanid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", schedule.enable);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_enable[%d]=%d&", iday,
                 schedule.schedule_day[iday].wholeday_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_action_type[%d]=%d&", iday,
                 schedule.schedule_day[iday].wholeday_action_type);
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
    return;
}

static void req_record_set_snapshot_schedule(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};
    int i = 0;
    char channels[MAX_CAMERA + 1] = {0};
    long long change_flag = 0;
    struct channel_for_batch info;


    struct snapshot_schedule schedule;
    memset(&schedule, 0, sizeof(struct snapshot_schedule));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);

    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    read_snapshot_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        schedule.enable = atoi(sValue);
    }

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(tmp, sizeof(tmp), "wholeday_enable[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            schedule.schedule_day[iday].wholeday_enable = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "wholeday_action_type[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            schedule.schedule_day[iday].wholeday_action_type = atoi(sValue);
        }
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; item++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[iday].schedule_item[item].start_time,
                         sizeof(schedule.schedule_day[iday].schedule_item[item].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[iday].schedule_item[item].end_time,
                         sizeof(schedule.schedule_day[iday].schedule_item[item].end_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule.schedule_day[iday].schedule_item[item].action_type = atoi(sValue);
            }
        }
    }

    memset(&info, 0x0, sizeof(struct channel_for_batch));
    change_flag = 0;
    if (!sdkp2p_get_section_info(buf, "copy=", sValue, sizeof(sValue))) {
        snprintf(channels, sizeof(channels), "%s", sValue);
        for (i = 0; i < MAX_CAMERA; i++) {
            if (channels[i] == '0' && i != chnid) {
                continue;
            }
            change_flag |= (Uint64)1 << i;
            info.chanid[info.size] = i;
            info.size++;
        }
    } else {
        change_flag |= (Uint64)1 << chnid;
        info.chanid[info.size] = chnid;
        info.size++;
    }

    copy_snapshot_schedules(SQLITE_FILE_NAME, &schedule, change_flag);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct channel_for_batch), SDKP2P_NOT_CALLBACK);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_record_get_live_snapshot(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[256] = {0};

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
}

static void resp_record_get_live_snapshot(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_snapshot_state *info = (struct resp_snapshot_state *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", info->status);
    *datalen = strlen(buf) + 1;
}

static void req_record_get_esata_backup(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    ESATA_AUTO_BACKUP info = {0};
    ESATA_AUTO_BACKUP anprInfo = {0};
    int backupType = 0;

    if (!sdkp2p_get_section_info(buf, "backupType=", sValue, sizeof(sValue))) {
        backupType = (atoi(sValue) == 0) ? 0 : 1;
    }

    read_esata_auto_backup(SQLITE_FILE_NAME, &info);
    read_anpr_auto_backup(SQLITE_FILE_NAME, &anprInfo);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", info.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "anprEnable=%d&", anprInfo.enable);
    
    if (backupType == 1) {
        memcpy(&info, &anprInfo, sizeof(ESATA_AUTO_BACKUP));
    }

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "backup_day=%d&", info.backup_day);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "backup_time=%s&", info.backup_time);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "stream_type=%d&", info.stream_type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "file_type=%d&", info.file_type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "recycle=%d&", info.recycle);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels=%s&", info.tri_channels);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "device=%d&", info.device);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "port=%d&", info.port);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_record_set_esata_backup(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    ESATA_AUTO_BACKUP info;
    int backupType = 0;

    if (!sdkp2p_get_section_info(buf, "backupType=", sValue, sizeof(sValue))) {
        backupType = (atoi(sValue) == 0) ? 0 : 1;
    }

    memset(&info, 0, sizeof(ESATA_AUTO_BACKUP));
    if (backupType == 0) {
        read_esata_auto_backup(SQLITE_FILE_NAME, &info);
        if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
            info.enable = (atoi(sValue) == 0) ? 0 : 1;
        }
    } else {
        read_anpr_auto_backup(SQLITE_FILE_NAME, &info);
        if (!sdkp2p_get_section_info(buf, "anprEnable=", sValue, sizeof(sValue))) {
            info.enable = (atoi(sValue) == 0) ? 0 : 1;
        }
    }

    if (!sdkp2p_get_section_info(buf, "backup_day=", sValue, sizeof(sValue))) {
        info.backup_day = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "backup_time=", sValue, sizeof(sValue))) {
        snprintf(info.backup_time, sizeof(info.backup_time), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "stream_type=", sValue, sizeof(sValue))) {
        info.stream_type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "file_type=", sValue, sizeof(sValue))) {
        info.stream_type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "recycle=", sValue, sizeof(sValue))) {
        info.stream_type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "tri_channels=", sValue, sizeof(sValue))) {
        snprintf(info.backup_time, sizeof(info.backup_time), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "device=", sValue, sizeof(sValue))) {
        info.device = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        info.port = atoi(sValue);
    }

    if (backupType == 0) {
        write_esata_auto_backup(SQLITE_FILE_NAME, &info);
    } else {
        write_anpr_auto_backup(SQLITE_FILE_NAME, &info);
    }
    sdkp2p_send_msg(conf, conf->req, (void *)&backupType, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_record_set_esata_backup(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_record_get_esata_backup_status(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_esata_backup_status *info = (struct resp_esata_backup_status *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", info->status);
    snprintf(buf + strlen(buf), len - strlen(buf), "last_time=%s&", info->last_time);
    snprintf(buf + strlen(buf), len - strlen(buf), "percent=%.lf&", info->percent);
    snprintf(buf + strlen(buf), len - strlen(buf), "capacity=%llu&", info->capacity);

    *datalen = strlen(buf) + 1;
}

static void resp_record_get_export_disk(void *param, int size, char *buf, int len, int *datalen)
{
    int i, cnt;
    int prot = -1;
    struct resp_usb_info *usb_info_list = (struct resp_usb_info *)param;

    cnt = size / sizeof(struct resp_usb_info);
    for (i = 0; i < cnt; ++i) {
        if (usb_info_list[i].type == DISK_TYPE_ESATA) {
            prot = usb_info_list[i].port;
            break;
        }
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "prot=%d&", prot);
    *datalen = strlen(buf) + 1;
}

static void req_record_format_esata_disk(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int port = -1;

    if (sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    port = atoi(sValue);
    if (port < 0 || port > MAX_MSDK_NUM) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&port, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_record_format_esata_disk(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_record_get_auto_backup_status(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void resp_record_get_format_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct RespForamtInfo *resp = (struct RespForamtInfo *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", resp->esataStatus);
    snprintf(buf + strlen(buf), len - strlen(buf), "percent=%d&", (int)resp->percent);
    snprintf(buf + strlen(buf), len - strlen(buf), "port=%d&", (int)resp->port);
    snprintf(buf + strlen(buf), len - strlen(buf), "type=%d&", (int)resp->type);
    *datalen = strlen(buf) + 1;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_RECORD_ADVANCED, req_record_get_record_advanced, -1, NULL},//get.record.advanced
    {REQUEST_FLAG_SET_RECORD_ADVANCED, req_record_set_record_advanced, -1, NULL},//set.record.advanced
    {REQUEST_FLAG_RECORD_TASK, record_task_to_struct, RESPONSE_FLAG_RECORD_TASK, record_task_to_str},//set.record.record
    {REQUEST_FLAG_SET_RECORDMODE, req_record_set_record_mode, RESPONSE_FLAG_SET_RECORDMODE, NULL},//set.record.mode
    {REQUEST_FLAG_GET_RECORD_SCHEDULE, req_record_get_record_schedule, -1, NULL},//get.record.schedule
    {REQUEST_FLAG_SET_RECORDSCHED, req_record_set_record_schedule, -1, NULL},//set.record.schedule
    {REQUEST_FLAG_SET_ALL_RECORD, req_record_set_all_record, RESPONSE_FLAG_SET_ALL_RECORD, NULL},//set.record.batchsetting
    {REQUEST_FLAG_GET_SNAPSHOT, req_record_get_snapshot_param, RESPONSE_FLAG_GET_SNAPSHOT, NULL},//get_record.snapshot_param
    {REQUEST_FLAG_SET_SNAPSHOT, req_record_set_snapshot_param, RESPONSE_FLAG_SET_SNAPSHOT, NULL},//set_record.snapshot_param
    {REQUEST_FLAG_GET_SNAPSHOT_SCHE, req_record_get_snapshot_schedule, RESPONSE_FLAG_GET_SNAPSHOT_SCHE, NULL},//get_record.snapshot_schedule
    {REQUEST_FLAG_SET_SNAPSHOT_SCHE, req_record_set_snapshot_schedule, RESPONSE_FLAG_SET_SNAPSHOT_SCHE, NULL},//set_record.snapshot_schedule
    {REQUEST_FLAG_LIVE_SNAPSHOT_PHOTO, req_record_get_live_snapshot, RESPONSE_FLAG_LIVE_SNAPSHOT_PHOTO, resp_record_get_live_snapshot},//get.record.live_snapshot
    {REQUEST_FLAG_GET_ESATA_AUTO_BACKUP, req_record_get_esata_backup, -1, NULL},//get.record.esata_auto_backup
    {REQUEST_FLAG_SET_ESATA_AUTO_BACKUP, req_record_set_esata_backup, RESPONSE_FLAG_SET_ESATA_AUTO_BACKUP, resp_record_set_esata_backup},//set.record.esata_auto_backup
    {-1, NULL, RESPONSE_FLAG_GET_ESATA_BACKUP_STATUS, resp_record_get_esata_backup_status},//get.record.esata_backup_status
    {-1, NULL, RESPONSE_FLAG_GET_EXPORT_DISK, resp_record_get_export_disk},//get.record.export_disk
    {REQUEST_FLAG_FORMAT_ESATA_DISK, req_record_format_esata_disk, RESPONSE_FLAG_FORMAT_ESATA_DISK, resp_record_format_esata_disk},//set.record.esata_disk_format
    {-1, NULL, RESPONSE_FLAG_GET_AUTO_BACKUP_STATUS, resp_record_get_auto_backup_status},//get.record.auto_backup_status
    {-1, NULL, RESPONSE_FLAG_GET_FORMAT_INFO_REMOTE, resp_record_get_format_info},//get.record.format_info
    // request register array end
};


void p2p_record_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_record_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

