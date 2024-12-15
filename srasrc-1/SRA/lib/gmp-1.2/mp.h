/* mp.h -- Definitions for Berkeley compatible multiple precision functions.

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

#ifndef __MP_H__
#define __MP_H__

#define __GNU_MP__

#ifndef __GMP_H__
#define __need_size_t
#include <stddef.h>
#endif

#ifndef MP_INT
#ifndef __MP_SMALL__
typedef struct
{
  long int alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  long int size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  unsigned long int *d;		/* Pointer to the limbs.  */
} __MP_INT;
#else
typedef struct
{
  short int alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  short int size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  unsigned long int *d;		/* Pointer to the limbs.  */
} __MP_INT;
#endif
#endif

#define MINT __MP_INT

#ifdef __STDC__
void mp_set_memory_functions (void *(*)(size_t),
					 void *(*)(void *, size_t, size_t),
					 void (*)(void *, size_t));
MINT *itom (signed short int initial_value);
MINT *xtom (const char *initial_value);
void move (const MINT *, MINT *);
void madd (const MINT *src_1, const MINT *src_2, MINT *destination);
void msub (const MINT *src_1, const MINT *src_2, MINT *destination);
void mult (const MINT *src_1, const MINT *src_2, MINT *destination);
void mdiv (const MINT *dividend, const MINT *divisor,
	   MINT *quotient, MINT *remainder);
void sdiv (const MINT *dividend, signed short int divisor,
	   MINT *quotient, signed short int *remainder);
void msqrt (const MINT *operand, MINT *root, MINT *remainder);
void pow (const MINT *base, const MINT *exp, const MINT *mod, MINT *dest);
void rpow (const MINT *base, signed short int exp, MINT *dest);
void gcd (const MINT *operand1, const MINT *operand2, MINT *res);
int mcmp (const MINT *operand1, const MINT *operand2);
void min (MINT *dest);
void mout (const MINT *src);
char *mtox (const MINT *operand);
void mfree (MINT *operand);

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

#endif /* __MP_H__ */
