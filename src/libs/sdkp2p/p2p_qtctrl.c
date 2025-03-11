#include "sdk_util.h"


static void req_qt_send_cmd(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    if (sdkp2p_get_section_info(buf, "cmd=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    sdkp2p_send_msg(conf, conf->req, sValue, sizeof(sValue), SDKP2P_NEED_CALLBACK);
    return ;
}

static void resp_qt_send_cmd(void *param, int size, char *buf, int len, int *datalen)
{
    struct get_qt_cmd_result *info = (struct get_qt_cmd_result *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%s&", info->result);
    snprintf(buf + strlen(buf), len - strlen(buf), "cmd=%s&", info->cmd);
    snprintf(buf + strlen(buf), len - strlen(buf), "page=%s&", info->curPage);
    snprintf(buf + strlen(buf), len - strlen(buf), "live_view_wnd_num=%d&", info->live_view_wnd_num);
    snprintf(buf + strlen(buf), len - strlen(buf), "play_back_wnd_num=%d&", info->play_back_wnd_num);
    snprintf(buf + strlen(buf), len - strlen(buf), "live_view_select_wnd=%d&", info->live_view_select_wnd);
    snprintf(buf + strlen(buf), len - strlen(buf), "play_back_select_wnd=%d&", info->play_back_select_wnd);
    snprintf(buf + strlen(buf), len - strlen(buf), "live_view_fullScreenWnd=%d&", info->live_view_fullScreenWnd);
    snprintf(buf + strlen(buf), len - strlen(buf), "play_back_fullScreenWnd=%d&", info->play_back_fullScreenWnd);
    snprintf(buf + strlen(buf), len - strlen(buf), "playbackspeed=%s&", info->playbackspeed);
    snprintf(buf + strlen(buf), len - strlen(buf), "playbackmode=%s&", info->playbackmode);
    snprintf(buf + strlen(buf), len - strlen(buf), "playbackdate=%s&", info->playbackdate);
    snprintf(buf + strlen(buf), len - strlen(buf), "playbacktime=%s&", info->playbacktime);
    *datalen = strlen(buf) + 1;

    return ;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_SENDTO_QT_CMD, req_qt_send_cmd, RESPONSE_FLAG_GET_QT_CMD_RESULT, resp_qt_send_cmd},//set.qtctrl.cmd
    // request register array end
};


void p2p_qtctrl_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_qtctrl_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

