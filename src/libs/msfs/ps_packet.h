/* 
 * ***************************************************************
 * Filename:      	ps_packet.h
 * Created at:    	2017.12.27
 * Description:   	ps stream <-> h264/h265.
 * Author:        	zbing
 * Copyright (C)  	milesight
 * ***************************************************************
 */
 
#ifndef __PS_PACKET_H__
#define __PS_PACKET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "msstd.h"
#include "mf_type.h"
#include "MFrecord.h"

#define PS_HDR_LEN	20
#define SYS_HDR_LEN 18
#define PSM_HDR_LEN 30	
#define PES_HDR_LEN 14 	
#define PS_PES_PAYLOAD_SIZE (65535 - 8)

typedef void * (*PS_ALLOC)(MF_U32 size);
typedef void (*PS_FREE)(void *p);

typedef enum{
	PS_ERR_BAD_FRAME = -2,
	PS_ERR_OVERFLOW = -3,
}PS_ERR_E;

typedef enum{	
	PS_VIDEO_STREAM = 0xe0,
	PS_AUDIO_STREAM = 0xc0,
	PS_PRIVATE_STREAM = 0xbd,
	PS_PRIVATE_STREAM_EXT = 0xbf,
}PS_STREAM_TYPE_E;

typedef enum{	
	PS_CODEC_H264 = CODECTYPE_H264,
	PS_CODEC_H265 = CODECTYPE_H265,
}PS_CODEC_TYPE;

typedef enum{	
	VIDEO_TYPE = ST_VIDEO,
	AUDIO_TYPE = ST_AUDIO,
	ST_TYPE = ST_DATA,
}STREAM_TYPE_EN;
	
typedef enum{	
	PS_OTHER_FRAME = FT_PFRAME,
	PS_IDR_FRAME = FT_IFRAME,
}FRAME_TYPE;	

typedef enum{	
	H264_NALU_IDR_FRAME = 0x05,
	H264_NALU_OTHER_FRAME =0x01,
	H264_NALU_PPS = 0x08,
	H264_NALU_SPS = 0x07,
	H264_NALU_SEI = 0x06, 
} H264_NALU_TYPE_E;
	
typedef enum{	
	H265_NALU_IDR_FRAME = 19,
	H265_NALU_OTHER_FRAME =1,
	H265_NALU_PPS = 34,
	H265_NALU_SPS = 33,
	H265_NALU_VPS = 32,
	H265_NALU_SEI = 39, 
}H265_NALU_TYPE_E;

typedef  struct bits_buffer_s{
	MF_U32  i_data;
	MF_U32 i_size;
	MF_U32 i_mask;
	MF_U8 * p_data;
}bits_buffer_t;

typedef struct nalu_para_s{
	MF_S32  type;
	MF_S32  count;
} nalu_para_t;

typedef struct frame_para_s{
	nalu_para_t nalu_para[32];
	MF_S32  nalu_cnt;
	MF_S32  stream_type;
	MF_S32  codec_type;
}frame_para_t;

typedef struct ps_frame_s{
	//data buff
	MF_S32  frame_size;	
	MF_S32  packet_size;
	void *data;
}ps_frame_t;

void
ps_packet_cb(PS_ALLOC ps_alloc, PS_FREE ps_free);
MF_S32
ps_packet_encoder(struct mf_frame* frame_info,struct ps_frame_s *ps_info);
MF_S32
ps_packet_decoder(struct ps_frame_s *packet_info, struct mf_frame *frame_info);
MF_S32
ps_packet_get_info(MF_S8 *data,  MF_U32 size, struct ps_frame_s *frame_info);
MF_S32
ps_packet_encoder_free(struct ps_frame_s *ps_info);

MF_S32
ps_packet_custom(MF_S8 *data, MF_S32 raw_length, struct ps_frame_s *ps_info);

MF_S32
ps_packet_get_header(void *data, MF_S32 length, struct mf_frame *frame);



#ifdef __cplusplus
}
#endif

#endif

