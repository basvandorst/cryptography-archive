/*
* bnprint.h
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: bnprint.h,v 1.10.2.1 1997/06/07 09:49:41 mhw Exp $
*/

#ifndef BNPRINT_H
#define BNPRINT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
struct BigNum;
#ifndef TYPE_BIGNUM
#define TYPE_BIGNUM 1
typedef struct BigNum BigNum;
#endif

/* Print in base 16 */
int BNExport bnPrint(FILE *f, char const *prefix, struct BigNum const *bn,
	char const *suffix);

/* Print in base 10 */
int BNExport bnPrint10(FILE *f, char const *prefix, struct BigNum const *bn,
	char const *suffix);

#ifdef __cplusplus
}
#endif

#endif /* BNPRINT_H */
