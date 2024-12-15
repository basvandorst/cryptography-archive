/*
* spgpSBuf.c -- Simple PGP API Sign buffer
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpSBuf.c,v 1.18.2.4 1997/06/07 09:51:50 mhw Exp $
*/

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int
SimplePGPSignBufferX (void *handle,
	char *InputBuffer, size_t InputBufferLen, char *InputBufferName,
	char *OutputBuffer, size_t *OutputBufferLen,
	int Armor, int Textmode, int SignatureOnly, int ClearSign,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	PGPFileRef *PrivateKeyRingRef, SPGPProgressCallBack callBack,
		void *callBackArg)
	{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfsec;	 /* Secret keyring file */
		RingSet const	*rssec;	/* Secret keyring ringset */
		RingObject	*robsign;	 /* RingObj for signing key */
		PgpFile	*pfsec;	/* Secret keyring file handle */
		PgpRandomContext	*rng;		 /* Random number generator */
		PgpSecKey	*sksign;	/* Signature key */
		PgpSigSpec	*sigspec;	 /* Signature data structure */
		PgpLiteralParams	literal;	/* Filename info */
		PgpPipeline	*head,		/* Pipeline head */
			**tail, /* Pipeline tail pointer */
			*pipebuf;	/* Output buffer pipeline */
		size_t	bufsize;	/* Output buffer size */
		int	err;	 		/* Error variable */
		int	rslt;		/* Return code from pgplib */

	(void)handle;
	(void)InputBufferName;

	/* Initialize some null pointers to simplify error cleanup */
	sksign = NULL;
	sigspec = NULL;
	rfsec = NULL;
	pfsec = NULL;
	rng = NULL;
	env = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	/* Some pointer checks */
	if (!InputBuffer) {
		err = SIMPLEPGPSIGNBUFFER_NULLPOINTERTOINPUTBUFFER;
		goto error;
	}
	if (!OutputBuffer) {
		err = SIMPLEPGPSIGNBUFFER_NULLPOINTERTOOUTPUTBUFFER;
		goto error;
	}
	if (InputBufferLen == 0) {
		err = SIMPLEPGPSIGNBUFFER_INPUTBUFFERLENGTHISZERO;
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

	/* Fake up literal params */
	literal.filename = "stdin";
	literal.timestamp = (word32) 0;

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
	bufsize = *OutputBufferLen;
	pipebuf = pgpMemModCreate (tail, OutputBuffer, bufsize);
	if (!pipebuf) {
		err = SIMPLEPGPSIGNBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
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
	if (sksign)
		pgpSecKeyDestroy (sksign);
	if (rng)
		pgpRandomDestroy (rng);
	if (rfsec)
		spgpRingFileClose (rfsec);
	if (pfsec)
		pgpFileClose (pfsec);
	spgpFinish (env, ringpool, err);
	return err;
}

int
SimplePGPSignBuffer (void *handle,
	char *InputBuffer, size_t InputBufferLen, char *InputBufferName,
	char *OutputBuffer, size_t *OutputBufferLen,
	int Armor, int Textmode, int SignatureOnly, int ClearSign,
	char *SignerKeyID, size_t SignerBufferLen,
	char *SignerPassphrase, size_t SignerPwdBufferLen,
	char *PrivateKeyRingName)
{
	PGPFileRef			*refPrivRing=NULL;
	PGPError			err;

	if (PrivateKeyRingName && PrivateKeyRingName[0]) {
		if (!(refPrivRing = pgpNewFileRefFromFullPath (PrivateKeyRingName))) {
			err = SIMPLEPGPSIGNBUFFER_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPSignBufferX (handle, InputBuffer, InputBufferLen,
		InputBufferName, OutputBuffer, OutputBufferLen, Armor, Textmode,
		SignatureOnly, ClearSign, SignerKeyID, SignerBufferLen,
		SignerPassphrase, SignerPwdBufferLen, refPrivRing, NULL, NULL);

error:

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
