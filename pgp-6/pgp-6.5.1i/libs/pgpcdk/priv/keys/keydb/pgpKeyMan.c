/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpKeyMan.c,v 1.208 1999/05/17 05:08:11 hal Exp $
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
#include "pgpStr2Key.h"
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
#include "pgpSymmetricCipherPriv.h"
#include "pgpX509Priv.h"


#define MAXRSABITS		2048

#define elemsof(x) ((unsigned)(sizeof(x)/sizeof(*x)))

/*  INTERNAL FUNCTIONS */

/*  Internal function for certifying a key or userid.  Object to be signed 
	must be	in dest.  Signing key should be in src.  selfsig should be
    set for self-certifying names. */

#define SIG_EXPORTABLE			TRUE
#define SIG_NON_EXPORTABLE		FALSE
#define SIG_EXPORTABLEHASHED	TRUE
#define SIG_EXPORTABLEUNHASHED	FALSE

static PGPError
pgpCertifyObject(
	PGPContextRef 		context,
	union RingObject 	*to_sign,
	RingSet	 			*dest,
	union RingObject 	*signer,
	RingSet const 		*src, 
	PGPByte 			sigtype,
	char const 			*passphrase,
	PGPSize 			passphraseLength,
	PGPBoolean 			hashedPhrase,
	PGPBoolean 			selfsig,
	PGPBoolean 			exportable,
	PGPBoolean 			exportableHashed,
	PGPTime				sigCreation,
	PGPUInt32 			sigExpiration,
	PGPByte 			trustDepth,
	PGPByte 			trustValue,
	char const 			*sRegExp,
	RingSet const		*rakset,
	PGPUInt32			rakclass
	)
{
    PGPSecKey		*seckey = NULL;
    PGPSigSpec		*sigspec = NULL;
	PGPEnv			*pgpEnv;
	PGPRandomContext *pgpRng;
	RingIterator	*rakIter;
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
	    if (IsNull( passphrase )) {
		    pgpSecKeyDestroy (seckey);
			return kPGPError_BadPassphrase;
		}
	    error = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, passphrase, 
								 passphraseLength, hashedPhrase);
		if (error != 1)
		{
	        pgpSecKeyDestroy (seckey);
			if (error == 0)
			    error = kPGPError_BadPassphrase;
			return error;
	    }
    }
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
		pgpSigSpecSetVersion (sigspec, PGPVERSION_4);
		if ((p=ringKeyFindSubpacket (signer, src,
				SIGSUB_PREFERRED_ENCRYPTION_ALGS, 0,
				&plen, NULL, NULL, NULL, NULL, NULL)) != 0) {
			pgpSigSpecSetPrefAlgs (sigspec, 0, p, plen);
		}
		if (ringKeyExpiration (src, signer)) {
			PGPUInt32 period = ringKeyExpiration (src, signer) -
				ringKeyCreation (src, signer);
			pgpSigSpecSetKeyExpiration (sigspec, 0, period);
		}
	}
	if (!exportable) {
		pgpSigSpecSetExportable (sigspec,
							 (exportableHashed ? 0 : kPGPSigFlags_Unhashed),
							 exportable);
	}
	
	if( sigCreation != 0 )
	{
		pgpSigSpecSetTimestamp( sigspec, sigCreation +
				(60 * 60 * pgpenvGetInt(pgpEnv, PGPENV_TZFIX,  NULL, NULL)));
	}
	
	if (sigExpiration)
		pgpSigSpecSetSigExpiration (sigspec, 0, sigExpiration);
	if( IsntNull( sRegExp ) )
		pgpSigSpecSetRegExp (sigspec, 0, sRegExp);

	/* Ignore trustValue for ordinary level 0 signatures */
	if (trustDepth != 0) {
		/* Convert trust value to extern format */
		if (trustValue != 0)
			trustValue = ringTrustOldToExtern(ringSetPool(dest), trustValue);
		/* Note that setting nonzero trustvalue forces V4 sigs */
		pgpSigSpecSetTrustLevel (sigspec, 0, trustDepth, trustValue);
	}
	pgpRng = pgpContextGetX9_17RandomContext( context );

	/* Due to a bug in 5.0, all sigs directly on keys must be version 2_6.
	 * However the only signatures 5.0 handles directly on keys are key
	 * revocations.
	 */
	if (ringObjectType( to_sign ) == RINGTYPE_KEY &&
			!ringKeyIsSubkey( dest, to_sign )  &&
			sigtype == PGP_SIGTYPE_KEY_REVOKE ) {
		pgpSigSpecSetVersion( sigspec, PGPVERSION_3 );
	}

	/* Handle revocation authorizations */
	if( IsntNull( rakset ) ) {
		rakIter = ringIterCreate (rakset);
		if (!rakIter) {
			pgpSecKeyDestroy (seckey);
			pgpSigSpecDestroy (sigspec);
			return ringSetError(rakset)->error;
		}
		while (ringIterNextObject (rakIter, 1) > 0) {
			PGPByte krinfo[22];
			PGPByte pkalg;
			RingObject *krkey = ringIterCurrentObject (rakIter, 1);
			/* Note that rakclass must have 0x80 set to be effective */
			ringKeyID8 (rakset, krkey, &pkalg, NULL);
			krinfo[0] = rakclass;
			krinfo[1] = pkalg;
			ringKeyFingerprint20 (rakset, krkey, krinfo+2);
			error = pgpSigSpecSetRevocationKey (sigspec, 0, krinfo,
												sizeof(krinfo) );
			if (IsPGPError(error)) {
				pgpSecKeyDestroy (seckey);
				pgpSigSpecDestroy (sigspec);
				ringIterDestroy (rakIter);
				return error;
			}
		}
		ringIterDestroy (rakIter);
		/* Make this signature non-revocable */
		pgpSigSpecSetRevocable (sigspec, 0, FALSE);
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
	PGPError			err			= kPGPError_NoErr;
	PGPByte *			keyIDData	= NULL;
	void *				vkeyIDData;
	PGPSize				keyIDSize	= 0;
	PGPContextRef		context		= pgpGetKeyDBContext( keyDB );
	
	PGPValidatePtr( outKey );
	*outKey	= kInvalidPGPKeyRef;
	
	err	= PGPsdkPrefGetData( context, kPGPsdkPref_DefaultKeyID,
				&vkeyIDData, &keyIDSize );
	keyIDData = vkeyIDData;
	if ( IsntPGPError( err ) )
	{
		PGPKeyID		keyID;
		
		err	= PGPImportKeyID( keyIDData, &keyID );
		if ( IsntPGPError( err ) )
		{
			PGPKeySetRef kset = pgpKeyDBRootSet( keyDB );
			
			err	= PGPGetKeyByKeyID( kset, &keyID,
						kPGPPublicKeyAlgorithm_Invalid, outKey );
			PGPFreeKeySet( kset );
		}
		
		/* we used public API call; must free using PGPFreeData() */
		PGPFreeData( keyIDData );
	}
	
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
	PGPBoolean			hashedPhrase
	)
{
    PGPKeyDBRef			 keys = NULL;
	RingSet	const *		allset = NULL;
	RingSet *			addset = NULL;
	union RingObject    *keyobj;
	union RingObject	*signkeyobj = NULL;
	PGPUInt32			 revnum;
	PGPError			 error = kPGPError_NoErr;
	
	keys =		key->keyDB;
	keyobj =	key->key;

	if ( !keys->objIsMutable( keys, keyobj ) )
		return kPGPError_ItemIsReadOnly;
	if ( IsPGPError( pgpKeyDeadCheck(key) ) )
	   return kPGPError_NoErr;	/* no need */
	
	allset = pgpKeyDBRingSet (keys);

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;

	revnum = 0;
	for ( ; ; ) {
		signkeyobj = keyobj;
		/* See if we have an authorized revocation signature */
		if (!ringKeyIsSec (allset, keyobj)) {
			PGPByte revclass;
			signkeyobj = ringKeyRevocationKey (keyobj, allset, revnum++,
											   NULL, NULL,
											   &revclass, NULL, &error);
			if( IsPGPError( error ) ) {
				if( error == kPGPError_ItemNotFound )
					error = kPGPError_NoErr;
				break;
			}
			if( IsNull( signkeyobj ) )
				continue;
			if (!(revclass & 0x80))
				continue;
			if (!ringKeyIsSec (allset, signkeyobj))
				continue;
		}
		error = pgpCertifyObject (context, keyobj, addset, signkeyobj, allset, 
							 PGP_SIGTYPE_KEY_REVOKE, passphrase,
							 passphraseLength, hashedPhrase, FALSE,
							 SIG_EXPORTABLE, 0, 0, kPGPExpirationTime_Never,
							 0, 0, NULL, NULL, 0);
		/* Retry if bad passphrase and we are an authorized revoker */
		if (error != kPGPError_BadPassphrase || signkeyobj == keyobj)
			break;
	}

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
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey
};

PGPError
pgpRevokeKeyInternal(
	PGPKeyRef			key,
	PGPOptionListRef	optionList )
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPBoolean			hashedPhrase = FALSE;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );

	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						revkeyOptionSet, elemsof( revkeyOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_Passphrase, FALSE,
						"%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	err = sRevokeKey( context, key, passphrase, passphraseLength,
					  hashedPhrase );
error:
	return err;
}


static const PGPOptionType keyentOptionSet[] = {
	kPGPOptionType_KeyGenParams,
	kPGPOptionType_KeyGenFast,
	kPGPOptionType_KeyGenUseExistingEntropy
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
	PGPUInt32			noentropy = FALSE;
	PGPUInt32			pkalg;
	PGPUInt32			bits;
	PGPError			err = kPGPError_NoErr;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						keyentOptionSet, elemsof( keyentOptionSet ) ) ) )
		return err;

	/* If generating with existing entropy, we don't need any amount */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenUseExistingEntropy, FALSE,
						"%d", &noentropy ) ) )
		goto error;
	if (noentropy)
		return 0;
	
	pgpEnv = pgpContextGetEnvironment( context );
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenParams, TRUE,
						"%d%d", &pkalg, &bits ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenFast, FALSE,
						"%b%d", &fastgenop, &fastgen ) ) )
		goto error;
	if( !fastgenop ) {
		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN, NULL, NULL);
	}

	return pgpSecKeyEntropy (pgpPkalgByNumber ((PGPByte)pkalg), bits,
							(PGPBoolean)fastgen);

	/* Should not have an error unless bad parameters */
error:
	pgpAssert(0);
	return ~(PGPUInt32)0;
}


/* Internal function for passphraseIsValid */
	static PGPError
sPassphraseIsValid(
	PGPKeyRef		key,
	const char *	passphrase,
	PGPSize			passphraseLength,
	PGPBoolean		hashedPhrase,
	PGPBoolean *	isValid)
{
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
									passphraseLength, hashedPhrase);
	pgpSecKeyDestroy( seckey );
	if (err == (PGPError)1) {
		*isValid = TRUE;
		err = kPGPError_NoErr;
	}
	return err;
}

	

static const PGPOptionType passphraseisvalidOptionSet[] = {
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey
};

	PGPBoolean
pgpPassphraseIsValidInternal(
	PGPKeyRef			key,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPBoolean			hashedPhrase = FALSE;
	PGPBoolean			rslt;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	if ( ! pgpKeyIsValid( key ) )
		return( FALSE );
	
	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
								passphraseisvalidOptionSet,
								elemsof( passphraseisvalidOptionSet ) ) ) )
		return FALSE;

	/* Pick up mandatory options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		return FALSE;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, TRUE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			return FALSE;
	}

	if( IsPGPError( sPassphraseIsValid( key, passphrase, passphraseLength,
										hashedPhrase, &rslt ) ) )
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
	PGPTime			creationDate,
	PGPUInt16		expirationDays,
	char const *	name,
	int				name_len, 
	char const *	passphrase,
	PGPSize			passphraseLength,
	PGPBoolean		passphraseIsKey,
	char const *	masterpass, 
	PGPSize			masterpassLength,
	PGPEventHandlerProcPtr progress,
	PGPUserValue	userValue,
	PGPBoolean		fastgen,
	PGPBoolean		checkentropy,
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
	const PGPPkAlg *		algInfo;
	
	if ( !pgpKeyDBIsMutable( keyDB ) )
		return kPGPError_ItemIsReadOnly;

	algInfo = pgpPkalgByNumber( pkalg );
	if( IsntNull( algInfo ) )
	{
		if( ( pgpKeyUse( algInfo ) & PGP_PKUSE_SIGN ) == 0 &&
			genMaster )
		{
			pgpDebugMsg( "Invalid master key algorithm" );
			error = kPGPError_BadParams;
		}
		else if( ( pgpKeyUse( algInfo ) & PGP_PKUSE_ENCRYPT ) == 0 &&
				! genMaster )
		{
			pgpDebugMsg( "Invalid subkey algorithm" );
			error = kPGPError_BadParams;
		}
	}
	else
	{
		pgpDebugMsg( "Invaid public key algorithm" );
		error = kPGPError_BadParams;
	}
	
	if( IsPGPError( error ) )
		goto cleanup;
		
	pgpEnv = pgpContextGetEnvironment( keyDB->context );
	if( checkentropy )
	{
		/* Check we have sufficient random bits to generate the keypair */
		entropy_needed = pgpSecKeyEntropy (algInfo, bits, fastgen);
		entropy_available = PGPGlobalRandomPoolGetEntropy ( );
		if (entropy_needed > entropy_available)
		{
			error = kPGPError_OutOfEntropy;
			goto cleanup;
		}
	}
	
	/* Generate the secret key */
	progressState.progress = progress;
	progressState.userValue = userValue;
	progressState.context = keyDB->context;
	pgpRng = pgpContextGetX9_17RandomContext( keyDB->context );
	seckey = pgpSecKeyGenerate( context, algInfo, bits, fastgen, pgpRng, 
				genProgress, &progressState, &error);
	if (error)
		goto cleanup;
	pgpRandomStir (pgpRng);

	/* Need to lock the SecKey with the passphrase.  */
	if (passphrase && passphraseLength > 0) {
		PGPStringToKeyType s2ktype;
		if (passphraseIsKey) {
			s2ktype = kPGPStringToKey_LiteralShared;
		} else if (seckey->pkAlg <= kPGPPublicKeyAlgorithm_RSA + 2) {
			s2ktype = kPGPStringToKey_Simple;
		} else {
			s2ktype = kPGPStringToKey_IteratedSalted;
		}
		error = (PGPError)pgpSecKeyChangeLock (seckey, pgpEnv, pgpRng, 
									passphrase, passphraseLength,
									s2ktype);
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
	
	if( creationDate != 0 )
	{
		pgpKeySpecSetCreation(keyspec, creationDate +
				(60 * 60 * pgpenvGetInt(pgpEnv, PGPENV_TZFIX,  NULL, NULL)));
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
									rakset, rakclass,
									prefalgByte, prefalgLength,
								    adkset, adkclass,
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
		   if (IsNull( masterpass )) {
			   error = kPGPError_BadPassphrase;
			   goto cleanup;
			}
			error = (PGPError)pgpSecKeyUnlock (masterseckey, pgpEnv,
										masterpass, masterpassLength, FALSE);
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
	kPGPOptionType_Passkey,
	kPGPOptionType_Expiration,
	kPGPOptionType_CreationDate,
	kPGPOptionType_EventHandler,
	kPGPOptionType_PreferredAlgorithms,
	kPGPOptionType_AdditionalRecipientRequestKeySet,
	kPGPOptionType_RevocationKeySet,
	kPGPOptionType_KeyGenFast,
	kPGPOptionType_KeyGenUseExistingEntropy
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
	PGPTime				creationDate;
	PGPByte				*name;
	PGPUInt32			nameLength;
	PGPByte				*passphrase;
	PGPUInt32			passphraseLength;
	PGPBoolean			passphraseIsKey = FALSE;
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
	PGPUInt32			noentropy = FALSE;
	PGPError			err;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						keygenOptionSet, elemsof( keygenOptionSet ) ) ) )
		return err;

	if( IsNull( key ) )
		return kPGPError_BadParams;

	pgpEnv = pgpContextGetEnvironment( context );

	/* First pick up mandatory options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeySetRef, TRUE,
						"%p", &keyset ) ) )
		goto error;
	
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenParams, TRUE,
						"%d%d", &pkalg, &bits ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenName, TRUE,
						"%p%l", &name, &nameLength ) ) )
		goto error;

	/* Now get optional parameters */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_Passphrase, FALSE,
						"%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
		if( IsntNull( passphrase ) )
			passphraseIsKey = TRUE;
	}

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_CreationDate, FALSE,
						"%T", &creationDate ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_Expiration, FALSE,
						"%d", &expiration ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_EventHandler, FALSE,
						"%p%p", &progress, &userValue ) ) )
		goto error;
	
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_PreferredAlgorithms, FALSE,
						"%p%l", &prefalg, &prefalgLength ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_AdditionalRecipientRequestKeySet,
						FALSE, "%p%d", &adkset, &adkclass ) ) )
		goto error;
	if( IsntNull( adkset ) ) {
		if( IsPGPError( err = pgpKeySetRingSet( adkset, TRUE, &adkringset ) ) )
			goto error;
	}

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_RevocationKeySet, FALSE,
						"%p%d", &rakset, &rakclass ) ) )
		goto error;
	if( IsntNull( rakset ) ) {
		if( IsPGPError( err = pgpKeySetRingSet( rakset, TRUE, &rakringset ) ) )
			goto error;
	}

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenFast, FALSE,
						"%b%d", &fastgenop, &fastgen ) ) )
		goto error;
	if( !fastgenop ) {
		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN, NULL, NULL);
	}

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenUseExistingEntropy, FALSE,
						"%d", &noentropy ) ) )
		goto error;
	
	*key = NULL;
	newkey = NULL;		/* Necessary to flag masterkey vs subkey */
	err = pgpDoGenerateKey( keyset->keyDB, &newkey, NULL, (PGPByte)pkalg,
							  bits, creationDate, (PGPUInt16)expiration,
							  (const char *)name, nameLength,
							  (const char *)passphrase, passphraseLength,
							  passphraseIsKey, NULL, 0,
							  progress, userValue,
							  (PGPBoolean)fastgen, (PGPBoolean)!noentropy,
							  adkringset, (PGPByte)adkclass,
							  rakringset, (PGPByte)rakclass,
							   prefalg, prefalgLength );
	
	if( IsntPGPError( err ) )
	    *key = newkey;

error:
	if( IsntNull( adkringset ) )
		ringSetDestroy( (RingSet *) adkringset );
	if( IsntNull( rakringset ) )
		ringSetDestroy( (RingSet *) rakringset );
	return err;
}

static const PGPOptionType subkeygenOptionSet[] = {
	 kPGPOptionType_KeyGenMasterKey,
	 kPGPOptionType_KeyGenParams,
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Passkey,
	 kPGPOptionType_Expiration,
	 kPGPOptionType_CreationDate,
	 kPGPOptionType_EventHandler,
	 kPGPOptionType_KeyGenFast,
	 kPGPOptionType_KeyGenUseExistingEntropy
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
	PGPTime				creationDate;
	PGPUInt32			expiration;
	PGPByte				*passphrase;
	PGPUInt32			passphraseLength;
	PGPBoolean			passphraseIsKey = FALSE;
	PGPEventHandlerProcPtr progress;
	PGPUserValue		userValue;
	PGPKeyRef			masterkey;
	PGPSubKeyRef		newsubkey;
	PGPEnv				*pgpEnv;
	PGPBoolean			fastgenop;
	PGPUInt32			fastgen;
	PGPUInt32			noentropy = FALSE;
	PGPError			err;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
					   subkeygenOptionSet, elemsof( subkeygenOptionSet ) ) ) )
		return err;

	if( IsNull( subkey ) )
		return kPGPError_BadParams;

	pgpEnv = pgpContextGetEnvironment( context );

	/* First pick up mandatory options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_KeyGenMasterKey, TRUE,
						 "%p", &masterkey ) ) )
		goto error;
	
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_KeyGenParams, TRUE,
						 "%d%d", &pkalg, &bits ) ) )
		goto error;


	/* Now get optional parameters */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
		if( IsntNull( passphrase ) )
			passphraseIsKey = TRUE;
	}

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_CreationDate, FALSE,
						 "%T", &creationDate ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Expiration, FALSE,
						 "%d", &expiration ) ) )
		goto error;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_EventHandler, FALSE,
						 "%p%p", &progress, &userValue ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_KeyGenFast, FALSE,
						 "%b%d", &fastgenop, &fastgen ) ) )
		goto error;
	if( !fastgenop ) {
		fastgen = pgpenvGetInt (pgpEnv, PGPENV_FASTKEYGEN, NULL, NULL);
	}
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_KeyGenUseExistingEntropy, FALSE,
						"%d", &noentropy ) ) )
		goto error;
	
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
							   (PGPByte)pkalg, bits, creationDate,
							   (PGPUInt16)expiration,
							   NULL, 0, (char const *)passphrase,
							   passphraseLength, passphraseIsKey,
							   (char const *)passphrase, passphraseLength,
							   progress, userValue,
							   (PGPBoolean)fastgen, (PGPBoolean)!noentropy,
							   NULL, (PGPByte)0, NULL, (PGPByte)0, NULL, 0);
	if( IsntPGPError( err ) )
	    *subkey = newsubkey;

error:
	return err;
}


/*  Handle editing key properties which are held in self signatures  */


static const PGPOptionType keyoptionOptionSet[] = {
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey,
	kPGPOptionType_RevocationKeySet,
	kPGPOptionType_PreferredAlgorithms,
#if 0
/* not yet implemented */
	kPGPOptionType_Expiration,
	kPGPOptionType_AdditionalRecipientRequestKeySet,
#endif
};


	PGPError
pgpAddKeyOptionsInternal (
	PGPKeyRef			key,
	PGPOptionListRef	optionList
	)
{
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPBoolean			hashedPhrase = FALSE;
	PGPKeySetRef		rakset = NULL;
	PGPUInt32			rakclass = 0;
	RingSet const		*rakringset = NULL;
	RingSet				*addset = NULL;
	RingSet				*rak1set = NULL;
	RingIterator		*rakiter = NULL;
	PGPKeyDB			*keys;
	RingObject			*keyobj;
	RingSet const		*allset;
	PGPContextRef		context;
	PGPError			err = kPGPError_NoErr;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						keyoptionOptionSet, elemsof( keyoptionOptionSet ) ) ) )
		goto error;

	/* Pick up passphrase options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	/* Get data to add (require revocationkeyset for now) */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_RevocationKeySet, TRUE,
						"%p%d", &rakset, &rakclass ) ) )
		goto error;
	pgpAssert( IsntNull( rakset ) );

	if( IsPGPError( err = pgpKeySetRingSet( rakset, TRUE, &rakringset ) ) ) {
		goto error;
	}


	/*
	 * This code is temporary and will be redesigned to support a wider
	 * set of key options.
	 */

	keys =		key->keyDB;
	keyobj =	key->key;
	context = 	keys->context;

	if ( !keys->objIsMutable( keys, keyobj ) ) {
		err = kPGPError_ItemIsReadOnly;
		goto error;
	}
	if ( IsPGPError( pgpKeyDeadCheck(key) ) ) {
		goto error;	/* no need if already revoked */
	}
	
	allset = pgpKeyDBRingSet (keys);

	if( IsPGPError( err = pgpCopyKey (allset, keyobj, &addset) ) )
		goto error;

		
	rakiter = ringIterCreate(rakringset);
	if( IsNull( rakiter ) ) {
		err = ringSetError(rakringset)->error;
		goto error;
	}

	/* Add 1 RAK key at a time in separate self signatures */
	while (ringIterNextObject (rakiter, 1) == 1) {
		RingObject *rakkey = ringIterCurrentObject (rakiter, 1);
		pgpAssert (ringObjectType(rakkey) == RINGTYPE_KEY);
		rak1set = ringSetCreate (ringSetPool(rakringset));
		if( IsNull( rak1set ) ) {
			err = ringSetError(rakringset)->error;
			goto error;
		}
		ringSetAddObject (rak1set, rakkey);
		ringSetFreeze (rak1set);

		err = pgpCertifyObject (context, keyobj, addset, keyobj, allset, 
								PGP_SIGTYPE_KEY_PROPERTY, passphrase,
								passphraseLength, hashedPhrase, FALSE,
								SIG_EXPORTABLE, 0,
								0, kPGPExpirationTime_Never,
								0, 0, NULL, rak1set, rakclass);
		if( IsPGPError( err ) ) {
			goto error;
		}
		ringSetDestroy (rak1set);
		rak1set = NULL;
	}

	/*  Update the KeyDB */
	err = pgpAddObjects (keys, addset);

	/* Calculate trust changes as a result */
	if( err == kPGPError_NoErr )
		(void)pgpPropagateTrustKeyDB (keys);

error:

	if( IsntNull( addset ) )
		ringSetDestroy (addset);
	if( IsntNull( rakringset ) )
		ringSetDestroy ((RingSet *)rakringset);
	if( IsntNull( rakiter ) )
		ringIterDestroy (rakiter);
	if( IsntNull( rak1set ) )
		ringSetDestroy (rak1set);

	return err;
}

	PGPError
pgpRemoveKeyOptionsInternal (
	PGPKeyRef			key,
	PGPOptionListRef	optionList
	)
{
	(void) key;
	(void) optionList;
	return kPGPError_FeatureNotAvailable;
}

	PGPError
pgpUpdateKeyOptionsInternal (
	PGPKeyRef			key,
	PGPOptionListRef	optionList
	)
{
	PGPKeyDB		  *keys;
	RingSet const 	  *allset;
	RingSet			  *addset = NULL;
	PGPEnv			  *pgpEnv;
	PGPRandomContext  *pgpRng;
	PGPSecKey		  *seckey = NULL;
	PGPSigSpec		  *sigspec = NULL;
	int				   tzFix;
	PGPTime			   timestamp;
	char *			   passphrase;
	PGPSize			   passphraseLength;
	PGPBoolean		   hashedPhrase = FALSE;
	RingObject		  *keyobj;
	PGPContextRef	   context;
	PGPCipherAlgorithm	*prefalg;
	PGPSize				prefalgLength;
	PGPByte			   *prefalgByte = NULL;
	PGPUserIDRef		userid;
	RingObject		   *latestsig;
	PGPError			err = kPGPError_NoErr;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						keyoptionOptionSet, elemsof( keyoptionOptionSet ) ) ) )
		goto error;

	/* Pick up passphrase options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	/* Get data to modify (require preferred algs  for now) */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						kPGPOptionType_PreferredAlgorithms, TRUE,
						"%p%l", &prefalg, &prefalgLength ) ) )
		goto error;
	if( IsNull( prefalg ) )
		goto error;		/* Nothing to do */

	/* Prepare for action */
	keys   = key->keyDB;
	keyobj = key->key;
	context = keys->context;
	pgpRng = pgpContextGetX9_17RandomContext( context );
	pgpEnv = pgpContextGetEnvironment( context );
	tzFix  = pgpenvGetInt (pgpEnv, PGPENV_TZFIX, NULL, NULL);
	timestamp = pgpTimeStamp (tzFix);
	allset = pgpKeyDBRingSet (keys);

	/* Parse preferred algorithms into tight buffer */
	prefalgByte = NULL;
	if (prefalgLength > 0) {
		PGPUInt32 i;
		/* Convert preferred algorithm to byte array */
		prefalgLength /= sizeof(PGPCipherAlgorithm);
		prefalgByte = (PGPByte *)pgpContextMemAlloc( context,
													prefalgLength, 0);
		if( IsNull( prefalgByte ) ) {
			err = kPGPError_OutOfMemory;
			goto error;
		}
		for (i=0; i<prefalgLength; ++i) {
			prefalgByte[i] = (PGPByte)prefalg[i];
		}
	}

	/* Try to unlock secret key object */
    seckey = ringSecSecKey (allset, keyobj, PGP_PKUSE_SIGN);
    if (!seckey) {
	    err = ringSetError(allset)->error;
		goto error;
	}
    if (pgpSecKeyIslocked (seckey)) {
	    if (IsNull( passphrase )) {
			err = kPGPError_BadPassphrase;
			goto error;
		}
	    err = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, passphrase, 
								 passphraseLength, hashedPhrase);
		if (err != 1)
		{
			if (err == 0)
			    err = kPGPError_BadPassphrase;
			goto error;
	    }
    }

	if ( !keys->objIsMutable( keys, keyobj ) ) {
		err = kPGPError_ItemIsReadOnly;
		goto error;
	}
	if ( IsPGPError( pgpKeyDeadCheck(key) ) ) {
		goto error;	/* no need if already revoked */
	}
	
	if( IsPGPError( err = pgpCopyKey (allset, keyobj, &addset) ) )
		goto error;

	/* Update each self-sig on each name */
	for (userid = (PGPUserID *) key->userIDs.next; 
		 userid != (PGPUserID *) &key->userIDs;
		 userid = userid->next) {
	    if (userid->removed)
			continue;
	
		latestsig = ringLatestSigByKey( userid->userID, addset, keyobj );
		if( IsntNull( latestsig ) )
		{
			/* Revoke existing cert */
			sigspec = pgpSigSpecCreate (pgpEnv, seckey,
										PGP_SIGTYPE_KEY_UID_REVOKE);
			pgpSigSpecSetTimestamp( sigspec, timestamp );
			err = ringSignObject (addset, userid->userID, sigspec, pgpRng);
			pgpSigSpecDestroy (sigspec);
			sigspec = NULL;

			/* Copy existing sigspec, set new timestamp */
			sigspec = ringSigSigSpec (latestsig, addset, &err);
			if( IsNull( sigspec ) )
				continue;
			pgpSecKeyDestroy( pgpSigSpecSeckey( sigspec ) );
			pgpSigSpecSetSeckey( sigspec, seckey );
			pgpSigSpecSetTimestamp( sigspec, timestamp+1 );

		}
		else
		{
			/* No previous sig, must create one */
			sigspec = pgpSigSpecCreate (pgpEnv, seckey,
										PGP_SIGTYPE_KEY_GENERIC);
			if( IsNull( sigspec ) )
				continue;
			pgpSigSpecSetTimestamp( sigspec, timestamp+1 );
		}

		/* Set new values */
		pgpSigSpecSetPrefAlgs (sigspec, 0, prefalgByte, prefalgLength);

		/* Issue new signature */
		err = ringSignObject (addset, userid->userID, sigspec, pgpRng);
		pgpSigSpecDestroy (sigspec);
		sigspec = NULL;

	}

	/*  Update the KeyDB */
	err = pgpAddObjects (keys, addset);

	/* Calculate trust changes as a result */
	if( err == kPGPError_NoErr )
		(void)pgpPropagateTrustKeyDB (keys);

error:

	if( IsntNull( addset ) )
		ringSetDestroy (addset);
	if( IsntNull( sigspec ) )
		pgpSigSpecDestroy (sigspec);
	if( IsntNull( prefalgByte ) )
		PGPFreeData (prefalgByte);
	if( IsntNull( seckey ) )
		pgpSecKeyDestroy (seckey);


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

PGPError
pgpDoChangePassphraseInternal (PGPKeyDB *keyDB, RingSet const *ringset, 
							 RingObject *keyobj, RingObject *masterkeyobj, 
							 const char *oldphrase, PGPSize oldphraseLength,
							 const char *newphrase, PGPSize newphraseLength,
							 PGPBoolean newPassphraseIsKey)
{
	unsigned long		 validity;
	RingSet				*addset = NULL;
	union RingObject	*newsecobj, *oldsecobj = NULL;
	PGPKeySpec			*keyspec = NULL;
	PGPError			 error = kPGPError_NoErr;
	PGPSecKey			*seckey = NULL;
	PgpVersion			 version;
	PGPEnv				*pgpEnv;
	PGPEnv				*pgpEnvCopy = NULL;
	PGPRandomContext	*pgpRng;
	PGPStringToKeyType	 s2ktype;
	PGPByte				*prefAlgs;
	PGPSize				 prefAlgsLength;
	PGPUInt32			 i;
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
	oldsecobj = ringBestSec (ringset, keyobj);
	if( IsNull( oldsecobj ) )
	    return kPGPError_SecretKeyNotFound;

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
								 oldphraseLength, FALSE);
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

	if (newPassphraseIsKey) {
		s2ktype = kPGPStringToKey_LiteralShared;
	} else if (seckey->pkAlg <= kPGPPublicKeyAlgorithm_RSA + 2) {
		s2ktype = kPGPStringToKey_Simple;
	} else {
		s2ktype = kPGPStringToKey_IteratedSalted;
	}

	/* Lock using key's preferred algorithm if known */
	if( IsPGPError( error = pgpenvCopy( pgpEnv, &pgpEnvCopy ) ) )
		goto cleanup;
	prefAlgs = (PGPByte *)ringKeyFindSubpacket (
			(masterkeyobj?masterkeyobj:keyobj), ringset,
			SIGSUB_PREFERRED_ENCRYPTION_ALGS, 0,
			&prefAlgsLength, NULL, NULL, NULL, NULL, &error);
	for( i = 0; i < prefAlgsLength; ++i ) {
		PGPCipherAlgorithm lockAlg = (PGPCipherAlgorithm)prefAlgs[i];
		if( IsntNull( pgpCipherGetVTBL ( lockAlg ) ) ) {
			pgpenvSetInt (pgpEnvCopy, PGPENV_CIPHER, lockAlg,
						  PGPENV_PRI_FORCE);
			break;
		}
	}

    error = (PGPError)pgpSecKeyChangeLock (seckey, pgpEnvCopy, pgpRng, 
								 newphrase, newphraseLength, s2ktype);
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
	if (pgpEnvCopy)
		pgpenvDestroy (pgpEnvCopy);
	return error;
}


static const PGPOptionType changepassphraseOptionSet[] = {
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey
};

PGPError
pgpChangePassphraseInternal(
	PGPKeyRef			key,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	PGPOption			oldOp, newOp;
	RingSet	const		*ringset;
	void *				oldPassphrase;
	void *				newPassphrase;
	PGPSize				oldPassphraseLength;
	PGPSize				newPassphraseLength;
	PGPBoolean			oldWasPasskey;
	PGPBoolean			newWasPasskey;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );
	
	oldWasPasskey = FALSE;
	newWasPasskey = FALSE;

	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
									changepassphraseOptionSet,
									elemsof( changepassphraseOptionSet ) ) ) )
		goto error;

	/*
	 * Can't use our regular parsing functions because we are allowing
	 * the same option to be used more than once.
	 */

	/* Pick up old passphrase */
	if( IsPGPError( err = pgpGetIndexedOption( optionList,
					  0, TRUE, &oldOp ) ) )
		goto error;
	
	oldWasPasskey = oldOp.type == kPGPOptionType_Passkey;

	if( !oldWasPasskey && oldOp.type != kPGPOptionType_Passphrase )
	{
		err = kPGPError_BadParams;
		goto error;
	}
		
	/* Pick up new passphrase */
	if( IsPGPError( err = pgpGetIndexedOption( optionList,
					  1, TRUE, &newOp ) ) )
		goto error;
	
	newWasPasskey = newOp.type == kPGPOptionType_Passkey;

	if( !newWasPasskey && newOp.type != kPGPOptionType_Passphrase )
	{
		err = kPGPError_BadParams;
		goto error;
	}
		
	pgpOptionPtrLength( &oldOp, &oldPassphrase, &oldPassphraseLength );
	pgpOptionPtrLength( &newOp, &newPassphrase, &newPassphraseLength );

	ringset = pgpKeyDBRingSet (key->keyDB);
	err = pgpDoChangePassphraseInternal (key->keyDB, ringset, key->key, NULL,
							 (char const *)oldPassphrase, oldPassphraseLength,
							 (char const *)newPassphrase, newPassphraseLength,
							 newWasPasskey);
	if (!err) {
		/* Ringset for keydb may be changed by above call */
		ringset = pgpKeyDBRingSet (key->keyDB);
	}

error:

	return err;
}


PGPError
pgpChangeSubKeyPassphraseInternal(
	PGPSubKeyRef		subkey,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	PGPOption			oldOp, newOp;
	RingSet	const		*ringset;
	void *				oldPassphrase;
	void *				newPassphrase;
	PGPSize				oldPassphraseLength;
	PGPSize				newPassphraseLength;
	PGPBoolean			oldWasPasskey;
	PGPBoolean			newWasPasskey;
	PGPError			err = kPGPError_NoErr;

	PGPValidateSubKey( subkey );
	
	oldWasPasskey = FALSE;
	newWasPasskey = FALSE;

	context = subkey->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
									changepassphraseOptionSet,
									elemsof( changepassphraseOptionSet ) ) ) )
		goto error;

	/*
	 * Can't use our regular parsing functions because we are allowing
	 * the same option to be used more than once.
	 */

	/*
	 * Can't use our regular parsing functions because we are allowing
	 * the same option to be used more than once.
	 */

	/* Pick up old passphrase */
	if( IsPGPError( err = pgpGetIndexedOption( optionList,
					  0, TRUE, &oldOp ) ) )
		goto error;
	
	oldWasPasskey = oldOp.type == kPGPOptionType_Passkey;

	if( !oldWasPasskey && oldOp.type != kPGPOptionType_Passphrase )
	{
		err = kPGPError_BadParams;
		goto error;
	}
		
	/* Pick up new passphrase */
	if( IsPGPError( err = pgpGetIndexedOption( optionList,
					  1, TRUE, &newOp ) ) )
		goto error;
	
	newWasPasskey = newOp.type == kPGPOptionType_Passkey;

	if( !newWasPasskey && newOp.type != kPGPOptionType_Passphrase )
	{
		err = kPGPError_BadParams;
		goto error;
	}
		
	pgpOptionPtrLength( &oldOp, &oldPassphrase, &oldPassphraseLength );
	pgpOptionPtrLength( &newOp, &newPassphrase, &newPassphraseLength );

	CHECKREMOVED(subkey);
	ringset = pgpKeyDBRingSet (subkey->key->keyDB);

	err = pgpDoChangePassphraseInternal (subkey->key->keyDB, ringset,
						 subkey->subKey, subkey->key->key,
						 (char const *)oldPassphrase, oldPassphraseLength,
						 (char const *)newPassphrase, newPassphraseLength,
						 newWasPasskey);
error:

	return err;
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
	PGPBoolean			hashedPhrase
	)
{
    PGPKeyDB			*keys;
	RingSet const		*allset;
	RingSet *			addset;
	union RingObject    *subkeyobj, *keyobj;
	union RingObject	*signkeyobj = NULL;
	PGPUInt32			 revnum;
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

	error = pgpCopyKey (allset, keyobj, &addset);
	if (error)
		return error;

	revnum = 0;
	for ( ; ; ) {
		signkeyobj = keyobj;
		/* See if we have an authorized revocation signature */
		if (!ringKeyIsSec (allset, keyobj)) {
			PGPByte revclass;
			signkeyobj = ringKeyRevocationKey (keyobj, allset, revnum++,
											   NULL, NULL,
											   &revclass, NULL, &error);
			if( IsPGPError( error ) ) {
				if( error == kPGPError_ItemNotFound )
					error = kPGPError_NoErr;
				break;
			}
			if( IsNull( signkeyobj ) ) {
				continue;
			}
			if (!(revclass & 0x80))
				continue;
			if (!ringKeyIsSec (allset, signkeyobj))
				continue;
		}
		/*  Note special subkey revocation sigtype */
		error = pgpCertifyObject (context, subkeyobj, addset, signkeyobj,
					allset, PGP_SIGTYPE_KEY_SUBKEY_REVOKE, passphrase,
					passphraseLength, hashedPhrase, FALSE,
					SIG_EXPORTABLE, 0, 0, kPGPExpirationTime_Never, 0, 0, NULL,
					NULL, 0);
		/* Retry if bad passphrase and we are an authorized revoker */
		if (error != kPGPError_BadPassphrase || signkeyobj == keyobj)
			break;
	}
			
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
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Passkey
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
	PGPBoolean			hashedPhrase = FALSE;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPSubKeyValid(subkey));
	PGPValidateSubKey( subkey );

	context = subkey->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
					   revsubkeyOptionSet, elemsof( revsubkeyOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	err = sRevokeSubKey( context, subkey, passphrase, passphraseLength,
						 hashedPhrase );
error:
	return err;
}


/*  Convert a passphrase to a passkeybuffer, for a given key */

static const PGPOptionType getpasskeyOptionSet[] = {
	kPGPOptionType_Passphrase,
};

PGPError
pgpGetKeyPasskeyBufferInternal (
	PGPKeyRef			key,
	void			   *passkeyBuffer,
	PGPOptionListRef	optionList
	)
{
	RingObject		   *keyObj;
	RingSet const	   *ringSet;
	PGPEnv			   *pgpEnv;
	PGPSecKey		   *secKey;
	PGPError			err;
	char const		   *passphrase;
	PGPSize				passphraseLength;
	
	/* Pick up mandatory passphrase option */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, TRUE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		return err;

	keyObj = key->key;
	ringSet = pgpKeyDBRingSet( key->keyDB );
	pgpEnv = pgpContextGetEnvironment( key->keyDB->context );
	secKey = ringSecSecKey( ringSet, keyObj, 0 );
	if( IsNull( secKey ) )
		return ringSetError(ringSet)->error;
	err =  pgpSecKeyConvertPassphrase( secKey, pgpEnv, passphrase,
						  passphraseLength, (PGPByte *) passkeyBuffer );
	pgpSecKeyDestroy (secKey);

	return err;
}

/*  Convert a passphrase to a passkeybuffer, for a given subkey */

PGPError
pgpGetSubKeyPasskeyBufferInternal (
	PGPSubKeyRef		subkey,
	void			   *passkeyBuffer,
	PGPOptionListRef	optionList
	)
{
	RingObject		   *keyObj;
	RingSet const	   *ringSet;
	PGPEnv			   *pgpEnv;
	PGPSecKey		   *secKey;
	PGPError			err;
	char const		   *passphrase;
	PGPSize				passphraseLength;
	
	/* Pick up mandatory passphrase option */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, TRUE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		return err;

	keyObj = subkey->subKey;
	ringSet = pgpKeyDBRingSet( subkey->key->keyDB );
	pgpEnv = pgpContextGetEnvironment( subkey->key->keyDB->context );
	secKey = ringSecSecKey( ringSet, keyObj, 0 );
	if( IsNull( secKey ) )
		return ringSetError(ringSet)->error;
	err =  pgpSecKeyConvertPassphrase( secKey, pgpEnv, passphrase,
						passphraseLength, (PGPByte *) passkeyBuffer );
	pgpSecKeyDestroy (secKey);

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
	PGPBoolean		isAttribute,
	PGPAttributeType attributeType,
	char const *	userIDData,
	PGPSize		   	userIDLength,
	char const *	passphrase,
	PGPSize			passphraseLength,
	PGPBoolean		hashedPhrase
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
	if (isAttribute)
		nameobj = ringCreateAttribute (addset, keyobj, (PGPByte)attributeType,
									   (PGPByte *)userIDData, userIDLength);
	else
		nameobj = ringCreateName (addset, keyobj, userIDData, userIDLength);
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
							  passphraseLength, hashedPhrase, TRUE,
							  SIG_EXPORTABLE, 0, 0, kPGPExpirationTime_Never,
							  0, 0, NULL, NULL, 0);
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
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey
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
	PGPBoolean			hashedPhrase = FALSE;
	PGPSize				userIDLength;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );
	
	if( IsNull( userID ) )
		return kPGPError_BadParams;

	userIDLength = strlen(userID);

	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						adduserOptionSet, elemsof( adduserOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	err = sAddUserID( context, key, FALSE, (PGPAttributeType)0,
					  userID, userIDLength, passphrase, passphraseLength,
					  hashedPhrase);
error:
	return err;
}


static const PGPOptionType addattrOptionSet[] = {
	kPGPOptionType_Passphrase,
	kPGPOptionType_Passkey
};

PGPError
pgpAddAttributeInternal(
	PGPKeyRef			key,
	PGPAttributeType	attributeType,
	PGPByte	const	   *attributeData,
	PGPSize				attributeLength,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPBoolean			hashedPhrase = FALSE;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );
	
	if( IsNull( attributeData ) )
		return kPGPError_BadParams;

	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						adduserOptionSet, elemsof( adduserOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	err = sAddUserID( context, key, TRUE, attributeType,
					  (const char *)attributeData, attributeLength,
					  passphrase, passphraseLength, hashedPhrase);
error:
	return err;
}


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

/*	Make the given User ID the primary User ID of the key */
/*	This version uses OpenPGP Primary UserID signature subpackets */

	static PGPError
sSetPrimaryUserID (PGPContextRef context, PGPUserID *userid,
	char *passphrase, PGPSize passphraseLength, PGPBoolean hashedPhrase )
{
	PGPKeyDB		  *keys;
	PGPKey			  *key;
	RingSet const 	  *allset;
	RingSet			  *addset;
	PGPEnv			  *pgpEnv;
	PGPRandomContext  *pgpRng;
	PGPSecKey		  *seckey;
	PGPUserID		  *otherID;
	RingObject		  *bestsig;
	PGPSigSpec		  *sigspec;
	PGPBoolean		   wasprimary;
	PGPError		   error;
	int				   tzFix;

	pgpRng = pgpContextGetX9_17RandomContext( context );
	pgpEnv = pgpContextGetEnvironment( context );
	tzFix  = pgpenvGetInt (pgpEnv, PGPENV_TZFIX, NULL, NULL);

	key = userid->key;
	keys = key->keyDB;
	if (!keys->objIsMutable(keys, key->key))
		return kPGPError_ItemIsReadOnly;
	
	allset = pgpKeyDBRingSet (keys);
	
    seckey = ringSecSecKey (allset, key->key, PGP_PKUSE_SIGN);
    if (!seckey)
	    return ringSetError(allset)->error;
    if (pgpSecKeyIslocked (seckey)) {
	    if (IsNull( passphrase )) {
		    pgpSecKeyDestroy (seckey);
			return kPGPError_BadPassphrase;
		}
	    error = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, passphrase, 
								 passphraseLength, hashedPhrase);
		if (error != 1)
		{
	        pgpSecKeyDestroy (seckey);
			if (error == 0)
			    error = kPGPError_BadPassphrase;
			return error;
	    }
    }

	error = pgpCopyKey (allset, key->key, &addset);
	if (error) {
		pgpSecKeyDestroy (seckey);
		ringSetDestroy (addset);
		return error;
	}
	
	/*
	 * For each name, if other than the selected one, remove any primary
	 * userid subpackets.  For selected one, add one.
	 */
	otherID = (PGPUserID *) &key->userIDs;
	for ( ; ; ) 
	{
		otherID = otherID->next;
		if (otherID == (PGPUserID *)&key->userIDs)
		{
			break;
		}
		if( otherID->removed )
			continue;
		if( otherID == userid )
		{
			/* Name which is becoming primary */
			bestsig = ringLatestSigByKey (otherID->userID, addset, key->key);
			if( IsntNull( bestsig ) )
			{
				sigspec = ringSigSigSpec (bestsig, addset, &error);
				if( IsNull( sigspec ) )
					continue;
				pgpSecKeyDestroy( pgpSigSpecSeckey( sigspec ) );
				pgpSigSpecSetSeckey( sigspec, seckey );
				pgpSigSpecSetTimestamp( sigspec, pgpTimeStamp (tzFix) );
			}
			else
			{
				/* No previous sig, must create one */
				sigspec = pgpSigSpecCreate (pgpEnv, seckey,
											PGP_SIGTYPE_KEY_GENERIC);
				if( IsNull( sigspec ) )
					continue;
			}
			pgpSigSpecSetPrimaryUserID (sigspec, 0, TRUE);
			error = ringSignObject (addset, otherID->userID, sigspec, pgpRng);
			pgpSigSpecDestroy (sigspec);
			if( IsntNull( bestsig ) )
			{
				if ( IsPGPError( error = pgpRemoveObject (keys, bestsig) ) )
				{
					pgpSecKeyDestroy( seckey );
					ringSetDestroy (addset);
					return error;
				}
				ringSetRemObject (addset, bestsig);
			}
		}
		else
		{
			bestsig = ringLatestSigByKey (otherID->userID, addset, key->key);
			if( IsNull( bestsig ) )
				continue;
			sigspec = ringSigSigSpec (bestsig, addset, &error);
			if( IsNull( sigspec ) )
				continue;
			pgpSigSpecPrimaryUserID (sigspec, &wasprimary);
			if( !wasprimary )
			{
				pgpSecKeyDestroy( pgpSigSpecSeckey( sigspec ) );
				pgpSigSpecDestroy (sigspec);
				continue;
			}
			pgpSecKeyDestroy( pgpSigSpecSeckey( sigspec ) );
			pgpSigSpecSetSeckey( sigspec, seckey );
			pgpSigSpecSetTimestamp( sigspec, pgpTimeStamp (tzFix) );
			pgpSigSpecSetPrimaryUserID (sigspec, 0, FALSE);
			error = ringSignObject (addset, otherID->userID, sigspec, pgpRng);
			pgpSigSpecDestroy (sigspec);
			if( IsPGPError( error = pgpRemoveObject (keys, bestsig) ) )
			{
				pgpSecKeyDestroy( seckey );
				ringSetDestroy (addset);
				return error;
			}
			ringSetRemObject (addset, bestsig);
		}
	}

	pgpSecKeyDestroy( seckey );

	/* Put new objects back into keydb (may change allset) */
	error = pgpAddObjects (keys, addset);
	ringSetDestroy (addset);
	allset = pgpKeyDBRingSet (keys);

	/* Raise the name to the top */
    ringRaiseName ((RingSet *)allset, userid->userID);
    
    /* Rearrange the circularly-linked list of userids */
    userid->prev->next = userid->next;
    userid->next->prev = userid->prev;
    userid->prev = (PGPUserID *)&key->userIDs;
    userid->next = (PGPUserID *)key->userIDs.next;
    userid->next->prev = userid;
    userid->prev->next = userid;
	
	return error;

}


static const PGPOptionType setprimaryOptionSet[] = {
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Passkey,
};

	PGPError
pgpCertifyPrimaryUserIDInternal (
	PGPUserID *			userid,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPBoolean			hashedPhrase = FALSE;
	PGPError			err = kPGPError_NoErr;

	PGPValidateUserID( userid );
	
	CHECKREMOVED(userid);
	err = pgpKeyDeadCheck( userid->key );
	if ( IsPGPError( err ) )
		goto error;

	context = userid->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
					setprimaryOptionSet, elemsof( setprimaryOptionSet ) ) ) )
		return err;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	err = sSetPrimaryUserID( context, userid,
							 passphrase, passphraseLength, hashedPhrase );

error:
	return err;
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
	PGPBoolean		hashedPhrase,
	PGPBoolean		exportable,
	PGPTime			creationDate,
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

	pgpAssert( IsNull( certifying_key ) || pgpKeyIsValid(certifying_key) );

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
					  passphraseLength, hashedPhrase, FALSE,
					  exportable, SIG_EXPORTABLEHASHED,
					  creationDate, expiration,
					  trustDepth, trustValue, sRegExp, NULL, 0);
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
	 kPGPOptionType_Passkey,
	 kPGPOptionType_Expiration,
	 kPGPOptionType_CreationDate,
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
	PGPTime				creationDate;
	PGPUInt32			expiration;
	PGPUInt32			exportable;
	PGPUInt32			trustDepth;
	PGPUInt32			trustLevel;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPBoolean			hashedPhrase = FALSE;
	char const *		sRegExp;
	PGPError			err = kPGPError_NoErr;

	context = userid->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						signuserOptionSet, elemsof( signuserOptionSet ) ) ) )
		return err;

	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}
		
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_CreationDate, FALSE,
						 "%T", &creationDate ) ) )
		goto error;
		
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Expiration, FALSE,
						 "%d", &expiration ) ) )
		goto error;
		
	/* Defaults exportable to false */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Exportable, FALSE,
						 "%d", &exportable ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_CertificateTrust, FALSE,
						 "%d%d", &trustDepth, &trustLevel ) ) )
		goto error;
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
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

	err = sCertifyUserID( context, userid, certifying_key,
						  passphrase, passphraseLength, hashedPhrase,
						  (PGPBoolean)exportable, creationDate, expiration,
						  (PGPByte) trustDepth, (PGPByte) trustLevel,
						  sRegExp);

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
		
		if (algTemp == 0)
			pubKeyAlg	= kPGPPublicKeyAlgorithm_Invalid;
		else
			pubKeyAlg	= (PGPPublicKeyAlgorithm)algTemp;
		
		err	= PGPGetKeyByKeyID(allkeys, &keyID, pubKeyAlg, certkey );
		/* PGPGetKeyByKeyID incremented the ref count */
	}
	
	pgpAssertErrWithPtr( err, *certkey );
	return err;
}


/*  Given an X.509 sig, return the certifying sig object. */

PGPError
PGPGetSigX509CertifierSig (
	PGPSigRef		cert,
	PGPKeySetRef	allkeys,
	PGPSig **		certsig)
{
	PGPContextRef			context;
	PGPBoolean				isX509;
	PGPKeyRef				certkey;
	PGPUserIDRef			userid;
	PGPSigRef				sig;
	PGPByte				   *issuername = NULL;
	PGPSize					issuernamelen;
	PGPByte				   *signame = NULL;
	PGPSize					signamelen;
	PGPBoolean				match = FALSE;
	PGPError				err	= kPGPError_NoErr;

	PGPValidatePtr( certsig );
	*certsig	= NULL;
	PGPValidateCert( cert );
	PGPValidateKeySet( allkeys );
	
	CHECKREMOVED(cert);
	
	context	= PGPGetKeySetContext( allkeys );
	
	err = PGPGetSigBoolean (cert, kPGPSigPropIsX509, &isX509);
	if( IsPGPError( err ) )
		goto error;
	if( !isX509 )
	{
		err = kPGPError_BadParams;
		goto error;
	}

	err = PGPGetSigCertifierKey( cert, allkeys, &certkey );
	if( IsPGPError( err ) )
		goto error;

	err = PGPGetSigPropertyBuffer( cert, kPGPSigPropX509IssuerLongName,
								   0, NULL, &issuernamelen );
	if( IsPGPError( err ) )
		goto error;

	issuername = pgpContextMemAlloc( context, issuernamelen, 0 );
	if( IsNull( issuername ) )
	{
		err = kPGPError_OutOfMemory;
		goto error;
	}

	err = PGPGetSigPropertyBuffer( cert, kPGPSigPropX509IssuerLongName,
								   issuernamelen, issuername, &issuernamelen );
	if( IsPGPError( err ) )
		goto error;

	for (userid = (PGPUserID *) certkey->userIDs.next; 
		 !match && userid != (PGPUserID *) &certkey->userIDs;
		 userid = userid->next)
	{ 
		if (userid->removed)
			continue;
		for (sig = (PGPSigRef) userid->certs.next;
			 !match && sig != (PGPSigRef)&userid->certs;
			 sig = sig->next)
		{
			/* Find a non-removed, X509 sig which matches issuername */
			if (sig->removed)
				continue;
			err = PGPGetSigBoolean (sig, kPGPSigPropIsX509, &isX509);
			if( IsPGPError( err ) )
				goto error;
			if( !isX509 )
				continue;
			err = PGPGetSigPropertyBuffer( sig, kPGPSigPropX509LongName,
										   0, NULL, &signamelen );
			if( IsPGPError( err ) )
				goto error;
			if( signamelen != issuernamelen )
				continue;
			signame = pgpContextMemAlloc( context, signamelen, 0 );
			if( IsNull( signame ) )
				{
					err = kPGPError_OutOfMemory;
					goto error;
				}
			err = PGPGetSigPropertyBuffer( sig, kPGPSigPropX509LongName,
										   signamelen, signame,
										   &signamelen );
			if( IsPGPError( err ) )
				goto error;

			match = pgpMemoryEqual( issuername, signame, signamelen );
			pgpContextMemFree( context, signame );
			signame = NULL;
			if( match )
				*certsig = sig;
		}
	 }


error:

	if( IsntNull( issuername ) )
		pgpContextMemFree( context, issuername );
	if( IsntNull( signame ) )
		pgpContextMemFree( context, signame );

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
	PGPSize			passphraseLength,
	PGPBoolean		hashedPhrase
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
				  passphraseLength, hashedPhrase, FALSE,
				  exportable, SIG_EXPORTABLEHASHED,
				  0, kPGPExpirationTime_Never, 0, 0, NULL,
				  NULL, 0);
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
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Passkey
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
	PGPBoolean			hashedPhrase = FALSE;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPCertValid(cert));
	PGPValidateCert( cert );
	PGPValidateKeySet( allkeys );

	context = cert->up.userID->key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
						revsigOptionSet, elemsof( revsigOptionSet ) ) ) )
		return err;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	err = sRevokeCert( context, cert, allkeys, passphrase, passphraseLength,
					   hashedPhrase);
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
		(void)ringKeyAdditionalRecipientRequestKey (keyobj, ringset, 0,
								  NULL, NULL, &tclass, &nadks, &err);
		if ( err == kPGPError_ItemNotFound )
		{
			nadks	= 0;
			err		= kPGPError_NoErr;
		}
	}
	
	*numARKeys = nadks;
	return err;
}


/*  Return the nth (0 based) additional decryption key and keyid,
	if one exists.
	It is an error to use an index >= K, where K is the number of ARR key ids.
	
 	Also return the class of the ADK.  The class is currently reserved
 	for use by PGP.
 	Any of the return pointers may be NULL.
 	
	Note that it is *not* safe to use the keyID returned from this function
	to get the ADK to use because KeyIDs are not unique.
	Instead, the keyID can be used to locate the actual key(s) with that
	key id.
	Then call this function again to get the ADK;
	it will check the key fingerprint, which is unique.

*/
	static PGPError
pgpGetIndexedAdditionalRecipientRequestKey(
	PGPKeyRef		basekey,
	PGPKeySetRef	allkeys,
	PGPUInt32		nth,
    PGPKeyRef*		adkey,
	PGPKeyID *		adkeyid,
    PGPByte *		adclass)
{
	RingSet const		*ringset;		/* Aurora ringset to look in */
	union RingObject    *keyobj;		/* Aurora base key */
	union RingObject    *rkey;			/* Aurora additional decryption key */
	unsigned			 nadks;			/* Number ADK's available */
	PGPByte				 tclass;		/* Class code from ADK */
	PGPError			 error;			/* Error return from Aurora */
	PGPByte				 pkalg;			/* pkalg of ADK */
	PGPKeyID			keyid;		/* keyid of ADK */
	PGPError			 err	= kPGPError_NoErr;
	PGPContextRef		 context;

	if( IsntNull( adkeyid ) )
		pgpClearMemory( adkeyid, sizeof( *adkeyid ) );
	if ( IsntNull( adclass ) )
		*adclass	= 0;
	if ( IsntNull( adkey ) )
		*adkey	= NULL;
		
	ringset = NULL;

	PGPValidateKey( basekey );
	PGPValidateKeySet( allkeys );
	
	context	= PGPGetKeyContext( basekey );
	
	error = pgpKeySetRingSet (allkeys, TRUE, &ringset);
	if( IsPGPError( error ) )
		return error;

	keyobj = basekey->key;
	rkey = ringKeyAdditionalRecipientRequestKey (keyobj, ringset, nth,
								 &pkalg, &keyid, &tclass, &nadks, &error);

	if( IsPGPError( error ) )
	{
		ringSetDestroy( (RingSet *) ringset );
		return error;
	}
	
	/* Success */
	if ( IsntNull( adkey ) )
	{
		PGPKeyID	keyID;
		
		if (IsNull( rkey ) ) {
			*adkey = NULL;
		} else {
			ringKeyID8 (ringset, rkey, &pkalg, &keyID);

			err = PGPGetKeyByKeyID (allkeys, &keyID,
					(PGPPublicKeyAlgorithm)pkalg, adkey);
		}
	}

	if ( IsntNull( adkeyid ) )
	{
		*adkeyid	= keyid;
	}
	
	if ( IsntNull( adclass ) )
		*adclass = tclass;

	if( IsntNull( ringset ) )
		ringSetDestroy( (RingSet *) ringset );

	return err;
}



/* Given a key, return the nth (0 based) additional decryption key, if
 	one exists.  Also return the keyid, the class of the ADK, and the
 	number of ADK's for the base key.  Any of the return pointers may
 	be NULL. */

	PGPError
PGPGetIndexedAdditionalRecipientRequestKey(
	PGPKeyRef		basekey,
	PGPKeySetRef	allkeys,
	PGPUInt32		nth,
    PGPKeyRef *		adkey,
	PGPKeyID *		adkeyid,
    PGPByte *		adclass)
{
	PGPError	err	= kPGPError_NoErr;
	PGPKeyID	tempKeyID;
	
	if ( IsntNull( adkey ) )
		*adkey	= NULL;
	if ( IsntNull( adkeyid ) )
		pgpClearMemory( adkeyid, sizeof( *adkeyid) );
	if ( IsntNull( adclass ) )
		*adclass	= 0;

	PGPValidateKey( basekey );
	PGPValidateKeySet( allkeys );
	
	err	= pgpGetIndexedAdditionalRecipientRequestKey( basekey,
			allkeys, nth, adkey, &tempKeyID, adclass );
	if ( IsntPGPError( err ) )
	{
		pgpAssert( pgpKeyIDIsValid( &tempKeyID ) );
		if( IsntNull( adkeyid ) )
		{
			*adkeyid	= tempKeyID;
		}
	}
	else
	{
		pgpClearMemory( adkeyid, sizeof( *adkeyid) );
	}
	
	return( err );
}


	PGPError
PGPCountRevocationKeys(
	PGPKeyRef		basekey,
    PGPUInt32 *		numRevKeys)
{
	RingSet const		*ringset = NULL;/* Aurora ringset to look in */
	PGPUInt32			 nrevs;			/* Number rev keys available */
	PGPError			err	= kPGPError_NoErr;
	
	PGPValidatePtr( numRevKeys );
	*numRevKeys	= 0;
	PGPValidateKey( basekey );
	
	ringset = pgpKeyDBRingSet (basekey->keyDB);
	if (IsNull( ringset ) )
		err = kPGPError_OutOfRings;
	
	if( IsntPGPError( err ) )
	{
		union RingObject    *keyobj;		/* Aurora base key */
		
		keyobj	= basekey->key;
		(void)ringKeyRevocationKey (keyobj, ringset, 0, NULL, NULL, NULL,
									&nrevs, &err);
		if ( err == kPGPError_ItemNotFound )
		{
			nrevs	= 0;
			err		= kPGPError_NoErr;
		}
	}
	
	*numRevKeys = nrevs;
	return err;
}


/*  Return the nth (0 based) revocation key and keyid,
	if one exists.
	It is an error to use an index >= K, where K is the number of ARR key ids.
	
 	Also return the class of the revkey.  The high bit is set for a
	reocation key.
 	Any of the return pointers may be NULL.
 	
	Note that it is *not* safe to use the keyID returned from this function
	to get the revkey to use because KeyIDs are not unique.
	Instead, the keyID can be used to locate the actual key(s) with that
	key id.
	Then call this function again to get the revkey;
	it will check the key fingerprint, which is unique.

*/
	static PGPError
pgpGetIndexedRevocationKey(
	PGPKeyRef		basekey,
	PGPKeySetRef	allkeys,
	PGPUInt32		nth,
    PGPKeyRef*		revkey,
	PGPKeyID *		revkeyid,
    PGPByte *		revclass)
{
	RingSet const		*ringset;		/* Aurora ringset to look in */
	union RingObject    *keyobj;		/* Aurora base key */
	union RingObject    *rkey;			/* Aurora revocation key */
	unsigned			 nrevks;		/* Number revkey's available */
	PGPByte				 tclass;		/* Class code from revkey */
	PGPError			 error;			/* Error return from Aurora */
	PGPByte				 pkalg;			/* pkalg of revkey */
	PGPKeyID			 keyid;			/* keyid of revkey */
	PGPError			 err	= kPGPError_NoErr;
	PGPContextRef		 context;

	if( IsntNull( revkeyid ) )
		pgpClearMemory( revkeyid, sizeof( *revkeyid ) );
	if ( IsntNull( revclass ) )
		*revclass	= 0;
	if ( IsntNull( revkey ) )
		*revkey	= NULL;
		
	ringset = NULL;

	PGPValidateKey( basekey );
	PGPValidateKeySet( allkeys );
	
	context	= PGPGetKeyContext( basekey );
	
	error = pgpKeySetRingSet (allkeys, TRUE, &ringset);
	if( IsPGPError( error ) )
		return error;

	keyobj = basekey->key;
	rkey = ringKeyRevocationKey (keyobj, ringset, nth,
								 &pkalg, &keyid, &tclass, &nrevks, &error);

	if( IsPGPError( error ) )
	{
		ringSetDestroy( (RingSet *) ringset );
		return error;
	}
	
	/* Success */
	if ( IsntNull( revkey ) )
	{
		PGPKeyID	keyID;
		
		if (IsNull( rkey ) ) {
			*revkey = NULL;
		} else {
			ringKeyID8 (ringset, rkey, &pkalg, &keyID);
		
			err = PGPGetKeyByKeyID (allkeys, &keyID,
									(PGPPublicKeyAlgorithm)pkalg, revkey);
		}
	}

	if ( IsntNull( revkeyid ) )
	{
		*revkeyid	= keyid;
	}
	
	if ( IsntNull( revclass ) )
		*revclass = tclass;

	if( IsntNull( ringset ) )
		ringSetDestroy( (RingSet *) ringset );

	return err;
}



/* Given a key, return the nth (0 based) revocation key, if
 	one exists.  Also return the keyid, the class of the revkey, and the
 	number of revkey's for the base key.  Any of the return pointers may
 	be NULL. */

	PGPError
PGPGetIndexedRevocationKey(
	PGPKeyRef		basekey,
	PGPKeySetRef	allkeys,
	PGPUInt32		nth,
    PGPKeyRef *		revkey,
	PGPKeyID *		revkeyid)
{
	PGPError	err	= kPGPError_NoErr;
	PGPKeyID	tempKeyID;
	
	if ( IsntNull( revkey ) )
		*revkey	= NULL;
	if ( IsntNull( revkeyid ) )
		pgpClearMemory( revkeyid, sizeof( *revkeyid) );

	PGPValidateKey( basekey );
	PGPValidateKeySet( allkeys );
	
	err	= pgpGetIndexedRevocationKey( basekey,
			allkeys, nth, revkey, &tempKeyID, NULL );
	if ( IsntPGPError( err ) )
	{
		pgpAssert( pgpKeyIDIsValid( &tempKeyID ) );
		if( IsntNull( revkeyid ) )
		{
			*revkeyid	= tempKeyID;
		}
	}
	else
	{
		pgpClearMemory( revkeyid, sizeof( *revkeyid) );
	}
	
	return( err );
}


/*
 * Return a buffer with CRL distribution points in it.  *pnDistPoints
 * tells how many distribution points there are; *pdpointLengths holds
 * the size of each distribution point; *pDpoints holds the actual
 * distribution point pointer.  The latter two values are dynamically
 * allocated and should be freed by the caller.
 */
	PGPError
PGPGetCRLDistributionPoints(
	PGPKeyRef cakey,
	PGPKeySetRef keyset,
	PGPUInt32 *pnDistPoints,			/* Output parameters */
	PGPByte **pDpoints,
	PGPSize **pdpointLengths
	)
{
	PGPContextRef		context;
	PGPMemoryMgrRef		mgr;
	RingSet const		*ringset;
	RingObject			*keyobj;
	PGPUInt32			nDistPoints;
	PGPByte				*dpoints;
	PGPSize				*dpointlens;
	PGPError			error = kPGPError_NoErr;

	if ( IsntNull( pnDistPoints ) )
		*pnDistPoints = 0;
	if ( IsntNull( pDpoints ) )
		*pDpoints = NULL;
	if ( IsntNull( pdpointLengths ) )
		*pdpointLengths = NULL;

	PGPValidateKey( cakey );
	PGPValidateKeySet( keyset );
	
	context	= PGPGetKeyContext( cakey );
	mgr = PGPGetContextMemoryMgr( context );
	
	error = pgpKeySetRingSet (keyset, TRUE, &ringset);
	if( IsPGPError( error ) )
		return error;

	keyobj = cakey->key;

	error = ringListCRLDistributionPoints( mgr, keyobj, ringset,
									&nDistPoints, &dpoints, &dpointlens );
	if( IsPGPError( error ) )
	{
		ringSetDestroy( (RingSet *) ringset );
		return error;
	}

	if ( IsntNull( pnDistPoints ) )
		*pnDistPoints = nDistPoints;
	if ( IsntNull( pDpoints ) )
		*pDpoints = dpoints;
	if ( IsntNull( pdpointLengths ) )
		*pdpointLengths = dpointlens;

	ringSetDestroy( (RingSet *) ringset );
	return kPGPError_NoErr;
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

	if( pgpTrustModel( pgpRingPool ) == PGPTRUST0 ) {

		error = pgpCopyKey (allset, keyobj, &addset);
		if (error)
			return error;

		ringKeySetTrust (allset, keyobj, (PGPByte)trust);
		pgpKeyDBChanged (keys, addset);
	} else {
		/* New trust model, set confidence on all userids */
		RingIterator *keyiter;
		PGPUInt32 level;
		PGPUInt16 confidence;

		error = pgpCopyKey (allset, keyobj, &addset);
		if( IsPGPError( error ) )
			goto error;
		ringSetFreeze (addset);

		confidence = ringTrustToIntern(
			(PGPByte)ringTrustOldToExtern( pgpRingPool, (PGPByte)trust ) );

		keyobj = key->key;
		keyiter = ringIterCreate( addset );
		if( IsNull( keyiter ) ) {
			error = ringSetError( addset )->error;
			goto error;
		}

		ringIterSeekTo( keyiter, keyobj );
		level = ringIterCurrentLevel( keyiter );
		ringIterRewind( keyiter, level+1 );
		while( ringIterNextObject( keyiter, level+1 ) > 0 ) {
			RingObject *nameobj = ringIterCurrentObject( keyiter, level+1 );
			if( ringObjectType( nameobj ) != RINGTYPE_NAME )
				continue;
			ringNameSetConfidence( addset, nameobj,
								   (unsigned short) confidence );
		}
		ringIterDestroy( keyiter );

		/* Also set key trust for consistency */
		ringKeySetTrust (allset, keyobj, (PGPByte)trust);

		pgpKeyDBChanged (keys, addset);
	}

error:
	if (addset)
		ringSetDestroy (addset);
	return error;
}


/*  Set a secret key as the axiomatic key.  If checkPassphrase == TRUE,
	the user must prove knowledge of the passphrase in order to do 
	this. */

	static PGPError
sSetKeyAxiomatic (
	PGPKey *		key,
	PGPBoolean		checkPassphrase,
	char const *	passphrase,
	PGPSize			passphraseLength,
	PGPBoolean		hashedPhrase
	)
{
    PGPBoolean               secret, axiomatic;
	RingSet const           *allset;
	RingSet					*addset = NULL;
	union RingObject        *keyobj;
	PGPSecKey         		*seckey;
	PGPEnv					*pgpEnv;
	PGPKeyDB                *keys;
	PGPError                 error = kPGPError_NoErr;

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
		    if (IsNull( passphrase )) {
			    pgpSecKeyDestroy (seckey);
			    return kPGPError_BadPassphrase;
			}
			pgpEnv = pgpContextGetEnvironment( keys->context );
			error = (PGPError)pgpSecKeyUnlock (seckey, pgpEnv, passphrase, 
									 passphraseLength, hashedPhrase);
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


static const PGPOptionType setkeyaxiomaticOptionSet[] = {
	 kPGPOptionType_Passphrase,
	 kPGPOptionType_Passkey
};

	PGPError
pgpSetKeyAxiomaticInternal(
	PGPKeyRef			key,
	PGPOptionListRef	optionList
	)
{
	PGPContextRef		context;
	char *				passphrase;
	PGPSize				passphraseLength;
	PGPBoolean			hashedPhrase = FALSE;
	PGPError			err = kPGPError_NoErr;

	pgpa(pgpaPGPKeyValid(key));
	PGPValidateKey( key );
	
	context = key->keyDB->context;

	if (IsPGPError( err = pgpCheckOptionsInSet( optionList,
								setkeyaxiomaticOptionSet,
								elemsof( setkeyaxiomaticOptionSet ) ) ) )
		goto error;

	/* Pick up optional options */
	if( IsPGPError( err = pgpFindOptionArgs( optionList,
						 kPGPOptionType_Passphrase, FALSE,
						 "%p%l", &passphrase, &passphraseLength ) ) )
		goto error;
	if (IsNull( passphrase )) {
		hashedPhrase = TRUE;
		if( IsPGPError( err = pgpFindOptionArgs( optionList,
							kPGPOptionType_Passkey, FALSE,
							"%p%l", &passphrase, &passphraseLength ) ) )
			goto error;
	}

	if( IsPGPError( err=sSetKeyAxiomatic( key, (PGPBoolean)(passphrase!=NULL),
									   passphrase, passphraseLength,
									   hashedPhrase) ) )
		goto error;

error:

	return err;
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
						 PGPKeyPropName propname, PGPInt32 *prop,
						 PGPEnv const *env)
{
	unsigned char		 pkalg;
	PGPSecKey *			seckey = NULL;
	PGPCipherAlgorithm	lockalg;
	PGPSize				lockbytes;
	PGPError			err;

	switch (propname) {
	case kPGPKeyPropAlgID:
		ringKeyID8 (ringset, keyobj, &pkalg, NULL);
		*prop = (long) pkalg;
		break;
	case kPGPKeyPropBits:
		*prop = (long) ringKeyBits (ringset, keyobj);
		break;
	case kPGPKeyPropLockingAlgID:
	case kPGPKeyPropLockingBits:
		if( !ringKeyIsSec (ringset, keyobj) )
			return kPGPError_InvalidProperty;
		seckey = ringSecSecKey (ringset, keyobj, 0);
		if( !seckey )
			return ringSetError(ringset)->error;
		if( !pgpSecKeyIslocked (seckey) ) {
			/* Use defaults if key is not locked.
			 * This is not really a very good guess, first because the
			 * default differs for RSA and other keys, and second because
			 * we use the key's preferred algorithm if it has one.
			 */
			PGPCipherVTBL const *cipher = pgpCipherDefaultKey( env );
			if( propname == kPGPKeyPropLockingAlgID )
				*prop = (long)cipher->algorithm;
			else
				*prop = (long) cipher->keysize * 8;
		} else {
			err = pgpSecKeyLockingalgorithm( seckey, &lockalg, &lockbytes );
			if( IsPGPError( err ) )
				return err;
			if( propname == kPGPKeyPropLockingAlgID )
				*prop = (long) lockalg;
			else
				*prop = (long) lockbytes * 8;
		}
		pgpSecKeyDestroy (seckey);
		break;
	case kPGPKeyPropFlags:
		{
			PGPUInt32 flags = 0;
			PGPByte const *pflags;
			PGPSize flaglen, i;

			pflags = ringKeyFindSubpacket( keyobj, ringset, SIGSUB_KEYFLAGS,
										   0, &flaglen, NULL, NULL, NULL,
										   NULL, &err );
			if( IsPGPError( err ) )
				return err;
			if( IsNull( pflags ) )
				return kPGPError_InvalidProperty;
			
			/* Only return first n bytes */
			if( flaglen > sizeof(flags) )
				flaglen = sizeof(flags);
			
			/* Pack bytes into flags word, first into LSB */
			for( i=0; i<flaglen; ++i ) {
				flags |= pflags[i] << (i*8);
			}

			/* Return flags */
			*prop = flags;
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
	PGPUserIDRef		userid;
	RingSet const *		ringset;
	RingPool *			pgpRingPool;
	PGPInt32            trustval;
	PGPEnv const *		pgpEnv;

	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateKey( key );
	
    switch (propname) {
	case kPGPKeyPropValidity:
		pgpRingPool = pgpContextGetRingPool( key->keyDB->context );
		*prop = kPGPValidity_Unknown;
		pgpIncKeyRefCount (key);
		for (userid = (PGPUserID *) key->userIDs.next; 
			 userid != (PGPUserID *) &key->userIDs; userid = userid->next)
			 { 
			if (!userid->removed) {
				PGPGetUserIDNumber (userid, kPGPUserIDPropValidity, &trustval);
				if (trustval > *prop)
					*prop = trustval;
			}
		}
		pgpFreeKey (key);
		break;
	case kPGPKeyPropTrust:
		pgpRingPool = pgpContextGetRingPool( key->keyDB->context );
	    ringset = pgpKeyDBRingSet (key->keyDB);
		if (pgpTrustModel (pgpRingPool) == PGPTRUST0) {
			PGPByte trust;
			trust = ringKeyTrust (ringset, key->key);
			if (trust == kPGPKeyTrust_Undefined ||
				trust == kPGPKeyTrust_Unknown)
			    trust = kPGPKeyTrust_Never;
			*prop = (long) trust;
			break;
		} else { /* new trust model */
			*prop = ringTrustExternToOld( pgpRingPool,
						  ringTrustToExtern(
							ringKeyConfidence( ringset, key->key ) ) );
		}
		break;
	default:
	    ringset = pgpKeyDBRingSet (key->keyDB);
		pgpEnv = pgpContextGetEnvironment( key->keyDB->context );
		error =  pgpGetKeyNumberInternal (ringset, key->key, propname, prop,
										  pgpEnv);
	}
	return error;
}

PGPError
PGPGetSubKeyNumber (
PGPSubKeyRef subkey, PGPKeyPropName propname, PGPInt32 *prop)
{
    PGPError            error = kPGPError_NoErr;
	RingSet const *		ringset;
	PGPEnv const *		pgpEnv;

	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateSubKey( subkey );
	
	CHECKREMOVED(subkey);
	switch (propname) {
	case kPGPKeyPropAlgID:
	case kPGPKeyPropBits:
	case kPGPKeyPropLockingAlgID:
	case kPGPKeyPropLockingBits:
	    ringset = pgpKeyDBRingSet (subkey->key->keyDB);
		pgpEnv = pgpContextGetEnvironment( subkey->key->keyDB->context );
	    error = pgpGetKeyNumberInternal (ringset, subkey->subKey,
										 propname, prop, pgpEnv);
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
	RingObject const *crl;

	switch (propname) {
	case kPGPKeyPropCreation:
		*prop = ringKeyCreation (ringset, keyobj);
		break;
	case kPGPKeyPropExpiration:
		*prop = ringKeyExpiration (ringset, keyobj);
		break;
	case kPGPKeyPropCRLThisUpdate:
		crl = ringKeyEarliestCRL( ringset, keyobj, FALSE );
		if( IsNull( crl ) )
			return kPGPError_InvalidProperty;
		*prop = ringCRLCreation( ringset, crl );
		break;
	case kPGPKeyPropCRLNextUpdate:
		crl = ringKeyEarliestCRL( ringset, keyobj, TRUE );
		if( IsNull( crl ) )
			return kPGPError_InvalidProperty;
		*prop = ringCRLExpiration( ringset, crl );
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
		if (ringKeyV3(ringset, keyobj)) {
			ringKeyFingerprint16 (ringset, keyobj, buffer);
			*actualLength = 16;
		}
		else {
			ringKeyFingerprint20 (ringset, keyobj, buffer);
			*actualLength = 20;
		}
		break;

	case kPGPKeyPropThirdPartyRevocationKeyID:
	{
		PGPKeyID		keyid;
		PGPByte const *	idBytes;
	
		if (!ringKeyHasThirdPartyRevocation (keyobj, ringset,
											 NULL, NULL, &keyid, &err)) {
			return kPGPError_BadParams;
		}
		idBytes	= pgpGetKeyBytes( &keyid );
		for (i = 0; i < 4; i++)
			buffer[i] = idBytes[i+4];
			
		*actualLength = 4;
		break;
	}

	case kPGPKeyPropPreferredAlgorithms:
		/* Must convert from byte form to array of PGPCipherAlgorithm */
		prefAlgs = (PGPByte *)ringKeyFindSubpacket (
				keyobj, ringset, SIGSUB_PREFERRED_ENCRYPTION_ALGS, 0,
				actualLength, NULL, NULL, NULL, NULL, &err);
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

	case kPGPKeyPropKeyData:
		{	/* MPI data from key, algorithm specific */
			PGPByte const *keyData;
			PGPSize keyDataLength, keyDataOffset;

			keyData = ringFetchObject( ringset, keyobj, &keyDataLength );
			pgpAssert( IsntNull( keyData ) );
			/* V4 keys are two bytes shorter than V3 */
			keyDataOffset = 6;
			if (keyData[0] <= PGPVERSION_3)
				keyDataOffset += 2;
			*actualLength = keyDataLength - keyDataOffset;
			err = pgpReturnPropBuffer ( (char *) keyData+keyDataOffset, prop,
										*actualLength, bufferSize);
			return err;
		}

	case kPGPKeyPropX509MD5Hash:
		{	/* Hash the key data in X.509 SubjPubKeyInfo (SPKI) format */
			PGPByte *keyData;
			PGPSize keyDataLength;
			PGPHashContextRef hc;
			PGPMemoryMgrRef	memoryMgr;

			if( IsNull( prop ) ) {
				*actualLength = 16;
				return kPGPError_NoErr;
			}
				
			context = ringPoolContext(ringSetPool(ringset));
			err = pgpKeyToX509SPKI( context, ringset, keyobj, NULL,
									&keyDataLength );
			if (IsPGPError( err ))
				return err;
			keyData = (PGPByte *)pgpContextMemAlloc( context,
													 keyDataLength, 0);
			if( IsNull( keyData ) )
				return kPGPError_OutOfMemory;
			err = pgpKeyToX509SPKI( context, ringset, keyobj, keyData,
									&keyDataLength );
			if (IsPGPError( err )) {
				pgpContextMemFree( context, keyData );
				return err;
			}
			
			memoryMgr = PGPGetContextMemoryMgr( context );
			err = PGPNewHashContext( memoryMgr, kPGPHashAlgorithm_MD5, &hc );
			if( IsPGPError( err ) ) {
				pgpContextMemFree( context, keyData );
				return err;
			}
			PGPContinueHash( hc, keyData, keyDataLength );
			pgpAssert (sizeof(buffer) >= 16);
			PGPFinalizeHash( hc, buffer );
			PGPFreeHashContext( hc );
			*actualLength = 16;
			pgpContextMemFree( context, keyData );
			break;
		}
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

/* Check other aspects of key usability for sign/encrypt etc. */
static PGPError
sIsUsableKey (RingSet const *ringset, RingObject *keyobj,
			  PGPKeyPropName propname, PGPBoolean *usable)
{
	PGPInt32		pkalg;
	PGPInt32		bits;
	RingPool		*pool;
	PGPContextRef 	context;
	PGPEnv			*env; 
	PGPError		err = kPGPError_NoErr;

	pgpAssert (IsntNull( usable ) );
	(void) propname;

	*usable = TRUE;

	/* Disallow RSA keys bigger than BSafe supports */
	pool = ringSetPool( ringset );
	context = ringPoolContext( pool );
	env = pgpContextGetEnvironment( context );

	err = pgpGetKeyNumberInternal( ringset, keyobj, kPGPKeyPropAlgID,
								   &pkalg, env );
	if( IsPGPError( err) )
		return err;
	err = pgpGetKeyNumberInternal( ringset, keyobj, kPGPKeyPropBits,
								   &bits, env );
	if( IsPGPError( err) )
		return err;

	if( pkalg <= kPGPPublicKeyAlgorithm_RSA + 2 && bits > MAXRSABITS)
		*usable = FALSE;

	return err;
}


static PGPError
pgpGetKeyBooleanInternal (RingSet const *ringset, RingObject *keyobj,
						  PGPKeyPropName propname, PGPBoolean *prop)
{
	PGPSecKey *			seckey = NULL;
	PGPUInt32			expiration;
	PGPError			err = kPGPError_NoErr;
	
	/*
	** Note: Some computed properties call this function recursively to get
	** other properties. This is done so the logic of implementing a
	** particular property only occurrs in one location
	*/
	
	switch (propname) {
	case kPGPKeyPropIsSecret:
		*prop = (ringKeyIsSec (ringset, keyobj) != 0);
		break;
	case kPGPKeyPropIsAxiomatic:
		*prop = (ringKeyAxiomatic (ringset, keyobj) != 0);
		break;
	case kPGPKeyPropIsRevoked:
		*prop = (ringKeyRevoked (ringset, keyobj) != 0);
		/* "Or" in revocation status of master key on subkeys */
		if (!*prop &&
			ringKeyIsSubkey (ringset, keyobj))
			*prop = (ringKeyRevoked (ringset,
									ringKeyMasterkey(ringset, keyobj))) != 0;
		break;
	case kPGPKeyPropIsRevocable:
		if (ringKeyIsSec (ringset, keyobj)) {
			*prop = TRUE;
		} else {
			RingObject *revobj;
			PGPByte revclass;
			PGPUInt32 i;

			*prop = FALSE;
			if (ringKeyIsSubkey (ringset, keyobj))
				keyobj = ringKeyMasterkey (ringset, keyobj);
			pgpAssert (IsntNull( keyobj ) );
			for (i=0; ; ++i) {
				revobj = ringKeyRevocationKey (keyobj, ringset, i, NULL, NULL,
											   &revclass, NULL, &err);
				if( IsPGPError( err ) ) {
					if( err == kPGPError_ItemNotFound )
						err = kPGPError_NoErr;
					break;
				}
				if( IsNull( revobj ) )
					continue;
				if ((revclass & 0x80) == 0)
					continue;
				if (ringKeyIsSec (ringset, revobj)) {
					*prop = TRUE;
					break;
				}
			}
		}
		break;

	case kPGPKeyPropHasThirdPartyRevocation:
		*prop = ringKeyHasThirdPartyRevocation (keyobj, ringset,
												NULL, NULL, NULL, &err);
		break;

	case kPGPKeyPropIsSecretShared:
		{
			PGPBoolean isSecretShared = FALSE;
			if( ringKeyIsSec (ringset, keyobj) )
			{
				seckey = ringSecSecKey (ringset, keyobj, 0);
				if (!seckey) {
					err = ringSetError(ringset)->error;
					/* Don't return an error for unsupported key types */
					if (err == kPGPError_FeatureNotAvailable) {
						*prop = FALSE;
						return kPGPError_NoErr;
					}
					return err;
				}
				if( pgpSecKeyIslocked (seckey) )
				{
					PGPStringToKeyType s2ktype;
					pgpSecKeyS2Ktype( seckey, &s2ktype );
					if( s2ktype == kPGPStringToKey_LiteralShared )
						isSecretShared = TRUE;
				}
				pgpSecKeyDestroy (seckey);
			}
			*prop = isSecretShared;
		}
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
			{
				err = kPGPError_OutOfMemory;
				break;
			}
			
			ringIterSeekTo (iter, keyobj);
			level = ringIterCurrentLevel (iter);
			ringIterRewind (iter, level+1);
			while (ringIterNextObject (iter, level+1) > 0) {
				obj = ringIterCurrentObject (iter, level+1);
				if (ringObjectType(obj) == RINGTYPE_SIG &&
					ringSigType (ringset, obj) == PGP_SIGTYPE_KEY_REVOKE) {
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
		{
			err = kPGPError_SecretKeyNotFound;
			break;
		}
		seckey = ringSecSecKey (ringset, keyobj, 0);
		if (!seckey)
		{
			err = ringSetError(ringset)->error;
			break;
		}
		
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
	case kPGPKeyPropIsSigningKey:
	    *prop = ((ringKeyUse (ringset, keyobj) & PGP_PKUSE_SIGN) != 0);
		break;
	case kPGPKeyPropIsEncryptionKey:
	    *prop = ((ringKeyUse (ringset, keyobj) & PGP_PKUSE_ENCRYPT) != 0);
		break;

	case kPGPKeyPropCanEncrypt:
	case kPGPKeyPropCanSign:
	{
		/* Not corrupted and not revoked and not expired and not disabled */

		PGPBoolean	notCorrupted;
		
		*prop = FALSE;
		
		if( propname == kPGPKeyPropCanSign )
		{
			PGPBoolean	isSecretKey;
			
			/* Quick reject non-secret keys in the signing case */
			err = pgpGetKeyBooleanInternal( ringset, keyobj,
						kPGPKeyPropIsSecret, &isSecretKey );
			if( IsPGPError( err ) || ! isSecretKey )
				break;
		}
		
		err = pgpGetKeyBooleanInternal( ringset, keyobj,
						kPGPKeyPropIsNotCorrupt, &notCorrupted );
		if( IsntPGPError( err ) && notCorrupted )
		{
			PGPBoolean	isRevoked;
			
			err = pgpGetKeyBooleanInternal( ringset, keyobj,
							kPGPKeyPropIsRevoked, &isRevoked );
			if( IsntPGPError( err ) && ! isRevoked )
			{
				PGPBoolean	isExpired;
				
				err = pgpGetKeyBooleanInternal( ringset, keyobj,
							kPGPKeyPropIsExpired, &isExpired );
				if( IsntPGPError( err ) && ! isExpired )
				{
					PGPBoolean	isDisabled;
					
					err = pgpGetKeyBooleanInternal( ringset, keyobj,
								kPGPKeyPropIsDisabled, &isDisabled );
					if( IsntPGPError( err ) && ! isDisabled )
					{
						PGPBoolean isUsable;

						err = sIsUsableKey( ringset, keyobj, propname,
											&isUsable );
						if( IsntPGPError( err ) && isUsable )
						{
							if( propname == kPGPKeyPropCanEncrypt )
							{
								*prop = ((ringKeyUnexpiredUse(ringset, keyobj)
										  & PGP_PKUSE_ENCRYPT) != 0);
							}
							else
							{
								err = pgpGetKeyBooleanInternal( ringset,
									keyobj, kPGPKeyPropIsSigningKey, prop );
							}
						}
					}
				}
			}
		}
		
		break;
	}

	case kPGPKeyPropCanDecrypt:
	{
		PGPBoolean	isSecretKey;
		
		/* Is secret key and not corrupted and is encryption key */

		*prop = FALSE;
		
		err = pgpGetKeyBooleanInternal( ringset, keyobj,
					kPGPKeyPropIsSecret, &isSecretKey );
		if( IsntPGPError( err ) && isSecretKey )
		{
			PGPBoolean	notCorrupted;
			
			err = pgpGetKeyBooleanInternal( ringset, keyobj,
							kPGPKeyPropIsNotCorrupt, &notCorrupted );
			if( IsntPGPError( err ) && notCorrupted )
			{
				PGPBoolean isUsable;

				err = sIsUsableKey( ringset, keyobj, propname,
									&isUsable );
				if( IsntPGPError( err ) && isUsable )
				{
					err = pgpGetKeyBooleanInternal( ringset, keyobj,
									kPGPKeyPropIsEncryptionKey, prop );
				}
			}
		}
		
		break;
	}
	
	case kPGPKeyPropCanVerify:
	{
		/* Can verify if not corrupted and a signature key */
		PGPBoolean	notCorrupted;

		err = pgpGetKeyBooleanInternal( ringset, keyobj,
						kPGPKeyPropIsNotCorrupt, &notCorrupted );
		if( IsntPGPError( err ) && notCorrupted )
		{
			PGPBoolean isUsable;

			err = sIsUsableKey( ringset, keyobj, propname,
								&isUsable );
			if( IsntPGPError( err ) && isUsable )
			{
				err = pgpGetKeyBooleanInternal( ringset, keyobj,
								kPGPKeyPropIsSigningKey, prop );
			}
		}
		break;
	}

	case kPGPKeyPropHasCRL:
		*prop = ringKeyHasCRL(ringset, keyobj );
		break;
		
	default:
		err = kPGPError_InvalidProperty;
		break;
	}
	
	return err;
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
	RingPool		    *pgpRingPool;

	pgpa(pgpaPGPUserIDValid(userid));
	
	PGPValidatePtr( prop );
	*prop	= 0;
	PGPValidateUserID( userid );
	
	CHECKREMOVED(userid);
	ringset		= pgpKeyDBRingSet (userid->key->keyDB);
	nameobj		= userid->userID;
	pgpRingPool	= ringSetPool( ringset );

	switch (propname) {
	case kPGPUserIDPropValidity:
		if (pgpTrustModel (pgpRingPool) == PGPTRUST0) 
		    *prop = (long) ringNameTrust (ringset, nameobj);
		else {
			PGPUInt32 namevalidity;
			namevalidity = ringTrustExternToOld( pgpRingPool,
					ringTrustToExtern(
						ringNameValidity( ringset, nameobj ) ) );
			/* namevalidity is keytrust scale */
			if (namevalidity <= kPGPKeyTrust_Unknown)
				*prop = kPGPNameTrust_Unknown;
			else if (namevalidity < kPGPKeyTrust_Marginal)
				*prop = kPGPNameTrust_Untrusted;
			else if (namevalidity == kPGPKeyTrust_Marginal)
				*prop = kPGPNameTrust_Marginal;
			else
				*prop = kPGPNameTrust_Complete;
		}
		break;
	case kPGPUserIDPropConfidence: 
		if (pgpTrustModel (ringSetPool(ringset)) > PGPTRUST0) {
			*prop = ringNameConfidence (ringset, nameobj);
			break;
		}
	case kPGPUserIDPropAttributeType:
		(void) ringNameAttributeSubpacket( nameobj, ringset, 0,
										   (PGPUInt32 *)prop, NULL, NULL );
		break;
		
	default:
		return kPGPError_InvalidProperty;
	}
	return kPGPError_NoErr;
}


PGPError
PGPGetUserIDBoolean (
PGPUserID *userid, PGPUserIDPropName propname, PGPBoolean *prop)
{
	RingSet const		*ringset = NULL;
	union RingObject    *nameobj = NULL;
	RingPool		    *pgpRingPool;

	pgpa(pgpaPGPUserIDValid(userid));
	
	PGPValidatePtr( prop );
	*prop	= FALSE;
	PGPValidateUserID( userid );
	
	CHECKREMOVED(userid);
	ringset		= pgpKeyDBRingSet (userid->key->keyDB);
	nameobj		= userid->userID;
	pgpRingPool	= ringSetPool( ringset );

	switch (propname) {
	case kPGPUserIDPropIsAttribute:
		*prop = ringNameIsAttribute( ringset, nameobj ) != 0;
		break;
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
	char const	        *bufptr, *bufptr2;
	PGPError			err	= kPGPError_NoErr;
	PGPSize				fullLength;
	PGPSize				nullOffset;

	if ( IsntNull( fullLengthOut ) )
		*fullLengthOut	= 0;
	PGPValidateUserID( userid );
#if PGP_DEBUG
	if ( IsntNull( outString ) )
		pgpClearMemory( outString, bufferSize );
#endif
	
	CHECKREMOVED(userid);
	ringset	=	pgpKeyDBRingSet (userid->key->keyDB);

	switch( propname ) {
	case kPGPUserIDPropName:
		if (ringNameIsAttribute (ringset, userid->userID))
			return kPGPError_InvalidProperty;
		bufptr	= ringNameName (ringset, userid->userID, &fullLength );
		++fullLength;	/* leave room for null terminator */
		break;

	case kPGPUserIDPropEmailAddress:
		if (ringNameIsAttribute (ringset, userid->userID))
			return kPGPError_InvalidProperty;
		bufptr2	= ringNameName (ringset, userid->userID, &fullLength );
		bufptr = (char *)memchr(bufptr2, '<', fullLength);
		if (bufptr == NULL)
		{
			fullLength = 0;
			break;
		}
		bufptr++;
		fullLength -= (bufptr - bufptr2);
		bufptr2 = (char *)memchr(bufptr, '>', fullLength);
		if (bufptr2 != NULL)
			fullLength = bufptr2 - bufptr;
		break;

	case kPGPUserIDPropCommonName:
		if (ringNameIsAttribute (ringset, userid->userID))
			return kPGPError_InvalidProperty;
		bufptr	= ringNameName (ringset, userid->userID, &fullLength );
		bufptr2 = (char *)memchr(bufptr, '<', fullLength);
		if (bufptr2 == NULL)
			break;
		while (bufptr2 > bufptr && bufptr2[-1] == ' ')
			bufptr2--;
		fullLength = bufptr2 - bufptr;
		break;

	case kPGPUserIDPropAttributeData:
		if (!ringNameIsAttribute (ringset, userid->userID))
			return kPGPError_InvalidProperty;
		bufptr	= (const char *)ringNameAttributeSubpacket (userid->userID,
									ringset, 0, NULL, &fullLength, &err );
		if( IsPGPError( err ) )
			return err;
		break;

	default:
		return kPGPError_InvalidProperty;
	}

	err	= pgpReturnPropBuffer ( bufptr,
		(PGPByte *)outString, fullLength, bufferSize);
	
	if ( IsntNull( outString ) && propname==kPGPUserIDPropName )
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


	static PGPError
pgpGetSigStringInternal(
	PGPContextRef		context,
	RingSet const *		ringset,
	RingObject *		sigobj, 
	PGPSigPropName		propname,
	void *				prop,
	PGPSize				bufferSize,
	PGPSize *			actualLength)
{
	PGPByte *			ptr;
	PGPSize				len;
	PGPError			error;

	switch (propname) {
	default:
		return kPGPError_InvalidProperty;
		
	case kPGPSigPropX509Certificate:
		if (!ringSigIsX509 (ringset, sigobj))
			return kPGPError_InvalidProperty;
		ptr = ringSigX509Certificate(ringset, sigobj, &len);
		if (IsNull( ptr ) )
			return kPGPError_InvalidProperty;
		*actualLength = len;
		break;

	/* Return IssuerAndSerialNumber sequence for specified signature */
	case kPGPSigPropX509IASN:
		{	PGPByte *iasn;
			PGPSize iasnLength;

			if (!ringSigIsX509 (ringset, sigobj))
				return kPGPError_InvalidProperty;
			ptr = ringSigX509Certificate(ringset, sigobj, &len);
			if (IsNull( ptr ) )
				return kPGPError_InvalidProperty;
			error = pgpX509CertToIASN( ptr, len, NULL, &iasnLength);
			if (IsPGPError( error ))
				return error;
			*actualLength = iasnLength;
			if( IsntNull( prop ) ) {
				iasn = (PGPByte *)pgpContextMemAlloc( context, iasnLength, 0);
				if( IsNull( iasn ) )
					return kPGPError_OutOfMemory;
				error = pgpX509CertToIASN( ptr, len, iasn, NULL);
				if (IsPGPError( error ))
					return error;
				if (bufferSize < iasnLength)
					iasnLength = bufferSize;
				pgpCopyMemory( iasn, prop, iasnLength );
				pgpContextMemFree( context, iasn );
			}
			return kPGPError_NoErr;
		}

	/* Return LongName for specified signature */
	case kPGPSigPropX509LongName:
	case kPGPSigPropX509IssuerLongName:
		{	PGPByte *name;
			PGPSize nameLength;
			PGPBoolean doIssuer = (propname==kPGPSigPropX509IssuerLongName);

			if (!ringSigIsX509 (ringset, sigobj))
				return kPGPError_InvalidProperty;
			ptr = ringSigX509Certificate(ringset, sigobj, &len);
			if (IsNull( ptr ) )
				return kPGPError_InvalidProperty;
			error = pgpX509CertToLongName( ptr, len, doIssuer,
										   NULL, &nameLength);
			if (IsPGPError( error ))
				return error;
			*actualLength = nameLength;
			if( IsntNull( prop ) ) {
				name = (PGPByte *)pgpContextMemAlloc( context, nameLength, 0);
				if( IsNull( name ) )
					return kPGPError_OutOfMemory;
				error = pgpX509CertToLongName( ptr, len, doIssuer,
											   name, &nameLength);
				if (IsPGPError( error ))
					return error;
				if (bufferSize < nameLength)
					nameLength = bufferSize;
				pgpCopyMemory( name, prop, nameLength );
				pgpContextMemFree( context, name );
			}
			return kPGPError_NoErr;
		}
	
	/* Return Subject DN for specified signature certificate */
	case kPGPSigPropX509DERDName:
		{	PGPByte *name;
			PGPSize nameLength;

			if (!ringSigIsX509 (ringset, sigobj))
				return kPGPError_InvalidProperty;
			ptr = ringSigX509Certificate(ringset, sigobj, &len);
			if (IsNull( ptr ) )
				return kPGPError_InvalidProperty;
			error = pgpX509CertToDName( ptr, len, FALSE, NULL, &nameLength);
			if (IsPGPError( error ))
				return error;
			*actualLength = nameLength;
			if( IsntNull( prop ) ) {
				name = (PGPByte *)pgpContextMemAlloc( context, nameLength, 0);
				if( IsNull( name ) )
					return kPGPError_OutOfMemory;
				error = pgpX509CertToDName( ptr, len, FALSE,
											name, &nameLength);
				if (IsPGPError( error ))
					return error;
				if (bufferSize < nameLength)
					nameLength = bufferSize;
				pgpCopyMemory( name, prop, nameLength );
				pgpContextMemFree( context, name );
			}
			return kPGPError_NoErr;
		}
	
	/* Return IP or DNS addr for specified signature */
	case kPGPSigPropX509IPAddress:
	case kPGPSigPropX509DNSName:
		{	PGPByte *val;
			PGPSize valLength;
			PGPBoolean doIP = (propname==kPGPSigPropX509IPAddress);

			if (!ringSigIsX509 (ringset, sigobj))
				return kPGPError_InvalidProperty;
			ptr = ringSigX509Certificate(ringset, sigobj, &len);
			if (IsNull( ptr ) )
				return kPGPError_InvalidProperty;
			error = pgpX509CertToIPDNS( ptr, len, doIP, NULL, &valLength);
			if (IsPGPError( error ))
				return error;
			*actualLength = valLength;
			/* Return error if there is no such data */
			if( valLength == 0 )
				return kPGPError_BadParams;
			if( IsntNull( prop ) ) {
				val = (PGPByte *)pgpContextMemAlloc( context, valLength, 0);
				if( IsNull( val ) )
					return kPGPError_OutOfMemory;
				error = pgpX509CertToIPDNS( ptr, len, doIP, val, &valLength);
				if (IsPGPError( error ))
					return error;
				if (bufferSize < valLength)
					valLength = bufferSize;
				pgpCopyMemory( val, prop, valLength );
				pgpContextMemFree( context, val );
			}
			return kPGPError_NoErr;
		}
	}
	
	return pgpReturnPropBuffer ( (char const *)ptr,
			prop, *actualLength, bufferSize);
}


	PGPError
PGPGetSigPropertyBuffer(
	PGPSigRef		cert,
	PGPSigPropName	propname,
	PGPSize			bufferSize,
	void *			outData,
	PGPSize *		outLength )
{
    RingSet const       *ringset;
	PGPContextRef		context;
    PGPError			err	= kPGPError_NoErr;

	PGPValidatePtr( outLength );
	*outLength	= 0;
	PGPValidateCert( cert );
	/* outData is allowed to be NULL */
	if ( IsntNull( outData ) )
	{
		pgpClearMemory( outData, bufferSize );
	}
	
	context = cert->up.userID->key->keyDB->context;
	ringset = pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	
	err	= pgpGetSigStringInternal (context, ringset, cert->cert,
			propname, outData, bufferSize, outLength );

	return( err );
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
	RingSet	const		*ringset = NULL;
	union RingObject    *sigobj = NULL, *obj = NULL;
	PGPKeyID			 keyid, revkeyid;
	RingIterator 		*iter = NULL;
	unsigned			  level;
	PGPTime				expiration;
	
	PGPValidatePtr( prop );
	*prop	= FALSE;
	PGPValidateCert( cert );
	
	CHECKREMOVED(cert);
	ringset =	pgpKeyDBRingSet (cert->up.userID->key->keyDB);
	sigobj =	cert->cert;

	switch (propname) {
	case kPGPSigPropHasUnverifiedRevocation:
		/* True automatically if sig is revoked */
		if( ringSigRevoked (ringset, sigobj) ) {
			*prop = 1;
			break;
		}
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
					       ringSigTimestamp (ringset, obj) >= 
					           ringSigTimestamp (ringset, sigobj)) {
					*prop = 1;
					break;
				}
			}
		}
		ringIterDestroy (iter);
		break;
	case kPGPSigPropIsRevoked:
		*prop = ringSigRevoked (ringset, sigobj);
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
	case kPGPSigPropIsExpired:
		expiration = ringSigExpiration (ringset, sigobj);
		if (expiration == 0)
			*prop = 0;
		else
			*prop = (expiration < (PGPUInt32) PGPGetTime());
		break;
	case kPGPSigPropIsX509:
		*prop = ringSigIsX509 (ringset, sigobj);
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
	sSetKeyAxiomatic( key, FALSE, NULL, 0, FALSE );
	
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

	static PGPError
sGetPrimaryUserID (
	PGPKey *		 key,
	PGPAttributeType attributeType,
	PGPUserIDRef *	 outRef)
{
	const PGPUserID *	userID;
	RingSet	const	*ringset;
	RingObject		*nameobj;
	PGPError		 err	= kPGPError_NoErr;

	ringset = pgpKeyDBRingSet (key->keyDB);
	nameobj = ringKeyPrimaryName (key->key, ringset, (PGPUInt32)attributeType);

	userID = (const PGPUserID *) &key->userIDs;
	for ( ; ; ) 
	{
		userID = userID->next;
		if (userID == (const PGPUserID *)&key->userIDs)
		{
			err		= kPGPError_ItemNotFound;
			userID	= kInvalidPGPUserIDRef;
			break;
		}
		if( userID->removed )
			continue;
		if( userID->userID == nameobj )
			break;
	}

	*outRef	= (PGPUserIDRef)userID;
	
	return err;
}

	PGPError 
PGPGetPrimaryUserID (
	PGPKey *		key,
	PGPUserIDRef *	outRef)
{
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKey( key );

	return sGetPrimaryUserID (key, (PGPAttributeType)0, outRef );
}


	PGPError 
PGPGetPrimaryAttributeUserID (
	PGPKey *		key,
	PGPAttributeType attributeType,
	PGPUserIDRef *	outRef)
{
	PGPValidatePtr( outRef );
	*outRef	= NULL;
	PGPValidateKey( key );

	return sGetPrimaryUserID( key, attributeType, outRef );
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
		return( kInvalidPGPContextRef );
		
	return( ref->context );
}

	PGPContextRef
PGPGetKeyListContext( PGPKeyListRef ref )
{
	if ( ! pgpKeyListIsValid( ref ) )
		return( kInvalidPGPContextRef );
		
	return( PGPGetKeySetContext( ref->keySet ) );
}


	PGPContextRef
PGPGetKeySetContext( PGPKeySetRef ref )
{
	if ( ! pgpKeySetIsValid( ref ) )
		return( kInvalidPGPContextRef );
		
	return( pgpGetKeyDBContext( ref->keyDB ) );
}

	PGPContextRef
PGPGetKeyIterContext( PGPKeyIterRef ref )
{
	if ( ! pgpKeyIterIsValid( ref ) )
		return( kInvalidPGPContextRef );
		
	return( PGPGetKeyListContext( ref->keyList ) );
}

	PGPContextRef
PGPGetKeyContext( PGPKeyRef ref )
{
	if ( ! pgpKeyIsValid( ref ) )
		return( kInvalidPGPContextRef );
		
	return( pgpGetKeyDBContext( ref->keyDB ) );
}

	PGPContextRef
PGPGetSubKeyContext( PGPSubKeyRef ref )
{
	if ( ! pgpSubKeyIsValid( ref ) )
		return( kInvalidPGPContextRef );
		
	return( pgpGetKeyDBContext( ref->key->keyDB ) );
}


	PGPContextRef
PGPGetUserIDContext( PGPUserIDRef ref )
{
	if ( ! pgpUserIDIsValid( ref ) )
		return( kInvalidPGPContextRef );
		
	return( PGPGetKeyContext( ref->key ) );
}

	PGPKeyRef
PGPGetUserIDKey( PGPUserIDRef ref )
{
	if ( ! pgpUserIDIsValid( ref ) )
		return( kInvalidPGPKeyRef );

	return ref->key;
}

	PGPUserIDRef
PGPGetSigUserID( PGPSigRef ref )
{
	PGPUserIDRef userid;

	if ( ! pgpSigIsValid( ref ) )
		return( kInvalidPGPUserIDRef );

	if ( ref->type != uidcert )
		return( kInvalidPGPUserIDRef );

	userid = ref->up.userID;

	if ( ! pgpUserIDIsValid( userid ) )
		return( kInvalidPGPUserIDRef );

	return userid;
}

	PGPKeyRef
PGPGetSigKey( PGPSigRef ref )
{
	PGPUserIDRef userid;

	if ( ! pgpSigIsValid( ref ) )
		return( kInvalidPGPKeyRef );

	if ( ref->type != uidcert )
		return ref->up.key;

	userid = ref->up.userID;

	if ( ! pgpUserIDIsValid( userid ) )
		return( kInvalidPGPKeyRef );

	return userid->key;
}


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
