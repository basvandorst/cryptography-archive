/* mpz_init() -- Make a new multiple precision number with value 0.

Copyright (C) 1991 Free Software Foundation, Inc.

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
mpz_init (MP_INT *x)
#else
mpz_init (x)
     MP_INT *x;
#endif
{
  x->alloc = 1;
  x->d = (mp_ptr) (*_mp_allocate_func) (BYTES_PER_MP_LIMB * x->alloc);
  x->size = 0;
}
