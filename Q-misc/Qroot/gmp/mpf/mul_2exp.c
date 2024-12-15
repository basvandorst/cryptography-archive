/* mpf_mul_2exp -- Multiply a float by 2^n.

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

void
#if __STDC__
mpf_mul_2exp (mpf_ptr r, mpf_srcptr u, unsigned long int exp)
#else
mpf_mul_2exp (r, u, exp)
     mpf_ptr r;
     mpf_srcptr u;
     unsigned long int exp;
#endif
{
  mp_srcptr up;
  mp_ptr rp = r->d;
  mp_size_t usize;
  mp_size_t abs_usize;
  mp_size_t prec = r->prec;
  mp_exp_t uexp = u->exp;

  usize = u->size;
  abs_usize = ABS (usize);
  up = u->d;

  if (abs_usize > prec)
    {
      up += abs_usize - prec;
      abs_usize -= abs_usize - prec; /* E.q. abs_usize = prec */
    }

  if ((exp & (BITS_PER_MP_LIMB - 1)) == 0)
    {
      if (rp != up)
	MPN_COPY_INCR (rp, up, abs_usize);
      r->size = usize >= 0 ? abs_usize : -abs_usize;
      r->exp = uexp + exp / BITS_PER_MP_LIMB;
    }
  else
    {
      if (r != u)
	{
	  mp_limb cy_limb;
	  cy_limb = mpn_lshift (rp, up, abs_usize, exp % BITS_PER_MP_LIMB);
	  rp[abs_usize] = cy_limb;
	  cy_limb = cy_limb != 0;
	  r->size = usize >= 0 ? (abs_usize + cy_limb) : -(abs_usize + cy_limb);
	  r->exp = uexp + exp / BITS_PER_MP_LIMB + cy_limb;
	}
      else
	{
	  /* This special case is needed since mpn_lshift operates downwards,
	     and we therefore would potentially clobber part of U before using
	     that part.  */
	  mp_limb cy_limb;
	  cy_limb = mpn_rshift (rp + 1, up, abs_usize, BITS_PER_MP_LIMB - exp % BITS_PER_MP_LIMB);
	  rp[0] = cy_limb;
	  cy_limb = rp[abs_usize] != 0;
	  r->size = usize >= 0 ? (abs_usize + cy_limb) : -(abs_usize + cy_limb);
	  r->exp = uexp + exp / BITS_PER_MP_LIMB + cy_limb;
	}
    }
}
