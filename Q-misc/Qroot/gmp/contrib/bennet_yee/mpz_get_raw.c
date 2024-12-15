/* mpz_put_raw -- Convert a MP_INT to a raw, but endianess-independent format.

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

#if __STDC__
void		mpz_get_raw(unsigned char *buf, const MP_INT *x)
#else
void		mpz_get_raw(buf, x)
unsigned char *buf;
const MP_INT *x;
#endif
{
	int i;
	mp_size s;
	mp_size xsize = x->size;
	mp_srcptr xp = x->d;
	mp_size out_size = xsize * BYTES_PER_MP_LIMB;

	/*
	 * We could rewrite this in terms of htonl and memcpy,
	 * but we might as well leave it looking more like the code
	 * in mpz_{out,inp}_raw.
	 * Furthermore, longs might not be 32 bits....
	 */
	for (i = 4; --i >= 0; )
		*buf++ = out_size >> (i * BITS_PER_CHAR);

	/*
	 * Output from the most significant digit
	 * to the least significant digit,
	 * with each digit also output
	 * in decreasing significance order.
	 */
	for (s = xsize; --s >= 0; ) {
		mp_limb x_digit;

		x_digit = xp[s];
		for (i = BYTES_PER_MP_LIMB; --i >= 0; )
			*buf++ = x_digit >> (i * BITS_PER_CHAR);
	}
}
