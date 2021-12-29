#include <stdio.h>
#include <stdlib.h>

#define SIGN_BIT (0x80)  /* Sign bit for a A-law byte. */
#define QUANT_MASK (0xf) /* Quantization field mask. */
#define NSEGS (8)        /* Number of A-law segments. */
#define SEG_SHIFT (4)    /* Left shift for segment number. */
#define SEG_MASK (0x70)  /* Segment field mask. */

static short qtab_721[7] = { -124, 80, 178, 246, 300, 349, 400 };

static short _dqlntab[16] = { -2048, 4,   135, 213, 273, 323, 373, 425,
                              425,   373, 323, 273, 213, 135, 4,   -2048 };
static short _witab[16] = { -12,  18,  41,  64,  112, 198, 355, 1122,
                            1122, 355, 198, 112, 64,  41,  18,  -12 };

static short _fitab[16] = { 0,     0,     0,     0x200, 0x200, 0x200, 0x600, 0xE00,
                            0xE00, 0x600, 0x200, 0x200, 0x200, 0,     0,     0 };
struct g72x_state {
    long yl;   /* Locked or steady state step size multiplier. */
    short yu;  /* Unlocked or non-steady state step size multiplier. */
    short dms; /* Short term energy estimate. */
    short dml; /* Long term energy estimate. */
    short ap;  /* Linear weighting coefficient of 'yl' and 'yu'. */

    short a[2];  /* Coefficients of pole portion of prediction filter. */
    short b[6];  /* Coefficients of zero portion of prediction filter. */
    short pk[2]; /*
                  * Signs of previous two samples of a partially
                  * reconstructed signal.
                  */
    short dq[6]; /*
                  * Previous 6 samples of the quantized difference
                  * signal represented in an internal floating point
                  * format.
                  */
    short sr[2]; /*
                  * Previous 2 samples of the quantized difference
                  * signal represented in an internal floating point
                  * format.
                  */
    char td;     /* delayed tone detect, new in 1988 version */
}enc_state,dec_state;


static short power2[15] = { 1,     2,     4,     8,     0x10,   0x20,   0x40,  0x80,
                            0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000 };


/* quantizes the input val against the table of size short integers.
 * It returns i if table[i - 1] <= val < table[i].
 *
 * Using linear search for simple coding.
 */
static int quan(int val, short* table, int size)
{
    int i;

    for (i = 0; i < size; i++)
        if (val < *table++)
            break;
    return (i);
}

/* returns the integer product of the 14-bit integer "an" and
 * "floating point" representation (4-bit exponent, 6-bit mantessa) "srn".
 */
static int fmult(int an, int srn)
{
    short anmag, anexp, anmant;
    short wanexp, wanmant;
    short retval;

    anmag = (an > 0) ? an : ((-an) & 0x1FFF);
    anexp = quan(anmag, power2, 15) - 6;
    anmant = (anmag == 0) ? 32 : (anexp >= 0) ? anmag >> anexp : anmag << -anexp;
    wanexp = anexp + ((srn >> 6) & 0xF) - 13;

    wanmant = (anmant * (srn & 077) + 0x30) >> 4;
    retval = (wanexp >= 0) ? ((wanmant << wanexp) & 0x7FFF) : (wanmant >> -wanexp);

    return (((an ^ srn) < 0) ? -retval : retval);
}

/*
 * This routine initializes and/or resets the g72x_state structure
 * pointed to by 'state_ptr'.
 * All the initial state values are specified in the CCITT G.721 document.
 */
void g72x_init_state(struct g72x_state* state_ptr)
{
    int cnta;

    state_ptr->yl = 34816;
    state_ptr->yu = 544;
    state_ptr->dms = 0;
    state_ptr->dml = 0;
    state_ptr->ap = 0;
    for (cnta = 0; cnta < 2; cnta++) {
        state_ptr->a[cnta] = 0;
        state_ptr->pk[cnta] = 0;
        state_ptr->sr[cnta] = 32;
    }
    for (cnta = 0; cnta < 6; cnta++) {
        state_ptr->b[cnta] = 0;
        state_ptr->dq[cnta] = 32;
    }
    state_ptr->td = 0;
}



//computes the estimated signal from 6-zero predictor.
int predictor_zero(struct g72x_state* state_ptr)
{
    int i;
    int sezi;

    sezi = fmult(state_ptr->b[0] >> 2, state_ptr->dq[0]);
    for (i = 1; i < 6; i++) /* ACCUM */
        sezi += fmult(state_ptr->b[i] >> 2, state_ptr->dq[i]);
    return (sezi);
}

//computes the estimated signal from 2-pole predictor.
 int predictor_pole(struct g72x_state* state_ptr)
{
    return (fmult(state_ptr->a[1] >> 2, state_ptr->sr[1]) +
            fmult(state_ptr->a[0] >> 2, state_ptr->sr[0]));
}
/*
 * step_size()
 *
 * computes the quantization step size of the adaptive quantizer.
 *
 */
int step_size(struct g72x_state* state_ptr)
{
    int y;
    int dif;
    int al;

    if (state_ptr->ap >= 256)
        return (state_ptr->yu);
    else {
        y = state_ptr->yl >> 6;
        dif = state_ptr->yu - y;
        al = state_ptr->ap >> 2;
        if (dif > 0)
            y += (dif * al) >> 6;
        else if (dif < 0)
            y += (dif * al + 0x3F) >> 6;
        return (y);
    }
}

/*
 * Given a raw sample, 'd', of the difference signal and a
 * quantization step size scale factor, 'y', this routine returns the
 * ADPCM codeword to which that sample gets quantized.  The step
 * size scale factor division operation is done in the log base 2 domain
 * as a subtraction.
 */
int quantize(int d,        /* Raw difference signal sample */
             int y,        /* Step size multiplier */
             short* table, /* quantization table */
             int size)     /* table size of short integers */
{
    short dqm;  /* Magnitude of 'd' */
    short exp;  /* Integer part of base 2 log of 'd' */
    short mant; /* Fractional part of base 2 log */
    short dl;   /* Log of magnitude of 'd' */
    short dln;  /* Step size scale factor normalized log */
    int i;

    // Compute base 2 log of 'd', and store in 'dl'.
     
    dqm = abs(d);
    exp = quan(dqm >> 1, power2, 15);
    mant = ((dqm << 7) >> exp) & 0x7F; /* Fractional portion. */
    dl = (exp << 7) + mant;

    /*
     * SUBTB
     *
     * "Divide" by step size multiplier.
     */
    dln = dl - (y >> 2);

    /*
     * QUAN
     *
     * Obtain codword i for 'd'.
     */
    i = quan(dln, table, size);
    if (d < 0) /* take 1's complement of i */
        return ((size << 1) + 1 - i);
    else if (i == 0)              /* take 1's complement of 0 */
        return ((size << 1) + 1); /* new in 1988 */
    else
        return (i);
}

 /* Returns reconstructed difference signal 'dq' obtained from
 * codeword 'i' and quantization step size scale factor 'y'.
 * Multiplication is performed in log base 2 domain as addition.
 */
int reconstruct(int sign, /* 0 for non-negative value */
                int dqln, /* G.72x codeword */
                int y)    /* Step size multiplier */
{
    short dql; /* Log of 'dq' magnitude */
    short dex; /* Integer part of log */
    short dqt;
    short dq; /* Reconstructed difference signal sample */

    dql = dqln + (y >> 2); /* ADDA */

    if (dql < 0) {
        return ((sign) ? -0x8000 : 0);
    } else { /* ANTILOG */
        dex = (dql >> 7) & 15;
        dqt = 128 + (dql & 127);
        dq = (dqt << 7) >> (14 - dex);
        return ((sign) ? (dq - 0x8000) : dq);
    }
}


//updates the state variables for each output code
void update(int code_size,                /* distinguish 723_40 with others */
            int y,                        /* quantizer step size */
            int wi,                       /* scale factor multiplier */
            int fi,                       /* for long/short term energies */
            int dq,                       /* quantized prediction difference */
            int sr,                       /* reconstructed signal */
            int dqsez,                    /* difference from 2-pole predictor */
            struct g72x_state* state_ptr) /* coder state pointer */
{
    int cnt;
    short mag, exp; /* Adaptive predictor, FLOAT A */
    short a2p = 0;  /* LIMC */
    short a1ul;     /* UPA1 */
    short pks1;     /* UPA2 */
    short fa1;
    char tr; /* tone/transition detector */
    short ylint, thr2, dqthr;
    short ylfrac, thr1;
    short pk0;

    pk0 = (dqsez < 0) ? 1 : 0; /* needed in updating predictor poles */

    mag = dq & 0x7FFF; /* prediction difference magnitude */
    /* TRANS */
    ylint = state_ptr->yl >> 15;           /* exponent part of yl */
    ylfrac = (state_ptr->yl >> 10) & 0x1F; /* fractional part of yl */
    thr1 = (32 + ylfrac) << ylint;         /* threshold */
    thr2 = (ylint > 9) ? 31 << 10 : thr1;  /* limit thr2 to 31 << 10 */
    dqthr = (thr2 + (thr2 >> 1)) >> 1;     /* dqthr = 0.75 * thr2 */
    if (state_ptr->td == 0)                /* signal supposed voice */
        tr = 0;
    else if (mag <= dqthr) /* supposed data, but small mag */
        tr = 0;            /* treated as voice */
    else                   /* signal is data (modem) */
        tr = 1;

    /*
     * Quantizer scale factor adaptation.
     */

    /* FUNCTW & FILTD & DELAY */
    /* update non-steady state step size multiplier */
    state_ptr->yu = y + ((wi - y) >> 5);

    /* LIMB */
    if (state_ptr->yu < 544) /* 544 <= yu <= 5120 */
        state_ptr->yu = 544;
    else if (state_ptr->yu > 5120)
        state_ptr->yu = 5120;

    /* FILTE & DELAY */
    /* update steady state step size multiplier */
    state_ptr->yl += state_ptr->yu + ((-state_ptr->yl) >> 6);

    /*
     * Adaptive predictor coefficients.
     */
    if (tr == 1) { /* reset a's and b's for modem signal */
        state_ptr->a[0] = 0;
        state_ptr->a[1] = 0;
        state_ptr->b[0] = 0;
        state_ptr->b[1] = 0;
        state_ptr->b[2] = 0;
        state_ptr->b[3] = 0;
        state_ptr->b[4] = 0;
        state_ptr->b[5] = 0;
    } else {                           /* update a's and b's */
        pks1 = pk0 ^ state_ptr->pk[0]; /* UPA2 */

        /* update predictor pole a[1] */
        a2p = state_ptr->a[1] - (state_ptr->a[1] >> 7);
        if (dqsez != 0) {
            fa1 = (pks1) ? state_ptr->a[0] : -state_ptr->a[0];
            if (fa1 < -8191) /* a2p = function of fa1 */
                a2p -= 0x100;
            else if (fa1 > 8191)
                a2p += 0xFF;
            else
                a2p += fa1 >> 5;

            if (pk0 ^ state_ptr->pk[1])
                /* LIMC */
                if (a2p <= -12160)
                    a2p = -12288;
                else if (a2p >= 12416)
                    a2p = 12288;
                else
                    a2p -= 0x80;
            else if (a2p <= -12416)
                a2p = -12288;
            else if (a2p >= 12160)
                a2p = 12288;
            else
                a2p += 0x80;
        }

        /* TRIGB & DELAY */
        state_ptr->a[1] = a2p;

        /* UPA1 */
        /* update predictor pole a[0] */
        state_ptr->a[0] -= state_ptr->a[0] >> 8;
        if (dqsez != 0) {
            if (pks1 == 0)
                state_ptr->a[0] += 192;
            else
                state_ptr->a[0] -= 192;
        }

        /* LIMD */
        a1ul = 15360 - a2p;
        if (state_ptr->a[0] < -a1ul)
            state_ptr->a[0] = -a1ul;
        else if (state_ptr->a[0] > a1ul)
            state_ptr->a[0] = a1ul;

        /* UPB : update predictor zeros b[6] */
        for (cnt = 0; cnt < 6; cnt++) {
            if (code_size == 5) /* for 40Kbps G.723 */
                state_ptr->b[cnt] -= state_ptr->b[cnt] >> 9;
            else /* for G.721 and 24Kbps G.723 */
                state_ptr->b[cnt] -= state_ptr->b[cnt] >> 8;
            if (dq & 0x7FFF) { /* XOR */
                if ((dq ^ state_ptr->dq[cnt]) >= 0)
                    state_ptr->b[cnt] += 128;
                else
                    state_ptr->b[cnt] -= 128;
            }
        }
    }

    for (cnt = 5; cnt > 0; cnt--)
        state_ptr->dq[cnt] = state_ptr->dq[cnt - 1];
    /* FLOAT A : convert dq[0] to 4-bit exp, 6-bit mantissa f.p. */
    if (mag == 0) {
        state_ptr->dq[0] = (dq >= 0) ? 0x20 : 0xFC20;
    } else {
        exp = quan(mag, power2, 15);
        state_ptr->dq[0] = (dq >= 0) ? (exp << 6) + ((mag << 6) >> exp)
                                     : (exp << 6) + ((mag << 6) >> exp) - 0x400;
    }

    state_ptr->sr[1] = state_ptr->sr[0];
    /* FLOAT B : convert sr to 4-bit exp., 6-bit mantissa f.p. */
    if (sr == 0) {
        state_ptr->sr[0] = 0x20;
    } else if (sr > 0) {
        exp = quan(sr, power2, 15);
        state_ptr->sr[0] = (exp << 6) + ((sr << 6) >> exp);
    } else if (sr > -32768) {
        mag = -sr;
        exp = quan(mag, power2, 15);
        state_ptr->sr[0] = (exp << 6) + ((mag << 6) >> exp) - 0x400;
    } else
        state_ptr->sr[0] = 0xFC20;

    /* DELAY A */
    state_ptr->pk[1] = state_ptr->pk[0];
    state_ptr->pk[0] = pk0;

    /* TONE */
    if (tr == 1)           /* this sample has been treated as data */
        state_ptr->td = 0; /* next one will be treated as voice */
    else if (a2p < -11776) /* small sample-to-sample correlation */
        state_ptr->td = 1; /* signal may be data */
    else                   /* signal is voice */
        state_ptr->td = 0;

    /*
     * Adaptation speed control.
     */
    state_ptr->dms += (fi - state_ptr->dms) >> 5;          /* FILTA */
    state_ptr->dml += (((fi << 2) - state_ptr->dml) >> 7); /* FILTB */

    if (tr == 1)
        state_ptr->ap = 256;
    else if (y < 1536) /* SUBTC */
        state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
    else if (state_ptr->td == 1)
        state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
    else if (abs((state_ptr->dms << 2) - state_ptr->dml) >= (state_ptr->dml >> 3))
        state_ptr->ap += (0x200 - state_ptr->ap) >> 4;
    else
        state_ptr->ap += (-state_ptr->ap) >> 4;
}

int g721_encoder(int sl, struct g72x_state* state_ptr)
{
    short sezi, se, sez; /* ACCUM */
    short d;             /* SUBTA */
    short sr;            /* ADDB */
    short y;             /* MIX */
    short dqsez;         /* ADDC */
    short dq, i;

    sl >>= 2; /* 14-bit dynamic range */

    sezi = predictor_zero(state_ptr);
    sez = sezi >> 1;
    se = (sezi + predictor_pole(state_ptr)) >> 1; /* estimated signal */

    d = sl - se; /* estimation difference */

    /* quantize the prediction difference */
    y = step_size(state_ptr);        /* quantizer step size */
    i = quantize(d, y, qtab_721, 7); /* i = ADPCM code */

    dq = reconstruct(i & 8, _dqlntab[i], y); /* quantized est diff */

    sr = (dq < 0) ? se - (dq & 0x3FFF) : se + dq; /* reconst. signal */

    dqsez = sr + sez - se; /* pole prediction diff. */

    update(4, y, _witab[i] << 5, _fitab[i], dq, sr, dqsez, state_ptr);

    return (i);
}

int g721_decoder(int i, struct g72x_state* state_ptr)
{
    short sezi, sei, sez, se; /* ACCUM */
    short y;                  /* MIX */
    short sr;                 /* ADDB */
    short dq;
    short dqsez;

    i &= 0x0f; /* mask to get proper bits */
    sezi = predictor_zero(state_ptr);
    sez = sezi >> 1;
    sei = sezi + predictor_pole(state_ptr);
    se = sei >> 1; /* se = estimated signal */

    y = step_size(state_ptr); /* dynamic quantizer step size */

    dq = reconstruct(i & 0x08, _dqlntab[i], y); /* quantized diff. */

    sr = (dq < 0) ? (se - (dq & 0x3FFF)) : se + dq; /* reconst. signal */

    dqsez = sr - se + sez; /* pole prediction diff. */

    update(4, y, _witab[i] << 5, _fitab[i], dq, sr, dqsez, state_ptr);

    return (sr << 2); /* sr was 14-bit dynamic range */
}

static short seg_end[8] = { 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF };

static int search(int val, short* table, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (val <= *table++)
            return (i);
    }
    return (size);
}

unsigned char linear2alaw(int pcm_val) /* 2's complement (16-bit range) */
{
    int mask;
    int seg;
    unsigned char aval;

    if (pcm_val >= 0) {
        mask = 0xD5; /* sign (7th) bit = 1 */
    } else {
        mask = 0x55; /* sign bit = 0 */
        pcm_val = -pcm_val - 8;
    }

    /* Convert the scaled magnitude to segment number. */
    seg = search(pcm_val, seg_end, 8);
    /* Combine the sign, segment, and quantization bits. */
    if (seg >= 8) /* out of range, return maximum value. */
        return (0x7F ^ mask);
    else {
        aval = seg << SEG_SHIFT;
        if (seg < 2)
            aval |= (pcm_val >> 4) & QUANT_MASK;
        else
            aval |= (pcm_val >> (seg + 3)) & QUANT_MASK;
        return (aval ^ mask);
    }
}

int alaw2linear(unsigned char a_val)
{
    int t;
    int seg;

    a_val ^= 0x55;

    t = (a_val & QUANT_MASK) << 4;
    seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
    switch (seg) {
    case 0:
        t += 8;
        break;
    case 1:
        t += 0x108;
        break;
    default:
        t += 0x108;
        t <<= seg - 1;
    }
    return ((a_val & SIGN_BIT) ? t : -t);
}

