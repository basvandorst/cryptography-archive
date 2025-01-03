/* mpz_cmp(u,v) -- Compare U, V.  Return postive, zero, or negative
   based on if U > V, U == V, or U < V.

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

#ifdef BERKELEY_MP
#include "mp.h"
#endif
#include "gmp.h"
#include "gmp-impl.h"

#ifndef BERKELEY_MP
int
#if __STDC__
mpz_cmp (mpz_srcptr u, mpz_srcptr v)
#else
mpz_cmp (u, v)
     mpz_srcptr u;
     mpz_srcptr v;
#endif
#else /* BERKELEY_MP */
int
#if __STDC__
mcmp (mpz_srcptr u, mpz_srcptr v)
#else
mcmp (u, v)
     mpz_srcptr u;
     mpz_srcptr v;
#endif
#endif /* BERKELEY_MP */
{
  mp_size_t usize = u->size;
  mp_size_t vsize = v->size;
  mp_size_t size;
  mp_size_t i;
  mp_limb a, b;
  mp_srcptr up, vp;

  if (usize != vsize)
    return usize - vsize;

  if (usize == 0)
    return 0;

  size = ABS (usize);

  up = u->d;
  vp = v->d;

  i = size - 1;
  do
    {
      a = up[i];
      b = vp[i];
      i--;
      if (i < 0)
	break;
    }
  while (a == b);

  if (a == b)
    return 0;

  if ((a < b) == (usize < 0))
    return 1;
  else
    return -1;
}
