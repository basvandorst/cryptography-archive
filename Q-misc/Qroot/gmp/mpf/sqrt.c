/* mpf_sqrt -- Compute the square root of a float.

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

/* This code is just correct if "unsigned char" has at least 8 bits.  It
   doesn't help to use CHAR_BIT from limits.h, as the real problem is
   the static arrays.  */

#include "gmp.h"
#include "gmp-impl.h"

void
#if __STDC__
mpf_sqrt (mpf_ptr r, mpf_srcptr u)
#else
mpf_sqrt (r, u)
     mpf_ptr r;
     mpf_srcptr u;
#endif
{
  mp_size_t usize, rsize;
  mp_ptr up, tp;
  mp_size_t prec;
  mp_exp_t tsize, rexp;

  usize = u->size;
  if (usize < 0)
    usize = 1 / usize > 0;	/* Divide by zero for negative OP.  */

  prec = r->prec;

  rexp = (u->exp + 1) / 2;

  tsize = 2 * prec + (u->exp & 1);

  up = u->d;
  tp = (mp_ptr) alloca (tsize * BYTES_PER_MP_LIMB);

  if (usize > tsize)
    {
      up += usize - tsize;
      usize = tsize;
      MPN_COPY (tp, up, tsize);
    }
  else
    {
      MPN_ZERO (tp, tsize - usize);
      MPN_COPY (tp + (tsize - usize), up, usize);
    }

  mpn_sqrt (r->d, NULL, tp, tsize);

  r->size = (tsize + 1) / 2;
  r->exp = rexp;
}
