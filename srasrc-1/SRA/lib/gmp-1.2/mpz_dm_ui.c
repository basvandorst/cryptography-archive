/* mpz_divmod_ui(quot,rem,dividend,short_divisor) --
   Set QUOT to DIVIDEND / SHORT_DIVISOR
   and REM to DIVIDEND mod SHORT_DIVISOR.

Copyright (C) 1991 Free Software Foundation, Inc.

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

void
#if __STDC__
mpz_divmod_ui (MP_INT *quot, MP_INT *rem,
	       const MP_INT *dividend, unsigned long int divisor_limb)
#else
mpz_divmod_ui (quot, rem, dividend, divisor_limb)
     MP_INT *quot;
     MP_INT *rem;
     const MP_INT *dividend;
     unsigned long int divisor_limb;
#endif
{
  mp_size sign_dividend;
  mp_size dividend_size, quot_size;
  mp_ptr dividend_ptr, quot_ptr;
  int normalization_steps;

  sign_dividend = dividend->size;
  dividend_size = abs (dividend->size);

  /* No need for temporary allocation and copying if QUOT == DIVIDEND as
     the divisor is just one limb, and thus no intermediate remainders
     need to be stored.  */

  if (quot->alloc < dividend_size)
    _mpz_realloc (quot, dividend_size);

  quot_ptr = quot->d;
  dividend_ptr = dividend->d;

  if (UDIV_NEEDS_NORMALIZATION)
    {
      count_leading_zeros (normalization_steps, divisor_limb);
      if (normalization_steps != 0)
	{
	  mp_limb carry_limb;
	  mp_ptr tp;

	  divisor_limb <<= normalization_steps;
	  tp = (mp_ptr) alloca ((dividend_size + 1) * BYTES_PER_MP_LIMB);

	  carry_limb = _mpn_lshift (tp, dividend_ptr, dividend_size,
				    normalization_steps);
	  if (carry_limb != 0)
	    {
	      tp[dividend_size] = carry_limb;
	      dividend_size++;
	    }

	  dividend_ptr = tp;
	}
    }

  {
    mp_size i;
    mp_limb n1, n0;

    i = dividend_size - 1;
    n1 = dividend_ptr[i];

    if (n1 >= divisor_limb)
      {
	n1 = 0;
	quot_size = dividend_size;
      }
    else
      {
	i--;
	quot_size = dividend_size - 1;
      }

    for (; i >= 0; i--)
      {
	n0 = dividend_ptr[i];
	udiv_qrnnd (quot_ptr[i], n1, n1, n0, divisor_limb);
      }

    if (n1 == 0)
      rem->size = 0;
    else
      {
	/* Store the single-limb remainder.  We don't check if there's space
	   for just one limb, since no function ever makes zero space.  */
	rem->size = sign_dividend >= 0 ? 1 : -1;
	if (UDIV_NEEDS_NORMALIZATION)
	  n1 >>= normalization_steps;
	rem->d[0] = n1;
      }
  }

  /* QUOT_SIZE is accurate without normalization.  */
  quot->size = sign_dividend >= 0 ? quot_size : -quot_size;
}
