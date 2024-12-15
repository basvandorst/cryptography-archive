/*
 * pgpSigMod.h -- A module that is used to make a signed message.
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpSigMod.h,v 1.10 1997/06/25 19:40:14 lloyd Exp $
 */

#ifndef Included_pgpSigMod_h
#define Included_pgpSigMod_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

/*
 * This creates a signing module.  Initially, it just passes through its
 * input data unmodified.
 */
PGPPipeline  **pgpSigCreate ( PGPContextRef cdkContext,
				PGPPipeline **head,
				PgpVersion version,
				PGPFifoDesc const *fd,
				PGPRandomContext const *rc);
/*
 * This adds a signature to the signing module.  It returns a tail pointer
 * which the signatures will come out of.  Each call returns the same
 * tail pointer - this joins them together internally.  If clearsig is
 * true it won't try to output the special one-pass signature.
 */
PGPPipeline  **pgpSigSignature (PGPPipeline *pipe,
				      PGPSigSpec *spec, PGPByte clearsig);
/*
 * This inserts some text into the signature output in the appropriate place
 * to make a signed message, as opposed to separate signatures.
 * It is possible that you might take the output of the signature from
 * pgpSigCreate, apply some processing (such as compression, or encapsulation 
 * in a literal which PGP does right now) and feed it back into here.
 * (Should we hide *that* processing, too?)
 */
PGPPipeline  **pgpSigTextInsert (PGPPipeline *pipe,
				       PGPPipeline **head);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpSigMod_h */
