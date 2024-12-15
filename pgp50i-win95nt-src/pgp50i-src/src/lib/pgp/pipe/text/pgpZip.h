/*
* pgpZip.h
*
* Copyright (C) 1990,1991 Mark Adler, Richard B. Wales, and Jean-loup
* Gailly. Permission is granted to any individual or institution to use,
* copy, or redistribute this software so long as all of the original files
* are included unmodified, that it is not sold for profit, and that this
* copyright notice is retained.
*
* $Id: pgpZip.h,v 1.2.2.1 1997/06/07 09:51:12 mhw Exp $
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
#include "pgpMem.h"
#include "pgpZTailor.h"

/* PGP defines */
#define PGP	 1
#ifdef MSDOS
#define SMALL_MEM 1
#endif
/* DYN_ALLOC is defined in pgpZTailor.h */

#define MIN_MATCH 3
#define MAX_MATCH 258
/* The minimum and maximum match lengths */

#define WSIZE	8192
/* The Window size must be 8k to be compatible with PGP 2.6.2 */

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
* See deflate.c for comments about the MIN_MATCH+1.
*/

#define MAX_DIST (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
* distances are limited to MAX_DIST instead of WSIZE.
*/

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Diagnostic functions */
#if ZIPDEBUG
extern int verbose;	 /* Verbose reporting flag */
# ifdef MSDOS
# undef stderr
# define stderr stdout
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
int lm_init(int pack_level);
void lm_free(void);

	/* in pgpZTrees.c */
int ct_init(void);
int ct_tally(unsigned dist, unsigned lc);
void ct_free(void);
word32 flush_block(char const *buf, word32 stored_len, int eof);

	/* in pgpZBits.c */
struct PgpFifoContext;
void bi_init(struct PgpFifoContext *zipfifo);
void send_bits(int value, int length);
void bi_flush(void);
void bi_windup(void);
void copy_block(char const far *buf, unsigned len, int header);

	/* in pgpZDeflate.c */
void zip_input(char const *buf, unsigned len);
void zip_finish(void);

#ifdef __cplusplus
}
#endif

/* end of pgpZip.h */
