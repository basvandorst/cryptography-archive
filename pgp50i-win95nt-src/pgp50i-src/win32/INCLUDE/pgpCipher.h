/*
* pgpCipher.h - header file for generic block ciphers
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a Public API Function Header
*
* $Id: pgpCipher.h,v 1.5.2.1 1997/06/07 09:49:51 mhw Exp $
*/

#ifndef PGPCIPHER_H
#define PGPCIPHER_H

#include "pgpUsuals.h" /* for byte, word16 */

#ifdef __cplusplus
extern "C" {
#endif

/* A description of a cipher */
struct PgpCipher {
			char const *name;
			byte type;			/* PGP Cipher Type Byte */
			unsigned blocksize;
			unsigned keysize;
			unsigned context_size;
			unsigned context_align;
			void (*key)(void *priv, byte const *key);
			void (*encrypt)(void *priv, byte const *in, byte *out);
			void (*wash)(void *priv, byte const *buf, unsigned len);
	};
#ifndef TYPE_PGPCIPHER
#define TYPE_PGPCIPHER 1
typedef struct PgpCipher PgpCipher;
#endif

/*
* A cipher's private context is an extension on this -
* the size and alignment restrictions for the type are documented
* in the struct Cipher.
*/
struct PgpCipherContext {
	struct PgpCipher const *cipher;
	void *priv;
};
#ifndef TYPE_PGPCIPHERCONTEXT
#define TYPE_PGPCIPHERCONTEXT 1
typedef struct PgpCipherContext PgpCipherContext;
#endif


/* Macros to access the member functions */
#define pgpCipherKey(cc, k) ((cc)->cipher->key((cc)->priv, k))
#define pgpCipherEncrypt(cc, in, out) \
			((cc)->cipher->encrypt((cc)->priv, in, out))
#define pgpCipherWash(cc, buf, len) ((cc)->cipher->wash((cc)->priv, buf, len))

/* Wipe out a PgpCipherContext without freeing it */
void PGPExport pgpCipherWipe (struct PgpCipherContext *cc);

/* Lookup ciphers by name and number */
struct PgpCipher const PGPExport *pgpCipherByNumber (unsigned num);
struct PgpCipher const PGPExport *pgpCipherByName (char const *name,
	size_t namelen);

/* create and destroy Cipher Contexts from a cipher */
struct PgpCipherContext PGPExport *pgpCipherCreate (
	struct PgpCipher const *cc);
void PGPExport pgpCipherDestroy (struct PgpCipherContext *cc);

struct PgpCipherContext PGPExport *pgpCipherCopy (
	struct PgpCipherContext const *cc);

/* Cipher Numbers */
#define PGP_CIPHER_NULL 0
#define PGP_CIPHER_IDEA 1
#define PGP_CIPHER_3DES 2
#define PGP_CIPHER_CAST5 3

/* Get the default ciphers for operations */
struct PgpEnv;
struct PgpCipher const PGPExport *pgpCipherDefaultBulk(
	struct PgpEnv const *env);
struct PgpCipher const PGPExport *pgpCipherDefaultKey(
	struct PgpEnv const *env);
struct PgpCipher const PGPExport *pgpCipherDefaultKeyV2(
		struct PgpEnv const *env);

	/* Static buffer size for keys */
	#define PGP_CIPHER_MAXKEYSIZE	32		/* 256 bits */

#ifdef __cplusplus
}
#endif

#endif /* !PGPCIPHER_H */
