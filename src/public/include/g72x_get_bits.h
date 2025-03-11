#include <stdint.h>

#define GET_BITS

/*    debug------- */
#define UNCHECKED_BITSTREAM_READER  1// !CONFIG_SAFE_BITSTREAM_READER
//#define BITSTREAM_READER_LE  //must be undefined
//#define LONG_BITSTREAM_READER
/* -------debug    */


#define PREV_SAMPLES_BUF_SIZE 1024
#define INT15_MAX (16383)
#define INT15_MIN (-16384)
#define INT_MAX             (2147483647)

#define AV_INPUT_BUFFER_PADDING_SIZE 64
#define MKTAG(a,b,c,d)              ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define ERRTAG(a, b, c, d)          (-(int)MKTAG(a, b, c, d))
#define AVERROR_INVALIDDATA         ERRTAG( 'I','N','D','A')

#ifndef MAX
#   define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#   define MIN(a,b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MUL16
#   define MUL16(ra, rb) ((ra) * (rb))
#endif

#ifndef MAC16
#   define MAC16(rt, ra, rb) rt += (ra) * (rb)
#endif

#ifndef AV_RB32
#   define AV_RB32(x)                                \
        (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
        (((const uint8_t*)(x))[1] << 16) |    \
        (((const uint8_t*)(x))[2] <<  8) |    \
        ((const uint8_t*)(x))[3])
#endif

#ifndef AV_RL32
#   define AV_RL32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[3] << 24) |    \
               (((const uint8_t*)(x))[2] << 16) |    \
               (((const uint8_t*)(x))[1] <<  8) |    \
                ((const uint8_t*)(x))[0])
#endif

#ifndef AV_RL64
#   define AV_RL64(x)                                   \
    (((uint64_t)((const uint8_t*)(x))[7] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[6] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[1] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[0])
#endif

#ifndef AV_RB64
#   define AV_RB64(x)                                   \
    (((uint64_t)((const uint8_t*)(x))[0] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[1] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[6] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[7])
#endif


#define OPEN_READER_NOSIZE(name, gb)            \
        unsigned int name ## _index = (gb)->index;  \
        unsigned int /*av_unused*/ name ## _cache

#if UNCHECKED_BITSTREAM_READER
#   define OPEN_READER(name, gb) OPEN_READER_NOSIZE(name, gb)
#else
#   define OPEN_READER(name, gb)                   \
        OPEN_READER_NOSIZE(name, gb);               \
        unsigned int name ## _size_plus8 = (gb)->size_in_bits_plus8
#endif

#ifdef LONG_BITSTREAM_READER
#   define UPDATE_CACHE_LE(name, gb) name ## _cache = \
        AV_RL64((gb)->buffer + (name ## _index >> 3)) >> (name ## _index & 7)
#   define UPDATE_CACHE_BE(name, gb) name ## _cache = \
        AV_RB64((gb)->buffer + (name ## _index >> 3)) >> (32 - (name ## _index & 7))
#else
#   define UPDATE_CACHE_LE(name, gb) name ## _cache = \
        AV_RL32((gb)->buffer + (name ## _index >> 3)) >> (name ## _index & 7)
#   define UPDATE_CACHE_BE(name, gb) name ## _cache = \
        AV_RB32((gb)->buffer + (name ## _index >> 3)) << (name ## _index & 7)
#endif

#ifdef BITSTREAM_READER_LE
#   define UPDATE_CACHE(name, gb) UPDATE_CACHE_LE(name, gb)
#else
#   define UPDATE_CACHE(name, gb) UPDATE_CACHE_BE(name, gb)
#endif

#define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))

#ifndef zero_extend
#   define zero_extend(val, bits) ((val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits))
#endif

#ifdef BITSTREAM_READER_LE
#   define SHOW_UBITS(name, gb, num) zero_extend(name ## _cache, num)
#else
#   define SHOW_UBITS(name, gb, num) NEG_USR32(name ## _cache, num)
#endif

#if UNCHECKED_BITSTREAM_READER
#   define SKIP_COUNTER(name, gb, num) name ## _index += (num)
#else
#   define SKIP_COUNTER(name, gb, num) \
            name ## _index = MIN(name ## _size_plus8, name ## _index + (num))
#endif

#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)
#define CLOSE_READER(name, gb) (gb)->index = name ## _index

//typedef struct G722DSPContext {
//    void (*apply_qmf)(const int16_t *prev_samples, int xout[2]);
//} G722DSPContext;

typedef struct G722Band
{
    int16_t s_predictor;         ///< predictor output value
    int32_t s_zero;              ///< previous output signal from zero predictor
    int8_t  part_reconst_mem[2]; ///< signs of previous partially reconstructed signals
    int16_t prev_qtzd_reconst;   ///< previous quantized reconstructed signal (internal value, using low_inv_quant4)
    int16_t pole_mem[2];         ///< second-order pole section coefficient buffer
    int32_t diff_mem[6];         ///< quantizer difference signal memory
    int16_t zero_mem[6];         ///< Seventh-order zero section coefficient buffer
    int16_t log_factor;          ///< delayed 2-logarithmic quantizer factor
    int16_t scale_factor;        ///< delayed quantizer scale factor
}G722Band;

typedef struct G722Context
{
    int     bits_per_codeword;
    short   prev_samples[PREV_SAMPLES_BUF_SIZE]; ///< memory of past decoded samples
    int     prev_samples_pos;        ///< the number of values in prev_samples

    G722Band band[2];
    //G722DSPContext dsp;
}G722Context;

typedef struct GetBitContext {
    const uint8_t *buffer, *buffer_end;
    int index;
    int size_in_bits;
    int size_in_bits_plus8;
} GetBitContext;

static __inline__ int16_t saturate(int32_t amp, int32_t min, int32_t max)
{
    if(amp >= max)
        return  max;
    if(amp <= min)
        return  min;
    
    return (int16_t) amp;  
}

static inline unsigned int get_bits(GetBitContext *s, int n)
{
    register int tmp;
    unsigned int re_index = s->index;
    unsigned int re_cache;//OPEN_READER(re, s);
    
    re_cache = AV_RB32(s->buffer + (re_index >> 3)) << (re_index & 7);//UPDATE_CACHE(re, s);
    
    tmp = (((uint32_t)(re_cache))>>(32-n));//SHOW_UBITS(re, s, n);
    
    re_index += n;//LAST_SKIP_BITS(re, s, n);
    
    s->index = re_index;//CLOSE_READER(re, s);
    return tmp;
}

static inline void skip_bits(GetBitContext *s, int n)
{
    unsigned int re_index = s->index;
//    unsigned int re_cache;//OPEN_READER(re, s);
    
    re_index += n;//LAST_SKIP_BITS(re, s, n);
    
    s->index = re_index;//CLOSE_READER(re, s);
}

static inline int init_get_bits(GetBitContext *s, const uint8_t *buffer, int bit_size)
{
#if 1

    s->buffer = buffer;
    s->index = 0;
    return 0;
    
#else

    int buffer_size;
    int ret = 0;

    if (bit_size >= INT32_MAX - MAX(7, AV_INPUT_BUFFER_PADDING_SIZE*8) || bit_size < 0 || !buffer)
    {
        bit_size    = 0;
        buffer      = NULL;
        ret         = AVERROR_INVALIDDATA;
    }

    buffer_size = (bit_size + 7) >> 3;

    s->buffer             = buffer;
    s->size_in_bits       = bit_size;
    s->size_in_bits_plus8 = bit_size + 8;
    s->buffer_end         = buffer + buffer_size;
    s->index              = 0;

    return ret;
    
#endif
}

