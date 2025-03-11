
#define IS_FFG726 (1)

#if IS_FFG726

#ifndef GET_BITS
#include "g72x_get_bits.h"
#endif


#define INT9_MAX            (255)
#define INT9_MIN            (-256)

#ifndef ABS
#define ABS(x)              ((x) >= 0 ? (x) : (-(x)))
#endif

#define av_log2_16bit(x)    (31 - __builtin_clz((x)|1))

typedef struct Float11
{
    uint8_t sign;   /**< 1 bit sign */
    uint8_t exp;    /**< 4 bits exponent */
    uint8_t mant;   /**< 6 bits mantissa */
} Float11;

static __inline__ int16_t saturate9(int32_t amp)
{
    return saturate(amp, INT9_MIN, INT9_MAX);
}

static inline Float11* i2f(int i, Float11* f)
{
    f->sign = (i < 0);
    if (f->sign)
        i = -i;
    f->exp = av_log2_16bit(i) + !!i;
    f->mant = i? (i<<6) >> f->exp : 1<<5;
    return f;
}

static inline int16_t mult(Float11* f1, Float11* f2)
{
    int res, exp;

    exp = f1->exp + f2->exp;
    res = (((f1->mant * f2->mant) + 0x30) >> 4);
    res = exp > 19 ? res << (exp - 19) : res >> (19 - exp);
    return (f1->sign ^ f2->sign) ? -res : res;
}

static inline int sgn(int value)
{
    return (value < 0) ? -1 : 1;
}

typedef struct G726Tables
{
    const int* quant;         /**< quantization table */
    const int16_t* iquant;    /**< inverse quantization table */
    const int16_t* W;         /**< special table #1 ;-) */
    const uint8_t* F;         /**< special table #2 */
} G726Tables;

typedef struct G726Context
{
    G726Tables tbls;    /**< static tables needed for computation */

    Float11 sr[2];      /**< prev. reconstructed samples */
    Float11 dq[6];      /**< prev. difference */
    int a[2];           /**< second order predictor coeffs */
    int b[6];           /**< sixth order predictor coeffs */
    int pk[2];          /**< signs of prev. 2 sez + dq */

    int ap;             /**< scale factor control */
    int yu;             /**< fast scale factor */
    int yl;             /**< slow scale factor */
    int dms;            /**< short average magnitude of F[i] */
    int dml;            /**< long average magnitude of F[i] */
    int td;             /**< tone detect */

    int se;             /**< estimated signal for the next iteration */
    int sez;            /**< estimated second order prediction */
    int y;              /**< quantizer scaling factor for the next iteration */
    int code_size;
    int little_endian;  /**< little-endian bitstream as used in aiff and Sun AU */
} G726Context;

unsigned int g726_decode(short* dst, const unsigned char* src, unsigned int srcSize);


//////////////////////////////////////////////////////////////////////
#else

/*! Bitstream handler state */
typedef struct bitstream_state_s
{
	/*! The bit stream. */
	unsigned int bitstream;
	/*! The residual bits in bitstream. */
	int residue;
}bitstream_state_t;

typedef struct g726_state_s g726_state_t;
typedef short (*g726_decoder_func_t)(g726_state_t *s, unsigned char code);
typedef unsigned char (*g726_encoder_func_t)(g726_state_t *s, short amp);


/*!
* The following is the definition of the state structure
* used by the G.726 encoder and decoder to preserve their internal
* state between successive calls.  The meanings of the majority
* of the state structure fields are explained in detail in the
* CCITT Recommendation G.726.  The field names are essentially indentical
* to variable names in the bit level description of the coding algorithm
* included in this recommendation.
*/
struct g726_state_s
{
	/*! The bit rate */
	int rate;
	/*! The external coding, for tandem operation */
	//int ext_coding;
	/*! The number of bits per sample */
	int bits_per_sample;
	/*! One of the G.726_PACKING_xxx options */
	//int packing;

	/*! Locked or steady state step size multiplier. */
	int yl;
	/*! Unlocked or non-steady state step size multiplier. */
	short yu;
	/*! short term energy estimate. */
	short dms;
	/*! Long term energy estimate. */
	short dml;
	/*! Linear weighting coefficient of 'yl' and 'yu'. */
	short ap;

	/*! Coefficients of pole portion of prediction filter. */
	short a[2];
	/*! Coefficients of zero portion of prediction filter. */
	short b[6];
	/*! Signs of previous two samples of a partially reconstructed signal. */
	short pk[2];
	/*! Previous 6 samples of the quantized difference signal represented in
	an internal floating point format. */
	short dq[6];
	/*! Previous 2 samples of the quantized difference signal represented in an
	internal floating point format. */
	short sr[2];
	/*! Delayed tone detect */
	int td;

	/*! \brief The bit stream processing context. */
	bitstream_state_t bs;

	/*! \brief The current encoder function. */
	g726_encoder_func_t enc_func;
	/*! \brief The current decoder function. */
	g726_decoder_func_t dec_func;
};

/*
* Maps G.726_16 code word to reconstructed scale factor normalized log
* magnitude values.
*/
static const int g726_16_dqlntab[4] =
{
	116, 365, 365, 116
};

/* Maps G.726_16 code word to log of scale factor multiplier. */
static const int g726_16_witab[4] =
{
	-704, 14048, 14048, -704
};

/*
* Maps G.726_16 code words to a set of values whose long and short
* term averages are computed and then compared to give an indication
* how stationary (steady state) the signal is.
*/
static const int g726_16_fitab[4] =
{
	0x000, 0xE00, 0xE00, 0x000
};

/*
* Maps G.726_24 code word to reconstructed scale factor normalized log
* magnitude values.
*/
static const int g726_24_dqlntab[8] =
{
	-2048, 135, 273, 373, 373, 273, 135, -2048
};

/* Maps G.726_24 code word to log of scale factor multiplier. */
static const int g726_24_witab[8] =
{
	-128, 960, 4384, 18624, 18624, 4384, 960, -128
};

/*
* Maps G.726_24 code words to a set of values whose long and short
* term averages are computed and then compared to give an indication
* how stationary (steady state) the signal is.
*/
static const int g726_24_fitab[8] =
{
	0x000, 0x200, 0x400, 0xE00, 0xE00, 0x400, 0x200, 0x000
};

/*
* Maps G.726_32 code word to reconstructed scale factor normalized log
* magnitude values.
*/
static const int g726_32_dqlntab[16] =
{
	-2048,   4, 135, 213, 273, 323, 373,   425,
	425, 373, 323, 273, 213, 135,   4, -2048
};

/* Maps G.726_32 code word to log of scale factor multiplier. */
static const int g726_32_witab[16] =
{
	-384,   576,  1312,  2048,  3584,  6336, 11360, 35904,
	35904, 11360,  6336,  3584,  2048,  1312,   576,  -384
};

/*
* Maps G.726_32 code words to a set of values whose long and short
* term averages are computed and then compared to give an indication
* how stationary (steady state) the signal is.
*/
static const int g726_32_fitab[16] =
{
	0x000, 0x000, 0x000, 0x200, 0x200, 0x200, 0x600, 0xE00,
	0xE00, 0x600, 0x200, 0x200, 0x200, 0x000, 0x000, 0x000
};

/*
* Maps G.726_40 code word to ructeconstructed scale factor normalized log
* magnitude values.
*/
static const int g726_40_dqlntab[32] =
{
	-2048, -66, 28, 104, 169, 224, 274, 318,
	358, 395, 429, 459, 488, 514, 539, 566,
	566, 539, 514, 488, 459, 429, 395, 358,
	318, 274, 224, 169, 104, 28, -66, -2048
};

/* Maps G.726_40 code word to log of scale factor multiplier. */
static const int g726_40_witab[32] =
{
	448,   448,   768,  1248,  1280,  1312,  1856,  3200,
	4512,  5728,  7008,  8960, 11456, 14080, 16928, 22272,
	22272, 16928, 14080, 11456,  8960,  7008,  5728,  4512,
	3200,  1856,  1312,  1280,  1248,   768,   448,   448
};

/*
* Maps G.726_40 code words to a set of values whose long and short
* term averages are computed and then compared to give an indication
* how stationary (steady state) the signal is.
*/
static const int g726_40_fitab[32] =
{
	0x000, 0x000, 0x000, 0x000, 0x000, 0x200, 0x200, 0x200,
	0x200, 0x200, 0x400, 0x600, 0x800, 0xA00, 0xC00, 0xC00,
	0xC00, 0xC00, 0xA00, 0x800, 0x600, 0x400, 0x200, 0x200,
	0x200, 0x200, 0x200, 0x000, 0x000, 0x000, 0x000, 0x000
};


g726_state_t *g726_init(g726_state_t *s, int bit_rate);

int g726_decode_frame(g726_state_t *s, short amp[], const unsigned char g726_data[], int g726_bytes);

int g726_encode(g726_state_t *s, unsigned char g726_data[], const short amp[], int len);
unsigned int g726_decode(short* dst, const unsigned char* src, unsigned int srcSize);

#endif

