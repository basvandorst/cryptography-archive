/******   bigint.c   *****/
/************************************************************************
*                                                                       *
*       Routines to implement optimal normal basis multiplication.      *
*  See Mullin, Onyszchuk, Vanstone, Wilson, "Optimal Normal Bases in    *
*  GF(p^n)", Discrete Applied Math, V22, 1988, p149                     *
*   Ash, Blake, Vanstone, "Low Complexity Normal Bases", Discrete       *
*   Applied Math, V25, 1989, p191                                       *
* Agnew, Mullin, Onyszchuk, Vanstone, "An Implementation for a Fast     *
*  Public-Key Cryptosystem", Jour. Cryptology, 1991, V3, p 63           *
* "Elliptic Curve Public Key Cryptosystems", A. Menezes, Kluwer, 1993   *
*  pages 83-86.                                                         *
*                                                                       *
*                       Jan. 22, 1995                                   *
*                                                                       *
************************************************************************/

#include <stdio.h>
#include "bigint.h"

/*  global structure for all multiply routines.  compute once at begining
to save disk space.  */

INDEX  Lambda[field_prime];   /*  for multiply  */
ELEMENT mask_table[WORDSIZE];   /*  for shift_index  */

/*  shift routines assume bigendian structure.  operate in place */

void shift_left(a)
BIGINT *a;
{
        register INDEX i;
        register ELEMENT bit,temp;

        bit = 0;
        for (i=LONGPOS; i>=STRTPOS; i--) {
           temp = ( a->b[i] << 1) | bit;
           bit = (a->b[i] & SUBMASK) ? 1L : 0L;
           a->b[i] = temp;
        }
        a->b[STRTPOS] &= UPRMASK;
}

void shift_right(a)
BIGINT *a;
{
        register INDEX i;
        register ELEMENT bit,temp;

        bit = (a->b[STRTPOS] & 1) ? SUBMASK : 0;
        for (i=STRTPOS; i< MAXLONG; i++) {
           temp = ( a->b[i] >> 1) | bit;
           bit = (a->b[i] & 1) ? SUBMASK : 0;
           a->b[i] = temp;
        }
}

/* an entirely different way to do the same thing */
void rot_left(a)
BIGINT *a;
{
        register INDEX i;
        register ELEMENT bit,temp;

        bit = (a->b[STRTPOS] & UPRBIT) ? 1L : 0L;
        for (i=LONGPOS; i>=STRTPOS; i--) {
           temp = (a->b[i] & SUBMASK) ? 1L : 0L;
           a->b[i] = ( a->b[i] << 1) | bit;
           bit = temp;
        }
        a->b[STRTPOS] &= UPRMASK;
}

void rot_right(a)
BIGINT *a;
{
        register INDEX i;
        register ELEMENT bit,temp;

        bit = (a->b[LONGPOS] & 1) ? UPRBIT : 0L;
        for (i=STRTPOS; i< MAXLONG; i++) {
           temp = ( a->b[i] >> 1)  | bit;
           bit = (a->b[i] & 1) ? SUBMASK : 0L;
           a->b[i] = temp;
        }
        a->b[STRTPOS] &= UPRMASK;
}

void null(a)
BIGINT *a;
{
        register INDEX i;

        for (i=0; i<MAXLONG; i++)  a->b[i] = 0;
}

void copy (a,b)
BIGINT *a,*b;
{
        register INDEX i;

        for (i=0; i<MAXLONG; i++)  b->b[i] = a->b[i];
}

/* create Lambda [i,j] table.  indexed by j, each entry contains the
value of i which satisfies 2^i + 2^j = 1 || 0 mod field_prime.  There are
two 16 bit entries per index j except for zero.  See references for
details.  Since 2^0 = 1 and 2^2n = 1, 2^n = -1 and the first entry would
be 2^0 + 2^n = 0.  Multiplying both sides by 2, it stays congruent to
zero.  So Half the table is unnecessary since multiplying exponents by
2 is the same as squaring is the same as rotation once.  Lambda[0] stores
n = (field_prime - 1)/2.  The terms congruent to one must be found via
lookup in the log table.  Since every entry for (i,j) also generates an
entry for (j,i), the whole 1D table can be built quickly.
*/

void genlambda()
{
        INDEX i,logof,n,index;
        INDEX log2[field_prime],twoexp;

        for (i=0; i<field_prime; i++) log2[i] = -1;

/*  build antilog table first  */

        twoexp = 1;
        for (i=0; i<field_prime; i++) {
          log2[twoexp] = i;
          twoexp = (twoexp << 1) % field_prime;
        }

/*  compute n for easy reference */

        n = (field_prime - 1)/2;
        Lambda[0] = n;
        Lambda[1] = n;
        Lambda[n] = 1;

/*  loop over result space.  Since we want 2^i + 2^j = 1 mod field_prime
        it's a ton easier to loop on 2^i and look up i then solve the silly
        equations.  Think about it, make a table, and it'll be obvious.  */

        for (i=2; i<=n; i++) {
          index = log2[i];
          logof = log2[field_prime - i + 1];
          Lambda[index] = logof;
          Lambda[logof] = index;
        }
/*  last term, it's the only one which equals itself.  See references.  */

        Lambda[log2[n+1]] = log2[n+1];
}

/*  for use with multiply routine.  indexed rotation uses:
        a is source
        b is destination
        amount is bit position which gets moved to zero bit
  For the moment, starting with rotations if shift amount is same
        size as first or last words.  May want to divide that by 2
        depending on how slow shift is compared with bit chunk mover.
  Bit chunk mover needs mask table, which should be built during
        initialization.
*/

void initmask()
{
        register INDEX i;

        mask_table[0] = -1L;
        for (i=1; i<WORDSIZE; i++) mask_table[i] = (ELEMENT)~(-1L << i);
}

void shift_index(src,dst,amount)
BIGINT *src,*dst;
INDEX amount;
{
        INDEX srcptr, srcstrt, srcwrd, limit;
        INDEX dstptr, dststrt, dstwrd, bitsmvd, strt;
        ELEMENT temp, mask;
        INDEX srcbits, dstbits, shift;

        null(dst);

/*  first determine if simple rotation is worth doing.  This could use
        some tuning, depending on processor, bus time, etc.
*/
        limit = 3;
        if ((amount < limit) || (amount > (NUMBITS - limit))){
           copy(src,dst);
           if (! amount) return;
           if (amount < limit) {
              while (amount) {
                rot_right(dst);
                amount--;
              }
           } else {
              while (amount < NUMBITS) {
                rot_left(dst);
                amount++;
              }
           }
           return;
        }

/*  rather than move every word amount times, try and only read it 2 or 3 and
        put each sub component in the right place.
*/

/*  initialize bit pointers */

        srcptr = amount;
        dstptr = 0;

/*  scan at bit level  */

        while (dstptr < NUMBITS) {

/*  find word offsets into bigint array  */

           srcwrd = LONGPOS - srcptr/WORDSIZE;
           dstwrd = LONGPOS - dstptr/WORDSIZE;

/*  locate starting bit positions within each ELEMENT  */

           srcstrt = srcptr % WORDSIZE;
           dststrt = dstptr % WORDSIZE;

/* smallest value determines maximum bits that can be moved  */

           srcbits = (srcwrd == STRTPOS) ? UPRSHIFT - srcstrt
                                        : WORDSIZE - srcstrt;
           dstbits = (dstwrd == STRTPOS) ? UPRSHIFT - dststrt
                                        : WORDSIZE - dststrt;
           bitsmvd = (srcbits < dstbits) ? srcbits : dstbits;

/* max bits set in zero index of mask_table, then adjust mask over
        result and compute source shift amount.   */

           bitsmvd %= WORDSIZE;
           mask = mask_table[bitsmvd] << dststrt;
           temp = src->b[srcwrd];
           shift = abs(srcstrt - dststrt);

/*  create result, or into place */

           if (dststrt < srcstrt)
             temp = (temp >> shift) & mask;
           else
             temp = (temp << shift) & mask;
           dst->b[dstwrd] |= temp;

/*  bump source cyclicly, destination linearly */

           if (!bitsmvd) bitsmvd = WORDSIZE;
           srcptr = (srcptr + bitsmvd) % NUMBITS;
           dstptr = (dstptr + bitsmvd);
        }  /* until dstptr >= NUMBITS */
}

/*  Normal Basis Multiplication.  Assumes Lambda vector already initialized
        for type 1 normal basis.  See above references for details
                Output = c = a*b over GF(2^NUMBITS)
*/

void opt_mul(a,b,c)
BIGINT *a,*b,*c;
{
        register INDEX i,j;
        BIGINT a1,a2,bcopy;

        null(c);
        copy(b,&bcopy);

/*  for each rotation of B vector there are at most two rotations of A vector
in a type 1 normal basis.  Lambda is lookup table of A rotations from 2^i +
2^j = 1 mod field_prime.  For 2^i + 2^j = 0, need only one master shift and
single rotations thereafter.  The Lambda table is uniformly scrambled and
shows why this is an efficient bit mixing algorithm.
*/

        shift_index(a,&a1,Lambda[0]);
        for (i=STRTPOS; i<MAXLONG; i++)
                c->b[i] = bcopy.b[i] & a1.b[i];

/*  main loop, two lookups for every position */

        for (j=1; j<NUMBITS; j++) {
           rot_right(&bcopy);
           rot_right(&a1);
           shift_index(a,&a2,Lambda[j]);
           for (i=STRTPOS; i<MAXLONG; i++) {
/*              c->b[i] ^= b->b[i] & (a1.b[i] ^ a2.b[i]);
        expression too complex!!??!!    */
              a2.b[i] ^= a1.b[i];
              a2.b[i] &= bcopy.b[i];
              c->b[i] ^= a2.b[i];
           }
        }
}

/*  opt_inv computes the inverse of a normal basis, GF(2^n) "number".
        Enter with pointers to source number, destination storage,
Lambda table and mask_table.  Leaves source alone and puts its inverse
into destination.  The algorithm is based on the explanation given in
Menezes book.  the bit pattern of n-1 determines the number of multiplies.
a sliding window starting from msb determines the power of 2 (shift index)
and the next bit determines the number of multiplies for that power.
Seems pretty cool to this programmer!
*/

void opt_inv(src,dst)
BIGINT *src, *dst;
{
        INDEX m, x, y, bitpos;
        BIGINT ap1,ap2;

        copy(src, &ap1);
        null(dst);
        m = field_prime - 2; /* = NUMBITS - 1 */
        bitpos = log2_fp;

        x = m >> bitpos;
        if (x != 1) {
           printf("Error attempting to compute inverse!!\n");
           printf("x=%d m=%d bitpos=%d\n",x,m,bitpos);
           return;
        }
        while (x < m) {
           bitpos--;
           y = m >> bitpos;
           shift_index( &ap1, &ap2, NUMBITS-x);
           opt_mul( &ap1, &ap2, dst);
           if (y & 1) {
              rot_left(dst);            /* this squares destination */
              opt_mul( src, dst, &ap1);
           } else
              copy(dst, &ap1);
           x = y;
        }
        rot_left( &ap1); /* final squaring */
        copy (&ap1,dst);
}

void init_opt_math()
{

        initmask();             /*  create shift_index mask table  */
        genlambda();            /*  create Lambda pointer table    */
}
