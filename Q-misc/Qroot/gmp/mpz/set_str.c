/* mpz_set_str(mp_dest, string, base) -- Convert the \0-terminated
   string STRING in base BASE to multiple precision integer in
   MP_DEST.  Allow white space in the string.  If BASE == 0 determine
   the base in the C standard way, i.e.  0xhh...h means base 16,
   0oo...o means base 8, otherwise assume base 10.

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
#include "longlong.h"

int
#if __STDC__
mpz_set_str (mpz_ptr x, const char *str, int base)
#else
mpz_set_str (x, str, base)
     mpz_ptr x;
     const char *str;
     int base;
#endif
{
  size_t str_size;
  char *s, *begs;
  size_t i;
  mp_size_t xsize;
  int c;
  int negative = 0;

  c = *str++;
  if (c == '-')
    {
      negative = 1;
      c = *str++;
    }

  /* If BASE is 0, try to find out the base by looking at the initial
     characters.  */
  if (base == 0)
    {
      base = 10;
      if (c == '0')
	{
	  base = 8;
	  c = *str++;
	  if (c == 'x' || c == 'X')
	    {
	      base = 16;
	      c = *str++;
	    }
	}
    }

  str_size = strlen (str - 1);
  s = begs = (char *) alloca (str_size + 1);

  for (i = 0; i < str_size; i++)
    {
      if (!isspace (c))
	{
	  if (isdigit (c))
	    c = c - '0';
	  else if (islower (c))
	    c = c - 'a' + 10;
	  else if (isupper (c))
	    c = c - 'A' + 10;
	  *s++ = c;
	}
      c = *str++;
    }

  str_size = s - begs;

  xsize = str_size / __mp_bases[base].chars_per_limb + 1;
  if (x->alloc < xsize)
    _mpz_realloc (x, xsize);

  xsize = mpn_set_str (x->d, (unsigned char *) begs, str_size, base);
  x->size = negative ? -xsize : xsize;

  /* ??? Botch, all of this.  What do we return?  RTFM.  */
}
