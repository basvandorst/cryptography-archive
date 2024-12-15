/*
 * pgpFixedKey.h
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * A single, fixed PGP key for decryption operations.
 *
 * $Id: pgpFixedKey.h,v 1.5.2.1 1997/06/07 09:51:26 mhw Exp $
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "pgpUsuals.h"

struct PgpPubKey;
#ifndef TYPE_PGPPUBKEY
#define TYPE_PGPPUBKEY 1
typedef struct PgpPubKey PgpPubKey;
#endif

struct PgpSecKey;
#ifndef TYPE_PGPSECKEY
#define TYPE_PGPSECKEY 1
typedef struct PgpSecKey PgpSecKey;
#endif

struct PgpPubKey PGPExport *fixedKeyPub(void);
struct PgpSecKey PGPExport *fixedKeySec(void);

extern unsigned char const PGPExport fixedKeyID[8];

/* Return > 0 if find fixed prime params of the specified size, 0 if not */
int
pgpDSAfixed (unsigned bits, byte const **p, size_t *plen,
	byte const **q, size_t *qlen);
int
pgpElGfixed (unsigned bits, byte const **p, size_t *plen,
	byte const **g, size_t *glen);


#ifdef __cplusplus
}
#endif
