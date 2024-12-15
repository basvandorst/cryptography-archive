/* mpz_divmod_floor_ui -- DIVision and MODulo rounding the quotient towards
   -infinity.  The remainder gets the same sign as the denominator.

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

unsigned long int
#if __STDC__
mpz_divmod_floor_ui (mpz_ptr quot, mpz_ptr rem,
		     mpz_srcptr dividend, unsigned long int divisor)
#else
mpz_mdivmod_ui (quot, rem, dividend, divisor)
     mpz_ptr quot;
     mpz_ptr rem;
     mpz_srcptr dividend;
     unsigned long int divisor;
#endif
{
  mp_size_t dividend_size = dividend->size;
  MP_INT temp_rem;

  /* If the user doesn't want the remainder to be stored in an integer
     object, allocate a scratch variable for it.  This is useful since the
     remainder is always returned.  */
  if (rem == NULL)
    {
      MPZ_TMP_INIT (temp_rem, 1 + ABS (dividend->size));
      rem = temp_rem;
    }

  mpz_divmod_trunc_ui (quot, rem, dividend, divisor);

  if (dividend_size < 0 && rem->size != 0)
    {
      mpz_sub_ui (quot, quot, 1);
      mpz_add_ui (rem, rem, divisor);
    }

  if (rem->size == 0)
    return 0;
  return rem->d[0];
}
