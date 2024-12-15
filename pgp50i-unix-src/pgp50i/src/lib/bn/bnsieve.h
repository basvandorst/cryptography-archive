/*
* bnsieve.h - Trial division for prime finding.
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is generally not intended for direct use by a user of the library;
* the bnprime.c and dhprime.c functions. are more likely to be used.
* However, a special application may need these.
*
* $Id: bnsieve.h,v 1.3.2.1 1997/06/07 09:49:42 mhw Exp $
*/

#ifdef __cplusplus
extern "C" {
#endif

struct BigNum;
#ifndef TYPE_BIGNUM
#define TYPE_BIGNUM 1
typedef struct BigNum BigNum;
#endif

/* Remove multiples of a single number from the sieve */
void BNExport sieveSingle(unsigned char *array, unsigned size, unsigned start,
	unsigned step);

/* Build a sieve starting at the number and incrementing by "step". */
int BNExport sieveBuild(unsigned char *array, unsigned size,
	struct BigNum const *bn, unsigned step, unsigned dbl);

/* Similar, but uses a >16-bit step size */
int BNExport sieveBuildBig(unsigned char *array, unsigned size,
	struct BigNum const *bn, struct BigNum const *step, unsigned dbl);

/* Return the next bit set in the sieve (or 0 on failure) */
unsigned BNExport sieveSearch(unsigned char const *array, unsigned size,
	unsigned start);

#ifdef __cplusplus
}
#endif
