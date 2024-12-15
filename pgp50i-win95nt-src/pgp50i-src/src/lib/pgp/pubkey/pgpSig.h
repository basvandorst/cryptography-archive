/*
* pgpSig.h
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpSig.h,v 1.3.2.1 1997/06/07 09:51:31 mhw Exp $
*/

#ifndef PGPSIG_H
#define PGPSIG_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpHash;
#ifndef TYPE_PGPHASH
#define TYPE_PGPHASH 1
typedef struct PgpHash PgpHash;
#endif

struct PgpPubKey;
#ifndef TYPE_PGPPUBKEY
#define TYPE_PGPPUBKEY 1
typedef struct PgpPubKey PgpPubKey;
#endif

struct PgpSig;
#ifndef TYPE_PGPSIG
#define TYPE_PGPSIG 1
typedef struct PgpSig PgpSig;
#endif

/* These are the public access functions to the Sig object */

int PGPExport pgpSigPKAlg (struct PgpSig const *sig);
struct PgpHash const PGPExport *pgpSigHash (struct PgpSig const *sig);
byte const PGPExport *pgpSigExtra (struct PgpSig const *sig, unsigned *len);
word32 PGPExport pgpSigTimestamp (struct PgpSig const *sig);
byte const PGPExport *pgpSigId8 (struct PgpSig const *sig);

/*
* Check a signature against a given public key and hash.
* Returns 0 if it did not check, and 1 if it did.
* Returns <0 on some sort of error.
* *Do* make sure it's the right hash you're passing in.
*/
int PGPExport pgpSigCheck (struct PgpSig const *sig,
	struct PgpPubKey const *pub, byte const *hash);

/*
* Below this line are the internal functions that are used only
* inside the PGP Library. Applciations probably shouldn't be
* using these functions.
*/

/*
* Internal variant of pgpSigCheck, not for public use. Signature *must*
* be known good.
*/
int
pgpSigCheckBuf (byte const *sig, size_t len, struct PgpPubKey const *pub,
		byte const *hash);

/* Check the buffer for a valid signature block and obtain the sigtype */
int PGPExport pgpSigSigType (byte const *buf, size_t len);

int PGPExport pgpSigNestFlag(byte const *buf, size_t len);

unsigned PGPExport pgpSigDistinctHashCount (struct PgpSig const *siglist);
unsigned PGPExport pgpSigDistinctHashes (struct PgpSig const *siglist,
	byte *buf);

int PGPExport pgpSigAdd (struct PgpSig **siglist, int type, byte const *buf,
	size_t len);
struct PgpSig PGPExport *pgpSigNext (struct PgpSig const *sig);
void PGPExport pgpSigFreeList (struct PgpSig *siglist);

#ifdef __cplusplus
}
#endif

#endif /* PGPSIG_H */
