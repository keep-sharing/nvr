#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "parse_sps.h"

#define MAX_LEN 32

//读1个bit
static int get_1bit(void *h)
{
    if (!h) {
        return -1;
    }

    get_bit_context *ptr = (get_bit_context *)h;
    if ((ptr->bit_pos + 1) > ptr->total_bit) {
        return -1;
    }

    int ret = 0;
    ret = ptr->buf[ptr->bit_pos / 8] & (0x80 >> (ptr->bit_pos % 8));
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

int h264dec_seq_parameter_set(void *buf_ptr, int spslen, SPS *sps_ptr)
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
        (profile_idc == 128)) {
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
        sps->frame_crop_left_offset = parse_ue(buf);
        sps->frame_crop_right_offset = parse_ue(buf);
        sps->frame_crop_top_offset = parse_ue(buf);
        sps->frame_crop_bottom_offset = parse_ue(buf);
    }
    sps->vui_parameters_present_flag = get_1bit(buf);
    if (sps->vui_parameters_present_flag) {
//      vui_parameters_set(buf, &sps->vui_parameters);
    }

    return 0;
}

int hevcdec_seq_parameter_set(void *buf_ptr, int spslen, SPS *sps_ptr)
{
    if (NULL == buf_ptr || NULL == sps_ptr) {
        return -1;
    }
    SPS *sps = sps_ptr;
    memset((void *)sps, 0, sizeof(SPS));

    //int profile_idc = 0;
    //int i,j,last_scale, next_scale, delta_scale;
    int tmp_i = 0;
    get_bit_context tmpctx = {0};
    de_emulation_prevention(buf_ptr, &spslen);

    tmpctx.buf = buf_ptr;
    tmpctx.total_bit = spslen * 8;
    void *buf = &tmpctx;

    get_bits(buf, 32);  //skip 0x00 0x00 0x01
    get_bits(buf, 8);   //skip 0x40

    get_bits(buf, 8);
    get_bits(buf, 8);
    for (tmp_i = 0; tmp_i < 80; tmp_i++) {
        get_bits(buf, 1);
    }

    get_bits(buf, 8);

    parse_ue(buf);
    parse_ue(buf);
    sps->pic_width_in_mbs_minus1 = parse_ue(buf);
    sps->pic_height_in_map_units_minus1 = parse_ue(buf);
    return 0;

}
