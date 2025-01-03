/* mpz_init_set_str(string, base) -- Convert the \0-terminated string
   STRING in base BASE to a multiple precision integer.  Return a MP_INT
   structure representing the integer.  Allow white space in the
   string.  If BASE == 0 determine the base in the C standard way,
   i.e.  0xhh...h means base 16, 0oo...o means base 8, otherwise
   assume base 10.

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

int
#if __STDC__
mpz_init_set_str (mpz_ptr x, const char *str, int base)
#else
mpz_init_set_str (x, str, base)
     mpz_ptr x;
     const char *str;
     int base;
#endif
{
  x->alloc = 1;
  x->d = (mp_ptr) (*_mp_allocate_func) (BYTES_PER_MP_LIMB * x->alloc);

  return mpz_set_str (x, str, base);
}
