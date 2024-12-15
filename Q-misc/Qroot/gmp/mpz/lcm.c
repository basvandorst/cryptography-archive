/* mpz_lcm -- Calculate the least common multiple of two integers.

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
/* #include "longlong.h" */

/* Prototype LCM implementation.  We compute (U / GCD(U,V)) * V.  Would it
   be faster to compute (V / GCD(U,V)) * U instead sometimes?  */

void
#if __STDC__
mpz_lcm (mpz_ptr w, mpz_srcptr u, mpz_srcptr v)
#else
mpz_lcm (w, u, v)
     mpz_ptr w;
     mpz_srcptr u;
     mpz_srcptr v;
#endif
{
  mp_size_t usize, vsize, tsize, qsize, wsize;
  mp_ptr up, uup, vp, tp, qp;
  mp_limb qhigh_limb;

  usize = ABS (u->size);
  vsize = ABS (v->size);
  up = u->d;
  vp = v->d;

  /* XXX Is it enough to allocate just MIN (usize, vsize), i.e., the max
     result size?  */
  tp = (mp_ptr) alloca (MAX (usize, vsize) * BYTES_PER_MP_LIMB);
  /* XXX Doesn't obey preconditions of mpn_gcd.
     XXX Does mpn_gcd clobber it's inputs?  */
  tsize = mpn_gcd (tp, up, usize, vp, vsize);

  /* XXX Should probably divide the smaller of U and V with T, since we'd
     need less temporary storage and less time in mpn_divmod.  */
  uup = (mp_ptr) alloca (usize * BYTES_PER_MP_LIMB);
  MPN_COPY (uup, up, usize);
  qhigh_limb = mpn_divmod (qp, uup, usize, tp, tsize);
  qsize = usize - tsize;
  if (qhigh_limb != 0)
    qp[qsize++] = qhigh_limb;
  mpn_mul (tp, qp, qsize, vp, vsize);

  tsize = qsize + vsize;
  tsize -= tp[tsize - 1] == 0;

  if (w->alloc < tsize)
    _mpz_realloc (w, tsize);
  MPN_COPY (w->d, tp, tsize);
  w->size = tsize;
}
