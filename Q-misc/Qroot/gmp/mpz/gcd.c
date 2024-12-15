/* mpz_gcd -- Calculate the greatest common divisior of two integers.

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

#ifndef BERKELEY_MP
void
#if __STDC__
mpz_gcd (mpz_ptr w, mpz_srcptr u, mpz_srcptr v)
#else
mpz_gcd (w, u, v)
     mpz_ptr w;
     mpz_srcptr u;
     mpz_srcptr v;
#endif
#else /* BERKELEY_MP */
void
#if __STDC__
gcd (mpz_srcptr u, mpz_srcptr v, mpz_ptr w)
#else
gcd (u, v, w)
     mpz_srcptr u;
     mpz_srcptr v;
     mpz_ptr w;
#endif
#endif /* BERKELEY_MP */
{
  mp_size_t usize, vsize, wsize;
  mp_ptr up_in, vp_in;
  mp_ptr up, vp;
  mp_ptr wp;
  mp_size_t i;
  mp_limb d;
  int bcnt;
  mp_size_t w_bcnt;
  mp_limb cy_digit;

  usize = ABS (u->size);
  vsize = ABS (v->size);

  /* GCD(0,v) == v.  */
  if (usize == 0)
    {
      if (w->alloc < vsize)
	_mpz_realloc (w, vsize);

      w->size = vsize;
      MPN_COPY (w->d, v->d, vsize);
      return;
    }

  /* GCD(0,u) == u.  */
  if (vsize == 0)
    {
      if (w->alloc < usize)
	_mpz_realloc (w, usize);

      w->size = usize;
      MPN_COPY (w->d, u->d, usize);
      return;
    }

  /* Make U odd by shifting it down as many bit positions as there
     are zero bits.  Put the result in temporary space.  */
  up = (mp_ptr) alloca (usize * BYTES_PER_MP_LIMB);
  up_in = u->d;
  for (i = 0; (d = up_in[i]) == 0; i++)
    ;
  count_leading_zeros (bcnt, d & -d);
  bcnt = BITS_PER_MP_LIMB - 1 - bcnt;
  usize -= i;
  if (bcnt != 0)
    {
      mpn_rshift (up, up_in + i, usize, bcnt);
      usize -= up[usize - 1] == 0;
    }
  else
    MPN_COPY (up, up_in + i, usize);

  bcnt += i * BITS_PER_MP_LIMB;
  w_bcnt = bcnt;

  /* Make V odd by shifting it down as many bit positions as there
     are zero bits.  Put the result in temporary space.  */
  vp = (mp_ptr) alloca (vsize * BYTES_PER_MP_LIMB);
  vp_in = v->d;
  for (i = 0; (d = vp_in[i]) == 0; i++)
    ;
  count_leading_zeros (bcnt, d & -d);
  bcnt = BITS_PER_MP_LIMB - 1 - bcnt;
  vsize -= i;
  if (bcnt != 0)
    {
      mpn_rshift (vp, vp_in + i, vsize, bcnt);
      vsize -= vp[vsize - 1] == 0;
    }
  else
    MPN_COPY (vp, vp_in + i, vsize);

  /* W_BCNT is set to the minimum of the number of zero bits in U and V.
     Thus it represents the number of common 2 factors.  */
  bcnt += i * BITS_PER_MP_LIMB;
  if (bcnt < w_bcnt)
    w_bcnt = bcnt;

  for (;;)
    {
      int cmp;

      cmp = usize - vsize != 0 ? usize - vsize : mpn_cmp (up, vp, usize);

      /* If U and V have become equal, we have found the GCD.  */
      if (cmp == 0)
	break;

      if (cmp > 0)
	{
	  /* Replace U by (U - V) >> cnt, with cnt being the least value
	     making U odd again.  */

	  mpn_sub (up, up, usize, vp, vsize);
	  for (i = 0; (d = up[i]) == 0; i++)
	    ;
	  count_leading_zeros (bcnt, d & -d);
	  bcnt = BITS_PER_MP_LIMB - 1 - bcnt;
	  usize -= i;
	  if (bcnt != 0)
	    {
	      mpn_rshift (up, up + i, usize, bcnt);
	      MPN_NORMALIZE_NOT_ZERO (up, usize);
	    }
	  else
	    MPN_COPY_INCR (up, up + i, usize);
	}
      else
	{
	  /* Replace V by (V - U) >> cnt, with cnt being the least value
	     making V odd again.  */

	  mpn_sub (vp, vp, vsize, up, usize);
	  for (i = 0; (d = vp[i]) == 0; i++)
	    ;
	  count_leading_zeros (bcnt, d & -d);
	  bcnt = BITS_PER_MP_LIMB - 1 - bcnt;
	  vsize -= i;
	  if (bcnt != 0)
	    {
	      mpn_rshift (vp, vp + i, vsize, bcnt);
	      MPN_NORMALIZE_NOT_ZERO (vp, vsize);
	    }
	  else
	    MPN_COPY_INCR (vp, vp + i, vsize);
	}
    }

  /* GCD(U_IN, V_IN) now is U * 2**W_BCNT.  */

  wsize = usize + w_bcnt / BITS_PER_MP_LIMB + 1;
  if (w->alloc < wsize)
    _mpz_realloc (w, wsize);

  wp = w->d;

  MPN_ZERO (wp, w_bcnt / BITS_PER_MP_LIMB);

  if (w_bcnt % BITS_PER_MP_LIMB != 0)
    {
      cy_digit = mpn_lshift (wp + w_bcnt / BITS_PER_MP_LIMB, up, usize,
			     w_bcnt % BITS_PER_MP_LIMB);
      wsize = usize + w_bcnt / BITS_PER_MP_LIMB;
      if (cy_digit != 0)
	{
	  wp[wsize] = cy_digit;
	  wsize++;
	}

      w->size = wsize;
    }
  else
    {
      MPN_COPY (wp + w_bcnt / BITS_PER_MP_LIMB, up, usize);
      w->size = w_bcnt / BITS_PER_MP_LIMB + usize;
    }
}
