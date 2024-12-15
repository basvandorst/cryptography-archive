/*
 * pgpPrsBin.h -- The PGP Binary Message parser
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPrsBin.h,v 1.8 1997/06/25 19:40:08 lloyd Exp $
 */

#ifndef Included_pgpPrsBin_h
#define Included_pgpPrsBin_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"
#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS


/*
 * Create a binary parser module.  This module will read in data,
 * figure out what kind of PGP message it is, and create the
 * appropriate sub-modules to actually decode the message, depending
 * on the user's wishes.
 *
 * It communicated with the Annotation Reader, which is a part of the
 * User Interface, to explain exactly what is going on with the
 * decoding process.  This module, once the pipeline is put into
 * action, will add and delete modules downstream.
 */
PGPPipeline  **pgpParseBinCreate (
		PGPContextRef cdkContext, PGPPipeline **head,
					PGPEnv const *env);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpPrsBin_h */
