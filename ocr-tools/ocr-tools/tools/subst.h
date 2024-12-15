/*
 * subst.h -- Header for repair substitutions
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Written by Colin Plumb
 *
 * $Id: subst.h,v 1.9 1997/11/03 22:12:00 colin Exp $
 */

/*
 * Give up if the list of pending changes to attempt grows to this many
 * elements.  Each element is 32 bytes, so 128K is 8 MB of memory.
 * (Other than this, repair's memory usage is fairly modest.)
 */
#define MAX_HEAP (1<<17)

/*
 * There is a hack in the code to find a single substitution that will fix a
 * line, even if it's not in the tables.  It gets added to the tables "on
 * probation", with an infinite cost, and if it leads to a successful
 * correction of the entire page, is "learned" for future use and its
 * cost reduced to something finite.
 * (This is not remembered across runs of the program, though.
 * Edit the tables in the source to fix it.)
 */
#define DYNAMIC_COST_LEARNED 15

/*
 * This negative-cost bonus for passing the end of a line with the right
 * CRC makes the search engine reluctant to backtrack past a correct CRC,
 * greatly improving efficiency.  It's rather a hack, though.  Think of
 * this in terms of "how many errors should be considered in the current
 * line before considering the possibility of errors in the previous line?"
 *
 * This bonus is halved for lines that are the result of a correction
 * that was computed from the checksum, since a correct checksum is
 * much less significant in such a case.
 */
#define COST_LINE -30

/* The cost of a full-line nastyline substitution. */
#define NASTY_COST 5

/* Type describing filter functions used in substitutions */
struct ParseNode;
struct Substitution;
#include "heap.h"
typedef HeapCost FilterFunc(struct ParseNode *parent, char const *limit,
	struct Substitution const *subst);
FilterFunc TabFilter,              FilterFollowsSpace, FilterNearBlanks;
FilterFunc FilterNearUpper,        FilterNearLower,    FilterNearXDigit;
FilterFunc FilterAfterRepeat,      FilterCharConst,    FilterChecksumFollows;
FilterFunc FilterLikelyUnderscore, FilterIsDynamic,    FilterIsBinary;

/* The external substitution format */
typedef struct RawSubst {
	char const *input;
	char const *output;
	HeapCost cost, cost2;
	FilterFunc *filter;
} RawSubst;

/* The substitutions to make */
extern struct RawSubst const substSingles[];
extern struct RawSubst const substMultiples[];
