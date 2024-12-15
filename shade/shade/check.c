#include <stdio.h>
#ifdef CHECK
#undef CHECK
#endif
#include "shade.h"

/* Checking for poly arithmetic
 *
 * These routines verify the result of various functions.
 * These checks are very secure and efficient.  Except in
 * the case of c_pinv(), the return value is garanteed to
 * be correct (Unless there is a bug in the checking code...).
 *
 * c_pinv() might return a 0 without checking that the arguments
 * really aren't relatively prime.
 *
 * When they detect an error, they exit with a return code 42
 * and an appropriate error message.
 *
 * Using the checked routines incurs a small performance penalty.
 * A quick test suggest the penalty is roughly 15-20%.
 */

/* Multiply back x and its inverse to check that the product falls
 * within the valid range.
 */
UWORD c_inv16( UWORD x)
{
    UWORD ret = inv16(x);
    if ((mul16( ret, x)>>16) ^ ret ^ x) {
        fprintf( stderr, "inv16 ERROR: 1%04X X 1%04X == 1%08X\n",
                 ret, x, mul16( ret, x) ^ (ret<<16) ^ (x<<16));
        exit( 42);
    }
    return ret;
}

/* Multiply back to see wether the modulo is small enough
 */
UWORD c_div16x( ULONG e, UWORD d)
{
    UWORD ret = div16x( e, d);
    if (((mul16( ret, d) ^ e)>>16) ^ ret) {
        fprintf( stderr, "dvi16x ERROR: %08X != %04X X 1%04X + %04X\n",
                 e, ret, d, mul16( ret, d) ^ (ret<<16) ^ e);
        exit( 42);
    }
    return ret;
}

/* Multiply back to see wether the modulo is small enough
 */
UWORD c_div16( ULONG e, UWORD d)
{
    UWORD ret = div16( e, d);
    if (((d>>1)|(mul16( ret, d) ^ e)) >= d) {
        fprintf( stderr, "dvi16 ERROR: %08X != %04X X %04X + %04X\n",
                 e, ret, d, mul16( ret, d) ^ e);
        exit( 42);
    }
    return ret;
}

/* Verify that a*x ^ b*y == 1 */
UWORD c_gcd( UWORD x, UWORD y, UWORD *a, UWORD *b)
{
    UWORD ret = gcd( x, y, a, b);
    if ((mul16( x, *a) ^ mul16( y, *b)) != ret) {
        fprintf( stderr, "gcd ERROR: %04X X %04X + %04X X %04X != %04X\n",
                 x, *a, y, *b, ret);
        exit( 42);
    }
    return ret;
}

/* When result is not 0, multiply y and the result to check wether
 * the product == 1 mod x
 */
UWORD c_pinv( UWORD y, UWORD x)
{
    ULONG zzz, zz, z = pinv( y, x);
    if (!z) return 0;
    zz = mul16( z, y);
    zzz = div16x( zz, x);
    if ((mul16( zzz, x) ^ (zzz<<16) ^ zz) != 1) {
        printf( "pinv ERROR: %04X X %04X != 1 (mod 1%04X)\n", z, y, x);
        exit( 10);
    }
    return z;
}



