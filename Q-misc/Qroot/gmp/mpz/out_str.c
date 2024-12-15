/* mpz_out_str(stream, base, integer) -- Output to STREAM the multi prec.
   integer INTEGER in base BASE.

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

#include <stdio.h>
#include "gmp.h"
#include "gmp-impl.h"

void
#if __STDC__
mpz_out_str (FILE *stream, int base, mpz_srcptr x)
#else
mpz_out_str (stream, base, x)
     FILE *stream;
     int base;
     mpz_srcptr x;
#endif
{
  mp_ptr xp;
  mp_size_t x_size = x->size;
  unsigned char *str;
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

  if (x_size == 0)
    {
      fputc ('0', stream);
      return;
    }
  if (x_size < 0)
    {
      fputc ('-', stream);
      x_size = -x_size;
    }

  str_size = ((size_t) (x_size * BITS_PER_MP_LIMB
			* __mp_bases[base].chars_per_bit_exactly)) + 3;
  str = (unsigned char *) alloca (str_size);

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

  /* Translate to printable chars.  */
  for (i = 0; i < str_size; i++)
    str[i] = num_to_text[str[i]];
  str[str_size] = 0;

  fputs (str, stream);
}
