/* mpz_gcd -- Calculate the greatest common divisior of two integers.
   Contributed by Ken Weber (kweber@mcs.kent.edu).

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


/* Different algorithms may be chosen by setting mpn_gcd to point to the
   appropriate subroutine.  */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

#if __STDC__
mp_size_t (*mpn_gcd) (mp_ptr, mp_ptr, mp_size_t, mp_ptr, mp_size_t) = mpn_accelgcd;
#else
mp_size_t (*mpn_gcd) () = mpn_accelgcd;
#endif

#ifndef BERKELEY_MP
void
#ifdef __STDC__
mpz_gcd (MP_INT *gz, const MP_INT *uz, const MP_INT *vz)
#else
mpz_gcd (gz, uz, vz)
     MP_INT *gz;
     const MP_INT *uz;
     const MP_INT *vz;
#endif
#else /* BERKELEY_MP */
void
#ifdef __STDC__
gcd (const MP_INT *uz, const MP_INT *vz, MP_INT *gz)
#else
gcd (uz, vz, gz)
     const MP_INT *uz;
     const MP_INT *vz;
     MP_INT *gz;
#endif
#endif /* BERKELEY_MP */
{
  mp_ptr u = uz->d;
  mp_ptr v = vz->d;
  mp_size_t ul = abs(uz->size);
  mp_size_t vl = abs(vz->size);
  mp_size_t g_low_0_limbs;
  int g_low_0_bits_mod_W;	/* # of low 0 bits in lowest nonzero limb */

  /* GCD(0, v) == v.  Set u to v and v to 0  */
  if (ul == 0)
    {
      ul = vl;
      vl = 0;
      u = vz->d;
    }

  /* GCD(u, 0) == u.  */
  if (vl == 0)
    {
      if (gz->alloc < ul)
	_mpz_realloc (gz, ul);
      gz->size = ul;
      if (gz->d != u)
	MPN_COPY (gz->d, u, ul);
      return;
    }

  /*  Eliminate non-zero limbs from u and v and if ul < vl, swap.  */
  while (*u == 0)
    u++;
  ul -= u - uz->d;

  while (*v == 0)
    v++;
  vl -= v - vz->d;

  /*  Calculate number of 0 limbs for gz and 0 bits for gz->d[0],
      eliminate 2s from u, v and move to temporary space.  */
  {
    unsigned long int u_low_0_bits_mod_W;
    unsigned long int v_low_0_bits_mod_W;

    count_trailing_zeros (u_low_0_bits_mod_W, *u);
    count_trailing_zeros (v_low_0_bits_mod_W, *v);

    g_low_0_limbs = u - uz->d;
    g_low_0_bits_mod_W = v_low_0_bits_mod_W;

    if (g_low_0_limbs < v - vz->d)
      g_low_0_bits_mod_W = u_low_0_bits_mod_W;
    else if (g_low_0_limbs > v - vz->d)
      g_low_0_limbs = v - vz->d;
    else if (u_low_0_bits_mod_W < g_low_0_bits_mod_W)
      g_low_0_bits_mod_W = u_low_0_bits_mod_W;

    {
      mp_ptr tmp;
      tmp = u;
      u = (mp_ptr) alloca (sizeof (mp_limb)*ul);
      if (u_low_0_bits_mod_W)
	{
	  mpn_rshift (u, tmp, ul, u_low_0_bits_mod_W);
	  if (u[ul-1] == 0)
	    ul--;
	}
      else
	MPN_COPY (u, tmp, ul);

      tmp = v;
      v = (mp_ptr) alloca (sizeof (mp_limb) * vl);
      if (v_low_0_bits_mod_W)
	{
	  mpn_rshift (v, tmp, vl, v_low_0_bits_mod_W);
	  if (v[vl-1] == 0)
	    vl--;
	}
      else
	MPN_COPY (v, tmp, vl);
    }
  }

  vl = (MPN_MORE_BITS (v, vl, u, ul))
    ? (*mpn_gcd) (v, v, vl, u, ul) : (*mpn_gcd) (v, u, ul, v, vl);

  /*  gz <-- (v, vl) << (g_low_0_limbs*W + g_low_0_bits_mod_W)  */
  gz->size = vl + g_low_0_limbs;
  if (gz->alloc <= gz->size)
    _mpz_realloc (gz, gz->size + 1);
  MPN_ZERO (gz->d, g_low_0_limbs);
  if (g_low_0_bits_mod_W != 0)
    {
      gz->d[gz->size] 
	= mpn_lshift (gz->d + g_low_0_limbs, v, vl, g_low_0_bits_mod_W);
      if (gz->d[gz->size] != 0)
	gz->size++;
    }
  else
    MPN_COPY (gz->d + g_low_0_limbs, v, vl);
}
