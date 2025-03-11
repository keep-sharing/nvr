#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ms_parse_sps.h"
#include "ms_res_decoder.h"

static char *find_nalu(char *data, int size)
{
	int i;
	for(i = 0;i < size - 4;i ++){
		if(!data[i] && !data[i + 1] && !data[i + 2] && (data[i + 3] == 1))
			return data + i;
	}
	return NULL;
}

static char *find_hevc_nalu(char *data, int size)
{
	int i;
	for(i = 0; i < size-3; i++)
	{
		if(!data[i] && !data[i+1] && (data[i+2] == 1))
			return data+i;
	}
	return NULL;
}

static int ms_getreso_spsinfo(char *data, int data_size, struct ms_decoder_context_t *sps)
{
	memset(sps, 0x00, sizeof(struct ms_decoder_context_t));
	char *p2nalu = data;
	while(1)
	{
		p2nalu = find_nalu(p2nalu, data_size - (p2nalu - (char *)data));
		if(!p2nalu || (p2nalu[4] & 0x1f) == 7)
			break;
		p2nalu += 1;
	}

	if(!p2nalu) return -1;
	char *spsend = find_nalu(p2nalu + 1, data_size - (p2nalu - (char *)data) - 1);
	if(!spsend) return -1;

	int spslen = spsend - p2nalu;
	sps->size = spslen > sizeof(sps->data)? sizeof(sps->data):spslen;
	if(sps->size >= sizeof(sps->data))
	{
		printf("[david debug] sps size(%d) > %d is error!!!\n", sps->size, sizeof(sps->data));
		return -1;
	}
	memcpy(sps->data, p2nalu, sps->size);
	return 0;
}

//add by larry v.7.0.5
static int ms_getreso_hevc_spsinfo(char *data, int data_size, struct ms_decoder_context_t *sps)
{
	memset(sps, 0x00, sizeof(struct ms_decoder_context_t));
	char *p2nalu = data;
	while(1){
		p2nalu = find_hevc_nalu(p2nalu, data_size - (p2nalu - (char *)data));
		if(!p2nalu || (p2nalu[3] & 0x7E)>>1 == 33)
		{
			break;
		}
		p2nalu += 1;
	}

	if(!p2nalu) return -1;
	char *spsend = find_hevc_nalu(p2nalu + 1, data_size - (p2nalu - (char *)data) - 1);
	if(!spsend) return -1;

	int spslen = spsend - p2nalu;
	sps->size = spslen > sizeof(sps->data)? sizeof(sps->data):spslen;
	memcpy(sps->data, p2nalu, sps->size);
	return 0;
}

int ms_parse_h2645_res(int is_h265,unsigned char *frame_data, int frame_data_size, struct ms_decoder_context_t *decoder_ctx)
{	
	SPS sps_info = {0};
	if(is_h265)
	{
		if(ms_getreso_hevc_spsinfo((char *)frame_data, frame_data_size, decoder_ctx))
		{
			return 0;
		}			
		if(ms_hevcdec_seq_parameter_set(decoder_ctx->data, decoder_ctx->size, &sps_info))
		{
			return 0;
		}	
		decoder_ctx->width = sps_info.pic_width_in_mbs_minus1;
		decoder_ctx->height = sps_info.pic_height_in_map_units_minus1;
        if(sps_info.frame_cropping_flag)
		{
			if(sps_info.frame_crop_left_offset > 0 || sps_info.frame_crop_right_offset > 0)
				decoder_ctx->width = decoder_ctx->width - (sps_info.frame_crop_right_offset + sps_info.frame_crop_left_offset);
			if(sps_info.frame_crop_top_offset > 0 || sps_info.frame_crop_bottom_offset > 0)
				decoder_ctx->height = decoder_ctx->height - (sps_info.frame_crop_top_offset + sps_info.frame_crop_bottom_offset);
		}
	}
	else
	{
		if(ms_getreso_spsinfo((char *)frame_data, frame_data_size, decoder_ctx))
		{
			return 0;
		}
		if(ms_h264dec_seq_parameter_set(decoder_ctx->data, decoder_ctx->size, &sps_info))
		{
			return 0;
		}
		decoder_ctx->width = (sps_info.pic_width_in_mbs_minus1 + 1) * 16;
		decoder_ctx->height = (sps_info.pic_height_in_map_units_minus1 + 1) * 16;
		if(sps_info.frame_cropping_flag)
		{
			if(sps_info.frame_crop_left_offset > 0 || sps_info.frame_crop_right_offset > 0)
				decoder_ctx->width = decoder_ctx->width - (sps_info.frame_crop_right_offset + sps_info.frame_crop_left_offset);
			if(sps_info.frame_crop_top_offset > 0 || sps_info.frame_crop_bottom_offset > 0)
				decoder_ctx->height = decoder_ctx->height - (sps_info.frame_crop_top_offset + sps_info.frame_crop_bottom_offset);
		}
	}
	return 0;
}

int check_if_res_changed(int is_h265, unsigned char *frame_data, int frame_data_size, struct ms_decoder_context_t *ctx)
{
	if(frame_data_size > 0)
	{
		struct ms_decoder_context_t new_context = {0};
		ms_parse_h2645_res(is_h265, frame_data, frame_data_size, &new_context);
		if((new_context.width*new_context.height == 0)||
			(new_context.width == ctx->width &&  new_context.height == ctx->height))
		{
			return 0;
		}		
	}
	return 1;
}

int ms_get_resolution(struct ms_decoder_context_t *decoder_ctx,unsigned int *width,unsigned int *height)
{	
	*width = decoder_ctx->width;
	*height = decoder_ctx->height;
	return 0;
}

