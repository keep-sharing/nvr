#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#define __USE_GNU
#include "recortsp.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "linkedlists.h"
#include <sys/prctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include "reco_rtsp_common.h"
#include "osa/osa_mutex.h"
#include "osa/osa_thr.h"
#include "memcpy_neon.h"
#include "parse_sps.h"
#include "XD_Stream.h"
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <arpa/inet.h>
#include "assert.h"
#include "ms_rtsp_client_stream.h"
#include "msstd.h"

#define MAX_FRAME_SIZE      20000
#define RECV_TIMEOUT        8
#define DEFAULT_RTSP_PORT   554
#define DEFAULT_HTTPS_PORT   443
#define ALIGN_UP(x, a)      ((x+a-1)&(~(a-1)))

typedef enum {
    VIDCODEC_TYPE_H264,
    VIDCODEC_TYPE_MPEG4,
    VIDCODEC_TYPE_MJPEG,
    VIDCODEC_TYPE_MPEG2,
    VIDCODEC_TYPE_HEVC,
    VIDCODEC_TYPE_MAX,
} VIDCODEC_TYPE;

typedef enum {
    AUDCODEC_TYPE_G711,
    AUDCODEC_TYPE_AAC,
} AUDCODEC_TYPE;

struct uri {
    char header[8];
    char host[128];
    char port[8];
    char sourcepath[256];
};

struct av_io_state {
    int normal_exit;
    int time_out;
    int exit;   //是否退出
};

struct rtsp_event {
    int event_stream_index;
    struct evsmsg emsg;
};

struct rtsp_metadata {
    int size;
    char data[1024];
};

struct rtsp_spsinfo {
    char data[1024];
    int size;
    int sps_width;
    int sps_height;
};

struct RtspTimestamp {
    int64_t pts;
    int64_t usec;
    int32_t rate;
};

//rtsp client list
struct reco_rtsp_client {
    struct rtsp_client_args cprm;
    int exec;                       //线程是否执行
    int dosleep;                    //线程是否休眠
    int state;                      /* 客户端当前状态 */
    int immediate_recon;            /* 是否立即连接 */

    int channel_id;
    int mainorsub;
    int vs_width;                   /* 视频帧宽 */
    int vs_height;                  /* 视频帧高 */
    int vs_fps;
    int vs_bitrate;

    int as_sample_rate;             /* 采样率 */
    int as_bitrate;                 /* 比特率 */

    int video_stream_index;
    int audio_stream_index;

    VIDCODEC_TYPE vs_codec;         /* 视频编码格式 */
    AUDCODEC_TYPE as_codec;         /* 音频编码格式 */

    struct rtsp_event evs;          //报警部分
    struct rtsp_spsinfo sps;
    ms_stream_rtspclient_handler *rtsp_handler;

    struct RtspTimestamp rtspTimestamp[MAX_MS_StreamStrmType];
    int64_t lastUsec;
    int timesChangedCnt;
};

#if 0
static int cpu0_bitrate_val = 0;
static int cpu1_bitrate_val = 0;
static int cpu2_bitrate_val = 0;
static int cpu3_bitrate_val = 0;

static int set_thread_realtime(pthread_t thread_id)
{
    struct sched_param param;
    int policy = SCHED_RR;
    int priority = 70;
    if (!thread_id) {
        return -1;
    }
    memset(&param, 0, sizeof(param));
    param.sched_priority = priority;
    if (pthread_setschedparam(thread_id, policy, &param) < 0) {
        perror("pthread_setschedparam");
    }
    pthread_getschedparam(thread_id, &policy, &param);
    if (param.sched_priority != priority) {
        return -1;
    }

    return 0;
}

static int set_thread_name(const char *name)
{
    if (!name) {
        return -1;
    }
    prctl(PR_SET_NAME, name, 0, 0, 0);

    return 0;
}
#endif

static int ms_inet_aton(const char *str, struct in_addr *add)
{
    return inet_aton(str, add);
}

/* resolve host with also IP address parsing */
static int resolve_host(struct in_addr *sin_addr, const char *hostname)
{
    if (!ms_inet_aton(hostname, sin_addr)) {
#if HAVE_GETADDRINFO
        struct addrinfo *ai, *cur;
        struct addrinfo hints = { 0 };
        hints.ai_family = AF_INET;
        if (getaddrinfo(hostname, NULL, &hints, &ai)) {
            return -1;
        }
        /* getaddrinfo returns a linked list of addrinfo structs.
         * Even if we set ai_family = AF_INET above, make sure
         * that the returned one actually is of the correct type. */
        for (cur = ai; cur; cur = cur->ai_next) {
            if (cur->ai_family == AF_INET) {
                *sin_addr = ((struct sockaddr_in *)cur->ai_addr)->sin_addr;
                freeaddrinfo(ai);
                return 0;
            }
        }
        freeaddrinfo(ai);
        return -1;
#else
        int i;
        int num[4];
        char pchar[4];
        struct sockaddr_in6 addr;
        if (hostname[0] == '\0') {
            return -1;
        }

        if (strchr(hostname, ':')) {
            if (strchr(hostname, '.')) {
                return -1;
            }
            if (inet_pton(AF_INET6, hostname, &addr.sin6_addr) == 1) {
                return 0;
            }

            return -1;
        } else {

            if (sscanf(hostname, "%d%c%d%c%d%c%d%c",
                       &num[0], &pchar[0], &num[1], &pchar[1],
                       &num[2], &pchar[2], &num[3], &pchar[3]) == 7) {
                for (i = 0; i < 3; i++) {
                    if (pchar[i] != '.') {
                        return -1;
                    }
                    for (i = 0; i < 4; i++) {
                        if (num[i] > 255 || num[i] < 0) {
                            return -1;
                        }
                    }
                    return 0;
                }
            }
        }
#endif
    }
    return 0;
}

static int check_hostname_valid(const char *hostname)
{
    if (hostname[0] == 0) {
        return 0;
    } else {
        if (inet_addr(hostname) != INADDR_NONE) {
            return 1;
        }
        struct in_addr addr;

        return resolve_host(&addr, hostname) == 0 ? 1 : 0;
    }
}

static inline void get_event_window(struct evsmsg *outmsg, const char *msg)
{
    char *start, *curr;
    curr = strstr(msg, "Name=\"window\"");
    if (!curr) {
        return;
    }
    start = curr;
    while (start - msg > 0) {
        if (*start == '<' || *start == '>') {
            break;
        }
        start -= 1;
    }
    curr = strstr(start, "Value=");
    if (!curr) {
        return;
    }
    sscanf(curr, "Value=\"%d\"", &outmsg->window_num);
}

static inline void get_event_detail(struct evsmsg *outmsg, char *msg)
{
    outmsg->evs_detail = EVS_MOTION;
    return;
}

static char dec2hex(short int c)
{
    if (0 <= c && c <= 9) {
        return c + '0';
    } else if (10 <= c && c <= 15) {
        return c + 'A' - 10;
    } else {
        return -1;
    }
}

static void rtsp_get_url_encode(const char *url, const int strSize, char *res, const int resultSize)
{
    int i = 0, j = 0;
    int res_len = 0;
    char c = '\0';

    for (i = 0; i < strSize; ++i) {
        c = url[i];
        if (('0' <= c && c <= '9') ||
            ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z')) {
            if (res_len < resultSize - 1) {
                res[res_len++] = c;
            } else {
                break;
            }
        } else {
            j = (short int)c;
            if (j < 0) {
                j += 256;
            }
            int i1, i0;
            i1 = j / 16;
            i0 = j - i1 * 16;
            if (res_len + 3 < resultSize - 1) {
                res[res_len++] = '%';
                res[res_len++] = dec2hex(i1);
                res[res_len++] = dec2hex(i0);
            } else {
                break;
            }
        }
    }

    res[res_len] = '\0';
    return;
}

int reco_rtsp_get_client_state(void *hdl)
{
    if (hdl == NULL) {
        return -1;
    }

    struct reco_rtsp_client *cli = (struct reco_rtsp_client *)(hdl);
    int state = cli->state;

    return state;
}

//流报警接口
int reco_rtsp_client_event(void *rtsphdl, struct evsmsg *outmsg)
{
    struct reco_rtsp_client *rhdl = (struct reco_rtsp_client *)(rtsphdl);

    memcpy(outmsg, &rhdl->evs.emsg, sizeof(struct evsmsg));
    memset(&rhdl->evs.emsg, 0, sizeof(struct evsmsg));

    return 0;
}

//暂停/不暂停 rtsp client
int reco_rtsp_client_dosleep(void *hdl, int dosleep)
{
    if (hdl == NULL) {
        return -1;
    }
    struct reco_rtsp_client *cli = (struct reco_rtsp_client *)(hdl);

    cli->dosleep = dosleep;
    ms_rtsp_client_dosleep(cli->rtsp_handler, dosleep);
    rtsp_client_log(TSAR_INFO, "cli:%p, dosleep:%d dosleep success!", cli, dosleep);

    return 0;
}

//生成rtsp地址
int reco_rtsp_make_client_url(char *url, int urlsize, const char *addr, int port, const char *user, const char *pass,
                              const char *resname)
{
    if (!url || !addr || port < 0) {
        return -1;
    }
    if (!user || user[0] == 0 || !pass) {
        if (strchr(addr, ':')) {
            snprintf(url, urlsize, "rtsp://%s:%d/%s", addr, port, resname);
        } else {
            snprintf(url, urlsize, "rtsp://[%s]:%d/%s", addr, port, resname);
        }
    } else {
        if (strchr(addr, '.')) {
            snprintf(url, urlsize, "rtsp://%s:%s@%s:%d/%s", user, pass, addr, port, resname);
        } else {
            snprintf(url, urlsize, "rtsp://%s:%s@[%s]:%d/%s", user, pass, addr, port, resname);
        }
    }

    return 0;
}

//改变状态
static inline void change_client_state(struct reco_rtsp_client *cli, int state)
{
    if (state == cli->state) {
        return;
    }

    //设置状态信息
    struct rtsp_client_state_info info = {0};
    info.width  = cli->vs_width;
    info.height = cli->vs_height;
    info.fps    = cli->vs_fps;
    info.sfmt   = cli->cprm.streamfmt;
    info.codec  = cli->vs_codec;

    info.as_codec = cli->as_codec;
    info.as_sample_rate = cli->as_sample_rate;
    info.as_bitrate = cli->as_bitrate;
    if (state == RTSP_CLIENT_RESOLUTION_CHANGE) {
        cli->state = RTSP_CLIENT_CONNECT;
    } else {
        cli->state  = state;
    }
    if (cli->state == RTSP_CLIENT_CONNECT) {
        ms_rtsp_client_get_extradate(cli->rtsp_handler, info.extradata, &info.extradata_size);
    }
    cli->cprm.cb_client_state(state, cli->cprm.arg, &info);

    char *str = NULL;
    switch (state) {
        case RTSP_CLIENT_DISCONNECT:
            str = "RTSP_CLIENT_DISCONNECT";
            break;
        case RTSP_CLIENT_NULL:
            str = "RTSP_CLIENT_NULL";
            break;
        case RTSP_CLIENT_WAITING:
            str = "RTSP_CLIENT_WAITING";
            break;
        case RTSP_CLIENT_CONNECT:
            str = "RTSP_CLIENT_CONNECT";
            break;
        case RTSP_CLIENT_RESOLUTION_CHANGE:
            str = "RTSP_CLIENT_RESOLUTION_CHANGE";
            break;
    }
    rtsp_client_log(TSAR_INFO, "notify rtsp client:%p state:%s for chanid:%d mainsub:%c sfmt:%d [%d*%d]", cli, str, cli->cprm.chanid,
                      cli->cprm.mainsub ? 'm' : 's', info.sfmt, info.width, info.height);
}

static inline void ms_copy_frame(struct reco_frame *dstframe, struct Ms_StreamPacket *srcpkt,
                                 struct reco_rtsp_client *client)
{
    dstframe->frame_type = srcpkt->i_flags ? FT_IFRAME : FT_PFRAME;
    INT64 usec = reco_rtsp_timestamp();
    struct RtspTimestamp *rtspTimestamp = NULL;
    int isVideoAudioData = 0;
    char value[64] = {0};
    
    switch (srcpkt->i_codec) {
        case MS_StreamCodecType_H264:
        case MS_StreamCodecType_H265:
        case MS_StreamCodecType_MJPEG:
            dstframe->strm_type = ST_VIDEO;
            dstframe->width = client->vs_width;
            dstframe->height = client->vs_height;
            dstframe->frame_rate = client->vs_fps;
            dstframe->codec_type = client->vs_codec;
            rtspTimestamp = &client->rtspTimestamp[MS_StreamStrmType_VIDEO];
            isVideoAudioData = 1;
            break;
            
        case MS_StreamCodecType_G711_ULAW:
        case MS_StreamCodecType_G711_ALAW:
        case MS_StreamCodecType_AAC:
        case MS_StreamCodecType_G722:
        case MS_StreamCodecType_G726:
            isVideoAudioData = 1;
            dstframe->strm_type = ST_AUDIO;
            dstframe->codec_type = client->as_codec;
            dstframe->sample = client->as_sample_rate ? client->as_sample_rate : 8000;
            dstframe->bps = client->as_bitrate ? client->as_bitrate : 64000;
            dstframe->achn = 1;
            rtspTimestamp = &client->rtspTimestamp[MS_StreamStrmType_AUDIO];
            break;
            
        case MS_StreamCodecType_ONVIF_METADATA:
            if (trim_char((char *)srcpkt->data_buffer, srcpkt->data_size, '\0') == -1) {
                return ;
            }
            get_xml_section((char *)srcpkt->data_buffer, "MOTION", value);
            if ((xml_true_value(value, 0) == 1 && judge_motion((char *)srcpkt->data_buffer, "MOTION") == 0) ||
                (xml_true_value(value, 1) == 1)) {
                client->evs.emsg.have_event = 1;
            } else if ((xml_false_value(value, 0) == 1 && judge_motion((char *)srcpkt->data_buffer, "MOTION") == 0)) {
                client->evs.emsg.have_event = 0;
                client->evs.emsg.evs_detail = 0;
            }
            if (!client->evs.emsg.have_event) {
                return;
            }
            unsigned char lastchar = srcpkt->data_buffer[srcpkt->data_size - 1];
            srcpkt->data_buffer[srcpkt->data_size - 1] = 0x00;
            get_event_detail(&client->evs.emsg, (char *)srcpkt->data_buffer);
            srcpkt->data_buffer[srcpkt->data_size - 1] = lastchar;
            break;
            
        case MS_StreamCodecType_MS_METADATA:
            dstframe->strm_type = MS_METADATA;
            rtspTimestamp = &client->rtspTimestamp[MS_StreamStrmType_METAD];
            break;
        
        default:
            rtsp_client_log(TSAR_DEBUG,"unknow codec type. %d url:%s", srcpkt->i_codec, client->cprm.rtspurl); 
            break;
    }

    dstframe->stream_format = client->cprm.streamfmt;
    dstframe->time_lower = (int32_t)((srcpkt->i_pts) & 0xFFFFFFFF);
    dstframe->time_upper = (int32_t)((srcpkt->i_pts) >> 32);
#if 0
    if (rtspTimestamp && rtspTimestamp->rate) {
        if (srcpkt->i_pts == 0 || !rtspTimestamp->usec) {
            dstframe->time_usec = usec;
        } else {
            dstframe->time_usec = rtspTimestamp->usec + 1000000 * (srcpkt->i_pts - rtspTimestamp->pts) / rtspTimestamp->rate;
        }
        rtspTimestamp->pts = srcpkt->i_pts;
        rtspTimestamp->usec = dstframe->time_usec;
        if (isVideoAudioData) {
            //音视频独立计算pts可能导致时间戳前移
            if (dstframe->time_usec < client->lastUsec) {
                //printf("### %s sync times. %llu ###\n", dstframe->strm_type == ST_AUDIO?"audio":"video", client->lastUsec - dstframe->time_usec);
                dstframe->time_usec = client->lastUsec;
                rtspTimestamp->usec = dstframe->time_usec;
            }
            client->lastUsec = dstframe->time_usec;
            if (dstframe->strm_type == ST_VIDEO) {
                //时间偏差600秒重新校准时间
                if ((client->lastUsec > (usec + 600000000)) || (client->lastUsec < (usec - 600000000))) {
                    client->timesChangedCnt++;
                    rtsp_log(TSAR_DEBUG, "client->timesChangedCnt++ %d %llu %llu %lld", client->timesChangedCnt,
                        client->lastUsec, usec, client->lastUsec - usec);
                    if (client->timesChangedCnt >= 5) {
                        //滤波
                        client->timesChangedCnt = 0;
                        client->rtspTimestamp[MS_StreamStrmType_VIDEO].usec = usec;
                        client->rtspTimestamp[MS_StreamStrmType_AUDIO].usec = usec;
                        client->rtspTimestamp[MS_StreamStrmType_METAD].usec = usec;
                        client->lastUsec = usec;
                    }
                } else {
                    client->timesChangedCnt = 0;
                }
            }
        }
    } else {
        dstframe->time_usec = usec;
    }
#else
    dstframe->time_usec = usec;
#endif

    dstframe->size = srcpkt->data_size;
    dstframe->data = srcpkt->data_buffer;
}

static void frame_data_callback(struct Ms_StreamPacket *packet, void *opaque)
{
    struct reco_rtsp_client *cli = (struct reco_rtsp_client *)(opaque);
    struct reco_frame frame = {0};
    ms_copy_frame(&frame, packet, cli);
    frame.sdk_alarm = cli->evs.emsg.have_event;

    //for delete event data stream
    if (frame.data && packet->i_codec != MS_StreamCodecType_ONVIF_METADATA) {
        cli->cprm.recvcb(cli->cprm.arg, &frame);
    }

    return;
}

static void event_data_callback(MS_StreamEventType event_type, struct Ms_StreamInfo *stream_info, void *opaque)
{
    struct reco_rtsp_client *cli = (struct reco_rtsp_client *)(opaque);
    if (event_type == MS_StreamEventType_WAITTING) {
        change_client_state(cli, RTSP_CLIENT_WAITING);
    } else if (event_type == MS_StreamEventType_CONNECT || event_type == MS_StreamEventType_CODECCHANGE) {
        cli->vs_fps = cli->cprm.fps;
        cli->vs_codec = stream_info->video_codec == MS_StreamCodecType_H264 ? VIDCODEC_TYPE_H264 : VIDCODEC_TYPE_HEVC;
        cli->vs_width = stream_info->video_width;
        cli->vs_height = stream_info->video_height;

        if (event_type == MS_StreamEventType_CONNECT) {
            cli->rtspTimestamp[MS_StreamStrmType_VIDEO].rate = stream_info->videoSample;
            if (stream_info->videoSample) {
                //确保音视频同时有效
                cli->rtspTimestamp[MS_StreamStrmType_AUDIO].rate = stream_info->audio_sample;
            }
            cli->rtspTimestamp[MS_StreamStrmType_METAD].rate = stream_info->metaSample;
            rtsp_log(TSAR_DEBUG, "sample video:%d audio:%d meta:%d", stream_info->videoSample, stream_info->audio_sample, stream_info->metaSample);
        }
        switch (stream_info->audio_codec) {
            case MS_StreamCodecType_G722:
                cli->as_codec = AUDIO_CODEC_G722;
                break;
            case MS_StreamCodecType_G726:
                cli->as_codec = AUDIO_CODEC_G726;
                break;
            case MS_StreamCodecType_AAC:
                cli->as_codec = AUDIO_CODEC_AAC;
                break;
            case MS_StreamCodecType_G711_ALAW:
                cli->as_codec = AUDIO_CODEC_G711_ALAW;
                break;
            case MS_StreamCodecType_G711_ULAW:
            default:
                cli->as_codec = AUDIO_CODEC_G711_MULAW;
                break;
        }
        cli->as_sample_rate = stream_info->audio_sample;

        rtsp_client_log(TSAR_DEBUG, "[RTSP_CLIENT] cb chanid:%d fps:%d codec:%d %d*%d event_type:%d rate:%d %d %d", \
                          cli->cprm.chanid, cli->vs_fps, cli->vs_codec, cli->vs_width, cli->vs_height, event_type,
                          stream_info->videoSample, stream_info->audio_sample, stream_info->metaSample);

        if (event_type == MS_StreamEventType_CONNECT) {
            change_client_state(cli, RTSP_CLIENT_CONNECT);
        } else {
            change_client_state(cli, RTSP_CLIENT_RESOLUTION_CHANGE);
        }
    } else if (event_type == MS_StreamEventType_BANDWIDTH) {
        change_client_state(cli, RTSP_CLIENT_BANDWIDTH);
    } else if (event_type == MS_StreamEventType_UNAUTH) {
        change_client_state(cli, RTSP_CLIENT_UNAUTH);
    } else if (event_type == MS_StreamEventType_NETWORK) {
        change_client_state(cli, RTSP_CLIENT_NETWORK);
    } else {
        change_client_state(cli, RTSP_CLIENT_DISCONNECT);
    }

    return ;
}

static int ms_stream_rtsp_client_open(struct reco_rtsp_client *cli)
{
    if (cli == NULL) {
        return -1;
    }

    struct uri tmpuri;
    char tname[16] = {0};
    char rtspuri[512] = {0};
    char encode_psw[128] = {0};

    int chnid  = cli->cprm.chanid;
    int mainSub = cli->cprm.mainsub;

    memset(&tmpuri, 0, sizeof(struct uri));
    if (!check_hostname_valid(cli->cprm.hostname)) {
        rtsp_client_log(TSAR_ERR, "invalid hostname:%s", cli->cprm.hostname);
        return -1;
    }

    if (cli->cprm.port <= 0 || cli->cprm.port > 65535) {
        cli->cprm.port = cli->cprm.trans_protocol == TRANSPROTOCOL_ROH? DEFAULT_HTTPS_PORT : DEFAULT_RTSP_PORT;
    }

    if (cli->cprm.add_protocol != 1) {
        snprintf(tmpuri.header, sizeof(tmpuri.header), "rtsp://");
        if (strchr(cli->cprm.hostname, '.')) {
            snprintf(tmpuri.host, sizeof(tmpuri.host), "%s", cli->cprm.hostname);
        } else {
            snprintf(tmpuri.host, sizeof(tmpuri.host), "[%s]", cli->cprm.hostname);
        }
        snprintf(tmpuri.port, sizeof(tmpuri.port), "%d", cli->cprm.port);
        snprintf(tmpuri.sourcepath, sizeof(tmpuri.sourcepath), "%s", cli->cprm.respath);
        rtsp_get_url_encode(cli->cprm.password, strlen(cli->cprm.password), encode_psw, sizeof(encode_psw));
        reco_rtsp_make_client_url(rtspuri, sizeof(rtspuri), cli->cprm.hostname, cli->cprm.port, cli->cprm.username, encode_psw,
                                  cli->cprm.respath);
    } else {
        strncpy(rtspuri, cli->cprm.rtspurl, sizeof(rtspuri));
    }

    snprintf(tname, sizeof(tname), "rcli_%02d_%c", chnid, (mainSub ? 'm' : 's'));
    cli->rtsp_handler = ms_rtsp_client_create(rtspuri, tname,cli->cprm.respath, cli->cprm.trans_protocol, cli);
    if (cli->rtsp_handler) {
        ms_rtsp_client_setDataCallback(cli->rtsp_handler, frame_data_callback);
        ms_rtsp_client_setEventCallback(cli->rtsp_handler, event_data_callback);
        ms_rtsp_client_openstream(cli->rtsp_handler);
    } else {
        return -1;
    }

    return 0;
}

void *reco_rtsp_client_create(struct rtsp_client_args *desc)
{
    if (!desc || !desc->recvcb || !desc->cb_client_state) {
        return NULL;
    }
    rtsp_client_log(TSAR_INFO, "rtsp add client for chanid:%d mainsub:%c size:%d",  desc->chanid, desc->mainsub ? 'm' : 's',
                      sizeof(struct reco_rtsp_client));

    struct reco_rtsp_client *cli = ms_malloc(sizeof(struct reco_rtsp_client));
    if (!cli) {
        return NULL;
    }

    memset(cli, 0, sizeof(struct reco_rtsp_client));
    memcpy(&cli->cprm, desc, sizeof(struct rtsp_client_args));

    //创建线程
    cli->exec = 1;
    cli->channel_id = desc->chanid;
    cli->mainorsub = desc->mainsub;

    //初始化
    cli->state = RTSP_CLIENT_WAITING;
    ms_stream_rtsp_client_open(cli);

    return cli;
}

int reco_rtsp_client_destroy(void *hdl)
{
    if (hdl == NULL) {
        return -1;
    }

    struct reco_rtsp_client *cli = (struct reco_rtsp_client *)(hdl);
    int chanid  = cli->cprm.chanid;
    int mainsub = cli->cprm.mainsub;
    time_t tmStart = 0, tmEnd = 0;

    tmStart = time(0);
    rtsp_client_log(TSAR_INFO, "chanid:%d mainsub:%c hdl:%p start.\n",  chanid, mainsub ? 'm' : 's', hdl);
    if (cli->exec) {
        cli->exec = 0;
        ms_rtsp_client_dosleep(cli->rtsp_handler, 1);
        ms_rtsp_client_destroy(cli->rtsp_handler);
    }

    ms_free(cli);
    tmEnd = time(0);
    rtsp_client_log(TSAR_INFO, "chanid:%d mainsub:%c hdl:%p timeVal:%ld end\n", chanid, mainsub ? 'm' : 's', hdl, (tmEnd - tmStart));

    return 0;
}
