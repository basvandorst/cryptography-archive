/******   eliptic.c   *****/
/************************************************************************
*                                                                       *
*       Elliptic curves over Galois Fields.  These routines find points *
*  on a curve, add and double points for type 1 optimal normal bases.   *
*                                                                       *
*	For succint explanaition, see Menezes, page 92                  *
*                                                                       *
************************************************************************/

#include <stdio.h>
#include "bigint.h"
#include "eliptic.h"

/************************************************************************
*  Note that the following is obvious to mathematicians.  I thought it  *
*  was pretty cool when I discovered it myself, <sigh>.			*
*                                                                       *
*       Routine to solve quadradic equation.  Enter with coeficients    *
*  a and b and it returns solutions y[2]: y^2 + ay + b = 0.             *
*  If Tr(b/a^2) != 0, returns y=0 and error code 1.                     *
*  If Tr(b/a^2) == 0, returns y[2] and error code 0.                    *
*  If solution fails, returns y=0 and error code 2.                     *
*                                                                       *
*      Algorithm used based on normal basis GF math.  Since (a+b)^2 =   *
*  a^2 + b^2 it follows that (a+b)^.5 = a^.5 + b^.5.  Note that squaring*
*  is a left shift and rooting is a right shift in a normal basis.      *
*  Transforming the source equation with y = ax and dividing by a^2     *
*  gives:                                                               *
*               x^2 + x + b/a^2 = 0                                     *
*                                                                       *
*       or      x = x^.5 + (b/a^2)^.5                                   *
*                                                                       *
*  Let k_i = the ith significant bit of (b/a^2)^.5 and                  *
*      x_i = the ith significant bit of x.                              *
*  The above equation is equivelent to a bitwise representation as      *
*                                                                       *
*               x_i = x_(i+1) + k_i                                     *
*       or                                                              *
*               x(i+1) = x_i + k_i.                                     *
*                                                                       *
*  Since both x and x+1 are solutions, and 1 is represented by all      *
*  bits set in a normal basis, we can start with x_0 = 0 or 1 at our    *
*  pleasure and use the recursion relation to discover every bit of x.  *
*  The answer is then ax and ax+a returned in y[0] and y[1] respectively*
*  If the sum of x_(n-1) + k_(n-1) != x_0, returns error code 2 and     *
*  y = 0.                                                               *
*                                                                       *
*       error code                      returns                         *
*          0                    y[0] and y[1] values                    *
*          1                    y[0] = y[1] = 0                         *
*          2                    mathematicly impossible !!!!            *
*                                                                       *
************************************************************************/

extern  ELEMENT  mask_table[WORDSIZE];
extern  opt_inv(), rot_left(), rot_right(), null(), opt_mul();
extern  big_print(),copy();

int gf_quadradic(a, b, y)
BIGINT *a, *b, *y;
{
        INDEX   i, l, bits;
        BIGINT  x, k, a2;
        ELEMENT  r, t, mask;

/*  test for b=0.  Won't work if it is.  */

        l = 1;
        for (i=STRTPOS; i<MAXLONG; i++) {
           if (b->b[i] == 0) continue;
           else {
              l = 0;
              break;
           }
        }
        if (l) {
           null(&y[0]);
           null(&y[1]);
           return(1);
        }

/*  find a^-2  */

        opt_inv( a, &a2);
        rot_left(&a2);

/*  find k=(b/a^2)^.5 */

        opt_mul( b, &a2, &k);
        rot_right(&k);
        r = 0;

/*  check that Tr(k) is zero.  Combine all words first. */

        for (i=STRTPOS; i<MAXLONG; i++)
                r ^= k.b[i];

/*  take trace of word, combining half of all the bits each time */

        for (bits = WORDSIZE/2; bits > 0; bits /= 2)
                r = ((r & mask_table[bits]) ^ (r >> bits));

/*  if not zero, return error code 1.  */

        if (r) {
           null(&y[0]);
           null(&y[1]);
           return(1);
        }

/*  point is valid, proceed with solution.  mask points to bit i,
which is known, in x bits previously found and k (=b/a^2).  */

        null(&x);
        mask = 1;
        for (bits=0; bits < NUMBITS ; bits++) {

/* source long word could be different than destination  */

           i = LONGPOS - bits/WORDSIZE;
           l = LONGPOS - (bits + 1)/WORDSIZE;

/*  use present bits to compute next one */

           r = k.b[i] & mask;
           t = x.b[i] & mask;
           r ^= t;

/*  same word, so just shift result up */

           if ( l == i ) {
              r <<= 1;
              x.b[l] |= r;
              mask <<= 1;
           } else {

/*  different word, reset mask and use a 1 */

              mask = 1;
              if (r) x.b[l] = 1;
           }
        }

/*  test that last bit generates a zero */

        r = k.b[STRTPOS] & UPRBIT;
        t = x.b[STRTPOS] & UPRBIT;
        if ( r^t ) {
           null(&y[0]);
           null(&y[1]);
           return(2);
        }

/*  convert solution back via y = ax */

        opt_mul(a, &x, &y[0]);

/*  and create complementary soultion y = ax + a */

	null (&y[1]);
        for (i = STRTPOS; i < MAXLONG;  i++)
                y[1].b[i] = y[0].b[i] ^ a->b[i];

/*  no errors, bye!  */

        return(0);
}

/*  compute R.H.S. f(x) = x^3 + a2*x^2 + a6  
    curv.form = 0 implies a2 = 0, so no extra multiply.  
    curv.form = 1 is the "twist" curve.
*/

fofx(x, curv, f)
BIGINT *x, *f;
CURVE *curv;
{

        BIGINT x2,x3;
        INDEX i;

        copy(x, &x2);
        rot_left(&x2);
        opt_mul(x, &x2, &x3);
        if (curv->form) opt_mul(&x2, &curv->a2, f);
	else null(f);
        for (i = STRTPOS; i < MAXLONG; i++)
             f->b[i] ^= (x3.b[i] ^ curv->a6.b[i]);
}

/****************************************************************************
*                                                                           *
*   Implement elliptic curve point addition for optimal normal basis form.  *
*   Enter with pointers to p1, p2, p3 and curve data for E.  Computes       *
*   p3 = p1 + p2 in projective coordinates over curve E.  See Menezes       *
*   "Elliptic Curve Public Key Cryptosystems."                              *
*   Had to derive this several times to get it correct.  Menezes form not   *
*   generic for both p1 and p2 being arbitrary projections.                 *
*   Note that this is not optimized to minimize multiplies, but to follow   *
*    the math easier.							    *
****************************************************************************/

esum (p1, p2, p3, curv)
POINT   *p1, *p2, *p3;
CURVE   *curv;
{
    INDEX   i;
    BIGINT  x1z2, y1z2, z1z2, x2z1, y2z1;
    BIGINT  A, B, AplusB, Bsqr, C, D, E;
    BIGINT  temp1, temp2;

/*  start with basic building blocks  */

    opt_mul (&p1->x, &p2->z, &x1z2);
    opt_mul (&p1->y, &p2->z, &y1z2);
    opt_mul( &p1->z, &p2->z, &z1z2);    /* z_1 * z_2 */
    opt_mul (&p2->x, &p1->z, &x2z1);
    opt_mul (&p2->y, &p1->z, &y2z1);

/*  find terms A and B  */

    SUMLOOP(i) {
       B.b[i] = x1z2.b[i] ^ x2z1.b[i];
       A.b[i] = y1z2.b[i] ^ y2z1.b[i];
    }

/*  Next group of terms A+B and B^2  */

    SUMLOOP(i) AplusB.b[i] = A.b[i] ^ B.b[i];
    copy( &B, &Bsqr);
    rot_left( &Bsqr);

/*  set up term C.  No multiply if a2 = 0  */

    copy( &B, &C);
    if (curv->form) {
       opt_mul(&curv->a2, &z1z2, &temp1);
       SUMLOOP(i) C.b[i] ^= temp1.b[i];
    }

/*  main term for x3, also used in y3 (D in Menezes book) */

    opt_mul(&A, &AplusB, &temp1);
    opt_mul(&z1z2, &temp1, &temp2);
    opt_mul(&Bsqr, &C, &D);
    SUMLOOP(i) D.b[i] ^= temp2.b[i];

/*  compute x3  */

    opt_mul( &B, &D, &p3->x);

/*  compute z3  */

    opt_mul( &z1z2, &B, &temp1);
    opt_mul(&Bsqr, &temp1, &p3->z);

/*  next major term for y  */

    opt_mul( &x1z2, &A, &temp1);
    opt_mul( &y1z2, &B, &E);
    SUMLOOP(i) E.b[i] ^= temp1.b[i];

/*  finally compute y3  */

    opt_mul( &AplusB, &D, &temp1);
    opt_mul( &Bsqr, &E, &temp2);
    SUMLOOP(i) p3->y.b[i] = temp1.b[i] ^ temp2.b[i];
}

/*  elliptic curve doubling routine for projective coordinates over normal
    basis.  Enter with p1, p3 as source and destination as well as curv
    to operate on.  Returns p3 = 2*p1.
*/

edbl (p1, p3, curv)
POINT *p1, *p3;
CURVE *curv;
{
    BIGINT  x1z1, x14, termd, temp1, temp2;
    INDEX   i;

    opt_mul( &p1->x, &p1->z, &x1z1);
    copy(&p1->x, &x14);              /*  x_1 ^ 4 */
    rot_left( &x14);
    rot_left( &x14);
    copy(&p1->z, &termd);       /*  z_1 ^ 4 */
    rot_left ( &termd);
    rot_left ( &termd);
    opt_mul ( &termd, &curv->a6, &temp1);
    SUMLOOP(i) termd.b[i] = x14.b[i] ^ temp1.b[i];  /*  D = x1^4 + a6*z1^4 */

/*  now compute results  */

    opt_mul ( &termd, &x1z1, &p3->x);
    copy ( &x1z1, &temp1);
    rot_left( &temp1);
    opt_mul ( &temp1, &x1z1, &p3->z);    /* z3 = A^3 */
    opt_mul ( &x1z1, &x14, &p3->y);      /* x1^5 * z1 */
    copy ( &p1->x, &temp1);
    rot_left( &temp1);          /* x1 ^ 2 */
    opt_mul ( &p1->y, &p1->z, &temp2);  /*  y1 * z1 */
    SUMLOOP(i) {
       temp2.b[i] ^= temp1.b[i];        /*  proto sum for D */
       p3->y.b[i] ^= p3->x.b[i];        /*  first 2 terms of y3 found */
    }
    opt_mul ( &termd, &temp2, &temp1);  /*  do multiply */
    SUMLOOP(i) p3->y.b[i] ^= temp1.b[i];    /* and finish y3 */
}

/*  subtract two points on a curve.  just negates p2 and does a sum.
    Returns p3 = p1 - p2 over curv.
*/

esub (p1, p2, p3, curv)
POINT   *p1, *p2, *p3;
CURVE   *curv;
{
    POINT   negp;
    INDEX   i;

    copy ( &p2->x, &negp.x);
    null (&negp.y);
    SUMLOOP(i) negp.y.b[i] = p2->x.b[i] ^ p2->y.b[i];
    copy ( &p2->z, &negp.z);
    esum (p1, &negp, p3, curv);
}

/*  need to move points around, not just values.  Optimize later.  */

void copy_point (p1, p2)
POINT *p1, *p2;
{
	copy (&p1->x, &p2->x);
	copy (&p1->y, &p2->y);
	copy (&p1->z, &p2->z);
}

/*  Routine to compute kP where k is an integer (base 2, not normal basis)
	and P is a point on an elliptic curve.  This routine assumes that K
	is representable in the same bit field as x, y or z values of P.
	This is for simplicity, larger or smaller fields can be independently 
	implemented.
    Enter with: integer k, source point P, curve to compute over (curv) and 
    Returns with: result point R.

  Reference: Koblitz, "CM-Curves with good Cryptografic Properties", 
	Springer-Verlag LNCS #576, p279 (pg 284 really), 1992
*/
ELEMENT	bit_table[32] = {1, 2, 4, 8, 16, 32, 64, 128, 256,
			0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000,
			0x10000, 0x20000, 0x40000, 0x80000,
			0x100000, 0x200000, 0x400000, 0x800000,
			0x1000000, 0x2000000, 0x4000000, 0x8000000,
			0x10000000, 0x20000000, 0x40000000, 0x80000000 };

void  elptic_mul(k, p, r, curv)
BIGINT	*k;
POINT	*p, *r;
CURVE	*curv;
{
	char		blncd[NUMBITS+1];
	ELEMENT		wordptr, bit_count, mskindx, sequnce;
	POINT		temp;

/*  scan across k from right to left to expand bits to chars  */

	for (bit_count = 0; bit_count < NUMBITS; bit_count++) {
	   wordptr = LONGPOS - bit_count/WORDSIZE;
	   mskindx = bit_count % WORDSIZE;
	   if (k->b[wordptr] & bit_table[mskindx]) blncd[bit_count] = 1;
	   else blncd[bit_count] = 0;
	}
	blncd[NUMBITS] = 0;

/*  Next convert balanced array to ballanced representation  */
/*  sequence flag used for multiple 1's being switched to 0's.  */

	sequnce = 0;
	bit_count = 0;
	while (bit_count <= NUMBITS) {
	   if ( blncd[bit_count]) {
	      if (sequnce)		/*  in middle of 1's sequence */
		blncd[bit_count] = 0;
	      else if (blncd[bit_count+1]) {
		sequnce = 1;		/*  next bit also set, begin sequnce */
		blncd[bit_count] = -1;
	      }
	      bit_count++;
	   } else {	
	      if (sequnce) { 		/* last bit of sequence */
		blncd[bit_count] = 1;
		sequnce = 0;
	      } else
		bit_count++;
	   }
	}

/*  now follow ballanced representation and compute kP  */

	bit_count = NUMBITS;
	while (!blncd[bit_count] && bit_count >= 0) bit_count--; /* find first bit */
	if (bit_count < 0) {
	   null (&r->x);
	   null (&r->y);
	   null (&r->z);
	   return;
	}
	copy_point(p,r);		/* first bit always set */
	while (bit_count > 0) {
	  edbl(r, &temp, curv);
	  bit_count--;
	  switch (blncd[bit_count]) {
	     case 1: esum (p, &temp, r, curv);
			break;
	     case -1: esub (&temp, p, r, curv);
			break;
	     case 0: copy_point (&temp, r);
	   }
	}
}

/*  One is not what it appears to be.  In any normal basis, "1" is the sum of
all powers of the generator.  So this routine puts ones to fill the number size
being used in the address of the BIGINT supplied.  */

void one (place)
BIGINT *place;
{
	INDEX i;

	SUMLOOP(i) place->b[i] = -1L;
	place->b[STRTPOS] &= UPRMASK;
}

/*  This routine "flatens" a projective 3D point back to the 2D plane.  */

void flaten (point)
POINT *point;
{
	POINT invers;

	opt_inv( &point->z, &invers.z);
	opt_mul( &invers.z, &point->x, &invers.x);
	opt_mul( &invers.z, &point->y, &invers.y);
	copy (&invers.x, &point->x);
	copy (&invers.y, &point->y);
	one(&point->z);
}

