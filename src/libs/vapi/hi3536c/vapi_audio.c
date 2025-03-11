/*
 * ***************************************************************
 * Filename:        vapi_audio.c
 * Created at:      2015.10.21
 * Description:     audio controller
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "vapi_comm.h"
#include "neaacdec.h"
#include "vapi.h"
#include "g722_codec.h"
#include "g726_codec.h"

#define ACODEC_FILE     "/dev/nau88c10"
#define MAX_BUFF_SIZE   5120

typedef struct dev_s {
    STATE_E                 enState;
    HI_U32                  ChnCnt;
    AUDIO_SAMPLE_RATE_E     enInSampleRate;
    AUDIO_SAMPLE_RATE_E     enOutSampleRate;
    HI_BOOL                 bVqeEn;
    HI_BOOL                 bRspEn;
    HI_U8                   *buffer;
    pthread_mutex_t          mutex;
} DEV_S;

typedef struct aio_info_s {
    DEV_S    stAiDev;
    DEV_S    stAoDev[AO_TYPE_NUM];
    NeAACDecHandle hDecoder;
    HI_BOOL  bTalk;
} AIO_INFO_S;

static AIO_INFO_S g_stAioInfo;

static inline unsigned char ALawEncode(short pcm16)
{
    int p = pcm16;
    unsigned a;  // A-law value we are forming

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

    a += p;  // a now equal to encoded A-law value

    return a ^ 0x55; // A-law has alternate bits inverted for transmission
}

static inline int ALawDecode(unsigned char alaw)
{
    unsigned sign;
    int linear;
    unsigned shift;

    alaw ^= 0x55;  // A-law has alternate bits inverted for transmission
    sign = alaw & 0x80;
    linear = alaw & 0x1f;
    linear <<= 4;
    linear += 8;  // Add a 'half' bit (0x08) to place PCM value in middle of range
    alaw &= 0x7f;

    if (alaw >= 0x20) {
        linear |= 0x100;  // Put in MSB
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
    int p = pcm16;
    unsigned u;  // u-law value we are forming

    if (p < 0) {
        // -ve value
        // Note, ones compliment is used here as this keeps encoding symetrical
        // and equal spaced around zero cross-over, (it also matches the standard).
        p = ~p;
        u = 0x80 ^ 0x10 ^ 0xff; // Sign bit = 1 (^0x10 because this will get inverted later) ^0xff to invert final u-Law code
    } else {
        // +ve value
        u = 0x00 ^ 0x10 ^
            0xff; // Sign bit = 0 (-0x10 because this amount extra will get added later) ^0xff to invert final u-Law code
    }

    p += 0x84; // Add uLaw bias

    if (p > 0x7f00) {
        p = 0x7f00;  // Clip to 15 bits
    }

    // Calculate segment and interval numbers
    p >>= 3;    // Shift down to 13bit
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
    int linear;
    unsigned shift;

    ulaw ^= 0xff;  // u-law has all bits inverted for transmission

    linear = ulaw & 0x0f;
    linear <<= 3;
    linear |= 0x84;  // Set MSB (0x80) and a 'half' bit (0x04) to place PCM value in middle of range

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
    unsigned ulaw;

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
    unsigned alaw;

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
    //return reco_rtsp_g722_decode(dst, src, srcSize);
}

unsigned int vapi_aio_g726_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    return g726_decode(dst, src, srcSize);
    //return reco_rtsp_g726_decode(dst, src, srcSize);
}

static int vapi_aio_aac_reopen(ARATE_E enRate)
{
    if (g_stAioInfo.hDecoder) {
        NeAACDecClose(g_stAioInfo.hDecoder);
        g_stAioInfo.hDecoder = NULL;
    }
    NeAACDecConfigurationPtr pstConfig;
    g_stAioInfo.hDecoder = NeAACDecOpen();
    pstConfig = NeAACDecGetCurrentConfiguration(g_stAioInfo.hDecoder);
    pstConfig->defObjectType = 0;
    pstConfig->outputFormat = 1;
    pstConfig->defSampleRate = enRate;
    pstConfig->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(g_stAioInfo.hDecoder, pstConfig);
    VAPILOG("vapi_aio_aac_reopen:%p sampleRate:%d\n", g_stAioInfo.hDecoder, enRate);
    return 0;
}
unsigned int vapi_aio_aac_decode(short *dst, const unsigned char *src, unsigned int srcSize, ARATE_E enRate)
{
    static NeAACDecFrameInfo frameInfo;
    static ARATE_E lastRate = 0;
    static short *pBuffer;
    static const unsigned char *lastSrc = 0;
    unsigned long samplerate = 0;
    unsigned char channels = 0;
    int i;

    if (lastRate != enRate) {
        vapi_aio_aac_reopen(enRate);
        NeAACDecInit(g_stAioInfo.hDecoder, (unsigned char *)src,  srcSize, &samplerate, &channels);
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
    //�䨮??�����̨���?��y?Y?D������?�̣������̨�
    for (i = 0; i < frameInfo.samples; i++) {
        dst[i] = pBuffer[i * frameInfo.channels];
    }

    return frameInfo.samples;
}


static int vapi_aio_aac_init(void)
{
#if 0
    NeAACDecConfigurationPtr pstConfig;

    g_stAioInfo.hDecoder = NeAACDecOpen();
    pstConfig = NeAACDecGetCurrentConfiguration(g_stAioInfo.hDecoder);
    pstConfig->defObjectType = 0;
    pstConfig->outputFormat = 1;
    pstConfig->defSampleRate = 8000;
    pstConfig->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(g_stAioInfo.hDecoder, pstConfig);
#else
    g_stAioInfo.hDecoder = NULL;
#endif

    return HI_SUCCESS;
}

static void vapi_aio_aac_uninit(void)
{
    NeAACDecClose(g_stAioInfo.hDecoder);
}

static inline HI_S32  vapi_aio_ai_set_resample(AUDIO_DEV AiDevId, AUDIO_SAMPLE_RATE_E enOutSampleRate)
{
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;

    if (enOutSampleRate != g_stAioInfo.stAiDev.enInSampleRate
        && enOutSampleRate != g_stAioInfo.stAiDev.enOutSampleRate) {
        for (i = 0; i < g_stAioInfo.stAiDev.ChnCnt; i++) {
            if (g_stAioInfo.stAiDev.bRspEn == HI_TRUE) {
                s32Ret = HI_MPI_AI_DisableReSmp(AiDevId, i);
            }
            s32Ret |= HI_MPI_AI_EnableReSmp(AiDevId, i, enOutSampleRate);
            if (s32Ret) {
                VAPILOG("HI_MPI_AI_EnableReSmp(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                return s32Ret;
            }
        }
        g_stAioInfo.stAiDev.bRspEn = HI_TRUE;
    } else if (enOutSampleRate == g_stAioInfo.stAiDev.enInSampleRate
               && g_stAioInfo.stAiDev.bRspEn == HI_TRUE)

    {
        for (i = 0; i < g_stAioInfo.stAiDev.ChnCnt; i++) {
            s32Ret |= HI_MPI_AI_DisableReSmp(AiDevId, i);
        }
        g_stAioInfo.stAiDev.bRspEn = HI_FALSE;
    }

    g_stAioInfo.stAiDev.enOutSampleRate = enOutSampleRate;

    return HI_SUCCESS;
}

static inline HI_S32  vapi_aio_ao_set_resample(AUDIO_DEV AoDevId, AUDIO_SAMPLE_RATE_E enInSampleRate)
{
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;

    if (enInSampleRate != g_stAioInfo.stAoDev[AoDevId].enOutSampleRate
        && enInSampleRate != g_stAioInfo.stAoDev[AoDevId].enInSampleRate) {
        for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
            if (g_stAioInfo.stAoDev[AoDevId].bRspEn == HI_TRUE) {
                s32Ret = HI_MPI_AO_DisableReSmp(AoDevId, i);
            }
            s32Ret |= HI_MPI_AO_EnableReSmp(AoDevId, i, enInSampleRate);
            if (s32Ret) {
                VAPILOG("HI_MPI_AO_EnableReSmp(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
                return s32Ret;
            }
            s32Ret |= HI_MPI_AO_ClearChnBuf(AoDevId, i);
            if (s32Ret) {
                VAPILOG("HI_MPI_AO_ClearChnBuf(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
                return s32Ret;
            }
        }
        g_stAioInfo.stAoDev[AoDevId].bRspEn = HI_TRUE;
    } else if (enInSampleRate == g_stAioInfo.stAoDev[AoDevId].enOutSampleRate
               && g_stAioInfo.stAoDev[AoDevId].bRspEn == HI_TRUE) {
        for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
            s32Ret |= HI_MPI_AO_DisableReSmp(AoDevId, i);
        }
        g_stAioInfo.stAoDev[AoDevId].bRspEn = HI_FALSE;
    }

    g_stAioInfo.stAoDev[AoDevId].enInSampleRate = enInSampleRate;

    return HI_SUCCESS;
}

#if 0
static HI_S32 vapi_aio_set_codec(AUDIO_SAMPLE_RATE_E enSample)
{
    HI_S32 fdAcodec = -1;
    HI_S32 ret = HI_SUCCESS;
    HI_U32 i2s_fs_sel = 0;
    HI_U32 InputMode = 1;
    HI_U32 OutputMode = 0;

    HI_U32 mixer_mic_ctrl = ACODEC_MIXER_LINEIN;
    HI_U32 output_ctrl = ACODEC_LINEOUTD_NONE;
    HI_U32 gain_mic = 0;
    HI_U32 mute = 0;
    HI_S32 pd = 0;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0) {
        VAPILOG("can't open Acodec,%s\n", ACODEC_FILE);
        ret = HI_FAILURE;
    }
    if (ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL)) {
        VAPILOG("Reset audio codec error\n");
    }

    if ((AUDIO_SAMPLE_RATE_8000 == enSample)
        || (AUDIO_SAMPLE_RATE_11025 == enSample)
        || (AUDIO_SAMPLE_RATE_12000 == enSample)) {
        i2s_fs_sel = 0x18;
    } else if ((AUDIO_SAMPLE_RATE_16000 == enSample)
               || (AUDIO_SAMPLE_RATE_22050 == enSample)
               || (AUDIO_SAMPLE_RATE_24000 == enSample)) {
        i2s_fs_sel = 0x19;
    } else if ((AUDIO_SAMPLE_RATE_32000 == enSample)
               || (AUDIO_SAMPLE_RATE_44100 == enSample)
               || (AUDIO_SAMPLE_RATE_48000 == enSample)) {
        i2s_fs_sel = 0x1a;
    } else {
        VAPILOG("not support enSample:%d\n", enSample);
        ret = HI_FAILURE;
    }

    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel)) {
        VAPILOG("set acodec sample rate[%d] failed\n", enSample);
        ret = HI_FAILURE;
    }

    switch (InputMode) {
        case 0: {
#if 1
            mixer_mic_ctrl = ACODEC_MIXER_MICIN;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl)) {
                VAPILOG(" set acodec ACODEC_SET_MIXER_MIC failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }
#endif

            /* set volume plus (0~0x1f,default 0) */
            gain_mic = 0x10;
            if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &gain_mic)) {
                VAPILOG(" set acodec ACODEC_SET_GAIN_MICL failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }
            if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &gain_mic)) {
                VAPILOG(" set acodec ACODEC_SET_GAIN_MICL failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

        }
        break;
        case 1: {
            mixer_mic_ctrl = ACODEC_MIXER_LINEIN;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl)) {
                VAPILOG(" set acodec ACODEC_SET_MIXER_MIC failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }
            mute = 0;
            if (ioctl(fdAcodec, ACODEC_SET_MICL_MUTE, &mute)) {
                VAPILOG(" set acodec ACODEC_SET_MICL_MUTE failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }
            if (ioctl(fdAcodec, ACODEC_SET_MICR_MUTE, &mute)) {
                VAPILOG(" set acodec ACODEC_SET_MICR_MUTE failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

        }
        break;
        case 2: {
            mixer_mic_ctrl = ACODEC_MIXER_MICIN_D;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl)) {
                VAPILOG(" set acodec ACODEC_SET_MIXER_MIC failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

        }
        break;
        case 3: {
            mixer_mic_ctrl = ACODEC_MIXER_LINEIN_D;
            if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl)) {
                VAPILOG(" set acodec ACODEC_SET_MIXER_MIC failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

        }
        break;
        default: {
            VAPILOG(" acodec input mod wrong!\n");
            close(fdAcodec);
            return HI_FAILURE;
        }

    }

    switch (OutputMode) {
        case 0: {

            output_ctrl = ACODEC_LINEOUTD_NONE;
            if (ioctl(fdAcodec, ACODEC_SET_DAC_LINEOUTD, &output_ctrl)) {
                VAPILOG(" set acodec ACODEC_SET_DAC_LINEOUTD failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

            mute = 1;
            if (ioctl(fdAcodec, ACODEC_SET_DACD_MUTE, &mute)) {
                VAPILOG(" set acodec ACODEC_SET_DACD_MUTE failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

            //pd = 1;
            //ioctl(fdAcodec, ACODEC_SET_PD_LINEOUTD, &pd);

        }
        break;
        case 1: {
            output_ctrl = ACODEC_LINEOUTD_LEFT;
            if (ioctl(fdAcodec, ACODEC_SET_DAC_LINEOUTD, &output_ctrl)) {
                VAPILOG(" set acodec ACODEC_SET_DAC_LINEOUTD failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }
            mute = 0;
            if (ioctl(fdAcodec, ACODEC_SET_DACD_MUTE, &mute)) {
                VAPILOG(" set acodec ACODEC_SET_DACD_MUTE failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

            pd = 0;
            ioctl(fdAcodec, ACODEC_SET_PD_LINEOUTD, &pd);

        }
        break;
        case 2: {
            output_ctrl = ACODEC_LINEOUTD_RIGHT;
            if (ioctl(fdAcodec, ACODEC_SET_DAC_LINEOUTD, &output_ctrl)) {
                VAPILOG(" set acodec ACODEC_SET_DAC_LINEOUTD failed\n");
                close(fdAcodec);
                return HI_FAILURE;
            }

        }
        break;
        default: {
            VAPILOG(" acodec input mod wrong!\n");
            close(fdAcodec);
            return HI_FAILURE;
        }

    }

    close(fdAcodec);

    return ret;
}
#else

static HI_S32 vapi_aio_set_codec(AUDIO_SAMPLE_RATE_E enSample)
{
    HI_S32 fdAcodec = -1;
    HI_S32 s32Samplerate;
//  HI_BOOL bMaster = 0;    //0:slaver
//  int bPCMmode = 0;       //0:I2S
    Nau88c10_Ctrl ctrl;

    if (AUDIO_SAMPLE_RATE_8000 == enSample) {
        s32Samplerate = 0x5;
    } else if (AUDIO_SAMPLE_RATE_12000 == enSample) {
        s32Samplerate = 0x4;
    } else if (AUDIO_SAMPLE_RATE_16000 == enSample) {
        s32Samplerate = 0x3;
    } else if (AUDIO_SAMPLE_RATE_24000 == enSample) {
        s32Samplerate = 0x2;
    } else if (AUDIO_SAMPLE_RATE_32000 == enSample) {
        s32Samplerate = 0x1;
    } else if (AUDIO_SAMPLE_RATE_48000 == enSample) {
        s32Samplerate = 0x0;
    } else {
        printf("not support enSample:%d\n", enSample);
        return HI_FAILURE;
    }

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0) {
        VAPILOG("can't open Acodec,%s\n", ACODEC_FILE);
        return HI_FAILURE;
    }
    ioctl(fdAcodec, SOFT_RESET);
    /* data bit width (0:16bit 1:20bit 2:24bit 3:32bit) */
    ctrl.data_length = 0;
    ioctl(fdAcodec, SET_DATA_LENGTH, &ctrl);
    /*set sample of DAC and ADC */
    ctrl.sample = s32Samplerate;
    ioctl(fdAcodec, SET_DAC_SAMPLE, &ctrl);
    ioctl(fdAcodec, SET_ADC_SAMPLE, &ctrl);
    close(fdAcodec);

    return 0;
}

#endif

static HI_S32 vapi_aio_ai_start(AUDIO_DEV AiDevId, AUDIO_SAMPLE_RATE_E enInSampleRate,
                                HI_U32 u32ChnCnt, AUDIO_SAMPLE_RATE_E enOutSamplerate)
{
    HI_S32 i;
    HI_S32 s32Ret;
    AIO_ATTR_S stAttr;
    AI_VQE_CONFIG_S stVqeConfig;
    AI_CHN_PARAM_S stAiChnPara;

    s32Ret = vapi_aio_set_codec(enInSampleRate);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_set_codec(%d) failed with %#x\n", enInSampleRate, s32Ret);
        return s32Ret;
    }

    stAttr.enSamplerate   = enInSampleRate;
    stAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAttr.u32EXFlag      = 1;
    stAttr.u32FrmNum      = 30;
    stAttr.u32PtNumPerFrm = stAttr.enSamplerate / 100 * 4;
    stAttr.u32ChnCnt      = u32ChnCnt;
    stAttr.u32ClkChnCnt   = u32ChnCnt;
    stAttr.u32ClkSel      = 1;

    s32Ret = HI_MPI_AI_SetPubAttr(AiDevId, &stAttr);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AI_SetPubAttr(%d) failed with %#x\n", AiDevId, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_MPI_AI_Enable(AiDevId);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AI_Enable(%d) failed with %#x\n", AiDevId, s32Ret);
        return s32Ret;
    }

    g_stAioInfo.stAiDev.enInSampleRate = enInSampleRate;

    //debug voice quality enhancement
    g_stAioInfo.stAiDev.bVqeEn = HI_TRUE;

    for (i = 0; i < u32ChnCnt; i++) {
        s32Ret = HI_MPI_AI_EnableChn(AiDevId, i);
        if (HI_SUCCESS != s32Ret) {
            VAPILOG("HI_MPI_AI_EnableChn(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            return s32Ret;
        }
        g_stAioInfo.stAiDev.ChnCnt++;

        s32Ret = HI_MPI_AI_GetChnParam(AiDevId, i, &stAiChnPara);
        if (HI_SUCCESS != s32Ret) {
            VAPILOG("HI_MPI_AI_GetChnParam(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            return s32Ret;
        }

        stAiChnPara.u32UsrFrmDepth = 30;
        s32Ret = HI_MPI_AI_SetChnParam(AiDevId, i, &stAiChnPara);
        if (HI_SUCCESS != s32Ret) {
            VAPILOG("HI_MPI_AI_SetChnParam(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            return s32Ret;
        }

        if (g_stAioInfo.stAiDev.bVqeEn == HI_TRUE) {
            memset(&stVqeConfig, 0, sizeof(AI_VQE_CONFIG_S));
            stVqeConfig.s32WorkSampleRate   = stAttr.enSamplerate;
            stVqeConfig.s32FrameSample      = stAttr.u32PtNumPerFrm;
            stVqeConfig.enWorkstate         = VQE_WORKSTATE_COMMON;
            stVqeConfig.bAecOpen            = HI_TRUE;  //AEC
            stVqeConfig.stAecCfg.bUsrMode   = HI_FALSE;
            stVqeConfig.bAnrOpen            = HI_TRUE;  //ANR
            stVqeConfig.stAnrCfg.bUsrMode   = HI_TRUE;
            stVqeConfig.stAnrCfg.s16NrIntensity = 8;
            stVqeConfig.stAnrCfg.s16NoiseDbThr  = 60;
            stVqeConfig.stAnrCfg.s8SpProSwitch  = 0;
            stVqeConfig.bHpfOpen = HI_FALSE;            //HPF
            stVqeConfig.bAgcOpen = HI_TRUE;             //AGC
            stVqeConfig.stAgcCfg.bUsrMode = HI_TRUE;
            stVqeConfig.stAgcCfg.s8TargetLevel = -10;
            stVqeConfig.stAgcCfg.s8MaxGain = 3;
            stVqeConfig.stAgcCfg.s8AdjustSpeed = 10;
            stVqeConfig.stAgcCfg.s8UseHighPassFilt = 3; //1:80Hz 2:120Hz  3:150Hz
            stVqeConfig.stAgcCfg.s8NoiseFloor = -30;
            stVqeConfig.stAgcCfg.s8ImproveSNR = 1;
            stVqeConfig.stAgcCfg.s8OutputMode = 0;
            stVqeConfig.stAgcCfg.s16NoiseSupSwitch = 1;
            stVqeConfig.bEqOpen = HI_TRUE;
            HI_S8 s8VqeGain[10] = {-50, -50, -5, 8, 8, 8, 8, 8, 8, -50};//100 200 250 350 500 800 1.2k 2.5k 4k 8k
            memcpy(stVqeConfig.stEqCfg.s8GaindB, &s8VqeGain, sizeof(s8VqeGain));

            s32Ret = HI_MPI_AI_SetVqeAttr(AiDevId, i, AO_SPK_MIC, i, &stVqeConfig);
            if (HI_SUCCESS != s32Ret) {
                VAPILOG("HI_MPI_AI_SetVqeAttr(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                return s32Ret;
            }
            s32Ret = HI_MPI_AI_EnableVqe(AiDevId, i);
            if (HI_SUCCESS != s32Ret) {
                VAPILOG("HI_MPI_AI_EnableVqe(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                return s32Ret;
            }
            HI_MPI_AI_SetVqeVolume(AiDevId, i, 10);
        }
    }

    s32Ret = vapi_aio_ai_set_resample(AiDevId, enOutSamplerate);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_ai_set_resample(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
        return s32Ret;
    }

    if (g_stAioInfo.stAiDev.buffer == HI_NULL) {
        g_stAioInfo.stAiDev.buffer = (HI_U8 *)malloc(MAX_BUFF_SIZE);
        if (g_stAioInfo.stAiDev.buffer == HI_NULL) {
            VAPILOG("no enoungh memory !\n");
            return HI_FAILURE;
        }
    }

    s32Ret = comm_mutex_init(&g_stAioInfo.stAiDev.mutex);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("comm_mutex_init(%d) failed with %#x\n", AiDevId, s32Ret);
        return s32Ret;
    }

    g_stAioInfo.stAiDev.enState = STATE_BUSY;

    return HI_SUCCESS;
}

static HI_S32 vapi_aio_ai_stop(AUDIO_DEV AiDevId)
{
    HI_S32 i;
    HI_S32 s32Ret;

    comm_mutex_lock(&g_stAioInfo.stAiDev.mutex);

    for (i = 0; i < g_stAioInfo.stAiDev.ChnCnt; i++) {
        if (g_stAioInfo.stAiDev.bRspEn == HI_TRUE) {
            s32Ret = HI_MPI_AI_DisableReSmp(AiDevId, i);
            if (HI_SUCCESS != s32Ret) {
                VAPILOG("vapi_aio_ai_set_resample(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
                return s32Ret;
            }
        }

        if (g_stAioInfo.stAiDev.bVqeEn == HI_TRUE) {
            s32Ret = HI_MPI_AI_DisableVqe(AiDevId, i);
            if (HI_SUCCESS != s32Ret) {
                VAPILOG("HI_MPI_AI_DisableVqe(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
                comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
                return s32Ret;
            }
        }

        s32Ret = HI_MPI_AI_DisableChn(AiDevId, i);
        if (HI_SUCCESS != s32Ret) {
            VAPILOG("HI_MPI_AI_DisableChn(%d,%d) failed with %#x\n", AiDevId, i, s32Ret);
            comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
            return s32Ret;
        }
    }
    g_stAioInfo.stAiDev.ChnCnt = 0;
    g_stAioInfo.stAiDev.enOutSampleRate = 0;
    g_stAioInfo.stAiDev.bRspEn = HI_FALSE;

    s32Ret = HI_MPI_AI_Disable(AiDevId);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AI_Disable(%d) failed with %#x\n", AiDevId, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return s32Ret;
    }

    if (g_stAioInfo.stAiDev.buffer != HI_NULL) {
        free(g_stAioInfo.stAiDev.buffer);
        g_stAioInfo.stAiDev.buffer = HI_NULL;
    }

    g_stAioInfo.stAiDev.enState = STATE_IDLE;

    comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);

    comm_mutex_uninit(&g_stAioInfo.stAiDev.mutex);

    return HI_SUCCESS;
}

static HI_S32 vapi_aio_ao_start(AUDIO_DEV AoDevId, AUDIO_SAMPLE_RATE_E enInSampleRate,
                                HI_U32 u32ChnCnt, AUDIO_SAMPLE_RATE_E enOutSampleRate)
{
    HI_S32 i;
    HI_S32 s32Ret;
    AIO_ATTR_S stAttr;
    AO_VQE_CONFIG_S stVqeConfig;

    stAttr.enSamplerate   = enOutSampleRate;
    stAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAttr.u32EXFlag      = 1;
    stAttr.u32FrmNum      = 30;
    stAttr.u32PtNumPerFrm = enOutSampleRate / 100 * 4;
    stAttr.u32ChnCnt      = u32ChnCnt;
    stAttr.u32ClkChnCnt   = u32ChnCnt;
    stAttr.u32ClkSel      = 1;

    s32Ret = HI_MPI_AO_SetPubAttr(AoDevId, &stAttr);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AO_SetPubAttr(%d) failed with %#x!\n", AoDevId, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_AO_Enable(AoDevId);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AO_Enable(%d) failed with %#x!\n", AoDevId, s32Ret);
        return HI_FAILURE;
    }

    g_stAioInfo.stAoDev[AoDevId].enOutSampleRate = enOutSampleRate;

    for (i = 0; i < u32ChnCnt; i++) {
        s32Ret = HI_MPI_AO_EnableChn(AoDevId, i);
        if (HI_SUCCESS != s32Ret) {
            VAPILOG("HI_MPI_AO_EnableChn(%d) failed with %#x!\n", i, s32Ret);
            return HI_FAILURE;
        }

        g_stAioInfo.stAoDev[AoDevId].ChnCnt++;

        if (enOutSampleRate != AUDIO_SAMPLE_RATE_8000
            && enOutSampleRate != AUDIO_SAMPLE_RATE_16000) {
            continue;
        }

        memset(&stVqeConfig, 0, sizeof(AO_VQE_CONFIG_S));
        stVqeConfig.s32WorkSampleRate           = stAttr.enSamplerate;
        stVqeConfig.s32FrameSample              = stAttr.u32PtNumPerFrm;
        stVqeConfig.enWorkstate                 = VQE_WORKSTATE_COMMON;
        stVqeConfig.bHpfOpen                    = HI_FALSE;
        stVqeConfig.bAnrOpen                    = HI_FALSE;
        stVqeConfig.bAgcOpen                    = HI_TRUE;
        stVqeConfig.stAgcCfg.bUsrMode           = HI_TRUE;
        stVqeConfig.stAgcCfg.s8TargetLevel      = -1;
        stVqeConfig.stAgcCfg.s8MaxGain          = 5;
        stVqeConfig.stAgcCfg.s8AdjustSpeed      = 0;
        stVqeConfig.stAgcCfg.s8UseHighPassFilt  = 3;
        stVqeConfig.stAgcCfg.s8NoiseFloor       = -30;
        stVqeConfig.stAgcCfg.s8ImproveSNR       = 1;
        stVqeConfig.stAgcCfg.s8OutputMode       = 0;
        stVqeConfig.stAgcCfg.s16NoiseSupSwitch  = 1;
        stVqeConfig.bEqOpen = HI_TRUE;
        HI_S8 s8VqeGain[10] = {-50, -50, 2, 3, 4, 4, 3, 0, 0, 0};//100 200 250 350 500 800 1.2k 2.5k 4k 8k
        memcpy(stVqeConfig.stEqCfg.s8GaindB, &s8VqeGain, sizeof(s8VqeGain));
        HI_MPI_AO_SetVqeAttr(AoDevId, i, &stVqeConfig);
        HI_MPI_AO_EnableVqe(AoDevId, i);
    }

    s32Ret = vapi_aio_ao_set_resample(AoDevId, enInSampleRate);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AO_EnableReSmp(%d,%d) failed with %#x!\n", AoDevId, i, s32Ret);
        return HI_FAILURE;
    }

    if (g_stAioInfo.stAoDev[AoDevId].buffer == HI_NULL) {
        g_stAioInfo.stAoDev[AoDevId].buffer = (HI_U8 *)malloc(MAX_BUFF_SIZE);
        memset(g_stAioInfo.stAoDev[AoDevId].buffer, 0, MAX_BUFF_SIZE);
        if (g_stAioInfo.stAoDev[AoDevId].buffer == HI_NULL) {
            VAPILOG("no enoungh memory !\n");
            return HI_FAILURE;
        }
    }

    s32Ret = comm_mutex_init(&g_stAioInfo.stAoDev[AoDevId].mutex);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("comm_mutex_init(%d) failed with %#x\n", AoDevId, s32Ret);
        return s32Ret;
    }

    g_stAioInfo.stAoDev[AoDevId].enState = STATE_BUSY;

    return HI_SUCCESS;
}

static HI_S32 vapi_aio_ao_stop(AUDIO_DEV AoDevId)
{
    HI_S32 i;
    HI_S32 s32Ret;

    comm_mutex_lock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
        if (g_stAioInfo.stAoDev[AoDevId].bRspEn == HI_TRUE) {
            s32Ret = HI_MPI_AO_DisableReSmp(AoDevId, i);
            if (HI_SUCCESS != s32Ret) {
                VAPILOG("HI_MPI_AO_DisableReSmp(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
                comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
                return s32Ret;
            }
        }
        if (AoDevId == AO_SPK_MIC) {
            HI_MPI_AO_DisableVqe(AoDevId, i);
        }
        s32Ret = HI_MPI_AO_DisableChn(AoDevId, i);
        if (HI_SUCCESS != s32Ret) {
            VAPILOG("HI_MPI_AO_DisableChn(%d,%d) failed with %#x\n", AoDevId, i, s32Ret);
            comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
            return s32Ret;
        }
    }
    g_stAioInfo.stAoDev[AoDevId].ChnCnt = 0;
    g_stAioInfo.stAoDev[AoDevId].enInSampleRate = 0;
    g_stAioInfo.stAoDev[AoDevId].bRspEn = HI_FALSE;

    s32Ret = HI_MPI_AO_Disable(AoDevId);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AO_Disable(%d) failed with %#x\n", AoDevId, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return s32Ret;
    }

    if (g_stAioInfo.stAoDev[AoDevId].buffer != HI_NULL) {
        free(g_stAioInfo.stAoDev[AoDevId].buffer);
        g_stAioInfo.stAoDev[AoDevId].buffer = HI_NULL;
    }

    g_stAioInfo.stAoDev[AoDevId].enState = STATE_IDLE;

    comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    comm_mutex_uninit(&g_stAioInfo.stAoDev[AoDevId].mutex);

    return HI_SUCCESS;
}

HI_S32 vapi_aio_ao_set_mute(HI_S32 devId, HI_S32 mute)
{
    HI_S32 fd = -1;
    HI_S32 ret = HI_TRUE;
    Nau88c10_Ctrl ctrl;

    if (devId == AO_SPK_MIC) {
        ctrl.mono_mute = (HI_U32)mute;

        fd = open(ACODEC_FILE, O_RDWR);
        if (fd < 0) {
            VAPILOG("can't open Acodec,%s\n", ACODEC_FILE);
            ret = HI_FAILURE;
        }
        if (ioctl(fd, MONO_OUTPUT_MUTE, &ctrl)) {
            VAPILOG("vapi_aio_ao_set_mute failed \n");
            ret = HI_FAILURE;
        }
        close(fd);
    }

    return ret;
}

static HI_S32 vapi_aio_ai_set_gain(HI_S32 VolumeDb)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_S32 fdAcodec = -1;

    fdAcodec = open(ACODEC_FILE, O_RDWR);
    if (fdAcodec < 0) {
        VAPILOG("can't open Acodec,%s\n", ACODEC_FILE);
        return HI_FAILURE;
    }

    if (ioctl(fdAcodec, ACODEC_SET_INPUT_VOL, &VolumeDb)) {
        VAPILOG("ACODEC_SET_INPUT_VOL(%d) failed \n", VolumeDb);
        s32Ret = HI_FAILURE;
    }

    close(fdAcodec);

    return s32Ret;
}

HI_S32 vapi_aio_ai_set_volume(AUDIO_DEV AiDevId, HI_U8 value)
{
    HI_S32 s32Ret;
    HI_S32 i;
    HI_S32 VolumeDb;

    if (g_stAioInfo.stAiDev.bRspEn == HI_TRUE) {
        //volumeDb [-20,10]
        VolumeDb = value * (10 - (-20)) / 100 + (-20);
        for (i = 0; i < g_stAioInfo.stAiDev.ChnCnt; i++) {
            s32Ret = HI_MPI_AI_SetVqeVolume(AiDevId, i, VolumeDb);
            if (s32Ret != HI_SUCCESS) {
                VAPILOG("HI_MPI_AI(%d)_SetVqeVolume(%d) failed with %#x\n", AiDevId, value, s32Ret);
                return s32Ret;
            }
        }
    } else {
        //volumeDb [-79,80] , recommand [19,50]
        VolumeDb = value * (50 - (19)) / 100 + (19);
        s32Ret = vapi_aio_ai_set_gain(VolumeDb);
        if (s32Ret != HI_SUCCESS) {
            VAPILOG("vapi_aio_ai(%d)_set_gain(%d) failed with %#x\n", AiDevId, value, s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;
}

HI_S32 vapi_aio_ao_set_volume(AUDIO_DEV AoDevId, HI_U8 value)
{
    HI_S32 s32Ret;
    HI_S32 VolumeDb = value * (6 - (-121)) / 100 + (-121);

    s32Ret = HI_MPI_AO_SetVolume(AoDevId, VolumeDb);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_MPI_AO(%d)_SetVolume(%d) failed with %#x\n", AoDevId, value, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

HI_S32 vapi_aio_ai_read_frame(AUDIO_DEV AiDevId, AUDIO_SAMPLE_RATE_E enOutSampleRate, AENC_E enAenc,
                              AIO_FRAME_S *pstFrame)
{
    HI_S32 s32Ret;
    AUDIO_FRAME_S stFrm;
    AEC_FRAME_S stAecFrm;
    unsigned char *dst;
    short *src;
    unsigned int srcSize;

    if (g_stAioInfo.stAiDev.enState == STATE_IDLE) {
        return HI_FAILURE;
    }

    comm_mutex_lock(&g_stAioInfo.stAiDev.mutex);

    if (g_stAioInfo.stAiDev.enState == STATE_IDLE) {
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return HI_FAILURE;
    }

    //reset resample rate if necessary
    s32Ret = vapi_aio_ai_set_resample(AiDevId, enOutSampleRate);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_ai_set_resample(%d, %d), failed with %#x!\n", AiDevId, enOutSampleRate, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return HI_FAILURE;
    }

    dst = g_stAioInfo.stAiDev.buffer;

    s32Ret = HI_MPI_AI_GetFrame(AiDevId, 0, &stFrm, &stAecFrm, 100);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AI_GetFrame(%d, %d), failed with %#x!\n", AiDevId, 0, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return HI_FAILURE;
    }
    src = (short *)stFrm.pVirAddr[0];
    srcSize = stFrm.u32Len;
    if (srcSize > MAX_BUFF_SIZE) {
        VAPILOG("read date size(%d) out of buffer max size(%d) ", srcSize, MAX_BUFF_SIZE);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return HI_FAILURE;
    }

    switch (enAenc) {
        case AENC_PCM_ULAW:
            pstFrame->len = vapi_aio_ulaw_encode(dst, src, srcSize);
            pstFrame->enAbit = ABIT_8_BIT;
            break;
        case AENC_PCM_ALAW:
            pstFrame->len = vapi_aio_alaw_encode(dst, src, srcSize);
            pstFrame->enAbit = ABIT_8_BIT;
            break;
        case AENC_PCM_RAW:
        default:
            pstFrame->len = srcSize;
            pstFrame->enAbit = ABIT_16_BIT;
            memcpy(dst, src, srcSize);
            break;
    }

    pstFrame->enArate   = enOutSampleRate;
    pstFrame->enAenc    = enAenc;
    pstFrame->pts       = stFrm.u64TimeStamp;
    pstFrame->date      = dst;

    /* finally you must release the stream */
    s32Ret = HI_MPI_AI_ReleaseFrame(AiDevId, 0, &stFrm, &stAecFrm);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("HI_MPI_AI_ReleaseFrame(%d, %d), failed with %#x!\n", AiDevId, 0, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);
        return HI_FAILURE;
    }

    comm_mutex_unlock(&g_stAioInfo.stAiDev.mutex);

    return HI_SUCCESS;
}

static HI_S32 vapi_aio_ao_get_multiple(AIO_FRAME_S *pstFrame)
{
    HI_S32 multiple = 0;

    if (pstFrame->enAenc == AENC_PCM_ULAW
        || pstFrame->enAenc == AENC_PCM_ALAW
        || pstFrame->enAenc == AENC_PCM_AAC) {
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

HI_S32 vapi_aio_ao_write_frame(AUDIO_DEV AoDevId, AIO_FRAME_S *pstFrame)
{
    HI_S32 s32Ret;
    HI_S32 i;
    HI_S32 multiple = 0;
    AUDIO_FRAME_S stData;
    short *dst;
    unsigned char *src = pstFrame->date;
    unsigned int srcSize = pstFrame->len;

    if (g_stAioInfo.stAoDev[AoDevId].enState == STATE_IDLE) {
        return HI_FAILURE;
    }

    comm_mutex_lock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    if (g_stAioInfo.stAoDev[AoDevId].enState == STATE_IDLE) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }

    multiple = vapi_aio_ao_get_multiple(pstFrame);

    if (srcSize * multiple > MAX_BUFF_SIZE) {
        VAPILOG("write date size(%d) out of buffer max size(%d) ", srcSize * multiple, MAX_BUFF_SIZE);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }
    //reset resample rate if necessary
    s32Ret = vapi_aio_ao_set_resample(AoDevId, pstFrame->enArate);
    if (HI_SUCCESS != s32Ret) {
        VAPILOG("vapi_aio_ao_set_resample(%d, %d), failed with %#x!\n", AoDevId, pstFrame->enArate, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }

    if (g_stAioInfo.stAoDev[AoDevId].buffer == HI_NULL) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }
    dst = (short *)g_stAioInfo.stAoDev[AoDevId].buffer;

    switch (pstFrame->enAenc) {
        case AENC_PCM_ULAW:
            stData.u32Len = vapi_aio_ulaw_decode(dst, src, srcSize);
            break;
        case AENC_PCM_ALAW:
            stData.u32Len = vapi_aio_alaw_decode(dst, src, srcSize);
            break;
        case AENC_PCM_AAC:
            stData.u32Len = vapi_aio_aac_decode(dst, src, srcSize, pstFrame->enArate);
            break;
        case AENC_PCM_G722:
            stData.u32Len = vapi_aio_g722_decode(dst, src, srcSize);
            break;
        case AENC_PCM_G726:
            stData.u32Len = vapi_aio_g726_decode(dst, src, srcSize);
            break;
        case AENC_PCM_RAW:
        default:
            stData.u32Len = srcSize;
            memcpy(dst, src, srcSize);
            break;
    }
    if (stData.u32Len < 0) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }

    if (pstFrame->enAenc == AENC_PCM_RAW && pstFrame->enAbit == ABIT_8_BIT) {
        for (i = 0; i < srcSize; i++) {
            dst[i] = (short)(src[i] - 0x80) << 8;
        }
        stData.u32Len = 2 * srcSize;
    }

    stData.enBitwidth = AUDIO_BIT_WIDTH_16;
    stData.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stData.pVirAddr[0] = dst;

    for (i = 0; i < g_stAioInfo.stAoDev[AoDevId].ChnCnt; i++) {
        s32Ret = HI_MPI_AO_SendFrame(AoDevId, i, &stData, 0);
        if (HI_SUCCESS != s32Ret) {
            if (HI_ERR_AO_BUF_FULL == s32Ret) {
                HI_MPI_AO_ClearChnBuf(AoDevId, i);
            } else {
                VAPILOG("HI_MPI_AO_SendFrame(%d, %d), failed with %#x!\n", AoDevId, i, s32Ret);
                comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
                return HI_FAILURE;
            }
        }
    }

    comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    return HI_SUCCESS;
}

static HI_VOID vapi_aio_set_talk_flag(void)
{
    FILE *pfd = NULL;
    HI_CHAR buff[64];

    pfd = popen("mschip_update", "r");
    if (pfd != NULL) {
        if (fgets(buff, sizeof(buff), pfd) != NULL) {
            if (strstr(buff, "MSN5008")) {
                g_stAioInfo.bTalk = HI_TRUE;
            } else {
                g_stAioInfo.bTalk = HI_FALSE;
            }
        }
        pclose(pfd);
    }
}

HI_BOOL vapi_aio_is_support_talk()
{
    return g_stAioInfo.bTalk;
}

HI_S32 vapi_aio_init()
{
    memset(&g_stAioInfo, 0, sizeof(AIO_INFO_S));

    vapi_aio_ai_start(0, AUDIO_SAMPLE_RATE_16000, 1, AUDIO_SAMPLE_RATE_16000);
    vapi_aio_ao_start(AO_SPK_MIC, AUDIO_SAMPLE_RATE_16000, 2, AUDIO_SAMPLE_RATE_16000);
    vapi_aio_ao_start(AO_MAIN_HDMI, AUDIO_SAMPLE_RATE_16000, 2, AUDIO_SAMPLE_RATE_32000);
//    vapi_aio_ao_start(AO_SUB_HDMI, AUDIO_SAMPLE_RATE_16000, 2, AUDIO_SAMPLE_RATE_32000);
    vapi_aio_aac_init();
    vapi_aio_set_talk_flag();


#if 0
    MPP_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId = HI_ID_AI;
    stSrcChn.s32ChnId = 0;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = 1;
    stDestChn.s32ChnId = 0;

    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
#endif
    return HI_SUCCESS;
}

HI_S32 vapi_aio_uninit()
{
    vapi_aio_aac_uninit();
    vapi_aio_ai_stop(0);
    vapi_aio_ao_stop(AO_SPK_MIC);
    vapi_aio_ao_stop(AO_MAIN_HDMI);
//    vapi_aio_ao_stop(AO_SUB_HDMI);

    return HI_SUCCESS;
}


