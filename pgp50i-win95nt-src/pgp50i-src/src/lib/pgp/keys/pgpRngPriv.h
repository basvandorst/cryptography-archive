/*
 * pgpRngPriv.h
 *
 * Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpRngPriv.h,v 1.11.2.1 1997/06/07 09:50:39 mhw Exp $
 */

#ifndef PGPRNGPRIV_H
#define PGPRNGPRIV_H

#include <stdio.h>		/* For size_t */
#include "pgpRngPub.h"

/* Private */

#include "pgpMemPool.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This number should probably be increased for multiple keyrings and GUIs.
 * At the moment, it is low to help memory consumption.
 */
#ifndef RINGMASKBITS
#define RINGMASKBITS 16
#endif

#define MEMRINGBIT (RINGMASKBITS-1)

#if RINGMASKBITS <= 8
typedef byte ringmask;
#elif RINGMASKBITS <= 16
typedef word16 ringmask;
#elif RINGMASKBITS <= 32
typedef word32 ringmask;
#else
#error Invalid RINGMASKBITS value
#endif

#define MEMRINGMASK ((ringmask)1<<MEMRINGBIT)

/* PGP version information */
#define PGPVERSION_2 2
#define KNOWN_PGP_VERSION(x) ((x)==PGPVERSION_2 || (x) == PGPVERSION_2_6 || \
	    	    	    	    						(x)==PGPVERSION_3)

/* Position in a file */
struct FilePos {
		union {
	struct FilePos *next;
			void *buf;
		} ptr;
		word32 fpos;
};

#define NULLFILEPOS { { (struct FilePos *)0 }, (word32)0 }

/*
 * Since C requires in-order allocation of structure fields, a poor choice of
 * field order can cause lots of holes in a structure for alignment reasons.
 * E.g. long/char/long/char/long/char/long/char assuming the usual 4-byte
 * longs aligned on 4-byte boundaries requires 32 bytes of storage, while
 * long/long/long/long/char/char/char/char requires only 20.
 * Storage required can be minimized by allocating the largest fields in
 * the structure at the ends, against the aligned edges of the structure,
 * and filling the hole in the middle with progressively less aligned
 * fields. We use this by declaring the common (generic) fields in
 * largest-to-smallest order, then the type-specific extensions in
 * smallest-to-largest.
 *
 * The sizes listed are for the platform where memory consumption is the
 * greatest concern - the IBM PC. Other platforms aren't quite as tight.
 */

struct RingGeneric
{
/* 0*/	union RingObject *next, *down, *up;
/*12*/	struct FilePos pos;
/*20*/	ringmask mask;
/*21*/	byte flags;
/*22*/
};

/* Generic flags (apply to more than one type) */
#define RINGOBJF_TRUST 128
#define RINGOBJF_TRUSTCHANGED 64
/*
 * The three type bits are encoded as follows:
 * 76543210
 * 1 - Name (n)
 * 01 - Signature (s)
 * 001 - Key (k)
 * 0010 - Topkey
 * 0011 - Subkey (letter "B")
 * 0001 - Secret (We use the letter "c" for this)
 * 0000 - Unknown keyring object
 * Changes here need to be propagated to ringObjectType().
 */
#define RINGOBJF_NAME 32		/* If 1, lower bits are used by UID */
#define RINGOBJF_SIG 16			/* If 1, this is a signature */
#define RINGOBJF_KEY 8			/* If 1, this is a key */
#define RINGOBJF_SUBKEY 12		/* If both bits set, this is a subkey */
#define RINGOBJF_SEC 4			/* If 1, this is a key's secret components */

/* Combinations of type flags */
#define RINGOBJF_NS	 (RINGOBJF_NAME | RINGOBJF_SIG)
#define RINGOBJF_NSK	(RINGOBJF_NS | RINGOBJF_KEY)
#define RINGOBJF_NSKB	(RINGOBJF_NSK | RINGOBJF_SUBKEY)
#define RINGOBJF_NSKC	(RINGOBJF_NSK | RINGOBJF_SEC)

#define OBJISNAME(obj) ((obj)->g.flags & RINGOBJF_NAME)
#define OBJISSIG(obj) (((obj)->g.flags & RINGOBJF_NS) == RINGOBJF_SIG)
/* Note that this includes top-level keys *and* subkeys */
#define OBJISKEY(obj) (((obj)->g.flags & RINGOBJF_NSK) == RINGOBJF_KEY)
#define OBJISTOPKEY(obj) (((obj)->g.flags & RINGOBJF_NSKB) == RINGOBJF_KEY)
#define OBJISSUBKEY(obj) (((obj)->g.flags & RINGOBJF_NSKB) == RINGOBJF_SUBKEY)
#define OBJISSEC(obj) (((obj)->g.flags & RINGOBJF_NSKC) == RINGOBJF_SEC)
#define OBJISUNK(obj) (((obj)->g.flags & RINGOBJF_NSKC) == 0)

/* These versions are for pgpAssert() macro use */
#define NAMEISNAME(name) ((name)->flags & RINGOBJF_NAME)
#define SIGISSIG(sig) (((sig)->flags & RINGOBJF_NS) == RINGOBJF_SIG)
#define KEYISKEY(key) (((key)->flags & RINGOBJF_NSK) == RINGOBJF_KEY)
#define KEYISTOPKEY(key) (((key)->flags & RINGOBJF_NSKB) == RINGOBJF_KEY)
#define KEYISSUBKEY(key) (((key)->flags & RINGOBJF_NSKB) == RINGOBJF_SUBKEY)

#define OBJFLAGS_NAME	RINGOBJF_NAME
#define OBJFLAGS_SIG	RINGOBJF_SIG
#define OBJFLAGS_KEY	RINGOBJF_KEY
#define OBJFLAGS_SUBKEY	RINGOBJF_SUBKEY
#define OBJFLAGS_SEC	RINGOBJF_SEC
#define OBJFLAGS_UNK	0

/* Does this object's up pointer point to a meaningful parent? */
#define OBJISTOP OBJISTOPKEY
/* Does this object's down pointer point to a meaningful child? */
#define OBJISBOT(obj) (OBJISSIG(obj)||OBJISSEC(obj))

/* Maximum depth of nesting in a keyring */
#define RINGMAXDEPTH 3

/* A subkey is the same, but the "sigsby" pointer is "up" */
struct RingKey
{
		union RingObject *next, *down, *sigsby;
		struct FilePos pos;
		ringmask mask;
		byte flags;
/*22*/	byte trust;
/*23*/	byte pkalg;
/*24*/	word16 validity;		/* Validity period, in days */
#if PGPTRUSTMODEL>0
/*26*/ word16 confidence;		/* A trust value temporary */
#endif
/*28*/	word16 keybits;
/*30*/	word32 tstamp;
/*34*/	byte keyID[8];
/*42*/	struct RingKey *util;
/*46*/
};

#if PGPTRUSTMODEL>0
#define NULLRINGKEY { (union RingObject *)0, (union RingObject *)0,	 \
	(union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_KEY,	\
	0, 0, (word16)0, (word16)0, (word16)0, (word32)0, \
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (struct RingKey *)0	}

#define NULLRINGSUBKEY { (union RingObject *)0, (union RingObject *)0,	\
	(union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_SUBKEY, \
	0, 0, (word16)0, (word16)0, (word16)0, (word32)0, \
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (struct RingKey *)0	}
#else
#define NULLRINGKEY { (union RingObject *)0, (union RingObject *)0,	 \
	(union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_KEY,	\
	0, 0, (word16)0, (word16)0, (word32)0,	\
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (struct RingKey *)0	}

#define NULLRINGSUBKEY { (union RingObject *)0, (union RingObject *)0,	\
	(union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_SUBKEY, \
	0, 0, (word16)0, (word16)0, (word32)0,	\
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (struct RingKey *)0	}
#endif

/* The key has some sort of error */
#define KEYF_ERROR 1

/* The key is a trusted introducer, used in the maintenance pass. */
#define KEYF_TRUSTED 2

/*
 * @@@ Who put these here? They conflict...
 * #define KEYF_USE_SIGN	0x80
 * #define KEYF_USE_ENCRYPT	0x40
 * #define KEYF_USE_SIGN_ENCRYPT	(KEYF_USE_SIGN | KEYF_USE_ENCRYPT)
 */

/*
 * This is used to record information about the locations for a key's
 * secret components. If present, it is always the FIRST child of a
 * key. This is a very boring structure, as there's nothing we can
 * read in about the encrypted secret componenets without the passphrase.
 * (Option: have multiples of these for different encrypted forms of the
 * secret components?)
 *
 * The FilePos entries on this are a subset of the entries in the
 * parent RingKey's.
 */
struct RingSec
{
		union RingObject *next, *down, *up;
		struct FilePos pos;
		ringmask mask;
		byte flags;
/*22*/
/*24*/	word32 hash;	/* Hash of secret fr matching purposes */
/*28*/
};

#define NULLRINGSEC { (union RingObject *)0, (union RingObject *)0,	 \
	(union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_SEC }

/*
 * If SECF_VERSION_BUG is set, then patch the version byte from 3 to 2
 * when using the secret key. This is to fix a bug in 2.6 through early 2.9
 * that, when editing a secret key, would write out the edited secret key
 * with a version byte of 3 even if it originally had a version byte
 * of 2. This has been fixed in 2.9, but it is common enough (due to 2.6)
 * that we need to kludge around it.
 */
#define SECF_VERSION_BUG 1

#if PGPTRUSTMODEL==0
struct RingName
{
		union RingObject *next, *down, *up;
		struct FilePos pos;
		ringmask mask;
		byte flags;
/*22*/	byte trust;
/*23*/	byte flags2;
/*24*/	int trustval;
/*26*/	size_t len;
/*28*/	union {
				char const *ptr;		/* If cached, pointer to string */
				word32 hash;			/* If not cached, hash code */
		} name;
/*32*/
};

#define NULLRINGNAME { (union RingObject *)0, (union RingObject *)0,	\
	    (union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_NAME,	\
	    0, 0, 0, (size_t)0, { (char const *)0 }}

#else /* New trust */
struct RingName
{
		union RingObject *next, *down, *up;
		struct FilePos pos;
		ringmask mask;
		byte flags;
/*22*/	byte trust;
/*23*/	byte validity;			/* Stored validity */
/*24*/	byte confidence;		/* Stored confidence */
/*25*/	byte flags2;
/*26*/	word16 valid;			/* Computed validity */
/*28*/	size_t len;
/*30*/	union {
				char const *ptr;		/* If cached, pointer to string */
				word32 hash;			/* If not cached, hash code */
		} name;
/*34*/
};

/*
 * The intermediate validity results are 16 bits wide instead of 8.
 * This is used for extra precision (fraction bits) and extra range.
 * This is the number of extra precision bits. (4 more range bits
 * increases the upper bound to 10^-25.)
 */
#define TRUST_CERTSHIFT 6

#define NULLRINGNAME { (union RingObject *)0, (union RingObject *)0,	\
	    (union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_NAME,	\
	    0, 0, 0, 0, 0, (size_t)0, { (char const *)0 }}
#endif

/* Access the flags2 field */
#define NAMEF2_ISCACHED 128
#define NAMEF2_NEWTRUST 64
#define NAMEF2_LEVELMASK 31

#define NAMEISCACHED(name) ((name)->flags2 & NAMEF2_ISCACHED)
#define NAMESETCACHED(name) ((name)->flags2 |= NAMEF2_ISCACHED)
#define NAMECLEARCACHED(name) ((name)->flags2 &= ~NAMEF2_ISCACHED)

/* Has new-style trust info */
#define NAMEHASNEWTRUST(name) ((name)->flags2 & NAMEF2_NEWTRUST)
#define NAMESETNEWTRUST(name) ((name)->flags2 |= NAMEF2_NEWTRUST)
#define NAMECLEARNEWTRUST(name) ((name)->flags2 &= ~NAMEF2_NEWTRUST)

/* Certification depth */
#define NAMELEVEL(name) ((name)->flags2 & NAMEF2_LEVELMASK)
#define NAMESETLEVEL(name,lv) \
	((name)->flags2 = ((name)->flags2 & ~NAMEF2_LEVELMASK) + lv)

#define NAMEF_FILEMASK 31

#if NAMEF_FILEMASK+1 < RINGMASKBITS
#error NAMEF_FILEMASK is too small...
#endif

struct RingSig
{
		union RingObject *next, *by, *up;
		struct FilePos pos;
		ringmask mask;
		byte flags;
		/* Prefix */
/*22*/	byte trust;
/*23*/	byte type;
/*24*/	byte hashalg;
/*25*/	byte version;
/*26*/	word16 validity;		/* Delete? */
/*28*/	word32 tstamp;
/*32*/	struct RingSig *nextby;
/*36*/
};

#define NULLRINGSIG { (union RingObject *)0, (union RingObject *)0,	 \
	(union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_SIG,	\
	0, 0, 0, 0, (word16)0, (word32)0, (struct RingSig *)0 }

/* Signature has some sort of parsing error */
#define SIGF_ERROR 1
/* Extra bytes are non-obvious */
#define SIGF_NONFIVE	2
/* This can be derived in other ways, but is kept around for convenience */
#define SIGF_KEY 4

/*
 * An unknown packet in a keyring. "pktbyte" is the type.
 * This is so we can at least propagate the thing, even if it's
 * incomprehensible.
 */
struct RingUnk
{
		union RingObject *next, *down, *up;
		struct FilePos pos;
		ringmask mask;
		byte flags;
		/* Prefix */
/*22*/	byte trust;
/*23*/	byte pktbyte;
/*24*/	word32 hash;
/*28*/
};

#define NULLRINGUNK { (union RingObject *)0, (union RingObject *)0,	 \
	(union RingObject *)0, NULLFILEPOS, (ringmask)0, OBJFLAGS_UNK,	\
	0, 0, (word32)0 }

/*
 * A generic RingObject. "obj->x.foo" occurs so often that the brevity
 * of the one-letter tags has advantages in clarity that override any
 * documentation gains from greater verbosity.
 */
union RingObject {
		struct RingGeneric g;
		struct RingKey k;
		struct RingSec c;		/* Note this odd choice of letter */
		struct RingName n;
		struct RingSig s;
		struct RingUnk u;
};

/*
* The basic collection-of-keys type.
* This comes in two flavours: mutable and immutable.
* A mutable set can have members added and removed,
* while an immutable one cannot.
*/

struct RingPool;
struct RingSet {
		struct RingSet *next;
		struct RingPool *pool;
		struct RingError err;
		ringmask mask;
		char type;
};

#define RINGSET_MUTABLE 0
#define RINGSET_IMMUTABLE 1
#define RINGSET_ITERATOR 2			/* Also immutable */
#define RINGSET_FILE 3	/* Also immutable */
#define RINGSET_FREE 4	/* Free flag */

#define RINGSETISMUTABLE(set) (!(set)->type)

struct RingTrouble;

/* A file that holds information about a file */
struct RingFile {
		struct RingSet set;
		struct PgpFile *f;	/* File handle */
		void (*destructor)(struct RingFile *, struct PgpFile *, void *);
		void *arg;
		struct MemPool strings;	/* Cache of name strings */
		struct MemPool troublepool;
		struct RingTrouble const *trouble, **troubletail;
		struct MemPool fpos;
		struct FilePos *freepos;
		ringmask higherpri;		/* see ringFetchPacket in pgpRngRead.c */
		byte flags;
	PgpVersion version;
};

#if PGPTRUSTMODEL>1
struct Path;
struct PathList;
#endif

/* Flags kept updated about a file */
#define RINGFILEF_DIRTY	1
#define RINGFILEF_TRUSTCHANGED	2

/* The home of a lot of keys */
struct RingPool {
		struct MemPool structs;
		union RingObject *keys;
		union RingObject *freeobjs[RINGTYPE_MAX];
		struct RingSet *sets;	 /* Dynamically allocated sets (not files!) */
		struct RingSet *freesets;
		struct RingIterator *freeiter;
		char *pktbuf;
		size_t pktbuflen;
		size_t pktbufalloc;
		ringmask allocmask;		/* Bits in use (cache) */
		ringmask filemask;		/* Bits in use for files */
		word32 flags;

		struct PgpEnv const *env;
		int certdepth;
#if PGPTRUSTMODEL==0
		int num_marginals;
		int num_completes;
#else
		/* Translations from old to new trust models */
		byte marginalconfidence;
		byte completeconfidence;
		int threshold;
#if PGPTRUSTMODEL==2
    struct MemPool pathpool;
    struct Path *paths;
		struct PathList *pathlists;
#endif
#endif

		/* In case of error, the following is set */
		struct RingError e;

	/* Some large items go at the end... */
	struct RingKey *hashtable[256];
	struct RingFile files[RINGMASKBITS];
};

/* Values for erraction */
#define ACTERR_NONE	 0
#define ACTERR_READ	 1
#define ACTERR_WRITE	2
#define ACTERR_SEEK		3
#define ACTERR_OPEN	4
#define ACTERR_CLOSE	5
#define ACTERR_FLUSH	6
#define ACTERR_HUGE		7

#define ACTERR_EOF				101
#define ACTERR_BADPKTBYTE			102
#define ACTERR_WRONGPKTBYTE			103
#define ACTERR_WRONGLEN				104

#define ACTERR_ALLOC	201

/*
 * How to distinguish starting from stopping?
 * It's NULL pointers either way.
 * Ah, wait! "level" is the most recent level.
 * If it's 0, start the ring. If it's 1, and the stack is NULL,
 * we're at the end. Yes, that works!
 *
 * Okay, so it works like this:
 * The stack[i] array holds the object which is the head of the
 * current list, the last object returned at level i+1.
 * The last object returned, which defines the maximum depth to
 * which the stack is valid, is at "level". Stack entries
 * up to level-1 are valid.
 * If a stack entry is NULL, is is the last entry at level-1,
 * and indicates that the list at this level is finished.
 * Repeated calls will keep returning NULL.
 *
 * If asked for a level one greater than the stored level, that's
 * a clue to start at the beginning of the list.
 *
 * stack[-1], corresponding to a level of 0, is implicitly the
 * RingPool.
 */
struct RingIterator {
	struct RingSet set;
	unsigned level;	/* 1-based */
	union RingObject *stack[RINGMAXDEPTH];
};

void ringErr(struct RingFile *file, word32 fpos, int code);
void ringSimpleErr(struct RingPool *pool, int code);
void ringAllocErr(struct RingPool *pool);

word32 ringHashBuf(byte const *buf, size_t len);
int ringLsBitFind(ringmask mask);
ringmask ringAllocMask(struct RingPool const *pool, struct RingSet const *);
ringmask
ringClearMask(struct RingPool *pool, union RingObject **objp, ringmask mask);
int ringGarbageCollect(struct RingPool *pool);
void ringGarbageCollectObject(struct RingPool *pool, union RingObject *obj);
int ringBitAlloc(struct RingPool *pool);

/* Misc utility functions */
union RingObject *ringNewObject(struct RingPool *pool, int objtype);
#define ringNewKey(pool) ringNewObject(pool, RINGTYPE_KEY)
#define ringNewSec(pool) ringNewObject(pool, RINGTYPE_SEC)
#define ringNewName(pool) ringNewObject(pool, RINGTYPE_NAME)
#define ringNewSig(pool) ringNewObject(pool, RINGTYPE_SIG)
#define ringNewUnk(pool) ringNewObject(pool, RINGTYPE_UNK)
void ringFreeObject(struct RingPool *pool, union RingObject *obj);
void ringRemObject(struct RingPool *pool, union RingObject *obj);

/* Insert a single key into the pool's hash table */
#define RINGPOOLHASHKEY(pool, key) \
	((key)->k.util = (pool)->hashtable[(key)->k.keyID[0]], \
	(pool)->hashtable[(key)->k.keyID[0]] = &(key)->k)
void ringPoolHash(struct RingPool *pool);
union RingObject *
ringPoolFindKey(struct RingPool const *pool, byte pkalg, byte const keyID[8]);
void ringPoolListSigsBy(struct RingPool *pool);
#define ringFileMarkDirty(file)	((file)->flags |= RINGFILEF_DIRTY)
void ringPoolMarkDirty(struct RingPool *pool, ringmask mask);
void ringPoolMarkTrustChanged(struct RingPool *pool, ringmask mask);

union RingObject *ringBestSec(struct RingSet const *set,
	union RingObject const *key);
int ringSubkeyValid(struct RingSet const *set, union RingObject *subkey);
void ringPurgeCachedName(struct RingName *name, ringmask mask);

/* To be added: a function to allocate one of these first! */
void ringPoolInit(struct RingPool *pool, struct PgpEnv const *env);
void ringPoolFini(struct RingPool *pool);

int pgpFingerprint20HashBuf(byte const *buf, size_t len, byte *hash);

void ringObjCheck (union RingObject *obj, ringmask validmask,
								ringmask parentmask);

#ifdef __cplusplus
}
#endif

#endif /* PGPRNGPRIV_H */
