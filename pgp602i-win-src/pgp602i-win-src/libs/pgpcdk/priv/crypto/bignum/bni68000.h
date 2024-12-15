/*
 * bni68000.h - 16-bit bignum primitives for the 68000 (or 68010) processors.
 *
 * These primitives use little-endian word order.
 * (The order of bytes within words is irrelevant.)
 *
 * $Id: bni68000.h,v 1.2 1997/05/05 19:49:10 lloyd Exp $
 */
#define BN_LITTLE_ENDIAN 1

typedef unsigned short bnword16;
#define BNWORD16 bnword16

PGP_BEGIN_C_DECLARATIONS

bnword16 bniSub1_16(bnword16 *num, unsigned len, bnword16 borrow);
bnword16 bniAdd1_16(bnword16 *num, unsigned len, bnword16 carry);
void bniMulN1_16(bnword16 *out, bnword16 const *in, unsigned len, bnword16 k);
bnword16
bniMulAdd1_16(bnword16 *out, bnword16 const *in, unsigned len, bnword16 k);
bnword16
bniMulSub1_16(bnword16 *out, bnword16 const *in, unsigned len, bnword16 k);
bnword16 bniDiv21_16(bnword16 *q, bnword16 nh, bnword16 nl, bnword16 d);
unsigned bniModQ_16(bnword16 const *n, unsigned len, bnword16 d);

int is68020(void);

PGP_END_C_DECLARATIONS

/* #define the values to exclude the C versions */
#define bniSub1_16 bniSub1_16
#define bniAdd1_16 bniAdd1_16
#define bniMulN1_16 bniMulN1_16
#define bniMulAdd1_16 bniMulAdd1_16
#define bniMulSub1_16 bniMulSub1_16
#define bniDiv21_16 bniDiv21_16
#define bniModQ_16 bniModQ_16

/* Also include the 68020 definitions for 16/32 bit switching versions. */
#include "bni68020.h"
