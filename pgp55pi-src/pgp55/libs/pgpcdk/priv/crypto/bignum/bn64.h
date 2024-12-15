/*
 * bn64.h - interface to 64-bit bignum routines.
 *
 * $Id: bn64.h,v 1.1 1997/06/20 23:47:02 hal Exp $
 */

PGP_BEGIN_C_DECLARATIONS

#include "pgpBigNumOpaqueStructs.h"

void bnInit_64(void);
void bnEnd_64(BigNum *bn);
int bnPrealloc_64(BigNum *bn, unsigned bits);
int bnCopy_64(BigNum *dest, BigNum const *src);
int bnSwap_64(BigNum *a, BigNum *b);
void bnNorm_64(BigNum *bn);
void bnExtractBigBytes_64(BigNum const *bn, unsigned char *dest,
	unsigned lsbyte, unsigned dlen);
int bnInsertBigBytes_64(BigNum *bn, unsigned char const *src,
	unsigned lsbyte, unsigned len);
void bnExtractLittleBytes_64(BigNum const *bn, unsigned char *dest,
	unsigned lsbyte, unsigned dlen);
int bnInsertLittleBytes_64(BigNum *bn, unsigned char const *src,
	unsigned lsbyte, unsigned len);
unsigned bnLSWord_64(BigNum const *src);
unsigned bnBits_64(BigNum const *src);
int bnAdd_64(BigNum *dest, BigNum const *src);
int bnSub_64(BigNum *dest, BigNum const *src);
int bnCmpQ_64(BigNum const *a, unsigned b);
int bnSetQ_64(BigNum *dest, unsigned src);
int bnAddQ_64(BigNum *dest, unsigned src);
int bnSubQ_64(BigNum *dest, unsigned src);
int bnCmp_64(BigNum const *a, BigNum const *b);
int bnSquare_64(BigNum *dest, BigNum const *src);
int bnMul_64(BigNum *dest, BigNum const *a,
	BigNum const *b);
int bnMulQ_64(BigNum *dest, BigNum const *a, unsigned b);
int bnDivMod_64(BigNum *q, BigNum *r, BigNum const *n,
	BigNum const *d);
int bnMod_64(BigNum *dest, BigNum const *src,
	BigNum const *d);
unsigned bnModQ_64(BigNum const *src, unsigned d);
int bnExpMod_64(BigNum *dest, BigNum const *n,
	BigNum const *exp, BigNum const *mod);
int bnDoubleExpMod_64(BigNum *dest,
	BigNum const *n1, BigNum const *e1,
	BigNum const *n2, BigNum const *e2,
	BigNum const *mod);
int bnTwoExpMod_64(BigNum *n, BigNum const *exp,
	BigNum const *mod);
int bnGcd_64(BigNum *dest, BigNum const *a,
	BigNum const *b);
int bnInv_64(BigNum *dest, BigNum const *src,
	BigNum const *mod);
int bnLShift_64(BigNum *dest, unsigned amt);
void bnRShift_64(BigNum *dest, unsigned amt);
unsigned bnMakeOdd_64(BigNum *n);

PGP_END_C_DECLARATIONS
