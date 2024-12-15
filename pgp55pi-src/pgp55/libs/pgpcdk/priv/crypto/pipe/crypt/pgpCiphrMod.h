/*
 * pgpCiphrMod.h -- secret-key cipher module
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpCiphrMod.h,v 1.13 1997/10/06 18:00:55 lloyd Exp $
 */

#ifndef Included_pgpCiphrMod_h
#define Included_pgpCiphrMod_h

#include "pgpUsuals.h"
#include "pgpEnv.h"

PGP_BEGIN_C_DECLARATIONS


/*
 * Create a Cfb Decryption Module using the passed-in PGPCFBContext.
 * This module assumes control of the PGPCFBContext and
 * will clear/free it upon teardown.
 */
PGPPipeline  **
pgpCipherModDecryptCreate ( PGPContextRef	cdkContext,
		PGPPipeline **head,
	 	PGPCFBContext *cfb, PGPEnv const *env);

/*
 * Create a Cfb Encryption Module using the passed-in PGPCFBContext.
 * The user must pass in an 8-byte iv, which will be encrypted into the
 * stream.  The Cipher Module assumes control of the PGPCFBContext 
 * and will clear/free it upon teardown.
 */
PGPPipeline  **
pgpCipherModEncryptCreate ( PGPContextRef	cdkContext,
				PGPPipeline **head, PgpVersion version,
			   PGPFifoDesc const *fd,
			   PGPCFBContext *cfb,
			   PGPByte const iv[IVLEN-2],
			   PGPEnv const *env);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpCiphrMod_h */
