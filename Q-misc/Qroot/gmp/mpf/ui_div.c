/* mpf_ui_div -- Divide an unsigned integer with a float.

Copyright (C) 1993 Free Software Foundation, Inc.

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
mpf_ui_div (mpf_ptr r, unsigned int dividend, mpf_srcptr v)
#else
mpf_ui_div (r, dividend, v)
     mpf_ptr r;
     unsigned int dividend;
     mpf_srcptr v;
#endif
{
  mp_ptr up, vp;
  mp_ptr rp;
  mp_size_t usize, vsize, rsize;
  mp_size_t abs_vsize;
  mp_size_t prec;
  unsigned normalization_steps;
  mp_limb qlimb;
  mp_exp_t rexp;

  prec = r->prec;
  rp = r->d;

  vp = v->d;
  rexp = 1 - v->exp;
  vsize = v->size;
  abs_vsize = ABS (vsize);

  usize = abs_vsize + prec;
  up = (mp_ptr) alloca ((usize + 1) * BYTES_PER_MP_LIMB);
  MPN_ZERO (up, usize);

  count_leading_zeros (normalization_steps, vp[abs_vsize - 1]);

  /* Normalize the denominator and the numerator.  */
  if (normalization_steps != 0)
    {
      mp_ptr tp;
      mp_limb dividend_high, dividend_low;

      /* Shift up the divisor setting the most significant bit of
	 the most significant limb.  Use temporary storage not to clobber
	 the original contents of the divisor.  */
      tp = (mp_ptr) alloca (abs_vsize * BYTES_PER_MP_LIMB);
      (void) mpn_lshift (tp, vp, abs_vsize, normalization_steps);
      vp = tp;

      /* Shift up the dividend, possibly introducing a new most
	 significant word.  */
      dividend_high = dividend >> (BITS_PER_MP_LIMB - normalization_steps);
      dividend_low = dividend << normalization_steps;

      if (dividend_high != 0)
	{
	  up[usize] = dividend_high;
	  up[usize - 1] = dividend_low;
	  rexp++;
	}
      else
	{
	  usize--;
	  up[usize] = dividend_low;
	}
    }
  else
    {
      /* The divisor is already normalized, as required.
	 Copy it to temporary space if it overlaps with the quotient.  */
      if (vp == rp)
	{
	  vp = (mp_ptr) alloca (abs_vsize * BYTES_PER_MP_LIMB);
	  MPN_COPY (vp, rp, abs_vsize);
	}

      usize--;
      up[usize] = dividend;
    }

  qlimb = mpn_divmod (rp, up, usize + 1, vp, abs_vsize);
  rsize = usize + 1 - abs_vsize;
  if (qlimb)
    {
      rp[rsize] = qlimb;
      rsize++;
      rexp++;
    }
  r->size = vsize >= 0 ? rsize : -rsize;
  r->exp = rexp;
}
