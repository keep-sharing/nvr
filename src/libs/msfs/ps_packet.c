/*
 * ***************************************************************
 * Filename:        ps_packet.h
 * Created at:      2017.12.27
 * Description:     ps stream <-> h264/h265.
 * Author:          wink
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include "ps_packet.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "mf_type.h"

struct ps_custom_s {
    MF_S32 type;
    MF_S32 size;
    MF_S8 *data;
};

static PS_ALLOC g_ps_alloc = NULL;
static PS_FREE g_ps_free = NULL;

/***
 *@remark:  讲传入的数据按地位一个一个的压入数据
 *@param :  buffer   [in]  压入数据的buffer
 *          count    [in]  需要压入数据占的位数
 *          bits     [in]  压入的数值
 */

#define bits_write(buffer, count, bits)\
    do\
    {\
        bits_buffer_t *p_buffer = (buffer);\
        MF_S32 i_count = (count);\
        MF_U64 i_bits = (bits);\
        while( i_count > 0 )\
        {\
            i_count--;\
            if( ( i_bits >> i_count )&0x01 )\
            {\
                p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;\
            }\
            else\
            {\
                p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;\
            }\
            p_buffer->i_mask >>= 1;\
            if( p_buffer->i_mask == 0 )\
            {\
                p_buffer->i_data++;\
                p_buffer->i_mask = 0x80;\
            }\
        }\
    }while(0)
#if 0
static inline MF_S32
bits_write(bits_buffer_t *buffer, MF_S32 count, MF_S32 bits)
{
    bits_buffer_t *p_buffer = (buffer);
    MF_S32 i_count = (count);
    MF_U64 i_bits = (bits);
    while (i_count > 0) {
        i_count--;
        if ((i_bits >> i_count) & 0x01) {
            p_buffer->p_data[p_buffer->i_data] |= p_buffer->i_mask;
        } else {
            p_buffer->p_data[p_buffer->i_data] &= ~p_buffer->i_mask;
        }
        p_buffer->i_mask >>= 1;         /*操作完一个字节第一位后，操作第二位*/
        if (p_buffer->i_mask == 0) {    /*循环完一个字节的8位后，重新开始下一位*/
            p_buffer->i_data++;
            p_buffer->i_mask = 0x80;
        }
    }
    return 0;
}
#endif


static int nalu_check(frame_para_t *para_info)
{
    int ret = MF_NO;
    int nalu_type;
    int i;
    if (para_info->nalu_para[0].count == 0) {
        if (para_info->codec_type == PS_CODEC_H264) {
            nalu_type = para_info->nalu_para[para_info->nalu_cnt - 1].type;
            nalu_type &= 0x1f;
            if (nalu_type == 0x05  && para_info->nalu_cnt > 1) {
                ret = MF_YES;
            } else if (nalu_type == 0x01 /*&& para_info->nalu_cnt == 1*/) {
                ret = MF_YES;
            }
        } else if (para_info->codec_type == PS_CODEC_H265) {
            nalu_type = para_info->nalu_para[para_info->nalu_cnt - 1].type;
            nalu_type = (nalu_type & 0x7e) >> 1;
            if (nalu_type == 0x13 && para_info->nalu_cnt > 1) {
                ret = MF_YES;
            } else if (nalu_type == 0x01 /*&& para_info->nalu_cnt == 1*/) {
                ret = MF_YES;
            }
        }
    }

    for (i = 0; i < para_info->nalu_cnt - 1; i++) {
        switch (para_info->nalu_para[i].type) {
            case H264_NALU_PPS:
            case H264_NALU_SPS:
            case H264_NALU_SEI:
            case H265_NALU_PPS:
            case H265_NALU_SPS:
            case H265_NALU_VPS:
            case H265_NALU_SEI:
                if (para_info->nalu_para[i + 1].count - para_info->nalu_para[i].count > 65535 - PES_HDR_LEN) {
                    ret = MF_NO;
                }
                break;
            default:
                break;
        }
    }

    return ret;

}

static MF_S32
packet_get_length(void *data, MF_S32 frame_length, frame_para_t *para_info)
{
    MF_S32 data_length = 0;
    MF_S8 *p_tmp = (MF_S8 *) data;
    MF_S32 b_run_flag = MF_YES;
    MF_S32 stream_id ;
    MF_S32 count = 0;
    MF_S32 nalu_header_size = 0;
    MF_S32 packet_cnt = 0;

    if (frame_length <= 0) {
        MFerr("frame_length is (%d)\n", frame_length);
        return 0;
    }

    if (para_info->stream_type == AUDIO_TYPE || para_info->stream_type == ST_TYPE) {
        // add a pes_header to every raw_data,the length of pes_packet is up to  65535
        packet_cnt = frame_length / PS_PES_PAYLOAD_SIZE;
        if (frame_length % PS_PES_PAYLOAD_SIZE != 0) {
            packet_cnt++;
        }
    } else {
        // add a pes_header to every nalu packet,
        while (b_run_flag == MF_YES && count < frame_length - 5) {
            stream_id = *(MF_S32 *)p_tmp;
            if ((stream_id & 0x00ffffff) == 0x00010000) {
                nalu_header_size = 3;
            } else if (stream_id == 0x01000000) {
                nalu_header_size = 4;
            } else {
                p_tmp ++;
                count ++;
                continue;
            }

            para_info->nalu_para[para_info->nalu_cnt++].count = count;
            p_tmp += nalu_header_size;
            count += nalu_header_size;

            stream_id = *p_tmp;
            para_info->nalu_para[para_info->nalu_cnt - 1].type = stream_id;
            if (para_info->codec_type == PS_CODEC_H264) {
                stream_id = stream_id & 0x1f;
                if (stream_id == 0x01 || stream_id == 0x05) {
                    b_run_flag = MF_NO;
                }
            } else if (para_info->codec_type == PS_CODEC_H265) {
                stream_id = (stream_id & 0x7e) >> 1;
                if (stream_id == 0x13 || stream_id == 0x01) {
                    b_run_flag = MF_NO;
                }
            }

            if (para_info->nalu_cnt > 32) {
                b_run_flag  = MF_YES;
                MFerr("nalu_cnt is too many (maybe packet loss)\n");
                break;
            }
        }
        if (b_run_flag == MF_YES || nalu_check(para_info) == MF_NO) {
            MFerr("nalu_check is No b_run_flag(%d) nalu_cnt(%d)\n", b_run_flag, para_info->nalu_cnt);
            return 0;
        }
        para_info->nalu_cnt--;
        // add a pes_header to every raw_data,the length of pes_packet is up to  65535
        packet_cnt = (frame_length - count + nalu_header_size) / PS_PES_PAYLOAD_SIZE;
        if ((frame_length - count + nalu_header_size) % PS_PES_PAYLOAD_SIZE != 0) {
            packet_cnt++;
        }
    }
    packet_cnt += para_info->nalu_cnt;
    if (packet_cnt != 0) {
        data_length = PS_HDR_LEN + PES_HDR_LEN * packet_cnt + SYS_HDR_LEN + PSM_HDR_LEN + frame_length + sizeof(
                          struct mf_frame);
    }
    return data_length;
}



/***
 *@remark:   ps header
 *@param :   pData  [in] the data that return
 *           timestamp_us [in]  timestamp on video
 *@return:   length of data
*/
static MF_S32
packet_header(MF_S8 *pData, MF_U64 timestamp_us, MF_S32 packet_length, MF_S32 frame_length)
{
    MF_U64 lScrbasc = timestamp_us / 1000 * 90; // timestamp  unit is 90khz
    MF_U64 lScrextra = 0;//(timestamp_us*27)%300; // timestamp  unit is 27Mhz

    bits_buffer_t   bitsBuffer;
    bitsBuffer.i_size = PS_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (MF_U8 *)(pData);
    memset(bitsBuffer.p_data, 0, PS_HDR_LEN);
    bits_write(&bitsBuffer, 32, 0x000001BA);            /*start codes*/
    bits_write(&bitsBuffer, 2,  1);                     /*marker bits '01b'*/
    bits_write(&bitsBuffer, 3, (lScrbasc >> 30) & 0x07); /*System clock [32..30]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 15, (lScrbasc >> 15) & 0x7FFF); /*System clock [29..15]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 15, (lScrbasc) & 0x7fff);       /*System clock [29..15]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 9, (lScrextra) & 0x01ff);       /*System clock [14..0]*/
    bits_write(&bitsBuffer, 1,  1);                     /*marker bit*/
    bits_write(&bitsBuffer, 22, (255) & 0x3fffff);      /*bit rate(n units of 50 bytes per second.)*/
    bits_write(&bitsBuffer, 2,  3);                     /*marker bits '11'*/
    bits_write(&bitsBuffer, 5,  0x1f);                  /*reserved(reserved for future use)*/
    bits_write(&bitsBuffer, 3,  0x6);                   /*stuffing length,can */
    bits_write(&bitsBuffer, 24, packet_length & 0x00ffffff);        /*the length of ps_packet */
    bits_write(&bitsBuffer, 24, frame_length & 0x00ffffff);         /*the length of frame_raw  */
    return PS_HDR_LEN;
}

/***
 *@remark:   packet_sys_header
 *@param :   pData  [in] the buf of header
 *@return:   0 success, others failed
*/
static MF_S32
packet_sys_header(MF_S8 *pData)
{
    bits_buffer_t   bitsBuffer;
    MF_S32 length;
    length = SYS_HDR_LEN;
    bitsBuffer.i_size = length;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (MF_U8 *)(pData);
    memset(bitsBuffer.p_data, 0, length);
    /*system header*/
    bits_write(&bitsBuffer, 32, 0x000001BB);     /*start code*/
    bits_write(&bitsBuffer, 16, length - 6); /*header_length 表示次字节后面的长度，后面的相关头也是次意思*/
    bits_write(&bitsBuffer, 1,  1);          /*marker_bit*/
    bits_write(&bitsBuffer, 22, 50000);      /*rate_bound*/
    bits_write(&bitsBuffer, 1,  1);          /*marker_bit*/
    bits_write(&bitsBuffer, 6,  1);          /*audio_bound*/
    bits_write(&bitsBuffer, 1,  0);          /*fixed_flag */
    bits_write(&bitsBuffer, 1,  1);          /*CSPS_flag */
    bits_write(&bitsBuffer, 1,  1);          /*system_audio_lock_flag*/
    bits_write(&bitsBuffer, 1,  1);          /*system_video_lock_flag*/
    bits_write(&bitsBuffer, 1,  1);          /*marker_bit*/
    bits_write(&bitsBuffer, 5,  1);          /*video_bound*/
    bits_write(&bitsBuffer, 1,  0);          /*dif from mpeg1*/
    bits_write(&bitsBuffer, 7,  0x7F);       /*reserver*/
    /*video stream bound*/
    bits_write(&bitsBuffer, 8,  0xE0);       /*stream_id*/
    bits_write(&bitsBuffer, 2,  3);          /*marker_bit */
    bits_write(&bitsBuffer, 1,  1);          /*PSTD_buffer_bound_scale*/
    bits_write(&bitsBuffer, 13, 2048);       /*PSTD_buffer_size_bound*/
    /*audio stream bound*/
    bits_write(&bitsBuffer, 8,  0xC0);       /*stream_id*/
    bits_write(&bitsBuffer, 2,  3);          /*marker_bit */
    bits_write(&bitsBuffer, 1,  0);          /*PSTD_buffer_bound_scale*/
    bits_write(&bitsBuffer, 13, 512);            /*PSTD_buffer_size_bound*/

    return length;
}

/***
 *@remark:   psm头的封装,里面的具体数据的填写已经占位，可以参考标准
 *@param :   pData  [in] 填充ps头数据的地址
 *@return:   0 success, others failed
*/
static MF_S32
packet_psm(MF_S8 *pData, MF_S32 type, MF_S8 *private_info)
{
    bits_buffer_t   bitsBuffer;
    MF_S32 private_length;

    private_length = sizeof(struct mf_frame);
    bitsBuffer.i_size = PSM_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (MF_U8 *)(pData);
    memset(bitsBuffer.p_data, 0, PSM_HDR_LEN);
    bits_write(&bitsBuffer, 24, 0x000001);  /*start code*/
    bits_write(&bitsBuffer, 8, 0xBC);       /*map stream id*/
    bits_write(&bitsBuffer, 16, 24 + private_length);   /*program stream map length*/
    bits_write(&bitsBuffer, 1, 1);          /*current next indicator */
    bits_write(&bitsBuffer, 2, 3);          /*reserved*/
    bits_write(&bitsBuffer, 5, 0);          /*program stream map version*/
    bits_write(&bitsBuffer, 7, 0x7F);       /*reserved */
    bits_write(&bitsBuffer, 1, 1);          /*marker bit */
    bits_write(&bitsBuffer, 16, private_length + 6);        /*programe stream info length*/

    bits_write(&bitsBuffer, 8, 0x05);       /*description_tag*/
    bits_write(&bitsBuffer, 8, private_length + 4);     /*description_length*/
    bits_write(&bitsBuffer, 32, 0x00);      /*marker bit*/

    /*private info*/
    memcpy(bitsBuffer.p_data + bitsBuffer.i_data, private_info, private_length);
    bitsBuffer.i_data += private_length;

    bits_write(&bitsBuffer, 16, 0);         /*elementary stream map length  is*/

    /*video*/
    if (type == PS_CODEC_H265) {
        bits_write(&bitsBuffer, 8, 0x24);    /*stream_type*/
    } else {
        bits_write(&bitsBuffer, 8, 0x1B);    /*stream_type*/
    }
    bits_write(&bitsBuffer, 8, 0xE0);       /*elementary_stream_id*/
    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length */
    /*audio*/
    if (type == AUDIO_CODEC_AAC) {
        bits_write(&bitsBuffer, 8, 0x0f);    /*stream_type*/
    } else if (type == AUDIO_CODEC_G711_MULAW) {
        bits_write(&bitsBuffer, 8, 0x91);    /*stream_type*/
    } else { //if (type == AUDIO_CODEC_G711_ALAW)
        bits_write(&bitsBuffer, 8, 0x90);    /*stream_type*/
    }
    bits_write(&bitsBuffer, 8, 0xC0);       /*elementary_stream_id*/
    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length is*/

    /*crc */
    bits_write(&bitsBuffer, 8, 0xfe);       /*crc (24~31) bits*/
    bits_write(&bitsBuffer, 8, 0xbf);       /*crc (16~23) bits*/
    bits_write(&bitsBuffer, 8, 0xb1);       /*crc (8~15) bits*/
    bits_write(&bitsBuffer, 8, 0xd7);       /*crc (0~7) bits*/


    return PSM_HDR_LEN + private_length;
}


static MF_S32
packet_pes_header(MF_S8 *pData, MF_S32 stream_id, MF_S32 payload_len, MF_U64 pts_us)
{

    bits_buffer_t   bitsBuffer;
    MF_U64 pts_90khz;
    pts_90khz = pts_us / 1000 * 90;
    bitsBuffer.i_size = PES_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (MF_U8 *)(pData);
    memset(bitsBuffer.p_data, 0, PES_HDR_LEN);
    /*system header*/
    bits_write(&bitsBuffer, 24, 0x000001);   /*start code*/
    bits_write(&bitsBuffer, 8, (stream_id));     /*streamID*/
    bits_write(&bitsBuffer, 16, (payload_len) + 8);  /*packet_len*/ //指出pes分组中数据长度和该字节后的长度和
    bits_write(&bitsBuffer, 2, 2);       /*'10'*/
    bits_write(&bitsBuffer, 2, 0);       /*scrambling_control*/
    bits_write(&bitsBuffer, 1, 0);       /*priority*/
    bits_write(&bitsBuffer, 1, 0);       /*data_alignment_indicator*/
    bits_write(&bitsBuffer, 1, 0);       /*copyright*/
    bits_write(&bitsBuffer, 1, 0);       /*original_or_copy*/
    bits_write(&bitsBuffer, 2, 2);       /*PTS_flag and DTS_flag*/
    bits_write(&bitsBuffer, 1, 0);       /*ESCR_flag*/
    bits_write(&bitsBuffer, 1, 0);       /*ES_rate_flag*/
    bits_write(&bitsBuffer, 1, 0);       /*DSM_trick_mode_flag*/
    bits_write(&bitsBuffer, 1, 0);       /*additional_copy_info_flag*/
    bits_write(&bitsBuffer, 1, 0);       /*PES_CRC_flag*/
    bits_write(&bitsBuffer, 1, 0);       /*PES_extension_flag*/
    bits_write(&bitsBuffer, 8, 5);       /*header_data_length*/


    /*PTS,DTS*/
    bits_write(&bitsBuffer, 4, 2);                   /*'0010'*/
    bits_write(&bitsBuffer, 3, ((pts_90khz) >> 30) & 0x07);  /*PTS[32..30]*/
    bits_write(&bitsBuffer, 1, 1);
    bits_write(&bitsBuffer, 15, ((pts_90khz) >> 15) & 0x7FFF); /*PTS[29..15]*/
    bits_write(&bitsBuffer, 1, 1);
    bits_write(&bitsBuffer, 15, (pts_90khz) & 0x7FFF);       /*PTS[14..0]*/
    bits_write(&bitsBuffer, 1, 1);
    return PES_HDR_LEN;
}


MF_S32
ps_packet_get_info(MF_S8 *data, MF_U32 size, struct ps_frame_s *frame_info)
{
    MF_S8 *pdata_raw = data;
    MF_S32 stream_id;

    if (size < PS_HDR_LEN) {
        MFerr("size < PS_HDR_LEN %d\r\n", size);
        return PS_ERR_OVERFLOW;
    }

    stream_id = *(MF_S32 *)(pdata_raw);

    if (stream_id != 0xba010000) {
        MFerr("not a header of frame[size:%d, stream_id:%#x]\r\n", size, stream_id);
        return PS_ERR_BAD_FRAME;
    } else {
        frame_info->packet_size = *(pdata_raw + PS_HDR_LEN - 6) << 16 | *(pdata_raw + PS_HDR_LEN - 5) << 8 | *
                                  (pdata_raw + PS_HDR_LEN - 4);
        frame_info->frame_size = *(pdata_raw + PS_HDR_LEN - 3) << 16 | *(pdata_raw + PS_HDR_LEN - 2) << 8 | *
                                 (pdata_raw + PS_HDR_LEN - 1);
        frame_info->data = data;
    }
    return MF_SUCCESS;
}

//decode a frame
MF_S32
ps_packet_decoder(struct ps_frame_s *packet_info, struct mf_frame *frame_info)
{
    struct mf_frame *f = frame_info;
    MF_S32 pes_length, psm_length;
    MF_S32 frame_length = packet_info->frame_size;
    MF_S32 packet_length = packet_info->packet_size;
    MF_S32 stream_id;
    MF_S32 ret = MF_SUCCESS;
    MF_S8 *pdata_raw = packet_info->data;
    MF_S8 *pdata_dst = f->data;
    MF_S8 *pdata_tmp = f->data;

    while (packet_length !=  0 &&  frame_length != 0) {
        stream_id = *(MF_S32 *)(pdata_raw);

        if ((stream_id & 0x00ffffff) != 0x00010000) {
            MFerr("data format is error (%d %d 0x%x)\r\n", packet_length, frame_length, stream_id);
            ret = MF_FAILURE;
            break;
        }

        switch (stream_id) {
            case 0xba010000:
                pdata_raw += PS_HDR_LEN;
                packet_length -= PS_HDR_LEN;
                break;
            case 0xbc010000:
                psm_length = (*(pdata_raw + 4)) * 256   + (*(pdata_raw + 5)) + 6;
                pdata_raw += 16;
                memcpy(frame_info, pdata_raw, sizeof(struct mf_frame));
                f->data = pdata_tmp;
                pdata_raw += psm_length - 16;
                packet_length -= psm_length;
                break;
            case 0xbb010000:
                pdata_raw += SYS_HDR_LEN;
                packet_length -= SYS_HDR_LEN;
                break;
            case 0xe0010000:
            case 0xc0010000:
                pes_length = (*(pdata_raw + 4)) * 256   + (*(pdata_raw + 5)) - 8;
                pdata_raw += PES_HDR_LEN;
                packet_length -= PES_HDR_LEN;
                memcpy(pdata_dst, pdata_raw, sizeof(MF_S8)*pes_length);
                pdata_raw += pes_length;
                frame_length -= pes_length;
                packet_length -= pes_length;
                pdata_dst += pes_length;
                break;
            default:
                break;
        }
    }

    return ret;
}

MF_S32
ps_packet_encoder(struct mf_frame *frame_info, struct ps_frame_s *ps_info)
{
    struct mf_frame *f = frame_info;
    MF_S32 data_length;
    MF_S32 raw_length = f->size;
    MF_S32 pes_length;
    MF_S32 stuff_length;
    MF_S32 i;
    MF_S32 ret;
    frame_para_t para_info;
    MF_S8 *pdata_raw = (MF_S8 *) f->data;
    MF_S8 *pdata_dst;

    ps_info->data = NULL;
    memset(&para_info, 0x0, sizeof(frame_para_t));
    para_info.codec_type = f->codec_type;
    para_info.stream_type = f->strm_type;
    data_length = packet_get_length(pdata_raw, f->size, &para_info);
    if (data_length == 0) {
        MFerr("no find the header of frame or not raw data(size %d ch:%d stream:%d)\r\n", f->size, frame_info->ch, frame_info->stream_format);
        return MF_FAILURE;
    }
    //align data to 512
    stuff_length = 512 - (data_length % 512);
    data_length += stuff_length;

    ps_info->packet_size = data_length;
    ps_info->frame_size = f->size;
    ps_info->data = (void *) g_ps_alloc(sizeof(MF_S8) * data_length);
    memset(ps_info->data, 0, sizeof(MF_S8)*data_length);
    pdata_dst = (MF_S8 *)ps_info->data;
    if (ps_info->data == NULL) {
        MFerr("malloc failed. ch:%d stream:%d size:%d\r\n", frame_info->ch, frame_info->stream_format, data_length);
        return MF_FAILURE;
    }

    //add ps_header
    ret = packet_header(pdata_dst, f->time_usec, data_length, raw_length);
    pdata_dst += ret;
    data_length -= ret;

    //add sys_header
    ret = packet_sys_header(pdata_dst);
    pdata_dst += ret;
    data_length -= ret;

    //add psm_header
    ret = packet_psm(pdata_dst, f->codec_type, (MF_S8 *)frame_info);
    pdata_dst += ret;
    data_length -= ret;

    //save raw_data
    if (f->strm_type == VIDEO_TYPE) {

        for (i = 0; i < para_info.nalu_cnt; i++) {
            pes_length = para_info.nalu_para[i + 1].count - para_info.nalu_para[i].count;
            packet_pes_header(pdata_dst, PS_VIDEO_STREAM, pes_length, f->time_usec);
            pdata_dst += PES_HDR_LEN;
            data_length -= PES_HDR_LEN;
            memcpy(pdata_dst, pdata_raw, pes_length);
            pdata_dst += pes_length;
            pdata_raw += pes_length;
            data_length -= pes_length;
            raw_length -= pes_length;
        }


        while (raw_length > 0) {
            pes_length = (raw_length > PS_PES_PAYLOAD_SIZE) ? PS_PES_PAYLOAD_SIZE : raw_length;
            packet_pes_header(pdata_dst, PS_VIDEO_STREAM, pes_length, f->time_usec);
            pdata_dst += PES_HDR_LEN;
            data_length -= PES_HDR_LEN;
            memcpy(pdata_dst, pdata_raw, sizeof(MF_S8)*pes_length);
            pdata_dst += pes_length;
            pdata_raw += pes_length;
            data_length -= pes_length;
            raw_length -= pes_length;
        }

    } else if (f->strm_type == AUDIO_TYPE || f->strm_type == ST_TYPE) {
        while (raw_length != 0) {
            pes_length = (raw_length > PS_PES_PAYLOAD_SIZE) ? PS_PES_PAYLOAD_SIZE : raw_length;
            packet_pes_header(pdata_dst, PS_AUDIO_STREAM, pes_length, f->time_usec);
            pdata_dst += PES_HDR_LEN;
            data_length -= PES_HDR_LEN;
            memcpy(pdata_dst, pdata_raw, sizeof(MF_S8)*pes_length);
            pdata_dst += pes_length;
            pdata_raw += pes_length;
            data_length -= pes_length;
            raw_length -= pes_length;
        }
    }

    // padding data with 0xff
    if (stuff_length != 0) {
        memset(pdata_dst, 0xff, stuff_length);
    }
    data_length -= stuff_length;

    //check the data if right
    if (data_length != 0) {
        MFerr("errro data:data_length[%d],frame_length[%d] ch:%d stream:%d\r\n", data_length, f->size, frame_info->ch, frame_info->stream_format);
        for (i = 0; i < para_info.nalu_cnt + 1; i++) {
            MFerr("%d:type[0x%x] dataoff[%d]\n", i, para_info.nalu_para[i].type, para_info.nalu_para[i].count);

        }

        ps_packet_encoder_free(ps_info);
        return MF_FAILURE;

    }

    return MF_SUCCESS;
}


MF_S32
ps_packet_encoder_free(struct ps_frame_s *ps_info)
{
    if (ps_info->data != NULL) {
        g_ps_free(ps_info->data);
    }
    ps_info->data = NULL;
    return MF_SUCCESS;
}

void
ps_packet_cb(PS_ALLOC ps_alloc, PS_FREE ps_free)
{
    if (ps_alloc == NULL || ps_free == NULL) {
        MFerr("ps_alloc or ps_free must not be NULL");
        return;
    }
    g_ps_alloc = ps_alloc;
    g_ps_free  = ps_free;
}

MF_S32
ps_packet_custom(MF_S8 *data, MF_S32 raw_length, struct ps_frame_s *ps_info)
{
    MF_S32 pes_length;
    MF_S32 ret;
    frame_para_t para_info;
    MF_S8 *pdata_dst;
    MF_S8 *pdata_raw = data;
    struct ps_custom_s custom;
    MF_S32 private_length = sizeof(struct ps_custom_s);

    ps_info->data = NULL;
    memset(&para_info, 0x0, sizeof(frame_para_t));
    //para_info.codec_type = f->codec_type;
    //para_info.stream_type = f->strm_type;

    if (raw_length == 0) {
        MFerr("no find the header of frame or not raw data(size %d)\r\n", raw_length);
        return MF_FAILURE;
    }

    MF_S32 packet_cnt = raw_length / PS_PES_PAYLOAD_SIZE;
    if (raw_length % PS_PES_PAYLOAD_SIZE != 0) {
        packet_cnt++;
    }
    MF_S32 data_length = PS_HDR_LEN + PES_HDR_LEN * packet_cnt + SYS_HDR_LEN + PSM_HDR_LEN + raw_length + private_length;

    ps_info->packet_size = data_length;
    ps_info->frame_size = raw_length;
    ps_info->data = (void *) g_ps_alloc(sizeof(MF_S8) * data_length);
    memset(ps_info->data, 0, sizeof(MF_S8)*data_length);
    pdata_dst = (MF_S8 *)ps_info->data;
    if (ps_info->data == NULL) {
        MFerr("malloc failed\r\n");
        return MF_FAILURE;
    }

    //add ps_header
    ret = packet_header(pdata_dst, 0, data_length, raw_length);
    pdata_dst += ret;
    data_length -= ret;

    //add sys_header
    ret = packet_sys_header(pdata_dst);
    pdata_dst += ret;
    data_length -= ret;

    //add psm_header
    bits_buffer_t   bitsBuffer;
    bitsBuffer.i_size = PSM_HDR_LEN;
    bitsBuffer.i_data = 0;
    bitsBuffer.i_mask = 0x80;
    bitsBuffer.p_data = (MF_U8 *)(pdata_dst);
    memset(bitsBuffer.p_data, 0, PSM_HDR_LEN);
    bits_write(&bitsBuffer, 24, 0x000001);  /*start code*/
    bits_write(&bitsBuffer, 8, 0xBC);       /*map stream id*/
    bits_write(&bitsBuffer, 16, 24 + private_length);   /*program stream map length*/
    bits_write(&bitsBuffer, 1, 1);          /*current next indicator */
    bits_write(&bitsBuffer, 2, 3);          /*reserved*/
    bits_write(&bitsBuffer, 5, 0);          /*program stream map version*/
    bits_write(&bitsBuffer, 7, 0x7F);       /*reserved */
    bits_write(&bitsBuffer, 1, 1);          /*marker bit */
    bits_write(&bitsBuffer, 16, private_length + 6);        /*programe stream info length*/

    bits_write(&bitsBuffer, 8, 0x05);       /*description_tag*/
    bits_write(&bitsBuffer, 8, private_length + 4);     /*description_length*/
    bits_write(&bitsBuffer, 32, 0x00);      /*marker bit*/

    /*private info*/
    memset(&custom, 0x0, sizeof(struct ps_custom_s));
    custom.type = 0;
    custom.size = raw_length;
    memcpy(bitsBuffer.p_data + bitsBuffer.i_data, &custom, private_length);
    bitsBuffer.i_data += private_length;

    bits_write(&bitsBuffer, 16, 0);         /*elementary stream map length  is*/

    /*video*/
    bits_write(&bitsBuffer, 8, 0x24);       /*stream_type*/
    bits_write(&bitsBuffer, 8, 0xE0);       /*elementary_stream_id*/
    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length */
    /*audio*/
    bits_write(&bitsBuffer, 8, 0x0f);       /*stream_type*/
    bits_write(&bitsBuffer, 8, 0xC0);       /*elementary_stream_id*/
    bits_write(&bitsBuffer, 16, 0);         /*elementary_stream_info_length is*/

    /*crc */
    bits_write(&bitsBuffer, 8, 0xfe);       /*crc (24~31) bits*/
    bits_write(&bitsBuffer, 8, 0xbf);       /*crc (16~23) bits*/
    bits_write(&bitsBuffer, 8, 0xb1);       /*crc (8~15) bits*/
    bits_write(&bitsBuffer, 8, 0xd7);       /*crc (0~7) bits*/

    ret = PSM_HDR_LEN + private_length;
    pdata_dst += ret;
    data_length -= ret;

    //data
    while (raw_length != 0) {
        pes_length = (raw_length > PS_PES_PAYLOAD_SIZE) ? PS_PES_PAYLOAD_SIZE : raw_length;
        packet_pes_header(pdata_dst, PS_PRIVATE_STREAM, pes_length, 0);
        pdata_dst += PES_HDR_LEN;
        data_length -= PES_HDR_LEN;
        memcpy(pdata_dst, pdata_raw, sizeof(MF_S8)*pes_length);
        pdata_dst += pes_length;
        pdata_raw += pes_length;
        data_length -= pes_length;
        raw_length -= pes_length;
    }
    if (data_length != 0) {
        MFerr("errro data:data_length[%d],frame_length[%d]\r\n", data_length, raw_length);
        ps_packet_encoder_free(ps_info);
        return MF_FAILURE;

    }

    return MF_SUCCESS;
}


MF_S32
ps_packet_get_header(void *data, MF_S32 length, struct mf_frame *frame)
{
    MF_S32 stream_id;
    MF_S8 *pdata_raw = data;
    MF_S32 start = PS_HDR_LEN + SYS_HDR_LEN;
    MF_S32 minSize = start + 16 + sizeof(struct mf_frame);


    if (length < minSize) {
        return MF_FAILURE;
    }

    stream_id = *(MF_S32 *)(pdata_raw + start);
    if (stream_id != 0xbc010000) {
        return MF_FAILURE;
    }

    pdata_raw += start + 16;
    memcpy(frame, pdata_raw, sizeof(struct mf_frame));
    return MF_SUCCESS;
}

