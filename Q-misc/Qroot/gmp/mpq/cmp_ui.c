/* mpq_cmp_ui(u,vn,vd) -- Compare U with Vn/Vd.  Return positive, zero, or
   negative based on if U > V, U == V, or U < V.  Vn and Vd must not have
   any common factors.

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

int
#if __STDC__ mpq_cmp_ui (const MP_RAT *op1, unsigned long int num2, unsigned long int den2)
#else
mpq_cmp (op1, op2)
     const MP_RAT *op1;
     unsigned long int num2;
     unsigned long int den2;
#endif
{
  mp_size_t num1_size = op1->num.size;
  mp_size_t den1_size = op1->den.size;
  mp_size_t tmp1_size, tmp2_size;
  mp_ptr tmp1_ptr, tmp2_ptr;
  mp_size_t num1_sign;
  int cc;

  if (num1_size == 0)
    return -(num2 != 0);
  if (num1_size < 0)
    return num1_size;
  if (num2 == 0)
    return num1_size;

  num1_sign = num1_size;
  num1_size = ABS (num1_size);

  /* NUM1 x DEN2 is either TMP1_SIZE limbs or TMP1_SIZE-1 limbs.
     Same for NUM1 x DEN1 with respect to TMP2_SIZE.  */
  if (num1_size > den1_size + 1)
    /* NUM1 x DEN2 is surely larger in magnitude than NUM2 x DEN1.  */
    return num1_sign;
  if (den1_size > num1_sign + 1)
    /* NUM1 x DEN2 is surely smaller in magnitude than NUM2 x DEN1.  */
    return -num1_sign;

  tmp1_ptr = (mp_ptr) alloca (num1_size * BYTES_PER_MP_LIMB);
  tmp2_ptr = (mp_ptr) alloca (den1_size * BYTES_PER_MP_LIMB);

  cy_limb_1 = mpn_mul_1 (tmp1_ptr, op1->num.d, num1_size, den2);
  cy_limb_2 = mpn_mul_1 (tmp2_ptr, op1->den.d, den1_size, num2);

  tmp1_size = num1_size + (cy_limb_1 != 0);
  tmp2_size = den1_size + (cy_limb_2 != 0);

  cc = tmp1_size - tmp2_size != 0
    ? tmp1_size - tmp2_size : mpn_cmp (tmp1_ptr, tmp2_ptr, tmp1_size);
  return (num1_sign < 0) ? -cc : cc;
}
