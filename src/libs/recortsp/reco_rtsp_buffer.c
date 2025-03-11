#include "reco_rtsp_buffer.h"
#include "reco_rtsp_common.h"
#include "msstd.h"
#include "recortsp.h"

struct rtsp_server_buffer *gServer = NULL;
void reco_rtsp_server_set_buffer(struct rtsp_server_buffer *buffer)
{
    if (!buffer) {
        rtsp_log(TSAR_WARN, "[david debug] reco_rtsp_server_set_buffer is faild");
    }
    gServer = buffer;
    rtsp_log(TSAR_INFO, "[david debug] reco_rtsp_server_set_buffer gServer:%p loaded:%d", gServer, gServer->loaded);

    return ;
}

void rtsp_buffer_show()
{
    if (gServer && gServer->loaded) {
        gServer->rtsp_show();
    } else {
        rtsp_log(TSAR_WARN, "rtsp_buffer show null.");
    }

    return ;
}

void *rtsp_buffer_malloc(int size)
{
    void *pbuffer = NULL;
    if (gServer && gServer->loaded) {
        gServer->rtsp_malloc(&pbuffer, size);
    } else {
        pbuffer = ms_malloc(size);
    }
    return pbuffer;
}

void *rtsp_buffer_calloc(int n, int size)
{
    void *pbuffer = NULL;

    if (gServer && gServer->loaded) {
        gServer->rtsp_calloc(&pbuffer, n, size);
    } else {
        pbuffer = ms_calloc(n, size);
    }

    return pbuffer;
}

void rtsp_buffer_free(void *p, int size)
{
    if (!p) {
        return;
    }

    if (gServer && gServer->loaded) {
        gServer->rtsp_free(p, size);
    } else {
        ms_free(p);
    }

    return ;
}

void rtsp_buffer_init(int bitrate)
{
    if (gServer && gServer->loaded) {
        gServer->rtsp_init(bitrate);
    } else {
        rtsp_log(TSAR_WARN, "rtsp_buffer init null.");
    }

    return ;
}

void rtsp_buffer_uninit()
{
    if (gServer && gServer->loaded) {
        gServer->rtsp_uninit();
    } else {
        rtsp_log(TSAR_WARN, "rtsp_buffer uninit null.");
    }

    return ;
}

void msrtsp_buffer_malloc(void **dst, int size, const char *file, int line)
{
    if (gServer && gServer->loaded) {
        gServer->rtsp_malloc(dst, size);
    } else {
        *dst = __ms_calloc(1, size, file, line);
    }
    return ;
}

void msrtsp_buffer_calloc(void **dst, int n, int size, const char *file, int line)
{
    if (gServer && gServer->loaded) {
        gServer->rtsp_calloc(dst, n, size);
    } else {
        *dst = __ms_calloc(n, size, file, line);
    }

    return ;
}

void msrtsp_buffer_free(void *p, int size, const char *file, int line)
{
    if (!p) {
        return;
    }

    if (gServer && gServer->loaded) {
        gServer->rtsp_free(p, size);
    } else {
        __ms_free(p, file, line);
    }

    return ;
}


