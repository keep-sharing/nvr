#include "rtsp-server-internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define MS_HTTP_RTSP_URI      "/ms/streaming/"
#define MS_WEB_RTSP_URI       "/ms/webstream/"
#define MS_ROHTTP_RTSP_URI    "/ms/httpstream/"

#if 0
void ms_server_time_to_string(char *stime, int nLen)
{
    if (!stime) {
        return;
    }
    struct tm temp;
    time_t ntime = time(0);

    localtime_r((long *)&ntime, &temp);
    snprintf(stime, nLen, "%4d-%02d-%02d_%02d_%02d_%02d",
             temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday,
             temp.tm_hour, temp.tm_min, temp.tm_sec);

    return ;
}

#define ms_rtp_server_print(format, ...) \
    do{\
        char stime[32] = {0};\
        ms_server_time_to_string(stime, sizeof(stime));\
        printf("[%s %d==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)
#else
#define ms_rtp_server_print(format, ...)
#endif
int rtsp_server_handle(struct rtsp_server_t *rtsp)
{
	char protocol[8];
	int major, minor;
	const char* uri;
	const char* method;
	const char* session;
    const char* pStr;
    MS_RTSP_OVER_HTTP_PACKET rt_packet;
        
	http_get_version(rtsp->parser, protocol, &major, &minor);
	if (1 != major && 0 != minor) {
		//505 RTSP Version Not Supported
		ms_rtp_server_print("#####505 RTSP Version Not Supported#####\n");
		return rtsp_server_reply(rtsp, 505);
	}

    uri = http_get_request_uri(rtsp->parser);
	method = http_get_request_method(rtsp->parser);
    
    if (!strstr(uri, MS_HTTP_RTSP_URI) && !strstr(uri, MS_WEB_RTSP_URI) && !strstr(uri, MS_ROHTTP_RTSP_URI)) {
        if (0 != http_get_header_by_name2(rtsp->parser, "CSeq", (int*)&rtsp->cseq)) {
    		// 400 Bad Request
    		ms_rtp_server_print("#####400 Bad Request#####\n");
    		return rtsp_server_reply(rtsp, 400);
    	}
    }
	
	// parse session
	rtsp->session.session[0] = 0; // clear session value
	session = http_get_header_by_name(rtsp->parser, "Session");
	if (session) {
        rtsp_header_session(session, &rtsp->session);
    }
		
    if (strstr(uri, MS_WEB_RTSP_URI)) {
        rtsp->isWebsocket = 1;
        pStr = http_get_header_by_name(rtsp->parser, "Sec-WebSocket-Key");
        snprintf(rtsp->websocketKey, sizeof(rtsp->websocketKey), "%s", pStr);
    }

    if (strstr(uri, MS_ROHTTP_RTSP_URI)) {
        if (strstr(method, "GET")) {
            rtsp->isRtspOverHttp = 1;
        } else {
            rtsp->isRtspOverHttp = 2;
        }
        pStr = http_get_header_by_name(rtsp->parser, "x-sessioncookie");
        snprintf(rtsp->xSessionCookie, sizeof(rtsp->xSessionCookie), "%s", pStr);
        memset(&rt_packet, 0, sizeof(MS_RTSP_OVER_HTTP_PACKET));
        rt_packet.sockType = rtsp->isRtspOverHttp;
        snprintf(rt_packet.uri, sizeof(rt_packet.uri), "%s", uri);
        snprintf(rt_packet.method, sizeof(rt_packet.method), "%s", method);
        snprintf(rt_packet.xSessionCookie, sizeof(rt_packet.xSessionCookie), "%s", rtsp->xSessionCookie);
    }
#if 1
    ms_rtp_server_print("#################[david debug] start####################\n");
    ms_rtp_server_print("[david debug] session:%s\n", session);
    ms_rtp_server_print("[david debug] method:%s\n", method);
    ms_rtp_server_print("[david debug] uri:%s\n", uri);
    if (rtsp->isWebsocket) {
        ms_rtp_server_print("[david debug] websocketKey:%s\n", rtsp->websocketKey);
    }
    if (rtsp->isRtspOverHttp) {
        ms_rtp_server_print("[david debug] xSessionCookie:%s\n", rtsp->xSessionCookie);
    }
    ms_rtp_server_print("#################[david debug] end######################\n");
#endif

	switch (*method)
	{
	case 'o':
	case 'O':
		if (0 == strcasecmp("OPTIONS", method))
			return rtsp_server_options(rtsp, uri);
		break;

	case 'd':
	case 'D':
		if (0 == strcasecmp("DESCRIBE", method) && rtsp->handler.ondescribe)
			return rtsp_server_describe(rtsp, uri);
		break;

	case 'g':
	case 'G':
		if (0 == strcasecmp("GET_PARAMETER", method) && rtsp->handler.ongetparameter) {
            return rtsp_server_get_parameter(rtsp, uri);
        }
        if (0 == strcasecmp("GET", method) && rtsp->handler.onwebsocket && rtsp->isWebsocket) {
            return rtsp_server_websocket(rtsp, uri, method);
        }
        if (0 == strcasecmp("GET", method) && rtsp->handler.onrtspoverhttp && rtsp->isRtspOverHttp) {
            return rtsp_server_rtsp_over_http(rtsp, &rt_packet);
        }
        if (0 == strcasecmp("GET", method) && rtsp->handler.onextratask) {
            return rtsp_server_extratask(rtsp, uri);
        }
		break;

	case 's':
	case 'S':
		if (0 == strcasecmp("SETUP", method) && rtsp->handler.onsetup)
			return rtsp_server_setup(rtsp, uri);
		else if (0 == strcasecmp("SET_PARAMETER", method) && rtsp->handler.onsetparameter)
			return rtsp_server_set_parameter(rtsp, uri);
		break;

	case 'p':
	case 'P':
        if (0 == strcasecmp("POST", method) && rtsp->handler.onrtspoverhttp && rtsp->isRtspOverHttp) {
            return rtsp_server_rtsp_over_http(rtsp, &rt_packet);
        }
		if (0 == strcasecmp("PLAY", method) && rtsp->handler.onplay) {
            return rtsp_server_play(rtsp, uri);
        }
		else if (0 == strcasecmp("PAUSE", method) && rtsp->handler.onpause) {
            return rtsp_server_pause(rtsp, uri);
        }
		break;

	case 't':
	case 'T':
		if (0 == strcasecmp("TEARDOWN", method) && rtsp->handler.onteardown)
			return rtsp_server_teardown(rtsp, uri);
		break;

    case 'a':
    case 'A':
        if (0 == strcasecmp("ANNOUNCE", method) && rtsp->handler.onannounce)
            return rtsp_server_announce(rtsp, uri);
        break;

    case 'r':
    case 'R':
        if (0 == strcasecmp("RECORD", method) && rtsp->handler.onrecord)
            return rtsp_server_record(rtsp, uri);
        break;
    default:
        break;
	}

	// 501 Not implemented
	ms_rtp_server_print("#####501 Not implemented#####\n");
	return rtsp_server_reply(rtsp, 501);
}
