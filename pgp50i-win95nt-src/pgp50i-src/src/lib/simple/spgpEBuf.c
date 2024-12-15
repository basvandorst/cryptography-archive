/*
 * spgpEBuf.c -- Simple PGP API Encrypt buffer
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: spgpEBuf.c,v 1.19.2.4 1997/06/07 09:51:47 mhw Exp $
 */

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int
SimplePGPEncryptBufferX (void *handle,
	char *InputBuffer, size_t InputBufferLen, char *InputBufferName,
	char *OutputBuffer, size_t *OutputBufferLen,
	int SignIt, int Armor, int Textmode, int IDEAOnly,
	int UseUntrustedKeys, int ForYourEyesOnly, char *RecipientList,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	char *IDEAPassphrase, size_t IDEAPwdBufferLen,
	PGPFileRef *PublicKeyRingRef, PGPFileRef *PrivateKeyRingRef,
	SPGPProgressCallBack callBack, void *callBackArg)
	{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub;	 /* Public keyring file */
		RingSet const	*rspub;	/* Public keyring ringset */
		RingFile	*rfsec;	 /* Secret keyring file */
		RingSet const	*rssec;	/* Secret keyring ringset */
		RingObject	*robsign;	 /* RingObj for signing key */
		PgpFile	*pfpub,		/* Public keyring file handle */
			*pfsec;		/* Secret keyring file handle */
		PgpRandomContext	*rng;			/* Random number generator */
		PgpPubKey	*pubkeys;	/* All public keys for recips */
		PgpSecKey	*sksign;		/* Signature key */
		PgpSigSpec	*sigspec;	 /* Signature data structure */
		PgpConvKey	convkey,	/* Conventional key */
			*convkeys;	/* Pointer to convkey */
		PgpLiteralParams	literal;	/* Filename info */
		PgpPipeline	*head,		/* Pipeline head */
			**tail, /* Pipeline tail pointer */
			*pipebuf;	/* Output buffer pipeline */
		size_t	bufsize; /* Output buffer size */
		int	err;	 	/* Error variable */

	(void)handle;
	(void)InputBufferName;

	/* Initialize some null pointers to simplify error cleanup */
	pubkeys = NULL;
	convkeys = NULL;
	sksign = NULL;
	sigspec = NULL;
	rfpub = rfsec = NULL;
	pfpub = pfsec = NULL;
	rng = NULL;
	env = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	/* Some pointer checks */
	if (!InputBuffer) {
		err = SIMPLEPGPENCRYPTBUFFER_NULLPOINTERTOINPUTBUFFER;
		goto error;
	}
	if (!OutputBuffer) {
		err = SIMPLEPGPENCRYPTBUFFER_NULLPOINTERTOOUTPUTBUFFER;
		goto error;
	}
	if (InputBufferLen == 0) {
		err = SIMPLEPGPENCRYPTBUFFER_INPUTBUFFERLENGTHISZERO;
		goto error;
	}
	if (!env || !ringpool) {
		err = SIMPLEPGPENCRYPTBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}

	/* Make sure we have our RNG seeded */
	if (spgpRngCheck () != PGPERR_OK) {
		err = SIMPLEPGP_RNGNOTSEEDED;
		goto error;
	}

	/* Set the armor flag if requested */
	pgpenvSetInt (env, PGPENV_ARMOR, Armor, PGPENV_PRI_FORCE);
	pgpenvSetInt (env, PGPENV_TEXTMODE, Textmode, PGPENV_PRI_FORCE);

	/* Open keyrings and get ringsets */
	err = spgpOpenRingfiles (env, ringpool,
		PublicKeyRingRef, PrivateKeyRingRef,
		&pfpub, &rfpub, &rspub, &pfsec, &rfsec, &rssec, FALSE);
	if (err)
		goto error;

	/* Initialize random number generator */
	rng = pgpRandomCreate ();

	if (IDEAOnly) {
		/* Conventional encryption */
		if (!IDEAPassphrase) {
			err = SIMPLEPGPENCRYPTBUFFER_NULLPOINTERTOIDEAPASSPHRASESTRING;
			goto error;
		}
		if (!*IDEAPassphrase || IDEAPwdBufferLen) {
			/* Can't ask user for pass phrase, not implementing UI */
			err = SIMPLEPGP_UNIMPLEMENTED;
		}
		convkey.next = NULL;
		convkey.stringToKey = 0;
		convkey.pass = IDEAPassphrase;
		convkey.passlen = strlen (convkey.pass);
		convkeys = &convkey;
	} else {
		/* Public key encryption */
		pubkeys = spgpRecipToPubkeys (RecipientList, rspub, UseUntrustedKeys,
			 		 		 		 		env, &err);
			 if (err)
			 	goto error;
		}

	/* Prepare signature struct if requested */
	if (SignIt) {
		int rslt;
		if (!SignerKeyID) {
			err = SIMPLEPGPENCRYPTBUFFER_NULLPOINTERTOSIGNERKEYIDSTRING;
			goto error;
		}
		if (!SignerPassphrase) {
			err = SIMPLEPGPENCRYPTBUFFER_NULLPOINTERTOSIGNERPASSPHRASESTRING;
			goto error;
		}
		/* Not implemented to ask for pass phrase, no UI functionality */
		if (*SignerPassphrase=='\0' && SignerPwdBufferLen) {
			err = SIMPLEPGP_UNIMPLEMENTED;
			goto error;
		}
		
		/* Get signing key and data */
		robsign = spgpGetSignerKey (SignerKeyID, SignerBufferLen, env, rssec);
		if (!robsign) {
			err = SIMPLEPGP_NONEXISTENTSIGNERKEY;
			goto error;
		}
		sksign = ringSecSecKey (rssec, robsign, PGP_PKUSE_SIGN);
		ringObjectRelease (robsign);
		if (!sksign || !sksign->sign) {
			err = SIMPLEPGP_SIGNERKEYENCRYPTIONONLY;
			if (!sksign) {
				/* Problem with key itself; if UNIMP, use high level error */
				PGPError err1 = ringSetError(rssec)->error;
				if (err1 != PGPERR_PUBKEY_UNIMP) {
					err = err1;
				}
			}
			goto error;
		}
		rslt = pgpSecKeyUnlock (sksign, env, SignerPassphrase,
								strlen(SignerPassphrase));
		if (rslt <= 0) {
			err = SIMPLEPGP_BADKEYPASSPHRASE;
			goto error;
		}

		sigspec = pgpSigSpecCreate (env, sksign,
				(byte)(Textmode ? PGP_SIGTYPE_TEXT : PGP_SIGTYPE_BINARY));
	}

	/* Through with key rings, close them */
	spgpRingFileClose (rfpub);
	spgpRingFileClose (rfsec);
	pgpFileClose (pfpub);
	pgpFileClose (pfsec);
	rfpub = rfsec = NULL;
	pfpub = pfsec = NULL;

	/* Fake up literal params */
	if (ForYourEyesOnly) {
		static char fyeomagic[] = "_CONSOLE";
		literal.filename = pgpMemAlloc (sizeof(fyeomagic));
		if (!literal.filename) {
			err = PGPERR_NOMEM;
			goto error;
		}
		strcpy ((char *)literal.filename, fyeomagic);
	} else {
		literal.filename = "stdin";
	}
	literal.timestamp = (word32) 0;

	/* Add some data from the buffer to the RNG for more entropy */
	pgpRandomAddBytes (rng, InputBuffer, min(1024, InputBufferLen));

	/* Set up pipeline */
head = NULL;
	tail = pgpEncryptPipelineCreate (&head, env, NULL, rng, convkeys, pubkeys,
									sigspec, &literal, 0);
	if (Armor) {
		/* Convert to local line endings if appropriate */
		tail = pgpTextFiltCreate (tail,
				pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL,
									NULL),
				0, SimplePGPGetLineEndType());
	}
	bufsize = *OutputBufferLen;
	pipebuf = pgpMemModCreate (tail, OutputBuffer, bufsize);
	if (!pipebuf) {
		err = SIMPLEPGPENCRYPTBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}

	/* Send data through */
	err = spgpMemPump (head, (byte *)InputBuffer, InputBufferLen,
						callBack, callBackArg);
	if (!err) {
		err = spgpMemOutput (pipebuf, 0, OutputBufferLen);
	}
	head->teardown (head);

error:
	if (sigspec)
		pgpSigSpecDestroy (sigspec);
	while (pubkeys) {
		PgpPubKey *pk1 = pgpPubKeyNext (pubkeys);
		pgpPubKeyDestroy (pubkeys);
		pubkeys = pk1;
	}
	if (sksign)
		pgpSecKeyDestroy (sksign);
	if (rng)
		pgpRandomDestroy (rng);
	if (rfpub)
		spgpRingFileClose (rfpub);
	if (rfsec)
		spgpRingFileClose (rfsec);
	if (pfpub)
		pgpFileClose (pfpub);
	if (pfsec)
		pgpFileClose (pfsec);
	spgpFinish (env, ringpool, err);
	return err;
}

int
SimplePGPEncryptBuffer (void *handle,
	char *InputBuffer, size_t InputBufferLen, char *InputBufferName,
	char *OutputBuffer, size_t *OutputBufferLen,
	int SignIt, int Armor, int Textmode, int IDEAOnly,
	int UseUntrustedKeys, char *RecipientList,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	char *IDEAPassphrase, size_t IDEAPwdBufferLen,
	char *PublicKeyRingName, char *PrivateKeyRingName)
{
	PGPFileRef			*refPubRing=NULL,
						*refPrivRing=NULL;
	PGPError			err;

	if (PublicKeyRingName && PublicKeyRingName[0]) {
		if (!(refPubRing = pgpNewFileRefFromFullPath (PublicKeyRingName))) {
			err = SIMPLEPGPENCRYPTBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}
	if (PrivateKeyRingName && PrivateKeyRingName[0]) {
		if (!(refPrivRing = pgpNewFileRefFromFullPath (PrivateKeyRingName))) {
			err = SIMPLEPGPENCRYPTBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPEncryptBufferX (handle, InputBuffer, InputBufferLen,
		InputBufferName, OutputBuffer, OutputBufferLen,
		SignIt, Armor, Textmode, IDEAOnly, UseUntrustedKeys, FALSE,
		RecipientList, SignerKeyID, SignerBufferLen, SignerPassphrase,
		SignerPwdBufferLen, IDEAPassphrase, IDEAPwdBufferLen,
		refPubRing, refPrivRing, NULL, NULL);

error:

	if (refPubRing)
		pgpFreeFileRef (refPubRing);
	if (refPrivRing)
		pgpFreeFileRef (refPrivRing);
	return err;
}

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
