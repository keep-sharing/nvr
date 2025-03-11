#include <sys/time.h>
#include <time.h>
#include "msdefs.h"
#include "msfs/msfs_rec.h"
#include "msfs/msfs_pb.h"
#include "sdk_util.h"

int get_playback_month_json(void *data, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }

    int i;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    char *sResp = NULL;
    struct pb_month_t *month = (struct pb_month_t *)(data);

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "valid", month->bHave);
    array = cJSON_CreateArray();
    for (i = 0; i < MONTH_MAX_DAY; i++) {
        cJSON_AddItemToArray(obj, cJSON_CreateNumber(month->enEvent[i]));
    }
    cJSON_AddItemToObject(json, "event", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_playback_common_record_info_json(void *data, int cnt, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }

    int i;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    char *sResp = NULL;
    struct resp_search_common_backup *record = (struct resp_search_common_backup *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "cnt", cnt);
    
    array = cJSON_CreateArray();
    for (i = 0; i < cnt; i++) {
        if (i == 0) {
            cJSON_AddNumberToObject(json, "allCnt", record[i].allCnt);
            cJSON_AddNumberToObject(json, "allSize", record[i].allSize);
            cJSON_AddNumberToObject(json, "sid", record[i].sid);
        }
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "chnid", record[i].chnid);
        cJSON_AddNumberToObject(obj, "size", record[i].size);
        cJSON_AddStringToObject(obj, "startTime", record[i].pStartTime);
        cJSON_AddStringToObject(obj, "endTime", record[i].pEndTime);
        cJSON_AddNumberToObject(obj, "port", record[i].port);
        cJSON_AddNumberToObject(obj, "lock", record[i].isLock);
        cJSON_AddNumberToObject(obj, "recordType", record[i].enEvent);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "info", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_playback_picture_record_info_json(void *data, int cnt, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }

    int i;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    char *sResp = NULL;
    struct resp_search_picture_backup *record = (struct resp_search_picture_backup *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "cnt", cnt);
    
    array = cJSON_CreateArray();
    for (i = 0; i < cnt; i++) {
        if (i == 0) {
            cJSON_AddNumberToObject(json, "allCnt", record[i].allCnt);
            cJSON_AddNumberToObject(json, "allSize", record[i].allSize);
            cJSON_AddNumberToObject(json, "sid", record[i].sid);
        }
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "chnid", record[i].chnid);
        cJSON_AddNumberToObject(obj, "port", record[i].port);
        cJSON_AddNumberToObject(obj, "size", record[i].size);
        cJSON_AddStringToObject(obj, "pTime", record[i].pTime);
        cJSON_AddNumberToObject(obj, "major", record[i].enMajor);
        cJSON_AddNumberToObject(obj, "width", record[i].width);
        cJSON_AddNumberToObject(obj, "height", record[i].height);
        cJSON_AddNumberToObject(obj, "pts", record[i].pts);
        cJSON_AddNumberToObject(obj, "objectType", record[i].objectType);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "info", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_playback_common_base_info_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }

    int i;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    char *sResp = NULL;
    char *picData = NULL;
    int cnt = 0;
    struct RespPbGeneralInfo *header = (struct RespPbGeneralInfo *)data;
    struct resp_search_common_backup *record;

    if (header && header->recordSize) {
        cnt = header->recordSize / sizeof(struct resp_search_common_backup);
        record = (struct resp_search_common_backup *)(data + sizeof(struct RespPbGeneralInfo));
    }

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "cnt", cnt);
    array = cJSON_CreateArray();
    for (i = 0; i < cnt; i++) {
        if (i == 0) {
            cJSON_AddNumberToObject(json, "allCnt", record[i].allCnt);
            cJSON_AddNumberToObject(json, "allSize", record[i].allSize);
            cJSON_AddNumberToObject(json, "sid", record[i].sid);
        }
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "chnid", record[i].chnid);
        cJSON_AddNumberToObject(obj, "size", record[i].size);
        cJSON_AddStringToObject(obj, "startTime", record[i].pStartTime);
        cJSON_AddStringToObject(obj, "endTime", record[i].pEndTime);
        cJSON_AddNumberToObject(obj, "port", record[i].port);
        cJSON_AddNumberToObject(obj, "lock", record[i].isLock);
        cJSON_AddNumberToObject(obj, "recordType", record[i].enEvent);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "info", array);
    if (header && header->picSize) {
        picData = get_base64_encode(data+sizeof(struct RespPbGeneralInfo) + header->recordSize, header->picSize);
    }

    if (picData) {
        cJSON_AddStringToObject(json, "pic", picData);
        ms_free(picData);
    } else {
        cJSON_AddStringToObject(json, "pic", "null");
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


int get_playback_event_record_info_json(void *data, int cnt, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }

    int i;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    char *sResp = NULL;
    struct resp_search_event_backup *record = (struct resp_search_event_backup *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "cnt", cnt);
    
    array = cJSON_CreateArray();
    for (i = 0; i < cnt; i++) {
        if (i == 0) {
            cJSON_AddNumberToObject(json, "allCnt", record[i].allCnt);
            cJSON_AddNumberToObject(json, "sid", record[i].sid);
        }
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "chnid", record[i].chnid);
        cJSON_AddNumberToObject(obj, "size", record[i].size);
        cJSON_AddStringToObject(obj, "startTime", record[i].pStartTime);
        cJSON_AddStringToObject(obj, "endTime", record[i].pEndTime);
        cJSON_AddNumberToObject(obj, "port", record[i].port);
        cJSON_AddStringToObject(obj, "source", record[i].source);
        cJSON_AddNumberToObject(obj, "objectType", record[i].objectType);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "info", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

int get_playback_play_json(void *data, int cnt, char *resp, int respLen)
{
    if (!data || !resp || respLen <= 0) {
        return -1;
    }

    cJSON *json = NULL;
    char *sResp = NULL;
    struct resp_get_remotepb_rtspaddr *info = (struct resp_get_remotepb_rtspaddr *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "sessionId", info->sid);
    cJSON_AddNumberToObject(json, "port", info->port);
    cJSON_AddNumberToObject(json, "external_port", info->external_port);
    cJSON_AddNumberToObject(json, "external_http_port", info->external_http_port);
    cJSON_AddNumberToObject(json, "ddns_port", info->ddns_port);
    cJSON_AddNumberToObject(json, "ddns_http_port", info->ddns_http_port);
    cJSON_AddStringToObject(json, "sourcePath", info->respath);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);

    return 0;
}

SDK_ERROR_CODE sdk_str_to_req_pushmsg_pictures(REQ_PUSHMSG_PICTURES_S *dst, const char *src)
{
    if (!dst || !src) {
        msdebug(DEBUG_INF, "arg err, dst = %p, src = %p", dst, src);
        return PARAM_ERROR;
    }

    char sValue[256] = {0};
    
    if (sdkp2p_get_section_info(src, "chnMask=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    dst->chnMask = (Uint64)sdkp2p_get_chnmask_to_atoll(sValue);
    if (!dst->chnMask) {
        return PARAM_ERROR;
    }
    
    if (sdkp2p_get_section_info(src, "time=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    dst->time = atol(sValue);
    
    if (!sdkp2p_get_section_info(src, "width=", sValue, sizeof(sValue))) {
        dst->width = atoi(sValue);
    } else {
        dst->width = 1280;
    }

    if (!sdkp2p_get_section_info(src, "height=", sValue, sizeof(sValue))) {
        dst->height = atoi(sValue);
    } else {
        dst->height = 720;
    }

    return REQUEST_SUCCESS;
}

static void get_pbdayinfo_msfs_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    //r.time=2018-07-05 00:00:00
    struct rep_get_month_event *res = (struct rep_get_month_event *)param;
    char sValue[256] = {0};
    struct tm t_time;
    int chnid = -1, i = 0;

    if (!sdkp2p_get_section_info(buf, "r.time=", sValue, sizeof(sValue))) {
        strptime(sValue, "%Y-%m-%d %H:%M:%S", &t_time);
        res->year = t_time.tm_year + 1900;
        res->month = t_time.tm_mon + 1;
    }

    if (!sdkp2p_get_section_info(buf, "r.stream=", sValue, sizeof(sValue))) {
        res->enType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    for (i = 0; i < MAX_LEN_64; i++) {
        res->chnMaskl[i] = '0';
    }

    if (chnid < MAX_LEN_64 && chnid >= 0) {
        res->chnMaskl[chnid] = '1';
    }

    *datalen = sizeof(struct rep_get_month_event);

    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void get_pbdayinfo_msfs_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct pb_month_t *
    struct pb_month_t *res = (struct pb_month_t *)param;
    char tmp[32] = {0};
    int i = 0;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.valid=%d&", res->bHave);
    for (i = 0 ; i < 31; i++) {
        if (res->enEvent[i] == REC_EVENT_NONE) {
            tmp[i] = '0';
        } else {
            tmp[i] = '1';
        }

        snprintf(buf + strlen(buf), len - strlen(buf), "r.event%d=%u&", i, res->enEvent[i]);
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "r.dayinfo=%s&", tmp);

    *datalen = strlen(buf) + 1;
}

static void get_msfs_pbhourinfo_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_search_common_backup *res = (struct req_search_common_backup *)param;
    char sValue[256] = {0};
    int i = 0;
    int chnid = -1;

    memset(res, 0x0, sizeof(struct req_search_common_backup));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    memset(res, 0x0, sizeof(struct req_search_common_backup));
    if (!sdkp2p_get_section_info(buf, "r.chnMaskl=", sValue, sizeof(sValue))) {
        snprintf(res->chnMaskl, sizeof(res->chnMaskl), "%s", sValue);
    }

    if (chnid >= 0 && chnid < MAX_CAMERA) {
        res->chnMaskl[chnid] = '1';
    }

    for (i = 0; i < MAX_LEN_64; i++) {
        if (res->chnMaskl[i] == '1') {
            res->chnNum++;
        }
    }

    res->enEvent = REC_EVENT_ALL;
    if (!sdkp2p_get_section_info(buf, "r.enEvent=", sValue, sizeof(sValue))) {
        res->enEvent = atoi(sValue);
    }

    res->enState = SEG_STATE_ALL;
    if (!sdkp2p_get_section_info(buf, "r.enState=", sValue, sizeof(sValue))) {
        res->enState = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.stream=", sValue, sizeof(sValue))) {
        res->enType = atoi(sValue);
        if (res->enType != FILE_TYPE_SUB) {
            res->enType = FILE_TYPE_MAIN;
        }
    }


    if (!sdkp2p_get_section_info(buf, "r.startTime=", sValue, sizeof(sValue))) {
        snprintf(res->pStartTime, sizeof(res->pStartTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.endTime=", sValue, sizeof(sValue))) {
        snprintf(res->pEndTime, sizeof(res->pEndTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.all=", sValue, sizeof(sValue))) {
        res->all = atoi(sValue);
    }
    
    snprintf(res->ipaddr, sizeof(res->ipaddr), "%s", conf->ip_addr);

    *datalen = sizeof(struct req_search_common_backup);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void get_msfs_pbhourinfo_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct pb_month_t *
    struct resp_search_common_backup *res = (struct resp_search_common_backup *)param;
    int num = size / sizeof(struct resp_search_common_backup);
    int i = 0;
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", num);

    if (num > 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "sid=%u&", res[0].sid);
        snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", res[0].chnid);
        snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", res[0].allCnt);
        snprintf(buf + strlen(buf), len - strlen(buf), "allSize=%lld&", res[0].allSize);
        for (i = 0; i < num; i++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].startTime=%s&", i, res[i].pStartTime);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].endTime=%s&", i, res[i].pEndTime);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].recordType=%d&", i, res[i].enEvent);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].lock=%d&", i, res[i].isLock);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].chnid=%d&", i, res[i].chnid);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].port=%d&", i, res[i].port);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].size=%d&", i, res[i].size);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void get_msfs_pbhourinfo_page_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    REQ_GET_SEARCH_BACKUP *res = (REQ_GET_SEARCH_BACKUP *)param;
    char sValue[256] = {0};

    memset(res, 0x0, sizeof(REQ_GET_SEARCH_BACKUP));
    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        res->sid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.page=", sValue, sizeof(sValue))) {
        res->npage = atoi(sValue);
    }

    *datalen = sizeof(REQ_GET_SEARCH_BACKUP);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void get_msfs_pbhourinfo_page_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    get_msfs_pbhourinfo_to_str(param, size, buf, len, datalen);
}

static void set_msfs_pbplay_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct rep_play_common_backup *res = (struct rep_play_common_backup *)param;
    char sValue[256] = {0};

    memset(res, 0x0, sizeof(struct rep_play_common_backup));
    res->sid = -1;
    res->chnid = -1;
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chnid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        res->sid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.startTime=", sValue, sizeof(sValue))) {
        snprintf(res->pStartTime, sizeof(res->pStartTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.endTime=", sValue, sizeof(sValue))) {
        snprintf(res->pEndTime, sizeof(res->pEndTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.playTime=", sValue, sizeof(sValue))) {
        snprintf(res->pPlayTime, sizeof(res->pPlayTime), "%s", sValue);
    }

    *datalen = sizeof(struct rep_play_common_backup);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_PLAYBACK_REMOTE, SUB_PARAM_NONE, res->chnid + 1, 0, NULL, 0);
}

static void get_msfs_pbplay_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    int *res = (int *)param;
    int ret = *res;
    if (ret < 0) {
        ret = -1;
    } else {
        ret = 0;
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", ret);

    *datalen = strlen(buf) + 1;
}

static void set_msfs_pbpause_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_playback_ctrl_batch *res = (struct req_playback_ctrl_batch *)param;
    char sValue[256] = {0};
    int i, cnt = 0, sid = 0;;
    char temp[256];

    memset(res, 0x0, sizeof(struct req_playback_ctrl_batch));
    if (!sdkp2p_get_section_info(buf, "r.cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }

    for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
        snprintf(temp, 256, "r.sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            sscanf(sValue, "%d", &sid);
            res->sid[res->num] = sid;
            res->num++;
        }
    }
    *datalen = sizeof(struct req_playback_ctrl_batch);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void set_msfs_pbrestart_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_remote_pb_resume info;
    char sValue[256] = {0};

    memset(&info, 0, sizeof(struct req_remote_pb_resume));
    info.sid = -1;
    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        info.sid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.direction=", sValue, sizeof(sValue))) {
        info.direction = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_remote_pb_resume), SDKP2P_NEED_CALLBACK);
}

static void set_msfs_pbseek_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_seek_time_batch *res = (struct req_seek_time_batch *)param;
    char sValue[256] = {0};
    int i, cnt = 0, sid = 0;;
    char temp[256];

    memset(res, 0x0, sizeof(struct req_seek_time_batch));
    if (!sdkp2p_get_section_info(buf, "r.cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }

    for (i = 0; i < cnt && i < MAX_LEN_64; i++) {
        snprintf(temp, 256, "r.sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            sscanf(sValue, "%d", &sid);
            res->sid[res->num] = sid;
            res->num++;
        }
    }

    if (!sdkp2p_get_section_info(buf, "r.jumpTime=", sValue, sizeof(sValue))) {
        snprintf(res->pseektime, sizeof(res->pseektime), "%s", sValue);
    }

    *datalen = sizeof(struct req_seek_time_batch);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void get_msfs_pbseek_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    int *res = (int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", *res);

    *datalen = strlen(buf) + 1;
}

static void set_msfs_pbstep_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_playback_ctrl_batch *res = (struct req_playback_ctrl_batch *)param;
    char sValue[256] = {0};
    int i, cnt = 0, sid = 0;;
    char temp[256];

    memset(res, 0x0, sizeof(struct req_playback_ctrl_batch));
    if (!sdkp2p_get_section_info(buf, "r.cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }

    for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
        snprintf(temp, 256, "r.sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            sscanf(sValue, "%d", &sid);
            res->sid[res->num] = sid;
            res->num++;
        }
    }

    if (!sdkp2p_get_section_info(buf, "r.direct=", sValue, sizeof(sValue))) {
        res->direct = atoi(sValue);
        if (res->direct != 1) {
            res->direct = 0;//0=forward 1=backward
        }
    }

    *datalen = sizeof(struct req_playback_ctrl_batch);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void set_msfs_pbstop_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_playback_ctrl_batch *res = (struct req_playback_ctrl_batch *)param;
    char sValue[256] = {0};
    int i, cnt = 0, sid = 0;;
    char temp[256];

    memset(res, 0x0, sizeof(struct req_playback_ctrl_batch));
    if (!sdkp2p_get_section_info(buf, "r.num=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }

    for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
        snprintf(temp, 256, "r.sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            sscanf(sValue, "%d", &sid);
            res->sid[res->num] = sid;
            res->num++;
        }
    }

    *datalen = sizeof(struct req_playback_ctrl_batch);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}


static void set_msfs_pbclose_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_playback_ctrl_batch *res = (struct req_playback_ctrl_batch *)param;
    char sValue[256] = {0};
    int i, cnt = 0, sid = 0;;
    char temp[256];

    memset(res, 0x0, sizeof(struct req_playback_ctrl_batch));
    if (!sdkp2p_get_section_info(buf, "r.num=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }

    for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
        snprintf(temp, 256, "r.sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            sscanf(sValue, "%d", &sid);
            res->sid[res->num] = sid;
            res->num++;
        }
    }

    *datalen = sizeof(struct req_playback_ctrl_batch);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void get_msfs_playtime_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }
    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void get_pbplaytime_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    long res = *(long *)param;

    char playtime[32] = {0};
    struct tm *p_tm;
    p_tm  = localtime(&res);
    strftime(playtime, sizeof(playtime), "%Y-%m-%d %H:%M:%S", p_tm);

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%s&", playtime);
    snprintf(buf + strlen(buf), len - strlen(buf), "sec=%ld&", res);

    *datalen = strlen(buf) + 1;
}

static void set_msfs_pbspeed_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_set_all_play_speed_remote *res = (struct req_set_all_play_speed_remote *)param;
    char sValue[256] = {0};
    char temp[256];
    int i = 0, cnt = 0, sid = 0;

    memset(res, 0x0, sizeof(struct req_set_all_play_speed_remote));
    if (!sdkp2p_get_section_info(buf, "r.cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }

    for (i = 0; i < cnt; i++) {
        snprintf(temp, 256, "r.sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            sscanf(sValue, "%d", &sid);
            res->sid[res->num] = sid;
            res->num++;
        }
    }

    if (!sdkp2p_get_section_info(buf, "r.speed=", sValue, sizeof(sValue))) {
        res->speed = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.playallframe=", sValue, sizeof(sValue))) {
        res->playallframe = atoi(sValue);
    }
    *datalen = sizeof(struct req_playback_ctrl_batch);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void p2p_pb_stop_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_p2p_pb_para *res = (struct req_p2p_pb_para *)param;

    char sValue[256] = {0};
    int i, cnt = 0, sid = 0;;
    char temp[256];

    memset(res, 0x0, sizeof(struct req_playback_ctrl_batch));
    if (!sdkp2p_get_section_info(buf, "r.cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }

    for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
        snprintf(temp, 256, "r.sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, temp, sValue, sizeof(sValue))) {
            sscanf(sValue, "%d", &sid);
            res->sid[res->num] = sid;
            res->num++;
        }
    }

    if (!sdkp2p_get_section_info(buf, "r.close=", sValue, sizeof(sValue))) {
        res->close = atoi(sValue);
        if (res->close != 1) {
            res->close = 0;
        }
    }

    *datalen = sizeof(struct req_p2p_pb_para);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void p2p_pb_set_trancode_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_set_trancode *res = (struct req_set_trancode *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(struct req_set_trancode));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chnid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        res->sid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.trancodingmode=", sValue, sizeof(sValue))) {
        res->mode = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.framesize=", sValue, sizeof(sValue))) {
        res->framesize = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.bitrate=", sValue, sizeof(sValue))) {
        res->bitrate = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.framerate=", sValue, sizeof(sValue))) {
        res->framerate = atoi(sValue);
    }
    *datalen = sizeof(struct req_set_trancode);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void playback_set_trancode_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_set_trancode  *res = (struct resp_set_trancode *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", res->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sid=%u&", res->sid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.state=%d&", res->state);

    //msprintf("[set_trancode] chnid:%d sid:%d state:%d", res->chnid, res->sid, res->state);
    *datalen = strlen(buf) + 1;
}

static void p2p_pb_get_trancode_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_get_trancode *res = (struct req_get_trancode *)param;
    char sValue[256] = {0};
    memset(res, 0x0, sizeof(struct req_get_trancode));
    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        res->chnid = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        res->sid = atoi(sValue);
    }
    *datalen = sizeof(struct req_get_trancode);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void playback_get_trancode_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_get_trancode  *res = (struct resp_get_trancode *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.chnid=%d&", res->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sid=%u&", res->sid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.state=%d&", res->state);

    //msprintf("[get_trancode] chnid:%d sid:%d state:%d", res->chnid, res->sid, res->state);
    *datalen = strlen(buf) + 1;
}

static void remove_remotepb_stream_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.ctrl->sid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }
    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void add_remotelive_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_remote_live *res = (struct req_remote_live *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.chanid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->chanid));
    }
    if (!sdkp2p_get_section_info(buf, "r.stream=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->stream));
    }
    if (!sdkp2p_get_section_info(buf, "r.stream_from=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->stream_from));
    }
    *datalen = sizeof(struct req_remote_live);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void add_remotelive_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_remote_live;
    struct resp_remote_live *res = (struct resp_remote_live *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.chanid=%d&", res->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.port=%d&", res->port);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.direct=%d&", res->direct);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.result=%d&", res->result);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.username=%s&", res->username);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.password=%s&", res->password);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.rtspaddr=%s&", res->rtspaddr);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.respath=%s&", res->respath);

    *datalen = strlen(buf) + 1;
}

static void get_remotepb_rtspaddr_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
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

static void get_remotepb_rtspaddr_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_get_remotepb_rtspaddr;
    struct resp_get_remotepb_rtspaddr *res = (struct resp_get_remotepb_rtspaddr *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.sid=%u&", res->sid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.port=%d&", res->port);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.respath=%s&", res->respath);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.external_port=%d&", res->external_port);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.external_http_port=%d&", res->external_http_port);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.ddns_port=%d&", res->ddns_port);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.ddns_http_port=%d&", res->ddns_http_port);

    *datalen = strlen(buf) + 1;
}

static void set_playback_stop(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_playback_ctrl *res = (struct req_playback_ctrl *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->sid));
    }
    if (!sdkp2p_get_section_info(buf, "r.direct=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->direct));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed));
    }
    if (!sdkp2p_get_section_info(buf, "r.sec=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%ld", &(res->sec));
    }
    *datalen = sizeof(struct req_playback_ctrl);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void playback_remote_chns_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_playback_remote_masks *res = (struct req_playback_remote_masks *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.chans=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%lld", &(res->chans));
    }
    if (!sdkp2p_get_section_info(buf, "r.time=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%ld", &(res->time));
    }

    *datalen = sizeof(struct req_playback_remote_masks);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void playback_remote_chns_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_playback_remote
    struct resp_playback_remote *res = (struct resp_playback_remote *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.sid=%u&", res->sid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.real_time=%s&", res->real_time);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.except=%d&", res->except);

    *datalen = strlen(buf) + 1;
}

static void req_p2p_set_evt_backup_close(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void get_evt_backup_close_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "r.res=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_p2p_get_evt_backup_search(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_search_event_backup *res = (struct req_search_event_backup *)param;
    char sValue[MAX_LEN_512] = {0};
    char nvrAlarm[MAX_LEN_64 + 1] = {0};
    char ipcAlarm[MAX_IPC_ALARM_IN][MAX_LEN_64 + 1];
    int i = 0, j = 0;
    char tmp[64] = {0};
    int nHeaderSize = sizeof(struct req_search_event_backup);
    int nBodySize = 0;
    int nSize = nHeaderSize + nBodySize;
    int offset = nHeaderSize;

    memset(res, 0, sizeof(struct req_search_event_backup));
    if (!sdkp2p_get_section_info(buf, "r.chnmaskl=", sValue, sizeof(sValue))) {
        snprintf(res->chnMaskl, sizeof(res->chnMaskl), "%s", sValue);
    }
    for (i = 0; i < MAX_CAMERA; i++) {
        if (res->chnMaskl[i] == '1') {
            res->chnNum++;
        }
    }

    if (!sdkp2p_get_section_info(buf, "r.num=", sValue, sizeof(sValue))) {
        res->chnNum = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.event=", sValue, sizeof(sValue))) {
        res->enMajor = atoi(sValue);//@see INFO_MAJOR_EN
    }

    if (!sdkp2p_get_section_info(buf, "r.subType=", sValue, sizeof(sValue))) {
        res->enMinor = atoi(sValue);//@see INFO_MINOR_EN
    }

    if (!sdkp2p_get_section_info(buf, "r.stream=", sValue, sizeof(sValue))) {
        res->enType = atoi(sValue);//@see FILE_TYPE_EN
    }

    if (!sdkp2p_get_section_info(buf, "r.ahead=", sValue, sizeof(sValue))) {
        res->ahead = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.delay=", sValue, sizeof(sValue))) {
        res->delay = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.startTime=", sValue, sizeof(sValue))) {
        snprintf(res->pStartTime, sizeof(res->pStartTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.endTime=", sValue, sizeof(sValue))) {
        snprintf(res->pEndTime, sizeof(res->pEndTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.map=", sValue, sizeof(sValue))) {
        snprintf(res->pMotMap, sizeof(res->pMotMap), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.objtype=", sValue, sizeof(sValue))) {
        res->objtype = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.all=", sValue, sizeof(sValue))) {
        res->all = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.close=", sValue, sizeof(sValue))) {
        res->close = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.limitNum=", sValue, sizeof(sValue))) {
        res->maxNum = atoi(sValue);
    }

    snprintf(res->ipaddr, sizeof(res->ipaddr), "%s", conf->ip_addr);

    if (res->enMajor == INFO_MAJOR_ALARMIN) {
        memset(nvrAlarm, 0x0, sizeof(nvrAlarm));
        memset(ipcAlarm, 0x0, sizeof(ipcAlarm));

        if (!sdkp2p_get_section_info(buf, "nvrAlarm=", sValue, sizeof(sValue))) {
            snprintf(nvrAlarm, sizeof(nvrAlarm), "%s", sValue);
        }

        /*
            旧版本参数是ipcAlarm1 ipcAlarm2对应0~1
            【Web-Playback/Retrieve：Eevnt Playback/Event Backup界面选择Alarm Input，勾选CH1_1，搜索出的为CH_2的事件录像】
            https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001076843
        */
        for (i = 1; i <= MAX_IPC_ALARM_IN; ++i) {
            snprintf(tmp, sizeof(tmp), "ipcAlarm%d=", i);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(ipcAlarm[i], sizeof(ipcAlarm[i]), "%s", sValue);
            }
        }

        for (i = 0; i < MAX_LEN_64; i++) {
            if (nvrAlarm[i] == '1') {
                res->enEventCnt++;
            }
            
            for (j = 0; j < MAX_IPC_ALARM_IN; ++j) {
                if (ipcAlarm[j][i] == '1') {
                    res->enEventCnt++;
                }
            }
        }
        res->pEventNameLen = MAX_LEN_64;
        nBodySize = res->pEventNameLen * res->enEventCnt;
        memset(param + nHeaderSize, 0x0, nBodySize);
        for (i = 0; i < MAX_LEN_64; i++) {
            if (nvrAlarm[i] == '1') {
                snprintf(param + offset, res->pEventNameLen, "%s %d", RECORD_TYPE_ALARM, i + 1);
                offset += res->pEventNameLen;
            }

            for (j = 0; j < MAX_IPC_ALARM_IN; ++j) {
                if (ipcAlarm[j][i] == '1') {
                    snprintf(param + offset, res->pEventNameLen, "CH%d_%d", i + 1, j + 1);
                    offset += res->pEventNameLen;
                }
            }
        }
    }

    nSize = nHeaderSize + nBodySize;
    sdkp2p_send_msg(conf, conf->req, res, nSize, SDKP2P_NEED_CALLBACK);
}

static void get_evt_backup_search_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    int cnt = size / sizeof(struct resp_search_event_backup);

    struct resp_search_event_backup *resp = (struct resp_search_event_backup *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.cnt=%d&", cnt);
    if (cnt > 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.sid=%u&", resp[0].sid);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.allCnt=%d&", resp[0].allCnt);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.allSize=%lld&", resp[0].allSize);
        for (i = 0; i < cnt; i++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].chnid=%d&", i, resp[i].chnid);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].startTime=%s&", i, resp[i].pStartTime);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].endTime=%s&", i, resp[i].pEndTime);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].recordType=%d&", i, resp[i].enEvent);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].source=%s&", i, resp[i].source);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].port=%d&", i, resp[i].port);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].size=%d&", i, resp[i].size);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].objectType=%d&", i, resp[i].objectType);
        }
    }

    *datalen = strlen(buf) + 1;
}

static void req_p2p_get_evt_backup_search_page(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct rep_get_search_backup *res = (struct rep_get_search_backup *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.sid=", sValue, sizeof(sValue))) {
        res->sid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.page=", sValue, sizeof(sValue))) {
        res->npage = atoi(sValue);
    }

    *datalen = sizeof(struct rep_get_search_backup);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}


static void req_p2p_get_anpr_backup_search(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0;
    struct req_search_anpr_backup search;

    memset(&search, 0x0, sizeof(struct req_search_anpr_backup));
    if (sdkp2p_get_section_info(buf, "chnMaskl=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    snprintf(search.chnMaskl, sizeof(search.chnMaskl), "%s", sValue);
    for (i = 0; i < MAX_CAMERA && i < strlen(search.chnMaskl); i++) {
        if (search.chnMaskl[i] == '1') {
            search.chnNum++;
        }

    }

    search.filter.type = MAX_ANPR_MODE;
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        search.filter.type = atoi(sValue);
        if (search.filter.type < ANPR_BLACK || search.filter.type > MAX_ANPR_MODE) {
            search.filter.type = MAX_ANPR_MODE;
        }
    }

    search.filter.plateColor = ANPR_COLOR_ALL;
    if (!sdkp2p_get_section_info(buf, "plateColor=", sValue, sizeof(sValue))) {
        search.filter.plateColor = atoi(sValue);
        if (search.filter.plateColor < ANPR_COLOR_NONE || search.filter.plateColor > ANPR_COLOR_ALL) {
            search.filter.plateColor = ANPR_COLOR_ALL;
        }
    }

    search.filter.vehicleType = ANPR_VEHICLE_ALL;
    if (!sdkp2p_get_section_info(buf, "vehicleType=", sValue, sizeof(sValue))) {
        search.filter.vehicleType = atoi(sValue);
        if (search.filter.vehicleType < ANPR_VEHICLE_NONE || search.filter.vehicleType > ANPR_VEHICLE_ALL) {
            search.filter.vehicleType = ANPR_VEHICLE_ALL;
        }
    }

    search.filter.vehicleColor = ANPR_COLOR_ALL;
    if (!sdkp2p_get_section_info(buf, "vehicleColor=", sValue, sizeof(sValue))) {
        search.filter.vehicleColor = atoi(sValue);
        if (search.filter.vehicleColor < ANPR_COLOR_NONE || search.filter.vehicleColor > ANPR_COLOR_ALL) {
            search.filter.vehicleColor = ANPR_COLOR_ALL;
        }
    }

    search.filter.speedType = ANPR_SPEED_ALL;
    if (!sdkp2p_get_section_info(buf, "speedType=", sValue, sizeof(sValue))) {
        search.filter.speedType = atoi(sValue);
        if (search.filter.speedType < ANPR_SPEED_NONE || search.filter.speedType > ANPR_SPEED_ALL) {
            search.filter.speedType = ANPR_SPEED_ALL;
        }
    }

    search.filter.speed = -1;
    if (!sdkp2p_get_section_info(buf, "speed=", sValue, sizeof(sValue))) {
        search.filter.speed = atoi(sValue);
    }

    search.filter.direction = ANPR_DIRECTION_ALL;
    if (!sdkp2p_get_section_info(buf, "direction=", sValue, sizeof(sValue))) {
        search.filter.direction = atoi(sValue);
        if (search.filter.direction < ANPR_DIRECTION_NONE || search.filter.direction > ANPR_DIRECTION_ALL) {
            search.filter.direction = ANPR_DIRECTION_ALL;
        }
    }

    if (!sdkp2p_get_section_info(buf, "key=", sValue, sizeof(sValue))) {
        snprintf(search.keyWord, sizeof(search.keyWord), "%s", sValue);
    }

    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(search.pStartTime, sizeof(search.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(search.pEndTime, sizeof(search.pEndTime), "%s", sValue);

    if (!sdkp2p_get_section_info(buf, "major=", sValue, sizeof(sValue))) {
        search.enMajor = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "minor=", sValue, sizeof(sValue))) {
        search.enMinor = atoi(sValue);
    }

    snprintf(search.ipaddr, sizeof(search.ipaddr), "%s", conf->ip_addr);

    sdkp2p_send_msg(conf, conf->req, &search, sizeof(struct req_search_anpr_backup), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_p2p_get_anpr_backup_search(void *param, int size, char *buf, int len, int *datalen)
{
    int offset = 0;
    char *resp = NULL;
    struct resp_search_anpr_backup *anpr_backup = (struct resp_search_anpr_backup *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", anpr_backup->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "sid=%u&", anpr_backup->sid);
    snprintf(buf + strlen(buf), len - strlen(buf), "port=%d&", anpr_backup->port);
    snprintf(buf + strlen(buf), len - strlen(buf), "index=%d&", anpr_backup->index);
    snprintf(buf + strlen(buf), len - strlen(buf), "time=%s&", anpr_backup->pTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "plate=%s&", anpr_backup->plate);
    snprintf(buf + strlen(buf), len - strlen(buf), "type=%s&", anpr_backup->ptype);
    snprintf(buf + strlen(buf), len - strlen(buf), "region=%s&", anpr_backup->region);
    snprintf(buf + strlen(buf), len - strlen(buf), "direction=%d&", anpr_backup->direction);
    snprintf(buf + strlen(buf), len - strlen(buf), "roiId=%d&", anpr_backup->roiId);
    snprintf(buf + strlen(buf), len - strlen(buf), "brand=%d&", anpr_backup->brand);
    snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", anpr_backup->allCnt);
    snprintf(buf + strlen(buf), len - strlen(buf), "allSize=%d&", anpr_backup->allSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "infoSize=%d&", anpr_backup->infoSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "bImageSize=%d&", anpr_backup->bImageSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "sImageSize=%d&", anpr_backup->sImageSize);

    if (anpr_backup->sImageSize > 0) {
        offset += sizeof(struct resp_search_anpr_backup);
        offset += anpr_backup->bImageSize;
        resp = get_base64_encode(param + offset, anpr_backup->sImageSize);
    }

    if (resp) {
        snprintf(buf + strlen(buf), len - strlen(buf), "sImg=%s", resp);
        ms_free(resp);
    }

    *datalen = strlen(buf) + 1;
}

static void req_p2p_get_anpr_backup_close(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int sid = 0;

    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    sid = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, &sid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_p2p_get_anpr_backup_close(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_p2p_get_anpr_backup_search_page(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    char sValue[256] = {0};
    REQ_GET_SEARCH_BACKUP info;

    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.sid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "index=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.npage = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(REQ_GET_SEARCH_BACKUP), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_p2p_get_anpr_backup_search_page(void *param, int size, char *buf, int len, int *datalen)
{
    int offset = 0;
    char *resp = NULL;
    struct resp_search_anpr_backup *anpr_backup = (struct resp_search_anpr_backup *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", anpr_backup->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "sid=%u&", anpr_backup->sid);
    snprintf(buf + strlen(buf), len - strlen(buf), "port=%d&", anpr_backup->port);
    snprintf(buf + strlen(buf), len - strlen(buf), "index=%d&", anpr_backup->index);
    snprintf(buf + strlen(buf), len - strlen(buf), "time=%s&", anpr_backup->pTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "plate=%s&", anpr_backup->plate);
    snprintf(buf + strlen(buf), len - strlen(buf), "type=%s&", anpr_backup->ptype);
    snprintf(buf + strlen(buf), len - strlen(buf), "region=%s&", anpr_backup->region);
    snprintf(buf + strlen(buf), len - strlen(buf), "direction=%d&", anpr_backup->direction);
    snprintf(buf + strlen(buf), len - strlen(buf), "roiId=%d&", anpr_backup->roiId);
    snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", anpr_backup->allCnt);
    snprintf(buf + strlen(buf), len - strlen(buf), "allSize=%d&", anpr_backup->allSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "infoSize=%d&", anpr_backup->infoSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "bImageSize=%d&", anpr_backup->bImageSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "sImageSize=%d&", anpr_backup->sImageSize);

    if (anpr_backup->sImageSize > 0) {
        offset += sizeof(struct resp_search_anpr_backup);
        offset += anpr_backup->bImageSize;
        resp = get_base64_encode(param + offset, anpr_backup->sImageSize);
    }

    if (resp) {
        snprintf(buf + strlen(buf), len - strlen(buf), "sImg=%s", resp);
        ms_free(resp);
    }

    *datalen = strlen(buf) + 1;
}

static void req_p2p_get_anpr_backup_bigimg(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    REQ_GET_SEARCH_BACKUP info;

    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.sid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "index=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.npage = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(REQ_GET_SEARCH_BACKUP), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_p2p_get_anpr_backup_bigimg(void *param, int size, char *buf, int len, int *datalen)
{
    int offset = 0;
    struct resp_search_anpr_backup *anpr_backup = (struct resp_search_anpr_backup *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", anpr_backup->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "sid=%u&", anpr_backup->sid);
    snprintf(buf + strlen(buf), len - strlen(buf), "port=%d&", anpr_backup->port);
    snprintf(buf + strlen(buf), len - strlen(buf), "index=%d&", anpr_backup->index);
    snprintf(buf + strlen(buf), len - strlen(buf), "time=%s&", anpr_backup->pTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "plate=%s&", anpr_backup->plate);
    snprintf(buf + strlen(buf), len - strlen(buf), "type=%s&", anpr_backup->ptype);
    snprintf(buf + strlen(buf), len - strlen(buf), "region=%s&", anpr_backup->region);
    snprintf(buf + strlen(buf), len - strlen(buf), "direction=%d&", anpr_backup->direction);
    snprintf(buf + strlen(buf), len - strlen(buf), "roiId=%d&", anpr_backup->roiId);
    snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", anpr_backup->allCnt);
    snprintf(buf + strlen(buf), len - strlen(buf), "allSize=%d&", anpr_backup->allSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "infoSize=%d&", anpr_backup->infoSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "bImageSize=%d&", anpr_backup->bImageSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "sImageSize=%d&", anpr_backup->sImageSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "%s", "bImg=");
    offset += sizeof(struct resp_search_anpr_backup);
    char *resp = get_base64_encode(param + offset, anpr_backup->bImageSize);

    if (resp) {
        snprintf(buf + strlen(buf), len - strlen(buf), "bImg=%s", resp);
        ms_free(resp);
    }

    *datalen = strlen(buf) + 1;
    return ;
}

static void req_playback_tags_search_open(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0;
    struct req_search_tags info;//

    memset(&info, 0x0, sizeof(struct req_search_tags));
    if (sdkp2p_get_section_info(buf, "chnMaskl=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);;
        return ;
    }
    snprintf(info.chnMaskl, sizeof(info.chnMaskl), "%s", sValue);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (info.chnMaskl[i] == '0') {
            continue;
        } else if (info.chnMaskl[i] == '1') {
            info.chnNum++;
        } else {
            break;
        }
    }

    if (sdkp2p_get_section_info(buf, "enType=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.enType = atoi(sValue);// @ FILE_TYPE_EN
    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    get_url_decode(sValue);
    snprintf(info.pStartTime, sizeof(info.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.pEndTime, sizeof(info.pEndTime), "%s", sValue);

    snprintf(info.ipaddr, sizeof(info.ipaddr), "%s", conf->ip_addr);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_search_tags), SDKP2P_NEED_CALLBACK);

    return;
}

static void resp_playback_tags_search_open(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    int cnt = 0;
    char encode[1024] = {0};

    struct resp_search_tags *resp = (struct resp_search_tags *)param;
    cnt = size / sizeof(struct resp_search_tags);

    if (cnt > 0 && resp && resp[0].allCnt > 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", cnt);
        snprintf(buf + strlen(buf), len - strlen(buf), "sid=%u&", resp[0].sid);
        snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", resp[0].allCnt);
        for (i = 0; i < cnt; i++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "chnid%d=%d&", i, resp[i].chnid);
            snprintf(buf + strlen(buf), len - strlen(buf), "id%d=%d&", i, resp[i].id);
            snprintf(buf + strlen(buf), len - strlen(buf), "index%d=%d&", i, resp[i].index);
            get_url_encode(resp[i].pName, strlen(resp[i].pName), encode, sizeof(encode));
            snprintf(buf + strlen(buf), len - strlen(buf), "name%d=%s&", i, encode);
            snprintf(buf + strlen(buf), len - strlen(buf), "time%d=%s&", i, resp[i].pTime);
        }
    } else {
        snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", 0);
    }

    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_tags_search_page(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    REQ_GET_SEARCH_BACKUP info;

    memset(&info, 0x0, sizeof(struct req_search_tags));
    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.sid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "npage=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.npage = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(REQ_GET_SEARCH_BACKUP), SDKP2P_NEED_CALLBACK);

    return;
}

static void resp_playback_tags_search_page(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    int cnt = 0;
    char encode[1024] = {0};

    struct resp_search_tags *resp = (struct resp_search_tags *)param;
    cnt = size / sizeof(struct resp_search_tags);

    if (cnt > 0 && resp && resp[0].allCnt > 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", cnt);
        snprintf(buf + strlen(buf), len - strlen(buf), "sid=%u&", resp[0].sid);
        snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", resp[0].allCnt);
        for (i = 0; i < cnt; i++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "chnid%d=%d&", i, resp[i].chnid);
            snprintf(buf + strlen(buf), len - strlen(buf), "id%d=%d&", i, resp[i].id);
            snprintf(buf + strlen(buf), len - strlen(buf), "index%d=%d&", i, resp[i].index);
            get_url_encode(resp[i].pName, strlen(resp[i].pName), encode, sizeof(encode));
            snprintf(buf + strlen(buf), len - strlen(buf), "name%d=%s&", i, encode);
            snprintf(buf + strlen(buf), len - strlen(buf), "time%d=%s&", i, resp[i].pTime);
        }
    } else {
        snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", 0);
    }

    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_tags_search_close(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[64] = {0};
    int i = 0, cnt = 0;
    struct req_playback_ctrl_batch info;
    memset(&info, 0x0, sizeof(struct req_playback_ctrl_batch));

    if (sdkp2p_get_section_info(buf, "cnt=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    cnt = atoi(sValue);
    for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
        snprintf(tmp, sizeof(tmp), "sid[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.sid[info.num] = atoi(sValue);
            info.num++;
        }
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_playback_ctrl_batch), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_tags_search_close(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);

    *datalen = strlen(buf) + 1;
    return;
}


static void req_playback_set_commom_backup_lock(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct rep_lock_common_backup info;
    memset(&info, 0x0, sizeof(struct rep_lock_common_backup));

    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.sid = atoi(sValue);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.chnid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "size=", sValue, sizeof(sValue))) {
        info.size = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lock=", sValue, sizeof(sValue))) {
        info.isLock = atoi(sValue);
    }

    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    get_url_decode(sValue);
    snprintf(info.pStartTime, sizeof(info.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    get_url_decode(sValue);
    snprintf(info.pEndTime, sizeof(info.pEndTime), "%s", sValue);

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct rep_lock_common_backup), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_set_commom_backup_lock(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_common_state *resp = (struct resp_common_state *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", resp->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "state=%d&", resp->state);

    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_get_rec_range(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_bkp_record_range info;
    memset(&info, 0x0, sizeof(struct req_bkp_record_range));

    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        info.type = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "chnMaskl=", sValue, sizeof(sValue))) {
        snprintf(info.chnMaskl, sizeof(info.chnMaskl), "%s", sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_bkp_record_range), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_get_rec_range(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_bkp_record_range *resp = (struct resp_bkp_record_range *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "startTime=%ld&", resp->startTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "endTime=%ld&", resp->endTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "pStartTime=%s&", resp->pStartTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "pEndTime=%s&", resp->pEndTime);

    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_remote_backup_start(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_remote_msfs_backup info;

    memset(&info, 0x0, sizeof(struct req_remote_msfs_backup));

    info.type = GENERAL;
    if (!sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        info.comm.sid = atoi(sValue);
        info.type = EVENT;
    }

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.comm.chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.comm.pStartTime, sizeof(info.comm.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.comm.pEndTime, sizeof(info.comm.pEndTime), "%s", sValue);

    if (!sdkp2p_get_section_info(buf, "streamType=", sValue, sizeof(sValue))) {
        info.streamType = atoi(sValue);
    }
    if (info.streamType == 0) {
        info.streamType = 1 << FILE_TYPE_MAIN;
    } else if (info.streamType == 1) {
        info.streamType = 1 << FILE_TYPE_SUB;
    }

    if (!sdkp2p_get_section_info(buf, "videoFormat=", sValue, sizeof(sValue))) {
        info.videoFormat = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "streamData=", sValue, sizeof(sValue))) {
        info.streamData = atoi(sValue);
    }

    //p2p视频数据默认单帧传输
    info.sendMode = 1;
    if (!sdkp2p_get_section_info(buf, "sendMode=", sValue, sizeof(sValue))) {
        info.streamData = atoi(sValue);
    }
    if (access(REMOTE_PB_DOWNLOAD, F_OK) == 0) {
        snprintf(conf->resp, conf->size, "%s", "existing");
        *(conf->len) = strlen(conf->resp) + 1;
        return;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct req_remote_msfs_backup), SDKP2P_NEED_CALLBACK);
}

static void resp_playback_remote_backup_start(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_remote_msfs_backup *resp = (struct resp_remote_msfs_backup *)param;

    switch (resp->type) {
        case ANPR:
            if (!resp->anpr.res) {
                memcpy(buf, resp, sizeof(struct resp_remote_msfs_backup));
                *datalen = sizeof(struct resp_remote_msfs_backup);
            }
            break;

        case GENERAL:
        case EVENT:
            if (!resp->comm.res) {
                snprintf(buf + strlen(buf), len - strlen(buf), "fileSize=%lld&", resp->comm.fileSize);
                snprintf(buf + strlen(buf), len - strlen(buf), "fileName=%s&", resp->comm.fileName);
                *datalen = strlen(buf) + 1;
            }
            break;

        default:
            break;
    }

    return;
}

static void resp_playback_remote_backup_start_data(void *param, int size, char *buf, int len, int *datalen)
{
    memcpy(buf, param, size);
    *datalen = size;
    return;
}

static void req_playback_remote_backup_stop(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_remote_backup_stop(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_p2p_split_playback_range(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_search_split_pb_range info;

    memset(&info, 0x0, sizeof(struct req_search_split_pb_range));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chnid = atoi(sValue);
    if (info.chnid < 0 || info.chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "enType=", sValue, sizeof(sValue))) {
        info.enType = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "enEvent=", sValue, sizeof(sValue))) {
        info.enEvent = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "enState=", sValue, sizeof(sValue))) {
        info.enState = atoi(sValue);
    }

    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.pStartTime, sizeof(info.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.pEndTime, sizeof(info.pEndTime), "%s", sValue);

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_search_split_pb_range), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_p2p_split_playback_range(void *param, int size, char *buf, int len, int *datalen)
{
    char startTime[32] = {0};
    char endTime[32] = {0};
    struct resp_search_split_pb_range *resp = (struct resp_search_split_pb_range *)param;

    struct tm *p_tm  = localtime(&resp->startTime);
    strftime(startTime, sizeof(startTime), "%Y-%m-%d %H:%M:%S", p_tm);
    p_tm  = localtime(&resp->endTime);
    strftime(endTime, sizeof(endTime), "%Y-%m-%d %H:%M:%S", p_tm);

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", resp->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "startTime=%ld&", resp->startTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "endTime=%ld&", resp->endTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "pStartTime=%s&", startTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "pEndTime=%s&", endTime);

    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_common_picture(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct rep_play_common_backup info;

    memset(&info, 0x0, sizeof(struct rep_play_common_backup));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.sid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.pStartTime, sizeof(info.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.pEndTime, sizeof(info.pEndTime), "%s", sValue);

    if (!sdkp2p_get_section_info(buf, "flag=", sValue, sizeof(sValue))) {
        info.flag = atoi(sValue);
    } else {
        info.flag = 1;
    }
    
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct rep_play_common_backup), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_playback_common_picture(void *param, int size, char *buf, int len, int *datalen)
{
    snprintf(buf + strlen(buf), len - strlen(buf), "%s", "iFrame=");
    *datalen = strlen(buf) + size;
    memcpy(buf + strlen(buf), param, size);

    return;
}

static void req_playback_get_tags(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_playback_get_tags(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    int cnt = 0;
    char encode[256];
    struct resp_search_tags *resp = (struct resp_search_tags *)param;

    cnt = size / sizeof(struct resp_search_tags);
    if (cnt > 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", resp[0].chnid);
        for (i = 0; i < cnt; i++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "chnid[%d]=%d&", i, resp[i].chnid);
            snprintf(buf + strlen(buf), len - strlen(buf), "id[%d]=%d\r\n", i, resp[i].id);
            snprintf(buf + strlen(buf), len - strlen(buf), "index[%d]=%d\r\n", i, resp[i].index);
            get_url_encode(resp[i].pName, strlen(resp[i].pName), encode, sizeof(encode));
            snprintf(buf + strlen(buf), len - strlen(buf), "name[%d]=%s\r\n", i, encode);
            snprintf(buf + strlen(buf), len - strlen(buf), "time[%d]=%s\r\n", i, resp[i].pTime);
        }
    } else {
        snprintf(buf + strlen(buf), len - strlen(buf), "allCnt=%d&", 0);
    }
    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_add_tags(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct rep_set_tags_record info;

    memset(&info, 0x0, sizeof(struct rep_set_tags_record));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.chnid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        info.sid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.pName, sizeof(info.pName), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "time=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.pTime, sizeof(info.pTime), "%s", sValue);
    }
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct rep_set_tags_record), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_playback_add_tags(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_common_state *resp = (struct resp_common_state *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", resp->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "state=%d&", resp->state);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_remove_tags(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[32];
    int i;
    struct rep_set_tags_record_batch info;

    memset(&info, 0x0, sizeof(struct rep_set_tags_record_batch));
    if (!sdkp2p_get_section_info(buf, "cnt=", sValue, sizeof(sValue))) {
        info.cnt = atoi(sValue);
    }

    for (i = 0; i < info.cnt && i < MAX_LEN_32; i++) {
        snprintf(tmp, sizeof(tmp), "c%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            info.chnid[i] = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "t%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            get_url_decode(sValue);
            snprintf(info.pTime[i], sizeof(info.pTime[i]), "%s", sValue);
        }
    }
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct rep_set_tags_record_batch), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_playback_remove_tags(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_edit_tags(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct rep_set_tags_record info;

    memset(&info, 0x0, sizeof(struct rep_set_tags_record));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.chnid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.pName, sizeof(info.pName), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "time=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(info.pTime, sizeof(info.pTime), "%s", sValue);
    }
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct rep_set_tags_record), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_playback_edit_tags(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_common_state *resp = (struct resp_common_state *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", resp->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "state=%d&", resp->state);
    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_clear_tags(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_playback_clear_tags(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp);
    *datalen = strlen(buf) + 1;
    return;
}

int get_face_backup_search_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(struct resp_search_face_backup) || !resp || respLen <= 0) {
        return -1;
    }
    int len = 0;
    int offset = 0;
    char *img = NULL;
    cJSON *json = NULL;
    char *sResp = NULL;
    struct resp_search_face_backup *faceBackup = (struct resp_search_face_backup *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);

    cJSON_AddNumberToObject(json, "chnId", faceBackup->chnId);
    cJSON_AddNumberToObject(json, "sid", faceBackup->sid);
    cJSON_AddNumberToObject(json, "port", faceBackup->port);
    cJSON_AddNumberToObject(json, "index", faceBackup->index);
    cJSON_AddStringToObject(json, "time", faceBackup->pTime);
    cJSON_AddNumberToObject(json, "id", faceBackup->sImgAtrb.id);
    cJSON_AddNumberToObject(json, "blur", faceBackup->sImgAtrb.blur);
    cJSON_AddNumberToObject(json, "age", faceBackup->sImgAtrb.age);
    cJSON_AddNumberToObject(json, "gender", faceBackup->sImgAtrb.gender);
    cJSON_AddNumberToObject(json, "glasses", faceBackup->sImgAtrb.glasses);
    cJSON_AddNumberToObject(json, "mask", faceBackup->sImgAtrb.mask);
    cJSON_AddNumberToObject(json, "cap", faceBackup->sImgAtrb.cap);
    cJSON_AddNumberToObject(json, "expression", faceBackup->sImgAtrb.expression);
    cJSON_AddNumberToObject(json, "allCnt", faceBackup->allCnt);
    cJSON_AddNumberToObject(json, "allSize", faceBackup->allSize);
    cJSON_AddNumberToObject(json, "infoSize", faceBackup->infoSize);
    cJSON_AddNumberToObject(json, "bImgSize", faceBackup->bImgSize);
    cJSON_AddNumberToObject(json, "sImgSize", faceBackup->sImgSize);

    if (faceBackup->sImgSize > 0) {
        offset += sizeof(struct resp_search_face_backup);
        offset += faceBackup->bImgSize;
        len = (faceBackup->sImgSize + 2) / 3 * 4;
        img = ms_malloc(len + 1);
        if (img) {
            if (0 < base64EncodeOrg((MS_U8 *)img, len + 1, (MS_U8 *)(data + offset), faceBackup->sImgSize)) {
                cJSON_AddStringToObject(json, "sImg", img);
            }
            ms_free(img);
        }
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

static void req_playback_search_face_backup_remote(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char sValue[256] = {0};
    struct req_search_face_backup search;

    memset(&search, 0x0, sizeof(search));
    if (sdkp2p_get_section_info(buf, "chnMask=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    snprintf(search.chnMask, sizeof(search.chnMask), "%s", sValue);
    for (i = 0; i < MAX_CAMERA && i < strlen(search.chnMask); i++) {
        if (search.chnMask[i] == '1') {
            search.chnNum++;
        }
    }

    search.filter.id = -1;
    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        search.filter.id = atoi(sValue);
        if (search.filter.id < 0 || search.filter.id > 99999) {
            search.filter.id = -1;
        }
    }

    search.filter.blur = -1;
    if (!sdkp2p_get_section_info(buf, "blur=", sValue, sizeof(sValue))) {
        search.filter.blur = atoi(sValue);
        if (search.filter.blur < 0 || search.filter.blur > 100) {
            search.filter.blur = -1;
        }
    }

    search.filter.age = FACE_AGE_ALL;
    if (!sdkp2p_get_section_info(buf, "age=", sValue, sizeof(sValue))) {
        search.filter.age = (FACE_AGE)atoi(sValue);
        if (search.filter.age < FACE_AGE_NONE || search.filter.age > FACE_AGE_ALL) {
            search.filter.age = FACE_AGE_ALL;
        }
    }

    search.filter.gender = FACE_GENDE_ALL;
    if (!sdkp2p_get_section_info(buf, "gender=", sValue, sizeof(sValue))) {
        search.filter.gender = atoi(sValue);
        if (search.filter.gender < FACE_GENDE_NONE || search.filter.gender > FACE_GENDE_ALL) {
            search.filter.gender = FACE_GENDE_ALL;
        }
    }

    search.filter.glasses = FACE_GLASSES_ALL;
    if (!sdkp2p_get_section_info(buf, "glasses=", sValue, sizeof(sValue))) {
        search.filter.glasses = atoi(sValue);
        if (search.filter.glasses < FACE_GLASSES_NONE || search.filter.glasses > FACE_GLASSES_ALL) {
            search.filter.glasses = FACE_GLASSES_ALL;
        }
    }

    search.filter.mask = FACE_MASK_ALL;
    if (!sdkp2p_get_section_info(buf, "mask=", sValue, sizeof(sValue))) {
        search.filter.mask = atoi(sValue);
        if (search.filter.mask < FACE_MASK_NONE || search.filter.mask > FACE_MASK_ALL) {
            search.filter.mask = FACE_MASK_ALL;
        }
    }

    search.filter.cap = FACE_CAP_ALL;
    if (!sdkp2p_get_section_info(buf, "cap=", sValue, sizeof(sValue))) {
        search.filter.cap = atoi(sValue);
        if (search.filter.cap < FACE_CAP_NONE || search.filter.cap > FACE_CAP_ALL) {
            search.filter.cap = FACE_CAP_ALL;
        }
    }

    search.filter.expression = FACE_EXPRESSION_ALL;
    if (!sdkp2p_get_section_info(buf, "expression=", sValue, sizeof(sValue))) {
        search.filter.expression = atoi(sValue);
        if (search.filter.expression < FACE_EXPRESSION_NONE || search.filter.expression > FACE_EXPRESSION_ALL) {
            search.filter.expression = FACE_EXPRESSION_ALL;
        }
    }

    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    get_url_decode(sValue);
    snprintf(search.pStartTime, sizeof(search.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    get_url_decode(sValue);
    snprintf(search.pEndTime, sizeof(search.pEndTime), "%s", sValue);

    if (!sdkp2p_get_section_info(buf, "major=", sValue, sizeof(sValue))) {
        search.enMajor = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "minor=", sValue, sizeof(sValue))) {
        search.enMinor = atoi(sValue);
    }
    snprintf(search.ipaddr, sizeof(search.ipaddr), "%s", conf->ip_addr);
    sdkp2p_send_msg(conf, conf->req, (void *)&search, sizeof(search), SDKP2P_NEED_CALLBACK);
}

static void resp_playback_search_face_backup_remote(void *param, int size, char *buf, int len, int *datalen)
{
    get_face_backup_search_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

int get_search_face_backup_close_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(int) || !resp || respLen <= 0) {
        return -1;
    }
    char *sResp = NULL;
    cJSON *json = NULL;
    int res = -1;
    res = *(int *)data;

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

static void req_playback_search_face_backup_close_remote(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int sid = 0;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return;
    }
    sid = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, (void *)&sid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_playback_search_face_backup_close_remote(void *param, int size, char *buf, int len, int *datalen)
{
    get_search_face_backup_close_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_playback_search_face_backup_page_remote(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
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

static void resp_playback_search_face_backup_page_remote(void *param, int size, char *buf, int len, int *datalen)
{
    get_face_backup_search_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

int get_face_backup_search_bigimg_json(void *data, int dataLen, char *resp, int respLen)
{
    if (!data || dataLen < sizeof(struct resp_search_face_backup) || !resp || respLen <= 0) {
        return -1;
    }
    int len = 0;
    int offset = 0;
    char *img = NULL;
    cJSON *json = NULL;
    char *sResp = NULL;
    struct resp_search_face_backup *faceBackup = (struct resp_search_face_backup *)data;

    json = cJSON_CreateObject();
    cJSON_AddNumberToObject(json, "status", SDK_SUC);

    cJSON_AddNumberToObject(json, "chnId", faceBackup->chnId);
    cJSON_AddNumberToObject(json, "sid", faceBackup->sid);
    cJSON_AddNumberToObject(json, "port", faceBackup->port);
    cJSON_AddNumberToObject(json, "index", faceBackup->index);
    cJSON_AddStringToObject(json, "time", faceBackup->pTime);
    cJSON_AddNumberToObject(json, "id", faceBackup->sImgAtrb.id);
    cJSON_AddNumberToObject(json, "blur", faceBackup->sImgAtrb.blur);
    cJSON_AddNumberToObject(json, "age", faceBackup->sImgAtrb.age);
    cJSON_AddNumberToObject(json, "gender", faceBackup->sImgAtrb.gender);
    cJSON_AddNumberToObject(json, "glasses", faceBackup->sImgAtrb.glasses);
    cJSON_AddNumberToObject(json, "mask", faceBackup->sImgAtrb.mask);
    cJSON_AddNumberToObject(json, "cap", faceBackup->sImgAtrb.cap);
    cJSON_AddNumberToObject(json, "expression", faceBackup->sImgAtrb.expression);
    cJSON_AddNumberToObject(json, "allCnt", faceBackup->allCnt);
    cJSON_AddNumberToObject(json, "allSize", faceBackup->allSize);
    cJSON_AddNumberToObject(json, "infoSize", faceBackup->infoSize);
    cJSON_AddNumberToObject(json, "bImgSize", faceBackup->bImgSize);
    cJSON_AddNumberToObject(json, "sImgSize", faceBackup->sImgSize);

    if (faceBackup->bImgSize > 0) {
        offset += sizeof(struct resp_search_face_backup);
        len = (faceBackup->bImgSize + 2) / 3 * 4;
        img = ms_malloc(len + 1);
        if (img) {
            if (0 < base64EncodeOrg((MS_U8 *)img, len + 1, (MS_U8 *)(data + offset), faceBackup->bImgSize)) {
                cJSON_AddStringToObject(json, "bImg", img);
            }
            ms_free(img);
        }
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

static void req_playback_search_face_backup_bigimg_remote(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    REQ_GET_SEARCH_BACKUP info;

    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    info.sid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "index=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, PARAM_ERROR, NULL);
        return ;
    }
    info.npage = atoi(sValue);

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(info), SDKP2P_NEED_CALLBACK);
}

static void resp_playback_search_face_backup_bigimg_remote(void *param, int size, char *buf, int len, int *datalen)
{
    get_face_backup_search_bigimg_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
}

static void req_playback_search_pushmsg_pictures(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    REQ_PUSHMSG_PICTURES_S info;
    SDK_ERROR_CODE ret = -1;
    memset(&info, 0, sizeof(REQ_PUSHMSG_PICTURES_S));
    
    ret = sdk_str_to_req_pushmsg_pictures(&info, buf);
    if (ret) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, (ret), NULL);
        return;
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SEARCH_PUSHMSG_PICTURES, (void *)&info, sizeof(info), SDKP2P_NEED_CALLBACK);
}

static void resp_playback_search_pushmsg_pictures(void *param, int size, char *buf, int len, int *datalen)
{
    if (!param || size <= 0) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    memcpy(buf, param, size);
    *datalen = strlen(buf) + 1;
}

static void req_playback_search_picture_backup_remote_open(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char sValue[256] = {0};
    int objectType = 0;
    struct req_search_picture_backup search;
    
    memset(&search, 0x0, sizeof(struct req_search_picture_backup));

    if (!sdkp2p_get_section_info(buf, "mask=", sValue, sizeof(sValue))) {
        snprintf(search.chnMaskl, sizeof(search.chnMaskl), "%s", sValue);
        for (i = 0; i < strlen(search.chnMaskl) && i < MAX_REAL_CAMERA; i++) {
            if (search.chnMaskl[i] == '1') {
                search.chnNum++;
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "majorType=", sValue, sizeof(sValue))) {
        search.enMajor = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "minorType=", sValue, sizeof(sValue))) {
        search.enMinor = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(search.pStartTime, sizeof(search.pStartTime), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(search.pEndTime, sizeof(search.pEndTime), "%s", sValue);
    }

    
    if (!sdkp2p_get_section_info(buf, "all=", sValue, sizeof(sValue))) {
        search.all = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "num=", sValue, sizeof(sValue))) {
        search.maxNum = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "objectType=", sValue, sizeof(sValue))) {
        objectType = atoi(sValue);
    } else {
        objectType = DETEC_ALL;
    }
    if (objectType > 2 || objectType < 0) {
        search.objectType = DETEC_ALL;
    } else {
        search.objectType = objectType;
    }

    if (!sdkp2p_get_section_info(buf, "vcaType=", sValue, sizeof(sValue))) {
        search.vcaType = atoi(sValue);
    } else {
        search.vcaType = MAX_SMART_EVENT;
    }

    snprintf(search.ipaddr, sizeof(search.ipaddr), "%s", conf->ip_addr);
    sdkp2p_send_msg(conf, conf->req, (void *)&search, sizeof(struct req_search_picture_backup), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_search_picture_backup_remote_open(void *param, int size, char *buf, int len, int *datalen)
{
    int cnt  = 0;
    struct resp_search_picture_backup *resp = (struct resp_search_picture_backup *)param;
    cnt = size / sizeof(struct resp_search_picture_backup);

    get_playback_picture_record_info_json(resp, cnt, buf, len);

    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_search_picture_backup_remote_index(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct rep_play_picture_backup search;
    
    memset(&search, 0x0, sizeof(struct rep_play_picture_backup));
    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    search.sid = atoi(sValue);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        search.chnid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "pts=", sValue, sizeof(sValue))) {
        search.pts = atoll(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&search, sizeof(struct rep_play_picture_backup), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_search_picture_backup_remote_index(void *param, int size, char *buf, int len, int *datalen)
{
    int ret = -1;
    char reqUrl[128] = {0};
    char *resp = NULL;
    int respLen;
    
    if (!param || size < 0) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        return;
    }

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SEARCH_PIC_INDEX_REMOTE);

    resp = get_base64_encode(param, size);
    if (!resp) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        return;
    }
    if (size % 3 == 0) {
        respLen = size / 3 * 4;
    } else {
        respLen = (size / 3 + 1) * 4;
    }
    
    ret = ware_playback_picture_search_index_out(reqUrl, resp, respLen, buf, len);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        ms_free(resp);
        return;
    }

    *datalen = strlen(buf) + 1;
    ms_free(resp);
    return;
}

static void req_playback_search_picture_backup_remote_page(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    REQ_GET_SEARCH_BACKUP search;
    
    memset(&search, 0x0, sizeof(REQ_GET_SEARCH_BACKUP));
    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    search.sid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "page=", sValue, sizeof(sValue))) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    search.npage = atoi(sValue);
    
    sdkp2p_send_msg(conf, conf->req, (void *)&search, sizeof(REQ_GET_SEARCH_BACKUP), SDKP2P_NEED_CALLBACK);
    return;
}

static void req_playback_search_picture_backup_remote_close(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int sid;
    
    if (sdkp2p_get_section_info(buf, "sid=", sValue, sizeof(sValue))) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    sid = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, (void *)&sid, sizeof(int), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_search_picture_backup_remote_close(void *param, int size, char *buf, int len, int *datalen)
{
    int ret = -1;
    char reqUrl[128] = {0};
    
    if (!param || size < 0) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        return;
    }

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_SEARCH_PIC_CLOSE_REMOTE);
    
    ret = ware_playback_picture_search_index_out(reqUrl, param, size, buf, len);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, &ret);
        return;
    }
    *datalen = strlen(buf) + 1;
    return;
}

static void req_playback_get_general_base_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnid = -1;
    struct req_search_common_backup search;
    
    memset(&search, 0x0, sizeof(struct req_search_common_backup));
    if (!sdkp2p_get_section_info(buf, "chnId=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    if (chnid < 0 || chnid >= MAX_REAL_CAMERA) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    search.chnMaskl[chnid] = '1';
    search.chnNum = 1;
    if (sdkp2p_get_section_info(buf, "stream=", sValue, sizeof(sValue))) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    search.enType = atoi(sValue);// @ FILE_TYPE_EN
    search.enEvent = REC_EVENT_ALL;
    if (!sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        search.enEvent = atoi(sValue);
    }

    search.enState = SEG_STATE_ALL;
    if (!sdkp2p_get_section_info(buf, "state=", sValue, sizeof(sValue))) {
        search.enState = atoi(sValue);
    }

    if (sdkp2p_get_section_info(buf, "startTime=", sValue, sizeof(sValue))) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    get_url_decode(sValue);
    snprintf(search.pStartTime, sizeof(search.pStartTime), "%s", sValue);

    if (sdkp2p_get_section_info(buf, "endTime=", sValue, sizeof(sValue))) {
        sdkp2p_json_resp(buf, len, PARAM_ERROR, NULL);
        return;
    }
    get_url_decode(sValue);
    snprintf(search.pEndTime, sizeof(search.pEndTime), "%s", sValue);
    search.all = 1;

    if (!sdkp2p_get_section_info(buf, "all=", sValue, sizeof(sValue))) {
        search.all = atoi(sValue);
    }

    snprintf(search.ipaddr, sizeof(search.ipaddr), "%s", conf->ip_addr);
    sdkp2p_send_msg(conf, conf->req, (void *)&search, sizeof(struct req_search_common_backup), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_playback_get_general_base_info(void *param, int size, char *buf, int len, int *datalen)
{
    get_playback_common_base_info_json(param, size, buf, len);
    *datalen = strlen(buf) + 1;
    return;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_MONTH_EVENT, get_pbdayinfo_msfs_to_struct, RESPONSE_FLAG_GET_MONTH_EVENT, get_pbdayinfo_msfs_to_str},//get.playback.pbdayinfo
    {REQUEST_FLAG_SEARCH_COM_REMOTEPB_OPEN, get_msfs_pbhourinfo_to_struct, RESPONSE_FLAG_SEARCH_COM_REMOTEPB_OPEN, get_msfs_pbhourinfo_to_str},//get.playback.recordinfo
    {REQUEST_FLAG_SEARCH_COM_REMOTEPB_PAGE, get_msfs_pbhourinfo_page_to_struct, RESPONSE_FLAG_SEARCH_COM_REMOTEPB_PAGE, get_msfs_pbhourinfo_page_to_str},//get.playback.recordinfo_page
    {REQUEST_FLAG_PLAY_COM_REMOTEPB, set_msfs_pbplay_to_struct, RESPONSE_FLAG_PLAY_COM_REMOTEPB, get_msfs_pbplay_to_str},//set.playback.start
    {REQUEST_FLAG_PAUSE_COM_REMOTEPB, set_msfs_pbpause_to_struct, RESPONSE_FLAG_PAUSE_COM_REMOTEPB, NULL},//set.playback.pause_batch
    {REQUEST_FLAG_RESTART_COM_REMOTEPB, set_msfs_pbrestart_to_struct, RESPONSE_FLAG_RESTART_COM_REMOTEPB, NULL},//set.playback.resume
    {REQUEST_FLAG_SEEK_REMOTEPB, set_msfs_pbseek_to_struct, RESPONSE_FLAG_SEEK_REMOTEPB, get_msfs_pbseek_to_str},//set.playback.jump_batch
    {REQUEST_FLAG_STEP_REMOTEPB, set_msfs_pbstep_to_struct, RESPONSE_FLAG_STEP_REMOTEPB, NULL},//set.playback.step_batch
    {REQUEST_FLAG_STOP_COM_REMOTEPB, set_msfs_pbstop_to_struct, RESPONSE_FLAG_STOP_COM_REMOTEPB, NULL},//set.playback.stop_batch
    {REQUEST_FLAG_SEARCH_COM_REMOTEPB_CLOSE, set_msfs_pbclose_to_struct, RESPONSE_FLAG_SEARCH_COM_REMOTEPB_CLOSE, NULL},//set.playback.close_batch
    {REQUEST_FLAG_GET_REMOTEPB_TIME, get_msfs_playtime_to_struct, RESPONSE_FLAG_GET_REMOTEPB_TIME, get_pbplaytime_to_str},//get.playback.playtime
    {REQUEST_FLAG_SPEED_REMOTEPB, set_msfs_pbspeed_to_struct, RESPONSE_FLAG_SPEED_REMOTEPB, NULL},//set.playback.speed_batch
    {REQUEST_FLAG_P2P_PB_STOP, p2p_pb_stop_to_struct, -1, NULL},
    {REQUEST_FLAG_SET_TRANCODING, p2p_pb_set_trancode_to_struct, RESPONSE_FLAG_SET_TRANCODING, playback_set_trancode_to_str},//set.playback.transcoding
    {REQUEST_FLAG_GET_TRANCODING, p2p_pb_get_trancode_to_struct, RESPONSE_FLAG_GET_TRANCODING, playback_get_trancode_to_str},//get.playback.transcoding
    {REQUEST_FLAG_REMOVE_REMOTEPB_STREAM, remove_remotepb_stream_to_struct, -1, NULL},
    {REQUEST_FLAG_ADD_REMOTELIVE, add_remotelive_to_struct, RESPONSE_FLAG_ADD_REMOTELIVE, add_remotelive_to_str},
    {REQUEST_FLAG_GET_REMOTEPB_RTSPADDR, get_remotepb_rtspaddr_to_struct, RESPONSE_FLAG_GET_REMOTEPB_RTSPADDR, get_remotepb_rtspaddr_to_str},//get.playback.rtspurl
    {REQUEST_FLAG_PLAYBACK_STOP, set_playback_stop, -1, NULL},
    {REQUEST_FLAG_PLAYBACK_REMOTE_CHNS, playback_remote_chns_to_struct, RESPONSE_FLAG_PLAYBACK_REMOTE_CHNS, playback_remote_chns_to_str},
    {REQUEST_FLAG_SEARCH_EVT_BACKUP_CLOSE, req_p2p_set_evt_backup_close, RESPONSE_FLAG_SEARCH_EVT_BACKUP_CLOSE, get_evt_backup_close_to_str},//get.playback.evt_backup_search_close
    {REQUEST_FLAG_SEARCH_EVT_BACKUP, req_p2p_get_evt_backup_search, RESPONSE_FLAG_SEARCH_EVT_BACKUP, get_evt_backup_search_to_str},//get.playback.evt_backup_search
    {REQUEST_FLAG_GET_SEARCH_EVT_PAGE, req_p2p_get_evt_backup_search_page, RESPONSE_FLAG_GET_SEARCH_EVT_PAGE, get_evt_backup_search_to_str},//get.playback.evt_backup_search_page

    {REQUEST_FLAG_SEARCH_ANPR_BACKUP_REMOTE, req_p2p_get_anpr_backup_search, RESPONSE_FLAG_SEARCH_ANPR_BACKUP_REMOTE, resp_p2p_get_anpr_backup_search},//get.playback.anpr_backup_search
    {REQUEST_FLAG_SEARCH_ANPR_BACKUP_CLOSE_REMOTE, req_p2p_get_anpr_backup_close, RESPONSE_FLAG_SEARCH_ANPR_BACKUP_CLOSE_REMOTE, resp_p2p_get_anpr_backup_close},//get.playback.anpr_backup_search_close
    {REQUEST_FLAG_SEARCH_ANPR_BACKUP_PAGE_REMOTE, req_p2p_get_anpr_backup_search_page, RESPONSE_FLAG_SEARCH_ANPR_BACKUP_PAGE_REMOTE, resp_p2p_get_anpr_backup_search_page},//get.playback.anpr_backup_search_page
    {REQUEST_FLAG_SEARCH_ANPR_BACKUP_BIGIMG_REMOTE, req_p2p_get_anpr_backup_bigimg, RESPONSE_FLAG_SEARCH_ANPR_BACKUP_BIGIMG_REMOTE, resp_p2p_get_anpr_backup_bigimg},//get.playback.anpr_backup_search_bigimg

    {REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_OPEN_REMOTE, req_playback_tags_search_open, RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_OPEN_REMOTE, resp_playback_tags_search_open},//get.playback.tags_search_open
    {REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_PAGE_REMOTE, req_playback_tags_search_page, RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_PAGE_REMOTE, resp_playback_tags_search_page},//get.playback.tags_search_page
    {REQUEST_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE_REMOTE, req_playback_tags_search_close, RESPONSE_FLAG_SEARCH_TAGS_PLAYBACK_CLOSE_REMOTE, resp_playback_tags_search_close},//get.playback.tags_search_close
    {REQUEST_FLAG_LOCK_COM_BACKUP_REMOTE, req_playback_set_commom_backup_lock, RESPONSE_FLAG_LOCK_COM_BACKUP_REMOTE, resp_playback_set_commom_backup_lock},//set.playback.common_backup_lock
    {REQUEST_FLAG_GET_REC_RANGE, req_playback_get_rec_range, RESPONSE_FLAG_GET_REC_RANGE, resp_playback_get_rec_range},//get.playback.rec_range
    {REQUEST_FLAG_REMOTE_BACKUP_START, req_playback_remote_backup_start, RESPONSE_FLAG_REMOTE_BACKUP_START, resp_playback_remote_backup_start},//get.playback.download
    {REQUEST_FLAG_REMOTE_BACKUP_START_DATA, NULL, RESPONSE_FLAG_REMOTE_BACKUP_START_DATA, resp_playback_remote_backup_start_data},
    {REQUEST_FLAG_REMOTE_BACKUP_STOP, req_playback_remote_backup_stop, RESPONSE_FLAG_REMOTE_BACKUP_STOP, resp_playback_remote_backup_stop},//get.playback.stop_download
    {REQUEST_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE, req_p2p_split_playback_range, RESPONSE_FLAG_SEARCH_SPLIT_PLAYBACK_RANGE, resp_p2p_split_playback_range},//get.playback.split_playback_range
    {REQUEST_FLAG_PLAY_COM_PICTURE, req_playback_common_picture, RESPONSE_FLAG_PLAY_COM_PICTURE, resp_playback_common_picture},//get.playback.common_play_picture

    {REQUEST_FLAG_GET_TEMP_TAGS_PLAYBACK, req_playback_get_tags, RESPONSE_FLAG_GET_TEMP_TAGS_PLAYBACK, resp_playback_get_tags},//get.playback.tags_get
    {REQUEST_FLAG_SET_TEMP_TAGS_PLAYBACK, req_playback_add_tags, RESPONSE_FLAG_SET_TEMP_TAGS_PLAYBACK, resp_playback_add_tags},//get.playback.tags_add
    {REQUEST_FLAG_REMOVE_TEMP_TAGS_PLAYBACK_BATCH, req_playback_remove_tags, RESPONSE_FLAG_REMOVE_TEMP_TAGS_PLAYBACK_BATCH, resp_playback_remove_tags},//get.playback.tags_remove
    {REQUEST_FLAG_EDIT_TEMP_TAGS_PLAYBACK, req_playback_edit_tags, RESPONSE_FLAG_EDIT_TEMP_TAGS_PLAYBACK, resp_playback_edit_tags},//get.playback.tags_edit
    {REQUEST_FLAG_CLEAR_TEMP_TAGS_PLAYBACK, req_playback_clear_tags, RESPONSE_FLAG_CLEAR_TEMP_TAGS_PLAYBACK, resp_playback_clear_tags},//get.playback.tags_clear

    {REQUEST_FLAG_SEARCH_FACE_BACKUP_REMOTE, req_playback_search_face_backup_remote, RESPONSE_FLAG_SEARCH_FACE_BACKUP_REMOTE, resp_playback_search_face_backup_remote},//get.playback.face_backup_search
    {REQUEST_FLAG_SEARCH_FACE_BACKUP_CLOSE_REMOTE, req_playback_search_face_backup_close_remote, RESPONSE_FLAG_SEARCH_FACE_BACKUP_CLOSE_REMOTE, resp_playback_search_face_backup_close_remote},//get.playback.face_backup_search_close
    {REQUEST_FLAG_SEARCH_FACE_BACKUP_PAGE_REMOTE, req_playback_search_face_backup_page_remote, RESPONSE_FLAG_SEARCH_FACE_BACKUP_PAGE_REMOTE, resp_playback_search_face_backup_page_remote},//get.playback.face_backup_search_page
    {REQUEST_FLAG_SEARCH_FACE_BACKUP_BIGIMG_REMOTE, req_playback_search_face_backup_bigimg_remote, RESPONSE_FLAG_SEARCH_FACE_BACKUP_BIGIMG_REMOTE, resp_playback_search_face_backup_bigimg_remote},//get.playback.face_backup_search_bigimg
    {REQUEST_FLAG_SEARCH_PUSHMSG_PICTURES, req_playback_search_pushmsg_pictures, RESPONSE_FLAG_SEARCH_PUSHMSG_PICTURES, resp_playback_search_pushmsg_pictures}, // get.playback.pushmsg.pictures
    {REQUEST_FLAG_SEARCH_PIC_BACKUP_REMOTE, req_playback_search_picture_backup_remote_open, RESPONSE_FLAG_SEARCH_PIC_BACKUP_REMOTE, resp_playback_search_picture_backup_remote_open},//get.playback.picture_backup_search_open
    {REQUEST_FLAG_SEARCH_PIC_PAGE_REMOTE, req_playback_search_picture_backup_remote_page, RESPONSE_FLAG_SEARCH_PIC_PAGE_REMOTE, resp_playback_search_picture_backup_remote_open},//get.playback.picture_backup_search_page
    {REQUEST_FLAG_SEARCH_PIC_INDEX_REMOTE, req_playback_search_picture_backup_remote_index, RESPONSE_FLAG_SEARCH_PIC_INDEX_REMOTE, resp_playback_search_picture_backup_remote_index},//get.playback.picture_backup_search_index
    {REQUEST_FLAG_SEARCH_PIC_CLOSE_REMOTE, req_playback_search_picture_backup_remote_close, RESPONSE_FLAG_SEARCH_PIC_CLOSE_REMOTE, resp_playback_search_picture_backup_remote_close},//get.playback.picture_backup_search_close
    {REQUEST_FLAG_GET_PB_COMM_BASE, req_playback_get_general_base_info, RESPONSE_FLAG_GET_PB_COMM_BASE, resp_playback_get_general_base_info},//get.playback.general.base.info

    // request register array end
};

void p2p_playback_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_playback_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

