/*
 * pgpEncPipe.c -- setup the Encryption Pipeline, given the arguments
 * of the UI. This will setup the type of pipeline that we want!
 *
 * Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by:	Derek Atkins <warlord@MIT.EDU>
 *
 * $Id: pgpEncPipe.c,v 1.12.2.1 1997/06/07 09:51:40 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpEncPipe.h"
#include "pgpArmor.h"
#include "pgpCipher.h"
#include "pgpCiphrMod.h"
#include "pgpCFB.h"
#include "pgpCompress.h"
#include "pgpCompMod.h"
#include "pgpConvKey.h"
#include "pgpConvMod.h"
#include "pgpDevNull.h"
#include "pgpFIFO.h"
#include "pgpHash.h"
#include "pgpHeader.h"
#include "pgpLiteral.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpEnv.h"
#include "pgpPKEMod.h"
#include "pgpPubKey.h"
#include "pgpRndPool.h"
#include "pgpSigMod.h"
#include "pgpSigSpec.h"
#include "pgpSplit.h"
#include "pgpStr2Key.h"
#include "pgpTextFilt.h"

/* Forward */
static byte * createHashList(struct PgpSigSpec const *sigspecs, int *count);


struct PgpPipeline **
pgpEncryptPipelineCreate (struct PgpPipeline **head,
			 		struct PgpEnv const *env,
			 		struct PgpFifoDesc const *fd,
			 		struct PgpRandomContext const *rng,
			 		struct PgpConvKey *convkeys,
			 		struct PgpPubKey *pubkeys,
			 		struct PgpSigSpec *sigspecs,
			 		struct PgpLiteralParams *literal,
			 		int sepsig)
	{
	struct PgpPipeline **tail = head;
	struct PgpPipeline *sighead = NULL, **sigtail = NULL;
	struct PgpPipeline **texttail = NULL;
	int clearsig, compress, text, armor, pgpmime;
	int err = 0, *error = &err;
	PgpVersion version;

	version = pgpenvGetInt (env, PGPENV_VERSION, NULL, error);
	compress = pgpenvGetInt (env, PGPENV_COMPRESS, NULL, error);
	text = pgpenvGetInt (env, PGPENV_TEXTMODE, NULL, error);
	armor = pgpenvGetInt (env, PGPENV_ARMOR, NULL, error);

	/* Force 1-pass sig packets if encrypting to only non-RSA keys */
	/* Don't do it if just signing, may be going to PGP 2.6 user */
	if (version <= PGPVERSION_2_6) {
		struct PgpPubKey *pklist;
		struct PgpSigSpec *sslist;
		pklist = pubkeys;
		if (pklist) {
			int oldkey = 0;
			while (pklist && !oldkey) {
				if (pklist->pkAlg <= PGP_PKALG_RSA + 2) {
					oldkey = 1;
				}
				pklist = pklist->next;
			}
			if (!oldkey) {
				/* Upgrade version if all post-RSA; also fix sigspecs */
				version = PGPVERSION_3;
				sslist = sigspecs;
				while (sslist) {
					if (pgpSigSpecVersion (sslist) <= PGPVERSION_2_6)
						pgpSigSpecSetVersion (sslist, PGPVERSION_3);
					sslist = pgpSigSpecNext (sslist);
				}
			}
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
			 (!sigspecs || compress || literal || convkeys || pubkeys))
			 goto err;

		/*
		* Don't allow the specification of convkeys and pubkeys at the
		* same time, yet.
		*/
		if (convkeys && pubkeys)
		goto err;

	if (convkeys && convkeys->next)
		goto err;

	/* This is how we define when to do a clearsigned message */
	clearsig = (text && sigspecs && armor &&
		pgpenvGetInt (env, PGPENV_CLEARSIG, NULL, error) &&
		!convkeys && !pubkeys && !sepsig);

	pgpmime = pgpenvGetInt (env, PGPENV_PGPMIME, NULL, error);

	if (!fd)
		fd = &pgpFlexFifoDesc;

	/*
	* If we can clearsign, then we do something special. Do not add
	* the literal packet type, do not compress, and create a split
	* for the input data.
	*/
	if (clearsig) {
		literal = NULL;
		compress = 0;
		if (!pgpmime) {
				texttail = pgpSplitCreate (tail);
				tail = pgpSplitAdd (*tail);
		}
	}

	if (text) {
		byte const *charmap;

		if (!literal && !clearsig && !sepsig)
			goto err;

		/*
		* Use the appropriate charmap. Stripspaces if we
		* are clearsignging or if it was requested.
		* Always add CRLF.
		*/
		charmap = (byte const *)
			pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL, NULL);
		tail = pgpTextFiltCreate (tail, charmap, clearsig ? 1 : 0,
					PGP_TEXTFILT_CRLF);
	}
	if (sigspecs) {
		struct PgpPipeline **temptail = NULL;

		sigtail = tail;
		tail = pgpSigCreate(sigtail, version, fd, rng);

		sighead = *sigtail;

		/* XXX: should I do this? */
		temptail = pgpSigSignature (sighead, sigspecs, clearsig);
		if (temptail)
			sigtail = temptail;
	}

	if (literal) {
		byte len = 0;

		if (literal->filename)
			len = strlen (literal->filename);

		tail = pgpLiteralCreate (tail, version, fd,
			 		 		(text ? PGP_LITERAL_TEXT :
			 		 		PGP_LITERAL_BINARY),
			 		 		(byte *) literal->filename, len,
			 		 		literal->timestamp);
		}

	if (sigspecs) {
		if (clearsig || sepsig) {
			if (!pgpDevNullCreate (tail))
				tail = NULL;
			else {
				if (sepsig) {
					tail = sigtail;
				} else {
					/* create clearsig armor here */
					byte *hashlist;
					int hashcount;
					hashlist = createHashList(sigspecs,
								&hashcount);
					if (pgpmime) {
						tail = pgpArmorWriteCreatePgpMimesig
								(head, sigtail, env,
								fd, version, hashlist, hashcount);
					} else {
						tail = pgpArmorWriteCreateClearsig
								(texttail, sigtail, env,
								fd, version, hashlist,
								hashcount);
					}
					pgpMemFree(hashlist);
				}
				
			}
		} else
			tail = pgpSigTextInsert (sighead, tail);
	}

	if (compress) {
		int alg = pgpenvGetInt (env, PGPENV_COMPRESSALG, NULL, NULL);
		int qual = pgpenvGetInt (env, PGPENV_COMPRESSQUAL, NULL, NULL);
		
		tail = pgpCompressModCreate (tail, version, fd, alg, qual);
	}

	if (convkeys || pubkeys) {
		struct PgpCipher const *cipher;
		struct PgpCfbContext *cfb;
		struct PgpStringToKey *s2k = NULL;
		byte iv[IVLEN-2];
		byte key[25];		 /* Max keysize */
		byte ciphernum = pgpenvGetInt (env, PGPENV_CIPHER, NULL, NULL);
		byte passkey = 0;

		if (!rng)
			rng = &pgpRandomPool;

		cipher = pgpCipherByNumber (ciphernum);
		if (!cipher)
			goto err;

		cfb = pgpCfbCreate (cipher);
		if (!cfb)
			goto err;

		if (convkeys) {
			s2k = pgpS2KdefaultV2 (env, rng);
			if (!s2k) {
				pgpCfbDestroy (cfb);
				goto err;
			}
		}

		/* Generate the key (randomly or not) */
		key[0] = ciphernum;
		if (convkeys && !convkeys->next && !pubkeys) {
			int ret;

			/* Use the pass phrase as the session key */
			ret = pgpStringToKey ((convkeys->stringToKey ?
			 		 		convkeys->stringToKey : s2k),
			 		 		convkeys->pass,
			 		 		convkeys->passlen, key+1,
			 		 		cipher->keysize);
			 	if (ret) {
				memset (key, 0, sizeof (key));
				pgpCfbDestroy (cfb);
				goto err;
			}
		} else {
			/* Generate a random session key */
			pgpRandomGetBytes (rng, key+1, cipher->keysize);
			passkey = 1;
		}

		/* Initialize the Cfb */
		pgpCfbInit (cfb, key+1, NULL);

		/* Generate the random IV and create the cipher module */
		pgpRandomGetBytes (rng, iv, sizeof (iv));
		tail = pgpCipherModEncryptCreate (tail, version, fd,
						cfb, iv, env);

		if (convkeys) {
			tail = pgpConvModCreate (tail, version, fd, s2k,
						convkeys,
						ciphernum,
						(passkey ? key : NULL));
			pgpS2Kdestroy (s2k);
		}
						
		if (pubkeys) {
			struct PgpPipeline **pketail;
			struct PgpPubKey const *thiskey;

			/* Save off the tail so we can talk to the PkeMod */
			pketail = tail;

			tail = pgpPkeCreate (pketail, version, key,
					cfb->cipher->cipher->keysize+1);

			if (!tail) {
				memset (key, 0, sizeof (key));
				goto err;
			}

			for (thiskey = pubkeys; thiskey;
			thiskey = thiskey->next)
				if (thiskey->encrypt)
					pgpPkeAddKey (*pketail, thiskey, rng);
		}
		memset (key, 0, sizeof (key));
	}

	if (!clearsig) {
		tail = pgpHeaderCreate (tail);
	}

	if (armor && !clearsig) {
		int		armorType = PGP_ARMOR_SEPSIG;

		switch (sepsig) {
			case 0:
				armorType = PGP_ARMOR_NORMAL;
				break;
			/*
			* XXX: This case is a hack until PGP/MIME is properly
			* implemented in the library. It indicates that the
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
		tail = pgpArmorWriteCreate (tail, env, fd, rng, version,
									armorType);
	}

	if (!tail) {
err:
		/* Clean up on error */
		tail = NULL;
	}
	return tail;
}


/* Create a list of hash bytes from a sigspec list, and return the length */
static byte *
createHashList(struct PgpSigSpec const *sigspecs, int *count)
{
	int cnt;
	struct PgpSigSpec const *ss1, *ss2;
	byte *list;

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
	list = (byte *)pgpMemAlloc(cnt);
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
