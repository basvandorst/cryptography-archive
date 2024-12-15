/* mpf_init_set -- Initialize a float and assign it from another float.

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
mpf_init_set (mpf_ptr r, mpf_srcptr s)
#else
mpf_init_set (r, s)
     mpf_ptr r;
     mpf_srcptr s;
#endif
{
  mp_ptr rp, sp;
  mp_size_t ssize, size;
  mp_size_t prec;

  r->alloc = __gmp_default_fp_limb_precision + 1;
  r->d = (mp_ptr) (*_mp_allocate_func) (BYTES_PER_MP_LIMB * r->alloc);
  r->prec = __gmp_default_fp_limb_precision;

  prec = r->prec;
  ssize = s->size;
  size = ABS (ssize);

  rp = r->d;
  sp = s->d;

  if (size > prec)
    {
      sp += size - prec;
      size = prec;
    }

  MPN_COPY (rp, sp, size);

  r->exp = s->exp;
  r->size = ssize >= 0 ? size : -size;
}
