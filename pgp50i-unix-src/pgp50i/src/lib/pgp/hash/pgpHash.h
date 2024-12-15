/*
* pgpHash.h -- An abstraction to various hash types
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a Public API Function Header.
*
* $Id: pgpHash.h,v 1.3.2.2 1997/06/07 09:49:57 mhw Exp $
*/

#ifndef PGPHASH_H
#define PGPHASH_H

#include <stddef.h>
#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

/* A static description of a hash */
struct PgpHash {
		char const *name;
		byte type;
		byte const *DERprefix; /* DER-encoded prefix */
		unsigned DERprefixsize; /* DER-encoded prefix length */
		unsigned hashsize; /* Bytes of output */
		unsigned context_size;
		unsigned context_align;
		void (*init) (void *priv);
		void (*update) (void *priv, byte const *buf, size_t len);
		byte const * (*final) (void *priv);
	};
#ifndef TYPE_PGPHASH
#define TYPE_PGPHASH 1
typedef struct PgpHash PgpHash;
#endif

/* A way to keep the two together */
struct PgpHashContext {
 struct PgpHash const *hash;
 void *priv;
};
#ifndef TYPE_PGPHASHCONTEXT
#define TYPE_PGPHASHCONTEXT 1
typedef struct PgpHashContext PgpHashContext;
#endif

/* Macros to invoke on a struct HashContext */
#define pgpHashInit(hc) (hc)->hash->init((hc)->priv)
#define pgpHashUpdate(hc, buf, len) (hc)->hash->update((hc)->priv, buf, len)
#define pgpHashFinal(hc) (hc)->hash->final((hc)->priv)

/* Allocate, deallocate, and copy a hash context */
struct PgpHashContext PGPExport *pgpHashCreate (struct PgpHash const *h);
void PGPExport pgpHashDestroy (struct PgpHashContext *hc);
struct PgpHashContext PGPExport *pgpHashClone (
 struct PgpHashContext const *hc);
void PGPExport pgpHashCopy (struct PgpHashContext *dest,
  struct PgpHashContext const *src);

/* Return a hash struction of the appropriate number */
struct PgpHash const PGPExport *pgpHashByNumber (byte type);
struct PgpHash const PGPExport *pgpHashByName (char const *name,
 size_t namelen);

/* Create and Destroy a big bunch of hashes at once. */
int PGPExport pgpHashListCreate (byte const *buf,
 struct PgpHashContext **hashes, unsigned len);
void PGPExport pgpHashListDestroy (struct PgpHashContext *hashes,
 unsigned len);

/* Find the context of a given type in the list */
struct PgpHashContext PGPExport *pgpHashListFind (
 struct PgpHashContext *hashes, unsigned len, struct PgpHash const *h);

#ifdef __cplusplus
}
#endif

/* Hash prefixes and lengths for argument lists (preprocessor evil) */
#define PGP_HASH_MD5 1
#define PGP_HASH_SHA 2
#define PGP_HASH_RIPEMD160 3

#endif /* PGPHASH_H */
