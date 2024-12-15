/* mp.h -- Definitions for Berkeley compatible multiple precision functions.

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

#ifndef __MP_H__

#ifndef __GNU_MP__
#define __need_size_t
#include <stddef.h>

#ifdef __STDC__
#define __gmp_const const
#else
#define __gmp_const
#endif

#ifdef __GNUC__
#define __gmp_inline inline
#else
#define __gmp_inline
#endif

#ifdef _LONGLONG_LIMB
typedef unsigned long long int	mp_limb;
typedef long long int		mp_limb_signed;
#else
typedef unsigned long int	mp_limb;
typedef long int		mp_limb_signed;
#endif

typedef mp_limb *		mp_ptr;
typedef __gmp_const mp_limb *	mp_srcptr;
typedef int			mp_size_t;
typedef long int		mp_exp_t;

#ifndef __MP_SMALL__
typedef struct
{
  long int alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  long int size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  mp_limb *d;			/* Pointer to the limbs.  */
} __mpz_struct;
#else
typedef struct
{
  short int alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  short int size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  mp_limb *d;			/* Pointer to the limbs.  */
} __mpz_struct;
#endif
#endif /* __GNU_MP__ */

typedef __mpz_struct MINT;

#ifdef __STDC__
void mp_set_memory_functions (void *(*) (size_t),
			      void *(*) (void *, size_t, size_t),
			      void (*) (void *, size_t));
MINT *itom (signed short int);
MINT *xtom (const char *);
void move (const MINT *, MINT *);
void madd (const MINT *, const MINT *, MINT *);
void msub (const MINT *, const MINT *, MINT *);
void mult (const MINT *, const MINT *, MINT *);
void mdiv (const MINT *, const MINT *, MINT *, MINT *);
void sdiv (const MINT *, signed short int, MINT *, signed short int *);
void msqrt (const MINT *, MINT *, MINT *);
void pow (const MINT *, const MINT *, const MINT *, MINT *);
void rpow (const MINT *, signed short int, MINT *);
void gcd (const MINT *, const MINT *, MINT *);
int mcmp (const MINT *, const MINT *);
void min (MINT *);
void mout (const MINT *);
char *mtox (const MINT *);
void mfree (MINT *);

#else

void mp_set_memory_functions ();
MINT *itom ();
MINT *xtom ();
void move ();
void madd ();
void msub ();
void mult ();
void mdiv ();
void sdiv ();
void msqrt ();
void pow ();
void rpow ();
void gcd ();
int mcmp ();
void min ();
void mout ();
char *mtox ();
void mfree ();
#endif

#define __GNU_MP__
#define __MP_H__
#endif /* __MP_H__ */
