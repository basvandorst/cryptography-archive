/*
 * spgpAddK.c -- Simple PGP API Add Key
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: spgpAddK.c,v 1.12.2.1 1997/06/07 09:51:46 mhw Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>	/* For unlink() */
#endif

#include "spgp.h"
#include "spgpint.h"

#include "pgpKeyDB.h"


static int
SimplePGPAddKeyCommon (int fFilein, PGPFileRef *InputFileRef,
	byte *InputBuffer, size_t InputBufferLen, PGPFileRef *KeyRingRef)
	{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub,		/* Public keyring file */
			*rftmp;	/* Keyring file for keyadd tmpfile */
		RingSet const	*rspub,		/* Public keyring ringset */
			*rstmp;	/* Ringset for keyadd tmpfile */
		RingSet	*rsnew;	/* Updated public keyring ringset */
		PgpFile	*pfpub,		/* Public keyring file handle */
			*pftmp;	/* Tmpfile handle for added key */
		byte	*filebuf;	/* Temp buffer for key data */
		size_t	filebuflen; /* Size of filebuf */
		int	err;	 	/* Error variable */
		Boolean		 			havesem; /* Have opened our semphore interlock */

	/* Initialize some null pointers to simplify error cleanup */
	rfpub = NULL;
	pfpub = NULL;
	rsnew = NULL;
	env = NULL;

	/* Need write permission on keyrings, get interlock semaphore */
	havesem = pgpOpenSemaphore();
	if (!havesem) {
		err = SIMPLEPGPADDKEY_OTHERINSTANCERUNNING;
		goto error;
	}

	/* Create global structures */
	spgpInit (&env, &ringpool);

	/* Some pointer checks */
	if (!env || !ringpool) {
		err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}

	/* Open keyring and get ringset */
	err = spgpOpenRingfiles (env, ringpool, KeyRingRef, NULL,
		&pfpub, &rfpub, &rspub, NULL, NULL, NULL, TRUE);
	if (err)
	{
		if (err == SIMPLEPGP_KEYRINGWRITELOCKED)
			err = SIMPLEPGPADDKEY_OTHERINSTANCERUNNING;
		goto error;
	}

	/* Make a mutable ringset to receive the new keys */
	rsnew = ringSetCreate (ringpool);
	if (!rsnew) {
		err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	ringSetAddSet (rsnew, rspub);

	/* Remove any ascii armor from input file */
	if (fFilein) {
		err = spgpDearmorKeyFile (env, InputFileRef, &filebuf, &filebuflen);
		} else {
			 err = spgpDearmorKeyBuffer (env, InputBuffer, InputBufferLen,
			 	&filebuf, &filebuflen);
		}
		if (err) {
			 goto error;
		}

		/* Open keyring data */
		pftmp = pgpFileMemReadOpen (filebuf, filebuflen);
		rftmp = ringFileOpen (ringpool, pftmp, 1, &err);
		if (!rftmp) {
			 pgpMemFree (filebuf);
			 err = KERNEL_KEYRING_ADD_ERROR;
			 goto error;
		}
		rstmp = ringFileSet(rftmp);

		/* Add the new ringset */
		ringSetAddSet (rsnew, rstmp);

		/* Write out updated ringset */
		ringSetFreeze (rsnew);
	err = spgpWriteoutSet (env, KeyRingRef, rsnew, rfpub, pfpub, FALSE);
	spgpRingFileClose (rftmp);
	pgpFileClose (pftmp);
	pgpMemFree (filebuf);
	if (!err) {
		rsnew = NULL;
		rfpub = NULL;
		pfpub = NULL;
	}

error:
	if (rsnew)
		ringSetDestroy (rsnew);
	if (rfpub)
		spgpRingFileClose (rfpub);
	if (pfpub)
		pgpFileClose (pfpub);
	spgpFinish (env, ringpool, err);
	if (havesem)
		pgpCloseSemaphore ();
	return err;
}


int
SimplePGPAddKeyX (void *handle, PGPFileRef *InputFileRef,
	PGPFileRef *KeyRingRef)
{
	(void)handle;

	return SimplePGPAddKeyCommon (TRUE, InputFileRef, NULL, (size_t)0,
		KeyRingRef);
}

int
SimplePGPAddKey (void *handle, char *InputFileName, char *KeyRingName)
{
	PGPFileRef			*refInput=NULL,
						*refKeyRing=NULL;
	PGPError			err;

	if (!InputFileName) {
		return SIMPLEPGPADDKEY_NULLPOINTERTOINPUTFILENAME;
	}
	if (!InputFileName[0]) {
		return SIMPLEPGPADDKEY_INPUTFILENAMESTRINGISEMPTY;
	}

	if (!(refInput = pgpNewFileRefFromFullPath (InputFileName))) {
		err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	if (KeyRingName && KeyRingName[0]) {
		if (!(refKeyRing = pgpNewFileRefFromFullPath (KeyRingName))) {
			err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPAddKeyX (handle, refInput, refKeyRing);

error:

	if (refInput)
		pgpFreeFileRef (refInput);
	if (refKeyRing)
	pgpFreeFileRef (refKeyRing);
return err;
}



int
SimplePGPAddKeyBufferX (void *handle, byte *InputBuffer,
	size_t InputBufferLen, PGPFileRef *KeyRingRef)
{
	(void)handle;

	if (!InputBuffer) {
		return SIMPLEPGPADDKEY_NULLPOINTERTOINPUTBUFFER;
	}
	return SimplePGPAddKeyCommon (FALSE, NULL, InputBuffer, InputBufferLen,
		KeyRingRef);
}

int
SimplePGPAddKeyBuffer (void *handle, byte *InputBuffer,
	size_t InputBufferLen, char *KeyRingName)
{
	PGPFileRef			*refKeyRing=NULL;
	PGPError			err;

	if (KeyRingName && KeyRingName[0]) {
		if (!(refKeyRing = pgpNewFileRefFromFullPath (KeyRingName))) {
			err = SIMPLEPGPADDKEY_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
			goto error;
		}
	}

	err = SimplePGPAddKeyBufferX (handle, InputBuffer, InputBufferLen,
		refKeyRing);

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
