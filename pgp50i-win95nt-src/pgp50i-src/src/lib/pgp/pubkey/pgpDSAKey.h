/*
 * pgpDSAKey.h
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpDSAKey.h,v 1.4.2.1 1997/06/07 09:51:22 mhw Exp $
 */

#ifndef PGPDSAKEY_H
#define PGPDSAKEY_H

#include "pgpUsuals.h"	/* For byte */
#include <stddef.h>	/* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Bits for uniqueness of p, q */
#define DSADUMMYBITS	64

struct PgpPubKey;
struct PgpSecKey;
struct PgpRandomContext;

struct PgpPubKey *dsaPubFromBuf(byte const *buf, size_t len, int *error);
struct PgpSecKey *dsaSecFromBuf(byte const *buf, size_t len, int *error);
int dsaPubKeyPrefixSize(byte const *buf, size_t size);

struct PgpSecKey *
dsaSecGenerate(unsigned bits, Boolean fastgen,
	struct PgpRandomContext const *rc,
	int progress(void *arg, int c), void *arg, int *error);

#ifdef __cplusplus
}
#endif

#endif /* PGPDSAKEY_H */
