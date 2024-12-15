/*
 * pgpKeyDB.h -- Public definitions for PGP KeyDB Library
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpKeyDB.h,v 1.72.2.12.2.2 1997/07/08 02:24:14 mhw Exp $
 */

#ifndef PGPKEYDB_H
#define PGPKEYDB_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

#include <time.h>

#include "pgpDebug.h"
#include "pgpTypes.h"
#include "pgpUsuals.h"
#include "pgpErr.h"
#include "pgpFileRef.h"
#include "pgpRngPub.h"
#include "pgpPubKey.h"
#include "pgpRndPool.h"
#include "pgpTrstPkt.h"
#include "pgpTrust.h"
#include "pgpHash.h"
#include "pgpTimeDate.h"

#ifndef TYPE_PGPKEYDB
#define TYPE_PGPKEYDB
typedef struct PGPKeyDB_	PGPKeyDB;
#endif

#ifndef TYPE_PGPKEY
#define TYPE_PGPKEY
typedef struct PGPKey_		PGPKey;
#endif

#ifndef TYPE_PGPSUBKEY
#define TYPE_PGPSUBKEY
typedef struct PGPSubKey_ PGPSubKey;
#endif

#ifndef TYPE_PGPUSERID
#define TYPE_PGPUSERID
typedef struct PGPUserID_	PGPUserID;
#endif

#ifndef TYPE_PGPCERT
#define TYPE_PGPCERT
typedef struct PGPCert_		PGPCert;
#endif

#ifndef TYPE_PGPKEYSET
#define TYPE_PGPKEYSET
typedef struct PGPKeySet_	PGPKeySet;
#endif

#ifndef TYPE_PGPKEYLIST
#define TYPE_PGPKEYLIST
typedef struct PGPKeyList_	PGPKeyList;
#endif

#ifndef TYPE_PGPKEYITER
#define TYPE_PGPKEYITER
typedef struct PGPKeyIter_	PGPKeyIter;
#endif

/* Key ordering */

typedef enum PGPKeyOrdering {
	kPGPAnyOrdering = 1,
	kPGPUserIDOrdering,
	kPGPReverseUserIDOrdering,
	kPGPKeyIDOrdering,
	kPGPReverseKeyIDOrdering,
	kPGPValidityOrdering,
	kPGPReverseValidityOrdering,
	kPGPTrustOrdering,
	kPGPReverseTrustOrdering,
	kPGPKeySizeOrdering,
	kPGPReverseKeySizeOrdering,
	kPGPCreationOrdering,
	kPGPReverseCreationOrdering
} PGPKeyOrdering;


/* Key and UserID validity values for kPGPKeyPropValidity and
	kPGPUserIDPropValidity properties. */

#define PGP_VALIDITY_UNKNOWN	PGP_NAMETRUST_UNKNOWN
#define PGP_VALIDITY_INVALID	PGP_NAMETRUST_UNTRUSTED
#define PGP_VALIDITY_MARGINAL	PGP_NAMETRUST_MARGINAL
#define PGP_VALIDITY_COMPLETE	PGP_NAMETRUST_COMPLETE

#define PGP_VALIDITY_UNTRUSTED PGP_VALIDITY_INVALID


/* Key properties */

typedef enum {
	/* String properties */
	kPGPKeyPropKeyId = 10,
	kPGPKeyPropFingerprint,
	/* Number properties */
	kPGPKeyPropAlgId,	
	kPGPKeyPropBits,	
	kPGPKeyPropTrust,	 		 /* old trust model only */
	kPGPKeyPropTrustVal = kPGPKeyPropTrust,
	kPGPKeyPropValidity, /* both trust models */
	/* Time properties */
	kPGPKeyPropCreation,
	kPGPKeyPropExpiration,
	/* Boolean properties */
	kPGPKeyPropIsSecret,
	kPGPKeyPropIsAxiomatic,
	kPGPKeyPropIsRevoked,
	kPGPKeyPropIsDisabled,
	kPGPKeyPropIsNotCorrupt,
	kPGPKeyPropIsValid = kPGPKeyPropIsNotCorrupt,
	kPGPKeyPropIsExpired,
	kPGPKeyPropNeedsPassphrase,
	kPGPKeyPropCanSign,
	kPGPKeyPropCanEncrypt
} PGPKeyPropName;

/* User ID properties */

typedef enum {
	/* String properties */
	kPGPUserIDPropName = 30,
	/* Number properties */
	kPGPUserIDPropValidity,		/* both trust models */
	kPGPUserIDPropTrustVal = kPGPUserIDPropValidity,
	kPGPUserIDPropConfidence	/* new trust model only */
	/* Time properties */
	/* Boolean properties */
} PGPUserIDPropName;

/* Certificate properties */

typedef enum {
	/* String properties */
	kPGPCertPropKeyID = 50,
	/* Number properties */
	/* Time properties */
	kPGPCertPropCreation,
	/* Boolean properties */
	kPGPCertPropIsRevoked,
	kPGPCertPropIsNotCorrupt,
	kPGPCertPropIsValid = kPGPCertPropIsNotCorrupt,
	kPGPCertPropIsTried,
	kPGPCertPropIsVerified,
	kPGPCertPropIsChecked = kPGPCertPropIsVerified,
	kPGPCertPropIsMyCert
} PGPCertPropName;

/*
 * Note on kPGPCertPropIsMyCert. This is a convenience property for
 * determining whether the certification was made by one of the
 * caller's own private keys. This can only return true if the
 * signing key is in the same base keyset as the certification. If the
 * signing key is (suspected to be) in a different base keyset, call
 * pgpGetCertifier (certset, signerset, &key) followed by
 * pgpGetKeyBoolean (key, kPGPKeyPropIsSecret, &secret).
 */

/*
 * Typedefs for preference file access
 *
 * WARNING:
 *   Note that pgpPrefsInt.h, pgpPrefMac.c and pgpPrefXXX.c must
 *   be updated if these change.
 *
 *   Also, always add new preferences to the end!!!
 */

typedef enum PGPPrefNameNumber_ {
	kPGPPrefNumFirst = 100,
	kPGPPrefVersion = kPGPPrefNumFirst,
	kPGPPrefCipherNum,
	kPGPPrefHashNum,
	kPGPPrefCompressNum,
	kPGPPrefArmorLines,
	kPGPPrefCertDepth,
	kPGPPrefCompressQual,
	kPGPPrefMailWordWrapWidth,
	kPGPPrefMailPassCacheDuration,	/* seconds */
	kPGPPrefSignCacheDuration,		/* seconds */
	kPGPPrefDateOfLastSplashScreen,	/* PGPTime */
	kPGPPrefHTTPKeyServerPort,
	kPGPPrefNumLast
} PGPPrefNameNumber;

typedef enum PGPPrefNameString_ {
	kPGPPrefStrFirst = 200,
	kPGPPrefUserID = kPGPPrefStrFirst,
	kPGPPrefPubRing,
	kPGPPrefPrivRing,
	kPGPPrefRandomDevice,
	kPGPPrefRandSeed,
	kPGPPrefLanguage,
	kPGPPrefOwnerName,
	kPGPPrefCompanyName,
	kPGPPrefLicenseNumber,
	kPGPPrefHTTPKeyServerHost,
	kPGPPrefLanguageFile,
	kPGPPrefStrLast
} PGPPrefNameString;

typedef enum PGPPrefNameBoolean_ {
	kPGPPrefBoolFirst = 300,
	kPGPPrefArmor = kPGPPrefBoolFirst,
	kPGPPrefEncryptToSelf,
	kPGPPrefCompress,
	kPGPPrefMailWordWrapEnable,
	kPGPPrefMailPassCacheEnable,
	kPGPPrefMailEncryptPGPMIME,
	kPGPPrefMailSignPGPMIME,
	kPGPPrefFastKeyGen,
	kPGPPrefSignCacheEnable,
	kPGPPrefWarnOnMixRSAElGamal,
	kPGPPrefMailEncryptDefault,
	kPGPPrefMailSignDefault,
	kPGPPrefMarginallyValidWarning,
	kPGPPrefFirstKeyGenerated,
	kPGPPrefWarnOnRSARecipAndNonRSASigner,
	kPGPPrefBoolLast
} PGPPrefNameBoolean;

typedef enum PGPPrefNameData_ {
	kPGPPrefDataFirst = 400,
	kPGPPrefPubRingRef = kPGPPrefDataFirst,
	kPGPPrefPrivRingRef,
	kPGPPrefRandSeedData,
	kPGPPrefPGPkeysMacMainWinPos,
	kPGPPrefPGPappMacPrivateData,
	kPGPPrefPGPmenuMacAppSignatures,
	kPGPPrefDataLast
} PGPPrefNameData;


/* Public entry points */

#ifdef __cplusplus
extern "C" {
#endif


/* Library initialization and cleanup */

PGPError PGPKDBExport
pgpLibInit (void);

void PGPKDBExport
pgpLibCleanup (void);

Boolean PGPKDBExport
pgpOpenSemaphore (void);

void PGPKDBExport
pgpCloseSemaphore (void);

/* KeySet manipulations */

PGPKeySet PGPKDBExport *
pgpNewKeySet (void);

PGPKeySet PGPKDBExport *
pgpNewSingletonKeySet (PGPKey *key);

PGPKeySet PGPKDBExport *
pgpCopyKeySet (PGPKeySet *orig);

PGPKeySet PGPKDBExport *
pgpFilterKeySetUserID (PGPKeySet *origset, char const *name, size_t namelen);

PGPKeySet PGPKDBExport *
pgpFilterKeySetKeyID (PGPKeySet *origset, byte const *keyID,
					size_t keyIDLength);

PGPKeySet PGPKDBExport *
pgpFilterKeySetAuto (PGPKeySet *origset, char const *pattern);

PGPKeySet PGPKDBExport *
pgpOpenDefaultKeyRings (Boolean isMutable, PGPError *errorPtr);

PGPKeySet PGPKDBExport *
pgpOpenKeyRingPair (Boolean isMutable, PGPFileRef *pubFileRef,
PGPFileRef *secFileRef, PGPError *errorPtr);

PGPKeySet PGPKDBExport *
pgpOpenKeyRing (Boolean isMutable, Boolean isPrivate, Boolean isTrusted,
PGPFileRef *fileRef, PGPError *errorPtr);

PGPError PGPKDBExport
pgpCommitKeyRingChanges (PGPKeySet *keys);

PGPError PGPKDBExport
pgpRevertKeyRingChanges (PGPKeySet *keys);

PGPError PGPKDBExport
pgpCheckKeyRingCertificates (PGPKeySet *keys);

PGPError PGPKDBExport
pgpReloadKeyRings (PGPKeySet *keys);

PGPKeySet PGPKDBExport *
pgpUnionKeySets (PGPKeySet *a, PGPKeySet *b);

PGPKey PGPKDBExport *
pgpGetKeyByKeyID (PGPKeySet *keys, byte *keyID, size_t keyIDLength);

Boolean PGPKDBExport
pgpKeySetMember(PGPKeySet *set, PGPKey *key);

long PGPKDBExport
pgpCountKeys (PGPKeySet *keys);

void PGPKDBExport
pgpIncKeySetRefCount (PGPKeySet *keys);

void PGPKDBExport
pgpFreeKeySet (PGPKeySet *keys);

Boolean PGPKDBExport
pgpKeySetIsMutable (PGPKeySet *keys);

Boolean PGPKDBExport
pgpKeySetIsDirty (PGPKeySet *keys);

PGPKeySet PGPKDBExport *
pgpImportKeyFile (PGPFileRef *fileRef);

PGPKeySet PGPKDBExport *
pgpImportKeyBuffer (byte *buffer, size_t length);

PGPError PGPKDBExport
pgpExportKeyFile (PGPKeySet *keys, PGPFileRef *fileRef);

PGPError PGPKDBExport
pgpExportKeyBuffer (PGPKeySet *keys, byte *buffer, size_t *length);

PGPError PGPKDBExport
pgpIncKeyRefCount (PGPKey *key);

PGPError PGPKDBExport
pgpFreeKey (PGPKey *key);

PGPError PGPKDBExport
pgpAddKeys (PGPKeySet *set, PGPKeySet *keysToAdd);

PGPError PGPKDBExport
pgpRemoveKeys (PGPKeySet *set, PGPKeySet *keysToRemove);


/* Key manipulation functions */

PGPError PGPKDBExport
pgpRevokeKey (PGPKey *key, char *passphrase);

unsigned PGPKDBExport
pgpQueryEntropyNeeded (byte pkalg, unsigned bits);

unsigned PGPKDBExport
pgpKeyEntropyNeeded (byte pkalg, unsigned bits);

PGPError PGPKDBExport
pgpAddRandomBits (byte *random_bits, unsigned num_bytes);

PGPError PGPKDBExport
pgpGenerateKey (PGPKeySet *keyset, byte pkalg, unsigned bits,
				word16 expiration, char *name, int nameLen, char *passphrase,
				int (*progress) (void *arg, int phase), void *arg,
				PGPKey **key);

PGPError PGPKDBExport
pgpGenerateSubKey (PGPKey *masterkey, unsigned bits, word16 expiration,
				char *passphrase, char *masterkeyphrase,
				int (*progress) (void *arg, int phase), void *arg);

PGPError PGPKDBExport
pgpDisableKey (PGPKey *key);

PGPError PGPKDBExport
pgpEnableKey (PGPKey *key);

PGPError PGPKDBExport
pgpChangePassphrase (PGPKey *key, char *oldphrase, char *newphrase);

PGPError PGPKDBExport
pgpChangeSubKeyPassphrase (PGPSubKey *subkey,
						char *oldphrase, char *newphrase);

PGPError PGPKDBExport
pgpRemoveSubKey (PGPSubKey *subkey);

PGPError PGPKDBExport
pgpRevokeSubKey (PGPSubKey *subkey, char *passphrase);

PGPError PGPKDBExport
pgpRemoveUserID (PGPUserID *userID);

PGPError PGPKDBExport
pgpAddUserID (PGPKey *key, char *name, int nameLen, char *passphrase);

PGPError PGPKDBExport
pgpSetPrimaryUserID (PGPUserID *userid);

PGPError PGPKDBExport
pgpCertifyUserID (PGPUserID *userID, PGPKey *certifying_key, char *passphrase);

PGPError PGPKDBExport
pgpGetCertifier (PGPCert *cert, PGPKeySet *allkeys, PGPKey **certkey);

PGPError PGPKDBExport
pgpRevokeCert (PGPCert *cert, PGPKeySet *allkeys, char *passphrase);

PGPError PGPKDBExport
pgpRemoveCert (PGPCert *cert);

PGPError PGPKDBExport
pgpGetMessageRecoveryKey (PGPKey *basekey, PGPKeySet *allkeys, unsigned nth,
	PGPKey **mrkey, byte *mrclass, unsigned *numbermrks);

/* New trust model */
PGPError PGPKDBExport
pgpSetUserIDConf (PGPUserID *userID, unsigned long confidence);		

/*
 * Trust values for pgpSetKeyTrust and kPGPKeyPropTrust property:
 *
 * PGP_KEYTRUST_UNDEFINED (do not pass to pgpSetKeyTrust)
 * PGP_KEYTRUST_UNKNOWN	 (unknown)
 * PGP_KEYTRUST_NEVER		 (never)
 * PGP_KEYTRUST_MARGINAL	(sometimes)
 * PGP_KEYTRUST_COMPLETE	(always)
 * PGP_KEYTRUST_ULTIMATE (do not pass to pgpSetKeyTrust)
 */

PGPError PGPKDBExport
pgpSetKeyTrust (PGPKey *key, unsigned char trust);	/* old trust model */

PGPError PGPKDBExport
pgpSetKeyAxiomatic (PGPKey *key, Boolean checkPassphrase, char *passphrase);

PGPError PGPKDBExport
pgpUnsetKeyAxiomatic (PGPKey *key);

/* Get property functions */

PGPError PGPKDBExport
pgpGetKeyBoolean (PGPKey *key, PGPKeyPropName propname, Boolean *prop);

PGPError PGPKDBExport
pgpGetKeyNumber (PGPKey *key, PGPKeyPropName propname, long *prop);

PGPError PGPKDBExport
pgpGetKeyString (PGPKey *key, PGPKeyPropName propname, char *prop,
				size_t *len);

PGPError PGPKDBExport
pgpGetKeyTime (PGPKey *key, PGPKeyPropName propname, PGPTime *prop);

PGPError PGPKDBExport
pgpGetSubKeyBoolean (PGPSubKey *subkey, PGPKeyPropName propname,
				Boolean *prop);

PGPError PGPKDBExport
pgpGetSubKeyNumber (PGPSubKey *subkey, PGPKeyPropName propname, long *prop);

PGPError PGPKDBExport
pgpGetSubKeyString (PGPSubKey *subkey, PGPKeyPropName propname, char *prop,
				size_t *len);

PGPError PGPKDBExport
pgpGetSubKeyTime (PGPSubKey *subkey, PGPKeyPropName propname, PGPTime *prop);

PGPError PGPKDBExport
pgpGetUserIDBoolean (PGPUserID *userID, PGPUserIDPropName propname,
					Boolean *prop);

PGPError PGPKDBExport
pgpGetUserIDNumber (PGPUserID *userID, PGPUserIDPropName propname, long *prop);

PGPError PGPKDBExport
pgpGetUserIDString (PGPUserID *userID, PGPUserIDPropName propname,
					char *prop, size_t *len);

PGPError PGPKDBExport
pgpGetCertBoolean (PGPCert *cert, PGPCertPropName propname, Boolean *prop);

PGPError PGPKDBExport
pgpGetCertNumber (PGPCert *cert, PGPCertPropName propname, long *prop);

PGPError PGPKDBExport
pgpGetCertString (PGPCert *cert, PGPCertPropName propname, char *prop,
				size_t *len);

PGPError PGPKDBExport
pgpGetCertTime (PGPCert *cert, PGPCertPropName propname, PGPTime *prop);

PGPError PGPKDBExport
pgpGetHashAlgUsed (PGPKey *key, long *hashAlg);


/* Convenience property functions */

PGPUserID PGPKDBExport *
pgpGetPrimaryUserID(PGPKey *key);

PGPError PGPKDBExport
pgpGetPrimaryUserIDName(PGPKey *key, char *buf, size_t *len);

PGPError PGPKDBExport
pgpGetPrimaryUserIDValidity(PGPKey *key, long *validity);


int PGPKDBExport
pgpCompareKeys(PGPKey *a, PGPKey *b, PGPKeyOrdering order);

int PGPKDBExport
pgpUserIDStrCompare(char const *a, char const *b);

PGPKeyList PGPKDBExport *
pgpOrderKeySet (PGPKeySet *src, PGPKeyOrdering order);

void PGPKDBExport
pgpIncKeyListRefCount (PGPKeyList *keys);

void PGPKDBExport
pgpFreeKeyList (PGPKeyList *keys);


/* Key iteration functions */

void PGPKDBExport
pgpIncSubKeyRefCount (PGPSubKey *subkey);

void PGPKDBExport
pgpIncUserIDRefCount (PGPUserID *userid);

void PGPKDBExport
pgpIncCertRefCount (PGPCert *cert);

void PGPKDBExport
pgpFreeSubKey (PGPSubKey *subkey);

void PGPKDBExport
pgpFreeUserID (PGPUserID *userid);

void PGPKDBExport
pgpFreeCert (PGPCert *cert);

PGPKeyIter PGPKDBExport *
pgpNewKeyIter (PGPKeyList *keys);

PGPKeyIter PGPKDBExport *
pgpCopyKeyIter (PGPKeyIter *orig);

void PGPKDBExport
pgpFreeKeyIter (PGPKeyIter *iter);

long PGPKDBExport
pgpKeyIterIndex (PGPKeyIter *iter);

void PGPKDBExport
pgpKeyIterRewind (PGPKeyIter *iter);

long PGPKDBExport
pgpKeyIterSet (PGPKeyIter *iter, PGPKey *key);

PGPKey PGPKDBExport *
pgpKeyIterMove (PGPKeyIter *iter, long relOffset);

PGPKey PGPKDBExport *
pgpKeyIterNext (PGPKeyIter *iter);

PGPKey PGPKDBExport *
pgpKeyIterPrev (PGPKeyIter *iter);

PGPKey PGPKDBExport *
pgpKeyIterKey (PGPKeyIter *iter);

PGPSubKey PGPKDBExport *
pgpKeyIterSubKey (PGPKeyIter *iter);

PGPUserID PGPKDBExport *
pgpKeyIterUserID (PGPKeyIter *iter);

PGPCert PGPKDBExport *
pgpKeyIterCert (PGPKeyIter *iter);

Boolean PGPKDBExport
pgpKeyIterValid (PGPKeyIter *iter);

PGPSubKey PGPKDBExport *
pgpKeyIterNextSubKey (PGPKeyIter *iter);

PGPSubKey PGPKDBExport *
pgpKeyIterPrevSubKey (PGPKeyIter *iter);

void PGPKDBExport
pgpKeyIterRewindSubKey (PGPKeyIter *iter);

PGPUserID PGPKDBExport *
pgpKeyIterNextUserID (PGPKeyIter *iter);

PGPUserID PGPKDBExport *
pgpKeyIterPrevUserID (PGPKeyIter *iter);

void PGPKDBExport
pgpKeyIterRewindUserID (PGPKeyIter *iter);

PGPCert PGPKDBExport *
pgpKeyIterNextUIDCert (PGPKeyIter *iter);

PGPCert PGPKDBExport *
pgpKeyIterPrevUIDCert (PGPKeyIter *iter);

void PGPKDBExport
pgpKeyIterRewindUIDCert (PGPKeyIter *iter);


/* Preference file access */

PGPError PGPKDBExport
pgpGetPrefNumber (PGPPrefNameNumber prefName, long *value);

PGPError PGPKDBExport
pgpGetPrefString (PGPPrefNameString prefName, char *buffer, size_t *length);

char PGPKDBExport *
pgpGetPrefCStringCopy (PGPPrefNameString prefName);

PGPError PGPKDBExport
pgpGetPrefBoolean (PGPPrefNameBoolean prefName, Boolean *value);

PGPError PGPKDBExport
pgpGetPrefData (PGPPrefNameData prefName, void *buffer, size_t *length);

void PGPKDBExport *
pgpGetPrefDataCopy (PGPPrefNameData prefName, size_t *len);

PGPFileRef PGPKDBExport *
pgpGetPrefFileRef (PGPPrefNameData prefName);

PGPError PGPKDBExport
pgpSetPrefNumber (PGPPrefNameNumber prefName, long value);

PGPError PGPKDBExport
pgpSetPrefString (PGPPrefNameString prefName, char const *buffer,
					size_t length);

PGPError PGPKDBExport
pgpSetPrefCString (PGPPrefNameString prefName, char const *string);

PGPError PGPKDBExport
pgpSetPrefBoolean (PGPPrefNameBoolean prefName, Boolean value);

PGPError PGPKDBExport
pgpSetPrefData (PGPPrefNameData prefName, void const *buffer, size_t length);

PGPError PGPKDBExport
pgpSetPrefFileRef (PGPPrefNameData prefName, PGPFileRef const *fileRef);


/* Get/set default private key */

PGPKey PGPKDBExport *
pgpGetDefaultPrivateKey (PGPKeySet *keyset);

PGPError PGPKDBExport
pgpSetDefaultPrivateKey (PGPKey *key);

PgpTrustModel PGPKDBExport
pgpGetTrustModel (void);		/* always returns PGPTRUST0 in this release */


/* Get/set user value */

PGPError PGPKDBExport
pgpSetKeyUserVal (PGPKey *key, long userVal);

PGPError PGPKDBExport
pgpSetUserIDUserVal (PGPUserID *userid, long userVal);

PGPError PGPKDBExport
pgpSetSubKeyUserVal (PGPSubKey *subkey, long userVal);

PGPError PGPKDBExport
pgpSetCertUserVal (PGPCert *cert, long userVal);

PGPError PGPKDBExport
pgpGetKeyUserVal (PGPKey *key, long *userVal);

PGPError PGPKDBExport
pgpGetUserIDUserVal (PGPUserID *userid, long *userVal);

PGPError PGPKDBExport
pgpGetSubKeyUserVal (PGPSubKey *subkey, long *userVal);

PGPError PGPKDBExport
pgpGetCertUserVal (PGPCert *cert, long *userVal);



/* Preference functions */

PGPError PGPKDBExport
pgpLoadPrefs (void);

PGPError PGPKDBExport
pgpSavePrefs (void);

PGPError PGPKDBExport
pgpSetDefaultKeyPath (void);

/*
 * Don't use this. It's only for use by the Simple PGP library,
 * and will be going away.
 */
PGPError PGPKDBExport
pgpGetKeyRingFileRefs (PGPFileRef **pubRef, PGPFileRef **privRef);

/*
 * Debugging support
 */
#define pgpaPGPKeyDBValid(db)		 		 		 		 		 		 \
			pgpaCall(pgpaInternalPGPKeyDBValid, (pgpaCallPrefix, db, #db))
#define pgpaPGPKeySetValid(set)												\
			pgpaCall(pgpaInternalPGPKeySetValid, (pgpaCallPrefix, set, #set))
#define pgpaPGPKeyListValid(list)		 		 		 		 		 	\
			pgpaCall(pgpaInternalPGPKeyListValid,	 		 		 		 \
					(pgpaCallPrefix, list, #list))
#define pgpaPGPKeyIterValid(iter)		 		 		 		 		 	\
			pgpaCall(pgpaInternalPGPKeyIterValid,	 		 		 		 \
					(pgpaCallPrefix, iter, #iter))
#define pgpaPGPKeyValid(key)												\
			pgpaCall(pgpaInternalPGPKeyValid, (pgpaCallPrefix, key, #key))
#define pgpaPGPSubKeyValid(subKey)											\
			pgpaCall(pgpaInternalPGPSubKeyValid,							\
				(pgpaCallPrefix, subKey, #subKey))
#define pgpaPGPUserIDValid(userID)											\
			pgpaCall(pgpaInternalPGPUserIDValid,							\
				(pgpaCallPrefix, userID, #userID))
#define pgpaPGPCertValid(cert)												\
			pgpaCall(pgpaInternalPGPCertValid, (pgpaCallPrefix, cert, #cert))

#if DEBUG	/* [ */

Boolean	PGPKDBExport	pgpaInternalPGPKeyDBValid(pgpaCallPrefixDef,
							PGPKeyDB const *keyDB, char const *varName);
Boolean	PGPKDBExport	pgpaInternalPGPKeySetValid(pgpaCallPrefixDef,
							PGPKeySet const *keySet, char const *varName);
Boolean	PGPKDBExport	pgpaInternalPGPKeyListValid(pgpaCallPrefixDef,
							PGPKeyList const *keyList, char const *varName);
Boolean	PGPKDBExport	pgpaInternalPGPKeyIterValid(pgpaCallPrefixDef,
							PGPKeyIter const *keyIter, char const *varName);
Boolean	PGPKDBExport	pgpaInternalPGPKeyValid(pgpaCallPrefixDef,
							PGPKey const *key, char const *varName);
Boolean	PGPKDBExport	pgpaInternalPGPSubKeyValid(pgpaCallPrefixDef,
							PGPSubKey const *subKey, char const *varName);
Boolean	PGPKDBExport	pgpaInternalPGPUserIDValid(pgpaCallPrefixDef,
							PGPUserID const *userID, char const *varName);
Boolean	PGPKDBExport	pgpaInternalPGPCertValid(pgpaCallPrefixDef,
							PGPCert const *cert, char const *varName);

#endif /* ] DEBUG */

#ifdef __cplusplus
}
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import reset
#endif

#endif /* PGPKEYDB_H */


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
