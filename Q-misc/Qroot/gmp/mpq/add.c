/* mpq_add -- add two rational numbers.

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

void
#if __STDC__
mpq_add (MP_RAT *sum, const MP_RAT *a1, const MP_RAT *a2)
#else
mpq_add (sum, a1, a2)
     MP_RAT *sum;
     const MP_RAT *a1;
     const MP_RAT *a2;
#endif
{
  MP_INT gcd;
  MP_INT tmp1, tmp2;
  mp_size_t a1_num_size = ABS (a1->num.size);
  mp_size_t a1_den_size = ABS (a1->den.size);
  mp_size_t a2_num_size = ABS (a2->num.size);
  mp_size_t a2_den_size = ABS (a2->den.size);

  MPZ_TMP_INIT (&gcd, MIN (a1_den_size, a2_den_size));
  MPZ_TMP_INIT (&tmp1, a1_num_size + a2_den_size);
  MPZ_TMP_INIT (&tmp2, a2_num_size + a1_den_size);

  /* SUM might be identical to either operand, so don't store the
     result there until we are finished with the input operands.  We
     dare to overwrite the numerator of SUM when we are finished
     with the numerators of A1 and A2.  */

  mpz_gcd (&gcd, &(a1->den), &(a2->den));
  if (gcd.size > 1 || gcd.d[0] != 1)
    {
      MP_INT t;

      mpz_div (&tmp1, &(a2->den), &gcd);
      mpz_mul (&tmp1, &(a1->num), &tmp1);

      mpz_div (&tmp2, &(a1->den), &gcd);
      mpz_mul (&tmp2, &(a2->num), &tmp2);

      MPZ_TMP_INIT (&t, MAX (ABS (tmp1.size), ABS (tmp2.size)) + 1);

      mpz_add (&t, &tmp1, &tmp2);
      mpz_div (&tmp1, &(a1->den), &gcd);
      mpz_gcd (&gcd, &t, &gcd);

      mpz_div (&(sum->num), &t, &gcd);

      mpz_div (&tmp2, &(a2->den), &gcd);
      mpz_mul (&(sum->den), &tmp1, &tmp2);
    }
  else
    {
      /* The common divisior is 1.  This is the case (for random input) with
	 probability 6/(pi**2).  */
      mpz_mul (&tmp1, &(a1->num), &(a2->den));
      mpz_mul (&tmp2, &(a2->num), &(a1->den));
      mpz_add (&(sum->num), &tmp1, &tmp2);
      mpz_mul (&(sum->den), &(a1->den), &(a2->den));
    }
}
