/*
 * pgpSigPipe.h -- Create a Signature Verification Pipeline
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpSigPipe.h,v 1.9 1997/06/25 19:42:05 lloyd Exp $
 */

#ifndef Included_pgpSigPipe_h
#define Included_pgpSigPipe_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS



PGPPipeline  *
pgpSignatureVerifyCreate (
		PGPContextRef	cdkContext,
		PGPPipeline **texthead,
		PGPPipeline **sighead,
		PGPEnv const *env,
		PGPFifoDesc const *fd,
		PGPByte const *hashlist,
		unsigned hashlen, int textmode,
		PGPUICb const *ui,
		void *ui_arg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpSigPipe_h */