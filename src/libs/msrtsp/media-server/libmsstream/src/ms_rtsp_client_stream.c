#include "ms_rtsp_client_stream.h"
#include "ms_rtp_session.h"
#include "rtsp-client.h"
#include "sockutil.h"
#include "sockpair.h"
#include "sys/thread.h"
#include "sys/system.h"
#include "ms_res_decoder.h"
#include "app-log.h"
#include <ctype.h>
#include "ms_parse_sps.h"
#include "ms_rtsp_client.h"
#include "base64.h"

#define RTSP_PROTOCOL_PREFIX            ("rtsp://")
#define RTSP_KEEPAVLIE_TIMEINTERVAL     (40000)
#define RTSP_CLIENT_TIMEOUTINTERVAL     (11)
#define MAX_RTSP_SOCKET_CONNECT_TIMES   (5000)

#if defined(_WIN32) || defined(_WIN64) || defined(OS_WINDOWS)
    #define strncasecmp _strnicmp
#endif

#ifndef min
    #define min(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef max
    #define max(x, y) ((x) > (y) ? (x) : (y))
#endif

MS_StreamErrorCode ms_stream_init()
{
    socket_init();
    app_log_setlevel(LOG_EMERG);
    return MS_StreamErrorCode_SUCCESS;
}

MS_StreamErrorCode ms_stream_uninit()
{
    socket_cleanup();
    return MS_StreamErrorCode_SUCCESS;
}

void ms_rtsp_client_stream_info_reset(ms_rtsp_client_t *context);
void ms_rtsp_client_notify_event(ms_rtsp_client_t *context, MS_StreamEventType event_type);

static int socket_send_tls_by_time(socket_t sock, SSL *ssl, const void* buf, size_t len, int timeout)
{
	int r;

	r = socket_select_write(sock, timeout);
	if (r <= 0)
		return 0 == r ? SOCKET_TIMEDOUT : r;
	assert(1 == r);

	r = SSL_write(ssl, buf, (int)len);
	return r;
}

static int socket_send_tls_all_by_time(socket_t sock, SSL *ssl, const void* buf, size_t len, int timeout)
{
	int r;
	size_t bytes = 0;

	while (bytes < len)
	{
		r = socket_send_tls_by_time(sock, ssl, (const char*)buf + bytes, len - bytes, timeout);
		if (r <= 0)
			return r;	// <0-error

		bytes += r;
	}
	return (int)bytes;
}

static int ms_rtsp_client_send(void *param, const char *uri, const void *req, size_t bytes)
{
    //TODO: check uri and make socket
    //1. uri != rtsp describe uri(user input)
    //2. multi-uri if media_count > 1
    int order = bytes;
    int result = 0;
    char message[2048] = {0};
    ms_rtsp_client_t *ctx    = (ms_rtsp_client_t *)param;
    if (ctx->transmit_protocol == MS_StreamTransportType_RTSP_OVER_HTTPS) {
        if (strstr(req, "application/x-rtsp-tunnelled") == NULL) { // rtsp over https
            bytes = base64_encode(message, req, bytes);
            //result = socket_send_all_by_time(ctx->rtsp_command_socket, message, bytes, 0, 2000);
            result = socket_send_tls_all_by_time(ctx->rtsp_command_socket, ctx->command_ssl, message, bytes, 2000);
        } else if (strstr(req, "POST") != NULL) { // POST /main HTTP/1.1
            //result = socket_send_all_by_time(ctx->rtsp_command_socket, req, bytes, 0, 2000);
            result = socket_send_tls_all_by_time(ctx->rtsp_command_socket, ctx->command_ssl, req, bytes, 2000);
        } else { // GET /main HTTP/1.1
            //result = socket_send_all_by_time(ctx->socket, req, bytes, 0, 2000);
            result = socket_send_tls_all_by_time(ctx->socket, ctx->data_ssl, req, bytes, 2000);
        }
    } else {
        result = socket_send_all_by_time(ctx->socket, req, bytes, 0, 2000);
    }
    if (result != bytes) {
        return result;
    }
    return order;
}

static void ms_rtsp_client_onrtprecv(void *param, uint8_t channel, const void *data, uint16_t bytes)
{
    //RTSP_TRANSPORT_RTP_TCP
    ms_rtsp_client_t *ctx = (ms_rtsp_client_t *)param;
    int tmp;
    
    if (channel / 2 < MAX_MEDIA_NUM) {
        if (!ctx->rtp_ctx[channel / 2]) {
            int i = 0;
            rtsp_client_debug(RTSP_INFO, "on rtp recv maybe channel do not have rtp_ctx,channel:%d\n", channel);
            for (i = 0; i < MAX_MEDIA_NUM; i++) {
                rtsp_client_debug(RTSP_INFO, "i:%d,rtp_ctx:%p\n", i, ctx->rtp_ctx[i]);
            }
            return ;
        }
        //rtsp_client_debug(RTSP_INFO, "onrtprecv ctx:%p url:%s thread_run:%d", ctx, ctx->url, ctx->rtsp_pool_thread_run);
        ms_rtp_receiver_tcp_input(ctx->rtp_ctx[channel / 2], channel, data, bytes);
    } else {
        //interleaved 值对不上? 偶数通道为流媒体内容，奇数通道为RTCP
        if (channel%2 == 1) {
            return;
        }
        tmp = channel % MAX_MEDIA_NUM;
        if (ctx->rtp_ctx[tmp]) {
            ms_rtp_receiver_tcp_input(ctx->rtp_ctx[tmp], channel, data, bytes);
        }
    }
}

static int ms_rtsp_client_onrtpport(void *param, int media, unsigned short *rtp_port)
{
    ms_rtsp_client_t *ctx = (ms_rtsp_client_t *)param;
    if (media >= MAX_MEDIA_NUM) {
        return -1;
    }

    rtsp_client_debug(RTSP_INFO, "[david debug] media:%d transport:%d", media, ctx->transport);
    switch (ctx->transport) {
        case RTSP_TRANSPORT_RTP_UDP:
            if (sockpair_create("0.0.0.0", ctx->rtp_socket[media], ctx->rtp_port[media])) {
                return -1;
            }
            *rtp_port = ctx->rtp_port[media][0];
            break;
        case RTSP_TRANSPORT_RTP_TCP:
            *rtp_port = 0;
            break;
        default:
            return -1;
    }

    return 0;
}

static int ms_rtsp_client_ondescriberesponse(void *param, const char *sdp)
{
    ms_rtsp_client_t *ctx = (ms_rtsp_client_t *)param;
    return rtsp_client_setup(ctx->rtsp_client, sdp);
}

int parse_packet(ms_rtsp_client_t *ctx, struct Ms_StreamPacket *packet)
{
    if ((packet->i_codec == MS_StreamCodecType_H264 || packet->i_codec == MS_StreamCodecType_H265) && packet->i_flags) {
        int check_resolution = 1;
        if (ctx->start_push_data) {
            if (!check_if_res_changed(packet->i_codec == MS_StreamCodecType_H264 ? 0 : 1, packet->data_buffer, packet->data_size,
                                      &ctx->decoder_ctx)) {
                check_resolution = 0;
            } else {
                rtsp_client_debug(RTSP_INFO, "resolution maybe change\n");
            }
        }
        if (check_resolution) {
            ms_parse_h2645_res(packet->i_codec == MS_StreamCodecType_H264 ? 0 : 1, packet->data_buffer, packet->data_size,
                               &ctx->decoder_ctx);
            ms_get_resolution(&ctx->decoder_ctx, &ctx->stream_info.video_width, &ctx->stream_info.video_height);
            rtsp_client_debug(RTSP_INFO, "ctx codetype:%d width:%d height:%d\n", packet->i_codec, ctx->stream_info.video_width,
                          ctx->stream_info.video_height);
            if (ctx->start_push_data) {
                if (ctx->stream_info.video_codec != MS_StreamCodecType_H264 && ctx->stream_info.video_codec != MS_StreamCodecType_H265) {
                    rtsp_client_debug(RTSP_DEBUG, "codec invalid. %d", ctx->stream_info.video_codec);
                    ctx->rtsp_pool_thread_run = 0;
                    return -1;
                }
                ms_rtsp_client_notify_event(ctx, MS_StreamEventType_CODECCHANGE);
            } else {
                ms_rtsp_client_notify_event(ctx, MS_StreamEventType_CONNECT);
            }
            ctx->start_push_data = 1;
        }
    } else {
        if (ctx->start_push_data) {
            return 0;
        } else {
            return -1;
        }
    }
    return 0;
}
static int ms_check_iframe_info(struct Ms_StreamPacket *packet)
{
    int ret = 0;
    if (!packet) {
        return -1;
    }
    if (!packet->i_flags) {
        return -1;
    }
    char startcode[4] = {0x00, 0x00, 0x00, 0x01};
    char h265_startcode[3] = {0x00, 0x00, 0x01};
    char nal;

    switch (packet->i_codec) {
        case MS_StreamCodecType_H264:
            if (packet->data_buffer && packet->data_size > 4) {
                if (!strncmp((char*)packet->data_buffer, h265_startcode, sizeof(h265_startcode))) {
                    nal = packet->data_buffer[3] & 0x1f;
                    if (nal == 0x06 || nal == 0x07 || nal == 0x08 || nal == 0x09) {
                        ret = 1;
                    }
                }
                if (!strncmp((char*)packet->data_buffer, startcode, sizeof(startcode))) {
                    nal = packet->data_buffer[4] & 0x1f;
                    if (nal == 0x06 || nal == 0x07 || nal == 0x08 || nal == 0x09) {
                        ret = 1;
                    }
                }
            }
            break;
        case MS_StreamCodecType_H265:
            if (packet->data_buffer && packet->data_size > 4) {
                if (!strncmp((char*)packet->data_buffer, h265_startcode, sizeof(h265_startcode))) {
                    if (packet->data_buffer[3] == 0x40) {
                        ret = 1;
                    }
                }
                if (!strncmp((char*)packet->data_buffer, startcode, sizeof(startcode))) {
                    if (packet->data_buffer[4] == 0x40) {
                        ret = 1;
                    }
                }
            }
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
}

static int ms_parse_rtp_extradata(ms_rtsp_client_t *ctx, struct Ms_StreamPacket *packet)
{
    //uint8_t flag;
    char startcode[4] = {0x00, 0x00, 0x00, 0x01};
    if (!ctx || !packet) {
        return -1;
    }
    if (packet->data_size > 100) {
        return -1;
    }
    if (packet->i_codec != MS_StreamCodecType_H264
        && packet->i_codec != MS_StreamCodecType_H265) {
        return -1;
    }

    //david add for datasize < 100 video type, hava extradata.
    if (packet->i_codec == MS_StreamCodecType_H264) {
        if ((ctx->rtp_extradata.size + sizeof(startcode) + packet->data_size) > sizeof(ctx->rtp_extradata.buf)) {
            //flag = packet->data_buffer[0];
            //printf("H264 extradata size error %d %d %d flag:0x%02x.\n", ctx->rtp_extradata.size, sizeof(startcode), packet->data_size, flag);
            return -1;
        }
        if ((packet->data_buffer[0] & 0x0F) == 0x07) {
            ctx->rtp_extradata.size = 0;
            memset(ctx->rtp_extradata.buf, 0, sizeof(ctx->rtp_extradata.buf));
            memcpy(ctx->rtp_extradata.buf, startcode, sizeof(startcode));
            ctx->rtp_extradata.size += sizeof(startcode);
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
            ctx->rtp_extradata.size += packet->data_size;
        } else if ((packet->data_buffer[0] & 0x0F) == 0x08) {
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, startcode, sizeof(startcode));
            ctx->rtp_extradata.size += sizeof(startcode);
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
            ctx->rtp_extradata.size += packet->data_size;
        } else if ((packet->data_buffer[0] & 0x0F) == 0x06) {
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, startcode, sizeof(startcode));
            ctx->rtp_extradata.size += sizeof(startcode);
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
            ctx->rtp_extradata.size += packet->data_size;
        } else {
            return -1;
        }
    }
    if (packet->i_codec == MS_StreamCodecType_H265) {
        if ((ctx->rtp_extradata.size + sizeof(startcode) + packet->data_size) > sizeof(ctx->rtp_extradata.buf)) {
            //flag = packet->data_buffer[0];
            //printf("H265 extradata size error %d %d %d flag:0x%02x.\n", ctx->rtp_extradata.size, sizeof(startcode), packet->data_size, flag);
            return -1;
        }
        if (packet->data_buffer[0] & 0x40) {
            ctx->rtp_extradata.size = 0;
            memset(ctx->rtp_extradata.buf, 0, sizeof(ctx->rtp_extradata.buf));
            memcpy(ctx->rtp_extradata.buf, startcode, sizeof(startcode));
            ctx->rtp_extradata.size += sizeof(startcode);
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
            ctx->rtp_extradata.size += packet->data_size;
        } else if (packet->data_buffer[0] & 0x42) {
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, startcode, sizeof(startcode));
            ctx->rtp_extradata.size += sizeof(startcode);
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
            ctx->rtp_extradata.size += packet->data_size;
        } else if (packet->data_buffer[0] & 0x44) {
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, startcode, sizeof(startcode));
            ctx->rtp_extradata.size += sizeof(startcode);
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
            ctx->rtp_extradata.size += packet->data_size;
        } else if (packet->data_buffer[0] & 0x4E) {
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, startcode, sizeof(startcode));
            ctx->rtp_extradata.size += sizeof(startcode);
            memcpy(ctx->rtp_extradata.buf + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
            ctx->rtp_extradata.size += packet->data_size;
        } else {
            return -1;
        }
    }

    return 0;
}

static void frame_packet_reciever(void *param, struct Ms_StreamPacket *packet)
{
    ms_rtsp_client_t *ctx = (ms_rtsp_client_t *)param;

    int extraflag = 0;
    struct Ms_StreamPacket tmp = {0};

    if (!packet->data_size) {
        rtsp_client_debug(RTSP_INFO, "maybe codec changed. codec:%d url:%s\n", packet->i_codec, ctx->url);
        ctx->rtsp_pool_thread_run = 0;
        return;
    }
    
    memcpy(&tmp, packet, sizeof(struct Ms_StreamPacket));
    ctx->frame_recv_flag = 1;

    if (!ms_parse_rtp_extradata(ctx, packet)) {
        return ;
    }

#if 1 // TODO: extradata导致amba IPC 4k花屏，但是直接屏蔽会导致部分第三方IPC预览异常，先恢复代码，需要另外再兼容处理
    if (!ms_check_iframe_info(packet)) {
#if 0
        if (ctx->extradata.size > 0) {
            extraflag = 1;
            tmp.data_size = packet->data_size + ctx->extradata.size;
            tmp.data_buffer = (uint8_t *)malloc(tmp.data_size);

            memcpy(tmp.data_buffer, ctx->extradata.buf, ctx->extradata.size);
            memcpy(tmp.data_buffer + ctx->extradata.size, packet->data_buffer, packet->data_size);
        }
#else
        do {
            if (ctx->rtp_extradata.size > 0) {
                extraflag = 1;
                tmp.data_size = packet->data_size + ctx->rtp_extradata.size;
                tmp.data_buffer = (uint8_t *)malloc(tmp.data_size);

                memcpy(tmp.data_buffer, ctx->rtp_extradata.buf, ctx->rtp_extradata.size);
                memcpy(tmp.data_buffer + ctx->rtp_extradata.size, packet->data_buffer, packet->data_size);
                ctx->rtp_extradata.size = 0;
                break;
            }
            if (ctx->extradata.size > 0) {
                extraflag = 1;
                tmp.data_size = packet->data_size + ctx->extradata.size;
                tmp.data_buffer = (uint8_t *)malloc(tmp.data_size);

                memcpy(tmp.data_buffer, ctx->extradata.buf, ctx->extradata.size);
                memcpy(tmp.data_buffer + ctx->extradata.size, packet->data_buffer, packet->data_size);
            }
        } while (0);
#endif
    }
#endif

    if (parse_packet(ctx, &tmp)) {
        if (extraflag) {
            free(tmp.data_buffer);
        }

        rtsp_client_debug(RTSP_INFO, "frame_packet reciever url:[%s], extraflag:[%d], recv_flag:[%d], start_push:[%d], i_flags:[%d], i_codec:[%d], data_size:[%d], i_pts:[%lld] timeout:[%d]",
                      ctx->url, extraflag, ctx->frame_recv_flag, ctx->start_push_data, tmp.i_flags, tmp.i_codec, tmp.data_size, tmp.i_pts,
                      RTSP_CLIENT_TIMEOUTINTERVAL);

        return;
    }

    if (ctx->data_callback) {
        ctx->data_callback(&tmp, ctx->param);
    }
    ctx->frame_recv_flag = 1;

    if (extraflag) {
        free(tmp.data_buffer);
    }

    return;
}

static int ms_parse_extradata(ms_rtsp_client_t *ctx, int media, MS_StreamCodecType codec_type)
{
    if (!ctx) {
        return -1;
    }

    int i = 0;
    int nspsLen = 0, nppsLen = 0, nvpsLen = 0;
    //int width, height;
    //SPS sps_info = {0};
    char startcode[4] = {0x00, 0x00, 0x00, 0x01};
    //char h265_startcode[3] = {0x00, 0x00, 0x01};
    char pExtradata[256] = {0};
    char sps_string[128] = {0};
    char pps_string[128] = {0};
    char vps_string[128] = {0};
    char sps_dec[128] = {0};
    char pps_dec[128] = {0};
    char vps_dec[128] = {0};

    rtsp_client_get_media_extradata(ctx->rtsp_client, media, pExtradata, sizeof(pExtradata));
    char *ptr = NULL;
    char *outer_ptr = NULL;
    char *inner_ptr = pExtradata;
    while ((ptr = strtok_r(inner_ptr, ",", &outer_ptr)) != NULL) {
        inner_ptr = NULL;
        if (i == 0) {
            snprintf(sps_string, sizeof(sps_string), "%s", ptr);
        } else if (i == 1) {
            snprintf(pps_string, sizeof(pps_string), "%s", ptr);
        } else if (i == 2) {
            snprintf(vps_string, sizeof(vps_string), "%s", ptr);
        }
        if (i++ > 10) {
            break;
        }
    }
    //printf("[david debug] sps_string:%s pps_string:%s vps_string:%s 1111\n", sps_string, pps_string, vps_string);
    if ((int)strlen(sps_string) > 0) {
        nspsLen = base64_decode(sps_dec, sps_string, strlen(sps_string));
    }
    if ((int)strlen(pps_string) > 0) {
        nppsLen = base64_decode(pps_dec, pps_string, strlen(pps_string));
    }
    if ((int)strlen(vps_string) > 0) {
        nvpsLen = base64_decode(vps_dec, vps_string, strlen(vps_string));
    }
    if (codec_type == MS_StreamCodecType_H264) {
        if (nspsLen > 0
            && (strlen(ctx->extradata.buf) + sizeof(startcode) + nspsLen) < sizeof(ctx->extradata.buf)) {
            memcpy(ctx->extradata.buf, startcode, sizeof(startcode));
            ctx->extradata.size += sizeof(startcode);
            memcpy(ctx->extradata.buf + ctx->extradata.size, sps_dec, nspsLen);
            ctx->extradata.size += nspsLen;
        }

        if (nppsLen > 0
            && (strlen(ctx->extradata.buf) + sizeof(startcode) + nppsLen) < sizeof(ctx->extradata.buf)) {
            memcpy(ctx->extradata.buf + ctx->extradata.size, startcode, sizeof(startcode));
            ctx->extradata.size += sizeof(startcode);
            memcpy(ctx->extradata.buf + ctx->extradata.size, pps_dec, nppsLen);
            ctx->extradata.size += nppsLen;
        }
    } else if (codec_type == MS_StreamCodecType_H265) {
        if (nvpsLen > 0
            && (strlen(ctx->extradata.buf) + sizeof(startcode) + nvpsLen) < sizeof(ctx->extradata.buf)) {
            if (vps_dec[0] != startcode[0] || vps_dec[1] != startcode[1]
                || vps_dec[2] != startcode[2] || vps_dec[3] != startcode[3]) {
                memcpy(ctx->extradata.buf, startcode, sizeof(startcode));
                ctx->extradata.size += sizeof(startcode);
            }
            memcpy(ctx->extradata.buf + ctx->extradata.size, vps_dec, nvpsLen);
            ctx->extradata.size += nvpsLen;
        }

        if (nspsLen > 0
            && (strlen(ctx->extradata.buf) + sizeof(startcode) + nspsLen) < sizeof(ctx->extradata.buf)) {
            if (sps_dec[0] != startcode[0] || sps_dec[1] != startcode[1]
                || sps_dec[2] != startcode[2] || sps_dec[3] != startcode[3]) {
                memcpy(ctx->extradata.buf + ctx->extradata.size, startcode, sizeof(startcode));
                ctx->extradata.size += sizeof(startcode);
            }
            memcpy(ctx->extradata.buf + ctx->extradata.size, sps_dec, nspsLen);
            ctx->extradata.size += nspsLen;
        }

        if (nppsLen > 0
            && (strlen(ctx->extradata.buf) + sizeof(startcode) + nppsLen) < sizeof(ctx->extradata.buf)) {
            if (pps_dec[0] != startcode[0] || pps_dec[1] != startcode[1]
                || pps_dec[2] != startcode[2] || pps_dec[3] != startcode[3]) {
                memcpy(ctx->extradata.buf + ctx->extradata.size, startcode, sizeof(startcode));
                ctx->extradata.size += sizeof(startcode);
            }
            memcpy(ctx->extradata.buf + ctx->extradata.size, pps_dec, nppsLen);
            ctx->extradata.size += nppsLen;
        }
    }
#if 0
    printf("=============ms_parse_extradata===========\n");
    for (i = 0; i < 512 && i < ctx->extradata.size; i++) {
        if (i % 63 == 0 && i != 0) {
            printf("%x\n", ctx->extradata.buf[i]);
        } else {
            printf("%x ", ctx->extradata.buf[i]);
        }
    }
    printf("\n===========ms_parse_extradata=============\n");
#endif

    return 0;
}

static int ms_rtsp_client_onsetupresponse(void *param)
{
    int i;
    uint64_t npt = 0;
    char ip[65];
    u_short rtspport;
    ms_rtsp_client_t *ctx = (ms_rtsp_client_t *)param;
    if (rtsp_client_play(ctx->rtsp_client, &npt, NULL)) {
        return -1;
    }
    for (i = 0; i < rtsp_client_media_count(ctx->rtsp_client) && i < MAX_MEDIA_NUM; i++) {
        int payload, port[2];
        const char *encoding;
        const char *media_type;
        const struct rtsp_header_transport_t *transport;
        transport = rtsp_client_get_media_transport(ctx->rtsp_client, i);
        encoding = rtsp_client_get_media_encoding(ctx->rtsp_client, i);
        payload = rtsp_client_get_media_payload(ctx->rtsp_client, i);
        media_type = rtsp_client_get_media_describption(ctx->rtsp_client, i);

        if (!transport || !media_type || !encoding) {
            rtsp_client_debug(RTSP_WARN, "invalid params. transport:%p media_type:%p encoding:%p", transport, media_type, encoding);
            return -1;
        }

        if (RTSP_TRANSPORT_RTP_UDP == transport->transport) {
            //assert(RTSP_TRANSPORT_RTP_UDP == transport->transport); // udp only
            assert(0 == transport->multicast); // unicast only
            assert(transport->rtp.u.client_port1 == ctx->rtp_port[i][0]);
            assert(transport->rtp.u.client_port2 == ctx->rtp_port[i][1]);

            port[0] = transport->rtp.u.server_port1;
            port[1] = transport->rtp.u.server_port2;
            if (*transport->source) {
                ctx->rtp_ctx[i] = ms_rtp_receiver_create(ctx->rtp_socket[i], transport->source, port, payload, encoding, ctx);
            } else {
                socket_getpeername(ctx->socket, ip, &rtspport);
                ctx->rtp_ctx[i] = ms_rtp_receiver_create(ctx->rtp_socket[i], ip, port, payload, encoding, ctx);
            }
        } else if (RTSP_TRANSPORT_RTP_TCP == transport->transport) {
            ctx->rtp_ctx[i] = ms_rtp_receiver_tcp_create(payload, encoding, ctx);
        } else {
            return -1;
        }
        //get sdp info
        if (!strcmp(media_type, "video")) {
            parse_codec_type(encoding, &ctx->stream_info.video_codec);
            ms_parse_extradata(ctx, i, ctx->stream_info.video_codec);
            ctx->stream_info.videoSample = rtsp_client_get_media_rate(ctx->rtsp_client, i);
        } else if (!strcmp(media_type, "audio")) {
            parse_codec_type(encoding, &ctx->stream_info.audio_codec);
            ctx->stream_info.audio_sample = rtsp_client_get_media_rate(ctx->rtsp_client, i);
        } else {
            ctx->stream_info.metaSample = rtsp_client_get_media_rate(ctx->rtsp_client, i);
            rtsp_client_debug(RTSP_INFO, "unknow codec:%s", encoding);
        }
        ms_rtp_session_set_callback(ctx->rtp_ctx[i], frame_packet_reciever);
    }

    return 0;
}

static int ms_rtsp_client_onplayresponse(void *param, int media, const uint64_t *nptbegin, const uint64_t *nptend,
                                         const double *scale, const struct rtsp_rtp_info_t *rtpinfo, int count)
{
    ms_rtsp_client_t *ctx = (ms_rtsp_client_t *)param;
    ctx->last_cmd_time = system_clock();
    return 0;
}

static int ms_rtsp_client_onoptionresponse(void *param)
{
    ms_rtsp_client_t *ctx = (ms_rtsp_client_t *)param;
    ctx->last_cmd_time = system_clock();
    if (rtsp_client_media_count(ctx->rtsp_client) > 0) {
        return 0;
    } else { //first time
        return rtsp_client_describe(ctx->rtsp_client);
    }
}

static int ms_rtsp_client_onpauseresponse(void *param)
{
    return 0;
}

static int ms_rtsp_client_onteardownresponse(void *param)
{
    return 0;
}

int ms_isxdigit(int c)
{
    c = tolower(c);
    return isdigit(c) || (c >= 'a' && c <= 'f');
}

static int urldecode(char *url)
{
    int s = 0, d = 0, url_len = 0;
    char c;
    char *dest = NULL;
    int tmp1, tmp2;

    if (!url) {
        return 0;
    }

    url_len = strlen(url) + 1;
    dest = url;

    while (s < url_len) {
        c = url[s++];
        if (c == '%' && s + 2 < url_len) {
            char c2 = url[s++];
            char c3 = url[s++];
            tmp1 = c2;
            tmp2 = c3;
            if (ms_isxdigit(tmp1) && ms_isxdigit(tmp2)) {
                c2 = tolower(tmp1);
                c3 = tolower(tmp2);

                if (c2 <= '9') {
                    c2 = c2 - '0';
                } else {
                    c2 = c2 - 'a' + 10;
                }

                if (c3 <= '9') {
                    c3 = c3 - '0';
                } else {
                    c3 = c3 - 'a' + 10;
                }

                dest[d++] = 16 * c2 + c3;

            } else { /* %zz or something other invalid */
                dest[d++] = c;
                dest[d++] = c2;
                dest[d++] = c3;
            }
        } else if (c == '+') {
            dest[d++] = ' ';
        } else {
            dest[d++] = c;
        }
    }
    dest[d] = '\0';
    return 1;
}

MS_StreamErrorCode parse_rtsp_url(const char *source_url, rtsp_url_t *rtsp_url)
{
    do {
        const char *p = NULL;
        const char *from = NULL;
        const char *ls = NULL;
        const char *password_start = NULL;
        const char *password_end = NULL;
        char password[128] = {0};
        //confirm url start with rtsp://
        if (strncasecmp(source_url, RTSP_PROTOCOL_PREFIX, strlen(RTSP_PROTOCOL_PREFIX)) != 0) {
            break;
        }
        from = &source_url[strlen(RTSP_PROTOCOL_PREFIX)];
        for (p = from; *p != '\0' && *p != '/'; p++) {
            if (*p == ':' && password_start == NULL) {
                password_start = p;
            } else if (*p == '@') {
                password_end = p;
                //have username:password@
                if (password_start && password_end && password_start < password_end) {
                    //username skip ':'
                    int length = min(password_start - from, sizeof(rtsp_url->username) - 1);
                    memcpy(rtsp_url->username, from, length);
                    rtsp_url->username[length] = '\0';
                    //password skip ':' and '@'
                    length = min(password_end - (password_start + 1), sizeof(password) - 1);
                    memcpy(password, password_start + 1, length);
                    //set from to skip @
                    from = p + 1;
                    break;
                }
            }
        }
        ls = strchr(from, '/');
        if (!ls) {
            ls = from + strlen(from);    // ls to '\0'
        }
        if (*from == '[' && (p = strchr(from, ']')) && p < ls) { //ipv6
            int length = min(sizeof(rtsp_url->host) - 1, p - from - 1);
            memcpy(rtsp_url->host, from + 1, length);
            rtsp_url->host[length] = '\0';
            if (p[1] == ':') {
                rtsp_url->rtsp_port = atoi(p + 2);
            } else {
                rtsp_url->rtsp_port = 554;
            }
        } else if ((p = strchr(from, ':')) && p < ls && p >= from/**/) {
            int length = min(sizeof(rtsp_url->host) - 1, p - from);
            memcpy(rtsp_url->host, from, length);
            rtsp_url->host[length] = '\0';
            rtsp_url->rtsp_port = atoi(p + 1);
        } else {
            int length = min(sizeof(rtsp_url->host) - 1, ls - from);
            memcpy(rtsp_url->host, from, length);
            rtsp_url->host[length] = '\0';
            rtsp_url->rtsp_port = 554;
        }
        snprintf(rtsp_url->url, sizeof(rtsp_url->url), "%s%s:%d%s", RTSP_PROTOCOL_PREFIX, rtsp_url->host, rtsp_url->rtsp_port,
                 ls);
        urldecode(password);
        snprintf(rtsp_url->password, sizeof(rtsp_url->password), "%s", password);
        rtsp_client_debug(RTSP_INFO, "url:%s, username:%s, password:%s, hostname:%s,port:%d\n", rtsp_url->url, rtsp_url->username,
                      rtsp_url->password, rtsp_url->host, rtsp_url->rtsp_port);
        return MS_StreamErrorCode_SUCCESS;
    } while (0);

    return MS_StreamErrorCode_FAILED;
}

struct rtsp_client_t *
ms_rtsp_client_session_create(ms_rtsp_client_t *context, rtsp_url_t *rtsp_url)
{
    struct rtsp_client_handler_t handler = {0};
    handler.send = ms_rtsp_client_send;
    handler.rtpport = ms_rtsp_client_onrtpport;
    handler.ondescribe = ms_rtsp_client_ondescriberesponse;
    handler.onsetup = ms_rtsp_client_onsetupresponse;
    handler.onplay = ms_rtsp_client_onplayresponse;
    handler.onpause = ms_rtsp_client_onpauseresponse;
    handler.onteardown = ms_rtsp_client_onteardownresponse;
    handler.onrtp = ms_rtsp_client_onrtprecv;
    handler.onoption = ms_rtsp_client_onoptionresponse;

    context->rtsp_client = rtsp_client_create(rtsp_url->url, rtsp_url->username, rtsp_url->password, &handler, context);
    return context->rtsp_client;
}

MS_StreamErrorCode ms_rtsp_client_session_destroy(struct rtsp_client_t *rtsp_client)
{
    if (!rtsp_client) {
        return MS_StreamErrorCode_FAILED;
    }
    rtsp_client_destroy(rtsp_client);
    return MS_StreamErrorCode_SUCCESS;
}

ms_rtsp_client_t *ms_rtsp_client_context_create()
{
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)malloc(sizeof(*context));
    if (!context) {
        return NULL;
    }
    memset(context, 0, sizeof(*context));
    context->socket = socket_invalid;
    context->rtsp_pool_thread = (pthread_t *)malloc(sizeof(*context->rtsp_pool_thread));
    if (!context->rtsp_pool_thread) {
        free(context);
        return NULL;
    }
    return context;
}

MS_StreamErrorCode ms_rtsp_client_context_destory(ms_rtsp_client_t *context)
{
    time_t t1, t2;
    
    if (!context) {
        return MS_StreamErrorCode_FAILED;
    }
    if (context->rtsp_pool_thread_run) {
        context->rtsp_pool_thread_run = 0;
        rtsp_client_debug(RTSP_INFO, "set destory context:%p URL:%s start.", context, context->url);
        t1 = time(0);
        thread_destroy(*context->rtsp_pool_thread);
        t2 = time(0);
        rtsp_client_debug(RTSP_INFO, "set destory context:%p URL:%s end. used:%ld(s)", context, context->url, t2-t1);
    }
    if (context->rtsp_pool_thread) {
        free(context->rtsp_pool_thread);
        context->rtsp_pool_thread = NULL;
    }
    free(context);
    return MS_StreamErrorCode_SUCCESS;
}

void ms_rtsp_client_rtp_session_init(ms_rtsp_client_t *context)
{
    int i = 0;
    for (i = 0; i < MAX_MEDIA_NUM; i++) {
        context->rtp_socket[i][0] = socket_invalid;
        context->rtp_socket[i][1] = socket_invalid;
        context->rtp_port[i][0] = 0;
        context->rtp_port[i][1] = 0;
        context->rtp_ctx[i] = NULL;
    }
}

void ms_rtsp_client_rtp_session_destory(ms_rtsp_client_t *context)
{
    int i = 0;
    for (i = 0; i < MAX_MEDIA_NUM; i++) {
        //first destory socket thread
        if (context->rtp_ctx[i]) {
            ms_rtp_session_destroy(context->rtp_ctx[i]);
            context->rtp_ctx[i] = NULL;
        }
        if (context->rtp_socket[i][0] != socket_invalid) {
            socket_close(context->rtp_socket[i][0]);
            context->rtp_socket[i][0] = socket_invalid;
        }
        if (context->rtp_socket[i][1] != socket_invalid) {
            socket_close(context->rtp_socket[i][1]);
            context->rtp_socket[i][1] = socket_invalid;
        }
        context->rtp_port[i][0] = 0;
        context->rtp_port[i][1] = 0;
    }
}

void ms_rtsp_client_create_session(char *session, int len)
{
    char str[MAX_SESSION_LENGTH] = {0};
    int i, n;
    srand((unsigned int)time(NULL));

    for (i = 0; i < MAX_SESSION_LENGTH - 1; i++) {
        n = rand() % 36; 
        if (n < 10) { 
            str[i] = '0' + n;
        } else  {
            str[i] = 'a' + (n - 10);
        } 
    }
    snprintf(session, len, "x-sessioncookie: %s\r\n", str);
}

void ms_rtsp_client_rtsp_quit(ms_rtsp_client_t *context) 
{
    ms_rtsp_client_rtp_session_destory(context);
    ms_rtsp_client_session_destroy(context->rtsp_client);
    ms_rtsp_client_notify_event(context, MS_StreamEventType_NETWORK);
}

void ms_rtsp_client_https_quit(ms_rtsp_client_t *context, int *socketfd)
{
    socket_close(*socketfd);
    *socketfd = socket_invalid;
    ms_rtsp_client_rtsp_quit(context);
    rtsp_client_debug(RTSP_WARN, "https end %s\n", context->url);
}

int ms_rtsp_client_select_next_proto_cb(SSL *ssl, unsigned char **out,
                                unsigned char *outlen, const unsigned char *in,
                                unsigned int inlen, void *arg)
{
    (void)ssl;
    (void)arg;
    nghttp2_select_next_protocol(out, outlen, in, inlen);

    return SSL_TLSEXT_ERR_OK;
}

void ms_rtsp_client_init_ssl_ctx(SSL_CTX *ssl_ctx)
{
    /* Disable SSLv2 and enable all workarounds for buggy servers */
    SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2);
    SSL_CTX_set_mode(ssl_ctx, SSL_MODE_AUTO_RETRY);
    SSL_CTX_set_mode(ssl_ctx, SSL_MODE_RELEASE_BUFFERS);
    SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
    /* Set NPN callback */
    SSL_CTX_set_next_proto_select_cb(ssl_ctx, ms_rtsp_client_select_next_proto_cb, NULL);
}

int ms_rtsp_client_ssl_handshake(SSL *ssl, int fd)
{
    int rv, errcode, cnt = 0;
    if (SSL_set_fd(ssl, fd) == 0) {
        return -1;
    }
    ERR_clear_error();
    while (cnt < 25) { // 5s
        rv = SSL_connect(ssl);
        if (rv == 1) { // ssl connect success
            return 0;
        }

        errcode = SSL_get_error(ssl, rv);
        if (errcode != SSL_ERROR_WANT_READ && errcode != SSL_ERROR_WANT_WRITE) {
            rtsp_client_debug(RTSP_WARN, "SSL_connect err, rv = %d, errcode = %d", rv, errcode);
            return -1;
        }

        ++cnt;
        usleep(200 * 1000);
    }

    return 0;
}

static void init_openssl()
{
    if (!CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON)) {
        SSL_library_init();
        ERR_load_BIO_strings();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }
}

int ms_rtsp_client_tls_connect(SSL **handle, int sockfd) 
{
    init_openssl();
    SSL *ssl;
    SSL_CTX *ssl_ctx;
    ssl_ctx = SSL_CTX_new(TLS_client_method());
    if (ssl_ctx == NULL) {
        return -1;
    }
    ms_rtsp_client_init_ssl_ctx(ssl_ctx);
    ssl = SSL_new(ssl_ctx);
    SSL_CTX_free(ssl_ctx);
    if (ssl == NULL) {
        return -1;
    }

    if (ms_rtsp_client_ssl_handshake(ssl, sockfd)) {
        SSL_free(ssl);
        return -1;
    }
    *handle = ssl;
    return 0;
}
void ms_rtsp_client_tls_close(SSL *handle)
{
    SSL_shutdown(handle);
    SSL_free(handle);
    handle = NULL;
}
#if 0
void *poll_rtsp_client_event(void *param)
{
    char ppacket[80 * 1024] = {0};
    rtsp_url_t rtsp_url = {0};
    char thread_name[32] = {0};
    uint64_t now_clock_time = 0, last_clock_time/*for frame check*/ = 0;
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)param;
    thread_get_name(thread_name, sizeof(thread_name));
    strcat(thread_name, "_R");
    thread_set_name(thread_name);
    if (!context) {
        return NULL;
    }
    if (parse_rtsp_url(context->url, &rtsp_url) != MS_StreamErrorCode_SUCCESS) {
        return NULL;
    }
    while (context->rtsp_pool_thread_run) {
        printf("Start Connect URL %s\n", context->url);
        ms_rtsp_client_rtp_session_init(context);
        ms_rtsp_client_stream_info_reset(context);
        context->rtsp_client = ms_rtsp_client_session_create(context, &rtsp_url);
        if (!context->rtsp_client) {
            break;
        }
        int r = -1;
        ms_rtsp_client_notify_event(context, MS_StreamEventType_WAITTING);
        context->socket = socket_connect_host(rtsp_url.host, rtsp_url.rtsp_port, 3000);
        if (socket_invalid == context->socket) {
            usleep(500000);
            ms_rtsp_client_rtp_session_destory(context);
            ms_rtsp_client_session_destroy(context->rtsp_client);
            ms_rtsp_client_notify_event(context, MS_StreamEventType_DISCONNECT);
            continue;
        }
        socket_setrecvbuf(context->socket, 500 * 1024);
        socket_setnonblock(context->socket, 1);
        if (rtsp_client_describe(context->rtsp_client)) {
            socket_close(context->socket);
            context->socket = socket_invalid;
            ms_rtsp_client_rtp_session_destory(context);
            ms_rtsp_client_session_destroy(context->rtsp_client);
            ms_rtsp_client_notify_event(context, MS_StreamEventType_DISCONNECT);
            continue;
        }
        last_clock_time = context->last_cmd_time = system_clock();
        while (context->rtsp_pool_thread_run) {
#if 0
            r = socket_recv_by_time(context->socket, ppacket, sizeof(ppacket), 0, 2000);
            if (r <= 0) {
                if (r != SOCKET_TIMEDOUT) {
                    printf("rtsp_client_socket_recv,error,%d\n", r);
                    break;
                }
            }
#else //maylong add for reduce cpu maybe delay more
            r = socket_recv(context->socket, ppacket, sizeof(ppacket), 0);
            if (r <= 0) {
                if (r == 0 || (socket_geterror() != EINTR && socket_geterror() !=  EAGAIN)) {
                    printf("rtsp_client_socket_recv,error,%d\n", r);
                    break;
                }
                usleep(20000);
            }
#endif
            else {
                //ppacket[r] = '\0';
                //rtsp_client_debug(RTSP_INFO, "recv request url:%s", ppacket);
                if (rtsp_client_input(context->rtsp_client, ppacket, r)) {
                    printf("rtsp_client_input,error\n");
                    break;
                }
            }
            now_clock_time = system_clock();
            if (now_clock_time - context->last_cmd_time > RTSP_KEEPAVLIE_TIMEINTERVAL * 1000) {
                //printf("send option keppalive\n");
                context->last_cmd_time = now_clock_time;
                if (rtsp_client_options(context->rtsp_client, NULL) < 0) {
                    break;
                }
            }
            if (now_clock_time - last_clock_time > RTSP_CLIENT_TIMEOUTINTERVAL * 1000) {
                if (context->frame_recv_flag == 1) {
                    last_clock_time = now_clock_time;
                    context->frame_recv_flag = 0;
                } else {
                    printf("frame recv timeout\n");
                    break;

                }
            }
        }
        printf("End Connect URL %s\n", context->url);
        rtsp_client_teardown(context->rtsp_client);
        socket_close(context->socket);
        context->socket = socket_invalid;
        ms_rtsp_client_rtp_session_destory(context);
        ms_rtsp_client_session_destroy(context->rtsp_client);
        ms_rtsp_client_notify_event(context, MS_StreamEventType_DISCONNECT);
    }
    return NULL;
}
#else
void *poll_rtsp_client_event(void *param)
{
    int retryFlag = 0;
    int counter = 0;
    char ppacket[80 * 1024] = {0};
    rtsp_url_t rtsp_url = {{0}};
    //char thread_name[32] = {0};
    uint64_t now_clock_time = 0, last_clock_time/*for frame check*/ = 0;
    uint8_t receive_frame = 0;
    int parserFailed = 0;
    int doTeardown;
    uint64_t timeout;
    MS_StreamEventType eventType = MS_StreamEventType_DISCONNECT;
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)param;
    uint32_t methodMask = 0;
    int ret = -1;

    if (!context) {
        return NULL;
    }

    thread_set_name(context->tname);

    if (parse_rtsp_url(context->url, &rtsp_url) != MS_StreamErrorCode_SUCCESS) {
        return NULL;
    }

RETRY_CONNECT: {
        rtsp_client_debug(RTSP_INFO, "Start Connect URL %s\n", context->url);
        doTeardown = 1;
        ms_rtsp_client_rtp_session_init(context);
        ms_rtsp_client_stream_info_reset(context);
        context->rtsp_client = ms_rtsp_client_session_create(context, &rtsp_url);
        if (!context->rtsp_client) {
            rtsp_client_debug(RTSP_INFO, "000 end Connect URL %s\n", context->url);
            return NULL;
        }
        int r = -1;
        if (!retryFlag) {
            /*
            【中心-转发：NVR1用RTSP方式添加NVR2的通道【主码流】，开启录像，之后检查录像：通道3NVR1回放中间有断开，
            NVR2回放连续】https://www.tapd.cn/21417271/bugtrace/bugs/view?bug_id=1121417271001083634
            */
            ms_rtsp_client_notify_event(context, MS_StreamEventType_WAITTING);
        }
        context->socket = socket_connect_host(rtsp_url.host, rtsp_url.rtsp_port, MAX_RTSP_SOCKET_CONNECT_TIMES);
        if (socket_invalid == context->socket) {
            ms_rtsp_client_rtsp_quit(context);
            rtsp_client_debug(RTSP_WARN, "111 end Connect URL %s\n", context->url);
            return NULL;
        }
        socket_setrecvbuf(context->socket, 500 * 1024);
        socket_setnonblock(context->socket, 1);
        //rtsp over https
        if (context->transmit_protocol == MS_StreamTransportType_RTSP_OVER_HTTPS && context->rtsp_pool_thread_run) {
            ms_rtsp_client_create_session(context->x_session, sizeof(context->x_session));
            if (ms_rtsp_client_tls_connect(&context->data_ssl, context->socket)) {
                ms_rtsp_client_https_quit(context, &context->socket);
                return NULL;
            }
            if (rtsp_client_over_https_data(context->rtsp_client, rtsp_url.host, context->x_session, context->respath)) {
                ms_rtsp_client_tls_close(context->data_ssl);
                ms_rtsp_client_https_quit(context, &context->socket);
                return NULL;
            }
            last_clock_time = system_clock();
            timeout = RTSP_KEEPAVLIE_TIMEINTERVAL;
            while (context->rtsp_pool_thread_run) {
                now_clock_time = system_clock();
                r = SSL_read(context->data_ssl, ppacket, sizeof(ppacket));
                if (r <= 0) {
                    if (r == 0 || (now_clock_time - last_clock_time > timeout) || (socket_geterror() != EINTR && socket_geterror() !=  EAGAIN)) {
                        r = -1;
                        break;
                    }   
                } else {
                    break;
                }
                usleep(20 * 1000);
            }

            //r = socket_recv(context->socket, ppacket, sizeof(ppacket), 0);
            if (r <= 0 || strstr(ppacket, "HTTP/1.0 200 OK") == NULL) {
                ms_rtsp_client_tls_close(context->data_ssl);
                socket_close(context->socket);
                context->socket = socket_invalid;
                ms_rtsp_client_rtp_session_destory(context);
                ms_rtsp_client_session_destroy(context->rtsp_client);
                ms_rtsp_client_notify_event(context, MS_StreamEventType_NETWORK);
                rtsp_client_debug(RTSP_WARN, "https end %s\n", context->url);
                printf("https end %s\n", context->url);
                return NULL;
            }
            context->rtsp_command_socket = socket_connect_host(rtsp_url.host, rtsp_url.rtsp_port, MAX_RTSP_SOCKET_CONNECT_TIMES);
            if (socket_invalid == context->rtsp_command_socket) {
                ms_rtsp_client_rtsp_quit(context);
                return NULL;
            }
            if (ms_rtsp_client_tls_connect(&context->command_ssl, context->rtsp_command_socket)) {
                ms_rtsp_client_https_quit(context, &context->rtsp_command_socket);
                return NULL;
            }
            if (rtsp_client_over_https_command(context->rtsp_client, rtsp_url.host, context->x_session, context->respath)) {
                ms_rtsp_client_tls_close(context->command_ssl);
                ms_rtsp_client_https_quit(context, &context->rtsp_command_socket);
                return NULL;
            }
        }
        if (rtsp_client_options(context->rtsp_client, NULL)) {
            if (context->transmit_protocol == MS_StreamTransportType_RTSP_OVER_HTTPS) {
                ms_rtsp_client_tls_close(context->data_ssl);
                ms_rtsp_client_tls_close(context->command_ssl);
                ms_rtsp_client_https_quit(context, &context->rtsp_command_socket);
            }
            socket_close(context->socket);
            context->socket = socket_invalid;
            ms_rtsp_client_rtsp_quit(context);
            rtsp_client_debug(RTSP_WARN, "222 end Connect URL %s\n", context->url);
            return NULL;
        }
        last_clock_time = context->last_cmd_time = system_clock();
        timeout = RTSP_KEEPAVLIE_TIMEINTERVAL;
        while (context->rtsp_pool_thread_run) {
            //david add for thread exit.
            if (context->dosleep) {
                counter++;
                if (counter < (3000)) {
                    usleep(20 * 1000);
                    continue;
                }
                counter = 0;
            }

            //maylong add for reduce cpu maybe delay more
            now_clock_time = system_clock();
            if (context->transmit_protocol == MS_StreamTransportType_RTSP_OVER_HTTPS) {
                r = SSL_read(context->data_ssl, ppacket, sizeof(ppacket));
                //r = socket_recv(context->socket, ppacket, sizeof(ppacket), 0);
            } else {
                r = socket_recv(context->socket, ppacket, sizeof(ppacket), 0);
            }
            
            if (r <= 0) {
                if (r == 0 || (socket_geterror() != EINTR && socket_geterror() !=  EAGAIN)) {
                    eventType = MS_StreamEventType_NETWORK;
                    doTeardown = 0;
                    rtsp_client_debug(RTSP_WARN, "socket recv error. code:%d url:%s\n", r, context->url);
                    break;
                }
                usleep(20 * 1000);
            } else {
                //ppacket[r] = '\0';
                //rtsp_client_debug(RTSP_INFO, "recv request url:%s", ppacket);
                //rtsp_client_debug(RTSP_INFO, "socket_recv context:%p ppacket len:%d URL:%s", context, r, context->url);
                if (rtsp_client_input(context->rtsp_client, ppacket, r, &context->dosleep, &timeout)) {
                    rtsp_client_debug(RTSP_WARN, "client input error. url:%s", context->url);
                    if (strstr(ppacket, "Not Enough Bandwidth")) {
                        eventType = MS_StreamEventType_BANDWIDTH;
                        rtsp_client_debug(RTSP_WARN, "Not Enough Bandwidth. event:%d url:%s", eventType, context->url);
                        break;
                    } else if (strstr(ppacket, "401 Unauthorized")) {
                        eventType = MS_StreamEventType_UNAUTH;
                        rtsp_client_debug(RTSP_WARN, "Unauthorized. event:%d url:%s", eventType, context->url);
                        break;
                    } else {
                        parserFailed++;
                        if (parserFailed >= 20) {
                            rtsp_client_debug(RTSP_WARN, "parser failed. event:%d url:%s", eventType, context->url);
                            break;
                        }
                    }
                } else {
                    parserFailed = 0;
                }
                last_clock_time = now_clock_time;
            }
            if (now_clock_time - context->last_cmd_time > timeout) {
                context->last_cmd_time = now_clock_time;
                methodMask = rtsp_client_get_methodmask(context->rtsp_client);
                if (methodMask & (1 << RTSP_OPTIONS)) {
                    ret = rtsp_client_options(context->rtsp_client, NULL);
                } else if (methodMask & (1 << RTSP_GET_PARAMETER)) {
                    ret = rtsp_client_get_parameter(context->rtsp_client, 0, NULL);
                } else {
                    ret = rtsp_client_options(context->rtsp_client, NULL);
                }
                if (ret < 0) {
                    eventType = MS_StreamEventType_NETWORK;
                    doTeardown = 0;
                    rtsp_client_debug(RTSP_WARN, "send keepalive failed. event:%d url:%s", eventType, context->url);
                    break;
                }
            }
            if (now_clock_time - last_clock_time > timeout) {
                if (context->frame_recv_flag == 1) {
                    last_clock_time = now_clock_time;
                    context->frame_recv_flag = 0;
                    receive_frame = 1;
                } else {
                    eventType = MS_StreamEventType_NETWORK;
                    rtsp_client_debug(RTSP_WARN, "frame recv timeout. event:%d url:%s timeout:%llu(ms)", eventType, context->url, timeout);
                    break;

                }
            }
        }

        rtsp_client_debug(RTSP_INFO, "End Connect URL %s\n", context->url);
        if (doTeardown) {
            rtsp_client_teardown(context->rtsp_client);
        }
        socket_close(context->socket);
        context->socket = socket_invalid;
        ms_rtsp_client_rtp_session_destory(context);
        ms_rtsp_client_session_destroy(context->rtsp_client);
#if 0   // for transp is auto       
        if (receive_frame == 0 && context->transport == RTSP_TRANSPORT_RTP_UDP) {
            context->transport = RTSP_TRANSPORT_RTP_TCP;
            rtsp_client_debug(RTSP_INFO, "Retry Connect RTSP With TCP: %s\n", context->url);
            goto RETRY_CONNECT;
        }
#else
        if (receive_frame == 0
            && context->transport == RTSP_TRANSPORT_RTP_TCP
            && context->transmit_protocol == MS_StreamTransportType_ATUO
            && context->rtsp_pool_thread_run) {
            context->transport = RTSP_TRANSPORT_RTP_UDP;
            retryFlag = 1;
            rtsp_client_debug(RTSP_DEBUG, "Auto Retry Connect RTSP From TCP to UDP: %s\n", context->url);
            goto RETRY_CONNECT;
        }
#endif
        ms_rtsp_client_notify_event(context, eventType);
    }

    return NULL;
}
#endif

ms_stream_rtspclient_handler
ms_rtsp_client_create(const char *url, const char *name, const char* path, MS_StreamTransportType transprtl, void *param)
{
    ms_rtsp_client_t *context = NULL;
    do {
        context = ms_rtsp_client_context_create();
        if (!context) {
            return NULL;
        }

        context->param = param;
        context->transmit_protocol = transprtl;

#if 1
        context->transport = transprtl == MS_StreamTransportType_UDP ? RTSP_TRANSPORT_RTP_UDP :
                             RTSP_TRANSPORT_RTP_TCP; /*default TCP*/
#else
        /*David debug Need To Do*/
        context->transport = RTSP_TRANSPORT_RTP_TCP;
#endif
        //rtsp_client_debug(RTSP_INFO, "[david debug] url:%s transport:%d", url, context->transport);
        snprintf(context->url, sizeof(context->url), "%s", url);
        snprintf(context->tname, sizeof(context->tname), "%s", name);
        snprintf(context->respath, sizeof(context->respath), "%s", path);
    } while (0);

    return context;
}

static int ms_rtsp_client_setsched_task(pthread_t thread_id)
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

MS_StreamErrorCode ms_rtsp_client_openstream(ms_stream_rtspclient_handler rtsp_client_context)
{
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)rtsp_client_context;
    if (context->rtsp_pool_thread_run == 1) {
        return MS_StreamErrorCode_FAILED;
    }
    context->rtsp_pool_thread_run = 1;
    if (thread_create2(context->rtsp_pool_thread, 128 * 1024, (thread_proc)poll_rtsp_client_event, context)) {
        context->rtsp_pool_thread_run = 0;
        return MS_StreamErrorCode_FAILED;
    }
    ms_rtsp_client_setsched_task(*context->rtsp_pool_thread);

    return MS_StreamErrorCode_SUCCESS;
}

MS_StreamErrorCode ms_rtsp_client_destroy(ms_stream_rtspclient_handler rtsp_client_context)
{
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)rtsp_client_context;
    if (context) {
        return ms_rtsp_client_context_destory(context);
    }
    return MS_StreamErrorCode_FAILED;
}

MS_StreamErrorCode ms_rtsp_client_setDataCallback(ms_stream_rtspclient_handler handle,
                                                  MS_RTSP_CLIENT_DataCallback callback)
{
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)handle;
    if (!context) {
        return MS_StreamErrorCode_FAILED;
    }
    context->data_callback = callback;
    return MS_StreamErrorCode_SUCCESS;
}

MS_StreamErrorCode ms_rtsp_client_setEventCallback(ms_stream_rtspclient_handler handle,
                                                   MS_RTSP_CLIENT_EventCallback callback)
{
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)handle;
    if (!context) {
        return MS_StreamErrorCode_FAILED;
    }
    context->event_callback = callback;
    return MS_StreamErrorCode_SUCCESS;
}

void ms_rtsp_client_stream_info_reset(ms_rtsp_client_t *context)
{
    memset(&context->stream_info, 0, sizeof(context->stream_info));
    context->start_push_data = 0;
}

void ms_rtsp_client_notify_event(ms_rtsp_client_t *context, MS_StreamEventType event_type)
{
    if (context->event_callback) {
        context->event_callback(event_type, &context->stream_info, context->param);
    }
}

int ms_rtsp_client_get_extradate(ms_stream_rtspclient_handler *handle, char *dst, int *ndstlen)
{
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)handle;
    if (!context) {
        return -1;
    }
    if (!dst || ndstlen <= 0) {
        return -1;
    }

    memcpy(dst, context->extradata.buf, context->extradata.size);
    *ndstlen = context->extradata.size;
    return 0;
}

int ms_rtsp_client_dosleep(ms_stream_rtspclient_handler rtsp_client_context, int dosleep)
{
    ms_rtsp_client_t *context = (ms_rtsp_client_t *)rtsp_client_context;
    if (context) {
        context->dosleep = dosleep;
        //printf("[david debug] ctx:%p url:%s dosleep:%d", context, context->url, dosleep);
    }

    return 0;
}

void ms_rtsp_client_set_debug(int enable)
{
    set_rtsp_stream_client_debug(enable);
}

void ms_rtsp_client_set_app_log(int enable)
{
    set_rtsp_stream_client_app_log(enable);
}


