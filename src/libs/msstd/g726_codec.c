
#include "g726_codec.h"

#if IS_FFG726

#include <stdio.h>
#include <string.h>

/**
 * G.726 11-bit float.
 * G.726 Standard uses rather odd 11-bit floating point arithmetic for
 * numerous occasions. It's a mystery to me why they did it this way
 * instead of simply using 32-bit integer arithmetic.
 */
static int is_g726_open = 0;

static const int quant_tbl16[] =                  /**< 16kbit/s 2 bits per sample */
           { 260, INT_MAX };
static const int16_t iquant_tbl16[] =
           { 116, 365, 365, 116 };
static const int16_t W_tbl16[] =
           { -22, 439, 439, -22 };
static const uint8_t F_tbl16[] =
           { 0, 7, 7, 0 };

static const int quant_tbl24[] =                  /**< 24kbit/s 3 bits per sample */
           {  7, 217, 330, INT_MAX };
static const int16_t iquant_tbl24[] =
           { INT16_MIN, 135, 273, 373, 373, 273, 135, INT16_MIN };
static const int16_t W_tbl24[] =
           { -4,  30, 137, 582, 582, 137,  30, -4 };
static const uint8_t F_tbl24[] =
           { 0, 1, 2, 7, 7, 2, 1, 0 };

static const int quant_tbl32[] =                  /**< 32kbit/s 4 bits per sample */
           { -125,  79, 177, 245, 299, 348, 399, INT_MAX };
static const int16_t iquant_tbl32[] =
         { INT16_MIN,   4, 135, 213, 273, 323, 373, 425,
                 425, 373, 323, 273, 213, 135,   4, INT16_MIN };
static const int16_t W_tbl32[] =
           { -12,  18,  41,  64, 112, 198, 355, 1122,
            1122, 355, 198, 112,  64,  41,  18, -12};
static const uint8_t F_tbl32[] =
           { 0, 0, 0, 1, 1, 1, 3, 7, 7, 3, 1, 1, 1, 0, 0, 0 };

static const int quant_tbl40[] =                  /**< 40kbit/s 5 bits per sample */
           { -122, -16,  67, 138, 197, 249, 297, 338,
              377, 412, 444, 474, 501, 527, 552, INT_MAX };
static const int16_t iquant_tbl40[] =
         { INT16_MIN, -66,  28, 104, 169, 224, 274, 318,
                 358, 395, 429, 459, 488, 514, 539, 566,
                 566, 539, 514, 488, 459, 429, 395, 358,
                 318, 274, 224, 169, 104,  28, -66, INT16_MIN };
static const int16_t W_tbl40[] =
           {   14,  14,  24,  39,  40,  41,   58,  100,
              141, 179, 219, 280, 358, 440,  529,  696,
              696, 529, 440, 358, 280, 219,  179,  141,
              100,  58,  41,  40,  39,  24,   14,   14 };
static const uint8_t F_tbl40[] =
           { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 6,
             6, 6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

static const G726Tables G726Tables_pool[] =
           {{ quant_tbl16, iquant_tbl16, W_tbl16, F_tbl16 },
            { quant_tbl24, iquant_tbl24, W_tbl24, F_tbl24 },
            { quant_tbl32, iquant_tbl32, W_tbl32, F_tbl32 },
            { quant_tbl40, iquant_tbl40, W_tbl40, F_tbl40 }};


static G726Context g726_c = 
{
    .little_endian = 0,// !strcmp(avctx->codec->name, "g726le");
    .code_size = 2,//avctx->bits_per_coded_sample;
    .yu = 544,
    .yl = 34816,
    .y = 544,
    
    .sr[0].mant = 1<<5,
    .sr[1].mant = 1<<5,
    .pk[0] = 1,
    .pk[1] = 1,
    
    .dq[0].mant = 1<<5,
    .dq[1].mant = 1<<5,
    .dq[2].mant = 1<<5,
    .dq[3].mant = 1<<5,
    .dq[4].mant = 1<<5,
    .dq[5].mant = 1<<5,
};


/**
 * Paragraph 4.2.3 page 22: Inverse adaptive quantizer.
 */
static inline int16_t inverse_quant(G726Context* c, int i)
{
    int dql, dex, dqt;

    dql = c->tbls.iquant[i] + (c->y >> 2);
    dex = (dql>>7) & 0xf;        /* 4-bit exponent */
    dqt = (1<<7) + (dql & 0x7f); /* log2 -> linear */
    return (dql < 0) ? 0 : ((dqt<<dex) >> 7);
}

static int16_t decode_g726(G726Context* c, int I)
{
    int dq, re_signal = 0, pk0, fa1, i, tr, ylint, ylfrac, thr2, al, dq0;
    Float11 f;

    int I_sig= I >> (c->code_size - 1);

    dq = inverse_quant(c, I);

    /* Transition detect */
    ylint = (c->yl >> 15);
    ylfrac = (c->yl >> 10) & 0x1f;
    thr2 = (ylint > 9) ? 0x1f << 10 : (0x20 + ylfrac) << ylint;
    tr= (c->td == 1 && dq > ((3*thr2)>>2));

    if (I_sig)  /* get the sign */
    {
        dq = -dq;
    }
    re_signal = (int16_t)(c->se + dq);

    /* Update second order predictor coefficient A2 and A1 */
    pk0 = (c->sez + dq) ? sgn(c->sez + dq) : 0;
    dq0 = dq ? sgn(dq) : 0;
    if (tr)
    {
        c->a[0] = 0;
        c->a[1] = 0;
        for (i=0; i<6; i++)
        {
            c->b[i] = 0;
        }
    }
    else
    {
        /* This is a bit crazy, but it really is +255 not +256 */
        fa1 = saturate9((-c->a[0]*c->pk[0]*pk0)>>5);

        c->a[1] += 128*pk0*c->pk[1] + fa1 - (c->a[1]>>7);
        c->a[1] = saturate(c->a[1], -12288, 12288);
        c->a[0] += 64*3*pk0*c->pk[0] - (c->a[0] >> 8);
        c->a[0] = saturate(c->a[0], -(15360 - c->a[1]), 15360 - c->a[1]);

        for (i=0; i<6; i++)
        {
            c->b[i] += 128*dq0*sgn(-c->dq[i].sign) - (c->b[i]>>8);
        }
    }
    /* Update Dq and Sr and Pk */
    c->pk[1] = c->pk[0];
    c->pk[0] = pk0 ? pk0 : 1;
    c->sr[1] = c->sr[0];
    
    i2f(re_signal, &c->sr[0]);
    for (i=5; i>0; i--)
    {
        c->dq[i] = c->dq[i-1];
    }
    i2f(dq, &c->dq[0]);
    c->dq[0].sign = I_sig; /* Isn't it crazy ?!?! */

    c->td = c->a[1] < -11776;

    /* Update Ap */
    c->dms += (c->tbls.F[I]<<4) + ((- c->dms) >> 5);
    c->dml += (c->tbls.F[I]<<4) + ((- c->dml) >> 7);
    if (tr)
    {
        c->ap = 256;
    }
    else
    {
        c->ap += (-c->ap) >> 4;
        if (c->y <= 1535 || c->td || ABS((c->dms << 2) - c->dml) >= (c->dml >> 3))
            c->ap += 0x20;
    }

    /* Update Yu and Yl */
    c->yu = saturate(c->y + c->tbls.W[I] + ((-c->y)>>5), 544, 5120);
    c->yl += c->yu + ((-c->yl)>>6);

    /* Next iteration for Y */
    al = (c->ap >= 256) ? 1<<6 : c->ap >> 2;
    c->y = (c->yl + (c->yu - (c->yl>>6))*al) >> 6;

    /* Next iteration for SE and SEZ */
    c->se = 0;
    for (i=0; i<6; i++)
    {
        c->se += mult(i2f(c->b[i] >> 2, &f), &c->dq[i]);
    }
    c->sez = c->se >> 1;
    for (i=0; i<2; i++)
    {
        c->se += mult(i2f(c->a[i] >> 2, &f), &c->sr[i]);
    }
    c->se >>= 1;
    return saturate(re_signal * 4, -0xffff, 0xffff);
}


static int g726_decode_frame(G726Context *c, int16_t *out_buf, const unsigned char*src, unsigned int srcSize)
{
    GetBitContext gb;
    int out_samples;
    unsigned int out_len;
    out_len = srcSize * 8;
    out_samples = out_len / c->code_size;
//    init_get_bits(&gb, src, out_len);
    gb.buffer = src;
    gb.index = 0;
    while (out_samples--)
    {
        *out_buf++ = decode_g726(c, get_bits(&gb, c->code_size));
    }
    return out_len;
}

unsigned int g726_decode(short* dst, const unsigned char* src, unsigned int srcSize)
{
    if(!is_g726_open)
    {
        g726_c.tbls = G726Tables_pool[g726_c.code_size - 2];//c->code_size - 2
        is_g726_open++;
//        printf("#new g726 c        [%d]\n\n", is_g726_open);
    }
    return g726_decode_frame(&g726_c, dst, src, srcSize);
}


#else

/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static const int qtab_726_16[1] =
{
    261
};

static const int qtab_726_24[3] =
{
    8, 218, 331
};

static const int qtab_726_32[7] =
{
    -124, 80, 178, 246, 300, 349, 400
};

static const int qtab_726_40[15] =
{
    -122, -16,  68, 139, 198, 250, 298, 339,
     378, 413, 445, 475, 502, 528, 553
};


static __inline int top_bit(unsigned int bits)
{
#if defined(__i386__)  ||  defined(__x86_64__)
	int res;

	__asm__ (" xorl %[res],%[res];\n"
		" decl %[res];\n"
		" bsrl %[bits],%[res]\n"
		: [res] "=&r" (res)
		: [bits] "rm" (bits));
	return res;
#elif defined(__ppc__)  ||   defined(__powerpc__)
	int res;

	__asm__ ("cntlzw %[res],%[bits];\n"
		: [res] "=&r" (res)
		: [bits] "r" (bits));
	return 31 - res;
#elif defined(_M_IX86) // Visual Studio x86
	__asm
	{
		xor eax, eax
			dec eax
			bsr eax, bits
	}
#else
	int res;

	if (bits == 0)
		return -1;
	res = 0;
	if (bits & 0xFFFF0000)
	{
		bits &= 0xFFFF0000;
		res += 16;
	}
	if (bits & 0xFF00FF00)
	{
		bits &= 0xFF00FF00;
		res += 8;
	}
	if (bits & 0xF0F0F0F0)
	{
		bits &= 0xF0F0F0F0;
		res += 4;
	}
	if (bits & 0xCCCCCCCC)
	{
		bits &= 0xCCCCCCCC;
		res += 2;
	}
	if (bits & 0xAAAAAAAA)
	{
		bits &= 0xAAAAAAAA;
		res += 1;
	}
	return res;
#endif
}


static bitstream_state_t *bitstream_init(bitstream_state_t *s)
{
	if (s == NULL)
		return NULL;
	s->bitstream = 0;
	s->residue = 0;
	return s;
}

/*
 * Given a raw sample, 'd', of the difference signal and a
 * quantization step size scale factor, 'y', this routine returns the
 * ADPCM codeword to which that sample gets quantized.  The step
 * size scale factor division operation is done in the log base 2 domain
 * as a subtraction.
 */
static short quantize(int d,                  /* Raw difference signal sample */
                        int y,                  /* Step size multiplier */
                        const int table[],     /* quantization table */
                        int quantizer_states)   /* table size of short integers */
{
    short dqm;    /* Magnitude of 'd' */
    short exp;    /* Integer part of base 2 log of 'd' */
    short mant;   /* Fractional part of base 2 log */
    short dl;     /* Log of magnitude of 'd' */
    short dln;    /* Step size scale factor normalized log */
    int i;
    int size;

    /*
     * LOG
     *
     * Compute base 2 log of 'd', and store in 'dl'.
     */
    dqm = (short) abs(d);
    exp = (short) (top_bit(dqm >> 1) + 1);
    /* Fractional portion. */
    mant = ((dqm << 7) >> exp) & 0x7F;
    dl = (exp << 7) + mant;

    /*
     * SUBTB
     *
     * "Divide" by step size multiplier.
     */
    dln = dl - (short) (y >> 2);

    /*
     * QUAN
     *
     * Search for codword i for 'dln'.
     */
    size = (quantizer_states - 1) >> 1;
    for (i = 0;  i < size;  i++)
    {
        if (dln < table[i])
            break;
    }
    if (d < 0)
    {
        /* Take 1's complement of i */
        return (short) ((size << 1) + 1 - i);
    }
    if (i == 0  &&  (quantizer_states & 1))
    {
        /* Zero is only valid if there are an even number of states, so
           take the 1's complement if the code is zero. */
        return (short) quantizer_states;
    }
    return (short) i;
}
/*- End of function --------------------------------------------------------*/


/*
* returns the integer product of the 14-bit integer "an" and
* "floating point" representation (4-bit exponent, 6-bit mantessa) "srn".
*/
static short fmult(short an, short srn)
{
	short anmag;
	short anexp;
	short anmant;
	short wanexp;
	short wanmant;
	short retval;

	anmag = (an > 0)  ?  an  :  ((-an) & 0x1FFF);
	anexp = (short) (top_bit(anmag) - 5);
	anmant = (anmag == 0)  ?  32  :  (anexp >= 0)  ?  (anmag >> anexp)  :  (anmag << -anexp);
	wanexp = anexp + ((srn >> 6) & 0xF) - 13;

	wanmant = (anmant*(srn & 0x3F) + 0x30) >> 4;
	retval = (wanexp >= 0)  ?  ((wanmant << wanexp) & 0x7FFF)  :  (wanmant >> -wanexp);

	return (((an ^ srn) < 0)  ?  -retval  :  retval);
}

/*
* Compute the estimated signal from the 6-zero predictor.
*/
static __inline short predictor_zero(g726_state_t *s)
{
	int i;
	int sezi;

	sezi = fmult(s->b[0] >> 2, s->dq[0]);
	/* ACCUM */
	for (i = 1;  i < 6;  i++)
		sezi += fmult(s->b[i] >> 2, s->dq[i]);
	return (short) sezi;
}
/*- End of function --------------------------------------------------------*/

/*
* Computes the estimated signal from the 2-pole predictor.
*/
static __inline short predictor_pole(g726_state_t *s)
{
	return (fmult(s->a[1] >> 2, s->sr[1]) + fmult(s->a[0] >> 2, s->sr[0]));
}

/*
* Computes the quantization step size of the adaptive quantizer.
*/
static int step_size(g726_state_t *s)
{
	int y;
	int dif;
	int al;

	if (s->ap >= 256)
		return s->yu;
	y = s->yl >> 6;
	dif = s->yu - y;
	al = s->ap >> 2;
	if (dif > 0)
		y += (dif*al) >> 6;
	else if (dif < 0)
		y += (dif*al + 0x3F) >> 6;
	return y;
}
/*- End of function --------------------------------------------------------*/

/*
* Returns reconstructed difference signal 'dq' obtained from
* codeword 'i' and quantization step size scale factor 'y'.
* Multiplication is performed in log base 2 domain as addition.
*/
static short reconstruct(int sign,    /* 0 for non-negative value */
						 int dqln,    /* G.72x codeword */
						 int y)       /* Step size multiplier */
{
	short dql;    /* Log of 'dq' magnitude */
	short dex;    /* Integer part of log */
	short dqt;
	short dq;     /* Reconstructed difference signal sample */

	dql = (short) (dqln + (y >> 2));  /* ADDA */

	if (dql < 0)
		return ((sign)  ?  -0x8000  :  0);
	/* ANTILOG */
	dex = (dql >> 7) & 15;
	dqt = 128 + (dql & 127);
	dq = (dqt << 7) >> (14 - dex);
	return ((sign)  ?  (dq - 0x8000)  :  dq);
}
/*- End of function --------------------------------------------------------*/

/*
* updates the state variables for each output code
*/
static void update(g726_state_t *s,
				   int y,       /* quantizer step size */
				   int wi,      /* scale factor multiplier */
				   int fi,      /* for long/short term energies */
				   int dq,      /* quantized prediction difference */
				   int sr,      /* reconstructed signal */
				   int dqsez)   /* difference from 2-pole predictor */
{
	short mag;
	short exp;
	short a2p;        /* LIMC */
	short a1ul;       /* UPA1 */
	short pks1;       /* UPA2 */
	short fa1;
	short ylint;
	short dqthr;
	short ylfrac;
	short thr;
	short pk0;
	int i;
	int tr;

	a2p = 0;
	/* Needed in updating predictor poles */
	pk0 = (dqsez < 0)  ?  1  :  0;

	/* prediction difference magnitude */
	mag = (short) (dq & 0x7FFF);
	/* TRANS */
	ylint = (short) (s->yl >> 15);            /* exponent part of yl */
	ylfrac = (short) ((s->yl >> 10) & 0x1F);  /* fractional part of yl */
	/* Limit threshold to 31 << 10 */
	thr = (ylint > 9)  ?  (31 << 10)  :  ((32 + ylfrac) << ylint);
	dqthr = (thr + (thr >> 1)) >> 1;            /* dqthr = 0.75 * thr */
	if (!s->td)                                 /* signal supposed voice */
		tr = 0;
	else if (mag <= dqthr)                      /* supposed data, but small mag */
		tr = 0;                             /* treated as voice */
	else                                        /* signal is data (modem) */
		tr = 1;

	/*
	* Quantizer scale factor adaptation.
	*/

	/* FUNCTW & FILTD & DELAY */
	/* update non-steady state step size multiplier */
	s->yu = (short) (y + ((wi - y) >> 5));

	/* LIMB */
	if (s->yu < 544)
		s->yu = 544;
	else if (s->yu > 5120)
		s->yu = 5120;

	/* FILTE & DELAY */
	/* update steady state step size multiplier */
	s->yl += s->yu + ((-s->yl) >> 6);

	/*
	* Adaptive predictor coefficients.
	*/
	if (tr)
	{
		/* Reset the a's and b's for a modem signal */
		s->a[0] = 0;
		s->a[1] = 0;
		s->b[0] = 0;
		s->b[1] = 0;
		s->b[2] = 0;
		s->b[3] = 0;
		s->b[4] = 0;
		s->b[5] = 0;
	}
	else
	{
		/* Update the a's and b's */
		/* UPA2 */
		pks1 = pk0 ^ s->pk[0];

		/* Update predictor pole a[1] */
		a2p = s->a[1] - (s->a[1] >> 7);
		if (dqsez != 0)
		{
			fa1 = (pks1)  ?  s->a[0]  :  -s->a[0];
			/* a2p = function of fa1 */
			if (fa1 < -8191)
				a2p -= 0x100;
			else if (fa1 > 8191)
				a2p += 0xFF;
			else
				a2p += fa1 >> 5;

			if (pk0 ^ s->pk[1])
			{
				/* LIMC */
				if (a2p <= -12160)
					a2p = -12288;
				else if (a2p >= 12416)
					a2p = 12288;
				else
					a2p -= 0x80;
			}
			else if (a2p <= -12416)
				a2p = -12288;
			else if (a2p >= 12160)
				a2p = 12288;
			else
				a2p += 0x80;
		}

		/* TRIGB & DELAY */
		s->a[1] = a2p;

		/* UPA1 */
		/* Update predictor pole a[0] */
		s->a[0] -= s->a[0] >> 8;
		if (dqsez != 0)
		{
			if (pks1 == 0)
				s->a[0] += 192;
			else
				s->a[0] -= 192;
		}
		/* LIMD */
		a1ul = 15360 - a2p;
		if (s->a[0] < -a1ul)
			s->a[0] = -a1ul;
		else if (s->a[0] > a1ul)
			s->a[0] = a1ul;

		/* UPB : update predictor zeros b[6] */
		for (i = 0;  i < 6;  i++)
		{
			/* Distinguish 40Kbps mode from the others */
			s->b[i] -= s->b[i] >> ((s->bits_per_sample == 5)  ?  9  :  8);
			if (dq & 0x7FFF)
			{
				/* XOR */
				if ((dq ^ s->dq[i]) >= 0)
					s->b[i] += 128;
				else
					s->b[i] -= 128;
			}
		}
	}

	for (i = 5;  i > 0;  i--)
		s->dq[i] = s->dq[i - 1];
	/* FLOAT A : convert dq[0] to 4-bit exp, 6-bit mantissa f.p. */
	if (mag == 0)
	{
		s->dq[0] = (dq >= 0)  ?  0x20  :  0xFC20;
	}
	else
	{
		exp = (short) (top_bit(mag) + 1);
		s->dq[0] = (dq >= 0)
			?  ((exp << 6) + ((mag << 6) >> exp))
			:  ((exp << 6) + ((mag << 6) >> exp) - 0x400);
	}

	s->sr[1] = s->sr[0];
	/* FLOAT B : convert sr to 4-bit exp., 6-bit mantissa f.p. */
	if (sr == 0)
	{
		s->sr[0] = 0x20;
	}
	else if (sr > 0)
	{
		exp = (short) (top_bit(sr) + 1);
		s->sr[0] = (short) ((exp << 6) + ((sr << 6) >> exp));
	}
	else if (sr > -32768)
	{
		mag = (short) -sr;
		exp = (short) (top_bit(mag) + 1);
		s->sr[0] =  (exp << 6) + ((mag << 6) >> exp) - 0x400;
	}
	else
	{
		s->sr[0] = (short) 0xFC20;
	}

	/* DELAY A */
	s->pk[1] = s->pk[0];
	s->pk[0] = pk0;

	/* TONE */
	if (tr)                 /* this sample has been treated as data */
		s->td = 0;      /* next one will be treated as voice */
	else if (a2p < -11776)  /* small sample-to-sample correlation */
		s->td = 1;       /* signal may be data */
	else                    /* signal is voice */
		s->td = 0;

	/* Adaptation speed control. */
	/* FILTA */
	s->dms += ((short) fi - s->dms) >> 5;
	/* FILTB */
	s->dml += (((short) (fi << 2) - s->dml) >> 7);

	if (tr)
		s->ap = 256;
	else if (y < 1536)                      /* SUBTC */
		s->ap += (0x200 - s->ap) >> 4;
	else if (s->td)
		s->ap += (0x200 - s->ap) >> 4;
	else if (abs((s->dms << 2) - s->dml) >= (s->dml >> 3))
		s->ap += (0x200 - s->ap) >> 4;
	else
		s->ap += (-s->ap) >> 4;
}

/*
* Decodes a 2-bit CCITT G.726_16 ADPCM code and returns
* the resulting 16-bit linear PCM, A-law or u-law sample value.
*/
static short g726_16_decoder(g726_state_t *s, unsigned char code)
{
	short sezi;
	short sei;
	short se;
	short sr;
	short dq;
	short dqsez;
	int y;

	/* Mask to get proper bits */
	code &= 0x03;
	sezi = predictor_zero(s);
	sei = sezi + predictor_pole(s);

	y = step_size(s);
	dq = reconstruct(code & 2, g726_16_dqlntab[code], y);

	/* Reconstruct the signal */
	se = sei >> 1;
	sr = (dq < 0)  ?  (se - (dq & 0x3FFF))  :  (se + dq);

	/* Pole prediction difference */
	dqsez = sr + (sezi >> 1) - se;

	update(s, y, g726_16_witab[code], g726_16_fitab[code], dq, sr, dqsez);

	return (sr << 2);
}
/*- End of function --------------------------------------------------------*/


/*
 * Encodes a linear PCM, A-law or u-law input sample and returns its 3-bit code.
 */
static unsigned char g726_16_encoder(g726_state_t *s, short amp)
{
    int y;
    short sei;
    short sezi;
    short se;
    short d;
    short sr;
    short dqsez;
    short dq;
    short i;
    
    sezi = predictor_zero(s);
    sei = sezi + predictor_pole(s);
    se = sei >> 1;
    d = amp - se;

    /* Quantize prediction difference */
    y = step_size(s);
    i = quantize(d, y, qtab_726_16, 4);
    dq = reconstruct(i & 2, g726_16_dqlntab[i], y);

    /* Reconstruct the signal */
    sr = (dq < 0)  ?  (se - (dq & 0x3FFF))  :  (se + dq);

    /* Pole prediction difference */
    dqsez = sr + (sezi >> 1) - se;
    
    update(s, y, g726_16_witab[i], g726_16_fitab[i], dq, sr, dqsez);
    return (unsigned char) i;
}

/*
* Decodes a 3-bit CCITT G.726_24 ADPCM code and returns
* the resulting 16-bit linear PCM, A-law or u-law sample value.
*/
static short g726_24_decoder(g726_state_t *s, unsigned char code)
{
	short sezi;
	short sei;
	short se;
	short sr;
	short dq;
	short dqsez;
	int y;

	/* Mask to get proper bits */
	code &= 0x07;
	sezi = predictor_zero(s);
	sei = sezi + predictor_pole(s);

	y = step_size(s);
	dq = reconstruct(code & 4, g726_24_dqlntab[code], y);

	/* Reconstruct the signal */
	se = sei >> 1;
	sr = (dq < 0)  ?  (se - (dq & 0x3FFF))  :  (se + dq);

	/* Pole prediction difference */
	dqsez = sr + (sezi >> 1) - se;

	update(s, y, g726_24_witab[code], g726_24_fitab[code], dq, sr, dqsez);

	return (sr << 2);
}
/*- End of function --------------------------------------------------------*/


/*
 * Encodes a linear PCM, A-law or u-law input sample and returns its 3-bit code.
 */
static unsigned char g726_24_encoder(g726_state_t *s, short amp)
{
    short sei;
    short sezi;
    short se;
    short d;
    short sr;
    short dqsez;
    short dq;
    short i;
    int y;
    
    sezi = predictor_zero(s);
    sei = sezi + predictor_pole(s);
    se = sei >> 1;
    d = amp - se;

    /* Quantize prediction difference */
    y = step_size(s);
    i = quantize(d, y, qtab_726_24, 7);
    dq = reconstruct(i & 4, g726_24_dqlntab[i], y);

    /* Reconstruct the signal */
    sr = (dq < 0)  ?  (se - (dq & 0x3FFF))  :  (se + dq);

    /* Pole prediction difference */
    dqsez = sr + (sezi >> 1) - se;
    
    update(s, y, g726_24_witab[i], g726_24_fitab[i], dq, sr, dqsez);
    return (unsigned char) i;
}


/*
* Decodes a 4-bit CCITT G.726_32 ADPCM code and returns
* the resulting 16-bit linear PCM, A-law or u-law sample value.
*/
static short g726_32_decoder(g726_state_t *s, unsigned char code)
{
	short sezi;
	short sei;
	short se;
	short sr;
	short dq;
	short dqsez;
	int y;

	/* Mask to get proper bits */
	code &= 0x0F;
	sezi = predictor_zero(s);
	sei = sezi + predictor_pole(s);

	y = step_size(s);
	dq = reconstruct(code & 8, g726_32_dqlntab[code], y);

	/* Reconstruct the signal */
	se = sei >> 1;
	sr = (dq < 0)  ?  (se - (dq & 0x3FFF))  :  (se + dq);

	/* Pole prediction difference */
	dqsez = sr + (sezi >> 1) - se;

	update(s, y, g726_32_witab[code], g726_32_fitab[code], dq, sr, dqsez);

	return (sr << 2);
}
/*- End of function --------------------------------------------------------*/

/*
 * Encodes a linear input sample and returns its 4-bit code.
 */
static unsigned char g726_32_encoder(g726_state_t *s, short amp)
{
    short sei;
    short sezi;
    short se;
    short d;
    short sr;
    short dqsez;
    short dq;
    short i;
    int y;
    
    sezi = predictor_zero(s);
    sei = sezi + predictor_pole(s);
    se = sei >> 1;
    d = amp - se;

    /* Quantize the prediction difference */
    y = step_size(s);
    i = quantize(d, y, qtab_726_32, 15);
    dq = reconstruct(i & 8, g726_32_dqlntab[i], y);

    /* Reconstruct the signal */
    sr = (dq < 0)  ?  (se - (dq & 0x3FFF))  :  (se + dq);

    /* Pole prediction difference */
    dqsez = sr + (sezi >> 1) - se;

    update(s, y, g726_32_witab[i], g726_32_fitab[i], dq, sr, dqsez);
    return (unsigned char) i;
}

/*
* Decodes a 5-bit CCITT G.726 40Kbps code and returns
* the resulting 16-bit linear PCM, A-law or u-law sample value.
*/
static short g726_40_decoder(g726_state_t *s, unsigned char code)
{
	short sezi;
	short sei;
	short se;
	short sr;
	short dq;
	short dqsez;
	int y;

	/* Mask to get proper bits */
	code &= 0x1F;
	sezi = predictor_zero(s);
	sei = sezi + predictor_pole(s);

	y = step_size(s);
	dq = reconstruct(code & 0x10, g726_40_dqlntab[code], y);

	/* Reconstruct the signal */
	se = sei >> 1;
	sr = (dq < 0)  ?  (se - (dq & 0x7FFF))  :  (se + dq);

	/* Pole prediction difference */
	dqsez = sr + (sezi >> 1) - se;

	update(s, y, g726_40_witab[code], g726_40_fitab[code], dq, sr, dqsez);

	return (sr << 2);
}
/*- End of function --------------------------------------------------------*/


/*
 * Encodes a 16-bit linear PCM, A-law or u-law input sample and retuens
 * the resulting 5-bit CCITT G.726 40Kbps code.
 */
static unsigned char g726_40_encoder(g726_state_t *s, short amp)
{
    short sei;
    short sezi;
    short se;
    short d;
    short sr;
    short dqsez;
    short dq;
    short i;
    int y;
    
    sezi = predictor_zero(s);
    sei = sezi + predictor_pole(s);
    se = sei >> 1;
    d = amp - se;

    /* Quantize prediction difference */
    y = step_size(s);
    i = quantize(d, y, qtab_726_40, 31);
    dq = reconstruct(i & 0x10, g726_40_dqlntab[i], y);

    /* Reconstruct the signal */
    sr = (dq < 0)  ?  (se - (dq & 0x7FFF))  :  (se + dq);

    /* Pole prediction difference */
    dqsez = sr + (sezi >> 1) - se;

    update(s, y, g726_40_witab[i], g726_40_fitab[i], dq, sr, dqsez);
    return (unsigned char) i;
}

g726_state_t *g726_init(g726_state_t *s, int bit_rate)
{
	int i;

	if (bit_rate != 16000  &&  bit_rate != 24000  &&  bit_rate != 32000  &&  bit_rate != 40000)
		return NULL;

	s->yl = 34816;
	s->yu = 544;
	s->dms = 0;
	s->dml = 0;
	s->ap = 0;
	s->rate = bit_rate;

	for (i = 0; i < 2; i++)
	{
		s->a[i] = 0;
		s->pk[i] = 0;
		s->sr[i] = 32;
	}
	for (i = 0; i < 6; i++)
	{
		s->b[i] = 0;
		s->dq[i] = 32;
	}
	s->td = 0;
	switch (bit_rate)
	{
	case 16000:
		s->enc_func = g726_16_encoder;
		s->dec_func = g726_16_decoder;
		s->bits_per_sample = 2;
		break;
	case 24000:
		s->enc_func = g726_24_encoder;
		s->dec_func = g726_24_decoder;
		s->bits_per_sample = 3;
		break;
	case 32000:
	default:
		s->enc_func = g726_32_encoder;
		s->dec_func = g726_32_decoder;
		s->bits_per_sample = 4;
		break;
	case 40000:
		s->enc_func = g726_40_encoder;
		s->dec_func = g726_40_decoder;
		s->bits_per_sample = 5;
		break;
	}
	bitstream_init(&s->bs);
	return s;
}

int g726_decode_frame(g726_state_t *s,
				short amp[],
				const unsigned char g726_data[],
				int g726_bytes)
{
	int i;
	int samples;
	unsigned char code;
	int sl;

	for (samples = i = 0;  ;  )
	{
		if (s->bs.residue < s->bits_per_sample)
		{
			if (i >= g726_bytes)
				break;
			s->bs.bitstream = (s->bs.bitstream << 8) | g726_data[i++];
			s->bs.residue += 8;
		}
		code = (unsigned char) ((s->bs.bitstream >> (s->bs.residue - s->bits_per_sample)) & ((1 << s->bits_per_sample) - 1));

		s->bs.residue -= s->bits_per_sample;

		sl = s->dec_func(s, code);

		amp[samples++] = (short) sl;
	}
	return samples;
}


int g726_encode(g726_state_t *s,
                unsigned char g726_data[],
                const short amp[],
                int len)
{
    int i;
    int g726_bytes;
    short sl;
    unsigned char code;

    for (g726_bytes = i = 0;  i < len;  i++)
    {
		sl = amp[i] >> 2;

        code = s->enc_func(s, sl);

		s->bs.bitstream = (s->bs.bitstream << s->bits_per_sample) | code;
		s->bs.residue += s->bits_per_sample;
		if (s->bs.residue >= 8)
		{
			g726_data[g726_bytes++] = (unsigned char) ((s->bs.bitstream >> (s->bs.residue - 8)) & 0xFF);
			s->bs.residue -= 8;
		}
		
    }
	
    return g726_bytes;
}

//int debugg = 0;
unsigned int g726_decode(short* dst, const unsigned char* src, unsigned int srcSize)
{
//    if(!debugg)printf("#old g726 c [%d]\n\n", ++debugg);
    g726_state_t *g726;
    g726_state_t s;
    g726 = g726_init(&s, 16000);
    g726_decode_frame(g726, dst, src, srcSize);
    return 8 * srcSize;
}
#endif

