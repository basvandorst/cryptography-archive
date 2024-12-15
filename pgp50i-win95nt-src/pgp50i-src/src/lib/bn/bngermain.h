/*
* bngermain.h
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bngermain.h,v 1.3.2.1 1997/06/07 09:49:28 mhw Exp $
*/

#ifndef BNGERMAIN_H
#define BNGERMAIN_H

#ifdef __cplusplus
extern "C" {
#endif

struct BigNum;
#ifndef TYPE_BIGNUM
#define TYPE_BIGNUM 1
typedef struct BigNum BigNum;
#endif

/* Generate a Sophie Germain prime */
int BNExport bnGermainPrimeGen(struct BigNum *bn, unsigned dbl,
	int (*f)(void *arg, int c), void *arg);

#ifdef __cplusplus
}
#endif

#endif /* BNGERMAIN_H */
