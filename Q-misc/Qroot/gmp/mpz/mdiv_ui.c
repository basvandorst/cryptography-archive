/* mpz_mdiv_ui -- Mathematical DIVision and MODulo, i.e. division that rounds
   the quotient towards -infinity.

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
mpz_mdiv_ui (mpz_ptr quot,
	     mpz_srcptr dividend, unsigned long int divisor)
#else
mpz_mdiv_ui (quot, dividend, divisor)
     mpz_ptr quot;
     mpz_srcptr dividend;
     unsigned long int divisor;
#endif
{
  MP_INT rem;

  MPZ_TMP_INIT (&rem, 1 + ABS (dividend->size));

  mpz_divmod_ui (quot, &rem, dividend, divisor);
  if (rem.size < 0)
    mpz_sub_ui (quot, quot, 1);
}
