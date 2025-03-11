/*
 * ***************************************************************
 * Filename:        vapi_audio.c
 * Created at:      2015.10.21
 * Description:     audio controller
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "g722_codec.h"
#include "g726_codec.h"
#include "neaacdec.h"
#include "vapi.h"
#include "vapi_comm.h"

#define ACODEC_FILE   "/dev/acodec"
#define MAX_BUFF_SIZE 5120

typedef enum VqeType
{
    VQE_TYPE_NULL = 0,
    VQE_TYPE_RECORD,
    VQE_TYPE_TALK,
} VQE_TYPE_E;

typedef struct dev_s
{
    STATE_E              enState;
    td_u32               ChnCnt;
    ot_audio_sample_rate enInSampleRate;
    ot_audio_sample_rate enOutSampleRate;
    VQE_TYPE_E           enVqe;
    td_bool              bRspEn;
    td_u8               *buffer;
    pthread_mutex_t      mutex;
} DEV_S;

typedef struct aio_info_s
{
    DEV_S          stAiDev;
    DEV_S          stAoDev[AO_TYPE_NUM];
    NeAACDecHandle hDecoder;
} AIO_INFO_S;

static AIO_INFO_S g_stAioInfo;

static inline unsigned char ALawEncode(short pcm16)
{
    int      p = pcm16;
    unsigned a; // A-law value we are forming

    if (p < 0) {
        // -ve value
        // Note, ones compliment is used here as this keeps encoding symetrical
        // and equal spaced around zero cross-over, (it also matches the standard).
        p = ~p;
        a = 0x00; // sign = 0
    } else {
        // +ve value
        a = 0x80; // sign = 1
    }

    // Calculate segment and interval numbers
    p >>= 4;
    if (p >= 0x20) {
        if (p >= 0x100) {
            p >>= 4;
            a += 0x40;
        }
        if (p >= 0x40) {
            p >>= 2;
            a += 0x20;
        }
        if (p >= 0x20) {
            p >>= 1;
            a += 0x10;
        }
    }
    // a&0x70 now holds segment value and 'p' the interval number

    a += p; // a now equal to encoded A-law value

    return a ^ 0x55; // A-law has alternate bits inverted for transmission
}

static inline int ALawDecode(unsigned char alaw)
{
    unsigned sign;
    int      linear;
    unsigned shift;

    alaw ^= 0x55; // A-law has alternate bits inverted for transmission
    sign   = alaw & 0x80;
    linear = alaw & 0x1f;
    linear <<= 4;
    linear += 8; // Add a 'half' bit (0x08) to place PCM value in middle of range
    alaw &= 0x7f;

    if (alaw >= 0x20) {
        linear |= 0x100; // Put in MSB
        shift = (alaw >> 4) - 1;
        linear <<= shift;
    }

    if (!sign) {
        return -linear;
    } else {
        return linear;
    }
}

static inline unsigned char ULawEncode(short pcm16)
{
    int      p = pcm16;
    unsigned u; // u-law value we are forming

    if (p < 0) {
        // -ve value
        // Note, ones compliment is used here as this keeps encoding symetrical
        // and equal spaced around zero cross-over, (it also matches the standard).
        p = ~p;
        u = 0x80 ^ 0x10 ^ 0xff; // Sign bit = 1 (^0x10 because this will get inverted later) ^0xff to invert final u-Law code
    } else {
        // +ve value
        u = 0x00 ^ 0x10 ^ 0xff; // Sign bit = 0 (-0x10 because this amount extra will get added later) ^0xff to invert final u-Law code
    }

    p += 0x84; // Add uLaw bias

    if (p > 0x7f00) {
        p = 0x7f00; // Clip to 15 bits
    }

    // Calculate segment and interval numbers
    p >>= 3; // Shift down to 13bit
    if (p >= 0x100) {
        p >>= 4;
        u ^= 0x40;
    }
    if (p >= 0x40) {
        p >>= 2;
        u ^= 0x20;
    }
    if (p >= 0x20) {
        p >>= 1;
        u ^= 0x10;
    }
    // (u^0x10)&0x70 now equal to the segment value and 'p' the interval number (^0x10)

    u ^= p; // u now equal to encoded u-law value (with all bits inverted)

    return u;
}

static inline int ULawDecode(unsigned char ulaw)
{
    int      linear;
    unsigned shift;

    ulaw ^= 0xff; // u-law has all bits inverted for transmission

    linear = ulaw & 0x0f;
    linear <<= 3;
    linear |= 0x84; // Set MSB (0x80) and a 'half' bit (0x04) to place PCM value in middle of range

    shift = ulaw >> 4;
    shift &= 7;
    linear <<= shift;

    linear -= 0x84; // Subract uLaw bias

    if (ulaw & 0x80) {
        return -linear;
    } else {
        return linear;
    }
}

static inline unsigned char ALawToULaw(unsigned char alaw)
{
    unsigned char sign = alaw & 0x80;
    unsigned      ulaw;

    alaw ^= sign;
    alaw ^= 0x55;

    if (alaw < 45) {
        if (alaw < 24) {
            ulaw = (alaw < 8) ? (alaw << 1) + 1 : alaw + 8;
        } else {
            ulaw = (alaw < 32) ? (alaw >> 1) + 20 : alaw + 4;
        }
    } else {
        if (alaw < 63) {
            ulaw = (alaw < 47) ? alaw + 3 : alaw + 2;
        } else {
            ulaw = (alaw < 79) ? alaw + 1 : alaw;
        }
    }
    ulaw ^= sign;

    return ulaw ^ 0x7f;
}

static inline unsigned char ULawToALaw(unsigned char ulaw)
{
    unsigned char sign = ulaw & 0x80;
    unsigned      alaw;

    ulaw ^= sign;
    ulaw ^= 0x7f;

    if (ulaw < 48) {
        if (ulaw <= 32) {
            alaw = (ulaw <= 15) ? ulaw >> 1 : ulaw - 8;
        } else {
            alaw = (ulaw <= 35) ? (ulaw << 1) - 40 : ulaw - 4;
        }
    } else {
        if (ulaw <= 63) {
            alaw = (ulaw == 48) ? ulaw - 3 : ulaw - 2;
        } else {
            alaw = (ulaw <= 79) ? ulaw - 1 : ulaw;
        }
    }
    alaw ^= sign;

    return alaw ^ 0x55;
}

unsigned int vapi_aio_alaw_encode(unsigned char *dst, short *src, unsigned int srcSize)
{
    unsigned char *end;

    srcSize >>= 1;
    end = dst + srcSize;

    while (dst < end) {
        *dst++ = ALawEncode(*src++);
    }

    return srcSize;
}

unsigned int vapi_aio_alaw_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    short *end = dst + srcSize;

    while (dst < end) {
        *dst++ = ALawDecode(*src++);
    }

    return srcSize << 1;
}

unsigned int vapi_aio_ulaw_encode(unsigned char *dst, short *src, unsigned int srcSize)
{
    unsigned char *end;

    srcSize >>= 1;
    end = dst + srcSize;

    while (dst < end) {
        *dst++ = ULawEncode(*src++);
    }

    return srcSize;
}

unsigned int vapi_aio_ulaw_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    short *end = dst + srcSize;

    while (dst < end) {
        *dst++ = ULawDecode(*src++);
    }

    return srcSize << 1;
}

unsigned int vapi_aio_alaw_to_ulaw(unsigned char *dst, const unsigned char *src, unsigned int srcSize)
{
    unsigned char *end = dst + srcSize;

    while (dst < end) {
        *dst++ = ALawToULaw(*src++);
    }

    return srcSize;
}

unsigned int vapi_aio_ulaw_to_alaw(unsigned char *dst, const unsigned char *src, unsigned int srcSize)
{
    unsigned char *end = dst + srcSize;

    while (dst < end) {
        *dst++ = ULawToALaw(*src++);
    }

    return srcSize;
}

unsigned int vapi_aio_g722_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    return g722_decode(dst, src, srcSize);
    // return reco_rtsp_g722_decode(dst, src, srcSize);
}

unsigned int vapi_aio_g726_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    return g726_decode(dst, src, srcSize);
    // return reco_rtsp_g726_decode(dst, src, srcSize);
}

static int vapi_aio_aac_reopen(ARATE_E enRate)
{
    if (g_stAioInfo.hDecoder) {
        NeAACDecClose(g_stAioInfo.hDecoder);
        g_stAioInfo.hDecoder = NULL;
    }
    NeAACDecConfigurationPtr pstConfig;
    g_stAioInfo.hDecoder               = NeAACDecOpen();
    pstConfig                          = NeAACDecGetCurrentConfiguration(g_stAioInfo.hDecoder);
    pstConfig->defObjectType           = 0;
    pstConfig->outputFormat            = 1;
    pstConfig->defSampleRate           = enRate;
    pstConfig->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(g_stAioInfo.hDecoder, pstConfig);
    VAPILOG("vapi_aio_aac_reopen:%p sampleRate:%d\n", g_stAioInfo.hDecoder, enRate);
    return 0;
}
unsigned int vapi_aio_aac_decode(short *dst, const unsigned char *src, unsigned int srcSize, ARATE_E enRate)
{
    static NeAACDecFrameInfo    frameInfo;
    static ARATE_E              lastRate = 0;
    static short               *pBuffer;
    static const unsigned char *lastSrc    = 0;
    unsigned long               samplerate = 0;
    unsigned char               channels   = 0;
    int                         i;

    if (lastRate != enRate) {
        vapi_aio_aac_reopen(enRate);
        NeAACDecInit(g_stAioInfo.hDecoder, (unsigned char *)src, srcSize, &samplerate, &channels);
        VAPILOG("NeAACDecDecode[%lu][%d] enRate with %d\n", samplerate, channels, enRate);
        lastRate = enRate;
    }

    if (lastSrc != src) {
        pBuffer = (short *)NeAACDecDecode(g_stAioInfo.hDecoder, &frameInfo, (unsigned char *)src, srcSize);
        if (frameInfo.error > 0) {
            VAPILOG("NeAACDecDecode failed with %#x\n", frameInfo.error);
            return 0;
        }
        lastSrc = src;
    }
    //
    for (i = 0; i < frameInfo.samples; i++) {
        dst[i] = pBuffer[i * frameInfo.channels];
    }

    return frameInfo.samples;
}

static int vapi_aio_aac_init(void)
{
    NeAACDecConfigurationPtr pstConfig;

    g_stAioInfo.hDecoder               = NeAACDecOpen();
    pstConfig                          = NeAACDecGetCurrentConfiguration(g_stAioInfo.hDecoder);
    pstConfig->defObjectType           = 0;
    pstConfig->outputFormat            = 1;
    pstConfig->defSampleRate           = 8000;
    pstConfig->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(g_stAioInfo.hDecoder, pstConfig);

    return TD_SUCCESS;
}

static void vapi_aio_aac_uninit(void)
{
    NeAACDecClose(g_stAioInfo.hDecoder);
}

static inline td_s32 vapi_aio_ai_set_resample(ot_audio_dev AiDevId, ot_audio_sample_rate enOutSampleRate)
{
    td_s32 i;
    td_s32 s32Ret = TD_SUCCESS;

    if (enOutSampleRate != g_stAioInfo.stAiDev.enInSampleRate && enOutSampleRate != g_stAioInfo.stAiDev.enOutSampleRate) {
        for (i = 0; i < g_stAioInfo.stAiDev.ChnCnt; i++) {
            if (g_stAioInfo.stAiDev.bRspEn == TD_TRUE) {
                s32Ret = ss_mpi_ai_disable_resample(AiDevId, i);
            }
            s32Ret |= ss_mpi_ai_enable_resample(AiDevId, i, enOutSampleRate);
            if (s32Ret) {
                VAPILOG("ss_mpi_ai_enable_resample(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                return s32Ret;
            }
        }
        g_stAioInfo.stAiDev.bRspEn = TD_TRUE;
    } else if (enOutSampleRate == g_stAioInfo.stAiDev.enInSampleRate && g_stAioInfo.stAiDev.bRspEn == TD_TRUE) {
        for (i = 0; i < g_stAioInfo.stAiDev.ChnCnt; i++) {
            s32Ret |= ss_mpi_ai_disable_resample(AiDevId, i);
        }
        g_stAioInfo.stAiDev.bRspEn = TD_FALSE;
    }

    g_stAioInfo.stAiDev.enOutSampleRate = enOutSampleRate;

    return TD_SUCCESS;
}

static inline td_s32 vapi_aio_ao_set_resample(ot_audio_dev AoDevId, ot_audio_sample_rate enInSampleRate)
{
    td_s32 i;
    td_s32 s32Ret = TD_SUCCESS;

    if (enInSampleRate != g_stAioInfo.stAoDev[AoDevId].enOutSampleRate && enInSampleRate != g_stAioInfo.stAoDev[AoDevId].enInSampleRate) {
        for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
            if (g_stAioInfo.stAoDev[AoDevId].bRspEn == TD_TRUE) {
                s32Ret = ss_mpi_ao_disable_resample(AoDevId, i);
            }
            s32Ret |= ss_mpi_ao_enable_resample(AoDevId, i, enInSampleRate);
            if (s32Ret) {
                VAPILOG("ss_mpi_ao_enable_resample(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
                return s32Ret;
            }
            s32Ret |= ss_mpi_ao_clr_chn_buf(AoDevId, i);
            if (s32Ret) {
                VAPILOG("ss_mpi_ao_clr_chn_buf(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
                return s32Ret;
            }
        }
        g_stAioInfo.stAoDev[AoDevId].bRspEn = TD_TRUE;
    } else if (enInSampleRate == g_stAioInfo.stAoDev[AoDevId].enOutSampleRate && g_stAioInfo.stAoDev[AoDevId].bRspEn == TD_TRUE) {
        for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
            s32Ret |= ss_mpi_ao_disable_resample(AoDevId, i);
        }
        g_stAioInfo.stAoDev[AoDevId].bRspEn = TD_FALSE;
    }

    g_stAioInfo.stAoDev[AoDevId].enInSampleRate = enInSampleRate;

    return TD_SUCCESS;
}

static ot_acodec_fs vapi_aio_get_i2s_fs(ot_audio_sample_rate enSample)
{
    ot_acodec_fs i2sFsSel = OT_ACODEC_FS_BUTT;

    switch (enSample) {
    case OT_AUDIO_SAMPLE_RATE_8000:
        i2sFsSel = OT_ACODEC_FS_8000;
        break;

    case OT_AUDIO_SAMPLE_RATE_11025:
        i2sFsSel = OT_ACODEC_FS_11025;
        break;

    case OT_AUDIO_SAMPLE_RATE_12000:
        i2sFsSel = OT_ACODEC_FS_12000;
        break;

    case OT_AUDIO_SAMPLE_RATE_22050:
        i2sFsSel = OT_ACODEC_FS_22050;
        break;

    case OT_AUDIO_SAMPLE_RATE_16000:
        i2sFsSel = OT_ACODEC_FS_16000;
        break;

    case OT_AUDIO_SAMPLE_RATE_24000:
        i2sFsSel = OT_ACODEC_FS_24000;
        break;

    case OT_AUDIO_SAMPLE_RATE_32000:
        i2sFsSel = OT_ACODEC_FS_32000;
        break;

    case OT_AUDIO_SAMPLE_RATE_44100:
        i2sFsSel = OT_ACODEC_FS_44100;
        break;

    case OT_AUDIO_SAMPLE_RATE_48000:
        i2sFsSel = OT_ACODEC_FS_48000;
        break;

    case OT_AUDIO_SAMPLE_RATE_64000:
        i2sFsSel = OT_ACODEC_FS_64000;
        break;

    case OT_AUDIO_SAMPLE_RATE_96000:
        i2sFsSel = OT_ACODEC_FS_96000;
        break;

    default:
        break;
    }

    return i2sFsSel;
}

static td_s32 vapi_aio_set_codec(ot_audio_sample_rate enSample)
{
    td_s32          fdAcodec   = -1;
    td_s32          ret        = TD_SUCCESS;
    ot_acodec_mixer InputMode  = OT_ACODEC_MIXER_IN1;
    td_u32          i2s_fs_sel = 0;
    td_u32          gain_mic   = 0;
    td_u32          boostl     = 0;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0) {
        VAPILOG("can't open Acodec,%s\n", ACODEC_FILE);
        ret = TD_FAILURE;
    }
    if (ioctl(fdAcodec, OT_ACODEC_SOFT_RESET_CTRL)) {
        VAPILOG("Reset audio codec error\n");
    }

    i2s_fs_sel = vapi_aio_get_i2s_fs(enSample);
    if (i2s_fs_sel == OT_ACODEC_FS_BUTT) {
        VAPILOG("not support enSample:%d\n", enSample);
        ret = TD_FAILURE;
    }

    if (ioctl(fdAcodec, OT_ACODEC_SET_I2S1_FS, &i2s_fs_sel)) {
        VAPILOG("set acodec sample rate[%d] failed\n", enSample);
        ret = TD_FAILURE;
    }

    if (ioctl(fdAcodec, OT_ACODEC_SET_MIXER_MIC, &InputMode)) {
        VAPILOG("set acodec OT_ACODEC_SET_MIXER_MIC failed\n");
        close(fdAcodec);
        return TD_FAILURE;
    }

    if (ioctl(fdAcodec, OT_ACODEC_ENABLE_BOOSTL, &boostl)) {
        VAPILOG("OT_ACODEC_ENABLE_BOOSTL(%d) failed \n", boostl);
        ret = TD_FAILURE;
    }

    gain_mic = 4;
    if (ioctl(fdAcodec, OT_ACODEC_SET_GAIN_MICL, &gain_mic)) {
        VAPILOG("OT_ACODEC_SET_GAIN_MICL(%d) failed \n", gain_mic);
        ret = TD_FAILURE;
    }

    close(fdAcodec);

    return ret;
}

static td_s32 vapi_aio_ai_start(ot_audio_dev AiDevId, ot_audio_sample_rate enInSampleRate, td_u32 u32ChnCnt, ot_audio_sample_rate enOutSamplerate)
{
    td_s32          i;
    td_s32          s32Ret;
    ot_aio_attr     stAttr;
    ot_ai_chn_param stAiChnPara;

    s32Ret = vapi_aio_set_codec(enInSampleRate);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_set_codec(%d) failed with %#x\n", enInSampleRate, s32Ret);
        return s32Ret;
    }

    stAttr.sample_rate         = enInSampleRate;
    stAttr.bit_width           = OT_AUDIO_BIT_WIDTH_16;
    stAttr.work_mode           = OT_AIO_MODE_I2S_MASTER;
    stAttr.snd_mode            = OT_AUDIO_SOUND_MODE_MONO;
    stAttr.expand_flag         = 0;
    stAttr.frame_num           = 30;
    stAttr.point_num_per_frame = stAttr.sample_rate / 100 * 2;
    stAttr.chn_cnt             = u32ChnCnt;
    stAttr.clk_share           = 1;
    stAttr.i2s_type            = OT_AIO_I2STYPE_INNERCODEC;

    s32Ret = ss_mpi_ai_set_pub_attr(AiDevId, &stAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ai_set_pub_attr(%d) failed with %#x\n", AiDevId, s32Ret);
        return s32Ret;
    }

    s32Ret = ss_mpi_ai_enable(AiDevId);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ai_enable(%d) failed with %#x\n", AiDevId, s32Ret);
        return s32Ret;
    }

    g_stAioInfo.stAiDev.enInSampleRate = enInSampleRate;

    for (i = 0; i < u32ChnCnt; i++) {
        s32Ret = ss_mpi_ai_enable_chn(AiDevId, i);
        if (TD_SUCCESS != s32Ret) {
            VAPILOG("ss_mpi_ai_enable_chn(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            return s32Ret;
        }
        g_stAioInfo.stAiDev.ChnCnt++;

        s32Ret = ss_mpi_ai_get_chn_param(AiDevId, i, &stAiChnPara);
        if (TD_SUCCESS != s32Ret) {
            VAPILOG("ss_mpi_ai_get_chn_param(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            return s32Ret;
        }

        stAiChnPara.usr_frame_depth = 30;
        s32Ret                      = ss_mpi_ai_set_chn_param(AiDevId, i, &stAiChnPara);
        if (TD_SUCCESS != s32Ret) {
            VAPILOG("ss_mpi_ai_set_chn_param(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            return s32Ret;
        }

        g_stAioInfo.stAiDev.enVqe = VQE_TYPE_TALK;
        if (g_stAioInfo.stAiDev.enVqe == VQE_TYPE_RECORD) {
            ot_ai_record_vqe_cfg recVqeAttr;
            s32Ret = ss_mpi_ai_set_record_vqe_attr(AiDevId, i, &recVqeAttr);
        } else if (g_stAioInfo.stAiDev.enVqe == VQE_TYPE_TALK) {
            ot_ai_talk_vqe_cfg stVqeConfig;
            memset(&stVqeConfig, 0, sizeof(stVqeConfig));
            stVqeConfig.open_mask                     = OT_AI_TALKVQE_MASK_AEC | OT_AI_TALKVQE_MASK_ANR;
            stVqeConfig.work_sample_rate              = stAttr.sample_rate; // must be 8K / 16k
            stVqeConfig.frame_sample                  = stAttr.point_num_per_frame;
            stVqeConfig.work_state                    = OT_VQE_WORK_STATE_COMMON;
            stVqeConfig.aec_cfg.double_talk_threshold = 16384;
            stVqeConfig.aec_cfg.cozy_noisy_mode       = 0;
            stVqeConfig.aec_cfg.echo_band_low         = 10;
            stVqeConfig.aec_cfg.echo_band_high        = 23;
            stVqeConfig.aec_cfg.echo_band_low2        = 27;
            stVqeConfig.aec_cfg.echo_band_high2       = 45;
            stVqeConfig.aec_cfg.near_all_pass_energy  = 2;
            stVqeConfig.aec_cfg.near_clean_sup_energy = 2;
            stVqeConfig.aec_cfg.voice_protect_freq_l  = 3;
            stVqeConfig.aec_cfg.voice_protect_freq_l1 = 6;

            td_s16 s16ErlBand[6] = {4, 6, 36, 49, 50, 51};
            td_s16 s16Erl[7]     = {7, 10, 16, 10, 18, 18, 18};
            memcpy(stVqeConfig.aec_cfg.erl_band, &s16ErlBand, sizeof(s16ErlBand));
            memcpy(stVqeConfig.aec_cfg.erl, &s16Erl, sizeof(s16Erl));

            stVqeConfig.aec_cfg.usr_mode = TD_SUCCESS;
            stVqeConfig.anr_cfg.usr_mode = TD_FALSE;

            s32Ret = ss_mpi_ai_set_talk_vqe_attr(AiDevId, i, AO_SPK_MIC, i, &stVqeConfig);
            if (TD_SUCCESS != s32Ret) {
                VAPILOG("ss_mpi_ai_set_talk_vqe_attr(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                return s32Ret;
            }

            if (g_stAioInfo.stAiDev.enVqe != VQE_TYPE_NULL) {
                s32Ret = ss_mpi_ai_enable_vqe(AiDevId, i);
                if (TD_SUCCESS != s32Ret) {
                    VAPILOG("ss_mpi_ai_enable_vqe(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                    return s32Ret;
                }
            }
        }
    }

    s32Ret = vapi_aio_ai_set_resample(AiDevId, enOutSamplerate);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_ai_set_resample(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
        return s32Ret;
    }

    if (g_stAioInfo.stAiDev.buffer == TD_NULL) {
        g_stAioInfo.stAiDev.buffer = (td_u8 *)malloc(MAX_BUFF_SIZE);
        if (g_stAioInfo.stAiDev.buffer == TD_NULL) {
            VAPILOG("no enoungh memory !\n");
            return TD_FAILURE;
        }
    }

    s32Ret = comm_mutex_init(&g_stAioInfo.stAiDev.mutex);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("comm_mutex_init(%d) failed with %#x\n", AiDevId, s32Ret);
        return s32Ret;
    }

    g_stAioInfo.stAiDev.enState = STATE_BUSY;

    return TD_SUCCESS;
}

static td_s32 vapi_aio_ai_stop(ot_audio_dev AiDevId)
{
    td_s32 i;
    td_s32 s32Ret;

    comm_mutex_lock(&g_stAioInfo.stAiDev.mutex);

    for (i = 0; i < g_stAioInfo.stAiDev.ChnCnt; i++) {
        if (g_stAioInfo.stAiDev.bRspEn == TD_TRUE) {
            s32Ret = ss_mpi_ai_disable_resample(AiDevId, i);
            if (TD_SUCCESS != s32Ret) {
                VAPILOG("ss_mpi_ai_disable_resample(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
                return s32Ret;
            }
        }

        if (g_stAioInfo.stAiDev.enVqe != VQE_TYPE_NULL) {
            s32Ret = ss_mpi_ai_disable_vqe(AiDevId, i);
            if (TD_SUCCESS != s32Ret) {
                VAPILOG("ss_mpi_ai_disable_vqe(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
                return s32Ret;
            }
        }

        s32Ret = ss_mpi_ai_disable_chn(AiDevId, i);
        if (TD_SUCCESS != s32Ret) {
            VAPILOG("ss_mpi_ai_disable_chn(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
            return s32Ret;
        }
    }
    g_stAioInfo.stAiDev.ChnCnt          = 0;
    g_stAioInfo.stAiDev.enOutSampleRate = 0;
    g_stAioInfo.stAiDev.bRspEn          = TD_FALSE;

    s32Ret = ss_mpi_ai_disable(AiDevId);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ai_disable(%d) failed with %#x\n", AiDevId, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return s32Ret;
    }

    if (g_stAioInfo.stAiDev.buffer != TD_NULL) {
        free(g_stAioInfo.stAiDev.buffer);
        g_stAioInfo.stAiDev.buffer = TD_NULL;
    }

    g_stAioInfo.stAiDev.enState = STATE_IDLE;

    comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);

    comm_mutex_uninit(&g_stAioInfo.stAiDev.mutex);

    return TD_SUCCESS;
}

static td_s32 vapi_aio_ao_start(ot_audio_dev AoDevId, ot_audio_sample_rate enInSampleRate, td_u32 u32ChnCnt, ot_audio_sample_rate enOutSampleRate)
{
    td_s32        i;
    td_s32        s32Ret;
    ot_aio_attr   stAttr = {0};
    ot_ao_vqe_cfg stVqeConfig;

    stAttr.sample_rate         = enOutSampleRate;
    stAttr.bit_width           = OT_AUDIO_BIT_WIDTH_16;
    stAttr.work_mode           = OT_AIO_MODE_I2S_MASTER;
    stAttr.snd_mode            = OT_AUDIO_SOUND_MODE_MONO;
    stAttr.expand_flag         = 0;
    stAttr.frame_num           = 30;
    stAttr.point_num_per_frame = enOutSampleRate / 100 * 2;
    stAttr.chn_cnt             = u32ChnCnt;
    stAttr.clk_share           = 1;

    if (AoDevId == AO_MAIN_HDMI || AoDevId == AO_SUB_HDMI) {
        stAttr.i2s_type = OT_AIO_I2STYPE_INNERHDMI;
    } else {
        stAttr.i2s_type = OT_AIO_I2STYPE_INNERCODEC;
    }

    s32Ret = ss_mpi_ao_set_pub_attr(AoDevId, &stAttr);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ao_set_pub_attr(%d) failed with %#x!\n", AoDevId, s32Ret);
        return TD_FAILURE;
    }

    s32Ret = ss_mpi_ao_enable(AoDevId);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ao_enable(%d) failed with %#x!\n", AoDevId, s32Ret);
        return TD_FAILURE;
    }

    g_stAioInfo.stAoDev[AoDevId].enOutSampleRate = enOutSampleRate;

    for (i = 0; i < u32ChnCnt; i++) {
        s32Ret = ss_mpi_ao_enable_chn(AoDevId, i);
        if (TD_SUCCESS != s32Ret) {
            VAPILOG("ss_mpi_ao_enable_chn(%d) failed with %#x!\n", i, s32Ret);
            return TD_FAILURE;
        }

        g_stAioInfo.stAoDev[AoDevId].ChnCnt++;

        g_stAioInfo.stAoDev[AoDevId].enVqe = VQE_TYPE_NULL;
        if (g_stAioInfo.stAoDev[AoDevId].enVqe == VQE_TYPE_TALK && (enOutSampleRate == OT_AUDIO_SAMPLE_RATE_8000 || enOutSampleRate == OT_AUDIO_SAMPLE_RATE_16000)) {
            memset(&stVqeConfig, 0, sizeof(ot_ao_vqe_cfg));
            stVqeConfig.work_sample_rate              = enOutSampleRate;
            stVqeConfig.frame_sample                  = enOutSampleRate / 100 * 4;
            stVqeConfig.work_state                    = OT_VQE_WORK_STATE_COMMON;
            stVqeConfig.agc_cfg.usr_mode              = TD_TRUE;
            stVqeConfig.agc_cfg.target_level          = -1;
            stVqeConfig.agc_cfg.max_gain              = 5;
            stVqeConfig.agc_cfg.adjust_speed          = 0;
            stVqeConfig.agc_cfg.use_hpf               = 3;
            stVqeConfig.agc_cfg.noise_floor           = -30;
            stVqeConfig.agc_cfg.improve_snr           = 1;
            stVqeConfig.agc_cfg.output_mode           = 0;
            stVqeConfig.agc_cfg.noise_suppress_switch = 1;
            stVqeConfig.open_mask                     = OT_AI_TALKVQE_MASK_AGC | OT_AI_TALKVQE_MASK_EQ;
            td_s8 s8VqeGain[10]                       = {-50, -50, 2, 3, 4, 4, 3, 0, 0, 0}; // 100 200 250 350 500 800 1.2k 2.5k 4k 8k
            memcpy(stVqeConfig.eq_cfg.gain_db, &s8VqeGain, sizeof(s8VqeGain));
            ss_mpi_ao_set_vqe_attr(AoDevId, i, &stVqeConfig);
            ss_mpi_ao_enable_vqe(AoDevId, i);
        }
    }

    s32Ret = vapi_aio_ao_set_resample(AoDevId, enInSampleRate);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_ao_set_resample(%d,%d) failed with %#x!\n", AoDevId, i, s32Ret);
        return TD_FAILURE;
    }

    if (g_stAioInfo.stAoDev[AoDevId].buffer == TD_NULL) {
        g_stAioInfo.stAoDev[AoDevId].buffer = (td_u8 *)malloc(MAX_BUFF_SIZE);
        if (g_stAioInfo.stAoDev[AoDevId].buffer == TD_NULL) {
            VAPILOG("no enoungh memory !\n");
            return TD_FAILURE;
        }
    }

    s32Ret = comm_mutex_init(&g_stAioInfo.stAoDev[AoDevId].mutex);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("comm_mutex_init(%d) failed with %#x\n", AoDevId, s32Ret);
        return s32Ret;
    }

    g_stAioInfo.stAoDev[AoDevId].enState = STATE_BUSY;

    return TD_SUCCESS;
}

static td_s32 vapi_aio_ao_stop(ot_audio_dev AoDevId)
{
    td_s32 i;
    td_s32 s32Ret;

    comm_mutex_lock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
        if (g_stAioInfo.stAoDev[AoDevId].bRspEn == TD_TRUE) {
            s32Ret = ss_mpi_ao_disable_resample(AoDevId, i);
            if (TD_SUCCESS != s32Ret) {
                VAPILOG("ss_mpi_ao_disable_resample(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
                comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
                return s32Ret;
            }
        }

        s32Ret = ss_mpi_ao_disable_chn(AoDevId, i);
        if (TD_SUCCESS != s32Ret) {
            VAPILOG("ss_mpi_ao_disable_chn(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
            comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
            return s32Ret;
        }
    }
    g_stAioInfo.stAoDev[AoDevId].ChnCnt         = 0;
    g_stAioInfo.stAoDev[AoDevId].enInSampleRate = 0;
    g_stAioInfo.stAoDev[AoDevId].bRspEn         = TD_FALSE;

    s32Ret = ss_mpi_ao_disable(AoDevId);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ao_disable(%d) failed with %#x\n", AoDevId, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return s32Ret;
    }

    if (g_stAioInfo.stAoDev[AoDevId].buffer != TD_NULL) {
        free(g_stAioInfo.stAoDev[AoDevId].buffer);
        g_stAioInfo.stAoDev[AoDevId].buffer = TD_NULL;
    }

    g_stAioInfo.stAoDev[AoDevId].enState = STATE_IDLE;

    comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    comm_mutex_uninit(&g_stAioInfo.stAoDev[AoDevId].mutex);

    return TD_SUCCESS;
}

static td_s32 vapi_aio_ai_set_gain(td_s32 VolumeDb)
{
    td_s32 s32Ret   = TD_SUCCESS;
    td_s32 fdAcodec = -1;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0) {
        VAPILOG("can't open Acodec,%s\n", ACODEC_FILE);
        return TD_FAILURE;
    }

    if (ioctl(fdAcodec, OT_ACODEC_SET_INPUT_VOLUME, &VolumeDb)) {
        VAPILOG("ACODEC_SET_INPUT_VOL(%d) failed \n", VolumeDb);
        s32Ret = TD_FAILURE;
    }

    close(fdAcodec);

    return s32Ret;
}

td_s32 vapi_aio_ai_set_volume(ot_audio_dev AiDevId, td_u8 value)
{
    td_s32 s32Ret;
    td_s32 VolumeDb;

    VolumeDb = value * (50 - (19)) / 100 + (19);
    s32Ret   = vapi_aio_ai_set_gain(VolumeDb);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("vapi_aio_ai(%d)_set_gain(%d) failed with %#x\n", AiDevId, value, s32Ret);
        return s32Ret;
    }

    return TD_SUCCESS;
}

td_s32 vapi_aio_ao_set_volume(ot_audio_dev AoDevId, td_u8 value)
{
    td_s32 s32Ret;
    td_s32 VolumeDb = value * (6 - (-121)) / 100 + (-121);

    s32Ret = ss_mpi_ao_set_volume(AoDevId, VolumeDb);
    if (s32Ret != TD_SUCCESS) {
        VAPILOG("ss_mpi_ao(%d)_set_volume(%d) failed with %#x\n", AoDevId, value, s32Ret);
        return s32Ret;
    }

    return TD_SUCCESS;
}

td_s32 vapi_aio_ai_read_frame(ot_audio_dev AiDevId, ot_audio_sample_rate enOutSampleRate, AENC_E enAenc, AIO_FRAME_S *pstFrame)
{
    td_s32         s32Ret;
    ot_audio_frame stFrm;
    ot_aec_frame   stAecFrm;
    unsigned char *dst;
    short         *src;
    unsigned int   srcSize;

    if (g_stAioInfo.stAiDev.enState == STATE_IDLE) {
        return TD_FAILURE;
    }

    comm_mutex_lock(&g_stAioInfo.stAiDev.mutex);

    if (g_stAioInfo.stAiDev.enState == STATE_IDLE) {
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return TD_FAILURE;
    }

    // reset resample rate if necessary
    s32Ret = vapi_aio_ai_set_resample(AiDevId, enOutSampleRate);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_ai_set_resample(%d, %d), failed with %#x!\n", AiDevId, enOutSampleRate, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return TD_FAILURE;
    }

    dst = g_stAioInfo.stAiDev.buffer;

    s32Ret = ss_mpi_ai_get_frame(AiDevId, 0, &stFrm, &stAecFrm, 100);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ai_get_frame(%d, %d), failed with %#x!\n", AiDevId, 0, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return TD_FAILURE;
    }

    src     = (short *)stFrm.virt_addr[0];
    srcSize = stFrm.len;
    if (srcSize > MAX_BUFF_SIZE) {
        VAPILOG("read date size(%d) out of buffer max size(%d) ", srcSize, MAX_BUFF_SIZE);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return TD_FAILURE;
    }

    switch (enAenc) {
    case AENC_PCM_ULAW:
        pstFrame->len    = vapi_aio_ulaw_encode(dst, src, srcSize);
        pstFrame->enAbit = ABIT_8_BIT;
        break;
    case AENC_PCM_ALAW:
        pstFrame->len    = vapi_aio_alaw_encode(dst, src, srcSize);
        pstFrame->enAbit = ABIT_8_BIT;
        break;
    case AENC_PCM_RAW:
    default:
        pstFrame->len    = srcSize;
        pstFrame->enAbit = ABIT_16_BIT;
        memcpy(dst, src, srcSize);
        break;
    }

    pstFrame->enArate = enOutSampleRate;
    pstFrame->enAenc  = enAenc;
    pstFrame->pts     = stFrm.time_stamp;
    pstFrame->date    = dst;

    /* finally you must release the stream */
    s32Ret = ss_mpi_ai_release_frame(AiDevId, 0, &stFrm, &stAecFrm);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("ss_mpi_ai_release_frame(%d, %d), failed with %#x!\n", AiDevId, 0, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return TD_FAILURE;
    }

    comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);

    return TD_SUCCESS;
}

static td_s32 vapi_aio_ao_get_multiple(AIO_FRAME_S *pstFrame)
{
    td_s32 multiple = 0;

    if (pstFrame->enAenc == AENC_PCM_ULAW || pstFrame->enAenc == AENC_PCM_ALAW || pstFrame->enAenc == AENC_PCM_AAC) {
        multiple = 2;
    } else if (pstFrame->enAenc == AENC_PCM_G722) {
        multiple = 4;
    } else if (pstFrame->enAenc == AENC_PCM_G726) {
        multiple = 8;
    } else if (pstFrame->enAenc == AENC_PCM_RAW) {
        if (pstFrame->enAbit == ABIT_8_BIT) {
            multiple = 2;
        } else if (pstFrame->enAbit == ABIT_16_BIT) {
            multiple = 1;
        }
    }

    return multiple;
}

td_s32 vapi_aio_ao_write_frame(ot_audio_dev AoDevId, AIO_FRAME_S *pstFrame)
{
    td_s32         s32Ret;
    td_s32         i;
    td_s32         multiple = 0;
    ot_audio_frame stData   = {0};
    short         *dst;
    unsigned char *src     = pstFrame->date;
    unsigned int   srcSize = pstFrame->len;

    // 3536a平台两个hdmi反过来用，这里音频也要反一下
    if (AoDevId == AO_MAIN_HDMI) {
        AoDevId = AO_SUB_HDMI;
    } else if (AoDevId == AO_SUB_HDMI) {
        AoDevId = AO_MAIN_HDMI;
    }

    comm_mutex_lock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    if (g_stAioInfo.stAoDev[AoDevId].enState == STATE_IDLE) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return TD_FAILURE;
    }

    multiple = vapi_aio_ao_get_multiple(pstFrame);

    if (srcSize * multiple > MAX_BUFF_SIZE) {
        VAPILOG("write date size(%d) out of buffer max size(%d) ", srcSize * multiple, MAX_BUFF_SIZE);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return TD_FAILURE;
    }
    // reset resample rate if necessary
    s32Ret = vapi_aio_ao_set_resample(AoDevId, pstFrame->enArate);
    if (TD_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_ao_set_resample(%d, %d), failed with %#x!\n", AoDevId, pstFrame->enArate, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return TD_FAILURE;
    }

    if (g_stAioInfo.stAoDev[AoDevId].buffer == TD_NULL) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return TD_FAILURE;
    }
    dst = (short *)g_stAioInfo.stAoDev[AoDevId].buffer;

    switch (pstFrame->enAenc) {
    case AENC_PCM_ULAW:
        stData.len = vapi_aio_ulaw_decode(dst, src, srcSize);
        break;
    case AENC_PCM_ALAW:
        stData.len = vapi_aio_alaw_decode(dst, src, srcSize);
        break;
    case AENC_PCM_AAC:
        stData.len = vapi_aio_aac_decode(dst, src, srcSize, pstFrame->enArate);
        break;
    case AENC_PCM_G722:
        stData.len = vapi_aio_g722_decode(dst, src, srcSize);
        break;
    case AENC_PCM_G726:
        stData.len = vapi_aio_g726_decode(dst, src, srcSize);
        break;
    case AENC_PCM_RAW:
    default:
        stData.len = srcSize;
        memcpy(dst, src, srcSize);
        break;
    }

    if (stData.len < 0) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return TD_FAILURE;
    }

    if (pstFrame->enAenc == AENC_PCM_RAW && pstFrame->enAbit == ABIT_8_BIT) {
        for (i = 0; i < srcSize; i++) {
            dst[i] = (short)(src[i] - 0x80) << 8;
        }
        stData.len = 2 * srcSize;
    }

    stData.bit_width    = OT_AUDIO_BIT_WIDTH_16;
    stData.snd_mode     = OT_AUDIO_SOUND_MODE_MONO;
    stData.time_stamp   = pstFrame->pts;
    stData.virt_addr[0] = dst;

    for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
        s32Ret = ss_mpi_ao_send_frame(AoDevId, i, &stData, 0);
        if (TD_SUCCESS != s32Ret) {
            if (OT_ERR_AO_BUF_FULL == s32Ret) {
                ss_mpi_ao_clr_chn_buf(AoDevId, i);
            }

            VAPILOG("ss_mpi_ao_send_frame(%d, %d), stData.u32Len(%d), failed with %#x! \n", AoDevId, i, stData.len, s32Ret);
            comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
            return TD_FAILURE;
        }
    }

    comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    return TD_SUCCESS;
}

td_bool vapi_aio_is_support_talk()
{
    return TD_TRUE;
}

td_s32 vapi_aio_init()
{
    memset(&g_stAioInfo, 0, sizeof(AIO_INFO_S));

    vapi_aio_ai_start(0, OT_AUDIO_SAMPLE_RATE_16000, 1, OT_AUDIO_SAMPLE_RATE_16000);
    vapi_aio_ao_start(AO_SPK_MIC, OT_AUDIO_SAMPLE_RATE_16000, 2, OT_AUDIO_SAMPLE_RATE_16000);
    vapi_aio_ao_start(AO_MAIN_HDMI, OT_AUDIO_SAMPLE_RATE_16000, 2, OT_AUDIO_SAMPLE_RATE_32000);
    vapi_aio_ao_start(AO_SUB_HDMI, OT_AUDIO_SAMPLE_RATE_16000, 2, OT_AUDIO_SAMPLE_RATE_32000);
    vapi_aio_aac_init();

    return TD_SUCCESS;
}

td_s32 vapi_aio_uninit()
{
    vapi_aio_aac_uninit();
    vapi_aio_ai_stop(0);
    vapi_aio_ao_stop(AO_SPK_MIC);
    vapi_aio_ao_stop(AO_MAIN_HDMI);
    vapi_aio_ao_stop(AO_SUB_HDMI);

    return TD_SUCCESS;
}
