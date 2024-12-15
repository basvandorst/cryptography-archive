/*
 * $Id: pgpSig.h,v 1.14 1998/06/03 20:09:32 hal Exp $
 */

#ifndef Included_pgpSig_h
#define Included_pgpSig_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"

/* These are the public access functions to the Sig object */

PGPByte pgpSigType (PGPSig const *sig);
PGPByte pgpSigVersion (PGPSig const *sig);
PGPByte pgpSigPKAlg (PGPSig const *sig);
PGPHashVTBL const  *pgpSigHash (PGPSig const *sig);
PGPByte const  *pgpSigExtra (PGPSig const *sig, unsigned *len);
PGPUInt32  pgpSigTimestamp (PGPSig const *sig);

/* caller must free the key ID */
PGPError		pgpGetSigKeyID( PGPSig const *sig, PGPKeyID *outRef);

/*
 * Check a signature against a given public key and hash.
 * Returns 0 if it did not check, and 1 if it did.
 * Returns <0 on some sort of error.
 * *Do* make sure it's the right hash you're passing in.
 */
int  pgpSigCheck (PGPSig const *sig,
	PGPPubKey const *pub, PGPByte const *hash);

/*
 * Below this line are the internal functions that are used only
 * inside the PGP Library.  Applciations probably shouldn't be
 * using these functions.
 */

/*
 * Internal variant of pgpSigCheck, not for public use.  Signature *must*
 * be known good.
 */
int
pgpSigCheckBuf (PGPByte const *sig, PGPSize len, PGPPubKey const *pub,
		void const *hash);

/* Check the buffer for a valid signature block and obtain the sigtype */
int  pgpSigSigType (PGPByte const *buf, size_t len);

int  pgpSigNestFlag(PGPByte const *buf, size_t len);

unsigned  pgpSigDistinctHashCount (PGPSig const *siglist);
unsigned  pgpSigDistinctHashes (PGPSig const *siglist,
	PGPByte *buf);

int  pgpSigAdd ( PGPContextRef cdkContext,
	PGPSig **siglist, int type, PGPByte const *buf,
	size_t len);
PGPSig  *pgpSigNext (PGPSig const *sig);
void  pgpSigFreeList (PGPSig *siglist);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpSig_h */
