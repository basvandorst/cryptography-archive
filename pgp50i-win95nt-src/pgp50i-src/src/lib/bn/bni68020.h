/*
* bni68020.h - 32-bit bignum primitives for the 68020 (or 683xx) processors.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* These primitives use little-endian word order.
* (The order of bytes within words is irrelevant.)
*
* $Id: bni68020.h,v 1.3.2.1 1997/06/07 09:49:32 mhw Exp $
*/
#define BN_LITTLE_ENDIAN 1

typedef unsigned long bnword32;
#define BNWORD32 bnword32

#ifdef __cplusplus
extern "C" {
#endif

bnword32 bniSub1_32(bnword32 *num, unsigned len, bnword32 borrow);
bnword32 bniAdd1_32(bnword32 *num, unsigned len, bnword32 carry);
void bniMulN1_32(bnword32 *out, bnword32 const *in, unsigned len, bnword32 k);
bnword32
bniMulAdd1_32(bnword32 *out, bnword32 const *in, unsigned len, bnword32 k);
bnword32
bniMulSub1_32(bnword32 *out, bnword32 const *in, unsigned len, bnword32 k);
bnword32 bniDiv21_32(bnword32 *q, bnword32 nh, bnword32 nl, bnword32 d);
unsigned bniModQ_32(bnword32 const *n, unsigned len, unsigned d);

#ifdef __cplusplus
}
#endif

/* #define the values to exclude the C versions */
#define bniSub1_32 bniSub1_32
#define bniAdd1_32 bniAdd1_32
#define bniMulN1_32 bniMulN1_32
#define bniMulAdd1_32 bniMulAdd1_32
#define bniMulSub1_32 bniMulSub1_32
#define bniDiv21_32 bniDiv21_32
#define bniModQ_32 bniModQ_32
