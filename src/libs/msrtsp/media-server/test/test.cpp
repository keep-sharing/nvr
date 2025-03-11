#include "sys/sock.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#if 0
extern "C" void amf0_test(void);
extern "C" void rtp_queue_test(void);
extern "C" void mpeg4_aac_test(void);
extern "C" void mpeg4_avc_test(void);
extern "C" void mpeg4_hevc_test(void);
extern "C" void mp3_header_test(void);
extern "C" void sdp_a_fmtp_test(void);
extern "C" void sdp_a_rtpmap_test(void);
extern "C" void rtsp_client_auth_test(void);
extern "C" void rtsp_header_range_test(void);
extern "C" void rtsp_header_rtp_info_test(void);
extern "C" void rtsp_header_transport_test(void);
extern "C" void http_header_host_test(void);
extern "C" void http_header_content_type_test(void);
extern "C" void http_header_authorization_test(void);
extern "C" void http_header_www_authenticate_test(void);
extern "C" void http_header_auth_test(void);

extern "C" void rtsp_example();
extern "C" void rtsp_push_server();
extern "C" void rtsp_client_test(const char* host, const char* file);
extern "C" void http_server_test(const char* ip, int port);
void rtp_payload_test();

void mpeg_ts_dec_test(const char* file);
void mpeg_ts_test(const char* input);
void mpeg_ps_test(const char* input);
void flv_2_mpeg_ps_test(const char* flv);
void mpeg_ps_dec_test(const char* file);

void flv_read_write_test(const char* flv);
void flv2ts_test(const char* inputFLV, const char* outputTS);
void ts2flv_test(const char* inputTS, const char* outputFLV);
void avc2flv_test(const char* inputH264, const char* outputFLV);
void hevc2flv_test(const char* inputH265, const char* outputFLV);
void flv_reader_test(const char* file);

void mov_2_flv_test(const char* mp4);
void mov_reader_test(const char* mp4);
void mov_writer_test(int w, int h, const char* inflv, const char* outmp4);
void fmp4_writer_test(int w, int h, const char* inflv, const char* outmp4);
void mov_writer_h264(const char* h264, int width, int height, const char* mp4);
void mov_writer_h265(const char* h265, int width, int height, const char* mp4);
void mov_writer_audio(const char* audio, int type, const char* mp4);

void hls_segmenter_flv(const char* file);
void hls_segmenter_fmp4_test(const char* file);
void hls_server_test(const char* ip, int port);
void dash_dynamic_test(const char* ip, int port, const char* file, int width, int height);
void dash_static_test(const char* mp4, const char* name);

void rtmp_play_test(const char* host, const char* app, const char* stream, const char* flv);
void rtmp_publish_test(const char* host, const char* app, const char* stream, const char* flv);
void rtmp_play_aio_test(const char* host, const char* app, const char* stream, const char* file);
void rtmp_publish_aio_test(const char* host, const char* app, const char* stream, const char* file);
void rtmp_server_vod_test(const char* flv);
void rtmp_server_publish_test(const char* flv);
void rtmp_server_vod_aio_test(const char* flv);
void rtmp_server_publish_aio_test(const char* flv);
void rtmp_server_forward_aio_test(const char* ip, int port);

extern "C" void sip_header_test(void);
extern "C" void sip_agent_test(void);
void sip_uac_message_test(void);
void sip_uas_message_test(void);
void sip_uac_test(void);
void sip_uas_test(void);
void sip_uac_test2(void);
void sip_uas_test2(void);

int binnary_diff(const char* file1, const char* file2);

int main(int argc, char* argv[])
{
	/*amf0_test();
	rtp_queue_test();
	mpeg4_aac_test();
	mpeg4_avc_test();
	mpeg4_hevc_test();
	mp3_header_test();
	sdp_a_fmtp_test();
	sdp_a_rtpmap_test();
	rtsp_header_range_test();
	rtsp_header_rtp_info_test();
	rtsp_header_transport_test();
	http_header_host_test();
	http_header_auth_test();
	http_header_content_type_test();
	http_header_authorization_test();
	http_header_www_authenticate_test();
	rtsp_client_auth_test();
	sip_header_test();
	sip_uac_message_test();
	sip_uas_message_test();*/
	
	socket_init();

	//mpeg_ts_dec_test("fileSequence0.ts");
	//mpeg_ts_test("hevc_aac.ts");
	//mpeg_ps_test("sjz.ps");
	
	//mov_2_flv_test("720p.mp4");
	//mov_reader_test("720p.mp4");
	//mov_writer_test(768, 432, "720p.mp4.flv", "720p.mp4.flv.mp4");
	//mov_writer_audio("720p.mp4", 1, "aac.mp4");
	//mov_writer_h264("720p.h264", 1280, 720, "720p.h264.mp4");
	//mov_writer_h265("720p.h265", 1280, 720, "720p.h265.mp4");
	//fmp4_writer_test(1280, 720, "720p.flv", "720p.frag.mp4");

	//flv_reader_test("720p.flv");
	//flv_read_write_test("720p.flv");
	//ts2flv_test("bipo.ts", "bipo.ts.flv");
	//flv2ts_test("bipo.ts.flv", "bipo.ts.flv.ts");
	//avc2flv_test("4k.h264", "out.flv");
	//hevc2flv_test("BigBuckBunny-3840x2160.h265", "out.flv");
	//flv_reader_test("out.flv");

	//hls_segmenter_flv("720p.flv");
#if defined(_HAVE_FFMPEG_)
	//hls_segmenter_fmp4_test("720p.mp4");
#endif
	//dash_dynamic_test(NULL, 80);
	//dash_static_test("720p.mp4", "name");
	//hls_server_test(NULL, 80);
	//http_server_test(NULL, 80);

	//rtsp_client_test("192.168.241.129", "test.rtp");
	//rtsp_example();
	//rtsp_push_server();

	//rtmp_play_test("192.168.241.129", "live", "hevc", "h265.flv");
	//rtmp_publish_test("192.168.241.129", "live", "avc", "h264.flv");
	//rtmp_play_aio_test("192.168.241.129", "live", "avc", "avc.flv");
	//rtmp_publish_aio_test("192.168.241.129", "live", "avc", "avc.flv");
	//rtmp_server_publish_test("h265.flv");
	//rtmp_server_vod_test("h264.flv");
	//rtmp_server_vod_aio_test("720p.flv");
	//rtmp_server_publish_aio_test("720p.flv");
	//rtmp_server_forward_aio_test(NULL, 1935);

	//sip_uac_test();
	//sip_uas_test();
	//sip_uas_test2();
	//sip_uac_test2();
	//sip_agent_test();

	socket_cleanup();
	return 0;
}
#else
#include "ms_rtsp_client_stream.h"

typedef unsigned long long time64_t;
time64_t time64_now(void)
{
	time64_t v;

#if defined(OS_WINDOWS)
	FILETIME ft;
	GetSystemTimeAsFileTime((FILETIME*)&ft);
	v = (((__int64)ft.dwHighDateTime << 32) | (__int64)ft.dwLowDateTime) / 10000; // to ms
	v -= 0xA9730B66800; /* 11644473600000LL */ // January 1, 1601 (UTC) -> January 1, 1970 (UTC).
#else
	struct timeval tv;	
	gettimeofday(&tv, NULL);
	v = tv.tv_sec;
	v *= 1000;
	v += tv.tv_usec / 1000;
#endif
	return v;
}

void data_callback(struct Ms_StreamPacket *packet, void* opaque)
{
	//if(packet->i_codec == MS_StreamCodecType_MS_METADATA)
	printf("poll,time:%lld, packet:size :%d, data:0x%02x,codec:%d,iframe:%d,timestamp:%llu\n", time64_now(), packet->data_size, packet->data_buffer[0], packet->i_codec, packet->i_flags, packet->i_pts);
	/*if(packet->i_flags)
	{
		int i = 0;
		for(i = 0; i < 200; i++)
		{
			if(i%5== 0)
			{
				printf("\n");	
			}
			printf("%02x,", packet->data_buffer[i]);
		}
	}*/
	return;
}

void event_data_callback(MS_StreamEventType event_type, struct Ms_StreamInfo *stream_info, void* opaque)
{
	//struct reco_rtsp_client* cli = (struct reco_rtsp_client*)(opaque);
	if(event_type == MS_StreamEventType_WAITTING)
	{
		printf("stream waiting\n");
	}
	else if(event_type == MS_StreamEventType_CONNECT || event_type == MS_StreamEventType_CODECCHANGE)
	{
		if(event_type == MS_StreamEventType_CONNECT)
		{
			printf("stream connect,video codec:%d,%d,%d,audio codec:%d,%d\n", stream_info->video_codec, stream_info->video_width
				, stream_info->video_height, stream_info->audio_codec, stream_info->audio_sample);
		}
		else
		{
			printf("stream resolution change,video codec:%d,%d,%d,audio codec:%d,%d\n", stream_info->video_codec, stream_info->video_width
				, stream_info->video_height, stream_info->audio_codec, stream_info->audio_sample);
		}
	}
	else
	{
		printf("stream disconnect\n");
	}
}


#include "sockutil.h"

int main(int argc, char* argv[])
{
	
	ms_stream_rtspclient_handler rtsp_client_handler = NULL;
	ms_stream_init();
	//ms_rtsp_set_server_debug(1);//需重编PUBLIC_STATIC_LIB_DIR？
	rtsp_client_handler = ms_rtsp_client_create("rtsp:/main", "rcli_00_m", "/main", MS_StreamTransportType_UDP, NULL);

	ms_rtsp_client_setDataCallback(rtsp_client_handler, data_callback);
	ms_rtsp_client_setEventCallback(rtsp_client_handler, event_data_callback);
	ms_rtsp_client_openstream(rtsp_client_handler);
	/*int i = 0;
	for(i = 0; i < 100; i++)
	{
		socket_t sock;
		size_t n;
		struct addrinfo hints, *addr, *ptr;
		
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET; // IPv4 only
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
		if (0 != getaddrinfo("127.0.0.1", 0, &hints, &addr))
			return socket_invalid;
		
		sock = socket_invalid;
		for (ptr = addr; socket_invalid == sock && ptr != NULL; ptr = ptr->ai_next)
		{
			assert(AF_INET == ptr->ai_family);
			
			// fixed ios getaddrinfo don't set port if nodename is ipv4 address
			//socket_addr_setport(ptr->ai_addr, ptr->ai_addrlen, port);
			
			sock = socket_udp_bind_addr(ptr->ai_addr, 0, 0);
		}
		
		freeaddrinfo(addr);
		//return sock;
		socket_setrecvbuf(sock, 1024*1024);
		socket_getrecvbuf(sock, &n);
		printf("recv buf %d\n", n);
	}*/
	while(1)
	{
		usleep(5000000);
	}
	ms_rtsp_client_destroy(rtsp_client_handler);
	ms_stream_uninit();
}
#endif
