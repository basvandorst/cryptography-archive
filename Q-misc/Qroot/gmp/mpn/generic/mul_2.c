/* mpn_mul_2 -- Multiply a limb vector with a limb pair and
   store the product in a second limb vector.

Copyright (C) 1991,1992, 1993 Free Software Foundation, Inc.

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
#include "longlong.h"

mp_limb
mpn_mul_2 (res_ptr, s1_ptr, s1_size, s2_limb0, s2_limb1)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     mp_size_t s1_size;
     register mp_limb s2_limb0;
     register mp_limb s2_limb1;
{
  res_ptr[s1_size] = mpn_mul_1 (res_ptr, s1_ptr, s1_size, s2_limb0);
  return mpn_addmul_1 (res_ptr + 1, s1_ptr, s1_size, s2_limb1);
}
