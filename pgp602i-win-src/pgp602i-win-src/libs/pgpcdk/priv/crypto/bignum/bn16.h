/*
 * bn16.h - interface to 16-bit bignum routines.
 *
 * $Id: bn16.h,v 1.4 1997/05/13 01:18:33 mhw Exp $
 */

PGP_BEGIN_C_DECLARATIONS

#include "pgpBigNumOpaqueStructs.h"

void bnInit_16(void);
void bnEnd_16(BigNum *bn);
int bnPrealloc_16(BigNum *bn, unsigned bits);
int bnCopy_16(BigNum *dest, BigNum const *src);
int bnSwap_16(BigNum *a, BigNum *b);
void bnNorm_16(BigNum *bn);
void bnExtractBigBytes_16(BigNum const *bn, unsigned char *dest,
	unsigned lsbyte, unsigned dlen);
int bnInsertBigBytes_16(BigNum *bn, unsigned char const *src,
	unsigned lsbyte, unsigned len);
void bnExtractLittleBytes_16(BigNum const *bn, unsigned char *dest,
	unsigned lsbyte, unsigned dlen);
int bnInsertLittleBytes_16(BigNum *bn, unsigned char const *src,
	unsigned lsbyte, unsigned len);
unsigned bnLSWord_16(BigNum const *src);
unsigned bnBits_16(BigNum const *src);
int bnAdd_16(BigNum *dest, BigNum const *src);
int bnSub_16(BigNum *dest, BigNum const *src);
int bnCmpQ_16(BigNum const *a, unsigned b);
int bnSetQ_16(BigNum *dest, unsigned src);
int bnAddQ_16(BigNum *dest, unsigned src);
int bnSubQ_16(BigNum *dest, unsigned src);
int bnCmp_16(BigNum const *a, BigNum const *b);
int bnSquare_16(BigNum *dest, BigNum const *src);
int bnMul_16(BigNum *dest, BigNum const *a,
	BigNum const *b);
int bnMulQ_16(BigNum *dest, BigNum const *a, unsigned b);
int bnDivMod_16(BigNum *q, BigNum *r, BigNum const *n,
	BigNum const *d);
int bnMod_16(BigNum *dest, BigNum const *src,
	BigNum const *d);
unsigned bnModQ_16(BigNum const *src, unsigned d);
int bnExpMod_16(BigNum *dest, BigNum const *n,
	BigNum const *exp, BigNum const *mod);
int bnDoubleExpMod_16(BigNum *dest,
	BigNum const *n1, BigNum const *e1,
	BigNum const *n2, BigNum const *e2,
	BigNum const *mod);
int bnTwoExpMod_16(BigNum *n, BigNum const *exp,
	BigNum const *mod);
int bnGcd_16(BigNum *dest, BigNum const *a,
	BigNum const *b);
int bnInv_16(BigNum *dest, BigNum const *src,
	BigNum const *mod);
int bnLShift_16(BigNum *dest, unsigned amt);
void bnRShift_16(BigNum *dest, unsigned amt);
unsigned bnMakeOdd_16(BigNum *n);

PGP_END_C_DECLARATIONS