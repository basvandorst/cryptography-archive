/*
* pgpRSAKey.h
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpRSAKey.h,v 1.3.2.2 1997/06/09 23:46:36 quark Exp $
*/

#ifndef PGPRSAKEY_H
#define PGPRSAKEY_H

#include "pgpUsuals.h"	/* For byte */
#include <stddef.h>	 /* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPubKey;
struct PgpSecKey;
struct PgpRandomContext;

struct PgpPubKey *rsaPubFromBuf(byte const *buf, size_t len, int *error);
struct PgpSecKey *rsaSecFromBuf(byte const *buf, size_t len, int *error);
int rsaPubKeyPrefixSize(byte const *buf, size_t size);

struct PgpSecKey *
rsaSecGenerate(unsigned bits, Boolean fastgen,
	struct PgpRandomContext const *rc,
	int progress(void *arg, int c), void *arg, int *error);
int rsaChangeLock(struct PgpSecKey *seckey, struct PgpEnv const *env,
	struct PgpRandomContext const *rc, char const *phrase, size_t plen);

#ifdef __cplusplus
}
#endif

#endif /* PGPRSAKEY_H */
