/* mpn_divq1 -- Divide two limb vectors producing the most significant
   quotient limb.

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

/* Divide num (NUM_PTR/NUM_SIZE) by den (DEN_PTR/DEN_SIZE) and return
   the most significant quotient limb.

   Argument constraints:
   1. The most significant bit of the divisor must be set.
   2. The most significant limb of the divisor must be less
      than the most significant limb of the dividend.  */

mp_limb
mpn_divq1 (mp_srcptr num_ptr, mp_size_t num_size,
	   mp_srcptr den_ptr, mp_size_t den_size)
{
  switch (den_size)
    {
    case 0:
      /* We are asked to divide by zero, so go ahead and do it!  (To make
	 the compiler not remove this statement, return the value.)  */
      return 1 / den_size;

    case 1:
      {
	mp_limb n1, n0;
	mp_limb d;

	d = den_ptr[0];
	n1 = num_ptr[num_size - 1];

	if (n1 >= d)
	  return 1;

	udiv_qrnnd (q, n1, n1, num_ptr[num_size - 2], d);
	return q;
      }
      break;

    case 2:
      {
	mp_limb n1, n0, n2;
	mp_limb d1, d0;
	mp_limb q;
	mp_limb r;

	num_ptr += num_size - 2;
	d1 = den_ptr[1];
	d0 = den_ptr[0];
	n1 = num_ptr[1];
	n0 = num_ptr[0];

	if (n1 >= d1 && (n1 > d1 || n0 >= d0))
	  return 1;

	num_ptr--;
	if (n1 == d1)
	  {
	    /* Q should be either 111..111 or 111..110.  Need special
	       treatment of this rare case as normal division would
	       give overflow.  */
	    q = ~(mp_limb) 0;

	    r = n0 + d1;
	    if (r < d1)	/* Carry in the addition? */
	      return q;
	    n1 = d0 - (d0 != 0);
	    n0 = -d0;
	  }
	else
	  {
	    udiv_qrnnd (q, r, n1, n0, d1);
	    umul_ppmm (n1, n0, d0, q);
	  }

	n2 = num_ptr[0];
      q_test:
	if (n1 > r || (n1 == r && n0 > n2))
	  {
	    /* The estimated Q was too large.  */
	    q--;

	    sub_ddmmss (n1, n0, n1, n0, 0, d0);
	    r += d1;
	    if (r >= d1)	/* If not carry, test Q again.  */
	      goto q_test;
	  }

	return q;
      }
      break;

    default:
      {
	mp_limb dX, d1, n0;
	mp_limb q;
	mp_limb n1;
	mp_limb cy_limb;
	mp_limb d, c;
	mp_size_t j;

	num_ptr += num_size;
	den_ptr += den_size;
	dX = den_ptr[-1];
	d1 = den_ptr[-2];
	n0 = num_ptr[-1];

	if (n0 >= dX)
	  {
	    if (n0 > dX
		|| mpn_cmp (num_ptr - den_size, den_ptr - den_size,
			    den_size - 1) >= 0)
	      {
		return 1;
	      }
	  }

	num_ptr--;
	if (n0 == dX)
	  /* This might over-estimate Q, but it's probably not worth
	     the extra code here to find out.  */
	  q = ~(mp_limb) 0;
	else
	  {
	    mp_limb r;

	    udiv_qrnnd (q, r, n0, num_ptr[-1], dX);
	    umul_ppmm (n1, n0, d1, q);

	    while (n1 > r || (n1 == r && n0 > num_ptr[-2]))
	      {
		q--;
		r += dX;
		if (r < dX)	/* I.e. "carry in previous addition?"  */
		  break;
		n1 -= n0 < d1;
		n0 -= d1;
	      }
	  }

#if X

	/* Now compute (NUM - Q * DEN) from most significant limb towards
	   least significant limb, until the result is non-zero.  */

	back;
	for (i = den_size - 2; i >= 0; i--)
	  {
	    umul_ppmm (n1, n0, q, den_ptr[i]);
	    if (n1 > r || (n1 == r && n0 > num_ptr[i - 2]))
	      {
		return q - 1;
#if THAT_DOES_NOT_SEEM_TO_WORK_TRY_THIS
		q--;
		goto back;
#endif
	      }
	    else if (..)
	    sub_ddmmss (dummy, r, r, num_ptr[i - 2], n1, n0);
	  }
	return q;
#endif

	/* Possible optimization: We already have (q * n0) and (1 * n1)
	   after the calculation of q.  Taking advantage of that, we
	   could make this loop make two iterations less.  */

	cy_limb = mpn_submul_1 (num_ptr - den_size,
				den_ptr - den_size, den_size, q);

	if (num_ptr[0] != cy_limb)
	  {
	    mp_limb cy;
	    cy = mpn_add_n (num_ptr - den_size,
			    num_ptr - den_size, den_ptr - den_size, den_size);
	    if (cy == 0)
	      abort();
	    q--;
	  }

	return q;
	n0 = num_ptr[-1];
      }
    }
}
