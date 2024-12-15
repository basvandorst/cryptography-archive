/*
 * pgpBufMod.h -- A Buffering Module; buffer until the first sizeAdvise
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpBufMod.h,v 1.9 1997/09/18 01:35:28 lloyd Exp $
 */

#ifndef Included_pgpBufMod_h
#define Included_pgpBufMod_h

#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS

PGPPipeline  **		pgpBufferModCreate ( PGPContextRef cdkContext,
						PGPPipeline **head, PGPFifoDesc const *fd);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpBufMod_h */