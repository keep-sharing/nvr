#ifndef _____MS_RES_DECODER_H__
#define _____MS_RES_DECODER_H__

struct ms_decoder_context_t
{
	unsigned int width;
	unsigned int height;
	char data[1024];
	int size;
};

struct ms_decoder_extra_t
{
	char buf[512];
	int size;
};

int check_if_res_changed(int is_h265, unsigned char *frame_data, int frame_data_size, struct ms_decoder_context_t *decoder_ctx);

int ms_parse_h2645_res(int is_h265,unsigned char *frame_data, int frame_data_size, struct ms_decoder_context_t *decoder_ctx);

int ms_get_resolution(struct ms_decoder_context_t *decoder_ctx,unsigned int *width,unsigned int *height);

#endif
