/*
 * bn64.c - the high-level bignum interface
 *
 * Like bni64.c, this reserves the string "64" for textual replacement.
 * The string must not appear anywhere unless it is intended to be replaced
 * to generate other bignum interface functions.
 *
 * Written by Colin Plumb
 *
 * $Id: bn64.c,v 1.10 1997/10/09 20:56:35 mhw Exp $
 */

#include "pgpConfig.h"

/*
 * Some compilers complain about #if FOO if FOO isn't defined,
 * so do the ANSI-mandated thing explicitly...
 */
#ifndef NO_STRING_H
#define NO_STRING_H 0
#endif
#ifndef HAVE_STRINGS_H
#define HAVE_STRINGS_H 0
#endif
#ifndef NEED_MEMORY_H
#define NEED_MEMORY_H 0
#endif

#if !NO_STRING_H
#include <string.h>	/* for memmove() in bnMakeOdd */
#elif HAVE_STRINGS_H
#include <strings.h>
#endif
#if NEED_MEMORY_H
#include <memory.h>
#endif

/*
 * This was useful during debugging, so it's left in here.
 * You can ignore it.  DBMALLOC is generally undefined.
 */
#ifndef DBMALLOC
#define DBAMLLOC 0
#endif
#if DBMALLOC
#include "../dbmalloc/malloc.h"
#define MALLOCDB malloc_chain_check(1)
#else
#define MALLOCDB (void)0
#endif

#include "bni.h"
#include "bni64.h"
#include "bnimem.h"
#include "bn64.h"
#include "bn.h"

/* Work-arounds for some particularly broken systems */
#include "bnkludge.h"	/* For memmove() */

#include "pgpDebug.h"

/* Functions */
void
bnInit_64(void)
{
	bnEnd = bnEnd_64;
	bnPrealloc = bnPrealloc_64;
	bnCopy = bnCopy_64;
	bnNorm = bnNorm_64;
	bnExtractBigBytes = bnExtractBigBytes_64;
	bnInsertBigBytes = bnInsertBigBytes_64;
	bnExtractLittleBytes = bnExtractLittleBytes_64;
	bnInsertLittleBytes = bnInsertLittleBytes_64;
	bnLSWord = bnLSWord_64;
	bnBits = bnBits_64;
	bnAdd = bnAdd_64;
	bnSub = bnSub_64;
	bnCmpQ = bnCmpQ_64;
	bnSetQ = bnSetQ_64;
	bnAddQ = bnAddQ_64;
	bnSubQ = bnSubQ_64;
	bnCmp = bnCmp_64;
	bnSquare = bnSquare_64;
	bnMul = bnMul_64;
	bnMulQ = bnMulQ_64;
	bnDivMod = bnDivMod_64;
	bnMod = bnMod_64;
	bnModQ = bnModQ_64;
	bnExpMod = bnExpMod_64;
	bnDoubleExpMod = bnDoubleExpMod_64;
	bnTwoExpMod = bnTwoExpMod_64;
	bnGcd = bnGcd_64;
	bnInv = bnInv_64;
	bnLShift = bnLShift_64;
	bnRShift = bnRShift_64;
	bnMakeOdd = bnMakeOdd_64;
}

void
bnEnd_64(BigNum *bn)
{
	if (bn->ptr) {
		BNIFREE((BNWORD64 *)bn->ptr, bn->allocated);
		bn->ptr = 0;
	}
	bn->size = 0;
	bn->allocated = 0;

	MALLOCDB;
}

/* Internal function.  It operates in words. */
static int
bnResize_64(BigNum *bn, unsigned len)
{
	void *p;

	/* Round size up: most mallocs impose 8-byte granularity anyway */
	len = (len + (8/sizeof(BNWORD64) - 1)) & ~(8/sizeof(BNWORD64) - 1);
	p = BNIREALLOC((BNWORD64 *)bn->ptr, bn->allocated, len);
	if (!p)
		return -1;
	bn->ptr = p;
	bn->allocated = len;

	MALLOCDB;

	return 0;
}

#define bnSizeCheck(bn, size) \
	if (bn->allocated < size && bnResize_64(bn, size) < 0) \
		return -1

int
bnPrealloc_64(BigNum *bn, unsigned bits)
{
	bits = (bits + 64-1)/64;
	bnSizeCheck(bn, bits);
	MALLOCDB;
	return 0;
}

int
bnCopy_64(BigNum *dest, BigNum const *src)
{
	bnSizeCheck(dest, src->size);
	dest->size = src->size;
	bniCopy_64((BNWORD64 *)dest->ptr, (BNWORD64 *)src->ptr, src->size);
	MALLOCDB;
	return 0;
}

void
bnNorm_64(BigNum *bn)
{
	bn->size = bniNorm_64((BNWORD64 *)bn->ptr, bn->size);
}

/*
 * Convert a bignum to big-endian bytes.  Returns, in big-endian form, a
 * substring of the bignum starting from lsbyte and "len" bytes long.
 * Unused high-order (leading) bytes are filled with 0.
 */
void
bnExtractBigBytes_64(BigNum const *bn, unsigned char *dest,
                  unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size * (64 / 8);

	/* Fill unused leading bytes with 0 */
	while (s < lsbyte+len && len) {
		*dest++ = 0;
		len--;
	}

	if (len)
		bniExtractBigBytes_64((BNWORD64 *)bn->ptr, dest, lsbyte, len);
	MALLOCDB;
}

int
bnInsertBigBytes_64(BigNum *bn, unsigned char const *src,
                 unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size;
	unsigned words = (len+lsbyte+sizeof(BNWORD64)-1) / sizeof(BNWORD64);

	/* Pad with zeros as required */
	bnSizeCheck(bn, words);

	if (s < words) {
		bniZero_64((BNWORD64 *)bn->ptr BIGLITTLE(-s,+s), words-s);
		s = words;
	}

	bniInsertBigBytes_64((BNWORD64 *)bn->ptr, src, lsbyte, len);

	bn->size = bniNorm_64((BNWORD64 *)bn->ptr, s);

	MALLOCDB;
	return 0;
}


/*
 * Convert a bignum to little-endian bytes.  Returns, in little-endian form, a
 * substring of the bignum starting from lsbyte and "len" bytes long.
 * Unused high-order (trailing) bytes are filled with 0.
 */
void
bnExtractLittleBytes_64(BigNum const *bn, unsigned char *dest,
                  unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size * (64 / 8);

	/* Fill unused leading bytes with 0 */
	while (s < lsbyte+len && len)
		dest[--len] = 0;

	if (len)
		bniExtractLittleBytes_64((BNWORD64 *)bn->ptr, dest,
		                         lsbyte, len);
	MALLOCDB;
}

int
bnInsertLittleBytes_64(BigNum *bn, unsigned char const *src,
                       unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size;
	unsigned words = (len+lsbyte+sizeof(BNWORD64)-1) / sizeof(BNWORD64);

	/* Pad with zeros as required */
	bnSizeCheck(bn, words);

	if (s < words) {
		bniZero_64((BNWORD64 *)bn->ptr BIGLITTLE(-s,+s), words-s);
		s = words;
	}

	bniInsertLittleBytes_64((BNWORD64 *)bn->ptr, src, lsbyte, len);

	bn->size = bniNorm_64((BNWORD64 *)bn->ptr, s);

	MALLOCDB;
	return 0;
}

/* Return the least-significant word of the input. */
unsigned
bnLSWord_64(BigNum const *src)
{
	return src->size ? (unsigned)((BNWORD64 *)src->ptr)[BIGLITTLE(-1,0)]
			 : 0;
}

unsigned
bnBits_64(BigNum const *src)
{
	return bniBits_64((BNWORD64 *)src->ptr, src->size);
}

int
bnAdd_64(BigNum *dest, BigNum const *src)
{
	unsigned s = src->size, d = dest->size;
	BNWORD64 t;

	if (!s)
		return 0;

	bnSizeCheck(dest, s);

	if (d < s) {
		bniZero_64((BNWORD64 *)dest->ptr BIGLITTLE(-d,+d), s-d);
		dest->size = d = s;
		MALLOCDB;
	}
	t = bniAddN_64((BNWORD64 *)dest->ptr, (BNWORD64 *)src->ptr, s);
	MALLOCDB;
	if (t) {
		if (d > s) {
			t = bniAdd1_64((BNWORD64 *)dest->ptr BIGLITTLE(-s,+s),
			               d-s, t);
			MALLOCDB;
		}
		if (t) {
			bnSizeCheck(dest, d+1);
			((BNWORD64 *)dest->ptr)[BIGLITTLE(-1-d,d)] = t;
			dest->size = d+1;
		}
	}
	return 0;
}

/*
 * dest -= src.
 * If dest goes negative, this produces the absolute value of
 * the difference (the negative of the true value) and returns 1.
 * Otherwise, it returls 0.
 */
int
bnSub_64(BigNum *dest, BigNum const *src)
{
	unsigned s = src->size, d = dest->size;
	BNWORD64 t;

	if (d < s  &&  d < (s = bniNorm_64((BNWORD64 *)src->ptr, s))) {
		bnSizeCheck(dest, s);
		bniZero_64((BNWORD64 *)dest->ptr BIGLITTLE(-d,+d), s-d);
		dest->size = d = s;
		MALLOCDB;
	}
	if (!s)
		return 0;
	t = bniSubN_64((BNWORD64 *)dest->ptr, (BNWORD64 *)src->ptr, s);
	MALLOCDB;
	if (t) {
		if (d > s) {
			t = bniSub1_64((BNWORD64 *)dest->ptr BIGLITTLE(-s,+s),
			               d-s, t);
			MALLOCDB;
		}
		if (t) {
			bniNeg_64((BNWORD64 *)dest->ptr, d);
			dest->size = bniNorm_64((BNWORD64 *)dest->ptr,
			                        dest->size);
			MALLOCDB;
			return 1;
		}
	}
	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, dest->size);
	return 0;
}

/*
 * Compare the BigNum to the given value, which must be < 65536.
 * Returns -1. 0 or 1 if a<b, a == b or a>b.
 * a <=> b --> bnCmpQ(a,b) <=> 0
 */
int
bnCmpQ_64(BigNum const *a, unsigned b)
{
	unsigned t;
	BNWORD64 v;

	t = bniNorm_64((BNWORD64 *)a->ptr, a->size);
	/* If a is more than one word long or zero, it's easy... */
	if (t != 1)
		return (t > 1) ? 1 : (b ? -1 : 0);
	v = (unsigned)((BNWORD64 *)a->ptr)[BIGLITTLE(-1,0)];
	return (v > b) ? 1 : ((v < b) ? -1 : 0);
}

int
bnSetQ_64(BigNum *dest, unsigned src)
{
	if (src) {
		bnSizeCheck(dest, 1);

		((BNWORD64 *)dest->ptr)[BIGLITTLE(-1,0)] = (BNWORD64)src;
		dest->size = 1;
	} else {
		dest->size = 0;
	}
	return 0;
}

int
bnAddQ_64(BigNum *dest, unsigned src)
{
	BNWORD64 t;

	if (!dest->size)
		return bnSetQ(dest, src);
	
	t = bniAdd1_64((BNWORD64 *)dest->ptr, dest->size, (BNWORD64)src);
	MALLOCDB;
	if (t) {
		src = dest->size;
		bnSizeCheck(dest, src+1);
		((BNWORD64 *)dest->ptr)[BIGLITTLE(-1-src,src)] = t;
		dest->size = src+1;
	}
	return 0;
}

/*
 * Return value as for bnSub: 1 if subtract underflowed, in which
 * case the return is the negative of the computed value.
 */
int
bnSubQ_64(BigNum *dest, unsigned src)
{
	BNWORD64 t;

	if (!dest->size)
		return bnSetQ(dest, src) < 0 ? -1 : (src != 0);

	t = bniSub1_64((BNWORD64 *)dest->ptr, dest->size, src);
	MALLOCDB;
	if (t) {
		/* Underflow. <= 1 word, so do it simply. */
		bniNeg_64((BNWORD64 *)dest->ptr, 1);
		dest->size = 1;
		return 1;
	}
/* Try to normalize?  Needing this is going to be pretty damn rare. */
/*		dest->size = bniNorm_64((BNWORD64 *)dest->ptr, dest->size); */
	return 0;
}

/*
 * Compare two BigNums.  Returns -1. 0 or 1 if a<b, a == b or a>b.
 * a <=> b --> bnCmp(a,b) <=> 0
 */
int
bnCmp_64(BigNum const *a, BigNum const *b)
{
	unsigned s, t;

	s = bniNorm_64((BNWORD64 *)a->ptr, a->size);
	t = bniNorm_64((BNWORD64 *)b->ptr, b->size);
	
	if (s != t)
		return s > t ? 1 : -1;
	return bniCmp_64((BNWORD64 *)a->ptr, (BNWORD64 *)b->ptr, s);
}

int
bnSquare_64(BigNum *dest, BigNum const *src)
{
	unsigned s;
	BNWORD64 *srcbuf;

	s = bniNorm_64((BNWORD64 *)src->ptr, src->size);
	if (!s) {
		dest->size = 0;
		return 0;
	}
	bnSizeCheck(dest, 2*s);

	if (src == dest) {
		BNIALLOC(srcbuf, BNWORD64, s);
		if (!srcbuf)
			return -1;
		bniCopy_64(srcbuf, (BNWORD64 *)src->ptr, s);
		bniSquare_64((BNWORD64 *)dest->ptr, (BNWORD64 *)srcbuf, s);
		BNIFREE(srcbuf, s);
	} else {
		bniSquare_64((BNWORD64 *)dest->ptr, (BNWORD64 *)src->ptr, s);
	}

	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, 2*s);
	MALLOCDB;
	return 0;
}

int
bnMul_64(BigNum *dest, BigNum const *a, BigNum const *b)
{
	unsigned s, t;
	BNWORD64 *srcbuf;

	s = bniNorm_64((BNWORD64 *)a->ptr, a->size);
	t = bniNorm_64((BNWORD64 *)b->ptr, b->size);

	if (!s || !t) {
		dest->size = 0;
		return 0;
	}

	if (a == b)
		return bnSquare_64(dest, a);

	bnSizeCheck(dest, s+t);

	if (dest == a) {
		BNIALLOC(srcbuf, BNWORD64, s);
		if (!srcbuf)
			return -1;
		bniCopy_64(srcbuf, (BNWORD64 *)a->ptr, s);
		bniMul_64((BNWORD64 *)dest->ptr, srcbuf, s,
		                                 (BNWORD64 *)b->ptr, t);
		BNIFREE(srcbuf, s);
	} else if (dest == b) {
		BNIALLOC(srcbuf, BNWORD64, t);
		if (!srcbuf)
			return -1;
		bniCopy_64(srcbuf, (BNWORD64 *)b->ptr, t);
		bniMul_64((BNWORD64 *)dest->ptr, (BNWORD64 *)a->ptr, s,
		                                 srcbuf, t);
		BNIFREE(srcbuf, t);
	} else {
		bniMul_64((BNWORD64 *)dest->ptr, (BNWORD64 *)a->ptr, s,
		                                 (BNWORD64 *)b->ptr, t);
	}
	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, s+t);
	MALLOCDB;
	return 0;
}

int
bnMulQ_64(BigNum *dest, BigNum const *a, unsigned b)
{
	unsigned s;

	s = bniNorm_64((BNWORD64 *)a->ptr, a->size);
	if (!s || !b) {
		dest->size = 0;
		return 0;
	}
	if (b == 1)
		return bnCopy_64(dest, a);
	bnSizeCheck(dest, s+1);
	bniMulN1_64((BNWORD64 *)dest->ptr, (BNWORD64 *)a->ptr, s, b);
	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, s+1);
	MALLOCDB;
	return 0;
}

int
bnDivMod_64(BigNum *q, BigNum *r, BigNum const *n,
            BigNum const *d)
{
	unsigned dsize, nsize;
	BNWORD64 qhigh;

	dsize = bniNorm_64((BNWORD64 *)d->ptr, d->size);
	nsize = bniNorm_64((BNWORD64 *)n->ptr, n->size);

	if (nsize < dsize) {
		q->size = 0;	/* No quotient */
		r->size = nsize;
		return 0;	/* Success */
	}

	bnSizeCheck(q, nsize-dsize);

	if (r != n) {	/* You are allowed to reduce in place */
		bnSizeCheck(r, nsize);
		bniCopy_64((BNWORD64 *)r->ptr, (BNWORD64 *)n->ptr, nsize);
	}
		
	qhigh = bniDiv_64((BNWORD64 *)q->ptr, (BNWORD64 *)r->ptr, nsize,
	                  (BNWORD64 *)d->ptr, dsize);
	nsize -= dsize;
	if (qhigh) {
		bnSizeCheck(q, nsize+1);
		*((BNWORD64 *)q->ptr BIGLITTLE(-nsize-1,+nsize)) = qhigh;
		q->size = nsize+1;
	} else {
		q->size = bniNorm_64((BNWORD64 *)q->ptr, nsize);
	}
	r->size = bniNorm_64((BNWORD64 *)r->ptr, dsize);
	MALLOCDB;
	return 0;
}

int
bnMod_64(BigNum *dest, BigNum const *src, BigNum const *d)
{
	unsigned dsize, nsize;

	nsize = bniNorm_64((BNWORD64 *)src->ptr, src->size);
	dsize = bniNorm_64((BNWORD64 *)d->ptr, d->size);


	if (dest != src) {
		bnSizeCheck(dest, nsize);
		bniCopy_64((BNWORD64 *)dest->ptr, (BNWORD64 *)src->ptr, nsize);
	}

	if (nsize < dsize) {
		dest->size = nsize;	/* No quotient */
		return 0;
	}

	(void)bniDiv_64((BNWORD64 *)dest->ptr BIGLITTLE(-dsize,+dsize),
	                (BNWORD64 *)dest->ptr, nsize,
	                (BNWORD64 *)d->ptr, dsize);
	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, dsize);
	MALLOCDB;
	return 0;
}

unsigned
bnModQ_64(BigNum const *src, unsigned d)
{
	unsigned s;

	s = bniNorm_64((BNWORD64 *)src->ptr, src->size);
	if (!s)
		return 0;
	
	if (d & (d-1))	/* Not a power of 2 */
		d = bniModQ_64((BNWORD64 *)src->ptr, s, d);
	else
		d = (unsigned)((BNWORD64 *)src->ptr)[BIGLITTLE(-1,0)] & (d-1);
	return d;
}

int
bnExpMod_64(BigNum *dest, BigNum const *n,
	BigNum const *exp, BigNum const *mod)
{
	unsigned nsize, esize, msize;

	nsize = bniNorm_64((BNWORD64 *)n->ptr, n->size);
	esize = bniNorm_64((BNWORD64 *)exp->ptr, exp->size);
	msize = bniNorm_64((BNWORD64 *)mod->ptr, mod->size);

	if (!msize || (((BNWORD64 *)mod->ptr)[BIGLITTLE(-1,0)] & 1) == 0)
		return -1;	/* Illegal modulus! */

	bnSizeCheck(dest, msize);

	/* Special-case base of 2 */
	if (nsize == 1 && ((BNWORD64 *)n->ptr)[BIGLITTLE(-1,0)] == 2) {
		if (bniTwoExpMod_64((BNWORD64 *)dest->ptr,
				    (BNWORD64 *)exp->ptr, esize,
				    (BNWORD64 *)mod->ptr, msize) < 0)
			return -1;
	} else {
		if (bniExpMod_64((BNWORD64 *)dest->ptr,
		                 (BNWORD64 *)n->ptr, nsize,
				 (BNWORD64 *)exp->ptr, esize,
				 (BNWORD64 *)mod->ptr, msize) < 0)
		return -1;
	}

	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, msize);
	MALLOCDB;
	return 0;
}

int
bnDoubleExpMod_64(BigNum *dest,
	BigNum const *n1, BigNum const *e1,
	BigNum const *n2, BigNum const *e2,
	BigNum const *mod)
{
	unsigned n1size, e1size, n2size, e2size, msize;

	n1size = bniNorm_64((BNWORD64 *)n1->ptr, n1->size);
	e1size = bniNorm_64((BNWORD64 *)e1->ptr, e1->size);
	n2size = bniNorm_64((BNWORD64 *)n2->ptr, n2->size);
	e2size = bniNorm_64((BNWORD64 *)e2->ptr, e2->size);
	msize = bniNorm_64((BNWORD64 *)mod->ptr, mod->size);

	if (!msize || (((BNWORD64 *)mod->ptr)[BIGLITTLE(-1,0)] & 1) == 0)
		return -1;	/* Illegal modulus! */

	bnSizeCheck(dest, msize);

	if (bniDoubleExpMod_64((BNWORD64 *)dest->ptr,
		(BNWORD64 *)n1->ptr, n1size, (BNWORD64 *)e1->ptr, e1size,
		(BNWORD64 *)n2->ptr, n2size, (BNWORD64 *)e2->ptr, e2size,
		(BNWORD64 *)mod->ptr, msize) < 0)
		return -1;

	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, msize);
	MALLOCDB;
	return 0;
}

int
bnTwoExpMod_64(BigNum *n, BigNum const *exp,
	BigNum const *mod)
{
	unsigned esize, msize;

	esize = bniNorm_64((BNWORD64 *)exp->ptr, exp->size);
	msize = bniNorm_64((BNWORD64 *)mod->ptr, mod->size);

	if (!msize || (((BNWORD64 *)mod->ptr)[BIGLITTLE(-1,0)] & 1) == 0)
		return -1;	/* Illegal modulus! */

	bnSizeCheck(n, msize);

	if (bniTwoExpMod_64((BNWORD64 *)n->ptr, (BNWORD64 *)exp->ptr, esize,
	                    (BNWORD64 *)mod->ptr, msize) < 0)
		return -1;

	n->size = bniNorm_64((BNWORD64 *)n->ptr, msize);
	MALLOCDB;
	return 0;
}

int
bnGcd_64(BigNum *dest, BigNum const *a, BigNum const *b)
{
	BNWORD64 *tmp;
	unsigned asize, bsize;
	int i;

	/* Kind of silly, but we might as well permit it... */
	if (a == b)
		return dest == a ? 0 : bnCopy(dest, a);

	/* Ensure a is not the same as "dest" */
	if (a == dest) {
		a = b;
		b = dest;
	}

	asize = bniNorm_64((BNWORD64 *)a->ptr, a->size);
	bsize = bniNorm_64((BNWORD64 *)b->ptr, b->size);

	bnSizeCheck(dest, bsize+1);

	/* Copy a to tmp */
	BNIALLOC(tmp, BNWORD64, asize+1);
	if (!tmp)
		return -1;
	bniCopy_64(tmp, (BNWORD64 *)a->ptr, asize);

	/* Copy b to dest, if necessary */
	if (dest != b)
		bniCopy_64((BNWORD64 *)dest->ptr,
			   (BNWORD64 *)b->ptr, bsize);
	if (bsize > asize || (bsize == asize &&
	        bniCmp_64((BNWORD64 *)b->ptr, (BNWORD64 *)a->ptr, asize) > 0))
	{
		i = bniGcd_64((BNWORD64 *)dest->ptr, bsize, tmp, asize,
			&dest->size);
		if (i > 0)	/* Result in tmp, not dest */
			bniCopy_64((BNWORD64 *)dest->ptr, tmp, dest->size);
	} else {
		i = bniGcd_64(tmp, asize, (BNWORD64 *)dest->ptr, bsize,
			&dest->size);
		if (i == 0)	/* Result in tmp, not dest */
			bniCopy_64((BNWORD64 *)dest->ptr, tmp, dest->size);
	}
	BNIFREE(tmp, asize+1);
	MALLOCDB;
	return (i < 0) ? i : 0;
}

int
bnInv_64(BigNum *dest, BigNum const *src,
         BigNum const *mod)
{
	unsigned s, m;
	int i;

	s = bniNorm_64((BNWORD64 *)src->ptr, src->size);
	m = bniNorm_64((BNWORD64 *)mod->ptr, mod->size);

	/* bniInv_64 requires that the input be less than the modulus */
	if (m < s ||
	    (m==s && bniCmp_64((BNWORD64 *)src->ptr, (BNWORD64 *)mod->ptr, s)))
	{
		bnSizeCheck(dest, s + (m==s));
		if (dest != src)
			bniCopy_64((BNWORD64 *)dest->ptr,
			           (BNWORD64 *)src->ptr, s);
		/* Pre-reduce modulo the modulus */
		(void)bniDiv_64((BNWORD64 *)dest->ptr BIGLITTLE(-m,+m),
			        (BNWORD64 *)dest->ptr, s,
		                (BNWORD64 *)mod->ptr, m);
		s = bniNorm_64((BNWORD64 *)dest->ptr, m);
		MALLOCDB;
	} else {
		bnSizeCheck(dest, m+1);
		if (dest != src)
			bniCopy_64((BNWORD64 *)dest->ptr,
			           (BNWORD64 *)src->ptr, s);
	}

	i = bniInv_64((BNWORD64 *)dest->ptr, s, (BNWORD64 *)mod->ptr, m);
	if (i == 0)
		dest->size = bniNorm_64((BNWORD64 *)dest->ptr, m);

	MALLOCDB;
	return i;
}

/*
 * Shift a bignum left the appropriate number of bits,
 * multiplying by 2^amt.
 */
int 
bnLShift_64(BigNum *dest, unsigned amt)
{
	unsigned s = dest->size;
	BNWORD64 carry;

	if (amt % 64) {
		carry = bniLshift_64((BNWORD64 *)dest->ptr, s, amt % 64);
		if (carry) {
			s++;
			bnSizeCheck(dest, s);
			((BNWORD64 *)dest->ptr)[BIGLITTLE(-s,s-1)] = carry;
		}
	}

	amt /= 64;
	if (amt) {
		bnSizeCheck(dest, s+amt);
		memmove((BNWORD64 *)dest->ptr BIGLITTLE(-s-amt, +amt),
		        (BNWORD64 *)dest->ptr BIG(-s),
			s * sizeof(BNWORD64));
		bniZero_64((BNWORD64 *)dest->ptr, amt);
		s += amt;
	}
	dest->size = s;
	MALLOCDB;
	return 0;
}

/*
 * Shift a bignum right the appropriate number of bits,
 * dividing by 2^amt.
 */
void bnRShift_64(BigNum *dest, unsigned amt)
{
	unsigned s = dest->size;

	if (amt >= 64) {
		memmove(
		        (BNWORD64 *)dest->ptr BIG(-s+amt/64),
			(BNWORD64 *)dest->ptr BIGLITTLE(-s, +amt/64),
			s-amt/64 * sizeof(BNWORD64));
		s -= amt/64;
		amt %= 64;
	}

	if (amt)
		(void)bniRshift_64((BNWORD64 *)dest->ptr, s, amt);

	dest->size = bniNorm_64((BNWORD64 *)dest->ptr, s);
	MALLOCDB;
}

/*
 * Shift a bignum right until it is odd, and return the number of
 * bits shifted.  n = d * 2^s.  Replaces n with d and returns s.
 * Returns 0 when given 0.  (Another valid answer is infinity.)
 */
unsigned
bnMakeOdd_64(BigNum *n)
{
	unsigned size;
	unsigned s;	/* shift amount */
	BNWORD64 *p;
	BNWORD64 t;

	p = (BNWORD64 *)n->ptr;
	size = bniNorm_64(p, n->size);
	if (!size)
		return 0;

	t = BIGLITTLE(p[-1],p[0]);
	s = 0;

	/* See how many words we have to shift */
	if (!t) {
		/* Shift by words */
		do {
			
			s++;
			BIGLITTLE(--p,p++);
		} while ((t = BIGLITTLE(p[-1],p[0])) == 0);
		size -= s;
		s *= 64;
		memmove((BNWORD64 *)n->ptr BIG(-size), p BIG(-size),
			size * sizeof(BNWORD64));
		p = (BNWORD64 *)n->ptr;
		MALLOCDB;
	}

	pgpAssert(t);

	/* Now count the bits */
	while ((t & 1) == 0) {
		t >>= 1;
		s++;
	}

	/* Shift the bits */
	if (s & (64-1)) {
		bniRshift_64(p, size, s & (64-1));
		/* Renormalize */
		if (BIGLITTLE(*(p-size),*(p+(size-1))) == 0)
			--size;
	}
	n->size = size;

	MALLOCDB;
	return s;
}
