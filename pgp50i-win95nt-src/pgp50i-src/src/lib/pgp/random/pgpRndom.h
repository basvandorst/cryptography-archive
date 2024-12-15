/*
* pgpRndom.h -- a General Random Number Generator interface
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by:	Colin Plumb and Derek Atkins <warlord@MIT.EDU>
*
* This is a Public API Function Header.
*
* $Id: pgpRndom.h,v 1.9.2.1 1997/06/07 09:51:37 mhw Exp $
*/

#ifndef PGPRNDOM_H
#define PGPRNDOM_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpCipher;
#ifndef TYPE_PGPCIPHER
#define TYPE_PGPCIPHER 1
typedef struct PgpCipher PgpCipher;
#endif

struct PgpRandom {
			char const *name;
			void (*addBytes) (void *priv, byte const *buf, unsigned len);
			void (*getBytesEntropy) (void *priv, byte *buf, unsigned len,
			unsigned bits);
			void (*stir) (void *priv);
			void (*wipe) (void *priv);
	};
#ifndef TYPE_PGPRANDOM
#define TYPE_PGPRANDOM 1
typedef struct PgpRandom PgpRandom;
#endif

struct PgpRandomContext {
	struct PgpRandom const *random;
	void *priv;
	void (*destroy) (struct PgpRandomContext *rc);
};
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

#define pgpRandomAddBytes(rc,b,l) (rc)->random->addBytes((rc)->priv, b, l)
#define pgpRandomGetBytesEntropy(rc,b,l,t) \
	(rc)->random->getBytesEntropy((rc)->priv, b, l, t)
#define pgpRandomStir(rc) (rc)->random->stir((rc)->priv)
#define pgpRandomWipe(rc) (rc)->random->wipe((rc)->priv)
#define pgpRandomDestroy(rc) (rc)->destroy(rc)

struct PgpCipher;

void pgpRandomGetBytes(struct PgpRandomContext const *rc,
	byte *buf, unsigned len);

/*
* Another: The X9.17 generator, with a specified cipher and
* base random generator used to seed it.
*/
struct PgpRandomContext PGPExport *
pgpRandomCreateX9_17(struct PgpCipher const *c,
		struct PgpRandomContext const *base_rc);

/* The default: X9.17, with CAST5, based on the pgpRandomPool */
struct PgpRandomContext PGPExport *pgpRandomCreate (void);

/*
* A non-strong pseudo-random number generator. Use this only when
* you don't need cryptographic strength, but just want some unique
* values, and don't want to draw down the random pool (for example,
* generating the public primes in a DSA key). Based on the dummy
* pool, which always produces zeros, and a CAST5 cipher used per
* X9.17 to generate the pseudo random values.
*/
struct PgpRandomContext PGPExport *pgpPseudoRandomCreate (void);

/* Generate a random number in the specified range */
unsigned PGPExport pgpRandomRange(struct PgpRandomContext const *rc,
	unsigned range);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNDOM_H */
