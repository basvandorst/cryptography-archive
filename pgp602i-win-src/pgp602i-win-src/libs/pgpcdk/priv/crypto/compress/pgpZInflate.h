/*
 * pgpZInflate.h - inflate a zip-compressed stream of bytes.
 *
 * This is a PRIVATE header file, for use only within the PGP Library.
 * You should not be using these functions in an application.
 *
 * $Id: pgpZInflate.h,v 1.5 1997/06/28 00:21:32 lloyd Exp $
 */

#include <stddef.h>	/* For size_t */

#include "pgpContext.h"


PGP_BEGIN_C_DECLARATIONS

/* Private context structure */
struct InflateContext;
#ifndef TYPE_InflateContext
#define TYPE_InflateContext 1
typedef struct InflateContext InflateContext;
#endif

InflateContext *infAlloc(PGPContextRef	context);
void infFree(InflateContext *ctx);

/* Write bytes.  Returns bytes written.  error < 0 if trouble */
size_t infWrite(InflateContext *ctx, unsigned char const *buf, 
	size_t len, PGPError *error);
/* Get pointer & len of available output bytes */
unsigned char const *infGetBytes(InflateContext *ctx, unsigned *len);
/* Tell InflateContext that you've read "len" of available bytes */
void infSkipBytes(InflateContext *ctx, unsigned len);
/* Returns 0 for okay, or <0 for error */
int infEOF(InflateContext *ctx);

PGP_END_C_DECLARATIONS

/* Error codes returned. */
#define INFERR_NONE	0
#define INFERR_NOMEM	-1	/* Unable to allocate as needed */
#define INFERR_BADINPUT	-2	/* Corrupt input */
#define INFERR_SHORT	-3	/* Unexpected EOF */
#define INFERR_LONG	-4	/* Padding after expected EOF */
