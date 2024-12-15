/*
 * bn16.c - the high-level bignum interface
 *
 * Like bni16.c, this reserves the string "16" for textual replacement.
 * The string must not appear anywhere unless it is intended to be replaced
 * to generate other bignum interface functions.
 *
 * Written by Colin Plumb
 *
 * $Id: bn16.c,v 1.5 1997/05/13 01:18:32 mhw Exp $
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
#include "bni16.h"
#include "bnimem.h"
#include "bn16.h"
#include "bn.h"

/* Work-arounds for some particularly broken systems */
#include "bnkludge.h"	/* For memmove() */

#include "pgpDebug.h"

/* Functions */
void
bnInit_16(void)
{
	bnEnd = bnEnd_16;
	bnPrealloc = bnPrealloc_16;
	bnCopy = bnCopy_16;
	bnNorm = bnNorm_16;
	bnExtractBigBytes = bnExtractBigBytes_16;
	bnInsertBigBytes = bnInsertBigBytes_16;
	bnExtractLittleBytes = bnExtractLittleBytes_16;
	bnInsertLittleBytes = bnInsertLittleBytes_16;
	bnLSWord = bnLSWord_16;
	bnBits = bnBits_16;
	bnAdd = bnAdd_16;
	bnSub = bnSub_16;
	bnCmpQ = bnCmpQ_16;
	bnSetQ = bnSetQ_16;
	bnAddQ = bnAddQ_16;
	bnSubQ = bnSubQ_16;
	bnCmp = bnCmp_16;
	bnSquare = bnSquare_16;
	bnMul = bnMul_16;
	bnMulQ = bnMulQ_16;
	bnDivMod = bnDivMod_16;
	bnMod = bnMod_16;
	bnModQ = bnModQ_16;
	bnExpMod = bnExpMod_16;
	bnDoubleExpMod = bnDoubleExpMod_16;
	bnTwoExpMod = bnTwoExpMod_16;
	bnGcd = bnGcd_16;
	bnInv = bnInv_16;
	bnLShift = bnLShift_16;
	bnRShift = bnRShift_16;
	bnMakeOdd = bnMakeOdd_16;
}

void
bnEnd_16(BigNum *bn)
{
	if (bn->ptr) {
		BNIFREE((BNWORD16 *)bn->ptr, bn->allocated);
		bn->ptr = 0;
	}
	bn->size = 0;
	bn->allocated = 0;

	MALLOCDB;
}

/* Internal function.  It operates in words. */
static int
bnResize_16(BigNum *bn, unsigned len)
{
	void *p;

	/* Round size up: most mallocs impose 8-byte granularity anyway */
	len = (len + (8/sizeof(BNWORD16) - 1)) & ~(8/sizeof(BNWORD16) - 1);
	p = BNIREALLOC((BNWORD16 *)bn->ptr, bn->allocated, len);
	if (!p)
		return -1;
	bn->ptr = p;
	bn->allocated = len;

	MALLOCDB;

	return 0;
}

#define bnSizeCheck(bn, size) \
	if (bn->allocated < size && bnResize_16(bn, size) < 0) \
		return -1

int
bnPrealloc_16(BigNum *bn, unsigned bits)
{
	bits = (bits + 16-1)/16;
	bnSizeCheck(bn, bits);
	MALLOCDB;
	return 0;
}

int
bnCopy_16(BigNum *dest, BigNum const *src)
{
	bnSizeCheck(dest, src->size);
	dest->size = src->size;
	bniCopy_16((BNWORD16 *)dest->ptr, (BNWORD16 *)src->ptr, src->size);
	MALLOCDB;
	return 0;
}

void
bnNorm_16(BigNum *bn)
{
	bn->size = bniNorm_16((BNWORD16 *)bn->ptr, bn->size);
}

/*
 * Convert a bignum to big-endian bytes.  Returns, in big-endian form, a
 * substring of the bignum starting from lsbyte and "len" bytes long.
 * Unused high-order (leading) bytes are filled with 0.
 */
void
bnExtractBigBytes_16(BigNum const *bn, unsigned char *dest,
                  unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size * (16 / 8);

	/* Fill unused leading bytes with 0 */
	while (s < lsbyte+len && len) {
		*dest++ = 0;
		len--;
	}

	if (len)
		bniExtractBigBytes_16((BNWORD16 *)bn->ptr, dest, lsbyte, len);
	MALLOCDB;
}

int
bnInsertBigBytes_16(BigNum *bn, unsigned char const *src,
                 unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size;
	unsigned words = (len+lsbyte+sizeof(BNWORD16)-1) / sizeof(BNWORD16);

	/* Pad with zeros as required */
	bnSizeCheck(bn, words);

	if (s < words) {
		bniZero_16((BNWORD16 *)bn->ptr BIGLITTLE(-s,+s), words-s);
		s = words;
	}

	bniInsertBigBytes_16((BNWORD16 *)bn->ptr, src, lsbyte, len);

	bn->size = bniNorm_16((BNWORD16 *)bn->ptr, s);

	MALLOCDB;
	return 0;
}


/*
 * Convert a bignum to little-endian bytes.  Returns, in little-endian form, a
 * substring of the bignum starting from lsbyte and "len" bytes long.
 * Unused high-order (trailing) bytes are filled with 0.
 */
void
bnExtractLittleBytes_16(BigNum const *bn, unsigned char *dest,
                  unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size * (16 / 8);

	/* Fill unused leading bytes with 0 */
	while (s < lsbyte+len && len)
		dest[--len] = 0;

	if (len)
		bniExtractLittleBytes_16((BNWORD16 *)bn->ptr, dest,
		                         lsbyte, len);
	MALLOCDB;
}

int
bnInsertLittleBytes_16(BigNum *bn, unsigned char const *src,
                       unsigned lsbyte, unsigned len)
{
	unsigned s = bn->size;
	unsigned words = (len+lsbyte+sizeof(BNWORD16)-1) / sizeof(BNWORD16);

	/* Pad with zeros as required */
	bnSizeCheck(bn, words);

	if (s < words) {
		bniZero_16((BNWORD16 *)bn->ptr BIGLITTLE(-s,+s), words-s);
		s = words;
	}

	bniInsertLittleBytes_16((BNWORD16 *)bn->ptr, src, lsbyte, len);

	bn->size = bniNorm_16((BNWORD16 *)bn->ptr, s);

	MALLOCDB;
	return 0;
}

/* Return the least-significant word of the input. */
unsigned
bnLSWord_16(BigNum const *src)
{
	return src->size ? (unsigned)((BNWORD16 *)src->ptr)[BIGLITTLE(-1,0)]
			 : 0;
}

unsigned
bnBits_16(BigNum const *src)
{
	return bniBits_16((BNWORD16 *)src->ptr, src->size);
}

int
bnAdd_16(BigNum *dest, BigNum const *src)
{
	unsigned s = src->size, d = dest->size;
	BNWORD16 t;

	if (!s)
		return 0;

	bnSizeCheck(dest, s);

	if (d < s) {
		bniZero_16((BNWORD16 *)dest->ptr BIGLITTLE(-d,+d), s-d);
		dest->size = d = s;
		MALLOCDB;
	}
	t = bniAddN_16((BNWORD16 *)dest->ptr, (BNWORD16 *)src->ptr, s);
	MALLOCDB;
	if (t) {
		if (d > s) {
			t = bniAdd1_16((BNWORD16 *)dest->ptr BIGLITTLE(-s,+s),
			               d-s, t);
			MALLOCDB;
		}
		if (t) {
			bnSizeCheck(dest, d+1);
			((BNWORD16 *)dest->ptr)[BIGLITTLE(-1-d,d)] = t;
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
bnSub_16(BigNum *dest, BigNum const *src)
{
	unsigned s = src->size, d = dest->size;
	BNWORD16 t;

	if (d < s  &&  d < (s = bniNorm_16((BNWORD16 *)src->ptr, s))) {
		bnSizeCheck(dest, s);
		bniZero_16((BNWORD16 *)dest->ptr BIGLITTLE(-d,+d), s-d);
		dest->size = d = s;
		MALLOCDB;
	}
	if (!s)
		return 0;
	t = bniSubN_16((BNWORD16 *)dest->ptr, (BNWORD16 *)src->ptr, s);
	MALLOCDB;
	if (t) {
		if (d > s) {
			t = bniSub1_16((BNWORD16 *)dest->ptr BIGLITTLE(-s,+s),
			               d-s, t);
			MALLOCDB;
		}
		if (t) {
			bniNeg_16((BNWORD16 *)dest->ptr, d);
			dest->size = bniNorm_16((BNWORD16 *)dest->ptr,
			                        dest->size);
			MALLOCDB;
			return 1;
		}
	}
	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, dest->size);
	return 0;
}

/*
 * Compare the BigNum to the given value, which must be < 65536.
 * Returns -1. 0 or 1 if a<b, a == b or a>b.
 * a <=> b --> bnCmpQ(a,b) <=> 0
 */
int
bnCmpQ_16(BigNum const *a, unsigned b)
{
	unsigned t;
	BNWORD16 v;

	t = bniNorm_16((BNWORD16 *)a->ptr, a->size);
	/* If a is more than one word long or zero, it's easy... */
	if (t != 1)
		return (t > 1) ? 1 : (b ? -1 : 0);
	v = (unsigned)((BNWORD16 *)a->ptr)[BIGLITTLE(-1,0)];
	return (v > b) ? 1 : ((v < b) ? -1 : 0);
}

int
bnSetQ_16(BigNum *dest, unsigned src)
{
	if (src) {
		bnSizeCheck(dest, 1);

		((BNWORD16 *)dest->ptr)[BIGLITTLE(-1,0)] = (BNWORD16)src;
		dest->size = 1;
	} else {
		dest->size = 0;
	}
	return 0;
}

int
bnAddQ_16(BigNum *dest, unsigned src)
{
	BNWORD16 t;

	if (!dest->size)
		return bnSetQ(dest, src);
	
	t = bniAdd1_16((BNWORD16 *)dest->ptr, dest->size, (BNWORD16)src);
	MALLOCDB;
	if (t) {
		src = dest->size;
		bnSizeCheck(dest, src+1);
		((BNWORD16 *)dest->ptr)[BIGLITTLE(-1-src,src)] = t;
		dest->size = src+1;
	}
	return 0;
}

/*
 * Return value as for bnSub: 1 if subtract underflowed, in which
 * case the return is the negative of the computed value.
 */
int
bnSubQ_16(BigNum *dest, unsigned src)
{
	BNWORD16 t;

	if (!dest->size)
		return bnSetQ(dest, src) < 0 ? -1 : (src != 0);

	t = bniSub1_16((BNWORD16 *)dest->ptr, dest->size, src);
	MALLOCDB;
	if (t) {
		/* Underflow. <= 1 word, so do it simply. */
		bniNeg_16((BNWORD16 *)dest->ptr, 1);
		dest->size = 1;
		return 1;
	}
/* Try to normalize?  Needing this is going to be pretty damn rare. */
/*		dest->size = bniNorm_16((BNWORD16 *)dest->ptr, dest->size); */
	return 0;
}

/*
 * Compare two BigNums.  Returns -1. 0 or 1 if a<b, a == b or a>b.
 * a <=> b --> bnCmp(a,b) <=> 0
 */
int
bnCmp_16(BigNum const *a, BigNum const *b)
{
	unsigned s, t;

	s = bniNorm_16((BNWORD16 *)a->ptr, a->size);
	t = bniNorm_16((BNWORD16 *)b->ptr, b->size);
	
	if (s != t)
		return s > t ? 1 : -1;
	return bniCmp_16((BNWORD16 *)a->ptr, (BNWORD16 *)b->ptr, s);
}

int
bnSquare_16(BigNum *dest, BigNum const *src)
{
	unsigned s;
	BNWORD16 *srcbuf;

	s = bniNorm_16((BNWORD16 *)src->ptr, src->size);
	if (!s) {
		dest->size = 0;
		return 0;
	}
	bnSizeCheck(dest, 2*s);

	if (src == dest) {
		BNIALLOC(srcbuf, BNWORD16, s);
		if (!srcbuf)
			return -1;
		bniCopy_16(srcbuf, (BNWORD16 *)src->ptr, s);
		bniSquare_16((BNWORD16 *)dest->ptr, (BNWORD16 *)srcbuf, s);
		BNIFREE(srcbuf, s);
	} else {
		bniSquare_16((BNWORD16 *)dest->ptr, (BNWORD16 *)src->ptr, s);
	}

	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, 2*s);
	MALLOCDB;
	return 0;
}

int
bnMul_16(BigNum *dest, BigNum const *a, BigNum const *b)
{
	unsigned s, t;
	BNWORD16 *srcbuf;

	s = bniNorm_16((BNWORD16 *)a->ptr, a->size);
	t = bniNorm_16((BNWORD16 *)b->ptr, b->size);

	if (!s || !t) {
		dest->size = 0;
		return 0;
	}

	if (a == b)
		return bnSquare_16(dest, a);

	bnSizeCheck(dest, s+t);

	if (dest == a) {
		BNIALLOC(srcbuf, BNWORD16, s);
		if (!srcbuf)
			return -1;
		bniCopy_16(srcbuf, (BNWORD16 *)a->ptr, s);
		bniMul_16((BNWORD16 *)dest->ptr, srcbuf, s,
		                                 (BNWORD16 *)b->ptr, t);
		BNIFREE(srcbuf, s);
	} else if (dest == b) {
		BNIALLOC(srcbuf, BNWORD16, t);
		if (!srcbuf)
			return -1;
		bniCopy_16(srcbuf, (BNWORD16 *)b->ptr, t);
		bniMul_16((BNWORD16 *)dest->ptr, (BNWORD16 *)a->ptr, s,
		                                 srcbuf, t);
		BNIFREE(srcbuf, t);
	} else {
		bniMul_16((BNWORD16 *)dest->ptr, (BNWORD16 *)a->ptr, s,
		                                 (BNWORD16 *)b->ptr, t);
	}
	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, s+t);
	MALLOCDB;
	return 0;
}

int
bnMulQ_16(BigNum *dest, BigNum const *a, unsigned b)
{
	unsigned s;

	s = bniNorm_16((BNWORD16 *)a->ptr, a->size);
	if (!s || !b) {
		dest->size = 0;
		return 0;
	}
	if (b == 1)
		return bnCopy_16(dest, a);
	bnSizeCheck(dest, s+1);
	bniMulN1_16((BNWORD16 *)dest->ptr, (BNWORD16 *)a->ptr, s, b);
	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, s+1);
	MALLOCDB;
	return 0;
}

int
bnDivMod_16(BigNum *q, BigNum *r, BigNum const *n,
            BigNum const *d)
{
	unsigned dsize, nsize;
	BNWORD16 qhigh;

	dsize = bniNorm_16((BNWORD16 *)d->ptr, d->size);
	nsize = bniNorm_16((BNWORD16 *)n->ptr, n->size);

	if (nsize < dsize) {
		q->size = 0;	/* No quotient */
		r->size = nsize;
		return 0;	/* Success */
	}

	bnSizeCheck(q, nsize-dsize);

	if (r != n) {	/* You are allowed to reduce in place */
		bnSizeCheck(r, nsize);
		bniCopy_16((BNWORD16 *)r->ptr, (BNWORD16 *)n->ptr, nsize);
	}
		
	qhigh = bniDiv_16((BNWORD16 *)q->ptr, (BNWORD16 *)r->ptr, nsize,
	                  (BNWORD16 *)d->ptr, dsize);
	nsize -= dsize;
	if (qhigh) {
		bnSizeCheck(q, nsize+1);
		*((BNWORD16 *)q->ptr BIGLITTLE(-nsize-1,+nsize)) = qhigh;
		q->size = nsize+1;
	} else {
		q->size = bniNorm_16((BNWORD16 *)q->ptr, nsize);
	}
	r->size = bniNorm_16((BNWORD16 *)r->ptr, dsize);
	MALLOCDB;
	return 0;
}

int
bnMod_16(BigNum *dest, BigNum const *src, BigNum const *d)
{
	unsigned dsize, nsize;

	nsize = bniNorm_16((BNWORD16 *)src->ptr, src->size);
	dsize = bniNorm_16((BNWORD16 *)d->ptr, d->size);


	if (dest != src) {
		bnSizeCheck(dest, nsize);
		bniCopy_16((BNWORD16 *)dest->ptr, (BNWORD16 *)src->ptr, nsize);
	}

	if (nsize < dsize) {
		dest->size = nsize;	/* No quotient */
		return 0;
	}

	(void)bniDiv_16((BNWORD16 *)dest->ptr BIGLITTLE(-dsize,+dsize),
	                (BNWORD16 *)dest->ptr, nsize,
	                (BNWORD16 *)d->ptr, dsize);
	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, dsize);
	MALLOCDB;
	return 0;
}

unsigned
bnModQ_16(BigNum const *src, unsigned d)
{
	unsigned s;

	s = bniNorm_16((BNWORD16 *)src->ptr, src->size);
	if (!s)
		return 0;
	
	if (d & (d-1))	/* Not a power of 2 */
		d = bniModQ_16((BNWORD16 *)src->ptr, s, d);
	else
		d = (unsigned)((BNWORD16 *)src->ptr)[BIGLITTLE(-1,0)] & (d-1);
	return d;
}

int
bnExpMod_16(BigNum *dest, BigNum const *n,
	BigNum const *exp, BigNum const *mod)
{
	unsigned nsize, esize, msize;

	nsize = bniNorm_16((BNWORD16 *)n->ptr, n->size);
	esize = bniNorm_16((BNWORD16 *)exp->ptr, exp->size);
	msize = bniNorm_16((BNWORD16 *)mod->ptr, mod->size);

	if (!msize || (((BNWORD16 *)mod->ptr)[BIGLITTLE(-1,0)] & 1) == 0)
		return -1;	/* Illegal modulus! */

	bnSizeCheck(dest, msize);

	/* Special-case base of 2 */
	if (nsize == 1 && ((BNWORD16 *)n->ptr)[BIGLITTLE(-1,0)] == 2) {
		if (bniTwoExpMod_16((BNWORD16 *)dest->ptr,
				    (BNWORD16 *)exp->ptr, esize,
				    (BNWORD16 *)mod->ptr, msize) < 0)
			return -1;
	} else {
		if (bniExpMod_16((BNWORD16 *)dest->ptr,
		                 (BNWORD16 *)n->ptr, nsize,
				 (BNWORD16 *)exp->ptr, esize,
				 (BNWORD16 *)mod->ptr, msize) < 0)
		return -1;
	}

	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, msize);
	MALLOCDB;
	return 0;
}

int
bnDoubleExpMod_16(BigNum *dest,
	BigNum const *n1, BigNum const *e1,
	BigNum const *n2, BigNum const *e2,
	BigNum const *mod)
{
	unsigned n1size, e1size, n2size, e2size, msize;

	n1size = bniNorm_16((BNWORD16 *)n1->ptr, n1->size);
	e1size = bniNorm_16((BNWORD16 *)e1->ptr, e1->size);
	n2size = bniNorm_16((BNWORD16 *)n2->ptr, n2->size);
	e2size = bniNorm_16((BNWORD16 *)e2->ptr, e2->size);
	msize = bniNorm_16((BNWORD16 *)mod->ptr, mod->size);

	if (!msize || (((BNWORD16 *)mod->ptr)[BIGLITTLE(-1,0)] & 1) == 0)
		return -1;	/* Illegal modulus! */

	bnSizeCheck(dest, msize);

	if (bniDoubleExpMod_16((BNWORD16 *)dest->ptr,
		(BNWORD16 *)n1->ptr, n1size, (BNWORD16 *)e1->ptr, e1size,
		(BNWORD16 *)n2->ptr, n2size, (BNWORD16 *)e2->ptr, e2size,
		(BNWORD16 *)mod->ptr, msize) < 0)
		return -1;

	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, msize);
	MALLOCDB;
	return 0;
}

int
bnTwoExpMod_16(BigNum *n, BigNum const *exp,
	BigNum const *mod)
{
	unsigned esize, msize;

	esize = bniNorm_16((BNWORD16 *)exp->ptr, exp->size);
	msize = bniNorm_16((BNWORD16 *)mod->ptr, mod->size);

	if (!msize || (((BNWORD16 *)mod->ptr)[BIGLITTLE(-1,0)] & 1) == 0)
		return -1;	/* Illegal modulus! */

	bnSizeCheck(n, msize);

	if (bniTwoExpMod_16((BNWORD16 *)n->ptr, (BNWORD16 *)exp->ptr, esize,
	                    (BNWORD16 *)mod->ptr, msize) < 0)
		return -1;

	n->size = bniNorm_16((BNWORD16 *)n->ptr, msize);
	MALLOCDB;
	return 0;
}

int
bnGcd_16(BigNum *dest, BigNum const *a, BigNum const *b)
{
	BNWORD16 *tmp;
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

	asize = bniNorm_16((BNWORD16 *)a->ptr, a->size);
	bsize = bniNorm_16((BNWORD16 *)b->ptr, b->size);

	bnSizeCheck(dest, bsize+1);

	/* Copy a to tmp */
	BNIALLOC(tmp, BNWORD16, asize+1);
	if (!tmp)
		return -1;
	bniCopy_16(tmp, (BNWORD16 *)a->ptr, asize);

	/* Copy b to dest, if necessary */
	if (dest != b)
		bniCopy_16((BNWORD16 *)dest->ptr,
			   (BNWORD16 *)b->ptr, bsize);
	if (bsize > asize || (bsize == asize &&
	        bniCmp_16((BNWORD16 *)b->ptr, (BNWORD16 *)a->ptr, asize) > 0))
	{
		i = bniGcd_16((BNWORD16 *)dest->ptr, bsize, tmp, asize,
			&dest->size);
		if (i > 0)	/* Result in tmp, not dest */
			bniCopy_16((BNWORD16 *)dest->ptr, tmp, dest->size);
	} else {
		i = bniGcd_16(tmp, asize, (BNWORD16 *)dest->ptr, bsize,
			&dest->size);
		if (i == 0)	/* Result in tmp, not dest */
			bniCopy_16((BNWORD16 *)dest->ptr, tmp, dest->size);
	}
	BNIFREE(tmp, asize+1);
	MALLOCDB;
	return (i < 0) ? i : 0;
}

int
bnInv_16(BigNum *dest, BigNum const *src,
         BigNum const *mod)
{
	unsigned s, m;
	int i;

	s = bniNorm_16((BNWORD16 *)src->ptr, src->size);
	m = bniNorm_16((BNWORD16 *)mod->ptr, mod->size);

	/* bniInv_16 requires that the input be less than the modulus */
	if (m < s ||
	    (m==s && bniCmp_16((BNWORD16 *)src->ptr, (BNWORD16 *)mod->ptr, s)))
	{
		bnSizeCheck(dest, s + (m==s));
		if (dest != src)
			bniCopy_16((BNWORD16 *)dest->ptr,
			           (BNWORD16 *)src->ptr, s);
		/* Pre-reduce modulo the modulus */
		(void)bniDiv_16((BNWORD16 *)dest->ptr BIGLITTLE(-m,+m),
			        (BNWORD16 *)dest->ptr, s,
		                (BNWORD16 *)mod->ptr, m);
		s = bniNorm_16((BNWORD16 *)dest->ptr, m);
		MALLOCDB;
	} else {
		bnSizeCheck(dest, m+1);
		if (dest != src)
			bniCopy_16((BNWORD16 *)dest->ptr,
			           (BNWORD16 *)src->ptr, s);
	}

	i = bniInv_16((BNWORD16 *)dest->ptr, s, (BNWORD16 *)mod->ptr, m);
	if (i == 0)
		dest->size = bniNorm_16((BNWORD16 *)dest->ptr, m);

	MALLOCDB;
	return i;
}

/*
 * Shift a bignum left the appropriate number of bits,
 * multiplying by 2^amt.
 */
int 
bnLShift_16(BigNum *dest, unsigned amt)
{
	unsigned s = dest->size;
	BNWORD16 carry;

	if (amt % 16) {
		carry = bniLshift_16((BNWORD16 *)dest->ptr, s, amt % 16);
		if (carry) {
			s++;
			bnSizeCheck(dest, s);
			((BNWORD16 *)dest->ptr)[BIGLITTLE(-s,s-1)] = carry;
		}
	}

	amt /= 16;
	if (amt) {
		bnSizeCheck(dest, s+amt);
		memmove((BNWORD16 *)dest->ptr BIGLITTLE(-s-amt, +amt),
		        (BNWORD16 *)dest->ptr BIG(-s),
			s * sizeof(BNWORD16));
		bniZero_16((BNWORD16 *)dest->ptr, amt);
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
void bnRShift_16(BigNum *dest, unsigned amt)
{
	unsigned s = dest->size;

	if (amt >= 16) {
		memmove(
		        (BNWORD16 *)dest->ptr BIG(-s+amt/16),
			(BNWORD16 *)dest->ptr BIGLITTLE(-s, +amt/16),
			s-amt/16 * sizeof(BNWORD16));
		s -= amt/16;
		amt %= 16;
	}

	if (amt)
		(void)bniRshift_16((BNWORD16 *)dest->ptr, s, amt);

	dest->size = bniNorm_16((BNWORD16 *)dest->ptr, s);
	MALLOCDB;
}

/*
 * Shift a bignum right until it is odd, and return the number of
 * bits shifted.  n = d * 2^s.  Replaces n with d and returns s.
 * Returns 0 when given 0.  (Another valid answer is infinity.)
 */
unsigned
bnMakeOdd_16(BigNum *n)
{
	unsigned size;
	unsigned s;	/* shift amount */
	BNWORD16 *p;
	BNWORD16 t;

	p = (BNWORD16 *)n->ptr;
	size = bniNorm_16(p, n->size);
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
		s *= 16;
		memmove((BNWORD16 *)n->ptr BIG(-size), p BIG(-size),
			size * sizeof(BNWORD16));
		p = (BNWORD16 *)n->ptr;
		MALLOCDB;
	}

	pgpAssert(t);

	/* Now count the bits */
	while ((t & 1) == 0) {
		t >>= 1;
		s++;
	}

	/* Shift the bits */
	if (s & (16-1)) {
		bniRshift_16(p, size, s & (16-1));
		/* Renormalize */
		if (BIGLITTLE(*(p-size),*(p+(size-1))) == 0)
			--size;
	}
	n->size = size;

	MALLOCDB;
	return s;
}
