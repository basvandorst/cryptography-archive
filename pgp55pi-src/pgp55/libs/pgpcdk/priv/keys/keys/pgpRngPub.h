/*
 * $Id: pgpRngPub.h,v 1.32 1997/10/17 06:15:10 hal Exp $
 */

#ifndef Included_pgpRngPub_h
#define Included_pgpRngPub_h
/* Public */

#include "pgpUsuals.h"
#include "pgpContext.h"		/* for PGPContextRef */

PGP_BEGIN_C_DECLARATIONS

#ifndef PGPTRUSTMODEL
#define PGPTRUSTMODEL 0
#endif

typedef enum {
	PGPTRUST0=0, PGPTRUST1, PGPTRUST2
} PgpTrustModel;

/* Signature subpacket types.  Most of these are not supported yet. */
enum sigsubbyte {
	/* Signature specific properties */
	SIGSUB_VERSION			= 1,
	SIGSUB_CREATION,
	SIGSUB_EXPIRATION,
	SIGSUB_EXPORTABLE,
	SIGSUB_TRUST,
	SIGSUB_REGEXP,
	SIGSUB_REVOKABLE,
	/* Properties of key being self signed */
	SIGSUB_KEY_CAPABILITIES = 8,
	SIGSUB_KEY_EXPIRATION,
	SIGSUB_KEY_ADDITIONAL_RECIPIENT_REQUEST,
	SIGSUB_PREFERRED_ALGS,
	SIGSUB_KEY_REVOCATION_KEY,
	/* Hints to find signer */
	SIGSUB_KEYID			= 16,
	SIGSUB_USERID,
	SIGSUB_URL,
	SIGSUB_FINGER
};


#include "pgpOpaqueStructs.h"

/*
 * Okay, finally we start the function declarations.
 */
RingPool  *ringPoolCreate(PGPEnv const *env);
/* Destroy everything immediately, dropping all locks! */
void  ringPoolDestroy(RingPool *);
PGPContextRef ringPoolContext(RingPool *pool);

struct RingError
{
	RingFile *	f;	/* The RingFile for I/O errors */
	PGPUInt32	fpos;	/* The file position for I/O errors */
	PGPError	error;	/* PGP error code - kPGPError_* */
	int			syserrno;	/* Don't use plain "errno"; that's a macro! */
};

RingError const  *ringPoolError(RingPool const *);
void  ringPoolClearError(RingPool *);

/*
 * A RingSet is the root of a tree of objects.  The RingSet itself
 * is not a RingObject, but everything else under it is.
 * A full tree looks basically like this:
 * RingSet
 *  +--Key
 *  |   +--Secret (0 or more)
 *  |   +--Signature (0 or more)
 *  |   +--Signature (0 or more)
 *  |   +--Name (0 or more)
 *  |   |   +--Signature (0 or more)
 *  |   |   \--Signature (0 or more)
 *  |   \--Name (0 or more)
 *  |       \--Signature (0 or more)
 *  +--Key
 *  |   +--etc.
 * etc.
 *
 * A "secret" object is present if the key's secret components are
 * available.  In the standard PGP keyring file format, this is
 * actually stored with the key as a different type of key packet,
 * but the representation here is logically equivalent.
 *
 * There is one secret object per encrypted form of the secret
 * components.  Barring duplicate key errors, there is only one
 * secret object per file (if you attempt to write out more, the
 * library will make a guess at the best and write that out), but,
 * for example, changing the passphrase will create a second secret.
 *
 * Some sets are mutable, and RingObjects can be added to or deleted from
 * them, but the tree property is always preserved.  Adding an object
 * implicitly adds all of its parents.  Deleting an object implicitly
 * deletes all of its children.
 */
int  ringObjectType(union RingObject const *obj);
/* Type 0 is reserved for application use; it will never be allocated */
#define RINGTYPE_KEY	1
#define RINGTYPE_SEC	2
#define RINGTYPE_NAME	3
#define RINGTYPE_SIG	4
#define RINGTYPE_UNK	5	/* Object of unknown type */
#define RINGTYPE_MAX	5
/* Adding a new type needs to update ringObjectType() and ringNewObject() */

/*
 * Increase and decrease RingObject reference counts.  The ringIter
 * functions hold their current objects (at the current level and all
 * parent levels) automatically and release them when the ringIterator is
 * advanced to another location.  If you wish to refer to them after
 * advancing the RingIterator, Other functions that return RingObject
 * pointers hold them automatically, and they must be released explicitly
 * by ringObjectRelease().
 */
void  ringObjectHold(union RingObject *obj);
void  ringObjectRelease(union RingObject *obj);

/* Operations on RingSets */

RingPool  *ringSetPool(RingSet const *);

RingError const  *ringSetError(RingSet const *);
void  ringSetClearError(RingSet *);

int  ringSetIsMember(RingSet const *set,
	union RingObject const *object);
int  ringSetCount(RingSet const *set, unsigned *counts,
	unsigned depth);
int  ringSetCountTypes(RingSet const *set, unsigned *counts,
	unsigned max);

/* Create a new mutable RingSet */
RingSet  *ringSetCreate(RingPool *pool);
/* Free a RingSet (mutable or immutable) */
void  ringSetDestroy(RingSet *set);

/* Operate on a mutable RingSet */
int  ringSetAddObject(RingSet *set, union RingObject *obj);
int  ringSetRemObject(RingSet *set, union RingObject *obj);
int  ringSetAddSet(RingSet *set, RingSet const *set2);
int  ringSetSubtractSet(RingSet *set,
	RingSet const *set2);
int  ringSetAddObjectChildren(RingSet *dest,
	RingSet const *src, union RingObject *obj);
/* Convert a mutable RingSet to immutable */
PGPError  ringSetFreeze(RingSet *set);

/* Operate on immutable RingSets */
RingSet  *ringSetCopy(RingSet const *s);
RingSet  *
ringSetUnion(RingSet const *s1, RingSet const *s2);
RingSet  *
ringSetIntersection(RingSet const *s1, RingSet const *s2);
RingSet  *
ringSetDifference(RingSet const *s1, RingSet const *s2);

/* Lookups by keyID */
union RingObject  *
ringKeyById8(RingSet const *set, PGPByte pkalg, PGPByte const *keyid);

/* Operations on RingIterators */

RingIterator  *ringIterCreate(RingSet const *set);
void  ringIterDestroy(RingIterator *iter);
RingSet const  *ringIterSet(RingIterator const *iter);
RingError const  *ringIterError(
	RingIterator const *iter);
void  ringIterClearError(RingIterator *iter);
int  ringIterNextObject(RingIterator *iter, unsigned level);
int  ringIterPrevObject(RingIterator *iter, unsigned level);
unsigned  ringIterCurrentLevel(RingIterator const *iter);
union RingObject  *ringIterCurrentObject(
	RingIterator const *iter, unsigned level);

int  ringIterNextObjectAnywhere(RingIterator *iter);
int  ringIterRewind(RingIterator *iter, unsigned level);
int  ringIterFastForward(RingIterator *iter, unsigned level);
int  ringIterSeekTo(RingIterator *iter, union RingObject *obj);

/* RingFile access functions */

struct PGPFile  *ringFileFile(RingFile const *file);
RingSet const  *ringFileSet(RingFile const *file);
PgpVersion  ringFileVersion(RingFile const *file);

int  ringFileIsDirty(RingFile const *file);
int  ringFileIsTrustChanged(RingFile const *file);

PGPError  ringFileSwitchFile(RingFile *file, struct PGPFile *newPGPFile);

/* Alias for ringSetError(ringFileSet(file)) */
RingError const  *ringFileError(RingFile const *file);
void  ringFileClearError(RingFile *file);

int 
ringSetFilter(RingSet const *src, RingSet *dest,
              int (*predicate)(void *arg, RingIterator *iter,
                               union RingObject *object, unsigned level),
              void *arg);
int  ringSetFilterSpec(RingSet const *src,
	RingSet *dest, char const *string, int use);

union RingObject  *
ringLatestSecret(RingSet const *set, char const *string, PGPUInt32 tstamp,
	int use);


/* Object access functions */

PgpTrustModel  pgpTrustModel(RingPool const *pool);
int  ringKeyError(RingSet const *set, union RingObject *key);
unsigned  ringKeyBits(RingSet const *set,
	union RingObject *key);
PGPUInt32  ringKeyCreation(RingSet const *set,
	union RingObject *key);
PGPUInt32  ringKeyExpiration(RingSet const *set,
	union RingObject *key);
void  ringKeyID8(RingSet const *set,
	union RingObject const *key, PGPByte *pkalg, PGPKeyID *keyID);
int  ringKeyUse(RingSet const *set, union RingObject *key);
PGPByte  ringKeyTrust(RingSet const *set, union RingObject *key);
void  ringKeySetTrust(RingSet const *set,
	union RingObject *key, PGPByte trust);
int  ringKeyDisabled(RingSet const *set,
	union RingObject *key);
void  ringKeyDisable(RingSet const *set,
	union RingObject *key);
void  ringKeyEnable(RingSet const *set,
	union RingObject *key);
int  ringKeyRevoked(RingSet const *set,
	union RingObject *key);
void  ringKeySetAxiomatic(RingSet const *set,
	union RingObject *key);
void  ringKeyResetAxiomatic(RingSet const *set,
	union RingObject *key);
int  ringKeyAxiomatic(RingSet const *set,
	union RingObject *key);
int  ringKeyIsSubkey(RingSet const *set,
	union RingObject const *key);

int  ringKeyFingerprint16(RingSet const *set,
	union RingObject *key, PGPByte *buf);
int  ringKeyFingerprint20(RingSet const *set,
	union RingObject *key, PGPByte *buf);
int  ringKeyAddSigsby(RingSet const *set,
	union RingObject *key, RingSet *dest);

/* Given a Ring Object, obtain a PGPPubKey or a PGPSecKey */
int  ringKeyIsSec(RingSet const *set, union RingObject *key);
int  ringKeyIsSecOnly(RingSet const *set,
				union RingObject *key);
union RingObject  *ringKeySubkey(RingSet const *set,
				union RingObject const *key);
union RingObject  *ringKeyMasterkey(RingSet const *set,
				union RingObject const *subkey);
PGPPubKey  *ringKeyPubKey(RingSet const *set,
				union RingObject *key, int use);
PGPSecKey  *ringSecSecKey(RingSet const *set,
				union RingObject *sec, int use);
PgpVersion  ringSecVersion (RingSet const *set,
				union RingObject *sec);

char const  *ringNameName(RingSet const *set,
	union RingObject *name, PGPSize *lenp);
PGPByte  ringNameTrust(RingSet const *set,
	union RingObject *name);
int  ringNameWarnonly(RingSet const *set,
	union RingObject *name);
void  ringNameSetWarnonly(RingSet const *set,
	union RingObject *name);

int  ringSigError(RingSet const *set, union RingObject *sig);
union RingObject  *ringSigMaker(RingSet const *sset,
	union RingObject *sig, RingSet const *kset);
void  ringSigID8(RingSet const *set,
	union RingObject const *sig, PGPByte *pkalg, PGPKeyID *buf);
PGPByte  ringSigTrust(RingSet const *set, union RingObject *sig);
PGPByte  ringSigTrustLevel(RingSet const *set, union RingObject const *sig);
PGPByte  ringSigTrustValue(RingSet const *set, union RingObject const *sig);
int  ringSigChecked(RingSet const *set, union RingObject const *sig);
int  ringSigTried(RingSet const *set, union RingObject const *sig);
int  ringSigType(RingSet const *Set, union RingObject const *sig);
PGPUInt32  ringSigTimestamp(RingSet const *Set, union RingObject const *sig);
PGPUInt32  ringSigExpiration(RingSet const *set, union RingObject const *sig);
int  ringSigRevoked (RingSet const *set, union RingObject *sig);
int	ringSigExportable (RingSet const *set, union RingObject const *sig);
PGPBoolean ringSigIsSelfSig(RingSet const *set, RingObject const *sig);

/* Only valid if PgpTrustModel > PGPTRUST0 */
PGPUInt16  ringKeyConfidence(RingSet const *set,
	union RingObject *key);
PGPUInt16  ringNameValidity(RingSet const *set,
	union RingObject *name);
PGPUInt16  ringNameConfidence(RingSet const *set,
	union RingObject *name);
int  ringNameConfidenceUndefined(RingSet const *set, 
				union RingObject *name);
void  ringNameSetConfidence(RingSet const *set, 
			     union RingObject *name, PGPUInt16 confidence);
PGPUInt16  ringSigConfidence(RingSet const *set,
	union RingObject *sig);

PGPByte const  *
ringKeyFindSubpacket (RingObject *obj, RingSet const *set,
	int subpacktype, unsigned nth,
	PGPSize *plen, int *pcritical, int *phashed, PGPUInt32 *pcreation,
	unsigned *pmatches, PGPError *error);
union RingObject  *
ringKeyAdditionalRecipientRequestKey (union RingObject *obj, RingSet const *set,
					unsigned nth, PGPByte *pclass, unsigned *pkeys,
					PGPError *error);
PGPError
ringKeyAdditionalRecipientRequestKeyID8 (union RingObject *obj,
					RingSet const *set, unsigned nth, PGPByte *pclass,
					PGPUInt32 *pkeys, PGPByte *pkalg, PGPKeyID *keyid);
union RingObject  *
ringKeyRevocationKey (union RingObject *obj, RingSet const *set, unsigned nth,
					PGPByte *pclass, unsigned *pkeys, PGPError *error);
PGPBoolean
ringKeyIsRevocationKey (RingObject *key, RingSet const *set,
					RingObject *rkey);

void ringPoolConsistent (RingPool *pool, int *pnsets,
	int *pnfiles);


PGP_END_C_DECLARATIONS

#endif /* Included_pgpRngPub_h */
