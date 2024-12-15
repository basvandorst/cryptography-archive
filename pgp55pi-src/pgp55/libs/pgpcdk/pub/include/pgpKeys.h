/*____________________________________________________________________________
	pgpKeys.h
	
	Copyright(C) 1996,1997 Pretty Good Privacy, Inc.
	All rights reserved.
	
	Public definitions for PGP KeyDB Library

	$Id: pgpKeys.h,v 1.94 1997/10/17 06:15:20 hal Exp $
____________________________________________________________________________*/
#ifndef Included_pgpKeys_h	/* [ */
#define Included_pgpKeys_h



#include "pgpPubTypes.h"
#include "pgpHash.h"
#include "pgpEncode.h"



/* Key ordering */
enum PGPKeyOrdering_
{
	kPGPAnyOrdering = 1,
	kPGPUserIDOrdering,
	kPGPReverseUserIDOrdering,
	kPGPKeyIDOrdering,
	kPGPReverseKeyIDOrdering,
	kPGPValidityOrdering,
	kPGPReverseValidityOrdering,
	kPGPTrustOrdering,
	kPGPReverseTrustOrdering,
	kPGPEncryptKeySizeOrdering,
	kPGPReverseEncryptKeySizeOrdering,
	kPGPSigKeySizeOrdering,
	kPGPReverseSigKeySizeOrdering,
	kPGPCreationOrdering,
	kPGPReverseCreationOrdering,
	kPGPExpirationOrdering,
	kPGPReverseExpirationOrdering,
	
	PGP_ENUM_FORCE( PGPKeyOrdering_ )
} ;
PGPENUM_TYPEDEF( PGPKeyOrdering_, PGPKeyOrdering );



/* Key properties */

enum PGPKeyPropName_
{	/* String properties */
	kPGPKeyPropUNUSED_1 = 10,	/* feel free to use this */
	kPGPKeyPropFingerprint,
	kPGPKeyPropPreferredAlgorithms,
	/* Number properties */
	kPGPKeyPropAlgID = 20,
	kPGPKeyPropBits,	
	kPGPKeyPropTrust,			/* old trust model only */
	kPGPKeyPropValidity,    /* both trust models */
	/* Time properties */
	kPGPKeyPropCreation = 30,
	kPGPKeyPropExpiration,
	/* PGPBoolean properties */
	kPGPKeyPropIsSecret = 40,
	kPGPKeyPropIsAxiomatic,
	kPGPKeyPropIsRevoked,
	kPGPKeyPropIsDisabled,
	kPGPKeyPropIsNotCorrupt,
	kPGPKeyPropIsExpired,
	kPGPKeyPropNeedsPassphrase,
	kPGPKeyPropCanSign,
	kPGPKeyPropCanEncrypt,
	kPGPKeyPropHasUnverifiedRevocation,
	
	PGP_ENUM_FORCE( PGPKeyPropName_ )
} ;
PGPENUM_TYPEDEF( PGPKeyPropName_, PGPKeyPropName );

/* User ID properties */

enum PGPUserIDPropName_
{	/* String properties */
	kPGPUserIDPropName = 50,
	/* Number properties */
	kPGPUserIDPropValidity = 60,/* both trust models */
	kPGPUserIDPropConfidence,	/* new trust model only */
	/* Time properties */
	/* PGPBoolean properties */
	
	PGP_ENUM_FORCE( PGPUserIDPropName_ )
} ;
PGPENUM_TYPEDEF( PGPUserIDPropName_, PGPUserIDPropName );

/* Signature properties */

enum PGPSigPropName_
{
	/* String properties */
	kPGPSigPropKeyID = 90,
	/* Number properties */
	kPGPSigPropAlgID = 100,
	kPGPSigPropTrustLevel,
	kPGPSigPropTrustValue,
	/* Time properties */
	kPGPSigPropCreation = 110,
	kPGPSigPropExpiration,
	/* PGPBoolean properties */
	kPGPSigPropIsRevoked = 120,
	kPGPSigPropIsNotCorrupt,
	kPGPSigPropIsTried,
	kPGPSigPropIsVerified,
	kPGPSigPropIsMySig,
	kPGPSigPropIsExportable,
	kPGPSigPropHasUnverifiedRevocation,
	
	PGP_ENUM_FORCE( PGPSigPropName_ )
} ;
PGPENUM_TYPEDEF( PGPSigPropName_, PGPSigPropName );
/*
 * Note on kPGPSigPropIsMySig.  This is a convenience property for
 * determining whether the certification was made by one of the
 * caller's own private keys.  This can only return true if the
 * signing key is in the same base keyset as the certification. If the
 * signing key is( suspected to be) in a different base keyset, call
 * PGPGetSigCertifierKey( certset, signerset, &key) followed by
 * PGPGetKeyBoolean( key, kPGPKeyPropIsSecret, &secret).
 */

enum	/* PGPKeyRingOpenFlags */
{
	kPGPKeyRingOpenFlags_Reserved			= (1UL << 0 ),
	kPGPKeyRingOpenFlags_Mutable			= (1UL << 1 ),
	kPGPKeyRingOpenFlags_Create				= (1UL << 2 ),

	/* The following flags are only used by PGPOpenKeyRing */
	kPGPKeyRingOpenFlags_Private			= (1UL << 8 ),
	kPGPKeyRingOpenFlags_Trusted			= (1UL << 9 )
} ;
typedef PGPUInt32		PGPKeyRingOpenFlags;


/*
 * Used by filtering functions to specify type of match.
 */

enum PGPMatchCriterion_
{
	kPGPMatchDefault = 1,
	kPGPMatchEqual = 1,			 /* searched val == supplied val */
	kPGPMatchGreaterOrEqual = 2, /* searched val >= supplied val */
	kPGPMatchLessOrEqual = 3, /* searched val <= supplied val */
	kPGPMatchSubString = 4,	 /* searched val is contained in supplied val */

	PGP_ENUM_FORCE( PGPMatchCriterion_ )
} ;
PGPENUM_TYPEDEF( PGPMatchCriterion_, PGPMatchCriterion );


/* This is the value of the expiration time which means "never expires" */
#define kPGPExpirationTime_Never	( (PGPTime)0 )


/* Public entry points */

PGP_BEGIN_C_DECLARATIONS
#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


/* Functions for setting up key filters. */

PGPError 		PGPIncFilterRefCount ( PGPFilterRef filter );

PGPError 		PGPFreeFilter ( PGPFilterRef filter );

PGPError 		PGPNewKeyIDFilter( PGPContextRef context, 
						PGPKeyID const * keyID, PGPFilterRef *outFilter);

PGPError		PGPNewSubKeyIDFilter( PGPContextRef		context,
						PGPKeyID const *	subKeyID,  
						PGPFilterRef *		outFilter);

PGPError 		PGPNewKeyEncryptAlgorithmFilter( PGPContextRef context, 
						PGPPublicKeyAlgorithm encryptAlgorithm,
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyFingerPrintFilter( PGPContextRef context, 
						void const *fingerPrint, 
						PGPSize fingerPrintLength, PGPFilterRef *outFilter );

PGPError 		PGPNewKeyCreationTimeFilter( PGPContextRef context, 
						PGPTime creationTime, PGPMatchCriterion match, 
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyExpirationTimeFilter( PGPContextRef context, 
						PGPTime expirationTime, 
						PGPMatchCriterion match, PGPFilterRef *outFilter );

PGPError 		PGPNewKeyRevokedFilter( PGPContextRef context,
						PGPBoolean revoked, PGPFilterRef *outFilter );

PGPError		PGPNewKeyDisabledFilter( PGPContextRef context, 
						PGPBoolean			disabled, 
						PGPFilterRef *		outFilter);

PGPError 		PGPNewKeySigAlgorithmFilter( PGPContextRef context,
						PGPPublicKeyAlgorithm sigAlgorithm, 
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeyEncryptKeySizeFilter( PGPContextRef context,
						PGPUInt32 keySize, PGPMatchCriterion match,
						PGPFilterRef *outFilter );

PGPError 		PGPNewKeySigKeySizeFilter( PGPContextRef context,
						PGPUInt32 keySize, PGPMatchCriterion match,
						PGPFilterRef *outFilter );

PGPError		PGPNewUserIDStringFilter( PGPContextRef context,
						char const *		userIDString, 
						PGPMatchCriterion	match,
						PGPFilterRef *		outFilter);

PGPError 		PGPNewUserIDEmailFilter( PGPContextRef context,
						char const *		emailString, 
						PGPMatchCriterion	match,
						PGPFilterRef *		outFilter);

PGPError 		PGPNewUserIDNameFilter( PGPContextRef context,
						char const *		nameString, 
						PGPMatchCriterion	match,
						PGPFilterRef *		outFilter);

PGPError 		PGPNewSigKeyIDFilter( PGPContextRef context,
						PGPKeyID const * keyID, PGPFilterRef *outFilter );


/* freeing outfilter will call PGPFreeFilter on filter */
PGPError 		PGPNegateFilter( PGPFilterRef filter,
						PGPFilterRef *outFilter);

/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
PGPError 		PGPIntersectFilters( PGPFilterRef filter1,
						PGPFilterRef filter2, PGPFilterRef *outFilter);

/* freeing outfilter will call PGPFreeFilter on filter1, filter2 */
PGPError 		PGPUnionFilters( PGPFilterRef filter1,
						PGPFilterRef filter2, PGPFilterRef *outFilter);


PGPError 		PGPFilterKeySet( PGPKeySetRef origSet,
						PGPFilterRef filter, PGPKeySetRef *resultSet);


/* Keyserver filter functions */

PGPError 		PGPLDAPQueryFromFilter( PGPFilterRef filter,
						char **queryOut );

PGPError 		PGPHKSQueryFromFilter( PGPFilterRef	filter,
						char **queryOut );

/* KeySet manipulations */


/* Creates a new memory-based KeyDB and returns its root set */
PGPError  		PGPNewKeySet(PGPContextRef context, PGPKeySetRef *keySet);


/* Used for building arbitrary subsets of an existing KeyDB */
PGPError  		PGPNewEmptyKeySet(PGPKeySetRef origSet, PGPKeySetRef *newSet);
PGPError  		PGPNewSingletonKeySet(PGPKeyRef key, PGPKeySetRef *keySet);
PGPError		PGPUnionKeySets(PGPKeySetRef set1, PGPKeySetRef set2,
								PGPKeySetRef *newSet);

PGPError  		PGPOpenDefaultKeyRings( PGPContextRef context,
						PGPKeyRingOpenFlags openFlags, PGPKeySetRef *keySet);

PGPError  		PGPOpenKeyRingPair( PGPContextRef context,
						PGPKeyRingOpenFlags openFlags, 
						PGPFileSpecRef pubFileRef, PGPFileSpecRef secFileRef,
						PGPKeySetRef *keySet);

PGPError  		PGPOpenKeyRing (PGPContextRef context,
						PGPKeyRingOpenFlags openFlags, PGPFileSpecRef fileRef,
						PGPKeySetRef *keySet);

PGPError 		PGPCommitKeyRingChanges (PGPKeySetRef keys);

PGPError 		PGPPropagateTrust (PGPKeySetRef keys);

PGPError 		PGPRevertKeyRingChanges (PGPKeySetRef keys);

PGPError 		PGPCheckKeyRingSigs (PGPKeySetRef keysToCheck,
						PGPKeySetRef keysSigning, PGPBoolean checkAll,
						PGPEventHandlerProcPtr progress,
						PGPUserValue userValue);

PGPError 		PGPReloadKeyRings (PGPKeySetRef keys);

PGPError 		PGPGetKeyByKeyID (PGPKeySetRef keys,
						PGPKeyID const * keyID,
						PGPPublicKeyAlgorithm pubKeyAlgorithm,
						PGPKeyRef *outRef);

PGPBoolean 		PGPKeySetIsMember(PGPKeyRef key, PGPKeySetRef set);

PGPError 		PGPCountKeys( PGPKeySetRef keys, PGPUInt32 *numKeys);

PGPError 		PGPIncKeySetRefCount( PGPKeySetRef keys);

PGPError 		PGPFreeKeySet( PGPKeySetRef keys);

PGPBoolean 		PGPKeySetIsMutable( PGPKeySetRef keys);

PGPBoolean 		PGPKeySetNeedsCommit( PGPKeySetRef keys);


PGPError 		PGPAddKeys( PGPKeySetRef keysToAdd, PGPKeySetRef set );

PGPError 		PGPRemoveKeys( PGPKeySetRef keysToRemove, PGPKeySetRef set );



/* Key manipulation functions */

PGPError 		PGPDisableKey( PGPKeyRef key);

PGPError 		PGPEnableKey( PGPKeyRef key);

PGPError 		PGPChangePassphrase( PGPKeyRef key, char const *oldphrase,
						char const *newphrase);

PGPError 		PGPChangeSubKeyPassphrase( PGPSubKeyRef subkey,
						   char const *oldphrase, char const *newphrase);

PGPError 		PGPRemoveSubKey( PGPSubKeyRef subkey);

PGPError 		PGPRemoveUserID( PGPUserIDRef userID);

PGPError 		PGPSetPrimaryUserID( PGPUserIDRef userid);

PGPError 		PGPGetSigCertifierKey( PGPSigRef cert, PGPKeySetRef allkeys,
						PGPKeyRef *certkey);

PGPError 		PGPRemoveSig( PGPSigRef cert);

PGPError		PGPCountAdditionalRecipientRequests( PGPKeyRef basekey,
					PGPUInt32 * numARKeys);
					
PGPError 		PGPGetIndexedAdditionalRecipientRequest(
						PGPKeyRef basekey, PGPKeySetRef allkeys, PGPUInt32 nth,
						PGPKeyRef *arkey, PGPByte *arclass );
					
PGPError 		PGPGetIndexedAdditionalRecipientRequestKeyID(
						PGPKeyRef basekey, PGPUInt32 nth, PGPKeyID *arkeyid,
						PGPByte *arclass );

PGPBoolean		PGPPassphraseIsValid( PGPKeyRef key, const char *passphrase );

/* Wrapper functions */
PGPError			PGPGenerateKey( PGPContextRef context, PGPKeyRef *key,
						PGPOptionListRef firstOption, ...);
PGPError			PGPGenerateSubKey( PGPContextRef context,
						PGPSubKeyRef *subkey,
						PGPOptionListRef firstOption, ...);
PGPUInt32			PGPGetKeyEntropyNeeded( PGPContextRef context,
						PGPOptionListRef firstOption, ...);
PGPError			PGPExportKeySet( PGPKeySetRef keys,
						PGPOptionListRef firstOption, ...);
PGPError			PGPImportKeySet( PGPContextRef context,
						PGPKeySetRef *keys,
						PGPOptionListRef firstOption, ...);
PGPError			PGPSignUserID( PGPUserIDRef userID,
						PGPKeyRef certifyingKey,
						PGPOptionListRef firstOption, ...);
PGPError			PGPAddUserID( PGPKeyRef key, char const *userID,
						PGPOptionListRef firstOption, ...);
PGPError			PGPRevokeSig( PGPSigRef cert, PGPKeySetRef allkeys,
						PGPOptionListRef firstOption, ...);
PGPError			PGPRevokeKey( PGPKeyRef key,
						PGPOptionListRef firstOption, ...);
PGPError			PGPRevokeSubKey( PGPSubKeyRef subkey,
						PGPOptionListRef firstOption, ...);


/*
 * Trust values for PGPSetKeyTrust and kPGPKeyPropTrust property:
 * 
 * kPGPKeyTrust_Undefined	(do not pass to PGPSetKeyTrust)
 * kPGPKeyTrust_Unknown		(unknown)
 * kPGPKeyTrust_Never		(never)
 * kPGPKeyTrust_Marginal	(sometimes)
 * kPGPKeyTrust_Complete	(always)
 * kPGPKeyTrust_Ultimate	(do not pass to PGPSetKeyTrust)
 */

 /* old trust model */
PGPError 		PGPSetKeyTrust( PGPKeyRef key, PGPUInt32 trust);

PGPError 		PGPSetKeyAxiomatic( PGPKeyRef key,
						PGPBoolean checkPassphrase, char const *passphrase);

PGPError 		PGPUnsetKeyAxiomatic( PGPKeyRef key);

/*  Get property functions */

PGPError 		PGPGetKeyBoolean( PGPKeyRef key, PGPKeyPropName propname,
						PGPBoolean *prop);

PGPError 		PGPGetKeyNumber( PGPKeyRef key, PGPKeyPropName propname,
						PGPInt32 *prop);

/* 'buffer' is NOT null-terminated */
/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetKeyPropertyBuffer( PGPKeyRef key,
						PGPKeyPropName propname,
						PGPSize bufferSize, void *data, PGPSize *fullSize);

PGPError 		PGPGetKeyTime( PGPKeyRef key, PGPKeyPropName propname,
						PGPTime *prop);

PGPError 		PGPGetSubKeyBoolean( PGPSubKeyRef subkey,
						PGPKeyPropName propname, PGPBoolean *prop);

PGPError 		PGPGetSubKeyNumber( PGPSubKeyRef subkey,
						PGPKeyPropName propname, PGPInt32 *prop);

/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetSubKeyPropertyBuffer( PGPSubKeyRef subkey,
						PGPKeyPropName propname,
						PGPSize bufferSize, void *prop, PGPSize *fullSize);

PGPError 		PGPGetSubKeyTime( PGPSubKeyRef subkey,
						PGPKeyPropName propname, PGPTime *prop);

PGPError 		PGPGetUserIDNumber( PGPUserIDRef userID,
						PGPUserIDPropName propname, PGPInt32 *prop);

/* 'string' is always NULL-terminated */
/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetUserIDStringBuffer( PGPUserIDRef userID,
						PGPUserIDPropName propname,
						PGPSize bufferSize,
						char *		string,
						PGPSize *	fullSize);

PGPError 		PGPGetSigBoolean( PGPSigRef cert,
						PGPSigPropName propname, PGPBoolean *prop);

PGPError 		PGPGetSigNumber( PGPSigRef cert, PGPSigPropName propname,
						PGPInt32 *prop);
						
PGPError 		PGPGetKeyIDOfCertifier( PGPSigRef sig, PGPKeyID *outID );
	
PGPError 		PGPGetSigTime( PGPSigRef cert, PGPSigPropName propname,
						PGPTime *prop);

PGPError 		PGPGetHashAlgUsed( PGPKeyRef key, PGPHashAlgorithm *hashAlg);


/* Convenience property functions */

PGPError  		PGPGetPrimaryUserID( PGPKeyRef key, PGPUserIDRef *outRef );

/* 'string' is always a C string and
	*fullSize includes the '\0' terminator */
/* returns kPGPError_BufferTooSmall if buffer is too small, but
fill buffer to capacity */
PGPError 		PGPGetPrimaryUserIDNameBuffer( PGPKeyRef key, 
					PGPSize bufferSize,
					char *	string, PGPSize *fullSize );

PGPError 		PGPGetPrimaryUserIDValidity(PGPKeyRef key,
					PGPValidity *validity);


PGPInt32 		PGPCompareKeys(PGPKeyRef a, PGPKeyRef b,
						PGPKeyOrdering order);

PGPInt32 		PGPCompareUserIDStrings(char const *a, char const *b);

PGPError  		PGPOrderKeySet( PGPKeySetRef src,
						PGPKeyOrdering order, PGPKeyListRef *outRef );

PGPError 		PGPIncKeyListRefCount( PGPKeyListRef keys);

PGPError 		PGPFreeKeyList( PGPKeyListRef keys);


/* Key iteration functions */



PGPError 		PGPNewKeyIter (PGPKeyListRef keys, PGPKeyIterRef *outRef);

PGPError 		PGPCopyKeyIter (PGPKeyIterRef orig, PGPKeyIterRef *outRef);
	
PGPError 		PGPFreeKeyIter (PGPKeyIterRef iter);

PGPInt32 		PGPKeyIterIndex (PGPKeyIterRef iter);

PGPError 		PGPKeyIterRewind (PGPKeyIterRef iter);

PGPInt32 		PGPKeyIterSeek (PGPKeyIterRef iter, PGPKeyRef key);

PGPError 		PGPKeyIterMove (PGPKeyIterRef iter, PGPInt32 relOffset,
						PGPKeyRef *outRef);

PGPError 		PGPKeyIterNext (PGPKeyIterRef iter, PGPKeyRef *outRef);

PGPError 		PGPKeyIterPrev (PGPKeyIterRef iter, PGPKeyRef *outRef);

PGPError 		PGPKeyIterKey (PGPKeyIterRef iter, PGPKeyRef *outRef);

PGPError  		PGPKeyIterSubKey (PGPKeyIterRef iter,
						PGPSubKeyRef *outRef );

PGPError  		PGPKeyIterUserID (PGPKeyIterRef iter,
						PGPUserIDRef *outRef );
PGPError  		PGPKeyIterSig (PGPKeyIterRef iter,
						PGPSigRef *outRef );

PGPError  		PGPKeyIterNextSubKey (PGPKeyIterRef iter,
						PGPSubKeyRef *outRef );

PGPError  		PGPKeyIterPrevSubKey (PGPKeyIterRef iter,
						PGPSubKeyRef *outRef);

PGPError 		PGPKeyIterRewindSubKey (PGPKeyIterRef iter);

PGPError  		PGPKeyIterNextUserID (PGPKeyIterRef iter,
						PGPUserIDRef *outRef);

PGPError  		PGPKeyIterPrevUserID (PGPKeyIterRef iter,
						PGPUserIDRef *outRef);

PGPError 		PGPKeyIterRewindUserID (PGPKeyIterRef iter);

PGPError  		PGPKeyIterNextUIDSig (PGPKeyIterRef iter,
						PGPSigRef *outRef);

PGPError  		PGPKeyIterPrevUIDSig (PGPKeyIterRef iter,
						PGPSigRef *outRef);

PGPError 		PGPKeyIterRewindUIDSig (PGPKeyIterRef iter);


/* Get/set default private key */

PGPError 		PGPGetDefaultPrivateKey( PGPKeySetRef keyset,
						PGPKeyRef *outRef );

PGPError 		PGPSetDefaultPrivateKey( PGPKeyRef key);



/* Get/set user value */

PGPError 		PGPSetKeyUserVal( PGPKeyRef key, PGPUserValue userValue);

PGPError 		PGPSetUserIDUserVal( PGPUserIDRef userid,
						PGPUserValue userValue);

PGPError 		PGPSetSubKeyUserVal( PGPSubKeyRef subkey,
						PGPUserValue userValue);

PGPError 		PGPSetSigUserVal( PGPSigRef cert,
						PGPUserValue userValue);

PGPError 		PGPGetKeyUserVal( PGPKeyRef key,
						PGPUserValue *userValue);

PGPError 		PGPGetUserIDUserVal( PGPUserIDRef userid,
						PGPUserValue *userValue);

PGPError 		PGPGetSubKeyUserVal( PGPSubKeyRef subkey,
						PGPUserValue *userValue);

PGPError 		PGPGetSigUserVal( PGPSigRef cert,
						PGPUserValue *userValue);


/*____________________________________________________________________________
	Key IDs
____________________________________________________________________________*/
PGPError 	PGPGetKeyIDFromString(
				const char *string, PGPKeyID *id );
					
PGPError 	PGPGetKeyIDFromKey( PGPKeyRef key, PGPKeyID *id );

PGPError 	PGPGetKeyIDFromSubKey( PGPSubKeyRef key, PGPKeyID * id );

enum PGPKeyIDStringType_
{
	kPGPKeyIDString_Abbreviated = 2,
	kPGPKeyIDString_Full = 3,
	PGP_ENUM_FORCE( PGPKeyIDStringType_ )
};
PGPENUM_TYPEDEF( PGPKeyIDStringType_, PGPKeyIDStringType );


#define kPGPMaxKeyIDStringSize		( 127 + 1 )
PGPError 	PGPGetKeyIDString( PGPKeyID const * ref,
					PGPKeyIDStringType type,
					char outString[ kPGPMaxKeyIDStringSize ] );


/* outputs opaque string of bytes for storage of maximum size as keyID*/
/* do NOT attempt to parse the output; it is opaque to you */
#define kPGPMaxExportedKeyIDSize		( sizeof( PGPKeyID ) )
PGPError 	PGPExportKeyID( PGPKeyID const * keyID,
					PGPByte exportedData[ kPGPMaxExportedKeyIDSize ],
					PGPSize *exportedLength );

/* must be in format output by PGPExportKeyID */
PGPError 	PGPImportKeyID(  void const * data, PGPKeyID * id );

/* returns 0 if equal, -1 if key1 < key2, 1 if key1 > key2 */
PGPInt32 	PGPCompareKeyIDs( PGPKeyID const * key, PGPKeyID const * key2);




/*____________________________________________________________________________
	Getting contexts back from key related items.  If the key is invalid,
	you get kPGPInvalidRef back.
____________________________________________________________________________*/

PGPContextRef	PGPGetKeyListContext( PGPKeyListRef ref );
PGPContextRef	PGPGetKeySetContext( PGPKeySetRef ref );
PGPContextRef	PGPGetKeyIterContext( PGPKeyIterRef ref );
PGPContextRef	PGPGetKeyContext( PGPKeyRef ref );
PGPContextRef	PGPGetSubKeyContext( PGPSubKeyRef ref );
PGPContextRef	PGPGetUserIDContext( PGPUserIDRef ref );





#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif
PGP_END_C_DECLARATIONS


#endif /* ] Included_pgpKeys_h */

/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/
