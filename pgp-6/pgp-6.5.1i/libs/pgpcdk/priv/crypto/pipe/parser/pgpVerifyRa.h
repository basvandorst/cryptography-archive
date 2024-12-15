/*
 * pgpVerifyRa.h -- Signature Verification Annotation Reader
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpVerifyRa.h,v 1.9 1997/06/25 19:40:10 lloyd Exp $
 */
#ifndef Included_pgpVerifyRa_h
#define Included_pgpVerifyRa_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS


PGPPipeline  *
pgpVerifyReaderCreate (
	PGPContextRef cdkContext,
	PGPPipeline **texthead,
	PGPPipeline **sighead,
	PGPEnv const *env,
	PGPFifoDesc const *fd,
	PGPByte const *hashlist,
	unsigned hashlen,
	int textmode,
	PGPUICb const *ui,
	void *ui_arg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpVerifyRa_h */
