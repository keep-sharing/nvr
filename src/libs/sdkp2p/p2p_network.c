#include "sdk_util.h"


int get_network_check_port_json(const char *data, char *resp, int respLen)
{
    if (!data) {
        return -1;
    }
    
    char *sResp = NULL;
    cJSON *json = NULL;
    char sValue[256] = {0};
    int port;

    json = cJSON_CreateObject();
    if (!json) {
        return -1;
    }
    
    if (sdkp2p_get_section_info(data, "port=", sValue, sizeof(sValue))) {
        return -1;
    }
    port = atoi(sValue);

    cJSON_AddNumberToObject(json, "status", SDK_SUC);
    if (is_port_use(port)) {
        cJSON_AddNumberToObject(json, "res", 1);
    } else {
        cJSON_AddNumberToObject(json, "res", 0);
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

static void req_network_set_network(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct req_set_sysconf sys;
    struct network net = {0};

    memset(&sys, 0, sizeof(struct req_set_sysconf));
    read_network(SQLITE_FILE_NAME, &net);
    snprintf(sys.arg, sizeof(sys.arg), "network");
    if (!sdkp2p_get_section_info(buf, "mode=", sValue, sizeof(sValue))) {
        net.mode = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "host_name=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(net.host_name, sizeof(net.host_name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_primary_net=", sValue, sizeof(sValue))) {
        net.bond0_primary_net = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_enable=", sValue, sizeof(sValue))) {
        net.bond0_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_type=", sValue, sizeof(sValue))) {
        net.bond0_type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_ip_address=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_ip_address, sizeof(net.bond0_ip_address), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_netmask=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_netmask, sizeof(net.bond0_netmask), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_gateway, sizeof(net.bond0_gateway), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_primary_dns=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_primary_dns, sizeof(net.bond0_primary_dns), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_second_dns=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_second_dns, sizeof(net.bond0_second_dns), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "bond0_mtu=", sValue, sizeof(sValue))) {
        net.bond0_mtu = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_enable=", sValue, sizeof(sValue))) {
        net.lan1_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_type=", sValue, sizeof(sValue))) {
        net.lan1_type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_ip_address=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_ip_address, sizeof(net.lan1_ip_address), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_netmask=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_netmask, sizeof(net.lan1_netmask), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_gateway, sizeof(net.lan1_gateway), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_primary_dns=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_primary_dns, sizeof(net.lan1_primary_dns), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_second_dns=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_second_dns, sizeof(net.lan1_second_dns), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_mtu=", sValue, sizeof(sValue))) {
        net.lan1_mtu = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_enable=", sValue, sizeof(sValue))) {
        net.lan2_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_type=", sValue, sizeof(sValue))) {
        net.lan2_type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_ip_address=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_ip_address, sizeof(net.lan2_ip_address), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_netmask=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_netmask, sizeof(net.lan2_netmask), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_gateway, sizeof(net.lan2_gateway), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_primary_dns=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_primary_dns, sizeof(net.lan2_primary_dns), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_second_dns=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_second_dns, sizeof(net.lan2_second_dns), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_mtu=", sValue, sizeof(sValue))) {
        net.lan2_mtu = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan1_dhcp_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_dhcp_gateway, sizeof(net.lan1_dhcp_gateway), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "lan2_dhcp_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_dhcp_gateway, sizeof(net.lan2_dhcp_gateway), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "tri_alarms=", sValue, sizeof(sValue))) {
        net.tri_alarms = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan1_ip6_addr=", sValue, sizeof(sValue))) { //hrz.milesight
        snprintf(net.lan1_ip6_address, sizeof(net.lan1_ip6_address), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan1_ip6_netmask=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_ip6_netmask, sizeof(net.lan1_ip6_netmask), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan1_ip6_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.lan1_ip6_gateway, sizeof(net.lan1_ip6_gateway), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan2_ip6_addr=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_ip6_address, sizeof(net.lan2_ip6_address), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan2_ip6_netmask=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_ip6_netmask, sizeof(net.lan2_ip6_netmask), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan2_ip6_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.lan2_ip6_gateway, sizeof(net.lan2_ip6_gateway), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan1_ip6_dhcp=", sValue, sizeof(sValue))) {
        net.lan1_ip6_dhcp = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "lan2_ip6_dhcp=", sValue, sizeof(sValue))) {
        net.lan2_ip6_dhcp = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "bond0_ip6_dhcp=", sValue, sizeof(sValue))) {
        net.bond0_ip6_dhcp = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "bond0_ip6_addr=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_ip6_address, sizeof(net.bond0_ip6_address), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "bond0_ip6_netmask=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_ip6_netmask, sizeof(net.bond0_ip6_netmask), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "bond0_ip6_gateway=", sValue, sizeof(sValue))) {
        snprintf(net.bond0_ip6_gateway, sizeof(net.bond0_ip6_gateway), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "defaultRoute=", sValue, sizeof(sValue))) {
        net.defaultRoute = atoi(sValue);
    }

    write_network(SQLITE_FILE_NAME, &net);
    sdkp2p_send_msg(conf, conf->req, &sys, sizeof(struct req_set_sysconf), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_NETWORK, 0, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_network_set_test_email(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_set_sysconf res;
    memset(&res, 0, sizeof(struct req_set_sysconf));
    snprintf(res.arg, sizeof(res.arg), "testmail");

    sdkp2p_send_msg(conf, conf->req, &res, sizeof(struct req_set_sysconf), SDKP2P_NEED_CALLBACK);
}

static void resp_network_set_test_email(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void resp_network_set_enable_p2p(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_network_get_network(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct network net = {0};

    if (p2p_format_json(buf)) {
        ware_network_get_general_cb(conf->reqUrl, conf->resp, conf->size);
    } else {
        read_network(SQLITE_FILE_NAME, &net);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "mode=%d&", net.mode);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "host_name=%s&", net.host_name);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_primary_net=%d&",
                 net.bond0_primary_net);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_enable=%d&", net.bond0_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_type=%d&", net.bond0_type);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_ip_address=%s&",
                 net.bond0_ip_address);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_netmask=%s&", net.bond0_netmask);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_gateway=%s&", net.bond0_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_primary_dns=%s&",
                 net.bond0_primary_dns);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_second_dns=%s&",
                 net.bond0_second_dns);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_mtu=%d&", net.bond0_mtu);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_enable=%d&", net.lan1_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_type=%d&", net.lan1_type);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_ip_address=%s&", net.lan1_ip_address);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_netmask=%s&", net.lan1_netmask);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_gateway=%s&", net.lan1_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_primary_dns=%s&",
                 net.lan1_primary_dns);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_second_dns=%s&", net.lan1_second_dns);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_mtu=%d&", net.lan1_mtu);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_enable=%d&", net.lan2_enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_type=%d&", net.lan2_type);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_ip_address=%s&", net.lan2_ip_address);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_netmask=%s&", net.lan2_netmask);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_gateway=%s&", net.lan2_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_primary_dns=%s&",
                 net.lan2_primary_dns);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_second_dns=%s&", net.lan2_second_dns);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_mtu=%d&", net.lan2_mtu);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_dhcp_gateway=%s&",
                 net.lan1_dhcp_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_dhcp_gateway=%s&",
                 net.lan2_dhcp_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "tri_alarms=%d&", net.tri_alarms);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_ip6_addr=%s&", net.lan1_ip6_address);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_ip6_netmask=%s&",
                 net.lan1_ip6_netmask);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_ip6_gateway=%s&",
                 net.lan1_ip6_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_ip6_addr=%s&", net.lan2_ip6_address);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_ip6_netmask=%s&",
                 net.lan2_ip6_netmask);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_ip6_gateway=%s&",
                 net.lan2_ip6_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan1_ip6_dhcp=%d&", net.lan1_ip6_dhcp);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "lan2_ip6_dhcp=%d&", net.lan2_ip6_dhcp);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_ip6_dhcp=%d&", net.bond0_ip6_dhcp);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_ip6_address=%s&",
                 net.bond0_ip6_address);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_ip6_netmask=%s&",
                 net.bond0_ip6_netmask);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "bond0_ip6_gateway=%s&",
                 net.bond0_ip6_gateway);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "defaultRoute=%d&", net.defaultRoute);
    }

    *(conf->len) = strlen(conf->resp) + 1;

}

static void req_network_get_network_more(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i, nValue = 0;
    struct network_more port = {0};
    struct push_msg_event push_msg[MAX_REAL_CAMERA];
    struct PushMsgNvrEvent dbPush;
    char tmp[16] = {0};

    read_port(SQLITE_FILE_NAME, &port);
    get_param_value(SQLITE_FILE_NAME, PARAM_PUSH_MSG, tmp, sizeof(tmp), "0");
    read_push_msg_events(SQLITE_FILE_NAME, push_msg, MAX_REAL_CAMERA);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable_ssh=%d&", port.enable_ssh);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ssh_port=%d&", port.ssh_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "http_port=%d&", port.http_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "rtsp_port=%d&", port.rtsp_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "url=%s&", port.url);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "url_enable=%d&", port.url_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable_push=%d&", atoi(tmp));
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "https_port=%d&", port.https_port);
    nValue = get_param_int(SQLITE_FILE_NAME, PARAM_PUSH_TYPE, 0);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "push_event_type=%d&", nValue);
    nValue = get_param_int(SQLITE_FILE_NAME, PARAM_PUSH_VIDEO_STREAM, 0);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "push_video_stream=%d&", nValue);
    nValue = get_param_int(SQLITE_FILE_NAME, PARAM_PUSH_ALARMIN_TYPE, 65535);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "push_type_nvr=%d&", nValue);

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "push_type_chn%d=%d\r\n", i,
                 push_msg[i].push_type);
    }

    memset(&dbPush, 0, sizeof(struct PushMsgNvrEvent));
    read_push_msg_nvr_event(SQLITE_FILE_NAME, &dbPush);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "pushNvrPos=%s&", dbPush.pos);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_network_set_network_more(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i, nValue = 0;
    char sValue[256] = {0};
    char pushMsg[64] = {0};
    struct network_more more = {0};
    struct req_set_sysconf sys;
    struct push_msg_event push_msg;
    struct PushMsgNvrEvent dbPush;

    memset(&sys, 0, sizeof(struct req_set_sysconf));
    snprintf(sys.arg, sizeof(sys.arg), "more");

    if (!sdkp2p_get_section_info(buf, "enable_ssh=", sValue, sizeof(sValue))) {
        more.enable_ssh = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ssh_port=", sValue, sizeof(sValue))) {
        more.ssh_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "http_port=", sValue, sizeof(sValue))) {
        more.http_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "rtsp_port=", sValue, sizeof(sValue))) {
        more.rtsp_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "https_port=", sValue, sizeof(sValue))) {
        more.https_port = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "url=", sValue, sizeof(sValue))) {
        snprintf(more.url, sizeof(more.url), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "url_enable=", sValue, sizeof(sValue))) {
        more.url_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enable_push=", sValue, sizeof(sValue))) {
        if (!check_param_key(SQLITE_FILE_NAME, PARAM_PUSH_MSG)) {
            set_param_value(SQLITE_FILE_NAME, PARAM_PUSH_MSG, sValue);
        } else {
            add_param_value(SQLITE_FILE_NAME, PARAM_PUSH_MSG, sValue);
        }
    }

    if (!sdkp2p_get_section_info(buf, "push_event_type=", sValue, sizeof(sValue))) {
        nValue = atoi(sValue);
        set_param_int(SQLITE_FILE_NAME, PARAM_PUSH_TYPE, nValue);
    }
    if (!sdkp2p_get_section_info(buf, "push_video_stream=", sValue, sizeof(sValue))) {
        nValue = atoi(sValue);
        set_param_int(SQLITE_FILE_NAME, PARAM_PUSH_VIDEO_STREAM, nValue);
    }
    if (!sdkp2p_get_section_info(buf, "push_type_nvr=", sValue, sizeof(sValue))) {
        nValue = atoi(sValue);
        set_param_int(SQLITE_FILE_NAME, PARAM_PUSH_ALARMIN_TYPE, nValue);
    }

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        snprintf(pushMsg, sizeof(pushMsg), "push_type_chn%d=", i);
        if (!sdkp2p_get_section_info(buf, pushMsg, sValue, sizeof(sValue))) {
            push_msg.push_type = atoi(sValue);
            push_msg.chnid = i;
            write_push_msg_event(SQLITE_FILE_NAME, &push_msg);
        }
    }

    
    if (!sdkp2p_get_section_info(buf, "pushNvrPos=", sValue, sizeof(sValue))) {
        memset(&dbPush, 0, sizeof(struct PushMsgNvrEvent));
        read_push_msg_nvr_event(SQLITE_FILE_NAME, &dbPush);
        snprintf(dbPush.pos, sizeof(dbPush.pos), "%s", sValue);
        write_push_msg_nvr_event(SQLITE_FILE_NAME, &dbPush);
    }

    write_port(SQLITE_FILE_NAME, &more);

    sdkp2p_send_msg(conf, REQUEST_FLAG_PUSH_MSG, NULL, 0, SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_NETWORK, &sys, sizeof(struct req_set_sysconf), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_NETWORK_MORE, 0, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_network_get_network_email(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct email email;
    struct network_more nm;

    memset(&email, 0, sizeof(struct email));
    memset(&nm, 0, sizeof(struct network_more));
    read_email(SQLITE_FILE_NAME, &email);
    read_port(SQLITE_FILE_NAME, &nm);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", email.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "username=%s&", email.username);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "password=%s&", email.password);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "smtp_server=%s&", email.smtp_server);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "port=%d&", email.port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sender_addr=%s&", email.sender_addr);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sender_name=%s&", email.sender_name);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable_tls=%d&", email.enable_tls);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable_attach=%d&", email.enable_attach);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "capture_interval=%d&",
             email.capture_interval);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_id0=%d&", email.receiver[0].id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_address0=%s&",
             email.receiver[0].address);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_name0=%s&",
             email.receiver[0].name);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_id1=%d&", email.receiver[1].id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_address1=%s&",
             email.receiver[1].address);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_name1=%s&",
             email.receiver[1].name);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_id2=%d&", email.receiver[2].id);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_address2=%s&",
             email.receiver[2].address);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "receiver_name2=%s&",
             email.receiver[2].name);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enableHostName=%d&", nm.url_enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "hostName=%s&", nm.url);

    *(conf->len) = strlen(conf->resp) + 1;

}

static void req_network_set_network_email(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char tmp[64] = {0};
    int i;
    struct email email;
    struct req_set_sysconf sys;
    struct network_more nm;

    memset(&email, 0, sizeof(struct email));
    memset(&sys, 0, sizeof(struct req_set_sysconf));
    memset(&nm, 0, sizeof(struct network_more));
    snprintf(sys.arg, sizeof(sys.arg), "%s", "mail");
    read_port(SQLITE_FILE_NAME, &nm);
    read_email(SQLITE_FILE_NAME, &email);

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        email.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(email.username, sizeof(email.username), "%s", sValue);
    }
    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(email.password, sizeof(email.password), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "smtp_server=", sValue, sizeof(sValue))) {
        snprintf(email.smtp_server, sizeof(email.smtp_server), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "port=", sValue, sizeof(sValue))) {
        email.port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sender_addr=", sValue, sizeof(sValue))) {
        snprintf(email.sender_addr, sizeof(email.sender_addr), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "sender_name=", sValue, sizeof(sValue))) {
        snprintf(email.sender_name, sizeof(email.sender_name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enable_tls=", sValue, sizeof(sValue))) {
        email.enable_tls = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enable_attach=", sValue, sizeof(sValue))) {
        email.enable_attach = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "capture_interval=", sValue, sizeof(sValue))) {
        email.capture_interval = atoi(sValue);
    }

    for (i = 0; i < EMAIL_RECEIVER_NUM; i++) {
        snprintf(tmp, sizeof(tmp), "receiver_id%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            email.receiver[i].id = atoi(sValue);
        }
        snprintf(tmp, sizeof(tmp), "receiver_address%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(email.receiver[i].address, sizeof(email.receiver[i].address), "%s", sValue);
        }
        snprintf(tmp, sizeof(tmp), "receiver_name%d=", i);
        if (!sdkp2p_get_section_info(buf, tmp, sValue, sizeof(sValue))) {
            snprintf(email.receiver[i].name, sizeof(email.receiver[i].address), "%s", sValue);
        }
    }

    write_email(SQLITE_FILE_NAME, &email);

    if (!sdkp2p_get_section_info(buf, "enableHostName=", sValue, sizeof(sValue))) {
        nm.url_enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "hostName=", sValue, sizeof(sValue))) {
        snprintf(nm.url, sizeof(nm.url), "%s", sValue);
    }
    write_port(SQLITE_FILE_NAME, &nm);

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_NETWORK, &sys, sizeof(struct req_set_sysconf), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_NETWORK_MAIL, 0, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_network_get_network_ddns(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct ddns ddns = {0};

    memset(&ddns, 0x0, sizeof(struct ddns));
    read_ddns(SQLITE_FILE_NAME, &ddns);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", ddns.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "domain=%s&", ddns.domain);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "username=%s&", ddns.username);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "password=%s&", ddns.password);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "host_name=%s&", ddns.host_name);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "free_dns_hash=%s&", ddns.free_dns_hash);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "update_freq=%d&", ddns.update_freq);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "http_port=%d&", ddns.http_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "rtsp_port=%d&", ddns.rtsp_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ddnsurl=%s&", ddns.ddnsurl);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_network_set_network_ddns(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ddns ddns = {0};
    struct req_set_sysconf sys;

    memset(&sys, 0, sizeof(struct req_set_sysconf));
    snprintf(sys.arg, sizeof(sys.arg), "ddns");

    memset(&ddns, 0, sizeof(struct ddns));
    read_ddns(SQLITE_FILE_NAME, &ddns);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        ddns.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "domain=", sValue, sizeof(sValue))) {
        snprintf(ddns.domain, sizeof(ddns.domain), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(ddns.username, sizeof(ddns.username), "%s", sValue);
    }
    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(ddns.password, sizeof(ddns.password), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "host_name=", sValue, sizeof(sValue))) {
        snprintf(ddns.host_name, sizeof(ddns.host_name), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "free_dns_hash=", sValue, sizeof(sValue))) {
        snprintf(ddns.free_dns_hash, sizeof(ddns.free_dns_hash), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "update_freq=", sValue, sizeof(sValue))) {
        ddns.update_freq = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "http_port=", sValue, sizeof(sValue))) {
        ddns.http_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "rtsp_port=", sValue, sizeof(sValue))) {
        ddns.rtsp_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "ddnsurl=", sValue, sizeof(sValue))) {
        snprintf(ddns.ddnsurl, sizeof(ddns.ddnsurl), "%s", sValue);
    }

    write_ddns(SQLITE_FILE_NAME, &ddns);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_NETWORK, &sys, sizeof(struct req_set_sysconf), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_NETWORK_DDNS, 0, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_network_get_network_pppoe(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct pppoe pppoe = {0};

    memset(&pppoe, 0x0, sizeof(struct pppoe));
    read_pppoe(SQLITE_FILE_NAME, &pppoe);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", pppoe.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "auto_connect=%d&", pppoe.auto_connect);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "username=%s&", pppoe.username);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "password=%s&", pppoe.password);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_network_set_network_pppoe(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct pppoe pppoe = {0};
    struct req_set_sysconf sys;

    memset(&pppoe, 0x0, sizeof(struct pppoe));
    memset(&sys, 0, sizeof(struct req_set_sysconf));
    snprintf(sys.arg, sizeof(sys.arg), "%s", "pppoe");
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        pppoe.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "auto_connect=", sValue, sizeof(sValue))) {
        pppoe.auto_connect = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(pppoe.username, sizeof(pppoe.username), "%s", sValue);
    }
    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(pppoe.password, sizeof(pppoe.password), "%s", sValue);
    }

    write_pppoe(SQLITE_FILE_NAME, &pppoe);
    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_NETWORK, &sys, sizeof(struct req_set_sysconf), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_NETWORK_PPPOE, 0, 0, NULL, 0);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_network_set_network_restart(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_network_set_enable_p2p(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct p2p_info info = {0};
    char sValue[256] = {0};

    info.enable = 1;
    info.region = (!sdkp2p_get_section_info(buf, "region=", sValue, sizeof(sValue)) ? atoi(sValue) : MS_INVALID_VALUE);
    write_p2p_info(SQLITE_FILE_NAME, &info);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct p2p_info), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_network_set_disable_p2p(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct p2p_info info = {0};
    char sValue[256] = {0};

    info.enable = 0;
    info.region = (!sdkp2p_get_section_info(buf, "region=", sValue, sizeof(sValue)) ? atoi(sValue) : MS_INVALID_VALUE);
    write_p2p_info(SQLITE_FILE_NAME, &info);
    sdkp2p_send_msg(conf, conf->req, &info, sizeof(struct p2p_info), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void resp_network_get_p2p_status_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_p2p_info *resp = (struct resp_p2p_info *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "enable=%d&", resp->enable);
    snprintf(buf + strlen(buf), len - strlen(buf), "p2pstatus=%d&", resp->p2p_status);
    snprintf(buf + strlen(buf), len - strlen(buf), "cloudAccount=%s&", resp->cloudAccount);
    snprintf(buf + strlen(buf), len - strlen(buf), "cloudServer=%s&", resp->cloudServer);
    snprintf(buf + strlen(buf), len - strlen(buf), "registerCode=%s&", resp->registerCode);
    snprintf(buf + strlen(buf), len - strlen(buf), "region=%d&", resp->region);

    *datalen = strlen(buf) + 1;
}

static void req_network_ip_conflict_by_dev(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ip_conflict conflict;

    memset(&conflict, 0, sizeof(struct ip_conflict));
    if (!sdkp2p_get_section_info(buf, "dev=", sValue, sizeof(sValue))) {
        snprintf(conflict.dev, sizeof(conflict.dev), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "ip=", sValue, sizeof(sValue))) {
        snprintf(conflict.ipaddr, sizeof(conflict.ipaddr), "%s", sValue);
    }

    sdkp2p_send_msg(conf, conf->req, &conflict, sizeof(struct ip_conflict), SDKP2P_NEED_CALLBACK);
}

static void resp_network_ip_conflict_by_dev(void *param, int size, char *buf, int len, int *datalen)
{
    //int
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r=%d&", res);

    *datalen = strlen(buf) + 1;
}

static void req_network_get_upnp(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct upnp upnp = {0};
    struct network_more network_more = {0};

    read_upnp(SQLITE_FILE_NAME, &upnp);
    read_port(SQLITE_FILE_NAME, &network_more);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable=%d&", upnp.enable);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "type=%d&", upnp.type);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "external_http_port=%d&", upnp.http_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "http_status=%d&", upnp.http_status);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "external_rtsp_port=%d&", upnp.rtsp_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "rtsp_status=%d&", upnp.rtsp_status);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "http_port=%d&", network_more.http_port);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "rtsp_port=%d&", network_more.rtsp_port);

    *(conf->len) = strlen(conf->resp) + 1;
}

static void req_network_set_upnp(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct upnp upnp = {0};

    read_upnp(SQLITE_FILE_NAME, &upnp);
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        upnp.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        upnp.type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "external_http_port=", sValue, sizeof(sValue))) {
        upnp.http_port = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "external_rtsp_port=", sValue, sizeof(sValue))) {
        upnp.rtsp_port = atoi(sValue);
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_SET_UPNP, &upnp, sizeof(struct upnp), SDKP2P_NOT_CALLBACK);
    sdkp2p_common_write_log(conf, MAIN_OP, SUB_OP_CONFIG_REMOTE, SUB_PARAM_NETWORK, 0, 0, NULL, 0);

    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void resp_network_get_ddns_status(void *param, int size, char *buf, int len, int *datalen)
{
    int resp = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "status=%d&", resp);
    *datalen = strlen(buf) + 1;
}

static void req_network_p2p_unbind_device(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    sdkp2p_send_msg(conf, conf->req, NULL, 0, SDKP2P_NEED_CALLBACK);
}

static void resp_network_p2p_unbind_device(void *param, int size, char *buf, int len, int *datalen)
{
    int res = *(int *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", res);
    *datalen = strlen(buf) + 1;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_NETWORK_PARAM, req_network_get_network, -1, NULL},//get.network.general
    {REQUEST_FLAG_SET_NETWORK, req_network_set_network, -1, NULL},//set.network.general
    {REQUEST_FLAG_GET_NETWORK_MORE, req_network_get_network_more, -1, NULL},//get.network.port
    {REQUEST_FLAG_SET_NETWORK_MORE, req_network_set_network_more, -1, NULL},//set.network.port
    {REQUEST_FLAG_GET_NETWORK_MAIL, req_network_get_network_email, -1, NULL},//get.network.email
    {REQUEST_FLAG_SET_NETWORK_MAIL, req_network_set_network_email, -1, NULL},//set.network.email
    {REQUEST_FLAG_TEST_MAIL, req_network_set_test_email, RESPONSE_FLAG_TEST_MAIL, resp_network_set_test_email},//test.network.email
    {REQUEST_FLAG_GET_NETWORK_DDNS, req_network_get_network_ddns, -1, NULL},//get.network.ddns
    {REQUEST_FLAG_SET_NETWORK_DDNS, req_network_set_network_ddns, -1, NULL},//set.network.ddns
    {REQUEST_FLAG_GET_NETWORK_PPPOE, req_network_get_network_pppoe, -1, NULL},//get.network.pppoe
    {REQUEST_FLAG_SET_NETWORK_PPPOE, req_network_set_network_pppoe, -1, NULL},//set.network.pppoe
    {REQUEST_FLAG_SET_NETWORK_RESTART, req_network_set_network_restart, -1, NULL},//set.network.restart
    {REQUEST_FLAG_P2P_STATUS_INFO, NULL, RESPONSE_FLAG_P2P_STATUS_INFO, resp_network_get_p2p_status_info},//get p2p status
    {REQUEST_FLAG_ENABLE_P2P, req_network_set_enable_p2p, RESPONSE_FLAG_ENABLE_P2P, resp_network_set_enable_p2p},//set.network.p2p enable
    {REQUEST_FLAG_DISABLE_P2P, req_network_set_disable_p2p, -1, NULL},//set.network.p2p disable
    {REQUEST_FLAG_IP_CONFLICT_BY_DEV, req_network_ip_conflict_by_dev, RESPONSE_FLAG_IP_CONFLICT_BY_DEV, resp_network_ip_conflict_by_dev},//get.network.ipconflict
    {REQUEST_FLAG_P2P_GET_UPNP, req_network_get_upnp, -1, NULL},//get.network.upnp
    {REQUEST_FLAG_P2P_SET_UPNP, req_network_set_upnp, -1, NULL},//set.network.upnp
    {REQUEST_FLAG_GET_DDNS_STATUS, NULL, RESPONSE_FLAG_GET_DDNS_STATUS, resp_network_get_ddns_status},//get.network.ddns_status
    {REQUEST_FLAG_P2P_UNBIND_IOT_DEVICE, req_network_p2p_unbind_device, RESPONSE_FLAG_P2P_UNBIND_IOT_DEVICE, resp_network_p2p_unbind_device},//set.network.p2p_unbind_device
    // request register array end
};


void p2p_network_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_network_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

