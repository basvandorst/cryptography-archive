/*	File mpn_bmod.c

	The bmod ("bit modulus") operation.

	    U <-- |U - (U/V mod 2^{d+1})*V|/2^t

	where U == (u,ul), V == (v,vl), 
	d = max(0, NumBits(u,ul) - NumBits(v,vl)),
	and 2^t exactly divides U - (U/V mod 2^{d+1})*V

	(The representation of the pair (x, xl) by the upper case X is used
	throughout the comments in this file.)

	Preconditions:

	1)  V is odd.
	2)  u[0] != 0.
	3)  ul >= vl > 0.

	Postcondition: U <-- |U - (U/V mod 2^(d+1))*V|/2^s

	Note that this postcondition implies:
	1.  U < V
	2.  gcd is preserved.
	3.  U is odd.

	When called from mpn_bmodgcd, it is often the case that
	(U/V mod 2^{d+1}) is a small, odd, positive integer,
	so that special-case multiplication makes sense. 
	See the macro SC_MUL_SWITCH below.

	Ken Weber (kweber@mcs.kent.edu)		07/15/93

	Refer to:
	    K. Weber, The accelerated integer gcd algorithm,
	    Dept. of Mathematics and Computer Science,
	    Kent State University, 1993.
*/

#ifdef TEST
#include <stdio.h>
#endif

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

/******************************************************************************/

/*  Used in shifting limb vectors.  Gives the shift value for the other part
    of the current limb.
*/
#define OPP_SHIFT(S) (BITS_PER_MP_LIMB-(S))

/*  True iff Numbits(U, L) >= Numbits(V, M)  */
#define AT_LEAST_BITS_PER_MP_LIMB_MORE_BITS(U, L, V, M) \
    (!MPN_LESS_BITS((U)+1, (L)-1, V, M))

/*  Twos complement representation of -1 in a limb.  */
#define NEG1	(-(mp_limb)1)

/*  The following macros, prefixed by SC_, are used to implement special case
    multiplication.
*/

/*  True iff X is a candidate for special case multiplication  */
#define SC(X) ((X) < (mp_limb)(1<<(BITS_PER_MP_LIMB/2)))

/*  (HI, LO) <-- (Y << S) + Y.  */
#define SC_SH_ADD(HI, LO, Y, S)\
    do			\
    {   mp_limb _ = (Y);\
	int __ = (S);	\
	add_ssaaaa(HI, LO, _>>OPP_SHIFT(__), _<<__, 0, _);\
    }   while (0)

/*  (HI, LO) <-- (Y << S) - Y.  */
#define SC_SH_SUB(HI, LO, Y, S)\
    do			\
    {   mp_limb _ = (Y);\
	int __ = (S);	\
	sub_ddmmss(HI, LO, _>>OPP_SHIFT(__), _<<__, 0, _);\
    }   while (0)

/*  (HI, LO) <-- (Y << S1) + (Y << S2) + Y.  */
#define SC_SH_ADD_SH_ADD(HI, LO, Y, S1, S2)\
    do			\
    {   mp_limb _ = (Y);\
	int __ = (S1); 	\
	mp_limb ___;	\
	mp_limb ____;	\
	add_ssaaaa(___, ____, _>>OPP_SHIFT(__),  _<<__,  0, _);	\
	__ = (S2);	\
	add_ssaaaa(HI, LO, _>>OPP_SHIFT(__), _<<__, ___, ____);	\
    }   while (0)

/*  (HI, LO) <-- (Y << S1) - (Y << S2) + Y.  */
#define SC_SH_ADD_SH_SUB(HI, LO, Y, S1, S2)\
    do			\
    {   mp_limb _ = (Y);\
	int __ = (S1); 	\
	mp_limb ___;	\
	mp_limb ____;	\
	add_ssaaaa(___, ____, _>>OPP_SHIFT(__),  _<<__,  0, _);	\
	__ = (S2);	\
	sub_ddmmss(HI, LO, _>>OPP_SHIFT(__), _<<__, ___, ____);	\
    }   while (0)


/*  Do "special case" multiplication inside a code segment.
    
	C1 is the code segment to supply if u_v == 1.

	General case:
	CP (Code Prefix)
	(HI, LO) <-- X*Y
	CS (Code Suffix)

	RC is the code segment to supply if u_v is not a special case.
*/
#ifdef __sparc__
#define SC_MUL_SWITCH(C1, CP, HI, LO, X, Y, CS, RC)			\
    do									\
    {	switch (X)							\
	{   case  1: C1;					   break;\
	    case  3: CP	SC_SH_ADD	(HI, LO, Y, 1);		CS break;\
	    case  5: CP	SC_SH_ADD	(HI, LO, Y, 2);		CS break;\
	    case  7: CP	SC_SH_SUB	(HI, LO, Y, 3);		CS break;\
	    case  9: CP	SC_SH_ADD	(HI, LO, Y, 3);		CS break;\
	    case 11: CP SC_SH_ADD_SH_ADD(HI, LO, Y, 1, 3);	CS break;\
	    case 13: CP SC_SH_ADD_SH_ADD(HI, LO, Y, 2, 3);	CS break;\
	    case 15: CP	SC_SH_SUB	(HI, LO, Y, 4);		CS break;\
	    case 17: CP	SC_SH_ADD	(HI, LO, Y, 4);		CS break;\
	    case 19: CP SC_SH_ADD_SH_ADD(HI, LO, Y, 1, 4);	CS break;\
	    case 21: CP SC_SH_ADD_SH_ADD(HI, LO, Y, 2, 4);	CS break;\
	    case 23: CP SC_SH_ADD_SH_SUB(HI, LO, Y, 3, 5);	CS break;\
	    case 25: CP SC_SH_ADD_SH_ADD(HI, LO, Y, 3, 4);	CS break;\
	    case 27: CP SC_SH_ADD_SH_SUB(HI, LO, Y, 2, 5);	CS break;\
	    case 29: CP SC_SH_ADD_SH_SUB(HI, LO, Y, 1, 5);	CS break;\
	    case 31: CP	SC_SH_SUB	(HI, LO, Y, 5);		CS break;\
	    case 33: CP	SC_SH_ADD	(HI, LO, Y, 5);		CS break;\
	    default: RC;						\
	}								\
    }   while (0)
#else /*  !__sparc__  */
#define SC_MUL_SWITCH(C1, CP, HI, LO, X, Y, CS, RC)			\
    do									\
    {	switch (X)							\
	{   case  1: C1;					   break;\
	    default: RC;						\
	}								\
    }   while (0)
#endif /* !__sparc__ */

/******************************************************************************/

/*  R <-- |U - V| >> s where s is sufficient to make result odd. 

    Precondition: vl + 1 >= ul >= vl.
*/
static mp_size_t
abs_sub_shift (r, u, ul, v, vl)
     mp_ptr r;
     mp_srcptr u;
     mp_size_t ul;
     mp_srcptr v;
     mp_size_t vl;
{
  mp_limb lowest, borrow;
  unsigned long int rshift;

  /*  Find lowest nonequal limb.  If U == V it will be found here.  */
  {
    mp_size_t zeros = 0;
    while (u[zeros] == v[zeros])
      {
	zeros += 1;
	if (zeros == vl)
	  {
	    if (ul == vl) return 0;
	    /*  When ul == vl + 1, u[vl] != 0.  */
	    lowest = u[zeros];
	    count_trailing_zeros (rshift, lowest);
	    r[0] = lowest >> rshift;
	    return 1;
	  }
      }
    u += zeros;
    ul -= zeros;
    v += zeros;
    vl -= zeros;
  }

  /*  Find highest nonequal limb.  Swap if U < V.  */
  if (ul == vl)
    {
      do vl -= 1; while (u[vl] == v[vl]);
      if (u[vl] < v[vl])
	{
	  mp_srcptr t;
	  t = u;
	  u = v;
	  v = t;
	}
      ul = vl += 1;
    }

  lowest = u[0] - v[0];
  if (lowest & 1)				/*  No shift--just subtract.  */
    {
      borrow = mpn_sub_n (r, u, v, vl);
      if (ul > vl)
	r[vl] = u[vl] - borrow;	/*  Borrow always satisfied.  */
    }
  else					/*  Shift required.  */
    {
      count_trailing_zeros (rshift, lowest);

      /*  The borrow out is in the low bit of the result.
	  Since the high bits of the result will be 0 here, the result
	  will be the borrow.  */
      borrow = mpn_sub_n_rsh (r, u, v, vl, rshift);
      if (ul > vl)
	{
	  borrow = u[vl] - borrow;
	  r[vl-1] |= borrow << (BITS_PER_MP_LIMB - rshift);
	  r[vl] = borrow >> rshift;
	}
    }

  MPN_NORMALIZE_NOT_ZERO (r, ul);
  return ul;
}

/******************************************************************************/

/*  External interface.  See beginning of file for description of routine.  */

mp_size_t
mpn_bmod (u, ul, v, vl)
     mp_ptr u;
     mp_size_t ul;
     mp_srcptr v;
     mp_size_t vl;
{
  mp_limb v_inv = 0;  
  mp_ptr orig_u = u;

  if (AT_LEAST_BITS_PER_MP_LIMB_MORE_BITS(u, ul, v, vl))
    {
      /* We do not shift within limbs here because to do so would require
	 all of (u,ul) to be shifted. If ul were much bigger than vl, this
	 would be very expensive.  */
      /*  First calculate v_inv  */
      {
	mp_limb make_zero = 1;
	mp_limb vshift = v[0];
	mp_limb two_i = 1;
	do
	  {
	    while ((two_i&make_zero) == 0)
	      {
		two_i <<= 1;
		vshift <<= 1;
	      }
	    v_inv += two_i;
	    make_zero -= vshift;
	  }
	while (make_zero);
      }

      /*  Compute ((u,ul) - u_v*(v,vl)) >> (s*BITS_PER_MP_LIMB).  */
      do
	{
	  mp_limb borrow = mpn_submul_1 (u, v, vl, u[0] * v_inv);
	  mp_ptr cur_u = u + vl;
	  mp_ptr u_end = u + ul;

	    /* Subtract borrow out from u[vl].  */
	  {
	    mp_limb this_u = *cur_u;
	    *cur_u++ = this_u - borrow;
	    borrow = (this_u < borrow);
	  }

	  if (borrow && cur_u != u_end)
	    {
	      while (*cur_u == 0)
		*cur_u++ = NEG1;
	      *cur_u -= 1; 			/*  Borrow is satisfied.  */
	      borrow = 0;
	    }

	  /*  Eliminate low zero limbs.  There is at least one.
	      If result is zero it will be found here.
	      If result is all zero limbs and borrow, 1 is returned.  */
	  do
	    {
	      if (++u == u_end)
		return *orig_u = borrow;
	    }
	  while (*u == 0);

	  if (borrow)				/*  Result < 0--complement.  */
	    {
	      do 
		u_end--; 
	      while (*u_end == NEG1);		/*  Still a 0 at u[-1].  */

	      u_end += 1;
	      if (u_end == u)
		return *orig_u = 1;
	      ul = u_end - u;
	      MPN_COMPL (u, u, ul);
	    }
	  else
	    {
	      ul = u_end - u;
	      MPN_NORMALIZE_NOT_ZERO (u, ul);
	    }
	    
	}
      while (AT_LEAST_BITS_PER_MP_LIMB_MORE_BITS (u, ul, v, vl));
    }

  if (MPN_MORE_BITS (u, ul, v, vl))
    /*  BITS_PER_MP_LIMB > Numbits(u,ul) - Numbits(v,vl) > 0  */
    {
      int rshift = 0;
      mp_limb mask = NEG1;
      mp_limb uhi_bits = u[vl-1];
      /*  Low bit of uhi_bits will correspond to highest bit of (v,vl).  */

      if (v[vl-1] != 1)
	{
	  unsigned long int vlz;
	  count_leading_zeros (vlz, v[vl-1]);
	  rshift = BITS_PER_MP_LIMB - 1 - vlz;
	  uhi_bits >>= rshift;
	  if (ul > vl) uhi_bits += u[ul-1] << OPP_SHIFT (rshift);
	}

      {
	unsigned long int ulz;
	count_leading_zeros(ulz, uhi_bits);
	mask >>= ulz;
      }

      if (mask&*u)
	{
	  mp_limb u_v, borrow;

	  if (v_inv)
	    u_v = (*u*v_inv) & mask;
	  else /*  Compute u/v (mod 2^(d+1)) in single loop.  */
	    {
	      mp_limb make_zero = *u;
	      mp_limb vshift = *v;
	      mp_limb two_i = 1;
	      u_v = 0;
	      do
		{
		  while ((two_i & make_zero) == 0) 
		    {
		      two_i <<= 1;
		      vshift <<= 1;
		    }
		  u_v += two_i;
		  make_zero -= vshift;
		}
	      while (make_zero & mask);
	    }

	  /*  Now do the multiplication.  */

	  SC_MUL_SWITCH
	    (
	     return abs_sub_shift (orig_u, u, ul, v, vl),
	     {
	       mp_ptr cur_u = u;
	       mp_srcptr cur_v = v;
	       mp_srcptr v_end = v + vl;
	       borrow = 0;
	       do
		 {
		   mp_limb this;
		   mp_limb this_u = *cur_u++;
		   mp_limb hi_prod;
		   mp_limb lo_prod = *cur_v++;
		   borrow = -borrow;
		   sub_ddmmss (borrow, this, 0, this_u, 0, borrow);,

		   hi_prod, lo_prod, u_v, lo_prod,	/*  multiplication args  */

		   sub_ddmmss (borrow, this, borrow, this, hi_prod, lo_prod);
		   cur_u[-1] = this;
		 }
	       while (cur_v != v_end);	/*  Try to keep pipe full.  */
	       borrow = -borrow;
	     },

	     borrow = mpn_submul_1 (u, v, vl, u_v)
	     );

	  /*  Subtract borrow out from u[vl] if ul == vl + 1.  */
	  if (ul > vl)
	    {
	      mp_limb this_u = u[vl];
	      u[vl] = this_u - borrow;
	      borrow = (this_u < borrow);
	    }

	  /*  At this point borrow = 0, 1, or 2.  */

	  /* Eliminate low zero limbs, if any.  If result is zero it will be
	     found here.  If result is all zero limbs and borrow, 1 is
	     returned.  */

	  while (*u == 0)
	    {
	      u++;
	      ul--;
	      if (ul == 0)
		{
		  *orig_u = borrow;
		  return borrow;
		}
	    }

	  if (borrow)			/*  Result < 0--complement.  */
	    {
	      do 
		ul--; 
	      while (u[ul] == NEG1);	/*  Still a 0 at u[-1].  */
	      ul++;
	      if (ul == 0)
		{
		  *orig_u = 1;
		  return 1;		/*  Works for 1 and 2.  */
		}
	      if (*u & 1)
		{
		  MPN_COMPL (orig_u, u, ul);
		  if (borrow == 2)
		    orig_u[ul++] = 1;
		}
	      else
		{
		  unsigned int rshift;
		  count_trailing_zeros (rshift, *u);
		  mpn_compl_rsh (orig_u, u, ul, rshift);
		  if (borrow == 2)
		    orig_u[ul-1] |= 1 << OPP_SHIFT (rshift);
		  if (orig_u[ul-1] == 0)
		    ul -= 1;
		}
	    }
	  else
	    {
	      MPN_NORMALIZE_NOT_ZERO (u, ul);
	      if (*u & 1)
		MPN_COPY (orig_u, u, ul);
	      else
		{
		  unsigned int rshift;
		  count_trailing_zeros (rshift, *u);
		  mpn_rshift (orig_u, u, ul, rshift);
		  if (orig_u[ul-1] == 0)
		    ul -= 1;
		}
	    }
	  return ul;
	}
    }

  /* When (u, ul) is odd and ul >= vl, a final subtract-shift is needed.  */
    if (*u & 1)
      {
	if (ul >= vl)
	  ul = abs_sub_shift (orig_u, u, ul, v, vl);
	else
	  MPN_COPY (orig_u, u, ul);
      }
    else				 /*  A shift only is indicated  */
      {
	unsigned int rshift;
	count_trailing_zeros (rshift, *u);
	mpn_rshift (orig_u, u, ul, rshift);
	if (orig_u[ul-1] == 0)
	  ul -= 1;
      }

  return ul;
}
