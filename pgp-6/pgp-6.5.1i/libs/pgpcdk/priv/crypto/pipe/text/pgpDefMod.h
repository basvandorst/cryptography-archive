/*
 * pgpDefMod.h -- Header for ZIP Compression (deflation)
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpDefMod.h,v 1.6 1997/06/25 19:40:15 lloyd Exp $
 */

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

PGPPipeline **defModCreate ( PGPContextRef cdkContext,
			PGPPipeline **head, int quality);

PGP_END_C_DECLARATIONS
