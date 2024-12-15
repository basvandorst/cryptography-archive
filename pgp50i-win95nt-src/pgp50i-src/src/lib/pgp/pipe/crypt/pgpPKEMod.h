/*
* pgpPKEMod.h -- A Module to create Public Key Encrypted packets
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* This is a Public API Function Header.
*
* $Id: pgpPKEMod.h,v 1.3.2.1 1997/06/07 09:50:50 mhw Exp $
*/

#ifndef PGPPKEMOD_H
#define PGPPKEMOD_H

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PgpPipeline;
#ifndef TYPE_PGPPIPELINE
#define TYPE_PGPPIPELINE 1
typedef struct PgpPipeline PgpPipeline;
#endif

struct PgpRandomContext;
#ifndef TYPE_PGPRANDOMCONTEXT
#define TYPE_PGPRANDOMCONTEXT 1
typedef struct PgpRandomContext PgpRandomContext;
#endif

struct PgpPubKey;
#ifndef TYPE_PGPPUBKEY
#define TYPE_PGPPUBKEY 1
typedef struct PgpPubKey PgpPubKey;
#endif

/*
* Create a module to output PKE packets. The key is a block of data
* of size keylen which is the session key and cipher type which should
* be encrypted in the public keys of the recipients.
*/
struct PgpPipeline PGPExport **
pgpPkeCreate (struct PgpPipeline **head, PgpVersion version, byte const *key,
	size_t keylen);

/*
* Encrypt the session key in the passed-in public key and add that
* to the PKE list. This will return 0 on success or an error code.
*
* Note: Once any data is written to this module, no more public
* keys can be added. Calling this function after any data has been
* written will always return an error.
*/
int PGPExport pgpPkeAddKey (struct PgpPipeline *myself,
	struct PgpPubKey const *pubkey, struct PgpRandomContext const *rng);

#ifdef __cplusplus
}
#endif

#endif /* PGPPKEMOD_H */
