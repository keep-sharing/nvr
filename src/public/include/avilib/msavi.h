#ifndef __MS_AVI_H__
#define __MS_AVI_H__
#include <stdio.h>

struct avi_format
{
	int video_width;
	int video_height;
	int video_codec_id;	// AV_CODEC_ID_H264 etc.
	int video_fps;
	//int video_bitrate;
	int audio_samplerate;
	int audio_bitrate;
	int audio_codec_id;	// AV_CODEC_ID_PCM_MULAW etc.
	int audio_channels;	//0x01:single channe x02:
	int reserved;		//"movi" LIST length
};

//local file
void *ms_avi_open(const char *filename, struct avi_format *avifmt);
int ms_avi_write(void *avihdl, void *data);
int ms_avi_close(void *avihdl);

//remote file
void *ms_avi_open_fp(FILE *fp);
int ms_avi_write_fp(void *avihdl, void *data);
int ms_avi_write_fp_ex(void *avihdl, const void *buf, const int size);
int ms_avi_close_fp(void *avihdl);


int ms_avi_update_fps(void *avihdl, int secs);

#endif
