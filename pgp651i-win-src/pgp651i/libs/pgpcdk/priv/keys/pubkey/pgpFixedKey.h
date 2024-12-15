/*
 * A single, fixed PGP key for decryption operations.
 *
 * $Id: pgpFixedKey.h,v 1.10 1997/09/30 21:03:01 lloyd Exp $
 */

#include "pgpPubTypes.h"
#include "pgpOpaqueStructs.h"

PGP_BEGIN_C_DECLARATIONS

/* Return > 0 if find fixed prime params of the specified size, 0 if not */
int
pgpDSAfixed (unsigned bits, PGPByte const **p, size_t *plen,
	PGPByte const **q, size_t *qlen);
int
pgpElGfixed (unsigned bits, PGPByte const **p, size_t *plen,
	PGPByte const **g, size_t *glen);


PGP_END_C_DECLARATIONS