/*
 * pgpVrfySig.h -- Signature Verification Code
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpVrfySig.h,v 1.10 1997/07/31 22:51:48 lloyd Exp $
 */
#ifndef Included_pgpVrfySig_h
#define Included_pgpVrfySig_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

int pgpSigSetupPipeline (
		PGPContextRef cdkContext,
		PGPPipeline **head,
		PGPHashListRef *hashes,
		PGPByte const *hashlist, PGPUInt16 numHashes,
		PGPByte const *charmap);

int pgpSigSetupHashes (
		PGPContextRef cdkContext,
		PGPPipeline **tail, PGPEnv const *env,
		PGPHashListRef *hashlist,
		PGPSig const *siglist,
		PGPUICb const *ui, void *ui_arg);

PGPError	pgpSigVerify (PGPSig const *siglist, PGPHashListRef hashes,
				PGPUICb const *ui, void *arg);

int pgpSepsigVerify (PGPSig const *siglist, PGPEnv const *env,
		     PGPUICb const *ui, void *ui_arg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpVrfySig_h */
