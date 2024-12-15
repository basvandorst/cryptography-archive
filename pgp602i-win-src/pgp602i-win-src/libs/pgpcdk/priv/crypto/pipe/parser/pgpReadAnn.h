/*
 * pgpReadAnn.h -- A debugging Annotation Reader.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpReadAnn.h,v 1.8 1997/06/25 19:40:09 lloyd Exp $
 */

#ifndef Included_pgpReadAnn_h
#define Included_pgpReadAnn_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS


/*
 * Create an annotation reader.  This will create output files as
 * appropriate.  If filename is non-NULL, it will use that filename
 * as the output file.  Otherwise it will ask the user or use the name
 * out of the message.
 */
PGPPipeline  **
pgpAnnotationReaderCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
	PGPEnv const *env,
	PGPUICb const *ui, void *arg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpReadAnn_h */
