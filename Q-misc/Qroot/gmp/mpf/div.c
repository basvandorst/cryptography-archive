/* mpf_div -- Divide two floats.

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
mpf_div (mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
#else
mpf_div (r, u, v)
     mpf_ptr r;
     mpf_srcptr u;
     mpf_srcptr v;
#endif
{
  mp_srcptr up, vp;
  mp_ptr qp, rp, rrp;
  mp_size_t usize = u->size;
  mp_size_t vsize = v->size;
  mp_size_t qsize, rsize;
  mp_size_t sign_quotient = usize ^ vsize;
  unsigned normalization_steps;
  mp_limb q_limb;
  mp_size_t prec;
  mp_exp_t rexp;

  prec = r->prec;
  usize = ABS (usize);
  vsize = ABS (vsize);

  rexp = u->exp - v->exp;

  qp = r->d;
  up = u->d;
  vp = v->d;

  if (vsize > prec)
    {
      vp += vsize - prec;
      vsize = prec;
    }

  rsize = vsize + prec;

  rp = (mp_ptr) alloca ((rsize + 1) * BYTES_PER_MP_LIMB);

  if (usize > rsize)
    {
      up += usize - rsize;
      usize = rsize;
      rrp = rp;
    }
  else
    {
      MPN_ZERO (rp, rsize - usize);
      rrp = rp + (rsize - usize);
    }

  count_leading_zeros (normalization_steps, vp[vsize - 1]);

  /* Normalize the divisor and the dividend.  */
  if (normalization_steps != 0)
    {
      mp_ptr tp;
      mp_limb nlimb;

      /* Shift up the divisor setting the most significant bit of
	 the most significant word.  Use temporary storage not to clobber
	 the original contents of the divisor.  */
      tp = (mp_ptr) alloca (vsize * BYTES_PER_MP_LIMB);
      (void) mpn_lshift (tp, vp, vsize, normalization_steps);
      vp = tp;

      /* Shift up the dividend, possibly introducing a new most
	 significant word.  Move the shifted dividend in the remainder
	 meanwhile.  */
      nlimb = mpn_lshift (rrp, up, usize, normalization_steps);
      if (nlimb != 0)
	{
	  rrp[usize] = nlimb;
	  rsize++;
	  rexp++;
	}
    }
  else
    {
      /* The divisor is already normalized, as required.
	 Copy it to temporary space if it overlaps with the quotient.  */
      if (vp == qp)
	{
	  vp = (mp_ptr) alloca (vsize * BYTES_PER_MP_LIMB);
	  MPN_COPY ((mp_ptr) vp, qp, vsize);
	}

      /* Move the dividend to the remainder.  */
      MPN_COPY (rrp, up, usize);
    }

  q_limb = mpn_divmod (qp, rp, rsize, vp, vsize);

  qsize = rsize - vsize;
  if (q_limb)
    {
      qp[qsize] = q_limb;
      qsize++;
      rexp++;
    }

  r->size = sign_quotient >= 0 ? qsize : -qsize;
  r->exp = rexp;
}
