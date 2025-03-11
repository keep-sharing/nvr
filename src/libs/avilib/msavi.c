#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "msavi.h"
#include "msstd.h"
#include "avilib.h"
#include "msdefs.h"

typedef unsigned char UCHAR;
#define DECODER_H264	"h264"
#define DECODER_H265	"HEVC"

void *ms_avi_open(const char *filename_bk, struct avi_format *avifmt)
{
	AVI_HDL avihdl = NULL;
	char filename[512] = {0};
	if (filename_bk == NULL || avifmt == NULL ) 
		goto errout;
	avihdl = (AVI_HDL)ms_malloc(sizeof(struct avi_hdl));
	if (avihdl == NULL ) 
		goto errout;
	memset(avihdl, 0, sizeof(struct avi_hdl));
	snprintf(filename, sizeof(filename) - 5, "%s", filename_bk);
	if (strncmp(filename + strlen(filename) - 4, ".avi", 4) != 0) 
		strcat(filename, ".avi");

	if(ms_file_existed(filename))
	{
		ms_free(avihdl);
		goto errout;
	}
	if (!(avihdl->handle = AVI_open_output_file(filename)))
	{
		ms_free(avihdl);
		goto errout;
	}
 	if(avifmt->video_codec_id == CODECTYPE_H265)
		AVI_set_video(avihdl->handle, avifmt->video_width, avifmt->video_height, avifmt->video_fps, DECODER_H265);
	else
		AVI_set_video(avihdl->handle, avifmt->video_width, avifmt->video_height, avifmt->video_fps, DECODER_H264);
	
	return avihdl;
errout:
	return NULL;
}

int getEncodeAudiotype(int codec_type)
{
	if(codec_type == AUDIO_CODEC_G711)
		return WAVE_FORMAT_PCM;
	else if(codec_type == AUDIO_CODEC_AAC)
		return WAVE_FORMAT_AAC;
	else if(codec_type == AUDIO_CODEC_G711_MULAW)
		return WAVE_FORMAT_MULAW;
	else if(codec_type == AUDIO_CODEC_G711_ALAW)
		return WAVE_FORMAT_ALAW;
    else if(codec_type == AUDIO_CODEC_G722)
		return WAVE_FORMAT_G722_ADPCM;
    else if(codec_type == AUDIO_CODEC_G726)
		return WAVE_FORMAT_G726_ADPCM;
	else
		return WAVE_FORMAT_MULAW;
}

static int avi_get_decode_audio_bitrate(int sample, int law)
{
    int bitrate;

    if (law == WAVE_FORMAT_G722_ADPCM) {
        bitrate = 64;
    } else if (law == WAVE_FORMAT_G726_ADPCM) {
        bitrate = 16;
    } else if (law == WAVE_FORMAT_ALAW || law == WAVE_FORMAT_MULAW) {
        if (sample >= 32000) {
            bitrate = sample / 125;
        } else {
            bitrate = (sample == 8000 ? 64 : 128);
        }
    } else {
        bitrate = (sample == 8000 ? 64 : 128);
    }
    msprintf("sample:%d law:%d bitrate:%d", sample, law, bitrate);
    return bitrate;
}


int ms_avi_write(void *avihdl, void *data)
{
	int ret = -1;
	struct reco_frame *frame = (struct reco_frame *)data;
	if (!avihdl)
		return ret;
	AVI_HDL fp = (AVI_HDL)avihdl;
	if (frame->strm_type == ST_VIDEO) 
	{
		ret = AVI_write_frame(fp->handle, frame->data, frame->size, frame->frame_type == FT_IFRAME ? 1: 0);
	} 
	else 
	{
		if (!fp->audio_on) 
		{
            int ach = frame->achn ? frame->achn : 1;
			int law = getEncodeAudiotype(frame->codec_type);
			int sample = frame->sample ? frame->sample : 8000;
			int bitrate = avi_get_decode_audio_bitrate(sample, law);
            msprintf("AVI_set_audio ach:(%d %d) sample:(%d %d) law:(%d %d) bitrate:%d", frame->achn, ach,
                     frame->sample, sample, frame->codec_type, law, bitrate);
			AVI_set_audio(fp->handle, ach, sample, law==WAVE_FORMAT_AAC?16:8, law, bitrate);
			fp->audio_on = 1;
		}
		ret = AVI_write_audio(fp->handle, frame->data, frame->size);
	}

	return ret;
}

int ms_avi_close(void *avihdl)
{
	int ret = -1;
	if (!avihdl) 
		return -1;
	AVI_HDL fp = (AVI_HDL)avihdl;
	msprintf("###### fps:%f cnt:%ld ####", fp->handle->fps, fp->handle->video_frames);
	ret = AVI_close(fp->handle);
	ms_free(fp);
	return ret;
}

void *ms_avi_open_fp(FILE *fp)
{
	AVI_HDL avihdl = NULL;
	do 
	{
		avihdl = (AVI_HDL)ms_calloc(1, sizeof(struct avi_hdl));
		if (!avihdl)
			break;
		avihdl->fp = fp;
		return avihdl;
	} while (0);
	
	if (avihdl)
		ms_free(avihdl);
	return NULL;
}

int ms_avi_write_fp(void *avihdl, void *data)
{
	AVI_HDL pavi = (AVI_HDL)avihdl;
	if(!pavi || !pavi->fp)
		return -1;
	struct reco_frame *frame = (struct reco_frame *)data;
	if (fwrite(frame, 1, sizeof(*frame), pavi->fp) != sizeof(*frame))
		return -1;
	if (fwrite(frame->data, 1, frame->size, pavi->fp) != frame->size)
		return -1;
	
	return (sizeof(*frame) + frame->size);
}

int ms_avi_write_fp_ex(void *avihdl, const void *buf, const int size)
{
	AVI_HDL pavi = (AVI_HDL)avihdl;
	if(!pavi || !pavi->fp)
		return -1;	
	if (fwrite(buf, 1, size, pavi->fp) != size)
		return -1;
	
	return size;
}
int ms_avi_close_fp(void *avihdl)
{
	if (!avihdl) 
		return -1;
	
	ms_free(avihdl);
	return 0;
}

int ms_avi_update_fps(void *avihdl, int secs)
{
	if (!avihdl)
		return -1;
	AVI_HDL pavi = (AVI_HDL)avihdl;
	double fps = pavi->handle->fps;
	pavi->handle->fps = (double)pavi->handle->video_frames / secs;
	msprintf("ms_avi_update_fps frame cnt:%ld sec:%d fps:%f %f", pavi->handle->video_frames, secs, fps, pavi->handle->fps);
	return avi_update_header(pavi->handle);
}

