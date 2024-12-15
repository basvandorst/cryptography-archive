/*
* spgpVFile.c -- Simple PGP API Verify detached signature file
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpVFile.c,v 1.16.2.5 1997/06/07 09:51:52 mhw Exp $
*/

#include <stdio.h>
#include <time.h>

#include "spgp.h"
#include "spgpint.h"

int SPGPExport
SimplePGPVerifyDetachedSignatureFileX (void *handle,
	PGPFileRef *InputDataFileRef, PGPFileRef *InputSignatureFileRef,
	int *SignatureStatus, byte *SignerKeyID, size_t SignerKeyIDBufLen,
	char *Signer, size_t SignerBufLen, byte *SignDate, size_t SignDateBufLen,
	PGPFileRef *PublicKeyRingRef, PGPFileOpenFlags localEncode,
	SPGPProgressCallBack callBack, void *callBackArg)
{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub;		/* Public keyring file */
		RingSet const	*rspub;		/* Public keyring ringset */
		PgpFile	*pfpub;		/* Public keyring file handle */
		PgpFileRead	*pfrdat,		/* Data file handle */
			*pfrsig;		/* Signature file handle */
		PgpPipeline	*texthead,	/* Pipeline text head */
			*sighead,	/* Pipeline signature head */
			*tail; /* Pipeline tail pointer */
		PgpUICb	ui;		/* Callback functions */
		SPgpSimpUI	ui_arg; /* Parameters for callbacks */
		int	err;	 	/* Error variable */

	pgpAssert((localEncode & ~(kPGPFileOpenMaybeLocalEncode |
								kPGPFileOpenForceLocalEncode)) == 0);
	(void)handle;

	/* Initialize some null pointers to simplify error cleanup */
	rfpub = NULL;
	pfpub = NULL;
	env = NULL;
	texthead = NULL;
	sighead = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	if (!env || !ringpool) {
		err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
		goto error;
	}

	/* Open keyring and get ringset */
	err = spgpOpenRingfiles (env, ringpool, PublicKeyRingRef, NULL,
		&pfpub, &rfpub, &rspub, NULL, NULL, NULL, FALSE);
	if (err)
		goto error;

	/* Setup the UI callback functions & args */
		spgpUISetup (&ui, &ui_arg);
		ui_arg.env = env;					/* Environment */
		ui_arg.ringset = rspub;		 		/* Keyring */
	
	/* Open data input file */
	pfrdat = pgpFileRefReadCreate (InputDataFileRef,
			(kPGPFileOpenReadPerm | localEncode |
			kPGPFileOpenLocalEncodeHashOnly),
			NULL);
	if (!pfrdat) {
		err = SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_INPUTDATAFILEDOESNOTEXIST;
		goto error;
	}
	pgpFileReadSetCallBack(pfrdat, callBack, callBackArg);
	
	/* Open signature input file */
	pfrsig = pgpFileRefReadCreate (InputSignatureFileRef,
									kPGPFileOpenReadPerm, NULL);
	if (!pfrsig) {
		err = SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_INPUTSIGFILEDOESNOTEXIST;
		goto error;
	}

	/* Set up pipeline */
	tail = pgpSignatureVerifyCreate (&texthead, &sighead, env, NULL,
				NULL, 0, 0, &ui, &ui_arg);
	if (!tail) {
		err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
		goto error;
	}

	/* Send data through */
	err = pgpFileReadPump (pfrsig, sighead);
	pgpFileReadDestroy (pfrsig);
	sighead->sizeAdvise (sighead, 0);
	if (err)
		goto error;
	err = pgpFileReadPump (pfrdat, texthead);
	pgpFileReadDestroy (pfrdat);
	texthead->sizeAdvise (texthead, 0);
	sighead->teardown (sighead);
	texthead->teardown (texthead);
	sighead = texthead = NULL;
	if (err)
		goto error;

	/* Set signer info if avail */
	spgpSignStatus (&ui_arg, rspub, SignatureStatus,
		SignerKeyID, SignerKeyIDBufLen, Signer, SignerBufLen,
		SignDate, SignDateBufLen);
	
	/* Through with key rings, close them */
	spgpRingFileClose (rfpub);
	pgpFileClose (pfpub);
	rfpub = NULL;
	pfpub = NULL;

	if (ui_arg.encfail || ui_arg.keyfail) {
		err = SIMPLEPGP_DETACHEDSIGNATURENOTFOUND;
	} else {
		err = 0;
	}

error:
	if (sighead)
		sighead->teardown (sighead);
	if (texthead)
		texthead->teardown (texthead);
	if (rfpub)
		spgpRingFileClose (rfpub);
	if (pfpub)
		pgpFileClose (pfpub);
	spgpFinish (env, ringpool, err);
	return err;
}

int
SimplePGPVerifyDetachedSignatureFile (void *handle,
	char *InputDataFileName, char *InputSignatureFileName,
	int *SignatureStatus, char *Signer, size_t SignerBufLen,
	byte *SignDate, size_t SignDateBufLen,
	char *PublicKeyRingName)
{
	PGPFileRef			*refData=NULL,
						*refSig=NULL,
						*refPubRing=NULL;
	PGPError			err;

	/* Some pointer checks */
	if (!InputDataFileName) {
		err =
		SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_NULLPOINTERTOINPUTDATAFILENAME;
		goto error;
	}
	if (!InputSignatureFileName) {
		err =
		SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_NULLPOINTERTOINPUTSIGFILENAME;
		goto error;
	}
	if (*InputDataFileName == '\0') {
		err =
		SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_EMPTYINPUTDATAFILENAMESTRING;
		goto error;
	}
	if (*InputSignatureFileName == '\0') {
		err =
			SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_EMPTYINPUTSIGFILENAMESTRING;
		goto error;
	}

	if (!(refData = pgpNewFileRefFromFullPath (InputDataFileName))) {
		err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
		goto error;
	}
	if (!(refSig = pgpNewFileRefFromFullPath (InputSignatureFileName))) {
		err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
		goto error;
	}
	if (PublicKeyRingName && PublicKeyRingName[0]) {
		if (!(refPubRing = pgpNewFileRefFromFullPath (PublicKeyRingName))) {
			err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREFILE_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPVerifyDetachedSignatureFileX (handle, refData, refSig,
		SignatureStatus, NULL, 0, Signer, SignerBufLen,
		SignDate, SignDateBufLen,
		refPubRing, kPGPFileOpenMaybeLocalEncode, NULL, NULL);

error:

	if (refData)
		pgpFreeFileRef (refData);
	if (refSig)
		pgpFreeFileRef (refSig);
	if (refPubRing)
		pgpFreeFileRef (refPubRing);
	return err;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
