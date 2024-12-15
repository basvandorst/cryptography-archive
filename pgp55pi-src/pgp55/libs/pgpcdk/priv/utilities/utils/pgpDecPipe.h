/*
 * pgpDecPipe.h -- Create a Decryption Pipeline
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpDecPipe.h,v 1.9 1997/06/25 19:41:45 lloyd Exp $
 */

#ifndef Included_pgpDecPipe_h
#define Included_pgpDecPipe_h

#include "pgpBase.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGPPipeline  **
pgpDecryptPipelineCreate ( PGPContextRef cdkContext,
			PGPPipeline **head,
			PGPEnv *env,  PGPFifoDesc const *fd,
			PGPUICb const *ui, void *ui_arg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpDecPipe_h */
