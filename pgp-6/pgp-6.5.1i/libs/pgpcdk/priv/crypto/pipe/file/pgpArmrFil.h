/*
 * pgpArmrFil.h -- Armor Write File module
 *
 * Creates output files based upon a "name".  If the data is within a
 * MULTIARMOR scope, multiple files will be opened for the output.
 * Otherwise, just a single file will be output when data is first
 * written into this module.
 *
 * Written By:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpArmrFil.h,v 1.8 1997/06/25 19:40:02 lloyd Exp $
 */

#ifndef Included_pgpArmrFil_h
#define Included_pgpArmrFil_h

PGP_BEGIN_C_DECLARATIONS

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGPPipeline  *
pgpArmorFileWriteCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head,
		 PGPFile * (*fileOpen) (void *arg,
				 void const *baseName, unsigned number),
	void *arg,
	void const *baseName);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpArmrFil_h */