/* mpn_mod_1(dividend_ptr, dividend_size, divisor_limb) --
   Divide (DIVIDEND_PTR,,DIVIDEND_SIZE) by DIVISOR_LIMB.
   Return the single-limb remainder.
   There are no constraints on the value of the divisor.

   QUOT_PTR and DIVIDEND_PTR might point to the same limb.

Copyright (C) 1991, 1993 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU MP Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU MP Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#ifndef UMUL_TIME
#define UMUL_TIME 1
#endif

#ifndef UDIV_TIME
#define UDIV_TIME UMUL_TIME
#endif

#if UDIV_TIME > 2 * UMUL_TIME
#undef UDIV_NEEDS_NORMALIZATION
#define UDIV_NEEDS_NORMALIZATION 1
#endif

mp_limb
#if __STDC__
mpn_mod_1 (mp_srcptr dividend_ptr, mp_size_t dividend_size,
	   unsigned long int divisor_limb)
#else
mpn_mod_1 (dividend_ptr, dividend_size, divisor_limb)
     mp_srcptr dividend_ptr;
     mp_size_t dividend_size;
     unsigned long int divisor_limb;
#endif
{
  mp_size_t i;
  mp_limb n1, n0, r;
  int dummy;

  /* Botch: Should this be handled at all?  Rely on callers?  */
  if (dividend_size == 0)
    return 0;

  if (UDIV_NEEDS_NORMALIZATION)
    {
      int normalization_steps;

      count_leading_zeros (normalization_steps, divisor_limb);
      if (normalization_steps != 0)
	{
	  divisor_limb <<= normalization_steps;

	  n1 = dividend_ptr[dividend_size - 1];
	  r = n1 >> (BITS_PER_MP_LIMB - normalization_steps);

	  /* Possible optimization:
	     if (r == 0
	     && divisor_limb > ((n1 << normalization_steps)
			     | (dividend_ptr[dividend_size - 2] >> ...)))
	     ...one division less... */

	  /* If multiplication is much faster than division, and the
	     dividend is large, pre-invert the divisor, and use
	     only multiplications in the inner loop.  */
	  if (UDIV_TIME > 2 * UMUL_TIME && dividend_size >= 4)
	    {
	      mp_limb divisor_limb_inverted;
	      int dummy;
	      /* Compute (2**64 - 2**32 * DIVISOR_LIMB) / DIVISOR_LIMB.
		 The result is an 33-bit approximation to 1/DIVISOR_LIMB,
		 with the most significant bit (with weight 2**32)
		 implicit.  */

	      /* Special case for DIVISOR_LIMB == 100...000.  */
	      if (divisor_limb << 1 == 0)
		divisor_limb_inverted = ~(mp_limb) 0;
	      else
		udiv_qrnnd (divisor_limb_inverted, dummy,
			    -divisor_limb, 0, divisor_limb);

	      for (i = dividend_size - 2; i >= 0; i--)
		{
		  n0 = dividend_ptr[i];
		  udiv_qrnnd_preinv (dummy, r, r,
				     ((n1 << normalization_steps)
				      | (n0 >> (BITS_PER_MP_LIMB - normalization_steps))),
				     divisor_limb, divisor_limb_inverted);
		  n1 = n0;
		}
	      udiv_qrnnd_preinv (dummy, r, r,
			  n1 << normalization_steps,
			  divisor_limb, divisor_limb_inverted);
	      return r >> normalization_steps;
	    }
	  else
	    {
	      for (i = dividend_size - 2; i >= 0; i--)
		{
		  n0 = dividend_ptr[i];
		  udiv_qrnnd (dummy, r, r,
			      ((n1 << normalization_steps)
			       | (n0 >> (BITS_PER_MP_LIMB - normalization_steps))),
			      divisor_limb);
		  n1 = n0;
		}
	      udiv_qrnnd (dummy, r, r,
			  n1 << normalization_steps,
			  divisor_limb);
	      return r >> normalization_steps;
	    }
	}
    }

  /* No normalization needed, either because udiv_qrnnd doesn't require
     it, or because DIVISOR_LIMB is already normalized.  */

  i = dividend_size - 1;
  r = dividend_ptr[i];

  if (r >= divisor_limb)
    {
      r = 0;
    }
  else
    {
      i--;
    }

  for (; i >= 0; i--)
    {
      n0 = dividend_ptr[i];
      udiv_qrnnd (dummy, r, r, n0, divisor_limb);
    }
  return r;
}
