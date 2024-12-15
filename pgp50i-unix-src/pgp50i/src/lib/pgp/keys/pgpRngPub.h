/*
 * pgpRngPub.h
 *
 * Copyright (C) 1994-1997 Pretty Good Privacy, Inc.  All rights reserved.
 *
 * $Id: pgpRngPub.h,v 1.15.2.2.2.1 1997/10/31 03:35:43 hal Exp $
 */

#ifndef PGPRNGPUB_H
#define PGPRNGPUB_H
/* Public */

#include "pgpUsuals.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PGPTRUSTMODEL
#define PGPTRUSTMODEL 0
#endif

typedef enum {
	PGPTRUST0=0, PGPTRUST1, PGPTRUST2
} PgpTrustModel;

/* Signature subpacket types. Most of these are not supported yet. */
enum sigsubbyte {
	/* Signature specific properties */
	SIGSUB_VERSION			= 1,
	SIGSUB_CREATION,
	SIGSUB_EXPIRATION,
	/* Properties of key being self signed */
	SIGSUB_KEY_CAPABILITIES = 8,
	SIGSUB_KEY_EXPIRATION,
	SIGSUB_KEY_RECOVERY_KEY,
	SIGSUB_PREFERRED_ALGS,
	/* Hints to find signer */
	SIGSUB_KEYID			= 16,
	SIGSUB_USERID,
	SIGSUB_URL,
	SIGSUB_FINGER
};

/*
 * Opaque types
 */
union RingObject;
#ifndef TYPE_RINGOBJECT
#define TYPE_RINGOBJECT 1
typedef union RingObject RingObject;
#endif

struct RingSet;
#ifndef TYPE_RINGSET
#define TYPE_RINGSET 1
typedef struct RingSet RingSet;
#endif

struct RingIterator;
#ifndef TYPE_RINGITERATOR
#define TYPE_RINGITERATOR 1
typedef struct RingIterator RingIterator;
#endif

struct RingFile;
#ifndef TYPE_RINGFILE
#define TYPE_RINGFILE 1
typedef struct RingFile RingFile;
#endif

struct RingPool;
#ifndef TYPE_RINGPOOL
#define TYPE_RINGPOOL 1
typedef struct RingPool RingPool;
#endif

/*
 * Types not defined here
 */
struct PgpEnv;
#ifndef TYPE_PGPENV
#define TYPE_PGPENV 1
typedef struct PgpEnv PgpEnv;
#endif

struct PgpPubKey;
#ifndef TYPE_PGPPUBKEY
#define TYPE_PGPPUBKEY 1
typedef struct PgpPubKey PgpPubKey;
#endif

struct PgpSecKey;
#ifndef TYPE_PGPSECKEY
#define TYPE_PGPSECKEY 1
typedef struct PgpSecKey PgpSecKey;
#endif

/*
 * Okay, finally we start the function declarations.
 */
struct RingPool PGPExport *ringPoolCreate(struct PgpEnv const *env);
/* Destroy everything immediately, dropping all locks! */
void PGPExport ringPoolDestroy(struct RingPool *);

struct RingError {
	struct RingFile *f;	/* The RingFile for I/O errors */
	word32 fpos;	/* The file position for I/O errors */
	int error;	/* PGP error code - PGPERR_* */
	int syserrno;	 /* Don't use plain "errno"; that's a macro! */
};

#ifndef TYPE_RINGERROR
#define TYPE_RINGERROR 1
typedef struct RingError RingError;
#endif

struct RingError const PGPExport *ringPoolError(struct RingPool const *);
void PGPExport ringPoolClearError(struct RingPool *);

/*
 * A RingSet is the root of a tree of objects. The RingSet itself
 * is not a RingObject, but everything else under it is.
 * A full tree looks basically like this:
 * RingSet
 * +--Key
 * | +--Secret (0 or more)
 * | +--Signature (0 or more)
 * | +--Signature (0 or more)
 * | +--Name (0 or more)
 * | | +--Signature (0 or more)
 * | | \--Signature (0 or more)
 * | \--Name (0 or more)
 * | \--Signature (0 or more)
 * +--Key
 * | +--etc.
 * etc.
 *
 * A "secret" object is present if the key's secret components are
 * available. In the standard PGP keyring file format, this is
 * actually stored with the key as a different type of key packet,
 * but the representation here is logically equivalent.
 *
 * There is one secret object per encrypted form of the secret
 * components. Barring duplicate key errors, there is only one
 * secret object per file (if you attempt to write out more, the
 * library will make a guess at the best and write that out), but,
 * for example, changing the passphrase will create a second secret.
 *
 * Some sets are mutable, and RingObjects can be added to or deleted from
 * them, but the tree property is always preserved. Adding an object
 * implicitly adds all of its parents. Deleting an object implicitly
 * deletes all of its children.
 */
int PGPExport ringObjectType(union RingObject const *obj);
/* Type 0 is reserved for application use; it will never be allocated */
#define RINGTYPE_KEY	1
#define RINGTYPE_SEC	2
#define RINGTYPE_NAME	3
#define RINGTYPE_SIG	4
#define RINGTYPE_UNK	5	 /* Object of unknown type */
#define RINGTYPE_MAX	5
/* Adding a new type needs to update ringObjectType() and ringNewObject() */

/*
 * Increase and decrease RingObject reference counts. The ringIter
 * functions hold their current objects (at the current level and all
 * parent levels) automatically and release them when the ringIterator is
 * advanced to another location. If you wish to refer to them after
 * advancing the RingIterator, Other functions that return RingObject
 * pointers hold them automatically, and they must be released explicitly
 * by ringObjectRelease().
 */
void PGPExport ringObjectHold(union RingObject *obj);
void PGPExport ringObjectRelease(union RingObject *obj);

/* Operations on RingSets */

struct RingPool PGPExport *ringSetPool(struct RingSet const *);

struct RingError const PGPExport *ringSetError(struct RingSet const *);
void PGPExport ringSetClearError(struct RingSet *);

int PGPExport ringSetIsMember(struct RingSet const *set,
	union RingObject const *object);
int PGPExport ringSetCount(struct RingSet const *set, unsigned *counts,
	unsigned depth);
int PGPExport ringSetCountTypes(struct RingSet const *set, unsigned *counts,
	unsigned max);

/* Create a new mutable RingSet */
struct RingSet PGPExport *ringSetCreate(struct RingPool *pool);
/* Free a RingSet (mutable or immutable) */
void PGPExport ringSetDestroy(struct RingSet *set);

/* Operate on a mutable RingSet */
int PGPExport ringSetAddObject(struct RingSet *set, union RingObject *obj);
int PGPExport ringSetRemObject(struct RingSet *set, union RingObject *obj);
int PGPExport ringSetAddSet(struct RingSet *set, struct RingSet const *set2);
int PGPExport ringSetSubtractSet(struct RingSet *set,
	struct RingSet const *set2);
int PGPExport ringSetAddObjectChildren(struct RingSet *dest,
	struct RingSet const *src, union RingObject *obj);
/* Convert a mutable RingSet to immutable */
int PGPExport ringSetFreeze(struct RingSet *set);

/* Operate on immutable RingSets */
struct RingSet PGPExport *ringSetCopy(struct RingSet const *s);
struct RingSet PGPExport *
ringSetUnion(struct RingSet const *s1, struct RingSet const *s2);
struct RingSet PGPExport *
ringSetIntersection(struct RingSet const *s1, struct RingSet const *s2);
struct RingSet PGPExport *
ringSetDifference(struct RingSet const *s1, struct RingSet const *s2);

/* Lookups by keyID */
union RingObject PGPExport *
ringKeyById8(struct RingSet const *set, byte pkalg, byte const *keyid);

/* Operations on RingIterators */

struct RingIterator PGPExport *ringIterCreate(struct RingSet const *set);
void PGPExport ringIterDestroy(struct RingIterator *iter);
struct RingSet const PGPExport *ringIterSet(struct RingIterator const *iter);
struct RingError const PGPExport *ringIterError(
	struct RingIterator const *iter);
void PGPExport ringIterClearError(struct RingIterator *iter);
int PGPExport ringIterNextObject(struct RingIterator *iter, unsigned level);
int PGPExport ringIterPrevObject(struct RingIterator *iter, unsigned level);
unsigned PGPExport ringIterCurrentLevel(struct RingIterator const *iter);
union RingObject PGPExport *ringIterCurrentObject(
	struct RingIterator const *iter, unsigned level);

int PGPExport ringIterNextObjectAnywhere(struct RingIterator *iter);
int PGPExport ringIterRewind(struct RingIterator *iter, unsigned level);
int PGPExport ringIterFastForward(struct RingIterator *iter, unsigned level);
int PGPExport ringIterSeekTo(struct RingIterator *iter,
	union RingObject *obj);

/* RingFile access functions */

struct PgpFile PGPExport *ringFileFile(struct RingFile const *file);
struct RingSet const PGPExport *ringFileSet(struct RingFile const *file);
PgpVersion PGPExport ringFileVersion(struct RingFile const *file);

int PGPExport ringFileIsDirty(struct RingFile const *file);
int PGPExport ringFileIsTrustChanged(struct RingFile const *file);

/* Alias for ringSetError(ringFileSet(file)) */
struct RingError const PGPExport *ringFileError(struct RingFile const *file);
void PGPExport ringFileClearError(struct RingFile *file);

int PGPExport
ringSetFilter(struct RingSet const *src, struct RingSet *dest,
int (*predicate)(void *arg, struct RingIterator *iter,
	union RingObject *object, unsigned level),
void *arg);
int PGPExport ringSetFilterSpec(struct RingSet const *src,
	struct RingSet *dest, char const *string, int use);

union RingObject PGPExport *
ringLatestSecret(struct RingSet const *set, char const *string, word32 tstamp,
	int use);


/* Object access functions */

PgpTrustModel PGPExport pgpTrustModel(struct RingPool const *pool);
int PGPExport ringKeyError(struct RingSet const *set, union RingObject *key);
unsigned PGPExport ringKeyBits(struct RingSet const *set,
	union RingObject *key);
word32 PGPExport ringKeyCreation(struct RingSet const *set,
	union RingObject *key);
word32 PGPExport ringKeyExpiration(struct RingSet const *set,
	union RingObject *key);
void PGPExport ringKeyID8(struct RingSet const *set,
	union RingObject const *key, byte *pkalg, byte *buf);
int PGPExport ringKeyUse(struct RingSet const *set, union RingObject *key);
byte PGPExport ringKeyTrust(struct RingSet const *set, union RingObject *key);
void PGPExport ringKeySetTrust(struct RingSet const *set,
	union RingObject *key, byte trust);
int PGPExport ringKeyDisabled(struct RingSet const *set,
	union RingObject *key);
void PGPExport ringKeyDisable(struct RingSet const *set,
	union RingObject *key);
void PGPExport ringKeyEnable(struct RingSet const *set,
	union RingObject *key);
int PGPExport ringKeyRevoked(struct RingSet const *set,
	union RingObject *key);
void PGPExport ringKeySetAxiomatic(struct RingSet const *set,
	union RingObject *key);
void PGPExport ringKeyResetAxiomatic(struct RingSet const *set,
	union RingObject *key);
int PGPExport ringKeyAxiomatic(struct RingSet const *set,
	union RingObject *key);
int PGPExport ringKeyIsSubkey(struct RingSet const *set,
	union RingObject *key);

int PGPExport ringKeyFingerprint16(struct RingSet const *set,
	union RingObject *key, byte *buf);
int PGPExport ringKeyFingerprint20(struct RingSet const *set,
	union RingObject *key, byte *buf);
int PGPExport ringKeyAddSigsby(struct RingSet const *set,
	union RingObject *key, struct RingSet *dest);

/* Given a Ring Object, obtain a PgpPubKey or a PgpSecKey */
int PGPExport ringKeyIsSec(struct RingSet const *set, union RingObject *key);
int PGPExport ringKeyIsSecOnly(struct RingSet const *set,
				union RingObject *key);
union RingObject PGPExport *ringKeySubkey(struct RingSet const *set,
				union RingObject const *key);
union RingObject PGPExport *ringKeyMasterkey(struct RingSet const *set,
				union RingObject const *subkey);
struct PgpPubKey PGPExport *ringKeyPubKey(struct RingSet const *set,
				union RingObject *key, int use);
struct PgpSecKey PGPExport *ringSecSecKey(struct RingSet const *set,
				union RingObject *sec, int use);
PgpVersion PGPExport ringSecVersion (struct RingSet const *set,
				union RingObject *sec);

char const PGPExport *ringNameName(struct RingSet const *set,
	union RingObject *name, size_t *lenp);
byte PGPExport ringNameTrust(struct RingSet const *set,
	union RingObject *name);
int PGPExport ringNameWarnonly(struct RingSet const *set,
	union RingObject *name);
void PGPExport ringNameSetWarnonly(struct RingSet const *set,
	union RingObject *name);

int PGPExport ringSigError(struct RingSet const *set, union RingObject *sig);
union RingObject PGPExport *ringSigMaker(struct RingSet const *sset,
	union RingObject *sig, struct RingSet const *kset);
void PGPExport ringSigID8(struct RingSet const *set,
	union RingObject const *sig, byte *pkalg, byte *buf);
byte PGPExport ringSigTrust(struct RingSet const *set, union RingObject *sig);
int PGPExport ringSigChecked(struct RingSet const *set,
	union RingObject *sig);
int PGPExport ringSigTried(struct RingSet const *set, union RingObject *sig);
int PGPExport ringSigType(struct RingSet const *Set,
	union RingObject const *sig);
word32 PGPExport ringSigTimestamp(struct RingSet const *Set,
	union RingObject const *sig);
int PGPExport ringSigRevoked (struct RingSet const *set,
	union RingObject *sig);
int ringSigIsSelfSig(RingSet const *set, RingObject const *sig);


/* Only valid if PgpTrustModel > PGPTRUST0 */
word16 PGPExport ringKeyConfidence(struct RingSet const *set,
	union RingObject *key);
word16 PGPExport ringNameValidity(struct RingSet const *set,
	union RingObject *name);
word16 PGPExport ringNameConfidence(struct RingSet const *set,
	union RingObject *name);
int PGPExport ringNameConfidenceUndefined(struct RingSet const *set,
				union RingObject *name);
void PGPExport ringNameSetConfidence(struct RingSet const *set,
			union RingObject *name, word16 confidence);
word16 PGPExport ringSigConfidence(struct RingSet const *set,
	union RingObject *sig);

byte const PGPExport *
ringKeyFindSubpacket (RingObject *obj, struct RingSet *set,
	int subpacktype, unsigned nth,
	size_t *plen, int *pcritical, int *phashed, word32 *pcreation,
	unsigned *pmatches, int *error);
union RingObject PGPExport *
ringKeyRecoveryKey (union RingObject *obj, struct RingSet *set, unsigned nth,
					byte *pclass, unsigned *pkeys, int *error);

void PGPExport ringPoolConsistent (struct RingPool *pool, int *pnsets,
	int *pnfiles);


#ifdef __cplusplus
}
#endif

#endif /* PGPRNGPUB_H */


/*
 * Local Variables:
 * tab-width: 4
 * End:
 * vi: ts=4 sw=4
 * vim: si
*/
