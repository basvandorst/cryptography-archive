/* mpf_get_str (digit_ptr, exp, base, n_digits, a) -- Convert the floating
  point number A to a base BASE number and store N_DIGITS raw digits at
  DIGIT_PTR, and the base BASE exponent in the word pointed to by EXP.  For
  example, the number 3.1416 would be returned as "31416" in DIGIT_PTR and
  1 in EXP.

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

/* Problems:

   1. We use truncation instead of rounding.  For example, 1.9999 is
   printed as 1 if only one digit is requested.

   2. Performance is poor if we have a very large or very small exponent.
   If the exponent is -exp, we have to multiply by big_base about exp
   times.  It might seem trivial to fix this by computing B^exp and
   multiply by this quantity.  But we don't want that entire (possibly
   HUGE) product, we just want enough to output n_digits.

   3. In general, we should limit our computation precision to what
   n_digits requires.

   4. Actually make big_base be != base.  Trivial.
*/

char *
#if __STDC__
mpf_get_str (char *digit_ptr, mp_exp_t *exp, int base, size_t n_digits, mpf_srcptr u)
#else
mpf_get_str (digit_ptr, exp, base, n_digits, u)
     char *digit_ptr;
     mp_exp_t *exp;
     int base;
     size_t n_digits;
     mpf_srcptr u;
#endif
{
  mp_size_t usize;
  mp_exp_t uexp;
  unsigned char *str;
  size_t str_size;
  char *num_to_text;
  int i;
  mp_ptr rp;
  mp_limb big_base;
  size_t digits_computed_so_far;
  mp_srcptr up;

  usize = u->size;
  uexp = u->exp;

  if (base >= 0)
    {
      if (base == 0)
	base = 10;
      num_to_text = "0123456789abcdefghijklmnopqrstuvwxyz";
    }
  else
    {
      base = -base;
      num_to_text = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    }

  if (digit_ptr == 0)
    {
      /* We didn't get a string from the user.  Allocate one (and return
	 a pointer to it) with space for `-' and terminating null.  */
      digit_ptr = (char *) (*_mp_allocate_func) (n_digits + 2);
    }

  str = (unsigned char *) digit_ptr;

  if (usize < 0)
    {
      *digit_ptr = '-';
      str++;
      usize = -usize;
    }

  digits_computed_so_far = 0;

  if (uexp > 0 && uexp > usize)
    {
      /* We have an integral number, and the exponent is so large
	 that the "limb point" is beyond the last limb in the mantissa.  */
      unsigned char *tstr;

      rp = (mp_ptr) alloca ((uexp + 1) * BYTES_PER_MP_LIMB);
      up = u->d;
      MPN_COPY (rp + uexp - usize, up, usize);
      MPN_ZERO (rp, uexp - usize);

      str_size = ((size_t) (uexp * BITS_PER_MP_LIMB * __mp_bases[base].chars_per_bit_exactly)) + 3;
      tstr = (unsigned char *) alloca (str_size);

      str_size = mpn_get_str (tstr, base, rp, uexp);
      while (tstr[0] == 0)
	{
	  tstr++;
	  str_size--;
	}

      for (i = 0; i < str_size; i++)
	{
	  *str++ = num_to_text[tstr[i]];
	  digits_computed_so_far++;
	  if (digits_computed_so_far >= n_digits)
	    break;
	}

      while (str[-1] == '0')
	str--;

      *str = 0;
      *exp = str_size;
      return digit_ptr;
    }

  if (uexp > 0 && uexp <= usize)
    {
      /* First convert the integral part.  */
      unsigned char *tstr;

      rp = (mp_ptr) alloca ((uexp + 1) * BYTES_PER_MP_LIMB);
      up = u->d + usize - uexp;
      MPN_COPY (rp, up, uexp);

      str_size = ((size_t) (uexp * BITS_PER_MP_LIMB * __mp_bases[base].chars_per_bit_exactly)) + 3;
      tstr = (unsigned char *) alloca (str_size);

      str_size = mpn_get_str (tstr, base, rp, uexp);
      while (tstr[0] == 0)
	{
	  tstr++;
	  str_size--;
	}

      for (i = 0; i < str_size; i++)
	{
	  *str++ = num_to_text[tstr[i]];
	  digits_computed_so_far++;
	  if (digits_computed_so_far >= n_digits)
	    {
	      while (str[-1] == '0')
		str--;

	      *str = 0;
	      *exp = str_size;
	      return digit_ptr;
	    }
	}

      /* Modify somewhat and fall out... */
      usize -= uexp;
      uexp = 0;
    }

  *exp = digits_computed_so_far;

  /* Convert the fraction.  */
#if 0
  big_base = __mp_bases[base].big_base;
  dig_per_u = __mp_bases[base].chars_per_limb;
  out_len = ((size_t) msize * BITS_PER_MP_LIMB * __mp_bases[base].chars_per_bit_exactly) + 1;
#else
  big_base = base;
#endif

  if (usize != 0)
    {
      mp_limb cy_limb;

      up = u->d;
      rp = (mp_ptr) alloca ((usize + 1) * BYTES_PER_MP_LIMB);
      MPN_COPY (rp, up, usize);

      while (uexp < 0)
	{
	  mp_ptr x_rp = rp;

	  /* For speed: skip trailing zeroes.  */
	  if (rp[0] == 0)
	    {
	      rp++;
	      usize--;
	      if (usize == 0)
		{
		  *str = 0;
		  return digit_ptr;
		}
	    }

#if 0
	  /* BOTCH: The copying here could be avoided
	     if we had one extra limb in the vector, and
	     maintained two pointers  */
	  cy_limb = mpn_mul_1 (rp, rp, usize, big_base);
	  if (cy_limb != 0)
	    {
	      MPN_COPY_INCR (rp, rp + 1, usize - 1);
	      rp[usize - 1] = cy_limb;
	      uexp++;
	    }
#else
	  /* We use a trick here to avoid copying R back one limb
	     every time CY_LIMB becomes non-zero.  */
	  cy_limb = mpn_mul_1 (x_rp, rp, usize, big_base);
	  rp = x_rp;
	  if (cy_limb != 0)
	    {
	      rp[usize] = cy_limb;
	      rp++;
	      uexp++;
	    }
#endif
	  (*exp)--;
	}

      while (digits_computed_so_far < n_digits)
	{
	  /* For speed: skip trailing zeroes.  */
	  if (rp[0] == 0)
	    {
	      rp++;
	      usize--;
	      if (usize == 0)
		{
		  *str = 0;
		  return digit_ptr;
		}
	    }

	  cy_limb = mpn_mul_1 (rp, rp, usize, big_base);
	  if (digits_computed_so_far == 0 && cy_limb == 0)
	    {
	      (*exp)--;
	      continue;
	    }
#if 0
	  /* Convert N1 from BIG_BASE to a string of digits in BASE
	     using single precision operations.  */
	  for (i = dig_per_u - 1; i >= 0; i--)
	    {
	      *--s = n1 % base;
	      n1 /= base;
	      if (n1 == 0 && msize == 0)
		break;
	    }
#endif
	  *str++ = num_to_text[cy_limb];
	  digits_computed_so_far++;
	}
    }

  /* We might have some trailing '0' here in two cases.
     1. We converted a number without fraction.
     2. The fraction was zero (i.e., the number was not strictly normalized).
   */
  while (str[-1] == '0')
    str--;

  *str = 0;
  return digit_ptr;
}

