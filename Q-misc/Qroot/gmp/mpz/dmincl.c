/* mpz_dmincl.c -- include file for mpz_dm.c, mpz_mod.c, mdiv.c.

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

/* If den == quot, den needs temporary storage.
   If den == rem, den needs temporary storage.
   If num == quot, num needs temporary storage.
   If den has temporary storage, it can be normalized while being copied,
     i.e no extra storage should be allocated.  */

/* This is the function body of mdiv, mpz_divmod, and mpz_mod.

   If COMPUTE_QUOTIENT is defined, the quotient is put in the MP_INT
   object quot, otherwise that variable is not referenced at all.

   The remainder is always computed, and the result is put in the MP_INT
   object rem.  */

{
  mp_ptr np, dp;
  mp_ptr qp, rp;
  mp_size_t nsize = num->size;
  mp_size_t dsize = den->size;
  mp_size_t qsize, rsize;
  mp_size_t sign_remainder = nsize;
#ifdef COMPUTE_QUOTIENT
  mp_size_t sign_quotient = nsize ^ dsize;
#endif
  unsigned normalization_steps;
  mp_limb q_limb;

  nsize = ABS (nsize);
  dsize = ABS (dsize);

  /* Ensure space is enough for quotient and remainder. */

  /* We need space for an extra limb in the remainder, because it's
     up-shifted (normalized) below.  */
  rsize = nsize + 1;
  if (rem->alloc < rsize)
    _mpz_realloc (rem, rsize);

  qsize = rsize - dsize;	/* qsize cannot be bigger than this.  */
  if (qsize <= 0)
    {
#ifdef COMPUTE_QUOTIENT
      quot->size = 0;
#endif
      if (num != rem)
	{
	  rem->size = num->size;
	  MPN_COPY (rem->d, num->d, nsize);
	}
      return;
    }

#ifdef COMPUTE_QUOTIENT
  if (quot->alloc < qsize)
    _mpz_realloc (quot, qsize);
#endif

  /* Read pointers here, when reallocation is finished.  */
  np = num->d;
  dp = den->d;
  rp = rem->d;

#ifdef COMPUTE_QUOTIENT
  qp = quot->d;

  /* Make sure QP and NP point to different objects.  Otherwise the
     numerator would be successively overwritten by the quotient limbs.  */
  if (qp == np)
    {
      /* Copy NP object to temporary space.  */
      np = (mp_ptr) alloca (nsize * BYTES_PER_MP_LIMB);
      MPN_COPY (np, qp, nsize);
    }

#else
  /* Put quotient at top of remainder. */
  qp = rp + dsize;
#endif

  count_leading_zeros (normalization_steps, dp[dsize - 1]);

  /* Normalize the denominator, i.e. make its most significant bit set by
     shifting it NORMALIZATION_STEPS bits to the left.  Also shift the
     numerator the same number of steps (to keep the quotient the same!).  */
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
      /* The denominator is already normalized, as required.  Copy it to
	 temporary space if it overlaps with the quotient or remainder.  */
#ifdef COMPUTE_QUOTIENT
      if (dp == rp || dp == qp)
#else
      if (dp == rp)
#endif
	{
	  mp_ptr tp;

	  tp = (mp_ptr) alloca (dsize * BYTES_PER_MP_LIMB);
	  MPN_COPY (tp, dp, dsize);
	  dp = tp;
	}

      /* Move the numerator to the remainder.  */
      if (rp != np)
	MPN_COPY (rp, np, nsize);

      rsize = nsize;
    }

  q_limb = mpn_divmod (qp, rp, rsize, dp, dsize);

#ifdef COMPUTE_QUOTIENT
  qsize = rsize - dsize;
  if (q_limb)
    {
      qp[qsize] = q_limb;
      qsize += 1;
    }

  quot->size = (sign_quotient >= 0) ? qsize : -qsize;
#endif

  rsize = dsize;
  MPN_NORMALIZE (rp, rsize);

  if (normalization_steps != 0 && rsize != 0)
    {
      mpn_rshift (rp, rp, rsize, normalization_steps);
      rsize -= rp[rsize - 1] == 0;
    }

  rem->size = (sign_remainder >= 0) ? rsize : -rsize;
}
