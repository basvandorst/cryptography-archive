/*
 * $Id: pgpRngPriv.h,v 1.30 1998/08/03 04:47:30 hal Exp $
 */

#ifndef Included_pgpRngPriv_h
#define Included_pgpRngPriv_h

#include <stdio.h>	/* For size_t */
#include "pgpRngPub.h"

/* Private */

#include "pgpMemPool.h"

PGP_BEGIN_C_DECLARATIONS

/* See pgpRngMnt.c for a definition of the various trust models */

#ifndef PGPTRUSTMODEL
#define PGPTRUSTMODEL 2
#endif


/* VIRTMASK enables virtual bitmask for unlimited number of sets */


#ifndef VIRTMASK
#define VIRTMASK 0
#endif


#define MEMRINGBIT 0

#if VIRTMASK

typedef struct virtmask
{
	PGPUInt32	nwords;
	PGPUInt32	*words;
} PGPVirtMask;

#define VIRTMASKZERO {0,NULL}

#else /* VIRTMASK */


/*
 * This number should probably be increased for multiple keyrings and GUIs.
 * At the moment, it is low to help memory consumption.
 */
#ifndef RINGMASKBITS
#define RINGMASKBITS 32
#endif

#if RINGMASKBITS <= 8
typedef PGPByte ringmask;
#elif RINGMASKBITS <= 16
typedef PGPUInt16 ringmask;
#elif RINGMASKBITS <= 32
typedef PGPUInt32 ringmask;
#else
#error Invalid RINGMASKBITS value
#endif

#define MEMRINGMASK ((ringmask)1<<MEMRINGBIT)


typedef ringmask PGPVirtMask;

#define VIRTMASKZERO (PGPVirtMask)0

#endif /* VIRTMASK */

/* PGP version information */
#define PGPVERSION_2 2
#define KNOWN_PGP_VERSION(x) ((x)==PGPVERSION_2 || (x) == PGPVERSION_3 || \
							  (x)==PGPVERSION_4)

/* Position in a file */
typedef struct FilePos {
	union {
		struct FilePos *next;
		void *buf;
	} ptr;
	PGPUInt32 fpos;
} FilePos;

#define NULLFILEPOS { { (FilePos *)0 }, (PGPUInt32)0 }

/* 
 * Since C requires in-order allocation of structure fields, a poor choice of
 * field order can cause lots of holes in a structure for alignment reasons.
 * E.g. long/char/long/char/long/char/long/char assuming the usual 4-byte
 * longs aligned on 4-byte boundaries requires 32 bytes of storage, while
 * long/long/long/long/char/char/char/char requires only 20.
 * Storage required can be minimized by allocating the largest fields in
 * the structure at the ends, against the aligned edges of the structure,
 * and filling the hole in the middle with progressively less aligned
 * fields.  We use this by declaring the common (generic) fields in
 * largest-to-smallest order, then the type-specific extensions in
 * smallest-to-largest.
 *
 * The sizes listed are for the platform where memory consumption is the
 * greatest concern - the IBM PC.  Other platforms aren't quite as tight.
 */

struct RingGeneric 
{
/* 0*/	union RingObject *next, *down, *up;
/*12*/	FilePos pos;
/*20*/	PGPVirtMask mask;
/*24*/	PGPByte flags;
/*25*/	PGPByte	flags2;
/*26*/
};

/* Generic flags (apply to more than one type) */
#define RINGOBJF_TRUST 128
#define RINGOBJF_TRUSTCHANGED 64
/*
 * The type bits are encoded as follows:
 * 76543210
 *   0000 - Unknown keyring object
 *   0001 - Secret (We use the letter "c" for this)
 *   0010 - Topkey
 *   0011 - Subkey (letter "B")
 *   0100 - Signature (s)
 *   0101 - Name (n)
 * Changes here need to be propagated to ringObjectType().
 */
#define RINGOBJF_TYPE		0x3c	/* Mask for object type */
#define RINGOBJF_KEYTYPE	0x38	/* Mask ignoring diff among keys */
#define RINGOBJ_SHIFT		2		/* Shift down for type */

#define RINGOBJF_UNK		0x00	/* Unknown object type */
#define RINGOBJF_SEC		0x04	/* Secret component object */
#define RINGOBJF_KEY		0x08	/* Key (top) object */
#define RINGOBJF_SUBKEY		0x0c	/* Subkey object */
#define RINGOBJF_SIG		0x10	/* Signature object */
#define RINGOBJF_NAME		0x14	/* Name object */

#define OBJISNAME(obj) (((obj)->g.flags & RINGOBJF_TYPE) == RINGOBJF_NAME)
#define OBJISSIG(obj) (((obj)->g.flags & RINGOBJF_TYPE) == RINGOBJF_SIG)
/* Note that this includes top-level keys *and* subkeys */
#define OBJISKEY(obj) (((obj)->g.flags & RINGOBJF_KEYTYPE) == RINGOBJF_KEY)
#define OBJISTOPKEY(obj) (((obj)->g.flags & RINGOBJF_TYPE) == RINGOBJF_KEY)
#define OBJISSUBKEY(obj) (((obj)->g.flags & RINGOBJF_TYPE) == RINGOBJF_SUBKEY)
#define OBJISSEC(obj) (((obj)->g.flags & RINGOBJF_TYPE) == RINGOBJF_SEC)
#define OBJISUNK(obj) (((obj)->g.flags & RINGOBJF_TYPE) == RINGOBJF_UNK)

/* These versions are for pgpAssert() macro use */
#define NAMEISNAME(name) (((name)->flags & RINGOBJF_TYPE) == RINGOBJF_NAME)
#define SIGISSIG(sig)   (((sig)->flags & RINGOBJF_TYPE) == RINGOBJF_SIG)
#define KEYISKEY(key)   (((key)->flags & RINGOBJF_KEYTYPE) == RINGOBJF_KEY)
#define KEYISTOPKEY(key)   (((key)->flags & RINGOBJF_TYPE) == RINGOBJF_KEY)
#define KEYISSUBKEY(key)   (((key)->flags & RINGOBJF_TYE) == RINGOBJF_SUBKEY)

#define OBJFLAGS_NAME	RINGOBJF_NAME
#define OBJFLAGS_SIG	RINGOBJF_SIG
#define OBJFLAGS_KEY	RINGOBJF_KEY
#define OBJFLAGS_SUBKEY	RINGOBJF_SUBKEY
#define OBJFLAGS_SEC	RINGOBJF_SEC
#define OBJFLAGS_UNK	RINGOBJF_UNK

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
	FilePos pos;
	PGPVirtMask mask;
	PGPByte flags;
	PGPByte flags2;
/*26*/	PGPByte trust;
/*27*/	PGPByte pkalg;
/*28*/	PGPUInt16 validity;	/* Validity period, in days */
#if PGPTRUSTMODEL>0
/*30*/  PGPUInt16 confidence;	/* A trust value temporary */
#endif
/*32*/	PGPUInt16 keybits;
/*34*/	PGPByte signedTrust;	/* Trust from sigs on key */
/*35*/
/*36*/	PGPUInt32 tstamp;
/*40*/	PGPByte keyID[8];
/*48*/	struct RingKey *util;
/*52*/	void *regexp;
/*56*/
};
typedef struct RingKey RingKey;

#if PGPTRUSTMODEL>0
#define NULLRINGKEY { (union RingObject *)0, (union RingObject *)0,		\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_KEY, 0,	\
	0, 0, (PGPUInt16)0, (PGPUInt16)0, (PGPUInt16)0, 0, (PGPUInt32)0, 	\
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (RingKey *)0, 0	}

#define NULLRINGSUBKEY { (union RingObject *)0, (union RingObject *)0,		\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_SUBKEY, 0,	\
	0, 0, (PGPUInt16)0, (PGPUInt16)0, (PGPUInt16)0, 0, (PGPUInt32)0,		\
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (RingKey *)0, 0	}
#else
#define NULLRINGKEY { (union RingObject *)0, (union RingObject *)0,		\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_KEY, 0,	\
	0, 0, (PGPUInt16)0, (PGPUInt16)0, 0, (PGPUInt32)0,					\
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (RingKey *)0, 0	}

#define NULLRINGSUBKEY { (union RingObject *)0, (union RingObject *)0,		\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_SUBKEY, 0,	\
	0, 0, (PGPUInt16)0, (PGPUInt16)0, 0, (PGPUInt32)0,						\
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, (RingKey *)0, 0	}
#endif

/* The key has some sort of error */
#define KEYF_ERROR 1

/* The key is a trusted introducer, used in the maintenance pass. */
#define KEYF_TRUSTED 2

/* Access the flags2 field */

/* V2 or V3 key, something earlier than V4 */
#define KEYF2_V3		128

#define KEYISV3(key) ((key)->flags2 & KEYF2_V3)
#define KEYSETV3(key) ((key)->flags2 |= KEYF2_V3)
#define KEYCLEARV3(key) ((key)->flags2 &= ~KEYF2_V3)



/*
 * This is used to record information about the locations for a key's
 * secret components.  If present, it is always the FIRST child of a
 * key.  This is a very boring structure, as there's nothing we can
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
	FilePos pos;
	PGPVirtMask mask;
	PGPByte flags;
	PGPByte flags2;
/*26*/
/*28*/	PGPUInt32 hash;	/* Hash of secret fr matching purposes */
/*32*/
};
typedef struct RingSec  RingSec;

#define NULLRINGSEC { (union RingObject *)0, (union RingObject *)0,	\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_SEC, 0, 0 }

/*
 * If SECF_VERSION_BUG is set, then patch the version byte from 3 to 2
 * when using the secret key.  This is to fix a bug in 2.6 through early 2.9
 * that, when editing a secret key, would write out the edited secret key
 * with a version byte of 3 even if it originally had a version byte
 * of 2.  This has been fixed in 2.9, but it is common enough (due to 2.6)
 * that we need to kludge around it.
 */
#define SECF_VERSION_BUG 1

#if PGPTRUSTMODEL==0
struct RingName
{
	union RingObject *next, *down, *up;
	FilePos pos;
	PGPVirtMask mask;
	PGPByte flags;
	PGPByte flags2;
/*26*/	PGPByte trust;
/*27*/
/*28*/	int trustval;
/*30*/	size_t len;
/*32*/	union {
		char const *ptr;	/* If cached, pointer to string */
		PGPUInt32 hash;		/* If not cached, hash code */
	} name;
/*36*/	PGPUInt32 filemaskindex;	/* PGPFile for cached name */
/*40*/
};
typedef struct RingName	RingName;

#define NULLRINGNAME { (union RingObject *)0, (union RingObject *)0,	\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_NAME, 0,	\
	0, 0, (size_t)0, { (char const *)0 }}

#else /* New trust */
struct RingName
{
	union RingObject *next, *down, *up;
	FilePos pos;
	PGPVirtMask mask;
	PGPByte flags;
	PGPByte flags2;
/*26*/	PGPByte trust;
/*27*/	PGPByte validity;		/* Stored validity */
/*28*/	PGPByte confidence;	/* Stored confidence */
/*29*/
/*30*/	PGPUInt16 valid;		/* Computed validity */
/*32*/	size_t len;
/*36*/	union {
		char const *ptr;	/* If cached, pointer to string */
		PGPUInt32 hash;		/* If not cached, hash code */
	} name;
/*40*/	PGPUInt32 filemaskindex;	/* PGPFile for cached name */
/*44*/
};
typedef struct RingName	RingName;

/*
 * The intermediate validity results are 16 bits wide instead of 8.
 * This is used for extra precision (fraction bits) and extra range.
 * This is the number of extra precision bits.  (4 more range bits
 * increases the upper bound to 10^-25.)
 */
#define TRUST_CERTSHIFT 6

#define NULLRINGNAME { (union RingObject *)0, (union RingObject *)0,	\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_NAME,	0,	\
	0, 0, 0, 0, (size_t)0, { (char const *)0 }, 0}
#endif

/* Access the flags2 field */
#define NAMEF2_ISCACHED	128
#define NAMEF2_NEWTRUST 64
#define NAMEF2_ATTR		32
#define NAMEF2_LEVELMASK 31

#define NAMEISCACHED(name) ((name)->flags2 & NAMEF2_ISCACHED)
#define NAMESETCACHED(name) ((name)->flags2 |= NAMEF2_ISCACHED)
#define NAMECLEARCACHED(name) ((name)->flags2 &= ~NAMEF2_ISCACHED)

/* Has new-style trust info */
#define NAMEHASNEWTRUST(name) ((name)->flags2 & NAMEF2_NEWTRUST)
#define NAMESETNEWTRUST(name) ((name)->flags2 |= NAMEF2_NEWTRUST)
#define NAMECLEARNEWTRUST(name) ((name)->flags2 &= ~NAMEF2_NEWTRUST)

/* Is general attribute */
#define NAMEISATTR(name) ((name)->flags2 & NAMEF2_ATTR)
#define NAMESETATTR(name) ((name)->flags2 |= NAMEF2_ATTR)
#define NAMECLEARATTR(name) ((name)->flags2 &= ~NAMEF2_ATTR)

/* Certification depth */
#define NAMELEVEL(name) ((name)->flags2 & NAMEF2_LEVELMASK)
#define NAMESETLEVEL(name,lv) \
	((name)->flags2 = ((name)->flags2 & ~NAMEF2_LEVELMASK) + lv)

#define NAMEF_FILEMASK 0xffffffff

/* File mask value tells where name is cached from */
#define NAMEFILEMASK(name) ((name)->filemaskindex & NAMEF_FILEMASK)
#define NAMESETFILEMASK(name,val) ((name)->filemaskindex = \
	((name)->filemaskindex &~NAMEF_FILEMASK) | ((val)&NAMEF_FILEMASK))

struct RingSig
{
	union RingObject *next, *by, *up;
	FilePos pos;
	PGPVirtMask mask;
	PGPByte flags;
	PGPByte flags2;
	/* Prefix */
/*26*/	PGPByte trust;
/*27*/	PGPByte type;
/*28*/	PGPByte hashalg;
/*29*/	PGPByte version;
/*30*/	PGPUInt16 validity;
/*32*/	PGPUInt32 tstamp;
/*36*/	struct RingSig *nextby;
/*40*/	void *regexp;
/*41*/	PGPByte trustLevel;	/* 0 for regular sig, 1 for trust, 2 for meta */
/*42*/	PGPByte	trustValue;	/* new "extern" format, valid if trustLevel>0 */
/*43*/
};
typedef struct RingSig	RingSig;

#define NULLRINGSIG { (union RingObject *)0, (union RingObject *)0,		\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_SIG, 0,	\
	0, 0, 0, 0, (PGPUInt16)0, (PGPUInt32)0, (RingSig *)0, 0, 0, 0 }

/* Signature has some sort of parsing error */
#define SIGF_ERROR 1
/* Extra bytes are non-obvious */
#define SIGF_NONFIVE	2

/* Access the flags2 field */
#define SIGF2_EXPORTABLE	128		/* Sig can be exported to others */
#define SIGF2_USESREGEXP	 64		/* (Trust) sig qualified with regexp */
#define SIGF2_REVOCABLE		 32		/* Sig can be revoked */
#define SIGF2_X509			 16		/* Sig is an imported X.509 cert */

#define SIGSETEXPORTABLE(sig)		(sig)->flags2 |= SIGF2_EXPORTABLE
#define SIGSETNONEXPORTABLE(sig)	(sig)->flags2 &= ~SIGF2_EXPORTABLE
#define SIGISEXPORTABLE(sig)		(((sig)->flags2 & SIGF2_EXPORTABLE)!=0)
#define SIGSETUSESREGEXP(sig)		(sig)->flags2 |= SIGF2_USESREGEXP
#define SIGCLEARUSESREGEXP(sig)		(sig)->flags2 &= ~SIGF2_USESREGEXP
#define SIGUSESREGEXP(sig)			(((sig)->flags2 & SIGF2_USESREGEXP)!=0)
#define SIGSETREVOCABLE(sig)		(sig)->flags2 |= SIGF2_REVOCABLE
#define SIGSETNONREVOCABLE(sig)		(sig)->flags2 &= ~SIGF2_REVOCABLE
#define SIGISREVOCABLE(sig)			(((sig)->flags2 & SIGF2_REVOCABLE)!=0)
#define SIGSETX509(sig)				(sig)->flags2 |= SIGF2_X509
#define SIGCLEARX509(sig)			(sig)->flags2 &= ~SIGF2_X509
#define SIGISX509(sig)				(((sig)->flags2 & SIGF2_X509)!=0)


/*
 * An unknown packet in a keyring.  "pktbyte" is the type.
 * This is so we can at least propagate the thing, even if it's
 * incomprehensible.
 */
struct RingUnk
{
	union RingObject *next, *down, *up;
	FilePos pos;
	PGPVirtMask mask;
	PGPByte flags;
	PGPByte flags2;
	/* Prefix */
/*26*/	PGPByte trust;
/*27*/	PGPByte pktbyte;
/*28*/	PGPUInt32 hash;
/*32*/
};
typedef struct RingUnk RingUnk;

#define NULLRINGUNK { (union RingObject *)0, (union RingObject *)0,		\
	(union RingObject *)0, NULLFILEPOS, VIRTMASKZERO, OBJFLAGS_UNK, 0,	\
	0, 0, (PGPUInt32)0 }

/*
 * A generic RingObject.  "obj->x.foo" occurs so often that the brevity
 * of the one-letter tags has advantages in clarity that override any
 * documentation gains from greater verbosity.
 */
union RingObject {
	struct RingGeneric g;
	RingKey k;
	RingSec c;	/* Note this odd choice of letter */
	RingName n;
	RingSig s;
	RingUnk u;
};

/*
 * The basic collection-of-keys type.
 * This comes in two flavours: mutable and immutable.
 * A mutable set can have members added and removed,
 * while an immutable one cannot.
 */

struct RingPool;
struct RingSet {
	RingSet *next;
	RingPool *pool;
	RingError err;
	PGPVirtMask mask;
	char type;
};

#define RINGSET_MUTABLE 0
#define RINGSET_IMMUTABLE 1
#define RINGSET_ITERATOR 2	/* Also immutable */
#define RINGSET_FILE 3	/* Also immutable */
#define RINGSET_FREE 4	/* Free flag */

#define RINGSETISMUTABLE(set) (!(set)->type)

struct RingTrouble;

/* A file that holds information about a file */
struct RingFile {
	RingSet set;
	PGPFile *f;	/* File handle */
	void (*destructor)(RingFile *, PGPFile *, void *);
	void *arg;
	MemPool strings;	/* Cache of name strings */
	MemPool troublepool;
	struct RingTrouble const *trouble, **troubletail;
	MemPool fpos;
	FilePos *freepos;
	PGPVirtMask higherpri;	/* see ringFetchPacket in pgpRngRead.c */
	PGPByte flags;
        PgpVersion version;     
};

#if PGPTRUSTMODEL>1

/* This is a path or a path segment.  next->src may be unequal to dest. */
typedef struct Path {
	struct Path         *next;
	RingObject          *src,
	                    *dest;
	double               confidence;
	DEBUG_STRUCT_CONSTRUCTOR( Path )
} Path, *pPath;

/* This is a list of paths.  Some segments may be on multiple paths. */
typedef struct PathList {
	struct PathList     *next;
	Path                *path;
	Path               **ptail;
	double               confidence;
	DEBUG_STRUCT_CONSTRUCTOR( PathList )
} PathList, *pPathList;
#endif

/* Flags kept updated about a file */
#define RINGFILEF_DIRTY	1
#define RINGFILEF_TRUSTCHANGED	2

/* The home of a lot of keys */
struct RingPool {
	PGPContextRef	context;
	
	MemPool structs;
	union RingObject *keys;
	union RingObject *freeobjs[RINGTYPE_MAX];
	RingSet *sets;	/* Dynamically allocated sets (not files!) */
	RingSet *freesets;
	RingIterator *freeiter;
	char *pktbuf;
	size_t pktbuflen;
	size_t pktbufalloc;
	PGPVirtMask allocmask;	/* Bits in use (cache) */
	PGPVirtMask filemask;	/* Bits in use for files */
	PGPVirtMask memringmask;	/* Const, has MEMRING bit set */
	PGPUInt32 flags;

	PGPEnv const *env;
	int certdepth;
	/* TRUSTMODEL 0 values */
	int num_marginals;
	int num_completes;
	/* TRUSTMODEL 1 values */
	/* Translations from old to new trust models */
	PGPByte marginalconfidence;
	PGPByte completeconfidence;
	int threshold;
#if PGPTRUSTMODEL==2
    MemPool pathpool;
    Path *paths;
	PathList *pathlists;
#endif
	MemPool regexps;

	/* In case of error, the following is set */
	RingError e;

	/* Some large items go at the end... */
	RingKey *hashtable[256];
	PGPUInt32 nfiles;
	RingFile **files;
};

/* Values for erraction */
#define ACTERR_NONE	0
#define ACTERR_READ	1
#define ACTERR_WRITE	2
#define ACTERR_SEEK	3
#define ACTERR_OPEN     4
#define ACTERR_CLOSE    5
#define ACTERR_FLUSH    6
#define ACTERR_HUGE	7

#define ACTERR_EOF		101
#define ACTERR_BADPKTBYTE	102
#define ACTERR_WRONGPKTBYTE	103
#define ACTERR_WRONGLEN		104

#define ACTERR_ALLOC	201

/* Test for membership of a RingObj in a RingSet */
#define pgpIsRingSetMember(s,o)	\
	pgpVirtMaskIsOverlapping(&(s)->mask, &(o)->g.mask)

/*
 * How to distinguish starting from stopping?
 * It's NULL pointers either way.
 * Ah, wait!  "level" is the most recent level.
 * If it's 0, start the ring.  If it's 1, and the stack is NULL,
 * we're at the end.  Yes, that works!
 *
 * Okay, so it works like this:
 * The stack[i] array holds the object which is the head of the
 * current list, the last object returned at level i+1.
 * The last object returned, which defines the maximum depth to
 * which the stack is valid, is at "level".  Stack entries
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
	RingSet set;
	unsigned level;	/* 1-based */
	union RingObject *stack[RINGMAXDEPTH];
};


PGPError pgpVirtMaskInit (RingPool const *pool, PGPVirtMask *mask);
PGPError pgpVirtMaskCleanup (RingPool const *pool, PGPVirtMask *mask);
PGPError pgpVirtMaskOR (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask);
PGPError pgpVirtMaskAND (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask);
PGPError pgpVirtMaskANDNOT (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask);
PGPError pgpVirtMaskSetBit (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 bitnumber);
PGPError pgpVirtMaskClearBit (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 bitnumber);
PGPError pgpVirtMaskClearGreaterBits (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 firstbitnumber);
PGPError pgpVirtMaskNOT (RingPool const *pool, PGPVirtMask *mask,
	PGPUInt32 highbitnumber);
PGPError pgpVirtMaskCopy (RingPool const *pool, PGPVirtMask const *imask,
	PGPVirtMask *omask);
PGPInt32 pgpVirtMaskLSBit (PGPVirtMask const *mask);
PGPBoolean pgpVirtMaskIsEmpty (PGPVirtMask const *mask);
PGPBoolean pgpVirtMaskIsEqual (PGPVirtMask const *mask1,
	PGPVirtMask const *mask2);
PGPBoolean pgpVirtMaskIsOverlapping (PGPVirtMask const *mask1,
	PGPVirtMask const *mask2);

void ringErr(RingFile *file, PGPUInt32 fpos, PGPError code);
void ringSimpleErr(RingPool *pool, PGPError code);
void ringAllocErr(RingPool *pool);

PGPUInt32 ringHashBuf(PGPByte const *buf, size_t len);
int ringLsBitFind(PGPUInt32 mask);
PGPError ringAllocMask(RingPool const *pool, RingSet const *,
	PGPVirtMask *omask);
PGPError
ringClearMask(RingPool *pool, union RingObject **objp, PGPVirtMask *andmask,
	PGPVirtMask *andnotmask, PGPVirtMask *omask);
PGPBoolean ringGarbageCollect(RingPool *pool);
void ringGarbageCollectObject(RingPool *pool, union RingObject *obj);
PGPError ringBitAlloc(RingPool *pool, PGPUInt32 *newbit);

/* Misc utility functions */
union RingObject *ringNewObject(RingPool *pool, int objtype);
#define ringNewKey(pool) ringNewObject(pool, RINGTYPE_KEY)
#define ringNewSec(pool) ringNewObject(pool, RINGTYPE_SEC)
#define ringNewName(pool) ringNewObject(pool, RINGTYPE_NAME)
#define ringNewSig(pool) ringNewObject(pool, RINGTYPE_SIG)
#define ringNewUnk(pool) ringNewObject(pool, RINGTYPE_UNK)
void ringFreeObject(RingPool *pool, union RingObject *obj);
void ringRemObject(RingPool *pool, union RingObject *obj);

/* Insert a single key into the pool's hash table */
#define RINGPOOLHASHKEY(pool, key)  \
	((key)->k.util = (pool)->hashtable[(key)->k.keyID[0]], \
	 (pool)->hashtable[(key)->k.keyID[0]] = &(key)->k)
void ringPoolHash(RingPool *pool);
union RingObject *
ringPoolFindKey(RingPool const *pool, PGPByte pkalg, PGPByte const keyID[8]);
void ringPoolListSigsBy(RingPool *pool);
#define ringFileMarkDirty(file)	((file)->flags |= RINGFILEF_DIRTY)
void ringPoolMarkDirty(RingPool *pool, PGPVirtMask *mask);
void ringPoolMarkTrustChanged(RingPool *pool, PGPVirtMask *mask);

PGPError ringObjBetters(union RingObject const *obj, RingPool const *pool,
	PGPVirtMask *omask);
int ringSubkeyValid(RingSet const *set, union RingObject *subkey,
	PGPBoolean unExpired);
void ringPurgeCachedName(RingPool const *pool, RingName *name,
	PGPVirtMask *mask);

void ringFilesInit(RingPool *pool, PGPUInt32 nfiles);
void ringPoolInit( PGPContextRef context, RingPool *pool, PGPEnv const *env);
void ringPoolFini(RingPool *pool);

int pgpFingerprint20HashBuf(PGPContextRef context, PGPByte const *buf,
							size_t len, PGPByte *hash);

void * ringSigRegexp(RingSet const *set, RingObject *sig);

PGPBoolean sigRevokesKey(RingSet const *set, RingObject *sig);

PGP_END_C_DECLARATIONS

#endif /* Included_pgpRngPriv_h */
