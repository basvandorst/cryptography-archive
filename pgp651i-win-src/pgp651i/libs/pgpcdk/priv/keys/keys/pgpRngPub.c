/*
 * pgpRngPub.c - keyring management public functions.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngPub.c,v 1.83.2.1 1999/06/04 00:28:54 heller Exp $
 */
#include "pgpConfig.h"
#include <string.h>

#include <ctype.h>	/* For tolower() */

#include "pgpDebug.h"
#include "pgpRngPriv.h"
#include "pgpRngPars.h"
#include "pgpTrstPkt.h"
#include "pgpTrust.h"
#include "pgpRngMnt.h"
#include "pgpErrors.h"
#include "pgpPubKey.h"
#include "pgpRngPub.h"
#include "pgpRngRead.h"
#include "pgpSigSpec.h"
#include "pgpMem.h"
#include "pgpContext.h"
#include "pgpEnv.h"
#include "pgpKeyIDPriv.h"
#include "pgpX509Priv.h"

#ifndef NULL
#define NULL 0
#endif

/*
 * The four type bits are encoded as described in pgpRngPriv.h.
 */
int
ringObjectType(union RingObject const *obj)
{
	int const types[] = {
		RINGTYPE_UNK, RINGTYPE_SEC, RINGTYPE_KEY, RINGTYPE_KEY,
		RINGTYPE_SIG, RINGTYPE_NAME, RINGTYPE_CRL };
	PGPByte type_index;

	pgpAssert (obj);
	type_index = (obj->g.flags&RINGOBJF_TYPE) >> RINGOBJ_SHIFT;
	pgpAssert (type_index < sizeof(types)/sizeof(types[0]));
	return types[type_index];
}

/*
 * These are intended to track reference counts for swapping
 * pieces of keyring out of memory, but are currently no-ops.
 *
 * They're called in a few places in the code as placeholders, but
 * that's just for documentation purposes.
 */
void
ringObjectHold(union RingObject *obj)
{
	(void)obj;
}

void
ringObjectRelease(union RingObject *obj)
{
	(void)obj;
}

RingPool *
ringSetPool(RingSet const *set)
{
	if (!set)
		return NULL;
	return set->pool;
}

/*
 * Return errors in all sorts of cases.
 */
RingError const *
ringPoolError(RingPool const *pool)
{
	pgpAssert (pool);
	return &pool->e;
}

void
ringPoolClearError(RingPool *pool)
{
	if (pool) {
		pool->e.f = (RingFile *)NULL;
		pool->e.fpos = (PGPUInt32)-1;
		pool->e.error = kPGPError_NoErr;
		pool->e.syserrno = 0;
	}
}

RingError const *
ringSetError(RingSet const *set)
{
	pgpAssert (set);
	return &set->pool->e;
}

void
ringSetClearError(RingSet *set)
{
	if (set)
		ringPoolClearError(set->pool);
}

RingError const *
ringIterError(RingIterator const *iter)
{
	pgpAssert (iter);
	return &iter->set.pool->e;
}

void
ringIterClearError(RingIterator *iter)
{
	if (iter)
		ringPoolClearError(iter->set.pool);
}

RingError const *
ringFileError(RingFile const *f)
{
	pgpAssert (f);
	return &f->set.pool->e;
}

void
ringFileClearError(RingFile *f)
{
	if (f)
		ringPoolClearError(f->set.pool);
}

PGPError
ringFileSwitchFile(RingFile *file, PGPFile *newPGPFile)
{
	pgpAssertAddrValid(file, RingFile);
	file->f = newPGPFile;
	return kPGPError_NoErr;
}

/*
 * Is the object a member of the set?
 * Returns the level of the object, or 0 if it is not.
 */
int
ringSetIsMember(RingSet const *set, union RingObject const *obj)
{
	int level = 1;

	while (pgpIsRingSetMember(set, obj)) {
		if (OBJISTOP(obj))
			return level;
		obj = obj->g.up;
		level++;
	}
	return 0;	/* Not a member of the iterator */
}

RingSet const *
ringIterSet(RingIterator const *iter)
{
	if (!iter)
		return NULL;
	return &iter->set;
}

RingSet const *
ringFileSet(RingFile const *file)
{
	if (!file)
		return NULL;
	return &file->set;
}

PgpVersion
ringFileVersion(RingFile const *file)
{
        if (!file)
	        return 0;
	return file->version;
}

/*
 * An iterator involves a current position which is a stack, with an
 * accessible stack-depth value.  The stack depth equals the level
 * of the last ringIterNextObject call, and the stack entries
 * are the return values.  If a query is made for a level which
 * is greater than the stack depth, the first entry on the list
 * of descendants of the top entry on the stack is returned.
 * Only the highest-level entry may be NULL; that indicates that the
 * end of the list there has been reached.  It is illegal to
 * ask for descendants of a NULL entry; although returning NULL
 * is another reasonable option, the usefulness is unclear and the
 * stricter rule has the advantage of catching bugs faster.
 */

/*
 * Find the next object of the given level in the given iterator.
 * Returns <0 on error, 0 if there is no object, or the level if
 * there is one.
 */
int
ringIterNextObject(RingIterator *iter, unsigned level)
{
	union RingObject *obj;

	pgpAssert(iter);
	pgpAssert(level);
	pgpAssert(iter->set.type == RINGSET_ITERATOR);

	/* Get the head of the list to search */
	if (level <= iter->level) {
		/* Going along an existing level */
		iter->level = level;
		obj = iter->stack[level-1];
		if (!obj)
			return 0;
		pgpAssert (pgpIsRingSetMember (&iter->set, obj));
		obj = obj->g.next;
	} else {
		/* Going down a level */
		pgpAssert(level == iter->level+1);

		if (level > 1) {
			obj = iter->stack[level-2];
			pgpAssert(obj);
			pgpAssert (pgpIsRingSetMember (&iter->set, obj));
			if (OBJISBOT(obj))
				return 0;
			obj = obj->g.down;
		} else {
			obj = iter->set.pool->keys;
		}
		iter->level = level;
	}

	/* Search for the next item of interest */
	while (obj && !pgpIsRingSetMember (&iter->set, obj))
		obj = obj->g.next;

	pgpAssert(level <= RINGMAXDEPTH);
	iter->stack[level-1] = obj;
	return obj ? level : 0;
}


/*
 * More complex because we need to find the head of the enclosing list and
 * search forwards for the last matching object that's not the target object.
 */
int
ringIterPrevObject(RingIterator *iter, unsigned level)
{
	union RingObject *obj, *found, *target;

	pgpAssert(iter);
	pgpAssert(level);
	pgpAssert(iter->set.type == RINGSET_ITERATOR);

	/* There's nothing before the beginning of a list */
	if (level > iter->level) {
		pgpAssert(level == iter->level+1);
		return 0;
	}

	/* The thing we want the predecessor of */
	target = iter->stack[level-1];

	/* The head of the list to search along */
	if (level > 1) {
		obj = iter->stack[level-2];
		pgpAssert(obj);
		pgpAssert (pgpIsRingSetMember (&iter->set, obj));
		obj = obj->g.down;
		/* obj = iter->stack[level-2]->g.down; */
	} else {
		obj = iter->set.pool->keys;
	}

	/*
	 * Search forward along the list until we hit the current
	 * object, kepping track of the last object in the desired
	 * ringSet.
	 */
	found = NULL;

	while (obj != target) {
		pgpAssert(obj);
		if (pgpIsRingSetMember (&iter->set, obj))
			found = obj;
		obj = obj->g.next;
	}

	if (!found) {
		/* Hit beginning of list, set up as beginning */
		iter->level = level-1;
		return 0;
	}
	iter->stack[level-1] = found;
	if (OBJISBOT(found)) {
		/* Found an object, but no children. */
		return iter->level = level;
	} else {
		/* An object with children - set up that list at end */
		pgpAssert(level <= RINGMAXDEPTH);
		iter->stack[level] = NULL;
		iter->level = level+1;
	}
	return (int)level;
}

/* The level of the most recent ringIterNextObject() call */
unsigned
ringIterCurrentLevel(RingIterator const *iter)
{
	return iter->level;
}

/*
 * A trivial little function that just returns the current object
 * at a given level, again.
 */
union RingObject *
ringIterCurrentObject(RingIterator const *iter, unsigned level)
{
	pgpAssert(iter);
	pgpAssert(level);
	pgpAssert(iter->set.type == RINGSET_ITERATOR);

	return level > iter->level ? NULL : iter->stack[level-1];
}

/*
 * Seek to the next object at the deepest level possible.
 *
 * Equivalent to:
 * int i;
 * unsigned l = ringIterCurrentLevel(iter)+1;
 *
 * while (l && !(i = ringIterNextObject(iter, l)))
 *	--l;
 * return i;
 */
int
ringIterNextObjectAnywhere(RingIterator *iter)
{
	union RingObject *obj;
	unsigned level = iter->level;

	pgpAssert(iter);
	pgpAssert(iter->set.type == RINGSET_ITERATOR);

	/* Find first object to be considered */
	if (!level) {
		level = 1;
		obj = iter->set.pool->keys;
	} else {
		obj = iter->stack[level-1];

		if (obj) {
			pgpAssert (pgpIsRingSetMember (&iter->set, obj));
			if (OBJISBOT(obj)) {
				obj = obj->g.next;
			} else {
				level++;
				obj = obj->g.down;
			}
		}
	}

	for (;;) {
		while (obj) {
			if (pgpIsRingSetMember(&iter->set, obj)) {
				iter->stack[level-1] = obj;
				iter->level = level;
				return (int)level;
			}
			obj = obj->g.next;
		}
		if (!--level)
			break;
		obj = iter->stack[level-1];
		pgpAssert(obj);
		obj = obj->g.next;
	}

	/* End of list, no luck */
	iter->stack[0] = NULL;
	iter->level = 1;
	return 0;
}

/* Reset the iterator to the beginning of the given level */
int
ringIterRewind(RingIterator *iter, unsigned level)
{
	pgpAssert(level);
	pgpAssert(iter->level >= level - 1);
	iter->level = level-1;
	return 0;
}

/* Reset the iterator to the end of the given level */
int
ringIterFastForward(RingIterator *iter, unsigned level)
{
	pgpAssert(level);
	pgpAssert(level <= iter->level + 1);
	if (level > RINGMAXDEPTH)
		level = RINGMAXDEPTH;
	else
		iter->stack[level-1] = NULL;
	iter->level = level;
	return 0;
}

/*
 * Seek the iterator to the given object, state as if it just
 * returned the object.	 Returns the level of the object, or <0
 * on error.
 */
int
ringIterSeekTo(RingIterator *iter, union RingObject *obj)
{
	union RingObject *p, *pp;
	int level;

	pgpAssert(iter->set.type == RINGSET_ITERATOR);

	if (!pgpIsRingSetMember (&iter->set, obj))
		return 0;	/* Not a member */

	/* A bit ad-hoc; there is a general way. */
	if (OBJISTOP(obj)) {
		iter->stack[0] = obj;
		level = 1;
	} else {
		p = obj->g.up;
		pgpAssert(pgpIsRingSetMember(&iter->set, p));
		if (OBJISTOP(p)) {
			iter->stack[0] = p;
			iter->stack[1] = obj;
			level = 2;
		} else {
			pp = p->g.up;
			pgpAssert(pgpIsRingSetMember(&iter->set, pp));
			pgpAssert(OBJISTOP(pp));
			iter->stack[0] = pp;
			iter->stack[1] = p;
			iter->stack[2] = obj;
			level = 3;
		}
	}
	return iter->level = level;
}

static void
ringSetCountList(union RingObject const *obj, PGPVirtMask const *mask,
                 unsigned *counts, unsigned depth)
{
	while (obj) {
		if (pgpVirtMaskIsOverlapping (&obj->g.mask, mask)) {
			counts[0]++;
			if (depth && !OBJISBOT(obj)) {
				ringSetCountList(obj->g.down, mask,
				                 counts+1, depth-1);
			}
		}
		obj = obj->g.next;
	}
}

/*
 * Count the number of objects in an iterator down to a given depth.
 */
int
ringSetCount(RingSet const *set, unsigned *counts, unsigned depth)
{
	unsigned i = 0;

	for (i = 0; i < depth; i++)
		counts[i] = 0;
	if (IsntNull(set) && !pgpVirtMaskIsEmpty(&set->mask) && depth > 0)
		ringSetCountList(set->pool->keys, &set->mask, counts, depth-1);
	return 0;
}

static void
ringSetCountTypesList(union RingObject const *obj, PGPVirtMask const *mask,
                      unsigned *counts, unsigned max)
{
	int t;

	while (obj) {
		if (pgpVirtMaskIsOverlapping (mask, &obj->g.mask)) {
			t = ringObjectType(obj);
			if ((unsigned)t <= max)
				counts[t-1]++;
			if (!OBJISBOT(obj))
				ringSetCountTypesList(obj->g.down, mask,
				                 counts, max);
		}
		obj = obj->g.next;
	}
}

/*
 * Count the number of objects in an iterator of various types.
 */
int
ringSetCountTypes(RingSet const *set, unsigned *counts, unsigned max)
{
	unsigned i = 0;

	for (i = 0; i < max; i++)
		counts[i] = 0;
	if (IsntNull(set) && !pgpVirtMaskIsEmpty(&set->mask) && max > 0)
		ringSetCountTypesList(set->pool->keys, &set->mask, counts, max);
	return 0;
}

RingIterator *
ringIterCreate(RingSet const *set)
{
	RingPool *pool = set->pool;
	RingIterator *iter;

	pgpAssert(!RINGSETISMUTABLE(set));

	/* Allocate the structure */
	iter = pool->freeiter;
	if (iter) {
		pool->freeiter = (RingIterator *)iter->set.next;
		pgpAssert(iter->set.type == RINGSET_FREE);
	} else {
		iter = (RingIterator *)memPoolNew(&pool->structs,
							 RingIterator);
		if (!iter) {
			ringAllocErr(pool);
			return NULL;
		}
	}

	/* Okay, allocated - fill it in */
	iter->set.pool = pool;
	iter->set.next = pool->sets;
	iter->set.type = RINGSET_ITERATOR;
	pool->sets = &iter->set;
	iter->set.mask = set->mask;
	iter->level = 0;	/* Rewind to beginning */
	return iter;
}

void
ringIterDestroy(RingIterator *iter)
{
	RingPool *pool;
	RingSet **setp;

	if (iter) {
		pool = iter->set.pool;

		pgpAssert(iter->set.type == RINGSET_ITERATOR);
		iter->set.type = RINGSET_FREE;

		/* Remove it from the list of allocated sets */
		setp = &pool->sets;
		while (*setp != &iter->set) {
			pgpAssert(*setp);
			setp = &(*setp)->next;
		}
		*setp = iter->set.next;

		/* Add to the list of free iterators. */
		iter->set.next = (RingSet *)pool->freeiter;
		pool->freeiter = iter;
	}
}

static RingSet *
ringSetAlloc(RingPool *pool)
{
	RingSet *set;

	/* Allocate the structure */
	set = pool->freesets;
	if (set) {
		pool->freesets = set->next;
	} else {
		set = (RingSet *)memPoolNew(&pool->structs,
						   RingSet);
		if (!set) {
			ringAllocErr(pool);
			return NULL;
		}
	}

	/* Okay, allocated - fill it in */
	set->pool = pool;
	set->next = pool->sets;
	pool->sets = set;
	pgpVirtMaskInit (pool, &set->mask);
	/* set->type uninitialized */

	return set;
}

RingSet *
ringSetCreate(RingPool *pool)
{
	PGPVirtMask mask;
	RingSet *set;
	unsigned bit;
	PGPError err;

	if (!pool)
		return NULL;

	/* Allocate a new bit */
	err = ringBitAlloc(pool, &bit);
	if (IsPGPError(err)) {
		ringSimpleErr(pool, err);
		return NULL;
	}

	pgpVirtMaskInit (pool, &mask);
	pgpVirtMaskSetBit (pool, &mask, bit);

	/* Allocate the structure */
	set = ringSetAlloc(pool);
	if (set) {
		pgpVirtMaskCopy (pool, &mask, &set->mask);
		set->type = RINGSET_MUTABLE;
		pgpVirtMaskOR (pool, &mask, &pool->allocmask);
	}
	pgpVirtMaskCleanup (pool, &mask);
	return set;
}

/* Create a "universal" set which represents all keys in the pool */
RingSet *
ringSetCreateUniversal(RingPool *pool)
{
	RingSet *set;

	if (!pool)
		return NULL;

	/* Allocate the structure */
	set = ringSetAlloc(pool);
	if (set) {
		pgpVirtMaskCopy (pool, &pool->allocmask, &set->mask);
		set->type = RINGSET_IMMUTABLE;
	}
	return set;
}

void
ringSetDestroy(RingSet *set)
{
	RingPool *pool;
	RingSet **setp;

	if (set) {
		pool = set->pool;
		pgpAssert(set->type < RINGSET_FREE);
		set->type = RINGSET_FREE;

		/* Remove it from the list of allocated sets */
		setp = &pool->sets;
		while (*setp != set) {
			pgpAssert(*setp);
			setp = &(*setp)->next;
		}
		*setp = set->next;

		pgpVirtMaskCleanup (pool, &set->mask);

		/* Add to the list of free sets. */
		set->next = pool->freesets;
		pool->freesets = set;
	}
}

/*
 * Freeze a RingSet so that you can start doing set operations
 * on it, copying it, etc.
 */
	PGPError
ringSetFreeze(RingSet *set)
{
	if (set) {
		if (set->type == RINGSET_MUTABLE)
			set->type = RINGSET_IMMUTABLE;
		pgpAssert(set->type == RINGSET_IMMUTABLE);
	}
	return kPGPError_NoErr;
}

RingSet *
ringSetCopy(RingSet const *s)
{
	RingSet *set;

	if (!s)
		return NULL;

	pgpAssert(!RINGSETISMUTABLE(s));
	set = ringSetAlloc(s->pool);
	if (set) {
		pgpVirtMaskCopy (s->pool, &s->mask, &set->mask);
		set->type = RINGSET_IMMUTABLE;
	}
	return set;
}

/* This accepts NULL as an alias for "no such set" */
RingSet *
ringSetUnion(RingSet const *s1, RingSet const *s2)
{
	RingSet *set;

	if (!s1)
		return ringSetCopy(s2);

	set = ringSetCopy(s1);
	if (set && s2) {
		pgpAssert(s1->pool == s2->pool);
		pgpAssert(!RINGSETISMUTABLE(s2));
		pgpVirtMaskOR (s1->pool, &s2->mask, &set->mask);
	}
	return set;
}


/** The following operations only apply to mutable RingSets **/

/*
 * Add an object to a mutable RingSet.  That includes the all of the
 * object's parents in order to main the proper RingSet invariants.
 */
	int
ringSetAddObject(RingSet *set, union RingObject *obj)
{
	pgpAssert(RINGSETISMUTABLE(set));

	if (IsntNull(obj) && !pgpIsRingSetMember(set, obj)) {
		pgpVirtMaskOR (set->pool, &set->mask, &obj->g.mask);
		/* Ensure all parents are added, too. */
		while (!OBJISTOP(obj)) {
			obj = obj->g.up;
			if (pgpIsRingSetMember(set, obj))
				break;
			pgpVirtMaskOR (set->pool, &set->mask, &obj->g.mask);
		}
	}
	return 0;
}

/*
 * Add an object and its children to a mutable RingSet.  Also will do
 * the object's parents.  src RingSet controls which children are added.
 */
	int
ringSetAddObjectChildren(RingSet *dest, RingSet const *src,
	union RingObject *obj)
{
	RingSet const *srcx;
	RingIterator *iter;
	int level, initlevel;
	int nobjs;
	int err;

	/* Need to iterate over src, make a copy if necessary */
	srcx = src;
	if (RINGSETISMUTABLE(src)) {
		RingSet *src1 = ringSetCreate (ringSetPool (src));
		if (!src1)
			return ringSetError(src)->error;
		ringSetAddSet (src1, src);
		ringSetFreeze (src1);
		src = (RingSet const *)src1;
	}
			
	/* First add the object */
	if ((err= (PGPError)ringSetAddObject(dest, obj)) < 0)
		return err;

	iter = ringIterCreate(src);
	if (!iter)
		return ringSetError(src)->error;

	nobjs = 1;
	initlevel=ringIterSeekTo(iter, obj);
	if (initlevel < 0)
		return initlevel;
	level = initlevel + 1;
	while (level > initlevel) {
		union RingObject *child;
		err = (PGPError)ringIterNextObject(iter, level);
		if (err < 0) {
			ringIterDestroy(iter);
			return err;
		}
		if (err > 0) {
			child = ringIterCurrentObject(iter, level);
			if (!child)
				return ringSetError(src)->error;
			if ((err=ringSetAddObject(dest, child)) < 0)
				return err;
			++nobjs;
			++level;
		} else {
			--level;
		}
	}
	ringIterDestroy(iter);
	/* Destroy set copy if we made one */
	if (src != srcx)
		ringSetDestroy ((RingSet *)src);
	return nobjs;
}

/*
 * Remove an object from a mutable RingSet.  That includes the all of the
 * object's children in order to main the proper RingSet invariants.
 * (Done recursively.)
 */
int
ringSetRemObject(RingSet *set, union RingObject *obj)
{
	pgpAssert(RINGSETISMUTABLE(set));

	/*
	 * Remove this object and all its children.
	 * As an optimization, omit scanning children if the
	 * object is not already in the set.
	 */
	if (IsntNull(obj) && pgpIsRingSetMember (set, obj)) {
		if (!OBJISBOT(obj)) {
			union RingObject *obj2 = obj;
			for (obj2 = obj2->g.down; obj2; obj2 = obj2->g.next)
				ringSetRemObject(set, obj2);
		}
		pgpVirtMaskANDNOT (set->pool, &set->mask, &obj->g.mask);
		ringGarbageCollectObject(set->pool, obj);
	}
	return 0;
}

/* Helper function for ringSetAddSet */
static void
ringSetAddList(union RingObject *obj, RingSet *dest, RingSet const *src)
{
	while (obj) {
		if (pgpIsRingSetMember(src, obj)) {
			pgpVirtMaskOR (dest->pool, &dest->mask, &obj->g.mask);
			if (!OBJISBOT(obj))
				ringSetAddList(obj->g.down, dest, src);
		}
		obj = obj->g.next;
	}
}

int
ringSetAddSet(RingSet *set, RingSet const *set2)
{
	pgpAssert(RINGSETISMUTABLE(set));
	if (set2) {
		pgpAssert(set->pool == set2->pool);

		ringSetAddList(set->pool->keys, set, set2);
	}
	return 0;
}

/*
 * Subtracting sets is simplified by the proper-set requirement.
 * If I remove a key (because it's in set2), I have to remove all
 * children of the key, so I just use ringClearMask to do
 * the job.  If I don't (because it's not in set2), then it's
 * guaranteed that none of its children are in set2 either,
 * so there's no need to examine any children.
 */
int
ringSetSubtractSet(RingSet *set, RingSet const *set2)
{
	union RingObject *obj;
	PGPVirtMask tmask;

	pgpAssert(RINGSETISMUTABLE(set));

	pgpVirtMaskInit (set->pool, &tmask);

	if (IsntNull(set2) && !pgpVirtMaskIsEmpty(&set2->mask)) {
		pgpAssert(set->pool == set2->pool);
		for (obj = set->pool->keys; obj; obj = obj->g.next) {
			if (pgpIsRingSetMember (set, obj)
				&& pgpIsRingSetMember (set2, obj)) {
				pgpVirtMaskANDNOT (set->pool, &set->mask, &obj->g.mask);
				if (!OBJISBOT(obj)) {
					ringClearMask(set->pool, &obj->g.down, NULL,
								  &set->mask, &tmask);
				}
			}
		}
	}
	pgpVirtMaskCleanup (set->pool, &tmask);
	return 0;
}

static int
ringSetIntersectList(union RingObject *obj, RingSet const *s1,
	RingSet const *s2, RingSet const *dest)
{
	int flag = 0;

	while (obj) {
		if (pgpIsRingSetMember(s1,obj) &&
			pgpIsRingSetMember(s2,obj)) {
			flag = 1;
			pgpVirtMaskOR (s1->pool, &dest->mask, &obj->g.mask);
			if (!OBJISBOT(obj))
				ringSetIntersectList(obj->g.down, s1, s2, dest);
		}
		obj = obj->g.next;
	}
	return flag;
}

RingSet *
ringSetIntersection(RingSet const *s1, RingSet const *s2)
{
	RingSet *set;
	PGPVirtMask mask;

	if (!s1 || !s2)
		return (RingSet *)NULL;

	pgpVirtMaskInit (s1->pool, &mask);

	pgpAssert(s1->pool == s2->pool);
	pgpAssert(!RINGSETISMUTABLE(s1));
	pgpAssert(!RINGSETISMUTABLE(s2));

	/* Do a few trivial cases without allocating bits. */
	pgpVirtMaskCopy (s1->pool, &s1->mask, &mask);
	pgpVirtMaskANDNOT (s1->pool, &s2->mask, &mask);
	if (!pgpVirtMaskIsEmpty(&mask)) {	/* s1 is a subset of s2 - copy s1 */
		pgpVirtMaskCleanup (s1->pool, &mask);
		return ringSetCopy(s1);
	}

	pgpVirtMaskCopy (s1->pool, &s2->mask, &mask);
	pgpVirtMaskANDNOT (s1->pool, &s1->mask, &mask);
	if (!pgpVirtMaskIsEmpty(&mask)) {	/* s2 is a subset of s1 - copy s2 */
		pgpVirtMaskCleanup (s1->pool, &mask);
		return ringSetCopy(s2);
	}

	set = ringSetCreate(s1->pool);
	if (set) {
		if (!ringSetIntersectList(s1->pool->keys, s1, s2, set))
		{
			/* Empty set - free bit */
			pgpVirtMaskANDNOT (s1->pool, &set->mask, &s1->pool->allocmask);
			pgpVirtMaskCleanup (s1->pool, &set->mask);
		}
		set->type = RINGSET_IMMUTABLE;
	}
	pgpVirtMaskCleanup (s1->pool, &mask);
	return set;
}

static void
ringSetDiffList(union RingObject *obj, RingSet const *s1, RingSet const *s2,
				RingSet const *dest)
{
	while (obj) {
		if (pgpIsRingSetMember(s1, obj) &&
			!pgpIsRingSetMember(s2, obj)) {
			pgpVirtMaskOR (s1->pool, &dest->mask, &obj->g.mask);
			if (!OBJISBOT(obj))
				ringSetDiffList(obj->g.down, s1, s2, dest);
		}
		obj = obj->g.next;
	}
}

/* Return s1-s2. */
RingSet *
ringSetDifference(RingSet const *s1, RingSet const *s2)
{
	RingSet *set;
	PGPVirtMask mask;

	if (IsNull(s1))
		return NULL;
	if (IsNull(s2) || pgpVirtMaskIsEmpty(&s2->mask))
		return ringSetCopy(s1);

	pgpAssert(s1->pool == s2->pool);
	pgpAssert(!RINGSETISMUTABLE(s1));
	pgpAssert(!RINGSETISMUTABLE(s2));

	pgpVirtMaskInit (s1->pool, &mask);
	pgpVirtMaskCopy (s1->pool, &s1->mask, &mask);
	pgpVirtMaskANDNOT (s1->pool, &s2->mask, &mask);
	if (pgpVirtMaskIsEmpty(&mask)) {
		/* s1->mask is a subset of s2->mask, so result is empty */
		set = ringSetAlloc(s1->pool);
		if (set) {
			pgpVirtMaskCleanup (s1->pool, &set->mask);
			set->type = RINGSET_IMMUTABLE;
		}
	} else {
		set = ringSetCreate(s1->pool);
		if (set) {
			ringSetDiffList(s1->pool->keys, s1, s2, set);
			set->type = RINGSET_IMMUTABLE;
		}
	}
	pgpVirtMaskCleanup (s1->pool, &mask);
	return set;
}

int
ringFileIsDirty(RingFile const *file)
{
	return file ? file->flags & RINGFILEF_DIRTY : 0;
}

int
ringFileIsTrustChanged(RingFile const *file)
{
	return file ? file->flags & RINGFILEF_TRUSTCHANGED : 0;
}

RingPool *
ringPoolCreate(PGPEnv const *env)
{
	RingPool *pool;
	PGPContextRef	context	= pgpenvGetContext( env );

	pool = (RingPool *)pgpContextMemAlloc( context,
		sizeof(*pool), kPGPMemoryMgrFlags_Clear );
	if (pool)
		ringPoolInit( context, pool, env);
	return pool;
}

void
ringPoolDestroy(RingPool *pool)
{
	if (pool) {
		PGPContextRef cdkContext = pool->context;
		ringPoolFini(pool);
		pgpContextMemFree( cdkContext, pool);
	}
}

PGPContextRef
ringPoolContext(RingPool *pool)
{
	pgpAssert( pool );
	return pool->context;
}

union RingObject *
ringKeyById8(
	RingSet const *		set,
	PGPByte				pkalg,
	PGPByte const *		keyID)
{
	RingKey *key;

	if ((pkalg|1) == 3)	/* viacrypt */
		pkalg = 1;
	for (key = set->pool->hashtable[keyID[0]]; key; key = key->util) {
		if ((((key->pkalg|1) == 3) ? 1 : key->pkalg) == pkalg &&
		    memcmp(keyID, key->keyID, 8) == 0) {
			if (!pgpIsRingSetMember(set, (union RingObject *)key))
				break;
			ringObjectHold((union RingObject *)key);
			return (union RingObject *)key;
		}
	}
	/* Failed */
	return NULL;
}

#if 0
union RingObject *
ringKeyById4(
	RingSet const *		set,
	PGPByte				pkalg,
	PGPByte const *		keyID)
{
	RingKey *key;
	RingKey *bestKey = NULL;

	if ((pkalg|1) == 3)	/* viacrypt */
		pkalg = 1;
		
	for (key = set->pool->hashtable[keyID[0]]; key; key = key->util)
	{
		if ((((key->pkalg|1) == 3) ? 1 : key->pkalg) == pkalg &&
		    memcmp(keyID, &key->keyID[4], 4) == 0)
		{
			if (pgpIsRingSetMember(set, (union RingObject *)key))
			{
				if( IsNull( bestKey ) )
				{
					bestKey = key;
				}
				else
				{
					/* Non-unique match. Fail */
					bestKey = NULL;
					break;
				}
			}
		}
	}
	
	if( IsntNull( bestKey ) )
	{
		ringObjectHold((union RingObject *)bestKey);
	}
	return (union RingObject *)bestKey;
}
#else
union RingObject *
ringKeyById4(
	RingSet const *		set,
	PGPByte				pkalg,
	PGPByte const *		keyID)
{
	union RingObject *key, *subkey;
	union RingObject *bestKey = NULL;

	if ((pkalg|1) == 3)	/* viacrypt */
		pkalg = 1;
		
	for (key = set->pool->keys; key; key = key->g.next)
	{
		if ((((key->k.pkalg|1) == 3) ? 1 : key->k.pkalg) == pkalg &&
		    memcmp(keyID, &key->k.keyID[4], 4) == 0)
		{
			if (pgpIsRingSetMember(set, key))
			{
				if( IsNull( bestKey ) )
				{
					bestKey = key;
				}
				else
				{
					/* Non-unique match. Fail */
					bestKey = NULL;
					goto exit;
				}
			}
		}
		
		for (subkey = key->g.down; subkey; subkey = subkey->g.next)
		{
			if ((((subkey->k.pkalg|1) == 3) ? 1 : subkey->k.pkalg) == pkalg &&
			    memcmp(keyID, &subkey->k.keyID[4], 4) == 0)
			{
				if (pgpIsRingSetMember(set, subkey))
				{
					if( IsNull( bestKey ) )
					{
						bestKey = subkey;
					}
					else
					{
						/* Non-unique match. Fail */
						bestKey = NULL;
						goto exit;
					}
				}
			}
		}
	}

exit:

	if( IsntNull( bestKey ) )
	{
		ringObjectHold( bestKey );
	}
	
	return bestKey;
}
#endif

/*** Access functions for information about objects ***/

int
ringKeyError(RingSet const *set, union RingObject *key)
{
	PGPByte const *p;
	PGPSize len;

	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	if (!(key->g.flags & KEYF_ERROR))
		return 0;

	p = (PGPByte const *)ringFetchObject(set, key, &len);
	if (!p)
		return ringSetError(set)->error;
	return ringKeyParse(set->pool->context, p, len, NULL, NULL, NULL, NULL,
						NULL, NULL, NULL, 0);
}

unsigned
ringKeyBits(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
	return key->k.keybits;
}

PGPUInt32
ringKeyCreation(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
	return key->k.tstamp;
}

PGPUInt32
ringKeyExpiration(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
	if (key->k.tstamp == 0 || key->k.validity == 0)
		return 0;    /* valid indefinitely */
	else
		return key->k.tstamp + (key->k.validity * 3600 * 24);
}

/*
 * If called for a subkey, force to just encryption.
 * If called for a key with a subkey, return the "or" of both.
 * Else just do the key itself.
 * Internal form of ringKeyUse - if unExpired is true, check that the
 * subkeys are unexpired before saying it has encryption usage.
 * ringKeyUse and ringKeyUnexpiredUse are macros that call this now.
 */
int
ringKeyUseInternal(RingSet const *set, union RingObject *key,
				   PGPBoolean unExpired)
{
	int use;

	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	use = pgpKeyUse(pgpPkalgByNumber(key->k.pkalg));
	if (OBJISSUBKEY(key))
		use &= PGP_PKUSE_ENCRYPT;
	for (key=key->g.down; key; key=key->g.next) {
		if (pgpIsRingSetMember(set, key) &&
			OBJISSUBKEY(key) && ringSubkeyValid(set, key, unExpired))
			use |= pgpKeyUse(pgpPkalgByNumber(key->k.pkalg));
	}
	return use;
}


PGPByte
ringKeyTrust(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
	if (!(key->g.flags & (RINGOBJF_TRUST)))
		ringMntValidateKey (set, key);	
	return pgpMax(key->k.trust & kPGPKeyTrust_Mask,
				  key->k.signedTrust & kPGPKeyTrust_Mask);
}

void
ringKeySetTrust(RingSet const *set, union RingObject *key, PGPByte trust)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	pgpAssert(trust==kPGPKeyTrust_Unknown  || trust==kPGPKeyTrust_Never ||
	       trust==kPGPKeyTrust_Marginal || trust==kPGPKeyTrust_Complete);
	pgpAssert (!(key->k.trust & PGP_KEYTRUSTF_BUCKSTOP));
	if (key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
	    return;
	if ((key->k.trust & kPGPKeyTrust_Mask) != trust) {
		key->k.trust = (key->k.trust & ~kPGPKeyTrust_Mask) + trust;
		key->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, &key->g.mask);
	}
	key->g.flags |= RINGOBJF_TRUST;
}


/*
 * Used to set a key as an "axiomatic" key, that is, one for which
 * we hold the private key.  This also involves setting each name on that
 * key as having complete validity.
 */
void
ringKeySetAxiomatic(RingSet const *set, union RingObject *key)
{
	union RingObject *name = NULL;

	(void)name;
    pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
    if (!ringKeyIsSec (set, key) || 
	key->k.trust & (PGP_KEYTRUSTF_BUCKSTOP | PGP_KEYTRUSTF_REVOKED))
        return;        /* already axiomatic or can't set */
    key->k.trust &= ~kPGPKeyTrust_Mask;
    key->k.trust |= (PGP_KEYTRUSTF_BUCKSTOP | kPGPKeyTrust_Ultimate);
    ringPoolMarkTrustChanged (set->pool, &key->g.mask);
    key->g.flags |= (RINGOBJF_TRUSTCHANGED | RINGOBJF_TRUST);
#if PGPTRUSTMODEL>0
	/* Make sure all names have axiomatic confidence */
	for (name=key->g.down; name; name=name->g.next) {
		if (OBJISNAME(name) && pgpIsRingSetMember(set, name))
			name->n.confidence = PGP_NEWTRUST_INFINITE;
	}
#endif
}


/*  Reset an axiomatic key.  Trust is set to undefined. */

void
ringKeyResetAxiomatic (RingSet const *set, union RingObject *key)
{
	union RingObject *name = NULL;

	(void)name;
    pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
    if (!(key->k.trust & PGP_KEYTRUSTF_BUCKSTOP))
        return;           /* not axiomatic */
    key->k.trust &= ~PGP_KEYTRUSTF_BUCKSTOP;
    key->k.trust = (key->k.trust & ~kPGPKeyTrust_Mask) + 
                           kPGPKeyTrust_Undefined;
    ringPoolMarkTrustChanged (set->pool, &key->g.mask);
    key->g.flags |= (RINGOBJF_TRUSTCHANGED | RINGOBJF_TRUST);
#if PGPTRUSTMODEL>0
	/* Make sure all names have undefined confidence */
	for (name=key->g.down; name; name=name->g.next) {
		if (OBJISNAME(name) && pgpIsRingSetMember(set, name))
			name->n.confidence = PGP_NEWTRUST_UNDEFINED;
	}
#endif
}

int
ringKeyAxiomatic(RingSet const *set, union RingObject *key)
{
	(void)set;	/* Avoid warning */
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	return key->k.trust & PGP_KEYTRUSTF_BUCKSTOP;
}


/* Return TRUE if the key is a subkey */
int
ringKeyIsSubkey (RingSet const *set, union RingObject const *key)
{
     (void)set;
     return OBJISSUBKEY(key);
}


int
ringKeyDisabled(RingSet const *set, union RingObject *key)
{
	(void)set;	/* Avoid warning */
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	return key->k.trust & PGP_KEYTRUSTF_DISABLED;
}

void
ringKeyDisable(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	if (!(key->k.trust & PGP_KEYTRUSTF_DISABLED)) {
		key->k.trust |= PGP_KEYTRUSTF_DISABLED;
		key->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, &key->g.mask);
	}
	key->g.flags |= RINGOBJF_TRUST;
}

void
ringKeyEnable(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	if (key->k.trust & PGP_KEYTRUSTF_DISABLED) {
		key->k.trust &= ~PGP_KEYTRUSTF_DISABLED;
		key->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, &key->g.mask);
	}
	key->g.flags |= RINGOBJF_TRUST;
}



int
ringKeyRevoked(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
#if PGPTRUSTMODEL>0
	if (!(key->g.flags & (RINGOBJF_TRUST)))
		ringMntValidateKey (set, key);
#endif
	return key->k.trust & PGP_KEYTRUSTF_REVOKED;
}


/* 
 * Return true if the specified signature has been revoked, that is,
 * if there is a newer signature by the same key which is of type
 * UID_REVOKE, or if it has been revoked by CRL.
 */
int
ringSigRevoked (RingSet const *set, union RingObject *sig)
{
	RingObject		*parent,
					*sibling;

	(void)set;	/* Avoid warning */
	pgpAssert (OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));

	/* Sigs can be declared irrevocable at creation time */
	if (!SIGISREVOCABLE(&sig->s))
		return FALSE;

	/* Check revoked flag */
	if (sig->s.trust & PGP_SIGTRUSTF_REVOKEDBYCRL)
		return TRUE;

	parent = sig->g.up;
	for (sibling = parent->g.down; sibling ; sibling = sibling->g.next) {
		if (!OBJISSIG(sibling) || sibling == sig)
			continue;
		if (sibling->s.by == sig->s.by) {
			if (sibling->s.trust & PGP_SIGTRUSTF_CHECKED) {
				if (sibling->s.tstamp > sig->s.tstamp) {
					if (sibling->s.type == PGP_SIGTYPE_KEY_UID_REVOKE) {
						/* Valid revocation */
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}


PgpTrustModel
pgpTrustModel (RingPool const *pool)
{
	(void)pool;
#if PGPTRUSTMODEL==0
	return PGPTRUST0;
#endif
#if PGPTRUSTMODEL==1
	return PGPTRUST1;
#endif
#if PGPTRUSTMODEL==2
	return PGPTRUST2;
#endif
}

PGPUInt16
ringKeyConfidence(RingSet const *set, union RingObject *key)
{
#if PGPTRUSTMODEL==0
	(void)set;
	(void)key;
	pgpAssert(0);
	return 0;
#else
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;

	/* Don't try on-the-fly validation, assume it's been done */
	/* if (!(key->g.flags & (RINGOBJF_TRUST))) */
	/* 	ringMntValidateKey (set, key); */

	/*  ringKeyCalcTrust handles revoked/expired keys */
	return ringKeyCalcTrust (set, key);
#endif
}

void
ringKeyID8(
	RingSet const *set,
	union RingObject const *key,
	PGPByte *pkalg,
	PGPKeyID *keyID)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
	if (pkalg) {
		*pkalg = key->k.pkalg;
		if ((*pkalg | 1) == 3)		/* ViaCrypt */
			*pkalg = 1;
	}
	if (keyID)
	{
		pgpNewKeyIDFromRawData( key->k.keyID, 8, keyID );
	}
}

void
ringKeyID4(
	RingSet const *set,
	union RingObject const *key,
	PGPByte *pkalg,
	PGPKeyID *keyID)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
	if (pkalg) {
		*pkalg = key->k.pkalg;
		if ((*pkalg | 1) == 3)		/* ViaCrypt */
			*pkalg = 1;
	}
	if (keyID)
	{
		pgpNewKeyIDFromRawData( &key->k.keyID[4], 4, keyID );
	}
}

PGPBoolean
ringKeyV3(RingSet const *set, union RingObject const *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	(void)set;
	return !!KEYISV3(&key->k);
}

int
ringKeyFingerprint16(RingSet const *set, union  RingObject *key,
	PGPByte *buf)
{
	PGPByte const *p;
	PGPSize len;

	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	p = (PGPByte const *)ringFetchObject(set, key, &len);
	if (!p)
		return ringSetError(set)->error;
	return ringKeyParseFingerprint16(set->pool->context, p, len, buf);
}

int
ringKeyFingerprint20(RingSet const *set, union RingObject *key,
	PGPByte *buf)
{
	PGPSize objlen;
	PGPByte const *objbuf;

	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	objbuf = (PGPByte const *)ringFetchObject(set, key, &objlen);
	return pgpFingerprint20HashBuf(set->pool->context, objbuf, objlen, buf);
}

/* This does a 20 byte fingerprint based solely on the numeric material */
int
ringKeyFingerprint20n(RingSet const *set, union RingObject *key,
	PGPByte *buf)
{
	PGPSize objlen;
	unsigned int numlen;
	PGPByte const *objbuf, *numbuf;

	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	objbuf = (PGPByte const *)ringFetchObject(set, key, &objlen);
	numbuf = ringKeyParseNumericData(objbuf, objlen, &numlen);
	return pgpFingerprint20HashBuf(set->pool->context, numbuf, numlen, buf);
}

int
ringKeyAddSigsby(RingSet const *set, union RingObject *key,
	RingSet *dest)
{
	RingSig *sig;
	int i = 0;

	pgpAssert(OBJISTOPKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));
	pgpAssert(RINGSETISMUTABLE(dest));

	for (sig = &key->k.sigsby->s; sig; sig = sig->nextby) {
		if (pgpIsRingSetMember(set, (union RingObject *)sig)) {
			i++;
			ringSetAddObject(dest, (union RingObject *)sig);
		}
	}
	return i;
}

/* Return TRUE if the key has a secret in the given set */
int
ringKeyIsSec(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	for (key = key->g.down; key; key = key->g.next)
		if (pgpIsRingSetMember(set, key) && OBJISSEC(key))
			return 1;
	return 0;
}

/*
 * Return TRUE if the key comes only from sources where it has secret
 * objects.  In other words, the key comes from a secret key ring.  This
 * is used in adding that key so that we only add it to the secret ring,
 * which is necessary due to complications relating to the "version bug".
 * Otherwise if we add a secret keyring we may end up putting the key on
 * the pubring, and it could have the incorrect version.
 * (See pgpRngRead.c for discussion of the version bug.)
 * Don't count if just on MEMRING, otherwise newly created keys return TRUE.
 */
int
ringKeyIsSecOnly(RingSet const *set, union RingObject *key)
{
	RingPool *pool = set->pool;
	PGPVirtMask keyfilemask;
	PGPVirtMask secfilemask;
	PGPBoolean seconly;

	pgpVirtMaskInit (pool, &keyfilemask);
	pgpVirtMaskInit (pool, &secfilemask);

	pgpVirtMaskCopy (pool, &key->g.mask, &keyfilemask);
	pgpVirtMaskAND (pool, &pool->filemask, &keyfilemask);

	if (pgpVirtMaskIsEqual (&keyfilemask, &pool->memringmask)) {
		pgpVirtMaskCleanup (pool, &keyfilemask);
		pgpVirtMaskCleanup (pool, &secfilemask);
		return 0;			/* Newly generated keys */
	}
		
	/* Accumulate all sec objects into secfilemask */
	for (key = key->g.down; key; key = key->g.next) {
		if (pgpIsRingSetMember(set, key) && OBJISSEC(key)) {
			pgpVirtMaskOR (pool, &key->g.mask, &secfilemask);
		}
	}
	pgpVirtMaskAND (pool, &pool->filemask, &secfilemask);
	pgpVirtMaskANDNOT (pool, &secfilemask, &keyfilemask);
	seconly = pgpVirtMaskIsEmpty(&keyfilemask);

	pgpVirtMaskCleanup (pool, &keyfilemask);
	pgpVirtMaskCleanup (pool, &secfilemask);

	return seconly;
}


/*
 * Return the most recent subkey associated with the key, if there is one.
 */
union RingObject *
ringKeySubkey(RingSet const *set, union RingObject const *key)
{
	union RingObject *obj, *best = NULL;
	PGPUInt32 objtime, besttime = 0;

	pgpAssert(OBJISKEY(key));
	for (obj = key->g.down; obj; obj = obj->g.next) {
		if (pgpIsRingSetMember(set, obj) && OBJISSUBKEY(obj)
		    && ringSubkeyValid(set, obj, TRUE)) {
			objtime = ringKeyCreation(set, obj);
			if (besttime <= objtime) {
				best = obj;
				besttime = objtime;
			}
		}
	}
	return best;
}

/* Given a subkey, return its master key */
union RingObject *
ringKeyMasterkey (RingSet const *set, union RingObject const *subkey)
{
	(void)set;

	pgpAssert (OBJISSUBKEY(subkey));
	pgpAssert (OBJISTOPKEY(subkey->g.up));
	return subkey->g.up;
}


/*
 * Given a public key on the keyring, get the corresponding PGPPubKey.
 * Use is a usage code which limits the kinds of keys we will accept.
 * For keys which have subkeys this chooses which one to use.  If use is
 * 0 we do a straight conversion of the key or subkey; if nonzero we
 * verify that the key has the required use.  Return NULL if we can't
 * get a key with the required use.
 */
PGPPubKey *
ringKeyPubKey(RingSet const *set, union RingObject *key, int use)
{
	PGPByte const *p;
	PGPSize len;
	PGPPubKey *pub;
	union RingObject *subkey = NULL;
	unsigned vsize;
	PGPError i;
	PGPContextRef	context;

	pgpAssertAddrValid( set, RingSet );
	pgpAssertAddrValid( set->pool, RingPool );
	context	= set->pool->context;

	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	/* Select between subkey and key if necessary */
	if (use &&  (OBJISSUBKEY(key)
			|| ((subkey=ringKeySubkey(set, key)) != NULL))) {
		if (use == PGP_PKUSE_SIGN_ENCRYPT) {
			ringSimpleErr(set->pool, kPGPError_PublicKeyUnimplemented);
			return NULL;
		}
		if (use == PGP_PKUSE_ENCRYPT) {
			if (OBJISTOPKEY(key)) {
				pgpAssert(subkey);
				key = subkey;
				pgpAssert (OBJISSUBKEY(key));
			}
		} else if (use == PGP_PKUSE_SIGN) {
			if (OBJISSUBKEY(key)) {
				key = key->g.up;
				pgpAssert (OBJISTOPKEY(key));
			}
		}
	}

	/* Verify key satisfies required usage */
	if (use && ((pgpKeyUse(pgpPkalgByNumber(key->k.pkalg)) & use) != use)){
		ringSimpleErr(set->pool, kPGPError_PublicKeyUnimplemented);
		return NULL;
	}

	p = (PGPByte const *)ringFetchObject(set, key, &len);
	if (!p)
		return NULL;
	if (key->g.flags & KEYF_ERROR || len < 8) {
		i = (PGPError)ringKeyParse(context, p, len, NULL, NULL, NULL, NULL,
								   NULL, NULL, NULL, 0);
		ringSimpleErr(set->pool, i);
		return NULL;
	}
	/*
	 * A key starts with 5 or 7 bytes of data, an algorithm byte, and
	 * the public components.
	 */
	if (p[0] == PGPVERSION_4) {
		vsize = 0;
	} else {
		vsize = 2;
	}
	pgpAssert(p[5+vsize] == key->k.pkalg);	/* Checked by ringKeyVerify */
	pub = pgpPubKeyFromBuf( context, p[5+vsize], p+6+vsize, len-6-vsize, &i);
	if (!pub) {
 		ringSimpleErr(set->pool, i);
		return NULL;
	}
	memcpy(pub->keyID, key->k.keyID, sizeof(key->k.keyID));
	return pub;
}

/*
 * Find the best Secret which is a descendant of the given key,
 * in the given set.
 */
union RingObject *
ringBestSec(RingSet const *set, union RingObject const *key)
{
	PGPVirtMask better;
	union RingObject *obj, *best = 0;

	pgpVirtMaskInit (set->pool, &better);
	pgpVirtMaskNOT (set->pool, &better, set->pool->nfiles);

	pgpAssert(OBJISKEY(key));
	for (obj = key->g.down; obj; obj = obj->g.next) {
		if (pgpVirtMaskIsOverlapping (&obj->g.mask, &better)
			&& pgpIsRingSetMember (set, obj)
		    && OBJISSEC(obj)) {
			best = obj;
			ringObjBetters(obj, set->pool, &better);
		}
	}
	pgpVirtMaskCleanup (set->pool, &better);
	return best;

}

/*
 * Return the version for a secret key.  Also permissible to pass a key.
 * This should be used when we edit a pass phrase to preserve the version
 * number and avoid the infamous "version bug".
 */
PgpVersion
ringSecVersion (RingSet const *set, union RingObject *sec)
{
	PGPByte *secdata;
	PGPSize secdatalen;

	if (!OBJISSEC(sec)) {
		sec = ringBestSec(set, sec);
		if (!sec) {
			ringSimpleErr(set->pool, kPGPError_SecretKeyNotFound);
			return (PgpVersion)0;
		}
	}
	secdata = (PGPByte *)ringFetchObject (set, sec, &secdatalen);
	return (PgpVersion)secdata[0];
}


/*
 * Given a secret on a keyring, get a PGPSecKey (possibly locked).
 * As a hack to help the lazy programmer, you can also pass a key.
 * Use is a usage code which limits the kinds of keys we will accept.
 * For keys which have subkeys this chooses which one to use.
 */
PGPSecKey *
ringSecSecKey(RingSet const *set, union RingObject *sec, int use)
{
	PGPByte const *p;
	PGPSize len;
	PGPSecKey *seckey;
	union RingObject *key;
	union RingObject *subkey = NULL;
	unsigned vsize;
	PGPError i;
	PGPContextRef	context;

	pgpAssertAddrValid( set, RingSet );
	pgpAssertAddrValid( set->pool, RingPool );
	context	= set->pool->context;

	if (OBJISSEC(sec)) {
		key = sec->g.up;
	} else {
		key = sec;
	}
	pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, sec));

	/* Select between subkey and key if necessary */
	if (use	 &&  (OBJISSUBKEY(key)
			|| ((subkey=ringKeySubkey(set, key)) != NULL))) {
		int newkey = 0;
		if (use == PGP_PKUSE_SIGN_ENCRYPT) {
			ringSimpleErr(set->pool, kPGPError_PublicKeyUnimplemented);
			return NULL;
		}
		if (use == PGP_PKUSE_ENCRYPT) {
			if (OBJISTOPKEY(key)) {
				pgpAssert(subkey);
				key = subkey;
				pgpAssert (OBJISSUBKEY(key));
				newkey = 1;
			}
		} else if (use == PGP_PKUSE_SIGN) {
			if (OBJISSUBKEY(key)) {
				key = key->g.up;
				pgpAssert (OBJISTOPKEY(key));
				newkey = 1;
			}
		}
		if (newkey || !OBJISSEC(sec)) {
			sec = ringBestSec(set, key);
			if (!sec) {
				ringSimpleErr(set->pool, kPGPError_SecretKeyNotFound);
				return NULL;
			}
		}
	} else if (OBJISKEY(sec)) {
		sec = ringBestSec(set, sec);
		if (!sec) {
			ringSimpleErr(set->pool, kPGPError_SecretKeyNotFound);
			return NULL;
		}
	}

	/* Verify key satisfies required usage */
	if (use && ((pgpKeyUse(pgpPkalgByNumber(key->k.pkalg)) & use) != use)){
		ringSimpleErr(set->pool, kPGPError_PublicKeyUnimplemented);
		return NULL;
	}

	p = (PGPByte const *)ringFetchObject(set, sec, &len);
	if (!p)
		return NULL;
	if (sec->g.up->g.flags & KEYF_ERROR || len < 8) {
		i = (PGPError)ringKeyParse(set->pool->context, p, len, NULL, NULL,
								  NULL, NULL, NULL, NULL, NULL, 0);
		ringSimpleErr(set->pool, i);
		return NULL;
	}
	if (p[0] == PGPVERSION_4) {
		vsize = 0;
	} else {
		vsize = 2;
	}
	pgpAssert(p[5+vsize] == sec->g.up->k.pkalg); /* Checked by ringKeyVerify */
	seckey = pgpSecKeyFromBuf( context, p[5+vsize], p+6+vsize, len-6-vsize,
							   (PGPBoolean)!!KEYISV3(&sec->g.up->k), &i);
	if (!seckey) {
 		ringSimpleErr(set->pool, i);
		return NULL;
	}
	memcpy(seckey->keyID, sec->g.up->k.keyID, sizeof(sec->g.up->k.keyID));
	return seckey;
}


/* Return the latest valid sig on the object by the specified key */
RingObject *
ringLatestSigByKey (RingObject const *obj, RingSet const *set,
	RingObject const *key)
{
	RingObject *sig;
	RingObject *latestsig;

	pgpAssert (pgpIsRingSetMember(set, obj));
	pgpAssert (!OBJISBOT(obj));

	latestsig = NULL;
	for (sig=obj->g.down; sig; sig=sig->g.next) {
		if (!OBJISSIG(sig))
			continue;
		if (!pgpIsRingSetMember(set, sig))
			continue;
		if (ringSigMaker (set, sig, set) != key)
			continue;
		if ((ringSigType (set, sig) & 0xf0) != PGP_SIGTYPE_KEY_GENERIC)
			continue;
		if (!ringSigChecked (set, sig) || ringSigRevoked (set, sig))
			continue;
		/* Save the newest such signature on the name */
		if (!latestsig) {
			latestsig = sig;
		} else {
			if (ringSigTimestamp(set,sig) > ringSigTimestamp(set,latestsig))
				latestsig = sig;
		}
	}		
	return latestsig;
}


/* There ain't much to know about a name... */
char const *
ringNameName(RingSet const *set, union RingObject *name, PGPSize *lenp)
{
	pgpAssert(pgpIsRingSetMember(set, name));
	pgpAssert(OBJISNAME(name));
	return (char const *)ringFetchObject(set, name, lenp);
}

PGPBoolean
ringNameIsAttribute(RingSet const *set, union RingObject *name)
{
	(void) set; /* Avoid warning */
	
	pgpAssert(pgpIsRingSetMember(set, name));
	pgpAssert(OBJISNAME(name));
	
	(void) set;
	(void) name;
	
	return (PGPBoolean)NAMEISATTR(&name->n);
}

PGPUInt32
ringNameCountAttributes(RingSet const *set, union RingObject *name)
{
	PGPByte			*p;
	PGPSize			len;
	
	pgpAssert(pgpIsRingSetMember(set, name));
	pgpAssert(OBJISNAME(name));
	if (!NAMEISATTR(&name->n))
		return 0;

	p = (PGPByte *)ringFetchObject(set, name, &len);
	if (!p) {
		return 0;
	}

	return ringAttrCountSubpackets (p, len);
}


/* Return the nth attribute subpacket for the specified name */
/* This reads from disk and munges the pktbuf */
PGPByte const *
ringNameAttributeSubpacket (RingObject *name, RingSet const *set,
	PGPUInt32 nth, PGPUInt32 *subpacktype, PGPSize *plen, PGPError *error)
{
	PGPByte			*p;
	PGPSize			len;
	RingPool *pool = set->pool;
	
	pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));

	if (error)
		*error = kPGPError_NoErr;

	if (!NAMEISATTR(&name->n)) {
		if (error)
			*error = kPGPError_BadParams;
		return NULL;
	}

	p = (PGPByte *)ringFetchObject(set, name, &len);

	/* ringAttrSubpacket munges the packets so make a copy */
	if (IsntNull(p)  &&  p != (PGPByte *)pool->pktbuf) {
		PGPByte *pktbuf = (PGPByte *)ringReserve(pool, len);
		if (IsntNull(pktbuf)) {
			pgpCopyMemory(p, pktbuf, len);
		}
		p = pktbuf;
	}
	if (!p) {
		if (error)
			*error = ringSetError(set)->error;
		return NULL;
	}

	return ringAttrSubpacket(p, len, nth, subpacktype, plen);
}	

/* Return the primary userid of the specified attribute type */
/* This reads from disk and munges the pktbuf for nonzero attribute */
RingObject *
ringKeyPrimaryName (RingObject *key, RingSet const *set,
	PGPUInt32 type)
{
	RingObject *name;
	RingObject *firstname;
	RingObject *sig;
	RingObject *newestsig;
	PGPUInt32 subpacktype;

    pgpAssert(OBJISKEY(key));
	pgpAssert(pgpIsRingSetMember(set, key));

	newestsig = NULL;
	firstname = NULL;
	for (name=key->g.down; name; name=name->g.next) {
		if (!pgpIsRingSetMember(set, name))
			continue;
		if (!OBJISNAME(name))
			continue;
		if ((type == 0) != !NAMEISATTR(&name->n))
			continue;
		subpacktype = 0;
		if (NAMEISATTR(&name->n)) {
			(void)ringNameAttributeSubpacket (name, set, 0, &subpacktype,
											  NULL, NULL);
			if (subpacktype != type)
				continue;
		}
		/* Have a name which is the right attribute type */
		if (!firstname)
			firstname = name;
		sig = ringLatestSigByKey (name, set, key);
		if (sig) {
			pgpAssert (OBJISSIG(sig));
			if (SIGISPRIMARYUID (&sig->s)) {
				if (!newestsig) {
					newestsig = sig;
				} else {
					/* Don't override irrevocable settings */
					if (SIGISREVOCABLE(&newestsig->s) &&
								(ringSigTimestamp(set, sig) >
								 ringSigTimestamp(set,newestsig))) {
						newestsig = sig;
					}
				}
			}
		}
	}

	if (firstname == NULL)
		return NULL;

	if (newestsig) {
		name = newestsig->g.up;
	} else {
		name = firstname;
	}

	pgpAssert (name);
	pgpAssert (OBJISNAME(name));
	return name;
}


/*  Return the validity (*not* the trust) of a name */

PGPByte
ringNameTrust(RingSet const *set, union RingObject *name)
{
    union RingObject *key;
	
    pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));
    (void)set;
    key = name->g.up;
    pgpAssert(OBJISTOPKEY(key));
    /*
	 * Force returned value if key is revoked or axiomatic.
	 * Allow expired keys to stay valid, so users can know what their status
	 * was before they expired.
	 */
    if (key->k.trust & PGP_KEYTRUSTF_REVOKED)
        return kPGPNameTrust_Untrusted;
    if (key->k.trust & PGP_KEYTRUSTF_BUCKSTOP)
        return kPGPNameTrust_Complete;
    if (!(name->g.flags & (RINGOBJF_TRUST)))
        ringMntValidateName (set, name);
    return name->n.trust & kPGPNameTrust_Mask;
}

int
ringNameWarnonly(RingSet const *set, union RingObject *name)
{
	pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));
	(void)set;
	return name->n.trust & PGP_NAMETRUSTF_WARNONLY;
}

void
ringNameSetWarnonly(RingSet const *set, union RingObject *name)
{
	pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));
	if (!(name->n.trust & PGP_NAMETRUSTF_WARNONLY)) {
		name->n.trust |= PGP_NAMETRUSTF_WARNONLY;
		name->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, &name->g.mask);
	}
	name->g.flags |= RINGOBJF_TRUST;
}

PGPUInt16
ringNameValidity(RingSet const *set, union RingObject *name)
{
#if PGPTRUSTMODEL==0
    (void)set;
    (void)name;
    pgpAssert(0);
    return 0;
#else
    union RingObject *key;
    pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));
    (void)set;
    key = name->g.up;
    pgpAssert (OBJISTOPKEY(key));
	/*
	 * Force returned value if key is revoked or axiomatic.
	 * Allow expired keys to stay valid, so users can know what their status
	 * was before they expired.
	 */
    if (key->k.trust & PGP_KEYTRUSTF_REVOKED)
        return 0;
    if (key->k.trust & PGP_KEYTRUSTF_BUCKSTOP)
        return PGP_TRUST_INFINITE;
    if (!(name->g.flags & (RINGOBJF_TRUST)))
        ringMntValidateName (set, name);
    return ringTrustToIntern (name->n.validity); 
#endif
}

PGPUInt16
ringNameConfidence(RingSet const *set, union RingObject *name)
{
#if PGPTRUSTMODEL==0
	(void)set;
	(void)name;
	pgpAssert(0);
	return 0;
#else
	pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));
	(void)set;
	return ringTrustToIntern (name->n.confidence);
#endif
}

int 
ringNameConfidenceUndefined(RingSet const *set, union RingObject *name)
{
#if PGPTRUSTMODEL==0
	(void)set;
	(void)name;
	pgpAssert(0);
	return 0;
#else
	pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));
	(void)set;
	return (name->n.confidence == PGP_NEWTRUST_UNDEFINED);
#endif
}


void
ringNameSetConfidence(RingSet const *set, union RingObject *name,
		      PGPUInt16 confidence)
{
#if PGPTRUSTMODEL==0
	(void)set;
	(void)name;
	(void)confidence;
	pgpAssert(0);
#else
	pgpAssert(OBJISNAME(name));
	pgpAssert(pgpIsRingSetMember(set, name));

	confidence = (PGPUInt16) ringTrustToExtern (confidence);
	
	if (name->n.confidence != confidence) {
		name->n.confidence = (PGPByte) confidence;
		name->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, &name->g.mask);
	}
	name->g.flags |= RINGOBJF_TRUST;
#endif
}

int
ringSigError(RingSet const *set, union RingObject *sig)
{
	PGPByte const *p;
	PGPSize len;

	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));

	if (!(sig->g.flags & SIGF_ERROR))
		return 0;

	p = (PGPByte const *)ringFetchObject(set, sig, &len);
	if (!p)
		return ringSetError(set)->error;
	return ringSigParse(p, len, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

union RingObject *
ringSigMaker(RingSet const *sset, union RingObject *sig,
	RingSet const *kset)
{
	(void)sset;	/* Avoid warning */
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(sset, sig));

	sig = sig->s.by;
	pgpAssert(OBJISKEY(sig));	/* "sig" is now a key! */
	if (!pgpIsRingSetMember(kset, sig))
		return NULL;
	ringObjectHold(sig);
	return sig;
}

void
ringSigID8(RingSet const *set, union RingObject const *sig,
	PGPByte *pkalg, PGPKeyID *keyID)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	sig = sig->s.by;
	pgpAssert(OBJISKEY(sig));
	if (pkalg) {
		*pkalg = sig->k.pkalg;
		if ((*pkalg | 1) == 3)
			*pkalg = 1;	/* ViaCrypt */
	}
	if ( keyID )
	{
		pgpNewKeyIDFromRawData( sig->k.keyID, 8, keyID );
	}
}

PGPByte
ringSigTrust(RingSet const *set, union RingObject *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	if (ringSigError (set, sig))
		return PGP_SIGTRUST_INVALID;
	if (sig->s.by == NULL)
		return PGP_SIGTRUST_NOKEY;
	if (!(sig->s.trust & PGP_SIGTRUSTF_TRIED))
		return PGP_SIGTRUST_UNTRIED;
	if (!(sig->s.trust & PGP_SIGTRUSTF_CHECKED))
		return PGP_SIGTRUST_BAD;
	if (!(sig->g.flags & (RINGOBJF_TRUST))) {
		ringMntValidateKey (set, sig->s.by);
		return sig->s.by->k.trust & kPGPKeyTrust_Mask;
	}
	else
	        return sig->s.trust & kPGPKeyTrust_Mask;
}

int
ringSigChecked(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	return sig->s.trust & PGP_SIGTRUSTF_CHECKED;
}

int
ringSigTried(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	return sig->s.trust & PGP_SIGTRUSTF_TRIED;
}

int
ringSigExportable(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	return SIGISEXPORTABLE(&sig->s);
}

PGPByte
ringSigTrustLevel(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	return sig->s.trustLevel;
}

PGPByte
ringSigTrustValue(RingSet const *set, union RingObject const *sig)
{
	PGPByte trustValue;

	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	trustValue = sig->s.trustValue;
#if PGPTRUSTMODEL==0
	trustValue = ringTrustExternToOld(set->pool, trustValue);
#endif
	return sig->s.trustValue;
}

/* Call ringSigTrust to get sig status, then call this function if
   sig is good and the confidence is required. */

PGPUInt16
ringSigConfidence(RingSet const *set, union RingObject *sig)
{
#if PGPTRUSTMODEL==0
	(void)set;
	(void)sig;
	pgpAssert(0);
	return 0;
#else
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	if (sig->s.by != NULL) {
		if (pgpIsRingSetMember(set, sig->s.by)) {
			if (sig->s.by->k.trust & PGP_KEYTRUSTF_REVOKED)
				return 0;
			else
				return ringKeyCalcTrust (set, sig->s.by);
		}
		else
			return 0;
	}
	else
		return 0;
#endif
}


int
ringSigType(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	return sig->s.type;
}

PGPUInt32
ringSigTimestamp(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	return sig->s.tstamp;
}

PGPUInt32
ringSigExpiration(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	(void)set;
	if (sig->s.tstamp == 0 || sig->s.sigvalidity == 0)
		return 0;    /* valid indefinitely */
	else
		return sig->s.tstamp + sig->s.sigvalidity;
}

/*
 * True if sig is a self-sig.
 */
PGPBoolean
ringSigIsSelfSig(RingSet const *set, RingObject const *sig)
{
	RingObject const	*parent;

	(void)set;
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));

	/* Find top-level key */
	parent = sig;
	while (!OBJISTOPKEY(parent))
		parent = parent->g.up;

	/* No good if not signed by top-level key (selfsig) */
	if (sig->s.by != parent)
		return FALSE;

	/* All OK */
	return TRUE;
}



/*
 * True if sig is an X509 sig.
 */
PGPBoolean
ringSigIsX509(RingSet const *set, RingObject const *sig)
{
	(void)set;
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));

	return SIGISX509(&sig->s) != 0;
}

PGPByte *
ringSigX509Certificate(RingSet const *set, RingObject *sig, PGPSize *len)
{
	PGPByte *ptr;

	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	pgpAssert(IsntNull(len));

	*len = 0;
	if (!SIGISX509 (&sig->s))
		return NULL;
	ptr = (PGPByte *)ringFetchObject(set, sig, len);
	if (IsNull( ptr ) )
		return NULL;
	ptr = (PGPByte *)ringSigFindNAISubSubpacket(ptr, SIGSUBSUB_X509, 0, len,
												NULL, NULL, NULL, NULL);
	if( IsntNull( ptr ) ) {
		/* Skip type, version bytes */
		pgpAssert (ptr[0] == SIGSUBSUB_X509);
		ptr += 3;
		*len -= 3;
	}

	return ptr;
}

int
ringCRLChecked(RingSet const *set, union RingObject const *crl)
{
	pgpAssert(OBJISCRL(crl));
	pgpAssert(pgpIsRingSetMember(set, crl));
	(void)set;
	return crl->r.trust & PGP_SIGTRUSTF_CHECKED;
}

int
ringCRLTried(RingSet const *set, union RingObject const *crl)
{
	pgpAssert(OBJISCRL(crl));
	pgpAssert(pgpIsRingSetMember(set, crl));
	(void)set;
	return crl->r.trust & PGP_SIGTRUSTF_TRIED;
}

PGPUInt32
ringCRLCreation(RingSet const *set, union RingObject const *crl)
{
	pgpAssert(OBJISCRL(crl));
	pgpAssert(pgpIsRingSetMember(set, crl));
	(void)set;
	return crl->r.tstamp;
}

PGPUInt32
ringCRLExpiration(RingSet const *set, union RingObject const *crl)
{
	pgpAssert(OBJISCRL(crl));
	pgpAssert(pgpIsRingSetMember(set, crl));
	(void)set;
	return crl->r.tstampnext;
}

/* True if the key has a CRL */
PGPBoolean
ringKeyHasCRL(RingSet const *set, RingObject *key)
{
	RingIterator *iter;
	union RingObject *obj;
	PGPInt32 level;

	if (!set)
		return FALSE;

	iter = ringIterCreate(set);
	if (!iter)
		return FALSE;	/* How to distinguish from no luck? */

	ringIterSeekTo (iter, key);
	level = ringIterCurrentLevel(iter);
	while (ringIterNextObject(iter, level+1) > 0) {
		obj = ringIterCurrentObject(iter, level+1);
		pgpAssert(obj);
		if (!OBJISCRL(obj))
			continue;
		/* Don't count unverifiable CRLs */
		if (!ringCRLChecked(set, obj))
			continue;
		ringIterDestroy(iter);
		return TRUE;
	}
	ringIterDestroy(iter);
	return FALSE;
}

/* Return nth CRL of key, along with a count of all CRLs if requested.
 * Doesn't count superceded CRLs.
 * Call this the first time with n=0 and &crlcount, and later times with
 * crlcount holding NULL.
 */
RingObject *
ringKeyNthCRL(RingSet const *set, RingObject *key, PGPUInt32 n,
	PGPUInt32 *crlcount)
{
	RingIterator *iter;
	RingObject *obj;
	RingObject *nthcrl = NULL;
	PGPUInt32 count = 0;
	PGPInt32 level;

	if( IsntNull( crlcount ) )
		*crlcount = 0;

	if (!set)
		return NULL;
	iter = ringIterCreate(set);
	if (!iter)
		return NULL;	/* How to distinguish from no luck? */

	ringIterSeekTo (iter, key);
	level = ringIterCurrentLevel(iter);
	while (ringIterNextObject(iter, level+1) > 0) {
		obj = ringIterCurrentObject(iter, level+1);
		pgpAssert(obj);
		if (!OBJISCRL(obj))
			continue;
		if (!ringCRLChecked(set, obj))
			continue;
		if (!ringCRLIsCurrent(set, obj, 0))
			continue;
		if (count++ == n) {
			nthcrl = obj;
			if (IsNull( crlcount ))
				break;
		}
	}
	ringIterDestroy(iter);
	if( IsntNull( crlcount ) )
		*crlcount = count;
	return nthcrl;
}


/*
 * Find the earliest non-replaced CRL issued by a key.
 * If expiration is true, use expiration dates, else creation dates.
 */
union RingObject *
ringKeyEarliestCRL(RingSet const *set, RingObject *key, PGPBoolean expiration)
{
	RingObject	*crl;
	RingObject *bestcrl = NULL;
	PGPUInt32 crltime;
	PGPUInt32 besttime = (PGPUInt32)-1L;
	PGPUInt32 n = 0;
	PGPUInt32 crlcount = 0;

	do {
		crl = ringKeyNthCRL(set, key, n, (n?NULL:&crlcount));
		if( IsNull( crl ) )
			break;
		if( expiration )
			crltime = ringCRLExpiration( set, crl );
		else
			crltime = ringCRLCreation( set, crl );
		if (crltime < besttime) {
			besttime = crltime;
			bestcrl = crl;
		}
	} while (n++ < crlcount);

	return bestcrl;
}


/*
 * See whether there is a more recent CRL for this key with the same
 * dist. point.
 * If tstamp is nonzero, also checks for expiration of CRL.
 */
PGPBoolean
ringCRLIsCurrent (RingSet const *set, RingObject *crl, PGPUInt32 tstamp)
{
	RingIterator *iter;
	union RingObject *obj;
	PGPUInt32 crlcreation, crlexpiration;
	PGPUInt32 objcreation;
	PGPInt32 level;
	PGPByte const *tmpdpoint;
	PGPByte *dpoint = NULL;
	PGPByte const *dpoint2;
	PGPSize dpointlen;
	PGPSize dpoint2len;
	PGPContextRef context;

	pgpAssert (IsntNull(set));
	pgpAssert (OBJISCRL(crl));
	pgpAssert (pgpIsRingSetMember(set, crl));

	context = ringPoolContext( ringSetPool(set) );

	crlcreation = ringCRLCreation(set, crl);
	crlexpiration = ringCRLExpiration(set, crl);
	if (tstamp != 0) {
		if (crlexpiration < tstamp)
			return FALSE;
	}

	iter = ringIterCreate(set);
	if (!iter)
		return TRUE;	/* How to distinguish from no luck? */

	if( CRLHASDPOINT( &crl->r ) ) {
		/* Read dpoint data structure */
		tmpdpoint = ringCRLDistributionPoint( set, crl, &dpointlen );
		dpoint = pgpContextMemAlloc( context, dpointlen, 0 );
		pgpCopyMemory( tmpdpoint, dpoint, dpointlen );
		if( IsNull( dpoint ) ) {
			ringIterDestroy( iter );
			return TRUE;
		}
	}

	/* Find key above CRL */
	ringIterSeekTo (iter, crl);
	level = ringIterCurrentLevel(iter);
	pgpAssert (level > 1);
	obj = ringIterCurrentObject(iter, level-1);
	pgpAssert (OBJISKEY(obj));
	while (ringIterNextObject(iter, level) > 0) {
		obj = ringIterCurrentObject(iter, level);
		pgpAssert(obj);
		if (obj == crl)
			continue;
		if (!OBJISCRL(obj))
			continue;
		if (!ringCRLChecked(set, obj))
			continue;
		if (CRLHASDPOINT(&obj->r) != CRLHASDPOINT(&crl->r))
			continue;
		if (CRLHASDPOINT(&crl->r)) {
			if (crl->r.dpointhash != obj->r.dpointhash)
				continue;
			dpoint2 = ringCRLDistributionPoint( set, obj, &dpoint2len );
			if (dpointlen != dpoint2len ||
				!pgpMemoryEqual( dpoint, dpoint2, dpointlen ))
				continue;
		}
		objcreation = ringCRLCreation(set, obj);
		if (objcreation > crlcreation) {
			ringIterDestroy(iter);
			if (CRLHASDPOINT( &crl->r ) )
				pgpContextMemFree( context, dpoint );
			return FALSE;
		}
	}
	ringIterDestroy(iter);
	if (CRLHASDPOINT( &crl->r ) )
		pgpContextMemFree( context, dpoint );
	return TRUE;
}

/* Return CRL Distribution Point, in the "fetch" buffer */
/* Return NULL if CRL has no distribution point */
PGPByte const *
ringCRLDistributionPoint( RingSet const *set, RingObject *crl, PGPSize *len )
{
	PGPByte const *ptr;

	pgpAssert (IsntNull(set));
	pgpAssert (OBJISCRL(crl));
	pgpAssert (pgpIsRingSetMember(set, crl));
	pgpAssert (IsntNull( len ) );

	*len = 0;
	if( !CRLHASDPOINT( &crl->r ) ) 
		return NULL;

	ptr = ringFetchObject(set, crl, len);
	if (IsNull( ptr ) )
		return NULL;
	ptr = ringCRLFindDPoint( ptr, *len, len );

	return ptr;
}


/*
 * Return a buffer full of all the CRL Distribution Points associated with
 * a given key.  We find them by listing those in the CRLs we have stored
 * with the key, plus we search all certs which the key has issued and get
 * any others from those.  Also return the number of CDP's in the buffer.
 * The buffer is a newly allocated buffer.
 * Also return a newly allocated buffer of sizes of the CDP's.
 */
PGPError
ringListCRLDistributionPoints(
	PGPMemoryMgrRef mgr,				/* Input parameters */
	RingObject *key,
	RingSet const *set,
	PGPUInt32 *pnDistPoints,			/* Output parameters */
	PGPByte **pbuf,
	PGPSize **pbufsizes
	)
{
	PGPASN_XTBSCertificate *xtbscert;
	PGPUInt32 nDistPoints = 0;
	PGPByte *buf = NULL;
	PGPSize bufsize = 0;
	PGPSize *bufsizes = NULL;
	PGPByte const *crldpoint;
	PGPByte const *certdpoint = NULL;		/* dynamically allocated */
	PGPSize dpointlen;
	PGPError err = kPGPError_NoErr;
	PGPUInt32 nth;
	RingObject *crlobj;
	RingObject *sig;
	PGPByte *p;
	PGPSize len;
	void *vbuf;
	PGPUInt32 crlcount;

	pgpAssert( IsntNull( pbuf ) );
	pgpAssert( IsntNull( pbufsizes ) );
	pgpAssert( IsntNull( pnDistPoints ) );

	*pbuf = NULL;
	*pbufsizes = NULL;
	*pnDistPoints = 0;

	/* First check any CRLs stored with the key */
	nth = 0;
	do
	{
		crlobj = ringKeyNthCRL( set, key, nth, &crlcount );
		if( IsNull( crlobj ) )
			break;
		crldpoint = ringCRLDistributionPoint( set, crlobj, &dpointlen );
		if( IsntNull( crldpoint ) )
		{
			if( !pgpX509BufInSequenceList( crldpoint,dpointlen,buf,bufsize ) )
			{
				bufsize += dpointlen;
				vbuf = buf;
				err = PGPReallocData( mgr, &vbuf, bufsize, 0 );
				buf = vbuf;
				if( IsPGPError( err ) )
					goto error;
				pgpCopyMemory( crldpoint, buf+bufsize-dpointlen, dpointlen );
				++nDistPoints;
				vbuf = bufsizes;
				err = PGPReallocData( mgr, &vbuf,
									  nDistPoints*sizeof(PGPSize), 0 );
				bufsizes = vbuf;
				if( IsPGPError( err ) )
					goto error;
				bufsizes[nDistPoints-1] = dpointlen;
			}
		}
	} while (++nth < crlcount);

	/* Now check all certs signed by key */
	for( sig = key->k.sigsby; IsntNull( sig );
		 						sig = (RingObject *)sig->s.nextby )
	{
		if( !pgpIsRingSetMember( set, sig ) )
			continue;
		if( !SIGISX509( &sig->s )  ||  !SIGHASDISTPOINT( &sig->s ) )
			continue;
		/* Find distribution point in cert */
		p = (PGPByte *)ringFetchObject(set, sig, &len);
		if( IsNull( p ) )
			continue;
		p = (PGPByte *)ringSigFindNAISubSubpacket(p, SIGSUBSUB_X509, 0, &len,
												  NULL, NULL, NULL, NULL);
		if( IsNull( p ) )
			continue;
		p += 3;
		len -= 3;
		err = pgpX509BufferToXTBSCert( set->pool->context, p, len, &xtbscert);
		if( IsPGPError( err ) ) {
			err = kPGPError_NoErr;
			continue;
		}
		/* Note that dpointcert is dynamically allocated */
		certdpoint = pgpX509XTBSCertToDistPoint( xtbscert, &dpointlen );
		pgpX509FreeXTBSCert( set->pool->context, xtbscert );

		if( IsntNull( certdpoint ) &&
			!pgpX509BufInSequenceList( certdpoint, dpointlen, buf, bufsize ) )
		{
			bufsize += dpointlen;
			vbuf = buf;
			err = PGPReallocData( mgr, &vbuf, bufsize, 0 );
			buf = vbuf;
			if( IsPGPError( err ) )
				goto error;
			pgpCopyMemory( certdpoint, buf+bufsize-dpointlen, dpointlen );
			++nDistPoints;
			vbuf = bufsizes;
			err = PGPReallocData( mgr, &vbuf,
								  nDistPoints*sizeof(PGPSize), 0);
			bufsizes = vbuf;
			if( IsPGPError( err ) )
				goto error;
			bufsizes[nDistPoints-1] = dpointlen;
		}
		PGPFreeData( (PGPByte *)certdpoint );
		certdpoint = NULL;
	}
	
	*pbuf = buf;
	*pbufsizes = bufsizes;
	*pnDistPoints = nDistPoints;

	return kPGPError_NoErr;

 error:

	if( IsntNull( buf ) )
		PGPFreeData( buf );
	if( IsntNull( bufsizes ) )
		PGPFreeData( bufsizes );
	if( IsntNull( certdpoint ) )
		PGPFreeData( (PGPByte *)certdpoint );
	return err;

}




/** Filtering functions to get sets from sets **/

/* The generic one - according to the predicate */

int
ringSetFilter(RingSet const *src, RingSet *dest,
	      int (*predicate)(void *arg, RingIterator *iter,
		               union RingObject *object, unsigned level),
	      void *arg)
{
	RingIterator *iter;
	union RingObject *obj;
	unsigned level;
	int i;
	unsigned total = 0;

	if (!src || !dest)
		return 0;

	pgpAssert(!RINGSETISMUTABLE(src));
	pgpAssert(RINGSETISMUTABLE(dest));

	iter = ringIterCreate(src);
	if (!iter)
		return ringSetError(src)->error;
	level = 1;
	for (;;) {
		i = ringIterNextObject(iter, level);
		if (i > 0) {
			obj = ringIterCurrentObject(iter, level);
			i = predicate(arg, iter, obj, level);
			if (i < 0) {
				ringIterDestroy(iter);
				return i;
			}
			if (i) {
				/* Calculate total number of keys */
				total += (level == 1);
				/* ringSetAddObject(dest, obj) */
				pgpVirtMaskOR (src->pool, &dest->mask, &obj->g.mask);
				++level; /* Recurse! */
				ringIterRewind(iter, level);
			}
		} else {
			if (i < 0 || !--level)
				break;
		}
	}
	ringIterDestroy(iter);
	/* Return error or number of keys found */
	return (i < 0) ? i : (total < INT_MAX) ? total : INT_MAX;
}

/*
 * Return pointer to first instance of (s1,l1) in (s0,l0),
 * ignoring case.  Uses a fairly simple-minded algorithm.
 * Search for the first char of s1 in s0, and when we have it,
 * scan for the rest.
 *
 * Is it worth mucking with Boyer-Moore or the like?
 */
static char const *
xmemimem(char const *s0, size_t l0, char const *s1, size_t l1)
{
	char c0, c1, c2;
	size_t l;

	/*
	 * The trivial cases - this means that NULL inputs are very legal
	 * if the corresponding lengths are zero.
	 */
	if (l0 < l1)
		return NULL;
	if (!l1)
		return s0;
	l0 -= l1;

	c1 = tolower((unsigned char)*s1);
	do {
		c0 = tolower((unsigned char)*s0);
		if (c0 == c1) {
			l = 0;
			do {
				if (++l == l1)
					return s0;
				c0 = tolower((unsigned char)s0[l]);
				c2 = tolower((unsigned char)s1[l]);
			} while (c0 == c2);
		}
		s0++;
	} while (l0--);
	return NULL;
}

typedef struct KeySpec
{
	char const *keyid, *name;
	size_t keyidlen, namelen;
	int use;
	DEBUG_STRUCT_CONSTRUCTOR( KeySpec )
} KeySpec;

/*
 * Allowed formats for the keyspec are:
 * NULL, "", "*" - Match everything
 * "0x123c" - match everything with a keyID containing "123c"
 * "Name" - match everything with a name containing "name" (case-insensitive)
 * "0x123c:name" - match everything satisfying both requirements
 *
 * This returns pointers to "keyidspec" and "uidspec", the portions of the
 * input keyspec string which should match the keyID and userID portions,
 * or NULL if there are no such portions (which means "always mauch").
 *
 * This function cannot have any errors.  At worst, the entire string
 * is taken to be a uid match.  Some corner cases:
 *
 * 0              -> No keyidspec, namespec of "0"
 * 0x             -> Empty keyidspec, no namespec
 * 0x:            -> Empty keyidspec, empty namespec
 * 0x12345678:    -> Keyidspec of "12345678", empty namespec
 * 0x12345678:foo -> Keyidspec of "12345678", namespec of "foo"
 * 0x12345678;foo -> No keyidspec, namespec of "0x12345678;foo"
 * 0x12345678	  -> Keyidspec of "12345678", no namespec
 *
 * Keyid's are now allowed to be up to 16 characters long.  If they are
 * 8 chars or less, they are only matched against the low 32 bits of
 * the key's keyid.  If greater, they are matched against the full 64
 * bits of keyid.
 */
static void
keyspecSplit(char const *string, int use, KeySpec *spec)
{
	unsigned i;

	spec->use = use;

	spec->keyidlen = spec->namelen = 0;	/* Match anything */

	/* NULL is nothing */
	if (!string)
		return;

	/* Does it look like it might start with a keyID spec? */
	if (string[0] == '0' && (string[1] == 'x' || string[1] == 'X')) {
		i = 2;
		/* Accept no more than 16 hex digits */
		while (isxdigit((int) string[i]) && ++i != 2+16)
			;
		/* Then check for proper termination: NULL or : */
		if (!string[i]) {
			spec->keyid = string+2;
			spec->keyidlen = i-2;
			return;
		} else if (string[i] == ':') {
			spec->keyid = string+2;
			spec->keyidlen = i-2;
			string += i+1;
		} /* Otherwise forget it, it's all namespec */
	}

	/* If not "*", it's a pattern */
	if (string[0] != '*' || string[1]) {
		spec->name = string;
		spec->namelen = strlen(string);
	}
}

/*
 * Return true if string "arg" (terminated by null or ':') appears in
 * the hex expansion of the given keyID.  Case-insensitive.
 */
static int
matchKeyID(PGPByte const keyID[8], char const *pat, size_t len)
{
	char buf[16];
	char const hex[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};

	buf[ 8] = hex[ keyID[4] >> 4 ];
	buf[ 9] = hex[ keyID[4] & 15 ];
	buf[10] = hex[ keyID[5] >> 4 ];
	buf[11] = hex[ keyID[5] & 15 ];
	buf[12] = hex[ keyID[6] >> 4 ];
	buf[13] = hex[ keyID[6] & 15 ];
	buf[14] = hex[ keyID[7] >> 4 ];
	buf[15] = hex[ keyID[7] & 15 ];

	if (len <= 8) {
		return xmemimem(buf+8, 8, pat, len) != NULL;
	}

	/* Here if input keyID was > 8 chars, we look at full 64 bit keyid */
	buf[0] = hex[ keyID[0] >> 4 ];
	buf[1] = hex[ keyID[0] & 15 ];
	buf[2] = hex[ keyID[1] >> 4 ];
	buf[3] = hex[ keyID[1] & 15 ];
	buf[4] = hex[ keyID[2] >> 4 ];
	buf[5] = hex[ keyID[2] & 15 ];
	buf[6] = hex[ keyID[3] >> 4 ];
	buf[7] = hex[ keyID[3] & 15 ];

	return xmemimem(buf, 16, pat, len) != NULL;
}

static int
predicateFilterName(void *arg, RingIterator *iter,
	union RingObject *obj, unsigned level)
{
	KeySpec const *spec;
	RingSet const *set;
	PGPKeyID		keyID;
	char const *nam;
	PGPSize len;
	int i;
	

	if (level > 1)
		return 1;	/* All children included if top level is */

	pgpAssert(OBJISKEY(obj));

	spec = (KeySpec const *)arg;
	set = ringIterSet(iter);

	/* Check for usage */
	if (spec->use  &&  (ringKeyUse(set,obj)&spec->use) != spec->use)
		return 0;	/* Doesn't have required usage */

	if (spec->keyidlen) {
		ringKeyID8(set, obj, NULL, &keyID);
		if (!matchKeyID( pgpGetKeyBytes( &keyID ),
				spec->keyid, spec->keyidlen))
		{
			union RingObject *subkey = ringKeySubkey(set, obj);
			if (!subkey)
				return 0;
			ringKeyID8(set, subkey, NULL, &keyID);
			if (!matchKeyID( pgpGetKeyBytes( &keyID ),
					spec->keyid, spec->keyidlen))
				return 0;
		}
	}

	/*
	 * This isn't quite consistent, because it'll accept a key
	 * with *no* names if the name specification is empty.
	 */
	if (spec->namelen == 0)
		return 1;	/* Match! */
	/*
	 * Search names for a matching name.  If *one* is found,
	 * the entire key, including all names, is taken
	 */
	while ((i = ringIterNextObject(iter, 2)) > 0) {
		obj = ringIterCurrentObject(iter, 2);
		if (ringObjectType(obj) != RINGTYPE_NAME)
			continue;
		nam = ringNameName(set, obj, &len);
		if (!nam)
			return ringSetError(set)->error;
		if (xmemimem(nam, len, spec->name, spec->namelen))
			return 1;	/* Match, take it! */
	}
	return i;	/* No match or error */
}

/*
 * Perform filtering based on a keyspec.
 * Use is a PGP_PKUSE value to specify the purpose of the key.
 * Pass 0 to match all keys.  PGP_PKUSE_SIGN_ENCRYPT gets only ones which
 * can do both things.
 */
int
ringSetFilterSpec(RingSet const *src, RingSet *dest,
	char const *string, int use)
{
	KeySpec spec;

	keyspecSplit(string, use, &spec);
	return ringSetFilter(src, dest, predicateFilterName, (void *)&spec);
}


/*
 * Find the most recent secret key matching a keyspec.
 * Limit keys to those which have the specified use.  Pass 0 to match
 * all uses.  If tstamp is nonzero, also checks for expiration of keys.
 */
union RingObject *
ringLatestSecret(RingSet const *set, char const *string,
		      PGPUInt32 tstamp, int use)
{
	KeySpec spec;
	RingIterator *iter;
	union RingObject *obj, *best = NULL;
	PGPUInt32 objtime, besttime = 0;
	PGPUInt32 exptime;
	int i;

	if (!set)
		return NULL;

	iter = ringIterCreate(set);
	if (!iter)
		return NULL;	/* How to distinguish from no luck? */
	keyspecSplit(string, use, &spec);

	while (ringIterNextObject(iter, 1) > 0) {
		obj = ringIterCurrentObject(iter, 1);
		pgpAssert(obj);
		if (!ringKeyIsSec(set, obj))
			continue;
		if (ringKeyRevoked(set, obj))
			continue;
		i = predicateFilterName((void *)&spec, iter, obj, 1);
		if (!i)
			continue;
		if (i < 0) {
			ringObjectRelease(best);
			best = NULL;
			break;
		}

		objtime = ringKeyCreation(set, obj);
		exptime = ringKeyExpiration(set, obj);
		if (besttime <= objtime && (!tstamp || !exptime ||
					    tstamp <= exptime)) {
			ringObjectRelease(best);	/* OK if best = NULL */
			best = obj;
			ringObjectHold(best);
			besttime = objtime;
		}
	}
	ringIterDestroy(iter);
	return best;
}


/*
 * Return key self-sig subpacket information.  Searches all sigs below
 * the key for a self sig, finds most recent one with desired info.
 * nth is 0 to find first matching packet, 1 for second, etc.  The
 * semantics have changed (yet again) from earlier versions in order
 * to more easily supercede old signatures.
 * 
 * The rule now is that non-revocable signatures always have precedence.
 * Any subpacket in such a signature cannot be revoked, and will be returned
 * first.  Then, we take the latest self signature on the primary userid
 * of the key and we see whether it has the subpacket of interest.
 * So there is one revocable self signature that counts, and it is the
 * latest one.  Plus, all the nonrevocable self sigs count.
 *
 * pmatches is filled in with the total
 * number of instances in all packets.  The plen, pcritical, phashed,
 * and pcreated values are filled in with the signature packet
 * corresponding to the nth instance of the data we want.
 *
 * key			key to use
 * set			set containing key
 * subpacktype	subpacket type to search for
 * nth			nth matching subpacket to find
 * *plen		return length of data
 * *pcritical	return criticality field of subpacket
 * *phashed		return whether subpacket was in hashed region
 * *pcreation	return creation time of matching signature
 * *pmatches	return number of matches of this subpack type
 * *error		return error code
 *
 * Function returns pointer to the data, of length *plen, or NULL with *error
 * set for error code.  If matching packet is not found, return NULL
 * with *error = 0.
 */
PGPByte const *
ringKeyFindSubpacket (RingObject *obj, RingSet const *set,
	int subpacktype, unsigned nth,
	PGPSize *plen, int *pcritical, int *phashed, PGPUInt32 *pcreation,
	unsigned *pmatches, PGPError *error)
{
	RingObject		*sig, *bestsig, *pname;
	RingIterator	*iter;
	PGPUInt32		bestcreation;
	PGPUInt32		totalmatches;
	PGPUInt32		skippedmatches;
	PGPByte			*p;
	PGPByte const	*bestp;
	PGPSize			bestlen;
	int				bestcritical;
	int				besthashed;
	int				level;
	PGPUInt32		creation;
	uint			matches;
	PGPSize			len;


	pgpAssert(OBJISKEY(obj));
	pgpAssert(pgpIsRingSetMember(set, obj));
	
	bestlen = bestcritical = besthashed = bestcreation = 0;
	totalmatches = skippedmatches = 0;
	sig = NULL;
	bestp = NULL;
	bestsig = NULL;

	if (error)
		*error = kPGPError_NoErr;
	iter = ringIterCreate (set);
	if (!iter) {
		if (error)
			*error = ringSetError(set)->error;
		return NULL;
	}
	ringIterSeekTo (iter, obj);
	/* First look for nonrevocable signatures */
	while (TRUE) {
		if ((level = ringIterNextObjectAnywhere (iter)) <= 0)
			break;
		sig = ringIterCurrentObject (iter, level);
		/* Abort when we come to another key or subkey. */
		if (OBJISKEY(sig))
			break;
		if (!OBJISSIG(sig))
			continue;
		/* Only count self-sigs that have been validated */
		if (ringSigMaker (set, sig, set) != obj)
			continue;
		if ((ringSigType (set, sig) & 0xf0) != PGP_SIGTYPE_KEY_GENERIC)
			continue;
		if (!ringSigChecked (set, sig))
			continue;
		if (SIGISREVOCABLE(&sig->s))
			continue;

		/* Here we have a nonrevocable self signature */
		p = (PGPByte *)ringFetchObject(set, sig, &len);
		if (!p) {
			if (error)
				*error = ringSetError(set)->error;
			ringIterDestroy (iter);
			return NULL;
		}
		p = (PGPByte *)ringSigFindSubpacket (p, subpacktype, 0, NULL, NULL,
			NULL, &creation, &matches);
		if (p) {
			totalmatches += matches;
			if (nth >= totalmatches-matches && nth < totalmatches) {
				/* This packet has the nth instance */
				skippedmatches = totalmatches-matches;
				bestcreation = creation;
				bestsig = sig;
				ringObjectHold (bestsig);
				/* If don't need to count all matches, done now */
				if (!pmatches)
					break;
			}
		}
	}
	ringIterDestroy (iter);

	pname = ringKeyPrimaryName (obj, set, 0);
	if (pname) {
		sig = ringLatestSigByKey( pname, set, obj );
		if (sig) {
			p = (PGPByte *)ringFetchObject(set, sig, &len);
			if (!p) {
				if (error)
					*error = ringSetError(set)->error;
				return NULL;
			}
			p = (PGPByte *)ringSigFindSubpacket (p, subpacktype, 0, NULL, NULL,
				NULL, &creation, &matches);
			if (p) {
				totalmatches += matches;
				if (nth >= totalmatches-matches && nth < totalmatches) {
					/* This packet has the nth instance */
					skippedmatches = totalmatches-matches;
					bestcreation = creation;
					bestsig = sig;
					ringObjectHold (bestsig);
				}
			}
		}
	}

	if (bestsig) {
		/* This had the sig with the nth instance of the type we need */
		p = (PGPByte *)ringFetchObject(set, bestsig, &bestlen);
		pgpAssert (p);
		/* Note that this may alter the contents of p */
		bestp = ringSigFindSubpacket (p, subpacktype, nth-skippedmatches,
			&bestlen, &bestcritical, &besthashed, NULL, NULL);
		ringObjectRelease (bestsig);
	}
	if (plen)
		*plen = bestlen;
	if (pcritical)
		*pcritical = bestcritical;
	if (phashed)
		*phashed = besthashed;
	if (pcreation)
		*pcreation = bestcreation;
	if (pmatches)
		*pmatches = totalmatches;
	return bestp;
}


/*
 * Find an additional decryption key for the given key, if one exists.
 * nth tells which one to find.  *pkeys is set to the number of add'l
 * decryption keys, *pclass is set to the class byte associated with
 * the decryption key.  *pkalg and *keyid are set to the algorithm and
 * keyid of the nth ADK key.  Returns NULL but no error in *error if
 * the ADK key is not in the specified ringset.  Return *error as
 * kPGPError_ItemNotFound if there are fewer than n+1 ADKs.
 */
union RingObject *
ringKeyAdditionalRecipientRequestKey (RingObject *obj, RingSet const *set,
	unsigned nth, PGPByte *pkalg, PGPKeyID *keyid,
	PGPByte *pclass, unsigned *pkeys, PGPError *error)
{
	RingObject	   *rkey;			/* Additional decryption key */
	PGPByte const  *krpdata;		/* Pointer to key decryption data */
	PGPSize			krdatalen;		/* Length of krdata */
	int				critical;		/* True if decryption field was critical */
	unsigned		matches;		/* Number of adk's found */
	PGPByte			fingerp[20];	/* Fingerprint of adk */
	PGPByte			krdata[22];		/* Copy of key decryption data packet */

	pgpAssert(OBJISKEY(obj));
	pgpAssert(pgpIsRingSetMember(set, obj));
	pgpAssert (error);

	*error	= kPGPError_NoErr;
	if( IsntNull( pkeys ) )
		*pkeys	= 0;
	if( IsntNull( pclass ) )
		*pclass	= 0;
	if( IsntNull( pkalg ) )
		*pkalg = 0;
	if( IsntNull( keyid ) )
	{
		pgpClearMemory( keyid, sizeof( *keyid ) );
	}
	
	krpdata = ringKeyFindSubpacket (obj, set,
		SIGSUB_KEY_ADDITIONAL_RECIPIENT_REQUEST, nth, &krdatalen,
		&critical, NULL, NULL, &matches, error);
	if (!krpdata) {
		if (IsntPGPError(*error))
			*error = kPGPError_ItemNotFound;
		return NULL;
	}
	/*
	 * krdata is 1 byte of class, 1 of pkalg, 20 bytes of fingerprint.
	 * Last 8 of 20 are keyid.  Make a copy because data is volatile when
	 * we do other operations.
	 */

	if (krdatalen < sizeof(krdata)) {
		/* malformed packet, can't use it */
		*error = kPGPError_ItemNotFound;
		return NULL;
	}
	pgpCopyMemory (krpdata, krdata, sizeof(krdata));

	/* Do we have ADK? */
	rkey = ringKeyById8 (set, krdata[1], krdata+2+20-8);
	if (IsntNull (rkey)) {
		if (pgpVirtMaskIsEmpty(&rkey->g.mask)) {
			rkey = NULL;
		} else {
			ringKeyFingerprint20 (set, rkey, fingerp);
			if (memcmp (fingerp, krdata+2, 20) != 0) {
				/* Have a key that matches in keyid but wrong fingerprint */
				rkey = NULL;
			}
		}
	}
	/* Success */
	if (pkeys) {
		*pkeys = matches;
	}
	if (pclass) {
		*pclass = krdata[0];
	}
	if (pkalg) {
		*pkalg = krdata[1];
	}
	if (keyid) {
		pgpNewKeyIDFromRawData( krdata+2+20-8, 8, keyid );
	}
	return rkey;
}

	
/*
 * Find a key revocation key and keyid for the given key.  nth tells
 * which one to find.  *pkeys is set to the number of key revocation
 * keys, *pclass is set to the class byte associated with the
 * revocation key.  Returns NULL but no error in *error if the ADK key
 * is not in the specified ringset.  Return *error as
 * kPGPError_ItemNotFound if there are fewer than n+1 ADKs.  The class
 * byte is intended for future expansion; for now the high order bit
 * is used to indicate a revocation authorization.  Later we could use
 * the other bits to authorize other kinds of signatures, perhaps.
 */
union RingObject *
ringKeyRevocationKey (union RingObject *obj, RingSet const *set, unsigned nth,
	PGPByte *pkalg, PGPKeyID *keyid, PGPByte *pclass, unsigned *pkeys,
	PGPError *error)
{
	RingObject	   *rkey;			/* Message revocation key */
	PGPByte const  *krpdata;		/* Pointer to key revocation data */
	PGPSize			krdatalen;		/* Length of krdata */
	int				critical;		/* True if revocation field was critical */
	unsigned		matches;		/* Number of revkey packets found */
	PGPByte			fingerp[20];	/* Fingerprint of revkey */
	PGPByte			krdata[22];		/* Copy of key revocation data packet */

	pgpAssert(OBJISKEY(obj));
	pgpAssert(pgpIsRingSetMember(set, obj));
	pgpAssert (error);

	*error	= kPGPError_NoErr;
	if( IsntNull( pkeys ) )
		*pkeys	= 0;
	if( IsntNull( pclass ) )
		*pclass	= 0;
	if( IsntNull( pkalg ) )
		*pkalg = 0;
	if( IsntNull( keyid ) )
	{
		pgpClearMemory( keyid, sizeof( *keyid ) );
	}
	
	krpdata = ringKeyFindSubpacket (obj, set, SIGSUB_KEY_REVOCATION_KEY, nth,
		&krdatalen, &critical, NULL, NULL, &matches, error);
	if (!krpdata) {
		if (IsntPGPError(*error))
			*error = kPGPError_ItemNotFound;
		return NULL;
	}
	/*
	 * krdata is 1 byte of class, 1 of pkalg, 20 bytes of fingerprint.
	 * Last 8 of 20 are keyid.  Make a copy because data is volatile when
	 * we do other operations.
	 */

	if (krdatalen < sizeof(krdata)) {
		/* malformed packet, can't use it */
		*error = kPGPError_ItemNotFound;
		return NULL;
	}
	pgpCopyMemory (krpdata, krdata, sizeof(krdata));

	/* Do we have revocation packet? */
	rkey = ringKeyById8 (set, krdata[1], krdata+2+20-8);
	if (IsntNull (rkey) ) {
		if (pgpVirtMaskIsEmpty(&rkey->g.mask)) {
			rkey = NULL;
		} else {
			ringKeyFingerprint20 (set, rkey, fingerp);
			if (memcmp (fingerp, krdata+2, 20) != 0) {
				/* Have a key that matches in keyid but wrong fingerprint */
				rkey = NULL;
			}
		}
	}
	/* Else success */
	if (pkeys) {
		*pkeys = matches;
	}
	if (pclass) {
		*pclass = krdata[0];
	}
	if (pkalg) {
		*pkalg = krdata[1];
	}
	if (keyid) {
		pgpNewKeyIDFromRawData( krdata+2+20-8, 8, keyid );
	}
	return rkey;
}


/*
 * Return true if rkey is a revocation key for key.
 * Includes case where rkey is a dummy key which has issued a signature
 * but is not local to the keyring.  In that case we just check for match
 * on keyid and pkalg.
 */
PGPBoolean
ringKeyIsRevocationKey (RingObject *key, RingSet const *set, RingObject *rkey)
{
	PGPByte				 revClass;
	PGPUInt32			 nRevKeys;
	PGPUInt32			 iRevKeys;
	PGPByte				 revAlg;
	PGPByte				 rKeyAlg;
	PGPKeyID			 revKeyID;
	PGPKeyID			 rKeyID;
	PGPBoolean			 nonLocal;
	PGPError			 error;

	nonLocal = pgpVirtMaskIsEmpty(&rkey->g.mask);
	if (nonLocal) {
		ringKeyID8 (set, rkey, &rKeyAlg, &rKeyID);
	}

	nRevKeys = 1;
	iRevKeys = 0;
	while (iRevKeys < nRevKeys) {
		RingObject const *revkey;
		revkey = ringKeyRevocationKey(key, set, iRevKeys++, &revAlg, &revKeyID,
									  &revClass, &nRevKeys, &error);
		if (IsPGPError(error))
			break;
		if (!(revClass & 0x80))
			continue;
		if (nonLocal) {
			if (rKeyAlg == revAlg &&
				0 == PGPCompareKeyIDs (&rKeyID, &revKeyID))
				return TRUE;
		} else if (revkey == rkey) {
			return TRUE;
		}
	}
	return FALSE;
}
	

/*
 * True if there is a third party revocation signature on the given
 * key.  Return the key (if local) and the keyid of the revoking key.
 * Note that this requires that there be both a revoking authorization
 * on the revoker key and a revocation issued by that key.
 */
PGPBoolean
ringKeyHasThirdPartyRevocation (RingObject *obj, RingSet const *set,
	RingObject **revkey, PGPByte *pkalg, PGPKeyID *keyid, PGPError *error)
{
	RingObject		*topkey;
	RingObject		*sig;
	PGPByte			 rKeyAlg;
	PGPKeyID		 rKeyID;

	pgpAssert(OBJISKEY(obj));
	pgpAssert(pgpIsRingSetMember(set, obj));
	pgpAssert (error);

	*error	= kPGPError_NoErr;
	if( IsntNull( revkey ) )
		*revkey = NULL;
	if( IsntNull( pkalg ) )
		*pkalg = 0;
	if( IsntNull( keyid ) )
	{
		pgpClearMemory( keyid, sizeof( *keyid ) );
	}

	/* May be called on subkey, so find top level key */
	topkey = obj;
	if (OBJISSUBKEY(topkey))
		topkey = ringKeyMasterkey (set, topkey );
	
	/* Search for a revocation signature on this key */
	for (sig=obj->g.down; sig; sig=sig->g.next) {
		if (!OBJISSIG(sig))
			continue;
		if (sig->s.type != ( (topkey==obj) ? PGP_SIGTYPE_KEY_REVOKE
										   : PGP_SIGTYPE_KEY_SUBKEY_REVOKE ))
			continue;
		if (sig->s.by == topkey)
			continue;

		if (ringKeyIsRevocationKey (topkey, set, sig->s.by))
			break;
	}

	if (IsNull (sig)) {
		/* No luck */
		return FALSE;
	}

	if( IsntNull( revkey ) ) {
		if (!pgpVirtMaskIsEmpty (&sig->s.by->g.mask))
		*revkey = sig->s.by;
	}

	ringKeyID8 (set, sig->s.by, &rKeyAlg, &rKeyID);
	
	if (pkalg) {
		*pkalg = rKeyAlg;
	}
	if (keyid) {
		*keyid = rKeyID;
	}
	return TRUE;
}


/*
 * Create a SigSpec structure which represents the given signature.
 * We must have the signing key as a private key.  We would have to
 * change the initialization of pgpSigSpec to avoid this, but normally
 * the reason for doing this is to re-issue a modified signature, so it
 * is a reasonable restriction.
 */
PGPSigSpec *
ringSigSigSpec (RingObject *sig, RingSet const *set,
	PGPError *error)
{
	PGPContextRef cdkContext;
	PGPEnv *env;
	PGPSecKey *signkey;
	PGPByte *p;
	PGPSigSpec *spec;
	PGPSize len;

	(void)set;
	pgpAssert(OBJISSIG(sig));
	pgpAssert(pgpIsRingSetMember(set, sig));
	pgpAssert( IsntNull( error ) );
	*error = kPGPError_NoErr;

	cdkContext = set->pool->context;
	env = pgpContextGetEnvironment( cdkContext );

	/* Get signing key - note that this corrupts ringFetchObject buffer */
	signkey = ringSecSecKey (set, sig->s.by, PGP_PKUSE_SIGN);
	if (!signkey) {
		*error = ringSetError(set)->error;
		return NULL;
	}

	spec = pgpSigSpecCreate (env, signkey, sig->s.type);
	if( IsNull( spec ) ) {
		*error = kPGPError_OutOfMemory;
		return NULL;
	}

	/* Fetch data for sig */
	p = (PGPByte *)ringFetchObject(set, sig, &len);
	if( IsNull( p ) ) {
		pgpSigSpecDestroy (spec);
		pgpSecKeyDestroy (signkey);
		*error = ringSetError(set)->error;
		return NULL;
	}

	pgpSigSpecSetHashtype (spec, sig->s.hashalg);
	pgpSigSpecSetVersion (spec, p[0]);
	pgpSigSpecSetTimestamp (spec, sig->s.tstamp);

	/* Incorporate data from subpackets if present */
	if (pgpSigSpecVersion(spec) == PGPVERSION_4) {
		*error = ringSigSubpacketsSpec (spec, p);
		if (IsPGPError(*error)) {
			pgpSigSpecDestroy (spec);
			pgpSecKeyDestroy (signkey);
			return NULL;
		}
	}
	return spec;
}


/*
 * Check set to see if any objects expire within the time window.
 * Return TRUE as a conservative answer in case of an error.
 */
PGPBoolean
ringSetHasExpiringObjects( RingSet const *set, PGPTime time1, PGPTime time2 )
{
	RingIterator *iter;
	int			  level;
	PGPTime		  exptime;

	iter = ringIterCreate (set);
	if (!iter)
		return FALSE;

	while ((level = ringIterNextObjectAnywhere (iter)) > 0) {
		RingObject *obj = ringIterCurrentObject( iter, level );
		int objtype = ringObjectType( obj );

		if( objtype == RINGTYPE_KEY ) {
			if (obj->k.tstamp && obj->k.validity) {
				exptime = obj->k.tstamp + obj->k.validity*60*60*24;
				if (exptime >= time1 && exptime < time2) {
					ringIterDestroy( iter );
					return TRUE;
				}
			}
		} else if( objtype == RINGTYPE_SIG ) {
			if( obj->s.tstamp && obj->s.sigvalidity ) {
				exptime = obj->s.tstamp + obj->s.sigvalidity;
				if (exptime >= time1 && exptime < time2) {
					ringIterDestroy( iter );
					return TRUE;
				}
			}
		}
	}
	ringIterDestroy( iter );
	return FALSE;
}


/* Check obj for consistency, make sure valid mask bits are in
 * parent mask
 */
static void
ringObjCheck (RingPool *pool, union RingObject *obj, PGPVirtMask *validmask,
			  PGPVirtMask *parentmask)
{
	PGPVirtMask mask;
   
	pgpVirtMaskInit (pool, &mask);
	pgpVirtMaskCopy (pool, &obj->g.mask, &mask);
	pgpVirtMaskAND (pool, validmask, &mask);
	pgpVirtMaskANDNOT (pool, parentmask, &mask);
	if (!pgpVirtMaskIsEmpty(&mask)) {
		pgpDebugMsg( "Child ringobj set not subset of parent set" );
	}

	if (OBJISSIG(obj)) {
	    /*  sig should point to top-level key */ 
		if (!OBJISTOPKEY(obj->s.by)) {
			pgpDebugMsg( "Signature by other than top-level key" );
		}
	}
	if (!OBJISBOT(obj)) {
		for (obj=obj->g.down; obj; obj=obj->g.next) {
			pgpVirtMaskCopy (pool, parentmask, &mask);
			pgpVirtMaskAND (pool, &obj->g.mask, &mask);
			ringObjCheck (pool, obj, validmask, &mask);
		}
	}
	pgpVirtMaskCleanup (pool, &mask);
}

/* Perform a consistency check on ring pool data structures */
void
ringPoolConsistent (RingPool *pool, int *pnsets, int *pnfiles)
{
	PGPVirtMask allocmask;
	PGPVirtMask mask;
	RingSet *set;
	union RingObject *obj;
	PGPInt32 bit;

	pgpVirtMaskInit (pool, &allocmask);
	pgpVirtMaskInit (pool, &mask);

	for (set = pool->sets; set; set = set->next)
		pgpVirtMaskOR (pool, &set->mask, &allocmask);

	pgpAssert (!pgpVirtMaskIsOverlapping (&allocmask, &pool->filemask));

	pgpVirtMaskCopy (pool, &allocmask, &mask);
	pgpVirtMaskOR (pool, &pool->filemask, &mask);

	for (obj=pool->keys; obj; obj=obj->g.next) {
		ringObjCheck (pool, obj, &mask, &mask);
	}
	if (pnsets) {
		int nsets = 0;
		while (!pgpVirtMaskIsEmpty(&allocmask)) {
			++nsets;
			bit = pgpVirtMaskLSBit(&allocmask);
			pgpVirtMaskClearBit (pool, &allocmask, bit);
		}
		*pnsets = nsets;
	}
	pgpVirtMaskCopy (pool, &pool->filemask, &mask);
	if (pnfiles) {
		int nfiles = 0;
		while (!pgpVirtMaskIsEmpty(&mask)) {
			++nfiles;
			bit = pgpVirtMaskLSBit(&mask);
			pgpVirtMaskClearBit (pool, &mask, bit);
		}
		*pnfiles = nfiles;
	}

	pgpVirtMaskCleanup (pool, &allocmask);
	pgpVirtMaskCleanup (pool, &mask);
}
