#include "rtsp-client.h"
#include "rtsp-client-internal.h"
#include "rtp-profile.h"
#include "sdp.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#if 0
void ms_client_time_to_string(char *stime, int nLen)
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

#define ms_rtp_client_print(format, ...) \
    do{\
        char stime[32] = {0};\
        ms_client_time_to_string(stime, sizeof(stime));\
        printf("[%s %d==func:%s file:%s line:%d]==" format "\n", \
               stime, getpid(), __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
    }while(0)
#else
#define ms_rtp_client_print(format, ...)
#endif

struct rtsp_client_t* rtsp_client_create(const char* uri, const char* usr, const char* pwd, const struct rtsp_client_handler_t *handler, void* param)
{
	struct rtsp_client_t *rtsp;
	rtsp = (struct rtsp_client_t*)calloc(1, sizeof(*rtsp));
	if(NULL == rtsp)
		return NULL;

	snprintf(rtsp->uri, sizeof(rtsp->uri) - 1, "%s", uri);
	snprintf(rtsp->usr, sizeof(rtsp->usr) - 1, "%s", usr ? usr : "");
	snprintf(rtsp->pwd, sizeof(rtsp->pwd) - 1, "%s", pwd ? pwd : "");
	
	rtsp->parser = http_parser_create(HTTP_PARSER_CLIENT);
	memcpy(&rtsp->handler, handler, sizeof(rtsp->handler));
	rtsp->rtp.onrtp = rtsp->handler.onrtp;
	rtsp->rtp.param = param;
	rtsp->state = RTSP_INIT;
	rtsp->param = param;
	rtsp->cseq = 1;
	rtsp->auth_failed = 0;

	return rtsp;
}

void rtsp_client_destroy(struct rtsp_client_t *rtsp)
{
	if (rtsp->parser)
	{
		http_parser_destroy(rtsp->parser);
		rtsp->parser = NULL;
	}

	if (rtsp->rtp.data)
	{
		assert(rtsp->rtp.capacity > 0);
		free(rtsp->rtp.data);
		rtsp->rtp.data = NULL;
		rtsp->rtp.capacity = 0;
	}

	free(rtsp);
}

static int rtsp_client_handle(struct rtsp_client_t* rtsp, http_parser_t* parser, uint64_t *timeout)
{
	switch (rtsp->state)
	{
	case RTSP_ANNOUNCE:
        {
            ms_rtp_client_print("[david debug] RTSP_ANNOUNCE\n");
            return rtsp_client_announce_onreply(rtsp, parser);
        }   
	case RTSP_DESCRIBE: 
        {
            ms_rtp_client_print("[david debug] RTSP_DESCRIBE\n");
            return rtsp_client_describe_onreply(rtsp, parser);
        }
	case RTSP_SETUP:
        {
            ms_rtp_client_print("[david debug] RTSP_SETUP\n");
            return rtsp_client_setup_onreply(rtsp, parser, timeout);
        }
	case RTSP_PLAY:
        {
            ms_rtp_client_print("[david debug] RTSP_PLAY\n");
            return rtsp_client_play_onreply(rtsp, parser);
        }      
	case RTSP_PAUSE:
        {
            ms_rtp_client_print("[david debug] RTSP_PAUSE\n");
            return rtsp_client_pause_onreply(rtsp, parser);
        }
	case RTSP_TEARDWON: 
        {
            ms_rtp_client_print("[david debug] RTSP_TEARDWON\n");
            return rtsp_client_teardown_onreply(rtsp, parser);
        }
	case RTSP_OPTIONS:	
        {
            ms_rtp_client_print("[david debug] RTSP_OPTIONS\n");
            return rtsp_client_options_onreply(rtsp, parser);
        }
	case RTSP_GET_PARAMETER: 
        {
            ms_rtp_client_print("[david debug] RTSP_GET_PARAMETER\n");
            return rtsp_client_get_parameter_onreply(rtsp, parser);
        }
	case RTSP_SET_PARAMETER: 
        {
            ms_rtp_client_print("[david debug] RTSP_SET_PARAMETER\n");
            return rtsp_client_set_parameter_onreply(rtsp, parser);
        }
	default: 
	    {
	        ms_rtp_client_print("[david debug] rtsp_client_handle default err.\n");
            assert(0);
            return -1;
        } 
	}
}

static inline char *rtsp_client_get_next_rcp(const char *p, const char *end)
{
    char *h = NULL;
    
    for (; p < end; p++) {
        if (*p == '$') {
            h = p;
            break;
        }
    }

    return h;
}

int rtsp_client_input(struct rtsp_client_t *rtsp, const void* data, size_t bytes, int *dosleep, uint64_t *timeout)
{
	int r, ndebugCnt = 0;
	size_t remain;
	const uint8_t* p, *end, *tmp;
    //ms_rtsp_client_t * 
	r = 0;
	p = (const uint8_t*)data;
	end = p + bytes;

    //printf("[david debug] rtsp_client_input bytes:%d state:%d\n", bytes, rtsp->state);
    //printf("[david debug] rtsp_client_input parser_need_more_data:%d p:%c\n", rtsp->parser_need_more_data, *p);
	do
	{
	    if (*dosleep) {
            //printf("###***###rtsp_client_input dosleep(%d)###999###\n", *dosleep);
            break;
        }
		if (0 == rtsp->parser_need_more_data && (*p == '$' || 0 != rtsp->rtp.state))
		{
			//printf("recv:channel:%d,bytes:%d,total length:%d\n", p[1], (int)(p[2]<<8|p[3]), bytes);
			p = rtp_over_rtsp(&rtsp->rtp, p, end, dosleep);
		}
		else
		{			
			remain = (size_t)(end - p);
			r = http_parser_input(rtsp->parser, p, &remain);
			if (r < 0) {
				rtsp->parser_need_more_data = 0;
				http_parser_clear(rtsp->parser);
				tmp = rtsp_client_get_next_rcp(p, end);
				if (tmp) {
					p = tmp;
					rtsp->rtp.state = 0;
					r = 0;
				} else {
					p = end - remain;
				}
			} else {
				rtsp->parser_need_more_data = r;
				//assert(r <= 2); // 1-need more data
				if (0 == r)
				{
					r = rtsp_client_handle(rtsp, rtsp->parser, timeout);
					http_parser_clear(rtsp->parser); // reset parser
					//assert((size_t)remain < bytes);
				}
				p = end - remain;
			}
		}
        ndebugCnt++;
	} while (p < end && r >= 0);

    //printf("###***###rtsp_client_input ndebugCnt(%d) r:%d ###***###\n", ndebugCnt, r);
	//assert(r <= 1);
	return r >= 0 ? 0 : r;
}

const char* rtsp_client_get_header(struct rtsp_client_t* rtsp, const char* name)
{
	return http_get_header_by_name(rtsp->parser, name);
}

int rtsp_client_media_count(struct rtsp_client_t *rtsp)
{
	return rtsp->media_count;
}

const struct rtsp_header_transport_t* rtsp_client_get_media_transport(struct rtsp_client_t *rtsp, int media)
{
	if(media < 0 || media >= rtsp->media_count)
		return NULL;
	return rtsp->transport + media;
}

const char* rtsp_client_get_media_encoding(struct rtsp_client_t *rtsp, int media)
{
	if (media < 0 || media >= rtsp->media_count)
		return NULL;
	return rtsp->media[media].avformats[0].encoding;
}

const char* rtsp_client_get_media_fmtp(struct rtsp_client_t *rtsp, int media)
{
	if (media < 0 || media >= rtsp->media_count)
		return NULL;
	return rtsp->media[media].avformats[0].fmtp;
}

int rtsp_client_get_media_payload(struct rtsp_client_t *rtsp, int media)
{
	if (media < 0 || media >= rtsp->media_count)
		return -1;
	return rtsp->media[media].avformats[0].fmt;
}

int rtsp_client_get_media_rate(struct rtsp_client_t *rtsp, int media)
{
	int rate;
	if (media < 0 || media >= rtsp->media_count)
		return -1;
	
	rate = rtsp->media[media].avformats[0].rate;
	if (0 == rate)
	{
		const struct rtp_profile_t* profile;
		profile = rtp_profile_find(rtsp->media[media].avformats[0].fmt);
		rate = profile ? profile->frequency : 0;
	}
	return rate;
}

const char* rtsp_client_get_media_describption(struct rtsp_client_t *rtsp, int media)
{
	if (media < 0 || media >= rtsp->media_count)
		return NULL;
	return rtsp->media[media].media;
}

int rtsp_client_get_media_extradata(struct rtsp_client_t *rtsp, int media, char *dst, int dstlen)
{
	if (media < 0 || media >= rtsp->media_count)
		return -1;
	
	snprintf(dst, dstlen, "%s", rtsp->media[media].avformats[0].extradata);
	return 0;
}

uint32_t rtsp_client_get_methodmask(struct rtsp_client_t *rtsp)
{
    if (!rtsp) {
        return 0;
    }

    return rtsp->methodMask;
}
