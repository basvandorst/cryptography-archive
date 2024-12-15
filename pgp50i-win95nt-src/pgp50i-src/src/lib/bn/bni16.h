/*
* bni16.h
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bni16.h,v 1.4.2.1 1997/06/07 09:49:30 mhw Exp $
*/

#ifndef BNI16_H
#define BNI16_H

#include "bni.h"

#ifndef BNWORD16
#error 16-bit bignum library requires a 16-bit data type
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bniCopy_16
void bniCopy_16(BNWORD16 *dest, BNWORD16 const *src, unsigned len);
#endif
#ifndef bniZero_16
void bniZero_16(BNWORD16 *num, unsigned len);
#endif
#ifndef bniNeg_16
void bniNeg_16(BNWORD16 *num, unsigned len);
#endif

#ifndef bniAdd1_16
BNWORD16 bniAdd1_16(BNWORD16 *num, unsigned len, BNWORD16 carry);
#endif
#ifndef bniSub1_16
BNWORD16 bniSub1_16(BNWORD16 *num, unsigned len, BNWORD16 borrow);
#endif

#ifndef bniAddN_16
BNWORD16 bniAddN_16(BNWORD16 *num1, BNWORD16 const *num2, unsigned len);
#endif
#ifndef bniSubN_16
BNWORD16 bniSubN_16(BNWORD16 *num1, BNWORD16 const *num2, unsigned len);
#endif

#ifndef bniCmp_16
int bniCmp_16(BNWORD16 const *num1, BNWORD16 const *num2, unsigned len);
#endif

#ifndef bniMulN1_16
void bniMulN1_16(BNWORD16 *out, BNWORD16 const *in, unsigned len, BNWORD16 k);
#endif
#ifndef bniMulAdd1_16
BNWORD16
bniMulAdd1_16(BNWORD16 *out, BNWORD16 const *in, unsigned len, BNWORD16 k);
#endif
#ifndef bniMulSub1_16
BNWORD16 bniMulSub1_16(BNWORD16 *out, BNWORD16 const *in, unsigned len,
		BNWORD16 k);
#endif

#ifndef bniLshift_16
BNWORD16 bniLshift_16(BNWORD16 *num, unsigned len, unsigned shift);
#endif
#ifndef bniDouble_16
BNWORD16 bniDouble_16(BNWORD16 *num, unsigned len);
#endif
#ifndef bniRshift_16
BNWORD16 bniRshift_16(BNWORD16 *num, unsigned len, unsigned shift);
#endif

#ifndef bniMul_16
void bniMul_16(BNWORD16 *prod, BNWORD16 const *num1, unsigned len1,
	BNWORD16 const *num2, unsigned len2);
#endif
#ifndef bniSquare_16
void bniSquare_16(BNWORD16 *prod, BNWORD16 const *num, unsigned len);
#endif

#ifndef bniNorm_16
unsigned bniNorm_16(BNWORD16 const *num, unsigned len);
#endif
#ifndef bniBits_16
unsigned bniBits_16(BNWORD16 const *num, unsigned len);
#endif

#ifndef bniExtractBigBytes_16
void bniExtractBigBytes_16(BNWORD16 const *bn, unsigned char *buf,
	unsigned lsbyte, unsigned buflen);
#endif
#ifndef bniInsertBigytes_16
void bniInsertBigBytes_16(BNWORD16 *n, unsigned char const *buf,
	unsigned lsbyte, unsigned buflen);
#endif
#ifndef bniExtractLittleBytes_16
void bniExtractLittleBytes_16(BNWORD16 const *bn, unsigned char *buf,
	unsigned lsbyte, unsigned buflen);
#endif
#ifndef bniInsertLittleBytes_16
void bniInsertLittleBytes_16(BNWORD16 *n, unsigned char const *buf,
	unsigned lsbyte, unsigned buflen);
#endif

#ifndef bniDiv21_16
BNWORD16 bniDiv21_16(BNWORD16 *q, BNWORD16 nh, BNWORD16 nl, BNWORD16 d);
#endif
#ifndef bniDiv1_16
BNWORD16 bniDiv1_16(BNWORD16 *q, BNWORD16 *rem,
	BNWORD16 const *n, unsigned len, BNWORD16 d);
#endif
#ifndef bniModQ_16
unsigned bniModQ_16(BNWORD16 const *n, unsigned len, unsigned d);
#endif
#ifndef bniDiv_16
BNWORD16
bniDiv_16(BNWORD16 *q, BNWORD16 *n, unsigned nlen, BNWORD16 *d, unsigned dlen);
#endif

#ifndef bniMontInv1_16
BNWORD16 bniMontInv1_16(BNWORD16 const x);
#endif
#ifndef bniMontReduce_16
void bniMontReduce_16(BNWORD16 *n, BNWORD16 const *mod, unsigned const mlen,
BNWORD16 inv);
#endif
#ifndef bniToMont_16
void bniToMont_16(BNWORD16 *n, unsigned nlen, BNWORD16 *mod, unsigned mlen);
#endif
#ifndef bniFromMont_16
void bniFromMont_16(BNWORD16 *n, BNWORD16 *mod, unsigned len);
#endif

#ifndef bniExpMod_16
int bniExpMod_16(BNWORD16 *result, BNWORD16 const *n, unsigned nlen,
	BNWORD16 const *exp, unsigned elen, BNWORD16 *mod, unsigned mlen);
#endif
#ifndef bniDoubleExpMod_16
int bniDoubleExpMod_16(BNWORD16 *result,
	BNWORD16 const *n1, unsigned n1len, BNWORD16 const *e1, unsigned e1len,
	BNWORD16 const *n2, unsigned n2len, BNWORD16 const *e2, unsigned e2len,
	BNWORD16 *mod, unsigned mlen);
#endif
#ifndef bniTwoExpMod_16
int bniTwoExpMod_16(BNWORD16 *n, BNWORD16 const *exp, unsigned elen,
	BNWORD16 *mod, unsigned mlen);
#endif
#ifndef bniGcd_16
int bniGcd_16(BNWORD16 *a, unsigned alen, BNWORD16 *b, unsigned blen,
	unsigned *rlen);
#endif
#ifndef bniInv_16
int bniInv_16(BNWORD16 *a, unsigned alen, BNWORD16 const *mod, unsigned mlen);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BNI16_H */
