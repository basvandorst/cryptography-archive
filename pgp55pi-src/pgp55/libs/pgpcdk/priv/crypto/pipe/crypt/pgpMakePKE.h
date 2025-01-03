/*
 * pgpMakePKE.h -- Make a Public Key Encrypted packet from a pubkey and a
 * session key.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpMakePKE.h,v 1.7 1997/06/25 19:39:59 lloyd Exp $
 */

#ifndef Included_pgpMakePKE_h
#define Included_pgpMakePKE_h

#include <stddef.h>	/* For size_t */

#include "pgpUsuals.h"	/* For PGPByte and PgpVersion */

PGP_BEGIN_C_DECLARATIONS

/* The maximum size of the public-key-encrypted packet */
size_t makePkeMaxSize(PGPPubKey const *pub, PgpVersion version);

/*
 * Given a buffer of at least "makePkeMaxSize" bytes, make a PKE packet
 * into it and return the size of the PKE, or 0.
 */
int makePke(PGPByte *buf, PGPPubKey const *pub,
	    PGPRandomContext const *rc,
	    PGPByte const *key, unsigned keylen, PgpVersion version);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpMakePKE_h */
