#include "rtsp-server.h"
#include "rtsp-reason.h"
#include "rfc822-datetime.h"
#include "rtsp-server-internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct rtsp_server_t* rtsp_server_create(const char ip[65], unsigned short port, struct rtsp_handler_t* handler, void* ptr, void* ptr2)
{
	struct rtsp_server_t* rtsp;

	rtsp = (struct rtsp_server_t *)calloc(1, sizeof(struct rtsp_server_t));
	if (NULL == rtsp) return NULL;

	snprintf(rtsp->ip, sizeof(rtsp->ip), "%s", ip);
	rtsp->port = port;
	rtsp->param = ptr;
	rtsp->sendparam = ptr2;
	memcpy(&rtsp->handler, handler, sizeof(rtsp->handler));
	rtsp->parser = http_parser_create(HTTP_PARSER_SERVER);
	return rtsp;
}

int rtsp_server_destroy(struct rtsp_server_t* rtsp)
{
	if (rtsp->handler.close)
		rtsp->handler.close(rtsp->sendparam);

	if (rtsp->parser)
	{
		http_parser_destroy(rtsp->parser);
		rtsp->parser = NULL;
	}

	free(rtsp);
	return 0;
}

int rtsp_server_input(struct rtsp_server_t* rtsp, const void* data, size_t* bytes)
{
	int r = 0;
    char uri[64] = {0};
    char method[64] = {0};
    const char *mydata = data;
    MS_WEBSOCKET_FRAME_PACKET head;
    MS_RTSP_OVER_HTTP_PACKET rtpacket;
    
    /*david for deal websocket reqs 20210914*/
    if (rtsp->isWebsocket) {
        memset(&head, 0, sizeof(MS_WEBSOCKET_FRAME_PACKET));
        rtsp_server_parse_websocket(mydata, *bytes, &head);  
    
        if (head.mask) {
            rtsp_server_decode_websocket_paydata(head.payload_data, head.ndatalen, head.masking_key);
        }
        snprintf(uri, sizeof(uri), "%s", head.payload_data);
        char *p = strstr(uri, ":");
        if(p) {
            *p = '\0';
        }
        p = strstr(head.payload_data, ":");
        if (p) {
            snprintf(method, sizeof(method), "%s", p+1);
        } else {
            snprintf(method, sizeof(method), "%s", "PLAY");
        }
        *bytes = 0;
        return rtsp_server_websocket(rtsp, uri, method);
    }

    /*david for deal rtsp over http reqs 20210915*/
    if (rtsp->isRtspOverHttp) {
        memset(&rtpacket, 0, sizeof(MS_RTSP_OVER_HTTP_PACKET));
        rtpacket.sockType = rtsp->isRtspOverHttp;
        snprintf(rtpacket.xSessionCookie, sizeof(rtpacket.xSessionCookie), "%s", rtsp->xSessionCookie);
        if (*bytes > sizeof(rtpacket.data)) {
            rtpacket.nSize = sizeof(rtpacket.data);
        } else {
            rtpacket.nSize = *bytes;
        }
        memcpy(rtpacket.data, mydata, rtpacket.nSize);
        *bytes = 0;
        return rtsp_server_rtsp_over_http(rtsp, &rtpacket);
    }
    
	r = http_parser_input(rtsp->parser, data, bytes);
	assert(r <= 2); // 1-need more data
	if (0 == r) {
		r = rtsp_server_handle(rtsp);
		http_parser_clear(rtsp->parser); // reset parser
	} else {
        printf("=======[david debug] rtsp_server_input r:%d=======\n", r);
        printf("=======[david debug] rtsp_server_input data:%s=======\n", (char*)data);
    }
    
	return r;
}

const char* rtsp_server_get_header(struct rtsp_server_t *rtsp, const char* name)
{
	return http_get_header_by_name(rtsp->parser, name);
}

const char* rtsp_server_get_client(rtsp_server_t* rtsp, unsigned short* port)
{
    if (!rtsp) {
        return NULL;
    }
	if (port) {
        *port = rtsp->port;
	}
	return rtsp->ip;
}

void *rtsp_server_get_session(rtsp_server_t* rtsp)
{
    if (!rtsp) {
        return NULL;
    }
    return rtsp->sendparam;
}


int rtsp_server_reply(struct rtsp_server_t *rtsp, int code)
{
	return rtsp_server_reply2(rtsp, code, NULL, NULL, 0);
}

int rtsp_server_reply2(struct rtsp_server_t *rtsp, int code, const char* header, const void* data, int bytes)
{
	int len;
	rfc822_datetime_t datetime;
	rfc822_datetime_format(time(NULL), datetime);

	// smpte=0:10:22-;time=19970123T153600Z
	len = snprintf(rtsp->reply, sizeof(rtsp->reply),
		"RTSP/1.0 %d %s\r\n"
		"CSeq: %u\r\n"
		"Date: %s\r\n",
		code, rtsp_reason_phrase(code), rtsp->cseq, datetime);

	// session header
	if (len > 0 && rtsp->session.session[0])
	{
		len += snprintf(rtsp->reply + len, sizeof(rtsp->reply) - len, "Session: %s\r\n", rtsp->session.session);
	}

	// other headers
	if(len > 0 && header && *header)
	{
		len += snprintf(rtsp->reply + len, sizeof(rtsp->reply) - len, "%s", header);
	}

	// Last: Add Content-Length
	if (len > 0)
	{
		len += snprintf(rtsp->reply + len, sizeof(rtsp->reply) - len, "Content-Length: %d\r\n\r\n", bytes);
	}

	if (len < 0 || bytes < 0 || len + bytes >= sizeof(rtsp->reply))
		return rtsp_server_reply(rtsp, 513 /*Message Too Large*/);
	memcpy(rtsp->reply + len, data, bytes);
	return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len + bytes);
}

int rtsp_server_reply3(struct rtsp_server_t *rtsp, int code, const char* header, const void* data, int bytes)
{
	int len;
	rfc822_datetime_t datetime;
	rfc822_datetime_format(time(NULL), datetime);

	// smpte=0:10:22-;time=19970123T153600Z
	len = snprintf(rtsp->reply, sizeof(rtsp->reply),
		"RTSP/1.0 %d %s\r\n"
		"CSeq: %u\r\n"
		"Date: %s\r\n",
		code, rtsp_reason_phrase(code), rtsp->cseq, datetime);

	// session header
	if (len > 0 && rtsp->session.session[0])
	{
		len += snprintf(rtsp->reply + len, sizeof(rtsp->reply) - len, "Session: %s\r\n", rtsp->session.session);
	}

	// other headers
	if(len > 0 && header && *header)
	{
		len += snprintf(rtsp->reply + len, sizeof(rtsp->reply) - len, "%s\r\n", header);
	}

	if (len < 0 || bytes < 0 || len + bytes >= sizeof(rtsp->reply))
		return rtsp_server_reply(rtsp, 513 /*Message Too Large*/);
	memcpy(rtsp->reply + len, data, bytes);
	return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len + bytes);
}

int rtsp_server_send_interleaved_data(rtsp_server_t* rtsp, const void* data, size_t bytes)
{
	return rtsp->handler.send(rtsp->sendparam, data, bytes);
}

const char *rtsp_server_get_stream_name(struct rtsp_server_t *rtsp)
{
    if (!rtsp) {
        return NULL;
    }
    
    return rtsp->streamName;
}

int rtsp_server_set_stream_name(struct rtsp_server_t *rtsp, const char *streamName)
{
    if (!rtsp || !streamName) {
        return -1;
    }
    
    snprintf(rtsp->streamName, sizeof(rtsp->streamName), streamName);

    return 0;
}

MS_STREAM_E rtsp_server_get_stream_type(struct rtsp_server_t *rtsp)
{
    if (!rtsp) {
        return MS_STREAM_UNKNOW;
    }

    return rtsp->streamType;
}

int rtsp_server_set_stream_type(struct rtsp_server_t *rtsp, MS_STREAM_E streamType)
{
    if (!rtsp) {
        return -1;
    }

    rtsp->streamType = streamType;
    
    return 0;
}


