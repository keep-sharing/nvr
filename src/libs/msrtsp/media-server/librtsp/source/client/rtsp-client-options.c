/*
C->S: 
OPTIONS * RTSP/1.0
CSeq: 1
Require: implicit-play
Proxy-Require: gzipped-messages

S->C: 
RTSP/1.0 200 OK
CSeq: 1
Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE
*/

#include "rtsp-client-internal.h"

static const char* sc_format =
		"OPTIONS %s RTSP/1.0\r\n"
		"CSeq: %u\r\n"
		"%s" // "Require: implicit-play\r\n"
		"%s" // "Session: xxx\r\n"
		"%s" // "Authorization: Digest xxx"
		"User-Agent: %s\r\n"
		"\r\n";

int rtsp_client_options(struct rtsp_client_t *rtsp, const char* commands)
{
	int r = 0;
	char require[128];
	char session[128];

	require[0] = '\0';
	session[0] = '\0';
	rtsp->state = RTSP_OPTIONS;

	if (commands && commands[0])
		snprintf(require, sizeof(require), "Require: %s\r\n", commands);
	if (rtsp->media_count > 0 && *rtsp->session[0].session)
		snprintf(session, sizeof(session), "Session: %s\r\n", rtsp->session[0].session);
	
	r = rtsp_client_authenrization(rtsp, "OPTIONS", rtsp->uri, NULL, 0, rtsp->authenrization, sizeof(rtsp->authenrization));
	r = snprintf(rtsp->req, sizeof(rtsp->req), sc_format, rtsp->uri, rtsp->cseq++, require, session, rtsp->authenrization, USER_AGENT);

	assert(r > 0 && r < sizeof(rtsp->req));
	return r == rtsp->handler.send(rtsp->param, rtsp->uri, rtsp->req, r) ? 0 : -1;
}

static uint32_t rtsp_client_method_str2mask(const char *methods)
{
    if (!methods) {
        return 0;
    }

    uint32_t methodMask = 0;
    
    if (strstr(methods, "OPTIONS")) {
        methodMask |= (1 << RTSP_OPTIONS);
    }
    if (strstr(methods, "DESCRIBE")) {
        methodMask |= (1 << RTSP_DESCRIBE);
    }
    if (strstr(methods, "SETUP")) {
        methodMask |= (1 << RTSP_SETUP);
    }
    if (strstr(methods, "TEARDOWN")) {
        methodMask |= (1 << RTSP_TEARDWON);
    }
    if (strstr(methods, "PLAY")) {
        methodMask |= (1 << RTSP_PLAY);
    }
    if (strstr(methods, "PAUSE")) {
        methodMask |= (1 << RTSP_PAUSE);
    }
    if (strstr(methods, "SET_PARAMETER")) {
        methodMask |= (1 << RTSP_SET_PARAMETER);
    }
    if (strstr(methods, "GET_PARAMETER")) {
        methodMask |= (1 << RTSP_GET_PARAMETER);
    }

    return methodMask;
}

int rtsp_client_options_onreply(struct rtsp_client_t* rtsp, void* parser)
{
	int code, r = -1;
	const char *methods;

	if(RTSP_OPTIONS != rtsp->state)
		return -1;
	code = http_get_status_code(parser);
	if (200 == code)
    {
        methods = http_get_header_by_name(parser, "Public");
        rtsp->methodMask = rtsp_client_method_str2mask(methods);
		rtsp->auth_failed = 0;//reset auth_failed
		r = rtsp->handler.onoption(rtsp->param);
		return r;
	}
	else if (401 == code)
	{
		// Unauthorized
		const char* authenticate;
		int ignore_pos = 0;
		if(0 == rtsp->auth_failed++)
		{
			do
			{
				authenticate = http_get_header_by_name_and_ignorepos(parser, "WWW-Authenticate", ignore_pos++);
				if (authenticate)
				{
					if(!rtsp_client_www_authenticate(rtsp, authenticate))
					{	
						r = rtsp_client_options(rtsp, NULL);
						break;
					}
				}
				else 
					break;
			}while(1);
		}
	}	

	return r;
}
