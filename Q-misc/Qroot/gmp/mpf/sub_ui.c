/* mpf_sub_ui -- Subtract an unsigned integer from a float.

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
mpf_sub_ui (mpf_ptr sum, mpf_srcptr u, unsigned int v)
#else
mpf_sub_ui (sum, u, v)
     mpf_ptr sum;
     mpf_srcptr u;
     unsigned int v;
#endif
{
  mp_srcptr up;
  mp_ptr sump = sum->d;
  mp_size_t usize, sumsize;
  mp_size_t abs_usize;
  mp_size_t prec = sum->prec;
  mp_exp_t uexp = u->exp;

  usize = u->size;
  abs_usize = ABS (usize);

  if (v == 0)
    {
    sum_is_u:
      if (u != sum)
	{
	  sumsize = MIN (abs_usize, prec);
	  MPN_COPY (sum->d, u->d, sumsize);
	  sum->size = sumsize;
	  sum->exp = u->exp;
	}
      return;
    }

  if (uexp > 0)
    {
      /* U >= 1.  */
      if (uexp > prec)
	{
	  /* U >> V, V is not part of final result.  */
	  goto sum_is_u;
	}
      else
	{
	  /* U's "limb point" is somewhere between the first limb
	     and the PREC:th limb.
	     Both U and V are part of the final result.  */
	  if (uexp > usize)
	    {
	      /*   uuuuuu0000. */
	      /* +          v. */
	      /* We begin with moving U to the top of SUM, to handle samevar(U, SUM).  */
	      MPN_COPY_DECR (sump + uexp - usize, u->d, usize);
	      sump[0] = v;
	      MPN_ZERO (sump + 1, uexp - usize - 1);
	      if (sum == u)
		MPN_COPY (sum->d, sump, uexp);
	      sum->size = uexp;
	      sum->exp = uexp;
	    }
	  else
	    {
	      /*   uuuuuu.uuuu */
	      /* +      v.     */
	      mp_limb cy_limb;
	      up = u->d;
	      if (usize > prec)
		{
		  /* Ignore excess limbs in U.  */
		  up += usize - prec;
		  usize -= usize - prec; /* Eq. usize = prec */
		}
	      if (sump != up)
		MPN_COPY (sump, up, usize - uexp);
	      cy_limb = mpn_sub_1 (sump + usize - uexp, up + usize - uexp, uexp, (mp_limb) v);
	      sump[usize] = cy_limb;
	      sum->size = usize + cy_limb;
	      sum->exp = uexp + cy_limb;
	    }
	}
    }
  else
    {
      /* U < 1, so V > U for sure.  */
      /* v.         */
      /*  .0000uuuu */
      up = u->d;
      if (usize + (-uexp) + 1 > prec)
	{
	  /* Ignore excess limbs in U.  */
	  up += usize + (-uexp) + 1 - prec;
	  usize -= usize + (-uexp) + 1 - prec;
	}
      if (sump != up)
	MPN_COPY (sump, up, usize);
      MPN_ZERO (sump + usize, -uexp);
      sump[usize + (-uexp)] = v;
      sum->size = usize + (-uexp) + 1;
      sum->exp = 1;
    }
}
