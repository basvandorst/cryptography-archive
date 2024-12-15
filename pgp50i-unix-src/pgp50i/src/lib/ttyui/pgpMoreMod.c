/*
 * pgpMoreMod.c -- A pipeline module to output text a page at a time.
 *
 * Copyright (C) 1996,1997 Pretty Good Privacy, Inc. All rights reserved.
 *
 * $Id: pgpMoreMod.c,v 1.3.2.1.2.1 1997/08/18 22:37:56 quark Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>

#include "pgpDebug.h"
#include "pgpAnnotate.h"
#include "pgpMem.h"
#include "pgpPipeline.h"
#include "pgpKB.h"
#include "pgpScreen.h"
#include "pgpMoreMod.h"

#define MOREMAGIC	0xa5f91780

#define TABSTOP		8

struct MoreModContext {
	int nrows;		/* Number of rows on screen, minus 1 */
	int row;		/* Next row of output, 0 based */
	int ncols;		/* Number of columns on screen */
	int col;		/* Next column of output, 0 based */
	long nbytes; 		/* Size of message, if given */
	long byt; 		/* Number of bytes we have output */
	char *rowbuf; 		/* Buffer holding row to be output */
	byte flushing; 		/* True if discarding all input */
	byte aftercr; 		/* Saw a cr, eat next lf if any */
	byte quitting; 		/* Have printed quitting message */
	int scope_depth;	/* Nesting scope depth */
	FILE *fp; 		/* Output file descriptor, must be console */
};

static void
moreflushrow(struct MoreModContext *context)
{
	if (context->col) {
		fputs (context->rowbuf, context->fp);
		fputc('\n', context->fp);
		context->col = 0;
	}
}

static void
moreeraseline(struct MoreModContext *context)
{
	int i;
	int ncols = context->ncols;
	FILE *fp = context->fp;

	/* Blank out prompt */
	for (i = 0; i < ncols; i++)
		fputc (' ', fp);
	fputc('\r', fp);
	fflush(fp);
}

static void
morenewline(struct MoreModContext *context)
{
	int c;

	moreflushrow(context);
	context->row += 1;
	if (context->row == context->nrows) {
		if (context->nbytes) {
		fprintf (context->fp, "\
More -- %d%% -- Hit space for next screen, Enter for new line\
, 'Q' to quit --\r", (int)((100 * context->byt) / context->nbytes));
		} else {
			fputs ("\
More -- Hit space for next screen, Enter for new line, 'Q' to quit --\r",
				context->fp);
		}
		fflush (context->fp);
		for ( ; ; ) {
		        kbCbreak(FALSE);
			c = kbGet();
			kbNorm();
			moreeraseline(context);
			c = toupper(c);
			if (c == 'Q') {
				context->flushing = 1;
				break;
			} else if (c == '\r' || c == '\n') {
				 context->row -= 1;
				 break;
			} else if (c == ' ') {
				 context->row = 0;
				 break;
			} else {
				fprintf (context->fp, "\b");
			}
		}
	}
}

static void
moreoutchar(struct MoreModContext *context, char c)
{
	int aftercr = context->aftercr;
	context->aftercr = 0;

	if (c=='\n') {
		 if (aftercr)
		 	return;
		 morenewline(context);
	} else if (c=='\r') {
		context->aftercr = 1;
		morenewline(context);
	} else if (c=='\t') {
		int addspaces = TABSTOP - (context->col % TABSTOP);
		while (addspaces--) {
			if (context->col == context->ncols) {
				context->rowbuf[context->col] = '\0';
				return;
			}
			context->rowbuf[context->col++] = ' ';
		}
		context->rowbuf[context->col] = '\0';
	} else if (c < ' ') {
		 return; /* Eat non-printing characters */
	} else {
		/* Eat characters beyond right column */
		if (context->col == context->ncols)
			return;
		context->rowbuf[context->col++] = c;
		context->rowbuf[context->col] = '\0';
	}
}

static size_t
Write (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
	struct MoreModContext *context;
	int sz = size;
	int c;

	pgpAssert(myself);
	pgpAssert(myself->magic == MOREMAGIC);

	context = (struct MoreModContext *)myself->priv;
	pgpAssert (context);

	pgpAssert(error);
	*error = 0;

	while (!context->flushing && sz--) {
		c = *(char *)buf++;
		moreoutchar(context, (char)c);
		 ++context->byt;
	}

	return size;
}

static int
Flush (struct PgpPipeline *myself)
{
	pgpAssert(myself);
	pgpAssert(myself->magic == MOREMAGIC);

	/* To shut up compiler warnings */
	(void)myself;

	return 0;
}

static int
Annotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
	  byte const *string, size_t size)
{
	struct MoreModContext *context;

	/* To shut up compiler warnings */
	(void)origin;
	(void)string;
	(void)size;

	pgpAssert(myself);
	pgpAssert(myself->magic == MOREMAGIC);

	context = (struct MoreModContext *)myself->priv;
	pgpAssert (context);

	/* Currently the annotation reader does not pass us any annotations */
	PGP_SCOPE_DEPTH_UPDATE(context->scope_depth, type);
	pgpAssert (context->scope_depth != -1);

	return 0;
}

static int
SizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
{
	struct MoreModContext *context;

	pgpAssert (myself);
	pgpAssert (myself->magic == MOREMAGIC);

	context = (struct MoreModContext *)myself->priv;
	pgpAssert (context);

	if (bytes)
		context->nbytes = bytes;

	if (bytes || context->scope_depth)
		 return 0;

	/* XXX
	 * End of "more" output.  Note that scope_depth is not correct
	 * currently because annotation reader doesn't tell us scope.
	 * Not clear if this is a bug...
	 */
	if (!context->quitting) {
		fputs ("\nDone...hit any key\n", context->fp);
		kbCbreak(FALSE);
		kbGet();
		kbNorm();
		fputs ("\n", context->fp);
		context->quitting = 1;
	}

	return 0;
}

static void
Teardown (struct PgpPipeline *myself)
{
	struct MoreModContext *context;

	pgpAssert(myself);
	pgpAssert(myself->magic == MOREMAGIC);

	context = (struct MoreModContext *)myself->priv;
	pgpAssert (context);

	memset (context->rowbuf, 0, context->ncols + 1);
	pgpMemFree (context->rowbuf);
	memset (context, 0, sizeof (*context));
	pgpMemFree (context);
	memset (myself, 0, sizeof (*myself));
	pgpMemFree (myself);
}

struct PgpPipeline *
pgpMoreModCreate (struct PgpPipeline **head)
{
	struct PgpPipeline *mod;
	struct MoreModContext *context;
	unsigned nrows, ncols;

	if (!head)
		return NULL;

	context = (struct MoreModContext *)pgpMemAlloc (sizeof(*context));
	if (!context)
		return NULL;
	memset(context, 0, sizeof(*context));

	mod = (struct PgpPipeline *)pgpMemAlloc (sizeof(*mod));
	if (!mod) {
		pgpMemFree(context);
		return NULL;
	}

	screenSizeGet (&nrows, &ncols);

	context->rowbuf = (char *)pgpMemAlloc (ncols + 1);
	if (!context->rowbuf) {
		pgpMemFree(context);
		pgpMemFree(mod);
		return NULL;
	}

	context->nrows = nrows - 1;
	context->ncols = ncols;
	context->fp = stdout;

	mod->magic = MOREMAGIC;
	mod->write = Write;
	mod->flush = Flush;
	mod->sizeAdvise = SizeAdvise;
	mod->annotate = Annotate;
	mod->teardown = Teardown;
	mod->name = "More";
	mod->priv = context;

	*head = mod;
	return *head;
}
