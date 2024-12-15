/*
* pgpCFB.h -- a Cipher FeedBack abstraction.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a Public API Function Header
*
* $Id: pgpCFB.h,v 1.3.2.1 1997/06/07 09:49:51 mhw Exp $
*/

#ifndef PGPCFB_H
#define PGPCFB_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpCipher;
#ifndef TYPE_PGPCIPHER
#define TYPE_PGPCIPHER 1
typedef struct PgpCipher PgpCipher;
#endif

struct PgpCipherContext;
#ifndef TYPE_PGPCIPHERCONTEXT
#define TYPE_PGPCIPHERCONTEXT 1
typedef struct PgpCipherContext PgpCipherContext;
#endif

#define PGP_CFB_MAXBLOCKSIZE 20
/*
* This structure does double duty for CFB encryption/decryption,
* and for X9.17 random number generation.
*
* When used for CFB, iv[] is used as a circular buffer. bufleft is
* the number of bytes at the end which have to be filled in before we
* crank the block cipher again. We do the block cipher operation
* lazily: bufleft may be 0. When we need one more byte, we
* crank the block cipher and set bufleft to 7.
*
* prev[] holds the previous 8 bytes of ciphertext, for use
* by ideaCfbSync() and Phil's, ahem, unique (not insecure, just
* unusual) way of doing CFB encryption.
*
* When used for X9.17 random number generation, iv[] holds the V initial
* vector, and prev[] holds the R random output. bufleft indicates how
* much of R is still available before the generator has to be cranked again.
*/
struct PgpCfbContext {
			struct PgpCipherContext *cipher;
			byte prev[PGP_CFB_MAXBLOCKSIZE];
			byte iv[PGP_CFB_MAXBLOCKSIZE];
			unsigned bufleft;
	};
#ifndef TYPE_PGPCFBCONTEXT
#define TYPE_PGPCFBCONTEXT 1
typedef struct PgpCfbContext PgpCfbContext;
#endif

/* Initialize a PgpCfbContext with the key and iv */
void PGPExport pgpCfbInit(struct PgpCfbContext *cfb, byte const *key,
	byte const *iv);

/* Clear a PgpCfbContext of its data */
void PGPExport pgpCfbWipe(struct PgpCfbContext *cfb);

/* Allocate a new PgpCfbContext structure */
struct PgpCfbContext PGPExport *pgpCfbCreate(struct PgpCipher const *c);

/* Copy an allocated PgpCfbContext structure */
struct PgpCfbContext PGPExport *pgpCfbCopy (struct PgpCfbContext const *cfb);

/* Free an allocated PgpCfbContext structure */
void PGPExport pgpCfbDestroy(struct PgpCfbContext *cfb);

void PGPExport pgpCfbEncrypt(struct PgpCfbContext *cfb, byte const *src,
	byte *dest, size_t len);
void PGPExport pgpCfbDecrypt(struct PgpCfbContext *cfb, byte const *src,
	byte *dest, size_t len);
void PGPExport pgpCfbSync(struct PgpCfbContext *cfb);

/* Returns as many bytes as are available before more salt is needed */
unsigned PGPExport pgpCfbRandBytes(struct PgpCfbContext *cfb, byte *dest,
	unsigned len);

/* Supply cfb->cipher->cipher->blocksize bytes of salt */
void PGPExport pgpCfbRandCycle(struct PgpCfbContext *cfb, byte const *salt);

/* Wash the state with the given buffer of entropy. */
void PGPExport pgpCfbRandWash(struct PgpCfbContext *cfb, byte const *buf,
	unsigned len);

#ifdef __cplusplus
}
#endif

#endif /* !PGPCFB_H */
