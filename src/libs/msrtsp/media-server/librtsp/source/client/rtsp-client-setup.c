/*
C->S:
SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0
CSeq: 302
Transport: RTP/AVP;unicast;client_port=4588-4589

S->C: 
RTSP/1.0 200 OK
CSeq: 302
Date: 23 Jan 1997 15:35:06 GMT
Session: 47112344
Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=6256-6257
*/
#include "rtsp-client-internal.h"

static const char* sc_rtsp_tcp = "SETUP %s RTSP/1.0\r\n"
								"CSeq: %u\r\n"
								"%s" // Session: %s\r\n
								"%s" // Authorization: Digest xxx
								"Transport: RTP/AVP/TCP;unicast;interleaved=%u-%u\r\n"
								"User-Agent: %s\r\n"
								"\r\n";

static const char* sc_rtsp_udp = "SETUP %s RTSP/1.0\r\n"
								"CSeq: %u\r\n"
								"%s" // Session: %s\r\n
								"%s" // Authorization: Digest xxx
								"Transport: RTP/AVP;unicast;client_port=%u-%u\r\n"
								"User-Agent: %s\r\n"
								"\r\n";

static int rtsp_client_media_setup(struct rtsp_client_t* rtsp, int i)
{
	int len;
	char session[sizeof(rtsp->session[0].session) + 12], *p;

	assert(i < rtsp->media_count);
	assert(RTSP_SETUP == rtsp->state);
	if (i >= rtsp->media_count) return -1;

	p = rtsp->session[0].session;
	//wyl del for rtsp->aggregate
	//len = snprintf(session, sizeof(session), (rtsp->aggregate && *p) ? "Session: %s\r\n" : "", p);
	len = snprintf(session, sizeof(session), (*p) ? "Session: %s\r\n" : "", p);
	assert(len >= 0 && len < sizeof(session));

	// TODO: multicast
	assert(0 == rtsp->transport[i].multicast);
	len = rtsp_client_authenrization(rtsp, "SETUP", rtsp->media[i].uri, NULL, 0, rtsp->authenrization, sizeof(rtsp->authenrization));
	len = snprintf(rtsp->req, sizeof(rtsp->req),
			RTSP_TRANSPORT_RTP_TCP==rtsp->transport[i].transport?sc_rtsp_tcp:sc_rtsp_udp,
			rtsp->media[i].uri, rtsp->cseq++, session, rtsp->authenrization, rtsp->transport[i].rtp.u.client_port1, rtsp->transport[i].rtp.u.client_port2, USER_AGENT);

	return len == rtsp->handler.send(rtsp->param, rtsp->media[i].uri, rtsp->req, len) ? 0 : -1;
}

int rtsp_client_setup(struct rtsp_client_t* rtsp, const char* sdp)
{
	int i, r;
	struct rtsp_header_transport_t *t;

	if (NULL == sdp || 0 == *sdp) {
        printf("[david debug] rtsp_client_setup sdp is null.\n");
        return -1;
    }
		
	r = rtsp_media_sdp(sdp, rtsp->media, sizeof(rtsp->media)/sizeof(rtsp->media[0]));
	if (r < 0 || r > sizeof(rtsp->media) / sizeof(rtsp->media[0])) {
        printf("[david debug] rtsp_client_setup media r is err.\n");
        return r < 0 ? r : -E2BIG; // too many media stream
    }

	rtsp->media_count = r;
    //printf("[david debug] rtsp_client_setup media_count:%d.\n", rtsp->media_count);
	//wyl add for content-base have aggregate control,session_uri is aggregate control is sdp front
	if(!rtsp->media[0].session_uri[0])
	{
		if(rtsp->baseuri[0])
		{
			for (i = 0; i < rtsp->media_count; i++)
				snprintf(rtsp->media[i].session_uri, sizeof(rtsp->media[i].session_uri), "%s", rtsp->baseuri);
		}
	}	
	//add end
	
	for (i = 0; i < rtsp->media_count; i++)
	{
		// rfc 2326 C.1.1 Control URL (p80)
		// If found at the session level, the attribute indicates the URL for aggregate control
		rtsp->aggregate = rtsp->media[0].session_uri[0] ? 1 : 0;
		rtsp_media_set_url(rtsp->media+i, rtsp->baseuri, rtsp->location, rtsp->uri);
		if(rtsp->aggregate)
			snprintf(rtsp->aggregate_uri, sizeof(rtsp->aggregate_uri), "%s", rtsp->media[i].session_uri);

		t = rtsp->transport + i;
		r = rtsp->handler.rtpport(rtsp->param, i, &t->rtp.u.client_port1);
		if (0 != r) {
            printf("[david debug] rtsp_client_setup rtpport r is err.\n");
            return r;
        }
			
		if (0 == t->rtp.u.client_port1)
		{
			t->transport = RTSP_TRANSPORT_RTP_TCP;
			t->rtp.u.client_port1 = 2 * (unsigned short)i;
			t->rtp.u.client_port2 = 2 * (unsigned short)i + 1;
		}
		else
		{
			assert(0 == t->rtp.u.client_port1 % 2);
			t->transport = RTSP_TRANSPORT_RTP_UDP;
			t->rtp.u.client_port2 = t->rtp.u.client_port1 + 1;
		}
	}

	rtsp->state = RTSP_SETUP;
	rtsp->progress = 0;
	return rtsp_client_media_setup(rtsp, rtsp->progress);
}

static void rtsp_client_setup_get_timeout(const char *session, uint64_t *timeout)
{
    int value = -1;
    char *p = NULL;

    /*
    Session: 5bad729a5bad729a97ad729a67ad729;timeout=60
    */
    p = strstr(session, "timeout=");
    if (p) {
        value = atoi(p + 8);
        if (value > 5) {
            *timeout = (value - 2) * 1000;
        } else if (*timeout > 0) {
            *timeout = 1000;
        }
    }

    //printf("session:%s p:%p value:%d timeout:%lld\n", session, p, value, *timeout);
}

int rtsp_client_setup_onreply(struct rtsp_client_t* rtsp, void* parser, uint64_t *timeout)
{
	int code;
	const char *session;
	const char *transport;

	assert(RTSP_SETUP == rtsp->state);
	assert(rtsp->progress < rtsp->media_count);

	code = http_get_status_code(parser);
	if (200 == code)
	{
		session = http_get_header_by_name(parser, "Session");
		transport = http_get_header_by_name(parser, "Transport");
		if (!session || 0 != rtsp_header_session(session, &rtsp->session[rtsp->progress])
			|| !transport || 0 != rtsp_header_transport(transport, &rtsp->transport[rtsp->progress]))
		{
			printf("Get rtsp transport error.\n");
			return -EINVAL;
		}

		//assert(rtsp->media[rtsp->progress].transport.transport != RTSP_TRANSPORT_RTP_TCP || (rtsp->media[rtsp->progress].transport.rtp.u.client_port1== rtsp->media[rtsp->progress].transport.interleaved1 && rtsp->media[rtsp->progress].transport.rtp.u.client_port2 == rtsp->media[rtsp->progress].transport.interleaved2));
		assert(strlen(session) < sizeof(rtsp->session[0].session));
		assert(!rtsp->aggregate || 0 == strcmp(rtsp->session[0].session, rtsp->session[rtsp->progress].session));
        
		//add by hrz for real keepalive interval
		rtsp_client_setup_get_timeout(session, timeout);

		if (rtsp->media_count == ++rtsp->progress)
		{
			return rtsp->handler.onsetup(rtsp->param);
		}
		else
		{
			// setup next media
			return rtsp_client_media_setup(rtsp, rtsp->progress);
		}
	}
	else if (401 == code)
	{
		// Unauthorized
		const char* authenticate;
		authenticate = http_get_header_by_name(parser, "WWW-Authenticate");
		if (authenticate)
		{
			rtsp_client_www_authenticate(rtsp, authenticate);
		}
		return -EACCES; // try again
	}
	else if (461 == code)
	{
		// Unsupported Transport
		return -1;
	}
	else
	{
		return -1;
	}
}
