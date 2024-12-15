/*
 * Copyright (C) 1990,1991 Mark Adler, Richard B. Wales, and Jean-loup
 * Gailly.  Permission is granted to any individual or institution to use,
 * copy, or redistribute this software so long as all of the original files
 * are included unmodified, that it is not sold for profit, and that this
 * copyright notice is retained.
 *
 * $Id: pgpZip.h,v 1.9 1997/11/06 20:00:23 lloyd Exp $
 */

/*
 * pgpZip.h by Mark Adler.
 */

/*
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 */

/* Set up portability */
#include "pgpUsuals.h"
#include "pgpContext.h"
#include "pgpMem.h"
#include "pgpZTailor.h"
#include "pgpOpaqueStructs.h"

/* Local data structures are defined in each module */
struct ZDeflateContext;
struct ZBitsContext;
struct ZTreesContext;

/* PGP defines */
#define PGP	1
#ifdef MSDOS
#define SMALL_MEM 1
#endif
/* DYN_ALLOC is defined in pgpZTailor.h */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define WSIZE	8192
/* The Window size must be 8k to be compatible with PGP 2.6.2 */

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#include <string.h>

PGP_BEGIN_C_DECLARATIONS

/* Diagnostic functions */
#if ZIPDEBUG
  extern int verbose;	/* Verbose reporting flag */
# ifdef MSDOS
#  undef  stderr
#  define stderr stdout
# endif
# define ZipAssert(cond,msg) {if(!(cond)) error(msg);}
# define Trace(x) fprintf x
# define Tracev(x) {if (verbose) fprintf x ;}
# define Tracevv(x) {if (verbose>1) fprintf x ;}
# define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
# define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
# define ZipAssert(cond,msg)
# define Trace(x)
# define Tracev(x)
# define Tracevv(x)
# define Tracec(c,x)
# define Tracecv(c,x)
#endif


/* Public function prototypes */

        /* in pgpZDeflate.c */
struct ZDeflateContext * lm_init(PGPContextRef cdkContext, int pack_level );
void lm_free(struct ZDeflateContext *zdcontext);

	/* in pgpZTrees.c */
struct ZTreesContext * ct_init(PGPContextRef cdkContext);
int ct_tally(struct ZTreesContext *ztcontext, unsigned dist, unsigned lc,
	long block_start, unsigned strstart);
void ct_free(struct ZTreesContext *ztcontext);
PGPUInt32 flush_block(struct ZTreesContext *ztcontext,
	struct ZBitsContext *zbcontext, char const *buf, PGPUInt32 stored_len,
	int eof, PGPByte *window);

	/* in pgpZBits.c */
struct ZBitsContext * bi_init(PGPContextRef cdkContext,
	PGPFifoContext *zipfifo);
void bi_free(struct ZBitsContext *zbcontext);
void send_bits(struct ZBitsContext *zbcontext, int value, int length);
void bi_flush(struct ZBitsContext *zbcontext);
void bi_windup(struct ZBitsContext *zbcontext);
void copy_block(struct ZBitsContext *zbcontext,
	char const far *buf, unsigned len, int header);

	/* in pgpZDeflate.c */
void zip_input(struct ZDeflateContext *zdcontext,
	struct ZTreesContext *ztcontext, struct ZBitsContext *zbcontext,
	char const *buf, unsigned len);
void zip_finish(struct ZDeflateContext *zdcontext,
	struct ZTreesContext *ztcontext, struct ZBitsContext *zbcontext);

PGP_END_C_DECLARATIONS

/* end of pgpZip.h */