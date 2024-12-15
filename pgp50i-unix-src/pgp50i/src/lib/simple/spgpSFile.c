/*
* spgpSFile.c -- Simple PGP API Sign file
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpSFile.c,v 1.27.2.4 1997/06/07 09:51:51 mhw Exp $
*/

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int
SimplePGPSignFileX (void *handle,
	PGPFileRef *InputFileRef, PGPFileRef *OutputFileRef,
	int Armor, int Textmode, int SignatureOnly, int ClearSign,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	PGPFileRef *PrivateKeyRingRef, PGPFileOpenFlags localEncode,
	SPGPProgressCallBack callBack, void *callBackArg)
{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfsec;	 /* Secret keyring file */
		RingSet const	*rssec;	/* Secret keyring ringset */
		RingObject	*robsign;	 /* RingObj for signing key */
		PgpFile	*pfsec,		/* Secret keyring file handle */
			*pfout;		/* Output file handle */
		PgpFileRead	*pfrin;		/* Input file handle */
		PgpRandomContext	*rng;			/* Random number generator */
		PgpSecKey	*sksign;		/* Signature key */
		PgpSigSpec	*sigspec;	 /* Signature data structure */
		PgpLiteralParams	literal;	/* Filename info */
		PgpPipeline	*head,			/* Pipeline head */
			**tail;	/* Pipeline tail pointer */
		int	err;	 		/* Error variable */
		int	rslt;		/* Return code from pgplib */
		PGPFileOpenFlags		openFlags;
		PGPFileType		 		fileType;

	pgpAssert((localEncode & ~(kPGPFileOpenMaybeLocalEncode |
								kPGPFileOpenForceLocalEncode)) == 0);
	(void)handle;

	/* Initialize some null pointers to simplify error cleanup */
	sksign = NULL;
	sigspec = NULL;
	rfsec = NULL;
	pfsec = NULL;
	rng = NULL;
	env = NULL;
	literal.filename = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	if (!env || !ringpool) {
		err = SIMPLEPGPSIGNFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}

	/* Make sure we have our RNG seeded */
	if (spgpRngCheck () != PGPERR_OK) {
		err = SIMPLEPGP_RNGNOTSEEDED;
		goto error;
	}

	/* Set the armor flag if requested */
	if (ClearSign) {
		/* Clearsign implies these others */
		Armor = Textmode = TRUE;
	}
	pgpenvSetInt (env, PGPENV_ARMOR, Armor, PGPENV_PRI_FORCE);
	pgpenvSetInt (env, PGPENV_TEXTMODE, Textmode, PGPENV_PRI_FORCE);
	pgpenvSetInt (env, PGPENV_CLEARSIG, ClearSign, PGPENV_PRI_FORCE);

	/* Open keyring and get ringset */
	err = spgpOpenRingfiles (env, ringpool, NULL, PrivateKeyRingRef,
		NULL, NULL, NULL, &pfsec, &rfsec, &rssec, FALSE);
	if (err)
		goto error;


	/* Initialize random number generator */
	rng = pgpRandomCreate ();

	/* Prepare signature struct */
	if (!SignerKeyID) {
		err = SIMPLEPGPSIGNFILE_NULLPOINTERTOSIGNERKEYIDSTRING;
		goto error;
	}
	if (!SignerPassphrase) {
		err = SIMPLEPGPSIGNFILE_NULLPOINTERTOSIGNERPASSPHRASESTRING;
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

	/* Through with key rings, close them */
	spgpRingFileClose (rfsec);
	pgpFileClose (pfsec);
	rfsec = NULL;
	pfsec = NULL;

	/* Open input and output files */

	openFlags = kPGPFileOpenReadPerm | localEncode;
	if (SignatureOnly)
		openFlags |= kPGPFileOpenLocalEncodeHashOnly;

	pfrin = pgpFileRefReadCreate (InputFileRef, openFlags, NULL);
	if (!pfrin) {
		err = SIMPLEPGPSIGNFILE_INPUTFILEDOESNOTEXIST;
		goto error;
	}
	pgpFileReadSetCallBack(pfrin, callBack, callBackArg);
	literal.filename = pgpGetFileRefName (InputFileRef);
	literal.timestamp = (word32) 0;
	
	if (Armor)
		fileType = kPGPFileTypeArmorFile;
	else if (SignatureOnly)
		fileType = kPGPFileTypeDetachedSig;
	else
		fileType = kPGPFileTypeSignedData;
	
	pfout = pgpFileRefOpen (OutputFileRef, kPGPFileOpenStdWriteFlags,
							fileType, NULL);
	if (!pfout) {
		err = SIMPLEPGPSIGNFILE_OUTPUTFILECREATIONERROR;
		goto error;
	}

	/* Set up pipeline */
	head = NULL;
	tail = pgpEncryptPipelineCreate (&head, env, NULL, rng, NULL, NULL,
							sigspec, &literal, SignatureOnly);
	if (Armor || ClearSign) {
		/* Convert to local line endings if appropriate */
		tail = pgpTextFiltCreate (tail,
				pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1, NULL,
								NULL),
				0, SimplePGPGetLineEndType());
	}
	if (!pgpFileWriteCreate (tail, pfout, 1)) {
		err = SIMPLEPGPSIGNFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
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
	if (sksign)
		pgpSecKeyDestroy (sksign);
	if (rng)
		pgpRandomDestroy (rng);
	if (rfsec)
		spgpRingFileClose (rfsec);
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
SimplePGPSignFile (void *handle,
	char *InputFileName, char *OutputFileName,
	int Armor, int Textmode, int SignatureOnly, int ClearSign,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	char *PrivateKeyRingName)
{
	PGPFileRef			*refInput=NULL,
						*refOutput=NULL,
						*refPrivRing=NULL;
	PGPError			err;

	/* Some pointer checks */
	if (!InputFileName) {
		err = SIMPLEPGPSIGNFILE_NULLPOINTERTOINPUTFILENAME;
		goto error;
	}
	if (!OutputFileName) {
		err = SIMPLEPGPSIGNFILE_NULLPOINTERTOOUTPUTFILENAME;
		goto error;
	}
	if (*InputFileName == '\0') {
		err = SIMPLEPGPSIGNFILE_EMPTYINPUTFILENAMESTRING;
		goto error;
	}
	if (*OutputFileName == '\0') {
		err = SIMPLEPGPSIGNFILE_EMPTYOUTPUTFILENAMESTRING;
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
	if (PrivateKeyRingName && PrivateKeyRingName[0]) {
		if (!(refPrivRing = pgpNewFileRefFromFullPath (PrivateKeyRingName))) {
			err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPSignFileX (handle, refInput, refOutput,
		Armor, Textmode, SignatureOnly, ClearSign, SignerKeyID,
		SignerBufferLen, SignerPassphrase, SignerPwdBufferLen,
		refPrivRing, kPGPFileOpenMaybeLocalEncode, NULL, NULL);

error:

	if (refInput)
		pgpFreeFileRef (refInput);
	if (refOutput)
		pgpFreeFileRef (refOutput);
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
