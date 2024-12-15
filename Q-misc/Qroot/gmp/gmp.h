/* gmp.h -- Definitions for GNU multiple precision functions.

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

#ifndef __GMP_H__

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

#ifdef _SHORT_LIMB
typedef unsigned int		mp_limb;
typedef int			mp_limb_signed;
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

/* User-visible types.  */
typedef __mpz_struct MP_INT;
typedef __mpz_struct mpz_t[1];

/* Structure for rational numbers.  Zero is represented as 0/any, i.e.
   the denominator is ignored.  Negative numbers have the sign in
   the numerator.  */
typedef struct
{
  __mpz_struct num;
  __mpz_struct den;
#if 0
  long int num_alloc;		/* Number of limbs allocated
				   for the numerator.  */
  long int num_size;		/* The absolute value of this field is the
				   length of the numerator; the sign is the
				   sign of the entire rational number.  */
  mp_ptr num;			/* Pointer to the numerator limbs.  */
  long int den_alloc;		/* Number of limbs allocated
				   for the denominator.  */
  long int den_size;		/* Length of the denominator.  (This field
				   should always be positive.) */
  mp_ptr den;			/* Pointer to the denominator limbs.  */
#endif
} __mpq_struct;

typedef __mpq_struct MP_RAT;
typedef __mpq_struct mpq_t[1];

typedef struct
{
  mp_size_t alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  mp_size_t prec;		/* Max precision, in number of `mp_limb's.
				   Set by mpf_init and modified by
				   mpf_set_prec.  */
  mp_size_t size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  mp_exp_t exp;			/* Exponent, in the base of `mp_limb'.  */
  mp_limb *d;			/* Pointer to the limbs.  */
} __mpf_struct;

/* typedef __mpf_struct MP_FLOAT; */
typedef __mpf_struct mpf_t[1];

/* Types for function declarations in gmp files.  */
/* ??? Should not pollute user name space ??? */
typedef __gmp_const __mpz_struct *mpz_srcptr;
typedef __mpz_struct *mpz_ptr;
typedef __gmp_const __mpf_struct *mpf_srcptr;
typedef __mpf_struct *mpf_ptr;


#ifdef __STDC__
void mp_set_memory_functions (void *(*) (size_t),
			      void *(*) (void *, size_t, size_t),
			      void (*) (void *, size_t));

/**************** Integer (i.e. Z) routines.  ****************/

void mpz_init (mpz_ptr);
void mpz_set (mpz_ptr, mpz_srcptr);
void mpz_set_ui (mpz_ptr, unsigned long int);
void mpz_set_si (mpz_ptr, signed long int);
int mpz_set_str (mpz_ptr, const char *, int);
void mpz_init_set (mpz_ptr, mpz_srcptr);
void mpz_init_set_ui (mpz_ptr, unsigned long int);
void mpz_init_set_si (mpz_ptr, signed long int);
int mpz_init_set_str (mpz_ptr, const char *, int);
unsigned long int mpz_get_ui (mpz_srcptr);
signed long int mpz_get_si (mpz_srcptr);
char * mpz_get_str (char *, int, mpz_srcptr);
void mpz_clear (mpz_ptr);
void * _mpz_realloc (mpz_ptr, mp_size_t);
void mpz_add (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_add_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_sub (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_sub_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_mul (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mul_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_div (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_div_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_mod (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mod_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_divmod (mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_divmod_ui (mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_mdiv (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_mdiv_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_mmod (mpz_ptr, mpz_srcptr, mpz_srcptr);
unsigned long int mpz_mmod_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_mdivmod (mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
unsigned long int mpz_mdivmod_ui (mpz_ptr, mpz_ptr, mpz_srcptr,
				  unsigned long int);
void mpz_sqrt (mpz_ptr, mpz_srcptr);
void mpz_sqrtrem (mpz_ptr, mpz_ptr, mpz_srcptr);
int mpz_perfect_square_p (mpz_srcptr);
int mpz_probab_prime_p (mpz_srcptr, int);
void mpz_powm (mpz_ptr, mpz_srcptr, mpz_srcptr, mpz_srcptr);
void mpz_powm_ui (mpz_ptr, mpz_srcptr, unsigned long int, mpz_srcptr);
void mpz_pow_ui (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_fac_ui (mpz_ptr, unsigned long int);
void mpz_gcd (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_gcdext (mpz_ptr, mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr);
int mpz_divides(mpz_srcptr, mpz_srcptr);
void mpz_neg (mpz_ptr, mpz_srcptr);
void mpz_com (mpz_ptr, mpz_srcptr src);
void mpz_abs (mpz_ptr, mpz_srcptr);
int mpz_cmp (mpz_srcptr, mpz_srcptr);
int mpz_cmp_ui (mpz_srcptr, unsigned long int);
int mpz_cmp_si (mpz_srcptr, signed long int);
void mpz_mul_2exp (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_div_2exp (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_mod_2exp (mpz_ptr, mpz_srcptr, unsigned long int);
void mpz_and (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_ior (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_xor (mpz_ptr, mpz_srcptr, mpz_srcptr);
void mpz_not (mpz_ptr, mpz_srcptr);

#ifdef FILE
void mpz_inp_raw (mpz_ptr, FILE *);
void mpz_inp_str (mpz_ptr, FILE *, int);
void mpz_out_raw (FILE *, mpz_srcptr);
void mpz_out_str (FILE *, int, mpz_srcptr);
#endif

void mpz_array_init (mpz_ptr [], size_t, mp_size_t);
void mpz_random (mpz_ptr, mp_size_t);
void mpz_random2 (mpz_ptr, mp_size_t);
size_t mpz_size (mpz_srcptr);
size_t mpz_sizeinbase (mpz_srcptr, int);

/**************** Rational (i.e. Q) routines.  ****************/

void mpq_init (MP_RAT *);
void mpq_clear (MP_RAT *);
void mpq_set (MP_RAT *, const MP_RAT *);
void mpq_set_ui (MP_RAT *, unsigned long int, unsigned long int);
void mpq_set_si (MP_RAT *, signed long int, unsigned long int);
void mpq_add (MP_RAT *, const MP_RAT *, const MP_RAT *);
void mpq_sub (MP_RAT *, const MP_RAT *, const MP_RAT *);
void mpq_mul (MP_RAT *, const MP_RAT *, const MP_RAT *);
void mpq_div (MP_RAT *, const MP_RAT *, const MP_RAT *);
void mpq_neg (MP_RAT *, const MP_RAT *);
int mpq_cmp (const MP_RAT *, const MP_RAT *);
void mpq_inv (MP_RAT *, const MP_RAT *);
void mpq_set_num (MP_RAT *, mpz_srcptr);
void mpq_set_den (MP_RAT *, mpz_srcptr);
void mpq_get_num (mpz_ptr, const MP_RAT *);
void mpq_get_den (mpz_ptr, const MP_RAT *);

/**************** Float (i.e. F) routines.  ****************/

char *mpf_get_str (char *, mp_exp_t *, int, size_t, mpf_srcptr);
mp_size_t mpf_set_default_prec (mp_size_t);
void mpf_init (mpf_ptr);
void mpf_init_set (mpf_ptr, mpf_srcptr);
void mpf_init_set_ui (mpf_ptr, unsigned int);
void mpf_init_set_si (mpf_ptr, int);
void mpf_set (mpf_ptr, mpf_srcptr);
void mpf_add (mpf_ptr, mpf_srcptr, mpf_srcptr);
void mpf_add_ui (mpf_ptr, mpf_srcptr, unsigned int);
void mpf_sub (mpf_ptr, mpf_srcptr, mpf_srcptr);
void mpf_sub_ui (mpf_ptr, mpf_srcptr, unsigned int);
void mpf_ui_sub (mpf_ptr, unsigned int, mpf_srcptr);
void mpf_mul (mpf_ptr, mpf_srcptr, mpf_srcptr);
void mpf_mul_ui (mpf_ptr, mpf_srcptr, unsigned int);
void mpf_div (mpf_ptr, mpf_srcptr, mpf_srcptr);
void mpf_div_ui (mpf_ptr, mpf_srcptr, unsigned int);
void mpf_ui_div (mpf_ptr, unsigned int, mpf_srcptr);
void mpf_sqrt (mpf_ptr, mpf_srcptr);
void mpf_sqrt_ui (mpf_ptr, unsigned int);
int mpf_cmp (mpf_srcptr, mpf_srcptr);


/************ Low level positive-integer (i.e. N) routines.  ************/

/* mp_limb mpn_add (mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t); */
mp_limb mpn_add_n (mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
/* mp_limb mpn_add_1 (mp_ptr, mp_srcptr, mp_size_t, mp_limb); */

/* mp_limb mpn_sub (mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t); */
mp_limb mpn_sub_n (mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
/* mp_limb mpn_sub_1 (mp_ptr, mp_srcptr, mp_size_t, mp_limb); */

mp_limb mpn_mul (mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t);
void mpn_mul_n (mp_ptr, mp_srcptr, mp_srcptr, mp_size_t);
mp_limb mpn_mul_1 (mp_ptr, mp_srcptr, mp_size_t, mp_limb);
mp_limb mpn_addmul_1 (mp_ptr, mp_srcptr, mp_size_t, mp_limb);
mp_limb mpn_submul_1 (mp_ptr, mp_srcptr, mp_size_t, mp_limb);

mp_limb mpn_divmod (mp_ptr, mp_ptr, mp_size_t, mp_srcptr, mp_size_t);
mp_limb mpn_divmod_1 (mp_ptr, mp_srcptr, mp_size_t, mp_limb);
mp_limb mpn_mod_1 (mp_srcptr, mp_size_t, mp_limb);

mp_limb mpn_lshift (mp_ptr, mp_srcptr, mp_size_t, unsigned int);
mp_limb mpn_rshift (mp_ptr, mp_srcptr, mp_size_t, unsigned int);
mp_limb mpn_rshiftci (mp_ptr, mp_srcptr, mp_size_t, unsigned int, mp_limb);
mp_size_t mpn_sqrt (mp_ptr, mp_ptr, mp_srcptr, mp_size_t);
int mpn_cmp (mp_srcptr, mp_srcptr, mp_size_t);

void mpn_random2 (mp_ptr, mp_size_t);
mp_size_t mpn_set_str (mp_ptr, const unsigned char *, size_t, int);
size_t mpn_get_str (unsigned char *, int, mp_ptr, mp_size_t);

mp_size_t mpn_bingcd (mp_ptr, mp_ptr, mp_size_t, mp_ptr, mp_size_t);
mp_size_t mpn_accelgcd (mp_ptr, mp_ptr, mp_size_t, mp_ptr, mp_size_t);
mp_size_t mpn_bmod (mp_ptr, mp_size_t, mp_srcptr, mp_size_t);
mp_size_t mpn_bmodgcd (mp_ptr, mp_ptr, mp_size_t, mp_ptr, mp_size_t);
void mpn_compl (mp_ptr, mp_srcptr, mp_size_t);
mp_limb mpn_compl_rsh (mp_ptr, mp_srcptr, mp_size_t, unsigned int);
mp_limb mpn_sub_n_rsh (mp_ptr, mp_srcptr, mp_srcptr, mp_size_t, unsigned int);

#else /* ! __STDC__ */
void mp_set_memory_functions ();

/**************** Integer (i.e. Z) routines.  ****************/

void mpz_init ();
void mpz_set ();
void mpz_set_ui ();
void mpz_set_si ();
int mpz_set_str ();
void mpz_init_set ();
void mpz_init_set_ui ();
void mpz_init_set_si ();
int mpz_init_set_str ();
unsigned long int mpz_get_ui ();
long int mpz_get_si ();
char * mpz_get_str ();
void mpz_clear ();
void * _mpz_realloc ();
void mpz_add ();
void mpz_add_ui ();
void mpz_sub ();
void mpz_sub_ui ();
void mpz_mul ();
void mpz_mul_ui ();
void mpz_div ();
void mpz_div_ui ();
void mpz_mod ();
void mpz_mod_ui ();
void mpz_divmod ();
void mpz_divmod_ui ();
void mpz_mdiv ();
void mpz_mdiv_ui ();
void mpz_mmod ();
unsigned long int mpz_mmod_ui ();
void mpz_mdivmod ();
unsigned long int mpz_mdivmod_ui ();
void mpz_sqrt ();
void mpz_sqrtrem ();
int mpz_perfect_square_p ();
int mpz_probab_prime_p ();
void mpz_powm ();
void mpz_powm_ui ();
void mpz_pow_ui ();
void mpz_fac_ui ();
void mpz_gcd ();
void mpz_gcdext ();
int mpz_divides();
void mpz_neg ();
void mpz_com ();
void mpz_abs ();
int mpz_cmp ();
int mpz_cmp_ui ();
int mpz_cmp_si ();
void mpz_mul_2exp ();
void mpz_div_2exp ();
void mpz_mod_2exp ();
void mpz_and ();
void mpz_ior ();
void mpz_xor ();
void mpz_not ();

#ifdef FILE
void mpz_inp_raw ();
void mpz_inp_str ();
void mpz_out_raw ();
void mpz_out_str ();
#endif

void mpz_array_init ();
void mpz_random ();
void mpz_random2 ();
size_t mpz_size ();
size_t mpz_sizeinbase ();

/**************** Rational (i.e. Q) routines.  ****************/

void mpq_init ();
void mpq_clear ();
void mpq_set ();
void mpq_set_ui ();
void mpq_set_si ();
void mpq_add ();
void mpq_sub ();
void mpq_mul ();
void mpq_div ();
void mpq_neg ();
int mpq_cmp ();
void mpq_inv ();
void mpq_set_num ();
void mpq_set_den ();
void mpq_get_num ();
void mpq_get_den ();

/**************** Float (i.e. F) routines.  ****************/

char *mpf_get_str ();
mp_size_t mpf_set_default_prec ();
void mpf_init ();
void mpf_init_set ();
void mpf_init_set_ui ();
void mpf_init_set_si ();
void mpf_set ();
void mpf_add ();
void mpf_add_ui ();
void mpf_sub ();
void mpf_sub_ui ();
void mpf_ui_sub ();
void mpf_mul ();
void mpf_mul_ui ();
void mpf_div ();
void mpf_div_ui ();
void mpf_ui_div ();
void mpf_sqrt ();
void mpf_sqrt_ui ();
int mpf_cmp ();

/************ Low level positive-integer (i.e. N) routines.  ************/

/* mp_limb mpn_add (); */
mp_limb mpn_add_n ();
/* mp_limb mpn_add_1 (); */

/* mp_limb mpn_sub (); */
mp_limb mpn_sub_n ();
/* mp_limb mpn_sub_1 (); */

mp_limb mpn_mul ();
void mpn_mul_n ();
mp_limb mpn_mul_1 ();
mp_limb mpn_addmul_1 ();
mp_limb mpn_submul_1 ();

mp_limb mpn_divmod ();
mp_limb mpn_divmod_1 ();
mp_limb mpn_mod_1 ();

mp_limb mpn_lshift ();
mp_limb mpn_rshift ();
mp_limb mpn_rshiftci ();
mp_size_t mpn_sqrt ();
int mpn_cmp ();

void mpn_random2 ();
mp_size_t mpn_set_str ();
size_t mpn_get_str ();

#endif /* __STDC__ */

static __gmp_inline mp_limb
#if __STDC__
mpn_add_1 (register mp_ptr res_ptr,
	   register mp_srcptr s1_ptr,
	   register mp_size_t s1_size,
	   register mp_limb s2_limb)
#else
mpn_add_1 (res_ptr, s1_ptr, s1_size, s2_limb)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_limb s2_limb;
#endif
{
  register mp_limb x;

  x = *s1_ptr++;
  s2_limb = x + s2_limb;
  *res_ptr++ = s2_limb;
  if (s2_limb < x)
    {
      while (--s1_size != 0)
	{
	  x = *s1_ptr++ + 1;
	  *res_ptr++ = x;
	  if (x != 0)
	    goto fin;
	}

      return 1;
    }

 fin:
  if (res_ptr != s1_ptr)
    memcpy (res_ptr, s1_ptr, (size_t) (s1_size - 1) * sizeof (mp_limb));
  return 0;
}

static __gmp_inline mp_limb
#if __STDC__
mpn_add (register mp_ptr res_ptr,
	 register mp_srcptr s1_ptr,
	 register mp_size_t s1_size,
	 register mp_srcptr s2_ptr,
	 register mp_size_t s2_size)
#else
mpn_add (res_ptr, s1_ptr, s1_size, s2_ptr, s2_size)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_srcptr s2_ptr;
     register mp_size_t s2_size;
#endif
{
  mp_limb cy_limb = 0;

  if (s2_size != 0)
    cy_limb = mpn_add_n (res_ptr,
			 s1_ptr,
			 s2_ptr,
			 s2_size);

  if (s1_size - s2_size != 0)
    cy_limb =  mpn_add_1 (res_ptr + s2_size,
			  s1_ptr + s2_size,
			  s1_size - s2_size,
			  cy_limb);
  return cy_limb;
}

static __gmp_inline mp_limb
#if __STDC__
mpn_sub_1 (register mp_ptr res_ptr,
	   register mp_srcptr s1_ptr,
	   register mp_size_t s1_size,
	   register mp_limb s2_limb)
#else
mpn_sub_1 (res_ptr, s1_ptr, s1_size, s2_limb)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_limb s2_limb;
#endif
{
  register mp_limb x;

  x = *s1_ptr++;
  s2_limb = x - s2_limb;
  *res_ptr++ = s2_limb;
  if (s2_limb > x)
    {
      while (--s1_size != 0)
	{
	  x = *s1_ptr++;
	  *res_ptr++ = x - 1;
	  if (x != 0)
	    goto fin;
	}

      return 1;
    }

 fin:
  if (res_ptr != s1_ptr)
    memcpy (res_ptr, s1_ptr, (size_t) (s1_size - 1) * sizeof (mp_limb));
  return 0;
}

static __gmp_inline mp_limb
#if __STDC__
mpn_sub (register mp_ptr res_ptr,
	 register mp_srcptr s1_ptr,
	 register mp_size_t s1_size,
	 register mp_srcptr s2_ptr,
	 register mp_size_t s2_size)
#else
mpn_sub (res_ptr, s1_ptr, s1_size, s2_ptr, s2_size)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_srcptr s2_ptr;
     register mp_size_t s2_size;
#endif
{
  mp_limb cy_limb = 0;

  if (s2_size != 0)
    cy_limb = mpn_sub_n (res_ptr,
			 s1_ptr,
			 s2_ptr,
			 s2_size);

  if (s1_size - s2_size != 0)
    cy_limb =  mpn_sub_1 (res_ptr + s2_size,
			  s1_ptr + s2_size,
			  s1_size - s2_size,
			  cy_limb);
  return cy_limb;
}

static __gmp_inline mp_size_t
#if __STDC__
mpn_normal_size (mp_srcptr ptr, mp_size_t size)
#else
mpn_normal_size (ptr, size)
     mp_srcptr ptr;
     mp_size_t size;
#endif
{
  while (size)
    {
      size--;
      if (ptr[size] != 0)
	return size + 1;
    }
  return 0;
}

#define __GNU_MP__
#define __GMP_H__
#endif /* __GMP_H__ */
