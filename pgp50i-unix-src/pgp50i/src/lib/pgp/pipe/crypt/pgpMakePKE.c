/*
* pgpMakePKE.c
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpMakePKE.c,v 1.3.2.1 1997/06/07 09:50:49 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpDebug.h"
#include "pgpMakePKE.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpUsuals.h"

/* The maximum size of the PKE */
size_t
makePkeMaxSize (struct PgpPubKey const *pub, PgpVersion version)
{
	/* version + keyID + type +bit count + mpi */
	return 10 + pgpPubKeyMaxesk(pub, version);
}

/*
* Given a buffer of at least "makePkeMaxSize" bytes, make a PKE
* into it and return the size of the PKE, or <0.
*
* Format of PKE packets:
*
	*	Offset	Length	Meaning
	*	0		1		Version byte (=2).
	*	1		8		KeyID
	*		9		1		PK algorithm (1 = RSA)
	*	10		2+?		MPI of PK-encrypted integer
*/
int
makePke (byte *buf, struct PgpPubKey const *pub,
			struct PgpRandomContext const *rc,
			byte const *key, unsigned keylen, PgpVersion version)
	{
			size_t esklen;
			int i;

(void) version;

/* XXX Should "die" gracefully, here */
pgpAssert (pub->encrypt);

i = pgpPubKeyEncrypt (pub, key, keylen, buf+10, &esklen, rc, version);
if (i < 0)
	return i;
	
/* Okay, build the PKE packet - fixed version number. */
buf[0] = (byte)PGPVERSION_2_6;
/* KeyID */
			memcpy(buf+1, pub->keyID, 8);
			/* Type */
			buf[9] = pub->pkAlg;

			return (int)esklen+10;
	}
