/* mpz_divmod_ui(quot,rem,dividend,short_divisor) --
   Set QUOT to DIVIDEND / SHORT_DIVISOR
   and REM to DIVIDEND mod SHORT_DIVISOR.

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

void
#if __STDC__
mpz_divmod_ui (mpz_ptr quot, mpz_ptr rem,
	       mpz_srcptr dividend, unsigned long int divisor_limb)
#else
mpz_divmod_ui (quot, rem, dividend, divisor_limb)
     mpz_ptr quot;
     mpz_ptr rem;
     mpz_srcptr dividend;
     unsigned long int divisor_limb;
#endif
{
  mp_size_t sign_dividend;
  mp_size_t dividend_size, quot_size;
  mp_ptr dividend_ptr, quot_ptr;
  mp_limb remainder_limb;

  sign_dividend = dividend->size;
  dividend_size = ABS (dividend->size);

  if (dividend_size == 0)
    {
      quot->size = 0;
      rem->size = 0;
      return;
    }

  /* No need for temporary allocation and copying if QUOT == DIVIDEND as
     the divisor is just one limb, and thus no intermediate remainders
     need to be stored.  */

  if (quot->alloc < dividend_size)
    _mpz_realloc (quot, dividend_size);

  quot_ptr = quot->d;
  dividend_ptr = dividend->d;

  remainder_limb = mpn_divmod_1 (quot_ptr,
				 dividend_ptr, dividend_size, divisor_limb);

  if (remainder_limb == 0)
    rem->size = 0;
  else
    {
      /* Store the single-limb remainder.  We don't check if there's space
	 for just one limb, since no function ever makes zero space.  */
      rem->size = sign_dividend >= 0 ? 1 : -1;
      rem->d[0] = remainder_limb;
    }

  /* The quotient is DIVIDEND_SIZE limbs, but the most significant
     might be zero.  Set QUOT_SIZE properly. */
  quot_size = dividend_size - (quot_ptr[dividend_size - 1] == 0);
  quot->size = sign_dividend >= 0 ? quot_size : -quot_size;
}
