/* mpf_set -- Assign a float from another float.

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
mpf_set (mpf_ptr w, mpf_srcptr u)
#else
mpf_set (w, u)
     mpf_ptr w;
     mpf_srcptr u;
#endif
{
  mp_ptr wp, up;
  mp_size_t usize, size;
  mp_size_t prec;

  prec = w->prec;
  usize = u->size;
  size = ABS (usize);

  wp = w->d;
  up = u->d;

  if (size > prec)
    {
      up += size - prec;
      size = prec;
    }

  MPN_COPY (wp, up, size);
  w->exp = u->exp;
  w->size = usize >= 0 ? size : -size;
}
