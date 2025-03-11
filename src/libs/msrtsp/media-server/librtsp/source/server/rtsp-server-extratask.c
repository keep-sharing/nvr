/*
 * Filename:        rtsp-server-extratask
 * Created at:      2021.08.19
 * Description:     rtsp server extratask for HTTP/Web
 * Author:          david
 * Copyright (C)    milesight
 * *******************************************************/
#include "rtsp-server-internal.h"
#include "rfc822-datetime.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define WEBSOCKET_MIN_OPCODE_LEN        126
#define WEBSOCKET_MEDIUM_OPCODE_LEN     127
#define WEBSOCKET_MEDIUM_LEN            65535
#define WEBSOCKET_TEXT_OPCODE           0x81
#define WEBSOCKET_BINARY_OPCODE         0x82

int rtsp_server_extratask(struct rtsp_server_t *rtsp, const char *uri)
{
    return rtsp->handler.onextratask(rtsp->param, rtsp, uri);
}

int rtsp_server_reply_unauthorized_extratask(struct rtsp_server_t *rtsp, int code, const char *sdp)
{
    int len;
    rfc822_datetime_t datetime;

    if (401 != code) {
        return rtsp_server_reply(rtsp, code);
    }

    len = snprintf(rtsp->reply, sizeof(rtsp->reply),
                   "RTSP/1.1 %d Unauthorized\r\n"
                   "CSeq: %u\r\n"
                   "Date: %s\r\n"
                   "%s\r\n"
                   "\r\n",
                   code,
                   rtsp->cseq,
                   rfc822_datetime_format(time(NULL), datetime),
                   sdp);

    return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

int rtsp_server_reply_extratask(struct rtsp_server_t *rtsp, int code, const char *session, const char *sdp)
{
    int len;
    rfc822_datetime_t datetime;

    if (200 != code) {
        return rtsp_server_reply(rtsp, code);
    }

    len = snprintf(rtsp->reply, sizeof(rtsp->reply),
                   "HTTP/1.1 200 OK\r\n"
                   "Server: Milesight HTTP Server\r\n"
                   "Date: %s\r\n"
                   "Session: %s\r\n"
                   "Connection: close\r\n"
                   "\r\n",
                   rfc822_datetime_format(time(NULL), datetime),
                   session);

    return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

int ms_packet_aio_send(struct rtsp_server_t *rtsp, char *dst, int nLen)
{
    if (!rtsp) {
        return -1;
    }

    if (!dst || nLen <= 0) {
        return -1;
    }

#if 0
    return rtsp->handler.send(rtsp->sendparam, dst, nLen);
#else
    return rtsp->handler.aiosend(rtsp->sendparam, dst, nLen);
#endif
}

int ms_rtp_tcp_transport_send(struct rtsp_server_t *rtsp, int rtporrtcp, char *dst, int nLen)
{
    if (!rtsp) {
        return -1;
    }

    if (!dst || nLen <= 0) {
        return -1;
    }
    
    int nSize = nLen;
    char m_packet[5120] = {0};
    if (nLen >= sizeof(m_packet)) {
        nSize = sizeof(m_packet) - 4;
    }

    //For Tcp InterLeaved Frame
    m_packet[0] = '$';
    m_packet[1] = rtporrtcp;
    m_packet[2] = (nLen >> 8) & 0xFF;
    m_packet[3] = nLen & 0xff;
    memcpy(m_packet + 4, dst, nSize);

    return rtsp->handler.send(rtsp->sendparam, m_packet, nSize + 4);
}

void rtsp_server_get_websocket_key(struct rtsp_server_t *rtsp, char *key, int size)
{
    snprintf(key, size, "%s", rtsp->websocketKey);

    return ;
}

int rtsp_server_reply_websocket(struct rtsp_server_t *rtsp, const char *response, int len)
{
    return rtsp->handler.send(rtsp->sendparam, response, len);
}

int rtsp_server_parse_websocket(const char *data, int size, MS_WEBSOCKET_FRAME_PACKET *head)
{
    int offset = 0;
    if (!data || size < 6) {
        printf("rtsp_server_parse_frame_head is err!!!\n");
        return -1;
    }

    /*fin and op code*/
    head->fin = (data[0] & 0x80) == 0x80;
    head->opcode = data[0] & 0x0F;
    offset += 1;
    
    /*mask*/
    head->mask = (data[1] & 0x80) == 0x80;
    
    /*get payload length*/
    head->payload_length = data[1] & 0x7F;
    offset += 1;
    
    if (head->payload_length == 126) {
        head->payload_length = (data[2] & 0xFF) << 8 | (data[3] & 0xFF);
        offset += 2;
    } else if (head->payload_length == 127) {
        memcpy((void *)&(head->payload_length), (void *)&data[2], 8);
        offset += 8;
    }

    /*read masking-key*/
    memcpy(head->masking_key, &data[offset], 4);
    offset += 4;
    
    //printf("===[davide debug] fin:%d opcode:%d mask:%d payload_length:%lld offset:%d===\n", head->fin, head->opcode,
    //       head->mask, head->payload_length, offset);
    if (size - offset > sizeof(head->payload_data)) {
        head->ndatalen = sizeof(head->payload_data);
    }else {
        head->ndatalen = size - offset;
    }
    memcpy(head->payload_data, &data[offset], head->ndatalen);
    //printf("===[davide debug] len:%d===\n", head->ndatalen);
    
    return head->ndatalen;
}

void rtsp_server_decode_websocket_paydata(char *data, int len, char *mask)
{
    int i = 0;
    for (i = 0; i < len; ++i) {
        *(data + i) ^= *(mask + (i % 4));
    }

    return ;
}

int rtsp_server_websocket(struct rtsp_server_t *rtsp, const char *uri, const char *method)
{
    if (!rtsp->handler.onwebsocket) {
        return -1;
    }
    
    return rtsp->handler.onwebsocket(rtsp->param, rtsp, uri, method);
}

int ms_websocket_header_send(struct rtsp_server_t *rtsp, uint64_t size, int type)
{
    if (!rtsp) {
        return -1;
    }

    if (size <= 0) {
        return -1;
    }
    
    int Opcode, len = 0;
    char data[10] = {0};
    
    Opcode = WEBSOCKET_BINARY_OPCODE;
	if (type) {
		Opcode = WEBSOCKET_TEXT_OPCODE;
	}
		
	if (size < WEBSOCKET_MIN_OPCODE_LEN) {       
		data[0] = Opcode;  
		data[1] = size;  
		len = 2;  
	} else if (size < WEBSOCKET_MEDIUM_LEN) {
		data[0] = Opcode;  
		data[1] = WEBSOCKET_MIN_OPCODE_LEN;  
		data[2] = (size>>8 & 0xFF);  
		data[3] = (size & 0xFF);  
		len = 4;  
	} else {
		data[0] = Opcode;  
		data[1] = WEBSOCKET_MEDIUM_OPCODE_LEN;  
		data[2] = ((size>>56) & 0xFF);
        data[3] = ((size>>48) & 0xFF);
        data[4] = ((size>>40) & 0xFF);
        data[5] = ((size>>32) & 0xFF);
        data[6] = ((size>>24) & 0xFF);
        data[7] = ((size>>16) & 0xFF);
        data[8] = ((size>>8) & 0xFF);
        data[9] = (size & 0xFF);
		len = 10; 
	} 
	
    return rtsp->handler.send(rtsp->sendparam, data, len);
}

int ms_websocket_packet_send(struct rtsp_server_t *rtsp, char *dst, int nLen)
{
    if (!rtsp) {
        return -1;
    }

    if (!dst || nLen <= 0) {
        return -1;
    }

    return rtsp->handler.send(rtsp->sendparam, dst, nLen);
}

int rtsp_server_rtsp_over_http(struct rtsp_server_t *rtsp, MS_RTSP_OVER_HTTP_PACKET *packet)
{
    if (!rtsp->handler.onrtspoverhttp) {
        return -1;
    }
    
    return rtsp->handler.onrtspoverhttp(rtsp->param, rtsp, packet);
}

int rtsp_server_reply_rohttp(struct rtsp_server_t *rtsp, int code, const char *sdp)
{
    int len;
    rfc822_datetime_t datetime;

    if (200 != code) {
        return rtsp_server_reply(rtsp, code);
    }
    
    len = snprintf(rtsp->reply, sizeof(rtsp->reply),
                   "HTTP/1.1 200 OK\r\n"
                   "Server: Milesight HTTP Server\r\n"
                   "Date: %s\r\n"
                   "Cache-Control: no-cache\r\n"
                   "Pragma: no-cache\r\n"
                   "Content-Type: application/x-rtsp-tunnelled\r\n"
                   "\r\n"
                   "%s",
                   rfc822_datetime_format(time(NULL), datetime),
                   sdp);

    return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

int rtsp_server_reply_rohttp_option(struct rtsp_server_t *rtsp, int code, const char *cseq)
{
    int len;
    rfc822_datetime_t datetime;
    rtsp->cseq = (unsigned int)atoi(cseq);
    if (200 != code) {
        return rtsp_server_reply(rtsp, code);
    }
    len = snprintf(rtsp->reply, sizeof(rtsp->reply),
                   "HTTP/1.1 200 OK\r\n"
                   "CSeq: %u\r\n"
                   "Date: %s\r\n"
                   "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER\r\n"
                   "\r\n",
                   rtsp->cseq, rfc822_datetime_format(time(NULL), datetime));
    return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

int rtsp_server_reply_rohttp_unauthorized(struct rtsp_server_t *rtsp, int code, const char *cseq, const char* sdp)
{
	int len;
	rfc822_datetime_t datetime;
    rtsp->cseq = (unsigned int)atoi(cseq);
    
	if (401 != code)
		return rtsp_server_reply(rtsp, code);
    
	len = snprintf(rtsp->reply, sizeof(rtsp->reply),
		"RTSP/1.0 %d Unauthorized\r\n"
		"CSeq: %u\r\n"
		"Date: %s\r\n"
		"%s\r\n"
		"\r\n",
		code,
		rtsp->cseq,
		rfc822_datetime_format(time(NULL), datetime),
		sdp);

	return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

int rtsp_server_reply_rohttp_describe(struct rtsp_server_t *rtsp, int code, const char *cseq, const char* sdp)
{
	int len;
	rfc822_datetime_t datetime;
    rtsp->cseq = (unsigned int)atoi(cseq);
    
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

int rtsp_server_reply_rohttp_setup(struct rtsp_server_t *rtsp, int code, const char *cseq, const char* sessionid, const char* transport)
{
	int n;
	char header[1024];

	// save session-id
	n = snprintf(rtsp->session.session, sizeof(rtsp->session.session), "%s", sessionid ? sessionid : "");
	if (n < 0 || n >= sizeof(rtsp->session.session))
	{
		assert(0); // sessionid too long
		return -1;
	}

	// RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257
	n = snprintf(header, sizeof(header), "Transport: %s\r\n", transport ? transport : "");
	if (n < 0 || n >= sizeof(header))
	{
		assert(0); // transport or sessionid too long
		return -1;
	}
    rtsp->cseq = (unsigned int)atoi(cseq);
	return rtsp_server_reply2(rtsp, code, header, NULL, 0);
}

int rtsp_server_reply_rohttp_play(struct rtsp_server_t *rtsp, int code, const char *cseq, const int64_t *nptstart, const int64_t *nptend, const char* rtp)
{
	int n = 0;
	char header[1024] = { 0 };

	if (n >= 0 && nptstart)
	{
		if (nptend)
			n += snprintf(header + n, sizeof(header) - n, "Range: npt=%.3f-%.3f\r\n", (float)(*nptstart / 1000.0f), (float)(*nptend / 1000.0f));
		else
			n += snprintf(header + n, sizeof(header) - n, "Range: npt=%.3f-\r\n", (float)(*nptstart / 1000.0f));
	}

	if (n >= 0 && rtp)
	{
		n += snprintf(header + n, sizeof(header) - n, "RTP-Info: %s\r\n", rtp);
	}

	if (n < 0 || n >= sizeof(header))
	{
		assert(0); // rtp-info too long
		return -1;
	}
    
    rtsp->cseq = (unsigned int)atoi(cseq);
	return rtsp_server_reply3(rtsp, code, header, NULL, 0);
}

int rtsp_server_reply_rohttp_pause(struct rtsp_server_t *rtsp, int code, const char *cseq)
{
    rtsp->cseq = (unsigned int)atoi(cseq);
	return rtsp_server_reply(rtsp, code);
}

int rtsp_server_reply_rohttp_teardown(struct rtsp_server_t *rtsp, int code, const char *cseq)
{
    rtsp->cseq = (unsigned int)atoi(cseq);
	return rtsp_server_reply(rtsp, code);
}

int rtsp_server_reply_rohttp_getparameter(struct rtsp_server_t *rtsp, int code, const char *cseq, const char* sessionid)
{
	int len;
	rfc822_datetime_t datetime;
    rtsp->cseq = (unsigned int)atoi(cseq);
    
	if (200 != code)
		return rtsp_server_reply(rtsp, code);

	len = snprintf(rtsp->reply, sizeof(rtsp->reply),
		"RTSP/1.0 200 OK\r\n"
		"CSeq: %u\r\n"
		"Date: %s\r\n"
		"Session: %s\r\n"
		"\r\n",
		rtsp->cseq,
		rfc822_datetime_format(time(NULL), datetime),
		sessionid);

	return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}


int rtsp_server_reply_forbidden_extratask(struct rtsp_server_t *rtsp, int code)
{
    int len;
    rfc822_datetime_t datetime;

    if (403 != code) {
        return rtsp_server_reply(rtsp, code);
    }

    len = snprintf(rtsp->reply, sizeof(rtsp->reply),
                   "RTSP/1.1 403 Forbidden\r\n"
                   "CSeq: %u\r\n"
                   "Date: %s\r\n"
                   "\r\n",
                   rtsp->cseq,
                   rfc822_datetime_format(time(NULL), datetime));

    return rtsp->handler.send(rtsp->sendparam, rtsp->reply, len);
}

