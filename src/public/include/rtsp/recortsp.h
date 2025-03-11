#ifndef _MS_RTSP_H_
#define _MS_RTSP_H_

#include "msdefs.h"
#if 1
    #define MAX_SESSION_FRAME_CNT 50        //maylong 20 -> 50
    #define MAX_SEND_BAND_WIDTH 268435456   // 256*1024*1024
    #define MAX_RTSP_SERVER_CONN (256)      //
#else
    #define MAX_SESSION_FRAME_CNT 50        //50
    #define MAX_SEND_BAND_WIDTH 335544320   //320*1024*1024
    #define MAX_RTSP_SERVER_CONN (256)      //256
#endif

//server
struct rtsp_user_info {
    char username[33];
    char password[MAX_PWD_LEN + 1];
    char password_ex[MAX_PWD_LEN + 1];
    int remoteLiveviewAudio;
    char remoteLiveviewChannel[MAX_LEN_65];
    int remotePlaybackAudio;
    char remotePlaybackChannel[MAX_LEN_65];
    int type;
};

enum NET_INDEX {
    NET_INDEX_BOND = 0,
    NET_INDEX_LAN0,
    NET_INDEX_LAN1,
};

struct rtsp_network {
    int enable;
    int dhcp;
    char ip_addr[64];
    char netmask[64];
    char gateway[64];
    char dns1[64];
    char dns2[64];
};

struct rtsp_netinfo {
    int mode;
    struct rtsp_network net[MAX_DEVICE_NUM];
};

typedef enum RtspCbReqNo
{
    RTSP_REQ_DEFAULT_RESP = -100,
    RTSP_REQ_NONE = 0,
    RTSP_REQ_OPEN_PB,
    RTSP_REQ_CLOSE_PB,
    RTSP_REQ_MAX,
}RTSP_CB_REQ_NO;

struct RtspCbReq
{
    RTSP_CB_REQ_NO req;
    char buf[128];
};

struct RtspCbResp
{
    char buf[128];
};

typedef int (*CbRtspServerReq)(struct RtspCbReq *req, struct RtspCbResp *resp);

struct reco_rtsp_server_conf {
    char username[64];
    char password[64];
    char bindaddr[16];
    int server_port;
    int connection_timeout;
    int max_connections;
    int rtp_port_min;
    int rtp_port_max;
    int send_on_key;
    int max_bitrate;
    int max_conn;
    struct rtsp_netinfo rtsp_net;
    int (*cb_user_req)(struct RtspCbReq *req, struct RtspCbResp *resp);
};

enum rtsp_server_state {
    RTSP_SERVER_CLOSED = -1,
    RTSP_SERVER_NULL,
    RTSP_SERVER_WAITING,
    RTSP_SERVER_OPENED,
    RTSP_SERVER_NOCLIENT,
};

typedef struct rtsp_server_args {
	int chnid;
    int timeout;
    char name[128];
    int width;
    int height;
    int audioon;
    int codec;
    int bandwidth;
    int acodec;
    int samplerate;
    int maxDataSize;//http转发队列最大值
    int (*cb_server_state)(int state, void *arg);
    void *arg;
    char extradata[MAX_LEN_256];
    int extradata_size;
}RTSP_SERVER_ARG;
//end server

//client
enum rtsp_client_state {
    RTSP_CLIENT_CODEC_DIFF = -5,
    RTSP_CLIENT_CODEC_INIT = -4,
    RTSP_CLIENT_CODEC_CHANED = -3,
    RTSP_CLIENT_NORESOURCE = -2,
    RTSP_CLIENT_DISCONNECT = -1,
    RTSP_CLIENT_NULL,
    RTSP_CLIENT_WAITING,
    RTSP_CLIENT_CONNECT,
    RTSP_CLIENT_RESOLUTION_CHANGE,
    RTSP_CLIENT_BANDWIDTH,
    RTSP_CLIENT_UNAUTH,
    RTSP_CLIENT_NETWORK,
};

struct rtsp_client_state_info {
    int width;
    int height;
    int fps;
    int sfmt;
    int codec;

    ///david.milesight modfiy
    int as_codec;
    int as_sample_rate;
    int as_bitrate;

    char extradata[MAX_LEN_256];
    int extradata_size;
};

struct rtsp_client_args {
    char hostname[64];
    char username[64];
    char password[64];
    char respath[256];
    int  chanid;
    int  mainsub;
    int  tindex;
    int  port;
    TRANSPROTOCOL trans_protocol;
    int (*recvcb)(void *arg, struct reco_frame *frame);
    int (*cb_client_state)(int state, void *arg, struct rtsp_client_state_info *info);
    void *arg;
    int streamfmt;
    int fps;
    int bitrate;
    int nvr_code;
    int ipc_model;
    int add_protocol;
    char rtspurl[256];
};

typedef struct rtsp_jpeg_info {
    void *data;
    int size;
    int width;
    int height;
} RTSP_JPEG_INFO;

//end client

struct rtsp_server_buffer {
    int loaded;
    void (*rtsp_show)();
    void (*rtsp_init)(int bitrate);
    void (*rtsp_uninit)();
    void (*rtsp_malloc)(void **dst, int size);
    void (*rtsp_calloc)(void **dst, int n, int size);
    void (*rtsp_free)(void *src, int size);
};
typedef enum {
    IMGFMT_JPEG = 0,
    IMGFMT_BMP,
} IMGFMT;

#ifndef __EVSMSG
#define __EVSMSG
typedef enum {
    EVS_UNKNOW = 0,
    EVS_MOTION,
} EVENT_TYPE;

struct evsmsg {
    int have_event;
    int window_num;
    EVENT_TYPE evs_detail;
};
#endif

typedef enum {
    CONN_RTSP = 0,
    CONN_HTTP,
} CONN_TYPE;

#define MS_HTTP_CONN        "GET /ms/streaming"
#define MS_HTTP_VERSION     "HTTP/1.1"

void reco_rtsp_init();
void reco_rtsp_uninit();
void reco_rtsp_set_debug(int type, int on);// set 0:server 1:client, 2:all debug 3:ffmpeg
void reco_rtsp_set_print(void *print);
void reco_rtsp_server_set(struct reco_rtsp_server_conf *conf);


void *reco_rtsp_server_create(struct rtsp_server_args *desc);
int reco_rtsp_server_set_user(struct rtsp_user_info user_info[RTSP_MAX_USER]);
int reco_rtsp_server_destroy(void *hdl);
int reco_rtsp_server_senddata(void *hdl, struct reco_frame *frame);
void reco_rtsp_server_showstreams();
int reco_rtsp_server_ctx_count(void *hdl);
int reco_rtsp_server_set_bandwidth(void *hdl, int bandwidth);


void *reco_rtsp_client_create(struct rtsp_client_args *desc);
int reco_rtsp_client_destroy(void *cli);
int reco_rtsp_get_client_state(void *cli);
int reco_rtsp_client_event(void *rtsphdl, struct evsmsg *outmsg);
int reco_rtsp_client_dosleep(void *hdl, int dosleep);
int reco_rtsp_make_client_url(char *url, int urlsize, const char *addr, int port, const char *user, const char *pass,
                              const char *resname);

void reco_cli_output(char *format, ...);
void *memcpy_neon(void *dst, const void *src, size_t n);

int reco_rtsp_savejpg(const char *jpg_file, struct reco_frame *frame, int quality, int depth);
int reco_rtsp_create_jpeg_file(const char *outfile, struct reco_frame *frame, int quality);
int reco_rtsp_create_jpeg_data(RTSP_JPEG_INFO *jpegData, struct reco_frame *frame, int quality);

void get_rtsp_version(char *pVer, int pLen);

void reco_rtsp_show_framedata(struct reco_frame *frame);

#if 0
    /*audio to decoder g722 | g726*/

    int reco_rtsp_open_g72x(void **handle, int codec_type);
    void reco_rtsp_close_g72x();

    unsigned int reco_rtsp_g722_decode(short *dst, const unsigned char *src, unsigned int srcSize);
    unsigned int reco_rtsp_g726_decode(short *dst, const unsigned char *src, unsigned int srcSize);
#endif

int reco_rtsp_update_access_filter(struct access_filter *conf);
void reco_rtsp_server_set_buffer(struct rtsp_server_buffer *buffer);

void reco_rtsp_set_multicast(int enable, char *ip);
void reco_rtsp_server_show_channel(char *name);



#endif//_MS_RTSP_H
