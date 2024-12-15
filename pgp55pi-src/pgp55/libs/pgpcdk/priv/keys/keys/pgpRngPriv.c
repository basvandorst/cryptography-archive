/*
 * Private helper functions for keyring manipulation.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngPriv.c,v 1.26 1997/10/28 02:19:29 hal Exp $
 */
#include "pgpConfig.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "pgpDebug.h"
#include "pgpPktByte.h"
#include "pgpRngPriv.h"
#include "pgpRngPars.h"
#include "pgpRngRead.h"		/* For ringFilePurgeTrouble */
#include "pgpTrstPkt.h"
#include "pgpHashPriv.h"
#include "pgpEnv.h"
#include "pgpErrors.h"
#include "pgpMem.h"
#include "pgpSigSpec.h"	/* for PGP_SIGTYPE */
#include "pgpTrust.h"
#include "pgpContext.h"
#include "pgpRegExp.h"

#ifndef NULL
#define NULL 0
#endif

#define PGP_TRUST_DECADE_INTERNAL (PGP_TRUST_DECADE >> 6)
#define PGP_TRUST_OCTAVE_INTERNAL (PGP_TRUST_OCTAVE >> 6)

/*
 * Small helpers to report errors
 */
/* Report an general I/O error */
void
ringErr(RingFile *file, PGPUInt32 pos, PGPError code)
{
	RingError *err = &file->set.pool->e;

	err->f = file;
	err->fpos = pos;
	err->error = code;
	err->syserrno = errno;
}

/* Report a non-I/O error */
void
ringSimpleErr(RingPool *pool, PGPError code)
{
	pool->e.f = (RingFile *)NULL;
	pool->e.fpos = (PGPUInt32)-1;
	pool->e.error = code;
	pool->e.syserrno = errno;
}

/* Report an allocation failure (called from many places) */
void
ringAllocErr(RingPool *pool)
{
	ringSimpleErr(pool, kPGPError_OutOfMemory);
}

/*
 * Hash a string of bytes.  Uses the CRC-32 polynomial, preset
 * to -1, non-invert.  Used to reduce userID collisions and to
 * create a fake keyID for unparseable keys.
 *
 * CRC-32 polynomial in little-endian order:
 *   1+x+x^2+x^4+x^5+x^7+x^8+x^10+x^11+x^12+x^16+x^22+x^23+x^26+x^32
 *               1   1   2   2   2   3
 *   0   4   8   2   6   0   4   8   2
 * = 111011011011100010000011001000001
 * =    e   d   b   8   8   3   2   0
 * = 0xedb88320
 */
#define CRCPOLY 0xedb88320
PGPUInt32
ringHashBuf(PGPByte const *buf, size_t len)
{
	PGPUInt32 crc;
	int i, j;
	static PGPUInt32 crctable[256];

	if (!crctable[255]) {
		/* crctable[0] is already 0 */
		crctable[128] = crc = CRCPOLY;
		i = 64;
		do {
			crc = crc>>1 ^ (crc & 1 ? CRCPOLY : 0);
			for (j = 0; j < 256; j += 2*i)
				crctable[i+j] = crc ^ crctable[j];
		} while (i >>= 1);
	}

	crc = 0xffffffff;
	while (len--)
		crc = (crc >> 8) ^ crctable[(crc ^ *buf++) & 255];
	return crc;
}

/*
 * Return the index of the least significant bit in the given mask,
 * or -1 if the mask is all 0.  This uses (almost) no branches,
 * so should be nice and fast on modern processors.
 *
 * Oh, how does it *work*, you ask?  I do confess that this uses
 * two evil bit-twiddling tricks.  The first is one to get a mask
 * of the least-significant set bit in few instructions.
 * Consider a binary number x, be it 11010000 or 00101111.
 * Then think about the form of x-1, 11001111 or 00101110.
 * Notice that the only difference is that some least-significant
 * bits have been complemented.  The bits complemented are
 * those up to and including the least-significant set bit.
 * (x+1 does the same with *clear* bits).  So ANDing the two
 * results in a number like the original, only without the
 * least-significant set bit.  XORing them produces a mask
 * with a number of least-significant bits set, depending on
 * the least-significant set bit,    00011111 or 00000001.
 *
 * Other tricks: x & (x-1) returns x, but with the least-significant
 * bit cleared.  Twos complement negation is -x = ~x+1 = ~(x-1),
 * so x & -x = x & ~(x-1) returns only the least-significant bit
 * set in x.
 *
 * All we need is a routine to count the bits, and we're done.
 * This is the *second* trick.  Consider an 8-bit word:
 * +-+-+-+-+-+-+-+-+
 * |a|b|c|d|e|f|g|h|
 * +-+-+-+-+-+-+-+-+
 * Now copy this word, shift one copy down one bit, and AND both
 * copies with 0x55 (01010101), to produce even and odd bits:
 * +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 * | |b| |d| |f| |h|  | |a| |c| |e| |g| (the blank squares are zero)
 * +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 * Then add the words together:
 * +-+-+-+-+-+-+-+-+
 * |a+b|c+d|e+f|g+h|
 * +-+-+-+-+-+-+-+-+
 * Note that each two-bit field contains a count of the number of
 * bits set in that part of the original word.  Repeating this produces:
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 * |   |c+d|   |g+h| + |   |a+b|   |e+f| = |a+b+c+d|e+f+g+h|
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 * and once more produces:
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 * |       |e+f+g+h| + |       |a+b+c+d| = |a+b+c+d+e+f+g+h|
 * +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+   +-+-+-+-+-+-+-+-+
 *
 * Ther masking is needed so that fields don't overflow into
 * adjacent fields.  Once the fields have gotten wide enough,
 * some of it can be reduced or eliminated.  In the last step,
 * ab+c+d and e+f+g+h are both at most 4 (binary 100) and
 * their sum is at most 8 (binary 1000), which still fits into a
 * 4-bit field, so it is possible to not mask the inputs to
 * the addition.  It's still necessary to mask the output, though,
 * since the next step (adding up to a maximum of 16) won't
 * fit into a 4-bit field.  Once you have an 8-bit field, though,
 * you can stop masking until the very end unless you have a
 * 256-bit word.
 */
int
ringLsBitFind(ringmask mask)
{
	if (!mask)
		return -1;
	mask ^= mask-1;	/* Number of bits set is position of lsbit + 1 */
#if RINGMASKBITS > 32
	mask = (mask & 0x5555555555555555) + (mask >> 1 & 0x5555555555555555);
	mask = (mask & 0x3333333333333333) + (mask >> 2 & 0x3333333333333333);
	mask = (mask + (mask >> 4)) & 0x0F0F0F0F0F0F0F0F;
	mask += mask >> 8;
	mask += mask >> 16;
	mask += mask >> 32;
	return (int)(mask & 255)-1;
#elif RINGMASKBITS > 16
	mask = (mask & 0x55555555) + (mask >> 1 & 0x55555555);
	mask = (mask & 0x33333333) + (mask >> 2 & 0x33333333);
	mask = (mask + (mask >> 4)) & 0x0F0F0F0F;
	mask += mask >> 8;
	mask += mask >> 16;
	return (int)(mask & 255)-1;
#elif RINGMASKBITS > 8
	mask = (mask & 0x5555) + (mask >> 1 & 0x5555);
	mask = (mask & 0x3333) + (mask >> 2 & 0x3333);
	mask = (mask + (mask >> 4)) & 0x0F0F;
	mask += mask >> 8;
	return (int)(mask & 255)-1;
#else
	mask = (mask & 0x55) + (mask >> 1 & 0x55);
	mask = (mask & 0x33) + (mask >> 2 & 0x33);
	mask = (mask + (mask >> 4)) & 0x0F;
	return (int)(mask - 1);
#endif
}

/*
 * Return the mask of bits which are actually in use, excepting the
 * given RingSet, if non-null.  I.e. the mask which would be in use
 * if that RingSet were discarded.  RingFile structures are not
 * included on the sets list, so they are accounted for in the
 * pool->filemask.
 */
ringmask
ringAllocMask(RingPool const *pool, RingSet const *set0)
{
	RingSet const *set;
	ringmask mask = pool->filemask;

	for (set = pool->sets; set; set = set->next)
		mask |= set->mask;
	if (set0)		/* set0 may or may not be on the sets list */
		mask &= ~set0->mask;
	return mask;
}

/* Helper function for ringGarbageCollect */
ringmask
ringClearMask(RingPool *pool, union RingObject **objp, ringmask mask)
{
	union RingObject *robj;
	ringmask omask;
	ringmask remmask = 0;

	while ((robj = *objp) != (union RingObject *) 0) {
	    omask = robj->g.mask &= mask;
	    if (!OBJISBOT(robj))
	        remmask |= ringClearMask(pool, &robj->g.down, mask);
	    /*  Skip dummy objects (robj->g.mask == 0), but delete
	        objects that are only in the memory ring. Also skip
		keys which must be kept as dummy keys (because they've
		signed something). */
	    if (robj->g.mask && !(omask&~MEMRINGMASK) &&
		!(OBJISKEY(robj) && robj->k.sigsby != NULL)) {
		/*
		 * Delete no-longer-used object.
		 * This does not free the memring data area, however it will
		 * be reclaimed in ringGarbageCollect once no objects are using
		 * that area.
		 *
		 */
	        pgpAssert(OBJISBOT(robj) || !robj->g.down);
		*objp = robj->g.next;
		ringFreeObject(pool, robj);
	    } 
	    else {
	        /* Skip to next object */
	        remmask |= omask;
		objp = &robj->g.next;
	    }
	}
	return remmask;
}

/*
 * Reclaim all unused bits and delete any unreferenced memory objects.
 */
int
ringGarbageCollect(RingPool *pool)
{
	ringmask mask;

	/*  Build sig lists so we know which keys act as signers.
	    These should be left as dummy keys rather than freed. */
	ringPoolListSigsBy (pool);
	mask = ringAllocMask(pool, (RingSet const *)NULL);
	if (mask != pool->allocmask) {
		pool->allocmask = mask;
		mask = ringClearMask(pool, &pool->keys, mask);
		if (!(mask & MEMRINGMASK)) {
			memPoolEmpty(&pool->files[MEMRINGBIT].strings);
			memPoolEmpty(&pool->files[MEMRINGBIT].fpos);
			pool->files[MEMRINGBIT].freepos = NULL;
			return 1;	/* Something freed */
		}
	}
	return 0;	/* Nothing freed */
}

/* Remove a single key from its hash chain */
static void
ringGarbageHackKey(RingPool *pool, RingKey *key)
{
	RingKey **keyp;
	
	keyp = &pool->hashtable[key->keyID[0]];

	while (*keyp != key) {
		pgpAssert(*keyp);
		keyp = &(*keyp)->util;
	}

	*keyp = key->util;
}

/* Remove a single signature from its sigsby list */
static void
ringGarbageHackSig(RingSig *sig)
{
	RingKey *key;
	RingSig **sigp;
	
	key = &sig->by->k;
	pgpAssert(KEYISKEY(key));

	/*
	 * This could be one loop but for type rules, sigh...
	 * The problem doesn't happen often, fortunately.
	 * (The single loop can be expressed in C using the
	 * cheat sigp = (RingSig **)&key->sigsby; but
	 * while that's portable in practice, we eschew it
	 * for the sake of ANSI C purity.)
	 */
	if (&key->sigsby->s == sig) {
		key->sigsby = (union RingObject *)sig->nextby;
	} else {
		sigp = &key->sigsby->s.nextby;
		while (*sigp != sig)
			sigp = &(*sigp)->nextby;
		*sigp = sig->nextby;
	}
}

/*
 * Delete a single object from the global pool if it is an unreferenced
 * memory object.
 */
void
ringGarbageCollectObject(RingPool *pool, union RingObject *robj)
{
	union RingObject **objp;

	if (robj->g.mask == MEMRINGMASK) {
		pgpAssert(!robj->g.down);
		objp = OBJISTOP(robj) ? &pool->keys : &robj->g.up->g.down;
		while (*objp != robj) {
			pgpAssert(*objp);
			objp = &(*objp)->g.next;
		}
		*objp = robj->g.next;
		if (OBJISKEY(robj))
			ringGarbageHackKey(pool, &robj->k);
		else if (OBJISSIG(robj))
			ringGarbageHackSig(&robj->s);
		ringFreeObject(pool, robj);
	}
}

int
ringBitAlloc(RingPool *pool)
{
	ringmask mask;

	/* Allocate a new bit */
	mask = ~pool->allocmask;
	if (!mask) {
		/* Wups, out of bits - try something before dying */
		ringGarbageCollect(pool);
		mask = ~pool->allocmask;
		if (!mask) {
			ringSimpleErr(pool, kPGPError_OutOfRings);
			return kPGPError_OutOfRings;
		}
	}
	return ringLsBitFind(mask);
}

/*
 * Allocate and deallocate useful structures.
 * Note the interesting shenanigans used to allocate
 * a structure the alignment of an enclosing union, ensuring
 * that even on a maximally-perverse ANSI C implementation,
 * it is safe to cast the returned structure pointer to a union
 * pointer.
 */
union RingObject *
ringNewObject(RingPool *pool, int objtype)
{
	union RingObject *robj;
	/* How to initialize each object to empty */
	static RingKey const nullkey = NULLRINGKEY;
	static RingSec const nullsec = NULLRINGSEC;
	static RingName const nullname = NULLRINGNAME;
	static RingSig const nullsig = NULLRINGSIG;
	static RingUnk const nullunk = NULLRINGUNK;
	static void const *nullobjs[RINGTYPE_MAX] = {
		&nullkey, &nullsec, &nullname, &nullsig, &nullunk
	};
	size_t const sizes[RINGTYPE_MAX] = {
		sizeof(RingKey), sizeof(RingSec),
		sizeof(RingName), sizeof(RingSig),
		sizeof(RingUnk)
	};

	/* Object types are 1-based */
	pgpAssert(objtype > 0);
	pgpAssert(objtype <= RINGTYPE_MAX);

	robj = pool->freeobjs[objtype-1];
	if (robj) {
		pool->freeobjs[objtype-1] = robj->g.next;
	} else {
		robj = (union RingObject *)
			memPoolAlloc(&pool->structs, sizes[objtype-1],
				     alignof(union RingObject));
		if (!robj) {
			ringAllocErr(pool);
			return NULL;
		}
	}
	memcpy(robj, nullobjs[objtype-1], sizes[objtype-1]);
	pgpAssert(ringObjectType(robj) == objtype);
	return robj;
}

/*
 * Free an object.  This does not do any cleanup with any pointers in the
 * object, except the regexp which belongs to sig objects.
 */
void
ringFreeObject(RingPool *pool, union RingObject *obj)
{
	int type = ringObjectType(obj);

	pgpAssert(type > 0);
	pgpAssert(type <= RINGTYPE_MAX);

	if (OBJISSIG(obj)) {
		if (obj->s.regexp) {
			pgpContextMemFree( pool->context, obj->s.regexp );
			obj->s.regexp = NULL;
		}
	}

	obj->g.next = pool->freeobjs[type-1];
	pool->freeobjs[type-1] = obj;
}

/*
 * Remove an object from its parent and free it.  This does not do
 * anything with the object's FilePos list.
 */
void
ringRemObject(RingPool *pool, union RingObject *obj)
{
	union RingObject **objp;

	pgpAssert(!OBJISTOP(obj));
	objp = &obj->g.up->g.down;

	/* Unlink the object from its parent */
	while (*objp != obj) {
		pgpAssert(*objp);
		objp = &(*objp)->g.next;
	}
	*objp = obj->g.next;
	ringFreeObject(pool, obj);
}

/*
 * Rebuild the pool's hash table from scratch,
 * inserting all keys and subkeys.
 */
void
ringPoolHash(RingPool *pool)
{
	union RingObject *key, *subkey;
	int i;

	for (i = 0; i < 256; i++)
		pool->hashtable[i] = NULL;

	for (key = pool->keys; key; key = key->g.next) {
		pgpAssert(OBJISKEY(key));
		RINGPOOLHASHKEY(pool, key);
		for (subkey = key->g.down; subkey; subkey = subkey->g.next) {
			if (OBJISKEY(subkey))
				RINGPOOLHASHKEY(pool, subkey);
		}
	}
}

/*
 * Find a key given a keyID.
 *
 * ViaCrypt added pkalgs 2 and 3 which are limited RSA, but doesn't
 * completely distinguish beterrn them, so this doesn't either.  Sigh.
 */
union RingObject *
ringPoolFindKey(RingPool const *pool, PGPByte pkalg, PGPByte const keyID[8])
{
	RingKey *key;

	if ((pkalg | 1) == 3)
		pkalg = 1;
	for (key = pool->hashtable[keyID[0]]; key; key = key->util) {
		if (memcmp(keyID, key->keyID, 8) == 0) {
			if (pkalg == key->pkalg)
				break;
			/* Cope with ViaCrypt's things */
			if (pkalg == 1 && (key->pkalg | 1) == 3)
				break;
		}
	}

	return (union RingObject *)key;
}


/*
 * Ensure that each key's list of the signatures by it is
 * valid.  This also establishes the extra invariant (used in
 * pgpRngMnt.c) that all signatures by one key on another object
 * are adjacent on that key's sigsby list.
 */
void
ringPoolListSigsBy(RingPool *pool)
{
	union RingObject *key, *n, *s;

	/* Initialize sigsby lists to null */
	for (key = pool->keys; key; key = key->g.next) {
		pgpAssert(OBJISKEY(key));
		key->k.sigsby = NULL;
	}

	/* Install every sig on a sigsby list */
	for (key = pool->keys; key; key = key->g.next) {
	    for (n = key->k.down; n; n = n->g.next) {
	        if (OBJISSIG(n)) {
		    n->s.nextby = (RingSig *) n->s.by->k.sigsby;
		    n->s.by->k.sigsby = n;
		} else for (s = n->g.down; s; s = s->g.next) {
		    if (OBJISSIG(s)) {
		        s->s.nextby = (RingSig *) s->s.by->k.sigsby;
			s->s.by->k.sigsby = s;
		    }
		}
	    }
	}
}


/*
 * Return the mask of RingFiles that are "better" (higher priority
 * for fetching) than *any* home of the specified object.
 */
static ringmask
ringObjBetters(union RingObject const *obj, RingPool const *pool)
{
	ringmask better = pool->filemask;
	ringmask mask = obj->g.mask & pool->filemask;
	int bit;

	pgpAssert(mask);

	do {
		bit = ringLsBitFind(mask);
		better &= pool->files[bit].higherpri;
	} while (mask &= mask-1);

	return better;
}


/*
 * Find the best Secret which is a descendant of the given key,
 * in the given set.
 */
union RingObject *
ringBestSec(RingSet const *set, union RingObject const *key)
{
	ringmask mask = set->mask;
	ringmask better = (ringmask)~(ringmask)0;
	union RingObject *obj, *best = 0;

	pgpAssert(OBJISKEY(key));
	for (obj = key->g.down; obj; obj = obj->g.next) {
		if (obj->g.mask & better
		    && obj->g.mask & mask
		    && OBJISSEC(obj)) {
			best = obj;
			better = ringObjBetters(obj, set->pool);
		}
	}
	return best;

}

/*
 * Return TRUE if the specified subkey has a valid sig from the main key.
 * Assumes subkey sigs are always tried, which should happen when they are
 * created or added to the keyring.  The only time this isn't true is when
 * we are considering adding a key.  We will give the sig the benefit of
 * the doubt in that case as we aren't using it yet.
 */
int
ringSubkeyValid(RingSet const *set, union RingObject *subkey)
{
	union RingObject *sig;
	union RingObject *key;
	PGPUInt32		  curtime;
	PGPUInt32		  exptime;

	pgpAssert(OBJISSUBKEY(subkey));
	pgpAssert(subkey->g.mask & set->mask);
	key = subkey->g.up;
	pgpAssert(OBJISTOPKEY(key));

	if (subkey->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
	    return 0;
	/* Don't use key if has expired or creation time is > 24 hours in future */
	curtime = (PGPUInt32) PGPGetTime();
	exptime = (PGPUInt32) ringKeyExpiration(set, subkey);
	if ((exptime != 0  &&  curtime > exptime)  ||
		curtime < ringKeyCreation(set, subkey) - 24*60*60)
		return 0;
	/* Check legality of subkey */
	for (sig = subkey->g.down; sig; sig = sig->g.next) {
		if (OBJISSIG(sig) && (sig->g.mask & set->mask) &&
		    ringSigMaker(set, sig, set)==key &&
		    ringSigType(set, sig) == PGP_SIGTYPE_KEY_SUBKEY) {
			if (!ringSigTried(set, sig))
				return 1; /* could check it here... */
			if (ringSigChecked(set, sig))
				return 1;
		}
	}
	return 0;
}

void
ringPurgeCachedName(RingName *name, ringmask mask)
{
	pgpAssert(NAMEISNAME(name));

	if (NAMEISCACHED(name) && (mask >> (name->flags & NAMEF_FILEMASK)) & 1)
	{
		/* Replace buffer with a hash of it */
		name->name.hash = ringHashBuf((PGPByte const *)name->name.ptr,
					      name->len);
		NAMECLEARCACHED(name);
	}
}

/*
 * This function is called by the MemPool code when it runs out of memory.
 * We try to free up more memory by purging the uids from cache.
 * Returns zero if it was unable to make more memory available;
 * non-zero if it might be useful to retry an allocation.
 */
static int
ringPurgeUidCache(void *arg)
{
	RingPool *pool = (RingPool *)arg;
	union RingObject *k, *n;
	int i;

	/*
	 * Quick check to see if we can do anything.  As memory gets
	 * full, the full walk needed to clear the cache gets expensive,
	 * so avoid it unless it does some good.
	 */
	i = 0;
	while (memPoolIsEmpty(&pool->files[i].strings)) {
		if (++i == MEMRINGBIT) /* Last resort: try garbage collect */
			return ringGarbageCollect(pool);
	}

	/*
	 * Okay, we have something cached to free; replace all the
	 * pointers to non-MEMRINGBIT cached named with hashes
	 * of the names and then deallocate the names.
	 */
	for (k = pool->keys; k; k = k->g.next) {
		pgpAssert(OBJISKEY(k));
		for (n = k->g.down; n; n = n->g.next) {
			if (OBJISNAME(n))
				ringPurgeCachedName(&n->n,
				                    (ringmask)~MEMRINGMASK);
		}
	}

	/* Free the pools */
	for (i = 0; i < MEMRINGBIT; i++)
		memPoolEmpty(&pool->files[i].strings);

	return 1;	/* We freed some memory */
}

/* 
 * Helper function for ringPoolInit.
 */
static void
ringFileInit(
	RingPool *pool, RingFile *file)
{
	pgpAssert( pgpContextIsValid( pool->context ) );
	
	file->set.pool = pool;
	file->set.next = NULL;
	file->set.mask = 0;
	file->set.type = RINGSET_FILE;

	file->f = NULL;
	file->destructor = NULL;
	file->arg = NULL;
	memPoolInit( pool->context, &file->strings);
	memPoolInit( pool->context, &file->troublepool);
	memPoolSetPurge(&file->troublepool, ringPurgeUidCache, (void *)pool);
	file->trouble = NULL;
	file->troubletail = &file->trouble;
	memPoolInit( pool->context, &file->fpos);
	memPoolSetPurge(&file->fpos, ringPurgeUidCache, (void *)pool);
	file->freepos = NULL;
	file->higherpri = 0;
	file->flags = 0;
}

/*
 * Initialize a newly allocated RingPool.
 */
void
ringPoolInit(
	PGPContextRef	context,
	RingPool *		pool,
	PGPEnv const *	env)
{
	int i;

	pgpAssert( pgpContextIsValid( context ) );
	
	pool->context	= context;
	
	memPoolInit( context, &pool->structs);
	memPoolSetPurge(&pool->structs, ringPurgeUidCache, (void *)pool);
	pool->keys = NULL;

	for (i = 0; i < RINGTYPE_MAX; i++)
		pool->freeobjs[i] = NULL;
	pool->sets = NULL;
	pool->freesets = NULL;
	pool->freeiter = NULL;

	pool->pktbuf = NULL;
	pool->pktbuflen = 0;
	pool->pktbufalloc = 0;

	/* Reserve last keyring for memory */
	pool->allocmask = MEMRINGMASK;
	pool->filemask = MEMRINGMASK;

	pool->flags = 0;

	if (env) {
		i = pgpenvGetInt(env, PGPENV_CERTDEPTH, NULL, NULL);
		pool->certdepth = i;

		/* Values used for TRUSTMODEL 0 */
		i = pgpenvGetInt(env, PGPENV_MARGINALS, NULL, NULL);
		pool->num_marginals = (i < 0) ? 0 : (i > 255) ? 255 : i;
		i = pgpenvGetInt(env, PGPENV_COMPLETES, NULL, NULL);
		pool->num_completes = (i < 0) ? 0 : (i > 255) ? 255 : i;

		/* Values used for TRUSTMODEL 1 */
		i = pgpenvGetInt(env, PGPENV_TRUSTED, NULL, NULL);
		pool->threshold = (i > PGP_NEWTRUST_INFINITE) ?
			PGP_NEWTRUST_INFINITE : (i < 0) ? 0 : i;

		i = pgpenvGetInt(env, PGPENV_MARGINALS, NULL, NULL);
		i = (i < 1) ? 0 : (pool->threshold+i-1)/i;
		pool->marginalconfidence = i;
	
		i = pgpenvGetInt(env, PGPENV_COMPLETES, NULL, NULL);
		i = (i < 1) ? 0 : (pool->threshold+i-1)/i;
		pool->completeconfidence = i;
	} else {
		pool->certdepth = 4;
		pool->num_marginals = 2;
		pool->num_completes = 1;
		pool->threshold = 3*PGP_TRUST_DECADE_INTERNAL;
		pool->marginalconfidence = 3*PGP_TRUST_DECADE_INTERNAL/2;
		pool->completeconfidence = 3*PGP_TRUST_DECADE_INTERNAL;
	}
#if PGPTRUSTMODEL==2
	memPoolInit (&pool->pathpool);
	pool->paths = NULL;
	pool->pathlists = NULL;
#endif
	
	ringPoolClearError(pool);

	for (i = 0; i < 256; i++)
		pool->hashtable[i] = NULL;

	for (i = 0; i < RINGMASKBITS; i++) {
		ringFileInit(pool, &pool->files[i]);
		pool->files[i].set.mask = (ringmask)1 << i;
	}

	/* Also purge strings cache if needed to create a new object. */
	memPoolSetPurge(&pool->files[MEMRINGBIT].strings,
	                ringPurgeUidCache, (void *)pool);
}

/*
 * Deallocate everything in sight on a RingPool preparatory to
 * deallocating it.
 */
void
ringPoolFini(RingPool *pool)
{
	RingFile *file;
	int bit;
	
	/*
	 * Do this first part, until the destructors are called,
	 * "properly" so structures aren't dangling undefined.
	 */
	for (bit = 0; bit <= MEMRINGBIT; bit++)
		ringFilePurgeTrouble(&pool->files[bit]);

	for (bit = 0; bit <= MEMRINGBIT; bit++) {
		file = &pool->files[bit];
		if (file->destructor) {
			file->destructor(file, file->f, file->arg);
			file->destructor = NULL;
		}
	}

	memPoolEmpty(&pool->structs);
#if PGPTRUSTMODEL==2
	memPoolEmpty (&pool->pathpool);
#endif

	for (bit = 0; bit <= MEMRINGBIT; bit++) {
		file = &pool->files[bit];
		memPoolEmpty(&file->strings);
		memPoolEmpty(&file->fpos);
	}

	if( IsntNull( pool->pktbuf ) )
		pgpContextMemFree( pool->context, pool->pktbuf);

	/* Nuke the lot */
	pgpClearMemory( pool,  sizeof(*pool));
}

/*
 * This is defined as a macro.
 *
 * void
 * ringFileMarkDirty(RingFile *file)
 * {
 *	file->flags |= RINGFILEF_DIRTY;
 * }
 */

/*
 * Mark every file under a given mask as dirty.
 */
void
ringPoolMarkDirty(RingPool *pool, ringmask mask)
{
	mask &= pool->filemask;
	while (mask) {
		ringFileMarkDirty(pool->files + ringLsBitFind(mask));
		mask &= mask-1;
	}
}

void
ringPoolMarkTrustChanged(RingPool *pool, ringmask mask)
{
	mask &= pool->filemask;
	while (mask) {
		pool->files[ringLsBitFind(mask)].flags |=
		                                       RINGFILEF_TRUSTCHANGED;
		mask &= mask-1;
	}
}


/*
 * Do a fingerprint20 (SHA-1) hash on the specified buffer, which
 * should be key data.  We prefix it with the type and length bytes
 * for compatibility with key signature hashes (once they become SHA
 * based).  Return the number of bytes in the hash, or negative on
 * error.
 */
int
pgpFingerprint20HashBuf(PGPContextRef context, PGPByte const *buf, size_t len,
						PGPByte *hash)
{
	PGPHashVTBL const *h;
	PGPHashContext *hc;
	PGPByte tmpbuf[3];
	PGPByte const *p;

	h = pgpHashByNumber (kPGPHashAlgorithm_SHA);
	if (!h)
		return kPGPError_BadHashNumber;
	hc = pgpHashCreate( context, h);
	if (!hc)
		return kPGPError_OutOfMemory;
	/* We use this format even for subkeys */
	tmpbuf[0] = PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1);
	tmpbuf[1] = (PGPByte)(len>>8);
	tmpbuf[2] = (PGPByte)len;
	PGPContinueHash(hc, tmpbuf, 3);
	PGPContinueHash(hc, buf, len);
	p = (PGPByte *) pgpHashFinal(hc);
	memcpy(hash, p, h->hashsize);
	PGPFreeHashContext(hc);
	return h->hashsize;
}

/* Call this for sigs known to use a regexp, to return the regexp.  Loads
 * from disk if necessary.  Returns NULL on error.
 */
	void *
ringSigRegexp( RingSet const *set, RingObject *sig )
{
	PGPByte const *buf;
	PGPSize len;
	regexp *rexp;
	char const *packet;

	pgpAssert( OBJISSIG( sig ) );
	pgpAssert( set->mask & sig->g.mask );
	pgpAssert( SIGUSESREGEXP( &sig->s ) );

	if( !sig->s.regexp ) {
		/* Here we must load the regexp */

		buf = (PGPByte const *)ringFetchObject(set, sig, &len);
		if( !buf )
			return NULL;
		packet = (char *)ringSigFindSubpacket(buf, SIGSUB_REGEXP, 0, &len,
											  NULL, NULL, NULL, NULL);
		pgpAssert( packet );
		if (IsPGPError( pgpRegComp( set->pool->context, packet, &rexp ) ) )
			return NULL;
		sig->s.regexp = (void *) rexp;
	}
	return sig->s.regexp;
}

	
/*
 * Return true if sig is a valid revocation signature.  It must either be
 * a self sig, or it must be by a revocation key.  May be on either a subkey
 * or main key.  This does not check for expirations.
 */
	PGPBoolean
sigRevokesKey (RingSet const *set, RingObject *sig)
{
	RingObject 			*parent;
	RingObject 			*top;

	pgpAssert (OBJISSIG (sig));
	pgpAssert (set->mask & sig->g.mask);
	
	parent = sig->g.up;
	if (!OBJISKEY(parent))
		return FALSE;
	top = parent;
	while (!OBJISTOPKEY(top))
		top = top->g.up;

	if (sig->s.type != ( (top==parent) ? PGP_SIGTYPE_KEY_COMPROMISE
						 			   : PGP_SIGTYPE_KEY_SUBKEY_REVOKE ))
		return FALSE;
	if ((sig->s.trust & PGP_SIGTRUSTF_TRIED) &&
		!(sig->s.trust & PGP_SIGTRUSTF_CHECKED))
		return FALSE;
	/*
	 * If untried, don't accept on a subkey.  Accept on a top-level key
	 * if already shown as revoked because we don't store trust packets
	 * on such keys, so as not to break PGP 2.X.
	 */
	if (!(sig->s.trust & PGP_SIGTRUSTF_TRIED) &&
		((top != parent) ||
		 !(parent->k.trust & PGP_KEYTRUSTF_REVOKED)))
		return FALSE;

	if (sig->s.by == top)
		return TRUE;

	/*
	 * Here we have a revocation signature which is valid but is by some
	 * other key.  We will accept it only if that is a key which is marked
	 * as a revocation authorization key by this one.
	 */

	return ringKeyIsRevocationKey (top, set, sig->s.by);
}
