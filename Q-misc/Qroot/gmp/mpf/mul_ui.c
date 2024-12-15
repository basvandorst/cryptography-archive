/* mpf_mul_ui -- Multiply a float and an unsigned integer.

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
mpf_mul_ui (mpf_ptr r, mpf_srcptr u, unsigned int v)
#else
mpf_mul_ui (r, u, v)
     mpf_ptr r;
     mpf_srcptr u;
     unsigned int v;
#endif
{
  mp_srcptr up;
  mp_size_t usize;
  mp_size_t size;
  mp_size_t prec = r->prec;
  mp_limb cy_limb;
  mp_ptr rp;

  usize = u->size;
  size = ABS (usize);

  rp = r->d;
  up = u->d;
  if (size > prec)
    {
      up += size - prec;
      size = prec;
    }

  cy_limb = mpn_mul_1 (rp, up, size, (mp_limb) v);
  rp[size] = cy_limb;
  cy_limb = cy_limb != 0;
  r->exp = u->exp + cy_limb;
  size += cy_limb;
  r->size = usize >= 0 ? size : -size;
}
