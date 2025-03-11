#ifndef _____MS_RTP_SESSION_H__
#define _____MS_RTP_SESSION_H__

#include "sockutil.h"

#include "ms_stream_common.h"

typedef void (*frame_packet_callback)(void* param, struct Ms_StreamPacket *packet);


void* ms_rtp_receiver_create(socket_t rtp[2], const char* peer, int peerport[2], int payload, const char* encoding, void *param);
void* ms_rtp_receiver_tcp_create(int payload, const char* encoding, void *param);

void ms_rtp_receiver_tcp_input(void *s_ctx, uint8_t channel, const void* data, uint16_t bytes);

void ms_rtp_session_set_callback(void *s_ctx, frame_packet_callback callback);

void ms_rtp_session_destroy(void *s_ctx);

#endif
