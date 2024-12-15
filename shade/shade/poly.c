#include <stdio.h>
#ifdef CHECK
#undef CHECK
#endif
#include "shade.h"

/* Polynomials.
 *
 * These routines deal with arithmetic on polynomials with coeficients
 * modulo 2.  We represent these poly with unsigned integer types.
 * The least significant bit of these integers is the constant term
 * of the poly.  Addition of two poly is simply the exclusive
 * or of the integers and multiplication is defined in terms of
 * shifts and xors.
 *
 * These polynomials are used because they share many properties
 * with natural numbers but there are no carries to propagate.
 *
 * To represent a polynomial of degree n, we need at least n+1 bits.
 * Therefore, a word can contain any polynomial of degree < 16.
 * But we often use a word to store polynomials of degree exactly
 * 16.  The 17-th bit being always 1, there is no need to store it.
 */

/* Multiplication of polynomials
 *
 * mul4_tab[] is a table lookup to multiply 2 poly of degree < 4
 * mul8() multiplies 2 poly of degree < 8
 * mul16() multiplies 2 poly of degree < 16
 */


/* mul4_tab[]
 *
 * x * y == mul4_tab[(x<<4)|y]
 * when x and y are 4-bit poly (degree < 4)
 *
 * see make_table.c for the code that precalculates this table.
 */

/* mul8()
 *
 * x * y == mul8( x, y)
 * when x and y are 8-bit poly (degree < 8)
 *
 * The multiplication is carried out using 3 lookups into mul4_tab.
 * say x == (a<<4)^b and y == (c<<4)^d
 * where a, b, c and d are 4-bit poly.
 *
 * let's note x' for (x<<4) and xy for x^y
 * then x * y == a'b * c'd == (a*c)'' ((a*d)(b*c))' (b*d)
 * and ab * cd             == (a*c)   (a*d)(b*c)    (b*d)
 *
 * then let i = a*c
 *          j = b*d
 *          k = ij(ab*cd)
 *
 * we have that
 *      x * y = i'' k' j
 * using 3 smaller multiplications.
 */
UWORD mul8( UBYTE x, UBYTE y) /* 8-bit * 8-bit --> 15-bit */
{
    register UBYTE i, j=x, k=y;
    i = (j&0xF0) ^ (k>>4);
    j = (j<<4) ^ (k&0x0F);
    k = mul4_tab[i^j];
    k ^= i = mul4_tab[i];
    k ^= j = mul4_tab[j];
    return (((UWORD)i)<<8) ^ (((UWORD)k)<<4) ^ ((UWORD)j);
}

/* mul16()
 *
 * x * y == mul16(x, y)
 * when x and y are 16-bit poly (degree < 16)
 *
 * we use the same algorithm as mul8, using 8-bit halves.
 *
 * often, we need to multiply a 16-bit poly by one of degree == 16;
 * let's say y is of degree 16 and x of lesser degree.
 * then x * y == mul16( x, y) ^ (x << 16)
 *
 * If both x and y are of degree 16
 * then x * y == mul16( x, y) ^ ((x^y)<<16) ^ (1<<32)
 */
ULONG mul16( UWORD x, UWORD y) /* 16-bit * 16-bit --> 31-bit */
{
    UWORD a, b, c;
    a = ((UBYTE*)&x)[MSB] ^ ((UBYTE*)&x)[LSB];
    b = ((UBYTE*)&y)[MSB] ^ ((UBYTE*)&y)[LSB];
    c = mul8( (UBYTE)a, (UBYTE)b);
    c ^= a = mul8( ((UBYTE*)&x)[MSB], ((UBYTE*)&y)[MSB]);
    c ^= b = mul8( ((UBYTE*)&x)[LSB], ((UBYTE*)&y)[LSB]);
    return (((ULONG)a)<<16) ^ (((ULONG)c)<<8) ^ ((ULONG)b);
}




/* Division of polynomials.
 *
 * When dividing a 32-bit poly by a poly of degree 16,
 * we first find the pseudo-inverse of the divisor.
 * Then a simple multiplication and shift yields the
 * result.  The remainder is always a 16-bit poly.
 *
 * Division by a 16-bit poly is only defined when the
 * quotient fits in a 16-bit poly (e.i div16( 0x8000000, 0x8000)
 * will return a gobus result).  Instead, use 2 divisions.
 *
 * To find the modulo, multiply back the quotient and the
 * divisor, then xor the product with the dividend.
 *
 * shft_tab[] finds the number of shifts necessary to make an
 *            poly of degree 8 from an 8-bit poly.
 * inv8_tab[] finds the pseudo-inverse of a poly of degree 8
 * inv16() finds the pseudo-inverse of a poly of degree 16
 * div16x() divides a 32-bit poly by a poly of degree 16
 * div16() divides a (i+16)-bit poly by a poly of degree i < 16
 */



/* Pseudo-inverse of polynomials
 *
 * If x is a poly of degree 16, the pseudo-inverse y of x
 * is a poly of degree 16 such that:
 *  0x100000000 <= x*y <= 0x10000FFFF
 * so that (z*y)>>32 == z/x where z is a 32-bit poly.
 *  let z = a*x + b where a and b are 16-bit poly.
 *  z*y = a*x*y + b*y
 *  (z*y)>>32 = (a*x*y)>>32 + (b*y)>>32
 *  (z*y)>>32 = (a*x*y)>>32
 *  (z*y)>>32 = (a*c)>>32, where 0x100000000 <= c < 0x10000FFFF
 *  (a*0x100000000)>>32 <= (z*y)>>32 <= (a*0x100000000)>>32 + (a*0x0000FFFF)>>32
 *  a <= (z*y)>>32 <= a
 *
 * To find this inverse, we start with an approximation stored in
 * a lookup table and use Newton's approximation.
 *
 * let y$ be the pseudo-inverse of (x>>8)
 * then y = (x * (y$ * y$)) >> 16
 */
UWORD inv16( UWORD x)
{
    UWORD i = inv8_tab[x>>8];
    i = mul8( i, i);
    return (mul16( i, x)>>16) ^ i ^ x;
}

UWORD div16x( ULONG e, UWORD d)
{
    d = inv16(d);
    e = e >> 16;
    return (mul16( e, d) >> 16) ^ e;
}

UWORD div16( ULONG e, UWORD d)
{
    if (!((UBYTE*)&d)[MSB]) { e<<=8; d<<=8; }
    e <<= shft_tab[((UBYTE*)&d)[MSB]];
    d <<= shft_tab[((UBYTE*)&d)[MSB]];
    return div16x( e, d);
}

/* Euclid's GCD, x >= y */
UWORD gcd( UWORD x, UWORD y, UWORD *a, UWORD *b)
{
    UWORD d; UWORD g;
    if (!y) { *a = 1; *b = 0; return x; }
    d = div16((ULONG)x, y);
    g = gcd( y, x ^ mul16( d, y), b, a);
    *b ^= mul16( d, *a);
    return g;
}


/*
 * Find the multiplicative inverse of x modulo m.  In
 * other words find y such that mul16(x,y) mod m == 1.
 *
 * m is a poly of degree 16 and x a 16-bit poly.
 * This is a variant of Euclid's extended gcd.
 * a*x + b*m == 1
 * a == y
 */
UWORD pinv( UWORD x, UWORD m)
{
    UWORD a, b, d;
    if (!x) return 0;
    if (x==1) return 1;
    d = div16( 0x10000|(ULONG)m, x);
    a = m ^ mul16( x, d);
    if (gcd( x, a, &b, &a) != 1) return 0;
    b ^= mul16( d, a);
    return b;
}




