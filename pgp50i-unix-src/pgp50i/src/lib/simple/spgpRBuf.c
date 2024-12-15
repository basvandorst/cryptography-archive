/*
* spgpRBuf.c -- Simple PGP API Receive buffer
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpRBuf.c,v 1.18.2.3 1997/06/07 09:51:49 mhw Exp $
*/

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int SPGPExport
SimplePGPReceiveBufferX (void *handle,
	char *InputBuffer, size_t InputBufferLen, char *InputBufferName,
	char *OutputBuffer, size_t *OutputBufferLen,
	char *DecryptPassPhrase, size_t decryptPwdBufferLen,
	int *SignatureStatus, byte *SignerKeyID, size_t SignerKeyIDBufLen,
	char *Signer, size_t SignerBufLen,
	byte *SignDate, size_t SignDateBufLen, Boolean *ForYourEyesOnly,
	PGPFileRef *PublicKeyRingRef, PGPFileRef *PrivateKeyRingRef,
	SPGPProgressCallBack callBack, void *callBackArg)
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
		PgpPipeline	*head,			/* Pipeline head */
			**tail;	/* Pipeline tail pointer */
		PgpUICb	ui;			/* Callback functions */
		SPgpSimpUI	ui_arg;	/* Parameters for callbacks */
		int	err;	 		/* Error variable */

	(void)handle;
	(void)InputBufferName;
	(void)decryptPwdBufferLen;

	/* Initialize some null pointers to simplify error cleanup */
	rfpub = rfsec = NULL;
	pfpub = pfsec = NULL;
	rsboth = NULL;
	env = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	/* Some pointer checks */
	if (!InputBuffer) {
		err = SIMPLEPGPRECEIVEBUFFER_NULLPOINTERTOINPUTBUFFER;
		goto error;
	}
	if (!OutputBuffer) {
		err = SIMPLEPGPRECEIVEBUFFER_NULLPOINTERTOOUTPUTBUFFER;
		goto error;
	}
	if (InputBufferLen == 0) {
		err = SIMPLEPGPRECEIVEBUFFER_INPUTBUFFERLENGTHISZERO;
		goto error;
	}
	if (!env || !ringpool) {
		err = SIMPLEPGPENCRYPTFILE_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
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
	ui_arg.passphrase = DecryptPassPhrase;	/* Key/msg passphrase */
	ui_arg.outbuf = (byte *)OutputBuffer;	 /* Output buffer */
	ui_arg.outbufsize = *OutputBufferLen;

	/* Set up pipeline */
	head = NULL;
	tail = pgpDecryptPipelineCreate (&head, env, NULL, &ui, &ui_arg);

	/* Send data through */
	err = spgpMemPump (head, (byte *)InputBuffer, InputBufferLen,
						callBack, callBackArg);
	if (!err) {
		if (ui_arg.outbuf) {
			/* If we got no data, outbuf will still be non-NULL */
			/* Gets recognized in spgpDecryptStatus */
		} else {
			err = spgpMemOutput (ui_arg.pipebuf, ui_arg.outbufsize,
				OutputBufferLen);
		}
	}
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
	err = spgpDecryptStatus (&ui_arg, rspub, NULL, 0);

	/* Through with key rings, close them */
	spgpRingFileClose (rfpub);
	spgpRingFileClose (rfsec);
	pgpFileClose (pfpub);
	pgpFileClose (pfsec);
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
	spgpFinish (env, ringpool, err);
	return err;
}

int
SimplePGPReceiveBuffer (void *handle,
	char *InputBuffer, size_t InputBufferLen, char *InputBufferName,
	char *OutputBuffer, size_t *OutputBufferLen,
	char *DecryptPassPhrase, size_t decryptPwdBufferLen,
	int *SignatureStatus, char *Signer, size_t SignerBufLen,
	byte *SignDate, size_t SignDateBufLen,
	char *PublicKeyRingName, char *PrivateKeyRingName)
{
	PGPFileRef			*refPubRing=NULL,
						*refPrivRing=NULL;
	PGPError			err;

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
	err = SimplePGPReceiveBufferX (handle, InputBuffer, InputBufferLen,
		InputBufferName, OutputBuffer, OutputBufferLen, DecryptPassPhrase,
		decryptPwdBufferLen, SignatureStatus, NULL, 0,
		Signer, SignerBufLen, SignDate, SignDateBufLen, NULL,
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
