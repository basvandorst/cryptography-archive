/* mpz_inp_raw -- Input a MP_INT in raw, but endianess, and wordsize
   independent format (as output by mpz_out_raw).

Copyright (C) 1992 Bennet Yee.
Copyright (C) 1991 Free Software Foundation, Inc.

This file is part of an extension to the GNU MP Library.

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
#include "gmp-ext.h"

/*
 * Returns number of bytes consumed; this allows RPCs to pack numbers
 * together as a huge byte vector.  We do the error checking here.
 */
#if __STDC__
int		mpz_set_raw(MP_INT *x, const unsigned char *buf, int bsize)
#else
int		mpz_set_raw(x, buf, bsize)
MP_INT *x;
const unsigned char *buf;
int bsize;
#endif
{
	int i, obsize = bsize;
	mp_size s;
	mp_size xsize = x->size;
	mp_ptr xp = x->d;
	unsigned int c;
	mp_limb x_digit;
	mp_size x_index, x_last;

	if (bsize < 4) return 0;
	xsize = 0;
	for (i = 4; --i >= 0; ) {
		c = *buf++;
		xsize = (xsize << BITS_PER_CHAR) | c;
	}
	bsize -= 4;

	if (bsize < abs(xsize)) return 0;
	if (x->alloc < abs (xsize))
		_mpz_realloc (x, abs (xsize));

	x_digit = 0;
	x_last =
	x_index = (xsize + BYTES_PER_MP_LIMB - 1) / BYTES_PER_MP_LIMB - 1;
	for (s = xsize; --s >= 0; ) {
		i = s % BYTES_PER_MP_LIMB;
		c = *buf++;
		x_digit = (x_digit << BITS_PER_CHAR) | c;
		if (i == 0) {
			xp[x_index--] = x_digit;
			x_digit = 0;
		}
	}
	if (!xp[x_last]) {
		/* invar violated! NOTE that x gets trashed. */
		mpz_clear(x);
		return 0;
	}
	return obsize - bsize;
}
