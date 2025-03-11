// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "jpegd_image.h"
#include "jpegd_drv.h"
#include "jpegd_entry.h"
#include "jpegd_error.h"

static const unsigned char g_zigzag_for_qtable[ZIGZAG_TABLE_SIZE] = {
    0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
};

motion_jpeg_obj g_jpegd_ctx;

static int jpegd_dec_baseline(motion_jpeg_obj *jpegd_hdl_ctx, unsigned char **str, unsigned int *flag,
    unsigned int *idx, unsigned int len)
{
    unsigned char *stream = *str;
    int result = TD_SUCCESS;
    if (*flag == 1) {
        int tmp_len = (stream[0] << 8) + stream[1]; /* shift left 8 bits */
        tmp_len = jpegd_min(tmp_len, len - *idx);
        jpegd_hdl_ctx->stream_buffer = stream;
        *idx += (tmp_len + 2); /* 2 number of bytes */
        stream += (tmp_len);
        result = decode_sof0(jpegd_hdl_ctx, jpegd_hdl_ctx->stream_buffer);
        if (result == OT_JPEG_DEC_OK) {
            *flag = 2; /* 2 decode sof0 success */
        }
    }
    *str = stream;
    return result;
}

static int jpegd_dec_dht(motion_jpeg_obj *jpegd_hdl_ctx, unsigned char **str, unsigned int len, unsigned int *idx)
{
    unsigned char *stream = *str;
    unsigned int tmp_len = (stream[0] << 8) + stream[1]; /* shift left 8 bits */
    tmp_len = jpegd_min(tmp_len, len - *idx);
    jpegd_hdl_ctx->stream_buffer = stream;
    *idx += (tmp_len + 2); /* 2 number of bytes */
    stream += (tmp_len);

    if (decode_dht(jpegd_hdl_ctx, jpegd_hdl_ctx->stream_buffer)) {
        return TD_FAILURE;
    }
    *str = stream;
    return TD_SUCCESS;
}

static int jpegd_dec_sos(motion_jpeg_obj *jpegd_hdl_ctx, unsigned char **str, unsigned int *flag,
    unsigned int *idx, unsigned int len)
{
    unsigned char *stream = *str;
    int result;
    if (*flag == 2) { /* 2 decode sof0 success */
        int tmp_len = (stream[0] << 8) + stream[1]; /* shift left 8 bits */
        tmp_len = jpegd_min(tmp_len, len - *idx);
        jpegd_hdl_ctx->stream_buffer = stream;
        *idx += (tmp_len);
        stream += (tmp_len);
        jpegd_hdl_ctx->first_mcu = 0;
        result = decode_sos(jpegd_hdl_ctx, jpegd_hdl_ctx->stream_buffer);
        if (result == OT_JPEG_DEC_OK) {
            *flag = 3;  /* 3 scan header decoding success */
        } else {
            return TD_FAILURE;
        }
    }
    *str = stream;
    return TD_SUCCESS;
}

static int jpegd_dec_dqt(motion_jpeg_obj *jpegd_hdl_ctx, unsigned char **str, unsigned int *idx, unsigned int len)
{
    unsigned char *stream = *str;
    int tmp_len = (stream[0] << 8) + stream[1]; /* shift left 8 bits */
    tmp_len = jpegd_min(tmp_len, len - *idx);
    jpegd_hdl_ctx->stream_buffer = stream;
    *idx += (tmp_len + 2); /* 2 DQT number of bytes */
    stream += (tmp_len);

    if (decode_dqt(jpegd_hdl_ctx, jpegd_hdl_ctx->stream_buffer)) {
        return TD_FAILURE;
    }
    *str = stream;
    return TD_SUCCESS;
}

static int jpegd_dec_dri(motion_jpeg_obj *jpegd_hdl_ctx, unsigned char **str, unsigned int *flag, unsigned int *idx)
{
    unsigned char *stream = *str;
    jpegd_hdl_ctx->stream_buffer = stream;
    if (decode_dri(jpegd_hdl_ctx, jpegd_hdl_ctx->stream_buffer)) { /* 4 dri len */
        *flag = 0;
        return TD_FAILURE;
    }
    *idx += 6; /* 6 define restart interval */
    stream += 4; /* 4 define restart interval */
    *str = stream;
    return TD_SUCCESS;
}

static int jpegd_dec_err(unsigned char **str, unsigned int *idx, unsigned int type, unsigned int *flag)
{
    unsigned char *stream = *str;
    int result = TD_SUCCESS;
    if ((type & 0xE0) == 0xE0) { /* 0xE0:APP_0 value */
        int tmp_len = (stream[0] << 8) + stream[1]; /* shift left 8 bits */
        *idx += (tmp_len + 2); /* 2 number of bytes */
        stream += (tmp_len);
    } else if ((type & 0xF0) == 0xC0) { /* 0xF0:JPG_0 0xC0:baseline */
        *flag = 0;
        result = TD_FAILURE;
    } else {
        *idx += 2; /* 2 number of bytes */
    }
    *str = stream;
    return result;
}

static void jpegd_dec_sos_success(motion_jpeg_obj *jpegd_hdl_ctx, bool quant_table_valid,
    bool huffman_table_valid, unsigned int idx)
{
    jpegd_hdl_ctx->stream_offest = idx + 2; /* 2 number of bytes */
    jpegd_hdl_ctx->valid_frame ^= 0x1;  /*  0x1:change to another buffer */

    if (quant_table_valid == TD_FALSE) {
        init_default_quant_table(jpegd_hdl_ctx);
    }
    if (huffman_table_valid == TD_FALSE) {
        init_default_huffman_table(jpegd_hdl_ctx);
    }
    return;
}

typedef struct {
    unsigned int type;
    unsigned int status;
    unsigned int str_len;
    td_bool qt_valid;
    td_bool ht_valid;
} jpegd_dec_attr;

static int jpegd_dec_syn(motion_jpeg_obj *ctx, unsigned char **stream, unsigned int *idx,
    jpegd_dec_attr *dec_attr)
{
    int result = TD_SUCCESS;
    unsigned char *str = *stream;
    unsigned int tmp_idx = *idx;
    switch (dec_attr->type) {
        case BASELINE: /* sequential DCT */
            result = jpegd_dec_baseline(ctx, &str, &dec_attr->status, &tmp_idx, dec_attr->str_len);
            break;

        case DHT: /* DHT */
            result = jpegd_dec_dht(ctx, &str, dec_attr->str_len, &tmp_idx);
            dec_attr->ht_valid = (result == TD_SUCCESS) ? TD_TRUE : TD_FALSE;
            break;

        case SOS: /* scan header */
            result = jpegd_dec_sos(ctx, &str, &dec_attr->status, &tmp_idx, dec_attr->str_len);
            break;

        case SOI: /* a new jpeg picture */
            tmp_idx += 2; /* 2 SOI number of bytes */
            dec_attr->status = 1;
            break;

        case EOI: /* end of jpeg picture */
            tmp_idx += 2; /* 2 EOI number of bytes */
            dec_attr->status = 3; /* 3:end of image, should return */
            break;

        case DQT:
            result = jpegd_dec_dqt(ctx, &str, &tmp_idx, dec_attr->str_len);
            dec_attr->qt_valid = (result == TD_SUCCESS) ? TD_TRUE : TD_FALSE;
            break;

        case DNL:
            tmp_idx += 6; /* 6 define number of lines */
            str += 4; /* 4 number of lines' size */
            dec_attr->status = 0;
            break;

        case DRI:
            result = jpegd_dec_dri(ctx, &str, &dec_attr->status, &tmp_idx);
            break;

        default: /* do not support */
            result = jpegd_dec_err(&str, &tmp_idx, dec_attr->type, &dec_attr->status);
            break;
    }
    *stream = str;
    *idx = tmp_idx;
    if (result != TD_SUCCESS) {
        ot_trace("jpeg decode type 0x%x error!\n", dec_attr->type);
    }
    return result;
}

static int jpegd_dec_end(motion_jpeg_obj *ctx, unsigned int idx, const jpegd_dec_attr *dec_attr)
{
    if (dec_attr->status == 3) { /* 3 scan header decoding success */
        jpegd_dec_sos_success(ctx, dec_attr->qt_valid, dec_attr->ht_valid, idx);
        return OT_MJPEG_DEC_OK;
    } else {
        return OT_MJPEG_NO_PICTURE;
    }
}

/* decode jpeg picture */
int jpegd_dec_frame(jpegd_handle handle, unsigned int flags)
{
    motion_jpeg_obj *jpegd_hdl_ctx = (motion_jpeg_obj *)handle;
    unsigned int i;
    unsigned int type;
    unsigned char *stream = TD_NULL;
    unsigned int len;
    jpegd_dec_attr dec_attr = {0};

    if (!(jpegd_hdl_ctx != TD_NULL && jpegd_hdl_ctx->stream.vir_addr != TD_NULL)) {
        ot_trace("jpegd handle is null!\n");
        return OT_MJPEG_ERR_HANDLE;
    }

    len = jpegd_hdl_ctx->stream.len;
    if (len > 4096 * 2160 * 3 / 4) { /* 4096 2160 3 4: 4K jpeg size */
        ot_trace("jpegd stream size over!\n");
        return OT_MJPEG_ERR_HANDLE;
    }

    stream = jpegd_hdl_ctx->stream.vir_addr;
    jpegd_hdl_ctx->frame.restart_interval = 0;
    dec_attr.str_len = len;

    for (i = 0; i < len;) {
        type = 0xFF; /* 0xFF:jpeg maker */
        while ((i < len) && (*(stream++) != 0xff)) { /* 0xFF:jpeg maker */
            i++;
        }
        type = *(stream++);
        while ((i < len) && (type == 0xff)) { /* 0xFF:jpeg maker */
            type = *(stream++);
            i++;
        }
        dec_attr.type    = type;
        if (jpegd_dec_syn(jpegd_hdl_ctx, &stream, &i, &dec_attr) != TD_SUCCESS) {
            goto end;
        }
        if (dec_attr.status == 3) { /* 3 sos decoding success or eoi */
            goto end;
        }
    }

end:
    return jpegd_dec_end(jpegd_hdl_ctx, i, &dec_attr);
}

/* create a JPEG decoder handle */
jpegd_handle jpegd_get_handle(void)
{
    motion_jpeg_obj *mjpeg = &g_jpegd_ctx;
    int pic_size;

    mjpeg->max_width = JPEGD_MAX_WIDTH;
    mjpeg->max_height = JPEGD_MAX_HEIGHT;

    pic_size = mjpeg->max_width * mjpeg->max_height;
    mjpeg->buf_size = pic_size + 0x10000; /* 0x10000:extra mem */
    mjpeg->stream_buffer = TD_NULL;
    mjpeg->pic_buffer = TD_NULL;
    mjpeg->valid_frame = 0;
    mjpeg->width_in_mcu = 1;
    mjpeg->height_in_mcu = 0;
    mjpeg->y_stride = 0;
    mjpeg->c_stride = 0;
    mjpeg->pic_format = PICTURE_FORMAT_BUTT;
    mjpeg->first_mcu = 0;
    mjpeg->bits.bit_len = 0;
    mjpeg->bits.bit_offset = 0;
    mjpeg->bits.buffer = TD_NULL;
    mjpeg->frame.nf = 0;
    mjpeg->frame.y_height = 0;
    mjpeg->frame.y_width = 0;
    mjpeg->frame.restart_interval = 0;
    mjpeg->frame.restart_interval_logic = 0;
    mjpeg->frame.max_mcu_number = 0;

    init_default_huffman_table(mjpeg);
    init_default_quant_table(mjpeg);

    mjpeg->state = STATE_IDLE;

    return (jpegd_handle)mjpeg;
}

static void jpegd_config_huffman_dc(const motion_jpeg_obj *jpeg_hdl_ctx, unsigned int *huffman_table, unsigned int cs)
{
    const huffman_tab *huf_tab = &jpeg_hdl_ctx->h_tab[cs];
    unsigned int i, num, j, index;
    for (index = 0, i = 0; i < JPEGD_CODE_LEN; i++) {
        num = huf_tab->len[i];
        for (j = 0; (j < num) && (index < 12); j++, index++) { /* 12 max index */
            int pos = huf_tab->huffman_val[index];
            if (cs == 1) {
                huffman_table[pos] |= ((i + 1) << 20) + /* shift left 20 bits 0xFF000:overflow protection */
                (((j + (unsigned int)huf_tab->min_code[i]) << 12) & 0xFF000); /* shift left 12 bits */
            } else {
                huffman_table[pos] |=
                ((i + 1) << 8) + ((j + (unsigned int)huf_tab->min_code[i]) & 0xFF); /* shift left 8 bits */
            }
        }
    }
    return;
}

static void jpegd_config_huffman_ac(const motion_jpeg_obj *jpeg_hdl_ctx, unsigned char *ac_min_table,
    unsigned char *ac_base_table, unsigned int idx)
{
    const huffman_tab *huf_tab = &jpeg_hdl_ctx->h_tab[idx];
    int i;

    for (i = 0; i < JPEGD_CODE_LEN; i++) {
        unsigned int base_code = 0;

        if (huf_tab->len[i]) {
            base_code = huf_tab->huffman_val_ptr[i] - huf_tab->min_code[i];
        }

        ac_min_table[i] = ((unsigned int)huf_tab->min_code[i]) & 0xFF; /* 0xFF:get low 8 bits */
        ac_base_table[i] = base_code & 0xFF; /* 0xFF:get low 8 bits */
    }
    return;
}

static void jpegd_config_huffman_table(motion_jpeg_obj *jpeg_hdl_ctx)
{
    unsigned int i;
    unsigned int huffman_table[HDC_TABLE_SIZE] = {0};
    unsigned char luma_ac_min_table[JPEGD_CODE_LEN]    = {0};
    unsigned char luma_ac_base_table[JPEGD_CODE_LEN]   = {0};
    unsigned char chroma_ac_min_table[JPEGD_CODE_LEN]  = {0};
    unsigned char chroma_ac_base_table[JPEGD_CODE_LEN] = {0};

    /* config huffman dc */
    jpegd_config_huffman_dc(jpeg_hdl_ctx, huffman_table, 1);
    jpegd_config_huffman_dc(jpeg_hdl_ctx, huffman_table, 0);

    /* config huffman ac */
    jpegd_config_huffman_ac(jpeg_hdl_ctx, luma_ac_min_table, luma_ac_base_table, 2); /* 2:table index */
    jpegd_config_huffman_ac(jpeg_hdl_ctx, chroma_ac_min_table, chroma_ac_base_table, 3); /* 3:table index */

    /* config huffman table */
    for (i = 0; i < HDC_TABLE_SIZE; i++) {
        jpeg_hdl_ctx->vpu_config.huffman_table[i] = huffman_table[i];
    }

    /* config huffman_min_table table */
    for (i = 0; i < HAC_MIN_TABLE_SIZE; i++) {
        jpeg_hdl_ctx->vpu_config.huffman_min_table[i] =
            (chroma_ac_min_table[2 * i + 1] << 24) + /* 2 config chroma ac table shift left 24 bits */
            (chroma_ac_min_table[2 * i] << 16) + /* 2 config chroma ac table shift left 16 bits */
            (luma_ac_min_table[2 * i + 1] << 8) + /* 2 config luma ac table shift left 8 bits */
            (luma_ac_min_table[2 * i]); /* 2 config luma ac min table */
    }

    /* config huffman_base_table table */
    for (i = 0; i < HAC_BASE_TABLE_SIZE; i++) {
        jpeg_hdl_ctx->vpu_config.huffman_base_table[i] =
            (chroma_ac_base_table[2 * i + 1] << 24) + /* 2 config chroma ac base table shift left 24 bits */
            (chroma_ac_base_table[2 * i] << 16) + /* 2 config chroma ac base table shift left 16 bits */
            (luma_ac_base_table[2 * i + 1] << 8) + /* 2 config luma ac base table shift left 8 bits */
            (luma_ac_base_table[2 * i]); /* 2 config luma ac base table */
    }

    /* config huffman_symbol_table table */
    for (i = 0; i < HAC_SYMBOL_TABLE_SIZE; i++) {
        jpeg_hdl_ctx->vpu_config.huffman_symbol_table[i] =
            (jpeg_hdl_ctx->h_tab[3].huffman_val[i] << 8) + /* table 3 shift left 8 bits */
            jpeg_hdl_ctx->h_tab[2].huffman_val[i]; /* plus table 2 */
    }
    return;
}

static void jpegd_config_quant_table(motion_jpeg_obj *jpeg_hdl_ctx)
{
    unsigned int i;
    unsigned int q_tab_y_index = jpeg_hdl_ctx->frame.tq[COM0] & 3; /* 3 overflow protection */
    unsigned int q_tab_u_index = jpeg_hdl_ctx->frame.tq[COM1] & 3; /* 3 overflow protection */
    unsigned int q_tab_v_index = jpeg_hdl_ctx->frame.tq[COM2] & 3; /* 3 overflow protection */

    unsigned char *q_cr = (unsigned char *)&jpeg_hdl_ctx->q_tab[q_tab_v_index];
    unsigned char *q_cb = (unsigned char *)&jpeg_hdl_ctx->q_tab[q_tab_u_index];
    unsigned char *q_y = (unsigned char *)&jpeg_hdl_ctx->q_tab[q_tab_y_index];
    int pos;

    for (i = 0; i < QUANT_TABLE_SIZE; i++) {
        pos = g_zigzag_for_qtable[i & 0x3f]; /* 0x3f:get low 6bits */
        if (pos >= QUANT_TABLE_SIZE) {
            printf("quan table pos out of range!\n");
            continue;
        }
        jpeg_hdl_ctx->vpu_config.quant_table[pos] =
            q_y[i] + (q_cb[i] << 8) + (q_cr[i] << 16); /* shift left 8 bits shift left 16 bits */
    }

    return;
}

static void jpegd_config_rgb_info(motion_jpeg_obj *jpeg_hdl_ctx)
{
    if (jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_YVU_SEMIPLANAR_420) {
        jpeg_hdl_ctx->vpu_config.out_yuv = TD_TRUE;
    } else {
        jpeg_hdl_ctx->vpu_config.out_yuv = TD_FALSE;
        if (jpeg_hdl_ctx->frame.y_width > UINT_MAX / 4) { /* 4:multiplier */
            printf("The frame width is out of range! stride may overflow!\n");
            return;
        }
        if ((jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_ARGB_8888) ||
            (jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_ABGR_8888)) {
            jpeg_hdl_ctx->vpu_config.rgb_stride =
                align_up(jpeg_hdl_ctx->frame.y_width * 4, JPEGD_ALIGN_16); /* 4 multiply */
        } else if ((jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_ARGB_1555) ||
                   (jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_ABGR_1555)) {
            jpeg_hdl_ctx->vpu_config.rgb_stride =
                align_up(jpeg_hdl_ctx->frame.y_width * 2, JPEGD_ALIGN_16); /* 2 multiply */
        } else if ((jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_RGB_888) ||
                   (jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_BGR_888)) {
            jpeg_hdl_ctx->vpu_config.rgb_stride =
                align_up(jpeg_hdl_ctx->frame.y_width * 3, JPEGD_ALIGN_16); /* 3 multiply */
        } else if ((jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_RGB_565) ||
                   (jpeg_hdl_ctx->vpu_config.pixel_format == PIXEL_FORMAT_BGR_565)) {
            jpeg_hdl_ctx->vpu_config.rgb_stride =
                align_up(jpeg_hdl_ctx->frame.y_width * 2, JPEGD_ALIGN_16); /* 2 multiply */
        }
    }
    return;
}

unsigned int jpegd_prepare_pic_type(picture_format pic_format)
{
    unsigned int pic_type = 0;
    switch (pic_format) {
        case PICTURE_FORMAT_YUV420:
            pic_type = 3; /* 3: value of hardware regulation */
            break;
        case PICTURE_FORMAT_YUV422:
            pic_type = 4; /* 4: value of hardware regulation */
            break;
        case PICTURE_FORMAT_YUV444:
            pic_type = 6; /* 6: value of hardware regulation */
            break;
        case PICTURE_FORMAT_YUV422V:
            pic_type = 5; /* 5: value of hardware regulation */
            break;
        case PICTURE_FORMAT_YUV400:
            pic_type = 0; /* 0: value of hardware regulation */
            break;
        default:
            printf("Unknown picture format %d!", pic_format);
    }
    return pic_type;
}

/* Prepare the jpegd Hardware Info */
static int jpegd_prepare_hardware_info(motion_jpeg_obj *jpeg_hdl_ctx)
{
    unsigned int ysize;

    if (align_up(jpeg_hdl_ctx->frame.y_height, JPEGD_ALIGN_16) >
        UINT_MAX / align_up(jpeg_hdl_ctx->frame.y_width, JPEGD_ALIGN_64)) {
        printf("The size of y may overflow.!\n");
        return TD_FAILURE;
    }

    ysize = align_up(jpeg_hdl_ctx->frame.y_width, JPEGD_ALIGN_64) *
        align_up(jpeg_hdl_ctx->frame.y_height, JPEGD_ALIGN_16);
    jpeg_hdl_ctx->vpu_config.width = jpeg_hdl_ctx->frame.y_width;
    jpeg_hdl_ctx->vpu_config.height = jpeg_hdl_ctx->frame.y_height;
    jpeg_hdl_ctx->vpu_config.width_in_mcu = jpeg_hdl_ctx->width_in_mcu;
    jpeg_hdl_ctx->vpu_config.height_in_mcu = jpeg_hdl_ctx->height_in_mcu;
    jpeg_hdl_ctx->vpu_config.c_phy_addr = jpeg_hdl_ctx->vpu_config.y_phy_addr + ysize;
    jpeg_hdl_ctx->vpu_config.y_stride = align_up(jpeg_hdl_ctx->frame.y_width, JPEGD_ALIGN_64);
    jpeg_hdl_ctx->vpu_config.c_stride = align_up(jpeg_hdl_ctx->frame.y_width, JPEGD_ALIGN_64);
    jpeg_hdl_ctx->vpu_config.phy_str_start = jpeg_hdl_ctx->vpu_config.phy_str_start + jpeg_hdl_ctx->stream_offest;
    jpeg_hdl_ctx->vpu_config.y_fac = jpeg_hdl_ctx->frame.h[0];
    jpeg_hdl_ctx->vpu_config.u_fac = jpeg_hdl_ctx->frame.h[1];
    jpeg_hdl_ctx->vpu_config.v_fac = jpeg_hdl_ctx->frame.h[2]; /* 2 array index */
    jpeg_hdl_ctx->vpu_config.dri = jpeg_hdl_ctx->frame.restart_interval_logic;
    jpeg_hdl_ctx->vpu_config.pic_format = jpeg_hdl_ctx->pic_format;
    jpeg_hdl_ctx->vpu_config.pic_type = jpegd_prepare_pic_type(jpeg_hdl_ctx->vpu_config.pic_format);

    /* config RGB info */
    jpegd_config_rgb_info(jpeg_hdl_ctx);

    /* config quant table */
    jpegd_config_quant_table(jpeg_hdl_ctx);

    /* config huffman table */
    jpegd_config_huffman_table(jpeg_hdl_ctx);

    return TD_SUCCESS;
}

int jpegd_start_one_frame(jpegd_handle handle, unsigned int flags)
{
    int ret;
    motion_jpeg_obj *jpegd_hdl_ctx = (motion_jpeg_obj *)handle;

    ret = jpegd_dec_frame(handle, flags);
    if (ret != TD_SUCCESS) {
        ot_trace("vdec_start_one_frame: decode stream fail for 0x%x\n", ret);
        return ret;
    }

    ret = jpegd_prepare_hardware_info(jpegd_hdl_ctx);
    if (ret != TD_SUCCESS) {
        ot_trace("vdec_start_one_frame: decode stream fail for 0x%x\n", ret);
        return ret;
    }

    return TD_SUCCESS;
}

/* write the jpegd register */
void jpegd_write_regs(jpegd_handle handle, s_jpgd_regs_type *reg_base)
{
    motion_jpeg_obj *jpeg_hdl_ctx = (motion_jpeg_obj *)handle;
    if (jpeg_hdl_ctx == TD_NULL) {
        ot_trace("jpeg_hdl_ctx is TD_NULL!\n");
        return;
    }

    jpegd_drv_write_regs(reg_base, &jpeg_hdl_ctx->vpu_config);
    return;
}

/* read the jpegd register */
void jpegd_read_regs(jpegd_handle handle, const s_jpgd_regs_type *reg_base)
{
    motion_jpeg_obj *jpeg_hdl_ctx = (motion_jpeg_obj *)handle;

    if (jpeg_hdl_ctx == TD_NULL) {
        ot_trace("jpeg_hdl_ctx is TD_NULL!\n");
        return;
    }
    jpegd_drv_read_regs(reg_base, &jpeg_hdl_ctx->vpu_status);
    return;
}

int jpegd_start_decoding(jpegd_handle handle)
{
    motion_jpeg_obj *jpegd_hld_ctx = TD_NULL;
    int ret;

    jpegd_hld_ctx = (motion_jpeg_obj *)handle;
    jpegd_hld_ctx->vpu_config.y_phy_addr = get_video_data_base();
    jpegd_hld_ctx->vpu_config.phy_str_buf_start = get_ot_logo();
    jpegd_hld_ctx->vpu_config.phy_str_buf_end = get_ot_logo() + align_up(get_jpeg_size_val(), JPEGD_ADDR_ALIGN);
    jpegd_hld_ctx->vpu_config.phy_str_start = get_ot_logo();
    jpegd_hld_ctx->vpu_config.phy_str_end = get_ot_logo() + get_jpeg_size_val();
    jpegd_hld_ctx->vpu_config.phy_emar_buffer0 = get_jpegd_emar_buf();
    jpegd_hld_ctx->vpu_config.phy_emar_buffer1 = get_jpegd_emar_buf() + ONE_EMAR_BUF_SIZE;
    jpegd_hld_ctx->stream.vir_addr = (unsigned char *)(uintptr_t)get_ot_logo();
    jpegd_hld_ctx->stream.len = get_jpeg_size_val();
    jpegd_hld_ctx->stream.phy_addr = get_ot_logo();
    jpegd_hld_ctx->stream.pts = 0;
    jpegd_hld_ctx->vpu_config.chn_id = 0;
    jpegd_hld_ctx->vpu_config.alpha = 255; /* 255 transparency */

    if (get_output_format() == 0) {
        jpegd_hld_ctx->vpu_config.pixel_format = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    } else if (get_output_format() == 1) {
        jpegd_hld_ctx->vpu_config.pixel_format = PIXEL_FORMAT_ARGB_1555;
    } else if (get_output_format() == 2) { /* 2 output format */
        jpegd_hld_ctx->vpu_config.pixel_format = PIXEL_FORMAT_ARGB_8888;
    } else {
        jpegd_hld_ctx->vpu_config.pixel_format = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }

    ret = jpegd_start_one_frame(handle, 0);
    if (ret != TD_SUCCESS) {
        ot_trace("jpegd_start_decoding: decode stream fail for 0x%x\n", ret);
        return ret;
    }

    jpegd_set_clock_en(0, TD_TRUE);
    jpegd_reset(0);
    jpegd_set_time_out(0, 0xFFFFFFFF); /* 0xFFFFFFFF:time out value */

    jpegd_write_regs(handle, (s_jpgd_regs_type *)JPEGD_REGS_ADDR);

    jpegd_start_vpu(0);

    return TD_SUCCESS;
}

void jpegd_finish_decoding(jpegd_handle handle)
{
    unsigned int int_statue;
    unsigned int cnt = 0;
    motion_jpeg_obj *jpegd_hld_ctx = (motion_jpeg_obj *)handle;

    while (1) {
        udelay(10); /* 10 delay time */
        int_statue = jpegd_read_int(0);
        if (int_statue & 0x1f) {
            break;
        }
        if (cnt++ > 2000) { /* 2000:Maximum decoding time */
            ot_trace("jpeg decode over time\n");
            break;
        }
    }

    jpegd_read_regs(handle, (s_jpgd_regs_type *)JPEGD_REGS_ADDR);
    if (jpegd_hld_ctx->vpu_status.int_dec_finish == 0) {
        printf("hardware decoding error!\n");
    } else {
        if (jpegd_hld_ctx->vpu_config.out_yuv != TD_TRUE) {
            printf("hardware decoding success! %ux%u, stride %u.\n",
                   jpegd_hld_ctx->frame.y_width, jpegd_hld_ctx->frame.y_height, jpegd_hld_ctx->vpu_config.rgb_stride);
        } else {
            printf("hardware decoding success! %ux%u, stride %u.\n",
                   jpegd_hld_ctx->frame.y_width, jpegd_hld_ctx->frame.y_height, jpegd_hld_ctx->vpu_config.y_stride);
        }
    }

    jpegd_clear_int(0);
    jpegd_reset_select(0, TD_TRUE);

    return;
}

