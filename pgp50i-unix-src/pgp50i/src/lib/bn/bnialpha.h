/*
* bnialpha.h - header file that declares the Alpha assembly-language
* subroutines. It is intended to be included via the
* BNINCLUDE mechanism.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bnialpha.h,v 1.2.4.1 1997/06/07 09:49:35 mhw Exp $
*/

#define BN_LITTLE_ENDIAN 1

typedef unsigned long bnword64;
#define BNWORD64 bnword64

#ifdef __cplusplus
/* These assembly-language primitives use C names */
extern "C" {
#endif

void bniMulN1_64(bnword64 *out, bnword64 const *in, unsigned len, bnword64 k);
#define bniMulN1_64 bniMulN1_64

bnword64
bniMulAdd1_64(bnword64 *out, bnword64 const *in, unsigned len, bnword64 k);
#define bniMulAdd1_64 bniMulAdd1_64

bnword64
bniMulSub1_64(bnword64 *out, bnword64 const *in, unsigned len, bnword64 k);
#define bniMulSub1_64 bniMulSub1_64

#ifdef __cplusplus
}
#endif
