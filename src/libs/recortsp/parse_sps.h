#ifndef _sps_pps_H_
#define _sps_pps_H_
#include <stdint.h>

#define Extended_SAR 255

typedef struct SPS {
    int profile_idc;
    int constraint_set0_flag;
    int constraint_set1_flag;
    int constraint_set2_flag;
    int constraint_set3_flag;
    int reserved_zero_4bits;
    int level_idc;
    int seq_parameter_set_id;           //ue(v)
    int chroma_format_idc;              //ue(v)
    int separate_colour_plane_flag;     //u(1)
    int bit_depth_luma_minus8;          //0 ue(v)
    int bit_depth_chroma_minus8;        //0 ue(v)
    int qpprime_y_zero_transform_bypass_flag;//0 u(1)
    int seq_scaling_matrix_present_flag; //0 u(1)
    int seq_scaling_list_present_flag[12];
    int UseDefaultScalingMatrix4x4Flag[6];
    int UseDefaultScalingMatrix8x8Flag[6];
    int ScalingList4x4[6][16];
    int ScalingList8x8[6][64];
    int log2_max_frame_num_minus4; //0ue(v)
    int pic_order_cnt_type; //0 ue(v)
    int log2_max_pic_order_cnt_lsb_minus4; //
    int delta_pic_order_always_zero_flag;  //u(1)
    int offset_for_non_ref_pic;            //se(v)
    int offset_for_top_to_bottom_field;    //se(v)
    int num_ref_frames_in_pic_order_cnt_cycle;    //ue(v)
    int offset_for_ref_frame_array[16];           //se(v)
    int num_ref_frames;                           //ue(v)
    int gaps_in_frame_num_value_allowed_flag;    //u(1)
    int pic_width_in_mbs_minus1;                //ue(v)
    int pic_height_in_map_units_minus1;         //u(1)
    int frame_mbs_only_flag;                 //0 u(1)
    int mb_adaptive_frame_field_flag;           //0 u(1)
    int direct_8x8_inference_flag;              //0 u(1)
    int frame_cropping_flag;                    //u(1)
    int frame_crop_left_offset;                //ue(v)
    int frame_crop_right_offset;                //ue(v)
    int frame_crop_top_offset;                  //ue(v)
    int frame_crop_bottom_offset;           //ue(v)
    int vui_parameters_present_flag;            //u(1)
    //vui_parameters_t vui_parameters;
} SPS;

typedef struct get_bit_context {
    uint8_t *buf;         /*指向SPS start*/
    int      buf_size;    /*SPS 长度*/
    int     bit_pos;      /*bit已读取位置*/
    int     total_bit;    /*bit总长度*/
    int     cur_bit_pos;  /*当前读取位置*/
} get_bit_context;

//buf = 0x00 0x00 0x00 0x01 0x67 .......
int h264dec_seq_parameter_set(void *buf, int buf_size, SPS *sps_ptr);

// buf = 0x00 0x00 0x01 0x40 .......
int hevcdec_seq_parameter_set(void *buf, int buf_size, SPS *sps_ptr);

#endif
