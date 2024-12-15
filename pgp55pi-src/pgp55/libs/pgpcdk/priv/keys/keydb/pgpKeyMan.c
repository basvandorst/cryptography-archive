/*____________________________________________________________________________
	Copyright (C) 1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	

	$Id: pgpKeyMan.c,v 1.132.4.1 1997/11/11 23:32:40 hal Exp $
____________________________________________________________________________*/
#include <string.h>

#include "pgpConfig.h"

#include "pgpContext.h"
#include "pgpEncodePriv.h"
#include "pgpEventPriv.h"
#include "pgpKDBInt.h"
#include "pgpDebug.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpPubKey.h"
#include "pgpRandomX9_17.h"
#include "pgpRandomPool.h"
#include "pgpSigSpec.h"
#include "pgpKeySpec.h"
#include "pgpTrstPkt.h"
#include "pgpTrust.h"
#include "pgpHash.h"
#include "pgpEnv.h"
#include "pgpSDKPrefs.h"
#include "bn.h"
#include "pgpRegExp.h"
#include "pgpRngPub.h"
#include "pgpRngMnt.h"
#include "pgpTimeDate.h"
#include "pgpKeyIDPriv.h"
#include "pgpUtilitiesPriv.h"


#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

/*  INTERNAL FUNCTIONS */

/*  Internal function for certifying a key or userid.  Object to be signed 
	must be	in dest.  Signing key should be in src.  selfsig should be
    set for self-certifying names. */

#define SIG_EXPORTABLE			TRUE
#define SIG_NON_EXPORTABLE		FALSE

static PGPError
pgpCertifyObject (PGPContextRef context, union RingObject *to_sign,
				  RingSet *dest, union RingObject *signer, RingSet const *src, 
				  PGPByte sigtype, char const *passphrase,
				  PGPSize passphraseLength, PGPBoolean selfsig,
				  PGPBoolean exportable, PGPUInt32 sigExpiration,
				  PGPByte trustDepth, PGPByte trustValue,
				  char const *sRegExp)
{
    PGPSecKey		*seckey = NULL;
    PGPSigSpec		*sigspec = NULL;
	PGPEnv			*pgpEnv;
	PGPRandomContext *pgpRng;
    PGPError		error = kPGPError_NoErr;

	pgpEnv = pgpContextGetEnvironment( context );
	pgpRng = pgpContextGetX9_17RandomContext( context );

	/* Error if not enough entropy for a safe signature */
	if( ! PGPGlobalRandomPoolHasMinimumEntropy() )
		return kPGPError_OutOfEntropy;
	
	if (IsntNull(passphrase) && passphraseLength == 0)
		passphrase = NULL;

	if (!signer || !ringKeyIsSec (src, signer) ||
		!(ringKeyUse (src, signer) & PGP_PKUSE_SIGN))
	    return kPGPError_SecretKeyNotFound;

    seckey = ringSecSecKey (src, signer, PGP_PKUSE_SIGN);
    if (!seckey)
	    return ringSetError(src)->error;
    if (pgpSecKeyIslocked (seckey)) {
	    if (!passphrase) {
		    pgpSecKeyDestroy (seckey);
			return kPGPError_BadPassphrase;
		}
	    error = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, passphrase, 
								 passphraseLength);
		if (error != 1)
		{
	        pgpSecKeyDestroy (seckey);
			if (error == 0)
			    error = kPGPError_BadPassphrase;
			return error;
	    }
    }
	ringKeySetAxiomatic (src, signer);  /* user knows the passphrase */
    sigspec = pgpSigSpecCreate (pgpEnv, seckey, sigtype);
    if (!sigspec) {
	    pgpSecKeyDestroy (seckey);
	    return kPGPError_OutOfMemory;
    }
	if (seckey->pkAlg > kPGPPublicKeyAlgorithm_RSA + 2 &&
			sigtype == PGP_SIGTYPE_KEY_GENERIC && selfsig) {
		/* Propagate sig subpacket information */
		PGPByte const *p;
		PGPSize plen;
		pgpSigSpecSetVersion (sigspec, PGPVERSION_3);
		if ((p=ringKeyFindSubpacket (signer, src, SIGSUB_PREFERRED_ALGS, 0,
				 &plen, NULL, NULL, NULL, NULL, NULL)) != 0) {
			pgpSigSpecSetPrefAlgs (sigspec, p, plen);
		}
		if (ringKeyExpiration (src, signer)) {
			PGPUInt32 period = ringKeyExpiration (src, signer) -
				ringKeyCreation (src, signer);
			pgpSigSpecSetKeyExpiration (sigspec, period);
		}
	}
	pgpSigSpecSetExportable (sigspec, exportable);
	pgpSigSpecSetSigExpiration (sigspec, sigExpiration);
	pgpSigSpecSetTrustLevel (sigspec, trustDepth);
	if( IsntNull( sRegExp ) )
		pgpSigSpecSetRegExp (sigspec, sRegExp);

	/* Ignore trustValue for ordinary level 0 signatures */
	if (trustDepth != 0) {
		/* Convert trust value to extern format */
		if (trustValue != 0)
			trustValue = ringTrustOldToExtern(ringSetPool(dest), trustValue);
		/* Note that setting nonzero trustvalue forces V3 sigs */
		pgpSigSpecSetTrustValue (sigspec, trustValue);
	}
	pgpRng = pgpContextGetX9_17RandomContext( context );

	/* Due to a bug in 5.0, all sigs directly on keys must be version 2_6.
	 * However the only signatures 5.0 handles directly on keys are key
	 * revocations.
	 */
	if (ringObjectType( to_sign ) == RINGTYPE_KEY &&
			!ringKeyIsSubkey( dest, to_sign )  &&
			sigtype == PGP_SIGTYPE_KEY_COMPROMISE ) {
		pgpSigSpecSetVersion( sigspec, PGPVERSION_2_6 );
	}

	/* Do the signature at the Aurora level */
	error = ringSignObject (dest, to_sign, sigspec, pgpRng);
	pgpSecKeyDestroy (seckey);
	pgpSigSpecDestroy (sigspec);

	return error;
}


/*  Check for a 'dead' key.  A dead key is revoked or expired. 
	There's not much you can do with such a key. */

	static PGPError
pgpKeyDeadCheck( PGPKeyRef	key)
{
    PGPBoolean	revoked, expired;
    PGPError	err;
	
	err	= PGPGetKeyBoolean (key, kPGPKeyPropIsRevoked, &revoked);
	if ( IsntPGPError( err ) && revoked )
		err	= kPGPError_KeyRevoked;
	
	if ( IsntPGPError( err ) )
	{
		err	= PGPGetKeyBoolean (key, kPGPKeyPropIsExpired, &expired);
		if ( IsntPGPError( err ) && expired )
			err	= kPGPError_KeyExpired;
	}
	
	return ( err );
}


/* Same for subkey... */

static PGPBoolean
pgpSubKeyIsDead (PGPSubKeyRef subkey)
{
    PGPBoolean   revoked, expired;
	
	PGPGetSubKeyBoolean (subkey, kPGPKeyPropIsRevoked, &revoked);
	PGPGetSubKeyBoolean (subkey, kPGPKeyPropIsExpired, &expired);
	return (revoked || expired);
}


/*  Find the default private key.  Get the name (or keyid) from the 
	environment, and find the PGPKey object.  If there is no default 
	key defined in the environment, return NULL unless there is 
	only one private key in the key database.
	
	The refCount on the key is incremented by this routine.
	*/

	static PGPError
pgpGetDefaultPrivateKeyInternal(
	PGPKeyDBRef	keyDB,
	PGPKey **	outKey)
{
	union RingObject	*keyobj = NULL, *selectedkey = NULL, *secobj;
	RingSet const		*ringset;
	RingIterator *		iter;
	PGPEnv		*		pgpEnv;
	PGPKey              *key		= NULL;
	char const *		myname		= NULL;
	PGPError			err			= kPGPError_NoErr;
	PGPByte *			keyIDData	= NULL;
	void *				vKeyIDData	= NULL;
	PGPSize				keyIDSize	= 0;
	PGPContextRef		context	= pgpGetKeyDBContext( keyDB );
	
	PGPValidatePtr( outKey );
	*outKey	= NULL;
	
	err	= PGPsdkPrefGetData( context, kPGPsdkPref_DefaultKeyID,
			&vKeyIDData, &keyIDSize );
	keyIDData = (PGPByte *)vKeyIDData;
	if ( IsntPGPError( err ) )
	{
		PGPKeyID		keyID;
		
		err	= PGPImportKeyID( keyIDData, &keyID );
		if ( IsntPGPError( err ) )
		{
			PGPKeySetRef kset = pgpKeyDBRootSet( keyDB );
			err	= PGPGetKeyByKeyID ( kset, &keyID,
									 kPGPPublicKeyAlgorithm_Invalid, &key);
			PGPFreeKeySet( kset );
		}
		
		/* we used public API call; must free using PGPFreeData() */
		PGPFreeData( keyIDData );
		keyIDData	= NULL;
		
		if ( IsntPGPError( err ) )
		{
			*outKey	= key;
			return( err );
		}
	}
	err	= kPGPError_NoErr;	/* ok if it failed */

	pgpEnv = pgpContextGetEnvironment( keyDB->context );
	myname	= pgpenvGetCString(pgpEnv, PGPENV_MYNAME, NULL);
	
	ringset	= pgpKeyDBRingSet( keyDB );
	iter	= ringIterCreate (ringset);
	if ( IsNull( iter ) )
		return kPGPError_OutOfMemory;

	if ( IsNull( myname ) || strlen (myname) == 0)
	{
		/*  If only one private key, return that */
		while (ringIterNextObject (iter, 1) > 0)
		{
			keyobj = ringIterCurrentObject (iter, 1);
			pgpa(pgpaAssert(ringObjectType(keyobj) == RINGTYPE_KEY));
			if (ringKeyIsSec (ringset, keyobj) &&
				(ringKeyUse (ringset, keyobj) & PGP_PKUSE_SIGN)!=0)
			{
				if (selectedkey)
				{            /* already found one */
				    selectedkey = NULL;
				}
				else
					selectedkey = keyobj;     /* found first one */
			}
		}
	}
	else
	{
		secobj = ringLatestSecret (ringset, myname, PGPGetTime(), 0);
		if (secobj)
		{
		    ringIterSeekTo (iter, secobj);
			selectedkey = ringIterCurrentObject (iter, 1);
			/*  Make sure it can sign */
			if (!(ringKeyUse (ringset, selectedkey) & PGP_PKUSE_SIGN))
			    selectedkey = NULL;
		}
	}
	
	ringIterDestroy (iter);
	if ( IsntNull( selectedkey ) )
	{
	    key = pgpGetKeyByRingObject (keyDB, selectedkey);
		/*  Ignore revoked or expired key */
		if (key && IsPGPError( pgpKeyDeadCheck(key) ) )
		    key = NULL;
		if (key != NULL)
		{
			PGPSetDefaultPrivateKey(key);
		}
	}
	
	if ( IsNull( key ) && IsntPGPError( err ) )
	{
		err	= kPGPError_ItemNotFound;
	}
	
	if ( IsntPGPError( err ) )
	{
		pgpIncKeyRefCount( key );
	}
	
	*outKey	= key;
	return err;
}

/*  END OF INTERNAL FUNCTIONS */


/*  Copy an entire key to a new ringset.  The newly created ringset is
	returned.  This function is necessary for two reasons:
    1. ringRaiseName requires all names to be present on the ringset to have 
	   any effect. 
	2. to ensure a complete key (i.e. all it's sub-objects) are copied from 
	   a modified read-only key to a writable keyring. 
*/

PGPError
pgpCopyKey (RingSet const *src, union RingObject *obj, RingSet **dest)
{
	RingIterator *iter = NULL;
	int					 level;

	if (!ringSetIsMember (src, obj))
		return kPGPError_BadParams;
	*dest = ringSetCreate (ringSetPool (src));
	if (!*dest)
		return kPGPError_OutOfMemory;
	iter = ringIterCreate (src);
	if (!iter) {
		ringSetDestroy (*dest);
		return kPGPError_OutOfMemory;
	}

	ringIterSeekTo (iter, obj);
	ringIterRewind (iter, 2);   /* reset iterator to key object */
	/*  Loop adding objects until next key (level 1), or no more keys 
		(level 0) */
	while ((level = ringIterNextObjectAnywhere (iter)) > 1) {
		obj = ringIterCurrentObject (iter, level);
		ringSetAddObject (*dest, obj);
	}
	ringIterDestroy (iter);
	return kPGPError_NoErr;
}


/*  Given a key ring object, find the corresponding PGPKey object. */

PGPKey *
pgpGetKeyByRingObject (PGPKeyDBRef keyDB, union RingObject *keyobj)
{
	PGPKeyRef	keyptr;

	pgpAssert (ringObjectType (keyobj) == RINGTYPE_KEY);

	for (keyptr = keyDB->firstKeyInDB; keyptr; keyptr = keyptr->nextKeyInDB) {
		if (keyobj == keyptr->key)
			return keyptr;
	}
	return NULL;
}


static PGPError
sRevokeKey (
	PGPContextRef		context,
	PGPKeyRef			key,
	char const *		passphrase,
	PGPSize				passphraseLength,
	PGPKeyRef			signkey
	)
{
    PGPKeyDBRef			 keys = NULL;
	RingSet	const *		allset = NULL;
	RingSet *			addset = NULL;
	union RingObject    *keyobj;
	union RingObject	*signkeyobj = NULL;
	PGPError			 error = kPGPError_NoErr;
	
	keys =		key->keyDB;
	keyobj =	key->key;

	if ( !keys->objIsMutable( keys, keyobj ) )
		return kPGPError_ItemIsReadOnly;
	if ( IsPGPError( pgpKeyDeadCheck(key) ) )
	   return kPGPError_NoErr;	/* no need */
	
	allset = pgpKeyDBRingSet (keys);

	/* If revoking someone else's key, they must have authorized us */
	if (IsntNull (signkey) ) {
		signkeyobj = signkey->key;
		if (!ringKeyIsRevocationKey (keyobj, allset, signkeyobj) )
			return kPGPError_SecretKeyNotFound;
	} else {
		signkeyobj = keyobj;
	}

	if (!ringKeyIsSec (allset, signkeyobj))
		return kPGPError_SecretKeyNotFound;			/* not our key */

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;
	error = pgpCertifyObject (context, keyobj, addset, signkeyobj, allset, 
							 PGP_SIGTYPE_KEY_COMPROMISE, passphrase,
							 passphraseLength, FALSE,
							 SIG_EXPORTABLE, kPGPExpirationTime_Never, 0, 0,
							 NULL);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}

	/*  Update the KeyDB */
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);

	/* Calculate trust changes as a result */
	if( error == kPGPError_NoErr )
		(void)pgpPropagateTrustKeyDB (keys);

	return error;
}
 

static const PGPOptionType revkeyOptionSet[] = {
#if PGP_THIRDPARTYREVOCATION
	kPGPOptionType_SignWithKey,
#endif
	kPGPOptionType_Passphrase
};

PGPError
pgpRevokeKeyInternal(
	PGPKeyRef			key,
	PGPOptionListRef	optionList )
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPKeyRef			signkey = NULL;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );

	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
						revkeyOptionSet, elemsof( revkeyOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_Passphrase, FALSE,
						"%p%l", &passphrase, &passphraseLength ) ) )
		goto error;

#if PGP_THIRDPARTYREVOCATION
	/* Used for revoking someone else's key */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_SignWithKey, FALSE,
						"%p", &signkey ) ) )
		goto error;
#endif

	err = sRevokeKey( context, key, passphrase, passphraseLength, signkey );
error:
	return err;
}


static const PGPOptionType keyentOptionSet[] = {
	kPGPOptionType_KeyGenParams,
	kPGPOptionType_KeyGenFast
};

/* Return the amount of entropy needed to create a key of the specified
   type and size.  The application must call pgpRandpoolEntropy() itself
   until it has accumulated this much. */

PGPUInt32
pgpKeyEntropyNeededInternal(
	PGPContextRef	context,
	PGPOptionListRef	optionList
	)
{
	PGPEnv				*pgpEnv;
	PGPUInt32			fastgen;
	PGPBoolean			fastgenop;
	PGPUInt32			pkalg;
	PGPUInt32			bits;
	PGPError			err = kPGPError_NoErr;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
						keyentOptionSet, elemsof( keyentOptionSet ) ) ) )
		return err;

	pgpEnv = pgpContextGetEnvironment( context );
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_KeyGenParams, TRUE,
						"%d%d", &pkalg, &bits ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_KeyGenFast, FALSE,
						"%b%d", &fastgenop, &fastgen ) ) )
		goto error;
	if( !fastgenop ) {
		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN, NULL, NULL);
	}
	fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN, NULL, NULL);

	return pgpSecKeyEntropy (pgpPkalgByNumber ((PGPByte)pkalg), bits,
							(PGPBoolean)fastgen);

	/* Should not have an error unless bad parameters */
error:
	pgpAssert(0);
	return ~(PGPUInt32)0;
}


/* Internal function for passphraseIsValid */
	static PGPError
pgpPassphraseIsValidInternal(
	PGPKeyRef		key,
	const char *	passphrase,
	PGPBoolean *	isValid)
{
	PGPSize			passphraseLength;
	RingObject *	keyobj;
	PGPKeyDBRef		keys;
	RingSet const *	ringset;
	PGPContextRef	context;
	PGPEnv *		pgpEnv;
	PGPSecKey *		seckey;
	
	PGPError		err = kPGPError_NoErr;

	PGPValidateKey( key );
	PGPValidateParam( passphrase );
	PGPValidateParam( isValid );

	/* Default return value */
	*isValid = FALSE;

	passphraseLength = strlen( passphrase );

	/* Dig stuff out of key structure */
	keyobj =	key->key;
	keys =		key->keyDB;
	ringset =	pgpKeyDBRingSet (keys);
	context =	keys->context;
	pgpEnv =	pgpContextGetEnvironment( context );

	seckey = ringSecSecKey (ringset, keyobj, 0);

	/* If not a secret key, just return */
	if( !seckey )
		return err;

	/* Returns 1 on success, 0 on failure, else error */
	err = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, passphrase, 
									passphraseLength);
	pgpSecKeyDestroy( seckey );
	if (err == (PGPError)1) {
		*isValid = TRUE;
		err = kPGPError_NoErr;
	}
	return err;
}

	

/* Return TRUE if passphrase unlocks the key */
	PGPBoolean
PGPPassphraseIsValid( PGPKeyRef key, const char *passphrase )
{
	PGPBoolean rslt;

	if ( (! pgpKeyIsValid( key )) || IsNull( passphrase ) )
		return( FALSE );
		
	if( IsPGPError( pgpPassphraseIsValidInternal( key, passphrase, &rslt ) ) )
		return FALSE;

	return rslt;
}


	


/*____________________________________________________________________________
	Key Generation
____________________________________________________________________________*/


/*
 * Callback impedence matching, convert from internal state to callback
 * state.
 */
typedef struct PGPKeyGenProgressState {
	PGPContextRef			context;
	PGPEventHandlerProcPtr	progress;
	PGPUserValue			userValue;
} PGPKeyGenProgressState;
	
static int								/* Return < 0 to abort run */
genProgress(
	void *arg,
	int c
	)
{
	PGPKeyGenProgressState	*s = (PGPKeyGenProgressState *)arg;
	PGPError				err = kPGPError_NoErr;
	PGPOptionListRef		newOptionList = NULL;

	if (IsntNull (s->progress)) {
		err = pgpEventKeyGen (s->context, &newOptionList,
							 s->progress, s->userValue, (PGPUInt32)c);
		if (IsntNull (newOptionList))
			pgpFreeOptionList (newOptionList);
	}
	return err;
}



/*  Common code for generating master keys and subkeys. *masterkey
    is NULL when generating a master key, and is used to return
	the master PGPKey object.  If *masterkey contains a value,
	a subkey is to be generated associated with the PGPKey object. */

	static PGPError
pgpDoGenerateKey (
	PGPKeyDB *		keyDB,
	PGPKey **		masterkey,
	PGPSubKey **	newsubkey,
	PGPByte			pkalg,
	unsigned		bits, 
	PGPUInt16		expirationDays,
	char const *	name,
	int				name_len, 
	char const *	passphrase,
	PGPSize			passphraseLength,
	char const *	masterpass, 
	PGPSize			masterpassLength,
	PGPEventHandlerProcPtr progress,
	PGPUserValue	userValue,
	PGPBoolean		fastgen,
	RingSet const *	adkset,
	PGPByte			adkclass,
	RingSet const *	rakset,
	PGPByte			rakclass,
	PGPCipherAlgorithm const * prefalg,
	PGPSize			prefalgLength)
{
	RingSet const			*allset;
	RingSet 				*addset = NULL;
	union RingObject    	*newobj = NULL;
	PGPError	          	error = kPGPError_NoErr;
	PGPSecKey				*seckey = NULL, *masterseckey = NULL;
	PGPKeySpec				*keyspec = NULL;
	long             		entropy_needed, entropy_available;
	PGPBoolean              genMaster = (*masterkey == NULL);
	PGPEnv					*pgpEnv;
	PGPRandomContext		*pgpRng;
	PGPKeyGenProgressState	progressState;
	PGPContextRef			context	= pgpGetKeyDBContext( keyDB );
	PGPByte					*prefalgByte;
	PGPUInt32				i;

	if ( !pgpKeyDBIsMutable( keyDB ) )
		return kPGPError_ItemIsReadOnly;

	pgpEnv = pgpContextGetEnvironment( keyDB->context );
	/* Check we have sufficient random bits to generate the keypair */
	entropy_needed = pgpSecKeyEntropy (pgpPkalgByNumber(pkalg), bits, fastgen);
	entropy_available = PGPGlobalRandomPoolGetEntropy ( );
	if (entropy_needed > entropy_available) {
	   return kPGPError_OutOfEntropy;
		goto cleanup;
	}
	
	/* Generate the secret key */
	progressState.progress = progress;
	progressState.userValue = userValue;
	progressState.context = keyDB->context;
	pgpRng = pgpContextGetX9_17RandomContext( keyDB->context );
	seckey = pgpSecKeyGenerate( context,
				pgpPkalgByNumber(pkalg), bits, fastgen, pgpRng, 
				genProgress, &progressState, &error);
	if (error)
		goto cleanup;
	pgpRandomStir (pgpRng);

	/* Need to lock the SecKey with the passphrase.  */
	if (passphrase && passphraseLength > 0) {
		error = (PGPError)pgpSecKeyChangeLock (seckey, pgpEnv, pgpRng, 
									passphrase, passphraseLength);
		if (error)
			goto cleanup;
	}

	/*  Generate the keyring objects.  Use keyspec defaults except for 
		expiration (validity) period */
	keyspec = pgpKeySpecCreate (pgpEnv);
	if (!keyspec) {
		error = kPGPError_OutOfMemory;
		goto cleanup;
	}
	pgpKeySpecSetValidity (keyspec, expirationDays);

	allset = pgpKeyDBRingSet (keyDB);

	if (genMaster) {
	   /* Generating master signing key */  
	   addset = ringSetCreate (ringSetPool (allset));
		if (!addset) {
		   error = kPGPError_OutOfMemory;
			goto cleanup;
		}
		prefalgByte = NULL;
		if (prefalgLength > 0) {
			/* Convert preferred algorithm to byte array */
			prefalgLength /= sizeof(PGPCipherAlgorithm);
			prefalgByte = (PGPByte *)pgpContextMemAlloc( context,
														prefalgLength, 0);
			if( IsNull( prefalgByte ) ) {
				error = kPGPError_OutOfMemory;
				goto cleanup;
			}
			for (i=0; i<prefalgLength; ++i) {
				prefalgByte[i] = (PGPByte)prefalg[i];
			}
		}
	   newobj = ringCreateKeypair (pgpEnv, seckey, keyspec, name,
									name_len, pgpRng, addset, addset,
									adkset, adkclass, rakset, rakclass,
									prefalgByte, prefalgLength,
									&error);
		if( IsntNull( prefalgByte ) ) {
			pgpContextMemFree( context, prefalgByte );
		}
	}
	else {
	   /* Generating encryption subkey.  Get the master seckey and 
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
			   error = kPGPError_BadPassphrase;
			   goto cleanup;
			}
			error = (PGPError)pgpSecKeyUnlock (masterseckey, pgpEnv,
										masterpass, masterpassLength);
			if (error != 1) {
			   if (error == 0) 
				   error = kPGPError_BadPassphrase;
				goto cleanup;
			}
		}
	   newobj = ringCreateSubkeypair (pgpEnv, masterseckey, seckey,
									keyspec, pgpRng, addset, addset, &error);
	}
	pgpRandomStir (pgpRng);		/* this helps us count randomness in pool */
	if (error)
		goto cleanup;

	/*  Add objects to main KeyDB.  Before doing so, locate
	   the master key object and return it. */
	ringSetFreeze (addset);
	error = pgpAddObjects (keyDB, addset);
	if (genMaster && !error) {
	   *masterkey = pgpGetKeyByRingObject (keyDB, newobj);
	} else if (!genMaster && !error && IsntNull( newsubkey ) ) {
		PGPSubKey *subk = (PGPSubKey *) (*masterkey)->subKeys.next;
		while( subk != (PGPSubKey *) &(*masterkey)->subKeys ) {
			if( subk->subKey == newobj )
				break;
			subk = subk->next;
		}
		pgpAssert( subk->subKey == newobj );
		*newsubkey = subk;
	}

cleanup:
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


static const PGPOptionType keygenOptionSet[] = {
	kPGPOptionType_KeySetRef,
	kPGPOptionType_KeyGenParams,
	kPGPOptionType_KeyGenName,
	kPGPOptionType_Passphrase,
	kPGPOptionType_Expiration,
	kPGPOptionType_EventHandler,
	kPGPOptionType_PreferredAlgorithms,
	kPGPOptionType_AdditionalRecipientRequestKeySet,
#if PGP_THIRDPARTYREVOCATION
	kPGPOptionType_RevocationKeySet,
#endif
	kPGPOptionType_KeyGenFast
};

PGPError
pgpGenerateKeyInternal(
	PGPContextRef		context,
	PGPKeyRef			*key,
	PGPOptionListRef	optionList
	)
{
	PGPKeySetRef		keyset;
	PGPUInt32			pkalg;
	PGPUInt32			bits;
	PGPUInt32			expiration;
	PGPByte				*name;
	PGPUInt32			nameLength;
	PGPByte				*passphrase;
	PGPUInt32			passphraseLength;
	PGPKeySetRef		adkset;
	PGPUInt32			adkclass;
	PGPKeySetRef		rakset = NULL;
	PGPUInt32			rakclass = 0;
	PGPEventHandlerProcPtr progress;
	PGPUserValue		userValue;
	RingSet const		*adkringset = NULL;
	RingSet const		*rakringset = NULL;
	PGPKeyRef			newkey;
	PGPCipherAlgorithm	*prefalg;
	PGPSize				prefalgLength;
	PGPEnv				*pgpEnv;
	PGPBoolean			fastgenop;
	PGPUInt32			fastgen;
	PGPError			err;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
						keygenOptionSet, elemsof( keygenOptionSet ) ) ) )
		return err;

	if( IsNull( key ) )
		return kPGPError_BadParams;

	pgpEnv = pgpContextGetEnvironment( context );

	/* First pick up mandatory options */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_KeySetRef, TRUE,
						"%p", &keyset ) ) )
		goto error;
	
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_KeyGenParams, TRUE,
						"%d%d", &pkalg, &bits ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_KeyGenName, TRUE,
						"%p%l", &name, &nameLength ) ) )
		goto error;

	/* Now get optional parameters */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_Passphrase, FALSE,
						"%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
		
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_Expiration, FALSE,
						"%d", &expiration ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_EventHandler, FALSE,
						"%p%p", &progress, &userValue ) ) )
		goto error;
	
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_PreferredAlgorithms, FALSE,
						"%p%l", &prefalg, &prefalgLength ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_AdditionalRecipientRequestKeySet,
						FALSE, "%p%d", &adkset, &adkclass ) ) )
		goto error;
	if( IsntNull( adkset ) ) {
		if( IsPGPError( err = pgpKeySetRingSet( adkset, &adkringset ) ) )
			goto error;
	}

#if PGP_THIRDPARTYREVOCATION
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_RevocationKeySet, FALSE,
						"%p%d", &rakset, &rakclass ) ) )
		goto error;
	if( IsntNull( rakset ) ) {
		if( IsPGPError( err = pgpKeySetRingSet( rakset, &rakringset ) ) )
			goto error;
	}
#else
	(void)rakset;
#endif

	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_KeyGenFast, FALSE,
						"%b%d", &fastgenop, &fastgen ) ) )
		goto error;
	if( !fastgenop ) {
		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN, NULL, NULL);
	}
	
	*key = NULL;
	newkey = NULL;		/* Necessary to flag masterkey vs subkey */
	err = pgpDoGenerateKey( keyset->keyDB, &newkey, NULL, (PGPByte)pkalg,
							  bits, (PGPUInt16)expiration,
							  (const char *)name, nameLength,
							  (const char *)passphrase, passphraseLength,
							  NULL, 0, progress, userValue,
							  (PGPBoolean)fastgen,
							  adkringset, (PGPByte)adkclass,
							  rakringset, (PGPByte)rakclass,
							   prefalg, prefalgLength );
	
	if( IsntPGPError( err ) )
	    *key = newkey;

error:
	if( IsntNull( adkringset ) )
		ringSetDestroy( (RingSet *) adkringset );
	return err;
}

static const PGPOptionType subkeygenOptionSet[] = {
	 kPGPOptionType_KeyGenMasterKey,
	 kPGPOptionType_KeyGenParams,
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Expiration,
	 kPGPOptionType_EventHandler,
	 kPGPOptionType_KeyGenFast
};

PGPError
pgpGenerateSubKeyInternal(
	PGPContextRef		context,
	PGPSubKeyRef		*subkey,
	PGPOptionListRef	optionList
	)
{
	PGPUInt32			pkalg;
	PGPUInt32			bits;
	PGPUInt32			expiration;
	PGPByte				*passphrase;
	PGPUInt32			passphraseLength;
	PGPEventHandlerProcPtr progress;
	PGPUserValue		userValue;
	PGPKeyRef			masterkey;
	PGPSubKeyRef		newsubkey;
	PGPEnv				*pgpEnv;
	PGPBoolean			fastgenop;
	PGPUInt32			fastgen;
	PGPError			err;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
					   subkeygenOptionSet, elemsof( subkeygenOptionSet ) ) ) )
		return err;

	if( IsNull( subkey ) )
		return kPGPError_BadParams;

	pgpEnv = pgpContextGetEnvironment( context );

	/* First pick up mandatory options */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_KeyGenMasterKey, TRUE,
						 "%p", &masterkey ) ) )
		goto error;
	
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_KeyGenParams, TRUE,
						 "%d%d", &pkalg, &bits ) ) )
		goto error;


	/* Now get optional parameters */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Expiration, FALSE,
						 "%d", &expiration ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_EventHandler, FALSE,
						 "%p%p", &progress, &userValue ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_KeyGenFast, FALSE,
						 "%b%d", &fastgenop, &fastgen ) ) )
		goto error;
	if( !fastgenop ) {
		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN, NULL, NULL);
	}
	
	
	err	= pgpKeyDeadCheck(masterkey);
	if ( IsPGPError( err ) )
	    return err;

#if 0
    if (bits < 512 || bits > 4096)
	    return kPGPError_BadParams;
	PGPGetKeyNumber (masterkey, kPGPKeyPropAlgID, &pkalg);
	if (pkalg != kPGPPublicKeyAlgorithm_DSA)
	    return kPGPError_BadParams;
#endif
	
	*subkey = NULL;
	err = pgpDoGenerateKey (masterkey->keyDB, &masterkey, &newsubkey,
							   (PGPByte)pkalg, bits, (PGPUInt16)expiration,
							   NULL, 0, (char const *)passphrase,
							   passphraseLength, (char const *)passphrase,
							   passphraseLength, progress, userValue,
							   (PGPBoolean)fastgen, NULL, (PGPByte)0,
							   NULL, (PGPByte)0, NULL, 0);
	if( IsntPGPError( err ) )
	    *subkey = newsubkey;

error:
	return err;
}




/*  Disable the key.  If key is not stored in a writeable KeySet, copy it 
	locally.  Private keys cannot be disabled. */

PGPError
PGPDisableKey (PGPKey *key)
{
	PGPKeyDB		   *keys = NULL;
	RingSet const    *allset = NULL;
	RingSet			*addset = NULL;
	union RingObject   *keyobj;
	PGPError			error = kPGPError_NoErr;

	PGPValidateKey( key );
	
	error	= pgpKeyDeadCheck( key ) ;
	if ( IsPGPError( error ) )
		return error;
		
	keys =		key->keyDB;
	keyobj =	key->key;
	allset =	pgpKeyDBRingSet (keys);

	/*  Axiomatic keys cannot be disabled, but plain old private
	    keys can (because they may belong to someone else).  */
	if (ringKeyAxiomatic (allset, keyobj))
		return kPGPError_BadParams;
	if (!keys->objIsMutable(keys, keyobj))
		return kPGPError_ItemIsReadOnly;

	if ((error = pgpCopyKey (allset, keyobj, &addset)) != kPGPError_NoErr)
		return error;
	if (!ringKeyDisabled (allset, keyobj)) {
	    ringKeyDisable (allset, keyobj);
		pgpKeyDBChanged (keys, addset);
	}

	if (addset)
		ringSetDestroy (addset);
	return error;
}


/*  Enable the key. */

PGPError
PGPEnableKey (PGPKey *key)
{
	PGPKeyDB		   *keys;
    RingSet const	*	allset;
    RingSet *			addset;
	union RingObject   *keyobj;
	PGPError		    error = kPGPError_NoErr;
	
	PGPValidateKey( key );
	error	= pgpKeyDeadCheck( key) ;
	if ( IsPGPError( error ) )
		return error;
		
	keys =		key->keyDB;
	keyobj =	key->key;
	allset =	pgpKeyDBRingSet (keys);

	if (!keys->objIsMutable(keys, keyobj))
		return kPGPError_ItemIsReadOnly;

	if (ringKeyDisabled (allset, keyobj)) {
		if ((error = pgpCopyKey (allset, keyobj, &addset)) != kPGPError_NoErr)
			return error;
	  	ringKeyEnable (allset, keyobj);
		pgpKeyDBChanged (keys, addset);
		ringSetDestroy (addset);
	}
	return kPGPError_NoErr;
}


/*  Change the passphrase.  If the new passphrase is the same as the
	old passphrase, we still unlock the key as the user may be trying to
	set the key's isAxiomatic flag.  */

static PGPError
pgpChangePassphraseInternal (PGPKeyDB *keyDB, RingSet const *ringset, 
							 RingObject *keyobj, RingObject *masterkeyobj, 
							 const char *oldphrase, PGPSize oldphraseLength,
							 const char *newphrase, PGPSize newphraseLength)
{
	unsigned long		 validity;
	RingSet				*addset = NULL;
	union RingObject	*newsecobj, *oldsecobj = NULL;
	PGPKeySpec			*keyspec = NULL;
	PGPError			 error = kPGPError_NoErr;
	PGPSecKey			*seckey = NULL;
	PgpVersion			 version;
	PGPEnv				*pgpEnv;
	PGPRandomContext	*pgpRng;
	PGPBoolean			 locked = 0;

	if (IsntNull(oldphrase) && oldphraseLength == 0)
		oldphrase = NULL;
	if (IsntNull(newphrase) && newphraseLength == 0)
		newphrase = NULL;

	if (!keyDB->objIsMutable(keyDB, keyobj))
		return kPGPError_ItemIsReadOnly;

	if (!ringKeyIsSec (ringset, keyobj))
	    return kPGPError_SecretKeyNotFound;

	/* Find old secret object */
	{
		RingIterator *riter;
		int level;
		riter = ringIterCreate (ringset);
		if (!riter)
			return kPGPError_OutOfMemory;
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
	pgpEnv = pgpContextGetEnvironment( keyDB->context );
	seckey = ringSecSecKey (ringset, oldsecobj, 0);
	if (!seckey)
	    return ringSetError(ringset)->error;
	if (pgpSecKeyIslocked (seckey)) {
		locked = 1;
	    if (!oldphrase) {
		    error = kPGPError_BadPassphrase;
			goto cleanup;
		}
		error = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, oldphrase, 
								 oldphraseLength);
		if (error != 1) {
		    if (error == 0) 
			    error = kPGPError_BadPassphrase;
			goto cleanup;
		}
	}
	
	/*  All done if passphrase has not changed */
	if ((!oldphrase && !newphrase) ||
		(oldphrase && locked && newphrase && (oldphraseLength==newphraseLength)
		 && strcmp (oldphrase, newphrase) == 0))
	{
	    error = kPGPError_NoErr;
		goto cleanup;
	}

	error = pgpCopyKey (ringset, keyobj, &addset);
	if (error)
		goto cleanup;

	pgpRng = pgpContextGetX9_17RandomContext( keyDB->context );
    error = (PGPError)pgpSecKeyChangeLock (seckey, pgpEnv, pgpRng, 
								 newphrase, newphraseLength);
	if (error)
	    goto cleanup;

	keyspec = pgpKeySpecCreate (pgpEnv);
	if (!keyspec) {
	    error = kPGPError_OutOfMemory;
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
	pgpKeySpecSetValidity (keyspec, (PGPUInt16) validity);

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
		error = keyDB->remove(keyDB, oldsecobj);
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
PGPChangePassphrase (PGPKey *key, char const *oldphrase, char const *newphrase)
{
	RingSet	const		*ringset;
	PGPError		 error;
	PGPSize			 oldphraseLength;
	PGPSize			 newphraseLength;

	PGPValidateKey( key );
	PGPValidatePtr( oldphrase );
	PGPValidatePtr( newphrase );
	
	oldphraseLength = strlen( oldphrase );
	newphraseLength = strlen( newphrase );
	ringset = pgpKeyDBRingSet (key->keyDB);
	error = pgpChangePassphraseInternal (key->keyDB, ringset, key->key, NULL,
										 oldphrase, oldphraseLength,
										 newphrase, newphraseLength);
	if (!error)
		ringKeySetAxiomatic (ringset, key->key);
	return error;
}


PGPError 
PGPChangeSubKeyPassphrase (PGPSubKeyRef subkey, 
						   char const *oldphrase, char const *newphrase)
{
	RingSet const			*ringset;
	PGPSize			 oldphraseLength;
	PGPSize			 newphraseLength;
	
	PGPValidateSubKey( subkey );
	PGPValidatePtr( oldphrase );
	PGPValidatePtr( newphrase );
	oldphraseLength = strlen( oldphrase );
	newphraseLength = strlen( newphrase );
	
	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	return pgpChangePassphraseInternal (subkey->key->keyDB, ringset, 
									    subkey->subKey,
										subkey->key->key,
										oldphrase, oldphraseLength,
										newphrase, newphraseLength);
}


/*  Remove a subkey */

PGPError
PGPRemoveSubKey (PGPSubKeyRef subkey)
{
	PGPKeyDB			  *keys;
	RingSet const        *allset;
	union RingObject      *subkeyobj;

	pgpa(pgpaPGPSubKeyValid(subkey));
	PGPValidateSubKey( subkey );
	
	CHECKREMOVED(subkey);
	keys = subkey->key->keyDB;
	
	allset =  pgpKeyDBRingSet (keys);
	subkeyobj = subkey->subKey;
	if (!keys->objIsMutable(keys, subkeyobj))
		return kPGPError_ItemIsReadOnly;

	return pgpRemoveObject (keys, subkeyobj);
}


static PGPError
sRevokeSubKey (
	PGPContextRef		context,
	PGPSubKeyRef		subkey,
	char const *		passphrase,
	PGPSize				passphraseLength,
	PGPKeyRef			signkey
	)
{
    PGPKeyDB			*keys;
	RingSet const		*allset;
	RingSet *			addset;
	union RingObject    *subkeyobj, *keyobj;
	union RingObject	*signkeyobj = NULL;
	PGPError			 error = kPGPError_NoErr;
	
	CHECKREMOVED(subkey);
	keys =		subkey->key->keyDB;
	subkeyobj =	subkey->subKey;
	keyobj = subkey->key->key;

	if (pgpSubKeyIsDead (subkey))
	    return kPGPError_NoErr;
	if (!keys->objIsMutable(keys, subkeyobj))
		return kPGPError_ItemIsReadOnly;
	
	allset = pgpKeyDBRingSet (keys);

	/* If revoking someone else's key, they must have authorized us */
	if (IsntNull (signkey) ) {
		signkeyobj = signkey->key;
		if (!ringKeyIsRevocationKey (keyobj, allset, signkeyobj) )
			return kPGPError_SecretKeyNotFound;
	} else {
		signkeyobj = keyobj;
	}

	if (!ringKeyIsSec (allset, signkeyobj))
		return kPGPError_SecretKeyNotFound;			/* not our key */

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;

	/*  Note special subkey revocation sigtype */
	error = pgpCertifyObject (context, subkeyobj, addset, signkeyobj,
					allset, PGP_SIGTYPE_KEY_SUBKEY_REVOKE, passphrase,
					passphraseLength, FALSE,
					SIG_EXPORTABLE, kPGPExpirationTime_Never, 0, 0, NULL);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	
	/*  Update the KeyDB */
	error = pgpAddObjects (keys, addset); 
	ringSetDestroy (addset);

	/* Calculate trust changes as a result */
	if( error == kPGPError_NoErr )
		(void)pgpPropagateTrustKeyDB (keys);

	return error;
}


static const PGPOptionType revsubkeyOptionSet[] = {
#if PGP_THIRDPARTYREVOCATION
	 kPGPOptionType_SignWithKey,
#endif
	 kPGPOptionType_Passphrase
};

PGPError
pgpRevokeSubKeyInternal(
	PGPSubKeyRef		subkey,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPKeyRef			signkey = NULL;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPSubKeyValid(subkey));
	PGPValidateSubKey( subkey );

	context = subkey->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
					   revsubkeyOptionSet, elemsof( revsubkeyOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;

#if PGP_THIRDPARTYREVOCATION
	/* Used for revoking someone else's key */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						kPGPOptionType_SignWithKey, FALSE,
						"%p", &signkey ) ) )
		goto error;
#endif

	err = sRevokeSubKey( context, subkey, passphrase, passphraseLength,
						 signkey );
error:
	return err;
}



/*  Remove a User ID.  If the KeySet is read-only, or the UserID object 
	itself is read-only, we return an error. */

PGPError
PGPRemoveUserID (PGPUserID *userid)
{
	PGPKeyDB			  *keys;
	PGPUserID             *uidtmp;
	int                    uidcount = 0;
	RingSet const        *allset;
	union RingObject      *nameobj;

	PGPValidateUserID( userid );
	
	CHECKREMOVED(userid);
	/*  Cannot remove only UserID */
	for (uidtmp = (PGPUserID *) userid->key->userIDs.next;
		 uidtmp != (PGPUserID *) &userid->key->userIDs;
		 uidtmp = uidtmp->next) {
		if (!uidtmp->removed)
			uidcount++;
	}
	if (uidcount == 1)
		return kPGPError_BadParams;
	keys = userid->key->keyDB;
	
	allset =  pgpKeyDBRingSet (keys);
	nameobj = userid->userID;
	if (!keys->objIsMutable(keys, nameobj))
		return kPGPError_ItemIsReadOnly;

	return pgpRemoveObject (keys, nameobj);
}


/*
 *	Add a new User ID to a key.  User IDs cannot be added to other than the 
 *	user's own keys.  The new User ID is added to the end of the list.  To
 *	make it the primary User ID, call PGPSetPrimaryUserID() below.
 */

static PGPError
sAddUserID (
	PGPContextRef	context,
	PGPKeyRef		key,
	char const *	userID,
	PGPSize		   	userIDLength,
	char const *	passphrase,
	PGPSize			passphraseLength
	)
{
	PGPKeyDB			*keys;
	PGPUserID			*userid;
	RingSet const		*allset;
	RingSet *			addset;
	union RingObject	*keyobj, *nameobj;
	PGPError			 error;

	error	= pgpKeyDeadCheck( key) ;
	if ( IsPGPError( error ) )
		return error;
	keys = key->keyDB;
	
	allset = pgpKeyDBRingSet (keys);
	keyobj = key->key;

	if (!keys->objIsMutable(keys, keyobj))
		return kPGPError_ItemIsReadOnly;

	/*  Can only add User ID to our own keys */
	if (!ringKeyIsSec (allset, keyobj)) 
		return kPGPError_SecretKeyNotFound;

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;
	nameobj = ringCreateName (addset, keyobj, userID, userIDLength);
	if (!nameobj) {
		error = ringSetError(addset)->error;
		ringSetDestroy (addset);
		return error;
	}

	/*  ringCreateName will return a duplicate nameobj if 
		the name already exists for this key.  Check the
		list of PGPUserID objects to see if the nameobj
		is already referenced. */
	for (userid = (PGPUserID *) key->userIDs.next; 
		 userid != (PGPUserID *) &key->userIDs;
		 userid = userid->next) {
	    if (!userid->removed && userid->userID == nameobj) {
		    ringSetDestroy (addset);
			return kPGPError_DuplicateUserID;
		}
	}

	/* Must self-certify here */
	error = pgpCertifyObject (context, nameobj, addset, keyobj, allset, 
							  PGP_SIGTYPE_KEY_GENERIC,  passphrase,
							  passphraseLength, TRUE,
							  SIG_EXPORTABLE, kPGPExpirationTime_Never, 0, 0,
							  NULL);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);

	/* Calculate trust changes as a result */
	if( error == kPGPError_NoErr )
		(void)pgpPropagateTrustKeyDB (keys);

	return error;
} 


static const PGPOptionType adduserOptionSet[] = {
	 kPGPOptionType_Passphrase
};

PGPError
pgpAddUserIDInternal(
	PGPKeyRef		key,
	char const *	userID,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPSize				userIDLength;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );
	
	if( IsNull( userID ) )
		return kPGPError_BadParams;

	userIDLength = strlen(userID);

	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
						adduserOptionSet, elemsof( adduserOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;

	err = sAddUserID( context, key, userID, userIDLength, passphrase,
					  passphraseLength);
error:
	return err;
}


/*	Make the given User ID the primary User ID of the key */

PGPError
PGPSetPrimaryUserID (PGPUserID *userid)
{
	PGPKeyDB		  *keys;
	PGPKey			  *key;
	RingSet const *		allset;
	RingSet *			addset;
	PGPError		   error;

	PGPValidateUserID( userid );
	
	CHECKREMOVED(userid);
	key = userid->key;
	error	= pgpKeyDeadCheck( key) ;
	if ( IsPGPError( error ) )
		return error;
	
	keys = key->keyDB;
	if (!keys->objIsMutable(keys, key->key))
		return kPGPError_ItemIsReadOnly;
	
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


/*  Certify a User ID.  Do not allow duplicate certification. If UserID
    is already certified, but revoked, the old cert can
	be removed and the UserID then recertified. */

	static PGPError
sCertifyUserID(
	PGPContextRef	context,
	PGPUserID *		userid,
	PGPKeyRef		certifying_key,
	char const *	passphrase,
	PGPSize			passphraseLength,
	PGPBoolean		exportable,
	PGPUInt32		expiration,
	PGPByte			trustDepth,
	PGPByte			trustValue,
	char const *	sRegExp
	)
{
	PGPKeyDB		    *keys;
	RingSet const		*ringset;
	RingSet				*addset;
	RingSet const		*signerset;
	union RingObject    *keyobj, *nameobj, *sigobj;
	RingIterator		*iter;
	PGPError			 error = kPGPError_NoErr;

	(void) context;

	error	= pgpKeyDeadCheck( userid->key ) ;
	if ( IsPGPError( error ) )
		return error;

	if (certifying_key)
		pgpa(pgpaPGPKeyValid(certifying_key));

	if (userid->removed)
		return kPGPError_BadParams;
	keys = userid->key->keyDB;

	ringset = pgpKeyDBRingSet (keys);
	nameobj = userid->userID;

	if (!keys->objIsMutable(keys, nameobj))
		return kPGPError_ItemIsReadOnly;

	/*  If certifying key was not passed, get the default */
	if (!certifying_key) {
		error = pgpGetDefaultPrivateKeyInternal(keys, &certifying_key);
		if ( IsPGPError( error ) )
		{
			error	= kPGPError_SecretKeyNotFound;
			return error;
		}
	}

	error	= pgpKeyDeadCheck( certifying_key ) ;
	if ( IsPGPError( error ) )
		return error;

	/*  Get RingSet for certifying key */
	if (certifying_key->keyDB != keys)
		signerset = pgpKeyDBRingSet (certifying_key->keyDB);
	else
		signerset = ringset;
	keyobj = certifying_key->key;

	/*  Check for duplicate certificate.  There may be some
		old revocation certs still laying around, which we
		should ignore.  */

	iter = ringIterCreate (ringset);
	if (!iter)
	    return ringSetError(ringset)->error;
	ringIterSeekTo (iter, nameobj);
	while ((error = (PGPError)ringIterNextObject (iter, 3)) > 0) {
	    sigobj = ringIterCurrentObject (iter, 3);
		if (ringSigMaker (ringset, sigobj, signerset) == keyobj &&
			ringSigType (ringset, sigobj) != PGP_SIGTYPE_KEY_UID_REVOKE) {
		    error = kPGPError_DuplicateCert;
			break;
		}
	}
	ringIterDestroy (iter);
	if (error)
	    return error;

	error = pgpCopyKey (ringset, nameobj, &addset);
	if (error)
		return error;
	error = pgpCertifyObject (keys->context, nameobj, addset, keyobj,
					  signerset, PGP_SIGTYPE_KEY_GENERIC, passphrase,
					  passphraseLength, FALSE,
					  exportable, expiration, trustDepth, trustValue, sRegExp);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);

	/* Calculate trust changes as a result */
	if( error == kPGPError_NoErr )
		(void)pgpPropagateTrustKeyDB (keys);

	return error;
}


static const PGPOptionType signuserOptionSet[] = {
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Expiration,
	 kPGPOptionType_Exportable,
	 kPGPOptionType_CertificateTrust,
	 kPGPOptionType_CertificateRegularExpression
};

	PGPError
pgpCertifyUserIDInternal (
	PGPUserID *			userid,
	PGPKey *			certifying_key,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	PGPUInt32			expiration;
	PGPUInt32			exportable;
	PGPUInt32			trustDepth;
	PGPUInt32			trustLevel;
	char *				passphrase;
	PGPSize				passphraseLength;
	char const *		sRegExp;
	PGPError			err = kPGPError_NoErr;

	context = userid->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
						signuserOptionSet, elemsof( signuserOptionSet ) ) ) )
		return err;

	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
		
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Expiration, FALSE,
						 "%d", &expiration ) ) )
		goto error;
	/* Defaults exportable to false */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Exportable, FALSE,
						 "%d", &exportable ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_CertificateTrust, FALSE,
						 "%d%d", &trustDepth, &trustLevel ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_CertificateRegularExpression, FALSE,
						 "%p", &sRegExp ) ) )
		goto error;

	/* Check regexp for validity */
	if( IsntNull( sRegExp ) ) {
		regexp *rexp;
		if (IsPGPError( pgpRegComp( context, sRegExp, &rexp ) ) ) {
			pgpDebugMsg( "Invalid regular expression" );
			err = kPGPError_BadParams;
			goto error;
		}
		pgpContextMemFree( context, rexp );
	}

	/* Expiration is given as days from today, we will convert to seconds */
	if( expiration != 0 )
		expiration *= (24*60*60);

	err = sCertifyUserID( context, userid, certifying_key, passphrase,
						  passphraseLength,
						  (PGPBoolean)exportable, expiration,
						  (PGPByte) trustDepth, (PGPByte) trustLevel, sRegExp);

error:
	return err;
}


/*  Given a cert, return the certifying key object.  The signing key does not 
	have to be in the same set as <cert>, and may be in the <allkeys> set. */

PGPError
PGPGetSigCertifierKey (
	PGPSigRef		cert,
	PGPKeySetRef	allkeys,
	PGPKey **		certkey)
{
	PGPKeyID				keyID;
	PGPInt32				algTemp;
	PGPError				err	= kPGPError_NoErr;
	PGPPublicKeyAlgorithm	pubKeyAlg;
	PGPContextRef			context;

	PGPValidatePtr( certkey );
	*certkey	= NULL;
	PGPValidateCert( cert );
	PGPValidateKeySet( allkeys );
	
	CHECKREMOVED(cert);
	
	context	= PGPGetKeySetContext( allkeys );
	
	err	= PGPGetKeyIDOfCertifier( cert, &keyID);
	if ( IsntPGPError( err ) )
	{
		PGPGetSigNumber(cert, kPGPSigPropAlgID, &algTemp);
		
		pubKeyAlg	= (PGPPublicKeyAlgorithm)algTemp;
		
		err	= PGPGetKeyByKeyID(allkeys, &keyID, pubKeyAlg, certkey );
		/* PGPGetKeyByKeyID incremented the ref count */
	}
	
	pgpAssertErrWithPtr( err, *certkey );
	return err;
}


/*  Revoke a certification.  If allkeys == NULL, the certifying key
	must be in the same keyDB as the certificate. */

static PGPError
sRevokeCert (
	PGPContextRef	context,
	PGPSigRef		cert,
	PGPKeySetRef	allkeys,
	char const *	passphrase,
	PGPSize			passphraseLength
	)
{
	PGPKeyDB			*keys;
	PGPKey				*certkey;
	RingSet const		*allset;
	RingSet				*addset;
	RingSet const 		*signerset;
	union RingObject	*sigobj, *nameobj;
	PGPBoolean			 revoked;
	PGPBoolean			 exportable;
	PGPError			 error = kPGPError_NoErr;

	CHECKREMOVED(cert);
	keys = cert->up.userID->key->keyDB;
	sigobj = cert->cert;
	if (!keys->objIsMutable(keys, sigobj))
		return kPGPError_ItemIsReadOnly;

	error = PGPGetSigBoolean (cert, kPGPSigPropIsRevoked, &revoked);
	if (error)
		return error;
	if (revoked)
		return kPGPError_NoErr;   /* already revoked */

	/*  Get certifying key and its RingSet */
	error = PGPGetSigCertifierKey (cert, allkeys, &certkey);
	if (error)
		return error;
	if (!certkey)
		return kPGPError_SecretKeyNotFound;

	error	= pgpKeyDeadCheck( certkey ) ;
	if ( IsPGPError( error ) )
		return error;
		
		
	signerset =  pgpKeyDBRingSet (certkey->keyDB);

	/*  Get signature RingSet and its name object */
	allset = pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	nameobj = cert->up.userID->userID;

	error = pgpCopyKey (allset, nameobj, &addset);
	if (error)
		return error;
	/* Copy exportability attribute from cert we are revoking */
	error = PGPGetSigBoolean (cert, kPGPSigPropIsExportable, &exportable);
	if (error)
		return error;
	error = pgpCertifyObject (context, nameobj, addset, certkey->key,
				  signerset, PGP_SIGTYPE_KEY_UID_REVOKE, passphrase,
				  passphraseLength, FALSE,
				  exportable, kPGPExpirationTime_Never, 0, 0, NULL);
	if (error) {
		ringSetDestroy (addset);
		return error;
	}
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);

	/* Calculate trust changes as a result */
	if( error == kPGPError_NoErr )
		(void)pgpPropagateTrustKeyDB (keys);

	return error;
}


static const PGPOptionType revsigOptionSet[] = {
	 kPGPOptionType_Passphrase
};

PGPError
pgpRevokeCertInternal(
	PGPSigRef			cert,
	PGPKeySetRef		allkeys,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPCertValid(cert));
	PGPValidateCert( cert );
	PGPValidateKeySet( allkeys );

	context = cert->up.userID->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( context, optionList,
						revsigOptionSet, elemsof( revsigOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( context, optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;

	err = sRevokeCert( context, cert, allkeys, passphrase, passphraseLength);
error:
	return err;
}



/*  Remove a certification.  If the certification was revoked, the
    revocation signature remains.  This ensures that the same
	signature on someone else's keyring is properly revoked
	if this key is exported.   A future certification will have
	a later creation timestamp than the revocation and will therefore
	not be affected. */

PGPError
PGPRemoveSig (PGPSig *cert)
{
	PGPKeyDB			*keys;
	union RingObject	*sigobj;

	PGPValidateCert( cert );
	
	CHECKREMOVED(cert);
	keys = cert->up.userID->key->keyDB;
	sigobj = cert->cert;
	if (!keys->objIsMutable(keys, sigobj))
		return kPGPError_ItemIsReadOnly;
	return pgpRemoveObject (keys, sigobj);
}


	PGPError
PGPCountAdditionalRecipientRequests(
	PGPKeyRef		basekey,
    PGPUInt32 *		numARKeys)
{
	RingSet const		*ringset = NULL;/* Aurora ringset to look in */
	PGPUInt32			 nadks;			/* Number ADK's available */
	PGPError			err	= kPGPError_NoErr;
	
	PGPValidatePtr( numARKeys );
	*numARKeys	= 0;
	PGPValidateKey( basekey );
	
	ringset = pgpKeyDBRingSet (basekey->keyDB);
	if (IsNull( ringset ) )
		err = kPGPError_OutOfRings;
	
	if( IsntPGPError( err ) )
	{
		PGPByte	 			tclass;		/* Class code from ADK */
		union RingObject    *keyobj;		/* Aurora base key */
		
		keyobj	= basekey->key;
		err = ringKeyAdditionalRecipientRequestKeyID8 (keyobj, ringset, 0,
								&tclass, &nadks, NULL, NULL);
		if ( err == kPGPError_ItemNotFound )
		{
			nadks	= 0;
			err		= kPGPError_NoErr;
		}
	}
	
	*numARKeys = nadks;
	return err;
}

/*  Return the nth (0 based) additional decryption keyID, if one exists.
	It is an error to use an index >= K, where K is the number of ARR key ids.
	
 	Also return the class of the ADK.  The class is currently reserved
 	for use by PGP.
 	Either of the return pointers may be NULL, but not both.
 	
	Note that it is *not* safe to use the keyID returned from this function
	to get the ADK to use because KeyIDs are not unique.
	Instead, the keyID can be used to locate the actual key(s) with that
	key id.
	Then call PGPGetIndexedAdditionalRecipientRequest to get the key;
	it will check the key fingerprint, which is unique.

*/

	static PGPError
pgpGetIndexedAdditionalRecipientRequestKeyID(
	PGPKeyRef		basekey,
	PGPUInt32		nth,
	PGPKeyID *		adkeyid,
    PGPByte *		adclass)
{
	RingSet const		*ringset;		/* Aurora ringset to look in */
	union RingObject    *keyobj;		/* Aurora base key */
	unsigned			 nadks;			/* Number ADK's available */
	PGPByte				 tclass;		/* Class code from ADK */
	PGPError			 error;			/* Error return from Aurora */
	PGPByte				 pkalg;			/* pkalg of ADK */
	PGPKeyID			keyid;		/* keyid of ADK */
	PGPError			 err	= kPGPError_NoErr;
	PGPContextRef		 context;

	pgpAssert( IsntNull( adkeyid ) || IsntNull( adclass ) );
	if( IsntNull( adkeyid ) )
		pgpClearMemory( adkeyid, sizeof( *adkeyid ) );
	if ( IsntNull( adclass ) )
		*adclass	= 0;
		
	ringset = NULL;

	pgpa(pgpaPGPKeyValid(basekey));
	PGPValidateKey( basekey );
	
	context	= PGPGetKeyContext( basekey );
	
	ringset = pgpKeyDBRingSet (basekey->keyDB);
	if (IsNull( ringset ) )
		return kPGPError_OutOfRings;

	keyobj = basekey->key;
	error = ringKeyAdditionalRecipientRequestKeyID8 (keyobj, ringset, nth,
									&tclass, &nadks, &pkalg, &keyid);

	if( IsPGPError( error ) )
	{
		return error;
	}
	
	/* Success */
	if ( IsntNull( adkeyid ) )
	{
		*adkeyid	= keyid;
	}
	
	if ( IsntNull( adclass ) )
		*adclass = tclass;
	return err;
}

	PGPError
PGPGetIndexedAdditionalRecipientRequestKeyID(
	PGPKeyRef		basekey,
	PGPUInt32		nth,
	PGPKeyID *		adkeyid,
    PGPByte *		adclass)
{
	PGPError	err	= kPGPError_NoErr;
	PGPKeyID	tempKeyID;
	
	if ( IsntNull( adkeyid ) )
		pgpClearMemory( adkeyid, sizeof( *adkeyid) );
	if ( IsntNull( adclass ) )
		*adclass	= 0;
	PGPValidatePtr( adkeyid );
	PGPValidateKey( basekey );
	
	err	= pgpGetIndexedAdditionalRecipientRequestKeyID( basekey, nth,
												  &tempKeyID, adclass );
	if ( IsntPGPError( err ) )
	{
		pgpAssert( pgpKeyIDIsValid( &tempKeyID ) );
		*adkeyid	= tempKeyID;
	}
	else
	{
		pgpClearMemory( adkeyid, sizeof( *adkeyid) );
	}
	
	return( err );
}


/* Given a key, return the nth (0 based) additional decryption key, if
 	one exists.  Also return the class of the ADK, and the number of
 	ADK's for the base key.  Any of the return pointers may be
 	NULL. */

	static PGPError
pgpGetIndexedAdditionalRecipientRequest(
	PGPKeyRef		basekey,
	PGPKeySetRef	allkeys,
	PGPUInt32		nth,
    PGPKeyRef*		adkey,
    PGPByte *		adclass)
{
	RingSet const		*ringset;		/* Aurora ringset to look in */
	union RingObject    *keyobj;		/* Aurora base key */
	union RingObject    *rkey;			/* Aurora additional decryption key */
	unsigned			 nadks;			/* Number ADK's available */
	PGPByte				 tclass;		/* Class code from ADK */
	PGPError			 error;			/* Error return from Aurora */
	PGPByte				 pkalg;			/* pkalg of ADK */
	PGPError			err	= kPGPError_NoErr;
	PGPContextRef		context;

	if ( IsntNull( adkey ) )
		*adkey	= NULL;
	if ( IsntNull( adclass ) )
		*adclass	= 0;
		
	ringset = NULL;

	PGPValidateKey( basekey );
	PGPValidateKeySet( allkeys );
	
	context	= PGPGetKeyContext( basekey );
	
	error = pgpKeySetRingSet (allkeys, &ringset);
	if( IsPGPError( error ) )
	{
		rkey = NULL;
	}
	else
	{
		keyobj = basekey->key;
		rkey = ringKeyAdditionalRecipientRequestKey (keyobj, ringset, nth,
								&tclass, &nadks, &error);
	}
	
	if( IsPGPError( error ) )
	{
		/* No key found, or error */
		if (ringset)
			ringSetDestroy( (RingSet *) ringset );
		return error;
	}
	
	/* Success */
	if ( IsntNull( adkey ) )
	{
		PGPKeyID	keyID;
		
		ringKeyID8 (ringset, rkey, &pkalg, &keyID);
		
		err = PGPGetKeyByKeyID (allkeys, &keyID,
				(PGPPublicKeyAlgorithm)pkalg, adkey);
}
	
	if ( IsntNull( adclass ) )
		*adclass = tclass;
	if (ringset)
		ringSetDestroy( (RingSet *) ringset );
		
	pgpAssertErrWithPtr( error, rkey );
	return err;
}


	PGPError
PGPGetIndexedAdditionalRecipientRequest(
	PGPKeyRef		basekey,
	PGPKeySetRef	allkeys,
	PGPUInt32		nth,
    PGPKeyRef*		adkey,
    PGPByte *		adclass)
{
	PGPError	err	= kPGPError_NoErr;
	
	if ( IsntNull( adkey ) )
		*adkey	= NULL;
	if ( IsntNull( adclass ) )
		*adclass	= 0;
	PGPValidatePtr( adkey );
	PGPValidateKey( basekey );
	PGPValidateKeySet( allkeys );
	
	err	= pgpGetIndexedAdditionalRecipientRequest( basekey,
			allkeys, nth, adkey, adclass );
	if ( IsntPGPError( err ) &&
		IsNull( *adkey ) )
	{
		err	= kPGPError_ItemNotFound;
	}
	
	pgpAssertErrWithPtr( err, *adkey );
	return( err );
}


/*  Trust-related functions */

#if 0	/* KEEP [ */
	PGPError
PGPSetUserIDConfidence(PGPUserID *userid, PGPUInt32 confidence)
{
	PGPKeyDB			*keys;
	RingSet const		*allset = NULL;
	RingSet				*addset = NULL;
	union RingObject    *nameobj;
	RingPool			*pgpRingPool;
	PGPError			 error = kPGPError_NoErr;

	pgpa(pgpaPGPUserIDValid(userid));
	PGPValidateUserID( userid );
	
	keys = userid->key->keyDB;

	pgpRingPool = pgpContextGetRingPool( keys->context );
	pgpAssert (pgpTrustModel (pgpRingPool) > PGPTRUST0);
	CHECKREMOVED(userid);
	err	= pgpKeyDeadCheck( userid->key ) ;
	if ( IsPGPError( err ) )
		return err;

	allset =  pgpKeyDBRingSet (keys);
	nameobj = userid->userID;
	if (ringKeyIsSec (allset, userid->key->key))
		return kPGPError_BadParams;
	if (!keys->objIsMutable(keys, nameobj))
		return kPGPError_ItemIsReadOnly;

	error = pgpCopyKey (allset, nameobj, &addset);
	if (error)
		return error;
	ringNameSetConfidence (allset, nameobj, (unsigned short) confidence);
	pgpKeyDBChanged (keys, addset);
cleanup:
	if (addset)
		ringSetDestroy (addset);
	return error;
}

#endif	/* ] KEEP */

/*  Set the trust on a key.  Cannot be used to set undefined or 
	axiomatic trust.   The key must be valid to assign trust. */

PGPError
PGPSetKeyTrust (PGPKey *key, PGPUInt32 trust)
{
	PGPKeyDB			*keys;
	RingSet	const		*allset;
	RingSet				*addset = NULL;
	union RingObject	*keyobj;
	RingPool			*pgpRingPool;
	PGPError			 error = kPGPError_NoErr;
#if ONLY_TRUST_VALID_KEYS
	long                 validity;
#endif

	PGPValidateKey( key );
	
	keys = key->keyDB;
	pgpRingPool = pgpContextGetRingPool( keys->context );
	pgpAssert (pgpTrustModel (pgpRingPool) == PGPTRUST0);

	allset =  pgpKeyDBRingSet (keys);
	keyobj =  key->key;

	if (!keys->objIsMutable(keys, keyobj))
		return kPGPError_ItemIsReadOnly;

	if (trust <= kPGPKeyTrust_Undefined || trust > kPGPKeyTrust_Complete ||
		 ringKeyAxiomatic (allset, keyobj))
	{
	    return kPGPError_BadParams;
	}
	    
	error	= pgpKeyDeadCheck( key);
	if ( IsPGPError( error ) )
		return error;

#if ONLY_TRUST_VALID_KEYS
	/*  Should not set trust on key that is not completely valid 
		(who is it we are trusting?) */
	PGPGetKeyNumber (key, kPGPKeyPropValidity, &validity);
	if (validity != kPGPValidity_Complete) 
	    return kPGPError_BadParams;
#endif

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;

	ringKeySetTrust (allset, keyobj, (PGPByte)trust);
	pgpKeyDBChanged (keys, addset);

	if (addset)
		ringSetDestroy (addset);
	return error;
}


/*  Set a secret key as the axiomatic key.  If checkPassphrase == TRUE,
	the user must prove knowledge of the passphrase in order to do 
	this. */

PGPError
PGPSetKeyAxiomatic (
	PGPKey *		key,
	PGPBoolean		checkPassphrase,
	char const *	passphrase)
{
    PGPBoolean               secret, axiomatic;
	RingSet const           *allset;
	RingSet					*addset = NULL;
	union RingObject        *keyobj;
	PGPSecKey         		*seckey;
	PGPEnv					*pgpEnv;
	PGPKeyDB                *keys;
	PGPError                 error = kPGPError_NoErr;

	PGPValidateKey( key );
	if ( checkPassphrase )
	{
		PGPValidatePtr( passphrase );
	}
	
	PGPGetKeyBoolean (key, kPGPKeyPropIsSecret, &secret);
	if (!secret)
	    return kPGPError_BadParams;
	PGPGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &axiomatic);
	if (axiomatic)
	    return kPGPError_NoErr;

	keys = key->keyDB;
	allset = pgpKeyDBRingSet (keys);
	keyobj = key->key;

	if (!keys->objIsMutable(keys, keyobj))
		return kPGPError_ItemIsReadOnly;

	if (checkPassphrase) {
	    /* Get the secret key and attempt to unlock it */
	    seckey = ringSecSecKey (allset, keyobj, PGP_PKUSE_SIGN);
		if (!seckey)
		    return ringSetError(allset)->error;
		if (pgpSecKeyIslocked (seckey)) {
		    if (!passphrase) {
			    pgpSecKeyDestroy (seckey);
			    return kPGPError_BadPassphrase;
			}
			pgpEnv = pgpContextGetEnvironment( keys->context );
			error = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, passphrase, 
									 strlen (passphrase));
			pgpSecKeyDestroy (seckey);
			if (error != 1) {
				if (error == 0)
				    error = kPGPError_BadPassphrase;
				return error;
			}
		}
	}

	/*  Make sure it's enabled first before setting axiomatic */
	if ((error = PGPEnableKey (key)) != kPGPError_NoErr)
	    return error;
	if ((error = pgpCopyKey (allset, keyobj, &addset)) != kPGPError_NoErr)
		return error;
	ringKeySetAxiomatic (allset, keyobj);
	pgpKeyDBChanged (keys, addset);

	if (addset)
	    ringSetDestroy (addset);
	return error;
}


PGPError
PGPUnsetKeyAxiomatic (PGPKey *key)
{
    PGPBoolean                  axiomatic;
    RingSet const          *allset;
    RingSet 				*addset = NULL;
	union RingObject        *keyobj;
	PGPKeyDB                 *keys;
	PGPError                  error = kPGPError_NoErr;

	PGPValidateKey( key );
	
	PGPGetKeyBoolean (key, kPGPKeyPropIsAxiomatic, &axiomatic);
	if (!axiomatic)
	    return kPGPError_BadParams;

	keys = key->keyDB;
	allset = pgpKeyDBRingSet (keys);
	keyobj = key->key;

	if (!keys->objIsMutable(keys, keyobj))
		return kPGPError_ItemIsReadOnly;

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;
	ringKeyResetAxiomatic (allset, keyobj);
	pgpKeyDBChanged (keys, addset);

	if (addset)
	    ringSetDestroy (addset);
	return error;
}



/*  Get property functions.  Internal GetKey functions work for both
    master keys and subkeys.  */


static PGPError
pgpReturnPropBuffer (char const *src, void *prop, 
					 PGPSize srclen, PGPSize proplen)
{
    PGPError result = kPGPError_NoErr;

    if (srclen > proplen) {
	    srclen = proplen;
		result = kPGPError_BufferTooSmall;
	}
	if ( IsntNull( prop ) && srclen > 0)
	    pgpCopyMemory( src, prop, srclen);
	return result;
}


static PGPError
pgpGetKeyNumberInternal (RingSet const *ringset, RingObject *keyobj,
						 PGPKeyPropName propname, PGPInt32 *prop)
{
	unsigned char		 pkalg;
	PGPByte                 trust;

	switch (propname) {
	case kPGPKeyPropAlgID:
		ringKeyID8 (ringset, keyobj, &pkalg, NULL);
		*prop = (long) pkalg;
		break;
	case kPGPKeyPropBits:
		*prop = (long) ringKeyBits (ringset, keyobj);
		break;
	case kPGPKeyPropTrust:
		if (pgpTrustModel (ringSetPool(ringset)) == PGPTRUST0) {
			trust = ringKeyTrust (ringset, keyobj);
			if (trust == kPGPKeyTrust_Undefined ||
				trust == kPGPKeyTrust_Unknown)
			    trust = kPGPKeyTrust_Never;
			*prop = (long) trust;
			break;
		}
	default:
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}


PGPError
PGPGetKeyNumber (PGPKey *key, PGPKeyPropName propname, PGPInt32 *prop)
{
    PGPError            error = kPGPError_NoErr;
	PGPUserID          *userid;
	RingSet const           *ringset;
	RingPool		   *pgpRingPool;
	PGPInt32                trustval;

	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateKey( key );
	
    switch (propname) {
	case kPGPKeyPropValidity:
		pgpRingPool = pgpContextGetRingPool( key->keyDB->context );
		if (pgpTrustModel (pgpRingPool) == PGPTRUST0) {
			*prop = kPGPValidity_Unknown;
			pgpIncKeyRefCount (key);
			for (userid = (PGPUserID *) key->userIDs.next; 
				 userid != (PGPUserID *) &key->userIDs; userid = userid->next)
				 { 
				if (!userid->removed) {
					PGPGetUserIDNumber (userid, kPGPUserIDPropValidity, 
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
		error =  pgpGetKeyNumberInternal (ringset, key->key, propname, prop);
	}
	return error;
}

PGPError
PGPGetSubKeyNumber (
PGPSubKeyRef subkey, PGPKeyPropName propname, PGPInt32 *prop)
{
    PGPError            error = kPGPError_NoErr;
	RingSet const            *ringset;

	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateSubKey( subkey );
	
	CHECKREMOVED(subkey);
	switch (propname) {
	case kPGPKeyPropAlgID:
	case kPGPKeyPropBits:
	    ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	    error = pgpGetKeyNumberInternal (ringset, subkey->subKey,
										 propname, prop);
		break;
	default:
		return kPGPError_InvalidProperty;
	}
	return error;
}


static PGPError
pgpGetKeyTimeInternal (RingSet const *ringset, RingObject *keyobj,
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
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}


PGPError
PGPGetKeyTime (PGPKey *key, PGPKeyPropName propname, PGPTime *prop)
{
    RingSet const       *ringset;
	
	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateKey( key );
	
	ringset = pgpKeyDBRingSet (key->keyDB);
	return pgpGetKeyTimeInternal (ringset, key->key, propname, prop);
}


PGPError
PGPGetSubKeyTime (PGPSubKeyRef subkey, PGPKeyPropName propname, PGPTime *prop)
{
    RingSet const        *ringset;
	
	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateSubKey( subkey );
	
	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	return pgpGetKeyTimeInternal (ringset, subkey->subKey, propname, prop);
}


	static PGPError
pgpGetKeyStringInternal(
	RingSet const *		ringset,
	RingObject *		keyobj, 
	 PGPKeyPropName		propname,
	 void *				prop,
	 PGPSize			bufferSize,
	 PGPSize *			actualLength)
{
	unsigned char		 pkalg;
	uchar                buffer[20];
	PGPCipherAlgorithm * prefAlgsLong;
	PGPByte *			 prefAlgs;
	PGPUInt32			 i;
	PGPContextRef		 context;
	PGPError			 err;

	switch (propname) {
	default:
		return kPGPError_InvalidProperty;
		
	case kPGPKeyPropFingerprint:
		ringKeyID8 (ringset, keyobj, &pkalg, NULL);
		if (pkalg == 1) {
			ringKeyFingerprint16 (ringset, keyobj, buffer);
			*actualLength = 16;
		}
		else {
			ringKeyFingerprint20 (ringset, keyobj, buffer);
			*actualLength = 20;
		}
		break;
	case kPGPKeyPropPreferredAlgorithms:
		/* Must convert from byte form to array of PGPCipherAlgorithm */
		prefAlgs = (PGPByte *)ringKeyFindSubpacket (
				keyobj, ringset, SIGSUB_PREFERRED_ALGS, 0, actualLength,
				NULL, NULL, NULL, NULL, &err);
		if( IsNull( prefAlgs ) ) {
			*actualLength = 0;
			return kPGPError_NoErr;
		}
		context = ringPoolContext(ringSetPool(ringset));
		prefAlgsLong = (PGPCipherAlgorithm *)pgpContextMemAlloc ( context,
							bufferSize * sizeof(PGPCipherAlgorithm), 0 );
		if( IsNull( prefAlgsLong ) )
			return kPGPError_OutOfMemory;
		for (i=0; i < bufferSize; ++i) {
			prefAlgsLong[i] = (PGPCipherAlgorithm)prefAlgs[i];
		}
		*actualLength *= sizeof(PGPCipherAlgorithm);
		err = pgpReturnPropBuffer (
				(char const *)prefAlgsLong, prop, *actualLength,
				bufferSize );
		pgpContextMemFree( context, prefAlgsLong );
		return err;
	}
	
	return pgpReturnPropBuffer ( (char const *)buffer,
			prop, *actualLength, bufferSize);
}

	PGPError
PGPGetKeyPropertyBuffer(
	PGPKeyRef		key,
	PGPKeyPropName	propname,
	PGPSize			bufferSize,
	void *			outData,
	PGPSize *		outLength )
{
    RingSet const       *ringset;
    PGPError			err	= kPGPError_NoErr;

	PGPValidatePtr( outLength );
	*outLength	= 0;
	PGPValidateKey( key );
	/* outData is allowed to be NULL */
	if ( IsntNull( outData ) )
	{
		pgpClearMemory( outData, bufferSize );
	}
	
	ringset = pgpKeyDBRingSet (key->keyDB);
	
	err	= pgpGetKeyStringInternal (ringset, key->key,
			propname, outData, bufferSize, outLength );

	return( err );
}


	PGPError
PGPGetSubKeyPropertyBuffer(
	PGPSubKeyRef	subKey,
	PGPKeyPropName	propname,
	PGPSize			bufferSize,
	void *			outData,
	PGPSize *		outLength )
{
    RingSet const       *ringset;
    PGPError			err	= kPGPError_NoErr;

	PGPValidatePtr( outLength );
	*outLength	= 0;
	PGPValidateSubKey( subKey );
	if ( IsntNull( outData ) )
		pgpClearMemory( outData, bufferSize );
	
	CHECKREMOVED(subKey);
	ringset = pgpKeyDBRingSet (subKey->key->keyDB);
	
	err	= pgpGetKeyStringInternal (ringset, subKey->subKey,
			propname, outData, bufferSize, outLength );

	return( err );
}



static PGPError
pgpGetKeyBooleanInternal (RingSet const *ringset, RingObject *keyobj,
						  PGPKeyPropName propname, PGPBoolean *prop)
{
	PGPSecKey *			seckey = NULL;
	PGPUInt32			expiration;

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
	case kPGPKeyPropHasUnverifiedRevocation:
		{
			/*  Must look for a revocation signature with the same signing 
				key id. */
			PGPKeyID			keyid;
			PGPKeyID			revkeyid;
			RingIterator *		iter = NULL;
			RingObject *		obj;
			unsigned			level;

			*prop = FALSE;
			ringKeyID8 (ringset, keyobj, NULL, &keyid);
			iter = ringIterCreate (ringset);
			if (!iter)
				return kPGPError_OutOfMemory;
			ringIterSeekTo (iter, keyobj);
			level = ringIterCurrentLevel (iter);
			ringIterRewind (iter, level+1);
			while (ringIterNextObject (iter, level+1) > 0) {
				obj = ringIterCurrentObject (iter, level+1);
				if (ringObjectType(obj) == RINGTYPE_SIG &&
					ringSigType (ringset, obj) == PGP_SIGTYPE_KEY_COMPROMISE) {
					ringSigID8 (ringset, obj, NULL, &revkeyid);
					if (pgpKeyIDsEqual( &keyid, &revkeyid )) {
						*prop = TRUE;
						break;
					}
				}
			}
			ringIterDestroy (iter);
		}
		break;
	case kPGPKeyPropIsDisabled:
		*prop = (ringKeyDisabled (ringset, keyobj) != 0);
		break;
	case kPGPKeyPropNeedsPassphrase:
		if (!ringKeyIsSec (ringset, keyobj))
			return kPGPError_SecretKeyNotFound;
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
			*prop = (expiration < (PGPUInt32) PGPGetTime());
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
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}


PGPError
PGPGetKeyBoolean (PGPKey *key, PGPKeyPropName propname, PGPBoolean *prop)
{
    RingSet const       *ringset;

	PGPValidatePtr( prop );
	*prop	= FALSE;
	PGPValidateKey( key );
	
	ringset = pgpKeyDBRingSet (key->keyDB);
	return pgpGetKeyBooleanInternal (ringset, key->key, propname, prop);
}


PGPError
PGPGetSubKeyBoolean (
PGPSubKeyRef subkey, PGPKeyPropName propname, PGPBoolean *prop)
{
    RingSet const       *ringset;

	pgpa(pgpaPGPSubKeyValid(subkey));
	
	PGPValidatePtr( prop );
	*prop	= FALSE;
	PGPValidateSubKey( subkey );
	
	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);
	if (propname == kPGPKeyPropIsAxiomatic)
	    return kPGPError_InvalidProperty;
	return pgpGetKeyBooleanInternal (ringset, subkey->subKey, propname, prop);
}


PGPError
PGPGetUserIDNumber (
PGPUserID *userid, PGPUserIDPropName propname, PGPInt32 *prop)
{
	RingSet const		*ringset = NULL;
	union RingObject    *nameobj = NULL;

	pgpa(pgpaPGPUserIDValid(userid));
	
	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateUserID( userid );
	
	CHECKREMOVED(userid);
	ringset =	pgpKeyDBRingSet (userid->key->keyDB);
	nameobj =	userid->userID;

	switch (propname) {
	case kPGPUserIDPropValidity:
		if (pgpTrustModel (ringSetPool(ringset)) == PGPTRUST0) 
		    *prop = (long) ringNameTrust (ringset, nameobj);
		else
			*prop = (long) ringNameValidity (ringset, nameobj);
		break;
	case kPGPUserIDPropConfidence: 
		if (pgpTrustModel (ringSetPool(ringset)) > PGPTRUST0) {
			*prop = ringNameConfidence (ringset, nameobj);
			break;
		}
	default:
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}


/*____________________________________________________________________________
	Name is always returned NULL terminated.
	
	if 'outString' is NULL, then just the size is returned.
____________________________________________________________________________*/
	PGPError
PGPGetUserIDStringBuffer(
	PGPUserIDRef		userid,
	PGPUserIDPropName	propname,
	PGPSize				bufferSize,
	char *				outString,
	PGPSize *			fullLengthOut )
{
	RingSet const		*ringset = NULL;
	char const	        *bufptr;
	PGPError			err	= kPGPError_NoErr;
	PGPSize				fullLength;
	PGPSize				nullOffset;

	if ( IsntNull( fullLengthOut ) )
		*fullLengthOut	= 0;
	PGPValidateUserID( userid );
	PGPValidateParam( bufferSize != 0 );
	PGPValidateParam( propname == kPGPUserIDPropName );
#if PGP_DEBUG
	if ( IsntNull( outString ) )
		pgpClearMemory( outString, bufferSize );
#endif
	
	CHECKREMOVED(userid);
	ringset	=	pgpKeyDBRingSet (userid->key->keyDB);
	bufptr	= ringNameName (ringset, userid->userID, &fullLength );

	++fullLength;	/* account for null terminator */
	err	= pgpReturnPropBuffer ( bufptr,
		(PGPByte *)outString, fullLength, bufferSize);
	
	if ( IsntNull( outString ) )
	{
		/* always null terminate since it's a string */
 		nullOffset				= pgpMin( fullLength, bufferSize ) - 1;
		outString[ nullOffset ]	= '\0';
	}
	
	if ( IsntNull( fullLengthOut ) )
		*fullLengthOut	= fullLength;
	
	return( err );
}

PGPError
PGPGetSigNumber (PGPSig *cert, PGPSigPropName propname, PGPInt32 *prop)
{
	RingSet const		*ringset = NULL;
	union RingObject    *sigobj = NULL;
	PGPByte				pkalg;
	unsigned long		longkeyid;
	int                 i;

	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateCert( cert );
	
	CHECKREMOVED(cert);
	ringset =	pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPSigPropKeyID:
	{
		PGPKeyID		keyid;
		PGPByte const *	idBytes;;
	
		ringSigID8 (ringset, sigobj, NULL, &keyid);
		longkeyid = 0;
		idBytes	= pgpGetKeyBytes( &keyid );
		for (i = 4; i < 8; i++)
			longkeyid = (longkeyid << 8) + idBytes[i];
			
		/* *prop should be cast to (unsigned long) */
		*prop = (long) longkeyid;  
		break;
	}
	
	case kPGPSigPropAlgID:
		ringSigID8 (ringset, sigobj, &pkalg, NULL);
		*prop = (long) pkalg;
		break;
	case kPGPSigPropTrustLevel:
		*prop = (long)ringSigTrustLevel(ringset, sigobj);
		break;
	case kPGPSigPropTrustValue:
		*prop = (long)ringSigTrustValue(ringset, sigobj);
		break;
	default:
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}



	PGPError
PGPGetKeyIDOfCertifier(
	PGPSig *		cert,
	PGPKeyID *	outID )
{
	RingSet const *		ringset = NULL;
	union RingObject *	sigobj = NULL;
	PGPError			err	= kPGPError_NoErr;

	PGPValidatePtr( outID );
	pgpClearMemory( outID, sizeof( *outID ) );
	PGPValidatePtr( cert );
	
	pgpa(pgpaPGPCertValid(cert));
	CHECKREMOVED(cert);
	ringset	= pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj	= cert->cert;

	ringSigID8( ringset, sigobj, NULL, outID );
	return err;
}



PGPError
PGPGetSigTime (PGPSig *cert, PGPSigPropName propname, PGPTime *prop)
{
	RingSet const		*ringset = NULL;
	union RingObject    *sigobj = NULL;

	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateCert( cert );
	
	CHECKREMOVED(cert);
	ringset =	pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPSigPropCreation:
		*prop = ringSigTimestamp (ringset, sigobj);
		break;
	case kPGPSigPropExpiration:
		*prop = ringSigExpiration (ringset, sigobj);
		break;
	default:
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}



PGPError
PGPGetSigBoolean (PGPSig *cert, PGPSigPropName propname, PGPBoolean *prop)
{
	RingSet	const	*ringset = NULL;
	union RingObject    *sigobj = NULL, *obj = NULL;
	PGPKeyID			 keyid, revkeyid;
	RingIterator *iter = NULL;
	unsigned			 level;

	PGPValidatePtr( prop );
	*prop	= FALSE;
	PGPValidateCert( cert );
	
	CHECKREMOVED(cert);
	ringset =	pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPSigPropIsRevoked:
	case kPGPSigPropHasUnverifiedRevocation:
		/*  Must look for a revocation signature with the same signing 
			key id.  The revocation sig must be the newer than the certifying
			sig to be considered. */
		*prop = 0;
		ringSigID8 (ringset, sigobj, NULL, &keyid);
		iter = ringIterCreate (ringset);
		if (!iter)
			return kPGPError_OutOfMemory;
		ringIterSeekTo (iter, sigobj);
		level = ringIterCurrentLevel (iter);
		ringIterRewind (iter, level);
		while (ringIterNextObject (iter, level) > 0) {
			obj = ringIterCurrentObject (iter, level);
			if (ringSigType (ringset, obj) == PGP_SIGTYPE_KEY_UID_REVOKE) {
				ringSigID8 (ringset, obj, NULL, &revkeyid);
				if (pgpKeyIDsEqual( &keyid, &revkeyid ) &&
					    ((propname==kPGPSigPropHasUnverifiedRevocation)
						 || ringSigChecked (ringset, obj)) &&
					       ringSigTimestamp (ringset, obj) >= 
					           ringSigTimestamp (ringset, sigobj)) {
					*prop = 1;
					break;
				}
			}
		}
		ringIterDestroy (iter);
		break;
	case kPGPSigPropIsNotCorrupt:
		*prop = (ringSigError (ringset, sigobj) == 0);
		break;
	case kPGPSigPropIsTried:
		*prop = ringSigTried (ringset, sigobj);
		break;
	case kPGPSigPropIsVerified:
		*prop = ringSigChecked (ringset, sigobj);
		break;
	case kPGPSigPropIsMySig:
		obj = ringSigMaker (ringset, sigobj, ringset);
		if (!obj)
			*prop = 0;
		else
			*prop = ringKeyIsSec (ringset, obj);
		break;
	case kPGPSigPropIsExportable:
		*prop = ringSigExportable (ringset, sigobj);
		break;
	default:
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}



/*  Get and Set default private key.  The identification of
	the key is stored as an ascii keyid in the preferences
	repository. */


	PGPError 
PGPGetDefaultPrivateKey (
	PGPKeySet *	keyset,
	PGPKeyRef *	outRef )
{
	PGPError	err	= kPGPError_NoErr;
	
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKeySet( keyset );
	
	err	= pgpGetDefaultPrivateKeyInternal( keyset->keyDB, outRef );
	/* refcount has been incremented */
	
	pgpAssertErrWithPtr( err, *outRef );
	return( err );
}



PGPError
PGPSetDefaultPrivateKey (PGPKeyRef	key)
{
	PGPBoolean	isSecret = 0;
	PGPBoolean	cansign = 0;
	PGPError	err	= kPGPError_NoErr;
	PGPKeyID	keyID;
	
	PGPValidateKey( key );
	
	err	= pgpKeyDeadCheck( key) ;
	if ( IsPGPError( err ) )
		return err;
	    
	/*  Default key must be secret and must be able to sign */
	PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &isSecret);
	PGPValidateParam( isSecret );
	
	PGPGetKeyBoolean( key, kPGPKeyPropCanSign, &cansign);
	PGPValidateParam( cansign );

	/* Set the default key axiomatic (note we don't require a passphrase) */
	PGPSetKeyAxiomatic( key, FALSE, NULL);
	
	err	= PGPGetKeyIDFromKey( key, &keyID );
	if ( IsntPGPError( err ) )
	{
		PGPByte		data[ kPGPMaxExportedKeyIDSize ];
		PGPSize		exportedSize;
		
		err	= PGPExportKeyID( &keyID, data, &exportedSize );
		if ( IsntPGPError( err ) )
		{
			err	= PGPsdkPrefSetData( PGPGetKeyContext( key),
					kPGPsdkPref_DefaultKeyID, data, exportedSize );
		}
	}
	
	return err;
}


PgpTrustModel
PGPGetTrustModel (
	PGPContextRef	context
	)
{
	RingPool			*pgpRingPool;

	pgpRingPool = pgpContextGetRingPool( context );
	return pgpTrustModel (pgpRingPool);
}


/*  UserVal functions */

PGPError
PGPSetKeyUserVal (PGPKey *key, PGPUserValue userVal)
{
	PGPValidateKey( key );
	key->userVal = userVal;
	return kPGPError_NoErr;
}


PGPError
PGPSetUserIDUserVal (PGPUserID *userid, PGPUserValue userVal)
{
	PGPValidateUserID( userid );
	userid->userVal = userVal;
	return kPGPError_NoErr;
}


PGPError
PGPSetSubKeyUserVal (PGPSubKeyRef subkey, PGPUserValue userVal)
{
	PGPValidateSubKey( subkey );
	subkey->userVal = userVal;
	return kPGPError_NoErr;
}


PGPError
PGPSetSigUserVal (PGPSig *cert, PGPUserValue userVal)
{
	PGPValidateCert( cert );
	cert->userVal = userVal;
	return kPGPError_NoErr;
}


PGPError
PGPGetKeyUserVal (PGPKey *key, PGPUserValue *userVal)
{
	PGPValidateKey( key );
	*userVal = key->userVal;
	return kPGPError_NoErr;
}


PGPError
PGPGetUserIDUserVal (PGPUserID *userid, PGPUserValue *userVal)
{
	PGPValidateUserID( userid );
	*userVal = userid->userVal;
	return kPGPError_NoErr;
}

PGPError
PGPGetSubKeyUserVal (PGPSubKeyRef subkey, PGPUserValue *userVal)
{
	PGPValidateSubKey( subkey );
	*userVal = subkey->userVal;
	return kPGPError_NoErr;
}

PGPError
PGPGetSigUserVal (PGPSig *cert, PGPUserValue *userVal)
{
	PGPValidateCert( cert );
	*userVal = cert->userVal;
	return kPGPError_NoErr;
}

	PGPError 
PGPGetPrimaryUserID (
	PGPKey *		key,
	PGPUserIDRef *	outRef)
{
	const PGPUserID *	userID;
	PGPError			err	= kPGPError_NoErr;

	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKey( key );

	userID = (const PGPUserID *) &key->userIDs;
	do
	{
		userID = userID->next;
		if (userID == (const PGPUserID *)&key->userIDs)
		{
			err	= kPGPError_ItemNotFound;
			break;
		}
	} while (userID->removed);

	*outRef	= (PGPUserIDRef)userID;
	
	return err;
}


/*____________________________________________________________________________
	Name is always returned NULL terminated.
	
	if name is null, then just the size is returned
____________________________________________________________________________*/
	PGPError
PGPGetPrimaryUserIDNameBuffer(
	PGPKeyRef	key,
	PGPSize		bufferSize,
	char *		name,
	PGPSize *	fullLength  )
{
	PGPUserIDRef		userID;
	PGPError			err	= kPGPError_NoErr;
	
	PGPValidateKey( key );
	PGPValidateParam( IsntNull( name ) || IsntNull( fullLength ) );

	err	 = PGPGetPrimaryUserID (key, &userID );
	if ( IsntPGPError( err ) )
	{
		err	= PGPGetUserIDStringBuffer( userID,
			kPGPUserIDPropName, bufferSize, name, fullLength);
	}
	
	return( err );
}

PGPError
PGPGetPrimaryUserIDValidity (PGPKey *key, PGPValidity *validity)
{
	PGPUserID *		userID;
	PGPError		err	= kPGPError_NoErr;

	PGPValidatePtr( validity );
	*validity	= kPGPValidity_Unknown;
	PGPValidateKey( key );

	err = PGPGetPrimaryUserID(key, &userID);
	if ( IsntPGPError( err ) )
	{
		PGPInt32	temp;
		
		err	= PGPGetUserIDNumber(userID, kPGPUserIDPropValidity, &temp);
		if ( IsntPGPError( err ) )
			*validity	= (PGPValidity)temp;
	}
	return( err );
}

/*
 * XXX: This routine must be kept in sync with the hash algorithm
 *      selection made in pgpSigSpecCreate (in pgpSigSpec.c)
 */
PGPError
PGPGetHashAlgUsed (PGPKey *key, PGPHashAlgorithm *hashAlg)
{
	PGPPublicKeyAlgorithm	pkAlg	= kPGPPublicKeyAlgorithm_Invalid;
	PGPEnv*					pgpEnv;
	PGPInt32				temp;
	PGPError				err;

	PGPValidatePtr( hashAlg );
	*hashAlg	= kPGPHashAlgorithm_Invalid;
	PGPValidateKey( key );

	err = PGPGetKeyNumber(key, kPGPKeyPropAlgID, &temp);
	pkAlg	= (PGPPublicKeyAlgorithm)temp;
	if ( IsntPGPError( err ) )
	{
		
		pgpEnv = pgpContextGetEnvironment( key->keyDB->context );
		if (pkAlg == kPGPPublicKeyAlgorithm_DSA)
			*hashAlg = kPGPHashAlgorithm_SHA;
		else
			*hashAlg = (PGPHashAlgorithm)
				pgpenvGetInt(pgpEnv, PGPENV_HASH, NULL, NULL);
	}
	return err;
}

/*
 * The following functions are for internal use within other parts of the
 * library, to access the lower level components of PGPKeys and associated
 * structures.
 */

PGPError
pgpGetKeyRingObject (PGPKey *key, PGPBoolean checkDead, RingObject **pRingKey)
{
	PGPError	err;
	
	pgpa((
		pgpaPGPKeyValid(key),
		pgpaAddrValid(pRingKey, RingObject *)));
	
	*pRingKey = NULL;
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pRingKey = key->key;
	return kPGPError_NoErr;
}

PGPError
pgpGetKeyRingSet (PGPKey *key, PGPBoolean checkDead, RingSet const**pRingSet)
{
	PGPError	err;
	
	pgpa((
		pgpaPGPKeyValid(key),
		pgpaAddrValid(pRingSet, RingSet *)));
	
	*pRingSet = NULL;
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pRingSet = pgpKeyDBRingSet( key->keyDB );
	return kPGPError_NoErr;
}

PGPError
pgpGetUserIDRingObject (PGPUserID *userid, PGPBoolean checkDead,
	RingObject **pRingName)
{
	PGPKey			*key;
	PGPError	err;

	pgpa((
		pgpaPGPUserIDValid(userid),
		pgpaAddrValid(pRingName, RingObject *)));

	*pRingName = NULL;
	if (userid->removed)
	    return kPGPError_BadParams;
	key = userid->key;
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pRingName = userid->userID;
	return kPGPError_NoErr;
}

PGPError
pgpGetUserIDRingSet (PGPUserID  *userid, PGPBoolean checkDead,
	RingSet const**pRingSet)
{
	PGPKey		*key;
	PGPError	err;

	pgpa((
		pgpaPGPUserIDValid(userid),
		pgpaAddrValid(pRingSet, RingSet *)));
	
	*pRingSet = NULL;
	if (userid->removed)
	    return kPGPError_BadParams;
	key = userid->key;
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pRingSet = pgpKeyDBRingSet( key->keyDB );
	return kPGPError_NoErr;
}

PGPError
pgpGetUserIDKey (PGPUserID *userid, PGPBoolean checkDead, PGPKey **pKey)
{
	PGPKey			*key;
	PGPError	err;

	pgpa((
		pgpaPGPUserIDValid(userid),
		pgpaAddrValid(pKey, PGPKey *)));

	*pKey = NULL;
	if (userid->removed)
	    return kPGPError_BadParams;
	key = userid->key;
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pKey = key;
	return kPGPError_NoErr;
}

PGPError
pgpGetCertRingObject (PGPSig *cert, PGPBoolean checkDead,
	RingObject **pRingSig)
{
	PGPKey			*key;
	PGPUserID		*userid;
	PGPError		err;

	pgpa((
		pgpaPGPCertValid(cert),
		pgpaAddrValid(pRingSig, RingObject *)));

	*pRingSig = NULL;
	if (cert->removed)
	    return kPGPError_BadParams;
	if (cert->type==uidcert) {
		userid = cert->up.userID;
		if (userid->removed)
			return kPGPError_BadParams;
		key = userid->key;
	} else {
		key = cert->up.key;
	}
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pRingSig = cert->cert;
	return kPGPError_NoErr;
}

PGPError
pgpGetCertRingSet (PGPSig *cert, PGPBoolean checkDead,
	RingSet const **pRingSet)
{
	PGPKey			*key;
	PGPUserID		*userid;
	PGPError		err;

	pgpa((
		pgpaPGPCertValid(cert),
		pgpaAddrValid(pRingSet, RingSet *)));
	
	*pRingSet = NULL;
	if (cert->removed)
	    return kPGPError_BadParams;
	if (cert->type==uidcert) {
		userid = cert->up.userID;
		if (userid->removed)
			return kPGPError_BadParams;
		key = userid->key;
	} else {
		key = cert->up.key;
	}
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pRingSet = pgpKeyDBRingSet( key->keyDB );
	return kPGPError_NoErr;
}

/* This sets *pUserid to NULL but returns no error if cert is on a key */
PGPError
pgpGetCertUserID (PGPSig *cert, PGPBoolean checkDead, PGPUserID **pUserid)
{
	PGPKey			*key;
	PGPUserID		*userid = NULL;
	PGPError		err;

	pgpa((
		pgpaPGPCertValid(cert),
		pgpaAddrValid(pUserid, PGPUserID *)));

	*pUserid = NULL;
	if (cert->removed)
	    return kPGPError_BadParams;
	if (cert->type==uidcert) {
		userid = cert->up.userID;
		if (userid->removed)
			return kPGPError_BadParams;
		key = userid->key;
	} else {
		key = cert->up.key;
	}
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pUserid = userid;			/* Will be NULL if keycert */
	return kPGPError_NoErr;
}

PGPError
pgpGetCertKey (PGPSig *cert, PGPBoolean checkDead, PGPKey **pKey)
{
	PGPKey			*key;
	PGPUserID		*userid;
	PGPError		err;

	pgpa((
		pgpaPGPCertValid(cert),
		pgpaAddrValid(pKey, PGPKey *)));

	*pKey = NULL;
	if (cert->removed)
	    return kPGPError_BadParams;
	if (cert->type==uidcert) {
		userid = cert->up.userID;
		if (userid->removed)
			return kPGPError_BadParams;
		key = userid->key;
	} else {
		key = cert->up.key;
	}
	if( checkDead ) {
		err	= pgpKeyDeadCheck( key) ;
		if ( IsPGPError( err ) )
			return err;
	}

	*pKey = key;
	return kPGPError_NoErr;
}




	PGPContextRef
pgpGetKeyDBContext( PGPKeyDBRef ref )
{
	pgpAssert( pgpKeyDBIsValid( ref ) );
	
	if ( ! pgpKeyDBIsValid( ref ) )
		return( kPGPInvalidRef );
		
	return( ref->context );
}

	PGPContextRef
PGPGetKeyListContext( PGPKeyListRef ref )
{
	if ( ! pgpKeyListIsValid( ref ) )
		return( kPGPInvalidRef );
		
	return( PGPGetKeySetContext( ref->keySet ) );
}


	PGPContextRef
PGPGetKeySetContext( PGPKeySetRef ref )
{
	if ( ! pgpKeySetIsValid( ref ) )
		return( kPGPInvalidRef );
		
	return( pgpGetKeyDBContext( ref->keyDB ) );
}

	PGPContextRef
PGPGetKeyIterContext( PGPKeyIterRef ref )
{
	if ( ! pgpKeyIterIsValid( ref ) )
		return( kPGPInvalidRef );
		
	return( PGPGetKeyListContext( ref->keyList ) );
}

	PGPContextRef
PGPGetKeyContext( PGPKeyRef ref )
{
	if ( ! pgpKeyIsValid( ref ) )
		return( kPGPInvalidRef );
		
	return( pgpGetKeyDBContext( ref->keyDB ) );
}

	PGPContextRef
PGPGetSubKeyContext( PGPSubKeyRef ref )
{
	if ( ! pgpSubKeyIsValid( ref ) )
		return( kPGPInvalidRef );
		
	return( pgpGetKeyDBContext( ref->key->keyDB ) );
}


	PGPContextRef
PGPGetUserIDContext( PGPUserIDRef ref )
{
	if ( ! pgpUserIDIsValid( ref ) )
		return( kPGPInvalidRef );
		
	return( PGPGetKeyContext( ref->key ) );
}



/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
