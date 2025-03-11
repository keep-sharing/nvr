#include <time.h>
#include "sdk_util.h"


int get_day_time_json(char *resp, int respLen)
{
    if (!resp || respLen <= 0) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    time_t tmNow = 0;
    time(&tmNow);
    struct tm *tm_Now;
    char sDateTime[128] = {0};
    char dateTmp[8] = {0};

    json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }

    tm_Now = localtime(&tmNow);
    strftime(sDateTime, sizeof(sDateTime), "%Y-%m-%d %H:%M:%S", tm_Now);
    
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    snprintf(dateTmp, sizeof(dateTmp), "%.4d", tm_Now->tm_year + 1900);
    cJSON_AddStringToObject(json, "dateYear", dateTmp);
    snprintf(dateTmp, sizeof(dateTmp), "%.2d", tm_Now->tm_mon + 1);
    cJSON_AddStringToObject(json, "dateMon", dateTmp);
    snprintf(dateTmp, sizeof(dateTmp), "%.2d", tm_Now->tm_mday);
    cJSON_AddStringToObject(json, "dateDay", dateTmp);

    
    snprintf(dateTmp, sizeof(dateTmp), "%.2d", tm_Now->tm_hour);
    cJSON_AddStringToObject(json, "dateHour", dateTmp);
    snprintf(dateTmp, sizeof(dateTmp), "%.2d", tm_Now->tm_min);
    cJSON_AddStringToObject(json, "dateMin", dateTmp);
    snprintf(dateTmp, sizeof(dateTmp), "%.2d", tm_Now->tm_sec);
    cJSON_AddStringToObject(json, "dateSec", dateTmp);
    cJSON_AddStringToObject(json, "serverTime", sDateTime);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}

SDK_ERROR_CODE set_hotspare_mode_by_text(const char *buff, struct ms_failover_change_mode *info)
{
    if (!buff || !info) {
        return OTHERS_ERROR;
    }

    char sValue[256] = {0};
    struct failover_list failover;

    if (sdkp2p_get_section_info(buff, "mode=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    info->mode = atoi(sValue);

    if (info->mode == FAILOVER_MODE_MASTER) {
        if (!sdkp2p_get_section_info(buff, "ip=", sValue, sizeof(sValue))) {
            snprintf(info->ipaddr, sizeof(info->ipaddr), "%s", sValue);
        }
        if (!sdkp2p_get_section_info(buff, "password=", sValue, sizeof(sValue))) {
            snprintf(info->password, sizeof(info->password), "%s", sValue);
        }

        memset(&failover, 0x0, sizeof(struct failover_list));
        read_failover(SQLITE_FILE_NAME, &failover, 0);
        if (info->ipaddr[0]) {
            snprintf(failover.ipaddr, sizeof(failover.ipaddr), info->ipaddr);
        }
        if (info->password[0]) {
            snprintf(failover.password, sizeof(failover.password), info->password);
        }
        write_failover(SQLITE_FILE_NAME, &failover);
    }

    set_param_int(SQLITE_FILE_NAME, PARAM_FAILOVER_MODE, info->mode);

    return REQUEST_SUCCESS;
}

int search_hotspare_masters_json(void *data, int cnt, char *resp, int respLen)
{
    if (!data || cnt <= 0 || !resp || respLen <= 0) {
        return -1;
    }

    struct resq_search_nvr *masters = (struct resq_search_nvr *)data;
    cJSON *root = cJSON_CreateObject();
    cJSON *masterArr = NULL;
    cJSON *obj = NULL;
    char *sResp = NULL;
    int i;

    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    masterArr = cJSON_AddArrayToObject(root, "masters");
    for (i = 0; i < cnt; ++i) {
        obj = cJSON_CreateObject();
        cJSON_AddItemToArray(masterArr, obj);

        cJSON_AddStringToObject(obj, "ip", masters[i].ipaddr);
        cJSON_AddStringToObject(obj, "mac", masters[i].mac);
        cJSON_AddStringToObject(obj, "model", masters[i].model);
        cJSON_AddNumberToObject(obj, "maxCamera", masters[i].max_camera);
    }

    sResp = cJSON_PrintUnformatted(root);
    if (!sResp) {
        cJSON_Delete(root);
        return -1;
    }
    
    snprintf(resp, respLen, "%s", sResp);
    
    cJSON_Delete(root);
    cJSON_free(sResp);

    return 0;
}

#if 0 // wcm delete, don't need logs
int init_hotspare_log(int action, const char *masterIp, const char *remoteIp, const char *user, struct log_data *pLogData)
{
    if (!masterIp || !remoteIp || !user) {
        return -1;
    }

    // struct log_data logData;
    struct detail_failover_operation info;
    // memset(&logData, 0, sizeof(struct log_data));
    memset(&info, 0x0, sizeof(struct detail_failover_operation));

    snprintf(pLogData->log_data_info.ip, sizeof(pLogData->log_data_info.ip), remoteIp);
    snprintf(pLogData->log_data_info.user, sizeof(pLogData->log_data_info.user), user);
    pLogData->log_data_info.mainType = MAIN_OP;
    pLogData->log_data_info.parameter_type = SUB_PARAM_NONE; //SUB_PARAM_DEL_MASTER;
    if (action == 0) {
        pLogData->log_data_info.subType = SUP_OP_FAILOVER_SLAVE_ADD_MASTER;
        info.action = OP_FAILOVER_ADD_MASTER;
    } else if (action == 1) {
        pLogData->log_data_info.subType = SUP_OP_FAILOVER_SLAVE_DEL_MASTER;
        info.action = OP_FAILOVER_DEL_MASTER;
    } else {
        return -1;
    }
    snprintf(info.ipaddr, sizeof(info.ipaddr), "%s", masterIp);

    msfs_log_pack_detail(pLogData, OP_FAILOVER_OPERATION, &info, sizeof(struct detail_failover_operation));

    return 0;
}
#endif 

SDK_ERROR_CODE add_hotspare_masters_by_text(const char *buf, struct failover_list *masters, 
    struct ms_failover_update_master *pUpdate, Uint32 *pMasterMask)
{
    if (!buf || !masters) {
        return OTHERS_ERROR;
    }

    int i, j = 0;
    int sum, maxCount;
    char sValue[256] = {0};
    int maxCamera;
    char sField[32] = {0};
    int id = -1;
    struct failover_list master;
    memset(&master, 0, sizeof(struct failover_list));

    if (sdkp2p_get_section_info(buf, "sum=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    sum = atoi(sValue);
    if (sum <= 0 || sum > MAX_FAILOVER) {
        return PARAM_ERROR;
    }

    read_failovers(SQLITE_FILE_NAME, masters, &maxCount);
    pUpdate->type = -1;
    for (i = 0; i < sum; ++i) {
        id = -1;

        snprintf(sField, sizeof(sField), "maxCamera[%d]=", i);
        if (sdkp2p_get_section_info(buf, sField, sValue, sizeof(sValue))) {
            continue;
        }
        maxCamera = atoi(sValue);
        if (maxCamera <= 0 || maxCamera > MAX_CAMERA) {
            continue;
        }
        
        snprintf(sField, sizeof(sField), "ip[%d]=", i);
        if (sdkp2p_get_section_info(buf, sField, sValue, sizeof(sValue))) {
            continue;
        }
        snprintf(master.ipaddr, sizeof(master.ipaddr), sValue);
        
        for (j = 0; j < MAX_FAILOVER; ++j) {
            if (masters[j].enable
                && !strcmp(masters[j].ipaddr, master.ipaddr)) {
                break;
            }
            if (id == -1
                && !masters[j].enable) {
                id = j;
            }
        }
        if (j != MAX_FAILOVER) {
            continue;
        }

        masters[id].enable = 1;
        snprintf(masters[id].ipaddr, sizeof(masters[id].ipaddr), master.ipaddr);
        
        snprintf(sField, sizeof(sField), "mac[%d]=", i);
        if (!sdkp2p_get_section_info(buf, sField, sValue, sizeof(sValue))) {
            snprintf(masters[id].mac, sizeof(masters[id].mac), sValue);
        }

        snprintf(sField, sizeof(sField), "model[%d]=", i);
        if (!sdkp2p_get_section_info(buf, sField, sValue, sizeof(sValue))) {
            snprintf(masters[id].model, sizeof(masters[id].model), sValue);
        }
        masters[id].maxCamera = maxCamera;
        write_failover(SQLITE_FILE_NAME, &masters[id]);
        *pMasterMask |= ((Uint32)1 << id);
        pUpdate->type = 0;
    }
    
    return REQUEST_SUCCESS;
}

SDK_ERROR_CODE get_hotspare_masters_status_json(void *data, int cnt, char *resp, int respSize)
{
    if (!data || cnt <= 0 || !resp || respSize <= 0) {
        return OTHERS_ERROR;
    }

    struct ms_failover_master_status *masters = (struct ms_failover_master_status *)data;
    cJSON *root = NULL;
    cJSON *masterArr = NULL;
    cJSON *obj = NULL;
    char *rootStr = NULL;
    int i;

    root = cJSON_CreateObject();
    if (!root) {
        return MEMORY_FAILED;
    }

    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    masterArr = cJSON_AddArrayToObject(root, "masterStatuses");
    for (i = 0; i < cnt; ++i) {
        obj = cJSON_CreateObject();
        cJSON_AddItemToArray(masterArr, obj);

        cJSON_AddStringToObject(obj, "ip", masters[i].ipaddr);
        cJSON_AddStringToObject(obj, "mac", masters[i].mac);
        cJSON_AddStringToObject(obj, "model", masters[i].model);
        cJSON_AddNumberToObject(obj, "connectStatus", masters[i].connect_status);
        cJSON_AddNumberToObject(obj, "workStatus", masters[i].work_status);
        cJSON_AddNumberToObject(obj, "returnPercent", masters[i].percent);
    }

    rootStr = cJSON_PrintUnformatted(root);
    if (!rootStr) {
        cJSON_Delete(root);
        return MEMORY_FAILED;
    }
    snprintf(resp, respSize, rootStr);

    cJSON_Delete(root);
    cJSON_free(rootStr);

    return REQUEST_SUCCESS;
}

SDK_ERROR_CODE del_hotspare_masters_by_text(const char *buf, char ips[][MAX_LEN_16], int *pSum, Uint32 *pMask)
{
    if (!buf) {
        return PARAM_ERROR;
    }

    char sValue[256] = {0};
    char sField[32] = {0};
    int i;

    if (sdkp2p_get_section_info(buf, "sum=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    *pSum = atoi(sValue);
    if (*pSum <= 0 || *pSum > MAX_FAILOVER) {
        return PARAM_ERROR;
    }

    for (i = 0; i < *pSum; ++i) {
        snprintf(sField, sizeof(sField), "ip[%d]=", i);
        if (!sdkp2p_get_section_info(buf, sField, sValue, sizeof(sValue))) {
            snprintf(ips[i], MAX_LEN_16, sValue);
        }
    }

    delete_failovers_by_ip(SQLITE_FILE_NAME, ips, *pSum, pMask);

    return REQUEST_SUCCESS;
}

SDK_ERROR_CODE get_hotspare_slave_status_json(void *data, int dataSize, char *resp, int respSize)
{
    if (!data || dataSize < sizeof(struct ms_failover_slave_status) || !resp || respSize <= 0) {
        return OTHERS_ERROR;
    }

    struct failover_list slave;
    struct ms_failover_slave_status *slaveStatus = (struct ms_failover_slave_status *)data;
    cJSON *root = NULL;
    char *rootStr = NULL;

    memset(&slave, 0, sizeof(struct failover_list));

    root = cJSON_CreateObject();
    if (!root) {
        return MEMORY_FAILED;
    }

    read_failover(SQLITE_FILE_NAME, &slave, 0);

    cJSON_AddNumberToObject(root, "status", SDK_SUC);
    cJSON_AddStringToObject(root, "ip", slave.ipaddr);
    cJSON_AddStringToObject(root, "password", slave.password);
    cJSON_AddNumberToObject(root, "slaveStatus", slaveStatus->status);
    cJSON_AddStringToObject(root, "returnPercent", slaveStatus->percent);

    rootStr = cJSON_PrintUnformatted(root);
    if (!rootStr) {
        cJSON_Delete(root);
        return OTHERS_ERROR;
    }
    snprintf(resp, respSize, rootStr);

    cJSON_Delete(root);
    cJSON_free(rootStr);
    
    return REQUEST_SUCCESS;    
}

static void sdk_check_version_to_struct(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }
    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void sdk_check_version_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_system_preview_operation(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_p2p_preview *res = (struct req_p2p_preview *)param;
    char sValue[256] = {0};

    memset(res, 0x0, sizeof(struct req_p2p_preview));
    if (!sdkp2p_get_section_info(buf, "r.ch_mask=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%lld", &(res->ch_mask));
    }

    if (!sdkp2p_get_section_info(buf, "r.strm_fmt=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%lld", &(res->strm_fmt));
    }

    if (!sdkp2p_get_section_info(buf, "r.audio_on=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%lld", &(res->audio_on));
    }

    if (!sdkp2p_get_section_info(buf, "r.mode=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%lld", &(res->mode));
    }

    if (!sdkp2p_get_section_info(buf, "r.pbsid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->pbsid));
    }

    if (!sdkp2p_get_section_info(buf, "r.split_playback=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->split_playback));
    }

    if (!sdkp2p_get_section_info(buf, "r.pushType=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->pushType));
    }

    *datalen = sizeof(struct req_p2p_preview);
}

static void req_syetem_set_device_id(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int devid = 0;

    if (!sdkp2p_get_section_info(buf, "device_id=", sValue, sizeof(sValue))) {
        if (!check_param_key(SQLITE_FILE_NAME, PARAM_DEVICE_ID)) {
            set_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_ID, sValue);
        } else {
            add_param_value(SQLITE_FILE_NAME, PARAM_DEVICE_ID, sValue);
        }
        devid = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &devid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void bandwidth_full_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int chan_id;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;

}

static void web_auto_login_check_to_str(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void resp_syetem_get_system_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct sysinfo *res = (struct sysinfo *)param;
    int max_poe_num = get_param_int(SQLITE_FILE_NAME, PARAM_POE_NUM, 0);
    struct device_info device;

    memset(&device, 0x0, sizeof(struct device_info));
    db_get_device(SQLITE_FILE_NAME, &device);
    if (device.oem_type != OEM_TYPE_STANDARD) {
        get_param_oem_value(SQLITE_FILE_NAME, PARAM_DEVICE_MODEL, device.model, sizeof(device.model), device.model);
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "r.hostname=%s&", res->hostname);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.deviceid=%d&", res->deviceid);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.company=%s&", res->company);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.productno=%s&", res->productno);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.hardwareversion=%s&", res->hardwareversion);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.softwareversion=%s&", res->softwareversion);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.oemversion=%s&", res->oemversion);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.devicetype=%s&", res->devicetype);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.devicename=%s&", res->devicename);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sn=%s&", res->sn);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.guistyle=%s&", res->guistyle);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.reserve=%s&", res->reserve);
    
    //resource
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_cameras=%d&", res->resource.max_cameras);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_analog_cameras=%d&", res->resource.max_analog_cameras);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_ipnc_cameras=%d&", res->resource.max_ipnc_cameras);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.camera_layout=%d&", res->resource.camera_layout);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.display_num=%d&", res->resource.display_num);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_sensor=%d&", res->resource.max_sensor);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_alarm=%d&", res->resource.max_alarm);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.hasBuzzer=%d&", res->resource.hasBuzzer);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.disk_num=%d&", res->resource.disk_num);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_dec_chan=%d&", res->resource.max_dec_chan);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_spot_out=%d&", res->resource.max_spot_out);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_audio_in=%d&", res->resource.max_audio_in);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_audio_out=%d&", res->resource.max_audio_out);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max_network_lan=%d&", res->resource.max_network_lan);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max485=%d&", res->resource.max485);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.max232=%d&", res->resource.max232);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.resource.has_ir=%d&", res->resource.has_ir);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.uptime=%s&", res->uptime);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.mac=%s&", res->mac);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sdkversion=%s&", PARAMS_P2P_SDK_VERSION);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.ms_platform=%s&", MS_PLATFORM);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.ipsdkversion=%s&", PARAMS_SDK_VERSION);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.max_poe_num=%d&", max_poe_num);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.prefix=%s&", device.prefix);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.model=%s&", device.model);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.max_alarm_in=%d&", device.max_alarm_in);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.max_alarm_out=%d&", device.max_alarm_out);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.max_lan=%d&", device.max_lan);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.msddns=%d&", device.msddns);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.sncode=%s&", device.sncode);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.oem_type=%d&", device.oem_type);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.is_support_talk=%d&", res->isSupportTalk);

    snprintf(buf + strlen(buf), len - strlen(buf), "get.event.push_msg=1&");

    *datalen = strlen(buf) + 1;
}

static void req_system_get_system_time(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct time stime = { 0 };
    char manual_time[32] = {0};
    time_t tmNow = 0;
    time(&tmNow);
    struct tm *tm_Now;
    tm_Now = localtime(&tmNow);

    strftime(manual_time, sizeof(manual_time), "%Y-%m-%d %H:%M:%S", tm_Now);
    read_time(SQLITE_FILE_NAME, &stime);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ntp_enable=%d&", stime.ntp_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "dst_enable=%d&", stime.dst_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "time_zone=%s&", stime.time_zone);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "time_zone_name=%s&", stime.time_zone_name);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ntp_server=%s&", stime.ntp_server);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "manual_time=%s&", manual_time);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "time=%d&", (int)time(NULL));
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sync_enable=%d&", stime.sync_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sync_interval=%d&", stime.sync_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ntp_pc=%d&", stime.sync_pc);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_syetem_set_system_time(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, n = 0;
    char sValue[256] = {0};
    char manual_timep[256] = {0};
    struct time time = {0};
    struct req_set_sysconf sys;

    memset(&sys, 0, sizeof(struct req_set_sysconf));
    read_time(SQLITE_FILE_NAME, &time);
    if (!sdkp2p_get_section_info(buf, "ntp_enable=", sValue, sizeof(sValue))) {
        time.ntp_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "dst_enable=", sValue, sizeof(sValue))) {
        time.dst_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "time_zone=", sValue, sizeof(sValue))) {
        snprintf(time.time_zone, sizeof(time.time_zone), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "time_zone_name=", sValue, sizeof(sValue))) {
        snprintf(time.time_zone_name, sizeof(time.time_zone_name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ntp_server=", sValue, sizeof(sValue))) {
        snprintf(time.ntp_server, sizeof(time.ntp_server), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "manual_time=", sValue, sizeof(sValue))) {
        if (strstr(sValue, "%20")) {
            while (sValue[i] != '%') {
                manual_timep[n++] = sValue[i++];
            }
            manual_timep[n++] = ' ';
            i = i + 3;
            while (sValue[i] != '\0') {
                manual_timep[n++] = sValue[i++];
            }
            manual_timep[n] = '\0';
        } else {
            snprintf(manual_timep, sizeof(manual_timep), "%s", sValue);
        }
    }
    if (time.ntp_enable == 0) {
        snprintf(sys.arg, sizeof(sys.arg), "\"%s\"", manual_timep);
    }
    if (!sdkp2p_get_section_info(buf, "sync_enable=", sValue, sizeof(sValue))) {
        time.sync_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sync_interval=", sValue, sizeof(sValue))) {
        time.sync_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sync_pc=", sValue, sizeof(sValue))) {
        time.sync_pc = atoi(sValue);
    }

    write_time(SQLITE_FILE_NAME, &time);

    sdkp2p_send_msg(conf, conf->req, &sys, sizeof(struct req_set_sysconf), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SYNC_IPC_TIME, &sys, sizeof(struct req_set_sysconf), SDKP2P_NOT_CALLBACK);

    struct log_data log_data;
    memset(&log_data, 0, sizeof(struct log_data));
    log_data.log_data_info.mainType =  MAIN_OP;
    log_data.log_data_info.subType = SUP_OP_SYSTEM_TIME_SYNC_REMOTE;
    log_data.log_data_info.parameter_type = SUB_PARAM_NONE;
    snprintf(log_data.log_data_info.ip, sizeof(log_data.log_data_info.ip), "%s", conf->ip_addr);
    sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE, &log_data, sizeof(struct log_data), SDKP2P_NOT_CALLBACK);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_syetem_get_system_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct resp_get_sysstatus resp = {0};

    memset(&resp, 0x0, sizeof(struct resp_get_sysstatus));
    //resp.cpu = get_cpu_status();
    //resp.cpu_temperature = get_cpu_temperature();
    //resp.memory = get_memory_status();
    //resp.usb = get_usb_status();
    float t = 0;
    ms_sys_get_cpu_temperature(&t);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cpu=%.1f&", resp.cpu);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cpu_temperature=%f&", t);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "memory=%d&", resp.memory);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "usb=%d&", resp.usb);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_syetem_set_reboot(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_syetem_set_shutdown(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_syetem_get_sdkversion(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sdkversion=%s&", PARAMS_SDK_VERSION);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "p2psdkversion=%s&", PARAMS_P2P_SDK_VERSION);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_syetem_set_system_reset(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct reset_type reset;

    memset(&reset, 0, sizeof(struct reset_type));
    if (!sdkp2p_get_section_info(buf, "keepIp=", sValue, sizeof(sValue))) {
        reset.keepIp = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "keepUser=", sValue, sizeof(sValue))) {
        reset.keepUser = atoi(sValue);
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RESET_TYPE, &reset, sizeof(struct reset_type), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_RESET, NULL, 0, SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SYSTEM_REBOOT, NULL, 0, SDKP2P_NOT_CALLBACK);

    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_ADMIN_RESET_REMOTE, SUB_PARAM_NONE, 0, 0, NULL, 0);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_REBOOT_REMOTE, SUB_PARAM_NONE, 0, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_syetem_send_fimage_init(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_p2p_send_data_header *fimage_header = (struct req_p2p_send_data_header *)param;

    memset(fimage_header, 0x0, sizeof(struct req_p2p_send_data_header));
    if (!sdkp2p_get_section_info(buf, "number=", sValue, sizeof(sValue))) {
        fimage_header->number = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "count=", sValue, sizeof(sValue))) {
        fimage_header->count = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "offset=", sValue, sizeof(sValue))) {
        fimage_header->offset = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "file_size=", sValue, sizeof(sValue))) {
        fimage_header->file_size = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "datalen=", sValue, sizeof(sValue))) {
        fimage_header->datalen = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "crc=", sValue, sizeof(sValue))) {
        fimage_header->crc = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "reset=", sValue, sizeof(sValue))) {
        fimage_header->reset = atoi(sValue);
    }
    *datalen = sizeof(struct req_p2p_send_data_header);
    //sdkp2p_send_msg(conf, conf->req, &fimage_header, sizeof(struct req_p2p_send_data_header), SDKP2P_NEED_CALLBACK);
}

static void resp_syetem_send_fimage_init(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_syetem_upgrade_system(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int reset = 0;

    sdkp2p_send_msg(conf, REQUEST_FLAG_UPGRADE_SYSTEM_INIT, NULL, 0, SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, conf->req, &reset, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_syetem_upgrade_system(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void resp_syetem_p2p_export_config(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    if (size > len) {
        *datalen = len;
    } else {
        *datalen = size;
    }

    memcpy(buf, param, *datalen);
}

static void req_syetem_p2p_import_config(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    //make sure sizeof param >= len;
    memcpy(param, buf, len);
    *datalen = len;
}

static void req_syetem_p2p_export_config(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    //*datalen = 0;
}

static void req_syetem_p2p_creat_chan(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    ;//
}

static void resp_syetem_p2p_creat_chan(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_syetem_p2p_close_chan(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    ;//
}

static void resp_syetem_p2p_close_chan(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}



static void req_system_check_online_nvr(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_system_check_online_nvr(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_check_online_upgrade *info = (struct resp_check_online_upgrade *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "state=%d&", info->state);
    snprintf(buf + strlen(buf), len - strlen(buf), "fileSize=%d&", info->fileSize);
    snprintf(buf + strlen(buf), len - strlen(buf), "softversion=%s&", info->pSoftversion);
    snprintf(buf + strlen(buf), len - strlen(buf), "url=%s&", info->pUrl);
    snprintf(buf + strlen(buf), len - strlen(buf), "description=%s&", info->pDescription);
    *datalen = strlen(buf) + 1;
}

static void req_syetem_get_upgrade_image(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int type = 0;
    struct ms_online_upgrade_image info;

    memset(&info, 0x0, sizeof(struct ms_online_upgrade_image));
    if (sdkp2p_get_section_info(buf, "url=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    get_url_decode(sValue);
    snprintf(info.pUrl, sizeof(info.pUrl), "%s", sValue);

    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }

    if (type == 1) {
        snprintf(info.filepath, sizeof(info.filepath), "%s", UPDATE_IMAGE_NAME_IPC);
    } else {
        snprintf(info.filepath, sizeof(info.filepath), "%s", UPDATE_IMAGE_NAME);
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct ms_online_upgrade_image), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_system_get_upgrade_image(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_system_del_upgrade_image(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int type = 0;

    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }

    if (type == 0) {
        if (!ms_check_file_exist(UPDATE_IMAGE_NAME)) {
            unlink(UPDATE_IMAGE_NAME);
        }
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    } else {
        sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    }

    return;
}

static void resp_system_del_upgrade_image(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_system_check_online_ipc(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[65] = {0};
    long long chs = 0;
    int i = 0;

    if (sdkp2p_get_section_info(buf, "chnMaskl=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        if (sValue[i] == '0') {
            continue;
        } else if (sValue[i] == '1') {
            chs |= (long long)1 << i;
        } else {
            break;
        }
    }

    sdkp2p_send_msg(conf, conf->req, &chs, sizeof(long long), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_system_check_online_ipc(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct resp_check_online_upgrade_ipc_batch *info = (struct resp_check_online_upgrade_ipc_batch *)param;

    for (i = 0; i < MAX_CAMERA; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "state[%d]=%d&", i, info->upInfo[i].state);
        snprintf(buf + strlen(buf), len - strlen(buf), "fileSize[%d]=%d&", i, info->upInfo[i].fileSize);
        snprintf(buf + strlen(buf), len - strlen(buf), "version[%d]=%s&", i, info->upInfo[i].pSoftversion);
        snprintf(buf + strlen(buf), len - strlen(buf), "url[%d]=%s&", i, info->upInfo[i].pUrl);
        snprintf(buf + strlen(buf), len - strlen(buf), "desc[%d]=%s&", i, info->upInfo[i].pDescription);
    }

    *datalen = strlen(buf) + 1;
}

static void req_system_ipc_online_upgrade(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[128] = {0};
    struct req_upgrade_ipc_image info;

    memset(&info, 0x0, sizeof(struct req_upgrade_ipc_image));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    info.chnid = atoi(sValue);
    info.keepconfig = 1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.keepconfig = atoi(sValue);
    }
    snprintf(info.filepath, sizeof(info.filepath), "%s", CAMERA_UPGRADE_FILE_PATH);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct req_upgrade_ipc_image), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_system_ipc_online_upgrade(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;
    struct resp_upgrade_ipc_image *info = (struct resp_upgrade_ipc_image *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid[%d]=%d&", i, info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "status[%d]=%d&", i, info->status);

    *datalen = strlen(buf) + 1;
}

static void req_syetem_p2p_creat_write_chan(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    ;//
}

static void resp_syetem_p2p_creat_write_chan(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_syetem_p2p_creat_close_chan(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    ;//
}

static void resp_syetem_p2p_creat_close_chan(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_syetem_upgrade_init(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_system_get_access_filter(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct access_list *info = (struct access_list *)ms_calloc(MAX_LIMIT_ADDR, sizeof(struct access_list));
    if (!info) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    int i = 0;
    int cnt = 0;
    read_access_filter(SQLITE_FILE_NAME, info, &cnt);
    int enable = get_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_ENABLE, 0);
    int type = get_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_TYPE, 1);

    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "filter_type=%d&", type);
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", cnt);

    for (i = 0; i < cnt; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "address_type[%d]=%d&", i, info->type);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "address[%d]=%s&", i, info->address);
    }

    *(conf->len) = strlen(conf->resp) + 1;
    ms_free(info);
    return ;
}

static void req_system_set_access_filter(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    int cnt = 0;
    int type = 0;
    int enable = 0;
    char tmp[65] = {0};
    char sValue[128] = {0};

    if (sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    enable = (atoi(sValue) == 1 ? 1 : 0);

    if (sdkp2p_get_section_info(buf, "filter_type=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    type = (atoi(sValue) == 1 ? 1 : 0);

    set_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_ENABLE, enable);
    set_param_int(SQLITE_FILE_NAME, PARAM_ACCESS_FILTER_TYPE, type);

    if (!sdkp2p_get_section_info(buf, "cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }
    if (cnt < 0 || cnt > MAX_LIMIT_ADDR) {
        cnt = 0;
    }

    struct access_list *info = (struct access_list *)ms_calloc(MAX_LIMIT_ADDR, sizeof(struct access_list));

    for (i = 0; i < cnt; i++) {
        snprintf(tmp, sizeof(tmp), "address_type[%d]=", i);
        if (!sdkp2p_get_section_info(buf, "filter_type=", sValue, sizeof(sValue))) {
            info[i].type = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "address[%d]=", i);
        if (!sdkp2p_get_section_info(buf, "filter_type=", sValue, sizeof(sValue))) {
            snprintf(info[i].address, sizeof(info[i].address), "%s", sValue);
        }
    }
    write_access_filter(SQLITE_FILE_NAME, info, cnt);
    ms_free(info);

    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_system_p2p_export_diagnosis(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    if (access(DIAGNOSTIC_BAK_LOG, F_OK) >= 0) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    char path[MAX_LEN_64] = {0};
    snprintf(path, sizeof(path), "%s", "");
    sdkp2p_send_msg(conf, conf->req, (void *)&path, sizeof(path), SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_system_p2p_export_diagnosis(void *param, int size, char *buf, int len, int *datalen)
{
    FILE *fp = NULL;
    char cmd[300] = {0};
    struct stat filestat;
    struct req_p2p_send_file file;
    char file_path[MAX_LEN_256] = {0};
    struct resp_diagnostic_log_export *info = (struct resp_diagnostic_log_export *)param;
    memset(&file, 0, sizeof(file));
    if (info) {
        if (info->res == 0) {
            snprintf(file_path, sizeof(file_path), "%s", info->filename);
            fp = fopen(file_path, "rb");
            if (fp) {
                if (fstat(fileno(fp), &filestat) != -1 && (filestat.st_size + sizeof(struct req_p2p_send_file) < len)) {
                    if (fread(buf + sizeof(struct req_p2p_send_file), filestat.st_size, 1, fp) == 1) {
                        snprintf(file.name, sizeof(file.name), "%s", 1 + strrchr(file_path, '/'));
                        file.datalen = filestat.st_size;
                        memcpy(buf, &file, sizeof(struct req_p2p_send_file));
                        msdebug(DEBUG_INF, "[htx]p2p_sdk 11111111 file.name:[%s], datalen:[%d]", file.name, file.datalen);
                    }
                }
                fclose(fp);
            }
            snprintf(cmd, sizeof(cmd), "/bin/rm %s", file_path);
            ms_system(cmd);
        } else {
            snprintf(buf + strlen(buf), len - strlen(buf), "error res=%d&", info->res);
        }
    }

    *datalen = strlen(buf) + 1;
    if (file.datalen > 0) {
        *datalen = sizeof(struct req_p2p_send_file) + file.datalen;
    }
}

static void req_system_get_auto_reboot(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    reboot_conf reboot;

    memset(&reboot, 0, sizeof(reboot_conf));
    read_autoreboot_conf(SQLITE_FILE_NAME, &reboot);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", reboot.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wday=%d&", reboot.wday);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "hour=%d&", reboot.hour);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "minutes=%d&", reboot.minutes);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "seconds=%d&", reboot.seconds);
    *(conf->len) = strlen(conf->resp) + 1;
    return;
}

static void req_system_set_auto_reboot(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[65] = {0};
    reboot_conf reboot;

    memset(&reboot, 0, sizeof(reboot_conf));
    read_autoreboot_conf(SQLITE_FILE_NAME, &reboot);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        reboot.enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "wday=", sValue, sizeof(sValue))) {
        reboot.wday = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "hour=", sValue, sizeof(sValue))) {
        reboot.hour = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "minutes=", sValue, sizeof(sValue))) {
        reboot.minutes = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "seconds=", sValue, sizeof(sValue))) {
        reboot.seconds = atoi(sValue);
    }
    write_autoreboot_conf(SQLITE_FILE_NAME, &reboot);
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_system_set_auto_reboot(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_get_audio_file_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_system_get_audio_file_info(void *param, int size, char *buf, int len, int *datalen)
{
    if (!param) {
        return ;
    }
    int i = 0;
    int cnt = 0;
    AUDIO_FILE *info = (AUDIO_FILE *)param;
    char name[AUDIO_FILE_NAME_SIZE] = {0};

    for (i = 0; i < cnt; i++) {
        if (info[i].enable) {
            snprintf(buf + strlen(buf), len - strlen(buf), "id[%d]=%d&", i, info[i].id);
            snprintf(buf + strlen(buf), len - strlen(buf), "enable[%d]=%d&", i, info[i].enable);
            if (base64DecodeUrl((MS_U8 *)name, sizeof(name), (MS_U8 *)info[i].name, strlen(info[i].name)) > 0) {
                snprintf(buf + strlen(buf), len - strlen(buf), "name[%d]=%s&", i, name);
            }
            cnt++;
        }
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", cnt);

    *datalen = strlen(buf) + 1;
}

static void req_system_upload_audio_file(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    //make sure sizeof param >= len;
    memcpy(param, buf, len);
    *datalen = len;
}

static void req_system_add_audio_file(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret = -100;
    struct log_data logData;
    char sValue[MAX_LEN_128] = {0};
    char name[AUDIO_FILE_NAME_SIZE] = {0};
    char pName[AUDIO_FILE_NAME_SIZE] = {0};
    char uploadName[AUDIO_FILE_NAME_SIZE] = {0};

    if (sdkp2p_get_section_info(buf, "uploadName=", sValue, sizeof(sValue))) {
        snprintf(uploadName, sizeof(uploadName), "%s/%s", PATH_TEMP_FILE, sValue);
    }

    if (access(uploadName, R_OK)) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    ret = base64EncodeUrl((MS_U8 *)name, sizeof(name), (MS_U8 *)sValue, strlen(sValue));
    snprintf(pName, sizeof(pName), "%s/%s", PATH_TEMP_FILE, name);
    if (ret > 0 && !rename(uploadName, pName)) {
        sdkp2p_send_msg(conf, conf->req, name, sizeof(name), SDKP2P_NEED_CALLBACK);

        logData.log_data_info.mainType = MAIN_OP;
        logData.log_data_info.subType = SUB_OP_CONFIG_REMOTE;
        logData.log_data_info.parameter_type = SUB_PARAM_AUDIO_FILE;
        snprintf(logData.log_data_info.ip, sizeof(logData.log_data_info.ip), "%s", conf->ip_addr);
        snprintf(logData.log_data_info.user, sizeof(logData.log_data_info.user), "%s", conf->user);
        sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE, &logData, sizeof(logData), SDKP2P_NOT_CALLBACK);
    }

    return ;
}

static void resp_system_add_audio_file(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_del_audio_file(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int idMask = 0;
    struct log_data logData;
    char sValue[MAX_LEN_128] = {0};
    memset(&logData, 0 , sizeof(logData));

    if (!sdkp2p_get_section_info(buf, "idMask=", sValue, sizeof(sValue))) {
        idMask = atoi(sValue);
    }

    if ((idMask & 0x03ff)) {
        sdkp2p_send_msg(conf, conf->req, &idMask, sizeof(idMask), SDKP2P_NEED_CALLBACK);
        
        logData.log_data_info.mainType = MAIN_OP;
        logData.log_data_info.subType = SUB_OP_CONFIG_REMOTE;
        logData.log_data_info.parameter_type = SUB_PARAM_AUDIO_FILE;
        snprintf(logData.log_data_info.ip, sizeof(logData.log_data_info.ip), "%s", conf->ip_addr);
        snprintf(logData.log_data_info.user, sizeof(logData.log_data_info.user), "%s", conf->user);
        sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE, &logData, sizeof(logData), SDKP2P_NOT_CALLBACK);
    } else {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "Param error");
        *(conf->len) = strlen(conf->resp) + 1;
    }

    return ;
}

static void resp_system_del_audio_file(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_edit_audio_file(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret = 0;
    AUDIO_FILE info = {0};
    struct log_data logData;
    char name[MAX_LEN_128] = {0};
    char sValue[MAX_LEN_128] = {0};

    info.enable = 1;
    memset(&logData, 0 , sizeof(logData));

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        info.id = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        snprintf(name, sizeof(name), "%s", sValue);
    }

    ret = base64EncodeUrl((MS_U8 *)info.name, sizeof(info.name), (MS_U8 *)name, strlen(name));
    if (ret > 0) {
        sdkp2p_send_msg(conf, conf->req, &info, sizeof(info), SDKP2P_NEED_CALLBACK);
        
        logData.log_data_info.mainType = MAIN_OP;
        logData.log_data_info.subType = SUB_OP_CONFIG_REMOTE;
        logData.log_data_info.parameter_type = SUB_PARAM_AUDIO_FILE;
        snprintf(logData.log_data_info.ip, sizeof(logData.log_data_info.ip), "%s", conf->ip_addr);
        snprintf(logData.log_data_info.user, sizeof(logData.log_data_info.user), "%s", conf->user);
        sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE, &logData, sizeof(logData), SDKP2P_NOT_CALLBACK);
    } else {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "Param error ret:[%d]", ret);
        *(conf->len) = strlen(conf->resp) + 1;
    }

    return ;
}

static void resp_system_edit_audio_file(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_play_audio_file(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int id = -1;
    char sValue[MAX_LEN_128] = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        id = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &id, sizeof(id), SDKP2P_NEED_CALLBACK);

    return ;
}

static void resp_system_play_audio_file(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_get_audio_file_play_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_system_get_audio_file_play_status(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_get_two_way_audio_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_system_get_two_way_audio_status(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "audio_busy=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_two_way_audio_open(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[128] = {0};
    REQ_P2P_TWO_WAY_AUDIO_OPT *res = (REQ_P2P_TWO_WAY_AUDIO_OPT *)param;

    if (!sdkp2p_get_section_info(buf, "platform=", sValue, sizeof(sValue))) {
        res->platform = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "opt=", sValue, sizeof(sValue))) {
        res->opt = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        res->chnid = atoi(sValue);
    }
    *datalen = sizeof(REQ_P2P_TWO_WAY_AUDIO_OPT);
    
    return ;
}

static void resp_system_two_way_audio_open(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

static void req_system_two_way_audio_close(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[128] = {0};
    REQ_P2P_TWO_WAY_AUDIO_OPT *res = (REQ_P2P_TWO_WAY_AUDIO_OPT *)param;
    
    if (!sdkp2p_get_section_info(buf, "platform=", sValue, sizeof(sValue))) {
        res->platform = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "opt=", sValue, sizeof(sValue))) {
        res->opt = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        res->chnid = atoi(sValue);
    }
    *datalen = sizeof(REQ_P2P_TWO_WAY_AUDIO_OPT);
    
    return ;
}

static void resp_system_two_way_audio_close(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -100;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}

SDK_ERROR_CODE get_system_environment_str(const char *buff, struct ReqEnvInfo *info)
{
    char sValue[MAX_LEN_128] = {0};
    int flag = 0;
    
    memset(info, 0, sizeof(struct ReqEnvInfo));
    if (!sdkp2p_get_section_info(buff, "notReboot=", sValue, sizeof(sValue))) {
        info->notReboot = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "sncode=", sValue, sizeof(sValue))) {
        if (strlen(sValue) == 0) {
            snprintf(info->sncode, sizeof(info->sncode), "%s", " ");
        } else {
            get_url_decode(sValue);
            snprintf(info->sncode, sizeof(info->sncode), "%s", sValue);
        }
        //if (check_sn_code(info->sncode) != 0) {
        //    return PARAM_ERROR;
        //}
        flag++;
    }
    if (!sdkp2p_get_section_info(buff, "ethaddr=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        if (check_mac_code(sValue) != 0) {
            return PARAM_ERROR;
        }
        snprintf(info->ethaddr, sizeof(info->ethaddr), "%s", sValue);
        flag++;
    }
    if (!sdkp2p_get_section_info(buff, "ethaddr2=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        if (check_mac_code(sValue) != 0) {
            return PARAM_ERROR;
        }
        snprintf(info->ethaddr2, sizeof(info->ethaddr2), "%s", sValue);
        flag++;
    }

    if (!flag) {
        return PARAM_ERROR;
    }
    
    return REQUEST_SUCCESS;
}

static void req_system_set_environment(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    SDK_ERROR_CODE ret;
    struct ReqEnvInfo info;

    ret = get_system_environment_str(buf, &info);
    if (ret != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, ret, NULL);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&info, sizeof(struct ReqEnvInfo), SDKP2P_NEED_CALLBACK);
}

static void resp_system_set_environment(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    *datalen = sdkp2p_json_resp(buf, len, REQUEST_SUCCESS, &res);
}

static void req_system_set_hotspare_mode(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    SDK_ERROR_CODE ret;
    struct ms_failover_change_mode mode;

    ret = set_hotspare_mode_by_text(buf, &mode);
    if (ret != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, ret, NULL);
        return;
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&mode, sizeof(struct ms_failover_change_mode), SDKP2P_NEED_CALLBACK);
}

static void resp_system_common_set(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    *datalen = sdkp2p_json_resp(buf, len, REQUEST_SUCCESS, &resp);

    return;
}

static void req_system_search_hotspare_masters(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_system_search_hotspare_masters(void *param, int size, char *buf, int len, int *datalen)
{
    int cnt =  size / sizeof(struct resq_search_nvr);
    char resp[4096] = {0};
    int ret;

    ret = search_hotspare_masters_json(param, cnt, resp, sizeof(resp));
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    snprintf(buf, len, resp);
    *datalen = strlen(buf) + 1;
}

static void req_system_add_hotspare_masters(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    SDK_ERROR_CODE ret;
    // int i;
    Uint32 masterMask = 0;
    Uint32 delMask = 0;
    struct failover_list masters[MAX_FAILOVER];
    struct ms_failover_update_master update;
    // struct log_data logData;
    memset(masters, 0, sizeof(masters));
    memset(&update, 0x0, sizeof(struct ms_failover_update_master));

    ret = add_hotspare_masters_by_text(buf, masters, &update, &masterMask);
    if (ret != REQUEST_SUCCESS) {
        *(conf->len) = sdkp2p_json_resp(conf->resp, conf->size, ret, NULL);
        return;
    }

    if (update.type == 0) {
        sdkp2p_send_msg(conf, REQUEST_FLAG_FAILOVER_UPDATE_MASTER, &delMask, sizeof(Uint32), SDKP2P_NOT_CALLBACK);
    }

#if 0 // wcm delete
    if (masterMask) {
        for (i = 0; i < MAX_FAILOVER; ++i) {
            if (!(masterMask & ((Uint32)1 << i))) {
                continue;
            }
            init_hotspare_log(0, masters[i].ipaddr, conf->ip_addr, conf->user, &logData);
            sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE, &logData, sizeof(struct log_data), SDKP2P_NOT_CALLBACK);
        }
    }
#endif            

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_system_get_hotspare_masters_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_system_get_hotspare_masters_status(void *param, int size, char *buf, int len, int *datalen)
{
    int ret = -1;
    int cnt = size / sizeof(struct ms_failover_master_status);

    ret = get_hotspare_masters_status_json(param, cnt, buf, len);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, ret, NULL);
        return;
    }
    *datalen = strlen(buf) + 1;
}

static void req_system_del_hotspare_masters(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int ret, sum = 0;
    Uint32 delMask = 0;
    char ips[MAX_FAILOVER][16] = {{0}};
    struct ms_failover_update_master update;
    // struct log_data logData;
    memset(&update, 0x0, sizeof(struct ms_failover_update_master));
    // memset(&logData, 0x0, sizeof(struct log_data));

    ret = del_hotspare_masters_by_text(buf, ips, &sum, &delMask);
    if (ret) {
        *(conf->len) = sdkp2p_json_resp(buf, len, OTHERS_ERROR, NULL);
        return;
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_FAILOVER_UPDATE_MASTER, &delMask, sizeof(Uint32), SDKP2P_NOT_CALLBACK);


#if 0 // wcm delete
    for (i = 0; i < sum; ++i) {
        update.type = 1;
        snprintf(update.ipaddr, sizeof(update.ipaddr), ips[i]);
        sdkp2p_send_msg(conf, REQUEST_FLAG_FAILOVER_UPDATE_MASTER, &update, sizeof(struct ms_failover_update_master), SDKP2P_NOT_CALLBACK);
        init_hotspare_log(1, ips[i], conf->ip_addr, conf->user, &logData);
        sdkp2p_send_msg(conf, REQUEST_FLAG_LOG_WRITE, &logData, sizeof(struct log_data), SDKP2P_NOT_CALLBACK);
    }
#endif

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_system_get_hotspare_slave_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_system_get_hotspare_slave_status(void *param, int size, char *buf, int len, int *datalen)
{
    int ret = -1;

    ret = get_hotspare_slave_status_json(param, size, buf, len);
    if (ret) {
        *datalen = sdkp2p_json_resp(buf, len, ret, NULL);
        return;
    }
    *datalen = strlen(buf) + 1;
}

static void req_system_get_online_upgrade_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
    return;
}

static void resp_system_get_online_upgrade_info(void *param, int size, char *buf, int len, int *datalen)
{
    char reqUrl[8] = {0};

    snprintf(reqUrl, sizeof(reqUrl), "%d", REQUEST_FLAG_GET_ONLINE_UPGRADE_INFO);
    ware_system_get_online_upgrade_info_out(reqUrl, param, size/sizeof(struct resp_check_online_upgrade), buf, len);
    *datalen = strlen(buf) + 1;
    return;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_SYSTEM_TIME, req_system_get_system_time, -1, NULL},//get.system.time
    {REQUEST_FLAG_SET_SYSTIME_SDK, req_syetem_set_system_time, -1, NULL},//set.system.time
    {REQUEST_FLAG_GET_SYSTEM_STATUS, req_syetem_get_system_status, -1, NULL},//get.system.sysstatus
    {REQUEST_FLAG_GET_SYSINFO, NULL, RESPONSE_FLAG_GET_SYSINFO, resp_syetem_get_system_info},//get.system.sysinfo
    {REQUEST_FLAG_SYSTEM_REBOOT, req_syetem_set_reboot, -1, NULL},//set.system.reboot
    {REQUEST_FLAG_SET_DEVICEID, req_syetem_set_device_id, -1, NULL},//set.system.devid
    {REQUEST_FLAG_SYSTEM_SHUTDOWN, req_syetem_set_shutdown, -1, NULL},//set.system.shutdown
    {REQUEST_FLAG_GET_SDK_VERSION, req_syetem_get_sdkversion, -1, NULL},//get.system.sdkversion
    {REQUEST_FLAG_SET_SYSTEM_RESET, req_syetem_set_system_reset, -1, NULL},//set.system.reset
    {REQUEST_FLAG_CHECK_ONLINE_NVR, req_system_check_online_nvr, RESPONSE_FLAG_CHECK_ONLINE_NVR, resp_system_check_online_nvr},//get.system.check_nvr_version
    {REQUEST_FLAG_GET_UPGRADE_IMAGE, req_syetem_get_upgrade_image, RESPONSE_FLAG_GET_UPGRADE_IMAGE, resp_system_get_upgrade_image},//get.system.get_image
    {REQUEST_FLAG_DEL_UPGRADE_IMAGE, req_system_del_upgrade_image, RESPONSE_FLAG_DEL_UPGRADE_IMAGE, resp_system_del_upgrade_image},//set.system.del_image
    {REQUEST_FLAG_P2P_SEND_FIMAGE_INIT, req_syetem_send_fimage_init, RESPONSE_FLAG_P2P_SEND_FIMAGE_INIT, resp_syetem_send_fimage_init},
    {REQUEST_FLAG_UPGRADE_SYSTEM, req_syetem_upgrade_system, RESPONSE_FLAG_UPGRADE_SYSTEM, resp_syetem_upgrade_system},///set.system.nvr_upgrade
    {REQUEST_FLAG_P2P_EXPORT_CONFIG, req_syetem_p2p_export_config, RESPONSE_FLAG_P2P_EXPORT_CONFIG, resp_syetem_p2p_export_config},
    {REQUEST_FLAG_P2P_IMPORT_CONFIG, req_syetem_p2p_import_config, RESPONSE_FLAG_P2P_IMPORT_CONFIG, NULL},
    {REQUEST_FLAG_P2P_CREAT_CHAN, req_syetem_p2p_creat_chan, RESPONSE_FLAG_P2P_CREAT_CHAN, resp_syetem_p2p_creat_chan},
    {REQUEST_FLAG_P2P_CLOSE_CHAN, req_syetem_p2p_close_chan, RESPONSE_FLAG_P2P_CLOSE_CHAN, resp_syetem_p2p_close_chan},

    {REQUEST_FLAG_SDK_CHECK_VERSION, sdk_check_version_to_struct, RESPONSE_FLAG_SDK_CHECK_VERSION, sdk_check_version_to_str},
    {REQUEST_FLAG_P2P_PREVIEW_OPERATION, req_system_preview_operation, -1, NULL},
    {REQUEST_FLAG_SET_SYSTIME, req_syetem_set_system_time, RESPONSE_FLAG_SET_SYSTIME, NULL},

    {-1, NULL, RESPONSE_FLAG_BANDWIDTH_FULL, bandwidth_full_to_str},
    {REQUEST_FLAG_WEB_AUTO_LOGIN_CHECK, NULL, RESPONSE_FLAG_WEB_AUTO_LOGIN_CHECK, web_auto_login_check_to_str},

    {REQUEST_FLAG_CHECK_ONLINE_IPC_BATCH, req_system_check_online_ipc, RESPONSE_FLAG_CHECK_ONLINE_IPC_BATCH, resp_system_check_online_ipc},//get.system.check_ipc_version
    {REQUEST_FLAG_UPGRADE_IPC_IMAGE, req_system_ipc_online_upgrade, RESPONSE_FLAG_UPGRADE_IPC_IMAGE, resp_system_ipc_online_upgrade},//set.system.ipc_upgrade
    {REQUEST_FLAG_P2P_CREAT_WRITE_CHAN, req_syetem_p2p_creat_write_chan, RESPONSE_FLAG_P2P_CREAT_WRITE_CHAN, resp_syetem_p2p_creat_write_chan},
    {REQUEST_FLAG_P2P_CLOSE_WRITE_CHAN, req_syetem_p2p_creat_close_chan, RESPONSE_FLAG_P2P_CLOSE_WRITE_CHAN, resp_syetem_p2p_creat_close_chan},
    {REQUEST_FLAG_UPGRADE_SYSTEM_INIT, req_syetem_upgrade_init, RESPONSE_FLAG_UPGRADE_SYSTEM_INIT, NULL},//set.system.upgrade_init
    {REQUEST_FLAG_GET_ACCESS_FILTER, req_system_get_access_filter, -1, NULL},//get.system.access_filter
    {REQUEST_FLAG_SET_ACCESS_FILTER, req_system_set_access_filter, RESPONSE_FLAG_SET_ACCESS_FILTER, NULL},//set.system.access_filter
    {REQUEST_FLAG_EXPORT_DIAGNOSTIC_LOG, req_system_p2p_export_diagnosis, RESPONSE_FLAG_EXPORT_DIAGNOSTIC_LOG, resp_system_p2p_export_diagnosis},
    {REQUEST_FLAG_GET_AUTO_REBOOT, req_system_get_auto_reboot, RESPONSE_FLAG_GET_AUTO_REBOOT, NULL},//get.system.auto_reboot
    {REQUEST_FLAG_SET_AUTO_REBOOT, req_system_set_auto_reboot, RESPONSE_FLAG_SET_AUTO_REBOOT, resp_system_set_auto_reboot},//set.system.auto_reboot

    {REQUEST_FLAG_GET_AUDIOFILE_INFO, req_system_get_audio_file_info, RESPONSE_FLAG_GET_AUDIOFILE_INFO, resp_system_get_audio_file_info},//get.system.audio_file_info
    {REQUEST_FLAG_P2P_UPLOAD_AUDIOFILE, req_system_upload_audio_file, RESPONSE_FLAG_P2P_UPLOAD_AUDIOFILE, NULL},//WEB_UPLOAD_AUDIO_FILE
    {REQUEST_FLAG_ADD_AUDIOFILE, req_system_add_audio_file, RESPONSE_FLAG_ADD_AUDIOFILE, resp_system_add_audio_file},//WEB_ADD_AUDIO_FILE
    {REQUEST_FLAG_DEL_AUDIOFILE, req_system_del_audio_file, RESPONSE_FLAG_DEL_AUDIOFILE, resp_system_del_audio_file},//del.system.audio_file
    {REQUEST_FLAG_EDIT_AUDIOFILE, req_system_edit_audio_file, RESPONSE_FLAG_EDIT_AUDIOFILE, resp_system_edit_audio_file},//edit.system.audio_file
    {REQUEST_FLAG_PLAY_AUDIOFILE, req_system_play_audio_file, RESPONSE_FLAG_PLAY_AUDIOFILE, resp_system_play_audio_file},//play.system.audio_file
    {REQUEST_FLAG_GET_AUDIOFILE_PALY_STATUS, req_system_get_audio_file_play_status, RESPONSE_FLAG_GET_AUDIOFILE_PALY_STATUS, resp_system_get_audio_file_play_status},//get.system.audio_file_play_status
    {REQUEST_FLAG_CHECK_AUDIOTALK, req_system_get_two_way_audio_status, RESPONSE_FLAG_CHECK_AUDIOTALK, resp_system_get_two_way_audio_status},//get.status.nvr_audio_status
    {REQUEST_P2P_TWOWAY_AUDIO_OPEN, req_system_two_way_audio_open, RESPONSE_P2P_TWOWAY_AUDIO_OPEN, resp_system_two_way_audio_open},//get.status.nvr_audio_status
    {REQUEST_P2P_TWOWAY_AUDIO_CLOSE, req_system_two_way_audio_close, RESPONSE_P2P_TWOWAY_AUDIO_CLOSE, resp_system_two_way_audio_close},//get.status.nvr_audio_status
    {REQUEST_FLAG_SET_ENVIRONMENT_INFO, req_system_set_environment, RESPONSE_FLAG_SET_ENVIRONMENT_INFO, resp_system_set_environment},//set.system.environment
    {REQUEST_FLAG_FAILOVER_CHANGE_MODE, req_system_set_hotspare_mode, RESPONSE_FLAG_FAILOVER_CHANGE_MODE, resp_system_common_set}, // set.system.hotspare.mode
    {REQUEST_FLAG_FAILOVER_SEARCH_NVR, req_system_search_hotspare_masters, RESPONSE_FLAG_FAILOVER_SEARCH_NVR, resp_system_search_hotspare_masters}, // get.system.hotspare.masters
    {REQUEST_FLAG_ADD_HOTSPARE_MASTERS, req_system_add_hotspare_masters, RESPONSE_FLAG_ADD_HOTSPARE_MASTERS, NULL}, // add.system.hotspare.masters
    {REQUEST_FLAG_FAILOVER_GET_MASTER_STATUS, req_system_get_hotspare_masters_status, RESPONSE_FLAG_FAILOVER_GET_MASTER_STATUS, resp_system_get_hotspare_masters_status}, // get.system.hotspare.masters.status
    {REQUEST_FLAG_DEL_HOTSPARE_MASTERS, req_system_del_hotspare_masters, RESPONSE_FLAG_DEL_HOTSPARE_MASTERS, NULL}, // del.system.hotspare.masters
    {REQUEST_FLAG_FAILOVER_GET_SLAVE_STATUS, req_system_get_hotspare_slave_status, RESPONSE_FLAG_FAILOVER_GET_SLAVE_STATUS, resp_system_get_hotspare_slave_status}, // get.system.hotspare.slave.status
    {REQUEST_FLAG_GET_ONLINE_UPGRADE_INFO, req_system_get_online_upgrade_info, RESPONSE_FLAG_GET_ONLINE_UPGRADE_INFO, resp_system_get_online_upgrade_info}
    // request register array end
};

void p2p_system_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_system_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

