/*
* spgpRFile.c -- Simple PGP API Receive file
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpRFile.c,v 1.20.2.4 1997/06/07 09:51:49 mhw Exp $
*/

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int SPGPExport
SimplePGPReceiveFileX (void *handle,
	PGPFileRef *InputFileRef, PGPFileRef *OutputFileRef,
	char *DecryptPassPhrase, size_t decryptPwdBufferLen,
	int *SignatureStatus, byte *SignerKeyID, size_t SignerKeyIDBufLen,
	char *Signer, size_t SignerBufLen,
	byte *SignDate, size_t SignDateBufLen, Boolean *ForYourEyesOnly,
	PGPFileRef *PublicKeyRingRef, PGPFileRef *PrivateKeyRingRef,
	PGPFileOpenFlags localEncode, SPGPProgressCallBack callBack,
		void *callBackArg)
	{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub;		/* Public keyring file */
		RingSet const	*rspub;		/* Public keyring ringset */
		RingFile	*rfsec;		/* Secret keyring file */
		RingSet const	*rssec;		/* Secret keyring ringset */
		RingSet		 			*rsboth;		/* Union of public and priv ringsets */
		PgpFile	*pfpub,		/* Public keyring file handle */
			*pfsec;		/* Secret keyring file handle */
		PgpFileRead	*pfrin;		/* Input file handle */
		PgpPipeline	*head,			/* Pipeline head */
			**tail;	/* Pipeline tail pointer */
		PgpUICb	ui;			/* Callback functions */
		SPgpSimpUI	ui_arg;	/* Parameters for callbacks */
		int	err;	 		/* Error variable */

	pgpAssert((localEncode & ~(kPGPFileOpenMaybeLocalEncode
							| kPGPFileOpenNoMacBinCRCOkay)) == 0);
	(void)handle;
	(void)decryptPwdBufferLen;

	/* Initialize some null pointers to simplify error cleanup */
	rfpub = rfsec = NULL;
	pfpub = pfsec = NULL;
	rsboth = NULL;
	env = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	if (!env || !ringpool) {
		err = SIMPLEPGPRECEIVEFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}

	/* Open keyrings and get ringsets */
	err = spgpOpenRingfiles (env, ringpool,
		PublicKeyRingRef, PrivateKeyRingRef,
		&pfpub, &rfpub, &rspub, &pfsec, &rfsec, &rssec, FALSE);
	if (err)
		goto error;

	/* Create union of the two ringsets */
	rsboth = ringSetUnion (rspub, rssec);

	/* Setup the UI callback functions & args */
	spgpUISetup (&ui, &ui_arg);
	ui_arg.env = env;	 		 	/* Environment */
	ui_arg.ringset = rsboth;		/* Keyrings */
	ui_arg.passphrase = DecryptPassPhrase; /* Key/msg passphrase */
	ui_arg.outref = OutputFileRef;	/* Filename */
	ui_arg.localEncode = localEncode;

	/* Open input file (output file will be opened by library callback) */
	pfrin = pgpFileRefReadCreate (InputFileRef, kPGPFileOpenReadPerm, NULL);
	if (!pfrin) {
		err = SIMPLEPGPRECEIVEFILE_INPUTFILEDOESNOTEXIST;
		goto error;
	}
	pgpFileReadSetCallBack(pfrin, callBack, callBackArg);

	/* Set up pipeline */
	head = NULL;
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);

	/* Send data through */
	err = pgpFileReadPump (pfrin, head);
	pgpFileReadDestroy (pfrin);
	head->sizeAdvise (head, 0);
	head->teardown (head);
	ringSetDestroy (rsboth);
	rsboth = NULL;

	/* PGPERR_CB_INVALID means we aborted due to no valid ESK decrypt */
	if (err != PGPERR_OK && err != PGPERR_CB_INVALID)
		goto error;

	/* Set FYEO mode if appropriate */
	if (ForYourEyesOnly) {
		*ForYourEyesOnly = ui_arg.fyeo;
	}

	/* Set signer info if avail */
	spgpSignStatus (&ui_arg, rspub, SignatureStatus,
		SignerKeyID, SignerKeyIDBufLen, Signer, SignerBufLen,
		SignDate, SignDateBufLen);

	/* Calculate error codes, in future return other recipients */
#if 0
	{ char tmpbuf[1024];
	size_t tmpbufsize = sizeof(tmpbuf);
	err = spgpDecryptStatus (&ui_arg, rspub, tmpbuf, &tmpbufsize);
	tmpbuf[min(tmpbufsize, sizeof(tmpbuf)-1)] = '\0';
	printf ("Also to: %s\n", tmpbuf);
	}
#endif
	err = spgpDecryptStatus (&ui_arg, rspub, NULL, NULL);

	/* Through with key rings, close them */
	spgpRingFileClose (rfpub);
	spgpRingFileClose (rfsec);
	pgpFileClose (pfpub);
	pgpFileClose (pfsec);
	rsboth = NULL;
	rfpub = rfsec = NULL;
	pfpub = pfsec = NULL;


error:
	if (rsboth)
		ringSetDestroy (rsboth);
	if (rfpub)
		spgpRingFileClose (rfpub);
	if (rfsec)
		spgpRingFileClose (rfsec);
	if (pfpub)
		pgpFileClose (pfpub);
	if (pfsec)
		pgpFileClose (pfsec);
	if (err != PGPERR_OK)
		pgpDeleteFile (OutputFileRef);
	spgpFinish (env, ringpool, err);
	return err;
}

int
SimplePGPReceiveFile (void *handle,
	char *InputFileName, char *OutputFileName,
	char *DecryptPassPhrase, size_t decryptPwdBufferLen,
	int *SignatureStatus, char *Signer, size_t SignerBufLen,
	byte *SignDate, size_t SignDateBufLen,
	char *PublicKeyRingName, char *PrivateKeyRingName)
{
	PGPFileRef			*refInput=NULL,
						*refOutput=NULL,
						*refPubRing=NULL,
						*refPrivRing=NULL;
	PGPError			err;

	/* Some pointer checks */
	if (!InputFileName) {
		err = SIMPLEPGPRECEIVEFILE_NULLPOINTERTOINPUTFILENAME;
		goto error;
	}
	if (!OutputFileName) {
		err = SIMPLEPGPRECEIVEFILE_NULLPOINTERTOOUTPUTFILENAME;
		goto error;
	}
	if (*InputFileName == '\0') {
		err = SIMPLEPGPRECEIVEFILE_EMPTYINPUTFILENAMESTRING;
		goto error;
	}
	if (*OutputFileName == '\0') {
		err = SIMPLEPGPRECEIVEFILE_EMPTYOUTPUTFILENAMESTRING;
		goto error;
	}

	if (!(refInput = pgpNewFileRefFromFullPath (InputFileName))) {
		err = SIMPLEPGPRECEIVEFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	if (!(refOutput = pgpNewFileRefFromFullPath (OutputFileName))) {
		err = SIMPLEPGPRECEIVEFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	if (PublicKeyRingName && PublicKeyRingName[0]) {
		if (!(refPubRing = pgpNewFileRefFromFullPath (PublicKeyRingName))) {
			err = SIMPLEPGPRECEIVEFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}
	if (PrivateKeyRingName && PrivateKeyRingName[0]) {
		if (!(refPrivRing = pgpNewFileRefFromFullPath (PrivateKeyRingName))) {
			err = SIMPLEPGPRECEIVEFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPReceiveFileX (handle, refInput, refOutput,
		DecryptPassPhrase, decryptPwdBufferLen, SignatureStatus,
		NULL, 0, Signer, SignerBufLen, SignDate, SignDateBufLen, NULL,
		refPubRing, refPrivRing,
		kPGPFileOpenMaybeLocalEncode, NULL, NULL);

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
