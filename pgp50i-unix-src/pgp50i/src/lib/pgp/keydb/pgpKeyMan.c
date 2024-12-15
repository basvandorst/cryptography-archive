/*
* pgpKeyMan.c -- Key Manipulation functions
*
* Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
*
* $Id: pgpKeyMan.c,v 1.61.2.6 1997/06/07 09:50:26 mhw Exp $
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpKDBint.h"
#include "pgpDebug.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpPubKey.h"
#include "pgpRndom.h"
#include "pgpSigSpec.h"
#include "pgpKeySpec.h"
#include "pgpTrstPkt.h"
#include "pgpTrust.h"
#include "pgpHash.h"
#include "pgpEnv.h"
#include "bn.h"

/* INTERNAL FUNCTIONS */

/* Internal function for certifying a key or userid. Object to be signed
	must be	in dest. Signing key should be in src. selfsig should be
set for self-certifying names. */

static PGPError
pgpCertifyObject (union RingObject *to_sign, struct RingSet *dest,
				union RingObject *signer, struct RingSet *src,
				byte sigtype, char *passphrase, Boolean selfsig)
{
struct PgpSecKey *seckey = NULL;
struct PgpSigSpec	*sigspec = NULL;
PGPError				error = PGPERR_OK;

	if (passphrase && strlen (passphrase) == 0)
		passphrase = NULL;

	if (!signer || !ringKeyIsSec (src, signer) ||
		!(ringKeyUse (src, signer) & PGP_PKUSE_SIGN))
		return PGPERR_NO_SECKEY;

  seckey = ringSecSecKey (src, signer, PGP_PKUSE_SIGN);
  if (!seckey)
		return ringSetError(src)->error;
  if (pgpSecKeyIslocked (seckey)) {
	if (!passphrase) {
		pgpSecKeyDestroy (seckey);
			return PGPERR_KEYDB_BADPASSPHRASE;
		}
	error = pgpSecKeyUnlock (seckey, pgpEnv, passphrase,
								strlen (passphrase));
		if (error != 1) {
	pgpSecKeyDestroy (seckey);
			if (error == 0)
			  error = PGPERR_KEYDB_BADPASSPHRASE;
			return error;
	}
  }
	ringKeySetAxiomatic (src, signer); /* user knows the passphrase */
		sigspec = pgpSigSpecCreate (pgpEnv, seckey, sigtype);
		if (!sigspec) {
			pgpSecKeyDestroy (seckey);
			return PGPERR_NOMEM;
		}
	if (seckey->pkAlg > PGP_PKALG_RSA + 2 &&
			sigtype == PGP_SIGTYPE_KEY_GENERIC && selfsig) {
		/* Propagate sig subpacket information */
		byte const *p;
		size_t plen;
		pgpSigSpecSetVersion (sigspec, PGPVERSION_3);
		if ((p=ringKeyFindSubpacket (signer, src, SIGSUB_PREFERRED_ALGS, 0,
				&plen, NULL, NULL, NULL, NULL, NULL)) != 0) {
			pgpSigSpecSetPrefAlgs (sigspec, p, plen);
		}
		if (ringKeyExpiration (src, signer)) {
			word32 period = ringKeyExpiration (src, signer) -
				ringKeyCreation (src, signer);
			pgpSigSpecSetKeyExpiration (sigspec, period);
		}
	}
	error = ringSignObject (dest, to_sign, sigspec, pgpRng);
	pgpSecKeyDestroy (seckey);
	pgpSigSpecDestroy (sigspec);
	return error;
}


/* Check for a 'dead' key. A dead key is revoked or expired.
	There's not much you can do with such a key. */

static Boolean
pgpKeyIsDead (PGPKey *key)
{
Boolean revoked, expired;
	
	pgpGetKeyBoolean (key, kPGPKeyPropIsRevoked, &revoked);
	pgpGetKeyBoolean (key, kPGPKeyPropIsExpired, &expired);
	return (revoked || expired);
}


/* Same for subkey... */

static Boolean
pgpSubKeyIsDead (PGPSubKey *subkey)
{
Boolean revoked, expired;
	
	pgpGetSubKeyBoolean (subkey, kPGPKeyPropIsRevoked, &revoked);
	pgpGetSubKeyBoolean (subkey, kPGPKeyPropIsExpired, &expired);
	return (revoked || expired);
}


/* Find the default private key. Get the name (or keyid) from the
	environment, and find the PGPKey object. If there is no default
	key defined in the environment, return NULL unless there is
	only one private key in the key database. */

static PGPKey *
pgpGetDefaultPrivateKeyInternal (PGPKeyDB *keyDB)
{
	union RingObject	*keyobj = NULL, *selectedkey = NULL, *secobj;
	struct RingSet		*ringset;
	struct RingIterator *iter;
	PGPKey *key = NULL;
	char const			*myname = pgpenvGetString
							(pgpEnv, PGPENV_MYNAME, NULL, NULL);
	
	ringset = (*keyDB->getRingSet) (keyDB);
	iter = ringIterCreate (ringset);
	if (!iter)
		return NULL;

	if (!myname || strlen (myname) == 0) {
		/* If only one private key, return that */
		while (ringIterNextObject (iter, 1) > 0) {
			keyobj = ringIterCurrentObject (iter, 1);
			pgpa(pgpaAssert(ringObjectType(keyobj) == RINGTYPE_KEY));
			if (ringKeyIsSec (ringset, keyobj) &&
				(ringKeyUse (ringset, keyobj) & PGP_PKUSE_SIGN)!=0) {
			 		 if (selectedkey) {	/* already found one */
			 		 selectedkey = NULL;
			 		 	goto cleanup;
			 		 }
			 		 else
			 		 	selectedkey = keyobj;	/* found first one */
			 	}
			 }
		}
		else {
		secobj = ringLatestSecret (ringset, myname, pgpGetTime(), 0);
		if (secobj) {
		ringIterSeekTo (iter, secobj);
			selectedkey = ringIterCurrentObject (iter, 1);
			/* Make sure it can sign */
			if (!(ringKeyUse (ringset, selectedkey) & PGP_PKUSE_SIGN))
			selectedkey = NULL;
		}
	}
cleanup:
	ringIterDestroy (iter);
	if (selectedkey) {
	key = pgpGetKeyByRingObject (keyDB, selectedkey);
		/* Ignore revoked or expired key */
		if (key && pgpKeyIsDead (key))
		key = NULL;
	}
	return key;
}

/* END OF INTERNAL FUNCTIONS */


/* Copy an entire key to a new ringset. The newly created ringset is
	returned. This function is necessary for two reasons:
	1. ringRaiseName requires all names to be present on the ringset to have
	any effect.
	2. to ensure a complete key (i.e. all it's sub-objects) are copied from
	a modified read-only key to a writable keyring.
*/

PGPError
pgpCopyKey (struct RingSet *src, union RingObject *obj, struct RingSet **dest)
{
	struct RingIterator *iter = NULL;
	int					level;

	if (!ringSetIsMember (src, obj))
		return PGPERR_BADPARAM;
	*dest = ringSetCreate (ringSetPool (src));
	if (!*dest)
		return PGPERR_NOMEM;
	iter = ringIterCreate (src);
	if (!iter) {
		ringSetDestroy (*dest);
		return PGPERR_NOMEM;
	}

	ringIterSeekTo (iter, obj);
	ringIterRewind (iter, 2); /* reset iterator to key object */
	/* Loop adding objects until next key (level 1), or no more keys
		(level 0) */
	while ((level = ringIterNextObjectAnywhere (iter)) > 1) {
		obj = ringIterCurrentObject (iter, level);
		ringSetAddObject (*dest, obj);
	}
	ringIterDestroy (iter);
	return PGPERR_OK;
}


/* Given a key ring object, find the corresponding PGPKey object. */

PGPKey *
pgpGetKeyByRingObject (PGPKeyDB *keyDB, union RingObject *keyobj)
{
	PGPKey				*keyptr;

	pgpAssert (ringObjectType (keyobj) == RINGTYPE_KEY);

	for (keyptr = keyDB->firstKeyInDB; keyptr; keyptr = keyptr->nextKeyInDB) {
		if (keyobj == keyptr->key)
			return keyptr;
	}
		return NULL;
}


PGPError
pgpRevokeKey (PGPKey *key, char *passphrase)
{
  PGPKeyDB			*keys = NULL;
	struct RingSet *allset = NULL, *addset = NULL;
	union RingObject *keyobj;
	PGPError			error = PGPERR_OK;
	
	pgpa(pgpaPGPKeyValid(key));
	keys =		key->keyDB;
	keyobj =	key->key;

	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;
	if (pgpKeyIsDead (key))
	return PGPERR_OK; /* no need */
	
	allset = pgpKeyDBRingSet (keys);
	if (!ringKeyIsSec (allset, keyobj))
		return PGPERR_NO_SECKEY;			/* not our key */

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;
	error = pgpCertifyObject (keyobj, addset, keyobj, allset,
							PGP_SIGTYPE_KEY_COMPROMISE, passphrase, FALSE);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	
	/* Update the KeyDB */
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);
	return error;
}


/* Check whether sufficient entropy exists to generate a key of the
   requested type with the requested number of keys. Returns
   0 if sufficient entropy exists, or a positive integer representing the
   number of bits required. */

unsigned
pgpQueryEntropyNeeded (byte pkalg, unsigned bits)
{
	long bits_needed;
	Boolean		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN,
												NULL, NULL);

	bits_needed = pgpSecKeyEntropy (pgpPkalgByNumber(pkalg), bits, fastgen) -
						pgpRandPoolEntropy ();
	return (unsigned) ((bits_needed <= 0) ? 0 : bits_needed);
}


/* Return the amount of entropy needed to create a key of the specified
   type and size. The application must call pgpRandpoolEntropy() itself
   until it has accumulated this much. */

unsigned
pgpKeyEntropyNeeded (byte pkalg, unsigned bits)
{
	Boolean		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN,
												NULL, NULL);

	return pgpSecKeyEntropy (pgpPkalgByNumber (pkalg), bits, fastgen);
}


/*	Allows the caller to add entropy to the random number pool. This will not
	normally be called - the app will call Aurora functions directly. */

PGPError
pgpAddRandomBits (byte *random_bits, unsigned num_bytes)
{
pgpRandomAddBytes (pgpRng, random_bits, num_bytes);
	return PGPERR_OK;
}


#if 0	/* [ */

static int (*generateYieldProgressFunc)(void *arg, int phase) = NULL;
static void *generateYieldArg = NULL;

static int
generateYieldFunc(void)
{
	/* Will this actually work consistently to abort from here? */
	if (generateYieldProgressFunc != NULL)
		return (*generateYieldProgressFunc)(generateYieldArg, 0);
	else
		return 0;
}

#endif	/* ] */

/* Common code for generating master keys and subkeys. *masterkey
is NULL when generating a master key, and is used to return
	the master PGPKey object. If *masterkey contains a value,
	a subkey is to be generated associated with the PGPKey object. */

static PGPError
pgpGenerateKeyInternal (PGPKeyDB *keyDB, PGPKey **masterkey,
						byte pkalg, unsigned bits,
						word16 expiration, char *name, int name_len,
						char *passphrase, char *masterpass,
						int (*progress) (void *arg, int phase),
						void *arg)
{
	struct RingSet		*allset, *addset = NULL;
	union RingObject *newobj = NULL;
	PGPError error = PGPERR_OK;
	struct PgpSecKey	*seckey = NULL, *masterseckey = NULL;
	struct PgpKeySpec *keyspec = NULL;
	long entropy_needed, entropy_available;
	Boolean genMaster = (*masterkey == NULL);
	Boolean				fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN,
												NULL, NULL);
#if 0	/* [ */
	int					(*oldBNYield)(void);

	generateYieldProgressFunc = progress;
	generateYieldArg = arg;
	oldBNYield = bnYield;
	bnYield = &generateYieldFunc;
#endif	/* ] */

	/* Check we have sufficient random bits to generate the keypair */
	entropy_needed = pgpSecKeyEntropy (pgpPkalgByNumber(pkalg), bits, fastgen);
	entropy_available = pgpRandPoolEntropy ();
	if (entropy_needed > entropy_available) {
	return PGPERR_KEYDB_NEEDMOREBITS;
		goto cleanup;
	}
	
	/* Generate the secret key */
	seckey = pgpSecKeyGenerate(pgpPkalgByNumber(pkalg), bits, fastgen, pgpRng,
								progress, arg, &error);
	if (error)
		goto cleanup;
	pgpRandomStir (pgpRng);

	/* Need to lock the SecKey with the passphrase. */
	if (passphrase && strlen (passphrase) > 0) {
		error = pgpSecKeyChangeLock (seckey, pgpEnv, pgpRng,
									passphrase, strlen (passphrase));
		if (error)
			goto cleanup;
	}

	/* Generate the keyring objects. Use keyspec defaults except for
		expiration (validity) period */
	keyspec = pgpKeySpecCreate (pgpEnv);
	if (!keyspec) {
		error = PGPERR_NOMEM;
		goto cleanup;
	}
	pgpKeySpecSetValidity (keyspec, expiration);

	allset = pgpKeyDBRingSet (keyDB);

	if (genMaster) {
	/* Generating master signing key */
	addset = ringSetCreate (ringSetPool (allset));
		if (!addset) {
		error = PGPERR_NOMEM;
			goto cleanup;
		}
	error = ringCreateKeypair (pgpEnv, seckey, keyspec, name, name_len,
								pgpRng, addset, addset);
	}
	else {
	/* Generating encryption subkey. Get the master seckey and
				unlock it */
	error = pgpCopyKey (allset, (*masterkey)->key, &addset);
		if (error)
		goto cleanup;
	masterseckey = ringSecSecKey (allset, (*masterkey)->key,
									PGP_PKUSE_SIGN);
		if (!masterseckey) {
		error = ringSetError(allset)->error;
			goto cleanup;
		}
		if (pgpSecKeyIslocked (masterseckey)) {
		if (!passphrase) {
			error = PGPERR_KEYDB_BADPASSPHRASE;
			goto cleanup;
			}
			error = pgpSecKeyUnlock (masterseckey, pgpEnv, masterpass,
									strlen (masterpass));
			if (error != 1) {
			if (error == 0)
				error = PGPERR_KEYDB_BADPASSPHRASE;
				goto cleanup;
			}
		}
	error = ringCreateSubkeypair (pgpEnv, masterseckey, seckey, keyspec,
									pgpRng, addset, addset);
	}
	pgpRandomStir (pgpRng);		/* this helps us count randomness in pool */
	if (error)
		goto cleanup;

	/* Add objects to main KeyDB. Before doing so, locate
	the master key object and return it. */
	ringSetFreeze (addset);
	if (genMaster) {
	RingIterator *iter = ringIterCreate (addset);
		if (!iter) {
		error = ringSetError(addset)->error;
			goto cleanup;
		}
		ringIterNextObject (iter, 1);
		newobj = ringIterCurrentObject (iter, 1);
		ringIterDestroy (iter);
	}
	error = pgpAddObjects (keyDB, addset);
	if (genMaster && !error)
	*masterkey = pgpGetKeyByRingObject (keyDB, newobj);

cleanup:
#if 0
	bnYield = oldBNYield;
#endif
	if (addset)
		ringSetDestroy (addset);
	if (seckey)
		pgpSecKeyDestroy (seckey);
	if (masterseckey)
	pgpSecKeyDestroy (masterseckey);
	if (keyspec)
		pgpKeySpecDestroy (keyspec);
	return error;
}


/* Generate a new master keypair. The caller must have already generated
sufficient entropy for the keys to be generated. The newly generated
keys are added to the PGPKeyDB referenced by "keyset". */

PGPError
pgpGenerateKey (PGPKeySet *keyset, byte pkalg, unsigned bits,
				word16 expiration, char *name, int name_len,
				char *passphrase, int (*progress) (void *arg, int phase),
				void *arg, PGPKey **key)
{
	PGPError error = PGPERR_OK;
	PGPKey *newkey = NULL;

	pgpa(pgpaPGPKeySetValid(keyset));

	if (bits < 512 || bits > 4096 ||
		(pkalg != PGP_PKALG_RSA && pkalg != PGP_PKALG_DSA) ||
		!name || name_len < 1 || name_len > 256)
	return PGPERR_BADPARAM;
	
	*key = NULL;
	error = pgpGenerateKeyInternal (keyset->keyDB, &newkey, pkalg, bits,
									expiration, name, name_len, passphrase,
									NULL, progress, arg);
	if (!error && key)
	*key = newkey;
	return error;
}


/* Generate a subkey pair. The passphrase is that of the master
	key. Note that the subkey is protected by the same passphrase
	as its master. */

PGPError
pgpGenerateSubKey (PGPKey *masterkey, unsigned bits,
				word16 expiration, char *passphrase, char *masterpass,
				int (*progress) (void *arg, int phase), void *arg)
{
	long pkalg = 0;

	pgpa(pgpaPGPKeyValid(masterkey));

	if (bits < 512 || bits > 4096)
	return PGPERR_BADPARAM;
	if (pgpKeyIsDead (masterkey))
	return PGPERR_BADPARAM;

	pgpGetKeyNumber (masterkey, kPGPKeyPropAlgId, &pkalg);
	if (pkalg != PGP_PKALG_DSA)
	return PGPERR_BADPARAM;

	return pgpGenerateKeyInternal (masterkey->keyDB, &masterkey,
								PGP_PKALG_ELGAMAL, bits,
								expiration, NULL, 0, passphrase,
								masterpass, progress, arg);
}


/* Disable the key. If key is not stored in a writeable KeySet, copy it
	locally. Private keys cannot be disabled. */

PGPError
pgpDisableKey (PGPKey *key)
{
	PGPKeyDB		*keys = NULL;
	struct RingSet *allset = NULL, *addset = NULL;
	union RingObject *keyobj;
	PGPError			error = PGPERR_OK;

	pgpa(pgpaPGPKeyValid(key));
	if (pgpKeyIsDead (key))
		return PGPERR_BADPARAM;
	keys =		key->keyDB;
	keyobj =	key->key;
	allset =	pgpKeyDBRingSet (keys);

	/* Axiomatic keys cannot be disabled, but plain old private
	keys can (because they may belong to someone else). */
	if (ringKeyAxiomatic (allset, keyobj))
		return PGPERR_BADPARAM;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;

	if ((error = pgpCopyKey (allset, keyobj, &addset)) != PGPERR_OK)
		return error;
	if (!ringKeyDisabled (allset, keyobj)) {
	if (!(*keys->objIsMutable) (keys, keyobj)) {
			if ((error = pgpAddObjects (keys, addset)) != PGPERR_OK)
				goto cleanup;
		}
	ringKeyDisable (allset, keyobj);
		pgpKeyDBChanged (keys, addset);
	}
cleanup:
	if (addset)
		ringSetDestroy (addset);
	return error;
}


/* Enable the key. */

PGPError
pgpEnableKey (PGPKey *key)
{
	PGPKeyDB		*keys;
struct RingSet *allset, *addset;
	union RingObject *keyobj;
	PGPError		error = PGPERR_OK;
	
	pgpa(pgpaPGPKeyValid(key));
	if (pgpKeyIsDead (key))
		return PGPERR_BADPARAM;
	keys =		key->keyDB;
	keyobj =	key->key;
	allset =	pgpKeyDBRingSet (keys);

	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;

	if (ringKeyDisabled (allset, keyobj)) {
		if ((error = pgpCopyKey (allset, keyobj, &addset)) != PGPERR_OK)
			return error;
		ringKeyEnable (allset, keyobj);
		pgpKeyDBChanged (keys, addset);
		ringSetDestroy (addset);
	}
	return PGPERR_OK;
}


/* Change the passphrase. If the new passphrase is the same as the
	old passphrase, we still unlock the key as the user may be trying to
	set the key's isAxiomatic flag. */

static PGPError
pgpChangePassphraseInternal (PGPKeyDB *keyDB, RingSet *ringset,
							RingObject *keyobj, RingObject *masterkeyobj,
							char *oldphrase, char *newphrase)
{
	unsigned long	 	validity;
	struct RingSet		*addset = NULL;
	union RingObject	*newsecobj, *oldsecobj = NULL;
	struct PgpKeySpec	 *keyspec = NULL;
	PGPError			error = PGPERR_OK;
	struct PgpSecKey	*seckey = NULL;
	PgpVersion			version;
	int					newphraselen = 0;
	Boolean				locked = 0;

	if (oldphrase && strlen (oldphrase) == 0)
		oldphrase = NULL;
	if (newphrase && (newphraselen = strlen (newphrase)) == 0)
		newphrase = NULL;

	if (!(*keyDB->isMutable) (keyDB))
		return PGPERR_KEYDB_KEYDBREADONLY;

	if (!ringKeyIsSec (ringset, keyobj))
	return PGPERR_NO_SECKEY;

	/* Find old secret object */
	{
		RingIterator *riter;
		int level;
		riter = ringIterCreate (ringset);
		if (!riter)
			return PGPERR_NOMEM;
		level = ringIterSeekTo (riter, keyobj);
		pgpAssert (level>0);
		while (ringIterNextObject (riter, level+1) == level+1) {
			oldsecobj = ringIterCurrentObject (riter, level+1);
			if (ringObjectType(oldsecobj) == RINGTYPE_SEC)
				break;
		}
		ringIterDestroy (riter);
		pgpAssert (oldsecobj);
	}

	/* Does the caller know the current passphrase? */
	seckey = ringSecSecKey (ringset, oldsecobj, 0);
	if (!seckey)
	return ringSetError(ringset)->error;
	if (pgpSecKeyIslocked (seckey)) {
		locked = 1;
	if (!oldphrase) {
		error = PGPERR_KEYDB_BADPASSPHRASE;
			goto cleanup;
		}
		error = pgpSecKeyUnlock (seckey, pgpEnv, oldphrase,
								strlen (oldphrase));
		if (error != 1) {
		if (error == 0)
			error = PGPERR_KEYDB_BADPASSPHRASE;
			goto cleanup;
		}
	}
	
	/* All done if passphrase has not changed */
	if ((!oldphrase && !newphrase) ||
		(oldphrase && locked && newphrase &&
				strcmp (oldphrase, newphrase) == 0)) {
	error = PGPERR_OK;
		goto cleanup;
	}

	error = pgpCopyKey (ringset, keyobj, &addset);
	if (error)
		goto cleanup;

	error = pgpSecKeyChangeLock (seckey, pgpEnv, pgpRng,
								newphrase, newphraselen);
	if (error)
	goto cleanup;

	keyspec = pgpKeySpecCreate (pgpEnv);
	if (!keyspec) {
	error = PGPERR_NOMEM;
		goto cleanup;
	}

	/* We need to make this keyspec just like the existing one */
	pgpKeySpecSetCreation (keyspec, ringKeyCreation (ringset, keyobj));

	/* Fix "version bug", don't change version from earlier one. */
	version = ringSecVersion (ringset, keyobj);
	pgpKeySpecSetVersion (keyspec, version);

	validity = ringKeyExpiration (ringset, keyobj);
	if (validity != 0) {
		validity -= ringKeyCreation (ringset, keyobj);
			validity /= 3600*24;
	}
	pgpKeySpecSetValidity (keyspec, (word16) validity);

	newsecobj = ringCreateSec (addset, masterkeyobj, seckey, keyspec,
							seckey->pkAlg);
	if (!newsecobj) {
	error = ringSetError(addset)->error;
	goto cleanup;
	}
	pgpKeySpecDestroy (keyspec); keyspec = NULL;
	pgpSecKeyDestroy (seckey); seckey = NULL;

	error = pgpAddObjects (keyDB, addset);

	/* This step is necessary for the RingFile to close cleanly */
	if (!error) {
		/*
		* pgpRemoveObject not appropriate since this is not an object
		* type that it knows how to deal with.
		*/
		(keyDB->remove) (keyDB, oldsecobj);
	}

cleanup:
	if (seckey)
		pgpSecKeyDestroy (seckey);
	if (addset)
		ringSetDestroy (addset);
	if (keyspec)
		pgpKeySpecDestroy (keyspec);
	return error;
}


PGPError
pgpChangePassphrase (PGPKey *key, char *oldphrase, char *newphrase)
{
	RingSet			*ringset;
	PGPError		error;

	pgpa(pgpaPGPKeyValid(key));
	ringset = pgpKeyDBRingSet (key->keyDB);
	error = pgpChangePassphraseInternal (key->keyDB, ringset, key->key, NULL,
										oldphrase, newphrase);
	if (!error)
		ringKeySetAxiomatic (ringset, key->key);
	return error;
}


PGPError
pgpChangeSubKeyPassphrase (PGPSubKey *subkey,
						char *oldphrase, char *newphrase)
{
	RingSet			*ringset;
	
	pgpa(pgpaPGPSubKeyValid(subkey));
	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	return pgpChangePassphraseInternal (subkey->key->keyDB, ringset,
									subkey->subKey,
										subkey->key->key,
										oldphrase, newphrase);
}


/* Remove a subkey */

PGPError
pgpRemoveSubKey (PGPSubKey *subkey)
{
	PGPKeyDB			*keys;
	struct RingSet *allset;
	union RingObject *subkeyobj;

	pgpa(pgpaPGPSubKeyValid(subkey));
	CHECKREMOVED(subkey);
	keys = subkey->key->keyDB;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;
	
	allset = pgpKeyDBRingSet (keys);
	subkeyobj = subkey->subKey;
	if (!(*keys->objIsMutable) (keys, subkeyobj))
		return PGPERR_KEYDB_OBJECTREADONLY;

	return pgpRemoveObject (keys, subkeyobj);
}


PGPError
pgpRevokeSubKey (PGPSubKey *subkey, char *passphrase)
{
	PGPKeyDB		 	*keys;
	struct RingSet *allset, *addset;
	union RingObject *subkeyobj, *keyobj;
	PGPError			error = PGPERR_OK;
	
	pgpa(pgpaPGPSubKeyValid(subkey));
	CHECKREMOVED(subkey);
	keys =		subkey->key->keyDB;
	subkeyobj =	subkey->subKey;
	keyobj = subkey->key->key;

	if (pgpSubKeyIsDead (subkey))
	return PGPERR_OK;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;
	
	allset = pgpKeyDBRingSet (keys);
	if (!ringKeyIsSec (allset, keyobj))
		return PGPERR_NO_SECKEY;			/* not our key */

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;

	/* Note special subkey revocation sigtype */
	error = pgpCertifyObject (subkeyobj, addset, keyobj, allset,
						PGP_SIGTYPE_KEY_SUBKEY_REVOKE, passphrase, FALSE);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	
	/* Update the KeyDB */
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);
	return error;
}


/* Remove a User ID. If the KeySet is read-only, or the UserID object
	itself is read-only, we return an error. */

PGPError
pgpRemoveUserID (PGPUserID *userid)
{
		PGPKeyDB	 			*keys;
		PGPUserID	*uidtmp;
		int	uidcount = 0;
		struct RingSet	*allset;
		union RingObject	*nameobj;

	pgpa(pgpaPGPUserIDValid(userid));
	CHECKREMOVED(userid);
	/* Cannot remove only UserID */
	for (uidtmp = (PGPUserID *) userid->key->userIDs.next;
		uidtmp != (PGPUserID *) &userid->key->userIDs;
		uidtmp = uidtmp->next) {
		if (!uidtmp->removed)
			uidcount++;
	}
	if (uidcount == 1)
		return PGPERR_BADPARAM;
	keys = userid->key->keyDB;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;
	
	allset = pgpKeyDBRingSet (keys);
	nameobj = userid->userID;
	if (!(*keys->objIsMutable) (keys, nameobj))
		return PGPERR_KEYDB_OBJECTREADONLY;

	return pgpRemoveObject (keys, nameobj);
}


/*
*	Add a new User ID to a key. User IDs cannot be added to other than the
*	user's own keys. The new User ID is added to the end of the list. To
*	make it the primary User ID, call pgpSetPrimaryUserID() below.
*/

PGPError
pgpAddUserID (PGPKey *key, char *name, int name_len, char *passphrase)
{
	PGPKeyDB		*keys;
	PGPUserID *userid;
	struct RingSet *allset, *addset;
	union RingObject *keyobj, *nameobj;
	PGPError		error;

	pgpa(pgpaPGPKeyValid(key));
	if (pgpKeyIsDead (key))
		return PGPERR_BADPARAM;
	keys = key->keyDB;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;
	
	allset = pgpKeyDBRingSet (keys);
	keyobj = key->key;

	/* Can only add User ID to our own keys */
	if (!ringKeyIsSec (allset, keyobj))
		return PGPERR_NO_SECKEY;

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;
	nameobj = ringCreateName (addset, keyobj, name, name_len);
	if (!nameobj) {
		error = ringSetError(addset)->error;
		ringSetDestroy (addset);
		return error;
	}

	/* ringCreateName will return a duplicate nameobj if
		the name already exists for this key. Check the
		list of PGPUserID objects to see if the nameobj
		is already referenced. */
	for (userid = (PGPUserID *) key->userIDs.next;
		userid != (PGPUserID *) &key->userIDs;
		userid = userid->next) {
	if (!userid->removed && userid->userID == nameobj) {
		ringSetDestroy (addset);
			return PGPERR_KEYDB_DUPLICATE_USERID;
		}
	}

	/* Must self-certify here */
	error = pgpCertifyObject (nameobj, addset, keyobj, allset,
							PGP_SIGTYPE_KEY_GENERIC, passphrase, TRUE);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);
	return error;
}



/*	Make the given User ID the primary User ID of the key */

PGPError
pgpSetPrimaryUserID (PGPUserID *userid)
{
		PGPKeyDB	 		*keys;
		PGPKey		 		*key;
		struct RingSet	*allset, *addset;
		PGPError	 		error;

	pgpa(pgpaPGPUserIDValid(userid));
	CHECKREMOVED(userid);
	key = userid->key;
	if (pgpKeyIsDead (key))
	return PGPERR_BADPARAM;
	
	keys = key->keyDB;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;
	
	allset = pgpKeyDBRingSet (keys);
	
	error = pgpCopyKey (allset, key->key, &addset);
	if (error)
		return error;
	
	/* Raise the name to the top */
ringRaiseName (addset, userid->userID);

/* Rearrange the circularly-linked list of userids */
userid->prev->next = userid->next;
userid->next->prev = userid->prev;
userid->prev = (PGPUserID *)&key->userIDs;
userid->next = (PGPUserID *)key->userIDs.next;
userid->next->prev = userid;
userid->prev->next = userid;
	
	error = pgpKeyDBChanged(keys, addset);
	ringSetDestroy (addset);
	return error;
}


/* Certify a User ID. Do not allow duplicate certification. If UserID
is already certified, but revoked, the old cert can
	be removed and the UserID then recertified. */

PGPError
pgpCertifyUserID (PGPUserID *userid, PGPKey *certifying_key, char *passphrase)
{
	PGPKeyDB		*keys;
	struct RingSet *ringset, *addset, *signerset;
	union RingObject *keyobj, *nameobj, *sigobj;
	struct RingIterator *iter;
	PGPError			error = PGPERR_OK;

	pgpa(pgpaPGPUserIDValid(userid));
	CHECKREMOVED(userid);
	if (pgpKeyIsDead (userid->key))
	return PGPERR_BADPARAM;

	if (certifying_key)
		pgpa(pgpaPGPKeyValid(certifying_key));

	if (userid->removed)
		return PGPERR_BADPARAM;
	keys = userid->key->keyDB;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;

	ringset = pgpKeyDBRingSet (keys);
	nameobj = userid->userID;

	/* If certifying key was not passed, get the default */
	if (!certifying_key) {
		certifying_key = pgpGetDefaultPrivateKeyInternal (keys);
		if (!certifying_key)
			return PGPERR_NO_SECKEY;
	}
	if (pgpKeyIsDead (certifying_key)) {
		return PGPERR_KEYDB_CERTIFYINGKEY_DEAD;
	}

	/* Get RingSet for certifying key */
	if (certifying_key->keyDB != keys)
		signerset = pgpKeyDBRingSet (certifying_key->keyDB);
	else
		signerset = ringset;
	keyobj = certifying_key->key;

	/* Check for duplicate certificate. There may be some
		old revocation certs still laying around, which we
		should ignore. */

	iter = ringIterCreate (ringset);
	if (!iter)
	return ringSetError(ringset)->error;
	ringIterSeekTo (iter, nameobj);
	while ((error = ringIterNextObject (iter, 3)) > 0) {
	sigobj = ringIterCurrentObject (iter, 3);
		if (ringSigMaker (ringset, sigobj, signerset) == keyobj &&
			ringSigType (ringset, sigobj) != PGP_SIGTYPE_KEY_UID_REVOKE) {
		error = PGPERR_KEYDB_DUPLICATE_CERT;
			break;
		}
	}
	ringIterDestroy (iter);
	if (error)
	return error;

	error = pgpCopyKey (ringset, nameobj, &addset);
	if (error)
		return error;
	error = pgpCertifyObject (nameobj, addset, keyobj, signerset,
							PGP_SIGTYPE_KEY_GENERIC, passphrase, FALSE);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);
	return error;
}


/* Given a cert, return the certifying key object. The signing key does not
	have to be in the same set as <cert>, and may be in the <allkeys> set. */

PGPError
pgpGetCertifier (PGPCert *cert, PGPKeySet *allkeys, PGPKey **certkey)
{
	char	keyID[33];
	size_t	keyIDLength = sizeof(keyID);

	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	pgpGetCertString(cert, kPGPCertPropKeyID, keyID, &keyIDLength);
	*certkey = pgpGetKeyByKeyID(allkeys, (unsigned char *) keyID, keyIDLength);
	return PGPERR_OK;
}


/* Revoke a certification. If allkeys == NULL, the certifying key
	must be in the same keyDB as the certificate. */

PGPError
pgpRevokeCert (PGPCert *cert, PGPKeySet *allkeys, char *passphrase)
{
	PGPKeyDB			*keys;
	PGPKey				*certkey;
	struct RingSet		*allset, *addset, *signerset;
	union RingObject	*sigobj, *nameobj;
	Boolean				revoked;
	PGPError			error = PGPERR_OK;

	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	keys = cert->up.userID->key->keyDB;
	sigobj = cert->cert;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;

	error = pgpGetCertBoolean (cert, kPGPCertPropIsRevoked, &revoked);
	if (error)
		return error;
	if (revoked)
		return PGPERR_OK; /* already revoked */

	/* Get certifying key and its RingSet */
	error = pgpGetCertifier (cert, allkeys, &certkey);
	if (error)
		return error;
	if (!certkey)
		return PGPERR_NO_SECKEY;
	if (pgpKeyIsDead (certkey))
		return PGPERR_KEYDB_CERTIFYINGKEY_DEAD;
	signerset = pgpKeyDBRingSet (certkey->keyDB);

	/* Get signature RingSet and its name object */
	allset = pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	nameobj = cert->up.userID->userID;

	error = pgpCopyKey (allset, nameobj, &addset);
	if (error)
		return error;
	error = pgpCertifyObject (nameobj, addset, certkey->key, signerset,
							PGP_SIGTYPE_KEY_UID_REVOKE, passphrase, FALSE);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);
	return error;
}


/* Remove a certification. If the certification was revoked, the
	revocation signature remains. This ensures that the same
	signature on someone else's keyring is properly revoked
	if this key is exported. A future certification will have
	a later creation timestamp than the revocation and will therefore
	not be affected. */

PGPError
pgpRemoveCert (PGPCert *cert)
{
	PGPKeyDB			*keys;
	union RingObject	*sigobj;

	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	keys = cert->up.userID->key->keyDB;
	sigobj = cert->cert;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;
	if (!(*keys->objIsMutable) (keys, sigobj))
		return PGPERR_KEYDB_OBJECTREADONLY;
	return pgpRemoveObject (keys, sigobj);
}


/* Given a key, return the nth (0 based) message recovery key, if one exists.
	Also return the class of the MRK, and the number of MRK's for the
	base key. Any of the return pointers may be NULL. */

PGPError
pgpGetMessageRecoveryKey (PGPKey *basekey, PGPKeySet *allkeys, unsigned nth,
PGPKey **mrkey, byte *mrclass, unsigned *numbermrks)
{
	struct RingSet		*ringset;		 /* Aurora ringset to look in */
	union RingObject *keyobj;	 	/* Aurora base key */
	union RingObject *rkey;		 	/* Aurora message recovery key */
	unsigned			nmrks;		 	/* Number MRK's available */
	byte				tclass;		 /* Class code from MRK */
	PGPError			error;		 	/* Error return from Aurora */
	byte				keyID[8];	 	/* keyid of MRK */

	pgpa(pgpaPGPKeyValid(basekey));
	ringset = pgpKeyDBRingSet (basekey->keyDB);
	keyobj = basekey->key;
	rkey = ringKeyRecoveryKey (keyobj, ringset, nth, &tclass, &nmrks, &error);
	if (!rkey) {
		/* No key found, or error */
		if (mrkey)
			*mrkey = NULL;
		if (mrclass)
			*mrclass = 0;
		if (numbermrks)
			*numbermrks = 0;
		return error;
	}
	/* Success */
	if (mrkey) {
		/* Can only look up master keys at this level */
		if (ringKeyIsSubkey(ringset, rkey))
			rkey = ringKeyMasterkey (ringset, rkey);
		ringKeyID8 (ringset, rkey, NULL, keyID);
		*mrkey = pgpGetKeyByKeyID (allkeys, keyID, sizeof(keyID));
	}
	if (mrclass)
		*mrclass = tclass;
	if (numbermrks)
		*numbermrks = nmrks;
	return PGPERR_OK;
}


/* Trust-related functions */

PGPError
pgpSetUserIDConf (PGPUserID *userid, unsigned long confidence)
{
	PGPKeyDB			*keys;
	struct RingSet		*allset = NULL, *addset = NULL;
	union RingObject *nameobj;
	PGPError			error = PGPERR_OK;

	pgpa(pgpaPGPUserIDValid(userid));
	pgpAssert (pgpTrustModel (pgpRingPool) > PGPTRUST0);
	CHECKREMOVED(userid);
	if (pgpKeyIsDead (userid->key))
		return PGPERR_BADPARAM;

	keys = userid->key->keyDB;
	allset = pgpKeyDBRingSet (keys);
	nameobj = userid->userID;
	if (ringKeyIsSec (allset, userid->key->key))
		return PGPERR_BADPARAM;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;

	error = pgpCopyKey (allset, nameobj, &addset);
	if (error)
		return error;
	if (!(*keys->objIsMutable) (keys, nameobj)) {
		error = pgpAddObjects (keys, addset);
		if (error)
			goto cleanup;
	}
	ringNameSetConfidence (allset, nameobj, (unsigned short) confidence);
	pgpKeyDBChanged (keys, addset);
cleanup:
	if (addset)
		ringSetDestroy (addset);
	return error;
}


/* Set the trust on a key. Cannot be used to set undefined or
	axiomatic trust. The key must be valid to assign trust. */

PGPError
pgpSetKeyTrust (PGPKey *key, unsigned char trust)
{
	PGPKeyDB			*keys;
	struct RingSet		*allset, *addset = NULL;
	union RingObject	*keyobj;
	PGPError			error = PGPERR_OK;
#if ONLY_TRUST_VALID_KEYS
	long validity;
#endif

	pgpa(pgpaPGPKeyValid(key));
	pgpAssert (pgpTrustModel (pgpRingPool) == PGPTRUST0);

	keys = key->keyDB;
	if (!(*keys->isMutable) (keys))
		return PGPERR_KEYDB_KEYDBREADONLY;

	allset = pgpKeyDBRingSet (keys);
	keyobj = key->key;

	if (trust <= PGP_KEYTRUST_UNDEFINED || trust > PGP_KEYTRUST_COMPLETE ||
		pgpKeyIsDead (key) || ringKeyAxiomatic (allset, keyobj))
	return PGPERR_BADPARAM;

#if ONLY_TRUST_VALID_KEYS
	/* Should not set trust on key that is not completely valid
		(who is it we are trusting?) */
	pgpGetKeyNumber (key, kPGPKeyPropValidity, &validity);
	if (validity != PGP_VALIDITY_COMPLETE)
	return PGPERR_BADPARAM;
#endif

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;
	if (!(*keys->objIsMutable) (keys, keyobj)) {
		error = pgpAddObjects (keys, addset);
		if (error)
			goto cleanup;
	}

	ringKeySetTrust (allset, keyobj, trust);
	pgpKeyDBChanged (keys, addset);
cleanup:
	if (addset)
		ringSetDestroy (addset);
	return error;
}


/* Set a secret key as the axiomatic key. If checkPassphrase == TRUE,
	the user must prove knowledge of the passphrase in order to do
	this. */

PGPError
pgpSetKeyAxiomatic (PGPKey *key, Boolean checkPassphrase, char *passphrase)
{
	Boolean	secret, axiomatic;
	struct RingSet	*allset, *addset = NULL;
	union RingObject	*keyobj;
	struct PgpSecKey	*seckey;
	PGPKeyDB	*keys;
	PGPError	error = PGPERR_OK;

	pgpa(pgpaPGPKeyValid(key));
	pgpGetKeyBoolean (key, kPGPKeyPropIsSecret, &secret);
	if (!secret)
	return PGPERR_BADPARAM;
	pgpGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &axiomatic);
	if (axiomatic)
	return PGPERR_OK;

	keys = key->keyDB;
	allset = pgpKeyDBRingSet (keys);
	keyobj = key->key;

	if (checkPassphrase) {
	/* Get the secret key and attempt to unlock it */
	seckey = ringSecSecKey (allset, keyobj, PGP_PKUSE_SIGN);
		if (!seckey)
		return ringSetError(allset)->error;
		if (pgpSecKeyIslocked (seckey)) {
		if (!passphrase) {
			pgpSecKeyDestroy (seckey);
			return PGPERR_KEYDB_BADPASSPHRASE;
			}
			error = pgpSecKeyUnlock (seckey, pgpEnv, passphrase,
									strlen (passphrase));
			pgpSecKeyDestroy (seckey);
			if (error != 1) {
				if (error == 0)
				error = PGPERR_KEYDB_BADPASSPHRASE;
				return error;
			}
		}
	}

	/* Make sure it's enabled first before setting axiomatic */
	if ((error = pgpEnableKey (key)) != PGPERR_OK)
	return error;
	if ((error = pgpCopyKey (allset, keyobj, &addset)) != PGPERR_OK)
		return error;
	if (!(*keys->objIsMutable) (keys, keyobj)) {
		if ((error = pgpAddObjects (keys, addset)) != PGPERR_OK)
			goto cleanup;
	}
	ringKeySetAxiomatic (allset, keyobj);
	pgpKeyDBChanged (keys, addset);
cleanup:
	if (addset)
	ringSetDestroy (addset);
	return error;
}


PGPError
pgpUnsetKeyAxiomatic (PGPKey *key)
{
	Boolean	axiomatic;
	struct RingSet	*allset, *addset = NULL;
		union RingObject	*keyobj;
		PGPKeyDB	*keys;
		PGPError	error = PGPERR_OK;

	pgpa(pgpaPGPKeyValid(key));
	pgpGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &axiomatic);
	if (!axiomatic)
	return PGPERR_BADPARAM;

	keys = key->keyDB;
	allset = pgpKeyDBRingSet (keys);
	keyobj = key->key;

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;
	if (!(*keys->objIsMutable) (keys, keyobj)) {
		error = pgpAddObjects (keys, addset);
		if (error)
			goto cleanup;
	}
	ringKeyResetAxiomatic (allset, keyobj);
	pgpKeyDBChanged (keys, addset);
cleanup:
	if (addset)
	ringSetDestroy (addset);
	return error;
}



/* Get property functions. Internal GetKey functions work for both
master keys and subkeys. */


static PGPError
pgpReturnPropBuffer (char const *src, char *prop,
					size_t srclen, size_t proplen)
{
PGPError result = PGPERR_OK;

	if (srclen > proplen) {
	srclen = proplen;
		result = PGPERR_KEYDB_BUFFERTOOSHORT;
	}
	if (prop && srclen > 0)
	pgpCopyMemory (src, prop, srclen);
	return result;
}


static PGPError
pgpGetKeyNumberInternal (RingSet *ringset, RingObject *keyobj,
			 		 			PGPKeyPropName propname, long *prop)
{
		byte	 		 		keyid[8];
		unsigned long			longkeyid;
		unsigned char			pkalg;
		int	i;
		byte	trust;

	switch (propname) {
	case kPGPKeyPropKeyId:
		ringKeyID8 (ringset, keyobj, NULL, keyid);
		longkeyid = 0;
		for (i = 4; i < 8; i++)
			longkeyid = (longkeyid << 8) + keyid[i];
		*prop = (long) longkeyid; /* *prop should be cast to (unsigned long) */
		break;
	case kPGPKeyPropAlgId:
		ringKeyID8 (ringset, keyobj, &pkalg, NULL);
		*prop = (long) pkalg;
		break;
	case kPGPKeyPropBits:
		*prop = (long) ringKeyBits (ringset, keyobj);
		break;
	case kPGPKeyPropTrust:
		if (pgpTrustModel (pgpRingPool) == PGPTRUST0) {
			trust = ringKeyTrust (ringset, keyobj);
			if (trust == PGP_KEYTRUST_UNDEFINED ||
				trust == PGP_KEYTRUST_UNKNOWN)
			trust = PGP_KEYTRUST_NEVER;
			*prop = (long) trust;
			break;
		}
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return PGPERR_OK;
}


PGPError
pgpGetKeyNumber (PGPKey *key, PGPKeyPropName propname, long *prop)
{
	PGPError	error = PGPERR_OK;
		PGPUserID	*userid;
		RingSet	*ringset;
		long	trustval;

	pgpa(pgpaPGPKeyValid(key));
  switch (propname) {
	case kPGPKeyPropValidity:
		if (pgpTrustModel (pgpRingPool) == PGPTRUST0) {
			*prop = PGP_VALIDITY_UNKNOWN;
			pgpIncKeyRefCount (key);
			for (userid = (PGPUserID *) key->userIDs.next;
				userid != (PGPUserID *) &key->userIDs;
				userid = userid->next) {
				if (!userid->removed) {
					pgpGetUserIDNumber (userid, kPGPUserIDPropValidity,
										&trustval);
					if (trustval > *prop)
						*prop = trustval;
				}
			}
			pgpFreeKey (key);
		}
		else {
		ringset = pgpKeyDBRingSet (key->keyDB);
			*prop = (long) ringKeyConfidence (ringset, key->key);
		}
		break;
	default:
	ringset = pgpKeyDBRingSet (key->keyDB);
		error = pgpGetKeyNumberInternal (ringset, key->key, propname, prop);
	}
	return error;
}

PGPError
pgpGetSubKeyNumber (PGPSubKey *subkey, PGPKeyPropName propname, long *prop)
{
PGPError error = PGPERR_OK;
	RingSet *ringset;

	pgpa(pgpaPGPSubKeyValid(subkey));
	CHECKREMOVED(subkey);
	switch (propname) {
	case kPGPKeyPropKeyId:
	case kPGPKeyPropAlgId:
	case kPGPKeyPropBits:
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	error = pgpGetKeyNumberInternal (ringset, subkey->subKey,
										propname, prop);
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return error;
}


static PGPError
pgpGetKeyTimeInternal (RingSet *ringset, RingObject *keyobj,
					PGPKeyPropName propname, PGPTime *prop)
{
	switch (propname) {
	case kPGPKeyPropCreation:
		*prop = ringKeyCreation (ringset, keyobj);
		break;
	case kPGPKeyPropExpiration:
		*prop = ringKeyExpiration (ringset, keyobj);
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return PGPERR_OK;
}


PGPError
pgpGetKeyTime (PGPKey *key, PGPKeyPropName propname, PGPTime *prop)
{
RingSet *ringset;
	
	pgpa(pgpaPGPKeyValid(key));
	ringset = pgpKeyDBRingSet (key->keyDB);
	return pgpGetKeyTimeInternal (ringset, key->key, propname, prop);
}


PGPError
pgpGetSubKeyTime (PGPSubKey *subkey, PGPKeyPropName propname, PGPTime *prop)
{
RingSet *ringset;
	
	pgpa(pgpaPGPSubKeyValid(subkey));
	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	return pgpGetKeyTimeInternal (ringset, subkey->subKey, propname, prop);
}


static PGPError
pgpGetKeyStringInternal (RingSet *ringset, RingObject *keyobj,
							PGPKeyPropName propname, char *prop, size_t *len)
{
		unsigned char			pkalg;
		unsigned char	buffer[20];
		size_t	length = *len;

	switch (propname) {
	case kPGPKeyPropKeyId:
		ringKeyID8 (ringset, keyobj, NULL, buffer);
		*len = 8;
		break;
	case kPGPKeyPropFingerprint:
		ringKeyID8 (ringset, keyobj, &pkalg, NULL);
		if (pkalg == 1) {
			ringKeyFingerprint16 (ringset, keyobj, buffer);
			*len = 16;
		}
		else {
			ringKeyFingerprint20 (ringset, keyobj, buffer);
			*len = 20;
		}
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return pgpReturnPropBuffer ((const char *) buffer, prop, *len, length);
}


PGPError
pgpGetKeyString (PGPKey *key, PGPKeyPropName propname, char *prop, size_t *len)
{
        RingSet *ringset;

	pgpa(pgpaPGPKeyValid(key));
	ringset = pgpKeyDBRingSet (key->keyDB);
	return pgpGetKeyStringInternal (ringset, key->key, propname, prop, len);
}


PGPError
pgpGetSubKeyString (PGPSubKey *subkey, PGPKeyPropName propname,
					char *prop, size_t *len)
{
RingSet *ringset;

	pgpa(pgpaPGPSubKeyValid(subkey));
	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	return pgpGetKeyStringInternal (ringset, subkey->subKey, propname,
									prop, len);
}

static PGPError
pgpGetKeyBooleanInternal (RingSet *ringset, RingObject *keyobj,
			 		 			PGPKeyPropName propname, Boolean *prop)
{
		struct PgpSecKey	*seckey = NULL;
		word32		 			expiration;

		switch (propname) {
	case kPGPKeyPropIsSecret:
		*prop = (ringKeyIsSec (ringset, keyobj) != 0);
		break;
	case kPGPKeyPropIsAxiomatic:
		*prop = (ringKeyAxiomatic (ringset, keyobj) != 0);
		break;
	case kPGPKeyPropIsRevoked:
		*prop = (ringKeyRevoked (ringset, keyobj) != 0);
		break;
	case kPGPKeyPropIsDisabled:
		*prop = (ringKeyDisabled (ringset, keyobj) != 0);
		break;
	case kPGPKeyPropNeedsPassphrase:
		if (!ringKeyIsSec (ringset, keyobj))
			return PGPERR_NO_SECKEY;
		seckey = ringSecSecKey (ringset, keyobj, 0);
		if (!seckey)
			return ringSetError(ringset)->error;
		*prop = (pgpSecKeyIslocked (seckey) != 0);
		pgpSecKeyDestroy (seckey);
		break;
	case kPGPKeyPropIsExpired:
		expiration = ringKeyExpiration (ringset, keyobj);
		if (expiration == 0)
			*prop = 0;
		else
			*prop = (expiration < (word32) pgpGetTime());
		break;
	case kPGPKeyPropIsNotCorrupt:
	*prop = (ringKeyError (ringset, keyobj) == 0);
		break;
	case kPGPKeyPropCanSign:
	*prop = ((ringKeyUse (ringset, keyobj) & PGP_PKUSE_SIGN) != 0);
		break;
	case kPGPKeyPropCanEncrypt:
	*prop = ((ringKeyUse (ringset, keyobj) & PGP_PKUSE_ENCRYPT) != 0);
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return PGPERR_OK;
}


PGPError
pgpGetKeyBoolean (PGPKey *key, PGPKeyPropName propname, Boolean *prop)
{
RingSet *ringset;

	pgpa(pgpaPGPKeyValid(key));
	ringset = pgpKeyDBRingSet (key->keyDB);
	return pgpGetKeyBooleanInternal (ringset, key->key, propname, prop);
}


PGPError
pgpGetSubKeyBoolean (PGPSubKey *subkey, PGPKeyPropName propname, Boolean *prop)
{
RingSet *ringset;

	pgpa(pgpaPGPSubKeyValid(subkey));
	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	if (propname == kPGPKeyPropIsAxiomatic)
	return PGPERR_KEYDB_INVALIDPROPERTY;
	return pgpGetKeyBooleanInternal (ringset, subkey->subKey, propname, prop);
}


PGPError
pgpGetUserIDNumber (PGPUserID *userid, PGPUserIDPropName propname, long *prop)
{
	struct RingSet		*ringset = NULL;
	union RingObject *nameobj = NULL;

	pgpa(pgpaPGPUserIDValid(userid));
	CHECKREMOVED(userid);
	ringset =	 pgpKeyDBRingSet (userid->key->keyDB);
	nameobj =	 userid->userID;

	switch (propname) {
	case kPGPUserIDPropValidity:
		if (pgpTrustModel (pgpRingPool) == PGPTRUST0)
		*prop = (long) ringNameTrust (ringset, nameobj);
		else
			*prop = (long) ringNameValidity (ringset, nameobj);
		break;
	case kPGPUserIDPropConfidence:
		if (pgpTrustModel (pgpRingPool) > PGPTRUST0) {
			*prop = ringNameConfidence (ringset, nameobj);
			break;
		}
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return PGPERR_OK;
}


PGPError
pgpGetUserIDString (PGPUserID *userid, PGPUserIDPropName propname,
					char *prop, size_t *len)
{
	struct RingSet		*ringset = NULL;
	union RingObject *nameobj = NULL;
	size_t length = *len;
	char const	*bufptr;

	pgpa(pgpaPGPUserIDValid(userid));
	CHECKREMOVED(userid);
	ringset =	 pgpKeyDBRingSet (userid->key->keyDB);
	nameobj =	 userid->userID;

	switch (propname) {
	case kPGPUserIDPropName:
		bufptr = ringNameName (ringset, nameobj, len);
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return pgpReturnPropBuffer (bufptr, prop, *len, length);
}

PGPError
pgpGetCertNumber (PGPCert *cert, PGPCertPropName propname, long *prop)
{
	struct RingSet		*ringset = NULL;
	union RingObject *sigobj = NULL;
	byte				keyid[8];
	unsigned long	 	longkeyid;
	int i;

	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	ringset =	 pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPCertPropKeyID:
		ringSigID8 (ringset, sigobj, NULL, keyid);
		longkeyid = 0;
		for (i = 4; i < 8; i++)
			longkeyid = (longkeyid << 8) + keyid[i];
		/* *prop should be cast to (unsigned long) below */
		*prop = (long) longkeyid;
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return PGPERR_OK;
}



PGPError
pgpGetCertString (PGPCert *cert, PGPCertPropName propname, char *prop,
				size_t *len)
{
	struct RingSet		*ringset = NULL;
	union RingObject *sigobj = NULL;
	unsigned char buffer[8];
	size_t length = *len;

	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	ringset =	 pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPCertPropKeyID:
		ringSigID8 (ringset, sigobj, NULL, buffer);
		*len = 8;
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return pgpReturnPropBuffer ((const char *) buffer, prop, *len, length);
}



PGPError
pgpGetCertTime (PGPCert *cert, PGPCertPropName propname, PGPTime *prop)
{
	struct RingSet		*ringset = NULL;
	union RingObject *sigobj = NULL;

	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	ringset =	 pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPCertPropCreation:
		*prop = ringSigTimestamp (ringset, sigobj);
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return PGPERR_OK;
}



PGPError
pgpGetCertBoolean (PGPCert *cert, PGPCertPropName propname, Boolean *prop)
{
	struct RingSet		*ringset = NULL;
	union RingObject *sigobj = NULL, *obj = NULL;
	char				keyid[8], revkeyid[8];
	struct RingIterator *iter = NULL;
	unsigned			level;

	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	ringset =	 pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPCertPropIsRevoked:
		/* Must look for a revocation signature with the same signing
			key id. The revocation sig must be the newer than the certifying
			sig to be considered. */
		*prop = 0;
		ringSigID8 (ringset, sigobj, NULL, (unsigned char *) keyid);
		iter = ringIterCreate (ringset);
		if (!iter)
			return PGPERR_NOMEM;
		ringIterSeekTo (iter, sigobj);
		level = ringIterCurrentLevel (iter);
		ringIterRewind (iter, level);
		while (ringIterNextObject (iter, level) > 0) {
			obj = ringIterCurrentObject (iter, level);
			if (ringSigType (ringset, obj) == PGP_SIGTYPE_KEY_UID_REVOKE) {
				ringSigID8 (ringset, obj, NULL, (unsigned char *) revkeyid);
				if (memcmp (keyid, revkeyid, 8) == 0 &&
					ringSigChecked (ringset, obj) &&
					ringSigTimestamp (ringset, obj) >=
						ringSigTimestamp (ringset, sigobj)) {
					*prop = 1;
					break;
				}
			}
		}
		ringIterDestroy (iter);
		break;
	case kPGPCertPropIsNotCorrupt:
		*prop = (ringSigError (ringset, sigobj) == 0);
		break;
	case kPGPCertPropIsTried:
		*prop = ringSigTried (ringset, sigobj);
		break;
	case kPGPCertPropIsVerified:
		*prop = ringSigChecked (ringset, sigobj);
		break;
	case kPGPCertPropIsMyCert:
		obj = ringSigMaker (ringset, sigobj, ringset);
		if (!obj)
			*prop = 0;
		else
			*prop = ringKeyIsSec (ringset, obj);
		break;
	default:
		return PGPERR_KEYDB_INVALIDPROPERTY;
	}
	return PGPERR_OK;
}



/* Get and Set default private key. The identification of
	the key is stored as an ascii keyid in the preferences
	repository. */


PGPKey *
pgpGetDefaultPrivateKey (PGPKeySet *keyset)
{
	pgpa(pgpaPGPKeySetValid(keyset));
	return pgpGetDefaultPrivateKeyInternal (keyset->keyDB);
}


static char const hex[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','a','b','c','d','e','f'
};

PGPError
pgpSetDefaultPrivateKey (PGPKey *key)
{
	int			i;
	size_t		len = 8;
	char		keyid[8];
	char		keyidstr[19];
	byte		digit;
	Boolean		secret = 0, cansign = 0;

	pgpa(pgpaPGPKeyValid(key));
	if (pgpKeyIsDead (key))
	return PGPERR_BADPARAM;
	/* Default key must be secret and must be able to sign */
	pgpGetKeyBoolean (key, kPGPKeyPropIsSecret, &secret);
	if (!secret)
		return PGPERR_BADPARAM;
	pgpGetKeyBoolean (key, kPGPKeyPropCanSign, &cansign);
	if (!cansign)
	return PGPERR_BADPARAM;

	/* Set the default key axiomatic (note we don't require a passphrase) */
	pgpSetKeyAxiomatic (key, FALSE, NULL);
	pgpGetKeyString (key, kPGPKeyPropKeyId, keyid, &len);
	keyidstr[0] = '0';
	keyidstr[1] = 'x';
	for (i = 0; i < 8; i++) {
		digit = (byte) keyid[i];
		keyidstr[i * 2 + 2] = hex[digit >> 4];
		keyidstr[i * 2 + 3] = hex[digit & 0xF];
	}
	keyidstr[18] = '\0';
	pgpSetPrefString (kPGPPrefUserID, keyidstr, 18);
	return PGPERR_OK;
}


PgpTrustModel
pgpGetTrustModel (void)
{
return pgpTrustModel (pgpRingPool);
}


/* UserVal functions */

PGPError
pgpSetKeyUserVal (PGPKey *key, long userVal)
{
	pgpa(pgpaPGPKeyValid(key));
	key->userVal = userVal;
	return PGPERR_OK;
}


PGPError
pgpSetUserIDUserVal (PGPUserID *userid, long userVal)
{
	pgpa(pgpaPGPUserIDValid(userid));
	userid->userVal = userVal;
	return PGPERR_OK;
}


PGPError
pgpSetSubKeyUserVal (PGPSubKey *subkey, long userVal)
{
	pgpa(pgpaPGPSubKeyValid(subkey));
	subkey->userVal = userVal;
	return PGPERR_OK;
}


PGPError
pgpSetCertUserVal (PGPCert *cert, long userVal)
{
	pgpa(pgpaPGPCertValid(cert));
	cert->userVal = userVal;
	return PGPERR_OK;
}


PGPError
pgpGetKeyUserVal (PGPKey *key, long *userVal)
{
	pgpa(pgpaPGPKeyValid(key));
	*userVal = key->userVal;
	return PGPERR_OK;
}


PGPError
pgpGetUserIDUserVal (PGPUserID *userid, long *userVal)
{
	pgpa(pgpaPGPUserIDValid(userid));
	*userVal = userid->userVal;
	return PGPERR_OK;
}

PGPError
pgpGetSubKeyUserVal (PGPSubKey *subkey, long *userVal)
{
	pgpa(pgpaPGPSubKeyValid(subkey));
	*userVal = subkey->userVal;
	return PGPERR_OK;
}

PGPError
pgpGetCertUserVal (PGPCert *cert, long *userVal)
{
	pgpa(pgpaPGPCertValid(cert));
	*userVal = cert->userVal;
	return PGPERR_OK;
}

PGPUserID *
pgpGetPrimaryUserID (PGPKey *key)
{
	PGPUserID *		userID;

	pgpa(pgpaPGPKeyValid(key));

	userID = (PGPUserID *) &key->userIDs;
	do {
		userID = userID->next;
		if (userID == (PGPUserID *)&key->userIDs)
			return NULL;
	} while (userID->removed);

	return userID;
}

PGPError
pgpGetPrimaryUserIDName(PGPKey *key, char *buf, size_t *len)
{
	PGPUserID *		userID;

	pgpa(pgpaPGPKeyValid(key));

	userID = pgpGetPrimaryUserID (key);
	if (userID == NULL)
		return PGPERR_KEYDB_CORRUPT;	/* XXX correct error? */
	return pgpGetUserIDString(userID, kPGPUserIDPropName, buf, len);
}

PGPError
pgpGetPrimaryUserIDValidity (PGPKey *key, long *validity)
{
	PGPUserID *		userID;

	pgpa(pgpaPGPKeyValid(key));

	userID = pgpGetPrimaryUserID(key);
	if (userID == NULL)
		return PGPERR_KEYDB_CORRUPT;	/* XXX correct error? */
	return pgpGetUserIDNumber(userID, kPGPUserIDPropValidity, validity);
}

/*
* XXX: This routine must be kept in sync with the hash algorithm
* selection made in pgpSigSpecCreate (in pgpSigSpec.c)
*/
PGPError
pgpGetHashAlgUsed (PGPKey *key, long *hashAlg)
{
	long			pkAlg;
	PGPError		result;

	pgpa((
		pgpaPGPKeyValid(key),
		pgpaAddrValid(hashAlg, uchar)));

	result = pgpGetKeyNumber(key, kPGPKeyPropAlgId, &pkAlg);
	if (result == PGPERR_OK)
	{
		if (pkAlg == PGP_PKALG_DSA)
			*hashAlg = PGP_HASH_SHA;
		else
			*hashAlg = pgpenvGetInt(pgpEnv, PGPENV_HASH, NULL, NULL);
	}
	return result;
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
