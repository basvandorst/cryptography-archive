/* msqrt(u, root, rem) --  root=floor(sqrt(u)), rem=u-root**2.

Copyright (C) 1991 Free Software Foundation, Inc.

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

/* This code is just correct if "unsigned char" has at least 8 bits.  It
   doesn't help to use CHAR_BIT from limits.h, as the real problem is
   the static arrays.  */

#include "mp.h"
#include "gmp.h"
#include "gmp-impl.h"

void
#if __STDC__
msqrt (const MINT *u, MINT *root, MINT *rem)
#else
msqrt (u, root, rem)
     const MINT *u;
     MINT *root;
     MINT *rem;
#endif
{
  _mpz_impl_sqrt (root, rem, u);
}
