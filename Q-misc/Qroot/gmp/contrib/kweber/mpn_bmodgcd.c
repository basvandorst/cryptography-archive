/*	File mpn_bmodgcd.c

	A gcd based on the bmod operation and then binary reduction
	once the size of the smaller reductand drops below BINARY_THRESHOLD.

	When compiled with the macro MPN_BINGCD defined, it produces

	    mpn_bingcd(g, u, ul, v, vl)

	which only uses binary reduction.

	Preconditions:

	1)  (v, vl) is odd.
	2)   *u != 0.
	3)   ul >= vl > 0.

	Ken Weber (kweber@mcs.kent.edu)		06/08/93

	Refer to:

	    K. Weber, The accelerated integer gcd algorithm, 
	    Dept. of Mathematics and Computer Science, 
	    Kent State University, 1993.
*/

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"


/* BINARY_THRESHOLD tells when the binary algorithm
   should take over from the bmod algorithm.  */

#ifndef BINARY_THRESHOLD
#define BINARY_THRESHOLD 12
#endif

/*  If ul - vl > SIZE_DIFF_THRESHOLD go ahead and use bmod reduction.  */
#define SIZE_DIFF_THRESHOLD 1

#define NEG1 (-(mp_limb)1)

mp_size_t
#ifdef MPN_BINGCD
mpn_bingcd(mp_ptr g, mp_ptr u, mp_size_t ul, mp_ptr v, mp_size_t vl)
{
#else	/*  !defined MPN_BINGCD  */
mpn_bmodgcd(mp_ptr g, mp_ptr u, mp_size_t ul, mp_ptr v, mp_size_t vl)
{
  if (vl > BINARY_THRESHOLD || ul - vl > SIZE_DIFF_THRESHOLD)
    do
      {
	ul = mpn_bmod (u, ul, v, vl);
	if (ul == 0)
	  {
	    if (g != v)
	      MPN_COPY (g, v, vl);
	    return vl;
	  }
	  MPN_SWAP (u, ul, v, vl);
      }   while (vl > BINARY_THRESHOLD);

  /*  Now go into binary algorithm  */

#endif	/*  MPN_BINGCD */

  while (ul > 2)		/*  Bigger than double precision  */
    {
      mp_size_t zeros = 0;
      mp_size_t vl2 = vl;
      mp_limb lowest;

      /*  Find lowest nonequal limb  */
      while ((lowest = u[zeros] - v[zeros]) == 0)
	{
	  zeros += 1;
	  if (zeros == vl2)
	    {
	      if (ul == vl2)
		{
		  if (g != v && g != u)
		    MPN_COPY (g, v, vl);
		  return vl;
		}
	      while ((lowest = u[zeros]) == 0)
		zeros += 1;
	      break;
	    }
	}

      /*  Find highest nonequal limb.  Swap if U < V.  */
      if (ul == vl2)
	{
	  do vl2--; while (u[vl2] == v[vl2]);
	  if (u[vl2] < v[vl2])
	    {mp_ptr t; t = u, u = v, v = t;}
	  ul = vl2 += 1;
	}

      u += zeros;
      ul -= zeros;
      vl2 -= zeros;

      if (lowest & 1)				/*  No shift--just subtract.  */
	{
	  if (vl2 > 0 && mpn_sub_n(u, u, v + zeros, vl2) != 0)
	    {
	      mp_ptr cur_u = u + vl2;		/*  Satisfy borrow.  */
	      while (NEG1 == (*cur_u -= 1)) 
		cur_u++;
	    }
	}
      else					/*  Shift required.  */
	{
	  unsigned int rshift;
	  count_trailing_zeros (rshift, lowest);

	  /*  Since shift out and borrow are combined for mpn_sub_n_rsh,
	      and the shift out here is always zero, the result will be 
	      nonzero iff a borrow occurred.  */
	  if (vl2 <= 0)			/*  No subtract.  */
	    mpn_rshift (u, u, ul, rshift);
	  else if (mpn_sub_n_rsh (u, u, v + zeros, vl2, rshift))
	    {
	      mp_ptr cur_u = u + vl2;		/*  Satisfy borrow.  */
	      mp_limb this = *cur_u - 1;
	      unsigned int lshift = BITS_PER_MP_LIMB - rshift;
	      cur_u[-1] += (this << lshift);
	      if (this == NEG1)
		{
		  while ((this = *++cur_u) == 0)
		    cur_u[-1] = NEG1;
		  this -= 1;
		  cur_u[-1] = (this << lshift) + (NEG1 >> rshift);
		}
	      this >>= rshift;
	      cur_u += 1;
	      if (u + ul - cur_u > 0)
		this += mpn_rshift (cur_u, cur_u, u + ul - cur_u, rshift);
	      cur_u[-1] = this;
	    }
	  else if (ul > vl2)			/*  No borrow.  */
	    u[vl2-1] += mpn_rshift (u + vl2, u + vl2, ul - vl2, rshift);
	}

      MPN_NORMALIZE_NOT_ZERO (u, ul);

      if (ul < vl)
	MPN_SWAP (u, ul, v, vl);
  }	/*  End general binary gcd loop  */

  /*  Double precision binary  */
  {
    mp_limb u0 = u[0];
    mp_limb v0 = v[0];

    if (ul == 2 || vl == 2)
      {
	mp_limb u1 = 0;
	mp_limb v1 = 0;
	if (ul == 2)
	  u1 = u[1];
	if (vl == 2)
	  v1 = v[1];
	for (;;)
	  {
	    if (u1 == v1 && u0 == v0)
	      {
		g[0] = v0;
		return v1 ? (g[1] = v1, 2) : 1;
	      }
	    if (u1 > v1 || (u1 == v1 && u0 > v0))
	      {
		sub_ddmmss (u1, u0, u1, u0, v1, v0);
		if ((u1) && (u0))
		  {
		    unsigned int rshift;
		    count_trailing_zeros (rshift, u0);
		    u0 = (u1 << (BITS_PER_MP_LIMB - rshift)) | (u0 >> rshift);
		    u1 >>= rshift;
		    if (u1 == 0 && v1 == 0)
		      break;
		  }
		else 
		  {
		    if (u0 == 0)
		      {
			u0 = u1;
			u1 = 0;
		      }
		    while ((u0 & 1) == 0)
		      u0 >>= 1;
		    if (v1 == 0)
		      break;
		  }
	      }
	    else
	      {
		sub_ddmmss (v1, v0, v1, v0, u1, u0);
		  if ((v1) && (v0))
		    {
		      unsigned int rshift;
		      count_trailing_zeros (rshift, v0);
		      v0 = (v1 << (BITS_PER_MP_LIMB - rshift)) | (v0 >> rshift);
		      v1 >>= rshift;
		      if (u1 == 0 && v1 == 0)
			break;
		    }
		  else 
		    {
		      if (v0 == 0)
			{
			  v0 = v1;
			  v1 = 0;
			}
		      while ((v0 & 1) == 0)
			v0 >>= 1;
		      if (u1 == 0)
			break;
		    }
	      }
	  }
      }

      /*  Single precision binary  */
    while (u0 != v0)
      if (u0 > v0)
	{
	  u0 -= v0;
	  while ((u0 & 1) == 0)
	    u0 >>= 1;
	}
      else
	{
	  v0 -= u0;
	  while ((v0 & 1) == 0)
	    v0 >>= 1;
	}

    g[0] = v0;

    return 1;
  }
}

/*  End file mpn_bmodgcd.c  */
