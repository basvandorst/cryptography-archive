/* mpf_mul -- Multiply two floats.

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
mpf_mul (mpf_ptr r, mpf_srcptr u, mpf_srcptr v)
#else
mpf_mul (r, u, v)
     mpf_ptr r;
     mpf_srcptr u;
     mpf_srcptr v;
#endif
{
  mp_srcptr up, vp;
  mp_size_t usize, vsize;
  mp_size_t rsize;
  mp_size_t sign_product;
  mp_size_t prec = r->prec;
  mp_limb cy_limb;
  mp_ptr rp, tp;

  usize = u->size;
  vsize = v->size;
  sign_product = usize ^ vsize;

  usize = ABS (usize);
  vsize = ABS (vsize);

  rp = r->d;
  up = u->d;
  vp = v->d;
  if (usize > prec)
    {
      up += usize - prec;
      usize = prec;
    }
  if (vsize > prec)
    {
      vp += vsize - prec;
      vsize = prec;
    }

  rsize = usize + vsize;
  tp = (mp_ptr) alloca (rsize * BYTES_PER_MP_LIMB);
  cy_limb = (usize >= vsize
	     ? mpn_mul (tp, up, usize, vp, vsize)
	     : mpn_mul (tp, vp, vsize, up, usize));
      
  cy_limb = cy_limb == 0;
  rsize -= cy_limb;
  if (rsize > prec)
    {
      tp += rsize - prec;
      rsize = prec;
    }
  MPN_COPY (rp, tp, rsize);
  r->exp = u->exp + v->exp - cy_limb;
  r->size = sign_product >= 0 ? rsize : -rsize;
}
