#ifndef _MS_PARSE_SPS_H_
#define _MS_PARSE_SPS_H_
#include <stdint.h>

#define DESCRIBE_HEADER_FMT "\
v=0\n\
o=- %llu %llu IN IP4 127.0.0.1\n\
s=MS SERVER\n\
c=IN IP4 %s\n\
t=0 0\n\
a=tool:MS Streaming Media v2021.07.28\n"

#define DESCRIBE_VIDEO_H264_FMT "\
m=video 0 RTP/AVP 96\n\
b=AS:400\n\
a=rtpmap:96 %s/90000\n\
a=fmtp:96 packetization-mode=1; sprop-parameter-sets=%s\n\
a=control:streamid=%d\n"

#define DESCRIBE_VIDEO_H265_FMT "\
m=video 0 RTP/AVP 96\n\
b=AS:400\n\
a=rtpmap:96 %s/90000\n\
a=fmtp:96 profile-space=0;profile-id=1;tier-flag=0;level-id=123;interop-constraints=B00000000000; \
sprop-vps=%s;sprop-sps=%s;sprop-pps=%s\n\
a=control:streamid=%d\n"

#define DESCRIBE_AUDIO_FMT "\
m=audio 0 RTP/AVP %d\n\
b=AS:64\n\
a=recvonly\n\
a=rtpmap:%d %s/%d\n\
a=control:streamid=%d\n"

#define DESCRIBE_AUDIO_AAC_FMT "\
m=audio 0 RTP/AVP %d\n\
b=AS:64\n\
a=recvonly\n\
a=rtpmap:%d %s/%d/1\n\
a=fmtp:97 streamtype=5;profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=%s\n\
a=control:streamid=%d\n"

#define DESCRIBE_METADATA_FMT "\
m=application 0 RTP/AVP 98\n\
b=AS:0\n\
a=rtpmap:98 vnd.onvif.metadata/9000\n\
a=recvonly\n\
a=control:streamid=%d\n"

typedef struct SPS{
	int profile_idc;
	int constraint_set0_flag;
	int constraint_set1_flag;
	int constraint_set2_flag;
	int constraint_set3_flag;
	int reserved_zero_4bits;
	int level_idc;
	int seq_parameter_set_id; //ue(v)
	int chroma_format_idc;//ue(v)
	int separate_colour_plane_flag;//u(1)
	int bit_depth_luma_minus8;//0 ue(v) 
	int bit_depth_chroma_minus8;//0 ue(v) 
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
	int delta_pic_order_always_zero_flag;           //u(1)
	int offset_for_non_ref_pic;                     //se(v)
	int offset_for_top_to_bottom_field;            //se(v)
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
}SPS;

#define HEVC_MAX_SUB_LAYERS 7

typedef struct PTLCommon {
    uint8_t profile_space;
    uint8_t tier_flag;
    uint8_t profile_idc;
    uint8_t profile_compatibility_flag[32];
    uint8_t level_idc;
    uint8_t progressive_source_flag;
    uint8_t interlaced_source_flag;
    uint8_t non_packed_constraint_flag;
    uint8_t frame_only_constraint_flag;
} PTLCommon;

typedef struct PTL {
    PTLCommon general_ptl;
    PTLCommon sub_layer_ptl[HEVC_MAX_SUB_LAYERS];

    uint8_t sub_layer_profile_present_flag[HEVC_MAX_SUB_LAYERS];
    uint8_t sub_layer_level_present_flag[HEVC_MAX_SUB_LAYERS];
} PTL;

typedef struct get_bit_context{
	uint8_t *buf;         /*指向SPS start*/
	int      buf_size;    /*SPS 长度*/
	int     bit_pos;      /*bit已读取位置*/
	int     total_bit;    /*bit总长度*/
	int     cur_bit_pos;  /*当前读取位置*/
}get_bit_context;


//buf = 0x00 0x00 0x00 0x01 0x67 .......
int ms_h264dec_seq_parameter_set(void *buf, int buf_size, SPS *sps_ptr);

// buf = 0x00 0x00 0x01 0x40 .......
int ms_hevcdec_seq_parameter_set(void *buf, int buf_size, SPS *sps_ptr);

int ms_sdp_video_describe(int code, char *data, int bytes, unsigned short port, int payload, int frequence, const void* extra, int extra_size);

#endif
