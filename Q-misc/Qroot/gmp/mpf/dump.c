/* mpf_dump -- Dump a float to stdout.

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

int __gmp_dump_digits = 40;

void
#if __STDC__
mpf_dump (mpf_srcptr u)
#else
mpf_dump (u)
     mpf_srcptr u;
#endif
{
  mp_exp_t exp;
  char *str;

  str = mpf_get_str (0, &exp, 10, __gmp_dump_digits, u);
  if (str[0] == '-')
    printf ("-0.%s*10**%ld\n", str + 1, exp);
  else
    printf ("0.%s*10**%ld\n", str, exp);
}
