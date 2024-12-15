/* mpz_get_str (string, base, mp_src) -- Convert the multiple precision
   number MP_SRC to a string STRING of base BASE.  If STRING is NULL
   allocate space for the result.  In any case, return a pointer to the
   result.  If STRING is not NULL, the caller must ensure enough space is
   available to store the result.

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

char *
#if __STDC__
mpz_get_str (char *res_str, int base, mpz_srcptr x)
#else
mpz_get_str (res_str, base, x)
     char *res_str;
     int base;
     mpz_srcptr x;
#endif
{
  mp_ptr xp;
  mp_size_t x_size = x->size;
  unsigned char *str;
  char *return_str;
  size_t str_size;
  char *num_to_text;
  int i;

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

  /* We allways allocate space for the string.  If the caller passed a
     NULL pointer for RES_STR, we allocate permanent space and return
     a pointer to that to the caller.  */
  str_size = ((size_t) (ABS (x_size) * BITS_PER_MP_LIMB
			* __mp_bases[base].chars_per_bit_exactly)) + 3;
  if (res_str == 0)
    {
      /* We didn't get a string from the user.  Allocate one (and return
	 a pointer to it).  */
      res_str = (char *) (*_mp_allocate_func) (str_size);
      /* Make str, the variable used for raw result from mpn_get_str,
	 point to the same string, but just after a possible minus sign.  */
      str = (unsigned char *) res_str + 1;
    }
  else
    {
      /* Use alloca to get temporary space, since we need a few extra bytes
	 that we can't expect to caller to supply us with.  */
      str = (unsigned char *) alloca (str_size);
    }

  return_str = res_str;

  if (x_size == 0)
    {
      res_str[0] = '0';
      res_str[1] = 0;
      return res_str;
    }
  if (x_size < 0)
    {
      *res_str++ = '-';
      x_size = -x_size;
    }

  /* Move the number to convert into temporary space, since mpn_get_str
     clobbers its argument + needs one extra high limb....  */
  xp = (mp_ptr) alloca ((x_size + 1) * BYTES_PER_MP_LIMB);
  MPN_COPY (xp, x->d, x_size);

  str_size = mpn_get_str (str, base, xp, x_size);

  /* mpn_get_str might make some leading zeros.  Skip them.  */
  while (*str == 0)
    {
      str_size--;
      str++;
    }

  /* Translate result to printable chars and move result to RES_STR.  */
  for (i = 0; i < str_size; i++)
    res_str[i] = num_to_text[str[i]];
  res_str[str_size] = 0;

  return return_str;
}
