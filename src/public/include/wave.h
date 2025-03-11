#ifndef WAVE_H
#define WAVE_H

#if defined (__cplusplus)
extern "C" {
#endif

/* Possible Audio formats */

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_IBM_CVSD            (0x0005)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM        (0x0020)
#define WAVE_FORMAT_DSP_TRUESPEECH      (0x0022)
#define WAVE_FORMAT_GSM610              (0x0031)
#define IBM_FORMAT_MULAW                (0x0101)
#define IBM_FORMAT_ALAW                 (0x0102)
#define IBM_FORMAT_ADPCM                (0x0103)
#define WAVE_FORMAT_AAC                 (0x00ff)
#define WAVE_FORMAT_G726_ADPCM          (0x0064)
#define WAVE_FORMAT_G722_ADPCM          (0x028F) /* G.722 ADPCM   0x0065 0x028F*/

#endif

typedef struct wav_riff {
    /* chunk "riff" */
    char chunkId[4];   /* "RIFF" */
    /* sub-chunk-size */
    uint32_t chunkSize; /* 36 + subchunk2Id */
    /* sub-chunk-data */
    char format[4];    /* "WAVE" */
} WAV_RIFF_T;

#define WAV_FMT_SIZE_PCM                (16)
typedef struct wav_fmt {
    /* sub-chunk "fmt" */
    char subchunk1Id[4];    /* "fmt " */
    /* sub-chunk-size */
    uint32_t subchunk1Size; /* 16 for PCM */
    /* sub-chunk-data */
    uint16_t audioFormat;   /* PCM = 1*/
    uint16_t numChannels;   /* Mono = 1, Stereo = 2, etc. */
    uint32_t sampleRate;    /* 8000, 44100, etc. */
    uint32_t byteRate;      /* = sampleRate * numChannels * bitsPerSample/8 */
    uint16_t blockAlign;    /* = NumChannels * BitsPerSample/8 */
    uint16_t bitsPerSample; /* 8bits, 16bits, etc. */
} WAV_FMT_T;

typedef struct wav_fact {
    uint16_t subchunk2Size; /* fact size */
    uint16_t payloadBps; /* <= bitsPerSample */
    uint32_t channelMask; /* 声道号与扬声器位置映射的二进制掩码 */
    uint16_t factFormat; /* actually  audio format */
    char resolve[14]; /* \x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71 */
} WAV_FACT_T;

typedef struct wav_data {
    /* sub-chunk "data" */
    char subchunk3Id[4];    /* "data" */
    /* sub-chunk-size */
    uint32_t subchunk3Size; /* data size */
    /* sub-chunk-data */
//    Data_block_t block;
} WAV_DATA_T;

//typedef struct WAV_data_block {
//} Data_block_t;

typedef struct wav {
   WAV_RIFF_T riff;
   WAV_FMT_T fmt;
   WAV_FACT_T fact;
   WAV_DATA_T data;
} WAV;

#if defined (__cplusplus)
}
#endif

#endif