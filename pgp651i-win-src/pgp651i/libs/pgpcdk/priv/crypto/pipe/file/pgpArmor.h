/*
 * pgpArmor.h -- Create an armor write module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpArmor.h,v 1.11 1997/07/25 00:52:32 hal Exp $
 */

#ifndef Included_pgpArmor_h
#define Included_pgpArmor_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"
#include "pgpUsuals.h"

PGP_BEGIN_C_DECLARATIONS

/*
 * This creates a module that will take its input and mask it in ASCII
 * Armor.  The armortype should be one of ARMOR_NORMAL, for a normal
 * armor, or ARMOR_SEPSIG, which is a separate signature.
 */
PGPPipeline  **
pgpArmorWriteCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head, PGPEnv const *env,
		     PGPFifoDesc const *fd,
		     PGPRandomContext const *rc, PgpVersion version,
		     PGPByte armortype);

/*
 * These are the valid armortypes that can be passed into pgpArmorWriteCreate
 */
#define PGP_ARMOR_NORMAL		0
#define PGP_ARMOR_SEPSIG		1

/*
 * XXX: PGP_ARMOR_SEPSIGMSG is identical to PGP_ARMOR_SEPSIG, except that
 * it outputs "BEGIN PGP MESSAGE" instead of "BEGIN PGP SIGNATURE" for
 * use in a PGP/MIME clear-signed message.  This is only temporary, and
 * should be deprecated when the library handles PGP/MIME internally.
 */
#define PGP_ARMOR_SEPSIGMSG		2

/*
 * Create an armor pipeline for clear signatures.  You should pass in the
 * tail pointers for the end of the text stream and the end of the signature
 * stream.  This will return a pointer to the stream that will contain the
 * clearsigned message.  Pass in a list of hash-type-bytes so it can say
 * what is being used.
 */
PGPPipeline  **
pgpArmorWriteCreateClearsig (PGPPipeline **texthead,
			     PGPPipeline **signhead,
			     PGPEnv const *env,
			     PGPFifoDesc const *fd,
			     PgpVersion version,
			     PGPByte *hashlist, unsigned hashlen);

/*
 * Create an armor pipeline for clear signatures, based on the PGPMIME
 * spec.  You should pass in the tail pointers for the end of the text
 * stream and the end of the signature stream.
 * This will return a pointer to the stream
 * that will contain the clearsigned message.  Pass in a list of
 * hash-type-bytes so it can say what is being used.
 */
PGPPipeline  **
pgpArmorWriteCreatePgpMimesig (PGPPipeline **texttail,
			     PGPPipeline **signtail,
			     PGPEnv const *env,
			     PGPFifoDesc const *fd,
				 PGPRandomContext const *rc,
			     PgpVersion version,
			     PGPByte *hashlist, unsigned hashlen);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpArmor_h */
