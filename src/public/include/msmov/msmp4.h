#ifndef MS_MP4_H
#define MS_MP4_H

#include "mov-writer.h"
#include "mov-format.h"
#include "msfs/mf_type.h"
#include "msdefs.h"

#define MS_MP4_VIDEO_DATA_SIZE (2*1024*1024)

#define H264_NAL(v) (v & 0x1F)
#define HEVC_NAL(v) ((v>>1) & 0x3f)

struct mpeg4_hevc_t {
    uint8_t  configurationVersion;  // 1-only
    uint8_t  general_profile_space; // 2bit,[0,3]
    uint8_t  general_tier_flag;     // 1bit,[0,1]
    uint8_t  general_profile_idc;   // 5bit,[0,31]
    uint32_t general_profile_compatibility_flags;
    uint64_t general_constraint_indicator_flags;
    uint8_t  general_level_idc;
    uint16_t min_spatial_segmentation_idc;
    uint8_t  parallelismType;       // 2bit,[0,3]
    uint8_t  chromaFormat;          // 2bit,[0,3]
    uint8_t  bitDepthLumaMinus8;    // 3bit,[0,7]
    uint8_t  bitDepthChromaMinus8;  // 3bit,[0,7]
    uint16_t avgFrameRate;
    uint8_t  constantFrameRate;     // 2bit,[0,3]
    uint8_t  numTemporalLayers;     // 3bit,[0,7]
    uint8_t  temporalIdNested;      // 1bit,[0,1]
    uint8_t  lengthSizeMinusOne;    // 2bit,[0,3]
};

typedef struct mp4_operate_s {
    mov_writer_t *mov;
    unsigned char *cache_ptr;
    int video_flag;
    int audio_flag;
    unsigned long long video_first_pts;
    unsigned long long audio_first_pts;
    int video_track;
    int audio_track;
    unsigned int audio_fmt;

    char *mov_array;
    uint32_t mov_array_size;
    uint64_t mov_array_offset;  //just g_mov_array
    uint64_t mov_array_len;

    uint64_t mov_offset;        //all mp4 data
    uint64_t mov_len;
} mp4_operate_t;

int init_mp4(mp4_operate_t *mp4, int mdat_size);
void uninit_mp4(mp4_operate_t *mp4);
void ms_mp4_write(mp4_operate_t *mp4, struct mf_frame *frame);
void ms_mp4_destroy(mp4_operate_t *mp4);
void clear_mov_buf(mp4_operate_t *mp4);
int get_mov_buf(mp4_operate_t *mp4, char *data);
int get_mov_buf_size(mp4_operate_t *mp4);
uint64_t mp4_get_mdat_size(mp4_operate_t *mp4);

#endif
