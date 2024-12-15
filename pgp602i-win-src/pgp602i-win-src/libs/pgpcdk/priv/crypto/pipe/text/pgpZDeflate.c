/*
 * Copyright (C) 1990-1993 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 * Kai Uwe Rommel and Igor Mandrichenko.
 * Permission is granted to any individual or institution to use, copy,
 * or redistribute this software so long as all of the original files
 * are included, that it is not sold for profit, and that this copyright
 * notice is retained.
 *
 * Hacked up for PGP by Colin Plumb
 *
 * $Id: pgpZDeflate.c,v 1.10 1997/12/12 01:14:11 heller Exp $
 */

/*
 * deflate.c by Jean-loup Gailly.
 *
 * PURPOSE
 *
 *   Identify new text as repetitions of old text within a fixed-
 *   length sliding window trailing behind the new text.
 *
 * DISCUSSION
 *
 *   The "deflation" process depends on being able to identify portions
 *   of the input text which are identical to earlier input (within a
 *   sliding window trailing behind the input currently being processed).
 *
 *   The most straightforward technique turns out to be the fastest for
 *   most input files: try all possible matches and select the longest.
 *   The key feature of this algorithm is that insertions into the string
 *   dictionary are very simple and thus fast, and deletions are avoided
 *   completely. Insertions are performed at each input character,
 *   whereas string matches are performed only when the previous match
 *   ends. So it is preferable to spend more time in matches to allow very
 *   fast string insertions and avoid deletions. The matching algorithm
 *   for small strings is inspired from that of Rabin & Karp.  A brute
 *   force approach is used to find longer strings when a small match has
 *   been found.  A similar algorithm is used in comic (by Jan-Mark Wams)
 *   and freeze (by Leonid Broukhis).
 *
 *   A previous version of this file used a more sophisticated algorithm
 *   (by Fiala and Greene) which is guaranteed to run in linear amortized
 *   time, but has a larger average cost, uses more memory and is patented.
 *   However the F&G algorithm may be faster for some highly redundant
 *   files if the parameter max_chain_length (described below) is too large.
 *
 * ACKNOWLEDGEMENTS
 *
 *   The idea of lazy evaluation of matches is due to Jan-Mark Wams, and
 *   I found it in 'freeze' written by Leonid Broukhis.
 *   Thanks to many info-zippers for bug reports and testing.
 *
 * REFERENCES
 *
 *   APPNOTE.TXT documentation file in PKZIP 1.93a distribution.
 *
 *   A description of the Rabin and Karp algorithm is given in the book
 *      "Algorithms" by R. Sedgewick, Addison-Wesley, p252.
 *
 *   Fiala,E.R., and Greene,D.H.
 *      Data Compression with Finite Windows, Comm.ACM, 32,4 (1989) 490-595
 *
 * INTERFACE
 *
 *   int lm_init (int pack_level)
 *       Initialize the "longest match" routines for a new file
 *
 *   PGPUInt32 deflate (void)
 *       Processes a new input file and return its compressed length. Sets
 *       the compressed length, crc, deflate flags and internal file
 *       attributes.
 */

#include "pgpConfig.h"
#include "pgpContext.h"

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
#   define HASH_BITS  13  /* Number of bits used to hash strings */
#endif
#ifdef MEDIUM_MEM
#   define HASH_BITS  14
#endif
#ifndef HASH_BITS
#   define HASH_BITS  15
   /* For portability to 16 bit machines, do not use values above 15. */
#endif

#define HASH_SIZE (unsigned)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK     (WSIZE-1)
/* HASH_SIZE and WSIZE must be powers of two */

#define NIL 0
/* Tail of hash chains */

#ifndef TOO_FAR
#  define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

#ifdef ATARI_ST
#  undef MSDOS /* avoid the processor specific parts */
   /* (but the Atari should never define MSDOS anyway ...) */
#endif
#if defined(MSDOS) && !defined(NO_ASM) && !defined(ASMV)
#  define ASMV
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
  typedef PGPUInt16 Pos;
#endif
typedef unsigned IPos;
/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

/* Private context structure for use by all functions. */
typedef struct ZDeflateContext {

	PGPContextRef		cdkContext;

#ifndef DYN_ALLOC
	PGPByte window[2L*WSIZE];
	/* Sliding window.  Input bytes are read into the second half of
	 * the window, and move to the first half later to keep a
	 * dictionary of at least WSIZE bytes.  With this organization,
	 * matches are limited to a distance of WSIZE-MAX_MATCH bytes, but
	 * this ensures that IO is always performed with a length multiple
	 * of the block size.  Also, it limits the window size to 64K,
	 * which is quite useful on MSDOS.  To do: limit the window size
	 * to WSIZE+BSZ if SMALL_MEM (the code would be less efficient
	 * since the data would have to be copied WSIZE/BSZ times)
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
	PGPByte * near o_window;
	Pos  * near o_prev;
	Pos  * near o_head;
	PGPByte * near window;
	Pos  * near prev;
	Pos  * near head;
#endif

	PGPUInt32 window_size;
	/* window size, 2*WSIZE except for MMAP or BIG_MEM, where it is the
	 * input file length plus MIN_LOOKAHEAD.
	 */

	long block_start;
	/* window position at the beginning of the current output block. Gets
	 * negative when the window is moved backwards.
	 */

	unsigned ins_h;  /* hash index of string to be inserted */
	unsigned int near prev_length;
	/* Length of the best match at previous step. Matches not greater than this
	 * are discarded. This is used in the lazy match evaluation.
	 */

	unsigned near strstart;         /* start of string to insert */
	unsigned near match_start;      /* start of matching string */
	int           eofile;    /* flag set at end of input file */
	unsigned      lookahead; /* number of valid bytes ahead in window */

	/* Saved copies of lazy match variables */
	int s_match_available;
	int s_match_length;

	unsigned near max_chain_length;
	/* To speed up deflation, hash chains are never searched beyond
	 * this length.  A higher limit improves compression ratio but
	 * degrades the speed.
	 */

	unsigned int max_lazy_match;
	/* Attempt to find a better match only when the current match is strictly
	 * smaller than this value. This mechanism is used only for compression
	 * levels >= 4.
	 */
#define max_insert_length  max_lazy_match
	/* Insert new strings in the hash table only if the match length
	 * is not greater than this length. This saves time but degrades
	 * compression.  max_insert_length is used only for compression
	 * levels <= 3.
	 */

	unsigned near good_match;
	/* Use a faster search when the previous match is longer than this */

#ifdef  FULL_SEARCH
# define nice_match MAX_MATCH
#else
	  int near nice_match; /* Stop searching when current match exceeds this */
#endif

	/*	DEBUG_STRUCT_CONSTRUCTOR( ZDeflateContext ) */
} ZDeflateContext;


#define H_SHIFT  ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS
 */


/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */

static const struct {
   PGPUInt16 good_length; /* reduce lazy search above this match length */
   PGPUInt16 max_lazy;/* don't perform lazy search above this match length */
   PGPUInt16 nice_length; /* quit search above this match length */
   PGPUInt16 max_chain;
} configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0},	/* store only */
/* 1 */ {4,    4,  8,    4},	/* maximum speed, no lazy matches */
/* 2 */ {4,    5, 16,    8},
/* 3 */ {4,    6, 32,   32},

/* 4 */ {4,    4, 16,   16},	/* lazy matches */
#if 0
/* 5 */ {8,   16, 32,   32},	/* Change to new? */
#else
	{8,   64, 258, 128},	/* Match old PGP params */
#endif 
/* 6 */ {8,   16, 128, 128},
/* 7 */ {8,   32, 128, 256},
/* 8 */ {32, 128, 258, 1024},
/* 9 */ {32, 258, 258, 4096}};	/* maximum compression */

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 * For deflate_fast() (levels <= 3) good is ignored and lazy has a different
 * meaning.
 */

/* ===========================================================================
 *  Prototypes for local functions.
 */

static void slide_window(ZDeflateContext *);
static int deflate_sub(ZDeflateContext *, struct ZTreesContext *,
	struct ZBitsContext *);

int  longest_match(ZDeflateContext *, IPos cur_match);
#ifdef ASMV
void match_init(void); /* asm code initialization */
#endif

#ifdef ZIPDEBUG
static void check_match(ZDeflateContext *, IPos start, IPos match, int length);
#endif

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

/* ===========================================================================
 * Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#define INSERT_STRING(ctx, s, match_head) \
  (UPDATE_HASH((ctx)->ins_h, (ctx)->window[(s) + MIN_MATCH-1]), \
  (ctx)->prev[(s) & WMASK] = (Pos)((match_head) = (ctx)->head[(ctx)->ins_h]), \
  (ctx)->head[(ctx)->ins_h] = (Pos)(s))

/* ===========================================================================
 * Initialize the "longest match" routines for a new file
 *
 * IN assertion: window_size is > 0 if the input file is already read or
 *    mmap'ed in the window[] array, 0 otherwise. In the first case,
 *    window_size is sufficient to contain the whole input file plus
 *    MIN_LOOKAHEAD bytes (to avoid referencing memory beyond the end
 *    of window[] when looking for matches towards the end).
 */
ZDeflateContext *
lm_init(PGPContextRef cdkContext, int pack_level)
	/* 0: store, 1: best speed, 9: best compression */
{
	ZDeflateContext *ctx;

    if (pack_level < 1 || pack_level > 9)
		return NULL;
	/* error("bad pack level"); */

	ctx = (ZDeflateContext *) pgpContextMemAlloc (cdkContext, 
								sizeof(*ctx), kPGPMemoryMgrFlags_Clear);
	if (IsNull (ctx) )
		return NULL;
	ctx->cdkContext = cdkContext;

    /* Do not slide the window if the whole input is already in memory
     * (window_size > 0)
     */
    if (ctx->window_size == 0)
		ctx->window_size = (PGPUInt32)2*WSIZE;

    /* Use dynamic allocation if compiler does not like big static arrays: */
    /* The +8 is for MS-DOS, so the pointers can be segment-aligned */
#ifdef DYN_ALLOC
	pgpAssert (ctx->o_window == NULL);
	ctx->window = ctx->o_window = (PGPByte *)pgpContextMemAlloc (cdkContext,
						(WSIZE+8)*2*sizeof(PGPByte), kPGPMemoryMgrFlags_Clear);
	if (ctx->o_window == NULL) {
		pgpContextMemFree (cdkContext, ctx);
		return NULL;
		/* err(ZE_MEM, "window allocation"); */
	}
	pgpAssert (ctx->o_prev == NULL);
	ctx->prev = ctx->o_prev = (Pos*) pgpContextMemAlloc (cdkContext,
				(WSIZE+8) * sizeof(Pos), kPGPMemoryMgrFlags_Clear);
	ctx->head = ctx->o_head = (Pos*) pgpContextMemAlloc (cdkContext,
				(HASH_SIZE+8) * sizeof(Pos), kPGPMemoryMgrFlags_Clear);
	if (ctx->o_prev == NULL || ctx->o_head == NULL) {
		if (ctx->o_prev)
			pgpContextMemFree (cdkContext, ctx->o_prev);
		pgpContextMemFree (cdkContext, ctx->o_window);
		pgpContextMemFree (cdkContext, ctx);
		return NULL;
		/* err(ZE_MEM, "hash table allocation"); */
	}
#endif /* DYN_ALLOC */

    /* Initialize the hash table (avoiding 64K overflow for 16 bit systems).
     * prev[] will be initialized on the fly.
     */
    ctx->head[HASH_SIZE-1] = NIL;
    pgpClearMemory((char*)ctx->head, (unsigned)(HASH_SIZE-1)*
				   					sizeof(*ctx->head));

    /* Set the default configuration parameters:
     */
    ctx->max_lazy_match   = configuration_table[pack_level].max_lazy;
    ctx->good_match       = configuration_table[pack_level].good_length;
#ifndef FULL_SEARCH
    ctx->nice_match       = configuration_table[pack_level].nice_length;
#endif
    ctx->max_chain_length = configuration_table[pack_level].max_chain;
    /* ??? reduce max_chain_length for binary files */

    ctx->strstart = 0;
    ctx->block_start = 0L;
#ifdef ASMV
    match_init(ctx); /* initialize the asm code */
#endif

    ctx->lookahead = 0;
    ctx->eofile = 0;

    ctx->s_match_available = 0; /* set if previous match exists */
    ctx->s_match_length = MIN_MATCH-1; /* length of best match */

    return ctx;
}

/* ===========================================================================
 * Free the window and hash table
 */
void
lm_free(ZDeflateContext *ctx)
{
	PGPContextRef cdkContext;

	cdkContext = ctx->cdkContext;

#ifdef DYN_ALLOC
	/* Wipe and free buffers */
    if (ctx->o_window != NULL) {
		pgpClearMemory(ctx->o_window, (WSIZE+8)*2*sizeof(PGPByte));
		pgpContextMemFree (cdkContext, ctx->o_window);
    }
    if (ctx->o_prev != NULL) {
		pgpClearMemory(ctx->o_prev, (WSIZE+8) * sizeof(Pos));
		pgpClearMemory(ctx->o_head, (HASH_SIZE+8) * sizeof(Pos));
		pgpContextMemFree (cdkContext, ctx->o_prev);
		pgpContextMemFree (cdkContext, ctx->o_head);
    }
#endif /* DYN_ALLOC */

	/* Wipe everything else */
	pgpClearMemory (ctx, sizeof(*ctx));
	pgpContextMemFree (cdkContext, ctx);
}

/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */
#ifndef ASMV
/* For MSDOS, OS/2 and 386 Unix, an optimized version is in match.asm or
 * match.s. The code is functionally equivalent, so you can use the C version
 * if desired.  A 68000 version is in amiga/match_68.a -- this could be used
 * with other 68000 based systems such as Macintosh with a little effort.
 */
int
longest_match(ZDeflateContext *ctx, IPos cur_match)
{
    unsigned chain_length = ctx->max_chain_length; /* max hash chain length */
    register PGPByte *scan = ctx->window + ctx->strstart; /* current string */
    register PGPByte *match;                    /* matched string */
    register int len;                           /* length of current match */
    int best_len = ctx->prev_length;            /* best match length so far */
    IPos limit = ctx->strstart > (IPos)MAX_DIST ?
						ctx->strstart - (IPos)MAX_DIST : NIL;
    /* Stop when cur_match becomes <= limit.  To simplify the code,
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
    register PGPByte *strend = ctx->window + ctx->strstart + MAX_MATCH - 1;
    register PGPUInt16 scan_start = *(PGPUInt16 *)scan;
    register PGPUInt16 scan_end   = *(PGPUInt16 *)(scan+best_len-1);
#else
    register PGPByte *strend = ctx->window + ctx->strstart + MAX_MATCH;
    register PGPByte scan_end1  = scan[best_len-1];
    register PGPByte scan_end   = scan[best_len];
#endif

    /* Do not waste too much time if we already have a good match: */
    if (ctx->prev_length >= ctx->good_match)
        chain_length >>= 2;
    ZipAssert(ctx->strstart <= ctx->window_size-MIN_LOOKAHEAD,
			  	"insufficient lookahead");

    do {
		ZipAssert(cur_match < ctx->strstart, "no future");
		match = ctx->window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
		 */
#if UNALIGNED_OK && MAX_MATCH == 258
		/* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
		if (*(PGPUInt16 *)(match+best_len-1) != scan_end ||
            *(PGPUInt16 *)match != scan_start) continue;

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
        } while (*(PGPUInt16 *)(scan+=2) == *(PGPUInt16 *)(match+=2) &&
				 *(PGPUInt16 *)(scan+=2) == *(PGPUInt16 *)(match+=2) &&
                 *(PGPUInt16 *)(scan+=2) == *(PGPUInt16 *)(match+=2) &&
                 *(PGPUInt16 *)(scan+=2) == *(PGPUInt16 *)(match+=2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window+strstart+257 */
        ZipAssert(scan <= ctx->window+(unsigned)(ctx->window_size-1),
				  "wild scan");
        if (*scan == *match)
			scan++;

		len = (MAX_MATCH - 1) - (int)(strend-scan);
		scan = strend - (MAX_MATCH-1);

#else /* UNALIGNED_OK */

        if (match[best_len]   != scan_end  ||
			match[best_len-1] != scan_end1 ||
            *match            != *scan     ||
            *++match          != scan[1])      continue;

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
			ctx->match_start = cur_match;
			best_len = len;
			if (len >= ctx->nice_match)
				break;
#if UNALIGNED_OK
			scan_end = *(PGPUInt16 *)(scan+best_len-1);
#else
			scan_end1  = scan[best_len-1];
			scan_end   = scan[best_len];
#endif
		}
    } while ((cur_match = ctx->prev[cur_match & WMASK]) > limit
             && --chain_length != 0);

    return best_len;
}
#endif /* ASMV */

#ifdef ZIPDEBUG
/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
static void
check_match(ZDeflateContext *ctx, IPos start, IPos match, int length)
{
    /* check that the match is indeed a match */
    if (memcmp(ctx->window + match, ctx->window + start, length) != 0) {
        fprintf(stderr, " start %d, match %d, length %d\n",
                start, match, length);
        error("invalid match");
    }
    if (ctx->verbose > 1) {
		fprintf(stderr,"\\[%d,%d]", start-match, length);
		do { putc(ctx->window[start++], stderr); } while (--length != 0);
    }
}
#else
#  define check_match(ctx, start, match, length)
#endif

static void
slide_window(ZDeflateContext *ctx)
{
    register unsigned n, m;

    memcpy(ctx->window, ctx->window+WSIZE, (size_t)WSIZE);
    ctx->match_start -= WSIZE;
    ctx->strstart    -= WSIZE; /* we now have strstart >= MAX_DIST: */

    ctx->block_start -= (long) WSIZE;

    for (n = 0; n < HASH_SIZE; n++) {
		m = ctx->head[n];
		ctx->head[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
    }
    for (n = 0; n < WSIZE; n++) {
		m = ctx->prev[n];
		ctx->prev[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
		/* If n is not on any hash chain, prev[n] is garbage but
		 * its value will never be used.
		 */
    }
}

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define FLUSH_BLOCK(ctx, ztctx, zbctx, eof) \
   flush_block((ztctx), (zbctx), (ctx)->block_start >= 0L ? \
		(char*)&(ctx)->window[(unsigned)(ctx)->block_start] : \
		(char*)NULL, (PGPUInt32)(ctx)->strstart - (ctx)->block_start, (eof), \
		(ctx)->window)

/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
static int
deflate_sub(ZDeflateContext *ctx, struct ZTreesContext *ztcontext,
	struct ZBitsContext *zbcontext)
{
    IPos hash_head;          /* head of hash chain */
    IPos prev_match;         /* previous match */
    int flush;               /* set if current block must be flushed */
    int match_available = 0; /* set if previous match exists */
    register unsigned match_length = MIN_MATCH-1; /* length of best match */

#if 0
    if (ctx->level <= 3) return deflate_fast(ctx); /* optimized for speed */
#endif

    match_available = ctx->s_match_available;
    match_length = ctx->s_match_length;

    /* Process the input block. */
    while (ctx->lookahead != 0) {
        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        INSERT_STRING(ctx, ctx->strstart, hash_head);

		/* Find the longest match, discarding those <= prev_length.
		 */
		ctx->prev_length = match_length, prev_match = ctx->match_start;
        match_length = MIN_MATCH-1;

        if (hash_head != NIL && ctx->prev_length < ctx->max_lazy_match &&
			ctx->strstart - hash_head <= MAX_DIST) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
			 * of the string with itself at the start of the input file).
             */
            match_length = longest_match (ctx, hash_head);
            /* longest_match() sets match_start */
            if (match_length > ctx->lookahead) match_length = ctx->lookahead;

            /* Ignore a length 3 match if it is too distant: */
            if (match_length == MIN_MATCH &&
				ctx->strstart-ctx->match_start > TOO_FAR){
                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
				 */
				match_length--;
			}
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
		if (ctx->prev_length >= MIN_MATCH &&
			match_length <= ctx->prev_length) {

            check_match(ctx, ctx->strstart-1, prev_match, ctx->prev_length);

            flush = ct_tally(ztcontext, ctx->strstart-1-prev_match,
							 ctx->prev_length - MIN_MATCH,
							 ctx->block_start, ctx->strstart);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted.
			 */
            ctx->lookahead -= ctx->prev_length-1;
			ctx->prev_length -= 2;
            do {
                ctx->strstart++;
                INSERT_STRING(ctx, ctx->strstart, hash_head);
				/* strstart never exceeds WSIZE-MAX_MATCH, so there are
                 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                 * these bytes are garbage, but it does not matter since the
				 * next lookahead bytes will always be emitted as literals.
                 */
            } while (--ctx->prev_length != 0);
            match_available = 0;
            match_length = MIN_MATCH-1;
            ctx->strstart++;
            if (flush) {
				FLUSH_BLOCK(ctx, ztcontext, zbcontext, 0);
				ctx->block_start = ctx->strstart;
			}

        } else if (match_available) {
			/* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
			 * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c",ctx->window[ctx->strstart-1]));
            if (ct_tally (ztcontext, 0, ctx->window[ctx->strstart-1],
						  ctx->block_start, ctx->strstart)) {
                FLUSH_BLOCK(ctx, ztcontext, zbcontext, 0);
				ctx->block_start = ctx->strstart;
            }
			ctx->strstart++;
			ctx->lookahead--;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
			match_available = 1;
			ctx->strstart++;
			ctx->lookahead--;
		}

		/* Make sure that we always have enough lookahead, except
		 * at the end of the input file. We need MAX_MATCH bytes
		 * for the next match, plus MIN_MATCH bytes to insert the
		 * string following the next match.
		 */
		if (ctx->lookahead < MIN_LOOKAHEAD && !ctx->eofile) {
			ctx->s_match_available = match_available;
			ctx->s_match_length = match_length;
			return 0;
			/* fill_window();*/
		}
    }
    if (match_available) ct_tally (ztcontext, 0, ctx->window[ctx->strstart-1],
								   ctx->block_start, ctx->strstart);

    return FLUSH_BLOCK(ctx, ztcontext, zbcontext, 1); /* eof */
}


/* Zip a buffer of input */
void
zip_input(ZDeflateContext *ctx, struct ZTreesContext *ztcontext,
	struct ZBitsContext *zbcontext, char const *buf, unsigned len)
{
    unsigned more;

    while (len) {
		more = (unsigned)((PGPUInt32)2*WSIZE - ctx->lookahead - ctx->strstart);

		if (more <= 1) {
			slide_window(ctx);
			more += WSIZE;
		}
		if (more > len)
			more = len;
		memcpy(ctx->window+ctx->strstart+ctx->lookahead, buf, more);
		ctx->lookahead += more;

		if (ctx->lookahead >= MIN_LOOKAHEAD) {
			if (!ctx->strstart) {	/* First time */
				register int j;
				ctx->ins_h = 0;
				for (j=0; j<MIN_MATCH-1; j++)
					UPDATE_HASH(ctx->ins_h, ctx->window[j]);
			}
			deflate_sub(ctx, ztcontext, zbcontext);
        }
		buf += more;
		len -= more;
    }

    return;
}

/* Zip EOF */
void
zip_finish(ZDeflateContext *ctx, struct ZTreesContext *ztcontext,
	struct ZBitsContext *zbcontext)
{
    ctx->eofile = 1;

    if (!ctx->strstart) {	/* First time */
		register int j;
		ctx->ins_h = 0;
		for (j=0; j<MIN_MATCH-1; j++)
			UPDATE_HASH(ctx->ins_h, ctx->window[j]);
    }

    deflate_sub(ctx, ztcontext, zbcontext);

    return;
}
