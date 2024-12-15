/*
 * pgpTextFilt.h -- Text Filter Module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpTextFilt.h,v 1.10 1997/07/25 00:52:38 hal Exp $
 */

#ifndef Included_pgpTextFilt_h
#define Included_pgpTextFilt_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"
#include "pgpEncode.h"

/*
 * Create a text filtering module.  It will use the appropriate
 * character map to map input to output characters.  It will optionally
 * strip ending spaces off the end of lines, if stripspace is non-zero.
 * It will also convert line-endings to the appropriate type if
 * crlf is non-zero.  Use TEXTFILT_* to designate the line-ending
 * required.
 */
PGPPipeline  **
pgpTextFiltCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head, PGPByte const *map, int stripspace,
	PGPLineEndType crlf);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpTextFilt_h */