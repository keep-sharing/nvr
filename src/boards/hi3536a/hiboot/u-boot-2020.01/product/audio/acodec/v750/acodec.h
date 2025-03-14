// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __ACODEC_H__
#define __ACODEC_H__

#include "audio_ao.h"

typedef enum {
    ACODEC_FS_8000 = 0x1,
    ACODEC_FS_11025 = 0x2,
    ACODEC_FS_12000 = 0x3,
    ACODEC_FS_16000 = 0x4,
    ACODEC_FS_22050 = 0x5,
    ACODEC_FS_24000 = 0x6,
    ACODEC_FS_32000 = 0x7,
    ACODEC_FS_44100 = 0x8,
    ACODEC_FS_48000 = 0x9,
    ACODEC_FS_64000 = 0xa,
    ACODEC_FS_96000 = 0xb,

    ACODEC_FS_BUTT = 0x1c,
} acodec_fs;

typedef enum {
    ACODEC_I2S_FS_8000 = 0x18,
    ACODEC_I2S_FS_11025 = 0x18,
    ACODEC_I2S_FS_12000 = 0x18,
    ACODEC_I2S_FS_16000 = 0x19,
    ACODEC_I2S_FS_22050 = 0x19,
    ACODEC_I2S_FS_24000 = 0x19,
    ACODEC_I2S_FS_32000 = 0x1a,
    ACODEC_I2S_FS_44100 = 0x1a,
    ACODEC_I2S_FS_48000 = 0x1a,
    ACODEC_I2S_FS_64000 = 0x1b,
    ACODEC_I2S_FS_96000 = 0x1b,

    ACODEC_I2S_FS_BUTT = 0x1c,
} acodec_i2s_fs;

typedef enum {
    ACODEC_ADC_MODESEL_6144 = 0x0,
    ACODEC_ADC_MODESEL_4096 = 0x1,

    ACODEC_ADC_MODESEL_BUTT = 0xff,
} acodec_adc_mode_sel;

int acodec_i2s_set(audio_sample_rate sample_rate);
int acodec_device_init(void);
int acodec_device_exit(void);

#endif /* End of #ifndef __ACODEC_H__ */
