#include <stdio.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>
#include <pthread.h>
#include <emscripten.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"   
#include "libswscale/swscale.h"
#include "linkedlists.h"

typedef void(*VideoCallback)(unsigned char *buff, int size, double timestamp, int width, int height);
typedef void(*AudioCallback)(unsigned char *buff, int size, double timestamp);
typedef void(*RequestCallback)(int offset, int available);

#ifdef __cplusplus
extern "C" {
#endif

enum frm_type {
    FT_IFRAME = 0,
    FT_PFRAME,
};

enum strm_type {
    ST_NONE = 0,
    ST_VIDEO,
    ST_AUDIO,
    ST_DATA,
    ST_METADATA,
    MAX_ST,
};

//video
#define CODECTYPE_H264                              0
#define CODECTYPE_MPEG4                             1
#define CODECTYPE_MJPEG                             2
#define CODECTYPE_H265                              3

//audio
#define AUDIO_CODEC_G711                            0
#define AUDIO_CODEC_AAC                             1
#define AUDIO_CODEC_G711_MULAW                      2
#define AUDIO_CODEC_G711_ALAW                       3
#define AUDIO_CODEC_G722                            4
#define AUDIO_CODEC_G726                            5


static int kInitialPcmBufferSize = 128 * 1024;
static int kMaxPacketSize = 60 * 1024 * 1024;
static int kMaxPacketCount = 200;


typedef enum ErrorCode {
    kErrorCodeFailed            = -1,
    kErrorCodeSuccess           = 0,
    kErrorCodeInvalidParam      = 1,
    kErrorCodeInvalidState      = 2,
    kErrorCodeInvalidData       = 3,
    kErrorCodeInvalidFormat     = 4,
    kErrorCodeFFmpegError       = 5,
    kErrorCodeOldFrame          = 6,
    kErrorCodeOverFrameLimit    = 7,
    kErrorCodeOverSizeLimit     = 8,
} ErrorCode;

typedef enum LogLevel {
    kLogLevel_None, //Not logging.
    kLogLevel_Core, //Only logging core module(without ffmpeg).
    kLogLevel_All   //Logging all, with ffmpeg.
} LogLevel;

struct WebHttpPacketHeader{
    int boundary;
    int version;
    char hsession[32];             //http session id
    unsigned int header_size;      // header size
    int ch;                        // ch number
    int strm_type;                 // strm_type: video data or audio data
    unsigned int size;             // size
    int codec_type;                // video codec type

    // for video
    int frame_type;                // i-frame or p-frame
    int width;
    int height;

    //for audio
    int audio_codec;               // -1 for no audio (when strm_type is VIDEO this params is AUDIO codec)
    int sample;
    int64_t time_usec;
};

struct WebPacketNode {
    struct WebHttpPacketHeader header;
    AVPacket avpkt;
    int size;
    
    unsigned char *data;
    AST_LIST_ENTRY(WebPacketNode) list;
};

typedef struct WebDecoder {
    AVFormatContext *avformatContext;
    AVCodecContext *videoCodecContext;
    AVCodecContext *audioCodecContext;
    AVFrame *avFrame;
    int videoStreamIdx;
    int audioStreamIdx;
    VideoCallback videoCallback;
    AudioCallback audioCallback;
    unsigned char *yuvBuffer;
    unsigned char *pcmBuffer;
    int currentPcmBufferSize;
    int videoBufferSize;
    int videoSize;
    
    double beginTimeOffset;
    int accurateSeek;


    /*ms ffmpeg*/
    int run;        //1:running
    int decode;     //0=disabled, 1=enable
    int audioOpen;  //0=close,1=open
    int format;     //1=yuv,0=rgb

    int videoWidth;
    int videoHeight;

    //list
    AST_LIST_HEAD_NOLOCK(packetList, WebPacketNode) packetList;
    pthread_mutex_t listMutex;
    int listSize;
    int listCount;

    int iFrameFlag;
    
} WebDecoder;

WebDecoder *decoder = NULL;
LogLevel logLevel = kLogLevel_None;

#define simpleLog1(format, ...) \
    do{\
        char szTime[32] = {0};\
        struct tm tmTime;\
        struct timeb tb;\
        int tmYear;\
        int tmMon;\
        int tmMday;\
        int tmHour;\
        int tmMin;\
        int tmSec;\
        int tmMillisec;\
        ftime(&tb);\
        localtime_r(&tb.time, &tmTime);\
        tmYear		= tmTime.tm_year + 1900;\
        tmMon		= tmTime.tm_mon + 1;\
        tmMday		= tmTime.tm_mday;\
        tmHour		= tmTime.tm_hour;\
        tmMin		= tmTime.tm_min;\
        tmSec		= tmTime.tm_sec;\
        tmMillisec	= tb.millitm;\
        sprintf(szTime, "%d-%d-%d %d:%d:%d.%d", tmYear, tmMon, tmMday, tmHour, tmMin, tmSec, tmMillisec);\
        printf("[%s][Core][func:%s line:%d]" format "\n", szTime, __func__, __LINE__);\
    }while(0)


void simpleLog(const char* format, ...) {
    if (logLevel == kLogLevel_None) {
        return;
    }

    char szBuffer[1024] = { 0 };
    char szTime[32]		= { 0 };
    char *p				= NULL;
    int prefixLength	= 0;
    const char *tag		= "Core";
    struct tm tmTime;
    struct timeb tb;

    ftime(&tb);
    localtime_r(&tb.time, &tmTime);

    if (1) {
        int tmYear		= tmTime.tm_year + 1900;
        int tmMon		= tmTime.tm_mon + 1;
        int tmMday		= tmTime.tm_mday;
        int tmHour		= tmTime.tm_hour;
        int tmMin		= tmTime.tm_min;
        int tmSec		= tmTime.tm_sec;
        int tmMillisec	= tb.millitm;
        sprintf(szTime, "%d-%d-%d %d:%d:%d.%d", tmYear, tmMon, tmMday, tmHour, tmMin, tmSec, tmMillisec);
    }

    prefixLength = sprintf(szBuffer, "[%s][%s][DT] ", szTime, tag);
    p = szBuffer + prefixLength;
    
    if (1) {
        va_list ap;
        va_start(ap, format);
        vsnprintf(p, 1024 - prefixLength, format, ap);
        va_end(ap);
    }

    printf("%s\n", szBuffer);
}

static void packet_list_free(struct WebPacketNode *node)
{
    if (node) {
        av_free_packet(&node->avpkt);
        av_free(node);
    }
}

static void packet_list_clear()
{
    struct WebPacketNode *node = NULL;
    pthread_mutex_lock(&decoder->listMutex);
    do {
        if (AST_LIST_EMPTY(&decoder->packetList)) {
            break;
        }
        node = AST_LIST_FIRST(&decoder->packetList);
        if (node) {
            AST_LIST_REMOVE(&decoder->packetList, node, list);
            decoder->listSize -= node->size;
            decoder->listCount -= 1;
            packet_list_free(node);
        }
    } while (1);
    pthread_mutex_unlock(&decoder->listMutex);
}

static int packet_list_insert(unsigned char *data, int size)
{
    struct WebPacketNode *node;
    struct WebHttpPacketHeader *header;
    int ret = kErrorCodeFailed;

    if (!decoder) {
        simpleLog("packet_list_insert failed. decoder not init");
        return kErrorCodeFailed;
    }

    if ((size < sizeof(struct WebHttpPacketHeader)) || !data) {
        simpleLog("packet_list_insert invalid. size:%d data:%p", size, data);
        return kErrorCodeFailed;
    }

    if (decoder->listSize > kMaxPacketSize) {
        simpleLog("packet_list_insert list size over limit %d, reset list", kMaxPacketSize);
        packet_list_clear();
        decoder->iFrameFlag = 1;
        ret = kErrorCodeOverSizeLimit;
    } else if (decoder->listCount > kMaxPacketCount) {
        simpleLog("packet_list_insert list count over limit %d reset list", kMaxPacketCount);
        packet_list_clear();
        decoder->iFrameFlag = 1;
        ret = kErrorCodeOverFrameLimit;
    }

    header = (struct WebHttpPacketHeader *)data;
    if (decoder->iFrameFlag == 1) {
        if (header->strm_type == ST_VIDEO && header->frame_type == FT_IFRAME) {
            decoder->iFrameFlag = 0;
        } else {
            return ret;
        }
    }

    node = av_mallocz(sizeof(struct WebPacketNode));
    if (!node) {
        simpleLog("packet_list_insert av_mallocz failed. size:%d", sizeof(struct WebPacketNode));
        packet_list_clear();
        decoder->iFrameFlag = 1;
        return kErrorCodeFailed;
    }
    memcpy(&node->header, data, sizeof(struct WebHttpPacketHeader));

    node->size = size - sizeof(struct WebHttpPacketHeader);
    av_new_packet(&node->avpkt, node->size);
    if (node->header.strm_type == ST_VIDEO) {
        node->avpkt.flags = (node->header.frame_type == FT_IFRAME ? 1 : 0);
    } else if (node->header.strm_type == ST_AUDIO){
        node->avpkt.flags = 1;
    }
    node->avpkt.pts = node->header.time_usec;
    node->avpkt.dts = node->header.time_usec;
    node->avpkt.size = node->header.size;
    memcpy(node->avpkt.data, data+sizeof(struct WebHttpPacketHeader), node->avpkt.size);
    pthread_mutex_lock(&decoder->listMutex);
    AST_LIST_INSERT_TAIL(&decoder->packetList, node, list);
    decoder->listSize += node->size;
    decoder->listCount += 1;
    pthread_mutex_unlock(&decoder->listMutex);

    //simpleLog("packet_list_insert success. total:%d %d size:%d %d %d ", decoder->listCount, decoder->listSize, size, sizeof(struct WebHttpPacketHeader), node->size);
    return kErrorCodeSuccess;
}

static struct WebPacketNode *packet_list_popfront()
{
    struct WebPacketNode *node = NULL;
    pthread_mutex_lock(&decoder->listMutex);
    do {
        if (AST_LIST_EMPTY(&decoder->packetList)) {
            break;
        }
        node = AST_LIST_FIRST(&decoder->packetList);
        if (node) {
            AST_LIST_REMOVE(&decoder->packetList, node, list);
            decoder->listSize -= node->size;
            decoder->listCount -= 1;
        }
    } while (0);
    pthread_mutex_unlock(&decoder->listMutex);

    return node;
}

void ffmpegLogCallback(void* ptr, int level, const char* fmt, va_list vl) {
    static int printPrefix	= 1;
    static int count		= 0;
    static char prev[1024]	= { 0 };
    char line[1024]			= { 0 };
    static int is_atty;
    AVClass* avc = ptr ? *(AVClass**)ptr : NULL;
    if (level > AV_LOG_DEBUG) {
        return;
    }

    line[0] = 0;

    if (printPrefix && avc) {
        if (avc->parent_log_context_offset) {
            AVClass** parent = *(AVClass***)(((uint8_t*)ptr) + avc->parent_log_context_offset);
            if (parent && *parent) {
                snprintf(line, sizeof(line), "[%s @ %p] ", (*parent)->item_name(parent), parent);
            }
        }
        snprintf(line + strlen(line), sizeof(line) - strlen(line), "[%s @ %p] ", avc->item_name(ptr), ptr);
    }

    vsnprintf(line + strlen(line), sizeof(line) - strlen(line), fmt, vl);
    line[strlen(line) + 1] = 0;
    simpleLog("%s", line);
}

int openCodecContext(enum AVMediaType type, int codecId, AVCodecContext **decCtx) {
    int ret = 0;
    do {
        AVCodec *dec		= NULL;
        AVDictionary *opts	= NULL;

        dec = avcodec_find_decoder(codecId);
        if (!dec) {
            simpleLog("Failed to find %s codec %d.", av_get_media_type_string(type), codecId);
            ret = AVERROR(EINVAL);
            break;
        }

        *decCtx = avcodec_alloc_context3(dec);
        if (!*decCtx) {
            simpleLog("Failed to allocate the %s codec context.", av_get_media_type_string(type));
            ret = AVERROR(ENOMEM);
            break;
        }

        if ((ret = avcodec_open2(*decCtx, dec, NULL)) != 0) {
            avcodec_close(*decCtx);
            *decCtx = NULL;
            simpleLog("Failed to open %s codec.", av_get_media_type_string(type));
            break;
        }
    } while (0);

    return ret;
}

void closeCodecContext(AVFormatContext *fmtCtx, AVCodecContext *decCtx, int streamIdx) {
    do {
        if (fmtCtx == NULL || decCtx == NULL) {
            break;
        }

        if (streamIdx < 0 || streamIdx >= fmtCtx->nb_streams) {
            break;
        }

        fmtCtx->streams[streamIdx]->discard = AVDISCARD_ALL;
        avcodec_close(decCtx);
    } while (0);
}

int roundUp(int numToRound, int multiple) {
    return (numToRound + multiple - 1) & -multiple;
}

ErrorCode processDecodedAudioFrame(AVFrame *frame) {
    ErrorCode ret       = kErrorCodeSuccess;
    int sampleSize      = 0;
    int audioDataSize   = 0;
    int targetSize      = 0;
    int offset          = 0;
    int i               = 0;
    int ch              = 0;
    double timestamp    = 0.0f;
    do {
        if (frame == NULL) {
            ret = kErrorCodeInvalidParam;
            break;
        }

        sampleSize = av_get_bytes_per_sample(decoder->audioCodecContext->sample_fmt);
        if (sampleSize < 0) {
            simpleLog("Failed to calculate data size.");
            ret = kErrorCodeInvalidData;
            break;
        }

        if (decoder->pcmBuffer == NULL) {
            decoder->pcmBuffer = (unsigned char*)av_mallocz(kInitialPcmBufferSize);
            decoder->currentPcmBufferSize = kInitialPcmBufferSize;
            simpleLog("Initial PCM buffer size %d.", decoder->currentPcmBufferSize);
        }

        audioDataSize = frame->nb_samples * decoder->audioCodecContext->channels * sampleSize;
        if (decoder->currentPcmBufferSize < audioDataSize) {
            targetSize = roundUp(audioDataSize, 4);
            simpleLog("Current PCM buffer size %d not sufficient for data size %d, round up to target %d.",
                decoder->currentPcmBufferSize,
                audioDataSize,
                targetSize);
            decoder->currentPcmBufferSize = targetSize;
            av_free(decoder->pcmBuffer);
            decoder->pcmBuffer = (unsigned char*)av_mallocz(decoder->currentPcmBufferSize);
        }

        for (i = 0; i < frame->nb_samples; i++) {
            for (ch = 0; ch < decoder->audioCodecContext->channels; ch++) {
                memcpy(decoder->pcmBuffer + offset, frame->data[ch] + sampleSize * i, sampleSize);
                offset += sampleSize;
            }
        }

        timestamp = (double)frame->pts * av_q2d(decoder->avformatContext->streams[decoder->audioStreamIdx]->time_base);

        if (decoder->accurateSeek && timestamp < decoder->beginTimeOffset) {
            //simpleLog("audio timestamp %lf < %lf", timestamp, decoder->beginTimeOffset);
            ret = kErrorCodeOldFrame;
            break;
        }
        if (decoder->audioCallback != NULL) {
            decoder->audioCallback(decoder->pcmBuffer, audioDataSize, timestamp);
        }
    } while (0);
    return ret;
}

static inline int getDecodeVideoType(int codec_type)
{
    if (codec_type == CODECTYPE_H265) {
        return AV_CODEC_ID_HEVC;//173
    } else {
        return AV_CODEC_ID_H264;
    }
}

static inline int getDecodeAudioType(int codec_type)
{
    if (codec_type == AUDIO_CODEC_AAC) {
        return AV_CODEC_ID_AAC;
    } else if (codec_type == AUDIO_CODEC_G711_MULAW) {
        return AV_CODEC_ID_PCM_MULAW;
    } else if (codec_type == AUDIO_CODEC_G711_ALAW) {
        return AV_CODEC_ID_PCM_ALAW;
    } else if (codec_type == AUDIO_CODEC_G722) {
        return AV_CODEC_ID_ADPCM_G722;
    } else if (codec_type == AUDIO_CODEC_G726) {
        return AV_CODEC_ID_ADPCM_G726;
    } else {
        return AV_CODEC_ID_PCM_MULAW;
    }
}

ErrorCode decodePacket(struct WebHttpPacketHeader *header, AVPacket *pkt)
{
    int ret = kErrorCodeSuccess;
    int isVideo = 0;
    int r;
    int codecId;
    int gotPicture;
    AVCodecContext *codecContext = NULL;
    int i, j;
    int shift;
    unsigned char *yuv = NULL;
    unsigned char *pSize = NULL;
    double timestamp = 0.0f;

    if (header->strm_type == ST_VIDEO) {
        if (!decoder->videoCodecContext) {
            //open video codec first
            decoder->videoWidth = header->width;
        	decoder->videoHeight = header->height;
            codecId = getDecodeVideoType(header->codec_type);
            simpleLog("start open video codec context  width:%d height:%d codecId:%d", decoder->videoWidth, decoder->videoHeight, codecId);
            r = openCodecContext(AVMEDIA_TYPE_VIDEO, codecId, &decoder->videoCodecContext);
            if (r != 0) {
                simpleLog("Open video codec context failed %d.", r);
                return kErrorCodeFFmpegError;
            }
            decoder->videoCodecContext->flags |= AV_CODEC_FLAG_LOW_DELAY;
            decoder->videoStreamIdx = 0;
            decoder->videoSize = avpicture_get_size(AV_PIX_FMT_YUV420P, decoder->videoWidth, decoder->videoHeight);
            decoder->videoBufferSize = 3 * decoder->videoSize;
            decoder->yuvBuffer = (unsigned char *)av_mallocz(decoder->videoBufferSize);
            simpleLog("Open video codec context success, video stream index %d %x. %d*%d codecId:%d",
                decoder->videoStreamIdx, (unsigned int)decoder->videoCodecContext, decoder->videoWidth, decoder->videoHeight, codecId);
        }

        r = avcodec_decode_video2(decoder->videoCodecContext, decoder->avFrame, &gotPicture, pkt);
        if (r < 0 || !gotPicture ) {
            simpleLog("Error sending a packet for decoding %d.", r);
            //todo need iframe
            return kErrorCodeFFmpegError;
        }

        if (decoder->avFrame->width != decoder->videoWidth || decoder->avFrame->height != decoder->videoHeight) {
            simpleLog("frame size changed from %d*%d to %d*%d.", decoder->videoWidth,
                decoder->videoHeight, decoder->avFrame->width, decoder->avFrame->height);
            decoder->videoWidth = decoder->avFrame->width;
        	decoder->videoHeight = decoder->avFrame->height;
            av_freep(&decoder->yuvBuffer);
            decoder->videoSize = avpicture_get_size(AV_PIX_FMT_YUV420P, decoder->videoWidth, decoder->videoHeight);
            decoder->videoBufferSize = 3 * decoder->videoSize;
            decoder->yuvBuffer = (unsigned char *)av_mallocz(decoder->videoBufferSize);
        }

        pSize = decoder->yuvBuffer;
        for (i = 0; i < 3; i++) {
            shift = i > 0 ? 1 : 0;
            yuv = decoder->avFrame->data[i];
            for (j = 0; j < decoder->avFrame->height >> shift; j++) {
                memcpy(pSize, yuv, decoder->avFrame->width >> shift);
                pSize += decoder->avFrame->width >> shift;
                yuv += decoder->avFrame->linesize[i];
            }
        }
        
        timestamp = header->time_usec;
        decoder->videoCallback(decoder->yuvBuffer, decoder->videoSize, timestamp, decoder->videoWidth, decoder->videoHeight);
    } else if (header->strm_type == ST_AUDIO) {
        if (!decoder->audioCodecContext) {
            //open audio codec first
            codecId = getDecodeAudioType(header->audio_codec);
            r = openCodecContext(AVMEDIA_TYPE_AUDIO, codecId, &decoder->audioCodecContext);
            if (r != 0) {
                ret = kErrorCodeFFmpegError;
                simpleLog("Open audio codec context failed %d.", ret);
                return ret;
            }
            decoder->audioStreamIdx = 1;
            simpleLog("Open audio codec context success, audio stream index %d %x.",
                decoder->audioStreamIdx, (unsigned int)decoder->audioCodecContext);

            simpleLog("Audio stream index:%d sample_fmt:%d channel:%d, sample rate:%d.",
                decoder->audioStreamIdx,
                decoder->audioCodecContext->sample_fmt,
                decoder->audioCodecContext->channels,
                decoder->audioCodecContext->sample_rate);
        }

        //ret = processDecodedAudioFrame(decoder->avFrame);
        
    } else {
        return kErrorCodeInvalidFormat;
    }

    return kErrorCodeSuccess;
}

ErrorCode closeDecoder() {
    ErrorCode ret = kErrorCodeSuccess;
    do {
        if (decoder == NULL || decoder->avformatContext == NULL) {
            break;
        }

        if (decoder->videoCodecContext != NULL) {
            closeCodecContext(decoder->avformatContext, decoder->videoCodecContext, decoder->videoStreamIdx);
            decoder->videoCodecContext = NULL;
            simpleLog("Video codec context closed.");
        }

        if (decoder->audioCodecContext != NULL) {
            closeCodecContext(decoder->avformatContext, decoder->audioCodecContext, decoder->audioStreamIdx);
            decoder->audioCodecContext = NULL;
            simpleLog("Audio codec context closed.");
        }

        AVIOContext *pb = decoder->avformatContext->pb;
        if (pb != NULL) {
            if (pb->buffer != NULL) {
                av_freep(&pb->buffer);
            }
            av_freep(&decoder->avformatContext->pb);
            simpleLog("IO context released.");
        }

        avformat_close_input(&decoder->avformatContext);
        decoder->avformatContext = NULL;
        simpleLog("Input closed.");

        if (decoder->yuvBuffer != NULL) {
            av_freep(&decoder->yuvBuffer);
        }

        if (decoder->pcmBuffer != NULL) {
            av_freep(&decoder->pcmBuffer);
        }
        
        if (decoder->avFrame != NULL) {
            av_freep(&decoder->avFrame);
        }
        simpleLog("All buffer released.");
    } while (0);
    return ret;
}

//////////////////////////////////Export methods////////////////////////////////////////
ErrorCode initDecoder(int logLv, long videoCallback, long audioCallback, int frameLimit, int sizeLimit) {
    ErrorCode ret = kErrorCodeSuccess;
    do {
        //Log level.
        logLevel = logLv;

        if (decoder != NULL) {
            simpleLog("Decoder already initialized.");
            break;
        }

        av_register_all();
        avcodec_register_all();
        
        if (logLevel == kLogLevel_All) {
            av_log_set_callback(ffmpegLogCallback);
        }

        decoder = (WebDecoder *)av_mallocz(sizeof(WebDecoder));
        decoder->run = 1;
        decoder->decode = 0;
        decoder->audioOpen = 0;
        decoder->format = 1;
        decoder->videoCallback = (VideoCallback)videoCallback;
        decoder->audioCallback = (AudioCallback)audioCallback;
        decoder->listSize = 0;
        decoder->listCount = 0;
        decoder->iFrameFlag = 1;
        if (frameLimit > 0) {
            kMaxPacketCount = frameLimit;
        }

        if (sizeLimit > 0) {
            kMaxPacketSize = sizeLimit;
        }
        decoder->avFrame = av_frame_alloc();
        pthread_mutex_init(&decoder->listMutex, NULL);
    } while (0);
    simpleLog("Decoder initialized logLv:%d limit:%d %d ret:%d.", logLv, frameLimit, sizeLimit, ret);
    return ret;
}

ErrorCode uninitDecoder() {
    if (decoder != NULL) {
        decoder->run = 0;
        decoder->decode = 0;
        decoder->audioOpen = 0;
        decoder->format = 0;
        decoder->iFrameFlag = 1;
        packet_list_clear();
        closeDecoder();
        pthread_mutex_destroy(&decoder->listMutex);
        av_freep(&decoder);
    }

    av_log_set_callback(NULL);
    simpleLog("Decoder uninitialized.");
    return kErrorCodeSuccess;
}

int sendData(unsigned char *buff, int size)
{
    return packet_list_insert(buff, size);
}

int setDecoderLimit(int frameLimit, int sizeLimit)
{
    simpleLog("Decoder set limit frame:%d size:%d", frameLimit, sizeLimit);
    if (frameLimit > 0) {
        kMaxPacketCount = frameLimit;
    }

    if (sizeLimit > 0) {
        kMaxPacketSize = sizeLimit;
    }

    packet_list_clear();
    return kErrorCodeSuccess;
}

ErrorCode decodeOnePacket()
{
    ErrorCode ret	= kErrorCodeSuccess;
    int decodedLen	= 0;
    int r			= 0;
    struct WebPacketNode *node;

    do {
        if (decoder == NULL) {
            ret = kErrorCodeInvalidState;
            break;
        }

        node = packet_list_popfront();
        if (!node) {
            break;
        }
        
        if (node->header.strm_type != ST_VIDEO && node->header.strm_type != ST_AUDIO) {
            packet_list_free(node);
            break;
        }

        //simpleLog("decodeOnePacket %d %d", node->header.strm_type, node->size);
        ret = decodePacket(&node->header, &node->avpkt);
        packet_list_free(node);
    } while (0);
    return ret;
}

int main() {
    simpleLog("Native loaded.");
    return 0;
}


#ifdef __cplusplus
}
#endif
