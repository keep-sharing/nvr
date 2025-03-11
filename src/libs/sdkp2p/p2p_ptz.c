#include "sdk_util.h"
#include "ptz_public.h"

static void req_ptz_set_action(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_ptz_action *res = (struct req_ptz_action *)param;
    char sValue[256] = {0};

    memset(res, 0x0, sizeof(struct req_ptz_action));
    if (!sdkp2p_get_section_info(buf, "r.action=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->action));
    }
    if (!sdkp2p_get_section_info(buf, "r.chn=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->chn));
    }
    if (!sdkp2p_get_section_info(buf, "r.param=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->param));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.tilt=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.tilt));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.pan=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.pan));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.zoom=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.zoom));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.focus=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.focus));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.timeout=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.timeout));
    }

    //if ipc is fishey, type==1
    if (!sdkp2p_get_section_info(buf, "r.type=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->type));
    }
    if (!sdkp2p_get_section_info(buf, "r.stream_id=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->stream_id));
    }

    if (!sdkp2p_get_section_info(buf, "r.name=", sValue, sizeof(sValue))) {
        snprintf(res->name, sizeof(res->name), "%s", sValue);
    } else {
        res->name[0] = '\0';
    }


    if (res->action == PTZ_AUTO_SCAN) {
        struct req_http_common_param speed;
        int tmp = res->speed.pan;
        if (tmp > 0) {
            tmp -= 1;
        } else {
            tmp = 1;
        }

        memset(&speed, 0, sizeof(struct req_http_common_param));
        speed.chnid = res->chn;
        snprintf(speed.info.httpname, sizeof(speed.info.httpname), "%s", "vsetting.html");
        snprintf(speed.info.key, sizeof(speed.info.key), "%s", "ptzspeed");
        snprintf(speed.info.value, sizeof(speed.info.value), "%d", tmp);
        sdkp2p_send_msg(conf, REQUEST_FLAG_SET_IPC_COMMON_PARAM, &speed, sizeof(struct req_http_common_param),
                        SDKP2P_NOT_CALLBACK);
    }

    *datalen = sizeof(struct req_ptz_action);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_PTZ_CONTROL_REMOTE, SUB_PARAM_NONE, res->chn + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_ptz_set_tour_add(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0;
    struct ptz_tour tour[TOUR_MAX];
    struct req_ptz_tour *res = (struct req_ptz_tour *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.chn=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->chn));
    }
    if (!sdkp2p_get_section_info(buf, "r.tourid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->tourid));
    }
    if (!sdkp2p_get_section_info(buf, "r.key=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->key));
    }
    if (!sdkp2p_get_section_info(buf, "r.preno=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->preno));
    }
    if (!sdkp2p_get_section_info(buf, "r.tour_spd=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->tour_spd));
    }
    if (!sdkp2p_get_section_info(buf, "r.tour_time=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->tour_time));
    }

    //for fisheye ipc
    if (!sdkp2p_get_section_info(buf, "r.fisheye=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->type));
    }

    if (!sdkp2p_get_section_info(buf, "r.streamid=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->stream_id));
    }

    if (conf->req == REQUEST_FLAG_PTZ_TOUR_CLEAR) {
        memset(tour, 0, sizeof(struct ptz_tour)*TOUR_MAX);
        for (i = 0; i < MS_KEY_MAX; i++) {
            tour[res->tourid - 1].keys[i].preset_id = 0;
            write_ptz_tour(SQLITE_FILE_NAME, tour, res->chn);
        }
    }

    *datalen = sizeof(struct req_ptz_tour);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_PTZ_CONTROL_REMOTE, SUB_PARAM_NONE, res->chn + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_ptz_get_serial_port(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = -1;
    char sValue[256] = {0};
    struct ptz_port ptz = {0};

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    chnid = atoi(sValue);
    if (chnid < 0 || chnid > MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    memset(&ptz, 0x0, sizeof(struct ptz_port));
    read_ptz_port(SQLITE_FILE_NAME, &ptz, chnid);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id=%d&", ptz.id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "baudrate=%d&", ptz.baudrate);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "data_bit=%d&", ptz.data_bit);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "stop_bit=%d&", ptz.stop_bit);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "parity_type=%d&", ptz.parity_type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "protocol=%d&", ptz.protocol);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "address=%d&", ptz.address);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "com_type=%d&", ptz.com_type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "connect_type=%d&", ptz.connect_type);
    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_ptz_set_serial_port(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_ptz_port *res = (struct req_ptz_port *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.id=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->id));
    }
    if (!sdkp2p_get_section_info(buf, "r.baudrate=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->baudrate));
    }
    if (!sdkp2p_get_section_info(buf, "r.data_bit=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->data_bit));
    }
    if (!sdkp2p_get_section_info(buf, "r.stop_bit=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->stop_bit));
    }
    if (!sdkp2p_get_section_info(buf, "r.parity_type=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->parity_type));
    }
    if (!sdkp2p_get_section_info(buf, "r.protocol=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->protocol));
    }
    if (!sdkp2p_get_section_info(buf, "r.address=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->address));
    }
    if (!sdkp2p_get_section_info(buf, "r.com_type=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->com_type));
    }
    if (!sdkp2p_get_section_info(buf, "r.connect_type=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->connect_type));
    }

    *datalen = sizeof(struct req_ptz_port);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_PTZ_CONTROL_REMOTE, SUB_PARAM_NONE, res->id + 1, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_ptz_get_ovf_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int *res = (int *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        *res = atoi(sValue);
    }

    *datalen = sizeof(int);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NEED_CALLBACK);
}

static void resp_ptz_get_ovf_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_ptz_ovf_info *info = (struct resp_ptz_ovf_info *)param;
    int i = 0, j = 0;

    for (i = 0; i < PRESET_MAX; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.preset[%d].enable=%d&", i, info->preset[i].enable);
        snprintf(buf + strlen(buf), len - strlen(buf), "r.preset[%d].name=%s&", i, info->preset[i].name);
    }

    for (i = 0; i < TOUR_MAX; i++) {
        for (j = 0; j < MS_KEY_MAX; j++) {
            if (info->tour[i].preset_id[j] <= 0) {
                break;
            }
            snprintf(buf + strlen(buf), len - strlen(buf), "r.tour[%d].preset[%d].id=%d&", i, j, info->tour[i].preset_id[j]);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.tour[%d].preset[%d].speed=%d&", i, j, info->tour[i].speed[j]);
            snprintf(buf + strlen(buf), len - strlen(buf), "r.tour[%d].preset[%d].timeout=%d&", i, j, info->tour[i].timeout[j]);
        }
    }

    for (i = 0; i < PATTERN_MAX; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "r.pattern[%d].enable=%d&", i, info->pattern[i]);
    }

    *datalen = strlen(buf) + 1;
}

static void req_ptz_get_fish_ovf_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct fisheye_ptz_preset_info *req = (struct fisheye_ptz_preset_info *)param;
    char sValue[256] = {0};

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        req->chn = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.streamid=", sValue, sizeof(sValue))) {
        req->stream_id = atoi(sValue);
    }

    *datalen = sizeof(struct fisheye_ptz_preset_info);
    sdkp2p_send_msg(conf, conf->req, req, *datalen, SDKP2P_NEED_CALLBACK);
}

static void req_ptz_set_fish_tour(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct fisheye_ptz_tour *req = (struct fisheye_ptz_tour *)param;
    char sValue[256] = {0};
    char sKey[256] = {0};
    int i = 0;
    int rowsize = 0;

    if (!sdkp2p_get_section_info(buf, "r.chnid=", sValue, sizeof(sValue))) {
        req->chn = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.tourid=", sValue, sizeof(sValue))) {
        req->param = atoi(sValue);
        if (req->param) {
            req->param -= 1;
        }
    }

    //if ipc is fisheye need set fisheye = 1
    if (!sdkp2p_get_section_info(buf, "r.streamid=", sValue, sizeof(sValue))) {
        req->stream_id = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "r.rowsize=", sValue, sizeof(sValue))) {
        rowsize = atoi(sValue);
    }

    for (i = 0; i < rowsize; i++) {
        snprintf(sKey, sizeof(sKey), "r.preset_id[%d]=", i);
        if (!sdkp2p_get_section_info(buf, sKey, sValue, sizeof(sValue))) {
            req->tour[req->param].preset_id[i] = atoi(sValue);
        }

        snprintf(sKey, sizeof(sKey), "r.speed[%d]=", i);
        if (!sdkp2p_get_section_info(buf, sKey, sValue, sizeof(sValue))) {
            req->tour[req->param].speed[i] = atoi(sValue);
        }

        snprintf(sKey, sizeof(sKey), "r.timeout[%d]=", i);
        if (!sdkp2p_get_section_info(buf, sKey, sValue, sizeof(sValue))) {
            req->tour[req->param].timeout[i] = atoi(sValue);
        }
    }

    *datalen = sizeof(struct fisheye_ptz_tour);
    sdkp2p_send_msg(conf, conf->req, req, *datalen, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_ptz_set_track_action(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_ptz_action *res = (struct req_ptz_action *)param;
    char sValue[256] = {0};

    memset(res, 0x0, sizeof(struct req_ptz_action));
    if (!sdkp2p_get_section_info(buf, "r.action=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->action));

        switch (res->action) {
            case 0:
                res->action = PTZ_PATTERN_RUN;
                break;
            case 1:
                res->action = PTZ_STOP_ALL;
                break;
            case 2:
                res->action = PTZ_PATTERN_START;
                break;
            case 3:
                res->action = PTZ_PATTERN_STOP;
                break;
            case 4:
                res->action = PTZ_PATTERN_DEL;
                break;
        }
    }
    if (!sdkp2p_get_section_info(buf, "r.chn=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->chn));
    }
    if (!sdkp2p_get_section_info(buf, "r.param=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->param));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.tilt=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.tilt));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.pan=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.pan));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.zoom=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.zoom));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.focus=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.focus));
    }
    if (!sdkp2p_get_section_info(buf, "r.speed.timeout=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->speed.timeout));
    }

    //if ipc is fishey, type==1
    if (!sdkp2p_get_section_info(buf, "r.type=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->type));
    }
    if (!sdkp2p_get_section_info(buf, "r.stream_id=", sValue, sizeof(sValue))) {
        sscanf(sValue, "%d", &(res->stream_id));
    }

    if (!sdkp2p_get_section_info(buf, "r.name=", sValue, sizeof(sValue))) {
        snprintf(res->name, sizeof(res->name), "%s", sValue);
    } else {
        res->name[0] = '\0';
    }

    *datalen = sizeof(struct req_ptz_action);
    sdkp2p_send_msg(conf, conf->req, res, *datalen, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_ptz_get_ptz_support(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int chnid = 0;
    char sValue[256] = {0};

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnid = atoi(sValue);
    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_ptz_get_ptz_support(void *param, int size, char *buf, int len, int *datalen)
{
    //int retval
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_ptz_set_ptz(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i = 0;
    char tmp[256] = {0};
    struct req_ptz_tour_all act = {0};
    struct ptz_tour tour[TOUR_MAX];

    if (sdkp2p_get_section_info(buf, "ch=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }

    memset(tour, 0, sizeof(struct ptz_tour)*TOUR_MAX);
    act.chn = atoi(sValue);;
    read_ptz_tour(SQLITE_FILE_NAME, tour, act.chn);

    if (!sdkp2p_get_section_info(buf, "tourid=", sValue, sizeof(sValue))) {
        act.tourid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "rowsize=", sValue, sizeof(sValue))) {
        act.rowsize = atoi(sValue);
    }

    for (i = 0; i < act.rowsize; i++) {
        snprintf(tmp, sizeof(tmp), "preset[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            act.path[i].preset_id = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "speed[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            act.path[i].speed = atoi(sValue);
        }

        snprintf(tmp, sizeof(tmp), "time[%d]=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            act.path[i].timeout = atoi(sValue);
        }

        tour[act.tourid - 1].keys[i].preset_id = act.path[i].preset_id;
        tour[act.tourid - 1].keys[i].speed = act.path[i].speed;
        tour[act.tourid - 1].keys[i].timeout = act.path[i].timeout;
    }

    write_ptz_tour(SQLITE_FILE_NAME, tour, act.chn);
    sdkp2p_send_msg(conf, conf->req, &act, sizeof(struct req_ptz_tour_all), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_ptz_set_control(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ms_ipc_ptz_control info;
    char sValue[256] = {0};

    memset(&info, 0, sizeof(struct ms_ipc_ptz_control));
    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    info.chnid = atoi(sValue);
    if (info.chnid < 0 || info.chnid >= MAX_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "ptzCmd=", sValue, sizeof(sValue))) {
        info.ptzCmd = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct ms_ipc_ptz_control), SDKP2P_NEED_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_PTZ_CONTROL_REMOTE, SUB_PARAM_NONE, info.chnid + 1, 0, NULL, 0);
}

static void resp_ptz_set_control(void *param, int size, char *buf, int len, int *datalen)
{
    int *info = (int *)param;
    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", *info);

    *datalen = strlen(buf) + 1;
}

static void req_ptz_get_led_status(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
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
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, &chnid, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_ptz_get_led_status(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_ipc_led_status_resp *info = (struct ms_ipc_led_status_resp *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d&", info->chnid);
    snprintf(buf + strlen(buf), len - strlen(buf), "led_status=%d&", info->led_status);
    *datalen = strlen(buf) + 1;
}

static void req_ptz_get_basic(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int chnId = 0;

    if (sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return;
    }
    chnId = atoi(sValue);
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, &chnId, sizeof(int), SDKP2P_NEED_CALLBACK);
}

static void resp_ptz_get_basic(void *param, int size, char *buf, int len, int *datalen)
{
    struct PtzBasicInfo *info = (struct PtzBasicInfo *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d\r\n", info->ret);
    snprintf(buf + strlen(buf), len - strlen(buf), "chnid=%d\r\n", info->chanid);
    snprintf(buf + strlen(buf), len - strlen(buf), "zoomStatus=%d\r\n", info->zoomStatus);
    snprintf(buf + strlen(buf), len - strlen(buf), "panTiltStatus=%d\r\n", info->panTiltStatus);
    snprintf(buf + strlen(buf), len - strlen(buf), "presetStatus=%d\r\n", info->presetStatus);
    snprintf(buf + strlen(buf), len - strlen(buf), "patrolStatus=%d\r\n", info->patrolStatus);
    snprintf(buf + strlen(buf), len - strlen(buf), "patternStatus=%d\r\n", info->patternStatus);
    snprintf(buf + strlen(buf), len - strlen(buf), "autoScanStatus=%d\r\n", info->autoScanStatus);
    snprintf(buf + strlen(buf), len - strlen(buf), "presetFreezing=%d\r\n", info->presetFreezing);
    snprintf(buf + strlen(buf), len - strlen(buf), "presetSpeed=%d\r\n", info->presetSpeed);
    snprintf(buf + strlen(buf), len - strlen(buf), "manualSpeed=%d\r\n", info->manualSpeed);
    snprintf(buf + strlen(buf), len - strlen(buf), "patrolRecovering=%d\r\n", info->patrolRecovering);
    snprintf(buf + strlen(buf), len - strlen(buf), "patrolRecoveryTime=%d\r\n", info->patrolRecoveryTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "focusMode=%d\r\n", info->focusMode);
    snprintf(buf + strlen(buf), len - strlen(buf), "minFocusDistance=%d\r\n", info->minFocusDistance);
    snprintf(buf + strlen(buf), len - strlen(buf), "resumeTime=%d\r\n", info->resumeTime);
    snprintf(buf + strlen(buf), len - strlen(buf), "fanWorkingMode=%d\r\n", info->fanWorkingMode);

    *datalen = strlen(buf) + 1;
}

static void req_ptz_set_basic(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct PtzBasicInfo info = {0};

    info.chanid = -1;
    if (!sdkp2p_get_section_info(buf, "chnid=", sValue, sizeof(sValue))) {
        info.chanid = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "channelMask=", sValue, sizeof(sValue))) {
        snprintf(info.channelMask, sizeof(info.channelMask), "%s", sValue);
    }

    if (strchr(info.channelMask, '1')) {
        info.chanid = -1;
    } else if (info.chanid < 0 || info.chanid >= MAX_REAL_CAMERA) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    if (!sdkp2p_get_section_info(buf, "zoomStatus=", sValue, sizeof(sValue))) {
        info.zoomStatus = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "panTiltStatus=", sValue, sizeof(sValue))) {
        info.panTiltStatus = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "presetStatus=", sValue, sizeof(sValue))) {
        info.presetStatus = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "patrolStatus=", sValue, sizeof(sValue))) {
        info.patrolStatus = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "patternStatus=", sValue, sizeof(sValue))) {
        info.patternStatus = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "autoScanStatus=", sValue, sizeof(sValue))) {
        info.autoScanStatus = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "presetFreezing=", sValue, sizeof(sValue))) {
        info.presetFreezing = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "presetSpeed=", sValue, sizeof(sValue))) {
        info.presetSpeed = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "manualSpeed=", sValue, sizeof(sValue))) {
        info.manualSpeed = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "patrolRecovering=", sValue, sizeof(sValue))) {
        info.patrolRecovering = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "patrolRecoveryTime=", sValue, sizeof(sValue))) {
        info.patrolRecoveryTime = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "focusMode=", sValue, sizeof(sValue))) {
        info.focusMode = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "minFocusDistance=", sValue, sizeof(sValue))) {
        info.minFocusDistance = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "resumeTime=", sValue, sizeof(sValue))) {
        info.resumeTime = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "fanWorkingMode=", sValue, sizeof(sValue))) {
        info.fanWorkingMode = atoi(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &info, sizeof(info), SDKP2P_NEED_CALLBACK);
}

static void resp_ptz_set_basic(void *param, int size, char *buf, int len, int *datalen)
{
    int res = -1;
    if (param) {
        res = *(int *)param;
    }

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);

    *datalen = strlen(buf) + 1;
}

SDK_ERROR_CODE get_ptz_cmd_info(const char *buff, struct req_ptz_action *info)
{
    if (!buff || !info) {
        return PARAM_ERROR;
    }
    char sValue[256] = {0};

    memset(info, 0, sizeof(struct req_ptz_action));
    if (sdkp2p_get_section_info(buff, "chnId=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    info->chn = atoi(sValue);

    if (sdkp2p_get_section_info(buff, "action=", sValue, sizeof(sValue))) {
        return PARAM_ERROR;
    }
    info->action = atoi(sValue);

    if (!sdkp2p_get_section_info(buff, "param=", sValue, sizeof(sValue))) {
        info->param = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "pan=", sValue, sizeof(sValue))) {
        info->speed.pan = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "tilt=", sValue, sizeof(sValue))) {
        info->speed.tilt = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "zoom=", sValue, sizeof(sValue))) {
        info->speed.zoom = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "focus=", sValue, sizeof(sValue))) {
        info->speed.focus = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "timeout=", sValue, sizeof(sValue))) {
        info->speed.timeout = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "name=", sValue, sizeof(sValue))) {
        snprintf(info->name, sizeof(info->name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buff, "type=", sValue, sizeof(sValue))) {
        info->type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buff, "streamId=", sValue, sizeof(sValue))) {
        info->stream_id = atoi(sValue);
    }

    return REQUEST_SUCCESS;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_PTZ_ACTION, req_ptz_set_action, -1, NULL},//5000
    {REQUEST_FLAG_PTZ_TOUR_ADD, req_ptz_set_tour_add, RESPONSE_FLAG_PTZ_TOUR_ADD, NULL},//set.ptz.tour_add
    {REQUEST_FLAG_PTZ_TOUR_DELETE, req_ptz_set_tour_add, -1, NULL},//set.ptz.tour_delete
    {REQUEST_FLAG_PTZ_TOUR_UP, req_ptz_set_tour_add, -1, NULL},//set.ptz.tour_up
    {REQUEST_FLAG_PTZ_TOUR_DOWN, req_ptz_set_tour_add, -1, NULL},//set.ptz.tour_down
    {REQUEST_FLAG_PTZ_TOUR_CLEAR, req_ptz_set_tour_add, -1, NULL},//set.ptz.tour_clear //6057
    {REQUEST_FLAG_PTZ_TOUR_RUN, req_ptz_set_tour_add, -1, NULL},//set.ptz.tour_run //5002
    {REQUEST_FLAG_PTZ_TOUR_STOP, req_ptz_set_tour_add, -1, NULL},//set.ptz.tour_stop //5003
    {REQUEST_FLAG_GET_PTZ_SERIAL_PORT, req_ptz_get_serial_port, RESPONSE_FLAG_GET_PTZ_SERIAL_PORT, NULL},//get.ptz.serial_port
    {REQUEST_FLAG_PTZ_SET_SERIAL_PORT, req_ptz_set_serial_port, -1, NULL},//set.ptz.serial_port
    {REQUEST_FLAG_PTZ_OVF_INFO, req_ptz_get_ovf_info, RESPONSE_FLAG_PTZ_OVF_INFO, resp_ptz_get_ovf_info},//5005 normal
    {REQUEST_FLAG_PTZ_HTTP_FISH_INFO, req_ptz_get_fish_ovf_info, RESPONSE_FLAG_PTZ_HTTP_FISH_INFO, resp_ptz_get_ovf_info},//5005 fish eye
    {REQUEST_FLAG_PTZ_PRESET_ACTION, req_ptz_set_action, -1, NULL},//5001
    {REQUEST_FLAG_PTZ_FISH_TOUR, req_ptz_set_fish_tour, -1, NULL},//6056
    {REQUEST_FLAG_PTZ_TRACK_ACTION, req_ptz_set_track_action, -1, NULL},//5004
    {REQUEST_FLAG_PTZ_SUPPORT, req_ptz_get_ptz_support, RESPONSE_FLAG_PTZ_SUPPORT, resp_ptz_get_ptz_support},//6052
    {REQUEST_FLAG_PTZ_SET, req_ptz_set_ptz, -1, NULL},//6055

    {REQUEST_FLAG_SET_IPC_PTZ_CONTROL, req_ptz_set_control, RESPONSE_FLAG_SET_IPC_PTZ_CONTROL, resp_ptz_set_control},//set.ptz.control
    {REQUEST_FLAG_GET_IPC_LED_STATUS, req_ptz_get_led_status, RESPONSE_FLAG_GET_IPC_LED_STATUS, resp_ptz_get_led_status},//get.ptz.led_status
    {REQUEST_FLAG_GET_PTZ_BASIC, req_ptz_get_basic, RESPONSE_FLAG_GET_PTZ_BASIC, resp_ptz_get_basic},//get.ptz.basic
    {REQUEST_FLAG_SET_PTZ_BASIC, req_ptz_set_basic, RESPONSE_FLAG_SET_PTZ_BASIC, resp_ptz_set_basic},//set.ptz.basic
    // request register array end
};


void p2p_ptz_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_ptz_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

