/*
* pgpZDeflate.c
*
* Copyright (C) 1990-1993 Mark Adler, Richard B. Wales, Jean-loup Gailly,
* Kai Uwe Rommel and Igor Mandrichenko.
* Permission is granted to any individual or institution to use, copy,
* or redistribute this software so long as all of the original files
* are included, that it is not sold for profit, and that this copyright
* notice is retained.
*
* Hacked up for PGP by Colin Plumb
*
* $Id: pgpZDeflate.c,v 1.1.2.1 1997/06/07 09:51:10 mhw Exp $
*/

/*
* deflate.c by Jean-loup Gailly.
*
* PURPOSE
*
* Identify new text as repetitions of old text within a fixed-
* length sliding window trailing behind the new text.
*
* DISCUSSION
*
* The "deflation" process depends on being able to identify portions
* of the input text which are identical to earlier input (within a
* sliding window trailing behind the input currently being processed).
*
* The most straightforward technique turns out to be the fastest for
* most input files: try all possible matches and select the longest.
* The key feature of this algorithm is that insertions into the string
* dictionary are very simple and thus fast, and deletions are avoided
* completely. Insertions are performed at each input character,
* whereas string matches are performed only when the previous match
* ends. So it is preferable to spend more time in matches to allow very
* fast string insertions and avoid deletions. The matching algorithm
* for small strings is inspired from that of Rabin & Karp. A brute
* force approach is used to find longer strings when a small match has
* been found. A similar algorithm is used in comic (by Jan-Mark Wams)
* and freeze (by Leonid Broukhis).
*
* A previous version of this file used a more sophisticated algorithm
* (by Fiala and Greene) which is guaranteed to run in linear amortized
* time, but has a larger average cost, uses more memory and is patented.
* However the F&G algorithm may be faster for some highly redundant
* files if the parameter max_chain_length (described below) is too large.
*
* ACKNOWLEDGEMENTS
*
* The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
* I found it in 'freeze' written by Leonid Broukhis.
* Thanks to many info-zippers for bug reports and testing.
*
* REFERENCES
*
* APPNOTE.TXT documentation file in PKZIP 1.93a distribution.
*
* A description of the Rabin and Karp algorithm is given in the book
* "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
*
* Fiala,E.R., and Greene,D.H.
* Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
*
* INTERFACE
*
* int lm_init (int pack_level)
* Initialize the "longest match" routines for a new file
*
* word32 deflate (void)
* Processes a new input file and return its compressed length. Sets
* the compressed length, crc, deflate flags and internal file
* attributes.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pgpZip.h"

/* ===========================================================================
* Configuration parameters
*/

/*
* Compile with MEDIUM_MEM to reduce the memory requirements or
* with SMALL_MEM to use as little memory as possible. Use BIG_MEM if the
* entire input file can be held in memory (not possible on 16 bit systems).
* Warning: defining these symbols affects HASH_BITS (see below) and thus
* affects the compression ratio. The compressed output
* is still correct, and might even be smaller in some cases.
*/

#ifdef SMALL_MEM
# define HASH_BITS 13 /* Number of bits used to hash strings */
#endif
#ifdef MEDIUM_MEM
# define HASH_BITS 14
#endif
#ifndef HASH_BITS
# define HASH_BITS 15
/* For portability to 16 bit machines, do not use values above 15. */
#endif

#define HASH_SIZE (unsigned)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK (WSIZE-1)
/* HASH_SIZE and WSIZE must be powers of two */

#define NIL 0
/* Tail of hash chains */

#ifndef TOO_FAR
# define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

#ifdef ATARI_ST
# undef MSDOS /* avoid the processor specific parts */
/* (but the Atari should never define MSDOS anyway ...) */
#endif
#if defined(MSDOS) && !defined(NO_ASM) && !defined(ASMV)
# define ASMV
#endif
#if defined(ASMV) && !defined(MSDOS) && defined(DYN_ALLOC)
error: DYN_ALLOC not yet supported in match.s
#endif

/* ===========================================================================
* Local data used by the "longest match" routines.
*/

#if defined(BIG_MEM) || defined(MMAP)
typedef size_t Pos; /* must be at least 32 bits */
#else
typedef word16 Pos;
#endif
typedef unsigned IPos;
/* A Pos is an index in the character window. We use short instead of int to
* save space in the various tables. IPos is used only for parameter passing.
*/

#ifndef DYN_ALLOC
byte window[2L*WSIZE];
/* Sliding window. Input bytes are read into the second half of the window,
* and move to the first half later to keep a dictionary of at least WSIZE
* bytes. With this organization, matches are limited to a distance of
* WSIZE-MAX_MATCH bytes, but this ensures that IO is always
* performed with a length multiple of the block size. Also, it limits
* the window size to 64K, which is quite useful on MSDOS.
* To do: limit the window size to WSIZE+BSZ if SMALL_MEM (the code would
* be less efficient since the data would have to be copied WSIZE/BSZ times)
*/
Pos prev[WSIZE];
/* Link to older string with same hash index. To limit the size of this
* array to 64K, this link is maintained only for the last 32K strings.
* An index in this array is thus a window index modulo 32K.
*/
Pos head[HASH_SIZE];
/* Heads of the hash chains or NIL. If your compiler thinks that
* HASH_SIZE is a dynamic value, recompile with -DDYN_ALLOC.
*/
#else
/*
* MS-DOS adjusts the pointers for zero offset, so we store the
* original pointers in these variables for later free()ing
*/
static byte * near o_window = NULL;
static Pos * near o_prev = NULL;
static Pos * near o_head;
byte * near window = NULL;
Pos * near prev = NULL;
Pos * near head;
#endif
word32 window_size;
/* window size, 2*WSIZE except for MMAP or BIG_MEM, where it is the
* input file length plus MIN_LOOKAHEAD.
*/

long block_start;
/* window position at the beginning of the current output block. Gets
* negative when the window is moved backwards.
*/

static unsigned ins_h; /* hash index of string to be inserted */

#define H_SHIFT ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
* input step. It must be such that after MIN_MATCH steps, the oldest
* byte no longer takes part in the hash key, that is:
* H_SHIFT * MIN_MATCH >= HASH_BITS
*/

unsigned int near prev_length;
/* Length of the best match at previous step. Matches not greater than this
* are discarded. This is used in the lazy match evaluation.
*/

unsigned near strstart; /* start of string to insert */
unsigned near match_start; /* start of matching string */
static int eofile; /* flag set at end of input file */
static unsigned lookahead; /* number of valid bytes ahead in window */

/* Saved copies of lazy match variables */
static int s_match_available;
static int s_match_length;

unsigned near max_chain_length;
/* To speed up deflation, hash chains are never searched beyond this length.
* A higher limit improves compression ratio but degrades the speed.
*/

static unsigned int max_lazy_match;
/* Attempt to find a better match only when the current match is strictly
* smaller than this value. This mechanism is used only for compression
* levels >= 4.
*/
#define max_insert_length max_lazy_match
/* Insert new strings in the hash table only if the match length
* is not greater than this length. This saves time but degrades compression.
* max_insert_length is used only for compression levels <= 3.
*/

unsigned near good_match;
/* Use a faster search when the previous match is longer than this */

#ifdef FULL_SEARCH
# define nice_match MAX_MATCH
#else
int near nice_match; /* Stop searching when current match exceeds this */
#endif


/* Values for max_lazy_match, good_match and max_chain_length, depending on
* the desired pack level (0..9). The values given below have been tuned to
* exclude worst case performance for pathological files. Better values may be
* found for specific files.
*/

static const struct {
word16 good_length; /* reduce lazy search above this match length */
word16 max_lazy; /* do not perform lazy search above this match length */
word16 nice_length; /* quit search above this match length */
word16 max_chain;
} configuration_table[10] = {
/* good lazy nice chain */
	/* 0 */ {0,	0, 0, 0}, /* store only */
	/* 1 */ {4,	4, 8, 4}, /* maximum speed, no lazy matches */
	/* 2 */ {4,	5, 16, 8},
	/* 3 */ {4,	6, 32, 32},

/* 4 */ {4, 4, 16, 16}, /* lazy matches */
#if 0
/* 5 */ {8, 16, 32, 32}, /* Change to new? */
#else
 {8, 64, 258, 128}, /* Match old PGP params */
#endif
/* 6 */ {8, 16, 128, 128},
/* 7 */ {8, 32, 128, 256},
/* 8 */ {32, 128, 258, 1024},
/* 9 */ {32, 258, 258, 4096}}; /* maximum compression */

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
* For deflate_fast() (levels <= 3) good is ignored and lazy has a different
* meaning.
*/

/* ===========================================================================
* Prototypes for local functions.
*/

static void slide_window(void);
static int deflate_sub(void);

	int longest_match(IPos cur_match);
#ifdef ASMV
void match_init(void); /* asm code initialization */
#endif

#ifdef ZIPDEBUG
static void check_match(IPos start, IPos match, int length);
#endif

/* ===========================================================================
* Update a hash value with the given input byte
* IN assertion: all calls to to UPDATE_HASH are made with consecutive
* input characters, so that a running hash key can be computed from the
* previous key instead of complete recalculation each time.
*/
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

/* ===========================================================================
* Insert string s in the dictionary and set match_head to the previous head
* of the hash chain (the most recent string with same hash key). Return
* the previous length of the hash chain.
* IN assertion: all calls to to INSERT_STRING are made with consecutive
* input characters and the first MIN_MATCH bytes of s are valid
* (except for the last MIN_MATCH-1 bytes of the input file).
*/
#define INSERT_STRING(s, match_head) \
(UPDATE_HASH(ins_h, window[(s) + MIN_MATCH-1]), \
prev[(s) & WMASK] = (Pos)((match_head) = head[ins_h]), \
head[ins_h] = (Pos)(s))

/* ===========================================================================
* Initialize the "longest match" routines for a new file
*
* IN assertion: window_size is > 0 if the input file is already read or
* mmap'ed in the window[] array, 0 otherwise. In the first case,
* window_size is sufficient to contain the whole input file plus
* MIN_LOOKAHEAD bytes (to avoid referencing memory beyond the end
* of window[] when looking for matches towards the end).
*/
int
lm_init(int pack_level) /* 0: store, 1: best speed, 9: best compression */
	{
		if (pack_level < 1 || pack_level > 9)
		return -1;
		/* error("bad pack level"); */

		/* Do not slide the window if the whole input is already in memory
		* (window_size > 0)
		*/
		if (window_size == 0)
		window_size = (word32)2*WSIZE;

/* Use dynamic allocation if compiler does not like big static arrays: */
/* The +8 is for MS-DOS, so the pointers can be segment-aligned */
#ifdef DYN_ALLOC
		if (o_window == NULL) {
		window = o_window = (byte *)fcalloc(WSIZE+8, 2*sizeof(byte));
		if (o_window == NULL)
		return -1;
		/* err(ZE_MEM, "window allocation"); */
		}
		if (o_prev == NULL) {
		prev = o_prev = (Pos*) fcalloc(WSIZE+8,	sizeof(Pos));
		head = o_head = (Pos*) fcalloc(HASH_SIZE+8, sizeof(Pos));
		if (o_prev == NULL || o_head == NULL) {
		if (o_prev)
			fcfree(o_prev);
			fcfree(o_window);
			o_window = 0;
			return -1;
			/* err(ZE_MEM, "hash table allocation"); */
		}
}
#endif /* DYN_ALLOC */

/* Initialize the hash table (avoiding 64K overflow for 16 bit systems).
* prev[] will be initialized on the fly.
*/
head[HASH_SIZE-1] = NIL;
memset((char*)head, NIL, (unsigned)(HASH_SIZE-1)*sizeof(*head));

/* Set the default configuration parameters:
*/
max_lazy_match = configuration_table[pack_level].max_lazy;
good_match = configuration_table[pack_level].good_length;
#ifndef FULL_SEARCH
nice_match = configuration_table[pack_level].nice_length;
#endif
max_chain_length = configuration_table[pack_level].max_chain;
/* ??? reduce max_chain_length for binary files */

strstart = 0;
block_start = 0L;
#ifdef ASMV
match_init(); /* initialize the asm code */
#endif

lookahead = 0;
eofile = 0;

s_match_available = 0; /* set if previous match exists */
s_match_length = MIN_MATCH-1; /* length of best match */

return 0;
}

/* ===========================================================================
* Free the window and hash table
*/
void
lm_free(void)
{
#ifdef PGP
		/* Security wipe of static variables */
		block_start = 0;
		ins_h = 0;
		prev_length = 0;
		strstart = 0;
		match_start = 0;
		s_match_available = 0;
		s_match_length = 0;
		/* eofile is 1 and lookahead is 0 - no need to wipe */
	#endif

#ifdef DYN_ALLOC
	if (o_window != NULL) {
#ifdef PGP
 memset(o_window, 0, WSIZE);
 memset(o_window+WSIZE, 0, WSIZE+16);
#endif
 fcfree(o_window);
 o_window = window = NULL;
}
if (o_prev != NULL) {
#ifdef PGP
		memset(o_prev, 0, WSIZE/2*sizeof(Pos));
		memset(o_prev+WSIZE/2, 0, WSIZE/2*sizeof(Pos)+16);
		memset(o_head, 0, HASH_SIZE/2*sizeof(Pos));
		memset(o_head+HASH_SIZE/2, 0, HASH_SIZE/2*sizeof(Pos)+16);
	#endif
		fcfree(o_prev);
		fcfree(o_head);
		o_prev = prev = o_head = head = NULL;
	}
#endif /* DYN_ALLOC */
}

/* ===========================================================================
* Set match_start to the longest match starting at the given string and
* return its length. Matches shorter or equal to prev_length are discarded,
* in which case the result is equal to prev_length and match_start is
* garbage.
* IN assertions: cur_match is the head of the hash chain for the current
* string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
*/
#ifndef ASMV
/* For MSDOS, OS/2 and 386 Unix, an optimized version is in match.asm or
* match.s. The code is functionally equivalent, so you can use the C version
* if desired. A 68000 version is in amiga/match_68.a -- this could be used
* with other 68000 based systems such as Macintosh with a little effort.
*/
int
longest_match(IPos cur_match)
{
unsigned chain_length = max_chain_length; /* max hash chain length */
register byte *scan = window + strstart; /* current string */
register byte *match; /* matched string */
register int len; /* length of current match */
int best_len = prev_length; /* best match length so far */
IPos limit = strstart > (IPos)MAX_DIST ? strstart - (IPos)MAX_DIST : NIL;
/* Stop when cur_match becomes <= limit. To simplify the code,
* we prevent matches with the string of window index 0.
*/

/* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
* It is easy to get rid of this optimization if necessary.
*/
#if HASH_BITS < 8 || MAX_MATCH != 258
# error Code too clever
#endif

#if UNALIGNED_OK
/* Compare two bytes at a time. Note: this is not always beneficial.
* Try with and without -DUNALIGNED_OK to check.
*/
register byte *strend = window + strstart + MAX_MATCH - 1;
register word16 scan_start = *(word16 *)scan;
register word16 scan_end = *(word16 *)(scan+best_len-1);
#else
register byte *strend = window + strstart + MAX_MATCH;
register byte scan_end1 = scan[best_len-1];
register byte scan_end = scan[best_len];
#endif

		/* Do not waste too much time if we already have a good match: */
		if (prev_length >= good_match)
		chain_length >>= 2;
		ZipAssert(strstart <= window_size-MIN_LOOKAHEAD, "insufficient lookahead");

do {
		ZipAssert(cur_match < strstart, "no future");
		match = window + cur_match;

/* Skip to next match if the match length cannot increase
* or if the match length is less than 2:
*/
#if UNALIGNED_OK && MAX_MATCH == 258
		/* This code assumes sizeof(unsigned short) == 2. Do not use
		* UNALIGNED_OK if your compiler uses a different size.
		*/
		if (*(word16 *)(match+best_len-1) != scan_end ||
		*(word16 *)match != scan_start) continue;

		/* It is not necessary to compare scan[2] and match[2] since they are
		* always equal when the other bytes match, given that the hash keys
		* are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
		* strstart+3, +5, ... up to strstart+257. We check for insufficient
		* lookahead only every 4th comparison; the 128th check will be made
		* at strstart+257. If MAX_MATCH-2 is not a multiple of 8, it is
		* necessary to put more guard bytes at the end of the window, or
		* to check more often for insufficient lookahead.
		*/
		scan++, match++;
		do {
		} while (*(word16 *)(scan+=2) == *(word16 *)(match+=2) &&
		 *(word16 *)(scan+=2) == *(word16 *)(match+=2) &&
		*(word16 *)(scan+=2) == *(word16 *)(match+=2) &&
		*(word16 *)(scan+=2) == *(word16 *)(match+=2) &&
		scan < strend);
		/* The funny "do {}" generates better code on most compilers */

		/* Here, scan <= window+strstart+257 */
		ZipAssert(scan <= window+(unsigned)(window_size-1), "wild scan");
		if (*scan == *match)
		 scan++;

		len = (MAX_MATCH - 1) - (int)(strend-scan);
		scan = strend - (MAX_MATCH-1);

#else /* UNALIGNED_OK */

		if (match[best_len] != scan_end ||
		match[best_len-1] != scan_end1 ||
		*match != *scan ||
	*++match != scan[1])	continue;

/* The check at best_len-1 can be removed because it will be made
		* again later. (This heuristic is not always a win.)
		* It is not necessary to compare scan[2] and match[2] since they
		* are always equal when the other bytes match, given that
		* the hash keys are equal and that HASH_BITS >= 8.
		*/
		scan += 2, match++;

		/* We check for insufficient lookahead only every 8th comparison;
		* the 256th check will be made at strstart+258.
		*/
		do {
		} while (*++scan == *++match && *++scan == *++match &&
		*++scan == *++match && *++scan == *++match &&
		 *++scan == *++match && *++scan == *++match &&
		*++scan == *++match && *++scan == *++match &&
		scan < strend);

		len = MAX_MATCH - (int)(strend - scan);
		scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

		if (len > best_len) {
		match_start = cur_match;
		best_len = len;
		if (len >= nice_match)
		 break;
#if UNALIGNED_OK
		scan_end = *(word16 *)(scan+best_len-1);
#else
		scan_end1 = scan[best_len-1];
		scan_end = scan[best_len];
	#endif
		}
} while ((cur_match = prev[cur_match & WMASK]) > limit
&& --chain_length != 0);

return best_len;
}
#endif /* ASMV */

#ifdef ZIPDEBUG
/* ===========================================================================
* Check that the match at match_start is indeed a match.
*/
static void
check_match(IPos start, IPos match, int length)
{
/* check that the match is indeed a match */
if (memcmp(window + match, window + start, length) != 0) {
fprintf(stderr, " start %d, match %d, length %d\n",
start, match, length);
error("invalid match");
}
if (verbose > 1) {
fprintf(stderr,"\\[%d,%d]", start-match, length);
do { putc(window[start++], stderr); } while (--length != 0);
}
  }
#else
# define check_match(start, match, length)
#endif

static void
slide_window(void)
	{
		register unsigned n, m;

		memcpy(window, window+WSIZE, (size_t)WSIZE);
		match_start -= WSIZE;
		strstart -= WSIZE; /* we now have strstart >= MAX_DIST: */

		block_start -= (long) WSIZE;

		for (n = 0; n < HASH_SIZE; n++) {
		m = head[n];
		head[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
		}
		for (n = 0; n < WSIZE; n++) {
		m = prev[n];
		prev[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
		/* If n is not on any hash chain, prev[n] is garbage but
		* its value will never be used.
		*/
		}
	}

/*	===========================================================================
* Flush the current block, with given end-of-file flag.
* IN assertion: strstart is set to the end of the current match.
*/
#define FLUSH_BLOCK(eof) \
	flush_block(block_start >= 0L ? (char*)&window[(unsigned)block_start] : \
  (char*)NULL, (word32)strstart - block_start, (eof))

/* ===========================================================================
* Same as above, but achieves better compression. We use a lazy
* evaluation for matches: a match is finally adopted only if there is
* no better match at the next window position.
*/
	static int
	deflate_sub(void)
{
	IPos hash_head;	/* head of hash chain */
	IPos prev_match;	/* previous match */
	int flush;	/* set if current block must be flushed */
int match_available = 0; /* set if previous match exists */
register unsigned match_length = MIN_MATCH-1; /* length of best match */

#if 0
if (level <= 3) return deflate_fast(); /* optimized for speed */
#endif

match_available = s_match_available;
match_length = s_match_length;

/* Process the input block. */
while (lookahead != 0) {
/* Insert the string window[strstart .. strstart+2] in the
* dictionary, and set hash_head to the head of the hash chain:
*/
INSERT_STRING(strstart, hash_head);

/* Find the longest match, discarding those <= prev_length.
*/
prev_length = match_length, prev_match = match_start;
match_length = MIN_MATCH-1;

if (hash_head != NIL && prev_length < max_lazy_match &&
			strstart - hash_head <= MAX_DIST) {
			/* To simplify the code, we prevent matches with the string
			* of window index 0 (in particular we have to avoid a match
			* of the string with itself at the start of the input file).
			*/
match_length = longest_match (hash_head);
/* longest_match() sets match_start */
if (match_length > lookahead) match_length = lookahead;

/* Ignore a length 3 match if it is too distant: */
if (match_length == MIN_MATCH && strstart-match_start > TOO_FAR){
/* If prev_match is also MIN_MATCH, match_start is garbage
* but we will ignore the current match anyway.
*/
			match_length--;
			}
		}
/* If there was a match at the previous step and the current
* match is not better, output the previous match:
*/
if (prev_length >= MIN_MATCH && match_length <= prev_length) {

check_match(strstart-1, prev_match, prev_length);

flush = ct_tally(strstart-1-prev_match, prev_length - MIN_MATCH);

/* Insert in hash table all strings up to the end of the match.
* strstart-1 and strstart are already inserted.
*/
			lookahead -= prev_length-1;
			prev_length -= 2;
			do {
			strstart++;
			INSERT_STRING(strstart, hash_head);
			/* strstart never exceeds WSIZE-MAX_MATCH, so there are
			* always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
			* these bytes are garbage, but it does not matter since the
			* next lookahead bytes will always be emitted as literals.
			*/
} while (--prev_length != 0);
match_available = 0;
match_length = MIN_MATCH-1;
strstart++;
if (flush) FLUSH_BLOCK(0), block_start = strstart;

} else if (match_available) {
			/* If there was no match at the previous position, output a
			* single literal. If there was a match but the current match
			* is longer, truncate the previous match to a single literal.
			*/
			Tracevv((stderr,"%c",window[strstart-1]));
			if (ct_tally (0, window[strstart-1])) {
			FLUSH_BLOCK(0), block_start = strstart;
			}
			strstart++;
			lookahead--;
} else {
/* There is no previous match to compare with, wait for
* the next step to decide.
*/
			match_available = 1;
			strstart++;
			lookahead--;
		}

		/* Make sure that we always have enough lookahead, except
		* at the end of the input file. We need MAX_MATCH bytes
		* for the next match, plus MIN_MATCH bytes to insert the
		* string following the next match.
		*/
		if (lookahead < MIN_LOOKAHEAD && !eofile) {
			s_match_available = match_available;
			s_match_length = match_length;
			return 0;
			/* fill_window();*/
		}
}
if (match_available) ct_tally (0, window[strstart-1]);

	return FLUSH_BLOCK(1); /* eof */
}


/* Zip a buffer of input */
void
zip_input(char const *buf, unsigned len)
	{
		unsigned more;

		while (len) {
		more = (unsigned)((word32)2*WSIZE - lookahead - strstart);

		if (more <= 1) {
		slide_window();
		more += WSIZE;
		}
		if (more > len)
		more = len;
		memcpy(window+strstart+lookahead, buf, more);
		lookahead += more;

		if (lookahead >= MIN_LOOKAHEAD) {
		if (!strstart) { /* First time */
				register int j;
				ins_h = 0;
				for (j=0; j<MIN_MATCH-1; j++)
				UPDATE_HASH(ins_h, window[j]);
			}
		deflate_sub();
		}
		buf += more;
		len -= more;
}

		return;
	}

/* Zip EOF */
void
zip_finish(void)
	{
		eofile = 1;

		if (!strstart) { /* First time */
		register int j;
		ins_h = 0;
		for (j=0; j<MIN_MATCH-1; j++)
		UPDATE_HASH(ins_h, window[j]);
		}

		deflate_sub();

		return;
	}
