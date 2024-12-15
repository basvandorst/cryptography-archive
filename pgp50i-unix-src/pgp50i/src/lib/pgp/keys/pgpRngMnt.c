/*
 * pgpRngMnt.c - Perform the maintenance pass over a keyring.
 * This is a lot more efficient that the way PGP 2.x did it
 * and, I modestly think, easier to understand to boot.
 *
 * Copyright (C) 1994-1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * Written by Colin Plumb
 *
 * $Id: pgpRngMnt.c,v 1.33.2.10.2.1 1997/10/31 03:28:43 hal Exp $
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>	/* For strlen() */
#include <time.h>
#include <stddef.h>

#include "pgpDebug.h"
#include "pgpMakeSig.h"
#include "pgpPktByte.h"
#include "pgpRngMnt.h"
#include "pgpRngPars.h"
#include "pgpRngPriv.h"
#include "pgpRngPkt.h"
#include "pgpRngRead.h"
#include "pgpTrust.h"
#include "pgpTrstPkt.h"
#include "pgpTimeDate.h"
#include "pgpHash.h"
#include "pgpErr.h"
#include "pgpPubKey.h"
#include "pgpRngRead.h"	/* for ringFetchObject */
#include "pgpSig.h"
#include "pgpSigSpec.h"
#include "pgpUsuals.h"
#include "pgpTypes.h"

#ifndef NULL
#define NULL 0
#endif

/*
 * This symbol marks code which prints the progress of the maintenance
 * pass. That code assumes stdio, which is not portable across GUIs, so
 * something will need to be done about it. The prints are left in the
 * source to make the location of the replacements obvious. The code
 * assumes a non-existent "FILE *f" to print to, so just #defining this
 * symbol will NOT work.
 */
#undef PRINT_PROGRESS

/*
 * The trust models...
 * All models start from a number of axiomatic "the buck stops here"
 * keys. These are the keys owned by the PGP user. This is the
 * initial set of trusted keys. Each key has a trust value - a
 * level of confidence that the user places in signatures made by
 * that key.
 *
 *
 * -- The old trust model (PGPTRUSTMODEL==0) --
 *
 * Each key has an introducer trust, either ultimate, complete, or
 * partial. These have scores of 1, 1/x and 1/y. For each key on the
 * axiomatically trusted list, and each signature made by that key on
 * a name, the introducer trust is added to the name's validity score.
 * If that score reaches 1, the name's key is added to the next introducer
 * list and its introducer trust is used for the level after that.
 *
 * At the end, names are graded according to their score.
 * If it is >= 1, they are completely valid. If it is >= 1/2,
 * they are partially valid. If < 1/2, they are not valid at all.
 * Partial versus lack of validity only affects the warning messages
 * printed out. If a key does not have any comletely valid names,
 * its introducer trust is set to 0.
 *
 * There are actually three kinds of zero introducer trust:
 * - Undefined, meaning that it has not been set. If a key has
 *   a completely valid name and undefined introducer trust, the
 *   user is asked to set one.
 * - Unknown
 * - Untrusted
 *   These latter two are equivalent valid "set" values, with the
 *   only difference being for user-interface purposes.
 *
 *
 * -- The new trust model (PGPTRUSTMODEL == 1 or 2) --
 *
 * This is based on a much more rigorous treatment of trust by Ueli
 * Maurer. See "Modelling a Public-Key Infrastructure", Ueli Maurer,
 * Proceedings 1996 European Symposium on Research in Computer Security
 * (ESORICS '96), E. Bertino (Ed.), Lecture Notes in Computer Science,
 * Berlin: Springer-Verlag, Rome, Italy, Sept. 1996. Also available from
 * <URL: http://www.inf.ethz.ch/personal/maurer/publications>.
 *
 * Consider the graph formed by taking each key as a node and
 * each signature as an edge, and assign each signature an independent
 * probability of being correct. (The probabilities don't have to be
 * independent, but it gets messy fast if they aren't, so the possibility
 * isn't considered any further.) For each possible combination of
 * valid and invalid signatures, assign it a probability based on the
 * probability of each signature being correct, and if there is a path
 * of valid signatures from the root set to a given name, it is correct.
 * Add up the probabilities of all the cases in which the name is correct,
 * and you have the computed probability that the name is correct.
 *
 * For PGPTRUSTMODEL==1:
 *
 * Basically, you have to consider every possible path from the root
 * set of axiomatically trusted keys to the name in question and correct
 * for overlaps and loops. This can't be done in a reasonable amount of
 * time, but a conservative approximation can be made. By leaving out
 * some of the more complex paths, a lower probability can be computed.
 * Since it is lower than the ideal computation, it can still be trusted.
 * A good approximation is fast to compute and yet approaches the ideal
 * approximation well.
 *
 * Since PGP supports multiple names on a key, the question arises
 * which validity value is used. The solution here is to assign a trust
 * value to each of the names, take the validity*trust product for each
 * name, and take the maximum. A name which is meaningless to the user
 * (e.g. a name in an unknown character set, such as cyrillic or kanji)
 * can't be trusted, but that doesn't mean that a second name which *is*
 * intelligible should be penalized.
 *
 * The approximation that PGP uses is as follows:
 * - Clear all trust accumulators to 0.
 * - Starting with a root set of keys (level 0), each of which have some
 * specified confidence, take that confidence directly as the trust.
 * This is currently assumed to be ultimate. For each signature made
 * by such a key, add the confidence to the name's valididty score.
 * - Using this provisional validity, multiply each names's confidence
 * by its validity to get a provisional trust. Use this trust to
 * increase the validity score of each name already processed.
 * - Multiply the final validity by the confidence to get the final trust
 * on the key, which is used
 *
 * Description of PGPTRUSTMODEL==2:
 *
 * This implements the basic Maurer trust model fairly literally. The
 * Maurer model is often considered to be "intractably complex" (an
 * out of context quote from his paper), but with certain
 * simplifications it should be feasible:
 *
 * - Focus on on-the-fly validation so we only have to find the validity of
 *   a single key.
 * - Use a ringset composed only of trusted introducers and the target key.
 * - Search for paths from the target key back to an axiomatic key such that
 *   all keys on the path are trusted introducers (nonzero confidence).
 * - Limit this search by a depth constraint, and possibly limit the total
 *   number of nodes we will look at if the depth constraint is inadequate.
 * - Find the basic trust contribution of each of the resulting paths (not yet
 *   considering interactions) as the product of the trust of each segment on
 *   the path.
 * - Sort the paths by this value and prune to keep just the top 10 or
 *   so paths.
 * - Apply Maurer's algorithm, considering all subsets of paths (2**n of them,
 *   where n is the number of paths). For each subset, form the union of all
 *   segments on the paths in that subset, and take the product of the segment
 *   trusts. Subsets with an odd number of paths add confidence, those with an
 *   even number of paths subtract.
 *
 * By limiting ourselves to just the trusted introducers (plus one
 * other) the total number of keys to be considered should not become
 * too large. Given practical and customary limitations on the number
 * of signatures a key will usually have, it should be possible to
 * find paths back to the axiomatic keys in a reasonable amount of
 * time, especially if the algorithm only needs to be run for a single
 * key (or a small number of keys) when we encrypt or check a
 * signature. Then when we prune the list of paths to just the top
 * 10, we should still be able to get a reasonably accurate
 * (under)estimate of the actual trust, unless the key gained trust by
 * having a large number of paths each with only a small amount of
 * trust. That case must be neglected for efficiency in this
 * algorithm. Having pruned the paths to a small enough set we can
 * then apply Maurer's algorithm efficiently.
 *
 * There is an additional complication specific to the PGP trust
* model. Maurer's algorithm is expressed in terms of paths where
* each segment has an associated trust level. These trusts are
* described in PGP as user-defined confidence levels specified for
* the name(s) on a key. This means that the output of the
* calculation above is a validity value for a particular name on a
* key, based on the signatures on that name. The inputs need to be
* based on the user defined confidence values for the names. The
* complication arises for keys which have more than one name (userid)
* with different user-defined confidence levels. It is not clear how
* to map this situation to Maurer's trust model.
*
* For keys with only one name, the confidence of that name is used.
* If there is more than one name, a conservative approach is to use
* the name with the smallest user-defined confidence value. (Names
* for which the user has not specified a confidence level do not
* enter into this calculation.) If all of the names (of those with
* non-zero confidence) have validity values (from earlier runs of the
* trust calculation) then we can choose the confidence of the name
* with the highest validity as in PGPTRUSTMODEL==1.
*
* Not all of the optimizations above are implemented in the current
* code, nor has it been thoroughly tested. In particular, the
* ringMnt function is still used and tries to run the algorithm on
* all keys in the set. Although a depth limitation is used there is
* no other way of limiting the construction of the paths. And the
* set of keys looked at is not pre-selected to be limited to the
* trusted introducers. That last change should be done before
* enabling this code.
*
* Some further ideas for improvement:
*
* - Paths don't need to include the last step to the trusted
* introducer, since that always has confidence of 1.0.
*
* - For creating the path subset unions, a linear search for overlap
* is used now. This may be OK for short paths but a more efficient
* algorithm would be better. Perhaps a hash table for path segments
* in the subset would be worthwhile.
*
* - When adding/subtracting the pathconf values, maybe it would be
* better to accumulate 1-confidence rather than confidence? I am
* worried about roundoff error for values very close to 1.0.
*
* - Could use a Gray code to go through path subsets in a sequence
* which would alternate even and odd parity subsets, so we would
* alternately add and subtract trust. This might help with
* accuracy. (A Gray code is a binary counter which has the property
* that only one bit changes each step. One example is:
* ++j, i ^= j & (j ^ (j-1)); which counts j as a regular counter and
* i as a Gray code counter.)
*/



/*
* Check to see if the signature is valid in time.
*
* To avoid overflow problems with 65535 days being larger than a 32-bit
* timestamp, compute how old the signature is in days, and compare
* that with the validity period. If its age, in days rounded down,
* is < the validity period, it is valid. To deal with a validity
* period of 0 being infinite, subtract one from the validity using
* unsigned arithmetic and change the < to <=. (n < 5 iff n <= 4, for
* integer n). 0 will wrap to UINT_MAX, and every unsigned number
* compared <= UINT_MAX, so the test will always be true.
*
* An intelligent 8086 or 68000 compiler would evaluate the division by
* 86400 = 675 * 128 as (x>>7)/675, which can use a 32/16 bit divide
* (which can't overflow), but I'd be surprised if any PC compilers
* actually did it that way.
*/
static int
mntSigIsValid(word32 timenow, word32 tstamp, unsigned validity)
{
	return
	(timenow <= tstamp || (unsigned)((timenow-tstamp)/86400) <= validity-1u);
}


#if PGPTRUSTMODEL==2


#define LOOKINGAT(key) ((key)->confidence & 1)
#define SETLOOKINGAT(key) ((key)->confidence |= 1)
#define CLEARLOOKINGAT(key) ((key)->confidence &= ~1)

/*
* Maximum number of paths we will keep.
* Must calculate trust for each subset.
*/
#define TOTALPATHMAX 10


/*
* The Path and PathList routines use a convention copied from the
* PgpPipeline package for tail pointers. The tail pointer points
* at the "next" pointer of the last packet, or if there are no packets
* it points at the "head" pointer for the whole list. This allows a
* uniform interface for adding packets whether they are the first or
* not.
*/

/* This is a path or a path segment. next->src may not equal dest. */
	typedef struct Path {
		struct Path	*next;
		RingObject	*src,
			*dest;
		double	confidence;
	} Path, *pPath;

/* This is a list of paths. Some segments may be on multiple paths. */
typedef struct PathList {
		struct PathList	*next;
		Path	*path;
	Path **ptail;
		double	confidence;
	} PathList, *pPathList;

	#undef DEBUGPATH

	#ifdef DEBUGPATH

static char const hexchar[16] = {
	'0','1','2','3','4','5','6','7',
	'8','9','A','B','C','D','E','F'
};

static int
ringTtyPrintKeyID(FILE *f, RingObject *key)
{
	byte *buf;
	int i;

	pgpAssert (OBJISKEY(key));
	buf = key->k.keyID;
	for (i = 4; i < 8; i++) {
		putc(hexchar[buf[i] >> 4 & 15], f);
		putc(hexchar[buf[i] & 15], f);
	}
	return 8;
}

static int
ringTtyPrintKeyName(FILE *f, RingObject *key, RingPool *pool)
{
	RingSet *set = ringSetCreate(pool);
	char const *sname;
	char bname[256];
	size_t len;

	key = key->g.down;
	while (!OBJISNAME(key)) {
		key = key->g.next;
	}
	ringSetAddObject (set, key);
	sname = ringNameName (set, key, &len);
	strncpy (bname, sname, len);
	bname[len] = '\0';
	fputs (bname, f);
	ringSetDestroy (set);
	return 0;
}

static int
ringTtyPrintKey(FILE *f, RingObject *key, RingPool *pool)
{

	ringTtyPrintKeyID (f, key);
	fprintf (f, "(");
	ringTtyPrintKeyName (f, key, pool);
	fprintf (f, ")");
	return 0;
}

static void
ringTtyPrintPath(FILE *f, Path *path, RingPool *pool)
{
	fprintf (f, " Path ");
	while (path) {
		fprintf (f, "from ");
		ringTtyPrintKey (f, path->src, pool);
		fprintf (f, " to ");
		ringTtyPrintKey (f, path->dest, pool);
		fprintf (f, ", conf: %g\n", path->confidence);
		path = path->next;
		if (path)
			fprintf (f, "  ...segment ");
	}
}

static void
ringTtyPrintPathList(FILE *f, PathList *pl, RingPool *pool)
{
	fprintf (f, "Begin PathList\n");
	while (pl) {
		fprintf (f, " (conf=%g) ", pl->confidence);
		ringTtyPrintPath (f, pl->path, pool);
		pl = pl->next;
	}
	fprintf (f, "End PathList\n");
}

#endif /* DEBUGPATH */


/* Allocate a Path segment */
static Path *
pathAlloc (RingPool *pool)
{
	Path *path = pool->paths;

	if (path) {
	pool->paths = path->next;
	} else {
		path = memPoolAlloc (&pool->pathpool, sizeof(struct Path),
			alignof(struct Path));
	}
	return path;
}

/* Free a Path segment */
static void
pathFree (Path *path, RingPool *pool)
{
	path->next = pool->paths;
	pool->paths = path;
}

/* Free an entire Path */
static void
pathFreeAll (Path *phead, RingPool *pool)
{
	Path **ptail = &phead->next;
	while (*ptail) {
		ptail = &(*ptail)->next;
	}
	*ptail = pool->paths;
	pool->paths = phead;
}

/* Allocate a PathList element */
static PathList *
pathListAlloc (RingPool *pool)
{
	PathList *plist = pool->pathlists;

	if (plist) {
		pool->pathlists = plist->next;
		plist->next = NULL;
	} else {
		plist = memPoolAlloc (&pool->pathpool, sizeof(struct PathList),
		alignof(struct PathList));
	}
	return plist;
}

/* Free a PathList element, also freeing any path it points to */
static void
pathListFree (PathList *plist, RingPool *pool)
{
	if (plist->path)
		pathFreeAll (plist->path, pool);
	plist->path = 0;
	plist->confidence = 0.;
	plist->next = pool->pathlists;
	pool->pathlists = plist;
}

/* Free a whole PathList, also freeing the Paths it points to */
static void
pathListFreeAll (PathList *plhead, RingPool *pool)
{
	PathList **pltail = &plhead->next;
	while (*pltail)
		pltail = &(*pltail)->next;
	*pltail = pool->pathlists;
	pool->pathlists = plhead;
	while (plhead != *pltail) {
		pathFreeAll (plhead->path, pool);
		plhead->path = 0;
		plhead->confidence = 0.;
		plhead = plhead->next;
	}
}


/* Add a Path segment to a Path
*
* Returns	updated Path tail pointer
*
* tail		 tail pointer for Path to be added to
* seg		 pointer to Path segment to add
*
*/
static Path **
pathAddSeg (Path **tail, Path *seg)
{
	seg->next = *tail;
	*tail = seg;
	return &seg->next;
}

/*
* Make a copy of a Path, leaving the original untouched. Optionally return a
* pointer to the tail of the new Path.
*
* Returns	pointer to new cloned path
*
* p1		pointer to the path to be cloned
* ptl		 optional return parameter for tail pointer of new path
* pool		 ringpool used for allocations of new path segments
*
*/
static Path *
pathClone (Path *p1, Path ***ptl, RingPool *pool)
{
	Path *phead,
	**ptail = &phead;

	while (p1) {
		*ptail = pathAlloc(pool);
		if (!*ptail)
			return NULL;
		**ptail = *p1;
		p1 = p1->next;
		ptail = &(*ptail)->next;
	}
	if (ptl)
		*ptl = ptail;
	return phead;
}


/* Add a copy of a Path to the PathList
*
* Returns	tail pointer of updated PathList
*
* tail		 tail pointer of PathList to be added to
* path		 pointer to Path to add to the PathList
* pool		 ringpool to use for memory allocations
*
*/
static PathList **
pathListAddPathClone (PathList **tail, Path *path, RingPool *pool)
{
	PathList *pl;

	pl = pathListAlloc(pool);
	if (!pl)
		return NULL;
	pl->path = pathClone (path, &pl->ptail, pool);
	pl->next = *tail;
	*tail = pl;
	return &pl->next;
}


/* Calculate the confidence associated with a path. */
static double
pathConfidence (Path *path)
{
	double conf = 1.0;

	while (path) {
		conf *= path->confidence;
		path = path->next;
	}
	return conf;
}

/*
* Prune the PathList to keep only the highest-confidence pathmax Paths.
* Return the new PathList, free the no longer used Paths.
*
* Returns	new PathList with no more than pathmax paths
*
* plist	 PathList to be pruned
* pathmax	maximum number of paths to allow
* pool		 ringpool to use for memory allocations
*
*/
static PathList *
pathListPrune (PathList *plist, unsigned pathmax, RingPool *pool)
{
	PathList *iplist;			/* Copy of plist */
	PathList **pl1,		 	/* Iterator over plist */
	**maxpl1;			/* Max confidence pl1 pointer */
	PathList *nplist,			/* New pathlist head */
	**nptail;			/* New pathlist tail */
	unsigned npaths,		 	/* Number of paths in plist initially */
	newnpaths;		/* Number of paths in final nplist */
	double maxconf;		 /* Best confidence value so far */

	/* Calculate confidence value for each path */
	iplist = plist;
	npaths = 0;
	while (plist) {
		npaths += 1;
		plist->confidence = pathConfidence (plist->path);
		plist = plist->next;
		}
		plist = iplist;

		/* Initialize the new path list */
		nplist = NULL;
		nptail = &nplist;
		newnpaths = min (pathmax, npaths);
		while (newnpaths--) {
			 /* Add the best path from plist to nplist */
			 pl1 = &plist;
			 maxconf = 0.;
			 maxpl1 = pl1;
			 while (*pl1) {
			 	/* Find best path in plist in **maxpl1 */
			 	if ((*pl1)->confidence > maxconf) {
			 		 maxconf = (*pl1)->confidence;
			 		 maxpl1 = pl1;
			 	}
			 	pl1 = &(*pl1)->next;
			 }
			 *nptail = *maxpl1;
			 *maxpl1 = (*maxpl1)->next;
			 nptail = &(*nptail)->next;
			 *nptail = NULL;
		}

		if (plist)
			 pathListFreeAll (plist, pool);
		return nplist;
	}


	/*
* Append a (copy of a) second path to the first, removing common segments.
* Return pointer to "next" pointer of last segment (and pass that as ptail)
*
* Return	tail pointer for updated Path
*
* phead	 pointer to Path which gets appended to
* ptail	 tail pointer for Path which gets appended to
* p2		pointer to Path to append (a copy of)
* pool		 ringpool to use for memory allocations
*
*/
static Path **
pathUnion (Path *phead, Path **ptail, Path *p2, RingPool *pool)
{
	Path *p1;
	Path **iptail = ptail;

	/* Add path p2 but skip those segments in p1 */
	for ( ; p2; p2 = p2->next) {
		p1 = phead;
		for (p1=phead; p1 && p1 != *iptail; p1 = p1->next) {
			/* Eliminate path segments which are in there already */
			if (p2->src == p1->src && p2->dest == p1->dest) {
				/* Make sure our confidence rules are working */
				pgpAssert (p2->confidence == p1->confidence);
				break;
			}
		}
		if (p1 && p1 != *iptail)
			continue;
		*ptail = pathAlloc(pool);
		if (!*ptail)
			return NULL;
		**ptail = *p2;
		ptail = &(*ptail)->next;
		*ptail = NULL;
	}
	return ptail;
}

/*
* Calculate the confidence for a list of paths. We add the confidence for
* each path, subtrace the confidence for the unions of all pairs of paths,
* add the confidence for the unions for all the triples, and so on.
* We actually do this by taking each subset of the paths, unioning them,
* calculating the confidence for that path, and adding or subtracting it
* based on the parity.
*/
static double
pathListConfidence (PathList *plist, RingPool *pool)
{
		unsigned	pathcount,
			mask,
				tmask;
		PathList	*iplist = plist;
		double	conf = 0.;

#ifdef DEBUGPATH
	fprintf (stdout, "Maurer alg:\n");
#endif

	pathcount = 0;
	while (plist) {
		++pathcount;
		plist = plist->next;
	}
	plist = iplist;

	pgpAssert (pathcount < sizeof(unsigned) * 8);
	for (mask=1; mask < (1U<<pathcount); ++mask) {
		double pathconf;
		Path *phead=NULL, **ptail=&phead;
		int oddparity = 0;
		plist = iplist;
		tmask = mask;
		while (plist && tmask) {
			if (tmask & 1) {
				ptail = pathUnion (phead, ptail, plist->path, pool);
				oddparity = !oddparity;
			}
			plist = plist->next;
			tmask >>= 1;
		}
		pathconf = pathConfidence (phead);
#ifdef DEBUGPATH
		fprintf (stdout, "%sing %g: ", oddparity?" add":" subb",pathconf);
		ringTtyPrintPath (stdout, phead, pool);
#endif
		if (oddparity)
			conf += pathconf;
		else
			 	conf -= pathconf;
		pathFreeAll (phead, pool);
		phead = NULL;
		ptail = &phead;
	}
#ifdef DEBUGPATH
	fprintf (stdout, "Net of %g\n", conf);
#endif
	return conf;
}

/*
* Find the confidence level to use for the given key.
* This is difficult in this trust model because we generally don't
* have validity on the names associated with the key, since there is
* no well defined (nor arbitrarily imposed) ordering of the graph.
* The result is that when we do our calculation we may end up with a
* de facto validity on a key/name that has no relation to the (cached)
* validity stored on the key.
*
* So we have a set of heuristics to choose the confidence associated
* with some name, to wit:
*
* If all the names have at least some validity, we choose the confidence
* associated with the most valid name. Otherwise we just choose the
* lowest confidence of all the names. The reason for the second case is so
* that if we have a true name and a false name, but the true name doesn't
* have validity yet, while the false name has a little tiny bit of validity,
* we don't want to choose the confidence of the false name, which might be
* very high. It might be that the next step in the path will give high
* validity to the true (but low confidence) name. We would end up counting
* this key as both high confidence and high validity, which may be wrong.
*
* This heuristic might seem to have the problem that if you get a new name
* for a trusted
* key its trust will go away until either the new name gets some validity
* or you set the confidence level on the new name. But this does not
* happen because such names are not included in the confident set.
*
* A worse problem is that Snidely could get Nell's name on his key, get
* some low-confidence signer to sign Nell, and get an even lower conf-
* idence signer to sign his own. Then we would use Nell's confidence
* on Snidely's key, and if there were also a good signer on Snidely's name,
* which we hadn't run into yet, we would again count Snidely's signatures
* too highly.
*
* So this suggests a safer heuristic which is to take either the lowest
* confidence, or else perhaps to downgrade the confidence by the
* validities. However doing this if all the validities are accurate will
* end up squaring them. Maybe we could just downgrade by the relative
* validities, best to worst...
*
* (For now we have an #if below to always use the lowest confidence)
*
* An idea suggested by Mark is to base the confidence on the particular
* name which is the next step in the path. However this complicates the
* union operation.
* Either a path no longer has a well defined confidence, preventing
* Maurer's algorithm from working, or else we have
* to define a path as being from name to name, and if a key has two
* names (different email addresses, say, home and work) we would end up
* counting its contribution to trust twice. (This is essentially because
* the assumption of independence fails spectacularly in this case!)
*
* Keep in mind that despite all these difficulties, if there is only one
* name then there is no problem! We just use the confidence on that name.
	*/
static double
pathKeyConfidence (RingObject *key, RingSet const *set)
	{
		RingObject	*name;
		double	minconfidence = 1.0,
			confidence,
			errorchance,
			maxvalidconfidence = 0.;
		word16	maxvalidity = 0;
	int				allvalid = 1;		 /* True if all names have validity */
	ringmask const	mask = set->mask;

	for (name = key->g.down; name; name = name->g.next) {
		if (!OBJISNAME(name) || !(name->g.mask & mask))
			continue;
		if (name->n.confidence == PGP_NEWTRUST_UNDEFINED)
			continue;
		errorchance = ringTrustToDouble (
		ringTrustToIntern(name->n.confidence));
		confidence = 1. - 1. / errorchance;
		if (confidence < minconfidence)
			minconfidence = confidence;
		if (!name->n.valid) {
			allvalid = 0;
		} else if (name->n.valid > maxvalidity ||
						(name->n.valid == maxvalidity &&
						confidence < maxvalidconfidence)) {
			 	maxvalidity = name->n.valid;
			 maxvalidconfidence = confidence;
		}
	}
	/*
* If all names have known validity, use the most valid name's confidence.
* Else choose the lowest established confidence of any name.
*/
#if 0
/* Too risky to use this heuristic, just take minimum confidence */
	if (allvalid)
		return maxvalidconfidence;
	else
#endif
		return minconfidence;
}

/* Prototype */
static PathList **
ringFindPathsKey (RingObject *start, PathList **ppath,
	Path *predh, Path **predt, unsigned depth,
unsigned maxdepth, RingSet const *set, word32 timenow, RingPool *pool);

/* Helper routines for ringFindPathsName */

/* Return the newest valid signature by the given key on the given name */
static RingObject *
ringNewestValidSig (RingObject *name, RingObject *key, unsigned depth,
	RingSet const *set, word32 timenow)
{
	RingObject		*bestsig = NULL,
					*sig;
	ringmask const	mask = set->mask;

	pgpAssert (OBJISNAME(name));
	pgpAssert (OBJISKEY(key));

	for (sig = name->g.down; sig; sig = sig->g.next) {
		if (OBJISSIG(sig)
			&& (sig->s.trust & PGP_SIGTRUSTF_CHECKED)
			&& ((sig->g.mask & mask) || depth==1)
			&& sig->s.by == key
			&& mntSigIsValid(timenow, sig->s.tstamp, sig->s.validity)) {
			/* Good sig by key, see if it is newer than bestsig */
			if (!bestsig
				|| bestsig->s.tstamp < sig->s.tstamp) {
				bestsig = sig;
			}
		}
	}
	return bestsig;
}

/*
* Return true if there is a signature by the key on an earlier sibling
* of this name. This alerts us to a condition which could cause duplicate
* paths, which will waste resources.
*/
static int
ringEarlierValidSig (RingObject *name, RingObject *key, RingSet const *set,
	word32 timenow)
{
	RingObject		*parent,
					*sibname,
					*sig;
	ringmask const	mask = set->mask;

	pgpAssert (OBJISNAME(name));
	pgpAssert (OBJISKEY(key));

	parent = name->g.up;
	pgpAssert (OBJISKEY(parent));

	for (sibname=parent->g.down; sibname!=name; sibname=sibname->g.next) {
		if (!OBJISNAME(sibname)
			|| !(sibname->g.mask & mask))
			continue;
		for (sig=sibname->g.down; sig; sig=sig->g.next) {
			if (OBJISSIG(sig)
			&& (sig->s.trust & PGP_SIGTRUSTF_CHECKED)
			&& (sig->g.mask & mask)
			&& sig->s.by == key
			&& mntSigIsValid(timenow, sig->s.tstamp, sig->s.validity)) {
				/* Have an earlier signature we are looking for */
				return TRUE;
			}
		}
	}
	return FALSE;
}

/*
* See if the specified key is in the special "pairing" relationship with
* the specified name. We are called with key signing name. The main point
* is whether key has a name which matches. If so, we return that name.
* It is also necessary that name be self-signed, so we will check for that
* here. The signing key must be valid as well.
*/
static RingObject *
ringPairedName (RingObject *name, RingObject *key, RingSet const *set)
{
	RingObject			*keyname;	 /* Names on key */
	RingObject			*namekey;	 /* Key parent of name */
	RingObject			*namesig;	 /* Sigs on name */
	byte const			*smatchname; /* Actual name of *name */
	byte const			*sname;		/* Actual name of *keyname */
	unsigned			lmatchname; /* Length of smatchname */
	unsigned			lname;		 /* Length of sname */
	ringmask const		mask = set->mask;
	
	pgpAssert (OBJISKEY(key));
	pgpAssert (OBJISNAME(name));

	/* Check for key validity */
	if (key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
		return NULL;
	
	/* Check that name is self-signed */
	namekey = name->n.up;
	pgpAssert (OBJISKEY(namekey));
	for (namesig = name->n.down; namesig; namesig = namesig->g.next) {
		if (!OBJISSIG(namesig))
			continue;
		/* XXX Should check expiration */
		if ((namesig->s.trust & PGP_SIGTRUSTF_CHECKED)
			&& (namesig->g.mask & mask)
			&& (namesig->s.type & 0xF0) == PGP_SIGTYPE_KEY_GENERIC) {
			break;				/* Exit with namesig nonnull on success */
		}
	}
	if (!namesig)	 		 	/* NULL namesig means no self sig */
		return NULL;
		
	/* Get userid string of name */
	smatchname = (byte const *)ringFetchObject(set, name, &lmatchname);

	/* See if any names on keyname match */
	for (keyname = key->k.down; keyname; keyname = keyname->n.next) {
		if (!OBJISNAME(keyname))
			continue;
		sname = (byte const *)ringFetchObject(set, name, &lname);
		if (lname == lmatchname && !memcmp (sname, smatchname, lname)) {
			/* Have a matching name! */
			return keyname;
		}
	}
	return NULL;
}


/*
* Find all the paths from the starting name back to an axiomatically
* trusted key.
* pred holds the path which leads to the starting key from some target.
* lastseg points at the last segment of the path, which already has
* the key this name is on as the dest. We just have to fill it in
* and recurse, or check for buckstop keys.
*
* We reject paths which are to keys which have
* already been processed as signing this key. This can happen
* if there are two names on a key, both signed by another key. Otherwise
* these would be treated as two different paths, but they
* end up being identical. The Maurer algorithm correctly accounts for this,
* but it is a waste. What we do is, for each signature, we check
* all our earlier name siblings and see if they have a signature from the
* same key. If so we skip this sig.
*
* Returns	tail pointer for updated PathList
*
* name		 starting name object for search backwards through web of trust
* ppath	 tail pointer of PathList to add our new paths to
* newseg	pointer to the last segment of the Path we will add to
* predh	 pointer to the (head of the) Path we will add to
* predt	 tail pointer to the Path we will add to (&newseg->tail)
* depth	 length of path so far, counting newseg (e.g. 1 at outer level)
* maxdepth	maximum length of paths we allow
* set		 ringset for trusted keys
* timenow	current timestamp
* pool		 ringpool to use for memory allocations
* skipsigs list of signatures, chained by next, not to follow key of
*
*/
static PathList **
ringFindPathsName (RingObject *name, PathList **ppath, Path *newseg,
	Path *predh, Path **predt, unsigned depth,
unsigned maxdepth, RingSet const *set, word32 timenow, RingPool *pool,
	RingObject *skipsigs)
{
	RingObject *sig,
	*sigkey,
				*pairedname,
				*ssig;
	ringmask const	mask = set->mask;

	for (sig = name->g.down; sig; sig = sig->g.next) {
		if (!OBJISSIG(sig))
			continue;
		/* Skip if invalid sig */
		if (!(sig->s.trust & PGP_SIGTRUSTF_CHECKED)
			|| (!(sig->g.mask & mask) && depth>1)
			|| !mntSigIsValid(timenow, sig->s.tstamp, sig->s.validity)
			|| (sig->s.type & 0xF0) != PGP_SIGTYPE_KEY_GENERIC)
			continue;
		/* Skip if signing key not in signer set */
		sigkey = sig->s.by;
		if (!(sigkey->g.mask & mask))
			continue;
		/* Skip if there is a newer valid sig by the same key */
		if (ringNewestValidSig (name, sigkey, depth, set, timenow) != sig)
			continue;
		/* Skip if we have already done this key on an earlier sibling */
		if (depth>1 && ringEarlierValidSig (name, sigkey, set, timenow))
			continue;
		/*
		* Skip if signing key is in skipsigs list. This is for the "paired
		* keys" feature.
		*/
		for (ssig = skipsigs; ssig; ssig = ssig->g.next) {
			if (OBJISSIG(ssig) && sigkey == ssig->s.by)
				break;			/* Exit from loop with ssig nonnull on match */
		}
		if (ssig)		 		 /* Non-null ssig means it was on skip list */
			continue;

		/* OK, investigate this signer further */
		newseg->src = sigkey;
		if (sigkey->k.trust & PGP_KEYTRUSTF_BUCKSTOP
			&& !(sigkey->k.trust & (PGP_KEYTRUSTF_REVOKED |
											PGP_KEYTRUSTF_EXPIRED))
			/*&& !(key->k.trust & PGP_KEYTRUSTF_DISABLED)*/) {
			/* Found a path */
			newseg->confidence = 1.0;
			ppath = pathListAddPathClone (ppath, predh, pool);
			if (!ppath) {
				/* Out of memory */
				return NULL;
			}
		} else if (depth < maxdepth &&
				!LOOKINGAT(&sigkey->k) &&
				(newseg->confidence =
					pathKeyConfidence(sigkey, set)) != 0) {
			/* Recurse */
			ppath = ringFindPathsKey (sigkey, ppath, predh, predt,
					depth+1, maxdepth, set, timenow, pool);
			if (!ppath) {
				/* Out of memory */
				return NULL;
			}
#if 0
/* This code is not yet tested */
		} else if (depth < maxdepth &&
				!LOOKINGAT(&sigkey->k) &&
				(pairedname = ringPairedName(name, sigkey, set)) != NULL) {
			 		/*
			* We have a signature by a sibling key, one which matches our
			* name. Recurse from that name. The resulting path element
			* will have as source, signer of that paired name, and as
			* destination, this key. Even though there is no "actual"
			* signature in such a form, there is an implied signature.
			* We won't increment depth in this case.
			* Here we use the skipsigs feature - we don't want to follow
			* any sigs from our pair which we are also following here.
			* This is the only place it is used (at this writing!).
			 		*/
			ppath = ringFindPathsName (pairedname, ppath, newseg, predh, predt,
				depth, maxdepth, set, timenow, pool, name->g.down);
			if (!ppath) {
				/* Out of memory */
				return NULL;
			}
#endif
		}
	}
	return ppath;
}


/*
* Find all the paths from the starting key back to an axiomatically
* trusted key.
*
* Returns	tail pointer for updated PathList
*
* start	 starting key object for search backwards through web of trust
* ppath	 tail pointer of PathList to add our new paths to
* predh	 pointer to the (head of the) Path we will add to
* predt	 tail pointer to the Path we will add to
* depth	 length of path, counting segment we will add (newseg, below)
* maxdepth	maximum length of paths we allow
* set		 ringset for trusted keys
* timenow	current timestamp
* pool		 ringpool to use for memory allocations
*
*/
static PathList **
ringFindPathsKey (RingObject *start, PathList **ppath,
	Path *predh, Path **predt, unsigned depth,
	unsigned maxdepth, RingSet const *set, word32 timenow, RingPool *pool)
{
	RingObject *name;
	Path *newseg;
	Path **newpredt;

	/* It's a key but not axiomatic */
	pgpAssert (OBJISKEY(start));
	pgpAssert (!(start->k.trust & PGP_KEYTRUSTF_BUCKSTOP));

	newseg = pathAlloc(pool);
	if (!newseg) {
		/* Out of memory */
		return NULL;
	}
	newseg->dest = start;
	newseg->next = NULL;
	newpredt = pathAddSeg (predt, newseg);
	SETLOOKINGAT(&start->k);
	for (name = start->g.down; name; name = name->g.next) {
		if (!OBJISNAME(name))
			continue;
		if (name->n.confidence == 0 ||
			name->n.confidence == PGP_NEWTRUST_UNDEFINED)
			continue;
		ppath = ringFindPathsName (name, ppath, newseg, predh, newpredt,
			depth, maxdepth, set, timenow, pool, NULL);
		if (!ppath)				/* out of memory */
			break;
	}
	CLEARLOOKINGAT(&start->k);
	pathFree (newseg, pool);
	*predt = NULL;
	return ppath;
}



/*
* Find all paths backwards from the specified target name to some key
* which is axiomatically trusted.
*
* Returns	tail pointer for new PathList
*
* name		 starting name object for search backwards through web of trust
* ppath	 address of PathList pointer we set to point to head of PathList
* maxdepth	maximum length of paths we allow
* set		 ringset for trusted keys
* timenow	current timestamp
* pool		 ringpool to use for memory allocations
*
*/
static PathList **
ringFindPathsBack (RingObject *name, PathList **ppath, unsigned maxdepth,
	RingSet const *set, word32 const timenow, RingPool *pool)
{
Path *phead = NULL,
**ptail = &phead;
	RingObject *key;
	ringmask const	mask = set->mask;

	pgpAssert (OBJISNAME(name));
	pgpAssert (name->g.mask & mask);
	key = name->g.up;
	phead = pathAlloc(pool);
	if (!phead)
		return NULL;
	SETLOOKINGAT(&key->k);			/* avoid loops involving target */
	phead->dest = key;
	phead->src = NULL;
	phead->confidence = 0.;
	phead->next = NULL;
	ptail = &phead->next;
	ppath = ringFindPathsName (name, ppath, phead, phead, ptail, 1, maxdepth,
		set, timenow, pool, NULL);
	CLEARLOOKINGAT(&key->k);
	pathFree (phead, pool);
	return ppath;
}

/*
* Create a copy of the given ringSet, but containing only keys which have
* at least one name with defined confidence.
*/
static struct RingSet *
ringSetConfidentSet (struct RingSet const *set)
{
	struct RingSet		*trustedset;
	struct RingIterator	*iter;

	trustedset = ringSetCreate (ringSetPool (set));
	if (!trustedset)
		return 0;
	iter = ringIterCreate (set);
	if (!iter) {
		ringSetDestroy (trustedset);
		return 0;
	}
	ringSetAddSet (trustedset, set);
	while (ringIterNextObject (iter, 1)) {
		int goodconf = 0;		 		 /* True if key has a conf. name */
		while (ringIterNextObject (iter, 2)) {
			union RingObject *nameobj;
			nameobj = ringIterCurrentObject (iter, 2);
			if (!OBJISNAME(nameobj))
				continue;
			/* Remove names with undefined or zero confidence */
			if (nameobj->n.confidence == PGP_NEWTRUST_UNDEFINED ||
				nameobj->n.confidence == 0) {
				ringSetRemObject (trustedset, nameobj);
			} else {
				goodconf = 1;
			}
		}
		if (!goodconf) {
			ringSetRemObject (trustedset, ringIterCurrentObject (iter, 1));
		}
	}
	ringIterDestroy (iter);
	ringSetFreeze (trustedset);
	return trustedset;
}

#endif /* PGPTRUSTMODEL==2 */

#if PGPTRUSTMODEL==0

/*
* Add "inc" trust to the name signed by the given sig.
* If the trust passes "thresh", mark the name with the level
* and add the key that owns it to the list. Return the expanded list.
*
* TODO: Do something sensible with signatures on keys.
*/
static struct RingKey const *
mntDoSig(struct RingSig const *sig, struct RingKey const *list,
	int const inc, int const thresh, word32 const timenow, int const level)
{
	union RingObject *name, *key;
	byte sigtype;

	(void)timenow;	/* Avoid warning */
	pgpAssert(mntSigIsValid(timenow, sig->tstamp, sig->validity));

	name = sig->up;
	if (!OBJISNAME(name)) {
		/* @@@ do anything with sigs on keys? */
		return list;
	}

#if 0
	/* Ignore self-signature */
	key = name->n.up;
	pgpAssert(OBJISKEY(key));
	if (key == sig->by)
	return list;
#endif

	sigtype = sig->type & 0xF0;
	if (sigtype != PGP_SIGTYPE_KEY_GENERIC)
		return list;

#if PRINT_PROGRESS
	printf("  Trust %d/%d -> %d/%d on name ",
	name->n.trustval, thresh,
	name->trustval + inc, thresh);
	(void)ringTtyPutString(name->name, name->len, -1u, stdout, '"', '"');
	putchar('\n');
#endif
	name->n.trustval += inc;
	/*
	* If we are not yet at the valid level or have already
	* marked this name as validated, bail out.
	*/
	if (name->n.trustval < thresh || NAMELEVEL(&name->n))
		return list;
	/* Passed threshold - label with certification level. */
	NAMESETLEVEL(&name->n, level);
	key = name->n.up;
	pgpAssert(OBJISKEY(key));
	if (key->g.flags & KEYF_TRUSTED ||	/* Already listed? */
	key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
		return list;
#if PRINT_PROGRESS
	ringKeyIDprint(stdout,
"    Potential introducer: first userID trust >= 1 on keyID ",
		 	key->keyID);
#endif
	key->k.util = (struct RingKey *)list;
	key->g.flags |= KEYF_TRUSTED;
	return &key->k;
}


/*
* Look for other keys signed by the grandparent of sig which have the same
* name. Propagate the same trust to those names. (Don't do it if they
* have a sig by the same signing key.) The reasoning is that the same
* person owns both keys, he has the same name, so sigs on the first name
* can be treated as though they are on the other name. This will facilitate
* moving to a new key from an old one.
*
* One issue not dealt with here is that if the grandparent key here is also
* a trusted introducer, trust will move to the destination key in two ways:
* here, by direct "pair propagation" from signatures on that key's names
* which match the names here; and also by normal trust propagation due to
* the signature. This could end up counting some trust twice, although
* in this trust model it doesn't look like it introduces any significant
* weaknesses.
*/

static struct RingKey const *
mntDoMatchingNames(struct RingSig const *sig, struct RingKey const *list,
	RingSet const *set, int const inc, int const thresh, word32 const timenow,
	int const level)
{
	RingObject			*name;		/* Name which sig is on */
	RingObject			*key;	 	/* Key which name is on */
	RingObject			*propsig;	 /* All sigs by key */
	RingObject			*propname;	/* Name which propsig is on */
	RingObject			*propkey;	 /* Key which propname is on */
	RingObject			*namesig;	 /* All sigs on propname */
	byte				sigtype;
	byte				selfsigfound;
	ringmask const		mask = set->mask;

	pgpAssert(mntSigIsValid(timenow, sig->tstamp, sig->validity));

	/* Ignore sigs not on names */
	name = sig->up;
	if (!OBJISNAME(name))
		return list;

	/* Ignore self signatures, invalid keys, and keys which don't sign */
	key = name->n.up;
	pgpAssert(OBJISKEY(key));
	if (key == sig->by)
	return list;
	if (key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
		return list;

	/* Ignore funny kinds of sigantures */
	sigtype = sig->type & 0xF0;
	if (sigtype != PGP_SIGTYPE_KEY_GENERIC)
		return list;

	/*
	* As a short-term security precaution, only propagate for fully valid
	* names. The problem is that otherwise, someone could have two keys,
	* A and B, which both are marginally valid.  If both A and B sign key
	* C which has the same name, the marginally valid signatures combine
	* to give full validity to C. We have a solution to this but it is
	* complex. For now we will only do propagation from names which are
	* fully valid, which will mean that only fully valid keys get their
	* trust propagated to matching names.
		*/
	if (!NAMELEVEL(&name->n))
		return list;

	/* Look for other keys with same name */
	for (propsig = key->k.sigsby; propsig;
						propsig = (RingObject *)propsig->s.nextby) {
		if (!(propsig->g.mask & mask))
			continue;
		propname = propsig->s.up;

		/* Ignore sigs not on names */
		if (!OBJISNAME(propname))
			continue;
		propkey = propname->n.up;

		/* Ignore nonkeys(?) or self sigs */
		if (!OBJISKEY(propkey) || propkey == key)
			continue;

		/* Make sure signature is valid */
		if (!(propsig->s.trust & PGP_SIGTRUSTF_CHECKED)
		|| !(propsig->s.mask & mask)
		|| !mntSigIsValid(timenow, propsig->s.tstamp, propsig->s.validity))
			continue;
		if ((propsig->s.type & 0xF0) != PGP_SIGTYPE_KEY_GENERIC)
			continue;

		/* See if names match */
		if (ringNamesDiffer (set, name, propname))
			continue;

		/* Skip if name already has a sig from our signer */
		/* Also, this must be the newest sig by us */
		/* Also, it must have a self signature */
		selfsigfound = 0;
		for (namesig=propname->n.down; namesig; namesig=namesig->s.next) {
			if (!OBJISSIG(namesig) || !(namesig->g.mask & mask))
				continue;
			if (namesig->s.by == sig->by)
				break;		/* Break out with nonnull namesig on match */
			if (namesig != propsig && namesig->s.by == key
						&& namesig->s.tstamp >= propsig->s.tstamp)
				break;		/* Break outwith nonnull namesig if not newest */
			/* Check for valid self signature */
			if (namesig->s.by == propkey
				&& (namesig->s.trust & PGP_SIGTRUSTF_CHECKED)
				&& (namesig->s.mask & mask)
				&& mntSigIsValid(timenow, namesig->s.tstamp,
												namesig->s.validity)) {
				selfsigfound = 1;
			}
		}

		/* If namesig is non-null it had a matching sig, skip this name */
		/* Also if there was no self signature */
		if (namesig || !selfsigfound)
			continue;

		/* OK, we have a name suitable for propagating trust to */
		list = mntDoSig(&propsig->s, list, inc, thresh, timenow, level);
	}
	return list;
}


/*
* Add "inc" trust to each name signed by a valid signature on
* the "sigs" list. If the trust passes "thresh", add the resultant
* key to the list. Return the expanded list.
*
* Signatures that are expired or have been superseded are weeded out.
* Note that a postdated signature does not supersede one dated yesterday!
	*/
static struct RingKey const *
mntWalkSigList(struct RingSig const *sigs, struct RingKey const *list,
	RingSet const *set, int const inc, int const thresh, word32 const timenow,
	int const level)
{
	struct RingSig const *cur;
	ringmask const mask = set->mask;

	while (sigs) {
		/* Find the first valid checked signature under the mask. */
		if (!(sigs->trust & PGP_SIGTRUSTF_CHECKED)
		|| !(sigs->mask & mask)
		|| !mntSigIsValid(timenow, sigs->tstamp, sigs->validity))
		{
			sigs = sigs->nextby;
			continue;
		}
		/* The first candidate signature */
		cur = sigs;

			/*
		* Search all other signatures by this key on the same object
		* for the most recent valid signature, which is the only
		* one which is accorded any weight. We use the fact that
		* after
			*/
		while ((sigs = sigs->nextby) != NULL && sigs->up == cur->up)
		{
				/*
			* So now that the signature at the front of the sigs
			* list is on the same thing as "cusrig", consider
			* replacing cur, if the new signature is is
			* checked good, valid, under the right mask, and
			* more recent than cur.
			 		*
			 		*/
			if (sigs->trust & PGP_SIGTRUSTF_CHECKED
			&& sigs->mask & mask
			&& cur->tstamp < sigs->tstamp
			&& mntSigIsValid(timenow, sigs->tstamp,
					sigs->validity))
				cur = sigs;
		}

		/* Do the trust computations on the resultant signature. */
		list = mntDoSig(cur, list, inc, thresh, timenow, level);

		/* Propagate sig info also to linked keys with matching names */
		list = mntDoMatchingNames(cur, list, set, inc, thresh, timenow, level);

	}
	return list;
}

/*
* Walk a list of keys (linked through the util field), adding the
* appropriate trust values to the names the key has signed.
*/
static struct RingKey const *
mntList(struct RingKey const *list, RingSet const *set, int const add[8],
	int const threshold, word32 const timenow, unsigned const level)
{
	struct RingKey const *newlist;
	int trustinc;

	for (newlist = 0; list; list = list->util) {
		pgpAssert(list->flags & KEYF_TRUSTED);
		trustinc = add[list->trust & PGP_KEYTRUST_MASK];
#if PRINT_PROGRESS
		printf(, "Adding %d/%d trust from ", trustinc, threshold);
		ringKeyPrint(stdout, (struct RingPool *)0, "", list);
#endif
#if 0
			/*
		* @@@ PGP 2.x allows disabled keys to participate in
		* trust computation. Should this be added?
			*/
		if (list->trust & (PGP_KEYTRUSTF_DISABLED))
			continue;
#endif
		if (trustinc <= 0)
			continue;
		newlist = mntWalkSigList(&list->sigsby->s, newlist, set, trustinc,
		threshold, timenow, level);
	}

	return newlist;
}

#else /* PGPTRUSTMODEL>0 */

/*
* This finds the combined trust value given two trusts which
* are arranged in series. Trust values are actually the
* scaled negative logarithms of *distrust*, so assuming
* independent failures, two signatures in parallel have a
* distrust which is the product of the input distrusts, and
* we only have to add the trust values. But when they're
* in series, we want to multiply the *trusts*, and it gets hairier...
* This is a quick and dirty brute-force-and-ignorance approach.
* I know of better solutions, but haven't had time to do the
* necessary numerical analysis.
*
* @@@ Optimize this code
*
* Given t1 = log(p1)/k and t2 = log(p2)/k, where p1 and p2 are
* probabilities that are bounded between 0 < p1,p2 <= 1, find
* t3 = log(1-(1-p1)*(1-p2))/k
* = log(1-(1-exp(t1*k))*(1-exp(t2*k)))/k.
* = log(p1 + p2 - p1*p2)
* = log(exp(t1*k) + exp(t2*k) - exp(t1*k)*exp(t2*k))/k
* k is chosen to make the logs come out right,
* so that an increment of TRUST_DECADE corresponds to 1/10.
* I.e. exp(TRUST_DECADE*k) = 1/10
* => TRUST_DECADE*k = log(1/10)
* => k = -log(10)/TRUST_DECADE;
* Well, actually there's an additional factor of TRUST_CERTSHIFT
* thrown in there, a factor of 64 which allows probabilities down to
* 10^-25 and a granularity of +/-0.09% in trust values.
* Note that p1+p2-p1*p2 = p1 + p2*(1-p1) >= p1, so the output
* probability is always greater than either of the input probabilities,
* so t3 is always less than t1 or t2. Thus, the computation can't
* overflow.
*
* I'd prefer to somehow evaluate it directly in log form, using
* something like Zech's log function to evaluate it.
* H'm... an alternate evaluation technique...
* p1 * (1 + p2/p1(1-p1))
* = p1 * (1 + p2*(1/p1-1))
* Choose p1 as the larger of the two... does this lead to anything?
* Yes. Two tables of size 771 (= ceil(log(2)*40*64)) will allow you
* to evaluate (1/p1-1) directly, guaranteed to round down, and then
* I think something similar will do for (t+1). For the (1/p1-1)
* evaluation, do the entries for p1 from 1 to 1/2 directly, and
* past that, the value of (1/p1-1) differes from the value of
* 1/p1 by at most 771. Record in another table the values at
* which the delta changes, do a binary search, and use the index as
* the delta.
*/
#include <math.h>
#ifndef M_LN10
#define M_LN10	2.30258509299404568402	/* ln(10) */
#endif
static word16
mergetrust(word16 t1, word16 t2)
{
	static double const k = -M_LN10 / PGP_TRUST_DECADE;
	double p, q;

	/* Saturate at infinity */
	if (t1 == (word16) PGP_TRUST_INFINITE)
		return t2;
	if (t2 == (word16) PGP_TRUST_INFINITE)
		return t1;
	p = exp(t1*k);
	q = exp(t2*k);

	/* Round down for conservative estimate */
	return (word16)floor(log(p + q - p*q)/k);
}

/*
* Compute the trust value associated with a key. This is the
* weight given to signatures made by that key. The decision is based
* on the validity of the key's name (how sure are we that the name
* belongs to that key) and confidence (how much do we trust the
* named individual). The product of those two gives the trust in
* the key as a whole.
*
* If the key is a buckstop key, ignore the validity and just take the
* confidence as-is.
*
* If there are multiple names on a key, compute the trust for each and
* use the maximum.
*
* BESTVALID alternate algorithm
*
* If BESTVALID if set, then the overall trust in a key is determined
* from the name with the highest validity. In the case of a tie,
* then the best validity*confidence is taken. Thus, overall trust
* is computed from the name this key is most likely to belong to.
*/
#ifndef BESTVALID
#define BESTVALID 1
#endif

static word16
calctrust(struct RingKey const *k, ringmask mask)
{
	union RingObject const *n;
	word16 cur, best = 0;
#if BESTVALID
	word16 bestvalid = 0;
#endif

	pgpAssert(KEYISKEY(k));

	/* For BUCKSTOP keys, validity and confidence are infinite.
	Revoked or expired keys have no validity or confidence. */
	if (k->trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
		return 0;
	if (k->trust & PGP_KEYTRUSTF_BUCKSTOP)
		return PGP_TRUST_INFINITE;

	for (n = k->down; n; n = n->g.next) {
#if BESTVALID
		if (OBJISNAME(n) && (n->g.mask & mask) &&
		n->n.valid >= bestvalid)
#else
		if (OBJISNAME(n) && (n->g.mask & mask))
#endif
		{
			cur = n->n.confidence;
			if (cur == PGP_NEWTRUST_INFINITE)
				cur = n->n.valid;
			else if (cur == PGP_NEWTRUST_UNDEFINED)
				cur = 0;
			else
				cur = mergetrust(cur << TRUST_CERTSHIFT,
				n->n.valid);
#if BESTVALID
			if (n->n.valid > bestvalid ||
			 		(n->n.valid == bestvalid && cur > best)) {
			 		best = cur;
			 			bestvalid = n->n.valid;
			 	}
	#else
			if (cur > best)
				best = cur;
#endif
		}
	}
	return best;
}

word16
ringKeyCalcTrust(struct RingSet const *set, union RingObject *key)
{
	word16 trust;
	pgpAssert(OBJISKEY(key));
	pgpAssert(set->mask & key->g.mask);
	trust = calctrust (&key->k, set->mask);
	if (trust == PGP_TRUST_INFINITE)
	return trust;
	return trust;
}
	
#endif /* PGPTRUSTMODEL>0 */


#if PGPTRUSTMODEL==1

/*
* Add confidence to the name signed by the given sig, and if the name
* has confidence and the key has not already been processed, add it
* to the list. Return the expanded list.
*
* TODO: Do something sensible with signatures on keys.
*/
static struct RingKey *
mntDoSig(struct RingSig const *sig, struct RingKey *list,
	word16 confidence, word32 const timenow, int const level,
	int pass2)
{
	union RingObject *name, *key;
	int i;
	byte sigtype;

	pgpAssert(mntSigIsValid(timenow, sig->tstamp, sig->validity));

	/* Okay, it hasn't expired, check that we have a name signature */
	name = sig->up;

	if (!OBJISNAME(name)) {
		/* @@@ Do anything here with signatures on keys? */
		return list;
	}

	sigtype = sig->type & 0xF0;
	if (sigtype != PGP_SIGTYPE_KEY_GENERIC)
		return list;
	key = name->n.up;
	pgpAssert(OBJISKEY(key));
	/* Do not add confidence from a key to itself */
	if (key == sig->by)
		return list;
		/*
	* There are two passes: first we add trust from keys at one level
	* to keys on the next level, then we add trust from keys at
	* that level to each other. Pass2 is a flag controlling which
	* we do.
	* Oh, names which we have no confidence in as introducers
	* get their certainty added from all levels, since they cannot
	* participate in cycles. This is done during pass 1.
	* So, during pass 1:
	* - Ignore names on keys that we have already recorded that are
	* at levels less than the current one.
	* And during pass 2:
	* - Ignore names other than ones at the specified level on keys
	* that *are* trusted. (On keys that are not will get added
	* next iteration).
		*/
	i = NAMELEVEL(&name->n);
	if (!pass2) {
		/* Pass 1 */
		if (!i)
			NAMESETLEVEL(&name->n, level);
		else if (i < level && key->g.flags & KEYF_TRUSTED)
			return list;
	} else {
		/* Pass 2 */
		if (i != level || !(key->g.flags & KEYF_TRUSTED))
			return list;
	}

	/* Okay, now add the confidence if needed. Do special
	checks for infinite confidence and validity. */

	if (confidence == PGP_TRUST_INFINITE)
		name->n.valid = PGP_TRUST_INFINITE;
	else if (name->n.valid != PGP_TRUST_INFINITE) {
		name->n.valid += confidence;
		/*
		* Saturate on overflow. 65535 is reserved for "perfect".
		* 65534 is 2.5164 * 10^-26, 2^-85.
		*/
		if (name->n.valid + 1 <= confidence)
			name->n.valid = (word16) PGP_TRUST_MAX;
			}

			/*
			* Don't add keys to the list if:
			* - Already on list
			* - Key is revoked (confidence is zero then)
			* - Key is expired (same as revoked)
			* - Name has no confidence
			* Currently, disabled keys *are* allowed on the list,
			* to participate in trust transactions. Disabling only
		* affects key selection. This is PGP 2.x compatible.
			*
			* Once a key is on the list, only one more round of adding
			* certainty to its names is possible, since two rounds could
			* create cycles that correcting for is difficult. Thus,
			* we want to avoid putting things on the list unnecessarily,
			* but prefer to weed out everything possible now.
			*/
	if (key->g.flags & KEYF_TRUSTED	/* Already listed? */
	|| key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED)
/*	|| key->k.trust & PGP_KEYTRUSTF_DISABLED */ /*PGP 2.x compatible*/
	|| !name->n.confidence)
		return list;
	key->k.util = (struct RingKey *)list;
	key->g.flags |= KEYF_TRUSTED;
	return &key->k;
}

/*
* Add confidence to each name signed by a valid signature on
* the "sigs" list. If the trust passes "thresh", add the resultant
* key to the list. Return the expanded list.
*
* Signatures that are expired or have been superseded are weeded out.
* Note that a postdated signature does not supersede one dated yesterday!
*/
static struct RingKey *
mntWalkSigList(struct RingSig const *sigs, struct RingKey *list,
	word16 confidence, ringmask const mask, word32 const timenow,
	int const level, int pass2)
{
	struct RingSig const *cur;

	while (sigs) {
		/* Find the first valid checked signature under the mask. */
		if (!(sigs->trust & PGP_SIGTRUSTF_CHECKED)
		|| !(sigs->mask & mask)
		|| !mntSigIsValid(timenow, sigs->tstamp, sigs->validity))
		{
			sigs = sigs->nextby;
			continue;
		}
		/* The first candidate signature */
		cur = sigs;

			/*
		* Search all other signatures by this key on the same object
		* for the most recent valid signature, which is the only
		* one which is accorded any weight. We use the fact that
		* after ringPoolListSigsBy places all signatures on
		* the same object together in the sigsby list.
				*/
		while ((sigs = sigs->nextby) != NULL && sigs->up == cur->up)
			 {
			 		/*
			 		* So now that the signature at the front of the sigs
			 		* list is on the same thing as "cusrig", consider
			 		* replacing cur, if the new signature is is
			 		* checked good, valid, under the right mask, and
			 		* more recent than cur.
			 		*
			 		*/
			if (sigs->trust & PGP_SIGTRUSTF_CHECKED
			&& sigs->mask & mask
			&& cur->tstamp < sigs->tstamp
			&& mntSigIsValid(timenow, sigs->tstamp,
			 		sigs->validity))
				cur = sigs;
			 }

		/* Do the trust computations on the resultant signature. */
		list = mntDoSig(cur, list, confidence, timenow, level, pass2);
	}
	return list;
}

/*
* Walk a list of keys (linked through the util field), adding the
* appropriate trust values to the names the key has signed.
*/
	static struct RingKey const *
	mntList(struct RingKey const *list, ringmask const mask,
		word32 const timenow, unsigned const level)
	{
		struct RingKey *newlist;
		struct RingKey *cur;
		word16 confidence;

		/* Do first pass, adding trust from old list to new things */
		for (newlist = NULL; list; list = list->util) {
			 pgpAssert(list->flags & KEYF_TRUSTED);
			 confidence = calctrust(list, mask);
			 if (!confidence)
			 	continue;
		newlist = mntWalkSigList(&list->sigsby->s, newlist, confidence,
		mask, timenow, level, 0);
			}
			/*
			* Do second pass, adding trust from new list to itself.
			* The overall confidence for each key must be calculated
			* before the second pass begins to avoid artificially high
			* confidence values caused by loops. Using a new field
			* 'list->confidence' increases the size of each key object
			* by 2 bytes (@@@ can we avoid this?)
			*/
	for (cur = newlist; cur; cur = cur->util) {
		pgpAssert(cur->flags & KEYF_TRUSTED);
		cur->confidence = calctrust(cur, mask);
	}
	for (cur = newlist; cur; cur = cur->util) {
		if (!cur->confidence)
			continue;
		newlist = mntWalkSigList(&cur->sigsby->s, newlist,
				cur->confidence,
				mask, timenow, level, 1);
		}

return newlist;
}

#endif /* PGPTRUSTMODEL==1 */

#if SORT_NAMES
/*
* Return 1 if n1 should be listed before n2; 0 if they're
* equivalent and the relative order should be unchanged, and -1
* if it should be the other way around.
*
* All non-names should be before all names; non-names are left in
* their original order.
*/
static int
mntCompareNames(union RingObject const *n1, union RingObject const *n2,
	word32 const timenow)
{
	word32 t1, t2;
	union RingObject const *sig;
	int i;

	/* Test 0: non-names always come first */
	if (!OBJISNAME(n1))
		return OBJISNAME(n2) ? 1 : 0;
	if (!OBJISNAME(n2))
		return -1;

	/* Test 1: more trusted */
	i = (n1->n.trust & PGP_NAMETRUST_MASK) -
		(n2->n.trust & PGP_NAMETRUST_MASK);
	if (i)
		return i > 0 ? 1 : -1;

	/* Test 2: more recent self-signature */
	t1 = 0;
	for (sig = n1->g.down; sig; sig = sig->g.next) {
		if (!OBJISSIG(sig) || sig->s.by != n1->g.up)
			continue;	 /* Not a self-sig */
		if ((sig->s.trust & PGP_SIGTRUSTF_CHECKED) == 0)
			continue;	 /* Not valid */
		if (sig->s.tstamp <= t1)
			continue;	 /* Too old */
		if (timenow > sig->s.tstamp &&
			(unsigned)((timenow-sig->s.tstamp) / 86400) > sig->s.validity-1u)
			continue;	 /* expired */
		t1 = sig->s.tstamp;
	}

	t2 = 0;
	for (sig = n2->g.down; sig; sig = sig->g.next) {
		if (!OBJISSIG(sig) || sig->s.by != n1->g.up)
			continue;	 /* Not a self-sig */
		if ((sig->s.trust & PGP_SIGTRUSTF_CHECKED) == 0)
			continue;	 /* Not valid */
		if (sig->s.tstamp <= t2)	
			continue;	 /* Too old */
		if (timenow > sig->s.tstamp &&
			(unsigned)((timenow-sig->s.tstamp) / 86400) > sig->s.validity-1u)
			continue;	 /* expired */
		t2 = sig->s.tstamp;
	}
	if (t1 != t2)
		return (t1 > t2 ? 1 : -1);

#if PGPTRUSTMODEL==0
	/* Test 3: lower certification depth */
	if (NAMELEVEL(&n1->n) != NAMELEVEL(&n2->n)) {
		return ((unsigned)NAMELEVEL(&n1->n) - 1u <
		(unsigned)NAMELEVEL(&n2->n) - 1u) ? 1 : -1;
	}

	/* Test 4: more trust accumulated */
	if (n1->n.trustval != n2->n.trustval)
		return n1->n.trustval > n2->n.trustval ? 1 : -1;
#else
	/* Test 3: More certainty */
	if (n1->n.valid != n2->n.valid)
		return n1->n.valid > n2->n.valid ? 1 : -1;

	/* Test 4: lower certification depth */
	if (NAMELEVEL(&n1->n) != NAMELEVEL(&n2->n)) {
		return ((unsigned)NAMELEVEL(&n1->n) - 1u <
		(unsigned)NAMELEVEL(&n2->n) - 1u) ? 1 : -1;
	}
#endif

	/* Okay, give up - declare 'em equal */
	return 0;
}

/*
* Sort the names in the given list according to a "goodness" measure.
* This may result in some keyrings getting dirtied. The mask of such
* keyrings is returned.
*
* This is a selection sort, not wonderfully efficient, but n^2 is okay
* for the small n we have here (5 names on a key is pretty unusual!),
* and it makes figuring out which keyrings need dirtying much easier.
*
* The technique is perhaps not entirely obvious, so I'll explain it.
* Selection sort involves searching the unsorted list for the object (name)
* that should go first (referred to as the best element in the code), and
* then removing it and placing it at the end of the (initially empty)
* sorted new list. We keep track of the union of the masks of the objects
* that the best object skips in front of. When we reach the end of the
* unsorted list, and the best pointer (and the bestmask) is known for
* sure, all keyrings which the best object is in and have been skipped
* over (i.e. bestmask & best->mask) are dirty.
*/
static ringmask
mntSortNameList(union RingObject **objp, word32 const timenow)
{
	union RingObject *newhead, **newtail;
	union RingObject *cur, **curp;
	union RingObject *best, **bestp;
	ringmask curmask, bestmask;
	ringmask dirtymask = 0;

	newtail = &newhead;

	/* Loop until unsorted list is empty */
	while (*objp) {
		/* Default best is head of list */
		bestp = objp;
		best = *bestp;
		bestmask = 0;

		/* Search rest of list for a better name */
		curp = &best->g.next;
		curmask = best->g.mask;
		while ((cur = *curp) != 0) {
			if (mntCompareNames(best, cur, timenow) < 0) {
				bestp = curp;
				best = cur;
				bestmask = curmask;
			}
			curmask |= cur->g.mask;
			curp = &cur->g.next;
		}

		*bestp = best->g.next;
		dirtymask |= bestmask & best->g.mask;
		*newtail = best;
		newtail = &best->g.next;
	}
	*objp = newhead;
	*newtail = NULL;
	return dirtymask;
}

/*
* Sort the names of each key under the given mask with the mntSortNameList
* function. The mask applies only to the keys; all names are sorted,
* even if they don't fall under the mask. And any keyrings may be dirtied
* by this operation.
*/
static void
mntSortNames(struct RingSet const *set, word32 const timenow)
{
	union RingObject *key;
	ringmask dirtymask = 0;

	/* Sort all the keys */
	for (key = set->pool->keys; key; key = key->g.next) {
		pgpAssert(OBJISKEY(key));
		if (key->g.mask & set->mask)
			dirtymask |= mntSortNameList(&key->g.down, timenow);
	}

	/* Deal with the dirtymask */
	ringPoolMarkDirty(set->pool, dirtymask);
}
#endif /* SORT_NAMES */

/*
* Add the "interesting" objects under the given parent to the destmask.
* An signature is interesting if it's by the given key or by a trusted
* introducer. A name is interesting if its trust is above a given threshold.
* Anything else (the only real choice is a secret) is by definition
* interesting.
*/
#if PGPTRUSTMODEL==0
static void
mntMarkInteresting(union RingObject *obj, union RingObject const *key,
	int thresh, ringmask srcmask, ringmask destmask)
{
	for (obj = obj->g.down; obj;obj = obj->g.next) {
		if (!(obj->g.mask & srcmask))
			continue;
		switch(ringObjectType(obj)) {
		case RINGTYPE_SIG:
			if (obj->s.by == key
			|| (obj->s.by->g.flags & KEYF_TRUSTED
			&& (obj->s.by->k.trust & PGP_KEYTRUST_MASK)
			 		>= PGP_KEYTRUST_MARGINAL))
			 		 break;
			 	continue;
		case RINGTYPE_NAME:
			if (obj->n.trustval >= thresh)
				break;
			continue;
		default:
			break;
		}
		obj->g.mask |= destmask;
		if (!OBJISBOT(obj))
			mntMarkInteresting(obj, key, thresh,
			srcmask, destmask);
	}
}
#else
static void
mntMarkInteresting(union RingObject *obj, union RingObject const *key,
		word16 thresh, ringmask srcmask, ringmask destmask)
{
	for (obj = obj->g.down; obj;obj = obj->g.next) {
		if (!(obj->g.mask & srcmask))
			continue;
		switch(ringObjectType(obj)) {
		case RINGTYPE_SIG:
			if (obj->s.by == key
			|| (obj->s.by->g.flags & KEYF_TRUSTED
			&& (obj->s.by->k.confidence > 0)))
				break;
			continue;
		case RINGTYPE_NAME:
			if (obj->n.valid >= thresh)
				break;
			continue;
		default:
			break;
		}
		obj->g.mask |= destmask;
		if (!OBJISBOT(obj))
			mntMarkInteresting(obj, key, thresh,
			 		srcmask, destmask);
		}
	}
	#endif


	/*
* Run the maintenance pass on the keyring. At the end, any keys which
* are flagged with KEYF_TRUSTED but still have PGP_KEYTRUST_UNDEFINED need
* to have their trusts updated. Returns the number of such keys, or
* <0 on error. (Currently impossible.)
	*/
int
ringMnt(struct RingSet const *set, struct RingSet *dest, word32 const timenow)
{
	struct RingKey const *list;
	union RingObject *key, *child;
	struct RingSig *sig;
	unsigned level;	/* Trust level and certification depth */
	unsigned tmptrust;
	unsigned t;
	unsigned long count;	/* Count of keys needing trust assigned */
	ringmask mask = set->mask;
	ringmask changemask = 0;	/* Keyrings with altered trust */
#if PGPTRUSTMODEL>0
	word16 validity;
	union RingObject *obj;
#else
	int trust[PGP_KEYTRUST_MASK + 1];
	int thresh;

		/*
	* Set up the trust table. The non-zero entries are 1/partials,
	* 1/completes, and 1, all scaled by completes*partials, except
	* that completes or partials set to 0 means "infinite".
		*/
	for (t = 0; t < sizeof(trust)/sizeof(*trust); t++)
		trust[t] = 0;
	trust[PGP_KEYTRUST_MARGINAL] = 1;
	trust[PGP_KEYTRUST_COMPLETE] = 1;
	trust[PGP_KEYTRUST_ULTIMATE] = 1;
	if (set->pool->num_marginals) {
		trust[PGP_KEYTRUST_COMPLETE] = set->pool->num_marginals;
		trust[PGP_KEYTRUST_ULTIMATE] = set->pool->num_marginals;
	} else {
		trust[PGP_KEYTRUST_MARGINAL] = 0;
	}
	if (set->pool->num_completes) {
		trust[PGP_KEYTRUST_MARGINAL] = set->pool->num_completes;
		trust[PGP_KEYTRUST_ULTIMATE] *= set->pool->num_completes;
	} else {
		trust[PGP_KEYTRUST_COMPLETE] = 0;
	}
	thresh = trust[PGP_KEYTRUST_ULTIMATE];
#endif

	pgpAssert(!dest || set->pool == dest->pool);
	pgpAssert(!dest || RINGSETISMUTABLE(dest));

	/*
	* Preprocessing: reset all name trusts to 0, and initialize list
	* of trusted keys to the axiomatic ones.
	*/
	list = 0;
	for (key = set->pool->keys; key; key = key->g.next) {
		pgpAssert(OBJISKEY(key));
		if ((key->g.mask & mask) == 0)
			continue;

		key->g.flags &= ~KEYF_TRUSTED;
#if PGPTRUSTMODEL==2
		CLEARLOOKINGAT(&key->k);
#endif
		for (child = key->k.down; child; child = child->g.next) {
			if (child->g.mask & mask && OBJISNAME(child)) {
#if PGPTRUSTMODEL==0
				child->n.trustval = 0;
#else
				child->n.valid = 0;
#endif
				NAMESETLEVEL(&child->n, 0);
					}
				}
				/*
		* Find each key's revoked status - find
		* the most important self-signature.
				*/
				sig = 0;
		for (child = key->g.down; child; child = child->g.next) {
			 		/*
			 	* Ignore sigs by others, unchecked sigs,
			* postdated sigs, and expired sigs.
			* @@@ TODO: change the CHECKED and TRIED bits
			* For untried revocation sigs, accept them if key's
			* trust bits already show it as revoked. PGP 2 does
			* not store trust packets on revocation signatures.
			 		*/
			if ((child->g.mask & mask)
				&& OBJISSIG(child)
			 			&& child->s.by == key
			 			&& child->s.trust & PGP_SIGTRUSTF_CHECKED
			 			&& child->s.trust & PGP_SIGTRUSTF_TRIED
			 			&& mntSigIsValid(timenow, child->s.tstamp,
			 			child->s.validity)) {

			 			/* Any compromise certificate wins, period. */
			 			if (child->s.type == PGP_SIGTYPE_KEY_COMPROMISE) {
			 				sig = &child->s;
					break;
				}
				/* Among others, choose the most recent */
				if (!sig ||
					sig->tstamp < child->s.tstamp)
				sig = &child->s;
			}
		}

		tmptrust = key->k.trust;
		/* Revocation is a function of compromise certificates.
		Make sure axiomatic (buckstop) bit is not set. */
		if (sig && sig->type == PGP_SIGTYPE_KEY_COMPROMISE) {
			tmptrust &= ~(PGP_KEYTRUSTF_BUCKSTOP | PGP_KEYTRUST_MASK);
			tmptrust |= PGP_KEYTRUST_NEVER | PGP_KEYTRUSTF_REVOKED;
		}
		else
			tmptrust &= ~PGP_KEYTRUSTF_REVOKED;

		/* Check if key has expired */
		if (key->k.tstamp < timenow && key->k.validity > 0 &&
			(unsigned)((timenow - key->k.tstamp)/86400) >= key->k.validity) {
			tmptrust &= ~(PGP_KEYTRUSTF_BUCKSTOP | PGP_KEYTRUST_MASK);
			tmptrust |= PGP_KEYTRUST_NEVER | PGP_KEYTRUSTF_EXPIRED;
		}
		else
			tmptrust &= ~PGP_KEYTRUSTF_EXPIRED;

		if (key->k.trust != tmptrust) {
			key->k.trust = (byte)tmptrust;
			key->g.flags |= (RINGOBJF_TRUST | RINGOBJF_TRUSTCHANGED);
			changemask |= key->g.mask;
		}

		/* Check any subkeys for revocation or expiration. */
		for (child = key->g.down; child; child = child->g.next) {
		if (child->g.mask & mask && OBJISSUBKEY(child)) {
			tmptrust = child->k.trust;
				tmptrust &= ~(PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED);
				for (sig = (struct RingSig *) child->g.down; sig;
					sig = (struct RingSig *) sig->next) {
			if (sig->by == key &&
						sig->trust & PGP_SIGTRUSTF_TRIED &&
						sig->trust & PGP_SIGTRUSTF_CHECKED &&
						sig->type == PGP_SIGTYPE_KEY_SUBKEY_REVOKE &&
						mntSigIsValid (timenow, sig->tstamp,
									sig->validity)) {
				tmptrust |= PGP_KEYTRUSTF_REVOKED;
						break;
					}
				}
				if (child->k.tstamp < timenow && child->k.validity > 0 &&
					(unsigned)((timenow - child->k.tstamp)/86400) >=
					child->k.validity)
			tmptrust |= PGP_KEYTRUSTF_EXPIRED;
				
				if (child->k.trust != tmptrust) {
				child->k.trust = (byte) tmptrust;
					child->g.flags |= (RINGOBJF_TRUST | RINGOBJF_TRUSTCHANGED);
					changemask |= child->g.mask;
				}
			}
		}

		/* Search for axiomatic keys at root of graph. Buckstop bit must be
		set, and key must not be revoked or expired. */
		if (key->g.mask & mask
					&& key->k.trust & PGP_KEYTRUSTF_BUCKSTOP
					&& !(key->k.trust & (PGP_KEYTRUSTF_REVOKED|PGP_KEYTRUSTF_EXPIRED))
	/*	 			&& !(key->k.trust & PGP_KEYTRUSTF_DISABLED) */	)
				{
					/* Make sure key marked as axiomatic has a secret component.
			 			(This may not be true if a public keyring is opened, but
			 			*not* its corresponding secret keyring). If not, reset
			 			the buckstop bit and set trust to 'never'. */
					tmptrust = key->k.trust & ~PGP_KEYTRUSTF_BUCKSTOP;
					for (child = key->g.down; child; child = child->g.next) {
			 			if (OBJISSEC(child) && (child->g.mask & mask)) {
				tmptrust |= PGP_KEYTRUSTF_BUCKSTOP;
					break; /* secret component found */
				}
			}
			if (!(tmptrust & PGP_KEYTRUSTF_BUCKSTOP)) {
			/* Don't have private key */
			tmptrust &= ~PGP_KEYTRUST_MASK;
				tmptrust |= PGP_KEYTRUST_NEVER;
			key->k.trust = tmptrust;
				changemask |= key->g.mask;
				key->g.flags |= (RINGOBJF_TRUST | RINGOBJF_TRUSTCHANGED);
			}
			else {
			/* Yes, we do have private key */
#if PRINT_PROGRESS
			ringKeyPrint(stdout, "Axiomatic key: ", set, key);
#endif
				key->k.util = (struct RingKey *)list;
				list = &key->k;
				key->g.flags |= KEYF_TRUSTED;
#if PGPTRUSTMODEL==0
				/* Axiomatic key should have ultimate trust */
				if ((key->k.trust & PGP_KEYTRUST_MASK) !=
					PGP_KEYTRUST_ULTIMATE) {
				key->k.trust &= ~PGP_KEYTRUST_MASK;
					key->k.trust |= PGP_KEYTRUST_ULTIMATE;
					changemask |= key->g.mask;
					key->g.flags |= (RINGOBJF_TRUST | RINGOBJF_TRUSTCHANGED);
				}
#else
				for (child = key->g.down;child;child = child->g.next) {
				if (OBJISNAME(child))
					child->n.valid = (word16) PGP_TRUST_INFINITE;
				}
#endif /* PGPTRUSTMODEL>0 */
			}
		}
	}

	/* The actual trust computation */

	ringPoolListSigsBy(set->pool);	/* Canonicalize these pointers */

	#if PGPTRUSTMODEL==2

		{
	PathList	*pathlist = NULL;
	RingObject	*name;
			 RingSet			*confidentset;
	unsigned	maxdepth = set->pool->certdepth;
	double	confidence;

		confidentset = ringSetConfidentSet (set);
		for (key = set->pool->keys; key; key = key->g.next) {
			pgpAssert(OBJISKEY(key));
			if ((key->g.mask & mask) == 0)
				continue;

			for (name = key->g.down; name; name = name->g.next) {
				if (!OBJISNAME(name) || !(name->g.mask & mask))
					continue;
				/* Calculate "real" validity for names. If the key
					is axiomatic, this will be overidden when the
					validity is returned by	ringNameValidity. */
#if 0
				if (key->k.trust & PGP_KEYTRUSTF_BUCKSTOP
					&& !(key->k.trust &
						(PGP_KEYTRUSTF_REVOKED|PGP_KEYTRUSTF_EXPIRED)
					/* && !(key->k.trust & PGP_KEYTRUSTF_DISABLED) */) {
					name->n.valid = PGP_TRUST_INFINITE;
					continue;
				}
#endif
				ringFindPathsBack (name, &pathlist, /*maxdepth*/100,
					confidentset, timenow, set->pool);
				if (pathlist) {
					pathlist = pathListPrune (pathlist, TOTALPATHMAX,
					set->pool);
#ifdef DEBUGPATH
					fprintf (stdout, "Found path for ");
					ringTtyPrintKey (stdout, key, set->pool);
					fprintf (stdout, "\n");
#endif
					confidence = pathListConfidence (pathlist, set->pool);
					if (confidence >= 1.)
						name->n.valid = PGP_TRUST_INFINITE;
					else
						name->n.valid = ringDoubleToTrust (1. /
			 		 		 		 		 		 			(1.-confidence));
			 		 	pathListFreeAll (pathlist, set->pool);
			 		 	pathlist = NULL;
					}
			 	}
			 }
			 memPoolEmpty(&set->pool->pathpool);
			 set->pool->paths = NULL;
			 set->pool->pathlists = NULL;
			 ringSetDestroy (confidentset);
		}
			 	
	#else /* PGPTRUSTMODEL!=2 */
		/* Propagate the trust upwards */
		t = (unsigned)set->pool->certdepth;
		for (level = 1; list && level < t; level++) {
	#if PRINT_PROGRESS
		printf("\nProcessing certification level %d\n", level);
#endif
#if PGPTRUSTMODEL==0
		list = mntList(list, set, trust, thresh, timenow, level);
#else
		list = mntList(list, mask, timenow, level);
#endif
	}
#endif /* PGPTRUSTMODEL */

	/* Postprocessing: set the various trust bytes, */
	count = 0;
#if PGPTRUSTMODEL==0
	for (key = set->pool->keys; key; key = key->g.next) {
		if (!(key->g.mask & mask))
			continue;
		/* Set the key trust bytes */
		t = key->k.trust;
		if (t & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED)) {
			tmptrust = PGP_KEYTRUST_NEVER;
		}
#if ONLY_TRUST_VALID_KEYS
	else if (!(key->k.flags & KEYF_TRUSTED)) {
			tmptrust = PGP_KEYTRUST_UNDEFINED;
		}
#endif
	else if ((tmptrust = (t&PGP_KEYTRUST_MASK)) ==
				PGP_KEYTRUST_UNDEFINED) {
			/* Key in need of a trust decision */
			count++;
			/* Record the key as interesting */
			if (dest) {
				key->g.mask |= dest->mask;
				mntMarkInteresting(key, key, thresh,
				mask, dest->mask);
			}
		}

		t = tmptrust | (key->k.trust & ~PGP_KEYTRUST_MASK);
		if (key->k.trust != t) {
			key->k.trust = t;
			key->g.flags |= RINGOBJF_TRUSTCHANGED;
			changemask |= key->g.mask;
		}
		key->g.flags |= RINGOBJF_TRUST;

		/* Set the signature trusts to match the key */
		for (sig = &key->k.sigsby->s; sig; sig = sig->nextby) {
			if (!(sig->mask & mask))
				continue;
			/* Trust is either key's, or NEVER if it's a bad sig */
			t = sig->trust & ~PGP_KEYTRUST_MASK;
			if (sig->trust & PGP_SIGTRUSTF_CHECKED)
				t |= tmptrust;
			else
				t |= PGP_KEYTRUST_NEVER;
			if (sig->trust != t)
			{
				sig->trust = t;
				sig->flags |= RINGOBJF_TRUSTCHANGED;
				changemask |= sig->mask;
			}
			sig->flags |= RINGOBJF_TRUST;
		}

		/* Update each name's trust byte */
		for (child = key->k.down; child; child = child->g.next) {
			if (!(child->g.mask & mask) || !OBJISNAME(child))
				continue;
			t = child->n.trust & ~PGP_NAMETRUST_MASK;
			if (key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
				t |= PGP_NAMETRUST_UNTRUSTED;
			else if (child->n.trustval >= thresh)
				t |= PGP_NAMETRUST_COMPLETE;
			else if (2 * child->n.trustval >= thresh)
				t |= PGP_NAMETRUST_MARGINAL;
			else
				t |= PGP_NAMETRUST_UNTRUSTED;

			if (child->n.trust != t) {
				child->n.trust = t;
				child->g.flags |= RINGOBJF_TRUSTCHANGED;
				changemask |= child->g.mask;
			}
			child->g.flags |= RINGOBJF_TRUST;
		}
	}
#else /* PGPTRUSTMODEL>0 */

	/* Note: For the new trust model, we need to find keys that
	have interesting names, as trust is no longer assigned to
	the key, but to each name. */

	for (key = set->pool->keys; key; key = key->g.next) {
		if (!(key->g.mask & mask))
			continue;

		/* Set the key trust bytes. The overall confidence
		value must be calculated (k.confidence contains
		the first-pass confidence value prior to this point) */

		key->k.confidence = calctrust(&key->k, mask);
		t = key->k.trust;
		if (t & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED)
		|| !(key->k.flags & KEYF_TRUSTED)) {
			key->k.confidence = 0;
		}
		else if (key->k.confidence == 0) {

			/* If at least one name has undefined confidence,
			mark the key as interesting. */

			for (obj = key->k.down; obj; obj = obj->g.next) {
				if (OBJISNAME(obj) &&
				obj->n.confidence==PGP_NEWTRUST_UNDEFINED) {
				count++;
					/* Record the key as interesting */
					if (dest) {
						key->g.mask |= dest->mask;
						mntMarkInteresting(key, key, 0,
			 		 		 		mask, dest->mask);
					}
					break;
				}
			} /* end for */
		}

		tmptrust = key->k.trust & PGP_KEYTRUST_MASK;
		/* Set the signature trusts to match the key */
		for (sig = &key->k.sigsby->s; sig; sig = sig->nextby) {
			if (!(sig->mask & mask))
				continue;
			/* Trust is either key's, or NEVER if it's a bad sig */
			t = sig->trust & ~PGP_KEYTRUST_MASK;
			if (sig->trust & PGP_SIGTRUSTF_CHECKED)
				t |= tmptrust;
			else
				t |= PGP_KEYTRUST_NEVER;
			if (sig->trust != t)
			{
				sig->trust = t;
				sig->flags |= RINGOBJF_TRUSTCHANGED;
				changemask |= sig->mask;
			}
			sig->flags |= RINGOBJF_TRUST;
		}

		/* Update each name's validity byte */
		for (child = key->k.down; child; child = child->g.next) {
			if (!(child->g.mask & mask) || !OBJISNAME(child))
				continue;
			if (key->k.trust & (PGP_KEYTRUSTF_REVOKED | PGP_KEYTRUSTF_EXPIRED))
				validity = 0;
			else if (child->n.valid == PGP_TRUST_INFINITE)
				validity = PGP_NEWTRUST_INFINITE;
			else {
				validity = child->n.valid >> TRUST_CERTSHIFT;
				if (validity > PGP_NEWTRUST_MAX)
					validity = PGP_NEWTRUST_MAX;
			}
			if (validity != child->n.validity) {
				child->n.validity = (byte) validity;
				child->g.flags |= RINGOBJF_TRUSTCHANGED;
				changemask |= child->g.mask;
			}

			/* Convert back to old KEYLEGIT values. */
			if (validity >= set->pool->threshold)
				tmptrust = PGP_NAMETRUST_COMPLETE;
			else if (validity >= set->pool->threshold/2)
				tmptrust = PGP_NAMETRUST_MARGINAL;
			else
				tmptrust = PGP_NAMETRUST_UNTRUSTED;
			t = tmptrust | (child->n.trust & ~PGP_NAMETRUST_MASK);
			if (t != child->n.trust) {
				child->n.trust = t;
				child->g.flags |= RINGOBJF_TRUSTCHANGED;
				changemask |= child->g.mask;
			}

			/* Reset n.valid with adjusted value. */
			if (child->n.validity == PGP_NEWTRUST_INFINITE)
				child->n.valid = PGP_TRUST_INFINITE;
			else
				child->n.valid =
				child->n.validity << TRUST_CERTSHIFT;
			child->g.flags |= RINGOBJF_TRUST;
		}
	}
#endif

	/* Rebuild the hash table */
	ringPoolHash(set->pool);

	/* Mark all keyrings under "changemask" as having had trust change */
	ringPoolMarkTrustChanged(set->pool, changemask);

	/* Sort the names @@@ do we want this? Not for now... */
#if SORT_NAMES
	mntSortNames(set, timenow);
#endif

	/* Et voila, we're done! Return number of trust decisions needed. */
	/* (Saturate on overflow if 16-bit ints.) */
	return count > INT_MAX ? INT_MAX : (int)count;
}


/*
* Keyring checking code.
*/

/*
* Count the number of unchecked signatures, if needed for a progress bar.
* The return value is the number of times the progress func() function
* will be called by ringPoolCheck().
* NOTE: this tries to be as accurate as possible, but user code
* should not die if it is slightly incorrect.
	*/
int
ringPoolCheckCount(struct RingSet const *sigset, struct RingSet const *keyset,
	int allflag)
{
	ringmask sigmask = sigset->mask;
	ringmask keymask = keyset->mask;
	union RingObject const *key;
	struct RingSig const *sig;
	unsigned long count = 0;

	pgpAssert(keyset->pool == sigset->pool);
	ringPoolListSigsBy(sigset->pool);

	for (key = sigset->pool->keys; key; key = key->g.next) {
		pgpAssert(OBJISKEY(key));
		if ((key->g.mask & keymask) == 0 || key->g.flags & KEYF_ERROR)
			continue;	 /* Skip dummy keys */
		for (sig = &key->k.sigsby->s; sig; sig = sig->nextby) {
				/*
			* Reasons why a signature might not be checked:
			* - Already tested and not allflag
			* - Not in sigmask
				*/
			if ((sig->mask & sigmask) != 0
			&& (allflag
				|| !(sig->trust&PGP_SIGTRUSTF_CHECKED_TRIED)))
				count++;
		}
	}

	return count > INT_MAX ? INT_MAX : (int)count;
}

/*
* Set the signature trust to the given value. Returns the bitmask of
* changed keyrings.
*/
static ringmask
setSigTrust(struct RingSig *sig, byte trust)
{
	if (sig->trust == trust)
		return 0;
	sig->trust = trust;
sig->flags |= RINGOBJF_TRUSTCHANGED;
return sig->mask;
}

/*
* Check the signatures on a keyring. Checks unchecked signatures,
* or all signatures if allflag is true.
*
* Checks signatures in "sigset" if they are made by keys in "keyset".
*
* If func is non-null, calls it with each checked signature, after
* checking. This is for progress indications.
*
* Question: should signatures be checked in order of the key they're
* on, or the key they're by? Should this be guaranteed? For now, use
* the 2.x order.
*/
int
ringPoolCheck(struct RingSet const *sigset, struct RingSet const *keyset,
	int allflag, void (*func)(void *, struct RingIterator *, int),
	void *arg)
{
	struct RingIterator *iter = NULL;
	union RingObject *obj, *key;
	union RingObject *sigparent=NULL, *sigowner=NULL;
	int sp = 0;	/* Iterator level (stack pointer) */
	union RingObject *hashed[RINGMAXDEPTH];
	struct PgpHashContext *hc;
	struct PgpHashContext *hcstack[RINGMAXDEPTH];
	struct PgpHash const *h, *hnew;
	byte const *buf, *extrabuf;
	struct PgpPubKey *pub;
	size_t len;
	unsigned extralen;
	ringmask changemask = 0;	/* Sets that have had trust changed */
	byte tmpbuf[5];
	byte t;						/* Trust on current signature */
	int sighashnew;				/* True if sig uses new hash conventions */
	int namehashnew=0;			/* True if name hash cache uses new "" */
	int i;
	union RingObject *name;

	pgpAssert(sigset->pool == keyset->pool);

	for (i = 0; i < RINGMAXDEPTH; i++) {
		hashed[i] = (union RingObject *)0;
		hcstack[i] = (struct PgpHashContext *)0;
	}

	iter = ringIterCreate(sigset);
	if (!iter)
		return PGPERR_NOMEM;

	h = (struct PgpHash const *)0;

	while ((sp = ringIterNextObjectAnywhere(iter)) > 0) {
		obj = ringIterCurrentObject(iter, (unsigned)sp);
		if (!OBJISSIG(obj))
			continue;
		t = obj->s.trust;

		/* See if we have the key */
		key = ringSigMaker(sigset, obj, keyset);

		/* If NOT allflag, and sig has been successfully checked,
			and signing key still exists, ignore it. */
		if (!allflag && (t & PGP_SIGTRUSTF_CHECKED_TRIED) && key) {
		ringObjectRelease (key);
			continue;
		}

		/* If the superior object is a name, reset the
		WARNONLY flag. */

		name = ringIterCurrentObject(iter,
					ringIterCurrentLevel(iter)-1);
		if (OBJISNAME(name) &&
				(name->n.trust & PGP_NAMETRUSTF_WARNONLY)) {
		name->n.trust &= ~PGP_NAMETRUSTF_WARNONLY;
		changemask |= name->g.mask;
		}

		/* We now have a signature to check - clear the bits */
		t &= ~PGP_SIGTRUSTF_CHECKED_TRIED;

		if (!key) {
			/* No key, so sig cannot be considered good. */
			changemask |= setSigTrust(&obj->s, t);
			continue;
		}

		/* The new hash style includes the name header and count */
		sighashnew = obj->s.version > PGPVERSION_2_6;

		/* Some reasons that we might not be able to check it */
		if (obj->g.flags & SIGF_ERROR
		|| key->g.flags & KEYF_ERROR
		|| (hnew = pgpHashByNumber(obj->s.hashalg)) == 0)
		{
			ringObjectRelease(key);
			goto uncheckable;
		}

		/* Get the key that made the signature */
		pub = ringKeyPubKey(keyset, key, PGP_PKUSE_SIGN);
		ringObjectRelease(key);
		if (!pub) {
			i = ringSetError(keyset)->error;
			/* @@@ ViaCrypt self-signs encryption-only keys
			(as they're RSA, it's possible), but PGP 3
			doesn't like this. Ignore this type of
			signature. */
			if ((i >= PGPERR_KEY_MIN && i <= PGPERR_KEY_MAX) ||
			i == PGPERR_PUBKEY_UNIMP)
				goto uncheckable;
			goto fatal;	/* I/O or memory error */
		}

		/* Okay, "hnew" is the current hash. Do checking. */
		i = 0;
		for (;;) {
			obj = ringIterCurrentObject(iter, ++i);
			hc = hcstack[i-1];
			if (hc && hc->hash != hnew) {
				pgpHashDestroy(hc);
				hc = 0;
			}
			if (!hc) {
				hc = pgpHashCreate(hnew);
				if (!hc)
					goto nomem;
				hcstack[i-1] = hc;
				hashed[i-1] = (union RingObject *)0;
			} else if (hashed[i-1] == obj) {
				/* Right hash, right object */
				/* Detect whether name has same hash style as we need */
				if (!OBJISNAME(obj) || namehashnew==sighashnew)
					continue;
			}

			buf = (byte const *)ringFetchObject(sigset, obj, &len);
			if (!buf)
				goto fatal;

			if (i == sp)
				break;	/* We've hit the signature */

			if (i == 1)
				pgpHashInit(hc);
			else
				pgpHashCopy(hc, hcstack[i-2]);

			/* We use this format even for subkeys */
			if (OBJISKEY(obj)) {
				tmpbuf[0] = PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1);
				tmpbuf[1] = (byte)(len>>8);
				tmpbuf[2] = (byte)len;
				pgpHashUpdate(hc, tmpbuf, 3);
				sigparent = obj;	/* Remember key above sig for below */
				if (OBJISTOPKEY(obj)) {
					sigowner = obj;	/* Also top key of sig for below */
				}
			} else {
				/* Not a signature, this must be a name */
				pgpAssert(i == 2);
				pgpAssert(OBJISNAME(obj));
				if (sighashnew) {
					tmpbuf[0] = PKTBYTE_BUILD(PKTBYTE_NAME, 0);
					tmpbuf[1] = (byte)(len>>24);
					tmpbuf[2] = (byte)(len>>16);
					tmpbuf[3] = (byte)(len>> 8);
					tmpbuf[4] = (byte)(len>> 0);
					pgpHashUpdate(hc, tmpbuf, 5);
				}
				namehashnew = sighashnew;
			}
			pgpHashUpdate(hc, buf, len);
			hashed[i-1] = obj;
				}
				/*
				* Okay, "obj" is the signature again, "hc" is the
				* appropriate hash context, and the signature has
				* been fetched.
				*/

		extrabuf = ringSigParseExtra(buf, len, &extralen);

		/*
		* extralen != 5 legal only if sig is on key directly,
		* or it is a new-format EXTENDED type.
		*/
		i = -1;		/* flag error on func call below */
		if (sp == 2									/* Sig directly on key */
			|| extralen == 5						/* Traditional case */
			 		/*
			* This was an earlier idea for forward compatibility, replaced
			* by the newer-version packets.
			* || (extralen > 5
			* && extrabuf[extralen-5]&PGP_SIGTYPEF_EXTENDED)
			 		*/
			|| obj->s.version > PGPVERSION_2_6		/* New format packets */
			|| sighashnew) {						/* DSA keys fix bug */
			/* Hash in extra bytes for signature */
			pgpHashCopy(hc, hcstack[sp-2]);
			pgpHashUpdate(hc, extrabuf, extralen);

			if (sighashnew) {
				/* Make sure hash can't match any old or doc hashes */
				byte postscript[6];
				postscript[0] = PGPVERSION_3;	/* actually a 4! */
				postscript[1] = 0xff;		 	/* different from sig type */
				postscript[2] = (byte)(extralen>>24);
				postscript[3] = (byte)(extralen>>16);
				postscript[4] = (byte)(extralen>> 8);
				postscript[5] = (byte)(extralen>> 0);
				pgpHashUpdate (hc, postscript, sizeof(postscript));
			}

			i = pgpSigCheckBuf(buf, len, pub, pgpHashFinal(hc));
			if (i == PGPERR_NOMEM)
				goto nomem;	/* Fatal error */
			/* @@@ TODO: use more error info */
			if (i == 1) {
				t |= PGP_SIGTRUSTF_CHECKED;
			 			/*
				* On finding a valid self signature, apply any info in sig
				* to parent key which goes there. "key" is signer, sigowner
				* is top level key above this sig, sigparent is closest
				* key above this sig (may differ for DSA/ElG keys).
			 			*/
				if (key == sigowner) {
					/* Self signature */
					byte const *pk;
					/* Look for expiration date, apply to key above us */
					pk = ringSigFindSubpacket (buf, SIGSUB_KEY_EXPIRATION,
						0, NULL, NULL, NULL, NULL, NULL);
					if (pk) {
						word32 keyexp;
						keyexp = (word32)((unsigned)pk[0]<<8|pk[1]) << 16 |
										((unsigned)pk[2]<<8|pk[3]);
						if (sigparent) {
							sigparent->k.validity = (word16)(keyexp/(24*3600));
						}
					}
				}
			}
			t |= PGP_SIGTRUSTF_TRIED;
		}
		pgpPubKeyDestroy(pub);
uncheckable:
		changemask |= setSigTrust(&obj->s, t);
		if (func)
			func(arg, iter, i);
	}
	ringIterDestroy(iter);
	iter = NULL;

	for (sp = 0; sp < (int)(sizeof(hcstack)/sizeof(*hcstack)); sp++)
		pgpHashDestroy(hcstack[sp]);	

#if 0
	/* Set the trustchanged bits under the mask */
	ringPoolMarkTrustChanged(ring, changemask);
#endif
	ringIterDestroy (iter);
	return 0;

nomem:
	i = PGPERR_NOMEM;
	ringSimpleErr(sigset->pool, i);
fatal:
	if (iter)
		ringIterDestroy(iter);
	if (pub)
		pgpPubKeyDestroy(pub);
	for (sp = 0; sp < (int)(sizeof(hcstack)/sizeof(*hcstack)); sp++)
		pgpHashDestroy(hcstack[sp]);	
	return i;
}

/*
* Update the hash context with the data in the object.
*
* hc		HashContext to update
* obj		 Object to update it with
* set		 Set which holds object, used for fetching its data
* hashnamelength	If true, include the length of a name packet in hash
*
* Return	0 on success, or an error code
*/
int ringHashObj (struct PgpHashContext *hc, union RingObject *obj,
		struct RingSet const *set, int hashnamelength)
{
	size_t objlen;
	byte const *objbuf;
	byte tmpbuf[5];

	if (!hc || !obj || !set)
		return PGPERR_BADPARAM;

	objbuf = (byte const *)ringFetchObject(set, obj, &objlen);
	if (!objbuf)
		return ringSetError(set)->error;
	/* We use this format even for subkeys */
	if (OBJISKEY(obj)) {
		pgpAssert(objlen <= 65535);
		tmpbuf[0] = PKTBYTE_BUILD(PKTBYTE_PUBKEY, 1);
		tmpbuf[1] = (byte)(objlen>>8);
		tmpbuf[2] = (byte)objlen;
		pgpHashUpdate(hc, tmpbuf, 3);
	} else if (OBJISNAME(obj) && hashnamelength) {
		/*
		* Use four bytes for name length for future expansion. Can't do
		* it for keys due to backwards compatibility.
		*/
		tmpbuf[0] = PKTBYTE_BUILD(PKTBYTE_NAME, 0);
		tmpbuf[1] = (byte)(objlen>>24);
		tmpbuf[2] = (byte)(objlen>>16);
		tmpbuf[3] = (byte)(objlen>> 8);
		tmpbuf[4] = (byte)(objlen>> 0);
		pgpHashUpdate(hc, tmpbuf, 5);
	}
	pgpHashUpdate(hc, objbuf, objlen);
	return 0;
}

/*
* Sign the specified object obj, along with its parents.
* Should be member of RingSet dest, which may be mutable or immutable.
* Place signature into sig buffer.
* sig buffer should be at least pgpMakeSigMaxSize(spec) bytes long.
* Returns size of sig in bytes on success, negative on error.
*/
int
ringSignObj(byte *sig, struct RingSet *dest, union RingObject *obj,
	struct PgpSigSpec *spec, struct PgpRandomContext const *rc)
{
	struct PgpHashContext *hc;
	union RingObject *parents[RINGMAXDEPTH];
	int len;
	int level;
	int retval;
	int sighashnew;

	/* Initialize hash */
	hc = pgpHashCreate(pgpSigSpecHash(spec));
	if (hc==NULL)
		return PGPERR_BAD_HASHNUM;

	/* Use new hashing convention (include name length) on new formats */
	sighashnew = pgpSigSpecVersion(spec) > PGPVERSION_2_6;

	/* Trace object's parents up to top level */
	level = 0;
	for ( ; ; ) {
		pgpAssert (level < RINGMAXDEPTH);
		parents[level++] = obj;
		if (OBJISTOP(obj))
			break;
		obj = obj->g.up;
	}

	/* Hash downwards from top to object */
	while (--level >= 0) {
		retval = ringHashObj (hc, parents[level], dest, sighashnew);
		if (retval) {
			pgpHashDestroy (hc);
			return retval;
		}
	}
	len = pgpMakeSig (sig, spec, rc, hc);
	pgpHashDestroy(hc);
	return len;
}

/*********** DYNAMIC KEY EVALUATION FOR NON-TRUSTED KEY SOURCES *********/

/* Dynamic evaluation functions. This is a bottom-up algorithm rather
than the top-down algorithm used by the static evaluation functions.
To evaluate a name, its signatures must be evaluated, which in turn
require evaluation of the signing keys names. Thus, the algorithm
is recursive. The recursion terminates when one of the following
conditions are detected:

- The key to which the name is attached is axiomatic
- The key to which the name is attached is revoked
- The signature is bad
- The signature is retired
- The signing key is axiomatic
- The signing key is revoked
- The signing key has expired
- A certification loop is detected
- Cert depth is exceeded

Certification loops are detected by maintaining a stack that represents
the certification path currently being evaluated. If the key we're
looking at is already on the stack, then it's a loop. Loops are handled
in a better manner than the static top-down algorithm (which sometimes
discards perfectly good signatures), resulting in more accurate results.

Objects from untrusted keyrings (keyfiles) do not have the RINGOBJF_TRUST
bit set. When we compute the validity of such an object, the
RINGOBJF_TRUSTCHANGED bit is set as a record that this has been done.

@@@ TODO: Optimize

The comment below is the discovery by Colin of the problem with
PGPTRUSTMODEL 1 that trust along shared paths is not properly discounted:

@@@ XXX
@@@ XXX This code is buggy! It does not compute the right answer!
@@@ XXX Let "X->Y" mean "X signs Y". Consider A->B->C->E, A->B->D->E.
@@@ XXX If A is perfectly valid and all signers are 90% valid,
@@@ XXX then

*/

static word16
ringMntIntValidateName (struct RingSet *set, union RingObject *name,
			int depth);

/* @@@ NOTE: This is not thread-safe */

static union RingObject *keystack[10];	/* max certdepth == 10 */
static int keystack_ptr = 0;
#if PGPTRUSTMODEL==0
static word16 trust[PGP_KEYTRUST_MASK + 1];
static word16 thresh;
static RingPool *trustpool = NULL;

static void
ringMntInitTrust (struct RingSet const *set)
{
unsigned t;
	
	/* Set up trust table once only, unless pool changes */

	if (trustpool == set->pool)
	return;
	trustpool = set->pool;
		/*
	* This is a simple scoring system. Multiply the number of completes
	* required by the number of marginals required. This gives the
	* 'ultimate' score. Divide the total by the number of completes to
	* give the 'score' for a complete, and by the number of marginals to
	* give the 'score' for a marginal.
		*/
	for (t = 0; t < sizeof(trust)/sizeof(*trust); t++)
		trust[t] = 0;
	trust[PGP_KEYTRUST_MARGINAL] = 1;
	trust[PGP_KEYTRUST_COMPLETE] = 1;
	trust[PGP_KEYTRUST_ULTIMATE] = 1;
	if (set->pool->num_marginals) {
		trust[PGP_KEYTRUST_COMPLETE] = set->pool->num_marginals;
		trust[PGP_KEYTRUST_ULTIMATE] = set->pool->num_marginals;
	} else {
		trust[PGP_KEYTRUST_MARGINAL] = 0;
	}
	if (set->pool->num_completes) {
		trust[PGP_KEYTRUST_MARGINAL] = set->pool->num_completes;
		trust[PGP_KEYTRUST_ULTIMATE] *= set->pool->num_completes;
	} else {
		trust[PGP_KEYTRUST_COMPLETE] = 0;
	}
	thresh = trust[PGP_KEYTRUST_ULTIMATE];
}
#endif

/* Check various attributes of key: revocation status, axiomatic, expired.
Ensure all sigs attached to the key have been checked */

static void
ringMntIntCheckKey (struct RingSet const *set, union RingObject *key,
		int *revoked, int *axiomatic, int *expired)
{
	union RingObject *obj, *obj2;
	ringmask mask = set->mask;
	struct RingSet *tmpset;

	pgpAssert (set);
	pgpAssert (key && OBJISKEY (key));
	pgpAssert (revoked && axiomatic && expired);
	*revoked = 0; *axiomatic = 0; *expired = 0;
	
	if (key->k.tstamp > 0 && key->k.validity > 0 &&
	(PGPTime) (key->k.tstamp +
	(key->k.validity * 24 * 3600)) < pgpGetTime())
		*expired = 1;

	if (key->k.trust & PGP_KEYTRUSTF_BUCKSTOP)
		/* Must be verified by passphrase check, not by simply looking
		for a secret key. So, key must be on a local ring and
		must have the buckstop bit set. */
		*axiomatic = 1;	

	/* Ensure all sigs have been checked for this key. Copy the
	key to a temporary set and pass it to ringPoolCheck */

	tmpset = ringSetCreate (set->pool);
	pgpAssert (tmpset);
	for (obj = key->k.down; obj; obj = obj->g.next) {
		if (OBJISBOT (obj))
			ringSetAddObject (tmpset, obj);
		else {
			for (obj2 = obj->g.down; obj2; obj2 = obj2->g.next)
				ringSetAddObject (tmpset, obj2);
		}
	}
	ringSetFreeze (tmpset);
	#if 0
		{
				int count = 0;
				fprintf (stdout, "Checking key:\n");
				ringKeyPrint (stdout, set, key, 0);
				if (*axiomatic)
				fprintf (stdout, "AXIOMATIC KEY\n");
				if ((count = ringPoolCheckCount (tmpset, set, 0)) > 0)
				fprintf (stdout, "Checking %d signatures...\n",
			 		count);
	}
#endif
	ringPoolCheck (tmpset, set, 0, NULL, NULL);
	ringSetDestroy (tmpset);
	ringGarbageCollect (set->pool);

	for (obj = key->k.down; obj; obj = obj->g.next) {
		if (mask & obj->g.mask && OBJISSIG (obj)) {
			if (obj->s.type == PGP_SIGTYPE_KEY_COMPROMISE &&
			obj->s.by == key &&
			obj->s.trust & PGP_SIGTRUSTF_CHECKED)
				*revoked = 1;
		}
	}
}

/* Check that the sig is good. Ensure that it has been checked, that
it has not expired, that it a key signature, that it has not been
superseded, and that is has not been retired. Expiration or revocation
	of the signing key is checked elsewhere.
*/

static int
ringMntGoodSig (struct RingSet const *set, union RingObject *sig,
				word32 const timenow)
{
	union RingObject *sig2;
	(void) set;

	pgpAssert (sig && OBJISSIG (sig));
	if (!(sig->s.trust & PGP_SIGTRUSTF_CHECKED) ||
	!mntSigIsValid (timenow, sig->s.tstamp, sig->s.validity) ||
	sig->s.type != PGP_SIGTYPE_KEY_GENERIC)
		return 0;

	for (sig2 = sig->s.up->g.down; sig2; sig2 = sig2->s.next) {
	if (sig2->s.by == sig->s.by) {
				/* If we find a revocation sig that's newer, assume
			 		the old sig is bad even if the revocation sig has
			 		not been checked (it's the safest option in case
			 		a good revocation sig has been corrupted) */
				if (sig2->s.type == PGP_SIGTYPE_KEY_UID_REVOKE &&
				sig2->s.tstamp > sig->s.tstamp)
			 		return 0;
			/* Otherwise, see if we have a newer sig that's been
			checked and is valid. */
			if (sig2->s.trust & PGP_SIGTRUSTF_CHECKED &&
			 			sig2->s.tstamp > sig->s.tstamp &&
			 			mntSigIsValid (timenow, sig2->s.tstamp, sig2->s.validity))
			 		return 0;
		}
	}
	return 1; /* good */
}

/*
* NO_RECURSION==1 causes sig validation to use the trust and confidence
* values on the signing key and its names. It is most appropriate for a
* model where trusted introducers are kept on a local keyring and the
* regular maintenance pass is used to keep their trust information up to
* date.
*
* NO_RECURSION==0 (e.g. recursion) causes sig validation to recursively
* dynamically check the validity of the signing key. That is most
* appropriate for the case where none of the public keys have trust
* information and the dynamic validation is run each time a key is used.
*/

#define NO_RECURSION 1


/* Validate a signature. May be necessary to recursively validate the
names attached to the signing key. */

static word16
ringMntIntValidateSig (struct RingSet *set, union RingObject *sig, int depth)
{
	union RingObject *key = sig->s.by;
	int i;
	int revoked = 0, axiomatic = 0, expired = 0;
	union RingObject *name = NULL;
#if !NO_RECURSION
	word16 validity = 0;
	word16 best_trust = 0;
#if PGPTRUSTMODEL>0
	word16 confidence = 0, trust = 0;
	word16 best_validity = 0;
#endif
#endif

	pgpAssert (set);
	pgpAssert (OBJISKEY (key));
	if (depth > set->pool->certdepth)
		return 0;		 /* exceeded cert depth */
#if 0
	fprintf (stdout, "Validating sig, depth = %d\n", depth);
#endif

	/* Check the stack for a certification loop */
	for (i = 0; i < keystack_ptr; i++)
		if (key == keystack[i])
			return 0;	 /* certification loop detected */
	
	if (key->g.mask == 0)
		return 0;		 /* dummy key */

	/*
	* See if key has any potential to be a trusted introducer, return 0
	* if not
	*/
#if PGPTRUSTMODEL==0
	if ((key->k.trust & PGP_KEYTRUST_MASK) <= PGP_KEYTRUST_NEVER)
	return 0;
#else
	for (name = key->g.down; name; name = name->n.next) {
		if (OBJISNAME(name) &&
			name->n.confidence != PGP_NEWTRUST_UNDEFINED &&
					name->n.confidence != 0)
			break;				/* exit loop with name<>NULL if trusted intr */
	}
	if (!name)
		return 0;
#endif

	/*
	* This call may be overkill as it checks all sigs on key, while if we are
	* not recursing we are only interested in revocation sigs.
	*/
	ringMntIntCheckKey (set, key, &revoked, &axiomatic, &expired);
	if (revoked || expired)
		return 0;
	if (axiomatic)
		return 0;		 		 /* XXX Shouldn't axiomatic sigs be valid? */

#if NO_RECURSION
	/* If a full dynamic validation is not required, simply
	get the confidence value from the pre-computed name
	validities. For old trust model, we must check for at least one
	valid name, and then return the trust in the key as a
	"score" */
#if PGPTRUSTMODEL==0
	for (name = key->g.down; name; name = name->n.next) {
			if (OBJISNAME (name) &&
				(name->n.trust & PGP_NAMETRUST_MASK) >
				PGP_NAMETRUST_UNTRUSTED)
				return trust[key->k.trust & PGP_KEYTRUST_MASK];
	}
	return 0;
#else
	return calctrust (&key->k, set->mask);
#endif
#else /* !NO_RECURSION */

	/* Recurse down (or up, whichever you prefer) and dynamically
	validate the signing key. */

	for (name = key->k.down; name; name = name->g.next) {
		if (!OBJISNAME (name))
			continue;
#if PGPTRUSTMODEL==0
		/* If the total "score" for a name exceeds thresh,
		then we consider the key to be valid and can
		return its trust as a "score" */

		validity = ringMntIntValidateName (set, name, depth);
		if (validity >= thresh) /* scored a winner! */
		return trust[key->k.trust & PGP_KEYTRUST_MASK];
#else /* PGPTRUSTMODEL>0 */
		/* Get the validity of the name, and combine with the
		confidence to get overall trust in the name.
		Keep track of the 'best' as this will be returned as
		the trust in the signing key. */

		if (name->n.confidence == PGP_NEWTRUST_INFINITE)
			confidence = PGP_TRUST_INFINITE;
		else if (name->n.confidence == PGP_NEWTRUST_UNDEFINED)
			confidence = 0;
		else
			confidence = name->n.confidence << TRUST_CERTSHIFT;

		/* Don't bother validating name if its confidence is 0,
		as it won't contribute to the confidence in the sig */

		if (confidence > 0) {
			validity = ringMntIntValidateName (set, name, depth);
			trust = mergetrust (confidence, validity);
			if (validity > best_validity ||
			(validity == best_validity &&
			trust > best_trust)) {
				best_validity = validity;
				best_trust = trust;
			}
		}
#endif
	}
	return best_trust;
#endif
}

/* Internal name validation function. Check all the signatures on the
name. We can stop early if we hit infinite validity. */

static word16
ringMntIntValidateName (struct RingSet *set, union RingObject *name, int depth)
	{
		RingObject	*obj,
			*key;
		word16	validity = 0,
		v = 0;
		ringmask	mask = set->mask;
		word32	timenow = pgpGetTime ();
#if PGPTRUSTMODEL==2
		PathList	*pathlist = NULL;
		RingSet			*confidentset;
		unsigned	maxdepth = set->pool->certdepth;
		double	confidence;
	#endif

	pgpAssert (set);
	pgpAssert (OBJISNAME (name));
	key = name->n.up;
#if 0
	fprintf (stdout, "Validating name, depth = %d\n", depth);
#endif

#if PGPTRUSTMODEL==2

	confidentset = ringSetConfidentSet (set);
	ringFindPathsBack (name, &pathlist, maxdepth, set, pgpGetTime(),
		set->pool);
	if (pathlist) {
		pathlist = pathListPrune (pathlist, TOTALPATHMAX, set->pool);
		confidence = pathListConfidence (pathlist, set->pool);
		if (confidence == 1.)
			validity = PGP_TRUST_INFINITE;
		else
			validity = ringDoubleToTrust (1. / (1.-confidence));
		pathListFreeAll (pathlist, set->pool);
		pathlist = NULL;
	}

	memPoolEmpty(&set->pool->pathpool);
	set->pool->paths = NULL;
	set->pool->pathlists = NULL;
	ringSetDestroy (confidentset);

#else /* PGPTRUSTMODEL!=2*/

	keystack[keystack_ptr++] = key;		/* push key on the stack */

	/* Combine the trust in each signature to get the validity of
	the name. Must be careful to check for overflow, and need
	special handling for infinite trust. */

	validity = 0;
	for (obj = name->n.down; obj; obj = obj->g.next) {
		pgpAssert (OBJISSIG (obj));
		if (mask & obj->g.mask && obj->s.by != key) {
			if (ringMntGoodSig (set, obj, timenow)) {
				v = ringMntIntValidateSig (set, obj,
				depth + 1);
#if PGPTRUSTMODEL==0
				validity += v; /* add in "score" for sig */
				if (validity >= thresh)
				break; /* name is valid */
#else		 	
				if (v == PGP_TRUST_INFINITE) {
					validity = v;
					break; /* can't do better than this! */
				}
				else if (validity < PGP_TRUST_MAX) {
					validity += v;
					if (validity + 1 <= v)
						validity = PGP_TRUST_MAX;
				}
#endif				
			}
		}
	}
	keystack_ptr--;		/* pop key off the stack */
#endif /*PGPTRUSTMODEL!=2*/
	return validity;
}

/* Validate a name from an untrusted keyfile. Externally visible. */

void
ringMntValidateName (struct RingSet const *set, union RingObject *name)
{
	word16 validity = 0;
	struct RingSet *allkeys;
	int axiomatic = 0, revoked = 0, expired = 0;

	pgpAssert (set);
	pgpAssert (name && OBJISNAME (name));
	if (name->g.flags & (RINGOBJF_TRUST | RINGOBJF_TRUSTCHANGED))
		return;	 /* already done */

	if (!(name->g.flags & RINGOBJF_TRUSTCHANGED)) {

		/* Here's a real cheat. We copy the set and then
		copy the allocmask from the parent pool for an instant
		union of all keys in the pool. */
	
		allkeys = ringSetCopy (set);
		pgpAssert (allkeys);
		ringSetFreeze (allkeys);
		allkeys->mask = ringAllocMask (set->pool, NULL);

		ringMntIntCheckKey (allkeys, name->n.up, &revoked, &axiomatic,
				&expired);
#if PGPTRUSTMODEL==0
		ringMntInitTrust (allkeys);
		if (revoked)
			validity = PGP_NAMETRUST_UNTRUSTED;
		else if (axiomatic)
			validity = PGP_NAMETRUST_COMPLETE;
		else {
			validity = ringMntIntValidateName (allkeys, name, 0);
			if (validity >= thresh)
				validity = PGP_NAMETRUST_COMPLETE;
		else if (validity * 2 >= thresh)
				validity = PGP_NAMETRUST_MARGINAL;
			else
				validity = PGP_NAMETRUST_UNTRUSTED;
		}
		name->n.trust = (name->n.trust & ~PGP_NAMETRUST_MASK) |
		validity;
#else
		name->n.valid = 0;
		if (revoked)
			name->n.valid = 0;
		else if (axiomatic)
			name->n.valid = PGP_TRUST_INFINITE;
		else
		name->n.valid = ringMntIntValidateName
			(allkeys, name, 0);
		if (name->n.valid == PGP_TRUST_INFINITE) {
			name->n.validity = PGP_NEWTRUST_INFINITE;
		} else {
			validity = name->n.valid >> TRUST_CERTSHIFT;
			name->n.validity = (validity > PGP_NEWTRUST_MAX) ?
			PGP_NEWTRUST_MAX : (byte) validity;
		}
#endif
		name->g.flags |= RINGOBJF_TRUSTCHANGED;
		ringSetDestroy (allkeys);
	}
}

/* Validate a key from an untrusted keyfile. Externally visible. */

void
ringMntValidateKey (struct RingSet const *set, union RingObject *key)
{
	union RingObject *name;
	int revoked = 0, axiomatic = 0, expired = 0;
	
	pgpAssert (set);
	pgpAssert (key && OBJISKEY (key));

	if (!(key->g.flags & (RINGOBJF_TRUST | RINGOBJF_TRUSTCHANGED))) {
		ringMntIntCheckKey (set, key, &revoked, &axiomatic, &expired);
		if (revoked)
			key->k.trust |= PGP_KEYTRUSTF_REVOKED;
		key->g.flags |= RINGOBJF_TRUSTCHANGED;
	}

	for (name = key->k.down; name; name = name->n.next) {
		if (ringObjectType (name) == RINGTYPE_NAME)
			ringMntValidateName (set, name);
	}
}

/*
* Local Variables:
* tab-width: 4
* End:
* vi: ts=4 sw=4
* vim: si
*/
