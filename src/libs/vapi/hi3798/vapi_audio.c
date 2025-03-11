/*
 * ***************************************************************
 * Filename:        vapi_audio.c
 * Created at:      2016.05.03
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

#include "gpio.h"
#include "vapi_comm.h"
#include "neaacdec.h"
#include "vapi.h"
#include "g722_codec.h"
#include "g726_codec.h"
#include "alsa/asoundlib.h"
#include "speex/speex_preprocess.h"

#define ACODEC_FILE     "/dev/acodec"
#define PCM_HW_NAME     "plughw:2,0"
#define MAX_BUFF_SIZE   10240

#define SPEEX

typedef struct snd_dev_s {
    char                *buff;
    snd_pcm_t           *handle;
    snd_pcm_hw_params_t *hw_parms;
    snd_pcm_uframes_t    frms;
} SND_DEV_S;

static SND_DEV_S g_stAiSnd, g_stAoSnd;

typedef struct speex_config {
    int dn_enable;
    int agc_enable;
    int level;
    int noise_suppress;
    SpeexPreprocessState *state;
} SPEEX_CFG_S;

static SPEEX_CFG_S g_stDnCfg;

typedef struct dev_s {
    STATE_E                 enState;
    HI_U32                  ChnCnt;
    HI_UNF_SAMPLE_RATE_E    enInSampleRate;
    HI_UNF_SAMPLE_RATE_E    enOutSampleRate;
    HI_BOOL                 bVqeEn;
    HI_BOOL                 bRspEn;
    HI_U8                  *buffer;
    pthread_mutex_t         mutex;
    HI_HANDLE               hTrack;
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
        VAPILOG("NeAACDecDecode:%p [%lu][%d] enRate with %d\n", g_stAioInfo.hDecoder, samplerate, channels, enRate);
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
    //´ÓË«ÉùµÀµÄÊý¾ÝÖÐÌáÈ¡µ¥Í¨µÀ
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
    if (g_stAioInfo.hDecoder) {
        NeAACDecClose(g_stAioInfo.hDecoder);
        g_stAioInfo.hDecoder = NULL;
    }
    return ;
}

static HI_S32 vapi_aio_uac_close(SND_DEV_S *snd_dev)
{
    if (NULL != snd_dev->hw_parms) {
        snd_pcm_hw_params_free(snd_dev->hw_parms);
        snd_dev->hw_parms = NULL;
    }

    if (NULL != snd_dev->handle) {
        snd_pcm_drain(snd_dev->handle);
        snd_pcm_close(snd_dev->handle);
        snd_dev->handle = NULL;
    }

    return HI_SUCCESS;
}

static HI_S32 vapi_aio_config_hw_params(SND_DEV_S *snd_dev, HI_UNF_SAMPLE_RATE_E samplerate, unsigned int chncnt)
{
    HI_S32 s32Ret, dir;
    unsigned int SmpRate;
    snd_pcm_access_t enAccess;
    snd_pcm_format_t enFormat;
    snd_pcm_t *handle = snd_dev->handle;
    snd_pcm_hw_params_t *hwParms = snd_dev->hw_parms;
    unsigned int buffer_time = 500000;  //500ms
    unsigned int period_time = buffer_time / 5;

    /* choose all parameters */
    s32Ret = snd_pcm_hw_params_any(handle, hwParms);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_hw_params_any failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    s32Ret = snd_pcm_hw_params_set_rate_resample(handle, hwParms, 1);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_hw_params_set_rate_resample failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    /* set the interleaved read/write format */
    enAccess = SND_PCM_ACCESS_RW_INTERLEAVED;
    s32Ret = snd_pcm_hw_params_set_access(handle, hwParms, enAccess);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_hw_params_set_access failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    /* set the sample format */
    enFormat = SND_PCM_FORMAT_S16_LE;
    s32Ret = snd_pcm_hw_params_set_format(handle, hwParms, enFormat);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_hw_params_set_format failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    /* set the count of channels */
    s32Ret = snd_pcm_hw_params_set_channels(handle, hwParms, chncnt);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_hw_params_set_channels failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    /* set the stream rate */
    SmpRate = samplerate;
    s32Ret = snd_pcm_hw_params_set_rate_near(handle, hwParms, &SmpRate, 0);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_hw_params_set_rate_near failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    /* set the buff size and period by time */
    s32Ret = snd_pcm_hw_params_set_buffer_time_near(handle, hwParms, &buffer_time, &dir);
    if (s32Ret < 0) {
        VAPILOG("Unable to set buffer time %u\n", buffer_time);
        return s32Ret;
    }
    s32Ret = snd_pcm_hw_params_set_period_time_near(handle, hwParms, &period_time, &dir);
    if (s32Ret < 0) {
        VAPILOG("Unable to set period time %u\n", period_time);
        return s32Ret;
    }

    s32Ret = snd_pcm_hw_params(handle, hwParms);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_hw_params failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = snd_pcm_hw_params_get_period_size(hwParms, &snd_dev->frms, &dir);
    if (s32Ret < 0) {
        printf("Unable to get period size for playback: %s\n", snd_strerror(s32Ret));
        return s32Ret;
    }

    return HI_SUCCESS;
}

#ifdef SPEEX    //now only denoisy
static HI_S32 vapi_aio_speex_echo_init(int rate, int FrameSize)
{
    VAPILOG("vapi aio_speex_echo_init rate:%d FrameSize:%d", rate, FrameSize);
    g_stDnCfg.state = speex_preprocess_state_init(FrameSize, rate);
    g_stDnCfg.dn_enable = 1;
    g_stDnCfg.agc_enable = 0;
    g_stDnCfg.level = 8000;
    g_stDnCfg.noise_suppress = -200;

    speex_preprocess_ctl(g_stDnCfg.state, SPEEX_PREPROCESS_SET_DENOISE, &g_stDnCfg.dn_enable);
    speex_preprocess_ctl(g_stDnCfg.state, SPEEX_PREPROCESS_SET_AGC, &g_stDnCfg.agc_enable);
    speex_preprocess_ctl(g_stDnCfg.state, SPEEX_PREPROCESS_SET_AGC_LEVEL, &g_stDnCfg.level);
    speex_preprocess_ctl(g_stDnCfg.state, SPEEX_PREPROCESS_SET_NOISE_SUPPRESS, &g_stDnCfg.noise_suppress);

    return HI_SUCCESS;
}

static HI_S32 vapi_aio_speex_echo_deinit(void)
{
    VAPILOG("vapi aio_speex_echo_deinit.");
    speex_preprocess_state_destroy(g_stDnCfg.state);
    return HI_SUCCESS;
}
static inline HI_S32  vapi_aio_speex_echo_reinit(HI_S32 AoDevId, HI_UNF_SAMPLE_RATE_E enInSampleRate, int FrameSize)
{
    unsigned int rate;
    snd_pcm_hw_params_get_rate(g_stAoSnd.hw_parms, &rate, 0);
    if (enInSampleRate == rate) {
        return HI_SUCCESS;
    } else {
        VAPILOG("vapi aio_speex_echo_reinit AoDevId:%d enInSampleRate:%d(%d) FrameSize:%d.", AoDevId, enInSampleRate, rate,
                FrameSize);
        vapi_aio_speex_echo_deinit();
        vapi_aio_speex_echo_init(enInSampleRate, FrameSize);
    }
    return HI_SUCCESS;
}
#endif

static HI_S32 vapi_aio_ai_start(HI_S32 AiDevId, HI_UNF_SAMPLE_RATE_E enInSampleRate,
                                HI_U32 u32ChnCnt, HI_UNF_SAMPLE_RATE_E enOutSamplerate)
{
    HI_S32 s32Ret;

    s32Ret = snd_pcm_open(&g_stAiSnd.handle, PCM_HW_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (s32Ret < 0) {
        g_stAiSnd.handle = NULL;
        VAPILOG("snd_pcm_open failed with %#x!\n", s32Ret);
        return HI_SUCCESS;
    }

    snd_pcm_hw_params_malloc(&g_stAiSnd.hw_parms);
    vapi_aio_config_hw_params(&g_stAiSnd, enInSampleRate, u32ChnCnt);
    g_stAiSnd.buff = (char *)malloc(g_stAiSnd.frms * 2);

    g_stAioInfo.stAiDev.enInSampleRate = enInSampleRate;
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

static HI_S32 vapi_aio_ai_stop(HI_S32 AiDevId)
{
    if (g_stAiSnd.handle != NULL) {
        free(g_stAiSnd.buff);
        g_stAiSnd.buff = NULL;
        vapi_aio_uac_close(&g_stAiSnd);
        comm_mutex_uninit(&g_stAioInfo.stAiDev.mutex);
        g_stAioInfo.stAiDev.enState = STATE_IDLE;
    }
    return HI_SUCCESS;
}

static void vapi_aio_hardware_enable(HI_BOOL bEnable)
{
    if (bEnable == HI_TRUE) {
        gpio_open();
        gpio_write(GIO_MUTE_CTL_EN, bEnable);
    } else {
        gpio_write(GIO_MUTE_CTL_EN, bEnable);
        gpio_close();
    }
}

static void vapi_aio_ao_volume_set_init(int mute)
{
    int vol, ret = -1;
    int alsa_mix_index = 0;
    long volMin, volMax, Val;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t *elem;
    snd_mixer_t *handle = NULL;

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, alsa_mix_index);
    snd_mixer_selem_id_set_name(sid, "Digital");

    ret = snd_mixer_open(&handle, 0);
    if (ret < 0) {
        printf("snd mixer open err!\n");
    }
    ret = snd_mixer_attach(handle, "hw:2");
    if (ret < 0) {
        printf("snd_mixer_attach err! return %d\n", ret);
    }
    ret = snd_mixer_selem_register(handle, NULL, NULL);
    if (ret < 0) {
        printf("snd_mixer_selem_register err!\n");
    }
    ret = snd_mixer_load(handle);
    if (ret < 0) {
        printf("snd_mixer_load err!\n");
    }

    elem = snd_mixer_first_elem(handle);
    if (elem) {
        printf("elem name : %s\n", snd_mixer_selem_get_name(elem));      //PCM
        //elem = snd_mixer_elem_next(elem);
    } else {
        printf("snd_mixer_first_elem err \n");
    }

    snd_mixer_selem_get_capture_volume_range(elem, &volMin, &volMax);
    printf("volume range: %ld -- %ld\n", volMin, volMax);

    snd_mixer_handle_events(handle);

    vol = mute ? 0 : 1000;
    snd_mixer_selem_set_playback_volume(elem, SND_MIXER_SCHN_MONO, vol);
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &Val);
    printf("cur volume is %ld\n", Val);

    snd_mixer_close(handle);
}

HI_S32 vapi_aio_ao_set_mute(HI_S32 mute)
{
    if (g_stAoSnd.handle == NULL) {
        return 0;
    }

    vapi_aio_ao_volume_set_init(mute);
    return 0;
}

static HI_S32 vapi_aio_xrun_recovery(snd_pcm_t *handle, HI_S32 err)
{
    if (err == -EPIPE) {
        err = snd_pcm_prepare(handle);
        if (err < 0) {
            printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
    } else if (err == -ESTRPIPE) {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN) {
            sleep(1);   /* wait until the suspend flag is released */
        }
        if (err < 0) {
            err = snd_pcm_prepare(handle);
            if (err < 0) {
                printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
            }
        }
        return 0;
    }

    return err;
}

static HI_S32 vapi_aio_config_sw_params(SND_DEV_S *dev)
{
    HI_S32 s32Ret = HI_SUCCESS;
    snd_pcm_sw_params_t *swparams;

    snd_pcm_sw_params_alloca(&swparams);

    s32Ret = snd_pcm_sw_params_current(dev->handle, swparams);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_sw_params_current failed return %d", s32Ret);
        return s32Ret;
    }
    s32Ret = snd_pcm_sw_params_set_start_threshold(dev->handle, swparams, 2 * dev->frms);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_sw_params_set_start_threshold failed return %d", s32Ret);
        return s32Ret;
    }
    s32Ret = snd_pcm_sw_params_set_silence_threshold(dev->handle, swparams, 2 * dev->frms);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_sw_params_set_silence_threshold failed return %d", s32Ret);
        return s32Ret;
    }
    s32Ret = snd_pcm_sw_params(dev->handle, swparams);
    if (s32Ret < 0) {
        VAPILOG("snd_pcm_sw_params failed return %d", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static HI_S32 vapi_aio_ao_start(HI_S32 AoDevId, HI_UNF_SAMPLE_RATE_E enInSampleRate,
                                HI_U32 u32ChnCnt, HI_UNF_SAMPLE_RATE_E enOutSampleRate)
{
    HI_S32 s32Ret;
    HI_UNF_SND_ATTR_S stAttr;
    HI_UNF_AUDIOTRACK_ATTR_S  stTrackAttr;
    HI_UNF_SND_OUTPUTPORT_E enOutPort;
    HI_UNF_SND_E enSND;
    snd_pcm_uframes_t frms;

    //frms = (enOutSampleRate/1000) * 30;

    g_stAioInfo.stAoDev[AoDevId].enState = STATE_IDLE;
    if (AoDevId == AO_SPK_MIC) {
        s32Ret = snd_pcm_open(&g_stAoSnd.handle, PCM_HW_NAME, SND_PCM_STREAM_PLAYBACK, 0);
        if (s32Ret < 0) {
            g_stAoSnd.handle = NULL;
            VAPILOG("snd_pcm_open failed with %#x!\n", s32Ret);
            //return s32Ret;
        }

        frms = g_stAoSnd.handle == NULL ? 480 : 240;
#ifdef SPEEX
        vapi_aio_speex_echo_init(enOutSampleRate, frms);    //denoisy
#endif
        if (g_stAoSnd.handle != NULL) {
            snd_pcm_hw_params_malloc(&g_stAoSnd.hw_parms);
            vapi_aio_config_hw_params(&g_stAoSnd, enOutSampleRate, 1);
            vapi_aio_config_sw_params(&g_stAoSnd);
            vapi_aio_ao_volume_set_init(0);
            goto VAPI_AO_INIT;
        }
    }

    if (AoDevId == AO_SPK_MIC) {
        enOutPort = HI_UNF_SND_OUTPUTPORT_DAC0;
        enSND = HI_UNF_SND_0;
    } else if (AoDevId == AO_MAIN_HDMI) {
        enOutPort = HI_UNF_SND_OUTPUTPORT_HDMI0;
        enSND = HI_UNF_SND_1;
    } else {
        VAPILOG("unknown ao device [%d] \n", AoDevId);
        return HI_FAILURE;
    }

    s32Ret = HI_UNF_SND_GetDefaultOpenAttr(enSND, &stAttr);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND[%d]_GetDefaultOpenAttr failed.with %#x\n", enSND, s32Ret);
        return s32Ret;
    }

    stAttr.u32MasterOutputBufSize = 8000 * 30; /* in order to increase the reaction of stop/start, the buf cannot too big*/
    stAttr.u32PortNum = 1;
    stAttr.stOutport[0].enOutPort = enOutPort;
    stAttr.enSampleRate = enOutSampleRate;

    s32Ret = HI_UNF_SND_Open(enSND, &stAttr);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND[%d]_Open failed.with %#x\n", enSND, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_UNF_SND_GetDefaultTrackAttr(HI_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND[%d]_GetDefaultTrackAttr failed.with %#x\n", enSND, s32Ret);
        return s32Ret;
    }

    s32Ret = HI_UNF_SND_CreateTrack(enSND, &stTrackAttr, &g_stAioInfo.stAoDev[AoDevId].hTrack);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND[%d]_CreateTrack failed.with %#x\n", enSND, s32Ret);
        return s32Ret;
    }

//    s32Ret = HI_UNF_SND_SetTrackMode(enSND, enOutPort, HI_UNF_TRACK_MODE_STEREO);
//    if (s32Ret != HI_SUCCESS)
//    {
//        VAPILOG("HI_UNF_SND_SetTrackMode[%d] failed.with %#x\n", enSND, s32Ret);
//        return s32Ret;
//    }
//
//    s32Ret = HI_UNF_SND_SetTrackChannelMode(g_stAioInfo.stAoDev[AoDevId].hTrack, HI_UNF_TRACK_MODE_ONLY_LEFT);
//    if (s32Ret != HI_SUCCESS)
//    {
//        VAPILOG("HI_UNF_SND_SetTrackChannelMode[%d] failed.with %#x\n", enSND, s32Ret);
//        return s32Ret;
//    }

VAPI_AO_INIT:
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

static HI_S32 vapi_aio_ao_stop(HI_S32 AoDevId)
{
    HI_S32 s32Ret;
    HI_UNF_SND_E enSND;

    if (AoDevId == AO_SPK_MIC) {
#ifdef SPEEX
        vapi_aio_speex_echo_deinit();
#endif
        if (g_stAoSnd.handle != NULL) {
            vapi_aio_uac_close(&g_stAoSnd);
        }
    }

    if (AoDevId == AO_SPK_MIC) {
        enSND = HI_UNF_SND_0;
    } else if (AoDevId == AO_MAIN_HDMI) {
        enSND = HI_UNF_SND_1;
    } else {
        VAPILOG("unknown ao device [%d] \n", AoDevId);
        return HI_FAILURE;
    }

    comm_mutex_lock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    s32Ret = HI_UNF_SND_DestroyTrack(g_stAioInfo.stAoDev[AoDevId].hTrack);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND[%d]_DestroyTrack failed.with %#x\n", enSND, s32Ret);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return s32Ret;
    }

    s32Ret = HI_UNF_SND_Close(enSND);
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND[%d]_Close failed.with %#x\n", enSND, s32Ret);
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

HI_S32 vapi_aio_ai_set_volume(HI_S32 AiDevId, HI_U8 value)
{
    return HI_SUCCESS;
}

HI_S32 vapi_aio_ao_set_volume(HI_S32 AoDevId, HI_U8 value)
{
    return HI_SUCCESS;
}

static inline HI_S32  vapi_aio_ai_set_resample(HI_S32 AiDevId, HI_UNF_SAMPLE_RATE_E enOutSampleRate)
{
    unsigned int rate;

    snd_pcm_hw_params_get_rate(g_stAiSnd.hw_parms, &rate, 0);
    if (enOutSampleRate == rate) {
        return HI_SUCCESS;
    } else {
        VAPILOG("vapi aio ai set resample AiDevId:%d enOutSampleRate:%d", AiDevId, enOutSampleRate);
        vapi_aio_ai_stop(0);
        vapi_aio_ai_start(0, enOutSampleRate, 1, enOutSampleRate);
        snd_pcm_hw_params_get_rate(g_stAiSnd.hw_parms, &rate, 0);
    }
    return HI_SUCCESS;
}

static inline HI_S32  vapi_aio_ao_set_resample(HI_S32 AoDevId, HI_UNF_SAMPLE_RATE_E enInSampleRate, int FrameSize)
{
    unsigned int rate, s32Ret;

    snd_pcm_hw_params_get_rate(g_stAoSnd.hw_parms, &rate, 0);
    if (enInSampleRate == rate) {
        return HI_SUCCESS;
    } else {
        VAPILOG("vapi aio_ao_set_resample AoDevId:%d enInSampleRate:%d(%d) FrameSize:%d.", AoDevId, enInSampleRate, rate,
                FrameSize);
        vapi_aio_uac_close(&g_stAoSnd);
        s32Ret = snd_pcm_open(&g_stAoSnd.handle, PCM_HW_NAME, SND_PCM_STREAM_PLAYBACK, 0);
        if (s32Ret < 0) {
            VAPILOG("snd_pcm_open failed with %#x!\n", s32Ret);
            return s32Ret;
        }
        snd_pcm_hw_params_malloc(&g_stAoSnd.hw_parms);
        s32Ret = vapi_aio_config_hw_params(&g_stAoSnd, enInSampleRate, 1);
        if (s32Ret != HI_SUCCESS) {
            VAPILOG("vapi_aio_config_hw_params failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        s32Ret = vapi_aio_config_sw_params(&g_stAoSnd);
        if (s32Ret != HI_SUCCESS) {
            VAPILOG("vapi_aio_config_sw_params failed with %#x!\n", s32Ret);
            return s32Ret;
        }

#if 0
#ifdef SPEEX
        vapi_aio_speex_echo_deinit();
        vapi_aio_speex_echo_init(enInSampleRate, FrameSize);
#endif
#endif
    }

    return HI_SUCCESS;
}

HI_S32 vapi_aio_ai_read_frame(HI_S32 AiDevId, HI_UNF_SAMPLE_RATE_E enOutSampleRate, AENC_E enAenc,
                              AIO_FRAME_S *pstFrame)
{
    unsigned int s32Ret, srcSize;
    unsigned char *dst;
    short *src;


    if (g_stAiSnd.handle == NULL) {
        return HI_FAILURE;
    }

    vapi_aio_ai_set_resample(0, enOutSampleRate);
    do {
        snd_pcm_prepare(g_stAiSnd.handle);
        s32Ret = snd_pcm_readi(g_stAiSnd.handle, g_stAiSnd.buff, g_stAiSnd.frms);
    } while (s32Ret == -EPIPE);

    src = (short *)g_stAiSnd.buff;
    dst = g_stAioInfo.stAiDev.buffer;
    srcSize = g_stAiSnd.frms * 2;
    if (srcSize > MAX_BUFF_SIZE) {
        VAPILOG("read date size(%d) out of buffer max size(%d) ", srcSize, MAX_BUFF_SIZE);
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
//    pstFrame->pts       = stFrm.u64TimeStamp;
    pstFrame->date      = dst;

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

HI_S32 vapi_aio_ao_write_frame(HI_S32 AoDevId, AIO_FRAME_S *pstFrame)
{
    HI_S32 s32Ret;
    HI_S32 i = 10;
    HI_UNF_AO_FRAMEINFO_S stAOFrame;
    short *dst;
    unsigned int srcSize = pstFrame->len;
    unsigned char *src = pstFrame->date;
    HI_S32 multiple = 0;

    stAOFrame.s32BitPerSample = 16;
    stAOFrame.u32Channels   = 1;
    stAOFrame.bInterleaved  = HI_TRUE;
    stAOFrame.u32SampleRate = (HI_U32)(pstFrame->enArate);
//    stAOFrame.u32PcmSamplesPerFrame = srcSize;//stAOFrame.u32SampleRate/100*4 ;
    stAOFrame.u32PtsMs = 0xffffffff;//(HI_U32)pstFrame->pts;
    stAOFrame.ps32BitsBuffer = HI_NULL;
    stAOFrame.u32BitsBytesPerFrame = 0;
    stAOFrame.u32FrameIndex = 0;

    comm_mutex_lock(&g_stAioInfo.stAoDev[AoDevId].mutex);
    if (g_stAioInfo.stAoDev[AoDevId].enState == STATE_IDLE) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }

    if (g_stAioInfo.stAoDev[AoDevId].buffer == HI_NULL) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }

    multiple = vapi_aio_ao_get_multiple(pstFrame);

    if (srcSize * multiple > MAX_BUFF_SIZE) {
        VAPILOG("write date size(%d) out of buffer max size(%d) ", srcSize, MAX_BUFF_SIZE);
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }

    dst = (short *)g_stAioInfo.stAoDev[AoDevId].buffer;
    memset(dst, 0, MAX_BUFF_SIZE);
    switch (pstFrame->enAenc) {
        case AENC_PCM_ULAW:
            srcSize = vapi_aio_ulaw_decode(dst, src, srcSize);
            break;
        case AENC_PCM_ALAW:
            srcSize = vapi_aio_alaw_decode(dst, src, srcSize);
            break;
        case AENC_PCM_AAC:
            srcSize = vapi_aio_aac_decode(dst, src, srcSize, pstFrame->enArate);
            break;
        case AENC_PCM_G722:
            srcSize = vapi_aio_g722_decode(dst, src, srcSize);
            break;
        case AENC_PCM_G726:
            srcSize = vapi_aio_g726_decode(dst, src, srcSize);
            break;
        case AENC_PCM_RAW:
        default:
            memcpy(dst, src, srcSize);
            break;
    }

    if (srcSize < 0) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
        return HI_FAILURE;
    }

    if (pstFrame->enAenc == AENC_PCM_RAW && pstFrame->enAbit == ABIT_8_BIT) {
        for (i = 0; i < srcSize; i++) {
            dst[i] = (short)(src[i] - 0x80) << 8;
        }
        srcSize *= 2;
    }

    stAOFrame.ps32PcmBuffer = (HI_S32 *)(dst);
    stAOFrame.u32PcmSamplesPerFrame = srcSize / 2; //stAOFrame.u32SampleRate/100*4 ;

    if (AO_SPK_MIC == AoDevId
        && g_stAoSnd.handle != NULL) {
        vapi_aio_ao_set_resample(0, stAOFrame.u32SampleRate, srcSize);
    }
#ifdef SPEEX
    if (g_stAoSnd.handle != NULL && pstFrame->enAenc != AENC_PCM_AAC) {
        vapi_aio_speex_echo_reinit(0, stAOFrame.u32SampleRate, srcSize);
        if (srcSize < 5120) {
            speex_preprocess_run(g_stDnCfg.state, (spx_int16_t *)(dst));
        }
    }
#endif

    if (AO_SPK_MIC == AoDevId) {
        if (g_stAoSnd.handle != NULL) {
            do {
                s32Ret = snd_pcm_writei(g_stAoSnd.handle, dst, stAOFrame.u32PcmSamplesPerFrame);
                if (s32Ret == -EAGAIN) {
                    snd_pcm_wait(g_stAoSnd.handle, 100);
                    continue;
                }
                if (s32Ret < 0 && vapi_aio_xrun_recovery(g_stAoSnd.handle, s32Ret) < 0) {
                    VAPILOG("write err: [%s] \n", snd_strerror(s32Ret));
                    break;
                }
            } while (s32Ret < 0);
            comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);
            return s32Ret;
        }
    }

    i = 10;
    while (i) {
        s32Ret = HI_UNF_SND_SendTrackData(g_stAioInfo.stAoDev[AoDevId].hTrack, &stAOFrame);
        if (HI_SUCCESS != s32Ret) {
            if (s32Ret == HI_ERR_AO_OUT_BUF_FULL) {
                usleep(20 * 1000);
                i--;
            } else {
                VAPILOG("HI_UNF_SND_SendTrackData(%d), failed with %#x!\n", AoDevId, s32Ret);
                break;
            }
        } else {
            break;
        }
    }

    comm_mutex_unlock(&g_stAioInfo.stAoDev[AoDevId].mutex);

    return s32Ret;
}

static HI_VOID vapi_aio_set_talk_flag(void)
{
    FILE *pfd;
    char buff[256];

    pfd = popen("aplay -l", "r");
    if (pfd != NULL) {
        if (fgets(buff, sizeof(buff), pfd) != NULL) {
//          printf("buff is %s\n", buff);
            if (strstr(buff, "List of PLAYBACK Hardware Devices")) {
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
    HI_S32 s32Ret;

    s32Ret = HI_UNF_SND_Init();
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND_Init failed.with %#x\n", s32Ret);
        return s32Ret;
    }

    memset(&g_stAioInfo, 0, sizeof(AIO_INFO_S));

    vapi_aio_ai_start(0, HI_UNF_SAMPLE_RATE_8K, 1, HI_UNF_SAMPLE_RATE_8K);
    vapi_aio_ao_start(AO_SPK_MIC, HI_UNF_SAMPLE_RATE_16K, 2, HI_UNF_SAMPLE_RATE_16K);
    vapi_aio_ao_start(AO_MAIN_HDMI, HI_UNF_SAMPLE_RATE_16K, 2, HI_UNF_SAMPLE_RATE_48K);
    vapi_aio_hardware_enable(HI_TRUE);
    vapi_aio_aac_init();
    vapi_aio_set_talk_flag();

    return HI_SUCCESS;
}

HI_S32 vapi_aio_uninit()
{
    HI_S32 s32Ret;

    vapi_aio_aac_uninit();
    vapi_aio_hardware_enable(HI_FALSE);
    vapi_aio_ai_stop(0);
    vapi_aio_ao_stop(AO_SPK_MIC);
    vapi_aio_ao_stop(AO_MAIN_HDMI);

    s32Ret = HI_UNF_SND_DeInit();
    if (s32Ret != HI_SUCCESS) {
        VAPILOG("HI_UNF_SND_DeInit failed.with %#x\n", s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}




