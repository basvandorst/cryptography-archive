/*
 * This program is copyright (c) Alec Muffett 1991 except for certain
 * portions of code ("crack-fcrypt.c") copyright (c) Robert Baldwin, Icarus
 * Sparry and Alec Muffett.  The author(s) disclaims all responsibility or
 * liability with respect to it's usage or its effect upon hardware or
 * computer systems.  This software is in the public domain and is freely
 * redistributable PROVIDED that this notice remains intact.
 */

/*
 * Misc defs for the fast password transform optimisations.
 */

#include "crack.h"             /* contains switches - AEM */

/*
 * Rename the types for greater convenience ? - This is from original code.
 */
#define        reg     register
#define        uns     unsigned
#define unsb   uns char
#define        unsl    uns long

/*
 * Types for the different ways to represent DES bit patterns.  Bits are
 * always right justified within fields.  Bits which have lower indices in
 * the NBS spec are stored in the vax bits with less significance (e.g., Bit
 * 1 of NBS spec is stored in the bit with weight 2 ** 0 to the Vax.
 */

#define        obpb1   unsb            /* One bit per byte. */
#define sbpb6  unsb            /* Six bits per byte, 6 held. */
#define sbpb6R unsb            /* Six bits per byte Reversed order, 6 held. */
#define        sbpb24  unsl            /* Six bits per byte, 24 held. */
#define        ebpb24  unsl            /* Eight bits per bit, 24 held. */
#define        fbpb4   unsb            /* Four bits per byte, 4 held. */
#define        fbpb4R  unsb            /* Four bits per byte Reversed order, 4 held. */

/*
 * The operation (6 * x) is often better optimised as this (for really
 * braindead compilers) - AEM
 */

#ifdef BRAINDEAD6
#define SIX_TIMES(exprn)               (((exprn) << 2) + ((exprn) << 1))
#else
#define SIX_TIMES(exprn)               (6 * (exprn))
#endif                         /* BRAINDEAD6 */

/*
 * Data segment gathered into one place - AEM
 */

/* Try to keep this stuff long aligned - AEM */
static char iobuf[16];
static obpb1 L[32], R[32];
static obpb1 crypt_block[72];  /* 72 is next multiple of 8 bytes after 66 */
static sbpb24 KS[32];
static sbpb24 S0H[64], S1H[64], S2H[64], S3H[64];
static sbpb24 S4H[64], S5H[64], S6H[64], S7H[64];
static sbpb24 S0L[64], S1L[64], S2L[64], S3L[64];
static sbpb24 S4L[64], S5L[64], S6L[64], S7L[64];
static sbpb24 out96[4];

/*
 * These used to be rather slow and frequently used functions - AEM
 */

#define TF_TO_SIXBIT(tf) \
       (sbpb24)((tf & 077L) | \
               ((tf & 07700L) << 2) | \
               ((tf & 0770000L) << 4) | \
               ((tf & 077000000L) << 6))

#define SIXBIT_TO_TF(sb) \
       (ebpb24)((sb & 0x3fL) | \
               ((sb & 0x3f00L) >> 2) | \
               ((sb & 0x3f0000L) >> 4) | \
               ((sb & 0x3f000000L) >> 6))
/*
 * Start of the real thing
 */

void
toBA64 (quarters /* , crypt_block */ )
    reg sbpb24 *quarters;
{
    static unsb UnDoE[] =
    {
       1, 2, 3, 4, 7, 8, 9, 10,
       13, 14, 15, 16, 19, 20, 21, 22,
       25, 26, 27, 28, 31, 32, 33, 34,
       37, 38, 39, 40, 43, 44, 45, 46
    };
    static unsb FP[] =
    {
       39, 7, 47, 15, 55, 23, 63, 31,
       38, 6, 46, 14, 54, 22, 62, 30,
       37, 5, 45, 13, 53, 21, 61, 29,
       36, 4, 44, 12, 52, 20, 60, 28,
       35, 3, 43, 11, 51, 19, 59, 27,
       34, 2, 42, 10, 50, 18, 58, 26,
       33, 1, 41, 9, 49, 17, 57, 25,
       32, 0, 40, 8, 48, 16, 56, 24,
    };

    reg i;
    static obpb1 tmpE[48];
    reg unsb *onebits48;
    reg sbpb24 quarter;

    onebits48 = tmpE;
    quarter = SIXBIT_TO_TF (*quarters);
    quarters++;

    /*
     * Testing one bit and setting another may be faster than shifting and
     * setting - it's certainly not slower - AEM
     */
    *onebits48++ = (quarter & 0x000001L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000002L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000004L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000008L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000010L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000020L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000040L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000080L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000100L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000200L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000400L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000800L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x001000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x002000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x004000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x008000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x010000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x020000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x040000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x080000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x100000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x200000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x400000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x800000L) ? 0x01 : 0;

    quarter = SIXBIT_TO_TF (*quarters);
    quarters++;

    *onebits48++ = (quarter & 0x000001L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000002L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000004L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000008L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000010L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000020L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000040L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000080L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000100L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000200L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000400L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000800L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x001000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x002000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x004000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x008000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x010000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x020000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x040000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x080000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x100000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x200000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x400000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x800000L) ? 0x01 : 0;

    /* the next loop used to be the call: undoe (tmpE, L); - AEM */

    for (i = 0; i < 32; i++)
    {
       L[i] = tmpE[UnDoE[i]];
    }

    onebits48 = tmpE;
    quarter = SIXBIT_TO_TF (*quarters);
    quarters++;

    *onebits48++ = (quarter & 0x000001L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000002L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000004L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000008L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000010L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000020L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000040L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000080L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000100L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000200L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000400L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000800L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x001000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x002000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x004000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x008000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x010000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x020000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x040000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x080000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x100000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x200000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x400000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x800000L) ? 0x01 : 0;

    quarter = SIXBIT_TO_TF (*quarters);
    quarters++;

    *onebits48++ = (quarter & 0x000001L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000002L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000004L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000008L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000010L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000020L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000040L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000080L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000100L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000200L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000400L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x000800L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x001000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x002000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x004000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x008000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x010000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x020000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x040000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x080000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x100000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x200000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x400000L) ? 0x01 : 0;
    *onebits48++ = (quarter & 0x800000L) ? 0x01 : 0;

    /* the next loop used to be the call: undoe (tmpE, R); - AEM */

    for (i = 0; i < 32; i++)
    {
       R[i] = tmpE[UnDoE[i]];
    }

    /* the next loop used to be the call: Fperm (crypt_block); - AEM */

    for (i = 0; i < 64; i++)
    {
       crypt_block[i] = L[FP[i]];
    }
}

void
fsetkey ()
{
    /*
     * This used to be utterly horrendous. It still is, but it's much, much,
     * smaller... AEM.
     */
    static unsb KeyToKS[] =
    {
       9, 50, 33, 59, 48, 16, 32, 56, 1, 8, 18, 41, 2, 34, 25, 24,
       43, 57, 58, 0, 35, 26, 17, 40, 21, 27, 38, 53, 36, 3, 46, 29,
       4, 52, 22, 28, 60, 20, 37, 62, 14, 19, 44, 13, 12, 61, 54, 30,
       1, 42, 25, 51, 40, 8, 24, 48, 58, 0, 10, 33, 59, 26, 17, 16,
       35, 49, 50, 57, 56, 18, 9, 32, 13, 19, 30, 45, 28, 62, 38, 21,
       27, 44, 14, 20, 52, 12, 29, 54, 6, 11, 36, 5, 4, 53, 46, 22,
       50, 26, 9, 35, 24, 57, 8, 32, 42, 49, 59, 17, 43, 10, 1, 0,
       48, 33, 34, 41, 40, 2, 58, 16, 60, 3, 14, 29, 12, 46, 22, 5,
       11, 28, 61, 4, 36, 27, 13, 38, 53, 62, 20, 52, 19, 37, 30, 6,
       34, 10, 58, 48, 8, 41, 57, 16, 26, 33, 43, 1, 56, 59, 50, 49,
       32, 17, 18, 25, 24, 51, 42, 0, 44, 54, 61, 13, 27, 30, 6, 52,
       62, 12, 45, 19, 20, 11, 60, 22, 37, 46, 4, 36, 3, 21, 14, 53,
       18, 59, 42, 32, 57, 25, 41, 0, 10, 17, 56, 50, 40, 43, 34, 33,
       16, 1, 2, 9, 8, 35, 26, 49, 28, 38, 45, 60, 11, 14, 53, 36,
       46, 27, 29, 3, 4, 62, 44, 6, 21, 30, 19, 20, 54, 5, 61, 37,
       2, 43, 26, 16, 41, 9, 25, 49, 59, 1, 40, 34, 24, 56, 18, 17,
       0, 50, 51, 58, 57, 48, 10, 33, 12, 22, 29, 44, 62, 61, 37, 20,
       30, 11, 13, 54, 19, 46, 28, 53, 5, 14, 3, 4, 38, 52, 45, 21,
       51, 56, 10, 0, 25, 58, 9, 33, 43, 50, 24, 18, 8, 40, 2, 1,
       49, 34, 35, 42, 41, 32, 59, 17, 27, 6, 13, 28, 46, 45, 21, 4,
       14, 62, 60, 38, 3, 30, 12, 37, 52, 61, 54, 19, 22, 36, 29, 5,
       35, 40, 59, 49, 9, 42, 58, 17, 56, 34, 8, 2, 57, 24, 51, 50,
       33, 18, 48, 26, 25, 16, 43, 1, 11, 53, 60, 12, 30, 29, 5, 19,
       61, 46, 44, 22, 54, 14, 27, 21, 36, 45, 38, 3, 6, 20, 13, 52,
       56, 32, 51, 41, 1, 34, 50, 9, 48, 26, 0, 59, 49, 16, 43, 42,
       25, 10, 40, 18, 17, 8, 35, 58, 3, 45, 52, 4, 22, 21, 60, 11,
       53, 38, 36, 14, 46, 6, 19, 13, 28, 37, 30, 62, 61, 12, 5, 44,
       40, 16, 35, 25, 50, 18, 34, 58, 32, 10, 49, 43, 33, 0, 56, 26,
       9, 59, 24, 2, 1, 57, 48, 42, 54, 29, 36, 19, 6, 5, 44, 62,
       37, 22, 20, 61, 30, 53, 3, 60, 12, 21, 14, 46, 45, 27, 52, 28,
       24, 0, 48, 9, 34, 2, 18, 42, 16, 59, 33, 56, 17, 49, 40, 10,
       58, 43, 8, 51, 50, 41, 32, 26, 38, 13, 20, 3, 53, 52, 28, 46,
       21, 6, 4, 45, 14, 37, 54, 44, 27, 5, 61, 30, 29, 11, 36, 12,
       8, 49, 32, 58, 18, 51, 2, 26, 0, 43, 17, 40, 1, 33, 24, 59,
       42, 56, 57, 35, 34, 25, 16, 10, 22, 60, 4, 54, 37, 36, 12, 30,
       5, 53, 19, 29, 61, 21, 38, 28, 11, 52, 45, 14, 13, 62, 20, 27,
       57, 33, 16, 42, 2, 35, 51, 10, 49, 56, 1, 24, 50, 17, 8, 43,
       26, 40, 41, 48, 18, 9, 0, 59, 6, 44, 19, 38, 21, 20, 27, 14,
       52, 37, 3, 13, 45, 5, 22, 12, 62, 36, 29, 61, 60, 46, 4, 11,
       41, 17, 0, 26, 51, 48, 35, 59, 33, 40, 50, 8, 34, 1, 57, 56,
       10, 24, 25, 32, 2, 58, 49, 43, 53, 28, 3, 22, 5, 4, 11, 61,
       36, 21, 54, 60, 29, 52, 6, 27, 46, 20, 13, 45, 44, 30, 19, 62,
       25, 1, 49, 10, 35, 32, 48, 43, 17, 24, 34, 57, 18, 50, 41, 40,
       59, 8, 9, 16, 51, 42, 33, 56, 37, 12, 54, 6, 52, 19, 62, 45,
       20, 5, 38, 44, 13, 36, 53, 11, 30, 4, 60, 29, 28, 14, 3, 46,
       17, 58, 41, 2, 56, 24, 40, 35, 9, 16, 26, 49, 10, 42, 33, 32,
       51, 0, 1, 8, 43, 34, 25, 48, 29, 4, 46, 61, 44, 11, 54, 37,
       12, 60, 30, 36, 5, 28, 45, 3, 22, 27, 52, 21, 20, 6, 62, 38
    };

    reg int i, j, r;
    reg unsb *k;

    k = KeyToKS;

    for (i = 0; i < 32; i++)   /* loops cache better ? - AEM */
    {
       r = 0;
       for (j = 0; j < 24; j++)
       {
           r |= crypt_block[*(k++)] << j;
       }
       KS[i] = TF_TO_SIXBIT (r);
    }
}

void
xform (quarters, saltvalue)
    sbpb24 *quarters;
    sbpb24 saltvalue;
{
    union
    {
       sbpb24 b[2];
       sbpb6 c[8];
    } sdata;

#ifdef sun                     /* Icarus Sparry, Bath */
#define STEP --
#define START &sdata.c[7]
#define Dl sdata.b[1]
#define Dh sdata.b[0]
#else
#define STEP ++
#define START &sdata.c[0]
#define Dl sdata.b[0]
#define Dh sdata.b[1]
#endif
    sbpb24 Rl, Rh;
    sbpb24 Ll, Lh;

    reg int loop;
    reg sbpb24 k;
    reg sbpb6 *dp;
    reg sbpb24 *kp;
    reg sbpb24 *kend;

    Ll = Lh = Rl = Rh = 0;

    kend = &KS[32];

    /*
     * Thanks to Matt Bishop for this idea... AEM.
     */

#ifndef FDES_4BYTE
#define SIZEFIX                0
#define INDIRECT(a,b)  (a)[b]
#else
#define SIZEFIX                2       /* "n" where 2^n == sizeof(sbpb24) */
#define INDIRECT(a,b)  (*((sbpb24 *)(((unsigned char *) a) + (b))))
#endif

    for (loop = 25; loop-- > 0; /* nothing */ )
    {
       for (kp = KS; kp < kend; /* nothing */ )
       {
           k = (Rl ^ Rh) & saltvalue;
           Dl = (k ^ Rl ^ *kp++) << SIZEFIX;
           Dh = (k ^ Rh ^ *kp++) << SIZEFIX;

           /*
            * Oddly enough, direct addressing of dp slows things down, as
            * well as knackering portability - AEM
            */
           dp = START;
           Lh ^= INDIRECT (S0H, *dp STEP);
           Lh ^= INDIRECT (S1H, *dp STEP);
           Lh ^= INDIRECT (S2H, *dp STEP);
           Lh ^= INDIRECT (S3H, *dp STEP);
           Lh ^= INDIRECT (S4H, *dp STEP);
           Lh ^= INDIRECT (S5H, *dp STEP);
           Lh ^= INDIRECT (S6H, *dp STEP);
           Lh ^= INDIRECT (S7H, *dp STEP);

           dp = START;
           Ll ^= INDIRECT (S0L, *dp STEP);
           Ll ^= INDIRECT (S1L, *dp STEP);
           Ll ^= INDIRECT (S2L, *dp STEP);
           Ll ^= INDIRECT (S3L, *dp STEP);
           Ll ^= INDIRECT (S4L, *dp STEP);
           Ll ^= INDIRECT (S5L, *dp STEP);
           Ll ^= INDIRECT (S6L, *dp STEP);
           Ll ^= INDIRECT (S7L, *dp STEP);

           k = (Ll ^ Lh) & saltvalue;
           Dl = (k ^ Ll ^ *kp++) << SIZEFIX;
           Dh = (k ^ Lh ^ *kp++) << SIZEFIX;

           dp = START;
           Rh ^= INDIRECT (S0H, *dp STEP);
           Rh ^= INDIRECT (S1H, *dp STEP);
           Rh ^= INDIRECT (S2H, *dp STEP);
           Rh ^= INDIRECT (S3H, *dp STEP);
           Rh ^= INDIRECT (S4H, *dp STEP);
           Rh ^= INDIRECT (S5H, *dp STEP);
           Rh ^= INDIRECT (S6H, *dp STEP);
           Rh ^= INDIRECT (S7H, *dp STEP);

           dp = START;
           Rl ^= INDIRECT (S0L, *dp STEP);
           Rl ^= INDIRECT (S1L, *dp STEP);
           Rl ^= INDIRECT (S2L, *dp STEP);
           Rl ^= INDIRECT (S3L, *dp STEP);
           Rl ^= INDIRECT (S4L, *dp STEP);
           Rl ^= INDIRECT (S5L, *dp STEP);
           Rl ^= INDIRECT (S6L, *dp STEP);
           Rl ^= INDIRECT (S7L, *dp STEP);
       }

       Ll ^= Rl;
       Lh ^= Rh;
       Rl ^= Ll;
       Rh ^= Lh;
       Ll ^= Rl;
       Lh ^= Rh;
    }

    {
       reg sbpb24 *qp;
       qp = quarters;
       *qp++ = Ll;
       *qp++ = Lh;
       *qp++ = Rl;
       *qp++ = Rh;
    }
    return;
}

char *
fcrypt (pw, salt)
    char *pw;
    char *salt;
{
    /* Table lookups for salts reduce fcrypt() overhead dramatically */
    static sbpb24 salt0[] =
    {
       18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
       34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
       50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 1,
       2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 5, 6, 7, 8, 9, 10,
       11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
       27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 32, 33, 34, 35, 36,
       37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
       53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 1, 2, 3, 4,
       5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
       21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
       37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
       53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 1, 2, 3, 4,
       5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
       21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
       37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
       53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 1, 2, 3, 4
    };
    static sbpb24 salt1[] =
    {
       1152, 1216, 1280, 1344, 1408, 1472, 1536, 1600,
       1664, 1728, 1792, 1856, 1920, 1984, 2048, 2112,
       2176, 2240, 2304, 2368, 2432, 2496, 2560, 2624,
       2688, 2752, 2816, 2880, 2944, 3008, 3072, 3136,
       3200, 3264, 3328, 3392, 3456, 3520, 3584, 3648,
       3712, 3776, 3840, 3904, 3968, 4032, 0, 64,
       128, 192, 256, 320, 384, 448, 512, 576,
       640, 704, 320, 384, 448, 512, 576, 640,
       704, 768, 832, 896, 960, 1024, 1088, 1152,
       1216, 1280, 1344, 1408, 1472, 1536, 1600, 1664,
       1728, 1792, 1856, 1920, 1984, 2048, 2112, 2176,
       2240, 2304, 2368, 2048, 2112, 2176, 2240, 2304,
       2368, 2432, 2496, 2560, 2624, 2688, 2752, 2816,
       2880, 2944, 3008, 3072, 3136, 3200, 3264, 3328,
       3392, 3456, 3520, 3584, 3648, 3712, 3776, 3840,
       3904, 3968, 4032, 0, 64, 128, 192, 256,
       320, 384, 448, 512, 576, 640, 704, 768,
       832, 896, 960, 1024, 1088, 1152, 1216, 1280,
       1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792,
       1856, 1920, 1984, 2048, 2112, 2176, 2240, 2304,
       2368, 2432, 2496, 2560, 2624, 2688, 2752, 2816,
       2880, 2944, 3008, 3072, 3136, 3200, 3264, 3328,
       3392, 3456, 3520, 3584, 3648, 3712, 3776, 3840,
       3904, 3968, 4032, 0, 64, 128, 192, 256,
       320, 384, 448, 512, 576, 640, 704, 768,
       832, 896, 960, 1024, 1088, 1152, 1216, 1280,
       1344, 1408, 1472, 1536, 1600, 1664, 1728, 1792,
       1856, 1920, 1984, 2048, 2112, 2176, 2240, 2304,
       2368, 2432, 2496, 2560, 2624, 2688, 2752, 2816,
       2880, 2944, 3008, 3072, 3136, 3200, 3264, 3328,
       3392, 3456, 3520, 3584, 3648, 3712, 3776, 3840,
       3904, 3968, 4032, 0, 64, 128, 192, 256
    };

    /* final perutation desalting */
    static obpb1 final[] =
    {
       46, 47, 48, 49, 50, 51, 52, 53,
       54, 55, 56, 57, 65, 66, 67, 68,
       69, 70, 71, 72, 73, 74, 75, 76,
       77, 78, 79, 80, 81, 82, 83, 84,
       85, 86, 87, 88, 89, 90, 97, 98,
       99, 100, 101, 102, 103, 104, 105, 106,
       107, 108, 109, 110, 111, 112, 113, 114,
       115, 116, 117, 118, 119, 120, 121, 122,
       123, 124, 125, 126, 127, 128, 129, 130,
       131, 132, 133, 134, 135, 136, 137, 138,
       139, 140, 141, 142, 143, 144, 145, 146,
       147, 148, 149, 150, 151, 152, 153, 154,
       155, 156, 157, 158, 159, 160, 161, 162,
       163, 164, 165, 166, 167, 168, 169, 170,
       171, 172, 173, 174, 175, 176, 177, 178,
       179, 180, 181, 182, 183, 184, 185, 186,
       187, 188, 189, 190, 191, 192, 193, 194,
       195, 196, 197, 198, 199, 200, 201, 202,
       203, 204, 205, 206, 207, 208, 209, 210,
       211, 212, 213, 214, 215, 216, 217, 218,
       219, 220, 221, 222, 223, 224, 225, 226,
       227, 228, 229, 230, 231, 232, 233, 234,
       235, 236, 237, 238, 239, 240, 241, 242,
       243, 244, 245, 246, 247, 248, 249, 250,
       251, 252, 253, 254, 255,
       /* Truncate overflow bits at 256 */
       0, 1, 2, 3, 4, 5, 6, 7,
       8, 9, 10, 11, 12, 13, 14, 15,
       16, 17, 18, 19, 20, 21, 22, 23,
       24, 25, 26, 27, 28, 29, 30, 31,
       32, 33, 34, 35, 36, 37, 38, 39,
       40, 41, 42, 43, 44, 45, 46, 47,
       48, 49, 50, 51, 52, 53, 54, 55,
       56, 57, 58
    };

    reg int i, j, k;
    reg long int *lip;
    sbpb24 saltvalue;

#if defined(BUILTIN_CLEAR)
    lip = (long int *) crypt_block;
    for (i = (sizeof (crypt_block) / sizeof (long int)); i > 0; i--)
    {
       *(lip++) = 0L;
    }
#elif defined(BZERO)
    bzero (crypt_block, 66);
#else
    for (i = 0; i < 66; i++)
    {
       crypt_block[i] = '\0';
    }
#endif

    for (i = 0; (k = *pw) && i < 64; pw++)
    {
       crypt_block[i++] = (k >> 6) & 01;
       crypt_block[i++] = (k >> 5) & 01;
       crypt_block[i++] = (k >> 4) & 01;
       crypt_block[i++] = (k >> 3) & 01;
       crypt_block[i++] = (k >> 2) & 01;
       crypt_block[i++] = (k >> 1) & 01;
       crypt_block[i++] = (k >> 0) & 01;
       i++;                    /* have to skip one here (parity bit) */
    }

    fsetkey ( /* crypt_block */ );


#if defined(BUILTIN_CLEAR)
    lip = (long int *) crypt_block;
    for (i = (sizeof (crypt_block) / sizeof (long int)); i > 0; i--)
    {
       *(lip++) = 0L;
    }
#elif defined(BZERO)
    bzero (crypt_block, 66);
#else
    for (i = 0; i < 66; i++)
    {
       crypt_block[i] = '\0';
    }
#endif

    iobuf[0] = salt[0];
    iobuf[1] = salt[1];

    saltvalue = salt0[iobuf[0]] | salt1[iobuf[1]];
    saltvalue = TF_TO_SIXBIT (saltvalue);

    xform (out96, saltvalue);

    toBA64 (out96 /* , crypt_block */ );

    for (i = 0; i < 11; i++)
    {
       k = 0;

       for (j = 0; j < 6; j++)
       {
           k = (k << 1) | crypt_block[SIX_TIMES (i) + j];
       }
       iobuf[i + 2] = final[k];
    }

    iobuf[i + 2] = 0;

    if (iobuf[1] == 0)
    {
       iobuf[1] = iobuf[0];
    }
    return (iobuf);
}
/********* INITIALISATION ROUTINES *********/

fbpb4
lookupS (tableno, t6bits)
    unsl tableno;
    sbpb6R t6bits;
{
    static fbpb4R S[8][64] =
    {
       14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7,
       0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8,
       4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0,
       15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13,

       15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10,
       3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5,
       0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15,
       13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9,

       10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8,
       13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1,
       13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7,
       1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12,

       7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15,
       13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9,
       10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4,
       3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14,

       2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9,
       14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6,
       4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14,
       11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3,

       12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11,
       10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8,
       9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6,
       4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13,

       4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1,
       13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6,
       1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2,
       6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12,

       13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7,
       1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2,
       7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8,
       2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11,
    };
    sbpb6 fixed6bits;
    fbpb4R r;
    fbpb4 fixedr;

    fixed6bits = (((t6bits >> 0) & 01) << 5) +
       (((t6bits >> 1) & 01) << 3) +
       (((t6bits >> 2) & 01) << 2) +
       (((t6bits >> 3) & 01) << 1) +
       (((t6bits >> 4) & 01) << 0) +
       (((t6bits >> 5) & 01) << 4);

    r = S[tableno][fixed6bits];

    fixedr = (((r >> 3) & 01) << 0) +
       (((r >> 2) & 01) << 1) +
       (((r >> 1) & 01) << 2) +
       (((r >> 0) & 01) << 3);

    return (fixedr);
}

void
init (tableno, lowptr, highptr)
    unsl tableno;
    sbpb24 *lowptr, *highptr;
{

    static unsb P[] =
    {
       15, 6, 19, 20,
       28, 11, 27, 16,
       0, 14, 22, 25,
       4, 17, 30, 9,
       1, 7, 23, 13,
       31, 26, 2, 8,
       18, 12, 29, 5,
       21, 10, 3, 24,
    };

    static unsb E[] =
    {
       31, 0, 1, 2, 3, 4,
       3, 4, 5, 6, 7, 8,
       7, 8, 9, 10, 11, 12,
       11, 12, 13, 14, 15, 16,
       15, 16, 17, 18, 19, 20,
       19, 20, 21, 22, 23, 24,
       23, 24, 25, 26, 27, 28,
       27, 28, 29, 30, 31, 0,
    };

    static obpb1 tmp32[32];
    static obpb1 tmpP32[32];
    static obpb1 tmpE[32];

    int j, k, i;
    int tablenoX4;
    reg sbpb24 spare24;

    tablenoX4 = tableno * 4;

    for (j = 0; j < 64; j++)
    {
       k = lookupS (tableno, j);

       for (i = 0; i < 32; i++)
       {
           tmp32[i] = 0;
       }
       for (i = 0; i < 4; i++)
       {
           tmp32[tablenoX4 + i] = (k >> i) & 01;
       }
       for (i = 0; i < 32; i++)
       {
           tmpP32[i] = tmp32[P[i]];
       }
       for (i = 0; i < 48; i++)
       {
           tmpE[i] = tmpP32[E[i]];
       }

       lowptr[j] = 0;
       highptr[j] = 0;

       for (i = 0; i < 24; i++)
       {
           lowptr[j] |= tmpE[i] << i;
       }
       for (k = 0, i = 24; i < 48; i++, k++)
       {
           highptr[j] |= tmpE[i] << k;
       }

       spare24 = lowptr[j];    /* to allow for macro expansion */
       lowptr[j] = TF_TO_SIXBIT (spare24);
       spare24 = highptr[j];   /* to allow for macro expansion */
       highptr[j] = TF_TO_SIXBIT (spare24);
    }
}
init_des ()
{
    init (0, S0L, S0H);
    init (1, S1L, S1H);
    init (2, S2L, S2H);
    init (3, S3L, S3H);
    init (4, S4L, S4H);
    init (5, S5L, S5H);
    init (6, S6L, S6H);
    init (7, S7L, S7H);
}
