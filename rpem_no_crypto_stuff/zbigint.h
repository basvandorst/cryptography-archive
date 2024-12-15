/*--- ZBIGINT.H:  Header file for basic bignum operations */

/* Most of these symbols were invented by Arjen Lenstra.
 * I have attempted to elucidate them here.  /mrr
 *
 * MYINT is a symbol which should be "long int" or "int",
 * depending on what's more efficient on your machine.
 * When in doubt, stick to "long int".
 *
 * Many of the other symbols depend upon how big MYINTs are
 * your machine.  Just define the symbol MYINT_BITS as 16, 32, or 64.
 *
 * Note:  The first three are the most likely to have to be
 * changed to meet your needs.  They affect the magnitude of
 * the largest BIGINT.
 *
 * ZMAX_BITS   The maximum number of bits that will ever need to
 *             fit in a BIGINT.  Make sure it's big enough to 
 *             handle intermediate results.  (Make it twice as big
 *             as your biggest modulus.)
 * ZSIZEP      The number of long ints taken up by a BIGINT.
 *             Note:  The largest BIGINT is 2^((ZSIZEP-1)*NBITS)-1.
 *             A ZSIZEP of 15 means that you can represent
 *             up to 2^(14*30)-1 = 2^520 - 1.  But note that you
 *             will probably want ZSIZEP to be big enough to
 *             accomodate numbers larger than you are using as
 *             input, to allow for intermediate results.
 * ZZSIZEP     The number of long ints required to represent the biggest
 *             BIGINT if you use only NBITSH bits of each BIGINT.
 *             ZZSIZEP = ZSIZEP*2 - 1 (the one for the first long int
 *             used to hold the length).
 * ZDECSIZE    Number of characters needed to represent the largest
 *             BIGINT in decimal ASCII form.
 * NBITS       Number of bits used in a long int.  Must be even and must
 *             be less than the size of a long int to avoid overflow
 *             to sign bit.
 * NBITSH      Half of NBITS (used in division, amoung others)
 * RADIX       2^NBITS
 * RADIXM      RADIX-1; in other words, a bit mask that covers all
 *             the bits of a long int that we use.
 * RADIXROOT   sqrt(RADIX)
 * RADIXROOTM  RADIXROOT-1; in other words, a bit mask that covers
 *             the bottom half of the bits used in a long int.
 * ZSINT       I can't tell the difference in meaning between this
 *             and RADIXROOT.  Used to avoid overflow in divisions (?).
 * PRIMBND     Used in prime number generation.
 * INDBND      Used in prime number generation.
 * MRANDBITS   The number of bits returned by zrandombits.  Should
 *             always be = NBITS with current implementation.
 * TWO_TO_THE_MRANDBITS  2^MRANDBITS, which should be a long int.
 * BITS_PER_BYTE   You can guess this one.
 */

/* 32 bits seems to be fastest even for PCs, although they are
 * emulating 32-bit arithmetic with 16-bit instructions.
 */

#define MYINT long int
#define MYINT_BITS 32

/* Very important:  Make sure that ZMAX_BITS is set big enough. */

#define ZMAX_BITS  2000

#if MYINT_BITS == 32
#define  NBITS       30
#define  NBITSH      15
#define  RADIX       1073741824
#define  RADIXM      1073741823
#define  RADIXROOT   32768
#define  RADIXROOTM  32767
#define  ZSINT       32768
#define  MRANDBITS   30
#define  TWO_TO_THE_MRANDBITS 1073741824
#endif
/* Some lousy C compilers don't recognize #elif. */
#if MYINT_BITS == 16
#define  NBITS       14
#define  NBITSH      7
#define  RADIX       16384
#define  RADIXM      16383
#define  RADIXROOT   128
#define  RADIXROOTM  127
#define  ZSINT       128
#define  MRANDBITS   14
#define  TWO_TO_THE_MRANDBITS 16384
#endif
#if MYINT_BITS == 64
#define  NBITS       62
#define  NBITSH      31
#define  RADIX       0x4000000000000000
#define  RADIXM      0x3fffffffffffffff
#define  RADIXROOT   0x80000000
#define  RADIXROOTM  0x7fffffff
#define  ZSINT       0x80000000
#define  MRANDBITS   62
#define  TWO_TO_THE_MRANDBITS 0x400000000000000
#endif

#define  ZSIZEP      ((ZMAX_BITS/NBITS)+1)
#define  ZZSIZEP     (2*ZSIZEP-1)
#define  ZDECSIZE    ((ZMAX_BITS)/3)
#define  PRIMBND     5000
#define  INDBND      48

#define  BITS_PER_BYTE  8

typedef struct { MYINT len; MYINT val[ZSIZEP-1]; } BIGINT;

#ifdef Z_DEFINE
#define DEF
#else
#define DEF extern
#endif

DEF BIGINT z_one;

/* The macros below mimic another multiple-precision package.
 * They are included here to ease porting of code and to allow
 * the programmer to maintain the coding discipline needed
 * in an environment in which the BIGINTs are dynamically allocated,
 * in case a dynamic version of this package is released.
 */

#define MINIT(x) zset((MYINT)0,x)
#define MSET(c,x) zset((MYINT)c,x)
#define MCOPY(x,y) zcopy(x,y)
#define MMOVE(x,y) MCOPY(x,y)
#define MFREE(x)
#define MMOVEFREE(x,y) MCOPY(x,y)


/*--- End of ZBIGINT.H ---------------------------------------- */
