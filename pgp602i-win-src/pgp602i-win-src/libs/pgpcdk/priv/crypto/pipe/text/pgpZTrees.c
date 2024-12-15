/*____________________________________________________________________________
	Copyright (C) 1997 Network Associates, Inc. and its affiliates.
	All rights reserved.
	
	

	$Id: pgpZTrees.c,v 1.13.12.1 1998/11/12 03:21:26 heller Exp $
____________________________________________________________________________*/

/*
 * Copyright (C) 1990-1993 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 * Kai Uwe Rommel and Igor Mandrichenko.
 * Permission is granted to any individual or institution to use, copy,
 * or redistribute this software so long as all of the original files
 * are included, that it is not sold for profit, and that this copyright
 * notice is retained.
 *
 * $Id: pgpZTrees.c,v 1.13.12.1 1998/11/12 03:21:26 heller Exp $
 */
 

/*
 *  trees.c by Jean-loup Gailly
 *
 *  This is a new version of im_ctree.c originally written by Richard B. Wales
 *  for the defunct implosion method.
 *
 *  PURPOSE
 *
 *      Encode various sets of source values using variable-length
 *      binary code trees.
 *
 *  DISCUSSION
 *
 *      The PKZIP "deflation" process uses several Huffman trees. The more
 *      common source values are represented by shorter bit sequences.
 *
 *      Each code tree is stored in the ZIP file in a compressed form
 *      which is itself a Huffman encoding of the lengths of
 *      all the code strings (in ascending order by source values).
 *      The actual code strings are reconstructed from the lengths in
 *      the UNZIP process, as described in the "application note"
 *      (APPNOTE.TXT) distributed as part of PKWARE's PKZIP program.
 *
 *  REFERENCES
 *
 *      Lynch, Thomas J.
 *          Data Compression:  Techniques and Applications, pp. 53-55.
 *          Lifetime Learning Publications, 1985.  ISBN 0-534-03418-7.
 *
 *      Storer, James A.
 *          Data Compression:  Methods and Theory, pp. 49-50.
 *          Computer Science Press, 1988.  ISBN 0-7167-8156-5.
 *
 *      Sedgewick, R.
 *          Algorithms, p290.
 *          Addison-Wesley, 1983. ISBN 0-201-06672-6.
 *
 *  INTERFACE
 *
 *      void ct_init (void))
 *          Allocate the match buffer and initialize the various tables.
 *
 *      void ct_tally (unsigned dist, unsigned lc);
 *          Save the match info and tally the frequency counts.
 *
 *      long flush_block (char const *buf, PGPUInt32 stored_len, int eof)
 *          Determine the best encoding for the current block: dynamic trees,
 *          static trees or store, and output the encoded block to the zip
 *          file. Returns the total compressed length for the file so far.
 *
 */

#include "pgpConfig.h"

#include <ctype.h>
#include "pgpZip.h"

/* ===========================================================================
 * Constants
 */

#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#define MAX_BL_BITS 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#define LENGTH_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#define LITERALS  256
/* number of literal bytes 0..255 */

#define END_BLOCK 256
/* end of block literal code */

#define L_CODES (LITERALS+1+LENGTH_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#define D_CODES   30
/* number of distance codes */

#define BL_CODES  19
/* number of codes used to transfer the bit lengths */


/* extra bits for each length code */
static int const near extra_lbits[LENGTH_CODES]
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

/* extra bits for each distance code */
static int const near extra_dbits[D_CODES]
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

/* extra bits for each bit length code */
static int const near extra_blbits[BL_CODES]
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

static PGPByte const near bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#ifndef LIT_BUFSIZE
#  ifdef SMALL_MEM
#    define LIT_BUFSIZE  0x2000
#  else
#    ifdef MEDIUM_MEM
#      define LIT_BUFSIZE  0x4000
#    else
#      define LIT_BUFSIZE  0x8000
#    endif
#  endif
#endif
#define DIST_BUFSIZE  LIT_BUFSIZE
/* Sizes of match buffers for literals/lengths and distances.  There are
 * 4 reasons for limiting LIT_BUFSIZE to < 64K:
 *   - frequencies can be kept in 16 bit counters
 *   - if compression is not successful for the first block, all input data is
 *     still in the window so we can still emit a stored block even when input
 *     comes from standard input.  (This can also be done for all blocks if
 *     LIT_BUFSIZE is not greater than 32K.)
 *   - if compression is not successful for a file smaller than 64K, we can
 *     even emit a stored file instead of a stored block (saving 5 bytes).
 *   - creating new Huffman trees less frequently may not provide fast
 *     adaptation to changes in the input data statistics. (Take for
 *     example a binary file with poorly compressible code followed by
 *     a highly compressible string table.) Smaller buffer sizes give
 *     fast adaptation but have of course the overhead of transmitting trees
 *     more frequently.
 *   - I can't count above 4
 * The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
 * memory at the expense of compression). Some optimizations would be possible
 * if we rely on DIST_BUFSIZE == LIT_BUFSIZE.
 */

#define REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#define REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#define REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

/* ===========================================================================
 * Local data
 */

/* Data structure describing a single value and its code string. */
typedef struct ct_data
{
    union {
        PGPUInt16 freq;       /* frequency count */
	PGPUInt16 code;       /* bit string */
    } fc;
    union {
        PGPUInt16 dad;        /* father node in Huffman tree */
        PGPUInt16 len;        /* length of bit string */
    } dl;
} ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

#define HEAP_SIZE (2*L_CODES+1)
/* maximum heap size */

typedef struct tree_desc {
    ct_data near *dyn_tree;    /* the dynamic tree */
    ct_data near *static_tree; /* corresponding static tree or NULL */
    int      const near *extra_bits;  /* extra bits for each code or NULL */
    int                  extra_base;  /* base index for extra_bits */
    int                  elems;       /* max number of elements in the tree */
    int                  max_length;  /* max bit length for the codes */
    int                  max_code;    /* largest code with non zero frequency*/
} tree_desc;

/* Tables for initializing context tables.  The local pointers are set up in
 * ct_init.
 */

static const tree_desc near l_desc_init =
{/*dyn_ltree*/0, /*static_ltree*/0, extra_lbits, LITERALS+1, L_CODES,
	MAX_BITS, 0};

static const tree_desc near d_desc_init =
{/*dyn_dtree*/0, /*fstatic_dtree*/0, extra_dbits, 0, D_CODES, MAX_BITS, 0};

static const tree_desc near bl_desc_init =
{/*bl_tree*/0, NULL,   extra_blbits, 0,         BL_CODES, MAX_BL_BITS, 0};

typedef struct ZTreesContext {

	PGPContextRef		cdkContext;

	ct_data near dyn_ltree[HEAP_SIZE];   /* literal and length tree*/
	ct_data near dyn_dtree[2*D_CODES+1]; /* distance tree */

	ct_data near static_ltree[L_CODES+2];
	/* The static literal tree. Since the bit lengths are imposed, there is no
	 * need for the L_CODES extra codes used during heap construction. However
	 * The codes 286 and 287 are needed to build a canonical tree (see ct_init
	 * below).
	 */

	ct_data near static_dtree[D_CODES];
	/* The static distance tree. (Actually a trivial tree since all codes use
	 * 5 bits.)
	 */

	ct_data near bl_tree[2*BL_CODES+1];
	/* Huffman tree for the bit lengths */


	tree_desc near l_desc;
	tree_desc near d_desc;
	tree_desc near bl_desc;

	PGPUInt16 near bl_count[MAX_BITS+1];
	/* number of codes at each bit length for an optimal tree */

	int near heap[2*L_CODES+1]; /*heap used to build the Huffman trees*/
	int heap_len;               /* number of elements in the heap */
	int heap_max;               /* element of largest frequency */
	/* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
	 * The same heap array is used to build all trees.
	 */

	PGPByte near depth[2*L_CODES+1];
	/* Depth of each subtree used as tie breaker for trees of equal frequency*/

	PGPByte length_code[MAX_MATCH-MIN_MATCH+1];
	/* length code for each normalized match length (0 == MIN_MATCH) */

	PGPByte dist_code[512];
	/* distance codes. The first 256 values correspond to the distances
	 * 3 .. 258, the last 256 values correspond to the top 8 bits of
	 * the 15 bit distances.
	 */

	int near base_length[LENGTH_CODES];
	/* First normalized length for each code (0 = MIN_MATCH) */

	int near base_dist[D_CODES];
	/* First normalized distance for each code (0 = distance of 1) */

#ifndef DYN_ALLOC
	  PGPByte far l_buf[LIT_BUFSIZE];  /* buffer for literals/lengths */
	  PGPUInt16 far d_buf[DIST_BUFSIZE]; /* buffer for distances */
#else
	  PGPByte far *l_buf;
	  PGPUInt16 far *d_buf;
#endif

	PGPByte near flag_buf[(LIT_BUFSIZE/8)];
	/* flag_buf is a bit array distinguishing literals from lengths in
	 * l_buf, and thus indicating the presence or absence of a distance.
	 */

	unsigned last_lit;    /* running index in l_buf */
	unsigned last_dist;   /* running index in d_buf */
	unsigned last_flags;  /* running index in flag_buf */
	PGPByte flags;        /* current flags not yet saved in flag_buf */
	PGPByte flag_bit;     /* current bit used in flags */
	/* bits are filled in flags starting at bit 0 (least significant).
	 * Note: these flags are overkill in the current code since we don't
	 * take advantage of DIST_BUFSIZE == LIT_BUFSIZE.
	 */

	/* bit length of current block with optimal trees*/
	PGPUInt32 opt_len;
	/* bit length of current block with static trees */
	PGPUInt32 static_len;

	PGPUInt32 compressed_len; /* total bit length of compressed file */

	PGPUInt32 input_len;      /* total byte length of input file */
	/* input_len is for debugging only since we can get it by other means. */
} ZTreesContext;

#ifdef ZIPDEBUG
extern PGPUInt32 bits_sent;  /* bit length of the compressed data */
extern PGPUInt32 isize;      /* byte length of input file */
#endif

/* ===========================================================================
 * Local (static) routines in this file.
 */

static void init_block(ZTreesContext *);
static void pqdownheap(ZTreesContext *, ct_data near *tree, int k);
static void gen_bitlen(ZTreesContext *, tree_desc near *desc);
static void gen_codes(ZTreesContext *, ct_data near *tree, int max_code);
static void build_tree(ZTreesContext *, tree_desc near *desc);
static void scan_tree(ZTreesContext *, ct_data near *tree, int max_code);
static void send_tree(ZTreesContext *, struct ZBitsContext *,
	ct_data near *tree, int max_code);
static int  build_bl_tree(ZTreesContext *);
static void send_all_trees(ZTreesContext *, struct ZBitsContext *,
	int lcodes, int dcodes, int blcodes);
static void compress_block(ZTreesContext *, struct ZBitsContext *,
	ct_data near *ltree, ct_data near *dtree);


#ifndef ZIPDEBUG
#  define send_code(zbctx, c, tree)	send_bits(zbctx, tree[c].Code, tree[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#else /* ZIPDEBUG */
#  define send_code(zbctx, c, tree) \
     { if (verbose > 1) fprintf(stderr,"\ncd %3d ",(c)); \
       send_bits(zbctx, tree[c].Code, tree[c].Len); }
#endif

#define d_code(ctx, dist) \
   ((dist) < 256 ? (ctx)->dist_code[dist] : (ctx)->dist_code[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. dist_code[256] and dist_code[257] are never
 * used.
 */

#define MAX(a,b) (a >= b ? a : b)
/* the arguments must not have side effects */

/* ===========================================================================
 * Reverse k low-order bits in a word.  Not speed-critical.
 */
static unsigned
bit_reverse(unsigned value, int len)
{
	unsigned result = 0;

	do {
		result <<= 1;
		result |= value & 1;
		value >>= 1;
	} while (--len);
	return result;
}

/* ===========================================================================
 * Allocate the match buffer and initialize the various tables.
 */
ZTreesContext *
ct_init(PGPContextRef cdkContext)
{
	ZTreesContext *ctx;
    int n;        /* iterates over tree elements */
    int bits;     /* bit counter */
    int length;   /* length value */
    int code;     /* code value */
    int dist;     /* distance index */

	ctx = (ZTreesContext *) pgpContextMemAlloc (cdkContext,
								sizeof(*ctx), kPGPMemoryMgrFlags_Clear);
	if (IsNull (ctx) )
		return NULL;
	ctx->cdkContext = cdkContext;

	/* Initialize tables */
	pgpCopyMemory (&l_desc_init,  &ctx->l_desc,  sizeof(l_desc_init));
	ctx->l_desc.dyn_tree = ctx->dyn_ltree;
	ctx->l_desc.static_tree = ctx->static_ltree;

	pgpCopyMemory (&d_desc_init,  &ctx->d_desc,  sizeof(d_desc_init));
	ctx->d_desc.dyn_tree = ctx->dyn_dtree;
	ctx->d_desc.static_tree = ctx->static_dtree;

	pgpCopyMemory (&bl_desc_init, &ctx->bl_desc, sizeof(bl_desc_init));
	ctx->bl_desc.dyn_tree = ctx->bl_tree;
	ctx->bl_desc.static_tree = NULL;

    ctx->compressed_len = ctx->input_len = 0L;

#ifdef DYN_ALLOC
    /* Allocate the buffers */

    ctx->d_buf = (PGPUInt16 far *)pgpContextMemAlloc (cdkContext,
				DIST_BUFSIZE * sizeof(PGPUInt16), kPGPMemoryMgrFlags_Clear);
    if (ctx->d_buf == NULL) {
		pgpContextMemFree (cdkContext, ctx);
		return NULL;
	}
    ctx->l_buf = (PGPByte far *)pgpContextMemAlloc (cdkContext,
				(LIT_BUFSIZE/2) * 2, kPGPMemoryMgrFlags_Clear);
    /* Avoid using the value 64K on 16 bit machines */
    if (ctx->l_buf == NULL) {
		pgpContextMemFree (cdkContext, ctx->d_buf);
		pgpContextMemFree (cdkContext, ctx);
		return NULL;
    }
/*    if (ctx->l_buf == NULL || ctx->d_buf == NULL)
 *		  error("ct_init: out of memory");
 */
#endif
    /* Initialize the counts for the first block */
    init_block(ctx);

    /* The following initializes constant data - it may be skipped */
    if (ctx->static_dtree[0].Len != 0) return 0; /* ct_init already called */

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for (code = 0; code < LENGTH_CODES-1; code++) {
		ctx->base_length[code] = length;
		for (n = 0; n < (1<<extra_lbits[code]); n++) {
			ctx->length_code[length++] = (PGPByte)code;
		}
    }
    ZipAssert (length == 256, "ct_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    ctx->length_code[length-1] = (PGPByte)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for (code = 0 ; code < 16; code++) {
		ctx->base_dist[code] = dist;
		for (n = 0; n < (1<<extra_dbits[code]); n++) {
			ctx->dist_code[dist++] = (PGPByte)code;
		}
    }
    ZipAssert (dist == 256, "ct_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    for ( ; code < D_CODES; code++) {
		ctx->base_dist[code] = dist << 7;
		for (n = 0; n < (1<<(extra_dbits[code]-7)); n++) {
			ctx->dist_code[256 + dist++] = (PGPByte)code;
		}
    }
    ZipAssert (dist == 256, "ct_init: 256+dist != 512");

    /* Construct the codes of the static literal tree */
    for (bits = 0; bits <= MAX_BITS; bits++)
		ctx->bl_count[bits] = 0;
    n = 0;
    while (n <= 143) ctx->static_ltree[n++].Len = 8, ctx->bl_count[8]++;
    while (n <= 255) ctx->static_ltree[n++].Len = 9, ctx->bl_count[9]++;
    while (n <= 279) ctx->static_ltree[n++].Len = 7, ctx->bl_count[7]++;
    while (n <= 287) ctx->static_ltree[n++].Len = 8, ctx->bl_count[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes(ctx, (ct_data near *)ctx->static_ltree, L_CODES+1);

    /* The static distance tree is trivial: */
    for (n = 0; n < D_CODES; n++) {
		ctx->static_dtree[n].Len = 5;
		ctx->static_dtree[n].Code = bit_reverse(n, 5);
    }

    return ctx;
}

void ct_free(ZTreesContext *ctx)
{
	PGPContextRef cdkContext;

	cdkContext = ctx->cdkContext;

#ifdef DYN_ALLOC
    /* Wipe and free buffers */
    pgpClearMemory(ctx->d_buf, DIST_BUFSIZE * sizeof(PGPUInt16));
    pgpClearMemory(ctx->l_buf, LIT_BUFSIZE);
    pgpContextMemFree (cdkContext, ctx->d_buf);
    pgpContextMemFree (cdkContext, ctx->l_buf);
#endif

    /* Wipe everything else */
	pgpClearMemory( ctx, sizeof(*ctx) );
	pgpContextMemFree( cdkContext, ctx );
}

/* ===========================================================================
 * Initialize a new block.
 */
static void
init_block(ZTreesContext *ctx)
{
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for (n = 0; n < L_CODES;  n++) ctx->dyn_ltree[n].Freq = 0;
    for (n = 0; n < D_CODES;  n++) ctx->dyn_dtree[n].Freq = 0;
    for (n = 0; n < BL_CODES; n++) ctx->bl_tree[n].Freq = 0;

    ctx->dyn_ltree[END_BLOCK].Freq = 1;
    ctx->opt_len = ctx->static_len = 0L;
    ctx->last_lit = ctx->last_dist = ctx->last_flags = 0;
    ctx->flags = 0; ctx->flag_bit = 1;
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#define pqremove(ctx, tree, top) \
{\
    top = (ctx)->heap[SMALLEST]; \
    (ctx)->heap[SMALLEST] = (ctx)->heap[(ctx)->heap_len--]; \
    pqdownheap(ctx, tree, SMALLEST); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define smaller(ctx, tree, n, m) \
   (tree[n].Freq < tree[m].Freq || \
   (tree[n].Freq == tree[m].Freq && (ctx)->depth[n] <= (ctx)->depth[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
static void
pqdownheap(ZTreesContext *ctx, ct_data near *tree, int k)
{
    int v = ctx->heap[k];
    int j = k << 1;  /* left son of k */
    int htemp;       /* required because of bug in SASC compiler */

    while (j <= ctx->heap_len) {
        /* Set j to the smallest of the two sons: */
        if (j < ctx->heap_len && smaller(ctx, tree, ctx->heap[j+1],
										 ctx->heap[j])) j++;

        /* Exit if v is smaller than both sons */
        htemp = ctx->heap[j];
        if (smaller(ctx, tree, v, htemp)) break;

        /* Exchange v with the smallest son */
		ctx->heap[k] = htemp;
        k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    ctx->heap[k] = v;
}

/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
static void
gen_bitlen(ZTreesContext *ctx, tree_desc near *desc)
{
    ct_data near *tree  = desc->dyn_tree;
    int const near *extra      = desc->extra_bits;
    int base                   = desc->extra_base;
    int max_code               = desc->max_code;
    int max_length             = desc->max_length;
    ct_data near *stree = desc->static_tree;
    int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    int bits;           /* bit length */
    int xbits;          /* extra bits */
    PGPUInt16 f;           /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

    for (bits = 0; bits <= MAX_BITS; bits++)
        ctx->bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    tree[ctx->heap[ctx->heap_max]].Len = 0; /* root of the heap */

    for (h = ctx->heap_max+1; h < HEAP_SIZE; h++) {
        n = ctx->heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > max_length) {
            bits = max_length;
            overflow++;
		}
        tree[n].Len = (PGPUInt16)bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        if (n > max_code)
            continue; /* not a leaf node */

        ctx->bl_count[bits]++;
        xbits = 0;
        if (n >= base)
            xbits = extra[n-base];
        f = tree[n].Freq;
        ctx->opt_len += (PGPUInt32)f * (bits + xbits);
        if (stree)
            ctx->static_len += (PGPUInt32)f * (stree[n].Len + xbits);
    }
    if (overflow == 0) return;

    Trace((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do {
        bits = max_length-1;
        while (ctx->bl_count[bits] == 0)
            bits--;
        ctx->bl_count[bits]--;      /* move one leaf down the tree */
        ctx->bl_count[bits+1] += 2; /* move one overflow item as its brother */
        ctx->bl_count[max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
        n = ctx->bl_count[bits];
        while (n != 0) {
            m = ctx->heap[--h];
            if (m > max_code)
                continue;
            if (tree[m].Len != (unsigned) bits) {
                Trace((stderr,"code %d bits %d->%d\n", m, tree[m].Len, bits));
                ctx->opt_len += ((long)bits - tree[m].Len) * tree[m].Freq;
                tree[m].Len = (PGPUInt16)bits;
            }
            n--;
        }
    }
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 *    the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
static void
gen_codes(ZTreesContext *ctx, ct_data near *tree, int max_code)
{
    PGPUInt16 next_code[MAX_BITS+1]; /* next code value for each bit length */
    PGPUInt16 code = 0;              /* running code value */
    int bits;                     /* bit index */
    int n;                        /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= MAX_BITS; bits++)
        next_code[bits] = code = (code + ctx->bl_count[bits-1]) << 1;
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    ZipAssert (code + ctx->bl_count[MAX_BITS]-1 == (1<<MAX_BITS)-1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].Len;
        if (len == 0)
            continue;
        /* Now reverse the bits */
        tree[n].Code = bit_reverse(next_code[len]++, len);

		Tracec(tree != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
             n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len]-1));
    }
}

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
static void
build_tree(ZTreesContext *ctx, tree_desc near *desc)
{
    ct_data near *tree   = desc->dyn_tree;
    ct_data near *stree  = desc->static_tree;
    int elems            = desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node = elems;  /* next internal node of the tree */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
     * heap[0] is not used.
     */
    ctx->heap_len = 0, ctx->heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].Freq != 0) {
            ctx->heap[++ctx->heap_len] = max_code = n;
            ctx->depth[n] = 0;
		} else {
            tree[n].Len = 0;
		}
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (ctx->heap_len < 2) {
        n = ctx->heap[++ctx->heap_len] = (max_code < 2 ? ++max_code : 0);
        tree[n].Freq = 1;
        ctx->depth[n] = 0;
        ctx->opt_len--;
        if (stree)
            ctx->static_len -= stree[n].Len;
        /* n is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = ctx->heap_len/2; n >= 1; n--)
        pqdownheap(ctx, tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    do {
        pqremove(ctx, tree, n);   /* n = node of least frequency */
        m = ctx->heap[SMALLEST];  /* m = node of next least frequency */

        ctx->heap[--ctx->heap_max] = n; /* keep the nodes sorted by frequency*/
        ctx->heap[--ctx->heap_max] = m;

        /* Create a new node father of n and m */
		tree[node].Freq = tree[n].Freq + tree[m].Freq;
        ctx->depth[node] = (PGPByte)(MAX(ctx->depth[n], ctx->depth[m]) + 1);
        tree[n].Dad = tree[m].Dad = (PGPUInt16)node;
#ifdef DUMP_BL_TREE
        if (tree == ctx->bl_tree)
            fprintf(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
                    node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
#endif
        /* and insert the new node in the heap */
        ctx->heap[SMALLEST] = node++;
        pqdownheap(ctx, tree, SMALLEST);

    } while (ctx->heap_len >= 2);

    ctx->heap[--ctx->heap_max] = ctx->heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen(ctx, desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes (ctx, tree, max_code);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree. Updates opt_len to take into account the repeat
 * counts.  (The contribution of the bit length codes will be added later
 * during the construction of bl_tree.)
 */
static void
scan_tree(ZTreesContext *ctx, ct_data near *tree, int max_code)
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0)
        max_count = 138, min_count = 3;
    tree[max_code+1].Len = (PGPUInt16)-1; /* guard */

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            ctx->bl_tree[curlen].Freq += count;
        } else if (curlen != 0) {
            if (curlen != prevlen)
                ctx->bl_tree[curlen].Freq++;
            ctx->bl_tree[REP_3_6].Freq++;
        } else if (count <= 10) {
            ctx->bl_tree[REPZ_3_10].Freq++;
        } else {
            ctx->bl_tree[REPZ_11_138].Freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
		} else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
		} else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
static void
send_tree(ZTreesContext *ctx, struct ZBitsContext *zbcontext,
	ct_data near *tree, int max_code)
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    /* tree[max_code+1].Len = -1; */  /* guard already set */
    if (nextlen == 0)
        max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen;
        nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do {
                send_code(zbcontext, curlen, ctx->bl_tree);
            } while (--count != 0);
        } else if (curlen != 0) {
			if (curlen != prevlen) {
                send_code(zbcontext, curlen, ctx->bl_tree);
                count--;
            }
            ZipAssert(count >= 3 && count <= 6, " 3_6?");
            send_code(zbcontext, REP_3_6, ctx->bl_tree);
            send_bits(zbcontext, count-3, 2);
        } else if (count <= 10) {
            send_code(zbcontext, REPZ_3_10, ctx->bl_tree);
            send_bits(zbcontext, count-3, 3);
        } else {
            send_code(zbcontext, REPZ_11_138, ctx->bl_tree);
            send_bits(zbcontext, count-11, 7);
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
static int
build_bl_tree(ZTreesContext *ctx)
{
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree(ctx, ctx->dyn_ltree, ctx->l_desc.max_code);
    scan_tree(ctx, ctx->dyn_dtree, ctx->d_desc.max_code);

    /* Build the bit length tree: */
    build_tree(ctx, &ctx->bl_desc);
    /* opt_len now includes the length of the tree representations, except
     * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (ctx->bl_tree[bl_order[max_blindex]].Len != 0)
            break;
    }
    /* Update opt_len to include the bit length tree and counts */
    ctx->opt_len += 3*(max_blindex+1) + 5+5+4;
    Tracev((stderr, "\ndyn trees: dyn %ld, stat %ld", ctx->opt_len,
			ctx->static_len));

    return max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
static void
send_all_trees(ZTreesContext *ctx, struct ZBitsContext *zbcontext,
	int lcodes, int dcodes, int blcodes)
{
    int rank;                    /* index in bl_order */

    ZipAssert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4,
            "not enough codes");
    ZipAssert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_bits(zbcontext, lcodes-257, 5);
    /* not +255 as stated in appnote.txt 1.93a or -256 in 2.04c */
    send_bits(zbcontext, dcodes-1,   5);
    send_bits(zbcontext, blcodes-4,  4); /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
        send_bits(zbcontext, ctx->bl_tree[bl_order[rank]].Len, 3);
    }
    Tracev((stderr, "\nbl tree: sent %ld", bits_sent));

    /* send the literal tree */
    send_tree(ctx, zbcontext, (ct_data near *)ctx->dyn_ltree, lcodes-1);
    Tracev((stderr, "\nlit tree: sent %ld", bits_sent));

    /* send the distance tree */
    send_tree(ctx, zbcontext, (ct_data near *)ctx->dyn_dtree, dcodes-1);
    Tracev((stderr, "\ndist tree: sent %ld", bits_sent));
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file. This function
 * returns the total compressed length for the file so far.
 *
 * buf: the input block, or NULL if too old
 * stored_len: the length of the input block
 * eof: true if this is the last block for the file
 */
PGPUInt32
flush_block(ZTreesContext *ctx, struct ZBitsContext *zbcontext,
	char const *buf, PGPUInt32 stored_len, int eof, PGPByte *window)
{
    PGPUInt32 opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex;  /* index of last bit length code of non zero freq */

	/* Save the flags for the last 8 items */
    ctx->flag_buf[ctx->last_flags] = ctx->flags;

    /* Construct the literal and distance trees */
    build_tree(ctx, (tree_desc near *)(&ctx->l_desc));
    Tracev((stderr, "\nlit data: dyn %ld, stat %ld", ctx->opt_len,
			ctx->static_len));

    build_tree(ctx, (tree_desc near *)(&ctx->d_desc));
    Tracev((stderr, "\ndist data: dyn %ld, stat %ld", ctx->opt_len,
			ctx->static_len));
    /* At this point, opt_len and static_len are the total bit lengths of
     * the compressed block data, excluding the tree representations.
     */

    /* Build the bit length tree for the above two trees, and get the index
     * in bl_order of the last bit length code to send.
     */
    max_blindex = build_bl_tree(ctx);

    /* Determine the best encoding. Compute first the block length in bytes */
    opt_lenb = (ctx->opt_len+3+7)>>3;
    static_lenb = (ctx->static_len+3+7)>>3;
    ctx->input_len += stored_len; /* for debugging only */

    Trace((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u dist %u ",
		   opt_lenb, ctx->opt_len, static_lenb, ctx->static_len, stored_len,
		   ctx->last_lit, ctx->last_dist));

    if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

#ifndef PGP /* PGP can't handle stored files - disable this test */
    /*
     * If compression failed and this is the first and last block,
     * and if the zip file can be seeked (to rewrite the local header),
     * the whole file is transformed into a stored file:
     */
#ifdef FORCE_METHOD
    if (ctx->level == 1 && eof && ctx->compressed_len == 0L) {
		/* force stored file */
#else
    if (stored_len <= opt_lenb && eof && ctx->compressed_len == 0L &&
		seekable()) {
#endif
        /* Since LIT_BUFSIZE <= 2*WSIZE, the input data must be there: */
        if (buf == NULL) error ("block vanished");

		/* without header */
        copy_block(zbcontext, buf, (unsigned)stored_len, 0);
        ctx->compressed_len = stored_len << 3;
    } else
#endif /* PGP */

#ifdef FORCE_METHOD
	if (ctx->level == 2 && buf != (char*)NULL) { /* force stored block */
#else
	if (stored_len+4 <= opt_lenb && buf != (char*)NULL) {
		/* 4: two words for the lengths */
#endif
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        send_bits(zbcontext, (STORED_BLOCK<<1)+eof, 3);  /* send block type */
        ctx->compressed_len = (ctx->compressed_len + 3 + 7) & ~7L;
		ctx->compressed_len += (stored_len + 4) << 3;

		copy_block(zbcontext, buf, (unsigned)stored_len, 1); /* with header */

#ifdef FORCE_METHOD
    } else if (ctx->level == 3) { /* force static trees */
#else
    } else if (static_lenb == opt_lenb) {
#endif
        send_bits(zbcontext, (STATIC_TREES<<1)+eof, 3);
        compress_block(ctx, zbcontext,
					   (ct_data near *)ctx->static_ltree,
					   (ct_data near *)ctx->static_dtree);
        ctx->compressed_len += 3 + ctx->static_len;
    } else {
        send_bits(zbcontext, (DYN_TREES<<1)+eof, 3);
        send_all_trees(ctx, zbcontext, ctx->l_desc.max_code+1,
					   ctx->d_desc.max_code+1, max_blindex+1);
        compress_block(ctx, zbcontext,
					   (ct_data near *)ctx->dyn_ltree,
					   (ct_data near *)ctx->dyn_dtree);
        ctx->compressed_len += 3 + ctx->opt_len;
    }
    ZipAssert (ctx->compressed_len == bits_sent, "bad compressed size");
    init_block(ctx);

    if (eof) {
#if defined(PGP) && !defined(MMAP)
        /* Wipe out sensitive data for pgp */
        pgpClearMemory( window,
        	(unsigned)(2*WSIZE-1)); /* -1 needed if WSIZE=32K */
#else /* !PGP */
        ZipAssert (ctx->input_len == ctx->isize, "bad input size");
#endif
		bi_windup(zbcontext);
        ctx->compressed_len += 7;  /* align on byte boundary */
    }
    Tracev((stderr,"\ncomprlen %lu(%lu) ", ctx->compressed_len>>3,
			ctx->compressed_len-7*eof));

    return ctx->compressed_len >> 3;
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 *
 * dist is the distance to the matched string, or 0 if none.
 * lc is the length of the match, or the character if dist == 0.
 */
int
ct_tally(ZTreesContext *ctx, unsigned dist, unsigned lc, long block_start,
	unsigned strstart)
{
    ctx->l_buf[ctx->last_lit++] = (PGPByte)lc;
    if (dist == 0) {
        /* lc is the unmatched char */
        ctx->dyn_ltree[lc].Freq++;
    } else {
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        ZipAssert((PGPUInt16)dist < (PGPUInt16)MAX_DIST &&
               (PGPUInt16)lc <= (PGPUInt16)(MAX_MATCH-MIN_MATCH) &&
               (PGPUInt16)d_code(ctx, dist) < (PGPUInt16)D_CODES, 
               "ct_tally: bad match");

        ctx->dyn_ltree[ctx->length_code[lc]+LITERALS+1].Freq++;
        ctx->dyn_dtree[d_code(ctx, dist)].Freq++;

        ctx->d_buf[ctx->last_dist++] = (PGPUInt16)dist;
        ctx->flags |= ctx->flag_bit;
    }
    ctx->flag_bit <<= 1;

    /* Output the flags if they fill a byte: */
    if ((ctx->last_lit & 7) == 0) {
        ctx->flag_buf[ctx->last_flags++] = ctx->flags;
        ctx->flags = 0, ctx->flag_bit = 1;
    }
    /* Try to guess if it is profitable to stop the current block here */
    if (/* ctx->level > 2 && */ (ctx->last_lit & 0xfff) == 0) {
        /* Compute an upper bound for the compressed length */
        PGPUInt32 out_length = (PGPUInt32)ctx->last_lit*8L;
        PGPUInt32 in_length = (PGPUInt32)strstart-block_start;
        int dcode;

        for (dcode = 0; dcode < D_CODES; dcode++)
            out_length += (PGPUInt32)
				ctx->dyn_dtree[dcode].Freq*(5L+extra_dbits[dcode]);
        out_length >>= 3;
        Trace((stderr,"\nlast_lit %u, last_dist %u, in %ld, out ~%ld(%ld%%) ",
               ctx->last_lit, ctx->last_dist, in_length, out_length,
               100L - out_length*100L/in_length));
        if (ctx->last_dist < ctx->last_lit/2 && out_length < in_length/2)
			return 1;
    }
    return (ctx->last_lit == LIT_BUFSIZE-1 || ctx->last_dist == DIST_BUFSIZE);
    /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
static void
compress_block(ZTreesContext *ctx, struct ZBitsContext *zbcontext,
	ct_data near *ltree, ct_data near *dtree)
{
    unsigned dist;      /* distance of matched string */
    int lc;             /* match length or unmatched char (if dist == 0) */
    unsigned lx = 0;    /* running index in l_buf */
    unsigned dx = 0;    /* running index in d_buf */
    unsigned fx = 0;    /* running index in flag_buf */
    PGPByte flag = 0;       /* current flags */
    unsigned code;      /* the code to send */
    int extra;          /* number of extra bits to send */

    if (ctx->last_lit != 0) do {
        if ((lx & 7) == 0)
			flag = ctx->flag_buf[fx++];
        lc = ctx->l_buf[lx++];
        if ((flag & 1) == 0) {
            send_code(zbcontext, lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } else {
            /* Here, lc is the match length - MIN_MATCH */
            code = ctx->length_code[lc];
			/* send the length code */
            send_code(zbcontext, code+LITERALS+1, ltree);
            extra = extra_lbits[code];
            if (extra != 0) {
                lc -= ctx->base_length[code];
				/* send the extra length bits */
                send_bits(zbcontext, lc, extra);
            }
            dist = ctx->d_buf[dx++];
            /* Here, dist is the match distance - 1 */
            code = d_code(ctx, dist);
            ZipAssert (code < D_CODES, "bad d_code");

			send_code(zbcontext, code, dtree);    /* send the distance code */
			extra = extra_dbits[code];
			if (extra != 0) {
				dist -= ctx->base_dist[code];
				/* send the extra distance bits */
				send_bits(zbcontext, dist, extra);
			}
		} /* literal or match pair ? */
		flag >>= 1;
    } while (lx < ctx->last_lit);

    send_code(zbcontext, END_BLOCK, ltree);
}
