/*
 * subst.c -- Repair substitution tables
 *
 * Copyright (C) 1997 Pretty Good Privacy, Inc.
 *
 * Written by Colin Plumb
 *
 * $Id: subst.c,v 1.14 1997/11/03 22:12:00 colin Exp $
 *
 * IT IS EXPECTED that users of this program will play with these tables
 * and the cost values in the subst.h header.  (Some day, they'll all
 * get moved to an external config file.)
 *
 * NOTE: Other cost are hiding in the Filter functions in repair.c.
 * Remember to keep them all on the same scale.
 */

/*
 * The repair program copies its input to its output, making various
 * substitutions, until it manages to produce a version that satisfies
 * the parser.  This includes having a correct CRC for each line.
 * Each substitution has a cost, and the combinations are tried in order
 * of increasing cost.  NOTE that even translating "A"->"A" counts as
 * a substitution, although it may have zero cost.
 *
 * The intention is to correct transcription errors, where the
 * errors have a distinctly non-uniform distribution.  Slight
 * differences in cost produce a preference in trying some errors
 * first.  If an error costs half as much as another, combinations
 * of two of that error will be compared to one of the more expensive.
 * Too many cheap substitutions will result is repair spending
 * a very log time searching before considering the more expensive
 * substitutions.
 *
 * The following parameters and the raw substitution tables are expected
 * to be edited by the user based on experience.  Eventually, this
 * will be moved into an external config file, but for now it's a matter
 * of recompiling.
 */

#include "subst.h"
#include "util.h"

/* what the OCR software reports for "unrecognizable */
#define UNRECOG_STRING "~\274"

/*
 * The input substitutions to make (one-to-one).   These are listed in
 * the order of correction. i.e. uncorrected input first, then corrected
 * output.  Substitutions are one-way; to get two-way, list it twice.
 */

struct RawSubst const substSingles[] = {
	/* Identity substitutions - note that period (.) is excluded */
	{ "!\"#$%&'()*+,-./0123456789:;<=>?" SPACE_STRING,
	  "!\"#$%&'()*+,-./0123456789:;<=>?" SPACE_STRING, 0, 0, NULL },
	{ "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\t" TAB_STRING,
	  "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_\t" TAB_STRING, 0, 0, NULL },
	{ "`abcdefghijklmnopqrstuvwxyz{|}~\f" FORMFEED_STRING,
	  "`abcdefghijklmnopqrstuvwxyz{|}~\f" FORMFEED_STRING, 0, 0, NULL },
#if (TAB_PAD_CHAR & 128)	/* Not already included? */
	{ TAB_PAD_STRING, TAB_PAD_STRING, 0, NULL },
#endif
	{ "\r\n" CONTIN_STRING, "\n\n" CONTIN_STRING, 0, 0, NULL },

	/* Occasionally these just get inserted as glitches */
	{ ".,'`", NULL, 5, 10, FilterNearBlanks },
	/* This is now pretty infrequent */
	{ "-_", "_-", 0, 10, FilterAfterRepeat },

	/*
	 * Capitalization errors are common in some cases
	 * c/C, s/S, u/U are fucked up all the time.
	 * Also o/O, v/V and w/W.  x, y and z also give some problems.
	 */
	{ "cilmopsuvwxyz", "CILMOPSUVWXYZ", 7, 13, FilterNearLower },
	{ "CILMOPSUVWXYZ", "cilmopsuvwxyz", 7, 13, FilterNearUpper },
	/* Other errors */
	{ "g9aaiji;xX00Si", "9gg2ji;i%%oO3f", 10, 0, NULL },
	/* This seems to happen a lot */
	{ "c", "r", 9, 0, NULL },

	{ "j", ";", 9, 0, NULL },
	{ "' ", "``", 10, 0, NULL },

	/* Uncommon errors */

	/* Wierd stuff that's happened in the checksum part */
	/* A highish weight is okay here */
	{ "sSEdJl", "554437", 15, 0, NULL },
	{ "LESsPZ", "bb8a22", 15, 0, NULL },

	/* Wierd stuff that has happened */
	{ "BasAeaeRoooo", "3334a@QQpqbd", 5, 15, FilterIsBinary },
	{ "oooo", "pqbd", 0, 15, FilterIsBinary },
	{ "ttTCCflO", "iff{[lfG", 12, 0, NULL },
#if 0
	/* If the line-breaks get screwed up, use these */
	{ " ", "\n", 10, COST_INFINITY, FilterChecksumFollows },
	{ "\n", " ", COST_INFINITY, 10, FilterChecksumFollows },
	{ "\n", NULL, COST_INFINITY , 11, FilterChecksumFollows },
#endif

{ NULL, NULL, 0, 0, NULL }
};

/* The many-to-many substitutions */
struct RawSubst const substMultiples[] = {
	{ "''", "\"", 2, 0, NULL },
	{ "``", "\"", 2, 0, NULL },
	{ ",'", "\"", 2, 0, NULL },
	{ "',", "\"", 2, 0, NULL },
	{ ",,", "\"", 2, 0, NULL },
	/* Extra inserted spaces are common */
	{ " ", " ", COST_INFINITY,  0, FilterFollowsSpace },
	{ " ", "", 0, 15, FilterFollowsSpace },
	{ "\t", " ", COST_INFINITY,  0, FilterFollowsSpace },
	{ "\t", "", 0, 10, FilterFollowsSpace },
	/* Convert between SPACE_CHAR dots and periods */
	{ ".", SPACE_STRING, 1, COST_INFINITY, FilterFollowsSpace },
	{ ".", " "SPACE_STRING, COST_INFINITY, 10, FilterFollowsSpace },
	{ SPACE_STRING, ".", 15, 5, FilterFollowsSpace },
	{ SPACE_STRING, " "SPACE_STRING, COST_INFINITY, 5, FilterFollowsSpace },

	/* Replace "unknown" by zero - it often is */
	{ UNRECOG_STRING, "0", 1, 0, NULL },
	{ UNRECOG_STRING, "_", 2, 0, NULL },
	{ UNRECOG_STRING, ")", 3, 0, NULL },
	{ UNRECOG_STRING, "^", 4, 0, NULL },
	/* Except that these glitches are common */
	{ UNRECOG_STRING"'", "\\\"", 0, 0, NULL },
	{ UNRECOG_STRING"'", "\"", 1, 0, NULL },
	{ "'"UNRECOG_STRING, "\"", 0, 0, NULL },
	{ UNRECOG_STRING UNRECOG_STRING , "\"", 0, 0, NULL },
	/* Something else that has been seen */
	{ "V'", "\\\"", 5, 0, NULL },

	/* A common transposition */
	{ "\"'", "'\"", 5, 0, NULL },
	{ "'\"", "\"'", 5, 0, NULL },
	/* These also happen fairly often */
	{ " \"", "''", 5, 0, NULL },
	{ "\" ", "''", 5, 0, NULL },

	/* Common glitches */
	{ "\t.\n", "\n", 5, 0, NULL },
	{ "\t,\n", "\n", 5, 0, NULL },
	{ "\t-\n", "\n", 5, 0, NULL },
	{ "\t_\n", "\n", 5, 0, NULL },
	{ "\t'\n", "\n", 5, 0, NULL },
	{ "\t`\n", "\n", 5, 0, NULL },
	{ "\t~\n", "\n", 5, 0, NULL },
	{ "\t:\n", "\n", 5, 0, NULL },
	{ "\t"SPACE_STRING"\n", "\n", 5, 0, NULL },

	/* Less common */
	{ " .\n", "\n", 10, 0, NULL },
	{ " ,\n", "\n", 10, 0, NULL },
	{ " -\n", "\n", 10, 0, NULL },
	{ " _\n", "\n", 10, 0, NULL },
	{ " '\n", "\n", 10, 0, NULL },
	{ " `\n", "\n", 10, 0, NULL },
	{ " ~\n", "\n", 10, 0, NULL },
	{ " :\n", "\n", 10, 0, NULL },
	{ " "SPACE_STRING"\n", "\n", 10, 0, NULL },

	/* Even less common */
	{ ".\n", "\n", 15, 0, NULL },
	{ ",\n", "\n", 15, 0, NULL },
	{ "-\n", "\n", 15, 0, NULL },
	{ "_\n", "\n", 15, 0, NULL },
	{ "'\n", "\n", 15, 0, NULL },
	{ "`\n", "\n", 15, 0, NULL },
	{ "~\n", "\n", 15, 0, NULL },
	{ ":\n", "\n", 15, 0, NULL },
	{ SPACE_STRING"\n", "\n", 15, 0, NULL },

	/* Wierd stuff that has happened */
	{ "lJ", "U", 10, 0, NULL },
	{ "ll", "U", 10, 0, NULL },
	{ "l1", "U", 10, 0, NULL },
	{ "il", "U", 10, 0, NULL },	/* Fairly common, actually */
	{ "li", "U", 10, 0, NULL },
	{ "l)", "U", 10, 0, NULL },
	{ "Ll", "U", 10, 0, NULL },
	{ "LI", "U", 10, 0, NULL },
	{ "L1", "U", 10, 0, NULL },

	{ "lo", "b", 10, 0, NULL },
	{ "cl", "d", 10, 0, NULL },
	{ "cliff", "diff", 2, 0, NULL },
	{ "*\n", "*/\n", 10, 0, NULL },

	/* That big black block has odd things happen to it */
	{ "d", CONTIN_STRING, 10, 0, NULL },
	{ "d\n", CONTIN_STRING"\n", 3, 0, NULL },
	{ "S", CONTIN_STRING, 10, 0, NULL },
	{ "S\n", CONTIN_STRING"\n", 3, 0, NULL },

	/* Tab-stop wonders */
	{ TAB_STRING, TAB_STRING"", 0, 0, TabFilter },
	{ TAB_STRING, TAB_STRING" ", 0, 0, TabFilter },
	{ TAB_STRING, TAB_STRING"  ", 0, 0, TabFilter },
	{ TAB_STRING, TAB_STRING"   ", 0, 0, TabFilter },
	{ TAB_STRING, TAB_STRING"    ", 0, 0, TabFilter },
	{ TAB_STRING, TAB_STRING"     ", 0, 0, TabFilter },
	{ TAB_STRING, TAB_STRING"      ", 0, 0, TabFilter },
	{ TAB_STRING, TAB_STRING"       ", 0, 0, TabFilter },
	/* Some scan errors */
	{ "D ", TAB_STRING"", 1, 5, TabFilter },
	{ "D ", TAB_STRING" ", 1, 5, TabFilter },
	{ "D ", TAB_STRING"  ", 1, 5, TabFilter },
	{ "D ", TAB_STRING"   ", 1, 5, TabFilter },
	{ "D ", TAB_STRING"    ", 1, 5, TabFilter },
	{ "D ", TAB_STRING"     ", 1, 5, TabFilter },
	{ "D ", TAB_STRING"      ", 1, 5, TabFilter },
	{ "D ", TAB_STRING"       ", 1, 5, TabFilter },
#if TAB_PAD_CHAR != ' '
#error Fix those tab patterns!
#endif
{ NULL, NULL, 0, 0, NULL }
};
