#include "rtsp-server-internal.h"
#include "rfc822-datetime.h"

int rtsp_server_describe(struct rtsp_server_t *rtsp, const char* uri)
{
	return rtsp->handler.ondescribe(rtsp->param, rtsp, uri);
}

int rtsp_server_reply_describe(struct rtsp_server_t *rtsp, int code, const char* sdp)
{
	int len;
	rfc822_datetime_t datetime;

	if (200 != code)
		return rtsp_server_reply(rtsp, code);

	len = snprintf(rtsp->reply, sizeof(rtsp->reply),
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %u\r\n"
		"Date: %s\r\n"
		"Content-Type: application/sdp\r\n"
		"Content-Length: %u\r\n"
		"\r\n"
		"%s",
		rtsp->cseq,
		rfc822_datetime_format(time(NULL), datetime),
		(unsigned int)strlen(sdp), sdp);

	return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

int rtsp_server_reply_unauthorized(struct rtsp_server_t *rtsp, int code, const char* sdp)
{
	int len;
	rfc822_datetime_t datetime;

	if (401 != code)
		return rtsp_server_reply(rtsp, code);

	len = snprintf(rtsp->reply, sizeof(rtsp->reply),
		"RTSP/1.0 %d Unauthorized\r\n"
		"CSeq: %u\r\n"
		"Date: %s\r\n"
		"Content-Type: application/sdp\r\n"
		"%s\r\n"
		"Content-Length: %u\r\n"
		"\r\n",
		code,
		rtsp->cseq,
		rfc822_datetime_format(time(NULL), datetime),
		sdp, 0);

	return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

int rtsp_server_reply_auth(struct rtsp_server_t *rtsp, char *dst, int nLen)
{
	if (!rtsp) {
        return -1;
    }

    if (!rtsp->parser) {
        return -1;
    }
    
    const char *auth = http_get_header_by_name(rtsp->parser, "Authorization");
    if (auth) {
        snprintf(dst, nLen, "%s", auth);
        return 0;
    }
    
	return -1;
}

int rtsp_server_auth_set_nonce(struct rtsp_server_t *rtsp, char *nonce)
{
    snprintf(rtsp->nonce, sizeof(rtsp->nonce), "%s", nonce);
    return 0;
}

char *rtsp_server_auth_get_nonce(struct rtsp_server_t *rtsp)
{
    return rtsp->nonce;
}

int rtsp_server_auth_set_audio_perm(struct rtsp_server_t *rtsp, int opt)
{
    rtsp->audioPerm = opt;
    return 0;
}

int rtsp_server_auth_get_audio_perm(struct rtsp_server_t *rtsp)
{
    return rtsp->audioPerm;
}

int rtsp_server_reply_forbidden(struct rtsp_server_t *rtsp, int code)
{
	int len;
	rfc822_datetime_t datetime;

	if (403 != code)
		return rtsp_server_reply(rtsp, code);

	len = snprintf(rtsp->reply, sizeof(rtsp->reply),
		"RTSP/1.0 403 Forbidden\r\n"
		"CSeq: %u\r\n"
		"Date: %s\r\n"
		"\r\n",
		rtsp->cseq,
		rfc822_datetime_format(time(NULL), datetime));

	return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

