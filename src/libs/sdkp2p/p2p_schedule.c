#include "sdk_util.h"


static void sche_on_set_record(struct req_conf_str *conf, const char *pScheduleParam)
{
    int iday, item, chnid = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};

    struct record record;
    memset(&record, 0, sizeof(struct record));

    if (!sdkp2p_get_section_info(pScheduleParam, "ch=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    record.id = chnid;

    struct record_schedule schedule_record;
    memset(&schedule_record, 0, sizeof(struct record_schedule));

    struct record m_srecord = {0};
    read_record(SQLITE_FILE_NAME, &m_srecord, record.id);
    m_srecord.mode = 2;
    write_record(SQLITE_FILE_NAME, &m_srecord);

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RECORDMODE, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);

    read_record_schedule(SQLITE_FILE_NAME, &schedule_record, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(tmp, sizeof(tmp), "wholeday_enable[%d]=", iday);
        if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
            schedule_record.schedule_day[iday].wholeday_enable = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "wholeday_action_type[%d]=", iday);
        if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
            schedule_record.schedule_day[iday].wholeday_action_type = atoi(sValue);
        }
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; item++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule_record.schedule_day[iday].schedule_item[item].start_time,
                         sizeof(schedule_record.schedule_day[iday].schedule_item[item].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule_record.schedule_day[iday].schedule_item[item].end_time,
                         sizeof(schedule_record.schedule_day[iday].schedule_item[item].end_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                schedule_record.schedule_day[iday].schedule_item[item].action_type = atoi(sValue);
            }
        }
    }
    write_record_schedule(SQLITE_FILE_NAME, &schedule_record, chnid);
}

static void sche_on_set_move_audible(struct req_conf_str *conf, const char *pScheduleParam)
{
    int iday, item, chnid = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};

    struct motion move_audible;
    memset(&move_audible, 0, sizeof(struct motion));
    struct motion_schedule schedule_audible;
    memset(&schedule_audible, 0, sizeof(struct motion_schedule));

    if (!sdkp2p_get_section_info(pScheduleParam, "ch=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    read_motion(SQLITE_FILE_NAME, &move_audible, chnid);
    move_audible.id = chnid;
    if (!sdkp2p_get_section_info(pScheduleParam, "audible_enable=", sValue, sizeof(sValue))) {
        move_audible.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pScheduleParam, "audible_interval=", sValue, sizeof(sValue))) {
        move_audible.buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pScheduleParam, "audioId=", sValue, sizeof(sValue))) {
        move_audible.tri_audio_id = atoi(sValue);
    }
    write_motion(SQLITE_FILE_NAME, &move_audible);

    read_motion_audible_schedule(SQLITE_FILE_NAME, &schedule_audible, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(tmp, sizeof(tmp), "start_time_audible[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule_audible.schedule_day[iday].schedule_item[item].start_time,
                         sizeof(schedule_audible.schedule_day[iday].schedule_item[item].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time_audible[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule_audible.schedule_day[iday].schedule_item[item].end_time,
                         sizeof(schedule_audible.schedule_day[iday].schedule_item[item].end_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type_audible[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                schedule_audible.schedule_day[iday].schedule_item[item].action_type = atoi(sValue);
            }
        }
    }
    write_motion_audible_schedule(SQLITE_FILE_NAME, &schedule_audible, chnid);
}

static void sche_on_set_move_email(struct req_conf_str *conf, const char *pScheduleParam)
{
    int iday, item, chnid = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct motion move;
    memset(&move, 0, sizeof(struct motion));
    struct motion_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));

    if (!sdkp2p_get_section_info(pScheduleParam, "ch=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }

    read_motion(SQLITE_FILE_NAME, &move, chnid);
    move.id = chnid;
    if (!sdkp2p_get_section_info(pScheduleParam, "email_enable=", sValue, sizeof(sValue))) {
        move.email_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pScheduleParam, "email_interval=", sValue, sizeof(sValue))) {
        move.email_buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pScheduleParam, "email_pic_enable=", sValue, sizeof(sValue))) {
        move.email_pic_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(pScheduleParam, "tri_channels_pic=", sValue, sizeof(sValue))) {
        snprintf(move.tri_channels_pic, sizeof(move.tri_channels_pic), "%s", sValue);
    }
    write_motion(SQLITE_FILE_NAME, &move);

    read_motion_email_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(tmp, sizeof(tmp), "start_time_email[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[iday].schedule_item[item].start_time,
                         sizeof(schedule.schedule_day[iday].schedule_item[item].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time_email[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[iday].schedule_item[item].end_time,
                         sizeof(schedule.schedule_day[iday].schedule_item[item].end_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type_email[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(pScheduleParam, tmp, sValue, sizeof(sValue))) {
                schedule.schedule_day[iday].schedule_item[item].action_type = atoi(sValue);
            }
        }
    }
    write_motion_email_schedule(SQLITE_FILE_NAME, &schedule, chnid);
}

static void req_sche_set_alarmpush_id(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_alarm_push_id *res = (struct req_alarm_push_id *)param;
    char sValue[256] = {0};
    struct db_user user;

    memset(res, 0x0, sizeof(struct req_alarm_push_id));
    if (!sdkp2p_get_section_info(buf, "r.id=", sValue, sizeof(sValue))) {
        snprintf(res->id, sizeof(res->id), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.sn=", sValue, sizeof(sValue))) {
        snprintf(res->sn, sizeof(res->sn), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->enable));
    }
    if (!sdkp2p_get_section_info(buf, "r.from=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->from));
    }
    if(!sdkp2p_get_section_info(buf, "r.oldId=", sValue, sizeof(sValue))){
        snprintf(res->oldId, sizeof(res->oldId), "%s", sValue);
    }
    if(!sdkp2p_get_section_info(buf, "r.del=", sValue, sizeof(sValue))){
        sscanf(sValue, "%d", &(res->del));
    }
    if(!sdkp2p_get_section_info(buf, "r.region=", sValue, sizeof(sValue))){
        snprintf(res->region, sizeof(res->region), "%s", sValue);
    } else {
        snprintf(res->region, sizeof(res->region), "%s", "GB");
    }
    if (conf->user[0]) {
        memset(&user, 0, sizeof(struct db_user));
        read_user_by_name(SQLITE_FILE_NAME, &user, conf->user);
        res->userId = user.id;
    }

    *datalen = sizeof(struct req_alarm_push_id);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    //*(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_set_alarmpush_interval(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_android_account pacc;

    memset(&pacc, 0, sizeof(struct req_android_account));
    if (!sdkp2p_get_section_info(buf, "key=", sValue, sizeof(sValue))) {
        snprintf(pacc.key, sizeof(pacc.key), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "passwd=", sValue, sizeof(sValue))) {
        snprintf(pacc.passwd, sizeof(pacc.passwd), "%s", sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &pacc, sizeof(struct req_android_account), SDKP2P_NOT_CALLBACK);
    //*(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_add_android_push_account(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_alarm_push_interval *res = (struct req_alarm_push_interval *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.id=", sValue, sizeof(sValue))) {
        snprintf(res->id, sizeof(res->id), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.interval=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->interval));
    }

    *datalen = sizeof(struct req_alarm_push_interval);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_del_android_push_account(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_android_account pacc;

    memset(&pacc, 0, sizeof(struct req_android_account));
    if (!sdkp2p_get_section_info(buf, "key=", sValue, sizeof(sValue))) {
        snprintf(pacc.key, sizeof(pacc.key), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "passwd=", sValue, sizeof(sValue))) {
        snprintf(pacc.passwd, sizeof(pacc.passwd), "%s", sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &pacc, sizeof(struct req_android_account), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_motion_effective_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = 0;
    struct smart_event_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chanid = atoi(sValue);
    memset(&schedule, 0, sizeof(struct smart_event_schedule));
    read_motion_effective_schedule(SQLITE_FILE_NAME, &schedule, chanid);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_motion_effective_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};
    long long batch_flag = 0;
    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct smart_event_schedule));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_motion_effective_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    copy_motion_effective_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION_BATCH, &batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_vca_effective_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chnid = -1, event = -1;

    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    event = atoi(sValue);

    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_vca_effective_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1, event = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};

    REQ_UPDATE_CHN req = {0};
    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    event = atoi(sValue);

    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "copyChn=", sValue, sizeof(sValue))) {
        req.chnMask = get_channel_mask(sValue);
    }

    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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
    write_smart_event_effective_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    copy_smart_event_effective_schedules(SQLITE_FILE_NAME, &schedule, req.chnMask, event);

    ms_set_bit(&req.chnMask, chnid, 1);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_SMART_EVENT, &req, sizeof(req), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_vca_audible_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chnid = -1, event = -1;

    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    event = atoi(sValue);

    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_smart_event_audible_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);


    struct smart_event smart;
    memset(&smart, 0x0, sizeof(struct smart_event));
    read_smart_event(SQLITE_FILE_NAME, &smart, chnid, event);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", smart.buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "audioId=%d&", smart.tri_audio_id);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_vca_audible_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1, event = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};
    REQ_UPDATE_CHN req = {0};

    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));
    struct smart_event smart;
    memset(&smart, 0x0, sizeof(struct smart_event));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    event = atoi(sValue);

    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "copyChn=", sValue, sizeof(sValue))) {
        req.chnMask = get_channel_mask(sValue);
    }

    read_smart_event(SQLITE_FILE_NAME, &smart, chnid, event);
    smart.id = chnid;
    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        smart.buzzer_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "audioId=", sValue, sizeof(sValue))) {
        smart.tri_audio_id = atoi(sValue);
    }
    write_smart_event(SQLITE_FILE_NAME, &smart, event);
    copy_smart_events(SQLITE_FILE_NAME, &smart, req.chnMask, event);

    read_smart_event_audible_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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
    write_smart_event_audible_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    copy_smart_event_audible_schedules(SQLITE_FILE_NAME, &schedule, req.chnMask, event);
    ms_set_bit(&req.chnMask, chnid, 1);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_SMART_EVENT, &req, sizeof(req), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_vca_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chnid = -1, event = -1;

    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    event = atoi(sValue);

    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_smart_event_mail_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);

    struct smart_event smart;
    memset(&smart, 0x0, sizeof(struct smart_event));
    read_smart_event(SQLITE_FILE_NAME, &smart, chnid, event);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", smart.email_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_pic_enable=%d&",
             smart.email_pic_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_pic=%s&",
             smart.tri_channels_pic);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_vca_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1, event = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};
    REQ_UPDATE_CHN req = {0};

    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));
    struct smart_event smart;
    memset(&smart, 0x0, sizeof(struct smart_event));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    event = atoi(sValue);

    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "copyChn=", sValue, sizeof(sValue))) {
        req.chnMask = get_channel_mask(sValue);
    }

    read_smart_event(SQLITE_FILE_NAME, &smart, chnid, event);
    smart.id = chnid;
    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        smart.email_interval = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "email_pic_enable=", sValue, sizeof(sValue))) {
        smart.email_pic_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_channels_pic=", sValue, sizeof(sValue))) {
        snprintf(smart.tri_channels_pic, sizeof(smart.tri_channels_pic), sValue);
    }
    write_smart_event(SQLITE_FILE_NAME, &smart, event);
    copy_smart_events(SQLITE_FILE_NAME, &smart, req.chnMask, event);

    read_smart_event_mail_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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
    write_smart_event_mail_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    copy_smart_event_mail_schedules(SQLITE_FILE_NAME, &schedule, req.chnMask, event);
    ms_set_bit(&req.chnMask, chnid, 1);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_SMART_EVENT, &req, sizeof(req), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_vca_others(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, event = -1;
    char sValue[256] = {0};

    struct smart_event smart;
    memset(&smart, 0x0, sizeof(struct smart_event));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    event = atoi(sValue);

    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_smart_event(SQLITE_FILE_NAME, &smart, chnid, event);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChannels=%s&", smart.tri_channels_ex);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%u&", smart.tri_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarmout1=%s&", smart.tri_chnout1_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChnAlarmout2=%s&", smart.tri_chnout2_alarms);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_vca_others(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, i = 0, event = -1;
    char sValue[368] = {0};

    struct smart_event smart;
    REQ_UPDATE_CHN info;
    memset(&smart, 0x0, sizeof(struct smart_event));
    memset(&info, 0, sizeof(REQ_UPDATE_CHN));

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);

    if (sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    event = atoi(sValue);
    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_smart_event(SQLITE_FILE_NAME, &smart, chnid, event);

    if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
        smart.tri_alarms = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarmout1=", sValue, sizeof(sValue))) {
        snprintf(smart.tri_chnout1_alarms, sizeof(smart.tri_chnout1_alarms), "%s", sValue);
    }
    
    if (!sdkp2p_get_section_info(buf, "triChnAlarmout2=", sValue, sizeof(sValue))) {
        snprintf(smart.tri_chnout2_alarms, sizeof(smart.tri_chnout2_alarms), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "copyChn=", sValue, sizeof(sValue))) {
        smart.copyMask = get_channel_mask(sValue);
        info.chnMask = smart.copyMask;
    }

    copy_smart_events(SQLITE_FILE_NAME, &smart, smart.copyMask, event);

    // no need to copy to other chns
    if (!sdkp2p_get_section_info(buf, "triChannels=", sValue, sizeof(sValue))) {
        snprintf(smart.tri_channels_ex, sizeof(smart.tri_channels_ex), "%s", sValue);
    }
    for (i = 0; i < strlen(smart.tri_channels_ex); i++) {
        if (smart.tri_channels_ex[i] == '1') {
            smart.tri_channels |= (1 << i);
        }
    }

    write_smart_event(SQLITE_FILE_NAME, &smart, event);

    ms_set_bit(&info.chnMask, chnid, 1);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_SMART_EVENT, &info, sizeof(info), SDKP2P_NEED_CALLBACK);
    
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_privacy_mask(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
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

static void resp_sche_get_privacy_mask(void *param, int size, char *buf, int len, int *datalen)
{
    int i;
    struct resp_privacy_mask *res = (struct resp_privacy_mask *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.id=%d&", res->id);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.num=%d&", res->num);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.cgiSdkFlag=%d&", res->cgiSdkFlag);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.maxPtzZoomRatio=%d&", res->maxPtzZoomRatio);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.areaMaskEnable=%d&", res->areaMaskEnable);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", res->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "mosaicSupport=%d&", res->system_mosaic_support);
    snprintf(buf + strlen(buf), len - strlen(buf), "mosaicNum=%d&", res->imaging_settings_mosaic_num);
    snprintf(buf + strlen(buf), len - strlen(buf), "colorSupport=%d&", res->colorSupport);
    snprintf(buf + strlen(buf), len - strlen(buf), "maxMaskSum=%d&", res->maxMaskSum);
    for (i = 0; i < res->maxMaskSum; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].area_id=%d&", i, res->area[i].area_id);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].enable=%d&", i, res->area[i].enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].start_x=%d&", i, res->area[i].start_x);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].start_y=%d&", i, res->area[i].start_y);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].area_width=%d&", i, res->area[i].area_width);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].area_height=%d&", i, res->area[i].area_height);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].width=%d&", i, 480);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].height=%d&", i, 270);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].fill_color=%d&", i, res->area[i].fill_color);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].name=%s&", i, res->area[i].name);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.area[%d].ratio=%d\r\n", i, res->area[i].ratio);
    }

    *datalen = strlen(buf) + 1;
}

static void req_sche_set_privacy_mask(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct resp_privacy_mask mask_info;

    memset(&mask_info, 0x0, sizeof(struct resp_privacy_mask));
    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    mask_info.id = atoi(sValue);

    if (!sdkp2p_get_section_info(buf, "num=", sValue, sizeof(sValue))) {
        mask_info.num = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "cgiSdkFlag=", sValue, sizeof(sValue))) {
        mask_info.cgiSdkFlag = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "maxPtzZoomRatio=", sValue, sizeof(sValue))) {
        mask_info.maxPtzZoomRatio = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "delMask=", sValue, sizeof(sValue))) {
        mask_info.delMask = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "mosaicSupport=", sValue, sizeof(sValue))) {
        mask_info.system_mosaic_support = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "mosaicNum=", sValue, sizeof(sValue))) {
        mask_info.imaging_settings_mosaic_num = atoi(sValue);
    }

    for (i = 0; i < MAX_MASK_AREA_NUM; i++) {
        snprintf(tmp, sizeof(tmp), "area_id[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].area_id = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "enable[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].enable = atoi(sValue);
            if (mask_info.area[i].enable) {
                mask_info.areaMaskEnable |= 1 << mask_info.area[i].area_id;
            }
        }
        snprintf(tmp, sizeof(tmp), "start_x[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].start_x = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "start_y[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].start_y = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "area_width[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].area_width = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "area_height[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].area_height = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "fill_color[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].fill_color = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "name[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(mask_info.area[i].name, sizeof(mask_info.area[i].name), "%s", sValue);
        }

        snprintf(tmp, sizeof(tmp), "ratio[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            mask_info.area[i].ratio = atoi(sValue);
        }
    }

    mask_info.areaMaskEnable = 0xffffffff;
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        mask_info.enable = atoi(sValue);
    } else {
        mask_info.enable = mask_info.areaMaskEnable;
    }

    sdkp2p_send_msg(conf, conf->req, &mask_info, sizeof(struct resp_privacy_mask), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_record_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct record_schedule schedule;

    memset(&schedule, 0, sizeof(struct record_schedule));
    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    read_record_schedule(SQLITE_FILE_NAME, &schedule, chanid);

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
}

static void req_sche_set_record_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int logs = 0;
    long long batch_flag = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct record_schedule schedule;

    memset(&schedule, 0, sizeof(struct record_schedule));
    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    read_record_schedule(SQLITE_FILE_NAME, &schedule, chnid);
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
                if (iday == 0) {
                    logs = 1;
                }
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

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    copy_record_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RECORDSCHED_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    if (logs == 1) {
        sdkp2p_common_write_log_batch(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_RECORD_SCHED, batch_flag);
    }
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_motion_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct motion_schedule schedule;
    struct motion move = {0};

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);

    memset(&schedule, 0, sizeof(struct motion_schedule));
    memset(&move, 0, sizeof(struct motion));
    read_motion_schedule(SQLITE_FILE_NAME, &schedule, chanid);
    read_motion(SQLITE_FILE_NAME, &move, chanid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camId=%d&", move.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", move.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sensitivity=%d&", move.sensitivity);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChannels=%s&", move.tri_channels_ex);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%u&", move.tri_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", move.buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "audioId=%d&", move.tri_audio_id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enableMS=%d&", move.ipc_sched_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "motionTable=%s&",
             (const char *)move.motion_table);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_enable=%d&", move.email_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&",
             move.email_buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_pic_enable=%d&",
             move.email_pic_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_pic=%s&",
             move.tri_channels_pic);

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
}

static void req_sche_set_motion_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1, i = 0, j = 0;
    char sValue[368] = {0};
    char tmp[64] = {0};
    int cnt = 0;
    struct motion motions[MAX_CAMERA];
    struct motion_schedule schedule;
    struct motion move = {0};

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    memset(motions, 0, sizeof(motions[0])*MAX_CAMERA);
    memset(&schedule, 0, sizeof(struct motion_schedule));
    read_motions(SQLITE_FILE_NAME, motions, &cnt);
    chnid = atoi(sValue);
    move.id = chnid;
    memcpy(&move, &motions[move.id], sizeof(struct motion));

    if (!sdkp2p_get_section_info(buf, "sensitivity=", sValue, sizeof(sValue))) {
        move.sensitivity = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enableMS=", sValue, sizeof(sValue))) {
        move.ipc_sched_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
        move.tri_alarms = atoi(sValue);
    }
    memset(move.tri_channels_ex, 0, sizeof(move.tri_channels_ex));
    move.tri_channels = 0;
    if (!sdkp2p_get_section_info(buf, "triChannels=", sValue, sizeof(sValue))) {
        snprintf(move.tri_channels_ex, sizeof(move.tri_channels_ex), "%s", sValue);
    }
    for (i = 0; i < strlen(move.tri_channels_ex); i++) {
        if (move.tri_channels_ex[i] == '1') {
            move.tri_channels |= (1 << i);
        }
    }
    if (!sdkp2p_get_section_info(buf, "motionTable=", sValue, sizeof(sValue))) {
        snprintf((char *)move.motion_table, sizeof(move.motion_table), "%s", sValue);
    }
    write_motion(SQLITE_FILE_NAME, &move);

    sche_on_set_record(conf, buf);
    sche_on_set_move_audible(conf, buf);
    sche_on_set_move_email(conf, buf);

    read_motion_schedule(SQLITE_FILE_NAME, &schedule, chnid);
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
    write_motion_schedule(SQLITE_FILE_NAME, &schedule, chnid);

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_RECORDSCHED, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);

    struct req_set_motionmap motionmap;
    memset(&motionmap, 0, sizeof(struct req_set_motionmap));
    motionmap.enable = move.ipc_sched_enable;
    motionmap.chanid = move.id;
    motionmap.sensitivity = move.sensitivity;
    snprintf(motionmap.mapbuf, sizeof(motionmap.mapbuf), "%s", move.motion_table);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPCMTMAP, &motionmap, sizeof(struct req_set_motionmap), SDKP2P_NOT_CALLBACK);

    if (move.ipc_sched_enable == 1) {
        struct req_set_motionsce tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.chanid = move.id;
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
        struct req_enable_motion enableMotion = {0};
        enableMotion.ch = move.id;
        enableMotion.enable = 1;
        sdkp2p_send_msg(conf, REQUEST_FLAG_OVF_ENABLE_MOT, &enableMotion, sizeof(struct req_enable_motion),
                        SDKP2P_NOT_CALLBACK);
        sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPCMTSCE, &tmp, sizeof(struct req_set_motionsce), SDKP2P_NOT_CALLBACK);
    } else {
        struct req_enable_motion enableMotion = {0};
        enableMotion.ch = move.id;
        enableMotion.enable = 0;
        sdkp2p_send_msg(conf, REQUEST_FLAG_OVF_ENABLE_MOT, (void *)&enableMotion, sizeof(struct req_enable_motion),
                        SDKP2P_NOT_CALLBACK);
    }
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_MOTION, chnid + 1, 0, NULL, 0);
}

static void req_sche_get_videoloss_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct video_loss_schedule schedule;
    struct video_loss videilost = {0};

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);

    memset(&schedule, 0, sizeof(struct video_loss_schedule));
    memset(&videilost, 0x0, sizeof(struct video_loss));
    read_video_loss_schedule(SQLITE_FILE_NAME, &schedule, chanid);
    read_video_lost(SQLITE_FILE_NAME, &videilost, chanid);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camId=%d&", videilost.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", videilost.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%u&", videilost.tri_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", videilost.buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "audioId=%d&", videilost.tri_audio_id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_enable=%d&", videilost.email_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&",
             videilost.email_buzzer_interval);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_enable[%d]=%d&", iday,
                 schedule.schedule_day[iday].wholeday_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_action_type[%d]=%d&", iday,
                 schedule.schedule_day[iday].wholeday_action_type);
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_videoloss_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct video_loss_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    memset(&schedule, 0, sizeof(struct video_loss_schedule));
    read_video_loss_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(tmp, sizeof(tmp), "wholeday_enable[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            schedule.schedule_day[iday].wholeday_enable = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "wholeday_action_type[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            schedule.schedule_day[iday].wholeday_action_type = atoi(sValue);
        }
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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
    write_video_loss_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_VIDEOLOSS, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_alarmin_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct alarm_in_schedule schedule;
    struct alarm_in alarmin = {0};

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);

    memset(&schedule, 0, sizeof(struct alarm_in_schedule));
    read_alarm_in_schedule(SQLITE_FILE_NAME, &schedule, chanid);
    memset(&alarmin, 0, sizeof(struct alarm_in));
    read_alarm_in(SQLITE_FILE_NAME, &alarmin, chanid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "camId=%d&", alarmin.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", alarmin.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%u&", alarmin.tri_alarms);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChannels=%s&", alarmin.tri_channels_ex);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triSnapshotChannels=%s&",
             alarmin.tri_channels_snapshot);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "interval=%d&", alarmin.buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "audioId=%d&", alarmin.tri_audio_id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_enable=%d&", alarmin.email_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_interval=%d&",
             alarmin.email_buzzer_interval);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "email_pic_enable=%d&",
             alarmin.email_pic_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_channels_pic=%s&",
             alarmin.tri_channels_pic);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_channel=%d&",
             alarmin.acto_ptz_channel);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_preset_enable=%d&",
             alarmin.acto_ptz_preset_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_preset=%d&",
             alarmin.acto_ptz_preset);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_patrol_enable=%d&",
             alarmin.acto_ptz_patrol_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_patrol=%d&",
             alarmin.acto_ptz_patrol);

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
}

static void req_sche_set_alarmin_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int logs = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct alarm_in_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    sche_on_set_record(conf, buf);
    memset(&schedule, 0, sizeof(struct alarm_in_schedule));
    read_alarm_in_schedule(SQLITE_FILE_NAME, &schedule, chnid);
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
                if (iday == 0) {
                    logs = 1;
                }
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
    write_alarm_in_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMIN, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);

    if (logs == 1) {
        sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMINPUT, chnid + 1, 0, NULL, 0);
    }
}

static void req_sche_get_alarmout_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct alarm_out_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&schedule, 0, sizeof(struct alarm_out_schedule));
    read_alarm_out_schedule(SQLITE_FILE_NAME, &schedule, chanid);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_enable[%d]=%d&", iday,
                 schedule.schedule_day[iday].wholeday_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "wholeday_action_type[%d]=%d&", iday,
                 schedule.schedule_day[iday].wholeday_action_type);
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_alarmout_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int logs = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct alarm_out_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    memset(&schedule, 0, sizeof(struct alarm_out_schedule));
    read_alarm_out_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        snprintf(tmp, sizeof(tmp), "wholeday_enable[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            schedule.schedule_day[iday].wholeday_enable = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "wholeday_action_type[%d]=", iday);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            schedule.schedule_day[iday].wholeday_action_type = atoi(sValue);
        }
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                if (iday == 0) {
                    logs = 1;
                }
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
    write_alarm_out_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMOUT, &chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);

    if (logs == 1) {
        sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMOUTPUT, chnid + 1, 0, NULL, 0);
    }
}

static void req_sche_get_motion_audible_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct motion_schedule moveSchedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&moveSchedule, 0, sizeof(struct motion_schedule));
    read_motion_audible_schedule(SQLITE_FILE_NAME, &moveSchedule, chanid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     moveSchedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     moveSchedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     moveSchedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_motion_audible_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int enable = -1;
    int num = 0;
    int interval = -1;
    long long batch_flag = 0;
    int i = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct motion motions[MAX_CAMERA];
    struct motion_schedule schedule;
    int audioId = -1;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    memset(&schedule, 0, sizeof(struct motion_schedule));
    memset(motions, 0x0, sizeof(struct motion) * MAX_CAMERA);
    read_motions(SQLITE_FILE_NAME, motions, &num);

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        enable = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "audioId=", sValue, sizeof(sValue))) {
        audioId = atoi(sValue);
    }

    read_motion_audible_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    for (i = 0; i < MAX_CAMERA; i++) {
        if (batch_flag >> i & 0x01) {
            if (enable != -1) {
                motions[i].enable = enable;
            }
            if (interval != -1) {
                motions[i].buzzer_interval = interval;
            }
            if (audioId != -1) {
                motions[i].tri_audio_id = audioId;
            }
        }
    }
    write_motions(SQLITE_FILE_NAME, motions, MAX_CAMERA, batch_flag);
    copy_motion_audible_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_motion_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct motion_schedule moveSchedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&moveSchedule, 0, sizeof(struct motion_schedule));
    read_motion_email_schedule(SQLITE_FILE_NAME, &moveSchedule, chanid);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     moveSchedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     moveSchedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     moveSchedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_motion_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int enable = -1, interval = -1;
    char sValue[256] = {0};
    long long batch_flag = 0;
    int i = 0;
    int num = 0;
    char tmp[64] = {0};
    struct motion motions[MAX_CAMERA];
    struct motion_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    memset(&motions, 0, sizeof(struct motion) * MAX_CAMERA);
    memset(&schedule, 0, sizeof(struct motion_schedule));
    read_motions(SQLITE_FILE_NAME, motions, &num);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        interval = atoi(sValue);
    }

    read_motion_email_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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


    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    for (i = 0; i < MAX_CAMERA; i++) {
        if (batch_flag >> i & 0x01) {
            if (enable != -1) {
                motions[i].email_enable = enable;
            }
            if (interval != -1) {
                motions[i].email_buzzer_interval = interval;
            }
        }
    }
    write_motions(SQLITE_FILE_NAME, motions, MAX_CAMERA, batch_flag);
    copy_motion_email_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_motion_others_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chanid = -1;
    struct motion move;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&move, 0, sizeof(struct motion));
    read_motion(SQLITE_FILE_NAME, &move, chanid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triChannels=%u&", move.tri_channels);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "triAlarms=%u&", move.tri_alarms);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_motion_others_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, type = -1, i = 0;
    char sValue[256] = {0};
    struct motion move;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }

    memset(&move, 0, sizeof(struct motion));
    read_motion(SQLITE_FILE_NAME, &move, chnid);
    move.id = chnid;
    if (type == 1) {
        memset(move.tri_channels_ex, 0, sizeof(move.tri_channels_ex));
        move.tri_channels = 0;
        if (!sdkp2p_get_section_info(buf, "triChannels=", sValue, sizeof(sValue))) {
            snprintf(move.tri_channels_ex, sizeof(move.tri_channels_ex), "%s", sValue);
        }
        for (i = 0; i < strlen(move.tri_channels_ex); i++) {
            if (move.tri_channels_ex[i] == '1') {
                move.tri_channels |= (1 << i);
            }
        }
    } else if (type == 2) {
        if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
            move.tri_alarms = atoi(sValue);
        }
    }
    write_motion(SQLITE_FILE_NAME, &move);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION, (void *)&chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_videoloss_audible_sche(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct video_loss_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&schedule, 0, sizeof(struct video_loss_schedule));
    read_videoloss_audible_schedule(SQLITE_FILE_NAME, &schedule, chanid);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_videoloss_audible_sche(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int enable = -1, interval = -1;
    long long batch_flag = 0;
    int i = 0, num = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    int audioId = -1;

    struct video_loss videolosss[MAX_CAMERA];
    struct video_loss_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    memset(videolosss, 0, sizeof(struct video_loss) * MAX_CAMERA);
    memset(&schedule, 0, sizeof(struct video_loss_schedule));
    read_video_losts(SQLITE_FILE_NAME, videolosss, &num);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "audioId=", sValue, sizeof(sValue))) {
        audioId = atoi(sValue);
    }

    read_videoloss_audible_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    for (i = 0; i < MAX_CAMERA; i++) {
        if (batch_flag >> i & 0x01) {
            if (enable != -1) {
                videolosss[i].enable = enable;
            }
            if (interval != -1) {
                videolosss[i].buzzer_interval = interval;
            }
            if (audioId != -1) {
                videolosss[i].tri_audio_id = audioId;
            }
        }
    }

    write_video_losts(SQLITE_FILE_NAME, videolosss, MAX_CAMERA, batch_flag);
    copy_videoloss_audible_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_VIDEOLOSS_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_videoloss_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct video_loss_schedule email_sche;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&email_sche, 0, sizeof(struct video_loss_schedule));
    read_videoloss_email_schedule(SQLITE_FILE_NAME, &email_sche, chanid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     email_sche.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     email_sche.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     email_sche.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_videoloss_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int enable = -1, interval = -1;
    long long batch_flag = 0;
    int i = 0, num = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct video_loss videolosss[MAX_CAMERA];
    struct video_loss_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    memset(&videolosss, 0, sizeof(struct video_loss) * MAX_CAMERA);
    memset(&schedule, 0, sizeof(struct video_loss_schedule));
    read_video_losts(SQLITE_FILE_NAME, videolosss, &num);

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        interval = atoi(sValue);
    }


    read_videoloss_email_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    for (i = 0; i < MAX_CAMERA; i++) {
        if (batch_flag >> i & 0x01) {
            if (enable != -1) {
                videolosss[i].email_enable = enable;
            }
            if (interval != -1) {
                videolosss[i].email_buzzer_interval = interval;
            }
        }
    }

    write_video_losts(SQLITE_FILE_NAME, videolosss, MAX_CAMERA, batch_flag);
    copy_videoloss_email_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_VIDEOLOSS_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_set_videoloss_alarm_output(char *buf, int len, void *param, int *datalen,
                                                struct req_conf_str *conf)
{
    int chnid = -1;
    long long batch = 0;
    char sValue[256] = {0};
    struct video_loss videolost;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    memset(&videolost, 0, sizeof(struct video_loss));
    read_video_lost(SQLITE_FILE_NAME, &videolost, chnid);
    videolost.id = chnid;
    if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
        videolost.tri_alarms = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triChnAlarms1=", sValue, sizeof(sValue))) {
        snprintf(videolost.tri_chnout1_alarms, sizeof(videolost.tri_chnout1_alarms), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "triChnAlarms2=", sValue, sizeof(sValue))) {
        snprintf(videolost.tri_chnout2_alarms, sizeof(videolost.tri_chnout2_alarms), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch |= (long long)1 << chnid;

    copy_video_losts(SQLITE_FILE_NAME, &videolost, batch);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_VIDEOLOSS_BATCH, (void *)&batch, sizeof(long long), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_alarmin_audible_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct alarm_in_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&schedule, 0, sizeof(schedule));
    read_alarmin_audible_schedule(SQLITE_FILE_NAME, &schedule, chanid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_alarmin_audible_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int logs = 0;
    int enable = -1, interval = -1;
    int i = 0, num = 0;
    long long batch_flag = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct alarm_in ais[MAX_CAMERA];
    struct alarm_in_schedule audible_sche;
    int audioId = -1;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    memset(ais, 0, sizeof(struct alarm_in) * MAX_CAMERA);
    memset(&audible_sche, 0, sizeof(struct alarm_in_schedule));
    read_alarm_ins(SQLITE_FILE_NAME, ais, &num);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "audioId=", sValue, sizeof(sValue))) {
        audioId = atoi(sValue);
    }

    read_alarmin_audible_schedule(SQLITE_FILE_NAME, &audible_sche, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                if (iday == 0) {
                    logs = 1;
                }
                snprintf(audible_sche.schedule_day[iday].schedule_item[item].start_time,
                         sizeof(audible_sche.schedule_day[iday].schedule_item[item].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(audible_sche.schedule_day[iday].schedule_item[item].end_time,
                         sizeof(audible_sche.schedule_day[iday].schedule_item[item].end_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                audible_sche.schedule_day[iday].schedule_item[item].action_type = atoi(sValue);
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    for (i = 0; i < MAX_CAMERA; i++) {
        if (batch_flag >> i & 0x01) {
            if (enable != -1) {
                ais[i].enable = enable;
            }
            if (interval != -1) {
                ais[i].buzzer_interval = interval;
            }
            if (audioId != -1) {
                ais[i].tri_audio_id = audioId;
            }
        }
    }
    write_alarm_ins(SQLITE_FILE_NAME, ais, MAX_CAMERA, batch_flag);
    copy_alarmin_audible_schedules(SQLITE_FILE_NAME, &audible_sche, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMIN_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    if (logs == 1) {
        sdkp2p_common_write_log_batch(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMINPUT, batch_flag);
    }
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_alarmin_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct alarm_in_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&schedule, 0, sizeof(schedule));
    read_alarmin_email_schedule(SQLITE_FILE_NAME, &schedule, chanid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_alarmin_email_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, chnid = -1;
    int logs = 0;
    int enable = -1, interval = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};
    long long batch_flag = 0;
    int i = 0, num = 0;
    int change_flag = 0;
    char tmp_snapshot[66] = {0};
    struct alarm_in ais[MAX_CAMERA];
    struct alarm_in_schedule email_sche;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    memset(ais, 0, sizeof(struct alarm_in) * MAX_CAMERA);
    memset(&email_sche, 0, sizeof(struct alarm_in_schedule));
    read_alarm_ins(SQLITE_FILE_NAME, ais, &num);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "interval=", sValue, sizeof(sValue))) {
        interval = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "triSnapshotChannels=", sValue, sizeof(sValue))) {
        change_flag = 1;
        snprintf(tmp_snapshot, sizeof(tmp_snapshot), "%s", sValue);
    }

    read_alarmin_email_schedule(SQLITE_FILE_NAME, &email_sche, chnid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                if (iday == 0) {
                    logs = 1;
                }
                snprintf(email_sche.schedule_day[iday].schedule_item[item].start_time,
                         sizeof(email_sche.schedule_day[iday].schedule_item[item].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(email_sche.schedule_day[iday].schedule_item[item].end_time,
                         sizeof(email_sche.schedule_day[iday].schedule_item[item].end_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", iday, item);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                email_sche.schedule_day[iday].schedule_item[item].action_type = atoi(sValue);
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        batch_flag = sdkp2p_get_chnmask_to_atoll(sValue);
    }
    batch_flag |= (long long)1 << chnid;
    for (i = 0; i < MAX_CAMERA; i++) {
        if (batch_flag >> i & 0x01) {
            if (enable != -1) {
                ais[i].email_enable = enable;
            }
            if (interval != -1) {
                ais[i].email_buzzer_interval = interval;
            }
            if (change_flag == 1) {
                snprintf(ais[i].tri_channels_snapshot, sizeof(ais[i].tri_channels_snapshot), "%s", tmp_snapshot);
            }
        }
    }
    write_alarm_ins(SQLITE_FILE_NAME, ais, MAX_CAMERA, batch_flag);
    copy_alarmin_email_schedules(SQLITE_FILE_NAME, &email_sche, batch_flag);
    if (logs == 1) {
        sdkp2p_common_write_log_batch(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMINPUT, batch_flag);
    }
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMIN_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_set_alarmin_alarm_output(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, chnid = -1, type = -1;
    char sValue[256] = {0};
    struct alarm_in alarmin;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    memset(&alarmin, 0, sizeof(struct alarm_in));
    read_alarm_in(SQLITE_FILE_NAME, &alarmin, chnid);
    alarmin.id = chnid;

    if (sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    type = atoi(sValue);
    if (!type) {
        memset(alarmin.tri_channels_ex, 0, sizeof(alarmin.tri_channels_ex));
        alarmin.tri_channels = 0;
        if (!sdkp2p_get_section_info(buf, "triChannels=", sValue, sizeof(sValue))) {
            snprintf(alarmin.tri_channels_ex, sizeof(sValue), "%s", sValue);
        }
        for (i = 0; i < strlen(alarmin.tri_channels_ex); i++) {
            if (alarmin.tri_channels_ex[i] == '1') {
                alarmin.tri_channels |= (1 << i);
            }
        }
    } else {
        if (!sdkp2p_get_section_info(buf, "triAlarms=", sValue, sizeof(sValue))) {
            alarmin.tri_alarms = atoi(sValue);
        }
    }
    write_alarm_in(SQLITE_FILE_NAME, &alarmin);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMIN, (void *)&chnid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMINPUT, chnid + 1, 0, NULL, 0);
}

static void req_sche_get_alarmin_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct alarm_in_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);

    struct alarm_in alin = {0};
    memset(&alin, 0x0, sizeof(struct alarm_in));
    read_alarm_in(SQLITE_FILE_NAME, &alin, chanid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&", alin.ptzaction_interval);

    memset(&schedule, 0, sizeof(schedule));
    read_alarmin_ptz_schedule(SQLITE_FILE_NAME, &schedule, chanid);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_alarmin_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, j = 0, chnid = -1, cnt = 0;
    int logs = 0;
    char sValue[256] = {0};
    char tmp[64] = {0};
    struct alarm_in ai;
    memset(&ai, 0, sizeof(struct alarm_in));
    struct alarm_in_schedule ptz_sche;
    memset(&ptz_sche, 0, sizeof(struct alarm_in_schedule));
    struct ptz_action_params ptzActionParams[MAX_CAMERA];
    memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params)*MAX_CAMERA);

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chnid = atoi(sValue);
    read_alarm_in(SQLITE_FILE_NAME, &ai, chnid);
    ai.id = chnid;
    if (!sdkp2p_get_section_info(buf, "acto_ptz_channel=", sValue, sizeof(sValue))) {
        ai.acto_ptz_channel = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "acto_ptz_preset_enable=", sValue, sizeof(sValue))) {
        ai.acto_ptz_preset_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "acto_ptz_preset=", sValue, sizeof(sValue))) {
        ai.acto_ptz_preset = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "acto_ptz_patrol_enable=", sValue, sizeof(sValue))) {
        ai.acto_ptz_patrol_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "acto_ptz_patrol=", sValue, sizeof(sValue))) {
        ai.acto_ptz_patrol = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        ai.ptzaction_interval = atoi(sValue);
    }

    read_ptz_params(SQLITE_FILE_NAME, ptzActionParams, ALARMIO, chnid, &cnt);
    write_alarm_in(SQLITE_FILE_NAME, &ai);
    read_alarmin_ptz_schedule(SQLITE_FILE_NAME, &ptz_sche, chnid);
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                if (i == 0) {
                    logs = 1;
                }
                snprintf(ptz_sche.schedule_day[i].schedule_item[j].start_time,
                         sizeof(ptz_sche.schedule_day[i].schedule_item[j].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(ptz_sche.schedule_day[i].schedule_item[j].end_time, sizeof(ptz_sche.schedule_day[i].schedule_item[j].end_time),
                         "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                ptz_sche.schedule_day[i].schedule_item[j].action_type = atoi(sValue);
            }
        }
    }
    write_alarmin_ptz_schedule(SQLITE_FILE_NAME, &ptz_sche, chnid);
    if (logs == 1) {
        sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_ALARMINPUT, chnid + 1, 0, NULL, 0);
    }
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMIN, (void *)&chnid, sizeof(int), SDKP2P_NOT_CALLBACK);

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        char copy[MAX_CAMERA + 1];
        memset(copy, 0x0, sizeof(copy));
        snprintf(copy, sizeof(copy), "%s", sValue);
        for (i = 0; i < strlen(copy) && i < MAX_ALARM_IN; i++) {
            if (copy[i] == '1' && i != chnid) {
                for (j = 0; j < MAX_CAMERA; j++) {
                    if (ptzActionParams[j].acto_ptz_channel > 0) {
                        ptzActionParams[j].chn_id = i;
                    }
                }
                write_ptz_params_all(SQLITE_FILE_NAME, ptzActionParams, ALARMIO, i);
                write_alarmin_ptz_schedule(SQLITE_FILE_NAME, &ptz_sche, i);
                sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMIN, (void *)&i, sizeof(int), SDKP2P_NOT_CALLBACK);
            }
        }
    }
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_set_alarmin_record_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, chanid = -1;
    struct record_schedule schedule;

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    chanid = atoi(sValue);
    memset(&schedule, 0, sizeof(struct record_schedule));
    read_record_schedule(SQLITE_FILE_NAME, &schedule, chanid);
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
}

static void req_sche_set_move_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, i = 0, j = 0, cnt = 0;
    long long batch_flag = 0;
    char sValue[256] = {0}, tmp[64] = {0};
    struct motion_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));
    struct ptz_action_params ptzActionParams[MAX_CAMERA];
    memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params)*MAX_CAMERA);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    batch_flag |= (long long)1 << chnid;
    read_ptz_params(SQLITE_FILE_NAME, ptzActionParams, MOTION, chnid, &cnt);
    read_motion_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[i].schedule_item[j].start_time,
                         sizeof(schedule.schedule_day[i].schedule_item[j].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[i].schedule_item[j].end_time, sizeof(schedule.schedule_day[i].schedule_item[j].end_time),
                         "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule.schedule_day[i].schedule_item[j].action_type = atoi(sValue);
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        for (i = 0; i < MAX_CAMERA; i++) {
            if (sValue[i] == '1') {
                batch_flag |= (long long)1 << i;
                for (j = 0; j < MAX_CAMERA; j++) {
                    if (ptzActionParams[j].acto_ptz_channel > 0) {
                        ptzActionParams[j].chn_id = i;
                    }
                }

                write_ptz_params_all(SQLITE_FILE_NAME, ptzActionParams, MOTION, i);
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        struct motion move;
        memset(&move, 0x0, sizeof(struct motion));
        read_motion(SQLITE_FILE_NAME, &move, chnid);
        if (move.ptzaction_interval != atoi(sValue)) {
            move.ptzaction_interval = atoi(sValue);
            write_motion(SQLITE_FILE_NAME, &move);
        }
    }

    copy_motion_ptz_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_MOTION_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_move_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, i = 0, j = 0;
    char sValue[256] = {0};
    struct motion_schedule schedule;
    memset(&schedule, 0, sizeof(struct motion_schedule));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    struct motion move;
    memset(&move, 0x0, sizeof(struct motion));
    read_motion(SQLITE_FILE_NAME, &move, chnid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&", move.ptzaction_interval);

    read_motion_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid);

    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", i, j,
                     schedule.schedule_day[i].schedule_item[j].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", i, j,
                     schedule.schedule_day[i].schedule_item[j].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", i, j,
                     schedule.schedule_day[i].schedule_item[j].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;;
}

static void req_sche_set_videoloss_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, i = 0, j = 0, cnt = 0;
    long long batch_flag = 0;
    char sValue[256] = {0}, tmp[64] = {0};
    struct video_loss_schedule schedule;
    memset(&schedule, 0, sizeof(struct video_loss_schedule));
    struct ptz_action_params ptzActionParams[MAX_CAMERA];
    memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params)*MAX_CAMERA);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    batch_flag |= (long long)1 << chnid;
    read_ptz_params(SQLITE_FILE_NAME, ptzActionParams, VIDEOLOSS, chnid, &cnt);
    read_videoloss_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid);
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[i].schedule_item[j].start_time,
                         sizeof(schedule.schedule_day[i].schedule_item[j].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[i].schedule_item[j].end_time, sizeof(schedule.schedule_day[i].schedule_item[j].end_time),
                         "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule.schedule_day[i].schedule_item[j].action_type = atoi(sValue);
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        for (i = 0; i < MAX_CAMERA; i++) {
            if (sValue[i] == '1') {
                batch_flag |= (long long)1 << i;
                for (j = 0; j < MAX_CAMERA; j++) {
                    if (ptzActionParams[j].acto_ptz_channel > 0) {
                        ptzActionParams[j].chn_id = i;
                    }
                }
                write_ptz_params_all(SQLITE_FILE_NAME, ptzActionParams, VIDEOLOSS, i);
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        struct video_loss videoloss;
        memset(&videoloss, 0x0, sizeof(struct video_loss));
        read_video_lost(SQLITE_FILE_NAME, &videoloss, chnid);
        if (videoloss.ptzaction_interval != atoi(sValue)) {
            videoloss.ptzaction_interval = atoi(sValue);
            write_video_lost(SQLITE_FILE_NAME, &videoloss);
        }
    }
    copy_videoloss_ptz_schedules(SQLITE_FILE_NAME, &schedule, batch_flag);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_VIDEOLOSS_BATCH, (void *)&batch_flag, sizeof(long long), SDKP2P_NOT_CALLBACK);
    usleep(100 * 1000);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_videoloss_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, i = 0, j = 0;
    char sValue[256] = {0};
    struct video_loss_schedule schedule;
    memset(&schedule, 0, sizeof(struct video_loss_schedule));
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    struct video_loss videoloss;
    memset(&videoloss, 0x0, sizeof(struct video_loss));
    read_video_lost(SQLITE_FILE_NAME, &videoloss, chnid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&",
             videoloss.ptzaction_interval);

    read_videoloss_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid);

    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", i, j,
                     schedule.schedule_day[i].schedule_item[j].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", i, j,
                     schedule.schedule_day[i].schedule_item[j].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", i, j,
                     schedule.schedule_day[i].schedule_item[j].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_alarmin_effe_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int iday, item, alarm_id = -1;
    char sValue[256] = {0};
    char tmp[64] = {0};
    int i = 0;
    long long changeFlag = 0;
    char copyChn[MAX_LEN_65] = {0};
    struct smart_event_schedule schedule;

    memset(&schedule, 0, sizeof(struct smart_event_schedule));

    if (!sdkp2p_get_section_info(buf, "alarm_id=", sValue, sizeof(sValue))) {
        alarm_id = atoi(sValue);
    }
    if (alarm_id < 0 || alarm_id >= MAX_ALARM_IN) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        snprintf(copyChn, sizeof(copyChn), "%s", sValue);
    }
    read_alarmin_effective_schedule(SQLITE_FILE_NAME, &schedule, alarm_id);

    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
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
    copyChn[alarm_id] = '1';
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        if (copyChn[i] == '1') {
            changeFlag |= 1 << i;
            sdkp2p_send_msg(conf, REQUEST_FLAG_SET_ALARMIN, &i, sizeof(int), SDKP2P_NOT_CALLBACK);
        }
    }
    copy_alarmin_effective_schedules(SQLITE_FILE_NAME, &schedule, changeFlag);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_alarmin_effe_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int iday, item, alarm_id = -1;
    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct smart_event_schedule));

    if (!sdkp2p_get_section_info(buf, "alarm_id=", sValue, sizeof(sValue))) {
        alarm_id = atoi(sValue);
    }
    if (alarm_id < 0 || alarm_id >= MAX_ALARM_IN) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    read_alarmin_effective_schedule(SQLITE_FILE_NAME, &schedule, alarm_id);
    for (iday = 0; iday < MAX_DAY_NUM; iday++) {
        for (item = 0; item < MAX_PLAN_NUM_PER_DAY; item++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", iday, item,
                     schedule.schedule_day[iday].schedule_item[item].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_ptz_params(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chn_id = -1, type = -1;
    char sValue[368] = {0};
    struct ptz_action_params ptzParams;
    memset(&ptzParams, 0, sizeof(struct ptz_action_params));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chn_id = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "eventType=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }
    if (chn_id < 0 || chn_id >= MAX_CAMERA
        || type < 0 || type >= MAXEVT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    ptzParams.chn_id = chn_id;
    if (!sdkp2p_get_section_info(buf, "acto_ptz_channel=", sValue, sizeof(sValue))) {
        ptzParams.acto_ptz_channel = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "acto_ptz_type=", sValue, sizeof(sValue))) {
        ptzParams.acto_ptz_type = atoi(sValue);
    }
    if (ptzParams.acto_ptz_type == PRESET) {
        if (!sdkp2p_get_section_info(buf, "acto_ptz_preset=", sValue, sizeof(sValue))) {
            ptzParams.acto_ptz_preset = atoi(sValue);
        }
    } else if (ptzParams.acto_ptz_type == PATROL) {
        if (!sdkp2p_get_section_info(buf, "acto_ptz_patrol=", sValue, sizeof(sValue))) {
            ptzParams.acto_ptz_patrol = atoi(sValue);
        }
    } else if (ptzParams.acto_ptz_type == PATTERN) {
        if (!sdkp2p_get_section_info(buf, "acto_ptz_pattern=", sValue, sizeof(sValue))) {
            ptzParams.acto_ptz_pattern = atoi(sValue);
        }
    }

    write_ptz_params(SQLITE_FILE_NAME, &ptzParams, type);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_ptz_params(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, chn_id = -1, type = -1, cnt = 0;
    char sValue[368] = {0};
    struct ptz_action_params ptzActionParams[MAX_CAMERA];
    memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params)*MAX_CAMERA);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chn_id = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "eventType=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }
    if (chn_id < 0 || chn_id >= MAX_CAMERA
        || type < 0 || type >= MAXEVT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    read_ptz_params(SQLITE_FILE_NAME, ptzActionParams, type, chn_id, &cnt);
    for (i = 0; i < cnt && i < MAX_CAMERA; i++) {
        if (ptzActionParams[i].acto_ptz_channel > 0) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_channel[%d]=%d&", i,
                     ptzActionParams[i].acto_ptz_channel);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_type[%d]=%d&", i,
                     ptzActionParams[i].acto_ptz_type);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_preset[%d]=%d&", i,
                     ptzActionParams[i].acto_ptz_preset);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "acto_ptz_patrol[%d]=%d&", i,
                     ptzActionParams[i].acto_ptz_patrol);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_delete_ptz_params(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int type = -1, chn_id = -1;
    char sValue[368] = {0};
    struct ptz_action_params ptzActionParams;
    memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chn_id = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "eventType=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }
    if (chn_id < 0 || chn_id >= MAX_CAMERA
        || type < 0 || type >= MAXEVT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    ptzActionParams.chn_id = chn_id;
    if (!sdkp2p_get_section_info(buf, "acto_ptz_channel=", sValue, sizeof(sValue))) {
        ptzActionParams.acto_ptz_channel = atoi(sValue);
    }
    if (ptzActionParams.acto_ptz_channel < 0) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    delete_ptz_params(SQLITE_FILE_NAME, &ptzActionParams, type);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_update_ptz_params(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chn_id = -1, type = -1, old_chn_id = -1;
    char sValue[256] = {0};
    struct ptz_action_params ptzParams;
    memset(&ptzParams, 0x0, sizeof(struct ptz_action_params));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chn_id = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "eventType=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "old_chn_id=", sValue, sizeof(sValue))) {
        old_chn_id = atoi(sValue);
    }

    if (chn_id < 0 || chn_id >= MAX_CAMERA
        || type < 0 || type >= MAXEVT
        || old_chn_id < 0 || old_chn_id >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    ptzParams.chn_id = chn_id;
    if (!sdkp2p_get_section_info(buf, "acto_ptz_channel=", sValue, sizeof(sValue))) {
        ptzParams.acto_ptz_channel = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "acto_ptz_type=", sValue, sizeof(sValue))) {
        ptzParams.acto_ptz_type = atoi(sValue);
    }

    if (ptzParams.acto_ptz_type == PRESET) {
        if (!sdkp2p_get_section_info(buf, "acto_ptz_preset=", sValue, sizeof(sValue))) {
            ptzParams.acto_ptz_preset = atoi(sValue);
        }
    } else if (ptzParams.acto_ptz_type == PATROL) {
        if (!sdkp2p_get_section_info(buf, "acto_ptz_patrol=", sValue, sizeof(sValue))) {
            ptzParams.acto_ptz_patrol = atoi(sValue);
        }
    } else if (ptzParams.acto_ptz_type == PATTERN) {
        if (!sdkp2p_get_section_info(buf, "acto_ptz_pattern=", sValue, sizeof(sValue))) {
            ptzParams.acto_ptz_pattern = atoi(sValue);
        }
    }

    update_ptz_params(SQLITE_FILE_NAME, &ptzParams, type, old_chn_id);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_delete_ptz_params_batch(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, type = -1, cnt = -1;
    char sValue[256] = {0}, tmp[64] = {0};
    struct ptz_action_params ptzActionParams;
    memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        ptzActionParams.chn_id = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "eventType=", sValue, sizeof(sValue))) {
        type = atoi(sValue);
    }
    if (ptzActionParams.chn_id < 0 || ptzActionParams.chn_id >= MAX_CAMERA
        || type < 0 || type >= MAXEVT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "cnt=", sValue, sizeof(sValue))) {
        cnt = atoi(sValue);
    }
    for (i = 0; i < cnt; i++) {
        snprintf(tmp, sizeof(tmp), "acto_ptz_channel[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            ptzActionParams.acto_ptz_channel = atoi(sValue);
            delete_ptz_params(SQLITE_FILE_NAME, &ptzActionParams, type);
        }
    }
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_get_vca_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0, j = 0, chnid = -1, event = -1;

    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct smart_event_schedule));

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        event = atoi(sValue);
    }
    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    SMART_EVENT smartEvent;
    memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
    read_smart_event(SQLITE_FILE_NAME, &smartEvent, chnid, event);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ptz_interval=%d&",
             smartEvent.ptzaction_interval);
    read_smart_event_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "start_time[%d][%d]=%s&", i, j,
                     schedule.schedule_day[i].schedule_item[j].start_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "end_time[%d][%d]=%s&", i, j,
                     schedule.schedule_day[i].schedule_item[j].end_time);
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "action_type[%d][%d]=%d&", i, j,
                     schedule.schedule_day[i].schedule_item[j].action_type);
        }
    }

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_sche_set_vca_ptz_sche(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1, i = 0, j = 0, event = -1, cnt = 0;
    char sValue[256] = {0}, tmp[64] = {0};
    REQ_UPDATE_CHN req = {0};
    struct smart_event_schedule schedule;
    memset(&schedule, 0, sizeof(struct smart_event_schedule));
    struct ptz_action_params ptzActionParams[MAX_CAMERA];
    memset(&ptzActionParams, 0x0, sizeof(struct ptz_action_params)*MAX_CAMERA);

    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        chnid = atoi(sValue);
    }
    if (chnid < 0 || chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "event=", sValue, sizeof(sValue))) {
        event = atoi(sValue);
    }
    if (event < REGIONIN && event > MAX_SMART_EVENT) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }
    if (!sdkp2p_get_section_info(buf, "copy_channel=", sValue, sizeof(sValue))) {
        req.chnMask = get_channel_mask(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "ptz_interval=", sValue, sizeof(sValue))) {
        SMART_EVENT smartEvent;
        memset(&smartEvent, 0x0, sizeof(SMART_EVENT));
        read_smart_event(SQLITE_FILE_NAME, &smartEvent, chnid, event);
        if (smartEvent.ptzaction_interval != atoi(sValue)) {
            smartEvent.ptzaction_interval = atoi(sValue);
            write_smart_event(SQLITE_FILE_NAME, &smartEvent, event);
            copy_smart_events(SQLITE_FILE_NAME, &smartEvent, req.chnMask, event);
        }
    }

    read_smart_event_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            snprintf(tmp, sizeof(tmp), "start_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[i].schedule_item[j].start_time,
                         sizeof(schedule.schedule_day[i].schedule_item[j].start_time), "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "end_time[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                snprintf(schedule.schedule_day[i].schedule_item[j].end_time, sizeof(schedule.schedule_day[i].schedule_item[j].end_time),
                         "%s", sValue);
            }
            snprintf(tmp, sizeof(tmp), "action_type[%d][%d]=", i, j);
            if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
                schedule.schedule_day[i].schedule_item[j].action_type = atoi(sValue);
            }
        }
    }
    write_smart_event_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnid, event);
    copy_smart_event_ptz_schedules(SQLITE_FILE_NAME, &schedule, req.chnMask, event);
    
    read_ptz_params(SQLITE_FILE_NAME, ptzActionParams, event, chnid, &cnt);
    copy_ptz_params_all(SQLITE_FILE_NAME, ptzActionParams, event, (long long)req.chnMask);
    ms_set_bit(&req.chnMask, chnid, 1);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_SMART_EVENT, (void *)&req, sizeof(req), SDKP2P_NOT_CALLBACK);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_sche_set_alarm_out(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct dio_alarm *res = (struct dio_alarm *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.id=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->id));
    }

    if (!sdkp2p_get_section_info(buf, "r.enable=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->enable));
    }

    *datalen = sizeof(struct dio_alarm);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
}

static void req_sche_set_alarmpush_name(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[512] = {0};
    struct req_alarm_push_name pacc;

    memset(&pacc, 0, sizeof(struct req_alarm_push_name));
    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        snprintf(pacc.id, sizeof(pacc.id), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(pacc.name, sizeof(pacc.name), "%s", sValue);
    } else {
        snprintf(pacc.name, sizeof(pacc.name), "%s", "NVR");
    }

    sdkp2p_send_msg(conf, conf->req, &pacc, sizeof(struct req_alarm_push_name), SDKP2P_NOT_CALLBACK);
    //*(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_RECORD_SCHE, req_sche_get_record_sche, -1, NULL},//get.schedule.recsched
    {REQUEST_FLAG_SET_RECORD_SCHE, req_sche_set_record_sche, -1, NULL},//set.schedule.recsched
    {REQUEST_FLAG_GET_MOTION_SCHE, req_sche_get_motion_sche, -1, NULL},//get.schedule.motionsched
    {REQUEST_FLAG_SET_MOTION_SCHE, req_sche_set_motion_sche, -1, NULL},//set.schedule.motionsched
    {REQUEST_FLAG_GET_VIDEOLOSS_SCHE, req_sche_get_videoloss_sche, -1, NULL},//get.schedule.videolosssched
    {REQUEST_FLAG_SET_VIDEOLOSS_SCHE, req_sche_set_videoloss_sche, -1, NULL},//set.schedule.videolosssched
    {REQUEST_FLAG_GET_ALARMIN_SCHE, req_sche_get_alarmin_sche, -1, NULL},//get.schedule.alarminsched
    {REQUEST_FLAG_SET_ALARMIN_SCHE, req_sche_set_alarmin_sche, -1, NULL},//set.schedule.alarminsched
    {REQUEST_FLAG_GET_ALARMOUT_SCHE, req_sche_get_alarmout_sche, -1, NULL},//get.schedule.alarmoutsched
    {REQUEST_FLAG_SET_ALARMOUT_SCHE, req_sche_set_alarmout_sche, -1, NULL},//set.schedule.alarmoutsched
    {REQUEST_FLAG_SET_ALARM_PUSH_ID, req_sche_set_alarmpush_id, -1, NULL},//set.schedule.alarmpush
    {REQUEST_FLAG_SET_ALARM_PUSH_INTERVAL, req_sche_set_alarmpush_interval, -1, NULL},//set.schedule.alarmpush_inteval
    {REQUEST_FLAG_ADD_ANDROID_PUSH_ACCOUNT, req_sche_add_android_push_account, -1, NULL},//add.schedule.android_account
    {REQUEST_FLAG_DEL_ANDROID_PUSH_ACCOUNT, req_sche_del_android_push_account, -1, NULL},//del.schedule.android_account
    {REQUEST_FLAG_GET_MOTION_AUDIBLE_SCHE, req_sche_get_motion_audible_sche, -1, NULL},//get.schedule.motion_audible_sched
    {REQUEST_FLAG_SET_MOTION_AUDIBLE_SCHE, req_sche_set_motion_audible_sche, -1, NULL},//set.schedule.motion_audible_sched
    {REQUEST_FLAG_GET_MOTION_EMAIL_SCHE, req_sche_get_motion_email_sche, -1, NULL},//get.schedule.motion_email_sched
    {REQUEST_FLAG_SET_MOTION_EMAIL_SCHE, req_sche_set_motion_email_sche, -1, NULL},//set.schedule.motion_email_sched
    {REQUEST_FLAG_GET_MOTION_OTHERS_SCHE, req_sche_get_motion_others_sche, -1, NULL},//get.schedule.motion_others
    {REQUEST_FLAG_SET_MOTION_OTHERS_SCHE, req_sche_set_motion_others_sche, -1, NULL},//set.schedule.motion_others
    {REQUEST_FLAG_GET_VIDEOLOSS_AUDIBLE_SCHE, req_sche_get_videoloss_audible_sche, -1, NULL},//get.schedule.videoloss_audible_sched
    {REQUEST_FLAG_SET_VIDEOLOSS_AUDIBLE_SCHE, req_sche_set_videoloss_audible_sche, -1, NULL},//set.schedule.videoloss_audible_sched
    {REQUEST_FLAG_GET_VIDEOLOSS_EMAIL_SCHE, req_sche_get_videoloss_email_sche, -1, NULL},//get.schedule.videoloss_email_sched
    {REQUEST_FLAG_SET_VIDEOLOSS_EMAIL_SCHE, req_sche_set_videoloss_email_sche, -1, NULL},//set.schedule.videoloss_email_sched
    {REQUEST_FLAG_SET_VIDEOLOSS_ALARMOUTPUT, req_sche_set_videoloss_alarm_output, -1, NULL},//set.schedule.videoloss_alarm_output
    {REQUEST_FLAG_GET_ALARMIN_AUDIBLE_SCHE, req_sche_get_alarmin_audible_sche, -1, NULL},//get.schedule.alarmin_audible_sched
    {REQUEST_FLAG_SET_ALARMIN_AUDIBLE_SCHE, req_sche_set_alarmin_audible_sche, -1, NULL},//set.schedule.alarmin_audible_sched
    {REQUEST_FLAG_GET_ALARMIN_EMAIL_SCHE, req_sche_get_alarmin_email_sche, -1, NULL},//get.schedule.alarmin_email_sched
    {REQUEST_FLAG_SET_ALARMIN_EMAIL_SCHE, req_sche_set_alarmin_email_sche, -1, NULL},//set.schedule.alarmin_email_sched
    {REQUEST_FLAG_SET_ALARMIN_ALARMOUTPUT, req_sche_set_alarmin_alarm_output, -1, NULL},//set.schedule.alarmin_alarm_output
    {REQUEST_FLAG_GET_ALARMIN_PTZ_SCHE, req_sche_get_alarmin_ptz_sche, -1, NULL},//get.schedule.alarmin_ptz_sched
    {REQUEST_FLAG_SET_ALARMIN_PTZ_SCHE, req_sche_set_alarmin_ptz_sche, -1, NULL},//set.schedule.alarmin_ptz_sched
    {REQUEST_FLAG_SET_ALARMIN_RECORD_SCHE, req_sche_set_alarmin_record_sche, -1, NULL},//get.schedule.alarmin_record
    {REQUEST_FLAG_SET_PRIVACY_MASK, req_sche_set_privacy_mask, -1, NULL},//set.camera.privacy_mask
    {REQUEST_FLAG_GET_PRIVACY_MASK, req_sche_get_privacy_mask, RESPONSE_FLAG_GET_PRIVACY_MASK, resp_sche_get_privacy_mask},//get.camera.privacy_mask
    {REQUEST_FLAG_GET_MOTION_EFFECTIVE_SCHE, req_sche_get_motion_effective_sche, -1, NULL},//get.schedule.motion_effective_sched
    {REQUEST_FLAG_SET_MOTION_EFFECTIVE_SCHE, req_sche_set_motion_effective_sche, -1, NULL},//set.schedule.motion_effective_sched
    {REQUEST_FLAG_GET_VCA_EFFECTIVE_SCHE, req_sche_get_vca_effective_sche, -1, NULL},//get.schedule.vca_effective_sched
    {REQUEST_FLAG_SET_VCA_EFFECTIVE_SCHE, req_sche_set_vca_effective_sche, -1, NULL},//set.schedule.vca_effective_sched
    {REQUEST_FLAG_GET_VCA_AUDIBLE_SCHE, req_sche_get_vca_audible_sche, -1, NULL},//get.schedule.vca_audible_sched
    {REQUEST_FLAG_SET_VCA_AUDIBLE_SCHE, req_sche_set_vca_audible_sche, -1, NULL},//set.schedule.vca_audible_sched
    {REQUEST_FLAG_GET_VCA_EMAIL_SCHE, req_sche_get_vca_email_sche, -1, NULL},//get.schedule.vca_email_sched
    {REQUEST_FLAG_SET_VCA_EMAIL_SCHE, req_sche_set_vca_email_sche, -1, NULL},//set.schedule.vca_email_sched
    {REQUEST_FLAG_GET_VCA_OTHERS, req_sche_get_vca_others, -1, NULL},//get.schedule.vca_others
    {REQUEST_FLAG_SET_VCA_OTHERS, req_sche_set_vca_others, -1, NULL},//set.schedule.vca_others
    {REQUEST_FLAG_SET_MOVE_PTZ_SCHE, req_sche_set_move_ptz_sche, -1, NULL},//set.schedule.move.ptz.schedule
    {REQUEST_FLAG_GET_MOVE_PTZ_SCHE, req_sche_get_move_ptz_sche, -1, NULL},//get.schedule.move.ptz.schedule
    {REQUEST_FLAG_SET_VIDEOLOSS_PTZ_SCHE, req_sche_set_videoloss_ptz_sche, -1, NULL},//set.schedule.videoloss.ptz.schedule
    {REQUEST_FLAG_GET_VIDEOLOSS_PTZ_SCHE, req_sche_get_videoloss_ptz_sche, -1, NULL},//get.schedule.videoloss.ptz.schedule
    {REQUEST_FLAG_SET_ALARMIN_EFFECTIVE_SCHE, req_sche_set_alarmin_effe_sche, -1, NULL},//set.schedule.alarmin.effe.schedule
    {REQUEST_FLAG_GET_ALARMIN_EFFECTIVE_SCHE, req_sche_get_alarmin_effe_sche, -1, NULL},//get.schedule.alarmin.effe.schedule
    {REQUEST_FLAG_SET_PTZ_PARAMS, req_sche_set_ptz_params, -1, NULL},//set.schedule.ptz.params
    {REQUEST_FLAG_GET_PTZ_PARAMS, req_sche_get_ptz_params, -1, NULL},//get.schedule.ptz.params
    {REQUEST_FLAG_DELETE_PTZ_PARAMS, req_sche_delete_ptz_params, -1, NULL},//delete.schedule.ptz.params
    {REQUEST_FLAG_UPDATE_PTZ_PARAMS, req_sche_update_ptz_params, -1, NULL},//update.schedule.ptz.params
    {REQUEST_FLAG_DELETE_PTZ_PARAMS_BATCH, req_sche_delete_ptz_params_batch, -1, NULL},//delete.schedule.ptz.paramsbatch
    {REQUEST_FLAG_SET_VCA_PTZ_SCHE, req_sche_set_vca_ptz_sche, -1, NULL},//set.schedule.vca_ptz_shced
    {REQUEST_FLAG_GET_VCA_PTZ_SCHE, req_sche_get_vca_ptz_sche, -1, NULL},//get.schedule.vca_ptz_shced
    {REQUEST_FLAG_SET_ALARMOUT_ACT, req_sche_set_alarm_out, -1, NULL},//set.schedule.alarm.out
    {REQUEST_FLAG_SET_ALARM_PUSH_NAME, req_sche_set_alarmpush_name, RESPONSE_FLAG_SET_ALARM_PUSH_NAME, NULL}//set.schedule.alarmpush_name
    // request register array end
};


void p2p_schedule_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_schedule_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

