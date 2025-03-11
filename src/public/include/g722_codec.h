/**************
IS_FF:1    ffmpeg
        :0    github
**************/
#define IS_FF 1

#if IS_FF

#ifndef GET_BITS
#include "g72x_get_bits.h"
#endif

unsigned int g722_decode(short* dst, const unsigned char* src, unsigned int srcSize);

#else

#include <endian.h>
#include <stdint.h>
#include <math.h>

enum
{
    G722_SAMPLE_RATE_8000 = 0x0001,
    G722_PACKED = 0x0002
};


#define _G722_ENC_CTX_DEFINED
typedef struct g722_codec_state G722_CTX;
#define _G722_DEC_CTX_DEFINED

struct g722_codec_state
{
    /*! TRUE if the operating in the special ITU test mode, with the band split filters
             disabled. */
    int itu_test_mode;
    /*! TRUE if the G.722 data is packed */
    int packed;
    /*! TRUE if decode or encode to 8k samples/second */
    int eight_k;
    /*! 6 for 48000kbps, 7 for 56000kbps, or 8 for 64000kbps. */
    int bits_per_sample;

    /*! Signal history for the QMF */
    int x[24];

    struct
    {
        int s;
        int sp;
        int sz;
        int r[3];
        int a[3];
        int ap[3];
        int p[3];
        int d[7];
        int b[7];
        int bp[7];
        int sg[7];
        int nb;
        int det;
    } band[2];

    unsigned int in_buffer;
    int in_bits;
    unsigned int out_buffer;
    int out_bits;
};

unsigned int g722_decode(short* dst, const unsigned char* src, unsigned int srcSize);

#endif

