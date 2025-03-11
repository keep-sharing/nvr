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

#include "neaacdec.h"
#include "vapi.h"
#include "g722_codec.h"
#include "g726_codec.h"
#include "samplerate.h"

#include "vapi_comm.h"
#include "vapi.h"
#include "speex/speex_preprocess.h"


#define ACODEC_FILE     "/dev/nau88c10"
#define MAX_BUFF_SIZE   12800
#define DEFAULT_SAMPLE_RATE  ARATE_8000
//#define SPEEX
//#define ADUIO_DENOISE

typedef struct snd_dev_s {
    char                *buff;
} SND_DEV_S;

typedef struct speex_config {
    int dn_enable;
    int agc_enable;
    int level;
    int noise_suppress;
    SpeexPreprocessState *state;
} SPEEX_CFG_S;

static SPEEX_CFG_S g_stDnCfg;

typedef struct dev_s {
    HD_PATH_ID      aiPathId;
    HD_PATH_ID      aePathId;
    HD_PATH_ID      adPathId;
    HD_PATH_ID      aoPathId;
    STATE_E         enState;
    ARATE_E         sampleRate;
    HI_U8           *buffer[2];
    pthread_mutex_t  mutex;
    char pcm_buff[2048];
    int pcm_buff_index;
} DEV_S;

typedef struct aio_info_s {
    DEV_S    stAiDev[AO_TYPE_NUM];
    DEV_S    stAoDev[AO_TYPE_NUM];
    HD_PATH_ID aiCtrlPath;
    HD_PATH_ID aoCtrlPath;
    NeAACDecHandle hDecoder;
    HI_BOOL bHasCodec;
    HI_BOOL  bTalk;
} AIO_INFO_S;

static AIO_INFO_S g_stAioInfo;
#ifdef ADUIO_DENOISE
static FILE *pfd_src = NULL;
static FILE *pfd_dst = NULL;
static FILE *pfd_dst_re = NULL;
#endif
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

static unsigned int vapi_aio_alaw_encode(unsigned char *dst, short *src, unsigned int srcSize)
{
    unsigned char *end;

    srcSize >>= 1;
    end = dst + srcSize;

    while (dst < end) {
        *dst++ = ALawEncode(*src++);
    }

    return srcSize;
}

static unsigned int vapi_aio_alaw_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    short *end = dst + srcSize;

    while (dst < end) {
        *dst++ = ALawDecode(*src++);
    }

    return srcSize << 1;
}

static unsigned int vapi_aio_ulaw_encode(unsigned char *dst, short *src, unsigned int srcSize)
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
}

unsigned int vapi_aio_g726_decode(short *dst, const unsigned char *src, unsigned int srcSize)
{
    return g726_decode(dst, src, srcSize);
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

static unsigned int vapi_aio_aac_decode(short *dst, const unsigned char *src, unsigned int srcSize, ARATE_E enRate)
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
    //´ÓË«ÉùµÀµÄÊý¾ÝÖÐÌáÈ¡µ¥Í¨µÀ
    for (i = 0; i < frameInfo.samples; i++) {
        dst[i] = pBuffer[i * frameInfo.channels];
    }

    return frameInfo.samples;
}

#ifdef SPEEX    //now only denoisy
static HI_S32 vapi_aio_speex_echo_init(int rate, int FrameSize)
{
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
    speex_preprocess_state_destroy(g_stDnCfg.state);
    return HI_SUCCESS;
}

#endif

static HI_S32 aio_resample(float *src, int srcSize, ARATE_E srcRate,
                           float *dst, unsigned int *dstSize, ARATE_E dstRate)
{
    SRC_DATA stData;
    int err;

    memset(&stData, 0, sizeof(stData));

    stData.data_in = src;
    stData.input_frames = srcSize;
    stData.src_ratio = 1.0 * dstRate / srcRate;
    stData.data_out = dst;
    *dstSize = (unsigned int)(srcSize * stData.src_ratio);
    stData.output_frames = *dstSize;
    err = src_simple(&stData, SRC_LINEAR, 1);
    if (err) {
        VAPILOG("resample failed:[%s]", src_strerror(err));
        return HI_FAILURE;
    }
    //VAPILOG("======srcRate[%d] srcSize[%d]===dstRate[%d] dstSize[%d]====\n", srcRate, srcSize, dstRate, *dstSize);
    return HI_SUCCESS;
}

SRC_STATE  *src_fullstate;

static HI_S32 aio_resample2(float *src, int srcSize, ARATE_E srcRate,
                            float *dst, unsigned int *dstSize, ARATE_E dstRate)
{
    int error;
    SRC_DATA    stData ;
    /* Perform sample rate conversion. */
    stData.end_of_input = 1 ; /* Set this later. */

    stData.data_in = src ;
    stData.input_frames = srcSize ;

    stData.src_ratio = 1.0 * dstRate / srcRate ;

    stData.data_out = dst ;
    *dstSize = (unsigned int)(srcSize * stData.src_ratio);
    stData.output_frames = *dstSize ;

    src_reset(src_fullstate);
    error = src_process(src_fullstate, &stData);
    if (error) {
        VAPILOG("\n\nLine %d : %s\n\n", __LINE__, src_strerror(error)) ;
        VAPILOG("stData.input_frames  : %ld\n", stData.input_frames) ;
        VAPILOG("stData.output_frames : %ld\n", stData.output_frames) ;
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

static HI_S32 aio_set_codec(ARATE_E enSample)
{
    HI_S32 fdAcodec = -1;
    HI_S32 s32Samplerate;
    Nau88c10_Ctrl ctrl;

    if (ARATE_8000 == enSample) {
        s32Samplerate = 0x5;
    } else if (ARATE_16000 == enSample) {
        s32Samplerate = 0x3;
    } else if (ARATE_24000 == enSample) {
        s32Samplerate = 0x2;
    } else if (ARATE_32000 == enSample) {
        s32Samplerate = 0x1;
    } else if (ARATE_48000 == enSample) {
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
    g_stAioInfo.bTalk = HI_TRUE;
    
    return 0;
}

HI_S32 vapi_aio_ao_set_mute(HI_S32 aoDevId, HI_S32 mute)
{
    HI_S32 fd = -1;
    HI_S32 ret = HI_TRUE;
    Nau88c10_Ctrl ctrl;

    if (aoDevId == AO_SPK_MIC) {
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

HI_S32 vapi_aio_ai_set_volume(HI_S32 aiDevId, HI_U8 value)
{
    return HI_SUCCESS;
}

HI_S32 vapi_aio_ao_set_volume(HI_S32 aoDevId, HI_U8 value)
{
    return HI_SUCCESS;
}


HI_BOOL vapi_aio_is_support_talk()
{
    return g_stAioInfo.bTalk;
}

static HD_RESULT aio_ai_read(HI_S32 aiDevId, void *data, HI_U32 *size)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOENC_POLL_LIST pollList;
    HD_AUDIOENC_RECV_LIST recvList;
    HD_PATH_ID pathId = g_stAioInfo.stAiDev[aiDevId].aePathId;

    pollList.path_id = pathId;
    /* call poll to check the stream availability */
    ret = hd_audioenc_poll_list(&pollList, 1, 1000);
    if (ret == HD_ERR_TIMEDOUT || pollList.revent.event != HI_TRUE) {
        VAPILOG("Poll timeout!!\n");
        return ret;
    }


    if (pollList.revent.bs_size > MAX_BUFF_SIZE) {
        VAPILOG("buffer size is not enough! %lu, %d\n",
                pollList.revent.bs_size, MAX_BUFF_SIZE);
        return HD_ERR_OVERRUN;
    }

    recvList.path_id = pathId;
    recvList.user_bs.p_user_buf = data;
    recvList.user_bs.user_buf_size = MAX_BUFF_SIZE;
    if ((ret = hd_audioenc_recv_list(&recvList, 1)) < 0) {
        VAPILOG("Error return value %d\n", ret);
    } else if (recvList.retval < 0) {
        VAPILOG("get bitstreame error! ret = %d\n", ret);

    } else {
        if (recvList.user_bs.size > 0) {
            memcpy(data, recvList.user_bs.p_user_buf, recvList.user_bs.size);
            *size = recvList.user_bs.size;
        }
    }

    return HD_OK;
}

static HD_RESULT aio_ao_write(AO_E aoDevId, HD_AUDIO_SR samplerate, void *data, HI_U32 size)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIODEC_SEND_LIST sendList[1];

//    msprintf_ms("====[%d]=========000000000000==============", aoDevId);
    memset(sendList, 0, sizeof(sendList));  /* clear all mutli bs */
    sendList[0].path_id = g_stAioInfo.stAoDev[aoDevId].adPathId;
    sendList[0].user_bs.p_user_buf = data;
    sendList[0].user_bs.user_buf_size = size;
    if ((ret = hd_audiodec_send_list(sendList, 1, 500)) < 0) {
        VAPILOG("<dev[%d] send bitstream fail(%d)!>\n", aoDevId, ret);
    }
//    msprintf_ms("====[%d]=========111111111111==============", aoDevId);

    return ret;
}

typedef struct buffer_s {
    //for real use
    HD_AUDIO_FRAME frame;

    //memory info
    UINT64 pool;
    INT size;
    HD_COMMON_MEM_DDR_ID ddr_id;

    //address
    HD_COMMON_MEM_VB_BLK blk;
    VOID *va;

} BUFFER_T;

/**
* @used to allocate memory for saving the audio frame.
* @pool: specify the pool type of allocated memory.
* @size: specify the size of allocated memory.
* @ddr_id: specify the DDR id where will be used to allocate buffer.
* @pBuffer: the information of allocated memory.
*/
static INT allocate_buffer(UINT64 pool, int size, HD_COMMON_MEM_DDR_ID ddr_id, BUFFER_T *pBuffer)
{
    INT ret = 0;
    HD_COMMON_MEM_VB_BLK blk;
    UINTPTR pa;
    VOID *va;

    pBuffer->pool = pool;
    pBuffer->size = size;
    pBuffer->ddr_id = ddr_id;

    blk = hd_common_mem_get_block(pool, size, ddr_id);
    if (HD_COMMON_MEM_VB_INVALID_BLK == blk) {
        VAPILOG("hd_common_mem_get_block fail\r\n");
        ret =  -1;
        goto exit;
    }
    pa = hd_common_mem_blk2pa(blk);
    if (pa == 0) {
        VAPILOG("hd_common_mem_blk2pa fail, blk = %#lx\r\n", blk);
        hd_common_mem_release_block(blk);
        ret =  -1;
        goto exit;
    }
    va = hd_common_mem_mmap(HD_COMMON_MEM_MEM_TYPE_NONCACHE, pa, size);
    pBuffer->blk = blk;
    pBuffer->va = va;

    pBuffer->frame.phy_addr[0] = pa;
exit:
    return ret;
}

/**
 * used to release allocate memory.
 * @pBuffer: the information of allocated memory.
 */
static INT free_buffer(BUFFER_T *pBuffer)
{
    INT ret = 0;
    HD_RESULT hd_ret;

    hd_ret = hd_common_mem_munmap(pBuffer->va, pBuffer->size);
    if (hd_ret != HD_OK) {
        VAPILOG("hd_common_mem_munmap fail\r\n");
        ret =  -1;
        goto exit;
    }
    hd_ret = hd_common_mem_release_block((HD_COMMON_MEM_VB_BLK)pBuffer->blk);
    if (hd_ret != HD_OK) {
        VAPILOG("hd_common_mem_munmap fail\r\n");
        ret =  -1;
        goto exit;
    }
exit:
    return ret;
}

static HD_RESULT aio_ao_write2(AO_E aoDevId, HD_AUDIO_SR samplerate, void *data, HI_U32 size)
{
    HD_RESULT ret = HD_OK;
    HD_PATH_ID adPathId = g_stAioInfo.stAoDev[aoDevId].adPathId;
    BUFFER_T stBuffer;
    HD_AUDIO_FRAME *pFrame = &stBuffer.frame;
    HD_AUDIO_BS bs = {0};

    pFrame->ddr_id = 0;
    pFrame->size = size;

    ret = allocate_buffer(HD_COMMON_MEM_USER_BLK, pFrame->size,
                          pFrame->ddr_id, &stBuffer);
    if (ret != HD_OK) {
        VAPILOG("allocate_buffer fail\n");
        return ret;
    }

    memcpy(stBuffer.va, (char *)data, size);

    memset(&bs, 0, sizeof(bs));
    bs.phy_addr = pFrame->phy_addr[0];
    bs.size = pFrame->size;
    ret = hd_audiodec_push_in_buf(adPathId, &bs, pFrame, 500);
    if (ret != HD_OK) {
        VAPILOG("hd_audioout_push_in_buf fail[%d]\n", ret);
        free_buffer(&stBuffer);
        return ret;
    }

    free_buffer(&stBuffer);

    return ret;
}

static HD_RESULT aio_ai_start(HI_S32 aiDevId, ARATE_E enInSampleRate)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIOCAP_IN aiParm;
    HD_AUDIOCAP_DEV_CONFIG aiCfg;
    HD_AUDIOENC_OUT aeParam;
    HD_PATH_ID aiPathId = 0;
    HD_PATH_ID aePathId = 0;

#if 0
    HD_AUDIOCAP_DRV_CONFIG aiDrvCfg;
    ret = hd_audiocap_get(g_stAioInfo.aiCtrlPath, HD_AUDIOCAP_PARAM_DRV_CONFIG, &aiDrvCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_get(HD_AUDIOCAP_PARAM_DRV_CONFIG) fail\n");
        return ret;
    }
    aiDrvCfg.ssp_config.ssp_num[0]      = 0;//speaker ssp0
    aiDrvCfg.ssp_config.enable[0]       = 1;
    aiDrvCfg.ssp_config.ssp_chan[0]     = 2;
    aiDrvCfg.ssp_config.sample_size[0]  = 16;
    aiDrvCfg.ssp_config.sample_rate[0]  = enInSampleRate;
    aiDrvCfg.ssp_config.ssp_clock[0]    = 12288000;
    aiDrvCfg.ssp_config.bit_clock[0]    = aiDrvCfg.ssp_config.ssp_chan[0] *
                                          aiDrvCfg.ssp_config.sample_size[0] *
                                          aiDrvCfg.ssp_config.sample_rate[0];
    aiDrvCfg.ssp_config.ssp_master[0]   = 0;
    aiDrvCfg.ssp_config.live_sound_ch[0] = -1;

    aiDrvCfg.ssp_config.ssp_num[1]      = 6; //hdmi ssp6
    aiDrvCfg.ssp_config.enable[1]       = 0;
    aiDrvCfg.ssp_config.ssp_chan[1]     = 1;
    aiDrvCfg.ssp_config.sample_size[1]  = 16;
    aiDrvCfg.ssp_config.sample_rate[1]  = DEFAULT_SAMPLE_RATE;
    aiDrvCfg.ssp_config.ssp_clock[1]    = 12288000;
    aiDrvCfg.ssp_config.bit_clock[1]    = 3072000;
    aiDrvCfg.ssp_config.ssp_master[1]   = 1;
    aiDrvCfg.ssp_config.live_sound_ch[1] = -1;

    ret = hd_audiocap_set(g_stAioInfo.aiCtrlPath, HD_AUDIOCAP_PARAM_DRV_CONFIG, &aiDrvCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_set(HD_AUDIOCAP_PARAM_DRV_CONFIG) fail\n");
        return ret;
    }
#endif
    ret = hd_audiocap_open(HD_AUDIOCAP_IN(aiDevId, 0), HD_AUDIOCAP_OUT(aiDevId, 0), &aiPathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_open[%d] fail[%d]", aiDevId, ret);
        return ret;
    }

    ret = hd_audioenc_open(HD_AUDIOENC_IN(0, aiDevId), HD_AUDIOENC_OUT(0, aiDevId), &aePathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audioenc_open[%d] fail[%d]\n", aiDevId, ret);
        return ret;
    }

    memset(&aiCfg, 0, sizeof(aiCfg));
    aiCfg.data_pool[0].mode = HD_AUDIOCAP_POOL_ENABLE;
    aiCfg.data_pool[0].ddr_id = 0;
    aiCfg.data_pool[0].frame_sample_size = 320 * 2;
    aiCfg.data_pool[0].counts = HD_AUDIOCAP_SET_COUNT(20, 0);
    ret = hd_audiocap_set(aiPathId, HD_AUDIOCAP_PARAM_DEV_CONFIG, &aiCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_set(HD_AUDIOCAP_PARAM_PATH_CONFIG) fail[%d]\n", ret);
        return ret;
    }

    /* set audiocap parameters */
    ret = hd_audiocap_get(aiPathId, HD_AUDIOCAP_PARAM_IN, &aiParm);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_get(HD_AUDIOCAP_PARAM_IN) fail[%d]\n", ret);
        return ret;
    }

    aiParm.sample_rate = enInSampleRate;
    aiParm.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    aiParm.mode = HD_AUDIO_SOUND_MODE_MONO;
    aiParm.frame_sample = 320;
    ret = hd_audiocap_set(aiPathId, HD_AUDIOCAP_PARAM_IN, &aiParm);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_set(HD_AUDIOCAP_PARAM_IN) fail[%d]\n", ret);
        return ret;
    }

    /* set audioenc parameters */
    ret = hd_audioenc_get(aePathId, HD_AUDIOENC_PARAM_OUT, &aeParam);
    if (ret != HD_OK) {
        VAPILOG("hd_audioenc_get(HD_AUDIOENC_PARAM_OUT) fail[%d]\n", ret);
        return ret;
    }

    aeParam.codec_type = HD_AUDIO_CODEC_PCM;
    ret = hd_audioenc_set(aePathId, HD_AUDIOENC_PARAM_OUT, &aeParam);
    if (ret != HD_OK) {
        VAPILOG("hd_audioenc_set(HD_AUDIOENC_PARAM_OUT) fail[%d]\n", ret);
        return ret;
    }

    ret = hd_audiocap_bind(HD_AUDIOCAP_OUT(aiDevId, 0), HD_AUDIOENC_IN(0, aiDevId));
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_bind fail[%d]\n", ret);
        return ret;
    }

    ret = hd_audiocap_start(aiPathId);
    if (ret != HD_OK) {
        VAPILOG("start audiocap fail[%d]\n", ret);
        return ret;
    }

    ret = hd_audioenc_start(aePathId);
    if (ret != HD_OK) {
        VAPILOG("start audioenc fail[%d]\n", ret);
        return ret;
    }

    if (g_stAioInfo.stAiDev[aiDevId].buffer[0] == HI_NULL) {
        g_stAioInfo.stAiDev[aiDevId].buffer[0] = (HI_U8 *)malloc(MAX_BUFF_SIZE);
        if (g_stAioInfo.stAiDev[aiDevId].buffer[0] == HI_NULL) {
            VAPILOG("no enoungh memory !\n");
            return HI_FAILURE;
        }
        g_stAioInfo.stAiDev[aiDevId].buffer[1] = (HI_U8 *)malloc(MAX_BUFF_SIZE);
        if (g_stAioInfo.stAiDev[aiDevId].buffer[1] == HI_NULL) {
            VAPILOG("no enoungh memory !\n");
            return HI_FAILURE;
        }
    }

    comm_mutex_init(&g_stAioInfo.stAiDev[aiDevId].mutex);
    g_stAioInfo.stAiDev[aiDevId].aiPathId = aiPathId;
    g_stAioInfo.stAiDev[aiDevId].aePathId = aePathId;
    g_stAioInfo.stAiDev[aiDevId].sampleRate = enInSampleRate;
    g_stAioInfo.stAiDev[aiDevId].enState = STATE_BUSY;

    return ret;
}

static HD_RESULT aio_ai_stop(HI_S32 aiDevId)
{
    HD_RESULT ret = HD_OK;

    comm_mutex_lock(&g_stAioInfo.stAiDev[aiDevId].mutex);

    ret = hd_audiocap_stop(g_stAioInfo.stAiDev[aiDevId].aiPathId);
    if (ret != HD_OK) {
        VAPILOG("stop audiocap fail[%d]\n", ret);
    }

    ret = hd_audioenc_stop(g_stAioInfo.stAiDev[aiDevId].aePathId);
    if (ret != HD_OK) {
        VAPILOG("stop audioenc fail[%d]\n", ret);
    }

    ret = hd_audiocap_unbind(HD_AUDIOCAP_OUT(aiDevId, 0));
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_unbind fail[%d]\n", ret);
    }

    ret = hd_audiocap_close(g_stAioInfo.stAiDev[aiDevId].aiPathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_close fail[%d]\n", ret);
    }

    ret = hd_audioenc_close(g_stAioInfo.stAiDev[aiDevId].aePathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audiocap_close fail[%d]\n", ret);
    }

    if (g_stAioInfo.stAiDev[aiDevId].buffer[0] != HI_NULL) {
        free(g_stAioInfo.stAiDev[aiDevId].buffer[0]);
        g_stAioInfo.stAiDev[aiDevId].buffer[0] = HI_NULL;
    }

    if (g_stAioInfo.stAiDev[aiDevId].buffer[1] != HI_NULL) {
        free(g_stAioInfo.stAiDev[aiDevId].buffer[1]);
        g_stAioInfo.stAiDev[aiDevId].buffer[1] = HI_NULL;
    }



    g_stAioInfo.stAiDev[aiDevId].enState = STATE_IDLE;

    comm_mutex_unlock(&g_stAioInfo.stAiDev[aiDevId].mutex);

    comm_mutex_uninit(&g_stAioInfo.stAiDev[aiDevId].mutex);

    return ret;
}

static HD_RESULT aio_ao_start(AO_E aoDevId, ARATE_E enOutSampleRate, int frame_sample_size)
{
    HD_RESULT ret = HD_OK;
    HD_AUDIODEC_PATH_CONFIG adCfg;
    HD_AUDIODEC_IN adParam;
    HD_AUDIOOUT_OUT aoParam;
    int error;
#if 0
    HD_AUDIOOUT_DRV_CONFIG aoDrvCfg;
    ret = hd_audioout_get(g_stAioInfo.aoCtrlPath, HD_AUDIOOUT_PARAM_DRV_CONFIG, &aoDrvCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_audioout_get(HD_AUDIOOUT_PARAM_DRV_CONFIG) fail\n");
        return ret;
    }

    aoDrvCfg.ssp_config.enable[aoDevId] = 1;
    aoDrvCfg.ssp_config.resample_ratio[aoDevId] = enOutSampleRate / DEFAULT_SAMPLE_RATE;
    aoDrvCfg.ssp_config.playback_chmap[aoDevId] = -1;

    ret = hd_audioout_set(g_stAioInfo.aoCtrlPath, HD_AUDIOOUT_PARAM_DRV_CONFIG, &aoDrvCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_audioout_set(HD_AUDIOOUT_PARAM_DRV_CONFIG) samplerate[%d]fail\n", enOutSampleRate);
        return ret;
    }
#endif
    ret = hd_audioout_open(HD_AUDIOOUT_IN(aoDevId, 0), HD_AUDIOOUT_OUT(aoDevId, 0),
                           &g_stAioInfo.stAoDev[aoDevId].aoPathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audioout_open[%d] fail[%d]\n", aoDevId, ret);
        return ret;
    }

    ret = hd_audiodec_open(HD_AUDIODEC_IN(0, aoDevId), HD_AUDIODEC_OUT(0, aoDevId),
                           &g_stAioInfo.stAoDev[aoDevId].adPathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_open[%d] fail[%d]\n", aoDevId, ret);
        return ret;
    }

    memset(&adCfg, 0, sizeof(adCfg));
    adCfg.data_pool[0].mode = HD_AUDIODEC_POOL_ENABLE;
    adCfg.data_pool[0].ddr_id = 0;
    adCfg.data_pool[0].frame_sample_size = frame_sample_size;
    adCfg.data_pool[0].counts = HD_AUDIODEC_SET_COUNT(10, 0);
    ret = hd_audiodec_set(g_stAioInfo.stAoDev[aoDevId].adPathId, HD_AUDIODEC_PARAM_PATH_CONFIG, &adCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_set(HD_AUDIODEC_PARAM_PATH_CONFIG) fail[%d]\n", ret);
        return ret;
    }

    /* set audiodec parameters */
    ret = hd_audiodec_get(g_stAioInfo.stAoDev[aoDevId].adPathId, HD_AUDIODEC_PARAM_IN, &adParam);
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_get(HD_AUDIODEC_PARAM_IN) fail[%d]\n", ret);
        return ret;
    }

    adParam.codec_type = HD_AUDIO_CODEC_PCM;
    adParam.sample_rate = enOutSampleRate;
    adParam.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    adParam.mode = HD_AUDIO_SOUND_MODE_MONO;
    ret = hd_audiodec_set(g_stAioInfo.stAoDev[aoDevId].adPathId, HD_AUDIODEC_PARAM_IN, &adParam);
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_set(HD_AUDIODEC_PARAM_IN) fail[%d]\n", ret);
        return ret;
    }

    /* set audioout parameters */
    HD_AUDIOOUT_IN audioin_param;
    memset(&audioin_param, 0, sizeof(audioin_param));
    audioin_param.sample_rate = enOutSampleRate;
    ret = hd_audioout_set(g_stAioInfo.stAoDev[aoDevId].aoPathId, HD_AUDIOOUT_PARAM_IN, &audioin_param);
    if (ret != HD_OK) {
        printf("hd_audiodec_set(HD_AUDIOOUT_PARAM_IN) fail\n");
        return ret;
    }

    ret = hd_audioout_get(g_stAioInfo.stAoDev[aoDevId].aoPathId, HD_AUDIOOUT_PARAM_OUT, &aoParam);
    if (ret != HD_OK) {
        VAPILOG("hd_audioout_get(HD_AUDIOOUT_PARAM_OUT) fail[%d]\n", ret);
        return ret;
    }

    aoParam.sample_rate = enOutSampleRate;
    aoParam.sample_bit = HD_AUDIO_BIT_WIDTH_16;
    aoParam.mode = HD_AUDIO_SOUND_MODE_MONO;
    ret = hd_audioout_set(g_stAioInfo.stAoDev[aoDevId].aoPathId, HD_AUDIOOUT_PARAM_OUT, &aoParam);
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_set(HD_AUDIOOUT_PARAM_OUT) fail[%d]\n", ret);
        return ret;
    }

    ret = hd_audiodec_bind(HD_AUDIODEC_OUT(0, aoDevId), HD_AUDIOOUT_IN(aoDevId, 0));
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_bind fail[%d]\n", ret);
        return ret;
    }

    ret = hd_audiodec_start(g_stAioInfo.stAoDev[aoDevId].adPathId);
    if (ret != HD_OK) {
        VAPILOG("start audiodec fail[%d]\n", ret);
        return ret;
    }
    ret = hd_audioout_start(g_stAioInfo.stAoDev[aoDevId].aoPathId);
    if (ret != HD_OK) {
        VAPILOG("start audioout fail[%d]\n", ret);
        return ret;
    }

    if (g_stAioInfo.stAoDev[aoDevId].buffer[0] == HI_NULL) {
        g_stAioInfo.stAoDev[aoDevId].buffer[0] = (HI_U8 *)malloc(MAX_BUFF_SIZE);
        if (g_stAioInfo.stAoDev[aoDevId].buffer[0] == HI_NULL) {
            VAPILOG("no enoungh memory !\n");
            return HI_FAILURE;
        }
        g_stAioInfo.stAoDev[aoDevId].buffer[1] = (HI_U8 *)malloc(MAX_BUFF_SIZE);
        if (g_stAioInfo.stAoDev[aoDevId].buffer[1] == HI_NULL) {
            VAPILOG("no enoungh memory !\n");
            return HI_FAILURE;
        }
    }

    if (aoDevId == AO_SPK_MIC) {
#ifdef SPEEX
        vapi_aio_speex_echo_init(enOutSampleRate, 640);    //denoisy
#endif
    }
    if ((src_fullstate = src_new(SRC_ZERO_ORDER_HOLD, 1, &error)) == NULL) {
        VAPILOG("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror(error)) ;
        return HI_FAILURE;
    }
    comm_mutex_init(&g_stAioInfo.stAoDev[aoDevId].mutex);
    g_stAioInfo.stAoDev[aoDevId].enState = STATE_BUSY;
    g_stAioInfo.stAoDev[aoDevId].sampleRate = enOutSampleRate;

    return ret;
}

static HD_RESULT aio_ao_stop(AO_E aoDevId)
{
    HD_RESULT ret = HD_OK;

    if (aoDevId == AO_SPK_MIC) {
#ifdef SPEEX
        vapi_aio_speex_echo_deinit();
#endif
    }
    src_fullstate = src_delete(src_fullstate);
    comm_mutex_lock(&g_stAioInfo.stAoDev[aoDevId].mutex);

    ret = hd_audiodec_stop(g_stAioInfo.stAoDev[aoDevId].adPathId);
    if (ret != HD_OK) {
        VAPILOG("stop audiodec fail[%d]\n", ret);
    }

    ret = hd_audioout_stop(g_stAioInfo.stAoDev[aoDevId].aoPathId);
    if (ret != HD_OK) {
        VAPILOG("stop audioout fail[%d]\n", ret);
    }

    ret = hd_audiodec_unbind(HD_AUDIODEC_OUT(0, aoDevId));
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_unbind fail[%d]\n", ret);
    }

    ret = hd_audioout_close(g_stAioInfo.stAoDev[aoDevId].aoPathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audioout_close fail[%d]\n", ret);
    }

    ret = hd_audiodec_close(g_stAioInfo.stAoDev[aoDevId].adPathId);
    if (ret != HD_OK) {
        VAPILOG("hd_audioout_close fail[%d]\n", ret);
    }

    if (g_stAioInfo.stAoDev[aoDevId].buffer[0] != HI_NULL) {
        free(g_stAioInfo.stAoDev[aoDevId].buffer[0]);
        g_stAioInfo.stAoDev[aoDevId].buffer[0] = HI_NULL;
    }

    if (g_stAioInfo.stAoDev[aoDevId].buffer[1] != HI_NULL) {
        free(g_stAioInfo.stAoDev[aoDevId].buffer[1]);
        g_stAioInfo.stAoDev[aoDevId].buffer[1] = HI_NULL;
    }

    g_stAioInfo.stAoDev[aoDevId].enState = STATE_IDLE;

    comm_mutex_unlock(&g_stAioInfo.stAoDev[aoDevId].mutex);
    comm_mutex_uninit(&g_stAioInfo.stAoDev[aoDevId].mutex);

    return ret;
}

static int reset_audiodec_frame_sample_size(int aoDevId, int frame_sample_size)
{
    HD_RESULT ret = HD_OK;

    ret = hd_audiodec_stop(g_stAioInfo.stAoDev[aoDevId].adPathId);
    if (ret != HD_OK) {
        VAPILOG("stop audiodec fail[%d]\n", ret);
    }

    HD_AUDIODEC_PATH_CONFIG adCfg;
    memset(&adCfg, 0, sizeof(adCfg));
    adCfg.data_pool[0].mode = HD_AUDIODEC_POOL_ENABLE;
    adCfg.data_pool[0].ddr_id = 0;
    adCfg.data_pool[0].frame_sample_size = frame_sample_size;
    adCfg.data_pool[0].counts = HD_AUDIODEC_SET_COUNT(10, 0);
    ret = hd_audiodec_set(g_stAioInfo.stAoDev[aoDevId].adPathId, HD_AUDIODEC_PARAM_PATH_CONFIG, &adCfg);
    if (ret != HD_OK) {
        VAPILOG("hd_audiodec_set(HD_AUDIODEC_PARAM_PATH_CONFIG) fail[%d]\n", ret);
        return ret;
    }

    ret = hd_audiodec_start(g_stAioInfo.stAoDev[aoDevId].adPathId);
    if (ret != HD_OK) {
        VAPILOG("start audiodec fail[%d]\n", ret);
        return ret;
    }

    return ret;
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

HI_S32 vapi_aio_ai_read_frame(HI_S32 aiDevId, ARATE_E enOutSampleRate, AENC_E enAenc,
                              AIO_FRAME_S *pstFrame)
{
    HI_S32 s32Ret;
    unsigned char *dst;
    short *src;
    unsigned int srcSize;

    if (!g_stAioInfo.bHasCodec) {
        return HI_FAILURE;
    }

    if (g_stAioInfo.stAiDev[aiDevId].enState == STATE_IDLE) {
        return HI_FAILURE;
    }

    if (enOutSampleRate != ARATE_8000) {
        VAPILOG("do not support sample rate [%d], only supported[%d]", enOutSampleRate, ARATE_8000);
        return HI_FAILURE;
    }

    comm_mutex_lock(&g_stAioInfo.stAiDev[aiDevId].mutex);

    if (g_stAioInfo.stAiDev[aiDevId].enState == STATE_IDLE) {
        comm_mutex_unlock(&g_stAioInfo.stAiDev[aiDevId].mutex);
        return HI_FAILURE;
    }

    //reset resample rate if necessary
    src = (short *)g_stAioInfo.stAiDev[aiDevId].buffer[0];
    dst = g_stAioInfo.stAiDev[aiDevId].buffer[1];

    s32Ret = aio_ai_read(aiDevId, src, &srcSize);
    if (HI_SUCCESS != s32Ret) {
        comm_mutex_unlock(&g_stAioInfo.stAiDev[aiDevId].mutex);
        return HI_FAILURE;
    }

    switch (enAenc) {
        case AENC_PCM_ULAW:
            pstFrame->len = vapi_aio_ulaw_encode(dst, src, srcSize);//音频压缩从src到dst
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
    pstFrame->pts       = 0;
    pstFrame->date      = dst;

    /* finally you must release the stream */

    comm_mutex_unlock(&g_stAioInfo.stAiDev[aiDevId].mutex);

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

HI_S32 vapi_aio_ao_write_frame(HI_S32 aoDevId, AIO_FRAME_S *pstFrame)
{
    int i = 0;
    HI_S32 multiple = 0;
    short *dst;
    float aoSrcBuf[4096];
    float aoResampleBuf[8192];
    unsigned char *src = pstFrame->date;
    unsigned int srcSize = pstFrame->len;
    unsigned int dstSize;
	
#ifdef ADUIO_DENOISE
    if (aoDevId == AO_MAIN_HDMI) {
        return 0;
    }
#endif
    if (aoDevId == AO_SPK_MIC && !g_stAioInfo.bHasCodec) {
        return HI_FAILURE;
    }
    comm_mutex_lock(&g_stAioInfo.stAoDev[aoDevId].mutex);
    if (g_stAioInfo.stAoDev[aoDevId].enState == STATE_IDLE) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[aoDevId].mutex);
        return HI_FAILURE;
    }

    multiple = vapi_aio_ao_get_multiple(pstFrame);

    if (srcSize * multiple > MAX_BUFF_SIZE) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[aoDevId].mutex);
        return HI_FAILURE;
    }
    //reset resample rate if necessary
#ifdef ADUIO_DENOISE
    fwrite(pstFrame->date, sizeof(unsigned char), pstFrame->len, pfd_src);
    printf("src_size : %d.\n", pstFrame->len);
#endif
    dst = (short *)g_stAioInfo.stAoDev[aoDevId].buffer[0];

    switch (pstFrame->enAenc) {
        case AENC_PCM_ULAW:
            dstSize = vapi_aio_ulaw_decode(dst, src, srcSize);
            break;
        case AENC_PCM_ALAW:
            dstSize = vapi_aio_alaw_decode(dst, src, srcSize);
            break;
        case AENC_PCM_AAC:
            dstSize = vapi_aio_aac_decode(dst, src, srcSize, pstFrame->enArate);
            break;
        case AENC_PCM_G722:
            dstSize = vapi_aio_g722_decode(dst, src, srcSize);
            break;
        case AENC_PCM_G726:
            dstSize = vapi_aio_g726_decode(dst, src, srcSize);
            break;
        case AENC_PCM_RAW:
        default:
            dstSize = srcSize;
            memcpy(dst, src, srcSize);
            break;
    }
    
    if (dstSize <= 0) {
        comm_mutex_unlock(&g_stAioInfo.stAoDev[aoDevId].mutex);
        return HI_FAILURE;
    }

    if (pstFrame->enAenc == AENC_PCM_RAW && pstFrame->enAbit == ABIT_8_BIT) {
        for (i = 0; i < srcSize; i++) {
            ((short *)dst)[i] = (short)(((unsigned char *)src)[i] - 0x80) << 8;
        }
        dstSize = 2 * srcSize;
    }

#ifdef ADUIO_DENOISE
    fwrite(dst, sizeof(unsigned char), dstSize, pfd_dst);
    printf("dst_size : %d.\n", dstSize);
#endif
    if (pstFrame->enArate != g_stAioInfo.stAoDev[aoDevId].sampleRate) {
        srcSize = dstSize / 2;
        src_short_to_float_array((const short *)dst, aoSrcBuf, srcSize);
        dst = (short *)g_stAioInfo.stAoDev[aoDevId].buffer[1];
        if (aio_resample(aoSrcBuf, srcSize, pstFrame->enArate,
                          aoResampleBuf, &dstSize, g_stAioInfo.stAoDev[aoDevId].sampleRate)) {
            comm_mutex_unlock(&g_stAioInfo.stAoDev[aoDevId].mutex);
            return HI_FAILURE;
        }
        src_float_to_short_array((const float *)aoResampleBuf, (short *)dst, dstSize);
        dstSize = dstSize * 2;
    }
#ifdef ADUIO_DENOISE
    fwrite(dst, sizeof(short), dstSize, pfd_dst_re);
    printf("resampler_size : %d.\n", dstSize);
#endif
#ifdef SPEEX
    speex_preprocess_run(g_stDnCfg.state, (spx_int16_t *)(dst));
#endif

    static int pcm_buff_size[2] = {320, 1920};
    int dst_index = 0;
    while (dst_index < dstSize) {
        if (dstSize - dst_index < pcm_buff_size[aoDevId] - g_stAioInfo.stAoDev[aoDevId].pcm_buff_index) {
            memcpy(g_stAioInfo.stAoDev[aoDevId].pcm_buff + g_stAioInfo.stAoDev[aoDevId].pcm_buff_index, (char *)dst + dst_index, dstSize - dst_index);
            g_stAioInfo.stAoDev[aoDevId].pcm_buff_index += dstSize - dst_index;
            break;
        } else {
            memcpy(g_stAioInfo.stAoDev[aoDevId].pcm_buff + g_stAioInfo.stAoDev[aoDevId].pcm_buff_index, (char *)dst + dst_index, pcm_buff_size[aoDevId] - g_stAioInfo.stAoDev[aoDevId].pcm_buff_index);
            dst_index += pcm_buff_size[aoDevId] - g_stAioInfo.stAoDev[aoDevId].pcm_buff_index;
            g_stAioInfo.stAoDev[aoDevId].pcm_buff_index = 0;
            aio_ao_write(aoDevId, pstFrame->enArate, g_stAioInfo.stAoDev[aoDevId].pcm_buff, pcm_buff_size[aoDevId]);
        }
    }

    comm_mutex_unlock(&g_stAioInfo.stAoDev[aoDevId].mutex);

    return HI_SUCCESS;
}

HD_RESULT vapi_aio_init()
{
    HD_RESULT ret = HD_OK;

#ifdef ADUIO_DENOISE
    pfd_src = fopen("/output/audio_src.pcm", "wb");
    if (pfd_src == NULL) {
        printf("open /output/audio_src.pcm failed.");
    }
    pfd_dst = fopen("/output/audio_dst.pcm", "wb");
    if (pfd_src == NULL) {
        printf("open /output/audio_dst.pcm failed.");
    }
    pfd_dst_re = fopen("/output/audio_dst_re.pcm", "wb");
    if (pfd_src == NULL) {
        printf("open /output/audio_dst_re.pcm failed.");
    }
    printf("open file %#x,%#x,%#x ...", pfd_src, pfd_dst, pfd_dst_re);
#endif

    memset(&g_stAioInfo, 0, sizeof(AIO_INFO_S));

    if ((ret = aio_set_codec(DEFAULT_SAMPLE_RATE)) != HD_OK) {
        g_stAioInfo.bHasCodec = HI_FALSE;
    } else {
        g_stAioInfo.bHasCodec = HI_TRUE;
    }

    if ((ret = hd_audiocap_init()) != HD_OK) {
        VAPILOG("hd_audiocap_init fail[%d]\n", ret);
        return ret;
    }

    if ((ret = hd_audioout_init()) != HD_OK) {
        VAPILOG("hd_audioout_init fail[%d]\n", ret);
        return ret;
    }
    if ((ret = hd_audiodec_init()) != HD_OK) {
        VAPILOG("hd_audiodec_init fail[%d]\n", ret);
        return ret;
    }

    if ((ret = hd_audioenc_init()) != HD_OK) {
        VAPILOG("hd_audiodec_init fail[%d]\n", ret);
        return ret;
    }

    if ((ret = hd_audiocap_open(0, HD_AUDIOCAP_0_CTRL, &g_stAioInfo.aiCtrlPath)) != HD_OK) {
        VAPILOG("hd_audiocap_open fail[%d]\n", ret);
        return ret;
    }

    if ((ret = hd_audioout_open(0, HD_AUDIOOUT_0_CTRL, &g_stAioInfo.aoCtrlPath)) != HD_OK) {
        VAPILOG("hd_audiocap_open fail[%d]\n", ret);
        return ret;
    }

    if ((ret = aio_ai_start(0, DEFAULT_SAMPLE_RATE)) != HD_OK) {
        VAPILOG("aio_ai_start 0 fail\n");
        return ret;
    }

    if ((ret = aio_ao_start(AO_SPK_MIC, DEFAULT_SAMPLE_RATE, 320)) != HD_OK) {
        VAPILOG("aio_ao_start AO_SPK_MIC fail\n");
        return ret;
    }

    if ((ret = aio_ao_start(AO_MAIN_HDMI, ARATE_48000, 1920)) != HD_OK) {
        VAPILOG("aio_ao_start AO_MAIN_HDMI fail\n");
        return ret;
    }

    vapi_aio_aac_init();

    return ret;
}

HD_RESULT vapi_aio_uninit()
{
    HD_RESULT ret = HD_OK;

    vapi_aio_aac_uninit();

    if ((ret = aio_ai_stop(0)) != HD_OK) {
        VAPILOG("aio_ai_stop 0 fail\n");
    }

    if ((ret = aio_ao_stop(AO_SPK_MIC)) != HD_OK) {
        VAPILOG("aio_ao_stop AO_SPK_MIC fail\n");
    }

    if ((ret = aio_ao_stop(AO_MAIN_HDMI)) != HD_OK) {
        VAPILOG("aio_ao_stop AO_MAIN_HDMI fail\n");
    }

    if ((ret = hd_audiocap_close(g_stAioInfo.aiCtrlPath)) != HD_OK) {
        VAPILOG("hd_audiocap_close fail[%d]\n", ret);
    }

    if ((ret = hd_audioout_close(g_stAioInfo.aoCtrlPath)) != HD_OK) {
        VAPILOG("hd_audioout_close fail[%d]\n", ret);
    }

    if ((ret = hd_audiocap_uninit()) != HD_OK) {
        VAPILOG("hd_audiocap_uninit fail[%d]\n", ret);
    }

    if ((ret = hd_audioout_uninit()) != HD_OK) {
        VAPILOG("hd_audioout_uninit fail[%d]\n", ret);
    }

    if ((ret = hd_audiodec_uninit()) != HD_OK) {
        VAPILOG("hd_audiodec_uninit fail[%d]\n", ret);
    }

    if ((ret = hd_audioenc_uninit()) != HD_OK) {
        VAPILOG("hd_audioenc_uninit fail[%d]\n", ret);
    }

#ifdef ADUIO_DENOISE
    fclose(pfd_src);
    pfd_src = NULL;
    fclose(pfd_dst);
    pfd_dst = NULL;
    fclose(pfd_dst_re);
    pfd_dst_re = NULL;
#endif
    return ret;
}




