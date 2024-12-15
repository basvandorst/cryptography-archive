/*
 * spgpExtK.c -- Simple PGP API Extract key
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: spgpExtK.c,v 1.12.2.1 1997/06/07 09:51:48 mhw Exp $
 */

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

static int
SimplePGPExtractKeyCommon (void *handle, char *UserIDKeyID, int fFileout,
	PGPFileRef *OutputFileRef, byte *OutputBuffer, size_t *OutputBufferLen,
	PGPFileRef *KeyRingRef)
	{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub;		/* Public keyring file */
		RingSet const	*rspub;		/* Public keyring ringset */
		RingSet	*rsuser,	/* Keys matching UserIDKeyID */
			*rsuser2;	/* Keys to extract */
		RingIterator	*riuser;	/* Iterator over rsuser */
		RingObject	*robuser;	/* Keyring object for rsuser2 */
		PgpFile	*pfpub;		/* Public keyring file handle */
		FILE	*fd;	/* Output file stream */
		int	level;	/* Iterator hierarchy level */
		int	err;	 		/* Error variable */

	(void)handle;
	/* Initialize some null pointers to simplify error cleanup */
		rfpub = NULL;
		pfpub = NULL;
		rsuser = NULL;
		rsuser2 = NULL;
		env = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	/* Some pointer checks */
	if (!UserIDKeyID) {
		err = SIMPLEPGPEXTRACTKEY_NULLPOINTERTOUIDKIDSTRING;
		goto error;
	}
	if (!UserIDKeyID[0]) {
		err = SIMPLEPGPEXTRACTKEY_UIDKIDSTRINGISEMPTY;
		goto error;
	}

	if (!env || !ringpool) {
		err = SIMPLEPGPEXTRACTKEY_NOTENOUGHMEMORYFOROUTPUTSTRUCTURE;
		goto error;
	}

	/* Open keyring and get ringset */
	err = spgpOpenRingfiles (env, ringpool, KeyRingRef, NULL,
		&pfpub, &rfpub, &rspub, NULL, NULL, NULL, FALSE);
	if (err)
		goto error;

	/* Find first matching key, add it and all its children to rsuser2 */
	rsuser = ringSetCreate (ringpool);
	rsuser2 = ringSetCreate (ringpool);
	if (!rsuser || !rsuser2) {
		err = SIMPLEPGPEXTRACTKEY_NOTENOUGHMEMORYFOROUTPUTSTRUCTURE;
		goto error;
	}
	if (!ringSetFilterSpec (rspub, rsuser, UserIDKeyID, PGP_PKUSE_ENCRYPT)) {
		err = SIMPLEPGP_NONEXISTENTEXTRACTIONKEY;
		goto error;
	}
	ringSetFreeze (rsuser);
	riuser = ringIterCreate (rsuser);
	if (!riuser) {
		err = SIMPLEPGPEXTRACTKEY_NOTENOUGHMEMORYFOROUTPUTSTRUCTURE;
		goto error;
	}
	level = ringIterNextObjectAnywhere (riuser);
	robuser = ringIterCurrentObject (riuser, level);
	do {
		ringSetAddObject (rsuser2, robuser);
		level = ringIterNextObjectAnywhere (riuser);
		if (level > 1)
			robuser = ringIterCurrentObject (riuser, level);
	} while (level > 1);
	ringIterDestroy (riuser);
	ringSetDestroy (rsuser);
	rsuser = NULL;
	ringSetFreeze (rsuser2);

	if (fFileout) {
		/* Open output file */
		fd = pgpFileRefStdIOOpen (OutputFileRef, kPGPFileOpenStdWriteFlags,
									kPGPFileTypeArmorFile, NULL);
		if (!fd) {
			err = SIMPLEPGPEXTRACTKEY_OUTPUTFILECREATIONERROR;
			goto error;
		}

		/* Do the extraction. This won't hurt, much. */
		err = spgpWriteArmoredSetFile (fd, rsuser2, env);
		if (err) {
			pgpStdIOClose (fd);
		}
	} else {
		/* Do it for an output buffer */
		size_t bufsize = *OutputBufferLen;
		err = spgpWriteArmoredSetBuffer (OutputBuffer, OutputBufferLen,
			rsuser2, env);
		if (bufsize < *OutputBufferLen)
			err = SIMPLEPGPEXTRACTKEY_OUTPUTBUFFERTOOSMALL;
	}
	ringSetDestroy (rsuser2);
	rsuser2 = NULL;

error:
	if (rsuser)
		ringSetDestroy (rsuser);
	if (rsuser2)
		ringSetDestroy (rsuser2);
	if (rfpub)
		spgpRingFileClose (rfpub);
	if (pfpub)
		pgpFileClose (pfpub);
	spgpFinish (env, ringpool, err);
	return err;
}


int
SimplePGPExtractKeyX (void *handle, char *UserIDKeyID,
	PGPFileRef *OutputFileRef, PGPFileRef *KeyRingRef)
{
	return SimplePGPExtractKeyCommon (handle, UserIDKeyID, 1, OutputFileRef,
		NULL, NULL, KeyRingRef);
}

int
SimplePGPExtractKey (void *handle, char *UserIDKeyID,
	char *OutputFileName, char *KeyRingName)
{
	PGPFileRef			*refOutput=NULL,
						*refKeyRing=NULL;
	PGPError			err;

	if (!OutputFileName) {
		return SIMPLEPGPEXTRACTKEY_NULLPOINTERTOOUTPUTFILENAME;
	}
	if (!OutputFileName[0]) {
		return SIMPLEPGPEXTRACTKEY_OUTPUTFILENAMESTRINGISEMPTY;
	}

	if (!(refOutput = pgpNewFileRefFromFullPath (OutputFileName))) {
		err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	if (KeyRingName && KeyRingName[0]) {
		if (!(refKeyRing = pgpNewFileRefFromFullPath (KeyRingName))) {
			err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPExtractKeyX (handle, UserIDKeyID, refOutput, refKeyRing);

error:

	if (refOutput)
		pgpFreeFileRef (refOutput);
	if (refKeyRing)
		pgpFreeFileRef (refKeyRing);
	return err;
}

int
SimplePGPExtractKeyBufferX (void *handle, char *UserIDKeyID,
	byte *OutputBuffer, size_t *OutputBufferLen, PGPFileRef *KeyRingRef)
{
	if (!OutputBuffer) {
		return SIMPLEPGPEXTRACTKEY_NULLPOINTERTOOUTPUTBUFFER;
	}
	if (!OutputBufferLen) {
		return SIMPLEPGPEXTRACTKEY_NULLPOINTERTOOUTPUTBUFFERLEN;
	}
	return SimplePGPExtractKeyCommon (handle, UserIDKeyID, 0, NULL,
		OutputBuffer, OutputBufferLen, KeyRingRef);
}

int
SimplePGPExtractKeyBuffer (void *handle, char *UserIDKeyID,
	byte *OutputBuffer, size_t *OutputBufferLen, char *KeyRingName)
{
	PGPFileRef			*refKeyRing=NULL;
	PGPError			err;

	if (KeyRingName && KeyRingName[0]) {
		if (!(refKeyRing = pgpNewFileRefFromFullPath (KeyRingName))) {
			err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPExtractKeyBufferX (handle, UserIDKeyID, OutputBuffer,
		OutputBufferLen, refKeyRing);

error:

	if (refKeyRing)
		pgpFreeFileRef (refKeyRing);
	return err;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
