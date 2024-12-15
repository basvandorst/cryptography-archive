/*
 * pgpPKEMod.h -- A Module to create Public Key Encrypted packets
 *
 * This is a Public API Function Header.
 *
 * $Id: pgpPKEMod.h,v 1.10 1997/06/25 19:40:00 lloyd Exp $
 */

#ifndef Included_pgpPKEMod_h
#define Included_pgpPKEMod_h

#include "pgpOpaqueStructs.h"
#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS


/*
 * Create a module to output PKE packets.  The key is a block of data
 * of size keylen which is the session key and cipher type which should
 * be encrypted in the public keys of the recipients.
 */
PGPPipeline  **
pgpPkeCreate (PGPContextRef	cdkContext,
	PGPPipeline **head, PgpVersion version, PGPByte const *key,
	size_t keylen);

/*
 * Encrypt the session key in the passed-in public key and add that
 * to the PKE list.  This will return 0 on success or an error code.
 *
 * Note: Once any data is written to this module, no more public
 * keys can be added.  Calling this function after any data has been
 * written will always return an error.
 */
int  pgpPkeAddKey (PGPPipeline *myself,
	PGPPubKey const *pubkey, PGPRandomContext const *rng);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpPKEMod_h */
