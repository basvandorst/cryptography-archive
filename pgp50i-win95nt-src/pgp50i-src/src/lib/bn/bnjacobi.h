/*
* bnjacobi.h
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* For a small (usually prime, but not necessarily) prime p,
* Return Jacobi(p,bn), which is -1, 0 or +1.
* bn must be odd.
*
* $Id: bnjacobi.h,v 1.3.2.1 1997/06/07 09:49:38 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

struct BigNum;
#ifndef TYPE_BIGNUM
#define TYPE_BIGNUM 1
typedef struct BigNum BigNum;
#endif

int BNExport bnJacobiQ(unsigned p, struct BigNum const *bn);

#ifdef __cplusplus
}
#endif
