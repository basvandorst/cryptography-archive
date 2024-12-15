/*
 * pgpLiteral.h -- Create a Literal Packet Module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpLiteral.h,v 1.10 1997/06/25 19:40:17 lloyd Exp $
 */

#ifndef Included_pgpLiteral_h
#define Included_pgpLiteral_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS



/*
 * This creates a module for a literal packet.  The type should be
 * either 'b' or 't' for binary or text.  The name is len bytes which
 * specifies the name of the file.  The timestamp is the timestamp of
 * the file.
 */
PGPPipeline  **
pgpLiteralCreate (PGPContextRef cdkContext,
	PGPPipeline **head, PgpVersion version,
	PGPFifoDesc const *fd, PGPByte type, PGPByte *name,
	PGPByte len, PGPUInt32 timestamp);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpLiteral_h */
