/*
 * $Id: bni64.h,v 1.3 1998/05/14 19:07:24 cbertsch Exp $
 */

#ifndef Included_bni64_h
#define Included_bni64_h

#include "bni.h"
#include "pgpMemoryMgr.h"

#ifndef BNWORD64
#error 64-bit bignum library requires a 64-bit data type
#endif

PGP_BEGIN_C_DECLARATIONS

#ifndef bniCopy_64
void bniCopy_64(BNWORD64 *dest, BNWORD64 const *src, unsigned len);
#endif
#ifndef bniZero_64
void bniZero_64(BNWORD64 *num, unsigned len);
#endif
#ifndef bniNeg_64
void bniNeg_64(BNWORD64 *num, unsigned len);
#endif

#ifndef bniAdd1_64
BNWORD64 bniAdd1_64(BNWORD64 *num, unsigned len, BNWORD64 carry);
#endif
#ifndef bniSub1_64
BNWORD64 bniSub1_64(BNWORD64 *num, unsigned len, BNWORD64 borrow);
#endif

#ifndef bniAddN_64
BNWORD64 bniAddN_64(BNWORD64 *num1, BNWORD64 const *num2, unsigned len);
#endif
#ifndef bniSubN_64
BNWORD64 bniSubN_64(BNWORD64 *num1, BNWORD64 const *num2, unsigned len);
#endif

#ifndef bniCmp_64
int bniCmp_64(BNWORD64 const *num1, BNWORD64 const *num2, unsigned len);
#endif

#ifndef bniMulN1_64
void bniMulN1_64(BNWORD64 *out, BNWORD64 const *in, unsigned len, BNWORD64 k);
#endif
#ifndef bniMulAdd1_64
BNWORD64
bniMulAdd1_64(BNWORD64 *out, BNWORD64 const *in, unsigned len, BNWORD64 k);
#endif
#ifndef bniMulSub1_64
BNWORD64 bniMulSub1_64(BNWORD64 *out, BNWORD64 const *in, unsigned len,
			BNWORD64 k);
#endif

#ifndef bniLshift_64
BNWORD64 bniLshift_64(BNWORD64 *num, unsigned len, unsigned shift);
#endif
#ifndef bniDouble_64
BNWORD64 bniDouble_64(BNWORD64 *num, unsigned len);
#endif
#ifndef bniRshift_64
BNWORD64 bniRshift_64(BNWORD64 *num, unsigned len, unsigned shift);
#endif

#ifndef bniMul_64
void bniMul_64(BNWORD64 *prod, BNWORD64 const *num1, unsigned len1,
	BNWORD64 const *num2, unsigned len2);
#endif
#ifndef bniSquare_64
void bniSquare_64(BNWORD64 *prod, BNWORD64 const *num, unsigned len);
#endif

#ifndef bniNorm_64
unsigned bniNorm_64(BNWORD64 const *num, unsigned len);
#endif
#ifndef bniBits_64
unsigned bniBits_64(BNWORD64 const *num, unsigned len);
#endif

#ifndef bniExtractBigBytes_64
void bniExtractBigBytes_64(BNWORD64 const *bn, unsigned char *buf,
	unsigned lsbyte, unsigned buflen);
#endif
#ifndef bniInsertBigytes_64
void bniInsertBigBytes_64(BNWORD64 *n, unsigned char const *buf,
	unsigned lsbyte,  unsigned buflen);
#endif
#ifndef bniExtractLittleBytes_64
void bniExtractLittleBytes_64(BNWORD64 const *bn, unsigned char *buf,
	unsigned lsbyte, unsigned buflen);
#endif
#ifndef bniInsertLittleBytes_64
void bniInsertLittleBytes_64(BNWORD64 *n, unsigned char const *buf,
	unsigned lsbyte,  unsigned buflen);
#endif

#ifndef bniDiv21_64
BNWORD64 bniDiv21_64(BNWORD64 *q, BNWORD64 nh, BNWORD64 nl, BNWORD64 d);
#endif
#ifndef bniDiv1_64
BNWORD64 bniDiv1_64(BNWORD64 *q, BNWORD64 *rem,
	BNWORD64 const *n, unsigned len, BNWORD64 d);
#endif
#ifndef bniModQ_64
unsigned bniModQ_64(BNWORD64 const *n, unsigned len, unsigned d);
#endif
#ifndef bniDiv_64
BNWORD64
bniDiv_64(BNWORD64 *q, BNWORD64 *n, unsigned nlen, BNWORD64 *d, unsigned dlen);
#endif

#ifndef bniMontInv1_64
BNWORD64 bniMontInv1_64(BNWORD64 const x);
#endif
#ifndef bniMontReduce_64
void bniMontReduce_64(BNWORD64 *n, BNWORD64 const *mod, unsigned const mlen,
                BNWORD64 inv);
#endif
#ifndef bniToMont_64
void bniToMont_64(BNWORD64 *n, unsigned nlen, BNWORD64 *mod, unsigned mlen);
#endif
#ifndef bniFromMont_64
void bniFromMont_64(BNWORD64 *n, BNWORD64 *mod, unsigned len);
#endif

#ifndef bniExpMod_64
int bniExpMod_64( PGPMemoryMgrRef mgr, PGPBoolean secure,
	BNWORD64 *result, BNWORD64 const *n, unsigned nlen,
	BNWORD64 const *exp, unsigned elen, BNWORD64 *mod, unsigned mlen);
#endif
#ifndef bniDoubleExpMod_64
int bniDoubleExpMod_64(
	PGPMemoryMgrRef mgr, PGPBoolean secure,
	BNWORD64 *result,
	BNWORD64 const *n1, unsigned n1len, BNWORD64 const *e1, unsigned e1len,
	BNWORD64 const *n2, unsigned n2len, BNWORD64 const *e2, unsigned e2len,
	BNWORD64 *mod, unsigned mlen);
#endif
#ifndef bniTwoExpMod_64
int bniTwoExpMod_64( PGPMemoryMgrRef mgr, PGPBoolean secure,
	BNWORD64 *n, BNWORD64 const *exp, unsigned elen,
	BNWORD64 *mod, unsigned mlen);
#endif
#ifndef bniGcd_64
int bniGcd_64(BNWORD64 *a, unsigned alen, BNWORD64 *b, unsigned blen,
	unsigned *rlen);
#endif
#ifndef bniInv_64
int bniInv_64( PGPMemoryMgrRef mgr, PGPBoolean secure,
		BNWORD64 *a, unsigned alen, BNWORD64 const *mod, unsigned mlen);
#endif

PGP_END_C_DECLARATIONS

#endif /* Included_bni64_h */
