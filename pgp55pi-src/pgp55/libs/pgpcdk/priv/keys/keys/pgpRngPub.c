/*
 * pgpRngPub.c - keyring management public functions.
 *
 * Written by Colin Plumb.
 *
 * $Id: pgpRngPub.c,v 1.43 1997/10/17 06:15:08 hal Exp $
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

#ifndef NULL
#define NULL 0
#endif

/*
 * The four type bits are encoded as follows:
 * 76543210
 *   1 - Name (n)
 *   01 - Signature (s)
 *   001 - Key (k)
 *   0001 - Secret (We use the letter "c" for this)
 *   0000 - Unknown keyring object
 */
int
ringObjectType(union RingObject const *obj)
{
	int const types[16] = {
		RINGTYPE_UNK, RINGTYPE_SEC, RINGTYPE_KEY, RINGTYPE_KEY,
		RINGTYPE_SIG, RINGTYPE_SIG, RINGTYPE_SIG, RINGTYPE_SIG,
		RINGTYPE_NAME, RINGTYPE_NAME, RINGTYPE_NAME, RINGTYPE_NAME,
		RINGTYPE_NAME, RINGTYPE_NAME, RINGTYPE_NAME, RINGTYPE_NAME
	};

	pgpAssert (obj);
	return types[obj->g.flags/RINGOBJF_SEC & 15];
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

	while (obj->g.mask & set->mask) {
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
		pgpAssert(obj->g.mask & iter->set.mask);
		obj = obj->g.next;
	} else {
		/* Going down a level */
		pgpAssert(level == iter->level+1);

		if (level > 1) {
			obj = iter->stack[level-2];
			pgpAssert(obj);
			pgpAssert(obj->g.mask & iter->set.mask);
			if (OBJISBOT(obj))
				return 0;
			obj = obj->g.down;
		} else {
			obj = iter->set.pool->keys;
		}
		iter->level = level;
	}

	/* Search for the next item of interest */
	while (obj && !(obj->g.mask & iter->set.mask))
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
		pgpAssert(obj->g.mask & iter->set.mask);
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
		if (obj->g.mask & iter->set.mask)
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
	ringmask mask = iter->set.mask;

	pgpAssert(iter);
	pgpAssert(iter->set.type == RINGSET_ITERATOR);

	/* Find first object to be considered */
	if (!level) {
		level = 1;
		obj = iter->set.pool->keys;
	} else {
		obj = iter->stack[level-1];

		if (obj) {
			pgpAssert(obj->g.mask & mask);
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
			if (obj->g.mask & mask) {
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

	if (!(obj->g.mask & iter->set.mask))
		return 0;	/* Not a member */

	/* A bit ad-hoc; there is a general way. */
	if (OBJISTOP(obj)) {
		iter->stack[0] = obj;
		level = 1;
	} else {
		p = obj->g.up;
		pgpAssert(p->g.mask & iter->set.mask);
		if (OBJISTOP(p)) {
			iter->stack[0] = p;
			iter->stack[1] = obj;
			level = 2;
		} else {
			pp = p->g.up;
			pgpAssert(pp->g.mask & iter->set.mask);
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
ringSetCountList(union RingObject const *obj, ringmask mask,
                 unsigned *counts, unsigned depth)
{
	while (obj) {
		if (obj->g.mask & mask) {
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
	if (set && set->mask && depth > 0)
		ringSetCountList(set->pool->keys, set->mask, counts, depth-1);
	return 0;
}

static void
ringSetCountTypesList(union RingObject const *obj, ringmask mask,
                      unsigned *counts, unsigned max)
{
	int t;

	while (obj) {
		if (obj->g.mask & mask) {
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
	if (set && set->mask && max > 0)
		ringSetCountTypesList(set->pool->keys, set->mask, counts, max);
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
	/* set->mask and set->type uninitialized */

	return set;
}

RingSet *
ringSetCreate(RingPool *pool)
{
	ringmask mask;
	RingSet *set;
	int bit;

	if (!pool)
		return NULL;

	/* Allocate a new bit */
	bit = ringBitAlloc(pool);
	if (bit < 0)
		return NULL;

	mask = (ringmask)1<<bit;

	/* Allocate the structure */
	set = ringSetAlloc(pool);
	if (set) {
		set->mask = mask;
		set->type = RINGSET_MUTABLE;
		pool->allocmask |= mask;
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
		set->mask = s->mask;
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
		set->mask |= s2->mask;
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
	ringmask mask = set->mask;

	pgpAssert(RINGSETISMUTABLE(set));
	pgpAssert(!(mask & (mask-1)));

	if (obj && !(obj->g.mask & mask)) {
		obj->g.mask |= mask;
		/* Ensure all parents are added, too. */
		while (!OBJISTOP(obj)) {
			obj = obj->g.up;
			if (obj->g.mask & mask)
				break;
			obj->g.mask |= mask;
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
	pgpAssert(!(set->mask & (set->mask-1)));

	/*
	 * Remove this object and all its children.
	 * As an optimization, omit scanning children if the
	 * object is not already in the set.
	 */
	if (obj && obj->g.mask & set->mask) {
		if (!OBJISBOT(obj)) {
			union RingObject *obj2 = obj;
			for (obj2 = obj2->g.down; obj2; obj2 = obj2->g.next)
				ringSetRemObject(set, obj2);
		}
		obj->g.mask &= ~set->mask;
		ringGarbageCollectObject(set->pool, obj);
	}
	return 0;
}

/* Helper function for ringSetAddSet */
static void
ringSetAddList(union RingObject *obj, ringmask destmask, ringmask srcmask)
{
	while (obj) {
		if (obj->g.mask & srcmask) {
			obj->g.mask |= destmask;
			if (!OBJISBOT(obj))
				ringSetAddList(obj->g.down, destmask, srcmask);
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

		ringSetAddList(set->pool->keys, set->mask, set2->mask);
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
	ringmask mask = set->mask;

	pgpAssert(RINGSETISMUTABLE(set));
	pgpAssert(mask);

	if (set2 && set2->mask) {
		pgpAssert(set->pool == set2->pool);
		for (obj = set->pool->keys; obj; obj = obj->g.next) {
			if (obj->g.mask & mask && obj->g.mask & set2->mask) {
				obj->g.mask &= ~mask;
				if (!OBJISBOT(obj))
					ringClearMask(set->pool, &obj->g.down,
					              ~mask);
			}
		}
	}
	return 0;
}

static int
ringSetIntersectList(union RingObject *obj, ringmask m1, ringmask m2,
                     ringmask mdest)
{
	int flag = 0;

	while (obj) {
		if ((obj->g.mask & m1) && (obj->g.mask & m2)) {
			flag = 1;
			obj->g.mask |= mdest;
			if (!OBJISBOT(obj))
				ringSetIntersectList(obj->g.down,
				                     m1, m2, mdest);
		}
		obj = obj->g.next;
	}
	return flag;
}

RingSet *
ringSetIntersection(RingSet const *s1, RingSet const *s2)
{
	RingSet *set;

	if (!s1 || !s2)
		return (RingSet *)NULL;

	pgpAssert(s1->pool == s2->pool);
	pgpAssert(!RINGSETISMUTABLE(s1));
	pgpAssert(!RINGSETISMUTABLE(s2));

	/* Do a few trivial cases without allocating bits. */
	if (!(s1->mask & ~s2->mask))	/* s1 is a subset of s2 - copy s1 */
		return ringSetCopy(s1);
	if (!(s2->mask & ~s1->mask))	/* s2 is a subset of s1 - copy s2 */
		return ringSetCopy(s2);

	set = ringSetCreate(s1->pool);
	if (set) {
		if (!ringSetIntersectList(s1->pool->keys, s1->mask, s2->mask,
		                          set->mask))
		{
			/* Empty set - free bit */
			s1->pool->allocmask &= ~set->mask;
			set->mask = 0;
		}
		set->type = RINGSET_IMMUTABLE;
	}
	return set;
}

static void
ringSetDiffList(union RingObject *obj, ringmask m1, ringmask m2,
                ringmask mdest)
{
	while (obj) {
		if ((obj->g.mask & m1) && !(obj->g.mask & m2)) {
			obj->g.mask |= mdest;
			if (!OBJISBOT(obj))
				ringSetDiffList(obj->g.down, m1, m2, mdest);
		}
		obj = obj->g.next;
	}
}

/* Return s1-s2. */
RingSet *
ringSetDifference(RingSet const *s1, RingSet const *s2)
{
	RingSet *set;

	if (!s1)
		return NULL;
	if (!s2 || !s2->mask)
		return ringSetCopy(s1);

	pgpAssert(s1->pool == s2->pool);
	pgpAssert(!RINGSETISMUTABLE(s1));
	pgpAssert(!RINGSETISMUTABLE(s2));

	if (!(s1->mask & ~s2->mask)) {
		/* s1->mask is a subset of s2->mask, so result is empty */
		set = ringSetAlloc(s1->pool);
		if (set) {
			set->mask = 0;
			set->type = RINGSET_IMMUTABLE;
		}
	} else {
		set = ringSetCreate(s1->pool);
		if (set) {
			ringSetDiffList(s1->pool->keys, s1->mask, s2->mask,
					set->mask);
			set->type = RINGSET_IMMUTABLE;
		}
	}
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
		sizeof(*pool), kPGPMemoryFlags_Clear );
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
			if (!(key->mask & set->mask))
				break;
			ringObjectHold((union RingObject *)key);
			return (union RingObject *)key;
		}
	}
	/* Failed */
	return NULL;
}

/*** Access functions for information about objects ***/

int
ringKeyError(RingSet const *set, union RingObject *key)
{
	PGPByte const *p;
	PGPSize len;

	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);

	if (!(key->g.flags & KEYF_ERROR))
		return 0;

	p = (PGPByte const *)ringFetchObject(set, key, &len);
	if (!p)
		return ringSetError(set)->error;
	return ringKeyParse(set->pool->context, p, len, NULL, NULL, NULL, NULL,
						NULL, 0);
}

unsigned
ringKeyBits(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
	(void)set;
	return key->k.keybits;
}

PGPUInt32
ringKeyCreation(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
	(void)set;
	return key->k.tstamp;
}

PGPUInt32
ringKeyExpiration(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
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
 */
int
ringKeyUse(RingSet const *set, union RingObject *key)
{
	int use;

	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);

	use = pgpKeyUse(pgpPkalgByNumber(key->k.pkalg));
	if (OBJISSUBKEY(key))
		use &= PGP_PKUSE_ENCRYPT;
	for (key=key->g.down; key; key=key->g.next) {
		if (OBJISSUBKEY(key) && ringSubkeyValid(set, key))
			use |= pgpKeyUse(pgpPkalgByNumber(key->k.pkalg));
	}
	return use;
}

PGPByte
ringKeyTrust(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
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
	pgpAssert(key->g.mask & set->mask);
	pgpAssert(trust==kPGPKeyTrust_Unknown  || trust==kPGPKeyTrust_Never ||
	       trust==kPGPKeyTrust_Marginal || trust==kPGPKeyTrust_Complete);
	pgpAssert (!(key->k.trust & PGP_KEYTRUSTF_BUCKSTOP));
	if (key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
	    return;
	if ((key->k.trust & kPGPKeyTrust_Mask) != trust) {
		key->k.trust = (key->k.trust & ~kPGPKeyTrust_Mask) + trust;
		key->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, key->g.mask);
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
    pgpAssert(OBJISKEY(key));
    pgpAssert(key->g.mask & set->mask);
    if (!ringKeyIsSec (set, key) || 
	key->k.trust & (PGP_KEYTRUSTF_BUCKSTOP | PGP_KEYTRUSTF_REVOKED))
        return;        /* already axiomatic or can't set */
    key->k.trust &= ~kPGPKeyTrust_Mask;
    key->k.trust |= (PGP_KEYTRUSTF_BUCKSTOP | kPGPKeyTrust_Ultimate);
    ringPoolMarkTrustChanged (set->pool, key->g.mask);
    key->g.flags |= (RINGOBJF_TRUSTCHANGED | RINGOBJF_TRUST);
}


/*  Reset an axiomatic key.  Trust is set to undefined. */

void
ringKeyResetAxiomatic (RingSet const *set, union RingObject *key)
{
    pgpAssert(OBJISKEY(key));
    pgpAssert(key->g.mask & set->mask);
    if (!(key->k.trust & PGP_KEYTRUSTF_BUCKSTOP))
        return;           /* not axiomatic */
    key->k.trust &= ~PGP_KEYTRUSTF_BUCKSTOP;
    key->k.trust = (key->k.trust & ~kPGPKeyTrust_Mask) + 
                           kPGPKeyTrust_Undefined;
    ringPoolMarkTrustChanged (set->pool, key->g.mask);
    key->g.flags |= (RINGOBJF_TRUSTCHANGED | RINGOBJF_TRUST);
}

int
ringKeyAxiomatic(RingSet const *set, union RingObject *key)
{
	(void)set;	/* Avoid warning */
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
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
	pgpAssert(key->g.mask & set->mask);
	return key->k.trust & PGP_KEYTRUSTF_DISABLED;
}

void
ringKeyDisable(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
	if (!(key->k.trust & PGP_KEYTRUSTF_DISABLED)) {
		key->k.trust |= PGP_KEYTRUSTF_DISABLED;
		key->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, key->g.mask);
	}
	key->g.flags |= RINGOBJF_TRUST;
}

void
ringKeyEnable(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
	if (key->k.trust & PGP_KEYTRUSTF_DISABLED) {
		key->k.trust &= ~PGP_KEYTRUSTF_DISABLED;
		key->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, key->g.mask);
	}
	key->g.flags |= RINGOBJF_TRUST;
}



int
ringKeyRevoked(RingSet const *set, union RingObject *key)
{
	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
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
 * UID_REVOKE.
 */
int
ringSigRevoked (RingSet const *set, union RingObject *sig)
{
	RingObject		*parent,
					*sibling;

	(void)set;	/* Avoid warning */
	pgpAssert (OBJISSIG(sig));
	pgpAssert (sig->g.mask & set->mask);

	/* Sigs can be declared irrevokable at creation time */
	if (!SIGISREVOKABLE(&sig->s))
		return FALSE;

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
	pgpAssert(key->g.mask & set->mask);
	(void)set;
	if (!(key->g.flags & (RINGOBJF_TRUST)))
		ringMntValidateKey (set, key);
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
	pgpAssert(key->g.mask & set->mask);
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

int
ringKeyFingerprint16(RingSet const *set, union  RingObject *key,
	PGPByte *buf)
{
	PGPByte const *p;
	PGPSize len;

	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);

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
	pgpAssert(key->g.mask & set->mask);

	objbuf = (PGPByte const *)ringFetchObject(set, key, &objlen);
	return pgpFingerprint20HashBuf(set->pool->context, objbuf, objlen, buf);
}

int
ringKeyAddSigsby(RingSet const *set, union RingObject *key,
	RingSet *dest)
{
	RingSig *sig;
	int i = 0;

	pgpAssert(OBJISKEY(key));
	pgpAssert(key->g.mask & set->mask);
	pgpAssert(RINGSETISMUTABLE(dest));

	for (sig = &key->k.sigsby->s; sig; sig = sig->nextby) {
		if (sig->mask & set->mask) {
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
	pgpAssert(key->g.mask & set->mask);

	for (key = key->g.down; key; key = key->g.next)
		if ((key->g.mask & set->mask) && OBJISSEC(key))
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
	ringmask keyfilemask = key->g.mask & pool->filemask;
	ringmask secfilemask = 0;

	if (keyfilemask == MEMRINGMASK)
		return 0;			/* Newly generated keys */

	for (key = key->g.down; key; key = key->g.next) {
		if ((key->g.mask & set->mask) && OBJISSEC(key)) {
			secfilemask = key->g.mask & pool->filemask;
		}
	}
	return !(keyfilemask & ~secfilemask);
}


/*
 * Return the most recent subkey associated with the key, if there is one.
 */
union RingObject *
ringKeySubkey(RingSet const *set, union RingObject const *key)
{
	ringmask mask = set->mask;
	union RingObject *obj, *best = NULL;
	PGPUInt32 objtime, besttime = 0;

	pgpAssert(OBJISKEY(key));
	for (obj = key->g.down; obj; obj = obj->g.next) {
		if ((obj->g.mask & mask) && OBJISSUBKEY(obj)
		    && ringSubkeyValid(set, obj)) {
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
	pgpAssert(key->g.mask & set->mask);

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
								   NULL, 0);
		ringSimpleErr(set->pool, i);
		return NULL;
	}
	/*
	 * A key starts with 5 or 7 bytes of data, an algorithm byte, and
	 * the public components.
	 */
	if (p[0] == PGPVERSION_3) {
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
	pgpAssert(sec->g.mask & set->mask);

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
								   NULL, NULL, NULL, 0);
		ringSimpleErr(set->pool, i);
		return NULL;
	}
	if (p[0] == PGPVERSION_3) {
		vsize = 0;
	} else {
		vsize = 2;
	}
	pgpAssert(p[5+vsize] == sec->g.up->k.pkalg); /* Checked by ringKeyVerify */
	seckey = pgpSecKeyFromBuf( context, p[5+vsize], p+6+vsize, len-6-vsize, &i);
	if (!seckey) {
 		ringSimpleErr(set->pool, i);
		return NULL;
	}
	memcpy(seckey->keyID, sec->g.up->k.keyID, sizeof(sec->g.up->k.keyID));
	return seckey;
}


/* There ain't much to know about a name... */
char const *
ringNameName(RingSet const *set, union RingObject *name, PGPSize *lenp)
{
	pgpAssert(OBJISNAME(name));
	return (char const *)ringFetchObject(set, name, lenp);
}

/*  Return the validity (*not* the trust) of a name */

PGPByte
ringNameTrust(RingSet const *set, union RingObject *name)
{
    union RingObject *key;
	
    pgpAssert(OBJISNAME(name));
    pgpAssert(name->g.mask & set->mask);
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
	pgpAssert(name->g.mask & set->mask);
	(void)set;
	return name->n.trust & PGP_NAMETRUSTF_WARNONLY;
}

void
ringNameSetWarnonly(RingSet const *set, union RingObject *name)
{
	pgpAssert(OBJISNAME(name));
	pgpAssert(name->g.mask & set->mask);
	if (!(name->n.trust & PGP_NAMETRUSTF_WARNONLY)) {
		name->n.trust |= PGP_NAMETRUSTF_WARNONLY;
		name->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, name->g.mask);
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
    pgpAssert(name->g.mask & set->mask);
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
	pgpAssert(name->g.mask & set->mask);
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
	pgpAssert(name->g.mask & set->mask);
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
	pgpAssert(name->g.mask & set->mask);

	confidence = (PGPUInt16) ringTrustToExtern (confidence);
	
	if (!(name->n.flags2 & NAMEF2_NEWTRUST) || 
	    name->n.confidence != confidence) {
		name->n.confidence = (PGPByte) confidence;
		name->n.flags2 |= NAMEF2_NEWTRUST; 
		name->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringPoolMarkTrustChanged (set->pool, name->g.mask);
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
	pgpAssert(sig->g.mask & set->mask);

	if (!(sig->g.flags & SIGF_ERROR))
		return 0;

	p = (PGPByte const *)ringFetchObject(set, sig, &len);
	if (!p)
		return ringSetError(set)->error;
	return ringSigParse(p, len, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

union RingObject *
ringSigMaker(RingSet const *sset, union RingObject *sig,
	RingSet const *kset)
{
	(void)sset;	/* Avoid warning */
	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & sset->mask);

	sig = sig->s.by;
	pgpAssert(OBJISKEY(sig));	/* "sig" is now a key! */
	if (!(sig->g.mask & kset->mask))
		return NULL;
	ringObjectHold(sig);
	return sig;
}

void
ringSigID8(RingSet const *set, union RingObject const *sig,
	PGPByte *pkalg, PGPKeyID *keyID)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & set->mask);
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
	pgpAssert(sig->g.mask & set->mask);
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
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	return sig->s.trust & PGP_SIGTRUSTF_CHECKED;
}

int
ringSigTried(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	return sig->s.trust & PGP_SIGTRUSTF_TRIED;
}

int
ringSigExportable(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	return SIGISEXPORTABLE(&sig->s);
}

PGPByte
ringSigTrustLevel(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	return sig->s.trustLevel;
}

PGPByte
ringSigTrustValue(RingSet const *set, union RingObject const *sig)
{
	PGPByte trustValue;

	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & set->mask);
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
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	if (sig->s.by != NULL) {
		if (set->mask & sig->s.by->g.mask) {
			if (!(sig->s.by->g.flags & (RINGOBJF_TRUST)))
				ringMntValidateKey (set, sig->s.by);
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
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	return sig->s.type;
}

PGPUInt32
ringSigTimestamp(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	return sig->s.tstamp;
}

PGPUInt32
ringSigExpiration(RingSet const *set, union RingObject const *sig)
{
	pgpAssert(OBJISSIG(sig));
	pgpAssert(sig->g.mask & set->mask);
	(void)set;
	if (sig->s.tstamp == 0 || sig->s.validity == 0)
		return 0;    /* valid indefinitely */
	else
		return sig->s.tstamp + (sig->s.validity * 3600 * 24);
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
	pgpAssert(sig->g.mask & set->mask);

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
				obj->g.mask |= dest->mask;
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
		while (isxdigit(string[i]) && ++i != 2+16)
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
 * semantics have changed from earlier versions in order to facilitate
 * dynamically adding new packets without invalidating old ones.  All
 * non-revoked sigs which contain packets of the type we are
 * interested in get counted.  pmatches is filled in with the total
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
	RingObject		*sig, *bestsig;
	RingIterator	*iter;
	PGPUInt32		bestcreation;
	PGPUInt32		totalmatches;
	PGPUInt32		skippedmatches;
	PGPByte const	*p, *bestp;
	PGPSize			bestlen;
	int				bestcritical;
	int				besthashed;

	pgpAssert(OBJISKEY(obj));
	pgpAssert (obj->g.mask & set->mask);
	
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
	while (TRUE) {
		int		level;
		PGPUInt32	creation;
		uint		matches;
		PGPSize	len;

		if ((level = ringIterNextObjectAnywhere (iter)) <= 0)
			break;
		sig = ringIterCurrentObject (iter, level);
		/*
		 * Abort when we come to a subkey or another key.  Sigs on subkeys
		 * shouldn't count, I don't think.
		 */
		if (OBJISKEY(sig))
			break;
		if (!OBJISSIG(sig))
			continue;
		/* Only count self-sigs that have been validated */
		if (ringSigMaker (set, sig, set) != obj)
			continue;
		if ((ringSigType (set, sig) & 0xf0) != PGP_SIGTYPE_KEY_GENERIC)
			continue;
		if (!ringSigChecked (set, sig) || ringSigRevoked (set, sig))
			continue;
		/* Here we have a self signature */
		p = (PGPByte const *)ringFetchObject(set, sig, &len);
		if (!p) {
			if (error)
				*error = ringSetError(set)->error;
			return NULL;
		}
		p = ringSigFindSubpacket (p, subpacktype, 0, NULL, NULL,
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
	if (bestsig) {
		/* This had the sig with the nth instance of the type we need */
		p = (PGPByte const *)ringFetchObject(set, bestsig, &bestlen);
		pgpAssert (p);
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
 * decryption keys, *pclass is set to the class byte associated with the
 * decryption key.  Return *error as kPGPError_ItemNotFound if no ADK
 * exists.
 */
union RingObject *
ringKeyAdditionalRecipientRequestKey (union RingObject *obj, RingSet const *set,
	unsigned nth, PGPByte *pclass, unsigned *pkeys, PGPError *error)
{
	RingObject	   *rkey;			/* Additional decryption key */
	PGPByte const  *krpdata;		/* Pointer to key decryption data */
	PGPSize			krdatalen;		/* Length of krdata */
	int				critical;		/* True if decryption field was critical */
	unsigned		matches;		/* Number of adk's found */
	PGPByte			fingerp[20];	/* Fingerprint of adk */
	PGPByte			krdata[22];		/* Copy of key decryption data packet */

	pgpAssert(OBJISKEY(obj));
	pgpAssert (obj->g.mask & set->mask);
	pgpAssert (error);

	*error	= kPGPError_NoErr;
	if( IsntNull( pkeys ) )
		*pkeys	= 0;
	if( IsntNull( pclass ) )
		*pclass	= 0;
	
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
	if (!rkey) {
		/* Have a problem - we don't have the KR key */
		*error = kPGPError_AdditionalRecipientRequestKeyNotFound;
		return NULL;
	}
	ringKeyFingerprint20 (set, rkey, fingerp);
	if (memcmp (fingerp, krdata+2, 20) != 0) {
		/* Have a bogus key that matches in keyid but wrong fingerprint */
		*error = kPGPError_AdditionalRecipientRequestKeyNotFound;
		return NULL;
	}
	/* Else success */
	if (pkeys) {
		*pkeys = matches;
	}
	if (pclass) {
		*pclass = krdata[0];
	}
	return rkey;
}
	
/*
 * Find an additional decryption keyID for the given key, if one
 * exists.  nth tells which one to find.  *pkeys is set to the number
 * of additional decryption keys, *pclass is set to the class byte associated
 * with the decryption key packet.  *pkalg and *keyid are filled in with the
 * algorithm and keyid.  Returns error kPGPError_ItemNotFound if <= n
 * ADK's exist.
 */
PGPError
ringKeyAdditionalRecipientRequestKeyID8 (union RingObject *obj,
	RingSet const *set, unsigned nth, PGPByte *pclass, PGPUInt32 *pkeys,
	PGPByte *pkalg, PGPKeyID *keyid)
{
	PGPByte const  *krpdata;		/* Pointer to key decryption data */
	PGPSize			krdatalen;		/* Length of krdata */
	int				critical;		/* True if decryption field was critical */
	unsigned		matches;		/* Number of adk's found */
	PGPError		error = kPGPError_NoErr;

	pgpAssert(OBJISKEY(obj));
	pgpAssert (obj->g.mask & set->mask);
	pgpAssert (IsntNull(pclass) || IsntNull(pkeys) || IsntNull(pkalg) ||
			   IsntNull(keyid));

	if( IsntNull(pkeys ) )
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
		&critical, NULL, NULL, &matches, &error);
	if (!krpdata) {
		if (IsntPGPError(error))
			error = kPGPError_ItemNotFound;
		return error;
	}

	/*
	 * krdata is 1 byte of class, 1 of pkalg, 20 bytes of fingerprint.
	 * Last 8 of 20 are keyid.
	 */
	if (krdatalen < 22) {
		/* malformed packet, can't use it */
		return kPGPError_ItemNotFound;
	}

	/* Success, can start filling in return parameters */
	if (pkeys) {
		*pkeys = matches;
	}
	if (pclass) {
		*pclass = krpdata[0];
	}
	if (pkalg) {
		*pkalg = krpdata[1];
	}
	if (keyid)
	{
		pgpNewKeyIDFromRawData( krpdata+2+20-8, 8, keyid );
	}

	return kPGPError_NoErr;
}
	

/*
 * Find a key revocation key for the given key, if one exists.  nth tells
 * which one to find.  *pkeys is set to the number of key revocation keys,
 * *pclass is set to the class byte associated with the revocation key.
 * Return *error as kPGPError_ItemNotFound if no revocation key exists.
 * The class byte is intended for future expansion; for now the high order
 * bit is used to indicate a revocation authorization.  Later we could use
 * the other bits to authorize other kinds of signatures, perhaps.
 */
union RingObject *
ringKeyRevocationKey (union RingObject *obj, RingSet const *set, unsigned nth,
					PGPByte *pclass, unsigned *pkeys, PGPError *error)
{
	RingObject	   *rkey;			/* Message revocation key */
	PGPByte const  *krpdata;		/* Pointer to key revocation data */
	PGPSize			krdatalen;		/* Length of krdata */
	int				critical;		/* True if revocation field was critical */
	unsigned		matches;		/* Number of revkey packets found */
	PGPByte			fingerp[20];	/* Fingerprint of revkey */
	PGPByte			krdata[22];		/* Copy of key revocation data packet */

	pgpAssert(OBJISKEY(obj));
	pgpAssert (obj->g.mask & set->mask);
	pgpAssert (error);

	*error	= kPGPError_NoErr;
	if( IsntNull( pkeys ) )
		*pkeys	= 0;
	if( IsntNull( pclass ) )
		*pclass	= 0;
	
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
	if (!rkey) {
		/* Have a problem - we don't have the KR key */
		*error = kPGPError_RevocationKeyNotFound;
		return NULL;
	}
	ringKeyFingerprint20 (set, rkey, fingerp);
	if (memcmp (fingerp, krdata+2, 20) != 0) {
		/* Have a bogus key that matches in keyid but wrong fingerprint */
		*error = kPGPError_RevocationKeyNotFound;
		return NULL;
	}
	/* Else success */
	if (pkeys) {
		*pkeys = matches;
	}
	if (pclass) {
		*pclass = krdata[0];
	}
	return rkey;
}

/* Return true if rkey is a revocation key for key */
PGPBoolean
ringKeyIsRevocationKey (RingObject *key, RingSet const *set, RingObject *rkey)
{
	PGPByte				 revClass;
	PGPUInt32			 nRevKeys;
	PGPUInt32			 iRevKeys;
	PGPError			 error;

	nRevKeys = 1;
	iRevKeys = 0;
	while (iRevKeys < nRevKeys) {
		RingObject const *revkey;
		revkey = ringKeyRevocationKey(key, set, iRevKeys++, &revClass,
									  &nRevKeys, &error);
		if (IsPGPError(error))
			break;
		if (revkey == rkey && (revClass & 0x80))
			return TRUE;
	}
	return FALSE;
}
	

	
/* Check obj for consistency, make sure valid mask bits are in
 * parent mask
 */
void
ringObjCheck (union RingObject *obj, ringmask validmask,
			  ringmask parentmask)
{
	pgpAssert (!((obj->g.mask&validmask)&~parentmask));
	if (OBJISSIG(obj)) {
	    /*  sig should point to top-level key */ 
	    pgpAssert (OBJISTOPKEY(obj->s.by));
	}
	if (OBJISBOT(obj))
		return;
	for (obj=obj->g.down; obj; obj=obj->g.next) {
		ringObjCheck (obj, validmask, parentmask & obj->g.mask);
	}
}

/* Perform a consistency check on ring pool data structures */
void
ringPoolConsistent (RingPool *pool, int *pnsets, int *pnfiles)
{
	ringmask allocmask;
	ringmask filemask = pool->filemask;
	ringmask mask;
	RingSet *set;
	union RingObject *obj;

	allocmask = (ringmask)0;
	for (set = pool->sets; set; set = set->next)
		allocmask |= set->mask;

	pgpAssert (!(allocmask & filemask));
	mask = allocmask | filemask;

	for (obj=pool->keys; obj; obj=obj->g.next) {
		ringObjCheck (obj, mask, mask);
	}
	if (pnsets) {
		int nsets = 0;
		while (allocmask) {
			++nsets;
			allocmask &= allocmask-1;
		}
		*pnsets = nsets;
	}
	if (pnfiles) {
		int nfiles = 0;
		while (filemask) {
			++nfiles;
			filemask &= filemask-1;
		}
		*pnfiles = nfiles;
	}
}