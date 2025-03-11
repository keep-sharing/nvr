#ifndef _____MS_RTSP_SERVER_STREAM_H__
#define _____MS_RTSP_SERVER_STREAM_H__

#include "ms_stream_common.h"
#include "ms_linkedlists.h"

#define DEFAULT_STREAM_VID (0)
#define DEFAULT_STREAM_AUD (1)
#define DEFAULT_STREAM_MED (2)

//#include "time64.h"

#define MS_AIO_THREAD       (4)
#define MS_MAX_UDP_PACKET   (1450-16)
#define MX_MAX_RTSP_CTX     (32)
#define MAX_CTX_DATA_SIZE   (3*1024*1024)

#define WEBSOCKET_GUID					"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define MS_AUTH_URI                     "Authorization:"
#define MS_RTSP_SERVER                  "RECOVISION Streaming Service"
#define ms_http_url_header              "/ms/streaming/"
#define ms_http_rtsp_url_header         "/ms/webstream/"
#define ms_http_over_rtsp_url_header    "/ms/httpstream/"

#define MS_RTSP_URL_KEY_MULTICAST       "&mcast"
#define MS_RTSP_URL_KEY_VERSION         "&version="
#define MS_RTSP_MULTICAST_PORT_BEGIN    7034

typedef enum {
    MS_RTSP_FRAME_VERSION_NONE = 0,
    MS_RTSP_FRAME_VERSION_2    = 2,//转发IPC&NVR自定义帧
} MS_RTSP_FRAME_VERSION_E;


typedef struct Ms_Rtp_frame{
    int chnid;      //channels id
    int strmType;   //ST_VIDEO & ST_AUDIO
    int frameType;   // FT_IFRAME &   FT_PFRAME
    
    //Timestamp
    uint64_t time_usec;
    uint32_t time_lower;
    uint32_t time_upper;
    
    void *data;
    int nDataSize;
    void *httpHeader;
    int nHttpHeaderSize;
}MS_RTP_FRAME;

typedef struct Ms_Aio_DataNode {
    void *pCtx;
    char *data;
    int size;
    int offset;
    MS_AST_LIST_ENTRY(Ms_Aio_DataNode) list;
}MS_AIO_DATANODE;

typedef struct Ms_Rtp_Source {
    void *owner;
    uint32_t m_ssrc;
    uint32_t m_timestamp;
    uint32_t m_transtype;
    uint32_t m_interleaved[2];
    uint64_t m_rtcp_clock;

    void *m_rtsp;
    void *m_rtp;
    void *m_encoder;
    void *m_transport;

    //void *m_post_rtsp;
    char m_sname[32];
    char sessionId[32];
    unsigned char m_packet[MS_MAX_UDP_PACKET + 14];
} MS_RTP_SOURCE;

typedef struct Ms_Rtp_Context {
    void *owner;
    int status;
    MS_MUTEX_OBJECT ctxMutex;
    int count;
    int connType;           //rtsp | http
    int transType;          //TCP  | UDP
    int iframeKey;

    int aioSend;
    int aioBuffSize;
    int bandwidth;
    
    char sessionId[32];
    char xSessionCookie[32];

    //Timestamp
    uint64_t time_usec;
    uint32_t time_lower;
    uint32_t time_upper;

    MS_RTP_SOURCE video;
    MS_RTP_SOURCE audio;
    MS_RTP_SOURCE metadata;

    //mcast
    int mcast;
    MS_RTSP_FRAME_VERSION_E frameVersion;
    char userName[64];
    int userLevel;
    int audioPerm;

    int maxDataSize;
    char name[32];
    char ip[64];

    MS_AIO_DATANODE *curPacket;
    int lastIframe;

    int datalistSize;
    int datalistCnt;
    MS_AST_LIST_HEAD_NOLOCK(ctxdatalist, Ms_Aio_DataNode) ctxdatalist;
} MS_RTP_CONTEXT;

typedef struct rtsp_server_stream{
    char name[64];
    int id;
    int nInit;
    int state;
    MS_MUTEX_OBJECT hdlMutex;
    int nb_streams;
    
    int chnid;
    int originVcodeType;
    int vCodeType;
    int bandWidth;
    int width;
    int height;
    
    int originAcodeType;
    int aCodeType;
    int samplerate;
    
    int exSize;
    char extraData[256];
    
    int ctxCnts;//MS_CTX_OK count
    MS_RTP_CONTEXT *ctx[MX_MAX_RTSP_CTX];
    uint32_t upTimeUsec[MAX_MS_StreamStrmType];

    int isPlayback;
    MS_RTP_CONTEXT *mcastCtx;
    int maxDataSize;
    
    void *arg;
    int (*cb_server_state)(int state, void *arg);
    MS_AST_LIST_ENTRY(rtsp_server_stream) list;  

    //last iframe
    MS_RTP_FRAME *lastIframe;
    
}RTSP_SERVER_STREAM;

struct MsRtspPermissions{
    char username[33];
    int userLevel;
    int liveviewAudio;
    char liveviewMask[65];
    int playbackAudio;
    char playbackMask[65];
};

typedef struct ms_aio_ready_send{
    void *ctx;
    void *rtsp;
    int action;  /*0 Add ready; 1 free Ctx buffer*/
    MS_AST_LIST_ENTRY(ms_aio_ready_send) list;    
}MS_AIO_READY_SEND;

typedef struct rtsp_msstream_conf {
    int port;
    int bandwidth;

    void (*ms_rp_malloc)(void **dst, int size, const char *file, int line);
    void (*ms_rp_calloc)(void **dst, int n, int size, const char *file, int line);
    void (*ms_rp_free)(void *src, int size, const char *file, int line);
    int (*ms_rp_permissions)(char *auth, char *nonce, char *cmd, struct MsRtspPermissions *perm);
    int (*ms_rp_access_filter)(const char *ip);
    void (*ms_rp_print)(int level, const char *file, const char *func, int line, char *msg);
    
}RTSP_MSSTREAM_CONF;

#ifdef __cplusplus
extern "C"
{
#endif

/////////start////////
uint32_t ms_rtp_ssrc(void);

void rtsp_server_task(void *params);
int ms_rtsp_handle_senddata(void *hdl, MS_RTP_FRAME *rtpframe);

void *create_rtsp_server_task(RTSP_MSSTREAM_CONF *conf);
void destory_rtsp_server_task(void *sock);

int ms_streamlist_find_nbstream(int id);
void *ms_streamlist_insert(RTSP_SERVER_STREAM *node);
void ms_streamlist_remove(RTSP_SERVER_STREAM *node);
void ms_streamlist_free(RTSP_SERVER_STREAM* node);
void ms_streamlist_exchange(RTSP_SERVER_STREAM *node);
void ms_streamlist_clear();
void ms_streamlist_show();
void ms_stream_channel_show(char *name);

RTSP_SERVER_STREAM* ms_streamlist_popfront();
RTSP_SERVER_STREAM *ms_streamlist_find_name(const char *name);

void *ms_removelist_insert(RTSP_SERVER_STREAM *node);
RTSP_SERVER_STREAM *ms_removelist_popfront();
void ms_removelist_free(RTSP_SERVER_STREAM *node);

MS_StreamCodecType ms_translate_forward(int strm_type, int codeType);
MS_StreamStrmType ms_translate_strm_type(int strm_type);

void *ms_rtsp_pool_malloc_ex(int size, const char *file, int line);
void *ms_rtsp_pool_calloc_ex(int n, int size, const char *file, int line);
void ms_rtsp_pool_free_ex(void *p, int size, const char *file, int line);

#define ms_rtsp_pool_malloc(n, m)  ms_rtsp_pool_malloc_ex(n, __FILE__, __LINE__)
#define ms_rtsp_pool_calloc(n, m)  ms_rtsp_pool_calloc_ex(n, m, __FILE__, __LINE__)
#define ms_rtsp_pool_free(n, m)  ms_rtsp_pool_free_ex(n, m, __FILE__, __LINE__)


void ms_rtsp_set_multicast(int enable, char *ip);
void ms_rtsp_update_filter();
void ms_rtsp_server_set_debug(int enable);
int ms_rtsp_server_update_bandwidth(RTSP_SERVER_STREAM *stream, int bandwidth);

/////////end////////

#ifdef __cplusplus
}
#endif

#endif
