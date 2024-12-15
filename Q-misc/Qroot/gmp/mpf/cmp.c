/* mpf_cmp -- Compare two floats.

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

int
#if __STDC__
mpf_cmp (mpf_srcptr u, mpf_srcptr v)
#else
mpf_cmp (u, v)
     mpf_srcptr u;
     mpf_srcptr v;
#endif
{
  mp_srcptr up, vp;
  mp_size_t usize, vsize;
  mp_exp_t uexp, vexp;
  int cmp;
  int usign;

  uexp = u->exp;
  vexp = v->exp;

  usize = u->size;
  vsize = v->size;

  /* 1. Are the signs different?  */
  if ((usize ^ vsize) >= 0)
    {
      /* U and V are both non-negative or both negative.  */
      if (usize == 0)
	/* vsize >= 0 */
	return -(vsize != 0);
      if (vsize == 0)
	/* usize >= 0 */
	return usize != 0;
      /* Fall out.  */
    }
  else
    {
      /* Either U or V is negative, but not both.  */
      return usize >= 0 ? 1 : -1;
    }

  /* U and V have the same sign and are both non-zero.  */

  usign = usize >= 0 ? 1 : -1;

  /* 2. Are the exponents different?  */
  if (uexp > vexp)
    return usign;
  if (uexp < vexp)
    return -usign;

  usize = ABS (usize);
  vsize = ABS (vsize);

  up = u->d;
  vp = v->d;

#define STRICT_MPF_NORMALIZATION 0
#if ! STRICT_MPF_NORMALIZATION
  /* Ignore zeroes at the low end of U and V.  */
  while (*up == 0)
    {
      up++;
      usize--;
    }
  while (*vp == 0)
    {
      vp++;
      vsize--;
    }
#endif

  /* 3. Now, if the number of limbs are different, we have a difference
     since we have made sure the trailing limbs are not zero.  */
  if (usize > vsize)
    return usign;
  if (usize < vsize)
    return -usign;

  /* 4. Compare the mantissas.  */
  cmp = mpn_cmp (up, vp, usize);
  if (cmp > 0)
    return usign;
  if (cmp < 0)
    return -usign;

  /* Wow, we got zero even if we tried hard to avoid it.  */
  return 0;
}
