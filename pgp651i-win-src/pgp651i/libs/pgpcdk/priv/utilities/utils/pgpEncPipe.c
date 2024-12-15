/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpEncPipe.c,v 1.50 1999/05/07 23:47:45 hal Exp $
____________________________________________________________________________*/

#include "pgpConfig.h"

#include <stdio.h>
#include <string.h>

#include "pgpEncPipe.h"
#include "pgpArmor.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpCiphrMod.h"
#include "pgpCFBPriv.h"
#include "pgpCompress.h"
#include "pgpCompMod.h"
#include "pgpConvKey.h"
#include "pgpConvMod.h"
#include "pgpDevNull.h"
#include "pgpEncode.h"
#include "pgpFIFO.h"
#include "pgpHash.h"
#include "pgpHeader.h"
#include "pgpJoin.h"
#include "pgpLiteral.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpEnv.h"
#include "pgpPKEMod.h"
#include "pgpPktByte.h"
#include "pgpPubKey.h"
#include "pgpRandomPoolPriv.h"
#include "pgpSigMod.h"
#include "pgpSigSpec.h"
#include "pgpSplit.h"
#include "pgpStr2Key.h"
#include "pgpTextFilt.h"
#include "pgpSymmetricCipherPriv.h"
#include "pgpContext.h"
#include "pgpRandomX9_17.h"

/* Forward */
static PGPByte * createHashList( PGPContextRef context,
	PGPSigSpec const *sigspecs, int *count);


PGPPipeline **
pgpEncryptPipelineCreate (
	PGPContextRef		cdkContext, 
	PGPPipeline **		head,
	PGPEnv const *		env,
	PGPFifoDesc const *	fd,
	PGPRandomContext const *rng,
	PGPConvKey *		convkeys,
	PGPPubKey *			pubkeys,
	PGPSigSpec *		sigspecs,
	PgpLiteralParams *	literal,
	int					sepsig,
	PGPByte *			sessionKey,
	PGPBoolean			knownBinaryInput,
	int					(*progress)(void *arg, int c),
	void *				arg,
	PGPSize *			sessionKeyLength,	/* output parameters */
	PGPError *			error)
{
	PGPPipeline **tail = head;
	PGPPipeline *sighead = NULL, **sigtail = NULL;
	PGPPipeline **texttail = NULL;
	PGPBoolean clearsig, compress, text, armor, pgpmime;
	PGPError	err = kPGPError_NoErr;
	PgpVersion version;
	PGPMemoryMgrRef	memoryMgr	= PGPGetContextMemoryMgr( cdkContext );

	version = pgpenvGetInt (env, PGPENV_VERSION, NULL, &err);
	compress = pgpenvGetInt (env, PGPENV_COMPRESS, NULL, &err);
	text = pgpenvGetInt (env, PGPENV_TEXTMODE, NULL, &err) && ! knownBinaryInput;
	armor = pgpenvGetInt (env, PGPENV_ARMOR, NULL, &err);

	/* Force 1-pass sig packets if encrypting to only non-RSA keys */
	/* Don't do it if just signing, may be going to PGP 2.6 user */
	if (version <= PGPVERSION_3) {
		PGPPubKey *pklist;
		pklist = pubkeys;
		if (pklist) {
			int oldkey = 0;
			while (pklist && !oldkey) {
				if ((PGPPublicKeyAlgorithm)pklist->pkAlg <=
					kPGPPublicKeyAlgorithm_RSA + 2)
				{
					oldkey = 1;
				}
				pklist = pklist->next;
			}
			if (!oldkey) {
				/*
				 * Upgrade version if all post-RSA.
				 * An earlier version changed sigspec here, but now setting
				 * version_3 in sigspec makes it use subpacket format, and
				 * that is not accepted in messages by earlier versions of
				 * the library.
				 */
				version = PGPVERSION_4;
			}
		}
	}

	/* Use version 4 packets if doing non-2.6 style conventional encryption */
	if (version <= PGPVERSION_3 && IsntNull( convkeys ) ) {
		PGPByte ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, NULL, NULL);
		if (ciphernum != kPGPCipherAlgorithm_IDEA ||
						IsntNull( convkeys->next ) || IsntNull( pubkeys ) ) {
			version = PGPVERSION_4;
		} else if (IsntNull( convkeys->stringToKey ) &&
				   !pgpS2KisOldVers (convkeys->stringToKey) ) {
			version = PGPVERSION_4;
		}
	}

	if (!head)
		goto err;

	/* Turn off literal and compression for sepsig creation */
	if (sepsig) {
		compress = 0;
		literal = NULL;
	}

	/* disallow separate signatures when we have these other types */
	if (sepsig &&
		(!sigspecs || compress || literal || convkeys || pubkeys)) {
		err = kPGPError_BadParams;
		goto err;
	}

	/* This is how we define when to do a clearsigned message */
	/* Now allow wrapping clearsigned messages in encryption */
	clearsig = (text && sigspecs && armor && 
				pgpenvGetInt (env, PGPENV_CLEARSIG, NULL, &err) && !sepsig);

	pgpmime = pgpenvGetInt (env, PGPENV_PGPMIME, NULL, &err);

	if (!fd)
		fd = &pgpFlexFifoDesc;

	/*
	 * If we can clearsign, then we do something special.  Do not add
	 * the literal packet type, do not compress, and create a split
	 * for the input data.
	 */
	if (clearsig) {
		literal = NULL;
		compress = 0;
		if (!pgpmime) {
				texttail = pgpSplitCreate ( cdkContext, tail);
				tail = pgpSplitAdd (*tail);
		}
	}

	if (text) {
		PGPByte const *charmap;

		if (!literal && !clearsig && !sepsig) {
			err = kPGPError_BadParams;
			goto err;
		}

		/*
		 * Use the appropriate charmap.  Stripspaces if we
		 * are clearsigning or if it was requested.
		 * Always add CRLF.
		 */
		charmap = (PGPByte const *)
			pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL);
		tail = pgpTextFiltCreate ( cdkContext, tail, charmap, clearsig ? 1 : 0,
					  kPGPLineEnd_CRLF);
	}
	if (sigspecs) {
		PGPPipeline **temptail = NULL;

		sigtail = tail;
		tail = pgpSigCreate( cdkContext, sigtail, version, fd, rng);

		sighead = *sigtail;

		/* XXX: should I do this? */
		temptail = pgpSigSignature (sighead, sigspecs, clearsig);
		if (temptail)
			sigtail = temptail;
	}

	if (literal) {
		PGPByte len = 0;

		if (literal->filename)
			len = strlen (literal->filename);
		
		tail = pgpLiteralCreate ( cdkContext, tail, version, fd,
					 (PGPByte)(text ? PGP_LITERAL_TEXT
									: PGP_LITERAL_BINARY),
					 (PGPByte *) literal->filename, len,
					 literal->timestamp);
	}

	if (sigspecs) {
		if (clearsig || sepsig) {
			if (!pgpDevNullCreate ( cdkContext, tail))
				tail = NULL;
			else {
				if (sepsig) {
					tail = sigtail;
				} else {
					/* create clearsig armor here */
					PGPByte *hashlist;
					int hashcount;
					hashlist = createHashList( cdkContext, sigspecs,
								  &hashcount);
					if (pgpmime) {
						tail = pgpArmorWriteCreatePgpMimesig
								(head, sigtail, env,
								 fd, rng, version, hashlist, hashcount);
					} else {
						tail = pgpArmorWriteCreateClearsig 
								(texttail, sigtail, env,
								 fd, version, hashlist,
								 hashcount);
					}
					pgpContextMemFree( cdkContext, hashlist);
				}
				
			}
		} else
			tail = pgpSigTextInsert (sighead, tail);
	}

	if (compress) {
		PGPByte alg = pgpenvGetInt (env, PGPENV_COMPRESSALG, NULL, NULL);
		int qual = pgpenvGetInt (env, PGPENV_COMPRESSQUAL, NULL, NULL);
		
		tail = pgpCompressModCreate( cdkContext, tail, version, fd, alg, qual);
	}

	if (convkeys || pubkeys) {
		PGPCipherVTBL const *cipher;
		PGPCFBContext *cfb;
		PGPStringToKey *s2k = NULL;
		PGPByte iv[MAXIVLEN];
		PGPByte ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, NULL, NULL);
		PGPByte passkey = 0;

		if (clearsig) {
			/* Insert a literal packet */
			tail = pgpLiteralCreate ( cdkContext, tail, version, fd,
									  PGP_LITERAL_TEXT,
									  NULL, 0, 0 );
		}

		/* Default for errors in this section */
		err = kPGPError_OutOfMemory;

		pgpAssert( IsntNull(sessionKey) );
		pgpAssert( IsntNull(sessionKeyLength) );

		if (!rng)
			rng = pgpContextGetX9_17RandomContext( cdkContext );

		cipher = pgpCipherGetVTBL ( (PGPCipherAlgorithm)ciphernum);
		if (!cipher)
			goto err;
		
		cfb = pgpCFBCreate ( memoryMgr, cipher);
		if (!cfb)
			goto err;
			
		if (convkeys) {
			if (version <= PGPVERSION_3)
				s2k = pgpS2KdefaultV3 (env, rng);
			else
				s2k = pgpS2Kdefault (env, rng);
			if (!s2k) {
				PGPFreeCFBContext (cfb);
				goto err;
			}
		}

		/* Generate the key (randomly or not) */
		sessionKey[0] = ciphernum;
		*sessionKeyLength = cipher->keysize + 1;
		if (convkeys && !convkeys->next && !pubkeys) {

			/* Use the pass phrase as the session key */
			err = pgpStringToKey ((convkeys->stringToKey ?
					       convkeys->stringToKey : s2k),
					      convkeys->pass,
					      convkeys->passlen, sessionKey+1,
					      cipher->keysize);
			if (IsPGPError(err)) {
				PGPFreeCFBContext (cfb);
				goto err;
			}
		} else {
			/* Generate a random session key */
			pgpRandomGetBytes (rng, sessionKey+1, cipher->keysize);
			passkey = 1;
		}

		/* Generate the random IV */
		pgpRandomGetBytes (rng, iv, sizeof (iv));

		/* Initialize the Cfb */
		/* No IV, message will start with random data */
		PGPInitCFB (cfb, sessionKey+1, NULL);
		
		/* Create the cipher module */
		tail = pgpCipherModEncryptCreate ( cdkContext, tail, version, fd,
										   cfb, iv, env );

		if (convkeys) {
			tail = pgpConvModCreate ( cdkContext, tail, version, fd, s2k,
						 convkeys,
						 ciphernum,
						 (passkey ? sessionKey : NULL));
			pgpS2Kdestroy (s2k);
		}
						  
		if (pubkeys) {
			PGPPipeline **pketail;
			PGPPubKey const *thiskey;
			PGPUInt32		keySize;
			int				encCount = 0;

			/* Save off the tail so we can talk to the PkeMod */
			pketail = tail;

			keySize	= pgpCFBGetKeySize( cfb );
			tail = pgpPkeCreate ( cdkContext,
				pketail, version, sessionKey, keySize + 1);

			if (!tail) {
				err = kPGPError_OutOfMemory;
				goto err;
			}

			for (thiskey = pubkeys; thiskey; thiskey = thiskey->next) {
				if (thiskey->encrypt) {
					err = pgpPkeAddKey (*pketail, thiskey, rng);
					if (IsntPGPError(err) && IsntNull(progress)) {
						 err = progress( arg, encCount++ );
					}
					if (IsPGPError (err) )
						goto err;
				}
			}
		}
	}

	/* Allow clearsig wrapped in encryption */
	if (!clearsig || convkeys || pubkeys) {
		tail = pgpHeaderCreate ( cdkContext, tail);
	}

	/* Allow clearsig wrapped in encryption */
	if (armor && (!clearsig || convkeys || pubkeys)) {
		PGPByte		armorType = PGP_ARMOR_SEPSIG;

		switch (sepsig) {
			case 0:
				armorType = PGP_ARMOR_NORMAL;
				break;
			/*
			 * XXX: This case is a hack until PGP/MIME is properly
			 * implemented in the library.  It indicates that the
			 * separate signature should say "BEGIN PGP MESSAGE"
			 * instead of "BEGIN PGP SIGNATURE", so that it can be
			 * used to form a PGP/MIME clear-signed message.
			 */
			case PGP_SEPSIGMSG:
				armorType = PGP_ARMOR_SEPSIGMSG;
				break;
			default:
				armorType = PGP_ARMOR_SEPSIG;
				break;
		}
		tail = pgpArmorWriteCreate ( cdkContext, tail, env, fd, rng, version,
									armorType);
	}

	if (!tail) {
		err = kPGPError_OutOfMemory;
		goto err;
	}

	/* Everything is OK */
	err = kPGPError_NoErr;
	if (error)
		*error = err;
	return tail;

err:
	/* Clean up on error */
	if (head && *head) {
		(*head)->teardown (*head);
		*head = NULL;
	}
	if (error)
		*error = err;
	return NULL;
}


/* Create a list of hash bytes from a sigspec list, and return the length */
static PGPByte *
createHashList(
	PGPContextRef	cdkContext,
	PGPSigSpec const *sigspecs, int *count)
{
	int cnt;
	PGPSigSpec const *ss1, *ss2;
	PGPByte *list;

	cnt = 0;
	/* Count unique hash types */
	for (ss1=sigspecs; ss1; ss1=pgpSigSpecNext(ss1)) {
		for (ss2=sigspecs; ss2!=ss1; ss2=pgpSigSpecNext(ss2)) {
			if (pgpSigSpecHashtype(ss2)==pgpSigSpecHashtype(ss1))
				break;
		}
		if (ss2==ss1)
			++cnt;
	}

	/* Allocate list */
	list = (PGPByte *)pgpContextMemAlloc( cdkContext,
				cnt, kPGPMemoryMgrFlags_Clear);
	if (!list) {
		*count = 0;
		return NULL;
	}

	/* Put unique hash types on list */
	cnt = 0;
	for (ss1=sigspecs; ss1; ss1=pgpSigSpecNext(ss1)) {
		for (ss2=sigspecs; ss2!=ss1; ss2=pgpSigSpecNext(ss2)) {
			if (pgpSigSpecHashtype(ss2)==pgpSigSpecHashtype(ss1))
				break;
		}
		if (ss2==ss1)
			list[cnt++] = pgpSigSpecHashtype(ss1);
	}
	*count = cnt;
	return list;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
