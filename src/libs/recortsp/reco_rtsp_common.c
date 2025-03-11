#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
#define __USE_GNU
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/time.h>
#include <pthread.h>

#include "recortsp.h"
#include "reco_rtsp_common.h"
#include "XD_Stream.h"

#include "jerror.h"
#include "jconfig.h"
#include "jmorecfg.h"
#include "jpeglib.h"
#include "turbojpeg.h"
#include "ms_rtsp_client_stream.h"
#include "ms_rtsp_server_stream.h"

static int global_init = 0;
static MUTEX_OBJECT global_mutex;

struct RecoRtspDebugStr g_recoRtspDebug = {
    .client = 0,
    .server = 0,
    .print  = NULL,
};

//时间格式
static void time_to_string_ex(char stime[32])
{
    if (!stime) {
        return;
    }
    struct tm temp;
    time_t ntime = time(0);

    localtime_r((long *)&ntime, &temp);
    snprintf(stime, 32, "%4d-%02d-%02d_%02d_%02d_%02d",
             temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday,
             temp.tm_hour, temp.tm_min, temp.tm_sec);
    return ;
}

void get_rtsp_version(char *pVer, int pLen)
{
    snprintf(pVer, pLen, "%s", MS_RTSP_VERSION);
    return ;
}

void reco_rtsp_init(void)
{
    if (global_init) {
        return;
    }
    global_init = 1;
    ms_mutex_init(&global_mutex);

    ms_stream_init();

    return ;
}

void reco_rtsp_uninit()
{
    if (!global_init) {
        return;
    }
    global_init = 0;

    reco_rtsp_server_uninit();
#ifdef ZOE_RTSP
    libXD_Stream_Fini();
#endif
    ms_stream_uninit();
    ms_mutex_uninit(&global_mutex);
    return ;
}

void rtsp_format_debug_info(int type, char *fmt, va_list vl)
{
    char debug[1024] = {0};
    char stime[32] = {0};
    time_to_string_ex(stime);
    switch (type) {
        case 0:
            snprintf(debug, sizeof(debug), "[RTSP_SERVER] %s", stime);
            break;
        case 1:
            snprintf(debug, sizeof(debug), "[RTSP_CLIENT] %s", stime);
            break;
        case 2:
            snprintf(debug, sizeof(debug), "[FFMPEG] %s", stime);
            break;
        default:
            break;
    }
    vsprintf(debug + strlen(debug), fmt, vl);
    reco_cli_output("%s\n", debug);
}

// set 0:server 1:client, 2:all debug 3:ffmpeg
void reco_rtsp_set_debug(int type, int on)
{
    if (type == 3) {
        ms_rtsp_client_set_app_log(on);
    } else {
        g_recoRtspDebug.server = (type == 0 || type == 2) ? on : g_recoRtspDebug.server;
        g_recoRtspDebug.client = (type == 1 || type == 2) ? on : g_recoRtspDebug.client;
        ms_rtsp_client_set_debug(g_recoRtspDebug.client);
        ms_rtsp_server_set_debug(g_recoRtspDebug.server);
    }
}

void reco_rtsp_set_print(void *print)
{
    g_recoRtspDebug.print = print;
}

void reco_rtsp_debug(int level, const char *file, const char *func, int line, char *msg)
{
    char stime[32] = {0};
    
    if (g_recoRtspDebug.print) {
        g_recoRtspDebug.print(level, file, func, line, msg);
    } else {
        time_to_string_ex(stime);
        printf("%s RTSP %s %s:%d %s\n", stime, file, func, line, msg);
    }
}

void reco_rtsp_log(int level, const char *file, const char *func, int line, const char* format, ...)
{
    char buf[1024] = {0};
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    reco_rtsp_debug(level, file, func, line, buf);
}

int trim_char(char *src, int size, char c)
{
    int i = 0;
    int idx = 0;
    char dst[2048]  = {0};
    if (size >= sizeof(dst)) {
        return -1;
    }
    for (; i < size; i++) {
        if (src[i] == c) {
            continue;
        }
        dst[idx++] = src[i];
    }
    dst[idx] = 0;
    snprintf(src, size, "%s", dst);
    return 0;
}

/* data format
<?xml version="1.0" encoding="UTF-8"?><tt:MetadataStream xmlns:tt="http://www.onvif.org/ver10/schema"
><tt:Event><wsnt:NotificationMessage xmlns:tns1="http://www.onvif.org/ver10/topics"  xmlns:wsnt="htt
p://docs.oasis-open.org/wsn/b-2" xmlns:wsa5="http://www.w3.org/2005/08/addressing"><wsnt:Topic Diale
ct="http://docs.oasis-open.org/wsn/t-1/TopicExpression/Simple">tns1:VideoSource/tns1:MotionAlarm</ws
nt:Topic><wsnt:ProducerReference><wsa5:Address>uri://ce70a274-592e-11df-a812-00408cac51a7/ProducerRe
ference</wsa5:Address></wsnt:ProducerReference><wsnt:Message><tt:Message UtcTime="2014-09-10T22:30:1
9Z" PropertyOperation="Changed"><tt:Source><tt:SimpleItem Name="Source" Value="VideoSource_1"/></tt:
Source><tt:Data><tt:SimpleItem Name="State" Value="true"/></tt:Data></tt:Message></wsnt:Message></ws
nt:NotificationMessage></tt:Event></tt:MetadataStream>   */
int get_xml_section(char *data, char *name, char value[64])
{
    if (!data || !name || !value) {
        return -1;
    }
    //search section name
    char *head = NULL;
    char *tail = NULL;
    char search[128] = {0};
    char item[256] = {0};
    char sValue[256] = {0};
    // snprintf(search, sizeof(search), "<tt:SimpleItem Name=\"%s\"", name);
    snprintf(search, sizeof(search), "%s", "Data>");
    head = strstr(data, search);
    if (!head) {
        return -1;
    }
    // tail = strstr(head, "/>");
    tail = strstr(head + 4, "Data>");
    if (!tail) {
        return -1;
    }
    if (tail - head < sizeof(item)) {
        snprintf(item, tail - head + 1,  "%s", head);
    } else {
        snprintf(item, sizeof(item), "%s", head);
    }
    //printf("item:%s [david debug]\n", item);
    head = strstr(item, "Value=");
    if (!head) {
        return -1;
    }
    sscanf(head + strlen("Value=") + 1, "%[^\"]", sValue);
    snprintf(value, 64, "%s", sValue);
    //printf("value:%s [david debug]\n", value);
    return 0;
}

int xml_true_value(char *value, int type)
{
    if (!value) {
        return 0;
    }
    if (type == 0) {
        if (!strcmp(value, "true") || !strcmp(value, "1")) {
            return 1;
        }
    } else if (type == 1) {
        if (!strcmp(value, "Idle") || !strcmp(value, "IsInside")) {
            return 1;
        }
    }
    // if (!strcmp(value, "true") || !strcmp(value, "1") ) return 1;
    return 0;
}

int xml_false_value(char *value, int type)
{
    if (!value) {
        return 0;
    }
    if (type == 0) {
        if (!strcmp(value, "false") || !strcmp(value, "0")) {
            return 1;
        }
    } 
    // if (!strcmp(value, "true") || !strcmp(value, "1") ) return 1;
    return 0;
}

int judge_motion(char *data,  char *name)
{
    if (!data || !name) {
        return -1;
    }
    //search section name
    char *head = NULL;
    char *tail = NULL;
    char search[128] = {0};
    char item[512] = {0};
    // snprintf(search, sizeof(search), "<tt:SimpleItem Name=\"%s\"", name);
    snprintf(search, sizeof(search), "%s", "<wsnt:Topic");
    head = strstr(data, search);
    if (!head) {
        return -1;
    }
    // tail = strstr(head, "/>");
    tail = strstr(head + 8, "wsnt:Topic>");
    if (!tail) {
        return -1;
    }
    if (tail - head < sizeof(item)) {
        snprintf(item, tail - head + 1,  "%s", head);
    } else {
        snprintf(item, sizeof(item), "%s", head);
    }
    //printf("item:%s\n", item);
    //head = strstr(item, "tns1:VideoSource/tns1:MotionAlarm");
    head = strstr(item, "Motion");
    if (!head) {
        return -1;
    }
    return 0;
}

void reco_rtsp_show_framedata(struct reco_frame *frame)
{
    char data[256] = {0};
    if (!frame) {
        return ;
    }
    
    if (frame->strm_type == ST_VIDEO
        && frame->frame_type == FT_IFRAME
        && frame->ch == 0
        && frame->stream_format == STREAM_TYPE_MAINSTREAM) {
        rtsp_log(TSAR_DEBUG, "[david debug] ch:%d size:%d strm_type:%d from:%d start0", frame->ch, frame->size, frame->strm_type,
                        frame->stream_from);
        int i = 0;
        char pdata[MAX_LEN_512] = {0};
        memcpy(pdata, frame->data, MAX_LEN_512 - 1);
        pdata[MAX_LEN_512 - 1] = '\0';
        data[0] = '\0';
        for (i = 0; i < MAX_LEN_512; i++) {
            if (i % 63 == 0 && i != 0) {
                snprintf(data+strlen(data), sizeof(data) - strlen(data), "%x\n", pdata[i]);
                rtsp_log(TSAR_DEBUG, data);
            } else {
                snprintf(data+strlen(data), sizeof(data) - strlen(data), "%x ", pdata[i]);
            }
        }
        rtsp_log(TSAR_DEBUG, data);
        rtsp_log(TSAR_DEBUG, "[david debug] show framedata end0.");
    }
    if (frame->strm_type == ST_VIDEO
        && frame->ch == 0
        && frame->size < 100
        && frame->stream_format == STREAM_TYPE_MAINSTREAM) {
        rtsp_log(TSAR_DEBUG, "[david debug] ch:%d size:%d strm_type:%d from:%d start1\n", frame->ch, frame->size, frame->strm_type,
                        frame->stream_from);
        int i = 0;
        char pdata[MAX_LEN_512] = {0};
        memcpy(pdata, frame->data, frame->size);
        pdata[frame->size] = '\0';
        data[0] = '\0';
        for (i = 0; i < frame->size; i++) {
            if (i % 63 == 0 && i != 0) {
                snprintf(data+strlen(data), sizeof(data) - strlen(data), "%x\n", pdata[i]);
                rtsp_log(TSAR_DEBUG, data);
            } else {
                snprintf(data+strlen(data), sizeof(data) - strlen(data), "%x ", pdata[i]);
            }
        }
        rtsp_log(TSAR_DEBUG, data);
        rtsp_log(TSAR_DEBUG, "[david debug] show framedata end1.\n");
    }
    return ;
}

#if 0
int MsCreateJPGFile(const char *outfile, AVFrame *pFrameIn)
{
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx = NULL;
    int i, ret, got_output;
    //FILE *fp_in;
    FILE *fp_out;
    AVFrame *pFrame;
    AVPacket pkt;
    //int y_size;
    int framecnt = 0;
    struct SwsContext *img_convert_ctx;

    avcodec_register_all();
    pCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!pCodec) {
        printf("Codec not found \n");
        return -1;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        printf("Could not allocate video codec context \n");
        return -1;
    }

    pCodecCtx->width = pFrameIn->width;
    pCodecCtx->height = pFrameIn->height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    if (!pFrame) {
        printf("Could not allocate video frame\n");
        return -1;
    }
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;
    pFrame->pts = 1;
    ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
    if (ret < 0) {
        printf("Could not allocate raw picture buffer\n");
        return -1;
    }

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                     pCodecCtx->height, AV_PIX_FMT_YUVJ420P, SWS_BICUBIC, NULL, NULL, NULL);
    sws_scale(img_convert_ctx, (const uint8_t *const *)pFrameIn->data, pFrameIn->linesize, 0, pCodecCtx->height,
              pFrame->data, pFrame->linesize);

    //Output file
    fp_out = fopen(outfile, "wb");
    if (!fp_out) {
        printf("Could not open %s\n", outfile);
        return -1;
    }

    //Encode init
    av_init_packet(&pkt);
    pkt.data = NULL;    // packet data will be allocated by the encoder
    pkt.size = 0;

    //encode the image
    ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
    if (ret < 0) {
        fclose(fp_out);
        printf("Error encoding frame\n");
        return -1;
    }
    if (got_output) {
        printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
        framecnt++;
        fwrite(pkt.data, 1, pkt.size, fp_out);
        av_free_packet(&pkt);
    }

    //Flush Encoder
    for (got_output = 1; got_output; i++) {
        ret = avcodec_encode_video2(pCodecCtx, &pkt, NULL, &got_output);
        if (ret < 0) {
            printf("Error encoding frame\n");
            fclose(fp_out);
            return -1;
        }
        if (got_output) {
            printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", pkt.size);
            fwrite(pkt.data, 1, pkt.size, fp_out);
            av_free_packet(&pkt);
        }
    }

    fclose(fp_out);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);
    sws_freeContext(img_convert_ctx);

    return 0;
}

int reco_rtsp_create_jpeg_file(const char *outfile, struct reco_frame *frame, int quality)
{
    if (!frame || frame->frame_type != FT_IFRAME || frame->strm_type != ST_VIDEO) {
        return -1;
    }
    int frameFinished = 0;
    //char* pTempData = NULL;
    AVPacket pkt;
    AVCodec *pCodec;
    AVFrame *pFrame;
    AVCodecContext *pCodecCtx;
    //struct SwsContext *img_convert_ctx = NULL;

    av_register_all();

    av_init_packet(&pkt);
    pkt.size = frame->size;
    pkt.data = frame->data;
    pkt.stream_index = frame->strm_type == ST_VIDEO ? 0 : 1;
    pkt.flags = 1;
    pkt.dts = pkt.pts = (((uint64_t)frame->time_upper) << 32) + frame->time_lower;
    pkt.pos = -1;
    pkt.duration = 3000;

    if (frame->codec_type == CODECTYPE_H265) {
        pCodec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    } else {
        pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    }

    if (pCodec == NULL) {
        printf("[david debug] Unsupported codec!\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);

    pCodecCtx->frame_number = 1;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->width = frame->width;
    pCodecCtx->height = frame->height;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        printf("Could not open codec\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &pkt);
    if (frameFinished) {
        MsCreateJPGFile(outfile, pFrame);
    }

    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);

    return 0;
}

int MsCreateJPGData(RTSP_JPEG_INFO *jpegData, AVFrame *pFrameIn)
{
    int ret = -1;
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx = NULL;
    int got_output;
    AVFrame *pFrame;
    AVPacket *pkt = NULL;
    struct SwsContext *img_convert_ctx;

    avcodec_register_all();
    pCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!pCodec) {
        rtsp_log(TSAR_WARN, "Codec found failed\n");
        return ret;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        rtsp_log(TSAR_WARN, "Could not allocate video codec context \n");
        return ret;
    }

    pCodecCtx->width = pFrameIn->width;
    pCodecCtx->height = pFrameIn->height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        av_free(pCodecCtx);
        rtsp_log(TSAR_WARN, "open codec failed\n");
        return ret;
    }

    pFrame = av_frame_alloc();
    if (!pFrame) {
        avcodec_close(pCodecCtx);
        av_free(pCodecCtx);
        rtsp_log(TSAR_WARN, "allocate video frame fialed\n");
        return ret;
    }
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;
    pFrame->pts = 1;
    ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 16);
    if (ret < 0) {
        av_frame_free(&pFrame);
        avcodec_close(pCodecCtx);
        av_free(pCodecCtx);
        rtsp_log(TSAR_WARN, "Could not allocate raw picture buffer\n");
        return ret;
    }

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                     pCodecCtx->height, AV_PIX_FMT_YUVJ420P, SWS_BICUBIC, NULL, NULL, NULL);
    sws_scale(img_convert_ctx, (const uint8_t *const *)pFrameIn->data, pFrameIn->linesize, 0, pCodecCtx->height,
              pFrame->data, pFrame->linesize);

    //Encode init
    pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    av_init_packet(pkt);
    pkt->data = NULL;    // packet data will be allocated by the encoder
    pkt->size = 0;

    //encode the image
    pCodecCtx->flags |= CODEC_FLAG_LOW_DELAY;
    ret = avcodec_encode_video2(pCodecCtx, pkt, pFrame, &got_output);
    if (ret < 0) {
        rtsp_log(TSAR_WARN, "encoding frame failed\n");
        avcodec_close(pCodecCtx);
        av_free(pCodecCtx);
        av_frame_free(&pFrame);
        sws_freeContext(img_convert_ctx);
        av_free_packet(pkt);
        av_free(pkt);
        return ret;
    }

    if (got_output) {
        jpegData->size = pkt->size;
        jpegData->width = pCodecCtx->width;
        jpegData->height = pCodecCtx->height;
        jpegData->data = av_malloc(pkt->size);
        memcpy(jpegData->data, pkt->data, jpegData->size);
    }

    av_free_packet(pkt);
    av_free(pkt);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);
    sws_freeContext(img_convert_ctx);

    return 0;
}

int reco_rtsp_create_jpeg_data(RTSP_JPEG_INFO *jpegData, struct reco_frame *frame, int quality)
{
    if (!frame || frame->frame_type != FT_IFRAME || frame->strm_type != ST_VIDEO) {
        return -1;
    }
    ms_mutex_lock(&global_mutex);
    int frameFinished = 0, ret = -1;
    //char* pTempData = NULL;
    AVPacket *pkt = (AVPacket *)av_malloc(sizeof(AVPacket));
    AVCodec *pCodec;
    AVFrame *pFrame;
    AVCodecContext *pCodecCtx;
    //struct SwsContext *img_convert_ctx = NULL;

    av_register_all();
    av_init_packet(pkt);
    pkt->size = frame->size;
    pkt->data = frame->data;
    pkt->stream_index = frame->strm_type == ST_VIDEO ? 0 : 1;
    pkt->flags = 1;
    pkt->dts = pkt->pts = (((uint64_t)frame->time_upper) << 32) + frame->time_lower;
    pkt->pos = -1;
    pkt->duration = 3000;
    if (frame->codec_type == CODECTYPE_H265) {
        pCodec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
        rtsp_log(TSAR_WARN, "JPEG H265\n");
    } else {
        pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
        rtsp_log(TSAR_WARN, "JPEG H264\n");
    }
    if (pCodec == NULL) {
        rtsp_log(TSAR_WARN, "Unsupported codec!\n");
        av_free_packet(pkt);
        av_free(pkt);
        ms_mutex_unlock(&global_mutex);
        return ret;
    }

    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL) {
        rtsp_log(TSAR_WARN, "alloc context failed!\n");
        av_free_packet(pkt);
        av_free(pkt);
        ms_mutex_unlock(&global_mutex);
        return ret;
    }
    pCodecCtx->frame_number = 1;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->width = frame->width;
    pCodecCtx->height = frame->height;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->qmin = 70;
    pCodecCtx->qmax = 70;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        rtsp_log(TSAR_WARN, "open codec fialed\n");
        av_free_packet(pkt);
        av_free(pkt);
        av_free(pCodecCtx);
        ms_mutex_unlock(&global_mutex);
        return ret;
    }
    pFrame = av_frame_alloc();
    if (pFrame == NULL) {
        rtsp_log(TSAR_WARN, "alloc frame failed!\n");
        av_free_packet(pkt);
        av_free(pkt);
        avcodec_close(pCodecCtx);
        av_free(pCodecCtx);
        ms_mutex_unlock(&global_mutex);
        return ret;
    }
    pCodecCtx->flags |= CODEC_FLAG_LOW_DELAY;
    av_opt_set(pCodecCtx->priv_data, "preset", "superfast", 0);
    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
    //pCodecCtx->codec->capabilities |= CODEC_CAP_FRAME_THREADS;
    ret = avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, pkt);
    if (frameFinished && ret > 0) {
        ret = MsCreateJPGData(jpegData, pFrame);
    }
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_free_packet(pkt);
    av_free(pkt);
    rtsp_log(TSAR_WARN, "ret :%d\n", ret);
    ms_mutex_unlock(&global_mutex);
    return ret;
}

#else
int MsCreateJPGFile(const char *outfile, void *pFrameIn)
{
    return -1;
}

int reco_rtsp_create_jpeg_file(const char *outfile, struct reco_frame *frame, int quality)
{
    return -1;
}

int MsCreateJPGData(RTSP_JPEG_INFO *jpegData, void *pFrameIn)
{
    return -1;
}

int reco_rtsp_create_jpeg_data(RTSP_JPEG_INFO *jpegData, struct reco_frame *frame, int quality)
{
    return -1;
}
#endif

INT64 reco_rtsp_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    return (INT64)tv.tv_sec * 1000000 + tv.tv_usec;
}

int reco_rtsp_cpu_setaffinity(int cpuCoreId, int pThread)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpuCoreId, &mask);
    if (pthread_setaffinity_np(pThread, sizeof(mask), &mask) < 0) {
        rtsp_log(TSAR_ERR, "set thread affinity failed.\n");
    }
    
    return 0;
}