/*
 * spgpEFile.c -- Simple PGP API Encrypt file
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: spgpEFile.c,v 1.24.2.4 1997/06/07 09:51:47 mhw Exp $
 */

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int
SimplePGPEncryptFileX (void *handle,
	PGPFileRef *InputFileRef, PGPFileRef *OutputFileRef,
	int SignIt, int Wipe, int Armor, int Textmode, int IDEAOnly,
	int UseUntrustedKeys, int ForYourEyesOnly, char *RecipientList,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	char *IDEAPassphrase, size_t IDEAPwdBufferLen,
	PGPFileRef *PublicKeyRingRef, PGPFileRef *PrivateKeyRingRef,
	PGPFileOpenFlags localEncode, SPGPProgressCallBack callBack,
	void *callBackArg)
{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub;	 /* Public keyring file */
		RingSet const	*rspub;	/* Public keyring ringset */
		RingFile	*rfsec;	 /* Secret keyring file */
		RingSet const	*rssec;	/* Secret keyring ringset */
		RingObject	*robsign;	 /* RingObj for signing key */
		PgpFile	*pfpub,		/* Public keyring file handle */
			*pfsec,		/* Secret keyring file handle */
			*pfout;		/* Output file handle */
		PgpFileRead	*pfrin;		/* Input file handle */
		PgpRandomContext	*rng;			/* Random number generator */
		PgpPubKey	*pubkeys;	/* All public keys for recips */
		PgpSecKey	*sksign;		/* Signature key */
		PgpSigSpec	*sigspec;	 /* Signature data structure */
		PgpConvKey	convkey,	/* Conventional key */
			*convkeys;	/* Pointer to convkey */
		PgpLiteralParams	literal;	/* Filename info */
		PgpPipeline	*head,		/* Pipeline head */
			**tail; /* Pipeline tail pointer */
		byte const		 		*peekbuf;	/* RNG seed data from file */
		size_t		 			peeklength; /* Size of peekbuf data */
		int	err;	 	/* Error variable */

	pgpAssert((localEncode & ~(kPGPFileOpenMaybeLocalEncode |
								kPGPFileOpenForceLocalEncode)) == 0);
	(void)handle;

	/* Initialize some null pointers to simplify error cleanup */
	pubkeys = NULL;
	convkeys = NULL;
	sksign = NULL;
	sigspec = NULL;
	rfpub = rfsec = NULL;
	pfpub = pfsec = NULL;
	rng = NULL;
	env = NULL;
	literal.filename = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	if (Wipe) {
		err = SIMPLEPGP_UNIMPLEMENTED;
		goto error;
	}
	if (!env || !ringpool) {
		err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
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
			err = SIMPLEPGPENCRYPTFILE_NULLPOINTERTOIDEAPASSPHRASESTRING;
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
			err = SIMPLEPGPENCRYPTFILE_NULLPOINTERTOSIGNERKEYIDSTRING;
			goto error;
		}
		if (!SignerPassphrase) {
			err = SIMPLEPGPENCRYPTFILE_NULLPOINTERTOSIGNERPASSPHRASESTRING;
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
	
	/* Open input and output files */
	pfrin = pgpFileRefReadCreate (InputFileRef, (kPGPFileOpenReadPerm |
												localEncode), NULL);
	if (!pfrin) {
		err = SIMPLEPGPENCRYPTFILE_INPUTFILEDOESNOTEXIST;
		goto error;
	}
	pgpFileReadSetCallBack(pfrin, callBack, callBackArg);
	if (ForYourEyesOnly) {
		static char fyeomagic[] = "_CONSOLE";
		literal.filename = pgpMemAlloc (sizeof(fyeomagic));
		if (!literal.filename) {
			err = PGPERR_NOMEM;
			goto error;
		}
		strcpy ((char *)literal.filename, fyeomagic);
	} else {
		literal.filename = pgpGetFileRefName (InputFileRef);
	}
	literal.timestamp = (word32) 0;
	pfout = pgpFileRefOpen (OutputFileRef, kPGPFileOpenStdWriteFlags,
		(Armor?kPGPFileTypeArmorFile:kPGPFileTypeEncryptedData), NULL);
	if (!pfout) {
		err = SIMPLEPGPENCRYPTFILE_OUTPUTFILECREATIONERROR;
		goto error;
	}

	/* Add some data from the file to the RNG for more entropy */
	peekbuf = pgpFileReadPeek (pfrin, &peeklength);
	pgpRandomAddBytes (rng, peekbuf, peeklength);

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
	if (!pgpFileWriteCreate (tail, pfout, 1)) {
		err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	/* Send data through */
	err = pgpFileReadPump (pfrin, head);
	pgpFileReadDestroy (pfrin);
	head->sizeAdvise (head, 0);
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
	if (literal.filename)
		pgpFree ((char *)literal.filename);
	if (err != PGPERR_OK)
		pgpDeleteFile (OutputFileRef);
	spgpFinish (env, ringpool, err);
	return err;
}

int
SimplePGPEncryptFile (void *handle,
	char *InputFileName, char *OutputFileName,
	int SignIt, int Wipe, int Armor, int Textmode, int IDEAOnly,
	int UseUntrustedKeys, char *RecipientList,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	char *IDEAPassphrase, size_t IDEAPwdBufferLen,
	char *PublicKeyRingName, char *PrivateKeyRingName)
{
	PGPFileRef			*refInput=NULL,
						*refOutput=NULL,
						*refPubRing=NULL,
						*refPrivRing=NULL;
	PGPError			err;

	/* Some pointer checks */
	if (!InputFileName) {
		err = SIMPLEPGPENCRYPTFILE_NULLPOINTERTOINPUTFILENAME;
		goto error;
	}
	if (!OutputFileName) {
		err = SIMPLEPGPENCRYPTFILE_NULLPOINTERTOOUTPUTFILENAME;
		goto error;
	}
	if (*InputFileName == '\0') {
		err = SIMPLEPGPENCRYPTFILE_EMPTYINPUTFILENAMESTRING;
		goto error;
	}
	if (*OutputFileName == '\0') {
		err = SIMPLEPGPENCRYPTFILE_EMPTYOUTPUTFILENAMESTRING;
		goto error;
	}

	if (!(refInput = pgpNewFileRefFromFullPath (InputFileName))) {
		err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	if (!(refOutput = pgpNewFileRefFromFullPath (OutputFileName))) {
		err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	if (PublicKeyRingName && PublicKeyRingName[0]) {
		if (!(refPubRing = pgpNewFileRefFromFullPath (PublicKeyRingName))) {
			err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}
	if (PrivateKeyRingName && PrivateKeyRingName[0]) {
		if (!(refPrivRing = pgpNewFileRefFromFullPath (PrivateKeyRingName))) {
			err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPEncryptFileX (handle, refInput, refOutput,
		SignIt, Wipe, Armor, Textmode, IDEAOnly, UseUntrustedKeys, FALSE,
		RecipientList, SignerKeyID, SignerBufferLen, SignerPassphrase,
		SignerPwdBufferLen, IDEAPassphrase, IDEAPwdBufferLen,
		refPubRing, refPrivRing, kPGPFileOpenMaybeLocalEncode, NULL, NULL);

error:

	if (refInput)
		pgpFreeFileRef (refInput);
	if (refOutput)
		pgpFreeFileRef (refOutput);
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
