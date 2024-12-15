/*
 * pgpCompMod.h -- this defines the compression and decompression
 * functions for PGP.. It provides a generalized interface to the
 * compression modules, and keeps applications from requiring
 * knowledge of all the different types of compression that may be
 * supported.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCompMod.h,v 1.10 1998/07/08 20:04:47 hal Exp $
 */

#ifndef Included_pgpCompMod_h
#define Included_pgpCompMod_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpPipelineOpaqueStructs.h"


/*
 * Create a compression module of the appropriate type (must be one of
 * the COMPRESSALG_* types) with the appropriate compression quality.
 *
 * Note:  There can only be one ZIP compression module in existance
 * at a time, since the underlying ZIP code is not re-entrant.
 */
PGPPipeline  **pgpCompressModCreate ( PGPContextRef cdkContext,
	PGPPipeline **head, PgpVersion version,
	PGPFifoDesc const *fd, PGPByte type, int quality);

/*
 * Create a decompression module of the appropriate type.
 */
PGPPipeline  **pgpDecompressModCreate ( PGPContextRef cdkContext,
	PGPPipeline **head, PGPByte type, PGPError *error);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpCompMod_h */
