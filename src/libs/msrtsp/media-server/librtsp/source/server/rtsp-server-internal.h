#ifndef _rtsp_server_internal_h_
#define _rtsp_server_internal_h_

#include "rtsp-server.h"
#include "http-parser.h"
#include "rtsp-header-session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(OS_WINDOWS)
#define strcasecmp _stricmp
#endif

#define MAX_UDP_PACKAGE 1024

struct rtsp_server_t
{
	struct rtsp_handler_t handler;
	void *param, *sendparam;

	http_parser_t* parser;
	struct rtsp_header_session_t session;

	unsigned int cseq;
	char reply[MAX_UDP_PACKAGE];

	char ip[65]; // IPv4/IPv6
	unsigned short port;

    /*Ms Websocket*/
    int isWebsocket;        //0:No 1:Yes
    char websocketKey[64];

    /*Ms Rtsp Over Http*/
    int isRtspOverHttp;     //0:No 1:Get 2:POST
    char xSessionCookie[64];

    /*Ms User Permissions*/
    char nonce[64];
    int audioPerm;

    /* Ms Simple Playback Url */
    char streamName[64];
    MS_STREAM_E streamType;
};

int rtsp_server_handle(struct rtsp_server_t *rtsp);
int rtsp_server_options(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_announce(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_describe(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_setup(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_play(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_pause(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_teardown(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_get_parameter(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_set_parameter(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_record(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_reply(struct rtsp_server_t *rtsp, int code);
int rtsp_server_reply2(struct rtsp_server_t *rtsp, int code, const char* header, const void* data, int bytes);
int rtsp_server_reply3(struct rtsp_server_t *rtsp, int code, const char* header, const void* data, int bytes);
int rtsp_server_extratask(struct rtsp_server_t *rtsp, const char* uri);
int rtsp_server_websocket(struct rtsp_server_t *rtsp, const char *uri, const char *method);
int rtsp_server_rtsp_over_http(struct rtsp_server_t *rtsp, MS_RTSP_OVER_HTTP_PACKET *packet);

#endif /* !_rtsp_server_internal_h_ */
