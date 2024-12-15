/*
 * pgpConvMod.c -- Convetional Encryption Module
 *
 * Uses the cipher module to encrypt a message; this will output the
 * appropriate PGP headers (ESK-type) packets associated with the
 * requested encryption type and pass phrase string.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpConvMod.c,v 1.33 1998/12/15 07:59:49 heller Exp $
 */

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpConvMod.h"
#include "pgpPktByte.h"
#include "pgpCFBPriv.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpConvKey.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpErrors.h"
#include "pgpPipeline.h"
#include "pgpStr2Key.h"
#include "pgpContext.h"

/*
 * A conventional ESK has the following format:
 *	<packet header> (type, llen, length)
 *	version[1]
 *	cipher[1]
 *	StringToKey Object[x]
 *	ESK (optional)[y]
 */
static int
addConvESKs (PGPPipeline *join, PgpVersion version, PGPByte cipher,
	     PGPStringToKey const *s2k,
	     PGPConvKey const *convkeys,
	     PGPByte const *session)
{
	PGPStringToKey const *sToK;
	PGPCipherVTBL const *	sc = NULL;
	PGPCipherVTBL const *		c =
			pgpCipherGetVTBL ( (PGPCipherAlgorithm)cipher);
	PGPCFBContext *cfb = NULL;
	size_t esklen, last = 0;
	PGPByte *buf = NULL, *ptr;
	PGPByte *key = NULL;
	int err = kPGPError_NoErr;
	PGPContextRef		cdkContext	= NULL;
	PGPMemoryMgrRef		memoryMgr	= NULL;

	pgpAssertAddrValid( join, PGPPipeline );
	cdkContext	= join->cdkContext;
	memoryMgr	= PGPGetContextMemoryMgr( cdkContext );

	pgpAssert( pgpContextIsValid( cdkContext ) );
	
	if (!c)
		return kPGPError_BadParams;

	if (session) {
		sc = pgpCipherGetVTBL ((PGPCipherAlgorithm)*session);
		if (!sc)
			return kPGPError_BadParams;
		cfb = pgpCFBCreate ( memoryMgr, c);
		if (!cfb)
			return kPGPError_OutOfMemory;
		key = (PGPByte *)pgpContextMemAlloc( cdkContext,
			c->keysize, kPGPMemoryMgrFlags_Clear );
		if (!key) {
			PGPFreeCFBContext (cfb);
			return kPGPError_OutOfMemory;
		}
	}

	for (; convkeys; convkeys = convkeys->next) {
		sToK = (convkeys->stringToKey ? convkeys->stringToKey : s2k);
		esklen = 1 + 1 + sToK->encodelen;
		if (sc)
			esklen += sc->keysize + 1;
		if (esklen + 3 > last) {
			if( IsNull( buf ) ) {
				buf = (PGPByte *)pgpContextMemAlloc( cdkContext,
							esklen + 3, 0 );
				if( IsNull( buf ) )
					err = kPGPError_OutOfMemory;
			} else {
				err = pgpContextMemRealloc ( cdkContext,
						(void **)&buf, esklen + 3, 0 );
			}
			if ( IsPGPError( err ) ) {
				pgpContextMemFree( cdkContext, buf );
				err = kPGPError_OutOfMemory;
				break;
			}
			last = esklen + 3;
		}
		if (esklen > 8192) {
			err = kPGPError_BadSessionKeySize;
			break;
		}
		ptr = buf+3;
		*ptr++ = (PGPByte) version;
		*ptr++ = cipher;
		memcpy (ptr, sToK->encoding, sToK->encodelen);
		ptr += sToK->encodelen;
		/* Add optional ESK */
		if (sc) {
			pgpStringToKey (sToK, convkeys->pass,
					convkeys->passlen, key, c->keysize);
			PGPInitCFB (cfb, key, NULL);
			pgpClearMemory( key,  c->keysize);
			pgpCFBEncryptInternal (cfb, session, sc->keysize+1, ptr );
			pgpCFBWipe (cfb);
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
			err = kPGPError_OutOfMemory;
			break;
		}
	}
	if (key) {
		pgpClearMemory( key,  c->keysize+1);
		pgpContextMemFree( cdkContext, key);
	}
	if (buf) {
		pgpClearMemory( buf,  last);
		pgpContextMemFree( cdkContext, buf);
	}
	if (cfb)
		PGPFreeCFBContext (cfb);

	return err;
}

PGPPipeline **
pgpConvModCreate (
	PGPContextRef cdkContext,
	PGPPipeline **head, PgpVersion version,
	PGPFifoDesc const *fd,
	PGPStringToKey const *s2k,
	PGPConvKey const *convkeys,
	PGPByte cipher, PGPByte const *session)
{
	PGPPipeline *join = NULL, **tail = &join;

	tail = pgpJoinCreate ( cdkContext, &join, fd);
	if (!tail)
		return NULL;

	/* Add the conventional ESKs here */
	if (version > PGPVERSION_3) {
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
