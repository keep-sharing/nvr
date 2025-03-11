#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "ms_parse_sps.h"
#include "ms_stream_common.h"
#include "base64.h"

#define MAX_LEN 32

struct ms_extra_t {
    int spsLen;
    int ppsLen;
    int vpsLen;
    int seiLen;
    uint8_t sps[128];
    uint8_t pps[128];
    uint8_t vps[128];
    uint8_t sei[128];
};

//读1个bit
static uint8_t get_1bit(void *h)
{
    if (!h) {
        return -1;
    }

    get_bit_context *ptr = (get_bit_context *)h;
    if ((ptr->bit_pos + 1) > ptr->total_bit) {
        return -1;
    }

    uint8_t ret = 0;
    ret = ptr->buf[ptr->bit_pos / 8] << (ptr->bit_pos % 8);
    ret = ret >> 7;
    ptr->bit_pos += 1;
    return ret;
}

//读n个bits，n不能超过32
static int get_bits(void *h, int n)
{
    if (!h || n < 0 || n > MAX_LEN) {
        return -1;
    }

    get_bit_context *ptr = (get_bit_context *)h;
    if ((ptr->bit_pos + n) > ptr->total_bit) {
        n = ptr->total_bit - ptr->bit_pos;
    }

    int dwRet = 0, i;
    for (i = 0; i < n; i++) {
        dwRet <<= 1;
        if (ptr->buf[ptr->bit_pos / 8] & (0x80 >> (ptr->bit_pos % 8))) {
            dwRet += 1;
        }
        ptr->bit_pos += 1;
    }

    return dwRet;
}

//指数哥伦布编码解析，参考h264标准第9节
static int parse_codenum(void *buf)
{
    uint8_t leading_zero_bits = -1;
    uint8_t b;
    uint32_t code_num = 0;

    for (b = 0; !b; leading_zero_bits++) {
        b = get_1bit(buf);
    }

    code_num = ((uint32_t)1 << leading_zero_bits) - 1 + get_bits(buf, leading_zero_bits);
    return code_num;
}

//指数哥伦布编码解析 ue
static int parse_ue(void *buf)
{
    return parse_codenum(buf);
}

//指数哥伦布编码解析 se
static int parse_se(void *buf)
{
    int ret = 0;
    int code_num;

    code_num = parse_codenum(buf);
    ret = (code_num + 1) >> 1;
    ret = (code_num & 0x01) ? ret : -ret;

    return ret;
}

//查找0x000003, 把0x03字节移除，减小相应spslen长度
static void de_emulation_prevention(char *buf_ptr, int *spslen)
{
    int i, j;
    for (i = 0; i < (*spslen) - 2; i++) {
        if (!buf_ptr[i] && !buf_ptr[i + 1] && buf_ptr[i + 2] == 0x03) {
            for (j = i + 2; j < (*spslen) - 1; j ++) {
                buf_ptr[j] = buf_ptr[j + 1];
            }
            (*spslen) -= 1;
        }
    }
}

int ms_h264dec_seq_parameter_set(void *buf_ptr, int spslen, SPS *sps_ptr)
{
    if (NULL == buf_ptr || NULL == sps_ptr) {
        return -1;
    }
    SPS *sps = sps_ptr;
    memset((void *)sps, 0, sizeof(SPS));

    int profile_idc = 0;
    int i, j, last_scale, next_scale, delta_scale;
    get_bit_context tmpctx = {0};

    de_emulation_prevention(buf_ptr, &spslen);

    tmpctx.buf = buf_ptr;
    tmpctx.total_bit = spslen * 8;
    void *buf = &tmpctx;

    get_bits(buf, 32);  //skip 0x00 0x00 0x00 0x01
    get_bits(buf, 8);   //skip 0x67
    sps->profile_idc          = get_bits(buf, 8);
    sps->constraint_set0_flag = get_1bit(buf);
    sps->constraint_set1_flag = get_1bit(buf);
    sps->constraint_set2_flag = get_1bit(buf);
    sps->constraint_set3_flag = get_1bit(buf);
    sps->reserved_zero_4bits  = get_bits(buf, 4);
    sps->level_idc            = get_bits(buf, 8);
    sps->seq_parameter_set_id = parse_ue(buf);
    profile_idc = sps->profile_idc;
    if ((profile_idc == 100) || (profile_idc == 110) || (profile_idc == 122) || (profile_idc == 244)
        || (profile_idc  == 44) || (profile_idc == 83) || (profile_idc  == 86) || (profile_idc == 118) ||
        (profile_idc == 128) || (profile_idc == 138) || (profile_idc == 144)) {
        sps->chroma_format_idc = parse_ue(buf);
        if (sps->chroma_format_idc == 3) {
            sps->separate_colour_plane_flag = get_1bit(buf);
        }
        sps->bit_depth_luma_minus8 = parse_ue(buf);
        sps->bit_depth_chroma_minus8 = parse_ue(buf);
        sps->qpprime_y_zero_transform_bypass_flag = get_1bit(buf);
        sps->seq_scaling_matrix_present_flag = get_1bit(buf);
        if (sps->seq_scaling_matrix_present_flag) {
            for (i = 0; i < ((sps->chroma_format_idc != 3) ? 8 : 12); i++) {
                sps->seq_scaling_list_present_flag[i] = get_1bit(buf);
                if (sps->seq_scaling_list_present_flag[i]) {
                    if (i < 6) {
                        last_scale = 8;
                        next_scale = 8;
                        for (j = 0; j < 16; j++) {
                            if (next_scale != 0) {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale + 256) % 256;
                                sps->UseDefaultScalingMatrix4x4Flag[i] = ((j == 0) && (next_scale == 0));
                            }
                            sps->ScalingList4x4[i][j] = (next_scale == 0) ? last_scale : next_scale;
                            last_scale = sps->ScalingList4x4[i][j];
                        }
                    } else {
                        int ii = i - 6;
                        next_scale = 8;
                        last_scale = 8;
                        for (j = 0; j < 64; j++) {
                            if (next_scale != 0) {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale + 256) % 256;
                                sps->UseDefaultScalingMatrix8x8Flag[ii] = ((j == 0) && (next_scale == 0));
                            }
                            sps->ScalingList8x8[ii][j] = (next_scale == 0) ? last_scale : next_scale;
                            last_scale = sps->ScalingList8x8[ii][j];
                        }
                    }//if(i<6)
                }//if(sps->seq_scaling_list_present_flag[i])
            }//for(i=0; i<((sps->chroma_format_idc != 3)?8:12); i++)
        }//if(sps->seq_scaling_matrix_present_flag)
    } else {
        sps->chroma_format_idc = 1;
        sps->bit_depth_luma_minus8   = 8;
        sps->bit_depth_chroma_minus8  = 8;
    }
    sps->log2_max_frame_num_minus4 = parse_ue(buf);
    sps->pic_order_cnt_type = parse_ue(buf);
    if (sps->pic_order_cnt_type == 0) {
        sps->log2_max_pic_order_cnt_lsb_minus4 = parse_ue(buf);
    } else if (sps->pic_order_cnt_type == 1) {
        sps->delta_pic_order_always_zero_flag = get_1bit(buf);
        sps->offset_for_non_ref_pic = parse_se(buf);
        sps->offset_for_top_to_bottom_field = parse_se(buf);
        sps->num_ref_frames_in_pic_order_cnt_cycle = parse_ue(buf);
        for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++) {
            sps->offset_for_ref_frame_array[i] = parse_se(buf);
        }
    }
    sps->num_ref_frames = parse_ue(buf);
    sps->gaps_in_frame_num_value_allowed_flag = get_1bit(buf);
    sps->pic_width_in_mbs_minus1 = parse_ue(buf);
    sps->pic_height_in_map_units_minus1 = parse_ue(buf);
    sps->frame_mbs_only_flag = get_1bit(buf);
    if (!sps->frame_mbs_only_flag) {
        sps->mb_adaptive_frame_field_flag = get_1bit(buf);
    }
    sps->direct_8x8_inference_flag = get_1bit(buf);
    sps->frame_cropping_flag = get_1bit(buf);
    if (sps->frame_cropping_flag) {
        int vsub   = (sps->chroma_format_idc == 1) ? 1 : 0;
        int hsub   = (sps->chroma_format_idc == 1 ||
                      sps->chroma_format_idc == 2) ? 1 : 0;
        int step_x = 1 << hsub;
        int step_y = (2 - sps->frame_mbs_only_flag) << vsub;
        sps->frame_crop_left_offset = parse_ue(buf);
        sps->frame_crop_right_offset = parse_ue(buf);
        sps->frame_crop_top_offset = parse_ue(buf);
        sps->frame_crop_bottom_offset = parse_ue(buf);
        sps->frame_crop_left_offset   = sps->frame_crop_left_offset   * step_x;
        sps->frame_crop_right_offset  = sps->frame_crop_right_offset  * step_x;
        sps->frame_crop_top_offset    = sps->frame_crop_top_offset    * step_y;
        sps->frame_crop_bottom_offset = sps->frame_crop_bottom_offset * step_y;

    }
    sps->vui_parameters_present_flag = get_1bit(buf);
    if (sps->vui_parameters_present_flag) {
//      vui_parameters_set(buf, &sps->vui_parameters);
    }

    return 0;
}

int decode_profile_tier_level(void *ptr)
{
    int i = 0;
    //just skip the buffer
    get_bits(ptr, 8); // profile_space 2bit, tier_flag,1bit, profile_idc
    //decode_profile_tier_level left value 80 bits
    for (i = 0; i < 10; i++) {
        get_bits(ptr, 8);
    }
    return 0;
}

int ms_hevcdec_seq_parameter_set(void *buf_ptr, int spslen, SPS *sps_ptr)
{
    if (NULL == buf_ptr || NULL == sps_ptr) {
        return -1;
    }
    SPS *sps = sps_ptr;
    memset((void *)sps, 0, sizeof(SPS));
    PTL ptl = {{0}};
    int tmp_i = 0;
    int max_sub_layers = 0;
    get_bit_context tmpctx = {0};
    de_emulation_prevention(buf_ptr, &spslen);

    tmpctx.buf = buf_ptr;
    tmpctx.total_bit = spslen * 8;
    void *buf = &tmpctx;

    get_bits(buf, 32);  //skip 0x00 0x00 0x01
    get_bits(buf, 8);   //skip 0x40

    get_bits(buf, 4); //vps_id first 4 bit ,
    max_sub_layers = get_bits(buf, 3) + 1; //max_sub_layers (3bit +1):usually 1
    get_bits(buf, 1); // temporal_id_nesting_flag 1bit
    //parse_ptl
    // first decode_profile_tier_level
    decode_profile_tier_level(buf);

    get_bits(buf, 8); // ptl general_ptl.level_idc
    //sub_layer_profile_present_flag sub_layer_level_present_flag
    for (tmp_i = 0; tmp_i < max_sub_layers - 1; tmp_i++) {
        ptl.sub_layer_profile_present_flag[tmp_i] = get_bits(buf, 1);
        ptl.sub_layer_level_present_flag[tmp_i] = get_bits(buf, 1);
    }

    if (max_sub_layers - 1 > 0)
        for (tmp_i = max_sub_layers - 1; tmp_i < 8; tmp_i++) {
            get_bits(buf, 2);    // reserved_zero_2bits[i]
        }
    for (tmp_i = 0; tmp_i < max_sub_layers - 1; tmp_i++) {
        if (ptl.sub_layer_profile_present_flag[tmp_i] &&
            decode_profile_tier_level(buf) < 0) {
            return -1;
        }
        if (ptl.sub_layer_level_present_flag[tmp_i]) {
            ptl.sub_layer_ptl[tmp_i].level_idc = get_bits(buf, 8);
        }
    }
    //parse_ptl end
    parse_ue(buf); // sps_id
    sps->chroma_format_idc = parse_ue(buf);
    if (sps->chroma_format_idc == 3) { // chroma_format_idc
        get_bits(buf, 1);
    }
    sps->pic_width_in_mbs_minus1 = parse_ue(buf);
    sps->pic_height_in_map_units_minus1 = parse_ue(buf);
    sps->frame_cropping_flag = get_bits(buf, 1);
    if (sps->frame_cropping_flag) {
        int vsub   = (sps->chroma_format_idc == 1) ? 1 : 0;
        int hsub   = (sps->chroma_format_idc == 1 ||
                      sps->chroma_format_idc == 2) ? 1 : 0;
        int step_x = 1 << hsub;
        int step_y = 1 << vsub;
        sps->frame_crop_left_offset = parse_ue(buf);
        sps->frame_crop_right_offset = parse_ue(buf);
        sps->frame_crop_top_offset = parse_ue(buf);
        sps->frame_crop_bottom_offset = parse_ue(buf);
        sps->frame_crop_left_offset   = sps->frame_crop_left_offset   * step_x;
        sps->frame_crop_right_offset  = sps->frame_crop_right_offset  * step_x;
        sps->frame_crop_top_offset    = sps->frame_crop_top_offset    * step_y;
        sps->frame_crop_bottom_offset = sps->frame_crop_bottom_offset * step_y;
    }

    return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////
static int ms_transfer_extra_data(int codeType, const uint8_t *src, int nLen, struct ms_extra_t *param)
{
    if (!src || !param || nLen < 7) {
        return -1;
    }
    if (codeType != MS_StreamCodecType_H264 && codeType != MS_StreamCodecType_H265) { /*264 & 265*/
        return -1;
    }
    int i = 0, n = 0, flagCnt = 0;;
    char startcode[4] = {0x00, 0x00, 0x00, 0x01};
#if 0
    printf("========================\n");
    for (i = 0; i < 512 && i < nLen; i++) {
        if (i % 63 == 0 && i != 0) {
            printf("%x\n", src[i]);
        } else {
            printf("%x ", src[i]);
        }
    }
    printf("\n========================\n");
#endif

    for (i = 0; i < nLen; i++) {
        if (i + 4 < nLen) {
            if (src[i] == startcode[0] && src[i + 1] == startcode[1] &&
                src[i + 2] == startcode[2] && src[i + 3] == startcode[3]) {
                if ((codeType == MS_StreamCodecType_H264 && (src[i + 4] == 0x67 || src[i + 4] == 0x68 || src[i + 4] == 0x06))
                    || (codeType == MS_StreamCodecType_H265 && (src[i + 4] == 0x40 || src[i + 4] == 0x42 || src[i + 4] == 0x44 ||
                                                                src[i + 4] == 0x4E || src[i + 4] == 0x26 || src[i + 4] == 0x02))) {
                    i += 3;
                    n = 0;
                    flagCnt++;
                    continue;
                }
            }
        }

        if (n >= 128) {
            n++;
            continue;
        }
        if (codeType == MS_StreamCodecType_H264) {/*H264*/
            switch (flagCnt) {
                case 1:
                    param->sps[n] = src[i];
                    param->spsLen = n + 1;
                    break;
                case 2:
                    param->pps[n] = src[i];
                    param->ppsLen = n + 1;
                    break;
                case 3:
                    param->sei[n] = src[i];
                    param->seiLen = n + 1;
                    break;
                default:
                    break;
            }
        } else {
            switch (flagCnt) {
                case 1:
                    param->vps[n] = src[i];
                    param->vpsLen = n + 1;
                    break;
                case 2:
                    param->sps[n] = src[i];
                    param->spsLen = n + 1;
                    break;
                case 3:
                    param->pps[n] = src[i];
                    param->ppsLen = n + 1;
                    break;
                case 4:
                    param->sei[n] = src[i];
                    param->seiLen = n + 1;
                    break;
                default:
                    break;
            }
        }
        n++;
    }

    return n;
}

static int ms_sdp_h264(char *data, int bytes, unsigned short port, int payload, int frequence, const void *extra,
                       int extra_size)
{
    int n = 0;
    char pSpropPsets[768] = {0};
    struct ms_extra_t pExtra;

    memset(&pExtra, 0, sizeof(struct ms_extra_t));
    ms_transfer_extra_data(MS_StreamCodecType_H264, (const uint8_t *)extra, extra_size, &pExtra);
    if (pExtra.spsLen) {
        n = base64_encode((char *)pSpropPsets + n, pExtra.sps, pExtra.spsLen);
    }
    if (pExtra.ppsLen) {
        pSpropPsets[n++] = ',';
        n = base64_encode((char *)pSpropPsets + n, pExtra.pps, pExtra.ppsLen);
    }
    if (pExtra.seiLen) {
        pSpropPsets[n++] = ',';
        n = base64_encode((char *)pSpropPsets + n, pExtra.sei, pExtra.seiLen);
    }

    snprintf(data, bytes, DESCRIBE_VIDEO_H264_FMT, "H264", pSpropPsets, 0);

    return 0;
}

static int ms_sdp_h265(char *data, int bytes, unsigned short port, int payload, int frequence, const void *extra,
                       int extra_size)
{
    char pVpsStr[256] = {0};
    char pSpsStr[256] = {0};
    char pPpsStr[256] = {0};
    struct ms_extra_t pExtra;

    memset(&pExtra, 0, sizeof(struct ms_extra_t));
    ms_transfer_extra_data(MS_StreamCodecType_H265, (const uint8_t *)extra, extra_size, &pExtra);
    if (pExtra.vpsLen) {
        base64_encode((char *)pVpsStr, pExtra.vps, pExtra.vpsLen);
    }
    if (pExtra.spsLen) {
        base64_encode((char *)pSpsStr, pExtra.sps, pExtra.spsLen);
    }
    if (pExtra.ppsLen) {
        base64_encode((char *)pPpsStr, pExtra.pps, pExtra.ppsLen);
    }

    snprintf(data, bytes, DESCRIBE_VIDEO_H265_FMT, "H265", pVpsStr, pSpsStr, pPpsStr, 0);

    return 0;
}

int ms_sdp_video_describe(int code, char *data, int bytes, unsigned short port, int payload, int frequence,
                    const void *extra, int extra_size)
{
    if (code == MS_StreamCodecType_H264) {
        return ms_sdp_h264(data, bytes, port, payload, frequence, extra, extra_size);
    }

    if (code == MS_StreamCodecType_H265) {
        return ms_sdp_h265(data, bytes, port, payload, frequence, extra, extra_size);
    }

    return 0;
}
