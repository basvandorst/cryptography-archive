/*
 * $Id: pgpMakePKE.c,v 1.11 1998/06/11 18:27:49 hal Exp $
 */

#include "pgpConfig.h"
#include <string.h>

#include "pgpDebug.h"
#include "pgpMakePKE.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpUsuals.h"

/* The maximum size of the PKE */
size_t
makePkeMaxSize (PGPPubKey const *pub, PgpVersion version)
{
	/* version + keyID + type +bit count + mpi */
	(void)version;
	return 10 + pgpPubKeyMaxesk(pub, kPGPPublicKeyMessageFormat_PGP);
}

/*
 * Given a buffer of at least "makePkeMaxSize" bytes, make a PKE
 * into it and return the size of the PKE, or <0.
 *
 * Format of PKE packets:
 *
 *      Offset	Length	Meaning
 *       0	1	Version byte (=2).
 *       1	8	KeyID
 *	 9	1	PK algorithm (1 = RSA)
 *      10	2+?	MPI of PK-encrypted integer
 */
int
makePke (PGPByte *buf, PGPPubKey const *pub,
	 PGPRandomContext const *rc,
	 PGPByte const *key, unsigned keylen, PgpVersion version)
{
	size_t esklen;
	int i;

	(void) version;

	/* XXX Should "die" gracefully, here */
	pgpAssert (pub->encrypt);

	i = pgpPubKeyEncrypt (pub, key, keylen, buf+10, &esklen, rc,
						  kPGPPublicKeyMessageFormat_PGP);
	if (i < 0)
		return i;
	
	/* Okay, build the PKE packet - fixed version number. */
	buf[0] = (PGPByte)PGPVERSION_3;
	/* KeyID */
	memcpy(buf+1, pub->keyID, 8);
	/* Type */
	buf[9] = pub->pkAlg;

	return (int)esklen+10;
}
