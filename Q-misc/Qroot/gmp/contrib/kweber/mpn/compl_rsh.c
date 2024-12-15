/* mpn_compl_rsh -- Twos complement and shift right.

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

/* Complement (2s complement) and shift U (pointed to by UP and USIZE limbs
   long) CNT bits to the right and store the USIZE least significant limbs of
   the result at WP.  Return -U[0] << (BITS_PER_MP_LIMB - CNT).

   Argument constraints:
   1. 0 < CNT < BITS_PER_MP_LIMB
   2. USIZE > 0
   3. If the result is to be written over the input, WP must be <= UP.
   4. U[0] != 0
*/

mp_limb
#if __STDC__
mpn_compl_rsh (register mp_ptr wp,
	       register mp_srcptr up, mp_size_t usize,
	       register unsigned int cnt)
#else
mpn_compl_rsh (wp, up, usize, cnt)
     register mp_ptr wp;
     register mp_srcptr up;
     mp_size_t usize;
     register unsigned int cnt;
#endif
{
  register mp_limb x, high_limb, low_limb;
  register unsigned sh_1, sh_2;
  register mp_size_t i;
  mp_limb retval;

#ifdef DEBUG
  if (usize == 0 || cnt == 0)
    abort ();
#endif

  sh_1 = cnt;

  wp -= 2;				/*  wp is 2 behind up.  */
  sh_2 = BITS_PER_MP_LIMB - sh_1;
  high_limb = -up[0];
  retval = high_limb << sh_2;

  i = 1;

  while (i < usize)
    {
      x = up[i];
      i += 1;
      low_limb = high_limb >> sh_1;
      high_limb = ~x;
      wp[i] = low_limb | (high_limb << sh_2);
    }
  high_limb >>= sh_1;
  wp[i+1] = high_limb;

  return retval;
}
