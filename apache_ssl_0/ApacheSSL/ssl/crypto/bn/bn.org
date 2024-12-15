/* lib/bn/bn.h */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_BN_H
#define HEADER_BN_H

#ifndef _Windows
#define BN_LLONG
#endif
#define RECP_MUL_MOD
#undef BN_POINTER

/* Only one for the following should be defined */
#undef SIXTY_FOUR_BIT
#define THIRTY_TWO_BIT
#undef SIXTEEN_BIT

#if !defined(SIXTY_FOUR_BIT) && !defined(THIRTY_TWO_BIT) && !defined(SIXTEEN_BIT)
#if sizeof(unsigned long) == 8
#undef SIXTY_FOUR_BIT
#endif
#if sizeof(unsigned long) == 4
#define THIRTY_TWO_BIT
#endif
#if sizeof(unsigned int) == 2
#define THIRTY_TWO_BIT
#undef SIXTEEN_BIT
#endif
#endif

/* assuming long is 64bit - this is the DEC Alpha */
#ifdef SIXTY_FOUR_BIT
#define BN_ULLONG	unsigned long long
#define BN_ULONG	unsigned long
#define BN_LONG		long
#define BN_BITS		128
#define BN_BYTES	8
#define BN_BITS2	64
#define BN_BITS4	32
#define BN_MASK2	(0xffffffffffffffffL)
#define BN_MASK2l	(0xffffffffL)
#define BN_MASK2h	(0xffffffff00000000L)
#define BN_MASK2h1	(0xffffffff80000000L)
#define BN_CBIT		(0x10000000000000000LL)
#define BN_TBIT		(0x8000000000000000)
#define BN_NOT_MASK2 ((unsigned long long)0xffffffffffffffff0000000000000000LL)
#endif

#ifdef THIRTY_TWO_BIT
#define BN_ULLONG	unsigned long long
#define BN_ULONG	unsigned long
#define BN_LONG		long
#define BN_BITS		64
#define BN_BYTES	4
#define BN_BITS2	32
#define BN_BITS4	16
#define BN_MASK2	(0xffffffffL)
#define BN_MASK2l	(0xffff)
#define BN_MASK2h1	(0xffff8000L)
#define BN_MASK2h	(0xffff0000L)
#define BN_CBIT		((unsigned long long)0x100000000LL)
#define BN_TBIT		(0x80000000L)
#define BN_NOT_MASK2	((unsigned long long)0xffffffff00000000LL)
#endif

#ifdef SIXTEEN_BIT
#define BN_ULLONG	unsigned long
#define BN_ULONG	unsigned short
#define BN_LONG		short
#define BN_BITS		32
#define BN_BYTES	2
#define BN_BITS2	16
#define BN_BITS4	8
#define BN_MASK2	(0xffff)
#define BN_MASK2l	(0xff)
#define BN_MASK2h1	(0xff80)
#define BN_MASK2h	(0xff00)
#define BN_CBIT		((unsigned long)0x10000L)
#define BN_TBIT		(0x8000)
#define BN_NOT_MASK2	((unsigned long)0xffff0000L)
#endif

#define BN_DEFAULT_BITS	1200

#ifdef BIGNUM
#undef BIGNUM
#endif

typedef struct bignum_st
	{
	BN_ULONG *d;	/* Pointer to an array of 'BN_BITS2' bit chunks. */
	int top;	/* Index of last used d +1. */
	/* The next are internal book keeping for bn_expand. */
	int max;	/* Size of the d array. */
	int neg;
	} BIGNUM;

#define BN_CTX_NUM	12
typedef struct bignum_ctx
	{
	int tos;
	BIGNUM *bn[BN_CTX_NUM];
	} BN_CTX;

#define BN_prime_checks		(5)

#define BN_num_bytes(a)	((BN_num_bits(a)+7)/8)
#define BN_is_zero(a)	(((a)->top <= 1) && ((a)->d[0] == 0))
#define BN_is_one(a)	(((a)->top == 1) && ((a)->d[0] == 1))
#define BN_is_word(a,w)	(((a)->top == 1) && ((a)->d[0] == (w)))
#define BN_one(a)	(BN_set_word((a),1))
#define BN_zero(a)	(BN_set_word((a),0))

#define bn_fix_top(a) \
	{ \
	BN_ULONG *l; \
	for (l= &((a)->d[(a)->top-1]); (a)->top > 0; (a)->top--) \
		if (*(l--)) break; \
	}

#define bn_expand(n,b) ((((b)/BN_BITS2) <= (n)->max)?(n):bn_expand2((n),(b)))

extern BIGNUM *BN_value_one;

#ifndef NOPROTO
BN_CTX *BN_CTX_new(void);
void	BN_CTX_free(BN_CTX *c);
int     BN_rand(BIGNUM *rnd, int bits, int top,int bottom);
int	BN_num_bits(BIGNUM *a);
BIGNUM *BN_new(void);
void	BN_clear_free(BIGNUM *a);
BIGNUM *BN_copy(BIGNUM *a, BIGNUM *b);
BIGNUM *BN_bin2bn(unsigned char *s,int len,BIGNUM *ret);
int	BN_bn2bin(BIGNUM *a, unsigned char *to);
int	BN_sub(BIGNUM *r, BIGNUM *a, BIGNUM *b);
int	BN_add(BIGNUM *r, BIGNUM *a, BIGNUM *b);
int	BN_mod(BIGNUM *rem, BIGNUM *m, BIGNUM *d, BN_CTX *ctx);
int	BN_div(BIGNUM *dv, BIGNUM *rem, BIGNUM *m, BIGNUM *d, BN_CTX *ctx);
int	BN_mul(BIGNUM *r, BIGNUM *a, BIGNUM *b);
int	BN_sqr(BIGNUM *r, BIGNUM *a,BN_CTX *ctx);
BN_ULONG BN_mod_word(BIGNUM *a, unsigned long w);
int	BN_add_word(BIGNUM *a, unsigned long w);
int	BN_set_word(BIGNUM *a, unsigned long w);
int	BN_cmp(BIGNUM *a, BIGNUM *b);
void	BN_free(BIGNUM *a);
int	BN_is_bit_set(BIGNUM *a, int n);
int	BN_lshift(BIGNUM *r, BIGNUM *a, int n);
int	BN_lshift1(BIGNUM *r, BIGNUM *a);
int	BN_mod_exp(BIGNUM *r, BIGNUM *a, BIGNUM *p, BIGNUM *m,BN_CTX *ctx);
int	BN_mask_bits(BIGNUM *a,int n);
int	BN_mod_mul_reciprocal(BIGNUM *r, BIGNUM *x, BIGNUM *y, BIGNUM *m, 
	BIGNUM *i, int nb, BN_CTX *ctx);
int	BN_mod_mul(BIGNUM *ret, BIGNUM *a, BIGNUM *b, BIGNUM *m,
	BN_CTX *ctx);
void	BN_print(FILE *fp, BIGNUM *a);
int	BN_reciprocal(BIGNUM *r, BIGNUM *m, BN_CTX *ctx);
int	BN_rshift(BIGNUM *r, BIGNUM *a, int n);
int	BN_rshift1(BIGNUM *r, BIGNUM *a);
void	BN_clear(BIGNUM *a);
BIGNUM *bn_expand2(BIGNUM *b, int bits);
BIGNUM *BN_dup(BIGNUM *a);
int	BN_ucmp(BIGNUM *a, BIGNUM *b);
int	BN_set_bit(BIGNUM *a, int n);
int	BN_clear_bit(BIGNUM *a, int n);
char *	BN_bn2ascii(BIGNUM *a);
int	BN_gcd(BIGNUM *r,BIGNUM *in_a,BIGNUM *in_b,BN_CTX *ctx);
BIGNUM *BN_mod_inverse(BIGNUM *a, BIGNUM *n,BN_CTX *ctx);
BIGNUM *BN_generate_prime(int bits,int strong,BIGNUM *add,
		BIGNUM *rem,void (*callback)());
int	BN_is_prime(BIGNUM *p,int nchecks,void (*callback)(),BN_CTX *ctx);
void	ERR_load_BN_strings(void );
BN_ULONG bn_mul_add_word(BN_ULONG *rp,BN_ULONG *ap, int num, BN_ULONG w);
void	bn_sqr_words(BN_ULONG *rp,BN_ULONG *ap,int num);

#else

BN_CTX *BN_CTX_new();
void	BN_CTX_free();
int     BN_rand();
int	BN_num_bits();
BIGNUM *BN_new();
void	BN_clear_free();
BIGNUM *BN_copy();
BIGNUM *BN_bin2bn();
int	BN_bn2bin();
int	BN_sub();
int	BN_add();
int	BN_mod();
int	BN_div();
int	BN_mul();
int	BN_sqr();
BN_ULONG BN_mod_word();
int	BN_add_word();
int	BN_set_word();
int	BN_cmp();
void	BN_free();
int	BN_is_bit_set();
int	BN_lshift();
int	BN_lshift1();
int	BN_mod_exp();
int	BN_mask_bits();
int	BN_mod_mul_reciprocal();
int	BN_mod_mul();
void	BN_print();
int	BN_reciprocal();
int	BN_rshift();
int	BN_rshift1();
void	BN_clear();
BIGNUM *bn_expand2();
BIGNUM *BN_dup();
int	BN_ucmp();
int	BN_set_bit();
int	BN_clear_bit();
char *	BN_bn2ascii();
int	BN_gcd();
BIGNUM *BN_mod_inverse();
BIGNUM *BN_generate_prime();
int	BN_is_prime();
void	ERR_load_BN_strings();
BN_ULONG bn_mul_add_word();

#endif

/* BEGIN ERROR CODES */
/* Error codes for the BN functions. */

/* Function codes. */
#define BN_F_BN_BN2ASCII				 100
#define BN_F_BN_CTX_NEW					 101
#define BN_F_BN_DIV					 102
#define BN_F_BN_EXPAND					 103
#define BN_F_BN_MOD_INVERSE				 104
#define BN_F_BN_MOD_MUL_RECIPROCAL			 105
#define BN_F_BN_NEW					 106
#define BN_F_BN_RAND					 107

/* Reason codes. */
#define BN_R_BAD_RECIPROCAL				 100
#define BN_R_DIV_BY_ZERO				 101
#define BN_R_NO_INVERSE					 102

#endif
