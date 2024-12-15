/*
* bni8086.h - This file defines the interfaces to the 8086
* assembly primitives for 16-bit MS-DOS environments.
* It is intended to be included in "bni.h"
* via the "#include BNINCLUDE" mechanism.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bni8086.h,v 1.2.4.1 1997/06/07 09:49:35 mhw Exp $
*/

#define BN_LITTLE_ENDIAN 1

#ifdef __cplusplus
/* These assembly-language primitives use C names */
extern "C" {
#endif

/* Set up the appropriate types */
typedef unsigned short bnword16;
#define BNWORD16 bnword16
typedef unsigned long bnword32;
#define BNWORD32 bnword32

void __cdecl __far
bniMulN1_16(bnword16 __far *out, bnword16 const __far *in,
unsigned len, bnword16 k);
#define bniMulN1_16 bniMulN1_16

bnword16 __cdecl __far
bniMulAdd1_16(bnword16 __far *out, bnword16 const __far *in,
unsigned len, bnword16 k);
#define bniMulAdd1_16 bniMulAdd1_16

bnword16 __cdecl __far
bniMulSub1_16(bnword16 __far *out, bnword16 const __far *in,
unsigned len, bnword16 k);
#define bniMulSub1_16 bniMulSub1_16

bnword16 __cdecl __far
bniDiv21_16(bnword16 __far *q, bnword16 nh, bnword16 nl, bnword16 d);
#define bniDiv21_16 bniDiv21_16

bnword16 __cdecl __far
bniModQ_16(bnword16 const __far *n, unsigned len, bnword16 d);
#define bniModQ_16 bniModQ_16



void __cdecl __far
bniMulN1_32(bnword32 __far *out, bnword32 const __far *in,
unsigned len, bnword32 k);
#define bniMulN1_32 bniMulN1_32

bnword32 __cdecl __far
bniMulAdd1_32(bnword32 __far *out, bnword32 const __far *in,
		unsigned len, bnword32 k);
#define bniMulAdd1_32 bniMulAdd1_32

bnword32 __cdecl __far
bniMulSub1_32(bnword32 __far *out, bnword32 const __far *in,
unsigned len, bnword32 k);
#define bniMulSub1_32 bniMulSub1_32

bnword32 __cdecl __far
bniDiv21_32(bnword32 __far *q, bnword32 nh, bnword32 nl, bnword32 d);
#define bniDiv21_32 bniDiv21_32

bnword16 __cdecl __far
bniModQ_32(bnword32 const __far *n, unsigned len, bnword32 d);
#define bniModQ_32 bniModQ_32

int __cdecl __far not386(void);

#ifdef __cplusplus
}
#endif
