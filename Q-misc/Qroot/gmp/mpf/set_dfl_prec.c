/* mpf_set_default_prec --

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

mp_size_t __gmp_default_fp_limb_precision = (53 + BITS_PER_MP_LIMB - 1) / BITS_PER_MP_LIMB;

mp_size_t
#if __STDC__
mpf_set_default_prec (mp_size_t prec)
#else
mpf_set_default_prec (prec)
     mp_size_t prec;
#endif
{
  mp_size_t prec_in_limbs;

  prec_in_limbs = (MAX (53, prec) + BITS_PER_MP_LIMB - 1) / BITS_PER_MP_LIMB;
  __gmp_default_fp_limb_precision = prec_in_limbs;
  return prec_in_limbs * BITS_PER_MP_LIMB;
}
