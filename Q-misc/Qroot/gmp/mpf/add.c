/* mpf_add -- Add two floats.

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
mpf_add (mpf_ptr sum, mpf_srcptr u, mpf_srcptr v)
#else
mpf_add (sum, u, v)
     mpf_ptr sum;
     mpf_srcptr u;
     mpf_srcptr v;
#endif
{
  mp_srcptr up, vp;
  mp_ptr sump;
  mp_size_t usize, vsize, sumsize;
  mp_size_t abs_usize;
  mp_size_t abs_vsize;
  mp_size_t prec = sum->prec;
  mp_size_t ediff;
  mp_limb cy_limb;

  if (u->exp < v->exp)
    {
      mpf_srcptr t;
      t = u; u = v; v = t;
    }

  usize = u->size;
  vsize = v->size;
  abs_usize = ABS (usize);
  abs_vsize = ABS (vsize);
  up = u->d;
  vp = v->d;
  sump = sum->d;

  ediff = u->exp - v->exp;

  /* U is >= V (well, at least it's exponent).  */
  if (abs_usize >= prec)
    {
      /* U is enough large to satisfy the precision demands.
	 SUM will become PREC or PREC+1 limbs.  */
      up += abs_usize - prec;
      if (ediff + abs_vsize < prec)
	{
	  /* uuuuuuuuuuuuuuuu */
	  /*   vvvvvvvvv      */
	  MPN_COPY (sump, up, prec - (ediff + abs_vsize));
	  sump += prec - (ediff + abs_vsize);
	  up += prec - (ediff + abs_vsize);
	  cy_limb = mpn_add (sump, up, ediff + abs_vsize, vp, abs_vsize);
	}
      else
	{
	  vp += ediff + (abs_vsize - prec);
	  abs_vsize -= ediff + (abs_vsize - prec);
	  abs_vsize = MAX (0, abs_vsize);
	  cy_limb = mpn_add (sump, up, ediff + abs_vsize, vp, abs_vsize);
	}

      sumsize = prec + cy_limb;
    }
  else
    {
      /* U is not enough large; we will use all its limbs.  Might need
	 limbs only from V for the least significant limbs.  SUM might
	 become less that PREC limbs (but might be as large as PREC+1). */

      /* First copy the part of V that doesn't reach up to U's least
	 significant limb.  */
      if (ediff >= prec)
	{
	  /* V completely cancelled.  */
	  if (sump != up)
	    MPN_COPY (sump, up, abs_usize);
	  sumsize = abs_usize;
	}
      else
	{
	  /* At least something of V should be in SUM.  */

	Cases:
	  /* uuuuuuu        */
	  /*   vvvvvvvvvvvv */

	  /* uuuuuuu        */
	  /*           vvvv */

	  if (abs_vsize + ediff > prec)
	    {
	      vp += abs_vsize + ediff - prec;
	      abs_vsize -= abs_vsize + ediff - prec;
	    }
	  if (abs_usize > ediff)
	    {
	      /* U and V partly overlaps.  */
	      MPN_COPY (sump, vp, abs_vsize - (abs_usize - ediff));
	      mpn_add (sump + abs_vsize - (abs_usize - ediff),
		       up, abs_usize, vp + abs_vsize - (abs_usize - ediff),
		       (abs_usize - ediff));
	      sumsize = abs_vsize + ediff;
	    }
	  else
	    {
	      /* U and V doesn't overlap at all.  (The block of zeros might be
		 0 limbs.)  We perform these copying operations in backward
		 order to allow overlapping between RES and U or V.  */
	      MPN_COPY_DECR (sump + abs_vsize + (ediff - abs_usize), up, abs_usize);
	      /* Could we use MPN_COPY_DECR here instead??? */
	      MPN_COPY_INCR (sump, vp, abs_vsize);
	      MPN_ZERO (sump + abs_vsize, ediff - abs_usize);
	      sumsize = abs_vsize + ediff;
	    }
	}
    }

  sum->size = sumsize;
  sum->exp = u->exp; /* Should add carry??? */
}
