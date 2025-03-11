#ifndef _RECO_RTSP_CLIENT_H_
#define _RECO_RTSP_CLIENT_H_

// create ms_rtsp_client.h for struct

#include <stdio.h>
#include "ms_rtsp_client_stream.h"
#include "sys/sock.h"
#include "../src/ms_res_decoder.h"
#include "../src/ms_rtp_session.h"
#include "sys/thread.h"
#include "rtp-packet.h"
#include <openssl/ssl.h>

#define MAX_MEDIA_NUM	5
#define TEST_URL "192.168.80.190:554"
#define MAX_SESSION_LENGTH	24

typedef struct ms_rtsp_client_s
{
    char tname[16];
	void* param;
	void* rtsp_client;
	socket_t socket;

    int dosleep;
	int transport;
	int transmit_protocol;  //for configue.
	socket_t rtp_socket[MAX_MEDIA_NUM][2];
	unsigned short rtp_port[MAX_MEDIA_NUM][2];
    void* rtp_ctx[MAX_MEDIA_NUM];

	pthread_t *rtsp_pool_thread;
	int rtsp_pool_thread_run;
	char url[256];

	MS_RTSP_CLIENT_DataCallback data_callback;
	MS_RTSP_CLIENT_EventCallback event_callback;

	//for rtsp keep alive
	uint64_t last_cmd_time;
	uint8_t frame_recv_flag;

	struct Ms_StreamInfo stream_info;

	int start_push_data;
	struct ms_decoder_context_t decoder_ctx;
	struct ms_decoder_extra_t extradata;
    struct ms_decoder_extra_t rtp_extradata;

	//for rtsp over https
	char x_session[128];
	char respath[256];// /main /sub
	socket_t rtsp_command_socket;
	SSL *data_ssl;
	SSL *command_ssl;
}ms_rtsp_client_t;

typedef struct rtsp_url_s
{
	char url[256];
	char username[64];
	char password[64];
	char host[64];
	int rtsp_port;
}rtsp_url_t;

typedef struct ms_rtp_session_context_s
{
	int rtp_session_type; //0 is rtp over udp, 1 is rtp over tcp
	void *param;
	//int keepalive;//maybe used for keepalive

	MS_StreamCodecType codec_type;
	//for rtp over udp:[0] for rtp [1] for rtcp
	u_short port[2];
	socket_t socket[2];
	struct sockaddr_storage ss;
	socklen_t len;

	//for rtp over udp buffer receive
	char data_buffer[10 * 1024];
	//thread
	int rtp_recieve_thread_run;
	pthread_t rtp_recieve_thread;
		
	//for rtp decoder
	void* payload;
	void* rtp;
	struct rtp_packet_t rtp_packet;

	//frame packet call back
	frame_packet_callback callback;
}ms_rtp_session_context_t;

#endif
