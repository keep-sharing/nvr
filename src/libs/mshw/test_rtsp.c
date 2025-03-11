#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>

#include "recortsp.h"

struct GLOBAL_RTSP {
    int tindex;
};

struct ms_rtsp {
    int chan_id;
    int state_client;
    char ip_addr[64];

    int state_server;
    void *handle_client;
    void *handle_server;
};

static struct ms_rtsp g_rtsp_info[128] = {{0}};

static struct GLOBAL_RTSP g_rtsp = {0};

extern int rtsp_write_data(struct reco_frame *frames, int capability);

void reco_cli_output(char *format, ...)
{
    char buf[512] = {0};
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);
    /*if (access("/tmp/.recocli_started", F_OK)  == 0){
    recoipc_send_msg(IPC_TYPE_CLI, 0, sizeof(buf), buf);
    }*/
    return;
}

void *memcpy_neon(void *dst, const void *src, size_t n)
{
    return memcpy(dst, src, n);
}

//static int g_frame_cnt = 0;
static int demo_callback_recv_rtsp(void *arg, struct reco_frame *frame)
{
    //if(frame->width==0 || frame->height == 0)
//  printf("resolution(%d, %d)\n", frame->width, frame->height);
    struct ms_rtsp *p_rtsp_info = (struct ms_rtsp *)arg;
    frame->ch = p_rtsp_info->chan_id;
    frame->stream_from = SST_IPC_STREAM;
    if (p_rtsp_info->handle_client && (frame->strm_type == ST_VIDEO || frame->strm_type == ST_AUDIO)) {
        rtsp_write_data(frame, 1);
    }
    return 0;
}

static int demo_callback_rtsp_client_state(int state, void *arg, struct rtsp_client_state_info *info)
{
    struct ms_rtsp *p_rtsp_info = (struct ms_rtsp *)arg;
    p_rtsp_info->state_client = state;
    if (RTSP_CLIENT_CONNECT == state) {
        printf("rtsp client state:RTSP_CLIENT_CONNECT.chan_id:%d ip:%s\n", p_rtsp_info->chan_id, (char *)p_rtsp_info->ip_addr);
    } else {
        printf("\n\nrtsp client[ip:%s] disconnected \n\n", (char *)p_rtsp_info->ip_addr);
    }
    return 0;
}

static int demo_callback_rtsp_server_state(int state, void *arg)
{
    struct ms_rtsp *p_rtsp_info = (struct ms_rtsp *)arg;
    p_rtsp_info->state_server = state;
    printf("rtsp server state:%d.chan_id:%d ip:%s\n", state, p_rtsp_info->chan_id, p_rtsp_info->ip_addr);
    return 0;
}

//#############################################################################
int ms_rtsp_create_client(int chan_id, int tcp_udp, int rtsp_port, char *ip_addr, char *username, char *password)
{
    struct rtsp_client_args param = {{0}};
    g_rtsp_info[chan_id].chan_id = chan_id;
    strcpy(g_rtsp_info[chan_id].ip_addr, ip_addr);

    param.recvcb = demo_callback_recv_rtsp;
    param.arg = &g_rtsp_info[chan_id];
    param.trans_protocol  = tcp_udp;//TRANSPROTOCOL_UDP;
    param.cb_client_state = demo_callback_rtsp_client_state;
    snprintf(param.hostname, sizeof(param.hostname), "%s", ip_addr);
    snprintf(param.username, sizeof(param.username), "%s", username);
    snprintf(param.password, sizeof(param.password), "%s", password);
    //snprintf(param.respath,  sizeof(param.respath),  "%s", respath);
    param.port = rtsp_port;
    param.chanid    = chan_id;
    param.mainsub   = 0;
    param.tindex    = g_rtsp.tindex++;
    param.streamfmt = STREAM_TYPE_MAINSTREAM;
    param.fps       = 25;
    param.bitrate   = 4096;
    param.nvr_code = CODECTYPE_H264 | CODECTYPE_H265;
    param.ipc_model = 0;
    param.add_protocol = IPC_PROTOCOL_ONVIF;
//  snprintf(param.rtspurl,  sizeof(param.rtspurl), "%s", client->url_path);
    g_rtsp_info[chan_id].handle_client = reco_rtsp_client_create(&param);

    return 0;
}

void ms_rtsp_destroy_client(int chan_id)
{
    if (g_rtsp_info[chan_id].handle_client) {
        reco_rtsp_client_dosleep(g_rtsp_info[chan_id].handle_client, 1);
        reco_rtsp_client_destroy(g_rtsp_info[chan_id].handle_client);
    }
}

static int demo_set_rtsp_server_user()
{
    int i, state;
    struct rtsp_user_info user_info[RTSP_MAX_USER];
    for (i = 0; i < RTSP_MAX_USER; i++) {
        strcpy(user_info[i].username, "admin");
        strcpy(user_info[i].password, "admin");
    }
    state = reco_rtsp_server_set_user(user_info);
    return state;
}

int ms_rtsp_create_server(int chan_id)
{
    struct rtsp_server_args desc;
    
    memset(&desc, 0, sizeof(struct rtsp_server_args));
    snprintf(desc.name, sizeof(desc.name), "ch_%d", chan_id + 100);
    printf("create rtsp server.path:%s\n", desc.name);
    desc.codec = 0;

    desc.cb_server_state = demo_callback_rtsp_server_state;
    desc.arg = &g_rtsp_info[chan_id];
    desc.timeout = -1;
    desc.audioon = 1;
    desc.maxDataSize = 3*1024*1024;
    demo_set_rtsp_server_user();
    g_rtsp_info[chan_id].handle_server = reco_rtsp_server_create(&desc);
    if (!g_rtsp_info[chan_id].handle_server) {
        printf("create rtsp server failed. path:%s\n", desc.name);
        return -1;
    }

    return 0;
}


int ms_rtsp_init(int rtsp_server_port)
{
    reco_rtsp_init();

    struct reco_rtsp_server_conf conf = {{0}};
    conf.server_port = rtsp_server_port;
    reco_rtsp_server_set(&conf);

    return 0;
}

int ms_rtsp_unit()
{
    reco_rtsp_uninit();

    return 0;
}

