#include "flv-muxer.h"
#include "flv-proto.h"
#include "amf0.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "mpeg4-aac.h"
#include "mpeg4-avc.h"
#include "mpeg4-hevc.h"
#include "mp3-header.h"

#define FLV_MUXER "libflv"

struct flv_muxer_t
{
	flv_muxer_handler handler;
	void* param;

	uint8_t aac_sequence_header;
	uint8_t avc_sequence_header;

	struct mpeg4_aac_t aac;
	union
	{
		struct mpeg4_avc_t avc;
		struct mpeg4_hevc_t hevc;
	} v;
	int vcl; // 0-non vcl, 1-idr, 2-p/b
	int update; // avc/hevc sequence header update

	uint8_t* ptr;
	size_t bytes;
	size_t capacity;
};

struct flv_muxer_t* flv_muxer_create(flv_muxer_handler handler, void* param)
{
	struct flv_muxer_t* flv;
	flv = (struct flv_muxer_t*)calloc(1, sizeof(struct flv_muxer_t));
	if (NULL == flv)
		return NULL;

	flv_muxer_reset(flv);
	flv->handler = handler;
	flv->param = param;
	return flv;
}

void flv_muxer_destroy(struct flv_muxer_t* flv)
{
	if (flv->ptr)
	{
		assert(flv->capacity > 0);
		free(flv->ptr);
		flv->ptr = NULL;
	}

	free(flv);
}

int flv_muxer_reset(struct flv_muxer_t* flv)
{
	memset(&flv->v, 0, sizeof(flv->v));
	flv->aac_sequence_header = 0;
	flv->avc_sequence_header = 0;
	return 0;
}

static int flv_muxer_alloc(struct flv_muxer_t* flv, size_t bytes)
{
	void* p;
	p = realloc(flv->ptr, bytes);
	if (!p)
		return ENOMEM;

	flv->ptr = (uint8_t*)p;
	flv->capacity = bytes;
	return 0;
}

int flv_muxer_mp3(struct flv_muxer_t* flv, const void* data, size_t bytes, uint32_t pts, uint32_t dts)
{
	uint8_t ch, hz;
	struct mp3_header_t mp3;
	(void)pts;

	if (0 == mp3_header_load(&mp3, data, bytes))
	{
		return EINVAL;
	}
	else
	{
		ch = 3 == mp3.mode ? 0 : 1;
		switch (mp3_get_frequency(&mp3))
		{
		case 5500: hz = 0; break;
		case 11000: hz = 1; break;
		case 22000: hz = 2; break;
		case 44100: hz = 3; break;
		default: hz = 3;
		}
	}

	if (flv->capacity < bytes + 1)
	{
		if (0 != flv_muxer_alloc(flv, bytes + 4))
			return ENOMEM;
	}

	flv->ptr[0] = (FLV_AUDIO_MP3 /*<< 4*/) /* SoundFormat */ | (hz << 2) /* SoundRate */ | (1 << 1) /* 16-bit samples */ | ch /* Stereo sound */;
	memcpy(flv->ptr + 1, data, bytes); // MP3
	return flv->handler(flv->param, FLV_TYPE_AUDIO, flv->ptr, bytes + 1, dts);
}

int flv_muxer_aac(struct flv_muxer_t* flv, const void* data, size_t bytes, uint32_t pts, uint32_t dts)
{
	int r, n, m;
	(void)pts;

	if (flv->capacity < bytes + 2/*AudioTagHeader*/ + 2/*AudioSpecificConfig*/)
	{
		if (0 != flv_muxer_alloc(flv, bytes + 4))
			return ENOMEM;
	}

	/* ADTS */
	n = mpeg4_aac_adts_load(data, bytes, &flv->aac);
	if (n <= 0)
		return -1; // invalid data

	if (0 == flv->aac_sequence_header)
	{
		flv->aac_sequence_header = 1; // once only

		flv->ptr[0] = (FLV_AUDIO_AAC /*<< 4*/) /* SoundFormat */ | (3 << 2) /* 44k-SoundRate */ | (1 << 1) /* 16-bit samples */ | 1 /* Stereo sound */;
		flv->ptr[1] = 0; // AACPacketType: 0-AudioSpecificConfig(AAC sequence header)
		m = mpeg4_aac_audio_specific_config_save(&flv->aac, flv->ptr + 2, flv->capacity - 2);
		assert(m + 2 <= (int)flv->capacity);
		r = flv->handler(flv->param, FLV_TYPE_AUDIO, flv->ptr, m + 2, dts);
		if (0 != r) return r;
	}

	flv->ptr[0] = (FLV_AUDIO_AAC /*<< 4*/) /* SoundFormat */ | (3 << 2) /* 44k-SoundRate */ | (1 << 1) /* 16-bit samples */ | 1 /* Stereo sound */;
	flv->ptr[1] = 1; // AACPacketType: 1-AAC raw
	memcpy(flv->ptr + 2, (uint8_t*)data + n, bytes - n); // AAC exclude ADTS
	assert(bytes - n + 2 <= (int)flv->capacity);
	return flv->handler(flv->param, FLV_TYPE_AUDIO, flv->ptr, bytes - n + 2, dts);
}

static int flv_muxer_h264(struct flv_muxer_t* flv, uint32_t pts, uint32_t dts)
{
	int r;
	int m, compositionTime;

	if ( /*0 == flv->avc_sequence_header &&*/ flv->update && flv->v.avc.nb_sps > 0 && flv->v.avc.nb_pps > 0)
	{
		flv->ptr[flv->bytes + 0] = (1 << 4) /*FrameType*/ | FLV_VIDEO_H264 /*CodecID*/;
		flv->ptr[flv->bytes + 1] = 0; // AVC sequence header
		flv->ptr[flv->bytes + 2] = 0; // CompositionTime 0
		flv->ptr[flv->bytes + 3] = 0;
		flv->ptr[flv->bytes + 4] = 0;
		m = mpeg4_avc_decoder_configuration_record_save(&flv->v.avc, flv->ptr + flv->bytes + 5, flv->capacity - flv->bytes - 5);
		if (m <= 0)
			return -1; // invalid data

		flv->avc_sequence_header = 1; // once only
		assert(flv->bytes + m + 5 <= (int)flv->capacity);
		r = flv->handler(flv->param, FLV_TYPE_VIDEO, flv->ptr + flv->bytes, m + 5, dts);
		if (0 != r) return r;
	}

	// has video frame
	if (flv->vcl && flv->avc_sequence_header)
	{
		compositionTime = pts - dts;
		flv->ptr[0] = ((1==flv->vcl ? 1 : 2) << 4) /*FrameType*/ | FLV_VIDEO_H264 /*CodecID*/;
		flv->ptr[1] = 1; // AVC NALU
		flv->ptr[2] = (compositionTime >> 16) & 0xFF;
		flv->ptr[3] = (compositionTime >> 8) & 0xFF;
		flv->ptr[4] = compositionTime & 0xFF;

		assert(flv->bytes <= (int)flv->capacity);
		return flv->handler(flv->param, FLV_TYPE_VIDEO, flv->ptr, flv->bytes, dts);
	}
	return 0;
}

int flv_muxer_avc(struct flv_muxer_t* flv, const void* data, size_t bytes, uint32_t pts, uint32_t dts)
{
	if (flv->capacity < bytes + 2048/*AVCDecoderConfigurationRecord*/)
	{
		if (0 != flv_muxer_alloc(flv, bytes + 2048))
			return ENOMEM;
	}

	flv->bytes = 5;
	flv->bytes += h264_annexbtomp4(&flv->v.avc, data, bytes, flv->ptr + flv->bytes, flv->capacity - flv->bytes, &flv->vcl, &flv->update);
	if (flv->bytes <= 5)
		return ENOMEM;

	return flv_muxer_h264(flv, pts, dts);
}

static int flv_muxer_h265(struct flv_muxer_t* flv, uint32_t pts, uint32_t dts)
{
	int r;
	int m, compositionTime;

	if ( /*0 == flv->avc_sequence_header &&*/ flv->update && flv->v.hevc.numOfArrays >= 3) // vps + sps + pps
	{
		flv->ptr[flv->bytes + 0] = (1 << 4) /*FrameType*/ | FLV_VIDEO_H265 /*CodecID*/;
		flv->ptr[flv->bytes + 1] = 0; // HEVC sequence header
		flv->ptr[flv->bytes + 2] = 0; // CompositionTime 0
		flv->ptr[flv->bytes + 3] = 0;
		flv->ptr[flv->bytes + 4] = 0;
		m = mpeg4_hevc_decoder_configuration_record_save(&flv->v.hevc, flv->ptr + flv->bytes + 5, flv->capacity - flv->bytes - 5);
		if (m <= 0)
			return -1; // invalid data

		flv->avc_sequence_header = 1; // once only
		assert(flv->bytes + m + 5 <= (int)flv->capacity);
		r = flv->handler(flv->param, FLV_TYPE_VIDEO, flv->ptr + flv->bytes, m + 5, dts);
		if (0 != r) return r;
	}

	// has video frame
	if (flv->vcl && flv->avc_sequence_header)
	{
		compositionTime = pts - dts;
		flv->ptr[0] = ((1==flv->vcl ? 1 : 2) << 4) /*FrameType*/ | FLV_VIDEO_H265 /*CodecID*/;
		flv->ptr[1] = 1; // HEVC NALU
		flv->ptr[2] = (compositionTime >> 16) & 0xFF;
		flv->ptr[3] = (compositionTime >> 8) & 0xFF;
		flv->ptr[4] = compositionTime & 0xFF;

		assert(flv->bytes <= (int)flv->capacity);
		return flv->handler(flv->param, FLV_TYPE_VIDEO, flv->ptr, flv->bytes, dts);
	}
	return 0;
}

int flv_muxer_hevc(struct flv_muxer_t* flv, const void* data, size_t bytes, uint32_t pts, uint32_t dts)
{
	if (flv->capacity < bytes + 2048/*HEVCDecoderConfigurationRecord*/)
	{
		if (0 != flv_muxer_alloc(flv, bytes + 2048))
			return ENOMEM;
	}

	flv->bytes = 5;
	flv->bytes += h265_annexbtomp4(&flv->v.hevc, data, bytes, flv->ptr + flv->bytes, flv->capacity - flv->bytes, &flv->vcl, &flv->update);
	if (flv->bytes <= 5)
		return ENOMEM;

	return flv_muxer_h265(flv, pts, dts);
}

int flv_muxer_metadata(flv_muxer_t* flv, const struct flv_metadata_t* metadata)
{
	uint8_t* ptr, *end;
	uint32_t count;

	if (!metadata) return -1;

	count = (metadata->audiocodecid ? 5 : 0) + (metadata->videocodecid ? 5 : 0) + 1;
	if (flv->capacity < 1024)
	{
		if (0 != flv_muxer_alloc(flv, 1024))
			return ENOMEM;
	}

	ptr = flv->ptr;
	end = flv->ptr + flv->capacity;
	count = (metadata->audiocodecid ? 5 : 0) + (metadata->videocodecid ? 5 : 0) + 1;

	// ScriptTagBody

	// name
	ptr = AMFWriteString(ptr, end, "onMetaData", 10);

	// value: SCRIPTDATAECMAARRAY
	ptr[0] = AMF_ECMA_ARRAY;
	ptr[1] = (uint8_t)((count >> 24) & 0xFF);;
	ptr[2] = (uint8_t)((count >> 16) & 0xFF);;
	ptr[3] = (uint8_t)((count >> 8) & 0xFF);
	ptr[4] = (uint8_t)(count & 0xFF);
	ptr += 5;

	if (metadata->audiocodecid)
	{
		ptr = AMFWriteNamedDouble(ptr, end, "audiocodecid", 12, metadata->audiocodecid);
		ptr = AMFWriteNamedDouble(ptr, end, "audiodatarate", 13, metadata->audiodatarate);
		ptr = AMFWriteNamedDouble(ptr, end, "audiosamplerate", 15, metadata->audiosamplerate);
		ptr = AMFWriteNamedDouble(ptr, end, "audiosamplesize", 15, metadata->audiosamplesize);
		ptr = AMFWriteNamedBoolean(ptr, end, "stereo", 6, (uint8_t)metadata->stereo);
	}

	if (metadata->videocodecid)
	{
		ptr = AMFWriteNamedDouble(ptr, end, "videocodecid", 12, metadata->videocodecid);
		ptr = AMFWriteNamedDouble(ptr, end, "videodatarate", 13, metadata->videodatarate);
		ptr = AMFWriteNamedDouble(ptr, end, "framerate", 9, metadata->framerate);
		ptr = AMFWriteNamedDouble(ptr, end, "height", 6, metadata->height);
		ptr = AMFWriteNamedDouble(ptr, end, "width", 5, metadata->width);
	}

	ptr = AMFWriteNamedString(ptr, end, "encoder", 7, FLV_MUXER, strlen(FLV_MUXER));
	ptr = AMFWriteObjectEnd(ptr, end);

	return flv->handler(flv->param, FLV_TYPE_SCRIPT, flv->ptr, ptr - flv->ptr, 0);
}
