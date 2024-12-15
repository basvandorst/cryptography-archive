/* mpz_divmod_floor -- DIVision and MODulo rounding the quotient towards
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

void
#if __STDC__
mpz_divmod_floor (mpz_ptr quot, mpz_ptr rem,
	     mpz_srcptr dividend, mpz_srcptr divisor)
#else
mpz_divmod_floor (quot, rem, dividend, divisor)
     mpz_ptr quot;
     mpz_ptr rem;
     mpz_srcptr dividend;
     mpz_srcptr divisor;
#endif
{
  mp_size_t dividend_size = dividend->size;

  /* We need the original value of the divisor after the quotient and
     remainder have been preliminary calculated.  We have to copy it to
     temporary space if it's the same variable as either QUOT or REM.  */
  if (quot == divisor || rem == divisor)
    {
      mpz_t temp_divisor;

      MPZ_TMP_INIT (temp_divisor, ABS (divisor->size));
      mpz_set (temp_divisor, divisor);
      divisor = temp_divisor;
    }

  mpz_divmod_trunc (quot, rem, dividend, divisor);

  if ((divisor->size ^ dividend_size) < 0 && rem->size != 0)
    {
      mpz_sub_ui (quot, quot, 1);
      mpz_add (rem, rem, divisor);
    }
}
