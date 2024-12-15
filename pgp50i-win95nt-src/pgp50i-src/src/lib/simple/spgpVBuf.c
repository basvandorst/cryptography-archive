/*
* spgpVBuf.c -- Simple PGP API Verify detached signature buffer
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpVBuf.c,v 1.13.2.4 1997/06/07 09:51:51 mhw Exp $
*/

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int SPGPExport
SimplePGPVerifyDetachedSignatureBufferX (void *handle,
	char *InputDataBuffer, size_t InputDataBufferLen,
	char *InputSignatureBuffer, size_t InputSignatureBufferLen,
	int *SignatureStatus, byte *SignerKeyID, size_t SignerKeyIDBufLen,
	char *Signer, size_t SignerBufLen, byte *SignDate, size_t SignDateBufLen,
	PGPFileRef *PublicKeyRingRef, SPGPProgressCallBack callBack,
		void *callBackArg)
	{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub;	 /* Public keyring file */
		RingSet const	*rspub;	/* Public keyring ringset */
		PgpFile	*pfpub;	/* Public keyring file handle */
		PgpPipeline	*texthead,	/* Pipeline text head */
			*sighead,	/* Pipeline signature head */
			*tail;	/* Pipeline tail pointer */
		PgpUICb	ui;			/* Callback functions */
		SPgpSimpUI	ui_arg;	/* Parameters for callbacks */
		int	err;	 		/* Error variable */

		(void)handle;

	/* Initialize some null pointers to simplify error cleanup */
	rfpub = NULL;
	pfpub = NULL;
	env = NULL;
	texthead = NULL;
	sighead = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	/* Some pointer checks */
	if (!InputDataBuffer) {
		err =
		SIMPLEPGPVERIFYDETACHEDSIGNATUREBUFFER_NULLPOINTERTOINPUTDATABUFFER;
		goto error;
	}
	if (!InputSignatureBuffer) {
		err =
		SIMPLEPGPVERIFYDETACHEDSIGNATUREBUFFER_NULLPOINTERTOINPUTSIGBUFFER;
		goto error;
	}
	if (InputDataBufferLen == 0) {
		err =
		  SIMPLEPGPVERIFYDETACHEDSIGNATUREBUFFER_INPUTDATABUFFERLENGTHISZERO;
		goto error;
	}
	if (InputSignatureBufferLen == 0) {
		err =
		  SIMPLEPGPVERIFYDETACHEDSIGNATUREBUFFER_INPUTSIGBUFFERLENGTHISZERO;
		goto error;
	}
	if (!env || !ringpool) {
		err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREBUFFER_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
		goto error;
	}

	/* Open keyring and get ringset */
	err = spgpOpenRingfiles (env, ringpool, PublicKeyRingRef, NULL,
		&pfpub, &rfpub, &rspub, NULL, NULL, NULL, FALSE);
	if (err)
		goto error;

	/* Setup the UI callback functions & args */
	spgpUISetup (&ui, &ui_arg);
	ui_arg.env = env;	 		 	/* Environment */
	ui_arg.ringset = rspub;			/* Keyring */

	/* Set up pipeline */
	tail = pgpSignatureVerifyCreate (&texthead, &sighead, env, NULL,
				NULL, 0, 0, &ui, &ui_arg);
	if (!tail) {
		err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREBUFFER_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
		goto error;
	}

	/* Send data through */
	err = spgpMemPump (sighead, (byte *)InputSignatureBuffer,
		InputSignatureBufferLen, NULL, NULL);
	if (err)
		goto error;
	err = spgpMemPump (texthead, (byte *)InputDataBuffer, InputDataBufferLen,
						callBack, callBackArg);
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
SimplePGPVerifyDetachedSignatureBuffer (void *handle,
	char *InputDataBuffer, size_t InputDataBufferLen,
	char *InputSignatureBuffer, size_t InputSignatureBufferLen,
	int *SignatureStatus, char *Signer, size_t SignerBufLen,
	byte *SignDate, size_t SignDateBufLen,
	char *PublicKeyRingName)
{
	PGPFileRef			*refPubRing=NULL;
	PGPError			err;

	if (PublicKeyRingName && PublicKeyRingName[0]) {
		if (!(refPubRing = pgpNewFileRefFromFullPath (PublicKeyRingName))) {
			err =
	SIMPLEPGPVERIFYDETACHEDSIGNATUREBUFFER_NOTENOUGHMEMORYFORINPUTDATASTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPVerifyDetachedSignatureBufferX (handle, InputDataBuffer,
		InputDataBufferLen, InputSignatureBuffer, InputSignatureBufferLen,
		SignatureStatus, NULL, 0, Signer, SignerBufLen,
		SignDate, SignDateBufLen, refPubRing, NULL, NULL);

error:

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
