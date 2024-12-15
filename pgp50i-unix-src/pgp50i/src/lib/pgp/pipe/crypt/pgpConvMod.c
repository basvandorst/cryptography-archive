/*
* pgpConvMod.c -- Convetional Encryption Module
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Uses the cipher module to encrypt a message; this will output the
* appropriate PGP headers (ESK-type) packets associated with the
* requested encryption type and pass phrase string.
*
* Written by:	Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpConvMod.c,v 1.3.2.1 1997/06/07 09:50:48 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpConvMod.h"
#include "pgpPktByte.h"
#include "pgpCFB.h"
#include "pgpCipher.h"
#include "pgpConvKey.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpErr.h"
#include "pgpPipeline.h"
#include "pgpStr2Key.h"
#include "pgpUsuals.h"

/*
	* A conventional ESK has the following format:
		*		<packet header> (type, llen, length)
		*		version[1]
		*		cipher[1]
		*		StringToKey Object[x]
		*		ESK (optional)[y]
	*/
static int
addConvESKs (struct PgpPipeline *join, PgpVersion version, byte cipher,
				struct PgpStringToKey const *s2k,
				struct PgpConvKey const *convkeys,
				byte const *session)
	{
				struct PgpStringToKey const *sToK;
				struct PgpCipher const *sc = NULL, *c = pgpCipherByNumber (cipher);
				struct PgpCfbContext *cfb = NULL;
				size_t esklen, last = 0;
				byte *buf = NULL, *ptr;
				byte *key = NULL;
				int err = 0;

if (!c)
	return PGPERR_BADPARAM;

if (session) {
sc = pgpCipherByNumber (*session);
if (!sc)
	return PGPERR_BADPARAM;
cfb = pgpCfbCreate (c);
if (!cfb)
	return PGPERR_NOMEM;
key = (byte *)pgpMemAlloc (c->keysize);
if (!key) {
	pgpCfbDestroy (cfb);
	return PGPERR_NOMEM;
}
}

			for (; convkeys; convkeys = convkeys->next) {
					sToK = (convkeys->stringToKey ? convkeys->stringToKey : s2k);
					esklen = 1 + 1 + sToK->encodelen;
					if (sc)
						esklen += sc->keysize + 1;
					if (esklen + 3 > last) {
							buf = (byte *)pgpMemRealloc (buf, esklen + 3);
					if (!buf) {
								 err = PGPERR_NOMEM;
								 break;
							}
							last = esklen + 3;
					}
					if (esklen > 8192) {
						err = PGPERR_ESK_TOOLONG;
						break;
					}
					ptr = buf+3;
					*ptr++ = (byte) version;
					*ptr++ = cipher;
					memcpy (ptr, sToK->encoding, sToK->encodelen);
					ptr += sToK->encodelen;
					/* Add optional ESK */
					if (sc) {
							pgpStringToKey (sToK, convkeys->pass,
								 	convkeys->passlen, key, c->keysize);
							pgpCfbInit (cfb, key, NULL);
							memset (key, 0, c->keysize);
							pgpCfbEncrypt (cfb, session, ptr, sc->keysize+1);
							pgpCfbWipe (cfb);
							ptr += sc->keysize + 1;
					}

					/* Create the ConvESK Packet */
					if (PKTLEN_ONE_BYTE(esklen)) {
						buf[1] = PKTBYTE_BUILD_NEW (PKTBYTE_CONVESK);
						buf[2] = PKTLEN_1BYTE(esklen);
						ptr = buf+1;
						esklen += 2;
					} else {
							buf[0] = PKTBYTE_BUILD_NEW (PKTBYTE_CONVESK);
							buf[1] = PKTLEN_BYTE0(esklen);
							buf[2] = PKTLEN_BYTE1(esklen);
							ptr = buf;
							esklen += 3;
}

if (pgpJoinBuffer (join, ptr, esklen) != esklen) {
			err = PGPERR_NOMEM;
			break;
	}
	}
	if (key) {
	memset (key, 0, c->keysize+1);
	pgpMemFree (key);
}
if (buf) {
	memset (buf, 0, last);
	pgpMemFree (buf);
}
if (cfb)
	pgpCfbDestroy (cfb);

return err;
}

struct PgpPipeline **
pgpConvModCreate (struct PgpPipeline **head, PgpVersion version,
				struct PgpFifoDesc const *fd,
				struct PgpStringToKey const *s2k,
				struct PgpConvKey const *convkeys,
				byte cipher, byte const *session)
	{
			struct PgpPipeline *join = NULL, **tail = &join;

/* Only allow conventional ESKs with PGPVERSION_3 packets */
if (version <= PGPVERSION_2_6) {
			if (cipher != PGP_CIPHER_IDEA || convkeys->next || session)
				 return NULL;
				
			if (convkeys->stringToKey)
					if (!pgpS2KisOldVers (convkeys->stringToKey))
						return NULL;
			else
					if (!pgpS2KisOldVers (s2k))
						return NULL;
	}

tail = pgpJoinCreate (&join, fd);
if (!tail)
	return NULL;

/* Add the conventional ESKs here */
if (version > PGPVERSION_2_6) {
			if (addConvESKs (join, version, cipher, s2k, convkeys,
						session)) {
					join->teardown (join);
					return NULL;
			}
	}

/* Splice in the join module */
*tail = *head;
*head = join;
return tail;
}
