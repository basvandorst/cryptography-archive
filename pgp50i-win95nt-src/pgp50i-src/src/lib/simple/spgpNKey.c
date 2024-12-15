/*
* spgpNKey.c -- Simple PGP API Open/GetNext/Close keyrings
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: spgpNKey.c,v 1.6.2.1 1997/06/07 09:51:48 mhw Exp $
*/

#include <stdio.h>

#include "spgp.h"
#include "spgpint.h"
#include "pgpTimeDate.h"

/* Struct to maintain state given that we may have several client apps */
	typedef struct OpenRing {
		struct OpenRing	*next; /* Chaining pointer */
		void	*handle; /* Opaque handle from application */
		PgpFile	*pgpfile; /* Open PgpFile */
		RingFile	*ringfile; /* RingFile from pgpfile */
		RingSet const	*ringset; /* RingSet from ringfile */
		RingIterator	*ringiterator;/* RingIterator from ringset */
		RingPool	 			*ringpool;	 /* Ringpool for all sets */
		PgpEnv		 			*pgpenv;	/* Global environment settings */
		int	privring; /* Flag for private ring */
	} OpenRing;

static OpenRing *openringlist;


/* Internal routine to open either public or private keyring */
static int
spgpOpenKeyRing (void *handle, int privring, PGPFileRef *KeyRingRef)
	{
		OpenRing	*openring; /* List element for open keyring */
		RingPool	*ringpool; /* Pool for this session */
		PgpEnv	*env;	/* PGP environment */
		PgpFile	*pfile;		/* Keyring file handle */
		RingFile	*rfile;	/* Ringfile for keyring */
		RingSet const	*rset;	/* Ringset for keyring */
		RingIterator	*riter;	/* RingIterator for keyring */
		int	err;	/* Return status */
		int	eoff;	/* Error code offset */

	eoff = 0;
	if (privring)
		eoff = SIMPLEPGPOPENPRIVATEKEYRING_CANTOPENKEYRINGFILE -
				SIMPLEPGPOPENPUBLICKEYRING_CANTOPENKEYRINGFILE;

	openring = (OpenRing *)pgpMemAlloc (sizeof(OpenRing));
	if (!openring) {
		return SIMPLEPGPOPENPUBLICKEYRING_OUTOFMEMORY+eoff;
	}

	env = NULL;

	/* Create global structures */
	spgpInit (&env, &ringpool);
	/* XXX musthandle failures on these */


	/* Open keyrings and get ringsets */
	if (privring) {
		err = spgpOpenRingfiles (env, ringpool, NULL, KeyRingRef,
			NULL, NULL, NULL, &pfile, &rfile, &rset, FALSE);
	} else {
		err = spgpOpenRingfiles (env, ringpool, KeyRingRef, NULL,
			&pfile, &rfile, &rset, NULL, NULL, NULL, FALSE);
	}
	if (err) {
		pgpMemFree (openring);
		return SIMPLEPGPOPENPUBLICKEYRING_CANTOPENKEYRINGFILE+eoff;
	}

	/* Create iterator, point at first key */
	riter = ringIterCreate (rset);
	if (!riter) {
		pgpMemFree (openring);
		spgpRingFileClose (rfile);
		pgpFileClose (pfile);
		return SIMPLEPGPOPENPUBLICKEYRING_CANTOPENKEYRINGFILE+eoff;
	}
	ringIterNextObject (riter, 1);

	/* Remember info for future calls */
	openring->handle = handle;
	openring->pgpfile = pfile;
	openring->ringfile = rfile;
	openring->ringset = rset;
	openring->ringiterator = riter;
	openring->ringpool = ringpool;
	openring->pgpenv = env;
	openring->privring = privring;

	openring->next = openringlist;
	openringlist = openring;

	return 0;
}


/* Internal routine to get next key from either public or private ring */
static int
spgpGetNextKey (void *handle, int privring, char *UserID, char *KeyID,
	int *KeyLength, char *CreationDate, char *ExpirationDate,
	int *ValidityDays, int *KeyType, char *KeyTypeES, char *KeyState)
	{
		OpenRing	*openring; /* List element for open keyring */
		char const	*sname;
		size_t	lname;
		RingIterator	*riter;
		RingSet const	*rset;
		RingObject	*name,
		*key;
		word32	cdate,
			edate;
		byte	pkalg;
		byte	kid[8];
		int	keyuse;
		int	eoff;

	eoff = 0;
	if (privring)
		eoff = SIMPLEPGPGETNEXTPRIVATEKEY_NULLUSERIDSTRINGPOINTER -
				SIMPLEPGPGETNEXTPUBLICKEY_NULLUSERIDSTRINGPOINTER;

	if (!UserID)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLUSERIDSTRINGPOINTER+eoff;
	if (!KeyID)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLKEYIDSTRINGPOINTER+eoff;
	if (!KeyLength)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLKEYLENGTHPOINTER+eoff;
	if (!CreationDate)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLCREATIONDATEPOINTER+eoff;
	if (!ExpirationDate)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLEXPIRATIONDATEPOINTER+eoff;
	if (!ValidityDays)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLVALIDDAYSPOINTER+eoff;
	if (!KeyType)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLKEYTYPEPOINTER+eoff;
	if (!KeyTypeES)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLKEYTYPEESPOINTER+eoff;
	if (!KeyState)
		return SIMPLEPGPGETNEXTPUBLICKEY_NULLKEYSTATEPOINTER+eoff;

	for (openring=openringlist; openring; openring=openring->next)
		if (openring->handle==handle && openring->privring==privring)
			break;

	if (!openring) {
		return SIMPLEPGPGETNEXTPUBLICKEY_BADHANDLE+eoff;
	}

	riter = openring->ringiterator;
	rset = openring->ringset;

	/* Scan for next name on keyring */
	for ( ; ; ) {
		if (ringIterNextObject (riter, 2) < 2) {
			if (ringIterNextObject (riter, 1) < 1) {
				return SIMPLEPGPGETNEXTPUBLICKEY_EOF;
			}
		} else {
			name = ringIterCurrentObject (riter, 2);
			if (ringObjectType(name) == RINGTYPE_NAME)
				break;
		}
	}

	key = ringIterCurrentObject (riter, 1);

	sname = ringNameName (rset, name, &lname);
	memcpy (UserID, sname, lname);
	UserID[lname] = '\0';

	KeyID[0] = '0';
	KeyID[1] = 'x';
	ringKeyID8 (rset, key, &pkalg, kid);
	spgpKeyIDText8 (KeyID+2, kid);

	*KeyLength = ringKeyBits (rset, key);

	cdate = ringKeyCreation (rset, key);
	spgpDateText10 (CreationDate, cdate);

	edate = ringKeyExpiration (rset, key);
	spgpDateText10 (ExpirationDate, edate);

	*ValidityDays = edate ? (int)((edate - cdate) / (3600L*24)) : 0;

	*KeyType = (pkalg==PGP_PKALG_RSA) ? KEYS_OLD :
	(pkalg==PGP_PKALG_RSA_ENC) ? KEYS_ENCR :
	(pkalg==PGP_PKALG_RSA_SIG) ? KEYS_SIGN : 0;

	keyuse = ringKeyUse (rset, key);

	if (keyuse == PGP_PKUSE_SIGN_ENCRYPT)
		strcpy (KeyTypeES, "ES");
	else if (keyuse == PGP_PKUSE_ENCRYPT)
		strcpy (KeyTypeES, "E");
	else
		strcpy (KeyTypeES, "S");

	if (ringKeyRevoked (rset, key))
		strcpy (KeyState, "rev");
	else if (edate && (word32)pgpGetTime() > edate)
		strcpy (KeyState, "exp");
	else if (ringKeyDisabled (rset, key))
		strcpy (KeyState, "dis");
	else if (edate)
		spgpExpText3 (KeyState, (int)((edate - pgpGetTime()) / (3600L*24)));
	else
		strcpy (KeyState, "   ");

	return 0;
}


/* Internal routine to close either private or public ring */
static int
spgpCloseKeyRing (void *handle, int privring)
	{
		OpenRing	*openring, /* List element for open keyring */
			**popenring; /* Pointer to next of prev element */
		RingPool	*ringpool; /* Global ring pool for this session */
		PgpEnv		 			*env;		/* Global environment with settings */
		int	eoff;

	eoff = 0;
	if (privring)
		eoff = SIMPLEPGPCLOSEPRIVATEKEYRING_BADHANDLE -
				SIMPLEPGPCLOSEPUBLICKEYRING_BADHANDLE;

	for (popenring=&openringlist; *popenring; popenring=&(*popenring)->next)
		if ((*popenring)->handle==handle && (*popenring)->privring==privring)
			break;

	if (!*popenring) {
		return SIMPLEPGPCLOSEPUBLICKEYRING_BADHANDLE+eoff;
	}

	openring = *popenring;
	*popenring = openring->next;

	ringpool = openring->ringpool;
	env = openring->pgpenv;

	ringIterDestroy (openring->ringiterator);
	spgpRingFileClose (openring->ringfile);
	pgpFileClose (openring->pgpfile);
	memset (openring, 0, sizeof (*openring));
	pgpMemFree (openring);

	spgpFinish (env, ringpool, (int)handle);

	return 0;
}

/* Entry points for "public keyring" versions */

int
SimplePGPOpenPublicKeyRing (void *handle, char *PublicKeyRingName)
{
	PGPFileRef	*refPubRing = NULL;
	PGPError	err;

	if (PublicKeyRingName && PublicKeyRingName[0]) {
		if (!(refPubRing = pgpNewFileRefFromFullPath (PublicKeyRingName))) {
			err = KERNEL_OUT_OF_MEM;
			goto error;
		}
	}
	
	err = SimplePGPOpenPublicKeyRingX (handle, refPubRing);

error:

	if (refPubRing)
		pgpFreeFileRef (refPubRing);
	return err;
}

int
SimplePGPOpenPublicKeyRingX (void *handle, PGPFileRef *PublicKeyRingRef)
{
	return spgpOpenKeyRing (handle, 0, PublicKeyRingRef);
}

int
SimplePGPGetNextPublicKey (void *handle, char *UserID, char *KeyID,
	int *KeyLength, char *CreationDate, char *ExpirationDate,
	int *ValidityDays, int *KeyType, char *KeyTypeES, char *KeyState)
{
	return spgpGetNextKey (handle, 0, UserID, KeyID, KeyLength,
		CreationDate, ExpirationDate, ValidityDays,
		KeyType, KeyTypeES, KeyState);
}

int
SimplePGPClosePublicKeyRing (void *handle)
{
	return spgpCloseKeyRing (handle, 0);
}

/* Entry points for "private keyring" versions */

int
SimplePGPOpenPrivateKeyRing (void *handle, char *PrivateKeyRingName)
{
	PGPFileRef	*refPrivRing = NULL;
	PGPError	err;

	if (PrivateKeyRingName && PrivateKeyRingName[0]) {
		if (!(refPrivRing = pgpNewFileRefFromFullPath (PrivateKeyRingName))) {
			err = KERNEL_OUT_OF_MEM;
			goto error;
		}
	}
	
	err = SimplePGPOpenPrivateKeyRingX (handle, refPrivRing);

error:

	if (refPrivRing)
		pgpFreeFileRef (refPrivRing);
	return err;
}

int
SimplePGPOpenPrivateKeyRingX (void *handle, PGPFileRef *PrivateKeyRingRef)
{
	return spgpOpenKeyRing (handle, 1, PrivateKeyRingRef);
}

int
SimplePGPGetNextPrivateKey (void *handle, char *UserID, char *KeyID,
	int *KeyLength, char *CreationDate, char *ExpirationDate,
	int *ValidityDays, int *KeyType, char *KeyTypeES, char *KeyState)
{
	return spgpGetNextKey (handle, 1, UserID, KeyID, KeyLength,
		CreationDate, ExpirationDate, ValidityDays,
		KeyType, KeyTypeES, KeyState);
}

int
SimplePGPClosePrivateKeyRing (void *handle)
{
	return spgpCloseKeyRing (handle, 1);
}


/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
