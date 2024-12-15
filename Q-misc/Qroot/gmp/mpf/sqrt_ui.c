/* mpf_sqrt_ui -- Compute the square root of an unsigned integer.

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
mpf_sqrt_ui (mpf_ptr r, unsigned int u)
#else
mpf_sqrt_ui (r, u)
     mpf_ptr r;
     unsigned int u;
#endif
{
  mp_size_t rsize;
  mp_ptr tp;
  mp_size_t prec;

  if (u == 0)
    {
      r->size = 0;
      r->exp = 0;
      return;
    }

  prec = r->prec;

  rsize = 2 * prec + 1;

  tp = (mp_ptr) alloca (rsize * BYTES_PER_MP_LIMB);

  MPN_ZERO (tp, rsize - 1);
  tp[rsize - 1] = u;

  mpn_sqrt (r->d, NULL, tp, rsize);

  r->size = prec + 1;
  r->exp = 1;
}
