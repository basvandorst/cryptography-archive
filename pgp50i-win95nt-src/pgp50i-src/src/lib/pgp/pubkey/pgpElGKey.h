/*
 * pgpElGKey.h
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpElGKey.h,v 1.4.2.1 1997/06/07 09:51:25 mhw Exp $
 */

#ifndef PGPELGKEY_H
#define PGPELGKEY_H

#include "pgpUsuals.h"	/* For byte */
#include <stddef.h>	/* For size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Bits for uniqueness of p, q */
#define ELGDUMMYBITS	64

struct PgpPubKey;
struct PgpSecKey;
struct PgpRandomContext;

struct PgpPubKey *elgPubFromBuf(byte const *buf, size_t len, int *error);
struct PgpSecKey *elgSecFromBuf(byte const *buf, size_t len, int *error);
int elgPubKeyPrefixSize(byte const *buf, size_t size);

struct PgpSecKey *
elgSecGenerate(unsigned bits, Boolean fastgen,
	struct PgpRandomContext const *rc,
	int progress(void *arg, int c), void *arg, int *error);

#ifdef __cplusplus
}
#endif

#endif /* PGPELGKEY_H */
