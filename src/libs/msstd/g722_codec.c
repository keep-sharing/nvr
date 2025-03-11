#include "g722_codec.h"

#if IS_FF
/*************************
c source code from ffmpeg
*************************/

#include <stdio.h>
#include <string.h>
#include "msstd.h"


//decode g722 :  16k sample rate  ; 64K bitrate

static G722Context g722_c = 
{
    .band[0].scale_factor = 8,
    .band[1].scale_factor = 2,
    .prev_samples_pos = 22,
    .bits_per_codeword = 8,// debug
//    .dsp.apply_qmf = g722_apply_qmf;
};

/**
 * quadrature mirror filter (QMF) coefficients
 * ITU-T G.722 Table 11
 */
static const int16_t qmf_coeffs[12] = {
    3, -11, 12, 32, -210, 951, 3876, -805, 362, -156, 53, -11,
};
static const int8_t sign_lookup[2] = { -1, 1 };
static const int16_t inv_log2_table[32] = {
    2048, 2093, 2139, 2186, 2233, 2282, 2332, 2383,
    2435, 2489, 2543, 2599, 2656, 2714, 2774, 2834,
    2896, 2960, 3025, 3091, 3158, 3228, 3298, 3371,
    3444, 3520, 3597, 3676, 3756, 3838, 3922, 4008
};
static const int16_t high_log_factor_step[2] = { 798, -214 };
const int16_t g722_high_inv_quant[4] = { -926, -202, 926, 202 };
/**
 * low_log_factor_step[index] == wl[rl42[index]]
 */
static const int16_t low_log_factor_step[16] = {
     -60, 3042, 1198, 538, 334, 172,  58, -30,
    3042, 1198,  538, 334, 172,  58, -30, -60
};
const int16_t g722_low_inv_quant4[16] = {
       0, -2557, -1612, -1121,  -786,  -530,  -323,  -150,
    2557,  1612,  1121,   786,   530,   323,   150,     0
};
const int16_t g722_low_inv_quant6[64] = {
     -17,   -17,   -17,   -17, -3101, -2738, -2376, -2088,
   -1873, -1689, -1535, -1399, -1279, -1170, -1072,  -982,
    -899,  -822,  -750,  -682,  -618,  -558,  -501,  -447,
    -396,  -347,  -300,  -254,  -211,  -170,  -130,   -91,
    3101,  2738,  2376,  2088,  1873,  1689,  1535,  1399,
    1279,  1170,  1072,   982,   899,   822,   750,   682,
     618,   558,   501,   447,   396,   347,   300,   254,
     211,   170,   130,    91,    54,    17,   -54,   -17
};
static const int16_t low_inv_quant5[32] = {
     -35,   -35, -2919, -2195, -1765, -1458, -1219, -1023,
    -858,  -714,  -587,  -473,  -370,  -276,  -190,  -110,
    2919,  2195,  1765,  1458,  1219,  1023,   858,   714,
     587,   473,   370,   276,   190,   110,    35,   -35
};

static const int16_t * const low_inv_quants[3] = { g722_low_inv_quant6,
                                                           low_inv_quant5,
                                                   g722_low_inv_quant4 };


static __inline__ int16_t saturate15(int32_t amp)
{
    return saturate(amp, INT15_MIN, INT15_MAX);
}

static __inline__ int16_t saturate16(int32_t amp)
{
    return saturate(amp, INT16_MIN, INT16_MAX);
}

static void g722_apply_qmf(const int16_t *prev_samples, int *xout1, int *xout2)
{
    int i;

    *xout1 = 0;
    *xout2 = 0;
    for (i = 0; i < 12; i++) 
    {
        MAC16(*xout2, prev_samples[2*i  ], qmf_coeffs[i   ]);
        MAC16(*xout1, prev_samples[2*i+1], qmf_coeffs[11-i]);
    }
}

static inline void s_zero(int cur_diff, G722Band *band)
{
    int s_zero = 0;

    #define ACCUM(k, x, d) do { \
            int tmp = x; \
            band->zero_mem[k] = ((band->zero_mem[k] * 255) >> 8) + \
               d*((band->diff_mem[k]^cur_diff) < 0 ? -128 : 128); \
            band->diff_mem[k] = tmp; \
            s_zero += (tmp * band->zero_mem[k]) >> 15; \
        } while (0)
    int i = 0;
    int value = (cur_diff == 0 ? 0 : 1);
    for(i = 0; i < 5; i++)
    {
        ACCUM(5-i, band->diff_mem[4-i], value);
    }
    ACCUM(0, cur_diff * 2, 0);

    #undef ACCUM
    band->s_zero = s_zero;
}

/**
 * adaptive predictor
 *
 * @param cur_diff the dequantized and scaled delta calculated from the
 *                 current codeword
 */
static void do_adaptive_prediction(G722Band *band, const int cur_diff)
{
    int sg[2], limit, cur_qtzd_reconst;

    const int cur_part_reconst = band->s_zero + cur_diff < 0;

    sg[0] = sign_lookup[cur_part_reconst != band->part_reconst_mem[0]];
    sg[1] = sign_lookup[cur_part_reconst == band->part_reconst_mem[1]];
    band->part_reconst_mem[1] = band->part_reconst_mem[0];
    band->part_reconst_mem[0] = cur_part_reconst;

    band->pole_mem[1] = saturate((sg[0] * saturate(band->pole_mem[0], -8191, 8191) >> 5) +
                                (sg[1] * 128) + (band->pole_mem[1] * 127 >> 7), -12288, 12288);

    limit = 15360 - band->pole_mem[1];
    band->pole_mem[0] = saturate(-192 * sg[0] + (band->pole_mem[0] * 255 >> 8), -limit, limit);

    s_zero(cur_diff, band);

    cur_qtzd_reconst = saturate16((band->s_predictor + cur_diff) * 2);
    band->s_predictor = saturate16(band->s_zero +
                                      (band->pole_mem[0] * cur_qtzd_reconst >> 15) +
                                      (band->pole_mem[1] * band->prev_qtzd_reconst >> 15));
    band->prev_qtzd_reconst = cur_qtzd_reconst;
}

static inline int linear_scale_factor(const int log_factor)
{
    const int wd1 = inv_log2_table[(log_factor >> 6) & 31];
    const int shift = log_factor >> 11;
    return shift < 0 ? (wd1 >> -shift) : (wd1 << shift);
}

void g722_update_low_predictor(G722Band *band, const int ilow)
{
    do_adaptive_prediction(band, band->scale_factor * g722_low_inv_quant4[ilow] >> 10);

    // quantizer adaptation
    band->log_factor = saturate((band->log_factor * 127 >> 7) +
                                 low_log_factor_step[ilow], 0, 18432);
    band->scale_factor = linear_scale_factor(band->log_factor - (8 << 11));
}

void g722_update_high_predictor(G722Band *band, const int dhigh, const int ihigh)
{
    do_adaptive_prediction(band, dhigh);

    // quantizer adaptation
    band->log_factor   = saturate((band->log_factor * 127 >> 7) +
                                 high_log_factor_step[ihigh&1], 0, 22528);
    band->scale_factor = linear_scale_factor(band->log_factor - (10 << 11));
}

static unsigned int g722_decode_frame(G722Context *c, int16_t *out_buf, const unsigned char*src, unsigned int srcSize)
{
    int j;
    const int skip = 8 - c->bits_per_codeword;
    const int16_t *quantizer_table = low_inv_quants[skip];
    GetBitContext gb;

    init_get_bits(&gb, src, srcSize * 8);

    for (j = 0; j < srcSize; j++)
    {
        int ilow, ihigh, rlow, rhigh, dhigh;
        int xout[2];

        ihigh = get_bits(&gb, 2);
        ilow = get_bits(&gb, 6 - skip);
        skip_bits(&gb, skip);

        rlow = saturate15(((c->band[0].scale_factor * quantizer_table[ilow]) >> 10) + c->band[0].s_predictor);

        g722_update_low_predictor(&c->band[0], (ilow >> (2 - skip)));

        dhigh = c->band[1].scale_factor * g722_high_inv_quant[ihigh] >> 10;
        rhigh = saturate15(dhigh + c->band[1].s_predictor);

        g722_update_high_predictor(&c->band[1], dhigh, ihigh);
        c->prev_samples[c->prev_samples_pos++] = rlow + rhigh;
        c->prev_samples[c->prev_samples_pos++] = rlow - rhigh;
        //c->dsp.apply_qmf(c->prev_samples + c->prev_samples_pos - 24, xout);
        g722_apply_qmf(c->prev_samples + c->prev_samples_pos - 24, &xout[0], &xout[1]);
        *out_buf++ = saturate16(xout[0] >> 11);
        *out_buf++ = saturate16(xout[1] >> 11);
        if (c->prev_samples_pos >= PREV_SAMPLES_BUF_SIZE)
        {
            memmove(c->prev_samples, c->prev_samples + c->prev_samples_pos - 22,
                    22 * sizeof(c->prev_samples[0]));
            c->prev_samples_pos = 22;
        }
    }

    return (srcSize * 4);
}
//int debugg = 0;
unsigned int g722_decode(short* dst, const unsigned char* src, unsigned int srcSize)
{   
//    if(!debugg)printf("#new c  [%d]\n\n", ++debugg);
    return g722_decode_frame(&g722_c, dst, src, srcSize);
}

#else


/*************************
c source code from github
*************************/

#include <stdio.h>
#include <stdlib.h>
#include "msstd.h"
#include <string.h>

//decode only 16k sample rate
#if !defined(FALSE)
#define FALSE 0
#endif
#if !defined(TRUE)
#define TRUE (!FALSE)
#endif

static __inline__ int16_t saturate(int32_t amp)
{
    int16_t amp16;

    /* Hopefully this is optimised for the common case - not clipping */
    amp16 = (int16_t) amp;
    if (amp == amp16)
        return amp16;
    if (amp > INT16_MAX)
        return  INT16_MAX;
    return  INT16_MIN;
}
/*- End of function --------------------------------------------------------*/

static void block4(G722_CTX *s, int band, int d);

static void block4(G722_CTX *s, int band, int d)
{
    int wd1;
    int wd2;
    int wd3;
    int i;

    /* Block 4, RECONS */
    s->band[band].d[0] = d;
    s->band[band].r[0] = saturate(s->band[band].s + d);

    /* Block 4, PARREC */
    s->band[band].p[0] = saturate(s->band[band].sz + d);

    /* Block 4, UPPOL2 */
    for (i = 0;  i < 3;  i++)
        s->band[band].sg[i] = s->band[band].p[i] >> 15;
    wd1 = saturate(s->band[band].a[1] << 2);

    wd2 = (s->band[band].sg[0] == s->band[band].sg[1])  ?  -wd1  :  wd1;
    if (wd2 > 32767)
        wd2 = 32767;
    wd3 = (s->band[band].sg[0] == s->band[band].sg[2])  ?  128  :  -128;
    wd3 += (wd2 >> 7);
    wd3 += (s->band[band].a[2]*32512) >> 15;
    if (wd3 > 12288)
        wd3 = 12288;
    else if (wd3 < -12288)
        wd3 = -12288;
    s->band[band].ap[2] = wd3;

    /* Block 4, UPPOL1 */
    s->band[band].sg[0] = s->band[band].p[0] >> 15;
    s->band[band].sg[1] = s->band[band].p[1] >> 15;
    wd1 = (s->band[band].sg[0] == s->band[band].sg[1])  ?  192  :  -192;
    wd2 = (s->band[band].a[1]*32640) >> 15;

    s->band[band].ap[1] = saturate(wd1 + wd2);
    wd3 = saturate(15360 - s->band[band].ap[2]);
    if (s->band[band].ap[1] > wd3)
        s->band[band].ap[1] = wd3;
    else if (s->band[band].ap[1] < -wd3)
        s->band[band].ap[1] = -wd3;

    /* Block 4, UPZERO */
    wd1 = (d == 0)  ?  0  :  128;
    s->band[band].sg[0] = d >> 15;
    for (i = 1;  i < 7;  i++)
    {
        s->band[band].sg[i] = s->band[band].d[i] >> 15;
        wd2 = (s->band[band].sg[i] == s->band[band].sg[0])  ?  wd1  :  -wd1;
        wd3 = (s->band[band].b[i]*32640) >> 15;
        s->band[band].bp[i] = saturate(wd2 + wd3);
    }

    /* Block 4, DELAYA */
    for (i = 6;  i > 0;  i--)
    {
        s->band[band].d[i] = s->band[band].d[i - 1];
        s->band[band].b[i] = s->band[band].bp[i];
    }

    for (i = 2;  i > 0;  i--)
    {
        s->band[band].r[i] = s->band[band].r[i - 1];
        s->band[band].p[i] = s->band[band].p[i - 1];
        s->band[band].a[i] = s->band[band].ap[i];
    }

    /* Block 4, FILTEP */
    wd1 = saturate(s->band[band].r[1] + s->band[band].r[1]);
    wd1 = (s->band[band].a[1]*wd1) >> 15;
    wd2 = saturate(s->band[band].r[2] + s->band[band].r[2]);
    wd2 = (s->band[band].a[2]*wd2) >> 15;
    s->band[band].sp = saturate(wd1 + wd2);

    /* Block 4, FILTEZ */
    s->band[band].sz = 0;
    for (i = 6;  i > 0;  i--)
    {
        wd1 = saturate(s->band[band].d[i] + s->band[band].d[i]);
        s->band[band].sz += (s->band[band].b[i]*wd1) >> 15;
    }
    s->band[band].sz = saturate(s->band[band].sz);

    /* Block 4, PREDIC */
    s->band[band].s = saturate(s->band[band].sp + s->band[band].sz);
}

// ================decode g722=================

int g722_decoder_destroy(G722_CTX *s)
{
    free(s);
    return 0;
}

static unsigned int g722_decode_frame(G722_CTX *s, const uint8_t g722_data[], int len, int16_t amp[])
{
    static const int wl[8] = {-60, -30, 58, 172, 334, 538, 1198, 3042 };
    static const int rl42[16] = {0, 7, 6, 5, 4, 3, 2, 1, 7, 6, 5, 4, 3,  2, 1, 0 };
    static const int ilb[32] =
    {
        2048, 2093, 2139, 2186, 2233, 2282, 2332,
        2383, 2435, 2489, 2543, 2599, 2656, 2714,
        2774, 2834, 2896, 2960, 3025, 3091, 3158,
        3228, 3298, 3371, 3444, 3520, 3597, 3676,
        3756, 3838, 3922, 4008
    };
    static const int wh[3] = {0, -214, 798};
    static const int rh2[4] = {2, 1, 2, 1};
    static const int qm2[4] = {-7408, -1616,  7408,   1616};
    static const int qm4[16] =
    {
              0, -20456, -12896,  -8968,
          -6288,  -4240,  -2584,  -1200,
          20456,  12896,   8968,   6288,
           4240,   2584,   1200,      0
    };
    static const int qm5[32] =
    {
           -280,   -280, -23352, -17560,
         -14120, -11664,  -9752,  -8184,
          -6864,  -5712,  -4696,  -3784,
          -2960,  -2208,  -1520,   -880,
          23352,  17560,  14120,  11664,
           9752,   8184,   6864,   5712,
           4696,   3784,   2960,   2208,
           1520,    880,    280,   -280
    };
    static const int qm6[64] =
    {
           -136,   -136,   -136,   -136,
         -24808, -21904, -19008, -16704,
         -14984, -13512, -12280, -11192,
         -10232,  -9360,  -8576,  -7856,
          -7192,  -6576,  -6000,  -5456,
          -4944,  -4464,  -4008,  -3576,
          -3168,  -2776,  -2400,  -2032,
          -1688,  -1360,  -1040,   -728,
          24808,  21904,  19008,  16704,
          14984,  13512,  12280,  11192,
          10232,   9360,   8576,   7856,
           7192,   6576,   6000,   5456,
           4944,   4464,   4008,   3576,
           3168,   2776,   2400,   2032,
           1688,   1360,   1040,    728,
            432,    136,   -432,   -136
    };
    static const int qmf_coeffs[12] =
    {
           3,  -11,   12,   32, -210,  951, 3876, -805,  362, -156,   53,  -11,
    };

    int dlowt;
    int rlow;
    int ihigh;
    int dhigh;
    int rhigh;
    int xout1;
    int xout2;
    int wd1;
    int wd2;
    int wd3;
    int code;
    int i;
    int j;
    unsigned int outlen;

    outlen = 0;
    rhigh = 0;
    for (j = 0;  j < len;  )
    {
        code = g722_data[j++];

        switch (s->bits_per_sample)
        {
        default:
        case 8:
            wd1 = code & 0x3F;
            ihigh = (code >> 6) & 0x03;
            wd2 = qm6[wd1];
            wd1 >>= 2;
            break;
        case 7:
            wd1 = code & 0x1F;
            ihigh = (code >> 5) & 0x03;
            wd2 = qm5[wd1];
            wd1 >>= 1;
            break;
        case 6:
            wd1 = code & 0x0F;
            ihigh = (code >> 4) & 0x03;
            wd2 = qm4[wd1];
            break;
        }
        /* Block 5L, LOW BAND INVQBL */
        wd2 = (s->band[0].det*wd2) >> 15;
        /* Block 5L, RECONS */
        rlow = s->band[0].s + wd2;
        /* Block 6L, LIMIT */
        if (rlow > 16383)
            rlow = 16383;
        else if (rlow < -16384)
            rlow = -16384;

        /* Block 2L, INVQAL */
        wd2 = qm4[wd1];
        dlowt = (s->band[0].det*wd2) >> 15;

        /* Block 3L, LOGSCL */
        wd2 = rl42[wd1];
        wd1 = (s->band[0].nb*127) >> 7;
        wd1 += wl[wd2];
        if (wd1 < 0)
            wd1 = 0;
        else if (wd1 > 18432)
            wd1 = 18432;
        s->band[0].nb = wd1;

        /* Block 3L, SCALEL */
        wd1 = (s->band[0].nb >> 6) & 31;
        wd2 = 8 - (s->band[0].nb >> 11);
        wd3 = (wd2 < 0)  ?  (ilb[wd1] << -wd2)  :  (ilb[wd1] >> wd2);
        s->band[0].det = wd3 << 2;

        block4(s, 0, dlowt);

        if (!s->eight_k)
        {
            /* Block 2H, INVQAH */
            wd2 = qm2[ihigh];
            dhigh = (s->band[1].det*wd2) >> 15;
            /* Block 5H, RECONS */
            rhigh = dhigh + s->band[1].s;
            /* Block 6H, LIMIT */
            if (rhigh > 16383)
                rhigh = 16383;
            else if (rhigh < -16384)
                rhigh = -16384;

            /* Block 2H, INVQAH */
            wd2 = rh2[ihigh];
            wd1 = (s->band[1].nb*127) >> 7;
            wd1 += wh[wd2];
            if (wd1 < 0)
                wd1 = 0;
            else if (wd1 > 22528)
                wd1 = 22528;
            s->band[1].nb = wd1;

            /* Block 3H, SCALEH */
            wd1 = (s->band[1].nb >> 6) & 31;
            wd2 = 10 - (s->band[1].nb >> 11);
            wd3 = (wd2 < 0)  ?  (ilb[wd1] << -wd2)  :  (ilb[wd1] >> wd2);
            s->band[1].det = wd3 << 2;

            block4(s, 1, dhigh);
        }

        if (s->itu_test_mode)
        {
            amp[outlen++] = (int16_t) (rlow << 1);
            amp[outlen++] = (int16_t) (rhigh << 1);
        }
        else
        {
            if (s->eight_k)
            {
                amp[outlen++] = (int16_t) (rlow << 1);
            }
            else
            {
                /* Apply the receive QMF */
                for (i = 0;  i < 22;  i++)
                    s->x[i] = s->x[i + 2];
                s->x[22] = rlow + rhigh;
                s->x[23] = rlow - rhigh;

                xout1 = 0;
                xout2 = 0;
                for (i = 0;  i < 12;  i++)
                {
                    xout2 += s->x[2*i]*qmf_coeffs[i];
                    xout1 += s->x[2*i + 1]*qmf_coeffs[11 - i];
                }
                amp[outlen++] = (int16_t) (xout1 >> 11);
                amp[outlen++] = (int16_t) (xout2 >> 11);
            }
        }
    }
    return outlen * 2;
}
//int debugg = 0;
unsigned int g722_decode(short* dst, const unsigned char* src, unsigned int srcSize)
{
//    if(!debugg)printf("#old c [%d]\n\n", ++debugg);
    int i;
    G722_CTX g722_dctx;
    memset(&g722_dctx, 0, sizeof(g722_dctx));
    g722_dctx.bits_per_sample = 8;
    g722_dctx.eight_k = 0;
//    g722_dctx.packed = 0;
    g722_dctx.band[0].det = 32;
    g722_dctx.band[1].det = 8;

    return g722_decode_frame(&g722_dctx, src, srcSize, dst);
}
#endif


