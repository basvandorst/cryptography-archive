/*
* bndsaprime.h
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bndsaprime.h,v 1.2.2.1 1997/06/07 09:49:28 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

struct BigNum;
#ifndef TYPE_BIGNUM
#define TYPE_BIGNUM 1
typedef struct BigNum BigNum;
#endif

/* Generate a pair of DSS primes */
int dsaPrimeGen(struct BigNum *bnq, struct BigNum *bnp,
 int (*f)(void *arg, int c), void *arg);

#ifdef __cplusplus
}
#endif
