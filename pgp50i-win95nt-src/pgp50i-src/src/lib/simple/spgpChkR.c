/*
 * spgpChkR.c -- Simple PGP API Check recipient
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: spgpChkR.c,v 1.7.2.1 1997/06/07 09:51:47 mhw Exp $
 */

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"

int SPGPExport
SimplePGPCheckRecipientX (void *handle, char *Recipient, int UseUntrustedKeys,
	PGPFileRef *PublicKeyRingRef)
	{
		PgpEnv	*env; /* PGP local environment */
		RingPool	*ringpool;	/* Keyring pool */
		RingFile	*rfpub;		/* Public keyring file */
		RingSet const	*rspub;		/* Public keyring ringset */
		PgpFile	*pfpub;		/* Public keyring file handle */
		RingSet	*rsuser;		/* Ringset holding keys for userid */
		RingIterator	*riuser;		/* Iterate over rsuser */
		RingObject	*robuser;	 /* RingObj for encryption key */
		int	err;	 	/* Error variable */

		(void)handle;

	/* Initialize some null pointers to simplify error cleanup */
	rfpub = NULL;
	pfpub = NULL;
	env = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);

	/* Some pointer checks */
	if (!Recipient) {
		err = SIMPLEPGPCHECKRECIPIENT_NULLPOINTERTORECIPIENT;
		goto error;
	}
	if (!Recipient[0]) {
		err = SIMPLEPGPCHECKRECIPIENT_RECIPIENTLENGTHISZERO;
		goto error;
	}
	if (!env || !ringpool) {
		err = SIMPLEPGPCHECKRECIPIENT_NOTENOUGHMEMORYFORINPUTSTRUCTURE;
		goto error;
	}
	/* Open keyring and get ringset */
	err = spgpOpenRingfiles (env, ringpool,
		PublicKeyRingRef, NULL,
		&pfpub, &rfpub, &rspub, NULL, NULL, NULL, FALSE);
	if (err)
		goto error;

	rsuser = ringSetCreate (ringpool);

	if (!ringSetFilterSpec (rspub, rsuser, Recipient,
							PGP_PKUSE_ENCRYPT)) {
		err = SIMPLEPGP_NONEXISTENTRECIPIENTKEY;
		ringSetDestroy (rsuser);
		goto error;
	}
	ringSetFreeze (rsuser);
	riuser = ringIterCreate (rsuser);
	ringIterNextObject (riuser, 1);
	robuser = ringIterCurrentObject (riuser, 1);
	if (!spgpKeyOKToEncrypt (rspub, robuser, UseUntrustedKeys)) {
		ringObjectRelease (robuser);
		ringIterDestroy (riuser);
		ringSetDestroy (rsuser);
		err = SIMPLEPGPCHECKRECIPIENT_CANNOTUSEUNTRUSTEDKEY;
		goto error;
	}
	ringObjectRelease (robuser);
	ringIterDestroy (riuser);
	ringSetDestroy (rsuser);

error:

	if (rfpub)
		spgpRingFileClose (rfpub);
	if (pfpub)
		pgpFileClose (pfpub);
	spgpFinish (env, ringpool, err);
	return err;
}

int
SimplePGPCheckRecipient (void *handle, char *Recipient, int UseUntrustedKeys,
	char *PublicKeyRingName)
{
	PGPFileRef	*refPubRing = NULL;
	PGPError	err;

	if (PublicKeyRingName && PublicKeyRingName[0]) {
		if (!(refPubRing = pgpNewFileRefFromFullPath (PublicKeyRingName))) {
			err = KERNEL_OUT_OF_MEM;
			goto error;
		}
	}
	
	err = SimplePGPCheckRecipientX (handle, Recipient, UseUntrustedKeys,
		refPubRing);

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
