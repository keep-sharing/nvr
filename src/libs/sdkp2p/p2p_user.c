#include "sdk_util.h"


static void req_user_get_info(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_auth *res = (struct req_auth *)param;
    char sValue[256] = {0};
    if (!sdkp2p_get_section_info(buf, "r.user=", sValue, sizeof(sValue))) {
        snprintf(res->user, sizeof(res->user), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "r.password=", sValue, sizeof(sValue))) {
        snprintf(res->passwd, sizeof(res->passwd), "%s", sValue);
    }
    *datalen = sizeof(struct req_auth);
}

static void resp_user_get_info(void *param, int size, char *buf, int len, int *datalen)
{
    struct db_user *res = (struct db_user *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "r.id=%d&", res->id);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.enable=%d&", res->enable);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.username=%s&", res->username);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.password=%s&", res->password);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.type=%d&", res->type);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.permission=%d&", res->permission);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.remote_permission=%d&", res->remote_permission);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.local_live_view=%lu&", res->local_live_view);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.local_playback=%lu&", res->local_playback);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.remote_live_view=%lu&", res->remote_live_view);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.remote_playback=%lu&", res->remote_playback);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.local_live_view_ex=%s&", res->local_live_view_ex);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.local_playback_ex=%s&", res->local_playback_ex);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.remote_live_view_ex=%s&", res->remote_live_view_ex);

    snprintf(buf + strlen(buf), len - strlen(buf), "r.remote_playback_ex=%s&", res->remote_playback_ex);
    snprintf(buf + strlen(buf), len - strlen(buf), "r.password_ex=%s&", res->password_ex);

    *datalen = strlen(buf) + 1;
}

static void req_user_get_user_member(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int cnt = 0, i = 0;
    struct db_user user[MAX_USER];

    memset(user, 0, sizeof(user));
    read_users(SQLITE_FILE_NAME, user, &cnt);
    for (i = 0; i < cnt; i++) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "id[%d]=%d&", i, user[i].id);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "enable[%d]=%d&", i, user[i].enable);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "username[%d]=%s&", i, user[i].username);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "password[%d]=%s&", i, user[i].password);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "type[%d]=%d&", i, user[i].type);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permission[%d]=%d&", i, user[i].permission);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remote_permission[%d]=%d&", i,
                 user[i].remote_permission);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "local_live_view[%d]=%ld&", i,
                 user[i].local_live_view);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "local_playback[%d]=%ld&", i,
                 user[i].local_playback);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remote_live_view[%d]=%ld&", i,
                 user[i].remote_live_view);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remote_playback[%d]=%ld&", i,
                 user[i].remote_playback);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "local_live_view_ex[%d]=%s&", i,
                 user[i].local_live_view_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "local_playback_ex[%d]=%s&", i,
                 user[i].local_playback_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remote_live_view_ex[%d]=%s&", i,
                 user[i].remote_live_view_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remote_playback_ex[%d]=%s&", i,
                 user[i].remote_playback_ex);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalLive[%d]=%d&", i,user[i].perm_local_live);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalPlayback[%d]=%d&", i,user[i].perm_local_playback);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalRetrieve[%d]=%d&", i,user[i].perm_local_retrieve);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalSmart[%d]=%d&", i,user[i].perm_local_smart);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalEvent[%d]=%d&", i,user[i].perm_local_event);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalCamera[%d]=%d&", i,user[i].perm_local_camera);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalStorage[%d]=%d&", i,user[i].perm_local_storage);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalSettings[%d]=%d&", i,user[i].perm_local_settings);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalStatus[%d]=%d&", i,user[i].perm_local_status);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permLocalShutdown[%d]=%d&", i,user[i].perm_local_shutdown);
        
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteLive[%d]=%d&", i,user[i].perm_remote_live);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemotePlayback[%d]=%d&", i,user[i].perm_remote_playback);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteRetrieve[%d]=%d&", i,user[i].perm_remote_retrieve);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteSmart[%d]=%d&", i,user[i].perm_remote_smart);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteEvent[%d]=%d&", i,user[i].perm_remote_event);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteCamera[%d]=%d&", i,user[i].perm_remote_camera);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteStorage[%d]=%d&", i,user[i].perm_remote_storage);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteSettings[%d]=%d&", i,user[i].perm_remote_settings);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteStatus[%d]=%d&", i,user[i].perm_remote_status);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permRemoteShutdown[%d]=%d&", i,user[i].perm_remote_shutdown);
    }

    *(conf->len) = strlen(conf->resp);
}


static void req_user_update_user(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    int i;
    struct db_user user = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        user.id = atoi(sValue);
    }
    read_user(SQLITE_FILE_NAME, &user, user.id);

    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        user.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(user.username, sizeof(user.username), "%s", sValue);
    }

    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(user.password, sizeof(user.password), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        user.type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permission=", sValue, sizeof(sValue))) {
        user.permission = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "remote_permission=", sValue, sizeof(sValue))) {
        user.remote_permission = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "local_live_view=", sValue, sizeof(sValue))) {
        user.local_live_view = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "local_playback=", sValue, sizeof(sValue))) {
        user.local_playback = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "remote_live_view=", sValue, sizeof(sValue))) {
        user.remote_live_view = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "remote_playback=", sValue, sizeof(sValue))) {
        user.remote_playback = atol(sValue);
    }


    if (!sdkp2p_get_section_info(buf, "local_live_view_ex=", sValue, sizeof(sValue))) {
        snprintf(user.local_live_view_ex, sizeof(user.local_live_view_ex), "%s", sValue);
        for (i = 0; i < strlen(user.local_live_view_ex); i++) {
            if (user.local_live_view_ex[i] == '1') {
                user.local_live_view |= (1 << i);
            } else {
                user.local_live_view &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.local_live_view & (1 << i)) {
                user.local_live_view_ex[i] = '1';
            } else {
                user.local_live_view_ex[i] = '0';
            }
        }
    }
    if (!sdkp2p_get_section_info(buf, "local_playback_ex=", sValue, sizeof(sValue))) {
        snprintf(user.local_playback_ex, sizeof(user.local_playback_ex), "%s", sValue);
        for (i = 0; i < strlen(user.local_playback_ex); i++) {
            if (user.local_playback_ex[i] == '1') {
                user.local_playback |= (1 << i);
            } else {
                user.local_playback &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.local_playback & (1 << i)) {
                user.local_playback_ex[i] = '1';
            } else {
                user.local_playback_ex[i] = '0';
            }
        }
    }
    if (!sdkp2p_get_section_info(buf, "remote_live_view_ex=", sValue, sizeof(sValue))) {
        snprintf(user.remote_live_view_ex, sizeof(user.remote_live_view_ex), "%s", sValue);
        for (i = 0; i < strlen(user.remote_live_view_ex); i++) {
            if (user.remote_live_view_ex[i] == '1') {
                user.remote_live_view |= (1 << i);
            } else {
                user.remote_live_view &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.remote_live_view & (1 << i)) {
                user.remote_live_view_ex[i] = '1';
            } else {
                user.remote_live_view_ex[i] = '0';
            }
        }
    }
    if (!sdkp2p_get_section_info(buf, "remote_playback_ex=", sValue, sizeof(sValue))) {
        snprintf(user.remote_playback_ex, sizeof(user.remote_playback_ex), "%s", sValue);
        for (i = 0; i < strlen(user.remote_playback_ex); i++) {
            if (user.remote_playback_ex[i] == '1') {
                user.remote_playback |= (1 << i);
            } else {
                user.remote_playback &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.remote_playback & (1 << i)) {
                user.remote_playback_ex[i] = '1';
            } else {
                user.remote_playback_ex[i] = '0';
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "permLocalLive=", sValue, sizeof(sValue))) {
        user.perm_local_live = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalPlayback=", sValue, sizeof(sValue))) {
        user.perm_local_playback = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalRetrieve=", sValue, sizeof(sValue))) {
        user.perm_local_retrieve = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalSmart=", sValue, sizeof(sValue))) {
        user.perm_local_smart = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalEvent=", sValue, sizeof(sValue))) {
        user.perm_local_event = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalCamera=", sValue, sizeof(sValue))) {
        user.perm_local_camera = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalStorage=", sValue, sizeof(sValue))) {
        user.perm_local_storage = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalSettings=", sValue, sizeof(sValue))) {
        user.perm_local_settings = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalStatus=", sValue, sizeof(sValue))) {
        user.perm_local_status = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalShutdown=", sValue, sizeof(sValue))) {
        user.perm_local_shutdown = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "permRemoteLive=", sValue, sizeof(sValue))) {
        user.perm_remote_live = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemotePlayback=", sValue, sizeof(sValue))) {
        user.perm_remote_playback = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteRetrieve=", sValue, sizeof(sValue))) {
        user.perm_remote_retrieve = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteSmart=", sValue, sizeof(sValue))) {
        user.perm_remote_smart = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteEvent=", sValue, sizeof(sValue))) {
        user.perm_remote_event = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteCamera=", sValue, sizeof(sValue))) {
        user.perm_remote_camera = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteStorage=", sValue, sizeof(sValue))) {
        user.perm_remote_storage = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteSettings=", sValue, sizeof(sValue))) {
        user.perm_remote_settings = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteStatus=", sValue, sizeof(sValue))) {
        user.perm_remote_status = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteShutdown=", sValue, sizeof(sValue))) {
        user.perm_remote_shutdown = atoi(sValue);
    }

    write_user(SQLITE_FILE_NAME, &user);
    
    sdkp2p_send_msg(conf, REQUEST_FLAG_P2P_SET_USER, &conf->p2psid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_user_get_user_auth(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    char auth_user[64] = {0}, auth_passwd[64] = {0};
    int ret = -1;
    struct db_user db = { 0 };

    if (!sdkp2p_get_section_info(buf, "auth_user=", sValue, sizeof(sValue))) {
        snprintf(auth_user, sizeof(auth_user), "%s", sValue);
    }
    if (!sdkp2p_get_section_info(buf, "auth_password=", sValue, sizeof(sValue))) {
        snprintf(auth_passwd, sizeof(auth_passwd), "%s", sValue);
    }

    do {
        if (read_user_by_name(SQLITE_FILE_NAME, &db, auth_user)) {
            //msprintf("read user: %s error\n", auth_user);
            break;
        }
        if (!db.enable) {
            //msprintf("user is disabled\n");
            break;
        }
        if (strcmp(db.password, auth_passwd)) {
            //msprintf("password error, db.password: %s, auth.passwd: %s\n",db.password, auth_passwd);
            break;
        }
        ret = 0;
    } while (0);

    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "ret=%d&", ret);
    if (ret == 0) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "permisson=%d&", db.permission);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remotePermisson=%d&", db.remote_permission);

        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "localliveview=%s&", db.local_live_view_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "localplayback=%s&", db.local_playback_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remoteliveview=%s&",
                 db.remote_live_view_ex);
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "remoteplayback=%s&", db.remote_playback_ex);
    }
    *(conf->len) = strlen(conf->resp);
}

static void req_user_get_user_count(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int cnt = 0;

    read_user_count(SQLITE_FILE_NAME, &cnt);
    snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "cnt=%d&", cnt);
    *(conf->len) = strlen(conf->resp);
}


static void req_user_add_user(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i;
    char sValue[256] = {0};
    struct db_user user = {0};

    if (!sdkp2p_get_section_info(buf, "id=", sValue, sizeof(sValue))) {
        user.id = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "enable=", sValue, sizeof(sValue))) {
        user.enable = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(user.username, sizeof(user.username), "%s", sValue);
    }
    if (!sdkp2p_get_special_info(buf, "password=", sValue, sizeof(sValue), "psw_len=")) {
        get_url_decode(sValue);
        snprintf(user.password, sizeof(user.password), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "passwordEx=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(user.password_ex, sizeof(user.password_ex), "%s", sValue);
    }
    
    if (!sdkp2p_get_section_info(buf, "type=", sValue, sizeof(sValue))) {
        user.type = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permission=", sValue, sizeof(sValue))) {
        user.permission = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "remote_permission=", sValue, sizeof(sValue))) {
        user.remote_permission = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "local_live_view=", sValue, sizeof(sValue))) {
        user.local_live_view = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "local_playback=", sValue, sizeof(sValue))) {
        user.local_playback = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "remote_live_view=", sValue, sizeof(sValue))) {
        user.remote_live_view = atol(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "remote_playback=", sValue, sizeof(sValue))) {
        user.remote_playback = atol(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "local_live_view_ex=", sValue, sizeof(sValue))) {
        snprintf(user.local_live_view_ex, sizeof(user.local_live_view_ex), "%s", sValue);
        for (i = 0; i < strlen(user.local_live_view_ex); i++) {
            if (user.local_live_view_ex[i] == '1') {
                user.local_live_view |= (1 << i);
            } else {
                user.local_live_view &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.local_live_view & (1 << i)) {
                user.local_live_view_ex[i] = '1';
            } else {
                user.local_live_view_ex[i] = '0';
            }
        }
    }
    if (!sdkp2p_get_section_info(buf, "local_playback_ex=", sValue, sizeof(sValue))) {
        snprintf(user.local_playback_ex, sizeof(user.local_playback_ex), "%s", sValue);
        for (i = 0; i < strlen(user.local_playback_ex); i++) {
            if (user.local_playback_ex[i] == '1') {
                user.local_playback |= (1 << i);
            } else {
                user.local_playback &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.local_playback & (1 << i)) {
                user.local_playback_ex[i] = '1';
            } else {
                user.local_playback_ex[i] = '0';
            }
        }
    }
    if (!sdkp2p_get_section_info(buf, "remote_live_view_ex=", sValue, sizeof(sValue))) {
        snprintf(user.remote_live_view_ex, sizeof(user.remote_live_view_ex), "%s", sValue);
        for (i = 0; i < strlen(user.remote_live_view_ex); i++) {
            if (user.remote_live_view_ex[i] == '1') {
                user.remote_live_view |= (1 << i);
            } else {
                user.remote_live_view &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.remote_live_view & (1 << i)) {
                user.remote_live_view_ex[i] = '1';
            } else {
                user.remote_live_view_ex[i] = '0';
            }
        }
    }
    if (!sdkp2p_get_section_info(buf, "remote_playback_ex=", sValue, sizeof(sValue))) {
        snprintf(user.remote_playback_ex, sizeof(user.remote_playback_ex), "%s", sValue);
        for (i = 0; i < strlen(user.remote_playback_ex); i++) {
            if (user.remote_playback_ex[i] == '1') {
                user.remote_playback |= (1 << i);
            } else {
                user.remote_playback &= ~(1 << i);
            }
        }
    } else {
        for (i = 0; i < 32; i++) {
            if (user.remote_playback & (1 << i)) {
                user.remote_playback_ex[i] = '1';
            } else {
                user.remote_playback_ex[i] = '0';
            }
        }
    }

    if (!sdkp2p_get_section_info(buf, "permLocalLive=", sValue, sizeof(sValue))) {
        user.perm_local_live = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalPlayback=", sValue, sizeof(sValue))) {
        user.perm_local_playback = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalRetrieve=", sValue, sizeof(sValue))) {
        user.perm_local_retrieve = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalSmart=", sValue, sizeof(sValue))) {
        user.perm_local_smart = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalEvent=", sValue, sizeof(sValue))) {
        user.perm_local_event = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalCamera=", sValue, sizeof(sValue))) {
        user.perm_local_camera = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalStorage=", sValue, sizeof(sValue))) {
        user.perm_local_storage = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalSettings=", sValue, sizeof(sValue))) {
        user.perm_local_settings = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalStatus=", sValue, sizeof(sValue))) {
        user.perm_local_status = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permLocalShutdown=", sValue, sizeof(sValue))) {
        user.perm_local_shutdown = atoi(sValue);
    }

    if (!sdkp2p_get_section_info(buf, "permRemoteLive=", sValue, sizeof(sValue))) {
        user.perm_remote_live = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemotePlayback=", sValue, sizeof(sValue))) {
        user.perm_remote_playback = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteRetrieve=", sValue, sizeof(sValue))) {
        user.perm_remote_retrieve = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteSmart=", sValue, sizeof(sValue))) {
        user.perm_remote_smart = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteEvent=", sValue, sizeof(sValue))) {
        user.perm_remote_event = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteCamera=", sValue, sizeof(sValue))) {
        user.perm_remote_camera = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteStorage=", sValue, sizeof(sValue))) {
        user.perm_remote_storage = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteSettings=", sValue, sizeof(sValue))) {
        user.perm_remote_settings = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteStatus=", sValue, sizeof(sValue))) {
        user.perm_remote_status = atoi(sValue);
    }
    if (!sdkp2p_get_section_info(buf, "permRemoteShutdown=", sValue, sizeof(sValue))) {
        user.perm_remote_shutdown = atoi(sValue);
    }

    snprintf(user.pattern_psw, sizeof(user.pattern_psw), "0");
    add_user(SQLITE_FILE_NAME, &user);
    sdkp2p_send_msg(conf, REQUEST_FLAG_P2P_SET_USER, &conf->p2psid, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}

static void req_user_del_user(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct db_user user = {0};
    char sValue[256] = {0};

    memset(&user, 0x0, sizeof(struct db_user));
    if (!sdkp2p_get_section_info(buf, "userid=", sValue, sizeof(sValue))) {
        user.id = atoi(sValue);
        write_user(SQLITE_FILE_NAME, &user);
    }

    sdkp2p_send_msg(conf, REQUEST_FLAG_P2P_SET_USER, &conf->p2psid, sizeof(int), SDKP2P_NOT_CALLBACK);
    sdkp2p_send_msg(conf, REQUEST_FLAG_P2P_DEL_USER, &user.id, sizeof(int), SDKP2P_NOT_CALLBACK);
    *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_OK, conf->resp, conf->size);
}


static void req_user_get_sqa_list(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    int i = 0, ret = 0;
    char encode[MAX_LEN_64 * 9];
    struct squestion sqas[MAX_SQA_CNT];

    memset(sqas, 0, sizeof(struct squestion)*MAX_SQA_CNT);
    ret = read_encrypted_list(SQLITE_FILE_NAME, sqas);
    if (ret) {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "res=%d&", -1);
    } else {
        snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sqa_enable=%d&", sqas[0].enable);
        for (i = 0; i < MAX_SQA_CNT; i++) {
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sqa_type_%d=%d&", i, sqas[i].sqtype);
            get_url_encode(sqas[i].squestion, strlen(sqas[i].squestion), encode, sizeof(encode));
            snprintf(conf->resp + strlen(conf->resp), conf->size - strlen(conf->resp), "sqa_question_%d=%s&", i, encode);
        }
    }

    *(conf->len) = strlen(conf->resp);
}

static void req_user_change_password(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_change_password *change = (struct req_change_password *)param;
    char sValue[512] = {0};

    if (!sdkp2p_get_section_info(buf, "username=", sValue, sizeof(sValue))) {
        snprintf(change->username, sizeof(change->username), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "username_ex=", sValue, sizeof(sValue))) {
        snprintf(change->username_ex, sizeof(change->username_ex), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "password=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(change->password, sizeof(change->password), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "password_ex=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(change->password_ex, sizeof(change->password_ex), "%s", sValue);
    }

    if (!sdkp2p_get_section_info(buf, "key=", sValue, sizeof(sValue))) {
        snprintf(change->key, sizeof(change->key), "%s", sValue);
    }

    change->p2psid = conf->p2psid;
    *datalen = sizeof(struct req_change_password);
    sdkp2p_send_msg(conf, conf->req, change, *datalen, SDKP2P_NEED_CALLBACK);
}

static void resp_user_change_password(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_change_password *resp = (struct resp_change_password *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%s&", resp->res);
    *datalen = strlen(buf) + 1;
}

static void req_user_key_tool(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    struct req_key_tool *key = (struct req_key_tool *)param;
    char sValue[1024] = {0};

    if (!sdkp2p_get_section_info(buf, "pub_key=", sValue, sizeof(sValue))) {
        get_url_decode(sValue);
        snprintf(key->pub_key, sizeof(key->pub_key), "%s", sValue);
    }

    *datalen = sizeof(struct req_key_tool);
    sdkp2p_send_msg(conf, conf->req, key, *datalen, SDKP2P_NEED_CALLBACK);
}

static void resp_user_key_tool(void *param, int size, char *buf, int len, int *datalen)
{
    struct resp_key_tool *resp = (struct resp_key_tool *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "res=%d&", resp->res);
    snprintf(buf + strlen(buf), len - strlen(buf), "key_id=%s&", resp->key_id);
    snprintf(buf + strlen(buf), len - strlen(buf), "key=%s&", resp->key);
    *datalen = strlen(buf) + 1;
}

static void resp_status_get_online_user(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_online_user_info *res = (struct ms_online_user_info *)param;
    int num = size / sizeof(struct ms_online_user_info);
    int i;

    snprintf(buf + strlen(buf), len - strlen(buf), "num=%d&", num);
    for (i = 0; i < num; i++) {
        snprintf(buf + strlen(buf), len - strlen(buf), "userName%d=%s&", i, res[i].userName);
        snprintf(buf + strlen(buf), len - strlen(buf), "userLevel%d=%d&", i, res[i].userLevel);
        snprintf(buf + strlen(buf), len - strlen(buf), "ipaddr%d=%s&", i, res[i].ipaddr);
        snprintf(buf + strlen(buf), len - strlen(buf), "loginTime%d=%s&", i, res[i].loginTime);
    }
    *datalen = strlen(buf) + 1;
}

static void req_status_set_online_user(char *buf, int len, void *param, int *datalen, struct req_conf_str *conf)
{
    char sValue[256] = {0};
    struct ms_online_user_action oInfo;
    memset(&oInfo, 0x0, sizeof(struct ms_online_user_action));

    if (sdkp2p_get_section_info(buf, "action=", sValue, sizeof(sValue))) {
        *(conf->len) = sdkp2p_get_resp(SDKP2P_RESP_ERROR, conf->resp, conf->size);
        return ;
    }

    oInfo.action = atoi(sValue);

    snprintf(oInfo.ipaddr, sizeof(oInfo.ipaddr), "%s", conf->ip_addr);
    snprintf(oInfo.userName, sizeof(oInfo.userName), "%s", conf->user);

    if (!sdkp2p_get_section_info(buf, "sessionId=", sValue, sizeof(sValue))) {
        oInfo.sessionId = (Uint64)atoll(sValue);
    }

    sdkp2p_send_msg(conf, conf->req, (void *)&oInfo, sizeof(oInfo), SDKP2P_NEED_CALLBACK);
}

static void resp_status_set_online_user(void *param, int size, char *buf, int len, int *datalen)
{
    struct ms_online_user_action *res = (struct ms_online_user_action *)param;

    snprintf(buf + strlen(buf), len - strlen(buf), "sessionId=%llu&", res->sessionId);

    *datalen = strlen(buf) + 1;
}
// request function define end

static struct translate_request_p2p sP2pResApplyChanges[] = {
    {REQUEST_FLAG_GET_USER_MEMBER, req_user_get_user_member, -1, NULL},//get.user.member
    {REQUEST_FLAG_USER_UPDATE, req_user_update_user, -1, NULL},//set.user.member
    {REQUEST_FLAG_GET_USER_AUTH, req_user_get_user_auth, -1, NULL},//get.user.auth
    {REQUEST_FLAG_GET_USER_COUNT, req_user_get_user_count, -1, NULL},//get.user.count
    {REQUEST_FLAG_SET_ADD_USER, req_user_add_user, -1, NULL},//add.user.member
    {REQUEST_FLAG_SET_DEL_USER, req_user_del_user, -1, NULL},//del.user.member
    {REQUEST_FLAG_GET_SQA_LIST, req_user_get_sqa_list, -1, NULL},//get.sqa.list
    {REQUEST_FLAG_CHANGE_PASSWORD, req_user_change_password, RESPONSE_FLAG_CHANGE_PASSWORD, resp_user_change_password},//http://%s:%s@%s:%d/modifypwd?%s
    {REQUEST_FLAG_KEY_TOOL, req_user_key_tool, RESPONSE_FLAG_KEY_TOOL, resp_user_key_tool},//http://@%s:%d/challenge?PublicKeytool=%s

    {REQUEST_FLAG_GET_USERINFO, req_user_get_info, RESPONSE_FLAG_GET_USERINFO, resp_user_get_info},

    {REQUEST_FLAG_GET_ONLINE_USER, NULL, RESPONSE_FLAG_GET_ONLINE_USER, resp_status_get_online_user}, //get.user.online_user
    {REQUEST_FLAG_SET_ONLINE_USER, req_status_set_online_user, RESPONSE_FLAG_SET_ONLINE_USER, resp_status_set_online_user}, //set.user.online_user
    // request register array end
};

void p2p_user_load_module()
{
    p2p_request_register(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

void p2p_user_unload_module()
{
    p2p_request_unregister(sP2pResApplyChanges, sizeof(sP2pResApplyChanges) / sizeof(struct translate_request_p2p));
}

