/*
* bnprime.h
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bnprime.h,v 1.3.2.1 1997/06/07 09:49:40 mhw Exp $
*/

#ifndef BNPRIME_H
#define BNPRIME_H

#ifdef __cplusplus
extern "C" {
#endif

struct BigNum;
#ifndef TYPE_BIGNUM
#define TYPE_BIGNUM 1
typedef struct BigNum BigNum;
#endif

/* Generate a prime >= bn. leaving the result in bn. */
int BNExport bnPrimeGen(struct BigNum *bn, unsigned (*randnum)(unsigned),
	int (*f)(void *arg, int c), void *arg, unsigned exponent, ...);

/*
* Generate a prime of the form bn + k*step. Step must be even and
* bn must be odd.
*/
int BNExport bnPrimeGenStrong(struct BigNum *bn, struct BigNum const *step,
		int (*f)(void *arg, int c), void *arg);

#ifdef __cplusplus
}
#endif

#endif /* BNPRIME_H */
