/* mpz_add -- Add two integers.

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

#ifndef BERKELEY_MP
void
#if __STDC__
mpz_add (mpz_ptr sum, mpz_srcptr u, mpz_srcptr v)
#else
mpz_add (sum, u, v)
     mpz_ptr sum;
     mpz_srcptr u;
     mpz_srcptr v;
#endif
#else /* BERKELEY_MP */
void
#if __STDC__
madd (mpz_srcptr u, mpz_srcptr v, mpz_ptr sum)
#else
madd (u, v, sum)
     mpz_srcptr u;
     mpz_srcptr v;
     mpz_ptr sum;
#endif
#endif /* BERKELEY_MP */
{
  mp_srcptr up, vp;
  mp_ptr sump;
  mp_size_t usize, vsize, sumsize;
  mp_size_t abs_usize;
  mp_size_t abs_vsize;

  usize = u->size;
  vsize = v->size;
  abs_usize = ABS (usize);
  abs_vsize = ABS (vsize);

  if (abs_usize < abs_vsize)
    {
      /* Swap U and V. */
      {const __mpz_struct *t = u; u = v; v = t;}
      {mp_size_t t = usize; usize = vsize; vsize = t;}
      {mp_size_t t = abs_usize; abs_usize = abs_vsize; abs_vsize = t;}
    }

  /* True: ABS_USIZE >= ABS_VSIZE
     N.B. We didn't swap USIZE and VSIZE, so they might be out of sync
     with ABS_USIZE and ABS_VSIZE!!  */

  /* If not space for sum (and possible carry), increase space.  */
  sumsize = abs_usize + 1;
  if (sum->alloc < sumsize)
    _mpz_realloc (sum, sumsize);

  /* These must be after realloc (u or v may be the same as sum).  */
  up = u->d;
  vp = v->d;
  sump = sum->d;

  if ((usize ^ vsize) < 0)
    {
      /* U and V have different sign.  Need to compare them to determine
	 which operand to subtract from which.  */

      if (abs_usize != abs_vsize)
	{
	  mpn_sub (sump, up, abs_usize, vp, abs_vsize);
	  sumsize = mpn_normal_size (sump, abs_usize);
	  if (usize < 0)
	    sumsize = -sumsize;
	}
      else if (mpn_cmp (up, vp, abs_usize) < 0)
	{
	  mpn_sub_n (sump, vp, up, abs_usize);
	  sumsize = mpn_normal_size (sump, abs_usize);
	  if (usize >= 0)
	    sumsize = -sumsize;
	}
      else
	{
	  mpn_sub_n (sump, up, vp, abs_usize);
	  sumsize = mpn_normal_size (sump, abs_usize);
	  if (usize < 0)
	    sumsize = -sumsize;
	}
    }
  else
    {
      /* U and V have same sign.  Add them.  */
      mp_limb cy_limb = mpn_add (sump, up, abs_usize, vp, abs_vsize);
      sump[abs_usize] = cy_limb;
      sumsize = abs_usize + cy_limb;
      if (usize < 0)
	sumsize = -sumsize;
    }

  sum->size = sumsize;
}
