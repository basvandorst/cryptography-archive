/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates Inc. and affiliated companies.
	All rights reserved.

	$Id: pgpKDBInt.h,v 1.52 1999/05/17 05:08:10 hal Exp $
____________________________________________________________________________*/

#ifndef Included_pgpKDBInt_h
#define Included_pgpKDBInt_h

#include <stdio.h>


#include "pgpKeyDB.h"
#include "pgpEventPriv.h"
#include "pgpFileRef.h"
#include "pgpTypes.h"
#include "pgpUsuals.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpMemPool.h"
#include "pgpRngPub.h"

#include "pgpKeyFilterPriv.h"
#include "pgpKeyDBOpaqueStructs.h"


/* 
 * An internal object representing a source of keys, either a file,
 * some memory, or a union of other PGPKeyDBs
 */
struct PGPKeyDB
{
#define PGPKDBFILEMAGIC		0x1000
#define PGPKDBUNIONMAGIC	0x2000
#define PGPKDBMEMMAGIC		0x3000
	PGPInt32			typeMagic;

#define kPGPKeyDBMagic		0x4B594442	/* 'KYDB' */
	PGPUInt32			fixedMagic;
	
	void *				priv;
	PGPInt32			refCount;
	PGPContextRef		context;
	PGPKeySetRef		firstSetInDB;

	MemPool				keyPool;
	long				numKeys;
	PGPKeyRef			firstKeyInDB;
	PGPKeyRef			firstFreeKey;
	PGPSubKeyRef		firstFreeSubKey;
	PGPUserIDRef		firstFreeUserID;
	PGPSigRef			firstFreeCert;
	PGPKeyListRef		keysByKeyID;

	/* Array of (PGPKeyDB *)s which this PGPKeyDB depends on until commit */
	PGPKeyDB **			keyDBDependencies;
	long				numKeyDBDependenciesAllocated;	/* number allocated */
	long				numKeyDBDependencies;			/* number used */

	PGPBoolean			(*isMutable)	(PGPKeyDBRef db);
	PGPBoolean			(*objIsMutable)	(PGPKeyDBRef db,
										RingObject *testObj);
	PGPBoolean			(*isDirty)		(PGPKeyDBRef db);
	RingSet const *		(*getRingSet)	(PGPKeyDBRef db);

	/* The RingSet arguments below must be of whole keys */
	PGPError			(*add)			(PGPKeyDBRef db,
										RingSet *toAdd);
	PGPError			(*changed)		(PGPKeyDBRef db,
										RingSet *changed);
	PGPError			(*remove)		(PGPKeyDBRef db,
										RingObject *toRemove);

	PGPError			(*commit)		(PGPKeyDBRef db);
	PGPError			(*revert)		(PGPKeyDBRef db);
	PGPError			(*reload)		(PGPKeyDBRef db);
	void				(*destroy)		(PGPKeyDBRef db);

	DEBUG_STRUCT_CONSTRUCTOR( PGPKeyDB )
} ;


/*
 * Objects that represent Aurora RingObjects.	If an attempt is made to
 * delete an object, and refCount > 0, then the removed flag should
 * be set to TRUE instead.	The object can be physically deleted when the 
 * refCount drops to zero.
 *
 * listhdr saves memory but not having to embed a PGPUserID in 
 * a PGPKey, or a PGPSig in a PGPUserID, to maintain the doubly-linked
 * lists.	It may be typecast to PGPUserID or PGPSig as required.
 */
typedef struct listhdr
{
	struct listhdr *	next;
	struct listhdr *	prev;
} listhdr;

enum certtype {keycert, uidcert};

/*____________________________________________________________________________
	To save memory, some items use magic fields in DEBUG mode only.  All
	the code to set and check them depends on the flag 'PGP_OPTIONAL_MAGICS'.
____________________________________________________________________________*/
#define PGP_OPTIONAL_MAGICS		PGP_DEBUG

#if PGP_OPTIONAL_MAGICS
#define pgpOptionalMagicMatches( ref, mgc )	( (ref)->magic == mgc  )
#else
#define pgpOptionalMagicMatches( ref, mgc )		TRUE
#endif

struct PGPSig
{
	/* caution: these must be first; see "struct listhdr" comments */
	PGPSigRef			next;	
	PGPSigRef			prev;

	RingObject *		cert;
	enum certtype		type;
	union
	{
		PGPUserIDRef	userID;	/* type == uidcert */
		PGPKeyRef		key;		/* type == keycert */
	} up;
	PGPInt32			refCount;
	PGPUserValue		userVal;
	PGPBoolean			removed;
#if PGP_OPTIONAL_MAGICS
#define kPGPCertMagic	0x43455254	/* 'CERT' */
	PGPUInt32			magic;
#endif
} ;


struct PGPUserID
{
	/* caution: these must be first; see "struct listhdr" comments */
	PGPUserIDRef		next;
	PGPUserIDRef		prev;
	listhdr		certs;		/* doubly-linked list of UserID certs */
	RingObject *		userID;
	PGPKeyRef			key;
	PGPInt32			refCount;
	PGPBoolean			removed;
	PGPUserValue		userVal;
	
#if PGP_OPTIONAL_MAGICS
#define kPGPUserIDMagic	0x55534552	/* 'USER' */
	PGPUInt32			magic;
#endif
}  ;


struct PGPSubKey
{
	/* caution: these must be first; see "struct listhdr" comments */
	PGPSubKeyRef		next;
	PGPSubKeyRef		prev;
	RingObject *		subKey;
	PGPKeyRef			key;
	PGPInt32			refCount;
	PGPBoolean			removed;
	PGPUserValue		userVal;
	
#if PGP_OPTIONAL_MAGICS
#define kPGPSubKeyMagic	0x5355424B	/* 'SUBK' */
	PGPUInt32			magic;
#endif
}  ;



struct PGPKey
{
	PGPInt32			refCount;
	PGPKeyDBRef			keyDB;
	PGPKeyRef			nextKeyInDB;
	RingObject *		key;
	listhdr				subKeys;		/* doubly linked list of Subkeys */
	listhdr				userIDs;		/* doubly linked list of UserIDs */
	PGPUserValue		userVal;
	
#if PGP_OPTIONAL_MAGICS
#define kPGPKeyMagic	0x4B455920	/* 'KEY ' */
	PGPUInt32			magic;
#endif
} ;


/*
 * The external object which represents a filtered PGPKeyDB
 */
struct PGPKeySet
{
#define kPGPKeySetMagic	0x4B534554	/* 'KSET' */
	PGPUInt32			magic;
	void *				priv;
	PGPInt32			refCount;
	PGPKeyDBRef			keyDB;
	PGPKeySetRef		prevSetInDB;
	PGPKeySetRef		nextSetInDB;
	PGPKeyListRef		firstListInSet;

	PGPBoolean			(*isMember)	(PGPKeySetRef set, PGPKeyRef key);
	PGPError			(*makeUnion)(PGPKeySetRef set1, PGPKeySetRef set2,
									 PGPKeySetRef *newSet);
	void				(*removeKey)(PGPKeySetRef set, PGPKeyRef key);
	void				(*destroy)	(PGPKeySetRef set);
} ;



/*
 * The external object which represents a sorted PGPKeySet
 */
struct PGPKeyList
{
#define kPGPKeyListMagic	0x4B4C5354	/* 'KLST' */
	PGPUInt32			magic;
	PGPInt32			refCount;
	PGPKeySetRef		keySet;
	PGPKeyOrdering		order;

	PGPKeyListRef		prevListInSet;
	PGPKeyListRef		nextListInSet;
	PGPKeyIterRef		firstIterInList;

	long				keyCount;		/* size of keys array */
	PGPKeyRef*			keys;
} ;


/*
 * The external object which represents a key iterator
 */
struct PGPKeyIter
{
#define kPGPKeyIterMagic	0x4B495452	/* 'KITR' */
	PGPUInt32			magic;
	
	PGPKeyListRef		keyList;		/* parent keylist */
	PGPKeyIterRef		prevIterInList;	/* next iter in keylist */
	PGPKeyIterRef		nextIterInList;	/* next iter in keylist */

	long				keyIndex;		/* current key index in keylist */
	PGPKeyRef			key;			/* ptr to current key */
	PGPSubKeyRef		subKey;			/* ptr to current subkey */
	PGPUserIDRef		userID;			/* ptr to current userid */
	PGPSigRef			uidCert;		/* ptr to current userid cert */
} ;


#define CHECKREMOVED(x)		\
if (!(x) || (x)->removed)	\
	return kPGPError_ItemWasDeleted



/* Global variables: These aren't valid until PGPLibInit() is called */



PGP_BEGIN_C_DECLARATIONS

/* Internal functions */

PGPKeyDBRef 	pgpKeyDBCreateInternal(PGPContextRef context);

void			pgpKeyDBInitInternal(PGPKeyDBRef db);

void			pgpKeyDBDestroyInternal (PGPKeyDBRef db);

PGPKeyDBRef 	pgpKeyDBCreate(PGPContextRef context);

PGPKeySetRef	pgpKeyDBRootSet(PGPKeyDBRef db);

RingSet const *	pgpKeyDBRingSet(PGPKeyDBRef db);

PGPBoolean		pgpKeyDBIsMutable(PGPKeyDBRef db);

PGPBoolean		pgpKeyDBIsDirty(PGPKeyDBRef db);

PGPError		pgpKeyDBChanged(PGPKeyDBRef db, RingSet *changedkeys);

PGPError		pgpCommitKeyDB(PGPKeyDBRef db);

PGPError		pgpPropagateTrustKeyDB(PGPKeyDBRef db);

PGPError		pgpCheckKeyDB (PGPKeyDBRef dbToCheck, PGPKeyDBRef dbSigning,
					PGPBoolean checkAll,
					PGPEventHandlerProcPtr progress, PGPUserValue userValue);

PGPError		pgpRevertKeyDB(PGPKeyDBRef db);

PGPError		pgpReloadKeyDB(PGPKeyDBRef db);

PGPKeyDBRef 	pgpCreateFileKeyDB (PGPContextRef context,
					PFLConstFileSpecRef fileRef, PGPKeyRingOpenFlags openFlags,
					RingPool *ringpool, PGPError *error);

PGPKeyDBRef 	pgpCreateMemFileKeyDB (PGPContextRef context,
					PGPByte *buf, size_t length, RingPool *ringpool,
					PGPError *error);

PGPKeyDBRef 
pgpCreateUnionKeyDB (PGPContextRef context, PGPError *error);

PGPError	pgpUnionKeyDBAdd (PGPKeyDBRef kdb, PGPKeyDBRef kdbnew);

PGPError	pgpKeySetRingSet(PGPKeySet *keys, PGPBoolean freezeit,
				RingSet const **ringset);

void		pgpKeyIterAddKey (PGPKeyListRef keys, long idx);

void		pgpKeyIterRemoveKey (PGPKeyListRef keys, long idx);

PGPError	pgpCopyKey (RingSet const *src, union RingObject *obj, 
				RingSet **dest);
				
PGPError	pgpCopyKeySet( PGPKeySetRef	orig, PGPKeySetRef * newOut );

PGPKeyRef	pgpGetKeyByRingObject (PGPKeyDBRef keys, union RingObject *obj);

void		pgpDeallocSubKey (PGPSubKeyRef subkey);

void		pgpDeallocUserID (PGPUserIDRef userid);

void 		pgpDeallocCert (PGPSigRef cert);

PGPError	pgpExpandKey (PGPKeyRef key);

PGPError	pgpCollapseKey (PGPKeyRef key);

PGPError	pgpAddObjects (PGPKeyDBRef keys, RingSet *addset);

PGPError	pgpRemoveObject (PGPKeyDBRef keys, union RingObject *obj); 

void		pgpIncKeyDBRefCount(PGPKeyDBRef db);

void		pgpFreeKeyDB(PGPKeyDBRef db);

PGPError		pgpBuildKeyPool(PGPKeyDBRef db, PGPBoolean deleteFlag);

PGPError		pgpReSortKeys(PGPKeyDBRef db, RingSet *set);

PGPError		pgpDearmorKeyFile (PGPEnv *env, PFLFileSpecRef InputFileRef,
					PGPByte **OutputBuffer, PGPSize *OutputBufferLen);

PGPError		pgpDearmorKeyBuffer (PGPEnv *env,
					PGPByte const *InputBuffer, PGPSize InputBufferLen,
					PGPByte **OutputBuffer, PGPSize *OutputBufferLen);

PGPError	pgpWriteArmoredSetFile (FILE *fp,
				RingSet const *set, PGPEnv *env);

PGPError		pgpWriteArmoredSetBuffer (PGPByte *buf,
					PGPSize *buflen, RingSet const *set,
					PGPEnv *env);



PGPContextRef	pgpGetKeyDBContext( PGPKeyDBRef ref );


PGPError 		pgpIncKeyRefCount( PGPKeyRef key);
PGPError 		pgpFreeKey(PGPKeyRef iter);
PGPError 		pgpIncUserIDRefCount (PGPUserIDRef userid);
PGPError 		pgpFreeUserID(PGPUserIDRef userid);
PGPError 		pgpIncSigRefCount (PGPSigRef cert);
PGPError 		pgpFreeSig (PGPSigRef cert);

PGPError 		pgpIncSubKeyRefCount (PGPSubKeyRef subkey);
PGPError 		pgpFreeSubKey(PGPSubKeyRef subkey);

PGPError		pgpDoChangePassphraseInternal (PGPKeyDB *keyDB,
					RingSet const *ringset, 
					RingObject *keyobj, RingObject *masterkeyobj, 
					const char *oldphrase, PGPSize oldphraseLength,
					const char *newphrase, PGPSize newphraseLength,
					PGPBoolean newPassphraseIsKey);


PGPError		pgpImportX509Certificate( PGPContextRef context,
					PGPKeySetRef *keys, PGPOptionListRef optionList);

PGPBoolean		pgpKeyDBIsValid( PGPKeyDB const *keyDB);
PGPBoolean		pgpKeySetIsValid( PGPKeySet const *keySet);
PGPBoolean		pgpKeyListIsValid( PGPKeyList const *keyList);
PGPBoolean		pgpKeyIterIsValid( PGPKeyIter const *keyIter);
PGPBoolean		pgpKeyIsValid( PGPKey const *key);
PGPBoolean		pgpSubKeyIsValid( PGPSubKey const *subKey);
PGPBoolean		pgpUserIDIsValid( PGPUserID const *userID);
PGPBoolean		pgpSigIsValid( PGPSig const *cert);

#define PGPValidateKeyDB( keyDB ) \
			PGPValidateParam( pgpKeyDBIsValid( keyDB ) )
#define PGPValidateKeySet( keySet ) \
			PGPValidateParam( pgpKeySetIsValid( keySet ) )
#define PGPValidateKeyList( keyList ) \
			PGPValidateParam( pgpKeyListIsValid( keyList ) )
#define PGPValidateKeyIter( keyIter ) \
			PGPValidateParam( pgpKeyIterIsValid( keyIter ) )
#define PGPValidateKey( key ) \
			PGPValidateParam( pgpKeyIsValid( key ) )
#define PGPValidateSubKey( subKey ) \
			PGPValidateParam( pgpSubKeyIsValid( subKey ) )
#define PGPValidateUserID( userID ) \
			PGPValidateParam( pgpUserIDIsValid( userID ) )
#define PGPValidateCert( cert ) \
			PGPValidateParam( pgpSigIsValid( cert ) )


#if PGP_DEBUG	/* [ */

/*
 * Debugging support
 */
#define pgpaPGPKeyDBValid(db)												\
			pgpaCall(pgpaInternalPGPKeyDBValid,( pgpaCallPrefix, db, #db))
#define pgpaPGPKeySetValid(set)												\
			pgpaCall(pgpaInternalPGPKeySetValid,( pgpaCallPrefix, set, #set))
#define pgpaPGPKeyListValid(list)											\
			pgpaCall(pgpaInternalPGPKeyListValid,( pgpaCallPrefix,list,#list))
#define pgpaPGPKeyIterValid(iter )											\
			pgpaCall(pgpaInternalPGPKeyIterValid,(pgpaCallPrefix,iter,#iter ))
#define pgpaPGPKeyValid(key )												\
			pgpaCall(pgpaInternalPGPKeyValid,( pgpaCallPrefix, key, #key ))
#define pgpaPGPSubKeyValid(subKey )											\
			pgpaCall(pgpaInternalPGPSubKeyValid,							\
				(pgpaCallPrefix, subKey, #subKey ))
#define pgpaPGPUserIDValid(userID)											\
			pgpaCall(pgpaInternalPGPUserIDValid,							\
				(pgpaCallPrefix, userID, #userID))
#define pgpaPGPCertValid(cert)												\
			pgpaCall(pgpaInternalPGPCertValid,( pgpaCallPrefix, cert, #cert))


PGPBoolean		pgpaInternalPGPKeyDBValid(pgpaCallPrefixDef,
							PGPKeyDB const *keyDB, char const *varName);
PGPBoolean		pgpaInternalPGPKeySetValid(pgpaCallPrefixDef,
							PGPKeySet const *keySet, char const *varName);
PGPBoolean		pgpaInternalPGPKeyListValid(pgpaCallPrefixDef,
							PGPKeyList const *keyList, char const *varName);
PGPBoolean		pgpaInternalPGPKeyIterValid(pgpaCallPrefixDef,
							PGPKeyIter const *keyIter, char const *varName);
PGPBoolean		pgpaInternalPGPKeyValid(pgpaCallPrefixDef,
							PGPKey const *key, char const *varName);
PGPBoolean		pgpaInternalPGPSubKeyValid(pgpaCallPrefixDef,
							PGPSubKey const *subKey, char const *varName);
PGPBoolean		pgpaInternalPGPUserIDValid(pgpaCallPrefixDef,
							PGPUserID const *userID, char const *varName);
PGPBoolean		pgpaInternalPGPCertValid(pgpaCallPrefixDef,
							PGPSig const *cert, char const *varName);

#endif /* ] PGP_DEBUG */

/* Access functions for internal use by other parts of the library */
PGPError		pgpGetKeyRingObject(PGPKey *key, PGPBoolean checkDead,
							RingObject **pRingKey);
PGPError		pgpGetKeyRingSet(PGPKey *key, PGPBoolean checkDead,
							RingSet const **pRingSet);
PGPError		pgpGetUserIDRingObject(PGPUserID *userid, PGPBoolean checkDead,
							RingObject **pRingName);
PGPError		pgpGetUserIDRingSet(PGPUserID *userid, PGPBoolean checkDead,
							RingSet const **pRingSet);
PGPError		pgpGetUserIDKey(PGPUserID *userid, PGPBoolean checkDead,
							PGPKey **pKey);
PGPError		pgpGetCertRingObject(PGPSig *cert, PGPBoolean checkDead,
							RingObject **pRingSig);
PGPError		pgpGetCertRingSet(PGPSig *cert, PGPBoolean checkDead,
							RingSet const **pRingSet);
PGPError		pgpGetCertUserID(PGPSig *cert, PGPBoolean checkDead,
							PGPUserID **pUserid);
PGPError		pgpGetCertKey(PGPSig *cert, PGPBoolean checkDead,
							PGPKey **pKey);

PGPError		pgpNewKeySetInternal(PGPKeyDBRef db, PGPKeySetRef *newSet);
PGPError		pgpGenericUnionOfKeySets(PGPKeySetRef set1, PGPKeySetRef set2,
							PGPKeySetRef *newSet);


PGP_END_C_DECLARATIONS



#endif /* PGPKBDINT.h */

/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
 */
