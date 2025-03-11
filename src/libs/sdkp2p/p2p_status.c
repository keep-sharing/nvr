#include "sdk_util.h"

struct EventStatusStr {
    char videoLoss[MAX_LEN_65];
    char motion[MAX_LEN_65];
    char audioAlarm[MAX_LEN_65];
    char lpr[MAX_LEN_65];
    char vcaEntrance[MAX_LEN_65];
    char vcaExiting[MAX_LEN_65];
    char vcaMotion[MAX_LEN_65];
    char vcaTamper[MAX_LEN_65];
    char vcaLinecross[MAX_LEN_65];
    char vcaLoiter[MAX_LEN_65];
    char vcaHuman[MAX_LEN_65];
    char vcaObject[MAX_LEN_65];
    char vcaPcntIn[MAX_LEN_65];
    char vcaPcntOut[MAX_LEN_65];
    char vcaAlarm[VCA_REGIONIN][MAX_LEN_65];
    char ipcAlarmInStatus[MAX_IPC_ALARM_IN][MAX_LEN_65];
    char ipcAlarmInType[MAX_IPC_ALARM_IN][MAX_LEN_65];
    char ipcAlarmOutStatus[MAX_IPC_ALARM_OUT][MAX_LEN_65];
    char ipcAlarmOutType[MAX_IPC_ALARM_OUT][MAX_LEN_65];
    char nvrAlarmInStatus[MAX_LEN_65];
    char nvrAlarmInType[MAX_LEN_65];
    char nvrAlarmOutStatus[MAX_LEN_65];
    char nvrAlarmOutType[MAX_LEN_65];
};

int get_poe_port_status_json(void *data, int cnt, char *resp, int respLen)
{
    char *sResp = NULL;
    cJSON *json = NULL;
    cJSON *array = NULL;
    cJSON *obj = NULL;
    int i;
    struct resq_poe_state *poe = (struct resq_poe_state *)data;

    if (!data || !cnt || !resp || respLen <= 0) {
        return -1;
    }
    json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }
    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    cJSON_AddNumberToObject(json, "num", poe->port_num);
    array = cJSON_CreateArray();
    for (i = 0; i < poe->port_num && i < MAX_PORT_NUM; i++) {
        obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "ip", poe->ipaddr[i]);
        cJSON_AddNumberToObject(obj, "status", poe->connected[i]);
        cJSON_AddNumberToObject(obj, "power", poe->poe_port_power[i]);
        cJSON_AddItemToArray(array, obj);
    }
    cJSON_AddItemToObject(json, "poe", array);
    sResp = cJSON_Print(json);
    cJSON_Delete(json);
    if (!sResp) {
        return -1;
    }
    snprintf(resp, respLen, "%s", sResp);
    cJSON_free(sResp);
    return 0;
}


static void resp_status_get_ipc_status(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_camera_status *;
    struct resp_camera_status *res = (struct resp_camera_status *)param;
    int num = size / sizeof(struct resp_camera_status);
    int i, j;
    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", num);
    for (i = 0; i < num; i++) {
        for (j = 0; j < 2; j++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status[%d].cur_res.width=%d&", i, j,
                     res[i].status[j].cur_res.width);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status[%d].cur_res.height=%d&", i, j,
                     res[i].status[j].cur_res.height);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status[%d].frame_rate=%d&", i, j, res[i].status[j].frame_rate);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status[%d].bit_rate=%d&", i, j, res[i].status[j].bit_rate);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status[%d].connection=%d&", i, j, res[i].status[j].connection);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status[%d].connectStatus=%d&", i, j,
                     res[i].status[j].connectStatus);
            snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].status[%d].connectStatus=%d&", i, j,
                     res[i].status[j].connectStatus);
        }
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].record=%d&", i, res[i].record);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].alarm=%d&", i, res[i].alarm);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].chnid=%d&", i, res[i].chnid);
    }

    *datalen = (int)strlen(buf) + 1;
}

static void resp_status_get_network_bandwidth(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_network_bandwidth *res = (struct resp_network_bandwidth *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.total=%d&", res->total);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.free=%d&", res->free);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.used=%d&", res->used);

    *datalen = strlen(buf) + 1;
}

static void resp_status_get_channel_status(void *param, int size, char *buf, int len, int *datalen)
{
    char encode[256] = {0};
    int i, cnt = 0;
    int nRes[MAX_CAMERA] = {0};
    struct osd osds[MAX_CAMERA];
    memset(osds, 0, sizeof(osds));
    memcpy(&nRes, param, sizeof(int)*MAX_CAMERA);
    read_osds(SQLITE_FILE_NAME, osds, &cnt);

    snprintf(buf + strlen(buf), len - strlen(buf), "%s", "channels state=");
    for (i = 0; i < MAX_CAMERA; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "%d", nRes[i]);
    }
    snprintf(buf + strlen(buf), len - strlen(buf), "%s", "&");
    snprintf(buf + strlen(buf), len - strlen(buf), "cnt=%d&", cnt);
    for (i = 0; i < cnt; i++) {
        get_url_encode(osds[i].name, strlen(osds[i].name), encode, sizeof(encode));
        snprintf(buf + strlen(buf), len - strlen(buf), "name[%d]=%s&", i, encode);
    }

    *datalen = strlen(buf) + 1;

}

static void resp_status_get_network_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_network_info *res = (struct resp_network_info *)param;
    int num = size / sizeof(struct resp_network_info);
    int i;
    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", num);
    for (i = 0; i < num; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ifname=%s&", i, res[i].ifname);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].netif_id=%d&", i, res[i].netif_id);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].conn_stat=%d&", i, res[i].conn_stat);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].speed_mode=%d&", i, res[i].speed_mode);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].duplex=%d&", i, res[i].duplex);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].enable=%d&", i, res[i].enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].dhcp_enable=%d&", i, res[i].dhcp_enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ip=%s&", i, res[i].ip);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].mac=%s&", i, res[i].mac);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].primary_dns=%s&", i, res[i].primary_dns);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].second_dns=%s&", i, res[i].second_dns);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].gateway=%s&", i, res[i].gateway);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].netmask=%s&", i, res[i].netmask);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].mtu=%d&", i, res[i].mtu);

        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ip6mode=%d&", i, res[i].ip6_mode); //hrz.milesight
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ip6address=%s&", i, res[i].ip6_address);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ip6prefix=%d&", i, res[i].ip6_prefix);
        snprintf(buf + strlen(buf), len - strlen(buf), "r[%d].ip6gateway=%s&", i, res[i].ip6_gateway);

    }

    *datalen = strlen(buf) + 1;
}

static void inline add_string_mask(char *str, int len, Uint64 mask, int index)
{
    snprintf(str + strlen(str), len - strlen(str), "%s", (mask >> index & 0x01) ? "1" : "0");
}

static void trans_event_status_str(struct EventStatusStr *des, struct resp_event_status *status)
{
    int i, j;
    
    for (i = 0; i < MAX_CAMERA && i < MAX_LEN_64; i++) {
        add_string_mask(des->videoLoss, sizeof(des->videoLoss), status->video_loss, i);
        add_string_mask(des->motion, sizeof(des->motion), status->motion_detect, i);
        add_string_mask(des->audioAlarm, sizeof(des->audioAlarm), status->audioAlarm, i);
        //add_string_mask(des->lpr, sizeof(des->lpr), conf->lprStatus, i);

        //vca status
        add_string_mask(des->vcaEntrance, sizeof(des->vcaEntrance), status->regionin_detect, i);
        add_string_mask(des->vcaExiting, sizeof(des->vcaExiting), status->regionexit_detect, i);
        add_string_mask(des->vcaMotion, sizeof(des->vcaMotion), status->advanced_motion_detect, i);
        add_string_mask(des->vcaTamper, sizeof(des->vcaTamper), status->tamper_detect, i);
        add_string_mask(des->vcaLinecross, sizeof(des->vcaLinecross), status->linecross_detect, i);
        add_string_mask(des->vcaLoiter, sizeof(des->vcaLoiter), status->loiter_detect, i);
        add_string_mask(des->vcaHuman, sizeof(des->vcaHuman), status->human_detect, i);
        add_string_mask(des->vcaObject, sizeof(des->vcaObject), status->object_left_remove, i);
        add_string_mask(des->vcaPcntIn, sizeof(des->vcaPcntIn), status->peopel_count[i][0], i);
        add_string_mask(des->vcaPcntOut, sizeof(des->vcaPcntOut), status->peopel_count[i][1], i);

        //ipc alarm in
        for (j = 0; j < MAX_IPC_ALARM_IN; j++) {
            add_string_mask(des->ipcAlarmInStatus[j], sizeof(des->ipcAlarmInStatus[j]), status->ipc_alarm_in[j], i);
            add_string_mask(des->ipcAlarmInType[j], sizeof(des->ipcAlarmInType[j]), status->ipc_alarm_in_type[j], i);
        }

        //ipc alarm out
        for (j = 0; j < MAX_IPC_ALARM_OUT; j++) {
            add_string_mask(des->ipcAlarmOutStatus[j], sizeof(des->ipcAlarmOutStatus[j]), status->ipc_alarm_out[j], i);
            add_string_mask(des->ipcAlarmOutType[j], sizeof(des->ipcAlarmOutType[j]), status->ipc_alarm_out_type[j], i);
        }
    }

    for (i = 0; i < MAX_ALARM_IN; i++) {
        add_string_mask(des->nvrAlarmInStatus, sizeof(des->nvrAlarmInStatus), status->alarm_in, i);
    }

    for (i = 0; i < MAX_ALARM_OUT; i++) {
        add_string_mask(des->nvrAlarmOutStatus, sizeof(des->nvrAlarmInStatus), status->alarm_out, i);
    }
}

static void resp_status_get_event_status(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0, j = 0, count = 0;
    struct resp_event_status *res = (struct resp_event_status *)param;
    char curTime[32] = {0};

    //int max_ipnc = get_param_int(SQLITE_FILE_NAME, PARAM_MAX_CAM, DEF_MAX_CAMERA_NUM);
    int max_sensor = get_param_int(SQLITE_FILE_NAME, PARAM_MAX_ALARM_IN, DEF_MAX_ALARM_IN_NUM);
    int max_alarm = get_param_int(SQLITE_FILE_NAME, PARAM_MAX_ALARM_OUT, DEF_MAX_ALARM_OUT_NUM);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.video_loss=%lld&", res->video_loss);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.motion_detect=%lld&", res->motion_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.alarm_out=%d&", res->alarm_out);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.alarm_in=%d&", res->alarm_in);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.regionin_detect=%lld&", res->regionin_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.regionexit_detect=%lld&", res->regionexit_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.advanced_motion_detect=%lld&", res->advanced_motion_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.tamper_detect=%lld&", res->tamper_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.linecross_detect=%lld&", res->linecross_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.loiter_detect=%lld&", res->loiter_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.human_detect=%lld&", res->human_detect);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.object_left_remove=%lld&", res->object_left_remove);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.audioAlarm=%lld&", res->audioAlarm);

    for (i = 0; i < MAX_CAMERA; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.in[%d]=%d&", i, res->peopel_count[i][0]);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.out[%d]=%d&", i, res->peopel_count[i][1]);
    }
    for (i = 0; i < MAX_IPC_ALARM_IN; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "ipc_alarm_in[%d]=%llu&", i, res->ipc_alarm_in[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "ipc_alarmin_type[%d]=%llu&", i, res->ipc_alarm_in_type[i]);
    }
    for (i = 0; i < MAX_IPC_ALARM_OUT; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "ipc_alarm_out[%d]=%llu&", i, res->ipc_alarm_out[i]);
        snprintf(buf + strlen(buf), len - strlen(buf), "ipc_alarmout_type[%d]=%llu&", i, res->ipc_alarm_out_type[i]);
    }


    //nvr alarm io
    struct alarm_in alarms[MAX_ALARM_IN];
    memset(&alarms, 0, sizeof(struct alarm_in)*MAX_ALARM_IN);
    read_alarm_ins(SQLITE_FILE_NAME, alarms, &count);
    for (i = 0; i < max_sensor && i < MAX_ALARM_IN; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "nInName[%d]=%s&", i, alarms[i].name);
        snprintf(buf + strlen(buf), len - strlen(buf), "nInType[%d]=%d&", i, alarms[i].type);
    }
    struct alarm_out outs[MAX_ALARM_OUT];
    memset(&outs, 0, sizeof(struct alarm_out) * MAX_ALARM_OUT);
    read_alarm_outs(SQLITE_FILE_NAME, outs, &count);
    for (i = 0; i < max_alarm && i < MAX_ALARM_OUT; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "nOutName[%d]=%s&", i, outs[i].name);
        snprintf(buf + strlen(buf), len - strlen(buf), "nOutType[%d]=%d&", i, outs[i].type);
        snprintf(buf + strlen(buf), len - strlen(buf), "nOutime[%d]=%d&", i, outs[i].duration_time);
    }

    //camer alarm io
    struct alarm_chn_out_name cameraOut[MAX_IPC_ALARM_OUT][MAX_CAMERA];
    struct alarm_chn_name cameraIn[MAX_IPC_ALARM_IN][MAX_CAMERA];
    for (i = 0; i < MAX_IPC_ALARM_NUM; i++) {
        if (i < MAX_IPC_ALARM_OUT) {
            memset(&cameraOut[i][0], 0x0, sizeof(struct alarm_chn_out_name) * MAX_CAMERA);
            read_alarm_chnOut_event_names(SQLITE_FILE_NAME, &cameraOut[i][0], MAX_CAMERA, i);
        }

        memset(&cameraIn[i][0], 0x0, sizeof(struct alarm_chn_name) * MAX_CAMERA);
        read_alarm_chnIn_event_names(SQLITE_FILE_NAME, &cameraIn[i][0], MAX_CAMERA, i);
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        for (j = 0; j < MAX_IPC_ALARM_NUM; j++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "cInName[%d][%d]=%s&", i, j, cameraIn[j][i].name);
            if (j < MAX_IPC_ALARM_OUT) {
                snprintf(buf + strlen(buf), len - strlen(buf), "cOutName[%d][%d]=%s&", i, j, cameraOut[j][i].name);
                snprintf(buf + strlen(buf), len - strlen(buf), "cOutDelay[%d][%d]=%d&", i, j, cameraOut[j][i].delay_time);
            }
        }
    }

    if (res->exceptionCnt >= 0) {
        snprintf(buf + strlen(buf), len - strlen(buf), "exceptionCnt=%d&", res->exceptionCnt);
        for (i = 0; i < res->exceptionCnt; ++i) {
            snprintf(buf + strlen(buf), len - strlen(buf), "exception[%d]=%s&", i, res->exceptions[i].pEvent);
            snprintf(buf + strlen(buf), len - strlen(buf), "information[%d]=%s&", i, res->exceptions[i].pInformation);
            time_to_string(res->exceptions[i].time, curTime);
            snprintf(buf + strlen(buf), len - strlen(buf), "time[%d]=%s&", i, curTime);
        }
    }
    struct EventStatusStr eventStatus;
    memset(&eventStatus, 0, sizeof(struct EventStatusStr));
    trans_event_status_str(&eventStatus, res);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "videoLoss=%s&", eventStatus.videoLoss);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "motion=%s&", eventStatus.motion);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "audioAlarm=%s&", eventStatus.audioAlarm);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaEntrance=%s&", eventStatus.vcaEntrance);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaExiting=%s&", eventStatus.vcaExiting);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaMotion=%s&", eventStatus.vcaMotion);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaTamper=%s&", eventStatus.vcaTamper);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaLinecross=%s&", eventStatus.vcaLinecross);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaLoiter=%s&", eventStatus.vcaLoiter);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaHuman=%s&", eventStatus.vcaHuman);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaObject=%s&", eventStatus.vcaObject);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaPcntIn=%s&", eventStatus.vcaPcntIn);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "vcaPcntOut=%s&", eventStatus.vcaPcntOut);
    for (i = 0; i < MAX_IPC_ALARM_IN; i++) {
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "ipcAlarmInStatus%d=%s&", i, eventStatus.ipcAlarmInStatus[i]);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "ipcAlarmInType%d=%s&", i, eventStatus.ipcAlarmInType[i]);
    }
    for (i = 0; i < MAX_IPC_ALARM_OUT; i++) {
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "ipcAlarmOutStatus%d=%s&", i, eventStatus.ipcAlarmOutStatus[i]);
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "ipcAlarmOutType%d=%s&", i, eventStatus.ipcAlarmOutType[i]);
    }
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "nvrAlarmInStatus=%s&", eventStatus.nvrAlarmInStatus);
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "nvrAlarmOutStatus=%s&", eventStatus.nvrAlarmOutStatus);

    *datalen = strlen(buf) + 1;
}

static void resp_status_get_iosensor_status(void *param, int size, char *buf, int len, int *datalen)
{
    //int sensor;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void resp_status_get_ioalarm_status(void *param, int size, char *buf, int len, int *datalen)
{
    //int alarm;
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

void resp_status_get_network_speed(void *param, int size, char *buf, int len, int *datalen)
{
    //struct resp_network_speed;
    int cnt = size / sizeof(struct resp_network_speed);
    int i = 0;
    struct resp_network_speed *res = (struct resp_network_speed *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.ifname=%s&", res[0].ifname);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.rx_speed=%d&", res[0].rx_speed);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.tx_speed=%d&", res[0].tx_speed);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.netif_id=%d&", res[0].netif_id);

    if (cnt >= 2) {
        for (i = 1; i < cnt; i++) {
            snprintf(buf + strlen(buf), len - strlen(buf), "r.ifname%d=%s&", i, res[i].ifname);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.rx_speed%d=%d&", i, res[i].rx_speed);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.tx_speed%d=%d&", i, res[i].tx_speed);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.netif_id%d=%d&", i, res[i].netif_id);
        }
    }

    *datalen = strlen(buf) + 1;
}


void resp_status_get_motion_event_status(void *param, int size, char *buf, int len, int *datalen)
{
    int i = 0;;
    char pMot[MAX_CAMERA + 1] = {0};
    long long motion = *(long long *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "motion=%lld&", motion);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (motion & ((long long)1 << i)) {
            pMot[i] = '1';
        } else {
            pMot[i] = '0';
        }
    }
    pMot[MAX_CAMERA] = '\0';
    snprintf(buf + strlen(buf), len - strlen(buf), "Motion=%s&", (char *)pMot);

    *datalen = strlen(buf) + 1;
}


static void req_status_get_all_ipc_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int index, cnt = 0;
    struct camera cam[MAX_CAMERA];

    memset(cam, 0, sizeof(struct camera)*MAX_CAMERA);
    read_cameras(SQLITE_FILE_NAME, cam, &cnt);

    for (index = 0; index < cnt; index++) {
        if (!cam[index].enable) {
            continue;
        }
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id[%d]=%d&", index, cam[index].id);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "type[%d]=%d&", index, cam[index].type);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable[%d]=%d&", index, cam[index].enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "covert_on[%d]=%d&", index,
                 cam[index].covert_on);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "brightness[%d]=%d&", index,
                 cam[index].brightness);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "contrast[%d]=%d&", index,
                 cam[index].contrast);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "saturation[%d]=%d&", index,
                 cam[index].saturation);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "username[%d]=%s&", index,
                 cam[index].username);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "password[%d]=%s&", index,
                 cam[index].password);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ip_addr[%d]=%s&", index,
                 cam[index].ip_addr);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "main_rtsp_port[%d]=%d&", index,
                 cam[index].main_rtsp_port);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "main_source_path[%d]=%s&", index,
                 cam[index].main_source_path);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_rtsp_enable[%d]=%d&", index,
                 cam[index].sub_rtsp_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_rtsp_port[%d]=%d&", index,
                 cam[index].sub_rtsp_port);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_source_path[%d]=%s&", index,
                 cam[index].sub_source_path);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "manage_port[%d]=%d&", index,
                 cam[index].manage_port);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camera_protocol[%d]=%d&", index,
                 cam[index].camera_protocol);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "transmit_protocol[%d]=%d&", index,
                 cam[index].transmit_protocol);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camera_protocol[%d]=%d&", index,
                 cam[index].camera_protocol);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "play_stream[%d]=%d&", index,
                 cam[index].play_stream);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "record_stream[%d]=%d&", index,
                 cam[index].record_stream);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sync_time[%d]=%d&", index,
                 cam[index].sync_time);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "codec[%d]=%d&", index, cam[index].codec);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "main_rtspurl[%d]=%s&", index,
                 cam[index].main_rtspurl);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_rtspurl[%d]=%s&", index,
                 cam[index].sub_rtspurl);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "anr[%d]=%d&", index, cam[index].anr);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "poe_channel[%d]=%d&", index,
                 cam[index].poe_channel);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "httpsPort[%d]=%d&", index,
                 cam[index].https_port);
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_status_get_one_ipc_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid;
    char sValue[256] = {0};
    struct camera cam = {0};

    if (!sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    read_camera(SQLITE_FILE_NAME, &cam, chnid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id=%d&", cam.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "type=%d&", cam.type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", cam.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "covert_on=%d&", cam.covert_on);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "brightness=%d&", cam.brightness);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "contrast=%d&", cam.contrast);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "saturation=%d&", cam.saturation);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "username=%s&", cam.username);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "password=%s&", cam.password);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ip_addr=%s&", cam.ip_addr);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "main_rtsp_port=%d&", cam.main_rtsp_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "main_source_path=%s&",
             cam.main_source_path);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_rtsp_enable=%d&", cam.sub_rtsp_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_rtsp_port=%d&", cam.sub_rtsp_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_source_path=%s&", cam.sub_source_path);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "manage_port=%d&", cam.manage_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camera_protocol=%d&", cam.camera_protocol);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "transmit_protocol=%d&",
             cam.transmit_protocol);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camera_protocol=%d&", cam.camera_protocol);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "play_stream=%d&", cam.play_stream);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "record_stream=%d&", cam.record_stream);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sync_time=%d&", cam.sync_time);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "codec=%d&", cam.codec);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "main_rtspurl=%s&", cam.main_rtspurl);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sub_rtspurl=%s&", cam.sub_rtspurl);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "https_port=%d&", cam.https_port);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_status_get_common_params(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char key[MAX_LEN_32] = {0};
    char tmp[MAX_LEN_32] = {0};
    int value = 0;
    int cnt = 0;
    int i = 0;

    if (sdkp2p_get_section_info(buf, "count=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    cnt = atoi(sValue);
    if (cnt <= 0) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    for (i = 0; i < cnt; i++) {
        snprintf(tmp, sizeof(tmp), "key%d=", i);
        if (sdkp2p_get_section_info(buf, tmp, key, sizeof(key))) {
            continue;
        }

        if (!strcmp(key, "day")) {
            snprintf(tmp, sizeof(tmp), "value%d=", i);
            if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                continue;
            }
            get_url_decode(sValue);
            value = ms_get_current_date_cnt_ex(sValue);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s=%d&", key, value);
        } else if (!strcmp(key, "hour")) {
            snprintf(tmp, sizeof(tmp), "value%d=", i);
            if (sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                continue;
            }
            get_url_decode(sValue);
            value = ms_get_current_hours_cnt_ex(sValue);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "%s=%d&", key, value);
        }
    }

    *datalen = strlen(conf->resp) + 1;
}

static void resp_status_get_nvr_audio_status(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "audio_busy=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void resp_status_get_poe_port_status(void *param, int size, char *buf, int len, int *datalen)
{
    int num = size / sizeof(struct resq_poe_state);

    get_poe_port_status_json(param, num, buf, len);
    *datalen = strlen(buf) + 1;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_ALL_IPC_INFO, req_status_get_all_ipc_info, -1, NULL},//get.status.allipc
    {REQUEST_FLAG_GET_ONE_IPC_INFO, req_status_get_one_ipc_info, -1, NULL},//get.status.ipcinfo
    {REQUEST_FLAG_GET_IPC_STATUS, NULL, RESPONSE_FLAG_GET_IPC_STATUS, resp_status_get_ipc_status},//get.status.ipcstatus
    {REQUEST_FLAG_GET_EVENT_STATUS, NULL, RESPONSE_FLAG_GET_EVENT_STATUS, resp_status_get_event_status},//get.status.event
    {REQUEST_FLAG_GET_IOALARM_STATUS, NULL, RESPONSE_FLAG_GET_IOALARM_STATUS, resp_status_get_ioalarm_status},//get.status.ioalarm
    {REQUEST_FLAG_GET_IOSENSOR_STATUS, NULL, RESPONSE_FLAG_GET_IOSENSOR_STATUS, resp_status_get_iosensor_status},//get.status.iosensor
    {REQUEST_FLAG_GET_NETWORK_BANDWIDTH, NULL, RESPONSE_FLAG_GET_NETWORK_BANDWIDTH, resp_status_get_network_bandwidth},//get.status.netbandwidth
    {REQUEST_FLAG_GET_NETWORK_INFO, NULL, RESPONSE_FLAG_GET_NETWORK_INFO, resp_status_get_network_info},//get.status.netinfo
    {REQUEST_FLAG_GET_NETWORK_SPEED, NULL, RESPONSE_FLAG_GET_NETWORK_SPEED, resp_status_get_network_speed},//get.status.netspeed
    {REQUEST_FLAG_GET_MOTION_EVENT_STATUS, NULL, RESPONSE_FLAG_GET_MOTION_EVENT_STATUS, resp_status_get_motion_event_status},//get.status.motion

    {REQUEST_FLAG_GET_CHANNELS_STATE, NULL, RESPONSE_FLAG_GET_CHANNELS_STATE, resp_status_get_channel_status},
    {REQUEST_FLAG_P2P_GET_COMMON_PARAMS, req_status_get_common_params, RESPONSE_FLAG_P2P_GET_COMMON_PARAMS, NULL},//get.status.common_params
    {REQUEST_FLAG_CHECK_AUDIOTALK, NULL, RESPONSE_FLAG_CHECK_AUDIOTALK, resp_status_get_nvr_audio_status},//get.status.nvr_audio_status
    {REQUEST_FLAG_GET_POE_STATE, NULL, RESPONSE_FLAG_GET_POE_STATE, resp_status_get_poe_port_status},//get.status.poe.port.status
    // request register array end
};


void p2p_status_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_status_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

