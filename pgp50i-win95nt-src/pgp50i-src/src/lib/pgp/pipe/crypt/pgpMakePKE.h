/*
* pgpMakePKE.h -- Make a Public Key Encrypted packet from a pubkey and a
* session key.
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a PRIVATE header file, for use only within the PGP Library.
* You should not be using these functions in an application.
*
* $Id: pgpMakePKE.h,v 1.2.2.1 1997/06/07 09:50:49 mhw Exp $
*/

#ifndef PGPMAKEPKE_H
#define PGPMAKEPKE_H

#include <stddef.h>	 /* For size_t */

#include "pgpUsuals.h"	/* For byte and PgpVersion */

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPubKey;
struct PgpRandomContext;

/* The maximum size of the public-key-encrypted packet */
size_t makePkeMaxSize(struct PgpPubKey const *pub, PgpVersion version);

/*
* Given a buffer of at least "makePkeMaxSize" bytes, make a PKE packet
* into it and return the size of the PKE, or 0.
*/
int makePke(byte *buf, struct PgpPubKey const *pub,
	struct PgpRandomContext const *rc,
	byte const *key, unsigned keylen, PgpVersion version);

#ifdef __cplusplus
}
#endif

#endif /* PGPMAKEPKE_H */
