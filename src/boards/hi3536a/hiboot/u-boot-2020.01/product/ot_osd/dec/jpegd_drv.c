// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#include "jpegd_drv.h"
#include <common.h>

#define osal_readl(x) (*((volatile int *)(x)))
#define osal_writel(v, x) (*((volatile int *)(x)) = (v))

__inline static unsigned int get_low_addr(unsigned long long phyaddr)
{
    return (unsigned int)phyaddr;
}

__inline static unsigned int get_high_addr(unsigned long long phyaddr)
{
    return (unsigned int)(phyaddr >> 32); /* shift right 32 bits */
}

void jpegd_set_clock_en(int vpu_id, td_bool is_run)
{
    unsigned long t, mask;
    const unsigned int offset = JPEGD_CKEN_OFFSET;
    mask = 1 << offset;
    t = osal_readl((const volatile void *)JPEGD_CRG_REG_ADDR);
    t &= ~mask;
    t |= ((unsigned int)is_run << offset) & mask;
    osal_writel(t, (volatile void *)JPEGD_CRG_REG_ADDR);

    return;
}

void jpegd_reset_select(int vpu_id, td_bool is_run)
{
    int i;
    volatile u_jpeg_dec_start d32;
    s_jpgd_regs_type *jpegd_reg = (s_jpgd_regs_type *)jpegd_get_reg_addr(vpu_id);

    if (is_run == TD_FALSE) {
        for (i = 0; i < 1000; i++) { /* 1000 cycle time */
            d32.u32 = jpegd_reg->jpeg_dec_start.u32;
            if (d32.bits.rst_busy == 0) {
                break;
            }
            udelay(10); /* 10 delay time */
        }
        if (i >= 1000) { /* 1000 cycle time */
            printf("JPEGD reset error!\n");
            return;
        }
    }

    unsigned long t, mask;
    const unsigned int offset = 0;
    mask = 1 << offset;
    t = osal_readl((const volatile void *)JPEGD_CRG_REG_ADDR);
    t &= ~mask;
    t |= ((unsigned int)is_run << offset) & mask;
    osal_writel(t, (volatile void *)JPEGD_CRG_REG_ADDR);

    return;
}

void jpegd_set_outstanding(int vpu_id, int outstanding)
{
    u_freq_scale d32;
    s_jpgd_regs_type *jpegd_reg = (s_jpgd_regs_type *)jpegd_get_reg_addr(vpu_id);

    d32.u32 = jpegd_reg->freq_scale.u32;
    d32.bits.outstanding = outstanding;
    jpegd_reg->freq_scale.u32 = d32.u32;
    return;
}

void *jpegd_get_reg_addr(int vpu_id)
{
    s_jpgd_regs_type *jpegd_reg = TD_NULL;

    jpegd_reg = (s_jpgd_regs_type *)JPEGD_REGS_ADDR;

    return jpegd_reg;
}

unsigned int jpegd_read_int(int vpu_id)
{
    s_jpgd_regs_type *jpegd_reg = (s_jpgd_regs_type *)jpegd_get_reg_addr(vpu_id);

    return jpegd_reg->jpeg_int.u32;
}

void jpegd_clear_int(int vpu_id)
{
    s_jpgd_regs_type *jpegd_reg = (s_jpgd_regs_type *)jpegd_get_reg_addr(vpu_id);
    /* check if it's frame end */
    jpegd_reg->jpeg_int.u32 = 0xffffffff; /* 0xffffffff:interrupt value */

    return;
}

void jpegd_set_int_mask(int vpu_id)
{
    s_jpgd_regs_type *jpegd_reg = (s_jpgd_regs_type *)jpegd_get_reg_addr(vpu_id);

    jpegd_reg->int_mask.u32 = 0x0; /* 0x0:interrupt mask value */

    return;
}

void jpegd_reset(int vpu_id)
{
    jpegd_reset_select(vpu_id, TD_TRUE);
    jpegd_reset_select(vpu_id, TD_FALSE);
    jpegd_set_int_mask(vpu_id);
    jpegd_set_outstanding(vpu_id, 0x3); /* 0x3:outstanding value */

    return;
}

void jpegd_start_vpu(int vpu_id)
{
    u_jpeg_dec_start d32;
    s_jpgd_regs_type *jpegd_reg = (s_jpgd_regs_type *)jpegd_get_reg_addr(vpu_id);

    d32.u32 = jpegd_reg->jpeg_dec_start.u32;
    d32.bits.jpeg_dec_start = 1;
    jpegd_reg->jpeg_dec_start.u32 = d32.u32;

    return;
}

int jpegd_set_time_out(int vpu_id, int time_out)
{
    s_jpgd_regs_type *jpegd_reg = (s_jpgd_regs_type *)jpegd_get_reg_addr(vpu_id);

    jpegd_reg->over_time_thd = (unsigned int)time_out;

    return TD_SUCCESS;
}

static void jpegd_resume_start(s_jpgd_regs_type *jpegd_reg)
{
    u_jpeg_resume_start d32;

    d32.u32 = 0;

    d32.bits.jpeg_resume_start = 0;
    d32.bits.last_resume_in_pic = 1;

    jpegd_reg->jpeg_resume_start.u32 = d32.u32;
    return;
}

static void jpegd_stride(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_jpeg_stride d32;

    d32.u32 = 0;

    d32.bits.y_stride = config->y_stride;
    d32.bits.uv_stride = config->c_stride;

    jpegd_reg->jpeg_stride.u32 = d32.u32;
    return;
}

static void jpegd_picture_size(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_picture_size d32;

    d32.u32 = 0;

    d32.bits.pic_width_in_mcu = config->width_in_mcu;
    d32.bits.pic_height_in_mcu = config->height_in_mcu;

    jpegd_reg->picture_size.u32 = d32.u32;
    return;
}

static void jpegd_picture_type(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_picture_type d32;

    d32.u32 = 0;

    d32.bits.pic_type = config->pic_type;

    jpegd_reg->picture_type.u32 = d32.u32;
    return;
}

static void jpegd_rgb_out_stride(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_rgb_out_stride d32;

    d32.u32 = 0;

    d32.bits.jcfg2jdo_out_stride = config->rgb_stride;

    jpegd_reg->rgb_out_stride.u32 = d32.u32;
    return;
}

static void jpegd_bit_buffer_addr(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    jpegd_reg->bitbuffer_staddr_lsb = get_low_addr(config->phy_str_buf_start);
    jpegd_reg->bitbuffer_staddr_msb.u32 = get_high_addr(config->phy_str_buf_start);

    jpegd_reg->bitbuffer_endaddr_lsb = get_low_addr(config->phy_str_buf_end);
    jpegd_reg->bitbuffer_endaddr_msb.u32 = get_high_addr(config->phy_str_buf_end);
    return;
}

static void jpegd_bit_stream_addr(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    jpegd_reg->bitstreams_staddr_lsb = get_low_addr(config->phy_str_start);
    jpegd_reg->bitstreams_staddr_msb.u32 = get_high_addr(config->phy_str_start);

    jpegd_reg->bitstreams_endaddr_lsb = get_low_addr(config->phy_str_end);
    jpegd_reg->bitstreams_endaddr_msb.u32 = get_high_addr(config->phy_str_end);
    return;
}

static void jpegd_pic_yuv_start_addr(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    jpegd_reg->picture_ystaddr_lsb = get_low_addr(config->y_phy_addr);
    jpegd_reg->picture_ystaddr_msb.u32 = get_high_addr(config->y_phy_addr);

    if (config->out_yuv == TD_TRUE) {
        jpegd_reg->picture_uvstaddr_lsb = get_low_addr(config->c_phy_addr);
        jpegd_reg->picture_uvstaddr_msb.u32 = get_high_addr(config->c_phy_addr);
    }
    return;
}

static void jpegd_freq_scale(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_freq_scale d32;
    d32.u32 = jpegd_reg->freq_scale.u32;

    d32.bits.freq_scale = 0;
    d32.bits.ck_gt_en = 0;
    d32.bits.outstanding = 3; /* 3:Simulation recommendation */
    d32.bits.axi_id = 0;
    if (config->out_yuv == TD_TRUE) {
        d32.bits.jidct_emar_en = 1;
    } else {
        d32.bits.jidct_emar_en = 0;
    }
    jpegd_reg->freq_scale.u32 = d32.u32;
    return;
}

static void jpegd_out_type(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_out_type d32;

    d32.u32 = 0;

    if (config->out_yuv == TD_TRUE) {
        if (config->pic_format == PICTURE_FORMAT_YUV400) {
            d32.bits.jidct_yuv420_en = 0;
        } else {
            d32.bits.jidct_yuv420_en = 1;
        }
    } else {
        d32.bits.jidct_yuv420_en = 0;

        d32.bits.hor_med_en = 0;
        d32.bits.ver_med_en = 0;
        d32.bits.dither_en = 0;
        if (config->pixel_format == PIXEL_FORMAT_ARGB_8888) {
            d32.bits.out_rgb_type = 0;
        } else if (config->pixel_format == PIXEL_FORMAT_ABGR_8888) {
            d32.bits.out_rgb_type = 1;
        } else if (config->pixel_format == PIXEL_FORMAT_ARGB_1555) {
            d32.bits.out_rgb_type = 2; /* 2 pixel format ARGB1555 */
        } else if (config->pixel_format == PIXEL_FORMAT_ABGR_1555) {
            d32.bits.out_rgb_type = 3; /* 3 pixel format ABGR1555 */
        } else if (config->pixel_format == PIXEL_FORMAT_RGB_565) {
            d32.bits.out_rgb_type = 4; /* 4 pixel format RGB565 */
        } else if (config->pixel_format == PIXEL_FORMAT_BGR_565) {
            d32.bits.out_rgb_type = 5; /* 5 pixel format BGR565 */
        } else if (config->pixel_format == PIXEL_FORMAT_RGB_888) {
            d32.bits.out_rgb_type = 6; /* 6 pixel format RGB888 */
        } else if (config->pixel_format == PIXEL_FORMAT_BGR_888) {
            d32.bits.out_rgb_type = 7; /* 7 pixel format BGR888 */
        }
    }

    jpegd_reg->out_type.u32 = d32.u32;
    return;
}

static void jpegd_alpha(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    if (config->out_yuv == TD_FALSE) {
        u_jpgd_alpha d32;

        d32.u32 = 0;

        d32.bits.jpgd_a = config->alpha;
        jpegd_reg->jpgd_alpha.u32 = d32.u32;
    }
    return;
}

static void jpegd_emar_buffer(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    unsigned int i;

    for (i = 0; i < JPEGD_IP_NUM; i++) {
        jpegd_reg->mtn_address0_lsb = get_low_addr(config->phy_emar_buffer0);
        jpegd_reg->mtn_address0_msb.u32 = get_high_addr(config->phy_emar_buffer0);

        jpegd_reg->mtn_address1_lsb = get_low_addr(config->phy_emar_buffer1);
        jpegd_reg->mtn_address1_msb.u32 = get_high_addr(config->phy_emar_buffer1);
    }
    return;
}

static void jpegd_pic_cut_pos(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    if (config->out_yuv == TD_FALSE) {
        {
            u_piccut_startpos d32;

            d32.u32 = 0;

            d32.bits.jcfg2jytr_pic_startx = 0;
            d32.bits.jcfg2jytr_pic_starty = 0;
            jpegd_reg->piccut_startpos.u32 = d32.u32;
        }
        {
            u_piccut_endpos d32;

            d32.u32 = 0;

            d32.bits.jcfg2jytr_pic_endx = config->width - 1;
            d32.bits.jcfg2jytr_pic_endy = config->height - 1;
            jpegd_reg->piccut_endpos.u32 = d32.u32;
        }
    }
    return;
}

static void jpegd_sampling_factor(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_sampling_factor d32;

    d32.u32 = 0;

    d32.bits.v_fac = config->v_fac;
    d32.bits.u_fac = config->u_fac;
    d32.bits.y_fac = config->y_fac;

    jpegd_reg->sampling_factor.u32 = d32.u32;
    return;
}

static void jpegd_dri(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    u_dri d32;

    d32.u32 = 0;

    d32.bits.jcfg2jvld_ri = config->dri;

    jpegd_reg->dri.u32 = d32.u32;
    return;
}

static void jpegd_config_quant_table_regs(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    unsigned int i;
    for (i = 0; i < QUANT_TABLE_SIZE; i++) {
        jpegd_reg->quant_table[i].u32 = config->quant_table[i];
    }
    return;
}

static void jpegd_config_huffman_table_regs(s_jpgd_regs_type *jpegd_reg, const jpegd_vpu_config *config)
{
    unsigned int i;
    for (i = 0; i < HDC_TABLE_SIZE; i++) {
        jpegd_reg->hdc_table[i].u32 = config->huffman_table[i];
    }

    for (i = 0; i < HAC_MIN_TABLE_SIZE; i++) {
        jpegd_reg->hac_min_table[i].u32 = config->huffman_min_table[i];
    }

    for (i = 0; i < HAC_BASE_TABLE_SIZE; i++) {
        jpegd_reg->hac_base_table[i].u32 = config->huffman_base_table[i];
    }

    for (i = 0; i < HAC_SYMBOL_TABLE_SIZE; i++) {
        jpegd_reg->hac_symbol_table[i] = config->huffman_symbol_table[i];
    }
    return;
}

void jpegd_drv_write_regs(s_jpgd_regs_type *reg_base, const jpegd_vpu_config *config)
{
    s_jpgd_regs_type *jpegd_reg = reg_base;

    /*  1. jpeg_resume_start */
    jpegd_resume_start(jpegd_reg);

    /*  2. jpeg_stride */
    jpegd_stride(jpegd_reg, config);

    /*  3. picture_size */
    jpegd_picture_size(jpegd_reg, config);

    /*  4. picture_type */
    jpegd_picture_type(jpegd_reg, config);

    /*  . rgb_out_stride */
    jpegd_rgb_out_stride(jpegd_reg, config);

    /*  5. BITBUFFER_STADDR */
    /*  6. BITBUFFER_ENDADDR */
    jpegd_bit_buffer_addr(jpegd_reg, config);

    /*  7. BITSTREAMS_STADDR */
    /*  8. BITSTREAMS_ENDADDR */
    jpegd_bit_stream_addr(jpegd_reg, config);

    /*  9. PICTURE_YSTADDR */
    /*  10. PICTURE_UVSTADDR */
    jpegd_pic_yuv_start_addr(jpegd_reg, config);

    /*  11. freq_scale */
    jpegd_freq_scale(jpegd_reg, config);

    /*  12. out_type */
    jpegd_out_type(jpegd_reg, config);

    /*  . jpgd_alpha */
    jpegd_alpha(jpegd_reg, config);

    jpegd_emar_buffer(jpegd_reg, config);

    /*  piccut_startpos     u_piccut_endpos */
    jpegd_pic_cut_pos(jpegd_reg, config);

    jpegd_sampling_factor(jpegd_reg, config);

    /*  13. dri */
    jpegd_dri(jpegd_reg, config);

    jpegd_config_quant_table_regs(jpegd_reg, config);

    jpegd_config_huffman_table_regs(jpegd_reg, config);

    return;
}

void jpegd_drv_read_regs(const s_jpgd_regs_type *reg_base, jpegd_vpu_status *jpegd_vpu_status)
{
    const s_jpgd_regs_type *jpegd_reg = reg_base;
    if (jpegd_reg == TD_NULL || jpegd_vpu_status == TD_NULL) {
        printf("jpegd_reg or jpegd_vpu_status is TD_NULL!\n");
        return;
    }

    jpegd_vpu_status->int_dec_finish = jpegd_reg->jpeg_int.bits.int_dec_fnsh;
    jpegd_vpu_status->int_dec_err = jpegd_reg->jpeg_int.bits.int_dec_err;
    jpegd_vpu_status->int_over_time = jpegd_reg->jpeg_int.bits.int_over_time;
    jpegd_vpu_status->int_bs_res = jpegd_reg->jpeg_int.bits.int_bs_res;
    return;
}

