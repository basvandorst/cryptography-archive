/*	File mpn_accelgcd.c

	accelerated gcd--based on Sorenson's k-ary gcd.

	Preconditions:
	1. (u, ul) has at least as many bits as (v, vl).
	2.  Both are odd.

	Postcondition: (g, return value) <-- gcd((u, ul), (v, vl))

	Ken Weber (kweber@mcs.kent.edu)		07/08/93

	Refer to
	    K. Weber, The accelerated integer gcd algorithm,
	    Dept. of Mathematics and Computer Science,
	    Kent State University, 1993.
*/

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#ifdef TIME
mp_limb gl_inter;
#endif /* TIME */

/* The algorithm will switch to bmod gcd when size of smaller
   operand <= ACCEL_THRESHOLD.  */
#ifndef ACCEL_THRESHOLD
#define ACCEL_THRESHOLD 7	/*  must be > 0  */
#endif /* ACCEL_THRESHOLD */

/* When the "reducants" differ in size by more than BMOD_THRESHOLD(l),
   the algorithm reduces using the bmod operation.  Otherwise, the
   k-ary reduction is used.

   It is required that 0 <= BMOD_THRESHOLD(l) < BITS_PER_MP_LIMB.  */

#if defined (__ns32000__) || defined (__m68000__) || defined (__i386__)
#define BMOD_THRESHOLD(l) 6
#endif /* BMOD_THRESHOLD */

#if ! defined (BMOD_THRESHOLD)
#define BMOD_THRESHOLD(l) 10
#endif

#if BMOD_THRESHOLD(0) >= BITS_PER_MP_LIMB
  lossage!
#endif

/* NEG1 == -1 (mod 2^BITS_PER_MP_LIMB  */
#define NEG1 (-(mp_limb)1)

/* MP_LIMB_HI_BIT = 2^(BITS_PER_MP_LIMB - 1)  */
#define MP_LIMB_HI_BIT ((mp_limb)1 << (BITS_PER_MP_LIMB - 1))

/* These macros perform operations on 2-limb operands.  */

/* (h, l) <<= i  */
#define DP_LSHIFT(h, l, i) \
    ((h) <<= (i), (h) |= (l) >> (BITS_PER_MP_LIMB - (i)), (l) <<= i)

/* (h, l) >>= 1  */
#define DP_DIV2(h, l) \
  ((l) = ((h) & 1) ? MP_LIMB_HI_BIT | ((l) >> 1) : (l) >> 1, (h) >>= 1)

/* (h1, l1) > (h2, l2)  */
#define DP_GT(h1, l1, h2, l2) ((h1) > (h2) || ((h1) == (h2) && (l1) > (l2)))

/**************************************************************************/

/*
	Replace U by |a*U + b*V|/2^s, where a, b are chosen so that
	a*U + b*V == 0 (mod 2^(2*BITS_PER_MP_LIMB)) and s is chosen so that the
	result is odd.

	Preconditions:
	1.  U = (u, ul) and V = (v, vl) are odd.
	2.  1 < vl <= ul <= vl + 1.
	3.  There must be at least -->ul + 2<-- limbs allocated at u.
*/

static mp_size_t
kary_reduce (u, ul, v, vl)
     mp_ptr u;
     mp_size_t ul;
     mp_srcptr v;
     mp_size_t vl;
{
  mp_limb a, b;
  int add;

  /* First find a and b  */
  if (u[1] == 0 && v[1] == 0) 		/* Easy case  */
    {
      a = v[0];
      b = u[0];
      add = 0;
    }
  else
    {
      mp_limb num1_hi = 0;
      mp_limb num1_lo = 0;

      /* num1 <-- u/v (mod 2^(2*BITS_PER_MP_LIMB))  */
      {
	mp_limb make_0_hi = u[1];
	mp_limb make_0_lo = u[0];
	mp_limb vshift_hi = v[1];
	mp_limb vshift_lo = v[0];
	mp_limb two_i = 1;

	/* First low half  */
	do
	  {
	    while ((two_i & make_0_lo) == 0)
	      {
		add_ssaaaa (vshift_hi, vshift_lo, vshift_hi, vshift_lo, vshift_hi, vshift_lo);
		two_i <<= 1;
	      }
	    num1_lo += two_i; 	/* no carry  */
	    sub_ddmmss (make_0_hi, make_0_lo, make_0_hi, make_0_lo, vshift_hi, vshift_lo);
	  }
	while (make_0_lo);

	/* Now high half  */
	two_i = 1;
	vshift_hi = v[0];
	while (make_0_hi)
	  {
	    while ((two_i & make_0_hi) == 0)
	      {
		two_i <<= 1;
		vshift_hi <<= 1;
	      }
	    num1_hi += two_i;
	    make_0_hi -= vshift_hi;
	  }
      }

      if (num1_hi)
	{
	  mp_limb denshift, numshift_hi, numshift_lo, num2_hi, num2_lo;
	  mp_limb den1 = 1;
	  mp_limb den2 = -1;
	  mp_limb high_bit = MP_LIMB_HI_BIT;
	  sub_ddmmss (num2_hi, num2_lo, 0, 0, num1_hi, num1_lo);
	  /* num2 < 2^BITS_PER_MP_LIMB - num1, den1 <-- 1, den2 <-- -1;
	     Would normally set den2 to 0 and num2 to 2^BITS_PER_MP_LIMB
	     but start them here so we can use double precision.  */

	  for (;;)
	    {
	      denshift = den1;
	      numshift_hi = num1_hi;
	      numshift_lo = num1_lo;

	      if (high_bit > numshift_hi)
		{
		  mp_limb shift_index = 1;
		  while ((high_bit >>= 1) > numshift_hi)
		    shift_index++;
		  denshift <<= shift_index;
		  DP_LSHIFT (numshift_hi, numshift_lo, shift_index);
		}

	      do
		{
		  if (DP_GT (num2_hi, num2_lo, numshift_hi, numshift_lo))
		    {
		      den2 -= denshift;
		      sub_ddmmss (num2_hi, num2_lo, num2_hi, num2_lo, numshift_hi, numshift_lo);
		    }
		  denshift >>= 1;
		  DP_DIV2 (numshift_hi, numshift_lo);
		}
	      while (DP_GT (num2_hi, num2_lo, num1_hi, num1_lo));

	      if (num2_hi == 0)
		{
		  a = -den2;
		  b = num2_lo;
		  add = !0;
		  break;
		}

	      /* Need to add denshift == (-den2<<i)  */

	      denshift = -den2;
	      numshift_hi = num2_hi;
	      numshift_lo = num2_lo;

	      if (high_bit > numshift_hi)
		{
		  mp_limb shift_index = 1;
		  while ((high_bit >>= 1) > numshift_hi)
		    shift_index++;
		  denshift <<= shift_index;
		  DP_LSHIFT (numshift_hi, numshift_lo, shift_index);
		}

		do
		  {
		    if (DP_GT (num1_hi, num1_lo, numshift_hi, numshift_lo))
		      {
			den1 += denshift;
			sub_ddmmss (num1_hi, num1_lo, num1_hi, num1_lo, numshift_hi, numshift_lo);
		      }
		    denshift >>= 1;
		    DP_DIV2 (numshift_hi, numshift_lo);
		  }
	      while (DP_GT (num1_hi, num1_lo, num2_hi, num2_lo));

	      if (num1_hi == 0)
		{
		  a = den1;
		  b = num1_lo;
		  add = 0;
		  break;
		}
	    }
	}
      else					/* num1_hi == 0  */
	{
	  a = 1;
	  b = num1_lo;
	  add = 0;
	}
    }

  /* Now U <-- |a*U - b*V|/2^s  */
  {
    mp_ptr orig_u = u;
    mp_limb borrow = 0;
    mp_limb hi_u = (a == 1) ? 0 : mpn_mul_1 (u, u, ul, a);

    /* U <-- |U +/- b*V|  */
    if (add)
      {
	mp_limb cy;

	cy = (b == 1) ? mpn_add_n (u, u, v, vl) : mpn_addmul_1 (u, v, vl, b);

	if (ul > vl) 			/* vl <= ul <= vl + 1  */
	  cy = ((u[vl] += cy) < cy);
	hi_u += cy;
	if (hi_u < cy)
	  u[ul++] = hi_u, u[ul++] = 1;
	else if (hi_u != 0)
	  u[ul++] = hi_u;
	/* Remove low zero limbs.  U guaranteed nonzero.  */
	for (u += 2, ul -= 2; *u == 0; u++) ul--;
      }
    else
      {
	borrow = (b == 1) ? mpn_sub_n (u, u, v, vl) : mpn_submul_1 (u, v, vl, b);

	if (ul > vl) 			/* vl <= ul <= vl + 1  */
	  {
	    mp_limb tmp = u[vl];
	    u[vl] = tmp - borrow;
	    borrow = (tmp < borrow);
	  }
	u[ul++] = hi_u - borrow;
	borrow = (hi_u < borrow);

	/* Remove low zero limbs and normalize U.  U may be negative
	   or zero.  At this point ul must be >= 2.  */

	u[ul] = 1;			/* Pseudodata to end loop.  */

	for (u += 2, ul -= 2; *u == 0; u++)
	  ul--;

	if (borrow)			/* U negative.  */
	  {
	    do
	      ul--;
	    while (u[ul] == NEG1);	/* Note that u[-1] == 0.  */
	    if (ul == -1)
	      {
		orig_u[0] = 1;
		return 1;		/* return abs(-1)  */
	      }
	  }
	else if (ul)			/* U positive.  */
	  {
	    do
	      ul--;
	    while (u[ul] == 0);
	  }
	else				/* U zero.  */
	  return 0;

	ul++;
      }

    if (*u & 1)
      {
	if (borrow)			/* Twos complement.  */
	  MPN_COMPL (orig_u, u, ul);
	else
	  MPN_COPY (orig_u, u, ul);
      }
    else
      {
	unsigned int rshift;
	count_trailing_zeros (rshift, *u);
	if (borrow)
	  mpn_compl_rsh (orig_u, u, ul, rshift);
	else
	  mpn_rshift (orig_u, u, ul, rshift);
	if (orig_u[ul-1] == 0)
	  ul--;
      }

    return ul;
  }
}

/**************************************************************************/

/* Return 1 if (u, ul) >> n has more bits than (v, vl) and 0 otherwise.
   Preconditions:
   1. 0 <= n < BITS_PER_MP_LIMB
   2. vl > 0.

   Note: ul < 0 is valid input and is treated as if ul == 0.
*/

static inline int
more_than_n_bits_more (u, ul, v, vl, n)
     mp_srcptr u;
     mp_size_t ul;
     mp_srcptr v;
     mp_size_t vl;
     int n;
{
  mp_limb ushift_hi;

  if (ul <= 0)
    return 0;

  ushift_hi = u[ul-1] >> n;

  if (ushift_hi == 0)
    {
      ushift_hi = u[ul-1] << (BITS_PER_MP_LIMB-n);
      ul--;
    }

  if (ul == vl)
    {
      mp_limb vhi = v[vl-1];
      return (ushift_hi > vhi && (ushift_hi^vhi) > vhi);
    }

  return ul > vl;
}

/***********************************************************************/

/* External interface for routine.  See top of file for description.  */

mp_size_t
mpn_accelgcd (g, u, ul, v, vl)
     mp_ptr g;
     mp_ptr u;
     mp_size_t ul;
     mp_ptr v;
     mp_size_t vl;
{
  mp_ptr sav_u;
  mp_ptr sav_v;
  mp_size_t sav_ul;
  mp_size_t sav_vl;

#ifdef TIME
  gl_inter = 0;
#endif TIME

  /* Do initial gcd-preserving reductions to get u, v about the same size  */
  do
    {
      ul = mpn_bmod (u, ul, v, vl);
      if (ul == 0)
	{
	  if (v != g)
	    MPN_COPY (g, v, vl);
	  return vl;
	}
      MPN_SWAP (u, ul, v, vl);
    }
  while (more_than_n_bits_more (u, ul, v, vl, BMOD_THRESHOLD (vl)));

  /* If vl is too small for accelgcd to be practical, defer to bmodgcd.
     Note that this step must weed out any cases where vl < 2.  */
  if (vl <= ACCEL_THRESHOLD)
    return mpn_bmodgcd (g, u, ul, v, vl);

  /* If the size of the operands is large enough, attempt k-ary reduction;
     if first try reduces to zero, just use bmod reduction.  It should go
     fairy quicky because failure indicates that the true gcd is very close
     in size to the size of (v, vl).  For the commonplace situation where
     gcd(u, v) == v, it is important to eliminate the extra step of
     computing gcd(original u, gcd(original v, v)).

     Note also that there are at least ul + 2 limbs allocated at u AND v;
     this is a precondition for kary_reduce.  */

  sav_u = u;
  sav_ul = ul;
  u = (mp_ptr) alloca (sizeof (mp_limb)*(ul + 2));
  MPN_COPY (u, sav_u, ul);

  ul = kary_reduce (u, ul, v, vl);

  if (ul == 0)
    return mpn_bmodgcd (g, sav_u, sav_ul, v, vl);

  /* Allocate sav_ul limbs at v to satisfy precondition mentioned above.  */
  sav_v = v;
  sav_vl = vl;
  v = (mp_ptr) alloca (sizeof (mp_limb) * (sav_ul + 2));
  MPN_COPY (v, sav_v, vl);

  while (ul > 1)		/* can't handle smaller than 2  */
    {
      MPN_SWAP (u, ul, v, vl);
      /* Now Numbits(u, ul) > Numbits(v, vl)  */
      ul = (more_than_n_bits_more (u, ul, v, vl, BMOD_THRESHOLD (vl)))
	? mpn_bmod (u, ul, v, vl) : kary_reduce (u, ul, v, vl);
    }

  if (ul > 0)
    {
      vl = ul;
      v = u;	/* take smallest  */
    }

#ifdef TIME
  gl_inter = vl;
#endif TIME

  return mpn_bmodgcd (g, sav_v, sav_vl, v, mpn_bmodgcd (v, sav_u, sav_ul, v, vl));
}
