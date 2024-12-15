/*
 * pgpPrsAsc.h -- An Ascii Armor Parser for PGP.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPrsAsc.h,v 1.8 1997/06/25 19:40:06 lloyd Exp $
 */

#ifndef Included_pgpPrsAsc_h
#define Included_pgpPrsAsc_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGPPipeline  **pgpParseAscCreate ( PGPContextRef cdkContext,
				PGPPipeline **head,
				PGPEnv const *env,
				PGPFifoDesc const *fd,
				PGPUICb const *ui,
				void *ui_arg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpPrsAsc_h */
