#ifndef _____MS_RTSP_CLIENT_STREAM_H__
#define _____MS_RTSP_CLIENT_STREAM_H__

#include "ms_stream_common.h"
#include "nghttp2/nghttp2.h"
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void* ms_stream_rtspclient_handler;

MS_StreamErrorCode ms_stream_init();
MS_StreamErrorCode ms_stream_uninit();

ms_stream_rtspclient_handler 
ms_rtsp_client_create(const char* url, const char* name,const char* path, MS_StreamTransportType transprtl, void *param);

MS_StreamErrorCode
ms_rtsp_client_openstream(ms_stream_rtspclient_handler rtsp_client_context);

MS_StreamErrorCode 
ms_rtsp_client_destroy(ms_stream_rtspclient_handler rtsp_client_context);

typedef void (*MS_RTSP_CLIENT_DataCallback)(struct Ms_StreamPacket *packet, void* opaque);
typedef void (*MS_RTSP_CLIENT_EventCallback)(MS_StreamEventType event_type, struct Ms_StreamInfo *stream_info, void* opaque);

MS_StreamErrorCode ms_rtsp_client_setDataCallback(ms_stream_rtspclient_handler handle, MS_RTSP_CLIENT_DataCallback callback);

MS_StreamErrorCode ms_rtsp_client_setEventCallback(ms_stream_rtspclient_handler handle, MS_RTSP_CLIENT_EventCallback callback);

int ms_rtsp_client_get_extradate(ms_stream_rtspclient_handler *handle, char *dst, int *ndstlen);
int ms_rtsp_client_dosleep(ms_stream_rtspclient_handler rtsp_client_context, int dosleep);
void ms_rtsp_client_set_debug(int enable);
void ms_rtsp_client_set_app_log(int enable);



#ifdef __cplusplus
}
#endif

#endif
