/* mpn_rshiftci -- Shift a low level natural-number integer with carry in.

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

/* Shift U (pointed to by UP and USIZE digits long) CNT bits to the right
   and store the USIZE least significant digits of the result at WP.
   Return the size of the result.

   Argument constraints:
   0. U must be normalized (i.e. it's most significant digit != 0).
   1. 0 <= CNT < BITS_PER_MP_LIMB
   2. If the result is to be written over the input, WP must be <= UP.
*/

mp_limb
#if __STDC__
mpn_rshiftci (register mp_ptr wp,
	      register mp_srcptr up, mp_size_t usize,
	      register unsigned int cnt,
	      mp_limb carry_in)
#else
mpn_rshiftci (wp, up, usize, cnt, carry_in)
     register mp_ptr wp;
     register mp_srcptr up;
     mp_size_t usize;
     register unsigned int cnt;
     mp_limb carry_in;
#endif
{
  register mp_limb high_limb, low_limb;
  register unsigned sh_1, sh_2;
  register mp_size_t i;

#ifdef DEBUG
  if (usize == 0 || cnt == 0)
    abort ();
#endif

  sh_1 = cnt;

#if 0
  if (sh_1 == 0)
    {
      if (wp != up)
	{
	  /* Copy from low end to high end, to allow specified input/output
	     overlapping.  */
	  for (i = 0; i < usize; i++)
	    wp[i] = up[i];
	}
      return usize;
    }
#endif

  wp -= 1;
  sh_2 = BITS_PER_MP_LIMB - sh_1;
  low_limb = up[0];
  for (i = 1; i < usize; i++)
    {
      high_limb = up[i];
      wp[i] = (low_limb >> sh_1) | (high_limb << sh_2);
      low_limb = high_limb;
    }
  low_limb = (low_limb >> sh_1) | (carry_in << sh_2);
  wp[i] = low_limb;

  return low_limb;
}
