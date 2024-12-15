/*
 * pgpAddHdr.h -- header file for a module to add a packet header.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpAddHdr.h,v 1.7 1997/06/25 19:40:19 lloyd Exp $
 */

#include "pgpPubTypes.h"
#include "pgpOpaqueStructs.h"
#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

PGPPipeline **
pgpAddHeaderCreate ( PGPContextRef cdkContext,
			PGPPipeline **head, PgpVersion version,
		    PGPFifoDesc const *fd, PGPByte pkttype, PGPByte llen,
		    PGPByte *header, size_t hdrlen);

PGP_END_C_DECLARATIONS
