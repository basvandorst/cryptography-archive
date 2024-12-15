/* mpz_size_raw -- Give the size of a MP_INT when converted to raw format.

Copyright (C) 1992 Bennet Yee.

This file is part of an extension of the GNU MP Library.

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
#include "gmp-mparam.h"
#include "gmp-ext.h"

#if	__STDC__
int		mpz_size_raw(const MP_INT *x)
#else
int		mpz_size_raw(x)
const MP_INT	*x;
#endif
{
	/*
	 * See mpz_put_raw/mpz_out_raw -- use 4 rather than sizeof(mp_size)
	 * to allow for eventual migration to non-32-bit machines.
	 *
	 * Probably should be a macro or inline'd function.
	 */
	return x->size * BYTES_PER_MP_LIMB + 4;
}
