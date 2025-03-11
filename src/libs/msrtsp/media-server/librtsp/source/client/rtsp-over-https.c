/*
C->S: 
GET /main HTTP/1.1
CSeq: 1
User-Agent: LibVLC/3.0.4 (LIVE555 Streaming Media v2016.11.28)
Host: 192.168.63.242
x-sessioncookie: 09203cfdf98ea43d26ed690
Accept: application/x-rtsp-tunnelled
Pragma: no-cache
Cache-Control: no-cache

S->C: 
HTTP/1.0 200 OK
Date: Thu, 19 Aug 1982 18:30:00 GMT
Cache-Control: no-cache
Pragma: no-cache
Content-Type: application/x-rtsp-tunnelled
*/

#include "rtsp-client-internal.h"
#include <time.h>
static const char* sc_format_data =
		"GET /%s HTTP/1.1\r\n"
		"CSeq: %u\r\n"
        "User-Agent: %s\r\n"
		"%s" // "Host:192.168.0.1:443\r\n"
		"%s" // "x-sessioncookie: xxx\r\n"
		"Accept: application/x-rtsp-tunnelled\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
		"\r\n";
static const char* sc_format_command =
		"POST /%s HTTP/1.1\r\n"
		"CSeq: 1\r\n"
        "User-Agent: %s\r\n"
		"%s" // "Host:192.168.0.1:443\r\n"
		"%s" // "x-sessioncookie: xxx\r\n"
		"Content-Type: application/x-rtsp-tunnelled\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache\r\n"
        "Content-Length: 32767\r\n"
        "Expires: Sun, 9 Jan 1972 00:00:00 GMT\r\n"
		"\r\n";


int rtsp_client_over_https_data(struct rtsp_client_t *rtsp, const char* host, const char* x_session, const char* path)
{
	int r = 0;
	char require[128] = {0};
    snprintf(require, sizeof(require),"Host: %s\r\n", host);
	r = snprintf(rtsp->req, sizeof(rtsp->req), sc_format_data, path, rtsp->cseq++, USER_AGENT, require, x_session);
	assert(r > 0 && r < sizeof(rtsp->req));
	return r == rtsp->handler.send(rtsp->param, rtsp->uri, rtsp->req, r) ? 0 : -1;
}

int rtsp_client_over_https_command(struct rtsp_client_t *rtsp, const char* host, const char* x_session, const char* path)
{
	int r = 0;
	char require[128] = {0};
    snprintf(require, sizeof(require),"Host: %s\r\n", host);
	r = snprintf(rtsp->req, sizeof(rtsp->req), sc_format_command, path, USER_AGENT, require, x_session);
	assert(r > 0 && r < sizeof(rtsp->req));
	return r == rtsp->handler.send(rtsp->param, rtsp->uri, rtsp->req, r) ? 0 : -1;
}