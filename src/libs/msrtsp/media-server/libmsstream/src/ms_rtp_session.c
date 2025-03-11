#include "ms_rtp_session.h"
#include <stdint.h>
#include "sockutil.h"
#include "sys/pollfd.h"
#include "sys/thread.h"
#include "rtp-profile.h"
#include "rtp-payload.h"
#include "rtp.h"
#include "time64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rtp-packet.h"
#include "ms_rtsp_client.h"

#if defined(OS_WINDOWS)
#define strcasecmp _stricmp
#endif

static int rtp_read(ms_rtp_session_context_t* ctx, socket_t s)
{
	int r;
	socklen_t len;
	struct sockaddr_storage ss;
	len = sizeof(ss);

	r = recvfrom(s, ctx->data_buffer, sizeof(ctx->data_buffer), 0, (struct sockaddr*)&ss, &len);
	if(r < 0)
	{	
		if((socket_geterror() == EINTR || socket_geterror() ==  EAGAIN))
			return 0;
	}
	else if (r < 12)
		return -1;

	//for confrim data del
	if(((struct sockaddr_in*)&ss)->sin_port != htons(ctx->port[0]))
	{
		return -1;
	}
	if(0 != memcmp(&((struct sockaddr_in*)&ss)->sin_addr, &((struct sockaddr_in*)&ctx->ss)->sin_addr, 4))
	{
		return -1;
	}	
	//assert(AF_INET == ss.ss_family);
	//assert(((struct sockaddr_in*)&ss)->sin_port == htons(ctx->port[0]));
	//assert(0 == memcmp(&((struct sockaddr_in*)&ss)->sin_addr, &((struct sockaddr_in*)&ctx->ss)->sin_addr, 4));

	rtp_payload_decode_input(ctx->payload, ctx->data_buffer, r);
	rtp_onreceived_rtppacket(ctx->rtp, &ctx->rtp_packet);
	return r;
}

static int rtcp_read(ms_rtp_session_context_t* ctx, socket_t s)
{
	int r;
	socklen_t len;
	struct sockaddr_storage ss;
	len = sizeof(ss);
	r = recvfrom(s, ctx->data_buffer, sizeof(ctx->data_buffer), 0, (struct sockaddr*)&ss, &len);
	if (r < 12)
		return -1;
	//for confrim data del
	if(((struct sockaddr_in*)&ss)->sin_port != htons(ctx->port[1]))
	{
		return -1;
	}
	if(0 != memcmp(&((struct sockaddr_in*)&ss)->sin_addr, &((struct sockaddr_in*)&ctx->ss)->sin_addr, 4))
	{
		return -1;
	}
	//assert(AF_INET == ss.ss_family);
	//assert(((struct sockaddr_in*)&ss)->sin_port == htons(ctx->port[1]));
	//assert(0 == memcmp(&((struct sockaddr_in*)&ss)->sin_addr, &((struct sockaddr_in*)&ctx->ss)->sin_addr, 4));

	r = rtp_onreceived_rtcp(ctx->rtp, ctx->data_buffer, r);
	return r;
}

#if 0
static int rtp_receiver(ms_rtp_session_context_t* ctx, socket_t rtp[2], int timeout)
{
	int i, r;
	int interval;
	time64_t clock;
	struct pollfd fds[2];

	for (i = 0; i < 2; i++)
	{
		fds[i].fd = rtp[i];
		fds[i].events = POLLIN;
		fds[i].revents = 0;
	}

	clock = time64_now();
	while (ctx->rtp_recieve_thread_run)
	{
		// RTCP report
		interval = rtp_rtcp_interval(ctx->rtp);
		if (clock + interval < time64_now())
		{
			r = rtp_rtcp_report(ctx->rtp, ctx->data_buffer, sizeof(ctx->data_buffer));
			r = socket_sendto_by_time(rtp[1], ctx->data_buffer, r, 0, (struct sockaddr*)&ctx->ss, ctx->len, timeout);
			clock = time64_now();
		}

		r = poll(fds, 2, timeout);
		while (-1 == r && EINTR == errno)
			r = poll(fds, 2, timeout);

		if (0 == r)
		{
			continue; // timeout
		}
		else if (r < 0)
		{
			return r; // error
		}
		else
		{
			if (0 != fds[0].revents)
			{
				rtp_read(ctx, rtp[0]);
				fds[0].revents = 0;
			}

			if (0 != fds[1].revents)
			{
				rtcp_read(ctx, rtp[1]);
				fds[1].revents = 0;
			}
		}
	}
	return r;
}
#else//maylong modify for reduce cpu
static int rtp_receiver(ms_rtp_session_context_t* ctx, socket_t rtp[2], int timeout)
{
    int i = 0, r = 0;
	//int interval;
	time64_t clock;
	clock = time64_now();
	while (ctx->rtp_recieve_thread_run)
	{
		// RTCP report
		//interval = rtp_rtcp_interval(ctx->rtp);
		if (i++ % 2 == 1 && clock + 5000 < time64_now())
		{
			r = rtp_rtcp_report(ctx->rtp, ctx->data_buffer, sizeof(ctx->data_buffer));
			r = socket_sendto_by_time(rtp[1], ctx->data_buffer, r, 0, (struct sockaddr*)&ctx->ss, ctx->len, timeout);
			clock = time64_now();
			i = 0;
		}

		if(rtp_read(ctx, rtp[0]) == 0)
		{
			//time64_t clock_now = time64_now();
			usleep(20000);
			//time64_t clock_last = time64_now();
			//printf("=time=diff======%lld\n", clock_last- clock_now);
		}
		rtcp_read(ctx, rtp[1]);
	}
	return r;
}

#endif

static void rtp_packet(void* param, const void *packet, int bytes, uint32_t timestamp, int flags)
{
	ms_rtp_session_context_t* ctx;
	struct Ms_StreamPacket frame_packet = {0};
	ctx = (ms_rtp_session_context_t*)param;
#if 0
	if (0 == strcmp("H264", ctx->encoding) || 0 == strcmp("H265", ctx->encoding))
	{
        //memcpy(data, start_code, 4);
        //size += 4;
	}
	else if (0 == strcasecmp("mpeg4-generic", ctx->encoding))
	{
		uint8_t adts[7];
		int len = bytes + 7;
		uint8_t profile = 2;
		uint8_t sampling_frequency_index = 4;
		uint8_t channel_configuration = 2;
		adts[0] = 0xFF; /* 12-syncword */
		adts[1] = 0xF0 /* 12-syncword */ | (0 << 3)/*1-ID*/ | (0x00 << 2) /*2-layer*/ | 0x01 /*1-protection_absent*/;
		adts[2] = ((profile - 1) << 6) | ((sampling_frequency_index & 0x0F) << 2) | ((channel_configuration >> 2) & 0x01);
		adts[3] = ((channel_configuration & 0x03) << 6) | ((len >> 11) & 0x03); /*0-original_copy*/ /*0-home*/ /*0-copyright_identification_bit*/ /*0-copyright_identification_start*/
		adts[4] = (uint8_t)(len >> 3);
		adts[5] = ((len & 0x07) << 5) | 0x1F;
		adts[6] = 0xFC | ((len / 1024) & 0x03);

	}
	else if (0 == strcmp("MP4A-LATM", ctx->encoding))
	{
		// add ADTS header
	}
#endif

//	(void)time;
	frame_packet.i_flags = (flags & RTP_PAYLOAD_FLAG_PACKET_IDR)? 1:0;
	frame_packet.i_pts = timestamp;
	frame_packet.data_buffer = (uint8_t *)packet;
	frame_packet.data_size = bytes;
	if(flags & RTP_PAYLOAD_FLAG_PACKET_MS_METADATA)
	{
		frame_packet.i_codec = MS_StreamCodecType_MS_METADATA;
	}
	else
		frame_packet.i_codec = ctx->codec_type;
/*
	if(frame_packet.data_size <= 0)
	{
		//david add for size is err! 2021.02.08
		//printf("[rtp_packet] The frame packet size is 0 !!!\n");
		return ;
	}
*/
	if(ctx->callback)
	{
		ctx->callback(ctx->param, &frame_packet);
	}
}

static void get_rtp_packet_ptr(void* param,void **packet)
{
	ms_rtp_session_context_t* ctx = (ms_rtp_session_context_t*)param;
	*packet = &ctx->rtp_packet;
//	memcpy(&ctx->rtp_packet, packet, sizeof(ctx->rtp_packet));
}

static void rtp_on_rtcp(void* param, const struct rtcp_msg_t* msg)
{
	//ms_rtp_session_context_t* ctx = (ms_rtp_session_context_t*)param;
	if (RTCP_MSG_BYE == msg->type)
	{
		printf("finished\n");
	}
}

static int STDCALL rtp_worker(void* param)
{
	char thread_name[32] = {0};
	ms_rtp_session_context_t* ctx = (ms_rtp_session_context_t*)param;
	thread_get_name(thread_name, sizeof(thread_name));
	if(strlen(thread_name) > 1)
	{
		switch (ctx->codec_type)
		{
			case MS_StreamCodecType_H264:
			case MS_StreamCodecType_H265:
			case MS_StreamCodecType_MJPEG:
                snprintf(thread_name + strlen(thread_name), sizeof(thread_name) - strlen(thread_name), "%s", "_vid");
				break;
			case MS_StreamCodecType_G711_ULAW:	  
			case MS_StreamCodecType_G711_ALAW:
			case MS_StreamCodecType_AAC:
			case MS_StreamCodecType_G722:
			case MS_StreamCodecType_G726:
            snprintf(thread_name + strlen(thread_name), sizeof(thread_name) - strlen(thread_name), "%s", "_aud");
				break;
			case MS_StreamCodecType_ONVIF_METADATA:
                snprintf(thread_name + strlen(thread_name), sizeof(thread_name) - strlen(thread_name), "%s", "_mda");
				break;
			default:
                snprintf(thread_name + strlen(thread_name), sizeof(thread_name) - strlen(thread_name), "%s", "_nul");
				break;			
		}
	}
	thread_set_name(thread_name);

	rtp_receiver(ctx, ctx->socket, 2000);

	return 0;
}

// rtp over udp
void* ms_rtp_receiver_create(socket_t rtp[2], const char* peer, int peerport[2], int payload, const char* encoding, void *param)
{
	size_t n;
	ms_rtp_session_context_t* ctx;
	struct rtp_event_t evthandler;
	struct rtp_payload_t handler;
	const struct rtp_profile_t* profile;

	ctx = malloc(sizeof(*ctx));
	if(!ctx)
		return NULL;
	memset(ctx, 0, sizeof(*ctx));
    ctx->param = param;
	ctx->rtp_session_type = 0;
	parse_codec_type(encoding, &ctx->codec_type);

	profile = rtp_profile_find(payload);
	handler.alloc = NULL;
	handler.free = NULL;
	handler.packet = rtp_packet;
	handler.get_rtp_packet_ptr = get_rtp_packet_ptr;
	ctx->payload = rtp_payload_decode_create(payload, encoding, &handler, ctx);
	if (NULL == ctx->payload)
	{
	    ms_rtp_session_destroy(ctx);
		return NULL; // ignore
	}
	
	evthandler.on_rtcp = rtp_on_rtcp;
	ctx->rtp = rtp_create(&evthandler, &ctx, (uint32_t)(intptr_t)&ctx, (uint32_t)(intptr_t)&ctx, profile ? profile->frequency : 9000, 2*1024*1024, 0);
	if (NULL == ctx->rtp)
	{
		ms_rtp_session_destroy(ctx);
		return NULL; // ignore
	}
	//assert(0 == socket_addr_from(&ctx->ss, &ctx->len, peer, (u_short)peerport[0]));
	if(socket_addr_from(&ctx->ss, &ctx->len, peer, (u_short)peerport[0]))
	{
		printf("socket_addr_from failed:%s,%d\n", peer, peerport[0]);
	}
	//socket_addr_setport((struct sockaddr*)&ctx->ss, ctx->len, (u_short)peerport[1]);
	//assert(0 == socket_addr_setport((struct sockaddr*)&ctx->ss, ctx->len, (u_short)peerport[1]));
	socket_setnonblock(rtp[0], 1);
	socket_setnonblock(rtp[1], 1);	
	socket_getrecvbuf(rtp[0], &n);
	if(ctx->codec_type == MS_StreamCodecType_H264 || ctx->codec_type == MS_StreamCodecType_H265)
		socket_setrecvbuf(rtp[0], 1024*1024);
	else
		socket_setrecvbuf(rtp[0], 1024*1024);
	socket_getrecvbuf(rtp[0], &n);	
	ctx->socket[0] = rtp[0];
	ctx->socket[1] = rtp[1];
	ctx->port[0] = (u_short)peerport[0];
	ctx->port[1] = (u_short)peerport[1];
	ctx->rtp_recieve_thread_run = 1;
	if (thread_create2(&ctx->rtp_recieve_thread, 32*1024, rtp_worker, ctx))
	{
		ctx->rtp_recieve_thread_run = 0;
		ms_rtp_session_destroy(ctx);
		return NULL;
	}	
	return ctx;
}

void* ms_rtp_receiver_tcp_create(int payload, const char* encoding, void *param)//ms_rtsp_client_t
{
	ms_rtp_session_context_t* ctx;
	struct rtp_event_t evthandler;
	struct rtp_payload_t handler;
	const struct rtp_profile_t* profile;

	ctx = malloc(sizeof(*ctx));
	if(!ctx)
		return NULL;	
	memset(ctx, 0, sizeof(*ctx));
    ctx->param = param;
	ctx->rtp_session_type = 1;
	parse_codec_type(encoding, &ctx->codec_type);

	profile = rtp_profile_find(payload);
	handler.alloc = NULL;
	handler.free = NULL;
	handler.packet = rtp_packet;
	handler.get_rtp_packet_ptr = get_rtp_packet_ptr;
	ctx->payload = rtp_payload_decode_create(payload, encoding, &handler, ctx);
	if (NULL == ctx->payload)
	{
		ms_rtp_session_destroy(ctx);
		return NULL;
	}	
	evthandler.on_rtcp = rtp_on_rtcp;
	ctx->rtp = rtp_create(&evthandler, &ctx, (uint32_t)(intptr_t)&ctx, (uint32_t)(intptr_t)&ctx, profile ? profile->frequency : 9000, 2 * 1024 * 1024, 0);
	if (NULL == ctx->rtp)
	{
		ms_rtp_session_destroy(ctx);
		return NULL;
	}
	return ctx;
}

void ms_rtp_receiver_tcp_input(void *s_ctx, uint8_t channel, const void* data, uint16_t bytes)
{
	ms_rtp_session_context_t* ctx = (ms_rtp_session_context_t *)s_ctx;

	if (0 == channel % 2)
	{
		if (ctx->payload && ctx->rtp)
		{
			rtp_payload_decode_input(ctx->payload, data, bytes);
			rtp_onreceived_rtppacket(ctx->rtp, &ctx->rtp_packet);
		}
	}
	else
	{
		if (ctx->rtp)
			rtp_onreceived_rtcp(ctx->rtp, data, bytes);
	}
}

void ms_rtp_session_set_callback(void *s_ctx, frame_packet_callback callback)
{
	ms_rtp_session_context_t* ctx = (ms_rtp_session_context_t*)s_ctx;
	if(!ctx)
		return;
	ctx->callback = callback;
}

void ms_rtp_session_destroy(void *s_ctx)
{
	ms_rtp_session_context_t* ctx = (ms_rtp_session_context_t*)s_ctx;
	if(!ctx)
		return;
	if(!ctx->rtp_session_type)
	{
		if(ctx->rtp_recieve_thread_run == 1)
		{
			ctx->rtp_recieve_thread_run = 0;
			thread_destroy(ctx->rtp_recieve_thread);
		}
	}
	if(ctx->rtp)
	{
		rtp_destroy(ctx->rtp);
		ctx->rtp = NULL;
	}
	if(ctx->payload)
	{
		rtp_payload_decode_destroy(ctx->payload);
	}
	free(ctx);
	return;
}

