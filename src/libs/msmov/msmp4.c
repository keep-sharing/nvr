#include "msmp4.h"
#include "msstd.h"
//#include "msavstream.h"

//static int mov_file_read(void* fp, void* data, uint64_t bytes)
//{
//    if (bytes == fread(data, 1, bytes, (FILE*)fp))
//        return 0;
//  return 0 != ferror((FILE*)fp) ? ferror((FILE*)fp) : -1 /*EOF*/;
//}

//static int mov_file_write(void* fp, const void* data, uint64_t bytes)
//{
//  return bytes == fwrite(data, 1, bytes, (FILE*)fp) ? 0 : ferror((FILE*)fp);
//}

//static int mov_file_seek(void* fp, uint64_t offset)
//{
//  return fseek((FILE*)fp, offset, SEEK_SET);
//}

//static uint64_t mov_file_tell(void* fp)
//{
//  return ftell((FILE*)fp);
//}
#define MIN_AAC_ADTS_HEADER_SIZE (7)

typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;
typedef signed int  s32;
typedef signed short    s16;
typedef signed char s8;

static u8 get_nalu_start_code_len(const u8 *p)
{
    if (p[2] == 0x01) {
        return 3;
    } else {
        return 4;
    }
}

static int mov_file_read(void *param, void *data, uint64_t bytes)
{
    return 0;
}

static int mov_file_write(void *param, const void *data, uint64_t bytes)
{
    mp4_operate_t *mp4 = (mp4_operate_t *)param;
    if (mp4->mov_array_offset + bytes > mp4->mov_array_size) {
        mp4->mov_array_size += (1024 * 200); // add 200k
        mp4->mov_array = ms_realloc(mp4->mov_array, mp4->mov_array_size);
    }
    memcpy(mp4->mov_array + mp4->mov_array_offset, data, bytes);
    if (mp4->mov_offset == mp4->mov_len) {  //write new
        mp4->mov_array_offset += bytes;
        mp4->mov_offset += bytes;
        mp4->mov_len += bytes;
        mp4->mov_array_len += bytes;
    } else {                            //rewrite
        mp4->mov_array_offset += bytes;
        mp4->mov_offset += bytes;
    }
    return 0;
}

static int mov_file_seek(void *param, uint64_t offset)
{
    mp4_operate_t *mp4 = (mp4_operate_t *)param;
    mp4->mov_array_offset = mp4->mov_array_len - (mp4->mov_len - offset);
    mp4->mov_offset = offset;
    return 0;
}

static uint64_t mov_file_tell(void *param)
{
    mp4_operate_t *mp4 = (mp4_operate_t *)param;
    return mp4->mov_offset;
}

static struct mov_buffer_t g_mov_file_io = {
    mov_file_read,
    mov_file_write,
    mov_file_seek,
    mov_file_tell,
};

static inline const BYTE *nalu_start_code(const BYTE *ptr, const BYTE *end)
{
    const BYTE *p = ptr;
    for (; p + 3 < end; p++) {
        if (0x00 == p[0] && 0x00 == p[1] && (0x01 == p[2] || (0x00 == p[2] && 0x01 == p[3]))) {
            return p;
        }
    }
    return end;
}

int h264_nal_type(const unsigned char *ptr)
{
    int i = 2;
    assert(0x00 == ptr[0] && 0x00 == ptr[1]);
    if (0x00 == ptr[2]) {
        ++i;
    }
    assert(0x01 == ptr[i]);
    return H264_NAL(ptr[i + 1]);
}

int hevc_nal_type(const unsigned char *ptr)
{
    int i = 2;
    assert(0x00 == ptr[0] && 0x00 == ptr[1]);
    if (0x00 == ptr[2]) {
        ++i;
    }
    assert(0x01 == ptr[i]);
    return HEVC_NAL(ptr[i + 1]);
}

static int h264_to_mp4_extra_data(const BYTE *frame, const BYTE *frame_end, char *extra_data, int extra_data_size)
{
    const BYTE *sps = NULL, *pps = NULL;
    int sps_size = 0, pps_size = 0, nalu_size = 0;
    const BYTE *nalu = nalu_start_code(frame, frame_end);
    char *p = NULL;
    if (extra_data_size < 8) {
        return -1;
    }
    while (nalu < frame_end) {
        const BYTE *nalu2 = nalu_start_code(nalu + 4, frame_end);
        if (h264_nal_type(nalu) == 7) { //sps
            if (sps_size == 0) {
                if (nalu[2] == 0x01) { //nalu length is 3
                    sps = &nalu[3];
                    nalu_size = 3;
                } else { //nalu length is 4
                    sps = &nalu[4];
                    nalu_size = 4;
                }
                sps_size = (int)(nalu2 - sps);
            }
        } else if (h264_nal_type(nalu) == 8) { //pps
            if (pps_size == 0) {
                if (nalu[2] == 0x01) { //nalu length is 3
                    pps = &nalu[3];
                } else { //nalu length is 4
                    pps = &nalu[4];
                }
                pps_size = (int)(nalu2 - pps);
            }
        } else if (h264_nal_type(nalu) <= 5) {
            break;
        }
        nalu = nalu2;
    }
    if (sps_size > 0 && pps_size > 0) {
        int mp4_extra_data_size = 5/*mp4 extra_size header*/ + 3/* sps num 1byte+ sps length 2 bytes*/ +
                                  3/* pps num 1byte+ pps length 2 bytes*/
                                  + sps_size + pps_size;
        if (mp4_extra_data_size > extra_data_size) {
            return -1;
        }
        p = extra_data;
        *p++ = 1;
        *p++ = sps[1];//profile
        *p++ = sps[2];//compatibility
        *p++ = sps[3];//level
        *p++ = 0xfc | (nalu_size - 1); //nalu size - 1;
        //start sps
        *p++ = 0xe0 | 0x01; // sps num
        *p++ = (sps_size >> 8) & 0xff;
        *p++ = (sps_size) & 0xff;
        memcpy(p, sps, sps_size);
        p = p + sps_size;
        *p++ = 0x01; // pps num
        *p++ = (pps_size >> 8) & 0xff;
        *p++ = (pps_size) & 0xff;
        memcpy(p, pps, pps_size);
        return mp4_extra_data_size;
    }
    return 0;
}

static size_t hevc_rbsp_decode(const uint8_t *nalu, size_t bytes, uint8_t *sodb)
{
    size_t i, j;
    for (j = i = 0; i < bytes; i++) {
        if (i + 2 < bytes && 0 == nalu[i] && 0 == nalu[i + 1] && 0x03 == nalu[i + 2]) {
            sodb[j++] = nalu[i];
            sodb[j++] = nalu[i + 1];
            i += 2;
        } else {
            sodb[j++] = nalu[i];
        }
    }
    return j;
}

static int hevc_extradata_to_mp4_extra_data(const BYTE *vps, int vps_size, const BYTE *sps, int sps_size,
                                            const BYTE *pps, int pps_size, int nalu_size, char *extra_data, int extra_data_size)
{
    char *p = NULL;
    struct mpeg4_hevc_t hevc = {0};
    BYTE vps_rbsp[64] = {0};
    size_t sodb_bytes = 0;
    int mp4_extra_data_size = 23/*hevc mp4 extra_size header*/ + 5/* nalu type 1 byte +vps num 2byte+ vps length 2 bytes*/ +
                              5/* nalu type 1 byte +sps num 2byte+ sps length 2 bytes*/ + 5/* nalu type 1 byte + pps num 2byte+ pps length 2 bytes*/
                              + vps_size + sps_size + pps_size;

    if (mp4_extra_data_size > extra_data_size || sizeof(vps_rbsp) < vps_size) {
        return -1;
    }
    hevc.configurationVersion = 1;
    sodb_bytes = hevc_rbsp_decode(vps, (size_t)vps_size, vps_rbsp);
    if (sodb_bytes < 6/*'profile_tier_level' offset*/ + 12/*num 'profile_tier_level' bytes*/) {
        return -1;
    }

    hevc.numTemporalLayers = ((vps_rbsp[3] >> 1) & 0x07) + 1;
    hevc.temporalIdNested = vps_rbsp[3] & 0x01;
    hevc.lengthSizeMinusOne = nalu_size - 1;
    //profile_ter_level
    hevc.general_profile_space = (vps_rbsp[6] >> 6) & 0x03;
    hevc.general_tier_flag = (vps_rbsp[6] >> 5) & 0x01;
    hevc.general_profile_idc = vps_rbsp[6] & 0x1f;
    hevc.general_profile_compatibility_flags = 0;
    hevc.general_profile_compatibility_flags |= vps_rbsp[7] << 24;
    hevc.general_profile_compatibility_flags |= vps_rbsp[8] << 16;
    hevc.general_profile_compatibility_flags |= vps_rbsp[9] << 8;
    hevc.general_profile_compatibility_flags |= vps_rbsp[10];

    hevc.general_constraint_indicator_flags = 0;
    hevc.general_constraint_indicator_flags |= ((uint64_t)vps_rbsp[11]) << 40;
    hevc.general_constraint_indicator_flags |= ((uint64_t)vps_rbsp[12]) << 32;
    hevc.general_constraint_indicator_flags |= ((uint64_t)vps_rbsp[13]) << 24;
    hevc.general_constraint_indicator_flags |= ((uint64_t)vps_rbsp[14]) << 16;
    hevc.general_constraint_indicator_flags |= ((uint64_t)vps_rbsp[15]) << 8;
    hevc.general_constraint_indicator_flags |= vps_rbsp[16];

    hevc.general_level_idc = vps_rbsp[17];

    extra_data[0] = hevc.configurationVersion;
    extra_data[1] = vps_rbsp[6]; //general_profile_space + general_tier_flag + general_profile_idc
    extra_data[2] = vps_rbsp[7];
    extra_data[3] = vps_rbsp[8];
    extra_data[4] = vps_rbsp[9];
    extra_data[5] = vps_rbsp[10];

    extra_data[6] = hevc.general_constraint_indicator_flags >> 40;
    extra_data[7] = hevc.general_constraint_indicator_flags >> 32;
    extra_data[8] = hevc.general_constraint_indicator_flags >> 24;
    extra_data[9] = hevc.general_constraint_indicator_flags >> 16;
    extra_data[10] = hevc.general_constraint_indicator_flags >> 8;
    extra_data[11] = hevc.general_constraint_indicator_flags >> 16;

    extra_data[12] = vps_rbsp[17];// general_level_idc

    // min_spatial_segmentation_idc
    hevc.min_spatial_segmentation_idc = 0;
    extra_data[13] = (0xF000 | hevc.min_spatial_segmentation_idc) >> 8;
    extra_data[14] = (0xF000 | hevc.min_spatial_segmentation_idc);

    extra_data[15] = 0xFC | 0;//hevc->parallelismType; 0
    extra_data[16] = 0xFC | 1;//hevc->chromaFormat;//yuv 420 1
    extra_data[17] = 0xF8 | 0;//hevc->bitDepthLumaMinus8;
    extra_data[18] = 0xF8 | 0;//hevc->bitDepthChromaMinus8;
    extra_data[19] = (0 >> 8);//hevc->avgFrameRate 16bit
    extra_data[20] = 0;//hevc->avgFrameRate 16bit
    extra_data[21] = (0 << 6) | ((hevc.numTemporalLayers & 0x07) << 3) | ((hevc.temporalIdNested & 0x01) << 2) |
                     (hevc.lengthSizeMinusOne & 0x03);
    extra_data[22] = 0x03;//vps num + sps num + pps num

    p = &extra_data[23];
    //start
    *p++ = (1 << 7) | ((vps[0] >> 1) & 0x3f); // 1 byte
    *p++ = (1 >> 8) & 0xff; //vps num
    *p++ = 1 & 0xff;
    *p++ = (vps_size >> 8) & 0xff;
    *p++ = (vps_size) & 0xff;
    memcpy(p, vps, vps_size);
    p = p + vps_size;
    *p++ = (1 << 7) | ((sps[0] >> 1) & 0x3f); // 1 byte
    *p++ = (1 >> 8) & 0xff; //sps num
    *p++ = 1 & 0xff;
    *p++ = (sps_size >> 8) & 0xff;
    *p++ = (sps_size) & 0xff;
    memcpy(p, sps, sps_size);
    p = p + sps_size;
    *p++ = (1 << 7) | ((pps[0] >> 1) & 0x3f); // 1 byte
    *p++ = (1 >> 8) & 0xff; //pps num
    *p++ = 1 & 0xff;
    *p++ = (pps_size >> 8) & 0xff;
    *p++ = (pps_size) & 0xff;
    memcpy(p, pps, pps_size);
    return mp4_extra_data_size;
}

static int hevc_to_mp4_extra_data(const BYTE *frame, const BYTE *frame_end, char *extra_data, int extra_data_size)
{
    const BYTE *sps = NULL, *pps = NULL, *vps = NULL;
    int sps_size = 0, pps_size = 0, vps_size = 0, nalu_size = 0;
    const BYTE *nalu = nalu_start_code(frame, frame_end);
    if (extra_data_size < 23) {
        return -1;
    }
    while (nalu < frame_end) {
        const BYTE *nalu2 = nalu_start_code(nalu + 4, frame_end);
        if (hevc_nal_type(nalu) == 32) { //vps
            if (vps_size == 0) {
                if (nalu[2] == 0x01) { //nalu length is 3
                    vps = &nalu[3];
                    nalu_size = 3;
                } else { //nalu length is 4
                    vps = &nalu[4];
                    nalu_size = 4;
                }
                vps_size = (int)(nalu2 - vps);
            }
        } else if (hevc_nal_type(nalu) == 33) { //sps
            if (sps_size == 0) {
                if (nalu[2] == 0x01) { //nalu length is 3
                    sps = &nalu[3];
                } else { //nalu length is 4
                    sps = &nalu[4];
                }
                sps_size = (int)(nalu2 - sps);
            }
        } else if (hevc_nal_type(nalu) == 34) { //pps
            if (pps_size == 0) {
                if (nalu[2] == 0x01) { //nalu length is 3
                    pps = &nalu[3];
                } else { //nalu length is 4
                    pps = &nalu[4];
                }
                pps_size = (int)(nalu2 - pps);
            }
        } else if (hevc_nal_type(nalu) <= 31) {
            break;
        }
        nalu = nalu2;
    }
    if (sps_size > 0 && pps_size > 0 && vps_size > 0) {
        return hevc_extradata_to_mp4_extra_data(vps, vps_size, sps, sps_size,
                                                pps, pps_size, nalu_size, extra_data, extra_data_size);
    }
    return 0;
}

int mpeg4_to_mp4_extra_data(int codec_type, const BYTE *frame, int frame_size, char *extra_data, int extra_data_size)
{
    int ret = 0;
    if (codec_type == CODECTYPE_H264) {
        ret = h264_to_mp4_extra_data(frame, frame + frame_size, extra_data, extra_data_size);
    } else if (codec_type == CODECTYPE_H265) {
        ret = hevc_to_mp4_extra_data(frame, frame + frame_size, extra_data, extra_data_size);
    } else {
        return -1;
    }
    return ret;
}

static int aac_to_mp4_extra_data(const BYTE *frame, const BYTE *frame_end, char *extra_data, int extra_data_size)
{
    if (extra_data_size < 2 || frame + 7 > frame_end) { //adts header 7 bytes
        return -1;
    }
    if (0xFF == frame[0] && 0xF0 == (frame[1] & 0xF0)) {
        uint8_t profile; // 0-NULL, 1-AAC Main, 2-AAC LC, 2-AAC SSR, 3-AAC LTP
        uint8_t sampling_frequency_index; // 0-96000, 1-88200, 2-64000, 3-48000, 4-44100, 5-32000, 6-24000, 7-22050, 8-16000, 9-12000, 10-11025, 11-8000, 12-7350, 13/14-reserved, 15-frequency is written explictly
        uint8_t channel_configuration; // 0-AOT, 1-1channel,front-center, 2-2channels, front-left/right, 3-3channels: front center/left/right, 4-4channels: front-center/left/right, back-center, 5-5channels: front center/left/right, back-left/right, 6-6channels: front center/left/right, back left/right LFE-channel, 7-8channels

        profile = ((frame[2] >> 6) & 0x03) + 1; // 2 bits: the MPEG-2 Audio Object Type add 1
        sampling_frequency_index = (frame[2] >> 2) & 0x0F; // 4 bits: MPEG-4 Sampling Frequency Index (15 is forbidden)
        channel_configuration = ((frame[2] & 0x01) << 2) | ((frame[3] >> 6) & 0x03); // 3 bits: MPEG-4 Channel Configuration
        if (channel_configuration == 0) {
            return 0;    //mpeg4_aac_adts_pce_load
        }

        extra_data[0] = (profile << 3) | ((sampling_frequency_index >> 1) & 0x07);
        extra_data[1] = ((sampling_frequency_index & 0x01) << 7) | ((channel_configuration & 0xF) << 3) |
                        (0 << 2) /* frame length-1024 samples*/ | (0 << 1) /* don't depend on core */ | 0 /* not extension */;
        extra_data_size = 2;
        return extra_data_size;
    }
    return 0;
}

int audio_to_mp4_extra_data(int audio_format, const BYTE *frame, int frame_size, char *extra_data, int extra_data_size)
{
    int ret = 0;
    if (audio_format == AUDIO_CODEC_AAC) {
        ret = aac_to_mp4_extra_data(frame, frame + frame_size, extra_data, extra_data_size);
    } else if (audio_format == AUDIO_CODEC_G711_MULAW || audio_format == AUDIO_CODEC_G711_ALAW) {
        ret = 0;
    } else {
        return -1;
    }
    return ret;
}

u32 annexb_to_mp4_avc_data(u8 *frame_addr, u32 frame_size, u8 *cache_ptr, u32 cache_size, u8 **video_data)
{
    u8 *frame_end = frame_addr + frame_size;
    u8 *ptr = cache_ptr;
    u32 video_data_size = 0;
    u8 naluStartCodeLen;
    const BYTE *nalu = nalu_start_code(frame_addr, frame_end);
    while (nalu < frame_end) {
        naluStartCodeLen = get_nalu_start_code_len(nalu);
        const BYTE *nalu2 = nalu_start_code(nalu + naluStartCodeLen, frame_end);
        int nal_type = h264_nal_type(nalu);
        int nal_unit_lengh = 0;
        switch (nal_type) {
            case 7://vps
            case 8://sps
                nalu = nalu2;
                continue;
            case 6://sei
                // zouwm add.SEI自定义信息中可能包含000001
                if (nalu[naluStartCodeLen+2] == 0x05) {
                    u8 seiSize = 4 + nalu[naluStartCodeLen+3];  // 06 05 size paylod 80
                    if (nalu + naluStartCodeLen + seiSize == nalu2) {   // real next nalu
                    } else {
                        nalu2 = nalu_start_code(nalu2 + naluStartCodeLen, frame_end);
                    }
                }
                break;
            default://5 or 1
                nalu2 = nalu_start_code(nalu + naluStartCodeLen, frame_end);
                break;
        }
        if (nalu2 - nalu <= naluStartCodeLen) {
            break;
        }
        nal_unit_lengh = (int)(nalu2 - nalu) - naluStartCodeLen;
        if (video_data_size + nal_unit_lengh + naluStartCodeLen > cache_size) {
            break;
        }
        ptr[0] = (nal_unit_lengh >> 24) & 0xff;
        ptr[1] = (nal_unit_lengh >> 16) & 0xff;
        ptr[2] = (nal_unit_lengh >> 8) & 0xff;
        ptr[3] = (nal_unit_lengh >> 0) & 0xff;
        memcpy(&ptr[4], &nalu[naluStartCodeLen], nal_unit_lengh);
        video_data_size += 4 + nal_unit_lengh;
        ptr += 4 + nal_unit_lengh;
        nalu = nalu2;
    }
    *video_data = cache_ptr;
    return video_data_size;
}

u32 annexb_to_mp4_hevc_data(u8 *frame_addr, u32 frame_size, u8 *cache_ptr, u32 cache_size, u8 **video_data)
{
    // all separator is 00000001
    u8 *frame_end = frame_addr + frame_size;
    u8 *ptr = cache_ptr;
    u32 video_data_size = 0;
    u8 naluStartCodeLen;
    const BYTE *nalu = nalu_start_code(frame_addr, frame_end);
    while (nalu < frame_end) {
        naluStartCodeLen = get_nalu_start_code_len(nalu);
        const BYTE *nalu2 = nalu_start_code(nalu + naluStartCodeLen, frame_end);
        int nal_type = hevc_nal_type(nalu);
        int nal_unit_lengh = 0;
        switch (nal_type) {
            case 32://vps
            case 33://sps
            case 34://pps
                nalu = nalu2;
                continue;
            case 39://sei
                // zouwm add.SEI自定义信息中可能包含000001
                if (nalu[naluStartCodeLen+2] == 0x05) {
                    u8 seiSize = 4 + nalu[naluStartCodeLen+3];  // 06 05 size paylod 80
                    if (nalu + naluStartCodeLen + seiSize == nalu2) {   // real next nalu
                    } else {
                        nalu2 = nalu_start_code(nalu2 + naluStartCodeLen, frame_end);
                    }
                }
                break;
            default:
                nalu2 = nalu_start_code(nalu + naluStartCodeLen, frame_end);
                break;
        }
        if (nalu2 - nalu <= naluStartCodeLen) {
            break;
        }
        
        nal_unit_lengh = (int)(nalu2 - nalu) - naluStartCodeLen;
        
        if (video_data_size + nal_unit_lengh + naluStartCodeLen > cache_size) {
            break;
        }
        ptr[0] = (nal_unit_lengh >> 24) & 0xff;
        ptr[1] = (nal_unit_lengh >> 16) & 0xff;
        ptr[2] = (nal_unit_lengh >> 8) & 0xff;
        ptr[3] = (nal_unit_lengh >> 0) & 0xff;
        memcpy(&ptr[4], &nalu[naluStartCodeLen], nal_unit_lengh);
        video_data_size += 4 + nal_unit_lengh;
        ptr += 4 + nal_unit_lengh;
        nalu = nalu2;
    }
    *video_data = cache_ptr;
    return video_data_size;
}

u32 annexb_to_mp4_video_data(int codec_type, u8 *frame_addr, u32 frame_size, u8 *cache_ptr, u32 cache_size,
                             u8 **video_data)
{
    if (frame_size > cache_size) {
        msdebug(DEBUG_ERR, "frame size bigger than cache_size so record the raw data");
        *video_data = frame_addr;
        return frame_size;
    }
    if (codec_type == CODECTYPE_H264) {
        return annexb_to_mp4_avc_data(frame_addr, frame_size, cache_ptr, cache_size, video_data);
    } else if (codec_type == CODECTYPE_H265) {
        return annexb_to_mp4_hevc_data(frame_addr, frame_size, cache_ptr, cache_size, video_data);
    } else { //mjpeg
        *video_data = frame_addr;
        return frame_size;
    }
    return 0;
}

int init_mp4(mp4_operate_t *mp4, int mdat_size)
{
    mp4->cache_ptr = ms_malloc(MS_MP4_VIDEO_DATA_SIZE);
    mp4->mov_array = ms_malloc(MS_MP4_VIDEO_DATA_SIZE);
    mp4->mov_array_size = MS_MP4_VIDEO_DATA_SIZE;
    mp4->mov_array_offset = 0;
    mp4->mov_array_len = 0;
    mp4->mov_offset = 0;
    mp4->mov_len = 0;
    mp4->video_flag = 0;
    mp4->audio_flag = 0;
    mp4->video_first_pts = 0;
    mp4->audio_first_pts = 0;
    mp4->video_track = -1;
    mp4->audio_track = -1;
    mp4->mov = mov_writer_create(&g_mov_file_io, mp4, MOV_FLAG_FASTSTART, mdat_size);
    if (!mp4->mov) {
        return -1;
    }
    return 0;
}

void ms_mp4_write(mp4_operate_t *mp4, struct mf_frame *frame)
{
    int64_t pts = 0;
    u32 ptr_size = 0;

    u8 *frame_data = NULL;
    if (frame->strm_type == ST_AUDIO &&
        (frame->codec_type == AUDIO_CODEC_G722 || frame->codec_type == AUDIO_CODEC_G726)) {
        return;
    }
    if (AUDIO_CODEC_AAC == frame->codec_type && frame->size < MIN_AAC_ADTS_HEADER_SIZE) {
        return;
    }
    if (mp4->video_flag == 0 && frame->strm_type == ST_VIDEO) {
        if (frame->frame_type != FT_IFRAME) {
            return;
        }
        mp4->video_flag = 1;
        char extra_data[256] = {0};
        int extra_size = 0;
        uint8_t object = 0;

        if (frame->codec_type == CODECTYPE_H264) {
            object = MOV_OBJECT_H264;
        } else if (frame->codec_type == CODECTYPE_H265) {
            object = MOV_OBJECT_HEVC;
        } else {
            object = MOV_OBJECT_JPEG;
        }
        if (object == MOV_OBJECT_H264 || object == MOV_OBJECT_HEVC) {
            extra_size = mpeg4_to_mp4_extra_data(frame->codec_type, frame->data, frame->size, extra_data, sizeof(extra_data));
            if (extra_size >= 0) {
                mp4->video_track = mov_writer_add_video(mp4->mov, object, frame->width, frame->height, extra_data, extra_size);
            }
        } else {
            mp4->video_track = mov_writer_add_video(mp4->mov, MOV_OBJECT_JPEG, frame->width, frame->height, NULL, 0);
        }
        mp4->video_first_pts = frame->time_usec;
    } else if (mp4->audio_flag == 0 && frame->strm_type == ST_AUDIO) {
        if (frame->codec_type == AUDIO_CODEC_G711_ALAW) {
            mp4->audio_fmt = MOV_OBJECT_G711a;
        } else if (frame->codec_type == AUDIO_CODEC_G711_MULAW) {
            mp4->audio_fmt = MOV_OBJECT_G711u;
        } else if (frame->codec_type == AUDIO_CODEC_AAC) {
            mp4->audio_fmt = MOV_OBJECT_AAC;
        }

        if (MOV_OBJECT_AAC == mp4->audio_fmt) {
            char extra_data[256] = {0};
            int extra_size = 0;
            extra_size = audio_to_mp4_extra_data(AUDIO_CODEC_AAC, frame->data, frame->size, extra_data, sizeof(extra_data));
            if (extra_size >= 0) {
                mp4->audio_track = mov_writer_add_audio(mp4->mov, mp4->audio_fmt, 1, 16, frame->sample, extra_data, extra_size);
            }
        } else {
            mp4->audio_track = mov_writer_add_audio(mp4->mov, mp4->audio_fmt, 1, 16, frame->sample, NULL, 0);
        }
        mp4->audio_flag = 1;
        mp4->audio_first_pts = frame->time_usec;
    }
    if (frame->strm_type == ST_VIDEO) {
        ptr_size = annexb_to_mp4_video_data(frame->codec_type, frame->data, frame->size, mp4->cache_ptr, MS_MP4_VIDEO_DATA_SIZE,
                                            &frame_data);
        pts = (frame->time_usec - mp4->video_first_pts) / 1000;
        if (ptr_size > 0) {
            mov_writer_write(mp4->mov, mp4->video_track, frame_data, ptr_size, pts, pts,
                             frame->frame_type == FT_IFRAME ? MOV_AV_FLAG_KEYFREAME : 0);
        }
    } else if (frame->strm_type == ST_AUDIO) {
        pts = (frame->time_usec - mp4->audio_first_pts) / 1000;
        if (MOV_OBJECT_AAC == mp4->audio_fmt) {
            mov_writer_write(mp4->mov, mp4->audio_track, frame->data + MIN_AAC_ADTS_HEADER_SIZE, frame->size - MIN_AAC_ADTS_HEADER_SIZE, pts, pts, 0);
        } else {
            mov_writer_write(mp4->mov, mp4->audio_track, frame->data, frame->size, pts, pts, 0);
        }
    }
}

void ms_mp4_destroy(mp4_operate_t *mp4)
{
    if (mp4->mov) {
        mov_writer_destroy(mp4->mov);
        mp4->mov = NULL;
    }
}

void uninit_mp4(mp4_operate_t *mp4)
{
    ms_free(mp4->cache_ptr);
    ms_free(mp4->mov_array);
}

void clear_mov_buf(mp4_operate_t *mp4)
{
    //memset(mp4->mov_array,0,MS_MP4_VIDEO_DATA_SIZE);
    mp4->mov_array_offset = 0;
    mp4->mov_array_len = 0;
}

int get_mov_buf(mp4_operate_t *mp4, char *data)
{
    memcpy(data, mp4->mov_array, mp4->mov_array_offset);
    return mp4->mov_array_offset;
}

int get_mov_buf_size(mp4_operate_t *mp4)
{
    return mp4->mov_array_offset;
}

uint64_t mp4_get_mdat_size(mp4_operate_t *mp4)
{
    return mov_get_mdat_size(mp4->mov);
}

