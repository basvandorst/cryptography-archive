/*
 * pgpPrsAsc.h -- An Ascii Armor Parser for PGP.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpPrsAsc.h,v 1.9 1999/02/05 01:51:54 hal Exp $
 */

#ifndef Included_pgpPrsAsc_h
#define Included_pgpPrsAsc_h

#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

void pgpParseAscInit (void);
int pgpDearmorLine (PGPByte const *in, PGPByte *out, unsigned inlen);

PGPPipeline  **pgpParseAscCreate ( PGPContextRef cdkContext,
				PGPPipeline **head,
				PGPEnv const *env,
				PGPFifoDesc const *fd,
				PGPUICb const *ui,
				void *ui_arg);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpPrsAsc_h */
