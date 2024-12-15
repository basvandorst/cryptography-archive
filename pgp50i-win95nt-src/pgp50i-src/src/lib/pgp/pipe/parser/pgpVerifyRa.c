/*
* pgpVerifyRa.c -- An Annotation Reader to do Signature Verification
*
* Copyright (C) 1995-1997 Pretty Good Privacy, Inc. All rights reserved.
*
* Written by: Derek Atkins <warlord@MIT.EDU>
*
* $Id: pgpVerifyRa.c,v 1.6.2.1 1997/06/07 09:51:01 mhw Exp $
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include "pgpDebug.h"
#include "pgpVerifyRa.h"
#include "pgpVrfySig.h"
#include "pgpAnnotate.h"
#include "pgpDevNull.h"
#include "pgpHash.h"
#include "pgpJoin.h"
#include "pgpMem.h"
#include "pgpEnv.h"
#include "pgpErr.h"
#include "pgpMsg.h"
#include "pgpUI.h"
#include "pgpPipeline.h"
#include "pgpSig.h"

#define VERIFYRAMAGICS 0x519c43c7
#define VERIFYRAMAGICT 0x519c43c8

struct Context {
		struct PgpPipeline *joinhead, *jointext, **jointail;
		struct PgpHashContext *hashes;
		struct PgpSig *siglist;
		struct PgpEnv const *env;
		struct PgpUICb const *ui;
		void *ui_arg;
		int numhashes;
		int scope_depth;
		int ignore_depth;
		byte ignore_text;
		byte textclosed;
		byte sigclosed;
		byte textdone;
		byte sigdone;
	};

static int
verifySizeAdvise (struct Context *ctx)
{
 int error = 0;

 /* when both text and sig are done, verify the signature */
 if (ctx->textclosed && ctx->sigclosed && ctx->siglist) {
  error = pgpSigVerify (ctx->siglist, ctx->hashes,
			  ctx->numhashes, ctx->ui, ctx->ui_arg);

			if (!error) {
				if (ctx->siglist) {
				 pgpSigFreeList (ctx->siglist);
				 ctx->siglist = 0;
				}
				if (ctx->hashes) {
					pgpHashListDestroy (ctx->hashes,
					 	ctx->numhashes);
					ctx->hashes = NULL;
					ctx->numhashes = 0;
				}
			}
		}

		return error;
}

static void
verifyTeardown (struct PgpPipeline *myself)
	{
		struct Context *ctx = (struct Context *)myself->priv;

pgpAssert (ctx->joinhead);
pgpAssert (ctx->jointext);

if (!ctx->textdone || !ctx->sigdone)
 return;

ctx->joinhead->teardown (ctx->joinhead);
ctx->jointext->teardown (ctx->jointext);

if (ctx->hashes) {
 pgpHashListDestroy (ctx->hashes, ctx->numhashes);
 ctx->hashes = NULL;
}

if (ctx->siglist) {
 pgpSigFreeList (ctx->siglist);
 ctx->siglist = 0;
}

memset (ctx, 0, sizeof (*ctx));
pgpMemFree (ctx);
}

static int
tFlush (struct PgpPipeline *myself)
{
		struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == VERIFYRAMAGICT);

		context = (struct Context *)myself->priv;
		pgpAssert (context);
		pgpAssert (context->jointext);

return context->jointext->flush (context->jointext);
}

static size_t
tWrite (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
{
		struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == VERIFYRAMAGICT);
pgpAssert (error);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->jointext);

return context->jointext->write (context->jointext, buf, size, error);
}

static int
tAnnotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
	{
		struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == VERIFYRAMAGICT);

context = (struct Context *)myself->priv;
pgpAssert (context);
pgpAssert (context->jointext);

return context->jointext->annotate (context->jointext, origin, type,
    string, size);
}

static int
tSizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
		struct Context *context;
		int error;

pgpAssert (myself);
pgpAssert (myself->magic == VERIFYRAMAGICT);

context = (struct Context *)myself->priv;
pgpAssert (context);

if (bytes)
 return 0;

error = context->jointext->sizeAdvise (context->jointext, 0);
if (error)
 return error;

context->textclosed = 1;

 return verifySizeAdvise (context);
}

static void
tTeardown (struct PgpPipeline *myself)
{
struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == VERIFYRAMAGICT);

context = (struct Context *)myself->priv;
pgpAssert (context);

context->textdone = 1;
verifyTeardown (myself);
memset (myself, 0, sizeof (*myself));
pgpMemFree (myself);
}

static int
sFlush (struct PgpPipeline *myself)
	{
		struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == VERIFYRAMAGICS);

context = (struct Context *)myself->priv;
pgpAssert (context);

return 0;
}

static size_t
sWrite (struct PgpPipeline *myself, byte const *buf, size_t size, int *error)
	{
		struct Context *context;

pgpAssert (myself);
pgpAssert (myself->magic == VERIFYRAMAGICS);
pgpAssert (error);

context = (struct Context *)myself->priv;
pgpAssert (context);

(void) buf;

if (size) {
 if (context->ignore_text)
  return size;

 *error = PGPERR_VRFYSIG_WRITE;
 return 0;
}

return 0;
}

static int
sAnnotate (struct PgpPipeline *myself, struct PgpPipeline *origin, int type,
		byte const *string, size_t size)
	{
		struct Context *context;
		int error = 0;

		pgpAssert (myself);
		pgpAssert (myself->magic == VERIFYRAMAGICS);

		context = (struct Context *)myself->priv;
		pgpAssert (context);

		(void)origin;
		/*
		* We only accept a few annotations here. Things we dont know
		* are considered an error. Only a known set of annotations are
		* accepted, and most of these are ignored. We are allowed to be
		* in armor, and only a sepsig is allowed.
		*
		* Text is only ignored if it is within a NONPGP scope. Otherwise
		* it is not ignored and an error will be returned.
		*
		* Commit requests are ignored, too.
		*/

		switch (type) {
		case PGPANN_ARMOR_BEGIN:
		case PGPANN_ARMOR_END:
		case PGPANN_FILE_BEGIN:
		case PGPANN_FILE_END:
		case PGPANN_INPUT_BEGIN:
		case PGPANN_INPUT_END:
		case PGPANN_COMMIT:
		case PGPANN_SIGNED_BEGIN:
		case PGPANN_SIGNED_END:
		case PGPANN_SIGNED_SEP:
		 break;

		case PGPANN_NONPGP_BEGIN:
			if (context->ignore_text)
			 break;

			context->ignore_text = 1;
			context->ignore_depth = context->scope_depth + 1;
			break;

		case PGPANN_NONPGP_END:
		 pgpAssert (context->ignore_text);

		 if (context->ignore_depth == context->scope_depth)
		  context->ignore_text = 0;

		 break;

		case PGPANN_SIGNED_SIG:
		 error = pgpSigAdd (&context->siglist, type, string, size);
			if (error) {
				context->ui->message (context->ui_arg, error,
				  PGPMSG_SIG_ADD_ERROR, 0);
			}
			break;

		default:
		 error = PGPERR_VRFYSIG_BADANN;
		}

		PGP_SCOPE_DEPTH_UPDATE (context->scope_depth, type);
		pgpAssert (context->scope_depth != -1);

		return error;
	}

static int
sSizeAdvise (struct PgpPipeline *myself, unsigned long bytes)
	{
		struct Context *context;
		int error = 0;

		pgpAssert (myself);
		pgpAssert (myself->magic == VERIFYRAMAGICS);

		context = (struct Context *)myself->priv;
		pgpAssert (context);

		if (bytes)
		 return 0;

		/* setup the hashes, then close the join module */
		if (!context->siglist)
		 return 0; /* No signatures??? */

		if (!context->numhashes)
		 error = pgpSigSetupHashes
		  (context->jointail, context->env,
		  &context->hashes, &context->numhashes,
		  context->siglist, context->ui, context->ui_arg);
		if (error)
		 return error;

		/* now turn on the pipe to verify the text */
		error = context->joinhead->sizeAdvise (context->joinhead, 0);
		if (error)
		 return error;

context->sigclosed = 1;

return verifySizeAdvise (context);
}

static void
sTeardown (struct PgpPipeline *myself)
{
 struct Context *context;

		pgpAssert (myself);
		pgpAssert (myself->magic == VERIFYRAMAGICS);

		context = (struct Context *)myself->priv;
		pgpAssert (context);

		context->sigdone = 1;
		verifyTeardown (myself);
		memset (myself, 0, sizeof (*myself));
		pgpMemFree (myself);
}

struct PgpPipeline *
pgpVerifyReaderCreate (struct PgpPipeline **texthead,
			struct PgpPipeline **sighead,
			struct PgpEnv const *env, struct PgpFifoDesc const *fd,
			byte const *hashlist, unsigned hashlen, int textmode,
			struct PgpUICb const *ui, void *ui_arg)
	{
		struct PgpPipeline *smod, *tmod;
		struct PgpPipeline *joinhead = NULL, *jointext, **jointail;
		struct Context *context;
		struct PgpHashContext *hashes = NULL;
		int numhashes = 0, err;

if (!texthead || !sighead || !env)
 return NULL;

		context = (struct Context *)pgpMemAlloc (sizeof (*context));
		if (!context)
		 return NULL;
		smod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*smod));
		if (!smod) {
		 pgpMemFree (context);
		 return NULL;
		}
		tmod = (struct PgpPipeline *)pgpMemAlloc (sizeof (*tmod));
		if (!tmod) {
		 pgpMemFree (context);
		 pgpMemFree (smod);
		 return NULL;
		}
		jointail = pgpJoinCreate (&joinhead, fd);
		if (!jointail) {
			pgpMemFree (context);
			pgpMemFree (smod);
			pgpMemFree (tmod);
			return NULL;
		}
jointext = pgpJoinAppend (joinhead);
if (!jointext) {
			joinhead->teardown (joinhead);
			pgpMemFree (context);
			pgpMemFree (smod);
			pgpMemFree (tmod);
			return NULL;
		}
if (hashlist && hashlen) {
			byte const *charmap;

			if (!pgpDevNullCreate (jointail)) {
				joinhead->teardown (joinhead);
				jointext->teardown (jointext);
				pgpMemFree (context);
				pgpMemFree (smod);
				pgpMemFree (tmod);
				return NULL;
			}
			charmap = (byte const *)
			 pgpenvGetPointer (env, PGPENV_CHARMAPTOLATIN1,
			   NULL, NULL);
			err = pgpSigSetupPipeline (jointail, &numhashes, &hashes,
			   hashlist, hashlen,
			   (textmode ? charmap : NULL));
			if (err || numhashes <= 0) {
				joinhead->teardown (joinhead);
				jointext->teardown (jointext);
				pgpMemFree (context);
				pgpMemFree (smod);
				pgpMemFree (tmod);
				return NULL;
			}
			err = joinhead->sizeAdvise (joinhead, 0);
			if (err) {
				joinhead->teardown (joinhead);
				jointext->teardown (jointext);
				pgpHashListDestroy (hashes, numhashes);
				pgpMemFree (context);
				pgpMemFree (smod);
				pgpMemFree (tmod);
				return NULL;
			}
		}

		smod->magic = VERIFYRAMAGICS;
		smod->write = sWrite;
		smod->flush = sFlush;
		smod->sizeAdvise = sSizeAdvise;
		smod->annotate = sAnnotate;
		smod->teardown = sTeardown;
		smod->name = "Signature Verification Annotation Reader (sighead)";
		smod->priv = context;

		tmod->magic = VERIFYRAMAGICT;
		tmod->write = tWrite;
		tmod->flush = tFlush;
		tmod->sizeAdvise = tSizeAdvise;
		tmod->annotate = tAnnotate;
		tmod->teardown = tTeardown;
		tmod->name = "Signature Verification Annotation Reader (texthead)";
		tmod->priv = context;

		memset (context, 0, sizeof (*context));
		context->joinhead = joinhead;
		context->jointext = jointext;
		context->jointail = jointail;
context->env = env;
context->ui = ui;
context->ui_arg = ui_arg;
context->hashes = hashes;
context->numhashes = numhashes;

*sighead = smod;
*texthead = tmod;
return smod;
}
