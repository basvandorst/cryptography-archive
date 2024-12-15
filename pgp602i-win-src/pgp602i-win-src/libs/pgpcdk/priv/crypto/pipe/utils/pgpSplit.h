/*
 * Split module - one input, multiple outputs.
 * All outputs get data, but ONLY THE ORIGINAL OUTPUT
 * GETS ANNOTATIONS.  (Error-handling is impossible otherwise.)
 *
 * $Id: pgpSplit.h,v 1.7 1997/06/25 19:40:26 lloyd Exp $
 */

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"


PGPPipeline  **pgpSplitCreate ( PGPContextRef cdkContext,
				PGPPipeline **head);
PGPPipeline  **pgpSplitAdd (PGPPipeline *myself);

PGP_END_C_DECLARATIONS