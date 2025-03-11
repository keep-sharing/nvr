/*
 * Filename:        ms_rtsp_server_stream.c
 * Created at:      2021.07.06
 * Description:     media server - rtsp server
 * Author:          david
 * Copyright (C)    milesight
 * *******************************************************/
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "sys/thread.h"
#include "sys/system.h"
#include "openssl/md5.h"
#include "openssl/sha.h"

#include "rtp.h"
#include "rtp-profile.h"
#include "rtp-payload.h"
#include "time64.h"
#include "base64.h"
#include "aio-worker.h"
#include "rtsp-server.h"
#include "rtsp-server-aio.h"

#include "uri-parse.h"
#include "urlcodec.h"
#include "cstringext.h"

#include "ms_parse_sps.h"

#include "ms_rtp_udp_transport.h"
#include "ms_rtsp_server_stream.h"

#define MS_RTSP_PREVIEW_URL     "ch_"
#define MS_RTSP_PLAYBACK_URL    "ms/playback_"

#define MS_RTSP_IFRAME_BUFFER   0

static void *ms_rtp_alloc(void *param, int bytes);
static void ms_rtsp_free(void *param, void *packet);
static void ms_rtp_packet(void *param, const void *packet, int bytes, uint32_t timestamp, int flags);
static void ms_rtp_packet_ptr(void *param, void **rtp_packet);
static int creat_multicast_ctx(RTSP_SERVER_STREAM *stream);
static void destroy_multicast_ctx(RTSP_SERVER_STREAM *stream);
static inline void ms_stop_rtp_context(RTSP_SERVER_STREAM *stream, MS_RTP_CONTEXT *ctx);
static void ms_ctx_datalist_clear(MS_RTP_CONTEXT *ctx);

enum {
    MS_RTSP_PLUS    = 0,
    MS_RTSP_MINUS   = 1,
    MS_RTSP_CHANGED = 2,
};

static struct rtp_payload_t ms_rtpfunc = {
    ms_rtp_alloc,
    ms_rtsp_free,
    ms_rtp_packet,
    ms_rtp_packet_ptr
};

struct rtsp_server_stream_conf {
    int loaded;
    int port;

    int cur_rtsp_bandwidth;
    int cur_rtsp_ctxcnt;
    
    int max_rtsp_bandwidth;
    int max_rtsp_ctxcnt;
    
    int rtsp_server_thread_run;
    MS_TASK_HANDLE rtsp_server_thread_handle;

    MS_MUTEX_OBJECT stream_mutex;
    MS_MUTEX_OBJECT remove_mutex;
    MS_MUTEX_OBJECT aioready_mutex;
    MS_MUTEX_OBJECT bandwidth_mutex;
    
    void (*ms_rp_malloc)(void **dst, int size, const char *file, int line);
    void (*ms_rp_calloc)(void **dst, int n, int size, const char *file, int line);
    void (*ms_rp_free)(void *src, int size, const char *file, int line);
    int (*ms_rp_permissions)(char *auth, char *nonce, char *cmd, struct MsRtspPermissions *perm);
    int (*ms_rp_access_filter)(const char *ip);

    //multicast
    int mcastEnable;
    char mcastIp[65];
};

typedef enum RtspCbReqNo
{
    RTSP_REQ_DEFAULT_RESP = -100,
    RTSP_REQ_NONE = 0,
    RTSP_REQ_OPEN_PB,
    RTSP_REQ_CLOSE_PB,
    RTSP_REQ_MAX,
}RTSP_CB_REQ_NO;

typedef struct RtspCbReq {
    RTSP_CB_REQ_NO req;
    char buf[128];
} RTSP_CB_REQ_S;

typedef struct RtspCbResp
{
    char buf[128];
} RTSP_CB_RESP_S;


static struct rtsp_server_stream_conf g_rtsp_conf;
static MS_AST_LIST_HEAD_NOLOCK_STATIC(streamlist, rtsp_server_stream);
static MS_AST_LIST_HEAD_NOLOCK_STATIC(removelist, rtsp_server_stream);
static MS_AST_LIST_HEAD_NOLOCK_STATIC(aioreadylist, ms_aio_ready_send);

typedef int (*CbRtspServerReq)(struct RtspCbReq *req, struct RtspCbResp *resp);
CbRtspServerReq cb_rtsp_server_req = NULL;

static inline void ms_change_bandwidth(int type, int bandwidth)
{
    if (!bandwidth) {
        rtsp_server_debug(RTSP_INFO, "bandwidth invalid.");
        return;
    }
    
    ms_com_mutex_lock(&g_rtsp_conf.bandwidth_mutex);
    if (type == MS_RTSP_PLUS) {
        g_rtsp_conf.cur_rtsp_bandwidth += bandwidth;
        g_rtsp_conf.cur_rtsp_ctxcnt++;
    } else if (type == MS_RTSP_MINUS) {
        g_rtsp_conf.cur_rtsp_bandwidth -= bandwidth;
        g_rtsp_conf.cur_rtsp_ctxcnt--;
        if (g_rtsp_conf.cur_rtsp_bandwidth < 0) {
            rtsp_server_debug(RTSP_INFO, "bandwidth maybe double minus.");
            g_rtsp_conf.cur_rtsp_bandwidth = 0;
        }
        if (g_rtsp_conf.cur_rtsp_ctxcnt < 0) {
            rtsp_server_debug(RTSP_INFO, "ctxcnt maybe double minus.");
            g_rtsp_conf.cur_rtsp_ctxcnt = 0;
        }
    } else if (type == MS_RTSP_CHANGED) {
        //bandwidth可能是负数
        g_rtsp_conf.cur_rtsp_bandwidth += bandwidth;
    }
    rtsp_server_debug(RTSP_INFO, "current:%d max:%d bandWidth:%d cnt:%d", 
        g_rtsp_conf.cur_rtsp_bandwidth, g_rtsp_conf.max_rtsp_bandwidth, bandwidth, g_rtsp_conf.cur_rtsp_ctxcnt);
    ms_com_mutex_unlock(&g_rtsp_conf.bandwidth_mutex);
}

//bandwidth可能是负数
static inline int ms_bandwidth_limit(int type, int bandwidth)
{
    int ret = 0;

    ms_com_mutex_lock(&g_rtsp_conf.bandwidth_mutex);
    if (type == MS_RTSP_PLUS && ((g_rtsp_conf.cur_rtsp_ctxcnt + 1) > g_rtsp_conf.max_rtsp_ctxcnt)) {
        rtsp_server_debug(RTSP_INFO, "bandwidth not enough. ctx limit %d curren:%d", 
                       g_rtsp_conf.max_rtsp_ctxcnt, g_rtsp_conf.cur_rtsp_ctxcnt);
        ret = 1;
    }
    if ((g_rtsp_conf.cur_rtsp_bandwidth + bandwidth) > g_rtsp_conf.max_rtsp_bandwidth) {
        rtsp_server_debug(RTSP_INFO, "bandwidth not enough. bandwidth limit:%d current:%d need:%d",
                      g_rtsp_conf.max_rtsp_bandwidth, g_rtsp_conf.cur_rtsp_bandwidth, bandwidth);
        ret = 1;
    }
    ms_com_mutex_unlock(&g_rtsp_conf.bandwidth_mutex);
    
    return ret;
}


/****** stream list *****/

void *ms_rtsp_pool_malloc_ex(int size, const char *file, int line)
{
    void *pbuffer = NULL;
    if (g_rtsp_conf.loaded) {
        g_rtsp_conf.ms_rp_malloc(&pbuffer, size, file, line);
    } else {
        pbuffer = calloc(1, size);
    }
    return pbuffer;
}

void *ms_rtsp_pool_calloc_ex(int n, int size, const char *file, int line)
{
    void *pbuffer = NULL;

    if (g_rtsp_conf.loaded) {
        g_rtsp_conf.ms_rp_calloc(&pbuffer, n, size, file, line);
    } else {
        pbuffer = calloc(n, size);
    }

    return pbuffer;
}

void ms_rtsp_pool_free_ex(void *p, int size, const char *file, int line)
{
    if (!p) {
        return;
    }

    if (g_rtsp_conf.loaded) {
        g_rtsp_conf.ms_rp_free(p, size, file, line);
    } else {
        free(p);
    }

    return ;
}

void *ms_streamlist_insert(RTSP_SERVER_STREAM *node)
{
    RTSP_SERVER_STREAM *stream;
    
    if (!node) {
        rtsp_server_debug(RTSP_WARN, "streamlist insert params is err.");
        return NULL;
    }

    stream = ms_streamlist_find_name(node->name);
    if (stream) {
        rtsp_server_debug(RTSP_WARN, "streamlist insert name %s is exist.", node->name);
        return NULL;
    }

    RTSP_SERVER_STREAM *pNode = ms_rtsp_pool_calloc(1, sizeof(RTSP_SERVER_STREAM));
    if (!pNode) {
        rtsp_server_debug(RTSP_WARN, "streamlist insert malloc is err.");
        return NULL;
    }

    memset(pNode, 0, sizeof(RTSP_SERVER_STREAM));
    memcpy(pNode, node, sizeof(RTSP_SERVER_STREAM));

    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_INSERT_TAIL(&streamlist, pNode, list);
    if (g_rtsp_conf.mcastEnable) {
        ms_com_mutex_lock(&pNode->hdlMutex);
        creat_multicast_ctx(pNode);
        ms_com_mutex_unlock(&pNode->hdlMutex);
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return pNode;
}

void ms_streamlist_remove(RTSP_SERVER_STREAM *node)
{
    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_REMOVE(&streamlist, node, list);
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);
}

RTSP_SERVER_STREAM *ms_streamlist_popfront()
{
    RTSP_SERVER_STREAM *node = NULL;
    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    if (MS_AST_LIST_EMPTY(&streamlist)) {
        ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);
        return node;
    }

    node = MS_AST_LIST_FIRST(&streamlist);
    if (node) {
        MS_AST_LIST_REMOVE(&streamlist, node, list);
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return node;
}

void ms_streamlist_free(RTSP_SERVER_STREAM *node)
{
    if (!node) {
        return ;
    }

    ms_rtsp_pool_free(node, sizeof(RTSP_SERVER_STREAM));
    return ;
}

RTSP_SERVER_STREAM *ms_streamlist_find_name(const char *name)
{
    RTSP_SERVER_STREAM *tmp  = NULL;
    RTSP_SERVER_STREAM *dst  = NULL;
    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, tmp, list) {
        if (!tmp) {
            continue;
        }
        if (!strcmp(tmp->name, name)) {
            dst = tmp;
            break;
        }
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return dst;
}

int ms_streamlist_find_nbstream(int id)
{
    int value = 0;
    RTSP_SERVER_STREAM *tmp  = NULL;

    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, tmp, list) {
        if (!tmp) {
            continue;
        }
        if (tmp->id == id) {
            value = tmp->nb_streams;
            break;
        }
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return value;
}

void ms_streamlist_exchange(RTSP_SERVER_STREAM *node)
{
    if (!node) {
        return ;
    }

    int i = 0;
    ms_streamlist_remove(node);
    ms_com_mutex_lock(&node->hdlMutex);
    for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
        if (!node->ctx[i]) {
            continue;
        }
        ms_com_mutex_lock(&node->ctx[i]->ctxMutex);
        if (node->ctx[i]->status == MS_CTX_OK) {
            node->ctx[i]->status = MS_CTX_REMOVE;
        }
        ms_com_mutex_unlock(&node->ctx[i]->ctxMutex);
    } 
    ms_com_mutex_unlock(&node->hdlMutex);
    ms_removelist_insert(node);

    return ;
}

void ms_streamlist_clear()
{
    while (1) {
        RTSP_SERVER_STREAM *node = ms_streamlist_popfront();
        if (!node) {
            break;
        }
        ms_removelist_insert(node);
    }

    return ;
}

void *ms_removelist_insert(RTSP_SERVER_STREAM *node)
{
    if (!node) {
        rtsp_server_debug(RTSP_WARN, "streamlist insert params is err.");
        return NULL;
    }
    ms_com_mutex_lock(&g_rtsp_conf.remove_mutex);
    MS_AST_LIST_INSERT_TAIL(&removelist, node, list);
    ms_com_mutex_unlock(&g_rtsp_conf.remove_mutex);

    return node;
}

RTSP_SERVER_STREAM *ms_removelist_popfront()
{
    RTSP_SERVER_STREAM *node = NULL;
    ms_com_mutex_lock(&g_rtsp_conf.remove_mutex);
    if (MS_AST_LIST_EMPTY(&removelist)) {
        ms_com_mutex_unlock(&g_rtsp_conf.remove_mutex);
        return node;
    }

    node = MS_AST_LIST_FIRST(&removelist);
    if (node) {
        MS_AST_LIST_REMOVE(&removelist, node, list);
    }
    ms_com_mutex_unlock(&g_rtsp_conf.remove_mutex);
    return node;
}

static inline void del_stream_iframe(MS_RTP_FRAME *lastIframe)
{
    if (lastIframe) {
        if (lastIframe->data) {
            ms_rtsp_pool_free(lastIframe->data, lastIframe->nDataSize);
        }
        if (lastIframe->httpHeader) {
            ms_rtsp_pool_free(lastIframe->httpHeader, lastIframe->nHttpHeaderSize);
        }
        
        ms_rtsp_pool_free(lastIframe, sizeof(MS_RTP_FRAME));
    }
}

static inline void update_stream_iframe(RTSP_SERVER_STREAM *stream, MS_RTP_FRAME *lastIframe)
{
    if (stream->lastIframe) {
        if (stream->lastIframe->data) {
            ms_rtsp_pool_free(stream->lastIframe->data, stream->lastIframe->nDataSize);
        }
        if (stream->lastIframe->httpHeader) {
            ms_rtsp_pool_free(stream->lastIframe->httpHeader,stream->lastIframe->nHttpHeaderSize);
        }
        
        ms_rtsp_pool_free(stream->lastIframe, sizeof(MS_RTP_FRAME));
        stream->lastIframe = NULL;
    }

    stream->lastIframe = ms_rtsp_pool_calloc(1, sizeof(MS_RTP_FRAME));
    if (stream->lastIframe) {
        memcpy(stream->lastIframe, lastIframe, sizeof(MS_RTP_FRAME));
        stream->lastIframe->data = ms_rtsp_pool_calloc(1, lastIframe->nDataSize);
        stream->lastIframe->httpHeader = ms_rtsp_pool_calloc(1, lastIframe->nHttpHeaderSize);
        memcpy(stream->lastIframe->data, lastIframe->data, lastIframe->nDataSize);
        memcpy(stream->lastIframe->httpHeader, lastIframe->httpHeader, lastIframe->nHttpHeaderSize);
    }
}

void ms_removelist_free(RTSP_SERVER_STREAM *node)
{
    if (!node) {
        return ;
    }

    rtsp_server_debug(RTSP_INFO, "[david debug] chnid:%d name:%s state:%d ctxcnt:%d", node->chnid, node->name, node->state,
                         node->ctxCnts);
    int i = 0;
    ms_com_mutex_lock(&node->hdlMutex);
    for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
        if (!node->ctx[i]) {
            continue;
        }
        ms_com_mutex_lock(&node->ctx[i]->ctxMutex);
        ms_stop_rtp_context(node, node->ctx[i]);
        ms_com_mutex_unlock(&node->ctx[i]->ctxMutex);
        ms_com_mutex_uninit(&node->ctx[i]->ctxMutex);
        ms_rtsp_pool_free(node->ctx[i], sizeof(MS_RTP_CONTEXT));
        node->ctx[i] = NULL;
    }
    destroy_multicast_ctx(node);
    del_stream_iframe(node->lastIframe);
    ms_com_mutex_unlock(&node->hdlMutex);
    ms_com_mutex_uninit(&node->hdlMutex);
    ms_rtsp_pool_free(node, sizeof(RTSP_SERVER_STREAM));

    return ;
}

void ms_stream_remove_clear()
{
    while (1) {
        RTSP_SERVER_STREAM *node = ms_removelist_popfront();
        if (!node) {
            break;
        }
        ms_removelist_free(node);
    }

    return ;
}
static inline void ms_ctx_datalist_insert_frame(MS_RTP_CONTEXT *ctx, MS_StreamCtxConnType type, MS_RTP_FRAME *rtpframe)
{
    int size;
    int curPacketSize = 0;
    int nWebSockSize = 0;
    char pWebSockData[10] = {0};
    
    if (!ctx || !rtpframe) {
       rtsp_server_debug(RTSP_WARN, "ctx datalist insert failed. %p %p", ctx, rtpframe);
       return ;
    }

    if (type == MS_StreamCtxConnType_HTTP) {
        size = rtpframe->nHttpHeaderSize + rtpframe->nDataSize;
    } else if (type == MS_StreamCtxConnType_WEBSOCK) {
        ms_get_websocket_header_opcode(pWebSockData, &nWebSockSize, rtpframe->nDataSize + rtpframe->nHttpHeaderSize, 0);
        size = nWebSockSize + rtpframe->nHttpHeaderSize + rtpframe->nDataSize;
    } else {
        return;
    }

    if (ctx->curPacket) {
        curPacketSize = ctx->curPacket->size;
    }
    
    if (ctx->datalistSize + curPacketSize + size> ctx->maxDataSize) {
        ctx->iframeKey = 0;
        rtsp_server_debug(RTSP_WARN, "frame Limit. max:%d cur:%d %d ip:%s url:%s", ctx->maxDataSize, 
            ctx->datalistSize + curPacketSize, ctx->datalistCnt, ctx->ip, ctx->name);
#if defined (_HI3536C_)
        rtsp_server_debug(RTSP_WARN, "3536C clear ctx list.");
        ms_ctx_datalist_clear(ctx);
#endif
        return ;
    }
    MS_AIO_DATANODE *node = NULL;
    node = (MS_AIO_DATANODE *)ms_rtsp_pool_calloc(1, sizeof(MS_AIO_DATANODE));
    if (!node) {
        rtsp_server_debug(RTSP_WARN, "ctx datalist insert node calloc failed.");
        return ;
    }
    
    node->pCtx = ctx;
    node->size = size;
    node->offset = 0;
    node->data = (char*)ms_rtsp_pool_calloc(1, node->size);
    if (!node->data) {
        rtsp_server_debug(RTSP_WARN, "ctx datalist insert node data %d calloc failed.", node->size);
        ms_rtsp_pool_free(node, sizeof(MS_AIO_DATANODE));
        return ;
    }

    if (nWebSockSize) {
        memcpy(node->data, pWebSockData, nWebSockSize);
    }

    memcpy(node->data + nWebSockSize, rtpframe->httpHeader, rtpframe->nHttpHeaderSize);
    memcpy(node->data + nWebSockSize + rtpframe->nHttpHeaderSize, rtpframe->data, rtpframe->nDataSize);
    ctx->datalistSize += size;
    ctx->datalistCnt++;
    //ms_com_mutex_lock(&ctx->ctxMutex);
    MS_AST_LIST_INSERT_TAIL(&ctx->ctxdatalist, node, list);
    //ms_com_mutex_unlock(&ctx->ctxMutex);
    return ;
}

void ms_ctx_datalist_insert_data(MS_RTP_CONTEXT *ctx, void *data, int size)
{
    int curPacketSize = 0;
    
    if (!ctx || !data) {
       rtsp_server_debug(RTSP_DEBUG, "ctx datalist insert failed. ctx:%p data:%p", ctx, data);
       return ;
    }

    if (ctx->curPacket) {
        curPacketSize = ctx->curPacket->size;
    }
    
    if (ctx->datalistSize + curPacketSize + size > ctx->maxDataSize) {
        ctx->iframeKey = 0;
        rtsp_server_debug(RTSP_WARN, "frame Limit. max:%d cur:%d %d ip:%s url:%s", ctx->maxDataSize, 
            ctx->datalistSize + curPacketSize, ctx->datalistCnt, ctx->ip, ctx->name);
#if defined (_HI3536C_)
        rtsp_server_debug(RTSP_WARN, "3536C clear ctx list.");
        ms_ctx_datalist_clear(ctx);
#endif
       return ;
    }
    MS_AIO_DATANODE *node = NULL;
    node = (MS_AIO_DATANODE *)ms_rtsp_pool_calloc(1, sizeof(MS_AIO_DATANODE));
    if (!node) {
        rtsp_server_debug(RTSP_WARN, "ctx datalist insert node calloc failed.");
        return ;
    }
    
    node->pCtx = ctx;
    node->size = size;
    node->offset = 0;
    node->data = (char*)ms_rtsp_pool_calloc(1, node->size);
    if (!node->data) {
        rtsp_server_debug(RTSP_WARN, "ctx datalist insert node data %d calloc failed.", node->size);
        ms_rtsp_pool_free(node, sizeof(MS_AIO_DATANODE));
        return ;
    }
    
    memcpy(node->data, data, node->size);
    ctx->datalistSize += size;
    ctx->datalistCnt++;
    //ms_com_mutex_lock(&ctx->ctxMutex);
    MS_AST_LIST_INSERT_TAIL(&ctx->ctxdatalist, node, list);
    //ms_com_mutex_unlock(&ctx->ctxMutex);
    return ;
}

MS_AIO_DATANODE *ms_ctx_datalist_popfront(MS_RTP_CONTEXT *ctx, int lock)
{
    MS_AIO_DATANODE *node = NULL;
    if (!ctx) {
       rtsp_server_debug(RTSP_INFO, "ctx datalist insert failed.");
       return node;
    }

    if (lock) {
        ms_com_mutex_lock(&ctx->ctxMutex);
    }
    if (ctx->status != MS_CTX_OK) {
        if (lock) {
            ms_com_mutex_unlock(&ctx->ctxMutex);
        }
        return node;
    }
    if (MS_AST_LIST_EMPTY(&ctx->ctxdatalist)) {
        if (lock) {
            ms_com_mutex_unlock(&ctx->ctxMutex);
        }
        return node;
    }
    node = MS_AST_LIST_FIRST(&ctx->ctxdatalist);
    if (node) {
        MS_AST_LIST_REMOVE(&ctx->ctxdatalist, node, list);
    }
    ctx->datalistSize -= node->size;
    ctx->datalistCnt--;
    if (lock) {
        ms_com_mutex_unlock(&ctx->ctxMutex);
    }

    return node;
}

void ms_ctx_datalist_free(MS_AIO_DATANODE *node)
{
    if (!node) {
        return ;
    }
    
    if (node->data) {
        ms_rtsp_pool_free(node->data, node->size);
    }
    ms_rtsp_pool_free(node, sizeof(MS_AIO_DATANODE));
    
    return ;
}

static void ms_ctx_datalist_clear(MS_RTP_CONTEXT *ctx)
{
    //ms_com_mutex_lock(&ctx->ctxMutex);
    while (1) {
        MS_AIO_DATANODE *node = NULL;
        if (MS_AST_LIST_EMPTY(&ctx->ctxdatalist)) {
            break;
        }
        
        node = MS_AST_LIST_FIRST(&ctx->ctxdatalist);
        if (node) {
            MS_AST_LIST_REMOVE(&ctx->ctxdatalist, node, list);
            ctx->datalistSize -= node->size;
            ctx->datalistCnt--;
            ms_ctx_datalist_free(node);
        }
    }
    //ms_com_mutex_unlock(&ctx->ctxMutex);

    return ;
}

/*0=failed. 1=success*/
int ms_aio_readylist_insert(MS_RTP_CONTEXT *ctx, rtsp_server_t *rtsp, MS_AIO_READY_ACTION action)
{
    if (!ctx) {
       rtsp_server_debug(RTSP_WARN, "[david debug] ms aio readylist insert failed.");
       return 0;
    }

    MS_AIO_READY_SEND *node = NULL;
    node = (MS_AIO_READY_SEND *)ms_rtsp_pool_calloc(1, sizeof(MS_AIO_READY_SEND));
    if (!node) {
        rtsp_server_debug(RTSP_WARN, "[david debug] ms aio readylist insert node calloc failed.");
        return 0;
    }
    node->ctx = (void *)ctx;
    node->rtsp = (void *)rtsp;
    node->action = action;
    ms_com_mutex_lock(&g_rtsp_conf.aioready_mutex);
    MS_AST_LIST_INSERT_TAIL(&aioreadylist, node, list);
    ms_com_mutex_unlock(&g_rtsp_conf.aioready_mutex);
    
    return 1;
}

MS_AIO_READY_SEND *ms_aio_readylist_popfront()
{
    MS_AIO_READY_SEND *node = NULL;
    ms_com_mutex_lock(&g_rtsp_conf.aioready_mutex);
    if (MS_AST_LIST_EMPTY(&aioreadylist)) {
        ms_com_mutex_unlock(&g_rtsp_conf.aioready_mutex);
        return node;
    }
    node = MS_AST_LIST_FIRST(&aioreadylist);
    if (node) {
        MS_AST_LIST_REMOVE(&aioreadylist, node, list);
    }
    ms_com_mutex_unlock(&g_rtsp_conf.aioready_mutex);

    return node;
}

void ms_aio_readylist_free(MS_AIO_READY_SEND *node)
{
    if (!node) {
        return ;
    }
    ms_rtsp_pool_free(node, sizeof(MS_AIO_READY_SEND));
    
    return ;
}

void ms_aio_readylist_clear()
{
    while (1) {
        MS_AIO_READY_SEND *node = ms_aio_readylist_popfront();
        if (!node) {
            break;
        }
        ms_aio_readylist_free(node);
    }

    return ;
}

void ms_aio_readylist_clear_ex(MS_RTP_CONTEXT *ctx)
{
    MS_AIO_READY_SEND *node = NULL;
    
    ms_com_mutex_lock(&g_rtsp_conf.aioready_mutex);
    MS_AST_LIST_TRAVERSE(&aioreadylist, node, list) {
        if (!node) {
            break;
        }

        if (node->ctx == ctx) {
            rtsp_server_debug(RTSP_WARN, "del readylist. url:%s ip:%s", ctx->name, ctx->ip);
            node->ctx = NULL;
        }
    }
    ms_com_mutex_unlock(&g_rtsp_conf.aioready_mutex);

    return;
}

static int ms_get_ctx_status_string(int status, char *dst, int size)
{
    if (status == MS_CTX_STOP) {
        snprintf(dst, size, "%s", "STOP");
    } else if (status == MS_CTX_INIT) {
        snprintf(dst, size, "%s", "INIT");
    } else if (status == MS_CTX_OK) {
        snprintf(dst, size, "%s", "OK");
    } else if (status == MS_CTX_REMOVE) {
        snprintf(dst, size, "%s", "REMOVE");
    } else {
        snprintf(dst, size, "%d", status);
    }

    return 0;
}

static int ms_get_vcodec_string(int codec, char *dst, int size)
{
    if (codec == MS_StreamCodecType_H264) {
        snprintf(dst, size, "%s", "H264");
    } else if (codec == MS_StreamCodecType_H265) {
        snprintf(dst, size, "%s", "H265");
    } else {
        rtsp_server_debug(RTSP_INFO, "[david debug] vcodec is error.");
    }

    return 0;
}

static int ms_get_ctx_conn_string(MS_StreamCtxConnType conn, char *dst, int size)
{
    if (conn == MS_StreamCtxConnType_RTSP) {
        snprintf(dst, size, "%s", "rtsp");
    } else if (conn == MS_StreamCtxConnType_HTTP) {
        snprintf(dst, size, "%s", "http");
    } else if (conn == MS_StreamCtxConnType_WEBSOCK) {
        snprintf(dst, size, "%s", "websock");
    } else if (conn == MS_StreamCtxConnType_ROHTTP) {
        snprintf(dst, size, "%s", "rohttp");
    } else {
        snprintf(dst, size, "%d", conn);
    }

    return 0;
}

static int ms_get_ctx_transport_string(int type, char *dst, int size)
{
    if (type == RTSP_TRANSPORT_RTP_UDP) {
        snprintf(dst, size, "%s", "udp");
    } else if (type == RTSP_TRANSPORT_RTP_TCP) {
        snprintf(dst, size, "%s", "tcp");
    } else if (type == RTSP_TRANSPORT_RAW) {
        snprintf(dst, size, "%s", "raw");
    } else {
        snprintf(dst, size, "%d", type);
    }

    return 0;
}

static int ms_get_acodec_string(int codec, char *dst, int size)
{
    int aType = RTP_PAYLOAD_PCMU;
    switch (codec) {
        case MS_StreamCodecType_G711_ULAW:
            aType = RTP_PAYLOAD_PCMU;
            snprintf(dst, size, "%s", "PCMU");
            break;
        case MS_StreamCodecType_G711_ALAW:
            aType = RTP_PAYLOAD_PCMA;
            snprintf(dst, size, "%s", "PCMA");
            break;
        case MS_StreamCodecType_AAC:
            aType = RTP_PAYLOAD_H264;
            snprintf(dst, size, "%s", "MPEG4-GENERIC");
            break;
        case MS_StreamCodecType_G722:
            aType = RTP_PAYLOAD_G722;
            snprintf(dst, size, "%s", "G722");
            break;
        case MS_StreamCodecType_G726:
            aType = RTP_PAYLOAD_MP4A;
            snprintf(dst, size, "%s", "AAL2-G726-16");
            break;
        default:
            aType = RTP_PAYLOAD_PCMU;
            snprintf(dst, size, "%s", "");
            break;
    }

    return aType;
}

void ms_streamlist_show()
{
    int nodeCnt = 0;
    RTSP_SERVER_STREAM *node  = NULL;
    char tmpVideo[32];
    char tmpAudio[32];

    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, node, list) {
        if (!node) {
            break;
        }
        ms_com_mutex_lock(&node->hdlMutex);
        nodeCnt++;
        if (!node->ctxCnts) {
            ms_com_mutex_unlock(&node->hdlMutex);
            continue;
        }
        tmpVideo[0] = '\0';
        tmpAudio[0] = '\0';
        ms_get_vcodec_string(node->vCodeType, tmpVideo, sizeof(tmpVideo));
        ms_get_acodec_string(node->aCodeType, tmpAudio, sizeof(tmpAudio));
        rtsp_server_debug(RTSP_DEBUG, "ch:%d %s cnt:%d video:%s %d*%d bandwidth:%d audio:%s %d state:%d nbs:%d ex:%d",
                    node->chnid, node->name, node->ctxCnts, tmpVideo, node->width, node->height,
                    node->bandWidth, tmpAudio, node->samplerate, node->state, node->nb_streams, node->exSize);
        //nSize = ms_sdp_video_describe(node->vCodeType, sdp, sizeof(sdp), 0, 96, 90000, node->extraData, node->exSize);
        //rtsp_server_debug(RTSP_INFO, "[david debug] sdp:%s nSize:%d", sdp, nSize);
        ms_com_mutex_unlock(&node->hdlMutex);
    }
    rtsp_server_debug(RTSP_DEBUG, "bandwidth  max:%d current:%d", g_rtsp_conf.max_rtsp_bandwidth, g_rtsp_conf.cur_rtsp_bandwidth);
    rtsp_server_debug(RTSP_DEBUG, "ctx count  max:%d current:%d", g_rtsp_conf.max_rtsp_ctxcnt, g_rtsp_conf.cur_rtsp_ctxcnt);
    rtsp_server_debug(RTSP_DEBUG, "stream cnt:%d", nodeCnt);
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return ;
}

void ms_stream_channel_show(char *name)
{
    int i = 0, ctxSize = 0;
    int all = 0;
    int streamCnt = 0;
    int ctxCnt = 0;
    int okCnt = 0;
    int totalCtx = 0;
    int index = 0;
    char tmpVideo[32];
    char tmpAudio[32];
    char tmpStatus[32];
    char tmpConn[32];
    char tmpTrans[32];
    int allListSize = 0;
    int oneListSize = 0;
    int allAioSize = 0;
    int oneAioSize = 0;
    RTSP_SERVER_STREAM *node  = NULL;

    if (strstr(name, "all")) {
        all = 1;
    }
    rtsp_server_debug(RTSP_DEBUG, "show rtsps info %s start.", name);
    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, node, list) {
        if (!node) {
            break;
        }
        ms_com_mutex_lock(&node->hdlMutex);
        if (!all && strcmp(node->name, name)) {
            ms_com_mutex_unlock(&node->hdlMutex);
            continue;
        }
        oneListSize = 0;
        oneAioSize = 0;
        okCnt += node->ctxCnts;
        streamCnt++;
        ctxSize = 0;
        ctxCnt = 0;
        tmpVideo[0] = '\0';
        tmpAudio[0] = '\0';
        ms_get_vcodec_string(node->vCodeType, tmpVideo, sizeof(tmpVideo));
        ms_get_acodec_string(node->aCodeType, tmpAudio, sizeof(tmpAudio));
        rtsp_server_debug(RTSP_DEBUG, "No.:%d ch:%d %s video:%s %d*%d bandwidth:%d audio:%s %d nbs:%d state:%d",
            index, node->chnid, node->name, tmpVideo, node->width, node->height, node->bandWidth,
            tmpAudio, node->samplerate, node->nb_streams, node->state);
        for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
            if (!node->ctx[i]) {
                continue;
            }
            ms_com_mutex_lock(&node->ctx[i]->ctxMutex);
            ms_get_ctx_status_string(node->ctx[i]->status, tmpStatus, sizeof(tmpStatus));
            ms_get_ctx_conn_string(node->ctx[i]->connType, tmpConn, sizeof(tmpConn));
            ms_get_ctx_transport_string(node->ctx[i]->transType, tmpTrans, sizeof(tmpTrans));
            rtsp_server_debug(RTSP_DEBUG, "-- i:%d %s %s %s %s aio:%d listSize:%d listCnt:%d",
                i, tmpStatus, tmpConn, tmpTrans, node->ctx[i]->ip,
                node->ctx[i]->aioBuffSize, node->ctx[i]->datalistSize, node->ctx[i]->datalistCnt);
            ctxSize += node->ctx[i]->aioBuffSize;
            ctxSize += node->ctx[i]->datalistSize;
            oneAioSize += node->ctx[i]->aioBuffSize;
            oneListSize += node->ctx[i]->datalistSize;
            
            ctxCnt++;
            totalCtx++;
            ms_com_mutex_unlock(&node->ctx[i]->ctxMutex);
        }
        rtsp_server_debug(RTSP_DEBUG, "## %s allCtx:%d okCtx:%d otherCtx:%d allSize:%u list:%u aio:%u\n", node->name, 
            ctxCnt, node->ctxCnts, ctxCnt - node->ctxCnts, oneListSize + oneAioSize, oneListSize, oneAioSize);
        ms_com_mutex_unlock(&node->hdlMutex);
        allListSize += oneListSize;
        allAioSize += oneAioSize;
        index++;
        if (!all) {
            break;
        }
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);
    rtsp_server_debug(RTSP_DEBUG, "show rtsps info %s end. streamCnt:%d allCtx:%d okCtx:%d otherCtx:%d allSize:%u list:%u aio:%u", name, 
        streamCnt, totalCtx, okCnt, totalCtx - okCnt, allListSize+allAioSize, allListSize, allAioSize);
    return ;
}

/************************/
///////////////////////funtction ////////////////////////////
/*translate for msdefs.h*/
MS_StreamCodecType ms_translate_forward(int strm_type, int codeType)
{
    if (strm_type != 1 && strm_type != 2) {
        return 0;
    }

    MS_StreamCodecType val = 0;
    if (strm_type == 1) { /*Video Codec*/
        switch (codeType) {
            case 0:
                val = MS_StreamCodecType_H264;
                break;
            case 1:
                val = MS_StreamCodecType_MPEG4;
                break;
            case 2:
                val = MS_StreamCodecType_MJPEG;
                break;
            case 3:
                val = MS_StreamCodecType_H265;
                break;
            default:
                break;
        }
    } else {
        switch (codeType) { /*Audio Codec*/
            case 0:
                val = MS_StreamCodecType_G711_ULAW;
                break;
            case 1:
                val = MS_StreamCodecType_AAC;
                break;
            case 2:
                val = MS_StreamCodecType_G711_ULAW;
                break;
            case 3:
                val = MS_StreamCodecType_G711_ALAW;
                break;
            case 4:
                val = MS_StreamCodecType_G722;
                break;
            case 5:
                val = MS_StreamCodecType_G726;
                break;
            default:
                break;
        }
    }

    rtsp_server_debug(RTSP_INFO, "strm_type:%d codeType:%d val:%d", strm_type, codeType, val);
    return val;
}

MS_StreamStrmType ms_translate_strm_type(int strm_type)
{
    if (strm_type == 1) {
        return MS_StreamStrmType_VIDEO;
    } else if (strm_type == 2) {
        return MS_StreamStrmType_AUDIO;
    } else {
        return MS_StreamStrmType_METAD;
    }
}

static int get_ms_rtsp_uri_multicast(const char *uri)
{
    int ret = 0;

    if (strstr(uri, MS_RTSP_URL_KEY_MULTICAST)) {
        ret = 1;
    }

    rtsp_server_debug(RTSP_INFO, "rtsp uri mcast %d", ret);
    return ret;
}

static MS_RTSP_FRAME_VERSION_E get_ms_rtsp_uri_version(const char *uri)
{
    int i;
    char sValue[32] = {0};
    int len;
    char *s, *v;
    MS_RTSP_FRAME_VERSION_E version = MS_RTSP_FRAME_VERSION_NONE;

    s = strstr(uri, MS_RTSP_URL_KEY_VERSION);
    if (s) {
        v = s + strlen(MS_RTSP_URL_KEY_VERSION);
        len = sizeof(sValue);
        for (i = 0; i < len; i++) {
            if (v[i] == '&' || v[i] == 0) {
                sValue[i] = 0;
                version = atoi(sValue);
                break;
            }
            sValue[i] = v[i];
        }
    }

    rtsp_server_debug(RTSP_INFO, "rtsp uri version %d", version);
    return version;
}

static int get_ms_rtsp_multicast_rtp_port(const char *uri)
{
    int value;
    int chnid;
    int isSub;
    int port;
    
    value = atoi(uri + 3);
    chnid = value % 100;
    isSub = value >= 400 ? 1 : 0;
    port = MS_RTSP_MULTICAST_PORT_BEGIN + chnid * 8 + isSub * 2;

    rtsp_server_debug(RTSP_INFO, "name:%s mcast rtp port:%d-%d", uri, port, port+1);
    return port;
}

static int ms_rtsp_uri_parse(const char *uri, char *path, int nLen)
{
    int i = 0, ix = 0;
    char path1[256];
    struct uri_t *r = uri_parse(uri, strlen(uri));
    if (!r) {
        return -1;
    }
    url_decode(r->path, strlen(r->path), path1, sizeof(path1));
    for (i = 0, ix = 0; i < 256 && i < nLen && path1[i] != '\0' && ix < nLen - 1; i++) {
        if (i == 0 && path1[0] == '/') {
            continue;
        }
        if (path1[i] == '&') {
            break;
        }
        path[ix] = path1[i];
        ix++;
    }
    path[ix] = '\0';
    uri_free(r);

    return 0;
}

static int ms_http_uri_parse(const char *uri, char *path, int nLen)
{
    char path1[256];
    char *ptr = NULL;
    int i = 0;
    struct uri_t *r = uri_parse(uri, strlen(uri));
    if (!r) {
        return -1;
    }
    url_decode(r->path, strlen(r->path), path1, sizeof(path1));
    ptr = strstr(path1, ms_http_url_header);
    if (ptr) {
        snprintf(path, nLen, "%s", ptr + strlen(ms_http_url_header));
    }

    ptr = strstr(path1, ms_http_rtsp_url_header);
    if (ptr) {
        snprintf(path, nLen, "%s", ptr + strlen(ms_http_rtsp_url_header));
    }

    ptr = strstr(path1, ms_http_over_rtsp_url_header);
    if (ptr) {
        snprintf(path, nLen, "%s", ptr + strlen(ms_http_over_rtsp_url_header));
    }

    do {
        if (path[i] == '\0') {
            break;
        }
        if (path[i] == '&') {
            path[i] = '\0';
            break;
        }
        i++;
    } while (1);

    uri_free(r);

    return 0;
}

static void ms_get_session_md5str(char *dest, int nSize)
{
    int i = 0;
    unsigned char buf[16] = {0};
    struct {
        struct timeval ts;
        unsigned int counter;
    } seeddata;
    static unsigned sCnt = 0;

    gettimeofday(&seeddata.ts, NULL);
    seeddata.counter = ++sCnt;

    MD5((const unsigned char *)&seeddata, sizeof(seeddata), buf);
#if 1
    snprintf(dest, nSize, "%02x%02x%02x%02x", buf[0], buf[1], buf[2], buf[3]);
    for (i = 0; i < 8; i++) {
        if (dest[i] >= 'a' && dest[i] <= 'z') {
            dest[i] = dest[i] - 32;
        }
    }
#else
    snprintf(dest, nSize, "%02x%02x%02x%02x%02x%02x%02x%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6],
             buf[7]);
    for (i = 0; i < 6; i++) {
        if (dest[i] >= 'a' && dest[i] <= 'z') {
            dest[i] = dest[i] - 32;
        }
    }
#endif

    return ;
}

static void ms_get_random_md5str(char *dest, int dstsize)
{
    unsigned char buf[16] = {0};
    struct {
        struct timeval ts;
        unsigned int counter;
    } seeddata;
    static unsigned counter = 0;

    gettimeofday(&seeddata.ts, NULL);
    seeddata.counter = ++counter;

    MD5((const unsigned char *)&seeddata, sizeof(seeddata), buf);
    snprintf(dest, dstsize, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8],
             buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);

    return ;
}

static void *ms_rtp_alloc(void *param, int bytes)
{
    MS_RTP_SOURCE *self = (MS_RTP_SOURCE *)param;
    assert(bytes <= sizeof(self->m_packet));
    return self->m_packet;
}

static void ms_rtsp_free(void *param, void *packet)
{
    MS_RTP_SOURCE *self = (MS_RTP_SOURCE *)param;
    assert(self->m_packet == packet);
}

static void ms_rtp_packet(void *param, const void *packet, int bytes, uint32_t timestamp, int flags)
{
    MS_RTP_SOURCE *self = (MS_RTP_SOURCE *)param;
    assert(self->m_packet == packet);
    
    int pDataLen = 0;
    char pData[2048] = {0};
    MS_RTP_CONTEXT *ctx = NULL;
#if 0
    int i = 0, len = 0;
    static int flag = 0;
    char pdata[512] = {0};
    if (!flag) {
        flag = 1;
        len = (bytes > 512) ? 512 : bytes;
        memcpy(pdata, packet, len);
        printf("*************start************\n");
        for (i = 0; i < len; i++) {
            if (i % 63 == 0 && i != 0) {
                printf("%x\n", pdata[i]);
            } else {
                printf("%x ", pdata[i]);
            }
        }
        printf("\n");
        printf("*************end************\n");
    }
#endif
    if (self->m_transtype == RTSP_TRANSPORT_RTP_TCP) {
#if 0
        ms_rtp_tcp_transport_send(self->m_rtsp, self->m_interleaved[0], packet, bytes);
#else
        ctx = self->owner;
        if (!ctx) {
            return;
        }

        //For Tcp InterLeaved Frame
        pData[0] = '$';
        pData[1] = self->m_interleaved[0];
        pData[2] = (bytes >> 8) & 0xFF;
        pData[3] = bytes & 0xff;
        
        if (bytes > (sizeof(pData)  - 4)) {
            rtsp_server_debug(RTSP_WARN, "data length error! bytes:%d", bytes);
            return;
        }
        memcpy(pData + 4, packet, bytes);
        pDataLen = 4 + bytes;
        ms_ctx_datalist_insert_data(ctx, pData, pDataLen);
        if (!ctx->aioSend) {
            ctx->aioSend = ms_aio_readylist_insert(ctx, self->m_rtsp, MS_AIO_ADD_TASK);
        }
#endif
    } else {
        ms_rtp_udp_transport_send(self->m_transport, 0, packet, bytes);
    }

    rtp_onsend(self->m_rtp, packet, bytes);
    return ;
}

static void ms_rtp_packet_ptr(void *param, void **rtp_packet)
{
    // @David no need to do
    return ;
}

static int ms_read_random(uint32_t *dst, const char *file)
{
    int fd = open(file, O_RDONLY);
    int err = -1;
    if (fd == -1) {
        return -1;
    }
    err = read(fd, dst, sizeof(*dst));
    close(fd);
    return err;
}

uint32_t ms_rtp_ssrc(void)
{
    uint32_t seed;
    if (ms_read_random(&seed, "/dev/urandom") == sizeof(seed)) {
        return seed;
    }
    if (ms_read_random(&seed, "/dev/random") == sizeof(seed)) {
        return seed;
    }
    return (uint32_t)rand();
}

static int ms_get_rtp_sdp_aac_config(int samplerate, char *pStr, int size)
{
    int i = 0;
    int profile = 1;
    int index = 11;
    int channels = 1;
    const unsigned int aac_samplerate[16] = {
        97000, 88200, 64000, 48000,
        44100, 32000, 24000, 22050,
        16000, 12000, 11025, 8000,
        7350, 0, 0, 0
    };

    for (i = 0; i < 16; i++) {
        if (aac_samplerate[i] == samplerate) {
            index = i;
            break;
        }
    }
    
    snprintf(pStr, size, "%02x%02x", (unsigned char)((profile + 1) << 3) | (index >> 1),
             (unsigned char)((index << 7) | (channels << 3)));
    
    return 0;
}

static void ms_rtcp_event_cb(void *param, const struct rtcp_msg_t *msg)
{
    MS_RTP_SOURCE *self = (MS_RTP_SOURCE *)param;
    if (self) {
        rtsp_server_debug(RTSP_INFO, "[david debug] self:%p msg:%p", self, msg);
    }

    return ;
}

static int ms_rtcp_packet_send(MS_RTP_SOURCE *param)
{
    if (!param) {
        rtsp_server_debug(RTSP_INFO, "ms_rtcp_packet_send param is Err.");
        return -1;
    }

    // make sure have sent RTP packet
    int pDataLen = 0;
    size_t n = 0;
    char rtcp[1024] = {0};
    char pData[2048] = {0};
    MS_RTP_CONTEXT *ctx = NULL;
    time64_t clock = time64_now();
    int interval = rtp_rtcp_interval(param->m_rtp);

    if (0 == param->m_rtcp_clock || param->m_rtcp_clock + interval < clock) {
        n = rtp_rtcp_report(param->m_rtp, rtcp, sizeof(rtcp));

        // send RTCP packet
        if (param->m_transtype == RTSP_TRANSPORT_RTP_TCP) {
#if 0
            ms_rtp_tcp_transport_send(param->m_rtsp, param->m_interleaved[1], rtcp, n);
#else
            ctx = param->owner;
            if (!ctx) {
                return -1;
            }

            //For Tcp InterLeaved Frame
            pData[0] = '$';
            pData[1] = param->m_interleaved[1];
            pData[2] = (n >> 8) & 0xFF;
            pData[3] = n & 0xff;
            if ((n > (sizeof(pData)  - 4)) || n > sizeof(rtcp)) {
                rtsp_server_debug(RTSP_WARN, "data length error! bytes:%d", n);
                return 0;
            }
            memcpy(pData + 4, rtcp, n);
            pDataLen = 4 + n;
            ms_ctx_datalist_insert_data(ctx, pData, pDataLen);
            if (!ctx->aioSend) {
                ctx->aioSend = ms_aio_readylist_insert(ctx, param->m_rtsp, MS_AIO_ADD_TASK);
            }
#endif
        } else {
            ms_rtp_udp_transport_send(param->m_transport, 1, rtcp, n);
        }

        param->m_rtcp_clock = clock;
    }

    return 0;
}

static void ms_create_rtp_encoder(MS_RTP_CONTEXT *handle)
{
    if (!handle) {
        return ;
    }
    if (handle->status != MS_CTX_INIT) {
        rtsp_server_debug(RTSP_WARN, "ctx already play. url:%s ip:%s", handle->name, handle->ip);
        return ;
    }

    int audioType = RTP_PAYLOAD_PCMU;
    char pVidCode[16] = {0};
    char pAudCode[16] = {0};
    struct rtp_event_t event;
    struct rtp_event_t audioEvent;
    RTSP_SERVER_STREAM *owner = (RTSP_SERVER_STREAM *)(handle->owner);

    memset(&event, 0, sizeof(struct rtp_event_t));
    memset(&audioEvent, 0, sizeof(struct rtp_event_t));

    handle->status = MS_CTX_INIT;

    //video init
    ms_get_vcodec_string(owner->vCodeType, pVidCode, sizeof(pVidCode));
    handle->video.m_rtcp_clock = 0;
    handle->video.m_ssrc = ms_rtp_ssrc();
    handle->video.m_transtype = handle->transType;
    handle->video.m_encoder = rtp_payload_encode_create(RTP_PAYLOAD_MP4V, pVidCode, (uint16_t)handle->video.m_ssrc,
                                                        handle->video.m_ssrc, &ms_rtpfunc, &handle->video);
    if (!handle->video.m_encoder) {
        rtsp_server_debug(RTSP_WARN, "creat video encoder failed. ip:%s url:%s codec:%d %s", handle->ip, 
            handle->name, owner->vCodeType, pVidCode);
    }
    event.on_rtcp = ms_rtcp_event_cb;
    handle->video.m_rtp = rtp_create(&event, &handle->video, handle->video.m_ssrc, handle->video.m_ssrc,
                                     90000,owner->bandWidth, 1);
    if (!handle->video.m_rtp) {
        rtsp_server_debug(RTSP_WARN, "creat video rtp failed. ip:%s url:%s codec:%d %s", handle->ip, 
            handle->name, owner->vCodeType, pVidCode);
    }
    snprintf(handle->video.m_sname, sizeof(handle->video.m_sname), "video-%d", handle->video.m_ssrc);
    rtp_set_info(handle->video.m_rtp, "RTSPServer", handle->video.m_sname);
    handle->video.m_timestamp = (uint32_t)handle->video.m_ssrc;
    handle->count += 1;

    //audio init
    if (owner->nb_streams & (1 << DEFAULT_STREAM_AUD)) {
        audioType = ms_get_acodec_string(owner->aCodeType, pAudCode, sizeof(pAudCode));
        handle->audio.m_rtcp_clock = 0;
        handle->audio.m_ssrc = ms_rtp_ssrc();
        handle->audio.m_transtype = handle->transType;
        handle->audio.m_encoder = rtp_payload_encode_create(audioType, pAudCode, (uint16_t)handle->audio.m_ssrc,
                                                            handle->audio.m_ssrc, &ms_rtpfunc, &handle->audio);
        if (!handle->audio.m_encoder) {
            rtsp_server_debug(RTSP_WARN, "creat audio encoder failed. ip:%s url:%s codec:%d %s", handle->ip, 
                handle->name, owner->aCodeType, pAudCode);
        }
        audioEvent.on_rtcp = ms_rtcp_event_cb;
        handle->audio.m_rtp = rtp_create(&audioEvent, &handle->audio, handle->audio.m_ssrc,
                                         handle->audio.m_ssrc, owner->samplerate,
                                         owner->samplerate, 1);
        if (!handle->audio.m_rtp) {
            rtsp_server_debug(RTSP_WARN, "creat audio rtp failed. ip:%s url:%s codec:%d %s", handle->ip, 
                handle->name, owner->aCodeType, pAudCode);
        }
        snprintf(handle->audio.m_sname, sizeof(handle->audio.m_sname), "audio-%d", handle->audio.m_ssrc);
        rtp_set_info(handle->audio.m_rtp, "RTSPServer", handle->audio.m_sname);
        handle->audio.m_timestamp = (uint32_t)handle->audio.m_ssrc;
        handle->count += 1;
    }

    //metadate init
    if (owner->nb_streams & (1 << DEFAULT_STREAM_MED)) {
        handle->metadata.m_rtcp_clock = 0;
        handle->metadata.m_ssrc = ms_rtp_ssrc();
        handle->metadata.m_transtype = handle->transType;
        handle->metadata.m_encoder = rtp_payload_encode_create(98, pAudCode, (uint16_t)handle->metadata.m_ssrc,
                                                            handle->metadata.m_ssrc, &ms_rtpfunc, &handle->metadata);
        if (!handle->metadata.m_encoder) {
            rtsp_server_debug(RTSP_WARN, "creat metadata encoder failed. ip:%s url:%s", handle->ip, handle->name);
        }
        audioEvent.on_rtcp = ms_rtcp_event_cb;
        handle->metadata.m_rtp = rtp_create(&audioEvent, &handle->metadata, handle->metadata.m_ssrc,
                                         handle->metadata.m_ssrc, owner->samplerate,
                                         owner->samplerate, 1);
        if (!handle->metadata.m_rtp) {
            rtsp_server_debug(RTSP_WARN, "creat metadata rtp failed. ip:%s url:%s", handle->ip, handle->name);
        }
        snprintf(handle->metadata.m_sname, sizeof(handle->metadata.m_sname), "metadata-%d", handle->metadata.m_ssrc);
        rtp_set_info(handle->metadata.m_rtp, "RTSPServer", handle->metadata.m_sname);
        handle->metadata.m_timestamp = (uint32_t)handle->metadata.m_ssrc;
        handle->count += 1;
    }

    return ;
}

static void ms_destroy_rtp_encoder(MS_RTP_CONTEXT *handle)
{
    if (handle->audio.m_rtp) {
        rtp_destroy(handle->audio.m_rtp);
        handle->audio.m_rtp = NULL;
    }

    if (handle->audio.m_encoder) {
        rtp_payload_encode_destroy(handle->audio.m_encoder);
        handle->audio.m_encoder = NULL;
    }

    if (handle->video.m_rtp) {
        rtp_destroy(handle->video.m_rtp);
        handle->video.m_rtp = NULL;
    }

    if (handle->video.m_encoder) {
        rtp_payload_encode_destroy(handle->video.m_encoder);
        handle->video.m_encoder = NULL;
    }

    if (handle->metadata.m_rtp) {
        rtp_destroy(handle->metadata.m_rtp);
        handle->metadata.m_rtp = NULL;
    }
    if (handle->metadata.m_encoder) {
        rtp_payload_encode_destroy(handle->metadata.m_encoder);
        handle->metadata.m_encoder = NULL;
    }

    return ;
}

static int ms_get_rtp_info(const char *uri, const char *session, char *rtpInfo, int nLen, rtsp_server_t *rtsp)
{
    uint16_t seq;
    uint32_t timestamp = 0;
    int i = 0;
    char streamUri[64] = {0};
    char pRtpInfo[256] = {0};
    RTSP_SERVER_STREAM *stream = NULL;

    if (!uri || !session) {
        return -1;
    }

    if (strstr(uri, MS_RTSP_PLAYBACK_URL)) {
        snprintf(streamUri, sizeof(streamUri), rtsp_server_get_stream_name(rtsp));
    } else {
        ms_rtsp_uri_parse(uri, streamUri, sizeof(streamUri));
    }

    stream = ms_streamlist_find_name(streamUri);
    if (!stream) {
        return -1;
    }

    ms_com_mutex_lock(&stream->hdlMutex);
    for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
        if (!stream->ctx[i]) {
            continue;
        }
        if (!strcmp(stream->ctx[i]->sessionId, session)) {
            if (stream->ctx[i]->video.sessionId[0] != '\0') {
                rtp_payload_encode_getinfo(stream->ctx[i]->video.m_encoder, &seq, &timestamp);
                timestamp = stream->ctx[i]->video.m_timestamp;
                snprintf(pRtpInfo, sizeof(pRtpInfo), "url=%s/streamid=%d;seq=%hu;rtptime=%u", uri, 0, seq, timestamp);
            }
            if (stream->ctx[i]->audio.sessionId[0] != '\0') {
                rtp_payload_encode_getinfo(stream->ctx[i]->audio.m_encoder, &seq, &timestamp);
                timestamp = stream->ctx[i]->audio.m_timestamp;
                snprintf(pRtpInfo + strlen(pRtpInfo), sizeof(pRtpInfo) - strlen(pRtpInfo), ",url=%s/streamid=%d;seq=%hu;rtptime=%u",
                         uri, 1, seq,
                         timestamp);
            }
            snprintf(pRtpInfo + strlen(pRtpInfo), sizeof(pRtpInfo) - strlen(pRtpInfo), ",url=%s/streamid=%d;seq=%hu;rtptime=%u",
                     uri, 2, 0, 0);
            break;
        }
    }
    ms_com_mutex_unlock(&stream->hdlMutex);
    snprintf(rtpInfo, nLen, "%s", pRtpInfo);

    return 0;
}

static int ms_get_rtsp_stream_name(const char *uri, char *dst, int size)
{
    int i = 0;
    char *ptr = NULL;
    if (!uri) {
        rtsp_server_debug(RTSP_INFO, "[david debug] uri is NULL.");
        return -1;
    }

    ptr = strstr(uri, "ch_");
    if (!ptr) {
        ptr = strstr(uri, "pb_");
        if (!ptr) {
            rtsp_server_debug(RTSP_INFO, "[david debug] uri %s is invalid.", uri);
            return -1;
        }
    }

    snprintf(dst, size, "%s", ptr);
    do {
        if (dst[i] == '\0') {
            break;
        }
        if (dst[i] == '/' || dst[i] == '&') {
            dst[i] = '\0';
            break;
        }
        i++;
    } while (1);

    return 0;
}

static MS_RTP_CONTEXT *ms_get_rtp_ctx(const char *uri, const char *session)
{
    int i = 0;
    char streamUri[64] = {0};
    RTSP_SERVER_STREAM *stream = NULL;
    MS_RTP_CONTEXT *ctx = NULL;

    if (!uri || !session) {
        return NULL;
    }

    ms_get_rtsp_stream_name(uri, streamUri, sizeof(streamUri));
    stream = ms_streamlist_find_name(streamUri);
    if (!stream) {
        return NULL;
    }

    ms_com_mutex_lock(&stream->hdlMutex);
    for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
        if (!stream->ctx[i]) {
            continue;
        }
        if (!strcmp(stream->ctx[i]->sessionId, session)) {
            ctx = stream->ctx[i];
            break;
        }
    }
    ms_com_mutex_unlock(&stream->hdlMutex);

    return ctx;
}

static int ms_get_rtp_streamid(const char *uri)
{
    int streamId;
    if (strstr(uri, "streamid=0")) {
        streamId = DEFAULT_STREAM_VID;
    } else if (strstr(uri, "streamid=1")) {
        streamId = DEFAULT_STREAM_AUD;
    } else if (strstr(uri, "streamid=2")) {
        streamId = DEFAULT_STREAM_MED;
    } else {
        streamId = DEFAULT_STREAM_VID;
    }

    return streamId;
}

static MS_StreamCtxConnType ms_get_rtsp_stream_contype(const char *uri)
{
    MS_StreamCtxConnType connType = MS_StreamCtxConnType_RTSP;
    if (strstr(uri, ms_http_url_header)) {
        connType = MS_StreamCtxConnType_HTTP;
    } else if (strstr(uri, ms_http_rtsp_url_header)) {
        connType = MS_StreamCtxConnType_WEBSOCK;
    } else if (strstr(uri, ms_http_over_rtsp_url_header)) {
        connType = MS_StreamCtxConnType_ROHTTP;
    }

    return connType;
}

static MS_RTP_CONTEXT *ms_get_rtp_context_by_rohttp(void *rtsp, MS_RTSP_OVER_HTTP_PACKET *reqs)
{
    if (!reqs || !reqs->uri) {
        return NULL;
    }

    int i, isfound = 0;
    char streamUri[64] = {0};
    MS_RTP_CONTEXT *ctx = NULL;
    RTSP_SERVER_STREAM *tmp  = NULL;

    //streamId = ms_get_rtp_streamid(reqs->uri);
    ms_get_rtsp_stream_name(reqs->uri, streamUri, sizeof(streamUri));

    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, tmp, list) {
        if (!tmp) {
            continue;
        }
        ms_com_mutex_lock(&tmp->hdlMutex);
        if (streamUri[0] != '\0') {
            if (strcmp(tmp->name, streamUri)) {
                ms_com_mutex_unlock(&tmp->hdlMutex);
                continue;
            }
        }
        
        for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
            if (!tmp->ctx[i]) {
                continue;
            }

            if (strstr(tmp->ctx[i]->xSessionCookie, reqs->xSessionCookie)) {
                isfound = 1;
                ctx = tmp->ctx[i];
                break;
            }
        }
        ms_com_mutex_unlock(&tmp->hdlMutex);
        if (isfound) {
            break;
        }
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return ctx;
}

static MS_RTP_CONTEXT *ms_get_rtp_context_by_rtsp(void *rtsp)
{
    if (!rtsp) {
        return NULL;
    }

    int i = 0;
    MS_RTP_CONTEXT *ctx = NULL;
    RTSP_SERVER_STREAM *stream = NULL;
    const char *name;

    name = rtsp_server_get_stream_name(rtsp);
    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, stream, list) {
        if (!stream) {
            break;
        }
        ms_com_mutex_lock(&stream->hdlMutex);
        if (strcmp(stream->name, name)) {
            ms_com_mutex_unlock(&stream->hdlMutex);
            continue;
        }
        
        if (stream->ctxCnts <= 0) {
            ms_com_mutex_unlock(&stream->hdlMutex);
            break;
        }

        for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
            if (!stream->ctx[i]) {
                continue;
            }
            ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
            if (stream->ctx[i]->status != MS_CTX_OK) {
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                continue;
            }
            if (stream->ctx[i]->video.m_rtsp == rtsp
                || stream->ctx[i]->audio.m_rtsp == rtsp
                || stream->ctx[i]->metadata.m_rtsp == rtsp) {
                ctx = stream->ctx[i];
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                break;
            }
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
        }
        ms_com_mutex_unlock(&stream->hdlMutex);
        break;
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return ctx;
}

static inline void ms_init_rtp_context(RTSP_SERVER_STREAM *stream, int index, MS_StreamCtxConnType connType, int transport, rtsp_server_t *rtsp)
{
    stream->ctxCnts++;
    stream->ctx[index]->status = MS_CTX_INIT;
    stream->ctx[index]->owner = (void *)stream;
    stream->ctx[index]->connType = connType;
    stream->ctx[index]->transType = transport;
    stream->ctx[index]->count++;
    stream->ctx[index]->frameVersion = MS_RTSP_FRAME_VERSION_NONE;
    stream->ctx[index]->iframeKey = 0;
    stream->ctx[index]->aioSend = 0;
    stream->ctx[index]->aioBuffSize = 0;
    stream->ctx[index]->lastIframe = 0;
    snprintf(stream->ctx[index]->ip, sizeof(stream->ctx[index]->ip), "%s", rtsp_server_get_client(rtsp, NULL));
    if (stream->maxDataSize) {
        stream->ctx[index]->maxDataSize = stream->maxDataSize;
    } else {
        stream->ctx[index]->maxDataSize = MAX_CTX_DATA_SIZE;
    }
    snprintf(stream->ctx[index]->name, sizeof(stream->ctx[index]->name), "%s", stream->name);
    stream->ctx[index]->bandwidth = stream->bandWidth * 1024;
    ms_change_bandwidth(MS_RTSP_PLUS, stream->ctx[index]->bandwidth);
    ms_get_session_md5str(stream->ctx[index]->sessionId, sizeof(stream->ctx[index]->sessionId));

    //noblock
    rtsp_set_socket_nonblock(rtsp);
}

static inline MS_RTP_SOURCE *ms_create_rtp_context(const char *uri, const char *session, int transport, rtsp_server_t *rtsp)
{
    int i = 0, streamId = -1;
    int bandwidth;
    char streamUri[64] = {0};
    RTSP_SERVER_STREAM *stream = NULL;
    MS_RTP_SOURCE *rtpSource = NULL;
    MS_RTP_CONTEXT *oldCtx = NULL;
    MS_StreamCtxConnType connType = MS_StreamCtxConnType_RTSP;

    if (!uri) {
        rtsp_server_debug(RTSP_WARN, "create rtp ctx failed. uri is null. transport:%d", transport);
        return rtpSource;
    }

    streamId = ms_get_rtp_streamid(uri);
    connType = ms_get_rtsp_stream_contype(uri);
    
    if (strstr(uri, MS_RTSP_PLAYBACK_URL)) {
        snprintf(streamUri, sizeof(streamUri), rtsp_server_get_stream_name(rtsp));
    } else {
        ms_get_rtsp_stream_name(uri, streamUri, sizeof(streamUri));
        rtsp_server_set_stream_name(rtsp, streamUri);
    }

    stream = ms_streamlist_find_name(streamUri);
    if (!stream) {
        rtsp_server_debug(RTSP_WARN, "create rtp ctx failed. uri %s not exist. stramid:%d connType:%d transport:%d",
                          streamUri, streamId, connType, transport);
        return rtpSource;
    }
    if (streamId == DEFAULT_STREAM_VID) {
        //当UDP连接不行，客户端会重新resetup用TCP连接，此时之前UDP的context要被销毁
        oldCtx = ms_get_rtp_context_by_rtsp(rtsp);
        if (oldCtx) {
            rtsp_server_debug(RTSP_WARN, "udp to tcp. uri:%s ctx:%p", uri, oldCtx);
            ms_com_mutex_lock(&oldCtx->ctxMutex);
            ms_stop_rtp_context(stream, oldCtx);
            ms_com_mutex_unlock(&oldCtx->ctxMutex);
        }
        bandwidth = stream->bandWidth * 1024;
        if (ms_bandwidth_limit(MS_RTSP_PLUS, bandwidth)) {
            return rtpSource;
        }
    }
    ms_com_mutex_lock(&stream->hdlMutex);
    if (streamId == DEFAULT_STREAM_VID) {
        for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
            if (stream->ctx[i]) {
                ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
                if (stream->ctx[i]->status == MS_CTX_STOP) {
                    ms_init_rtp_context(stream, i, connType, transport, rtsp);
                    rtpSource = &stream->ctx[i]->video;
                    rtpSource->owner = (void *)stream->ctx[i];
                    snprintf(rtpSource->sessionId, sizeof(rtpSource->sessionId), "%s", stream->ctx[i]->sessionId);
                    rtsp_server_debug(RTSP_INFO, "create ctx. i:%d uri:%s ip:%s ctx:%p sessionId:%s maxData:%d [OLD]",
                        i, streamUri, stream->ctx[i]->ip, stream->ctx[i], rtpSource->sessionId, stream->ctx[i]->maxDataSize);
                    ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                    break;
                }
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            } else {
                stream->ctx[i] = (MS_RTP_CONTEXT *)ms_rtsp_pool_calloc(1, sizeof(MS_RTP_CONTEXT));
                ms_com_mutex_init(&stream->ctx[i]->ctxMutex);
                ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
                ms_init_rtp_context(stream, i, connType, transport, rtsp);
                MS_AST_LIST_HEAD_INIT_NOLOCK(&stream->ctx[i]->ctxdatalist);
                rtpSource = &stream->ctx[i]->video;
                rtpSource->owner = (void *)stream->ctx[i];
                snprintf(rtpSource->sessionId, sizeof(rtpSource->sessionId), "%s", stream->ctx[i]->sessionId);
                rtsp_server_debug(RTSP_INFO, "create ctx. i:%d uri:%s ip:%s ctx:%p sessionId:%s maxData:%d [NEW]",
                    i, streamUri, stream->ctx[i]->ip, stream->ctx[i], rtpSource->sessionId, stream->ctx[i]->maxDataSize);
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                break;
            }
        }
    } else {
        for (i = 0; i < MX_MAX_RTSP_CTX && session; i++) {
            if (stream->ctx[i]) {
                ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
                if (!strcmp(stream->ctx[i]->sessionId, session)) {
                    if (streamId == DEFAULT_STREAM_AUD) {
                        rtpSource = &stream->ctx[i]->audio;
                    } else {
                        rtpSource = &stream->ctx[i]->metadata;
                    }
                    rtpSource->owner = (void *)stream->ctx[i];
                    stream->ctx[i]->count++;
                    snprintf(rtpSource->sessionId, sizeof(rtpSource->sessionId), "%s", stream->ctx[i]->sessionId);
                    ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                    break;
                }
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            }
        }
    }
    ms_com_mutex_unlock(&stream->hdlMutex);
    return rtpSource;
}

static inline void ms_stop_rtp_context(RTSP_SERVER_STREAM *stream, MS_RTP_CONTEXT *ctx)
{
    if (ctx->status == MS_CTX_STOP) {
        return;
    }

    rtsp_server_debug(RTSP_INFO, "[rtsps debug] stop rtp context session:%s", ctx->sessionId);
    if (ctx->status == MS_CTX_OK && stream) {
        stream->ctxCnts--;
    }
    ctx->status = MS_CTX_STOP;
    ms_aio_readylist_clear_ex(ctx);
    ms_destroy_rtp_encoder(ctx);
    if (ctx->video.m_transport) {
        ms_rtp_udp_transport_destroy(ctx->video.m_transport);
        ctx->video.m_transport = NULL;
    }
    if (ctx->audio.m_transport) {
        ms_rtp_udp_transport_destroy(ctx->audio.m_transport);
        ctx->audio.m_transport = NULL;
    }
    if (ctx->metadata.m_transport) {
        ms_rtp_udp_transport_destroy(ctx->metadata.m_transport);
        ctx->metadata.m_transport = NULL;
    }
    ctx->video.m_rtsp = NULL;
    ctx->audio.m_rtsp = NULL;
    ctx->metadata.m_rtsp = NULL;
    ms_change_bandwidth(MS_RTSP_MINUS, ctx->bandwidth);
    ctx->bandwidth = 0;
    ctx->aioBuffSize = 0;
    ctx->name[0] = '\0';
    ctx->ip[0] = '\0';
    ctx->sessionId[0] = '\0';
    if (ctx->curPacket) {
        ms_ctx_datalist_free(ctx->curPacket);
        ctx->curPacket = NULL;
    }
    ms_ctx_datalist_clear(ctx);
}

static int ms_destroy_rtp_context(const char *uri, const char *session)
{
    int i = 0;
    int flag = 0;
    char streamUri[64] = {0};
    RTSP_SERVER_STREAM *stream = NULL;

    if (!uri || !session) {
        return -1;
    }
    if (ms_get_rtsp_stream_name(uri, streamUri, sizeof(streamUri))) {
        return -1;
    }
    
    stream = ms_streamlist_find_name(streamUri);
    if (!stream) {
        rtsp_server_debug(RTSP_WARN, "not find stream by uri %s", streamUri);
        return -1;
    }

    rtsp_server_debug(RTSP_INFO, "[david debug] uri:%s session:%s streamUri:%s", uri, session, streamUri);
    ms_com_mutex_lock(&stream->hdlMutex);
    for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
        if (!stream->ctx[i]) {
            continue;
        }
        ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
        if (stream->ctx[i]->status != MS_CTX_OK) {
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            continue;
        }
        if (!strcmp(stream->ctx[i]->sessionId, session)) {
            ms_stop_rtp_context(stream, stream->ctx[i]);
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            flag = 1;
            break;
        }
        ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
    }
    ms_com_mutex_unlock(&stream->hdlMutex);

    return flag;
}

static void ms_rtsp_onaioready_send(void *param, rtsp_server_t *rtsp, size_t bytes)
{
    /*aio 发送监听线程发送回调接口，不能延迟等待*/
    MS_AIO_DATANODE *datanode = NULL;
    const char *ip;
    const char *url;
    MS_RTP_CONTEXT *ctx;
    int r;

    ctx = ms_get_rtp_context_by_rtsp(rtsp);
    if (!ctx) {
        url = rtsp_server_get_stream_name(rtsp);
        ip = rtsp_server_get_client(rtsp, NULL);
        rtsp_server_debug(RTSP_DEBUG, "ms rtsp on aio ready is failed. url:%s ip:%s", url, ip);
        return ;
    }

    ms_com_mutex_lock(&ctx->ctxMutex);
    if (ctx->status != MS_CTX_OK) {
        rtsp_server_debug(RTSP_WARN, "ctx is not ok. url:%s ip:%s", ctx->name, ctx->ip);
        ms_com_mutex_unlock(&ctx->ctxMutex);
        return ;
    }
    datanode = ctx->curPacket;
    if (datanode) {
        datanode->offset += bytes;
        if (datanode->offset >= datanode->size) {
            ms_ctx_datalist_free(datanode);
            ctx->curPacket = NULL;
        } else {
            r = ms_packet_aio_send(rtsp, datanode->data + datanode->offset, datanode->size - datanode->offset);
            ms_com_mutex_unlock(&ctx->ctxMutex);
            return;
        }
    }
    datanode = ms_ctx_datalist_popfront(ctx, 0);
    if (!datanode) {
        ctx->aioSend = 0;
        //rtsp_server_debug(RTSP_DEBUG, "list is empty");
        ms_com_mutex_unlock(&ctx->ctxMutex);
        return;
    }
     
    ctx->aioBuffSize = datanode->size;
    r = ms_packet_aio_send(rtsp, datanode->data, datanode->size);
    if (r < 0) {
        rtsp_server_debug(RTSP_WARN, "aio send faied. r:%d size:%d", r, datanode->size);
    }
    ctx->curPacket = datanode;
    ms_com_mutex_unlock(&ctx->ctxMutex);
    return ;
}

static void ms_rtsp_onaioready_start(MS_RTP_CONTEXT *ctx, rtsp_server_t *rtsp)
{
    MS_AIO_DATANODE *datanode = NULL;
    
    if (!ctx) {
        rtsp_server_debug(RTSP_WARN, "ms rtsp on aio ready is failed. ctx is null.");
        return ;
    }

    ms_com_mutex_lock(&ctx->ctxMutex);
    if (ctx->status != MS_CTX_OK) {
        rtsp_server_debug(RTSP_WARN, "ctx status is not ok. status:%d url:%s ip:%s", ctx->status, ctx->name, ctx->ip);
        ctx->aioSend = 0;
        ms_com_mutex_unlock(&ctx->ctxMutex);
        return;
    }
    datanode = ms_ctx_datalist_popfront(ctx, 0);
    if (!datanode) {
        rtsp_server_debug(RTSP_WARN, "aio start node is null. url:%s ip:%s",ctx->name, ctx->ip);
        ctx->aioSend = 0;
        ms_com_mutex_unlock(&ctx->ctxMutex);
        return;
    }
    
    ctx->aioBuffSize = datanode->size;
    int r = ms_packet_aio_send(rtsp, datanode->data, datanode->size);
    if (r < 0) {
        rtsp_server_debug(RTSP_WARN, "aio start failed. r:%d size:%d", r, datanode->size);
    }
    ctx->curPacket = datanode;
    ms_com_mutex_unlock(&ctx->ctxMutex);
    
    return ;
}

static inline int ms_rtsp_custom_frame_support(MS_RTSP_FRAME_VERSION_E frameVersion, MS_StreamStrmType stremType)
{
    if (stremType == MS_StreamStrmType_METAD && frameVersion < MS_RTSP_FRAME_VERSION_2) {
        return 0;
    }

    return 1;
}

static inline int ms_rtsp_audio_frame_support(MS_StreamStrmType stremType, int audioPerm)
{
    if (stremType == MS_StreamStrmType_AUDIO && !audioPerm) {
        return 0;
    }

    return 1;
}

int ms_rtsp_handle_senddata(void *hdl, MS_RTP_FRAME *rtpframe)
{
    RTSP_SERVER_STREAM *stream = (RTSP_SERVER_STREAM *)hdl;
    if (!stream) {
        return -1;
    }
    ms_com_mutex_lock(&stream->hdlMutex);
    if (stream->state == 0) {
        ms_com_mutex_unlock(&stream->hdlMutex);
        return -1;
    }
    if (stream->ctxCnts <= 0) {
        ms_com_mutex_unlock(&stream->hdlMutex);
        return -1;
    }
    if (!rtpframe) {
        ms_com_mutex_unlock(&stream->hdlMutex);
        return -1;
    }

    int i, val = 0;
    int mcast = 0;
    http_frame_packet_header_t *header = NULL;
    MS_RTP_FRAME *realFrame;

    if (stream->upTimeUsec[rtpframe->strmType] == 0) {
        stream->upTimeUsec[rtpframe->strmType] = rtpframe->time_lower;
    }
    val = abs(rtpframe->time_lower - stream->upTimeUsec[rtpframe->strmType]);
    stream->upTimeUsec[rtpframe->strmType] = rtpframe->time_lower;

    for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
        if (!stream->ctx[i]) {
            continue;
        }
        
        ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
        if (stream->ctx[i]->status != MS_CTX_OK) {
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            continue;
        }

        if (!ms_rtsp_custom_frame_support(stream->ctx[i]->frameVersion, rtpframe->strmType)) {
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            continue;
        }

        if (!ms_rtsp_audio_frame_support(rtpframe->strmType, stream->ctx[i]->audioPerm)) {
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            continue;
        }
        
        if (stream->ctx[i]->mcast) {
            mcast++;
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            continue;
        }

        realFrame = rtpframe;
        if (!stream->ctx[i]->iframeKey) {
            if (rtpframe->strmType == MS_StreamStrmType_VIDEO && !rtpframe->frameType) {
                stream->ctx[i]->iframeKey = 1;
                if (strstr(stream->name , "pb_")) {
                    header = (http_frame_packet_header_t *)rtpframe->httpHeader;
                    stream->width = header->width;
                    stream->height = header->height;
                }
            } else {
                if (MS_RTSP_IFRAME_BUFFER == 0 || stream->ctx[i]->lastIframe || !stream->lastIframe
                        || stream->ctx[i]->connType == MS_StreamCtxConnType_RTSP) {
                    ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                    continue;
                }
                //http && webscoket 第一次连接上，直接转发最近的I帧
                realFrame = stream->lastIframe;
                stream->ctx[i]->lastIframe = 1;
                //rtsp_server_debug(RTSP_WARN, "### send last iframe type:%d iframe:%d size:%d usec:%llu ###.",
                //    realFrame->strmType, realFrame->frameType, realFrame->nDataSize, realFrame->time_usec);
            }
        }

        if (stream->ctx[i]->connType == MS_StreamCtxConnType_RTSP
            || stream->ctx[i]->connType == MS_StreamCtxConnType_ROHTTP) {
            if (realFrame->strmType == MS_StreamStrmType_VIDEO) {                /*video*/
                stream->ctx[i]->video.m_timestamp += val;
                rtp_payload_encode_input(stream->ctx[i]->video.m_encoder, realFrame->data, realFrame->nDataSize,
                                         stream->ctx[i]->video.m_timestamp);
                ms_rtcp_packet_send(&stream->ctx[i]->video);
            } else if (realFrame->strmType == MS_StreamStrmType_AUDIO) {         /*audio*/
                if (!stream->ctx[i]->audio.m_encoder) {
                    ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                    continue;
                }
                stream->ctx[i]->audio.m_timestamp += val;
                rtp_payload_encode_input(stream->ctx[i]->audio.m_encoder, realFrame->data, realFrame->nDataSize,
                                         stream->ctx[i]->audio.m_timestamp);
                ms_rtcp_packet_send(&stream->ctx[i]->audio);
            } else if (rtpframe->strmType == MS_StreamStrmType_METAD) {
                if (!stream->ctx[i]->metadata.m_encoder) {
                    rtsp_server_debug(RTSP_WARN, "!!!!");
                    ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                    continue;
                }
                stream->ctx[i]->metadata.m_timestamp += val;
                rtp_payload_encode_input(stream->ctx[i]->metadata.m_encoder, rtpframe->data, rtpframe->nDataSize,
                                         stream->ctx[i]->metadata.m_timestamp);
                ms_rtcp_packet_send(&stream->ctx[i]->metadata);
            } else {
                rtsp_server_debug(RTSP_INFO, "play type is Err. %d", rtpframe->strmType);
            }
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
        } else if (stream->ctx[i]->connType == MS_StreamCtxConnType_HTTP) {     /*http*/
            header = (http_frame_packet_header_t *)realFrame->httpHeader;
            snprintf(header->hsession, sizeof(header->hsession), "%s", stream->ctx[i]->sessionId);
            ms_ctx_datalist_insert_frame(stream->ctx[i], MS_StreamCtxConnType_HTTP, realFrame);
            if (!stream->ctx[i]->aioSend) {
                stream->ctx[i]->aioSend = ms_aio_readylist_insert(stream->ctx[i], stream->ctx[i]->video.m_rtsp, MS_AIO_ADD_TASK);
            }
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
        } else if (stream->ctx[i]->connType == MS_StreamCtxConnType_WEBSOCK) {  /*websocket*/
            ms_ctx_datalist_insert_frame(stream->ctx[i], MS_StreamCtxConnType_WEBSOCK, realFrame);
            if (!stream->ctx[i]->aioSend) {
                stream->ctx[i]->aioSend = ms_aio_readylist_insert(stream->ctx[i], stream->ctx[i]->video.m_rtsp, MS_AIO_ADD_TASK);
            }
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
        } else {
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
        }
    }

    if (mcast) {
        if (stream->mcastCtx) {
            ms_com_mutex_lock(&stream->mcastCtx->ctxMutex);
            if (rtpframe->strmType == MS_StreamStrmType_VIDEO) {                /*video*/
                stream->mcastCtx->video.m_timestamp += val;
                rtp_payload_encode_input(stream->mcastCtx->video.m_encoder, rtpframe->data, rtpframe->nDataSize,
                                         stream->mcastCtx->video.m_timestamp);
                ms_rtcp_packet_send(&stream->mcastCtx->video);
            } else if (rtpframe->strmType == MS_StreamStrmType_AUDIO) {         /*audio*/
                if (ms_rtsp_audio_frame_support(rtpframe->strmType, stream->mcastCtx->audioPerm)) {
                    stream->mcastCtx->audio.m_timestamp += val;
                    rtp_payload_encode_input(stream->mcastCtx->audio.m_encoder, rtpframe->data, rtpframe->nDataSize,
                                             stream->mcastCtx->audio.m_timestamp);
                    ms_rtcp_packet_send(&stream->mcastCtx->audio);
                }
            }
            ms_com_mutex_unlock(&stream->mcastCtx->ctxMutex);
        }
    }

    if (MS_RTSP_IFRAME_BUFFER == 1) {
        if (rtpframe->strmType == MS_StreamStrmType_VIDEO && !rtpframe->frameType) {
            update_stream_iframe(stream, rtpframe);
        }
    }
    ms_com_mutex_unlock(&stream->hdlMutex);

    return 0;
}

/******************************************************/
static int ms_rtsp_onoptions(void *ptr, rtsp_server_t *rtsp, const char *uri)
{
    rtsp_server_debug(RTSP_INFO,"[OPTION] uri:%s rtsp:%p", uri, rtsp);
    return rtsp_server_reply_options(rtsp, 200);
}

static int ms_rtsp_chanenl_permissions(RTSP_SERVER_STREAM *stream, struct MsRtspPermissions *perm)
{
    int ret = 1;
    
    if (!perm->userLevel) {
        return 1;
    }

    if (stream->chnid < 0 || stream->chnid >= 64) {
        return 1;
    }

    rtsp_server_debug(RTSP_INFO, "stream name:%s chnid:%d isplayback:%d user:%s level:%d", stream->name, stream->chnid, 
           stream->isPlayback, perm->username, perm->userLevel);
    if (stream->isPlayback) {
        if (perm->playbackMask[stream->chnid] != '1') {
            ret = 0;
        }
        rtsp_server_debug(RTSP_INFO, "playback audio:%d mask:%s ret:%d", perm->playbackAudio, perm->playbackMask, ret);
    } else {
        if (perm->liveviewMask[stream->chnid] != '1') {
            ret = 0;
        }
        rtsp_server_debug(RTSP_INFO, "liveview audio:%d mask:%s ret:%d", perm->liveviewAudio, perm->liveviewMask, ret);
    }
    
    return ret;
}

static int ms_rtsp_audio_permissions(RTSP_SERVER_STREAM *stream, struct MsRtspPermissions *perm)
{
    int audio;

    if (!perm->userLevel) {
        return 1;
    }
    if (stream->isPlayback) {
        audio = perm->playbackAudio;
    } else {
        audio = perm->liveviewAudio;
    }

    return audio ? 1 : 0;
}

static int ms_rtsp_req_close_pb_stream(const char *streamName)
{
    if (!streamName 
        || !strstr(streamName, "pb_")
        || !cb_rtsp_server_req) {
        return -1;
    }

    RTSP_CB_REQ_S pbReq;
    RTSP_CB_RESP_S pbResp;

    memset(&pbReq, 0, sizeof(RTSP_CB_REQ_S));
    memset(&pbResp, 0, sizeof(RTSP_CB_RESP_S));

    pbReq.req = RTSP_REQ_CLOSE_PB;
    snprintf(pbReq.buf, sizeof(pbReq.buf), streamName);
    cb_rtsp_server_req(&pbReq, &pbResp);

    return 0;
}

static int ms_rtsp_ondescribe(void *ptr, rtsp_server_t *rtsp, const char *uri)
{
    int audioType = -1;
    char pAudCode[16] = {0};
    char pConfigStr[16] = {0};
    char nonce[64] = {0};
    char auth[512] = {0};
    char streamUri[256] = {0};
    char buffer[1024] = {0};
    int mcast;
    RTSP_SERVER_STREAM *stream = NULL;
    char *tmpNonce;
    struct MsRtspPermissions perm;
    int audioPerm;
    const char *ip = rtsp_server_get_client(rtsp, NULL);
    int ret = -1;
    RTSP_CB_REQ_S pbReq;
    RTSP_CB_RESP_S pbResp;
    memset(&pbReq, 0, sizeof(RTSP_CB_REQ_S));
    memset(&pbResp, 0, sizeof(RTSP_CB_RESP_S));

    rtsp_server_debug(RTSP_INFO,"[DESCRIBE] uri:%s rtsp:%p", uri, rtsp);
    if (g_rtsp_conf.ms_rp_access_filter(ip) == 0) {
        return rtsp_server_reply_forbidden(rtsp, 403);
    }

    if (!uri) {
        return rtsp_server_reply_describe(rtsp, 404, buffer);
    }
    
    ms_rtsp_uri_parse(uri, streamUri, sizeof(streamUri));
    rtsp_server_reply_auth(rtsp, auth, sizeof(auth));
    tmpNonce = rtsp_server_auth_get_nonce(rtsp);
    if (g_rtsp_conf.ms_rp_permissions(auth, tmpNonce, "DESCRIBE", &perm)) {
        ms_get_random_md5str(nonce, sizeof(nonce));
        rtsp_server_auth_set_nonce(rtsp, nonce);
        snprintf(buffer, sizeof(buffer), "WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"", MS_RTSP_SERVER, nonce);
        return rtsp_server_reply_unauthorized(rtsp, 401, buffer);
    }

    if (!strncmp(streamUri, MS_RTSP_PLAYBACK_URL, strlen(MS_RTSP_PLAYBACK_URL))) { // playback
        pbReq.req = RTSP_REQ_OPEN_PB;
        snprintf(pbReq.buf, sizeof(pbReq.buf), uri);
        if (cb_rtsp_server_req) {
            ret = cb_rtsp_server_req(&pbReq, &pbResp);
        }
        if (!ret) {
            rtsp_server_set_stream_type(rtsp, MS_STREAM_URL_PB);
            rtsp_server_set_stream_name(rtsp, pbResp.buf);
            stream = ms_streamlist_find_name(pbResp.buf);
        }
    } else {
        stream = ms_streamlist_find_name(streamUri);
    }
    
    if (!stream) {
        rtsp_server_debug(RTSP_INFO, "Stream:%s is Not Found", streamUri);
        return rtsp_server_reply_describe(rtsp, 404, buffer);
    }

    if (!ms_rtsp_chanenl_permissions(stream, &perm)) {
        rtsp_server_debug(RTSP_INFO, "streamUri:%s username:%s forbidden.", perm.username, streamUri);
        ms_rtsp_req_close_pb_stream(rtsp_server_get_stream_name(rtsp));
        return rtsp_server_reply_forbidden(rtsp, 403);
    }

    audioPerm = ms_rtsp_audio_permissions(stream, &perm);
    rtsp_server_auth_set_audio_perm(rtsp, audioPerm);
    mcast = get_ms_rtsp_uri_multicast(uri);
    ms_com_mutex_lock(&stream->hdlMutex);

    if (mcast) {
        snprintf(buffer, sizeof(buffer), DESCRIBE_HEADER_FMT, (unsigned long long)0, (unsigned long long)0, g_rtsp_conf.mcastIp);
    } else {
        snprintf(buffer, sizeof(buffer), DESCRIBE_HEADER_FMT, (unsigned long long)0, (unsigned long long)0, "0.0.0.0");
    }
    
    if (stream->nb_streams & (1 << DEFAULT_STREAM_VID)) {
        ms_sdp_video_describe(stream->vCodeType, buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), 0, 96, 90000,
                              stream->extraData, stream->exSize);
    }
    if (stream->nb_streams & (1 << DEFAULT_STREAM_AUD)) {
        audioType = ms_get_acodec_string(stream->aCodeType, pAudCode, sizeof(pAudCode));
        if (stream->aCodeType != MS_StreamCodecType_AAC) {
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), DESCRIBE_AUDIO_FMT, audioType, audioType, pAudCode,
                     stream->samplerate, 1);
        } else {
            ms_get_rtp_sdp_aac_config(stream->samplerate, pConfigStr, sizeof(pConfigStr));
            snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), DESCRIBE_AUDIO_AAC_FMT, audioType, audioType,
                     pAudCode, stream->samplerate, pConfigStr, 1);
        }
    }
    if (stream->nb_streams & (1 << DEFAULT_STREAM_MED)) {
        snprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), DESCRIBE_METADATA_FMT, 2);
    }
    ms_com_mutex_unlock(&stream->hdlMutex);

    return rtsp_server_reply_describe(rtsp, 200, buffer);
}

static int ms_rtsp_onsetup(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                           const struct rtsp_header_transport_t transports[], size_t num)
{
    int i = 0;
    char pSessionId[32] = {0};
    char streamUri[256] = {0};
    char rtsp_transport[128];
    int mcastRtpPort;
    int streamId;
    MS_RTSP_FRAME_VERSION_E frameVersion;
    MS_RTP_SOURCE *rtpSource = NULL;
    MS_RTP_CONTEXT *ctx = NULL;
    int audioPerm;
    const struct rtsp_header_transport_t *transport = NULL;
    const char *streamName;

    if (!uri) {
        rtsp_server_debug(RTSP_INFO, "[SETUP] uri is null. rtsp:%p", rtsp);
        streamName = rtsp_server_get_stream_name(rtsp);
        ms_rtsp_req_close_pb_stream(streamName);
        return rtsp_server_reply_setup(rtsp, 404, NULL, NULL);
    }

    rtsp_server_debug(RTSP_INFO,"[SETUP] uri:%s rtsp:%p session:%s", uri, rtsp, session);
    ms_rtsp_uri_parse(uri, streamUri, sizeof(streamUri));

    for (i = 0; i < num && !transport; i++) {
        if (RTSP_TRANSPORT_RTP_UDP == transports[i].transport) {
            // RTP/AVP/UDP
            transport = &transports[i];
        } else if (RTSP_TRANSPORT_RTP_TCP == transports[i].transport) {
            // RTP/AVP/TCP
            // 10.12 Embedded (Interleaved) Binary Data (p40)
            transport = &transports[i];
        }
    }

    if (!transport) {
        // 461 Unsupported Transport
        rtsp_server_debug(RTSP_WARN, "Stream:%s Unsupported Transport", streamUri);
        streamName = rtsp_server_get_stream_name(rtsp);
        ms_rtsp_req_close_pb_stream(streamName);
        return rtsp_server_reply_setup(rtsp, 461, NULL, NULL);
    }

    frameVersion = get_ms_rtsp_uri_version(uri);
    audioPerm = rtsp_server_auth_get_audio_perm(rtsp);
    if (RTSP_TRANSPORT_RTP_TCP == transport->transport) {
        // 10.12 Embedded (Interleaved) Binary Data (p40)
        int interleaved[2];
        if (transport->interleaved1 == transport->interleaved2) {
            interleaved[0] = i++;
            interleaved[1] = i++;
        } else {
            interleaved[0] = transport->interleaved1;
            interleaved[1] = transport->interleaved2;
        }

        // RTP/AVP/TCP;interleaved=0-1
        snprintf(rtsp_transport, sizeof(rtsp_transport), "RTP/AVP/TCP;interleaved=%d-%d", interleaved[0], interleaved[1]);
        rtpSource = ms_create_rtp_context(uri, session, RTSP_TRANSPORT_RTP_TCP, rtsp);
        if (!rtpSource) {
            rtsp_server_debug(RTSP_WARN, "[SETUP] Create failed, Not Enough Bandwidth. uri:%s", uri);
            streamName = rtsp_server_get_stream_name(rtsp);
            ms_rtsp_req_close_pb_stream(streamName);
            return rtsp_server_reply_setup(rtsp, 453, NULL, NULL);
        }
        ctx = (MS_RTP_CONTEXT *)rtpSource->owner;
        ms_com_mutex_lock(&ctx->ctxMutex);
        rtpSource->m_rtsp = rtsp;
        rtpSource->m_interleaved[0] = interleaved[0];
        rtpSource->m_interleaved[1] = interleaved[1];
        snprintf(pSessionId, sizeof(pSessionId), "%s", rtpSource->sessionId);
        ctx->mcast = 0;
        ctx->frameVersion = frameVersion;
        ctx->audioPerm = audioPerm;
        ms_com_mutex_unlock(&ctx->ctxMutex);
    } else {
        if (transport->multicast && !g_rtsp_conf.mcastEnable) {
            // RFC 2326 1.6 Overall Operation p12
            // Multicast, client chooses address
            // Multicast, server chooses address
            //461 Unsupported Transport
            return rtsp_server_reply_setup(rtsp, 461, NULL, NULL);
        }
        unsigned short port[2] = { transport->rtp.u.client_port1, transport->rtp.u.client_port2 };
        const char *ip = transport->destination[0] ? transport->destination : rtsp_server_get_client(rtsp, NULL);
        rtpSource = ms_create_rtp_context(uri, session, RTSP_TRANSPORT_RTP_UDP, rtsp);
        if (!rtpSource) {
            rtsp_server_debug(RTSP_WARN, "[SETUP] Create failed, Not Enough Bandwidth. uri:%s", uri);
            streamName = rtsp_server_get_stream_name(rtsp);
            ms_rtsp_req_close_pb_stream(streamName);
            return rtsp_server_reply_setup(rtsp, 453, NULL, NULL);
        }
        ctx = (MS_RTP_CONTEXT *)rtpSource->owner;
        ms_com_mutex_lock(&ctx->ctxMutex);
        rtpSource->m_rtsp = rtsp;
        snprintf(pSessionId, sizeof(pSessionId), "%s", rtpSource->sessionId);
        if (ms_rtp_udp_transport_create((MSRTPUDPTRANSPORT_S **)&rtpSource->m_transport)) {
            ms_com_mutex_unlock(&ctx->ctxMutex);
            ms_destroy_rtp_context(streamUri, session);
            rtsp_server_debug(RTSP_WARN, "[SETUP] IP:%s Rtp Udp Transport create is err!!!", ip);
            streamName = rtsp_server_get_stream_name(rtsp);
            ms_rtsp_req_close_pb_stream(streamName);
            return rtsp_server_reply_setup(rtsp, 500, NULL, NULL);
        }
        if (ms_rtp_udp_transport_init((MSRTPUDPTRANSPORT_S *)rtpSource->m_transport, ip, port)) {
            ms_com_mutex_unlock(&ctx->ctxMutex);
            ms_destroy_rtp_context(streamUri, session);
            rtsp_server_debug(RTSP_WARN, "[SETUP] IP:%s Rtp Udp Transport init is err!!!", ip);
            streamName = rtsp_server_get_stream_name(rtsp);
            ms_rtsp_req_close_pb_stream(streamName);

            return rtsp_server_reply_setup(rtsp, 500, NULL, NULL);
        }
        ctx->mcast = transport->multicast;
        ctx->frameVersion = frameVersion;
        ctx->audioPerm = audioPerm;
        ms_com_mutex_unlock(&ctx->ctxMutex);
        if (transport->multicast) {
            // multicast
            mcastRtpPort = get_ms_rtsp_multicast_rtp_port(streamUri);
            streamId = ms_get_rtp_streamid(uri);
            if (streamId != 0) {
                mcastRtpPort += 2;
            }
            snprintf(rtsp_transport, sizeof(rtsp_transport),"RTP/AVP/UDP;mcast;destination=%s;port=%d-%d",
                     g_rtsp_conf.mcastIp, mcastRtpPort, mcastRtpPort+1);
        } else {
            // unicast
            snprintf(rtsp_transport, sizeof(rtsp_transport),
                     "RTP/AVP;unicast;client_port=%hu-%hu;server_port=%hu-%hu%s%s",
                     transport->rtp.u.client_port1, transport->rtp.u.client_port2,
                     port[0], port[1],
                     transport->destination[0] ? ";destination=" : "",
                     transport->destination[0] ? transport->destination : "");
        }
    }

    return rtsp_server_reply_setup(rtsp, 200, pSessionId, rtsp_transport);
}

static int ms_rtsp_onplay(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                          const double *scale)
{
    // RFC 2326 12.33 RTP-Info (p55)
    // 1. Indicates the RTP timestamp corresponding to the time value in the Range response header.
    // 2. A mapping from RTP timestamps to NTP timestamps (wall clock) is available via RTCP.
    char rtpinfo[512] = {0};
    const char *streamName = NULL;
    MS_RTP_CONTEXT *ctx = NULL;

    if (!uri) {
        rtsp_server_debug(RTSP_WARN,"[PLAY] uri is null.");
        streamName = rtsp_server_get_stream_name(rtsp);
        ms_rtsp_req_close_pb_stream(streamName);
        rtsp_server_reply_play(rtsp, 404, npt, NULL, NULL);
    }
    rtsp_server_debug(RTSP_INFO,"[PLAY] uri:%s rtsp:%p session:%s", uri, rtsp, session);

    if (strstr(uri, MS_RTSP_PLAYBACK_URL)) {
        streamName = rtsp_server_get_stream_name(rtsp);
        ctx = ms_get_rtp_ctx(streamName, session);
    } else {
        ctx = ms_get_rtp_ctx(uri, session);
    }

    if (ctx) {
        ms_com_mutex_lock(&ctx->ctxMutex);
        ms_create_rtp_encoder(ctx);
        ctx->status = MS_CTX_OK;
        ms_com_mutex_unlock(&ctx->ctxMutex);
    }

    ms_get_rtp_info(uri, session, rtpinfo, sizeof(rtpinfo), rtsp);

    return rtsp_server_reply_play(rtsp, 200, npt, NULL, rtpinfo);
}

static int ms_rtsp_onpause(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt)
{
    rtsp_server_debug(RTSP_INFO,"[PAUSE] uri:%s rtsp:%p session:%s", uri, rtsp, session);
    return rtsp_server_reply_pause(rtsp, 200);
}

static int ms_rtsp_onteardown(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session)
{
    rtsp_server_debug(RTSP_INFO,"[TEARDOWN] uri:%s rtsp:%p session:%s", uri, rtsp, session);
    const char *pbStreamName = rtsp_server_get_stream_name(rtsp);
    ms_rtsp_req_close_pb_stream(pbStreamName);
    
    // 454 Session Not Found @David need to do
    ms_destroy_rtp_context(uri, session);
    return rtsp_server_reply_teardown(rtsp, 200);
}

static int ms_rtsp_onclose(void *ptr2)
{
    // TODO: notify rtsp connection lost
    // start a timer to check rtp/rtcp activity
    // close rtsp media session on expired
    rtsp_server_debug(RTSP_INFO, "rtsp close(%p)", ptr2);
    return 0;
}

static int ms_rtsp_onannounce(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *sdp)
{
    return rtsp_server_reply_announce(rtsp, 200);
}

static int ms_rtsp_onrecord(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session, const int64_t *npt,
                            const double *scale)
{
    return rtsp_server_reply_record(rtsp, 200, NULL, NULL);
}

static int ms_rtsp_ongetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                  const void *content, int bytes)
{
    // const char *ctype = rtsp_server_get_header(rtsp, "Content-Type");
    // const char *encoding = rtsp_server_get_header(rtsp, "Content-Encoding");
    // const char *language = rtsp_server_get_header(rtsp, "Content-Language");
    rtsp_server_debug(RTSP_INFO, "[david debug] uri:%s rtsp:%p", uri, rtsp);
    MS_RTP_CONTEXT *ctx = ms_get_rtp_ctx(uri, session);
    if (ctx) {
        if (ctx->connType == MS_StreamCtxConnType_HTTP) {
            rtsp_server_debug(RTSP_INFO, "Heartbeat package by Http, no need responese to send.");
            return 0;
        }
    }

    return rtsp_server_reply_get_parameter(rtsp, 200, NULL, 0);
}

static int ms_rtsp_onsetparameter(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *session,
                                  const void *content, int bytes)
{
    // const char *ctype = rtsp_server_get_header(rtsp, "Content-Type");
    // const char *encoding = rtsp_server_get_header(rtsp, "Content-Encoding");
    // const char *language = rtsp_server_get_header(rtsp, "Content-Language");
    return rtsp_server_reply_set_parameter(rtsp, 200);
}

static void ms_rtsp_onerror(void *param, rtsp_server_t *rtsp, int code)
{
    int i = 0, flag = 0;
    RTSP_SERVER_STREAM *stream = NULL;
    const char *streamName = NULL;

    if (!rtsp) {
        return;
    }

    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, stream, list) {
        if (!stream) {
            break;
        }
        ms_com_mutex_lock(&stream->hdlMutex);
        if (stream->ctxCnts <= 0) {
            ms_com_mutex_unlock(&stream->hdlMutex);
            continue;
        }

        for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
            if (!stream->ctx[i]) {
                continue;
            }

            ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
            if (stream->ctx[i]->status != MS_CTX_OK) {
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                continue;
            }
            if (stream->ctx[i]->video.m_rtsp == rtsp
                || stream->ctx[i]->audio.m_rtsp == rtsp
                || stream->ctx[i]->metadata.m_rtsp == rtsp) {
                ms_stop_rtp_context(stream, stream->ctx[i]);
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                flag = 1;
                break;
            }
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
        }
        ms_com_mutex_unlock(&stream->hdlMutex);
        if (flag) {
            break;
        }
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    if (rtsp_server_get_stream_type(rtsp) == MS_STREAM_URL_PB) {
        streamName = rtsp_server_get_stream_name(rtsp);
        ms_rtsp_req_close_pb_stream(streamName);
    }
    
    return ;
}

static int ms_rtsp_onextratask(void *ptr, rtsp_server_t *rtsp, const char *uri)
{
    char pSessionId[64] = {0};
    char nonce[64] = {0};
    char auth[256] = {0};
    char streamUri[256] = {0};
    char buffer[512] = {0};
    RTSP_SERVER_STREAM *stream = NULL;
    MS_RTP_CONTEXT *ctx = NULL;
    MS_RTP_SOURCE *rtpSource = NULL;
    MS_RTSP_FRAME_VERSION_E frameVersion;
    struct MsRtspPermissions perm;

    const char *ip = rtsp_server_get_client(rtsp, NULL);
    if (g_rtsp_conf.ms_rp_access_filter(ip) == 0) {
        rtsp_server_debug(RTSP_INFO, "403-access ip:%s uri:%s", ip, uri);
        return rtsp_server_reply_forbidden(rtsp, 403);
    }
    
    ms_http_uri_parse(uri, streamUri, sizeof(streamUri));
    frameVersion = get_ms_rtsp_uri_version(uri);
    rtsp_server_reply_auth(rtsp, auth, sizeof(auth));
    if (g_rtsp_conf.ms_rp_permissions(auth, NULL, "GET", &perm)) {
        ms_get_random_md5str(nonce, sizeof(nonce));
        snprintf(buffer, sizeof(buffer), "WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"", MS_RTSP_SERVER, nonce);
        rtsp_server_debug(RTSP_INFO, "401 ip:%s url:%s", ip, streamUri);
        return rtsp_server_reply_unauthorized_extratask(rtsp, 401, buffer);
    }
    
    stream = ms_streamlist_find_name(streamUri);
    if (!stream) {
        rtsp_server_debug(RTSP_INFO, "404 ip:%s url:%s", ip, streamUri);
        return rtsp_server_reply_extratask(rtsp, 404, NULL, NULL);
    }

    if (!ms_rtsp_chanenl_permissions(stream, &perm)) {
        rtsp_server_debug(RTSP_INFO, "403-permissions ip:%s url:%s", ip, streamUri);
        return rtsp_server_reply_forbidden_extratask(rtsp, 403);
    }

    rtpSource = ms_create_rtp_context(uri, NULL, RTSP_TRANSPORT_RAW, rtsp);
    if (!rtpSource) {
        rtsp_server_debug(RTSP_INFO, "453 Not Enough Bandwidth. ip:%s url:%s", ip, streamUri);
        return rtsp_server_reply_extratask(rtsp, 453, NULL, NULL);
    }

    ctx = (MS_RTP_CONTEXT *)rtpSource->owner;
    ms_com_mutex_lock(&ctx->ctxMutex);
    snprintf(pSessionId, sizeof(pSessionId), "%s", rtpSource->sessionId);
    rtpSource->m_rtsp = rtsp;
    ctx->frameVersion = frameVersion;
    ctx->audioPerm = ms_rtsp_audio_permissions(stream, &perm);
    ctx->status = MS_CTX_OK;
    ms_com_mutex_unlock(&ctx->ctxMutex);
    rtsp_server_debug(RTSP_INFO, "http client 200 ok. ip:%s url:%s", ip, streamUri);
    
    return rtsp_server_reply_extratask(rtsp, 200, pSessionId, NULL);
}

static inline int ms_rtsp_websocket_reply(rtsp_server_t *rtsp, int code, char *data)
{
    int len;
    char tmp[256] = {0};
    char resp[512] = {0};
    int nWebSockSize = 0;

    if (code == 101) {
        len = snprintf(resp, sizeof(resp),
                   "HTTP/1.1 %d Switching Protocols\r\n"
                   "Upgrade: websocket\r\n"
                   "Connection: Upgrade\r\n"
                   "Sec-WebSocket-Accept: %s\r\n"
                   "\r\n",
                   code, data);
    } else {
        len = snprintf(tmp, sizeof(tmp),
                   "RTSP/1.1 %d %s\r\n"
                   "\r\n",
                   code,
                   data);

        ms_get_websocket_header_opcode(resp, &nWebSockSize, len, 0);
        snprintf(resp+nWebSockSize, sizeof(resp) - nWebSockSize, "%s", tmp);
        len += nWebSockSize;
    }

    return rtsp_server_reply_websocket(rtsp, resp, len);
}

static int ms_rtsp_onwebsocket(void *ptr, rtsp_server_t *rtsp, const char *uri, const char *method)
{
    char buffer[256] = {0};
    char streamUri[64] = {0};
    char websocketKey[64] = {0};
    char websocketAccept[128] = {0};
    MS_RTSP_FRAME_VERSION_E frameVersion;
    unsigned char sha1_data[SHA_DIGEST_LENGTH + 1] = {0};
    RTSP_SERVER_STREAM *stream = NULL;
    MS_RTP_CONTEXT *ctx = NULL;
    MS_RTP_SOURCE *rtpSource = NULL;

    ms_http_uri_parse(uri, streamUri, sizeof(streamUri));
    frameVersion = get_ms_rtsp_uri_version(uri);
    rtsp_server_debug(RTSP_INFO, "rtsp:%p uri:%s streamUri:%s method:%s", rtsp, uri, streamUri, method);

    if (strstr(method, "GET_PARAMETER")) {
        //Heartbeat package by Websocket, no need responese to send.
        rtsp_server_debug(RTSP_INFO, "websocket GET_PARAMETER");
        return 0;
    }

    if (strstr(method, "GET")) {
        rtsp_server_get_websocket_key(rtsp, websocketKey, sizeof(websocketKey));
        snprintf(buffer, sizeof(buffer), "%s%s", websocketKey, WEBSOCKET_GUID);
        SHA1((unsigned char *)&buffer, strlen(buffer), (unsigned char *)&sha1_data);
        base64_encode(websocketAccept, sha1_data, SHA_DIGEST_LENGTH);
        rtsp_server_debug(RTSP_INFO, "websocket 101");
        return ms_rtsp_websocket_reply(rtsp, 101, websocketAccept);
    }

    if (strstr(method, "PLAY")) {
        stream = ms_streamlist_find_name(streamUri);
        if (!stream) {
            rtsp_server_debug(RTSP_INFO, "404 url:%s", streamUri);
            return ms_rtsp_websocket_reply(rtsp, 404, "Not Found");
        }

        rtpSource = ms_create_rtp_context(uri, NULL, RTSP_TRANSPORT_RAW, rtsp);
        if (!rtpSource) {
            rtsp_server_debug(RTSP_INFO, "453 Not Enough Bandwidth. url:%s", streamUri);
            return ms_rtsp_websocket_reply(rtsp, 453, "Not Enough Bandwidth");
        }

        ctx = (MS_RTP_CONTEXT *)rtpSource->owner;
        ms_com_mutex_lock(&ctx->ctxMutex);
        rtpSource->m_rtsp = rtsp;
        ctx->status = MS_CTX_OK;
        ctx->frameVersion = frameVersion;
        ctx->audioPerm = 1;
        ctx->userLevel = 0;
        ms_com_mutex_unlock(&ctx->ctxMutex);
        rtsp_server_debug(RTSP_INFO, "websocket client 200 ok. url:%s", streamUri);
        return 0;
    }

    return -1;
}

static int get_reqs_cmd(char *pReq, char *method, int len)
{
    if (pReq[0] == '\0') {
        return -1;
    }
    int i = 0;
    char pCmdTmp[32] = {0};
    do {
        if (pReq[i] == '\0') {
            break;
        }
        if (pReq[i] == ' ') {
            break;
        }
        if (i < sizeof(pCmdTmp) - 1) {
            pCmdTmp[i] = pReq[i];
            i++;
            continue;
        }
        pCmdTmp[i] = '\0';
        break;
    } while (1);

    snprintf(method, len, "%s", pCmdTmp);

    return 0;
}

static int get_reqs_url(char *pReq, char *url, int len)
{
    if (pReq[0] == '\0') {
        return -1;
    }
    int i = 0;
    char pUrlTmp[512] = {0};
    char *pStr = strstr(pReq, "rtsp:");
    if (!pStr) {
        return -1;
    }
    do {
        if (pStr[i] == ' ') {
            break;
        }
        if (pStr[i] == '\0') {
            break;
        }
        if (i < sizeof(pUrlTmp) - 1) {
            pUrlTmp[i] = pStr[i];
            i++;
            continue;
        }
        url[i] = '\0';
        break;
    } while (1);

    snprintf(url, len, "%s", pUrlTmp);

    return 0;
}

static int get_reqs_cseq(char *pReq, char *cseq, int len)
{
    if (pReq[0] == '\0') {
        return -1;
    }
    int i = 0;
    int nStep = strlen("CSeq: ");
    char *pStr = strstr(pReq, "CSeq: ");
    do {
        if (!pStr) {
            break;
        }
        if (pStr[i + nStep] != ' ' && pStr[i + nStep] != '\0'
            && pStr[i + nStep] != '\r' && i < (len - 1)) {
            cseq[i] = pStr[i + nStep];
            i++;
            continue;
        }
        cseq[i] = '\0';
        break;
    } while (1);

    return 0;
}

static int get_reqs_interleaved(char *pReq, int *interleaved0, int *interleaved1)
{
    if (pReq[0] == '\0') {
        return -1;
    }

    int i = 0, c = 0, n = 0;
    char str[8] = {0};
    int nStep = strlen("interleaved=");
    char *pStr = strstr(pReq, "interleaved=");
    do {
        if (!pStr) {
            break;
        }
        if (pStr[i + nStep] != ' ' && pStr[i + nStep] != '\0'
            && pStr[i + nStep] != '\r' && pStr[i + nStep] != '-' && c < 8) {
            str[c] = pStr[i + nStep];
            i++;
            c++;
            continue;
        }
        if (c < 8) {
            str[c] = '\0';
            if (n == 0) {
                *interleaved0 = atoi(str);
            } else {
                *interleaved1 = atoi(str);
            }
        } else {
            printf("error, get_reqs_interleaved, pReq:%s\n", pReq);
        }
        n++;
        i++;
        c = 0;
    } while (n < 2);
    rtsp_server_debug(RTSP_INFO, "[david debug] interleaved[0]:%d interleaved[1]:%d", *interleaved0, *interleaved0);

    return 0;
}

static int get_reqs_streamId(char *pReq, int *streamId)
{
    if (pReq[0] == '\0') {
        return -1;
    }
    int i = 0;
    char str[8] = {0};
    int nStep = strlen("streamid=");
    char *pStr = strstr(pReq, "streamid=");
    do {
        if (!pStr) {
            break;
        }
        if (pStr[i + nStep] != ' ' && pStr[i + nStep] != '\0'
            && pStr[i + nStep] != '\r') {
            str[i] = pStr[i + nStep];
            i++;
            continue;
        }
        str[i] = '\0';
        break;
    } while (1);
    *streamId = atoi(str);

    return 0;
}

static int ms_rtsp_over_http_response(MS_RTP_CONTEXT *ctx, char *data, int size)
{
    if (!ctx || !data || size <= 0) {
        return -1;
    }
    int audioType = -1;
    int streamId = -1;
    int interleaved[2] = {0};
    char pAudCode[16] = {0};
    char pConfigStr[16] = {0};
    char method[32] = {0};
    char cSeq[32] = {0};
    char nonce[64] = {0};
    char sTransport[128];
    char url[128] = {0};
    char sdp[1024] = {0};
    char rtpinfo[512] = {0};
    char buffer[1024] = {0};

    rtsp_server_t *rtsp = NULL;
    RTSP_SERVER_STREAM *stream = NULL;

    rtsp = ctx->video.m_rtsp;
    base64_decode(buffer, data, size);
    get_reqs_cmd(buffer, method, sizeof(method));
    get_reqs_url(buffer, url, sizeof(url));
    get_reqs_cseq(buffer, cSeq, sizeof(cSeq));
    get_reqs_interleaved(buffer, &interleaved[0], &interleaved[1]);
    get_reqs_streamId(buffer, &streamId);
    rtsp_server_debug(RTSP_INFO, "[david debug] rtsp:%p url:%s method:%s cSeq:%s", rtsp, url, method, cSeq);
    rtsp_server_debug(RTSP_INFO, "[david debug] interleaved[0]:%d interleaved[1]:%d streamId:%d", interleaved[0], interleaved[1],
                         streamId);
    switch (method[0]) {
        case 'o':
        case 'O':
            if (0 == strcasecmp("OPTIONS", method)) {
                return rtsp_server_reply_rohttp_option(rtsp, 200, cSeq);
            }
            break;

        case 'd':
        case 'D':
            if (0 == strcasecmp("DESCRIBE", method)) {
                if (!strstr(buffer, "Authorization")) {
                    ms_get_random_md5str(nonce, sizeof(nonce));
                    snprintf(sdp, sizeof(sdp), "WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"", MS_RTSP_SERVER, nonce);
                    return rtsp_server_reply_rohttp_unauthorized(rtsp, 401, cSeq, sdp);
                }
                stream = ctx->owner;
                ms_com_mutex_lock(&stream->hdlMutex);
                snprintf(sdp, sizeof(sdp), DESCRIBE_HEADER_FMT, (unsigned long long)0, (unsigned long long)0, "0.0.0.0");
                if (stream->nb_streams & (1 << DEFAULT_STREAM_VID)) {
                    ms_sdp_video_describe(stream->vCodeType, sdp + strlen(sdp), sizeof(sdp) - strlen(sdp), 0, 96, 90000,
                                          stream->extraData, stream->exSize);
                }
                if (stream->nb_streams & (1 << DEFAULT_STREAM_AUD)) {
                    audioType = ms_get_acodec_string(stream->aCodeType, pAudCode, sizeof(pAudCode));
                    
                    if (stream->aCodeType != MS_StreamCodecType_AAC) {
                        snprintf(sdp + strlen(sdp), sizeof(sdp) - strlen(sdp), DESCRIBE_AUDIO_FMT, audioType, audioType, pAudCode,
                             stream->samplerate, 1);
                    } else {
                        ms_get_rtp_sdp_aac_config(stream->samplerate, pConfigStr, sizeof(pConfigStr));
                        snprintf(sdp + strlen(sdp), sizeof(sdp) - strlen(sdp), DESCRIBE_AUDIO_AAC_FMT, audioType, audioType,
                                 pAudCode, stream->samplerate, pConfigStr, 1);
                    }
                }
                if (stream->nb_streams & (1 << DEFAULT_STREAM_MED)) {
                    snprintf(sdp + strlen(sdp), sizeof(sdp) - strlen(sdp), DESCRIBE_METADATA_FMT, 2);
                }
                ms_com_mutex_unlock(&stream->hdlMutex);

                return rtsp_server_reply_rohttp_describe(rtsp, 200, cSeq, sdp);
            }
            break;

        case 'g':
        case 'G':
            if (0 == strcasecmp("GET_PARAMETER", method)) {
                return rtsp_server_reply_rohttp_getparameter(rtsp, 200, cSeq, ctx->sessionId);
            }
            break;

        case 's':
        case 'S':
            if (0 == strcasecmp("SETUP", method)) {
                snprintf(sTransport, sizeof(sTransport), "RTP/AVP/TCP;interleaved=%d-%d", interleaved[0], interleaved[1]);
                if (streamId == DEFAULT_STREAM_VID) {
                    ms_com_mutex_lock(&ctx->ctxMutex);
                    ctx->video.m_interleaved[0] = interleaved[0];
                    ctx->video.m_interleaved[1] = interleaved[1];
                    ms_com_mutex_unlock(&ctx->ctxMutex);
                } else if (streamId == DEFAULT_STREAM_AUD) {
                    ms_create_rtp_context(buffer, ctx->sessionId, RTSP_TRANSPORT_RTP_TCP, rtsp);
                    ms_com_mutex_lock(&ctx->ctxMutex);
                    ctx->audio.m_interleaved[0] = interleaved[0];
                    ctx->audio.m_interleaved[1] = interleaved[1];
                    ms_com_mutex_unlock(&ctx->ctxMutex);
                } else {
                    ms_com_mutex_lock(&ctx->ctxMutex);
                    ctx->metadata.m_interleaved[0] = interleaved[0];
                    ctx->metadata.m_interleaved[1] = interleaved[1];
                    ms_com_mutex_unlock(&ctx->ctxMutex);
                }
                return rtsp_server_reply_rohttp_setup(rtsp, 200, cSeq, ctx->sessionId, sTransport);
            }
            break;

        case 'p':
        case 'P':
            if (0 == strcasecmp("PLAY", method)) {
                ms_com_mutex_lock(&ctx->ctxMutex);
                ms_create_rtp_encoder(ctx);
                ctx->status = MS_CTX_OK;
                ms_com_mutex_unlock(&ctx->ctxMutex);
                ms_get_rtp_info(url, ctx->sessionId, rtpinfo, sizeof(rtpinfo), rtsp);
                return rtsp_server_reply_rohttp_play(rtsp, 200, cSeq, 0, 0, rtpinfo);
            } else if (0 == strcasecmp("PAUSE", method)) {
                return rtsp_server_reply_rohttp_pause(rtsp, 200, cSeq);
            }
            break;

        case 't':
        case 'T':
            if (0 == strcasecmp("TEARDOWN", method)) {
                return rtsp_server_reply_rohttp_teardown(rtsp, 200, cSeq);
            }
            break;

        default:
            break;
    }

    return rtsp_server_reply_rohttp(rtsp, 500, "");
}

static int ms_rtsp_onrtspoverhttp(void *ptr, rtsp_server_t *rtsp, MS_RTSP_OVER_HTTP_PACKET *packet)
{
    char streamUri[64] = {0};

    RTSP_SERVER_STREAM *stream = NULL;
    MS_RTP_CONTEXT *ctx = NULL;
    MS_RTP_SOURCE *rtpSource = NULL;
    MS_RTSP_FRAME_VERSION_E frameVersion;

    ms_http_uri_parse(packet->uri, streamUri, sizeof(streamUri));
    frameVersion = get_ms_rtsp_uri_version(packet->uri);
    rtsp_server_debug(RTSP_INFO, "rtsp:%p uri:%s streamUri:%s method:%s type:%s xsessioncookie:%s", rtsp, packet->uri, streamUri,
                         packet->method,
                         (packet->sockType == 1) ? "GET" : "POST", packet->xSessionCookie);
    if (packet->sockType == 1) {
        if (strstr(packet->method, "GET_PARAMETER")) {
            rtsp_server_debug(RTSP_INFO, "rohttp GET_PARAMETER");
            return 0;
        }
        if (strstr(packet->method, "GET")) {
            stream = ms_streamlist_find_name(streamUri);
            if (!stream) {
                rtsp_server_debug(RTSP_INFO, "404 uri:%s", streamUri);
                return rtsp_server_reply_rohttp(rtsp, 404, "");
            }
            
            rtpSource = ms_create_rtp_context(packet->uri, NULL, RTSP_TRANSPORT_RTP_TCP, rtsp);
            if (!rtpSource) {
                rtsp_server_debug(RTSP_INFO, "453 Not Enough Bandwidth. uri:%s", streamUri);
                return rtsp_server_reply_rohttp(rtsp, 453, "");
            }

            ctx = (MS_RTP_CONTEXT *)rtpSource->owner;
            ms_com_mutex_lock(&ctx->ctxMutex);
            ctx->frameVersion = frameVersion;
            ctx->audioPerm = 1;
            ctx->userLevel = 0;
            rtpSource->m_rtsp = rtsp;
            snprintf(ctx->xSessionCookie, sizeof(ctx->xSessionCookie), "%s", packet->xSessionCookie);
            ms_com_mutex_unlock(&ctx->ctxMutex);
            return rtsp_server_reply_rohttp(rtsp, 200, "");
        }
    } else if (packet->sockType == 2) {
        ctx = ms_get_rtp_context_by_rohttp(rtsp, packet);
        if (!ctx) {
            return -1;
        }
        ms_com_mutex_lock(&ctx->ctxMutex);
        ctx->frameVersion = frameVersion;
        ctx->audioPerm = 1;
        ctx->userLevel = 0;
        ms_com_mutex_unlock(&ctx->ctxMutex);

        if (!strcmp(packet->method, "POST")) {
            return 0;
        }
        return ms_rtsp_over_http_response(ctx, packet->data, packet->nSize);
    } else {
        return -1;
    }

    return -1;
}

void *thread_rtsp_server_task(void *param)
{
    thread_set_name("rtsptask");
    int doFlag = 0;
    MS_AIO_READY_SEND *node = NULL;
    RTSP_SERVER_STREAM *stream = NULL;
    do {
        node = ms_aio_readylist_popfront();
        if (node) {
            if (node->action == MS_AIO_ADD_TASK) {
                ms_rtsp_onaioready_start(node->ctx, node->rtsp);
            }
            ms_aio_readylist_free(node);
            doFlag = 1;
        }

        stream = ms_removelist_popfront();
        if (stream) {
            rtsp_server_debug(RTSP_INFO, "[david debug] chnid:%d name:%s state:%d ctxcnt:%d", stream->chnid, stream->name, stream->state,
                         stream->ctxCnts);
            ms_removelist_free(stream);
            doFlag = 1;
        }
        
        if(!doFlag) {
            usleep(20*1000);
        }
        doFlag = 0;
    }while(g_rtsp_conf.rtsp_server_thread_run);

    thread_quit(NULL);
    return NULL;
}

void *create_rtsp_server_task(RTSP_MSSTREAM_CONF *conf)
{
    struct aio_rtsp_handler_t handler;
    
    aio_worker_init(MS_AIO_THREAD);
    set_rtsp_stream_print_func(conf->ms_rp_print);
    memset(&g_rtsp_conf, 0, sizeof(struct rtsp_server_stream_conf));
    g_rtsp_conf.loaded = 1;
    g_rtsp_conf.port = conf->port;
    g_rtsp_conf.max_rtsp_ctxcnt = 1024;
    g_rtsp_conf.max_rtsp_bandwidth = conf->bandwidth;
    
    g_rtsp_conf.ms_rp_malloc = conf->ms_rp_malloc;
    g_rtsp_conf.ms_rp_calloc = conf->ms_rp_calloc;
    g_rtsp_conf.ms_rp_free = conf->ms_rp_free;
    g_rtsp_conf.ms_rp_permissions = conf->ms_rp_permissions;
    g_rtsp_conf.ms_rp_access_filter = conf->ms_rp_access_filter;
    
    ms_com_mutex_init(&g_rtsp_conf.stream_mutex);
    ms_com_mutex_init(&g_rtsp_conf.remove_mutex);
    ms_com_mutex_init(&g_rtsp_conf.bandwidth_mutex);
    ms_com_mutex_init(&g_rtsp_conf.aioready_mutex);

    memset(&handler, 0, sizeof(handler));
    //tcp/udp
    handler.base.onoptions = ms_rtsp_onoptions;
    handler.base.ondescribe = ms_rtsp_ondescribe;
    handler.base.onsetup = ms_rtsp_onsetup;
    handler.base.onplay = ms_rtsp_onplay;
    handler.base.onpause = ms_rtsp_onpause;
    handler.base.onteardown = ms_rtsp_onteardown;
    handler.base.close = ms_rtsp_onclose;

    handler.base.onannounce = ms_rtsp_onannounce;
    handler.base.onrecord = ms_rtsp_onrecord;
    handler.base.onextratask = ms_rtsp_onextratask;//http
    handler.base.onwebsocket = ms_rtsp_onwebsocket;//websocket
    handler.base.onrtspoverhttp = ms_rtsp_onrtspoverhttp;//rtsp over http
    
    handler.base.ongetparameter = ms_rtsp_ongetparameter;
    handler.base.onsetparameter = ms_rtsp_onsetparameter;
    handler.onerror = ms_rtsp_onerror;
    handler.onaioready = ms_rtsp_onaioready_send;
    void *sock = rtsp_server_listen(NULL, g_rtsp_conf.port, &handler, NULL);
    if (!sock) {
        rtsp_server_debug(RTSP_ERR, "rtsp server listen is failed!\n");
        aio_worker_clean(MS_AIO_THREAD);
        return NULL;
    }

    g_rtsp_conf.rtsp_server_thread_run = 1;
    if (thread_create2(&g_rtsp_conf.rtsp_server_thread_handle, 16 * 1024, (thread_proc)thread_rtsp_server_task, NULL)) {
        g_rtsp_conf.rtsp_server_thread_run = 0;
        rtsp_server_debug(RTSP_ERR, "rtsp server listen is failed!\n");
        return NULL;
    }

    return sock;
}

void destory_rtsp_server_task(void *sock)
{
    if (g_rtsp_conf.rtsp_server_thread_run) {
        g_rtsp_conf.rtsp_server_thread_run = 0;
        thread_destroy(g_rtsp_conf.rtsp_server_thread_handle);
    }
    
    rtsp_server_unlisten(sock);
    ms_streamlist_clear();
    ms_stream_remove_clear();
    ms_aio_readylist_clear();
    
    ms_com_mutex_uninit(&g_rtsp_conf.stream_mutex);
    ms_com_mutex_uninit(&g_rtsp_conf.remove_mutex);
    ms_com_mutex_uninit(&g_rtsp_conf.bandwidth_mutex);
    ms_com_mutex_uninit(&g_rtsp_conf.aioready_mutex);
    
    aio_worker_clean(MS_AIO_THREAD);

    return ;
}

static int creat_multicast_ctx(RTSP_SERVER_STREAM *stream)
{
    int rtpPort;
    unsigned short port[2];
    MS_RTP_SOURCE *rtpSource = NULL;
    
    if (!stream) {
        return -1;
    }

    if (stream->mcastCtx) {
        return -2;
    }

    if (stream->isPlayback) {
        //pb stream not need.
        return -3;
    }
    
    stream->mcastCtx = (MS_RTP_CONTEXT *)ms_rtsp_pool_calloc(1, sizeof(MS_RTP_CONTEXT));
    memset(stream->mcastCtx, 0, sizeof(MS_RTP_CONTEXT));
    ms_com_mutex_init(&stream->mcastCtx->ctxMutex);
    ms_com_mutex_lock(&stream->mcastCtx->ctxMutex);
    stream->mcastCtx->owner = (void *)stream;
    stream->mcastCtx->connType = MS_StreamCtxConnType_RTSP;
    stream->mcastCtx->transType = RTSP_TRANSPORT_RTP_UDP;
    stream->mcastCtx->aioSend = 0;
    stream->mcastCtx->aioBuffSize = 0;
    MS_AST_LIST_HEAD_INIT_NOLOCK(&stream->mcastCtx->ctxdatalist);
    ms_create_rtp_encoder(stream->mcastCtx);
    //video
    rtpSource = &stream->mcastCtx->video;
    rtpSource->owner = (void *)stream->mcastCtx;
    if (ms_rtp_udp_transport_create((MSRTPUDPTRANSPORT_S **)&rtpSource->m_transport)) {
       ms_com_mutex_unlock(&stream->mcastCtx->ctxMutex);
       destroy_multicast_ctx(stream);
       return -3;
    }
    rtpPort = get_ms_rtsp_multicast_rtp_port(stream->name);
    port[0] = rtpPort;
    port[1] = port[0] + 1;
    if (ms_rtp_udp_transport_init((MSRTPUDPTRANSPORT_S *)rtpSource->m_transport, g_rtsp_conf.mcastIp, port)) {
        ms_com_mutex_unlock(&stream->mcastCtx->ctxMutex);
       destroy_multicast_ctx(stream);
       return -4;
    }
    ms_rtp_udp_transport_set_multicast((MSRTPUDPTRANSPORT_S *)rtpSource->m_transport, g_rtsp_conf.mcastIp);

    //audio
    rtpSource = &stream->mcastCtx->audio;
    rtpSource->owner = (void *)stream->mcastCtx;
    if (ms_rtp_udp_transport_create((MSRTPUDPTRANSPORT_S **)&rtpSource->m_transport)) {
       ms_com_mutex_unlock(&stream->mcastCtx->ctxMutex);
       destroy_multicast_ctx(stream);
       return -5;
    }
    port[0] = rtpPort + 2;
    port[1] = port[0] + 1;
    if (ms_rtp_udp_transport_init((MSRTPUDPTRANSPORT_S *)rtpSource->m_transport, g_rtsp_conf.mcastIp, port)) {
        ms_com_mutex_unlock(&stream->mcastCtx->ctxMutex);
       destroy_multicast_ctx(stream);
       return -6;
    }
    ms_rtp_udp_transport_set_multicast((MSRTPUDPTRANSPORT_S *)rtpSource->m_transport, g_rtsp_conf.mcastIp);
    
    ms_com_mutex_unlock(&stream->mcastCtx->ctxMutex);
    return 0;
}

static void destroy_multicast_ctx(RTSP_SERVER_STREAM *stream)
{
    if (!stream || !stream->mcastCtx) {
        return;
    }

    ms_com_mutex_lock(&stream->mcastCtx->ctxMutex);
    ms_stop_rtp_context(NULL, stream->mcastCtx);
    ms_com_mutex_unlock(&stream->mcastCtx->ctxMutex);
    ms_com_mutex_uninit(&stream->mcastCtx->ctxMutex);
    ms_rtsp_pool_free(stream->mcastCtx, sizeof(MS_RTP_CONTEXT));
    stream->mcastCtx = NULL;
    return;
}

static void init_ms_rtsp_multicast()
{
    RTSP_SERVER_STREAM *stream  = NULL;
    
    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, stream, list) {
        if (!stream) {
            continue;
        }

        ms_com_mutex_lock(&stream->hdlMutex);
        destroy_multicast_ctx(stream);
        if (g_rtsp_conf.mcastEnable) {
            creat_multicast_ctx(stream);
        }
        ms_com_mutex_unlock(&stream->hdlMutex);
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);

    return;
}

void ms_rtsp_set_multicast(int enable, char *ip)
{
    if (g_rtsp_conf.mcastEnable == enable && !strcmp(ip, g_rtsp_conf.mcastIp)) {
        return;
    }

    g_rtsp_conf.mcastEnable = enable;
    snprintf(g_rtsp_conf.mcastIp, sizeof(g_rtsp_conf.mcastIp), "%s", ip);
    init_ms_rtsp_multicast();
    return;
}

void ms_rtsp_update_filter()
{
    int i = 0;
    RTSP_SERVER_STREAM *stream = NULL;
    rtsp_server_t *rtsp = NULL;
    const char *ip;
    
    ms_com_mutex_lock(&g_rtsp_conf.stream_mutex);
    MS_AST_LIST_TRAVERSE(&streamlist, stream, list) {
        if (!stream) {
            break;
        }
        ms_com_mutex_lock(&stream->hdlMutex);
        if (stream->ctxCnts <= 0) {
            ms_com_mutex_unlock(&stream->hdlMutex);
            continue;
        }

        for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
            if (!stream->ctx[i]) {
                continue;
            }

            ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
            if (stream->ctx[i]->status != MS_CTX_OK) {
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                continue;
            }
            rtsp = stream->ctx[i]->video.m_rtsp;
            ip = rtsp_server_get_client(rtsp, NULL);
            if (g_rtsp_conf.ms_rp_access_filter(ip) != 0) {
                ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
                continue;
            }
            ms_stop_rtp_context(stream, stream->ctx[i]);
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
        }
        ms_com_mutex_unlock(&stream->hdlMutex);
    }
    ms_com_mutex_unlock(&g_rtsp_conf.stream_mutex);
    return ;
}

void ms_rtsp_server_set_debug(int enable)
{
    set_rtsp_stream_server_debug(enable);
}

int ms_rtsp_server_update_bandwidth(RTSP_SERVER_STREAM *stream, int bandwidth)
{
    int i;
    int value;
    char tmpStatus[32];
    char tmpConn[32];
    char tmpTrans[32];
    
    if (!stream) {
        rtsp_server_debug(RTSP_WARN, "stream:%p bandwidth:%d", stream, bandwidth);
        return -1;
    }
    
    ms_com_mutex_lock(&stream->hdlMutex);
    if (stream->bandWidth == bandwidth) {
        rtsp_server_debug(RTSP_DEBUG, "%s bandwidth not need changed. %d", stream->name, bandwidth);
        ms_com_mutex_unlock(&stream->hdlMutex);
        return 0;
    }
    value = (bandwidth - stream->bandWidth) * 1024;
    rtsp_server_debug(RTSP_DEBUG, "%s bandwidth changed from %d to %d", stream->name, stream->bandWidth, bandwidth);
    for (i = 0; i < MX_MAX_RTSP_CTX; i++) {
        if (!stream->ctx[i]) {
            continue;
        }
        ms_com_mutex_lock(&stream->ctx[i]->ctxMutex);
        if (stream->ctx[i]->status != MS_CTX_OK) {
            ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
            continue;
        }

        if (ms_bandwidth_limit(MS_RTSP_CHANGED, value)) {
            ms_get_ctx_status_string(stream->ctx[i]->status, tmpStatus, sizeof(tmpStatus));
            ms_get_ctx_conn_string(stream->ctx[i]->connType, tmpConn, sizeof(tmpConn));
            ms_get_ctx_transport_string(stream->ctx[i]->transType, tmpTrans, sizeof(tmpTrans));
            rtsp_server_debug(RTSP_DEBUG, "i:%d %s %s %s %s over bandwidth limit.", i, tmpStatus, tmpConn, tmpTrans, stream->ctx[i]->ip);
            ms_stop_rtp_context(stream, stream->ctx[i]);
        } else {
            stream->ctx[i]->bandwidth = bandwidth * 1024;
            ms_change_bandwidth(MS_RTSP_CHANGED, value);
        }
        ms_com_mutex_unlock(&stream->ctx[i]->ctxMutex);
    }
    stream->bandWidth = bandwidth;
    ms_com_mutex_unlock(&stream->hdlMutex);
    
    return 0;
}

