/* mpz_div -- divide two integers and produce a quotient.

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
mpz_div (mpz_ptr quot, mpz_srcptr num, mpz_srcptr den)
#else
mpz_div (quot, num, den)
     mpz_ptr quot;
     mpz_srcptr num;
     mpz_srcptr den;
#endif
{
  mp_srcptr np, dp;
  mp_ptr qp, rp;
  mp_size_t nsize = num->size;
  mp_size_t dsize = den->size;
  mp_size_t qsize, rsize;
  mp_size_t sign_quotient = nsize ^ dsize;
  unsigned normalization_steps;
  mp_limb q_limb;

  nsize = ABS (nsize);
  dsize = ABS (dsize);

  /* Ensure space is enough for quotient. */

  qsize = nsize - dsize + 1;	/* qsize cannot be bigger than this.  */
  if (qsize <= 0)
    {
      quot->size = 0;
      return;
    }

  if (quot->alloc < qsize)
    _mpz_realloc (quot, qsize);

  qp = quot->d;
  np = num->d;
  dp = den->d;
  rp = (mp_ptr) alloca ((nsize + 1) * BYTES_PER_MP_LIMB);

  count_leading_zeros (normalization_steps, dp[dsize - 1]);

  /* Normalize the denominator and the numerator.  */
  if (normalization_steps != 0)
    {
      mp_ptr tp;
      mp_limb nlimb;

      /* Shift up the denominator setting the most significant bit of
	 the most significant word.  Use temporary storage not to clobber
	 the original contents of the denominator.  */
      tp = (mp_ptr) alloca (dsize * BYTES_PER_MP_LIMB);
      (void) mpn_lshift (tp, dp, dsize, normalization_steps);
      dp = tp;

      /* Shift up the numerator, possibly introducing a new most
	 significant word.  Move the shifted numerator in the remainder
	 meanwhile.  */
      nlimb = mpn_lshift (rp, np, nsize, normalization_steps);
      if (nlimb != 0)
	{
	  rp[nsize] = nlimb;
	  rsize = nsize + 1;
	}
      else
	rsize = nsize;
    }
  else
    {
      /* The denominator is already normalized, as required.
	 Copy it to temporary space if it overlaps with the quotient.  */
      if (dp == qp)
	{
	  dp = (mp_ptr) alloca (dsize * BYTES_PER_MP_LIMB);
	  MPN_COPY ((mp_ptr) dp, qp, dsize);
	}

      /* Move the numerator to the remainder.  */
      MPN_COPY (rp, np, nsize);
      rsize = nsize;
    }

  q_limb = mpn_divmod (qp, rp, rsize, dp, dsize);

  qsize = rsize - dsize;
  if (q_limb)
    {
      qp[qsize] = q_limb;
      qsize += 1;
    }

  quot->size = (sign_quotient >= 0) ? qsize : -qsize;
}
