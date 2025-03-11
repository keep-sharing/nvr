#ifndef _____MS_STREAM_COMMON_H__
#define _____MS_STREAM_COMMON_H__

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

//lock mutex
#define MS_TASK_OK      (0)
#define MS_TASK_ERROR   (-1)
#define MS_MUTEX_OBJECT pthread_mutex_t
#define MS_COND_OBJECT  pthread_cond_t
#define MS_TASK_HANDLE  pthread_t

#define WEBSOCKET_MIN_OPCODE_LEN        126
#define WEBSOCKET_MEDIUM_OPCODE_LEN     127
#define WEBSOCKET_MEDIUM_LEN            65535
#define WEBSOCKET_TEXT_OPCODE           0x81
#define WEBSOCKET_BINARY_OPCODE         0x82

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(HAVE_STDINT_H) || __GNUC__ >= 4 || _MSC_VER > 1500
#  include <stdint.h>  // for int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t;
#else
#  if defined(_MSC_VER)
typedef signed __int8       int8_t;
typedef unsigned __int8     uint8_t;
typedef signed __int16      int16_t;
typedef unsigned __int16    uint16_t;
typedef signed __int32      int32_t;
typedef unsigned __int32    uint32_t;
typedef signed __int64      int64_t;
typedef unsigned __int64    uint64_t;
#  else
#  endif
#endif

typedef enum MS_StreamStrmType {
    MS_StreamStrmType_VIDEO = 0,
    MS_StreamStrmType_AUDIO = 1,
    MS_StreamStrmType_METAD = 2,
    MAX_MS_StreamStrmType,
} MS_StreamStrmType;

typedef enum MS_StreamCtxConnType {
    MS_StreamCtxConnType_RTSP = 0,
    MS_StreamCtxConnType_HTTP = 1,
    MS_StreamCtxConnType_WEBSOCK = 2,
    MS_StreamCtxConnType_ROHTTP = 3,
} MS_StreamCtxConnType;

typedef enum MS_StreamErrorCode {
    MS_StreamErrorCode_FAILED = -1,
    MS_StreamErrorCode_SUCCESS = 0,
} MS_StreamErrorCode;

typedef enum MS_StreamTransportType {
    MS_StreamTransportType_UDP,
    MS_StreamTransportType_TCP,
    MS_StreamTransportType_ATUO,
    MS_StreamTransportType_RTSP_OVER_HTTPS,
} MS_StreamTransportType;

typedef enum MS_StreamCodecType {
//video codec
    MS_StreamCodecType_H264 = 0,
    MS_StreamCodecType_H265,
    MS_StreamCodecType_MJPEG,
    MS_StreamCodecType_MPEG4,
//audio codec
    MS_StreamCodecType_G711_ULAW = 1000,
    MS_StreamCodecType_G711_ALAW,
    MS_StreamCodecType_AAC,
    MS_StreamCodecType_G722,
    MS_StreamCodecType_G726,
//user define codec
    MS_StreamCodecType_ONVIF_METADATA = 2000,
//ms define metadata
    MS_StreamCodecType_MS_METADATA = 2001,
} MS_StreamCodecType;

typedef enum MS_RTP_CONTEXT_Type {
    MS_CTX_STOP = -1,
    MS_CTX_INIT = 0,
    MS_CTX_OK = 1,
    MS_CTX_REMOVE = 2,
} MS_RTP_CONTEXT_TYPE;

typedef enum MS_Aio_Ready_Action {
    MS_AIO_ADD_TASK = 0,
    MS_AIO_DEL_BUFF = 1,
} MS_AIO_READY_ACTION;

struct Ms_StreamPacket {
    uint8_t            *data_buffer;       ///frame data
    uint32_t            data_size;         ///frame data_size

    uint32_t            i_flags;        ///1 is iframe
    MS_StreamCodecType  i_codec;        ///< 编码类型FOURCC('a','b','c','d')
    int64_t             i_pts;          ///timestamp
};

struct Ms_StreamInfo {
    MS_StreamCodecType  video_codec;
    uint32_t            video_width;
    uint32_t            video_height;
    MS_StreamCodecType  audio_codec;
    uint32_t            audio_sample;
    uint32_t            videoSample;
    uint32_t            metaSample;
};

typedef enum MS_StreamEventType {
    MS_StreamEventType_WAITTING,
    MS_StreamEventType_CONNECT,
    MS_StreamEventType_DISCONNECT,
    MS_StreamEventType_CODECCHANGE,
    MS_StreamEventType_BANDWIDTH,//ms ipc Not Enough Bandwidth
    MS_StreamEventType_UNAUTH,//401 Unauthorized
    MS_StreamEventType_NETWORK,
} MS_StreamEventType;

typedef struct http_frame_packet_header_s {
    int boundary;             //0x00112233
    int version;              //version
    char hsession[32];        //http session id
    unsigned int header_size; // header size
    int ch;                   // ch number
    int strm_type;            // strm_type: video data or audio data
    unsigned int size;        // size

    // for video
    int codec_type;           // video codec type
    int frame_type;           // i-frame or p-frame
    int width;
    int height;

    //for audio
    int audio_codec;         // -1 for no audio
    int sample;

    unsigned long long time_usec;
} http_frame_packet_header_t;

void parse_codec_type(const char *encoding, MS_StreamCodecType *codec_type);

//debug api start
typedef enum
{
    RTSP_INFO   = 0,
    RTSP_DEBUG  = 1,
    RTSP_WARN   = 2,
    RTSP_ERR    = 3,
    RTSP_FATAL  = 4
} RTSP_DEBUG_LEVEL; //@ TSAR_DEBUG_LEVEL

typedef struct {
    int enable;
    RTSP_DEBUG_LEVEL level;
} RTSP_STREAM_DEBUG;

typedef struct RtspStreamConf {
    RTSP_STREAM_DEBUG client;
    RTSP_STREAM_DEBUG server;
    void (*print)(int level, const char *file, const char *func, int line, char *msg);
} RTSP_STREAM_CONF;

extern RTSP_STREAM_CONF g_rtspComonConf;

void ms_time_to_string_ex(char *stime, int nLen);
void ms_rtsp_log(int level, const char *file, const char *func, int line, const char* format, ...);
void set_rtsp_stream_client_debug(int enable);
void set_rtsp_stream_server_debug(int enable);
void set_rtsp_stream_print_func(void *print);

#define ms_rtsp_print(format, ...) \
    do{\
        char _stime[32] = {0};\
        ms_time_to_string_ex(_stime, sizeof(_stime));\
        printf("%s %s %s:%d " format "\n", \
          _stime, __FILE__, __func__, __LINE__, ##__VA_ARGS__); \
    }while(0)

#define rtsp_server_debug(_level, _format, ...) \
    do{\
        if (g_rtspComonConf.server.enable) {\
            if (g_rtspComonConf.print) {\
                ms_rtsp_log(_level, __FILE__, __func__, __LINE__, "[RTSP_SERVER] " _format, ## __VA_ARGS__);\
            } else {\
                ms_rtsp_print("[RTSP_SERVER] " _format, ##__VA_ARGS__);\
            }\
        }\
    }while(0)

#define rtsp_client_debug(_level, _format, ...) \
    do{\
        if (g_rtspComonConf.client.enable) {\
            if (g_rtspComonConf.print) {\
                ms_rtsp_log(_level, __FILE__, __func__, __LINE__, "[RTSP_CLIENT] " _format, ## __VA_ARGS__);\
            } else {\
                ms_rtsp_print("[RTSP_CLIENT] " _format, ##__VA_ARGS__);\
            }\
        }\
    }while(0)
//debug api end


int ms_get_websocket_header_opcode(char *dst, int *dstlen, uint64_t size, int type);
int ms_com_mutex_init(MS_MUTEX_OBJECT *mutex);
void ms_com_mutex_lock(MS_MUTEX_OBJECT *mutex);
int ms_com_mutex_trylock(MS_MUTEX_OBJECT *mutex);
int ms_com_mutex_timelock(MS_MUTEX_OBJECT *mutex, int us);
void ms_com_mutex_unlock(MS_MUTEX_OBJECT *mutex);
void ms_com_mutex_uninit(MS_MUTEX_OBJECT *mutex);

#ifdef __cplusplus
}
#endif

#endif
