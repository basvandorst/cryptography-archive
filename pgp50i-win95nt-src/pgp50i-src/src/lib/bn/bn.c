/*
 * bn.c - the high-level bignum interface
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by Colin Plumb
 *
 * $Id: bn.c,v 1.18.4.1 1997/06/07 09:49:25 mhw Exp $
 */

#include "bn.h"

/* Functions */
void
bnBegin(struct BigNum *bn)
{
		static int bninit = 0;

		if (!bninit) {
			bnInit();
			bninit = 1;
		}

		bn->ptr = 0;
		bn->size = 0;
		bn->allocated = 0;
}

void
bnSwap(struct BigNum *a, struct BigNum *b)
{
		void *p;
		unsigned t;

		p = a->ptr;
		a->ptr = b->ptr;
		b->ptr = p;

		t = a->size;
		a->size = b->size;
		b->size = t;

		t = a->allocated;
		a->allocated = b->allocated;
		b->allocated = t;
}

int (*bnYield)(void);

void (*bnEnd)(struct BigNum *bn);
int (*bnPrealloc)(struct BigNum *bn, unsigned bits);
int (*bnCopy)(struct BigNum *dest, struct BigNum const *src);
void (*bnNorm)(struct BigNum *bn);
void (*bnExtractBigBytes)(struct BigNum const *bn, unsigned char *dest,
	unsigned lsbyte, unsigned len);
int (*bnInsertBigBytes)(struct BigNum *bn, unsigned char const *src,
	unsigned lsbyte, unsigned len);
void (*bnExtractLittleBytes)(struct BigNum const *bn, unsigned char *dest,
	unsigned lsbyte, unsigned len);
int (*bnInsertLittleBytes)(struct BigNum *bn, unsigned char const *src,
	unsigned lsbyte, unsigned len);
unsigned (*bnLSWord)(struct BigNum const *src);
unsigned (*bnBits)(struct BigNum const *src);
int (*bnAdd)(struct BigNum *dest, struct BigNum const *src);
int (*bnSub)(struct BigNum *dest, struct BigNum const *src);
int (*bnCmpQ)(struct BigNum const *a, unsigned b);
int (*bnSetQ)(struct BigNum *dest, unsigned src);
int (*bnAddQ)(struct BigNum *dest, unsigned src);
int (*bnSubQ)(struct BigNum *dest, unsigned src);
int (*bnCmp)(struct BigNum const *a, struct BigNum const *b);
int (*bnSquare)(struct BigNum *dest, struct BigNum const *src);
int (*bnMul)(struct BigNum *dest, struct BigNum const *a,
	struct BigNum const *b);
int (*bnMulQ)(struct BigNum *dest, struct BigNum const *a, unsigned b);
int (*bnDivMod)(struct BigNum *q, struct BigNum *r, struct BigNum const *n,
	struct BigNum const *d);
int (*bnMod)(struct BigNum *dest, struct BigNum const *src,
	struct BigNum const *d);
unsigned (*bnModQ)(struct BigNum const *src, unsigned d);
int (*bnExpMod)(struct BigNum *result, struct BigNum const *n,
	struct BigNum const *exp, struct BigNum const *mod);
int (*bnDoubleExpMod)(struct BigNum *dest,
	struct BigNum const *n1, struct BigNum const *e1,
	struct BigNum const *n2, struct BigNum const *e2,
	struct BigNum const *mod);
int (*bnTwoExpMod)(struct BigNum *n, struct BigNum const *exp,
	struct BigNum const *mod);
int (*bnGcd)(struct BigNum *dest, struct BigNum const *a,
	struct BigNum const *b);
int (*bnInv)(struct BigNum *dest, struct BigNum const *src,
	struct BigNum const *mod);
int (*bnLShift)(struct BigNum *dest, unsigned amt);
void (*bnRShift)(struct BigNum *dest, unsigned amt);
unsigned (*bnMakeOdd)(struct BigNum *n);
